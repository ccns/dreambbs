# BBS-Lua Changelog

本文說明 BBS-Lua 的 DreamBBS 移植版本 (DlPatch 版) 對原版本的更動。

主程式：https://github.com/ccns/dreambbs/blob/master/so/bbslua.c

## BBS-Lua 0.119-DlPatch-1

### Bug Fixes
- 修正使用 <kbd>Ctrl</kbd>-<kbd>C</kbd> 結束程式時會輸出無意義字串的問題

### Bug Fixes for PttBBS
- `getdata/getstr()`: 修正 `echo` = 8 時會斷線的問題
- `getch()`/`kball()`: 修正 <kbd>Meta</kbd>-modified 的按鍵會被當成 <kbd>Esc</kbd> 鍵的問題

### Other Changes

- 修正原始碼註解錯字
- `getch()`/`kball()`: 支援 <kbd>Shift</kbd>-<kbd>TAB</kbd> 特殊按鍵
- 使用 BitOp <http://bitop.luajit.org/> 取代已被棄用的 bitlib，不過還是可以使用 bitlib 編譯
- 向下支援 bitlib 各函式（如果使用 BitOp）
- 支援使用 LuaJIT <http://luajit.org/>
- `getch()`/`kball()`: 增加對 [<kbd>Ctrl</kbd>-/<kbd>Meta</kbd>-(<kbd>Alt</kbd>-/<kbd>Esc</kbd>-)/<kbd>Shift</kbd>-]modified 特殊按鍵的支援
- 其它 coding style 上的調整
- 支援 Maple3 系統
    - 仍需調整檔案開頭的編譯設定，因為這些設定是針對 DreamBBS 的

### 在未修改的 Maple3 上的已知問題 / 需要修改 Maple3 系統來修正的問題

#### 需要修改函式 `vget()`
- `getdata()`/`getstr()`:
    - 不能用 <kbd>Ctrl</kbd>-<kbd>C</kbd> 跳出
    - 在 `echo` = 0 時會畫出 `*` 代表輸入而不會完全隱藏
    - 在 `echo` = 0 時會忽略預設字串
    - 不支援 `echo` = 2 (小寫), 4 (數字) 等特殊模式

修改參考：
- `include/bbs.h` 中的 `*ECHO` 和 `VGET_*` macros
- `maple/visio.c` 中的 `vget()`

#### 需要修改函式 `vkey()`
- `getch()`/`kball()` 不支援特殊按鍵。

修改參考：
- `include/global.h` 中的 `KEY_`, `Ctrl`, `Meta`, 和 `Shift` 等 macros
- `maple/visio.c` 中的 `vkey()`