# ------------------------------------------------------- #
#  test/Makefile    ( NTHU CS MapleBBS Ver 3.x )          #
# ------------------------------------------------------- #
#  target : Makefile for common library test utils        #
#  create :   /  /                                        #
#  update : 19/07/28                                      #
# ------------------------------------------------------- #
#
# TODO: make variable size test utils.
#

SRCROOT	= ..
.include "$(SRCROOT)/dreambbs.mk"

.SUFFIXES: .o .c

.c.o:	; $(CC) $(MAKEFLAG) $(CFLAGS) -g -c $*.c

LIBSTR	= lib_str_decode lib_str_pat lib_str_xor

LIBFILE	= lib_f_mv lib_f_cp lib_f_rm

LIBOTHER= lib_xsort lib_splay testsize

EXE	= $(LIBSTR) $(LIBFILE) $(LIBOTHER)

all: $(EXE)

.for targets in $(EXE)
$(targets): $(targets).o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)
.endfor

runtest: noargtest scripttest

noargtest:
	@(>&2 printf "\n"); for i in $(LIBSTR) $(LIBOTHER); do ./$$i && (>&2 printf "\033[1;32m$$i test done!\033[0m\n") || ( (>&2 printf "\033[1;31m$$i test failed!\n\033[0m" ) && exit 1 ); done

scripttest:
	@(>&2 printf "\n\033[1;33mrunning testing script...\033[0m\n"); sh testscript.sh

clean:
	rm -fr $(EXE) *.o *.bak *.BAK *.log *~
