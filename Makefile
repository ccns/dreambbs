# ------------------------------------------------------- #
#  Makefile  ( NTHU CS MapleBBS Ver 3.x )                 #
# ------------------------------------------------------- #
#  target : Makefile for ALL                              #
#  create : 00/02/12                                      #
#  update : 18/03/28                                      #
# ------------------------------------------------------- #

OPSYS	!= uname -o
NPROC	!= getconf _NPROCESSORS_ONLN

REALSRCROOT	:= $(.CURDIR)
EXPORT_FILE	:= "$(REALSRCROOT)/make_export.conf"

!= touch $(EXPORT_FILE)  # Needed by `include/config.h`
.include "$(REALSRCROOT)/dreambbs.mk"
.export
!= [ -s $(EXPORT_FILE) ] || rm $(EXPORT_FILE)  # Empty

# some directories need to be compiled:
# lib innbbsd maple so util test

all:
	@(cd lib; $(MAKE) all)
	@(cd maple; $(MAKE) all)
	@(cd util; $(MAKE) all)
	@(cd innbbsd; $(MAKE) all)
.if !$(NO_SO)
	@(cd so; $(MAKE) all)
.endif
	@(cd test; $(MAKE) all)

njob:
	@(cd lib; $(MAKE) -j$(NPROC) all)
	@(cd maple; $(MAKE) -j$(NPROC) all)
	@(cd util; $(MAKE) -j$(NPROC) all)
	@(cd innbbsd; $(MAKE) -j$(NPROC) all)
.if !$(NO_SO)
	@(cd so; $(MAKE) -j$(NPROC) all)
.endif
	@(cd test; $(MAKE) -j$(NPROC) all)

export:
	@> $(EXPORT_FILE)
	@$(EXPORTVAR$(exconf::= BBSUID))
	@$(EXPORTVAR$(exconf::= BBSGID))
	@$(EXPORTVAR$(exconf::= WWWGID))
	@$(EXPORTCONF$(exconf::= BBSHOME)$(exvalue::= \"$(BBSHOME)\"))
	@$("$(ARCHI)" == "64" :? $(EXPORTCONF$(exconf::= _FILE_OFFSET_BITS)$(exvalue::= 64)) :)
	@$("$(NO_SO)" != "" :? $(EXPORTCONF$(exconf::= NO_SO)$(exvalue::= 1)) :)

configure:
	@printf "\033[1;36mGenerating '$(EXPORT_FILE)'...\033[0m\n" >&2
	@$(DREAMBBS_MK::=)
	@sh -c "$(MAKE) export $(MAKEFLAGS)"
	@printf "\033[1;33m"
	@cat $(EXPORT_FILE)
	@printf "\033[m"
	@$(TARGETS_REST::=$(.TARGETS:tW:C/^ *configure *//))
	# Continue execution with a new `bmake` instace and stop current `bmake` instance
	@if [ "$(TARGETS_REST)" ]; then sh -c "$(MAKE) $(TARGETS_REST) $(MAKEFLAGS)"; printf "\033[1;36mJob done. Force stop.\033[m\n" >&2; false; fi

runtest:
	@(cd test; $(MAKE) runtest)

install:
	@(cd test; $(MAKE) runtest)
	@(cd maple; $(MAKE) install)
	@(cd innbbsd; $(MAKE) install)
	@(cd util; $(MAKE) install)
.if !$(NO_SO)
	@(cd so; $(MAKE) install)
.endif
	@(cd scripts; $(MAKE) install)

clean:
	@(cd lib; $(MAKE) clean)
	@(cd test; $(MAKE) clean)
	@(cd maple; $(MAKE) clean)
	@(cd util; $(MAKE) clean)
	@(cd innbbsd; $(MAKE) clean)
	@(cd so; $(MAKE) clean)
