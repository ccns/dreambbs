/*-------------------------------------------------------*/
/* struct.h	( NTHU CS MapleBBS Ver 2.36 )		 */
/*-------------------------------------------------------*/
/* target : all definitions about data structure	 */
/* create : 95/03/29				 	 */
/* update : 95/12/15				 	 */
/*-------------------------------------------------------*/


#ifndef _STRUCT_H_
#define _STRUCT_H_


#define STRLEN   80		/* Length of most string data */
#define BTLEN    42		/* Length of board title */
#define BMLEN    36		/* Length of board managers */
#define TTLEN    72		/* Length of title */
#define FNLEN    28		/* Length of filename  */
#define IDLEN	 12		/* Length of board / user id */
#define PASSLEN  14		/* Length of encrypted passwd field */
#define BCLEN    4       /* Length of board class */

#define T_LINES		50	/* maximum total lines */
#define T_COLS		120	/* maximum total columns�A�n�� ANSILINELEN �p */
#define TAB_STOP	4	/* �� TAB �����X��ť� (�n�O 2 ������) */
#define TAB_WIDTH	(TAB_STOP - 1)

#define b_cols   79
#define d_cols   (b_cols - 79)

#define	SCR_WIDTH	80
/* #define	VE_WIDTH	(ANSILINELEN - 1) */
/* Thor.990330: ������ި���, ">"�n�ܦ�, �@��|�W�LANSILINELEN, �G�h�d�Ŷ� */
#define	VE_WIDTH	(ANSILINELEN - 11)

#define	BFLAG(n)	(1 << n)/* 32 bit-wise flag */


typedef char const *STRING;
typedef unsigned char uschar;	/* length = 1 */
typedef unsigned int usint;	/* length = 4 */
typedef struct UTMP UTMP;

/* ban email ��H�� */

typedef struct
{
  char name[IDLEN+1];
}	ADMIN;

typedef struct
{
  char name[IDLEN+1];
  char email[60];
}	CONTACT;

typedef struct
{
  char date[8];
  char time[8];
  char work[50];
}	MEMORANDUM;

typedef struct 
{
  char	  data[48];
  int 	  mode;
  time_t  time;
  int     usage;
}	BANMAIL;

typedef struct
{
  char    data[48];
  int     mode;  time_t  time;
  int     usage;
  char    name[IDLEN+1];
}	FW;

#define	FW_OWNER	0x01
#define	FW_TITLE	0x02
#define	FW_TIME		0x04
#define	FW_PATH		0x08
#define	FW_ORIGIN	0x10
#define	FW_CANCEL	0x20

#define	FW_ALL		0xFF

/* ----------------------------------------------------- */
/* �ϥΪ̱b�� .ACCT struct : 512 bytes			 */
/* ----------------------------------------------------- */


typedef struct
{
  int userno;			/* unique positive code */
  char userid[IDLEN + 1];	/* userid */
  char passwd[PASSLEN];		/* user password crypt by DES */
  uschar signature;		/* user signature number */
  char realname[20];		/* user realname */
  char username[24];		/* user nickname */
  usint userlevel;		/* user perm */
  int numlogins;		/* user login times */
  int numposts;			/* user post times */
  usint ufo;			/* user basic flags */
  time_t firstlogin;		/* user first login time */
  time_t lastlogin;		/* user last login time */
  time_t staytime;		/* user total stay time */
  time_t tcheck;		/* time to check mbox/pal */
  char lasthost[32];		/* user last login remote host */
  int numemail;			/* �쬰�H�o Inetrnet E-mail ����, �b������Ƶ��c�����p�U, �X�R���n�� */
  time_t tvalid;		/* �q�L�{�ҡB��� mail address ���ɶ� */
  char email[60];		/* user email */
  char address[60];		/* user address */
  char justify[60];		/* FROM of replied justify mail */
  char vmail[60];		/* �q�L�{�Ҥ� email */
  time_t deny;			/* user violatelaw time */
  int request;			/* �I�q�t�� */
  int money;             /* �ڹ� */
  usint ufo2;			/* �������ӤH�]�w */
  char ident[96];		/* user remote host ident */
  int point1;           /* �u�}�n�� */
  int point2;           /* �H�� */
  time_t vtime;			/* validate time */
}      ACCT;

