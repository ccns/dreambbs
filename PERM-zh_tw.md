# 權限系統

## 賬號權限

### 用於 `USER::userlevel` 的 macros

Macro | 值或定義 | 出處 | 說明
---   | ---     | ---  | ---
`PERM_BASIC` | `0x00000001` | | 有 基本權力
`PERM_CHAT` | `0x00000002` | | 可 進入聊天室
`PERM_PAGE` | `0x00000004` | | 可 找人聊天
`PERM_POST` | `0x00000008` | | 可 發表文章
`PERM_VALID` | `0x00000010` | | 已 身分認證
`PERM_MBOX` | `0x00000020` | | 有 信件無上限 (大信箱特權)
`PERM_CLOAK` | `0x00000040` | | 可用 隱身術
`PERM_XEMPT` | `0x00000080` | | 會 永久保留帳號
`PERM_9` | `0x00000100` | | 有 公告看板發文權限 (未使用)
`PERM_10` | `0x00000200` | | (未使用)
`PERM_11` | `0x00000400` | | (未使用)
`PERM_12` | `0x00000800` | | (未使用)
`PERM_13` | `0x00001000` | | (未使用)
`PERM_14` | `0x00002000` | | (未使用)
`PERM_15` | `0x00004000` | | (未使用)
`PERM_16` | - `0x00008000` <br> - (更名為 `PERM_SP`) | | 有 特殊標記 (未使用)
`PERM_SP` | `0x00008000` | | 有 特殊標記 (未使用)
`PERM_DENYPOST` | `0x00010000` | | 禁止發表文章
`PERM_DENYTALK` | `0x00020000` | | 禁止 talk
`PERM_DENYCHAT` | `0x00040000` | | 禁止 chat
`PERM_DENYMAIL` | `0x00080000` | | 禁止 mail
`PERM_DENYSTOP` | `0x00100000` | | 停權 (權限同 guest)
`PERM_DENYNICK` | `0x00200000` | | 禁止更改暱稱
`PERM_DENYLOGIN` | `0x00400000` | | 禁止 login
`PERM_PURGE` | `0x00800000` | | 將被 清除帳號
`PERM_BM` | `0x01000000` | | 有 板主 的權限
`PERM_SEECLOAK` | `0x02000000` | | 可 看見忍者 (隱身使用者)
`PERM_KTV` | `0x04000000` | | 有 點歌系統總管 的權限
`PERM_GEM` | `0x08000000` | | 有 精華區總管 的權限
`PERM_ACCOUNTS` | `0x10000000` | | 有 帳號總管 的權限
`PERM_CHATROOM` | `0x20000000` | | 有 聊天室總管 的權限
`PERM_BOARD` | `0x40000000` | | 有 看板總管 的權限
`PERM_SYSOP` | `0x80000000` | | 有 站長 的權限
`PERM_SYSOPX` | `PERM_SYSOP \| PERM_BOARD \| PERM_CHATROOM \| PERM_ACCOUNTS` | | 各種站務權限
`PERM_MANAGE` | `0xFE0000E0` (`PERM_SYSOPX \| PERM_GEM \| PERM_KTV \| PERM_SEECLOAK \| PERM_XEMPT \| PERM_CLOAK \| PERM_MBOX`) | | 各種站務特權
`PERM_CRIMINAL` | `0x007F0000` (`PERM_DENYLOGIN \| PERM_DENYNICK \| PERM_DENYSTOP \| PERM_DENYMAIL \| PERM_DENYCHAT \| PERM_DENYTALK \| PERM_DENYPOST`) | | 各種禁制權限 (清除帳號除外)
`PERM_DEFAULT` | `PERM_BASIC` | | 新賬號的預設權限
`PERM_ADMIN` | `PERM_BOARD \| PERM_ACCOUNTS \| PERM_SYSOP \| PERM_CHATROOM \| PERM_KTV` | | 各種站務權限
`PERM_ALLBOARD` | `PERM_SYSOP \| PERM_BOARD` | | 可 查看所有看板
`PERM_LOGINCLOAK` | `PERM_SYSOP \| PERM_ACCOUNTS \| PERM_BOARD \| PERM_CHATROOM` | | 會 免除登入時的自動解除隱身
`PERM_SEEULEVELS` | `PERM_SYSOP` | | 可 查看使用者權限 (未使用)
`PERM_SEEBLEVELS` | `PERM_SYSOP \| PERM_BM` | | 可 查看看板權限 (未使用)
`PERM_BBSLUA` | `PERM_BASIC` | | 可 執行 BBS-Lua
`PERM_BBSRUBY` | `PERM_BASIC` | | 可 執行 BBS-Ruby
`PERM_NOTIMEOUT` | `PERM_SYSOP` | | 會 免除閒置逾時時的強制登出 (未使用)
`PERM_READMAIL` | `PERM_BASIC` | | 可 進入信箱
`PERM_INTERNET` | `PERM_VALID` | | 可 寄站外信
`PERM_FORWARD` | `PERM_INTERNET` | | 可 轉寄文章
`HAS_PERM(x)` | - `x ? cuser.userlevel & x : 1` <br> - (= `HAVE_PERM(x)`) | | - 目前使用者是否擁有任一或所有指定賬號權限 <br> - (= `HAS_PERM(x)`)
`HAVE_PERM(x)` | `cuser.userlevel & x` | | 目前使用者是否擁有任一指定賬號權限
`NUMPERMS` | `32` | | 權限棋標的數量

