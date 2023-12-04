# Coding Style and Conventions

本頁說明 DreamBBS 的 coding style 與慣例 (不含 indentation style)。

Indentation style 的說明，請見 [[INDENT]]。

## 語法
- 語法要符合 10 年前最新的 ISO C 標準 (C11)，但不應使用不被最新 ISO C++ 標準或草案 (C++23) 所支援的語法
    - 至 2019-09-01 為止的程式碼，在語法上符合 C99 而已經不符合 C90，已不必再繼續維持 C90 語法
- 需要支援 C++ 時，僅考慮過去 10 年內的 ISO C++ 標準 (最舊到 C++14)，不必考慮更久遠的標準
- 新的程式碼不能將最新 ISO C++ 標準中的關鍵字當作變數／函式／型別名
    - 至 2019-09-01 為止沒有轉移使用 C++ 的計畫，不須完全相容標準 C++ 語法
    - 不過，至 2020-02-24 為止已基本相容 C++20 語法，可通過 `g++-8` 或 `clang++-6` 編譯並正常執行
- 可以使用 GNU C extensions；
  但若能以等效 ISO C 語法代替且不影響可讀性，就僅使用 ISO C 語法
    - 目前 (2022-03-19) 僅使用 GCC 和 Clang 編譯器，而它們都支援 GNU C extensions

## 人類語言的使用

### 程式碼與註解
- 原則上一律使用英文
- 不應使用其它語言的拉丁化文字
- 為求用法的一致，遇到用詞上有英式與美式之別時，原則上一律使用美式用法
- 如用詞包含附加符號，原則上不省略附加符號

但以下情況例外：

- 用於顯示的字串
    - 目前此專案尚待支援介面上的國際化
- 從其它程式專案引進的註解
- 舊有註解
- 直接引用的文字註解
- 尚未有正式英文名稱或尚不能以英文精準描述的概念
- 官方名稱原文並非英文的專有名詞 (但有英文名稱時須列出補充)

上述例外情況中，若有必要，可再補充人工翻譯至英文的文字。

如使用的非英文語言使用拉丁字母系統作為文字，則須標明所使用的語言。

### Identifiers
基本上適用程式碼與註解的規則，再加上以下規則：
- 僅能使用 7-bit ASCII 字元

### 說明文件
- 可使用其它語言，惟使用語言非英文時，原則上應標明文件所使用的語言
    - 例外：舊有說明文件可不標明所使用的語言

## Commit 訊息的格式
- Commit 標題應為以下格式之一
    - `<修改種類>(<修改範圍>): <簡要說明>`
    - `<修改種類>(<修改範圍>): <原文字> -> <新文字>: <簡要說明>`：有重要的文字替換
        - `原文字` 或 `新文字`，若含有空格或 `->`
 的話，應被 <code>\`…\`</code> 包圍
    - `<修改種類>: <簡要說明>`：全範圍修改
    - 簡要說明末可加上 `[<相關 issues 的編號>]` 或 `[<關鍵字> <相關 issues 的編號>]`
        - `<關鍵字>` 可為 `fix` 或 `close`
    - 修改種類可為下列之一：
        - `docs`：僅說明文件或註解改變，實際程式碼不變
        - `refactor`：程式碼改變，但程式邏輯不變
        - `chore`：程式邏輯改變，效能不變或降低，但功能不變
        - `perf`：程式邏輯改變，效能改善
        - `fix`：程式邏輯或功能的修正
            - 包含效能從 0 至非 0 的改善
        - `feat`：新功能或功能改變
        - `test`：測試程式的新增或功能改變
            - 相當於測試程式的 `feat`
        - `build`：以 Makefile 的改變或修正為主
        - `ci`：以 GitHub Action 的改變或修正為主
    - 修改範圍可為下列之一或其組合：
        - `<目錄名稱>/`
        - `<檔案名稱或路徑>`
            - 非 header 檔案，若無其它主檔名相同的檔案的話，可省略其副檔名
            - 可包含萬用字元
        - `<概念名稱>`，應使用大寫開頭。代表性的有：
            - `UI`：使用者介面
            - `Xover`
            - `Main Menu`
            - `M3 More`：Maple 3.xx 原生文章瀏覽函式庫（非 `pmore`）
            - `M3 Visio Screen`：Maple 3.xx 原生畫面繪製函式庫（非 `pfterm`）
            - `DBCS`：CJK 雙位元組字元集
            - `LP64`：見於 x86_64 Linux 平臺的 LP64 架構
            - `UB`：C/C++ 語言的未定義行為
        - `<大範圍概念名稱>/<小範圍概念名稱>`
        - `<範圍> <程式物件名>`
        - `<範圍>, <適用條件>`
            - `<適用條件>` 代表性的有：
                - `C`，使用 C 語言模式編譯
                - `C++20`，使用 C++20 語言模式編譯
                - `GNU C`，使用帶有 GNU 語言擴充的 C 語言模式編譯
                - `pfterm`，啟用了 `pfterm`
                - `!NO_SO`，未 `#define` `NO_SO`、`#undef NO_SO`、或 `#define` `NO_SO` 為 `0`
        - `<範圍>; <注意事項>`
            - `<注意事項>` 全為大寫，代表性的有：
                - `NEEDS SHM RELOAD`
                - `NEEDS CONF CHANGE`
    - 簡要說明中，應敘述具體變更或新的行為
        - 敘述舊的行為時，應使用 `fix …`、`instead of …` 等字詞
        - 修改種類為 `fix` 時，禁止將舊的行為直接當作簡要說明
            - 應使用 `fix(…): fix …` 的方式呈現
        - 需敘述多個不相關的變更或行為時，應先嘗試分拆 commit；需敘述多個彼此相關的變更或行為時，應先考慮以其它方式敘述，不可行的話再以分號 `;` 分隔
            - 以網頁介面編輯 wiki 時，可不必分拆 commit，亦不必以其它方式敘述，直接以分號 `;` 分隔即可
