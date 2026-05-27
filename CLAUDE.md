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

The single configured environment is `[env:megaatmega2560]` in `platformio.ini` (platform `atmelavr`, framework `arduino`). `.pio/` is the build cache (gitignored).

## Architecture

The runtime has two concurrent surfaces driven from `loop()` in `src/main.cpp`:

1. **Story engine** — keypad input (A/B/C/D) is mapped to a `Choice` and dispatched to a `StoryGraph`. Story text is rendered over `Serial`.
2. **Map minigame** — joystick input moves a single lit pixel on the 8×8 LED matrix. Polled every loop iteration with a 300 ms debounce delay inside `mapWalk()`.

Hardware globals (`customKeypad`, `lc`, joystick pins) live in `src/hardware.cpp` / `include/hardware.h` and are wired up by `setupHardware()`. Pin assignments are hard-coded there — change them in one place.

### Story graph

`StoryGraph` (in `src/story.cpp`) is a fixed-capacity (`nodes[20]`) collection of `StoryNode`s. Each node has a `gameStateID`, a display string, and four child pointers (`a`/`b`/`c`/`d`) — one per `Choice`. Story content is authored as:

1. `StoryNode` literal definitions at file scope (e.g. `startNode`, `forestNode`, `caveNode`).
2. `storyGraph.addNode(&node)` to register each node.
3. `storyGraph.connectNodes(fromID, Choice::X, toID)` to wire transitions.
4. `storyGraph.jumpToNode(id)` to set the starting state.

Both registration and wiring happen in `storyBegin()`, which is called once from `setup()`. A `nullptr` child means that choice is invalid — `handleChoice` falls through to `printWrongChoice()`. Adding a new scene means defining the `StoryNode`, calling `addNode`, and adding the relevant `connectNodes` calls; the `gameStateID` is the addressing scheme.

### Types

`include/types.h` is the shared vocabulary: `Direction`, `Position` (with bounds-checked `move()` clamped to 0–7 for the LED matrix), `Choice` (A/B/C/D backed by their ASCII chars), and `StoryNode`. Inline helpers (`charToChoice`, `printSerial`, `printDirection`) live in `include/utils.h` and are header-only.

### Libraries

`lib/` holds **vendored** library sources (Keypad, LedControl, plus several currently-unused sensors/actuators: DHT, DS3231, HC-SR04, IRremote, LiquidCrystal, MPU6050, QMI8658C, Servo, Stepper, pitches, rfid). PlatformIO's LDF picks them up automatically; don't add them as `lib_deps` in `platformio.ini`. Only Keypad and LedControl are linked from the current source.

## Conventions

- Headers use uppercase include guards (`MAGICBOX_*_H`); match the pattern when adding new ones.
- Serial output uses `printSerial()` from `utils.h` (wraps lines with a separator) and `F("...")` macros to keep strings in flash — preserve this when adding messages.
- The LED matrix is addressed as `lc.setLed(0, x, y, on)`. `Position::move()` already clamps to the 0–7 grid.
