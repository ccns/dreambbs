# ------------------------------------------------------ #
#  innbbsd/Makefile     ( NTHU CS MapleBBS Ver 3.10 )    #
# ------------------------------------------------------ #
#  target : Makefile for 轉信程式 innbbsd                #
#  create : 95/03/29                                     #
#  update : 01/08/26                                     #
#  author : skhuang                                      #
# ------------------------------------------------------ #

SRCROOT	= $(.CURDIR)/..
.include "$(SRCROOT)/dreambbs.mk"

INNOBJS	= channel.o bbslib.o convcode.o inntobbs.o rec_article.o nocem.o history.o

LNKOBJS	= bbslink.o bbslib.o convcode.o inntobbs.o rec_article.o nocem.o history.o

EXE	= innbbsd bbslink

all: $(EXE)

bbslink: $(LNKOBJS)
	$(CC) -o bbslink $(LNKOBJS) $(OPT) $(LDFLAGS)

innbbsd: $(INNOBJS)
	$(CC) -o innbbsd $(INNOBJS) $(OPT) $(LDFLAGS)

install: $(EXE)
	install -d "$(BBSHOME)/innd"
	install -m 0700 $? $(BBSHOME)/innd

clean:
	rm -f $(EXE) *.o *~

