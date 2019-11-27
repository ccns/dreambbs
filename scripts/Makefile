SRCROOT = $(.CURDIR)/..
.include "$(SRCROOT)/dreambbs.mk"

SH = checkACCT.sh  checkbrdDIR.sh  checkusrDIR.sh  clean.sh  start.sh  top.sh
WSPROXY = wsproxy/wsproxy.lua

install:
	$(INSTALL) -d "$(BBSHOME)/sh$(BBSVER)"
	$(INSTALL) -m 0700 $(SH) "$(BBSHOME)/sh$(BBSVER)"
	$(INSTALL) -d "$(BBSHOME)/wsproxy$(BBSVER)"
	$(INSTALL) -m 0755 $(WSPROXY) "$(BBSHOME)/wsproxy$(BBSVER)"
