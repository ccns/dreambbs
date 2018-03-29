# ------------------------------------------------------- #
#  util/Makefile    ( NTHU CS MapleBBS Ver 3.x )          #
# ------------------------------------------------------- #
#  target : Makefile for 寄信、統計、備份、系統維護工具	  #
#  create :   /  /                                        #
#  update : 18/03/28                                      #
# ------------------------------------------------------- #

MAKE	+=	-f Makefile.gnu

UNAME	:= $(shell uname)

ARCHI	:= $(shell uname -m)

CC	= clang
CFLAGS	= -g -O2 -I../include -fomit-frame-pointer -Wunused
LDFLAGS	= -L../lib -ldao -lcrypt

ifeq ($(ARCHI),x86_64) 
CFLAGS	+= -m32
LDFLAGS	+= -m32
else
ifeq ($(ARCHI),amd64)
CFLAGS	+= -m32
LDFLAGS	+= -m32
endif
endif

ifeq ($(UNAME),Linux)
LDFLAGS	+= -lresolv
endif

.SUFFIXES: .o .c

.c.o:   ;   $(CC) $(MAKEFLAG) $(CFLAGS) -g -c $*.c

EXE	= account acl-sort camera expire gem-check gem-index makeUSR \
	  hdr-dump poststat reaper countstar rmbadmail restorebrd \
	  mailpost bbsmail topusr acpro addsong userno template restoregem \
	  checkemail match mailexpire countbirth countage utmp-dump \
	  stopperm mailtoall clean_acl makefw \
	  resetvmail counter restoreusr makeusrno outgo redir \
	  classtable_alert bmw-dump tranBRD \
	  brdstat dump-brdstat base64encode \
	  msgall transacct showACCT showBRD \
	  backup brdmail

all: 
	@$(MAKE) CC="$(CC)" CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)" $(EXE)

msgall: msgall.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

makeUSR: makeUSR.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

tranufo: tranufo.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

clean_acl: clean_acl.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

rmbadmail: rmbadmail.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

countstar: countstar.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

classtable_alert: classtable_alert.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

restorebrd: restorebrd.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

restoregem: restoregem.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

brdstat: brdstat.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

countage: countage.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

makeusrno: makeusrno.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

getstar: getstar.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

tranBRD: tranBRD.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

dump-brdstat: dump-brdstat.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

utmp-dump: utmp-dump.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

mailtoall: mailtoall.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

mailexpire: mailexpire.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

outgo: outgo.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

2nd_expire: 2nd_expire.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

restoreusr: restoreusr.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

counter: counter.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

base64encode: base64encode.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

bmw-dump: bmw-dump.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

stopperm: stopperm.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

checkemail: checkemail.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

makefw: makefw.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

match: match.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

resetvmail: resetvmail.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

account: account.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

acpro: acpro.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

redir: redir.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS) 

userno: userno.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

template: template.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

addsong: addsong.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

acl-sort: acl-sort.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

bbsmail: bbsmail.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

bquota: bquota.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

bquota-vacation: bquota-vacation.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

camera: camera.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

expire: expire.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

gem-check: gem-check.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

gem-index: gem-index.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

hdr-dump: hdr-dump.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

mailpost: mailpost.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

poststat: poststat.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

reaper: reaper.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

reaper-vacation: reaper-vacation.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

topusr: topusr.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

transbrd: transbrd.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

transusr: transusr.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

transman: transman.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

birth: birth.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

yearsold: yearsold.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

star: star.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

test: test.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

countbirth: countbirth.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

showACCT: showACCT.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

showBRD: showBRD.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

transacct: transacct.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

backup: backup.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

brdmail: brdmail.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

install: $(EXE)
	install -m 0700 $? $(HOME)/bin/

clean:
	rm -fr $(EXE) *.o *.bak *.BAK *.log *~ DEADJOE

clear:
	rm -fr *.bak *.BAK *.log *~ DEADJOE
