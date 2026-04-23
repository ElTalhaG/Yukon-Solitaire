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

- `[done]` Implement `SI <split>` interleaving shuffle with user-provided split.
- `[done]` Implement random split when `SI` is called without an argument.
- `[done]` Implement `SR` random insertion shuffle.
- `[done]` Validate shuffle preconditions such as deck existence and valid split range.
- `[done]` Seed and manage randomness responsibly for repeatable debugging if needed.

## H. Game Setup

- `[done]` Implement `P` to deal the current deck into the Yukon tableau layout.
- `[done]` Ensure row-wise dealing matches the assignment exactly.
- `[done]` Mark the correct cards hidden and visible at game start.
- `[done]` Reset foundations when a new play session begins.
- `[done]` Implement `Q` to leave PLAY phase and return to STARTUP while keeping the deck in memory.

## I. Terminal Rendering

- `[done]` Print the tableau header with `C1` to `C7`.
- `[done]` Print foundations `F1` to `F4` in the required layout.
- `[done]` Render hidden cards as `[ ]`.
- `[done]` Render visible cards with the correct two-character format.
- `[done]` Print `Last Command`, `Message`, and `INPUT >` consistently after each action.

## J. Command Parser

- `[done]` Read user input safely from the terminal.
- `[done]` Parse STARTUP commands.
- `[done]` Parse PLAY commands.
- `[done]` Parse move commands of the form `<from>-><to>` without spaces around `->`.
- `[done]` Reject malformed commands without crashing.

## K. Move Validation

- `[done]` Validate source selection from tableau columns.
- `[done]` Validate source selection from foundations.
- `[done]` Validate moves from column to column.
- `[done]` Validate moves from column to foundation.
- `[done]` Validate moves from foundation back to column.

## L. Move Execution

- `[done]` Move a single card from column bottom when requested.
- `[done]` Move a selected card plus all visible cards beneath it as one segment.
- `[done]` Move only the top card from a foundation.
- `[done]` Reveal a face-down card when it becomes exposed after a move.
- `[done]` Update last-message state to `OK` or the relevant error message.

## M. Foundation Logic

- `[done]` Allow any Ace on any empty foundation.
- `[done]` Lock each foundation to the suit of its first Ace.
- `[done]` Enforce ascending order from Ace to King.
- `[done]` Reject invalid suit and rank placements.
- `[done]` Detect when all cards have reached the foundations.

## N. Phase Management

- `[done]` Block STARTUP-only commands during PLAY.
- `[done]` Block PLAY-only commands during STARTUP.
- `[done]` Keep game state transitions predictable across `LD`, `P`, `Q`, and `QQ`.
- `[done]` Exit cleanly on `QQ`.

## O. Error Handling And Robustness

- `[done]` Prevent crashes on empty input.
- `[done]` Prevent crashes on invalid filenames and missing files.
- `[done]` Prevent crashes on malformed commands and impossible moves.
- `[done]` Handle memory allocation failures gracefully where possible.
- `[done]` Standardize user-facing messages for easier debugging and testing.

## P. Backend Refactoring For Reuse

- `[done]` Separate core game logic from terminal-specific code.
- `[done]` Create a clean interface the GUI can reuse.
- `[done]` Minimize duplicated logic between text and GUI versions.
- `[done]` Move rendering concerns out of the backend logic layer.

## Q. GUI Planning

- `[done]` Choose GUI approach: SDL in C or another language with C backend communication.
- `[done]` Define the GUI architecture around the shared backend.
- `[done]` Decide how moves are entered in the GUI.
- `[done]` Decide how game state is exposed from the backend to the GUI.
- `[done]` Gather any card assets or simple placeholders needed for display.

## R. GUI Implementation

- `[done]` Create the window and main event loop.
- `[done]` Draw tableau columns and foundation piles.
- `[done]` Draw face-up and face-down cards.
- `[done]` Support starting a game and restarting from the current deck.
- `[done]` Support legal moves and visual state updates.

## S. GUI Polish

- `[done]` Show clear feedback for invalid moves.
- `[done]` Show win state when the game is completed.
- `[done]` Improve layout spacing and readability.
- `[done]` Keep the GUI simple but fully functional.

## T. Testing

- `[done]` Create manual test cases for every STARTUP command.
- `[done]` Create manual test cases for every PLAY command.
- `[done]` Test legal and illegal tableau moves.
- `[done]` Test foundation assignment and ordering rules.
- `[done]` Test file loading edge cases and malformed deck files.

## U. Memory And Quality Checks

- `[done]` Compile with strict warnings enabled.
- `[done]` Fix all compiler warnings that matter.
- `[done]` Run leak checks and memory diagnostics.
- `[done]` Review ownership of every allocated node and game structure.

## V. Optional Extensions

- `[not started]` Input hardening for any malformed terminal input.
- `[not started]` Undo support with unlimited rollback to the start of the game.
- `[not started]` Redo support after undo.
- `[not started]` Save current game state with `S <filename>`.
- `[not started]` Load current game state with `L <filename>`.
- `[not started]` Timer for current game duration.
- `[not started]` Best completion time tracking.

## W. Report Writing

- `[done]` Create the report structure early instead of leaving it to the end.
- `[in progress]` Add cover page with project title, names, and photos.
- `[in progress]` Write individualized contribution statements for each group member.
- `[in progress]` Write requirements, analysis, design, implementation, tests, discussion, conclusion, and appendix sections.
- `[in progress]` Include a short discussion of how generative AI was used in the project.

## X. Final Submission Preparation

- `[done]` Verify both terminal and GUI versions are included and working.
- `[done]` Clean the repository structure for submission.
- `[done]` Make sure build and run instructions are clear.
- `[done]` Make sure required files are included and unnecessary files are excluded.

## Y. Final Validation

- `[done]` Re-run the manual test checklist on the final code.
- `[done]` Verify the game starts, plays, and exits cleanly.
- `[done]` Verify the GUI uses the same backend logic as the terminal version.
- `[done]` Verify memory cleanup on exit.

## Z. Delivery

- `[not started]` Tag or archive the final version used for submission.
- `[not started]` Submit code, report, and any required artifacts according to course instructions.
- `[not started]` Keep one final backup of the deliverables outside the main working copy.
