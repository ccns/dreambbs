## Common BSD make rules for DreamBBS Project

## Toolchain settings

CC	= clang

RANLIB	= ranlib

CPROTO	= cproto -E"$(CC) -pipe -E" -I$$(SRCROOT)/include


.ifndef DREAMBBS_MK
DREAMBBS_MK	:= 1

REALSRCROOT	?= $(SRCROOT)

ARCHI	!= getconf LONG_BIT

OPSYS	!= uname -o

BUILDTIME	!= date '+%s'

BBSHOME	?= $(HOME)

## To be expanded

CFLAGS_WARN	= -Wall -Wpointer-arith -Wcast-qual -Wwrite-strings -Wstrict-prototypes
CFLAGS_MK	= -ggdb3 -O0 -pipe -fomit-frame-pointer $(CFLAGS_WARN) -I$$(SRCROOT)/include $(CFLAGS_ARCHI) $(CFLAGS_COMPAT)

LDFLAGS_MK = -L$$(SRCROOT)/lib -ldao -lcrypt $(LDFLAGS_ARCHI)


## Tool functions
## Called with $(function$(para1::=arg1)$(para2::=arg2)...)
UNQUOTE = S/^"//:S/"$$//
VALUEIF = "\#ifdef $(conf)$(.newline)$(conf:M*)$(.newline)\#else$(.newline)$(default:M*)$(.newline)\#endif"
GETCONFS = echo "" | $(CC) -x c -dM -E -P $(hdr:@v@-imacros "$v"@) -
GETVALUE = echo $(VALUEIF$(conf::= $(conf:M*:$(UNQUOTE)))$(default::= $(default:M*))) | $(CC) -x c -E -P $(hdr:@v@-imacros "$v"@) - | xargs

## BBS Release Version Prefix
BBSCONF_ORIGIN		:= $(REALSRCROOT)/include/config.h
BBSVER != $(GETVALUE$(conf::= "BBSVER_SUFFIX")$(default::= "")$(hdr::= $(BBSCONF_ORIGIN)))

# rules ref: PttBBS: mbbsd/Makefile
BBSCONF		:= $(REALSRCROOT)/dreambbs.conf
DEF_LIST	!= sh -c '$(GETCONFS$(hdr::= $(BBSCONF)))'
DEF_TEST	 = [ $(DEF_LIST:M$(conf:M*:S/"//g:N")) ]  # Balance the quotes
DEF_YES		:= && echo "YES" || echo ""
USE_PMORE	!= sh -c '$(DEF_TEST$(conf::= "M3_USE_PMORE")) $(DEF_YES)'
USE_PFTERM	!= sh -c '$(DEF_TEST$(conf::= "M3_USE_PFTERM")) $(DEF_YES)'
USE_BBSLUA	!= sh -c '$(DEF_TEST$(conf::= "M3_USE_BBSLUA")) $(DEF_YES)'
USE_BBSRUBY	!= sh -c '$(DEF_TEST$(conf::= "M3_USE_BBSRUBY")) $(DEF_YES)'
USE_LUAJIT	!= sh -c '$(DEF_TEST$(conf::= "BBSLUA_USE_LUAJIT")) $(DEF_YES)'

CC_HASFLAGS = echo "" | $(CC) -x c -E $(flags:M*) -Werror - >/dev/null 2>&1

.if $(CC:Mclang*)
CFLAGS_WARN	+= -Wno-invalid-source-encoding
.endif

.if $(ARCHI)=="64"
CFLAGS_ARCHI	+= -m32 -D_FILE_OFFSET_BITS=64
LDFLAGS_ARCHI	+= -m32
.endif

.if $(OPSYS) == "GNU/Linux"
LDFLAGS_ARCHI	+= -lresolv -ldl -rdynamic
.endif

.if $(OPSYS) == "FreeBSD"
LDFLAGS_ARCHI	+= -Wl,-export-dynamic
.endif

.if $(OPSYS) == "Cygwin"
NO_SO		 = YES
.else
NO_SO		?=
.endif

.if $(NO_SO)
CFLAGS_MAPLE	+= -DNO_SO
.endif

.if $(CC:M*++) || $(CC:M*++-*)
CFLAGS_COMPAT	+= -x c++
.endif

CC_HAS_W_UNREACHABLE_CODE_AGGRESSIVE != $(CC_HASFLAGS$(flags::= -Wunreachable-code-aggressive)) $(DEF_YES)
.if $(CC_HAS_W_UNREACHABLE_CODE_AGGRESSIVE)
CFLAGS_COMPAT  += -Wunreachable-code-aggressive
.else
CFLAGS_COMPAT  += -Wunreachable-code
.endif

# BBS-Lua & BBS-Ruby make rule definitions

.if $(USE_BBSLUA)
.if $(OPSYS) == "FreeBSD"
    LUA_LDFLAGS_ARCHI	= -Wl,--no-as-needed
.endif

.if $(USE_LUAJIT)
    LUA_PKG_NAME	?= luajit
.else
.if $(OPSYS) == "FreeBSD"
        LUA_PKG_NAME	?= lua-5.1
.else
        LUA_PKG_NAME	?= lua5.1
.endif
.endif

LUA_CFLAGS	!= pkg-config --cflags $(LUA_PKG_NAME)
LUA_LDFLAGS	!= pkg-config --libs $(LUA_PKG_NAME)
.endif


.if $(USE_BBSRUBY)
.if $(OPSYS) == "FreeBSD"
    RUBY_LDFLAGS_ARCHI	= -Wl,--no-as-needed
.endif
.if $(ARCHI) == "64"
    RUBY_CFLAGS_CMD	= | sed 's/x86_64/i386/'
.endif

RUBY_CFLAGS	!= pkg-config --cflags ruby-2.2 $(RUBY_CFLAGS_CMD)
RUBY_LDFLAGS	!= pkg-config --libs ruby-2.2
.endif

.endif  # .ifndef DREAMBBS_MK


## Expand `SRCROOT`
.ifdef SRCROOT
CPROTO	:= $(CPROTO)
CFLAGS	:= $(CFLAGS_MK)
LDFLAGS	:= $(LDFLAGS_MK)
.endif
