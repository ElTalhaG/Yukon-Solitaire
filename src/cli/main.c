#include <stdio.h>

#include "deck_io.h"
#include "game.h"

int main(void)
{
    GameState game_state;

    /*
     * This temporary CLI entry point exists so CLion can configure, build,
     * and run the project immediately. The real terminal command loop will
     * replace this once the parser and renderer phases are implemented.
     */
    game_state_init(&game_state);

    printf("Yukon Solitaire backend scaffold is ready.\n");
    printf("Current phase: STARTUP\n");

    game_state_destroy(&game_state);
    return 0;
}
