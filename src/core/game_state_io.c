#include "game_state_io.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "card.h"
#include "list.h"
#include "timer.h"

static void set_message(char *message, size_t message_size, const char *text)
{
    if (message == NULL || message_size == 0 || text == NULL) {
        return;
    }

    snprintf(message, message_size, "%s", text);
}

static const char *phase_to_text(GamePhase phase)
{
    if (phase == GAME_PHASE_PLAY) {
        return "PLAY";
    }

    return "STARTUP";
}

static int text_to_phase(const char *text, GamePhase *phase)
{
    if (text == NULL || phase == NULL) {
        return 0;
    }

    if (strcmp(text, "PLAY") == 0) {
        *phase = GAME_PHASE_PLAY;
        return 1;
    }

    if (strcmp(text, "STARTUP") == 0) {
        *phase = GAME_PHASE_STARTUP;
        return 1;
    }

    return 0;
}

static int write_card_list(FILE *file, const Card *head)
{
    const Card *current;
    char code[3];

    current = head;
    while (current != NULL) {
        if (!card_format_face_up(current, code)) {
            return 0;
        }

        fprintf(file, "%s %d\n", code, current->is_face_up ? 1 : 0);
        current = current->next;
    }

    return 1;
}

static int read_card_into_list(FILE *file, Card **head, int *size)
{
    Card *card;
    CardRank rank;
    CardSuit suit;
    char code[3];
    int face_up;

    /*
     * Every saved card line is tiny: card code + visibility flag.
     * Example: "JD 1" means jack of diamonds, visible.
     */
    if (fscanf(file, "%2s %d", code, &face_up) != 2) {
        return 0;
    }

    if ((face_up != 0 && face_up != 1) || !card_parse_code(code, &rank, &suit)) {
        return 0;
    }

    card = card_create(rank, suit, face_up == 1);
    if (card == NULL) {
        return 0;
    }

    card_list_append(head, card);
    (*size)++;
    return 1;
}

static int read_line_payload(FILE *file, const char *expected_label,
                             char *buffer, size_t buffer_size)
{
    char label[32];
    size_t length;

    if (fscanf(file, "%31s", label) != 1 || strcmp(label, expected_label) != 0) {
        return 0;
    }

    /*
     * The value itself lives on the next line. That lets messages and last
     * commands contain spaces without us inventing escaping rules.
     */
    if (fgetc(file) == EOF || fgets(buffer, (int) buffer_size, file) == NULL) {
        return 0;
    }

    length = strlen(buffer);
    while (length > 0 && (buffer[length - 1] == '\n' || buffer[length - 1] == '\r')) {
        buffer[length - 1] = '\0';
        length--;
    }

    return 1;
}

static int write_line_payload(FILE *file, const char *label, const char *text)
{
    fprintf(file, "%s\n%s\n", label, text == NULL ? "" : text);
    return ferror(file) == 0;
}

static int save_one_foundation(FILE *file, int index, const FoundationPile *foundation)
{
    fprintf(file, "FOUNDATION %d %d %d %d\n",
            index,
            foundation->size,
            foundation->is_suit_assigned,
            foundation->is_suit_assigned ? (int) foundation->suit : -1);

    return write_card_list(file, foundation->top);
}

static int load_one_foundation(FILE *file, GameState *state)
{
    FoundationPile *foundation;
    int card_count;
    int foundation_index;
    int is_suit_assigned;
    int suit;
    int index;
    char label[32];

    if (fscanf(file, "%31s %d %d %d %d",
               label, &foundation_index, &card_count, &is_suit_assigned, &suit) != 5 ||
        strcmp(label, "FOUNDATION") != 0 ||
        foundation_index < 0 || foundation_index >= FOUNDATION_COUNT ||
        card_count < 0 || (is_suit_assigned != 0 && is_suit_assigned != 1)) {
        return 0;
    }

    if (is_suit_assigned && (suit < CARD_SUIT_CLUBS || suit > CARD_SUIT_SPADES)) {
        return 0;
    }

    foundation = &state->foundations[foundation_index];
    foundation->is_suit_assigned = is_suit_assigned;
    foundation->suit = is_suit_assigned ? (CardSuit) suit : CARD_SUIT_CLUBS;

    for (index = 0; index < card_count; index++) {
        if (!read_card_into_list(file, &foundation->top, &foundation->size)) {
            return 0;
        }
    }

    return 1;
}

static int load_one_card_group(FILE *file, const char *expected_label,
                               Card **head, int *size, int expected_index)
{
    int card_count;
    int group_index;
    int index;
    char label[32];

    if (fscanf(file, "%31s %d %d", label, &group_index, &card_count) != 3 ||
        strcmp(label, expected_label) != 0 ||
        group_index != expected_index ||
        card_count < 0) {
        return 0;
    }

    for (index = 0; index < card_count; index++) {
        if (!read_card_into_list(file, head, size)) {
            return 0;
        }
    }

    return 1;
}

