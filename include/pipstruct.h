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
  unsigned int age;			  /* 年齡	  4 bytes */
  unsigned char race;			  /* 種族	  1 bytes */
  unsigned char subrace;		  /* 副業	  1 bytes */
  ushort level;			  /* 等級	  2 bytes */  
  char family[20];		  /* 家族	 20 bytes */
  char nick[20];		  /* 封號	 20 bytes */
  int hp;			  /* 體力	  4 bytes */
  int mp;			  /* 法力	  4 bytes */
  unsigned int skill;			  /* 技能	  4 bytes */
  ushort str;			  /* 力量	  2 bytes */
  ushort dex;			  /* 敏捷	  2 bytes */
  ushort wis;			  /* 智慧	  2 bytes */
  ushort con;			  /* 體質	  2 bytes */
  ushort kar;			  /* 運氣	  2 bytes */
  unsigned char weapon;		  /* 武器	  1 bytes */
  unsigned char armor;			  /* 防具	  1 bytes */
  unsigned int object;			  /* 物件	  4 bytes */
  char pad[164];
};
typedef struct rpgrec rpgrec;

#if 0
/* ----------------------------------------------------- */
/* Structure used in UTMP file : ??? bytes               */
/* ----------------------------------------------------- */

/* 電子雞 */
typedef struct pipdata
{
  char name[20];
  int hp;         /*體力*/
  int maxhp;      /*體力上限*/
  int mp;         /*法力*/
  int maxmp;      /*法力上限*/
  int attack;     /*攻擊*/
  int resist;     /*防禦*/
  int speed;      /*速度*/
  int mresist;    /*魔法防禦*/
  int resistmode; /*防禦型態*/
  int nodone;     /*完成*/
  int leaving;    /*離開*/
  int pipmode;    /*狀態*/
  int money;      /*金錢*/
  int msgcount;   /*訊息個數*/
  int chatcount;
  char msg[150];  /*訊息內容*/
  char chat[10][150]; /*聊天內容*/
}pipdata;


struct user_info
{
  int uid;                      /* Used to find user name in passwd file */
  pid_t pid;                    /* kill() to notify user of talk request */
  int sockaddr;                 /* ... */
  int destuid;                  /* talk uses this to identify who called */
  struct user_info* destuip;
  unsigned char active;                /* When allocated this field is true */
  unsigned char invisible;             /* Used by cloaking function in Xyz menu */
  unsigned char sockactive;            /* Used to coordinate talk requests */
  unsigned int userlevel;
  unsigned char mode;                  /* UL/DL, Talk Mode, Chat Mode, ... */
  unsigned char pager;                 /* pager toggle, YEA, or NA */
  unsigned char in_chat;               /* for in_chat commands   */
  unsigned char sig;                   /* signal type */
  char userid[IDLEN + 1];
  char chatid[11];              /* chat id, if in chat mode */
  char realname[20];
  char username[24];
  char from[27];                /* machine name the user called in from */
  int from_alias;
  char birth;                   /* 是否是生日 Ptt*/
  char tty[11];                 /* tty port */
  unsigned char msgcount;
  time_t uptime;
  time_t lastact;             /* 上次使用者動的時間 */
  unsigned int brc_id;
  unsigned char lockmode;
  pipdata pip;
  int turn;
  char feeling[4];		/* 心情 */
};
typedef struct user_info user_info;
#endif


#endif
