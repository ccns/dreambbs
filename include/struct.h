/*-------------------------------------------------------*/
/* struct.h     ( NTHU CS MapleBBS Ver 3.02 )            */
/*-------------------------------------------------------*/
/* target : all definitions about data structure         */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/

#ifndef STRUCT_H
#define STRUCT_H

#include "cppdef.h"
#include "attrdef.h"

#include "dns.h"

#ifdef __cplusplus
  #include <unordered_map>
  #include <utility>
#endif

/* String length and buffer size */

#define STRSIZE         80             /* Buffer size of most string data */
#define BTLEN           42             /* Length of board title */
#define BMLEN           36             /* Length of board managers */
#define TTLEN           72             /* Length of title */
#define FNLEN           28             /* Length of filename  */
#define IDLEN           12             /* Length of board / user id */
#define PASSSIZE        14             /* Buffer size of (salt-part) encrypted passwd field */
#define PLAINPASSSIZE   37             /* Buffer size of plaintext passwd field */
#define OLDPLAINPASSSIZE 9             /* Buffer size of old plaintext passwd field */
#define PASSHASHSIZE    45             /* Buffer size of hash-part encrypted passwd field */
#define BCLEN           4              /* Length of board class */
#define ANSILINESIZE    4000           /* Buffer size for holding screen line data�A����W�L 1023 */
                                       /* temperary expand ANSILINELEN to 4000 before
                                          inplementing dynamic allocating size */

/* Aliases for backward compatibility */

#define STRLEN          STRSIZE
#define PASSLEN         PASSSIZE
#define PLAINPASSLEN    PLAINPASSSIZE
#define OLDPLAINPASSLEN OLDPLAINPASSSIZE
#define PASSHASHLEN     PASSHASHSIZE
#define ANSILINELEN     ANSILINESIZE

/* screen control */

#define T_LINES         50             /* maximum total lines */
#define T_COLS          120            /* maximum total columns�A�n�� ANSILINESIZE �p */
#define TAB_STOP        4U             /* �� TAB �����X��ť� (��ĳ�O 2 ������A�i�K�h���k) */

#define SCR_WIDTH       80
/* #define VE_WIDTH        (ANSILINESIZE - 1) */
/* Thor.990330: ������ި���, ">"�n�ܦ�, �@��|�W�LANSILINELEN, �G�h�d�Ŷ� */
#define VE_WIDTH        (ANSILINESIZE - 11)

/* IID.20200113: For `get[y|x]_ref()` & `move_ref()` */
/* Use 2's power to prevent division */
#define T_LINES_REF  (T_LINES_DIV_RES * 2*T_LINES_OFF_MAX)
#define T_LINES_DIV_RES  128U  /* Divisor resolution */
#define T_LINES_OFF_MAX  512U  /* Maximum offset; >= T_LINES */
#define T_COLS_REF   (T_COLS_DIV_RES * 2*T_COLS_OFF_MAX)
#define T_COLS_DIV_RES   128U
#define T_COLS_OFF_MAX   512U  /* >= T_COLS */

#define B_LINES_REF  (T_LINES_REF - 1)  /* Mapped to `b_lines` */
#define P_LINES_REF  (B_LINES_REF - 5)  /* Mapped to `p_lines` */
#define B_COLS_REF   (T_COLS_REF - 1)   /* Mapped to `b_cols` */
#define D_COLS_REF   (B_COLS_REF - 79)  /* Mapped to `d_cols` */


typedef char const *STRING;
typedef struct UTMP UTMP;

/* ban email ��H�� */

typedef struct
{
    char name[IDLEN+1];
} ADMIN;  /* DISKDATA(raw) */

typedef struct
{
    char name[IDLEN+1];
    char email[60];
} CONTACT;  /* DISKDATA(raw) */

typedef struct
{
    char date[8];
    char time[8];
    char work[50];
} MEMORANDUM;  /* DISKDATA(raw) */

typedef struct
{
    char    data[48];
    int32_t mode;
    time_t  time;
    int32_t usage;
} BANMAIL;  /* DISKDATA(raw) */

typedef struct
{
    char    data[48];
    int32_t mode;
    time_t  time;
    int32_t usage;
    char    name[IDLEN+1];
} FW;  /* SHMDATA(raw); dependency */

#define FW_OWNER        0x01
#define FW_TITLE        0x02
#define FW_TIME         0x04
#define FW_PATH         0x08
#define FW_ORIGIN       0x10
#define FW_CANCEL       0x20

#define FW_ALL          0xFF

/* ----------------------------------------------------- */
/* �ϥΪ̱b�� .ACCT struct : 512 bytes                   */
/* ----------------------------------------------------- */


typedef struct
{
    int32_t userno;             /* unique positive code */
    char userid[IDLEN + 1];     /* userid */
    char passwd[PASSSIZE];      /* user password crypt by DES / salt for SHA-256 */
    uint8_t signature;          /* user signature number */
    char realname[20];          /* user realname */
    char username[24];          /* user nickname */
    uint32_t userlevel;         /* user perm */
    int32_t numlogins;          /* user login times */
    int32_t numposts;           /* user post times */
    uint32_t ufo;               /* user basic flags */
    time_t firstlogin;          /* user first login time */
    time_t lastlogin;           /* user last login time */
    time_t staytime;            /* user total stay time */
    time_t tcheck;              /* time to check mbox/pal */
    char lasthost[32];          /* user last login remote host */
    int32_t numemail;           /* �쬰�H�o Inetrnet E-mail ����, �b������Ƶ��c�����p�U, �X�R���n�� */
    time_t tvalid;              /* �q�L�{�ҡB��� mail address ���ɶ� */
    char email[60];             /* user email */
    char address[60];           /* user address */
    char justify[60];           /* FROM of replied justify mail */
    char vmail[60];             /* �q�L�{�Ҥ� email */
    time_t deny;                /* user violatelaw time */
    int32_t request;            /* �I�q�t�� */
    int32_t money;              /* �ڹ� */
    uint32_t ufo2;              /* �������ӤH�]�w */
    char passhash[66];          /* user password, encrypted using SHA-256 (extra bytes reserved for SHA-512) */
    GCC_DEPRECATED("see FN_USIES log file for ident info instead")
    char ident[30];             /* user remote host ident */
    int32_t point1;             /* �u�}�n�� */
    int32_t point2;             /* �H�� */
    time_t vtime;               /* validate time */
} ACCT;  /* DISKDATA(raw) */

