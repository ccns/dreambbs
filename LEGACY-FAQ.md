```
註： 以下文件乃編改自清大楓橋驛站 Maple BBS 3.02 版之使用手冊。
Frequently Asked Question                                      [WindTopBBS]
=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

                          WindTopBBS Ver 3.02

                             常見問題回答

               Modified by 元智資工  沈俊興(statue.bbs@bbs.yzu.edu.tw)

=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

Q.  簽名檔只有一個 ?

A.  每六行為一個簽名檔的單位.

> -------------------------------------------------------------------------- <

Q.  how to debug *.so ?

A.  edit Makefile
    add test.c to SRC
    add test.o to OBJ
    del test.so from SO

> -------------------------------------------------------------------------- <

Q.  make[1]: *** No rule to make target `echobbslib.c',
      needed by `echobbslib.o'.
    Stop.
    make[1]: Leaving directory `/home/bbs/src/inbbsd'
    make: *** [linux] error 2

A.  make link linux install

> -------------------------------------------------------------------------- <

Q.  make[1]: bison: Command not found
    make[1]: *** [parsedate.c] Error 127
    bison 是什麼碗糕啊?

A.  FreeBSD 4.0 以後內訂不安裝.. 要自己裝
    fetch ftp://140.138.3.243/pub/bison-1.28.tar.gz
    tar zxvf bison-1.28.tar.gz
    cd bison-1.28
    ./congifure
    make
    make install

> -------------------------------------------------------------------------- <

Q.  新開看板時有沒有該注意的地方？

A.  如果是站內版記得開不轉信，這樣能減輕 bbslink 的負擔。

> -------------------------------------------------------------------------- <

Q.  如何更新 source ？

A.  目前還沒有像 FireBird 的 CVS 機制，所以只能看看新的 source 然後
    自己慢慢改。

> -------------------------------------------------------------------------- <

Q.  修改新功能後，要如何讓新進來的 BBS user 看到改好的東西？
    有沒有啥該注意的？

A.  如果是修改新的功能，建議以其他的 port 來測試，例如使用
        bin/bbsd 3456
    這樣 telnet 0 3456 就可以去檢查自己剛剛修改好的程式部分。
    這樣改錯也不會有其他使用者知道，另外在 run/bbs.pid 的最後一行會有
        27247   05/Jul/2000 20:05:01    3456
    當測試完後就可以用
        kill `tail -n -1 /home/bbs/run/bbs.pid | awk '{print $1}'`
    或是直接用 kill 27247 來關閉剛剛開的 test port
    如果測試沒問題的話，就去 run/bbs.pid 中尋找最後一個
        25329   05/Jul/2000 17:22:23    23      inetd -i
    然後 kill 25329 這樣就可以讓你剛剛改好的功能讓新進來的 user
    看到了。

> -------------------------------------------------------------------------- <

Q.  sendmail & bmtad ?

