/*-------------------------------------------------------*/
/* global.h     ( NTHU CS MapleBBS Ver 3.02 )            */
/*-------------------------------------------------------*/
/* target : global variables                             */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/
#ifndef GLOBAL_H
#define GLOBAL_H

#include "config.h"
#include "cppdef.h"
#include "global_def.h"
#ifdef NO_SO
  #include "bbs_script.h"
  #include "struct.h"
#endif


#ifdef  MAIN_C
# undef MAIN_C          /* For including declarations */
# undef GLOBAL_H        /* Temporarily disable the header guard */
# include __FILE__      /* Include the declarations */
# define MAIN_C         /* Restore `MAIN_C` */
# define VAR
# define INI(x)         = x
#else
# define VAR            extern
# define INI(x)
#endif

/* ----------------------------------------------------- */
/* GLOBAL VARIABLE                                       */
/* ----------------------------------------------------- */

#ifdef __cplusplus
extern "C" {
#endif

VAR pid_t currpid;              /* current process ID */
VAR unsigned int bbsmode;       /* bbs operating mode, see modes.h */
VAR int bbstate;                /* bbs operating state */
VAR int bbsothermode;
VAR int supervisor;
VAR UTMP *cutmp;
VAR int curredit;
VAR int checkqt;
VAR int showansi INI(1);
#define FT_COLOR_NOCOLOR  (!showansi)  /* For pfterm */
VAR time_t ap_start;
VAR ACCT cuser;                 /* current user structure */
VAR time_t currchrono;          /* current file timestamp @ bbs.c mail.c */

VAR int b_lines;                /* bottom line */
// VAR int t_lines;
VAR int b_cols;                 /* bottom columns */
VAR int d_cols;                 /* difference columns from standard */

VAR char fromhost[48];

VAR char quote_file[80];
VAR char quote_user[80];
VAR char quote_nick[80];
VAR char currtitle[80];

VAR char hunt[32];              /* hunt keyword */

VAR char ve_title[80];
VAR char currboard[IDLEN + 2];          /* name of currently selected board */
VAR char currBM[BMLEN + 7];             /* BM of currently selected board */
VAR int  currbno        INI(-1);
VAR const char str_ransi[4]   INI("\x1b[m");
VAR unsigned int currbattr;            /* currently selected board battr */
VAR char ipv6addr[INET6_ADDRSTRLEN];   /* User's IP (IPv4 or IPv6) */

VAR int  chk_mailstat   INI(0);

/* filename */
VAR const char *const fn_dir          INI(FN_DIR);

/* message */
VAR const char *const msg_separator   INI(MSG_SEPARATOR);

VAR const char *const msg_cancel      INI(MSG_CANCEL);

VAR const char *const msg_sure_ny     INI(MSG_SURE_NY);

VAR const char *const msg_uid         INI(MSG_UID);

VAR const char *const msg_del_ny      INI(MSG_DEL_NY);

VAR const char *const err_bid         INI(ERR_BID);
VAR const char *const err_uid         INI(ERR_UID);

VAR const char *const str_sysop       INI(STR_SYSOP);
VAR const char *const brd_sysop       INI("SYSOP");
VAR const char *const str_author1     INI(STR_AUTHOR1);
VAR const char *const str_author2     INI(STR_AUTHOR2);
VAR const char *const str_post1       INI(STR_POST1);
VAR const char *const str_post2       INI(STR_POST2);
VAR const char *const str_host        INI(MYHOSTNAME);
VAR const char *const str_site        INI(BOARDNAME);

#ifdef  HAVE_RECOMMEND
VAR int recommend_time  INI(0);
#endif

#if 0
VAR int aprilfirst      INI(0);
#endif

VAR int total_num;                  /* 重設 站上人數*/

/* Configuration for `x_siteinfo()` */

/* Client version information */
VAR const char bbsvername[]           INI(BBSVERNAME);
VAR const char bbsversion[]           INI(BBSVERSION);

/* Git version information */
VAR const char build_make[]           INI(BUILD_MAKE);
VAR const char build_arch[]           INI(BUILD_ARCH);
VAR const char build_branch[]         INI(BUILD_BRANCH);
VAR const char build_head[]           INI(BUILD_HEAD);
VAR const char build_branch_remote[]  INI(BUILD_BRANCH_REMOTE);
VAR const char build_head_remote[]    INI(BUILD_HEAD_REMOTE);
VAR const char build_remote_url[]     INI(BUILD_REMOTE_URL);
VAR const char build_date[]           INI(BUILD_DATE);
VAR const time_t build_time           INI(BUILD_TIME);

/* Compiler information */
#define __cplusplus_IS_DEF  IS_DEF_TEST
#define __clang___IS_DEF  IS_DEF_TEST

VAR const char build_compiler[]   INI(
    IF_DEF(__clang__,
        "Clang-" VER_PATCH_STR(__clang_major__, __clang_minor__, __clang_patchlevel__),
        "GCC-" VER_PATCH_STR(__GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__))
);
VAR const char build_lang[]       INI(IF_DEF(__cplusplus, "GNU-C++", "GNU-C"));
VAR const int build_langver       INI(IF_DEF(__cplusplus, (int)__cplusplus, (int)__STDC_VERSION__));

#undef __cplusplus_IS_DEF
#undef __clang___IS_DEF

/* Client module status information */
#define MODULE_MultiRecommend     (1U << 0)
#define MODULE_M3_USE_PMORE       (1U << 1)
#define MODULE_M3_USE_PFTERM      (1U << 2)
#define MODULE_GRAYOUT            (1U << 3)
#define MODULE_M3_USE_BBSLUA      (1U << 4)
#define MODULE_M3_USE_BBSRUBY     (1U << 5)
#define MODULE_DL_HOTSWAP         (1U << 6)

#ifdef  HAVE_BBSLUA
VAR const char bbslua_version_str[]   INI(
    BBSLUA_VERSION_STR
    IF_ON(M3_USE_BBSLUA, " / " LUA_RELEASE IF_ON(BBSLUA_USE_LUAJIT, " / " LUAJIT_VERSION))
);
#endif
#ifdef  HAVE_BBSRUBY
VAR const char bbsruby_version_str[]  INI(
    BBSRUBY_VERSION_STR " Interface: " BBSRUBY_INTERFACE_VER_STR
    IF_ON(M3_USE_BBSRUBY, " / Ruby " RUBY_RELEASE_STR
        IF_ON(BBSRUBY_USE_MRUBY, " / " MRUBY_RUBY_ENGINE " " MRUBY_VERSION))
);
#endif

#define MODULE_STATUS(conf)  IF_ON(conf, MODULE_ ## conf, 0)
VAR const unsigned int module_flags   INI(
    MODULE_STATUS(MultiRecommend)
    | MODULE_STATUS(M3_USE_PMORE)
    | MODULE_STATUS(M3_USE_PFTERM)
    | MODULE_STATUS(GRAYOUT)
    | MODULE_STATUS(M3_USE_BBSLUA)
    | MODULE_STATUS(M3_USE_BBSRUBY)
    | MODULE_STATUS(DL_HOTSWAP)
);
#undef MODULE_STATUS

#undef  VAR
#undef  INI

#ifdef NO_SO  /* For the main program and the modules of `bbsd` */
/* maple/bbsd.c */
extern Logger g_logger;      /* IID.2021-02-09: The global message logger */
extern char rusername[];     /* Thor: 記錄 RFC931, 怕guest verify */

/* maple/board.c */
extern int boardmode;
extern char brd_bits[];
extern time_t brd_visit[MAXBOARD];     /* 最近瀏覽時間 */

/* maple/cache.c */
extern BCACHE *bshm;
extern UCACHE *ushm;
extern FWCACHE *fwshm;

/* maple/edit.c */
extern int wordsnum;            /* itoc.010408: 計算文章字數 */
extern int keysnum;

/* maple/mail.c */
extern LinkList *ll_head;

/* maple/post.c */
extern KeyFuncList post_cb;
#ifdef  HAVE_DETECT_CROSSPOST
extern CHECKSUMCOUNT cksum;
#endif

/* maple/talk.c */
extern int pickup_way;
extern KeyFuncList bmw_cb;

/* maple/visio.c */
extern screenline *cur_slp;
#ifdef EVERY_Z
extern int vio_fd;              /* Thor.0725: 為talk, chat可用^z作準備 */
extern int holdon_fd;
#endif
extern char lastcmd[MAXLASTCMD][80];

/* maple/xover.c */
extern XZ xz[];
/* XO's data pool */
GCC_DEPRECATED("Use `xo_pool_base` instead")
extern char *xo_pool;           /* The `xo->top`-st item of the pool */
extern char *xo_pool_base;      /* The beginning of the pool */
/* Tags */
extern int TagNum;              /* Thor.0724: For tag_char */
extern TagItem TagList[];

#ifdef MODE_STAT
extern UMODELOG modelog;
extern time_t mode_lastchange;
#endif
#endif  /* #ifdef NO_SO */

#ifdef __cplusplus
}  // extern "C"
#endif

#endif                          /* GLOBAL_H */
