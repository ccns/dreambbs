# ------------------------------------------------------- #
#  test/Makefile    ( NTHU CS MapleBBS Ver 3.x )          #
# ------------------------------------------------------- #
#  target : Makefile for common library test utils        #
#  create :   /  /                                        #
#  update : 18/11/20                                      #
# ------------------------------------------------------- #
#
# TODO: make variable size test utils.
#

SRCROOT	= ..
.include "$(SRCROOT)/dreambbs.mk"

.SUFFIXES: .o .c

.c.o:	; $(CC) $(MAKEFLAG) $(CFLAGS) -g -c $*.c

LIBSTR	= lib_str_decode lib_str_pat lib_str_xor

LIBFILE	= lib_f_mv lib_f_cp

LIBOTHER= lib_xsort lib_splay testsize

EXE	= $(LIBSTR) $(LIBFILE) $(LIBOTHER)

all: $(EXE)

lib_str_decode: lib_str_decode.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

lib_str_pat: lib_str_pat.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

lib_str_xor: lib_str_xor.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

lib_xsort: lib_xsort.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

lib_splay: lib_splay.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

lib_f_mv: lib_f_mv.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

lib_f_cp: lib_f_cp.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

testsize: testsize.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

runtest: noargtest scripttest sizetest

noargtest:
	@(>&2 printf "\n"); for i in $(LIBSTR) $(LIBOTHER); do ./$$i && (>&2 printf "\033[1;32m$$i test done!\033[0m\n") || ( (>&2 printf "\033[1;31m$$i test failed!\n\033[0m" ) && exit 1 ); done

scripttest:
	@(>&2 printf "\n\033[1;33mrunning testing script...\033[0m\n"); sh testscript.sh

sizetest:
	@(>&2 printf "\n"); ./testsize && (>&2 printf "\033[1;32msize test done!\033[0m\n") || ( (>&2 printf "\033[1;31msize test failed!\n\033[0m" ) && exit 1 )

clean:
	rm -fr $(EXE) *.o *.bak *.BAK *.log *~
