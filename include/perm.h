/*-------------------------------------------------------*/
/* perm.h       ( NTHU CS MapleBBS Ver 3.02 )            */
/*-------------------------------------------------------*/
/* target : permission levels of user & board            */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/


#ifndef PERM_H
#define PERM_H


/* ----------------------------------------------------- */
/* These are the 32 basic permission bits.               */
/* ----------------------------------------------------- */

#define PERM_BASIC      0x00000001      /* 1-8 : 基本權限 */
#define PERM_CHAT       0x00000002
#define PERM_PAGE       0x00000004
#define PERM_POST       0x00000008
#define PERM_VALID      0x00000010      /* LOGINOK */
#define PERM_MBOX       0x00000020
#define PERM_CLOAK      0x00000040
#define PERM_XEMPT      0x00000080

#define PERM_9          0x00000100
#define PERM_10         0x00000200
#define PERM_11         0x00000400
#define PERM_12         0x00000800
#define PERM_13         0x00001000
#define PERM_14         0x00002000
#define PERM_15         0x00004000
#define PERM_SP         0x00008000

#define PERM_DENYPOST   0x00010000      /* 17-24 : 禁制權限 */
#define PERM_DENYTALK   0x00020000
#define PERM_DENYCHAT   0x00040000
#define PERM_DENYMAIL   0x00080000
#define PERM_DENYSTOP   0x00100000
#define PERM_DENYNICK   0x00200000
#define PERM_DENYLOGIN  0x00400000
#define PERM_PURGE      0x00800000

#define PERM_BM         0x01000000      /* 25-32 : 管理權限 */
#define PERM_SEECLOAK   0x02000000
#define PERM_KTV        0x04000000
#define PERM_GEM        0x08000000
#define PERM_ACCOUNTS   0x10000000
#define PERM_CHATROOM   0x20000000
#define PERM_BOARD      0x40000000
#define PERM_SYSOP      0x80000000
#define PERM_SYSOPX     (PERM_SYSOP | PERM_BOARD | PERM_CHATROOM | PERM_ACCOUNTS)

#define PERM_MANAGE     0xFE0000E0
#define PERM_CRIMINAL   0x007F0000


/* ----------------------------------------------------- */
/* These permissions are bitwise ORs of the basic bits.  */
/* ----------------------------------------------------- */


/* This is the default permission granted to all new accounts. */
#define PERM_DEFAULT    (PERM_BASIC)

#define PERM_ADMIN      (PERM_BOARD | PERM_ACCOUNTS | PERM_SYSOP | PERM_CHATROOM | PERM_KTV)
#define PERM_ALLBOARD   (PERM_SYSOP | PERM_BOARD)
#define PERM_LOGINCLOAK (PERM_SYSOP | PERM_ACCOUNTS | PERM_BOARD | PERM_CHATROOM)
#define PERM_SEEULEVELS PERM_SYSOP
#define PERM_SEEBLEVELS (PERM_SYSOP | PERM_BM)

#define PERM_BBSLUA     (PERM_BASIC)
#define PERM_BBSRUBY    (PERM_BASIC)


/*
#define PERM_NOTIMEOUT  PERM_SYSOP
*/

#define PERM_READMAIL   PERM_BASIC

#define PERM_INTERNET   PERM_VALID      /* 身份認證過關的才能寄信到
                                            Internet */

#define PERM_FORWARD    PERM_INTERNET   /* to do the forwarding */

/*
#define HAS_PERM(x)     ((x)?cuser.userlevel&(x):1)
*/
#define HAS_PERM        HAVE_PERM
#define HAVE_PERM(x)    (cuser.userlevel&(x))


/* ----------------------------------------------------- */
/* 各種權限的中文意義                                    */
/* ----------------------------------------------------- */

#define NUMPERMS        32


#ifdef ADMIN_C


static const char *const perm_tbl[] = {
  "基本權力",                   /* PERM_BASIC */
  "進入聊天室",                 /* PERM_CHAT */
  "找人聊天",                   /* PERM_PAGE */
  "發表文章",                   /* PERM_POST */
  "身分認證",                   /* PERM_VALID */
  "信件無上限",                 /* PERM_MBOX */
  "隱身術",                     /* PERM_CLOAK */
  "永久保留帳號",               /* PERM_XEMPT */

  "公告看板發文權限",           /* PERM_9 */
  NULL,                         /* PERM_10 */
  NULL,                         /* PERM_11 */
  NULL,                         /* PERM_12 */
  NULL,                         /* PERM_13 */
  NULL,                         /* PERM_14 */
  NULL,                         /* PERM_15 */
  "特殊標記",                   /* PERM_16 */

  "禁止發表文章",               /* PERM_DENYPOST */
  "禁止 talk",                  /* PERM_DENYTALK */
  "禁止 chat",                  /* PERM_DENYCHAT */
  "禁止 mail",                  /* PERM_DENYMAIL */
  "停權",                       /* PERM_DENYSTOP */
  "禁止更改暱稱",               /* PERM_DENYNICK */
  "禁止 login",                 /* PERM_DENYLOGIN */
  "清除帳號",                   /* PERM_PURGE */

  "板主",                       /* PERM_BM */
  "看見忍者",                   /* PERM_SEECLOAK */
  "點歌系統總管",               /* PERM_KTV */
  "精華區總管",                 /* PERM_GEM */
  "帳號總管",                   /* PERM_ACCOUNTS */
  "聊天室總管",                 /* PERM_CHATCLOAK */
  "看板總管",                   /* PERM_BOARD */
  "站長"                        /* PERM_SYSOP */
};

#endif  /* #ifdef ADMIN_C */


/* ----------------------------------------------------- */
/* 精華區 (gem / gopher) 中的 user permission level      */
/* ----------------------------------------------------- */


#define GEM_QUIT        -2      /* recursive quit */
#define GEM_VISIT       -1
#define GEM_USER        0
#define GEM_RECYCLE     1
#define GEM_LMANAGER    2
#define GEM_MANAGER     3
#define GEM_SYSOP       4


#endif                          /* PERM_H */
