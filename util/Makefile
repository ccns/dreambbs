# ------------------------------------------------------- #
#  util/Makefile    ( NTHU CS MapleBBS Ver 3.x )          #
# ------------------------------------------------------- #
#  target : Makefile for 寄信、統計、備份、系統維護工具   #
#  create :   /  /                                        #
#  update : 18/03/28                                      #
# ------------------------------------------------------- #

SRCROOT	= ..
.include "$(SRCROOT)/dreambbs.mk"

.SUFFIXES: .o .c

.c.o:	; $(CC) $(MAKEFLAG) $(CFLAGS) -g -c $*.c

EXE	= account acl-sort camera expire gem-check gem-index makeUSR \
	  hdr-dump poststat reaper rmbadmail restorebrd \
	  mailpost bbsmail topusr acpro addsong userno template restoregem \
	  checkemail match mailexpire utmp-dump \
	  stopperm mailtoall clean_acl makefw \
	  resetvmail counter restoreusr makeusrno outgo redir \
	  classtable_alert bmw-dump tranBRD \
	  brdstat brdcheck dump-brdstat base64encode \
	  msgall transacct showACCT showBRD \
	  backup brdmail addpost

all: $(EXE)

.for target in $(EXE)
$(target): $(target).o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)
.endfor

install: $(EXE)
	install -m 0700 $? $(BBSHOME)/bin$(BBSVER)/

clean:
	rm -fr $(EXE) *.o *.bak *.BAK *.log *~

