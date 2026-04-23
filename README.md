# Yukon Solitaire

This repository contains our course project for `02322 Machine Oriented Programming`.

The project goal is to implement Yukon Solitaire with:
- a terminal version
- a GUI version
- one shared backend for the actual game logic

## Project layout

- `include/`: shared headers for the whole project
- `src/core/`: backend game logic used by both interfaces
- `src/cli/`: terminal app loop, parser, and rendering
- `src/gui/`: GUI bridge in C and Tkinter frontend in Python
- `docs/`: architecture and quality notes
- `docs/report/`: report draft material and templates
- `tests/`: manual testing notes
- `test-data/`: malformed deck samples and other testing files
- `Tasks/`: course PDFs and lab files

## Build with CLion

This project is already set up for CLion through `CMakeLists.txt`.

The main build targets are:
- `yukon_cli`
- `yukon_gui_bridge`

In CLion the normal flow is:
1. Open the project folder.
2. Let CLion load the CMake project.
3. Build `yukon_cli` if you want the terminal version.
4. Build `yukon_gui_bridge` before starting the Tkinter GUI.

## Build from terminal

If `cmake` is available on the machine, a normal local build can look like this:

```bash
cmake -S . -B build
cmake --build build
```

## Run the terminal version

After building, run:

```bash
./build/yukon_cli
```

Or in CLion just run the `yukon_cli` target.

## Run the GUI version

The GUI is a Python Tkinter frontend that talks to the C backend bridge.

Build the bridge first, then run:

```bash
python3 src/gui/tk_yukon.py
```

The GUI tries a few common places for the bridge binary:
- `cmake-build-debug/yukon_gui_bridge`
- `cmake-build-release/yukon_gui_bridge`
- `./yukon_gui_bridge`

## Current implemented scope

Already implemented in the repo:
- startup commands like `LD`, `SW`, `SI`, `SR`, `SD`, `QQ`
- play start and quit with `P` and `Q`
- tableau and foundation move handling
- terminal rendering
- shared command engine for reuse
- GUI bridge plus first working Tkinter interface
- manual tests and malformed deck samples
- quality-check notes for strict builds and sanitizer runs

## Notes for submission

The course PDF is still the source of truth.

The roadmap and project description in the repo are there to keep implementation aligned with the assignment, not to replace it.
