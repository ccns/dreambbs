# ------------------------------------------------------ #
#  Makefile	( NTHU CS MapleBBS Ver 2.36 )		 #
# ------------------------------------------------------ #
#  author : opus.bbs@bbs.cs.nthu.edu.tw		 	 #
#  target : Makefile for MapleBBS main programs		 #
#  create : 95/03/29				 	 #
#  update : 95/12/15					 #
# ------------------------------------------------------ #


STRIP   = /usr/bin/strip

# ------------------------------------------------------ #
# 下列的 make rules 不需修改				 #
# ------------------------------------------------------ #

SRC =	acct.c bbsd.c board.c cache.c edit.c\
	gem.c mail.c menu.c more.c post.c banmail.c\
	talk.c visio.c xover.c favorite.c socket.c popupmenu.c\
	pmore.c window.c myfavorite.c\
	# If you prefer not to define M3_USE_PMORE , remove "pmore.c" on the start of last line

OBJ =	acct.o bbsd.o board.o cache.o edit.o\
	gem.o mail.o menu.o more.o post.o banmail.o\
	talk.o visio.o xover.o favorite.o socket.o popupmenu.o\
	pmore.o window.o myfavorite.o\
	# If you prefer not to define M3_USE_PMORE , remove "pmore.o" on the start of last line

EXE =	bbsd xchatd


CPROTO  = `which cproto` -E"gcc -pipe -E" -I../include -I/usr/local/include

.SUFFIXES: .o .c

.c.o:	;	$(CC) $(CFLAGS) -c $*.c

all: 
	@echo "Please enter 'make sys-type', "
	@echo " make sun     : for Sun-OS 4.x and maybe some BSD systems, cc or gcc" 
	@echo " make linux   : for Linux"
	@echo " make solaris : for Sun-OS 5.x gcc" 
	@echo " make sol-x86 : for Solaris 7 x86"
	@echo " make freebsd : for BSD 4.4 systems"
	@echo " make bsd     : for BSD systems, cc or gcc, if not in the above lists"

sun:
	@$(MAKE) CC=gcc CFLAGS="-O2 -pipe -fomit-frame-pointer -Wunused -I../include" LDFLAGS="-s -L../lib -ldao" $(EXE)

linux:
	@$(MAKE) CC=gcc CPROTO="cproto -E\"gcc -pipe -E\" -I../include" CFLAGS="-m32 -DLINUX -O2 -pipe -fomit-frame-pointer -Wunused -I../include" LDFLAGS="-m32 -s -L../lib -ldao -lcrypt -lresolv -ldl -rdynamic" $(EXE)

solaris:
	@$(MAKE) CC=gcc CFLAGS="-DSOLARIS -DSYSV -O2 -pipe -fomit-frame-pointer -Wunused -I../include" LDFLAGS="-s -L../lib -ldao -ltermcap -lsocket -lnsl -lresolv -lelf -ldl" $(EXE)

sol-x86:
	@$(MAKE) CC=gcc CFLAGS="-DSOLARIS -DSYSV -O2 -fomit-frame-pointer -Wunused -I../include" LDFLAGS="-s -L../lib -ldao -ltermcap -lsocket -lnsl -lresolv -lelf -ldl" $(EXE)

freebsd:
	@$(MAKE) CC=gcc CFLAGS="-DBSD44 -Os -pipe -fomit-frame-pointer -Wunused -I../include" LDFLAGS="-s -L../lib -ldao -lcrypt -export-dynamic" $(EXE)

bsd:
	@$(MAKE) CC=gcc CFLAGS="-DBSD44 -O2 -pipe -fomit-frame-pointer -Wunused -I../include" LDFLAGS="-s -L../lib -ldao" $(EXE)

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

