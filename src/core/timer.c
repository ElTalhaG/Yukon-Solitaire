#include "timer.h"

#include <limits.h>
#include <stdio.h>
#include <time.h>

static int safe_seconds_between(time_t start_time, time_t end_time)
{
    double difference;

    /*
     * difftime is the portable C way to compare time_t values.
     * We clamp the result because the rest of the program only needs a simple
     * integer number of seconds for printing the timer.
     */
    difference = difftime(end_time, start_time);
    if (difference < 0.0) {
        return 0;
    }

    if (difference > (double) INT_MAX) {
        return INT_MAX;
    }

    return (int) difference;
}

void game_timer_reset(GameState *game_state)
{
    if (game_state == NULL) {
        return;
    }

    /*
     * Reset means "no current game time".
     * This is what we want when the program is back in STARTUP.
     */
    game_state->timer_is_running = false;
    game_state->timer_started_at = (time_t) 0;
    game_state->timer_elapsed_before_start = 0;
}

void game_timer_start(GameState *game_state)
{
    if (game_state == NULL) {
        return;
    }

    /*
     * A new P command should begin a fresh game timer, not continue from an old
     * game. So we wipe previous time and remember the current clock moment.
     */
    game_state->timer_elapsed_before_start = 0;
    game_state->timer_started_at = time(NULL);
    game_state->timer_is_running = true;
}

void game_timer_stop(GameState *game_state)
{
    if (game_state == NULL || !game_state->timer_is_running) {
        return;
    }

    /*
     * Store the visible elapsed time before stopping.
     * That way a UI can still print the final time after the clock pauses.
     */
    game_state->timer_elapsed_before_start = game_timer_elapsed_seconds(game_state);
    game_state->timer_started_at = (time_t) 0;
    game_state->timer_is_running = false;
}

int game_timer_elapsed_seconds(const GameState *game_state)
{
    int running_seconds;

    if (game_state == NULL) {
        return 0;
    }

    if (!game_state->timer_is_running) {
        return game_state->timer_elapsed_before_start;
    }

    running_seconds = safe_seconds_between(game_state->timer_started_at, time(NULL));
    if (INT_MAX - game_state->timer_elapsed_before_start < running_seconds) {
        return INT_MAX;
    }

    return game_state->timer_elapsed_before_start + running_seconds;
}

void game_timer_format(int seconds, char *buffer, size_t buffer_size)
{
    int hours;
    int minutes;
    int remaining_seconds;

    if (buffer == NULL || buffer_size == 0) {
        return;
    }

    if (seconds < 0) {
        seconds = 0;
    }

    hours = seconds / 3600;
    minutes = (seconds / 60) % 60;
    remaining_seconds = seconds % 60;

    if (hours > 0) {
        snprintf(buffer, buffer_size, "%d:%02d:%02d", hours, minutes, remaining_seconds);
    } else {
        snprintf(buffer, buffer_size, "%02d:%02d", minutes, remaining_seconds);
    }
}
