/*-------------------------------------------------------*/
/* config.h	( NCKU CCNS WindTop-DreamBBS Rev.Beta 3) */
/*-------------------------------------------------------*/
/* target : site-configurable settings		 	 */
/* create : 95/03/29				 	 */
/* update : 106/2/24				 	 */
/*-------------------------------------------------------*/

#ifndef	_CONFIG_H_
#define	_CONFIG_H_

#undef	TREAT
#undef	TRANUFO

/* ----------------------------------------------------- */
/* �w�q BBS ���W��}					 */
/* ------------------------------------------------------*/

#define BOARDNAME       "�ڤ��j�a"                  /* ���寸�W */
#define NICKNAME	"�ڤ��j�a"                  /* ����²�� */
#define OWNER	        "NCKU.CCNS"                 /* �Ǯ�²�� */
#define MYHOSTNAME	"ccns.cc"                   /* ������} */
#define BBSVERSION      "Rev.Beta 3"                /* �����s�� */
#define SYSOPNICK       "�ڤ����F"                  /* SYSOP �ʺ� */
#define BBSHOME		"/home/bbs"                 /* BBS ���a */
#define BBSUID	        9999
#define BBSGID          999
#define TAG_VALID       "[DreamBBS]To "             /* �����{�Ҩ�token */
#define	HIDEDN_SRC	"bbs.ccns.ncku.edu.tw"      /* ���èӷ���m */
#define BBSNAME		"DreamBBS"                  /* �^�寸�W */
#define	BBSIP           "140.116.249.140"           /* bbs �� ip */

/* ----------------------------------------------------- */
/* Modules & Plug-in                       by cache      */
/* ------------------------------------------------------*/

#define USE_M3_MENU             /* �}�� M3 ��椶�� */

#undef  AUTO_FIX_INFO           /* �}�ҨϥΪ̦C��P���ˬd�W��/�O�͸�T (���D�����D�_�h�ФŶ}��) */

#define loginAD                 /* �i���[�ݼs�i */ 
#define NOIDENT                 /* �i�����Ϭd,�[�ֳs�u�t�� */
#define ANTI_PHONETIC           /* �}���ˬd�`����\�� */

#define Modules                 /* ��ܼҲո�T */
#define MultiRecommend          /* ��ܦh������ */
#define M3_USE_PMORE            /* pmore 2007 */
#define GRAYOUT                 /* �H�J�H�X�S�Ĩt�� */

/* ----------------------------------------------------- */
/* �H BBS ���W�Ҧ������X�W				 */
/* ----------------------------------------------------- */

#ifndef	MAXBOARD
#define MAXBOARD	(7000)		/* �̤j�}�O�Ӽ� */
#endif

#ifndef	MAXACTIVE
#define MAXACTIVE	(4096)		/* �̦h�P�ɤW���H�� (ex.1400)*/
					/* �Y�n�W�L 1024 �Эק� kernel */
					/* options         SHMMAXPGS=? */ 
#endif

#define MAXFIREWALL	(300)		/* ����׫H�C��W�� */

#define MAXOFILEWALL	(500)		/* �ݪ��׫H�C��W�� */

#define PAL_MAX		(512)		/* �n�ͦW��W�� */

#define PAL_ALMR	(450)		/* ĵ�i */

#define	BANMSG_MAX	(32)		/* �ڦ��T���W��W�� */

/* ----------------------------------------------------- */
/* guest���W�r                             by Jerics     */
/* ------------------------------------------------------*/

#define	MAXGUEST	(16)		/* �̦h���X�� guest�H */

#define GUESTNAME       (1)             /* guest�W�٪��ƶq */
#define GUEST_NAMES     GUEST_1
#define GUEST_1         "�ڤ��j�a�X��"

#define CHATROOMNAME    "�ڨ��ڻy"

/* ----------------------------------------------------- */
/* �N���\�j�Ǫ� mail server �w�� define                  */
/* ----------------------------------------------------- */

#define	DEFAULTSERVER   "mail.ncku.edu.tw"

#define NCKUMAIL        "mail.ncku.edu.tw"

/* ----------------------------------------------------- */
/* �պA�W��						 */
/* ------------------------------------------------------*/

#define CLASS_HOT (10)                 /* �����ݪO�{�ɭ� */

#undef CAN_POSTFIX                     /* po��e��ܤ峹���O */

