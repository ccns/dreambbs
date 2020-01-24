# ANSI 控制碼標準格式

這裡所說的「ANSI 控制碼」，其實是 ECMA-48 標準中所稱的「control function」，分為格式不盡相同的 5 種。

## ECMA-48 中的各種 control function（「ANSI 控制碼」）的標準格式
(參考資料: [ECMA-48](http://www.ecma-international.org/publications/files/ECMA-ST/Ecma-048.pdf) chapter 5 `Coded representation`, chapter 8 `Control functions`)
### a) elements of the C0 set (表示法: `(C0)`): `'\x00'-'\x1f'`
C0 set 的功能表請見 ECMA-48 Table 1，或[直接查維基百科](https://zh.wikipedia.org/wiki/C0%E4%B8%8EC1%E6%8E%A7%E5%88%B6%E5%AD%97%E7%AC%A6)

### b) elements of the C1 set (表示法: `(C1)`): (7-bit mode) `ESC Fe` / (8-bit mode) `'\x80'-'\x9f'`
- `ESC`: `'\x1b'`
- `Fe`: `'\x40'('@')-'x5f'('_')` (`'@'`, `'A'-'Z'`, `[\]^_`)

C1 set 的具體功能表與 independent control functions 一起整理到了下面。

在 8-bit 環境下可以使用 7-bit mode 和 8-bit mode 的控制碼；而在 7-bit 環境下只能用 7-bit mode 的控制碼。

但因為 8-bit mode 的 C1 控制碼本身與 UTF-8 編碼衝突，且編碼時會編碼成兩個字元，沒有特別優勢，現在的 terminal 通常不支援。\
現在的 terminal 一般只支援 7-bit mode 的 C1 控制碼。

另外 8-bit mode 的 C1 控制碼也會和 Big5-UAO 衝突，更無法在現在許多中文 BBS 系統上使用。

### c) control sequences: `CSI P ... P I ... I F`
- `CSI`: (7-bit mode) `'\x1b' '\x5b'` (`ESC '['`) / (8-bit mode) `'\x9b'`
    - `CSI` 本身是 C1 控制碼
- `P` (parameter byte; optional): `'\x30'('0')-'\x3f'('?')` (`0123456789:;<=>?`)
- `I` (intermediate byte; optional): `'\x20'(SPACE)-'\x2f'('/')` (` !"#$%&'()*+,-./`)
- `F` (final byte): `'\x40'('@')-'\x7e'('~')` (`'@'`, `'A'-'Z'`, `[\]^_`, ``'`'``, `'a'-'z'`, `{|}~`)
    - Private use: `'\x70'('p')-'\x7e'('~')` (`'p'-'z'`, `{|}~`)

`P ... P` 的格式由其第一個 byte 決定:
- `'\x30'('0')-'\x3b'(';')` (`0123456789:;`): ECMA-48 parameter string
    - 一般 BBS 使用者較為熟悉的控制碼格式
    - 例如白底黑字的控制碼 (`SGR 47 30`):
        - `'\x1b' '\x5b' '\x34' '\x37' '\x3b' '\x33' '\x30' '\x6d'` (`ESC '[' '4' '7' ';' '3' '0' 'm'`)
            - `'\x34' '\x37' '\x3b' '\x33' '\x30'` (`'4' '7' ';' '3' '0'`) 是 parameter bytes (parameter string)
            - 沒有 intermediate bytes
            - `'\x6d'('m')` 是 final byte
- `'\x3c'('<')-'\x3f'('?')` (`<=>?`): Private/experimental use (格式與意義不由 ECMA-48 定義)
    - 例如 xterm 用以啟用 SGR mouse tracking 的控制碼 (`DECSET 1006`):
        - `'\x1b' '\x5b' '\x3f' '\x31' '\x30' '\x30' '\x36' '\x68'` (`ESC '[' '?' '1' '0' '0' '6' 'h'`)
            - `'\x3f' '\x31' '\x30' '\x30' '\x36'` (`'?' '1' '0' '0' '6'`) 是 parameter bytes
            - 沒有 intermediate bytes
            - `'\x68'('h')` 是 final byte

#### ECMA-48 parameter string 格式:
- 由 parameter sub-strings 組成
    - parameter sub-string:
        1. 是用十進位表示的數字
        2. 由 `'\x30'('0')-'\x3a'(':')` (`0123456789:`) 組成；`':'` 可被用作分隔符號，例如用作小數點
        3. 由 `'\x3b'(';')` 分開
        4. `'\x3c'('<')-'\x3f'('?')` (`<=>?`) 的使用，留待未來標準定義
        5. 空的 parameter sub-string 表示預設值
        6. `'\x30'('0')` 開頭不重要；全由 `'0'` 組成則代表 0
- 整個 parameter string:
    1. 以 `'\x3b'(';')` 開頭的話，視為在 `';'` 前有個空的 parameter sub-string；
       以 `';'` 結束的話，視為在 `';'` 後有個空的 parameter sub-string；
       含有連續的 `';'` 的話，視為每個 `';'` 之間都有一個空的 parameter sub-strings
    2. (譯自原文) `如果這個 control function 含有超過一個 parameter，並且某些 parameter sub-strings 是空的，分隔符號 (bit combination 03/11` (譯按：也就是 `'\x3b'(';')`)`) 還是得存在。`\
       `不過，如果最後幾個 parameter sub-string 都是空的，在這之前的分隔符號可被省略，見 B.2 in annex B.`
- 表示法:
    - `(NP)`: 沒有 parameter
    - `(Pn)`: 一個 numeric parameter (`numeric`: 數字的意義是純數值；允許任何數字)
    - `(Pn1;Pn2)`: 兩個 numeric parameters
    - `(Pn...)`: 任何數量的 numeric parameters
    - `(Ps)`: 一個 selective parameters (`selective`: 數字有特別意義；未指派意義的留待未來標準定義)
    - `(Ps1;Ps2)`: 兩個 selective parameters
    - `(Ps...)`: 任何數量的 selective parameters

#### Control sequence 的範例: (取自 ECMA-48 annex B.1)
- 使用 control function `CURSOR RIGHT (CUF; 游標右移)` 右移 1 格的控制碼:
    - (8-bit mode) `'\x9b' '\x31' '\x43'` (`CSI '1' 'C'`)
    - (7-bit mode) `'\x1b' '\x5b' '\x31' '\x43'` (`ESC '[' '1' 'C'`)
        - `'\x31'('1')` 是 parameter byte
        - 沒有 intermediate bytes
        - `'\x43'('C')` 是 final byte
    - 這與以下控制碼相等:
        - (8-bit mode) `'\x9b' '\x30' '\x31 '\x43'` (`CSI '0' '1' 'C'`)
        - (7-bit mode) `'\x1b' '\x5b' '\x30' '\x31' '\x43'` (`ESC '[' '0' '1' 'C'`)
            - 因為 `'\x30'('0')` 開頭不重要，多加也無妨
        - (8-bit mode) `'\x9b' '\x43'` (`CSI 'C'`)
        - (7-bit mode) `'\x1b' '\x5b' '\x43'` (`ESC '[' 'C'`)
            - 因為 `CUF` 參數的預設值定義為 `1`，可以省略

- 使用 control function `SCROLL RIGHT (SR; 右捲)` 右捲 28 格的控制碼:
    - (8-bit mode) `'\x9b' '\x32' '\x38' '\x20' '\x41'` (`CSI '2' '8' SPACE 'A'`)
    - (7-bit mode) `'\x1b' '\x5b' '\x32' '\x38' '\x20' '\x41'` (`ESC '[' '2' '8' SPACE 'A'`)
        - `'\x32' '\x38'` (`'2' '8'`) 是 parameter bytes
        - `'\x20'(SPACE)` 是 intermediate byte
        - `'\x41'('A')` 是 final byte

- 使用 control function `DEFINE AREA QUALIFICATION (DAQ; 定義區域屬性)` 允許將數字 (parameter sub-string `'3'`) 和字母 (parameter sub-string `'4'`) 輸入到目前游標所開始的區域的控制碼:
    - (8-bit mode) `'\x9b' '\x33' '\x3b' '\x34' '\x6f'` (`CSI '3' ';' '4' 'o'`)
    - (7-bit mode) `'\x1b' '\x5b' '\x33' '\x3b' '\x34' '\x6f'` (`ESC '[' '3' ';' '4' 'o'`)
        - `'\x33' '\x3b' '\x34'` (`'3' ';' '4'`) 是 parameter bytes
        - 沒有 intermediate bytes
        - `'\x6f'('o')` 是 final byte
        - 註: 使用 `'\x1b' '\x5b' '\x6f'` (`ESC '[' 'o'`) (將區域屬性設為預設) 來設定前面設下的區域的終點

#### Control sequence parameter string 的範例: (取自 ECMA-48 annex B.2)
Parameter String | 編碼 | 解釋
--- | --- | ---
`7` | `'\x37'` | 一個參數，值為 `7`
`98` | `'\x39' '\x38'` | 一個參數，值為 `98`
`4;2` | `'\x34' '\x3b' '\x38'` | 兩個參數，第一個值為 `4`，第二個值為 `2`
`=3` | `'\x3d' '\x33'` | Private parameter string
`6;` | `'\x36' '\x3b'` | 兩個參數，第一個值為 `6`，第二個為預設值 (`';'` 可省略)
`;5` | `'\x3b' '\x35'` | 兩個參數，第一個為預設值，第二個值為 `5`
`1;;4` | `'\x31' '\x3b' '\x3b' '\x34'` | 三個參數，第一個值為 `1`，第二個為預設值，第三個值為 `4`
`0007` | `'\x30' '\x30' '\x30' '\x37'` | 一個參數，值為 `7`

#### Control sequence 功能表:

- 沒有 intermediate bytes 時的功能表 (取自 ECMA-48 Table 3)

`F`	|	0x40	|	0x50	|	0x60	|	0x70
---:	|	---	|	---	|	---	|	---
0	|	`'@'` ICH 插字	|	`'P'` DCH 刪字	|	``'`'`` HPA 字定位	|	`'p'` Private Use
1	|	`'A'` CUU 游標上	|	`'Q'` SEE 選編輯範圍	|	`'a'` HPR 到下字	|	`'q'` Private Use
2	|	`'B'` CUD 游標下	|	`'R'` CPR 游標位報	|	`'b'` REP 重複	|	`'r'` Private Use
3	|	`'C'` CUF 游標右	|	`'S'` SU 捲上	|	`'c'` DA 裝置屬性	|	`'s'` Private Use
4	|	`'D'` CUB 游標左	|	`'T'` SD 捲下	|	`'d'` VPA 行定位	|	`'t'` Private Use
5	|	`'E'` CNL 游標下行首	|	`'U'` NP 到下頁裡	|	`'e'` VPR 到下行	|	`'u'` Private Use
6	|	`'F'` CPL 游標上行首	|	`'V'` PP 到上頁裡	|	`'f'` HVP 字行定位	|	`'v'` Private Use
7	|	`'G'` CHA 游標字定位	|	`'W'` CTC 游標縮排控	|	`'g'` TBC 縮排點清除	|	`'w'` Private Use
8	|	`'H'` CUP 游標定位	|	`'X'` ECH 清字	|	`'h'` SM 設模式	|	`'x'` Private Use
9	|	`'I'` CHT 游標進縮排	|	`'Y'` CVT 游標行縮排	|	`'i'` MC 媒體傳輸	|	`'y'` Private Use
a	|	`'J'` ED 清頁	|	`'Z'` CBT 游標退縮排	|	`'j'` HPB 到前字	|	`'z'` Private Use
b	|	`'K'` EL 清行	|	`'['` SRS 始逆向字串	|	`'k'` VPB 到上行	|	`'{'` Private Use
c	|	`'L'` IL 插行	|	`'\'` PTX 併行文	|	`'l'` RM 重設模式	|	`'\|'` Private Use
d	|	`'M'` DL 刪行	|	`']'` SDS 始徑向字串	|	`'m'` SGR 設繪字彩現	|	`'}'` Private Use
e	|	`'N'` EF 清欄位	|	`'^'` SIMD 設隱含移向	|	`'n'` DSR 裝置態報	|	`'~'`  Private Use
f	|	`'O'` EA 清區域	|	`'_'` -- 	|	`'o'` DAQ 定區域	|	`DEL` Private Use

註: 游標定位功能不受文字方向 (轉向 (`orientation`; 文字旋轉角度) 與徑向 (`character path`; 文字從左到右，還是從右到左)) 的影響，而行字定位功能會受到文字方向的影響

- Intermediate bytes 為 `'\x20'` (`SPACE`) 時的功能表 (取自 ECMA-48 Table 4)

`F`	|	0x40	|	0x50	|	0x60	|	0x70	
---:	|	---	|	---	|	---	|	---
0	|	`'@'` SL 捲左	|	`'P'` PPA 頁定位	|	``'`'`` TATE 縮排靠尾邊	|	`'p'` Private Use
1	|	`'A'` SR 捲右	|	`'Q'` PPR 到下頁	|	`'a'` TALE 縮排靠頭邊	|	`'q'` Private Use
2	|	`'B'` GSM 繪字大小改	|	`'R'` PPB 到前頁	|	`'b'` TAC 縮排靠中間	|	`'r'` Private Use
3	|	`'C'` GSS 繪字大小選	|	`'S'` SPD 設文體方向	|	`'c'` TCC 縮排靠字央	|	`'s'` Private Use
4	|	`'D'` FNT 選字型	|	`'T'` DTA 文區域大小	|	`'d'` TSR 縮排點移除	|	`'t'` Private Use
5	|	`'E'` TSS 細空格指定	|	`'U'` SLH 設行首	|	`'e'` SCO 設字轉向	|	`'u'` Private Use
6	|	`'F'` JFY 設左右靠	|	`'V'` SLL 設行尾	|	`'f'` SRCS 設減字隔間	|	`'v'` Private Use
7	|	`'G'` SPI 間距增加	|	`'W'` FNK 功能鍵	|	`'g'` SCS 設字間距	|	`'w'` Private Use
8	|	`'H'` QUAD 單行左右靠	|	`'X'` SPQR 設列印品質	|	`'h'` SLS 設行間距	|	`'x'` Private Use
9	|	`'I'` SSU 設大小單位	|	`'Y'` SEF 出紙換紙	|	`'i'` SPH 設頁首	|	`'y'` Private Use
a	|	`'J'` PFS 頁格式選擇	|	`'Z'` PEC 設文體寬縮	|	`'j'` SPL 設頁尾	|	`'z'` Private Use
b	|	`'K'` SHS 選字間距	|	`'['` SSW 設空格寬	|	`'k'` SCP 設字徑向	|	`'{'` Private Use
c	|	`'L'` SVS 選行間距	|	`'\'` SACS 設增字隔間	|	`'l'` --	|	`'\|'` Private Use
d	|	`'M'` IGS 認繪字字表	|	`']'` SAPV 設文體變體	|	`'m'` --	|	`'}'` Private Use
e	|	`'N'` --	|	`'^'` STAB 挑剔縮排	|	`'n'` --	|	`'~'` Private Use
f	|	`'O'` IDCS 認裝控字串	|	`'_'` GCC 繪字字組合	|	`'o'` -- 	|	`DEL` Private Use

- 目前 ECMA-48 沒有指派 intermediate bytes 為其它組合時的功能
- 標準中未被指派功能的字元組合 (上述列表中以 `--` 表示者，或者其 intermediate bytes 不為上述組合) 保留給未來標準，不應被私自使用

### d) independent control functions: `ESC Fs` / `ESC '\x23'('#') F`
- `ESC Fs` (表示法: `(Fs)`)
    - `ESC`: `'\x1b'`
    - `Fs`: ``'\x60'('`')-'x7e'('~')`` (``'`'``, `'a'-'z'`, `{|}~`)
    -  由 ISO 2375 標準標準化
- `ESC '\x23'('#') F` (沒有表示法；不包含在 ECMA-48 標準中，未來應該也不會)

#### `C1` 以及 `(Fs)` 的功能表 (整理自 ECMA-48 Table 2a, Table 2b, Table 5)

`Fe`/`Fs`	|	0x40 / 0x80 `(C1)`	|	0x50 / 0x90 `(C1)`	|	0x60 `(Fs)`	|	0x70 `(Fs)`
---:	|	---	|	---	|	---	|	---
0	|	`'@' `--	|	`'P'` DCS 裝置控字串	|	``'`'`` DMI 禁鍵盤輸入	|	`'p'` --
1	|	`'A'` --	|	`'Q'` PU1 Private Use	|	`'a'` INT 中斷	|	`'q'` --
2	|	`'B'` BPH 可在此換行	|	`'R'` PU2 Private Use	|	`'b'` EMI 准鍵盤輸入	|	`'r'` --
3	|	`'C'` NBH 別在此換行	|	`'S'` STS 設傳輸態	|	`'c'` RIS 重設初始態	|	`'s'` --
4	|	`'D'` --	|	`'T'` CCH 取消字	|	`'d'` CMD 編碼法分隔	|	`'t'` --
5	|	`'E'` NEL 下一行	|	`'U'` MW 訊息等待	|	`'e'` --	|	`'u'` --
6	|	`'F'` SSA 選擇區域始	|	`'V'` SPA 保護區域始	|	`'f'` --	|	`'v'` --
7	|	`'G'` ESA 選擇區域終	|	`'W'` EPA 保護區域終	|	`'g'` --	|	`'w'` --
8	|	`'H'` HTS 設字縮排點	|	`'X'` SOS 字串始	|	`'h'` --	|	`'x'` --
9	|	`'I'` HTJ 縮排改右靠	|	`'Y'` --	|	`'i'` --	|	`'y'` --
a	|	`'J'` VTS 設行縮排點	|	`'Z'` SCI 單字始	|	`'j'` --	|	`'z'` --
b	|	`'K'` PLD 到下一併行	|	`'['` CSI 控制序列始	|	`'k'` --	|	`'{'` --
c	|	`'L'` PLU 到上一併行	|	`'\'` ST 字串終	|	`'l'` --	|	`'\|'` LS3R 鎖移三右
d	|	`'M'` RI 逆換行	|	`']'` OSC 系統指令	|	`'m'` --	|	`'}'` LS2R 鎖移二右
e	|	`'N'` SS2 單移二	|	`'^'` PM 隱私訊息	|	`'n'` LS2 鎖移二	|	`'~'` LS1R 鎖移一右
f	|	`'O'` SS3 單移三	|	`'_'` APC 應用指令	|	`'o'` LS3 鎖移三	|	`DEL` -- 

- Independent control functions (`(Fs)`) 在標準中未被指派功能的字元組合，保留給未來標準，不應被私自使用

### e) control strings: `opening-delimiter command-string/character-string ST`
- `opening-delimiter`: 在 ECMA-48 中有定義的有: (C1 set) `APC`, `DCS`, `OSC`, `PM`, `SOS`
- `command-string`: 由 `'\x08'-'\x0d'` 和 `'\x20'-'\x7e'` 組成的 sequence (由實作決定格式)
- `character-string`: 不包含 `SOS`, `ST` 的任何 sequence (由實作決定格式)
- `ST`: (7-bit mode) `'\x1b' '\x5c'` (`ESC '\'`) / (8-bit mode) `'\x9c'`

## 使用注意

在 BBS 上，主要會使用到 `C0 set (C0)`, `C1 set (C1)`, `control sequences`, `independent control functions (Fs)` 四類 control functions。

其中
- `(C0)` 是單字元的控制碼，主要在撰寫程式時才會使用到，因為使用者無法輸入；
- `(C1)` 的 8-bit mode 版也是單字元的控制碼，但因為與 Big5-UAO 以及 UTF-8 衝突不能使用，現在的 terminal 通常也不支援；
- `(C1)` 的 7-bit mode 版與 `(Fs)` 兩種 control functions 都是形如 `ESC <ch>` 的控制碼；
- `control sequences` 則是 `ESC [` 開頭的控制碼，是 BBS 系統上最常用的控制碼。