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

/* General/other attributes */

#ifndef GCC_UNUSED
  #if defined __GNUC__
    #define GCC_UNUSED    __attribute__((__unused__))
  #else
    #define GCC_UNUSED    /* Ignored */
  #endif
#endif

#endif                          /* ATTRDEF_H */
