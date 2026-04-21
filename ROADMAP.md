# Yukon Solitaire Project Roadmap

Status labels:
- `done`: completed
- `in progress`: partly completed
- `not started`: not implemented yet

Assumption for current status:
- These labels are based on the current repository snapshot and the project PDFs reviewed on April 18, 2026.

## A. Project Foundation

- `[done]` Repository exists and is connected to GitHub.
- `[done]` Project brief and lab/report PDFs are available in the repository.
- `[done]` High-level assignment requirements have been reviewed.
- `[done]` Agree on folder structure for source, headers, assets, tests, saves, and docs.
- `[done]` Decide coding conventions, file naming, and branch workflow for the team.

## B. Requirements Breakdown

- `[done]` Convert the PDF requirements into a short written specification in the repo.
- `[done]` Separate mandatory features from optional extensions.
- `[done]` List all STARTUP commands and their expected behaviors.
- `[done]` List all PLAY commands and their expected behaviors.
- `[done]` Define error cases and expected messages for invalid input and invalid moves.

## C. Core Data Model

- `[done]` Design the `Card` node structure with rank, suit, visibility, and next pointer.
- `[done]` Design linked-list structures for the deck, tableau columns, and foundations.
- `[done]` Define a full game-state structure that can be reused by both terminal and GUI versions.
- `[done]` Decide how to represent phase state: STARTUP vs PLAY.
- `[done]` Decide where last command and last message are stored for the terminal UI.

## D. Card Utilities

- `[done]` Implement rank and suit parsing helpers.
- `[done]` Implement card validation helpers.
- `[done]` Implement card formatting for terminal output such as `AH`, `TD`, and `[ ]`.
- `[done]` Implement rank comparison helpers for tableau and foundation rules.
- `[done]` Implement suit comparison helpers.

## E. Linked-List Operations

- `[done]` Implement node creation and destruction.
- `[done]` Implement push/pop operations for pile manipulation.
- `[done]` Implement list traversal helpers.
- `[done]` Implement split, splice, append, and segment-move helpers.
- `[done]` Implement memory cleanup for every game structure.

## F. Deck Loading And Saving

- `[done]` Implement `LD <filename>` with default deck generation when no filename is given.
- `[done]` Validate deck files for line format, duplicates, suit counts, and exact size of 52 cards.
- `[done]` Report the first file error with a useful line number.
- `[done]` Implement `SD <filename>` with default fallback filename.
- `[done]` Ensure load/save preserve deck order exactly.

## G. Shuffle Features

- `[not started]` Implement `SI <split>` interleaving shuffle with user-provided split.
- `[not started]` Implement random split when `SI` is called without an argument.
- `[not started]` Implement `SR` random insertion shuffle.
- `[not started]` Validate shuffle preconditions such as deck existence and valid split range.
- `[not started]` Seed and manage randomness responsibly for repeatable debugging if needed.

## H. Game Setup

- `[not started]` Implement `P` to deal the current deck into the Yukon tableau layout.
- `[not started]` Ensure row-wise dealing matches the assignment exactly.
- `[not started]` Mark the correct cards hidden and visible at game start.
- `[not started]` Reset foundations when a new play session begins.
- `[not started]` Implement `Q` to leave PLAY phase and return to STARTUP while keeping the deck in memory.

## I. Terminal Rendering

- `[not started]` Print the tableau header with `C1` to `C7`.
- `[not started]` Print foundations `F1` to `F4` in the required layout.
- `[not started]` Render hidden cards as `[ ]`.
- `[not started]` Render visible cards with the correct two-character format.
- `[not started]` Print `Last Command`, `Message`, and `INPUT >` consistently after each action.

## J. Command Parser

- `[not started]` Read user input safely from the terminal.
- `[not started]` Parse STARTUP commands.
- `[not started]` Parse PLAY commands.
- `[not started]` Parse move commands of the form `<from>-><to>` without spaces around `->`.
- `[not started]` Reject malformed commands without crashing.

## K. Move Validation

- `[not started]` Validate source selection from tableau columns.
- `[not started]` Validate source selection from foundations.
- `[not started]` Validate moves from column to column.
- `[not started]` Validate moves from column to foundation.
- `[not started]` Validate moves from foundation back to column.

## L. Move Execution

- `[not started]` Move a single card from column bottom when requested.
- `[not started]` Move a selected card plus all visible cards beneath it as one segment.
- `[not started]` Move only the top card from a foundation.
- `[not started]` Reveal a face-down card when it becomes exposed after a move.
- `[not started]` Update last-message state to `OK` or the relevant error message.

