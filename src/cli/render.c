#include "render.h"

#include <stddef.h>

#include "card.h"
#include "list.h"

/*
 * The terminal output is rendered row by row so it matches the way the course
 * handout shows the Yukon tableau in the terminal window.
 */

static int max_tableau_height(const GameState *game_state)
{
    int column_index;
    int max_height;

    max_height = 0;
    for (column_index = 0; column_index < TABLEAU_COLUMNS; column_index++) {
        if (game_state->tableau[column_index].size > max_height) {
            max_height = game_state->tableau[column_index].size;
        }
    }

    return max_height;
}

static void format_or_blank(const Card *card, char output[4])
{
    if (card == NULL) {
        output[0] = '\0';
        return;
    }

    /*
     * card_format_display already knows how to produce either a visible code
     * such as "7H" or the hidden placeholder "[ ]".
     */
    if (!card_format_display(card, output)) {
        output[0] = '?';
        output[1] = '?';
        output[2] = '\0';
    }
}

static void render_header(FILE *stream)
{
    int column_index;

    for (column_index = 0; column_index < TABLEAU_COLUMNS; column_index++) {
        fprintf(stream, "C%d", column_index + 1);
        if (column_index + 1 < TABLEAU_COLUMNS) {
            fputc('\t', stream);
        }
    }

    fputc('\n', stream);
    fputc('\n', stream);
}

static void render_foundation_slot(FILE *stream, const GameState *game_state, int foundation_index)
{
    const Card *top_card;
    const Card *current;
    char foundation_text[4];

    /*
     * Foundations grow at the tail, so the visible "top" card is the last node.
     * Using the head here would look fine for one card and then go wrong later.
     */
    current = game_state->foundations[foundation_index].top;
    while (current != NULL && current->next != NULL) {
        current = current->next;
    }

    top_card = current;
    format_or_blank(top_card, foundation_text);

    if (foundation_text[0] == '\0') {
        fprintf(stream, "[] F%d", foundation_index + 1);
    } else {
        fprintf(stream, "%s F%d", foundation_text, foundation_index + 1);
    }
}

static void render_tableau_row(FILE *stream, const GameState *game_state, int row_index)
{
    int column_index;
    const Card *card;
    char card_text[4];

    for (column_index = 0; column_index < TABLEAU_COLUMNS; column_index++) {
        card = card_list_get_at(game_state->tableau[column_index].top, row_index);
        format_or_blank(card, card_text);

        if (card_text[0] != '\0') {
            fputs(card_text, stream);
        }

        if (column_index + 1 < TABLEAU_COLUMNS) {
            fputc('\t', stream);
        }
    }

    /*
     * The handout places one foundation label on every other rendered row.
     * We mirror that layout so the text version stays visually familiar.
     */
    if (row_index % 2 == 0 && row_index / 2 < FOUNDATION_COUNT) {
        fputs("\t\t", stream);
        render_foundation_slot(stream, game_state, row_index / 2);
    }

    fputc('\n', stream);
}

static void render_footer(FILE *stream, const GameState *game_state)
{
    fprintf(stream, "\nLast Command:%s\n", game_state->last_command);
    fprintf(stream, "Message:%s\n", game_state->message);
    fputs("INPUT > ", stream);
}

void render_game_state(FILE *stream, const GameState *game_state)
{
    int row_index;
    int row_count;

    if (stream == NULL || game_state == NULL) {
        return;
    }

    render_header(stream);

    row_count = max_tableau_height(game_state);
    if (row_count == 0) {
        /*
         * The startup screen still needs enough rows to show all foundations,
         * even before any cards have been loaded or dealt into the tableau.
         */
        row_count = FOUNDATION_COUNT * 2 - 1;
    }

    for (row_index = 0; row_index < row_count; row_index++) {
        render_tableau_row(stream, game_state, row_index);
    }

    render_footer(stream, game_state);
}
