/*-------------------------------------------------------*/
/* attrdef.h    ( NCKU CCNS WindTop-DreamBBS 2.0 )       */
/*-------------------------------------------------------*/
/* Author: 37586669+IepIweidieng@users.noreply.github.com*/
/* Target: Implementation-defined attribute macros       */
/* Create: 2019/09/25                                    */
/*-------------------------------------------------------*/

#ifndef ATTRDEF_H
#define ATTRDEF_H

/* Attributes about the arguments of a function */

/* For checking the arguments for variadic functions */

#ifndef GCC_CHECK_FORMAT
  #ifdef __GNUC__
    #define GCC_CHECK_FORMAT(ifmt, iarg)  __attribute__((format(printf, ifmt, iarg)))
  #else
    #define GCC_CHECK_FORMAT(ifmt, iarg)  /* Ignored */
  #endif
#endif

/* Checks whether the `ri_nul`-th-to-last argument (`0`-th for the last) is NULL */
#ifndef GCC_CHECK_SENTINEL
  #if defined __GNUC__
    #define GCC_CHECK_SENTINEL(ri_nul)  __attribute__((sentinel(ri_nul)))
  #else
    #define GCC_CHECK_SENTINEL(ri_nul)  /* Ignored */
  #endif
#endif

/* For functions with pointer parameters */

/* Checks whether the arguments (`1`-st for the first) is not NULL */
#if !defined GCC_CHECK_NONNULL_ALL || !defined GCC_CHECK_NONNULL
  #undef GCC_CHECK_NONNULL_ALL
  #undef GCC_CHECK_NONNULL
  #if defined __GNUC__
    #define GCC_CHECK_NONNULL_ALL   __attribute__((nonnull))
    #define GCC_CHECK_NONNULL(...)  __attribute__((nonnull(__VA_ARGS__)))
  #else
    #define GCC_CHECK_NONNULL_ALL   /* Ignored */
    #define GCC_CHECK_NONNULL(...)  /* Ignored */
  #endif
#endif

/* Attributes about the return value of a function */

#ifndef GCC_NORETURN
  #if __STDC_VERSION__ >= 201112L  /* C11 */
    #define GCC_NORETURN  _Noreturn
  #elif __cplusplus >= 201103L  /* C++11 */
    #define GCC_NORETURN  [[noreturn]]
  #elif defined __GNUC__
    #define GCC_NORETURN  __attribute__((__noreturn__))
  #else
    #define GCC_NORETURN  /* Ignored */
  #endif
#endif

#ifndef GCC_NODISCARD
  #if __cplusplus >= 201703L  /* C++17 */
    #define GCC_NODISCARD  [[nodiscard]]
  #elif defined __GNUC__
    #define GCC_NODISCARD  __attribute__((warn_unused_result))
  #else
    #define GCC_NODISCARD  /* Ignored */
  #endif
#endif

/* Denotes whether the arguments is not NULL (`1` for the first) */
#ifndef GCC_MALLOC
  #if defined __GNUC__
    #define GCC_MALLOC  __attribute__((malloc))
  #else
    #define GCC_MALLOC  /* Ignored */
  #endif
#endif

#ifndef GCC_RETURNS_NONNULL
  #if defined __GNUC__
    #define GCC_RETURNS_NONNULL  __attribute__((returns_nonnull))
  #else
    #define GCC_RETURNS_NONNULL  /* Ignored */
  #endif
#endif

/* Attributes about the side-effects of a function */

/* Denotes that the function does not write to any non-local objects or static local objects */
#ifndef GCC_PURE
  #if defined __GNUC__
    #define GCC_PURE  __attribute__((__pure__))
  #else
    #define GCC_PURE  /* Ignored */
  #endif
#endif

/* Denotes that the function does not read or write to any non-local objects or static local objects */
#ifndef GCC_CONSTEXPR
  #if defined __GNUC__
    #define GCC_CONSTEXPR  __attribute__((__const__))
  #else
    #define GCC_CONSTEXPR  /* Ignored */
  #endif
#endif

/* General/other attributes */

#ifndef GCC_UNUSED
  #if defined __GNUC__
    #define GCC_UNUSED    __attribute__((__unused__))
  #else
    #define GCC_UNUSED    /* Ignored */
  #endif
#endif

#ifndef GCC_DEPRECATED
  #if __cplusplus >= 201402L  /* C++14 */
    #define GCC_DEPRECATED(msg_str)  [[deprecated(msg_str)]]
  #elif defined __GNUC__
    #define GCC_DEPRECATED(msg_str)  __attribute__((__deprecated__(msg_str)))
  #else
    #define GCC_DEPRECATED(msg_str)  /* Ignored */
  #endif
#endif

#ifndef GCC_FALLTHROUGH
  #if __cplusplus >= 201703L  /* C++17 */
    #define GCC_FALLTHROUGH  [[fallthrough]]
  #elif defined __GNUC__
    #define GCC_FALLTHROUGH  __attribute__((__fallthrough__))
  #else
    #define GCC_FALLTHROUGH  /* Ignored */
  #endif
#endif

#endif                          /* ATTRDEF_H */