typedef struct			/* 16 bytes */
{
  time_t uptime;
  char userid[IDLEN];
}      SCHEMA;


#ifdef	HAVE_REGISTER_FORM

typedef struct	/* ���U��� (Register From) 256 bytes */
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
}      RFORM;

#ifndef	HAVE_SIMPLE_RFORM
typedef struct
{
  int userno;
  char userid[IDLEN + 1];
  char msg[80];
}	RFORM_R;
#endif
#endif


/* ----------------------------------------------------- */
/* User Flag Option : flags in ACCT.ufo			 */
/* ----------------------------------------------------- */


//#define	UFO_COLOR	BFLAG(0)	/* true if the ANSI color mode open */
//#define	UFO_MOVIE	BFLAG(1)	/* true if show movie */
//#define	UFO_BRDNEW	BFLAG(2)	/* �s�峹�Ҧ� */
//#define	UFO_BNOTE	BFLAG(3)	/* ��ܶi�O�e�� */
//#define	UFO_VEDIT	BFLAG(4)	/* ²�ƽs�边 */

#define UFO_PAGER	BFLAG(5)	/* �����I�s�� */
#define	UFO_QUIET	BFLAG(6)	/* ���f�b�H�ҡA�ӵL������ */

#define	UFO_MAXMSG	BFLAG(7)	/* �T���W���ڦ��T�� */
#define	UFO_FORWARD	BFLAG(8)	/* �۰���H */

#define	UFO_CLASSTABLE	BFLAG(9)	/* �\�Ҫ�q�� */
//#define	UFO_MIME	BFLAG(13)	/* MIME �ѽX */
#define	UFO_BROADCAST	BFLAG(14)	/* �ڦ��s�� */
//#define	UFO_SIGN	BFLAG(15)	/* ñ�W�� */
//#define	UFO_SHOWUSER	BFLAG(16)	/* ��� ID �M �ʺ� */

#define UFO_HIDEDN	BFLAG(18)	/* ���èӷ� */
#define UFO_CLOAK	BFLAG(19)	/* true if cloak was ON */
//#define	UFO_ACL		BFLAG(20)	/* true if ACL was ON */
#define	UFO_NET    BFLAG(21)	/* visor.991030: �����{�� */ 
#define	UFO_WEB		BFLAG(22)	/* visor.020325: WEB */
#define	UFO_MPAGER	BFLAG(10)	/* lkchu.990428: �q�l�l��ǩI */
//#define	UFO_NWLOG	BFLAG(11)	/* lkchu.990510: ���s��ܬ��� */
//#define	UFO_NTLOG	BFLAG(12)	/* lkchu.990510: ���s��Ѭ��� */
#define UFO_MESSAGE	BFLAG(23)	/* visor.991030: �T������ */	
#define UFO_PAGER1	BFLAG(26)	/* visor.991030: �I�s������ */


/* ----------------------------------------------------- */
/* bit 24-27 : client/server or telnet BBS		 */
/* ----------------------------------------------------- */

#define	UFO_BIFFN	BFLAG(24)	/* ���s�T�� */
#define	UFO_SPARE	BFLAG(25)	/* ready for connection */

/* these are flags in UTMP.ufo */

#define	UFO_BIFF	BFLAG(27)	/* ���s�H�� */
#define	UFO_SOCKET	BFLAG(28)	/* true if socket port active */
#define	UFO_REJECT	BFLAG(29)	/* true if reject any body */

/* special purpose */

#define	UFO_FCACHE	BFLAG(30)	/* ���n�� */
#define	UFO_MQUOTA	BFLAG(31)	/* �H�c�����ݲM�z���H�� */

#define UFO_UTMP_MASK	(UFO_BIFF|UFO_BIFFN)	
/* Thor.980805: �w�qufo���Hutmp->ufo�����L��flag, �ѨM�Pcuser.ufo���P�B�����D */

/* ----------------------------------------------------- */
/* User Flag Option Extend: flags in ACCT.ufo2		 */
/* ----------------------------------------------------- */


