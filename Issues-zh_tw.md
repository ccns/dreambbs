# DreamBBS issue list (暫)

<style>
a.anchor + .task-list-item-checkbox {
  margin: .31em 0.3em .2em 0 !important
}
@keyframes inlineSpoilerExpand {
  0% {
    color: transparent;
    cursor: pointer;
    user-select: none;
  }
  100% {
    color: unset;
    background: #a0a0a040;
    cursor: auto;
    user-select: auto;
  }
}
@keyframes inlineSpoilerCollapse {
  0%, 100% {
    background: #a0a0a0;
  }
  20% {
    background: #b0b0b0;
  }
}
.inline-spoiler > span {
  color: transparent;
  background: #a0a0a0;
  border-radius: 3px;
  animation: inlineSpoilerExpand .016s forwards paused;
  transition: opacity .5s, background .5s;
}
.inline-spoiler:hover > span, .inline-spoiler:hover .inline-spoiler > span {
  background: #c0c0c0;
}
.inline-spoiler:active > span {
  animation-play-state: running;
  transition: all .2s;
}
.inline-spoiler:not(:hover) > span {
  animation: inlineSpoilerCollapse .5s forwards;
}
.inline-spoiler-group .inline-spoiler:not(:hover) > span {
  animation: inlineSpoilerExpand .016s forwards paused;
}
.inline-spoiler-group:not(:hover) .inline-spoiler:not(:hover) > span {
  animation: inlineSpoilerCollapse .5s forwards;
}
</style>

## 近期有機會解決的 TODO

