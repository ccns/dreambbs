# ------------------------------------------------------ #
#  innbbsd/Makefile     ( NTHU CS MapleBBS Ver 3.10 )    #
# ------------------------------------------------------ #
#  target : Makefile for 轉信程式 innbbsd                #
#  create : 95/03/29                                     #
#  update : 01/08/26                                     #
#  author : skhuang                                      #
# ------------------------------------------------------ #

SRCROOT	= ..
.include "$(SRCROOT)/dreambbs.mk"

INNOBJS	= channel.o bbslib.o convcode.o inntobbs.o rec_article.o nocem.o history.o

LNKOBJS	= bbslink.o bbslib.o convcode.o inntobbs.o rec_article.o nocem.o history.o

EXE	= innbbsd bbslink

all: 
	@$(MAKE) CC="$(CC)" CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)" $(EXE)

bbslink: $(LNKOBJS)
	$(CC) -o bbslink $(LNKOBJS) $(OPT) $(LDFLAGS)

innbbsd: $(INNOBJS)
	$(CC) -o innbbsd $(INNOBJS) $(OPT) $(LDFLAGS)

install: $(EXE)
	install -m 0700 $? $(HOME)/innd

update:
	-csh -c "kill `tail -1 $(HOME)/run/innbbsd.pid | awk '{print $$1}'`";exit 0

clean:
	rm -f $(EXE) *.o *~

