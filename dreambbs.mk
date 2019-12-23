## Common BSD make rules for DreamBBS Project

## Toolchain settings

CC	= clang

RANLIB	= ranlib

CPROTO	= cproto -E"$(CC) -pipe -E" -I$$(SRCROOT)/include

INSTALL	= install -o $(BBSUSR)


.if "$(DREAMBBS_MK)" == ""
DREAMBBS_MK	:= 1

REALSRCROOT	?= $(SRCROOT)

ARCHI	!= getconf LONG_BIT

OPSYS	!= uname -o

BUILDTIME	!= date '+%s'

## To be expanded

CFLAGS_WARN	= -Wall -Wpointer-arith -Wcast-qual -Wwrite-strings
CFLAGS_MK	= -ggdb3 -O0 -pipe -fomit-frame-pointer $(CFLAGS_WARN) -I$$(SRCROOT)/include $(CFLAGS_ARCHI) $(CFLAGS_COMPAT)

LDFLAGS_MK = -L$$(SRCROOT)/lib -ldao -lcrypt $(LDFLAGS_ARCHI)


## Tool functions
## Called with $(function$(para1::=arg1)$(para2::=arg2)...)

UNQUOTE = S/^"//:S/"$$//

VALUEIF = "\#ifdef $(conf)$(.newline)$(conf:M*)$(.newline)\#else$(.newline)$(default:M*)$(.newline)\#endif"
DEFVAR = "\#undef $(exconf:M*)$(.newline)\#define $(exconf:M*) $($(exconf:M*):M*)"
DEFCONF = "\#undef $(exconf:M*)$(.newline)\#define $(exconf:M*) $(exvalue:M*)"

GETVAR = [ "$(var:M*:$(UNQUOTE))" ] && echo "$(var:M*:$(UNQUOTE))" || $(else_var)
GETCONFS = echo "" | $(CC) -x c -dM -E -P $(hdr:@v@-imacros "$v"@) - 2>/dev/null
GETVALUE = { echo $(VALUEIF$(conf::= $(conf:M*:$(UNQUOTE)))$(default::= $(default:M*))) | $(CC) -x c -E -P $(hdr:@v@-imacros "$v"@) - | xargs; } 2>/dev/null
EXPORTVAR = echo $(DEFVAR$(exconf::= $(exconf:M*))) >> $(EXPORT_FILE)
EXPORTCONF = echo $(DEFCONF$(exconf::= $(exconf:M*))$(exvalue::= $(exvalue:M*))) >> $(EXPORT_FILE)

# Read variables from the configuration C files

BBSCONF		:= $(REALSRCROOT)/dreambbs.conf
BBSCONF_ORIGIN		:= $(REALSRCROOT)/include/config.h
EXPORT_MAPLE	:= $(REALSRCROOT)/maple/make_export.conf
!= touch $(EXPORT_MAPLE)

# User names and group names
BBSUSR != $(GETVAR$(var::= "$(BBSUSR)")$(else_var::= $(GETVALUE$(conf::= "BBSUSR")$(default::= "$(:!id -un!)")$(hdr::= $(BBSCONF_ORIGIN)))))
BBSGROUP != $(GETVAR$(var::= "$(BBSGROUP)")$(else_var::= $(GETVALUE$(conf::= "BBSGROUP")$(default::= "$(:!id -gn!)")$(hdr::= $(BBSCONF_ORIGIN)))))
WWWGROUP != $(GETVAR$(var::= "$(WWWGROUP)")$(else_var::= $(GETVALUE$(conf::= "WWWGROUP")$(default::= "www-data")$(hdr::= $(BBSCONF_ORIGIN)))))

# UIDs and GIDs
ID_DEFAULT = 9999
ID_FALLBACK = 2>/dev/null || echo $(ID_DEFAULT)
BBSUID != $(GETVAR$(var::= "$(BBSUID)")$(else_var::= $(GETVALUE$(conf::= "BBSUID")$(default::= "$(:!id -u $(BBSUSR) $(ID_FALLBACK)!)")$(hdr::= $(BBSCONF)))))
BBSGROUP_GID != getent group $(BBSGROUP) | cut -d: -f3
BBSGROUP_GID != $(GETVAR$(var::= "$(BBSGROUP_GID)")$(else_var::= echo $(ID_DEFAULT)))
BBSGID != $(GETVAR$(var::= "$(BBSGID)")$(else_var::= $(GETVALUE$(conf::= "BBSGID")$(default::= "$(BBSGROUP_GID)")$(hdr::= $(BBSCONF)))))
WWWGROUP_GID != getent group $(WWWGROUP) | cut -d: -f3
WWWGROUP_GID != $(GETVAR$(var::= "$(WWWGROUP_GID)")$(else_var::= echo $(ID_DEFAULT)))
WWWGID != $(GETVAR$(var::= "$(WWWGID)")$(else_var::= $(GETVALUE$(conf::= "WWWGID")$(default::= "$(WWWGROUP_GID)")$(hdr::= $(BBSCONF)))))

## BBS path prefixes and suffixes
BBSVER != $(GETVALUE$(conf::= "BBSVER_SUFFIX")$(default::= "")$(hdr::= $(BBSCONF_ORIGIN)))
BBSUSR_HOME != getent passwd $(BBSUSR) | cut -d: -f6
BBSHOME != $(GETVAR$(var::= "$(BBSHOME)")$(else_var::= $(GETVALUE$(conf::= "BBSHOME")$(default::= "$(BBSUSR_HOME)")$(hdr::= $(BBSCONF)))))

