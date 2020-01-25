# 初步架站手冊

這篇文件說明快速安裝本 BBS 系統的方法。

首先感謝眾多前輩們無私分享這些 source code 以及不少參考文件（包括 Maple-itoc 版本的相關教學資料）。

這邊僅依據目前可以測試成功的部分做個紀錄。


## 0. 作業系統環境

本程式經測試可以安裝的環境：FreeBSD 11.1~12.0, GNU/Linux, Cygwin in Windows 7/10

Arch Linux 請先開啟 `[multilib]` 套件庫，並從 AUR 安裝 `lib32-ncurses` 等 32-bit 版本的相依函式庫。

## 1. 安裝作業系統及相關環境

作業系統部分前面提過了，套件部分則建議先裝好 

`git`, `bmake` (BSD-like 使用原生的 `make` 即可), `clang` 等程式。
v2.0 以後的版本，可以不用 `bmake`，改用 `cmake`。

## 2. 建立 BBS 帳號

這裡先把相關的帳號建立好，以避免之後權限問題。

以下方法僅為參考，可用 `useradd`、`groupadd` 等指令替代。
註：v2.0 以後的版本，不再假設使用者名稱為 `bbs`，可自行取名。


**== 以下用 root 權限 !! ==**

    # mkdir /home/bbs
    # vipw

若使用 Linux，可於進入編輯器後，在最後一行加上：

    bbs:x:9999:9999:BBS Administrator:/home/bbs:/bin/bash

*(為求保險起見，請儘量保持跟 `/etc/passwd` 裡列出的其他使用者的格式一樣)*

*(也要注意 UID 不能與檔案裡其他使用者的重複，以免出現權限問題)*


接下來編輯 `/etc/group` 來增加名為 `bbs` 的群組。這裡使用 `vim` 編輯，
註：v2.0 以後的版本，不再假設群組名稱為 `bbs`，可自行取名。

若對類似編輯器介面不熟，也可用 `nano` 或其他更簡易的編輯器介面來編輯，

    # vim /etc/group

在該檔最後一行加上：

    bbs:x:9999:bbs

*(為求保險起見，請保持跟 `/etc/group` 裡列出的其他使用者的格式一樣)*

*(要注意 UID 不能與檔案裡其他使用者的重複，以免出現權限問題)*


然後設定剛才增加的使用者 bbs (管理員) 的密碼：

    # passwd bbs

記得將 bbs 的家目錄擁有者設定成 bbs 自己：

    # chown -R bbs:bbs /home/bbs

主機帳戶部分設定完成！


## 3. 下載 BBS 程式

== 以下請登入 bbs 帳號後操作 ==

    $ cd /home/bbs; git clone https://github.com/ccns/dreambbs; cd dreambbs; git checkout v2.0.1

接著進去 dreambbs 主目錄。

## 4. 設定編譯相關檔案

接著將範例裡的設定檔 `sample/dreambbs.conf` 複製到原始碼第一層主目錄裡，
準備開始設定與編譯：

    $ cp sample/dreambbs.conf ./

接著編輯 `dreambbs.conf` 檔案：

    $ vim -c 'set fenc=big5 enc=big5 tenc=utf8' -c 'e!' dreambbs.conf

在 v2.0 後的版本，還需要使用 `bmake` 或 `cmake` 產生自動設定檔：
若您的作業系統有安裝 `bmake` 套件 (在BSD-like系統中，改用 `make` 指令即可)，請執行以下指令：

    $ bmake configure

如果要使用 `cmake`，則建議改用以下指令：

    $ mkdir build/
    $ cd build/
    $ cmake ..

可檢查所產生的設定是否符合需求。

## 5. 確認 BBS 目錄架構配置

設定完之後，先不要急著執行編譯指令。請先檢視 BBS 家目錄下全部的目錄結構，確認是否已完整配置。

若您的 BBS 家目錄 ( `/home/bbs` ) 下沒有任何 source code ( `dreambbs/` ) 以外的資料，

