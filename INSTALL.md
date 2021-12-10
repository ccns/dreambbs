# 初步架站手冊

這篇文件說明快速安裝本 BBS 系統的方法。

首先感謝眾多前輩們無私分享這些 source code 以及不少參考文件（包括 Maple-itoc 版本的相關教學資料）。

這邊僅依據目前可以測試成功的部分做個紀錄。


## 0. 作業系統環境

本程式經測試可以安裝的環境：FreeBSD 11.1~12.0, GNU/Linux(CentOS 7/8, Debian 10, Ubuntu 20.04), Cygwin in Windows 7/10

欲安裝早於 v3.0 的版本時，Arch Linux 請先開啟 `[multilib]` 套件庫，並從 AUR 安裝 `lib32-ncurses` 等 32-bit 版本的相依函式庫。

v3.0 支援在原生 x86_64 環境中編譯與執行，不須安裝 32-bit 版本的相依函式庫。

## 1. 安裝作業系統及相關環境

作業系統部分請見前文。

套件部分，須要先安裝：`git`, `make`, `cmake`, `gcc`/`clang`, `g++`/`clang++`。

也建議安裝：`vim`。若對類似編輯器介面不熟，也可用 `nano` 或其他更簡易的編輯器介面來編輯，

## 2. 建立 BBS 帳號

這裡先把相關的帳號建立好，以避免之後權限問題。

### 方式：一行命令

在 v3.0 以後的版本，以 root 權限執行 `useradd -m bbs` (使用者名稱可自訂) 即可完成設定。

在早於 v3.0 的版本下，可以 root 權限執行以下命令來完成設定：
```
groupadd --gid 99 bbs && useradd -m -g bbs -s /bin/bash --uid 9999 bbs
```

### 方式：手動設定

如不使用 `useradd`、`groupadd` 等命令，而要手動設定，可改參考以下方法操作。

(註：v2.0 以後的版本，不再假設使用者名稱為 `bbs`，可自行取名)

(註：v3.0 以後的版本，不再假設 BBS 家目錄為 `/home/bbs`，可自行選擇適合路徑)

首先編輯 `/etc/passwd` 來增加名為 `bbs` 的使用者：

**== 以下用 root 權限 !! ==**

    # mkdir /home/bbs
    # vipw

若使用 Linux，可於進入編輯器後，在最後一行加上：

    bbs:x:9999:9999:BBS Administrator:/home/bbs:/bin/bash

*(為求保險起見，請儘量保持跟 `/etc/passwd` 裡列出的其他使用者的格式一樣)*

*(也要注意 UID 不能與檔案裡其他使用者的重複，以免出現權限問題)*


接下來編輯 `/etc/group` 來增加名為 `bbs` 的群組。

(註：v2.0 以後的版本，不再假設群組名稱為 `bbs`，可自行取名)

    # vigr

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

    $ cd /home/bbs; git clone https://github.com/ccns/dreambbs; cd dreambbs

接著進去 dreambbs 主目錄。

## 4. 設定編譯相關檔案

接著將範例裡的設定檔 `sample/dreambbs.conf` 複製到原始碼第一層主目錄裡，
準備開始設定與編譯：

    $ cp sample/dreambbs.conf ./

接著編輯 `dreambbs.conf` 檔案：

    $ vim -c 'set fenc=big5 enc=big5 tenc=utf8' -c 'e!' dreambbs.conf

確定已安裝 `cmake` 套件後，請執行以下命令：

    $ mkdir build/
    $ cd build/
    $ cmake ..

其中最後的 `cmake ..` 命令，也可參考以下命令調整，直接指定使用者資訊（未指定之選項將使用目前使用者的使用者資訊）（所列選項僅為範例，請自行斟酌是否合適）：

    $ BBSUSR=bbs BBSGROUP=bbs WWWGROUP=www-data BBSHOME=/home/bbs cmake ..

可檢查所產生的設定是否符合需求。

此外，預設會使用系統之預設編譯器，並以 C 語言模式編譯。如要指定編譯器或使用 C++ 語言模式編譯，則可參考以下命令調整上述 `cmake ..` 命令（所列選項僅為範例，請自行斟酌是否合適）：

    $ CC=gcc USE_CXX=1 cmake ..

或是

    $ cmake -DCMAKE_C_COMPILER=gcc -DUSE_CXX=ON ..

## 5. 確認 BBS 目錄架構配置

設定完之後，先不要急著執行編譯命令。請先檢視 BBS 家目錄下全部的目錄結構，確認是否已完整配置。

若您的 BBS 家目錄 ( `/home/bbs` ) 下沒有任何 source code ( `dreambbs/` ) 以外的資料，

