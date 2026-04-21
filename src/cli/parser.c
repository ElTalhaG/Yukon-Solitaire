#include "parser.h"

#include <ctype.h>
#include <stddef.h>
#include <string.h>

/*
 * This file is the boring but super necessary string-cleanup zone.
 * If we do this part carefully now, the actual game loop later gets way cleaner.
 */

static void init_move_reference(MoveReference *reference)
{
    reference->type = MOVE_REF_NONE;
    reference->pile_index = -1;
    reference->has_card_code = false;
    reference->rank = CARD_RANK_ACE;
    reference->suit = CARD_SUIT_CLUBS;
}

static void init_parsed_command(ParsedCommand *command)
{
    command->type = COMMAND_TYPE_INVALID;
    command->raw_text[0] = '\0';
    command->argument[0] = '\0';
    command->has_argument = false;
    command->has_split = false;
    command->split = 0;
    init_move_reference(&command->from);
    init_move_reference(&command->to);
}

static void trim_line_endings(char *text)
{
    size_t length;

    if (text == NULL) {
        return;
    }

    length = strlen(text);
    while (length > 0 && (text[length - 1] == '\n' || text[length - 1] == '\r')) {
        text[length - 1] = '\0';
        length--;
    }
}

static void trim_spaces(char *text)
{
    char *start;
    char *end;
    size_t new_length;

    if (text == NULL || text[0] == '\0') {
        return;
    }

    /*
     * We trim both ends so commands like "  LD cards.txt  " still work.
     * Internal spaces are not normalized here because sometimes they matter.
     */
    start = text;
    while (*start != '\0' && isspace((unsigned char) *start)) {
        start++;
    }

    if (*start == '\0') {
        text[0] = '\0';
        return;
    }

    end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char) *end)) {
        *end = '\0';
        end--;
    }

    if (start != text) {
        new_length = strlen(start);
        memmove(text, start, new_length + 1);
    }
}

static bool copy_argument(char *destination, const char *source)
{
    size_t length;

    if (destination == NULL || source == NULL) {
        return false;
    }

    length = strlen(source);
    if (length >= MAX_COMMAND_LENGTH) {
        return false;
    }

    memcpy(destination, source, length + 1);
    return true;
}

static bool parse_positive_integer(const char *text, int *value)
{
    int result;
    int index;

    if (text == NULL || text[0] == '\0' || value == NULL) {
        return false;
    }

    result = 0;
    for (index = 0; text[index] != '\0'; index++) {
        if (!isdigit((unsigned char) text[index])) {
            return false;
        }

        result = result * 10 + (text[index] - '0');
    }

    *value = result;
    return true;
}

static bool parse_column_number(char prefix, const char *text, int *pile_index)
{
    int value;

    /*
     * This helper handles both C1..C7 and F1..F4 style names.
     * We keep it simple on purpose: one letter and one digit.
     */
    if (text == NULL || pile_index == NULL || text[0] != prefix || text[1] == '\0' || text[2] != '\0') {
        return false;
    }

    if (!parse_positive_integer(&text[1], &value)) {
        return false;
    }

    *pile_index = value - 1;
    return true;
}

static bool parse_move_source(const char *text, MoveReference *reference)
{
    char card_code[3];
    const char *separator;

    if (text == NULL || reference == NULL) {
        return false;
    }

    /*
     * Sources can be:
     * - C6       -> bottom card of a tableau column
     * - C6:4H    -> a specific visible card inside a tableau column
     * - F3       -> top card of a foundation
     */
    separator = strchr(text, ':');
    if (separator == NULL) {
        if (parse_column_number('C', text, &reference->pile_index) &&
            reference->pile_index >= 0 && reference->pile_index < TABLEAU_COLUMNS) {
            reference->type = MOVE_REF_TABLEAU_BOTTOM;
            return true;
        }

        if (parse_column_number('F', text, &reference->pile_index) &&
            reference->pile_index >= 0 && reference->pile_index < FOUNDATION_COUNT) {
            reference->type = MOVE_REF_FOUNDATION;
            return true;
        }

        return false;
    }

    if (separator == text || separator[0] == '\0' || separator[1] == '\0') {
        return false;
    }

    if (separator - text != 2) {
        return false;
    }

    if (!parse_column_number('C', text, &reference->pile_index) ||
        reference->pile_index < 0 || reference->pile_index >= TABLEAU_COLUMNS) {
        return false;
    }

    if (strlen(separator + 1) != 2) {
        return false;
    }

    card_code[0] = separator[1];
    card_code[1] = separator[2];
    card_code[2] = '\0';

    if (!card_parse_code(card_code, &reference->rank, &reference->suit)) {
        return false;
    }

    reference->type = MOVE_REF_TABLEAU_CARD;
    reference->has_card_code = true;
    return true;
}

static bool parse_move_destination(const char *text, MoveReference *reference)
{
    if (text == NULL || reference == NULL) {
        return false;
    }

    /*
     * Destinations are intentionally stricter:
     * - Cx means append to a tableau column
     * - Fx means move onto a foundation
     * Nothing else should reach the move-execution layer.
     */
    if (parse_column_number('C', text, &reference->pile_index) &&
        reference->pile_index >= 0 && reference->pile_index < TABLEAU_COLUMNS) {
        reference->type = MOVE_REF_TABLEAU_BOTTOM;
        return true;
    }

    if (parse_column_number('F', text, &reference->pile_index) &&
        reference->pile_index >= 0 && reference->pile_index < FOUNDATION_COUNT) {
        reference->type = MOVE_REF_FOUNDATION;
        return true;
    }

    return false;
}

