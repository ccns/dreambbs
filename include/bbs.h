/*-------------------------------------------------------*/
/* bbs.h        ( NTHU CS MapleBBS Ver 3.02 )            */
/*-------------------------------------------------------*/
/* target : all header files                             */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/

#ifndef BBS_H
#define BBS_H

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
#include <sys/ipc.h>
#include <sys/shm.h>
#include "cppdef.h"             /* Preprocessor utility macros */
#include "config.h"             /* User-configurable stuff */
#include "struct.h"             /* data structure */
#include "dao.h"                /* common library */
#include "perm.h"               /* user/board permission */
#include "modes.h"              /* The list of valid user modes */
#include "global.h"             /* global variable & definition */
#include "proto.h"              /* prototype of functions */
#include "skin.h"               /* skin definition */
#include "attr.h"               /* Thor.990311: dynamic attribute database */
#include "popup.h"              /* popup menu define */
#include "battr.h"              /* board attribution */
#include "theme.h"              /* custom theme */

#ifdef __linux__
    #include <time.h>
#else
    #include <sys/time.h>
#endif // #ifdef __linux__

#ifdef __linux__
    #include <sgtty.h>
#else
    #include <termios.h>
#endif // #ifdef __linux__

#ifdef  SYSV

#ifndef LOCK_EX
#define LOCK_EX         F_LOCK
#define LOCK_UN         F_ULOCK
#endif /* LOCK_EX */


#define getdtablesize()         (64)

#define usleep(usec)         do {               \
    struct timeval t;                           \
    t.tv_sec = (usec) / 1000000;                \
    t.tv_usec = (usec) % 1000000;               \
    select( 0, NULL, NULL, NULL, &t);           \
} while (0)

#endif  /* SYSV */


#define BMIN(a, b)      ((a<b)?a:b)
#define BMAX(a, b)      ((a>b)?a:b)

/* #define countof(x)      (sizeof(x)/sizeof(x[0])) */

#define YEA     (1)             /* Booleans  (Yep, for true and false) */
#define NA      (0)

#ifndef _BBTP_

#define NOECHO          0x0000          /* Flags to getdata input function */
#define DOECHO          0x0100
#define LCECHO          0x0200
#define NUMECHO         0x0400
#define GCARRY          0x0800
#define PASSECHO        (NOECHO)        /* `NOECHO` without `VGET_STEALTH_NOECHO` */

#define GET_LIST        0x1000          /* 取得 Link List */
#define GET_USER        0x2000          /* 取得 user id */
#define GET_BRD         0x4000          /* 取得 board id */

#define VGET_IMPLY_DOECHO (LCECHO | NUMECHO | GCARRY)  /* The flags which imply `DOECHO` */
#define VGET_FORCE_DOECHO (GET_LIST | GET_USER | GET_BRD)  /* The flags which force `DOECHO` */

#define VGET_STRICT_DOECHO  0x0001      /* Show the input only if `DOECHO` is set (ignore implications) */
#define VGET_STEALTH_NOECHO 0x0002      /* Hide the entire input field if `DOECHO` is not set */
#define VGET_BREAKABLE      0x0004      /* Whether Ctrl-C closes the input field */

#define VGET_EXIT_BREAK     -1          /* The input field is closed with Ctrl-C */

#endif  /* #ifndef _BBTP_ */

#endif                          /* BBS_H */
