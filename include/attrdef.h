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

/* Indicates that the `ifmt`-th argument (`1` for the first) is a format string which should follow the `printf` syntax,
 * and that the `iarg`-th and the following arguments are the format arguments which should be consistent with the format string */
#ifndef GCC_CHECK_FORMAT
  #ifdef __GNUC__
    #define GCC_CHECK_FORMAT(ifmt, iarg)  __attribute__((format(printf, ifmt, iarg)))
  #else
    #define GCC_CHECK_FORMAT(ifmt, iarg)  /* Ignored */
  #endif
#endif
#define GCC_FORMAT GCC_CHECK_FORMAT

/* Indicates that the `ri_nul`-th-to-last argument (`0` for the last) should be NULL */
#ifndef GCC_CHECK_SENTINEL
  #if defined __GNUC__
    #define GCC_CHECK_SENTINEL(ri_nul)  __attribute__((sentinel(ri_nul)))
  #else
    #define GCC_CHECK_SENTINEL(ri_nul)  /* Ignored */
  #endif
#endif
#define GCC_SENTINEL GCC_CHECK_SENTINEL

/* For functions with pointer parameters */

/* Indicates that the specified arguments (`1` for the first) should not be NULL */
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
#define GCC_NONNULL_ALL GCC_CHECK_NONNULL_ALL
#define GCC_NONNULLS    GCC_CHECK_NONNULL_ALL
#define GCC_NONNULL     GCC_CHECK_NONNULL

/* Attributes about the return value of a function */

/* Indicates that the function does not return normally */
#ifndef GCC_NORETURN
  #if __STDC_VERSION__ >= 201112L  /* C11 */
    #define GCC_NORETURN  _Noreturn
  #elif __cplusplus >= 201103L  /* C++11 */
    #define GCC_NORETURN  [[noreturn]]
  #elif defined __GNUC__
    #define GCC_NORETURN  __attribute__((noreturn))
  #else
    #define GCC_NORETURN  /* Ignored */
  #endif
#endif

/* Indicates that the return value should not be ignored */
#ifndef GCC_NODISCARD
  #if __cplusplus >= 201703L  /* C++17 */
    #define GCC_NODISCARD  [[nodiscard]]
  #elif defined __GNUC__
    #define GCC_NODISCARD  __attribute__((warn_unused_result))
  #else
    #define GCC_NODISCARD  /* Ignored */
  #endif
#endif

/* Indicates that the memory to which the return value points always contains no pointers */
#ifndef GCC_MALLOC
  #if defined __GNUC__
    #define GCC_MALLOC  __attribute__((malloc))
  #else
    #define GCC_MALLOC  /* Ignored */
  #endif
#endif

/* Indicates that the return value is always non-NULL */
#ifndef GCC_RETURNS_NONNULL
  #if defined __GNUC__
    #define GCC_RETURNS_NONNULL  __attribute__((returns_nonnull))
  #else
    #define GCC_RETURNS_NONNULL  /* Ignored */
  #endif
#endif
#define GCC_RET_NONNULL GCC_RETURNS_NONNULL

/* Attributes about the side-effects of a function */

/* Indicates that the function does not write to any non-local objects or static local objects */
#ifndef GCC_PURE
  #if defined __GNUC__
    #define GCC_PURE  __attribute__((pure))
  #else
    #define GCC_PURE  /* Ignored */
  #endif
#endif

/* Indicates that the function does not read or write to any non-local objects or static local objects */
#ifndef GCC_CONSTEXPR
  #if defined __GNUC__
    #define GCC_CONSTEXPR  __attribute__((const))
  #else
    #define GCC_CONSTEXPR  /* Ignored */
  #endif
#endif

/* General/other attributes */

/* Indicates that the array of `char` or the pointer to the array may contain a byte sequence without the `\0` string end */
#ifndef GCC_NONSTRING
  #if defined __GNUC__ && !defined __clang__ && __GNUC__ >= 8 /* GCC 8 */
    #define GCC_NONSTRING __attribute__((nonstring))
  #else
    #define GCC_NONSTRING /* Ignored */
  #endif
#endif

/* Indicates that the object is unused in some build configurations */
#ifndef GCC_UNUSED
  #if defined __GNUC__
    #define GCC_UNUSED    __attribute__((unused))
  #else
    #define GCC_UNUSED    /* Ignored */
  #endif
#endif

/* Indicates that the object is deprecated
 * and a message providing available alternatives will be shown if used */
#ifndef GCC_DEPRECATED
  #if __cplusplus >= 201402L  /* C++14 */
    #define GCC_DEPRECATED(msg_str)  [[deprecated(msg_str)]]
  #elif defined __GNUC__
    #define GCC_DEPRECATED(msg_str)  __attribute__((deprecated(msg_str)))
  #else
    #define GCC_DEPRECATED(msg_str)  /* Ignored */
  #endif
#endif

/* Indicates that the fall through behavior of the `switch` case which ends at the null statement where the attribute applies is intentional */
#ifndef GCC_FALLTHROUGH
  #if __cplusplus >= 201703L  /* C++17 */
    #define GCC_FALLTHROUGH  [[fallthrough]]
  #elif defined __GNUC__
    #define GCC_FALLTHROUGH  __attribute__((fallthrough))
  #else
    #define GCC_FALLTHROUGH  /* Ignored */
  #endif
#endif

#endif                          /* ATTRDEF_H */
