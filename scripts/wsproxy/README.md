# wsproxy

This is the websocket to telnet bbs proxy.

## Dependency

- [OpenResty](https://openresty.org) (>= 1.15.8.1) -- Official version of OpenResty, needs no patches.
- [vstruct](https://github.com/toxicfrog/vstruct/) -- Lua library for binary manipulation.

## Install

- [Install OpenResty](https://openresty.org/en/linux-packages.html) (example for Debian/Ubuntu).
```
# systemctl disable nginx
# systemctl stop nginx

$ wget -qO - https://openresty.org/package/pubkey.gpg | sudo apt-key add -
# apt-get -y install software-properties-common
# add-apt-repository -y "deb http://openresty.org/package/$(lsb_release -sic | tr '[:upper:]' '[:lower:]') openresty"
# apt-get update
# apt-get install --no-install-recommends openresty
```

- Download vstruct
```
$ cd ~bbs/pttbbs/daemon/wsproxy && mkdir lib && cd lib
$ git clone https://github.com/toxicfrog/vstruct/
```

- Configure nginx
```nginx
lua_package_path ";;/home/bbs/pttbbs/daemon/wsproxy/lib/?.lua;/home/bbs/pttbbs/daemon/wsproxy/lib/?/init.lua";
server {
        listen 80 default_server;
        location /bbs {
                #set $bbs_secure 1;
                content_by_lua_file /home/bbs/pttbbs/daemon/wsproxy/wsproxy.lua;
        }
}
```

- Configure wsproxy.lua (interesting variables listed)
```lua
local logind_addr = "unix:/home/bbs/run/logind.connfwd.sock"

local origin_whitelist = {
    ["http://www.ptt.cc"] = true,
    ["https://www.ptt.cc"] = true,
    ["https://robertabcd.github.io"] = true,
    ["app://pcman"] = true,
}
```

- Configure logind (etc/bindports.conf)
```
logind unix run/logind.connfwd.sock
```
