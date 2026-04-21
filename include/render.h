#ifndef RENDER_H
#define RENDER_H

#include <stdio.h>

#include "game.h"

/*
 * Terminal rendering belongs to the CLI layer rather than the backend rules.
 * These helpers print the current game state in the text format described by
 * the course project document.
 */

/* Print the current tableau, foundations, and bottom status lines. */
void render_game_state(FILE *stream, const GameState *game_state);

#endif
