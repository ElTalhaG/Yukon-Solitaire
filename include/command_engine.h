#ifndef COMMAND_ENGINE_H
#define COMMAND_ENGINE_H

#include "game.h"
#include "parser.h"

/*
 * This is the shared command-execution layer.
 * The CLI and the future GUI can both hand parsed commands to this module and
 * get the same backend behavior out of it.
 */

typedef enum {
    COMMAND_EXECUTION_CONTINUE,
    COMMAND_EXECUTION_QUIT
} CommandExecutionResult;

/* Store the raw command text in the game state for later rendering/debugging. */
void game_remember_command(GameState *game_state, const char *command_text);

/* Validate phase availability, run the command, and update the status message. */
CommandExecutionResult game_execute_command(GameState *game_state, const ParsedCommand *command);

#endif
