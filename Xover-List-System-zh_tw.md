# Xover 列表系統

Xover 列表系統是 MapleBBS 3.x 中所大量使用的列表顯示系統。

不同於 Pirate BBS 原本的 `i_read` 列表系統只能用在文章與信件列表，Xover 列表系統能夠用來顯示任何的列表。

以下使用歷史比較的方法概覽 Xover 列表系統。
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
 :---                   | ---                 | ---                | ---          
按鍵處理函數            | `i_read()`          | `i_read_key()`     | `xover()`
Callback 列表資料結構   | `struct one_key[]`  | `onekey_t[126]`    | `KeyFunc[]`
　                      | 　                  | 　                 | `std::unordered_map<unsigned int, XoFunc>` (DreamBBS v3; C++)
Callback 列表結尾或條件 | `!one_key::fptr`    | (固定長度)          | `KeyFunc::key == 'h'`
　                     | 　                   | 　                  | `KeyFunc::first == 'h'` (DreamBBS v3; C)
　                     | 　                   | 　                  | `std::unordered_map::end()` (DreamBBS v3; C++)
Callback 取得方法　　 　| Loop/O(n)            | Direct index/O(1)  | Loop/O(n)
　                     | 　                   | 　                  | Hash table/O(1) (DreamBBS v3; C++)

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
　                     | 　                         | 以 `KEEPSLOT` (`10`) 為 1 block |
游標資料取得方法        | Loop/String comparison     | Loop/String hash                | Loop/String comparison

### 列表資料
　                      | Pirate BBS                    | PttBBS                       | MapleBBS 3
 :---                   | ---                           | ---                          | ---
列表資料結構             | `struct fileheader`           | `fileheader_t`               | (any; 另有 `HDR` 對應 `fileheader_t`)
列表資料儲存 (全域變數)  | `struct fileheader *files`    | `fileheader_t *headers`       | 通常為 `char xo_pool[]`
　                      | 　                            | 　                            | 通常為 `char *xo_pool_base` (DreamBBS v3)
列表資料儲存空間取得     | `calloc()` + `realloc()`      | `calloc()` + `realloc()`      | (通常為 statically allocated)
　                      | 　                            | 　                            | 通常為 `mmap()` (DreamBBS v3)
列表資料取得             | `get_records()`              | `get_records_and_bottom()`    | 通常為 `xo_load()`
列表資料取得方法         | `lseek()` + `read()` 部分載入 | `lseek()` + `read()` 部分載入 | 通常為 `lseek()` + `read()` 部分載入
　                      | 　                            | 　                            | 通常為 `mmap()` 映射整個列表 (DreamBBS v3)

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
　                       | 　                  | 　                 | 有些分支有增加 `XO_KNEE`
重繪畫面底部              | (`PARTUPDATE`)     | `READ_REDRAW`      | `XO_FOOT`
　                       | 　                  | 　                 | WindTop BBS 3.x: 只清除螢幕底部 (不會有 footer)
　                       | 　                  | 　                 | MapleBBS-itoc: 在螢幕底部畫出 `XZ::feeter`
　                       | 　                  | 　                 | DreamBBS v3: 呼叫 callback 列表對應 `XO_FOOT` 的函數
重繪畫面頂部              | (`FULLUPDATE`)     | `TITLE_REDRAW`     | (`XO_HEAD`)
重新載入資料但不重繪      | (`PARTUPDATE`)      | `HEADERS_RELOAD`   | (直接操作資料結構重新載入)

### 列表操作的相關 macros (括號：無直接對應，替代的處理方式)
使用場合                 | Pirate BBS          | PttBBS                    | MapleBBS 3
 :---                    | ---                 | ---                       | ---
指定某功能需要動態載入    | (無)                | (無)　                     | `cmd \| XO_DL`
將游標放到最尾項          | (直接操作)          | (直接操作)                 | (直接操作: `xo->pos = XO_TAIL`)
移動游標                 | (直接操作)           | `READ_NEXT` & `READ_PREV` | `XO_MOVE + pos`
移動游標（頭尾循環）      | (無)                | (無)                      | `XO_MOVE + XO_WRAP + pos`
　                       | 　                  | 　                        | `XO_WRAP + pos` 要大於等於 `XO_WRAP_MIN` (DreamBBS v3 新增)
翻頁                     | (直接操作)          | (直接操作)                 | `XO_MOVE + pos ± XO_TALL`
翻頁（頭尾循環）          | (無)                | (無)                      | `XO_MOVE + XO_WRAP + pos ± XO_TALL`
捲動列表                 | (無)                | (無)                      | `XO_MOVE + XO_SCRL + pos` (DreamBBS v3 新增)
　                       | 　                  | 　                        | `XO_SCRL + pos` 要大於等於 `XO_SCRL_MIN` (DreamBBS v3 新增)
捲動列表（頭尾循環）      | (無)                | (無)                      | `XO_MOVE + XO_SCRL + XO_WRAP + pos` (DreamBBS v3 新增)
切換列表                 | (無)                | (無)                      | `XZ_<ZONE>` = `XO_ZONE + zone`
回到上層列表             | (無)                | (無)                      | (無；有 `XO_LAST`，但未實作)
離開列表                 | `DOQUIT`            | `DOQUIT`                  | `XO_QUIT`