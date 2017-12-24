/*-------------------------------------------------------*/
/* bbs.h	( NTHU CS MapleBBS Ver 2.36 )		 */
/*-------------------------------------------------------*/
/* target : all header files			 	 */
/* create : 95/03/29				 	 */
/* update : 95/12/15				 	 */
/*-------------------------------------------------------*/


#ifndef	_BBS_H_
#define	_BBS_H_


#include <stdio.h>
#include <setjmp.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/wait.h>

#ifdef AIX
#include <time.h>
#else
#ifndef LINUX
#include <sys/time.h>
#else
#include <time.h>
#endif
#endif


#ifdef	LINUX
#include <sgtty.h>
#else
#include <termios.h>
#endif

#ifdef  SYSV

#ifndef LOCK_EX
#define LOCK_EX		F_LOCK
#define LOCK_UN		F_ULOCK
#endif

#define getdtablesize()         (64)

#define usleep(usec)            {               \
    struct timeval t;                           \
    t.tv_sec = usec / 1000000;                  \
    t.tv_usec = usec % 1000000;                 \
    select( 0, NULL, NULL, NULL, &t);           \
}

#endif				/* SYSV */


#define	BMIN(a,b)	((a<b)?a:b)
#define	BMAX(a,b)	((a>b)?a:b)


/* #define	countof(x)		(sizeof(x)/sizeof(x[0])) */


#define	YEA	(1)		/* Booleans  (Yep, for true and false) */
#define	NA	(0)


#ifndef	_BBTP_


#define NOECHO		0x0000		/* Flags to getdata input function */
#define DOECHO		0x0100
#define LCECHO		0x0200
#define GCARRY		0x0400

#define	GET_LIST	0x1000		/* 取得 Link List */
#define	GET_USER	0x2000		/* 取得 user id */
#define	GET_BRD		0x4000		/* 取得 board id */


#define	b_lines		(23)


#include "config.h"		/* User-configurable stuff */
#include "struct.h"		/* data structure */
#include "dao.h"
#include "perm.h"		/* user/board permission */
#include "modes.h"		/* The list of valid user modes */
#include "global.h"		/* global variable & definition */
#include "proto.h"		/* prototype of functions */
#include "skin.h"		/* skin definition */
#include "attr.h"		/* Thor.990311: dynamic attribute database */
#include "popup.h"		/* popup menu define */ 
#include "battr.h"		/* board attribution */
#include "theme.h"		/* custom theme */

#endif

#endif				/* _BBS_H_ */
