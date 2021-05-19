# Dreamland-BBS

Build status: [![master branch build status](https://github.com/bbsdocker/imagedreambbs/actions/workflows/push_container.yml/badge.svg)](https://github.com/bbsdocker/imagedreambbs/actions/workflows/push_container.yml) [![develop branch build status](https://github.com/bbsdocker/imagedreambbs/actions/workflows/push_container_develop.yml/badge.svg)](https://github.com/bbsdocker/imagedreambbs/actions/workflows/push_container_develop.yml)

[![](https://i.imgur.com/0EpI7Fa.png)](https://github.com/ccns/dreambbs)

This repository contains the ***current code*** of Dreamland BBS ( telnet://ccns.cc ) at NCKU. (under re-construction)

The source code of Dreamland BBS is modified from [Wind's Top BBS](http://windtop.yzu.edu.tw/)
by cache@ccns and other volunteer developers
at the Campus Computer & Network Society ( [CCNS](https://ccns.github.io) ) at NCKU in Taiwan.

For the copyright information of this repo, please view the document `COPYRIGHT`.

For the licenses for **pmore (piaip's more)** and **pfterm (piaip's flat terminal system)** modules, please view `maple/pmore.c` and `maple/pfterm.c`, respectively.

## To Build a New BBS

* Please pull the repository [dreambbs_snap](https://github.com/ccns/dreambbs_snap) for the essential files.

## Directory Structure

+ **include**  : Data definitions and headers for compiling.

+ **innbbsd**  : An [Internet News Daemon or Usenet Newsgroup](https://en.wikipedia.org/wiki/Usenet) client for Eagles BBS series. (patched for Maple 3.x series by itoc)

+ **lib**      : Static libraries for the BBS program.

+ **maple**    : The **main** BBS program.

+ **so**       : Shared libraries for the BBS program.

+ **util**     : Tool programs for this BBS.

+ **test**     : Internal test tools for functions and data structures of this BBS.

