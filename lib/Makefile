# ------------------------------------------------------ #
#  lib/Makefile  ( NTHU CS MapleBBS Ver 3.00 )           #
# ------------------------------------------------------ #
#  author : opus.bbs@bbs.cs.nthu.edu.tw                  #
#  target : Makefile for MapleBBS library routines       #
#  create : 95/03/29                                     #
#  update : 95/12/15                                     #
# ------------------------------------------------------ #

SRCROOT	= ..
.include "$(SRCROOT)/dreambbs.mk"

HDR 	= dao.h dao.p

SRC	= acl.c       chrono32.c  file.c    isnot.c    radix32.c   shm.c   \
	  archiv32.c  dl_lib.c    record.c  splay.c    date_str.c  xsort.c \
	  attr_lib.c  dns.c       header.c  rfc2047.c  string.c    xwrite.c

OBJ	= acl.o       chrono32.o  file.o    isnot.o    radix32.o   shm.o   \
	  archiv32.o  dl_lib.o    record.o  splay.o    date_str.o  xsort.o \
	  attr_lib.o  dns.o       header.o  rfc2047.o  string.o    xwrite.o

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
