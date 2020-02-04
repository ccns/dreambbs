# 畫面座標系統

這篇文章介紹在 BBS 系統中處理文字輸出的位置時，會使用的幾種意義不同的座標系統。

## 座標軸定義
　        　   | 傳統 (Pirate BBS)        | MapleBBS-itoc
:---           | ---                     | ---
表示法         | `(y, x)`                 | `(x, y)`
類型           | 二維的左手直角座標系，向上為 -y <br> 電腦繪圖領域常用    | 二維的右手直角座標系，向上為 -x
顯示座標範圍    | 最左上角為 `(0, 0)` <br> 最右下角為 `(b_lines, b_cols)` | 最左上角為 `(0, 0)` <br> 最右下角為 `(b_lines, b_cols)`
x 軸           | x 為 column；+x 方向向右 | x 為 row；+x 方向向下
y 軸           | y 為 row；+y 方向向下    | y 為 column；+y 方向向右
右手定則之 z 軸 | 入螢幕面                | 出螢幕面

- 以下為方便說明，使用傳統定義 (`(y, x)`)

## 不同的座標系統間的比較
　              | 原始字元座標                | 顯示座標              | 畫面大小座標 (DreamBBS v3 新增)
:---            | ---                        | ---                  | ---
定義            | 包含控制碼之字元位置        | 不包含控制碼之字元位置 | 可隨畫面大小變化對應不同顯示座標的字元位置
與前者關係      | --                         | - 在 `(y, 0)` 至 `(y, x)` 範圍內不包含任意控制碼之任意部分時，等同於原始字元座標 <br> - 不計移動控制碼效果，則 y 位置等同於原始字元座標之 y 位置 | 在 `(-T_LINES_OFF_MAX, -T_COLS_OFF_MAX)` 至 `(T_LINES_OFF_MAX-1, T_COLS_OFF_MAX-1)` 之矩形範圍內，等同於顯示座標
目的            | - 處理要輸出畫面之原始字元資料 (screen & visio) <br> - (不存在) (pfterm) | - 計算輸出畫面後的顯示位置 <br> - 處理要輸出畫面之字元資料 (pfterm) | 處理畫面大小改變時的顯示位置變化
作為原始座標     | - screen <br> - visio      | pfterm              | (無)
最小有效座標     | `(0, 0)`                   | `(0, 0)`            | - (理論上無限制) <br> - `(0, 0)` (目前之簡化實作)
最大有效座標     | `(b_lines, ANSILINELEN-1)` | `(b_lines, b_cols)` | - (理論上無限制) <br> - `(B_LINES_REF, B_COLS_REF)` (目前之簡化實作)
真實畫面上的游標座標 | (無)                     | - `(tc_line, tc_col)` (screen) <br> - `(tc_row, tc_col)` (visio) <br> - `(ft.ry, ft.rx)` (pfterm) | (無)
內部使用的游標座標 | - `(cur_ln, cur_col)` (screen) <br> - `(cur_row, cur_col)` (visio) | - `(cur_ln, <動態算出>)` (PttBBS screen) <br> - `(cur_row, cur_pos)` (visio) (`move()` 後 `cur_pos` 變為原始字元座標而無效) <br> - `(ft.y, ft.x)` (pfterm) | (無)
取得內部使用的游標座標 | - `getyx()` (screen & visio) <br> - `getxy()` (MapleBBS-itoc visio; 被註解不可用) | - `getyx_ansi()` (pfterm & PttBBS screen) <br> - `getyx()` (pfterm) | (無)
設定內部使用的游標座標 | `move()` (screen & visio) | - `ansi_move()` (MapleBBS 3) <br> - `move_ansi()` (pfterm & PttBBS screen; DreamBBS v3 起改用此名) <br> - `move()` (pfterm) | `move_ref()`
轉成原始字元座標 | --                     | 一對多，但不考慮控制碼時為一對一 <br> - `move_ansi(y, x), getyx()` (screen & visio) | 多對多，但不考慮控制碼時為多對一 <br> - `move_ref(y, x), getyx()` (screen & visio)
轉成顯示座標     | 多對一，但不考慮控制碼時為一對一 <br> - `getyx_ansi()` (PttBBS screen) | --                  | 多對一 <br> - `gety_ref(y)` & `getx_ref(x)`
轉成畫面大小座標 | 多對多，但不考慮控制碼時為一對多 <br> - `getyx_ansi()` (PttBBS screen) | 一對多 (但本身已是畫面大小座標) | --

