/*-------------------------------------------------------*/
/* modes.h      ( NTHU CS MapleBBS Ver 3.02 )            */
/*-------------------------------------------------------*/
/* target : user operating mode & status                 */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/


#ifndef MODES_H
#define MODES_H

enum
{STRIP_ALL, ONLY_COLOR, NO_RELOAD};

/* ----------------------------------------------------- */
/* user 操作狀態與模式                                   */
/* ----------------------------------------------------- */

#define M_IDLE          0
#define M_MMENU         1       /* menu mode */
#define M_ADMIN         2
#define M_MAIL          3
#define M_TMENU         4
#define M_UMENU         5
#define M_SMENU         6
#define M_XMENU         7
#define M_PROFESS       8
#define M_CLASS         9

#define M_LOGIN         10      /* main menu */

#define M_GEM           11      /* announce */

#define M_BOARD         12
#define M_READA         13

#define M_XFILES        14
#define M_UFILES        15      /* user menu */

#define M_RMAIL         16      /* mail menu */

#define M_PAL           17
#define M_LUSERS        18

#define M_BBTP          19
#define M_WEB           20

#define M_POST          21      /* -> active mode : 不能接受 talk request */
#define M_SMAIL         22      /* Thor.981020: 目前為 M_BBTP與 M_CHAT間不talk*/
#define M_TRQST         23
#define M_TALK          24
#define M_CHAT          25

#define M_PAGE          26
#define M_QUERY         27      /* <- mated mode : 可接對象 */

#define M_VOTE          28

#define M_SYSTEM        29
#define M_BMW_REPLY     30      /* lkchu.981201: 熱訊回應 */
#define M_BMW           31      /* lkchu.981230: 熱訊回顧*/
#define M_BANMAIL       32      /* visor: 擋信列表 */
#define M_OMENU         33
#define M_CHESS         M_CHAT

#define M_MIME          34      /* statue: mime 解碼 */
#define M_CHICKEN       35
#define M_XMODE         36
#define M_MYFAVORITE    37

#define M_MAX           M_MYFAVORITE
#define M_BBSNET        0


#ifdef  MODES_C
static const char *ModeTypeTable[] =
{
  "發呆",           /* M_IDLE */
  "主選單",         /* M_MMENU */
  "系統維護",       /* M_ADMIN */
  "郵件選單",       /* M_MAIL */
  "交談選單",       /* M_TMENU */
  "使用者選單",     /* M_UMENU */
  "系統資訊",       /* M_SMENU */
  "工具選單",       /* M_XMENU */
  "專業看板",       /* M_PROFESS */
  "分類看板",       /* M_CLASS */
  "上站途中",       /* M_LOGIN */
  "公佈欄",         /* M_GEM */
  "看板列表",       /* M_BOARD */
  "閱\讀文章",      /* M_READA */
  "編系統檔案",     /* M_XFILES */
  "編個人檔案",     /* M_UFILES */
  "讀信",           /* M_RMAIL */
  "結交朋友",       /* M_PAL */
  "使用者名單",     /* M_LUSERS */
  "[BBTP]",         /* M_BBTP */
  "[WEB]",          /* M_WEB */
  "發表文章",       /* M_POST */
  "寫信",           /* M_SMAIL */
  "待機",           /* M_TRQST */
  "交談",           /* M_TALK */
  "聊天室",         /* M_CHAT */
  "呼叫",           /* M_PAGE */
  "查詢",           /* M_QUERY */
  "投票中",         /* M_VOTE */
  "系統管理",       /* M_SYSTEM */
  "熱訊回應",       /* M_BMW_REPLY *//*熱訊回應*/
  "查看訊息",       /* M_BMW *//*查看訊息*/
  "擋信列表",       /* M_BANMAIL */
  "其他選單",       /* M_OMENU */
  "MIME讀信",       /* M_MIME */
  "養雞場",         /* M_CHICKEN */
  "其他",           /* M_XMODE */
  "我的最愛"        /* MyFavorite */
};


#endif                          /* MODES_C */


/* ----------------------------------------------------- */
/* menu.c 中的模式                                       */
/* ----------------------------------------------------- */


#define XEASY   0x333           /* Return value to un-redraw screen */
#define QUIT    0x666           /* Return value to abort recursive functions */
#define SKIN    0x999           /* Return value to change skin */


/* ----------------------------------------------------- */
/* read.c 中的模式                                       */
/* ----------------------------------------------------- */


#define TAG_NIN         0       /* 不屬於 TagList */
#define TAG_TOGGLE      1       /* 切換 Taglist */
#define TAG_INSERT      2       /* 加入 TagList */


/* for bbstate : bbs operating state */


