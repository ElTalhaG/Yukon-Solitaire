#ifndef BEST_TIME_H
#define BEST_TIME_H

#include "game.h"

/*
 * Optional extension: remember the fastest completed game.
 * We keep this separate from the normal save-game file because this is more
 * like a small high-score value shared across program runs.
 */

/* Load best time from disk. If no file exists yet, best time stays as 0. */
void best_time_load(GameState *game_state);

/* Save the current best time to disk. */
void best_time_save(const GameState *game_state);

/* Update the best time if elapsed_seconds is the first or fastest win. */
int best_time_record_if_faster(GameState *game_state, int elapsed_seconds);

#endif
