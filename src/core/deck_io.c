#include "deck_io.h"

#include <stdio.h>
#include <string.h>

#include "list.h"

/*
 * This module implements the deck file format described by the course:
 * one two-character card code per line, for exactly 52 unique cards.
 */

static void set_error_message(char *error_message, size_t error_size, const char *message)
{
    if (error_message == NULL || error_size == 0) {
        return;
    }

    snprintf(error_message, error_size, "%s", message);
}

static void trim_line_endings(char *line)
{
    size_t length;

    if (line == NULL) {
        return;
    }

    length = strlen(line);
    while (length > 0 && (line[length - 1] == '\n' || line[length - 1] == '\r')) {
        line[length - 1] = '\0';
        length--;
    }
}

static bool append_validated_card(Deck *deck, CardRank rank, CardSuit suit, bool is_face_up,
                                  char *error_message, size_t error_size)
{
    Card *card;

    card = card_create(rank, suit, is_face_up);
    if (card == NULL) {
        set_error_message(error_message, error_size, "Memory allocation failed while creating a card.");
        return false;
    }

    deck_append_card(deck, card);
    return true;
}

static bool load_default_suit(Deck *deck, CardSuit suit, char *error_message, size_t error_size)
{
    CardRank rank;

    /*
     * The default startup deck is deterministic so repeated project tests
     * always begin from exactly the same baseline order.
     */
    for (rank = CARD_RANK_ACE; rank <= CARD_RANK_KING; rank++) {
        if (!append_validated_card(deck, rank, suit, false, error_message, error_size)) {
            return false;
        }
    }

    return true;
}

static bool parse_file_line(const char *line, int line_number, CardRank *rank, CardSuit *suit,
                            char *error_message, size_t error_size)
{
    if (!card_parse_code(line, rank, suit)) {
        snprintf(error_message, error_size, "Invalid card at line %d.", line_number);
        return false;
    }

    return true;
}

static bool validate_unique_card(CardRank rank, CardSuit suit, bool seen[FOUNDATION_COUNT][DECK_CARD_COUNT / FOUNDATION_COUNT],
                                 int line_number, char *error_message, size_t error_size)
{
    int suit_index;
    int rank_index;

    suit_index = (int) suit;
    rank_index = (int) rank - 1;

    if (seen[suit_index][rank_index]) {
        snprintf(error_message, error_size, "Duplicate card at line %d.", line_number);
        return false;
    }

    seen[suit_index][rank_index] = true;
    return true;
}

static bool validate_complete_deck(int card_count, bool seen[FOUNDATION_COUNT][DECK_CARD_COUNT / FOUNDATION_COUNT],
                                   char *error_message, size_t error_size)
{
    int suit_index;
    int rank_index;

    if (card_count != DECK_CARD_COUNT) {
        snprintf(error_message, error_size, "Deck must contain exactly %d cards.", DECK_CARD_COUNT);
        return false;
    }

    /*
     * Exact count alone is not enough; we also verify that every suit contains
     * every rank exactly once to match the course requirement.
     */
    for (suit_index = 0; suit_index < FOUNDATION_COUNT; suit_index++) {
        for (rank_index = 0; rank_index < DECK_CARD_COUNT / FOUNDATION_COUNT; rank_index++) {
            if (!seen[suit_index][rank_index]) {
                set_error_message(error_message, error_size, "Deck is missing one or more required cards.");
                return false;
            }
        }
    }

    return true;
}

void deck_clear(Deck *deck)
{
    if (deck == NULL) {
        return;
    }

    card_list_destroy(&deck->top);
    deck->size = 0;
}

bool deck_load_default(Deck *deck, char *error_message, size_t error_size)
{
    if (deck == NULL) {
        set_error_message(error_message, error_size, "Deck pointer is NULL.");
        return false;
    }

    deck_clear(deck);

    if (!load_default_suit(deck, CARD_SUIT_CLUBS, error_message, error_size)) {
        deck_clear(deck);
        return false;
    }

    if (!load_default_suit(deck, CARD_SUIT_DIAMONDS, error_message, error_size)) {
        deck_clear(deck);
        return false;
    }

    if (!load_default_suit(deck, CARD_SUIT_HEARTS, error_message, error_size)) {
        deck_clear(deck);
        return false;
    }

    if (!load_default_suit(deck, CARD_SUIT_SPADES, error_message, error_size)) {
        deck_clear(deck);
        return false;
    }

    set_error_message(error_message, error_size, "OK");
    return true;
}

bool deck_load_from_file(Deck *deck, const char *filename, char *error_message, size_t error_size)
{
    FILE *file;
    char line[32];
    int line_number;
    int card_count;
    CardRank rank;
    CardSuit suit;
    bool seen[FOUNDATION_COUNT][DECK_CARD_COUNT / FOUNDATION_COUNT];

    if (deck == NULL) {
        set_error_message(error_message, error_size, "Deck pointer is NULL.");
        return false;
    }

    if (filename == NULL || filename[0] == '\0') {
        set_error_message(error_message, error_size, "Filename is missing.");
        return false;
    }

    file = fopen(filename, "r");
    if (file == NULL) {
        set_error_message(error_message, error_size, "File does not exist or could not be opened.");
        return false;
    }

    deck_clear(deck);
    memset(seen, 0, sizeof(seen));
    line_number = 0;
    card_count = 0;

    while (fgets(line, sizeof(line), file) != NULL) {
        line_number++;
        trim_line_endings(line);

        if (!parse_file_line(line, line_number, &rank, &suit, error_message, error_size)) {
            fclose(file);
            deck_clear(deck);
            return false;
        }

        if (!validate_unique_card(rank, suit, seen, line_number, error_message, error_size)) {
            fclose(file);
            deck_clear(deck);
            return false;
        }

        if (!append_validated_card(deck, rank, suit, false, error_message, error_size)) {
            fclose(file);
            deck_clear(deck);
            return false;
        }

        card_count++;
    }

    fclose(file);

    if (!validate_complete_deck(card_count, seen, error_message, error_size)) {
        deck_clear(deck);
        return false;
    }

    set_error_message(error_message, error_size, "OK");
    return true;
}

bool deck_save_to_file(const Deck *deck, const char *filename, char *error_message, size_t error_size)
{
    FILE *file;
    const Card *current;
    char face_up_text[3];

    if (deck == NULL) {
        set_error_message(error_message, error_size, "Deck pointer is NULL.");
        return false;
    }

    if (filename == NULL || filename[0] == '\0') {
        filename = "cards.txt";
    }

    file = fopen(filename, "w");
    if (file == NULL) {
        set_error_message(error_message, error_size, "Could not open file for writing.");
        return false;
    }

    /*
     * Save preserves the current linked-list order exactly, since the course
     * treats the deck order as meaningful for shuffling and dealing later on.
     */
    current = deck->top;
    while (current != NULL) {
        if (!card_format_face_up(current, face_up_text)) {
            fclose(file);
            set_error_message(error_message, error_size, "Encountered an invalid card while saving.");
            return false;
        }

        fprintf(file, "%s\n", face_up_text);
        current = current->next;
    }

    fclose(file);
    set_error_message(error_message, error_size, "OK");
    return true;
}
