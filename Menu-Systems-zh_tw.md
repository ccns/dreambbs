# Menu Systems | 選單系統

這篇文章介紹 DreamBBS v3 中所存在的不同的選單系統。

系統 | 主程式檔 | 主函數 | 出處 | 說明
--- | --- | --- | --- | ---
主選單 | - `bbs.c` (Pirate BBS) <br> - `nmenus.c` (Eagles BBS 3.1) <br> - `comm_list.c` (FireBird BBS) <br> - `menu.c` (MapleBBS 2.0) | - `docmd()` (Pirate BBS) <br> - `NdoMenu()` (Eagles BBS 3.1) <br> - `domenu()` (MapleBBS 2.0 & FireBird BBS) <br> - `menu()` (MapleBBS 3) <br> - `main_menu()` & `domenu()` (DreamBBS v3) | Pirate BBS |
Popupmenu | `popupmenu.c` | - `popupmenu()` | WindTop 3.02 | 使用主選單系統的資料結構，但使用不同的特殊值
Popupmenu Prompt | `popupmenu.c` | - `popupmenu_ans()` <br> - `pmsg()` | WindTop 3.02 | - 實質上與 Popupmenu 不是同一系統 <br> - 本文特稱之為 `Popupmenu Prompt` <br> - 使用次數稀少
Window | `window.c` | - `popupmenu_ans2()` <br> - `pmsg2()` | MapleBBS-itoc | - 與 Popupmenu Prompt 系統相似 <br> - 在 MapleBBS-itoc 中無 `2` 後綴

其中本文主要介紹的是主選單與 Popupmenu 2 種使用相同資料結構的系統。

### 主選單重繪指令
　   | 值  | 出處 | 說明
--- | --- | --- | ---
`MENU_LOAD` | - `1` <br> - 移除 (DreamBBS v3) | MapleBBS 3 | DreamBBS v3 中以 `XR_PART_LOAD` 取代
`MENU_DRAW` | - `2` <br> - 移除 (DreamBBS v3) | MapleBBS 3 | DreamBBS v3 中以 `XR_PART_HEAD \| XR_BODY` 取代
`MENU_FILM` | - `4` <br> - 移除 (DreamBBS v3) | MapleBBS 3 | DreamBBS v3 中以 `XR_PART_NECK` 取代

DreamBBS v3 改用 Xover 指令指定主選單畫面的重繪。

### 主選單所呼叫函數之 return 值
　      | 值      | 出處 | 說明
  ---   |  ---    | --- | ---
`XEASY` | - `0x333` <br> - `XO_FOOT` (DreamBBS v3) | Phoenix BBS (?) | 回到選單後只重繪畫面底部
`QUIT`  | - `0x666` <br> - 移除 (PttBBS current & MapleBBS-itoc) <br> - `XO_QUIT` (DreamBBS v3) | Pirate BBS | 強制退出選單
`SKIN`  | - `0x999` <br> - `XO_SKIN + {XO_WRAP\|XO_SCRL\|XO_REL} + idx` (DreamBBS v3) | WindTop BBS 3.02 | 切換 skin (未實作)
(其它)  | - `-1` - `'\a'` (`7`)  | ---  | 回到選單後重繪整個畫面 (Deprecated in DreamBBS v3)
(其它)  | (其它)  | ---  | - 回到選單後重繪整個畫面 <br> - 當作 Xover 指令執行 (DreamBBS v3)

DreamBBS v3 的主選單改為接受 Xover 指令。

### MapleBBS 3 / WindTop 3.02 主選單系統之資料結構
(取自 DreamBBS v3)
```cpp
typedef struct MENU
{
    MenuItem item;
    unsigned int level;
    int umode;
    const char *desc;
} MENU;
```
`MenuItem item` 為一 union 物件；`item` 在 MapleBBS 3 / WindTop 3.02 中原為 `void *func`，實際使用時須依 `umode` 及 `level` 的值手動轉型。

DreamBBS v2.1 時將其改為 union，以減少手動轉型錯誤的可能。

