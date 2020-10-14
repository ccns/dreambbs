# ------------------------------------------------------- #
#  sample/Makefile ( NCKU CCNS DreamBBS 3.0 )             #
# ------------------------------------------------------- #
#  Author: 37586669+IepIweidieng@users.noreply.github.com #
#  Target: Makefile for DreamBBS sample files             #
#  Create: 2020-10-15                                     #
# ------------------------------------------------------- #

SRCROOT	= $(.CURDIR)/..
.include "$(SRCROOT)/dreambbs.mk"

DEST 	+= bbsd.service bbsd_unixsocket.service \
	   startbbs.service xchatd.service \
	   crontab

all: $(DEST)

.for target in $(DEST)
$(target): $(target).in
	sed 's/@BBSHOME@/$(BBSHOME:S/\//\\\//g)/; s/@BBSUSR@/$(BBSUSR)/; s/@BBSGROUP@/$(BBSGROUP)/' $? > $@
.endfor

clean:
	rm -rf $(DEST) *~
