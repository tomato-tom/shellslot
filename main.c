#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "terminal_utils.h"

#define SYMBOLS_COUNT   12
#define VISIBLE_SYMBOLS 3
#define REELS_COUNT     3
#define SPIN_DELAY      30000 // microseconds (0.03 seconds)
#define WINNING_LINES   5

#define CLEAR       "\033[2J\033[H"
#define CLEAR_LINE  "\r\033[2K"
#define SHOW_CURSOR "\e[?25h"
#define HIDE_CURSOR "\e[?25l"
#define BLUE        "\033[34m"
#define MAGENTA     "\033[35m"
#define RESET       "\033[39m"

typedef enum {
    MANUAL,
    SEMIAUTO,
    AUTO
} Mode;

const char* symbols[SYMBOLS_COUNT] = {"🦞", "🐧", "🌛", "🐟", "🦑", "🍒", "🐙", "🐟", "🍺", "🐋", "👻", "🍒"};

// Parse command-line arguments to determine game mode
Mode parseArgs(int argc, char *argv[]) {
    if (argc > 1) {
        if (strcmp(argv[1], "--autostop") == 0 || strcmp(argv[1], "-s") == 0) {
            return SEMIAUTO;
        }
        if (strcmp(argv[1], "--auto") == 0 || strcmp(argv[1], "-a") == 0) {
            return AUTO;
        }
        // Invalid argument handling
        printf("Invalid argument: %s\n", argv[1]);
        printUsage();
        exit(EXIT_FAILURE);
    }
    return MANUAL; // Default mode
}

// Display message character by character
void displayMessage(char *message) {
    int i = 0;
    while (message[i] != '\0') {
        printf("%c", message[i]);
        fflush(stdout);
        usleep(20000); // 0.02 seconds delay
        if (kbhit()) break; 
        i++;
    }
}

// Generate beep sound
void beep(int count) {
    for (int i = 0; i < count; i++) {
        printf("\a");
        fflush(stdout);
        usleep(150000);
    }
}

// Display instructions based on game mode
void displayModeInstructions(Mode mode) {
    switch (mode) {
        case MANUAL:
            printf("Starting in Manual mode.\n");
            printf("Spin Reels: (Space)\n");
            printf("Stop Reel : (j, k, l)\n");
            printf("Stop game : (q)\n");
            break;
        case SEMIAUTO:
            printf("Starting in Semi-automatic mode.\n");
            printf("Spin Reels: (Space)\n");
            printf("Stop game : (q)\n");
            break;
        case AUTO:
            printf("Starting in Automatic mode.\n");
            printf("Stop game : (q)\n");
            break;
    }
    printf("\n");

    char *message = "Press any key to start the game...";
    displayMessage(message);
}

// Clean up and exit the game
void endGame() {
    restoreTerminal();
    printf(SHOW_CURSOR);
    printf(CLEAR_LINE);
    printf("\nExiting the program\n");
    exit(0);
}

// Check for winning combinations
int checkWinningLines(int positions[REELS_COUNT][VISIBLE_SYMBOLS]) {
    int winningLines = 0;
    const int lines[WINNING_LINES][3][2] = {
        {{0,0}, {1,0}, {2,0}},  // Top row
        {{0,1}, {1,1}, {2,1}},  // Middle row
        {{0,2}, {1,2}, {2,2}},  // Bottom row
        {{0,0}, {1,1}, {2,2}},  // Diagonal from top-left to bottom-right
        {{0,2}, {1,1}, {2,0}}   // Diagonal from bottom-left to top-right
    };

    for (int i = 0; i < WINNING_LINES; i++) {
        if (symbols[positions[lines[i][0][0]][lines[i][0][1]]] ==
            symbols[positions[lines[i][1][0]][lines[i][1][1]]] &&
            symbols[positions[lines[i][1][0]][lines[i][1][1]]] ==
            symbols[positions[lines[i][2][0]][lines[i][2][1]]]) {
            winningLines++;
            printf("Winning line with %s: ", symbols[positions[lines[i][0][0]][lines[i][0][1]]]);
            switch(i) {
                case 0: printf("Top row\n"); break;
                case 1: printf("Middle row\n"); break;
                case 2: printf("Bottom row\n"); break;
                case 3: printf("Diagonal from top-left to bottom-right\n"); break;
                case 4: printf("Diagonal from bottom-left to top-right\n"); break;
            }
            beep(3);
        }
    }
    return winningLines;
}

// Display game header
void displayHeader() {
    printf("\n\t   %sSHELL SLOT%s\n", MAGENTA, RESET);
    printf("\t❍ ❍  ❍   ❍  ❍ ❍ ❍❍❍❍\n\n");
}

