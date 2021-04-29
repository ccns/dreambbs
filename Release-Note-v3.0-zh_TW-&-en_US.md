# [WIP] DreamBBS-202X v0-Azure 發佈說明 Release Note

## 版本名稱 Version Name

DreamBBS-202X v0-Azure (或稱 v3.0/3.21) 是續 DreamBBS v2.0-Artoria (或稱 3.12) 的下一個主要發佈版本。\
DreamBBS-202X v0-Azure (also v3.0/3.21) is the next major release after DreamBBS v2.0-Artoria (also 3.12).

## 改變說明 Changes

有關從 v2.0 到此版本的改變細節，請見此發佈版本的計畫專案中的各個 pull request 的說明：\
For details of changes from v2.0 until now, please refer to the description of pull requests of the project for this release:\
https://github.com/ccns/dreambbs/projects/4

有關快速鍵的改變，請見：\
For hotkey changes, please refer to:\
https://github.com/ccns/dreambbs/wiki/VS-MapleBBS-3-Keyboard-zh_tw

以下說明從 v2.0 的重大改變。\
The major changes from v2.0 are explained below.

### 使用者介面改變 UI Changes

+ 使用者閒置時間精度現為秒，其顯示範圍現涵蓋 32 位元整數\
  The users' idle time now has the resolution of one second as well as the display range of 32-bit integer

+ 現在聊天室的聊天訊息一律採用使用向上捲動的顯示方式\
  Now the chat messages in the chatroom are always displayed as being scrolled upward

* 現在進入聊天室時，聊天暱稱欄會預先填入使用者 ID，而送出空暱稱可取消進入聊天室\
  Now, when the user enters the chatroom, the input field for the user's nickname in chatroom will be pre-filled with the user's ID, and sending an empty nickname cancels the entry

+ 現在被 zapped 掉的看板及其文章會被顯示為已讀\
  Zapped boards and all posts in them are now displayed as read

+ 改進了編輯器在 ANSI 預覽模式中，對編輯按鍵的處理\
  Improve the handling of editing keys in ANSI preview mode in the editor

  已可在編輯器中的 ANSI 預覽模式中直接使用倒退鍵\
  Backspace can now be used directly in the ANSI mode in the editor

+ 現在當看板分類文字色彩設定為黑色時，會顯示為亮黑色以避免看不見\
  Now, when the color of the board category text is set as black, it is displayed as bright black to prevent it from being invisible

