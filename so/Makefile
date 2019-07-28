# ------------------------------------------------------ #
#  so/Makefile ( NTHU CS MapleBBS Ver 3.x )              #
# ------------------------------------------------------ #
#  author : opus.bbs@bbs.cs.nthu.edu.tw                  #
#  target : Makefile for MapleBBS main programs          #
#  create : 95/03/29                                     #
#  update : 18/03/28                                     #
# ------------------------------------------------------ #

SRCROOT	= ..
.include "$(SRCROOT)/dreambbs.mk"

HDR	= bbs.h config.h global.h modes.h perm.h struct.h

SO	= chat.so vote.so xyz.so guessnum.so \
	  admin.so chatmenu.so  mailgem.so\
	  memorandum.so aloha.so newboard.so violate.so song.so same_mail.so\
	  showvote.so list.so mine.so bj.so \
	  pnote.so passwd.so adminutil.so ascii.so\
	  classtable2.so observe.so pip.so brdstat.so personal.so \
	  cleanrecommend.so shop.so bank.so innbbs.so contact.so

.SUFFIXES: .o .c .ln .x .so

.c.o:	;   $(CC) $(MAKEFLAG) $(CFLAGS) -c $*.c
.c.x:	;   $(CPROTO) -o $*.x $*.c
.c.ln:	;   lint -abhi $*.c

.if    $(ARCHI)=="64"
.if      $(OPSYS) == "FreeBSD"
.o.so:	;   ld -G $*.o -o $*.so -L../lib -ldao -melf_i386_fbsd
.else
.o.so:	;   ld -G $*.o -o $*.so -L../lib -ldao -melf_i386
.endif   #OPSYS
.else
.o.so:  ;   ld -G $*.o -o $*.so -L../lib -ldao
.endif #ARCHI

all: $(SO)

install: $(SO)
	install -m 0700 $(SO) "$(BBSHOME)/bin$(BBSVER)"

clean: /tmp
	rm -fr $(SO) *~ *.o