註: 這裡所稱的 `visio` 為 MapleBBS 3 之系統輸出入函數庫，非 PttBBS 中現名為 `vtuikit` 的使用者介面函數庫

## 「原始字元座標」與「顯示座標」的範例解說
　                  | Row 中內容
:---                | ---
顯示文字 (忽略格式)  | `Hello, world!　　　　　`
對應原始字元座標之 x | `0123456123456012345678` (個位) <br> `0000000111111222222222` (十位)
對應顯示座標之 x     | `0123456789012345678901` (個位) <br> `0000000000111111111122` (十位)
原始字元            | `Hello, *[1mworld!*[m　　　　　` (`*` 表示 `'\x1b'` (`ESC`))
對應原始字元座標之 x | `01234567890123456789012345678` (個位) <br> `00000000001111111111222222222` (十位)
對應顯示座標之 x     | `0123456????789012???345678901` (個位) <br> `0000000????000111???111111122` (十位) (記為 `?` 之座標無定義值，實際值依實作不同)
對應顯示座標之 x (PttBBS screen) | `01234566666789012222345678901` (個位) <br> `00000000000000111111111111122` (十位)
實際輸出後所對應的顯示座標之 x | `01234567777789012333345678901` (個位) <br> `00000000000000111111111111122` (十位)

- `Hello, ` 共 7 個字元的顯示座標與原始字元座標相同
- 字母 `w` 的原始字元座標之 x 為 `11`，顯示座標之 x 則是 `7`
- 控制碼 `*[1m` 共 4 個字的原始字元座標為 `7`-`10`，對應的顯示座標之 `x` 則沒有定義，使用 PttBBS 的 screen 函數庫計算會得到與實際輸出位置 (`7`) 不同的 `6`
- 控制碼 `*[m` 共 3 個字的原始字元座標為 `17`-`19`，對應的顯示座標之 `x` 則沒有定義，使用 PttBBS 的 screen 函數庫計算會得到與實際輸出位置 (`13`) 不同的 `12`

## 「畫面大小座標」(DreamBBS v3 新增) 的解說
### 「顯示座標」值域之使用
由於顯示座標的 y, x 座標值，實際用到的值域很小：
- PttBBS 限制最大大小為 `(100, 200)` (顯示範圍內最右下角為 `(99, 199)`)；
- MapleBBS-itoc 與 DreamBBS v1 起限制最大大小為 `(T_LINES, T_COLS)`，也就是 `(50, 120)` (顯示範圍內最右下角為 `(49, 119)`)；

因此可以利用顯示座標中的無效值域，使用常數記錄相對於畫面大小的位置資料。

### 說明
「畫面大小座標」的座標軸使用 `segment` 和 `offset` 兩部分加權相加，表達一個位置。
- `segment` 是將一個畫面的高度或寬度分割成 `T_LINES_DIV_RES` 或 `T_COLS_DIV_RES` (都為 `128`) 後的一段
    - 值域理論上無上下限，目前實作限制最小為 `0`，最大為 `T_LINES_DIV_RES` 或 `T_COLS_DIV_RES`
    - 需要這麼多精度，是因為一般排版時，通常會需要「畫面的幾分之幾」的高度或寬度的資訊，而這些比例的分母不一定能整除 `segment` 的數量，只能近似
        - 例如畫面寬度的 `1/3`，可以用 `43/128` 來近似，誤差是畫面寬度的 `1/384`，在寬度都不超過 `192` 的畫面上沒有差異
