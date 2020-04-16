```
註： 以下文件乃編改自清大楓橋驛站 Maple BBS 3.02 版之使用手冊。
MANIFEST Guide                                                 [WindTopBBS]
=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

                          WindTopBBS Ver 3.02

                             系統相關檔案

               Modified by 元智資工  沈俊興(statue.bbs@bbs.yzu.edu.tw)

=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

                        目      錄

                (一) bbs 的家之下

                (二) brd/ 每個看版目錄下

                (三) etc/ 所有的管理相關檔

                (四) gem/ 所有精華區

                (五) innd/ 轉信相關檔

                (六) log/ 歷代記錄備份處

                (七) run/ 現在記錄所在

                (八) usr/ 所有使用者

                (九) doc/ 相關說明文件

─────────────────────────────────────
(一) bbs 的家之下
─────────────────────────────────────

 .BRD  看版檔, 包含看版所有屬性, 此檔即為 board shm 中的資料

 .USR  user id檔, 用來管理 user id 和 user unique no的 mapping
       user本身資料則分散至各user目錄下的 .ACCT檔

     bbs的家之下主要有幾個目錄：

             2nd/ 二手市場相關檔案

             bin/ 所有bbs相關的執行檔

             brd/ 所有的看版

             etc/ 所有的管理相關檔

             gem/ 所有精華區

             innd/ 轉信相關檔

             log/ 歷代記錄備份處

             mailservice/ 電子郵件服務暫存目錄

             net/ 絲路資料存放區

             run/ 現在記錄所在

             tmp/ 暫存檔區 (ex. zmodem傳檔)

             usr/ 所有使用者

             usr/ 所有使用者

             doc/ 相關說明文件

     這幾個目錄，都可以在不同的partition，用 symbolic link 方式放在
     bbs 的家下，經過有效的放置，可以有效分散 I/O 至不同的硬碟上，
     在 inode 的數量上，也可有適當控制。

     建議 brd 可與 gem 放在同一顆，usr 另外放一顆, 平行效果甚佳

     在規畫 inode 時，brd/gem 那顆可用 -i 2048，usr 那顆強烈建議 -i 1024 :p


─────────────────────────────────────
(二) brd/ 每個看版目錄下
─────────────────────────────────────

   note         進版畫面
   .DIR         版上文章目錄檔
   fimage       秘密版user名單 image檔
   friend       秘密版user名單

─────────────────────────────────────
(三) etc/ 所有的管理相關檔
─────────────────────────────────────

   2ndhand_decl      二手市場公告事項
   2ndhand_help      二手市場線上求助
   @/                舊紀錄
   Version           風之塔版本宣告
   admin.db          超級站務列表資料庫
   allow.acl         白名單
   announce          進站宣告
   approved          通過身分認證的通知信(註冊單方式)
   badid             禁用的id
   banmail.acl       系統檔信列表資料庫
   bbs.acl           禁止上站位置
   chat.condition.db 第三類聊天室動詞
   chat.party        第一類聊天室動詞
   chat.parth2       第四類聊天室動詞
   chat.speak        第二類聊天室動詞
   checkmail.mail    當註冊資料更新收集完畢時, 會將此封信寄給你
   confirm           認證信函
   counter           系統歷史軌跡
   crontab           風之塔目前的 crontab backup
   e-mail            修改 E-mail address，進行身分認證的說明檔
   expire.conf       看版文章expire的相關參數, 含保留天數, max,min
   justified         通過身分認證的通知信(E-mail reply方式)
   justify           E-mail reply 身分認證的方式說明
   lovepaper.txt     情書產生器資料庫
   mail.acl          供 bmtad 擋垃圾信及禁止 e-mail reply的黑名單
   mail.over         信件過量禁止信箱操作的說明
   mailservice       電子郵件服務
   mquota            過期信件的砍除通知
   newuser           新手上路須知(新手上路,三天後開放權限)
   notify            未通過身分認證的警告
   pip/              風之塔戰鬥雞相關mob設定檔
   re-reg            半年身分認證到期, 通知重新認證的警告
   same_email.acl    相同的 email 資料庫, 由 util/stopperm.c 產生
   stop.log          自動處罰程式暫存檔
   stopperm.mail     自動處罰程式處理完畢會寄此封信給你
   summer.mail       暑假期間及將來的 <帳號、信件> 管理辦法
   sysop             Y)es, Sir! 的 sysop及權責畫分名單
   valid             身分認證信函
   violatelaw.acl    自動處罰程式名單中暫時禁止註冊之 email 資料庫


─────────────────────────────────────
(四) gem/ 所有精華區
─────────────────────────────────────

   brd/         所有看版精華區
     ActiveInfo/@/@note 動態看版類別, 在此類別中的所有文章
                        將自動被 camera 收錄至 movie shm
   @/
     @-act      今日上站人次統計, account產生
     @=act      昨日上站人次統計, account產生
     @-day      本日十大熱門話題, poststat產生
     @-week     本週熱門話題, poststat產生
     @-month    本月熱門話題, poststat產生
     @-year     年度熱門話題, poststat產生
     @Class     C)lass的主類別, 由此分出Class的目錄,
                account需用此產生 class.img, 再由user login時載入
     @NewBoard  新增看版類別, 新增時會自動詢問是否放入此類別中
     @Profess   P)rofess的主類別, 由此分出 Profess的目錄,
                account需用此產生 profess.img, 再由user login時載入

     以下檔案會被 camera 收錄至 movie shm
     @admin.hlp   系統管理員操作說明
     @aloha.hlp   上站通知名單操作說明
     @banmail.hlp 擋信列表操作說明
     @bmw.hlp     熱訊列表說明
     @board.hlp   看版操作說明
     @class.hlp   群組操作說明
     @contact.hlp 聯絡名單操作說明
     @edit.hlp    編輯器說明
     @friend.hlp  好友名單說明
     @gem.hlp     精華區說明
     @mbox.hlp    個人信箱說明
     @mime.hlp    元智校園學生郵件系統說明
     @more.hlp    瀏灠閱讀說明
     @signup.hlp  連署系統操作說明
     @song.hlp    點歌系統操作說明
     @ulist.hlp   網友列表說明
     @vote.hlp    投票操作說明

     @apply       申請新帳號畫面
     @bye         離站再見畫面
     @error-camera 錯誤 camera 時畫面
     @income      進站畫面
     @post        文章發表綱領
     @tryout      login錯誤的結束登錄畫面
     @welcome     進站歡迎畫面

     此外, 每個 board的gem, 或是gem目錄下,
     @/@index     精華區索引檔, 由 gem-index 產生
     @/@log       精華區異動檔, 線上編輯精華區時產生
     .DIR         精華區主目錄檔
     .GEM         資源回收筒


─────────────────────────────────────
(五) innd/ 轉信相關檔
─────────────────────────────────────

     history*
     bbsname.bbs
     innbbs.log
     newsfeeds.bbs
     nodelist.bbs  以上見 doc/README.INNBBSD 相關說明
     out.bntp      轉出文章檔
     cancel.bntp   轉出cancel message檔


─────────────────────────────────────
(六) log/ 歷代記錄備份處
─────────────────────────────────────

     mail-??????.gz     mail.log檔
     maple-??????.gz    maple source檔
     reaper-??????.tgz   久未上站被清出的帳號壓縮檔
     usies-??????.gz    usies檔
     util-??????.gz     util source檔


─────────────────────────────────────
(七) run/ 現在記錄所在
─────────────────────────────────────

     anonymous.log  匿名文章的 log
     banemail.log   擋信紀錄
     bbs.pid        bbsd的process id檔
     bbs.usage      bbsd的usage情況 (目前停用)
     bguard.log     bguard的log
     bguard.pid     bguard的process id
     bmta.log*      bmtad的log
     bmta.pid       bmtad的process id
     bquota.log     bquota的log (目前停用)
     camera.log     camera的log
     chat.log       xchatd的log
     chat.pid       xchatd的process id
     class.img      C)lass的image檔, account產生, bbsd載入
     expire.log     expire的log
     gcheck.log     gem-check的log
     gemd.log       gemd的log
     gemd.pid       gemd的process id
     innbbsd*.pid   innbbsd的process id
     lazybm.log     偷懶版主的log
     mail.log       bbsd寄信的log
     manager.log    特殊權限user的log, 由 reaper產生
     match.log      ID, 真實姓名, Email 對映檔
     note.all       留言版全部
     note.pad       留言版儲存檔
     pal.log        好友名單超長名單, 由 bbsd/talk.c的pal_cache()產生
     pass.log       密碼修改的log
     pip.log        小雞的log
     pipmoney.log   小雞金錢的log
     pop3.log       bpop3d的log
     pop3.pid       bpop3d的process id
     pop3mail.log   pop3mail的log
     post.db        post統計檔, 由 bbsd/edit.c產生
     profess.img    profess的image檔
     reaper.log     reaper的log
     spam.log       spam的log
     stop.log       停權的暫存檔
     userno.log     userno的log, 由 util/userno.c 產生
     usies          bbs usies檔
     usies=         累計usies檔
     var/           統計用暫存目錄
       act          account暫用
       post.author  poststat統計作者用
       post.old.db  同上
       day.*
       week.*
       month.*
       year.*       poststat統計用


─────────────────────────────────────
(八) usr/ 所有使用者
─────────────────────────────────────

 usr/ 所有使用者, 下再分 a-z子目錄, 以user第一字區分
     @/放被 reaper砍除的id


 每個user的子目錄/
   .ACCT           使用者資料
   .BRH            已閱讀文章之範圍
   .DIR            信箱資料
   @/              信箱子目錄
   acl             容許上站地點, SYSOP權限專用, 全小寫
   aloha           上站通知
   bmw             熱訊記錄
   benz            系統協尋好友上站
   chicken         小雞紀錄檔
   buf.*           暫存檔
   chicken         小雞
   email           email reply認證記錄
   friend          好友檔
   frienz          上站通知好友設定檔
   justify         保留認證方式
   log             上站記錄
   logins.bad      密碼錯誤紀錄
   newboard        連署系統紀錄檔
   note            寄信備份用
   p_note.ans      答錄機中新的流言
   p_note.ans.save 答錄機保留下來的留言
   plans           名片檔
   pnote.dat       答錄機資料庫新的流言
   pnote.tmp       答錄機資料庫新的流言暫存檔
   pnote.dat.save  答錄機資料庫保留下來的流言
   sign            簽名檔
   stopperm.log    個人停權紀錄
   vote            投票資料

─────────────────────────────────────
(九) doc/ 相關說明文件
─────────────────────────────────────

 doc/ 相關說明文件
    FAQ                 常見問題收集
    FEATURE             WindTop BBS 3.02 風之塔的特色
    MAPLE3.FEATURE      [M3] MapleBBS特色 --系統效能觀點
    INSTALL             WindTop BBS 3.02 快速安裝文件
    README.ADMIN        WindTop BBS 3.02 系統管理者使用手冊
    README.BM           WindTop BBS 3.02 版主使用手冊
    README.CRONTAB      WindTop BBS 3.02 crontab 範例
    README.MANIFEST     WindTop BBS 3.02 系統相關檔案
    README.SONG         WindTop BBS 3.02 點歌系統使用手冊
    README.SYSOP        WindTop BBS 3.02 系統操作者使用手冊
    README.TAG          WindTop BBS 3.02 標籤Tag說明
    README.UTIL         WindTop BBS 3.02 外部工具程式  簡易版
    SITE                目前使用 WindTop BBS 的站台
    sendmail.cf         8.9.3 的 sendmail.cf 設定檔

以上是所有特殊檔案的列表

--
                                                    元智資工 沈俊興
                                        E-Mail: <statue.bbs@bbs.yzu.edu.tw>
```
