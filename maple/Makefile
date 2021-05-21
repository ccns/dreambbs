# ------------------------------------------------------ #
#  maple/Makefile  ( NTHU CS MapleBBS Ver 3.x )          #
# ------------------------------------------------------ #
#  author : opus.bbs@bbs.cs.nthu.edu.tw                  #
#  target : Makefile for MapleBBS main programs          #
#  create : 95/03/29                                     #
#  update : 18/03/28                                     #
# ------------------------------------------------------ #

SRCROOT	= $(.CURDIR)/..
EXPORT_FILE	= $(.CURDIR)/make_export.conf
.include "$(SRCROOT)/dreambbs.mk"

.if $(NO_SO)
.include "$(SRCROOT)/so/so.mk"
.endif

CFLAGS	+= $(CFLAGS_MAPLE)

.if $(USE_BBSLUA)
CFLAGS	+= $(LUA_CFLAGS)
.endif

.if $(USE_BBSRUBY)
CFLAGS	+= $(RUBY_CFLAGS)
.endif

.if $(USE_PFTERM)
SRC	+= pfterm.c
OBJ	+= pfterm.o
.endif

SRC 	+= acct.c bbsd.c board.c cache.c edit.c \
	  gem.c mail.c menu.c more.c post.c \
	  talk.c visio.c xover.c socket.c popupmenu.c \
	  window.c myfavorite.c

OBJ	+= acct.o bbsd.o board.o cache.o edit.o \
	  gem.o mail.o menu.o more.o post.o \
	  talk.o visio.o xover.o socket.o popupmenu.o \
	  window.o myfavorite.o

.if $(USE_PMORE)
SRC	+= pmore.c
OBJ	+= pmore.o
.endif

.if $(NO_SO)
SO_SRC	= $(SO:R:@v@$(SRCROOT)/so/$v.c@)  # Add path
SO_OBJ	= $(SO:R:@v@$(SRCROOT)/so/$v.o@)

.if $(USE_BBSLUA)
LDFLAGS	+= $(LUA_LDFLAGS) $(LUA_LDFLAGS_ARCHI)
.if !$(USE_LUAJIT)
SO_SRC	+= $(SRCROOT)/so/bbsluaext.c
SO_OBJ	+= $(SRCROOT)/so/bbsluaext.o
.endif
.endif

.if $(USE_BBSRUBY)
LDFLAGS	+= $(RUBY_LDFLAGS) $(RUBY_LDFLAGS_ARCHI)
.endif

SRC	+= $(SO_SRC)
OBJ	+= $(SO_OBJ)

.for target in $(SO_OBJ)
$(target):
	$(CC) $(CFLAGS) -c $(target:R).c -o $(target)
.endfor
.endif

EXE	= bbsd xchatd

.SUFFIXES: .o .c

.c.o:	; $(CC) $(CFLAGS) -c $*.c

all: $(EXE)

xchatd: xchatd.o
	$(CC) -o $@ $? $(LDFLAGS)

bbsd: $(OBJ)
	$(CC) $(MAKEFLAG) -o $@ $(OBJ) $(LDFLAGS) $(LIBS)

maple.p: $(SRC)
	$(CPROTO) -o $@ $?

export:
	@> $(EXPORT_FILE)
	@$("$(NO_SO_CLI:M*YES*)" != "" :? $(EXPORTCONF$(exconf::= NO_SO)$(exvalue::= 1)) :)

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

install: $(EXE)
	$(INSTALL) -d "$(BBSHOME)/bin$(BBSVER)"
	$(INSTALL) -m 0755 $? "$(BBSHOME)/bin$(BBSVER)"
	mv -f "$(BBSHOME)/bin$(BBSVER)/bbsd" "$(BBSHOME)/bin$(BBSVER)/bbsd.$(BUILDTIME)"
	ln -sv "$(BBSHOME)/bin$(BBSVER)/bbsd.$(BUILDTIME)" "$(BBSHOME)/bin$(BBSVER)/bbsd"

clean:
	rm -rf $(OBJ) $(EXE) *~ *.o *.so *.p tags

tags: $(SRC) $(SRCROOT)/include/*.h $(SRCROOT)/lib/*.c
	exctags $(SRC) $(SRCROOT)/include/*.h $(SRCROOT)/lib/*.c

