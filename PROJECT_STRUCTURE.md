# Project Structure

This repository is organized to keep the shared Yukon Solitaire backend separate from the terminal UI, GUI, tests, and report material.

## Directories

- `src/core/`
  Shared game logic in C. This is where the linked-list data structures, command logic, move validation, shuffle logic, and game-state handling should live.

- `src/cli/`
  Terminal-specific code for input parsing, screen rendering, prompts, and command dispatch.

- `src/gui/`
  GUI-specific code. This layer should call into the shared backend instead of reimplementing the rules.

- `include/`
  Header files for the project. Keep public interfaces here so both the CLI and GUI can reuse the same backend declarations.

- `tests/`
  Manual test notes, test drivers, and later any small verification programs.

- `test-data/`
  Input decks, malformed deck samples, and saved game samples used for testing.

- `assets/cards/`
  Card images or placeholders for the GUI if the project uses them.

- `docs/report/`
  Report drafts, figures, diagrams, and other report-writing assets.

- `saves/`
  Optional save files for the extension tasks.

- `build/`
  Local build output. This is useful during development, but generated files should usually not be committed unless needed by the course.

## Suggested Near-Term File Layout

As implementation begins, a good next step is to add files roughly like this:

```text
include/
  card.h
  game.h
  deck.h
  cli.h

src/core/
  card.c
  deck.c
  game.c
  moves.c
  shuffle.c

src/cli/
  main.c
  parser.c
  render.c

src/gui/
  main.c
  bridge.c
```

## Rule For New Code

If code is reusable between terminal and GUI, it belongs in `src/core/` and should be exposed through `include/`.