typedef struct                  /* 16 bytes */
{
    time_t uptime;
    char userid[IDLEN] GCC_NONSTRING;
} SCHEMA;  /* DISKDATA(raw) */


#ifdef  HAVE_REGISTER_FORM

typedef struct  /* ���U��� (Register From) 256 bytes */
{
    int32_t userno;
    time_t rtime;
    char userid[IDLEN + 1];
    char agent[IDLEN + 1];
    char realname[20];
    char career[50];
    char address[60];
    char phone[20];
    char reply[61];
    char idno[11];
} RFORM;  /* DISKDATA(raw) */

#ifndef HAVE_SIMPLE_RFORM
typedef struct
{
    int32_t userno;
    char userid[IDLEN + 1];
    char msg[80];
} RFORM_R;  /* DISKDATA(raw) */
#endif
#endif  /* #ifdef  HAVE_REGISTER_FORM */


/* ----------------------------------------------------- */
/* User Flag Option : flags in ACCT.ufo                  */
/* ----------------------------------------------------- */

#define BFLAG(n)        (1U << n)       /* 32 bit-wise flag */

//#define UFO_COLOR       BFLAG(0)        /* true if the ANSI color mode open */
//#define UFO_MOVIE       BFLAG(1)        /* true if show movie */
//#define UFO_BRDNEW      BFLAG(2)        /* �s�峹�Ҧ� */
//#define UFO_BNOTE       BFLAG(3)        /* ��ܶi�O�e�� */
//#define UFO_VEDIT       BFLAG(4)        /* ²�ƽs�边 */

#define UFO_PAGER       BFLAG(5)        /* �����I�s�� */
#define UFO_QUIET       BFLAG(6)        /* ���f�b�H�ҡA�ӵL������ */

#define UFO_MAXMSG      BFLAG(7)        /* �T���W���ڦ��T�� */
#define UFO_FORWARD     BFLAG(8)        /* �۰���H */

#define UFO_CLASSTABLE  BFLAG(9)        /* �\�Ҫ�q�� */
//#define UFO_MIME        BFLAG(13)       /* MIME �ѽX */
#define UFO_BROADCAST   BFLAG(14)       /* �ڦ��s�� */
//#define UFO_SIGN        BFLAG(15)       /* ñ�W�� */
//#define UFO_SHOWUSER    BFLAG(16)       /* ��� ID �M �ʺ� */

#define UFO_HIDDEN      BFLAG(18)       /* ���èӷ� */
#define UFO_CLOAK       BFLAG(19)       /* true if cloak was ON */
//#define UFO_ACL         BFLAG(20)       /* true if ACL was ON */
#define UFO_NET         BFLAG(21)       /* visor.991030: �����{�� */
#define UFO_WEB         BFLAG(22)       /* visor.020325: WEB */
#define UFO_MPAGER      BFLAG(10)       /* lkchu.990428: �q�l�l��ǩI */
//#define UFO_NWLOG       BFLAG(11)       /* lkchu.990510: ���s��ܬ��� */
//#define UFO_NTLOG       BFLAG(12)       /* lkchu.990510: ���s��Ѭ��� */
#define UFO_MESSAGE     BFLAG(23)       /* visor.991030: �T������ */
#define UFO_PAGER1      BFLAG(26)       /* visor.991030: �I�s������ */


/* ----------------------------------------------------- */
/* bit 24-27 : client/server or telnet BBS               */
/* ----------------------------------------------------- */

#define UFO_BIFFN       BFLAG(24)       /* ���s�T�� */
#define UFO_SPARE       BFLAG(25)       /* ready for connection */

/* these are flags in UTMP.ufo */

#define UFO_BIFF        BFLAG(27)       /* ���s�H�� */
#define UFO_SOCKET      BFLAG(28)       /* true if socket port active */
#define UFO_REJECT      BFLAG(29)       /* true if reject any body */

/* special purpose */

#define UFO_FCACHE      BFLAG(30)       /* ���n�� */
#define UFO_MQUOTA      BFLAG(31)       /* �H�c�����ݲM�z���H�� */

#define UFO_UTMP_MASK   (UFO_BIFF|UFO_BIFFN)
/* Thor.980805: �w�qufo���Hutmp->ufo�����L��flag, �ѨM�Pcuser.ufo���P�B�����D */

/* ----------------------------------------------------- */
/* User Flag Option Extend: flags in ACCT.ufo2           */
/* ----------------------------------------------------- */


#define UFO2_COLOR              BFLAG(0)        /* true if the ANSI color mode open */
#define UFO2_MOVIE              BFLAG(1)        /* true if show movie */
#define UFO2_BRDNEW             BFLAG(2)        /* �s�峹�Ҧ� */
#define UFO2_BNOTE              BFLAG(3)        /* ��ܶi�O�e�� */
#define UFO2_VEDIT              BFLAG(4)        /* ²�ƽs�边 */

#define UFO2_PAL                BFLAG(5)        /* true if show pals only */

#define UFO2_MOTD               BFLAG(6)        /* ²�ƶi���e�� */
#define UFO2_MIME               BFLAG(7)        /* MIME �ѽX */
#define UFO2_SIGN               BFLAG(8)        /* ñ�W�� */
#define UFO2_SHOWUSER           BFLAG(9)        /* ��� ID �M �ʺ� */

#define UFO2_PRH                BFLAG(10)       /* ��ܱ��ˤ峹���� */

