# Yukon Solitaire Project Specification

This document is a short working specification based on the assignment PDF for 02322 Machine Oriented Programming Project 2.

## 1. Project Goal

Implement the Yukon Solitaire card game using linked lists as the main storage structure for cards.

The final project must include:
- a text-based terminal version
- a GUI-based version
- shared core game logic reused by both versions

The implementation must comply with at least C11.

## 2. Required Data Structures

The following card collections must be implemented using linked lists:
- the deck
- the 7 tableau columns
- the 4 foundation piles

Arrays may still be used for fixed-size helper tasks such as labels, input buffers, and bookkeeping, but not as the main storage for the card collections.

## 3. Game Layout

The game uses 7 tableau columns.

At the start of play, the columns contain:
- `C1`: 1 card
- `C2`: 6 cards
- `C3`: 7 cards
- `C4`: 8 cards
- `C5`: 9 cards
- `C6`: 10 cards
- `C7`: 11 cards

Cards are dealt row by row from left to right.

At the start of a game:
- 21 cards are face down
- the remaining cards are face up

## 4. Objective

The game is won when all cards are moved to the four foundation piles in ascending order from Ace to King.

## 5. Startup Phase Commands

These commands are available before gameplay starts.

### `LD <filename>`

- Loads a deck from a file, or creates the default ordered deck if no file is provided.
- A valid file must contain exactly 52 valid cards.
- The deck must contain all ranks from Ace to King in all four suits.
- Loaded cards are initially hidden in the interface.
- If the file is invalid, report the first detected problem including line number.

### `SW`

- Shows all cards in the current deck in the terminal.
- Returns an error if no deck has been loaded.

### `SI <split>`

- Performs an interleaving shuffle.
- If `<split>` is provided, split the deck at that position.
- If `<split>` is omitted, choose a random split.
- The split must be greater than 0 and less than the number of cards in the deck.

### `SR`

- Performs a random shuffle by inserting each next card into a random position in a new pile.

### `SD <filename>`

- Saves the current deck to a file.
- If no filename is provided, use `cards.txt`.

### `QQ`

- Exits the program.

## 6. Play Phase Commands

### `P`

- Starts a game using the current deck.
- Deals cards into the Yukon tableau.
- After this command, STARTUP-only commands are no longer available.

### `Q`

- Quits the current game and returns to STARTUP phase.
- The deck used for the current game remains in memory, so `P` can start that same setup again.

### Move Commands

Moves use the format:

```text
<from>-><to>
```

There must be no spaces around `->`.

Valid sources:
- `<column>:<card>` such as `C6:4H`
- `<column>` to mean the bottom card of a column
- `<foundation>` such as `F3`

Valid destinations:
- `<column>`
- `<foundation>`

## 7. Move Rules

### Column To Column

- A selected visible card and all visible cards below it may be moved together.
- The selected card must exist.
- The selected card must be visible.
- The destination column bottom card must be one rank higher.
- The selected card must be of a different suit than the destination bottom card.
- If the destination column is empty, only a King may be moved there.

### Column To Foundation

- Only the bottom card of a tableau column may be moved to a foundation.
- If the foundation is empty, only an Ace may be placed there.
- Once an Ace is placed, that foundation becomes tied to that suit.
- Further cards on that foundation must match the suit and increase by one rank each time.

### Foundation To Column

- Only the top card of a foundation may be moved back to a tableau column.
- The move must satisfy the normal column placement rules.

### Reveal Rule

- If a move exposes a hidden card in a tableau column, that card becomes visible.

## 8. Terminal Interface Requirements

The terminal version must:
- display columns `C1` to `C7`
- display foundations `F1` to `F4`
- show visible cards as two-character codes such as `AH`, `7D`, `TC`
- show hidden cards as `[ ]`
- display:
  - `Last Command:`
  - `Message:`
  - `INPUT >`

## 9. GUI Requirements

The GUI is required, but it is less important than the backend and terminal version.

The GUI must support the same core functionality as the terminal version:
- start a game
- display the current state
- make legal moves
- move cards to and from foundations where allowed
- detect when the game is won

The GUI should reuse the same backend game logic instead of reimplementing rules separately.

## 10. Mandatory Vs Optional Scope

### Mandatory

- linked-list-based card storage
- text-based version
- GUI-based version
- shared backend logic
- command handling for startup and play phases
- move validation
- foundation logic
- win detection

### Optional Extensions

- robust handling of arbitrary invalid input
- undo
- redo
- save game state
- load saved game state
- timer
- best completion time tracking

## 11. Expected Error Handling

The program should return useful error messages instead of crashing when:
- no deck is loaded
- a deck file does not exist
- a deck file is malformed
- a move is invalid
- a command is malformed
- a command is used in the wrong phase

## 12. Implementation Direction

Recommended build order:
1. shared backend data model
2. linked-list utilities
3. deck loading and validation
4. terminal rendering and parser
5. move validation and execution
6. play-phase flow
7. GUI on top of the same backend
