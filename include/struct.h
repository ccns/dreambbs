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

/* screen control */

#define STRLEN          80             /* Length of most string data */
#define BTLEN           42             /* Length of board title */
#define BMLEN           36             /* Length of board managers */
#define TTLEN           72             /* Length of title */
#define FNLEN           28             /* Length of filename  */
#define IDLEN           12             /* Length of board / user id */
#define PASSLEN         14             /* Length of (salt-part) encrypted passwd field */
#define PLAINPASSLEN    37             /* Length of plaintext passwd field */
#define OLDPLAINPASSLEN 9              /* Length of old plaintext passwd field */
#define PASSHASHLEN     45             /* Length of hash-part encrypted passwd field */
#define BCLEN           4              /* Length of board class */
#define ANSILINELEN     500            /* Maximum Screen width in chars，不能超過 1023 */

#define T_LINES         50             /* maximum total lines */
#define T_COLS          120            /* maximum total columns，要比 ANSILINELEN 小 */
#define TAB_STOP        4              /* 按 TAB 換成幾格空白 (要是 2 的次方) */
#define TAB_WIDTH       (TAB_STOP - 1)

#define SCR_WIDTH       80
/* #define VE_WIDTH        (ANSILINELEN - 1) */
/* Thor.990330: 為防止引言後, ">"要變色, 一行會超過ANSILINELEN, 故多留空間 */
#define VE_WIDTH        (ANSILINELEN - 11)

#define BFLAG(n)        (1U << n)       /* 32 bit-wise flag */


typedef char const *STRING;
typedef struct UTMP UTMP;

/* ban email 轉信用 */

typedef struct
{
    char name[IDLEN+1];
} ADMIN;

typedef struct
{
    char name[IDLEN+1];
    char email[60];
} CONTACT;

typedef struct
{
    char date[8];
    char time[8];
    char work[50];
} MEMORANDUM;

typedef struct
{
    char    data[48];
    int     mode;
    time_t  time;
    int     usage;
} BANMAIL;

typedef struct
{
    char    data[48];
    int     mode;  time_t  time;
    int     usage;
    char    name[IDLEN+1];
} FW;

#define FW_OWNER        0x01
#define FW_TITLE        0x02
#define FW_TIME         0x04
#define FW_PATH         0x08
#define FW_ORIGIN       0x10
#define FW_CANCEL       0x20

#define FW_ALL          0xFF

/* ----------------------------------------------------- */
/* 使用者帳號 .ACCT struct : 512 bytes                   */
/* ----------------------------------------------------- */


typedef struct
{
    int userno;                 /* unique positive code */
    char userid[IDLEN + 1];     /* userid */
    char passwd[PASSLEN];       /* user password crypt by DES / salt for SHA-256 */
    unsigned char signature;    /* user signature number */
    char realname[20];          /* user realname */
    char username[24];          /* user nickname */
    unsigned int userlevel;     /* user perm */
    int numlogins;              /* user login times */
    int numposts;               /* user post times */
    unsigned int ufo;           /* user basic flags */
    time_t firstlogin;          /* user first login time */
    time_t lastlogin;           /* user last login time */
    time_t staytime;            /* user total stay time */
    time_t tcheck;              /* time to check mbox/pal */
    char lasthost[32];          /* user last login remote host */
    int numemail;               /* 原為寄發 Inetrnet E-mail 次數, 在不更改資料結構的狀況下, 擴充為積分 */
    time_t tvalid;              /* 通過認證、更改 mail address 的時間 */
    char email[60];             /* user email */
    char address[60];           /* user address */
    char justify[60];           /* FROM of replied justify mail */
    char vmail[60];             /* 通過認證之 email */
    time_t deny;                /* user violatelaw time */
    int request;                /* 點歌系統 */
    int money;                  /* 夢幣 */
    unsigned int ufo2;          /* 延伸的個人設定 */
    char passhash[66];          /* user password, encrypted using SHA-256 (extra bytes reserved for SHA-512) */
    char ident[30];             /* user remote host ident */
    int point1;                 /* 優良積分 */
    int point2;                 /* 劣文 */
    time_t vtime;               /* validate time */
} ACCT;

