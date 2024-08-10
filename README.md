# Shell Slot

ディレクトリ内で以下のコマンドを実行します
```
make
```

コンパイルしてできたファイル`slot`に以下の引数渡すと自動モードになります
```
Usage:
  ./slot                 : Manual mode(default)
  ./slot --autostop (-s) : Semi-automatic mode
  ./slot --auto     (-a) : Automatic mode
```

手動での遊び方
```
スペースキー: リール回転
j, k, l     : 各リールストップ
q           : 終了
``` 

MacbookAir6.1<br>
ArchLinux<br>
gcc (GCC) 14.1.1 20240720<br>
Good!<br>

Lenovo G560E<br>
Ubuntu server 24.04<br>
gcc (Ubuntu 13.2.0-23ubuntu4) 13.2.0<br>
SSH接続: 動作OKだが描画のチラツキあり<br>

一部の環境ではビープ音`printf("\a")`ならないかもしれません

<br>


### 追加機能?

a. 統計情報:
プレイ回数、獲得コインなどの統計を表示する。

b. ボーナスゲーム:
特定の条件でボーナスゲームに移行する機能を追加する。

c. 役のフラグ
抽選してボーナス、小役などのフラグたてる。
    リールの動きを制御
    - フラグ立ってる場合は該当絵柄が揃いやすい
    - フラグ立ってない場合は絵柄が揃わない

d. リールのすべり
ストップボタンを押したときにランダムなリールのスベリ等のリール制御


