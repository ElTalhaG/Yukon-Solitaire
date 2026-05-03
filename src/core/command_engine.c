#include "command_engine.h"

#include <stdio.h>
#include <string.h>

#include "best_time.h"
#include "deck_io.h"
#include "game_setup.h"
#include "game_state_io.h"
#include "move.h"
#include "shuffle.h"
#include "timer.h"

/*
 * This file is the "what happens when a command is valid?" layer.
 * The parser already cleaned up the text, so this part can stay focused on
 * phase rules, deck actions, play actions, and status messages.
 */

static void set_message(GameState *game_state, const char *message)
{
    if (game_state == NULL || message == NULL) {
        return;
    }

    strncpy(game_state->message, message, MAX_MESSAGE_LENGTH - 1);
    game_state->message[MAX_MESSAGE_LENGTH - 1] = '\0';
}

static int is_startup_only_command(CommandType type)
{
    return type == COMMAND_TYPE_LD ||
           type == COMMAND_TYPE_SW ||
           type == COMMAND_TYPE_SI ||
           type == COMMAND_TYPE_SR ||
           type == COMMAND_TYPE_SD ||
           type == COMMAND_TYPE_P;
}

static int is_any_phase_command(CommandType type)
{
    return type == COMMAND_TYPE_SAVE_GAME ||
           type == COMMAND_TYPE_LOAD_GAME;
}

static int is_play_only_command(CommandType type)
{
    return type == COMMAND_TYPE_Q ||
           type == COMMAND_TYPE_MOVE;
}

static int execute_startup_command(GameState *game_state, const ParsedCommand *command)
{
    if (command->type == COMMAND_TYPE_LD) {
        if (command->has_argument) {
            if (!deck_load_from_file(&game_state->deck, command->argument,
                                     game_state->message, sizeof(game_state->message))) {
                return 1;
            }
        } else {
            if (!deck_load_default(&game_state->deck,
                                   game_state->message, sizeof(game_state->message))) {
                return 1;
            }
        }

        game_state->startup_show_all = false;
        return 1;
    }

    if (command->type == COMMAND_TYPE_SW) {
        if (game_state->deck.size != DECK_CARD_COUNT) {
            set_message(game_state, "No deck is loaded.");
            return 1;
        }

        game_state->startup_show_all = true;
        set_message(game_state, "OK");
        return 1;
    }

    if (command->type == COMMAND_TYPE_SI) {
        if (command->has_split) {
            if (!deck_shuffle_interleave(&game_state->deck, command->split,
                                         game_state->message, sizeof(game_state->message))) {
                return 1;
            }
        } else {
            if (!deck_shuffle_interleave_random_split(&game_state->deck,
                                                      game_state->message, sizeof(game_state->message))) {
                return 1;
            }
        }

        game_state->startup_show_all = false;
        return 1;
    }

    if (command->type == COMMAND_TYPE_SR) {
        if (!deck_shuffle_random_insert(&game_state->deck,
                                        game_state->message, sizeof(game_state->message))) {
            return 1;
        }

        game_state->startup_show_all = false;
        return 1;
    }

    if (command->type == COMMAND_TYPE_SD) {
        if (!deck_save_to_file(&game_state->deck,
                               command->has_argument ? command->argument : NULL,
                               game_state->message, sizeof(game_state->message))) {
            return 1;
        }

        return 1;
    }

    if (command->type == COMMAND_TYPE_P) {
        game_state->startup_show_all = false;
        game_start_play(game_state, game_state->message, sizeof(game_state->message));
        return 1;
    }

    return 0;
}

static int execute_play_command(GameState *game_state, const ParsedCommand *command)
{
    int elapsed_seconds;
    int is_new_best;
    char elapsed_text[16];
    char best_text[16];

    if (command->type == COMMAND_TYPE_Q) {
        game_quit_play(game_state);
        set_message(game_state, "OK");
        return 1;
    }

    if (command->type == COMMAND_TYPE_MOVE) {
        if (game_apply_move(game_state, &command->from, &command->to) &&
            game_is_won(game_state)) {
            /*
             * The exact moment the game is won is the finish line.
             * We stop the timer before comparing best times so the result does
             * not keep changing while the player reads the message.
             */
            elapsed_seconds = game_timer_elapsed_seconds(game_state);
            game_timer_stop(game_state);
            is_new_best = best_time_record_if_faster(game_state, elapsed_seconds);

            game_timer_format(elapsed_seconds, elapsed_text, sizeof(elapsed_text));
            game_timer_format(game_state->best_time_seconds, best_text, sizeof(best_text));

            if (is_new_best) {
                snprintf(game_state->message, sizeof(game_state->message),
                         "OK - Game won. New best time: %s.", elapsed_text);
            } else {
                snprintf(game_state->message, sizeof(game_state->message),
                         "OK - Game won. Time: %s. Best: %s.", elapsed_text, best_text);
            }
        }

        return 1;
    }

    return 0;
}

void game_remember_command(GameState *game_state, const char *command_text)
{
    if (game_state == NULL || command_text == NULL) {
        return;
    }

    strncpy(game_state->last_command, command_text, MAX_COMMAND_LENGTH - 1);
    game_state->last_command[MAX_COMMAND_LENGTH - 1] = '\0';
}

CommandExecutionResult game_execute_command(GameState *game_state, const ParsedCommand *command)
{
    if (game_state == NULL || command == NULL) {
        return COMMAND_EXECUTION_CONTINUE;
    }

    if (game_state->phase == GAME_PHASE_STARTUP && is_play_only_command(command->type)) {
        set_message(game_state, "Command not available in the STARTUP phase.");
        return COMMAND_EXECUTION_CONTINUE;
    }

    if (game_state->phase == GAME_PHASE_PLAY && is_startup_only_command(command->type)) {
        set_message(game_state, "Command not available in the PLAY phase.");
        return COMMAND_EXECUTION_CONTINUE;
    }

    if (command->type == COMMAND_TYPE_QQ) {
        return COMMAND_EXECUTION_QUIT;
    }

    if (is_any_phase_command(command->type)) {
        if (command->type == COMMAND_TYPE_SAVE_GAME) {
            game_state_save_to_file(game_state,
                                    command->has_argument ? command->argument : NULL,
                                    game_state->message, sizeof(game_state->message));
        } else {
            /*
             * Loading replaces the whole game state. After that we write the
             * current L command back into last_command so the UI still shows
             * what the player just did, not whatever was stored in the file.
             */
            if (game_state_load_from_file(game_state,
                                          command->has_argument ? command->argument : NULL,
                                          game_state->message, sizeof(game_state->message))) {
                game_remember_command(game_state, command->raw_text);
            }
        }

        return COMMAND_EXECUTION_CONTINUE;
    }

    if (game_state->phase == GAME_PHASE_STARTUP) {
        execute_startup_command(game_state, command);
    } else {
        execute_play_command(game_state, command);
    }

    return COMMAND_EXECUTION_CONTINUE;
}
