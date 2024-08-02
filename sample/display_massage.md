`displayMessage`関数にメッセージを渡す際の書き方にはいくつかの方法があります。それぞれの例を具体的に見てみましょう。


### 1. 文字列リテラルを直接ポインターとして渡す

```c
char *message = "Message.";
displayMessage(message);
```

- **説明**: 文字列リテラル `"Message."` を `char *` 型のポインターとして `displayMessage` 関数に渡します。リテラルは変更不可で、直接ポインターとして渡せます。


### 2. `strcpy`を使用して配列にコピーしてから渡す

```c
char message[100];
strcpy(message, "Message.");
displayMessage(message);
```

- **説明**: `message` 配列に文字列 `"Message."` をコピーしてから `displayMessage` 関数に渡します。`message` は配列で、変更可能なメモリ領域を持っています。


### 3. `snprintf`を使用してフォーマットされた文字列を配列に生成してから渡す

```c
char message[100];
snprintf(message, sizeof(message), "Message.");
displayMessage(message);
```

- **説明**: `snprintf` を使用してフォーマットされた文字列を `message` 配列に生成します。`message` 配列のサイズを指定して、バッファオーバーフローを防ぎます。生成された文字列を `displayMessage` 関数に渡します。

### 使い分けのポイント

- **文字列リテラル (`char *message = "Message.";`)**:
  - メッセージが固定で変更の必要がない場合に使用します。

- **`strcpy` (`char message[100]; strcpy(message, "Message.");`)**:
  - メッセージを配列にコピーすることで、変更可能なメッセージを使用する場合に適しています。

- **`snprintf` (`char message[100]; snprintf(message, sizeof(message), "Message.");`)**:
  - フォーマットされた文字列や変数を含むメッセージを生成する場合に適しています。バッファのサイズを指定することで、安全に文字列を生成できます。


### 例

以下に、これらの方法を含む簡単な例を示します。

```c
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
```

この例では、`displayMessage` 関数に対して、異なる方法でメッセージを渡していることがわかります。どの方法を使用するかは、特定の要件や状況に応じて選択してください。
