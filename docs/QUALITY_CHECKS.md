# Quality Checks

This file is the short record of what we actually checked for code quality and memory safety.

## 1. Strict compiler builds

Both main C entry points were built with:

```text
-std=c11 -Wall -Wextra -Wpedantic
```

Checked targets:
- terminal CLI path
- GUI bridge path

At the time of this note, those builds were clean after fixing the small compile issues found during development.

## 2. AddressSanitizer pass

We also ran AddressSanitizer builds on:
- the CLI flow
- the GUI bridge flow

Checked command path:
```text
LD
P
C1->F1
Q
QQ
```

And for the bridge:
```text
LD
P
C1->F1
__QUIT__
```

Those runs completed without sanitizer errors.

## 3. Ownership notes

This is the practical ownership model in the code right now.

### Deck ownership

- `game_state.deck` owns the startup deck nodes.
- `deck_clear()` and `game_state_destroy()` can free them through `card_list_destroy()`.

### Tableau ownership

- each tableau column owns its own linked list while a game is in PLAY
- `game_start_play()` creates fresh dealt card copies for tableau use
- `game_quit_play()` and `game_state_destroy()` clear tableau-owned nodes

### Foundation ownership

- each foundation owns its own linked list
- moving a card to a foundation transfers ownership into that foundation list
- foundation nodes are also released by `game_state_destroy()`

### Why dealing copies cards

This is important enough to say plainly:

- the startup deck must stay intact
- `Q` has to go back to STARTUP without losing the original deck
- `P` has to restart from that saved deck

Because of that, `game_start_play()` copies cards into the tableau instead of consuming the startup deck list.

## 4. Things still worth checking later

Even though the current checks are good, before final submission it is still smart to:
- rerun strict builds after every larger feature
- rerun AddressSanitizer after optional extensions if we add them
- do one last end-to-end CLI test pass
- do one last GUI bridge + Tkinter test pass