#define STAT_POST           0x01000000      /* 是否可以在 currboard 發表文章 */
#define STAT_BOARD          0x02000000      /* 是否可以在 currboard 刪除、mark文章 */
#define STAT_VISIT          0x04000000      /* 是否已經看過進板畫面 */
#define STAT_DIRTY          0x08000000      /* 是否更動過 userflag */
#define STAT_MODERATED      0x10000000      /* currboard 是否為 moderated board */
#define STAT_LOCK           0x20000000      /* 是否為鎖定螢幕 */
#define STAT_BM             0x40000000      /* 是否為 currboard 的板主 */
#define STAT_STARTED        0x80000000      /* 是否已經進入系統 */

#define OTHERSTAT_EDITING   0x0000001       /* 編輯中 */


/* for user's board permission level & state record */


#define BRD_R_BIT       0x01    /* 可讀，read */
#define BRD_W_BIT       0x02    /* 可寫，write */
#define BRD_X_BIT       0x04    /* 可管，execute，板主、看板總管、站長 */
#define BRD_V_BIT       0x08    /* 已經逛過了 ==> 看過「進板畫面」 */
#define BRD_H_BIT       0x10    /* .boardrc 中有閱讀記錄 (history) */
#define BRD_Z_BIT       0x20    /* zap 掉了 */
#define BRD_F_BIT       0x40    /* 好友看板*/

/* for curredit */


#define EDIT_MAIL       0x01    /* 目前是 mail/board ? */
#define EDIT_LIST       0x02    /* 是否為 mail list ? */
#define EDIT_BOTH       0x04    /* both reply to author/board ? */
#ifdef HAVE_ANONYMOUS
#define EDIT_ANONYMOUS  0x08    /* 匿名模式 */
#endif  /* #ifdef HAVE_ANONYMOUS */
#define EDIT_OUTGO      POST_OUTGO
#define EDIT_RESTRICT   0x10

/* ----------------------------------------------------- */
/* xover.c 中的模式                                      */
/* ----------------------------------------------------- */

#if 1
#define XO_DL           0x80000000
#endif

#define XO_MODE         0x10000000


#define XO_NONE         (XO_MODE + 0)
#define XO_INIT         (XO_MODE + 1)
#define XO_LOAD         (XO_MODE + 2)
#define XO_HEAD         (XO_MODE + 3)
#define XO_NECK         (XO_MODE + 4)
#define XO_BODY         (XO_MODE + 5)
#define XO_FOOT         (XO_MODE + 6)
#define XO_LAST         (XO_MODE + 7)
#define XO_QUIT         (XO_MODE + 8)

#define XO_RSIZ         256             /* max record length */
#define XO_TALL         (b_lines - 3)   /* page size = b_lines - 3 (扣去 head/neck/foot 共三行) */


#define XO_MOVE         0x20000000      /* cursor movement */
#define XO_WRAP         0x08000000      /* cursor wrap in movement */
#define XO_TAIL         (XO_MOVE - 999) /* init cursor to tail */


#define XO_ZONE         0x40000000      /* 進入某一個 zone */
#define XZ_BACK         0x100


#define XZ_CLASS        (XO_ZONE + 0)   /* 看板列表 */
#define XZ_ULIST        (XO_ZONE + 1)   /* 線上使用者名單 */
#define XZ_PAL          (XO_ZONE + 2)   /* 好友名單 */
#define XZ_VOTE         (XO_ZONE + 3)   /* 投票 */
#define XZ_BMW          (XO_ZONE + 4)   /* 熱訊 */

#ifdef HAVE_XYPOST
#define XZ_XPOST        (XO_ZONE + 5)   /* 搜尋文章模式 */
#endif

/* 以下的有 thread 主題式閱讀的功能 */

#define XZ_MBOX         (XO_ZONE + 6)   /* 信箱 */
#define XZ_BOARD        (XO_ZONE + 7)   /* 看板 */
#define XZ_GEM          (XO_ZONE + 8)   /* 精華區 */
#define XZ_MAILGEM      (XO_ZONE + 9)   /* 信件精華區 */
#define XZ_BANMAIL      (XO_ZONE + 10)  /* 擋信列表 */
#define XZ_OTHER        (XO_ZONE + 11)  /* 其他列表 */
#define XZ_MYFAVORITE   (XO_ZONE + 12)  /* 我的最愛 */

#define XZ_POST         XZ_BOARD



//cache, 20101119

#define PALTYPE_PAL     0               /* 朋友名單 */
#define PALTYPE_LIST    1               /* 群組名單 */
#define PALTYPE_BPAL    2               /* 板友名單 */
#define PALTYPE_VOTE    3               /* 限制投票名單 */

#endif                          /* MODES_H */
