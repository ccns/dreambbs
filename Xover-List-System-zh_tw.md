# Xover 列表系統

Xover 列表系統是 MapleBBS 3.x 中所大量使用的列表顯示系統。

不同於 Pirate BBS 原本的 `i_read` 列表系統只能用在文章與信件列表，Xover 列表系統能夠用來顯示任何的列表。

## 文章概覽

- [Pirate BBS、PttBBS、MapleBBS 3 的列表顯示函數比較](https://github.com/ccns/dreambbs/wiki/Xover-List-System-zh_tw#pirate-bbspttbbsmaplebbs-3-%E7%9A%84%E5%88%97%E8%A1%A8%E9%A1%AF%E7%A4%BA%E5%87%BD%E6%95%B8%E6%AF%94%E8%BC%83) 一節使用歷史比較的方法概覽 Xover 列表系統。

- [MapleBBS 3 與 DreamBBS v3 的 Xover callback key value 的分配](https://github.com/ccns/dreambbs/wiki/Xover-List-System-zh_tw#maplebbs-3-%E8%88%87-dreambbs-v3-%E7%9A%84-xover-callback-key-value-%E7%9A%84%E5%88%86%E9%85%8D) 一節比較了 MapleBBS 3 與 DreamBBS 的用於 Xover callback 列表的 key value 分配上的差異。

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
其它列表的處理方法   | 寫新的列表處理函數   | 寫新的列表處理函數  | 寫新的列表顯示函數，用 `xo_cursor()` 處理游標位置
　                  |                     |                    | DreamBBS v3 不再使用 `xo_cursor()`，將其移除

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
Callback 列表資料結構   | `struct one_key[]`  | `onekey_t[126]`    | `KeyFunc[]`
　                      |                     |                    | `std::unordered_map<unsigned int, XoFunc>` (DreamBBS v3; C++)
Callback 列表結尾或條件 | `!one_key::fptr`    | (固定長度)         | `KeyFunc::key == 'h'`
　                     |                     |                    | `KeyFunc::first == 'h'` (DreamBBS v3; C)
　                     |                      |                    | `std::unordered_map::end()` (DreamBBS v3; C++)
Callback 取得方法　　 　| Loop/O(n)            | Direct index/O(1) | Loop/O(n)
　                     |                      |                    | Hash table/O(1) (DreamBBS v3; C++)

### 游標處理
　                      | Pirate BBS                | PttBBS                          | MapleBBS 3
 :---                   | ---                       | ---                             | ---
嘗試取得已載入列表的游標 | `getkeep()`               | `getkeep()`                     | `xo_get()`
直接初始化游標到最後     | (`malloc()`)              | (`malloc()`)                    | `xo_new()`
游標資料結構            | `struct keeploc`           | `keeploc_t`                     | `XO`
游標紀錄資料結構        | `struct keeploc *`         | `struct keepsome *`             | `XO *`
游標紀錄儲存            | `struct keeploc *keeplist` | `struct keeploc *keeplist`      | `XO *xo_root`
　                     | Function-scope global      | Function-scope global           | File-scope Global
游標紀錄資料結構類型    | Singly linked list         | Singly linked block             | Singly linked list
　                     |                            | 以 `KEEPSLOT` (`10`) 為 1 block |
游標資料取得方法        | Loop/String comparison     | Loop/String hash                | Loop/String comparison

### 列表資料
　                      | Pirate BBS                    | PttBBS                       | MapleBBS 3
 :---                   | ---                           | ---                          | ---
列表資料結構             | `struct fileheader`           | `fileheader_t`               | (any; 另有 `HDR` 對應 `fileheader_t`)
列表資料儲存 (全域變數)  | `struct fileheader *files`    | `fileheader_t *headers`       | 通常為 `char xo_pool[]`
　                      |                               |                               | 通常為 `char *xo_pool_base` (DreamBBS v3)
列表資料儲存空間取得     | `calloc()` + `realloc()`      | `calloc()` + `realloc()`      | (通常為 statically allocated)
　                      |                               |                               | 通常為 `mmap()` (DreamBBS v3)
列表資料取得             | `get_records()`              | `get_records_and_bottom()`    | 通常為 `xo_load()`
列表資料取得方法         | `lseek()` + `read()` 部分載入 | `lseek()` + `read()` 部分載入 | 通常為 `lseek()` + `read()` 部分載入
　                      |                               |                               | 通常為 `mmap()` 映射整個列表 (DreamBBS v3)

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
從列表後說明處重繪        | (`PARTUPDATE`)     | (`PART_REDRAW`)    | (`XO_BODY`)
　                       |                     |                    | 有些分支有增加 `XO_KNEE`
重繪畫面底部              | (`PARTUPDATE`)     | `READ_REDRAW`      | `XO_FOOT`
　                       |                     |                    | WindTop BBS 3.x: 只清除螢幕底部 (不會有 footer)
　                       |                     |                    | MapleBBS-itoc: 在螢幕底部畫出 `XZ::feeter`
　                       |                     |                    | DreamBBS v3: 呼叫 callback 列表對應 `XO_FOOT` 的函數
重繪畫面頂部              | (`FULLUPDATE`)     | `TITLE_REDRAW`     | (`XO_HEAD`)
　                       |                     |                    | `XR_HEAD` (DreamBBS v3)
重新載入資料但不重繪      | (`PARTUPDATE`)      | `HEADERS_RELOAD`   | (直接操作資料結構重新載入)
　                       |                     |                    | `XR_LOAD` (DreamBBS v3)

### 列表操作的相關 macros (括號：無直接對應，替代的處理方式)
使用場合                 | Pirate BBS          | PttBBS                    | MapleBBS 3
 :---                    | ---                 | ---                       | ---
指定某功能需要動態載入    | (無)                | (無)　                     | `cmd \| XO_DL`
將游標放到最尾項          | (直接操作)          | (直接操作)                 | (直接操作: `xo->pos = XO_TAIL`)
　                       |                     |                           | `XO_MOVE + XO_TAIL` (DreamBBS v3)
移動游標                 | (直接操作)           | `READ_NEXT` & `READ_PREV` | `XO_MOVE + pos`
移動游標 (頭尾循環)       | (無)                | (無)                      | `XO_MOVE + XO_WRAP + pos`
翻頁                     | (直接操作)          | (直接操作)                 | `XO_MOVE + pos ± XO_TALL`
翻頁 (頭尾循環)           | (無)                | (無)                      | `XO_MOVE + XO_WRAP + pos ± XO_TALL`
捲動列表                 | (無)                | (無)                      | `XO_MOVE + XO_SCRL + pos` (DreamBBS v3 新增)
捲動列表 (頭尾循環)       | (無)                | (無)                      | `XO_MOVE + XO_SCRL + XO_WRAP + pos` (DreamBBS v3 新增)
切換列表                 | (無)                | (無)                      | `XZ_<ZONE>` = `XO_ZONE + zone`
回到上層列表             | (無)                | (無)                      | (無；有 `XO_LAST`，但未實作)
離開列表                 | `DOQUIT`            | `DOQUIT`                  | `XO_QUIT`

## MapleBBS 3 與 DreamBBS v3 的 Xover callback key value 的分配

### 輸入按鍵的值
範圍或對應的 bit mask                | 相關 macro         | 功能                         | 註解
 :---                               | ---                | ---                          | ---
`0x00000000` - `0x0000001f`         | `Ctrl()`           | `Ctrl-` 一般按鍵             |
`0x00000020` - `0x000000ff`         | (無)               | 一般按鍵                     |
`0x00000100` - `0x00001fff`         | (無)               | 傳統特殊按鍵 (Phoenix BBS)   | DreamBBS v1 起恢復使用
`0x00002000` - `0x000020ff`         | `Meta()`/`Esc()`   | `Esc-`/`Alt-` 一般按鍵       | Maple-itoc 不使用
`0x00002100` - `0x00003fff`         | `Meta()`/`Esc()`   | `Esc-`/`Alt-` 特殊按鍵       | DreamBBS v1 起新增
`0x00000060` (mask)                 | `Shift()`/`Ctrl()` | 特殊按鍵的 `Shift-`/`Ctrl-`  | DreamBBS v1 起新增
　                                  | `Ctrl(key)`        | `0x00`: `Ctrl-`              | Mask 後變 `0x00`
　                                  | `Shift(Ctrl(key))` | `0x20`: `Shift-Ctrl-`        | Mask 後變 `0x20`
　                                  | `key`              | `0x40`: 正常                 | Mask 後變 `0x40`
　                                  | `Shift(key)`       | `0x60`: `Shift-`             | Mask 後變 `0x60`
`0x00004000` - `0x7fffffff`         | (不使用)           | (不使用)                      | 保留給 Xover 列表系統使用
`0x80000000` - `0xffffffff`         | (無)               | MapleBBS 3 特殊按鍵 (負數)    | DreamBBS v1 起不使用，保留給 Xover 系統

### MapleBBS 3 的 Xover callback key value 的分配
範圍或對應的 bit mask                | 相關 macro         | 功能                         | 註解
 :---                               | ---                | ---                          | ---
`0x00000000` - `0x00003fff`         | (無)               | 按鍵輸入                     |
`0x00004000` - `0x0fffffff`         | (未使用)           | (未使用)                     |
`0x10000000` - `0x1fffffff-XO_TALL` | `XO_MODE`          | 畫面重繪、資料載入、離開列表  | 實際上只使用 9 或 10 個 (有些分支有 `XO_KNEE`，有些沒有)
`0x20000000-XO_TALL` - `0x1fffffff` | `XO_MOVE + pos` (`pos < 0`)            | 設定游標位置 (頭尾循環)      | Maple-itoc 修正前
`0x20000000` - `0x27ffffff-XO_TALL` | `XO_MOVE + pos`  (`pos >= 0`)          | 設定游標位置                 | Maple-itoc 修正前
`0x28000000` - `0x3fffffff`         | `XO_MOVE + XO_WRAP + pos` (`pos >= 0`) | 設定游標位置 (頭尾循環)      | Maple-itoc 修正前
`0x20000000-XO_TALL` - `0x20800000` | `XO_MOVE + pos`                        | 設定游標位置                 | Maple-itoc 修正後
`0x20800001` - `0x3fffffff`         | `XO_MOVE + XO_WRAP + pos`              | 設定游標位置 (頭尾循環)       | Maple-itoc 修正後
`0x40000000` - `0x7fffffff`         | `XO_ZONE + zone` (`zone >= 0`)         | 列表切換                      | Maple-itoc 只使用 14 個
`0x80000000` - `0xffffffff`         | `key \| XO_DL`     | 特殊按鍵 (負數) 或動態載入功能 | 不能以特殊按鍵做為 callback 列表的 key
　                                  |                    |                               | DreamBBS v1 起將特殊按鍵值恢復為傳統的正數，可做為 callback 列表的 key

### DreamBBS v3 的 Xover callback key value 的分配
範圍或對應的 bit mask                | 相關 macro         | 功能                         | 註解
 :---                               | ---                | ---                         | ---
`0x00000000` - `0x00003fff`         | (無)               | 按鍵輸入                     |
`0x00004000`                        | `XO_NONE`          | 什麼都不做                   | 與 `Ctrl(' ')` (`'\0'`) 作區分
`0x00ffffff` (mask)                 | `XO_MOVE_MASK`     | 游標移動相關                 |
`0x001fffff` (mask)                 | `XO_POS_MASK`      | 取得游標目標位置             |
`0x00004001` - `0x001fffff`         | `XO_MOVE + pos`                               | 單純設定游標位置             | 這限制了游標的移動範圍為 `-0x0000bfff` (-49151) - `0x001effff` (2031615)
`0x**000000`                        | `XR_*`                                        | 單純畫面重繪、資料載入        |
`0x**200000`                        | `XR_* + XO_SCRL`                              | 單純畫面重繪、資料載入        |
`0x**400000`                        | `XR_* + XO_WRAP`                              | 單純畫面重繪、資料載入        |
`0x**600000`                        | `XR_* + XO_WRAP + XO_SCRL`                    | 單純畫面重繪、資料載入        |
`0x**000001` - `0x**1fffff`         | `XR_* + XO_MOVE + pos`                        | 設定游標位置                 | `XO_MOVE` 是游標位置的 bias，設定為 `0x00010000`
`0x**200001` - `0x**3fffff`         | `XR_* + XO_MOVE + XO_SCRL + pos`              | 設定畫面中列表首項            |
`0x**400001` - `0x**5fffff`         | `XR_* + XO_MOVE + XO_WRAP + pos`              | 設定游標位置 (頭尾循環)       |
`0x**600001` - `0x**7fffff`         | `XR_* + XO_MOVE + XO_WRAP + XO_SCRL + pos`    | 設定畫面中列表首項 (頭尾循環) |
`0x3f000000` (mask)                 | `XO_REDO_MASK`     | 畫面重繪、資料載入相關        | 把 `XR_*` macros `or` 起來的值
`0x01000000` (mask)                 | `XR_LOAD`          | 重新載入列表資料             | `XO_INIT` = `XR_LOAD + XO_HEAD`, `XO_LOAD` = `XR_LOAD + XO_BODY`
`0x02000000` (mask)                 | `XR_HEAD`          | 重繪畫面頂部                 | `XO_HEAD` = `XR_HEAD + XO_NECK`
`0x04000000` (mask)                 | `XR_NECK`          | 重繪列表前說明               | `XO_NECK` = `XR_NECK + XO_BODY`
`0x08000000` (mask)                 | `XR_BODY`          | 重繪列表                     | `XO_BODY` = `XR_BODY + XO_KNEE`
`0x10000000` (mask)                 | `XR_KNEE`          | 重繪列表後說明處             | `XO_KNEE` = `XR_KNEE + XO_FOOT`
`0x20000000` (mask)                 | `XR_FOOT`          | 重繪畫面底部                 | `XO_FOOT` = `XR_FOOT`
`0x7f000000` (mask)                 | `XO_ZONE_MASK`     | 列表切換相關                 | 把 `XZ_*` macros `or` 起來的值
`0x40000000` (mask)                 | `XZ_ZONE`          | 列表切換相關                 | `XO_ZONE` = `XZ_ZONE + XO_MOVE`
`0x01000000` (mask)                 | `XZ_INIT`          | 進行某 zone 的初始化工作      |
`0x02000000` (mask)                 | `XZ_FINI`          | 進行某 zone 的收拾工作        |
`0x04000000` (mask)                 | `XZ_BACK`          | 回到上次所在的 zone           | `XO_LAST` = `XZ_ZONE + XZ_BACK`
`0x08000000` (mask)                 | `XZ_QUIT`          | 離開 `xover()` 函數          | `XO_QUIT` = `XZ_ZONE + XZ_QUIT`
`0x10000000` (mask)                 | (未使用)           | (未使用)                     |
`0x20000000` (mask)                 | (未使用)           | (未使用)                     |
`0x80000000` (mask)                 | `key \| XO_DL`     | 動態載入功能                 |
#### 新的 key value 分配的特點
- 將 `XO_MOVE` 重新定義為游標位置的 bias，避免游標位置為負時 flag bits 的改變
- 重繪畫面的各個 `XO_*` macros 之間的相對大小不變
- 允許畫面重繪/重新載入 (`XR_*`) 的各個部分自由組合 (尚未實作)
- 允許畫面重繪/重新載入 (`XR_*`) 的同時移動游標
- 使用與游標移動相同的方法切換 zone