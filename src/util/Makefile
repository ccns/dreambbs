# ------------------------------------------------------- #
#  util/Makefile	( NTHU CS MapleBBS Ver 2.36 )	  #
# ------------------------------------------------------- #
#  target : Makefile for 寄信、統計、備份、系統維護工具	  #
#  create : 95/03/29				 	  #
#  update : 95/12/15				 	  #
# ------------------------------------------------------- #

#CC	= gcc
#CFLAGS	= -O2 -pipe -I../include -fomit-frame-pointer -Wunused
#LDFLAGS = -s -lresolv -lsocket -L../lib -ldao
#LDFLAGS = -s -L../lib -ldao

# For Linux
# CFLAGS	= -O2 -pipe -fomit-frame-pointer -I../include -DLINUX
#CFLAGS	= -O2 -s -pipe -I../include -fomit-frame-pointer -fforce-mem -funroll-loops -fstrict-prototype -Wunused

.SUFFIXES: .o .c

.c.o:   ;   $(CC) $(MAKEFLAG) $(CFLAGS) -g -c $*.c

TOOL	= account acl-sort camera expire gem-check gem-index makeUSR \
	  hdr-dump poststat reaper webx countstar rmbadmail restorebrd \
	  mailpost bbsmail topusr acpro addsong userno template restoregem \
	  checkemail match mailexpire countbirth countage utmp-dump \
	  stopperm mailtoall clean_acl backupusr backupbrd makefw backupgem \
	  resetvmail 2nd_expire counter restoreusr makeusrno outgo redir \
	  flowlog news news_expire classtable_alert bmw-dump tranBRD \
	  brdstat dump-brdstat base64encode msgall transacct showACCT showBRD backup
	  #getstar \
	  #brdcheck brdmail 

all: 
	@echo "Please enter 'make sys-type', "
	@echo " make sun     : for Sun-OS 4.x and maybe some BSD systems, cc or gcc" 
	@echo " make linux   : for Linux"
	@echo " make solaris : for Sun-OS 5.x gcc" 
	@echo " make sol-x86 : for Solaris 7 x86"
	@echo " make freebsd : for BSD 4.4 systems" 
	@echo " make bsd     : for BSD systems, cc or gcc, if not in the above lists" 

sun:
	@$(MAKE) CC=gcc CFLAGS="-O2 -pipe -fomit-frame-pointer -Wunused -I../include" LDFLAGS="-s -L../lib -ldao" $(TOOL)

linux:
	@$(MAKE) CC=gcc CFLAGS="-DLINUX -DREDHAT -DREDHAT7 -O2 -pipe -I../include -fomit-frame-pointer -Wunused" LDFLAGS=" -s -L../lib -ldao -lcrypt -lresolv" $(TOOL)

solaris:
	@$(MAKE) CC=gcc CFLAGS="-DSOLARIS -O2 -pipe -I../include -fomit-frame-pointer -Wunused" LDFLAGS="-s -L../lib -ldao -lsocket -lresolv -lnsl -L/usr/ucblib -lucb" $(TOOL)

sol-x86:
	@$(MAKE) CC=gcc CFLAGS="-DSOLARIS -O2 -I../include -fomit-frame-pointer -Wunused" LDFLAGS="-s -L../lib -ldao -lsocket -lresolv -lnsl" $(TOOL)

freebsd:
	@$(MAKE) CC=gcc MAKEFLAG=$(MAKEFLAG) CFLAGS="-O2 -pipe -I../include -fomit-frame-pointer -Wunused" LDFLAGS="-s -L../lib -ldao -lcrypt" $(TOOL)

bsd:
	@$(MAKE) CC=gcc CFLAGS="-O2 -pipe -I../include -fomit-frame-pointer -Wunused" LDFLAGS="-s -L../lib -ldao" $(TOOL)

flowlog2: flowlog2.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

msgall: msgall.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

flowlog: flowlog.o
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

backupgem: backupgem.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

news_expire: news_expire.o
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

news: news.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

backupbrd: backupbrd.o
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

backupusr: backupusr.o
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

#ifsigned: ifsigned.o
#	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

mailpost: mailpost.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

poststat: poststat.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

reaper: reaper.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

reaper-vacation: reaper-vacation.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

webc: webc.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

webx: webx.o
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

#brdmail: brdmail.o
#	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

#brdcheck: brdcheck.o
#	        $(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

install: $(TOOL)
	install -m 0700 $? $(HOME)/bin/

clean:
	rm -fr $(TOOL) *.o *.bak *.BAK *.log *~ DEADJOE

clear:
	rm -fr *.bak *.BAK *.log *~ DEADJOE