#define UFO2_SHIP               BFLAG(11)       /* visor.991030: �n�ʹy�z */
#define UFO2_NWLOG              BFLAG(12)       /* lkchu.990510: ���s��ܬ��� */
#define UFO2_NTLOG              BFLAG(13)       /* lkchu.990510: ���s��Ѭ��� */
#define UFO2_CIRCLE             BFLAG(14)       /* �`���\Ū */
#define UFO2_ORIGUI             BFLAG(15)       /* ����������W������ */

#define UFO2_DEF_ANONY          BFLAG(16)       /* �w�]���ΦW */
#define UFO2_DEF_LEAVE          BFLAG(17)       /* �w�]������ */
#define UFO2_REPLY              BFLAG(18)       /* �O�����y��T */
#define UFO2_DEF_LOCALMAIL      BFLAG(19)       /* �u�������H */

#define UFO2_MENU_LIGHTBAR      BFLAG(20)       /* IID.20191223: �ϥΥ��ο��t�� */

#define UFO2_ACL                BFLAG(24)       /* true if ACL was ON */
#define UFO2_REALNAME           BFLAG(28)       /* visor.991030: �u��m�W */


/* IID.20191224: Macros for checking config-dependent ufo2 flags */

#define HAVE_UFO2_CONF(ufo2_flag)  IF_ON(CONF_ ## ufo2_flag, ((bool)(cuser.ufo2 & ufo2_flag)), CONF_DEFAULT_ ## ufo2_flag)

#define CONF_UFO2_MIME  HAVE_MIME_TRANSFER
#define CONF_DEFAULT_UFO2_MIME  false
#define CONF_UFO2_PRH  HAVE_RECOMMEND
#define CONF_DEFAULT_UFO2_PRH  true
#define CONF_UFO2_MENU_LIGHTBAR  HAVE_MENU_LIGHTBAR
#define CONF_DEFAULT_UFO2_MENU_LIGHTBAR  false


#include "hdr.h"


/* ----------------------------------------------------- */
/* control of board vote : 256 bytes                     */
/* ----------------------------------------------------- */

#if 0
typedef struct VoteControlHeader
{
    time_t chrono;                /* �벼�}��ɶ� */      /* Thor: �� key �ӥB match HDR chrono */
    time_t bstamp;                /* �ݪO���ѥN�X */      /* Thor: �� key */
    time_t vclose;                /* �벼�����ɶ� */

    char xname[32];               /* �D�ɦW */            /* Thor: match HDR �� xname */
    char date[9];                 /* �}�l��� */          /* Thor: match HDR �� date */
    char cdate[9];                /* ������� */          /* Thor: �u����ܡA������� */
    char owner[IDLEN + 1];        /* �|��H */
    char title[TTLEN + 1];        /* �벼�D�D */
    char vgamble;                 /* �O�_����L        '$':��L  ' ':�@��벼 */
    char vsort;                   /* �}�����G�O�_�Ƨ�  's':�Ƨ�  ' ':���Ƨ� */
    char vpercent;                /* �O�_��ܦʤ����  '%':�ʤ�  ' ':�@�� */
    char vprivate;                /* �O�_���p�H�벼    ')':�p�H  ' ':���} */
    int maxblt;                   /* �C�H�i��X�� */
    int price;                    /* �C�i�䲼����� */

    int limitlogins;              /* ����n�n�J�W�L�X���H�W���ϥΪ̤~��벼 */
    int limitposts;               /* ����n�o��W�L�X���H�W���ϥΪ̤~��벼 */

    char nouse[88];
} VCH;
#endif  /* #if 0 */


typedef struct VoteControlHeader
{
    time_t chrono;              /* �벼�}��ɶ� */  /* Thor:�� key */
                                                    /* �ӥB match HDR chrono */
    time_t bstamp;              /* �ݪO���ѥN�X */  /* Thor:�� key */
    time_t vclose;              /* �벼�����ɶ� */
    char xname[17];             /* �D�ɦW */ /* Thor: match HDR��xname */
    char vsort;                 /* �}�����G�O�_�Ƨ� */
    char vpercent;              /* �O�_��ܦʤ���� */
    char cdate[9];              /* ������� */ /* Thor.990329: �u�����, y2k */
    int32_t maxblt;             /* �C�H�i��X�� */
    char owner[129];            /* �|��H */
    int8_t check;               /* �ǥͧ벼�t�� */
    char date[9];               /* �}�l��� */ /* Thor: match HDR��date*/
    char title[TTLEN + 1];      /* �벼�D�D */
} VCH;  /* DISKDATA(raw) */


typedef struct VoteStudents
{
/*  �������Ǹ��榡 (s) 99 9 999
    char grad[3];
    char major[2];
    char first[4];
    char last[4];
    char end;
*/
    /* ���j�Ǹ��榡 B1 4 11 1111*/
    char inst[3];  /* �|�t institute:         A ~ Z + 0 ~ 9 */
    char level[2]; /* �� level:               4, 7, 6, 8    */
    char admis[3]; /* �J�Ǧ~�� admission year: 00 ~ 99      */
    char first[5]; /* �����O�ΤJ�ǧ� :        0000 ~ 9999   */
    char last[5];
    char end;      /* \n�A�s�ɮɭn�Ϊ�                      */
} VCHS;  /* DISKDATA(raw) */


typedef char vitem_t[32];       /* �벼�ﶵ */ /* DISKDATA(raw) */


typedef struct
{
    char userid[IDLEN + 1];
    int8_t numvotes;
    uint32_t choice;
} VLOG;


/* filepath : brd/<board>/.VCH, brd/<board>/@/... */


/* ----------------------------------------------------- */
/* Mail-Queue struct : 256 bytes                         */
/* ----------------------------------------------------- */


typedef struct
{
    time_t mailtime;            /* �H�H�ɶ� */
    int8_t method;
    char sender[IDLEN + 1];
    char username[24];
    char subject[TTLEN + 1];
    char rcpt[60];
    char filepath[77];
    char *revdomain;            /* reverse domain */
} MailQueue;  /* DISKDATA(raw) */


#define MQ_UUENCODE     0x01    /* �� uuencode �A�H�X */
#define MQ_JUSTIFY      0x02    /* �����{�ҫH�� */
#define MQ_MIME         0x04    /* MIME */

