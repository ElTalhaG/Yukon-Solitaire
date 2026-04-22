#include "move.h"

#include <stddef.h>
#include <string.h>

#include "list.h"

/*
 * This file is basically the card-rules brain of the project.
 * Most bugs in solitaire games show up here, so the comments are a bit more
 * chatty on purpose. Future us is definitely going to thank us for that.
 */

typedef enum {
    RESOLVED_SOURCE_NONE,
    RESOLVED_SOURCE_TABLEAU,
    RESOLVED_SOURCE_FOUNDATION
} ResolvedSourceType;

typedef struct {
    ResolvedSourceType source_type;
    int source_index;
    int destination_index;
    Card *source_prev;
    Card *source_start;
    int moved_card_count;
} ResolvedMove;

static void set_message(GameState *game_state, const char *message)
{
    if (game_state == NULL || message == NULL) {
        return;
    }

    strncpy(game_state->message, message, MAX_MESSAGE_LENGTH - 1);
    game_state->message[MAX_MESSAGE_LENGTH - 1] = '\0';
}

static Card *tableau_bottom_card(TableauColumn *column)
{
    Card *current;

    if (column == NULL || column->top == NULL) {
        return NULL;
    }

    current = column->top;
    while (current->next != NULL) {
        current = current->next;
    }

    return current;
}

static const Card *foundation_top_card_const(const FoundationPile *foundation)
{
    const Card *current;

    if (foundation == NULL || foundation->top == NULL) {
        return NULL;
    }

    /*
     * Foundations grow by appending cards to the end, so the visible top card
     * is the tail node, not the head node.
     */
    current = foundation->top;
    while (current->next != NULL) {
        current = current->next;
    }

    return current;
}

static bool find_tableau_card(TableauColumn *column, CardRank rank, CardSuit suit,
                              Card **previous_out, Card **card_out)
{
    Card *previous;
    Card *current;

    if (column == NULL || previous_out == NULL || card_out == NULL) {
        return false;
    }

    previous = NULL;
    current = column->top;
    while (current != NULL) {
        if (current->rank == rank && current->suit == suit) {
            *previous_out = previous;
            *card_out = current;
            return true;
        }

        previous = current;
        current = current->next;
    }

    return false;
}

static bool column_target_accepts_card(const TableauColumn *column, const Card *moving_card)
{
    const Card *bottom_card;

    if (column == NULL || moving_card == NULL) {
        return false;
    }

    bottom_card = tableau_bottom_card((TableauColumn *) column);
    if (bottom_card == NULL) {
        return moving_card->rank == CARD_RANK_KING;
    }

    /*
     * Yukon here uses "different suit", not alternating color.
     * So we only care about rank difference and suit equality.
     */
    return card_rank_difference(bottom_card->rank, moving_card->rank) == 1 &&
           !card_same_suit(bottom_card, moving_card);
}

static bool foundation_accepts_card(FoundationPile *foundation, const Card *moving_card)
{
    const Card *top_card;

    if (foundation == NULL || moving_card == NULL) {
        return false;
    }

    top_card = foundation_top_card_const(foundation);
    if (top_card == NULL) {
        return moving_card->rank == CARD_RANK_ACE;
    }

    if (!foundation->is_suit_assigned) {
        return false;
    }

    return moving_card->suit == foundation->suit &&
           card_rank_difference(moving_card->rank, top_card->rank) == 1;
}