#define	UFO2_COLOR	BFLAG(0)	/* true if the ANSI color mode open */
#define	UFO2_MOVIE	BFLAG(1)	/* true if show movie */
#define	UFO2_BRDNEW	BFLAG(2)	/* �s�峹�Ҧ� */
#define UFO2_BNOTE	BFLAG(3)	/* ��ܶi�O�e�� */
#define UFO2_VEDIT	BFLAG(4)	/* ²�ƽs�边 */

#define UFO2_PAL	BFLAG(5)	/* true if show pals only */

#define	UFO2_MOTD	BFLAG(6)	/* ²�ƶi���e�� */
#define UFO2_MIME	BFLAG(7)	/* MIME �ѽX */
#define UFO2_SIGN	BFLAG(8)	/* ñ�W�� */
#define UFO2_SHOWUSER	BFLAG(9)	/* ��� ID �M �ʺ� */

#define UFO2_PRH	BFLAG(10)	/* ��ܱ��ˤ峹���� */

#define UFO2_SHIP	BFLAG(11)	/* visor.991030: �n�ʹy�z */
#define	UFO2_NWLOG	BFLAG(12)	/* lkchu.990510: ���s��ܬ��� */
#define UFO2_NTLOG	BFLAG(13)	/* lkchu.990510: ���s��Ѭ��� */
#define	UFO2_CIRCLE	BFLAG(14)	/* �`���\Ū */
#define	UFO2_ORIGUI	BFLAG(15)	/* ����������W������ */

#define	UFO2_DEF_ANONY	BFLAG(16)	/* �w�]���ΦW */
#define	UFO2_DEF_LEAVE	BFLAG(17)	/* �w�]������ */
#define	UFO2_REPLY	BFLAG(18)	    /* �O�����y��T */
#define UFO2_DEF_LOCALMAIL	BFLAG(19)	/* �u�������H */

#define	UFO2_ACL	BFLAG(24)	/* true if ACL was ON */
#define UFO2_REALNAME	BFLAG(28)	/* visor.991030: �u��m�W */ 


#include "hdr.h"


/* ----------------------------------------------------- */
/* control of board vote : 256 bytes			 */
/* ----------------------------------------------------- */

#if 0
typedef struct VoteControlHeader
{
	time_t chrono;                /* ��????????}???????????? */      /* Thor: ??�X key ??????B match HDR chrono */
	time_t bstamp;                /* ??????O??????????N??X */      /* Thor: ??�X key */
	time_t vclose;                /* ��??????????��?????????? */

	char xname[32];               /* ??D??????W */            /* Thor: match HDR ???? xname */
	char date[9];                 /* ??}??l???????? */          /* Thor: match HDR ???? date */
	char cdate[9];                /* ????��?????????? */          /* Thor: ??u??????????????A????�X?????????? */
	char owner[IDLEN + 1];        /* ??|??????H */
	char title[TTLEN + 1];        /* ��????????D??D */
	char vgamble;                 /* ??O��_??�X??????L        '$':??????L  ' ':??@��??��?????? */
	char vsort;                   /* ??}??????????G??O��_��??��??  's':��??��??  ' ':????��??��?? */
	char vpercent;                /* ??O��_????????????????????????  '%':????????  ' ':??@��?? */
	char vprivate;                /* ??O��_??�X??p??H��??????    ')':??p??H  ' ':??????} */
	int maxblt;                   /* ??C??H??i��????X???? */
	int price;                    /* ??C��i????????????�X?????? */

	int limitlogins;              /* ??????????n??n??J??W??L??X??????H??W??????????????????~��??��?????? */
	int limitposts;               /* ??????????n??o??????W??L??X??????H??W??????????????????~��??��?????? */

	char nouse[88];
}      VCH;
#endif


