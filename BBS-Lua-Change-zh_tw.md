# BBS-Lua Changelog

本文說明 BBS-Lua 的 DreamBBS 移植版本 (DlPatch 版) 對原版本的更動。

主程式：https://github.com/ccns/dreambbs/blob/master/so/bbslua.c

## BBS-Lua 0.119-DlPatch-1

### Bug Fixes
- 修正使用 <kbd>Ctrl</kbd>-<kbd>C</kbd> 結束程式時會輸出無意義字串的問題 ([28626e0a14](https://github.com/ccns/dreambbs/commit/28626e0a1422f85a91eb61a3bd4d9791139533b5))

### Bug Fixes for PttBBS
- `getdata()`/`getstr()`: 修正 `echo` = 8 時會斷線的問題 ([90e0464c9d](https://github.com/ccns/dreambbs/commit/90e0464c9d3d6d1f3dc14fa2e696608fa9de3e34))
- `getch()`/`kball()`: 修正 <kbd>Meta</kbd>-modified 的按鍵會被當成 <kbd>Esc</kbd> 鍵的問題 ([bf960173ea](https://github.com/ccns/dreambbs/commit/bf960173eaf42f8cfdeccd806b96556d59668907#diff-3cc4b761fdc37db96f8fc5de75e12ece404e431b5e503896976b77d3c71cb9cdR1006-R1010))

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
- `include/global.h` 中的 `KEY_*`, `Ctrl`, `Meta`, 和 `Shift` 等 macros
- `maple/visio.c` 中的 `vkey()`


## [WIP] BBS-Lua 0.119-DlPatch-2

### Bug Fixes
- 修正 BBS-Lua 的執行錯誤訊息未重設顏色而可能隱形的問題 ([6edaa29d72](https://github.com/ccns/dreambbs/commit/6edaa29d7222228433ee3d0e106c584acba1f289))

### Bug Fixes for Maple3
- (修正 DlPatch-1) 修正啟用 `BBSLUA_EXPOSED_VISIO_IDLE` 時，無法正確忽略已處理的 Telnet 命令的問題 ([44e454e363](https://github.com/ccns/dreambbs/commit/44e454e3632d6818daf475837d71cc2cf6fb00bc))
- (修正 DlPatch-1) 修正外加欄位隱形效果的 `NOECHO` (`NOECHO | VGET_STEALTH_NOECHO`) 未忽略預設字串的問題 ([eff2749402](https://github.com/ccns/dreambbs/commit/eff27494029a2fd2485ff19e28f6451b33b47a04))

### Bug Fixes for PttBBS
- (修正 DlPatch-1) 修正 LuaJIT 被強制停用的問題 ([a822fd54ab](https://github.com/ccns/dreambbs/commit/a822fd54ab79b2e39348d193062fa6f4b694593c))

### Other Changes

- `bl_k2s()`: 將 `'\0'` (<kbd>Ctrl</kbd>-<kbd>Shift</kbd>-<kbd>2</kbd>) 視為合法按鍵輸入
- 其它 coding style 上的調整
- (針對 Maple3) 將程式檔所在目錄改成 `so/`，編成執行時期載入函式庫
- 避免將 64-bit 數值截斷成 32-bit

### 已知在 Maple3 上的問題
- (DlPatch-1 起) 啟用 `BBSLUA_EXPOSED_VISIO_IDLE` 時，若系統採用時間戳式的閒置計算方式，更新時會錯誤歸零，使系統認為已閒置達一千多天