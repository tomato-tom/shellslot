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

void spinReel(int games) {
    int currentIndex = 0;

    printf("Games: %d\n", games);
    printf("リールを回転中... スペースキーを押して停止してください。\n");

    while (1) {
        printf("\r%s", symbols[currentIndex]);
        fflush(stdout);

        if (kbhit()) {
            char key = getchar();
            printf("\n");
            if (key == ' ') {
                printf("\a");
                break;
            } else if (key == 'q') {
                exit(0);
            }
        }

        currentIndex = (currentIndex + 1) % SYMBOLS_COUNT;
        usleep(SPIN_DELAY);
    }

    printf("リールが停止しました: %s\n", symbols[currentIndex]);
}

int main() {
    srand(time(NULL));
    int games = 0;

    while (1) {
        printf("\rスペースキーでリール回転: ");

        if (kbhit()) {
            char key = getchar();
            printf("\n");
            if (key == ' ') {
                printf("\a");
                games++;
                spinReel(games);
            } else if (key == 'q') {
                break;
            }
        }

        usleep(SPIN_DELAY);
    }
    return 0;
}