## DENY 停權系統

DENY 停權系統是源自 WindTopBBS 的可將使用者自動停權的系統。

MapleBBS-itoc 無此系統。

### 主要函式

函式 <br> 型別 (DreamBBS v3.0) | 定義檔案 | 說明
---                           | ---     | ---
`add_deny()` <br> `int (ACCT *u, int adm, int cross)` | `maple/acct.c` | BBS 主程式處理停權的主要函式 <br> - DreamBBS v3.0 後改為間接呼叫 `add_deny_exer()`，`exer` 引數為目前使用者帳號名稱。
`add_deny_exer()` <br> `int (ACCT *u, int adm, int cross, const char *exer)` | - `util/stopperm.c` <br> - `lib/acct.c` (DreamBBS v3.0) | `stopperm` 處理停權的主要函式 (WindTopBBS 3.02) <br> 通用的處理停權的主要函式 (DreamBBS v3.0) <br> - 停權期間再被停權，原會重設停權期間，與 `add_deny()` 有別；DreamBBS v3.0 已修正為延長停權期間。

以下將 `add_deny_exer()` 當作最主要的函式以方便說明。

#### `add_deny_exer()` 參數與回傳值說明
　      | 型別 (DreamBBS v3.0) | 說明
---     | ---                  | ---
`u`     | `ACCT *`             | 被停權或復權的使用者
`adm`   | `int`                | 停權參數（下述）
`cross` | `int`                | 是否為連坐處罰 (非 `0` 為真，`0` 為假)
`exer`  | `const char *`       | 處罰執行者
回傳值   | `int`               | `adm` 引數的有效值 <br> - 可當作下次呼叫 `add_deny_exer()` 時的 `adm` 引數。

#### 相關資料結構欄位

資料結構 | 欄位  | 說明
---     | ---   | ---
`ACCT`  | `deny` | 停權到期時間
`ACCT`  | `userlevel` | 使用者權限 <br> - 停權時會設定禁制權限 `PERM_DENY*`。

### 相關 macros

#### 用於 `add_deny_exer()` 的 `adm` 參數的 macros