typedef struct                  /* 16 bytes */
{
    time_t uptime;
    char userid[IDLEN];
} SCHEMA;


#ifdef  HAVE_REGISTER_FORM

typedef struct  /* 註冊表單 (Register From) 256 bytes */
{
    int userno;
    time_t rtime;
    char userid[IDLEN + 1];
    char agent[IDLEN + 1];
    char realname[20];
    char career[50];
    char address[60];
    char phone[20];
    char reply[61];
    char idno[11];
} RFORM;

#ifndef HAVE_SIMPLE_RFORM
typedef struct
{
    int userno;
    char userid[IDLEN + 1];
    char msg[80];
} RFORM_R;
#endif
#endif  /* #ifdef  HAVE_REGISTER_FORM */


/* ----------------------------------------------------- */
/* User Flag Option : flags in ACCT.ufo                  */
/* ----------------------------------------------------- */


//#define UFO_COLOR       BFLAG(0)        /* true if the ANSI color mode open */
//#define UFO_MOVIE       BFLAG(1)        /* true if show movie */
//#define UFO_BRDNEW      BFLAG(2)        /* 新文章模式 */
//#define UFO_BNOTE       BFLAG(3)        /* 顯示進板畫面 */
//#define UFO_VEDIT       BFLAG(4)        /* 簡化編輯器 */

#define UFO_PAGER       BFLAG(5)        /* 關閉呼叫器 */
#define UFO_QUIET       BFLAG(6)        /* 結廬在人境，而無車馬喧 */

#define UFO_MAXMSG      BFLAG(7)        /* 訊息上限拒收訊息 */
#define UFO_FORWARD     BFLAG(8)        /* 自動轉寄 */

#define UFO_CLASSTABLE  BFLAG(9)        /* 功課表通知 */
//#define UFO_MIME        BFLAG(13)       /* MIME 解碼 */
#define UFO_BROADCAST   BFLAG(14)       /* 拒收廣播 */
//#define UFO_SIGN        BFLAG(15)       /* 簽名檔 */
//#define UFO_SHOWUSER    BFLAG(16)       /* 顯示 ID 和 暱稱 */

#define UFO_HIDDEN      BFLAG(18)       /* 隱藏來源 */
#define UFO_CLOAK       BFLAG(19)       /* true if cloak was ON */
//#define UFO_ACL         BFLAG(20)       /* true if ACL was ON */
#define UFO_NET         BFLAG(21)       /* visor.991030: 網路程式 */
#define UFO_WEB         BFLAG(22)       /* visor.020325: WEB */
#define UFO_MPAGER      BFLAG(10)       /* lkchu.990428: 電子郵件傳呼 */
//#define UFO_NWLOG       BFLAG(11)       /* lkchu.990510: 不存對話紀錄 */
//#define UFO_NTLOG       BFLAG(12)       /* lkchu.990510: 不存聊天紀錄 */
#define UFO_MESSAGE     BFLAG(23)       /* visor.991030: 訊息全關 */
#define UFO_PAGER1      BFLAG(26)       /* visor.991030: 呼叫器全關 */


/* ----------------------------------------------------- */
/* bit 24-27 : client/server or telnet BBS               */
/* ----------------------------------------------------- */

#define UFO_BIFFN       BFLAG(24)       /* 有新訊息 */
#define UFO_SPARE       BFLAG(25)       /* ready for connection */

/* these are flags in UTMP.ufo */

#define UFO_BIFF        BFLAG(27)       /* 有新信件 */
#define UFO_SOCKET      BFLAG(28)       /* true if socket port active */
#define UFO_REJECT      BFLAG(29)       /* true if reject any body */

/* special purpose */

#define UFO_FCACHE      BFLAG(30)       /* 有好友 */
#define UFO_MQUOTA      BFLAG(31)       /* 信箱中有待清理之信件 */

#define UFO_UTMP_MASK   (UFO_BIFF|UFO_BIFFN)
/* Thor.980805: 定義ufo中以utmp->ufo為本尊的flag, 解決與cuser.ufo不同步的問題 */

/* ----------------------------------------------------- */
/* User Flag Option Extend: flags in ACCT.ufo2           */
/* ----------------------------------------------------- */


