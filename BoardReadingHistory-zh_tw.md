# BRH 看板閱讀紀錄系統

`BRH` 是 MapleBBS 3 的看板閱讀紀錄系統所使用的資料結構。

在 `BBS` 使用者目錄下的 `.BRH` 檔是匯集了此使用者閱讀的所有看板的 `BRH` 的輸出。

## 名詞說明

為表示區別，本文將一個看板對應之閱讀紀錄稱為「BRH」，將匯集了多篇看板的閱讀紀錄稱為「BRHs」，而將在使用者目錄下的閱讀紀錄檔案稱為「`.BRH` 檔」。

### 相關的操作
- 工作區：目前所閱讀的看板的 BRH 的記憶體存放位置 (`brh_tail` – `brh_tail + BRH_WINDOW`)
    - 本文的術語；「正讀」為 MapleBBS 3 原始碼所使用的對應術語
- 解開：將 BRH 中的 `{final | BRH_SIGN}`；僅出現在未解開的 BRH 中；解開後變為 `{final, final}`
- 壓縮：「解開」的逆操作
- Zap：在看板列表中預設不列出某看板；此看板內的文章閱讀紀錄不被記錄
    - 本文將 zapped 看板在 `.BRH` 檔中所對應的 BRH 稱為「zapped BRH」，而連續的多個 zapped BRH 組成的檔案區段稱為「zapped BRHs 區」

## 相關函式
- `brh_alloc`: 分配空間給新 BRH，會依狀況重新配置記憶體空間
- `brh_put`: 將工作區的 BRH 壓縮並放至非工作區的結尾
- `brh_get`: 從非工作區的 BRHs 找出看板所對應的 BRH，將之放至工作區並解開；未找到時則建立新的 BRH
    - 會先執行 `brh_put`
- `brh_unread`: 判斷指定時間的文章是否為未讀
- `brh_visit`: 已讀或未讀看板所有文章
- `brh_add`: 將指定時間的文章標記為已讀
- `brh_load`: 從使用者的 `.BRH` 檔載入 BRHs 至工作區以及載入 zapped 看板列表；無則建立新的 BRHs
- `brh_save`: 儲存非工作區的 BRHs 以及 zapped 看板列表至 `.BRH` 檔，
    - 會先執行 `brh_put`
    - 在 DreamBBS v2.0 後還會解除相關記憶體的配置

## BRH 的資料結構

MapleBBS 3 並未直接在程式碼中使用資料結構操作 BRH，不過仍然定義了可做實作參考的資料結構 `struct BoardReadingHistory`/struct `BRH`。

注意本文的 `time_t` 為 32 bits 整數，同 `int`。

欄位      | 型別     | 說明
:---     | ---      | ---
`bstamp` | `time_t` | 看板建立時間 <br> - Zapped BRH 僅有此欄位
`bvisit` (非工作區) <br> `bhno` (工作區) | `time_t` | - 上次進入看板的時間 (非工作區) <br> - 看板的 `HDR` 的編號 (工作區)
`bcount` | `int`    | 組成已讀時間區間的 `time_t` 數量
`tags`   | `time_t[]` (大小不定) | 已讀時間區間 <br> - 原無正式命名，本文稱之為 `tags`

### `tags` 的格式

一個已讀時間區間的可能格式：
- `{final, begin}`：代表 `[begin, final]` 的閉區間。
    - 若文章時間 `t` 在任一區間符合 `begin` ≤ `t` ≤ `final`，則為已讀
- `{final | BRH_SIGN}`；僅出現在未解開的 BRH 中；解開後變為 `{final, final}`

## BRHs 的相關全域變數

變數          | 型別      | 說明
 :---        | ---      | ---
`brh_base`   | `int *`  | - 指向用以存放 BRHs 的動態配置記憶體的開頭 <br> - 非工作區 BRHs 的開頭
`brh_tail`   | `int *`  | - 非工作區 BRHs 的結尾 <br> - 指向工作區 BRH 的 `bstamp`
`brh_size`   | `int`    | 用以存放 BRHs 的動態配置記憶體的 byte 大小
`brh_expire` | `time_t` | 讓此前的閱讀紀錄被忽略 <br> - 載入 `.BRH` 檔時才設定

## 相關 macros

Macro        | 值     | 說明
 :---        | ---    | ---