Macro | 值或定義 | 出處 | 說明
---   | ---     | ---  | ---
`DENY_SEL`       | - `(DENY_SEL_TALK\|DENY_SEL_POST\|DENY_SEL_MAIL\|DENY_SEL_AD\|DENY_SEL_SELL)` <br> - `0x0000000F` (DreamBBS v3.0) | WindTopBBS 3.02 | `DENY_SEL_*` 的位元遮罩 <br> （不含 `DENY_SEL_OK`） <br> 指定停權原因 <br> - `DENY_SEL_*` 只能擇一。
`DENY_SEL_NONE`  | `0x00000000` | DreamBBS v3.0   | 不進行停權
`DENY_SEL_TALK`  | `0x00000001` | WindTopBBS 3.02 | 停權理由為「不當言論」
`DENY_SEL_POST`  | `0x00000002` | WindTopBBS 3.02 | 停權理由為「Cross Post」
`DENY_SEL_MAIL`  | - `0x00000004` <br> - `0x00000003` (DreamBBS v3.0) | WindTopBBS 3.02 | 停權理由為「散發連鎖信」
`DENY_SEL_AD`    | - `0x00000008` <br> - `0x00000004` (DreamBBS v3.0) | WindTopBBS 3.02 | 停權理由為「散發廣告信」
`DENY_SEL_SELL`  | - `0x00000010` <br> - `0x00000005` (DreamBBS v3.0) | WindTopBBS 3.02 | 停權理由為「販賣非法事物」
`DENY_SEL_OK`    | - `0x00000020` <br> - `(DENY_SEL_NONE \| DENY_DAYS_RESET)` (DreamBBS v3.0) | WindTopBBS 3.02 | 立即復權 <br> - DreamBBS v3.0 起會一併恢復基本使用者權限
`DENY_DAYS`      | `(DENY_DAYS_1\|DENY_DAYS_2\|DENY_DAYS_3\|DENY_DAYS_4\|DENY_DAYS_5)` <br> - 改為 function-like macro（見下）(DreamBBS v3.0) | WindTopBBS 3.02 | `DENY_DAYS_*` 的位元遮罩 <br> 指定停權期間 <br> - `DENY_DAYS_*` 只能擇一。
`DENY_DAYS(_x)`  | `((_x) << 16U)` | DreamBBS v3.0 | 指定停權日數 <br> - 最大值為 65535 日 (約 179.43 年)。
`DENY_DAYS_ADM(_adm)` | `((_adm) >> 16U)` | DreamBBS v3.0 | 取得停權日數用的 macro
`DENY_DAYS_PERM` | `0x00004000` | DreamBBS v3.0 | 無限期停權（設定禁制權限 `PERM_DENYSTOP`）
`DENY_DAYS_RESET`| `0x00008000` | DreamBBS v3.0 | 重設停權期間
`DENY_DAYS_1`    | - `0x00010000` <br> - `DENY_DAYS(7)` (DreamBBS v3.0) | WindTopBBS 3.02 | 停權一星期 (7 日)
`DENY_DAYS_2`    | - `0x00020000` <br> - `DENY_DAYS(14)` (DreamBBS v3.0) | WindTopBBS 3.02 | 停權兩星期 (14 日)
`DENY_DAYS_3`    | - `0x00040000` <br> - `DENY_DAYS(21)` (DreamBBS v3.0) | WindTopBBS 3.02 | 停權參星期 (21 日)
`DENY_DAYS_4`    | - `0x00080000` <br> - `DENY_DAYS(31)` (DreamBBS v3.0) | WindTopBBS 3.02 | 停權一個月 (31 日)
`DENY_DAYS_5`    | - `0x00100000` <br> - `(DENY_DAYS(31) \| DENY_DAYS_PERM)` (DreamBBS v3.0) | WindTopBBS 3.02 | 無限期停權（停權 31 日＋設定禁制權限 `PERM_DENYSTOP`）
`DENY_MODE`       | `0x00000FF0` | DreamBBS v3.0 | `DENY_MODE_*` 的位元遮罩 <br> 指定停權權限 <br> - `DENY_MODE_*` 只能擇一或全選 (WindTopBBS 3.02)；<br> - 可任意組合 (DreamBBS v3.0)。
`DENY_MODE_ALL`   | - `(DENY_MODE_TALK\|DENY_MODE_MAIL\|DENY_MODE_POST\|DENY_MODE_NICK\|DENY_MODE_GUEST)` <br> (WindTopBBS 3.02) <br> - `(DENY_MODE_TALK\|DENY_MODE_MAIL\|DENY_MODE_POST\|DENY_MODE_NICK)` <br> (WindTopBBS 3.10 & DreamBBS-2010) <br> - `(DENY_MODE_POST \| DENY_MODE_TALK_PERM \| DENY_MODE_CHAT \| DENY_MODE_MAIL \| DENY_MODE_NICK)` (DreamBBS v3.0) | WindTopBBS 3.02 | 設定所有禁制權限 `PERM_DENY*`
`DENY_MODE_POST`  | `0x04000000` <br> - `0x00000010` (DreamBBS v3.0) | WindTopBBS 3.02 | 設定禁制權限 `PERM_DENYPOST`
`DENY_MODE_TALK_PERM`  | `0x00000020` | DreamBBS v3.0 | 設定禁制權限 `PERM_DENYTALK`
`DENY_MODE_CHAT`  | `0x00000040` | DreamBBS v3.0 | 設定禁制權限 `PERM_DENYCHAT`
`DENY_MODE_MAIL`  | `0x02000000` <br> - `0x00000080` (DreamBBS v3.0) | WindTopBBS 3.02 | 設定禁制權限 `PERM_DENYMAIL`
`DENY_MODE_NICK`  | `0x10000000` <br> - `0x00000100` (DreamBBS v3.0) | WindTopBBS 3.02 | 設定禁制權限 `PERM_DENYNICK`
`DENY_MODE_LEVEL` | `0x00000200` | DreamBBS v3.0 | 除禁制權限外，只保留 guest 權限
`DENY_MODE_VMAIL` | `0x00000400` | DreamBBS v3.0 | 無限期禁止以同認證信箱註冊
`DENY_MODE_UNUSED7` | `0x00000800` | DreamBBS v3.0 | （未使用）
`DENY_MODE_TALK`  | - `0x01000000` <br> - `(DENY_MODE_TALK_PERM \| DENY_DAYS_CHAT)` (DreamBBS v3.0) | WindTopBBS 3.02 | 設定禁制權限 `PERM_DENYTALK\|PERM_DENYCHAT`
`DENY_MODE_GUEST` | - `0x08000000` <br> - `(DENY_MODE_ALL \| DENY_DAYS_PERM)` (DreamBBS v3.0) | WindTopBBS 3.02 | 設定所有禁制權限 <br> 除禁制權限外，只保留 guest 權限 <br> 無限期禁止以同認證信箱註冊 <br> - 無限期停權（同 `DENY_DAYS_5`）；<br> - 無限期停權（同 `DENY_DAYS_PERM`） (DreamBBS v3.0)。

