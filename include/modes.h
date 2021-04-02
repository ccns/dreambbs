/*-------------------------------------------------------*/
/* modes.h      ( NTHU CS MapleBBS Ver 3.02 )            */
/*-------------------------------------------------------*/
/* target : user operating mode & status                 */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/


#ifndef MODES_H
#define MODES_H

#include "perm.h"

enum
{STRIP_ALL, ONLY_COLOR, NO_RELOAD};

/* For `genpasswd()` */
#define GENPASSWD_DES      0
#define GENPASSWD_SHA256   5

/* For `dbcs_state()` */
enum DbcsState {
    DBCS_ASCII = 0,

    /* The bit range 0x1 - 0x8 is reserved for encoding the byte position in the DBCS character */
    DBCS_INTERESC = 0x40,
    DBCS_LEAD = 0x20,
    DBCS_TRAIL = 0x10,

    /* ANSI escapes occur between the byte and the trailing byte */
    DBCS_LEAD_INTERESC = DBCS_LEAD | DBCS_INTERESC,
    /* ANSI escapes occur between the leading byte and the byte */
    DBCS_TRAIL_INTERESC = DBCS_TRAIL | DBCS_INTERESC,
};

/* ----------------------------------------------------- */
/* user �ާ@���A�P�Ҧ�                                   */
/* ----------------------------------------------------- */

#define M_MENU          M_MMENU     /* The first menu mode */
#define M_FUN           M_PROFESS   /* The first non-menu, non-idle mode */

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

#define M_POST          21      /* -> active mode : ���౵�� talk request */
#define M_SMAIL         22      /* Thor.981020: �ثe�� M_BBTP�P M_CHAT����talk*/
#define M_TRQST         23
#define M_TALK          24
#define M_CHAT          25

#define M_PAGE          26
#define M_QUERY         27      /* <- mated mode : �i����H */

#define M_VOTE          28

#define M_SYSTEM        29
#define M_BMW_REPLY     30      /* lkchu.981201: ���T�^�� */
#define M_BMW           31      /* lkchu.981230: ���T�^�U*/
#define M_BANMAIL       32      /* visor: �׫H�C�� */
#define M_OMENU         33
#define M_CHESS         M_CHAT

#define M_MIME          34      /* statue: mime �ѽX */
#define M_CHICKEN       35
#define M_XMODE         36
#define M_MYFAVORITE    37

#define M_MAX           M_MYFAVORITE


#ifdef  MODES_C
static const char *const ModeTypeTable[] =
{
  "�o�b",           /* M_IDLE */
  "�D���",         /* M_MMENU */
  "�t�κ��@",       /* M_ADMIN */
  "�l����",       /* M_MAIL */
  "��Ϳ��",       /* M_TMENU */
  "�ϥΪ̿��",     /* M_UMENU */
  "�t�θ�T",       /* M_SMENU */
  "�u����",       /* M_XMENU */
  "�M�~�ݪO",       /* M_PROFESS */
  "�����ݪO",       /* M_CLASS */
  "�W���~��",       /* M_LOGIN */
  "���G��",         /* M_GEM */
  "�ݪO�C��",       /* M_BOARD */
  "�\\Ū�峹",      /* M_READA */
  "�s�t���ɮ�",     /* M_XFILES */
  "�s�ӤH�ɮ�",     /* M_UFILES */
  "Ū�H",           /* M_RMAIL */
  "����B��",       /* M_PAL */
  "�ϥΪ̦W��",     /* M_LUSERS */
  "[BBTP]",         /* M_BBTP */
  "[WEB]",          /* M_WEB */
  "�o��峹",       /* M_POST */
  "�g�H",           /* M_SMAIL */
  "�ݾ�",           /* M_TRQST */
  "���",           /* M_TALK */
  "��ѫ�",         /* M_CHAT */
  "�I�s",           /* M_PAGE */
  "�d��",           /* M_QUERY */
  "�벼��",         /* M_VOTE */
  "�t�κ޲z",       /* M_SYSTEM */
  "���T�^��",       /* M_BMW_REPLY *//*���T�^��*/
  "�d�ݰT��",       /* M_BMW *//*�d�ݰT��*/
  "�׫H�C��",       /* M_BANMAIL */
  "��L���",       /* M_OMENU */
  "MIMEŪ�H",       /* M_MIME */
  "�i����",         /* M_CHICKEN */
  "��L",           /* M_XMODE */
  "�ڪ��̷R"        /* MyFavorite */
};


