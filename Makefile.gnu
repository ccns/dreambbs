# ------------------------------------------------------- #
#  Makefile  ( NTHU CS MapleBBS Ver 3.x )                 #
# ------------------------------------------------------- #
#  target : Makefile for ALL                              #
#  create : 00/02/12                                      #
#  update : 18/03/28                                      #
# ------------------------------------------------------- #

OPSYS	:= $(shell uname -o)

# 需要 compile 的目錄
# lib innbbsd maple so util

MAKE	+=	-f Makefile.gnu

all:
	@cd lib; $(MAKE) all
	@cd innbbsd; $(MAKE) all
	@cd maple; $(MAKE) all
ifeq ($(OPSYS),Cygwin)
	@cd so; $(MAKE) all
endif
	@cd util; $(MAKE) all

install:
	@cd innbbsd; $(MAKE) install
	@cd maple; $(MAKE) install
ifeq ($(OPSYS),Cygwin)
	@cd so; $(MAKE) install
endif
	@cd util; $(MAKE) install

clean:
	@cd lib; $(MAKE) clean
	@cd innbbsd; $(MAKE) clean
	@cd maple; $(MAKE) clean
	@cd so; $(MAKE) clean
	@cd util; $(MAKE) clean