static bool resolve_tableau_source(const GameState *game_state, const MoveReference *from,
                                   ResolvedMove *resolved)
{
    TableauColumn *column;
    Card *previous;
    Card *selected;

    if (game_state == NULL || from == NULL || resolved == NULL) {
        return false;
    }

    if (from->pile_index < 0 || from->pile_index >= TABLEAU_COLUMNS) {
        return false;
    }

    column = (TableauColumn *) &game_state->tableau[from->pile_index];
    if (column->top == NULL) {
        return false;
    }

    if (from->type == MOVE_REF_TABLEAU_BOTTOM) {
        previous = NULL;
        selected = column->top;
        while (selected->next != NULL) {
            previous = selected;
            selected = selected->next;
        }
    } else if (from->type == MOVE_REF_TABLEAU_CARD) {
        if (!find_tableau_card(column, from->rank, from->suit, &previous, &selected)) {
            return false;
        }
    } else {
        return false;
    }

    /*
     * If the chosen card is still face down, the move should die right here.
     * No point continuing deeper and pretending that could ever be legal.
     */
    if (!selected->is_face_up) {
        return false;
    }

    resolved->source_type = RESOLVED_SOURCE_TABLEAU;
    resolved->source_index = from->pile_index;
    resolved->source_prev = previous;
    resolved->source_start = selected;
    resolved->moved_card_count = card_list_count(selected);
    return true;
}

static bool resolve_foundation_source(const GameState *game_state, const MoveReference *from,
                                      ResolvedMove *resolved)
{
    FoundationPile *foundation;
    Card *previous;
    Card *current;

    if (game_state == NULL || from == NULL || resolved == NULL) {
        return false;
    }

    if (from->type != MOVE_REF_FOUNDATION ||
        from->pile_index < 0 || from->pile_index >= FOUNDATION_COUNT) {
        return false;
    }

    foundation = (FoundationPile *) &game_state->foundations[from->pile_index];
    if (foundation->top == NULL) {
        return false;
    }

    previous = NULL;
    current = foundation->top;
    while (current->next != NULL) {
        previous = current;
        current = current->next;
    }

    resolved->source_type = RESOLVED_SOURCE_FOUNDATION;
    resolved->source_index = from->pile_index;
    resolved->source_prev = previous;
    resolved->source_start = current;
    resolved->moved_card_count = 1;
    return true;
}

static bool resolve_move(const GameState *game_state, const MoveReference *from,
                         const MoveReference *to, ResolvedMove *resolved)
{
    if (game_state == NULL || from == NULL || to == NULL || resolved == NULL) {
        return false;
    }

    memset(resolved, 0, sizeof(*resolved));
    resolved->source_index = -1;
    resolved->destination_index = to->pile_index;

    if (from->type == MOVE_REF_FOUNDATION) {
        return resolve_foundation_source(game_state, from, resolved);
    }

    return resolve_tableau_source(game_state, from, resolved);
}

static bool validate_move_path(const GameState *game_state, const MoveReference *to,
                               const ResolvedMove *resolved)
{
    const Card *moving_card;
    const TableauColumn *destination_column;
    FoundationPile *destination_foundation;

    if (game_state == NULL || to == NULL || resolved == NULL || resolved->source_start == NULL) {
        return false;
    }

    moving_card = resolved->source_start;

    if (resolved->source_type == RESOLVED_SOURCE_TABLEAU && to->type == MOVE_REF_TABLEAU_BOTTOM) {
        if (resolved->source_index == to->pile_index) {
            return false;
        }

        destination_column = &game_state->tableau[to->pile_index];
        return column_target_accepts_card(destination_column, moving_card);
    }

    if (resolved->source_type == RESOLVED_SOURCE_TABLEAU && to->type == MOVE_REF_FOUNDATION) {
        if (resolved->moved_card_count != 1) {
            return false;
        }

        /*
         * Only the real bottom card of a tableau column may move to a foundation.
         * If source_start has a next node, then there were still cards below it.
         */
        if (resolved->source_start->next != NULL) {
            return false;
        }

        destination_foundation = &((GameState *) game_state)->foundations[to->pile_index];
        return foundation_accepts_card(destination_foundation, moving_card);
    }

    if (resolved->source_type == RESOLVED_SOURCE_FOUNDATION && to->type == MOVE_REF_TABLEAU_BOTTOM) {
        destination_column = &game_state->tableau[to->pile_index];
        return column_target_accepts_card(destination_column, moving_card);
    }

    return false;
}

static void detach_tableau_segment(GameState *game_state, const ResolvedMove *resolved, Card **segment_head)
{
    TableauColumn *source_column;

    source_column = &game_state->tableau[resolved->source_index];
    *segment_head = resolved->source_start;

    if (resolved->source_prev == NULL) {
        source_column->top = NULL;
    } else {
        resolved->source_prev->next = NULL;
    }

    source_column->size -= resolved->moved_card_count;
}