`BRH_EXPIRE` | `180`  | 讓舊過此天數的文章被視為已讀
`BRH_MAX`    | `200`  | `tags` 所含的最大 `time_t` 數量
`BRH_PAGE`   | `2048` | 原作為動態配置的 BRHs 空間每次重新配置所增加的 byte 大小，但未使用
`BRH_MASK`   | `0x7fffffff` | `bstamp` 與 `tags` 的實際時間的 mask
`BRH_SIGN`   | `0x80000000` | - 表示 zapped 看板 (`bstamp`) <br> - 表示頭尾時間相同的已讀時間區間 (`tags` 中)
`BRH_WINDOW` | - `(sizeof(BRH) + sizeof(time_t) * BRH_MAX * 2)` <br> - `(sizeof(BRH) + sizeof(time_t) * BRH_MAX)` (DreamBBS v3.0) | BRH 的最大 byte 大小

## 不同狀態的 BRH/BRHs 的特性

為便於說明，本段定義以下的常數與變數：
- `BBS_BIRTH_TIME` 為 BBS 系統誕生的時間，設為 1978 年 2 月 16 日 UTC-0600 (`0x0F493860`)
- `list` 為具有型別 `int *` 的指標，指向 BRHs 中的未知位置

### 工作區的未解開 BRH
- 時間 tag 一定由大排到小 (`(tags[k] & BRH_MASK) > (tags[k+1] & BRH_MASK)`)
- 不會有相同的 tag
- `bstamp <= bvisit`
- `bstamp` 和 `bvisit` 都大於 `BBS_BIRTH_TIME`

開頭與結尾判斷：
- BRH 的開頭：`list` 符合
`list[0] >= BBS_BIRTH_TIME && list[0] <= list[1] && list[2] < BBS_BIRTH_TIME`

### 非工作區的 BRHs 中的 BRH
- 與工作區的未解開 BRH 基本相同
- `.BRH` 檔中的 BRHs 的結尾可能尚有 zapped BRHs  區 (只有 `bstamp`)；已載入記憶體的 BRHs 則沒有 zapped BRHs 區
- Zapped BRH 的 `(bstamp & BRH_SIGN) != 0` 且 `(bstamp & BRH_MASK) >= BBS_BIRTH_TIME`

開頭與結尾判斷：
- 未 zapped BRH 的開頭：`list` 符合「工作區的未解開 BRH」的 BRH 開頭
- Zapped BRHs 區的開頭：`list` 是符合不在其它 BRH 中，或是在最後一個 BRH 的範圍中但 `list[0] >= list[-1]` 中的第一個指標
- Zapped BRH：`(*list & BRH_MASK) >= BBS_BIRTH_TIME && (*list & BRH_SIGN) != 0`，而且 `list` 是 zapped BRHs 區的開頭，或是 `list[-1]` 是一個 zapped BRH

### 已解開的 BRH
- 開頭為 `brh_tail`
- 時間 tag 一定由大排到小 (`tags[k] >= tags[k+1]`)
- 相同 tag 必在同一區間
- 沒有 `bvisit`，只有 `bhno`
- 除了 `{1, 1}`, `{mode, 0}`, `{time(), 0}` 三種特殊區間外，`tags[k] >= bstamp`
- `{1, 1}`, `{mode, 0}`, `{time(), 0}` 三種特殊區間出現時，必是最後一個區間

開頭與結尾判斷：
- `tags` 的結尾（可能是 `tags` 已解開但仍有 `bvisit` 的 BRH 開頭）：`list` 符合 `k>0, list == &tag[2*k]`, `list[0] >= list[-1] || list[-1] <= 1 || list[-2] < bstamp`，或是 `k>=0, list == &tag[2*k]`, `list[1] > list[0]`

## 運作問題
MapleBBS 3.00a 時，在 `brh_load`, `brh_save`, `brh_put` 等函式誤用了 `memcpy` 來移動記憶體範圍可能重疊的 BRH 資料，產生 undefined behavior 而未加以修正。

記憶體範圍可能重疊時，應使用 `memmove` 而不是 `memcpy` 以避免 undefined behavior。

在較舊的作業系統上，因為 `memcpy` 沒有特別的最佳化，所以不會出現問題。但是在較新的作業系統上 `memcpy` 有特別的最佳化，會讓記憶體範圍重疊的資料複製時出現問題。

MapleBBS 3 的各分支，如未加以修正，在較新的作業系統上運作時，會因為 `memcpy` 最佳化的 undefined behavior 而出現 BRHs 損壞的現象。

DreamBBS v2.1 時已完全修正此問題。