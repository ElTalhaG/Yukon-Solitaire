#include "game_setup.h"

#include <stdio.h>

#include "list.h"
#include "timer.h"

/*
 * Yukon starts with fixed column sizes and a fixed number of hidden cards.
 * The arrays are indexed by column number 0..6.
 */
static const int COLUMN_CARD_COUNTS[TABLEAU_COLUMNS] = {1, 6, 7, 8, 9, 10, 11};
static const int COLUMN_HIDDEN_COUNTS[TABLEAU_COLUMNS] = {0, 1, 2, 3, 4, 5, 6};

static void set_error_message(char *error_message, size_t error_size, const char *message)
{
    if (error_message == NULL || error_size == 0) {
        return;
    }

    snprintf(error_message, error_size, "%s", message);
}

static bool game_can_start(const GameState *game_state, char *error_message, size_t error_size)
{
    if (game_state == NULL) {
        set_error_message(error_message, error_size, "Game state pointer is NULL.");
        return false;
    }

    /*
     * STARTUP keeps the canonical deck that P should deal from.
     * If it is missing or incomplete, the game cannot begin yet.
     */
    if (game_state->deck.top == NULL || game_state->deck.size != DECK_CARD_COUNT) {
        set_error_message(error_message, error_size, "No deck is loaded.");
        return false;
    }

    return true;
}

static bool append_dealt_card(TableauColumn *column, const Card *source_card, bool is_face_up,
                              char *error_message, size_t error_size)
{
    Card *dealt_card;

    /*
     * The startup deck must stay intact so Q can return to STARTUP and P can
     * restart the same game later. Because of that, dealing creates copies of
     * the deck cards instead of moving the original deck nodes.
     */
    dealt_card = card_create(source_card->rank, source_card->suit, is_face_up);
    if (dealt_card == NULL) {
        set_error_message(error_message, error_size, "Memory allocation failed while dealing cards.");
        return false;
    }

    tableau_append_card(column, dealt_card);
    return true;
}

static bool deal_row_wise(GameState *game_state, char *error_message, size_t error_size)
{
    const Card *current;
    int row;
    int column_index;

    current = game_state->deck.top;

    /*
     * The assignment says cards must be placed row by row from left to right.
     * We therefore iterate by row first and only append to columns that still
     * need more cards at that row depth.
     */
    for (row = 0; current != NULL; row++) {
        for (column_index = 0; column_index < TABLEAU_COLUMNS && current != NULL; column_index++) {
            if (row >= COLUMN_CARD_COUNTS[column_index]) {
                continue;
            }

            if (!append_dealt_card(&game_state->tableau[column_index], current,
                                   row >= COLUMN_HIDDEN_COUNTS[column_index],
                                   error_message, error_size)) {
                return false;
            }

            current = current->next;
        }
    }

    /*
     * After dealing exactly 52 cards this pointer should be NULL.
     * If it is not, the stored deck size/layout and the traversal disagreed.
     */
    if (current != NULL) {
        set_error_message(error_message, error_size, "Deck still contained undealt cards after setup.");
        return false;
    }

    return true;
}

bool game_start_play(GameState *game_state, char *error_message, size_t error_size)
{
    if (!game_can_start(game_state, error_message, error_size)) {
        return false;
    }

    /*
     * Starting a new game always clears any previous tableau/foundation state
     * first, while intentionally preserving the startup deck.
     */
    game_state_reset_play_area(game_state);

    if (!deal_row_wise(game_state, error_message, error_size)) {
        game_state_reset_play_area(game_state);
        return false;
    }

    game_state->phase = GAME_PHASE_PLAY;
    game_timer_start(game_state);
    set_error_message(error_message, error_size, "OK");
    return true;
}

void game_quit_play(GameState *game_state)
{
    if (game_state == NULL) {
        return;
    }

    /*
     * Quitting the current game clears only the live play area.
     * The saved startup deck remains untouched so the same game can be started
     * again immediately with another P command.
     */
    game_state_reset_play_area(game_state);
}