- Commit 內文依序可包含以下內容：
    - 原因：陳述一般事實
    - `* <修改方式或新程式行為>`
        - 行尾可加上 `[<單字元代號>]` 作為編號
        - 具體原因可在內部項目列出
        - 影響範圍可用 `* <受影響程式物件>` 的方式在項目內部列出呈現
    - `* <被修改程式物件>`
        - 行尾可加上 `[<代號字串>]` 表示受到對應的修改
        - 具體修改可在內部項目列出
    - `Reference:`
        - `* <參考資料的項目>`（無 indentation）
            - `* > 引用文字`
    - `DEPRECATED: <程式物件名>`
        - (空一行)
        - `<原因及更新方式說明>` (無 indentation)
        - 若要 deprecate 多個不直接相關的物件的話，應先嘗試分拆 commit，不可行的話應以多個 `DEPRECATED:` 分別列出
    - `BREAKING CHANGE: <更改簡述>`
        - (空一行)
        - `原因及更新方式說明` (無 indentation)
        - 若有多個不直接相關的 breaking changes 的話，應先嘗試分拆 commit，不可行的話應以多個 `BREAKING CHANGE:` 分別列出
- Issue 編號格式應為 `#<編號>` 或 `<Repo 擁有者>/<Repo 名>#<編號>` 其一
- Commit hash 格式應為 `<完整 hash>` 或 `<Repo 擁有者>/<Repo 名>@<完整 hash>` 其一
- 其它 repo 的檔案路徑格式應為 `<Repo 擁有者>/<Repo 名>@<完整 hash 或 branch/tag 名>/<檔案路徑>`
- 程式物件名應為下列格式之一：
    - `<`&#8203;`<路徑>`&#8203;`>` (被角括號 `<…>` 包圍)：物件為系統 header 檔
    - <code>\`<路徑>\`</code>：物件為檔案路徑，且出現於內文中
    - <code>\`<struct/union/enum/class> <型別名>\`</code>：物件為 `struct`/`union`/`enum`/`class` 型別，且非以 `typedef` 或 `using` 定義之別名
    - <code><struct/union/enum/class> \`<型別名>\`</code>：物件為其它 `struct`/`union`/`enum`/`class` 型別
        - <code>\`…\`</code> 可依照「其它種物件」的原則決定是否省略
    - `<型別>::<成員>`：物件為型別成員（可與以下格式組合）
    - `<物件名>[]`：物件為陣列
    - `<物件名>()`：物件為函式或 function-like macro
    - <code>\`<物件名>\`</code>：其它種物件
        - <code>\`…\`</code> 在物件名符合以下任一情況時可省略：
            - 並非出現於內文中
            - 含底線 `_` 或貨幣符號 `$`
            - 包含非位於開頭的大寫字母
            - 不在句首而以大寫字母開頭
    - 物件或一群物件為 macro 時，其名稱前方應加上 `macro` 或 `macros`
    - 物件或一群物件為參數時，其名稱前方應加上 `param` 或 `params`
- 直接引用 shell 命令或不只包含物件名的程式碼時，應使用 <code>\`…\`</code> 的方式呈現
    - 以無引數的方式呼叫函式的程式碼應使用 <code>\`func()\`</code> 的方式呈現，而非 `func()`
    - 物件名可包含萬用字元
- 單純並列 2 個事物時，應以 `A & B` 的方式呈現
- 單純並列 ≥ 3 個事物時，應以 `A, B, …, & Z` 的方式呈現
- 列出多個事物作為選項時，應以 `A/B/…/Z` 的方式呈現
- 應統一以 `*` 作為列表項目符號
- 應使用 4-space 的 indent
- 項目文字需換行時，換行後增加 4-space 的 indentation
- 非項目文字需換行時，換行後不增加 indentation
- 陳述程式碼變更時，句首以小寫開頭的原形動詞開頭，不以句號結尾
- 陳述新的程式動作時，句首以大寫開頭的原形動詞開頭，不以句號結尾
- 陳述一般事實時，句首以大寫開頭，並以句號結尾

## 註解的使用
- 註解單獨成行時，應以 `/**/` 的形式說明其後方的程式碼，以 `//` 的形式說明其前方的程式碼
- 註解位於行末時，應使用 `//` 的形式
- 單行中，若行內註解過多，應分拆為多行並改為使用行末註解
- 註解開頭可包含以下額外資訊：
    - `// <TAG>(<註解者暱稱>.<日期>): `
    - `// <TAG>(<註解者暱稱>): `
    - `// <TAG>: `
    - `// <註解者暱稱>.<日期>: `
    - `// <註解者暱稱>: `
    - 禁止僅包含標註日期的註解開頭
    - `<TAG>` 可為下列之一：
        - `XXX`：須特別注意的程式碼；建議改用下列其一
        - `HACK`：原理特別或艱澀，可能需改寫的程式碼
        - `FIXME`：待修正的程式碼
        - `TODO`：待辦事項
    - 日期格式須為下列之一：
        - `YYYY-MM-DD`：帶 ISO 式日期的註解（建議新註解使用）
        - `YYYYMMDD`：舊式帶日期的註解（不建議新註解使用）
        - `YYMMDD`：主要見於公元 2000 年前的程式碼（禁止新註解使用）
- 註解內容不應包含「註解」兩字或其同義詞
- 禁止忽略註解開頭後的內容符合以下任一情況的新註解：
    - 無內容的註解
    - 內容僅包含「註解」兩字或其同義詞的註解
    - 內容與被註解的程式碼的字面意義相同的註解
- 禁止改動註解開頭包含註解者暱稱的註解，應另外增加註解以說明原註解的問題
    - 但以下情況除外：
        - 原註解含有錯字
        - 原註解為亂碼，但可幾乎完全解讀，或是可找到非亂碼的版本
        - 註解內含有過時程式碼
            - 此情況宜改增加註解說明，或是直接刪除原註解並重寫
    - 若改動或刪除此類註解，須記錄於 commit 訊息
- 使用某事物的注意事項的註解，應只撰寫一份並置於其定義處，而非複製多份置於其使用處
    - 若此事物僅與語言本身或標準函式庫相關，則不應註解
        - 若需進行說明，則應記錄於 commit 訊息
- 陳述程式動作時，句首以小寫開頭的原形動詞開頭，不以句號結尾
- 簡述程式物件或其用途時，句首以大寫開頭，不以句號結尾
- 陳述一般事實時，句首以大寫開頭，並以句號結尾

## 命名原則
- 不應以底線 `_` 後接大寫字母開頭，或包含連續兩個或以上的底線 `__`，
  因其被 ISO C 與 ISO C++ 標準保留給編譯器與標準函式庫的內部實作
- Enumeration、object-like macro、等等編譯時期常數的名稱應為 MACRO_CASE
- Function-like macro 的名稱應為 MACRO_CASE
    - 若不以動詞或助動詞開頭，所包含的單字間應使用底線 `_` 隔開
    - 若會造成與系統函式庫的 macro 名稱衝突，且為常用 macro，方可改用 PascalCase，
      但不應包含兩個或以上的大寫字母
    - 若其用法及作用與某普通函式相似，且要使用類似的名稱以便於記憶，方可改用函式的命名方式