#define CHK_MAIL_NOMSG  0x01
#define CHK_MAIL_NORMAL 0x02
#define CHK_MAIL_VALID  0x04


/* ----------------------------------------------------- */
/* PAL : friend struct : 64 bytes                        */
/* ----------------------------------------------------- */


typedef struct
{
    char userid[IDLEN + 1];
    int8_t ftype;
    char ship[46];
    int32_t userno;
} PAL;  /* DISKDATA(raw) */

#ifdef  HAVE_BANMSG
typedef struct
{
    char userid[IDLEN + 1];
    char ship[46];
    int32_t userno;
} BANMSG;  /* DISKDATA(raw) */
#endif


/* �W���q���W�� */
typedef struct
{
    char userid[IDLEN+1];
    int32_t userno;
} ALOHA;  /* DISKDATA(raw) */


#define PAL_BAD 0x02    /* �n�� vs �l�� */


/* ----------------------------------------------------- */
/* structure for call-in message : 100 bytes             */
/* ----------------------------------------------------- */


typedef struct
{
    time_t btime;
    UTMP *caller;               /* who call-in me ? */
    int32_t sender;             /* calling userno */
    int32_t recver;             /* called userno */
    char userid[IDLEN + 1];
    char msg[71];               /* ���T */
} BMW;                     /* bbs message write */ /* DISKDATA(raw) */

#define BMW_MODE        0x01


/* ----------------------------------------------------- */
/* Structure used in UTMP file : 148 bytes               */
/* ----------------------------------------------------- */
#ifdef  HAVE_PIP_FIGHT1
typedef struct
{
    char name[20];
    int32_t pid;
    int32_t hp;                 /*��O*/
    int32_t maxhp;              /*��O�W��*/
    int32_t mp;                 /*�k�O*/
    int32_t maxmp;              /*�k�O�W��*/
    int32_t attack;             /*����*/
    int32_t resist;             /*���m*/
    int32_t speed;              /*�t��*/
    int32_t mresist;            /*�]�k���m*/
    int32_t resistmode;         /*���m���A*/
    int32_t nodone;             /*����*/
    int32_t leaving;            /*���}*/
    int32_t pipmode;            /*���A*/
    int32_t mode;               /*�X�N��*/
    int32_t msgcount;           /*�T���Ӽ�*/
    int32_t chatcount;
    char msg[150];              /*�T�����e*/
    char chat[10][150];         /*��Ѥ��e*/
} pipdata;  /* DISKDATA(raw) */

typedef struct
{
    pipdata pip1;
    pipdata pip2;
} PIPUTMP;  /* SHMDATA(raw); dependency */
#endif  /* #ifdef  HAVE_PIP_FIGHT1 */

struct UTMP  /* SHMDATA(raw) */
{
    pid_t         pid;                          /* process ID */
    int32_t       userno;                       /* user number in .PASSWDS */

    time_t        idle_time;                    /* active time for last event */
    uint32_t      mode;                         /* bbsmode */
    uint32_t      ufo;                          /* the same as userec.ufo */
    uint32_t      flag;                         /* user flag */
    ip_addr       in_addr;                      /* Internet address */
    int32_t       sockport;                     /* socket port for talk */
    UTMP          *talker;                      /* who talk-to me ? */

    BMW           *mslot[BMW_PER_USER];

    char          userid[IDLEN + 1];            /* user's ID */
    char          mateid[IDLEN + 1];            /* partner's ID */
    char          username[24];
    char          realname[20];
    uint32_t      userlevel;
    char          from[48];                     /* remote host */
#ifdef  HAVE_SHOWNUMMSG
    int32_t       num_msg;                      /* receive messages */
#endif
    int32_t       pal_max;                      /* friends max */
    int32_t       pal_spool[PAL_MAX];           /* friends spool */
#ifdef  HAVE_BANMSG
    int32_t       banmsg_max;                   /* banmsg max */
    int32_t       banmsg_spool[BANMSG_MAX];     /* banmsg spool */
#endif
#ifdef HAVE_BOARD_PAL
    int32_t       board_pal;
#endif
#ifdef  HAVE_PIP_FIGHT1
    pipdata       *pip;
#endif
#ifdef  PREFORK
    int32_t       bgen;                         /* generation */
#endif

};


/* ----------------------------------------------------- */
/* BOARDS struct : 256 bytes                             */
/* ----------------------------------------------------- */

/* 090519.cache: �O�D�ۭq�ݪO�o���v��*/
typedef struct
{
    int32_t age;
    int32_t numlogins;
    int32_t numposts;
    int32_t point2;
} THRESHOLD;  /* DISKDATA(raw) */

typedef struct BoardHeader
{
    char brdname[IDLEN + 1];    /* board ID */
    char title[BTLEN + 1];
    int8_t color;
    char class_[5];
    char BM[BMLEN + 1];         /* BMs' uid, token '/' */

    uint8_t bvote;              /* �@���X���벼�|�椤 */

    time_t bstamp;              /* �إ߬ݪO���ɶ�, unique */
    uint32_t readlevel;         /* �\Ū�峹���v�� */
    uint32_t postlevel;         /* �o��峹���v�� */
    uint32_t battr;             /* �ݪO�ݩ� */
    time_t btime;               /* .DIR �� st_mtime */
    int32_t bpost;              /* �@���X�g post */
    time_t blast;               /* �̫�@�g post ���ɶ� */
    uint32_t expiremax;         /* Expire Max Post */
    uint32_t expiremin;         /* Expire Min Post */
    uint32_t expireday;         /* Expire old Post */
    uint32_t n_reads;           /* �ݪO�\Ū�֭p times/hour */
    uint32_t n_posts;           /* �ݪO�o��֭p times/hour */
    uint32_t n_news;            /* �ݪO��H�֭p times/hour */
    uint32_t n_bans;            /* �ݪO�ɫH�֭p times/hour */
    char  reserve[100];         /* �O�d���� */
} BRD;  /* DISKDATA(raw) */

