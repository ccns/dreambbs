# ------------------------------------------------------ #
#  so/Makefile ( NTHU CS MapleBBS Ver 3.x )              #
# ------------------------------------------------------ #
#  author : opus.bbs@bbs.cs.nthu.edu.tw                  #
#  target : Makefile for MapleBBS main programs	         #
#  create : 95/03/29                                     #
#  update : 18/03/28                                     #
# ------------------------------------------------------ #

MAKE	+=  -f Makefile.gnu

OPSYS	:= $(shell uname -o)

ARCHI	:= $(shell uname -m)

HDR	= bbs.h config.h global.h modes.h perm.h struct.h

SO	= chat.so vote.so xyz.so guessnum.so \
	  admin.so chatmenu.so  mailgem.so\
	  memorandum.so aloha.so newboard.so violate.so song.so same_mail.so\
	  showvote.so list.so mine.so bj.so \
	  pnote.so passwd.so adminutil.so ascii.so\
	  classtable2.so observe.so pip.so brdstat.so personal.so \
	  cleanrecommend.so shop.so bank.so innbbs.so contact.so 

CC	= clang

CPROTO	= cproto -E\"clang -pipe -E\" -I../include

CFLAGS	= -g -O2 -pipe -fomit-frame-pointer -Wunused -Wno-invalid-source-encoding -I../include

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

ifeq ($(OPSYS),Linux)
LDFLAGS	+= -rdynamic -lresolv -ldl
else
ifeq ($(OPSYS),FreeBSD)
LDFLAGS	+= -Wl,-export-dynamic
endif
endif

EXE = so

ETC = Makefile $(HDR)


.SUFFIXES: .o .c .ln .x .so

.c.o:	; $(CC) $(MAKEFLAG) $(CFLAGS) -c $*.c
.c.x:	; $(CPROTO) -o $*.x $*.c
.c.ln:	; lint -abhi $*.c

ifeq ($(OPSYS),FreeBSD)
.o.so:	; ld -G $*.o -o $*.so -L../lib -ldao -melf_i386_fbsd
else
.o.so:	; ld -G $*.o -o $*.so -L../lib -ldao -melf_i386
endif

all: 
	@$(MAKE) CC=$(CC) CPROTO="$(CPROTO)" CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)" $(EXE)

so: $(SO)

install: $(EXE)
	install -m 0700 $(SO) $(HOME)/bin

clean: /tmp
	rm -fr $(SO) *~ *.o
