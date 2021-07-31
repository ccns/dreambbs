# Deny 停權系統

Deny 停權系統是源自 WindTopBBS 的可將使用者自動停權的系統。

MapleBBS-itoc 無此系統。

## 主要函式

函式 <br> 型別 (DreamBBS v3.0) | 定義檔案 | 說明
---                           | ---     | ---
`add_deny()` <br> `int (ACCT *u, int adm, int cross)` | `maple/acct.c` | BBS 主程式處理停權的主要函式 <br> - DreamBBS v3.0 後改為間接呼叫 `add_deny_exer()`，`exer` 引數為目前使用者帳號名稱。
`add_deny_exer()` <br> `int (ACCT *u, int adm, int cross, const char *exer)` | - `util/stopperm.c` <br> - `lib/acct.c` (DreamBBS v3.0) | `stopperm` 處理停權的主要函式 (WindTopBBS 3.02) <br> 通用的處理停權的主要函式 (DreamBBS v3.0) <br> - 停權期間再被停權，原會重設停權期間，與 `add_deny()` 有別；DreamBBS v3.0 已修正為延長停權期間。

以下將 `add_deny_exer()` 當作最主要的函式以方便說明。

### `add_deny_exer()` 參數與回傳值說明
　      | 型別 (DreamBBS v3.0) | 說明
---     | ---                  | ---
`u`     | `ACCT *`             | 被停權或復權的使用者
`adm`   | `int`                | 停權參數（下述）
`cross` | `int`                | 是否為連坐處罰 (非 `0` 為真，`0` 為假)
`exer`  | `const char *`       | 處罰執行者
回傳值   | `int`               | `adm` 引數的有效值 <br> - 可當作下次呼叫 `add_deny_exer()` 時的 `adm` 引數。

## 相關資料結構欄位

資料結構 | 欄位  | 說明
---     | ---   | ---
`ACCT`  | `deny` | 停權到期時間
`ACCT`  | `userlevel` | 使用者權限 <br> - 停權時會設定禁制權限 `PERM_DENY*`。

## 相關 macros

### 用於 `add_deny_exer()` 的 `adm` 參數的 macros

Macro | 值或定義 | 出處 | 說明
---   | ---     | ---  | ---
`DENY_SEL`       | `(DENY_SEL_TALK\|DENY_SEL_POST\|DENY_SEL_MAIL\|DENY_SEL_AD\|DENY_SEL_SELL)` | WindTopBBS 3.02 | `DENY_SEL_*` 的位元遮罩 <br> （不含 `DENY_SEL_OK`） <br> 指定停權原因 <br> - `DENY_SEL_*` 只能擇一。
`DENY_SEL_TALK`  | `0x00000001` | WindTopBBS 3.02 | 停權理由為「不當言論」
`DENY_SEL_POST`  | `0x00000002` | WindTopBBS 3.02 | 停權理由為「Cross Post」
`DENY_SEL_MAIL`  | `0x00000004` | WindTopBBS 3.02 | 停權理由為「散發連鎖信」
`DENY_SEL_AD`    | `0x00000008` | WindTopBBS 3.02 | 停權理由為「散發廣告信」
`DENY_SEL_SELL`  | `0x00000010` | WindTopBBS 3.02 | 停權理由為「販賣非法事物」
`DENY_SEL_OK`    | `0x00000020` | WindTopBBS 3.02 | 立即復權
`DENY_DAYS`      | `(DENY_DAYS_1\|DENY_DAYS_2\|DENY_DAYS_3\|DENY_DAYS_4\|DENY_DAYS_5)` | WindTopBBS 3.02 | `DENY_DAYS_*` 的位元遮罩 <br> 指定停權期間 <br> - `DENY_DAYS_*` 只能擇一。
`DENY_DAYS_1`    | `0x00010000` | WindTopBBS 3.02 | 停權一星期 (7 日)
`DENY_DAYS_2`    | `0x00020000` | WindTopBBS 3.02 | 停權兩星期 (14 日)
`DENY_DAYS_3`    | `0x00040000` | WindTopBBS 3.02 | 停權參星期 (21 日)
`DENY_DAYS_4`    | `0x00080000` | WindTopBBS 3.02 | 停權一個月 (31 日)
`DENY_DAYS_5`    | `0x00100000` | WindTopBBS 3.02 | 無限期停權（停權 31 日＋設定停權權限 `PERM_DENYSTOP`）
`DENY_MODE_ALL`   | - `(DENY_MODE_TALK\|DENY_MODE_MAIL\|DENY_MODE_POST\|DENY_MODE_NICK\|DENY_MODE_GUEST)` <br> (WindTopBBS 3.02) <br> - `(DENY_MODE_TALK\|DENY_MODE_MAIL\|DENY_MODE_POST\|DENY_MODE_NICK)` <br> (WindTopBBS 3.10 & DreamBBS-2010) | WindTopBBS 3.02 | `DENY_MODE_*` 的位元遮罩 <br> 指定停權權限 <br> - `DENY_MODE_*` 只能擇一。
`DENY_MODE_TALK`  | `0x01000000` | WindTopBBS 3.02 | 設定停權權限 `PERM_DENYTALK\|PERM_DENYCHAT`
`DENY_MODE_MAIL`  | `0x02000000` | WindTopBBS 3.02 | 設定停權權限 `PERM_DENYMAIL`
`DENY_MODE_POST`  | `0x04000000` | WindTopBBS 3.02 | 設定停權權限 `PERM_DENYPOST`
`DENY_MODE_GUEST` | `0x08000000` | WindTopBBS 3.02 | 設定停權權限 `PERM_DENYPOST \| PERM_DENYTALK \| PERM_DENYCHAT \| PERM_DENYMAIL \| PERM_DENYSTOP` <br> 外加無限期禁止以同認證信箱註冊
`DENY_MODE_NICK`  | `0x10000000` | WindTopBBS 3.02 | 設定停權權限 `PERM_DENYNICK`