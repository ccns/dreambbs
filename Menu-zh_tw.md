# 選單系統

這篇文章介紹 DreamBBS v3 中所存在的不同的選單系統。

系統 | 主程式檔 | 主函式 | 出處 | 說明
--- | --- | --- | --- | ---
主選單 | - `bbs.c` (Pirate BBS) <br> - `nmenus.c` (Eagles BBS 3.0) <br> - `comm_list.c` (Phoenix BBS 4.0) <br> `main.c` (SecretBBS 4.0) <br> - `menu.c` (MapleBBS 2.0.5) | - `docmd()` (Pirate BBS & SecretBBS 4.0) <br> - `NdoMenu()` (Eagles BBS 3.0) <br> - `domenu()` (Phoenix BBS 4.0 & MapleBBS 2.0.5) <br> - `menu()` (MapleBBS 3) <br> - `main_menu()` & `domenu()` (DreamBBS v3) | Pirate BBS |
Popupmenu | `popupmenu.c` | - `popupmenu()` | WindTop 3.02 | - 使用主選單系統的資料結構 (WindTop 3.02 & DreamBBS v3) <br> - 使用與主選單系統不同的特殊值 <br> - 使用與主選單系統一致的特殊值 (DreamBBS v3)
Popupmenu Prompt | `popupmenu.c` | - `popupmenu_ans()` <br> - `pmsg()` | WindTop 3.02 | -  實質上與 Popupmenu 不是同一系統 <br> - 本文特稱之為 `Popupmenu Prompt` <br> - 使用次數稀少
Window | `window.c` | - `popupmenu_ans2()` <br> - `pmsg2()` | MapleBBS-itoc | - 由 Popupmenu Prompt 系統演變而來 <br> - 在 MapleBBS-itoc 中無 `2` 後綴 <br> - DreamBBS 2010 時引入，加上 `2` 後綴

其中本文主要介紹的是主選單與 Popupmenu 2 種使用相同資料結構的系統。

## 主選單重繪指令
|    | 值  | 出處 | 說明
 --- | --- | --- | ---
`MENU_LOAD` | - `1` <br> - 移除 (DreamBBS v3) | MapleBBS 3 | DreamBBS v3 中以 `XR_PART_LOAD` 取代
`MENU_DRAW` | - `2` <br> - 移除 (DreamBBS v3) | MapleBBS 3 | DreamBBS v3 中以 `XR_PART_HEAD \| XR_BODY` 取代
`MENU_FILM` | - `4` <br> - 移除 (DreamBBS v3) | MapleBBS 3 | DreamBBS v3 中以 `XR_PART_NECK` 取代

DreamBBS v3 改用 Xover opcode 指定主選單畫面的重繪。

## 主選單所呼叫函式之 return 值
|       | 值      | 出處 | 說明
 ---    |  ---    | --- | ---
`XEASY` | - `0x333` <br> - `XO_FOOT` (DreamBBS v3) | MapleBBS 2.0.5 | 回到選單後只重繪畫面底部
`QUIT`  | - `0x666` <br> - 移除 (PttBBS current & MapleBBS-itoc) <br> - `XO_QUIT` (DreamBBS v3) | Pirate BBS | 強制退出選單
`SKIN`  | - `0x999` <br> - `XO_SKIN + {XO_WRAP\|XO_SCRL\|XO_REL} + idx` (DreamBBS v3) | WindTop BBS 3.02 | 切換 skin (未實作)
(其它)  | - `-1` - `'\a'` (`7`)  | ---  | 回到選單後重繪整個畫面 (Deprecated in DreamBBS v3)
(其它)  | (其它)  | ---  | - 回到選單後重繪整個畫面 <br> - 當作 Xover opcode 執行 (DreamBBS v3)

DreamBBS v3 的主選單改為接受 Xover opcode。

## MapleBBS 3 / WindTop 3.02 主選單系統之資料結構
(取自 DreamBBS v3 之早期版本)
```cpp
typedef struct MENU
{
    MenuItem item;
    unsigned int level;
    int umode;
    const char *desc;
} MENU;
```
- DreamBBS v3 時將 `umode` 之型別改為 `unsigned int`。

