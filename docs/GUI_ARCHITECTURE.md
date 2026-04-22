# GUI Architecture

This project now uses a simple two-part GUI approach:

1. A C backend bridge executable called `yukon_gui_bridge`
2. A Python Tkinter frontend called `tk_yukon.py`

## Why this approach

The course allows the GUI to be written in another language as long as the C backend still owns the game logic.

This setup gives us that:
- the rules stay in C
- the parser stays in C
- the command execution stays in C
- the GUI only draws the current state and sends commands

So the Python part is really just the window and widgets. The actual solitaire logic is still the same shared backend as the terminal version.

## How data flows

The frontend starts the C bridge as a subprocess.

The Python GUI sends normal command strings like:
- `LD`
- `SW`
- `P`
- `C1->F1`
- `Q`

The bridge parses and executes those commands using the same backend modules as the CLI.

After each command, the bridge prints a machine-readable state dump.
The GUI reads that dump and redraws the screen.

## Why this is nice

- No duplicated game rules in the GUI
- The GUI and CLI stay consistent
- Debugging is easier because the same commands work in both places
- We can keep the GUI simple and still stay inside the course requirements

## Move input choice

For now, the GUI accepts moves through a command entry field.

That means the user can type exact backend commands such as:
- `C1->F1`
- `C6:4H->C4`
- `F2->C3`

This is not the fanciest interaction style, but it is very practical right now:
- it keeps the GUI small
- it makes testing easier
- it directly reuses the parser we already trust

Later, if there is time, drag-and-drop or click-select interactions can be added on top.

## State exposure format

The bridge prints a custom line-based state format between:
- `BEGIN_STATE`
- `END_STATE`

That format includes:
- current phase
- last command
- current message
- startup deck visibility mode
- deck cards in saved order
- tableau cards
- foundation top cards and sizes

This format is intentionally plain text so it is easy to inspect and debug by hand.

## Assets

Right now the GUI uses simple drawn rectangles and text labels instead of image assets.

That is enough for the required functionality and keeps the project lightweight.
If we want more polish later, we can add real card images in `assets/cards/`.
