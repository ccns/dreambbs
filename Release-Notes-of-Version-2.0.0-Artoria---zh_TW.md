# Version 2.0.0 Artoria 發行說明

## 發行代號：Artoria

* 靈感來源：Fate Stay/Night 系列作品的登場角色，職階 Saber。

## 已經反向移植到 v1.x 版本的新增功能與變動

### 直接影響使用者操作及介面的改變

- 現在按鍵 <kbd>H</kbd> 和 <kbd>F1</kbd> 都可以用來打開 pmore/more 的 help 頁面
- 現在在輸入欄位中，可以用按鍵 <kbd>Del</kbd> 刪除目前游標上的字元
- 現在在輸入欄位中的錯誤輸入或操作會產生 bell 提示
- 支援即時偵測 terminal 的大小調整
- 讓 `vs_bar()` 的樣式與 `vs_head()` 一致
- 修正使用者介面中的錯字，並改善部分用字
- 現在大部分的使用者介面中的元素都支援寬螢幕顯示了
- 重新開放 <kbd>Ctrl</kbd>-<kbd>Z</kbd> 選單中的 `我的最愛` 選項
- 現在 <kbd>Ctrl</kbd>-<kbd>Z</kbd> 選單中的 `螢幕擷取` 功能支援寬螢幕了
- 開放當 pfterm 啟用時，<kbd>Ctrl</kbd>-<kbd>Z</kbd> 選單中的 `螢幕擷取` 功能
- 從 PttBBS 增加了編輯器介面的 <kbd>F*</kbd> 和 <kbd>Esc</kbd>-* 快速鍵
- 將閒置時間的格式由 `mmmm` 改為 `hh:mm`
- 現在會踢出閒置過久的 guest
- 現在閒置警告會附帶 bell
- 移除使用者名單介面的無用的大 <kbd>V</kbd> 功能
- 現在系統踢出使用者時，使用者端顯示出斷線訊息後會立刻斷線
- 現在使用者登出時會先進行登出作業，再顯示登出訊息
- 現在轉貼和轉寄文章時會顯示作業不成功或權限不足的文章數量
- 編輯器的快速鍵 <kbd>Ctrl</kbd>-<kbd>U</kbd> 改成輸入 `ESC` 字元；請改用 <kbd>Esc</kbd> <kbd>U</kbd> 叫出使用者名單
- 編輯器增加快速鍵 <kbd>Esc</kbd> <kbd>1</kbd> ~ <kbd>Esc</kbd> <kbd>5</kbd>，分別會讀入 1 ~ 5 號暫存檔的內容

### 直接影響使用者操作及介面的錯誤修正

- 修正 `blog()` 產生格式對齊錯誤的 log files，而造成使用者平均使用時間計算錯誤的問題
- 將系統維護選單的使用者模式從 `M_XMENU` 改為 `M_ADMIN` 以更好地說明使用者狀態
- 將使用者模式 `M_XMENU` 的有誤導之嫌的說明 `"網路連線選單"` 改為 `"工具選單"`
- 修正 `我的最愛` 的使用者狀態顯示錯誤的問題
- 修正使用 pfterm 時，有時畫完輸入欄位後，反色屬性沒有關掉的問題
- 修正多層 popupmenu 的內層在進入時不會被重畫的問題
- 修正在精華區轉貼沒有讀取權限的文章時會 crash 的問題
- 修正 `u_register()`（填寫註冊單）在用 `getfield()` 讀取輸入時，
   使用者的輸入會被寫入到程式的唯讀記憶體而 crash 的問題
- 修正 `x_siteinfo()` (系統程式資訊) 中的資料大小單位 `bytes` 誤植為 `KB` 的問題
- 修正文章轉貼失敗也能增加個人文章數的問題
- 修正 `topusr` 產生的使用者排行榜的標題變成 `%s` 的問題
- 修正編輯器的符號輸入工具因為 buffer overflow 而造成界面損壞，甚至 crash 的問題
- 修正進入好友看板被拒絕時，目前進入的看板的顯示名稱會變成該好友看板的名稱的問題
- 修正 `class_yank2()` (只顯示好友板、秘密板) 無法列出所有好友板和秘密板的問題
- 修正：使用 `class_yank()` (顯示被 zap 掉的板) 或是 `class_yank2()` (只顯示好友板、秘密板) 時，如果沒有相符的看板，就會造成使用者被踢出或是無法進入看板列表的問題。 
- 修正熱門看板只列出看板 SYSOP 的問題；應為列出所有熱門看板
- 修正空的熱門看板列表會造成使用者無法進入分類看板列表的問題

