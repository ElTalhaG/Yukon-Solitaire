#ifndef CARD_H
#define CARD_H

#include <stdbool.h>

/*
 * Card ranks follow normal card order from Ace to King.
 * The numeric values are intentional so later game logic can
 * compare ranks directly when checking move legality.
 */
typedef enum {
    CARD_RANK_ACE = 1,
    CARD_RANK_2,
    CARD_RANK_3,
    CARD_RANK_4,
    CARD_RANK_5,
    CARD_RANK_6,
    CARD_RANK_7,
    CARD_RANK_8,
    CARD_RANK_9,
    CARD_RANK_10,
    CARD_RANK_JACK,
    CARD_RANK_QUEEN,
    CARD_RANK_KING
} CardRank;

/*
 * Suits are stored as an enum instead of characters so the backend
 * can work with a compact and type-safe representation.
 */
typedef enum {
    CARD_SUIT_CLUBS,
    CARD_SUIT_DIAMONDS,
    CARD_SUIT_HEARTS,
    CARD_SUIT_SPADES
} CardSuit;

/*
 * A Card is the fundamental linked-list node used everywhere in the
 * project: the deck, tableau columns, and foundation piles.
 */
typedef struct Card {
    /* Rank/value of the card, for example Ace, 7, Queen, or King. */
    CardRank rank;

    /* Suit of the card: clubs, diamonds, hearts, or spades. */
    CardSuit suit;

    /* True when the card is visible to the player in the interface. */
    bool is_face_up;

    /* Pointer to the next card in the same linked list. */
    struct Card *next;
} Card;

#endif