#define POST_STOP_PERM   0x01   /* �۰ʧ� crosspost �}���X�� */

typedef struct NewBoardHeader
{
    char          brdname[IDLEN + 1];
    char          title[49];
    time_t        btime;
    time_t        etime;
    char          xname[32];
    char          owner[IDLEN +1];
    char          date[9];
    uint32_t      mode;
    uint32_t      total;
    uint32_t      agree;
    uint32_t      assist;
    char          reserve[64];
} NBRD;  /* DISKDATA(raw) */


#define NBRD_ING        0x01    /* �s�p�� */
#define NBRD_OPEN       0x02    /* �w�}�� */
#define NBRD_CLOSE      0x04    /* ���F�s�p�H�� */
#define NBRD_STOP       0x08    /* �T��}�� ����s�p */
#define NBRD_OK         0x10    /* �s�p���� */
#define NBRD_START      0x20    /* �ӽгq�L */
#define NBRD_REJECT     0x40    /* �ӽФ��q�L */
#define NBRD_NBRD       0x1000  /* �s�� */
#define NBRD_CANCEL     0x2000  /* �o���D */
#define NBRD_OTHER      0x8000  /* �䥦 */

#define NBRD_MASK       (NBRD_NBRD|NBRD_CANCEL|NBRD_OTHER)

/* ----------------------------------------------------- */
/* �ݪO�\Ū�O�� .BRH (Board Reading History)             */
/* ----------------------------------------------------- */

typedef struct BoardReadingHistory
{
    time_t bstamp;              /* �إ߬ݪO���ɶ�, unique */ /* Thor.brh_tail*/
    time_t bvisit;              /* �W���\Ū�ɶ� */ /* Thor.980902:�S�Ψ�? */
                        /* Thor.980904:��Ū�ɩ�W��Ū���ɶ�, ��Ū�ɩ� bhno */
    int32_t bcount;                                /* Thor.980902:�S�Ψ�? */
                                                   /* Thor.980902:���ۤv�ݪ� */
    /* --------------------------------------------------- */
    /* time_t {final, begin} / {final | BRH_SIGN}          */
    /* --------------------------------------------------- */
                            /* Thor.980904:����: BRH_SIGN�N��final begin �ۦP */
                            /* Thor.980904:����: �Ѥj��p�ƦC, �s��wŪinterval */
} BRH;  /* DISKDATA(raw) */


#define BRH_EXPIRE      180          /* Thor.980902:����:�O�d�h�֤� */
#define BRH_MAX         200          /* Thor.980902:����:�C���̦h���X�Ӽ��� */
#define BRH_PAGE        2048         /* Thor.980902:����:�C���h�t�q, �Τ���F */
#define BRH_MASK        0x7fffffff   /* Thor.980902:����:�̤j�q��2038�~1�뤤*/
#define BRH_SIGN        0x80000000   /* Thor.980902:����:zap����final�M�� */
#define BRH_WINDOW      (sizeof(BRH) + sizeof(time_t) * BRH_MAX)

/* ----------------------------------------------------- */
/* Class image                                           */
/* ----------------------------------------------------- */


#define CLASS_INIFILE   "Class"
#define CLASS_IMGFILE   "run/class.img"
#define PROFESS_INIFILE "Profess"
#define PROFESS_IMGFILE "run/profess.img"

#define CH_END          -1
#define CH_TTSIZE       64

/* Aliases for backward compatibility */
#define CH_TTLEN        CH_TTSIZE

typedef struct
{
    int32_t count;
    char title[CH_TTSIZE];
    int16_t chno[FLEX_SIZE];
} ClassHeader;  /* DISKDATA(raw); runtime */
#define ClassHeader_FLEX_MEMBER    chno


/* ----------------------------------------------------- */
/* cache.c ���B�Ϊ���Ƶ��c                              */
/* ----------------------------------------------------- */


typedef struct
{
    int32_t shot[MOVIE_MAX];    /* Thor.980805: �i���٭n�A�[1, �]�X�z�d��0..MOVIE_MAX */
    char film[MOVIE_SIZE];
} FCACHE;  /* SHMDATA(raw) */


#define FILM_SIZ        4000    /* max size for each film */


#define FILM_WELCOME    (0)
#define FILM_GOODBYE    (1)
#define FILM_APPLY      (2)     /* new account */
#define FILM_TRYOUT     (3)
#define FILM_POST       (4)
#define FILM_GEM        (5)     /* help message */
#define FILM_BOARD      (6)
#define FILM_CLASS      (7)
#define FILM_PAL        (8)
#define FILM_MAIL       (9)
#define FILM_ULIST      (10)
#define FILM_VOTE       (11)
#define FILM_MORE       (12)
#define FILM_EDIT       (13)
#define FILM_BMW        (14)
#define FILM_BANMAIL    (15)
#define FILM_INCOME     (16)
#define FILM_ADMIN      (17)
#define FILM_SONG       (18)
#define FILM_MIME       (19)
#define FILM_CONTACT    (20)
#define FILM_MEMORANDUM (21)
#define FILM_ALOHA      (22)
#define FILM_SIGNUP     (23)
#define FILM_FAVORITE   (24)
#define FILM_MOVIE      (25)    /* normal movies */


#define FILM_ROW        (40)

typedef struct
{
    UTMP uslot[MAXACTIVE];      /* UTMP slots */
    int32_t count;              /* number of active session */
    uint32_t offset;            /* offset for last active UTMP */

    double sysload[3];
    int32_t avgload;

    BMW *mbase;                 /* sequential pointer for BMW */
    BMW mpool[BMW_MAX];
#ifdef  HAVE_PIP_FIGHT1
    PIPUTMP pip[PIP_MAX];
#endif

} UCACHE;  /* SHMDATA(raw) */


typedef struct
{
    BRD bcache[MAXBOARD];
    int32_t mantime[MAXBOARD];  /* �U�O�ثe�����h�֤H�b�\Ū */
    int32_t number;
    time_t uptime;
} BCACHE;  /* SHMDATA(raw) */

