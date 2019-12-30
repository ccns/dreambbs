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

// Choose the nth argument without macro expansions
#define CPP_SELECT_0_PRIME(_0, ...)  _0
#define CPP_SELECT_1_PRIME(_0, _1, ...)  _1
#define CPP_SELECT_2_PRIME(_0, _1, _2, ...)  _2

// Expand macros in the arguments and then choose the nth
#define CPP_SELECT_0(...)  CPP_SELECT_0_PRIME(__VA_ARGS__)
#define CPP_SELECT_1(...)  CPP_SELECT_1_PRIME(__VA_ARGS__)
#define CPP_SELECT_2(...)  CPP_SELECT_2_PRIME(__VA_ARGS__)

#define CPP_APPEND_CLOSEPAREN(...)  __VA_ARGS__)

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

/* Macros for conditional macro expansion */

// If `conf` without expansion is ON, then expand to the 1st item of `__VA_ARGS__`,
//    else expand to the 2nd item of `__VA_ARGS__`
#define IF_ON_PRIME(conf, ...)  CPP_SELECT_2(CPP_IF_ON_TEST_ ## conf, __VA_ARGS__,,)

// Expand `conf` and then test whether `conf` is ON
#define IF_ON(conf, ...)  IF_ON_PRIME(conf, __VA_ARGS__)

// These values are treated as ON
#define CPP_IF_ON_TEST_  ,
#define CPP_IF_ON_TEST_1  ,
#define CPP_IF_ON_TEST_true  ,


// Test if `conf` is expanded, then select the 0th item of `__VA_ARGS__`,
//    else select the 1st item of `__VA_ARGS__`
#define IF_DEF_PRIME(conf, ...)  CPP_SELECT_2(conf ## _IS_DEF, CPP_SELECT_1, CPP_SELECT_0,)(__VA_ARGS__)

// Try to expand `conf` and then test whether `conf` is expanded
#define IF_DEF(conf, ...)  IF_DEF_PRIME(conf, __VA_ARGS__)

// Usage: `#define <conf>_IS_DEF  IS_DEF_TEST`
// Not expanded => not defined
#define IS_DEF_TEST  ,


/* Macros for config-dependent attributes for user or board */

#define ATTR_CONF_STR(attr_str, conf)  attr_str IF_ON(conf, "", "(系統功\能未開啟)")

/* Macros for standard-dependent constructs */

#if __cplusplus >= 201103L  /* C++11 */
  #define CXX_CONSTEXPR_STRICT  constexpr
#else
  #define CXX_CONSTEXPR_STRICT  /* Empty */
#endif
#if __cplusplus >= 201402L  /* C++14 */
  #define CXX_CONSTEXPR_RELAXED  constexpr
#else
  #define CXX_CONSTEXPR_RELAXED  /* Empty */
#endif
#if __cplusplus >= 201907L
  #define CXX_CONSTEXPR_TRY_ASM  constexpr
#else
  #define CXX_CONSTEXPR_TRY_ASM  /* Empty */
#endif

#ifdef __cplusplus
template <class T>
    using Identity_T = T;
  /* List literal (C: compound literal / C++: list-initialization of unnamed temporary) */
  #define LISTLIT(Type)  Identity_T<Type>

  /* Temporary lvalue */
  /* Usage: `TEMPLVAL(Type, {init_values...})` (preferred) */
  /*        `TEMPLVAL(Type, (init_value))` */
  #define TEMPLVAL(Type, ...)  \
    ((void)0, const_cast<Identity_T<Type> &>((const Identity_T<Type> &)LISTLIT(Type) __VA_ARGS__))
#else
  #define LISTLIT(Type)  (Type)
  #define TEMPLVAL(Type, ...)  LISTLIT(Type) __VA_ARGS__
#endif

/* Temporary lvalue whose value is not important */
#define SINKVAL(Type)  TEMPLVAL(Type, {0})

#if __cplusplus >= 201103L  /* C++11 */
  #define CPP_TYPEOF(...) decltype(__VA_ARGS__)
#elif defined __GNUC__
  #define CPP_TYPEOF(...) __typeof__(__VA_ARGS__)
#endif

/* Block in expressions */
#if defined __cplusplus
  #define EXPR_BLOCK_BEGIN  [&](void) {
  #define EXPR_BLOCK_END  }()
  #define EXPR_BLOCK_RETURN  return
#elif defined __GNUC__
  #define EXPR_BLOCK_BEGIN  __extension__ ({
  #define EXPR_BLOCK_END  })
  #define EXPR_BLOCK_RETURN  /* Empty */
#endif

/* Macros for manipulating structs */

#include <stddef.h>

#define FLEX_SIZE       /* For declaration of flexible array member */

