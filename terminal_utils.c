#include "terminal_utils.h"
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

struct termios orig_termios;

void disableEcho() {
    struct termios new_termios;

    // Save current terminal settings
    tcgetattr(STDIN_FILENO, &orig_termios);
    new_termios = orig_termios;

    // Turn off echo
    new_termios.c_lflag &= ~(ICANON | ECHO);

    // Apply changes immediately
    tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);
}

void restoreTerminal() {
    // Restore original terminal settings
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
}

int kbhit() {
    int ch;
    int oldf;

    // Get current file status flags
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    
    // Set non-blocking mode
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    // Restore original file status flags
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if(ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }

    return 0;
}

void printUsage() {
    printf("Usage:\n");
    printf("  ./slot                 : Manual mode\n");
    printf("  ./slot --autostop (-s) : Semi-automatic mode\n");
    printf("  ./slot --auto     (-a) : Automatic mode\n");
}
