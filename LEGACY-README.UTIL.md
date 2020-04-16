```
註： 以下文件乃編改自清大楓橋驛站 Maple BBS 3.02 版之使用手冊。
Util Guide                                                     [WindTopBBS]
=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

                          WindTopBBS Ver 3.02

                          外部工具程式  簡易版

               Modified by 元智資工  沈俊興(statue.bbs@bbs.yzu.edu.tw)

=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

     ■ util/2nd_expire.c：跳蚤市場佈告刪除程式，由 crontab 執行 ★
                           用法: 2nd_expire

     ■ util/account.c   ：每小時上站人次統計，並進行資料壓縮備份，並重新
                           建立 class image，由 crontab 執行。
                           用法: account

     ■ util/acl-sort.c  ：sort access control list file
                           用法: acl-sort [file]

     ■ util/acpro.c     ：建立專業討論區 cache，由 crontab 執行 ★
                           用法: acpro

     ■ util/addsong.c   ：每個星期增加點歌次數，由 crontab 執行 ★
                           用法: addsong

     ■ util/backupbrd.c ：備份所有版面資料，由 crontab 執行 ★

     ■ util/backupgem.c ：備份所有精華區資料，由 crontab 執行 ★

     ■ util/backupusr.c ：備份所有使用者資料，由 crontab 執行 ★

     ■ util/bbsmail.c   ：由 Internet 寄信給 BBS 站內使用者，由 bmtad 取代

     ■ util/bquota.c    ：BBS user quota maintain & mail expire
                           風之塔改成 quota 制，不幫使用者清除信件。
                           用法: bquota

     ■ util/camera.c    ：建立動態看板 cache，由 crontab 執行
                           用法: camera

     ■ util/checkemail.c：檢查是否為核可之 Email ，由 crontab 執行 ★
                           用法: checkmail

     ■ util/clean_acl.c ：清除 mail.acl 中重複之紀錄 ★
                           用法: clean_acl [acl file]

     ■ util/counter.c   ：歷史軌跡的紀錄，由 crontab 執行 ★
                           用法: counter

     ■ util/expire.c    ：自動砍信工具程式，由 crontab 執行

     ■ util/gem-check.c ：精華區整理程式，由 crontab 執行
                           用法: gem-check

     ■ util/gem-index.c ：精華區索引程式，由 crontab 執行
                           用法: gem-index

     ■ util/hdr-dump.c  ：看板標題列表，沒用過 :p

     ■ util/mailpost.c  ：(1) general user E-mail post 到看板
                           (2) BM E-mail post 到精華區
                           (3) 自動審核身份認證信函之回信
                           由 bmtad 取代

     ■ util/mailtoall.c ：寄信給全站使用者或所有版主 ★
                           mailtoall [who] [fpath] [title]
                           who： 1 版主通告:寄給所有版主
                                 2 系統通告:寄給所有使用者

     ■ util/makefw.c    ：更新各版面的檔信列表，由 crontab 執行 ★
                           用法: makefw

     ■ util/makeusrno.c ：自動重整 userno 重複的程式
                           轉換部分[水桶名單，好友名單，熱訊紀錄]
                           好友上站通知無法轉換 ★
                           PS.請關站使用

     ■ util/match.c     ：ID, 真實姓名, Email對照表 ★
                           用法: match [userid]

     ■ util/poststat.c  ：統計今日、週、月、年熱門話題，由 crontab 執行
                           用法: poststat [day]

     ■ util/reaper.c    ：使用者帳號定期清理，由 crontab 執行
                           用法: reaper

     ■ util/resetvmail.c：清除特定已註冊之 Email 成未註冊 ★
                           用法: resetvmail (停用中)

     ■ util/restoreuser.c：將備份的資料夾中的所有使用者復原 ★
                            用法: restoreuser

     ■ util/stopperm.c  ：連座處罰程式 ★
                           用法:stopperm [userid] [vmail] [mode] [exer]

     ■ util/topusr.c    ：上站次數、灌水次數、掛站時數排行榜，
                           由 crontab 執行 ★
                           用法: topuser > gem/@/@pop.new
                                 mv gem/@/@pop.new gem/@/@pop

     ■ util/userno.c    ：檢查是否有相同的 userno ，由 account 執行 ★
                           用法: rebuilduser

    工具程式的詳細說明將會慢慢補全  :)


--
                                                    元智資工 沈俊興
                                        E-Mail: <statue.bbs@bbs.yzu.edu.tw>
```