typedef struct VoteControlHeader
{
  time_t chrono;		/* �벼�}��ɶ� */  /* Thor:�� key */
 						/* �ӥB match HDR chrono */
  time_t bstamp;		/* �ݪO���ѥN�X */  /* Thor:�� key */
  time_t vclose;		/* �벼�����ɶ� */
  char xname[17];		/* �D�ɦW */ /* Thor: match HDR��xname */
  char vsort;			/* �}�����G�O�_�Ƨ� */
  char vpercent;		/* �O�_��ܦʤ���� */
  char cdate[9];		/* ������� */ /* Thor.990329: �u�����, y2k */
  int maxblt;			/* �C�H�i��X�� */
  char owner[129];		/* �|��H */
  char check;			/* �ǥͧ벼�t�� */
  char date[9];			/* �}�l��� */ /* Thor: match HDR��date*/
  char title[TTLEN + 1];	/* �벼�D�D */
}           VCH;


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
}           VCHS;


typedef char vitem_t[32];	/* �벼�ﶵ */


typedef struct
{
	  char userid[IDLEN + 1];
	    char numvotes;             
		  usint choice;
}      VLOG;


/* filepath : brd/<board>/.VCH, brd/<board>/@/... */


/* ----------------------------------------------------- */
/* Mail-Queue struct : 256 bytes			 */
/* ----------------------------------------------------- */


typedef struct
{
  time_t mailtime;		/* �H�H�ɶ� */
  char method;
  char sender[IDLEN + 1];
  char username[24];
  char subject[TTLEN + 1];
  char rcpt[60];
  char filepath[77];
  char *niamod;			/* reverse domain */
}      MailQueue;


#define	MQ_UUENCODE	0x01	/* �� uuencode �A�H�X */
#define	MQ_JUSTIFY	0x02	/* �����{�ҫH�� */
#define MQ_MIME		0x04	/* MIME */

#define	CHK_MAIL_NOMSG	0x01
#define	CHK_MAIL_NORMAL	0x02
#define	CHK_MAIL_VALID	0x04


/* ----------------------------------------------------- */
/* PAL : friend struct : 64 bytes			 */
/* ----------------------------------------------------- */


typedef struct
{
  char userid[IDLEN + 1];
  char ftype;
  char ship[46];
  int userno;
}      PAL;

#ifdef	HAVE_BANMSG
typedef struct
{
  char userid[IDLEN + 1];
  char ship[46];
  int userno;
}      BANMSG;
#endif


/* �W���q���W�� */
typedef struct
{
  char userid[IDLEN+1];
  int userno;
}       ALOHA;


#define	PAL_BAD	0x02	/* �n�� vs �l�� */


/* ----------------------------------------------------- */
/* structure for call-in message : 100 bytes		 */
/* ----------------------------------------------------- */


typedef struct
{
  time_t btime;
  UTMP *caller;			/* who call-in me ? */
  int sender;			/* calling userno */
  int recver;			/* called userno */
  char userid[IDLEN + 1];
  char msg[71];			/* ���T */
}      BMW;			/* bbs message write */

#define	BMW_MODE	0x01


/* ----------------------------------------------------- */
/* Structure used in UTMP file : 148 bytes		 */
/* ----------------------------------------------------- */
#ifdef  HAVE_PIP_FIGHT1
typedef struct
{
        char name[20];
	int pid;
        int hp;         /*��O*/
        int maxhp;      /*��O�W��*/
        int mp;         /*�k�O*/
        int maxmp;      /*�k�O�W��*/
        int attack;     /*����*/
        int resist;     /*���m*/
        int speed;      /*�t��*/
        int mresist;    /*�]�k���m*/
        int resistmore; /*���m���A*/
        int nodone;     /*����*/
        int leaving;    /*���}*/
        int pipmode;    /*���A*/
        int mode;       /*�X�N��*/
        int msgcount;   /*�T���Ӽ�*/
        int chatcount;
        char msg[150];  /*�T�����e*/
        char chat[10][150]; /*��Ѥ��e*/
}       pipdata; 

typedef struct
{
	pipdata pip1;
	pipdata pip2;
}	PIPUTMP;
#endif

struct UTMP
{
  pid_t pid;			/* process ID */
  int userno;			/* user number in .PASSWDS */

  time_t idle_time;		/* active time for last event */
  usint  mode;			/* bbsmode */
  usint  ufo;			/* the same as userec.ufo */
  usint  flag;			/* user flag */
  u_long in_addr;		/* Internet address */
  int   sockport;		/* socket port for talk */
  UTMP  *talker;		/* who talk-to me ? */

  BMW   *mslot[BMW_PER_USER];