## M. Foundation Logic

- `[not started]` Allow any Ace on any empty foundation.
- `[not started]` Lock each foundation to the suit of its first Ace.
- `[not started]` Enforce ascending order from Ace to King.
- `[not started]` Reject invalid suit and rank placements.
- `[not started]` Detect when all cards have reached the foundations.

## N. Phase Management

- `[not started]` Block STARTUP-only commands during PLAY.
- `[not started]` Block PLAY-only commands during STARTUP.
- `[not started]` Keep game state transitions predictable across `LD`, `P`, `Q`, and `QQ`.
- `[not started]` Exit cleanly on `QQ`.

## O. Error Handling And Robustness

- `[not started]` Prevent crashes on empty input.
- `[not started]` Prevent crashes on invalid filenames and missing files.
- `[not started]` Prevent crashes on malformed commands and impossible moves.
- `[not started]` Handle memory allocation failures gracefully where possible.
- `[not started]` Standardize user-facing messages for easier debugging and testing.

## P. Backend Refactoring For Reuse

- `[not started]` Separate core game logic from terminal-specific code.
- `[not started]` Create a clean interface the GUI can reuse.
- `[not started]` Minimize duplicated logic between text and GUI versions.
- `[not started]` Move rendering concerns out of the backend logic layer.

## Q. GUI Planning

- `[not started]` Choose GUI approach: SDL in C or another language with C backend communication.
- `[not started]` Define the GUI architecture around the shared backend.
- `[not started]` Decide how moves are entered in the GUI.
- `[not started]` Decide how game state is exposed from the backend to the GUI.
- `[not started]` Gather any card assets or simple placeholders needed for display.

## R. GUI Implementation

- `[not started]` Create the window and main event loop.
- `[not started]` Draw tableau columns and foundation piles.
- `[not started]` Draw face-up and face-down cards.
- `[not started]` Support starting a game and restarting from the current deck.
- `[not started]` Support legal moves and visual state updates.

## S. GUI Polish

- `[not started]` Show clear feedback for invalid moves.
- `[not started]` Show win state when the game is completed.
- `[not started]` Improve layout spacing and readability.
- `[not started]` Keep the GUI simple but fully functional.

## T. Testing

- `[not started]` Create manual test cases for every STARTUP command.
- `[not started]` Create manual test cases for every PLAY command.
- `[not started]` Test legal and illegal tableau moves.
- `[not started]` Test foundation assignment and ordering rules.
- `[not started]` Test file loading edge cases and malformed deck files.

## U. Memory And Quality Checks

- `[not started]` Compile with strict warnings enabled.
- `[not started]` Fix all compiler warnings that matter.
- `[not started]` Run leak checks and memory diagnostics.
- `[not started]` Review ownership of every allocated node and game structure.

## V. Optional Extensions

- `[not started]` Input hardening for any malformed terminal input.
- `[not started]` Undo support with unlimited rollback to the start of the game.
- `[not started]` Redo support after undo.
- `[not started]` Save current game state with `S <filename>`.
- `[not started]` Load current game state with `L <filename>`.
- `[not started]` Timer for current game duration.
- `[not started]` Best completion time tracking.

## W. Report Writing

- `[not started]` Create the report structure early instead of leaving it to the end.
- `[not started]` Add cover page with project title, names, and photos.
- `[not started]` Write individualized contribution statements for each group member.
- `[not started]` Write requirements, analysis, design, implementation, tests, discussion, conclusion, and appendix sections.
- `[not started]` Include a short discussion of how generative AI was used in the project.

## X. Final Submission Preparation

- `[not started]` Verify both terminal and GUI versions are included and working.
- `[not started]` Clean the repository structure for submission.
- `[not started]` Make sure build and run instructions are clear.
- `[not started]` Make sure required files are included and unnecessary files are excluded.

## Y. Final Validation

- `[not started]` Re-run the manual test checklist on the final code.
- `[not started]` Verify the game starts, plays, and exits cleanly.
- `[not started]` Verify the GUI uses the same backend logic as the terminal version.
- `[not started]` Verify memory cleanup on exit.

## Z. Delivery

- `[not started]` Tag or archive the final version used for submission.
- `[not started]` Submit code, report, and any required artifacts according to course instructions.
- `[not started]` Keep one final backup of the deliverables outside the main working copy.
