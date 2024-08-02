#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>

#define SYMBOLS_COUNT 12
#define VISIBLE_SYMBOLS 3
#define SPIN_DELAY 100000 // マイクロ秒単位 (0.1秒)

#define CLEAR "\033[2J\033[H"
#define CLEAR_LINE "\r\033[2K"
#define SHOW_CURSOR "\e[?25h"
#define HIDE_CURSOR "\e[?25l"
#define BLUE "\033[34m"
#define RESET "\033[39m"

const char* symbols[SYMBOLS_COUNT] = {"🦞", "🐧", "🌛", "🐟", "🦑", "🍒", "🐙", "🐟", "🍺", "🐋", "👻", "🍒"};

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


void displayReel(int positions[VISIBLE_SYMBOLS], int spinning) {
    if (spinning) {
        printf(CLEAR);
        printf("Shell Slots\n\n");
        printf(BLUE);
        printf("\t┌────┐\n");
        printf("\t│    │\n");
        printf("\t│ %s │\n", symbols[positions[1]]);
        printf("\t│    │\n");
        printf("\t│ %s │\n", symbols[positions[2]]);
        printf("\t│    │\n");
        printf("\t└────┘\n\n");
        printf(RESET);
    }

    usleep(SPIN_DELAY);

    printf(CLEAR);
    printf("Shell Slots\n\n");
    printf(BLUE);
    printf("\t┌────┐\n");
    printf("\t│ %s │\n", symbols[positions[0]]);
    printf("\t│    │\n");
    printf("\t│ %s │\n", symbols[positions[1]]);
    printf("\t│    │\n");
    printf("\t│ %s │\n", symbols[positions[2]]);
    printf("\t└────┘\n\n");
    printf(RESET);
}

int spinReel(int offset) {
    int positions[VISIBLE_SYMBOLS];
    int spinning = 1;

    for (int i = 0; i < VISIBLE_SYMBOLS; i++) {
        positions[i] = (offset + i) % SYMBOLS_COUNT;
    }

    while (spinning) {
        // リールを表示
        displayReel(positions, spinning);
        //usleep(SPIN_DELAY);

        // 絵柄を下方向に回転
        offset == 0 ? (offset = SYMBOLS_COUNT - 1) : (offset--);
        for (int i = 0; i < VISIBLE_SYMBOLS; i++) {
            positions[i] = (offset + i) % SYMBOLS_COUNT;
        }

        // キー入力をチェック
        if (kbhit()) {
            if (getchar() == ' ') {
                spinning = 0;
                printf("\a");
                fflush(stdout);
            }
        }
        usleep(SPIN_DELAY);
    }

    return offset;
}

int main() {
    srand(time(NULL));
    printf(HIDE_CURSOR); // カーソル非表示

    char input;
    int positions[VISIBLE_SYMBOLS] = {0, 1, 2};
    int offset = 0;
    int spinning = 0;

    displayReel(positions, spinning);

    while (1) {
        printf("\rスペースキーでリール回転/停止、'q'で終了: ");
        fflush(stdout);  // 確実に出力されるようにバッファをフラッシュ

        while (!kbhit()) {
            usleep(100000);  // CPUの使用率を下げるため、短い休止を入れる
        }

        input = getchar();

        // キー入力をチェック
        if (input == 'q') {
            break;                     // ループを抜けてプログラムを終了
        } else if (input == ' ') {
            for (int i = 0; i < 3; i++) {
                printf("\a");
                fflush(stdout);
                usleep(100000);
            }
            offset = spinReel(offset); // リールを回転
        }

        // 入力バッファをクリア
        while (kbhit()) {
            getchar();
        }
    }

    printf(SHOW_CURSOR);
    printf(CLEAR_LINE);
    printf("プログラムを終了します。\n");
    return 0;
}
