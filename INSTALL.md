# 初步架站手冊

這篇文件說明快速安裝本 BBS 系統的方法。

首先感謝眾多前輩們無私分享這些 source code 以及不少參考文件（包括 Maple-itoc 版本的相關教學資料）。

這邊僅依據目前可以測試成功的部分做個紀錄。


## 0. 作業系統環境

本程式經測試可以安裝的環境：

- FreeBSD 11.1~12.0
- GNU/Linux
    - CentOS 7/8
    - Debian 10~12
    - Ubuntu 20.04~22.04
- WSL2, Windows 10/11

欲安裝早於 v3.0 的版本時，Arch Linux 請先開啟 `[multilib]` 套件庫，並從 AUR 安裝 `lib32-ncurses` 等 32-bit 版本的相依函式庫。

v3.0 支援在原生 x86_64 環境中編譯與執行，不須安裝 32-bit 版本的相依函式庫。

## 1. 安裝作業系統及相關環境

作業系統部分請見前文。

套件部分，須要先安裝：

- `git`
- `make`
- `cmake`
- `gcc`/`clang` 或 `g++`/`clang++`，擇一即可

若要使用 `bbs-sshd`，編譯 `bbs-sshd` 前須安裝：

- `cargo`
- `perl`

也建議安裝：

- `vim`。若對類似編輯器介面不熟，也可用 `nano` 或其他更簡易的編輯器介面來編輯，

## 2. 建立 BBS 帳號

這裡先把相關的帳號建立好，以避免之後權限問題。

### 帳號 `bbs` 的設定

**== 以下請在有 sudo 權限的帳號操作 !! ==**

#### 方式：純命令

在 v3.0 以後的版本，執行 `sudo useradd --create-home bbs`（帳號名可自訂）即可完成設定。

在早於 v3.0 的版本，BBS 帳號的名稱與相關設定必須是特定值，可以執行以下命令來完成設定：

```sh
sudo groupadd --gid 99 bbs
sudo useradd --create-home --gid bbs --shell /bin/bash --uid 9999 bbs
```

#### 方式：手動設定

如不使用 `useradd`、`groupadd` 等命令，而要手動設定，可改參考以下方法操作。

(註：v2.0 以後的版本，不再假設使用者名稱為 `bbs`，可自行取名)

(註：v3.0 以後的版本，不再假設 BBS 家目錄為 `/home/bbs`，可自行選擇適合路徑)

首先編輯 `/etc/passwd` 來增加名為 `bbs` 的使用者：

```sh
sudo mkdir /home/bbs
sudo vipw
```

若使用 Linux，可於進入編輯器後，在最後一行加上：

    bbs:x:9999:9999:BBS Administrator:/home/bbs:/bin/bash

*(為求保險起見，請儘量保持跟 `/etc/passwd` 裡列出的其他使用者的格式一樣)*

*(也要注意 UID 不能與檔案裡其他使用者的重複，以免出現權限問題)*


接下來編輯 `/etc/group` 來增加名為 `bbs` 的群組。

(註：v2.0 以後的版本，不再假設群組名稱為 `bbs`，可自行取名)

```sh
sudo vigr
```

在該檔最後一行加上：

    bbs:x:9999:bbs

*(為求保險起見，請保持跟 `/etc/group` 裡列出的其他使用者的格式一樣)*

*(要注意 UID 不能與檔案裡其他使用者的重複，以免出現權限問題)*


然後設定剛才增加的使用者 bbs (管理員) 的密碼：

```sh
sudo passwd bbs
```

記得將 bbs 的家目錄擁有者設定成 bbs 自己：

```sh
sudo chown --recursive bbs:bbs /home/bbs
```

### 帳號 `www-data` 的設定

此外，如果要設定 `wsproxy`（見：[設定：透過 `wsproxy` 提供 WebSocket 連線](#設定透過-wsproxy-提供-WebSocket-連線
)），還需再將建立的使用者加入 `www-data` (或其它適合的使用者群組) 以便使 BBS 主程式自動設定 UNIX socket 的權限。可執行以下命令：

```sh
sudo usermod --append --groups www-data bbs
```

主機帳戶部分設定完成！


## 3. 下載 BBS 程式

== 以下請登入 bbs 帳號後操作 ==

```sh
cd /home/bbs; git clone https://github.com/ccns/dreambbs; cd dreambbs
```

完成時會進入 dreambbs 主目錄。

## 4. 設定編譯相關檔案

接著將範例裡的設定檔 `sample/dreambbs.conf` 複製到原始碼第一層主目錄裡，
準備開始設定與編譯：

