# SuwaUI

C言語、C++、Zigで使えるGUIライブラリ（現在X11のみ）。\
このライブラリは１つのヘッダーのみで使えます。

A GUI library for C, C++, and Zig (currently X11 only).
This library can be used with just one header.

## 使い方 | Usage
### ウィンドウのみ | Only window
```sh
cc -DSUWAUI_IMPLEMENTS_SUWAWINDOW *.c ...
```

### ウィンドウ・ボタン・ラベル | Window・Button・Label
```sh
cc -DSUWAUI_IMPLEMENTS_SUWAWINDOW -DSUWAUI_IMPLEMENTS_SUWALABEL -DSUWAUI_IMPLEMENTS_SUWABUTTON *.c ...
```
