/*-------------------------------------------------------*/
/* modes.h	( NTHU CS MapleBBS Ver 2.36 )		 */
/*-------------------------------------------------------*/
/* target : user operating mode & status		 */
/* create : 95/03/29				 	 */
/* update : 95/12/15				 	 */
/*-------------------------------------------------------*/


#ifndef _MODES_H_
#define _MODES_H_

enum
{STRIP_ALL, ONLY_COLOR, NO_RELOAD};

/* ----------------------------------------------------- */
/* user �ާ@���A�P�Ҧ�					 */
/* ----------------------------------------------------- */

#define M_IDLE		0
#define M_MMENU		1	/* menu mode */
#define M_ADMIN		2
#define M_MAIL		3
#define M_TMENU		4
#define M_UMENU		5
#define M_SMENU		6
#define M_XMENU		7
#define M_PROFESS       8
#define M_CLASS		9

#define M_LOGIN		10	/* main menu */

#define M_GEM		11	/* announce */

#define M_BOARD		12
#define M_READA		13

#define M_XFILES	14
#define M_UFILES	15	/* user menu */

#define M_RMAIL		16	/* mail menu */

#define M_PAL		17
#define M_LUSERS	18

#define	M_BBTP		19
#define	M_WEB		20

#define M_POST		21	/* -> active mode : ���౵�� talk request */
#define M_SMAIL		22      /* Thor.981020: �ثe�� M_BBTP�P M_CHAT����talk*/
#define M_TRQST		23
#define M_TALK		24
#define M_CHAT		25

#define M_PAGE		26
#define M_QUERY		27	/* <- mated mode : �i����H */

#define M_VOTE		28

#define M_SYSTEM	29
#define M_BMW_REPLY     30      /* lkchu.981201: ���T�^�� */
#define M_BMW           31      /* lkchu.981230: ���T�^�U*/
#define M_BANMAIL	32	/* visor: �׫H�C�� */
#define M_OMENU		33
#define M_CHESS		M_CHAT

#define M_MIME		34	/* statue: mime �ѽX */
#define M_CHICKEN	35
#define M_XMODE		36
#define M_MYFAVORITE    37

#define M_MAX           M_XMODE
#define	M_BBSNET	0


#ifdef	_MODES_C_
static char *ModeTypeTable[] =
{
  "�o�b",			/* M_IDLE */
  "�D���",			/* M_MMENU */
  "�t�κ��@",			/* M_ADMIN */
  "�l����",			/* M_MAIL */
  "��Ϳ��",			/* M_TMENU */
  "�ϥΪ̿��",			/* M_UMENU */
  "�t�θ�T",			/* M_SMENU */
  "�����s�u���",		/* M_XMENU */
  "�M�~�ݪO",			/* M_PROFESS */
  "�����ݪO",			/* M_CLASS */
  "�W���~��",			/* M_LOGIN */
  "���G��",			/* M_GEM */
  "�ݪO�C��",			/* M_BOARD */
  "�\\Ū�峹",			/* M_READA */
  "�s�t���ɮ�",			/* M_XFILES */
  "�s�ӤH�ɮ�",			/* M_UFILES */
  "Ū�H",			/* M_RMAIL */
  "����B��",			/* M_PAL */
  "�ϥΪ̦W��",			/* M_LUSERS */
  "[BBTP]",			/* M_BBTP */
  "[WEB]",			/* M_WEB */
  "�o��峹",			/* M_POST */
  "�g�H",			/* M_SMAIL */
  "�ݾ�",			/* M_TRQST */
  "���",			/* M_TALK */
  "��ѫ�",			/* M_CHAT */
  "�I�s",			/* M_PAGE */
  "�d��",			/* M_QUERY */
  "�벼��",			/* M_VOTE */
  "�t�κ޲z",			/* M_SYSTEM */
  "���T�^��",                   /* M_BMW_REPLY *//*���T�^��*/
  "��ݰT��",                   /* M_BMW *//*��ݰT��*/
  "�׫H�C��",			/* M_BANMAIL */
  "��L���",                   /* M_OMENU */
  "MIMEŪ�H",			/* M_MIME */
  "�i����",			/* M_CHICKEN */
  "��L"			/* M_XMODE */
  "�ڪ��̷R"			/* MyFavorate */
};


#endif				/* _MODES_C_ */


/* ----------------------------------------------------- */
/* menu.c �����Ҧ�					 */
/* ----------------------------------------------------- */


#define XEASY	0x333		/* Return value to un-redraw screen */
#define QUIT	0x666		/* Return value to abort recursive functions */
#define	SKIN	0x999		/* Return value to change skin */


/* ----------------------------------------------------- */
/* read.c �����Ҧ�					 */
/* ----------------------------------------------------- */


#define	TAG_NIN		0	/* ���ݩ� TagList */
#define	TAG_TOGGLE	1	/* ���� Taglist */
#define	TAG_INSERT	2	/* �[�J TagList */