#define UFO2_COLOR              BFLAG(0)        /* true if the ANSI color mode open */
#define UFO2_MOVIE              BFLAG(1)        /* true if show movie */
#define UFO2_BRDNEW             BFLAG(2)        /* 新文章模式 */
#define UFO2_BNOTE              BFLAG(3)        /* 顯示進板畫面 */
#define UFO2_VEDIT              BFLAG(4)        /* 簡化編輯器 */

#define UFO2_PAL                BFLAG(5)        /* true if show pals only */

#define UFO2_MOTD               BFLAG(6)        /* 簡化進站畫面 */
#define UFO2_MIME               BFLAG(7)        /* MIME 解碼 */
#define UFO2_SIGN               BFLAG(8)        /* 簽名檔 */
#define UFO2_SHOWUSER           BFLAG(9)        /* 顯示 ID 和 暱稱 */

#define UFO2_PRH                BFLAG(10)       /* 顯示推薦文章分數 */

#define UFO2_SHIP               BFLAG(11)       /* visor.991030: 好友描述 */
#define UFO2_NWLOG              BFLAG(12)       /* lkchu.990510: 不存對話紀錄 */
#define UFO2_NTLOG              BFLAG(13)       /* lkchu.990510: 不存聊天紀錄 */
#define UFO2_CIRCLE             BFLAG(14)       /* 循環閱讀 */
#define UFO2_ORIGUI             BFLAG(15)       /* 關閉風之塔超炫介面 */

#define UFO2_DEF_ANONY          BFLAG(16)       /* 預設不匿名 */
#define UFO2_DEF_LEAVE          BFLAG(17)       /* 預設不離站 */
#define UFO2_REPLY              BFLAG(18)       /* 記錄水球資訊 */
#define UFO2_DEF_LOCALMAIL      BFLAG(19)       /* 只收站內信 */

#define UFO2_ACL                BFLAG(24)       /* true if ACL was ON */
#define UFO2_REALNAME           BFLAG(28)       /* visor.991030: 真實姓名 */


#include "hdr.h"


/* ----------------------------------------------------- */
/* control of board vote : 256 bytes                     */
/* ----------------------------------------------------- */

#if 0
typedef struct VoteControlHeader
{
    time_t chrono;                /* 投票開辦時間 */      /* Thor: 為 key 而且 match HDR chrono */
    time_t bstamp;                /* 看板辨識代碼 */      /* Thor: 為 key */
    time_t vclose;                /* 投票結束時間 */

    char xname[32];               /* 主檔名 */            /* Thor: match HDR 的 xname */
    char date[9];                 /* 開始日期 */          /* Thor: match HDR 的 date */
    char cdate[9];                /* 結束日期 */          /* Thor: 只供顯示，不做比較 */
    char owner[IDLEN + 1];        /* 舉辦人 */
    char title[TTLEN + 1];        /* 投票主題 */
    char vgamble;                 /* 是否為賭盤        '$':賭盤  ' ':一般投票 */
    char vsort;                   /* 開票結果是否排序  's':排序  ' ':不排序 */
    char vpercent;                /* 是否顯示百分比例  '%':百分  ' ':一般 */
    char vprivate;                /* 是否為私人投票    ')':私人  ' ':公開 */
    int maxblt;                   /* 每人可投幾票 */
    int price;                    /* 每張賭票的售價 */

    int limitlogins;              /* 限制要登入超過幾次以上的使用者才能投票 */
    int limitposts;               /* 限制要發文超過幾次以上的使用者才能投票 */

    char nouse[88];
} VCH;
#endif  /* #if 0 */


typedef struct VoteControlHeader
{
    time_t chrono;              /* 投票開辦時間 */  /* Thor:為 key */
                                                    /* 而且 match HDR chrono */
    time_t bstamp;              /* 看板辨識代碼 */  /* Thor:為 key */
    time_t vclose;              /* 投票結束時間 */
    char xname[17];             /* 主檔名 */ /* Thor: match HDR的xname */
    char vsort;                 /* 開票結果是否排序 */
    char vpercent;              /* 是否顯示百分比例 */
    char cdate[9];              /* 結束日期 */ /* Thor.990329: 只供顯示, y2k */
    int maxblt;                 /* 每人可投幾票 */
    char owner[129];            /* 舉辦人 */
    char check;                 /* 學生投票系統 */
    char date[9];               /* 開始日期 */ /* Thor: match HDR的date*/
    char title[TTLEN + 1];      /* 投票主題 */
} VCH;


