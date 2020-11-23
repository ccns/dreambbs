# [WIP] 與 MapleBBS 3 的差異

本文說明 DreamBBS 與其它 MapleBBS 3 分支的差異。

目前本文以 DreamBBS 的目前版本對於 DreamBBS 2010 既有功能的更改為主。

預計未來將再加入更多新增功能的說明以及與其它 MapleBBS 3 主要分支的比較。

## 相關頁面
- 有關 xover 系統使用上的差異，請見 [[Xover 列表系統|Xover-List-System-zh_tw]]。

- 有關選單系統的差異，請見[[選單系統|Menu-Systems-zh_tw]]。

- 有關畫面座標系統的差異，請見[[畫面座標系統|Screen-Coordinate-System-zh_tw]]。

- 有關按鍵的輸入系統與對應功能的差異，請見[[與 MapleBBS 3 的按鍵差異|VS MapleBBS 3 Keyboard zh_tw]]。

## 既有功能與可啟用功能
DreamBBS 3.10.95:
- 移除已不再使用的 daemons
    - `bgopherd`
    - `bmtad`: 使用 `bbsmail`/`brdmail` 配合 `sendmail`/`postfix` 來替代
    - `bpop3d`
- 移除不再適用 DreamBBS 的 shell scripts，尤其是僅適用於 WindTopBBS 的
- 移除 WindTopBBS 的 Web BBS 相關程式 (`HAVE_WEBBBS`)
- 移除 Kimo News (奇摩新聞) 相關程式
- 移除 `so/` 中已不再使用或難以維護的工具或遊戲
    - `bbcall.c`, `bbsnet.c`, `bbsnet2.c`, `bwboard.c`, `chess.c`, `classtable.c`, `classtable2_verit.c`, `dictd.c`, `dreye.c`, `emailpage.c`, `fortune.c`, `grade.c`, `graduate.c`, `icq.c`, `imap4mail.c`, `km.c`, `lovepaper.c`, `netwhois.c`, `news_viewer.c`, `pop3mail.c`, `puzzle.c`, `pydict.c`, `qkmj.c`, `railway.c`, `sec_hand.c`, `seven.c`, `star.c`, `ueequery.c`, & `ueequery2.c`
- 移除 `util/` 中的過時維護工具
    - `2nd_expire.c` & `flowlog.c`
- 移除與帳號註冊不直接相關的個人資料，如生日、年齡等 (`HAVE_PERSON_DATA`)

## 專案建置
DreamBBS 3.10.95:
- 取消對 SunOS/Solaris (`make sun/solaris/sol-x86`)、BSD < 4.4 (`make bsd`)、Cygwin (`make cygwin`) 平臺的正式支援
- 統一不同平臺的 `make` 指令
    - 在 makefile 中用 shell 指令判斷作業系統與處理器架構
    - 在原始碼中使用系統預定義的 macro 判斷作業系統 (參見：<https://sourceforge.net/p/predef/wiki/OperatingSystems/>)
- 改為從 `dreambbs.conf` 設定站臺參數，`include/config.h` 僅提供預設值

## 原始碼架構
DreamBBS 3.10.95:
- Makefile 改為 NetBSD 格式，在其它平臺下需用 `bmake` 執行；`Makefile.bsd` 不再需要而移除
- 修正註解錯字
- 將 `lib/` 中性質相近的零散原始碼檔案合併

DreamBBS 3.20:
- 將 `include/global.h` 中的 macro 定義獨立成 `include/global_def.h`

## 資料結構名稱與欄位名稱
DreamBBS 3.10.95:
- `include/struct.h`
    - struct `MailQueue`:
        - `niamod` -> `revdomain`
    - struct `BSTATCOUNT`:
        - `herfyear` -> `halfyear`

DreamBBS 3.20:
- `maple/xover.c`:
    - struct `KeyMap`:
        - `key` -> `first`
        - `map` -> `second`
- `include/struct.h`:
    - struct `KeyFunc`:
        - `key` -> `first`
        - `func` -> `second.func`
        - `dlfunc` -> `second.dlfunc`
    - struct `ChatAction`:
        - `chinese` -> `brief_desc`

## 全域函式名稱
DreamBBS 3.20:
- `lib/string.c`:
    - `str_add` -> `str_cpy`
    - `str_cmp` -> `str_casecmp`
    - `str_cut` -> `str_split_2nd`
    - `str_decode` -> `mmdecode_text` (`lib/mime.c`)
    - `str_folder` -> `setdirpath_root`
    - `str_from` -> `str_getfrom`
    - `str_len` -> `str_len_nospace`
    - `str_lowest` -> `str_lower_dbcs`
    - `str_ncmp` -> `str_ncasecmp`
    - `str_strip` -> `str_rstrip_tail`
    - `str_rev` -> `str_rev_tail`
    - `str_rle` -> `str_rleencode`
    - `str_str` -> `str_casestr`
    - `str_sub` -> `str_casestr_dbcs`
    - `str_trim` -> `str_rtrim`

## 全域變數名稱
DreamBBS 3.10.95:
- `include/global.h`:
    - `msg_seperator` -> `msg_separator`
    - `recommand_time` -> `recommend_time`

## 全域或用於控制程式功能的 macro 名稱
DreamBBS 3.10.95:
- `maple/visio.c`:
    - `KICK_IDLE_TIMTOUT` -> `KICK_IDLE_TIMEOUT`
- `include/config.c`:
    - `HAVE_DETECT_VIOLAWATE` -> `HAVE_DETECT_VIOLATELAW`
    - `HIDEDN_SRC` -> `HIDDEN_SRC`
    - `RECOMMAND_TIME` -> `RECOMMEND_TIME`
    - `MSG_SEPERATOR` -> `MSG_SEPARATOR`
- `include/struct.h`:
    - `UFO_HIDEDN` -> `UFO_HIDDEN`

## 主程式流程