`MenuItem item` 為一 union 物件；`item` 在 MapleBBS 3 / WindTop 3.02 中原為 `void *func`，實際使用時須依 `umode` 及 `level` 的值手動轉型。

DreamBBS v2.1 時將其改為匿名 union，以減少手動轉型錯誤的可能。

DreamBBS v3 時將此 union 分開定義為 `MenuItem`。

Member 名稱 | 型別               | `umode` 指定方式 (主選單) | `umode` 指定方式 (Popupmenu) | 說明
 ---        | ---               | ---                       | ---                         | ---
`func`      | `int (*)(void)`   | `umode` <br> (`(umode & M_MASK) > M_XMENU`) | - `POPUP_FUN` <br> - `umode` <br> (`(umode & M_MASK) > M_XMENU`) (DreamBBS v3 新增) | 無參數函式
`funcarg`   | `FuncArg *`       | `umode \| M_ARG`          | - `POPUP_FUN \| POPUP_ARG` <br> - `umode \| M_ARG` (DreamBBS v3 新增) | - 帶參數的函式物件 <br> - DreamBBS v3 新增
`xofunc`    | `int (*)(XO *xo)` | - 無 <br> - `umode \| M_XO` (DreamBBS v3) | - `POPUP_XO` <br> - `umode \| M_XO` (DreamBBS v3 新增) | - Xover 函式 (WindTop 3.02 & DreamBBS v3) <br> - 未實作 <br> - DreamBBS v3 新增實作
`title`     | `const char *`    | - 無 <br> - `umode \| M_MENUTITLE` (DreamBBS v3 新增) | - `POPUP_MENUTITLE` <br> - `umode \| M_MENUTITLE` (DreamBBS v3 新增) | - 顯示在彈出式選單上的選單標題 <br> - 表示 Popupmenu 使用的 `MENU` 列表的最末項
`menu`      | `struct MENU *`   | `umode` <br> (`(umode & M_MASK) <= M_XMENU`) | - `POPUP_MENU` <br> - `umode` <br> (`(umode & M_MASK) <= M_XMENU`) (DreamBBS v3 新增) | 內層選單
`dlfuncarg` | `DlFuncArg *`     | - `M_DL(umode \| M_ARG)` <br> - `M_DL(umode) \| M_ARG` (DreamBBS v3 新增；建議寫法) | - `M_DL(umode \| M_ARG)` <br> - `POPUP_SO \| POPUP_ARG` <br> - `M_DL(umode) \| M_ARG` (DreamBBS v3 新增) | - 帶參數的函式物件，函式需動態載入 <br> - DreamBBS v3 新增
| `dl`      | `DlMenuItem`      | `M_DL(umode)` | `M_DL(umode)` | - 需動態載入的功能 <br> - DreamBBS v3 新增
| - `dlfunc` <br> - `dl.func` (DreamBBS v3) | - `const char *` (使用動態載入) <br> - `int (*)(void)` (不使用動態載入) | - `M_DL(umode)` <br> - `M_DL(umode)` (`(umode & M_MASK) > M_XMENU`) (DreamBBS v3) | - `M_DL(umode)` <br> - `M_DL(umode)` <br> (`(umode & M_MASK) > M_XMENU`) (DreamBBS v3) <br> - `POPUP_SO` (WindTop 3.02 & DreamBBS v3) | - 需動態載入的函式 <br> - Popupmenu 使用 `M_DL(umode)` 時，會看 `umode` 決定實際型別 (DreamBBS v3 移除)
`dl.xofunc`   | - `const char *` (使用動態載入) <br> - `int (*)(XO *xo)` (不使用動態載入) | `M_DL(umode \| M_XO)`    | - `M_DL(POPUP_XO)` <br> - `M_DL(umode \| M_XO)` | - 需動態載入的 Xover 函式 <br> - DreamBBS v3 新增
`dl.title`    | `const char *`  | `M_DL(umode \| M_MENUTITLE)` | - `M_DL(POPUP_MENUTITLE)` <br> - `M_DL(umode \| M_MENUTITLE)` | - 需動態載入的顯示在彈出式選單上的選單標題 <br> - DreamBBS v3 新增
`dl.menu`     | - `const char *` (使用動態載入) <br> - `struct MENU *` (不使用動態載入) | `M_DL(umode)` <br> (`(umode & M_MASK) <= M_XMENU`) | - `M_DL(POPUP_MENU)` <br> - `M_DL(umode)` <br> (`(umode & M_MASK) <= M_XMENU`) | - 需動態載入的內層選單 <br> - DreamBBS v3 新增

