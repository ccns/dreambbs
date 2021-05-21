# ------------------------------------------------------ #
#  so/Makefile ( NTHU CS MapleBBS Ver 3.x )              #
# ------------------------------------------------------ #
#  author : opus.bbs@bbs.cs.nthu.edu.tw                  #
#  target : Makefile for MapleBBS main programs          #
#  create : 95/03/29                                     #
#  update : 18/03/28                                     #
# ------------------------------------------------------ #

SRCROOT	= $(.CURDIR)/..
.include "$(SRCROOT)/dreambbs.mk"
.include "./so.mk"

.if $(USE_BBSLUA)
CFLAGS	+= $(LUA_CFLAGS)
.endif

.if $(USE_BBSRUBY)
CFLAGS	+= $(RUBY_CFLAGS)
.endif

.SUFFIXES: .o .c .ln .x .so

CFLAGS_SO += -fPIC

.c.o:	;   $(CC) $(MAKEFLAG) $(CFLAGS) $(CFLAGS_SO) -DDL_CURRENT_MODULE_STR="\"$*.so\"" -c $*.c
.c.x:	;   $(CPROTO) -o $*.x $*.c
.c.ln:	;   lint -abhi $*.c

LDFLAGS_SO = -fPIC -shared -Wl,-G

LDFLAGS_SO += -L$(SRCROOT)/lib -ldao

.o.so:  ;   $(CC) $*.o -o $*.so $(LDFLAGS_SO) $(LDFLAGS_SO_ARCHI)

.if $(USE_BBSLUA)
BBSLUA_OBJ	= bbslua.o
.if !$(USE_LUAJIT)
BBSLUA_OBJ	+= bbsluaext.o
.endif

bbslua.so: $(BBSLUA_OBJ)
	$(CC) $*.o -o $*.so $(LDFLAGS_SO) $(LDFLAGS_SO_ARCHI) $(LUA_LDFLAGS) $(LUA_LDFLAGS_ARCHI)
.endif

.if $(USE_BBSRUBY)
BBSRUBY_OBJ	= bbsruby.o
bbsruby.so: $(BBSRUBY_OBJ)
	$(CC) $*.o -o $*.so $(LDFLAGS_SO) $(LDFLAGS_SO_ARCHI) $(RUBY_LDFLAGS) $(RUBY_LDFLAGS_ARCHI)
.endif

all: $(SO)

install: $(SO)
	$(INSTALL) -m 0700 $(SO) "$(BBSHOME)/bin$(BBSVER)"

clean: /tmp
	rm -fr $(SO) *~ *.o
