#include "card.h"

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
