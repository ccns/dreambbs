SRCROOT = $(.CURDIR)/..
.include "$(SRCROOT)/dreambbs.mk"

SH = checkACCT.sh  checkbrdDIR.sh  checkusrDIR.sh  clean.sh  start.sh  top.sh

install: $(SH)
	install -d "$(BBSHOME)/sh$(BBSVER)"
	install -m 0700 $? "$(BBSHOME)/sh$(BBSVER)"