# rules ref: PttBBS: mbbsd/Makefile
DEF_LIST	!= sh -c '$(GETCONFS$(hdr::= $(BBSCONF)))'
DEF_TEST	 = [ $(DEF_LIST:M$(conf:M*:S/"//g:N")) ]  # Balance the quotes
DEF_YES		:= && echo "YES" || echo ""
USE_PMORE	!= sh -c '$(DEF_TEST$(conf::= "M3_USE_PMORE")) $(DEF_YES)'
USE_PFTERM	!= sh -c '$(DEF_TEST$(conf::= "M3_USE_PFTERM")) $(DEF_YES)'
USE_BBSLUA	!= sh -c '$(DEF_TEST$(conf::= "M3_USE_BBSLUA")) $(DEF_YES)'
USE_BBSRUBY	!= sh -c '$(DEF_TEST$(conf::= "M3_USE_BBSRUBY")) $(DEF_YES)'
USE_LUAJIT	!= sh -c '$(DEF_TEST$(conf::= "BBSLUA_USE_LUAJIT")) $(DEF_YES)'

# Flags for disabling shared objects
DEF_LIST	!= sh -c '$(GETCONFS$(hdr::= $(EXPORT_MAPLE)))'
NO_SO_CLI	:= $("$(NO_SO_CLI)" != "" :? $(NO_SO_CLI) : $(NO_SO:DYES:UNO))
NO_SO_CONF	!= sh -c '$(DEF_TEST$(conf::= "NO_SO")) $(DEF_YES)'
NO_SO		= $(NO_SO_CLI:S/NO//g)$(NO_SO_CONF)

CC_HASFLAGS = echo "" | $(CC) -x c -E $(flags:M*) -Werror - >/dev/null 2>&1

.if $(CC:Mclang*)
CFLAGS_WARN	+= -Wno-invalid-source-encoding
.endif

.if $(ARCHI)=="64"
CFLAGS_ARCHI	+= -m32
LDFLAGS_ARCHI	+= -m32

# Set up the search paths for `pkg-config`
MULTIARCH_NATIVE	!= $(CC) -dumpmachine | sed 's/^\(.*\)-\(.*\)-\(.*\)-\(.*\)$$/\1-\3-\4/'
MULTIARCH	= $(MULTIARCH_NATIVE:S/x86_64/i386/g)
TRIPLETS	= $(MULTIARCH_NATIVE:S/x86_64/i486/g) $(MULTIARCH_NATIVE:S/x86_64/i686/g)

PKG_CONFIG_LIBDIR	:= $(/usr/local/lib/$(MULTIARCH)/pkgconfig:L:Q)
PKG_CONFIG_LIBDIR	+= $(TRIPLETS:@v@$(/usr/local/$v/lib/pkgconfig:L:Q)@)
PKG_CONFIG_LIBDIR	+= $(/usr/local/share/pkgconfig:L:Q)
PKG_CONFIG_LIBDIR	:= $(PKG_CONFIG_LIBDIR) $(PKG_CONFIG_LIBDIR:S/local\///g)
PKG_CONFIG_LIBDIR	:= "$(PKG_CONFIG_LIBDIR:ts:)"
.export PKG_CONFIG_LIBDIR
.endif

.if $(OPSYS) == "GNU/Linux"
LDFLAGS_ARCHI	+= -lresolv -ldl -rdynamic
.endif

.if $(OPSYS) == "FreeBSD"
LDFLAGS_ARCHI	+= -Wl,-export-dynamic
.endif

.if $(OPSYS) == "Cygwin"
NO_SO		 = YES
.endif

.if $(NO_SO)
CFLAGS_MAPLE	+= -DNO_SO
.else
CFLAGS_MAPLE	+= -DNO_SO=0
.endif

.if $(CC:M*++) || $(CC:M*++-*)
CFLAGS_COMPAT	+= -x c++
.else
CFLAGS_WARN	+= -Wstrict-prototypes
.endif

CC_HAS_W_UNREACHABLE_CODE_AGGRESSIVE != $(CC_HASFLAGS$(flags::= -Wunreachable-code-aggressive)) $(DEF_YES)
.if $(CC_HAS_W_UNREACHABLE_CODE_AGGRESSIVE)
CFLAGS_COMPAT  += -Wunreachable-code-aggressive
.else
CFLAGS_COMPAT  += -Wunreachable-code
.endif

CC_HAS_F_COLOR_DIAGNOSTICS != $(CC_HASFLAGS$(flags::= -fcolor-diagnostics)) $(DEF_YES)
.if $(CC_HAS_F_COLOR_DIAGNOSTICS)
CFLAGS_COMPAT  += -fcolor-diagnostics
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

RUBY_CFLAGS	!= pkg-config --cflags ruby-2.2
RUBY_LDFLAGS	!= pkg-config --libs ruby-2.2
.endif

.endif  # .ifndef DREAMBBS_MK


## Expand `SRCROOT`
.ifdef SRCROOT
CPROTO	:= $(CPROTO)
CFLAGS	:= $(CFLAGS_MK)
LDFLAGS	:= $(LDFLAGS_MK)
.endif
