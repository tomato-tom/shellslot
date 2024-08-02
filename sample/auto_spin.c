// 自動プレイ

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>

#define GAMES           100
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

void beep(int count) {
    for (int i = 0; i < count; i++) {
        printf("\a");
        fflush(stdout);
        usleep(100000);
    }
}

void displayHeader() {
    printf("\n\t   %sSHELL SLOT%s\n", MAGENTA, RESET);
    printf("\t❍ ❍  ❍   ❍  ❍ ❍ ❍❍❍❍\n\n");
}

void displayButton(int spinning[REELS_COUNT]) {
    // リールの状態によって停止キー表示の色を変更
    printf("\t %s(j)%s    %s(k)%s    %s(l)%s    (Ctrl-c)で終了\n\n\n",
        spinning[0] ? MAGENTA : BLUE, RESET,
        spinning[1] ? MAGENTA : BLUE, RESET,
        spinning[2] ? MAGENTA : BLUE, RESET);
}

void displayReels(int positions[REELS_COUNT][VISIBLE_SYMBOLS], int spinning[REELS_COUNT]) {
    // 状態別のリール絵文字表示
    //
    // スピン中(stop: 0) スピンリール(spinning[reel])  奇数ライン (絵文字)
    // スピン中(stop: 0) 停止リール(!spinning[reel])   偶数ライン (絵文字)
    // スピン中(stop: 0) スピンリール(spinning[reel])  偶数ライン (空白)
    // スピン中(stop: 0) 停止リール(!spinning[reel])   奇数ライン (空白)
    //
    // スピン完(stop: 1) スピンリール(spinning[reel])  偶数ライン (絵文字)
    // スピン完(stop: 1) 停止リール(!spinning[reel])   偶数ライン (絵文字)
    // スピン完(stop: 1) スピンリール(spinning[reel])  奇数ライン (空白)
    // スピン完(stop: 1) 停止リール(!spinning[reel])   奇数ライン (空白)

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
        
        // 停止フラグ抽選
        for (int reel = 0; reel < REELS_COUNT; reel++) {
            if (spinning[reel]) {
                if (!(rand() % 5)) {
                    spinning[reel] = 0;
                    beep(1);
                    fflush(stdout);
                    break;
                }
            }
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
    printf(HIDE_CURSOR);

    int positions[REELS_COUNT][VISIBLE_SYMBOLS] = {{0, 1, 2}, {0, 1, 2}, {0, 1, 2}};
    int offsets[REELS_COUNT] = {0, 0, 0};
    int spinning[REELS_COUNT] = {0, 0, 0};
    char input;

    displayReels(positions, spinning);

    for (int game = 0; game < GAMES; game++) {

        usleep(10000);  // CPUの使用率を下げるため、短い休止を入れる

        for (int i = 0; i < REELS_COUNT; i++) {
            spinning[i] = 1;
        }
        spinReels(positions, offsets, spinning);

        // 全てのリールが停止した後に３つ揃い判定
        int winningLines = checkWinningLines(positions);
        if (winningLines > 0) {
            printf("\n%s%d ラインが揃いました！おめでとうございます！%s\n",MAGENTA, winningLines, RESET);
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
