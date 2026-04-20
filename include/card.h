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

/* Allocate and initialize one new card node. Returns NULL on failure. */
Card *card_create(CardRank rank, CardSuit suit, bool is_face_up);

/* Free one card node allocated with card_create. */
void card_destroy(Card *card);

/* Convert one rank character such as 'A' or 'T' into a CardRank. */
bool card_rank_from_char(char rank_char, CardRank *rank);

/* Convert one suit character such as 'H' or 'S' into a CardSuit. */
bool card_suit_from_char(char suit_char, CardSuit *suit);

/* Convert enum values back into their single-character text form. */
char card_rank_to_char(CardRank rank);
char card_suit_to_char(CardSuit suit);

/* Parse a two-character card code such as "AH" or "TD". */
bool card_parse_code(const char *text, CardRank *rank, CardSuit *suit);

/* Validate that rank and suit values are inside the expected enum ranges. */
bool card_is_valid(CardRank rank, CardSuit suit);

/* Format one face-up card into a three-byte buffer such as "AH". */
bool card_format_face_up(const Card *card, char output[3]);

/* Format either a face-up or hidden card for the terminal UI. */
bool card_format_display(const Card *card, char output[4]);

/* Comparison helpers used later for move validation. */
int card_rank_difference(CardRank left, CardRank right);
bool card_same_suit(const Card *left, const Card *right);
bool card_same_rank(const Card *left, const Card *right);

#endif
