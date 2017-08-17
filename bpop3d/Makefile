# ------------------------------------------------------- #
#  bpop3d/Makefile	( NTHU CS MapleBBS Ver 2.36 )	  #
# ------------------------------------------------------- #
#  target : Makefile for 寄信、統計、備份、系統維護工具	  #
#  create : 95/03/29				 	  #
#  update : 95/12/15				 	  #
# ------------------------------------------------------- #

#CC	= gcc
#CFLAGS	= -O2 -pipe -I../include -fomit-frame-pointer -Wunused #-DSOLARIS
#LDFLAGS = -s -L../lib -ldao -lcrypt #-lsocket

.SUFFIXES: .o .c

.c.o:   ;   $(CC) $(MAKEFLAG) $(CFLAGS) -c $*.c

EXE	= bpop3d

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
	@$(MAKE) CC=gcc CFLAGS="-DLINUX -O2 -pipe -I../include -fomit-frame-pointer -Wunused" LDFLAGS="-s -L../lib -ldao -lcrypt" $(EXE)

solaris:
	@$(MAKE) CC=gcc CFLAGS="-DSOLARIS -O2 -pipe -I../include -fomit-frame-pointer -Wunused" LDFLAGS="-s -L../lib -ldao -lsocket -lresolv -lnsl" $(EXE)

sol-x86:
	@$(MAKE) CC=gcc CFLAGS="-DSOLARIS -O2 -I../include -fomit-frame-pointer -Wunused" LDFLAGS="-s -L../lib -ldao -lsocket -lresolv -lnsl" $(EXE)

freebsd:
	@$(MAKE) CC=gcc MAKEFLAG=$(MAKEFLAG) CFLAGS="-O2 -pipe -I../include -fomit-frame-pointer -Wunused" LDFLAGS="-s -L../lib -ldao -lcrypt" $(EXE)

bsd:
	@$(MAKE) CC=gcc CFLAGS="-O2 -pipe -I../include -fomit-frame-pointer -Wunused" LDFLAGS="-s -L../lib -ldao" $(EXE)

bpop3d: bpop3d.o
	$(CC) $(MAKEFLAG) -o $@ $? $(CFLAGS) $(LDFLAGS)

install: $(EXE)
	install -m 0700 $? $(HOME)/bin/

clean:
	rm -fr $(EXE) *.o *.bak *.BAK *.log *~