typedef struct VoteStudents
{
/*  元智的學號格式 (s) 99 9 999
    char grad[3];
    char major[2];
    char first[4];
    char last[4];
    char end;
*/
    /* 成大學號格式 B1 4 11 1111*/
    char inst[3];  /* 院系 institute:         A ~ Z + 0 ~ 9 */
    char level[2]; /* 部 level:               4, 7, 6, 8    */
    char admis[3]; /* 入學年份 admission year: 00 ~ 99      */
    char first[5]; /* 身分別及入學序 :        0000 ~ 9999   */
    char last[5];
    char end;      /* \n，存檔時要用的                      */
} VCHS;


typedef char vitem_t[32];       /* 投票選項 */


typedef struct
{
    char userid[IDLEN + 1];
    char numvotes;
    unsigned int choice;
} VLOG;


/* filepath : brd/<board>/.VCH, brd/<board>/@/... */


/* ----------------------------------------------------- */
/* Mail-Queue struct : 256 bytes                         */
/* ----------------------------------------------------- */


typedef struct
{
    time_t mailtime;            /* 寄信時間 */
    char method;
    char sender[IDLEN + 1];
    char username[24];
    char subject[TTLEN + 1];
    char rcpt[60];
    char filepath[77];
    char *revdomain;            /* reverse domain */
} MailQueue;


#define MQ_UUENCODE     0x01    /* 先 uuencode 再寄出 */
#define MQ_JUSTIFY      0x02    /* 身分認證信函 */
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
    char ftype;
    char ship[46];
    int userno;
} PAL;

#ifdef  HAVE_BANMSG
typedef struct
{
    char userid[IDLEN + 1];
    char ship[46];
    int userno;
} BANMSG;
#endif


/* 上站通知名單 */
typedef struct
{
    char userid[IDLEN+1];
    int userno;
} ALOHA;


#define PAL_BAD 0x02    /* 好友 vs 損友 */


/* ----------------------------------------------------- */
/* structure for call-in message : 100 bytes             */
/* ----------------------------------------------------- */


typedef struct
{
    time_t btime;
    UTMP *caller;               /* who call-in me ? */
    int sender;                 /* calling userno */
    int recver;                 /* called userno */
    char userid[IDLEN + 1];
    char msg[71];               /* 熱訊 */
} BMW;                     /* bbs message write */

#define BMW_MODE        0x01


/* ----------------------------------------------------- */
/* Structure used in UTMP file : 148 bytes               */
/* ----------------------------------------------------- */
#ifdef  HAVE_PIP_FIGHT1
typedef struct
{
    char name[20];
    int pid;
    int hp;                     /*體力*/
    int maxhp;                  /*體力上限*/
    int mp;                     /*法力*/
    int maxmp;                  /*法力上限*/
    int attack;                 /*攻擊*/
    int resist;                 /*防禦*/
    int speed;                  /*速度*/
    int mresist;                /*魔法防禦*/
    int resistmode;             /*防禦型態*/
    int nodone;                 /*完成*/
    int leaving;                /*離開*/
    int pipmode;                /*狀態*/
    int mode;                   /*幾代雞*/
    int msgcount;               /*訊息個數*/
    int chatcount;
    char msg[150];              /*訊息內容*/
    char chat[10][150];         /*聊天內容*/
} pipdata;

typedef struct
{
    pipdata pip1;
    pipdata pip2;
} PIPUTMP;
#endif  /* #ifdef  HAVE_PIP_FIGHT1 */

struct UTMP
{
    pid_t         pid;                          /* process ID */
    int           userno;                       /* user number in .PASSWDS */

