# ------------------------------------------------------ #
#  maple/Makefile  ( NTHU CS MapleBBS Ver 3.x )          #
# ------------------------------------------------------ #
#  author : opus.bbs@bbs.cs.nthu.edu.tw                  #
#  target : Makefile for MapleBBS main programs          #
#  create : 95/03/29                                     #
#  update : 18/03/28                                     #
# ------------------------------------------------------ #

MAKE	+= -f Makefile.gnu

OPSYS	:= $(shell uname -o)

ARCHI	:= $(shell uname -m)

# ------------------------------------------------------ #
# 下列的 make rules 不需修改                             #
# ------------------------------------------------------ #

SRC	= acct.c bbsd.c board.c cache.c edit.c\
	  gem.c mail.c menu.c more.c post.c banmail.c\
	  talk.c visio.c xover.c favorite.c socket.c popupmenu.c\
	  window.c myfavorite.c #pmore.c 
	  # If you want to define M3_USE_PMORE , add "pmore.c" on the end of last line

OBJ	= acct.o bbsd.o board.o cache.o edit.o\
	  gem.o mail.o menu.o more.o post.o banmail.o\
	  talk.o visio.o xover.o favorite.o socket.o popupmenu.o\
	  window.o myfavorite.o #pmore.o
	  # If you want to define M3_USE_PMORE , add "pmore.o" on the end of last line

EXE	= bbsd xchatd

CC	= clang

CPROTO	= cproto -E\"clang -pipe -E\" -I../include -I/usr/local/include

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

ifeq ($(OPSYS),GNU/Linux)
LDFLAGS	+= -lresolv -ldl -rdynamic 
else
ifeq ($(OPSYS),FreeBSD)
LDFLAGS	+= -Wl,-export-dynamic
endif
endif

.SUFFIXES: .o .c

.c.o:	; $(CC) $(CFLAGS) -c $*.c

all: 
	@$(MAKE) CC=$(CC) CPROTO="$(CPROTO)" CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)" $(EXE)

xchatd:	xchatd.o
	$(CC) -o $@ $? $(LDFLAGS)

bbsd: $(OBJ)
	$(CC) $(MAKEFLAG) -o $@ $(OBJ) $(LDFLAGS) $(LIBS)

maple.p: $(SRC)
	$(CPROTO) $> | sed '/ main(/d' > maple.p

install: $(EXE)
	install -m 0700 $? $(HOME)/bin

clean:
	rm -rf $(OBJ) $(EXE) $(LNFILES)  *~ *.o *.so

tags:	$(SRC) ../include/*.h ../lib/*.c
	exctags $(SRC) ../include/*.h ../lib/*.c

