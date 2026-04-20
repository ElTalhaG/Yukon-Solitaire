#include "list.h"

#include <stddef.h>

/*
 * This file contains the low-level pointer work for card-linked lists.
 * Keeping it centralized makes later deck loading and move execution much
 * easier to reason about and much safer to change.
 */

void card_list_destroy(Card **head)
{
    Card *current;
    Card *next;

    /* A NULL head pointer means the caller had no list variable to clear. */
    if (head == NULL) {
        return;
    }

    /* Walk through the list once, freeing each node exactly one time. */
    current = *head;
    while (current != NULL) {
        next = current->next;
        card_destroy(current);
        current = next;
    }

    *head = NULL;
}

Card *card_list_tail(Card *head)
{
    Card *current;

    /* Empty lists simply have no tail node. */
    current = head;
    if (current == NULL) {
        return NULL;
    }

    /* The tail is the first node whose next pointer is NULL. */
    while (current->next != NULL) {
        current = current->next;
    }

    return current;
}

int card_list_count(const Card *head)
{
    int count;
    const Card *current;

    /* This helper is intentionally traversal-based for verification work. */
    count = 0;
    current = head;
    while (current != NULL) {
        count++;
        current = current->next;
    }

    return count;
}

void card_list_append(Card **head, Card *card)
{
    Card *tail;

    /* Appending does nothing if either the list or node is missing. */
    if (head == NULL || card == NULL) {
        return;
    }

    /* The appended node becomes the new tail, so its next must be cleared. */
    card->next = NULL;
    if (*head == NULL) {
        *head = card;
        return;
    }

    /* Non-empty append means linking the current tail to the new node. */
    tail = card_list_tail(*head);
    tail->next = card;
}

void card_list_push_front(Card **head, Card *card)
{
    /* Push-front is the cheapest insert because it only rewires one pointer. */
    if (head == NULL || card == NULL) {
        return;
    }

    card->next = *head;
    *head = card;
}

Card *card_list_pop_front(Card **head)
{
    Card *card;

    /* Popping from an empty list simply yields no card. */
    if (head == NULL || *head == NULL) {
        return NULL;
    }

    /* Detach the first node so callers receive an isolated segment. */
    card = *head;
    *head = card->next;
    card->next = NULL;
    return card;
}

Card *card_list_get_at(Card *head, int index)
{
    int current_index;
    Card *current;

    /* Negative positions are always invalid in our list API. */
    if (index < 0) {
        return NULL;
    }

    /* Move forward until we either hit the index or run out of nodes. */
    current = head;
    current_index = 0;
    while (current != NULL && current_index < index) {
        current = current->next;
        current_index++;
    }

    return current;
}

Card *card_list_split_at(Card **head, int split_index)
{
    Card *current;
    Card *second_head;
    int index;

    /*
     * split_index is the first element that should move into the second list.
     * A split at 0 would mean "move the whole list", which we do not use here.
     */
    if (head == NULL || *head == NULL || split_index <= 0) {
        return NULL;
    }

    /* Stop on the node just before the split point so we can cut the chain. */
    current = *head;
    index = 0;
    while (current != NULL && index < split_index - 1) {
        current = current->next;
        index++;
    }

    if (current == NULL || current->next == NULL) {
        return NULL;
    }

    /* The second list starts right after the cut position. */
    second_head = current->next;
    current->next = NULL;
    return second_head;
}

void card_list_splice_end(Card **destination, Card **source)
{
    Card *tail;

    /* Splicing only makes sense if there is a source segment to attach. */
    if (destination == NULL || source == NULL || *source == NULL) {
        return;
    }

    /* Empty destination means the source segment becomes the whole list. */
    if (*destination == NULL) {
        *destination = *source;
        *source = NULL;
        return;
    }

    /* Otherwise, connect the existing tail to the start of the source list. */
    tail = card_list_tail(*destination);
    tail->next = *source;
    *source = NULL;
}

void card_list_move_segment(Card **destination, Card **source)
{
    /* Right now moving a segment is the same pointer operation as splicing. */
    card_list_splice_end(destination, source);
}

void deck_append_card(Deck *deck, Card *card)
{
    /* Wrapper helpers update both the list pointers and the tracked size. */
    if (deck == NULL || card == NULL) {
        return;
    }

    card_list_append(&deck->top, card);
    deck->size++;
}

Card *deck_draw_top(Deck *deck)
{
    Card *card;

    /* The deck's logical top is stored at the front of the linked list. */
    if (deck == NULL) {
        return NULL;
    }

    card = card_list_pop_front(&deck->top);
    if (card != NULL) {
        deck->size--;
    }

    return card;
}

void tableau_append_card(TableauColumn *column, Card *card)
{
    if (column == NULL || card == NULL) {
        return;
    }

    /* Appending preserves the current top-to-bottom order of the column list. */
    card_list_append(&column->top, card);
    column->size++;
}

Card *tableau_take_top(TableauColumn *column)
{
    Card *card;

    /* The first node represents the current accessible top of the column list. */
    if (column == NULL) {
        return NULL;
    }

    card = card_list_pop_front(&column->top);
    if (card != NULL) {
        column->size--;
    }

    return card;
}

void foundation_append_card(FoundationPile *foundation, Card *card)
{
    if (foundation == NULL || card == NULL) {
        return;
    }

    /* Foundations use the same list mechanics as other piles. */
    card_list_append(&foundation->top, card);
    foundation->size++;
}

Card *foundation_take_top(FoundationPile *foundation)
{
    Card *card;

    /* Removing from the front mirrors the current list ordering convention. */
    if (foundation == NULL) {
        return NULL;
    }

    card = card_list_pop_front(&foundation->top);
    if (card != NULL) {
        foundation->size--;
    }

    return card;
}

void game_state_destroy(GameState *game_state)
{
    int index;

    /* This releases every card currently owned by the game state. */
    if (game_state == NULL) {
        return;
    }

    /* The deck may still own cards if a game never started or was quit early. */
    card_list_destroy(&game_state->deck.top);
    game_state->deck.size = 0;

    /* Clear all tableau columns independently because each owns its own list. */
    for (index = 0; index < TABLEAU_COLUMNS; index++) {
        card_list_destroy(&game_state->tableau[index].top);
        game_state->tableau[index].size = 0;
    }

    /* Reset foundation state as well as freeing the nodes inside each pile. */
    for (index = 0; index < FOUNDATION_COUNT; index++) {
        card_list_destroy(&game_state->foundations[index].top);
        game_state->foundations[index].size = 0;
        game_state->foundations[index].is_suit_assigned = 0;
        game_state->foundations[index].suit = CARD_SUIT_CLUBS;
    }
}
