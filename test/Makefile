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

EXE	= lib_str_decode lib_str_pat lib_str_xor lib_xsort lib_splay testsize

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

testsize: testsize.o
	$(CC) $(MAKEFLAG) -o $@ $? $(LDFLAGS)

test:
	@printf "\n"; for i in $(EXE); do ./$$i && printf "\033[1;32m$$i test done!\033[0m\n" || ( printf "\033[1;31m$$i test failed!\n\033[0m" && exit 1 ); done

sizetest:
	@printf "\n"; ./testsize && printf "\033[1;32msize test done!\033[0m\n" || ( printf "\033[1;31msize test failed!\n\033[0m" && exit 1 )

clean:
	rm -fr $(EXE) *.o *.bak *.BAK *.log *~