// Display stop buttons for reels
void displayButton(int spinning[REELS_COUNT]) {
    printf("\t %s(j)%s    %s(k)%s    %s(l)%s\n\n\n",
        spinning[0] ? MAGENTA : BLUE, RESET,
        spinning[1] ? MAGENTA : BLUE, RESET,
        spinning[2] ? MAGENTA : BLUE, RESET);
}

// Display the current state of the reels
void displayReels(int positions[REELS_COUNT][VISIBLE_SYMBOLS], int spinning[REELS_COUNT]) {
    int reel_display_lines = 5; // Number of lines per reel, including spaces

    for (int stop = 0; stop < 2; stop++) {
        printf(CLEAR);
        displayHeader();
        printf(BLUE);
        printf("\t┌────┐ ┌────┐ ┌────┐\n");

        for (int line = 0; line < reel_display_lines; line++) {
            printf("\t");
            for (int reel = 0; reel < REELS_COUNT; reel++) {
                if (!stop && spinning[reel] && (line % 2)) {
                    printf("│ %s │ ", symbols[positions[reel][line / 2 + 1]]);
                } else if ((stop && spinning[reel] && !(line % 2)) || (!spinning[reel] && !(line % 2))) {
                    printf("│ %s │ ", symbols[positions[reel][line / 2]]);
                } else {
                    printf("│ 　 │ ");  // Space between symbols
                }
            }
            printf("\n");
        }

        printf("\t└────┘ └────┘ └────┘\n");
        printf(RESET);
        displayButton(spinning);

        usleep(SPIN_DELAY);
    }
}

// Main game logic for spinning reels
void spinReels(int mode,
               int positions[REELS_COUNT][VISIBLE_SYMBOLS],
               int offsets[REELS_COUNT],
               int spinning[REELS_COUNT]) {

    while (spinning[0] || spinning[1] || spinning[2]) {
        for (int reel = 0; reel < REELS_COUNT; reel++) {
            if (spinning[reel]) {
                offsets[reel] = (offsets[reel] - 1 + SYMBOLS_COUNT) % SYMBOLS_COUNT;
                for (int i = 0; i < VISIBLE_SYMBOLS; i++) {
                    positions[reel][i] = (offsets[reel] + i) % SYMBOLS_COUNT;
                }
            }
        }

        displayReels(positions, spinning);
        usleep(SPIN_DELAY);

        if (kbhit()) {
            beep(1);
            char c = getchar();
            if (c == 'q') endGame();
            else if (c == 'j' && mode == MANUAL) spinning[0] = 0;
            else if (c == 'k' && mode == MANUAL) spinning[1] = 0;
            else if (c == 'l' && mode == MANUAL) spinning[2] = 0;
        } else if (mode != MANUAL) {
            for (int reel = 0; reel < REELS_COUNT; reel++) {
                if (spinning[reel] && !(rand() % 5)) {
                    spinning[reel] = 0;
                    beep(1);
                    break;
                }
            }
        }
    }
    displayReels(positions, spinning);
}

// Main game loop
void startGame(Mode mode) {
    int positions[REELS_COUNT][VISIBLE_SYMBOLS] = {{0, 1, 2}, {0, 1, 2}, {0, 1, 2}};
    int offsets[REELS_COUNT] = {0, 0, 0};
    int spinning[REELS_COUNT] = {0, 0, 0};
    char c;
    char message[100];

    displayReels(positions, spinning);
    displayModeInstructions(mode);

    c = getchar();
    printf("%sGame started! You pressed '%c'.\n", CLEAR_LINE, c);
    usleep(200000);

    while (1) {
        if (mode == AUTO) {
            usleep(100000);
            if (kbhit()) {
                c = getchar();
            }
        } else {
            c = getchar();
        }

        if (c == 'q') break;                     

        if (mode == AUTO || c == ' ') {
            beep(2);
            for (int i = 0; i < REELS_COUNT; i++) {
                spinning[i] = 1;
            }
            spinReels(mode, positions, offsets, spinning);

            int winningLines = checkWinningLines(positions);
            if (winningLines > 0) {
                snprintf(message, sizeof(message), "\n%s%d line(s) matched! Congratulations!%s\n", MAGENTA, winningLines, RESET);
                displayMessage(message);
            }
        }

        while (kbhit()) {
            getchar();
        }
    }
}

int main(int argc, char *argv[]) {
    Mode mode = parseArgs(argc, argv);

    srand(time(NULL));
    disableEcho();
    printf(HIDE_CURSOR);

    startGame(mode);
    endGame();

    return 0;
}
