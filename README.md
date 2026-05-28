# MagicBox

An interactive puzzlebox powered by an Arduino Mega 2560 and a few peripherals.

Built as the **meta-game for Bday '26** — a little puzzle game for the nerds. It's a hobby project, so it almost certainly has no real use for anyone else. 🙃

## What it is

A "choose your own adventure" text game played over the serial console, fused with an on-device minigame on an 8×8 LED matrix:

- **Story** — a 4×4 keypad drives choices (A/B/C/D) and PIN entry (`*1234#`). Text is rendered over `Serial` @ 9600 baud.
- **Map** — story scenes can switch on the LED matrix, where you walk a pixel (analog joystick) toward blinking targets. Reaching one jumps back into a specific story node, so map and story drive each other.

## Hardware

Arduino Mega 2560 · 4×4 keypad · MAX7219 8×8 LED matrix · analog joystick. Pins are hard-coded in `src/hardware.cpp`.

## Build & deploy

[PlatformIO](https://platformio.org/) drives everything (env `megaatmega2560`):

```sh
pio run -t upload          # build + flash
pio device monitor -b 9600 # play over serial
```

## Architecture

- `src/main.cpp` — `loop()` runs the map and reads keypad input (incl. the PIN buffer).
- `src/story.cpp` / `include/types.h` — the `StoryGraph` and declarative `StoryNode` literals (choices, PIN, map targets, direct transitions). See `CLAUDE.md` for the authoring macros.
- `src/map.cpp` — LED-matrix minigame and the map↔story bridge.
- `src/hardware.cpp` — pin wiring and peripheral setup.
- `editor.html` — standalone visual node-graph editor (see below).

## Story editor

`editor.html` is a self-contained editor (just open it in a browser — Tailwind + highlight.js via CDN, no build step). Author stories as a node graph instead of hand-writing C++:

- Pan/zoom canvas; nodes are boxes you drag around, with **Tidy** auto-layout.
- Drag from a node's output ports to wire choices, PIN success, map targets, and direct transitions.
- Per-node props panel for id, display text, map config, PIN, and a syntax-highlighted `onEnter` C++ snippet.
- **Export C++** emits the `StoryNode` literals + `storyBegin()` ready to paste into `src/story.cpp`; **JSON** import/export round-trips the full editor state (incl. node positions). Auto-saves to `localStorage`. Dark/light mode.

More detail in [`CLAUDE.md`](./CLAUDE.md).
