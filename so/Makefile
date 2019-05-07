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
.include "./so.mk"

.if $(USE_BBSLUA)
CFLAGS	+= ${LUA_CFLAGS}
.endif

.if $(USE_BBSRUBY)
CFLAGS	+= ${RUBY_CFLAGS}
.endif

.SUFFIXES: .o .c .ln .x .so

.c.o:	;   $(CC) $(MAKEFLAG) $(CFLAGS) -c $*.c
.c.x:	;   $(CPROTO) -o $*.x $*.c
.c.ln:	;   lint -abhi $*.c

.if    $(ARCHI)=="64"
.if      $(OPSYS) == "FreeBSD"
.o.so:	;   ld -G $*.o -o $*.so -L$(SRCROOT)/lib -ldao -L/usr/lib32 -L/lib32 -lc -melf_i386_fbsd
.else
.o.so:	;   ld -G $*.o -o $*.so -L$(SRCROOT)/lib -ldao -L/usr/lib32 -L/lib32 -lc -melf_i386
.endif   #OPSYS
.else
.o.so:  ;   ld -G $*.o -o $*.so -L$(SRCROOT)/lib -ldao -lc
.endif #ARCHI

all: $(SO)

install: $(SO)
	install -m 0700 $(SO) $(BBSHOME)/bin$(BBSVER)

clean: /tmp
	rm -fr $(SO) *~ *.o
