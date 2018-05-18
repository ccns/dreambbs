# ------------------------------------------------------ #
#  maple/Makefile  ( NTHU CS MapleBBS Ver 3.x )          #
# ------------------------------------------------------ #
#  author : opus.bbs@bbs.cs.nthu.edu.tw                  #
#  target : Makefile for MapleBBS main programs          #
#  create : 95/03/29                                     #
#  update : 18/03/28                                     #
# ------------------------------------------------------ #

ARCHI	!= uname -m

OPSYS	!= uname -o

SRCROOT	= ..

# rules ref: PttBBS: mbbsd/Makefile
BBSCONF		:= $(SRCROOT)/dreambbs.conf
DEF_PATTERN	:= ^[ \t]*\#[ \t]*define[ \t]*
DEF_CMD		:= grep -Ew "${DEF_PATTERN}"
DEF_YES		:= && echo "YES" || echo ""
USE_PMORE	!= sh -c '${DEF_CMD}"M3_USE_PMORE" ${BBSCONF} ${DEF_YES}'
#USE_PFTERM	!= sh -c '${DEF_CMD}"M3_USE_PFTERM" ${BBSCONF} ${DEF_YES}'

SRC 	= acct.c bbsd.c board.c cache.c edit.c \
	  gem.c mail.c menu.c more.c post.c banmail.c \
	  talk.c visio.c xover.c favorite.c socket.c popupmenu.c \
	  window.c myfavorite.c

OBJ	= acct.o bbsd.o board.o cache.o edit.o \
	  gem.o mail.o menu.o more.o post.o banmail.o \
	  talk.o visio.o xover.o favorite.o socket.o popupmenu.o \
	  window.o myfavorite.o 

.if $(USE_PMORE)
SRC	+= pmore.c 
OBJ	+= pmore.o
.endif

EXE	= bbsd xchatd

CC	= clang 

CPROTO	= cproto -E"clang -pipe -E" -I$(SRCROOT)/include

CFLAGS	= -g -O2 -pipe -fomit-frame-pointer -Wunused -Wno-invalid-source-encoding -I$(SRCROOT)/include

LDFLAGS	= -L../lib -ldao -lcrypt  

.if  $(ARCHI)=="x86_64" || $(ARCHI)=="amd64"
CFLAGS	+= -m32
LDFLAGS	+= -m32
.endif

.if $(OPSYS) == "GNU/Linux"
LDFLAGS	+= -lresolv -ldl -rdynamic
.endif

.if $(OPSYS) == "FreeBSD"
LDFLAGS	+= -Wl,-export-dynamic
.endif

.SUFFIXES: .o .c

.c.o:	; $(CC) $(CFLAGS) -c $*.c

all: 
	@$(MAKE) CC=$(CC) CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)" $(EXE)

xchatd:	xchatd.o
	$(CC) -o $@ $? $(LDFLAGS)

bbsd: $(OBJ)
	$(CC) $(MAKEFLAG) -o $@ $(OBJ) $(LDFLAGS) $(LIBS)

maple.p: $(SRC)
	$(CPROTO) -o $@ $?

install: $(EXE)
	install -m 0700 $? $(HOME)/bin

clean:
	rm -rf $(OBJ) $(EXE) $(LNFILES)  *~ *.o *.so *.p tags

tags:	$(SRC) ../include/*.h ../lib/*.c
	exctags $(SRC) $(SRCROOT)/include/*.h $(SRCROOT)/lib/*.c