```sh
cp sample/dreambbs.conf ./
```

接著編輯 `dreambbs.conf` 檔案：

```sh
vim -c 'set fenc=big5 enc=big5 tenc=utf8' -c 'e!' dreambbs.conf
```

確定已安裝 `cmake` 套件後，請執行以下命令：

```sh
mkdir build/
cd build/
cmake ..
```

其中最後的 `cmake ..` 命令，也可參考以下命令調整，直接指定使用者資訊（未指定之選項將使用目前使用者的使用者資訊）（所列選項僅為範例，請自行斟酌是否合適）：

```sh
BBSUSR=bbs BBSGROUP=bbs WWWGROUP=www-data BBSHOME=/home/bbs cmake ..
```

在原始碼主目錄下（非 build 目錄）會產生以下設定檔，可檢查所產生的設定是否符合需求。如不合可直接編輯。

- `dreambbs.conf`
- `make_export.conf`
- `maple/make_export.conf`

設定檔會由 `cmake` 讀取作為預設設定。因此，若要在不同的專案原始碼目錄套用相同的 BBS 編譯設定，複製以上檔案至對應目錄即可。

此外，預設會使用系統之預設編譯器，並以 C 語言模式編譯。如要指定編譯器或使用 C++ 語言模式編譯，則可參考以下命令調整上述 `cmake ..` 命令（所列選項僅為範例，請自行斟酌是否合適）：

```sh
CC=gcc USE_CXX=1 cmake ..
```

或是

```sh
cmake -DCMAKE_C_COMPILER=gcc -DUSE_CXX=ON ..
```

## 5. 確認 BBS 目錄架構配置

設定完之後，先不要急著執行編譯命令。請先檢視 BBS 家目錄下全部的目錄結構，確認是否已完整配置。

若您的 BBS 家目錄 ( `/home/bbs` ) 下沒有任何 source code ( `dreambbs/` ) 以外的資料，

