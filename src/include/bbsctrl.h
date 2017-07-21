/*------------------------------------------------------*/
/* bbsctrl.h     ( YZU WindTopBBS Ver 3.02 )		*/
/*------------------------------------------------------*/
/* author : visor.bbs@bbs.yzu.edu.tw			*/
/* target : header for WindTopBBS control util		*/
/* create : 						*/
/* update : 						*/
/*------------------------------------------------------*/

#ifndef	_BBSCTRL_H_
#define	_BBSCTRL_H_

#ifdef  _BBSCTRL_C_
# define VAR
# define INI(x)		= x
#else
# define VAR		extern
# define INI(x)
#endif

#define	TITLE		"YZU WindTopBBS Control Util Ver 0.0.1 [20000927]"

#include "../bbsctrl/bbsctrl.p"

#define	TR_USR		0x00000001
#define	TR_BRD		0x00000010
#define	TR_GEM		0x00000100
#define	UTIL_OPEN	0x10000000

typedef	struct
{
  char key;
  void (*init)();
  int (*opts)();
  int (*help)();
  int (*main)();
  void (*outs)();
  int open;
  char *des;
}	PKEY_MAP;

typedef struct
{
  char key;
  int (*main)();
  int open;
  char *des;
}       GKEY_MAP;

/*------------------------------------------------------*/
/* declare keymap					*/
/*------------------------------------------------------*/
#ifdef  _BBSCTRL_C_

PKEY_MAP pmap[] = {
  'a',	c_age_init, c_age_opts, c_age_help, c_age_main, 
  c_age_outs, TR_USR, "参璸闹だガ",
  
  'b', c_birth_init, c_birth_opts, c_birth_help, c_birth_main,
  c_birth_outs, TR_USR, "参璸关琍参璸",
  
  'B', c_bm_init, c_bm_opts, c_bm_help, c_bm_main,
  c_bm_outs, TR_USR, "参璸",

  0, NULL, NULL, NULL, NULL, NULL, 0, NULL
};

GKEY_MAP gmap[] = {
  'h', help, 0, "ㄏノ弧",
  
  0, NULL, 0, NULL
};

#endif
#endif

