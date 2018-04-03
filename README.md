# Dreamland-BBS (夢之大地電子布告欄系統)

This repository is the ***current code*** of Dreamland-bbs ( telnet://ccns.cc ) at NCKU. (under re-construction)

Dreamland BBS source code is modified from [Wind's Top BBS version](http://windtop.yzu.edu.tw/) by cache@ccns and other volunteer developer from The Campus Computer & Network Society of NCKU in Taiwan.

Copyright information for this repo, please view document in `COPYRIGHT`. 

LICENSE for **piaip's more** module, please view `maple/pmore.c` instead.

![](https://i.imgur.com/c0mC6eX.png)

## Directory Structure

* **CHANGELOG**      : Some changes in this BBS version before Initial Commit.

* **COPYRIGHT**      : Something historical claim about copyright of this BBS version.

* **docs**   - Some information about :
    + **INSTALL.md**     : ***How to install*** this BBS version.
    + **ANCESTOR**       : This past history about this BBS version
    + **old**            : Some old information about [Wind's Top BBS verion](http://windtop.yzu.edu.tw) (ancestor of DreamBBS).

* **sample** - Sample codes for:
    + **dreambbs.conf** : ***Important Configuration file*** before Compiling for this BBS version.
    + **crontab**       : Some regular job for OS to let BBS works continually.
    + **bbs**           : If you don't have any BBS data before, you can copy this into your BBS Home(`/home/bbs`) .<br>
                     (related reference: `docs/INSTALL.md`)

+ **include**  : Some definition datas and headers for compiling.

+ **innbbsd**  : Internet News Daemon or Usenet Newsgroup Client for Eagle BBS Series (patched for Maple 3.x Series by itoc) 

+ **lib**      : Static Libraries of this BBS program

+ **maple**    : Main BBS program is here.

+ **so**       : Dynamic Shared Object of this BBS program

+ **util**     : Utility programs for this BBS.