- Function-like macro 的參數名稱必須為 snake_case，且應有單個底線 `_` 的前綴
- 資料結構的名稱應為 PascalCase，且不應為單字元，亦不應包含動詞或助動詞
    - 若為既有資料結構，方可維持 UPPERFLATCASE，惟不應包含底線 `_`
    - 不應使用 snake_case 接上 `_t` 後綴，因其被系統函式庫所保留
- 函式的名稱可為 snake_case、camelCase、或 flatcase 前綴後接底線 `_` 再接 camelCase，
  且不應以底線 `_` 開頭
    - 若為既有資料結構，方可維持 PascalCase，惟應含有動詞或以 `Xo` 為前綴詞
    - 若不以動詞或助動詞開頭，所包含的單字間應使用底線 `_` 隔開
    - 若命名時所使用的前綴詞與標準函式庫中某些函式的前綴詞相同，前綴詞後須使用底線 `_`
- 避免在 PascalCase 或 camelCase 中使用連續的大寫字母
- 變數與資料結構成員的名稱必須為 snake_case，且不應以底線 `_` 開頭
- 程式碼檔的去除副檔名後的檔名應為 snake_case
- 命名長度
    - 簡易判斷原則：名稱長度應與其作用域大小成正比，並與其常用程度成反比
    - 應使全域變數的名稱與區域變數的名稱易於區分
    - 禁止在標頭檔中宣告或定義單字元的 enumeration、macro、變數、或函式
    - 禁止程式碼檔的去除副檔名後的檔名為空或為單字元
- 型別命名法
    - `bool` 變數的名稱應含有形容詞
    - 回傳 `bool` 的函式的名稱應含有 `do` 以外的助動詞
    - N 層指標變數的名稱應有 N 個 `p` 前綴，
      但若解參照 M 層後的指標值本身被當作陣列或字串使用則應有 M 個 `p` 前綴，
      而原名已由 `ptr` 開頭時則省略最後一個 `p` 前綴
    - 變數名稱不應包含資料大小、有號與否、或資料對齊單位的資訊
- 若某事物的定義改變，且導致其用法改變時，應更改其名稱，尤其是下列事物：
    - 全域函式的回傳值 → 更改函式名稱
    - 全域函式的參數 → 更改此參數名稱
    - 全域資料結構的成員 → 更改此成員名稱

### 常用的區域變數名稱列表
- 改寫並修訂自 itoc 所撰寫的〈[文件] 一些常用參數的名稱〉

