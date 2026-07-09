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

The single configured environment is `[env:megaatmega2560]` in `platformio.ini` (platform `atmelavr`, framework `arduino`). `.pio/` is the build cache (gitignored). `build_flags = -std=gnu++17` (unflagging `gnu++11`) is **required** — `StoryNode` literals use designated initializers. `extra_scripts = pre:inject_build_id.py` injects a fresh `FIRMWARE_BUILD_ID` each build (used by the EEPROM save below; this also forces a project recompile each build).

> avr-gcc 7 caveat: designated initializers must set a **contiguous prefix** of struct fields (in order, no gaps); trailing fields may be omitted (zero-initialized). Hence the authoring macros below.

## Architecture

The runtime has three concurrent surfaces driven from `loop()` in `src/main.cpp`:

1. **Story engine** — keypad input is mapped to a `Choice` (A/B/C/D) or a PIN and dispatched to a `StoryGraph`. Story text is rendered over `Serial`.
2. **Map minigame** — joystick moves a lit pixel on the 8×8 LED matrix; target pixels blink. Polled every loop iteration. Only active while a story node enabled it.
3. **NFC reader** — `nfcPoll()` runs first in every loop iteration (before `mapWalk()`, which can `delay()`). Only does radio work while a read/write request is pending.

`handleInput()` in `main.cpp` also runs the **PIN buffer**: `*` starts/resets capture, digits accumulate, `#` submits the digits since the last `*` to `handlePin()` (so `*234*4652#` → `4652`, `2345#` → ignored).

Hardware globals (`customKeypad`, `lc`, joystick pins) live in `src/hardware.cpp` / `include/hardware.h`, wired up by `setupHardware()`. Pin assignments are hard-coded there — change them in one place.

### Story graph

`StoryGraph` is a fixed-capacity (`nodes[20]`) collection of `StoryNode`s. **Engine** (`StoryGraph` + handlers) lives in `src/story.cpp`; the **authored content** (node literals + `storyBegin()`) lives in `src/story_content.cpp` — that file is what the editor's "Export C++" overwrites wholesale, so the engine is never clobbered. The graph is **fully declarative**: each `StoryNode` literal wires itself via direct `StoryNode*` pointers (choices, `pinSuccess`, `next`) — there is no `connectNodes`/`connectPin`. `storyBegin()` just `addNode(&n)`s every node (feeding the `id`→node lookup used by `setGameState`/map collisions) and `jumpToNode(startId)`.

Author nodes with designated initializers + the macros in `types.h`:

- `CHOICES(a,b,c,d)` — the four choice targets (`nullptr` = invalid → `printWrongChoice()`). Implicitly sets `.next = nullptr`. `NO_CHOICES` = all `nullptr`.
- `GOTO(&node)` — **direct transition**: after the display text, auto-advance to `node` without input (used to merge branches). `enterNode` follows the `next` chain in a loop (guarded to 64 hops).
- `MAP_TARGETS({x,y,storyID}, …)` — activate the map + register blinking targets; `MAP_OFF` is the explicit gap-filler. `PIN("1234", &successNode)` — PIN entry.
- `NFC("TEXT", &successNode)` — NFC tag gate (sets `expectedNfc`/`nfcSuccess`, see below); `NFC_OFF` is the gap-filler needed when `.onEnter` is set without NFC. The NFC fields sit between `pinSuccess` and `onEnter` in the struct; `onEnter` stays last.
- `.display` is a `const __FlashStringHelper*` pointing into **flash** (not RAM — keeps long paragraphs out of the 8 KB SRAM). Author it as `.display = FSTR(T_node)` with a preceding `const char T_node[] PROGMEM = "...";`. The editor's "Export C++" emits one PROGMEM array per node automatically.

`enterNode()` on each transition: `nfcCancel()` → `mapDisable()` → print display → apply map config → run optional `onEnter` (escape-hatch callback, e.g. `mapTeleportPlayer(x,y)`) → follow `next` → arm NFC for the resting node → **save the resting node id to EEPROM**.

### Progress persistence (EEPROM)

`enterNode()` writes `{FIRMWARE_BUILD_ID, currentNode->id}` to EEPROM (addr 0, `EEPROM.put` = update-based, no needless wear). The generated `storyBegin()` ends with `resumeOrStart(startId)` (not `jumpToNode`): on boot it jumps to the saved node iff the stored build id matches this firmware's `FIRMWARE_BUILD_ID`. So a **power loss resumes** where you were, but a **fresh flash** (new build id from `inject_build_id.py`) ignores the stale save and restarts. Only the node id is saved — transient map state (player position, dynamically-added targets/blockers) is reconstructed from the resumed node's declarative config, not restored.

