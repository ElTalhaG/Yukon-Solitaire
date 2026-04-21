#ifndef SHUFFLE_H
#define SHUFFLE_H

#include <stdbool.h>
#include <stddef.h>

#include "game.h"

/*
 * Shuffle helpers operate directly on the current deck order.
 * They implement the two startup-phase shuffle commands required by the course:
 * - SI: split the deck into two piles and interleave them
 * - SR: random insertion shuffle
 */

/* Seed the pseudo-random generator once near program startup. */
void shuffle_seed_random(unsigned int seed);

/* Return true after the random generator has been seeded at least once. */
bool shuffle_is_seeded(void);

/* Interleave the deck using a caller-provided split position. */
bool deck_shuffle_interleave(Deck *deck, int split, char *error_message, size_t error_size);

/* Interleave the deck using a random valid split position. */
bool deck_shuffle_interleave_random_split(Deck *deck, char *error_message, size_t error_size);

/* Randomly insert each next card into a new pile, as required by SR. */
bool deck_shuffle_random_insert(Deck *deck, char *error_message, size_t error_size);

#endif