#define SIZEOF_FLEX(Type, n) \
    (offsetof(Type, Type##_FLEX_MEMBER) \
      + (n) * sizeof(((Type *)NULL)->Type##_FLEX_MEMBER[0]))

/* #define countof(x)      (sizeof(x)/sizeof(x[0])) */

/* Macros for managing loading of dynamic libraries */

#if NO_SO

#define DL_NAME(module_str, obj)   (&obj)
#define DL_GET(dl_name)   (&dl_name)
#define DL_CALL(dl_name)  dl_name

#define DL_NAME_GET(module_str, obj)  (&obj)
#define DL_NAME_CALL(module_str, func)  func

#else  // #if NO_SO

#define DL_NAME(module_str, obj) \
    BINARY_SUFFIX module_str ":" CPP_STR(CPP_UNPAREN_OPT(obj))
#define DL_GET(dl_name)  DL_get(dl_name)
#define DL_CALL(dl_name)  DL_func((dl_name), CPP_APPEND_CLOSEPAREN

#ifdef CPP_TYPEOF
  #define DL_NAME_GET(module_str, obj)  \
      ((CPP_TYPEOF(obj) *) DL_GET(DL_NAME(module_str, obj)))

  #define DL_NAME_CALL(module_str, func)  EXPR_BLOCK_BEGIN \
    CPP_TYPEOF(obj) *pfunc = DL_NAME_GET(module_str, func); \
    EXPR_BLOCK_RETURN (pfunc) ? pfunc DL_APPEND_ELSE_VAL
  #define DL_APPEND_ELSE_VAL(...)  (__VA_ARGS__) : -1; EXPR_BLOCK_END
#else
  #define DL_NAME_GET(module_str, obj)  DL_GET(DL_NAME(module_str, obj))
  #define DL_NAME_CALL(module_str, func)   DL_CALL(DL_NAME(module_str, va ## func))
#endif

#endif  // #if NO_SO

/* Macros for emitting warnings */

#define CPP_PRAGMA(arg)  _Pragma(CPP_STR(arg))

#define CPP_MACRO_DEPRECATED(msg) \
    CPP_PRAGMA(GCC warning CPP_STR(deprecated macro: msg))

/* Macros for limiting the value range */

#ifdef __cplusplus
  #include <algorithm>
  #include <cmath>

  #define BMIN(a, b)      std::min(a, b)
  #define BMAX(a, b)      std::max(a, b)
  #define UABS(a)         std::abs(a)
  #if __cplusplus >= 201703L  /* C++17 */
    #define TCLAMP(x, low, high)  std::clamp(x, low, high)
  #else
    #define TCLAMP(x, low, high)  std::min(std::max(x, low), high)
  #endif
#else
  #define CPP_MIN(a, b)   (((a) < (b)) ? (a) : (b))
  #define CPP_MAX(a, b)   (((a) > (b)) ? (a) : (b))
  #define CPP_ABS(a)      (((a) < 0) ? -(a) : (a))
  #define CPP_CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))
  #ifdef __GNUC__
    #define BMIN(a, b)  __extension__ \
        ({ __typeof__(a) _a = (a); __typeof__(b) _b = (b); CPP_MIN(_a, _b); })
    #define BMAX(a, b)  __extension__ \
        ({ __typeof__(a) _a = (a); __typeof__(b) _b = (b); CPP_MAX(_a, _b); })
    #define UABS(a)  __extension__ ({ __typeof__(a) _a = (a); CPP_ABS(_a); })
    #define TCLAMP(x, low, high)  __extension__ \
        ({ __typeof__(x) _x = (x); \
          __typeof__(low) _low = (low); __typeof__(high) _high = (high); \
          CPP_CLAMP(_x, _low, _high); })
  #else
/* `long double` can hold the value of all the standard scale types */
static inline long double ld_min(long double a, long double b) { return CPP_MIN(a, b); }
static inline long double ld_max(long double a, long double b) { return CPP_MAX(a, b); }
static inline long double ld_abs(long double a) { return CPP_ABS(a); }
static inline long double ld_clamp(long double x, long double low, long double high)
    { return CPP_CLAMP(x, low, high); }
    #define BMIN(a, b)    ld_min(a, b)
    #define BMAX(a, b)    ld_max(a, b)
    #define UABS(a)       ld_abs(a)
    #define TCLAMP(x, low, high)  ld_clamp(x, low, high)
  #endif
#endif

/* Macros for booleans */

/* Standard boolean type and values */
#include <stdbool.h>

/* Booleans  (Yep, for true and false) (Deprecated) */
#define YEA     CPP_MACRO_DEPRECATED(use 'true' instead)  (1)
#define NA      CPP_MACRO_DEPRECATED(use 'false' instead) (0)

#endif  // #ifndef CPPDEF_H