Member 名稱 | 型別               | `umode` 指定方式 (主選單) | `umode` 指定方式 (Popupmenu) | 說明
---         | ---               | ---                       | ---                         | ---
`func`      | `int (*)(void)`   | `umode` <br> (`(umode & M_MASK) > M_XMENU`) | `POPUP_FUN`                | 無參數函數
`funcarg`   | `FuncArg`         | `umode \| M_ARG`          | `POPUP_FUN \| POPUP_ARG`    | - 帶參數的函數物件 <br> - DreamBBS v3 新增
`xofunc`    | `int (*)(XO *xo)` | 無                        | `POPUP_XO`                  | - Xover 函數 <br> - 未使用
`dlfunc`    | - `const char *` (使用動態載入) <br> - `int (*)(void)` (不使用動態載入) | `M_DL(umode)` | - `M_DL(umode)` <br> - `POPUP_SO` | - 需動態載入的函數 <br> - Popupmenu 使用 `M_DL(umode)` 時，會看 `umode` 決定實際型別
`dlfuncarg` | `DlFuncArg`       | `M_DL(umode \| M_ARG)`    | `M_DL(POPUP_FUN \| POPUP_ARG)` | - 帶參數的函數物件，函數需動態載入 <br> - DreamBBS v3 新增
`title`     | `const char *`    | 無                        | `POPUP_MENUTITLE`            | - 顯示在彈出式選單上的選單標題 <br> - 表示 Popupmenu 使用的 `MENU` 列表的最末項
`menu`      | `struct MENU *`   | `umode` <br> (`(umode & M_MASK) <= M_XMENU`) | `POPUP_MENU`              | 內層選單

Member 名稱 | 型別               | `level` 指定方式 (主選單) | `level` 指定方式 (Popupmenu) | 說明
---         | ---               | ---                       | ---                         | ---
`menu`      | `struct MENU *`   | `PERM_MENU + key`         | 無                          | - 上層選單 <br> - 表示主選單系統使用的 `MENU` 列表的最末項

### WindTop 3.02 主選單系統使用的特殊值
Macro (主選單) | 值 (主選單)   | Macro (Popupmenu)  | 值 (Popupmenu) | 使用對象 | 說明
---            | ---          | ---                | ---            | ---     | ---
無             | ---          | `POPUP_QUIT`       | `0x00`         | `umode` | 選擇後退出本層選單
無             | `umode` <br> (`(umode & M_MASK) > M_XMENU`) | `POPUP_FUN` | `0x01` | `umode` | 選擇後執行函數
無             | ---          | `POPUP_XO`         | `0x02`         | `umode` | 選擇後執行 Xover 函數
無             | `umode` <br> (`(umode & M_MASK) <= M_XMENU`) | `POPUP_MENU` | `0x04` | `umode` | 選擇後進入內層選單
無             | ---          | `POPUP_MENUTITLE`  | `0x08`         | `umode` | - 非選項；設定選單標題 <br> - 要在 `MENU` 列表的最末項出現
無             | ---          | `POPUP_SO` | - `0x10` (使用動態載入) <br> - `POPUP_FUN` (不使用動態載入) | `umode` | 選擇後執行無參數函數；此函數需要動態載入
`M_ARG`        | `0x40000000` | `POPUP_ARG`        | `0x40000000`   | `umode` | - 執行函數時，以函數物件所帶的參數呼叫函數 <br> - DreamBBS v3 新增
`M_DL(umode)`  | - `-umode` (使用動態載入) <br> - `umode` (不使用動態載入) | `M_DL(umode)` | - `-umode` (使用動態載入) <br> - `umode` (不使用動態載入) | `umode` | - 此選項功能需要動態載入 <br> - DreamBBS v2.0 時新增
`M_MASK`       | `0x0000FFFF` |                    |                | `umode` | - `umode` 的有效範圍；範圍之外為 flags <br> - DreamBBS v3 新增
無             |              | `POPUP_MASK`       | `0x000000FF`   | `level` | `level` 的有效範圍；範圍之外為 flags
`PERM_MENU`    | `PERM_PURGE` (`0x00800000`) | 無  | ---            | `level` | - 非選項；退出本層選單後將跳到該選單 <br> - 要在 `MENU` 列表的最末項出現
無             | ---          | `POPUP_DO_INSTANT` | `0x01000000`   | `level` | 透過指令配對跳到此選項時，立即執行對應功能

### User Mode、Menu Index、動態看板編號
#### User Mode (無 `M_` 前綴)
出處：Pirate BBS

MapleBBS 3 中改為有 `M_` 前綴，已不存在。

在 PttBBS current 中則保留原名，仍為 user mode。

#### User Mode / Menu Index (`M_*`)
出處：Eagles BBS (?)

(於 MapleBBS 2.36 及 FireBird 3.1 中皆不存在；應為 PttBBS current 與 MapleBBS 3 各自引入)

在 MapleBBS 3 中為 user mode，同時為 menu index；動態看板編號另外由 `FILM_*` 指定。

在 PttBBS 中為 menu index，同時為動態看板編號。

　               | User Mode    | Menu Index  | 動態看板編號
 ---             | ---          | ---         | ---
Pirate BBS (1.9) | 無 `M_` 前綴 | 無 `M_` 前綴 | (無動態看板)
Eagles BBS (3.1) | `M_*`        | `M_*`       | (無動態看板)
MapleBBS 3       | `M_*`        | `M_*`       | `FILM_*`
PttBBS current   | 無 `M_` 前綴 | `M_*`       | `M_*`