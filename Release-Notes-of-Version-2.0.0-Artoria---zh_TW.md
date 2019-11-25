# Version 2.0.0 Artoria 發行說明 (草稿)

## 發行代號：Artoria

* 靈感來源：Fate Stay/Night 系列作品的登場角色，職階 Saber。

## 新增功能與變動

### 直接影響使用者操作與介面的修正

- 現在使用 POP3 認證時的密碼欄位的長度增加到 36 個字元
- 現在使用 POP3 認證時的密碼欄位會隱形
- 使用處理器數量作為系統負載高低的判斷基準
- 將處理器數量與系統負載一同顯示

#### 命令列管理工具介面的改進

- 改善 BBS 工具的不正確 command-line 用法的回報
- 修正當 `poststat` 的參數為 1、2、或 100 以外的正數時，會造成越界存取的問題
- 修正 `addsong` 的參數為負數時，會減少點歌次數的問題
- 修正 `counter w` 可以設定最大同時上線人數、最大註冊人數、
   最大每小時上線數、及最大每天上線數為負數的問題
- 接受兩個以上參數的 BBS 工具，現在可以使用 `-?` 語法指定參數以及跳過某些參數

#### WebSocket proxy 的支援

- `bbsd` 支援透過 unix socket 傳送連線資料；相容於 PttBBS 的 WebSocket proxy 模組
- 從 PttBBS 引進 `wsproxy` 模組
- `wsproxy`: 使用新版本的官方 OpenResty 提供的 method `receiveany()`
   取代須自行 patch 出的 method `receiveatmost()`

#### 與 pfterm 有關的改進

- 初步實作 pfterm 與 pmore 所使用的 `vkey_is_typeahead()` 函數
- 更新 pfterm 的註解與參考資料
- 增加 pfterm 對 ANSI escape sequence `ESC [ <n> d` (移動到第 `<n>` 行) 的支援
- 增加 pfterm 對 ANSI escape sequence `ESC [ 27` (反色屬性關) 的支援
   (`ESC [ 7` 的作用是開啟或關閉反色屬性)

#### 其它改進

- 解決：開啟 `CHAT_SECURE` 編譯選項時，
         會造成程式將使用者密碼明碼儲存在全域變數中的問題。
- 解決：因檢查密碼後會抹除密碼明碼，
         造成存儲密碼明碼的全域變數無法用以登入 xchatd 的問題。
- 移除在函數宣告中無效的 top-level cvr-qualifier 以及 `register` 關鍵字
- 將 14 個 static storage 的指標的指向型別加上 `const`
- 將超過 374 個函數參數及回傳值的指標的指向型別加上 `const`
- 將超過 26 個 static storage 的指標的型別加上 `const`
- 將超過 90 個陣列的元素型別加上 `const`
- 將超過 11 個指向字串的變數的型別加上 `const`
- 引入 PttBBS 對 GCC attribute 定義的一些有用 macros
- 定義一些有用的 GCC attribute macros
- 其它較小的 refactor

### 直接影響使用者操作及介面的改變

- 移除使用者名單介面的無用的大 V 功能
- 現在系統踢出使用者時，使用者端顯示出斷線訊息後會立刻斷線
- 現在使用者登出時會先進行登出作業，再顯示登出訊息
- 現在轉貼和轉寄文章時會顯示作業不成功或權限不足的文章數量
- 編輯器的快速鍵 `Ctrl-U` 改成輸入 `ESC` 字元；請改用 `ESC-U` 叫出使用者名單
- 編輯器增加快速鍵 `ESC-1` ~ `ESC-5`，分別會讀入 1 ~ 5 號暫存檔的內容

## 修正

### 直接影響使用者操作及介面的錯誤修正

