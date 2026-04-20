#ifndef LIST_H
#define LIST_H

#include "game.h"

/*
 * These helpers centralize linked-list manipulation for cards so the deck,
 * tableau, and foundation code can share the same pointer logic.
 *
 * The raw list helpers work on Card * head pointers directly.
 * The deck/tableau/foundation helpers are thin wrappers that also keep the
 * stored size fields in sync with the actual linked-list contents.
 */

/* Remove and free every card in a linked list. */
void card_list_destroy(Card **head);

/* Return the last node in a list, or NULL if the list is empty. */
Card *card_list_tail(Card *head);

/* Count nodes by traversal. Useful for validation and debugging. */
int card_list_count(const Card *head);

/* Append one card node to the end of a list. */
void card_list_append(Card **head, Card *card);

/* Push one card onto the front of a list. */
void card_list_push_front(Card **head, Card *card);

/* Remove and return the first card in a list. */
Card *card_list_pop_front(Card **head);

/* Return the node at a zero-based index, or NULL if out of range. */
Card *card_list_get_at(Card *head, int index);

/* Split a list so the node at split_index becomes the new head of the second list. */
Card *card_list_split_at(Card **head, int split_index);

/* Append all nodes from source onto the end of destination. */
void card_list_splice_end(Card **destination, Card **source);

/* Move a whole card segment from source to the end of destination. */
void card_list_move_segment(Card **destination, Card **source);

/* Convenience helpers that keep the stored size in sync with the list contents. */
void deck_append_card(Deck *deck, Card *card);
Card *deck_draw_top(Deck *deck);
void tableau_append_card(TableauColumn *column, Card *card);
Card *tableau_take_top(TableauColumn *column);
void foundation_append_card(FoundationPile *foundation, Card *card);
Card *foundation_take_top(FoundationPile *foundation);

/* Free every card owned by the current game state. */
void game_state_destroy(GameState *game_state);

#endif
