#include "shuffle.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "list.h"

/*
 * The standard C rand() generator is enough for this project.
 * We only need consistent deck randomization, not cryptographic security.
 */

static int shuffle_seeded = 0;

static void set_error_message(char *error_message, size_t error_size, const char *message)
{
    if (error_message == NULL || error_size == 0) {
        return;
    }

    snprintf(error_message, error_size, "%s", message);
}

static int random_index_inclusive(int max_index)
{
    /*
     * The caller guarantees max_index is non-negative.
     * The returned value is in the inclusive range [0, max_index].
     */
    return rand() % (max_index + 1);
}

static bool deck_can_shuffle(const Deck *deck, char *error_message, size_t error_size)
{
    if (deck == NULL) {
        set_error_message(error_message, error_size, "Deck pointer is NULL.");
        return false;
    }

    if (deck->size <= 1 || deck->top == NULL) {
        set_error_message(error_message, error_size, "No deck is loaded.");
        return false;
    }

    return true;
}

static bool split_is_valid(const Deck *deck, int split, char *error_message, size_t error_size)
{
    if (!deck_can_shuffle(deck, error_message, error_size)) {
        return false;
    }

    /*
     * The course requires a positive split that is still smaller than the
     * total number of cards currently in the deck.
     */
    if (split <= 0 || split >= deck->size) {
        set_error_message(error_message, error_size, "Split must be greater than 0 and smaller than the deck size.");
        return false;
    }

    return true;
}

static void list_insert_at(Card **head, Card *card, int index)
{
    Card *current;
    int current_index;

    if (head == NULL || card == NULL) {
        return;
    }

    /*
     * Inserting at position 0 means the new card becomes the new head.
     * Every later position inserts after the node just before the target slot.
     */
    if (index <= 0 || *head == NULL) {
        card_list_push_front(head, card);
        return;
    }

    current = *head;
    current_index = 0;
    while (current->next != NULL && current_index < index - 1) {
        current = current->next;
        current_index++;
    }

    card->next = current->next;
    current->next = card;
}

static void rebuild_deck_from_head(Deck *deck, Card *head)
{
    deck->top = head;
    deck->size = card_list_count(head);
}

void shuffle_seed_random(unsigned int seed)
{
    srand(seed);
    shuffle_seeded = 1;
}

bool shuffle_is_seeded(void)
{
    return shuffle_seeded != 0;
}

bool deck_shuffle_interleave(Deck *deck, int split, char *error_message, size_t error_size)
{
    Card *left;
    Card *right;
    Card *shuffled;
    Card *card;

    if (!split_is_valid(deck, split, error_message, error_size)) {
        return false;
    }

    /*
     * The current deck becomes two linked lists:
     * - left: the first split cards
     * - right: the remaining cards
     */
    left = deck->top;
    right = card_list_split_at(&left, split);
    shuffled = NULL;

    /*
     * Interleaving alternates one card from each pile until one runs out.
     * Any remaining tail is then appended unchanged to the shuffled pile.
     */
    while (left != NULL && right != NULL) {
        card = card_list_pop_front(&left);
        card_list_append(&shuffled, card);

        card = card_list_pop_front(&right);
        card_list_append(&shuffled, card);
    }

    card_list_splice_end(&shuffled, &left);
    card_list_splice_end(&shuffled, &right);

    rebuild_deck_from_head(deck, shuffled);
    set_error_message(error_message, error_size, "OK");
    return true;
}

bool deck_shuffle_interleave_random_split(Deck *deck, char *error_message, size_t error_size)
{
    int split;

    if (!deck_can_shuffle(deck, error_message, error_size)) {
        return false;
    }

    /*
     * Valid split positions are 1 through size - 1.
     * We generate a zero-based random offset and then shift it into range.
     */
    split = random_index_inclusive(deck->size - 2) + 1;
    return deck_shuffle_interleave(deck, split, error_message, error_size);
}

bool deck_shuffle_random_insert(Deck *deck, char *error_message, size_t error_size)
{
    Card *unshuffled;
    Card *shuffled;
    Card *card;
    int insert_index;
    int shuffled_size;

    if (!deck_can_shuffle(deck, error_message, error_size)) {
        return false;
    }

    /*
     * If nobody seeded the generator yet, we seed it on demand from time().
     * That keeps the shuffle usable even before the final command loop exists.
     */
    if (!shuffle_is_seeded()) {
        shuffle_seed_random((unsigned int) time(NULL));
    }

    unshuffled = deck->top;
    shuffled = NULL;
    shuffled_size = 0;

    /*
     * SR repeatedly removes the next card from the old pile and reinserts it
     * into a random position in the new pile. As the new pile grows, the valid
     * insertion range grows with it.
     */
    while (unshuffled != NULL) {
        card = card_list_pop_front(&unshuffled);
        insert_index = random_index_inclusive(shuffled_size);
        list_insert_at(&shuffled, card, insert_index);
        shuffled_size++;
    }

    rebuild_deck_from_head(deck, shuffled);
    set_error_message(error_message, error_size, "OK");
    return true;
}