- 修正 `x_siteinfo()` (系統程式資訊) 中的資料大小單位 `bytes` 誤植為 `KB` 的問題
- 修正文章轉貼失敗也能增加個人文章數的問題
- 修正 `topusr` 產生的使用者排行榜的標題變成 `%s` 的問題
- 修正編輯器的符號輸入工具因為 buffer overflow 而造成界面損壞，甚至 crash 的問題
- 修正進入好友看板被拒絕時，目前進入的看板的顯示名稱會變成該好友看板的名稱的問題
- 修正 `class_yank2()` (只顯示好友板、秘密板) 無法列出所有好友板和秘密板的問題
- 修正熱門看板只列出看板 SYSOP 的問題；改為僅不列出看板 SYSOP
- 修正：`VGET_*` flags 的值與 `BRD_*_BIT` flags 衝突，造成無法搜尋看板的問題
- 修正 header 上的新信件與新留言的訊息 `NEW[MAIL|PASS]MSG` 被截斷的問題
- 將餘下的 hardcoded 的程式執行路徑 `/home/bbs` 取代為 macro `BBSHOME`
- 修正 `dns_open()` 的 `host` 參數為 IPv4 address 而連線失敗時，
   會存取未初始化的變數當作 `while` 條件，而導致程式當住甚至 crash 的問題
- 修正 `innbbsd` 的 `連線人數過多` 的訊息 `msg_no_desc` 被截斷的問題
- 修正 `innbbsd/inntobbs.h` 中的函數 `HISfetch()` 宣告錯誤的問題

#### 密碼安全修正

- 修正造成 `xchatd` 將使用者帳號與密碼明碼當作使用者連線來源的邏輯錯誤
- 現在處理完使用者輸入的密碼後，會立刻抹除密碼明碼
- 發站外信時使用的 BBS 站簽章：在處理完 BBS 站的私鑰後，會立刻抹除私鑰

#### 其它系統安全修正

- 修正當使用者的 IPv4 address 長度為 15 時，
   會造成全域變數 `ipv4addr` 發生 buffer overflow 的問題
- 修正 format string 的參數型別不合，而造成潛藏的 buffer overflow 的問題
- 修正 `sprintf()` 的輸入與輸出的 buffer 重疊，而造成 undefined behavior 的問題
- 其它雜項安全修正

#### 有關 pfterm 的修正

- 修正不使用 pfterm 時，`popupmenu_ans2()` 及 `pmsg2()` 不會讓背景變按的問題
- 修正不使用 pfterm 時，
   如果參數為 `NULL`，`pmsg2()` 不會使用 `vmsg()` 顯示暫停訊息的問題
- 修正：不使用 pfterm 時，
         macros `STANDOUT` 及 `STANDEND` 會展開成多個 statements，
         造成 current 版本的 `vget()` 的 `STEALTH_NOECHO` 模式顯示不正常的問題。
- 修正 pfterm 將 ANSI escape sequence `ESC <ch>` 誤當作 `ESC [ <ch>` 處理的問題
   (如將 `ESC m` 誤當作 `ESC [ m`)

#### 其它修正與改進

- 修正不正確的縮排
- 微調程式排版
- 其它較小的 refactor 和 bug 修正
- 修正熱門看板會列出所有非 SYSOP 的看板以及熱門的看板 SYSOP 的問題；
   應為列出所有熱門看板
- 修正 `class_yank2()` (只顯示好友板、秘密板) 會列出可閱讀的所有看板的問題
- 修正：使用 `class_yank()` (顯示被 zap 掉的板) 或是 `class_yank2()` (只顯示好友板、秘密板) 時，如果沒有相符的看板，就會造成使用者被踢出看板列表的問題。 
- 修正：`class_yank()` 或是 `class_yank2()` 作用中時，如果沒有相符的看板，就會導致使用者無法進入看板列表的問題。
- 修正空的熱門看板列表會造成使用者無法進入分類看板列表的問題
- 編輯器：修正使用快速鍵 `ESC-1`~`ESC-5` 會出現多餘的檔案選擇訊息的問題。
- 解決使用 DES 加密的密碼無法用以登入 xchatd 的問題