或是尚未熟悉本版本 BBS 運作所必要的目錄結構，可參考 [dreambbs_snap](https://github.com/ccns/dreambbs_snap) 中的範例目錄。

也可以透過執行以下命令來完成目錄結構的配置：
```
git clone https://github.com/ccns/dreambbs_snap.git bbs
cp -r bbs /home/
```
此範例目錄是使用 WindTopBBS-3.02-20040420-SNAP 的架構為基礎，加以修改而來的。
（參考連結：https://github.com/bbsmirror/BBSmirror/blob/master/WindTop/WindTopBBS-3.02-20040420-SNAP.tgz）

若發現仍有不合之處，請於編譯完成，確認程式可成功執行後，再自行對目錄調整修改。

## 6. 編譯 BBS 執行檔

接著就開始編譯囉！

請在剛才所建立的 `build/` 目錄下，執行：

    $ make all install

如果 `dreambbs.conf` 中的相關變數都有定義到，應該可以順利編譯完成。

(註：v2.0 以後的版本，即使 `dreambbs.conf` 中未定義任何變數，也可順利編譯完成並正常執行)

如果要重新指定編譯器以及程式語言模式，請見前文：[設定編譯相關檔案](#4-%E8%A8%AD%E5%AE%9A%E7%B7%A8%E8%AD%AF%E7%9B%B8%E9%97%9C%E6%AA%94%E6%A1%88)。

## 7. 系統環境調校與設定

### 設定：系統例行工作

然後記得設定系統排程：

    $ crontab sample/crontab

(建議您自行檢視裡面的設定是否符合需求，以及視需要利用 `crontab -e` 調整裡面一些程式的執行路徑)

(註：v3.0 以後的版本會在 `make` 時產生有正確路徑的 `crontab`，請到上述的 `build/` 目錄下執行該命令)

### BBS 的執行

至於設定 bbs 執行環境的部分：

在啟動 bbsd 主程式前，請務必先執行相關程式，建立 SHM：

    $ /home/bbs/bin/camera
    $ /home/bbs/bin/account
    $ /home/bbs/bin/acpro
    $ /home/bbs/bin/makefw

(註：v2.1 以後的版本，將 `account` 中建立分類看板的工作移到了 `acpro` 中，不須執行 `account`)

或者是拿 `scripts` 裡面的 `start.sh` 這個 shell script 去執行。

(註：v3.0 以後的版本在使用 `make install` 安裝時，會將已產生正確路徑的此腳本安裝至 BBS 家目錄下的 `sh/start.sh`，可直接執行)

之後若要提供 port 23 的 telnet 連線的話，以 root 權限執行即可，如：

    # /home/bbs/bin/bbsd 23

若要提供連線的 port 編號 > 3000，則以 bbs 權限執行即可，如:

    $ /home/bbs/bin/bbsd 3456

不加參數時，則會使用預設 port 設定。在 v3.0 後可執行 `bin/bbsd -?` 查看預設的 port 設定。

(註：v2.0 後額外支援 `/home/bbs/bin/bbsd -p 3456` 的語法)

(註：v3.0 後可一次指定多個 ports)

### 設定：開機自動啟動

之後是開機自動執行的設定部分。

#### 方法：Systemd unit 設定檔

在 CentOS 7 與 Ubuntu 18.04 以後的作業系統上，系統服務是由 Systemd 管理的，建議使用這個設定方式。

在 v2.0 後的 `sample/` 下，提供了範例的 Systemd unit 設定檔。

請參考 `sample/bbsd.service`，建立 `/etc/systemd/system/bbsd.service` 設定檔。

也可依需求，自行參考 `sample/` 下的其它 `*.service` 檔以建立對應的 Systemctl unit 設定檔。

(註：v3.0 以後的版本會在 `make` 時產生已代入正確的環境參數的設定檔，可直接使用；請到上述的 `build/` 目錄下查看)

#### 方法：`rc.local`

在系統服務由 `systemd` 所管理的系統上，預設不會執行 `rc.local` 檔案。若仍要以此方法設定，請先以 root 權限執行以下命令，以在開機時啟動 `rc.local` 服務。

    # chmod +x /etc/rc.d/rc.local
    # systemctl enable rc-local

(註：在 CentOS 7/8 上，`/etc/rc.local` 是指向 `/etc/rc.d/rc.local` 的符號連結，或是說「檔案捷徑」。請依作業系統中的實際路徑調整以上命令。)

如要立即啟動 `rc.local` 服務，可執行以下命令：

    # systemctl start rc-local

接著可參考 `sh/start.sh` 的內容，或自己建立 `/etc/rc.local` 檔案，寫進以下內容：

```
#! /bin/sh
# MapleBBS-WindTop-DreamBBS

su bbs -c '/home/bbs/bin/camera'
su bbs -c '/home/bbs/bin/account'
su bbs -c '/home/bbs/bin/acpro'
su bbs -c '/home/bbs/bin/makefw'
```

v3.0 後也可改寫進以下內容：
```
#! /bin/sh
# MapleBBS-WindTop-DreamBBS

su bbs -c '/home/bbs/sh/start.sh'
```

(註：v2.1 後不需 `su bbs` 也可正常運作)

並確認已將 `rc.local` 的權限設定為「可執行」(`+x`)（見以上說明）。

如不使用 `xinetd`，而要直接以 standalone 模式啟動 BBS 主程式，請在 `/etc/rc.local` 中再加上：

```
# 前略..
su bbs -c '/home/bbs/bin/bbsd 3456'  # 大於3000的備用port可這樣設定
/home/bbs/bin/bbsd 23                # port 23 請直接用 root 權限啟動
```

### 設定：`xinetd`

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

### 設定：監聽 UNIX socket

從 v2.0 起，BBS 主程式支援從 UNIX socket 監聽連入連線。此模式與 PttBBS 的 `logind` 所提供的 UNIX socket 連線模式相容。

可透過以下命令啟動 BBS 主程式，以從某個 UNIX socket 監聽連線。

    $ /home/bbs/bin/bbsd -u /home/bbs/run/bbsd.socket

如要設定在開機時自動以此命令啟動 BBS 主程式，請見：[方法：Systemd unit 設定檔](<#方法Systemd-unit-設定檔>)。

#### 設定：透過 `wsproxy` 提供 WebSocket 連線

`wsproxy` 是由 [@robertabcd](https://github.com/robertabcd) 針對 OpenResty 所開發的 Lua 腳本，可透過 Websocket 傳送 Telnet 協定的資料。

v2.0 時，本專案新增了目錄 `scripts/wsproxy/`，內含 wsproxy 的說明文件 (`README.md`) 與主程式 (`wsproxy.lua`)（此目錄來自 PttBBS，程式行為相同）。

安裝與設定的細節可參考此目錄下的 `README.md` 修改 OpenResty 的設定檔。

(v2.1 後會將 `wsproxy.lua` 安裝至 `/home/bbs/sh/wsproxy/wsproxy.lua`，設定時可直接使用該路徑)

#### 設定：透過 `bbs-sshd` 提供 SSH 連線

注意：執行 `bbs-sshd` 前，請先確定電腦的 22 號連接埠並未被使用。尤其是電腦已有執行 `sshd` 供自行連線時，請另外設定額外的連接埠以供自行連線，避免被 `bbs-sshd` 佔用而無法連線。

`bbs-sshd` 是由 [@robertabcd](https://github.com/robertabcd) 以 Rust 語言開發的 SSH 伺服器，可將使用者端的 SSH 協定資料與 BBS 伺服器端的 Telnet 協定資料進行雙向轉換 。

請選擇適當目錄（比如 `/home/bbs/`）並執行以下命令以取得 `bbs-sshd` 原始碼：

    $ cd /home/bbs; git clone https://github.com/ptt/bbs-sshd.git; cd bbs-sshd

接著請參考該專案的 `README.md` 進行 `bbs-sshd` 的建置與設定。

完成後，請參考該專案主目錄下的 `sample/etc/bbs-sshd.toml` 的內容，在專案主目錄下建立 `bbs-sshd.toml`。接著使用以下命令啟動 `bbs-sshd`（假設使用了 `cargo build --release` 進行建置）。

    $ /home/bbs/bbs-sshd/target/release/bbs-sshd -f /home/bbs/bbs-sshd/bbs-sshd.toml

(如設定使用了編號 < 1024 的連接埠，則需要 root 權限)

v3.1 時，新增了對應的 Systemd unit 設定檔，以在開機時自動啟動 `bbs-sshd`，請見：[方法：Systemd unit 設定檔](<#方法Systemd-unit-設定檔>)。

### 設定：網路連線

這樣設定之後，從外面應該就可以連進自己啟動的 BBS 程式了。


但要注意 CentOS >7.x 作業系統內可能有 `firewalld`（`iptables`/`nftables`前端) 等防火牆設定擋住連線。

若有，建議自行參閱相關資料進行設定。

這裡的作法是先執行：

    # firewall-cmd --zone=public --permanent --add-port=23/tcp
    # firewall-cmd --zone=public --permanent --add-port=3456/tcp

之後直接重新啟動：

    # firewall-cmd --reload

即可完成相關防火牆設定。


## 結語

以上為大致安裝記錄。若有其他可補充之處，將會儘快更新相關文件 (若有其他開發中的 branch 也可以參考)。

歡迎有興趣者路過參考並提出相關建議。 :)