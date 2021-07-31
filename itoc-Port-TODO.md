# MapleBBS 3.10-itoc Change -> DreamBBS 3.21 Porting TODO

```c
/*-------------------------------------------------------*/
/* CHANGE       ( NTHU CS MapleBBS Ver 3.10 )            */
/*-------------------------------------------------------*/
/* target : difference from MapleBBS Ver 3.10            */
/* create : 00/02/01                                     */
/* update :   /  /  					 */
/* author : itoc.bbs@bbs.tnfsh.tn.edu.tw		 */
/*-------------------------------------------------------*/
```


 本 BBS 版本是由 NTHU CS MapleBBS Ver 3.10 改版而來。

 其中參考 bbs.cs.nthu.edu.tw `[plan]` 板 諸位大大的智慧結晶，

 以及 WindTop WD Ptt Firebird 等 BBS 程式。

 所有智慧財產均屬於原作者。



## 2000/02/
- [ ] `bbsd.c` 改成上站一分鐘以後才增加上站次數。
- [ ] `acct.c` `perm.h` 新增『註冊組長』權限  `PERM_REGISTRAR`，給審核註冊單用。
- [ ] `post.c` 文章作者可以 edit 自己的文章。

## 2000/02/13
- [ ] `post.c` 原作者可以按 <kbd>T</kbd> 修改自己的文章標題。
- [ ] `talk.c` 好友廣播出現廣播者的 id。為以示區別，水球出現 id，廣播出現 id >。

## 2000/02/19
- [ ] `acct.c` `menu.c` `maple.p` 加/減 `PERM_BM` 程式。

## 2000/02/20
- [ ] `gem.c` 新增小板主功能。

## 2000/02/25
- [ ] `talk.c` 新增使用者名單好友置前依ID排序，且設為預設值。

## 2000/02/26
- [ ] `gem.c` 精華區限制級文章名稱保密。

