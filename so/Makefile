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

.if $(USE_BBSLUA)
.if $(OPSYS) == "FreeBSD"
    LUA_PKG_NAME	?= lua-5.1
.else
    LUA_PKG_NAME	?= lua5.1
.endif

LUA_CFLAGS	!= pkg-config --cflags ${LUA_PKG_NAME}
CFLAGS	+= ${LUA_CFLAGS}
.endif


.if $(USE_BBSRUBY)
.if $(ARCHI) == "64"
    RUBY_CFLAGS_CMD	= | sed 's/x86_64/i386/'
.endif

RUBY_CFLAGS	!= pkg-config --cflags ruby-2.2 ${RUBY_CFLAGS_CMD}
CFLAGS	+= ${RUBY_CFLAGS}
.endif

HDR	= bbs.h config.h global.h modes.h perm.h struct.h bbs_script.h

SO	= chat.so vote.so xyz.so guessnum.so \
	  admin.so chatmenu.so  mailgem.so\
	  memorandum.so aloha.so newboard.so violate.so song.so same_mail.so\
	  showvote.so list.so mine.so bj.so \
	  pnote.so passwd.so adminutil.so ascii.so\
	  classtable2.so observe.so pip.so brdstat.so personal.so \
	  cleanrecommend.so shop.so bank.so innbbs.so contact.so

EXE	= so

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

all:
	@$(MAKE) CC=$(CC) CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)" $(EXE)

so: $(SO)

install: $(EXE)
	install -m 0700 $(SO) $(BBSHOME)/bin$(BBSVER)

clean: /tmp
	rm -fr $(SO) *~ *.o