  char  userid[IDLEN + 1];	/* user's ID */
  char  mateid[IDLEN + 1];	/* partner's ID */
  char  username[24];
  char  realname[20];
  usint userlevel;
  char  from[30];		/* remote host */
#ifdef	HAVE_SHOWNUMMSG  
  int   num_msg;		/* receive messages */
#endif
  int   pal_max;		/* friends max */
  int   pal_spool[PAL_MAX];	/* friends spool */
#ifdef	HAVE_BANMSG
  int   banmsg_max;		/* banmsg max */
  int   banmsg_spool[BANMSG_MAX];	/* banmsg spool */
#endif
#ifdef HAVE_BOARD_PAL
  int   board_pal;
#endif
#ifdef  HAVE_PIP_FIGHT1
  pipdata *pip;
#endif
#ifdef	PREFORK
  int bgen;			/* generation */
#endif

};


/* ----------------------------------------------------- */
/* BOARDS struct : 256 bytes				 */
/* ----------------------------------------------------- */

/* 090519.cache: �O�D�ۭq�ݪO�o���v��*/
typedef struct
{
  int age;
  int numlogins;
  int numposts;
  int point2;  
}    THRESHOLD;

typedef struct BoardHeader
{
  char brdname[IDLEN + 1];	/* board ID */
  char title[BTLEN + 1];
  char color;
  char class[5];
  char BM[BMLEN + 1];		/* BMs' uid, token '/' */

  uschar bvote;			/* �@���X���벼�|�椤 */

  time_t bstamp;		/* �إ߬ݪO���ɶ�, unique */
  usint readlevel;		/* �\Ū�峹���v�� */
  usint postlevel;		/* �o��峹���v�� */
  usint battr;			/* �ݪO�ݩ� */
  time_t btime;			/* .DIR �� st_mtime */
  int bpost;			/* �@���X�g post */
  time_t blast;			/* �̫�@�g post ���ɶ� */
  usint expiremax;		/* Expire Max Post */
  usint expiremin;		/* Expire Min Post */
  usint expireday;		/* Expire old Post */
  usint n_reads;		/* �ݪO�\Ū�֭p times/hour */
  usint n_posts;		/* �ݪO�o��֭p times/hour */
  usint n_news;			/* �ݪO��H�֭p times/hour */
  usint n_bans;			/* �ݪO�ɫH�֭p times/hour */
  char  reserve[100];		/* �O�d���� */
}           BRD;

#define POST_STOP_PERM   0x01    /* �۰ʧ� crosspost �}���X�� */

typedef struct NewBoardHeader
{
  char brdname[IDLEN + 1];
  char title[49];
  time_t btime;
  time_t etime;
  char xname[32];
  char owner[IDLEN +1];
  char date[9];
  usint mode;
  usint total;
  usint	agree;
  usint	assist;
  char	reserve[64];
}       NBRD;


#define NBRD_ING	0x01	/* �s�p�� */
#define NBRD_OPEN	0x02	/* �w�}�� */
#define NBRD_CLOSE	0x04	/* ���F�s�p�H�� */
#define NBRD_STOP	0x08	/* �T��}�� ����s�p */
#define NBRD_OK		0x10	/* �s�p���� */
#define NBRD_START	0x20	/* �ӽгq�L */
#define NBRD_REJECT	0x40	/* �ӽФ��q�L */
#define NBRD_NBRD	0x1000	/* �s�� */
#define NBRD_CANCEL	0x2000	/* �o���D */
#define	NBRD_OTHER	0x8000	/* �䥦 */

#define NBRD_MASK	(NBRD_NBRD|NBRD_CANCEL|NBRD_OTHER)

/* ----------------------------------------------------- */
/* Class image						 */
/* ----------------------------------------------------- */


#define CLASS_INIFILE   "Class"
#define CLASS_IMGFILE	"run/class.img"
#define PROFESS_INIFILE	"Profess"
#define PROFESS_IMGFILE	"run/profess.img"

#define	CH_END	-1
#define	CH_TTLEN	64


typedef	struct
{
  int count;
  char title[CH_TTLEN];
  short chno[0];
} ClassHeader;


