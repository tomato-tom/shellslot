#include "terminal_utils.h"
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

struct termios orig_termios;

void disableEcho() {
    struct termios new_termios;

    // 現在の端末設定を保存
    tcgetattr(STDIN_FILENO, &orig_termios);
    new_termios = orig_termios;

    // エコーをオフにする
    new_termios.c_lflag &= ~(ICANON | ECHO);

    // 即座に適用
    tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);
}

void restoreTerminal() {
    // 元の端末設定に戻す
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
}

int kbhit() {
    int ch;
    int oldf;

    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if(ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }

    return 0;
}
