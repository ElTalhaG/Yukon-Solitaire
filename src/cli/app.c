#include "app.h"

#include <stdio.h>
#include <string.h>

#include "command_engine.h"
#include "game.h"
#include "parser.h"
#include "render.h"

/*
 * This is the actual terminal app loop now.
 * It is intentionally pretty direct: render, read, parse, execute, repeat.
 */

int run_cli_application(void)
{
    GameState game_state;
    ParsedCommand command;
    char input[MAX_COMMAND_LENGTH];

    game_state_init(&game_state);

    while (1) {
        render_game_state(stdout, &game_state);

        if (!read_command_line(stdin, input, sizeof(input))) {
            putchar('\n');
            break;
        }

        putchar('\n');
        game_remember_command(&game_state, input);

        if (!parse_command(input, &command)) {
            strncpy(game_state.message, "Invalid command.", MAX_MESSAGE_LENGTH - 1);
            game_state.message[MAX_MESSAGE_LENGTH - 1] = '\0';
            continue;
        }

        if (game_execute_command(&game_state, &command) == COMMAND_EXECUTION_QUIT) {
            break;
        }
    }

    game_state_destroy(&game_state);
    return 0;
}
