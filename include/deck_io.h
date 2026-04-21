#ifndef DECK_IO_H
#define DECK_IO_H

#include <stddef.h>
#include <stdbool.h>

#include "game.h"

/*
 * Deck loading and saving stays in its own module so startup-phase commands
 * can reuse the same logic from both the terminal version and the future GUI.
 */

/* Remove every card currently stored in the deck. */
void deck_clear(Deck *deck);

/* Load the default ordered deck: clubs, diamonds, hearts, spades, each A to K. */
bool deck_load_default(Deck *deck, char *error_message, size_t error_size);

/* Load and validate a 52-card deck from a text file. */
bool deck_load_from_file(Deck *deck, const char *filename, char *error_message, size_t error_size);

/* Save the current deck order back to a text file, one card per line. */
bool deck_save_to_file(const Deck *deck, const char *filename, char *error_message, size_t error_size);

#endif
