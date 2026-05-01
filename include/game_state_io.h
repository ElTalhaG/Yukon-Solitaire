#ifndef GAME_STATE_IO_H
#define GAME_STATE_IO_H

#include "game.h"

#include <stdbool.h>
#include <stddef.h>

/*
 * Optional extension: save/load the full current game state.
 * This is different from SD/LD, which only save or load the startup deck.
 */

bool game_state_save_to_file(const GameState *game_state, const char *filename,
                             char *message, size_t message_size);

bool game_state_load_from_file(GameState *game_state, const char *filename,
                               char *message, size_t message_size);

#endif
