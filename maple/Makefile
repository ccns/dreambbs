# ------------------------------------------------------ #
#  maple/Makefile  ( NTHU CS MapleBBS Ver 3.x )          #
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
    LDFLAGS	+= -Wl,--no-as-needed
.else
    LUA_PKG_NAME	?= lua5.1
.endif

LUA_CFLAGS	!= pkg-config --cflags ${LUA_PKG_NAME}
LUA_LDFLAGS	!= pkg-config --libs ${LUA_PKG_NAME}
CFLAGS	+= ${LUA_CFLAGS}
LDFLAGS	+= ${LUA_LDFLAGS}
SRC	+= bbslua.c bbsluaext.c
OBJ	+= bbslua.o bbsluaext.o
.endif


.if $(USE_BBSRUBY)
.if $(OPSYS) == "FreeBSD"
    LDFLAGS	+= -Wl,--no-as-needed
.endif

RUBY_CFLAGS	!= pkg-config --cflags ruby
RUBY_LDFLAGS	!= pkg-config --libs ruby
CFLAGS	+= ${RUBY_CFLAGS}
LDFLAGS	+= ${RUBY_LDFLAGS}
SRC	+= bbsruby.c
OBJ	+= bbsruby.o
.endif


.if $(USE_PFTERM)
SRC	+= pfterm.c
OBJ	+= pfterm.o
.endif

SRC 	+= acct.c bbsd.c board.c cache.c edit.c \
	  gem.c mail.c menu.c more.c post.c banmail.c \
	  talk.c visio.c xover.c favorite.c socket.c popupmenu.c \
	  window.c myfavorite.c

OBJ	+= acct.o bbsd.o board.o cache.o edit.o \
	  gem.o mail.o menu.o more.o post.o banmail.o \
	  talk.o visio.o xover.o favorite.o socket.o popupmenu.o \
	  window.o myfavorite.o

.if $(USE_PMORE)
SRC	+= pmore.c
OBJ	+= pmore.o
.endif

EXE	= bbsd xchatd

.SUFFIXES: .o .c

.c.o:	; $(CC) $(CFLAGS) -c $*.c

all:
	@$(MAKE) CC=$(CC) CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)" $(EXE)

xchatd: xchatd.o
	$(CC) -o $@ $? $(LDFLAGS)

bbsd: $(OBJ)
	$(CC) $(MAKEFLAG) -o $@ $(OBJ) $(LDFLAGS) $(LIBS)

maple.p: $(SRC)
	$(CPROTO) -o $@ $?

install: $(EXE)
	install -d $(BBSHOME)/bin$(BBSVER)
	install -m 0755 $? $(BBSHOME)/bin$(BBSVER)
	mv -f $(BBSHOME)/bin$(BBSVER)/bbsd $(BBSHOME)/bin$(BBSVER)/bbsd.$(BUILDTIME)
	ln -sv $(BBSHOME)/bin$(BBSVER)/bbsd.$(BUILDTIME) $(BBSHOME)/bin$(BBSVER)/bbsd

clean:
	rm -rf $(OBJ) $(EXE) $(LNFILES)  *~ *.o *.so *.p tags

tags: $(SRC) ../include/*.h ../lib/*.c
	exctags $(SRC) $(SRCROOT)/include/*.h $(SRCROOT)/lib/*.c