/* ----------------------------------------------------- */
/* cache.c ���B�Ϊ���Ƶ��c				 */
/* ----------------------------------------------------- */


typedef struct
{
  int shot[MOVIE_MAX]; /* Thor.980805: �i���٭n�A�[1,�]�X�z�d��0..MOVIE_MAX */
  char film[MOVIE_SIZE];
} FCACHE;


#define	FILM_SIZ	4000	/* max size for each film */


#define	FILM_WELCOME	(0)
#define	FILM_GOODBYE	(1)
#define	FILM_APPLY	(2)	/* new account */
#define	FILM_TRYOUT	(3)
#define	FILM_POST	(4)
#define	FILM_GEM	(5)	/* help message */
#define	FILM_BOARD	(6)
#define	FILM_CLASS	(7)
#define	FILM_PAL	(8)
#define	FILM_MAIL	(9)
#define	FILM_ULIST	(10)
#define	FILM_VOTE	(11)
#define	FILM_MORE	(12)
#define	FILM_EDIT	(13)
#define FILM_BMW        (14)
#define FILM_BANMAIL	(15)
#define FILM_INCOME	(16)
#define FILM_ADMIN	(17)
#define FILM_SONG	(18)
#define FILM_MIME	(19)
#define FILM_CONTACT	(20)
#define FILM_MEMORANDUM	(21)
#define FILM_ALOHA	(22)
#define FILM_SIGNUP	(23)
#define	FILM_FAVORITE	(24)
#define	FILM_MOVIE	(25)	/* normal movies */


#define FILM_ROW	(40)

typedef struct
{
  UTMP uslot[MAXACTIVE];	/* UTMP slots */
  usint count;			/* number of active session */
  usint offset;			/* offset for last active UTMP */

  double sysload[3];
  int avgload;

  BMW *mbase;			/* sequential pointer for BMW */
  BMW mpool[BMW_MAX];
#ifdef	HAVE_PIP_FIGHT1
  PIPUTMP pip[PIP_MAX];
#endif

} UCACHE;


typedef struct
{
  BRD bcache[MAXBOARD];
  int mantime[MAXBOARD];           /* �U�O�ثe�����h�֤H�b�\Ū */  
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
  char verb[9];                   /* �ʵ� */
  char chinese[7];                /* ����½Ķ */
  char part1_msg[60];              /* ���� */
  char part2_msg[60];              /* �ʧ@ */  
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
/* screen.c ���B�Ϊ���Ƶ��c				 */
/* ----------------------------------------------------- */


#define ANSILINELEN (255)	/* Maximum Screen width in chars */


/* Screen Line buffer modes */


#define SL_MODIFIED	(1)	/* if line has been modifed, screen output */
#define SL_STANDOUT	(2)	/* if this line contains standout code */
#define SL_ANSICODE	(4)	/* if this line contains ANSI code */


typedef struct screenline
{
  uschar oldlen;		/* previous line length */
  uschar len;			/* current length of line */
  uschar width;			/* padding length of ANSI codes */
  uschar mode;			/* status of line, as far as update */
  uschar smod;			/* start of modified data */
  uschar emod;			/* end of modified data */
  uschar sso;			/* start of standout data */
  uschar eso;			/* end of standout data */
  uschar data[ANSILINELEN];
}          screenline;


typedef struct LinkList
{
  struct LinkList *next;
  char data[0];
}        LinkList;


/* ----------------------------------------------------- */
/* xover.c ���B�Ϊ���Ƶ��c				 */
/* ----------------------------------------------------- */


typedef struct OverView
{
  int pos;			/* current position */
  int top;			/* top */
  int max;			/* max */
  int key;			/* key */
  char *xyz;			/* staff */
  struct OverView *nxt;		/* next */
  char dir[0];			/* data path */
}        XO;


typedef struct
{
  int key;
  int (*func) ();
}      KeyFunc;


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
}      TagItem;


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
/* acct.c ���B�Ϊ���Ƶ��c        	                 */
/* ----------------------------------------------------- */
#define	DENY_SEL_TALK	0x00000001		
#define	DENY_SEL_POST	0x00000002
#define	DENY_SEL_MAIL	0x00000004
#define	DENY_SEL_AD	0x00000008
#define	DENY_SEL_SELL	0x00000010
#define	DENY_SEL_OK	0x00000020
#define	DENY_SEL	(DENY_SEL_TALK|DENY_SEL_POST|DENY_SEL_MAIL|DENY_SEL_AD|DENY_SEL_SELL)

