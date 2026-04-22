#ifndef MOVE_H
#define MOVE_H

#include <stdbool.h>

#include "game.h"
#include "parser.h"

/*
 * This module is where parsed move commands finally become actual game-state
 * changes. The parser figures out what the user typed; this part decides
 * whether the move is legal and, if yes, rewires the linked lists.
 */

/* Validate and execute one move, then update game_state->message. */
bool game_apply_move(GameState *game_state, const MoveReference *from, const MoveReference *to);

/* True when all 52 cards have reached the foundations. */
bool game_is_won(const GameState *game_state);

#endif
