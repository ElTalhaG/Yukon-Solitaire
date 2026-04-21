#ifndef GAME_SETUP_H
#define GAME_SETUP_H

#include <stdbool.h>
#include <stddef.h>

#include "game.h"

/*
 * These helpers implement the backend state transition between STARTUP and PLAY.
 * The command parser will call them later for:
 * - P: start a game from the current deck
 * - Q: leave the current game and return to STARTUP
 */

/* Deal the current deck into the Yukon tableau layout and enter PLAY mode. */
bool game_start_play(GameState *game_state, char *error_message, size_t error_size);

/* Leave PLAY mode, clear the tableau/foundations, and keep the saved deck. */
void game_quit_play(GameState *game_state);

#endif