- `offset` 是目標位置與 `segment` 開始位置的差異
    - 最小為 `-T_LINES_OFF_MAX` 或 `-T_COLS_OFF_MAX` (都為 `-512`)，最大為 `T_LINES_OFF_MAX - 1` 或 `T_COLS_OFF_MAX` (都為 `511`)
    - `segment` 的加權值是 `offset` 的範圍，所以 `offset` 超出範圍後，會被當作在別的 `segment` 內
    - 考慮邊界條件，假設畫面高度為 `T_LINES_OFF_MAX`：
        - 設定 `segment` 為 `0` 時，畫面顯示範圍最下方的 row (y 為 `T_LINES_OFF_MAX - 1`) 對應的 `offset` 值是 `T_LINES_OFF_MAX - 1`
            - `segment` 的開始位置是 `0`，`offset` 是 `T_LINES_OFF_MAX - 1`，加起來會得到 `T_LINES_OFF_MAX - 1`
        - 而設定 `segment` 為 `T_LINES_DIV_RES` 時，畫面顯示範圍最上方的 row (y 為 `0`) 對應的 `offset` 值是 `-T_LINES_OFF_MAX`
            - `segment` 的開始位置是 `T_LINES_OFF_MAX`，`offset` 是 `-T_LINES_OFF_MAX`，加起來是 `0`
        - 因此設定 `segment` 在最小 `0` 及最大 `-T_LINES_OFF_MAX` 的範圍內時，畫面顯示範圍中的每一 row 都可以用「畫面大小座標」來表達
    - 同樣地，假設畫面寬度為 `T_COLS_OFF_MAX`，設定 `segment` 在最小 `0` 及最大 `-T_COLS_OFF_MAX` 的範圍內時，畫面顯示範圍中的每一 column 也都可以用「畫面大小座標」來表達
- `T_LINES_REF` 和 `T_COLS_REF` 是 `segment` 分別設定為 `T_LINES_DIV_RES` (開始位置為畫面高度) 和 `T_COLS_DIV_RES` (開始位置為畫面寬度)，且 `offset` 為 `0` 的值，分別代表畫面高度和寬度
- 符合以下條件時，畫面大小座標的 `(T_LINES_REF/a + b, T_COLS_REF/c + d)` 對應顯示座標的 `(t_lines/a + b, t_cols/c + d)`：
    - `a` 和 `c` 要整除 `segment` 的數量 (`T_LINES_DIV_RES` 或 `T_COLS_DIV_RES`)
    - `b` 和 `d` 要在 `offset` 的範圍內 (最小 `-T_LINES_OFF_MAX` 或 `-T_COLS_OFF_MAX`，最大 `T_LINES_OFF_MAX - 1` 或 `T_COLS_OFF_MAX - 1`)

### 範例
位置　                | 顯示座標                      | 畫面大小座標 (常數)
:---                 | ---                           | ---
畫面中央處            | `(t_lines/2, t_columns/2)`    | `(T_LINES_REF/2, T_COLS_REF/2)`
畫面底行左方 1/3 處   | `(b_lines, t_columns/3)`      | `(B_LINES_REF, T_COLS_REF*43/128)`

