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
# define VAR
# define INI(x)         = x
#else
# define VAR            extern
# define INI(x)
#endif

VAR void (*s_menu)(void);
VAR int skin INI(1);

#undef  VAR
#undef  INI

#endif                          /* SKIN_H */