#define MAX_MODIFY (5)                 /* �̤j�ק�峹�� */

#define HAVE_DETECT_ZHANGBA            /* �����i���� */

#define	HAVE_WEBBBS                    /* only for WindTop */

#undef	HAVE_PIP_FIGHT                 /* �p����� */ /* �|�����u �ܦh bug */
#define	HAVE_PIP_FIGHT1

#define	EMAIL_JUSTIFY                  /* �o�X InterNet Email �����{�ҫH�� */

#define	HAVE_BRDTITLE_CHANGE           /* ���D�ק睊�W */

#define	HAVE_OBSERVE_LIST              /* �t���[��W�� */

#undef	NEWUSER_LIMIT                  /* �s��W�����T�ѭ��� */

#undef 	HAVE_REPORT                    /* �t�ΰl�ܳ��i */ /* �|�����u�ФŶ}�� */

#define	HAVE_RECOMMEND                 /* ���ˤ峹 */

#define	HAVE_MMAP               /* �ĥ� mmap(): memory mapped I/O */
				/* �b SunOS�BFreeBSD �W�i�H�[�t 30 �� */
				/* Linux �� mmap() �����D�A�мȮɤ��n�� */

#ifdef	HAVE_MMAP
#include <sys/mman.h>
#ifdef MAP_FILE		        /* 44BSD defines this & requires it to mmap files */
#  define BBS_MAP	(MAP_SHARED | MAP_FILE)
#else
#  define BBS_MAP	(MAP_SHARED)
#endif
#endif

#define	HAVE_INPUT_TOOLS	/* �Ÿ���J�u�� */

#define	HAVE_BANMSG		/* �ڦ��T���\��C�� */

#define	HAVE_DETECT_CROSSPOST	/* cross-post �۰ʰ��� *//* 2010/3/24 �Ȯɰ��� by float*/

#define	HAVE_DETECT_VIOLAWATE	/* �H�k���� */

#define	HAVE_RESIST_WATER	/* ������\�� */   

#define	HAVE_CLASSTABLE		/* �ӤH�\�Ҫ� */

#ifdef	HAVE_CLASSTABLE
#define	HAVE_CLASSTABLEALERT	/* �Ҫ�ɨ�q�� */
#endif

#undef	HAVE_CHANGE_SKIN        /* ���� skin *//* �|���������n�}�� */

#define	HAVE_SEM                /* �ϥ� Semaphores */

#define	HAVE_DOWNLOAD           /* ���]�^�a */ 

#define	HAVE_MULTI_CROSSPOST    /* �s����K */

#define	HAVE_CROSSPOSTLOG       /* ��K�峹������ CrossPost �ݪO */

#define	HAVE_BM_CHECK           /* �O�D�T�{ */

#define	HAVE_RAND_INCOME        /* �üƶi���e�� */

#define	HAVE_MAILGEM            /* �ϥΫH���ذ� */

#define	HAVE_FAVORITE           /* �ϥΧڪ��̷R */

#undef	HAVE_PROFESS            /* �ϥαM�~�Q�װ� */

#define HAVE_STUDENT            /* �ϥξǥͤ��i�� */

#define HAVE_ACTIVITY		/* gaod:���ʤ��i�� */
#ifdef HAVE_ACTIVITY
#define BRD_ABULLETIN   "activity"
#endif

#define	HAVE_USER_MODIFY        /* �ϥΪ̭ק�峹 */

#define	HAVE_INFO               /* �ϥήդ褽�i�� */

#define	HAVE_SHOWNUMMSG         /* ��ܤ��y�Ӽ� */

#undef	HAVE_MAXRESERVE	        /* �����O�d�W����m *//* �|�����D  �ФŶ}�� */

#define	HAVE_MAILUNDELETE       /* �_��R���H�c�����H�� */

#define HAVE_WRITE              /* �զW�� */

#define	HAVE_SONG               /* �����I�q�t�� */

#define	HAVE_SONG_TO_CAMERA     /* �O�_�I�q��ʺA�ݪ� */

#define	HAVE_MAIL_FIX           /* �ץ� mail title ���\�� *//* ps:�p�ߤ��n�ö} */

#define	HAVE_MODERATED_BOARD    /* ���ѯ��K�� */