### Map ↔ Story integration

`map.cpp` exposes `mapEnable/mapDisable/mapIsActive`, `mapSetTarget(x,y,storyID)` (add or overwrite — targets persist across disables for progress), `mapRemoveTarget`, `mapClearTargets`, `mapTeleportPlayer`. Walking onto a target calls `setGameState(storyID)` → jumps to that node and disables the map. Rendering uses a cached `frameBuffer[8]` + `lc.setRow` (atomic, flicker-free); targets blink via `millis()`.

**Blockers** are map cells with `storyID == MAP_BLOCK` (sentinel `-1`), stored in the same table as targets: they render **constant-lit** (no blink), the player cannot walk onto them, and they trigger no story. Set via `mapSetBlocker(x,y)` / declaratively via `MAP_TARGETS({x,y,MAP_BLOCK})`; remove with `mapRemoveBlocker(x,y)` or `mapClearBlockers()` (clears only blockers, keeps real targets). The editor has a "Map blockers" list per node that exports as `MAP_BLOCK` entries.

### NFC (MFRC522)

`nfc.cpp`/`nfc.h` wrap the RC522 reader (vendored `lib/rfid`, hardware SPI: MISO 50 / MOSI 51 / SCK 52, SS 53 / RST 49 in `hardware.cpp`, **3.3 V supply**) as an async request/callback module:

- `nfcRequestRead(cb)` / `nfcRequestWrite(text, cb)` arm a pending operation (replacing any previous one); `nfcPoll()` executes it against the next presented tag and fires the callback. `nfcCancel()`, `nfcBusy()` round out the API.
- **Tag payload**: 16 bytes, null-terminated text, max 15 chars (`NFC_TEXT_MAX`). Supported tags are auto-detected via SAK: MIFARE Classic Mini/1K/4K (block 4 = sector 1, factory key A `FF×6`, `PCD_StopCrypto1()` after every transaction) and NTAG/Ultralight (pages 4–7, no auth; NTAG stickers can be written with a phone app).
- **Errors keep the request armed** (retry with another tag); the write callback only ever fires with `success == true`. A 2 s same-tag cooldown stops a tag resting on the reader from re-triggering.
- **Story integration** (`story.cpp`): after each transition the resting node arms a read iff it has `expectedNfc` or `onNfc` — unless something (e.g. `onEnter`) already requested a write, which takes priority; reads then re-arm on the next transition. On a read, precedence is: `onNfc(text)` fires first (sees every tag, may transition — then the old node's gate is skipped) → `expectedNfc` gate (`strcmp` match → `enterNode(nfcSuccess)`) → no transition → re-arm.

### Types

`include/types.h` is the shared vocabulary: `Direction`, `Position` (bounds-checked `move()` clamped 0–7), `Choice`, `MapTarget`, and `StoryNode` (a plain aggregate — see macros above). Inline helpers (`charToChoice`, `printSerial`, `printSerialBlock`, `printDirection`) are header-only in `include/utils.h`.

### Visual editor

`editor.html` (project root, standalone — Tailwind + highlight.js CDNs, no build) is a node-graph editor for authoring stories: pan/zoom canvas, drag-to-connect ports, per-field props panel, an `onEnter` C++ code editor, Tidy auto-layout. **Export C++** emits the node literals + `storyBegin()` ready to paste into `story.cpp`; JSON import/export round-trips the editor state (incl. positions).

### Libraries

`lib/` holds **vendored** library sources (Keypad, LedControl, rfid/MFRC522, plus several currently-unused sensors/actuators: DHT, DS3231, HC-SR04, IRremote, LiquidCrystal, MPU6050, QMI8658C, Servo, Stepper, pitches). PlatformIO's LDF picks them up automatically (incl. the framework `SPI` lib via MFRC522's include); don't add them as `lib_deps` in `platformio.ini`. Keypad, LedControl and rfid are linked from the current source.

## Conventions

- Headers use uppercase include guards (`MAGICBOX_*_H`); match the pattern when adding new ones.
- Serial output uses the helpers in `utils.h`: `printSerialBlock()` wraps a paragraph in a separator + blank lines (story node text, result messages); `printSerial()` just prints raw text (inline echoes). Prefer these over raw `Serial.print*` in project code (`src/`); vendored `lib/` code is left untouched.
- The LED matrix is addressed as `lc.setLed(0, x, y, on)`. `Position::move()` already clamps to the 0–7 grid.
