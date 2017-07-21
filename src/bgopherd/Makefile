# ------------------------------------------------------- #
#  util/Makefile	( NTHU CS MapleBBS Ver 2.36 )	  #
# ------------------------------------------------------- #
#  target : Makefile for 寄信、統計、備份、系統維護工具	  #
#  create : 95/03/29				 	  #
#  update : 95/12/15				 	  #
# ------------------------------------------------------- #


#CC	= gcc
#CFLAGS	= -O2 -pipe -s -fomit-frame-pointer -I../include #-DSOLARIS -lsocket

# For Linux
# CFLAGS	= -O2 -pipe -s -fomit-frame-pointer -I../maple -DLINUX

.c.o:   ;   $(CC) $(MAKEFLAG) $(CFLAGS) -c $*.c


EXE	= gemd

#all: $(EXE)

all: 
	@echo "Please enter 'make sys-type', "
	@echo " make sun     : for Sun-OS 4.x and maybe some BSD systems, cc or gcc" 
	@echo " make linux   : for Linux"
	@echo " make solaris : for Sun-OS 5.x gcc" 
	@echo " make sol-x86 : for Solaris 7 x86"
	@echo " make freebsd : for BSD 4.4 systems" 
	@echo " make bsd     : for BSD systems, cc or gcc, if not in the above lists" 

sun:
	@$(MAKE) CC=gcc CFLAGS="-O2 -pipe -s -fomit-frame-pointer -Wunused -I../include" $(EXE)

linux:
	@$(MAKE) CC=gcc CFLAGS="-DLINUX -O2 -pipe -s -fomit-frame-pointer -I../include" $(EXE)

solaris:
	@$(MAKE) CC=gcc CFLAGS="-DSOLARIS -O2 -pipe -s -I../include -fomit-frame-pointer -Wunused -lsocket -lnsl -lresolv -lelf" $(EXE)

sol-x86:
	@$(MAKE) CC=gcc CFLAGS="-DSOLARIS -O2 -s -I../include -fomit-frame-pointer -Wunused -lsocket -lnsl -lresolv -lelf" $(EXE)

freebsd:
	@$(MAKE) CC=gcc MAKEFLAG=$(MAKEFLAG) CFLAGS="-O2 -pipe -s -fomit-frame-pointer -Wunused -I../include" $(EXE)

bsd:
	@$(MAKE) CC=gcc CFLAGS="-O2 -pipe -s -fomit-frame-pointer -I../include" $(EXE)

gemd: gemd.o
	$(CC) $(MAKEFLAG) -o $@ $? $(CFLAGS) $(LDFLAGS)

install: $(EXE)
	install -m 0700 $? $(HOME)/bin/

clean:
	rm -fr $(EXE) *.o *.bak *.BAK *.log *~
