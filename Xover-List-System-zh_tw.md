# Xover 列表系統

Xover 列表系統是 MapleBBS 3.x 中所大量使用的列表顯示系統。

不同於 Pirate BBS 原本的 `i_read` 列表系統只能用在文章與信件列表，Xover 列表系統能夠用來顯示任何的列表。

## 文章概覽

- [Pirate BBS、PttBBS、MapleBBS 3 的列表顯示函數比較](https://github.com/ccns/dreambbs/wiki/Xover-List-System-zh_tw#pirate-bbspttbbsmaplebbs-3-%E7%9A%84%E5%88%97%E8%A1%A8%E9%A1%AF%E7%A4%BA%E5%87%BD%E6%95%B8%E6%AF%94%E8%BC%83) 一節使用歷史比較的方法概覽 Xover 列表系統。

- [MapleBBS 3 與 DreamBBS v3 的 Xover callback key value 的分配](https://github.com/ccns/dreambbs/wiki/Xover-List-System-zh_tw#maplebbs-3-%E8%88%87-dreambbs-v3-%E7%9A%84-xover-callback-key-value-%E7%9A%84%E5%88%86%E9%85%8D) 一節比較了 MapleBBS 3 與 DreamBBS 的用於 Xover callback 列表的 key value 分配上的差異。

- [MapleBBS 3 與 DreamBBS v3 的 Xover 特殊值](https://github.com/ccns/dreambbs/wiki/Xover-List-System-zh_tw#maplebbs-3-%E8%88%87-dreambbs-v3-%E7%9A%84-xover-%E7%89%B9%E6%AE%8A%E5%80%BC) 一節說明了 MapleBBS 3 與 DreamBBS 中的 Xover 系統使用到的特殊數值。

- [DreamBBS v3 的 Xover callback 指令連鎖機制](https://github.com/ccns/dreambbs/wiki/Xover-List-System-zh_tw#dreambbs-v3-%E7%9A%84-xover-callback-%E6%8C%87%E4%BB%A4%E9%80%A3%E9%8E%96%E6%A9%9F%E5%88%B6) 一節說明了 DreamBBS v3 新增的複合型指令的效果疊加機制。

## Pirate BBS、PttBBS、MapleBBS 3 的列表顯示函數比較

本文所提到的 BBS 系統間的演化關係
- Pirate BBS 是 PttBBS 和 MapleBBS 3.x 的共同祖先
    - MapleBBS 2.36 是 PttBBS 和 MapleBBS 3.x 的最後共同祖先
        - MapleBBS-itoc 是 MapleBBS 3.10 的後代，架構上屬於 MapleBBS 3.x
    - MapleBBS 2.36 也是 WindTop BBS 2.3x 和 MapleBBS 3.x 的最後共同祖先
        - WindTop BBS 3.0x 是在 WindTop BBS 2.3x 基礎上，加入 MapleBBS 3.0x 的元素改版而來，主要架構已屬於 MapleBBS 3.x
            - DreamBBS 2010 是 WindTop BBS 3.0x 的後代分支，因此主要架構也屬於 MapleBBS 3.x
                - 現在的 DreamBBS v1 及更新版本是 DreamBBS 2010 的後代，架構上也屬於 MapleBBS 3.x

### 列表主函數
　                  | Pirate BBS          | PttBBS             | MapleBBS 3
 :---               | ---                 | ---                | ---          
列表主程式檔         | bbs/read.c          | mbbsd/read.c       | maple/xover.c
列表主函數           | `i_read()`          | `i_read()`         | `xover()`
用途                | 文章與信件列表       | 文章與信件列表      | 大部分列表
其它列表的處理方法   | 寫新的列表處理函數   | 寫新的列表處理函數  | 寫新的列表顯示函數，用 `xo_cursor()` 處理游標位置 <br> - DreamBBS v3 不再使用 `xo_cursor()`，將其移除

### 列表主函數與其參數

#### Pirate BBS (v1.9)
```c
i_read(direct, dotitle, doentry, rcmdlist)
char *direct ;
int (*dotitle)() ;
char *(*doentry)() ;
struct one_key *rcmdlist ;
```

#### PttBBS (r4903)
```c
void
i_read(int cmdmode, const char *direct, void (*dotitle) (),
       void (*doentry) (), const onekey_t * rcmdlist, int bidcache)
```
- 多出參數 `cmdmode` 和 `bidcache`

#### MapleBBS 3 (取自 DreamBBS v2.0)
```cpp
void
xover(int cmd)
```
- 參數 `cmdmode` 併入 `XZ::mode`
- 參數 `direct` 併入 `XZ::dir`
- 參數 `dotitle()` 和 `doentry()` 放入 callback list
- 參數 `rcmdlist` 併入 `XZ::cb`
    - DreamBBS v3: 改併入 `XO::cb`
- 無參數 `bidcache` (使用 bbstate + currbid)

### 其它列表的處理方法之範例

- Pirate BBS (v1.9) 使用者列表的顯示函數與其參數（無游標，只能換頁）
```c
printcuent(uentp)
struct user_info *uentp ;
```

- PttBBS (r4903) 使用者列表的游標及顯示函數與其參數
```c
static void
pickup(pickup_t * currpickup, int pickup_way, int *page,
       int *nfriend, int *myfriend, int *friendme, int *bfriend, int *badfriend)
```
8 個參數
```c
static void
draw_pickup(int drawall, pickup_t * pickup, int pickup_way,
	    int page, int show_mode, int show_uid, int show_board,
	    int show_pid, int myfriend, int friendme, int bfriend, int badfriend)
```
12 個參數

- MapleBBS 3 的 `xo_cursor()` 與其參數 (取自 DreamBBS v2.0)
```cpp
int
xo_cursor(
    int ch, int pagemax, int num,
    int *pageno, int *cur, int *redraw)
```
6 個參數

### 按鍵處理
　                     | Pirate BBS          | PttBBS             | MapleBBS 3
 :---                  | ---                 | ---                | ---          
按鍵處理函數            | `i_read()`          | `i_read_key()`     | `xover()`
Callback 列表資料結構   | `struct one_key[]`  | `onekey_t[126]`    | - `KeyFunc[]` <br> - `std::unordered_map<unsigned int, XoFunc>` (DreamBBS v3; C++)
Callback 列表結尾或條件 | `!one_key::fptr`    | (固定長度)         | - `KeyFunc::key == 'h'` <br> - `KeyFunc::first == 'h'` (DreamBBS v3; C) <br> - `std::unordered_map::end()` (DreamBBS v3; C++)
Callback 取得方法　   　| Loop/O(n)            | Direct index/O(1) | - Loop/O(n) <br> - Hash table/O(1) (DreamBBS v3; C++)

### 游標處理
　                      | Pirate BBS                | PttBBS                          | MapleBBS 3
 :---                   | ---                       | ---                             | ---
嘗試取得已載入列表的游標 | `getkeep()`               | `getkeep()`                     | `xo_get()`
游標資料結構儲存空間取得 | (`malloc()`)              | (`malloc()`)                    | `xo_new()`
游標資料結構            | `struct keeploc`           | `keeploc_t`                     | `XO`
游標紀錄資料結構        | `struct keeploc *`         | `struct keepsome *`             | `XO *`
游標紀錄儲存            | `struct keeploc *keeplist` | `struct keeploc *keeplist`      | `XO *xo_root`
游標紀錄儲存可見度      | Function-scope global      | Function-scope global           | File-scope Global
游標紀錄資料結構類型    | Singly linked list         | Singly linked block <br> 以 `KEEPSLOT` (`10`) 為 1 block | Singly linked list
游標資料取得方法        | Loop/String comparison     | Loop/String hash                | Loop/String comparison

### 列表資料
　                      | Pirate BBS                    | PttBBS                       | MapleBBS 3
 :---                   | ---                           | ---                          | ---
列表資料結構             | `struct fileheader`           | `fileheader_t`               | (any; 另有 `HDR` 對應 `fileheader_t`)
列表資料儲存 (全域變數)  | `struct fileheader *files`    | `fileheader_t *headers`       | - 通常為 `char xo_pool[]` <br> - 通常為 `char *xo_pool_base` (DreamBBS v3)
列表資料儲存空間取得     | `calloc()` + `realloc()`      | `calloc()` + `realloc()`      | - 通常為 statically allocated <br> - 通常為 `mmap()` (DreamBBS v3)
列表資料取得             | `get_records()`              | `get_records_and_bottom()`    | 通常為 `xo_load()`
列表資料取得方法         | `lseek()` + `read()` 部分載入 | `lseek()` + `read()` 部分載入 | - 通常為 `lseek()` + `read()` 部分載入 <br> - 通常為 `mmap()` 映射整個列表 (DreamBBS v3)

### 重新載入與重繪的相關 macros (括號：無直接對應，替代的處理方式)
使用場合                 | Pirate BBS          | PttBBS             | MapleBBS 3
 :---                    | ---                 | ---                | ---
什麼都不做                | `DONOTHING`        | `DONOTHING`        | `XO_NONE`
切換列表資料檔，重新載入   | `NEWDIRECT`        | `NEWDIRECT`        | `XO_INIT`
列表資料檔有更動，重新載入 | (`FULLUPDATE`)     | `DIRCHANGED`       | (`XO_INIT`)
重新載入資料並重繪全畫面   | `FULLUPDATE`       | `FULLUPDATE`       | `XO_INIT`
重新載入資料並從列表頭重繪 | `PARTUPDATE`       | `PARTUPDATE`       | `XO_LOAD`
重繪全畫面                | (`FULLUPDATE`)     | (`FULLUPDATE`)     | `XO_HEAD`
從列表前說明處重繪        | (`FULLUPDATE`)     | (`FULLUPDATE`)     | `XO_NECK`
從列表頭重繪              | (`PARTUPDATE`)     | `PART_REDRAW`      | `XO_BODY`
重繪某項                  | (直接呼叫函數)     | (直接呼叫函數)      | - (直接呼叫函數) <br> - 有些分支有增加 `XO_ITEM`
從列表後說明處重繪        | (`PARTUPDATE`)     | (`PART_REDRAW`)    | - (`XO_BODY`) <br> - 有些分支有增加 `XO_KNEE`
重繪畫面底部              | (`PARTUPDATE`)     | `READ_REDRAW`      | `XO_FOOT` <br> - WindTop BBS 3.x: 只清除螢幕底部 (不會有 footer) <br> - MapleBBS-itoc: 在螢幕底部畫出 `XZ::feeter` <br> - DreamBBS v3: 呼叫 callback 列表對應 `XO_FOOT` 的函數
重繪畫面頂部              | (`FULLUPDATE`)     | `TITLE_REDRAW`     | - (`XO_HEAD`) <br> - `XR_PART_HEAD + key` (DreamBBS v3)
重新載入資料但不重繪      | (`PARTUPDATE`)      | `HEADERS_RELOAD`   | - (直接操作資料結構重新載入) <br> - `XR_PART_LOAD + key` (DreamBBS v3)

### 列表操作的相關 macros (括號：無直接對應，替代的處理方式)
使用場合                 | Pirate BBS          | PttBBS                    | MapleBBS 3
 :---                    | ---                 | ---                       | ---
指定某功能需要動態載入    | (無)                | (無)　                     | `cmd \| XO_DL`
將游標放到最尾項          | (直接操作)          | (直接操作)                 | - (直接操作: `xo->pos = XO_TAIL`) <br> - `XO_MOVE + XO_TAIL` (DreamBBS v3 起支援)
移動游標                 | (直接操作)           | `READ_NEXT` & `READ_PREV` | - `XO_MOVE + pos` <br> - `XO_MOVE + XO_REL + diff` (DreamBBS v3 起支援)
移動游標 (頭尾循環)       | (無)                | (無)                      | - `XO_MOVE + XO_WRAP + pos` <br> - `XO_MOVE + XO_WRAP + XO_REL + diff` (DreamBBS v3 起支援)
翻頁                     | (直接操作)          | (直接操作)                 | - `XO_MOVE + pos ± XO_TALL` <br> - `XO_MOVE + XO_REL ± XO_TALL` (DreamBBS v3 起支援) <br> - 尾項上捲: `XO_MOVE + XO_REL - ((xo->max-1 - xo->top) % XO_TALL + 1)` (DreamBBS v3)
翻頁 (頭尾循環)           | (無)                | (無)                      | - `XO_MOVE + XO_WRAP + pos ± XO_TALL` <br> - 尾項下捲: `xo->top = 0, XR_BODY + XO_MOVE + XO_WRAP + XO_REL + BMIN(xo->max, XO_TALL)` (DreamBBS v3) <br> - 首尾項上捲: `XO_MOVE + XO_WRAP + XO_REL - ((xo->max-1 - xo->top) % XO_TALL + 1)` (DreamBBS v3)
捲動列表                 | (無)                | (無)                      | - `XO_MOVE + XO_SCRL + pos` (DreamBBS v3 新增) <br> - `XO_MOVE + XO_SCRL + XO_REL + diff` (DreamBBS v3 新增)
捲動列表 (頭尾循環)       | (無)                | (無)                      | - `XO_MOVE + XO_WRAP + XO_SCRL + pos` (DreamBBS v3 新增) <br> - `XO_MOVE + XO_WRAP + XO_SCRL + XO_REL + diff` (DreamBBS v3 新增)
切換列表                 | (無)                | (無)                      | - `XZ_<ZONE>` = `XO_ZONE + zone` <br> - `XO_ZONE + XO_WRAP + zone` (DreamBBS v3 起支援) <br> - `XO_ZONE + XO_REL + diff` (DreamBBS v3 起支援) <br> - `XO_ZONE + XO_WRAP + XO_REL + diff` (DreamBBS v3 起支援)
回到上層列表             | (無)                | (無)                      | (無；有 `XO_LAST`，但未實作)
離開列表                 | `DOQUIT`            | `DOQUIT`                  | `XO_QUIT`

## MapleBBS 3 與 DreamBBS v3 的 Xover callback key value 的分配

### 輸入按鍵的值
範圍或對應的 bit mask                | 相關 macro         | 功能                          | 註解
 :---                               | ---                | ---                           | ---
`0x00000000` - `0x0000001f`         | `Ctrl()`           | `Ctrl-` 一般按鍵              |
`0x00000020` - `0x000000ff`         | (無)               | 一般按鍵                      |
`0x00000100` - `0x00001fff`         | (無)               | 傳統特殊按鍵 (Phoenix BBS (?)) | DreamBBS v1 起恢復使用
`0x00002000` - `0x000020ff`         | `Meta()`/`Esc()`   | `Esc-`/`Alt-` 一般按鍵        | Maple-itoc 不使用
`0x00002100` - `0x00003fff`         | `Meta()`/`Esc()`   | `Esc-`/`Alt-` 特殊按鍵        | DreamBBS v1 起新增
`0x00000060` (mask)                 | `Shift()`/`Ctrl()` | 特殊按鍵的 `Shift-`/`Ctrl-`   | DreamBBS v1 起新增
　                                  | `Ctrl(key)`        | `0x00`: `Ctrl-`               | Mask 後變 `0x00`
　                                  | `Shift(Ctrl(key))` | `0x20`: `Shift-Ctrl-`         | Mask 後變 `0x20`
　                                  | `key`              | `0x40`: 正常                  | Mask 後變 `0x40`
　                                  | `Shift(key)`       | `0x60`: `Shift-`              | Mask 後變 `0x60`
`0x00004000` - `0x7fffffff`         | `KEY_NONE` = `0x4000`| (不使用)                    | 保留給 Xover 列表系統使用
`0x80000000` - `0xffffffff`         | (無)               | MapleBBS 3 特殊按鍵 (負數)     | DreamBBS v1 起不使用，保留給 Xover 系統

### MapleBBS 3 的 Xover callback key value 的分配
範圍或對應的 bit mask                | 相關 macro         | 功能                         | 註解
 :---                               | ---                | ---                          | ---
`0x00000000` - `0x00003fff`         | (無)               | 按鍵輸入                     |
`0x00004000` - `0x0fffffff`         | (未使用)           | (未使用)                     |
`0x10000000` - `0x1fffffff-XO_TALL` | `XO_MODE`          | 畫面重繪、資料載入、離開列表  | 實際上只使用 9 到 11 個 (有些分支有 `XO_ITEM` 及 `XO_KNEE`，有些沒有)
`0x20000000-XO_TALL` - `0x1fffffff` | `XO_MOVE + pos` (`pos < 0`)            | 設定游標位置 (頭尾循環)      | Maple-itoc 修正前
`0x20000000` - `0x27ffffff-XO_TALL` | `XO_MOVE + pos`  (`pos >= 0`)          | 設定游標位置                | Maple-itoc 修正前
`0x28000000` - `0x3fffffff`         | `XO_MOVE + XO_WRAP + pos` (`pos >= 0`) | 設定游標位置 (頭尾循環)      | Maple-itoc 修正前
`0x20000000-XO_TALL` - `0x20800000` | `XO_MOVE + pos`                        | 設定游標位置                | Maple-itoc 修正後
`0x20800001` - `0x3fffffff`         | `XO_MOVE + XO_WRAP + pos`              | 設定游標位置 (頭尾循環)      | Maple-itoc 修正後 <br> - 循環發生時跳到頭尾 <br> - 循環發生時跳到循環後的對應項 (DreamBBS v3)
`0x40000000` - `0x7fffffff`         | `XO_ZONE + zone` (`zone >= 0`)         | 列表切換                    | Maple-itoc 只使用 14 個
`0x80000000` - `0xffffffff`         | `key \| XO_DL`     | 特殊按鍵 (負數) 或動態載入功能 | 不能以特殊按鍵做為 callback 列表的 key <br> - DreamBBS v1 起將特殊按鍵值恢復為傳統的正數，可做為 callback 列表的 key

### DreamBBS v3 的 Xover callback key value 的分配
範圍或對應的 bit mask                | 相關 macro         | 功能                         | 註解
 :---                               | ---                | ---                          | ---
`0x00000000` - `0x00003fff`         | `key`              | 按鍵輸入                      |
`0x00004000`                        | `XO_NONE` = `KEY_NONE` | 什麼都不做 <br> - 可視為不對應任何功能的按鍵 | 與 `Ctrl(' ')` (`'\0'`) 作區分
`0x00ffffff` (mask)                 | `XO_MOVE_MASK`     | 游標移動相關                  |
`0x001fffff` (mask)                 | `XO_POS_MASK`      | 取得游標目標位置              | 實際目標位置是 `pos - XO_MOVE` <br> `XO_MOVE` 是游標位置的 bias，設定為 `0x00100000`
　                                  | - `XR_* + key` <br> - `XZ_ZONE + XZ_* + key` | - 執行按鍵功能後進行畫面重繪及資料載入 <br> - 執行按鍵功能後進行列表相關操作 | 經過 `XO_MOVE_MASK` mask 後為 `0x00000000` - `0x00004000`
　                                  | - `XR_* + {XO_WRAP\|XO_SCRL\|XO_REL} + key` <br> - `XZ_ZONE + XZ_* + {XO_WRAP\|XO_SCRL\|XO_REL} + key` | (未使用)                     | 經過 `XO_MOVE_MASK` mask 後不為 `0x00000000` - `0x00004000`，而經過 `XO_POS_MASK` mask 後為 `0x00000000` - `0x00004000` <br> - 不需要操作游標的雜項功能放這一區
　                                  | - `XR_* + move` <br> - `XZ_ZONE + XZ_* + move` | - 設定列表游標位置後進行畫面重繪及資料載入 <br> - 設定 zone 游標位置後進行列表相關操作 | 經過 `XO_POS_MASK` mask 後為 `0x00004001` - `0x001fffff` <br> 這限制了游標的移動範圍為 `-0x000fbfff` (-1032191‬) - `0x000fffff` (1048575‬)
`0x00200000` (mask)                 | `XO_REL`           | 將游標位置解釋為相對位置      |
`0x00400000` (mask)                 | `XO_SCRL`          | - 將游標移動解讀為捲動列表 (無 `XZ_ZONE`) <br> - (未使用) (有 `XZ_ZONE`) |
`0x00800000` (mask)                 | `XO_WRAP`          | 讓游標位置頭尾循環            | 會跳到循環後的對應項而非頭尾
`0x3f000000` (mask)                 | `XO_REDO_MASK`     | 畫面重繪及資料載入相關        | 把 `XR_PART_*` macros `or` 起來的值
`0x01000000` (mask)                 | `XR_PART_LOAD`     | 重新載入列表資料             | - `XR_INIT` = `XR_PART_LOAD \| XR_HEAD` <br> - `XR_LOAD` = `XR_PART_LOAD \| XR_BODY` <br> - `XO_INIT` = `XR_INIT + XO_NONE` <br> - `XO_LOAD` = `XR_LOAD + XO_NONE`
`0x02000000` (mask)                 | `XR_PART_HEAD`     | 重繪畫面頂部                 | - `XR_HEAD` = `XR_PART_HEAD \| XR_NECK` <br> - `XO_HEAD` = `XR_HEAD + XO_NONE`
`0x04000000` (mask)                 | `XR_PART_NECK`     | 重繪列表前說明               | - `XR_NECK` = `XR_PART_NECK \| XR_BODY` <br> - `XO_NECK` = `XR_NECK + XO_NONE`
`0x08000000` (mask)                 | `XR_PART_BODY`     | 重繪列表                     | - `XR_BODY` = `XR_PART_BODY \| XR_KNEE` <br> - `XO_BODY` = `XR_BODY + XO_NONE`
`0x10000000` (mask)                 | `XR_PART_KNEE`     | 重繪列表後說明處             | - `XR_KNEE` = `XR_PART_KNEE \| XR_FOOT` <br> - `XO_KNEE` = `XR_KNEE + XO_NONE`
`0x20000000` (mask)                 | `XR_PART_FOOT`     | 重繪畫面底部                 | - `XR_FOOT` = `XR_PART_FOOT` <br> - `XO_FOOT` = `XR_FOOT + XO_NONE`
`0x7f000000` (mask)                 | `XO_ZONE_MASK`     | 列表操作相關                 | 把 `XZ_*` macros `or` 起來的值
`0x40000000` (mask)                 | `XZ_ZONE`          | 將操作解讀為列表操作          | `XO_ZONE` = `XZ_ZONE + XO_MOVE`
`0x01000000` (mask)                 | `XZ_INIT`          | 進行某 zone 的初始化工作      |
`0x02000000` (mask)                 | `XZ_FINI`          | 進行某 zone 的收拾工作        |
`0x04000000` (mask)                 | `XZ_BACK`          | 回到上次所在的 zone (未實作)  | `XO_LAST` = `(XZ_ZONE \| XZ_BACK) + XO_NONE`
`0x08000000` (mask)                 | `XZ_QUIT`          | 離開 `xover()` 函數          | `XO_QUIT` = `(XZ_ZONE \| XZ_QUIT) + XO_NONE`
`0x10000000` (mask)                 | `XZ_UNUSED4`       | (未使用)                     |
`0x20000000` (mask)                 | `XZ_UNUSED5`       | (未使用)                     |
`0x80000000` (mask)                 | `key \| XO_DL`     | 動態載入功能                 |
#### 新的 key value 分配的特點
- 除了頭尾循環邏輯改變，舊的 macro 使用方法仍然有效
- 將 `XO_MOVE` 重新定義為游標位置的 bias，避免游標位置為負時 flag bits 的改變
- 重繪畫面的各個 `XO_*` macros 之間的相對大小不變
- 允許畫面重繪/重新載入 (`XR_*`) 的各個部分自由組合 (尚未實作)
- 允許將畫面重繪/重新載入 (`XR_*`) 或列表操作 (`XZ_ZONE + XZ_*`)，與按鍵輸入或移動游標的操作組合表示
- 使用游標移動表示 zone 的切換

## MapleBBS 3 與 DreamBBS v3 的 Xover 特殊值
Macro             | 值                        | 功能                                  | 註解
 :---             | ---                       | ---                                   | ---
`XO_MODE`         | `0x10000000`              | 表示畫面重繪、資料載入、離開列表等操作  | DreamBBS v3 中已移除
`XO_NONE`         | - `0x10000000` <br> - `0x00004000` (DreamBBS v3)      | - 什麼都不作 <br> - 最小的被當作指令的 Xover key value |
`XR_<redo>`       | (多個)                    | 預先定義的畫面重繪及資料載入的組合動作  | DreamBBS v3 新增
`XR_PART_<redo>`  | (多個)                    | 畫面重繪及資料載入中的某部分            | DreamBBS v3 新增
`XO_MOVE`         | - `0x20000000` <br> - `0x00100000` (DreamBBS v3)      | - 表示游標移動 <br> - 游標移動的 bias (DreamBBS v3)
`XO_RSIZ`         | `256`                     | 列表資料的資料結構大小限制              | DreamBBS v3 起不使用
`XO_TALL`         | `(b_lines - 3)`           | 翻頁所跳行數                           | 非常數
`XO_MOVE_MAX`     | `(XO_POS_MASK - XO_MOVE)` | 可加在 `XO_MOVE` 上的最大值            | DreamBBS v3 新增
`XO_MOVE_MIN`     | `(XO_NONE + 1 - XO_MOVE)` | 可加在 `XO_MOVE` 上的最小值            | DreamBBS v3 新增
`XO_TAIL`         | - `(XO_MOVE - 999)` <br> - `(XO_WRAP - 1)` (DreamBBS v3)  | - 用來將游標 `XO::pos` 初始化到列表尾項 <br> - 用在 `XO_MOVE + XO_TAIL` 中，將游標移到列表尾項 (DreamBBS v3 增加支援) | 注意是 `TAIL`，與 `XO_TALL` 不同
`XO_ZONE`         | - `0x40000000` <br> - `(XZ_ZONE + XO_MOVE)` (DreamBBS v3) | 切換到某個列表 |
`XZ_ZONE`         | - `0x40000000`            | 將操作解釋為列表相關操作                | DreamBBS v3 新增
`XZ_BACK`         | - `0x100` <br> - `0x04000000` (DreamBBS v3) | - (未使用) <br> - 加在 `XZ_ZONE` 上，表示回到上次進入的 zone (DreamBBS v3)    |
`XZ_<zone>`       | `(XO_ZONE + <zone>)`      | 切換到某個 zone                        |
`XZ_INDEX_<zone>` | `<zone>`                  | Zone 的 index 值                      | DreamBBS v3 新增
`XZ_INDEX_MAX`    | `XZ_INDEX_MYFAVORITE`     | 最後一個 zone 的 index 值              | DreamBBS v3 新增
`XZ_COUNT`        | `(XZ_INDEX_MAX + 1)`      | Xover zone 的數量                     | DreamBBS v3 新增

## DreamBBS v3 的 Xover callback 指令連鎖機制
### 名詞說明
#### 連鎖
`i_read` 與 Xover 列表系統的 callback 函數都可以透過回傳值，呼叫下一個 callback，本文稱之為「連鎖」。

例如 `'i'` 對應的 callback 回傳 `'j'`，`'j'` 對應的 callback 回傳 `'k'`，`'k'` 對應的 callback 回傳 `XO_BODY`，`XO_BODY` 對應的 callback 回傳 `XO_FOOT`，`XO_FOOT` 對應的 callback 回傳 `XO_NONE`，稱為一個連鎖，可以記為 `'i' -> 'j' -> 'k' -> XO_BODY -> XO_FOOT -> XO_NONE`。

連鎖的結尾 callback 可以回傳游標移動指令、`XO_NONE` 指令、或沒有對應 callback 的指令。

#### 指令連鎖
透過回傳按鍵值而呼叫下一個 callback 的連鎖，稱為「指令連鎖」。

例如連鎖 `'i' -> 'j' -> 'k' -> XO_BODY -> XO_FOOT -> XO_NONE` 中就包含了指令連鎖 `'i' -> 'j' -> 'k'`。

#### 組合操作 - Redo 組合操作與 zone 組合操作
從 DreamBBS v3 開始，可以將畫面重繪/重新載入或列表操作，與按鍵輸入或游標移動的操作組合表示，因此需要特別的規則來處理帶有這些組合的指令連鎖。

其中畫面重繪/重新載入 (`XR_*`) 等操作，本文稱之為「redo-simple 操作」，與其組合的操作則稱為「redo 組合操作」；\
列表操作 (`XZ_ZONE + XZ_*`) 等操作，本文稱之為「zone-simple 操作」，與其組合的操作則稱為「zone 組合操作」。

「Redo 組合操作」與「zone 組合操作」，本文將其合稱為「組合操作」。

#### 簡單操作 - Redo-simple 操作、zone-simple 操作、pure 操作、以及 null 操作
不是組合操作的操作，本文稱為「簡單操作」，因此，前文中的「redo-simple 操作」與「zone-simple 操作」都屬於「簡單操作」。

代表單純的按鍵輸入或游標移動的簡單操作，本文稱之為「pure 操作」。\
之所以稱為「pure 操作」，是因為當按鍵連鎖中的各個 callback 的回傳值只有單純的按鍵輸入與游標移動操作時，回傳值之間不會互相影響。

Null 操作是 pure 操作的特例，是導致連鎖終止的單純操作，包含游標移動操作、`XO_NONE` 操作、以及沒有對應 callback 的值所代表的操作。

此外，DreamBBS v3 的切換列表操作 (`XZ_<zone>`)，是用游標移動達成的，本文將其歸類為 pure 操作中的 null 操作，而非 zone 組合操作；\
MapleBBS 3 原本的 Xover 列表系統的切換列表操作，不是使用游標移動達到的，但本文也將其看作游標操作。

### 組合指令連鎖規則
#### 通則
組合操作中的 redo-simple 部分及 zone-simple 部分，會累積起來，在指令連鎖結束時再執行。

其中，如果同時累積有 redo-simple 及 zone-simple 部分，則會先執行 zone-simple 部分，再執行 redo-simple 部分。

#### Pure-redo & Redo-pure 連鎖 -> pure-pure-redo-simple 連鎖
先後執行兩個操作的 pure 部分後，執行 redo-simple 操作。
#### Pure-zone & zone-pure 連鎖 -> pure-pure-zone-simple 連鎖
先後執行兩個操作的 pure 部分後，執行 zone-simple 操作。
#### Redo-redo 連鎖 -> pure-pure-redo-simple 連鎖
先後執行兩個操作的 pure 部分後，將與兩者組合的 redo-simple 操作一齊執行。

例如有 2 個 callback 函數：
- `'i'` 對應的 `func_info()` 會增加某項目的查詢次數，並且在列表後說明處畫東西，執行完後需要用 `XR_KNEE` 重繪
- `Meta('i')` 對應的 `func_info_full()` 會在列表內容處畫東西，需要用 `XR_BODY` 重繪，但需要呼叫 `func_info()` 來畫出剩下的部分

而使用者按 `'i'` 執行 `func_info()` 後，要從畫面中列表後說明處向下重繪；按 `Esc-i` 間接執行 `func_info()` 後，則要從畫面中列表內容處向下重繪。

可以這樣寫：
- 在 `func_info()` 中增加查詢次數，並 `return XR_KNEE + XO_NONE` (或 `return XO_KNEE`)
- `func_info_full()` 則有不同寫法：
    - 沒有連鎖規則時，要直接呼叫 `func_info(xo)` 再 `return XO_BODY`
        - DreamBBS v3 不使用 redo-redo 連鎖機制時，可以使用 `return XR_BODY | func_info(xo)`，但要確定 `func_info()` 不會回傳按鍵輸入值，否則需要連鎖規則才能處理 
    - 有連鎖規則的話，則可以直接 `return XR_BODY + 'i'`，這會間接呼叫 `func_info()`，並要求至少從畫面中列表內容處向下重繪

#### Zone-zone 連鎖 -> pure-pure-zone-simple 連鎖
先後執行兩個操作的 pure 部分後，將與兩者組合的 zone-simple 操作一齊執行。
#### Redo-zone & zone-redo 連鎖 -> pure-pure-zone-simple-redo-simple 連鎖
先後執行兩個操作的 pure 部分後，先執行 zone-simple 操作，再執行 redo-simple 操作。

在某些 zone-simple 操作後可以省略部分或全部的 redo-simple 操作。