或是尚未熟悉本版本 BBS 運作所必要的目錄結構，可參考 [dreambbs_snap](https://github.com/ccns/dreambbs_snap) 中的範例目錄。

也可以透過執行以下命令來完成目錄結構的配置：

```sh
git clone https://github.com/ccns/dreambbs_snap.git
cp -r dreambbs_snap/. /home/bbs
```

此範例目錄是使用 WindTopBBS-3.02-20040420-SNAP 的架構為基礎，加以修改而來的。
（參考連結：https://github.com/bbsmirror/BBSmirror/blob/master/WindTop/WindTopBBS-3.02-20040420-SNAP.tgz）

若發現仍有不合之處，請於編譯完成，確認程式可成功執行後，再自行對目錄調整修改。

## 6. 編譯 BBS 執行檔

接著就開始編譯囉！

請在剛才所建立的 `build/` 目錄下，執行：

```sh
make all install
```

如果 `dreambbs.conf` 中的相關 macro 設定都有定義到，應該可以順利編譯完成。

(註：v2.0 以後的版本，`dreambbs.conf` 無須定義任何 macros 也可順利編譯完成並正常執行，但會套用預設的站臺資訊)

如果要重新指定編譯器以及程式語言模式，請見前文：[設定編譯相關檔案](#4-設定編譯相關檔案)。

## 7. 系統環境調校與設定

### 設定：系統例行工作

然後記得設定系統排程。在 `build/` 目錄下（早於 v3.0 的版本請在原始碼主目錄下）執行：

```sh
crontab sample/crontab
```

(建議您自行檢視裡面的設定是否符合需求)

(早於 v3.0 的版本，建議視需要利用 `crontab -e` 調整裡面一些程式的執行路徑)

### BBS 的執行

至於設定 bbs 執行環境的部分：

在啟動 bbsd 主程式前，請務必先執行相關程式建立 SHM。v3.0 以後的版本可直接執行

```sh
/home/bbs/sh/start.sh
```

早於 v3.0 的版本，可參考 `scripts` 裡面的 `start.sh` 的內容去執行。

這兩個 scripts 都會去執行：

```sh
/home/bbs/bin/camera
/home/bbs/bin/account
/home/bbs/bin/acpro
/home/bbs/bin/makefw
```

(註：v2.1 以後的版本，將 `account` 中建立分類看板的工作移到了 `acpro` 中，`start.sh` 不再執行 `account`)

之後若要提供包含 port 23 的 telnet 連線的話，請回到有 sudo 權限的帳號執行，如：

```sh
sudo /home/bbs/bin/bbsd 23
```

若要提供連線的所有 port 編號 > 3000，則繼續以 bbs 權限執行即可，如:

```sh
/home/bbs/bin/bbsd 3456
```

不加參數時，則會使用預設 port 設定。在 v3.0 後可執行 `bin/bbsd -?` 查看預設的 port 設定。

(註：v2.0 後額外支援 `/home/bbs/bin/bbsd -p 3456` 的語法)

(註：v3.0 後可一次指定多個 ports)

### 設定：開機自動啟動

之後是開機自動執行的設定部分。

#### 方法：Systemd unit 設定檔

在 CentOS 7 與 Ubuntu 18.04 以後的作業系統上，系統服務是由 Systemd 管理的，建議使用這個設定方式。

若使用 WSL2，可在 `/etc/wsl.conf` 加入以下內容並完全重新啟動 WSL2 以啟用 Systemd：

```ini
[boot]
systemd=true
```

在 v2.0 後的 `sample/` 下，提供了範例的 Systemd unit 設定檔。

可在 `build/` 目錄下（早於 v3.0 的版本請在原始碼主目錄下）找到範例 Systemd unit 設定檔。v3.0 以後的範例設定檔已代入正確的環境參數，可直接執行：

```sh
sudo cp sample/bbsd.service /etc/systemd/system/
```

早於 v3.0 的版本，請參考 `sample/bbsd.service`，建立 `/etc/systemd/system/bbsd.service` 設定檔。

也可依需求，自行參考 `sample/` 下的其它 `*.service` 檔以建立對應的 Systemctl unit 設定檔。

#### 方法：`rc.local`

在系統服務由 `systemd` 所管理的系統上，預設不會執行 `rc.local` 檔案。若仍要以此方法設定，請先以有 sudo 權限的帳號執行以下命令，以在開機時啟動 `rc.local` 服務。

```sh
sudo chmod +x /etc/rc.d/rc.local
sudo systemctl enable rc-local
```

(註：在 CentOS 7/8 上，`/etc/rc.local` 是指向 `/etc/rc.d/rc.local` 的符號連結，或是說「檔案捷徑」。請依作業系統中的實際路徑調整以上命令。)

如要立即啟動 `rc.local` 服務，可執行以下命令：

```sh
sudo systemctl start rc-local
```

接著建立 `/etc/rc.local` 檔案。v3.0 以後的版本，請寫進以下內容：

```sh
#! /bin/sh
# MapleBBS-WindTop-DreamBBS

sudo -u bbs /home/bbs/sh/start.sh
```

早於 v3.0 的版本，請參考 `scripts/start.sh`，寫進以下內容：

```sh
#! /bin/sh
# MapleBBS-WindTop-DreamBBS

sudo -u bbs /home/bbs/bin/camera
sudo -u bbs /home/bbs/bin/account
sudo -u bbs /home/bbs/bin/acpro
sudo -u bbs /home/bbs/bin/makefw
```

(註：v2.1 後不需 `sudo -u bbs` 也可正常運作)

並確認已將 `rc.local` 的權限設定為「可執行」(`+x`)（見以上說明）。

如不使用 `xinetd`，而要直接以 standalone 模式啟動 BBS 主程式，請在 `/etc/rc.local` 中再加上：

```sh
# 前略..
sudo -u bbs /home/bbs/bin/bbsd 3456  # 大於3000的備用port可這樣設定
/home/bbs/bin/bbsd 23                # port 23 請直接用 sudo 權限啟動
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

```sh
/home/bbs/bin/bbsd -u /home/bbs/run/bbsd.socket
```

請先記住該 socket 的路徑（`/home/bbs/run/bbsd.socket`），後續設定時會使用到。

如要設定在開機時自動以此命令啟動 BBS 主程式，請見：[方法：Systemd unit 設定檔](<#方法Systemd-unit-設定檔>)。

#### 設定：透過 `wsproxy` 提供 WebSocket 連線

`wsproxy` 是由 [@robertabcd](https://github.com/robertabcd) 針對 OpenResty 所開發的 Lua 腳本，可透過 Websocket 傳送 Telnet 協定的資料。它使用 UNIX socket 與 `logind` 或與之相容的伺服器程式溝通。

v2.0 時，本專案新增了目錄 `scripts/wsproxy/`，內含 wsproxy 的說明文件 (`README.md`) 與主程式 (`wsproxy.lua`)（此目錄來自 PttBBS，程式行為相同）。

安裝與設定的細節可參考此目錄下的 `README.md` 修改 OpenResty 的設定檔 `nginx.conf`。

請確認在本 BBS 專案主目錄生成的 `make_export.conf` 中的 `WWWGROUP` (預設為 `www-data`) 的值與 `nginx.conf` 的使用者設定相符。如生成的 `WWWGROUP` 不合需求，可編輯 `make_export.conf` 並再次執行 `cmake` 與 `make` 以重新建置 BBS 主程式。

`nginx.conf` 的使用者設定預設是 `user nobody;`，須改為 `user www-data www-data;` (或其它使用者；視自行設定的 `WWWGROUP` 的值而定)，以使 OpenResty 能夠成功存取與 BBS 主程式溝通用的 UNIX socket。請見：[帳號 `www-data` 的設定](#帳號-www-data-的設定)。

而 `wsproxy` 所使用的 UNIX socket 的路徑，請設定成前面所設定的 socket 路徑。

(v2.1 後會將 `wsproxy.lua` 安裝至 `/home/bbs/sh/wsproxy/wsproxy.lua`，設定時可直接使用該路徑)

#### 設定：透過 `bbs-sshd` 提供 SSH 連線

注意：執行 `bbs-sshd` 前，請先確定電腦的 22 號連接埠並未被使用。尤其是電腦已有執行 `sshd` 供自行連線時，請另外設定額外的連接埠以供自行連線，避免被 `bbs-sshd` 佔用而無法連線。

`bbs-sshd` 是由 [@robertabcd](https://github.com/robertabcd) 以 Rust 語言開發的 SSH 伺服器，可將使用者端的 SSH 協定資料與 BBS 伺服器端的 Telnet 協定資料進行雙向轉換。它使用 UNIX socket 與 `logind` 或與之相容的伺服器程式溝通。

請選擇適當目錄（比如 `/home/bbs/`）並執行以下命令以取得 `bbs-sshd` 原始碼：

```sh
cd /home/bbs; git clone https://github.com/ptt/bbs-sshd.git; cd bbs-sshd
```

接著請參考該專案的 `README.md` 進行 `bbs-sshd` 的建置與設定。

完成後，請參考該專案主目錄下的 `sample/etc/bbs-sshd.toml` 的內容，在專案主目錄下建立 `bbs-sshd.toml`。

其中，`host_key` 的部份，若要使用 `bbs` 賬號專用的 SSH 伺服器公／私鑰，可藉以下命令產生：

```
mkdir /home/bbs/etc/ssh/
ssh-keygen -A -f /home/bbs/
```

並將對應的設定改為：
```
# Host keys
host_keys = [
    "/home/bbs/etc/ssh/ssh_host_ed25519_key",
    "/home/bbs/etc/ssh/ssh_host_ecdsa_key",
    "/home/bbs/etc/ssh/ssh_host_rsa_key",
]
```

而 UNIX socket 的路徑 (`logind_paths`)，請設定成前面所設定的 socket 路徑。

接著使用以下命令啟動 `bbs-sshd`（假設使用了 `cargo build --release` 進行建置）。

```sh
/home/bbs/bbs-sshd/target/release/bbs-sshd -f /home/bbs/bbs-sshd/bbs-sshd.toml
```

(如設定使用了編號 < 1024 的連接埠，則需要 sudo 權限)

v3.1 時，新增了對應的 Systemd unit 設定檔，以在開機時自動啟動 `bbs-sshd`，請見：[方法：Systemd unit 設定檔](<#方法Systemd-unit-設定檔>)。

在使用 `make` 建置本專案前，請先確認在專案主目錄生成的 `make_export.conf` 中的 `BBS_SSHD_ROOT` 的值為 `bbs-sshd` 的專案主目錄，以使產生的 Systemd unit 設定檔具有正確的執行路徑。

若自動生成的路徑不合，可直接編輯 `make_export.conf`，並再次執行 `cmake` 以讀取修改。

### 設定：網路連線

這樣設定之後，從外面應該就可以連進自己啟動的 BBS 程式了。


但要注意 CentOS >7.x 作業系統內可能有 `firewalld`（`iptables`/`nftables`前端) 等防火牆設定擋住連線。

若有，建議自行參閱相關資料進行設定。

這裡的作法是先執行：

```sh
sudo firewall-cmd --zone=public --permanent --add-port=23/tcp
sudo firewall-cmd --zone=public --permanent --add-port=3456/tcp
```

之後直接重新啟動：

```sh
sudo firewall-cmd --reload
```

即可完成相關防火牆設定。


## 結語

以上為大致安裝記錄。若有其他可補充之處，將會儘快更新相關文件 (若有其他開發中的 branch 也可以參考)。

歡迎有興趣者路過參考並提出相關建議。 :)