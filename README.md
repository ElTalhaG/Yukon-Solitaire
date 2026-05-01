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

Better using your build-in terminal with the respective project path. 
The executable is here, just copy-paste:

```bash
./cmake-build-debug/yukon_cli
```

Or in CLion just run the `yukon_cli` target.

### Terminal command guide

The terminal program has two phases:
- STARTUP: load, show, shuffle, save, start, or quit
- PLAY: move cards, quit the current game, or quit the program

Useful STARTUP commands:
- `LD`: load the default ordered deck
- `LD <filename>`: load a deck from a file
- `SW`: show the currently loaded deck
- `SI 26`: interleave shuffle with split position 26
- `SI`: interleave shuffle with a random split
- `SR`: random insertion shuffle
- `SD <filename>`: save the current deck order
- `P`: start the game
- `QQ`: quit the program

Useful PLAY commands:
- `C1->F1`: move the bottom card from column 1 to foundation 1
- `C2->C7`: move the bottom card from column 2 to column 7
- `C5:JD->C1`: move `JD` and all visible cards below it from column 5 to column 1
- `F1->C3`: move the top foundation card from foundation 1 back to column 3
- `Q`: quit the current game and return to STARTUP
- `QQ`: quit the program

The strict command format uses no spaces around `->`, but the parser also accepts
some common typing mistakes now:
- lowercase commands, such as `ld`, `sw`, and `p`
- lowercase card codes, such as `c5:jd->c1`
- extra spaces around `->`, such as `C5:JD -> C1`
- extra spaces around `:`, such as `C5 : JD -> C1`

Good quick test flow:

```text
LD
SW
P
C1->F1
Q
QQ
```

Good selected-card move format:

```text
C<source column>:<card code>->C<destination column>
```

Example:

```text
C6:4H->C4
```

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

### for users with macOS + Homebrew "note"

If you use Homebrew Python on macOS, `tkinter` may be missing even though Python itself works.

For the Python `3.12` setup used in this project, the fix is:

```bash
brew install python-tk@3.12
```

After that, the project virtual environment can use Tkinter normally.

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
- optional input hardening for common command typing mistakes

## Notes for submission

The course PDF is still the source of truth.

The roadmap and project description in the repo are there to keep implementation aligned with the assignment, not to replace it.
