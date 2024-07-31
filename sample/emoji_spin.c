#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>

#define SYMBOLS_COUNT 6
#define SPIN_DELAY 100000 // マイクロ秒単位 (0.1秒)

const char* symbols[SYMBOLS_COUNT] = {"🍋", "🍒", "🍊", "🔔", "🍉", "🤡"};

int kbhit() {
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if(ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }

    return 0;
}

void spinReel() {
    int spinning = 1;
    int currentIndex = 0;

    printf("リールを回転中... スペースキーを押して停止してください。\n");

    while (spinning) {
        printf("\r%s", symbols[currentIndex]);
        fflush(stdout);

        usleep(SPIN_DELAY);

        currentIndex = (currentIndex + 1) % SYMBOLS_COUNT;

        if (kbhit()) {
            if (getchar() == ' ') {
                spinning = 0;
            }
        }
    }

    printf("\nリールが停止しました: %s\n", symbols[currentIndex]);
}

int main() {
    srand(time(NULL));
    spinReel();
    return 0;
}