#define	DENY_DAYS_1	0x00010000
#define	DENY_DAYS_2	0x00020000
#define	DENY_DAYS_3	0x00040000
#define	DENY_DAYS_4	0x00080000
#define DENY_DAYS_5	0x00100000
#define DENY_DAYS	(DENY_DAYS_1|DENY_DAYS_2|DENY_DAYS_3|DENY_DAYS_4|DENY_DAYS_5)

#define	DENY_MODE_TALK	0x01000000
#define	DENY_MODE_MAIL	0x02000000
#define	DENY_MODE_POST	0x04000000
#define DENY_MODE_GUEST	0x08000000
#define	DENY_MODE_NICK	0x10000000
#define	DENY_MODE_ALL	(DENY_MODE_TALK|DENY_MODE_MAIL|DENY_MODE_POST|DENY_MODE_NICK)


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
/* classtable.c ���B�Ϊ���Ƶ��c                         */
/* ----------------------------------------------------- */
typedef struct
{
  int lost;
  char name[9];
  char teacher[9];
  char class[5];
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
}  CLASS_TIME;

typedef struct
{
  int mode;
  CLASS table[6][13];
  CLASS_TIME time;
}   CLASS_TABLE;

/* ----------------------------------------------------- */
/* list.c ���B�Ϊ���Ƶ��c                               */
/* ----------------------------------------------------- */
typedef struct
{
  char userid[IDLEN+1];
}       LIST;

typedef struct
{
  char title[MAX_LIST][41];
}       LIST_TITLE;

/* ----------------------------------------------------- */
/* counter.c ���B�Ϊ���Ƶ��c                            */
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
}	COUNTER;

typedef struct
{
  time_t date;
  int mode;
  char userid[IDLEN + 1];
  char username[19];
  char buf[3][80];
}	notedata;

typedef struct
{
  int start;
  int end;
}	REAPER_TIME;

/* ----------------------------------------------------- */
/* �ͤ�~�֬P�y�έp ���B�Ϊ���Ƶ��c                     */
/* ----------------------------------------------------- */

#ifdef	HAVE_PERSON_DATA
#define	USER_ATTR_SUPPORT	0x00000001


typedef struct
{
  int year;
  int month;
  int day;
  int sex;
  int blood;
  int mode;
  char reserve[232];
}	USER_ATTR;
#endif

typedef	struct
{
  char *grad;
  char *server;
}	YZU_MAIL;

typedef struct
{
  char name[20];
  char host[26];
  char ip[16];
  char port[6];
}	BBSNET;  


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

#ifdef	HAVE_CLASSTABLE
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
}	CLASS_TABLE2;

#ifdef	HAVE_CLASSTABLEALERT
typedef struct
{
  char condensation[9];
  char room[20];
  usint used;
}	CLASS_TABLE_ALERT_ITEM;

typedef struct
{
  CLASS_TABLE_ALERT_ITEM item[78];
  usint userno;
}	CLASS_TABLE_ALERT;

#endif
#endif

#ifdef	HAVE_OBSERVE_LIST
typedef struct
{
  char userid[16];
  usint userno;
  char title[64];
  char reserve[44];
}	OBSERVE;

typedef struct
{
  usint userno[MAXOBSERVELIST];
  int total;
  int mode;
} OCACHE;

#endif

#if 0
#ifdef	HAVE_RECOMMEND
typedef struct PostRecommendHistory
{
  time_t chrono;
  time_t bstamp;
}	PRH;
#endif
#endif

typedef struct
{
  void *func;
  /* int (*func) (); */
  usint level;
  int umode;
  char *desc;
}      MENU;

#ifdef	HAVE_COUNT_BOARD
#define	BSTAT_LAST	0x01
#define	BSTAT_TOTAL	0x02