    time_t        idle_time;                    /* active time for last event */
    unsigned int  mode;                         /* bbsmode */
    unsigned int  ufo;                          /* the same as userec.ufo */
    unsigned int  flag;                         /* user flag */
    unsigned long in_addr;                      /* Internet address */
    int           sockport;                     /* socket port for talk */
    UTMP          *talker;                      /* who talk-to me ? */

    BMW           *mslot[BMW_PER_USER];

    char          userid[IDLEN + 1];            /* user's ID */
    char          mateid[IDLEN + 1];            /* partner's ID */
    char          username[24];
    char          realname[20];
    unsigned int  userlevel;
    char          from[30];                     /* remote host */
#ifdef  HAVE_SHOWNUMMSG
    int           num_msg;                      /* receive messages */
#endif
    int           pal_max;                      /* friends max */
    int           pal_spool[PAL_MAX];           /* friends spool */
#ifdef  HAVE_BANMSG
    int           banmsg_max;                   /* banmsg max */
    int           banmsg_spool[BANMSG_MAX];     /* banmsg spool */
#endif
#ifdef HAVE_BOARD_PAL
    int           board_pal;
#endif
#ifdef  HAVE_PIP_FIGHT1
    pipdata       *pip;
#endif
#ifdef  PREFORK
    int           bgen;                         /* generation */
#endif

};


/* ----------------------------------------------------- */
/* BOARDS struct : 256 bytes                             */
/* ----------------------------------------------------- */

/* 090519.cache: 板主自訂看板發文權限*/
typedef struct
{
    int age;
    int numlogins;
    int numposts;
    int point2;
} THRESHOLD;

typedef struct BoardHeader
{
    char brdname[IDLEN + 1];    /* board ID */
    char title[BTLEN + 1];
    char color;
    char class_[5];
    char BM[BMLEN + 1];         /* BMs' uid, token '/' */

    unsigned char bvote;        /* 共有幾項投票舉行中 */

    time_t bstamp;              /* 建立看板的時間, unique */
    unsigned int readlevel;     /* 閱讀文章的權限 */
    unsigned int postlevel;     /* 發表文章的權限 */
    unsigned int battr;         /* 看板屬性 */
    time_t btime;               /* .DIR 的 st_mtime */
    int bpost;                  /* 共有幾篇 post */
    time_t blast;               /* 最後一篇 post 的時間 */
    unsigned int expiremax;     /* Expire Max Post */
    unsigned int expiremin;     /* Expire Min Post */
    unsigned int expireday;     /* Expire old Post */
    unsigned int n_reads;       /* 看板閱讀累計 times/hour */
    unsigned int n_posts;       /* 看板發表累計 times/hour */
    unsigned int n_news;        /* 看板轉信累計 times/hour */
    unsigned int n_bans;        /* 看板檔信累計 times/hour */
    char  reserve[100];         /* 保留未用 */
} BRD;

#define POST_STOP_PERM   0x01   /* 自動抓 crosspost 開關旗標 */

typedef struct NewBoardHeader
{
    char          brdname[IDLEN + 1];
    char          title[49];
    time_t        btime;
    time_t        etime;
    char          xname[32];
    char          owner[IDLEN +1];
    char          date[9];
    unsigned int  mode;
    unsigned int  total;
    unsigned int  agree;
    unsigned int  assist;
    char          reserve[64];
} NBRD;


#define NBRD_ING        0x01    /* 連署中 */
#define NBRD_OPEN       0x02    /* 已開版 */
#define NBRD_CLOSE      0x04    /* 未達連署人數 */
#define NBRD_STOP       0x08    /* 禁止開版 停止連署 */
#define NBRD_OK         0x10    /* 連署完成 */
#define NBRD_START      0x20    /* 申請通過 */
#define NBRD_REJECT     0x40    /* 申請不通過 */
#define NBRD_NBRD       0x1000  /* 新版 */
#define NBRD_CANCEL     0x2000  /* 廢版主 */
#define NBRD_OTHER      0x8000  /* 其它 */

#define NBRD_MASK       (NBRD_NBRD|NBRD_CANCEL|NBRD_OTHER)

/* ----------------------------------------------------- */
/* 看板閱讀記錄 .BRH (Board Reading History)             */
/* ----------------------------------------------------- */

