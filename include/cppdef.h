/*-------------------------------------------------------*/
/* cppdef.h     ( NCKU CCNS WindTop-DreamBBS 2.0 )       */
/*-------------------------------------------------------*/
/* Author: 37586669+IepIweidieng@users.noreply.github.com*/
/* Target: C preprocessor utility macros                 */
/* Create: 2019/03/30                                    */
/*-------------------------------------------------------*/

#ifndef CPPDEF_H
#define CPPDEF_H

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
//    CPP_UNPAREN_OPT((STH)) => CPP_CAT(NIL_, CPP_DUP (STH)) => CPP_CAT(NIL_, CPP_DUP STH) => NIL_CPP_DUP STH => STH
//    CPP_UNPAREN_OPT(STH) => CPP_CAT(NIL_, CPP_DUP STH) => NIL_CPP_DUP STH => STH
#define CPP_UNPAREN_OPT(x)  CPP_CAT(NIL_, CPP_DUP x)

// Stringfy the argument without macro expansions
#define CPP_STR_PRIME(x)  #x

// Expand macros in the argument and then stringify it
#define CPP_STR(x)  CPP_STR_PRIME(x)


/* Macros for acquiring version strings from number literals */

#define DL_PATCH_STR  "DlPatch"
#define DL_PATCH_SEP  "-" DL_PATCH_STR "-"

// major . minor
#define VER_STR(major, minor)  CPP_STR(CPP_UNPAREN_OPT(major)) "." CPP_STR(CPP_UNPAREN_OPT(minor))

// major . minor . patch
#define VER_PATCH_STR(major, minor, patch) \
    VER_STR(major, minor) "." CPP_STR(CPP_UNPAREN_OPT(patch))

// ver -DlPatch- dl_patch
// `ver` is a string
#define VERSION_STR_DL_PATCH_STR(version_str, dl_patch) \
    version_str DL_PATCH_SEP CPP_STR(CPP_UNPAREN_OPT(dl_patch))

// `ver` is not a string
#define VERSION_DL_PATCH_STR(version, dl_patch) \
    VERSION_STR_DL_PATCH_STR(CPP_STR(CPP_UNPAREN_OPT(version)), dl_patch)

// major . minor -DlPatch- dl_patch
#define VER_DL_STR(major, minor, dl_patch) \
    VERSION_STR_DL_PATCH_STR(VER_STR(major, minor), dl_patch)

// major . minor . patch -DlPatch- dl_patch
#define VER_PATCH_DL_STR(major, minor, patch, dl_patch) \
    VERSION_STR_DL_PATCH_STR(VER_PATCH_STR(major, minor, patch), dl_patch)


/* Macros for manipulating structs with flexible array member */

#include <stddef.h>

#if __STDC_VERSION__ >= 199901L
  #define FLEX_SIZE     /* For declaration of flexible array member */
#else
  #define FLEX_SIZE     0
#endif

#define SIZEOF_FLEX(Type, n) \
    (offsetof(Type, Type##_FLEX_MEMBER) \
      + (n) * sizeof(((Type *)NULL)->Type##_FLEX_MEMBER[0]))

/* Macros for managing loading of dynamic libraries */

#ifdef NO_SO

#define DL_NAME(module_str, func)   func
#define DL_GET(dl_name)   dl_name
#define DL_CALL(dl_name)  dl_name

#else  // #ifdef NO_SO

#define DL_NAME(module_str, func) \
    BINARY_SUFFIX module_str ":" CPP_STR(CPP_UNPAREN_OPT(func))

#define DL_GET(dl_name)  DL_get(dl_name)

#if __STDC_VERSION__ >= 199901L
  #define CPP_APPEND_CLOSEPAREN(...)  __VA_ARGS__)
#else
  #define CPP_APPEND_CLOSEPAREN(args...)  args)
#endif

#define DL_CALL(dl_name)  DL_func((dl_name), CPP_APPEND_CLOSEPAREN

#endif  // #ifdef NO_SO

#endif  // #ifndef CPPDEF_H
