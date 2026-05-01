#ifndef TIMER_H
#define TIMER_H

#include "game.h"

#include <stddef.h>

/* Reset the game timer back to zero and mark it as not running. */
void game_timer_reset(GameState *game_state);

/* Start the timer from zero when a new Yukon game begins. */
void game_timer_start(GameState *game_state);

/* Stop the timer but keep the current elapsed value in the game state. */
void game_timer_stop(GameState *game_state);

/* Get the current elapsed time in seconds, whether the timer is running or not. */
int game_timer_elapsed_seconds(const GameState *game_state);

/* Format seconds as MM:SS, or H:MM:SS if the game somehow goes very long. */
void game_timer_format(int seconds, char *buffer, size_t buffer_size);

#endif
