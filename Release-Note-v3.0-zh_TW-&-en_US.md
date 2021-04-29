# [WIP] DreamBBS-202X v0-Azure 發佈說明 Release Note

## 版本名稱 Version Name

DreamBBS-202X v0-Azure (或稱 v3.0/3.21) 是續 DreamBBS v2.0-Artoria (或稱 3.12) 的下一個主要發佈版本。\
DreamBBS-202X v0-Azure (also v3.0/3.21) is the next major release after DreamBBS v2.0-Artoria (also 3.12).

## 改變說明 Changes

有關從 v2.0 到此版本的改變細節，請見此發佈版本的計畫專案中的各個 pull request 的說明：\
For details of changes from v2.0 until now, please refer to the description of pull requests of the project for this release:\
<https://github.com/ccns/dreambbs/projects/4>

以下說明從 v2.0 的重大改變。\
The major changes from v2.0 are explained below.

### 使用者介面改變 UI Changes

+ 使用者閒置時間精度現為秒，其顯示範圍現涵蓋 32 位元整數\
  The users' idle time now has the resolution of one second as well as the display range of 32-bit integer

* 現在進入聊天室時，聊天暱稱欄會預先填入使用者 ID，而送出空暱稱可取消進入聊天室\
  Now, when the user enters the chatroom, the input field for the user's nickname in chatroom will be pre-filled with the user's ID, and sending an empty nickname cancels the entry

+ 現在被 zapped 掉的看板及其文章會被顯示為已讀\
  Zapped boards and all posts in them are now displayed as read

+ 已可在編輯器中的 ANSI 預覽模式中直接使用倒退鍵\
  Backspace can now be used directly in the ANSI mode in the editor

+ 現在改變畫面大小時會進行畫面自動重繪\
  Now an auto redraw is performed when the screen size is changed

  亦可按 Esc-Ctrl-L 手動重繪\
  Esc-Ctrl-L can be used for manual redrawing if needed

+ 改進文章標題色彩突顯系統 (`hdr_outs()`)\
  Improve the color-highlighting system for article titles (`hdr_outs()`)

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

* 修正 visio screen 的畫面備份機制在寬／高螢幕下以及螢幕大小變更時，不能正確運作的問題
  Fix the screen backup mechanism of visio screen broken for wide/tall screens and resizing screen

* 修正 Xover 列表項目在某些操作後的顯示樣式不正確的問題\
  Fix incorrect appearance of Xover list items after certain operations
  
  現在單一項目的顯示有所更動時，會一律重新繪製此項目\
  Now the redrawing of the whole item is always performed when the display of the item alters

* 修正編輯器的編輯行為問題\
  Fix the editing behavior issues of the editor

  詳細的行為修正可見：\
  For details of fixed behaviors, please refer to:\
  https://github.com/ccns/dreambbs/pull/58

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

### 程式架構改變 Program Architecture Changes

+ 支援使用帶有 GCC 擴充語法的 C++ 模式編譯\
  Support compilation with C++ mode with GCC extensions

- 移除未使用的 WindTop BBS 式我的最愛系統 (`Favorite`)\
  Remove unused WindTop-BBS–style favorite system (`Favorite`)

  目前使用的是類似 MapleBBS-itoc 的我的最愛系統\
  The system currently in use is MapleBBS-itoc–like favorite system

+ 支援 DSO（動態載入函式庫）的熱插拔\
  Support hot-swapping of DSO (dynamically-loaded library)

  本功能允許在函式宣告未變更的前提下，直接取代安裝的函式庫檔案，不必重新啟動 `bbsd` 就能使編譯為 DSO 的程式中的變更生效\
  This feature allows changes in programs compiled as DSO to become effective by directly replacing the installed library file without relaunching `bbsd`

  在 `dreambbs.conf` 中定義 `DL_HOTSWAP` 以啟用此功能\
  Define `DL_HOTSWAP` in `dreambbs.conf` to enable this feature

  如果設定了 Makefile 變數 `NO_SO` 以將 `so/` 下的程式編譯為靜態連結函式庫，則此功能無效\
  This feature is disabled if Makefile variable `NO_SO` is set to build programs under `so/` as static-linking libraries

  **本功能會使 DSO 中的全域變數在每次進入函式庫的主函式時重置，請注意使用。**\
  **This feature makes global variables in DSO reset every time the main library function is entered, please use it with caution.**

* 支援在 64 位元環境下原生編譯而不影響資料結構的記憶體佈局\
  Support native compilation under 64-bit environment without disrupting the memory layout of data structures

  **由於資料結構 `UCACHE` 中的一些資料欄位定義已更改，請務必使用系統殼層命令 `ipcrm` 重開 SHM（共用記憶體）。**\
  **Please reload the SHM (shared memory) using system shell command `ipcrm`, since some data fields in the data structure `UCACHE` has been redefined.**

  強制使用固定寬度的整數型別作為硬碟上與 SHM（共用記憶體）中儲存的資料結構的成員\
  Force using fixed-width integer types for structure members for data structures stored on hard disk or in SHM (shared memory)

  改寫硬碟上與 SHM 中儲存的資料結構中的指標為索引值\
  For data structures stored on hard disk and in SHM, change their raw pointers to integer indexes

+ 現已完全支援 IPv6\
  IPv6 is now fully supported

  **由於資料結構 `UTMP` 已更改，請務必使用系統殼層命令 `ipcrm` 重開 SHM（共用記憶體）。**\
  **Please reload the SHM (shared memory) using system shell command `ipcrm`, since the data structure `UTMP` has been modified.**

  相關程式已全面改用 `getaddrinfo()`\
  Relevant codes now use `getaddrinfo()` instead

  `lib/dns.c` 亦已實作 IPv6 地址之 AAAA 紀錄的 DNS 反查\
  DNS lookup for AAAA records of IPv6 addresses is implemented in `lib/dns.c` as well

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