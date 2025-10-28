# DreamBBS

Build status: `master` [![master branch build status](https://github.com/ccns/dreambbs/actions/workflows/build.yml/badge.svg?branch=master)](https://github.com/ccns/dreambbs/actions/workflows/build.yml?query=branch%3Amaster)

[![](https://i.imgur.com/0EpI7Fa.png)](https://github.com/ccns/dreambbs)

The current code of the BBS server for Dream-Land BBS at NCKU &mdash; <https://ccns.cc>.

This project was based on Wind's Top BBS &mdash; <http://windtop.yzu.edu.tw> &mdash; and has subsequently been developed by Pang-Wei Tsai ([cache](https://github.com/pwtsai)) and other volunteers from the Campus Computer & Network Society (CCNS &mdash; <https://ccns.io> ) at National Cheng Kung University (NCKU) in Taiwan.

For the copyright information of this project, please refer to `COPYRIGHT` and `VERSION`.

Different licenses may apply to individual modules. Please refer to their source code file for the license. Such modules are list below.

## To Set up a New BBS site

Please refer to <https://github.com/ccns/dreambbs/wiki/INSTALL> for the instructions for setting up a BBS site using DreamBBS.

Note that you would need to clone the repository *dreambbs_snap* &mdash; <https://github.com/ccns/dreambbs_snap> &mdash; for the essential files.

If you have cloned this repository, you can also find the wiki contents on the branch "wiki".

## Project Layout

* `CHANGELOG`: The changelog. Unmaintained since 2018.
  For more up-to-dated information, please refer to the release note pages of <https://github.com/ccns/dreambbs/wiki>

* `COPYRIGHT`: The historical copyright declaration from the MapleBBS 3 core team.

* `VERSION`: The copyright information of the ancestors of this project as well as this project.

* `dreambbs.conf`: The compile-time configuration file.
    * *Not present after cloning*. `sample/dreambbs.conf` is a basic example for this file.

* `make_export.conf`: The automatically genetated compile-time configuration file.
    * *Not present after cloning*. This file will be generated upon the first build of the project and can be edited afterward.
  
* `build/`: The recommended building directory.
    * *Not present after cloning*. Please refer to [To Set up a New BBS site](#to-set-up-a-new-bbs-site) for building instructions.

* `sample/`: Sample configuration files and templates. Containing templates of [systemd](https://en.wikipedia.org/wiki/Systemd) service unit files.
    * `bbs-sshd.service.in`: Template file for starting *bbs-sshd*, an SSH-to-Telnet-over-UNIX-socket proxy server, Apache License 2.0 &mdash; <https://github.com/ptt/bbs-sshd>.
      bbs-sshd is developed by Robert Wang (robertabcd) et al.
    * You can find the processed sample configuration files under `build/sample/`

* `innbbsd/`: A [Usenet](https://en.wikipedia.org/wiki/Usenet) newsgroup client daemon.
  First developed by Shih-Kun Huang (skhuang) et al. for Eagles BBS&ndash;derived BBSes.
  Later maintained by Yu-Xuan Tu (itoc) specifically for MapleBBS 3 BBSes.

* `include/`: The header files. Some header files are under `innbbsd/` instead.

* `lib/`: The static library *libdao* (Data Abstract Operation).

* `maple/`: The BBS server `bbsd` and the chat room&ndash;managing daemon `xchatd`. Containing frequently used modules.
    * `pmore.c`: The optional module *pmore* (piaip's more), BSD-like custom license.
      Developed by Hung-Te Lin (piaip) et al.
    * `pfterm.c`: The optional module *pfterm* (piaip's flat terminal system), BSD-like custom license.
      Developed by Hung-Te Lin (piaip) et al.
    * Installed to `${BBSHOME}/bin/`

* `so/`: The shared libraries. Containing less frequently used modules.
    * `bbslua.c`: The optional module *BBS-Lua*, MIT license (Expat).
      Developed by Hung-Te Lin (piaip) et al.
    * `bbsluaext.c`: The dependency module *Lua BitOp* for `bbslua.c`, MIT license (Expat).
      Developed by Mike Pall et al.
    * `bbsruby.c`: The optional module *BBS-Ruby*, MIT license (Expat).
      Developed by Hung-Te Lin (piaip) et al.
    * Installed to `${BBSHOME}/bin/` if allowed to be compiled into shared libraries.

* `util/`: Helper programs. Containing programs to be run periodly by [cron](https://en.wikipedia.org/wiki/Cron).

* `scripts/`: Helper scripts and templates. Containing `wsproxy`.
    * `wsproxy/`: *wsproxy*, a Telnet-over-WebSocket server script for OpenResty, MIT license (Expat).
      Developed by Robert Wang (robertabcd) et al.
    * Installed to `${BBSHOME}/sh/`

* `test/`: Test programs and scripts. Used for verifying the build.

* `.github/`: The GitHub configurations.
    * `workflows/`: The GitHub Action automation configurations for aiding development.