#ifdef	HAVE_MODERATED_BOARD
#define HAVE_WATER_LIST	        /* ���Ѥ���\�� */

#ifdef	HAVE_WATER_LIST
#define	HAVE_SYSOP_WATERLIST    /* SYSOP ����W�� */
#endif
#endif	

#undef	HAVE_GAME_QKMJ          /* ���� QKMJ �C�� */

#define	HAVE_BOARD_PAL          /* ���ѪO�ͥ\�� */

#define HAVE_ANONYMOUS          /* ���� anonymous �O */

#define	HAVE_ORIGIN             /* ��� author �Ӧۦ�B */

#define HAVE_MIME_TRANSFER      /* ���� MIME �ѽX */

#define HAVE_SIGNED_MAIL        /* Thor.990409: �~�e�H��[ñ�W */

#ifdef HAVE_SIGNED_MAIL
#define PRIVATE_KEY_PERIOD      0   /* Thor.990409: ����key, auto gen */
#endif

#define SHOW_USER_IN_TEXT       /* �b��󤤦۰���� User ���W�r */

#ifndef	SHOW_USER_IN_TEXT
#define outx	outs
#endif

#undef  HAVE_BOARD_SEE          /* PERM_ALLBOARD �i�H�ݨ�����ݪO */

#define	HAVE_PERSON_DATA        /* ���ѥͤ鵥��L��� */

#define	HAVE_TERMINATOR         /* �׵��峹�j�k --- �تḨ���� */

#define HAVE_XYPOST             /* �峹�걵�Ҧ� */

#define	LOGIN_NOTIFY            /* �t�Ψ�M���� */

#define	AUTHOR_EXTRACTION       /* �M��P�@�@�̤峹 */

#define EVERY_Z                 /* ctrl-z Everywhere (���F�g�峹) */

#define EVERY_BIFF              /* Thor.980805: �l�t��B�ӫ��a */

#define BMW_TIME                /* Thor.980911: ���T�ɶ��O�� */

#undef	HAVE_COUNT_BOARD        /* visor.20030816: �ݪO��T�έp */

#undef	MODE_STAT               /* �[��βέp user ���ͺA, �H�����g���w */

#define	HAVE_DECLARE            /* �ϬY��title����� */

#define	HAVE_CHK_MAILSIZE       /* visor.20031111: �H�c�W�L�j�p�A�W����H�c */

#define	HAVE_ALOHA              /* lkchu.981201: �n�ͤW���q�� */

#define	HAVE_DECORATE           /* �аO�M�W�� */

#define MULTI_MAIL              /* �s�ձH�H�\�� */ /* Thor.981016: �������B�H */

#undef	HAVE_REGISTER_FORM      /* ���U��{�� */

#ifdef	HAVE_REGISTER_FORM
#undef	HAVE_SIMPLE_RFORM       /* ²�Ƶ��U�� */
#endif

#define	JUSTIFY_PERIODICAL      /* �w�������{�� */

#define	LOG_ADMIN               /* Thor.990405: lkchu patch: ��������v�� */

#define LOG_BMW                 /* lkchu.981201: ���T�O���B�z */

#define	LOG_TALK                /* lkchu.981201: ��ѰO���B�z */

#define	LOG_CHAT                /* ��ѫǬ��� */

#undef	LOG_BRD_USIES           /* lkchu.981201: �O���ݪO�\Ū���p�i�Ѳέp */

#define HAVE_ETC_HOSTS          /* lkchu.981201: �ѷ� etc/hosts �� database */

#define HAVE_CHANGE_FROM        /* lkchu.981201: ^F �Ȯɧ��G�m */

#define COLOR_HEADER            /* lkchu.981201: �ܴ��m����Y */

#undef	EMAIL_PAGE              /* lkchu.990429: �q�l�l��ǩI */

#define	FRIEND_FIRST            /* lkchu.990512: �n�͸m�e */