typedef struct
{
  time_t chrono;
  char  type[16];		/* �έp���A */
  usint n_reads;		/* �ݪO�\Ū�֭p times/hour */
  usint n_posts;		/* �ݪO�o��֭p times/hour */
  usint n_news;			/* �ݪO��H�֭p times/hour */
  usint n_bans;			/* �ݪO�ɫH�֭p times/hour */
  char reserve[28];		/* �O�d */
}	BSTAT;

typedef struct
{
  BSTAT hour;
  BSTAT day;
  BSTAT week;
  BSTAT twoweek;
  BSTAT month;
  BSTAT threemonth;
  BSTAT herfyear;
  BSTAT year;
  BSTAT lhour[24];
  BSTAT lday[24];
  BSTAT lweek[24];
  BSTAT lmonth[24];
}	BSTATCOUNT;
#endif


#ifdef  HAVE_DETECT_CROSSPOST
typedef struct
{
   int sum;
   int total;
}	CHECKSUM;

typedef struct
{
   CHECKSUM checksum[5];
   int post_modetype;
   int checknum;
}	CHECKSUMCOUNT;

#endif

typedef struct PersonalBoard
{
  char userid[IDLEN + 1];
  char email[60];
  char brdname[IDLEN + 1];
  char brdtitle[BTLEN + 1];
  usint state;
}	PB;

#define PB_APPLY	0x01	/* �ӽФ� */
#define PB_OPEN		0x02	/* �w�}�� */

/* ----------------------------------------------------- */
/* innbbsd ���B�Ϊ���Ƶ��c				 */
/* ----------------------------------------------------- */

typedef struct
{
  time_t chrono;	/* >=0:stamp -1:cancel */
  char board[IDLEN + 1];

  /* �H�U��쪺�j�p�P HDR �ۦP */
  char xname[32];
  char owner[47];
  char nick[50];
  char title[73];
} bntp_t;


#define INN_USEIHAVE	0x0001
#define INN_USEPOST	0x0002
#define INN_FEEDED	0x0010

typedef struct
{
char name[13];	/* �ӯ��� short-name */
char host[83];	/* �ӯ��� host */
int port;		/* �ӯ��� port */
usint xmode;		/* �ӯ��� xmode */
char blank[20];	/* �O�d */
int feedfd;		/* bbslink.c �ϥ� */
}nodelist_t;


#define INN_NOINCOME	0x0001
#define INN_ERROR	0x0004

typedef struct
{
char path[13];	/* �Ӹs�թҹ��઺���x */
char newsgroup[83];	/* �Ӹs�ժ��W�� */
char board[IDLEN + 1];/* �Ӹs�թҹ������ݪO */
char charset[11];	/* �Ӹs�ժ��r�� */
usint xmode;		/* �Ӹs�ժ� xmode */
int high;		/* �ثe���Ӹs�ժ����@�g */
}newsfeeds_t;


typedef struct
{
  char issuer[128];	/* NCM message ���o�H�H */
  char type[64];	/* NCM message �������W�� */
  int perm;		/* ���\�� NCM message �R�H (1:�} 0:��) */
  char blank[60];	/* �O�d */
} ncmperm_t;


#define INN_SPAMADDR	0x0001
#define INN_SPAMNICK	0x0002
#define INN_SPAMSUBJECT	0x0010
#define INN_SPAMPATH	0x0020
#define INN_SPAMMSGID	0x0040
#define INN_SPAMBODY	0x0100
#define INN_SPAMSITE	0x0200
#define INN_SPAMPOSTHOST 0x0400

typedef struct
{
  char detail[80];	/* �ӳW�h�� ���e */
  usint xmode;		/* �ӳW�h�� xmode */
  char board[IDLEN + 1];/* �ӳW�h�A�Ϊ��ݪO */
  char path[13];	/* �ӳW�h�A�Ϊ����x */
  char blank[18];	/* �O�d */
} spamrule_t;

/* ----------------------------------------------------- */
/* PAYCHECK : 32 bytes                            */
/* ----------------------------------------------------- */


typedef struct
{
	  time_t tissue;                /* µo¤ä²¼®?¶¡ */
	    int money;
		  int gold;
		    char reason[20];
}      PAYCHECK;



#endif				/* _STRUCT_H_ */
