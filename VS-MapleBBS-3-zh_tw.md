# [WIP] 與 MapleBBS 3 的差異

本文說明 DreamBBS 與其它 MapleBBS 3 分支的差異。

目前本文以 DreamBBS 的目前版本對於 DreamBBS 2010 既有功能的更改為主。

預計未來將再加入更多新增功能的說明以及與其它 MapleBBS 3 主要分支的比較。

🚧 尚缺少 3D.10.98 - 3D.12 間的資訊。

## 相關頁面

- 有關 xover 系統使用上的差異，請見 [[Xover 列表系統|Xover-zh_tw]]。

- 有關選單系統的差異，請見[[選單系統|Menu-zh_tw]]。

- 有關畫面座標系統的差異，請見[[畫面座標系統|Screen-Coord-zh_tw]]。

- 有關按鍵的輸入系統與對應功能的差異，請見[[與 MapleBBS 3 的按鍵差異|VS MapleBBS 3 Keyboard zh_tw]]。

## 既有功能與可啟用功能

### DreamBBS 3D.10.95:
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

### DreamBBS 3D.10.97:
- 移除 gopher 功能
- 移除已註解掉的 `pip_request()` 函式（將養雞遊戲貨幣轉為點歌次數）

### DreamBBS 3D.12.1-rc1:
- 移除未使用的 WindTop BBS 式我的最愛系統 (`Favorite`)

## 專案建置

