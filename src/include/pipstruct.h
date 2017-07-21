/*-------------------------------------------------------*/
/* struct.h     ( NTHU CS MapleBBS Ver 2.36 )            */
/*-------------------------------------------------------*/
/* target : all definitions about data structure         */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/


#ifndef _PIPSTRUCT_H_
#define _PIPSTRUCT_H_


/* ----------------------------------------------------- */
/* RPG struct :256 bytes    		                 */
/* ----------------------------------------------------- */
struct rpgrec
{
  char userid[IDLEN+1];		  /* User ID     13 bytes */
  usint age;			  /* �~��	  4 bytes */
  uschar race;			  /* �ر�	  1 bytes */
  uschar subrace;		  /* �Ʒ~	  1 bytes */
  ushort level;			  /* ����	  2 bytes */  
  char family[20];		  /* �a��	 20 bytes */
  char nick[20];		  /* �ʸ�	 20 bytes */
  int hp;			  /* ��O	  4 bytes */
  int mp;			  /* �k�O	  4 bytes */
  usint skill;			  /* �ޯ�	  4 bytes */
  ushort str;			  /* �O�q	  2 bytes */
  ushort dex;			  /* �ӱ�	  2 bytes */
  ushort wis;			  /* ���z	  2 bytes */
  ushort con;			  /* ���	  2 bytes */
  ushort kar;			  /* �B��	  2 bytes */
  uschar weapon;		  /* �Z��	  1 bytes */
  uschar armor;			  /* ����	  1 bytes */
  usint object;			  /* ����	  4 bytes */
  char pad[164];
};
typedef struct rpgrec rpgrec;

#if 0
/* ----------------------------------------------------- */
/* Structure used in UTMP file : ??? bytes               */
/* ----------------------------------------------------- */

/* �q�l�� */
typedef struct pipdata
{
  char name[20];
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
  int money;      /*����*/
  int msgcount;   /*�T���Ӽ�*/
  int chatcount;
  char msg[150];  /*�T�����e*/
  char chat[10][150]; /*��Ѥ��e*/
}pipdata;


struct user_info
{
  int uid;                      /* Used to find user name in passwd file */
  pid_t pid;                    /* kill() to notify user of talk request */
  int sockaddr;                 /* ... */
  int destuid;                  /* talk uses this to identify who called */
  struct user_info* destuip;
  uschar active;                /* When allocated this field is true */
  uschar invisible;             /* Used by cloaking function in Xyz menu */
  uschar sockactive;            /* Used to coordinate talk requests */
  usint userlevel;
  uschar mode;                  /* UL/DL, Talk Mode, Chat Mode, ... */
  uschar pager;                 /* pager toggle, YEA, or NA */
  uschar in_chat;               /* for in_chat commands   */
  uschar sig;                   /* signal type */
  char userid[IDLEN + 1];
  char chatid[11];              /* chat id, if in chat mode */
  char realname[20];
  char username[24];
  char from[27];                /* machine name the user called in from */
  int from_alias;
  char birth;                   /* �O�_�O�ͤ� Ptt*/
  char tty[11];                 /* tty port */
  uschar msgcount;
  time_t uptime;
  time_t lastact;             /* �W���ϥΪ̰ʪ��ɶ� */
  usint brc_id;
  uschar lockmode;
  pipdata pip;
  int turn;
  char feeling[4];		/* �߱� */
};
typedef struct user_info user_info;
#endif


#endif
