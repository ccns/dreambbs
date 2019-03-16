# 初步架站手冊

這篇文件說明快速安裝的方法,

首先感謝眾前輩們無私分享這些 source code 以及不少參考文件 ( 包括 Maple-itoc 版本的相關教學資料 )

這邊僅依據目前可以測試成功的部分做個記錄


## 0. 作業系統環境：

本程式試用可以安裝的環境：FreeBSD 11.1-RELEASE , GNU/Linux , Cygwin in Windows 7/10

Arch Linux 請先開啟 `[multilib]` 套件庫，並從 AUR 安裝 `lib32-ncurses` 等相依函式庫

## 1. 安裝作業系統及相關環境:

作業系統部分前面提過了, 套件部分則建議先裝好 

`git` `make` `clang` 等程式 （`bmake` `openssh-server` `nano` `vim` `xinetd` 可自行選擇是否安裝 )

## 2. 建立 BBS 帳號:

目前先嘗試把相關的帳號建立好, 以避免之後權限問題

以下僅為相關參考方法，也可以 `useradd` 、 `groupadd` 等指令替代。


**== 以下用 root 權限 !! ==**

    # mkdir /home/bbs
    # vipw

若使用 Linux , 可於進入編輯器後, 在最後一行加上:

    bbs:x:9999:9999:BBS Administrator:/home/bbs:/bin/bash

*(為求保險起見盡量跟 /etc/passwd 裡列出其他使用者的格式一樣)*

*(也要注意 UID 是否與檔案裡其他提到的使用者重複, 以免出現權限問題)*


接下來編輯 `/etc/group` 來增加名為 `bbs` 群組, 自己目前是用 `vim` 編輯

若對類似編輯器介面不熟也可用 `nano` 或其他更簡易的編輯器介面來修改

    # vim /etc/group

在該檔最後一行加上:

    bbs:x:9999:bbs

*(為求保險起見跟 `/etc/group` 裡列出其他使用者的格式一樣)*

*(要注意 UID 是否與檔案裡其他提到的使用者重複, 以免出現權限問題)*


然後設定 bbs(管理員) 的密碼

    # passwd bbs

記得將 bbs 的家目錄擁有者設定成它自己

    # chown -R bbs:bbs /home/bbs

主機帳戶部分設定完成!


## 3. 下載 BBS 程式:

== 以下用 bbs 的權限即可!! ==

    $ cd /home/bbs; git clone https://github.com/ccns/dreambbs; cd dreambbs; git checkout v0.95.2

接著進去 dreambbs 主目錄

## 4. 設定編譯相關檔案

接著將範例裡的設定檔 `sample/dreambbs.conf` 複製到原始碼第一層主目錄裡後,
準備開始設定與編譯:

    $ cp sample/dreambbs.conf ./

接著編輯 `include/config.h` 檔案:

    $ vim -c 'set fenc=big5 enc=big5 tenc=utf8' -c 'e!' dreambbs.conf

## 5. 確認 BBS 目錄架構配置

設定完之後, 先不要急著執行編譯指令, 而是先檢視 BBS 家目錄下全部的目錄結構, 確認已完整配置

若您的 BBS 家目錄( `/home/bbs` )下沒有任何 source code ( `dreambbs/` ) 以外的資料, 

或是尚未熟悉本版本 BBS 運作必要的目錄結構, 可參考本版本目前在 sample/bbs 下的範例目錄

此範例目錄是套用 WindTopBBS-3.02-20040420-SNAP 的架構來修改的
( 參考連結: http://ftp.isu.edu.tw/pub/Unix/BBS/WindTop/WindTopBBS-3.02-20040420-SNAP.tgz )

若發現仍有不合之處, 請成功執行程式後, 再自行調整修改.

執行以下指令可使範例目錄架構直接安裝到 BBS 家目錄 下:

    $ cp -r sample/bbs /home/ # 若家目錄名稱不是 /home/bbs, 請自行調整修改

## 6. 編譯 BBS 執行檔

接著就開始編譯囉!
若您的作業系統有安裝 bmake 套件，則建議執行以下指令編譯：

    $ bmake all install clean

如果相關變數都有定義到，應該可以順利編譯完成

## 7. 系統環境調校與設定

然後記得系統執行正常排程:

    $ crontab sample/crontab

(建議自行檢視裡面的設定是否符合需求, 以及自己調整裡面一些程式的執行路徑)

至於在設定bbs執行環境的部分

在啟動 bbsd 主程式前, 請務必先相關程式先啟動

    $ /home/bbs/bin/camera
    $ /home/bbs/bin/account
    $ /home/bbs/bin/acpro
    $ /home/bbs/bin/makefw

或是自行拿 `sample/bbs/sh` 裡面的 `start.sh` 這個 shell script 去執行也可以

之後要提供 port 23 的 telnet 連線的話, 請用 root 權限執行:

    # /home/bbs/bin/bbsd

即可, 若要提供連線的 port 編號 > 3000, 則以 bbs 權限執行即可, 如:

    $ /home/bbs/bin/bbsd 3456

之後開機自動執行的部分, 可以參考 `sample/bbs/sh/start.sh` 的內容

或自己建立 `/etc/rc.d/rc.local` 檔案寫進去:

```

#! /bin/sh
# MapleBBS-WindTop-DreamBBS

su bbs -c '/home/bbs/bin/camera'
su bbs -c '/home/bbs/bin/account'
su bbs -c '/home/bbs/bin/acpro'
su bbs -c '/home/bbs/bin/makefw'

```

並確認已讓 `rc.local` 的權限設定為「可執行」(`+x`)

所以若要裝 xinetd 提供telnet連線至 BBS 主程式, 可照以下設定:

(安裝xinetd套件後, 將以下內容複製到 /etc/xinetd.d/telnet 裡[原本無此檔案])

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

# 前略..
su bbs -c '/home/bbs/bin/bbsd 3456'  # 大於3000的備用port可這樣設定
/home/bbs/bin/bbsd                   # port 23 請直接用 root 權限啟動

```

理論上之後外面就可以連進來自己啟動的 BBS 程式了。


但要注意 CentOS 7.x 作業系統內是否有 `firewalld` `iptables` 等防火牆設定擋住連線

若有請自己參閱相關資料進行設定。

自己是先:

    # firewall-cmd --zone=public --permanent --add-port=23/tcp
    # firewall-cmd --zone=public --permanent --add-port=3456/tcp

之後直接重新啟動:

    # service firewalld restart 

即可完成相關防火牆設定


以上為大致安裝記錄敘述, 若有其他補充, 將會盡快更新相關文件 (若有其他開發中的 branch 也可以參考)

歡迎有興趣者路過參考並提出相關建議 :)