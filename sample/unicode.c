// unicode 文字幅チェック
#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <wchar.h>
#include <locale.h>
#include <string.h>

int main() {
    setlocale(LC_ALL, "");

    const char *moji = "─";
    int byte_size = strlen(moji);
    
    wchar_t wmoji[] = L"─";
    int char_width = wcswidth(wmoji, 1);

    printf("Byte size of ─: %d\n", byte_size);
    printf("Display width of ─: %d\n", char_width);

    return 0;
}