typedef struct BoardReadingHistory
{
    time_t bstamp;              /* 建立看板的時間, unique */ /* Thor.brh_tail*/
    time_t bvisit;              /* 上次閱讀時間 */ /* Thor.980902:沒用到? */
                        /* Thor.980904:未讀時放上次讀的時間, 正讀時放 bhno */
    int bcount;                                    /* Thor.980902:沒用到? */
                                                   /* Thor.980902:給自己看的 */
    /* --------------------------------------------------- */
    /* time_t {final, begin} / {final | BRH_SIGN}          */
    /* --------------------------------------------------- */
                            /* Thor.980904:註解: BRH_SIGN代表final begin 相同 */
                            /* Thor.980904:註解: 由大到小排列, 存放已讀interval */
} BRH;


#define BRH_EXPIRE      180          /* Thor.980902:註解:保留多少天 */
#define BRH_MAX         200          /* Thor.980902:註解:每版最多有幾個標籤 */
#define BRH_PAGE        2048         /* Thor.980902:註解:每次多配量, 用不到了 */
#define BRH_MASK        0x7fffffff   /* Thor.980902:註解:最大量為2038年1月中*/
#define BRH_SIGN        0x80000000   /* Thor.980902:註解:zap及壓final專用 */
#define BRH_WINDOW      (sizeof(BRH) + sizeof(time_t) * BRH_MAX * 2)

/* ----------------------------------------------------- */
/* Class image                                           */
/* ----------------------------------------------------- */


#define CLASS_INIFILE   "Class"
#define CLASS_IMGFILE   "run/class.img"
#define PROFESS_INIFILE "Profess"
#define PROFESS_IMGFILE "run/profess.img"

#define CH_END          -1
#define CH_TTLEN        64


typedef struct
{
    int count;
    char title[CH_TTLEN];
    short chno[FLEX_SIZE];
} ClassHeader;
#define ClassHeader_FLEX_MEMBER    chno


/* ----------------------------------------------------- */
/* cache.c 中運用的資料結構                              */
/* ----------------------------------------------------- */


typedef struct
{
    int shot[MOVIE_MAX];        /* Thor.980805: 可能還要再加1, 因合理範圍為0..MOVIE_MAX */
    char film[MOVIE_SIZE];
} FCACHE;


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
    int count;                  /* number of active session */
    unsigned int offset;        /* offset for last active UTMP */

    double sysload[3];
    int avgload;

    BMW *mbase;                 /* sequential pointer for BMW */
    BMW mpool[BMW_MAX];
#ifdef  HAVE_PIP_FIGHT1
    PIPUTMP pip[PIP_MAX];
#endif

} UCACHE;


typedef struct
{
    BRD bcache[MAXBOARD];
    int mantime[MAXBOARD];      /* 各板目前正有多少人在閱讀 */
    int number;
    time_t uptime;
} BCACHE;

typedef struct
{
    BANMAIL fwcache[MAXFIREWALL];
    int number;
} FWCACHE;

typedef struct
{
    FW fwocache[MAXOFILEWALL];
    int mode;
} FWOCACHE;


typedef struct
{
    char verb[9];               /* 動詞 */
    char chinese[7];            /* 中文翻譯 */
    char part1_msg[60];         /* 介詞 */
    char part2_msg[60];         /* 動作 */
} ChatAction;


typedef struct
{
    ChatAction chat_speak[SPEAK_MAX+1];
    ChatAction chat_condition[CONDITION_MAX+1];
    ChatAction chat_party[PARTY_MAX+1];
    ChatAction chat_party2[PARTY_MAX+1];
    ChatAction chat_person[CONDITION_MAX+1];
} MUD;


/* ----------------------------------------------------- */
/* screen.c 中運用的資料結構                             */
/* ----------------------------------------------------- */

/* Screen Line buffer modes */


#define SL_MODIFIED     (1)     /* if line has been modified, screen output */
#define SL_STANDOUT     (2)     /* if this line contains standout code */
#define SL_ANSICODE     (4)     /* if this line contains ANSI code */


