# Dreamland-BBS

Travis CI status: [![](https://travis-ci.com/ccns/dreambbs.svg?branch=master)](https://travis-ci.com/ccns/dreambbs)

[![](https://i.imgur.com/0EpI7Fa.png)](https://github.com/ccns/dreambbs)

This repository is the ***current code*** of Dreamland-bbs ( telnet://ccns.cc ) at NCKU. (under re-construction)

Dreamland BBS source code is modified from [Wind's Top BBS version](http://windtop.yzu.edu.tw/)
by cache@ccns and other volunteer developer from
The Campus Computer & Network Society ( [CCNS](https://ccns.github.io) ) of NCKU in Taiwan.

Copyright information for this repo, please view document in `COPYRIGHT`.

LICENSE for **piaip's more**, **piaip's flat terminal system** module, please view `maple/pmore.c`, `maple/pfterm.c` instead.

## Build New BBS

* Please pull [dreambbs_snap](https://github.com/ccns/dreambbs_snap) repository to add the essential files.

## Directory Structure

+ **include**  : Some definition data and headers for compiling.

+ **innbbsd**  : [Internet News Daemon or Usenet Newsgroup](https://en.wikipedia.org/wiki/Usenet) Client for Eagle BBS Series (patched for Maple 3.x Series by itoc)

+ **lib**      : Common Libraries of this BBS program

+ **maple**    : Main BBS program is **here**.

+ **so**       : Dynamic Shared Object of this BBS program

+ **util**     : Tool programs for this BBS.

+ **test**     : Internal Test Tools for BBS functions or structures.