或是尚未熟悉本版本 BBS 運作所必要的目錄結構，可參考 [dreambbs-snap](https://github.com/ccns/dreambbs-snap) 中的範例目錄。

也可以透過執行以下指令來完成目錄結構的配置：
```
git clone https://github.com/ccns/dreambbs-snap.git bbs
cp -r bbs /home/
```

此範例目錄是使用 WindTopBBS-3.02-20040420-SNAP 的架構為基礎，加以修改而來的。
（參考連結：https://github.com/bbsmirror/BBSmirror/blob/master/WindTop/WindTopBBS-3.02-20040420-SNAP.tgz）

若發現仍有不合之處，請於編譯完成，確認程式可成功執行後，再自行對目錄調整修改。

## 6. 編譯 BBS 執行檔

接著就開始編譯囉！
若您的作業系統有安裝 `bmake` 套件 (在BSD-like系統中，改用 `make` 指令即可)，則建議執行以下指令來編譯：

    $ bmake all install

在 v2.0 以後的版本，若要使用 `cmake` 編譯，則進入剛才所建立的 `build/` 資料夾下，執行以下指令：

    $ make all install

如果 `dreambbs.conf` 中的相關變數都有定義到，應該可以順利編譯完成。
註：v2.0 以後的版本，即使 `dreambbs.conf` 中未定義任何變數，也可順利編譯完成。

## 7. 系統環境調校與設定

然後記得設定系統排程：

    $ crontab sample/crontab

(建議您自行檢視裡面的設定是否符合需求，以及視需要調整裡面一些程式的執行路徑)

至於設定 bbs 執行環境的部分：

在啟動 bbsd 主程式前，請務必先執行相關程式，建立 SHM：

    $ /home/bbs/bin/camera
    $ /home/bbs/bin/account
    $ /home/bbs/bin/acpro
    $ /home/bbs/bin/makefw

註：v2.1 以後的版本，將 `account` 中建立分類看板的工作移到 `acpro` 中，不須執行 `account`
或者是拿 `scripts` 裡面的 `start.sh` 這個 shell script 去執行。

之後若要提供 port 23 的 telnet 連線的話，以 root 權限執行即可，如：

    # /home/bbs/bin/bbsd

若要提供連線的 port 編號 > 3000，則以 bbs 權限執行即可，如:

    $ /home/bbs/bin/bbsd 3456

註：v2.0 後額外支援 `/home/bbs/bin/bbsd -p 3456` 的語法。
之後開機自動執行的設定部分，可以參考 `sample/bbs/sh/start.sh` 的內容，

或自己建立 `/etc/rc.d/rc.local` 檔案，寫進以下內容：

```

#! /bin/sh
# MapleBBS-WindTop-DreamBBS

su bbs -c '/home/bbs/bin/camera'
su bbs -c '/home/bbs/bin/account'
su bbs -c '/home/bbs/bin/acpro'
su bbs -c '/home/bbs/bin/makefw'

```

註：v2.1 後不須 `su bbs` 也可正常運作。
並確認已將 `rc.local` 的權限設定為「可執行」(`+x`)。

若要裝 xinetd 提供 telnet 連線至 BBS 主程式，可參考以下設定：

(安裝 `xinetd` 套件後，將以下內容複製到 `/etc/xinetd.d/telnet` 裡 [原本無此檔案])

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

如要以 standalone 模式啟動，請在 `/etc/rc.local` 中再加上：

```

# 前略..
su bbs -c '/home/bbs/bin/bbsd 3456'  # 大於3000的備用port可這樣設定
/home/bbs/bin/bbsd                   # port 23 請直接用 root 權限啟動

```

這樣設定之後，從外面應該就可以連進自己啟動的 BBS 程式了。


但要注意 CentOS 7.x 作業系統內可能有 `firewalld` `iptables` 等防火牆設定擋住連線。

若有，建議自行參閱相關資料進行設定。

這裡的作法是先執行：

    # firewall-cmd --zone=public --permanent --add-port=23/tcp
    # firewall-cmd --zone=public --permanent --add-port=3456/tcp

之後直接重新啟動：

    # service firewalld restart 

即可完成相關防火牆設定。


以上為大致安裝記錄。若有其他可補充之處，將會儘快更新相關文件 (若有其他開發中的 branch 也可以參考)。

歡迎有興趣者路過參考並提出相關建議。 :)