typedef struct screenline
{
    unsigned short oldlen;               /* previous line length */
    unsigned char len;                  /* current length of line */
    unsigned char width;                /* padding length of ANSI codes */
    unsigned char mode;                 /* status of line, as far as update */
    unsigned char smod;                 /* start of modified data */
    unsigned short emod;                 /* end of modified data */
    unsigned char sso;                  /* start of standout data */
    unsigned char eso;                  /* end of standout data */
    unsigned char data[ANSILINELEN];
} screenline;


typedef struct LinkList
{
    struct LinkList *next;
    char data[FLEX_SIZE];
} LinkList;
#define LinkList_FLEX_MEMBER    data


/* ----------------------------------------------------- */
/* xover.c 中運用的資料結構                              */
/* ----------------------------------------------------- */


typedef struct OverView
{
    int pos;                    /* current position */
    int top;                    /* top */
    int max;                    /* max */
    int key;                    /* key */
    char *xyz;                  /* staff */
    struct OverView *nxt;       /* next */
    char dir[FLEX_SIZE];        /* data path */
} XO;
#define OverView_FLEX_MEMBER    dir
#define XO_FLEX_MEMBER          dir


typedef struct
{
    unsigned int key;
    union {  /* IID.20191106: The field to be used is determined by the value of `key` */
        int (*func)(XO *xo);  /* Default */
#ifdef NO_SO
        int (*dlfunc)(XO *xo);  /* `key | XO_DL` */
#else
        const char *dlfunc;
#endif
    };
} KeyFunc;


typedef struct
{
    XO *xo;
    KeyFunc *cb;
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
} UMODELOG;


typedef struct
{
    time_t logtime;
    time_t used_time[30];
    int count[30];
    int usercount;
} MODELOG;
#endif

/* ----------------------------------------------------- */
/* acct.c 中運用的資料結構                               */
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
    int num;
} SAME_EMAIL;

typedef struct
{
    char email[56];
    int times;
    time_t deny;
} EMAIL;

/* ----------------------------------------------------- */
/* classtable.c 中運用的資料結構                         */
/* ----------------------------------------------------- */
typedef struct
{
    int lost;
    char name[9];
    char teacher[9];
    char class_[5];
    char obj_id[7];
}   CLASS;

typedef struct
{
    int hour;
    int min;
}  CLOCK;


typedef struct
{
    CLOCK start[13];
    CLOCK end[13];
} CLASS_TIME;

typedef struct
{
    int mode;
    CLASS table[6][13];
    CLASS_TIME time;
} CLASS_TABLE;

/* ----------------------------------------------------- */
/* list.c 中運用的資料結構                               */
/* ----------------------------------------------------- */
typedef struct
{
    char userid[IDLEN+1];
} LIST;

typedef struct
{
    char title[MAX_LIST][41];
} LIST_TITLE;

/* ----------------------------------------------------- */
/* counter.c 中運用的資料結構                            */
/* ----------------------------------------------------- */
typedef struct
{
    int hour_max_login;
    int day_max_login;
    int samehour_max_login;
    int max_regist;
    int cur_hour_max_login;
    int cur_day_max_login;
    time_t samehour_max_time;
    int max_regist_old;
    int samehour_max_login_old;
    char ident[90];
} COUNTER;

typedef struct
{
    time_t date;
    int mode;
    char userid[IDLEN + 1];
    char username[19];
    char buf[3][80];
} notedata;

typedef struct
{
    int start;
    int end;
} REAPER_TIME;

/* ----------------------------------------------------- */
/* 生日年齡星座統計 中運用的資料結構                     */
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
    int  valid;
    int  x;
    int  y;
} CLASS_TABLE2;

#ifdef  HAVE_CLASSTABLEALERT
typedef struct
{
    char condensation[9];
    char room[20];
    unsigned int used;
} CLASS_TABLE_ALERT_ITEM;

typedef struct
{
    CLASS_TABLE_ALERT_ITEM item[78];
    unsigned int userno;
} CLASS_TABLE_ALERT;

#endif
#endif  /*#ifdef  HAVE_CLASSTABLE  */

#ifdef  HAVE_OBSERVE_LIST
typedef struct
{
    char userid[16];
    unsigned int userno;
    char title[64];
    char reserve[44];
} OBSERVE;