bool game_state_save_to_file(const GameState *game_state, const char *filename,
                             char *message, size_t message_size)
{
    FILE *file;
    int index;

    if (game_state == NULL || filename == NULL || filename[0] == '\0') {
        set_message(message, message_size, "Missing save filename.");
        return false;
    }

    file = fopen(filename, "w");
    if (file == NULL) {
        set_message(message, message_size, "Could not open save file.");
        return false;
    }

    /*
     * Version marker first. If we change this format later, old save files can
     * fail cleanly instead of being half-loaded as nonsense.
     */
    fprintf(file, "YUKON_STATE 1\n");
    fprintf(file, "PHASE %s\n", phase_to_text(game_state->phase));
    fprintf(file, "SHOW_ALL %d\n", game_state->startup_show_all ? 1 : 0);
    fprintf(file, "TIMER_ELAPSED %d\n", game_timer_elapsed_seconds(game_state));
    fprintf(file, "TIMER_RUNNING %d\n", game_state->timer_is_running ? 1 : 0);
    write_line_payload(file, "LAST_COMMAND", game_state->last_command);
    write_line_payload(file, "MESSAGE", game_state->message);

    fprintf(file, "DECK 0 %d\n", game_state->deck.size);
    if (!write_card_list(file, game_state->deck.top)) {
        fclose(file);
        set_message(message, message_size, "Could not write deck to save file.");
        return false;
    }

    for (index = 0; index < TABLEAU_COLUMNS; index++) {
        fprintf(file, "TABLEAU %d %d\n", index, game_state->tableau[index].size);
        if (!write_card_list(file, game_state->tableau[index].top)) {
            fclose(file);
            set_message(message, message_size, "Could not write tableau to save file.");
            return false;
        }
    }

    for (index = 0; index < FOUNDATION_COUNT; index++) {
        if (!save_one_foundation(file, index, &game_state->foundations[index])) {
            fclose(file);
            set_message(message, message_size, "Could not write foundation to save file.");
            return false;
        }
    }

    if (fclose(file) != 0) {
        set_message(message, message_size, "Could not finish writing save file.");
        return false;
    }

    set_message(message, message_size, "OK");
    return true;
}

bool game_state_load_from_file(GameState *game_state, const char *filename,
                               char *message, size_t message_size)
{
    FILE *file;
    GameState loaded_state;
    GamePhase phase;
    char label[32];
    char version_text[32];
    int version;
    int show_all;
    int elapsed_seconds;
    int timer_running;
    int index;

    if (game_state == NULL || filename == NULL || filename[0] == '\0') {
        set_message(message, message_size, "Missing load filename.");
        return false;
    }

    file = fopen(filename, "r");
    if (file == NULL) {
        set_message(message, message_size, "Could not open save file.");
        return false;
    }

    game_state_init(&loaded_state);

    if (fscanf(file, "%31s %d", version_text, &version) != 2 ||
        strcmp(version_text, "YUKON_STATE") != 0 || version != 1 ||
        fscanf(file, "%31s %31s", label, version_text) != 2 ||
        strcmp(label, "PHASE") != 0 || !text_to_phase(version_text, &phase) ||
        fscanf(file, "%31s %d", label, &show_all) != 2 ||
        strcmp(label, "SHOW_ALL") != 0 || (show_all != 0 && show_all != 1) ||
        fscanf(file, "%31s %d", label, &elapsed_seconds) != 2 ||
        strcmp(label, "TIMER_ELAPSED") != 0 || elapsed_seconds < 0 ||
        fscanf(file, "%31s %d", label, &timer_running) != 2 ||
        strcmp(label, "TIMER_RUNNING") != 0 || (timer_running != 0 && timer_running != 1) ||
        !read_line_payload(file, "LAST_COMMAND", loaded_state.last_command, sizeof(loaded_state.last_command)) ||
        !read_line_payload(file, "MESSAGE", loaded_state.message, sizeof(loaded_state.message))) {
        fclose(file);
        game_state_destroy(&loaded_state);
        set_message(message, message_size, "Save file has invalid header data.");
        return false;
    }

    loaded_state.phase = phase;
    loaded_state.startup_show_all = show_all == 1;
    loaded_state.timer_elapsed_before_start = elapsed_seconds;
    loaded_state.timer_is_running = timer_running == 1;
    loaded_state.timer_started_at = loaded_state.timer_is_running ? time(NULL) : (time_t) 0;

    if (!load_one_card_group(file, "DECK", &loaded_state.deck.top, &loaded_state.deck.size, 0)) {
        fclose(file);
        game_state_destroy(&loaded_state);
        set_message(message, message_size, "Save file has invalid deck data.");
        return false;
    }

    for (index = 0; index < TABLEAU_COLUMNS; index++) {
        if (!load_one_card_group(file, "TABLEAU",
                                 &loaded_state.tableau[index].top,
                                 &loaded_state.tableau[index].size,
                                 index)) {
            fclose(file);
            game_state_destroy(&loaded_state);
            set_message(message, message_size, "Save file has invalid tableau data.");
            return false;
        }
    }

    for (index = 0; index < FOUNDATION_COUNT; index++) {
        if (!load_one_foundation(file, &loaded_state)) {
            fclose(file);
            game_state_destroy(&loaded_state);
            set_message(message, message_size, "Save file has invalid foundation data.");
            return false;
        }
    }

    fclose(file);

    /*
     * Only now do we replace the live game. If loading failed above, the player
     * keeps their old state instead of losing it to a bad save file.
     */
    game_state_destroy(game_state);
    *game_state = loaded_state;
    set_message(game_state->message, sizeof(game_state->message), "OK");
    set_message(message, message_size, "OK");
    return true;
}