Member 名稱 | 型別               | `level` 指定方式 (主選單) | `level` 指定方式 (Popupmenu) | 說明
 ---        | ---               | ---                       | ---                         | ---
| - `menu` <br> - 依 `umode` 而定 (DreamBBS v3) | - `struct MENU *` <br> - 依 `umode` 而定 (DreamBBS v3)  | `PERM_MENU + key`         | - 無 <br> - `PERM_MENU + key` (DreamBBS v3 新增) | - 上層選單 <br> - 回上層時執行的功能 (DreamBBS v3) <br> - 表示主選單系統使用的 `MENU` 列表的最末項 (WindTop 3.02 & DreamBBS v3)

- 在 DreamBBS v3 的主選單中，當 MENU 列表某一項的 `desc` 字串中包含換行字元 (`'\n'`)，會把換行字元後的文字當作這一項的選項說明。

  如果這一項不是選項，則當作整個選單的預設選項說明。選項說明會在畫面底部上方一行向右對齊顯示。

## WindTop 3.02 與 DreamBBS v3 的主選單系統使用的特殊值

### WindTop 3.02 主選單系統使用的特殊值
Macro (主選單) | 值 (主選單)   | Macro (Popupmenu)  | 值 (Popupmenu) | 使用對象 | 說明
---            | ---          | ---                | ---            | ---     | ---
無             | ---          | `POPUP_QUIT`       | `0x00`         | `umode` | 選擇後退出本層選單
無             | `umode` <br> (`(umode & M_MASK) > M_XMENU`) | `POPUP_FUN` | `0x01` | `umode` | 選擇後執行函式
無             | ---          | `POPUP_XO`         | `0x02`         | `umode` | 選擇後執行 Xover 函式
無             | `umode` <br> (`(umode & M_MASK) <= M_XMENU`) | `POPUP_MENU` | `0x04` | `umode` | 選擇後進入內層選單
無             | ---          | `POPUP_MENUTITLE`  | `0x08`         | `umode` | - 非選項；設定選單標題 <br> - 代表 Popupmenu 系統的 `MENU` 列表的最末項
無             | ---          | `POPUP_SO` | - `0x10` (使用動態載入) <br> - `POPUP_FUN` (不使用動態載入) | `umode` | - 選擇後執行無參數函式；此函式需要動態載入 <br> - DreamBBS v3 移除
`M_ARG`        | `0x40000000` | `POPUP_ARG`        | `0x40000000`   | `umode` | - 執行函式時，以函式物件所帶的參數呼叫函式 <br> - DreamBBS v3 新增
`MENU_CHANG`   | `0x80000000` | 無                 | ---            | `umode`? | - 功能未知 (未使用) <br> - WindTopBBS 3.02 rev.20040420 新增
`M_DL(umode)`  | - `-umode` (使用動態載入) <br> - `umode` (不使用動態載入) | `M_DL(umode)` | - `-umode` (使用動態載入) <br> - `umode` (不使用動態載入) | `umode` | - 此選項功能需要動態載入 <br> - DreamBBS v2.0 時新增
`M_MASK`       | `0x0000FFFF` |                    |                | `umode` | - `umode` 的有效範圍；範圍之外為 flags <br> - DreamBBS v3 新增
無             |              | `POPUP_MASK`       | `0x000000FF`   | `level` | - `level` 的有效範圍；範圍之外為 flags <br> - DreamBBS v3 移除
`PERM_MENU`    | `PERM_PURGE` (`0x00800000`) | 無  | ---            | `level` | - 非選項；退出本層選單後將跳到該選單 <br> - 代表主選單系統的 `MENU` 列表的最末項
無             | ---          | `POPUP_DO_INSTANT` | `0x01000000`   | `level` | - 非選項；透過命令配對跳到該選單某選項時，立即執行對應功能 <br> - 在 `MENU` 列表的最末項出現時才有效

### DreamBBS v3 主選單系統使用的特殊值
DreamBBS v3 將主選單系統與 Popupmenu 系統的特殊值整合為一，並使用相同條件來表示 `MENU` 列表的結束。

