
首先感謝眾前輩們無私分享這些 source code 以及不少參考文件

這邊只是就可以try成功的部分做個記錄


作業系統環境：

CentOS 5 ~ 7 32-bit 下應該都可以裝起來 ( 7 要裝 Alt-Arch 版本 )

Debian GNU/Linux 32 bit 下目前還沒有編譯成功 , 還在嘗試


大致快速安裝步驟：
(直接拿 Maple3-itoc 安裝相關文件, 針對不同的部分來修改)


1. 安裝作業系統及相關環境:

作業系統部分前面提過了,
套件部分則建議先裝好 `openssh-server` `nano`或`vim` `make` `gcc` `git` `xinetd` 等程式


2. 建立 BBS 帳號:

目前自己打算先嘗試把相關的帳號建立好, 以避免之後權限問題

於是就按照手冊該部分的方式

== 以下用 root 權限 !! ==

```
# mkdir /home/bbs
# vipw
```

我是用 Linux , 所以在最後一行加上:

```
bbs:x:9999:994:BBS Administrator:/home/bbs:/bin/bash
```

(為求保險起見跟 /etc/passwd 裡列出其他使用者的格式一樣)
(要注意 GID 是否與檔案裡其他提到的使用者重複, 以免出現權限問題)


接下來編輯 /etc/group 來增加 bbs 群組, 自己目前是用 vim 編輯
若對類似編輯器介面不熟也可用 nano 編輯

```
# vim /etc/group
```
在該檔最後一行加上:

```
bbs:x:994:bbs
```

(為求保險起見跟 /etc/group 裡列出其他使用者的格式一樣)
(要注意 GID 是否與檔案裡其他提到的使用者重複, 以免出現權限問題)

然後設定 bbs(管理員) 的密碼

```
# passwd bbs
```

記得將 bbs 的家目錄擁有者設定成它自己

```
# chown -R bbs:bbs /home/bbs
```

主機帳戶部分設定完成!

3. 下載 BBS 程式:

**== 以下用 bbs 的權限即可!! ==**

```
$ cd /home/bbs; git clone https://github.com/ccns/dreamlandbbs ;cd dreamlandbbs
```

進去 dreamlandbbs 目錄

會看到除了 README.md 、 .gitignore 、 sample 、 docs 以外

裡面主要分成二個部分:

src 和 web

src 裡面的東西是我們主要要編譯安裝的 BBS 主程式原始碼

web 部分則是現行該版本BBS採用的網頁前端相關檔案 (此部分安裝暫不記錄)


4. git 版控環境初始化設定:

已經有github帳號了並有該 repo 的 commit 權限(或者自己fork)了, 所以先將自己的git環境設定好

```
$ git config --global user.name  "<自己的github帳號>" 
$ git config --global user.email "<自己github的註冊信箱>"
```

之後若要自行開發改良裡面的 code , 就可用 git 指令進行版本控制

並在必要時自己先另外開 branch 測試/建立debug環境 了

5. 接著將範例裡的設定檔複製到 `src/include/` 目錄裡後,
   進入 `src/` 開始準備設定與編譯:

```
$ cp sample/config.h src/include/
```

接著編輯 `include/config.h` 檔案:
(採用 lantw44.bbs<at>ptt.cc 的建議, 使vim猜編碼只有 Big5 這種選擇)

```
$ vim -c 'set fencs=big5' -c 'e!' src/include/config.h
```

設定完之後, 先不要急著make, 而是先自行檢視 BBS 家目錄下全部的目錄結構, 確認已完整配置


若您的 BBS 家目錄( `/home/bbs` )下沒有任何 source code 以外的資料, 
或是尚未熟悉本版本 BBS 運作必要的目錄結構, 可參考本版本目前在 sample/bbs 下的範例目錄

此範例目錄是套用 **WindTopBBS-3.02-20040420-SNAP** 的架構來修改的
( 參考連結: http://ftp.isu.edu.tw/pub/Unix/BBS/WindTop/WindTopBBS-3.02-20040420-SNAP.tgz )

若發現仍有不合之處, 請成功執行程式後, 再自行調整修改.

執行以下指令可使範例目錄架構直接安裝到 BBS 家目錄 下:

```
$ cp -r sample/bbs/* ~/;cp sample/bbs/.BRD ~/
```

接著就開始編譯囉!

```
$ cd src/; make clean linux install; cd ../
```

目前測試是在 CentOS 32bit 環境下 (Debian GNU/Linux 32bit 下還沒編譯成功)

如果相關變數都有定義到的會應該就是可以順利編譯完成

然後記得系統執行正常排程:

`$ crontab sample/crontab`

(裡面的設定調整還有很多還沒整理,
 建議自行檢視裡面的設定是否符合需求, 以及自己調整裡面一些程式的執行路徑)

至於在設定bbs執行環境的部分

在啟動 bbsd 主程式前, 請務必先相關程式先啟動

```
$ /home/bbs/bin/camera
$ /home/bbs/bin/account
$ /home/bbs/bin/acpro
$ /home/bbs/bin/makefw
```

或是自行拿 `sample/bbs/sh` 裡面的 `start.sh` 這個 shell script 去執行也可以

之後要提供 port 23 的 telnet 連線的話, 請用 root 權限執行:

`# /home/bbs/bin/bbsd`

即可, 若要提供連線的 port 編號 > 3000, 則以 bbs 權限執行即可, 如:

`$ /home/bbs/bin/bbsd 3456`

之後開機自動執行的部分, 可以參考 sample/bbs/sh/start.sh 的內容

或自己建立 /etc/rc.local 檔案寫進去:

```

#! /bin/sh
# MapleBBS-WindTop-DreamBBS

su -c bbs '/home/bbs/bin/camera'
su -c bbs '/home/bbs/bin/account'
su -c bbs '/home/bbs/bin/acpro'
su -c bbs '/home/bbs/bin/makefw'

```

並讓 `rc.local` 的權限設定為「可執行」(`+x`)


其他部分因為目前只想先提供telnet連線

所以若要裝 `xinetd` 可照以下設定

(安裝`xinetd`套件後, 將以下內容複製到 `/etc/xinetd.d/telnet` 裡[原本無此檔案])

```
service telnet
{
        disable         = no
        flags           = REUSE
        socket_type     = stream
        wait            = yes
        user            = bbs
        server          = /home/bbs/bin/bbsd
        server_args     = -i
}
```


或 standalone 啟動: 將 /etc/rc.local 的內容再加上:

```

su -c bbs '/home/bbs/bin/bbsd 3456'  # 大於3000的備用port可這樣設定
/home/bbs/bin/bbsd                   # port 23 請直接用 root 權限啟動

```

理論上之後外面就可以連進來自己啟動的 BBS 程式了。


但要注意 CentOS 作業系統內是否有 firewalld iptables 等防火牆設定擋住連線

若有請自己參閱相關資料進行設定。

自己是先: (以 CentOS 7 系列為例)

```
# firewall-cmd --zone=public --permanent --add-port=23/tcp
# firewall-cmd --zone=public --permanent --add-port=3456/tcp
```

之後直接重新啟動:

`# service firewalld restart `

即可完成相關防火牆設定


以上為大致安裝記錄敘述, 若有其他補充, 將會盡快更新相關文件

歡迎有興趣者路過參考並提出相關建議 :)