typedef struct
{
    BANMAIL fwcache[MAXFIREWALL];
    int32_t number;
} FWCACHE;  /* SHMDATA(raw) */

typedef struct
{
    FW fwocache[MAXOFILEWALL];
    int32_t mode;
} FWOCACHE;  /* SHMDATA(raw) */


typedef struct
{
    char verb[9];               /* �ʵ� */
    char brief_desc[7];         /* ����½Ķ */
    char part1_msg[60];         /* ���� */
    char part2_msg[60];         /* �ʧ@ */
} ChatAction;  /* DISKDATA(raw) */


typedef struct
{
    ChatAction chat_speak[SPEAK_MAX+1];
    ChatAction chat_condition[CONDITION_MAX+1];
    ChatAction chat_party[PARTY_MAX+1];
    ChatAction chat_party2[PARTY_MAX+1];
    ChatAction chat_person[CONDITION_MAX+1];
} MUD;  /* SHMDATA(raw) */


/* ----------------------------------------------------- */
/* screen.c ���B�Ϊ���Ƶ��c                             */
/* ----------------------------------------------------- */

/* Screen Line buffer modes */


#define SL_MODIFIED     (1)     /* if line has been modified, screen output */
#define SL_STANDOUT     (2)     /* if this line contains standout code */
#define SL_ANSICODE     (4)     /* if this line contains ANSI code */


typedef struct screenline
{
    uint16_t oldlen;                    /* previous line length */
    uint8_t len;                        /* current length of line */
    uint8_t width;                      /* padding length of ANSI codes */
    uint8_t mode;                       /* status of line, as far as update */
    uint8_t smod;                       /* start of modified data */
    uint16_t emod;                      /* end of modified data */
    uint8_t sso;                        /* start of standout data */
    uint8_t eso;                        /* end of standout data */
    unsigned char data[ANSILINESIZE];
} screenline;


typedef struct LinkList
{
    struct LinkList *next;
    char data[FLEX_SIZE];
} LinkList;
#define LinkList_FLEX_MEMBER    data


/* ----------------------------------------------------- */
/* xover.c ���B�Ϊ���Ƶ��c                              */
/* ----------------------------------------------------- */


typedef struct OverView XO;

typedef union {  /* IID.20191106: The field to be used is determined by the value of `key` */
    int (*func)(XO *xo);  /* Default */
#if NO_SO
    int (*dlfunc)(XO *xo);  /* `key | XO_DL` */
#else
    const char *dlfunc;
#endif
} XoFunc;

/* XXX(IID.20191227): Workaround for g++ not currently supporting
 *    function overload resolution for designated initializer */
#if defined __cplusplus && defined __clang__
/* IID.20191225: Use hash table for xover callback function list */

#define HAVE_HASH_KEYFUNCLIST

#define PAIR_T(first_type, second_type)  \
    std::pair<first_type, second_type>
template <class PairT>
using UnorderedMapPair = std::unordered_map<typename PairT::first_type, typename PairT::second_type>;

typedef PAIR_T(unsigned int, XoFunc) KeyFunc;
typedef UnorderedMapPair<KeyFunc> KeyFuncList;
typedef KeyFuncList::iterator KeyFuncIter;
struct KeyFuncListRef {
    KeyFuncList *ptr_;

    constexpr KeyFuncListRef(KeyFuncList *ptr = NULL): ptr_ (ptr) { }
    CXX_CONSTEXPR_RELAXED KeyFuncList *&operator=(KeyFuncList *ptr) { return ptr_ = ptr; }
    constexpr KeyFuncListRef(KeyFuncList& ref): ptr_ (&ref) { }
    CXX_CONSTEXPR_RELAXED KeyFuncList *&operator=(KeyFuncList& ref) { return ptr_ = &ref; }
    constexpr KeyFuncList *operator->(void) const { return ptr_; }
};

#else

#define PAIR_T(first_type, second_type)  \
    struct { \
        first_type first; \
        second_type second; \
    }

typedef PAIR_T(unsigned int, XoFunc) KeyFunc;
typedef KeyFunc KeyFuncList[];
typedef KeyFunc *KeyFuncIter;
typedef KeyFunc *KeyFuncListRef;
#endif


typedef struct OverView
{
    int pos;                    /* current position */
    int top;                    /* top */
    int max;                    /* max */
    int key;                    /* key */
    void *xyz;                  /* staff */
    struct OverView *nxt;       /* next */
    KeyFuncListRef cb;          /* Callback functions */
    int recsiz;                 /* Record size */
    char dir[FLEX_SIZE];        /* data path */
} XO;
#define OverView_FLEX_MEMBER    dir
#define XO_FLEX_MEMBER          dir


typedef struct
{
    XO *xo;
    int mode;
} XZ;


typedef struct
{
    time_t chrono;
    int recno;
} TagItem;


#ifdef MODE_STAT
typedef struct
{
    time_t logtime;
    time_t used_time[30];
} UMODELOG;  /* DISKDATA(raw) */


typedef struct
{
    time_t logtime;
    time_t used_time[30];
    int32_t count[30];
    int32_t usercount;
} MODELOG;  /* DISKDATA(raw) */
#endif

/* ----------------------------------------------------- */
/* acct.c ���B�Ϊ���Ƶ��c                               */
/* ----------------------------------------------------- */
#define DENY_SEL_TALK   0x00000001
#define DENY_SEL_POST   0x00000002
#define DENY_SEL_MAIL   0x00000004
#define DENY_SEL_AD     0x00000008
#define DENY_SEL_SELL   0x00000010
#define DENY_SEL_OK     0x00000020
#define DENY_SEL        (DENY_SEL_TALK|DENY_SEL_POST|DENY_SEL_MAIL|DENY_SEL_AD|DENY_SEL_SELL)

#define DENY_DAYS_1     0x00010000
#define DENY_DAYS_2     0x00020000
#define DENY_DAYS_3     0x00040000
#define DENY_DAYS_4     0x00080000
#define DENY_DAYS_5     0x00100000
#define DENY_DAYS       (DENY_DAYS_1|DENY_DAYS_2|DENY_DAYS_3|DENY_DAYS_4|DENY_DAYS_5)

