# Manual Test Plan

This file is the practical "sit down and try it" checklist for the project.
The idea is not to be fancy. The idea is just to have a repeatable set of tests
we can run before demos, commits, and final submission.

## 1. Startup Commands

### `LD`

Input:
```text
LD
```

Expected:
- message becomes `OK`
- startup view shows the Yukon layout with hidden cards as `[ ]`
- deck size is effectively 52 in the backend

### `LD <filename>`

Input:
```text
LD test-data/invalid_bad_code.txt
```

Expected:
- message reports an invalid card and line number
- program does not crash

Input:
```text
LD test-data/invalid_short_deck.txt
```

Expected:
- message says the deck must contain exactly 52 cards
- program does not crash

Input:
```text
LD test-data/invalid_duplicate_deck.txt
```

Expected:
- message reports a duplicate card and line number
- program does not crash

### `SW`

Input:
```text
SW
```

Expected:
- if no deck is loaded: message says no deck is loaded
- if a deck is loaded: message becomes `OK`
- startup deck becomes visible with codes like `AC`, `2C`, `TD`

### `SI`

Input:
```text
LD
SI 26
```

Expected:
- message becomes `OK`
- deck order changes
- deck still has 52 cards

Input:
```text
SI
```

Expected:
- message becomes `OK`
- random split is used

### `SR`

Input:
```text
LD
SR
```

Expected:
- message becomes `OK`
- deck order changes
- deck still has 52 cards

### `SD`

Input:
```text
LD
SD test-data/output_deck.txt
```

Expected:
- message becomes `OK`
- output file is created
- cards are saved one per line

## 2. Phase Switching

### `P`

Input:
```text
LD
P
```

Expected:
- phase becomes PLAY
- tableau layout matches Yukon
- visible/hidden pattern matches the assignment
- message becomes `OK`

### `Q`

Input:
```text
LD
P
Q
```

Expected:
- phase returns to STARTUP
- foundations clear
- startup deck is still there
- running `P` again restarts the same game

### `QQ`

Input:
```text
QQ
```

Expected:
- program exits cleanly

## 3. Legal Move Tests

### Tableau to foundation

Input:
```text
LD
P
C1->F1
```

Expected:
- first Ace moves to foundation
- message becomes `OK`
- source column size decreases

### Foundation back to tableau

Input:
```text
LD
P
C1->F1
F1->C2
```

Expected:
- move only succeeds if the tableau rule is satisfied
- otherwise message says move is not valid

### Tableau segment move

Use a shuffled deck until a visible segment move is possible.

Expected:
- selected visible card and all cards below it move together
- source column reveals a face-down card if one becomes exposed

## 4. Illegal Move Tests

Try these and make sure the program does not crash:

```text
C1->F1
P
F1->F2
C1 -> F1
HELLO
SI 0
SI 99
```

Expected:
- invalid commands get an error message
- commands in the wrong phase get a phase error
- malformed move syntax gets rejected

## 5. Foundation Rule Tests

### Any Ace can start a foundation

Expected:
- first Ace can go to any empty foundation slot

### Suit lock after first Ace

Expected:
- once a foundation starts with one suit, another suit cannot be added there

### Ascending order only

Expected:
- a card can only be placed if it is exactly one rank above the current foundation top

## 6. GUI Tests

### GUI startup

Expected:
- GUI opens without crashing
- bridge connects
- startup screen is visible

### GUI command buttons

Expected:
- `Load Default`, `Show Deck`, `Start Game`, and `Quit Game` update the state

### GUI manual command entry

Input:
```text
C1->F1
```

Expected:
- same backend result as the terminal version

### GUI feedback

Expected:
- invalid moves show clear message feedback
- winning shows a pop-up once

## 7. Before Submission

Run this minimum set:
- `LD`
- `SW`
- `SI`
- `SR`
- `SD`
- `P`
- one legal move
- one illegal move
- `Q`
- `QQ`
- open GUI and do at least one move
