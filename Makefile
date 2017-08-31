# ------------------------------------------------------- #
#  src/Makefile	( NTHU CS MapleBBS Ver 3.10 )	          #
# ------------------------------------------------------- #
#  target : Makefile for ALL				  #
#  create : 00/02/12                                      #
#  update :   /  /                                        #
# ------------------------------------------------------- #


# 支援的 OS-type
# sun linux solaris sol-x86 freebsd bsd cygwin

# 需要 compile 的目錄
# lib bgopherd bmtad bpop3d innbbsd maple so util


all:
	@echo "Please enter 'make sys-type', "
	@echo " make sun     : for Sun-OS 4.x and maybe some BSD systems, cc or gcc"
	@echo " make linux   : for Linux"
	@echo " make solaris : for Sun-OS 5.x gcc"
	@echo " make sol-x86 : for Solaris 7 x86"
	@echo " make freebsd : for BSD 4.4 systems"
	@echo " make bsd     : for BSD systems, cc or gcc, if not in the above lists"
	@echo " make cygwin  : for Microsoft Windows and Cygwin gcc"


sun:
	@cd lib; make
	@cd bmtad; make sun
	@cd bpop3d; make sun
	@cd innbbsd; make sun
	@cd maple; make sun
	@cd so; make sun
	@cd util; make sun

linux:
	@cd lib; make
	@cd bmtad; make linux
	@cd bpop3d; make linux
	@cd innbbsd; make linux
	@cd maple; make linux
	@cd so; make linux
	@cd util; make linux

solaris:
	@cd lib; make
	@cd bmtad; make solaris
	@cd bpop3d; make solaris
	@cd innbbsd; make solaris
	@cd maple; make solaris
	@cd so; make solaris
	@cd util; make solaris

sol-x86:
	@cd lib; make
	@cd bmtad; make sol-x86
	@cd bpop3d; make sol-x86
	@cd innbbsd; make sol-x86
	@cd maple; make sol-x86
	@cd so; make sol-x86
	@cd util; make sol-x86

freebsd:
	@cd lib; make
	@cd bmtad; make freebsd
	@cd bpop3d; make freebsd
	@cd innbbsd; make freebsd
	@cd maple; make freebsd
	@cd so; make freebsd
	@cd util; make freebsd

bsd:
	@cd lib; make
	@cd bmtad; make bsd
	@cd bpop3d; make bsd
	@cd innbbsd; make bsd
	@cd maple; make bsd
	@cd so; make bsd
	@cd util; make bsd

install:
	@cd bmtad; make install
	@cd bpop3d; make install
	@cd innbbsd; make install
	@cd maple; make install
	@cd so; make install
	@cd util; make install

clean:
	@cd lib; make clean
	@cd bmtad; make clean
	@cd bpop3d; make clean
	@cd innbbsd; make clean
	@cd maple; make clean
	@cd so; make clean
	@cd util; make clean