#define DENY_MODE_TALK  0x01000000
#define DENY_MODE_MAIL  0x02000000
#define DENY_MODE_POST  0x04000000
#define DENY_MODE_GUEST 0x08000000
#define DENY_MODE_NICK  0x10000000
#define DENY_MODE_ALL   (DENY_MODE_TALK|DENY_MODE_MAIL|DENY_MODE_POST|DENY_MODE_NICK)


typedef struct
{
    char email[60];
    int32_t num;
} SAME_EMAIL;  /* DISKDATA(raw) */

typedef struct
{
    char email[56];
    int32_t times;
    time_t deny;
} EMAIL;  /* DISKDATA(raw) */

/* ----------------------------------------------------- */
/* classtable.c ���B�Ϊ���Ƶ��c                         */
/* ----------------------------------------------------- */
typedef struct
{
    int32_t lost;
    char name[9];
    char teacher[9];
    char class_[5];
    char obj_id[7];
}   CLASS;

typedef struct
{
    int32_t hour;
    int32_t min;
}  CLOCK;


typedef struct
{
    CLOCK start[13];
    CLOCK end[13];
} CLASS_TIME;

typedef struct
{
    int32_t mode;
    CLASS table[6][13];
    CLASS_TIME time;
} CLASS_TABLE;

/* ----------------------------------------------------- */
/* list.c ���B�Ϊ���Ƶ��c                               */
/* ----------------------------------------------------- */
typedef struct
{
    char userid[IDLEN+1];
} LIST;  /* DISKDATA(raw) */

typedef struct
{
    char title[MAX_LIST][41];
} LIST_TITLE;  /* DISKDATA(raw) */

/* ----------------------------------------------------- */
/* counter.c ���B�Ϊ���Ƶ��c                            */
/* ----------------------------------------------------- */
typedef struct
{
    int32_t hour_max_login;
    int32_t day_max_login;
    int32_t samehour_max_login;
    int32_t max_regist;
    int32_t cur_hour_max_login;
    int32_t cur_day_max_login;
    time_t samehour_max_time;
    int32_t max_regist_old;
    int32_t samehour_max_login_old;
    char ident[90];
} COUNTER;  /* SHMDATA(raw) */

typedef struct
{
    time_t date;
    int32_t mode;
    char userid[IDLEN + 1];
    char username[19];
    char buf[3][80];
} notedata;  /* DISKDATA(raw) */

typedef struct
{
    int start;
    int end;
} REAPER_TIME;

/* ----------------------------------------------------- */
/* �ͤ�~�֬P�y�έp ���B�Ϊ���Ƶ��c                     */
/* ----------------------------------------------------- */

typedef struct
{
    char *grad;
    char *server;
} YZU_MAIL;

typedef struct
{
    char name[20];
    char host[26];
    char ip[16];
    char port[6];
} BBSNET;


#define TOPLOGINS       (0)
#define TOPPOSTS        (1)

#define TOPNUM          (50)
#define TOPNUM_HALF     (TOPNUM/2)

typedef struct
{
    char userid[IDLEN + 1];
    char username[24];
    int num;
} DATA;

#ifdef  HAVE_CLASSTABLE
typedef struct
{
    char condensation[9];
    char name[30];
    char teacher[11];
    char other[30];
    char room[20];
    int32_t valid;
    int32_t x;
    int32_t y;
} CLASS_TABLE2;  /* DISKDATA(raw) */

#ifdef  HAVE_CLASSTABLEALERT
typedef struct
{
    char condensation[9];
    char room[20];
    uint32_t used;
} CLASS_TABLE_ALERT_ITEM;  /* DISKDATA(raw); dependency */

typedef struct
{
    CLASS_TABLE_ALERT_ITEM item[78];
    uint32_t userno;
} CLASS_TABLE_ALERT;  /* DISKDATA(raw) */

#endif
#endif  /*#ifdef  HAVE_CLASSTABLE  */

#ifdef  HAVE_OBSERVE_LIST
typedef struct
{
    char userid[16];
    uint32_t userno;
    char title[64];
    char reserve[44];
} OBSERVE;  /* DISKDATA(raw) */

typedef struct
{
    uint32_t userno[MAXOBSERVELIST];
    int32_t total;
    int32_t mode;
} OCACHE;  /* SHMDATA(raw) */

#endif

#if 0
#ifdef  HAVE_RECOMMEND
typedef struct PostRecommendHistory
{
    time_t chrono;
    time_t bstamp;
} PRH;
#endif
#endif  /* #if 0 */

typedef struct {
    union {
        int (*func)(const void *arg);
        int (*xofunc)(XO *xo, const void *arg);
    };
    const void *arg;
} FuncArg;

typedef struct {
    union {
#if NO_SO
        int (*func)(const void *arg);
        int (*xofunc)(XO *xo, const void *arg);
#else
        const char *func;
        const char *xofunc;
#endif
    };
    const void *arg;
} DlFuncArg;

typedef union {
    /* The field to be used is determined by the value of `umode` */
#if NO_SO
    int (*func) (void);  /* `M_DL(umode)`, `umode` `>= M_FUN` or `M_IDLE` */
    int (*xofunc) (XO *xo);  /* `M_DL(umode | M_XO)` */
    const char *title;  /* `M_DL(umode | M_MENUTITLE)` */
    struct MENU *menu;  /* `M_DL(umode)`, `umode` `>= M_MENU` and `< M_FUN` */
#else
    const char *func;
    const char *xofunc;
    const char *title;
    const char *menu;
#endif
} DlMenuItem;

typedef union {
    /* The field to be used is determined by the value of `umode` */
    int (*func) (void);  /* `>= M_FUN` or `M_IDLE` */
    int (*xofunc) (XO *xo);  /* `umode | M_XO` */
    FuncArg funcarg;  /* `umode | M_ARG` */

    DlMenuItem dl;  /* `M_DL(umode)` */
    DlFuncArg dlfuncarg;  /* `M_DL(umode) | M_ARG` */

    const char *title;  /* `umode | M_MENUTITLE` */
    struct MENU *menu;  /* `>= M_MENU` and `< M_FUN` */
} MenuItem;