typedef struct
{
    unsigned int userno[MAXOBSERVELIST];
    int total;
    int mode;
} OCACHE;

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

typedef struct MENU
{
    union {  /* The field to be used is determined by the value of `umode` */
        int (*func) (void);  /* Default (menu) or `POPUP_FUN` (popupmenu) */
        int (*xofunc) (XO *xo);  /* `POPUP_XO` (popupmenu) */
#ifdef NO_SO
        int (*dlfunc) (void);  /* `M_DL(umode)` (menu & popupmenu) or `POPUP_SO` (popupmenu) */
#else
        const char *dlfunc;
#endif
        const char *title;  /* `POPUP_MENUTITLE` (popupmenu) */
        struct MENU *menu;  /* `<= M_XMENU` (menu) or `POPUP_MENU` (popupmenu) */
    };
    unsigned int level;
    int umode;
    const char *desc;
} MENU;

#ifdef  HAVE_COUNT_BOARD
#define BSTAT_LAST      0x01
#define BSTAT_TOTAL     0x02

typedef struct
{
    time_t chrono;
    char  type[16];             /* 統計型態 */
    unsigned int n_reads;       /* 看板閱讀累計 times/hour */
    unsigned int n_posts;       /* 看板發表累計 times/hour */
    unsigned int n_news;        /* 看板轉信累計 times/hour */
    unsigned int n_bans;        /* 看板檔信累計 times/hour */
    char reserve[28];           /* 保留 */
} BSTAT;

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
} BSTATCOUNT;
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
    unsigned int state;
} PB;

#define PB_APPLY        0x01    /* 申請中 */
#define PB_OPEN         0x02    /* 已開版 */

/* ----------------------------------------------------- */
/* innbbsd 中運用的資料結構                              */
/* ----------------------------------------------------- */

typedef struct
{
    time_t chrono;              /* >=0:stamp -1:cancel */
    char board[IDLEN + 1];

    /* 以下欄位的大小與 HDR 相同 */
    char xname[32];
    char owner[47];
    char nick[50];
    char title[73];
} bntp_t;


#define INN_USEIHAVE    0x0001
#define INN_USEPOST     0x0002
#define INN_FEEDED      0x0010

typedef struct
{
    char name[13];              /* 該站的 short-name */
    char host[83];              /* 該站的 host */
    int port;                   /* 該站的 port */
    unsigned int xmode;         /* 該站的 xmode */
    char blank[20];             /* 保留 */
    int feedfd;                 /* bbslink.c 使用 */
} nodelist_t;

#define INN_NOINCOME    0x0001
#define INN_ERROR       0x0004

typedef struct
{
    char path[13];              /* 該群組所對轉的站台 */
    char newsgroup[83];         /* 該群組的名稱 */
    char board[IDLEN + 1];      /* 該群組所對應的看板 */
    char charset[11];           /* 該群組的字集 */
    unsigned int xmode;         /* 該群組的 xmode */
    int high;                   /* 目前抓到該群組的哪一篇 */
    } newsfeeds_t;


typedef struct
{
    char issuer[128];           /* NCM message 的發信人 */
    char type[64];              /* NCM message 的種類名稱 */
    int perm;                   /* 允許此 NCM message 刪信 (1:開 0:關) */
    char blank[60];             /* 保留 */
} ncmperm_t;


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
    char detail[80];            /* 該規則的 內容 */
    unsigned int xmode;         /* 該規則的 xmode */
    char board[IDLEN + 1];      /* 該規則適用的看板 */
    char path[13];              /* 該規則適用的站台 */
    char blank[18];             /* 保留 */
} spamrule_t;

/* ----------------------------------------------------- */
/* PAYCHECK : 32 bytes                                   */
/* ----------------------------------------------------- */

typedef struct
{
    time_t tissue;              /* 開支票時間 */
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
    unsigned int cb;            /* size of current structure */
    unsigned int encoding;
    unsigned int raddr_len;
    unsigned char raddr[16];
    unsigned short int rport;
    unsigned short int lport;
    unsigned int flags;
} conn_data_t;

#endif                          /* STRUCT_H */
