#include "card.h"

#include <stddef.h>
#include <stdlib.h>

/*
 * This file is intentionally small for now.
 * The important part in the current phase is agreeing on the Card node
 * layout so every other backend structure can rely on the same shape.
 *
 * In the next phase this file will grow to include helpers for:
 * - parsing cards from text such as "AH" or "TD"
 * - formatting cards back to text for the terminal UI
 * - comparing ranks and suits for move validation
 */

Card *card_create(CardRank rank, CardSuit suit, bool is_face_up)
{
    Card *card;

    /* Never allocate a card with impossible enum values. */
    if (!card_is_valid(rank, suit)) {
        return NULL;
    }

    /* Each card is an independent heap-allocated linked-list node. */
    card = malloc(sizeof(*card));
    if (card == NULL) {
        return NULL;
    }

    card->rank = rank;
    card->suit = suit;
    card->is_face_up = is_face_up;
    card->next = NULL;
    return card;
}

void card_destroy(Card *card)
{
    /* free(NULL) is safe, so callers do not need extra guards here. */
    free(card);
}

bool card_rank_from_char(char rank_char, CardRank *rank)
{
    /* The out-parameter must exist before we try to write into it. */
    if (rank == NULL) {
        return false;
    }

    /* Match the exact rank letters required by the assignment format. */
    switch (rank_char) {
    case 'A':
        *rank = CARD_RANK_ACE;
        return true;
    case '2':
        *rank = CARD_RANK_2;
        return true;
    case '3':
        *rank = CARD_RANK_3;
        return true;
    case '4':
        *rank = CARD_RANK_4;
        return true;
    case '5':
        *rank = CARD_RANK_5;
        return true;
    case '6':
        *rank = CARD_RANK_6;
        return true;
    case '7':
        *rank = CARD_RANK_7;
        return true;
    case '8':
        *rank = CARD_RANK_8;
        return true;
    case '9':
        *rank = CARD_RANK_9;
        return true;
    case 'T':
        *rank = CARD_RANK_10;
        return true;
    case 'J':
        *rank = CARD_RANK_JACK;
        return true;
    case 'Q':
        *rank = CARD_RANK_QUEEN;
        return true;
    case 'K':
        *rank = CARD_RANK_KING;
        return true;
    default:
        return false;
    }
}

bool card_suit_from_char(char suit_char, CardSuit *suit)
{
    if (suit == NULL) {
        return false;
    }

    /* Suit letters follow the course specification: C, D, H, and S. */
    switch (suit_char) {
    case 'C':
        *suit = CARD_SUIT_CLUBS;
        return true;
    case 'D':
        *suit = CARD_SUIT_DIAMONDS;
        return true;
    case 'H':
        *suit = CARD_SUIT_HEARTS;
        return true;
    case 'S':
        *suit = CARD_SUIT_SPADES;
        return true;
    default:
        return false;
    }
}

char card_rank_to_char(CardRank rank)
{
    /* This is the inverse mapping of card_rank_from_char. */
    switch (rank) {
    case CARD_RANK_ACE:
        return 'A';
    case CARD_RANK_2:
        return '2';
    case CARD_RANK_3:
        return '3';
    case CARD_RANK_4:
        return '4';
    case CARD_RANK_5:
        return '5';
    case CARD_RANK_6:
        return '6';
    case CARD_RANK_7:
        return '7';
    case CARD_RANK_8:
        return '8';
    case CARD_RANK_9:
        return '9';
    case CARD_RANK_10:
        return 'T';
    case CARD_RANK_JACK:
        return 'J';
    case CARD_RANK_QUEEN:
        return 'Q';
    case CARD_RANK_KING:
        return 'K';
    default:
        return '\0';
    }
}

char card_suit_to_char(CardSuit suit)
{
    /* This is the inverse mapping of card_suit_from_char. */
    switch (suit) {
    case CARD_SUIT_CLUBS:
        return 'C';
    case CARD_SUIT_DIAMONDS:
        return 'D';
    case CARD_SUIT_HEARTS:
        return 'H';
    case CARD_SUIT_SPADES:
        return 'S';
    default:
        return '\0';
    }
}

bool card_parse_code(const char *text, CardRank *rank, CardSuit *suit)
{
    /* We expect exactly two printable card characters plus the terminator. */
    if (text == NULL || rank == NULL || suit == NULL) {
        return false;
    }

    if (text[0] == '\0' || text[1] == '\0' || text[2] != '\0') {
        return false;
    }

    return card_rank_from_char(text[0], rank) && card_suit_from_char(text[1], suit);
}

bool card_is_valid(CardRank rank, CardSuit suit)
{
    /* Enum range checks protect later code from malformed card values. */
    return rank >= CARD_RANK_ACE && rank <= CARD_RANK_KING &&
           suit >= CARD_SUIT_CLUBS && suit <= CARD_SUIT_SPADES;
}

bool card_format_face_up(const Card *card, char output[3])
{
    /* A face-up card is shown as exactly two characters such as "QS". */
    if (card == NULL || output == NULL || !card_is_valid(card->rank, card->suit)) {
        return false;
    }

    output[0] = card_rank_to_char(card->rank);
    output[1] = card_suit_to_char(card->suit);
    output[2] = '\0';
    return output[0] != '\0' && output[1] != '\0';
}

bool card_format_display(const Card *card, char output[4])
{
    /* The display format is either two card characters or the hidden token. */
    if (card == NULL || output == NULL) {
        return false;
    }

    /* Hidden cards must appear exactly as [ ] in the terminal UI. */
    if (!card->is_face_up) {
        output[0] = '[';
        output[1] = ' ';
        output[2] = ']';
        output[3] = '\0';
        return true;
    }

    if (!card_format_face_up(card, output)) {
        return false;
    }

    output[2] = '\0';
    return true;
}

int card_rank_difference(CardRank left, CardRank right)
{
    /* Subtracting enum values is enough because rank order is intentional. */
    return (int) left - (int) right;
}

bool card_same_suit(const Card *left, const Card *right)
{
    /* Null checks keep comparison helpers safe in validation code. */
    return left != NULL && right != NULL && left->suit == right->suit;
}

bool card_same_rank(const Card *left, const Card *right)
{
    return left != NULL && right != NULL && left->rank == right->rank;
}
