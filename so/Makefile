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

SO =	chat.so vote.so xyz.so guessnum.so \
	admin.so chatmenu.so  mailgem.so\
	memorandum.so aloha.so newboard.so violate.so song.so same_mail.so\
	showvote.so list.so mine.so bj.so \
	pnote.so passwd.so adminutil.so ascii.so\
	classtable2.so observe.so pip.so brdstat.so personal.so \
	cleanrecommend.so shop.so bank.so innbbs.so \

EXE =	so

ETC =	Makefile $(HDR)

.SUFFIXES: .o .c .ln .x .so

.c.o:   ;   $(CC) $(MAKEFLAG) $(CFLAGS) -c $*.c
.c.x:   ;   $(CPROTO) -o $*.x $*.c
.c.ln:  ;   lint -abhi $*.c
.o.so:	;   ld -s -G $*.o -o $*.so -L../lib -ldao -melf_i386


all: 
	@echo "Please enter 'make sys-type', "
	@echo " make linux   : for Linux"
	@echo " make freebsd : for BSD 4.4 systems"

linux:
	@$(MAKE) CC=gcc CPROTO="cproto -E\"gcc -pipe -E\" -I../include" CFLAGS="-g -m32 -DLINUX -DREDHAT -DREDHAT7 -O2 -pipe -fomit-frame-pointer -Wunused -I../include" LDFLAGS="-m32 -L../lib -ldao -lcrypt -lresolv -ldl -rdynamic" $(EXE)

freebsd:
	@$(MAKE) CC=gcc CPROTO="cproto -E\"gcc -pipe -E\" -I../include" MAKEFLAG=$(MAKEFLAG) CFLAGS="-g -DBSD44 -DTWO_LONGJMP_ARGS -O2 -pipe -fomit-frame-pointer -Wunused -I../include" LDFLAGS="-L../lib -ldao -lcrypt -export-dynamic" $(EXE)

so: $(SO)

install: $(EXE)
	install -m 0700 $(SO) $(HOME)/bin

clean: /tmp
	rm -fr $(SO) *~ *.o DEADJOE