## 2000/02/27
- [ ] `post.c` 串接搜尋如果沒有輸入標題、作者，則搜尋游標所在文章標題。(已取消)
- [ ] `post.c` 新增串列搜尋下 <kbd>[</kbd> 的功能。

## 2000/02/28
- [ ] `gem.c` 精華區收錄方式從 link 變 copy，避免修改到板上的文章。

## 2000/03/03
- [ ] `post.c` 修正串接搜尋模式後 <kbd>h</kbd> 離開會有問題。

## 2000/03/04
- [ ] `xover.c` 閱讀文章時不循環閱讀。

## 2000/03/05
- [ ] `account.c` 上站人次記錄 年份 `% 100`。

## 2000/03/12
- [ ] `acct.c` `board.c` 板主可以修改中文板名敘述。
- [ ] `talk.c` 聊天室新增指令 <kbd>HOME</kbd>/<kbd>END</kbd>/<kbd>Ctrl</kbd>+<kbd>A</kbd>/<kbd>Ctrl</kbd>+<kbd>E</kbd> 移到行頭/行尾。

## 2000/03/15
- [ ] `config.h` `acct.c` `pop3_check.c` 新增 POP3 自動認證。

## 2000/03/17
- [ ] `post.c` 修正 `more` 完畢時不會出現 help。

## 2000/03/19
- [ ] `xover.c` `every_Z` 最多二層。
- [ ] `edit.c` 新增 文章編輯時 <kbd>Ctrl</kbd>+<kbd>W</kbd> 符號輸入工具。
- [ ] `xover.c` 修正限制級文章不得轉寄、匯入暫存檔、 Z-modem 下載。
- [ ] `post.c` 自己剛發表後的文章不要出現未閱讀的`＋`號。
- [ ] `bbsd.c` guest 亂數取暱稱。

## 2000/03/20
- [ ] `acct.c` `struch.h` User -> Setup 新增不使用簽名檔 `UFO_NOSIGN`。
- [ ] `acct.c` `struct.h` `edit.c` User -> Setup 新增顯示簽名檔 `UFO_SHOWSIGN`。
- [ ] `bbsd.c` 變動新使用者的預設旗標。

## 2000/03/25
- [ ] `global.h` `mail.c` 修改群組寄信與回水球程式衝突的問題。

## 2000/03/28
- [ ] `modes.h` 編系統檔案 & 編個人檔案 時候取消可以被 talk request。

## 2000/04/07
- [ ] `bbsd.c` `board.c` 0Announce 有新文章時強迫閱讀。
- [ ] `acct.c` 設定 簡化進/離站畫面 者，離站不顯示個人資料。
- [ ] `post.c` 發表文章超過 100 篇以後不提示文章發表綱領。
- [ ] `xover.c` 修正 `M_VOTE` `M_BMW` 動態錯誤。

## 2000/04/26
- [ ] `acct.c` 開板最後會設定 `expire.conf`。(已取消)

## 2000/05/07
- [ ] `sh/bakbbs/bak*` 新增備份 script。

## 2000/05/12
- [ ] `acct.c` 解決同時砍除同一個看板會造成精華區、看板全毀的問題。
- [ ] `visio.c` 閒置過久給予警告。
- [ ] `mail.c` `menu.c` `admutil.c` 寄信給全部板主/使用者。

## 2000/05/13
- [ ] `mail.c` 信讀到一半可以 reply mark delete forward。
- [ ] `more.c` 文章/信閱讀到一半的時候按 <kbd>q</kbd> 可以直接離開。
- [ ] `more.c` 改變一些按鍵定義。

## 2000/05/15
- [ ] `post.c` `mail.c` `post_browse` `mbox_browse` 時可存入暫存檔、看 help。

## 2000/05/16
- [ ] `acct.c` 斷線時註冊單修復。

## 2000/05/27
- [ ] `bbsd.c` 避免有站長丟水球給 guest。

## 2000/09/30
- [ ] `xover.c` 修正 `every_Z` 後板主權限錯誤。

## 2000/10/29
- [ ] `mail.c` 站長可以讀取使用者信箱。

## 2000/11/01
- [ ] `perm.h` `*.c` 把站務的權限畫清。

## 2000/11/02
- [ ] `bbsd.c` 以 sysop 登入可以變更使用者身分。

## 2000/11/09
- [ ] `menu.c` `post.c` 改變 `PERM_ALLBOARD` 在精華區的權限。
- [ ] `gem.c` 把精華區收錄提示字樣改成中文。

## 2000/11/18
- [ ] `post.c` 新增 `XoXsearch()` 搜尋相同標題文章。
- [ ] `bbsd.c` `config.h` `HAVE_WHERE` 修正故鄉判定。
- [ ] `acct.c` `menu.c` 如果沒有 define `HAVE_ALOHA`，就不用 `UFO_ALOHA`。

## 2000/11/19
- [ ] `acct.c` 移除 `etc/register`。

## 2000/11/25
- [ ] `mail.c` 改寫群組回信的檢查。

## 2000/11/26
- [ ] `maple.p` `Makefile` `acct.c` `talk.c` `bbsd.c` `xover.c` `aloha.c` `camera.c` `rec_loc.c`
- [ ] `global.h` `modes.h` `struct.h` `@aloha.hlp` 新的 `HAVE_ALOHA`。
- [ ] `talk.c` 新增水球記錄區段刪除。

## 2000/12/02
- [ ] `maple.p` `Makefile` `favorite.c` `bbsd.c` `board.c` `menu.c` `xover.c` `camera.c` 
- [ ] `config.h` `global.h` `modes.h` `struct.h` `@mf.hlp` 新增 `MY_FAVORITE`。
- [ ] `aloha.c` 新增好友名單排序、查詢功能。

## 2000/12/03
- [ ] `outgo.c` 自動送信程式。
- [ ] `camera.c` 移除 `FILM_GOODBYE`。

## 2000/12/17
- [ ] `perm.h` 修改 `HAS_PERM(x)` `HAVE_PERM(x)`。
- [ ] `*.c` `HAVE_PERM(x)` 及 `cuser.level & x` 換成 `HAS_PERM()`。

## 2000/12/20
- [ ] `config.h` 把 `SYSOPNICK` 獨立出來。
- [ ] `post.c` 新增 `post_cross_terminator()` 刪除相同作者。
- [ ] `xover.c` 改變一些按鍵 `keymap`。
- [ ] `post.c` 新增 `XoX` 系列的搜尋，使與 其他版本 BBS 相同。

## 2000/12/21
- [ ] `talk.c` Talk 時可以方向鍵移動、<kbd>Ctrl</kbd>+<kbd>Y</kbd> 清除之類的功能鍵。
- [ ] `config.h` `talk.c` 移除 `TALK_USER_LIST`。

## 2000/12/22
- [ ] `talk.c` 使用者名單加好友時檢查好友個數是否超過上限。
- [ ] `global.h` 把許多 message define 在這，並將 COLOR 獨立出來。

## 2000/12/23
- [ ] `talk.c` `cache.c` `bbsd.c` 把壞人也載入 `CACHE`。

## 2000/12/24
- [ ] `config.h` `global.h` `talk.c` `ADV_ULIST` 使用者名單中，不同的好友會區分顏色。
- [ ] `aloha.c` 引入好友至多只引到 `ALOHA_MAX` 的個數

## 2001/01/01
- [ ] `acct.c` 修正 POP3 認證時不會儲存使用者信箱。
- [ ] `talk.c` 使用者名單中 `UFO_PAL` 不出現與我為友，以免好友廣播錯誤。

## 2001/01/02
- [ ] `acct.c` 系統重置。

## 2001/01/06
- [ ] `dns_aton.c` `dns_open.c` `dns_smtp.c` `bmtad.c` 支援 mail-abuse。
- [ ] `config.h` `global.h` `acct.c` untrust.acl trust.acl `HAVE_TRUST` 認證黑白名單。
- [ ] `acct.c` `camera.c` `struct.h` 把 一些常用的文件抓去 `FILM_XX`。
- [ ] `talk.c` 加長水球長度。
- [ ] `talk.c` `@ulist.hlp` 使用者名單中按 <kbd>d</kbd> 刪除好友/壞人。

## 2001/01/08
- [ ] `config.h` `global.h` `struct.h` `modes.h` `camera.c` `menu.c` `newbrd.c`
    提供看板連署功能。
- [ ] `talk.c` 記錄聊天開始時間。

## 2001/01/10
- [ ] 把 `src/maple` 中的 `so` 搬去 `src/so`。
- [ ] `admutil.c` `menu.c` `acct.c` `maple.p` 
    把站長指令 `m_xfile` `m_resetsys` `m_reg_merge` 外掛。
- [ ] `favorite.c` `@mf.hlp` 我的最愛中可以修改看板。

## 2001/01/11
- [ ] `*.c` 把 `xxx_head` 內的提示字樣格式改成相同。
- [ ] `config.h` `mail.c` `acct.c` 使 undef `EMAIL_JUSTIFY` 有效。

## 2001/01/12
- [ ] `config.h` `global.h` `acct.c` `mail.c` `menu.c` `maple.p` `bmtad.c`
- [ ] `@justify` `@e-mail` `valid` `HAVE_REGKEY_CHECK` 認證碼驗證。
- [ ] `bbsd.c` 使故鄉排序正常。

## 2001/01/13
- [ ] `config.h` `menu.c` 提供選單光棒。

## 2001/01/15
- [ ] `xover.c` `menu.c` `Ctrl('U')` 跳到使用者名單。

## 2001/01/16
- [ ] 把 `COLOR3` 獨立出來。

## 2001/01/17
- [ ] `mail.c` 文章、郵件日期上色。

## 2001/01/27
- [ ] `newbrd.c` `struct.h` 匿名連署機制。
- [ ] `visio.c` `menu.c` 修正 `vmsg(NULL)` `pad_view()` 
    在偵測左右鍵全形下，按左鍵會跳離二層選單的問題。
- [ ] `talk.c` 對方即使沒開「好友上站通知」也通知。
- [ ] `talk.c` 自己是對方的好友且對方沒有「遠離塵囂」才可以回扣上站通知。
- [ ] 更新 `bpop3d.c`。

## 2001/01/28
- [ ] `bbsd.c` `admutil.c` 故鄉比對可以比對 FQDN。
- [ ] `board.c` `@class.hlp` 看板列表按 <kbd>v</kbd> 設定看板已讀。

## 2001/01/29
- [ ] `bmtad.c` 以 `indent` 排版。
- [ ] `board.c` `@class.hlp` 看板列表按 <kbd>V</kbd> 設定看板未讀。

## 2001/02/05
- [ ] `edit.c` `post.c` `bmtad.c` `config.h` 變換 Origin 樣式。
- [ ] `post.c` 限制文章轉錄篇數。
- [ ] `post.c` 發表後的文章就記錄，使不出現未閱讀的`＋`號。
- [ ] `config.h` `struct.h` `global.h` `menu.c` `xover.c` `song.c` `camera.c` 
- [ ] `@song.hlp` 提供點歌功能。

## 2001/02/07
- [ ] `camera.c` 提供點歌到動態看板功能。
- [ ] `global.h` `*.c` 把系統板名獨立出來 define `BN_XXXX`。
- [ ] `bwboard.c` `guessnum.c` 以 `indent` 排版。
- [ ] `post.c` `board.c` 文章列表部分 visit。
- [ ] `bmtad.c` `mailpost` 時檢查使用者權限是否滿足看板閱讀/發表權限。

## 2001/02/08
- [ ] `bbsd.c` `talk.c` guest 亂數取故鄉。
- [ ] `modes.h` `menu.c` 動態重排。
- [ ] `km.c` `menu.c` 提供孔明棋遊戲。

## 2001/02/09
- [ ] `menu.c` 重新整理選單。
- [ ] `menu.c` 主選單按 <kbd>s</kbd> 直接進入 `Select()`，減少選單長度。

## 2001/02/10
- [ ] `dns_smtp.c` 用 relay server。

## 2001/02/11
- [ ] `acct.c` `bbsd.c` 自動取下離職板主權限。
- [ ] `acct.c` 新看板預設發表權限為發表文章，預設屬性為不轉信。

## 2001/02/12
- [ ] `src/Makefile` 快速安裝用的 `Makefile`。
- [ ] `acct.c` 開新板/修改看板自動加上板主權限。

## 2001/02/15
- [ ] `mine.c` `menu.c` 提供踩地雷遊戲。

## 2001/02/17
- [ ] `mail.c` 文章、信件列表日期改用星期幾上色。
- [ ] `talk.c` Talk 時有 insert 功能。

## 2001/02/18
- [ ] `gem.c` 精華區異動記錄格式變動。

## 2001/02/20
- [ ] `fantan.c` `menu.c` 提供接龍遊戲。

## 2001/02/21
- [ ] `favorite.c` 限制我的最愛只能刪除空目錄，以免留下檔案殘骸。
- [ ] `global.h` `edit.c` 把 origin banner 的 logo 獨立出來。

## 2001/02/22
- [ ] `gem-check.c` 避免精華區 Class下放 Folder 被 expire 掉。
- [ ] `transacct.c` `.ACCT` 轉換程式。

## 2001/02/23
- [ ] `maple/Makefile` 不用 `make clean` 直接對所有的 `.o` 做連結。
- [ ] `bbsd.c` 站務不做定期認證。
- [ ] `perm.h` `*.c` 把站務權限做更細步的重整。
- [ ] `*.hlp` 加入 `xover.c` 中的指令，並重新整理排版成相同格式。
- [ ] `post.c` `gem.c` 新增使用者習慣的 c/p 收錄精華區。

## 2001/02/24
- [ ] `gem-check.c` 無條件刪除沒用的檔案。
- [ ] `account.c` 把一些文件放在 `[BN_SECURITY]`。
- [ ] `usies-sort.c` `account.c` 統計低使用率的看板到 `[BN_SECURITY]`。
- [ ] `acct.c` `account.c` 記錄站長修改權限到 `[BN_SECURITY]`。

## 2001/02/25
- [ ] `mine.c` 修正小錯誤。
- [ ] `talk.c` 新增第三版 `talk_char()`。

## 2001/02/26
- [ ] `board.c` `favorite.c` 修正中文板名太長時，某些 term 看起來會跳行的問題。

## 2001/02/28
- [ ] `config.h` `menu.c` `mail.c` `maple.p` `HAVE_MAIL_ZIP` 
- [ ] 提供把信件/精華區壓縮轉寄的功能。

## 2001/03/01
- [ ] `xyz.c` `menu.c` `etc/tip` 每日小秘訣。
- [ ] `post.c` `edit.c` 提供編輯(但不能儲存)其他人發表文章的功能。
- [ ] `mail.c` 提供編輯(但不能儲存)信箱中信件的功能。
- [ ] `fantan.c` 修正破關時不會結束的問題。

## 2001/03/02
- [ ] `inst.sh` 自動安裝 script。
- [ ] `util/uno/Makefile` `util/tran/Makefile` 方便 `make`。
- [ ] `config.h` `talk.c` `HAVE_BADPAL` 可以拿掉整個壞人功能。

## 2001/03/03
- [ ] `bbsd.c` `currtitle` 還原。
- [ ] `more.c` `@more.hlp` 新增上捲一行。

## 2001/03/04
- [ ] `newbrd.c` `@cosign.hlp` 把連署界面做得更像閱讀文章一樣。
- [ ] `global.h` `*.c` 整理所有的 footer。
- [ ] `talk.c` 增長水球記錄。
- [ ] `menu.c` 進出子選單都重撥動態看板。
- [ ] `menu.c` 新增我的最愛快速鍵。
- [ ] `talk.c` 在水球回顧中，讓傳訊/收訊/廣播的水球都可以回。

## 2001/03/06
- [ ] `config.h` `mail.c` `CHECK_ONLINE` 文章列表中可以顯示使用者是否在站上。

## 2001/03/07
- [ ] `xyz.c` `menu.c` 提供 BBSNET 服務。

## 2001/03/09
- [ ] `menu.c` 不必離站可以寫留言板。
- [ ] `menu.c` 留言板提供不同的顏色及頁數。
- [ ] `acct.c` 把 User/Setup 的說明寫明白一點。
- [ ] `acct.c` 輸入 id 時提示可按 <kbd>space</kbd> 自動搜尋。
- [ ] `menu.c` 選單敘述編排。
- [ ] `struct.h` `bbsd.c` 把 userlevel 也放入 cache。
- [ ] `talk.c` 利用 cache 的 userlevel 改善檢查速度。

## 2001/03/10
- [ ] `mail.c` 修正群組寄信結束時`請按任意鍵繼續`會怪怪的。

## 2001/03/11
- [ ] `global.h` `menu.c` `acct.c` `visio.c` 抓出 `MENU_XYPOS` 定義，以便修改介面。
- [ ] `global.h` `acct.c` `menu.c` 抓出 `MENU_YNOTE` 定義，以便修改介面。
- [ ] `cache.c` `camera.c` 抓出 `MOVIE_LINES` 定義，以便修改介面。
- [ ] `post.c` `mbox.c` `@board.hlp` `@mbox.hlp` 統一按鍵。
- [ ] `'x'` 轉給 user  `'X'` 轉到 board  <kbd>^X</kbd> 拂楓落葉

## 2001/03/12
- [ ] `config.h` `modes.h` `talk.c` `visio.c` `BMW_COUNT` 計算中水球的個數。
- [ ] `visio.c` `acct.c` 修正 `vget()` 游標移動顯示的問題。
- [ ] `fantan.c` `guessnum.c` `mine.c` 取消一些沒用的訊息。
- [ ] `config.h` `*.c` 把一些參數整理一下，搬去他們歸屬的程式。

## 2001/03/13
- [ ] `config.h` `talk.c` `BMW_DISPLAY` 顯示前幾個水球。
- [ ] `acct.c` 加強版的螢幕鎖定功能，讓鎖定時不能接/傳水球，且清除螢幕。

## 2001/03/15
- [ ] `struct.h` `acct.c` `talk.c` 加入自定 <kbd>Ctrl</kbd>+<kbd>R</kbd> 是否要回顧水球的選項。

## 2001/03/16
- [ ] `*.c` 統一問題格式為 `"你要嗎(Y/N)？[N] "`  全形問號  大寫 `Y/N`  預設用 `[]` 。
                                   ^ 無空白  ^空白

## 2001/03/17
- [ ] `acct.c` `bbsd.c` 不讓 user 修改姓名欄位。
- [ ] `util/backup/*.c` 自動備份程式。

## 2001/03/19
- [ ] `*.c` `*.h` 把所有的 define 分類整理到 `config.h`。
- [ ] `config.h` `modes.h` `bbsd.c` `mail.c` `xover.c` `mboxgem.c` `reaper.c`
- [ ] `HAVE_MBOX_GEM` 提供個人信箱精華區。

## 2001/03/21
- [ ] `xchatd.c` MUD-like 的指令部分 match 就算。
- [ ] `xchatd.c` MUD-like 指令按字母排序。

## 2001/03/22
- [ ] `chat.c` 進入聊天室時預設改為不聊天。
- [ ] `xover.c` 把不常用的鍵拿掉，釋出給其他程式使用。
- [ ] `*.c` `@*.hlp` 按鍵重新統一。
- [ ] `edit.c` ANSI 編輯時按後退鍵回到非 ANSI 模式。

## 2001/03/24
- [ ] `post.c` 串列搜尋結果提示作者/主題，並解決提示中文字會怪怪的問題。
- [ ] `mail.c` 解決信件瀏覽只把最後一篇設已讀的問題。
- [ ] `more.c` `KEY_UP` `KEY_PGUP` 對二頁以上的文章可離開並翻上一篇。
- [ ] `post.c` 串列搜尋模式中時按上下鍵也可以循序閱讀。

## 2001/03/25
- [ ] `bmtad.c` mail post 檢查是否有發表的權限。
- [ ] `post.c` `@board.hlp` 搜尋所有 mark 的文章。
- [ ] `talk.c` 調整使用者名單的動態長度。

## 2001/03/27
- [ ] `post.c` 提供板主在板內修改中文板名。
- [ ] `xover.c` 修正在精華區不可轉寄。

## 2001/03/28
- [ ] `post.c` `cache.c` `@board.hlp` 在文章列表可以丟線上作者水球。
- [ ] `config.h` `acct.c` `more.c` `post.c` `xover.c` 抓出 `HAVE_NOFORWARD`。
- [ ] `*.c` 把看版、數主都改成「板」，統一用字。 :p
- [ ] `*.c` 讓 `xxx_cb` 中的 dynamic load 不會有 warning。

## 2001/03/29
- [ ] `mode.h` 重新整理。

## 2001/03/30
- [ ] `bmw.c` `board.c` `pal.c` `post.c` `talk.c` `maple.p`
    把 `talk.c` 裡面的程式分散到各適當的地方。
- [ ] `util/tran/*.c` 提供 sob Ptt WD FireBird 轉 maple 3 的程式。

## 2001/04/02
- [ ] `favor.c` 我的最愛中可以設定看板已讀/未讀。
- [ ] `board.c` `favor.c` `mf.hlp` 看板列表中設定看板未讀後，未讀的燈會亮起來。

## 2001/04/03
- [ ] `bmw.c` 修復水球區塊刪除的功能。
- [ ] `xover.c` `*.c` `global.h` 在最後一行提示指令。
- [ ] `song.c` 歌本中讓一般使用者也可以 edit 看控制碼。

## 2001/04/07
- [ ] `config.h` `board.c` `favor.c` `ENHANCED_VISIT` 已讀/未讀改為判斷最後一篇文章。
- [ ] `menu.c` 修正 guest 可以進入我的最愛的 bug。
- [ ] `cache.c` `edit.c` `more.c` `visio.c` 增加 `SHOW_USER_IN_TEXT` 的種類。

## 2001/04/08
- [ ] `acct.c` `talk.c` 新增金錢、生日、性別欄位。
- [ ] `post.c` `edit.c` 發表文章加錢。
- [ ] `mail.c` 在信件列表可以丟線上作者水球。

## 2001/04/13
- [ ] `mboxgem.c` 小 patch。
- [ ] `@board.hlp` `@mbox.hlp` 加強。
- [ ] `struch.h` `@class.hlp` `board.c` `acct.c` `account.c` 看板列表可選擇依字母/分類排序。

## 2001/04/14
- [ ] `vote.c` `menu.c` `global.h` 新增投票中心的功能。

## 2001/04/15
- [ ] `account.c` 新增同一 email 認證過多的記錄。
- [ ] `bbsd.c` 生日當天上站有特別的進站畫面。

## 2001/04/19
- [ ] `@gem.hlp` `gem.c` `mboxgem.c` 加入精華區新增文章/卷宗的快速鍵。

## 2001/04/20
- [ ] `*.c` 把一些 `vget()` 不需要 `GCARRY` 的換成 `DOECHO`。
- [ ] `dice.c` `menu.c` 新增擲骰子遊戲。

## 2001/04/21
- [ ] `gp.c` `menu.c` 新增金撲克梭哈遊戲。
- [ ] `race.c` `menu.c` 新增賽馬場遊戲。
- [ ] `bingo.c` `menu.c` 新增賓果遊戲。
- [ ] `bmw.c` `cache.c` 解決如果 post 時中水球，不會顯示來源的問題。

## 2001/04/23
- [ ] `bj.c` `menu.c` 新增黑傑克二十一點遊戲。
- [ ] `admutil.c` `acct.c` `menu.c` `maplep.p` 把註冊單認證的程式搬去 dynamic load。

## 2001/04/24
- [ ] `nine.c` `menu.c` `etc/game/99` 新增天地九九遊戲。

## 2001/04/25
- [ ] `chessmj.c` `menu.c` 新增象棋麻將遊戲。

## 2001/04/26
- [ ] `util/tran/wd2brd.c` 提供轉換看板屬性。
- [ ] `seven.c` `menu.c` 新增賭城七張遊戲。
- [ ] `marie.c` `etc/game/marie` `menu.c` 新增小瑪麗樂園遊戲。

## 2001/04/27
- [ ] `bar.c` `menu.c` 新增吧台瑪麗遊戲。

## 2001/05/02
- [ ] `km.c` 孔明棋改進，使會出現棋譜名稱。

## 2001/05/08
- [ ] `stock.c` `menu.c` `etc/game/stock.name` `etc/game/stock.now` `etc/game/stock.sh`
    新增股市大亨遊戲。

## 2001/05/09
- [ ] `km.c` 孔明棋改進，使能記錄完成的棋譜及悔棋。
- [ ] `etc/game/km` 輸入大量孔明棋譜。

## 2001/05/25
- [ ] `visio.c` 「請按任意鍵繼續」跑馬燈。

## 2001/05/28
- [ ] `chat.c` 不可以用別人的 id 當做聊天室代號。

## 2001/05/29
- [ ] `struct.h` 增加 `BMW.userid` 的長度，給廣播符號用。

## 2001/06/02
- [ ] `menu.c` 改掉一些重覆關鍵字的選項。
- [ ] `config.h` `post.c` `xover.c` `@board.hlp` `HAVE_REFUSEMARK` 提供看板文章加密功能。
- [ ] `xover.c` 修正已刪除文章不得轉寄、匯入暫存檔、 Z-modem 下載。
- [ ] `bmtad.c` `mailpost.c` 解決不能 Email Justufy 認證。

## 2001/06/06
- [ ] `bpop3d.c` 修正某些 client 取信會卡死的問題。
- [ ] `dl_lib.c` 解決外掛太多`.so` 爆掉了的問題。

## 2001/06/09
- [ ] `railway.c` 輸入車站名時改用選號碼的。

## 2001/06/16
- [ ] `menu.c` 解決動態看板 12 行，又使用選單光棒時，畫面會有殘骸的問題。

## 2001/06/19
- [ ] `wd2usr.c` 解決 `.PASSWD` 常有空白欄位的問題。

## 2001/06/29
- [ ] `more.c` 將引言分顏色。

## 2001/07/09
- [ ] `acct.c` POP3 REGKEY 認證只能有 `PERM_VALID`，在下次 login 時自動加上其他權限，
    以免被停權者藉此方法復權，也不會與 `NEWUSER_LIMIT` 衝突。

## 2001/07/09
- [ ] `post.c` 修改文章標題時順便修改內文的標題。

## 2001/07/10
- [ ] `chip.c` 玩遊戲不能 `multi_login`。
- [ ] `backupacct.c` 備份全站使用者 `.ACCT`。
- [ ] `menu.c` 修正 system load 查詢。

## 2001/07/11
- [ ] `board.c` 看板依名稱/中文板名切換時應儲存閱讀記錄。
- [ ] `topusr.c` 可統計上站次數/灌水次數/銀幣/金幣/年齡/星座。
- [ ] `mine.c` 修正玩踩地雷隨便亂標記，當標記數=地雷數就說你過關了。
- [ ] `post.c` 刪除文章要扣錢。

## 2001/07/12
- [ ] `chip.c` `marie.c` `etc/game/marie` 修改合理的賠率。
- [ ] `post.c` 改用字數及花費時間來計算稿費。

## 2001/07/14
- [ ] `dice.c` 修改合理的賠率。
- [ ] `gp.c` 修改使能作弊來降低勝率。
- [ ] `.BRD` `brd/*` `gem/brd/*` `run/class*.img` 把預設看板建立起來。
- [ ] `gem/@` 把 `(A)nnounce` 選單建立起來。

## 2001/07/16
- [ ] `showUSR.c` `showACCT.c` 顯示 `.USR` `.ACCT` 用程式。
- [ ] `setusr.c` `setperm.c` 外部設定使用者資料。
- [ ] `menu.c` `bank.c` 加入銀行，提供轉帳、匯兌功能。
- [ ] `config.h` `struct.h` `acct.c` `bbsd.c` `HAVE_NOALOHA` 提供上站不通知好友。
- [ ] `config.h` `struct.h` `acct.c` `talk.c` `HAVE_NOBROAD` 提供拒收廣播。
- [ ] `bbsd.c` 保證人制度記錄保人。
- [ ] `stock.c` 預防股價 = 0 (公司倒閉)。

## 2001/07/17
- [ ] `post.c` 提供 XPOST 中可以編輯標題、文章，加密。
- [ ] `config.h` `menu.c` `bank.c` 提供購買權限。
- [ ] `menu.c` 調整 feeter 的長度。
- [ ] `config.h` `talk.c` `@ulist.hlp` `HAVE_CHANGE_ID` 使用者名單站長暫時修改ID。

## 2001/07/19
- [ ] `menu.c` `recall.c` 提供記憶遊戲。

## 2001/07/20
- [ ] `vote.c` 修正 `vote_all` 按 <kbd>End</kbd> 或 <kbd>$</kbd> 應檢查是否已到最後一項。

## 2001/07/23
- [ ] `xover.c` `post.c` `gem.c` 改寫 `gem_gather()`。
- [ ] `menu.c` `gray.c` 提供淺灰大戰遊戲。
- [ ] `gem.c` `mboxgem.c` 修正某些情況下不能 <kbd>Ctrl</kbd>+<kbd>P</kbd> `gem_add` 的問題。

## 2001/07/25
- [ ] `menu.c` `pip/*` `etc/game/pipgame` `etc/game/pipdata` 提供電子小雞遊戲。
- [ ] `post.c` 即使是原文轉載，也不要用 `HDR_LINK` 的方法，就視為是不同的文章。
- [ ] `gem.c` 所有收錄文章的動作都改成沒有 `HDR_LINK`，一篇文章就是一個檔案。

## 2001/07/26
- [ ] `gem.c` 由於一篇文章就是一個檔案，所以刪除時改成直接刪除檔案。
- [ ] `gem.c` 提供區段刪除功能。
- [ ] `gem.c` 支援跨區拷貝。
- [ ] `gem-check.c` 不再使用。
- [ ] `favor.c` 系統自動移除遭砍板的看板、看板捷徑。

## 2001/07/27
- [ ] `acct.c` 修改看板時，提供清除所有板主(板主欄留白)。
- [ ] `mboxgem.c` `config.h` `global.h` `modes.h` 取消不使用。
- [ ] `gem.c` 由於支援跨區拷貝，允許所有人收錄看板、複製精華區文章到他擔任(小)板主/個人信箱的地盤。
- [ ] `gem.c` 精華區貼上時檢查避免發生迴圈。
- [ ] `bbsd.c` `gem.c` 將個人信箱精華區和看板精華區整合。
- [ ] `fb2usr.c` `sob2usr.c` `wd2usr.c` 轉換程式增加個人精華區。

## 2001/07/29
- [ ] `dreye.c` 提供譯典通線上字典功能。

## 2001/08/02
- [ ] `etc/pipgame/*` `pip/*` 電子雞遊戲大改版。

## 2001/08/03
- [ ] `@goodbye` `struct.h` `camera.c` `menu.c` 離站時秀一張圖。

## 2001/08/04
- [ ] `acct.c` 復權功能。

## 2001/08/05
- [ ] `acct.c` 修正重新認證時權限不對的問題。
- [ ] `visio.c` 按任一鍵清除水球數。
- [ ] `newbrd.c` `acct.c` 新板連署完，站長可以直接開板。

## 2001/08/11
- [ ] `acct.c` `so/*.c` `pip/pip_basic.c` `post.c` 把加減錢回存硬碟的函式獨立成 `acct_savemoney()`。
- [ ] `cache.c` `acct.c` `perm.h` `bbsd.c` `admutil.c` `maple.p` 動態設定使用者資料。

## 2001/08/12
- [ ] `menu.c` `so/*.c` 把遊戲問籌碼的函式併入 `menu.c`。

## 2001/08/14
- [ ] `pip_stuff.c` 改變存檔格式。
- [ ] `pipstruct.h` `pip_fight.c` `pip_prac.c` 新增護身、輕功、心法、刀法等戰鬥技能。
- [ ] `pip_fight.c` 豐富怪獸產生器加入怪物屬性。

## 2001/08/15
- [ ] `mail.c` 提供把個人精華區壓縮寄回去的功能。

## 2001/08/16
- [ ] `xchatd.c` 談天室中 mud-like 指令敘述改寫。
- [ ] `pip/*` `etc/game/pip/*` 重新整理。

## 2001/08/20
- [ ] `pip.c` `pip_fight.c` `pipstruct.h` 新增特殊技能。
- [ ] `gem.c` `post.c` 收錄精華區不加上 `POST_GEM`。
- [ ] `*.c` `etc/*` 統一「身分」用字。
- [ ] `acct.c` `bmtad.c` `mailpost.c` 改寫認證部分程式。

## 2001/08/21
- [ ] `chessmj.c` 修正加錢錯誤。
- [ ] `global.h` `*.c` 整理分類 `FN_XXXX`。
- [ ] `so/*.c` 取消玩遊戲不需要重繪選單。
- [ ] `jcee.c` `etc/jcee/*` 大學聯考查榜。
- [ ] `favor.c` 修正看板英文名稱變動後，我的最愛會出錯；修正權限變動後，看到新權限不能看到的看板。
- [ ] `newbrd.c` `acct.c` 修正連署開板時不會把作者自動加上板主權限的問題。
- [ ] `global.h` `edit.c` `more.c` 改一些 FOOTER 使等長對齊。

## 2001/08/22
- [ ] `config.h` `mode.h` `global.h` `xover.c` `post.c` `HAVE_XYNEWS` 新聞閱讀模式。
- [ ] `post.c` 搜尋本地文章。

## 2001/08/24
- [ ] `pipstruct.h` `pip_fight.c` 新增技能暗器。
- [ ] `xyz.c` 改善 BBSNET 的界面。

## 2001/08/25
- [ ] `xyz.c` `etc/game/qkmj/*` 新增 QKMJ。
- [ ] `global.h` `bmtad.c` `mailpost.c` `mail.c` `acct.c` 認證碼認證不需要開檔。

## 2001/08/27
- [ ] `innbbsd/Makefile` 改寫。
- [ ] `gem-expire.c` 改寫。
- [ ] `visio.c` 新增請按特殊鍵繼續。

## 2001/08/29
- [ ] `talk.c` 改善使用者名單效率。
- [ ] `pal.c` 修正還沒到好友上限就警告好友過多的問題。

## 2001/08/30
- [ ] `gp.c` 修正特殊牌型輸錢的問題。

## 2001/08/31
- [ ] `acct.c` `bbsd.c` `menu.c` `post.c` `so/*.c` `pip_basic.c` `perm.h`
    利用 disable 第二隻以後的 multi-login 的錢幣來預防 multi 賺錢，
    如此便可以取消籌碼制度，並減少大量的回存 `.ACCT`。 

## 2001/09/01
- [ ] `struct.h` 修正 `MODE_STAT` 會錯亂的問題。
- [ ] `vote.c` `global.h` 投票區右鍵即可投票。

## 2001/09/02
- [ ] `admutil.c` `menu.c` 搜尋使用者。

## 2001/09/07
- [ ] `visio.c` 修正用 <kbd>Ctrl</kbd>-<kbd>R</kbd> 回水球不會重計水球數。
- [ ] `classtable.c` `menu.c` 提供功課表功能。
- [ ] `gray.c` 離開淺灰遊戲要重繪 menu。

## 2001/09/08
- [ ] `wd2pal.c` `sob2pal.c` 轉換好友名單程式。

## 2001/09/09
- [ ] `talk.c` 快速隱身。
- [ ] `brd2gem.c` 轉換看板分類到精華區。
- [ ] `board.c` `favor.c` `vote.c` 看板列表看板分類加顏色。
- [ ] `talk.c` `bmw.c` `pal.c` `struct.h` 修正鎖定回水球判斷不正確的問題。

## 2001/09/10
- [ ] `config.h` `board.c` `xover.c` `maple.p` 第一次進入文章列表，把游標放在第一篇未讀。
- [ ] `config.h` `board.c` 看板列表自動跳去下一個未讀看板。
- [ ] `menu.c` Select Favorite everywhere。

## 2001/09/11
- [ ] `edit.c` `@edit.hlp` `etc/model` 編輯文章時插入範本精靈。

## 2001/09/12
- [ ] `struct.h` `board.c` `favor.c` `acct.c` 恆為新文章模式。
- [ ] `talk.c` `menu.c` `maple.p` 隱身密法。

## 2001/09/13
- [ ] `board.c` `talk.c` `xover.c` 修正搜尋時不會清除最後一行的問題。

## 2001/09/15
- [ ] `struct.h` `acct.c` `bbsd.c` `talk.c` 把 realname 和 username 的長度抓出來 define。
- [ ] `wd2mf.c` 我的最愛轉換程式。

## 2001/09/17
- [ ] `gem.c` 修正精華區砍目錄誤砍的問題。

## 2001/09/22
- [ ] `talk.c` `maple.p` `menu.c` `cache.c` 加入選擇 Talk 功能。
- [ ] `acct.c` User/Info 換版面。
- [ ] `admutil.c` 特殊搜尋後可以修改該使用者。

## 2001/09/23
- [ ] `talk.c` 選單切換扣機。
- [ ] `edit.c` 修正 `SHOW_USER_IN_TEXT` 在重新編輯文章時的 bug。
- [ ] `wd2usr.c` 轉換 habit。
- [ ] `pal.c` 修正在看板好友名單中可以重覆加入。
- [ ] `config.h` `pal.c` `menu.c` `maple.p` 新增特別名單的功能。
- [ ] `pal.c` `aloha.c` `@pal.hlp` `@aloha.hlp` 在好友/上站名單中可以寄信/水球。
- [ ] `maple/*.c` `xyz.c` `modes.h` 統一字串為「水球」。

## 2001/09/24
- [ ] `bbsd.c` `fb2usr.c` `sob2usr.c` `wd2usr.c` 改變個人精華區架構來減少目錄個數。
- [ ] `modeh.h` 修正 mode 和 type 對應錯誤。
- [ ] `bbsd.c` `etc/mboxgem.over` 檢查個人精華區是否過大。

## 2001/09/24
- `gem.c` `post.c` `@gem.hlp` `@board.hlp` 改寫精華區：
    - [ ] 修正一般使用者在非板主看板精華區不能 `gem_copy()`。
    - [ ] 讓所有使用者在精華區可以編輯文章(不能儲存)。
    - [ ] 小板主可以看見自己管轄區中的限制級目錄名稱。
    - [ ] 修正跨區拷貝選附加檔案會失敗。
- [ ] 限制 `gem_gather()` 一定要先定錨，以免暫存個人精華區回收筒太多。
- [ ] 新增 `post_copy()` 收錄看板中文章到精華區。

## 2001/09/26
- [ ] `vote.c` 提供立即開票選項。
- [ ] `jcee.c` 修正只輸入姓名查詢時錯誤的問題。

## 2001/09/27
- [ ] `util/tran/*.c` 補上漏掉的 `closedir()`。
- [ ] `src/Makefile` `bin/install.sh` 改寫。
- [ ] `xchatd.c` 改善 Mud-like 指令搜尋。

## 2001/09/28
- [ ] `config.h` `global.h` `struct.h` `song.c` `topsong.c` `@-topsong`
- [ ] `LOG_SONG_USIES` 歌本使用統計。
- [ ] `global.h` `bbsd.c` `talk.c` `menu.c` 修正站上總使用者顯示不正確的問題。

## 2001/09/29
- [ ] `admutil.c` 補上漏掉的 `closedir()`。
- [ ] `board.c` `menu.c` `maple.p` 增加修改看板選項。

## 2001/10/01
- [ ] `topusr.c` `@-birthday` 統計本日壽星。

## 2001/10/02
- [ ] `reaper.c` `bquota.c` `crontab` (`reaper-vac.c` `bquota-vac.c`)
    把平時/暑假用的帳號/信件清除程式整合成一支，自動判斷日期。
- [ ] `talk.c` 修正 guest 可以藉由使用者名單進入 `XZ_BMW`。
- [ ] `talk.c` 修正使用者名單寄信重覆檢查權限。

## 2001/10/03
- [ ] `np2gem.c` Napoleon 轉換精華區程式。

## 2001/10/04
- [ ] `nine.c` 修正天地九九會自己幫玩家出牌的問題。
- [ ] `account.c` 重新整理編排一下。

## 2001/10/05
- [ ] `showACCT.c` `showBRD.c` 顯示 `.ACCT` `.BRD`。
- [ ] `acct.c` 修改 `acct_show()` 旗標顯示。
- [ ] `gem.c` 限制性文章不能被複製貼上。

## 2001/10/11
- [ ] `dice.c` 修正顯示倍率和加錢不合。
- [ ] `bbsd.c` 大小寫來源都能被成功判定故鄉。
- [ ] `chessmj.c` 修正加錢不對的問題。
- [ ] `bj.c` 修正可以無限次 double、電腦拿到 AA 牌時的判斷錯誤。

## 2001/10/12
- [ ] `etc/tip` 改一些不對的。
- [ ] `edit.c` 用 -- 分割簽名檔，不必要六行。

## 2001/10/13
- [ ] `song.c` 點歌最後一行加上 `<~Src~>` 想對 `<~Des~>` 說 `<~Say~>` (為WD的歌本設計)。

## 2001/10/18
- [ ] `gem.c` 精華區拷貝/刪除部分程式改善，修正待刪除卷宗下有資料、
    看板、限制級文章時會發生無窮迴圈的問題。

## 2001/10/19
- [ ] `backup*.c` 備份程式。

## 2001/10/22
- [ ] `bar.c` 修正錯誤。
- [ ] `struct.h` `bbsd.c` `talk.c` 使用者名單顯示壽星。

## 2001/10/23
- [ ] `pip_stuff.c` 修正收穫季比賽加錢不對。
- [ ] `gp.c` 若繼續壓注，給錢會不對。

## 2001/10/24
- [ ] `bj.c` 莊家顯示點數錯誤。
- [ ] `etc/game/pip/badman/pic000` `pip_fight.c` 戰鬥修行場景地圖。

## 2001/10/25
- [ ] `board.c` `@class.hlp` 依板主搜尋看板。
- [ ] `mail.c` 信箱文章拷貝。

## 2001/10/26
- [ ] `struct.h` `talk.c` `post.c` `transacct.c` 把 `ACCT` 中 `money/gold` 欄位改成 int。
- [ ] `post.c` 修正拂楓落葉斬功能。
- [ ] `src/util/backup/*` 備份/備份恢復程式。

## 2001/10/27
- [ ] `global.h` `bbsd.c` `menu.c` `talk.c` 站上總人數顯示修正。

## 2001/10/30
- [ ] `pip_prac.c` 練習時選取消一樣會加屬性。

## 2001/11/04
- [ ] `modes.h` `vote.c` `board.c` `maple.p` 提供好友看板屬性。
- [ ] `vote.c` 修正投票中心按 <kbd>PGDN</kbd> <kbd>PGUP</kbd> 會翻過頭的問題。

## 2001/11/05
- [ ] `camera.c` 允許動態看板分類中還有卷宗分類。

## 2001/11/10
- [ ] `aloha.c` 修正刪除上站通知名單時，若被刪除的使用者已遭 reaper 會進入無窮迴圈的問題。

## 2001/11/15
- [ ] `mail.c` `maple.p` `acct.c` `km.c` 把寄檔案的函式給獨立出來。
- [ ] `song.c` 讓沒有 `<~Src~>` 的歌本也能主動加入 `<~Src~>`。

## 2001/11/16
- [ ] `etc/re-reg` `global.h` `bbsd.c` 重新認證通知信。

## 2001/11/20
- [ ] `config.h` `global.h` `mail.c` `bmw.c` `menu.c` `talk.c` `xover.c` `chat.c`
- [ ] `topsong.c` `wd2mf.c` 修正一些 #undef 時的小錯誤。
- [ ] `acct.c` 站長可以放水加使用者認證權限。
- [ ] `showACCT.c` 改正錯誤的程式。

## 2001/11/23
- [ ] `bmw.c` 換水球記錄格式。
- [ ] `@bmw.hlp` `bmw.c` 水球列表中可選擇儲存，不必等再離站。
- [ ] `admutil.c` 站長全站使用者/板主寄信時，若選擇取消，檔案不會清除。

## 2001/11/24
- [ ] `talk.c` 使用者名單中不能丟水球給隱形的使用者。
- [ ] `talk.c` 修正使用者名單中看到隱形的使用者及不是好友的好友。

## 2001/11/25
- [ ] `post.c` 看板中修改中文板名後，要更新版面。
- [ ] `vote.c` 板主在沒有投票的看板舉行投票，但不輸入標題，會發生錯誤。

## 2001/11/26
- [ ] `talk.c` 修正廣播誤植的問題。

## 2001/12/08
- [ ] `acct.c` `newbrd.c` 改用較便利的看板權限設定。
- [ ] `bank.c` 檢查錢是否為負的。

## 2001/12/09
- [ ] `pip_stuff.c` 修正小雞備份儲存/讀取會失敗。
- [ ] `modes.h` 修正動態為查詢/談天時不會出現對象 id。
- [ ] `xover.c` guest 不得藉 `every_Z` 進入水球回顧。
- [ ] `xover.c` `menu.c` `talk.c` `bwboard.c` `chat.c`  修正
    進我的最愛/水球回顧後用 `every_Z` 跳去其他選單再出來會發生錯誤。

## 2001/12/10
- [ ] `pip_basic.c` 看 playboy 時罪孽是增加不是減少。

## 2001/12/17
- [ ] `acct.c` `board.c` `maple.p` `is_bm()` 函式整合在一起。
- [ ] `gem.c` 讓精華區中 `[userA/userB` 的多位小板主模式也適用。

## 2001/12/21
- [ ] `bmtad.c` ID 長度恰為 `IDLEN` 的使用者無法收來自Internet的信件。

## 2001/12/22
- [ ] `pipstruct.h` `pip_menu.c` `pip_visio.c` `pip_quest.c` `etc/game/pip/quest/` 電子雞任務。

## 2001/12/26
- [ ] `modes.h` `talk.c` `acct.c` 發呆可輸入理由。

## 2001/12/30
- [ ] `pip.h` `pip_fight.c` `pip_quest.c` `pip_item.c` 修正 bug 及增加任務。
- [ ] `post.c` `@board.hlp` `mail.c` `maple.p` 新增轉錄看板文章給使用者，並與 mbox 整合在一起。

## 2002/01/04
- [ ] `bmw.c` `global.h` 回水球時可用 <kbd>^R</kbd> <kbd>^T</kbd> 捲動。

## 2002/01/05
- [ ] `bmw.c` 讓回水球時，在上方的回顧有箭頭隨水球移動而動。
- [ ] `wd2brd.c` `wd2gem.c` `wd2usr.c` WD->M3 轉換程式可一次轉換單人或全站。

## 2002/01/07
- [ ] `util/uno/*.c` 加上 `chdir(BBSHOME);` 以免沒在 `BBSHOME` 下執行。

## 2002/01/08
- [ ] `global.h` 加入按鍵對應註解。

## 2002/01/13
- [ ] `xover.c` 修正若取消 `every_Z` 以後不能再進 `every_Z`，
- [ ] 以及用 `every_Z` 從 `XZ_` 回 menu 時最後一行未回復。
- [ ] `vote.c` 修改投票選項的時候會預設出現原先的選項。
- [ ] `maple.p` `mail.c` `post.c` 讓自己可以編輯自己信箱的信的標題及內容。
- [ ] `config.h` `post.c` `POST_PREFIX` 發表文章時選擇文章標題分類。

## 2002/01/14
- [ ] `post.c` guest 不能對其他 guest 的文章加密。

## 2002/01/17
- [ ] `struct.h` `vote.c` `pal.c` `maple.p` 提供限制名單投票。
- [ ] `acct.c` 刪除看板時順便刪除 `gem/@/@Class` 下的看板精華區捷徑。

## 2002/01/18
- [ ] `board.c` `Ben_Perm()` 修改。

## 2002/01/20
- [ ] `acct.c` POP3 認證時，加入主機名稱 `pop3.xxx` 的連線。

## 2002/01/22
- [ ] `newbrd.c` 在沒有連署時舉行連署，但取消，會發生錯誤。
- [ ] `wd2bmw.c` WD->M3 轉換水球記錄。
- [ ] `menu.c` 修正閱讀留言板時，頁碼顯示錯誤及若恰有 5n 篇時會多秀一頁空白。

## 2002/01/23
- [ ] `board.c` `favor.c` 不管有無 `UFO_BRDPOST`，一概更新看板狀態，以免未讀燈不會亮。
- [ ] `acct.c` XFile 分二欄，使可以編輯比較多檔案。
- [ ] `acct.c` `edit.c` 三個簽名檔檔案分開。

## 2002/01/24
- [ ] `xover.c` 在最後一頁按 `KEY_PGDN` 可翻到最前面。

## 2002/01/25
- [ ] `config.h` `bmtad.c` `HOST_ALIAS` 抓出來 define。
- [ ] `mail.c` 站內 internet mail 時會攔截寄信到本站的。

## 2002/01/26
- [ ] `config.h` `global.h` `credit.c` `menu.c` `HAVE_CREDIT` 提供記帳本功能。
- [ ] `mail.c` 調整信箱版面使和其他 `XZ_` 版面相同。
- [ ] `admutil.c` 全站/板主通告程式修正。
- [ ] `wd2pip.c` 轉換電子雞程式。
- [ ] `wd2list.c` 轉換特殊名單程式。
- [ ] `global.h` `bbsd.c` `maple.p` `menu.c` `bmw.c` 水球瀏覽界面。

## 2002/01/27
- [ ] `topgem.c` 不檢查秘密看板。
- [ ] `topusr.c` 版面修正。

## 2002/01/29
- [ ] `stock-open.c` `crontab` 取股市資料。
- [ ] `enews.c` `menu.c` `global.h` `struch.h` `enews-open.c` `run/kimo/*` `crontab` 提供電子報功能。

## 2002/01/30
- [ ] `mail.c` `post.c` `global.h` `@mbox.hlp` `@board.hlp` 統一在 `post/mbox` 中
    都是小寫 <kbd>x</kbd> 轉看板，大寫 <kbd>X</kbd> 轉使用者。
- [ ] `xover.c` 若無選 Tag 時，`AskTag` 不問問題。

## 2002/02/17
- [ ] `config.h` `bbsd.c` `mail.c` `OVERDUE_MAILDEL` 檢查過期信件。
- [ ] `railway.c` 號碼改為鐵路站的車站代碼。
- [ ] `enews.c` 修正超過一行的標題會跳行。
- [ ] `config.h` `pipstruct.h` `pip_menu.c` `pip_pk.c` `cache.c` `maple.p` 電子雞 PK 對戰。

## 2002/02/27
- [ ] `mail.c` 打包看板文章。

## 2002/03/01
- [ ] `visio.c` 讓 `ESC` + `*` + `s` 等控制碼在文章編輯/閱讀也適用。
- [ ] `config.h` `post.c` `board.c` `HAVE_BUCKET` 提供看板水桶功能。
- [ ] `bank.c` 轉帳也可以轉金幣。
- [ ] `menu.c` 主選單閱讀看板。

## 2002/03/02
- [ ] `board.c` 修正某些隱藏板會被看見的問題。

## 2002/03/05
- [ ] `post.c` 修正轉達文章回信箱，文章裡面暱稱錯誤。
- [ ] `xover.c` 避免 `every_U` 進入太多層以及修正某些斷線問題。
- [ ] `menu.c` `xover.c` 選單特殊鍵程式更新。

## 2002/03/07
- [ ] `config.h` `hdr.h` `@board.hlp` `post.c` `HAVE_LABELMARK` 看板文章加待砍標記功能。

## 2002/03/08
- [ ] `post.c` 修正由 `every_Z` 重覆進入串接模式/新聞閱讀，離開會發生錯誤。

## 2002/03/10
- [ ] `talk.c` 隱形的人在 talk 時不被知道和誰 talk。

## 2002/03/14
- [ ] `talk.c` `bwboard.c` `chat.c` 談天/聊天/下棋時按 <kbd>Ctrl</kbd>+<kbd>Z</kbd> 回來後螢幕不會恢復。

## 2002/03/16
- [ ] `config.h` `talk.c` `visio.c` `bbsd.c` `bguard.c` `DETAIL_IDLETIME` 時常更新閒置時間。
- [ ] `bbsd.c` 更改故鄉判定程式

## 2002/03/21
- [ ] `talk.c` `bmw.c` `maple.p` `post.c` `pal.c` `aloha.c` 更新 `can_override()`。

## 2002/03/25
- [ ] `shm.c` `cache.c` `maple.p` `account.c` `mailtoall.c` `bmtad.c` `bguard.c`
- [ ] `pip_pk.c` `stock.c` 把 `attach_shm()` 用 `lib/shm.c` 中的 `shm_new()` 取代。

## 2002/03/26
- [ ] `restoreacct.c` 還原備份的 `.ACCT`。

## 2002/04/01
- [ ] `poststat.c` 修正本週/本月/本年的十大話題無法顯示。

## 2002/04/02
- [ ] `mail.c` 修正群組寄信如果收信人太多的話會掛掉。

## 2002/04/05
- [ ] `song.c` 在點歌時，進入看完歌單再出來後，底部的提示訊息會消失。
- [ ] `board.c` 看板前按 <kbd>v</kbd>/<kbd>V</kbd> 於 `.BRD` 第一個板無效。

## 2002/04/10
- [ ] `bmw.c` `maple.p` `post.c` `pal.c` `talk.c` `aloha.c` 所有 `xo_write()` 都整合起來。
- [ ] `xyz.c` BBSNET 程式修正。

## 2002/04/15
- [ ] `menu.c` `mail.c` 改 `vs_head()`。

## 2002/04/27
- [ ] `bbsmail.c` 避免 `SIGSEGV`。

## 2002/04/31
- [ ] `newbrd.c` 把連署記錄移到 `run/newbrd/` 下。
- [ ] `vote.c` 把投票記錄移到 `brd/_/@/G*`。

## 2002/05/02
- [ ] `newbrd.c` `@cosign.hlp` 修改連署內容。

## 2002/05/04
- [ ] `board.c` guest 不能從看板列表加入我的最愛。

## 2002/05/10
- [ ] `acct.c` 可設定使用者帳號。

## 2002/05/14
- [ ] `setusr.c` 可設定使用者習慣。
- [ ] `ufo.h` `struct.h` `acct.c` 把 `UFO` 獨立出來成為 `ufo.h`。
- [ ] `acct.c` 可設定使用者習慣。

## 2002/05/23
- [ ] `liteon.c` `menu.c` 新增開燈遊戲。

## 2002/05/29
- [ ] `mail.c` `@mbox.hlp` 信件設定閱讀記錄。

## 2002/05/30
- [ ] `railway.c` 新版程式。

## 2002/05/31
- [ ] `talk.c` 新的 Talk 程式。
- [ ] `bbsnet.c` 獨立於 `xyz.c`。
- [ ] `visio.c` `save_foot()` `restore_foot()` 修正。

## 2002/06/01
- [ ] `innbbsd/Makefile` `bbslib.c` `bbslink.c` `receive_article.c` `ncmbbs.h` `ncmbbs.c` `innd/ncmperm.bbs`
    提供 NoCeM 功能。

## 2002/06/02
- [ ] `menu.c` `xyz.c` `etc/loveletter` 提供情書產生器。
- [ ] `config.h` `global.h` `board.c` `talk.c` `HAVE_BRDPAL` 提供看板好友功能。
- [ ] `config.h` `ufo.h` `talk.c` `bmw.c` `@ulist.hlp` `HAVE_SUPERCLOAK` 提供超級隱形(紫隱)功能。
- [ ] `config.h` `global.h` `post.c` `HAVE_UNANONYMOUS_BOARD` 反匿名板。

## 2002/06/08
- [ ] `account.c` 空檔案不在板上 keeplog。
- [ ] `usies-sort.c` 修正如果沒有人上站會發現錯誤。

## 2002/06/11
- [ ] `post.c` 待砍的文章不能 mark。
- [ ] `config.h` `maple.p` `menu.c` `visio.c` `acct.c` `talk.c` `pal.c` `mine.c` 統一座標為 (x, y)。

## 2002/06/12
- [ ] `post.c` 修正在串接模式中板主 copy 文章到精華區，回串接會錯誤。
- [ ] `post.c` 修正在串接模式中閱讀文章 `more()` 時，板主按 <kbd>m</kbd> 會出鎚。
- [ ] `etc/feast` `global.h` `menu.c` `bbsd.c` 選單 feeter 顯示節日。
- [ ] `config.h` `mail.c` `post.c` `HAVE_DECORATE` 的部分程式刪除。
- [ ] `mail.c` 只讓板主把看板/精華區打包回家。

## 2002/06/13
- [ ] `check_newsfeeds.c` 檢查 `newsfeeds.bbs` 是否有不存在的看板還轉信的。

## 2002/06/15
- [ ] `board.c` `favor.c` 分類看板及我的最愛都可以使用自動跳去下一個未讀看板功能。

## 2002/06/15
- [ ] `talk.c` `t_pager()` 拒收廣播的程式修正。
- [ ] `newbrd.c` `nbrd_browse()` 讓瀏覽更流暢。

## 2002/06/17
- [ ] `topusr.c` 統計本月壽星。

## 2002/06/18
- [ ] `xover.c` `xo_forward()` 程式修正。

## 2002/06/19
- [ ] `bbsd.c` 修正 multi-login 踢到自己可以重複洗錢。

## 2002/06/21
- [ ] `fb2usr.c` `sob2usr.c` `wd2usr.c` 轉換程式要建 `usr/_/_/MF` 這目錄。
- [ ] `mail.c` 閱讀信件到一半按 <kbd>d</kbd> 刪除郵件以後要重新載入 `.DIR`。

## 2002/06/23
- [ ] `stock.c` 程式換新。

## 2002/07/02
- [ ] `camera.c` 程式修正。

## 2002/07/03
- [ ] `mail.c` `m_internet()` 版面更動。
- [ ] `visio.c` `bmw.c` 把回水球搬去最上面一行。

## 2002/07/04
- [ ] `gem.c` 精華區刪除時要確認。
- [ ] `gem-index.c` 程式修正。
- [ ] `src/Makefile` 修正有些 OS 會出現語法錯誤的問題。
- [ ] `*.h` `*.c` LINUX 下 `sys/time.h` 換成 `time.h`。
- [ ] `wd2gem.c` `wd2usr.c` 修正轉換過的精華區和精華區索引/異動衝突的問題。

## 2002/07/08
- [ ] `menu.c` `pad_draw()` 修正會斷線的問題。

## 2002/07/12
- [ ] `bbsnet.c` BBSNET 支援 port 不是 `23` 的 telnet。
- [ ] `classtable.c` 功課表功能加強。

## 2002/07/13
- [ ] `menu.c` `vs_head()` 程式修正。
- [ ] `railway.c` 修正抓不到網頁會進入無窮迴圈的問題。

## 2002/07/16
- [ ] `bank.c` 買完權限會提示重新上站字眼，避免使用者一直重覆買權限。
- [ ] `cache.c` `talk.c` 紫隱於作者線上及查詢時也看不見。

## 2002/07/18
- [ ] `bbsnet.c` BBSNET 加入記錄。
- [ ] `board.c` `gem/@/@class.hlp` 新增 Zap 掉全部看板的功能。
- [ ] `talk.c` 使用者名單看不到板友的故鄉。
- [ ] `talk.c` 和隱形人 Talk 不在動態中顯示。

## 2002/07/20
- [ ] `bank.c` 修正匯錢有 bug 的問題。
- [ ] `enews.c` `enews-open.c` `run/kimo/L` 奇摩新聞新增休閒類。

## 2002/07/21
- [ ] `post.c` 讓一般使用者也能看到進板畫面的 ANSI 碼 

## 2002/07/25
- [ ] `etc/webx.conf.cwb` 氣象報導。
- [ ] `newbrd.c` `struct.h` `@cosign.hlp` 新增作者查詢/設定及連署設定的功能。

## 2002/07/26
- [ ] `railway.c` 修正會留下殘餘檔案的問題。
- [ ] `railway.c` 修正選最後一行的站時會斷線。
- [ ] `bbcall.c` `dreye.c` `enew.c` `fortune.c` 錯字訂正。
- [ ] `mail.c` 站長可看 guest 的信箱。
- [ ] `acct.c` `newbrd.c` 開新板連署時要檢查看板板名是否合法。

## 2002/07/28
- [ ] `km.c` 遊戲成功或失敗都可接關。

## 2002/07/30
- [ ] `check_newsfeeds.c` 檢查 `newsfeeds.bbs` 中是否有未轉信的看板。

## 2002/08/05
- [ ] `bwboard.c` 新增軍棋/暗棋遊戲。

## 2002/08/06
- [ ] `show_id_in_USR.c` 可以搜尋某 id 在 `.USR` 中的 userno。

## 2002/08/07
- [ ] `acct.c` 修正 0Admin/QSetBoard 時若是 edit `currboard` 會失敗。
- [ ] `board.c` 修正在 Boards 看完某板文章後，再從選單按 <kbd>s</kbd> 進去該板，剛看完的文章還是沒看完。
- [ ] `acct.c` `pop3.c` POP3 認證程式修正。
- [ ] `bbsd.c` 讓 `ps` 時能出現 `bbsd` 是哪個使用者及來源。

## 2002/08/08
- [ ] `post.c` 站長改文章標題時，也可順便改暱稱。

## 2002/08/09
- [ ] `etc/jcee/90.txt` `etc/jcee/91.txt` `jcee.c` 提供 91 年大學聯招查榜。

## 2002/08/11
- [ ] `talk.c` `@ulist.hlp` 使用者名單切換故鄉/友誼。

## 2002/08/12
- [ ] `src/innbbsd/Makefile` `innbbsd.h` `innbbsd.c` 拿掉 `ADMINUSER`。
- [ ] `menu.c` 留言板改變版面。
- [ ] `stock-open.c` 修正星期一不會抓股市收盤價的問題。

## 2002/08/13
- [ ] `config.h` `global.h` `maple.p` `edit.c` `post.c` `km.c` `bmtad.c` `mailpost.c` 移除 `RANDOM_BANNER`。
- [ ] `edit.c` `post.c` `km.c` 增加函式 `ve_banner()`。

## 2002/08/14
- [ ] `modes.h` `struct.h` `bguard.c` `bmw.c` `cache.c` `talk.c` `visio.c` `BMW_COUNT` 程式改寫。

## 2002/08/16
- [ ] `talk.c` `@ulist.hlp` 使用者名單按 <kbd>L</kbd> 瀏覽水球。
- [ ] `bbsd.c` 修正 multi-login 時錯誤的 `acct_save()`。

## 2002/08/17
- [ ] `newbrd.c` `credit.c` `aloha.c` 統一 `vs_head()` 的用法。
- [ ] `bbs.h` `config.h` `global.h` `theme.h` `*.c` 佈景主題。

## 2002/08/21
- [ ] `visio.c` 修正在上站途中/精華區 idle 過久，會出現 `status_foot`。

## 2002/08/22
- [ ] `mailpost.c` 把一些函式用 `lib/` 中的代掉。
- [ ] `cache.c` `menu.c` 把亂數選動態看板的程式從 `film_out()` 移到 `movie()`。

## 2002/08/23
- [ ] `bmw.c` `theme.h` 回水球時按上下鍵改成`lastcmd`而不是回前後一個使用者。

## 2002/08/28
- [ ] `bbsd.c` 修正在開頭畫面過久未輸入時 igetch 會出現 movie。
- [ ] `so/*.c` `pip.c` 修正同一時間不同帳號亂數也相同的問題。
- [ ] `chessmj.c` 遊戲一開始時就先扣一份本金，以免玩家直接跳出時沒扣到錢。
- [ ] `bbs.h` `proto.h` `so/*.c` 修正亂數程式。
- [ ] `mail.c` multi-send 時取消而會被當成 `MAIL_REPLIED`。

## 2002/08/29
- [ ] `talk.c` 上站通知/協尋，即使我隱形，對方如果有看見隱形也通知。
- [ ] `bmw.c` 紫隱的人可以丟紫隱的人水球。
- [ ] `menu.c` 選單小幅調整。
- [ ] `expire.c` 程式修改。
- [ ] `talk.c` `bwboard.c` `cache.c` 聊天對象隱形時在使用者名單動態不被看到。
- [ ] `bbsd.c` `ufo.h` `maple.p` 把上站不送協尋併入 `UFO_NOALOHA`。

## 2002/08/30
- [ ] `cola2brd.c` `cola2gem.c` Cola 轉換程式。

## 2002/08/31
- [ ] `src/maple/Makefile` `src/util/Makefile` solaris 的 `Makefile`。
- [ ] `menu.c` SunOS 下沒有 `getloadavg()`。
- [ ] `global.h` `bank.c` `account.c` 轉帳記錄。
- [ ] `calendar.c` `menu.c` 萬年曆。

## 2002/09/01
- [ ] `src/maple/Makefile` `src/so/Makefile` `src/pip/Makefile` 改寫。
- [ ] `doc/b_postfix` FreeBSD 下 `postfix` + `bbsmail` + `mailpost` 的使用說明。
- [ ] `talk.c` `pal_ship()` 程式修正。

## 2002/09/02
- [ ] `receive_article.c` 修正使 NoCeM notice 有效。
- [ ] `gem/?/*` 新增一些說明文件。

## 2002/09/04
- [ ] `edit.c` 內碼輸入工具程式修正。
- [ ] `rec_sync.c` 新增 record 重整 library。

## 2002/09/05
- [ ] `menu.c` `pushbox.c` `etc/game/pushbox.map` 提供倉庫番遊戲。
- [ ] `wd2*.c` `sob2*.c` `src/util/tran/Makefile` 小改寫。
- [ ] `talk.c` 上站通知/協尋，紫隱不通知。

## 2002/09/10
- [ ] `mag2*.c` Magic 轉換程式。
- [ ] `make_new_USR.c` 程式修正。
- [ ] `etc/game/qkmj/*` QKMJ 程式版本更新。

## 2002/09/11
- [ ] `receive_article.c` 程式修正。

## 2002/09/14
- [ ] `pip.h` `pip_play.c` `pip_weapon.c` `gem.c` `railway.c` 程式修正。

## 2002/09/15
- [ ] `bar.c` 修改吧台瑪莉的中獎機率。

## 2002/09/16
- [ ] `gem.c` `favor.c` `song.c` `vote.c` 程式修正。

## 2002/09/17
- [ ] `acct.c` 修正用 multi-login + User/Info 的方法來使錢倍增。

## 2002/09/29
- [ ] `nine.c` 天地久久程式改寫。

## 2002/10/09
- [ ] `marie.c` `etc/game/marie` 小瑪莉程式改寫。
- [ ] `more.c` 修正在二頁以上的文章中按 `KEY_DEL` 發現錯誤。
- [ ] `song.c` 程式修正。

## 2002/10/11
- [ ] `post.c` 拂楓落葉斬可自定刪除的作者標題。

## 2002/10/14
- [ ] `marie.c` 小瑪莉限制總押金。
- [ ] `bmtad.c` `bbsmail.c` 擋 html 格式的信件。

## 2002/10/15
- [ ] `marie.c` 修正銘謝惠顧時中獎項目會出現錯誤。
- [ ] `menu.c` `tetris.c` 新增俄羅斯方塊遊戲。

## 2002/10/18
- [ ] `fortune.c` 運勢預測網頁路徑修正。
- [ ] `gem.c` 進入空精華區可以馬上貼複。

## 2002/10/21
- [ ] `acct.c` 增加刪除帳號的選項。

## 2002/10/26
- [ ] `wd2usr.c` `wd2gem.c` 程式修正。
- [ ] `sob.h` `sob2brd.c` `sob2gem.c` `sob2usr.c` SOB 轉換程式改寫。

## 2002/10/27
- [ ] `post.c` 修正加密文章可任意轉達使用者。

## 2002/10/28
- [ ] `post.c` 修正 guest 可藉回作者信箱來寄信。
- [ ] `mail.c` 群組回信程式改寫。

## 2002/10/29
- [ ] `gem.c` 修正精華區刪除文章完以後，目錄內剩 0 篇，螢幕會不乾淨。

## 2002/10/31
- [ ] `post.c` 修正 guest 可轉達文章。
- [ ] `src/pip/*` 電子雞遊戲新增升級任務、武器系統。
- [ ] `enews.c` 修正奇摩新聞轉錄取消螢幕錯亂的問題。

## 2002/11/01
- [ ] `board.c` `favor.c` `config.h` `ENHANCED_BSHM_UPDATE`
- [ ] 看板列表刪除/標記文章不列入未讀的燈。

## 2002/11/02
- [ ] `bmw.c` `config.h` 水球存證。
- [ ] `song.c` 限制每天點歌次數。

## 2002/11/05
- [ ] `bguard.c` `bmw.c` `talk.c` 把檢查是否能看見對方的程式整理在一起。

## 2002/11/07
- [ ] `src/web/` `html/` WEB-BBS 功能。

## 2002/11/11
- [ ] `bj.c` `chessmj.c` `fantan.c` `gp.c` `seven.c` 改變取牌的亂數程式。

## 2002/11/13
- [ ] `showUSR.c` 程式改寫。

## 2002/11/14
- [ ] `Makefile` 提供 `make update` 的指令。

## 2002/11/17
- [ ] `make_new_USR.c` 修正錯誤。

## 2002/11/22
- [ ] `xover.c` `vote.c` 把類 `XZ_` 結構的游標移動程式整理在一起。
- [ ] `xover.c` `maple.p` `camera.c` `struct.h` `*.c` 把 help 獨立出 `film_out` 變成選單。

## 2002/11/26
- [ ] `favor.c` 修正精華區捷徑板主權限錯誤的問題。

## 2002/12/04
- [ ] `favor.c` 新增我的最愛選取看板功能。

## 2002/12/05
- [ ] `menu.c` 修正 #define `COLOR_HEADER` 時，郵差來按鈴的顏色會不對。
- [ ] `talk.c` 若在 Talk 時按了一堆印不出來的字，就不寫進 talk-log。
- [ ] `*.c` `mail_self()` 多一個參數，更方便使用。
- [ ] `talk.c` 把 `mail_self()` 整合進 `talk_save()`。
- [ ] `talk.c` 修正 Talk 時最後一句話不會被記在談天記錄中。
- [ ] `talk.c` 修正 Talk 時當游標再對方區域時按下<kbd>Ctrl</kbd>+<kbd>k</kbd>會發生錯誤。

## 2002/12/06
- [ ] `board.c` `favor.c` `theme.h` `maple.p` 讓看板列表中的秘密板、好友板、ZAP 板標示清楚。

## 2002/12/09
- [ ] `enews-open.c` 抓奇摩新聞的程式更新。

## 2002/12/10
- [ ] `pal.c` 編輯板友名單時，引入好友名單。

## 2002/12/13
- [ ] `global.h` `struct.h` `account.c` `board.c` 可以 zap 掉分類。

## 2002/12/15
- [ ] `account.c` 程式改寫。

## 2002/12/16
- [ ] `bbslib.c` 如果 nodelist.bbs 裡面填的不是正解，對方還是可以 access。
- [ ] `mail.c` 修改群組寄信時，若名單過多會斷線。

## 2002/12/18
- [ ] `song.c` 程式修正。
- [ ] `bbslink.c` news 輸出 RFC 2045。

## 2002/12/21
- [ ] `dns_ident.c` `dl_lib.c` 程式更新。

## 2002/12/22
- [ ] `*.c` Query 程式更新。

## 2002/12/24
- [ ] `struct.h` `acct.c` `bbsd.c` 把密碼長度統一限定為 `8`。
- [ ] `edit.c` 因斷線未完成的文章可選擇丟去暫存檔或信箱。

## 2002/12/26
- [ ] `post.c` 在串接模式下 edit 文章完，應該出現 `xpost_head()` 而不是 `post_head()`。

## 2002/12/27
- [ ] `xover.c` `post.hlp` `gem.hlp` 移除 Z-modem 下載功能。
- [ ] `fortune.c` `menu.c` 移除運勢預測功能。
- [ ] `bbcall.c` `menu.c` 移除 BBCall 功能。
- [ ] `emailpage.c` `mail.c` `bmtad.c` 移除 emailpage 功能。
- [ ] `config.h` `edit.c` 刪除 `HAVE_ORIGIN`。
- [ ] `config.h` `km.c` `edit.c` `talk.c` 刪除 `REALINFO`。
- [ ] `cache.c` 程式更新。
- [ ] `bingo.c` 程式更新。

## 2002/12/28
- [ ] `camera.c` `menu.c` `config.h` 程式更新。

## 2002/12/30
- [ ] `camera.c` `cache.c` `str_rle.c` `fshm` 不壓縮。

## 2003/01/01
- [ ] `talk.c` 修正站長做好友廣播時可能會誤植的問題。
- [ ] `ufo.h` `acct.c` 把習慣的長度抓出來 define 成 `NUMUFOS`。
- [ ] `bmta.sh` 分析 `bmta.log` 的連線次數。

## 2003/01/02
- [ ] `pip_fight.c` 修正有些技能學到以後無法使用的問題。

## 2003/01/03
- [ ] `src/innbbsd/*.c` 程式精簡。
- [ ] `*.c` 移除 `HAVE_NOFORWARD`，並把 `NUMATTRS` 和 `STR_BATTR` 抓出來定義。
- [ ] `*.c` 調整某些站長權限。

## 2003/01/10
- [ ] `topgem.c` 程式更新。

## 2003/01/11
- [ ] `more.c` 程式換新，修正上捲一行錯誤、搜尋字串會找不到的問題。
- [ ] `innbbsd/*` 轉信程式改版。
- [ ] `aloha.c` `newbrd.c` 程式更新。
- [ ] `xover.c` 修正在 help 時按 `KEY_HOME` 會斷線的問題。

## 2003/01/12
- [ ] `bmw.c` 修正水球儲存變成相反對話的問題。

## 2003/01/13
- [ ] `song.c` 匿名點歌也算入每日只能點三首。

## 2003/01/15
- [ ] `innbbsd/*` 程式更新。
- [ ] `xover.c` 修正 `every_Z` 進入看板時可能造成文章已讀/未讀錯誤。

## 2003/01/19
- [ ] `admutil.c` 站長重置系統時儲存看板閱讀記錄。
- [ ] `modes.h` `menu.c` 拿掉 `M_CLASS`。

## 2003/01/23
- [ ] `railway.c` 隨台鐵的格式更新。

## 2003/02/07
- [ ] `enews.c` 奇摩新聞轉寄到站外。

## 2003/02/08
- [ ] `edit.c` 限制暫存檔大小。
- [ ] `marie.c` 降低大小瑪莉遊戲的期望值。
- [ ] `dice.c` 降低狂擲骰子遊戲的期望值。
- [ ] `xover.c` `enews.c` `mail.c` 整理 `DENYMAIL` 的程式。

## 2003/02/11
- [ ] `cola.h` `cola2brd.c` `cola2gem.c` `cola2usr.c` ColaBBS 轉換程式。

## 2003/02/19
- [ ] `innbbsd/*` `innd/newsfeeds.bbs` `etc/b2g_table` `etc/g2b_table` 簡繁體轉信。

## 2003/02/21
- [ ] `cola2post.c` Cola 看板文章格式轉換。

## 2003/02/23
- [ ] `post.c` 修正轉錄文章到不做熱門話題統計的看板，這篇被轉錄的文章仍然會被列入統計。
- [ ] `acct.c` 只有 `PERM_SYSOP` 能變更其他站務的密碼。

## 2003/02/25
- [ ] `cola2gem.c` `sob2gem.c` `wd2gem.c` 修正子目錄轉換失敗的問題。

## 2003/03/02
- [ ] `brdmail.c` 由 Internet 寄信給 BBS 站內看板。

## 2003/03/03
- [ ] `counter.c` 歷史軌跡。

## 2003/03/04
- [ ] `enews-open.c` 奇摩新聞標題太長時會有錯誤。
- [ ] `song.c` 歌本加密時不能點那首歌。

## 2003/03/05
- [ ] `edit.c` 簽名檔也算引言長度。

## 2003/03/06
- [ ] `song.c` `global.h` 歌本改放在 ktv 板精華區。
- [ ] `gem-index.c` 精華區索引/異動 可隱藏。

## 2003/03/07
- [ ] `gem.c` 修正精華區複製的部分程式。
- [ ] `hdr.h` `*.c` 不用 `lazy_delete`，`rec_del()` 拿掉一個欄位，`POST_DELETE` 移除。
- [ ] `hdr.h` `post.c` `POST_LABEL` 換編號。
- [ ] `*.c` 更新 record 時要檢查。
- [ ] `rec_article.c` 收到 cancel 文章直接刪除，不 `lazy_delete`。
- [ ] `hdr.h` `*.c` `POST_CANCEL` 移除。

## 2003/03/10
- [ ] `vote.c` 分身不能投票。
- [ ] `rec_loc.c` `bmw.c` `aloha.c` 區段刪除程式修正。

## 2003/03/15
- [ ] `edit.c` Show 簽名檔時，前兩個只能秀出 `MAXSIGLINES-1` 行。

## 2003/03/23
- [ ] `bmtad.c` 寄信給看板。
- [ ] `mail.c` mail 輸出 RFC 2045。

## 2003/03/25
- [ ] `hdr_stamp.c` `post.c` `enews.c` `gem.c` `hdr_stamp()` 新增 `HDR_COPY` 這個 token。
- [ ] `bank.c` 轉帳程式修正。
- [ ] `post.c` `outgo.c` 避免沒有 nick 時會造成轉信失敗。
- [ ] `expire.c` 程式小改版。

## 2003/03/26
- [ ] `admutil.c` 修正全站/板主通告部分程式，並加上郵差來按鈴。

## 2003/03/27
- [ ] `post.c` `mail.c` 改標題時若沒有變動，則不問 `Y/N`。
- [ ] `more.c` 程式修正。

## 2003/03/30
- [ ] `rec_article.c` 修正輸入 RFC 2047 與 ANSI 控制碼的問題。

## 2003/03/31
- [ ] `pip_race.c` 收穫季烹飪比賽新增菜色選擇。

## 2003/04/01
- [ ] `gem.c` 精華區的符號分清楚一點。

## 2003/04/07
- [ ] `post.c` `mail.c` `hdr_out()` 從 `mail.c` 搬去 `post.c`。
- [ ] `post.c` 刪除 `getsubject()`。
- [ ] `rec_put.c` 程式修改。

## 2003/04/08
- [ ] `xover.c` 類 `XZ_*` 結構的游標移動程式修改。

## 2003/04/11
- [ ] `rfc2047.c` `dao.p` `bbslink.c` `mail.c` news/mail 輸出 RFC 2047。

## 2003/04/15
- [ ] `inntobbs.c` `inntobbs.h` `rec_article.c` 沒用到的 `HEADER` 就不取了。

## 2003/04/16
- [ ] `board.c` 第二次進入 `board_main` 時，要 `free` 掉 `class_img`。

## 2003/04/22
- [ ] `talk.c` `config.h` `theme.h` 將 #ndef `ADV_ULIST` 的程式刪除。
- [ ] `aloha.c` `bmw.c` `pal.c` `xxxx_body()` 統一和其他格式一樣。
- [ ] `talk.c` `ulist_body()` 統一和其他格式一樣，並獨立出 `ulist_item`。
- [ ] `railway.c` 台鐵網頁格式變動。

## 2003/04/23
- [ ] `post.c` `bmtad.c` `topgem.c` `bwboard.c` 修改一下使符合 gcc 3.2.2。

## 2003/04/26
- [ ] `talk.c` `ulist_cb[]` 程式修正。
- [ ] `xover.c` 修正於 help 新增說明時，若超過一頁會發生錯誤。

## 2003/05/03
- [ ] `edit.c` 注音文過濾。

## 2003/05/04
- [ ] `Makefile` 全部統一寫法。

## 2003/05/10
- [ ] `str_ansi.c` `bmtad.c` `rec_article.c` `bbsmail.c` `brdmail.c` `mailpost.c` `cola2post.c`
    把 `strip_ansi` 的程式統一在 `lib/`。
- [ ] `cola2brd.c` `fb2brd.c` `mag2brd.c` `sob2brd.c` `wd2brd.c`
    轉換程式處理文章標題時加入 `strip_ansi`。
- [ ] `mak_dirs.c` `bbsd.c` `cola2brd.c` `fb2brd.c` `mag2brd.c` `sob2brd.c` `wd2brd.c` `transacct.c`
    把 `mak_links` 的程式統一在 `lib/`。
- [ ] `help.c` `xover.c` `xo_help()` 搬去 `so/`。

## 2003/05/11
- [ ] innbbsd 部分程式改寫。

## 2003/05/13
- [ ] `bmtad.c` `bbsmail.c` 擋掉非 big5 的信件。

## 2003/05/14
- [ ] `config.h` `visio.c` `bguard.c` `TIME_KICKER` 是否自動簽退 idle 過久的使用者。
- [ ] `bbs.h` `Makefile` 拿掉 `REDHAT` 的部分。

## 2003/05/15
- [ ] `modes.h` `board.c` `post.c` `xover.c` 將 `STAT_BOARD` 與 `STAT_BM` 分離出來。
- [ ] `post.c` `xover.c` 站長無法看到加密文章。

## 2003/05/24
- [ ] `showDIR.c` 秀出 `.DIR` 的資訊。

## 2003/05/25
- [ ] `board.c` 修正錯誤的看板閱讀記錄會當在上站畫面。
- [ ] `mail.c` 寄出的信的檔頭 `From` 欄位加上 real name 欄位。

## 2003/05/26
- [ ] `lottery.c` `lottery-open.c` `etc/game/lottery.main` `etc/game/lottery.rule` 樂透遊戲。

## 2003/05/27
- [ ] `*.c` `*.h` 支援超過 24 列長螢幕的畫面。
- [ ] `util/Makefile` 改寫。

## 2003/05/28
- [ ] `post.c` 串列/新聞程式改寫。

## 2003/06/04
- [ ] `config.h` `bmtad.c` `bbsmail.c` `mail.c` 把 `MYCHARSET` 抓出來定義。

## 2003/06/05
- [ ] `cache.c` `post.c` `utmp_get()` 程式修正。
- [ ] `hdr.h` `gem.c` `xover.c` `song.c` `camera.c` `gem-index.c` `gem-check.c` 刪除 `GEM_HTTP` 及 `GEM_URL`。

## 2003/06/06
- [ ] `innbbsd/*` 轉信程式修改。

## 2003/06/07
- [ ] `post.c` `mail.c` `maple.p` 把 tag 標 `*` 的坐標從 `(x, 8)` 移到 `(x, 6)`。
- [ ] `config.h` `hdr.h` `post.c` 文章評分功能。

## 2003/06/11
- [ ] `global.h` `*.c` 使用全域變數 `currbno` 來代替 `brd_bno(currboard)`。

## 2003/06/13
- [ ] `rec_article.c` 保留被 cancel 的文章於 deleted 板。

## 2003/06/18
- [ ] `global.h` `post.c` `edit.c` 使用全域變數 `currbattr` 將 `battr` 獨立出 `bbstate`。

## 2003/06/19
- [ ] `bmw.c` `mail.c` 刪除 `bmw_choose()` 的功能。
- [ ] `bmw.c` `theme.h` `visio.c` `talk.c` 改變水球格式。

## 2003/06/20
- [ ] `bmw.c` `visio.c` 在 reply 水球時會秀出上次傳給這人的話。

## 2003/06/27
- [ ] `global.h` `ufo.h` `visio.c` `more.c` `acct.c` 移除 `UFO_COLOR`。

## 2003/06/28
- [ ] `battr.h` `struct.h` `bbs.h` `acct.c` 將 battr 整理到 `battr.h`。
- [ ] `bbslink.c` 修正 cmsg 與要砍的文章 Message-ID 相同會被 news server 拒絕的問題。
- [ ] `rec_article.c` `brdmail.c` `bmtad.c` 拿掉 `轉信站:` 及 `Origin:` 的文章檔頭。
- [ ] `more.c` 不偵測 `轉信站:` 及 `Origin:` 的文章檔頭。

## 2003/06/30
- [ ] `struct.h` `acct.c` `board.c` `post.c` `gem.c` `vote.c` `newbrd.c` `tran/*brd.c` `account.c`
    增加 `BRD.class` 看板分類欄位。
- [ ] `theme.h` `board.c` 抓出 `ICON_(NO)TRAN_BRD` 來定義。
- [ ] `windtop.h` `windtop2usr.c` `windtop2brd.c` WindTop 轉換程式。

## 2003/07/02
- [ ] `bbsd.c` 允許 `belong_list()` 中的 `desc` 可以有空白。

## 2003/07/03
- [ ] `weather.sh` 天氣預報加上 more 檔頭。
- [ ] `topusr.c` 統計性別。

## 2003/07/04
- [ ] `bbslink.c` 從上次中斷處繼續送信。
- [ ] `admutil.c` 審完註冊單通過，會使當時在線上的使用者亮起郵差來按鈴。
- [ ] `acct.c` `showACCT.c` `topusr.c` 改變 `sex` 欄位對性別的定義，增加中性。

## 2003/07/05
- [ ] `struct.h` `ufo.h` `perm.h` 將 `PERM_DATALOCK` `PERM_COINLOCK` `UFO_BIFF`
- [ ] `UFO_REJECT` `UFO_BIRTHDAY` `UFO_MQUOTA` 獨立出來成為 `UTMP.status`。
- [ ] 將 `cuser.ufo` 及 `cutmp->ufo` 同步。

## 2003/07/06
- [ ] `bbslink.c` `bbsnnrp.c` 支援需要帳號/密碼的 news server。
- [ ] `talk.c` `bmw.c` 廣播程式修正。

## 2003/07/07
- [ ] `bhttpd.c` 簡易的 Web-BBS。

## 2003/07/09
- [ ] `transbrd.c` `.BRD` 轉換程式。

## 2003/07/13
- [ ] 將 doc 換成 html 檔。

## 2003/07/14
- [ ] `decode.ic` 刪除。
- [ ] `mailpost.c` 程式改寫。

## 2003/07/15
- [ ] `tetris.c` 程式改寫。

## 2003/07/18
- [ ] `bmw.c` 修正若對方下站了，水球可能會丟錯人。
- [ ] `board.c` 全域搜尋可同時搜尋標題。
- [ ] `pal.c` 好友/群組名單廣播。
- [ ] `bbsnnrp.c` 大幅改寫。

## 2003/07/19
- [ ] `gem.c` `post.c` `mail.c` `edit.c` `vote.c` 標題 `vget()` 長度的修改。
- [ ] `admutil.c` 全站使用者/板主寄信程式修改。

## 2003/07/20
- [ ] `hdr.h` `hdr_fpath.c` `gem.c` 把 `GEM_EXTEND` 的功能移除。
- [ ] `gem.c` 修正板主可以在其他板使用 `gem_state`。

## 2003/07/22
- [ ] `bmw.c` 與特定對象的水球存至信箱。

## 2003/07/23
- [ ] `dragon.c` 接龍遊戲。
- [ ] `config.h` `window.c` `edit.c` `visio.c` 蹦出式視窗。
- [ ] `etc/game/pip` 補齊電子雞的插圖。
- [ ] `account.c` 檢查 `CH_MAX` 是否小於 Class 數量。

## 2003/07/24
- [ ] `camera.c` 系統文件有缺時，會自動補上通知。

## 2003/07/25
- [ ] `account.c` 整理 `.BRD` 中空白的欄位。
- [ ] `bmw.c` `visio.c` Ctrl+T 可以回別顆水球。

## 2003/07/27
- [ ] `post.c` `manage.c` 將板主選單整合在一起。
- [ ] `bhttpd.c` bshm 程式改寫。
- [ ] `gemd.c` 修正可以由 gopher 看秘密板精華區的問題。

## 2003/07/28
- [ ] `bbsd.c` `mail.c` `global.h` 將 `etc/*` 的檔案路徑整理出來 `FN_ETC_*`。

## 2003/07/31
- [ ] `xover.c` `every_Z` 修正藉著 <kbd>^Z</kbd> 跳去別板再回這板時的錯誤。

## 2003/08/01
- [ ] `bbsd.c` 長螢幕程式修正。

## 2003/08/03
- [ ] `Makefile` `make update` 若出現 error 則忽略。

## 2003/08/07
- [ ] `pal.c` 修正板友名單引入好友錯誤。

## 2003/08/09
- [ ] `jcee.c` `etc/jcee/92.txt` 92 年查榜服務。

## 2003/08/13
- [ ] `struct.h` `enews.c` `xover.c` 移除 `MQ_UUENCODE`。
- [ ] `struct.h` `mail.c` 修正信件打包時 Content-Type 錯誤。

## 2003/08/15
- [ ] `board.c` 搜尋下一個未讀看板。
- [ ] `rec_del.c` 程式修正。

## 2003/08/16
- [ ] `aloha.c` 用 `rec_del` 來取代 `rec_loc`。
- [ ] `src/lib` 重新整理，將沒用到的刪除。
- [ ] `bbsmail.c` `brdmail.c` 解碼 quoted-printable/base64 的信件。
- [ ] `str_decode.c` 修正當信件內文為 QP 編碼時會發生錯誤。
- [ ] `str_decode.c` `bbsmail.c` `brdmail.c` `bmtad.c` 將取出 charset 及 encode 的程式集中到 `lib`。

## 2003/08/17
- [ ] `bmw.c` 程式修正。

## 2003/08/23
- [ ] `src/Makefile` `src/game/` 將 `so/` 下的遊戲及網路服務搬到 `game/`。
- [ ] `str_has.c` `acct.c` `board.c` `bmtad.c` 板主名單程式部分整併。
- [ ] `edit.c` 修正簡化編輯器。

## 2003/08/24
- [ ] `todo.c` `etc/todo.welcome` `global.h` `menu.c` 行事曆。

## 2003/08/27
- [ ] `bbsnnrp.c` 修正當 host/active 不存在時，無法 unlock 的問題。
- [ ] `favor.c` 我的最愛搜尋下一個未讀看板。

## 2003/08/29
- [ ] `stock.c` 程式修正。

## 2003/09/06
- [ ] `pal.c` 好友/板友/特別名單交叉引用。
- [ ] `vote.c` 投票到一半，板主可以偷看目前投票狀況。

## 2003/09/07
- [ ] `pal.c` 好友名單區段刪除。
- [ ] `config.h` `menu.c` `acct.c` 刪除 `HAVE_REPORT` 記錄除錯的程式。
- [ ] `visio.c` `ANSILINELEN` 程式修正。

## 2003/09/11
- [ ] `account.c` `admutil.c` 重置系統時不做熱門話題統計。

## 2003/09/12
- [ ] `favor.c` 我的最愛區段刪除。

## 2003/09/13
- [ ] `bhttpd.c` 提供 email 發表文章。
- [ ] `bguard.c` `talk.c` 站長 kick 使用者時應將 `ushm->count` 減一。

## 2003/09/14
- [ ] `mail.c` 站務信箱選 ID 時可以按空白鍵自動搜尋。
- [ ] `post.c` 全文搜尋。

## 2003/09/23
- [ ] `xpost.c` `post.c` `mbox.c` 將串接搜尋模式整合入信箱。

## 2003/09/26
- [ ] `bbsd.c` `mail.c` 將 `m_count()` 檢查信箱已經超過上限的部分移到 `bbsd.c`。
- [ ] `mail.c` 將回信的程式整併到 `do_mreply()` 裡面。
- [ ] `mail.c` `post.c` 改寫 `mbox_browse()` 及 `post_browse()`。
- [ ] `mail.c` `xpost.c` 串接郵件中可回信。
- [ ] `menu.c` `mail.c` `admutil.c` 將選單中會用到的郵件選項都改名為 `m_xxxx()`。
- [ ] `menu.c` `acct.c` `admutil.c` 將選單中會用到的站務選項都改名為 `a_xxxx()`。

## 2003/09/27
- [ ] `post.c` `gem.c` 精華區轉錄到看板、轉達使用者。

## 2003/09/28
- [ ] `bbsd.c` 改寫 `tn_login()`。

## 2003/09/29
- [ ] `ufo.h` `*.c` 將 `HAS_STATUS` 巨集整理出來。

## 2003/10/01
- [ ] `xover.c` `bmw.c` `pal.c` `favor.c` `aloha.c` `post.c` `mail.c` `gem.c` 將區段刪除的程式整合在一起。

## 2003/10/03
- [ ] `gem.c` `global.h` `backupgem.c` `restoregem.c` 廢除精華區資源回收筒的功能。
- [ ] `gem.c` 改寫精華區刪除的程式。
- [ ] `xover.c` `post.c` `mail.c` `gem.c` 將標籤刪除的程式整合在一起。

## 2003/10/04
- [ ] `edit.c` `post.c` `modes.h` 發文加密存檔。
- [ ] `enews.c` 程式修正。

## 2003/10/08
- [ ] `gem.c` 修正加密文章可藉由精華區附加檔案來偷看。

## 2003/10/20
- [ ] `bhttpd.c` 允許使用者登入。

## 2003/11/09
- [ ] `edit.c` 亂數選擇簽名檔。

## 2003/11/10
- [ ] `stock.c` `stock-open.c` 移除股市遊戲。

## 2003/11/18
- [ ] `visio.c` `more.c` `edit.c` `etc/*` 停用 `###` `%%%` 顯示使用名稱控制碼，改用 `**s` 這類型的。

## 2003/11/22
- [ ] `dl_lib.c` `visio.c` `bbslink.c` `fileglue.c` `verbose.c` 將 `<varargs.h>` 改用 `<stdarg.h>`。

## 2003/11/23
- [ ] `more.c` 修正 `more_line()` 在剛好 80 字會多一空行。
- [ ] `struct.h` `edit.c` `more.c` `talk.c` `visio.c` 將螢幕寬度都統一為 `SCR_WIDTH`。

## 2003/11/26
- [ ] `account.c` 開票以後要清除可投票名單、已領票記錄。

## 2003/11/28
- [ ] `board.c` `class_namemode()` 加入時間限制。

## 2003/11/30
- [ ] `visio.c` `vget()` 支援 `KEY_DEL`。

## 2003/12/01
- [ ] `str_time.c` `*.c` 將所有時間的格式都統一。

## 2003/12/02
- [ ] `acct.c` `admutil.c` `manage.c` `newbrd.c` 修正處理權限變動的程式。
- [ ] `post.c` 自動偵測 cross-post。

## 2003/12/03
- [ ] `game/*.c` 修正玩遊戲時若開啟方向鍵全型偵測會造成錯誤。

## 2003/12/04
- [ ] `more.c` `xover.c` `SLIDE_SHOW` 文章/動畫自動播放。

## 2003/12/11
- [ ] `rec_sync.c` `pal.c` `aloha.c` 將索引排序的程式獨立出來。

## 2003/12/14
- [ ] `bbslink.c` 簡化 Message-ID 的產生公式，直接拿檔名、板名來表示。
- [ ] `bnntpd.c` BBS-NNTP server。

## 2004/01/02
- [ ] `struct.h` `account.c` `board.c` `manage.c` `bhttpd.c` `bmtad.c` 將板友名單放進 shm。

## 2004/01/03
- [ ] `attr_lib.c` `attr.h` 刪除 attr library。
- [ ] `bwboard.c` 改寫戰績記錄程式。

## 2004/01/04
- [ ] `pal.c` `manage.c` `board.c` 板友/好友快取程式整理在一起。

## 2004/01/06
- [ ] `acct.c` `bbsd.c` `bank.c` `game/*.c` 金銀幣溢位檢查。

## 2004/01/10
- [ ] `bbsd.c` 登入時在輸入完 ID/密碼 以後才載入 `.ACCT`，避免有人藉 `vget` 產生的時差來洗錢。
- [ ] `global.h` `bbsd.c` 拿掉 `dns_ident` 的步驟。

## 2004/01/12
- [ ] `global.h` `cache.c` `bbsd.c` 把一些 `etc/` 文件搬進 cache。
- [ ] `admutil.c` 把 `etc/` 文件都加入系統檔案清單中。
- [ ] `expire.c` `rec_article.c` 修正 `chrono32` 錯誤。

## 2004/01/13
- [ ] `str_hash2.c` `str_xor.c` `mail.c` `ifsigned.c` 改變電子簽章的產生公式。

## 2004/01/30
- [ ] `ufo.h` `edit.c` `help.c` 修正在 `vedit` 時進入 help 再進入 `vedit` 造成文章流失。

## 2004/02/11
- [ ] `enews-open.c` 雅虎！奇摩新聞網頁改版。

## 2004/02/16
- [ ] `dreye.c` `menu.c` 譯點通線上字典服務停止。

## 2004/02/17
- [ ] `more.c` `post.c` `mail.c` `gem.c` 搜尋字串反白。

## 2004/02/21
- [ ] `bhttpd.c` 登入、發表文章、我的最愛程式修正。

## 2004/02/28
- [ ] `post.c` 轉錄時可選擇加密存檔。

## 2004/03/03
- [ ] `xpost.c` 串列模式閱讀一頁文章完可搜尋字串、暫存、評分。

## 2004/03/05
- [ ] `pal.c` `aloha.c` `bmw.c` 好友/上站通知/水球系統支援 tag 標籤。
- [ ] `xover.c` `post.c` `mbox.c` `gem.c` `pal.c` `aloha.c` `bmw.c` 標籤刪除整合在一起。

## 2004/03/06
- [ ] `vote.c` 加入區段刪除及標籤刪除。
- [ ] `more.c` `xpost.c` 串列/字串搜尋時，處理中文字變小寫的問題。

## 2003/03/11
- [ ] `board.c` `post.c` `account.c` `rec_article.c` `bmtad.c` `brdmail.c` 修正看板 `btime` 更新的程式。

## 2003/03/13
- [ ] `bbsmail.c` `brdmail.c` 改寫，盡量改用 `libdao` 的函式。

## 2003/03/14
- [ ] `mailpost.c` `bmtad.c` 拿掉 `mailpost` 的部分。

## 2004/03/15
- [ ] `struct.h` `*.c` 將 `PASSLEN` 替換成 `PASSLEN + 1`，並將 `PASSLEN` 從 `14` 改為 `13`。

## 2004/03/18
- [ ] `bquota.c` 不刪除擋信名單上來源的信。
- [ ] `config.h` `acct.c` `admutil.c` `HAVE_TRUST` 變成始終 `#define`。
- [ ] `global.h` `admutil.c` `bmtad.c` `acl.ic` `etc/mail.acl` `etc/unmail.acl` 收信黑白名單。

## 2004/03/22
- [ ] `topgem.c` 程式改寫。

## 2004/03/30
- [ ] `favor.c` 我的最愛可以到下一頁搜尋未讀看板。

## 2004/04/01
- [ ] `innbbsd/*` InnNNSD 改版。

## 2004/04/05
- [ ] `str_len.c` `talk.c` `acct.c` 拿掉 `str_len`。
- [ ] `board.c` `acct.c` 在 Class 下直接新增看板到該分類。

## 2004/04/13
- [ ] 將 `bguard.c` `bmtad.c` `bpop3d.c` `gemd.c` `xchatd.c` `bhttpd.c` `bnntpd.c` 搬到 `daemon/` 目錄。

## 2004/04/24
- [ ] `song.c` 點歌到站外信箱。

## 2004/04/25
- [ ] `innbbsd/*.c` `admutil.c` 將轉信設定和 BBS 結合。

## 2004/04/30
- [ ] `struct.h` `bbslink.c` `post.c` `outgo.c` 將 `out.bntp/*.link` 的改為 binary 格式。

## 2004/05/11
- [ ] `mail.c` `post.c` 統一寄信完的訊息。

## 2004/05/19
- [ ] `game/*.c` `so/*.c` 遊戲訊息中的「你」改成「您」。
- [ ] `mailpost.c` 認證信部分程式修正。

## 2004/05/21
- [ ] `qkmj.c` `etc/game/qkmj` 移除 QKMJ 連線麻將。
- [ ] `jcee.c` `etc/jcee` 移除聯考榜單查詢。
- [ ] `webx.c` 移除氣象報導抓取。
- [ ] `edit.c` 移動插入範本。

## 2004/06/13
- [ ] `newbrd.c` 簡化連署系統。

## 2004/06/16
- [ ] `board.c` 修正可進入已刪除的看板。

## 2004/06/21
- [ ] `*.c` `*.h` 支援超過 80 行寬螢幕的畫面。

## 2004/07/21
- [ ] `bhttpd.c` 發表文章取消字數限制，增加發文來源，並能轉信。

## 2004/07/22
- [ ] `favor.c` 我的最愛提供分隔線。

## 2004/08/02
- [ ] `more.c` 中文字斷行。

## 2004/08/04
- [ ] `bbsmail.c` `brdmail.c` 加入擋信黑白名單機制。

## 2004/08/06
- [ ] `str_lowest.c` `str_sub.c` 抓到 library。
- [ ] `struct.h` `admutil.c` `bbslib.c` `rec_article.c` 加入擋信規則。

## 2004/08/24
- [ ] `xpost.c` 全文搜尋加入範圍限制。

## 2004/09/01
- [ ] `menu.c` `cache.c` 主選單/動態看板依寬螢幕置中。

## 2004/09/07
- [ ] `config.h` `xover.c` `post.c` `mail.c` `xpost.c` `HAVE_XYPOST` 變成始終 #define。

## 2004/09/11
- [ ] `bbsd.c` `struct.c` `camera.c` 刪除 `FILM_WELCOME` 簡化進站畫面。

## 2004/09/16
- [ ] `xover.c` `xo_thread()` 改寫。

## 2004/09/17
- [ ] `more.c` 超過 256 頁的檔案也能夠正確的被計算。

## 2004/09/22
- [ ] `bbslink.c` `bbsnnrp.c` 修正錯誤並增加 `-v` 的訊息。

## 2004/09/30
- [ ] `windtop2pip.c` WindTop 轉換電子雞。

## 2004/10/01
- [ ] `talk.c` 使用者名單程式翻新。

## 2004/10/02
- [ ] `*.c` `*.h` `etc/help/pal` 「好友名單」改為中性的「朋友名單」，以區分「好友」、「壞人」、「板友」。
- [ ] `theme.h` `talk.c` `board.c` 「板友」改為「板伴」。

## 2004/10/06
- [ ] `more.c` 閱讀到一半時若使用者中斷，則不秀 footer。 
- [ ] `bmtad.c` 黑/白名單檢查程式修正。
- [ ] `talk.c` 使用者名單依板伴排序。

## 2004/10/07
- [ ] `ulist.c` 將使用者名單部分的程式從 `talk.c` 獨立出來。
- [ ] `ulist.c` 移除任意排列。

## 2004/10/10
- [ ] `mail.c` `song.c` 站外寄信失敗就存底稿。
- [ ] `talk.c` `bbsd.c` `FN_FRIEND_BENZ` 同步程式。

## 2004/10/11
- [ ] `struct.h` `talk.c` 將系統協尋的 `struct BENZ` 獨立出來。
- [ ] `struct.h` `talk.c` `aloha.c` 將上站通知的 `struct FRIENZ` 獨立出來。

## 2004/10/16
- [ ] `util/uno/*` 重寫整理 `userno` 的程式。

## 2004/10/19
- [ ] `ulist.c` 閱讀匿名板不看到板伴。

## 2004/10/23
- [ ] `gem.c` `mail.c` `post.c` `xpost.c` 閱讀到到一半時可以按 <kbd>E</kbd> 編輯文章。
- [ ] `more.c` 自動播放改為只限定在單一篇文章。
- [ ] `bbslink.c` `bbsnnrp.c` 將送信、抓信合併為單一程式。

## 2004/10/25
- [ ] `struct.h` `admutil.c` 註冊單刪除真實姓名欄位。

## 2004/10/27
- [ ] `lottery.c` `lottery-open.c` `etc/game/lottery.main` `etc/game/lottery.rule` 拿掉樂透遊戲。

## 2004/10/28
- [ ] `struct.h` `admutil.c` `rec_article.c` 擋信規則增加 `SITE`。
- [ ] `struct.h` `global.h` `bank.c` 支票匯款。

## 2004/11/01
- [ ] `vote.c` 立即開票。
- [ ] `vote.c` `account.c` `struct.h` 提供賭盤功能。

## 2004/11/10
- [ ] `struct.h` `cache.c` `account.c` `bno` 依板名排序，加速看板搜尋。

## 2004/11/13
- [ ] `cache.c` `menu.c` 動態看板改為完全隨機播放，而不是循序隨機播放。

## 2004/11/14
- [ ] `more.c` 更新閱讀文章時，按 <kbd>Up</kbd>/<kbd>PgUp</kbd> 上捲的程式。

## 2004/11/16
- [ ] `mail.c` `account.c` 記錄站內寄信，並將站內/站外寄信的記錄放在 log 板。

## 2004/11/19
- [ ] `struct.h` `admutil.c` `rec_article.c` 擋信規則增加 `POSTHOST`。

## 2004/11/25
- [ ] `admutil.c` `rec_article.c` 擋信規則可各板特別訂定。

## 2004/11/29
- [ ] `redir.c` 精華區 `.DIR` 重建程式。

## 2004/12/10
- [ ] `rec_bot.c` `*.c` 重要文章置底。

## 2004/12/11
- [ ] `cache.c` 修正從朋友名單加入/移除線上使用者時，自己的使用者名單不會變色。

## 2004/12/23
- [ ] `config.h` `board.c` `HAVE_MMAP` 變成始終 `#define`。

## 2004/12/26
- [ ] `more.c` 修正 <kbd>Home</kbd> 會發生錯誤。

## 2004/12/30
- [ ] `admutil.c` 轉信設定增加搜尋的功能。

## 2004/12/31
- [ ] `more.c` 改善在閱讀長篇文章時上捲速度很慢的問題。

## 2005/01/01
- [ ] `bbsd.c` 刪除最後一個重複 login 後變成本尊。

## 2005/01/08
- [ ] `global.h` `acct.c` `mail.c` `xover.c` `enews.c` 將錯誤 E-mail address 的訊息統一成 `ERR_EMAIL`。
- [ ] `mail.c` 從站上寄信給 `xyz.brd@mydoamil` 反而會寄給 `xyz` 這 ID。

## 2005/01/11
- [ ] `manage.c` `post.c` 新增看板屬性：不能評分。
- [ ] `admutil.c` 認證過期前十天可填註冊單。
- [ ] `acct.c` 個人上站紀錄檢索。
- [ ] `xyz.c` 忘記密碼服務。
- [ ] `etc/cross-post` `global.h` `post.c` Cross-Post 停權寄信通知。

## 2005/02/28
- [ ] `enews.c` `enews-open.c` 移除奇摩新聞。

## 2005/03/08
- [ ] `global.h` `edit.c` `post.c` `more.c` 把 `QUOTE_CHAR` 統一在一起。

## 2005/03/09
- [ ] `bhttpd.c` 將信箱列表改為一次只印出五十封信。

## 2005/03/13
- [ ] `vote.c` 刪除賭盤時會退換賭金。

## 2005/03/16
- [ ] `config.h` `global.h` `board.c` `account.c` `usies-sort.c` 移除 `LOG_BRD_USIES`。

## 2005/03/23
- [ ] `board.c` `bhttpd.c` `pal.c` 看板壞人。

## 2005/03/28
- [ ] `board.c` `pal.c` `ulist.c` `xover.c` 使搜尋中文關鍵字時不會出錯。

## 2005/03/29
- [ ] `acct.c` 修改看板板名時，`@Class` 會一併變動。

## 2005/04/05
- [ ] `innbbs.c` `admutil.c` 將轉信設定獨立成 `innbbs.c`。
- [ ] `acct.c` `admutil.c` 將站務指令集中到 `admutil.c`。
- [ ] `acct.c` `user.c` 將使用者指令集中到 `user.c`。
- [ ] `struct.h` `user.c` `admutil.c` 使用者自行復權。

## 2005/04/13
- [ ] `innbbs.c` `channel.c` `bbslink.c` `inntobbs.c` `rec_article.c` 擋信規則可侷限擋某特定新聞伺服器的信。

## 2005/04/14
- [ ] `gem.c` 修正精華區複製會斷線的問題。

## 2005/04/20
- [ ] `bwboard.c` 修正軍棋炮/包行走方式。
- [ ] `bwboard.c` 暗棋按照第一個翻子決定顏色。

## 2005/04/23
- [ ] `manage.c` 拂楓落葉斬可指定可砍(不)轉信板。
- [ ] `post.c` 板主在管理的看板中轉錄不算跨貼。
- [ ] `vote.c` 賭盤的票價由板主來決定。

## 2005/04/24
- [ ] `innbbs.c` `innbbsd/*` active 檔的設定搬入 `newsfeeds.bbs` 內。

## 2005/05/17
- [ ] `struct.h` `cache.c` `menu.c` `camera.c` 將節日搬到 `fshm`。
- [ ] `camera.c` `etc/feast` 支援某月第幾個星期幾為節日的格式。

## 2005/05/18
- [ ] `xpost.c` 修正同標題搜尋要標題完全相同才 match。

## 2005/05/19
- [ ] `*.h` `*.c` 將看板長度從 `IDLEN` 獨立成 `BNLEN`。

## 2005/05/28
- [ ] `camera.c` `etc/feast` 支援農曆節慶。

## 2005/06/04
- [ ] `perm.h` `*.c` 把站內寄信的權限獨立成 `PERM_LOCAL`。

## 2005/06/06
- [ ] `config.h` `bbsd.c` `user.c` `admutil.c` 將 `INVALID_NOTICE_PERIOD` 整理出來。

## 2005/06/09
- [ ] `theme.h` `edit.c` `bhttpd.c` 把 `BANNER` 整理出來。
- [ ] `struct.h` `board.c` `vote.c` `account.c` 看板列表顯示該看板有賭盤進行中。

## 2005/06/11
- [ ] `topusr.c` 排行榜允許同名次。

## 2005/06/13
- [ ] `struct.h` `board.c` `post.c` `bbsd.c` 看板人氣。

## 2005/06/17
- [ ] `bbsd.c` `game/*.c` 遊戲亂數選取。

## 2005/06/25
- [ ] `modes.h` `perm.h` `gem.c` 精華區權限與看板權限同步。

## 2005/06/26
- [ ] `*.c` 精華區不提供 Gopher。

## 2005/06/30
- [ ] `*.c` 將 `xsort` 改用 `qsort`。

## 2005/07/01
- [ ] `bnntpd.c` `BNNTPD` 改為 daemon。

## 2005/07/09
- [ ] `bhttpd.c` `BHTTPD` 改為 daemon。

## 2005/07/12
- [ ] `xover.c` `bmw.c` 將 `bmw_cb` 直接寫到 `xz[]`。
- [ ] `xover.c` `pal.c` `manage.c` `vote.c` 將 `pal_cb` 直接寫到 `xz[]`。
- [ ] `modes.h` `pal.c` `manage.c` `vote.c` 區分朋友/群組/板友名單的提示字樣。

## 2005/07/16
- [ ] `post.c` `cache.c` 就算沒有 #define `CHECK_ONLINE`，也可以丟文章/信件作者水球。
- [ ] `pal.c` `aloha.c` 朋友名單/上站通知名單中，若該使用者在站上會亮色。
- [ ] `pal.c` 修正改群組名單/板友名單/限制投票名單時會錯誤加上 `STATUS_PALDIRTY`。

## 2005/08/10
- [ ] `bhttpd.c` 大幅改版。

## 2005/08/14
- [ ] `cache.c` 修正使用者不在線上，但在列表中卻亮色。

## 2005/08/26
- [ ] `visio.c` 改寫螢幕控制部分。

## 2005/08/31
- [ ] `*.c` 將所有換大小寫的程式統一。

## 2005/09/01
- [ ] `inntobbs.h` `inntobbs.c` `rec_article.c` Organization 不列為必要檔頭。

## 2005/10/07
- [ ] `*.c` 站長行為記錄。

## 2005/10/08
- [ ] `admutil.c` 還原備份。

## 2005/10/15
- [ ] `config.h` `innbbsd/*` 把 `BBSNAME2`、`TAG_VALID`、`bbsname.bbs` 整合在一起。

## 2005/10/19
- [ ] `visio.c` `talk.c` `chat.c` `bwboard.c` 修正 `vio_fd` 與 mysql 命名衝突。

## 2005/10/20
- [ ] `post.c` 從秘密看板轉錄出去的文章不顯示來源看板。

## 2005/11/17
- [ ] `xover.c` 修正在文章列表按 <kbd>^Z</kbd> 選看板時，會多做一次 `XoPost()`。

## 2005/11/18
- [ ] `more.c` 捨棄 buffered file read 的方式，而採 full file read。

## 2005/11/23
- [ ] `bmw.c` 提供刪除所有水球的功能。

## 2005/12/09
- [ ] `bbsnet.c` `connlist` 移除 BBSNET。

## 2005/12/25
- [ ] `acct.c` `manage.c` 板主名單輸入同樣 ID 時可移除該 ID。

## 2006/01/23
- [ ] `camera.c` `etc/feasts` 特殊節日的開頭畫面。

## 2006/02/06
- [ ] `xpost.c` `post.c` `mail.c` `xover.c` 串列搜尋允許增加條件再度搜尋。

## 2006/02/08
- [ ] `struct.h` `innbbs.c` `bbslink.c` 在 newsfeeds 裡面顯示找不到群組。

## 2006/03/24
- [ ] `board.c` `vote.c` 修正看板人氣計算錯誤。

## 2006/03/30
- [ ] `bhttpd.c` 提供 robot exclusion。
- [ ] `ats2*.c` 提供 ATS 轉換程式。

## 2006/04/10
- [ ] `board.c` 在看板列表將看板加入我的最愛不會重覆加入。
- [ ] `visio.c` `vget()` 時按空白鍵儘量補完板名。

## 2006/04/17
- [ ] `camera.c` 限制動態看板每列長度。

## 2006/04/20
- [ ] `more.c` 修正文章閱讀到一半時，進出 help 會錯誤。
- [ ] `str_ttl.c` `post.c` 將轉錄字樣改成 `Fw:`。

## 2006/05/10
- [ ] `visio.c` `edit.c` 支援漢字整字刪除、移動、顯示。

## 2006/05/12
- [ ] `chat.c` 提供 `KEY_DEL` 刪字功能。
- [ ] `talk.c` `chat.c` 支援漢字整字刪除、移動、顯示。

## 2006/05/21
- [ ] `railway.c` 移除鐵路時刻查詢。

## 2006/05/24
- [ ] `gem.c` 精華區增加分隔線。

## 2006/05/29
- [ ] `vote.c` 投票限制登入/發文次數。

## 2006/06/05
- [ ] `gem.c` 將 tag 符號位置與加密符號位置對調，使與文章列表相同。
- [ ] `gem.c` 修正精華區可能發生文章和卷宗 `chrono` 相同的問題。
- [ ] `gem.c` `post.c` `mail.c` 可以將加密文章複製貼到精華區。

## 2006/10/13
- [ ] `favor.c` `board.c` 我的最愛可以收整個分類群組。

## 2006/12/06
- [ ] `acct.c` 看板編輯可異動板名大小寫。

## 2006/12/26
- [ ] `board.c` 看板全域搜尋可中斷搜尋。

## 2007/04/05
- [ ] `mail.c` 修正打包資料時信件的檔頭。

## 2007/04/26
- [ ] `post.c` 當帳號被清除後，新註冊相同 ID 的帳號並不擁有過去該 ID 發表的文章之所有權。

## 2007/05/09
- [ ] `bbsmail.c` `brdmail.c` 改用 `str_from()`。

## 2007/05/17
- [ ] `visio.c` `talk.c` 將 `b_cols` 的定義從每列字數改為游標最大可及處 (和 `b_lines` 相同)。
- [ ] `struct.h` `edit.c` `talk.c` `camera.c` 重新整理 `SCR_WIDTH` 相關程式。
- [ ] `more.c` 將 `more()` 的寬度放大到與 telnet term 螢幕寬度相同。

## 2007/05/18
- [ ] `struct.h` 加大 `ANSILINELEN`。

## 2007/06/09
- [ ] `xover.c` `xo_cursor()` 加入可直接輸入位置跳去該篇的功能。

## 2007/10/16
- [ ] `acct.c` 避免多位站務重覆開板。

## 2007/11/18
- [ ] `reversi.c` 將 `gray.c` 正名。

## 2011/08/23
- [ ] `user.c` 修正 pop3 信箱認證 timeout。

## 2012/06/18
- [ ] `*.c` 修正數個潛在的記憶體存取問題。

## 2012/10/19
- [ ] `*/Makefile` 修正錯誤。
- [ ] `*.c` 清掉部分語法錯誤。

## 2012/10/21
- [ ] `expire.c` `reaper.c` `config.h` 設定搬去 `config.h`。