/* for bbstate : bbs operating state */


#define	STAT_POST	    0x01000000	/* �O�_�i�H�b currboard �o��峹 */
#define	STAT_BOARD	    0x02000000	/* �O�_�i�H�b currboard �R���Bmark�峹 */
#define	STAT_VISIT	    0x04000000	/* �O�_�w�g�ݹL�i�O�e�� */
#define	STAT_DIRTY	    0x08000000	/* �O�_��ʹL userflag */
#define	STAT_MODERATED	0x10000000	/* currboard �O�_�� moderated board */
#define	STAT_LOCK	    0x20000000	/* �O�_����w�ù� */
#define STAT_BM         0x40000000  /* �O�_�� currboard ���O�D */
#define	STAT_STARTED	0x80000000	/* �O�_�w�g�i�J�t�� */

#define OTHERSTAT_EDITING    0x0000001       /* �s�褤 */


/* for user's board permission level & state record */


#define BRD_R_BIT	0x01	/* �iŪ�Aread */
#define BRD_W_BIT	0x02	/* �i�g�Awrite */
#define BRD_X_BIT	0x04	/* �i�ޡAexecute�A�O�D�B�ݪO�`�ޡB���� */
#define BRD_V_BIT	0x08	/* �w�g�}�L�F ==> �ݹL�u�i�O�e���v */
#define BRD_H_BIT	0x10	/* .boardrc �����\Ū�O�� (history) */
#define BRD_Z_BIT	0x20	/* zap ���F */
#define BRD_F_BIT	0x40	/* �n�ͬݪO*/

/* for curredit */


#define	EDIT_MAIL	0x01	/* �ثe�O mail/board ? */
#define	EDIT_LIST	0x02	/* �O�_�� mail list ? */
#define	EDIT_BOTH	0x04	/* both reply to author/board ? */
#ifdef HAVE_ANONYMOUS
#define EDIT_ANONYMOUS  0x08	/* �ΦW�Ҧ� */
#endif
#define	EDIT_OUTGO	POST_OUTGO
#define EDIT_RESTRICT	0x10

/* ----------------------------------------------------- */
/* xover.c �����Ҧ�					 */
/* ----------------------------------------------------- */

#if 1
#define XO_DL		0x80000000
#endif

#define	XO_MODE		0x10000000


#define XO_NONE		(XO_MODE + 0)
#define XO_INIT		(XO_MODE + 1)
#define XO_LOAD		(XO_MODE + 2)
#define XO_HEAD		(XO_MODE + 3)
#define XO_NECK		(XO_MODE + 4)
#define XO_BODY		(XO_MODE + 5)
#define XO_FOOT		(XO_MODE + 6)
#define XO_LAST		(XO_MODE + 7)
#define	XO_QUIT		(XO_MODE + 8)

#define	XO_RSIZ		256		/* max record length */
#define	XO_TALL		20		/* page size */


#define	XO_MOVE		0x20000000	/* cursor movement */
#define	XO_WRAP		0x08000000	/* cursor wrap in movement */
#define	XO_TAIL		(XO_MOVE - 999)	/* init cursor to tail */


#define	XO_ZONE		0x40000000	/* �i�J�Y�@�� zone */
#define	XZ_BACK		0x100


#define	XZ_CLASS	(XO_ZONE + 0)	/* �ݪO�C�� */
#define	XZ_ULIST	(XO_ZONE + 1)	/* �u�W�ϥΪ̦W�� */
#define	XZ_PAL		(XO_ZONE + 2)	/* �n�ͦW�� */
#define	XZ_VOTE		(XO_ZONE + 3)	/* �벼 */
#define XZ_BMW          (XO_ZONE + 4)   /* ���T */

#ifdef HAVE_XYPOST
#define XZ_XPOST        (XO_ZONE + 5)   /* �j�M�峹�Ҧ� */
#endif

/* �H�U���� thread �D�D���\Ū���\�� */

#define	XZ_MBOX		(XO_ZONE + 6)	/* �H�c */
#define	XZ_BOARD	(XO_ZONE + 7)	/* �ݪO */
#define XZ_GEM		(XO_ZONE + 8)	/* ��ذ� */
#define	XZ_MAILGEM	(XO_ZONE + 9)	/* �H���ذ� */
#define XZ_BANMAIL	(XO_ZONE + 10)	/* �׫H�C�� */
#define XZ_OTHER	(XO_ZONE + 11)	/* ��L�C�� */
#define XZ_MYFAVORITE	(XO_ZONE + 12)  /* �ڪ��̷R */

#define	XZ_POST		XZ_BOARD



//cache,20101119

#define PALTYPE_PAL     0               /* ªB¤?¦W³æ */
#define PALTYPE_LIST    1               /* ¸s²?¦W³æ */
#define PALTYPE_BPAL    2               /* ªO¤?¦W³æ */
#define PALTYPE_VOTE    3               /* ­­¨î§ë²¼¦W³æ */



#endif				/* _MODES_H_ */