Macro          | 值           | 使用對象 | 說明
---            | ---          | ---     | ---
無             | `umode` <br> (`(umode & M_MASK) > M_XMENU`) | `umode` | 選擇後執行函式
`M_FUN`        | `M_PROFESS` (`8`) | `umode` | - 選擇後執行函式 <br> - 最小的非 menu、非 idle 的 user mode <br> - 對應原 `POPUP_FUN`
無             | `umode` <br> (`(umode & M_MASK) <= M_XMENU`) | `umode` | 選擇後進入內層選單
`M_MENU`       | `M_MMENU` (`1`) | `umode` | - 選擇後進入內層選單 <br> - 最小的 menu user mode <br> - 對應原 `POPUP_MENU`
`M_DL(umode)`  | - `umode \| 0x80000000` (使用動態載入) <br> - `umode` (不使用動態載入) | `umode` | - 此選項功能需要動態載入 <br> - DreamBBS v2.0 時新增
`M_QUIT`       | `0x01000000` | `umode` | - 選擇後退出本層選單 <br> - 對應原 `POPUP_QUIT`
`M_XO`         | `0x02000000` | `umode` | - 選擇後執行 Xover 函式 <br> - 對應原 `POPUP_XO`
`M_ARG`        | `0x04000000` | `umode` | - 執行函式時，以函式物件所帶的參數呼叫函式 <br> - DreamBBS v3 新增
`M_DOINSTANT`  | `0x00010000` | `umode` | - 非選項；透過命令配對跳到該選單某選項時，立即執行對應功能 <br> - 表示 `MENU` 列表的最末項 <br> - 對應原 `POPUP_DO_INSTANT` <br> - 注意是對 `umode` 使用
`M_MENUTITLE`  | `0x00020000` | `umode` | - 非選項；指定選單標題 <br> - 表示 `MENU` 列表的最末項 <br> - 對應原 `POPUP_MENUTITLE`
`M_TAIL_MASK`  | `0x00FF0000` | `umode` | - 可代表 `MENU` 列表的最末項的 flags <br> - DreamBBS v3 新增
`M_MASK`       | `0x0000FFFF` | `umode` | - `umode` 的有效範圍；範圍之外為 flags <br> - DreamBBS v3 新增
`PERM_MENU`    | `PERM_PURGE` (`0x00800000`) | `level` | - 非選項；退出本層選單時執行該項所指定的功能 <br> - 表示 `MENU` 列表的最末項 <br> - 不再影響 `item` 的型別

- `POPUP_MASK` 被移除
- `POPUP_DO_INSTANT` 被移除，因 `M_DOINSTANT` 與 `POPUP_DO_INSTANT` 的用法不相同
- 其餘的 `POPUP_*` macros 以 `M_*` macros 重新定義
- `M_DOINSTANT`、`M_MENUTITLE`、以及 `PERM_MENU` 都能表示 `MENU` 列表的最末項
- `item` 的型別不再與 `level` 有關；`PERM_MENU` 出現時不再假設為 `item` 的型別為 `struct MENU *`

## User Mode、Menu Index、動態看板編號
### User Mode (無 `M_` 前綴)
出處：Pirate BBS

MapleBBS 3 中改為有 `M_` 前綴，已不存在。

在 PttBBS current 中則保留原名，仍為 user mode。

### User Mode / Menu Index (`M_*`)
出處：Eagles BBS、MapleBBS 3、PttBBS current (為各自引入)

(於 SecretBBS 4.0 及 Phoenix BBS 4.0 中皆不存在)

在 MapleBBS 3 中為 user mode，同時為 menu index；動態看板編號另外由 `FILM_*` 指定。

在 PttBBS 中為 menu index，同時為動態看板編號。

|                | User Mode    | Menu Index  | 動態看板編號
 ---             | ---          | ---         | ---
Pirate BBS (1.9) | 無 `M_` 前綴 | 無 `M_` 前綴 | (無動態看板)
Eagles BBS (3.0) | `M_*`        | `M_*`       | (無動態看板)
MapleBBS 3       | `M_*`        | `M_*`       | `FILM_*`
PttBBS current   | 無 `M_` 前綴 | `M_*`       | `M_*`