#define	HAVE_SMTP_SERVER        /* bmtad�O���H�{��, ���O�H�H�{��
				�H�H�{���O�bbbsd��, mail.c��bsmtp
				�nrelay���� statue.00725 */
#ifdef	HAVE_SMTP_SERVER
#define	SMTP_SERVER             {"mail.ncku.edu.tw",NULL}
#endif

#define	HAVE_FORCE_BOARD        /* �j�� user login �ɭ�Ū���Y���i�ݪO: */	
#ifdef	HAVE_FORCE_BOARD
#define	FORCE_BOARD             BRD_ANNOUNCE  /* statue.000725 */
#endif

#define	HAVE_BBSNET             /* ���� BBSNET */

#define HAVE_COLOR_VMSG	        /* �m�⪺VMSG */

#define HAVE_POST_BOTTOM        /* �峹�m�� */


/* ----------------------------------------------------- */
/* ��L�t�ΤW���Ѽ�					 */
/* ----------------------------------------------------- */

#ifdef  HAVE_PIP_FIGHT1
#define	PIP_MAX	        (16)
#endif

#ifdef	HAVE_RECOMMEND
#define	RECOMMAND_TIME  (30)    /* ����ɶ� */
#endif

#define LOGINATTEMPTS   (3)     /* �̤j�i�����~���� */

#define LOGINASNEW      (1)     /* �ĥΤW���ӽбb����� */

#define	MAX_REGIST      (10)    /* �̤j���U�� */

#define	MAX_RESERVE     (16)    /* ���ȫO�d��m */

#define MAX_MEMORANDUM  (200)   /* �̤j�Ƨѿ��Ӽ� */

#define MAX_CONTACT     (200)   /* �̤j�p���W��Ӽ� */

#define	BMW_MAX	        (128)   /* ���T�W�� */

#define	BMW_PER_USER    (9)     /* �C��ϥΪ̼��T�W�� */

#define	BMW_EXPIRE      (60)    /* ���T�B�z�ɶ� */

#define BMW_MAX_SIZE    (100)   /* ���T�W�� (K) */

#define	MAX_BBSMAIL     (10000) /* bbsmail ���H�W�� (��) */

#define	MAX_VALIDMAIL   (500)   /* �{�� user ���H�W�� (��) */

#define	MAX_NOVALIDMAIL	(500)   /* ���{�� user ���H�W�� (��) */

#define MAIL_SIZE_NO	(3000)  /* ���{�� user ���H�W�� (K) */

#define MAIL_SIZE	(5000)  /* �̤j�H�c�W�� (K) */

#define	MAX_LIST	(6)     /* �s�զW��̤j�� */

#define MAX_MAIL_SIZE	(102400) /* �`�H�c�e�q */

                       /* Thor.981011: �H�Wfor bbs�D�{����, �W�L�h���� */
                       /* �O�o�P�B�ק� etc/mail.over */
                       /* Thor.981014: bquota �ΨӲM�L���ɮ�/�H��, �ɶ��p�U */
                       /* �O�o�P�B�ק� etc/justified �q����H�� etc/approved */

#define MARK_DUE        (180)    /* �аO�O�s���H�� (��) */
#define MAIL_DUE        (60)     /* �@��H�� (��) */
#define FILE_DUE        (30)     /* ��L�ɮ� (��) */

#define	MAX_ALOHA	(64)     /* �W���q���ޤJ�� */

#define MAX_BOOKS	(16)     /* �̤j���W�U�� */

#define NBRD_MAX	(30)     /* �̤j�s�p�� */

#define NBRD_DAYS	(14)     /* �s�p�Ѽ� */

#define NBRD_MAX_CANCEL	(30)     /* �̤j�o���D�H�� */

#define	MAXPAGES	(256)    /* more.c ���峹���ƤW�� (lines/22),�w�]:128 */

#define	MOVIE_MAX	(180)    /* �ʵe�i�� */

#define	MOVIE_SIZE	(108*1024) /* �ʵe cache size */

#ifdef	HAVE_RECOMMEND
#define	MIN_RECOMMEND	(3)      /* �̤p���ˤ峹��ܼƦr */
#endif

#ifdef	HAVE_DETECT_CROSSPOST
#define MAX_CHECKSUM	(6)      /* crosspost checksum ��� */

#define MAX_CROSS_POST	(10)     /* cross post �̤j�ƶq */
#endif

#ifdef	HAVE_OBSERVE_LIST        /* �t���[��W�� */
#define	MAXOBSERVELIST	(32)
#define	BRD_OBSERVE              "ObserveList"
#endif

#define	CHECK_PERIOD	(86400 * 14)      /* ��z�H�c/�n�ͦW�檺�g�� */

#define	VALID_PERIOD	(86400 * 365 * 4) /* �����{�Ҧ��Ĵ� */
                                          /* Thor.981014: �O�o�P�B�ק� etc/re-reg */

#define CHECK_BM_TIME	(86400 * 14)      /* ���D�T�{�ɶ� */

#define IDLE_TIMEOUT	(60)              /* �o�b�L�[�۰�ñ�h */

#define	MAX_HOTBOARD	(20)              /* �����ݪO */

#ifdef	HAVE_RESIST_WATER
#define	CHECK_QUOT	(3)               /* �峹����̤p��� */

#define CHECK_QUOT_MAX	(3)               /* �峹����s��g�� */	
#endif

#define	BANMAIL_EXPIRE	(30)              /* �׫H�C���s (��) */

#define CNA_MAX         20                /* lkchu.981201: �Y�ɷs�D�W�� */

#define	BBSNETMAX	10                /* BBSNET �̤j�s�u�ƶq */

/* ----------------------------------------------------- */
/* chat.c & xchatd.c ���ĥΪ� port �� protocol		 */
/* ------------------------------------------------------*/

#define	BBTP_PORT       4884

#define CHAT_PORT       3838
#define CHAT_SECURE     /* �w������ѫ� */

#define EXIT_LOGOUT     0
#define EXIT_LOSTCONN   -1
#define EXIT_CLIERROR   -2
#define EXIT_TIMEDOUT   -3
#define EXIT_KICK       -4

#define CHAT_LOGIN_OK       "OK"
#define CHAT_LOGIN_EXISTS   "EX"
#define CHAT_LOGIN_INVALID  "IN"
#define CHAT_LOGIN_BOGUS    "BG"

/* more.c edit.c ½�� */

#define PAGE_SCROLL	(b_lines - 1)   /* �� PgUp/PgDn �n���ʴX�C */

#define MAXLASTCMD	8		/* line input buffer */
#define	BLK_SIZ		4096		/* disk I/O block size */
#define MAXSIGLINES	6		/* edit.c ñ�W�ɤޤJ�̤j��� */
#define MAXQUERYLINES	15		/* talk.c xchatd.c ��� Query/Plan �T���̤j��� */
#define	MAX_CHOICES	32		/* vote.c �벼�ɳ̦h�� 32 �ؿ�� */
#define	TAG_MAX		256		/* xover.c TagList ���Ҽƥؤ��W�� */
#define LINE_HEADER	3		/* more.c bhttpd.c ���Y���T�C */
#define IS_ZHC_HI(x)    (x & 0x80)

/* �ʺA�ݪO & menu ��m */

#define MOVIE_LINES	(11)            /* �ʵe�̦h�� 11 �C */

#define MENU_XPOS	23              /* ���}�l�� (x, y) �y�� */
#define MENU_YPOS	13
#define	MENU_LOAD	1
#define	MENU_DRAW	2
#define	MENU_FILM	4

/* ----------------------------------------------------- */
/* �t�ΰѼ�						 */
/* ----------------------------------------------------- */

#define BRDSHM_KEY	(2997)
#define UTMPSHM_KEY	(1998)
#define FILMSHM_KEY	(2999)
#define FWSHM_KEY	(3999)
#define FWOSHM_KEY	(5000)
#define COUNT_KEY	(4000)

#ifdef	HAVE_OBSERVE_LIST               /* �t���[��W�� */
#define	OBSERVE_KEY	(6000)
#endif

#define	SPEAK_MAX	(50)
#define	CONDITION_MAX	(100)
#define	PARTY_MAX	(150)



#define	BSEM_KEY	2000            /* semaphore key */
#define	BSEM_FLG	0600            /* semaphore mode */
#define BSEM_ENTER      -1              /* enter semaphore */
#define BSEM_LEAVE      1               /* leave semaphore */
#define BSEM_RESET	0               /* reset semaphore */


/* ----------------------------------------------------- */
/* �ӽбb���ɭn�D�ӽЪ̯u����				 */
/* ----------------------------------------------------- */

#define REALINFO

#ifdef	REALINFO
#undef	POST_REALNAMES          /* �K���ɪ��W�u��m�W */
#undef	MAIL_REALNAMES          /* �H�����H��ɪ��W�u��m�W */
#undef	QUERY_REALNAMES         /* �Q Query �� User �i���u��m�W */
#endif

#endif                          /* _CONFIG_H_ */