+ 加大了使用者名單中的「故鄉」（使用者的網際網路地址訊息）欄位的最大寬度，請使用寬螢幕看看。\
  The maximum width of the "homeland" (users' internet address information) field in the online user list has been increased. Please check it out with a wide screen.

+ 增加自動生成的程式版本與編譯環境資訊\
  Add automatic generated program version and compilation environment information

  可從 BBS 介面主選單選擇以下選項以查看：\
  To view it, please choose the following menu item from the main menu:
  - `(X)yz` (系統資訊區 System Information Area)\
    -> `(X)info` (系統程式資訊 System Environment and Program Information)

+ 現在使用者上站紀錄的 IP 地址會被印成標準格式，並且連線類型在由 WebSocket 連入時會顯示為 `WSP` (WebSocket Proxy)\
  Now, in the user's login history, the IP is printed in the standard format, and the connection type is displayed as `WSP` (WebSocket Proxy) if the user connects via WebSocket

+ 改進多重選項設定介面（`bitset()`），並防止使用者更改系統保留的設定\
  Refine the UI of multiple selection setting (`bitset()`) and prevent the user from changing options reserved by the system

+ 使用者／版主的資料查詢與設定介面（`acct_setup()`/`bm_setup()`），現在在送出查詢理由前會維持隱藏隱私資料，並且在查詢完畢後會清除畫面\
  The UI for querying and setting user/board moderator (`acct_setup()`/`bm_setup()`) now hides the private information before the reason for querying is given and clears the screen when the query is done

+ 選擇使用者／系統檔案以編輯的介面（`x_file()`）改為使用主選單函式 (`domenu()`，原 `menu()`) 實作\
  The UI for choosing user/system files (`x_file()`) for editing is reimplemented with the main menu function (`domenu()`, formerly `menu()`)

+ 實作 pfterm 的黑白模式\
  Implement the Black-and-White mode for pfterm

  可從 BBS 介面主選單選擇以下選項以使用功能：\
  To access this function, please choose the following menu item from the main menu:
  - `(D)Service` (服務區 Service Area)\
    -> `(U)ser` (個人工具區 User Utilities)\
    -> `(C)onfigure` (註冊及設定個人資訊 Register and User Information Setting)\
    -> `(F)avorite` (個人喜好設定 User Preference)\
    -> `0 □` (彩色模式 Color Mode)

  並使該選項的圖案變為空心 And make the icon for the option unfilled: `□`

+ 現在 pfterm 支援更多彩現屬性\
  pfterm now supports more render attributes

  v2.0 引進的 Virtual Attribute 已更名為 Cursor Attribute\
  The Virtual Attribute introduced in v2.0 has been renamed to Cursor Attribute

  有關完整的新增支援的彩現屬性清單，請見：\
  For the complete list of newly supported render attributes, please refer to:\
  https://github.com/ccns/dreambbs/pull/61

+ 引進 PttBBS 式的光棒選單系統\
  Introduce the PttBBS-style lightbar menu system

  可從 BBS 介面主選單選擇以下選項以啟用：\
  To enable it, please choose the following menu item from the main menu:
  - `(D)Service` (服務區 Service Area)\
    -> `(U)ser` (個人工具區 User Utilities)\
    -> `(C)onfigure` (註冊及設定個人資訊 Register and User Information Setting)\
    -> `(F)avorite` (個人喜好設定 User Preference)\
    -> `K ■` (使用光棒選單系統 Enable Lightbar Menu System)

  並使該選項的圖案填滿 And make the icon for the option filled: `■`

  如果 pfterm 未啟用，則本功能無效。\
  This feature is disabled when pfterm is not enabled.

+ 現在標題列的中央標題會儘可能向整個標題列的中央置中\
  Now the middle title on the title bar is placed on the entire title bar as close to the center as possible

+ 轉信設定介面改用 `xover()` 實作，可使用 Xover 列表的預設快速鍵\
  The UI of internet article sending/receiving setting is reimplemented using `xover()`, so the default hotkeys of Xover list can be used

+ 重新實作 Xover 列表的 footer 顯示，改善 Xover 快速訊息的顯示\
  Reimplement the footer display in Xover list and refine the display of Xover quick messages

  現在執行會改變 footer 所在橫行的顯示的操作後，會正確地重繪 footer 了\
  Now the footer is redrawn correctly after operations which alter the display of the row for the footer

  現在 Xover 快速訊息出現時，按任意按鍵會清除此訊息並重繪回 footer\
  Now, when the Xover quick messages appear, pressing any key causes the message to be cleared and the footer to be redrawn

+ 改善 `vget()` 輸入框系統的歷史紀錄處理\
  Refine the history entry handling of the `vget()` input field system

  不能再向後跳到空白紀錄或向前跳到最舊紀錄了。\
  It is no longer possible to jump backward to empty entries or jump forward to the oldest entry.

  已編輯的新輸入與歷史在前後切換歷史時會被儲存，而送出為輸入的歷史會變成最新的歷史。\
  Edited new input and history entries are saved when switching between the history entry, and the history entry which sent as the input becomes the newest entry.

+ 現在改變畫面大小時會進行畫面自動重繪\
  Now an automatic redraw is performed when the screen size is changed

  亦可按 <kbd>Esc</kbd>-<kbd>Ctrl</kbd>-<kbd>L</kbd> 手動重繪\
  <kbd>Esc</kbd>-<kbd>Ctrl</kbd>-<kbd>L</kbd> can be used for manual redraws if needed

+ 改進文章標題色彩突顯系統 (`hdr_outs()`)\
  Improve the color-highlighting system for article titles (`hdr_outs()`)

  現在文章標題色彩突顯會正確處理雙位元字元\
  Now the color-highlighting of article titles correctly handles DBCS characters

  同時使相關程式碼更簡潔並易讀\
  Also improve the conciseness and readability of relevant codes

+ 已實作伺服器端雙位元字元偵測\
  Server-side DBCS character detection is now implemented

  同時，BBS 使用者端所送出的雙位元字元按鍵自動重複會被偵測出並忽略\
  Also, the auto repeats for DBCS character are detected and ignored

  在編輯器中，Esc-r 可暫時開關雙位元字元偵測\
  In the editor, Esc-r can temporarily toggles DBCS character detection

### 錯誤修正 Bug Fixes

* 修正使用者的文章閱讀紀錄（BRHs）會因為 `memcpy()` 在較新的作業系統上的未定義行為而損壞的問題\
  Fix users' article-reading record (BRHs) corrupted due to the undefined behavior of `memcpy()` on recent OSs

* 修正其它執行資料會因為 `memcpy()` 在較新的作業系統上的未定義行為而損壞的問題\
  Fix other runtime data corrupted due to the undefined behavior of `memcpy()` on recent OSs

* 修正 visio screen 的畫面備份機制在寬／高螢幕下以及螢幕大小變更時，不能正確運作的問題\
  Fix the screen backup mechanism of visio screen was broken for wide/tall screens and resizing screen

* 修正 Xover 列表項目在某些操作後的顯示樣式不正確的問題\
  Fix incorrect appearance of Xover list items after certain operations
  
  現在單一項目的顯示有所更動時，會一律重新繪製此項目\
  Now the redrawing of the entire item is always performed when the display of the item alters

* 修正編輯器的游標在空行或只有空白的行的前一行行尾時，按下 <kbd>Del</kbd> 無效的問題\
  Fix <kbd>Del</kbd> not worked at the end of a line above an empty or space-only line

* 修正編輯器的游標位於非檔案第一行的畫面第一行的行首時，按下倒退鍵會導致程式崩潰的問題\
  Fix crash when pressing backspace at the beginning of the topmost line which is not the beginning of a file in the editor

* 修正在高螢幕下，閱讀文章時進入並退出使用者列表，可能會產生暫時甚至永久的無效文章項目的問題\
  Fix possible generation of temporary or even persistent corrupted article entries when the user enter and exit the online user list while using a tall screen

* 修正將線上使用者自動停權所使用的函式（`remove_perm()`）只會將使用者的對系統內的前 1750（`MAXBOARD/4`）個看板的寫入權限移除的問題\
  Fix the function for automatically suspending online users (`remove_perm()`) only removed their write permission of the first 1750 (`MAXBOARD/4`) boards in the system

* 修正信件列表中的 tag 記號在畫面重繪後會消失的問題\
  Fix the tag marks in the mail list disappeared after redraws

* 修正群組名單搜尋（`list_search()`）的目標不在第一頁時會跳到錯誤項的問題\
  Fix the issue that the searching function for the contact group list (`list_search()`) jumped to the wrong item when the target is not in the first page

* 修正了 BBS-Lua 與 BBS-Ruby 的問題\
  Fix issues about BBS-Lua and BBS-Ruby

  詳細的問題修正可見：\
  For details of fixed issues, please refer to:\
  https://github.com/ccns/dreambbs/pull/61

* 改善寵物雞遊戲的程式碼與修正諸多邏輯錯誤\
  Refine the code of the digital pet game and fix many logic errors

  詳情請見：\
  For details, please refer to:\
  https://github.com/ccns/dreambbs/pull/61

* 修正文章轉綠時間變為上次推文時間的問題\
  Fix the date of article reposting became the date of the last comment

* 修正張爸關鍵字的偵測會產生未定義結果，造成關鍵字被隨機允許的問題\
  Fix the Zhangba pattern matching produced undefined result and wrongly, randomly allowing some patterns

* 修正 rxvt 式的 <kbd>Ctrl</kbd>-方向鍵被誤認為 <kbd>Shift</kbd>-方向鍵的問題
  Fix rxvt-style <kbd>Ctrl</kbd>-arrow keys were misinterpreted as <kbd>Shift</kbd>-arrow keys

### 專案部屬工具改變 Project Deployment Tool Changes

+ Make 腳本新增了設定自動推導機制，可推測 Unix 使用者 UID、GID、家目錄等等的設定\
  Add configuration deduction mechanism to the Make script which can deducing configurations such as Unix user's UID, GID, and home directory.

+ 現在會自動設定安裝的檔案的擁有者與權限\
  The owner and permission for install files are now configurated automatically

+ 已可用 CMake 建置 DreamBBS\
  DreamBBS can now be built with CMake

  **BSD Makefile 檔案已被棄用，將在 v3.1 時被移除。請改用 CMake 建置本專案。**\
  **BSD Makefiles are now deprecated and will be removed in v3.1. Please build this project with CMake instead.**

+ `wsproxy` 模組不再需要 `vstruct` 函式庫\
  The `wsproxy` module no longer requires the library `vstruct`

  此修改已移植回 PttBBS：\
  This change has been backported to PttBBS:\
  https://github.com/ptt/pttbbs/pull/87

- 移除專案中 Travis CI 的設定檔\
  Remove the configure file for Travis CI from the project

### 程式架構改變 Program Architecture Changes

+ 支援使用帶有 GCC 擴充語法的 C++ 模式編譯\
  Support compilation with C++ mode with GCC extensions

- 移除未使用的 WindTop BBS 式我的最愛系統 (`Favorite`)\
  Remove unused WindTop-BBS–style favorite system (`Favorite`)

  目前使用的是類似 MapleBBS-itoc 的我的最愛系統\
  The system currently in use is MapleBBS-itoc–like favorite system

+ 現已完全支援 IPv6\
  IPv6 is now fully supported

  **由於資料結構 `UTMP` 已更改，請務必使用系統殼層命令 `ipcrm` 重開 SHM（共用記憶體）。**\
  **Please reload the SHM (shared memory) using system shell command `ipcrm`, since the data structure `UTMP` has been modified.**

  相關程式已全面改用 `getaddrinfo()`\
  Relevant codes now use `getaddrinfo()` instead

  `lib/dns.c` 亦已實作 IPv6 地址之 AAAA 紀錄的 DNS 反查\
  DNS lookup for AAAA records of IPv6 addresses is implemented in `lib/dns.c` as well

+ `vkey()` 按鍵輸入系統已加入延時判斷機制\
  The delayed key determination mechanism is implemented in the `vkey()` key input system

  現在一律定義 `TRAP_ESC` 以正確處理被 `Esc` 中斷的不確定是否完整的按鍵序列碼。\
  `TRAP_ESC` is now always defined to correctly handle key sequences which may not be complete.

+ 引進畫面大小參考座標系統\
  Introduce the Screen-Size–Referencing Coordination System

  此座標系中的固定座標值能隨畫面大小對應不同的實際座標。\
  In this coordination system, a fixed coordination value can map to different actual coordination values depends on the screen size.

  範例 Examples:
  - 畫面底部橫行 The bottom line of the screen: `(T_LINES_REF-1, 0)` == `(B_LINES_REF, 0)`
  - 畫面中央 The screen center: `(T_LINES_REF/2, T_COLS_REF/2)`

  其中 `T_LINES_REF`（對應畫面高度）與 `T_COLS_REF`（對應畫面寬度）皆為常數。\
  Where `T_LINES_REF` (which maps to the screen height) and `T_COLS_REF` (which maps to the screen width) are both constants.

  對此座標系統的更深入介紹，請見：\
  For a deeper introduction of the coordination system, please refer to:\
  https://github.com/ccns/dreambbs/wiki/Screen-Coordinate-System-zh_tw

  目前 DreamBBS 使用此座標系統處理主選單、彈出式選單、以及 `vget()` 輸入框在畫面大小變更時的重繪。\
  The coordination is currently used in DreamBBS for handling the redrawing of the main menu, the popup menu, and the `vget()` input field on screen resizing.

+ 改進並擴充 Xover 列表系統\
  Improve and extend the Xover list system

  **Xover 列表系統所使用的特殊值定義已變更，請重新編譯。**\
  **The special values used in Xover list system are reassigned, please recompile.**

  重新設計的 Xover 特殊值分配方式避免了游標移動的負值問題，並可用回傳值同時指定重繪與按鍵操作。\
  The redesigned Xover special value scheme avoids the negative issue of cursor movement, and allows specifying the redraws and key operations at the same time.

  改進後的 Xover 列表系統使用 `mmap()` 一次載入整個列表的資料。\
  The improved Xover list system uses `mmap()` to load the data of the entire list at once.

  引進新的特殊值，包含而不限於：\
  New special values are introduced, including but not limited to:
  - `XO_REL + diff`: 相對移動 Relative move
  - `XO_SCRL + move`: 行捲動 Scroll by rows
  - `XO_CUR + diff`: 重繪單項並相對移動 Redraw a single item and relative move
  - `XO_POSF + key`: 表示按鍵對應的回呼函式有位置參數 Specify that the callback function for the key has a position parameter

  有關 Xover 列表系統變更前後的詳細說明，請見：\
  For details for Xover list system before and after the change, please refer to:\
  https://github.com/ccns/dreambbs/wiki/Xover-List-System-zh_tw

  有關詳細變更的介紹，請見：\
  For the introduction of the changes in detail, please refer to:\
  https://github.com/ccns/dreambbs/pull/61

+ 改進並擴充主選單系統，使其它外觀相似的選單能共用其函式\
  Improve and extend the main menu system to make the functions able to be reused for other menu with similar appearance

  **主選單系統與 WindTop BBS 的彈出式選單系統所使用的特殊值定義已變更，請重新編譯。**\
  **The special values used in the main menu system and WindTop BBS popup menu system are reassigned, please recompile.**

  將主選單系統與 WindTop BBS 的彈出式選單系統所使用的特殊值整合為一\
  Merge the special values used for the main menu system and the WindTop BBS popup menu system

  有關主選單系統的特殊值變更前後的詳細說明，請見：\
  For details for the special values used in main menu before and after the change, please refer to:\
  https://github.com/ccns/dreambbs/wiki/Menu-Systems-zh_tw

  有關主選單的詳細行為變更，請見：\
  For details for behavior changes of the main menu, please refer to:\
  https://github.com/ccns/dreambbs/pull/61

+ 改進轉信程式 (`innbbsd`) 的時間格式解析邏輯 (`parse_date()`)，使之能處理 RFC-1123 所允許的除文字時區與任意括號外的其它時間格式\
  Improve the date parsing logic (`parse_date()`) of the internet article sending/receiving program (`innbbsd`) to make it handles other date formats permitted by RFC-1123 other than the time zone texts and arbitrary parentheses

+ 現在用 GNU C 或 ISO C++11 編譯時，可以檢查從 DSO（動態載入物件）函式庫載入的物件的型別是否正確\
  Now the type correctness of objects from DSO (dynamic shared object) libraries can be checked when compiling with GNU C or ISO C++11

+ 支援 DSO 函式庫的熱插拔\
  Support hot-swapping of DSO libraries

  本功能允許在函式宣告未變更的前提下，直接取代安裝的函式庫檔案，不必重新啟動 `bbsd` 就能使編譯為 DSO 函式庫的程式中的變更生效\
  This feature allows changes in programs compiled as DSO libraries to become effective by directly replacing the installed library file without relaunching `bbsd`

  在 `dreambbs.conf` 中定義 `DL_HOTSWAP` 以啟用此功能\
  Define `DL_HOTSWAP` in `dreambbs.conf` to enable this feature

  如果設定了 Makefile 變數 `NO_SO` 以將 `so/` 下的程式編譯為靜態連結函式庫，則此功能無效\
  This feature is disabled if Makefile variable `NO_SO` is set to build programs under `so/` as static-linking libraries

  **本功能會使 DSO 函式庫中的全域變數在每次進入函式庫的主函式時重置，請注意使用。**\
  **This feature makes global variables in DSO libraries reset every time the main library function is entered, please use it with caution.**

+ 將 `util/account.c` 中的看板分類區建立工作移至 `util/acpro.c`（WindTop BBS 用於建立專業討論區的程式）中\
  Move the task of building board category area from `util/account.c` to `util/acpro.c` (The program for building professional board area in WindTop BBS)

  **執行 `bbsd` 前不再需要執行 `account`。請將執行 `bbsd` 用的腳本中的 `account` 執行命令移除。**\
  **`account` is no longer needed to run before `bbsd` is launched. Please remove the command for running `account` in scripts for launching `bbsd`**

  移除看板分類區在一天的特定時段中不能重建的限制\
  Lift the limitation that the board category area should not be rebuilt in the certain time period of a day

* 支援在 64 位元環境下原生編譯而不影響資料結構的記憶體佈局\
  Support native compilation under 64-bit environment without disrupting the memory layout of data structures

  **由於資料結構 `UCACHE` 中的一些資料欄位定義已更改，請務必使用系統殼層命令 `ipcrm` 重開 SHM（共用記憶體）。**\
  **Please reload the SHM (shared memory) using system shell command `ipcrm`, since some data fields in the data structure `UCACHE` has been redefined.**

  使用固定寬度的整數型別作為硬碟上與 SHM（共用記憶體）中儲存的資料結構的成員\
  Use fixed-width integer types for structure members for data structures stored on hard disk or in SHM (shared memory)

  在 64 位元環境中，內部會使用 64 位元寬的 `time_t` 處理時間，而固定寬度的時間型別僅會在上述情況使用。\
  64-bit `time_t` is used for internal time processing in 64-bit environment, while fixed-width time types is used only for the condition above.

  改寫硬碟上與 SHM 中儲存的資料結構中的指標為索引值\
  For data structures stored on hard disk and in SHM, change their raw pointers to integer indexes

## 與程式風格相關的改變 Coding-Style–Related Changes

+ 將會存入硬碟的資料結構的定義加上 `DISKDATA` 註解，將其它會放入 SHM（共用記憶體）的資料結構的定義加上 `SHMDATA` 註解，方便未來資料結構轉換時的參考\
  Add `DISKDATA` comments to the definition of data structures which are stored on the hard disk and `SHMDATA` comments to the definition of other data structures which are loaded into the SHM (shared memory) for the reference of future data structure migrations

+ 限制位元運算的使用，以改進程式可讀性\
  Limit the use of binary operations to improve code readability

  現代的編譯器會將部份乘除運算替換成位元運算，不需要手動在程式碼中替換\
  Since modern compilers substitute some multiplications/divisions with binary operations, there is no need to substitute them in the code by hand

+ 強化 `const` 限定詞的使用\
  Enforce the use of `const` qualifier

+ 新增 macros 以使用 ISO C99 與 ISO C++11 中功能相近但形式不同的語法\
  Add macros to use syntaxes with the similar functionality but in different forms between ISO C99 and ISO C++11

- 移除毫無意義地寫死了的顯式地指定了的陣列大小\
  Remove unmeaningfully hardcoded explicit array size

* 修正用 `#if 0` 取代標準註解語法當作文字註解的問題\
  Fix `#if 0` replaced the standard comment syntaxes for text comments

## 統計資訊 Statistic Information

本發佈版本的專案大小 The project size of this release:
- 檔案數 Files: 226
- 行數 Lines: 105980
- 位元組大小 Byte size: 2,778,572 B (≈ 2.65 MB)

與 v2.0.0 的程式碼差異（排除空白字元的改變）The code differences from v2.0.0 (excluding whitespace changes):
- Commits: 713
- 修改檔案數 File changed: 218 (≈ 96.5%)
- 增加行數 Line insertions: 20323 (≈ 19.2%)
- 刪除行數 Line deletions: 17632 (≈ 16.6%)