#endif                          /* MODES_C */


/* ----------------------------------------------------- */
/* menu.c & popupmenu.c �����Ҧ�                         */
/* ----------------------------------------------------- */

/* For `umode` */

#if NO_SO
#define M_DL(umode)     (umode)
#else
#define M_DL(umode)     ((umode) | 0x80000000)  /* For dynamic library loading */
#endif

#define M_QUIT          0x01000000  /* Return to the outer menu after selection */
#define M_XO            0x02000000  /* `item` is a xover function */
#define M_ARG           0x04000000  /* `item` is a function and a `void *` argument */

#define M_DOINSTANT     0x00010000  /* Immidiately select a menu item after command match */
#define M_MENUTITLE     0x00020000  /* `item` is a string for the menu title */
#define M_TAIL_MASK     0x00FF0000  /* Mask for the flags which indicate the menu list tail */

#define M_MASK          0x0000FFFF  /* Mask for valid user modes */

/* For `level` */

#define PERM_MENU       PERM_PURGE  /* A permission flag reserved for valid users */

/* Traditional return values */

#define XEASY   XO_FOOT         /* Return value to un-redraw screen */
#define QUIT    XO_QUIT         /* Return value to abort recursive functions */
#define SKIN    XO_SKIN         /* Return value to change skin */


/* ----------------------------------------------------- */
/* read.c �����Ҧ�                                       */
/* ----------------------------------------------------- */


#define TAG_NIN         0       /* ���ݩ� TagList */
#define TAG_TOGGLE      1       /* ���� Taglist */
#define TAG_INSERT      2       /* �[�J TagList */


/* for bbstate : bbs operating state */


#define STAT_POST           0x01000000      /* �O�_�i�H�b currboard �o��峹 */
#define STAT_BOARD          0x02000000      /* �O�_�i�H�b currboard �R���Bmark�峹 */
#define STAT_VISIT          0x04000000      /* �O�_�w�g�ݹL�i�O�e�� */
#define STAT_DIRTY          0x08000000      /* �O�_��ʹL userflag */
#define STAT_MODERATED      0x10000000      /* currboard �O�_�� moderated board */
#define STAT_LOCK           0x20000000      /* �O�_����w�ù� */
#define STAT_BM             0x40000000      /* �O�_�� currboard ���O�D */
#define STAT_STARTED        0x80000000      /* �O�_�w�g�i�J�t�� */

#define OTHERSTAT_EDITING   0x0000001       /* �s�褤 */


/* for user's board permission level & state record */


#define BRD_R_BIT       0x01    /* �iŪ�Aread */
#define BRD_W_BIT       0x02    /* �i�g�Awrite */
#define BRD_X_BIT       0x04    /* �i�ޡAexecute�A�O�D�B�ݪO�`�ޡB���� */
#define BRD_V_BIT       0x08    /* �w�g�}�L�F ==> �ݹL�u�i�O�e���v */
#define BRD_H_BIT       0x10    /* .boardrc �����\Ū�O�� (history) */
#define BRD_Z_BIT       0x20    /* zap ���F */
#define BRD_F_BIT       0x40    /* �n�ͬݪO*/

/* for curredit */


#define EDIT_MAIL       0x01    /* �ثe�O mail/board ? */
#define EDIT_LIST       0x02    /* �O�_�� mail list ? */
#define EDIT_BOTH       0x04    /* both reply to author/board ? */
#ifdef HAVE_ANONYMOUS
#define EDIT_ANONYMOUS  0x08    /* �ΦW�Ҧ� */
#endif  /* #ifdef HAVE_ANONYMOUS */
#define EDIT_OUTGO      POST_OUTGO
#define EDIT_RESTRICT   0x10

