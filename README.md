# Dreamland-BBS (夢之大地電子布告欄系統)

This repository is the ***current code*** of [dreamland-bbs](http://bbs.ccns.cc) ( telnet://ccns.cc ) at NCKU.

Dreamland BBS source code is cache@ccns modified from [Wind's Top BBS version](http://windtop.yzu.edu.tw/)

![](https://i.imgur.com/c0mC6eX.png)

## Directory Structure

* **docs**   - Some information about :
    + **INSTALL**        : ***How to install*** this BBS version.
    + **ANCESTOR**       : This past history about this BBS version
    + **src-ChangeLog**  : Some changes in this BBS version before Initial Commit.
    + **old**            : Some old information about [Wind's Top BBS verion](http://windtop.yzu.edu.tw) (ancestor of DreamBBS).

* **sample** - Sample codes for:
    + **config.h**     : ***Important Configuration file*** before Compiling for this BBS version.
    + **crontab**      : Some regular job for OS to let BBS works continually.
    + **bbs**          : If you don't have any BBS data before, you can copy this into your BBS Home(`/home/bbs`) .<br>
                     (related reference: `docs/INSTALL.md`)

+ **bgopherd** : BBS gopher daemon. *(deprecated)*

+ **bmtad**    : Mail Transport Agent for BBS. *(some features not works until now)*

+ **bpop3d**   : Simple POP3 server for BBS user.

+ **include**  : some definition datas and headers for compiling.

+ **innbbsd**  : Cross BBS sites posting. *(deprecated)*

+ **lib**      : Static Libraries of this BBS program

+ **maple**    : main BBS program is here.

+ **so**       : Shared Object Libraries of this BBS program

+ **util**     : other utility programs for this BBS.

