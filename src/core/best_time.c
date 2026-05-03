#include "best_time.h"

#include <stdio.h>

/*
 * Simple on purpose: the file contains only one integer, the best time in
 * seconds. If the file is missing or broken, we just behave as if no best time
 * exists yet. That is nicer than making the game fail because of a high score.
 */
static const char *BEST_TIME_FILE = "yukon_best_time.txt";

void best_time_load(GameState *game_state)
{
    FILE *file;
    int saved_seconds;

    if (game_state == NULL) {
        return;
    }

    game_state->best_time_seconds = 0;

    file = fopen(BEST_TIME_FILE, "r");
    if (file == NULL) {
        return;
    }

    if (fscanf(file, "%d", &saved_seconds) == 1 && saved_seconds > 0) {
        game_state->best_time_seconds = saved_seconds;
    }

    fclose(file);
}

void best_time_save(const GameState *game_state)
{
    FILE *file;

    if (game_state == NULL || game_state->best_time_seconds <= 0) {
        return;
    }

    file = fopen(BEST_TIME_FILE, "w");
    if (file == NULL) {
        return;
    }

    fprintf(file, "%d\n", game_state->best_time_seconds);
    fclose(file);
}

int best_time_record_if_faster(GameState *game_state, int elapsed_seconds)
{
    if (game_state == NULL || elapsed_seconds <= 0) {
        return 0;
    }

    /*
     * First completed game always becomes the best.
     * Later wins only replace it if the time is actually faster.
     */
    if (game_state->best_time_seconds == 0 ||
        elapsed_seconds < game_state->best_time_seconds) {
        game_state->best_time_seconds = elapsed_seconds;
        best_time_save(game_state);
        return 1;
    }

    return 0;
}