#### 其它介面修正與改進

- 用 `int` 取代 `char` 來儲存 `vkey()` 的回傳值
- 重新指定特殊按鍵的值
- 支援功能鍵 <kbd>F1</kbd> - <kbd>F12</kbd>
- 支援按鍵組合 <kbd>Shift</kbd>-<kbd>Tab</kbd>
- `maple/visio.c`: 重新啟用 `ansi_move()`，並重命名為 `move_ansi()` 以和 pfterm 一致。
- 讓 `maple/visio.c` `grayout()` 的參數和 pfterm 一致
- `so/adminutil.c`: `top()`: 修正 shell 指令 `top` 不能正常執行的問題。
- 移除函數 `clrtohol()`
- 支援更多不同 terminals 的特殊按鍵
- 支援 <kbd>Ctrl</kbd>/<kbd>Meta</kbd>/<kbd>Shift</kbd> 與特殊按鍵的組合
- 修正 `innbbsd` 的 `連線人數過多` 的訊息 `msg_no_desc` 被截斷的問題
- 修正 `innbbsd/inntobbs.h` 中的函數 `HISfetch()` 宣告錯誤的問題

#### 有關 pfterm 的修正

- 修正不使用 pfterm 時，`popupmenu_ans2()` 及 `pmsg2()` 不會讓背景變按的問題
- 修正不使用 pfterm 時，
   如果參數為 `NULL`，`pmsg2()` 不會使用 `vmsg()` 顯示暫停訊息的問題
- 修正：不使用 pfterm 時，
         macros `STANDOUT` 及 `STANDEND` 會展開成多個 statements，
         造成 current 版本的 `vget()` 的 `STEALTH_NOECHO` 模式顯示不正常的問題。
- 修正 pfterm 將 ANSI escape sequence `ESC <ch>` 誤當作 `ESC [ <ch>` 處理的問題
   (如將 `ESC m` 誤當作 `ESC [ m`)

### 密碼安全修正

- 調整偽隨機亂數產生器 (pseudorandom number generators) 的 seeding
- 使用密碼學安全偽隨機亂數產生器
   (cryptographically secure pseudorandom number generator; CSPRNG)
   來提升新隨機產生或加密的密碼的安全
- 修正造成 `xchatd` 將使用者帳號與密碼明碼當作使用者連線來源的邏輯錯誤
- 現在處理完使用者輸入的密碼後，會立刻抹除密碼明碼
- 發站外信時使用的 BBS 站簽章：在處理完 BBS 站的私鑰後，會立刻抹除私鑰
- 解決：開啟 `CHAT_SECURE` 編譯選項時，
         會造成程式將使用者密碼明碼儲存在全域變數中的問題。
- 解決：因檢查密碼後會抹除密碼明碼，
         造成存儲密碼明碼的全域變數無法用以登入 xchatd 的問題。

### 其它系統安全修正

- 修正 38 處存取未初始化或內容為垃圾的變數的問題
- 修正 3 處將未初始化或垃圾的 bytes 寫入到硬碟檔案中的問題
- 修正 8 處 buffer overflows 以及越界存取
- 修正 9 處 unreachable memory leaks
- 修正 4 處會毀壞程式記憶體結構的操作
- 修正 7 處 file resource leaks
- 修正 25 處的無效 `fclose()/close()` 函數呼叫
- 修正 30 處 `open()` 回傳值的誤用，以及其所造成的問題
- 修正一些 undefined behaviors
- 修正當使用者的 IPv4 address 長度為 15 時，
   會造成全域變數 `ipv4addr` 發生 buffer overflow 的問題
- 修正 format string 的參數型別不合，而造成潛藏的 buffer overflow 的問題
- 修正 `sprintf()` 的輸入與輸出的 buffer 重疊，而造成 undefined behavior 的問題
- 其它雜項安全修正

#### BRH（閱讀紀錄）修正

- `brh_get()`: 修正會多 `memcpy()` 不必要的 3 個 `time_t` 大小的空間的問題。
- `brh_add()`: 修正當 BRH 滿時，閱讀比最舊的已讀文章還舊的文章時會發生越界寫入的問題。
- `brh_add()`: 修正閱讀比最舊的已讀文章還舊的文章時，必定導致時間區間數增加的問題。
- `brh_load()`: 避免 `memcpy()` 0 個或更少的 bytes。
- `brh_get()`: 修正使用 `memcpy()` 在重疊的範圍間移動資料，而導致 BRH 損壞的問題。

### 未分類的修正

