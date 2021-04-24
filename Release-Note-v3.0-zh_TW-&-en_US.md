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

+ 現在改變畫面大小時會進行畫面自動重繪\
  Now an auto redraw is performed when the screen size is changed
  亦可按 Esc-Ctrl-L 手動重繪\
  Esc-Ctrl-L can be used for manual redrawing if needed

+ 已實作伺服器端雙位元字元偵測\
  Server-side DBCS character detection is now implemented

  同時，BBS 使用者端所送出的雙位元字元按鍵自動重複會被偵測出並忽略\
  Also, the auto repeats for DBCS character are detected and ignored

  在編輯器中，Esc-r 可暫時開關雙位元字元偵測\
  In the editor, Esc-r can temporarily toggles DBCS character detection

### 錯誤修正 Bug Fixes

* 修正使用者的文章閱讀紀錄（BRHs）會因為 `memcpy()` 在較新的作業系統上的未定義行為而損壞的問題\
  Fix users' article-reading record (BRHs) corrupted due to the undefined behavior of `memcpy()` on recent OSs

* 修正編輯器的編輯行為問題
  Fix the editing behavior issues of the editor

### 程式架構改變 Program Architecture Changes

+ 支援使用帶有 GCC 擴充語法的 C++ 模式編譯\
  Support compilation with C++ mode with GCC extensions

- 移除未使用的 WindTop BBS 式我的最愛系統 (`Favorite`)\
  Remove unused WindTop-BBS–style favorite system (`Favorite`)\
  目前使用的是類似 MapleBBS-itoc 的我的最愛系統\
  The system currently in use is MapleBBS-itoc–like favorite system

+ 支援 DSO（動態載入函式庫）的熱插拔\
  Support hot-swapping of DSO (dynamically-loaded library)\
  本功能允許在函式宣告未變更的前提下，直接取代安裝的函式庫檔案，不必重新啟動 `bbsd` 就能使編譯為 DSO 的程式中的變更生效\
  This feature allows changes in programs compiled as DSO to become effective by directly replacing the installed library file without relaunching `bbsd`
  在 `dreambbs.conf` 中定義 `DL_HOTSWAP` 以啟用此功能\
  Define `DL_HOTSWAP` in `dreambbs.conf` to enable this feature
  如果設定了 Makefile 變數 `NO_SO` 以將 `so/` 下的程式編譯為靜態連結函式庫，則此功能無效\
  This feature is disabled if Makefile variable `NO_SO` is set to build programs under `so/` as static-linking libraries
  本功能會使 DSO 中的全域變數在每次進入函式庫的主函式時重置，請注意使用\
  This feature makes global variables in DSO reset every time the main library function is entered, please use it with caution

* 支援在 64 位元環境下原生編譯而不影響資料結構的記憶體佈局\
  Support native compilation under 64-bit environment without disrupting the memory layout of data structures
  強制使用固定寬度的整數型別作為硬碟或 SHM（共用記憶體）中的資料結構的成員\
  Force using fixed-width integer types for structure members for data structures used in in hard disk or SHM (shared memory)
  改寫硬碟或 SHM 中使用的資料結構中的指標為索引值\
  Change pointer members in data structures used in hard disk or SHM to use index values instead

+ 現已完全支援 IPv6。由於資料結構 `UTMP` 已更改，請務必使用系統殼層命令 `ipcrm` 重開 SHM（共用記憶體）。\
  IPv6 is now fully supported. Please reload the SHM (shared memory) using system shell command `ipcrm`, since the data structure `UTMP` has been modified.\
  相關程式已全面改用 `getaddrinfo()`\
  Relevant codes now use `getaddrinfo()` instead
  `lib/dns.c` 亦已實作 IPv6 地址之 AAAA 紀錄的 DNS 反查\
  DNS lookup for AAAA records of IPv6 addresses is implemented in `lib/dns.c` as well