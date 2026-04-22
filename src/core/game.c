#include "game.h"

#include <stddef.h>

void deck_init(Deck *deck)
{
    /* An empty deck has no first node and contains zero cards. */
    deck->top = NULL;
    deck->size = 0;
}

void tableau_column_init(TableauColumn *column)
{
    /* Each column starts empty until cards are dealt into the tableau. */
    column->top = NULL;
    column->size = 0;
}

void foundation_pile_init(FoundationPile *foundation)
{
    /* Foundations begin empty and are not tied to any suit yet. */
    foundation->top = NULL;
    foundation->size = 0;
    foundation->is_suit_assigned = 0;

    /*
     * The suit value is only meaningful after is_suit_assigned becomes true.
     * We still set a predictable default so the whole struct is well defined.
     */
    foundation->suit = CARD_SUIT_CLUBS;
}

void game_state_init(GameState *game_state)
{
    int index;

    /* Reset the deck used during startup commands such as LD, SI, SR, and SD. */
    deck_init(&game_state->deck);

    /* Start with seven empty tableau columns. */
    for (index = 0; index < TABLEAU_COLUMNS; index++) {
        tableau_column_init(&game_state->tableau[index]);
    }

    /* Start with four empty foundation piles. */
    for (index = 0; index < FOUNDATION_COUNT; index++) {
        foundation_pile_init(&game_state->foundations[index]);
    }

    /* A fresh program always begins in the STARTUP phase. */
    game_state->phase = GAME_PHASE_STARTUP;
    game_state->startup_show_all = false;

    /* No command has been entered yet, so both UI strings start empty. */
    game_state->last_command[0] = '\0';
    game_state->message[0] = '\0';
}

void game_state_reset_play_area(GameState *game_state)
{
    int index;

    /*
     * This helper only resets the actual play area.
     * It is useful when leaving a game or preparing to deal a new one
     * while keeping the current deck data available in memory.
     */
    for (index = 0; index < TABLEAU_COLUMNS; index++) {
        tableau_column_init(&game_state->tableau[index]);
    }

    for (index = 0; index < FOUNDATION_COUNT; index++) {
        foundation_pile_init(&game_state->foundations[index]);
    }

    /* After clearing the play area, the program returns to STARTUP mode. */
    game_state->phase = GAME_PHASE_STARTUP;
    game_state->startup_show_all = false;
}
