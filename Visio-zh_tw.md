# Visio 輸出入函式庫

Visio 是 MapleBBS 3 的系統輸出入函式庫，功能上包含：
- MapleBBS 3 前：`io.c` + `screen.c` + `term.c` + 部份 `stuff.c`
- PttBBS：`io.c` (+ `nios.c` + `vtkbd.c`) + `screen.c` (或 `pfterm.c`) + `term.c` + `vtuikit.c` + 部份 `stuff.c`

MapleBBS 3 的 Visio 有以下相關但來源不同的同名函式庫，非本文主題：
- PttBBS 中現名為 `vtuikit` 的使用者介面函式庫，曾也稱為 `visio`。
- WD BBS 一些分支中的 `visio.c` 為 `io.c` + `term.c` + `screen.c` (不含 `stuff.c`) 的直接合併。
    - 較早出現於 AT-BBS 1.5.1，但可追溯至 StarRiver BBS 20000619。

亦有同源的同名函式庫，但本文不著重論述：
- SOB-fromzero 的 `visio.c`，為 1996 年由 MapleBBS 3.00 原作者 opus 所發佈的早期版本的 `visio.c` 加上小幅度修正而成。

## 名稱

### MapleBBS 3.0x 官方解釋

- Visio 是「***Vi***rtual ***S***creen ***I***nput ***O***utput Routines」的縮寫。
- "***Vi***rtual terminal" 的 ***s***creen 與 ***I***/***O***。

※ 註：同是 "virtual terminal" 的一部份，`mbbsd.c` 功能上包含 <code>in.z***bbsd***</code> + <code>***m***ain.c</code>，是 "virtual terminal" 的 daemon 與子處理程序的 main routines。

資料來源：楓橋驛站 SysSuggest 板精華區，opus 所撰寫的〈[code] virtual terminal 的寫法 (1/3)〉與〈[code] virtual terminal 的寫法 (2/3)〉。

### 沒有文獻驗證的考據

- [Vīsiō](https://en.wiktionary.org/wiki/visio#Latin) 是英文 vision 的拉丁文詞源，有「視覺」、「所見」的意思。暗指 Visio 的輸出功能。
- [Visio](https://en.wikipedia.org/wiki/Microsoft_Visio) 是在公元 1992 年初次釋出的圖表繪製軟體，由 [Shapeware](https://en.wikipedia.org/wiki/Visio_Corporation) 公司釋出。Shapeware 公司在公元 1995 年改名為 Visio 公司，在公元 2000 年被 Microsoft 收購。該軟體現名為 Microsoft Visio。同樣暗指 Visio 的輸出功能。
- ***Vi***rtual <code>***s***creen.c</code> + <code>***io***.c</code>。
- [怡申科技股份有限公司 (Essen***Visio***n Inc.)](https://web.archive.org/web/20010402004131/http://essenvision.com:80/)。在 1998-02-23 成立，2001-07-05 解散，惟至 2022-07-07 仍未清算。
    - 資料來源：商工登記公示資料查詢服務 <https://findbiz.nat.gov.tw/fts/query/QueryBar/queryInit.do?banNo=16307857>
    - 但 `visio.c` 在 1996-11-13 已公開發佈（見以上資料來源）。

## 輸入按鍵的值

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

輸入按鍵的值可以用作 Xover 系統的回呼函式特殊值。請見 [[Xover 列表系統§MapleBBS 3 與 DreamBBS v3 的 Xover callback key value 的分配|Xover-zh_tw#maplebbs-3-與-dreambbs-v3-的-xover-callback-key-value-的分配]]。

### 特殊按鍵的值

與 Pirate BBS 衍生之 BBS 比較。

Macro | 值 (傳統) | 值 (MapleBBS 3) | 值 (DreamBBS v3) | 意義 | 出處 | 註解
 --- | --- | --- | --- | --- | --- | ---
`I_TIMEOUT` | - `-2` <br> - `0x180` (NSYSU BBS 2.2.1 & Formosa BBS CE) <br> - `0x05fd` (PttBBS vtkbd) | `-31` | `0x04fd` | 按鍵逾時 | Pirate BBS | `I_` 前綴也見於 `i_getch()`/`igetch()`，表示 ***i***nput
`I_OTHERDATA` | - `-3` (Pirate BBS, Eagles BBS, & PalmBBS) <br> - `0x181` (NSYSU BBS 2.2.1 & Formosa BBS CE) <br> - `-333` <br> - `0x05fe` (PttBBS vtkbd) | `-32` | `0x04fe` | 接收到外來輸入 | Pirate BBS | 
`I_SIGNAL` | `-4` | (無) | (無) | 收到 UNIX signal | Eagles BBS 3.0 | 未使用 <br> 只見於該分支與衍生分支
`I_RESIZETERM` | | | `Meta(Ctrl('L'))` (`0x200c`) | 畫面大小變更 | DreamBBS v3.0 | 可使畫面強制重繪 <br> 可用 <kbd>Esc</kbd> + <kbd>Ctrl</kbd>-<kbd>L</kbd> 手動輸入
`KEY_INCOMPLETE` | `0x0420` | (無) | (無) | 輸入按鍵碼不完整 | PttBBS vtkbd |
`KEY_UNKNOWN` | - `0x0fff` (PttBBS git r3492 & FormosaBBS CE) <br> - `0x0f20` (PttBBS vtkbd) | (無) | (無) | 不支援的輸入按鍵碼 | PttBBS |
`KEY_INVALID` | | | `0x03fe`| 不支援的輸入按鍵碼 | DreamBBS v3.0 |
`KEY_NONE` | | | - `12345` (DreamBBS v1.0) <br> - `0x4000` (DreamBBS v2.0+) | 表示未輸入按鍵 <br> - 定義：按鍵值 < `KEY_NONE` (DreamBBS v3.0) | DreamBBS v1.0 |
`KEY_BS` | `0x08` (`'\b'`) | (無) | (無) | <kbd>Backspace</kbd> | PttBBS |
`KEY_BS2` | - `'\x7f'` (PttBBS git r3492 & Formosa BBS CE) <br> - (移除) (PttBBS vtkbd) | (無) | (無) | <kbd>Backspace</kbd> | PttBBS | 實作 PttBBS vtkbd 後併入 `KEY_BS`
`KEY_BKSP` | | `8` (`'\b'`) | (無) | <kbd>Backspace</kbd> | MapleBBS-itoc |
`KEY_BACKSPACE` | | | `'\b'` (`0x08`) | <kbd>Backspace</kbd> | DreamBBS v3.1 | 依 ncurses 函式庫介面取名
`KEY_TAB` | `9` (`'\t'`) | | `'\t'` | <kbd>Tab</kbd> | Phoenix BBS 3.0 |
`KEY_CR` | `'\r'` (`0x0d`) | (無) | (無) | <kbd>Enter</kbd> | PttBBS
`KEY_LF` | `'\n'` (`0x0a`) | (無) | (無) | <kbd>Enter</kbd> | PttBBS | 在 PttBBS 中被忽略
`KEY_ENTER` | - `10` (`'\n'`) (MapleBBS 3.10) <br> - `KEY_CR` (`'\r'`) (PttBBS) | | `'\n'` | <kbd>Enter</kbd> | MapleBBS 3.10 & PttBBS |
`KEY_ESC` | `27` (`0x1b`) | | `'\x1b'` | - <kbd>Esc</kbd> + 一般按鍵 (Phoenix BBS) <br> - 單獨的 <kbd>Esc</kbd> (DreamBBS v3) | Phoenix BBS 3.0 | 見 [[Visio-zh_tw#輸入按鍵的值]]
`KEY_UP` | - `0x0101` <br> - `0x100 + 'A'` (`0x141`) (Pivot BBS) <br> - `512` (`0x0100`) (PalmBBS) | `-1` | `0x0141` | <kbd>↑</kbd> | Phoenix BBS 3.0 |
`KEY_DOWN` | - `0x0102` <br> - `0x142` (Pivot BBS) <br> - `0x0101` (PalmBBS) | `-2` | `0x0142` | <kbd>↓</kbd> | Phoenix BBS 3.0 |
`KEY_RIGHT` | - `0x0103` <br> - `0x143` (Pivot BBS) <br> - `0x0102` (PalmBBS) | `-3` | `0x0143` | <kbd>→</kbd> | Phoenix BBS 3.0 |
`KEY_LEFT` | - `0x0104` <br> - `0x144` (Pivot BBS) <br> - `0x0103` (PalmBBS) | `-4` | `0x0144` | <kbd>←</kbd> | Phoenix BBS 3.0 |
`KEY_STAB` | - `0x0105` (PttBBS git r3492 & Formosa BBS CE) <br> - `0x0109` (PttBBS vtkbd) | (無) | `KEY_BTAB` | <kbd>Shift</kbd>-<kbd>Tab</kbd> | PttBBS |
`KEY_BTAB` | | | `0x015a` (`0x0100 + 'Z'`) | <kbd>Shift</kbd>-<kbd>Tab</kbd> | DreamBBS v3.0 | 依 ncurses 函式庫介面取名
`KEY_HOME` | - `0x0201` <br> - `0x100 + '1'` (`0x131`) (Pivot BBS) <br> - `0x0401` (PalmBBS) | - `0x0181` (MapleBBS 3.00a 註解；未採用) <br> - `-21` | `0x0241` | <kbd>Home</kbd> | Phoenix BBS 3.0 |
`KEY_INS` | - `0x0202` <br> - `0x0402` (PalmBBS) | `-22` | `0x0242` | <kbd>Insert</kbd> | Phoenix BBS 3.0 |
`KEY_INSERT` | `0x132` | (無) | (無) | <kbd>Insert</kbd> | Pivot BBS 5.04 | 此名稱未見於其它分支
`KEY_DEL` | - `0x0203` <br> - `0x133` (Pivot BBS) <br> - `0x0403` (PalmBBS) | `-23` | `0x0243` | <kbd>Delete</kbd> | Phoenix BBS 3.0 |
`KEY_END` | - `0x0204` <br> - `0x134` (Pivot BBS) <br> - `0x0404` (PalmBBS) | `-24` | `0x0244` | <kbd>End</kbd> | Phoenix BBS 3.0 |
`KEY_PGUP` | - `0x0205` <br> - `0x0405` (PalmBBS) | `-25` | `0x0245` | <kbd>PgUp</kbd> | Phoenix BBS 3.0 |
`KEY_PGDN` | - `0x0206` <br> - `0x0406` (PalmBBS) | `-26` | `0x0246` | <kbd>PgDn</kbd> | Phoenix BBS 3.0 |
`KEY_F1`&ndash;`KEY_F12` | `0x0301`&ndash;`0x030C` | (無) | `0x0251`&ndash;`0x025C` | <kbd>F1</kbd>&ndash;<kbd>F12</kbd> | PttBBS |
`BACK` | - `0x7f` <br> - (移除) (Formosa BBS 1.0.0) | (無) | (無) | <kbd>Backspace</kbd> | NSYSU BBS 2.2.1 |
`NL` | `0x0a` (`'\n'`) | (無) | (無) | <kbd>Enter</kbd> | NSYSU BBS 2.2.1 |
`CR` | `0x0d` (`'\r'`) | (無) | (無) | <kbd>Enter</kbd> | NSYSU BBS 2.2.1 |
`ENTER` | `CR` | (無) | (無) | <kbd>Enter</kbd> | NSYSU BBS 2.2.1 |
`SP` | `0x20` | (無) | (無) | <kbd>Space</kbd> | NSYSU BBS 2.2.1 |
`ESC` | `0x1b` | (無) | (無) | <kbd>Esc</kbd> | NSYSU BBS 2.2.1 |
`TAB` | `0x09` | (無) | (無) | <kbd>Tab</kbd> | NSYSU BBS 2.2.1 |
`CTRLA`&ndash;`CTRLZ` | - `0x01`&ndash;`0x1a` <br> - (移除) (Formosa BBS 1.0.0) | (無) | (無) | <kbd>Ctrl</kbd> + <kbd>A</kbd>&ndash;<kbd>Z</kbd> | NSYSU BBS 2.2.1 | 被 `CTRL()` 取代而移除

「值 (MapleBBS 3)」為 MapleBBS 3.00a 起採用的新值；MapleBBS 3.00b 與之前採用「值 (傳統)」。

值與「值 (傳統)」相同者省略不列。

## Vget 輸入框函式

`vget()` 輸入框函式是 MapleBBS 3 的輸入框函式。

在 MapleBBS 3 以前，此函式原名為 `getdata()`。

PttBBS 的 `vtuikit` 函式庫有提供介面與 MapleBBS 3 相容的 `vget()`。

### 函式宣告

#### PirateBBS 1.9
```c
getdata(line,col,prompt,buf,len,echo,complete)
int line,col ;
char *prompt, *buf ;
int len, echo ;
int (*complete)() ;
```
- 支援自動完成，函式 `complete()` 需另外傳入

#### MapleBBS 2.36
```c
getdata(line, col, prompt, buf, len, echo)
  int line, col;
  char *prompt, *buf;
  int len, echo;
```
- 不支援自動完成

#### MapleBBS 3.00
```c
int
vget(line, col, prompt, data, max, echo)
  int line, col;
  uschar *prompt, *data;
  int max, echo;
```
- 支援自動完成，用 `echo` 指定使用預定義的自動完成函式

#### DreamBBS v3.0
```cpp
int vget(int y_ref, int x_ref, const char *prompt, char *data, int max, int echo)
```
- 支援自動完成，用 `echo` 指定使用預定義的自動完成函式
- 使用了畫面大小座標，支援畫面大小改變時的自動重繪

### Echo flags
　              | 值      | 出處  | 說明
:---            | ---     | ---  | ---
`NOECHO`        | `0` <br> - `0x0000` (MapleBBS 3.00) <br> - `HIDEECHO` (DreamBBS v3.0) | PirateBBS | - 完全不顯示輸入框 <br> (PirateBBS, MapleBBS 2.36 (有 `dumb_term`), & PttBBS) <br> (DreamBBS v2.0 (有 `VGET_STEALTH_NOECHO`)) <br> - 將輸入字元顯示為 `*` <br> (MapleBBS 2.36 (無 `dumb_term`) & MapleBBS 3) <br> (DreamBBS v2.0 (無 `VGET_STEALTH_NOECHO`))
`DOECHO`        | `1` <br> - `0x0100` (MapleBBS 3.00) <br> - `0` (DreamBBS v3.0) | PirateBBS | 正常顯示輸入框
`HIDEECHO`      | `0x0100` (DreamBBS v3.0) | DreamBBS v3.0 | - 完全不顯示輸入框 (有 `VGET_STEALTH_NOECHO`) <br> - 將輸入字元顯示為 `*` (無 `VGET_STEALTH_NOECHO`)
`LCECHO`        | - `2` (MapleBBS 2.36 & PttBBS) <br> - `0x0200` (MapleBBS 3.00) | MapleBBS 2.36 | - 將輸入的第一個字元轉為小寫 <br> - 將輸入全部轉為小寫 (DreamBBS v2.0)
`NUMECHO`       | - `4` (PttBBS) <br> - `0x0400` (DreamBBS v2.0) | PttBBS | 只能輸入數字 `0`–`9` <br> - DreamBBS v2.0 時引入
`GCARRY`        | - `0x0800` (MapleBBS 3.00) <br> - `8` (PttBBS) | MapleBBS 3.00 | 將輸出 `buf` 的初始內容用作預設已輸入內容
`PASSECHO`      | - `0x10` (PttBBS) <br> - `NOECHO` (DreamBBS v2.0) <br> - `HIDEECHO` (DreamBBS v3.0) | PttBBS | 將輸入字元顯示為 `*` <br> - `NOECHO` 的預設行為 (DreamBBS v2.0) <br> - DreamBBS v2.0 時引入
`GET_LIST`      | `0x1000` | MapleBBS 3.00 | 依照 link list `ll_head` 的內容進行自動完成 <br> 需先用 `ll_new()`/`ll_add()`/`ll_del()` 設定 `ll_head`
`GET_USER`      | `0x2000` | MapleBBS 3.00 | 自動完成 user ID
`GET_BRD`       | `0x4000` | MapleBBS 3.02 | 自動完成 board ID
`VGET_STRICT_DOECHO` | - `0x10000` (DreamBBS v2.0) <br> - `false` (DreamBBS v3.0) | DreamBBS v2.0 | 忽略 flags 所預設附帶的 `DOECHO` 效果
`VGET_STEALTH_NOECHO` | `0x20000` | DreamBBS v2.0 | 完全不顯示輸入框 (沒有 `DOECHO` 時)
`VGET_BREAKABLE` | `0x40000` | DreamBBS v2.0 | 允許用 <kbd>Ctrl</kbd>-<kbd>C</kbd> 關閉輸入框 <br> PttBBS `vget()` 的預設行為

- 在 MapleBBS 2.36 與 PttBBS 中，echo flags 不可疊加使用
- 在 MapleBBS 3.00 後，`0x01` 到 `0x80` 的位元範圍保留給 BRD bits (下述)，於 `GET_BRD` 時疊加使用
- 在 DreamBBS v2.0 後，echo flags 可用 bitwise OR (`|`) 自由疊加使用
- DreamBBS v3.0 將 `DOECHO` 改為 `0`，以表示無 flags 時為一般狀態

### 特殊值
　                  | 值   | 出處      | 說明
:---                | ---  | ---      | ---
`VGET_IMPLY_DOECHO` | `LCECHO \| NUMECHO \| GCARRY` | DreamBBS v2.0 | 預設附帶有 `DOECHO` 效果的 flags
`VGET_FORCE_DOECHO` | `GET_LIST \| GET_USER \| GET_BRD` | DreamBBS v2.0 | 會強制開啟 `DOECHO` 效果的 flags
`VGET_EXIT_BREAK`   | `-1` | DreamBBS v2.0 | 輸入框被 <kbd>Ctrl</kbd>-<kbd>C</kbd> 關閉時回傳的值

### 自動完成功能所使用的 macros

#### BRD bits (MapleBBS 3, WindTopBBS, & DreamBBS)
　          | 值     | 出處           | 說明
:---        | ---    | ---           | ---
`BRD_R_BIT` | `0x01` | MapleBBS 3.00 | 使用者可進入閱覽此看板
`BRD_W_BIT` | `0x02` | MapleBBS 3.00 | 使用者可於此看板發佈與編輯文章
`BRD_X_BIT` | `0x04` | MapleBBS 3.00 | 使用者可管理此看板
`BRD_V_BIT` | `0x08` | MapleBBS 3.00 | 使用者本次上站期間已進入過此看板
`BRD_H_BIT` | `0x10` | MapleBBS 3.00 | 使用者的 `.BRH` 中有此看板的閱讀紀錄
`BRD_Z_BIT` | `0x20` | MapleBBS 3.00 | 被使用者 zap 的看板
`BRD_F_BIT` | `0x40` | DreamBBS-2010 | 使用者的看板清單中可出現此看板 (但僅看板好友可進入)

- MapleBBS 3.10-itoc 有不同的定義 (下述)

#### BRD bits (MapleBBS 3.10-itoc)
　          | 值     | 說明
:---        | ---    | ---
`BRD_L_BIT` | `0x0001` | 使用者的看板清單中可出現此看板
`BRD_R_BIT` | `0x0002` | 使用者可進入閱覽此看板
`BRD_W_BIT` | `0x0004` | 使用者可於此看板發佈與編輯文章
`BRD_X_BIT` | `0x0008` | 使用者可管理此看板
`BRD_M_BIT` | `0x0010` | 使用者可存取需版主權限的功能
`BRD_V_BIT` | `0x0020` | 使用者本次上站期間已進入過此看板
`BRD_H_BIT` | `0x0040` | 使用者的 `.BRH` 中有此看板的閱讀紀錄
`BRD_Z_BIT` | `0x0080` | 被使用者 zap 的看板

#### Match Operations
　            | 值  | 出處           | 說明
:---          | --- | ---           | ---
`MATCH_END`   | - `1` <br> - `0x8000` (MapleBBS 3.02) | MapleBBS 3.00 | 進行自動完成並結束自動完成 <br> 限內部處理 <br> - 可疊加在 echo flags 上 (MapleBBS 3.02)
`MATCH_LIST`  | - `2` <br> - (移除) (MapleBBS 3.02) | MapleBBS 3.00 | 進行自動完成並顯示自動完成清單 <br> 限內部處理 <br> - 等效於沒有 `MATCH_END` 而移除 (MapleBBS 3.02)
`MATCH_CHECK` | - `4` <br> - (移除) (MapleBBS 3.02) | MapleBBS 3.00 | 檢查輸入是否存在於自動完成清單，不進行自動完成 <br> 限內部處理 <br> - 僅用於 `GET_LIST` (MapleBBS 3.00) <br> - 一律進行自動完成而移除 (MapleBBS 3.02)