static bool parse_move_command(const char *text, ParsedCommand *command)
{
    char left[MAX_COMMAND_LENGTH];
    char right[MAX_COMMAND_LENGTH];
    const char *arrow;
    size_t left_length;
    size_t right_length;

    arrow = strstr(text, "->");
    if (arrow == NULL) {
        return false;
    }

    /*
     * The project says there must be no spaces around ->, so we reject commands
     * like "C1 -> C2" here instead of trying to be clever.
     */
    if (arrow == text || arrow[2] == '\0' ||
        isspace((unsigned char) arrow[-1]) || isspace((unsigned char) arrow[2])) {
        return false;
    }

    left_length = (size_t) (arrow - text);
    right_length = strlen(arrow + 2);

    if (left_length == 0 || right_length == 0 ||
        left_length >= sizeof(left) || right_length >= sizeof(right)) {
        return false;
    }

    memcpy(left, text, left_length);
    left[left_length] = '\0';
    memcpy(right, arrow + 2, right_length + 1);

    if (strstr(arrow + 2, "->") != NULL) {
        return false;
    }

    if (!parse_move_source(left, &command->from)) {
        return false;
    }

    if (!parse_move_destination(right, &command->to)) {
        return false;
    }

    command->type = COMMAND_TYPE_MOVE;
    return true;
}

static bool parse_word_and_rest(char *text, char **word, char **rest)
{
    char *cursor;

    if (text == NULL || word == NULL || rest == NULL) {
        return false;
    }

    trim_spaces(text);
    if (text[0] == '\0') {
        return false;
    }

    *word = text;
    cursor = text;
    while (*cursor != '\0' && !isspace((unsigned char) *cursor)) {
        cursor++;
    }

    if (*cursor == '\0') {
        *rest = cursor;
        return true;
    }

    *cursor = '\0';
    cursor++;
    while (*cursor != '\0' && isspace((unsigned char) *cursor)) {
        cursor++;
    }

    *rest = cursor;
    return true;
}

static bool parse_startup_or_play_command(char *text, ParsedCommand *command)
{
    char *word;
    char *rest;

    if (!parse_word_and_rest(text, &word, &rest)) {
        return false;
    }

    /*
     * We keep the command words very direct and case-sensitive for now.
     * That matches the assignment examples and avoids a bunch of parser noise.
     */
    if (strcmp(word, "LD") == 0) {
        command->type = COMMAND_TYPE_LD;
        if (*rest != '\0') {
            command->has_argument = copy_argument(command->argument, rest);
            return command->has_argument;
        }
        return true;
    }

    if (strcmp(word, "SW") == 0) {
        command->type = COMMAND_TYPE_SW;
        return *rest == '\0';
    }

    if (strcmp(word, "SI") == 0) {
        command->type = COMMAND_TYPE_SI;
        if (*rest == '\0') {
            return true;
        }

        command->has_split = parse_positive_integer(rest, &command->split);
        return command->has_split;
    }

    if (strcmp(word, "SR") == 0) {
        command->type = COMMAND_TYPE_SR;
        return *rest == '\0';
    }

    if (strcmp(word, "SD") == 0) {
        command->type = COMMAND_TYPE_SD;
        if (*rest != '\0') {
            command->has_argument = copy_argument(command->argument, rest);
            return command->has_argument;
        }
        return true;
    }

    if (strcmp(word, "QQ") == 0) {
        command->type = COMMAND_TYPE_QQ;
        return *rest == '\0';
    }

    if (strcmp(word, "P") == 0) {
        command->type = COMMAND_TYPE_P;
        return *rest == '\0';
    }

    if (strcmp(word, "Q") == 0) {
        command->type = COMMAND_TYPE_Q;
        return *rest == '\0';
    }

    return false;
}

bool read_command_line(FILE *stream, char *buffer, int buffer_size)
{
    if (stream == NULL || buffer == NULL || buffer_size <= 1) {
        return false;
    }

    /*
     * fgets is enough here and matches what the course material expects us to use.
     * It also keeps us out of the classic gets/overflow mess.
     */
    if (fgets(buffer, buffer_size, stream) == NULL) {
        return false;
    }

    trim_line_endings(buffer);
    return true;
}

bool parse_command(const char *input, ParsedCommand *command)
{
    char working_copy[MAX_COMMAND_LENGTH];
    size_t length;

    if (input == NULL || command == NULL) {
        return false;
    }

    init_parsed_command(command);

    length = strlen(input);
    if (length >= sizeof(working_copy)) {
        return false;
    }

    memcpy(command->raw_text, input, length + 1);
    memcpy(working_copy, input, length + 1);

    trim_line_endings(working_copy);
    trim_spaces(working_copy);

    /*
     * Empty input is treated as malformed input, not as "do nothing".
     * That way the command loop can respond with a clear error message later.
     */
    if (working_copy[0] == '\0') {
        return false;
    }

    if (parse_move_command(working_copy, command)) {
        return true;
    }

    if (parse_startup_or_play_command(working_copy, command)) {
        return true;
    }

    return false;
}
