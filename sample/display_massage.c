#include <stdio.h>
#include <string.h> // strcpy関数のために必要
#include <unistd.h> // usleep関数のために必要

#define RESET "\x1b[0m"
#define MAGENTA "\x1b[35m"

// 文字列を１文字ずつ表示
void displayMessage(char *message) {
    int i = 0;
    while (message[i] != '\0') {
        printf("%c", message[i]);
        fflush(stdout);
        usleep(100000); // 100,000マイクロ秒（0.1秒）待つ
        i++;
    }
}

int main() {
    // 文字列リテラルを直接渡す
    char *message1 = "Message 1.\n";
    displayMessage(message1);

    // 配列にコピーしてから渡す
    char message2[100];
    strcpy(message2, "Message 2.\n");
    displayMessage(message2);

    // フォーマットされた文字列を配列に生成してから渡す
    char message3[100];
    snprintf(message3, sizeof(message3), "%sMessage 3.%s\n", MAGENTA, RESET);
    displayMessage(message3);

    return 0;
}

