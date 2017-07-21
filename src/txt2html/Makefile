# ------------------------------------------------------ #
#  Makefile	( YZU WindTopBBS Ver 3.XX )		 #
# ------------------------------------------------------ #
#  author : visor.bbs@bbs.yzu.edu.tw		 	 #
#  target : Makefile for txt2html main programs	 	 #
#  create : 					 	 #
#  update : 						 #
# ------------------------------------------------------ #

CFLAGS  = -O2 -o

# ------------------------------------------------------ #
# 下列的 make rules 不需修改				 #
# ------------------------------------------------------ #

SRC =	txt2html.c getopt.c getopt1.c

OBJ = 	txt2html.o getopt.o getopt1.o

EXE =	txt2html

ETC =	Makefile $(HDR)

.SUFFIXES: .o .c .so

.c.o:   ;   $(CC) $(CFLAGS) -c $*.c
.o.so:	;   ld -s -G $*.o -o $*.so -L../lib -ldao

all: 
	@echo "Please enter 'make sys-type', "
	@echo " make sun     : for Sun-OS 4.x and maybe some BSD systems, cc or gcc" 
	@echo " make linux   : for RedHat 6.0"
	@echo " make solaris : for Sun-OS 5.x gcc" 
	@echo " make sol-x86 : for Solaris 7 x86"
	@echo " make freebsd : for BSD 4.4 systems"
	@echo " make bsd     : for BSD systems, cc or gcc, if not in the above lists"

sun:
	@$(MAKE) CC=gcc CPROTO="cproto -E\"gcc -pipe -E\" -I../include" CFLAGS="-O2 -pipe -fomit-frame-pointer -Wunused -I../include" LDFLAGS="-s -L../lib -ldao" $(EXE)

linux:
	@$(MAKE) CC=gcc CPROTO="cproto -E\"gcc -pipe -E\" -I../include" CFLAGS="-DLINUX -DREDHAT -O2 -pipe -fomit-frame-pointer -Wunused -I../include" LDFLAGS="-s -L../lib -ldao -lcrypt -lresolv -ldl -rdynamic" $(EXE)

solaris:
	@$(MAKE) CC=gcc CPROTO="cproto -E\"gcc -pipe -E\" -I../include" CFLAGS="-DSOLARIS -DSYSV -DREMAP_LSEEK -O2 -pipe -fomit-frame-pointer -Wunused -I../include" LDFLAGS="-s -L../lib -ldao -ltermcap -lsocket -lnsl -lresolv -lelf -ldl" $(EXE)

sol-x86:
	@$(MAKE) CC=gcc CPROTO="cproto -E\"gcc -E\" -I../include" CFLAGS="-DSOLARIS -DSYSV -DREMAP_LSEEK -O2 -fomit-frame-pointer -Wunused -I../include" LDFLAGS="-s -L../lib -ldao -ltermcap -lsocket -lnsl -lresolv -lelf -ldl" $(EXE)

freebsd:
	@$(MAKE) CC=gcc CPROTO="cproto -E\"gcc -pipe -E\" -I../include" CFLAGS="-DBSD44 -DTWO_LONGJMP_ARGS -O2 -pipe -fomit-frame-pointer -Wunused -I../include" LDFLAGS="-s -L../lib -ldao -lcrypt -export-dynamic" $(EXE)

bsd:
	@$(MAKE) CC=gcc CPROTO="cproto -E\"gcc -pipe -E\" -I../include" CFLAGS="-DBSD44 -DTWO_LONGJMP_ARGS -O2 -pipe -fomit-frame-pointer -Wunused -I../include" LDFLAGS="-s -L../lib -ldao" $(EXE)

#so: $(SO)

txt2html: $(OBJ)
	$(CC) -o $@ $(OBJ) $(LDFLAGS) $(LIBS)

install: $(EXE)
	csh -c "cp ~bbs/src/txt2html/txt2html ~bbs/bin"

clean: /tmp
	rm -fr $(GARBAGE) $(OBJ) $(EXE) $(LNFILES) $(PRO) proto.x *~ *.o *.so DEADJOE *.core

clear: /tmp
	rm -rf *~ DEADJOE