#### `adm` 參數值的位元分配比較

圖例：`欄位名(所佔位元數)`

(`--` 表示未使用的位元；已省略前綴 `DENY_` 以求簡潔)

##### WindTopBBS 3.02
`32| --(3) | MODE_*(5) | --(3) | DAYS_*(5) | --(10) | SEL_*(6) |0`
##### DreamBBS v3.0
`32| DAYS_*(16) | DAYS_RESET(1) | DAYS_PERM(1) | --(2) | MODE_*(8) | SEL_*(4) |0`

#### 新的 `adm` 參數值定義的特點
- 舊的 macro 使用方法仍然有效。
- 將邏輯上僅能擇一的 macros，從可以相組合的位元旗標形式，改成僅能擇一的列舉形式，以充分運用值域空間。
- 停權期間改以日數指定，範圍與精細度皆增加了。
    - 可指定範圍為 0 日至 65535 日。
- 原同時控制多個權限的 macros 已被分解，以利於自由組合。
- 為 `0` 時不再有未定義行為。
    - `add_deny()` 的 `adm` 引數為 `0` 時，會詢問操作者要進行的操作。詢問完仍為 `0` 則會取消操作（既不停權也不復權）。
    - `add_deny_exer()` 的 `adm` 引數為 `0` 時，則會有以下行為：
        - DreamBBS v3.0 前：引發未定義行為。
        - DreamBBS v3.0 起：既不停權也不復權（`0` 會被解析為 `DENY_SEL_NONE | DENY_DAYS(0) | !DENY_DAYS_PERM | !DENY_DAYS_RESET`）。

## 精華區權限

### 在精華區中用於 `XO::key` 的 macros

Macro | 值或定義 | 出處 | 說明
---   | ---     | ---  | ---
`GEM_QUIT` | `-2` | | 遞迴離開精華區 (未使用)
`GEM_VISIT` | `-1` | | 瀏覽精華區
`GEM_USER` | `0` | | 有 一般使用者 的權限
`GEM_RECYCLE` | `1` | | 在 回收筒中
`GEM_LMANAGER` | `2` | | 有 小板主 的權限
`GEM_MANAGER` | `3` | | 有 板主 的權限
`GEM_SYSOP` | `4` | | 有 站長 的權限
