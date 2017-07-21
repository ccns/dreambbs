/*-------------------------------------------------------*/
/* perm.h	( NTHU CS MapleBBS Ver 2.36 )		 */
/*-------------------------------------------------------*/
/* target : permission levels of user & board		 */
/* create : 95/03/29				 	 */
/* update : 95/12/15				 	 */
/*-------------------------------------------------------*/


#ifndef	_PERM_H_
#define	_PERM_H_


/* ----------------------------------------------------- */
/* These are the 32 basic permission bits.		 */
/* ----------------------------------------------------- */

#define	PERM_BASIC	0x00000001	/* 1-8 : ���v�� */
#define PERM_CHAT	0x00000002
#define	PERM_PAGE	0x00000004
#define PERM_POST	0x00000008
#define	PERM_VALID 	0x00000010	/* LOGINOK */
#define PERM_MBOX	0x00000020
#define PERM_CLOAK	0x00000040
#define PERM_XEMPT 	0x00000080

#define	PERM_9		0x00000100
#define	PERM_10		0x00000200
#define	PERM_11		0x00000400
#define	PERM_12		0x00000800
#define	PERM_13		0x00001000
#define	PERM_14		0x00002000
#define	PERM_15		0x00004000
#define	PERM_SP		0x00008000

#define PERM_DENYPOST	0x00010000	/* 17-24 : �T���v�� */
#define	PERM_DENYTALK	0x00020000
#define	PERM_DENYCHAT	0x00040000
#define	PERM_DENYMAIL	0x00080000
#define	PERM_DENYSTOP	0x00100000
#define	PERM_DENYNICK	0x00200000
#define	PERM_DENYLOGIN	0x00400000
#define	PERM_PURGE	0x00800000

#define PERM_BM		0x01000000	/* 25-32 : �޲z�v�� */
#define PERM_SEECLOAK	0x02000000
#define PERM_KTV	0x04000000
#define PERM_GEM	0x08000000
#define PERM_ACCOUNTS	0x10000000
#define PERM_CHATROOM	0x20000000
#define	PERM_BOARD	0x40000000
#define PERM_SYSOP	0x80000000
#define PERM_SYSOPX	(PERM_SYSOP | PERM_BOARD | PERM_CHATROOM | PERM_ACCOUNTS)

#define PERM_MANAGE	0xFE0000E0
#define PERM_CRIMINAL	0x007F0000


/* ----------------------------------------------------- */
/* These permissions are bitwise ORs of the basic bits.	 */
/* ----------------------------------------------------- */


/* This is the default permission granted to all new accounts. */
#define PERM_DEFAULT 	(PERM_BASIC) 

#define PERM_ADMIN	(PERM_BOARD | PERM_ACCOUNTS | PERM_SYSOP | PERM_CHATROOM | PERM_KTV)
#define PERM_ALLBOARD	(PERM_SYSOP | PERM_BOARD)
#define PERM_LOGINCLOAK	(PERM_SYSOP | PERM_ACCOUNTS | PERM_BOARD | PERM_CHATROOM )
#define PERM_SEEULEVELS	PERM_SYSOP
#define PERM_SEEBLEVELS	(PERM_SYSOP | PERM_BM)


/*
#define PERM_NOTIMEOUT	PERM_SYSOP
*/

#define PERM_READMAIL	PERM_BASIC

#define PERM_INTERNET	PERM_VALID	/* �����{�ҹL�����~��H�H��
					   Internet */

#define PERM_FORWARD	PERM_INTERNET	/* to do the forwarding */

#define HAS_PERM(x)	((x)?cuser.userlevel&(x):1)
#define HAVE_PERM(x)	(cuser.userlevel&(x))


/* ----------------------------------------------------- */
/* �U���v��������N�q					 */
/* ----------------------------------------------------- */

#define	NUMPERMS	32


#ifdef _ADMIN_C_


static char *perm_tbl[] = {
  "���v�O",			/* PERM_BASIC */
  "�i�J��ѫ�",			/* PERM_CHAT */
  "��H���",			/* PERM_PAGE */
  "�o��峹",			/* PERM_POST */
  "�����{��",			/* PERM_VALID */
  "�H��L�W��",			/* PERM_MBOX */
  "�����N",			/* PERM_CLOAK */
  "�ä[�O�d�b��",		/* PERM_XEMPT */

  "���i�ݪO�o���v��",			/* PERM_9 */
  "�O�d",			/* PERM_10 */
  "�O�d",			/* PERM_11 */
  "�O�d",			/* PERM_12 */
  "�O�d",			/* PERM_13 */
  "�O�d",			/* PERM_14 */
  "�O�d",			/* PERM_15 */
  "�S��аO",			/* PERM_16 */

  "�T��o��峹",		/* PERM_DENYPOST */
  "�T�� talk",			/* PERM_DENYTALK */
  "�T�� chat",			/* PERM_DENYCHAT */
  "�T�� mail",			/* PERM_DENYMAIL */
  "���v",			/* PERM_DENYSTOP */
  "�T����ʺ�",		/* PERM_DENYNICK */
  "�T�� login",			/* PERM_DENYLOGIN */
  "�M���b��",			/* PERM_PURGE */

  "�O�D",			/* PERM_BM */
  "�ݨ��Ԫ�",			/* PERM_SEECLOAK */
  "�I�q�t���`��",		/* PERM_KTV */
  "��ذ��`��",			/* PERM_GEM */
  "�b���`��",			/* PERM_ACCOUNTS */
  "��ѫ��`��",			/* PERM_CHATCLOAK */
  "�ݪO�`��",			/* PERM_BOARD */
  "����"			/* PERM_SYSOP */
};

#endif


/* ----------------------------------------------------- */
/* ��ذ� (gem / gopher) ���� user permission level	 */
/* ----------------------------------------------------- */


#define GEM_QUIT	-2	/* recursive quit */
#define	GEM_VISIT	-1
#define	GEM_USER	0
#define	GEM_RECYCLE	1
#define	GEM_LMANAGER	2
#define GEM_MANAGER     3
#define	GEM_SYSOP	4


#endif				/* _PERM_H_ */
