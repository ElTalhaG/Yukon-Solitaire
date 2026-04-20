#ifndef GAME_H
#define GAME_H

#include "card.h"

/* Fixed game limits taken directly from the project description. */
#define TABLEAU_COLUMNS 7
#define FOUNDATION_COUNT 4
#define DECK_CARD_COUNT 52
#define MAX_COMMAND_LENGTH 128
#define MAX_MESSAGE_LENGTH 256

/*
 * The program behaves differently before and after a game starts.
 * This enum lets the backend and UI know which command set is valid.
 */
typedef enum {
    GAME_PHASE_STARTUP,
    GAME_PHASE_PLAY
} GamePhase;

/*
 * The deck is stored as a linked list of cards.
 * "top" points to the first card in the current deck order.
 */
typedef struct {
    Card *top;
    int size;
} Deck;

/*
 * Each tableau column is also a linked list.
 * The exact interpretation of "top" stays consistent across the project:
 * it points to the first node in the column's current list order.
 */
typedef struct {
    Card *top;
    int size;
} TableauColumn;

/*
 * A foundation pile keeps normal linked-list information and also stores
 * whether a suit has already been assigned by placing the first Ace.
 */
typedef struct {
    Card *top;
    int size;
    int is_suit_assigned;
    CardSuit suit;
} FoundationPile;

/*
 * GameState collects everything needed to represent one full session.
 * Both the terminal version and the future GUI should work through this
 * shared structure instead of keeping separate copies of the rules state.
 */
typedef struct {
    /* Current deck used for loading, shuffling, and starting a game. */
    Deck deck;

    /* The seven Yukon tableau columns. */
    TableauColumn tableau[TABLEAU_COLUMNS];

    /* The four foundations used to build Ace through King by suit. */
    FoundationPile foundations[FOUNDATION_COUNT];

    /* Whether the program is waiting in STARTUP or actively playing. */
    GamePhase phase;

    /* Last raw command entered by the player in the terminal UI. */
    char last_command[MAX_COMMAND_LENGTH];

    /* Result of the last action, such as OK or an error message. */
    char message[MAX_MESSAGE_LENGTH];
} GameState;

/* Initialize one deck structure to a clean empty state. */
void deck_init(Deck *deck);

/* Initialize one tableau column to a clean empty state. */
void tableau_column_init(TableauColumn *column);

/* Initialize one foundation pile to a clean empty state. */
void foundation_pile_init(FoundationPile *foundation);

/* Initialize the complete game state when the program starts. */
void game_state_init(GameState *game_state);

/* Clear the tableau/foundation area so a new play session can begin. */
void game_state_reset_play_area(GameState *game_state);

#endif
