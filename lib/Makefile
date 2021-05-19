# ------------------------------------------------------ #
#  lib/Makefile  ( NTHU CS MapleBBS Ver 3.00 )           #
# ------------------------------------------------------ #
#  author : opus.bbs@bbs.cs.nthu.edu.tw                  #
#  target : Makefile for MapleBBS library routines       #
#  create : 95/03/29                                     #
#  update : 95/12/15                                     #
# ------------------------------------------------------ #

SRCROOT	= $(.CURDIR)/..
.include "$(SRCROOT)/dreambbs.mk"

HDR 	= dao.h dao.p

SRC	= acl.c       file.c    isnot.c    radix32.c   shm.c   \
	  dl_lib.c    record.c  splay.c    date.c      xsort.c \
	  attr_lib.c  dns.c       header.c  string.c    xwrite.c \
	  proc.c      passwd.c  mime.c     logger.c    acct.c

OBJ	= acl.o       file.o    isnot.o    radix32.o   shm.o   \
	  dl_lib.o    record.o  splay.o    date.o      xsort.o \
	  attr_lib.o  dns.o       header.o  string.o    xwrite.o \
	  proc.o      passwd.o  mime.o     logger.o    acct.c

.c.o:   ;$(CC) $(CFLAGS) -c $*.c

all:	libdao.a

dao.p:	$(SRC)
	$(CPROTO) $> | sed '/querybuf/d' > dao.p

libdao.a: $(OBJ)
	ar rv $@ $?
	$(RANLIB) $@

clean:
	rm -fr $(OBJ) *.a *.bak *.BAK *~ *.p tags

tags:
	ctags $(SRC) $(SRCROOT)/include/*.h
