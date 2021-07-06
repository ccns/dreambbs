# 與 MapleBBS 3 的按鍵差異

本文說明 DreamBBS 與其它 MapleBBS 3 分支的按鍵的輸入系統與對應功能的差異。

目前本文以列舉 DreamBBS 的目前版本與 DreamBBS 2010 的差異為主。

預計未來將再加入更多與其它 MapleBBS 3 主要分支的比較。

## v0.95 (3.10.95)
- 文章列表：<kbd>%</kbd> 鍵可推文 (同 <kbd>X</kbd>) ([125219571](https://github.com/ccns/dreambbs/commit/12521957189b4da9a44ff30ab0259e6e824561dc))
    - 閱讀文章時也可 ([6cad7ae9f](https://github.com/ccns/dreambbs/commit/6cad7ae9f31b7b32731ca8b1ad75f64b25edfd14))
- 主選單：按 <kbd>s</kbd> 可以搜尋看板 (同 <kbd>Ctrl</kbd>-<kbd>S</kbd>) ([3742a2766](https://github.com/ccns/dreambbs/commit/3742a27669ce517bb00c58f6f9cbd74137eeea35))
- 我的最愛：<kbd>a</kbd> 可以新增項目 (同 <kbd>Ctrl</kbd>-<kbd>P</kbd>) ([da39b02d5](https://github.com/ccns/dreambbs/commit/da39b02d5ca2ef4f196dfa3d319c2ac47f33789a))
- xover 選單：
    - 按 <kbd>e</kbd> 可以跳回上層 (同 <kbd>←</kbd>) ([a868a17f2](https://github.com/ccns/dreambbs/commit/a868a17f2fdb7cb50a61c2099cbbf5f840a42577))
    - <kbd>p</kbd>/<kbd>n</kbd> 與 <kbd>k</kbd>/<kbd>j</kbd> 可移至上/下一項 (同 <kbd>↑</kbd>/<kbd>↓</kbd>) ([550deb4b3](https://github.com/ccns/dreambbs/commit/550deb4b30ab209c920799611976d22f55c3a052))
- 文章列表：<kbd>~</kbd> 鍵可叫出主題搜尋介面 (同 <kbd>/</kbd>) ([e8e4e215e](https://github.com/ccns/dreambbs/commit/e8e4e215e43e2b4e5357809930e61b44ea10de9a))

## v1.0-alpha3 (3.11.0)
- 輸入系統：
    - 恢復 PheonixBBS 式 (類 PttBBS) 的特殊按鍵的正數按鍵碼 ([5f16be63b](https://github.com/ccns/dreambbs/commit/5f16be63bb8776c07bb01276ac06b637fe0b9192))
        - 解決特殊按鍵的負數按鍵碼與 xover callback 的 flag 衝突的問題，讓 xover callback 列表可以使用特殊按鍵作為 key
    - 支援 <kbd>F1</kbd> ~ <kbd>F12</kbd> ([5f16be63b](https://github.com/ccns/dreambbs/commit/5f16be63bb8776c07bb01276ac06b637fe0b9192))
    - 支援 <kbd>Shift</kbd>-<kbd>Tab</kbd> ([8b9343910](https://github.com/ccns/dreambbs/commit/8b934391017e9355f301308d087adb75248be843))

## v2.0-rc1 (3.12.0)
- 輸入系統：
    - 重新啟用對 `Meta(c)` (<kbd>Esc</kbd> 加按鍵) 的處理；即使不定義 `TRAP_ESC` 也使用 `Meta(c)` ([ebde965c4](https://github.com/ccns/dreambbs/commit/ebde965c4bc0c147825e081590f93c592ae840fb))
    - 參考 PttBBS，支援更多瀏覽程式的特殊按鍵的控制碼 ([592dd971d](https://github.com/ccns/dreambbs/commit/592dd971d1dc93dbc73ae7bba4a170824abe54b3))
    - 調整特殊按鍵的按鍵碼的取值，以配合與 `Meta(c)`、`Shift(c)` ([af186d205](https://github.com/ccns/dreambbs/commit/af186d205ed35ecb1580d5b64fbcb335a73ae384) 新增)、`Ctrl(c)` 的組合 ([9d4c6bf47](https://github.com/ccns/dreambbs/commit/9d4c6bf47824717c1514972648591851b1c50683))
    - 支援 <kbd>Ctrl</kbd>/<kbd>Shift</kbd>/<kbd>Esc</kbd> 與特殊按鍵的組合 ([5398b5072](https://github.com/ccns/dreambbs/commit/5398b50721cd651f90377bcd90eb4285dc894e7e))
- 文章瀏覽：
    - 按 <kbd>L</kbd>/<kbd>l</kbd> 可以執行 BBS-Lua ([effe3c894](https://github.com/ccns/dreambbs/commit/effe3c894389f9b0d954ce7ec0e0ada8da91811d))
    - Maple3 傳統文章瀏覽器：<kbd>H</kbd> 鍵與 <kbd>F1</kbd> 鍵也可叫出幫助畫面 (同 <kbd>h</kbd>/<kbd>?</kbd>) ([a98db13cb](https://github.com/ccns/dreambbs/commit/a98db13cbcb108fe2a7050a16b0156bcad792691))
    - pmore 文章瀏覽器：<kbd>F1</kbd> 鍵可叫出幫助畫面 (同 <kbd>h</kbd>/<kbd>H</kbd>/<kbd>?</kbd>) ([17b5a87b5](https://github.com/ccns/dreambbs/commit/17b5a87b52e3520792d29871bfc14850dcd9c972))
    - 按 <kbd>!</kbd> 可以執行 BBS-Ruby ([400f19ce5](https://github.com/ccns/dreambbs/commit/400f19ce52c1416fae6ad78a0545af2bfb17d441))
- `vget`：
    - 可以按 <kbd>Del</kbd> 鍵刪除字元 (同 <kbd>Ctrl</kbd>-<kbd>D</kbd>) ([f92103667](https://github.com/ccns/dreambbs/commit/f92103667f8caf646064643253891c4bcd9dac62))
    - `NUMECHO` 模式可以按基本編輯鍵 (與 PttBBS 比較的差別；DreamBBS 2010 沒有 `NUMECHO`) ([41ceafb46](https://github.com/ccns/dreambbs/commit/41ceafb468887caba7a65c88eeb97587e19ece9f))
- BBS-Lua：支援 <kbd>Ctrl</kbd>/<kbd>Shift</kbd>/<kbd>Esc</kbd> 與特殊按鍵的組合 ([bf960173e](https://github.com/ccns/dreambbs/commit/bf960173eaf42f8cfdeccd806b96556d59668907))
- 編輯器：
    - 移植一些 PttBBS 快速鍵 ([a24c6c401](https://github.com/ccns/dreambbs/commit/a24c6c401cacdeadd71f46474627f8ea1cb1c36d))
        - <kbd>Esc</kbd>-<kbd>o</kbd> 可切換 insert mode (同 <kbd>Ins</kbd>)
        - <kbd>Esc</kbd>-<kbd>v</kbd> 可上捲一頁 (同 <kbd>PgUp</kbd>)
        - <kbd>Esc</kbd>-<kbd>.</kbd> 可跳到文章結尾 (同 <kbd>Ctrl</kbd>-<kbd>T</kbd>)
        - <kbd>Esc</kbd>-<kbd>,</kbd> 可跳到文章開頭 (同 <kbd>Ctrl</kbd>-<kbd>S</kbd>)
        - <kbd>Esc</kbd>-<kbd>A</kbd> 與 <kbd>Esc</kbd>-<kbd>a</kbd> 可切換 ANSI mode (同 <kbd>Ctrl</kbd>-<kbd>V</kbd>)
        - <kbd>Esc</kbd>-<kbd>X</kbd> 與 <kbd>F10</kbd> 可叫出檔案選單 (同 <kbd>Ctrl</kbd>-<kbd>X</kbd>)
        - <kbd>F1</kbd> 鍵可叫出幫助畫面 (同 <kbd>Ctrl</kbd>-<kbd>Z</kbd>)
        - <kbd>Esc</kbd>-<kbd>U</kbd> 與 <kbd>F8</kbd> 可叫出線上使用者名單 (原 <kbd>Ctrl</kbd>-<kbd>U</kbd>)
    - <kbd>Ctrl</kbd>-<kbd>U</kbd> 改成輸入 `Esc` 字元 ([92d116ec3](https://github.com/ccns/dreambbs/commit/92d116ec372ff39ec6b7b9d39cef8eea0c3144c9))
    - 可以按 <kbd>Esc</kbd>-<kbd>1</kbd> 到 <kbd>Esc</kbd>-<kbd>5</kbd> 來直接貼上 1 到 5 號暫存檔的內容 ([255c1276a](https://github.com/ccns/dreambbs/commit/255c1276afba8e7a4153febe1c56148c3d78c9b2))
- 使用者名單：移除測試用 <kbd>V</kbd> 鍵 ([9a34b6a24](https://github.com/ccns/dreambbs/commit/9a34b6a249d2a8866d448cf7cf039b8ec0860cc6))

## v2.1-rc1 (3.12.1)
- 編輯器：可以在 ANSI 模式中直接使用 <kbd>Backspace</kbd>，不需離開 ANSI 模式 ([d52ad44f2](https://github.com/ccns/dreambbs/commit/d52ad44f29c75765ce573833fed2a9eeb02620ff))
- 傳統切換選單：按 <kbd>f</kbd> 鍵可正確進入夢大式我的最愛列表 ([10598d476](https://github.com/ccns/dreambbs/commit/10598d476749b00bd3ee415d2caefa35b29aadc0))
- 分類看板列表：移除 <kbd>Ctrl</kbd>-<kbd>P</kbd>、<kbd>d</kbd>、<kbd>M</kbd> 等 WindTop 式我的最愛專用按鍵 ([3760a95c1](https://github.com/ccns/dreambbs/commit/3760a95c1be5ba7caa0199fac831484be6735408))

## v3.0 (3.21.0)
- 輸入系統：
    - 改進按鍵控制碼的可能結尾字元的下一字元的處理 ([0d4885448](https://github.com/ccns/dreambbs/commit/0d48854480c3e49d63508880a23337b80d7c1436))
        - 可依下一字元是否逾時判斷按鍵是否已送完
        - 現在遇到不支援或不合法的控制碼時，若沒有遇到系統特殊按鍵碼，會回傳 `KEY_INVALID` 而不是最後一個字元
    - 實作按鍵控制碼的延時判斷 ([4e5f21070](https://github.com/ccns/dreambbs/commit/4e5f21070d50d765d02a87c6cee29dba80dd0fb4))
        - 現在按下 <kbd>Esc</kbd> 後 0.65 秒內沒按任何鍵會直接送出 <kbd>Esc</kbd> 鍵
            - 此時 <kbd>Ctrl</kbd>-<kbd>L</kbd> (重繪) 與 <kbd>Ctrl</kbd>-<kbd>R</kbd> (熱訊) 按鍵功能會暫時禁用，當作一般字元處理
        - 收到 `Esc` 的下一字元後要在 0.01 秒或更短的間隔內送出控制碼的其餘部分，否則視為送完按鍵
            - 可以使用 <kbd>Esc</kbd>-<kbd>O</kbd>、<kbd>Esc</kbd>-<kbd>[</kbd> 等控制碼與其它按鍵控制碼開頭衝突的按鍵
            - 防止以一個字元一個字元手動輸入控制碼的方式輸入特殊按鍵
    - 支援 `\xff` 字元的輸入 (telnet: `IAC IAC`) ([60b6353b7](https://github.com/ccns/dreambbs/commit/60b6353b7c108472dd07ce557e0ecbbf786bb55f))
    - 調整特殊按鍵的按鍵碼的取值 ([c1a7930a6](https://github.com/ccns/dreambbs/commit/c1a7930a6becc9031e3398eea45667f3c9bb7f43))
        - 騰出更多編碼空間，並預留未來可能增加支援的按鍵的空位
        - <kbd>Shift</kbd>-<kbd>Tab</kbd> 正名為 `KEY_BTAB`，原名 `KEY_STAB` 為了向舊版相容而保留
    - 可以按 <kbd>Esc</kbd>-<kbd>Ctrl</kbd>-<kbd>L</kbd> 手動送出調整畫面大小後的重繪指令 (`I_RESIZETERM`) ([cc14385ce](https://github.com/ccns/dreambbs/commit/cc14385ce7346d18aaf71a1469d77ce477ee5a1a))
    - `\r` 的處理改為與按鍵控制碼的 0.01 秒延時判斷類似，收到下一字元或逾時後才回傳按鍵 ([111524a46](https://github.com/ccns/dreambbs/commit/111524a468d57d1bc7a79f240ddd0ccd40a92fb5))
        - 這樣收到 `\r\0` 或 `\r\n` 後能立即離開 `vkey`，而不會停在 `igetch` 等下一字元
    - 改進按鍵控制碼的可能結尾字元的下一字元為其它按鍵控制碼開頭或系統特殊按鍵碼的處理 ([7b362c9ae](https://github.com/ccns/dreambbs/commit/7b362c9ae8485f6881926abf99197a27bbb465ab))
        - 將這一字元記錄起來並在下次進入 `igetch` 時回傳
        - 一律定義 `TRAP_ESC` 以確保可能已結束的按鍵控制碼不被緊接著的 `Esc` 無效化，並移除不使用的程式碼
    - 調整按鍵處理順序 ([1e9d59de7](https://github.com/ccns/dreambbs/commit/1e9d59de7e82357974d3943b3b75cb304f951cbd))
        - 將 `\r`、`\x7f` 與其它按鍵控制碼一起處理
        - 解析出按鍵後再處理重繪及熱訊等特殊按鍵功能
        - 現在按 <kbd>Ctrl</kbd>-<kbd>L</kbd> 重繪畫面後會再送出 `Ctrl('L')` 按鍵碼
    - 現在會過濾掉瀏覽程式送出的雙位元字的自動重複按鍵 ([df69c19ed2](https://github.com/ccns/dreambbs/commit/df69c19ed21a65c749f3b4d52f4928437bf4f5c0))
- `vget` 的自動完成列表 ([2b48a99db](https://github.com/ccns/dreambbs/commit/2b48a99dbdbcdff4ed16cfbfa0311b5b20a262a8))：
    - 按 <kbd>Space</kbd> 或 <kbd>Tab</kbd> 會自動完成
    - 按 <kbd>Enter</kbd>、<kbd>Space</kbd>、<kbd>Tab</kbd> 鍵會繼續列出，按其它鍵繼續輸入
- 編輯器：
    - 按 <kbd>Esc</kbd> 鍵後等 0.65 秒，或快速按兩下 <kbd>Esc</kbd> 鍵 (即 <kbd>Esc</kbd>-<kbd>Esc</kbd>) 可以輸入 `Esc` 字元 ([97812e94d](https://github.com/ccns/dreambbs/commit/97812e94d5d8bd7392a53f7dc1335e8c3a3c1c07))
    - 按 <kbd>Esc</kbd>-<kbd>r</kbd> 可開關雙位元字偵測 ([3fbbf54e4](https://github.com/ccns/dreambbs/commit/3fbbf54e4a01618b0f482f21da2c7ac784702dfb))
- 踩地雷遊戲：要用 <kbd>Esc</kbd> 鍵退出遊戲時，可快速按兩下以送出 <kbd>Esc</kbd>-<kbd>Esc</kbd> ([97812e94d](https://github.com/ccns/dreambbs/commit/97812e94d5d8bd7392a53f7dc1335e8c3a3c1c07))
- 現在許多選單可以用 <kbd>Esc</kbd> 與 <kbd>Esc</kbd>-<kbd>Esc</kbd> 跳回上層
    - 彈出式選單 ([73480e7b9](https://github.com/ccns/dreambbs/commit/73480e7b955996df67e7958f8b710e92c62a775b))
    - 主選單 ([8b9dc9e2d](https://github.com/ccns/dreambbs/commit/8b9dc9e2ddf74702a93b202e883aaba2b56f3bb4))
    - xover 選單 ([1967c852b](https://github.com/ccns/dreambbs/commit/1967c852b9a11480fd2aa6f36a2cd102fff564c7))
- xover 選單：按 <kbd>Esc</kbd>-<kbd>↑</kbd>/<kbd>Esc</kbd>-<kbd>↓</kbd> 或 <kbd>K</kbd>/<kbd>J</kbd> 可以上/下捲動一項  ([25a2f8b21](https://github.com/ccns/dreambbs/commit/25a2f8b21a93921f2071fbd3843ef86f622df927))
- 主選單：
    - 有多個豎排時，可以按 <kbd>←</kbd>/<kbd>→</kbd> 鍵橫向移動游標 ([90ab320c0](https://github.com/ccns/dreambbs/commit/90ab320c0dd16bbb43184f7639150ee6c6ed9319))
    - 現在選項第一字元也是快速鍵時不會自動觸發按鍵功能，只是不能按快速鍵跳過去 ([e54219ab8](https://github.com/ccns/dreambbs/commit/e54219ab8b3a520d616613d2cc208e476e6ace43))
    - 按 <kbd>S</kbd> 鍵或 <kbd>/</kbd> 鍵可以搜尋看板 (同 <kbd>S</kbd>/<kbd>Ctrl</kbd>-<kbd>s</kbd>) ([6c64679f5](https://github.com/ccns/dreambbs/commit/6c64679f5853600b03a83aa84744fc18054d14fd))
- 針對 PttChrome 中不能按的按鍵組合，所另外追加的替代快速鍵 ([998d34502](https://github.com/ccns/dreambbs/commit/998d3450214be77323b86974270ea47ae1e19d0a))
    - <kbd>Esc</kbd>-<kbd>R</kbd> (同 <kbd>Ctrl</kbd>-<kbd>R</kbd>)：
        - 回覆熱訊 ([1e9d59de7](https://github.com/ccns/dreambbs/commit/1e9d59de7e82357974d3943b3b75cb304f951cbd) 後才可正常使用)
        - 從熱訊介面進入詳細熱訊
    - <kbd>Esc</kbd>-<kbd>W</kbd> (同 <kbd>Ctrl</kbd>-<kbd>W</kbd>)：
        - 編輯器：叫出檔案選單
        - 文章列表：將文章標記為垃圾訊息
        - 精華區：進入精華區回收筒
    - <kbd>Esc</kbd>-<kbd>T</kbd> (同 <kbd>Ctrl</kbd>-<kbd>T</kbd>)：
        - 編輯器：跳到檔案結尾
        - xover 列表：標記同標題文章
        - 小雞對戰時叫出聊天紀錄
    - <kbd>Esc</kbd>-<kbd>A</kbd> (同 <kbd>Ctrl</kbd>-<kbd>A</kbd>) ([3d99c78ab](https://github.com/ccns/dreambbs/commit/3d99c78abc1c89c8362acf3c32de337a9cb57a20))：
       - 私訊聊天：執行 `BWboard` 遊戲 (但已移除；相關程式碼已禁用)
       - xover 列表：標記同作者文章
    - <kbd>Esc</kbd>-<kbd>N</kbd> (同 <kbd>Ctrl</kbd>-<kbd>N</kbd>) ([3d99c78ab](https://github.com/ccns/dreambbs/commit/3d99c78abc1c89c8362acf3c32de337a9cb57a20))：
       - 文章列表：直接從列表移除文章