static Card *detach_foundation_top(GameState *game_state, const ResolvedMove *resolved)
{
    FoundationPile *foundation;
    Card *card;

    foundation = &game_state->foundations[resolved->source_index];
    card = resolved->source_start;

    if (resolved->source_prev == NULL) {
        foundation->top = NULL;
    } else {
        resolved->source_prev->next = NULL;
    }

    foundation->size--;
    card->next = NULL;
    return card;
}

static void append_segment_to_tableau(GameState *game_state, int destination_index, Card *segment_head, int moved_count)
{
    TableauColumn *destination_column;

    destination_column = &game_state->tableau[destination_index];
    card_list_splice_end(&destination_column->top, &segment_head);
    destination_column->size += moved_count;
}

static void append_card_to_foundation(GameState *game_state, int destination_index, Card *card)
{
    FoundationPile *foundation;

    foundation = &game_state->foundations[destination_index];
    card_list_append(&foundation->top, card);
    foundation->size++;

    /*
     * The first Ace permanently chooses the suit for that foundation pile.
     * Even if the Ace moves away later, the pile still stays locked to that suit.
     */
    if (!foundation->is_suit_assigned) {
        foundation->is_suit_assigned = 1;
        foundation->suit = card->suit;
    }
}

static void reveal_new_tableau_bottom(GameState *game_state, int source_index)
{
    Card *new_bottom;

    new_bottom = tableau_bottom_card(&game_state->tableau[source_index]);
    if (new_bottom != NULL && !new_bottom->is_face_up) {
        new_bottom->is_face_up = true;
    }
}

bool game_apply_move(GameState *game_state, const MoveReference *from, const MoveReference *to)
{
    ResolvedMove resolved;
    Card *segment_head;
    Card *card;

    if (game_state == NULL || from == NULL || to == NULL) {
        return false;
    }

    if (game_state->phase != GAME_PHASE_PLAY) {
        set_message(game_state, "Move commands are only available in the PLAY phase.");
        return false;
    }

    if (!resolve_move(game_state, from, to, &resolved)) {
        set_message(game_state, "Move is not valid.");
        return false;
    }

    if (!validate_move_path(game_state, to, &resolved)) {
        set_message(game_state, "Move is not valid.");
        return false;
    }

    /*
     * Once we reach this point, the move is fully legal and we can actually
     * start cutting and reconnecting linked-list pieces.
     */
    if (resolved.source_type == RESOLVED_SOURCE_TABLEAU && to->type == MOVE_REF_TABLEAU_BOTTOM) {
        detach_tableau_segment(game_state, &resolved, &segment_head);
        append_segment_to_tableau(game_state, to->pile_index, segment_head, resolved.moved_card_count);
        reveal_new_tableau_bottom(game_state, resolved.source_index);
    } else if (resolved.source_type == RESOLVED_SOURCE_TABLEAU && to->type == MOVE_REF_FOUNDATION) {
        detach_tableau_segment(game_state, &resolved, &segment_head);
        append_card_to_foundation(game_state, to->pile_index, segment_head);
        reveal_new_tableau_bottom(game_state, resolved.source_index);
    } else if (resolved.source_type == RESOLVED_SOURCE_FOUNDATION && to->type == MOVE_REF_TABLEAU_BOTTOM) {
        card = detach_foundation_top(game_state, &resolved);
        append_segment_to_tableau(game_state, to->pile_index, card, 1);
    } else {
        set_message(game_state, "Move is not valid.");
        return false;
    }

    set_message(game_state, "OK");
    return true;
}

bool game_is_won(const GameState *game_state)
{
    int foundation_index;
    int total_cards;

    if (game_state == NULL) {
        return false;
    }

    total_cards = 0;
    for (foundation_index = 0; foundation_index < FOUNDATION_COUNT; foundation_index++) {
        total_cards += game_state->foundations[foundation_index].size;
    }

    return total_cards == DECK_CARD_COUNT;
}