/* ----------------------------------------------------- */
/* xover.c �����Ҧ�                                      */
/* ----------------------------------------------------- */

/* IID.20200128: Reassign xover key values */

/* For specify functions which require dynamic loading */

#if NO_SO
#define XO_DL           0x00000000
#else
#define XO_DL           0x80000000
#endif

/* Screen redraw/reloading modes */

#define XO_REDO_MASK    0x3f000000      /* Apply this mask to get the redraw/reloading mode */

/* Elements of Redraw/reloading (cannot be combined freely for now) */

#define XR_PART_LOAD    0x01000000      /* Reload content */
#define XR_PART_HEAD    0x02000000      /* Redraw header (the title at screen top) */
#define XR_PART_NECK    0x04000000      /* Redraw necker (the description above list) */
#define XR_PART_BODY    0x08000000      /* Redraw body (the list body) */
#define XR_PART_KNEE    0x10000000      /* Redraw knee (the description below list) */
#define XR_PART_FOOT    0x20000000      /* Redraw footer (the status bar at screen bottom) */

/* Predefined combination of elements of redraw/reloading  */

#define XR_INIT         (XR_PART_LOAD | XR_HEAD)
#define XR_LOAD         (XR_PART_LOAD | XR_BODY)
#define XR_HEAD         (XR_PART_HEAD | XR_NECK)
#define XR_NECK         (XR_PART_NECK | XR_BODY)
#define XR_BODY         (XR_PART_BODY | XR_KNEE)
#define XR_KNEE         (XR_PART_KNEE | XR_FOOT)
#define XR_FOOT         (XR_PART_FOOT)

/* Legacy xover modes */

/* Redraw/reloading */
#define XO_NONE         (KEY_NONE)
#define XO_INIT         (XR_INIT + XO_NONE)
#define XO_LOAD         (XR_LOAD + XO_NONE)
#define XO_HEAD         (XR_HEAD + XO_NONE)
#define XO_NECK         (XR_NECK + XO_NONE)
#define XO_BODY         (XR_BODY + XO_NONE)
#define XO_KNEE         (XR_KNEE + XO_NONE)
#define XO_FOOT         (XR_FOOT + XO_NONE)
/* Zone operations (see below) */
#define XO_LAST         ((XZ_ZONE | XZ_BACK) + XO_NONE)
#define XO_QUIT         ((XZ_ZONE | XZ_QUIT) + XO_NONE)

/* Miscellaneous commands ({XO_WRAP|XO_SCRL|XO_REL} + key; key <= KEY_NONE) */
#define XO_CUR          (XO_REL + XO_CUR_BIAS)  /* Redraw the item under the cursor and do relative move */
#define XO_CUR_BIAS     0x2000               /* Bias of relative move */
#define XO_CUR_MIN      (XO_REL + 0)         /* The minimum value of relative move */
#define XO_CUR_MAX      (XO_REL + KEY_NONE)  /* The maximum value of relative move */

/* Special values */

//#define XO_RSIZ         256             /* max record length */ /* IID.20200102: Unlimited. */
#define XO_TALL         (b_lines - 3)   /* page size = b_lines - 3 (���h head/neck/foot �@�T��) */

/* Cursor movements */

#define XO_MOVE_MASK    0x00ffffff      /* Apply this mask to get the cursor movement */
#define XO_POS_MASK     0x001fffff      /* Apply this mask to get the cursor position */

#define XO_MOVE         0x00100000      /* cursor movement bias */
#define XO_WRAP         0x00800000      /* cursor wrap in movement */
#define XO_SCRL         0x00400000      /* Scroll list instead */
#define XO_REL          0x00200000      /* Relative movement */

#define XO_MOVE_MAX     (XO_POS_MASK - XO_MOVE)  /* The maximum value of cursor position */
#define XO_MOVE_MIN     (XO_NONE + 1 - XO_MOVE)  /* The minimum value of cursor position */
#define XO_TAIL         (XO_WRAP - 1)   /* Move or init cursor to tail */