- 讓 `bin/account` 可以正常地在每小時整點後的 10-59 分鐘執行
- 修正當 `bin/account` 不在上午 1 點執行時，登入次數不會重設的問題
- `scripts/checkusrDIR.sh`: 修正 `run/NOUSRDIR.log` 永不被清除的問題。
- 修正 `base64encode` 工具會產生錯誤結果的問題
- 修正 `checkemail` 工具傳入參數時，會 `mail` 兩次到相同目的地的問題
- 將餘下的 hardcoded 的程式執行路徑 `/home/bbs` 取代為 macro `BBSHOME`
- 修正 `dns_open()` 的 `host` 參數為 IPv4 address 而連線失敗時，
   會存取未初始化的變數當作 `while` 條件，而導致程式當住甚至 crash 的問題

### 與編譯及架站過程有關的改進

- 如果 macro `M3_USE_*` 被定義，就連 `USE_*` 也一起定義
- 現在 scripts 會隨著 `bmake install` 一起被安裝
- 增加 systemd unit 設定檔
- 修正在 64-bit 作業系統上編譯 dynamic libraries 時所需的 32-bit glibc 的 library 路徑
- 使用 Travis CI 進行 Build Verification Test

### 其它修正與改進

- 修正程式註解中的錯字，並改善部分用字
- 改善部分 variables 與 struct members 的名稱
- 修正與程式碼不符的註解
- 消除型別轉換式中的 K&R-style 函數指標
- 消除 variable-length arrays
- 修正 shell scripts 中不正確的 shebang
- 修正 shell scripts 用 `shellcheck` 檢查時發出的警告
- 消除大部分的 `-Wall` 警告
- 消除由超過 438 處的程式碼引起的大部分的 `-Wwrite-strings` 警告
- 修正不正確的縮排
- 微調程式排版
- 其它較小的 refactor 和 bug 修正

## v2.0.0 的新增功能與變動

### 直接影響使用者操作與介面的修正

- 支援使用 SHA-256 加密密碼
- 支援使用 SHA-256 產生的發站外信時所使用的 BBS 站簽章
- 加大最大密碼長度到 36 位
- 將密碼欄位隱形
- 設定或產生新密碼時，讓使用者可以選擇要使用新的或舊的密碼加密方法
- 現在使用 POP3 認證時的密碼欄位的長度增加到 36 個字元
- 現在使用 POP3 認證時的密碼欄位會隱形
- 使用處理器數量作為系統負載高低的判斷基準
- 將處理器數量與系統負載一同顯示
- 將 yes/no 提示框的非預設選項的字母變為小寫
- 移除登入畫面的系統負載資訊後的破折號

### Command line 工具介面的改進

- 改善 BBS 工具的不正確 command-line 用法的回報
- 修正當 `poststat` 的參數為 1、2、或 100 以外的正數時，會造成越界存取的問題
- 修正 `addsong` 的參數為負數時，會減少點歌次數的問題
- 修正 `counter w` 可以設定最大同時上線人數、最大註冊人數、
   最大每小時上線數、及最大每天上線數為負數的問題
- 接受兩個以上參數的 BBS 工具，現在可以使用 `-?` 語法指定參數以及跳過某些參數

### 其它 UI 修正及改進

- 系統程式資訊: 一律顯示所有模組
- 增加 `vget()` 的 `echo` flags `VGET_STRICT_DOECHO`, `VGET_STEALTH_NOECHO`, `PASSECHO`, `VGET_BREAKABLE`, & `NUMECHO`
- 現在使用 `LCECHO` 時，`vget()` 會將整個輸入字串轉成小寫
- 系統程式資訊: 啟用部分系統模組時，顯示其使用的外部函數庫的版本資訊
- 移除會呼叫 shell 指令的一部分 adminutil 工具

### 與 pfterm 有關的改進

- 初步實作 pfterm 與 pmore 所使用的 `vkey_is_typeahead()` 函數
- 更新 pfterm 的註解與參考資料
- 增加 pfterm 對 ANSI escape sequence `ESC [ <n> d` (移動到第 `<n>` 行) 的支援
- 增加 pfterm 對 ANSI escape sequence `ESC [ 27` (反色屬性關) 的支援
   (`ESC [ 7` 的作用是開啟或關閉反色屬性)

### BBS-Lua 的支援

#### 改進

