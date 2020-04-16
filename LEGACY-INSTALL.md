```
註： 以下文件乃編改自清大楓橋驛站 Maple BBS 3.02 版之使用手冊。
Quick Install Guide                                            [WindTopBBS]
=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

                          WindTopBBS Ver 3.02

                             快速安裝文件

               Modified by 元智資工  沈俊興(statue.bbs@bbs.yzu.edu.tw)

=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

                        目      錄

                (一) 建立 BBS 帳號

                (二) 取得最新版本的 WindTopBBS

                (三) 編譯程式

                (四) 安裝及完成後的測試

                (五) 其他

──────────────────────────────────────
(一) 建立 BBS 帳號
──────────────────────────────────────

1.0     以 [root] 的帳號 login。

1.1     建立 [/home/bbs] 目錄作為 BBS 的家。

1.2     在 /etc/group 中設定 bbs 的 group :

        如果是 BSD 系統，請增加：

        bbs:*:99:bbs

        如果是 Linux 系統，請增加：

        bbs:*:999:bbs

        並修改 src/include/config.h 中的

        #define BBSGID          999

1.3     如果是 BSD 系統，使用 vipw 增加下列 user :

        bbs:*:9999:99::0:0:BBS Administrator:/home/bbs:/bin/sh

        如果是 Linux 系統，請打以下的指令：

        useradd -g bbs -u 9999 bbs

        然後，修改 bbs 使用者的密碼：

        passwd bbs

1.4     設定檔案的屬性：

        chown -R bbs.bbs /home/bbs

──────────────────────────────────────
(二) 取得最新版本的 WindTopBBS
──────────────────────────────────────

2.0     以 [bbs] 的帳號 login。

2.1     Anonymous ftp://ftp-cnpa.admin.yzu.edu.tw/WindTop/
                  ftp://140.138.3.243/pub/
                  ftp://140.138.3.241/WindTop/

2.2     將所取得的程式 WindTopBBS-xxx.tar.gz 放到 [home/bbs] 底下，執行：

           gtar xvfz WindTopBBS-xxx.tar.gz  (如果使用 GNU tar)
        或
           gzip -d WindTopBBS-xxx.tar.gz | tar xvf -

──────────────────────────────────────
(三) 編譯程式
──────────────────────────────────────

3.0     更換工作目錄至 src/include，修改 config.h

3.1     更換工作目錄至 src/lib，修改 Makefile，挑選合適的 OS 設定
        執行 make

3.2     依序更換工作目錄至 src/maple src/util src/bmtad src/bgopherd
        src/bpop3d，執行 make，依說明挑選合適的 OS，然後執行
        make os-type install

3.3     若須轉信則更換工作目錄至 src/innbbsd，修改 Makefile，設定
        適當之 ADMINUSER BBSHOME BBSADDR 三欄位
        執行 make 挑選合適的 OS，完成後執行 make install

──────────────────────────────────────
(四) 安裝及完成後的測試
──────────────────────────────────────

4.0     WindTopBBS 內含 telnetd、smtpd、fingerd、gopherd、pop3d
        若要使用上述 WindTopBBS 量身訂製的 daemon，請先停止系統原有相對
        應之服務。各 service 均可以 standalone 或是 inetd 啟動。

4.1     啟動方式 inetd 或 standlone 選一個即可, 建議選以 inetd 啟動:
        以 inetd 啟動：

        ■ 修改 /etc/inetd.conf，增加如下設定，若原先已有相同的 service
           請取消原先的設定。

telnet  stream  tcp     wait    bbs     /home/bbs/bin/bbsd      bbsd -i
finger  stream  tcp     wait    bbs     /home/bbs/bin/bguard    bguard -i
pop3    stream  tcp     wait    bbs     /home/bbs/bin/bpop3d    bpop3d -i
gopher  stream  tcp     wait    bbs     /home/bbs/bin/gemd      gemd -i
smtp    stream  tcp     wait    bbs     /home/bbs/bin/bmtad     bmtad -i
xchat   stream  tcp     wait    bbs     /home/bbs/bin/xchatd    xchatd -i
bbsnntp stream  tcp     wait    bbs     /home/bbs/innd/innbbsd  innbbsd -i

        ■ 修改 /etc/services，增加下列設定

xchat           3838/tcp
xchat           3838/udp
bbsnntp         7777/tcp           usenet       #Network News Transfer Protocol
bbsnntp         7777/udp           usenet       #Network News Transfer Protocol

        ■ 以 [root] 帳號執行 kill -1 `cat /var/run/inetd.pid`
           以重新啟動 inetd

        以 standalone 啟動：

        ■ 先確定各 service 是否已有其他程式提供服務
        ■ 以 [root] 帳號執行

             /home/bbs/bin/bbsd
             /home/bbs/bin/gemd
             /home/bbs/bin/bpop3d
             /home/bbs/bin/bguard

        ■ 以 [bbs] 帳號執行

             /home/bbs/bin/bmtad
             /home/bbs/bin/xchatd

4.3     以 [bbs] 帳號執行 bin/camera; bin/account;

4.4     測試 bbs 主程式：

        telnet localhost
        打 [new] 來註冊，先以 [SYSOP] 這個 ID 來取得 BBS 站的管理權。

4.5     建立基本看板 [sysop]、[junk]、[deleted]、[ActiveInfo]，先試試 post。

    ■ 請參照 doc/README.SYSOP 的 "如何建立看板？" 一節

4.6     再 new 一個 ID 試試 talk、chat。

[註1]  'sysop' 這個字不區分大小寫。
[註2]   inetd 與 standalone 可交叉配合使用，只要不相衝突即可
        另，M3 依然可用 sendmail 搭配 mailpost/bbsmail 運作。

[註3]   若以 standalone 啟動且測試正常後，可加入 /etc/rc.local 開機自動啟動

#
# WindTopBBS
#
for i in /home/bbs/bin/bbsd /home/bbs/bin/bmtad /home/bbs/bin/bpop3d \
         /home/bbs/bin/gemd /home/bbs/bin/bguard /home/bbs/bin/xchatd ; do
       if test -x $i; then
               $i
       fi
done

if [ -f /home/bbs/innd/innbbsd ]; then
    su bbs -c '/home/bbs/innd/innbbsd'
fi

[註4]   將須手動跑的部份寫在 /etc/rc.local 開機自動啟動

#
# WindTopBBS
#
su bbs -c "/home/bbs/bin/camera"
su bbs -c "/home/bbs/bin/account"
su bbs -c "/home/bbs/bin/acpro"
su bbs -c "/home/bbs/bin/makefw"


──────────────────────────────────────
(五) 其他
──────────────────────────────────────

5.0     如何放上新程式呢?
        因為目前 WindTopBBS 已經改為 daemon 啟動，所以在重跑新 daemon 之前
        需先將原先的 parent process kill 掉。

        執行方法：

        tail /home/bbs/run/bbs.pid 找出 processid

        246     30/Jul/1998 15:05:11    3001
        268     30/Jul/1998 15:05:11    3456
        244     30/Jul/1998 15:05:11    23

        列在第一欄的就是 pid，用 [root] 帳號執行 kill -9 246 268 244
        然後再重跑 bbsd 即可。

5.1     假如您要別人不用註冊就能參觀您的站台，請用 new 建立 guest 帳號，
        如果您不想有 geust 帳號，請在 etc/badid 中加入 guest。

5.2     另外，如果您使用 FreeBSD，請注意您系統的密碼編碼方式是 MD5 還是
        DES，若沒安裝的話，請先安裝 DES ，否則使用者將無法再次上站，
        因為 MD5 編碼後的長度超過 PASSLEN 。

5.3     寫了一個 sed.sh 用來替代相關的字串，如 "元智大學", "風之塔",
        "bbs.yzu.edu.tw", "140.138.2.235" 等字串，請修改 sed.sh 中的設定
        部分後使用，請在 install.sh 前使用，。 install.sh 將以上的步驟寫
        成 script ，如果是要安裝全新的 BBS 可以試試，目前在 FreeBSD 底下
        測試正常。
        sed.sh install.sh 仍在測試中，所以............:)

--
                                                    元智資工 沈俊興
                                   E-Mail: <statue.bbs@bbs.yzu.edu.tw>
```
