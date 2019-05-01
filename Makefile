# ------------------------------------------------------- #
#  Makefile  ( NTHU CS MapleBBS Ver 3.x )                 #
# ------------------------------------------------------- #
#  target : Makefile for ALL                              #
#  create : 00/02/12                                      #
#  update : 18/03/28                                      #
# ------------------------------------------------------- #

OPSYS	!= uname -o
NPROC	!= nproc

REALSRCROOT	:= .
.include "$(REALSRCROOT)/dreambbs.mk"
.export

# some directories need to be compiled:
# lib innbbsd maple so util test

all:
	@cd lib; $(MAKE) all
	@cd maple; $(MAKE) all
	@cd util; $(MAKE) all
	@cd innbbsd; $(MAKE) all
.if $(OPSYS) != "Cygwin"
	@cd so; $(MAKE) all
.endif
	@cd test; $(MAKE) all

njob:
	@cd lib; $(MAKE) -j$(NPROC) all
	@cd maple; $(MAKE) -j$(NPROC) all
	@cd util; $(MAKE) -j$(NPROC) all
	@cd innbbsd; $(MAKE) -j$(NPROC) all
.if $(OPSYS) != "Cygwin"
	@cd so; $(MAKE) -j$(NPROC) all
.endif
	@cd test; $(MAKE) -j$(NPROC) all

runtest:
	@cd test; $(MAKE) runtest

install:
	@cd test; $(MAKE) runtest
	@cd maple; $(MAKE) install
	@cd innbbsd; $(MAKE) install
	@cd util; $(MAKE) install
.if $(OPSYS) != "Cygwin"
	@cd so; $(MAKE) install
.endif
	@cd scripts; $(MAKE) install

clean:
	@cd lib; $(MAKE) clean
	@cd test; $(MAKE) clean
	@cd maple; $(MAKE) clean
	@cd util; $(MAKE) clean
	@cd innbbsd; $(MAKE) clean
	@cd so; $(MAKE) clean
