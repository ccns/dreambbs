## Common BSD make rules for DreamBBS Project

ARCHI	!= uname -m

OPSYS	!= uname -o

# rules ref: PttBBS: mbbsd/Makefile
BBSCONF		:= $(SRCROOT)/dreambbs.conf
DEF_PATTERN	:= ^[ \t]*\#[ \t]*define[ \t]*
DEF_CMD		:= grep -Ew "${DEF_PATTERN}"
DEF_YES		:= && echo "YES" || echo ""
USE_PMORE	!= sh -c '${DEF_CMD}"M3_USE_PMORE" ${BBSCONF} ${DEF_YES}'
#USE_PFTERM	!= sh -c '${DEF_CMD}"M3_USE_PFTERM" ${BBSCONF} ${DEF_YES}'

CC	= clang 

RANLIB	= ranlib

CPROTO	= cproto -E"clang -pipe -E" -I$(SRCROOT)/include

CFLAGS	= -g -O2 -pipe -fomit-frame-pointer -Wunused -Wno-invalid-source-encoding -I$(SRCROOT)/include

LDFLAGS	= -L../lib -ldao -lcrypt  

.if  $(ARCHI)=="x86_64" || $(ARCHI)=="amd64"
CFLAGS	+= -m32
LDFLAGS	+= -m32
.endif

.if $(OPSYS) == "GNU/Linux"
LDFLAGS	+= -lresolv -ldl -rdynamic
.endif

.if $(OPSYS) == "FreeBSD"
LDFLAGS	+= -Wl,-export-dynamic
.endif

