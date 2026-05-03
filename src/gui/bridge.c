#include <stdio.h>
#include <string.h>

#include "card.h"
#include "command_engine.h"
#include "game.h"
#include "parser.h"
#include "timer.h"

/*
 * This file is the little adapter between the shared C backend and the Python GUI.
 * The GUI sends normal command strings, and this bridge answers with a full state dump.
 * So yeah, the GUI gets a simple protocol and the C code keeps the real rules.
 */

static const char *phase_name(GamePhase phase)
{
    if (phase == GAME_PHASE_PLAY) {
        return "PLAY";
    }

    return "STARTUP";
}

static void format_card_code_or_dash(const Card *card, char output[3])
{
    if (card == NULL || !card_format_face_up(card, output)) {
        output[0] = '-';
        output[1] = '\0';
    }
}

static const Card *foundation_top_card(const FoundationPile *foundation)
{
    const Card *current;

    if (foundation == NULL || foundation->top == NULL) {
        return NULL;
    }

    current = foundation->top;
    while (current->next != NULL) {
        current = current->next;
    }

    return current;
}

static void dump_state(FILE *stream, const GameState *game_state)
{
    const Card *current;
    const Card *top_card;
    char card_code[3];
    int index;
    int card_index;

    fprintf(stream, "BEGIN_STATE\n");
    fprintf(stream, "PHASE\t%s\n", phase_name(game_state->phase));
    fprintf(stream, "SHOW_ALL\t%d\n", game_state->startup_show_all ? 1 : 0);
    fprintf(stream, "LAST_COMMAND\t%s\n", game_state->last_command);
    fprintf(stream, "MESSAGE\t%s\n", game_state->message);
    fprintf(stream, "ELAPSED_SECONDS\t%d\n", game_timer_elapsed_seconds(game_state));
    fprintf(stream, "BEST_SECONDS\t%d\n", game_state->best_time_seconds);
    fprintf(stream, "DECK_SIZE\t%d\n", game_state->deck.size);

    current = game_state->deck.top;
    card_index = 0;
    while (current != NULL) {
        format_card_code_or_dash(current, card_code);
        fprintf(stream, "DECK_CARD\t%d\t%s\t%d\n", card_index, card_code, current->is_face_up ? 1 : 0);
        current = current->next;
        card_index++;
    }

    for (index = 0; index < FOUNDATION_COUNT; index++) {
        top_card = foundation_top_card(&game_state->foundations[index]);
        format_card_code_or_dash(top_card, card_code);
        fprintf(stream, "FOUNDATION\t%d\t%d\t%d\t%d\t%s\n",
                index,
                game_state->foundations[index].size,
                game_state->foundations[index].is_suit_assigned,
                game_state->foundations[index].is_suit_assigned ? (int) game_state->foundations[index].suit : -1,
                card_code);
    }

    for (index = 0; index < TABLEAU_COLUMNS; index++) {
        fprintf(stream, "TABLEAU_SIZE\t%d\t%d\n", index, game_state->tableau[index].size);

        current = game_state->tableau[index].top;
        card_index = 0;
        while (current != NULL) {
            format_card_code_or_dash(current, card_code);
            fprintf(stream, "TABLEAU_CARD\t%d\t%d\t%s\t%d\n",
                    index, card_index, card_code, current->is_face_up ? 1 : 0);
            current = current->next;
            card_index++;
        }
    }

    fprintf(stream, "END_STATE\n");
    fflush(stream);
}

int main(void)
{
    GameState game_state;
    ParsedCommand command;
    char input[MAX_COMMAND_LENGTH];

    game_state_init(&game_state);
    dump_state(stdout, &game_state);

    while (read_command_line(stdin, input, sizeof(input))) {
        if (strcmp(input, "__STATE__") == 0) {
            dump_state(stdout, &game_state);
            continue;
        }

        if (strcmp(input, "__QUIT__") == 0) {
            break;
        }

        game_remember_command(&game_state, input);

        if (!parse_command(input, &command)) {
            strncpy(game_state.message, "Invalid command.", MAX_MESSAGE_LENGTH - 1);
            game_state.message[MAX_MESSAGE_LENGTH - 1] = '\0';
            dump_state(stdout, &game_state);
            continue;
        }

        if (game_execute_command(&game_state, &command) == COMMAND_EXECUTION_QUIT) {
            break;
        }

        dump_state(stdout, &game_state);
    }

    game_state_destroy(&game_state);
    return 0;
}
