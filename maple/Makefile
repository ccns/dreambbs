# ------------------------------------------------------ #
#  maple/Makefile  ( NTHU CS MapleBBS Ver 3.x )          #
# ------------------------------------------------------ #
#  author : opus.bbs@bbs.cs.nthu.edu.tw                  #
#  target : Makefile for MapleBBS main programs          #
#  create : 95/03/29                                     #
#  update : 18/03/28                                     #
# ------------------------------------------------------ #

UNAME	!= uname

# ------------------------------------------------------ #
# 下列的 make rules 不需修改                             #
# ------------------------------------------------------ #

SRC 	= acct.c bbsd.c board.c cache.c edit.c \
	  gem.c mail.c menu.c more.c post.c banmail.c \
	  talk.c visio.c xover.c favorite.c socket.c popupmenu.c \
	  pmore.c window.c myfavorite.c
	  # If you prefer not to define M3_USE_PMORE , remove "pmore.c" on the start of last line

OBJ	= acct.o bbsd.o board.o cache.o edit.o \
	  gem.o mail.o menu.o more.o post.o banmail.o \
	  talk.o visio.o xover.o favorite.o socket.o popupmenu.o \
	  pmore.o window.o myfavorite.o 
	# If you prefer not to define M3_USE_PMORE , remove "pmore.o" on the start of last line

EXE	= bbsd xchatd

CC	= clang 

CPROTO	= cproto -E\"clang -pipe -E\" -I../include -I/usr/local/include

CFLAGS	= -g -m32 -O2 -pipe -fomit-frame-pointer -Wunused -I../include

LDFLAGS	= -m32 -L../lib -ldao -lcrypt  

.if $(UNAME) == "Linux"
LDFLAGS	+= -lresolv -ldl -rdynamic
.endif

.if $(UNAME) == "FreeBSD"
LDFLAGS	+= -Wl,-export-dynamic
.endif

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
	rm -rf $(OBJ) $(EXE) $(LNFILES)  *~ *.o *.so DEADJOE

tags:	$(SRC) ../include/*.h ../lib/*.c
	exctags $(SRC) ../include/*.h ../lib/*.c

