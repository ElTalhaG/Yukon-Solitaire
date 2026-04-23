# Final Validation Notes

This file is the "last serious check before delivery" note.

The point here is simple:
- rerun the important flows on the current code
- record what actually passed
- avoid saying "it should work" without evidence

## 1. Strict final builds

We rebuilt both C entry points with:

```text
-std=c11 -Wall -Wextra -Wpedantic
```

Checked targets:
- CLI executable path
- GUI bridge executable path

We also syntax-checked the Tkinter GUI file with:

```text
python3 -m py_compile src/gui/tk_yukon.py
```

That passed too.

## 2. Final CLI flow checks

We reran a practical end-to-end CLI path with:

```text
LD
SW
SI 26
SR
P
C1->F1
Q
QQ
```

What mattered in this pass:
- load worked
- show-deck worked
- both shuffle commands worked
- play mode started
- a move command was accepted and handled cleanly
- quitting PLAY returned to STARTUP
- quitting the program exited cleanly

We also reran a default-deck legal move path:

```text
LD
P
C1->F1
```

That confirmed the first Ace can move to a foundation in the expected way.

## 3. Final invalid-input checks

We reran some wrong-phase, malformed, and file-error cases:

```text
C1->F1
P
LD
P
C1 -> F1
HELLO
Q
QQ
```

And also:

```text
LD
SI 0
SI 99
QQ
```

And:

```text
LD test-data/invalid_bad_code.txt
QQ
```

Things confirmed here:
- wrong-phase commands return a phase error
- trying to start PLAY without a deck gives a sensible message
- malformed move syntax gets rejected
- random unknown commands get rejected
- invalid split ranges are handled
- malformed deck files report a useful error

## 4. Save-file check

We also reran:

```text
LD
SW
SI 26
SR
SD /tmp/final_saved_deck.txt
QQ
```

The saved file was created and contained `52` lines, which matches a full deck save.

## 5. GUI/backend consistency check

For the GUI side, we validated the backend path through the bridge process.

Checked bridge flow:

```text
LD
P
C1->F1
__QUIT__
```

Why this matters:
- the bridge uses the shared parser and shared command engine
- the same default-deck Ace move succeeded there too
- foundation state updated as expected

So even though the Tkinter file is the visual layer, the actual move logic is still shared with the terminal version.

## 6. Memory cleanup check

We reran AddressSanitizer builds on:
- the CLI path
- the GUI bridge path

Those final sanitizer runs completed without reported memory errors in the checked flows.

## 7. Practical conclusion

At the time of this note:
- the game builds cleanly
- the core terminal flow works
- the bridge-based GUI backend flow works
- invalid input is handled without crashing in the tested cases
- cleanup on exit looks good in sanitizer runs

That is enough to treat the current codebase as having passed the final validation phase for the implemented scope.