名稱 | 常見型別 <br> ***粗斜體***表示限用此型別 | 意義 | 備註
--- | ---      | --- | ---
`rc` | `int`          | return code 回傳碼 | 應改用 `ret`
`ret` | `int`         | return value 回傳值 | 也可回傳其它型別，只是原始 MapleBBS 3 未如此使用
`res` | (不定)        | result value 結果值 | DreamBBS v2.0 引入
`fp` | ***`FILE *`*** | file pointer 檔案指標 | 不應與 `fd` 混淆
`fd` | ***`int`***    | file descriptor 檔案描述子 | 不應與 `fp` 混淆
`ch` | `int`          | (temporary) character (暫時) 字元 |
`num` | `int`         | (temporary) number (暫時) 數字 | 迴圈數字變數應使用 `i`、`j`、`k`、等等名稱
`pos` | `int`         | position 位置 <br> - 元素索引位置 (Xover, etc.) <br> - 包含 ANSI escapes 後的游標原始縱排座標 (visio) <br> - 游標顯示縱排座標 (edit) | 在 visio 與在 edit 中的定義相反，不應混淆
`col` | `int`         | column (position) 縱排 (位置) <br> - 游標顯示縱排座標 (visio) <br> - 包含 ANSI escapes 後的游標原始縱排座標 (edit) | 在 visio 與在 edit 中的定義相反，不應混淆
`max` | `int`         | maximum 最大值 | 常用於 `x < max` (排除性上界)
`ufo` | `unsigned int` | user favorite option <br> (= user preference 使用者偏好設定) |
`buf` | `char []`     | (temporary) buffer (暫時) 緩衝區 |
`msg` | `char []` <br> (for chatting, displayed message, etc.) | message 訊息 |
`tmp` | - `char []` <br> - (any) | - temporary (buffer) 暫時 (緩衝區) <br> - temporary (variable) 暫時 (變數) | 應改用其它意義更明確的名稱
`cmd` | - `char []` (for chatting, etc.) <br> - `int` (for Xover, etc.) | - (text) command (文字) 命令 <br> - command (code) 命令 (代碼)
`ans` | - `char [3]` (for `vget()`, etc.) <br> - `int` (for `vmsg()`, etc.) | answer (= response 回應) |
`uid` | `char [IDLEN + 1]`/`const char *` | user ID 使用者 ID |
`bid` | `char [IDLEN + 1]`/`const char *` | board ID 看板 ID | 罕用
`fpath` | `char []`/`const char *` | file path 檔案路徑 |
`folder` | `char []`/`const char *` | folder (path) 資料夾 (路徑) |
`str` | ***`const char *`*** | string 字串 | 唯讀；僅用於讀取
`ptr` | `(const) char *` | pointer 指標 |
`dir` | `const char *` | directory 目錄 |
`slp` | ***`screenline *`*** | `screenline` pointer <br> `screenline` 指標 |
`slt` | ***`screenline`/`screenline []`*** | `screenline` temporary <br> 暫時 `screenline` | 罕用
`hdr` | ***`HDR`/`(const) HDR *`*** | (generic) (file) header (通用) (檔案) 標頭 |
`mhdr` | ***`HDR`/`(const) HDR *`*** | mail (file) header 信件 (檔) 標頭 |
`fhdr` | ***`HDR`/`(const) HDR *`*** | file header 檔案標頭 |
`ghdr` | ***`HDR`/`(const) HDR *`*** | gem (file) header 精華區 (檔) 標頭 |
`brd` | ***`BRD`/`(const) BRD *`*** | board (header) 看板 (標頭) |
`mf` | ***`MF`/`(const) MF *`*** | my favorite 我的最愛 (MapleBBS-itoc 版) | DreamBBS 未使用 <br> - pmore 亦使用 `mf` 作為存放執行資訊的變數名
`myfavorite` | ***`HDR`/`(const) HDR *`*** | my favorite 我的最愛 (DreamBBS 版) | DreamBBS 特有 <br> - 罕用，常以 `hdr` 代之
`nbrd` | ***`NBRD`/`(const) NBRD *`*** | new(ly applied) board 新 (申請) 看板 |
`acct` | ***`ACCT`/`(const) ACCT *`*** | (user) account (data) (temporary) (暫時) (使用者) 賬號 (資料) |
`u` | ***`(const) ACCT *`*** | user (account data) (pointer) 使用者 (賬號資料) (指標) | 名稱過短，應改用 `acct`
`cuser` | ***`ACCT`*** | current user (account data) 目前使用者 (帳號資料) | 全域變數
`utmp` | ***`UTMP`/`(const) UTMP *`*** | user (online) temporary (data) 使用者 (線上) 暫時 (資料) |
`up` | ***`(const) UTMP *`*** | user (online temporary data) pointer 使用者 (線上暫時資料) 指標 |
`cutmp` | ***`UTMP *`*** | current user (online) temporary (data) 目前使用者 (線上) 暫時 (資料) | 全域變數
`pal` | ***`PAL`/`(const) pal *`*** | pal 好友 |
`aloha` | ***`ALOHA`/`(const) ALOHA *`*** | aloha 打招呼 <br> (= element of user login notification list 使用者登入通知名單元素) |
`bmw` | ***`BMW`/`(const) BMW *`*** | BBS message [`write`](<https://en.wikipedia.org/wiki/Write_(Unix)>) <br> (= user message 使用者訊息) | 又稱「熱訊」、「水球」、等等
`benz` | ***`BMW`/`(const) BMW *`*** | similar to BMW 類似於 BMW <br> (= user login message 使用者登入訊息) | Maple-itoc 使用 `BENZ`
`xo` | ***`XO`/`XO *`*** | Xover (data) Xover 資料 |
`xt` | ***`XO *`*** | Xover (data) temporary (pointer) 暫時 Xover (資料) (指標) |
`xz` | ***`XZ []`*** | Xover zone (data) Xover 區域 (資料) | 全域變數

## 區域變數的使用
- 應透過限制變數的 scope 而非重用變數來節省記憶體的使用量
    - 限制變數 scope 有利於編譯器分析變數的使用狀況，利於讓編譯器重新利用不使用的變數的記憶體空間；  
      而重用變數不利於編譯器分析變數的使用狀況
- 不應一次將所有變數宣告於函式定義的開頭
    - 應善用 block scope 變數以及 C99 的迴圈 scope 變數
    - 可依可讀性的需要，而在須使用之處時再定義變數

**Good:**
```cpp
for (int i = 0, n = get_n(sth); i < n; ++i) {
    code;
}
```
**Bad:**
```cpp
int i, n;
n = get_n(sth);
for (i = 0; i < n; ++i) {
    code;
}
```

- 宣告變數時，應將其顯式初始化為所需的值
    - 如為 `struct` 型別的變數，且所需的值為 `0`，則必須將其顯式初始化
        - ISO C++ 會將隱式初始化的 `struct` 型別變數進行 `0` 初始化，若其後再使用 `memset()` 將其歸零，則有礙閱讀，且有效能不彰之虞
- 在維持易讀性的前提下，儘可能不要定義暫時變數，尤其是不要定義未使用的變數；既有的未使用變數則應移除
- 非得使用暫時變數時，則儘可能使用 `const`

**Good:**
```cpp
char buf[32];
const char *str = "<anonymous>";
const char *const name = get_name();
if (name) {
    strlcpy(buf, name, sizeof(buf));
    str = buf;
}
process(str);
```
**Bad:**
```cpp
char buf[32];
char *str = "<anonymous>";
char *name = get_name();
size_t len;
if (name)
    len = strlcpy(str = buf, name, sizeof(buf));
process(str);
```

## 全域變數的使用
- 欲僅宣告全域變數並於稍後定義時，應使用 `extern`
- 減少與避免全域變數的使用
    - 不要使用全域變數回傳函式執行結果；盡量使用 `return` 或 output arguments

**Good:**
```cpp
bool func(void)
{
    if (do_task() == TASK_SUCCESS)
        return true;
    return false;
}

void process(void)
{
    if (func())
        do_sth();
    else
        do_sth_else();
}
```
**Bad:**
```cpp
static bool ok = false;

void func(void)
{
    if (do_task() == TASK_SUCCESS)
        ok = true;
    else
        ok = false;
}

void process(void)
{
    func();
    if (ok)
        do_sth();
    else
        do_sth_else();
}
```

-   - 如未能完全避免全域變數的使用，則應將用於同一功能的全域變數以 struct 組織起來

## 可讀性與可移植性
- 程式碼不應造成 compiler 發出容易解決的 warning
    - 對於用語言標準難以解決的 compiler warning，如果使用 GNU C extension 可容易解決，就使用 GNU C extension；  
      如果還是難以解決，就暫不解決，等待新的語法標準或新的 GNU C extensions
- 不應假設函式的回傳值必為某值
- 不應為了節省記憶體的使用，而將函式的指標參數所指向的 struct 暫時用作其他型別資料的 buffer
    - 此能避免改動相關程式後出現 buffer 大小不足的狀況

**Good:**
```cpp
int func(Struct *obj)
{
    FILE *fp;
    {
        char path[LENGTH];
        get_path(path);
        if (!(fp = fopen(path, "r")))
            return 1;
    }
    code_about_obj;
    fclose(fp);
    return 0;
}
```
**Bad:**
```cpp
int func(Struct *obj)
{
    FILE *fp;
    get_path((char *)obj);
    if (!(fp = fopen((char *)obj, "r")))
        return 1;
    code_about_obj;
    fclose(fp);
    return 0;
}
```

- 不要使用避免或依賴編譯器最佳化的 workarounds
- 不要以破壞可讀性的方式手動最佳化運算式
    - 現代許多編譯器已經能夠自動最佳化運算式（`gcc` 及 `clang` 在 `-O0` 下也會最佳化）

**Good:**
```cpp
int y = get_value();
int x = 31 * y;
```
**Bad:**
```cpp
int y = get_value();
int x = (y << 5) - y;
```

- 避免撰寫不必要的程式分支
    - 避免 control hazard

**Good:**
```cpp
x = 0;
```
**Bad:**
```cpp
if (x != 0)
    x = 0;
```

**Good:**
```cpp
free(ptr);
```
**Bad:**
```cpp
if (!ptr)
    free(ptr);
```

- 根據 ISO C 與 ISO C++ 標準，`free(NULL)` 不具有任何作用，無須手動進行空指標檢查。

<p/>

- 避免 boilerplate code，以減少 code size
    - 需要增加新功能時，盡量使用或擴充既有的函式，不要複製原有函式

**Good:**
```cpp
void func(const char *str_task)
{
    do_sth(str_task);
}
```
**Bad:**
```cpp
void func(void)
{
    do_sth("sth");
}

void func2(void)
{
    do_sth("sth_else");
}
```

## 運算式的使用

### 位元與邏輯運算

- 常用的位元與邏輯運算的形式：

運算 | 邏輯運算 | 位元運算 | 位元賦值
--- | --- | --- | ---
BUF 緩衡 | `!!x` <br> `(bool)x` | `x` | (no-op)
NOT 反相 | `!x` | `~x` | `x = ~x` <br> `x ^= ~(0 ? x : 0)`
AND 及 | `x && y` | `x & y` | `x &= y` (bit mask)
NAND 反及 | `!x \|\| !y` | `~(x & y)` | 
OR 或 | `x \|\| y` | `x \| y` | `x \|= y` (bit set)
NOR 反或 | `!x && !y` | `~(x \| y)` |
XOR 互斥或 | `!!x != !!y` <br> `(bool)x != (bool)y` | `x ^ y` | `x ^= y` (bit toggle)
XNOR 反互斥或 | `!!x == !!y` <br> `(bool)x == (bool)y` | `~(x ^ y)` |
IMPLY 蘊含 | **`y \|\| !x`** <br> `!x \|\| y` <br> **`(bool)y >= (bool)x`** <br> `(bool)x <= (bool)y` | `~((x \| y) ^ y)` <br> `~(x & ~(0 ? x : y))` |
NIMPLY 反蘊含 | `x && !y` <br> **`!y && x`** <br> `(bool)x > (bool)y` <br> **`(bool)y < (bool)x`** | `(x \| y) ^ y` <br> `x & ~(0 ? x : y)` | `x &= ~(0 ? x : y)` (bit clear)

- 避免對二元位元運算的運算元直接使用 `~`，以使該運算元為無號整數且寬度至少為 `int` 但窄於另一運算元時（*e.g.*, `~(unsigned int)x & (long long)y`）的結果正確。
    - 若需使用 `~`，應先將該運算元轉型成至少與另一運算元同寬的型別，再對其結果取反相。

**Good:**
```cpp
flag &= ~(0 ? flag : FLAG_X); // 確保 bit mask 的寬度不比 `flag` 窄
```
**Bad:**
```cpp
if (flag & FLAG_X)
    flag ^= FLAG_X;
```

- 避免不必要的 `!` 與 `~` 的使用
    - 例外：`!!` 等效於轉型爲 `bool`，故可使用

**Good:**
```cpp
if (((bool)x == (bool)y) && ((bool)y == (bool)z))
```
**Good:**
```cpp
if ((!!x == !!y) && (!!y == !!z))
```
**Bad:**
```cpp
if ((!x != !!y) && (!y == !z))
```

- 值互相反相的邏輯或位元表達式在臨近之處出現時，其中一個應使用另一個的結果的反相的形式

**Good:**
```cpp
if ((a || b) && c)
    sth();
if (!(a || b) && d)
    sth_else();
```
**Bad:**
```cpp
if ((a || b) && c)
    sth();
if (!a && !b && d)
    sth_else();
```

### 乘法、除法、與取餘運算

- 常用的除法與取餘運算的參考形式：

除法定義 | 取商 | 餘數範圍 | 取餘
--- | --- | --- | ---
floored (toward -∞) | `floor((double)x / y)` <br> `(x - (((x < 0) != (y < 0)) ? ((y < 0) ? -1 : 1) * (abs(y) - 1) : 0)) / y` | (y > 0) → `[0, abs(y))` <br> (y < 0) → `(-abs(y), 0]` | `x % y + (((x % y) && ((x < 0) != (y < 0))) ? y : 0)`
truncated <br> floored toward 0 | `trunc((double)x / y)` <br> `x / y` | (x ≥ 0) → `[0, abs(y))` <br> (x ≤ 0) → `(-abs(y), 0]` | `x % y`
Euclidean | `copysign(1, y) * floor((double)x / abs(y))` <br> `(x - ((x < 0) ? abs(y) - 1 : 0)) / y` | `[0, abs(y))` | `x % y + ((x % y < 0) ? abs(y) : 0)`
ceiling (away from 0) | `copysign(ceil(fabs((double)x / y)), x * copysign(1, y))` <br> `(x + (((x < 0)) ? -1 : 1) * (abs(y) - 1)) / y` | (x ≥ 0) → `(-abs(y), 0]` <br> (x ≤ 0) → `[0, -abs(y))` | `x % y - ((x % y) ? ((x < 0) ? -1 : 1) * abs(y) : 0)`
ceiling (toward +∞) | `ceil((double)x / y)` <br> `(x + (((x < 0) == (y < 0)) ? ((x < 0) ? -1 : 1) * (abs(y) - 1) : 0)) / y` | (y > 0) → `(-abs(y), 0]` <br> (y < 0) → `[0, abs(y))` | `x % y - (((x % y) && ((x < 0) == (y < 0))) ? y : 0)`
rounded (.5 toward -∞) | `round((double)x / y) - (fmod((double)x / y, 1) == 0.5)` <br> `(x + (((x < 0)) ? -1 : 1) * ((abs(y) - ((x < 0) == (y < 0))) >> 1)) / y` | (y > 0) → `(-abs(y) / 2.0, abs(y) / 2.0]` <br> (y < 0) → `[-abs(y) / 2.0, abs(y) / 2.0)` | `x % y - ((abs(x % y) > ((abs(y) - ((x < 0) != (y < 0))) >> 1)) ? ((x < 0) ? -1 : 1) * abs(y) : 0)`
rounded (.5 toward 0) | `copysign(round(fabs((double)x / y)) - (fmod(fabs((double)x / y), 1) == 0.5), x * copysign(1, y))` <br> `(x + (((x < 0)) ? -1 : 1) * ((abs(y) - 1) >> 1)) / y` | (x ≥ 0) → `(-abs(y) / 2.0, abs(y) / 2.0]` <br> (x ≤ 0) → `[-abs(y) / 2.0, abs(y) / 2.0)` | `x % y - ((abs(x % y) > (abs(y) >> 1)) ? ((x < 0) ? -1 : 1) * abs(y) : 0)`
rounded (.5 to even) | `round((double)x / y) - (fmod((double)x / y, 2) == 0.5) + (fmod((double)x / y, 2) == -0.5)` <br> `(x + (((x < 0)) ? -1 : 1) * ((abs(y) - !((x / y) % 2U)) >> 1)) / y` | (q ≡ 0 (mod 2)) → `[-abs(y) / 2.0, abs(y) / 2.0]` <br> else → `(-abs(y) / 2.0, abs(y) / 2.0)` | `x % y - ((abs(x % y) > ((abs(y) - !!((x / y) % 2U)) >> 1)) ? ((x < 0) ? -1 : 1) * abs(y) : 0)`
rounded (.5 away from 0) | `round((double)x / y)` <br> `(x + (((x < 0)) ? -1 : 1) * (abs(y) >> 1)) / y` | (x ≥ 0) → `[-abs(y) / 2.0, abs(y) / 2.0)` <br> (x ≤ 0) → `(-abs(y) / 2.0, abs(y) / 2.0]` | `x % y - ((abs(x % y) > ((abs(y) - 1) >> 1)) ? ((x < 0) ? -1 : 1) * abs(y) : 0)`
rounded (.5 toward +∞) | `round((double)x / y) + (fmod((double)x / y, 1) == -0.5)` <br> `(x + (((x < 0)) ? -1 : 1) * ((abs(y) - ((x < 0) != (y < 0))) >> 1)) / y` | (y > 0) → `[-abs(y) / 2.0, abs(y) / 2.0)` <br> (y < 0) → `(-abs(y) / 2.0, abs(y) / 2.0]` | `x % y - ((abs(x % y) > ((abs(y) - ((x < 0) == (y < 0))) >> 1)) ? ((x < 0) ? -1 : 1) * abs(y) : 0)`

-   - 實際使用時應依已知除數範圍簡化。
    - 特例：若除數 n 為 2 的 m 次方，且 m 為非負整數，則 x 的 integer floored division 與 modulo 應分別用 `x >> m` 與 `x % (unsigned)n`（或 `x & ((1U << m) - 1)`）實作
        - `lhs >> rhs` 的 `lhs` 為負時，依據 C99 及 C++11 標準會產生 implementation-defined 的結果；依據 C++20 標準則會產生 arithmetic right shift 的結果（同 2 的幂次的 integer floored division）

**Good:**
```cpp
int y = get_value();
int q = y >> 5;
unsigned int r = y % 32U; // or `y & ((1U << 5) - 1)`
```
**Bad:**
```cpp
int y = get_value();
int q = (y - ((y < 0) ? 32 - 1 : 0)) / 32;
int r = y % 32 + ((y < 0) ? 32 : 0);
```

### 遞增、遞減、與複合運算式

- 禁止連續使用前綴運算子 `++`、`--`、`+`、`-`、`~`、或 `!`、以及後綴運算子 `++` 或 `--`
    - 例外：`!!` 等效於將運算元轉型爲 `bool`，故可使用，惟此時不應再使用前綴運算子 `!`
    - 連用兩次的前綴運算子 `-` 或 `~` 等效於單個前綴運算子 `+`
    - 在 C++ 中，對某 glvalue `x` 連用 `n` 次的前綴運算子 `++` 或 `--` 等效於 `x += n` 或 `x -= n`；在 C 中僅可使用 `x += n` 或 `x -= n`
    - 在 C 與 C++ 中，內建的後綴運算子 `++` 與 `--` 的結果爲非 (g)lvalue，無法連續使用
    - 在 C++ 中，因存在運算子多載，前綴運算子 `++` 與 `--` 的結果可能爲 glvalue，惟此時仍適用此規則，不應連續使用

**Good:**
```cpp
i += 2;
!i++;
```
**Bad:**
```cpp
++ ++i;
!~~+ +- -i++;
```

### 其它運算式

- 禁止對內建的以下運算子的運算結果使用前綴運算子 `+`：前綴運算子 `+`、`-`、`~`、與 `!`、後綴運算子 `++` 與 `--`、以及二元運算子 `*`、`/`、`%`、`+`、`-`、`>>`、`<<`、`&`、`^`、與 `|`
    - 由於這些運算子的運算結果會被提昇爲至少與 `int` 同寬，因此前綴運算子 `+` 對其無作用
    - 前綴運算子 `++` 與 `--` 以及 (複合) 賦值運算子的運算結果，在 C 中亦會被提昇爲至少與 `int` 同寬，但在 C++ 中則不會，因此仍可直接對其結果使用前綴運算子 `+`

## 迴圈的使用
- 應利用 `continue`、`break`、`return`、或 `goto` 減少 block 深度
- 在函式定義中，無限迴圈內不應包含無限迴圈
- 迴圈應為以下形式之一

### 迴圈的形式
- 無限迴圈
    - `for (;;)`
- 條件迴圈
    - `while (cond())`：不暫存結果
        - 可為普通 `whlie` 迴圈或 `do`-`while` 迴圈
    - `for (Type v; (v = get_val());)`：暫存結果，並進行非零判斷
    - `for (Type v; v = get_val(), cond(v);)`：暫存結果，並進行其它判斷
- 範圍迴圈
    - 遞增型
        - `for (int i = get_start(); i < n; ++i)`：不暫存終點
        - `for (int i = get_start(), n = get_end(); i < n; ++i)`：暫存終點
    - 遞減型
        - `for (int i = get_end(); i-- > 0;)`：不暫存終點
        - `for (int i = get_end(), b = get_begin(); i-- > b;)`：暫存終點
- 指標型 for-each 迴圈
    - 遞增型
        - `for (Type *p = get_start(), *const n = get_end(); p != n; ++p)`
    - 遞減型
        - `for (Type *p = get_end(), *const b = get_start(); p-- != b;)`
- 若需在迴圈之後使用迴圈變數的值，方可將迴圈的初始化語句搬出至迴圈之前

## `goto` 的使用
- 如使用 `goto` 可避免暫時變數的使用或是程式碼的重複，且難以透過使用迴圈或呼叫函式達成，應使用 `goto`
- `goto` 的目標應在 `goto` 語句之後，除非要從無限迴圈跳出並重新進入
- 從 `goto` 語句前往目標的途中須僅允許跳出及跳過 blocks，以及最終跳入一層無區域變數的 block 的開頭。
    - 若難以改寫為上述形式，可嘗試改用迴圈或函式呼叫，或是將 `goto` 置入迴圈

## Macro 的使用
- 不要定義實作過於複雜的 macro 來處理容易解決的 C 語法問題
    - 例如：不要用 macro 生成 `malloc` 回傳指標的轉型（parse 實作過於複雜），而應直接手寫轉型
- 如果定義了較為複雜的 macro，應該使用註解解釋背後邏輯
    - 參考 `include/cppdef.h`
- 不應為了過舊的編譯環境或編譯器而將程式邏輯複雜化
    - 目前 (2022-03-19) 主要考量的編譯環境為 Linux；  
      考量的 Linux 版本最舊為 4.18，glibc 版本最舊為 2.28
    - 目前 (2022-03-19) 所考量的編譯器，Clang 版本最舊為 13，GCC 版本最舊為 11

### Macro 定義的撰寫
- 定義 function-like macro 時，參數出現時，應被圓括號 `()` 及逗號 `,` 緊包圍，
  如：`(_x)`、`(_x,`、`, _x,`、與 `, _x)`
    - 如展開後可能產生未被圓括號包圍的逗號，則此參數須被圓括號 `()` 緊包圍，
      如：`(_x)`
- 如上包圍參數後，應改寫定義為以下形式之一

### Macro 定義的形式
- 類常數定義 –– 只含有 macro 參數、算數型別常數、與無副作用的表達式的類表達式定義；
  若預期任一參數會接受非算數型別或非常數的引數，則應使用下述的非類常數的類表達式定義
    - `expr`，若看似不直接包含運算子或僅直接包含後綴運算子
    - `(expr)`，若否
    - 注意：
        - `expr` 可為其他類常數定義 macro。
        - 包含運算子 `,` 的表達式，在 ISO C 中無法被用作常數，
          且函式呼叫運算子（後綴運算子 `()`）應被視作有副作用（用以呼叫其它類常數定義 macro 時除外）。
          故符合以上任一情況的 macro 定義為非類常數定義。
        - 數字的前綴 `+`/`-` 符號為前綴運算子，故含有這些符號的數字無法使用上述的 `expr` 形式，
          而必須使用 `(expr)` 之形式。
- 非類常數的類表達式定義 –– 預期使用其計算結果；
  若定義中含有非參數的非全域變數，即使其不需參數，也應被定義為 function-like macro
    - `((void)0)`，若表達式為空
    - `expr`，若未被圓括號 `()` 包圍，且看似不直接包含運算子或僅直接包含後綴運算子
    - `(exprs)`，若否，但包含任何 `void` 表達式
    - `((void)expr_first, exprs)`，若 `expr_first` 看似不直接包含運算子或僅直接包含前綴與後綴運算子
    - `((void)0, exprs)`，若否
    - 注意：
        - 若在任一非結尾的表達式中，具有最低優先結合順序的運算子不帶副作用，
          應將此表達式轉型為 `void` 並視其為 `void` 表達式。
          （否則可能會引發編譯時期警告。）
        - 每一表達式可為其它類表達式定義 macro。
        - 請將單獨出現的 expression block（GCC extension）或 C++ 匿名函式視為不直接包含運算子。
        - 若某 macro `F` 的定義為 `expr++`/`expr--`，
          其中的 `++`/`--` 必會被剖析為遞增/遞減後綴運算子，
          即使遇到 `F 1` 的非預期用法也是如此；此非預期用法會由於不符語法而被避免。
          因此在上述的 `expr` 形式中，允許直接包含後綴運算子 `++` 與 `--`。
        - 包含至少一個 `void` 表達式是為了使 `func_a MACROB` 的用法不符語意而被避免，
          且可彰顯其並非類常數定義 macro。
        - 雖然 `(Type) (exprs)` 之形式更為理想，
          但這需要事先得知 `exprs` 的型別，實行上會造成不便，因此不採用。
- 可改寫為一連串表達式的類陳述式定義 –– 預期忽略其計算結果；
  應改寫為表達式，且即使其不需參數，也應被定義為 function-like macro
    - `(void) ((void)0)`，若陳述式為空
    - `(void) (exprs)`，若重寫後的一連串表達式以 `void` 表達式結尾
    - `(void) (exprs, (void)expr_last)`，若 `expr_last` 看似不直接包含運算子或僅直接包含前綴與後綴運算子
    - `(void) (exprs, (void)0)`，若否
    - 注意：
        - 若在任一表達式中，具有最低優先結合順序的運算子不帶副作用，
          應將此表達式轉型為 `void` 並視其為 `void` 表達式。
        - 開頭的 `(void)` 是為了使 `MACROA MACROB` 的用法不符語法而被避免，
          且可彰顯其為某陳述式之替代。
        - 結尾的 `(void)` 可避免此 macro 的呼叫式被用作後綴運算子的運算元。
- 無法改寫為表達式的類陳述式定義；
  即使其不需參數，也應被定義為 function-like macro
    - `statement`，若為單一陳述式，且其前後緊接任何運算子或運算元的話，會造成用法不符語法
        - 不應包含陳述式結尾的 `;`
    - `do { statements } while (0)`，若否
- 其它定義（非類陳述式定義） –– 無須改寫

## Binary compatibility
- 目前 (2022-03-19) 考量的編譯環境的系統為 32-bit 及 64-bit x86 架構
- 在會被讀出／寫入 binary file 的資料結構中，不應使用 `long`, `time_t`, 以及其它會因編譯環境架構而有不同大小的資料型別
    - 參見 `include/struct.h`
    - 目前 (2022-03-19) 已無在相關資料結構中使用這些資料型別
- 在會被讀出／寫入 binary file 或是 shared memory 的資料結構中，不應使用指標型別
    - 目前 (2022-03-19) 已無在相關資料結構中使用指標型別
- 應當使用下列形式的註解以標註會被讀出／寫入硬碟或 shared memory 的資料結構
    - `<STORAGE_TYPE>(<formatting_type>); <dependency_type>`
    - `STORAGE_TYPE` 可為下列之一：
        - `DISKDATA`：會直接或間接地讀出／寫入硬碟的資料；標註上比 `SHMDATA` 優先
        - `SHMDATA`：會直接或間接地讀出／寫入 shared memory 的資料
    - `formatting_type` 可為下列之一：
        - `raw`：Binary data 形式
        - `format`：已格式化之文字形式
    - `dependency_type` 可為下列之一：
        - `dependency(<Type>)`：由於此資料結構被包含於 `Type`，而被間接地讀出／寫入硬碟或 shared memory 的資料結構
        - `runtime`：僅於程式執行時期需要使用，而結束執行後可捨棄的資料
    - 至 2020-02-24 為止，所有符合 `DISKDATA(raw)` 的資料結構都已被標註

## Race condition 的預防
- 應假設執行環境為即時作業系統中的多執行緒環境
- 操作檔案、共用記憶體、等等被多個執行緒與處理程序所共用的物件（下稱「共用物件」）時，應避免 race condition
- 建立或刪除共用物件時，應先直接嘗試進行操作，再透過例外處理檢查是否符合操作條件
- 存取共用物件時，應使用 atomic 操作或使用 lock 機制
    - 若需一次進行多個不彼此獨立的操作，或是無對應的 atomic 操作可用時，應使用 lock 機制
        - 若寫入期間需進行耗時操作，可先持著 read lock 複製一份資料，對資料副本更新，再持著 write lock 更新原資料；
          或改用 read-copy-update 機制
        - 若為檔案，應使用 `lib/file.c` 所提供之 `f_exlock()` 與 `f_unlock()` 函式
    - 否則，應使用 atomic 操作
        - POSIX.1/2008 標準的 `read()` 與 `write()` 為 atomic，故僅需進行單個此類操作的話，不需要 lock 機制
        - ISO C/ISO C++ 的內建遞增、遞減、與複合賦值運算子並非 atomic，不應直接使用，而須搭配 atomic 型別：
            - 包含 `++`、`--`、`&=`、`|=`、`^=`、`>>=`、`<<=`、`+=`、`-=`、`*=`、`/=`、`%=`、等等
        - 操作 atomic 型別的物件時，應使用專用函式，以避免誤用非 atomic 操作
- 開啟某共用物件後，不彼此獨立的操作應使用同一個 handler，不應重新取得 handler
    - 若為檔案，handler 為 file descriptor 或 file pointer，直接使用檔案路徑則視為重新取得 handler

## Header 的使用
- 不同支程式使用的 header 應該分開，以方便控制特定程式的編譯環境

**Good:**

`a.h`:
```cpp
#include "lib.h"
void do_sth(void);
```
`b.h`:
```cpp
#include "lib.h"
```
`lib.h`:
```cpp
void do_lib(void);
```
`a.c`:
```cpp
#include "a.h"

void do_sth(void) { }

int main(int argc, char *argv[]) { }
```
`b.c`:
```cpp
#include "b.h"

static void do_sth(void) { }

int main(int argc, char *argv[]) { }
```
`lib.c`:
```cpp
void do_lib(void) { }
```

**Bad:**

`main.h`:
```cpp
void do_lib(void);
#ifdef A_C
void do_sth(void);
#endif
```
`a.c`:
```cpp
#define A_C
#include "main.h"

void do_sth(void) { }

int main(int argc, char *argv[]) { }
```
`b.c`:
```cpp
#define B_C
#include "main.h"

static void do_sth(void) { }

int main(int argc, char *argv[]) { }
```
`lib.c`: 同前

- 避免在原始碼中自行宣告函式；統一使用 `#include`
    - 自行宣告容易有型別錯誤，而且用 C++ 編譯時沒有統一 `extern "C"` 的使用時容易發生 linker errors
- 決定一段宣告所屬的 header 時，先依循「用途」，再依循「語法類型」
    - 泛用的宣告才可僅依循「語法類型」決定其所屬的 header

## Library 使用
- 應先了解函式各個參數以及回傳值的意義，再使用該函式，以避免誤用而造成邏輯錯誤
- 使用功能相似的 library/system 函式的考量重點
    - 除了以效能或安全為重點的情況下，如果在 BBS 中已經實作了所需要的功能的函式，就使用它
        - 參見 `lib/*.c`
    - 原則上，以在編譯環境中最可能存在的函式為優先
    - 優先度高到低：C standard library 函式 > GCC built-in 函式 > glibc 專有函式 = POSIX 系統函式 > *NIX 系統函式 > 外部 library 函式
    - 在一般情況下，如果使用某兩個函式寫出的程式碼差不多一樣複雜，使用優先度高的函式；  
      否則，使用讓程式碼較簡潔的函式；  
      但如果編譯環境可能缺少該函式，就依序使用其它優先度高的函式作為後備

**Good:**
```c
int diff = strncasecmp(str1, str2, LENGTH);
```
- `strncasecmp()` 在 glibc 2.5 前就已存在，可假設編譯環境有此函式

**Bad:**
```c
int diff = 0;
const char *ptr1 = str1;
const char *ptr2 = str2;
int len = LENGTH;
while (len--) {
    char ch1 = *ptr1;
    char ch2 = *ptr2;
    if (ch1 >= 'A' && ch1 <= 'Z')
        ch1 += 'a' - 'A';
    if (ch2 >= 'A' && ch2 <= 'Z')
        ch2 += 'a' - 'A';
    diff = ch1 - ch2;
    if (diff || !*ptr1 || !*ptr2)
        break;
    ++ptr1;
    ++ptr2;
}
```

- 冗長

**Worse:**
```c
int diff;
char buf1[LENGTH+1], buf2[LENGTH+1];
strncpy(buf1, str1, LENGTH);
buf1[LENGTH] = '\0';
strncpy(buf2, str2, LENGTH);
buf2[LENGTH] = '\0';
for (char *ptr = buf1; *ptr; ptr++)
    *ptr = tolower(*ptr);
for (char *ptr = buf2; *ptr; ptr++)
    *ptr = tolower(*ptr);
diff = strncmp(buf1, buf2, LENGTH);
```

- 又冗長又浪費記憶體，而且不使用 variable length array (C99 或 GNUC++ 之功能) 的話，字串長度會有限制

<p/>

-   - 在以效能或安全為重點的情況下，優先使用效能或安全較好的函式；  
      但如果編譯環境可能缺少該函式，就依序使用其它效能或安全較好的函式作為後備，  
      最後應使用在各個編譯環境中都能夠確定存在的函式作為最終後備
    - 如果選擇了多個函式，而所寫出的程式碼會被重複利用，應將該段程式碼獨立定義成函式
- 不應將外部 libraries 放進 BBS 程式碼中，而應該以 git submodule + symbolic link 的方式引用
    - 原則上，不維護不是由自己維護的程式碼

## Directory layout
- 盡量保持整個專案結構的扁平；限制 Makefile 的層次在 3 層以下
    - 目前 (2022-03-19) 整個 DreamBBS 專案只有 `scripts/wsproxy/` 一個內層目錄，但沒有自己的 Makefile