### <input class="task-list-item-checkbox" disabled type="checkbox" checked> `acct_save()` 有 6 個，待合併
:::spoiler 問題敘述 (solved by [4a1da1733](https://github.com/ccns/dreambbs/commit/4a1da1733758e2b1ba3e3ecc8f1fce7479025c9f))
> 有六個 `acct_save()`。
      ![](https://cdn.discordapp.com/attachments/489808831337988107/614089600587137041/unknown.png) [name=IID] [time=2019_08_22]
:::

### <input class="task-list-item-checkbox" disabled type="checkbox"> 將 `system()` 全面取代成 `execl()`/`execv()`
:::spoiler {state=open} 需要的 piping 機制的程式碼參考
> 之後應該把所有的 `system()` 取代成 `execl()`/`execv()`，然後用 C 操作 IO redirection 和 piping。 \
https://stackoverflow.com/questions/8082932/connecting-n-commands-with-pipes-in-a-shell [name=IID] [time=2019_08_22]
:::

### <input class="task-list-item-checkbox" disabled type="checkbox"> 無用的 `NULL` 指標判定
:::spoiler {state=open} 問題敘述
```c
while (p != NULL)
{
    free(p);
}
```
> ![](https://media.discordapp.net/attachments/370600485612290060/625319470583382017/unknown.png) [name=IID] [time=2019_09_22 (Sun) 21:16 UTC+8]
:::

### <input class="task-list-item-checkbox" disabled type="checkbox"> 新增 pfterm 的 "256 color" 與 "true color" 的支援
:::spoiler {state=open} 構想方案
> *[前略...]* \
> 該開始來做 pfterm 的 indexed color ("256 color") 與 direct color ("true color") 的支援了。
- [ ] 先在 `fterm_exec()` 裡實作讀取這些 ANSI escape sequence 的邏輯，並用 down sampling 的方式把色彩降成 8 色
- [ ] 改寫 `ftattr` 和 `ftcolor` 資料結構，讓其在對應的編譯選項開啟後，可以記錄足夠的色彩資訊
- [ ] 改寫使用了上述資料結構的相關程式碼
- [ ] 在 `fterm_chattr()` 中實作將這些資料結構輸出成 ANSI escape sequence 的邏輯。

> 不過在這之前：
- [x] 將 pfterm 程式碼與 PttBBS 上的同步，只保留必要的邏輯修改與格式修正。

> [name=IID] [time=2019_09_30]
:::

###  <input class="task-list-item-checkbox" disabled type="checkbox"> 看板列表中的 `[y]載入` 字樣需要調整
:::spoiler {state=open} 問題敘述
> 話說為何按鍵說明欄中有 `[y]載入`？ [name=IID] [time=2019_10_18]
:::

### <input class="task-list-item-checkbox" disabled type="checkbox"> 新增短於 6 行的簽名檔的支援
:::spoiler {state=open} 構想
> 覺得可以考慮實作：允許三個簽名檔的其中任一個可以短於六行。 \
預設是每六行斷開，但用了單獨一行的特殊分隔線，像是 `--` 的話，會提早從分隔線所在的行斷開。 \
範例：
```
C18   # 第一個簽名檔
C11
C99
C89
C87
C876  # 結束
C++17  # 第二個簽名檔
C++14
C++11
C++03
C++98  # 結束
--
B  # 第三個簽名檔  # 結束
```
> [name=IID] [time=2019_10_18]
:::

### <input class="task-list-item-checkbox" disabled type="checkbox"> Big5 轉 UTF-8 的實作
:::spoiler {state=open} 實作參考
> 看起來轉 utf8 時，會設定 pfterm 成自動去除一色雙字。 \
<https://github.com/ptt/pttbbs/blob/master/include/convert.h> [name=IID] [time=2019_11_29]

> <https://github.com/ptt/pttbbs/blob/master/mbbsd/convert.c> \
之後得加轉 utf8 的功能。 [name=IID] [time=2019_11_29]

> 想到一種自動判斷終端機編碼的方法。 \
連線時送出 UTF-8 編碼的測試字串，然後送 `\x1b[6n` 以取得終端機游標位置。如果游標位置與測試字串的預期長度相符，就假設終端機使用的是 UTF-8 編碼。 [name=IID] [time=2021_04_08 (Thu) 20:19 UTC+8]

> 例如送 `"\n正在偵測 BBS 瀏覽器編碼……\x1b[6n\n"`，然後立即讀 BBS 瀏覽器的回應。
如果回應不符合，就假設是 Big5 編碼。 [name=IID] [time=2021_04_08 (Thu) 20:23 UTC+8]

> 然後在登入畫面上顯示 `Using UTF-8/Big5; press , for Big5/UTF-8` 的字樣，在 `vget()` 裏偵測使用者按下 `,` 的動作來即時切換編碼 。
但是限制只能按 3 次，防止被機器人濫用。 [name=IID] [time=2021_04_08 (Thu) 20:48 UTC+8]

> 在 Footer 上顯示 `[UTF-8] Big5 (,) Switch encoding`/`UTF-8 [Big5] (,) Switch encoding` 的字樣。 [name=IID] [time=2021_04_08 (Thu) 20:54 UTC+8]
:::

### <input class="task-list-item-checkbox" disabled type="checkbox"> 解決 `util/utmp-dump.c` 與 `maple/talk.c` 的內容重複
:::spoiler {state=open} 方案構想
> `#50: sync or merge content util/utmp-dump.c and maple/talk.c` 的部分： [name=IID] [time=2020_02_24]
1. [ ] 建立可以放不同程式共用的函數庫的目錄 `common/` (放在 `lib/` 下也可)
2. [ ] 從 `visio` 分離出可獨立運作的 `visio_core` 移到 `common/` 下。
    - 要增加 `visio.h`
3. [ ] 改進 `xover`，將 `xover` 分離出可獨立運作的 `xover_core` 移到 `common/` 下。
    - 要增加 `xover.h`
4. [ ] 將 `taik.c` 與 `utmp-dump.c` 中的相似函數合併成 `ulist` 移到 `common/` 下。
:::

### <input class="task-list-item-checkbox" disabled type="checkbox"> 預計 v3.1 推出的 Xover 系統底層的改進
:::spoiler {state=open} 計畫說明
> 計畫未來對 xover 系統的改動 (預計在 v3.1 推出)： [name=IID] [time=2020_12_21]
- [ ] 廢除 struct `XZ`，相關的 data members 併入 struct `XO`
    - [ ] 可以解決使用 `XZ_OTHER` 時，使用者模式的設定問題
- [ ] 將所有 `XO` 都放至 `xo_root` 以及 `XO::nxt` 所串連的 singly linked list 上
    - 解決有些 `XO` 的游標會儲存，有些不會的不一致
    - [ ] 需要在 struct `XO` 中加入標明其所屬的 zone 的 data member，或是直接比較 `XO::cb`，以免取到別的列表去，雖然不會造成程式不安全運作
        - 例如點歌看板既可用來點歌，也可單純瀏覽，其對應的列表不同，需要區分
:::

### <input class="task-list-item-checkbox" disabled type="checkbox"> 使用新的 "xover tab" 架構取代原本的 xover zone 設計
:::spoiler {state=open} 方案構想
- [ ] 增加 `XO::up` 來描述 `XO` 之間的階層關係，並且由 xover tab 記錄最末層的 `XO`，取代 xover stack 的設計
- [ ] 將 `XO_ZONE + idx` 重新解釋為切換至此 tab 從最底往上數第 `idx` 層 (0-indexed) 的 `XO`
- [ ] 要進入新 `XO` 列表時，先將新的 `XO` 的 `up` 設為目前 tab 上的 `XO`，然後將新的 `XO` 放至目前的 tab 上，再回傳 `XZ_ZONE + XZ_INIT + key` (語法待議) 讓 xover 系統處理即可，避免函式 stack 變高
- [ ] 離開列表時，如要回收 `XO` 物件，則可以回傳 `XZ_ZONE + XZ_FINI + key` (語法待議) 取代 `XO_QUIT`，讓 xover 系統進行 `XO` 的回收
    - [ ] 不想讓 `XO` 留在 `xo_root` 的 linked list 上時，可以執行此指令，不用刻意避免呼叫 `xo_get`
    - [ ] 可以處理目前啟用動態函式庫的 hotswap 時，防止還在使用的函式庫被 swap 掉的上鎖與解鎖機制
- [ ] 主選單的 `XO` 放在第一個 tab，快速選單切換到的 `XO` 則放在最前面的空 tab；沒空位時則提示 tab 已滿
- [ ] 回到上層選單時，先看 `XO::up`，再看前面一個 tab，再沒有就離站
- [ ] 再增加 tab 的切換選單，則可解決現行 xover stack 開到極限時，只能先退出才能再存取其它功能的問題；此外不切換 tab 的話，回上層的具體行為會與現在完全相同

> [name=IID] [time=2020_12_21]
:::

### <input class="task-list-item-checkbox" disabled type="checkbox"> 對 `xo_root` 結構進行改進
:::spoiler {state=open} 構想
> 如果不考慮自動回收機制的話，可以用 hash table 取代 `xo_root`，增加存取效率。 \
不過考慮自動回收機制的話，則可以將 `xo_root` 分兩堆：使用中的與未使用的，先找使用中的再找未使用的。 \
未使用的可以繼續用 singly linked list，並限制長度，回收 list 末端超出限制的 `XO`。 [name=IID] [time=2020_12_21]
:::

### 對 Xover 系統擴充的構想
:::spoiler {state=open} 構想
- [ ] 支援 xterm-style SGR mouse-tracking mode
- [x] 從 `xover` 獨立出函式 `xover_exec` 與 `xover_key`
- [ ] 實作 `XO_MISS` 來進行未知按鍵的 fallback 處理
    - [ ] 支援用 `XO_MISS + key` 強制呼叫 fallback 的按鍵功能
- [ ] 增加指標 `XO::disp` 來存放 `XO` 的顯示參數，並將目前 `XO` 的顯示參數移至其中，以免 `XO` 變得太複雜
- [ ] 廢除 `XO_TALL`，增加 `XO::disp->rows` 與 `XO::disp->cols` 指定一次要顯示多少橫排與直排，達到 amaki patch 的 `XO_XTALL` 功能
- [ ] 增加 `XO::disp->y`、`XO::disp->x` 指定選單的起始座標，以及 `XO::disp->height` 與 `XO::disp->width` 指定一個元素的長寬，以達到 amaki patch 的 `XO_OFFSET` 功能
- [x] 增加 `XO_ITEM` 指令來重繪某項
     - [ ] 目前是叫 `XO_CUR`，避免與某些分支既有的 `XO_ITEM` 衝突，但又與 Formosa BBS 的僅重繪游標的 `CX_CUR` 名稱衝突，計畫改成 `XO_ELEM`
- [ ] 新增 `XO::knee` 用來放置要在 `XR_PART_KNEE` 執行的繪圖函式

> [name=IID] [time=2020_12_21]

- [x] Xover 清單系統應該改成用加在 callback key 上的 bit flag 區分有 `pos` 參數與無 `pos` 參數的 callback 函式。\
而發現清單為空時就顯示空的訊息，並且只能執行無 `pos` 參數的 callback 函式。\
這樣就不用再在 `*_body()` 函式中手動確認清單是否為空，然後還要用 `vget()`/`vans()` 另外處理按鍵。 ==[name=IID] [time=2021_02_22 (Mon) 19:05]==
    - [x] _[前略……]_ 不過反過來說，可以設計成用 `cb_key | XO_POS` (`XO_POS = XO_REL`) 指定 callback 函式有 `pos` 參數。 ==[name=IID] [time=2021_02_22 (Mon) 19:34]==
    - 註：`XO_POS` 實作時已改名為 `XO_POSF`。
:::

### <input class="task-list-item-checkbox" disabled type="checkbox"> `more()` 的回傳值的處理的問題
:::spoiler {state=open} 問題敘述
> 發現 DreamBBS 有幾個判斷 `more()` 的回傳是否為 `-2` 的地方，但是 `more()` 的回傳值不可能是 `-2`。\
MapleBBS 3.xx 的 `KEY_UP` 值是 `-1`，而 `KEY_LEFT` 值是 `-2`。不過 DreamBBS v1.0 時被我改回正數。\
DreamBBS 從 DreamBBS-2010 的 `more.c` 就是 itoc 版本的，當不被處理的按鍵是特殊按鍵時 (當時是 `< 0`)，會改回傳 `'q'`。\
所以我拿了 WindTopBBS 3.02 rev.20040420 的 `more.c` 比較，發現原本在 `KEY_LEFT` 及 `'q'` 時，`more()` 會回傳 `-2`。\
> \
> 原本的邏輯是如果拿到 `-2` 就要在外面重繪畫面，但是 itoc 版改成離開呼叫 `more()` 的函式前一律重繪畫面。
> ![](https://media.discordapp.net/attachments/370600485612290060/798938919886520330/unknown.png) [name=IID] [time=2021_01_13 23:38 (Wed) UTC+8]
:::

### <input class="task-list-item-checkbox" disabled type="checkbox"> 在我的最愛按下 `c` 的問題
:::spoiler {state=open} 問題敘述
> 在我的最愛按下 `c` 會變更 necker 的顯示，但是沒作用。 [name=IID] [time=2021_04_03]

> 要去看板清單切換，再到我的最愛才會出現變化。 [name=IID] [time=2021_04_03]
:::

### <input class="task-list-item-checkbox" disabled type="checkbox" checked> Xover 清單翻頁時的游標清除問題
:::spoiler 問題敘述 (solved by [e330cfcf9](https://github.com/ccns/dreambbs/commit/e330cfcf99fe18fbfafa3ddc164e864963242518))
> 游標進行跳躍時，如果向下翻頁，清除此前游標時會誤清到清單項目以上的行。
> ![](https://cdn.discordapp.com/attachments/370600485612290060/829661080128585728/unknown.png) [name=IID] [time=2021_04_08 (Thu) 18:16 UTC+8]

> `i` 是 `【` Big5 碼的後半位元。 [name=IID] [time=2021_04_08 (Thu) 18:27 UTC+8]

> 然後因為開啟了光棒選單系統，清除游標時會把項目變暗，結果讓最上面的標題列也變暗。 [name=IID] [time=2021_04_08 (Thu) 18:28 UTC+8]

> 大概知道問題了。 \
`xo_thread()` 內部在找到項目時會自己設定游標，然後回傳 `XO_MOVE + XO_REL` (使游標相對移動 0 個選項) 代表有找到，並讓 `xover()` 移游標，並且要換頁的時候會再加上 `XR_BODY` 讓 `xover()` 重繪列表。 \
因為 `xover()` 會先移動游標再重繪畫面，但是 `xo_thread()` 已經設定過游標位置了，所以移動游標時就沒偵測到游標已換頁，結果就清到 header 及 necker 上，然後又因為只重繪 body (列表內容)，所以就沒畫回去。 [name=IID] [time=2021_04_11 (Sun) 17:49 UTC+8]

> 找到正確的問題所在了。 \
前面的猜測是錯誤的，因為 `xover_cursor()` 在游標目的地為目前游標位置時，會略過重繪游標  (所以 `XO_MOVE + XO_REL + 0` 無實際效果)，所以問題不在此。 \
問題在於 `xover_key()` 中處理呼叫 `xo_thread()` 後清除游標的地方，沒有考慮到翻頁的問題，所以會無條件清除游標，造成會清到列表外面的問題。 [name=IID] [time=2021_04_12 (Mon) 02:38 UTC+8]

> 被這個 commit 改壞的。 \
原本 `xo_thread()` 是未找到文章時回傳 `XO_NONE`，翻頁時回傳 `XO_BODY`，而未翻頁時回傳 `-1`。 \
這個 commit 將 `xo_thread()` 改成未找到文章時，有在 footer 上畫東西時回傳 `XO_FOOT`，否則回傳 `XO_NONE`；翻頁時回傳 `XR_BODY + XO_MOVE + XO_REL`，而未翻頁時回傳 `XO_MOVE + XO_REL`。 \
`xover()` 原本只在 `-1` 時清除游標，但是這個 commit 改成只要回傳的 Xover 命令包含移動命令就清除游標，所以才造成了這個問題。 \
https://github.com/ccns/dreambbs/commit/4940e01306d24fb244154383e9bfaee20c3898f1 [name=IID] [time=2021_04_12 (Mon) 03:20 UTC+8]
:::

### <input class="task-list-item-checkbox" disabled type="checkbox"> 尚須實作調整畫面大小時的自動重繪的畫面

:::spoiler {state=open} 詳細清單
- [ ] 進站畫面
- [ ] 編輯器
- [ ] Maple3 more
- [ ] pmore

> [name=IID] [time=2021_04_11 (Sun) 05:15 UTC+8]
:::

### <input class="task-list-item-checkbox" disabled type="checkbox"> 整理位元清除的運算式

:::spoiler {state=open} 問題敘述
> 不要一直用 XOR 來清除位元旗標。
如果確定資料寬度不超過 `int` 的話，最好用 `&= ~BFLAG`。
![](https://cdn.discordapp.com/attachments/370600485612290060/831035421633937409/unknown.png) [name=IID] [time=2021_04_12 (Mon) 13:18 UTC+8]

> 因為 C 與 C++ 的算術運算子只接受寬度至少為 `int` 的運算元，如果運算元比 `int` 窄就會被整型提升為 `int`/`unsigned int`。 [name=IID] [time=2021_04_12 (Mon) 13:33 UTC+8]

> 要安全一點就先把 `BFLAG` 轉成 `unsigned long long`。 \
`uintmax_t` [name=IID] [time=2021_04_12 (Mon) 13:40 UTC+8]

> 是寫成 `flags &= ~((flags & 0) | BFLAG)`。 [name=IID] [time=2021_04_12 (Mon) 15:17 UTC+8]
:::

### <input class="task-list-item-checkbox" disabled type="checkbox"> 修正標題列的中央標題過長的處理

:::spoiler {state=open} 問題敘述
> ![](https://cdn.discordapp.com/attachments/370600485612290060/835281694029512724/Screenshot_20210424-063026_Chrome.png) [name=IID] [time=2021_04_24 (Sat) 06:31 UTC+8]
:::

### <input class="task-list-item-checkbox" disabled type="checkbox"> 在使用者介面上顯示過長的文字時，加上刪節號，並正確處理 DBCS 文字的截斷

:::spoiler {state=open} 實作構想
- [ ] 新增 `STR_ELLIPSIS` 表示刪節號字串
- [ ] 實作 `strlen_ellipsis()` 以傳回可顯示的文字位元組長度，不含刪節號
- [ ] 實作 `strlen_ellipsis_dbcs()` 以處理 DBCS 字元的截斷
- [ ] 在使用者介面上需要截斷文字的地方使用這兩個函式
[name=IID] [time=2021_04_26 (Mon) 17:30 UTC+8]
:::

### <input class="task-list-item-checkbox" disabled type="checkbox"> 實作在連續刪除的文章標題上隨機生成的圖案

:::spoiler {state=open} 功能敘述
> Ptt 的「本文已被吃掉」的隨機刪除訊息是怎麼實作的？
https://pttpedia.fandom.com/zh/wiki/%E5%8D%A1%E5%8D%A1%E7%8D%B8 [name=IID] [time=2021_04_26 17:04 (Mon) UTC+8]
> https://www.ptt.cc/bbs/PttNewhand/M.1224678027.A.552.html [name=IID] [time=2021_04_26 17:09 (Mon) UTC+8]
:::

### <input class="task-list-item-checkbox" disabled type="checkbox"> 修正排行榜在主選單上的名稱不符合內容的問題

:::spoiler {state=open} 問題敘述
> 這個不是上站次數排行榜。
> ![](https://cdn.discordapp.com/attachments/370600485612290060/836235021685227520/Screenshot_20210426-213856_Chrome.png) [name=IID] [time=2021_04_26 21:39 UTC+8]
:::

### <input class="task-list-item-checkbox" disabled type="checkbox"> 支援時間精度最大 48-bit 且允許一秒多篇文章的唯一文章編號與閱讀紀錄

:::spoiler {state=open} 參考資料與構想
> 之後想引入 PttBBS 的 AIDS 系統。

> PttBBS 文章編號系統 AIDS 的說明: <https://github.com/ptt/pttbbs/blob/master/docs/aids.txt>\
PttBBS 的文章檔名格式是：

- <文章類型>`.`<開始編輯時間的十進位制數字>`.A`(`.`<同秒唯一編號的 3 位十六進位制數字>)

> 文章類型：`M` 爲普通文章、`G` 爲文摘文章、`X` (檔案系統上)/`L` (資料結構上) 爲已鎖定文章、`.` 爲已刪除文章。

> AIDu 是二進制整數形式，格式是：
- (共 64 bits) <16 bits: 空白> <4 bits: 文章類型><32 bits: 開始編輯時間値><12 bits: 同秒唯一編號値>
允許的文章類型的値與對應類型：`0` (`M`)、`1` (`G`)。\
AIDc 是字串形式，是經過 base64url 編碼的 AIDu。

> MapleBBS 3 的文章檔名格式是：
- <檔案類型><發表時間的 7 位 radix32 編碼數字 (35-bit)>\
檔案類型：`A` 爲文章檔、`F` 爲目錄檔。

> 之後實作時，我會以 64-bit `time_t` 爲基礎，更改 MapleBBS 3 文章檔名格式爲：
- <檔案類型><時間的 13 位 radix32 編碼數字 (65-bit)>.<同秒唯一編號的 3 位十六進位制數字>
> 並且更改 AIDu 格式爲：
- (共 64 bits) <4 bits: 文章類型><48 bits: 文章時間値><12 bits: 同秒唯一編號値>

> 其中文章類型在 MapleBBS 3 上會固定爲 `0`。\
不過這樣的 AIDu 格式只能使用到公元 8,92 1,564 年，到時要再調整格式。

> 然後還要考慮閱讀紀錄的實作。
>
> PttBBS BRC 的說明: <https://github.com/ptt/pttbbs/blob/master/docs/brc.txt>\
BRC 可視為 MapleBBS 3 的 BRH 的前身。\
BRC 是採用一篇文章一個紀錄的方式記錄閱讀紀錄，並且分開記錄文章與修改時間。\
但是 BRH 是採用閉區間的方式記錄閱讀紀錄，沒有辦法單獨記錄文章的修改時間，所以修文與推文的閱讀紀錄部份，只能恢復一篇文章一個紀錄的紀錄方式。
>
> PttBBS BRC 並沒有考慮文章的同秒唯一編號値。所以只要讀了一篇文章，其他同秒文章也會變成已讀。

> 之後實作 BRH v3/4 時，我會使用 AIDu 格式：
- (共 64 bits) <4 bits: 額外的 flags><48 bits: 發文時間値><12 bits: 同秒唯一編號値>

> 並且讓同秒唯一編號値隨發文順序嚴格遞增，以處理同秒發文的狀況。\
另外，也要確保 AIDu 的 <48 bits: 發文時間値><12 bits: 同秒唯一編號値> 的部份爲唯一値。

> PttBBS 中，使用者發文的機制：\
發表新文章 (`do_post_article()`) 時，會先由 `stampfile()` -> `fhdr_stamp()` 決定檔案名稱（會不斷嘗試隨機產生同秒唯一編號以保證檔名唯一），然後才由 `vedit()` 編輯文章。

> MapleBBS 3 中，使用者發文的機制：\
發表新文章 (`do_post()`) 時，文章會暫時由 `vedit()` -> `ve_filer()` 寫到 `usr/<user_suffix>/<user>/note`，然後由 `hdr_stamp()` 決定實際的檔案名稱並使用 hard link 的方式建立文章檔（會不斷逐秒增加發表時間以保證檔名唯一），隨後會 `unlink()` 使用者目錄下的原檔。\
在同一個使用者同時登入並同時發表文章時，可能會發生問題，需要把暫時檔案也引進同秒唯一編號値。

> [name=IID] [time=2021_04_28 17:19 (Wed) UTC+8]
:::

### <input class="task-list-item-checkbox" disabled type="checkbox"> 使用者名單的按鍵與 Xover 列表預設按鍵衝突

:::spoiler {state=open} 問題敘述
> @r2 你的 `p` 撞到呼叫器了。\
![](https://cdn.discordapp.com/attachments/370600485612290060/837657458948046848/Screenshot_20210430-195105_Chrome.png)[name=IID] [time=2021_04_30 19:51 (Fri) UTC+8]

> https://github.com/ccns/dreambbs/commit/550deb4b30ab209c920799611976d22f55c3a052#diff-857e64831907733ab494766591e0c1271cfd66886bb19d5452da70cebb6ca921\
原來，本來只是想說要用 jk 所以順便的 [name=r2] [time=2021_04_30 19:54 (Fri) UTC+8]

> 我覺得 `p` 與 `n` 不要用來移動。 [name=IID] [time=2021_04_30 19:52 (Fri) UTC+8]

> 可以啊 [name=r2] [time=2021_04_30 19:54 (Fri) UTC+8]

> 不然就要把 Xover 的 KeyFuncList 按鍵變成優先於 Xover 預設按鍵。\
> 但這樣怕被亂設定。 [name=IID] [time=2021_04_30 19:52 (Fri) UTC+8]

> `n` 與 `p`，我記得是從前 PirateBBS 不支援方向按鍵而用的。\
> 但是太不直覺。 [name=IID] [time=2021_04_30 19:52 (Fri) UTC+8]

> previous next 可以這樣理解吧 [name=r2] [time=2021_04_30 19:58 (Fri) UTC+8]

> 上上下下時很不方便。 [name=IID] [time=2021_04_30 19:58 (Fri) UTC+8]

> 又不會拿來打音樂遊戲 (X\
> 不過既然有衝突，加上沒多少人用\
> 就丟掉吧 [name=r2] [time=2021_04_30 19:58 (Fri) UTC+8]

> `j` 塞了。\
![](https://cdn.discordapp.com/attachments/370600485612290060/837700330842095636/Screenshot_20210430-224119_Chrome.png) [name=IID] [time=2021_04_30 22:42 (Fri) UTC+8]
:::

### <input class="task-list-item-checkbox" disabled type="checkbox" checked> 無法自動踢除閒置訪客

:::spoiler 問題敘述 (未再遭遇)
> 爲何 guest 能一直佔著？\
![](https://cdn.discordapp.com/attachments/370600485612290060/837702589688053760/20210430_225029.png) [name=IID] [time=2021_04_30 22:51 (Fri) UTC+8]

> https://github.com/ccns/dreambbs/commit/d164a323758e4db1b85543fb5a7e5a6b775140d4
是不是這個沒弄好? [name=r2] [time=2021_04_30 22:53 (Fri) UTC+8]

> [中略……]

> `bbsd` 會加預設 userlevel。\
> 只是 guest 的 `cuser.userlevel` 在登入時的瞬間會成爲 `0`。\
> 但是隨後就不再 `0` 了。\
> 用是不是 `0` 判斷身份是不對的。 [name=IID] [time=2021_04_30 23:00 (Fri) UTC+8]

> 再仔細看了 `bbsd`，發現不是這個問題。用 `gdb` 驗證發現是可以觸發閒置踢 guest 的，所以應該另有原因。\
> 程式邏輯上是閒置達到 56 分鐘時警告；閒置達到 60 分鐘時踢人。\
> 不過因爲每次鍵盤輸入逾時時，逾時時間會增加 1 分鐘，所以如果登入後直接閒置的話，理論上會在閒置達到 66 分鐘時直接被踢。 [name=IID] [time=2021_09_15 05:44 (Wed) UTC+8]
:::

### <input class="task-list-item-checkbox" disabled type="checkbox" checked> 在編輯器中的原始文字模式中，雙位元組字元後的 ANSI 控制碼會被誤判爲雙位元組字元

:::spoiler 問題敘述與構想 (solved by [b6f6e6e744](https://github.com/ccns/dreambbs/commit/b6f6e6e74487c6e01e646feff55778b97bc9895a ) & [a2bffc7150](https://github.com/ccns/dreambbs/commit/a2bffc7150ae6d8705b4c02ed3051a459996c49a ))
因爲編輯器程式用以取得 DBCS 狀態所呼叫的 `str_nstate_ansi()`，其中用以忽略 ANSI 控制碼而呼叫的 `str_nmove_ansi()`，被設計爲內部游標初始位置位於 ANSI 控制碼的非 `ESC` 字元上時，其結果並沒有特別定義。

正確做法是在非 ANSI 預覽模式下改呼叫 `str_nstate()` 以取得 DBCS 狀態。
> [name=IID] [time=2021_05_06 (Thu) 02:55 UTC+8]
:::

### <input class="task-list-item-checkbox" disabled type="checkbox" checked> `str_nmove_ansi()` 有會導致越界存取的情況

:::spoiler 問題敘述 (solved by [1a0838a9c4](https://github.com/ccns/dreambbs/commit/1a0838a9c46effb0851d797cae02895a8c441473))
內部游標往後移動時，在最後處跳過 ANSI 控制碼時，讀取字元的時候沒有確認是否已到達字串結尾。
> [name=IID] [time=2021_05_06 (Thu) 17:48 UTC+8]
:::

### <input class="task-list-item-checkbox" disabled type="checkbox"> 個人上站紀錄的在 pmore 顯示時的不正確的換行

:::spoiler {state=open} 問題敘述
因爲換行符號被填到了橫座標爲畫面的半形字元寬度 - 1 處，而 pfterm 在此時會進行自動換行，導致實際顯示時多出一個空行。
> [name=IID] [time=2021_05_06 (Thu) 18:15 UTC+8]
:::

### <input class="task-list-item-checkbox" disabled type="checkbox"> 分開新文章與新修文／推文的顯示

:::spoiler {state=open} 方案構想
> 爲何不用不同顏色或圖案區分新文章與推文或修文？ [name=IID] [time=2021_05_11 (Tue) 14:02 UTC+8]

> 要怎麼設計？
```
☆ —— 無新文章、修文、或推文
★ (紅) —— 新文章
★ (藍) —— 無新文章，有新修文或推文
—— 文章、修文、及推文均已讀
+ (暗白) —— 文章未讀
+ (亮白或靑藍) —— 原文章已讀，新修文或推文未讀
```
> 或是色彩對換，只是因爲新文章比較多而修文及推文少，怕不習慣。 [name=IID] [time=2021_05_11 (Tue) 14:43 UTC+8]

> PttBBS 是用 `~` 表示文章有修文或推文未讀。[name=IID] [time=2021_07_08 (Tue) 02:53 UTC+8]
> `mbbsd/bbs.c`: `readdoent()`[name=IID] [time=2021_07_08 (Thu) 02:54 UTC+8]
> 不過符號種類太多會對使用者造成困擾。
> 用顏色區分比較容易看得懂。[name=IID] [time=2021_07_08 (Thu) 02:56 UTC+8]

:::

### <input class="task-list-item-checkbox" disabled type="checkbox" checked> Esc- 瀏覽器端雙位元字自動重複按鍵的過濾不正確

:::spoiler 問題敘述 (solved by [0db749149b](https://github.com/ccns/dreambbs/commit/0db749149bdc9321d6f741ff2887901985e83fd0 ) & [95fc710dd2](https://github.com/ccns/dreambbs/commit/95fc710dd2a2b174e5556c8c17110a028b82744f))
> 如果使用者快速按下 `Esc` 、`Right`，而瀏覽軟體因爲全形字偵測而送出 `Esc`、`Right-Right` 時，伺服器端應該要判斷爲單個 `Esc-Right`。
> 但在目前的夢大上會判斷爲 `Esc-Right`、`Right`，而自動重複送出的 `Right` 沒被濾掉。
[name=IID] [time=2021_06_18 (Fri) 03:29 UTC+8]
:::

### <input class="task-list-item-checkbox" disabled type="checkbox"> UTMP Ownership

:::spoiler {state=open} 方案構想
> 總之構想了這樣的系統。
![BBS UTMP Ownership 構想](https://cdn.discordapp.com/attachments/370600485612290060/863811252421787648/BBS_UTMP_Ownership.png) [name=IID] [time=2021_07_11 (Sun) 23:57 UTC+8]

:::info
#### BBS UTMP Ownership 構想

重複登入時，只允許 owner 爲空的連線存取交易功能。\
如果擁有者登出，就將 ownership 傳給下一個連線。\
取得擁有權的步驟：(0→3)

若擁有者的 owner 不爲空，就暫時禁止取得擁有權。

<span style="float: right;">_2021-07-11 Iweidieng Iep_</span>&#8288;
:::

### <input class="task-list-item-checkbox" disabled type="checkbox"> 主選單的 <kbd>Ctrl</kbd>- 快速鍵所對應的功能增加各畫面通用的 <kbd>Esc</kbd>- 的按法

:::spoiler {state=open} 方案構想
<kbd>Esc</kbd>- 按鍵 | 既有按鍵 | 對應函式 | 功能
:---: | :---: | --- | ---
<kbd>Esc</kbd>‍‍-‍<kbd>Z</kbd> | <kbd>Ctrl</kbd>‍-‍<kbd>Z</kbd> | `every_Z()` | 叫出快速切換列選單
<kbd>Esc</kbd>-<kbd>U</kbd> | <kbd>Ctrl</kbd>-<kbd>U</kbd> | `every_U()` | 叫出使用者名單
<kbd>Esc</kbd>-<kbd>S</kbd> | <kbd>Ctrl</kbd>-<kbd>S</kbd> | `every_S()` | 快速切換看板
<kbd>Esc</kbd>-<kbd>B</kbd> | <kbd>Ctrl</kbd>-<kbd>B</kbd> | `every_B()` | 快速鎖定畫面

> [name=IID] [time=2021_09_29 (Wed) 20:10 UTC+8]
:::

## 文件上的 TODO

### <input class="task-list-item-checkbox" disabled type="checkbox"> 撰寫系統管理細節
:::spoiler {state=open} 問題敘述
> 要不要把夢大的系統管理細節寫在 GitHub wiki 頁面上？ \
例如怎麼設定轉信，以及 `sample/` 底下各腳本的使用時機與方式。 \
如果使用 PttBBS 的設定方式也可以的話，可以在相關條目直接用外部連結的方式連到 PttBBS 的 GitHub wiki 頁面，再輔以在 DreamBBS 上設定時要另外注意的事項。 \
\
專案的說明文件應該與專案放在一起，不要分開。 \
應該也把現在夢大的按鍵說明檔案移到程式專案中維護，避免按鍵說明檔案與實際按鍵愈偏愈遠的情況繼續發生。 \
\
舉個負面例子：[TJAPlayer3](<https://github.com/twopointzero/TJAPlayer3>)。它的原始說明檔案是維護在原開發者的個人網站上的，但後來原開發者不維護專案了，之後適逢個人網站更新，於是就把專案頁面連同說明檔案一齊刪除了。 [name=IID] [time=2020_12_06]
:::

### 需要完成的其它紀錄文件

:::spoiler {state=open} 詳細計畫
> TODO: [name=IID] [time=2021_01_17]
- [ ] 新增 `wsproxy` 與 PttChrome 安裝步驟至 GitHub wiki
- [ ] 將 DreamBBS Development 播放清單中，未完成的影片說明悉數完成
- [x] 更新 DreamBBS 的 `VERSION`
    - [x] 保留 WindTop BBS 格式，並加上可能遺漏的版本的著作權宣告
    - [x] 現在版本的著作權宣告，改用更正式的英文名字，而非僅有暱稱或代號
- [ ] 寫 GitHub 頁面說明 DreamBBS git 版本以來的程式修改部分的著作權授權
- [ ] 更新 `README.md`，加上夢之大地與 DreamBBS 的相關文化歷史資訊
:::

## 長期 TODO

### <input class="task-list-item-checkbox" disabled type="checkbox"> 新增編輯器的語法色彩強調

:::spoiler {state=open} 問題敘述
> 我覺得編輯器應該要 highlight 控制序列，像是 `*` (Esc 字元)，不然要按 `Ctrl-V` 才看得出來到底是普通星號還是 esc。 \
> 也許和 BBS-Lua / pmore 的 syntax highlight 一起實作。 [name=IID] [time=2019_10_18]
:::

### <input class="task-list-item-checkbox" disabled type="checkbox"> 改用較新的系統函式

:::spoiler {state=open} 詳細計畫
> 之後的計畫：
- [ ] 把 shared memory 的操作函數從 System V 換成 POSIX 的。
- [ ] 把 semaphore 操作函數從 System V 換成 POSIX 的。
- [ ] 用 `epoll()` 取代 `select()`。
> http://man7.org/linux/man-pages/man7/shm_overview.7.html
http://man7.org/linux/man-pages/man7/sem_overview.7.html
http://man7.org/linux/man-pages/man7/epoll.7.html [name=IID] [time=2019_12_05]
:::

### Unicode 支援計畫

:::spoiler {state=open} 詳細計畫
> 編碼相關的計畫： [name=IID] [time=2019_12_05]
- [ ] 支援從 Big-5 UAO 轉 utf-8，以及反向轉換 (參考 PttBBS)
- [ ] 支援 utf-8 文章
   - [ ] `HDR` 要增加編碼的 flag，或是自動辨識編碼，誤判時再讓使用者選擇編碼
   - [ ] 最好讓 pfterm 支援完整的 utf-8
   - [ ] 要有辦法支援 unicode 全形字元的的一字雙色
- [ ] 將系統說明文件全部轉成 utf-8
- [ ] 把系統 UI 轉成 utf-8 (參考 FormosaBBS；可能不會實行)
   - [ ] 把訊息字串改成 utf-8
      - [ ] 至少要把訊息從原始碼中分離
   - [ ] 轉換所有有放字串的資料結構，或是增加相關 flag
      - 有點複雜，而且這樣要把放字串的空間調大，可能不會實行
:::

### 專案目錄結構重整的計畫

:::spoiler {state=open} 詳細計畫
> 計畫： [name=IID] [time=2020_02_24]
1. 重整專案目錄結構：
    - [ ] 建立可以放不同程式共用的函數庫的目錄 `common/` (放在 `lib/` 下也可)
        - 避免在 `lib/` 下建子目錄；盡量讓整個專案結構扁平化
            - 禁止第 3 層次的 Makefile
                - 之前試著編譯 MapleBBS-itoc 時很挫折，因為有些目錄下的子目錄有自己的 Makefile
            - 目前整個 DreamBBS 專案只有 `scripts/wsproxy/` 一個內層目錄，但沒有自己的 Makefile
    - [ ] 把 `xchatd` 從 `maple/` 移出，放到 `xchatd/` 下
    - [ ] 可把 `maple/` 改叫 `bbsd/`，或學 PttBBS 改叫 `mbbsd`
2. 重整理 header 檔：
    - [ ] 將不同支程式要 include 的 header 分開，不要全部都 `#include "bbs.h"`，這樣很難控制哪些程式該使用哪些函數
        - [ ] 將 `bbs.h` 拆成 `system.h` (deprecated), `common.h`, `bbsd.h`/`mbbsd.h` 三部分
            - `include/` 可以有子目錄，因為子目錄不需要自己的 Makefile
    - [ ] 參考 MapleBBS-itoc，將各個 header 內的特殊值整理出來
        - 可能需要將 `struct.h` 拆開
    - [ ] 避免在原始碼中自行宣告函數，統一使用 `#include`，不然容易出錯，而且用 C++ 編譯時容易失敗
:::

### 夢大行動版的方案構想

:::spoiler {state=open} 構想
> 行動版夢大構想： [name=IID] [time=2020_03_16]
- [ ] 偵測畫面寬度，小於 80 時改變排版，使其適應窄螢幕。
- 需要實作 `XO_KNEE` 相關功能，以額外顯示在窄螢幕下沒有空間可顯示的資訊。
    - [ ] 實作 `int *_knee(XO *xo)`，其中不同的 knee 的繪畫函數放在 `xo->xyz` 中 (或 `xo->knee`)，而 `*_knee()` 會去呼叫 `xo` 中所指定的函數；為 `NULL` 時則不畫出 knee。
    - 注意 `*_knee()` 不一定要畫在列表內容之下，而是可以覆蓋部分列表內容。
- [ ] 在窄螢幕下預設開啟 `XO_KNEE`。

> 然後是畫面大小，我覺得使用 pfterm 時可以考慮改限制畫面的長寬乘積，這樣更容易控制記憶體使用量。 \
使用 visio screen 時，因為畫面的資料結構的 row 與 column 大皆為固定大小，只能分開限制畫面長寬。 [name=IID] [time=2020_03_16]

> 應該要可以隱藏網址列。
https://developers.google.com/web/fundamentals/native-hardware/fullscreen?hl=zh-tw [name=IID] [time=2021_04_08 (Thu) 21:43 UTC+8]
:::

### MapleBBS 4 Roadmap

:::spoiler {state=open} 詳細計畫
> MapleBBS 4 Roadmap，TODO 格式: [name=IID] [time=2020_12_06]
- [ ] 整理尚未記載的重要資料至 DreamBBS wiki
    - [ ] 轉信設定、BBS 信箱設定等系統設定（可參考 PttBBS）
    - [ ] `sample/` 及 `scripts/` 下各腳本的使用時機及使用方式
    - [ ] MapleBBS-itoc 精華區資源（需取得原作者同意）
- [ ] 將目前夢大的系統說明檔案移到 DreamBBS 專案中
- [ ] 架站時不再依賴 `dreambbs_snap` 的資料夾骨架，像 Pirate BBS 一樣有初始化資料夾結構的腳本，可以從零開始架站
- [ ] 必要核心功能與附加功能的分離
    - [ ] 整理出哪些功能是 BBS 系統的必要功能
    - [ ] 將非必要功能模組化並移出主程式資料夾（`bbsd/`）
    - [ ] 將選單與按鍵功能的設定的相關程式碼獨立出來，並移出主程式資料夾（`bbsd/`）
    - [ ] 整合其它 MapleBBS 3 主要分支的核心功能的修正與改進
- [ ] MapleBBS 4 釋出計畫（專案成熟後再行考慮）
    - [ ] 寫專案總覽文件，簡述與 MapleBBS 3 的主要差異、以及目前 MapleBBS 3 各分支的開發狀況
    - [ ] 與 MapleBBS 原開發群討論名稱問題
:::

## 未完成的 TODO
- [ ] 修正公告未讀判斷 ==[name=IID] [time=2020_11_16]==
- [ ] 編輯 DreamBBS-202X v0 release note ==[name=IID] [time=2021_01_02]==

### 初步整理的 v3.1 的 TODO

:::spoiler {state=open} 詳細計畫
> DreamBBS v3.1 TODO: [name=IID] [time=2021_03_12]
- [ ] 合併 `so/*.c` 的 `XO` 初始化程式碼
- [ ] 增加 `FOR_IDX` 與 `FOR_PTR` 巨集
:::

### [#bbs-dev](https://discord.com/channels/330361502643257345/370600485612290060/) 中的部份 TODO（剩下長期 TODO 未完成）

:::spoiler {state=open} 詳細計畫
> TODO (搬移自 [#bbs-dev](https://discord.com/channels/330361502643257345/370600485612290060/)): \
*[中略...]* \
**長期 TODO** [name=IID] [time=2021_01_22]
- [ ] TODO: 檢視所有 C-style 顯式轉型，消除會困惑程式碼讀者或是會引發未指定行為的指標轉型 ==[name=IID] [time=2020_12_10]==
- [ ] 新增 wiki 頁面補充 `SGR` (`ESC '[' ... 'm'`) 的 parameter 與對應功能的說明 (<https://github.com/ccns/dreambbs/wiki/ANSI-Escape-Code-Standard-Format-zh_tw>) ==[name=IID] [time=2021_01_06]==
- [ ] 整理近期 MapleBBS 3.0x 發展的參考資料至 __in2__ 討論板
    > 我用最近挖掘的證據鬧板看看好了。 [name=IID] [time=2021_01_10]
:::

### 這學期的目標

:::spoiler {state=open} 詳細計畫
> Long term TODO (大致定為這學期的目標): [name=IID] [time=2021_01_17]
- [ ] 設定更多自動化開發工具，協助專案維護
- [ ] 設定編輯程式碼所使用的開發環境，增進開發效率
- [ ] 在 GitHub wiki 上記錄如何設定以上工具
- [ ] 在 GitHub wiki 上撰寫針對 BBS 使用者的簡易使用介紹以及使用介面截圖
- [ ] 建立 DreamBBS 專案的 GitHub page，內容包含：
    - [ ] GitHub wiki 的定期備份
    - [ ] 程式碼導覽 (內容類似先前實驗過的 Doxygen 頁面)
:::

## [#bbs-dev](https://discord.com/channels/330361502643257345/370600485612290060/) 整理 (by @r2#7033)

> Bug List, 之後應該整理到 HackMD 或在 GitHub 開 issue 追蹤 [name=r2] [time=2021_04_07 (Wed) 00:30 UTC+8]

### 夢大 Bug List - 2017-10-20, by @r2#7033
> - Updated on 2019-10-19, by IID (@Dom2112#3054)

:::spoiler {state=open} 詳細內容
> 回顧二年前的已知問題，記錄現在已修了多少。

- [x] 1. 簽名檔: 有三個可選，但只能編輯一個 (已經改善提示介面)
2. 看板:
    - [ ] 如果採用(B)回文模式，在看板上的推文會與信件底下的同步
    - [x] 進不去看板但看板state仍會顯示該板板名，且會累積人氣… <http://cpu.tfcis.org:8080/bmore?itoc&10484>
        - > 見 <https://github.com/ccns/dreambbs/commit/347f33a66baa946ac6bdf010e3a320d865ab89b8>，我把確認可否進入看板的邏輯，往前移到了函數開頭處 [name=IID] [time=2019_10_19]
- [x] 3.轉寄: 大 F 因為權限相關安全問題停到現在… (不太好解決orz)
    - postfix 問題近期找時間解決
    - 權限問題再看 code 能解決多少
    - > 見 <https://github.com/ccns/dreambbs/commit/c2ecedc0be94c88468a8582b6f7e079940197789> 及 <https://github.com/ccns/dreambbs/commit/f3130caeae0647a94a38601dda7e6c589bd4cd78>， \
    先前問題在於轉寄／轉貼多篇文章時，即使其中幾篇已經被權限系統擋下，界面上仍然是顯示轉寄／轉貼成功； \
    我加了「成功計數器」解決了這個問題， \
    順便解決了可以透過無效轉貼文章刷個人文章數的問題 <span class=inline-spoiler-group><span class=inline-spoiler>我利用這個 bug 把自己的文章數刷到超過<span class=inline-spoiler>一</span><span class=inline-spoiler>百</span><span class=inline-spoiler>萬</span>篇</span></span> [name=IID] [time=2019_10_19]
- [x] 4. 最愛: 在我的最愛那邊時，使用者名單顯示的動態是顯示該人上一個Q的ID…
    - > 見 <https://github.com/ccns/dreambbs/commit/39510cd5055238851b7aa80d11d4c9c46d7005d5>， \
       問題在於沒有定義到 `M_MYFAVORITE` 的 xover zone，造成取 `XZ_MYFAVORITE` 時發生 out-of-bounds accessing；補上去就修好了) [name=IID] [time=2019_10_19]
- [ ] 5. 好讀版: 圖片影片連結已經無法預覽。
    - (其他細節待補)
- [x] 6. 人氣: 記錄顯示人氣的daemon爆掉了。 (人氣自動+n*10)
    - (重開機後觀察暫時是解決了)
- [x] 7. 最愛: h 功能目前有 bug 未解 (solved)
    + [x] 在那邊 s 進看板出來後，上面的指令提示選項還是 for 看板文章目錄… (solved)
- [x] 8. 點歌: `確定點歌嗎 [Y/n]：` 預設是 n，應該改成 `確定點歌嗎 [y/N]：` (done)
- [x] 9. 遊戲: 小雞檔案缺漏，導致不正常斷線 (done)
- [x] 10. 點歌: 點歌次數無法累積! (solved)
:::

---

> 不確定有沒有 solved 的 [name=r2] [time=2021_04_07 (Wed) 00:30 UTC+8]

### <input class="task-list-item-checkbox" disabled type="checkbox"> 修正 `ulist_fromchange()` 中的 buffer 大小不同步的問題
:::spoiler {state=open} 問題敘述
> 大小不同步了。
![](https://cdn.discordapp.com/attachments/370600485612290060/827768186111262720/unknown.png) [name=IID] [time=2021_04_03]
:::

---

> staled TODO [name=r2] [time=2021_04_07 (Wed) 00:37 UTC+8]

- > *[中略...]*

### <input class="task-list-item-checkbox" disabled type="checkbox"> 實作有橫向選項的彈出式選單
:::spoiler {state=open} 問題敘述
> 新版彈出式選單的構想圖。
  ![](https://cdn.discordapp.com/attachments/370600485612290060/689081754543194132/1584358576207.png) [name=IID] [time=2020_03_16]
:::

---

> 這兩個 TODO 或 memo 也 unpinned, 理由一樣, 盡量整理在同一個地方 [name=r2] [time=2021_04_07 (Wed) 00:56 UTC+8]

### <input class="task-list-item-checkbox" disabled type="checkbox"> 支援 Xover 清單在含有超大量文章時的絕對移動
:::spoiler {state=open} 問題敘述
> 如果要支援超大量文章的話，就要直接判斷移動範圍，太遠時就直接設定 `xo->pos`。 \
> 大概是
```cpp
xo->pos = pos;
return XR_BODY + XO_REL + 0;
```
> 這樣。 [name=IID] [time=2021_04_06]

- 替代方案：
```cpp
return XO_DMOVE + pos;
```
其中 `XO_DMOVE` (Xover distant movement) 爲 `XO_DL`。
> [name=IID] [time=2021_09_30 (Thu) 20:04 UTC+8]
:::

### <input class="task-list-item-checkbox" disabled type="checkbox"> 實作以 Tab 字元劃分欄位的推文的顯示
:::spoiler {state=open} 問題敘述
> 我想，要讓推文有完整時間與 IP 來源的話，可以利用 Tab。 \
因為 `vget()` 不允許輸入 Tab 字元，所以 Tab 可以拿來當作分隔字元。 \
大概像這樣： \
`→          ID：推文內容<tab>2001:288:7001:249::140<tab>2021/04/03 13:42:23` \
然後實際顯示時根據畫面寬度決定要顯示多少。 [name=IID] [time=2021_04_07]
:::

## 其它 issue 連結

- DreamBBS 官方 Git repository 的 issue 清單：<https://github.com/ccns/dreambbs/issues>
- IID 的 issue 清單：<https://github.com/IepIweidieng/dreambbs/projects>