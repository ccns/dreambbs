# ------------------------------------------------------ #
#  Makefile	( NTHU CS MapleBBS Ver 2.36 )		 #
# ------------------------------------------------------ #
#  author : opus.bbs@bbs.cs.nthu.edu.tw		 	 #
#  target : Makefile for MapleBBS main programs		 #
#  create : 95/03/29				 	 #
#  update : 95/12/15					 #
# ------------------------------------------------------ #

CFLAGS  = -O2 -o
STRIP   = /usr/bin/strip

# ------------------------------------------------------ #
# 下列的 make rules 不需修改				 #
# ------------------------------------------------------ #

HDR = 	bbs.h config.h global.h modes.h perm.h struct.h

SO =	chat.so vote.so xyz.so bwboard.so guessnum.so bbcall.so railway.so puzzle.so\
	emailpage.so chess.so admin.so chatmenu.so contact.so mailgem.so\
	memorandum.so aloha.so newboard.so violate.so song.so same_mail.so\
	classtable.so lovepaper.so showvote.so sec_hand.so list.so km.so\
	pnote.so passwd.so pop3mail.so imap4mail.so adminutil.so ascii.so\
	ueequery.so fortune.so ueequery2.so mine.so bbsnet.so bj.so pydict.so\
	dreye.so news_viewer.so icq.so star.so dictd.so classtable2.so \
	observe.so pip.so grade.so qkmj.so brdstat.so netwhois.so personal.so \
	cleanrecommend.so shop.so bank.so innbbs.so \

EXE =	so

ETC =	Makefile $(HDR)

.SUFFIXES: .o .c .ln .x .so

.c.o:   ;   $(CC) $(MAKEFLAG) $(CFLAGS) -c $*.c
.c.x:   ;   $(CPROTO) -o $*.x $*.c
.c.ln:  ;   lint -abhi $*.c
.o.so:	;   ld -s -G $*.o -o $*.so -L../lib -ldao


all: 
	@echo "Please enter 'make sys-type', "
	@echo " make sun     : for Sun-OS 4.x and maybe some BSD systems, cc or gcc" 
	@echo " make linux   : for Linux"
	@echo " make solaris : for Sun-OS 5.x gcc" 
	@echo " make sol-x86 : for Solaris 7 x86"
	@echo " make freebsd : for BSD 4.4 systems"
	@echo " make bsd     : for BSD systems, cc or gcc, if not in the above lists"

sun:
	@$(MAKE) CC=gcc CPROTO="cproto -E\"gcc -pipe -E\" -I../include" CFLAGS="-O2 -pipe -fomit-frame-pointer -Wunused -I../include" LDFLAGS="-s -L../lib -ldao" $(EXE)

linux:
	@$(MAKE) CC=gcc CPROTO="cproto -E\"gcc -pipe -E\" -I../include" CFLAGS="-DLINUX -DREDHAT -DREDHAT7 -O2 -pipe -fomit-frame-pointer -Wunused -I../include" LDFLAGS="-s -L../lib -ldao -lcrypt -lresolv -ldl -rdynamic" $(EXE)

solaris:
	@$(MAKE) CC=gcc CPROTO="cproto -E\"gcc -pipe -E\" -I../include" CFLAGS="-DSOLARIS -DSYSV -DREMAP_LSEEK -O2 -pipe -fomit-frame-pointer -Wunused -I../include" LDFLAGS="-s -L../lib -ldao -ltermcap -lsocket -lnsl -lresolv -lelf -ldl" $(EXE)

sol-x86:
	@$(MAKE) CC=gcc CPROTO="cproto -E\"gcc -E\" -I../include" CFLAGS="-DSOLARIS -DSYSV -DREMAP_LSEEK -O2 -fomit-frame-pointer -Wunused -I../include" LDFLAGS="-s -L../lib -ldao -ltermcap -lsocket -lnsl -lresolv -lelf -ldl" $(EXE)

freebsd:
	@$(MAKE) CC=gcc CPROTO="cproto -E\"gcc -pipe -E\" -I../include" MAKEFLAG=$(MAKEFLAG) CFLAGS="-DBSD44 -DTWO_LONGJMP_ARGS -O2 -pipe -fomit-frame-pointer -Wunused -I../include" LDFLAGS="-s -L../lib -ldao -lcrypt -export-dynamic" $(EXE)

bsd:
	@$(MAKE) CC=gcc CPROTO="cproto -E\"gcc -pipe -E\" -I../include" CFLAGS="-DBSD44 -DTWO_LONGJMP_ARGS -O2 -pipe -fomit-frame-pointer -Wunused -I../include" LDFLAGS="-s -L../lib -ldao" $(EXE)

so: $(SO)

install: $(EXE)
	install -m 0700 $(SO) $(HOME)/bin

clean: /tmp
	rm -fr $(SO) *~ *.o DEADJOE