### DreamBBS 3D.10.95:
- 取消對 SunOS/Solaris (`make sun/solaris/sol-x86`)、BSD < 4.4 (`make bsd`)、Cygwin (`make cygwin`) 平臺的正式支援
- 統一不同平臺的 `make` 指令
    - 在 makefile 中用 shell 指令判斷作業系統與處理器架構
    - 在原始碼中使用系統預定義的 macro 判斷作業系統 (參見：<https://sourceforge.net/p/predef/wiki/OperatingSystems/>)
- 改為從 `dreambbs.conf` 設定站臺參數，`include/config.h` 僅提供預設值
- Makefile 改為 NetBSD 格式，在其它平臺下需用 `bmake` 執行；`Makefile.bsd` 不再需要而移除

### DreamBBS 3D.10.97:
- 移除 GNU 格式的 Makefile (`Makefile.gnu`)

### DreamBBS 3D.12.1-rc1:
- 新增 CMake 手稿，功能涵蓋先前的 BSD Makefile 手稿

## 原始碼架構

### DreamBBS 3D.10.95:
- 修正註解錯字
- 將 `lib/` 中性質相近的零散原始碼檔案合併

### DreamBBS 3D.10.96:
- 移除 `acl.ic`, `bbsctrl.h`, `bbsnet.h`, `rpg.h`
- 移除過去的程式碼備份檔

### DreamBBS 3D.10.97:
- 將舊有說明文件移出程式碼分支。目前已收錄在本 wiki 中，以及本專案的 `wiki` 分支中。

### DreamBBS 3D.21.0:
- 將 `util/account.c` 中的看板分類區建立工作移至 `util/acpro.c`（WindTop BBS 用於建立專業討論區的程式）中。
- 將 `include/global.h` 中的 macro 定義獨立成 `include/global_def.h`

### DreamBBS 3D.21.1-rc.1
- 將 `maple/acct.c` 中未使用 `bbsd` 全域變數的函式移至 `lib/acct.c`，並與 `util/` 中的幾乎重複的函式合併。
- 將 `maple/banmail.c` 移至 `so/` 下。

## 資料結構名稱、欄位名稱、與定義

### DreamBBS 3D.10.95:

#### 錯字更正
- `include/struct.h`
    - struct `MailQueue`:
        - `niamod` -> `revdomain`
    - struct `BSTATCOUNT`:
        - `herfyear` -> `halfyear`

### DreamBBS 3D.12.1-rc1:

#### `pfterm` 相容
- `include/struct.h`
    - `typedef screenline screen_backup_t[T_LINES];` ->
        ```c
        typedef struct {
            int old_t_lines;
            int old_roll;
            screenline *slp;
        } screen_backup_t;
        ```

### DreamBBS 3D.21.0:

#### C++ 相容、類型安全、與命名慣例
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
    - struct `MENU`:
        - `void *func` -> `MenuItem item`

#### 64-bit 相容與型別安全
- `include/struct.h`:
    - struct `UCACHE`:
        - `uint32_t offset` -> `utmp_uidx32_t ubackidx`
            - The member now stores the array index instead of the byte offset
        - `BMW *mbase` -> `bmw_idx32_t mbase`
    - struct `BMW`:
        - `UTMP *caller` -> `utmp_idx32_t caller`
            - The special value `NULL` has been reassigned to `-1`
    - struct `UTMP`:
        - `UTMP *talker` -> `utmp_idx32_t talker`
            - The special value `NULL` has been reassigned to `-1`
        - `BMW *mslot[BMW_PER_USER]` -> `bmw_idx32_t mslot[BMW_PER_USER]`
            - The special value `NULL` has been reassigned to `-1`
        - `pipdata *pip` -> `pipdata_idx32_t pip` (from WindTop BBS; for `HAVE_PIP_FIGHT1`)
            - The special value `NULL` has been reassigned to `-1`

#### 程式碼改善
- `include/struct.h`:
    - ```c
      typedef struct
      {
          pipdata pip1;
          pipdata pip2;
      } PIPUTMP;
      ```
      -> `typedef pipdata PIPUTMP[2];`

## 全域函式名稱、參數定義、回傳值定義變更

### DreamBBS 3D.11.0:

#### 命名慣例
- `maple/visio.c`:
    - `ansi_move()` (from WindTopBBS) -> `move_ansi()`

### DreamBBS 3D.12.0:

#### 功能擴充
- `maple/edit.c`:
    - `char *tbf_ask(void)`\
      -> `char *tbf_ask(int n)`
    - `FILE *tbf_open(void)`\
      -> `FILE *tbf_open(int n)`
        - 新增參數 `n` 以直接指定暫存檔編號；傳入 `-1` 以詢問使用者暫存檔編號

### DreamBBS 3D.21.0:

#### 邊界檢查
- `lib/dns.c`:
    - `void dns_ident(int sock, const ip_addr *from, char *rhost, char *ruser)`\
      -> `void dns_ident(int sock, const ip_addr *from, char *rhost, int rhost_sz, char *ruser, int ruser_sz)`
        - 新增參數 `rhost_sz` 與 `ruser_sz` 以指定對應參數的緩衝區大小
    - `int dns_name(const ip_addr *addr, char *name)`\
      -> `int dns_name(const ip_addr *addr, char *name, int name_sz)`
        - 新增參數 `name_sz` 以指定 `name` 的緩衝區大小
- `maple/visio.c`:
    - `int iac_count(const unsigned char *current)` (deprecated)\
      -> `int iac_process(const unsigned char *current, const unsigned char *end, int *pcount)`
        - 新增參數 `end` 以指定緩衝區終點；新增參數 `pcount` 以回傳原回傳值；新回傳值改為按鍵值

#### 功能擴充與最佳化
- `maple/menu.c`:
    - `void domenu(void)`\
      -> `void domenu(MENU *menu, int y_ref, int x_ref, int height_ref, int width_ref, int cmdcur_max)`
    - `const char *check_info(const void *func)`\
      -> `const char *check_info(const void *func, const char *input)`
        - 新增參數 `input`
    - `count_len()` (from WindTopBBS) -> `strip_ansi_len()`
- `maple/xover.c`:
    - `void every_Z(void)`\
      -> `void every_Z(XO *xo)`
        - 新增參數 `xo`

#### 去混淆與命名慣例
- `lib/string.c`:
    - `str_add()` -> `str_pcpy()`
    - `str_cut()` -> `str_split_2nd()`
    - `str_cmp()` -> `str_casecmp()`
    - `str_decode()` -> `mmdecode_str()` (to `lib/mime.c`)
    - `str_folder()` -> `setdirpath_root()`
    - `str_from()` -> `from_parse()`
    - `str_len()` -> `str_len_nospace()`
    - `str_lowest()` -> `str_lower_dbcs()`
    - `str_ncmp()` -> `str_ncasecmp()`
    - `str_strip()` -> `str_rstrip_tail()`
    - `str_rev()` -> `str_rev_tail()`
    - `str_rle()` -> `rle_encode()`
    - `str_str()` -> `str_casestr()`
    - `str_sub()` -> `str_casestr_dbcs()`
    - `str_trim()` -> `str_rtrim()`
    - `void str_ncpy()` -> `ssize_t str_scpy()`
        - 回傳 `-1` 代表發生字串截斷，否則回傳複製的非 `\0` 位元組數量

#### 重複函式合併
- `lib/shm.c`:
    - `attach_shm()` (not-initializing version, from `util/*.c`) -> `attach_shm_noinit()`
    - `init_ushm()` (from `util/*.c`) -> `ushm_attach()`
    - `init_bshm()` (from `innbbsd/*.c` & `util/*.c`) -> `bshm_attach()`
    - `rewrite()` (from WindTopBBS `util/makefw.c`) -> `fwoshm_load()`

## 全域變數名稱

### DreamBBS 3D.10.95:

#### 錯字更正
- `include/global.h`:
    - `msg_seperator` -> `msg_separator`
    - `recommand_time` -> `recommend_time`

### DreamBBS 3D.21.0:

#### 用途變更與命名慣例
- `include/global.h`:
    - `ipv4addr` -> `ipv6addr`
    - `curcount` (from `maple/cache.c`) -> `countshm`
- `maple/popupmenu.c`:
    - `screenline sl` -> `screen_backup_t *popup_old_screen`

## 全域或用於控制程式功能的 macro 名稱

### DreamBBS 3D.10.95:

#### 錯字更正與不使用程式碼移除
- `maple/visio.c`:
    - `KICK_IDLE_TIMTOUT` -> `KICK_IDLE_TIMEOUT`
- `include/config.c`:
    - 移除 `HAVE_STUDENT`
    - 移除 `HAVE_ACTIVITY`
    - `HAVE_DETECT_VIOLAWATE` -> `HAVE_DETECT_VIOLATELAW`
    - `HIDEDN_SRC` -> `HIDDEN_SRC`
    - `RECOMMAND_TIME` -> `RECOMMEND_TIME`
    - `MSG_SEPERATOR` -> `MSG_SEPARATOR`
- `include/struct.h`:
    - `UFO_HIDEDN` -> `UFO_HIDDEN`

### DreamBBS 3D.21.0:

#### 去混淆與命名慣例
- `include/cppdef.h`:
    - `countof()` (from `include/bbs.h`) -> `COUNTOF()`
- `include/global_def.h`
    - `IS_ZHC_HI()` (from `include/config.h`) -> `IS_DBCS_HI()`
- `include/struct.h`:
    - `STRLEN` -> `STRSIZE` (alias)
    - `PASSLEN` -> `PASSSIZE` (alias)
    - `PLAINPASSLEN` -> `PLAINPASSSIZE` (alias)
    - `OLDPLAINPASSLEN` -> `OLDPLAINPASSSIZE` (alias)
    - `PASSHASHLEN` -> `PASSHASHSIZE` (alias)
    - `ANSILINELEN` -> `ANSILINESIZE` (alias)
    - `CH_TTLEN` -> `CH_TTSIZE` (alias)
- `include/nntp.h`:
    - `NNTP_STRLEN` -> `NNTP_STRSIZE` (alias)
- `include/nocem.h`:
    - `LINELEN` -> `LINESIZE` (alias)
- `maple/mail.c`:
    - `SIGNATURESIZE` -> `SIGNATURELEN` (alias)

#### 程式碼改善
- `include/theme.h`:
    - `FOOTER_VEDIT_BIFF` -> `FOOTER_VEDIT` (merged)
    
## 主程式流程
