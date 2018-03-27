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
	@cd lib; make
	@cd innbbsd; make all
	@cd maple; make all
	@cd so; make all
	@cd util; make all

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
