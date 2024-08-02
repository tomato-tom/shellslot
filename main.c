#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "terminal_utils.h"

#define SYMBOLS_COUNT   12
#define VISIBLE_SYMBOLS 3
#define REELS_COUNT     3
#define SPIN_DELAY      30000 // マイクロ秒単位 (0.03秒)
#define WINNING_LINES   5

#define CLEAR       "\033[2J\033[H"
#define CLEAR_LINE  "\r\033[2K"
#define SHOW_CURSOR "\e[?25h"
#define HIDE_CURSOR "\e[?25l"
#define BLUE        "\033[34m"
#define MAGENTA     "\033[35m"
#define RESET       "\033[39m"

const char* symbols[SYMBOLS_COUNT] = {"🦞", "🐧", "🌛", "🐟", "🦑", "🍒", "🐙", "🐟", "🍺", "🐋", "👻", "🍒"};


void beep(int count) {
    for (int i = 0; i < count; i++) {
        printf("\a");
        fflush(stdout);
        usleep(150000);
    }
}

void displayHeader() {
    printf("\n\t   %sSHELL SLOT%s\n", MAGENTA, RESET);
    printf("\t❍ ❍  ❍   ❍  ❍ ❍ ❍❍❍❍\n\n");
}

void displayButton(int spinning[REELS_COUNT]) {
    // リールの状態によって停止キー表示の色を変更
    printf("\t %s(j)%s    %s(k)%s    %s(l)%s\n\n\n",
        spinning[0] ? MAGENTA : BLUE, RESET,
        spinning[1] ? MAGENTA : BLUE, RESET,
        spinning[2] ? MAGENTA : BLUE, RESET);
}

void displayReels(int positions[REELS_COUNT][VISIBLE_SYMBOLS], int spinning[REELS_COUNT]) {
    int reel_display_lines = 5; // 空白含む各リールのコマ数

    // リール表示
    for (int stop = 0; stop < 2; stop++) {
        printf(CLEAR);
        displayHeader();
        printf(BLUE);
        printf("\t┌────┐ ┌────┐ ┌────┐\n");

        // リール内の絵文字表示
        for (int line = 0; line < reel_display_lines; line++) {
            printf("\t");
            for (int reel = 0; reel < REELS_COUNT; reel++) {
                if (!stop && spinning[reel] && (line % 2)) {
                    printf("│ %s │ ", symbols[positions[reel][line / 2 + 1]]);
                } else if ((stop && spinning[reel] && !(line % 2)) || (!spinning[reel] && !(line % 2))) {
                    printf("│ %s │ ", symbols[positions[reel][line / 2]]);
                } else {
                    printf("│ 　 │ ");  // シンボル間の空白
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

void spinReels(int positions[REELS_COUNT][VISIBLE_SYMBOLS], int offsets[REELS_COUNT], int spinning[REELS_COUNT]) {
    // いずれかのリールのスピン有効中はループ継続
    while (spinning[0] || spinning[1] || spinning[2]) {
        for (int reel = 0; reel < REELS_COUNT; reel++) {
            if (spinning[reel]) {
                offsets[reel] = (offsets[reel] - 1 + SYMBOLS_COUNT) % SYMBOLS_COUNT;
                for (int i = 0; i < VISIBLE_SYMBOLS; i++) {
                    positions[reel][i] = (offsets[reel] + i) % SYMBOLS_COUNT;
                }
            }
        }

        // リール表示更新
        displayReels(positions, spinning);
        usleep(SPIN_DELAY);

        // 停止キー押されたか？
        if (kbhit()) {
            char key = getchar();
            if (key == 'j') spinning[0] = 0;
            else if (key == 'k') spinning[1] = 0;
            else if (key == 'l') spinning[2] = 0;
            beep(1);
            fflush(stdout);
        }
    }
    // すべてのリール停止後にもう一度更新して停止キーの色をリセット
    displayReels(positions, spinning);
}


int checkWinningLines(int positions[REELS_COUNT][VISIBLE_SYMBOLS]) {
    int winningLines = 0;
    const int lines[WINNING_LINES][3][2] = {
        {{0,0}, {1,0}, {2,0}},  // 上段
        {{0,1}, {1,1}, {2,1}},  // 中段
        {{0,2}, {1,2}, {2,2}},  // 下段
        {{0,0}, {1,1}, {2,2}},  // 左上から右下への斜め
        {{0,2}, {1,1}, {2,0}}   // 左下から右上への斜め
    };

    for (int i = 0; i < WINNING_LINES; i++) {
        if (symbols[positions[lines[i][0][0]][lines[i][0][1]]] ==
            symbols[positions[lines[i][1][0]][lines[i][1][1]]] &&
            symbols[positions[lines[i][1][0]][lines[i][1][1]]] ==
            symbols[positions[lines[i][2][0]][lines[i][2][1]]]) {
            winningLines++;
            printf("%s揃いライン: ", symbols[positions[lines[i][0][0]][lines[i][0][1]]]);
            switch(i) {
                case 0: printf("上段\n"); break;
                case 1: printf("中段\n"); break;
                case 2: printf("下段\n"); break;
                case 3: printf("左上から右下への斜め\n"); break;
                case 4: printf("左下から右上への斜め\n"); break;
            }
            beep(3);
        }
    }
    return winningLines;
}

int main() {
    srand(time(NULL));
    disableEcho();  // エコーを無効化
    printf(HIDE_CURSOR); // カーソル非表示

    int positions[REELS_COUNT][VISIBLE_SYMBOLS] = {{0, 1, 2}, {0, 1, 2}, {0, 1, 2}};
    int offsets[REELS_COUNT] = {0, 0, 0};
    int spinning[REELS_COUNT] = {0, 0, 0};
    char input;

    displayReels(positions, spinning);

    while (1) {
        printf("\rスペースキーでリール回転、停止キー(j, k, l)、'q'で終了: ");
        fflush(stdout);

        while (!kbhit()) {
            usleep(100000);  // CPUの使用率を下げるため、短い休止を入れる
        }

        input = getchar();

        // キー入力をチェック
        if (input == 'q') {
            break;                     // ループを抜けてプログラムを終了
        } else if (input == ' ') {
            beep(2);
            for (int i = 0; i < REELS_COUNT; i++) {
                spinning[i] = 1;
            }
            spinReels(positions, offsets, spinning);

            // 全てのリールが停止した後に３つ揃い判定
            int winningLines = checkWinningLines(positions);
            if (winningLines > 0) {
                printf("\n%s%d ラインが揃いました！おめでとうございます！%s\n",MAGENTA, winningLines, RESET);
            }
        }

        // 入力バッファをクリア
        while (kbhit()) {
            getchar();
        }
    }

    restoreTerminal();  // 元の端末設定に戻す
    printf(SHOW_CURSOR);
    printf(CLEAR_LINE);
    printf("プログラムを終了します。\n");
    return 0;
}