A.  皆為處理信件之 daemon，同樣都是 bind 住 port 25
    一般 UNIX 系統皆是用 sendmail 來處理信件，但是對於一個
    BBS 系統來說，sendmail 過去的歷史讓系統相當不安全，為了
    讓被侵入的危險降至最低，MapleBBS 的開發者自己寫了一個類似
    Sendmail 的 daemon，專門用來處理 BBS 信件，當然功能比不上
    Sendmail 囉，而原來的系統帳號也不能收信。但是擋信的部分就
    必須自己加上了。

    bmtad: Mail Transport Agent Daemon for BBS

    [1]sendmail安裝：
             1.將 bbs mail 的 rule 加到 sendmail.cf 中。
               並編譯 bbsmail 與 mailpost 與 bbsmail 。
               bbsmail 用來收取 Internet 的信件，而
               mailpost 用來貼文章到看板上，可隨需求選擇加或不加。
               ※doc/sendmail.cf 為 8.9.3 的範例檔
    (1) 在 [S0]、[# handle local hacks] 的後面，加上 :
Rbbs < @ $=w . >        $#mailpost $: bbs               mailpost for bbs
R$+.bbs < @ $=w .>      $#bbsmail $: $1                 bbs mail gateway
                  ^^^^^^               ^^^^^^^^^^^^^^^^^
                  [Tab]                     [Tab]

    (2) 在 [# everything else is a local name] 的後面，加上
Rbbs                    $#mailpost $:bbs                mailpost for bbs
R$+.bbs                 $#bbsmail $:$1                  bbs mail gateway

    (3) 在 [#   Local and Program Mailer specification] 的後面加上
Mmailpost,[Tab] P=/home/bbs/bin/mailpost, F=lsSDFMhPu, U=bbs, S=10, R=20/40,
[Tab]   [Tab]   A=mailpost $u
Mbbsmail,[Tab]  P=/home/bbs/bin/bbsmail, F=lsSDFMhPu, U=bbs, S=10, R=20/40,
[Tab]   [Tab]   A=bbsmail $u

    設定 sendmail.cf 時要特別注意 TAB/space 的區別，
    可以先寫進 sendmail.test，用 sendmail -Csendmail.test -bt
    測試看看 mailer rule parsing 的結果對不對。
             2.編輯 /etc/rc.conf (for FreeBSD)
               sendmail_enable="YES"
               sendmail_flags="-bd -q30m"
             3.重新啟動 sendmail
               kill -9 `cat /var/run/sendmail.pid`
               sendmail -bd -q30m
             4.測試 sendmail
               sendmail -C/etc/sendmail.cf -bt
ADDRESS TEST MODE (ruleset 3 NOT automatically invoked)
Enter <ruleset> <address>
> 0 bbs
rewrite: ruleset  0   input: bbs
rewrite: ruleset 98   input: bbs
rewrite: ruleset 98 returns: bbs
rewrite: ruleset  0 returns: $# mailpost $: bbs
> 0 SYSOP.bbs
rewrite: ruleset  0   input: SYSOP . bbs
rewrite: ruleset 98   input: SYSOP . bbs
rewrite: ruleset 98 returns: SYSOP . bbs
rewrite: ruleset  0 returns: $# bbsmail $: SYSOP
             5.擋信機制，修改 sendmail.cf 或與 tcp_wrappers 結合。
                 參考網頁:http://www.sendmail.org/m4/anti-spam.html
                 在此僅介紹修改 sendmail.cf
a.首先當然是/etc/sendmail.cf得要有 LOCAL_RULESETS 設定
  沒有也沒關係，自行加入sendmail.cf也可以,要不然
  用/usr/src/contrib/sendmail/cf/cf/knecht.mc 與m4指令
  加入也可以,詳細操作不在此贅述,有興趣的再發問吧!!...
b.到/etc/mail/access(沒有??.自己造一個吧)下編輯你想ban掉
  的 Mail server可以是Domain Name，IP，E-Mail Address都可以
  編輯格式如下:

123.456.789     REJECT      #ban123.456.789網域下所有E-Mail Server
ncku.edu.tw     REJECT      #banncku.edu.tw網域下所有E-Mail Server
abc@ncku.edu.tw     REJECT  #只對abc由ncku.edu.tw進來的信擋住
abc@            REJECT      #ban掉所有以abc名義寄出的信(?)
        ^^^^^
        [TAB]

c.直接在/etc/mail/底下執行make
  當然前提是要有/etc/mail/Makefile
/etc/mail/Makefile 內容(借花獻佛一下!!...)
#       $Id: Makefile,v 1.5.2.2 1999/05/11 03:20:44 jmb Exp $
install:
        /usr/sbin/makemap hash /etc/mail/access < /etc/mail/access
all: install
  接著你會看到access.db(Database)

上面/etc/mail/access 寫REJECT位置的地方,其實不只有REJECT可以使用!!...

可供選擇的一些參數如下:

REJECT :只要合乎IP或Domain Name 一律拒絕
RELAY  :名單上合乎的IP或Domain Name,若不是回本站發出的信,就拒絕
        換句話說:以bbs認證為例...
        ncku.edu.tw 被處罰不得寄信給本站User,但為了顧及其他人認證權力
        於是 ncku.edu.tw           REPLY 產生
OK     :完全接受該IP或Domain Name
DISCARD:看不懂
        Discard the message completely using the $#discard mailer.
        This only works for sender addresses (i.e., it indicates that you
        should discard anything received from the indicated domain).
### text:它也可以允許在RFC 821相容的錯誤碼後,輸入自己的原因和文字
         (中文可不可行??...還沒試...不知道!!)
         例如:
         ncku.edu.tw        550 No!! You are dead!!....

利用/etc/sendmail.cf也可以設定,ban掉某一些信件標題....
請自行參考 http://www.sendmail.org/m4/anti-spam.html

或是在 sendmail.cf 中的 local info 的 section 加入下列兩行:

F{BU} /etc/sendmail.baduser
F{BD} /etc/sendmail.baddomain

        然後, 在 ruleset 98 的最後, 加入 check_mail 這個 ruleset:

Scheck_mail
# reject spam user
R<$={BU}>               $#error $@ 5.7.1 $: "571 we don't accept junk mail"
R$={BU}                 $#error $@ 5.7.1 $: "571 we don't accept junk mail"
# reject spam domain
R$*                     $: $>3 $1
R$*<@$={BD}.>$*         $#error $@ 5.7.1 $: "571 we don't accept junk mail"
R$*<@$={BD}>$*          $#error $@ 5.7.1 $: "571 we don't accept junk mail"
# reject subdomain of bad domain
R$*<@$*.$={BD}.>$*      $#error $@ 5.7.1 $: "571 we don't accept junk mail"
R$*<@$*.$={BD}>$*       $#error $@ 5.7.1 $: "571 we don't accept junk mail"

        (ps. 廢話一下, ruleset 的 LHS 跟 RHS 是用 [Tab] 隔開的)
        假設不想收來自 joe@foo.bar 的信, 就可以把 joe@foo.bar
        放入 /etc/sendmail.baduser 檔中:

%cat /etc/sendmail.baduser
joe@foo.bar

        假設不想收來自 foo.bar 這個 domain 的信, 就把 foo.bar 放
        入 /etc/sendmail.baddomain 檔中:

%cat /etc/sendmail.baddomain
foo.bar

        每改完 /etc/sendmail.bad* , 把 sendmail 重叫, 那新進來的連線
        就會受到影響了.

        要說明一下的是, 這邊 check 的 from 是 SMTP 的 "mail from: "
        所看到的 (有的 OS 在 /var/log/syslog 中有記載), 並非信中
        的 "From: " 的內容. 擋錯了就沒效了.

        資料來源: http://www.informatik.uni-kiel.de/%7Eca/email/check.html
             6.認證機制，修改 sendmail.cf 或修改 src/maple/mail.c
                 a.修改 sendmail.cf，增加 bbsreg 那兩行即可。
# handle local hacks
R$*                     $: $>98 $1
Rbbs < @ $=w . >        $#mailpost $: bbs               mailpost for bbs
Rbbsreg < @ $=w . >     $#mailpost $: bbsreg            mailpost for bbs
R$+.bbs < @ $=w .>      $#bbsmail $: $1                 bbs mail gateway

# everything else is a local name
Rbbs                    $#mailpost $:bbs                mailpost for bbs
Rbbsreg                 $#mailpost $:bbsreg             mailpost for bbs
R$+.bbs                 $#bbsmail $:$1                  bbs mail gateway
                 b.修改 src/maple/mail.c
                     把 mail.c 的 bbsreg@ 改成 bbs@ 就可以了
             7.測試 mailpost
      寫一篇測試用的 mail 叫 email_test, 檔頭一開始是

       #name: user-id
       #password: user-password
       #board: board-name
       #title: article-tile
       #localpost:           <---如果有加這行此篇文章便不轉信出去
       (空一行)
       ...............
       ...............

      然後 mail bbs@your-domain-name < email_test 看看 E-mail post 是否正常

    [2]bmtad安裝：
             bmtad 有自己寫好的認證與擋信機制。
             1.將 src/bmtad 安裝。
               cd src/bmtad
               make sys-type install
             2.編輯 /etc/rc.conf (for FreeBSD)
               sendmail_enable="NO"
             3.關閉 sendmail
               kill -9 `cat /var/run/sendmail.pid`
             4.編輯 /etc/inetd.conf
smtp    stream  tcp     wait    bbs     /home/bbs/bin/bmtad     bmtad -i
             5.重開 inetd
               kill -1 `ps -auxwww | grep inetd | awk '{printf $2}'`

        感謝:curtain.bbs@YKLM.twbbs.org (time to say goodbye)
             fjj.bbs@virus.bio.ncku.edu.tw (亞瑟)
             opus.bbs@bbs.cs.nthu.edu.tw (獨立蒼茫)
             SoC.bbs@bbs.cs.nthu.edu.tw (Fade out!)

> -------------------------------------------------------------------------- <

Q.  bmtad在自動增加spammer到 spamer.acl 內時,常會一次加入好幾個 ?

A.  應該是因為那郭 spammer 一次發信給好幾個 user,
    通通都被擋掉了,所以會一次加入好幾個 spammer,
    從那個時間的地方就可以看出來.

        感謝:Davie.bbs@bbs.mhit.edu.tw (藍色天空－FANTASY)

> -------------------------------------------------------------------------- <

Q.  Error! shmat error! key = ooxx

A.  請執行 ipcs 指令，查看 shared memory, SHM的使用情形與 owner 是否正確
    ，正常的 owner 應該是 bbs，如果不是的話，請利用 ipcrm清除之，
    然後重新執行 bbs。

> -------------------------------------------------------------------------- <

Q.  請問一下，修改完 source 後，要如何讓新的 bbsd 重跑？

A.  將最後一個 bbs pid 砍除即可
    kill `tail -n -1 /home/bbs/run/bbs.pid | awk '{print $1}'`

> -------------------------------------------------------------------------- <

Q.  請問一下，那個個人設定中 "2 □ 新文章" 是否不能用了
    無論我是否打 ■ 進看版列表還是不會有 。‧
    而且 下站後，又變回 □ 了

A.  因為 [新文章] 模式代價很高，所以需要時按 [c] 即可，
    下站後重新 login，也不會自動進入 [新文章] 模式。

> -------------------------------------------------------------------------- <

Q.  有新信的時候不會自動default到Mail了..

A.  按 [^z] + [m] 即可。

> -------------------------------------------------------------------------- <

Q. 建議在社團板,及系板中增加"限制投票"的功能,因為此種類的看板投票的議題
   多半不是全站性的事務,而是自己系上,社內的事.且符合投票資格的人也不會太多,
   因此希望站長可以增加這樣的功能,以避免不相干人士介入投票.

A. 無法限定。

   現階段的網路投票充其量只能當作是一種「民意調查」，僅供參考，
   效力不等同於正式的投票。

> -------------------------------------------------------------------------- <

Q.  7   syltp        惡魔的女兒         u860429.SHIN78.AB. 待機
   這個狀態是什麼意思哩~?

A.  A 呼叫 (page) B，
    而 B 的畫面上出現 A 正在呼叫，等待回音，
    這時候就是「待機」模式。

> -------------------------------------------------------------------------- <

Q.  使用者名單中的"暫時變換匿稱"的功能不是暫時性的嗎....
    甚麼時後會變回原來的呢???
    我的匿稱一直到下一次上站都沒變回來.....

A.  已經不是暫時性的, 而是設定 nick name 的另一種方式了,
    似乎較簡便。

> -------------------------------------------------------------------------- <

Q.  最近我每次上站都會看到您有新信件的訊息, 可是去看又沒有新信件?

A.  進入信箱, 移至最後一封信, 按下 ` (不是 ')
    看看跳到哪封信或有什麼訊息

> -------------------------------------------------------------------------- <

Q.  請問楓橋可以解 mime 格式的信件嗎?

A.  目前不行

> -------------------------------------------------------------------------- <

Q.  我聊完天，居然那個人的動態還是在和我聊天說 :)
    更新了好像也沒用...

A.  要等他作一些別的動作:p

> -------------------------------------------------------------------------- <

Q.  為什麼我都不能用F鍵把文章轉寄到我的信箱ㄋ??

A.  未通過身分認證

> -------------------------------------------------------------------------- <

Q. 找不到認證信怎麼辦

A. 用 U)ser->A)ddress再寄一次

> -------------------------------------------------------------------------- <

Q.  不知道什麼時候～　我就不能傳訊給他～　也不能主動找他 Talk～
    但是他是可以傳訊給我～　也可以找我 Talk～
    我就是不能主動找他啦！！～
    不知道是為什麼不能ㄌㄟ？？～

A.  看看他是不是不小心把你設成損友了...:P

> -------------------------------------------------------------------------- <

Q.  現在是否沒辦法將call in 保留下來了呢???
    有些很重要的訊息想要留下來,可是現在好像沒這樣的功能了......

A.  現在在看文章的過程, 多了 C 的功能鍵, 可以將目將所看的存入暫存檔中

    如果要保留call in, 只需在 回顧熱訊時(選單or完全聊天手冊), 按下 大C 鍵

> -------------------------------------------------------------------------- <

Q.  new一個新的ID有PERM_BASIC,PERM_PAGE,
    但是在經過身分認證之後,只多了PERM_CHAT,PERM_VALID,
    少了個PERM_POST,也就是說不能post...

A.  新手上路, 見習三天...

> -------------------------------------------------------------------------- <

--
                                                    元智資工 沈俊興
                                        E-Mail: <statue.bbs@bbs.yzu.edu.tw>
```
