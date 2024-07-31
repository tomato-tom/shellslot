// リールを追加して３つに

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>

#define SYMBOLS_COUNT   12
#define VISIBLE_SYMBOLS 3
#define REELS_COUNT     3
#define SPIN_DELAY      100000 // マイクロ秒単位 (0.1秒)
#define WINNING_LINES 5

#define CLEAR       "\033[2J\033[H"
#define CLEAR_LINE  "\r\033[2K"
#define SHOW_CURSOR "\e[?25h"
#define HIDE_CURSOR "\e[?25l"
#define BLUE        "\033[34m"
#define MAGENTA     "\033[35m"
#define RESET       "\033[39m"


const char* symbols[SYMBOLS_COUNT] = {"🦞", "🐧", "🌛", "🐟", "🦑", "🍒", "🐙", "🐟", "🍺", "🐋", "👻", "🍒"};
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

void displayReels(int positions[REELS_COUNT][VISIBLE_SYMBOLS], int spinning[REELS_COUNT]) {
    // positions
    // スピン中　( 1 2 )
    // スピン完　(0 1 2)
    int visible = 5; // 空白含む各リールのコマ数

    // 回転中
    printf(CLEAR);
    printf("Shell Slots\n\n");
    printf(BLUE);
    printf("\t┌────┐ ┌────┐ ┌────┐\n");

    for (int v = 0; v < visible; v++) {
        printf("\t");
        for (int reel = 0; reel < REELS_COUNT; reel++) {
            if (spinning[reel] && (v % 2)) {
                printf("│ %s │ ", symbols[positions[reel][v / 2 + 1]]); // スピン中奇数ライン(1, 3)
            } else if (!spinning[reel] && !(v % 2)) {
                printf("│ %s │ ", symbols[positions[reel][v / 2]]); // 停止中偶数ライン(0, 2, 4)
            } else {
                printf("│ 　 │ ");  // シンボル間の空白
            }
        }
        printf("\n");
    }

    printf("\t└────┘ └────┘ └────┘\n");
    printf(RESET);
    // リールの状態によって停止キー表示の色を変更
    printf("\t  %s(j)%s    %s(k)%s    %s(l)%s\n",
        spinning[0] ? MAGENTA : BLUE, RESET,
        spinning[1] ? MAGENTA : BLUE, RESET,
        spinning[2] ? MAGENTA : BLUE, RESET);

    usleep(SPIN_DELAY);
    
    // 回転完了
    printf(CLEAR);
    printf("Shell Slots\n\n");
    printf(BLUE);
    printf("\t┌────┐ ┌────┐ ┌────┐\n");

    for (int v = 0; v < visible; v++) {
        printf("\t");
        for (int reel = 0; reel < REELS_COUNT; reel++) {
            if (!(v % 2)) {
                printf("│ %s │ ", symbols[positions[reel][v / 2]]); // 停止中偶数ライン(0, 2, 4)
            } else {
                printf("│ 　 │ ");  // シンボル間の空白
            }
        }
        printf("\n");
    }

    printf("\t└────┘ └────┘ └────┘\n");
    printf(RESET);
    printf("\t  %s(j)%s    %s(k)%s    %s(l)%s\n\n",
        spinning[0] ? MAGENTA : BLUE, RESET,
        spinning[1] ? MAGENTA : BLUE, RESET,
        spinning[2] ? MAGENTA : BLUE, RESET);
}

void spinReels(int positions[REELS_COUNT][VISIBLE_SYMBOLS], int offsets[REELS_COUNT], int spinning[REELS_COUNT]) {
    // いずれかのリール回転の有効中はループ継続
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
            printf("\a");
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
        }
    }
    return winningLines;
}

int main() {
    srand(time(NULL));
    disableEcho();  // エコーを無効化
    printf(HIDE_CURSOR);

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
            for (int i = 0; i < 3; i++) { // 回転開始音
                printf("\a");
                fflush(stdout);
                usleep(100000);
            }

            for (int i = 0; i < REELS_COUNT; i++) {
                spinning[i] = 1;
            }
            spinReels(positions, offsets, spinning);

            // 全てのリールが停止した後に３つ揃い判定
            int winningLines = checkWinningLines(positions);
            if (winningLines > 0) {
                printf("\n%d ラインが揃いました！おめでとうございます！\n", winningLines);
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
