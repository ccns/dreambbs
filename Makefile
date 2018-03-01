# ------------------------------------------------------- #
#  src/Makefile	( NTHU CS MapleBBS Ver 3.10 )	          #
# ------------------------------------------------------- #
#  target : Makefile for ALL				  #
#  create : 00/02/12                                      #
#  update :   /  /                                        #
# ------------------------------------------------------- #


# 支援的 OS-type
# linux freebsd

# 需要 compile 的目錄
# lib innbbsd maple so util


all:
	@echo "Please enter 'make sys-type', "
	@echo " make linux   : for Linux"
	@echo " make freebsd : for BSD 4.4 systems"

linux:
	@cd lib; make
	@cd innbbsd; make linux
	@cd maple; make linux
	@cd so; make linux
	@cd util; make linux

freebsd:
	@cd lib; make
	@cd innbbsd; make freebsd
	@cd maple; make freebsd
	@cd so; make freebsd
	@cd util; make freebsd

install:
	@cd innbbsd; make install
	@cd maple; make install
	@cd so; make install
	@cd util; make install

clean:
	@cd lib; make clean
	@cd innbbsd; make clean
	@cd maple; make clean
	@cd so; make clean
	@cd util; make clean
