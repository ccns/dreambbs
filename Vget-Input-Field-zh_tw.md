# Vget 輸入框函式

`vget()` 輸入框函式是 MapleBBS 3 的輸入框函式。

在 MapleBBS 3 以前，此函式原名為 `getdata()`。

PttBBS 的 `vtuikit` 函式庫有提供介面與 MapleBBS 3 相容的 `vget()`。

## 函式宣告

### PirateBBS 1.9
```c
getdata(line,col,prompt,buf,len,echo,complete)
int line,col ;
char *prompt, *buf ;
int len, echo ;
int (*complete)() ;
```
- 支援自動完成，函式 `complete()` 需另外傳入

### MapleBBS 2.36
```c
getdata(line, col, prompt, buf, len, echo)
  int line, col;
  char *prompt, *buf;
  int len, echo;
```
- 不支援自動完成

### MapleBBS 3.00
```c
int
vget(line, col, prompt, data, max, echo)
  int line, col;
  uschar *prompt, *data;
  int max, echo;
```
- 支援自動完成，用 `echo` 指定使用預定義的自動完成函式

### DreamBBS v3.0
```cpp
int vget(int y_ref, int x_ref, const char *prompt, char *data, int max, int echo)
```
- 支援自動完成，用 `echo` 指定使用預定義的自動完成函式
- 使用了畫面大小座標，支援畫面大小改變時的自動重繪

## Echo flags
　              | 值      | 出處  | 說明
:---            | ---     | ---  | ---
`NOECHO`        | `0` <br> - `0x0000` (MapleBBS 3.00) | PirateBBS | - 完全不顯示輸入框 <br> (PirateBBS, MapleBBS 2.36 (有 `dumb_term`), & PttBBS) <br> (DreamBBS v2.0 (有 `VGET_STEALTH_NOECHO`)) <br> - 將輸入字元顯示為 `*` <br> (MapleBBS 2.36 (無 `dumb_term`) & MapleBBS 3) <br> (DreamBBS v2.0 (無 `VGET_STEALTH_NOECHO`))
`DOECHO`        | `1` <br> - `0x0100` (MapleBBS 3.00) | PirateBBS | 正常顯示輸入框
`LCECHO`        | - `2` (MapleBBS 2.36 & PttBBS) <br> - `0x0200` (MapleBBS 3.00) | MapleBBS 2.36 | - 將輸入的第一個字元轉為小寫 <br> - 將輸入全部轉為小寫 (DreamBBS v2.0)
`NUMECHO`       | - `4` (PttBBS) <br> - `0x0400` (DreamBBS v2.0) | PttBBS | 只能輸入數字 `0`–`9` <br> - DreamBBS v2.0 時引入
`GCARRY`        | - `0x0800` (MapleBBS 3.00) <br> - `8` (PttBBS) | MapleBBS 3.00 | 將輸出 `buf` 的初始內容用作預設已輸入內容
`PASSECHO`      | - `0x10` (PttBBS) <br> - `NOECHO` (DreamBBS v2.0) | PttBBS | 將輸入字元顯示為 `*` <br> - `NOECHO` 的預設行為 (DreamBBS v2.0) <br> - DreamBBS v2.0 時引入
`GET_LIST`      | `0x1000` | MapleBBS 3.00 | 依照 link list `ll_head` 的內容進行自動完成 <br> 需先用 `ll_new()`/`ll_add()`/`ll_del()` 設定 `ll_head`
`GET_USER`      | `0x2000` | MapleBBS 3.00 | 自動完成 user ID
`GET_BRD`       | `0x4000` | MapleBBS 3.02 | 自動完成 board ID
`VGET_STRICT_DOECHO` | `0x10000` | DreamBBS v2.0 | 忽略 flags 所預設附帶的 `DOECHO` 效果
`VGET_STEALTH_NOECHO` | `0x20000` | DreamBBS v2.0 | 完全不顯示輸入框 (沒有 `DOECHO` 時)
`VGET_BREAKABLE` | `0x40000` | DreamBBS v2.0 | 允許用 <kbd>Ctrl</kbd>-<kbd>C</kbd> 關閉輸入框 <br> PttBBS `vget()` 的預設行為

- 在 MapleBBS 2.36 與 PttBBS 中，echo flags 不可疊加使用
- 在 MapleBBS 3.00 後，`0x01` 到 `0x80` 的位元範圍保留給 BRD bits (下述)，於 `GET_BRD` 時疊加使用
- 在 DreamBBS v2.0 後，echo flags 可用 bitwise OR (`|`) 自由疊加使用

## 特殊值
　                  | 值   | 出處      | 說明
:---                | ---  | ---      | ---
`VGET_IMPLY_DOECHO` | `LCECHO \| NUMECHO \| GCARRY` | DreamBBS v2.0 | 預設附帶有 `DOECHO` 效果的 flags
`VGET_FORCE_DOECHO` | `GET_LIST \| GET_USER \| GET_BRD` | DreamBBS v2.0 | 會強制開啟 `DOECHO` 效果的 flags
`VGET_EXIT_BREAK`   | `-1` | DreamBBS v2.0 | 輸入框被 <kbd>Ctrl</kbd>-<kbd>C</kbd> 關閉時回傳的值

## 自動完成功能所使用的 macros

### BRD bits (MapleBBS 3, WindTopBBS, & DreamBBS)
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

### BRD bits (MapleBBS 3.10-itoc)
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

### Match Operations
　            | 值  | 出處           | 說明
:---          | --- | ---           | ---
`MATCH_END`   | - `1` <br> - `0x8000` (MapleBBS 3.02) | MapleBBS 3.00 | 進行自動完成並結束自動完成 <br> - 可疊加在 echo flags 上 (MapleBBS 3.02)
`MATCH_LIST`  | - `2` <br> - (移除) (MapleBBS 3.02) | MapleBBS 3.00 | 進行自動完成並顯示自動完成清單 <br> - 等效於沒有 `MATCH_END` 而移除 (MapleBBS 3.02)
`MATCH_CHECK` | - `4` <br> - (移除) (MapleBBS 3.02) | MapleBBS 3.00 | 檢查輸入是否存在於自動完成清單，不進行自動完成 <br> - 僅用於 `GET_LIST` (MapleBBS 3.00) <br> - 一律進行自動完成而移除 (MapleBBS 3.02)