typedef struct MENU
{
    MenuItem item;
    uint32_t level;
    uint32_t umode;
    const char *desc;
} MENU;

#ifdef  HAVE_COUNT_BOARD
#define BSTAT_LAST      0x01
#define BSTAT_TOTAL     0x02

typedef struct
{
    time_t chrono;
    char  type[16];             /* �έp���A */
    uint32_t n_reads;           /* �ݪO�\Ū�֭p times/hour */
    uint32_t n_posts;           /* �ݪO�o��֭p times/hour */
    uint32_t n_news;            /* �ݪO��H�֭p times/hour */
    uint32_t n_bans;            /* �ݪO�ɫH�֭p times/hour */
    char reserve[28];           /* �O�d */
} BSTAT;  /* DISKDATA(raw) */

typedef struct
{
    BSTAT hour;
    BSTAT day;
    BSTAT week;
    BSTAT twoweek;
    BSTAT month;
    BSTAT threemonth;
    BSTAT halfyear;
    BSTAT year;
    BSTAT lhour[24];
    BSTAT lday[24];
    BSTAT lweek[24];
    BSTAT lmonth[24];
} BSTATCOUNT;  /* DISKDATA(raw) */
#endif  /* #ifdef  HAVE_COUNT_BOARD */


#ifdef  HAVE_DETECT_CROSSPOST
typedef struct
{
    int sum;
    int total;
} CHECKSUM;

typedef struct
{
    CHECKSUM checksum[5];
    int post_modetype;
    int checknum;
} CHECKSUMCOUNT;

#endif

typedef struct PersonalBoard
{
    char userid[IDLEN + 1];
    char email[60];
    char brdname[IDLEN + 1];
    char brdtitle[BTLEN + 1];
    uint32_t state;
} PB;  /* DISKDATA(raw) */

#define PB_APPLY        0x01    /* �ӽФ� */
#define PB_OPEN         0x02    /* �w�}�� */

/* ----------------------------------------------------- */
/* innbbsd ���B�Ϊ���Ƶ��c                              */
/* ----------------------------------------------------- */

typedef struct
{
    time_t chrono;              /* >=0:stamp -1:cancel */
    char board[IDLEN + 1];

    /* �H�U��쪺�j�p�P HDR �ۦP */
    char xname[32];
    char owner[47];
    char nick[50];
    char title[73];
} bntp_t;  /* DISKDATA(raw) */


#define INN_USEIHAVE    0x0001
#define INN_USEPOST     0x0002
#define INN_FEEDED      0x0010

typedef struct
{
    char name[13];              /* �ӯ��� short-name */
    char host[83];              /* �ӯ��� host */
    int32_t port;               /* �ӯ��� port */
    uint32_t xmode;             /* �ӯ��� xmode */
    char blank[20];             /* �O�d */
    int32_t feedfd;             /* bbslink.c �ϥ� */
} nodelist_t;  /* DISKDATA(raw) */

#define INN_NOINCOME    0x0001
#define INN_ERROR       0x0004

typedef struct
{
    char path[13];              /* �Ӹs�թҹ��઺���x */
    char newsgroup[83];         /* �Ӹs�ժ��W�� */
    char board[IDLEN + 1];      /* �Ӹs�թҹ������ݪO */
    char charset[11];           /* �Ӹs�ժ��r�� */
    uint32_t xmode;             /* �Ӹs�ժ� xmode */
    int32_t high;               /* �ثe���Ӹs�ժ����@�g */
} newsfeeds_t;  /* DISKDATA(raw) */


typedef struct
{
    char issuer[128];           /* NCM message ���o�H�H */
    char type[64];              /* NCM message �������W�� */
    int32_t perm;               /* ���\�� NCM message �R�H (1:�} 0:��) */
    char blank[60];             /* �O�d */
} ncmperm_t;  /* DISKDATA(raw) */


#define INN_SPAMADDR            0x0001
#define INN_SPAMNICK            0x0002
#define INN_SPAMSUBJECT         0x0010
#define INN_SPAMPATH            0x0020
#define INN_SPAMMSGID           0x0040
#define INN_SPAMBODY            0x0100
#define INN_SPAMSITE            0x0200
#define INN_SPAMPOSTHOST        0x0400

typedef struct
{
    char detail[80];            /* �ӳW�h�� ���e */
    uint32_t xmode;             /* �ӳW�h�� xmode */
    char board[IDLEN + 1];      /* �ӳW�h�A�Ϊ��ݪO */
    char path[13];              /* �ӳW�h�A�Ϊ����x */
    char blank[18];             /* �O�d */
} spamrule_t;  /* DISKDATA(raw) */

/* ----------------------------------------------------- */
/* PAYCHECK : 32 bytes                                   */
/* ----------------------------------------------------- */

typedef struct
{
    time_t tissue;              /* �}�䲼�ɶ� */
    int money;
    int gold;
    char reason[20];
} PAYCHECK;

/* ----------------------------------------------------- */
/* Data structure for screen backup                      */
/* ----------------------------------------------------- */

#ifdef M3_USE_PFTERM
typedef struct {
    int row, col;
    int y, x;
    void *raw_memory;
} screen_backup_t, footer_backup_t;
#else
typedef struct {
    int old_t_lines;
    int old_roll;
    screenline *slp;
} screen_backup_t;
typedef screenline footer_backup_t[2];
#endif

/* ----------------------------------------------------- */
/* Data structure for passing connection data            */
/* ----------------------------------------------------- */

typedef struct
{
    uint32_t cb;                /* size of current structure */
    uint32_t encoding;
    uint32_t raddr_len;
    uint8_t raddr[16];
    uint16_t rport;
    uint16_t lport;
    uint32_t flags;
} conn_data_t;

#endif                          /* STRUCT_H */
