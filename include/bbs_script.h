#ifndef BBS_SCRIPT_H
#define BBS_SCRIPT_H

/* Utility macros */

#define CPP_EVAL(x)  x

// Remove a pair of parentheses: CPP_UNPAREN((STH)) => CPP_EVAL (STH) => STH
#define CPP_UNPAREN(x)  CPP_EVAL x

#define CPP_DUP(x)  CPP_DUP x
#define NIL_CPP_DUP  /* Empty */

// Concate the arguments without macro expansions
#define CPP_CAT_PRIME(x, y)  x##y

// Expand macros in the arguments and then concate them
#define CPP_CAT(x, y)  CPP_CAT_PRIME(x, y)

// Remove a pair of parentheses, or noop if nothing to remove:
//    CPP_UNPAREN_OPT((STH)) => CPP_CAT(NIL_, CPP_DUP (STH) => CPP_CAT(NIL_, CPP_DUP STH) => NIL_CPP_DUP STH => STH
//    CPP_UNPAREN_OPT(STH) => CPP_CAT(NIL_, CPP_DUP STH) => NIL_CPP_DUP STH => STH
#define CPP_UNPAREN_OPT(x)  CPP_CAT(NIL_, CPP_DUP x)

// Stringfy the argument without macro expansions
#define CPP_STR_PRIME(x)  #x

// Expand macros in the argument and then stringfy it
#define CPP_STR(x)  CPP_STR_PRIME(x)


/* Macros for acquiring version strings from number literals */

#define DB_PATCH_STR  "DBpatch"
#define DB_PATCH_SEP  "-" DB_PATCH_STR "-"

// major . minor
#define VER_STR(major, minor)  CPP_STR(CPP_UNPAREN_OPT(major)) "." CPP_STR(CPP_UNPAREN_OPT(minor))

// major . minor . patch
#define VER_PATCH_STR(major, minor, patch) \
    VER_STR(major, minor) "." CPP_STR(CPP_UNPAREN_OPT(patch))

// major . minor -DBpatch- db_patch
#define VER_DB_STR(major, minor, db_patch) \
    VER_STR(major, minor) DB_PATCH_SEP CPP_STR(CPP_UNPAREN_OPT(db_patch))

// major . minor . patch -DBpatch- db_patch
#define VER_PATCH_DB_STR(major, minor, patch, db_patch) \
    VER_PATCH_STR(major, minor, patch) DB_PATCH_SEP CPP_STR(CPP_UNPAREN_OPT(db_patch))


/* BBS-Lua */

#ifdef M3_USE_BBSLUA
  #include <lua.h>
#endif

#define HAVE_BBSLUA

// Must be consistent with "bbslua.c"

#define BBSLUA_INTERFACE_VER    0.119 // (0.201)
#define BBSLUA_INTERFACE_VER_STR  CPP_STR(BBSLUA_INTERFACE_VER)


/* BBS-Ruby */

#ifdef M3_USE_BBSRUBY
  #include <ruby/version.h>

  #define RUBY_RELEASE_STR \
      VER_PATCH_STR(RUBY_API_VERSION_MAJOR, RUBY_API_VERSION_MINOR, RUBY_API_VERSION_TEENY)
#endif

#define HAVE_BBSRUBY

// Must be consistent with "bbsruby.c"

#define BBSRUBY_MAJOR_VERSION   (0)
#define BBSRUBY_MINOR_VERSION   (3)
#define BBSRUBY_DB_PATCH_VERSION  1
#define BBSRUBY_VERSION_STR \
    VER_DB_STR(BBSRUBY_MAJOR_VERSION, BBSRUBY_MINOR_VERSION, BBSRUBY_DB_PATCH_VERSION)

#define BBSRUBY_INTERFACE_VER   0.111
#define BBSRUBY_INTERFACE_VER_STR  CPP_STR(BBSRUBY_INTERFACE_VER)

#endif  // #ifndef BBS_SCRIPT_H
