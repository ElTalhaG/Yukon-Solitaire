# Team Rules And Workflow

This file defines the working conventions for the Yukon Solitaire project so the codebase stays consistent while multiple people contribute.

## 1. General Principles

- Keep the shared game rules in `src/core/`.
- Keep terminal-only behavior in `src/cli/`.
- Keep GUI-only behavior in `src/gui/`.
- Put reusable declarations in `include/`.
- Prefer small, readable commits over large mixed commits.

## 2. Naming Conventions

### Files

- Use lowercase file names.
- Use short, descriptive names.
- Use `.c` for implementation files and `.h` for headers.
- Examples:
  - `card.c`
  - `card.h`
  - `game_state.c`
  - `move_rules.h`

### Functions

- Use `snake_case`.
- Start function names with the module name when helpful.
- Examples:
  - `card_create`
  - `deck_append_card`
  - `game_start_play`
  - `move_is_valid`

### Variables

- Use `snake_case`.
- Use descriptive names instead of short unclear names.
- Good examples:
  - `source_column`
  - `destination_foundation`
  - `last_command`

### Types

- Use `PascalCase` for structs, enums, and typedef names.
- Examples:
  - `Card`
  - `Deck`
  - `GameState`
  - `GamePhase`

### Constants And Macros

- Use uppercase with underscores.
- Examples:
  - `TABLEAU_COLUMNS`
  - `FOUNDATION_COUNT`
  - `MAX_COMMAND_LENGTH`

## 3. Formatting Rules

- Use 4 spaces for indentation.
- Keep lines reasonably short and readable.
- Use braces consistently for conditionals and loops.
- Prefer one declaration per line.
- Add comments only when they explain non-obvious logic.
- Avoid deeply nested control flow where possible.

## 4. Header Rules

- Every header should have include guards.
- Headers should expose interfaces, not internal implementation details unless necessary.
- If a function is only used in one `.c` file, prefer `static` instead of putting it in a header.

## 5. Git Workflow

- Keep `main` as the stable branch.
- For larger tasks, create a short-lived feature branch and merge it back into `main`.
- For very small documentation or cleanup tasks, direct commits to `main` are acceptable if the repo is quiet.
- Pull or fetch before pushing if others may have updated the repo.
- If push is rejected, fetch first and rebase or merge carefully.

## 6. Commit Style

- Commit one logical change at a time.
- Keep commit messages short and specific.
- Good examples:
  - `Add project structure guide`
  - `Implement card parsing helpers`
  - `Validate deck file input`

## 7. Division Of Responsibilities

- Shared backend code should be reviewed carefully before GUI code depends on it.
- If one person edits `src/core/`, the other should avoid changing the same files at the same time unless coordinated.
- Report writing can happen in parallel, but the final report should reflect the actual implementation.

## 8. Testing Expectations

- New command behavior should be tested manually before being pushed.
- Invalid input cases should be tested as well as normal cases.
- Any bug fix should include at least one repeatable test case in `tests/` or `docs/report/`.

## 9. AI Usage

- AI can help with planning, explanations, refactoring suggestions, and documentation.
- AI-generated code must still be read, understood, tested, and adapted to the assignment requirements before being trusted.
