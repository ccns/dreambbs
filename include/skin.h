/*-------------------------------------------------------*/
/* skin.h	( YZU WindTopBBS Ver 3.02 )		 */
/*-------------------------------------------------------*/
/* target : skin definitions & variables		 */
/* create : 					 	 */
/* update : 					 	 */
/*-------------------------------------------------------*/
#ifndef	_SKIN_H_
#define	_SKIN_H_

#ifdef	_MAIN_C_
# define VAR
# define INI(x) 	= x
#else
# define VAR		extern
# define INI(x)
#endif


VAR void (*s_menu)(void);
VAR int skin INI(1);

#undef	VAR
#undef	INI

#endif				/* _SKIN_H_ */
