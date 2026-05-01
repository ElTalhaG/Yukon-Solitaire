#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>
#include <stdio.h>

#include "card.h"
#include "game.h"

/*
 * This parser only answers "what command did the user type?"
 * It does not execute anything yet. That keeps parsing and game logic separate,
 * which is way easier to debug once commands start failing for weird reasons.
 */

typedef enum {
    COMMAND_TYPE_INVALID,
    COMMAND_TYPE_LD,
    COMMAND_TYPE_SW,
    COMMAND_TYPE_SI,
    COMMAND_TYPE_SR,
    COMMAND_TYPE_SD,
    COMMAND_TYPE_SAVE_GAME,
    COMMAND_TYPE_LOAD_GAME,
    COMMAND_TYPE_QQ,
    COMMAND_TYPE_P,
    COMMAND_TYPE_Q,
    COMMAND_TYPE_MOVE
} CommandType;

typedef enum {
    MOVE_REF_NONE,
    MOVE_REF_TABLEAU_BOTTOM,
    MOVE_REF_TABLEAU_CARD,
    MOVE_REF_FOUNDATION
} MoveReferenceType;

typedef struct {
    MoveReferenceType type;

    /*
     * We store columns/foundations as zero-based indexes internally.
     * That makes array access easier later, even though the user types C1/F1.
     */
    int pile_index;

    /* Only used when the move source is something like C6:4H. */
    bool has_card_code;
    CardRank rank;
    CardSuit suit;
} MoveReference;

typedef struct {
    CommandType type;

    /*
     * Keeping the original text is handy for "Last Command:" rendering and for
     * those moments where debugging gets annoying and we just want to see exactly
     * what the parser received.
     */
    char raw_text[MAX_COMMAND_LENGTH];

    /* Optional filename for LD, SD, S, and L. */
    char argument[MAX_COMMAND_LENGTH];
    bool has_argument;

    /* Optional numeric split for SI. */
    bool has_split;
    int split;

    /* Used only for parsed move commands. */
    MoveReference from;
    MoveReference to;
} ParsedCommand;

/* Read one command line safely from the terminal input stream. */
bool read_command_line(FILE *stream, char *buffer, int buffer_size);

/* Parse one raw command line into a structured command description. */
bool parse_command(const char *input, ParsedCommand *command);

#endif
