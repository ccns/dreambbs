# Xover 列表系統

Xover 列表系統是 MapleBBS 3.x 中所大量使用的列表顯示系統。

不同於 Pirate BBS 原本的 `i_read` 列表系統只能用在文章與信件列表，Xover 列表系統能夠用來顯示任何的列表。

## 名稱

### Xover

Xover (唸若 *ex(tension)-over*)，取自 [NNTP](https://zh.wikipedia.org/wiki/網路新聞傳輸協定) (網路新聞傳輸協定) 的命令 [`XOVER`](https://datatracker.ietf.org/doc/html/rfc2980#section-2.8)，使用此命令可列出 news server 上的指定範圍的文章的 metadata。

MapleBBS 3.00 的原作者 opus 將閱讀 BBS 文章的系統取名為 Xover，可能是暗指其「列出文章 metadata」之意。

NNTP 命令的 [`X` 前綴](https://datatracker.ietf.org/doc/html/rfc977#section-3) 表示尚未被正式標準化的命令。而 NNTP 命令 `XOVER` 已被標準化為 RFC 3977 的 NNTP 命令 [`OVER`](https://datatracker.ietf.org/doc/html/rfc3977.html#section-8.3)。

DreamBBS v3.0 起，指稱此系統時，固定寫成僅首字母大寫的「Xover」。

※ 註：NNTP 命令並不區分大小寫。

### `HDR`

`HDR` (唸若 *header*) 是 Xover 列表系統所使用的列表資料的資料結構名稱，取自 NNTP 命令 [`XHDR`](https://datatracker.ietf.org/doc/html/rfc2980#section-2.6)，使用此命令可取得指定範圍的文章的指定 metadata 欄位 ("header")。可能暗指 `HDR` 能儲存 metadata 欄位的資訊。這也是變數名稱 `xhdr` 的最初由來。

NNTP 命令 `XHDR` 也已被標準化為 RFC 3977 的 NNTP 命令 [`HDR`](https://datatracker.ietf.org/doc/html/rfc3977.html#section-8.5)。

### `struct OverView`

`struct OverView` 是 Xover 列表系統所使用的列表狀態的資料結構 `XO` 的 `struct` 名稱，取自一些 NNTP server 以 [News Overview](<https://en.wikipedia.org/wiki/NOV_(computers)>) 所建立的 overview 資料庫，此資料庫能讓 NNTP 命令 `XOVER` 有效率地取得文章的 metadata。可能暗指 `struct OverView` 能有效率地瀏覽文章的 metadata。

## Pirate BBS 衍生之 BBS 的列表函式比較

本文所提到的 BBS 系統間的衍生關係，可見 [[TANet BBS 家族譜系圖|TANet-BBS-Chart-zh_tw]]，其中須注意：
- WindTop BBS 3.02 是以較早期的 MapleBBS 3.10 為基礎開發而來，主要架構屬於 MapleBBS 3.10
    - DreamBBS 2010 是 WindTop BBS 3.02 的後代分支，因此主要架構也屬於 MapleBBS 3.10
        - 現在的 DreamBBS v1 及更新版本是 DreamBBS 2010 的後代，架構上也屬於 MapleBBS 3.10

### 列表主函式
　                  | Pirate BBS <br> MapleBBS 2.36 <br> PttBBS <br> FireBird BBS 2.51 <br> Formosa BBS 1.1.1 | Formosa BBS 1.4.1 | MapleBBS 3
 :---               | ---              | ---              | ---          
列表主程式檔名       | `read.c`         | `cursor.c`       | `xover.c`
列表主函式           | `i_read()`       | `cursor_menu()`  | `xover()`
用途                | 文章與信件列表 <br> - 精華區、看板列表 (Formosa BBS 1.1.1) <br> - 好友列表 (FireBird BBS 2.51) | 主選單、文章、精華區、 <br> 投票、信件、看板、分類看板、 <br> 使用者、好友列表 <br> - 可處理大部分全螢幕列表 | 除主選單外的大部分全螢幕列表
其它列表的處理方法   | 寫新的列表處理函式 | 用 `cursor_menu()` 改寫 | - 用 `xover()` 改寫 <br> - 寫新的列表顯示函式，用 `xo_cursor()` 處理游標位置 <br> - DreamBBS v3.0 不再使用 `xo_cursor()`，將其移除

### 列表主函式與其參數

#### Pirate BBS (1.9)
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

#### Formosa BBS (1.4.1; r410)
```c
int cursor_menu( int y, int x,
				 char *direct,
				 struct one_key *comm,
				 int hdrsize,
				 int *ccur,
				 void (*cm_title) (),
				 void (*cm_btitle) (),
				 void (*cm_entry) (int, void *, int, int, int, int),
				 int (*cm_get) (char *, void *, int, int),
				 int (*cm_max) (char *, int),
				 int (*cm_findkey) (char *, void *, int, int ),
				 int opt, int autowarp)
```
- 參數 `dotitle` 更名為 `cm_title`
- 參數 `doentry` 更名為 `cm_entry`
- 參數 `rcmdlist` 更名為 `comm`
- 多出參數 `y`, `x`, `hdrsize`, `ccur`, `cm_btitle`, `cm_get`, `cm_max`, `cm_findkey`, `opt`, & `autowarp`
- 無參數 `cmdmode` 和 `bidcache`
- 共 14 個參數

#### MapleBBS 3 (取自 DreamBBS v2.0)
```cpp
void
xover(int cmd)
```
- 參數 `cmdmode` 併入 `XZ::mode`
- 參數 `direct` 併入 `XZ::dir`
- 參數 `dotitle` 和 `doentry` 放入 callback list
- 參數 `rcmdlist` 併入 `XZ::cb`
    - DreamBBS v3.0: 改併入 `XO::cb`
- 無參數 `bidcache` (使用 `bbstate` + `currbid`)

### 其它列表的處理方法之範例

- Pirate BBS (1.9) 使用者列表的顯示函式與其參數（無游標，只能換頁）
```c
printcuent(uentp)
struct user_info *uentp ;
```

- PttBBS (r4903) 使用者列表的游標及顯示函式與其參數
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
　                     | Pirate BBS <br> MapleBBS 2.36 <br> PttBBS <br> FireBird BBS 2.51 <br> MapleBBS 2.39 <br> WD BBS <br> Formosa BBS | MapleBBS 3
 :---                  | ---                 | ---          
按鍵處理函式            | - `i_read()` (Pirate BBS & Formosa BBS 1.1.1) <br> - `cursor_menu()` (Formosa BBS 1.4.1) <br> - `i_read_key()` (etc.) | - `xover()` <br> - `xover_exec_cb()` & `xover_key()` (DreamBBS v3)
Callback 列表資料結構   | - `struct one_key[]` <br> - `onekey_t[126]` (PttBBS) | - `KeyFunc[]` <br> - `std::unordered_map<unsigned int, XoFunc>` (DreamBBS v3; C++)
Callback 列表結尾或條件 | - `!one_key::fptr` <br> - `!one_key::key` (MapleBBS 2.39) <br> - (固定長度) (PttBBS) <br> - `one_key::key == 'h' && (currmode & MODE_DIGEST)` <br> (MapleBBS 2.36, MapleBBS 2.39, & WD BBS) | - `KeyFunc::key == 'h'` <br> - `KeyFunc::first == 'h'` (DreamBBS v3; C) <br> - `std::unordered_map::end()` (DreamBBS v3; C++)
Callback 取得方法　   　| - Loop/O(n) <br> - Direct indexing/O(1) (PttBBS) | - Loop/O(n) <br> - Hash table/O(1) (DreamBBS v3; C++)

### 游標紀錄處理
　                     | Pirate BBS <br> MapleBBS 2.36 <br> PttBBS <br> FireBird BBS 2.51 <br> MapleBBS 2.39 <br> WD BBS | Formosa BBS | MapleBBS 3
 :---                  | ---                        | ---                  | ---
嘗試取得已載入列表的游標 | `getkeep()`                | (Array indexing) <br> - 或不儲存 (1.4.1) | `xo_get()` <br> - 或不儲存
游標資料結構儲存空間取得 | (`malloc()`)               | Statically allocated <br> - 或 stack-based (1.4.1) | `xo_new()` <br> - 或 stack-based
游標資料結構            | - `struct keeploc` <br> - `keeploc_t` (PttBBS) | (None) | `XO`
游標紀錄資料結構        | - `struct keeploc *` <br> - `struct keepsome *` (PttBBS) | (None) | `XO *`
游標紀錄儲存            | - `struct keeploc *keeplist` <br> - `struct keepsome *keeplist` (PttBBS) | - `int t_top[TREASURE_DEPTH],` <br> `t_cur[TREASURE_DEPTH]` <br> `int mailtop, mailcur;` (1.1.1) <br> - (各自儲存或不儲存) (1.4.1) | `XO *xo_root` <br> - 或不儲存
游標紀錄儲存可見度      | Function-scope global       | File-scope global | File-scope global
游標紀錄資料結構類型    | - Singly linked list <br> - Singly linked block (1 block = `KEEPSLOT` (`10`)) (PttBBS) | Array | Singly linked list
游標資料取得方法        | - Loop/String comparison <br> - Loop/Board number comparison (MapleBBS 2.39) <br> - Loop/String hash (PttBBS) | Array indexing | Loop/String comparison

### 列表資料
　                      | Pirate BBS <br> MapleBBS 2.36 <br> PttBBS <br> FireBird BBS 2.51 <br> MapleBBS 2.39 <br> WD BBS | Formosa BBS | MapleBBS 3
 :---                   | ---                           | ---                           | ---
列表資料結構             | - `struct fileheader` <br> - `fileheader_t` (PttBBS) <br> - (any; 另有 `struct fileheader`) (FireBird BBS 2.51) | - `FILEHEADER` (1.1.1) <br> - (any; 另有 `FILEHEADER`) (1.4.1) | (any; 另有 `HDR` 對應 `struct fileheader`)
列表資料儲存 (全域變數)  | - `struct fileheader *files` <br> - `fileheader_t *headers` (PttBBS) <br> - `char *pnt` (FireBird BBS 2.51) | - `FILEHEADER *fheads` (1.1.1) <br> - `char hdrs[MAX_SCREEN_SIZE * MAX_HDRSIZE]` (1.4.1) | - 通常為 `char xo_pool[]` <br> - 通常為 `char *xo_pool_base` (DreamBBS v3)
列表資料儲存空間取得     | - `calloc()` <br> - `calloc()` + `realloc()` (PttBBS) | - `malloc()` (1.1.1) <br> - Statically allocated (1.4.1) | - 通常為 statically allocated <br> - 通常為 `mmap()` (DreamBBS v3)
列表資料取得             | - `get_records()` <br> - `get_records_and_bottom()` (PttBBS) | - `get_list()` (1.1.1) <br> - `*_get()` (1.4.1) | 通常為 `xo_load()`
列表資料取得方法         | `lseek()` + `read()` 載入部分列表 | `lseek()` + `read()` 載入部分列表 <br> - 或用 `memcpy()` 取得已載入資料的一部分 (1.4.1) | - 通常為 `lseek()` + `read()` 載入部分列表 <br> - 通常為 `mmap()` 映射整個列表 (DreamBBS v3)

### 重新載入與重繪的相關 macros (括號：無直接對應，替代的處理方式)
使用場合                  | Pirate BBS <br> MapleBBS 2.36 <br> PttBBS <br> FireBird BBS 2.51 | Formosa BBS | MapleBBS 2.39 <br> WD BBS | MapleBBS 3
 :---                    | ---                | ---           | ---           | ---
什麼都不做                | `DONOTHING`        | - `R_NO`/`B_NO`/`M_NO` <br> (定義一致) (1.1.1) <br> - `C_NONE` (1.4.1) | `RC_NONE`     | `XO_NONE`
切換列表資料檔，重新載入   | `NEWDIRECT`        | - `R_NEW`/`B_NEW`/`M_NEW` <br> (定義一致) (1.1.1) <br> - `C_INIT` (1.4.1) | `RC_NEWDIR`   | `XO_INIT`
列表資料檔有更動，重新載入 | - (`FULLUPDATE`) (Pirate BBS) <br> - `DIRCHANGED` (etc.) | - (`R_NEW`) (1.1.1) <br> - (`C_INIT`) (1.4.1) | `RC_CHDIR` | (`XO_INIT`)
重新載入資料並重繪全畫面   | `FULLUPDATE`       | - (`R_NEW`) (1.1.1) <br> - `C_LOAD` (1.4.1) | `RC_FULL`     | `XO_INIT`
重新載入資料並從列表頭重繪 | `PARTUPDATE`       | - (`R_NEW`) (1.1.1) <br> - (`C_LOAD`) (1.4.1) | `RC_BODY`     | `XO_LOAD`
重繪全畫面               | (`FULLUPDATE`)      | - `R_FULL`/`B_FULL`/`M_FULL` <br> (定義一致) (1.1.1) <br> - `C_FULL` (1.4.1) | (`RC_FULL`)   | `XO_HEAD`
從列表前說明處重繪        | (`FULLUPDATE`)      | - (`R_FULL`) (1.1.1) <br> - (`C_FULL`) (1.4.1) | (`RC_FULL`)   | `XO_NECK`
從列表頭重繪              | - (`PARTUPDATE`) <br> - `PART_REDRAW` <br> (MapleBBS 2.36 & PttBBS) | - `R_PART`/`B_PART` (定義一致) (1.1.1) <br> - (`C_FULL`) (1.4.1) <br> - `CX_GET` (限內部處理) (1.4.1) | `RC_DRAW` | `XO_BODY`
重繪某項                  | (直接呼叫函式)      | - (`R_LINE`) (1.1.1) <br> - (`C_LINE`) (1.4.1) | - `RC_ITEM` <br> - (直接呼叫函式) (WD BBS) | - (直接呼叫函式) <br> - 有些分支有增加 `XO_ITEM` <br> - `XO_CUR + diff` (DreamBBS v3)
重繪某項及畫面底部         | (直接呼叫函式)      | - `R_LINE`/`B_LINE`/`M_LINE` <br> (定義一致) (1.1.1) <br> - `C_LINE` (1.4.1) | (直接呼叫函式) | - (直接呼叫函式) <br> - `XR_FOOT + XO_CUR + diff` (DreamBBS v3)
從列表後說明處重繪        | - (`PARTUPDATE`) <br> - (`PART_REDRAW`) <br> (MapleBBS 2.36 & PttBBS) | - (`R_PART`) (1.1.1) <br> - (`C_FULL`) (1.4.1) | (`RC_DRAW`) | - (`XO_BODY`) <br> - 有些分支有增加 `XO_KNEE`
重繪畫面底部              | - (`PARTUPDATE`) <br> - `READ_REDRAW` <br> (MapleBBS 2.36 & PttBBS) | - (`R_PART`) (1.1.1) <br> - `C_FOOT` (1.4.1) | `RC_FOOT` | `XO_FOOT` <br> - WindTop BBS 3.x: 只清除螢幕底部 (不會有 footer) <br> - MapleBBS-itoc: 在螢幕底部畫出 `XZ::feeter` <br> - DreamBBS v3: 呼叫 callback 列表對應 `XO_FOOT` 的函式
重繪畫面頂部              | - (`FULLUPDATE`) <br> - `TITLE_REDRAW` (PttBBS) | - (`R_FULL`) (1.1.1) <br> - (`C_FULL`) (1.4.1) | (`RC_FULL`) | - (`XO_HEAD`) <br> - `XR_PART_HEAD + key` (DreamBBS v3)
重新載入資料但不重繪      | - (`PARTUPDATE`) <br> - `HEADERS_RELOAD` (PttBBS) | (操作資料結構重新載入) | (操作資料結構重新載入) | - (操作資料結構重新載入) <br> - `XR_PART_LOAD + key` (DreamBBS v3)

### 列表操作的相關 macros (括號：無直接對應，替代的處理方式)
使用場合                 | Pirate BBS <br> MapleBBS 2.36 <br> PttBBS <br> FireBird BBS 2.51 <br> MapleBBS 2.39 <br> WD BBS | Formosa BBS | MapleBBS 3
 :---                    | ---                  | ---                  | ---
指定某功能需要動態載入    | (無)                  | (無)                 | `cmd \| XO_DL` (MapleBBS 3.10)
將游標放到最尾項          | (直接操作)            | (直接操作)            | - (直接操作: `xo->pos = XO_TAIL`) <br> - `XO_MOVE + XO_TAIL` (DreamBBS v3.0 起支援)
移動游標                 | - (直接操作) <br> - `GOTO_NEXT` (FireBird BBS 2.51) | - `CAREYDOWN` & `CAREYUP` (1.1.1) <br> - `C_DOWN & C_UP` (1.4.1) | - `XO_MOVE + pos` <br> - `XO_MOVE + XO_REL + diff` (DreamBBS v3.0 起支援)
移動游標 (頭尾循環)       | (無)                  | (無)                 | - `XO_MOVE + XO_WRAP + pos` <br> - `XO_MOVE + XO_WRAP + XO_REL + diff` (DreamBBS v3.0 起支援)
翻頁                     | (直接操作)            | - (直接操作 + `CAREYDOWN`/`CAREYUP`) (1.1.1) <br> - (直接操作 + `C_MOVE`) (1.4.1) | - `XO_MOVE + pos ± XO_TALL` <br> - `XO_MOVE + XO_REL ± XO_TALL` (DreamBBS v3.0 起支援) <br> - 尾項上捲: `XO_MOVE + XO_REL - ((xo->max-1 - xo->top) % XO_TALL + 1)` (DreamBBS v3.0)
翻頁 (頭尾循環)           | (無)                 | (無)                  | - `XO_MOVE + XO_WRAP + pos ± XO_TALL` <br> - 尾項下捲: `xo->top = 0, XR_BODY + XO_MOVE + XO_WRAP + XO_REL + BMIN(xo->max, XO_TALL)` (DreamBBS v3.0) <br> - 首尾項上捲: `XO_MOVE + XO_WRAP + XO_REL - ((xo->max-1 - xo->top) % XO_TALL + 1)` (DreamBBS v3.0)
捲動列表                 | (無)                  | (無)                  | - `XO_MOVE + XO_SCRL + pos` (DreamBBS v3.0 新增) <br> - `XO_MOVE + XO_SCRL + XO_REL + diff` (DreamBBS v3.0 新增)
捲動列表 (頭尾循環)       | (無)                  | (無)                  | - `XO_MOVE + XO_WRAP + XO_SCRL + pos` (DreamBBS v3.0 新增) <br> - `XO_MOVE + XO_WRAP + XO_SCRL + XO_REL + diff` (DreamBBS v3.0 新增)
切換列表                 | (無)                  | (無)                  | - `XZ_<ZONE>` = `XO_ZONE + zone` <br> - `XO_ZONE + XO_WRAP + zone` (DreamBBS v3.0 起支援) <br> - `XO_ZONE + XO_REL + diff` (DreamBBS v3.0 起支援) <br> - `XO_ZONE + XO_WRAP + XO_REL + diff` (DreamBBS v3.0 起支援)
回到上層列表             | (無)                  | (無)                  | (無；有 `XO_LAST`，但未實作)
離開列表函式             | - `DOQUIT` <br> - `QUIT` (MapleBBS 2.39 & WD BBS) | - (直接操作) (1.1.1) <br> -`C_REDO` (1.4.1) | `XO_QUIT` <br> - `QUIT` 重新定義為 `XO_QUIT` (DreamBBS v3.0)

## MapleBBS 3 與 DreamBBS v3 的 Xover callback key value 的分配

### 輸入按鍵的值
範圍或對應的 bit mask                | 相關 macro         | 功能                          | 註解
 :---                               | ---                | ---                           | ---
`0x00000000` - `0x0000001f`         | `CTRL()`/`Ctrl()`  | <kbd>Ctrl</kbd> + 一般按鍵    | - `CTRL()` 出自 Eagles BBS <br> - `Ctrl()` 出自 Phoenix BBS
`0x00000020` - `0x000000ff`         | (無)               | 一般按鍵                      |
`0x00000100` - `0x00001fff`         | (無)               | 傳統特殊按鍵                  | 出自 Phoenix BBS <br> MapleBBS 3 不使用 <br> DreamBBS v1.0 恢復使用
`0x0000001f`                        | `KEY_ESC`          | - <kbd>Esc</kbd>/<kbd>Alt</kbd> + 一般按鍵 <br> - 單獨的 <kbd>Esc</kbd> (DreamBBS v3) | - 按下的一般按鍵需用 `KEY_ESC_arg` 取得 <br> - 出自 Phoenix BBS <br> - MapleBBS 3 不使用 <br> - DreamBBS v3.0 新增按鍵延時判斷機制，恢復使用
`0x00002000` - `0x000020ff`         | `Meta()`/`Esc()`   | <kbd>Esc</kbd>/<kbd>Alt</kbd> + 一般按鍵 | - `Meta()` 出自 MapleBBS 3（未使用） <br> - `Esc()` 出自 Maple-itoc（未使用）
`0x00002100` - `0x00003fff`         | `Meta()`           | <kbd>Esc</kbd>/<kbd>Alt</kbd> + 特殊按鍵 | DreamBBS v2.0 起支援
`0x00000060` (mask)                 | `Shift()`/`Ctrl()` | 特殊按鍵的 <kbd>Shift</kbd>/<kbd>Ctrl</kbd> | DreamBBS v2.0 起支援
　                                  | `Ctrl(key)`        | `0x00`: <kbd>Ctrl</kbd>       | Mask 後變 `0x00`
　                                  | `Shift(Ctrl(key))` | `0x20`: <kbd>Shift</kbd> + <kbd>Ctrl</kbd> | Mask 後變 `0x20`
　                                  | `key`              | `0x40`: 正常                  | Mask 後變 `0x40`
　                                  | `Shift(key)`       | `0x60`: <kbd>Shift</kbd>      | `Shift()` 為 DreamBBS v2.0 新增 <br> Mask 後變 `0x60`
`0x00004000` - `0x7fffffff`         | `KEY_NONE` = `0x4000`| (不使用)                    | 保留給 Xover 列表系統使用 <br> - DreamBBS v1.0 新增 <br> - DreamBBS v2.0 改為現值 <br> - DreamBBS v3.0 改為現用法
`0x80000000` - `0xffffffff`         | (無)               | MapleBBS 3 特殊按鍵 (負數)     | DreamBBS v1.0 起不使用，保留給 Xover 系統

### MapleBBS 3 的 Xover callback key value 的分配
範圍或對應的 bit mask                | 相關 macro         | 功能                         | 註解
 :---                               | ---                | ---                          | ---
`0x00000000` - `0x00003fff`         | (無)               | 按鍵輸入                     |
`0x00004000` - `0x0fffffff`         | (未使用)           | (未使用)                     |
`0x10000000` - `0x1fffffff-XO_TALL` | `XO_MODE`          | 畫面重繪、資料載入、離開列表  | 實際上只使用 9 到 11 個 (有些分支有 `XO_ITEM` 及 `XO_KNEE`，有些沒有)
`0x20000000-XO_TALL` - `0x1fffffff` | `XO_MOVE + pos` (`pos < 0`)            | 設定游標位置 (頭尾循環)      | Maple-itoc 修正前 <br> - 會導致在列表第一頁向上翻頁時直接跳到列表最後一項 (MapleBBS 3; WindTopBBS 使用循環閱讀 `UFO2_CIRCLE` 時)
`0x20000000` - `0x27ffffff-XO_TALL` | `XO_MOVE + pos`  (`pos >= 0`)          | 設定游標位置                | Maple-itoc 修正前 <br> - 會導致在列表第一項向上移動游標時，無視循環閱讀設定而跳到列表最後一項 (WindTopBBS 不使用循環閱讀 `UFO2_CIRCLE` 時)
`0x28000000` - `0x3fffffff`         | `XO_MOVE + XO_WRAP + pos` (`pos >= 0`) | 設定游標位置 (頭尾循環)      | Maple-itoc 修正前
`0x20000000-XO_TALL` - `0x20800000` | `XO_MOVE + pos`                        | 設定游標位置                | Maple-itoc 修正後
`0x20800001` - `0x3fffffff`         | `XO_MOVE + XO_WRAP + pos`              | 設定游標位置 (頭尾循環)      | Maple-itoc 修正後 <br> - 循環發生時跳到頭尾 <br> - 循環發生時跳到循環後的對應項 (DreamBBS v3.0)
`0x40000000` - `0x7fffffff`         | `XO_ZONE + zone` (`zone >= 0`)         | 列表切換                    | Maple-itoc 只使用 14 個
`0x80000000` - `0xffffffff`         | - `key` (`key < 0`) <br> - `key \| XO_DL` (MapleBBS 3.10) | 特殊按鍵 (負數) <br> - 或動態載入功能 (MapleBBS 3.10) | MapleBBS 3.10 後不能以特殊按鍵做為 callback 列表的 key <br> - DreamBBS v1.0 將特殊按鍵值恢復為傳統的正數，可做為 callback 列表的 key

### DreamBBS v3 的 Xover callback key value 的分配
範圍或對應的 bit mask                | 相關 macro         | 功能                         | 註解
 :---                               | ---                | ---                          | ---
`0x00000000` - `0x00003fff`         | `key`              | 按鍵輸入                      |
`0x00004000`                        | `XO_NONE` = `KEY_NONE` | 什麼都不做 <br> - 可視為不對應任何功能的按鍵 | 與 `Ctrl(' ')` / `Ctrl('@')` (`'\0'`) 作區分
`0x00ffffff` (mask)                 | `XO_MOVE_MASK`     | 游標移動相關                  |
`0x001fffff` (mask)                 | `XO_POS_MASK`      | 取得游標目標位置              | 實際目標位置是 `pos - XO_MOVE` <br> `XO_MOVE` 是游標位置的 bias，設定為 `0x00100000`
　                                  | - `XR_* + key` <br> - `XZ_ZONE + XZ_* + key` | - 執行按鍵功能後進行畫面重繪及資料載入 <br> - 執行按鍵功能後進行列表相關操作 | 經過 `XO_MOVE_MASK` mask 後為 `0x00000000` - `0x00004000`
　                                  | - `XR_* + {XO_WRAP\|XO_SCRL\|XO_REL} + key` <br> - `XZ_ZONE + XZ_* + {XO_WRAP\|XO_SCRL\|XO_REL} + key` | - `XO_CUR` (`XO_REL + XO_CUR_BIAS`) <br> - `XO_RS` (`XO_SCRL`) (v3.1) <br> - (未使用) (其它) | 經過 `XO_MOVE_MASK` mask 後不為 `0x00000000` - `0x00004000`，而經過 `XO_POS_MASK` mask 後為 `0x00000000` - `0x00004000` <br> - 不需要操作游標的雜項功能放這一區
　                         　       | - `XR_* + XO_CUR + diff` <br> - `XZ_ZONE + XZ_* + XO_CUR + diff` | 重繪游標所在行，並移動游標到指定的相對位置 | 經過 `XO_MOVE_MASK` mask 後為 `0x00200000` - `0x00204000` <br> - 移動範圍為 `-0x2000` (`-8192`) - `0x2000` (`8192`)
　                         　       | - `XR_* + XO_RS + RS_*` <br> - `XZ_ZONE + XZ_* + XO_RS + RS_*` | 主題式閱讀命令 | 經過 `XO_MOVE_MASK` mask 後為 `0x00400000` - `0x00404000` <br> - v3.1 新增
　                                  | - `XR_* + XO_MOVE + move` <br> - `XZ_ZONE + XZ_* + XO_MOVE + move` | - 設定列表游標位置後進行畫面重繪及資料載入 <br> - 設定 zone 游標位置後進行列表相關操作 | 經過 `XO_POS_MASK` mask 後為 `0x00004001` - `0x001fffff` <br> - 這限制了游標的移動範圍為 `-0x000fbfff` (-1032191‬) - `0x000fffff` (1048575‬)
`0x00e00000` (mask)                 | `XO_MFLAG_MASK`    | 游標移動方式相關             | 把 `XO_WRAP`, `XO_SCRL`, & `XO_REL` `or` 起來的值 <br> - v3.1 新增
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
`0x40000000` (mask)                 | `XZ_ZONE`          | 將操作解讀為列表操作         | `XO_ZONE` = `XZ_ZONE + XO_MOVE`
`0x01000000` (mask)                 | `XZ_INIT`          | - 進行某 zone 的初始化工作 (無 `XZ_SKIN`) <br> - (未使用) (有 `XZ_SKIN`) | 未實作
`0x02000000` (mask)                 | `XZ_FINI`          | - 進行某 zone 的收拾工作 (無 `XZ_SKIN`) <br> - (未使用) (有 `XZ_SKIN`) | 未實作
`0x04000000` (mask)                 | `XZ_BACK`          | - 回到上次所在的 zone (未實作) <br> - (未使用) (有 `XZ_SKIN`) | `XO_LAST` = `(XZ_ZONE \| XZ_BACK) + XO_NONE`
`0x08000000` (mask)                 | `XZ_QUIT`          | 離開 `xover()` 函式          | `XO_QUIT` = `(XZ_ZONE \| XZ_QUIT) + XO_NONE`
`0x10000000` (mask)                 | `XZ_SKIN`          | 將操作解讀為使用者介面 skin 切換 (未實作) | `XO_SKIN` = `(XZ_ZONE \| XZ_SKIN) + XO_MOVE`
`0x20000000` (mask)                 | `XZ_UNUSED5`       | (未使用)                     |
`0x80100000` (mask)                 | `XO_FSPEC_MASK`    | 指定回呼函式類別相關 <br> 用在 `KeyFunc` 的按鍵值上 | 把相關的 macros `or` 起來的值 <br> DreamBBS v3.0 新增
`0x80000000` (mask)                 | `key \| XO_DL`     | 指定回呼函式需動態載入        |
`0x00100000` (mask)                 | `key \| XO_POSF` <br> (= `XO_MOVE`) | 指定回呼函式具有 `pos` 參數   | DreamBBS v3.0 新增
`0x7fefffff` (mask)                 | `XO_FUNC_MASK` <br> (= `~XO_FSPEC_MASK`) | 用以取得忽略函式類別指定 flags 後的實際對應按鍵值 | DreamBBS v3.0 新增

### Xover callback key value 的位元分配比較

圖例：`欄位名(所佔位元數)`、`欄位名 = 此欄定值(所佔位元數)`、`欄位名 (= 此欄通常值)(所佔位元數)`

(`--` 表示未使用或須為定值的位元)

#### MapleBBS 3
- Xover 命令與回呼函式指定：
    -   - MapleBBS 3.10 後，若在 _命令_ 中加了 `XO_DL`（**不應使用**），會造成需動態載入的按鍵功能載入後仍有 `XO_DL`，再次嘗試載入時會造成程式崩潰。
    - 一般按鍵：`32| XO_DL(1) | -- = 0(3) | -- (= 0)(20) | 按鍵值(8) |0`
    - 特殊按鍵（**不應使用**）：`32| XO_DL = 1(1) | -- (= 0x7fffff)(23) | 按鍵值的絕對值的二補數(8) |0`
        - MapleBBS 3.10 後，若用來 _指定回呼函式_，會被解析成有 `XO_DL` 而強制使用動態載入的方式載入按鍵功能，會造成程式崩潰。
    - 畫面重繪：`32| XO_DL(1) | -- = 0(2) | XO_MODE = 1(1) | -- (= 0)(24) | 重繪索引值(4) |0`
- 限 Xover 命令：
    - 游標移動：`32| -- (= 0)(1) | -- = 0(1) | XO_MOVE = 1(1) | --(1) | XO_WRAP(1) | 游標位置(27) |0`
        - 正游標位置，有加 `XO_WRAP`：`30 | XO_MOVE = 1(1) | XO_MODE = 0(1) | XO_WRAP = 1(1) | 游標位置(27) |0`
        - 負游標位置，有加 `XO_WRAP`：`30 | XO_MOVE = 1(1) | XO_MODE = 0(1) | XO_WRAP = 0(1) | 游標位置的絕對值的二補數(27) |0`
        - 正游標位置，未加 `XO_WRAP`：`30 | XO_MOVE = 1(1) | XO_MODE = 0(1) | XO_WRAP = 0(1) | 游標位置(27) |0`
        - 負游標位置，未加 `XO_WRAP`：`30 | XO_MOVE = 0(1) | XO_MODE = 1(1) | XO_WRAP = 1(1) | 游標位置的絕對值的二補數(27) |0`
    - Zone 切換：`32| -- (= 0)(1) | XO_ZONE = 1(1) | -- (= 0)(26) | Zone 索引值(4) |0`

#### DreamBBS v3.0
- Xover 命令：`32| -- (= 0)(1) | XZ_ZONE(1) | XR_PART_*/XZ_* flags(6) | XO_WRAP(1) | XO_SCRL(1) | XO_REL(1) | 游標位置值或按鍵(21) |0`
    - 若加上 `XO_DL`（**不建議**），會找不到對應的回呼函式 (C)，或僅嘗試呼叫需動態載入的版本 (C++)。
    - 無法在 Xover _命令_ 直接加上 `XO_POSF`（會被解釋為 `XO_MOVE`），不過可透過 `xover_exec_cb()` 直接呼叫對應的回呼函式（行為見後述）。
    - 其中 `21| 游標位置值或按鍵(21) |0` 的部份為：
        - 正游標位置：`21 | XO_MOVE = 1(1) | 游標位置(20) |0`
        - 負游標位置：`21 | XO_MOVE = 0(1) | 游標位置的絕對值的二補數(20) |0` (游標位置的絕對值的二補數 > `XO_NONE`/`0x4000`)
        - 僅重繪畫面或切換 zone：`21 | XO_MOVE = 0(1) | -- = 0(5) | -- = XO_NONE(15) |0`
        - 按鍵與雜項功能：`21 | XO_MOVE = 0(1) | -- = 0(6) | 按鍵值(14) |0`
- 回呼函式指定與呼叫：
    -   - 呼叫時，若加上 `XO_POSF`（**不建議**），會找不到對應的回呼函式 (C)，或僅嘗試呼叫帶 `pos` 參數的版本 (C++)。
    - 按鍵與雜項功能：`32| XO_DL(1) | -- = 0(7) | XO_WRAP(1) | XO_SCRL(1) | XO_REL(1) | XO_POSF(1) | -- = 0(6) | 按鍵值(14) |0`
    - 重繪畫面或切換 zone：`32| XO_DL(1) | XZ_ZONE(1) | XR_PART_*/XZ_* flags(6) | -- = 0(3) | XO_POSF(1) | -- = 0(5) | -- = XO_NONE(15) |0`
其中 `31| XZ_ZONE(1) | XR_PART_*/XZ_* flags(6) |24` 的部份為：
- 無 `XZ_ZONE` 時：`31| XZ_ZONE = 0(1) | XR_PART_FOOT(1) | XR_PART_KNEE(1) | XR_PART_BODY(1) | XR_PART_NECK(1) | XR_PART_HEAD(1) | XR_PART_LOAD(1) |24`
- 有 `XZ_ZONE` 時：`31| XZ_ZONE = 1(1) | XZ_UNUSED5(1) | XZ_SKIN(1) | XZ_QUIT(1) | XZ_BACK(1) | XZ_FINI(1) | XZ_INIT(1) |24`

### DreamBBS v3.0 的新的 key value 分配的特點
- 除了頭尾循環邏輯改變，舊的 macro 使用方法仍然有效
- 將 `XO_MOVE` 重新定義為游標位置的 bias，避免游標位置為負時 flag bits 的改變
- 重繪畫面的各個 `XO_*` macros 之間的相對大小不變
- 允許畫面重繪/重新載入 (`XR_*`) 的各個部分自由組合 (尚未實作)
- 允許將畫面重繪/重新載入 (`XR_*`) 或列表操作 (`XZ_ZONE + XZ_*`)，與按鍵輸入或移動游標的操作組合表示
- 使用游標移動表示 zone 的切換

## MapleBBS 3 Xover 系統之資料結構

### 列表區域資料結構 `XZ`
此結構為 MapleBBS 3.02 時引入。MapleBBS 3.00 的 `xover()` 尚未將相關處理邏輯結構化。

Member 名稱 | 型別           | 出處          | 說明
 ---        | ---           | ---           | ---
`xo`        | `XO *`        | MapleBBS 3.02 | 用以存放此區域的游標資料
`cb` <br> - 移入 `XO` (DreamBBS v3) | - `KeyFunc *` <br> - `KeyFuncListRef` (後移入 `XO`) <br> (DreamBBS v3) | MapleBBS 3.02 | 此區域的回呼函式清單
`mode`      | `int`         | MapleBBS 3.02 | 此區域對應的使用者狀態
`feeter`    | `char *`      | MapleBBS-itoc | 存放執行 `XO_FOOT` 命令時要畫出的 footer <br> - DreamBBS 未引入此成員 <br> (DreamBBS v3.0 執行 `XO_FOOT` 命令時會呼叫回呼函式清單中的對應函式)

### 游標資料結構 `XO`
又名 `struct OverView`。

Member 名稱 | 型別           | 出處          | 說明
 ---        | ---           | ---           | ---
`pos`       | `int`         | MapleBBS 3.00 | 目前游標在列表中的絕對位置
`top`       | `int`         | MapleBBS 3.00 | 畫面中列表第一項在列表中的絕對位置
`max`       | `int`         | MapleBBS 3.00 | 列表項目數量 <br> `0` 代表列表為空
`key`       | `int`         | MapleBBS 3.00 | 內部使用值 <br> 意義隨回呼函式而定
`xyz`       | - `char *` <br> - `void *` (DreamBBS v3.0) | MapleBBS 3.00 | 內部雜項資料 <br> 實際內容及意義隨回呼函式而定
`nxt`       | `XO *`        | MapleBBS 3.00 | 指向游標紀錄串列中的下一筆資料 <br> 游標資料紀錄用
`cb`        | `KeyFuncListRef` | DreamBBS v3.0 | 回呼函式清單 <br> 原 `XZ::cb`
`recsiz`    | `int`         | DreamBBS v3.0 | 列表資料項目的資料位元組大小 <br> `0` 代表未設定完成
`dir`       | - `char [0]` <br> - `char [FLEX_SIZE]` (DreamBBS v2.0) | MapleBBS 3.00 | 列表資料檔的系統路徑字串

### Xover 回呼函式資料結構 `KeyFunc`

Member 名稱 | 型別   | 出處          | 說明
 ---        | ---   | ---           | ---
`key` <br> - `first` (DreamBBS v3.0) | - `int` <br> - `unsigned int` (DreamBBS v2.1) | MapleBBS 3.00 | 回呼函式對應按鍵的值
`func`  <br> - `second` (DreamBBS v3.0) | - `int (*)()` (MapleBBS 3 / WindTop 3.02) <br> - `int (*)(XO *xo)` (DreamBBS v0.96) <br> - `XoFunc` (DreamBBS v3.0) | MapleBBS 3.00 | 回呼函式

`func`/`second` 為一 union 物件；在 MapleBBS 3 / WindTop 3.02 中原為 `int (*)()`，實際使用時須依 `key`/`first` 的值手動轉型。

DreamBBS v2.1 時將其改為匿名 union，以減少手動轉型錯誤的可能。

DreamBBS v3.0 時將此 union 分開定義為 `XoFunc`。

#### `KeyFunc` 相關型別
DreamBBS v3.0 特別定義了 `KeyFunc` 相關型別以利支援使用 hash table 實作回呼函式清單。

名稱        | 等效 C 定義        | 等效 C++ 定義  | 說明
 ---        | ---               | ---           | ---
`KeyFunc`   | 含有上述成員的 `struct` | 含有上述成員的 `std::pair` | `KeyFuncList` 的元素
`KeyFuncList` | `KeyFunc []` | 模板參數為上述成員型別的 `std::unordered_map` | 含有多個 `KeyFunc` 的表
`KeyFuncIter` | `KeyFunc *` | `KeyFuncList::iterator` | 用以遍歷 `KeyFuncList`
`KeyFuncListRef` | `KeyFunc *` | 自訂了建構式、`operator =()`、與 `operator ->()` 的結構 | 用以參照整個 `KeyFuncList`

#### Union `XoFunc`

Member 名稱 | 型別               | 出處          | `key`/`first` 指定方式 | 說明
 ---        | ---               | ---           | ---                   | ---
`func`      | `int (*)(XO *xo)` | DreamBBS v2.1 | `key`                 | 普通 Xover 回呼函式
`posf`      | `int (*)(XO *xo, int pos)` | DreamBBS v3.0 | `key \| XO_POSF` | 帶 `pos` 參數的 Xover 回呼函式
`dlfunc`    | - `const char *` (使用動態載入) <br> - `int (*)(XO *xo)` (不使用動態載入) | DreamBBS v2.1 | `key \| XO_DL` | 需動態載入的普通 Xover 回呼函式
`dlposf`    | - `const char *` (使用動態載入) <br> - `int (*)(XO *xo, int pos)` (不使用動態載入) | DreamBBS v3.0 | `key \| XO_POSF \| XO_DL` | 需動態載入的帶 `pos` 參數的 Xover 回呼函式

## MapleBBS 3 與 DreamBBS v3 的 Xover 特殊值
Macro             | 值 (省略最外層括號)        | 功能                                  | 註解
 :---             | ---                       | ---                                   | ---
`XO_DL`           | `0x80000000` <br> `0x00000000` (DreamBBS v2.0; 不使用動態載入時) | 指定回呼函式需要動態載入 | MapleBBS 3.10 新增
`XO_POSF`         | `XO_MOVE`                 | 指定回呼函式具有 `pos` 參數            | DreamBBS v3.0 新增
`XO_MODE`         | `0x10000000`              | 表示畫面重繪、資料載入、離開列表等操作  | DreamBBS v3.0 中已移除
`XO_NONE`         | - `0x10000000` <br> - `0x00004000` (DreamBBS v3)      | - 什麼都不作 <br> - 最小的被當作命令的 Xover key value |
`XR_<redo>`       | (多個)                    | 預先定義的畫面重繪及資料載入的組合動作  | DreamBBS v3.0 新增
`XR_PART_<redo>`  | (多個)                    | 畫面重繪及資料載入中的某部分            | DreamBBS v3.0 新增
`XO_MOVE`         | - `0x20000000` <br> - `0x00100000` (DreamBBS v3)      | - 表示游標移動 <br> - 游標移動的 bias (DreamBBS v3)
`XO_CUR`          | `XO_REL + XO_CUR_BIAS`    | 重繪游標所在行，並移動游標到指定的相對位置 | DreamBBS v3.0 新增
`XO_CUR_BIAS`     | `0x2000`                  | `XO_CUR` 內部處理相對位置時的 bias      | DreamBBS v3.0 新增
`XO_CUR_MIN`      | `XO_REL + 0 - XO_CUR`     | `XO_CUR` 可指定的相對位置的最小值       | DreamBBS v3.0 新增
`XO_CUR_MAX`      | `XO_REL + KEY_NONE - XO_CUR` | `XO_CUR` 可指定的相對位置的最大值  | DreamBBS v3.0 新增
`XO_RS`           | `XO_SCRL`                 | 將操作解釋為主題式閱讀命令              | DreamBBS v3.1 新增
`XO_RSIZ`         | `256`                     | 列表資料的資料結構大小限制              | DreamBBS v3.0 起不使用
`XO_TALL`         | `b_lines - 3`             | 翻頁所跳行數                           | 非常數
`XO_MOVE_MAX`     | `XO_POS_MASK - XO_MOVE`   | 可加在 `XO_MOVE` 上的最大值            | DreamBBS v3.0 新增
`XO_MOVE_MIN`     | `XO_NONE + 1 - XO_MOVE`   | 可加在 `XO_MOVE` 上的最小值            | DreamBBS v3.0 新增
`XO_TAIL`         | - `XO_MOVE - 999` <br> - `XO_WRAP - 1` (DreamBBS v3)  | - 用來將游標 `XO::pos` 初始化到列表尾項 <br> - 用在 `XO_MOVE + XO_TAIL` 中，將游標移到列表尾項 (DreamBBS v3.0 增加支援) | 注意是 `TAIL`，與 `XO_TALL` 不同
`XO_ZONE`         | - `0x40000000` <br> - `XZ_ZONE + XO_MOVE` (DreamBBS v3) | 切換到某個列表 |
`XZ_ZONE`         | - `0x40000000`            | 將操作解釋為列表相關操作                | DreamBBS v3.0 新增
`XZ_BACK`         | - `0x100` <br> - `0x04000000` (DreamBBS v3) | - (未使用) <br> - 加在 `XZ_ZONE` 上，表示回到上次進入的 zone (DreamBBS v3)    |
`XZ_<zone>`       | `XO_ZONE + <zone>`        | 切換到某個 zone                        |
`XZ_INDEX_<zone>` | `<zone>`                  | Zone 的 index 值                      | DreamBBS v3.0 新增
`XZ_INDEX_MAX`    | `XZ_INDEX_MYFAVORITE`     | 最後一個 zone 的 index 值              | DreamBBS v3.0 新增
`XZ_COUNT`        | `XZ_INDEX_MAX + 1`        | Xover zone 的數量                     | DreamBBS v3.0 新增
`XO_SKIN`         | `(XZ_ZONE \| XZ_SKIN) + XO_MOVE` | 套用某個使用者介面 skin (未實作) | DreamBBS v3.0 新增
`XZ_SKIN`         | `0x10000000`              | 將操作解讀為使用者介面 skin 切換 (未實作) | DreamBBS v3.0 新增

### MapleBBS 3 的 `XO_MODE + <mode>` (含 `XO_MODE`) 與 DreamBBS v3 Xover 特殊值的對應

Macro     | 值 (MapleBBS 3) | 值 (DreamBBS v3) | 註解
 :---     | ---             | ---              | ---
`XO_MODE` | `0x10000000`    | (移除)           | 表示畫面重繪、資料載入、離開列表等操作
`XO_NONE` | `XO_MODE + 0`   | `KEY_NONE` <br> (= `0x4000`) | 什麼都不做
`XO_INIT` | `XO_MODE + 1`   | `XR_INIT + XO_NONE` <br> (= `(XR_PART_LOAD \| XR_HEAD) + XO_NONE`) | 重新載入列表資料並從畫面頂部重繪
`XO_LOAD` | `XO_MODE + 2`   | `XR_LOAD + XO_NONE` <br> (= `(XR_PART_LOAD \| XR_BODY) + XO_NONE`) | 重新載入列表資料並從列表前說明處重繪
`XO_HEAD` | `XO_MODE + 3`   | `XR_HEAD + XO_NONE` <br> (= `(XR_PART_HEAD \| XR_NECK) + XO_NONE`) | 從畫面頂部重繪
`XO_NECK` | `XO_MODE + 4`   | `XR_NECK + XO_NONE` <br> (= `(XR_PART_NECK \| XR_BODY) + XO_NONE`) | 從列表前說明處重繪
`XO_BODY` | `XO_MODE + 5`   | `XR_BODY + XO_NONE` <br> (= `(XR_PART_BODY \| XR_KNEE) + XO_NONE`) | 從列表處重繪
`XO_KNEE` | (無)            | `XR_KNEE + XO_NONE` <br> (= `(XR_PART_KNEE \| XR_FOOT) + XO_NONE`) | 從列表後說明處重繪 <br> - DreamBBS v3.0 新增 <br> - 未實際使用 (DreamBBS v3.0)
`XO_FOOT` | `XO_MODE + 6`   | `XR_FOOT + XO_NONE` <br> (= `(XR_PART_FOOT) + XO_NONE`) | 從畫面底部重繪
`XO_LAST` | `XO_MODE + 7`   | `(XZ_ZONE \| XZ_BACK) + XO_NONE` | 回到上次所在的 zone (未實作)
`XO_QUIT` | `XO_MODE + 8`   | `(XZ_ZONE \| XZ_QUIT) + XO_NONE` | 離開 `xover()` 函式
`XO_ITEM` | `XO_MODE + 9` <br> (MapleBBS-itoc 非標準版) | `XO_CUR + diff` | 重繪游標所在行 <br> - 並移動游標到指定的相對位置 (DreamBBS v3)

### `XZ_<zone>` (含 `XZ_BACK`)

Macro      | 值 (MapleBBS 3) | 值 (DreamBBS v3) | 說明
 :---      | ---             | ---              | ---
`XZ_CLASS` | - `XO_ZONE + 0x0002` (MapleBBS 3.00) <br> - `XO_ZONE + 0` (MapleBBS 3.02) | `XO_ZONE + XZ_INDEX_CLASS` <br> (= `XO_ZONE + 0`) | 看板列表
`XZ_ULIST` | - `XO_ZONE + 0x0001` (MapleBBS 3.00) <br> - `XO_ZONE + 1` (MapleBBS 3.02) | `XO_ZONE + XZ_INDEX_ULIST` <br> (= `XO_ZONE + 1`) | 線上使用者名單
`XZ_PAL`   | - `XO_ZONE + 0x0020` (MapleBBS 3.00) <br> - `XO_ZONE + 2` (MapleBBS 3.02) | `XO_ZONE + XZ_INDEX_PAL` <br> (= `XO_ZONE + 2`) | 好友名單
`XZ_BLIST` | - `XO_ZONE + 0x0040` (MapleBBS 3.00) <br> - (移除) (MapleBBS 3.02) | (無) | 看板名單 (moderated) <br> (未使用)
`XZ_ALOHA` | `XO_ZONE + 3` (MapleBBS-itoc) | (無) | 上站通知名單
`XZ_VOTE`  | - `XO_ZONE + 0x0080` (MapleBBS 3.00) <br> - `XO_ZONE + 4` (MapleBBS 3.02 & MapleBBS-itoc) <br> - `XO_ZONE + 3` (MapleBBS 3.10) | `XO_ZONE + XZ_INDEX_VOTE` <br> (= `XO_ZONE + 3`) | 投票
`XZ_BMW`   | - `XO_ZONE + 3` (MapleBBS 3.02) <br> - `XO_ZONE + 4` (MapleBBS 3.10) <br> - `XO_ZONE + 5` (MapleBBS-itoc) | `XO_ZONE + XZ_INDEX_BMW` <br> (= `XO_ZONE + 4`) | 熱訊 (「水球」)
`XZ_MF` | `XO_ZONE + 6` (MapleBBS-itoc) | (無) | 我的最愛
`XZ_COSIGN` | `XO_ZONE + 7` (MapleBBS-itoc) | (無) | 連署
`XZ_SONG` | `XO_ZONE + 8` (MapleBBS-itoc) | (無) | 點歌
`XZ_NEWS` | `XO_ZONE + 9` (MapleBBS-itoc) | (無) | 新聞閱讀模式
`XZ_XPOST` | - `XO_ZONE + 0x0100` (MapleBBS 3.00) <br> - `XO_ZONE + 5` (MapleBBS 3.02) <br> - `XO_ZONE + 10` (MapleBBS-itoc) | `XO_ZONE + XZ_INDEX_XPOST` <br> (= `XO_ZONE + 5`) | 搜尋文章模式
`XZ_YPOST` | - `XO_ZONE + 0x0200` (MapleBBS 3.00) <br> - (移除) (MapleBBS 3.02) | (無) | 串接文章模式 <br> - MapleBBS 3.02 時併入 `XZ_XPOST` 而移除
`XZ_MBOX`  | - `XO_ZONE + 0x1000` (MapleBBS 3.00) <br> - `XO_ZONE + 6` (MapleBBS 3.02) <br> - `XO_ZONE + 11` (MapleBBS-itoc) | `XO_ZONE + XZ_INDEX_MBOX` <br> (= `XO_ZONE + 6`) | 信箱
`XZ_BOARD` | - `XO_ZONE + 0x2000` (MapleBBS 3.00) <br> - `XO_ZONE + 7` (MapleBBS 3.02) <br> - (移除) (MapleBBS-itoc) | `XO_ZONE + XZ_INDEX_BOARD` <br> (= `XO_ZONE + 7`) | 看板
`XZ_POST`  | - `XZ_BOARD` (MapleBBS 3.00) <br> - `XO_ZONE + 12` (MapleBBS-itoc) | `XZ_BOARD` <br> (= `XO_ZONE + XZ_INDEX_POST`) <br> (= `XO_ZONE + 7`) | 看板 (同 `XZ_BOARD`)
`XZ_GEM`   | - `XO_ZONE + 0x4000` (MapleBBS 3.00) <br> - `XO_ZONE + 8` (MapleBBS 3.02) <br> - `XO_ZONE + 13` (MapleBBS-itoc) | `XO_ZONE + XZ_INDEX_GEM` <br> (= `XO_ZONE + 8`) | 精華區
`XZ_MAILGEM` | `XO_ZONE + 9` (WindTopBBS 3.02)  | `XO_ZONE + XZ_INDEX_MAILGEM` <br> (= `XO_ZONE + 9`) | 信件精華區
`XZ_BANMAIL` | `XO_ZONE + 10` (WindTopBBS 3.02) | `XO_ZONE + XZ_INDEX_BANMAIL` <br> (= `XO_ZONE + 10`) | 擋信列表
`XZ_OTHER` | `XO_ZONE + 11` (WindTopBBS 3.02) | `XO_ZONE + XZ_INDEX_OTHER` <br> (= `XO_ZONE + 11`) | 其它列表
`XZ_MYFAVORITE` | `XO_ZONE + 12` (DreamBBS-2010) | `XO_ZONE + XZ_INDEX_MYFAVORITE` <br> (= `XO_ZONE + 12`) | 我的最愛
`XZ_BACK`  | `0x100` (MapleBBS 3.00) | `0x04000000` | 回到上次進入的 zone <br> - (未實作)

## MapleBBS 2.36b 與 DreamBBS v3 的主題式閱讀命令

### MapleBBS 2.36b 與 DreamBBS v3 的主題式閱讀特殊值

Macro        | 值 (MapleBBS 2.36b) | 值 (DreamBBS v3.1) | 說明
 :---        | ---                 | ---                | ---
`RS_TITLE`   | `0x02` <br> - `0x001` (MapleBBS 3.00a) | `0x0001` | - 有：搜尋標題相符的文章 <br> - 無：搜尋作者相符的文章
`RS_FORWARD` | `0x01` <br> - `0x002` (MapleBBS 3.00a) | `0x0002` | - 有：向下搜尋 <br> - 無：向上搜尋
`RS_RELATED` | `0x04`              | `0x0004`           | - 有：以目前文章當作搜尋條件 <br> - 無：不限以目前文章當作搜尋條件；需額外條件時詢問使用者
`RS_FIRST`   | `0x08`              | `0x0008`           | - 有：搜尋離游標最遠的符合文章 <br> - 無：搜尋離游標最近的符合文章
`RS_CURRENT` | `0x10`              | `0x0010`           | - 有：以最近一次閱讀的文章為目前文章 <br> - 無：以游標所在的文章為目前文章
`RS_THREAD`  | `0x20`              | `0x0020`           | - 有：排除回應、轉錄文章 <br> - 無：不排除
`RS_NEXT`    | - `0x40` (MapleBBS 2.39) <br> - (移除) (MapleBBS 3.00a) | `RS_READ_NEXT` | - 下一篇文章
`RS_PREV`    | - `0x80` (MapleBBS 2.39) <br> - (移除) (MapleBBS 3.00a) | `RS_READ_PREV` | - 上一篇文章
`RS_SEQUENT` | `0x40` (MapleBBS 3.00b) | `0x0040`       | - 有：游標上下篇文章
`RS_MARKED`  | `0x80` (MapleBBS 3.00b) | `0x0080`       | - 有：搜尋有 mark 標記的文章
`RS_UNREAD`  | `0x100` (MapleBBS 3.00b) | `0x0100`      | - 有：搜尋未讀的文章 <br> - 有 `RS_FIRST`：最上篇未讀 <br> - 無 `RS_FIRST`：上一篇**已**讀 (MapleBBS 3.02)
`RS_UNUSED9` | (無)                | `0x0200`           | (未使用)
`RS_UNUSED10` | (無)               | `0x0400`           | (未使用)
`RS_UNUSED11` | (無)               | `0x0800`           | (未使用)
`RS_BOARD`   | `0x1000` (MapleBBS 3.00a) | `0x1000`     | - 有：搜尋看板文章 <br> - 無：搜尋信箱文章 <br> - 限內部處理，配合 `RS_UNREAD`。
`RS_UNUSED13` | (無)               | `0x2000`           | (未使用)

### MapleBBS 2.36b 的主題式閱讀命令與 DreamBBS v3 Xover 特殊值的對應

Macro         | 值 (MapleBBS 2.36b) | 值 (DreamBBS v3.1) | 說明
 :---         | ---             | ---              | ---
`CURSOR_FIRST` | `RS_RELATED \| RS_TITLE \| RS_FIRST` | `XO_RS + RS_CURSOR_FIRST` <br> (= `XO_RS + (RS_RELATED \| RS_TITLE \| RS_FIRST)`) | 最上篇與游標所在文章標題相符的文章
`CURSOR_NEXT` | `RS_RELATED \| RS_TITLE \| RS_FORWARD` | `XO_RS + RS_CURSOR_NEXT` <br> (= `XO_RS + (RS_RELATED \| RS_TITLE \| RS_FORWARD)`)  | 下一篇與游標所在文章標題相符的文章
`CURSOR_PREV` | `RS_RELATED \| RS_TITLE` | `XO_RS + RS_CURSOR_PREV` <br> (= `XO_RS + (RS_RELATED \| RS_TITLE)`)  | 上一篇與游標所在文章標題相符的文章
`RELATE_FIRST` | `RS_RELATED \| RS_TITLE \| RS_FIRST \| RS_CURRENT` | `XO_RS + RS_RELATE_FIRST` <br> (= `XO_RS + (RS_RELATED \| RS_TITLE \| RS_FIRST \| RS_CURRENT)`)  | 最上篇與最近一次閱讀的文章標題相符的文章
`RELATE_NEXT` | `RS_RELATED \| RS_TITLE \| RS_FORWARD \| RS_CURRENT` | `XO_RS + RS_RELATE_NEXT` <br> (= `XO_RS + (RS_RELATED \| RS_TITLE \| RS_FORWARD \| RS_CURRENT)`)  | 下一篇與最近一次閱讀的文章標題相符的文章
`RELATE_PREV` | `RS_RELATED \| RS_TITLE \| RS_CURRENT` | `XO_RS + RS_RELATE_PREV` <br> (= `XO_RS + (RS_RELATED \| RS_TITLE \| RS_CURRENT)`)  | 上一篇與最近一次閱讀的文章標題相符的文章
`THREAD_NEXT` | `RS_THREAD \| RS_FORWARD` | `XO_RS + RS_THREAD_NEXT` <br> (= `XO_RS + (RS_THREAD \| RS_FORWARD)`) | 下一篇非回應、非轉錄文章
`THREAD_PREV` | `RS_THREAD`     | `XO_RS + RS_THREAD_PREV` <br> (= `XO_RS + RS_THREAD`) | 上一篇非回應、非轉錄文章
`READ_NEXT`   | - `5` <br> - (移除) (MapleBBS 3.00b) | `XO_RS + RS_READ_NEXT` <br> (= `XO_RS + (RS_SEQUENT \| RS_FORWARD)`)  | 下一篇文章
`READ_PREV`   | - `6` <br> - (移除) (MapleBBS 3.00b) | `XO_RS + RS_READ_PREV` <br> (= `XO_RS + RS_SEQUENT`)  | 上一篇文章
`MARK_NEXT`   | `RS_MARKED \| RS_FORWARD \| RS_CURRENT` (MapleBBS 3.00b) | `XO_RS + RS_MARK_NEXT` <br> (= `XO_RS + (RS_MARKED \| RS_FORWARD \| RS_CURRENT)`) | 下一篇有 mark 標記的文章
`MARK_PREV`   | `RS_MARKED \| RS_CURRENT` (MapleBBS 3.00b) | `XO_RS + RS_MARK_PREV` <br> (= `XO_RS + (RS_MARKED \| RS_CURRENT)`) | 上一篇有 mark 標記的文章

## DreamBBS v3 的 Xover callback 命令鏈鎖機制
### 名詞說明
#### 鏈鎖
`i_read` 與 Xover 列表系統的 callback 函式都可以透過回傳值，呼叫下一個 callback，本文稱之為「鏈鎖」。

例：假設 `'i'` 對應的 callback 回傳 `'j'`，`'j'` 對應的 callback 回傳 `'k'`，`'k'` 對應的 callback 回傳 `XO_BODY`，`XO_BODY` 對應的 callback 回傳 `XO_FOOT`，`XO_FOOT` 對應的 callback 回傳 `XO_NONE`，即可稱其為一個鏈鎖，可以記為 `'i' -> 'j' -> 'k' -> XO_BODY -> XO_FOOT -> XO_NONE`。

鏈鎖的結尾 callback 可以回傳游標移動命令、`XO_NONE` 命令、或沒有對應 callback 的命令。

#### 命令鏈鎖
透過回傳按鍵值而呼叫下一個 callback 的鏈鎖，稱為「命令鏈鎖」。

例：假設有一鏈鎖 `'i' -> 'j' -> 'k' -> XO_BODY -> XO_FOOT -> XO_NONE`，其中就包含了命令鏈鎖 `'i' -> 'j' -> 'k'`。

#### 組合操作 - Redo 組合操作與 zone 組合操作
從 DreamBBS v3.0 開始，可以將畫面重繪/重新載入或列表操作，與按鍵輸入或游標移動的操作組合表示，因此需要特別的規則來處理帶有這些組合的命令鏈鎖。

其中畫面重繪/重新載入 (`XR_*`) 等操作，本文稱之為「redo-simple 操作」，與其組合的操作則稱為「redo 組合操作」；\
列表操作 (`XZ_ZONE + XZ_*`) 等操作，本文稱之為「zone-simple 操作」，與其組合的操作則稱為「zone 組合操作」。

「Redo 組合操作」與「zone 組合操作」，本文將其合稱為「組合操作」。

#### 簡單操作 - Redo-simple 操作、zone-simple 操作、pure 操作、以及 null 操作
不是組合操作的操作，本文稱為「簡單操作」，因此，前文中的「redo-simple 操作」與「zone-simple 操作」都屬於「簡單操作」。

代表單純的按鍵輸入或游標移動的簡單操作，本文稱之為「pure 操作」。\
之所以稱為「pure 操作」，是因為當按鍵鏈鎖中的各個 callback 的回傳值只有單純的按鍵輸入與游標移動操作時，回傳值之間不會互相影響。

Null 操作是 pure 操作的特例，是導致鏈鎖終止的單純操作，包含游標移動操作、`XO_NONE` 操作、以及沒有對應 callback 的值所代表的操作。

此外，DreamBBS v3 的切換列表操作 (`XZ_<zone>`)，是用游標移動達成的，本文將其歸類為 pure 操作中的 null 操作，而非 zone 組合操作；\
MapleBBS 3 原本的 Xover 列表系統的切換列表操作，不是使用游標移動達到的，但本文也將其看作游標操作。

### 組合命令鏈鎖規則
#### 通則
組合操作中的 redo-simple 部分及 zone-simple 部分，會累積起來，在命令鏈鎖結束時再執行。

其中，如果同時累積有 redo-simple 及 zone-simple 部分，則會先執行 zone-simple 部分，再執行 redo-simple 部分。

#### Pure-redo & Redo-pure 鏈鎖 -> pure-pure-redo-simple 鏈鎖
先後執行兩個操作的 pure 部分後，執行 redo-simple 操作。
#### Pure-zone & zone-pure 鏈鎖 -> pure-pure-zone-simple 鏈鎖
先後執行兩個操作的 pure 部分後，執行 zone-simple 操作。
#### Redo-redo 鏈鎖 -> pure-pure-redo-simple 鏈鎖
先後執行兩個操作的 pure 部分後，將與兩者組合的 redo-simple 操作一齊執行。

例：假設有 2 個 callback 函式：
- `'i'` 對應的 `func_info()` 會增加某項目的查詢次數，並且在列表後說明處畫東西，執行完後需要用 `XR_KNEE` 重繪
- `Meta('i')` 對應的 `func_info_full()` 會在列表內容處畫東西，需要用 `XR_BODY` 重繪，但需要呼叫 `func_info()` 來畫出剩下的部分

而使用者按 <kbd>i</kbd> 執行 `func_info()` 後，要從畫面中列表後說明處向下重繪；按 <kbd>Esc</kbd> <kbd>i</kbd> 間接執行 `func_info()` 後，則要從畫面中列表內容處向下重繪。

可以這樣寫：
- 在 `func_info()` 中增加查詢次數，並 `return XR_KNEE + XO_NONE` (或 `return XO_KNEE`)
- `func_info_full()` 則有不同寫法：
    - 沒有鏈鎖規則時，要直接呼叫 `func_info(xo)` 再 `return XO_BODY`
        - DreamBBS v3 不使用 redo-redo 鏈鎖機制時，可以使用 `return XR_BODY | func_info(xo)`，但要確定 `func_info()` 不會回傳按鍵輸入值，否則需要鏈鎖規則才能處理
    - 有鏈鎖規則的話，則可以直接 `return XR_BODY + 'i'`，這會間接呼叫 `func_info()`，並要求至少從畫面中列表內容處向下重繪

`return XR_BODY | func_info(xo)` 寫法的優缺點：
- 適合用在按鍵的對應功能的函式已知的情況
- 限制是 `func_info(xo)` 不能回傳 `XZ_*` 命令

`return XR_BODY + 'i'`  寫法的優缺點：
- 適合用在按鍵的對應功能的函式未知的情況，尤其是在按鍵本身的值未知時
- 可以正確處理按鍵的對應功能的函式回傳 `XZ_*` 命令的狀況
- 限制是按鍵本身不能是 `XZ_*` 命令

#### Zone-zone 鏈鎖 -> pure-pure-zone-simple 鏈鎖
先後執行兩個操作的 pure 部分後，將與兩者組合的 zone-simple 操作一齊執行。
#### Redo-zone & zone-redo 鏈鎖 -> pure-pure-zone-simple-redo-simple 鏈鎖
先後執行兩個操作的 pure 部分後，先執行 zone-simple 操作，再執行 redo-simple 操作。

在某些 zone-simple 操作後可以省略部分或全部的 redo-simple 操作。
