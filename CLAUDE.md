# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

MagicBox is a PlatformIO/Arduino project targeting the **ATmega2560** (Arduino Mega 2560). It is an interactive "choose your own adventure" text game played over `Serial` (9600 baud), combined with an on-device minigame using an 8×8 LED matrix and analog joystick.

## Build & Upload

PlatformIO drives the build. There is no custom Makefile or test runner.

```sh
pio run                          # build for env:megaatmega2560
pio run -t upload                # build + flash
pio run -t clean                 # clean
pio device monitor -b 9600       # open serial monitor (game I/O)
pio device monitor -b 9600 -e megaatmega2560
```

The single configured environment is `[env:megaatmega2560]` in `platformio.ini` (platform `atmelavr`, framework `arduino`). `.pio/` is the build cache (gitignored). `build_flags = -std=gnu++17` (unflagging `gnu++11`) is **required** — `StoryNode` literals use designated initializers.

> avr-gcc 7 caveat: designated initializers must set a **contiguous prefix** of struct fields (in order, no gaps); trailing fields may be omitted (zero-initialized). Hence the authoring macros below.

## Architecture

The runtime has two concurrent surfaces driven from `loop()` in `src/main.cpp`:

1. **Story engine** — keypad input is mapped to a `Choice` (A/B/C/D) or a PIN and dispatched to a `StoryGraph`. Story text is rendered over `Serial`.
2. **Map minigame** — joystick moves a lit pixel on the 8×8 LED matrix; target pixels blink. Polled every loop iteration. Only active while a story node enabled it.

`handleInput()` in `main.cpp` also runs the **PIN buffer**: `*` starts/resets capture, digits accumulate, `#` submits the digits since the last `*` to `handlePin()` (so `*234*4652#` → `4652`, `2345#` → ignored).

Hardware globals (`customKeypad`, `lc`, joystick pins) live in `src/hardware.cpp` / `include/hardware.h`, wired up by `setupHardware()`. Pin assignments are hard-coded there — change them in one place.

### Story graph

`StoryGraph` is a fixed-capacity (`nodes[20]`) collection of `StoryNode`s. **Engine** (`StoryGraph` + handlers) lives in `src/story.cpp`; the **authored content** (node literals + `storyBegin()`) lives in `src/story_content.cpp` — that file is what the editor's "Export C++" overwrites wholesale, so the engine is never clobbered. The graph is **fully declarative**: each `StoryNode` literal wires itself via direct `StoryNode*` pointers (choices, `pinSuccess`, `next`) — there is no `connectNodes`/`connectPin`. `storyBegin()` just `addNode(&n)`s every node (feeding the `id`→node lookup used by `setGameState`/map collisions) and `jumpToNode(startId)`.

Author nodes with designated initializers + the macros in `types.h`:

- `CHOICES(a,b,c,d)` — the four choice targets (`nullptr` = invalid → `printWrongChoice()`). Implicitly sets `.next = nullptr`. `NO_CHOICES` = all `nullptr`.
- `GOTO(&node)` — **direct transition**: after the display text, auto-advance to `node` without input (used to merge branches). `enterNode` follows the `next` chain in a loop (guarded to 64 hops).
- `MAP_TARGETS({x,y,storyID}, …)` — activate the map + register blinking targets; `MAP_OFF` is the explicit gap-filler. `PIN("1234", &successNode)` — PIN entry.
- `.display` is a `const __FlashStringHelper*` pointing into **flash** (not RAM — keeps long paragraphs out of the 8 KB SRAM). Author it as `.display = FSTR(T_node)` with a preceding `const char T_node[] PROGMEM = "...";`. The editor's "Export C++" emits one PROGMEM array per node automatically.

`enterNode()` on each transition: `mapDisable()` → print display → apply map config → run optional `onEnter` (escape-hatch callback, e.g. `mapTeleportPlayer(x,y)`) → follow `next`.

### Map ↔ Story integration

`map.cpp` exposes `mapEnable/mapDisable/mapIsActive`, `mapSetTarget(x,y,storyID)` (add or overwrite — targets persist across disables for progress), `mapRemoveTarget`, `mapClearTargets`, `mapTeleportPlayer`. Walking onto a target calls `setGameState(storyID)` → jumps to that node and disables the map. Rendering uses a cached `frameBuffer[8]` + `lc.setRow` (atomic, flicker-free); targets blink via `millis()`.

### Types

`include/types.h` is the shared vocabulary: `Direction`, `Position` (bounds-checked `move()` clamped 0–7), `Choice`, `MapTarget`, and `StoryNode` (a plain aggregate — see macros above). Inline helpers (`charToChoice`, `printSerial`, `printSerialBlock`, `printDirection`) are header-only in `include/utils.h`.

### Visual editor

`editor.html` (project root, standalone — Tailwind + highlight.js CDNs, no build) is a node-graph editor for authoring stories: pan/zoom canvas, drag-to-connect ports, per-field props panel, an `onEnter` C++ code editor, Tidy auto-layout. **Export C++** emits the node literals + `storyBegin()` ready to paste into `story.cpp`; JSON import/export round-trips the editor state (incl. positions).

### Libraries

`lib/` holds **vendored** library sources (Keypad, LedControl, plus several currently-unused sensors/actuators: DHT, DS3231, HC-SR04, IRremote, LiquidCrystal, MPU6050, QMI8658C, Servo, Stepper, pitches, rfid). PlatformIO's LDF picks them up automatically; don't add them as `lib_deps` in `platformio.ini`. Only Keypad and LedControl are linked from the current source.

## Conventions

- Headers use uppercase include guards (`MAGICBOX_*_H`); match the pattern when adding new ones.
- Serial output uses the helpers in `utils.h`: `printSerialBlock()` wraps a paragraph in a separator + blank lines (story node text, result messages); `printSerial()` just prints raw text (inline echoes). Prefer these over raw `Serial.print*` in project code (`src/`); vendored `lib/` code is left untouched.
- The LED matrix is addressed as `lc.setLed(0, x, y, on)`. `Position::move()` already clamps to the 0–7 grid.
