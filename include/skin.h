/*-------------------------------------------------------*/
/* skin.h       ( YZU WindTopBBS Ver 3.02 )              */
/*-------------------------------------------------------*/
/* target : skin definitions & variables                 */
/* create :                                              */
/* update :                                              */
/*-------------------------------------------------------*/

#ifndef SKIN_H
#define SKIN_H

#ifdef  MAIN_C
# undef MAIN_C          /* For including declarations */
# undef SKIN_H          /* Temporarily disable the header guard */
# include __FILE__      /* Include the declarations */
# define MAIN_C         /* Restore `MAIN_C` */
# define VAR
# define INI(...)       = __VA_ARGS__
#else
# define VAR            extern
# define INI(...)
#endif

#ifdef __cplusplus
extern "C" {
#endif

VAR void (*s_menu)(void);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#undef  VAR
#undef  INI

#endif                          /* SKIN_H */
