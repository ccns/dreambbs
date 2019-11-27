SRCROOT = $(.CURDIR)/..
.include "$(SRCROOT)/dreambbs.mk"

SH = checkACCT.sh  checkbrdDIR.sh  checkusrDIR.sh  clean.sh  start.sh  top.sh
WSPROXY = wsproxy/wsproxy.lua

install:
	install -d "$(BBSHOME)/sh$(BBSVER)"
	install -m 0700 $(SH) "$(BBSHOME)/sh$(BBSVER)"
	install -d "$(BBSHOME)/wsproxy$(BBSVER)"
	install -m 0755 $(WSPROXY) "$(BBSHOME)/wsproxy$(BBSVER)"