- 從 PttBBS 引進 bbslua 模組
- 增加編譯設定 macros
- 實作轉接 macros 和函數
- 現在執行 BBS-Lua 前會先檢查使用者有無足夠權限
- 改善特殊按鍵的處理過程與其它 BBS 系統的相容度
- 支援 <kbd>Shift</kbd>-<kbd>Tab</kbd>
- 實作 `bl_getdata()` 的 <kbd>Ctrl</kbd>-<kbd>C</kbd> 偵測
- 增加 `HIDEECHO` (32) flag 給 Maple3 版的 `bl_getdata()`，以讓 `NOECHO` 效果能與其它 `echo` flags 自由組合
- 將 deprecated 的 bitlib library 以 BitOp <http://bitop.luajit.org/> 取代
- 支援 LuaJIT
- 重新實作 BBS-Lua 在 Maple3 上的鍵盤輸入支援
- 更新 BBS-Lua 的版本號為 `0.119-DlPatch-1`
- 其它較小的 refactoring

#### 修正

- 修正使用 <kbd>Ctrl</kbd>-<kbd>C</kbd> 結束程式時會輸出無意義字串的問題。
- `getdata/getstr()`: 修正在 PttBBS 上當 `echo` == 8 時會斷線的問題。
- `getch()`/`kball()`: 修正在 PttBBS 上 <kbd>Esc</kbd>-* 組合鍵會被誤當作 <kbd>Esc</kbd> 鍵的問題。

### BBS-Ruby support

- 從 itszero/bbs-ruby 引進 bbsruby 模組
- 不載入 `"empty.rb"`
- 讓 BBS-Ruby 和 Ruby 1.9 - 2.2 相容
- 將 BBS-Ruby C API 函數名前綴由 `bbs_` 改成 `brb_`
- 重新整理 `row`/`line`/`y` 和 `column`/`x` 的用法
- 重新啟用和修復 interface 版本的檢查.
- 改進 exception 處理以及 debug 訊息
- 讓程式行號和文章行號一致
- 現在在執行前會清除畫面
- 放寬 TOC tags 的語法
- 現在執行 BBS-Ruby 前會先檢查使用者有無足夠權限
- 增加編譯設定 macros
- 讓 BBS-Ruby 能在 PttBBS 上通過編譯
- 如果可能，讓 `getdata()` 在 `NOECHO` echo mode 中把輸入欄位隱形
- 如果可能，讓 BBS-Ruby 可用 <kbd>Ctrl</kbd>-<kbd>C</kbd> 結束
- 其它較小的 refactoring

#### Fixes

- 修正 memory leaks
- 修正存取未初始化變數的問題
- 修正 Ruby interpreter 會因為取到垃圾值而隨機回報 parsing 錯誤的問題
- 繞過不能使用 `rb_compile_string()` + `ruby_exec_node()` 的問題
- 修正 BBS-Ruby 遇到錯誤會直接造成 segmentation fault 的問題
- 修正執行時會導致所有的 signal handlers 被取代的問題
- 部份解決 class `BBS` 和 variables 不會重設的問題
- 修正 `move()` 和 `moverel()` 因為沒有轉換到函數參數而造成結果不正確的問題
- 其它較小修正

### WebSocket proxy 的支援

- `bbsd` 支援透過 unix socket 傳送連線資料；相容於 PttBBS 的 WebSocket proxy 模組
- 從 PttBBS 引進 `wsproxy` 模組
- `wsproxy`: 使用新版本的官方 OpenResty 提供的 method `receiveany()`
   取代須自行 patch 出的 method `receiveatmost()`

### 程式變數型別的改善

- 移除在函數宣告中無效的 top-level cvr-qualifier 以及 `register` 關鍵字
- 將 14 個 static storage 的指標的指向型別加上 `const`
- 將超過 374 個函數參數及回傳值的指標的指向型別加上 `const`
- 將超過 26 個 static storage 的指標的型別加上 `const`
- 將超過 90 個陣列的元素型別加上 `const`
- 將超過 11 個指向字串的變數的型別加上 `const`

### 與編譯及架站過程有關的改進

- 編譯時使用 `-ggdb3 -O0` 以方便 debug
- 避免在非設定檔中定義 `M3_USE_*` 之類的 `macro`
- 增加 `libdao` 函數 `f_mv()` 與 `f_cp()` 的測試
- Refactor Makefiles
- 消除在 makefiles 中進行 `.include` 時不必要的載入
- 支援不使用 dynamic library 載入機制來編譯及使用 BBS 系統的各個程式模組

### 其它改進

- 將沒有專門功能的 macros 從 'include/bbs_script.h' 移到 'include/cppdef.h'
- 改善與 flexible array member 相關的程式碼，並修正 allocate 時微小的 over allocation
- 引入 PttBBS 對 GCC attribute 定義的一些有用 macros
- 定義一些有用的 GCC attribute macros
- 改善 `README.md` 的語法與用詞
- 其它較小的 refactor