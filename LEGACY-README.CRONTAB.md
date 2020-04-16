```
註： 以下文件乃編改自清大楓橋驛站 Maple BBS 3.02 版之使用手冊。
註： 以下文件乃編改自中正資工 Firebird BBS 3.0 版之使用手冊。
Crontab Guide                                                  [WindTopBBS]
=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

                          WindTopBBS Ver 3.02

                             crontab 範例

               Modified by 元智資工  沈俊興(statue.bbs@bbs.yzu.edu.tw)

=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

                        目      錄

                (零) 寫在前面

                (一) 目前風之塔的 crontab 的設定如下


─────────────────────────────────────
(零) 寫在前面
─────────────────────────────────────

 [1] 簡介自動程序

    在完成基本的 BBS 架設之後，就要開始安裝 BBS 程式以外的一些應用程式，而
    這些應用程式有些可能 5 分鐘就要跑一次, 有些可能是定時每天的 xx 時 xx 分
    要執行某這些動作當然不可能每次都由我們自己來做，所以才需要由電腦定時的
    為我們執行。以下便是簡單的介紹執行自動程序的方法。

    一個描述自動程序的檔內容可能是這樣的：

    % crontab -e

0,30    8,9,10,11       *       *       *       /home/bbs/bin/account
#^^^    ^^^^^^^^^       ^       ^       ^       ^^^^^^^^^^^^^^^^^^^^^
#minute hour         monthday month  weekday    command

    此時 User 指定的自動程序就會被加入系統中。


─────────────────────────────────────
(一) 目前風之塔的 crontab 的設定如下
─────────────────────────────────────

#!/bin/sh
# minute hour monthday month weekday command

# 每小時轉信出去 6 次
8,18,28,38,48,58 * * * * innd/bbslink /home/bbs > /dev/null 2>&1

# 每小時作一次人次統計及開票
2 * * * * exec bin/account

# 每小時更新一次動態看版
43 * * * * bin/camera

# 每小時作一次話題統計, 特別疝N是在 account執行之前
50 * * * * bin/poststat

# 每天根據 expire.conf對看版文章作expire
30 3 * * * bin/expire

# 每天對精華區作 garbage collection, garbage的形成詳見精華區編輯之document
30 4 * * * bin/gem-check

# 每天對精華區作索引檔, 方便使用者搜尋
30 6 * * * bin/gem-index

# 每天對二手市場作 expire
30 7 * * 5 exec bin/2nd_expire

# 每小時作歷史紀錄
58 * * * * exec bin/counter

# 每天更新註冊資料
40 6 * * * exec bin/checkemail

# 每星期三增加點歌次數
40 4 * * 3 exec bin/addsong

# 排行榜
0 */6 * * * /home/bbs/top.sh

# 更新各版面的檔信列表
50 */2 * * * bin/makefw

# comment out for summer vacation
# 每天清理久未上站使用者, 清出的user會被搬至 usr/@目錄下, 請自行再刪除
30 2 * * 0 bin/reaper

# 每天清理個人信箱, 風之塔改成 quota 制
#10 5 * * * bin/bquota

# 每十分鐘清除一次佔用系統資源過大之 bbsd
*/10 * * * * /home/bbs/killtop.sh > /dev/null 2>&1

# 每星期天備份一次使用者、看板、精華區資料
0 5 * * 0 /home/bbs/backup.sh

# 每天備份一次 source
0 5 * * * /home/bbs/backsrc.sh


--
                                                    元智資工 沈俊興
                                        E-Mail: <statue.bbs@bbs.yzu.edu.tw>
```
