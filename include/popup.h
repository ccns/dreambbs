/*-------------------------------------------------------*/
/* popup.h       ( YZU_CSE WindTop BBS )                 */
/*-------------------------------------------------------*/
/* author : verit.bbs@bbs.yzu.edu.tw                     */
/* target : popup menu                                   */
/* create : 2003/02/12                                   */
/*-------------------------------------------------------*/


#ifndef POPUP_H
#define POPUP_H

#include "modes.h"

#define POPUP_QUIT              M_QUIT
#define POPUP_FUN               M_FUN
#define POPUP_XO                (M_FUN | M_XO)
#define POPUP_MENU              M_MENU
#define POPUP_MENUTITLE         (M_MENU | M_MENUTITLE)
#if NO_SO
  #define POPUP_SO              M_FUN
#else
  #define POPUP_SO              M_DL(M_FUN) /* For dynamic library loading */
#endif

#define POPUP_ARG               M_ARG  /* `item` is a function and a `void *` argument */

#endif  /* #ifndef POPUP_H */