/* Zone operations */

#define XO_ZONE_MASK    0x7f000000      /* Apply this mask to get the zone operation */

#define XZ_ZONE         0x40000000      /* �i�J�Y�@�� zone */
#define XZ_INIT         0x01000000      /* Perform initialization tasks for a zone */
#define XZ_FINI         0x02000000      /* Perform finalization tasks for a zone */
#define XZ_BACK         0x04000000      /* Return to the last zone */
#define XZ_QUIT         0x08000000      /* Exit `xover()` */
#define XZ_SKIN         0x10000000      /* Change the skin instead of the zone */

#define XZ_UNUSED5      0x20000000

#define XO_ZONE         (XZ_ZONE + XO_MOVE)
#define XO_SKIN         ((XZ_ZONE | XZ_SKIN) + XO_MOVE)

/* Zone indexes */

#define XZ_INDEX_CLASS  0               /* �ݪO�C�� */
#define XZ_INDEX_ULIST  1               /* �u�W�ϥΪ̦W�� */
#define XZ_INDEX_PAL    2               /* �n�ͦW�� */
#define XZ_INDEX_VOTE   3               /* �벼 */
#define XZ_INDEX_BMW    4               /* ���T */

#ifdef HAVE_XYPOST
#define XZ_INDEX_XPOST  5               /* �j�M�峹�Ҧ� */
#endif

/* �H�U���� thread �D�D���\Ū���\�� */

#define XZ_INDEX_MBOX     6             /* �H�c */
#define XZ_INDEX_BOARD    7             /* �ݪO */
#define XZ_INDEX_GEM      8             /* ��ذ� */
#define XZ_INDEX_MAILGEM  9             /* �H���ذ� */

/* �H�W���� thread �D�D���\Ū���\�� */

#define XZ_INDEX_BANMAIL      10        /* �׫H�C�� */
#define XZ_INDEX_OTHER        11        /* ��L�C�� */
#define XZ_INDEX_MYFAVORITE   12        /* �ڪ��̷R */

/* Count of zone indexes */

#define XZ_INDEX_MAX    XZ_INDEX_MYFAVORITE
#define XZ_COUNT        (XZ_INDEX_MAX + 1)

/* Legacy zone values */

#define XZ_CLASS        (XO_ZONE + XZ_INDEX_CLASS)
#define XZ_ULIST        (XO_ZONE + XZ_INDEX_ULIST)
#define XZ_PAL          (XO_ZONE + XZ_INDEX_PAL)
#define XZ_VOTE         (XO_ZONE + XZ_INDEX_VOTE)
#define XZ_BMW          (XO_ZONE + XZ_INDEX_BMW)

#ifdef HAVE_XYPOST
#define XZ_XPOST        (XO_ZONE + XZ_INDEX_XPOST)
#endif

#define XZ_MBOX         (XO_ZONE + XZ_INDEX_MBOX)
#define XZ_BOARD        (XO_ZONE + XZ_INDEX_BOARD)
#define XZ_GEM          (XO_ZONE + XZ_INDEX_GEM)
#define XZ_MAILGEM      (XO_ZONE + XZ_INDEX_MAILGEM)

#define XZ_BANMAIL      (XO_ZONE + XZ_INDEX_BANMAIL)
#define XZ_OTHER        (XO_ZONE + XZ_INDEX_OTHER)
#define XZ_MYFAVORITE   (XO_ZONE + XZ_INDEX_MYFAVORITE)

/* Zone aliases */
#define XZ_INDEX_POST   XZ_INDEX_BOARD
#define XZ_POST         XZ_BOARD


//cache, 20101119

#define PALTYPE_PAL     0               /* �B�ͦW�� */
#define PALTYPE_LIST    1               /* �s�զW�� */
#define PALTYPE_BPAL    2               /* �O�ͦW�� */
#define PALTYPE_VOTE    3               /* ����벼�W�� */

#endif                          /* MODES_H */