## 座標特殊值
　                | 值                 | 類型                      | 出處            | 說明
:---              | ---                | ---                       | ---            | ---
`LINELEN`         | - `80` <br> - (移除) (Eagles BBS) | 原始字元座標與顯示座標之 x   | Pirate BBS    | 單行最大字元數，也是最大畫面寬度 <br> (Pirate BBS 並未處理控制碼)
`ANSILINELEN`     | - `511` (PttBBS) <br> - `500` (MapleBBS 3) | 原始字元座標之 x           | Eagles BBS    | 單行最大字元數
`t_lines`         | - (變數，預設 `24`) <br> - `24` (MapleBBS 3) <br> - (移除) (MapleBBS-itoc) <br> - `b_lines + 1` (DreamBBS v1) | 顯示座標之 y               | Pirate BBS    | 畫面高度
`b_lines`         | - `t_lines - 1` (MapleBBS 2 & PttBBS) <br> - `23` (MapleBBS 3) <br> - (變數，預設 `23`) (MapleBBS-itoc & DreamBBS v1) | 顯示座標之 y               | MapleBBS 2    | 顯示座標之 y 的有效最大值
`p_lines`         | - `t_lines - 4` (MapleBBS 2 & PttBBS) <br> - `18` (MapleBBS 3) <br> - (移除) (MapleBBS-itoc) <br> - `b_lines - 5` (DreamBBS v1) | 顯示座標之 y               | MapleBBS 2    | 除去標題列、說明列、狀態列後的畫面高度 <br> - 頁面捲動用
`t_columns`       | - (變數，預設 `80`) <br> - `80` (MapleBBS 3) <br> - (移除) (MapleBBS-itoc) <br> - `b_cols + 1` (DreamBBS v1) | 顯示座標之 x               | Pirate BBS    | 畫面寬度
`b_cols`          | (變數，預設 `79`)   | 顯示座標之 x               | MapleBBS-itoc | 顯示座標之 x 的有效最大值 <br> (DreamBBS v1 時引入)
`d_cols`          | `b_cols - 79`      | 顯示座標之 x               | MapleBBS-itoc | 畫面寬度與標準寬度 `80` 的差 <br> (DreamBBS v1 時引入)
`T_LINES`         | `50`               | 顯示座標之 y               | MapleBBS-itoc | (最大畫面高度 <br> DreamBBS v1 時引入)
`T_COLS`          | `120`              | 顯示座標之 x               | MapleBBS-itoc | (最大畫面寬度 <br> DreamBBS v1 時引入)
`T_LINES_REF`     | `T_LINES_DIV_RES * 2*T_LINES_OFF_MAX` | 畫面大小座標之 y           | DreamBBS v3   | 對應畫面高度 (`t_lines`) 的值
`T_LINES_DIV_RES` | `128U`             | 畫面大小座標之 y           | DreamBBS v3   | 畫面高度 `segment` 的數量
`T_LINES_OFF_MAX` | `512U`             | 畫面大小座標之 y           | DreamBBS v3   | 畫面高度 `offset` 的絕對值範圍 <br> - 至少要與 `T_LINES` 一樣大
`B_LINES_REF`     | `T_LINES_REF - 1`  | 畫面大小座標之 y           | DreamBBS v3   | 對應 `b_lines` 的值
`P_LINES_REF`     | `B_LINES_REF - 5`  | 畫面大小座標之 y           | DreamBBS v3   | 對應 `p_lines` 的值
`T_COLS_REF`      | `T_COLS_DIV_RES * 2*T_COLS_OFF_MAX` | 畫面大小座標之 x           | DreamBBS v3   | 對應畫面寬度 (`t_columns`) 的值
`T_COLS_DIV_RES`  | `128U`             | 畫面大小座標之 x           | DreamBBS v3   | 畫面寬度 `segment` 的數量
`T_COLS_OFF_MAX`  | `512U`             | 畫面大小座標之 x           | DreamBBS v3   | 畫面寬度 `offset` 的絕對值範圍 <br> - 至少要與 `T_COLS` 一樣大
`B_COLS_REF`      | `T_COLS_REF - 1`   | 畫面大小座標之 x           | DreamBBS v3   | 對應 `b_cols` 的值
`D_COLS_REF`      | `B_COLS_REF - 79`  | 畫面大小座標之 x           | DreamBBS v3   | 對應 `d_cols` 的值

- MapleBBS 3 及更舊的直系分支，不支援 telnet 協定的畫面大小改變指令，僅有 shell 模式下的畫面大小改變機制
- MapleBBS 3 取消了 shell 模式，因此沒有改變畫面大小的機制
- PttBBS 與 MapleBBS-itoc 分別增加了 telnet 協定的畫面大小改變指令的支援；DreamBBS v1 從 MapleBBS-itoc 將其引入