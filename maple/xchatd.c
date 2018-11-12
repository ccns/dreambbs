/*-------------------------------------------------------*/
/* xchatd.c     ( NTHU CS MapleBBS Ver 3.00 )            */
/*-------------------------------------------------------*/
/* target : super KTV daemon for chat server             */
/* create : 95/03/29                                     */
/* update : 2000/01/02                                   */
/*-------------------------------------------------------*/


#include "bbs.h"
#include "xchat.h"

#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/shm.h>



#define	SERVER_USAGE
#define WATCH_DOG
#undef	DEBUG			/* 程式除錯之用 */
#undef	MONITOR			/* 監督 chatroom 活動以解決糾紛 */
#undef	STAND_ALONE		/* 不搭配 BBS 獨立執行 */


#ifdef	DEBUG
#define	MONITOR
#endif

static int gline;

#ifdef  WATCH_DOG
#define MYDOG  gline = __LINE__
#else
#define MYDOG			/* NOOP */
#endif


#define CHAT_PIDFILE    "run/chat.pid"
#define CHAT_LOGFILE    FN_CHAT_LOG
#define	CHAT_TALKFILE	FN_CHATDATA_LOG
#define	CHAT_INTERVAL	(60 * 30)
#define SOCK_QLEN	3


/* name of the main room (always exists) */


#define	MAIN_NAME	"Lobby"
#define	MAIN_TOPIC	"隨便聊聊..."




#define ROOM_LOCKED	1
#define ROOM_SECRET	2
#define ROOM_OPENTOPIC  4
#define ROOM_ALL	(NULL)


#define LOCKED(room)	(room->rflag & ROOM_LOCKED)
#define SECRET(room)	(room->rflag & ROOM_SECRET)
#define OPENTOPIC(room) (room->rflag & ROOM_OPENTOPIC)


#define RESTRICTED(usr)	(usr->uflag == 0)	/* guest */
#define CHATSYSOP(usr)	(usr->uflag & (PERM_SYSOP | PERM_CHATROOM))
#define	PERM_ROOMOP	PERM_CHAT	/* Thor: 借 PERM_CHAT為 PERM_ROOMOP */
#define	PERM_CHATOP	PERM_DENYCHAT	/* Thor: 借 PERM_DENYCHAT為 PERM_CHATOP */
/* #define ROOMOP(usr)  (usr->uflag & ( PERM_ROOMOP | PERM_SYSOP | PERM_CHATROOM)) */
/* Thor.980603: PERM_CHATROOM改為 default 沒有 roomop, 但可以自己取得 chatop*/
#define ROOMOP(usr)  (usr->uflag & (PERM_ROOMOP|PERM_CHATOP))
#define CLOAK(usr)	(usr->uflag & PERM_CLOAK)


/* ----------------------------------------------------- */
/* ChatRoom data structure                               */
/* ----------------------------------------------------- */


typedef struct ChatRoom ChatRoom;
typedef struct ChatUser ChatUser;
typedef struct UserList UserList;
typedef struct ChatCmd ChatCmd;
/*typedef struct ChatAction ChatAction;*/


MUD muddata;
MUD *mud;
void mudshm_init();


struct ChatUser
{
  ChatUser *unext;
  ChatRoom *room;
  UserList *ignore;
  int sock;			/* user socket */
  int userno;
  int uflag;
  int clitype;			/* Xshadow: client type. 1 for common client,
				 * 0 for bbs only client */
  time_t tbegin;
  time_t uptime;
  int sno;
  int xdata;
  int retry;

  int isize;			/* current size of ibuf */
  char ibuf[80];		/* buffer for non-blocking receiving */
  char userid[IDLEN + 1];	/* real userid */
  char chatid[9];		/* chat id */
  char rhost[30];		/* host address */
};


struct ChatRoom
{
  ChatRoom *next, *prev;
  UserList *invite;
  char name[IDLEN];
  char topic[48];		/* Let the room op to define room topic */
  int rflag;			/* ROOM_LOCKED, ROOM_SECRET, ROOM_OPENTOPIC */
  int occupants;		/* number of users in room */
};


struct UserList
{
  UserList *next;
  int userno;
  char userid[0];
};


struct ChatCmd
{
  char *cmdstr;
  void (*cmdfunc) ();
  int exact;
};


static ChatRoom mainroom, *roompool;
static ChatUser *mainuser, *userpool;
static fd_set mainfset;
static int totaluser;		/* current number of connections */
static struct timeval zerotv;	/* timeval for selecting */
static int common_client_command;


#ifdef STAND_ALONE
static int userno_inc = 0;	/* userno auto-incrementer */
#endif


static char msg_not_op[] = "◆ 您不是這間" CHATROOMNAME "的 Op";
static char msg_no_such_id[] = "◆ 目前沒有人使用 [%s] 這個聊天代號";
static char msg_not_here[] = "◆ [%s] 不在這間" CHATROOMNAME "。";


#define	FUZZY_USER	((ChatUser *) -1)

void load_mud_like(void);

/* ----------------------------------------------------- */
/* operation log and debug information                   */
/* ----------------------------------------------------- */


static FILE *flog;
static FILE *ftalk;


/* Thor.990211: 統一用 dao library */
#define str_time(t) Btime(t)

static void
logtalk(key, msg)
  char *key;
  char *msg;
{
  time_t now;
  struct tm *p;

  time(&now);
  p = localtime(&now);
  fprintf(ftalk, "%02d/%02d/%02d %02d:%02d:%02d %-13s%s\n",
    p->tm_year % 100, p->tm_mon + 1, p->tm_mday,
    p->tm_hour, p->tm_min, p->tm_sec, key, msg);
}


static void
logit(key, msg)
  char *key;
  char *msg;
{
  time_t now;
  struct tm *p;

  time(&now);
  p = localtime(&now);
  fprintf(flog, "%02d/%02d/%02d %02d:%02d:%02d %-13s%s\n",
    p->tm_year % 100, p->tm_mon + 1, p->tm_mday,
    p->tm_hour, p->tm_min, p->tm_sec, key, msg);
}


static inline void
log_init()
{
  FILE *fp;

  /* --------------------------------------------------- */
  /* log daemon's PID					 */
  /* --------------------------------------------------- */

  if ((fp = fopen(CHAT_PIDFILE, "w")))
  {
    fprintf(fp, "%d\n", getpid());
    fclose(fp);
  }

  flog = fopen(CHAT_LOGFILE, "a+");
  logit("START", "chat daemon");

  ftalk = fopen(CHAT_TALKFILE, "a+");
  logtalk("START", "chat room");
}


#ifdef	DEBUG
static char chatbuf[256];	/* general purpose buffer */


static void
debug_list(list)
  UserList *list;
{
  char buf[80];
  int i = 0;

  if (!list)
  {
    logit("DEBUG_L", "NULL");
    return;
  }
  while (list)
  {
    sprintf(buf, "%d) list: %p userno: %d next: %p", i++, list, list->userno, list->next);
    logit("DEBUG_L", buf);

    list = list->next;
  }
  logit("DEBUG_L", "end");
}


static void
debug_user()
{
  ChatUser *user;
  int i;
  char buf[80];

  sprintf(buf, "mainuser: %p userpool: %p", mainuser, userpool);
  logit("DEBUG_U", buf);
  for (i = 0, user = mainuser; user; user = user->unext)
  {
    /* MYDOG; */
    sprintf(buf, "%d) %p %-6d %s %s", ++i, user, user->userno, user->userid, user->chatid);
    logit("DEBUG_U", buf);
  }
}


static void
debug_room()
{
  ChatRoom *room;
  int i;
  char buf[80];

  i = 0;
  room = &mainroom;

  sprintf(buf, "mainroom: %p roompool: %p", mainroom, roompool);
  logit("DEBUG_R", buf);
  do
  {
    MYDOG;
    sprintf(buf, "%d) %p %s %d", ++i, room, room->name, room->occupants);
    logit("DEBUG_R", buf);
  } while (room = room->next);
}


static void
log_user(cu)
  ChatUser *cu;
{
  static int log_num;

  if (cu)
  {
    if (log_num > 100 && log_num < 150)
    {
      sprintf(chatbuf, "%d: %p <%d>", log_num, cu, gline);
      logit("travese user ", chatbuf);
    }
    else if (log_num == 100)
    {
      sprintf(chatbuf, "BOOM !! at line %d", gline);
      logit("travese user ", chatbuf);
    }
    log_num++;
  }
  else
    log_num = 0;
}
#endif				/* DEBUG */


/* ----------------------------------------------------- */
/* string routines                                       */
/* ----------------------------------------------------- */


static int
valid_chatid(id)
  char *id;
{
  int ch, len;

  for (len = 0; (ch = *id); id++)
  { /* Thor.980921: 空白為不合理chatid, 怕getnext判斷錯誤等等 */
    if (ch == '/' || ch == '*' || ch == ':' || ch ==' ')
      return 0;
    if (++len > 8)
      return 0;
  }
  return len;
}


/* Case Independent strcmp : 1 ==> euqal */


/* Thor.990211: 統一使用dao library */
#define str_equal(s1,s2) (!str_cmp(s1,s2))

/* ----------------------------------------------------- */
/* match strings' similarity case-insensitively          */
/* ----------------------------------------------------- */
/* str_match(keyword, string)				 */
/* ----------------------------------------------------- */
/* 0 : equal            ("foo", "foo")                   */
/* -1 : mismatch        ("abc", "xyz")                   */
/* ow : similar         ("goo", "good")                  */
/* ----------------------------------------------------- */


static int
str_match(s1, s2)
  uschar *s1;		/* lower-case (sub)string */
  uschar *s2;
{
  int c1, c2;

  for (;;)
  {
    c1 = *s1;
    c2 = *s2;

    if (!c1)
      return c2;

    if (c2 >= 'A' && c2 <= 'Z')
      c2 += 32;

    if (c1 != c2)
      return -1;

    s1++;
    s2++;
  }
}


/* ----------------------------------------------------- */
/* search user/room by its ID                            */
/* ----------------------------------------------------- */


static ChatUser *
cuser_by_userid(userid)
  char *userid;
{
  ChatUser *cu;
  char buf[80]; /* Thor.980727: 一次最長才80 */

  str_lower(buf, userid);
  for (cu = mainuser; cu; cu = cu->unext)
  {
    if (!cu->userno)
      continue;
    if (str_equal(buf, cu->userid))
      break;
  }
  return cu;
}


static ChatUser *
cuser_by_chatid(chatid)
  char *chatid;
{
  ChatUser *cu;
  char buf[80]; /* Thor.980727: 一次最長才80 */

  str_lower(buf, chatid);

  for (cu = mainuser; cu; cu = cu->unext)
  {
    if (!cu->userno)
      continue;
    if (str_equal(buf, cu->chatid))
      break;
  }
  return cu;
}


static ChatUser *
fuzzy_cuser_by_chatid(chatid)
  char *chatid;
{
  ChatUser *cu, *xuser;
  int mode;
  char buf[80]; /* Thor.980727: 一次最長才80 */

  str_lower(buf, chatid);
  xuser = NULL;

  for (cu = mainuser; cu; cu = cu->unext)
  {
    if (!cu->userno)
      continue;

    mode = str_match(buf, cu->chatid);
    if (mode == 0)
      return cu;

    if (mode > 0)
    {
      if (xuser)
	return FUZZY_USER;	/* 符合者大於 2 人 */

      xuser = cu;
    }
  }
  return xuser;
}


static ChatRoom *
croom_by_roomid(roomid)
  char *roomid;
{
  ChatRoom *room;
  char buf[80]; /* Thor.980727: 一次最長才80 */

  str_lower(buf, roomid);
  room = &mainroom;
  do
  {
    if (str_equal(buf, room->name))
      break;
  } while ((room = room->next));
  return room;
}


/* ----------------------------------------------------- */
/* UserList routines                                     */
/* ----------------------------------------------------- */


static void
list_free(list)
  UserList **list;
{
  UserList *user, *next;

  for (user = *list, *list = NULL; user; user = next)
  {
    next = user->next;
    free(user);
  }
}


static void
list_add(list, user)
  UserList **list;
  ChatUser *user;
{
  UserList *node;
  char *userid;
  int len;

  len = strlen(userid = user->userid) + 1;
  if ((node = (UserList *) malloc(sizeof(UserList) + len)))
  {
    node->next = *list;
    node->userno = user->userno;
    memcpy(node->userid, userid, len);
    *list = node;
  }
}


static int
list_delete(list, userid)
  UserList **list;
  char *userid;
{
  UserList *node;
  char buf[80]; /* Thor.980727: 輸入一次最長才 80 */

  str_lower(buf, userid);

  while ((node = *list))
  {
    if (str_equal(buf, node->userid))
    {
      *list = node->next;
      free(node);
      return 1;
    }
    list = &node->next;
  }

  return 0;
}


static int
list_belong(list, userno)
  UserList *list;
  int userno;
{
  while (list)
  {
    if (userno == list->userno)
      return 1;
    list = list->next;
  }
  return 0;
}


/* ------------------------------------------------------ */
/* non-blocking socket routines : send message to users   */
/* ------------------------------------------------------ */

static int
str_swap(str,src,des)
  char *str;
  char *src;
  char *des;
{
  char *ptr,*tmp;
  char buf[600];
  ptr = strstr(str,src);
  if(ptr)
  {
    *ptr = '\0';
    tmp = ptr + strlen(src);
    sprintf(buf,"%s%s%s",str,des,tmp);
    strcpy(str,buf);
    return 1;
  }
  else
    return 0;
}

static void
do_send(nfds, wset, msg)
  int nfds;
  fd_set *wset;
  char *msg;
{
  int len, sr;

#if 1
  /* Thor: for future reservation bug */
  zerotv.tv_sec = 0;
  zerotv.tv_usec = 0;
#endif
  while(str_swap(msg,"\\033","\033"));

  sr = select(nfds + 1, NULL, wset, NULL, &zerotv);

  if (sr > 0)
  {
    len = strlen(msg) + 1;
    do
    {
      if (FD_ISSET(nfds, wset))
      {
	send(nfds, msg, len, 0);
	if (--sr <= 0)
	  return;
      }
    } while (--nfds > 0);
  }
}


static void
send_to_room(room, msg, userno, number)
  ChatRoom *room;
  char *msg;
  int userno;
  int number;
{
  ChatUser *cu;
  fd_set wset;
  int sock, max;
  int clitype;			/* 分為 bbs client 及 common client 兩次處理 */
  char *str, buf[256];

  for (clitype = (number == MSG_MESSAGE || !number) ? 0 : 1;
    clitype < 2; clitype++)
  {
    FD_ZERO(&wset);
    max = -1;

    for (cu = mainuser; cu; cu = cu->unext)
    {
      if (cu->userno && (cu->clitype == clitype) &&
	(room == ROOM_ALL || room == cu->room) &&
	(!userno || !list_belong(cu->ignore, userno)))
      {
	sock = cu->sock;

	FD_SET(sock, &wset);

	if (max < sock)
	  max = sock;
      }
    }

    if (max <= 0)
      continue;

    if (clitype)
    {
      str = buf;

      if (*msg)
	sprintf(str, "%3d %s", number, msg);
      else
	sprintf(str, "%3d", number);
    }
    else
    {
      str = msg;
    }

    do_send(max, &wset, str);
  }
}


static void
send_to_user(user, msg, userno, number)
  ChatUser *user;
  char *msg;
  int userno;
  int number;
{
  int sock;

#if 0
  if (!user->userno || (!user->clitype && number && number != MSG_MESSAGE))
#endif
  /* Thor.980911: 如果查user->userno則在login_user的error message會無法送回 */
  if (!user->clitype && number != MSG_MESSAGE)
    return;

  if ((sock = user->sock) <= 0)
    return;

  if (!userno || !list_belong(user->ignore, userno))
  {
    fd_set wset;
    char buf[256];

    FD_ZERO(&wset);
    FD_SET(sock, &wset);

    if (user->clitype)
    {
      if (*msg)
	sprintf(buf, "%3d %s", number, msg);
      else
	sprintf(buf, "%3d", number);
      msg = buf;
    }

    do_send(sock, &wset, msg);
  }
}


/* ----------------------------------------------------- */


static void
room_changed(room)
  ChatRoom *room;
{
  if (room)
  {
    char buf[256];

    sprintf(buf, "= %s %d %d %s",
      room->name, room->occupants, room->rflag, room->topic);
    send_to_room(ROOM_ALL, buf, 0, MSG_ROOMNOTIFY);
  }
}


static void
user_changed(cu)
  ChatUser *cu;
{
  if (cu)
  {
    ChatRoom *room;
    char buf[256];

    room = cu->room;
    sprintf(buf, "= %s %s %s %s%s",
      cu->userid, cu->chatid, room->name, cu->rhost,
      ROOMOP(cu) ? " Op" : "");
    send_to_room(room, buf, 0, MSG_USERNOTIFY);
  }
}


static void
exit_room(user, mode, msg)
  ChatUser *user;
  int mode;
  char *msg;
{
  ChatRoom *room;
  char buf[128];

  if (!(room = user->room))
    return;

  user->room = NULL;
  /* user->uflag &= ~(PERM_ROOMOP | PERM_SYSOP | PERM_CHATROOM); */
  user->uflag &= ~PERM_ROOMOP;
  /* Thor.980601: 離開房間時只清 room op, 不清 sysop,chatroom,因天生具有 */

  if (room->occupants -= (CLOAK(user)) ? 0 : 1 )
  {
    char *chatid;

    chatid = user->chatid;
    switch (mode)
    {
    case EXIT_LOGOUT:

      sprintf(buf, "◆ %s 離開了 ...", chatid);
      if (msg && *msg)
      {
	strcat(buf, ": ");

	msg[79] = 0;		/* Thor:防止太長 */

	strncat(buf, msg, 79);
      }
      break;

    case EXIT_LOSTCONN:

      sprintf(buf, "◆ %s 成了斷線的風箏囉", chatid);
      break;

    case EXIT_KICK:

      sprintf(buf, "◆ 哈哈！%s 被踢出去了", chatid);
      break;
    }

    if (!CLOAK(user))
      send_to_room(room, buf, 0, MSG_MESSAGE);

    sprintf(buf, "- %s", user->userid);
    send_to_room(room, buf, 0, MSG_USERNOTIFY);
/*    room->occupants += CLOAK(user) ? 1 : 0 ;*/
    room_changed(room);
  }
  else if (room != &mainroom)
  {
    ChatRoom *next;

    fprintf(flog, "room-\t[%d] %s\n", user->sno, room->name);
    sprintf(buf, "- %s", room->name);

    room->prev->next = next = room->next;
    if (next)
      next->prev = room->prev;

    list_free(&room->invite);

    /* free(room); */

    /* 回收 */
    room->next = roompool;
    roompool = room;

    send_to_room(ROOM_ALL, buf, 0, MSG_ROOMNOTIFY);
  }
}


/* ----------------------------------------------------- */
/* chat commands                                         */
/* ----------------------------------------------------- */


#ifndef STAND_ALONE
/* ----------------------------------------------------- */
/* BBS server side routines                              */
/* ----------------------------------------------------- */

/* Thor.990211: 統一用 dao library */
/* static */
int
acct_load(acct, userid)
  ACCT *acct;
  char *userid;
{
  int fd;

  usr_fpath((char *) acct, userid, FN_ACCT);
  fd = open((char *) acct, O_RDONLY);
  if (fd >= 0)
  {
    read(fd, acct, sizeof(ACCT));
    close(fd);
  }
  return fd;
}


/* Thor.990211: 統一用 dao library */

static void
chat_query(cu, msg)
  ChatUser *cu;
  char *msg;
{
  FILE *fp;
  ACCT acct;
  char buf[256];

  /* Thor.980617: 可先查是否為空字串 */
  if (*msg && acct_load(&acct, msg) >= 0)
  {
    sprintf(buf, "%s(%s) 共上站 %d 次，文章 %d 篇",
      acct.userid, acct.username, acct.numlogins, acct.numposts);
    send_to_user(cu, buf, 0, MSG_MESSAGE);

    sprintf(buf, "最近(%s)從(%s)上站", Ctime(&acct.lastlogin),
      (acct.lasthost[0] ? acct.lasthost : "外太空"));
    send_to_user(cu, buf, 0, MSG_MESSAGE);

    usr_fpath(buf, acct.userid, FN_PLANS);
    if ((fp = fopen(buf, "r")))
    {
      int i;

      i = 0;
      while (fgets(buf, 255, fp) && buf[0])
      {
	buf[strlen(buf) - 1] = 0;
	send_to_user(cu, buf, 0, MSG_MESSAGE);
	if (++i >= MAXQUERYLINES)
	  break;
      }
      fclose(fp);
    }
  }
  else
  {
    sprintf(buf, msg_no_such_id, msg);
    send_to_user(cu, buf, 0, MSG_MESSAGE);
  }
}
#endif


static void
chat_clear(cu, msg)
  ChatUser *cu;
  char *msg;
{
  if (cu->clitype)
    send_to_user(cu, "", 0, MSG_CLRSCR);
  else
    send_to_user(cu, "/c", 0, MSG_MESSAGE);
}


static void
chat_date(cu, msg)
  ChatUser *cu;
  char *msg;
{
  time_t thetime;
  char buf[128];

  time(&thetime);
  sprintf(buf, "◆ 標準時間: %s", Ctime(&thetime));
  send_to_user(cu, buf, 0, MSG_MESSAGE);
}


static void
chat_mud(cu, msg)
  ChatUser *cu;
  char *msg;
{
  ChatRoom *room;

  room = cu->room;  

  if (!ROOMOP(cu))
  {
    return;
  }
  mudshm_init();  
  send_to_room(room,  "※ 更新" CHATROOMNAME "動態詞", 0, MSG_MESSAGE);

}

static void
chat_topic(cu, msg)
  ChatUser *cu;
  char *msg;
{
  ChatRoom *room;
  char *topic, buf[128];

  room = cu->room;

  if (!ROOMOP(cu) && !OPENTOPIC(room))
  {
    send_to_user(cu, msg_not_op, 0, MSG_MESSAGE);
    return;
  }
  
  if(strstr(msg,"\\033"))
  {
    send_to_user(cu, "※ 不合格的話題", 0, MSG_MESSAGE);
    return;
  }
  if (*msg == '\0')
  {
    send_to_user(cu, "※ 請指定話題", 0, MSG_MESSAGE);
    return;
  }

  topic = room->topic;
  /* str_ncpy(topic, msg, sizeof(room->topic) - 1); */
  str_ncpy(topic, msg, sizeof(room->topic));
  /* Thor.980921: str_ncpy 已含 0之空間 */

  if (cu->clitype)
  {
    send_to_room(room, topic, 0, MSG_TOPIC);
  }
  else
  {
    sprintf(buf, "/t%s", topic);
    send_to_room(room, buf, 0, MSG_MESSAGE);
  }

  room_changed(room);

  if (!CLOAK(cu))
  {
    sprintf(buf, "◆ %s 將話題改為 \x1b[1;32m%s\x1b[m", cu->chatid, topic);
    send_to_room(room, buf, 0, MSG_MESSAGE);
  }
}


static void
chat_version(cu, msg)
  ChatUser *cu;
  char *msg;
{
  char buf[80];

  sprintf(buf, "%d %d", XCHAT_VERSION_MAJOR, XCHAT_VERSION_MINOR);
  send_to_user(cu, buf, 0, MSG_VERSION);
}


static void
chat_nick(cu, msg)
  ChatUser *cu;
  char *msg;
{
  char *chatid, *str, buf[128];
  ChatUser *xuser;

  chatid = nextword(&msg);
  chatid[8] = '\0';
  if (!valid_chatid(chatid) || strstr(chatid,"\\033"))
  {
    send_to_user(cu, "※ 這個聊天代號是不正確的", 0, MSG_MESSAGE);
    return;
  }

  xuser = cuser_by_chatid(chatid);
  if (xuser != NULL && xuser != cu)
  {
    send_to_user(cu, "※ 已經有人捷足先登囉", 0, MSG_MESSAGE);
    return;
  }

  str = cu->chatid;

  if (!CLOAK(cu))
  {
    sprintf(buf, "※ %s 將聊天代號改為 \x1b[1;33m%s\x1b[m", str, chatid);
    send_to_room(cu->room, buf, cu->userno, MSG_MESSAGE);
  }

  strcpy(str, chatid);

  user_changed(cu);

  if (cu->clitype)
  {
    send_to_user(cu, chatid, 0, MSG_NICK);
  }
  else
  {
    sprintf(buf, "/n%s", chatid);
    send_to_user(cu, buf, 0, MSG_MESSAGE);
  }
}


static void
chat_list_rooms(cuser, msg)
  ChatUser *cuser;
  char *msg;
{
  ChatRoom *cr, *room;
  char buf[128],from[128];
  int mode;

  if (RESTRICTED(cuser))
  {
    send_to_user(cuser, "※ 您沒有權限列出現有的" CHATROOMNAME, 0, MSG_MESSAGE);
    return;
  }

  mode = common_client_command;

  if (mode)
    send_to_user(cuser, "", 0, MSG_ROOMLISTSTART);
  else
    send_to_user(cuser, "\x1b[7m  聊天室名稱  │人數│話題        \x1b[m", 0,
      MSG_MESSAGE);

  room = cuser->room;
  cr = &mainroom;

  do
  {
    if ((cr == room) || !SECRET(cr) || CHATSYSOP(cuser))
    {
      if (mode)
      {
	sprintf(buf, "%s %d %d %s",
	  cr->name, cr->occupants, cr->rflag, cr->topic);
	send_to_user(cuser, buf, 0, MSG_ROOMLIST);
      }
      else
      {
        sprintf(from,"%%-%ds│%%4d│%%s",strlen(CHATROOMNAME)-6+12);
	sprintf(buf, from, cr->name, cr->occupants, cr->topic);
	if (LOCKED(cr))
	  strcat(buf, " [鎖住]");
	if (SECRET(cr))
	  strcat(buf, " [秘密]");
	if (OPENTOPIC(cr))
	  strcat(buf, " [話題]");
	send_to_user(cuser, buf, 0, MSG_MESSAGE);
      }
    }
  } while ((cr = cr->next));

  if (mode)
    send_to_user(cuser, "", 0, MSG_ROOMLISTEND);
}


static void
chat_do_user_list(cu, msg, theroom)
  ChatUser *cu;
  char *msg;
  ChatRoom *theroom;
{
  ChatRoom *myroom, *room;
  ChatUser *user;
  int start, stop, curr, mode; /* , uflag; */
  char buf[128];

  curr = 0; /* Thor.980619: initialize curr */
  start = atoi(nextword(&msg));
  stop = atoi(nextword(&msg));

  mode = common_client_command;

  if (mode)
    send_to_user(cu, "", 0, MSG_USERLISTSTART);
  else
    send_to_user(cu, "\x1b[7m 聊天代號│使用者代號  │" CHATROOMNAME " \x1b[m", 0,
      MSG_MESSAGE);

  myroom = cu->room;

  /* Thor.980717: 需要先排除 cu->userno == 0 的狀況嗎? */
  for (user = mainuser; user; user = user->unext)
  {
    /*
    if (!cu->userno)
      continue;
     */ /* Thor.980717: 既然 cu 都空了那還進來幹麼? */
    if (!user->userno) 
      continue;

    room = user->room;
    if ((theroom != ROOM_ALL) && (theroom != room))
      continue;

#if 0
    uflag = user->uflag;
    if ((myroom != room) && (!uflag ||
	(room && SECRET(room) && !(uflag & (PERM_SYSOP | PERM_CHATROOM)))))
      continue;

    if ((uflag & PERM_CLOAK) && (user != cu) &&
      !(uflag & (PERM_SYSOP | PERM_CHATROOM)))
      continue;
#endif
    /* Thor.980717:上述logic有誤 */

    /* Thor.980717: viewer check */
    if ((myroom != room) && (RESTRICTED(cu) || 
                             (room && SECRET(room) && !CHATSYSOP(cu))))
      continue;

    /* Thor.980717: viewee check */
    if (CLOAK(user) && (user != cu) && !CHATSYSOP(cu))
      continue;
    /* Thor.980717:重寫*/


    curr++;
    if (start && curr < start)
      continue;
    else if (stop && (curr > stop))
      break;

    if (mode)
    {
      if (!room)
	continue;		/* Xshadow: 還沒進入任何房間的就不列出 */

      sprintf(buf, "%s %s %s %s",
	user->chatid, user->userid, room->name, user->rhost);
/* Thor.980603: PERM_CHATROOM改為 default 沒有 roomop, 但可以自己取得 */
/*       if (uflag & (PERM_ROOMOP | PERM_SYSOP | PERM_CHATROOM)) */
      if (ROOMOP(user))
	strcat(buf, " Op");
    }
    else
    {
      sprintf(buf, " %-8s│%-12s│%s",
	user->chatid, user->userid, room ? room->name : "[在門口徘徊]");
/* Thor.980603: PERM_CHATROOM改為 default 沒有 roomop, 但可以自己取得 */
/*       if (uflag & (PERM_ROOMOP | PERM_SYSOP | PERM_CHATROOM)) */
      /* if (uflag & (PERM_ROOMOP|PERM_CHATOP)) */
      if (ROOMOP(user))  /* Thor.980602: 統一用法 */
	strcat(buf, " [Op]");
    }

    send_to_user(cu, buf, 0, mode ? MSG_USERLIST : MSG_MESSAGE);
  }

  if (mode)
    send_to_user(cu, "", 0, MSG_USERLISTEND);
}


static void
chat_list_by_room(cu, msg)
  ChatUser *cu;
  char *msg;
{
  ChatRoom *whichroom;
  char *roomstr, buf[128];

  roomstr = nextword(&msg);
  if (!*roomstr)
  {
    whichroom = cu->room;
  }
  else
  {
    if (!(whichroom = croom_by_roomid(roomstr)))
    {
      sprintf(buf, "※ 沒有 [%s] 這個" CHATROOMNAME, roomstr);
      send_to_user(cu, buf, 0, MSG_MESSAGE);
      return;
    }

    if (whichroom != cu->room && SECRET(whichroom) && !CHATSYSOP(cu))
    {
      send_to_user(cu, "※ 無法列出在秘密" CHATROOMNAME "的使用者", 0, MSG_MESSAGE);
      return;
    }
  }
  chat_do_user_list(cu, msg, whichroom);
}


static void
chat_list_users(cu, msg)
  ChatUser *cu;
  char *msg;
{
  chat_do_user_list(cu, msg, ROOM_ALL);
}


static void
chat_chatroom(cu, msg)
  ChatUser *cu;
  char *msg;
{
  if (common_client_command)
    send_to_user(cu, NICKNAME CHATROOMNAME, 0, MSG_CHATROOM);
}


static void
chat_map_chatids(cu, whichroom)
  ChatUser *cu;			/* Thor: 還沒有作不同間的 */
  ChatRoom *whichroom;
{
  int c;
  ChatRoom *myroom, *room;
  ChatUser *user;
  char buf[128];

  myroom = cu->room;

  send_to_user(cu,
    "\x1b[7m 聊天代號 使用者代號  │ 聊天代號 使用者代號  │ 聊天代號 使用者代號 \x1b[m", 0, MSG_MESSAGE);

  for (c = 0, user = mainuser; user; user = user->unext)
  {
    if (!cu->userno)
      continue;

    room = user->room;
    if (whichroom != ROOM_ALL && whichroom != room)
      continue;

    if (myroom != room)
    {
      if (RESTRICTED(cu) ||	/* Thor: 要先check room 是不是空的 */
	(room && SECRET(room) && !CHATSYSOP(cu)))
	continue;
    }

    if (CLOAK(user) && (user != cu) && !CHATSYSOP(cu))	/* Thor:隱身術 */
      continue;

    sprintf(buf + (c * 24), " %-8s%c%-12s%s",
      user->chatid, ROOMOP(user) ? '*' : ' ',
      user->userid, (c < 2 ? "│" : "  "));

    if (++c == 3)
    {
      send_to_user(cu, buf, 0, MSG_MESSAGE);
      c = 0;
    }
  }

  if (c > 0)
    send_to_user(cu, buf, 0, MSG_MESSAGE);
}


static void
chat_map_chatids_thisroom(cu, msg)
  ChatUser *cu;
  char *msg;
{
  chat_map_chatids(cu, cu->room);
}


static void
chat_setroom(cu, msg)
  ChatUser *cu;
  char *msg;
{
  char *modestr;
  ChatRoom *room;
  char *chatid;
  int sign, flag;
  char *fstr=NULL, buf[128];

  if (!ROOMOP(cu))
  {
    send_to_user(cu, msg_not_op, 0, MSG_MESSAGE);
    return;
  }

  modestr = nextword(&msg);
  sign = 1;
  if (*modestr == '+')
  {
    modestr++;
  }
  else if (*modestr == '-')
  {
    modestr++;
    sign = 0;
  }

  if (*modestr == '\0')
  {
    send_to_user(cu,
      "※ 請指定狀態: {[+(設定)][-(取消)]}{[L(鎖住)][s(秘密)][t(開放話題)}", 0, MSG_MESSAGE);
    return;
  }

  room = cu->room;
  chatid = cu->chatid;

  while (*modestr)
  {
    flag = 0;
    switch (*modestr)
    {
    case 'l':
    case 'L':
      flag = ROOM_LOCKED;
      fstr = "鎖住";
      break;

    case 's':
    case 'S':
      flag = ROOM_SECRET;
      fstr = "秘密";
      break;

    case 't':
    case 'T':
      flag = ROOM_OPENTOPIC;
      fstr = "開放話題";
      break;

    default:
      sprintf(buf, "※ 狀態錯誤：[%c]", *modestr);
      send_to_user(cu, buf, 0, MSG_MESSAGE);
    }

    /* Thor: check room 是不是空的, 應該不是空的 */

    if (flag && (room->rflag & flag) != sign * flag)
    {
      room->rflag ^= flag;

      if (!CLOAK(cu))
      {
	sprintf(buf, "※ 本" CHATROOMNAME "被 %s %s [%s] 狀態",
	  chatid, sign ? "設定為" : "取消", fstr);
	send_to_room(room, buf, 0, MSG_MESSAGE);
      }
    }
    modestr++;
  }

  /* Thor.980602: 不准 Main room 鎖起 or 秘密, 這成何體統? 
                  離開的就進不來, 要看也看不到?
                  想要踢人也踢不進 main room, 不會很奇怪嗎? */
  if(str_equal(MAIN_NAME, room->name))
  {
    if(room->rflag & (ROOM_LOCKED | ROOM_SECRET))
    {
        send_to_room(room, "※ 但 " SYSOPNICK " 施了『復原』的魔法", 0, MSG_MESSAGE);
        room->rflag &= ~(ROOM_LOCKED|ROOM_SECRET);
    }
  }

  room_changed(room);
}


static char *chat_msg[] =
{
  "[//]help", "MUD-like 社交動詞",
  "[/h]elp op", CHATROOMNAME "管理員專用指令",
  "[/a]ct <msg>", "做一個動作",
  "[/b]ye [msg]", "道別",
  "[/c]lear  [/d]ate", "清除螢幕  目前時間",
  /* "[/d]ate", "目前時間", *//* Thor: 指令太多 */

#if 0
  "[/f]ire <user> <msg>", "發送熱訊",	/* Thor.0727: 和 flag 衝key */
#endif

  "[/i]gnore [user]", "忽略使用者",
  "[/j]oin <room>", "建立或加入" CHATROOMNAME,
  "[/l]ist [start [stop]]", "列出" CHATROOMNAME "使用者",
  "[/m]sg <id|user> <msg>", "跟 <id> 說悄悄話",
  "[/n]ick <id>", "將談天代號換成 <id>",
  "[/p]ager", "切換呼叫器",
  "[/q]uery <user>", "查詢網友",
  "[/r]oom", "列出一般" CHATROOMNAME,
  "[/t]ape", "開關錄音機",
  "[/u]nignore <user>", "取消忽略",

#if 0
  "[/u]sers", "列出站上使用者",
#endif

  "[/w]ho", "列出本" CHATROOMNAME "使用者",
  "[/w]hoin <room>", "列出" CHATROOMNAME "<room> 的使用者",
  NULL
};


static char *room_msg[] =
{
  "[/f]lag [+-][lst]", "設定鎖定、秘密、開放話題",
  "[/i]nvite <id>", "邀請 <id> 加入" CHATROOMNAME,
  "[/kick] <id>", "將 <id> 踢出" CHATROOMNAME,
  "[/o]p [<id>]", "將 Op 的權力轉移給 <id>",
  "[/topic] <text>", "換個話題",
  "[/mud]","更新" CHATROOMNAME "動態詞",
  "[/w]all", "廣播 (站長專用)",
  NULL
};


static void
chat_help(cu, msg)
  ChatUser *cu;
  char *msg;
{
  char **table, *str, buf[128];

  if (str_equal("op", nextword(&msg)))
  {
    send_to_user(cu, CHATROOMNAME "管理員專用指令", 0, MSG_MESSAGE);
    table = room_msg;
  }
  else
  {
    table = chat_msg;
  }

  while ((str = *table++))
  {
    sprintf(buf, "  %-20s- %s", str, *table++);
    send_to_user(cu, buf, 0, MSG_MESSAGE);
  }
}


static void
chat_private(cu, msg)
  ChatUser *cu;
  char *msg;
{
  ChatUser *xuser;
  int userno;
  char *recipient, buf[128];

  userno = 0;
  recipient = nextword(&msg);
  xuser = (ChatUser *) fuzzy_cuser_by_chatid(recipient);
  if (xuser == NULL)		/* Thor.0724: 用 userid也可傳悄悄話 */
  {
    xuser = cuser_by_userid(recipient);
  }

  if (xuser == NULL)
  {
    sprintf(buf, msg_no_such_id, recipient);
  }
  else if (xuser == FUZZY_USER)
  {				/* ambiguous */
    strcpy(buf, "※ 請指明聊天代號");
  }
  else if (*msg)
  {
    userno = cu->userno;
    sprintf(buf, "\x1b[1m*%s*\x1b[m ", cu->chatid);
    msg[79] = 0;		/* Thor:防止太長 */
    strncat(buf, msg, 80);
    send_to_user(xuser, buf, userno, MSG_MESSAGE);

    if (xuser->clitype)
    {				/* Xshadow: 如果對方是用 client 上來的 */
      sprintf(buf, "%s %s ", cu->userid, cu->chatid);
      msg[79] = 0;
      strncat(buf, msg, 80);
      send_to_user(xuser, buf, userno, MSG_PRIVMSG);
    }

    if (cu->clitype)
    {
      sprintf(buf, "%s %s ", xuser->userid, xuser->chatid);
      msg[79] = 0;
      strncat(buf, msg, 80);
      send_to_user(cu, buf, 0, MSG_MYPRIVMSG);
    }

    sprintf(buf, "%s> ", xuser->chatid);
    strncat(buf, msg, 80);
  }
  else
  {
    sprintf(buf, "※ 您想對 %s 說什麼話呢？", xuser->chatid);
  }

  send_to_user(cu, buf, userno, MSG_MESSAGE);

  /* Thor: userno 要改成 0 嗎? */
}


static void
chat_cloak(cu, msg)
  ChatUser *cu;
  char *msg;
{
  if (CHATSYSOP(cu))
  {
    char buf[128];
    ChatRoom *room;
    

    cu->uflag ^= PERM_CLOAK;
    room = cu->room;
    room->occupants += CLOAK(cu) ? -1 : 1 ;
    sprintf(buf, "◆ %s", CLOAK(cu) ? MSG_CLOAKED : MSG_UNCLOAK);
    send_to_user(cu, buf, 0, MSG_MESSAGE);
  }
}


/* ----------------------------------------------------- */


static void
arrive_room(cuser, room)
  ChatUser *cuser;
  ChatRoom *room;
{
  char *rname, buf[256];

  /* Xshadow: 不必送給自己, 反正換房間就會重新 build user list */

  sprintf(buf, "+ %s %s %s %s",
    cuser->userid, cuser->chatid, room->name, cuser->rhost);
  if (ROOMOP(cuser))
    strcat(buf, " Op");
  send_to_room(room, buf, 0, MSG_USERNOTIFY);

  if(!CLOAK(cuser))
    room->occupants++;
  room_changed(room);

  cuser->room = room;
  rname = room->name;

  if (cuser->clitype)
  {
    send_to_user(cuser, rname, 0, MSG_ROOM);
    send_to_user(cuser, room->topic, 0, MSG_TOPIC);
  }
  else
  {
    sprintf(buf, "/r%s", rname);
    send_to_user(cuser, buf, 0, MSG_MESSAGE);
    sprintf(buf, "/t%s", room->topic);
    send_to_user(cuser, buf, 0, MSG_MESSAGE);
  }

  sprintf(buf, "※ \x1b[32;1m%s\x1b[m 進入 \x1b[33;1m[%s]\x1b[m 包廂",
    cuser->chatid, rname);

  if (!CLOAK(cuser))
    send_to_room(room, buf, cuser->userno, MSG_MESSAGE);
  else
    send_to_user(cuser, buf, 0, MSG_MESSAGE);
}


static int
enter_room(cuser, rname, msg)
  ChatUser *cuser;
  char *rname;
  char *msg;
{
  ChatRoom *room;
  int create;
  char buf[256];

  create = 0;
  room = croom_by_roomid(rname);

  if (room == NULL)
  {
    /* new room */

#ifdef DEBUG
    logit(cuser->userid, "create new room");
    debug_room();
#endif

    if ((room = roompool))
    {
      roompool = room->next;
    }
    else
    {
      room = (ChatRoom *) malloc(sizeof(ChatRoom));
    }

    if (room == NULL)
    {
      send_to_user(cuser, "※ 無法再新闢包廂了", 0, MSG_MESSAGE);
      return 0;
    }

    memset(room, 0, sizeof(ChatRoom));
    /* str_ncpy(room->name, rname, IDLEN - 1); */
    str_ncpy(room->name, rname, IDLEN);
    /* Thor.980921: 已包含 0 */
    strcpy(room->topic, "這是一個新天地");

    sprintf(buf, "+ %s 1 0 %s", room->name, room->topic);
    send_to_room(ROOM_ALL, buf, 0, MSG_ROOMNOTIFY);

    if (mainroom.next)
      mainroom.next->prev = room;
    room->next = mainroom.next;

    mainroom.next = room;
    room->prev = &mainroom;

#ifdef DEBUG
    logit(cuser->userid, "create room succeed");
    debug_room();
#endif

    create = 1;
    fprintf(flog, "room+\t[%d] %s\n", cuser->sno, rname);
  }
  else
  {
    if (cuser->room == room)
    {
      sprintf(buf, "※ 您本來就在 [%s] " CHATROOMNAME "囉 :)", rname);
      send_to_user(cuser, buf, 0, MSG_MESSAGE);
      return 0;
    }

    if (!CHATSYSOP(cuser) && LOCKED(room) &&
      !list_belong(room->invite, cuser->userno))
    {
      send_to_user(cuser, "※ 內有惡犬，非請莫入", 0, MSG_MESSAGE);
      return 0;
    }
  }

  exit_room(cuser, EXIT_LOGOUT, msg);
  arrive_room(cuser, room);

  if (create)
    cuser->uflag |= PERM_ROOMOP;

  return 0;
}


static void
cuser_free(cuser)
  ChatUser *cuser;
{
  int sock;

  sock = cuser->sock;
  shutdown(sock, 2);
  close(sock);

  FD_CLR(sock, &mainfset);

  list_free(&cuser->ignore);
  totaluser--;

  if (cuser->room)
  {
    exit_room(cuser, EXIT_LOSTCONN, NULL);
  }
  fprintf(flog, "BYE\t[%d] T%d X%d\n",
    cuser->sno, (int) (time(0) - cuser->tbegin), cuser->xdata);
}


static void
print_user_counts(cuser)
  ChatUser *cuser;
{
  ChatRoom *room;
  int num, userc, suserc, roomc, number;
  char buf[256];

  userc = suserc = roomc = 0;

  room = &mainroom;
  do
  {
    num = room->occupants;
    if (SECRET(room))
    {
      suserc += num;
      if (CHATSYSOP(cuser))
	roomc++;
    }
    else
    {
      userc += num;
      roomc++;
    }
  } while ((room = room->next));

  number = (cuser->clitype) ? MSG_MOTD : MSG_MESSAGE;

  sprintf(buf, "⊙ 歡迎光臨【" NICKNAME CHATROOMNAME "】，按 \x1b[1;33m/h\x1b[m 顯示說明，按 \x1b[1;33m/b\x1b[m 離開");
  send_to_user(cuser, buf, 0, number);

  sprintf(buf, "⊙ 目前開了 \x1b[1;31m%d\x1b[m 間包廂，共有 \x1b[1;36m%d\x1b[m 人來擺\龍門陣", roomc, userc);
  if (suserc)
    sprintf(buf + strlen(buf), " [ %d 人在秘密包廂]", suserc);
  send_to_user(cuser, buf, 0, number);

//  load_mud_like();
}


static int
login_user(cu, msg)
  ChatUser *cu;
  char *msg;
{
  int utent;

  char *userid;
  char *chatid, *passwd;
  ChatUser *xuser;
  int level;
/*   struct hostent *hp; */

#ifndef STAND_ALONE
  ACCT acct;
#endif

  /* Xshadow.0915: common client support : /-! userid chatid password */
  /* client/server 版本依據 userid 抓 .PASSWDS 判斷 userlevel */

  userid = nextword(&msg);
  chatid = nextword(&msg);

#ifdef	DEBUG
  logit("ENTER", userid);
#endif

#ifndef STAND_ALONE
  /* Thor.0730: parse space before passwd */

  passwd = msg;

  /* Thor.0813: 跳過一空格即可, 因為反正如果chatid有空格, 密碼也不對 */
  /* 就算密碼對, 也不會怎麼樣:p */
  /* 可是如果密碼第一個字是空格, 那跳太多空格會進不來... */
  /* Thor.980910: 由於 nextword修改為後接空格填0, 傳入值則直接後移至0後,
                  所以不需作此動作 */
#if 0
  if (*passwd == ' ')
    passwd++;
#endif

  /* Thor.0729: load acct */

  if (!*userid || (acct_load(&acct, userid) < 0))
  {

#ifdef	DEBUG
    logit("noexist", userid);
#endif

    if (cu->clitype)
      send_to_user(cu, "錯誤的使用者代號", 0, ERR_LOGIN_NOSUCHUSER);
    else
      send_to_user(cu, CHAT_LOGIN_INVALID, 0, MSG_MESSAGE);

    return -1;
  }

  /* Thor.0813: 改用真實 password check, for C/S bbs */

  /* Thor.990214: 注意,daolib中 非0代表失敗 */
  /* if (!chkpasswd(acct.passwd, passwd)) */
  if (chkpasswd(acct.passwd, passwd))
  {

#ifdef	DEBUG
    logit("fake", userid);
#endif

    if (cu->clitype)
      send_to_user(cu, "密碼錯誤", 0, ERR_LOGIN_PASSERROR);
    else
      send_to_user(cu, CHAT_LOGIN_INVALID, 0, MSG_MESSAGE);

    return -1;
  }

  level = acct.userlevel;
  utent = acct.userno;

#else				/* STAND_ALONE */
  level = 1;
  utent = ++userno_inc;
#endif				/* STAND_ALONE */

  /* Thor.0819: for client/server bbs */

#ifdef DEBUG
  log_user(NULL);
#endif

  for (xuser = mainuser; xuser; xuser = xuser->unext)
  {

#ifdef DEBUG
    log_user(xuser);
#endif

    if (xuser->userno == utent)
    {

#ifdef	DEBUG
      logit("enter", "bogus");
#endif

      if (cu->clitype)
	send_to_user(cu, "請勿派遣分身進入" CHATROOMNAME " !!", 0,
	  ERR_LOGIN_USERONLINE);
      else
	send_to_user(cu, CHAT_LOGIN_BOGUS, 0, MSG_MESSAGE);
      return -1;		/* Thor: 或是0等它自己了斷? */
    }
  }


#ifndef STAND_ALONE
  /* Thor.980629: 暫時借用 invalid_chatid 濾除 沒有PERM_CHAT的人 */
               
  if (!valid_chatid(chatid) || !(level & PERM_CHAT) || (level & PERM_DENYCHAT))
  { /* Thor.981012: 徹底一些, 連 denychat也BAN掉, 免得 client作怪 */

#ifdef	DEBUG
    logit("enter", chatid);
#endif

    if (cu->clitype)
      send_to_user(cu, "不合法的" CHATROOMNAME "代號 !!", 0, ERR_LOGIN_NICKERROR);
    else
      send_to_user(cu, CHAT_LOGIN_INVALID, 0, MSG_MESSAGE);
    return 0;
  }
#endif

#ifdef	DEBUG
  debug_user();
#endif

  if (cuser_by_chatid(chatid) != NULL)
  {
    /* chatid in use */

#ifdef	DEBUG
    logit("enter", "duplicate");
#endif

    if (cu->clitype)
      send_to_user(cu, "這個代號已經有人使用", 0, ERR_LOGIN_NICKINUSE);
    else
      send_to_user(cu, CHAT_LOGIN_EXISTS, 0, MSG_MESSAGE);
    return 0;
  }

#ifdef DEBUG			/* CHATSYSOP 一進來就隱身 */
  cu->uflag = level & ~(PERM_ROOMOP | PERM_CHATOP | (CHATSYSOP(cu) ? 0 : PERM_CLOAK));
#else
  cu->uflag = level & ~(PERM_ROOMOP | PERM_CHATOP | PERM_CLOAK);
#endif

  /* Thor: 進來先清空 ROOMOP (同PERM_CHAT) */

  strcpy(cu->userid, userid);
  str_ncpy(cu->chatid, chatid, sizeof(cu->chatid));
  /* Thor.980921: str_ncpy與一般 strncpy有所不同, 特別注意 */

  /* Thor.980921: 防止太長 */
  /* cu->chatid[8]=0; */

  fprintf(flog, "ENTER\t[%d] %s\n", cu->sno, userid);

  /* Xshadow: 取得 client 的來源 */

  dns_name(cu->rhost, cu->ibuf);
  str_ncpy(cu->rhost, cu->ibuf, sizeof(cu->rhost));
#if 0
  hp = gethostbyaddr(cu->rhost, sizeof(struct in_addr), AF_INET);
  str_ncpy(cu->rhost,
    hp ? hp->h_name : inet_ntoa((struct in_addr *) cu->rhost),
    sizeof(cu->rhost) - 1);
#endif

  cu->userno = utent;

  if (cu->clitype)
    send_to_user(cu, "順利", 0, MSG_LOGINOK);
  else
    send_to_user(cu, CHAT_LOGIN_OK, 0, MSG_MESSAGE);

  arrive_room(cu, &mainroom);

  send_to_user(cu, "", 0, MSG_MOTDSTART);
  print_user_counts(cu);
  send_to_user(cu, "", 0, MSG_MOTDEND);

#ifdef	DEBUG
  logit("enter", "OK");
#endif

  return 0;
}


static void
chat_act(cu, msg)
  ChatUser *cu;
  char *msg;
{
  if (*msg)
  {
    char buf[256];

    sprintf(buf, "%s \x1b[36m%s\x1b[m", cu->chatid, msg);
    send_to_room(cu->room, buf, cu->userno, MSG_MESSAGE);
  }
}


static void
chat_ignore(cu, msg)
  ChatUser *cu;
  char *msg;
{
  char *str, buf[256];

  if (RESTRICTED(cu))
  {
    str = "※ 您沒有 ignore 別人的權利";
  }
  else
  {
    char *ignoree;

    str = buf;
    ignoree = nextword(&msg);
    if (*ignoree)
    {
      ChatUser *xuser;

      xuser = cuser_by_userid(ignoree);

      if (xuser == NULL)
      {
	sprintf(str, msg_no_such_id, ignoree);
      }
      else if (xuser == cu || CHATSYSOP(xuser) ||
	(ROOMOP(xuser) && (xuser->room == cu->room)))
      {
	sprintf(str, "◆ 不可以 ignore [%s]", ignoree);
      }
      else
      {
	if (list_belong(cu->ignore, xuser->userno))
	{
	  sprintf(str, "※ %s 已經被凍結了", xuser->chatid);
	}
	else
	{
	  list_add(&(cu->ignore), xuser);
	  sprintf(str, "◆ 將 [%s] 打入冷宮了 :p", xuser->chatid);
	}
      }
    }
    else
    {
      UserList *list;

      if ((list = cu->ignore))
      {
	int len;
	char userid[16];

	send_to_user(cu, "◆ 這些人被打入冷宮了：", 0, MSG_MESSAGE);
	len = 0;
	do
	{
	  sprintf(userid, "%-13s", list->userid);
	  strcpy(str + len, userid);
	  len += 13;
	  if (len >= 78)
	  {
	    send_to_user(cu, str, 0, MSG_MESSAGE);
	    len = 0;
	  }
	} while ((list = list->next));

	if (len == 0)
	  return;
      }
      else
      {
	str = "◆ 您目前並沒有 ignore 任何人";
      }
    }
  }

  send_to_user(cu, str, 0, MSG_MESSAGE);
}


static void
chat_unignore(cu, msg)
  ChatUser *cu;
  char *msg;
{
  char *ignoree, *str, buf[80];

  ignoree = nextword(&msg);

  if (*ignoree)
  {
    sprintf(str = buf, (list_delete(&(cu->ignore), ignoree)) ?
      "◆ [%s] 不再被你冷落了" :
      "◆ 您並未 ignore [%s] 這號人物", ignoree);
  }
  else
  {
    str = "◆ 請指明 user ID";
  }
  send_to_user(cu, str, 0, MSG_MESSAGE);
}


static void
chat_join(cu, msg)
  ChatUser *cu;
  char *msg;
{
  if (RESTRICTED(cu))
  {
    send_to_user(cu, "※ 您沒有加入其他" CHATROOMNAME "的權限", 0, MSG_MESSAGE);
  }
  else
  {
    char *roomid = nextword(&msg);

    if (*roomid)
      enter_room(cu, roomid, msg);
    else
      send_to_user(cu, "※ 請指定" CHATROOMNAME, 0, MSG_MESSAGE);
  }
}


static void
chat_kick(cu, msg)
  ChatUser *cu;
  char *msg;
{
  char *twit, buf[80];
  ChatUser *xuser;
  ChatRoom *room;

  if (!ROOMOP(cu))
  {
    send_to_user(cu, msg_not_op, 0, MSG_MESSAGE);
    return;
  }

  twit = nextword(&msg);
  xuser = cuser_by_chatid(twit);

  if (xuser == NULL)
  {                       /* Thor.980604: 用 userid也嘛通 */
    xuser = cuser_by_userid(twit);
  }
               
  if (xuser == NULL)
  {
    sprintf(buf, msg_no_such_id, twit);
    send_to_user(cu, buf, 0, MSG_MESSAGE);
    return;
  }

  room = cu->room;
  if (room != xuser->room || CLOAK(xuser))
  {
    sprintf(buf, msg_not_here, twit);
    send_to_user(cu, buf, 0, MSG_MESSAGE);
    return;
  }

  if (CHATSYSOP(xuser))
  {
    sprintf(buf, "◆ 不可以 kick [%s]", twit);
    send_to_user(cu, buf, 0, MSG_MESSAGE);
    return;
  }

  exit_room(xuser, EXIT_KICK, (char *) NULL);

  if (room == &mainroom)
    xuser->uptime = 0;		/* logout_user(xuser); */
  else
    enter_room(xuser, MAIN_NAME, (char *) NULL);  
    /* Thor.980602: 其實踢就踢,不要show出xxx離開了的訊息比較好 */
}

static void
chat_makeop(cu, msg)
  ChatUser *cu;
  char *msg;
{
  char *newop, buf[80];
  ChatUser *xuser;
  ChatRoom *room;

  /* Thor.980603: PERM_CHATROOM改為 default 沒有 roomop, 但可以自己取得 */

  newop = nextword(&msg);

  room = cu->room;

  if(!*newop && CHATSYSOP(cu))
  {
    /* Thor.980603: PERM_CHATROOM改為 default 沒有 roomop, 但可以自己取得 */
    cu->uflag ^= PERM_CHATOP;

    user_changed(cu);
    if (!CLOAK(cu))
    {
      sprintf(buf,ROOMOP(cu) ? "※ " SYSOPNICK " 將 Op 權力授予 %s"
                             : "※ " SYSOPNICK " 將 %s 的 Op 權力收回", cu->chatid);
      send_to_room(room, buf, 0, MSG_MESSAGE);
    }
    
    return;
  }

  /* if (!ROOMOP(cu)) */
  if (!(cu->uflag & (PERM_ROOMOP|PERM_CHATOP))) /* Thor.980603: chat room總管不能轉移 Op 權力 */
  {
    send_to_user(cu, "◆ 您不能轉移 Op 的權力" /* msg_not_op */, 0, MSG_MESSAGE);
    return;
  }

  xuser = cuser_by_chatid(newop);

#if 0
  if (xuser == NULL)
  {                       /* Thor.980604: 用 userid也嘛通 */
    xuser = cuser_by_userid(newop);
  }
#endif
               

  if (xuser == NULL)
  {
    sprintf(buf, msg_no_such_id, newop);
    send_to_user(cu, buf, 0, MSG_MESSAGE);
    return;
  }

  if (cu == xuser)
  {
    send_to_user(cu, "※ 您早就已經是 Op 了啊", 0, MSG_MESSAGE);
    return;
  }

  /* room = cu->room; */

  if (room != xuser->room || CLOAK(xuser))
  {
    sprintf(buf, msg_not_here, xuser->chatid);
    send_to_user(cu, buf, 0, MSG_MESSAGE);
    return;
  }

  cu->uflag &= ~PERM_ROOMOP;
  xuser->uflag |= PERM_ROOMOP;

  user_changed(cu);
  user_changed(xuser);

  if (!CLOAK(cu))
  {
    sprintf(buf, "※ %s 將 Op 權力轉移給 %s",
      cu->chatid, xuser->chatid);
    send_to_room(room, buf, 0, MSG_MESSAGE);
  }
}


static void
chat_invite(cu, msg)
  ChatUser *cu;
  char *msg;
{
  char *invitee, buf[80];
  ChatUser *xuser;
  ChatRoom *room;
  UserList **list;

  if (!ROOMOP(cu))
  {
    send_to_user(cu, msg_not_op, 0, MSG_MESSAGE);
    return;
  }

  invitee = nextword(&msg);
  xuser = cuser_by_chatid(invitee);

#if 0
  if (xuser == NULL)
  {                       /* Thor.980604: 用 userid也嘛通 */
    xuser = cuser_by_userid(invitee);
  }
#endif
               
  if (xuser == NULL)
  {
    sprintf(buf, msg_no_such_id, invitee);
    send_to_user(cu, buf, 0, MSG_MESSAGE);
    return;
  }

  room = cu->room;		/* Thor: 是否要 check room 是否 NULL ? */
  list = &(room->invite);

  if (list_belong(*list, xuser->userno))
  {
    sprintf(buf, "※ %s 已經接受過邀請了", xuser->chatid);
    send_to_user(cu, buf, 0, MSG_MESSAGE);
    return;
  }
  list_add(list, xuser);

  sprintf(buf, "※ %s 邀請您到 [%s] " CHATROOMNAME,
    cu->chatid, room->name);
  send_to_user(xuser, buf, 0, MSG_MESSAGE);
  sprintf(buf, "※ %s 收到您的邀請了", xuser->chatid);
  send_to_user(cu, buf, 0, MSG_MESSAGE);
}


static void
chat_broadcast(cu, msg)
  ChatUser *cu;
  char *msg;
{
  char buf[80];

  if (!CHATSYSOP(cu))
  {
    send_to_user(cu, "※ 您沒有在" CHATROOMNAME "廣播的權力!", 0, MSG_MESSAGE);
    return;
  }

  if (*msg == '\0')
  {
    send_to_user(cu, "※ 請指定廣播內容", 0, MSG_MESSAGE);
    return;
  }

  sprintf(buf, "\x1b[1m※ " BOARDNAME CHATROOMNAME "廣播中 [%s].....\x1b[m",
    cu->chatid);
  send_to_room(ROOM_ALL, buf, 0, MSG_MESSAGE);
  sprintf(buf, "◆ %s", msg);
  send_to_room(ROOM_ALL, buf, 0, MSG_MESSAGE);
}


static void
chat_bye(cu, msg)
  ChatUser *cu;
  char *msg;
{
  exit_room(cu, EXIT_LOGOUT, msg);
  cu->uptime = 0;
  /* logout_user(cu); */
}


/* --------------------------------------------- */
/* MUD-like social commands : action             */
/* --------------------------------------------- */

ChatAction *party_data;
ChatAction *party_data2;

static int
party_action(cu, cmd, party,mode)
  ChatUser *cu;
  char *cmd;
  char *party;
  int mode;
{
  ChatAction *cap;
  char *verb, buf[256];
  for (cap = mode ? party_data : party_data2 ; strlen(verb = cap->verb); cap++)
  {
    if (str_equal(verb, cmd))
    {
      if (*party == '\0')
      {
	party = "大家";
      }
      else
      {
	ChatUser *xuser;

	xuser = fuzzy_cuser_by_chatid(party);
	if (xuser == NULL)
	{			/* Thor.0724: 用 userid也嘛通 */
	  xuser = cuser_by_userid(party);
	}

	if (xuser == NULL)
	{
	  sprintf(buf, msg_no_such_id, party);
	  send_to_user(cu, buf, 0, MSG_MESSAGE);
	  return 0;
	}
	else if (xuser == FUZZY_USER)
	{
	  send_to_user(cu, "※ 請指明聊天代號", 0, MSG_MESSAGE);
	  return 0;
	}
	else if (cu->room != xuser->room || CLOAK(xuser))
	{
	  sprintf(buf, msg_not_here, party);
	  send_to_user(cu, buf, 0, MSG_MESSAGE);
	  return 0;
	}
	else
	{
	  party = xuser->chatid;
	}
      }
      sprintf(buf, "\x1b[1;32m%s \x1b[31m%s\x1b[33m %s \x1b[31m%s\x1b[m",
	cu->chatid, cap->part1_msg, party, cap->part2_msg);
      send_to_room(cu->room, buf, cu->userno, MSG_MESSAGE);
      return 0;			/* Thor: cu->room 是否為 NULL? */
    }
  }
  return 1;
}


/* --------------------------------------------- */
/* MUD-like social commands : speak              */
/* --------------------------------------------- */

ChatAction *speak_data;

static int
speak_action(cu, cmd, msg)
  ChatUser *cu;
  char *cmd;
  char *msg;
{
  ChatAction *cap;
  char *verb, buf[256];

  for (cap = speak_data; strlen(verb = cap->verb); cap++)
  {
    if (str_equal(verb, cmd))
    {
      sprintf(buf, "\x1b[1;32m%s \x1b[31m%s：\x1b[33m %s\x1b[m",
	cu->chatid, cap->part1_msg, msg);
      send_to_room(cu->room, buf, cu->userno, MSG_MESSAGE);
      return 0;
    }
  }
  return 1;
}


/* ----------------------------------------------------- */
/* MUD-like social commands : condition			 */
/* ----------------------------------------------------- */

ChatAction *condition_data,*person_data;

static int
condition_action(cu, cmd)
  ChatUser *cu;
  char *cmd;
{
  ChatAction *cap;
  char *verb, buf[256];

  for (cap = condition_data; strlen(verb = cap->verb); cap++)
  {
    if (str_equal(verb, cmd))
    {
      sprintf(buf, "\x1b[1;32m%s \x1b[31m%s\x1b[m",
	cu->chatid, cap->part1_msg);
      send_to_room(cu->room, buf, cu->userno, MSG_MESSAGE);
      return 0;
    }
  }
  return 1;
}

static int
person_action(cu, cmd,party)
  ChatUser *cu;
  char *cmd;
  char *party;
{
  ChatAction *cap;
  char *verb, buf[256];

  for (cap = person_data; strlen(verb = cap->verb); cap++)
  {
    if (str_equal(verb, cmd))
    {
      if(!*(cap->part2_msg))
      {
        sprintf(buf, "\x1b[1;32m%s \x1b[31m%s\x1b[m",
          cu->chatid, cap->part1_msg);
      }
      else
      {
    
        if (*party == '\0')
        {
          party = "大家";
        }
        else
        {
          ChatUser *xuser;

          xuser = fuzzy_cuser_by_chatid(party);
          if (xuser == NULL)
          {
            xuser = cuser_by_userid(party);
          }
 
          if (xuser == NULL)
          {
            sprintf(buf, msg_no_such_id, party);
            send_to_user(cu, buf, 0, MSG_MESSAGE);
            return 0;
          }
          else if (xuser == FUZZY_USER)
          {
            send_to_user(cu, "※ 請指明聊天代號", 0, MSG_MESSAGE);
            return 0;
          }
          else if (cu->room != xuser->room || CLOAK(xuser))
          {
            sprintf(buf, msg_not_here, party);
            send_to_user(cu, buf, 0, MSG_MESSAGE);
            return 1;
          }
          else
          {
            party = xuser->chatid;
          }
        }
        sprintf(buf, "\x1b[1;32m%s \x1b[31m%s\x1b[33m %s \x1b[31m%s\x1b[m",
          cu->chatid, cap->part1_msg, party, cap->part2_msg);    
      }
      send_to_room(cu->room, buf, cu->userno, MSG_MESSAGE);
      return 1;
    }
  }
  return 0;
}


/* --------------------------------------------- */
/* MUD-like social commands : help               */
/* --------------------------------------------- */


static char *dscrb[] =
{
  "\x1b[1;37m【 Verb + Nick：   動詞 + 對方名字 】\x1b[36m  例：//kick piggy\x1b[m",
  "\x1b[1;37m【 Verb + Message：動詞 + 要說的話 】\x1b[36m  例：//sing 天天天藍\x1b[m",
  "\x1b[1;37m【 Verb：動詞 】   ↑↓：舊話重提\x1b[m", 
  "\x1b[1;37m【 Verb + Nick：   動詞 + 對方名字 】\x1b[36m  例：//kick piggy\x1b[m",
  "\x1b[1;37m【 Verb：動詞 】   ↑↓：個人類動詞\x1b[m",
  NULL
};


static ChatAction *catbl(int in)
{
  if(in==0) return party_data;
  else if(in==1) return speak_data;
  else if(in==2) return condition_data;
  else if(in==3) return party_data2;
  else if(in==4) return person_data;
  else return NULL;
}

static void
chat_partyinfo(cu, msg)
  ChatUser *cu;
  char *msg;
{
  if (common_client_command)
  {
    send_to_user(cu, "3 動作  交談  狀態", 0, MSG_PARTYINFO);
  }
}


static void
chat_party(cu, msg)
  ChatUser *cu;
  char *msg;
{
  int kind, i;
  ChatAction *cap;
  char buf[80];

  if (!common_client_command)
    return;

  kind = atoi(nextword(&msg));
  if (kind < 0 || kind > 2)
    return;

  sprintf(buf, "%d\t%s", kind, kind == 2 ? "I" : "");

  /* Xshadow: 只有 condition 才是 immediate mode */
  send_to_user(cu, buf, 0, MSG_PARTYLISTSTART);

  cap = catbl(kind);
  for (i = 0; cap[i].verb; i++)
  {
    sprintf(buf, "%-10s %-20s", cap[i].verb, cap[i].chinese);
    send_to_user(cu, buf, 0, MSG_PARTYLIST);
  }

  sprintf(buf, "%d", kind);
  send_to_user(cu, buf, 0, MSG_PARTYLISTEND);
}


#define	SCREEN_WIDTH	80
#define	MAX_VERB_LEN	8
#define VERB_NO		10


static void
view_action_verb(cu, cmd)	/* Thor.0726: 新加動詞分類顯示 */
  ChatUser *cu;
  int cmd;
{
  int i;
  char *p, *q, *data, *expn, buf[256];
  ChatAction *cap;

  send_to_user(cu, "/c", 0, MSG_CLRSCR);

  data = buf;

  if (cmd < '1' || cmd > '5')
  {				/* Thor.0726: 寫得不好, 想辦法改進... */
    for (i = 0; (p = dscrb[i]); i++)
    {
      sprintf(data, "  [//]help %d          - MUD-like 社交動詞   第 %d 類", i + 1, i + 1);
      send_to_user(cu, data, 0, MSG_MESSAGE);
      send_to_user(cu, p, 0, MSG_MESSAGE);
      send_to_user(cu, " ", 0, MSG_MESSAGE);	/* Thor.0726: 換行 */
    }
  }
  else
  {
    i = cmd - '1';

    send_to_user(cu, dscrb[i], 0, MSG_MESSAGE);

    expn = buf + 100;		/* Thor.0726: 應該不會overlap吧? */

    *data = '\0';
    *expn = '\0';

    cap = catbl(i);

    for (i = 0; strlen(p = cap[i].verb); i++)
    {
      q = cap[i].chinese;

      strcat(data, p);
      strcat(expn, q);

      if (((i + 1) % VERB_NO) == 0)
      {
	send_to_user(cu, data, 0, MSG_MESSAGE);
	send_to_user(cu, expn, 0, MSG_MESSAGE);	/* Thor.0726: 顯示中文註解 */
	*data = '\0';
	*expn = '\0';
      }
      else
      {
	strncat(data, "        ", MAX_VERB_LEN - strlen(p));
	strncat(expn, "        ", MAX_VERB_LEN - strlen(q));
      }
    }

    if (i % VERB_NO)
    {
      send_to_user(cu, data, 0, MSG_MESSAGE);
      send_to_user(cu, expn, 0, MSG_MESSAGE);	/* Thor.0726: 顯示中文註解 */
    }
  }
  /* send_to_user(cu, " ",0); *//* Thor.0726: 換行, 需要 " " 嗎? */
}


/* ----------------------------------------------------- */
/* chat user service routines                            */
/* ----------------------------------------------------- */


static ChatCmd chatcmdlist[] =
{
  {"act", chat_act, 0},
  {"bye", chat_bye, 0},
  {"chatroom", chat_chatroom, 1},	/* Xshadow: for common client */
  {"clear", chat_clear, 0},
  {"cloak", chat_cloak, 2},
  {"mud",chat_mud,2},
  {"date", chat_date, 0},
  {"flags", chat_setroom, 0},
  {"help", chat_help, 0},
  {"ignore", chat_ignore, 1},
  {"invite", chat_invite, 0},
  {"join", chat_join, 0},
  {"kick", chat_kick, 1},
  {"msg", chat_private, 0},
  {"nick", chat_nick, 0},
  {"operator", chat_makeop, 0},
  {"party", chat_party, 1},	/* Xshadow: party data for common client */
  {"partyinfo", chat_partyinfo, 1},	/* Xshadow: party info for common
					 * client */

#ifndef STAND_ALONE
  {"query", chat_query, 0},
#endif

  {"room", chat_list_rooms, 0},
  {"unignore", chat_unignore, 1},
  {"whoin", chat_list_by_room, 1},
  {"wall", chat_broadcast, 2},

  {"who", chat_map_chatids_thisroom, 0},
  {"list", chat_list_users, 0},
  {"topic", chat_topic, 1},
  {"version", chat_version, 1},

  {NULL, NULL, 0}
};


/* Thor: 0 不用 exact, 1 要 exactly equal, 2 秘密指令 */


static int
command_execute(cu)
  ChatUser *cu;
{
  char *cmd, *msg, buf[128];
  /* Thor.981108: lkchu patch: chatid + msg 只用 80 bytes 不夠, 改為 128 */
  ChatCmd *cmdrec;
  int match, ch,check;

  msg = cu->ibuf;
  match = *msg;

  /* Validation routine */

  if (cu->room == NULL)
  {
    /* MUST give special /! or /-! command if not in the room yet */

    if (match == '/' && ((ch = msg[1]) == '!' || (ch == '-' && msg[2] == '!')))
    {
      if (ch == '-')
	fprintf(flog, "cli\t[%d] S%d\n", cu->sno, cu->sock);

      cu->clitype = (ch == '-') ? 1 : 0;
      return (login_user(cu, msg + 2 + cu->clitype));
    }
    else
    {
      return -1;
    }
  }

  /* If not a /-command, it goes to the room. */

  if (match != '/')
  {
    if (match)
    {
      if (cu->room && !CLOAK(cu))	/* 隱身的人也不能說話哦 */
      {
	char chatid[16];

	sprintf(chatid, "%s:", cu->chatid);
	sprintf(buf, "%-10s%s", chatid, msg);
	send_to_room(cu->room, buf, cu->userno, MSG_MESSAGE);
      }
    }
    return 0;
  }

  msg++;
  cmd = nextword(&msg);
  match = 0;
  check = 0;

  if (*cmd == '/')
    if(*(cmd+1) <= '5' && *(cmd+1) >= '1')
    {
      view_action_verb(cu, *(cmd+1));
      check = 1;
    }

  if (*cmd == '/'&&!(*(cmd+1) <= '5' && *(cmd+1) >= '1'))
  {
    cmd++;
    if (!*cmd || str_equal("help", cmd))
    {
      cmd = nextword(&msg);	/* Thor.0726: 動詞分類 */
      view_action_verb(cu, *cmd);
      match = 1;
    }
    else if (party_action(cu, cmd, msg,0) == 0)
      match = 1;
    else if (party_action(cu, cmd, msg,1) == 0)
      match = 1;
    else if (speak_action(cu, cmd, msg) == 0)
      match = 1;
    else if (condition_action(cu, cmd) == 0)
      match = 1;
    else
      match = person_action(cu, cmd, msg);
  }
  else
  {
    char *str;

    common_client_command = 0;
    if (*cmd == '-')
    {
      if (cu->clitype)
      {
	cmd++;			/* Xshadow: 指令從下一個字元才開始 */
	common_client_command = 1;
      }
      else
      {
	/* 不是 common client 但送出 common client 指令 -> 假裝沒看到 */
      }
    }

    str_lower(buf, cmd);

    for (cmdrec = chatcmdlist; (str = cmdrec->cmdstr); cmdrec++)
    {
      switch (cmdrec->exact)
      {
      case 1:			/* exactly equal */
	match = str_equal(str, buf);
	break;

      case 2:			/* Thor: secret command */
	if (CHATSYSOP(cu))
	  match = str_equal(str, buf);
	break;

      default:			/* not necessary equal */
	match = str_match(buf, str) >= 0;
	break;
      }

      if (match)
      {
	cmdrec->cmdfunc(cu, msg);
	break;
      }
    }
  }

  if (!match && !check)
  {
    sprintf(buf, "◆ 指令錯誤：/%s", cmd);
    send_to_user(cu, buf, 0, MSG_MESSAGE);
  }

  return 0;
}


/* ----------------------------------------------------- */
/* serve chat_user's connection                          */
/* ----------------------------------------------------- */


static int
cuser_serve(cu)
  ChatUser *cu;
{
  int ch, len, isize;
  char *str, *cmd, buf[256];
  char logbuf[256];

  str = buf;
  len = recv(cu->sock, str, sizeof(buf) - 1, 0);
  if (len < 0)
  {
    ch = errno;

    exit_room(cu, EXIT_LOSTCONN, NULL);
    logit("recv", strerror(ch));
    return -1;

#if 0
    if (ch != EWOULDBLOCK)
    {
      exit_room(cu, EXIT_LOSTCONN, NULL);
      logit("recv", sys_errlist[ch]);
      return -1;
    }

    return 0;			/* would block, so leave it to do later */
#endif
  }

  if (len == 0)
  {
    if (++cu->retry > 100)
      return -1;
    return 0;
  }

#if 1
  /* Xshadow: 將送達的資料忠實紀錄下來 */
  memcpy(logbuf, buf, sizeof(buf));
  for (ch = 0; ch < sizeof(buf); ch++)
    if (!logbuf[ch] || logbuf[ch] == '\r' || logbuf[ch] == '\n')
      logbuf[ch] = '$';

  logbuf[len + 1] = '\0';
  if(strncmp(logbuf,"/! ",3))
    logtalk(cu->userid, logbuf);
#endif

#if 0
  logit(cu->userid, str);
#endif

  cu->xdata += len;

  isize = cu->isize;
  cmd = cu->ibuf + isize;
  while (len--)
  {
    ch = *str++;

    if (ch == '\r' || !ch)
      continue;

    if (ch == '\n')
    {
      *cmd = '\0';

      if (command_execute(cu) < 0)
	return -1;

      isize = 0;
      cmd = cu->ibuf;

      continue;
    }

    if (isize < 79)
    {
      *cmd++ = ch;
      isize++;
    }
  }
  cu->isize = isize;
  return 1;
}


/* ----------------------------------------------------- */
/* chatroom server core routines                         */
/* ----------------------------------------------------- */


static int
/* start_daemon(mode) 
  int mode; */
servo_daemon(inetd)
  int inetd;
{
  int fd, value;
  char buf[80];
  struct sockaddr_in sin;
  struct linger ld;
#ifdef RLIMIT
  struct rlimit limit;
#endif //RLIMIT

  /*
   * More idiot speed-hacking --- the first time conversion makes the C
   * library open the files containing the locale definition and time zone.
   * If this hasn't happened in the parent process, it happens in the
   * children, once per connection --- and it does add up.
   */

  time((time_t *) &value);
  gmtime((time_t *) &value);
  strftime(buf, 80, "%d/%b/%Y:%H:%M:%S", localtime((time_t *) &value));

  /* --------------------------------------------------- */
  /* speed-hacking DNS resolve                           */
  /* --------------------------------------------------- */

  dns_init();
#if 0
  gethostname(buf, sizeof(buf));
  gethostbyname(buf);
#endif

#ifdef RLIMIT
  /* --------------------------------------------------- */
  /* adjust the resource limit                           */
  /* --------------------------------------------------- */

  getrlimit(RLIMIT_NOFILE, &limit);
  limit.rlim_cur = limit.rlim_max;
  setrlimit(RLIMIT_NOFILE, &limit);

  limit.rlim_cur = limit.rlim_max = 4 * 1024 * 1024;
  setrlimit(RLIMIT_DATA, &limit);

#ifdef SOLARIS
#define RLIMIT_RSS RLIMIT_AS
  /* Thor.981206: port for solaris 2.6 */
#endif  

  setrlimit(RLIMIT_RSS, &limit);

  limit.rlim_cur = limit.rlim_max = 0;
  setrlimit(RLIMIT_CORE, &limit);

#if 0
  limit.rlim_cur = limit.rlim_max = 60 * 20;
  setrlimit(RLIMIT_CPU, &limit);
#endif

#endif //RLIMIT

  /* --------------------------------------------------- */
  /* detach daemon process                               */
  /* --------------------------------------------------- */

  close(2);
  close(1);

  /* if (mode > 1) */
  if(inetd)
    return 0;

  close(0);

  if (fork())
    exit(0);

  setsid();

  if (fork())
    exit(0);

  /* --------------------------------------------------- */
  /* bind the service port				 */
  /* --------------------------------------------------- */

  fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  /*
   * timeout 方面, 將 socket 改成 O_NDELAY (no delay, non-blocking),
   * 如果能順利送出資料就送出, 不能送出就算了, 不再等待 TCP_TIMEOUT 時間。
   * (default 是 120 秒, 並且有 3-way handshaking 機制, 有可能一等再等)。
   */

#if 1
  fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NDELAY);
#endif

  value = 1;
  setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *) &value, sizeof(value));

  value = 1;
  setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char *) &value, sizeof(value));

  ld.l_onoff = ld.l_linger = 0;
  setsockopt(fd, SOL_SOCKET, SO_LINGER, (char *) &ld, sizeof(ld));

  sin.sin_family = AF_INET;
  sin.sin_port = htons(CHAT_PORT);
  sin.sin_addr.s_addr = htonl(INADDR_ANY);
  memset(sin.sin_zero, 0, sizeof(sin.sin_zero));

  if ((bind(fd, (struct sockaddr *) & sin, sizeof(sin)) < 0) ||
    (listen(fd, SOCK_QLEN) < 0))
    exit(1);

  return fd;
}


#ifdef	SERVER_USAGE
static void
server_usage()
{
  struct rusage ru;

  if (getrusage(RUSAGE_SELF, &ru))
    return;

  fprintf(flog, "\n[Server Usage]\n\n"
    "user time: %.6f\n"
    "system time: %.6f\n"
    "maximum resident set size: %lu P\n"
    "integral resident set size: %lu\n"
    "page faults not requiring physical I/O: %d\n"
    "page faults requiring physical I/O: %d\n"
    "swaps: %d\n"
    "block input operations: %d\n"
    "block output operations: %d\n"
    "messages sent: %d\n"
    "messages received: %d\n"
    "signals received: %d\n"
    "voluntary context switches: %d\n"
    "involuntary context switches: %d\n"
    "gline: %d\n\n",

    (double) ru.ru_utime.tv_sec + (double) ru.ru_utime.tv_usec / 1000000.0,
    (double) ru.ru_stime.tv_sec + (double) ru.ru_stime.tv_usec / 1000000.0,
    ru.ru_maxrss,
    ru.ru_idrss,
    (int) ru.ru_minflt,
    (int) ru.ru_majflt,
    (int) ru.ru_nswap,
    (int) ru.ru_inblock,
    (int) ru.ru_oublock,
    (int) ru.ru_msgsnd,
    (int) ru.ru_msgrcv,
    (int) ru.ru_nsignals,
    (int) ru.ru_nvcsw,
    (int) ru.ru_nivcsw,
    (int) gline);

  fflush(flog);
}
#endif


static void
reaper()
{
  while (waitpid(-1, NULL, WNOHANG | WUNTRACED) > 0);
}


static void
sig_trap(sig)
  int sig;
{
  char buf[80];

  sprintf(buf, "signal [%d] at line %d (errno: %d)", sig, gline, errno);
  logit("EXIT", buf);
  fclose(flog);
  exit(1);
}


static void
sig_over()
{
  int fd;

  server_usage();
  logit("OVER", "");
  fclose(flog);
  for (fd = 0; fd < 64; fd++)
    close(fd);
  execl("bin/xchatd", NULL);
}

static void
sig_log()
{
  char buf[128];

  logit("OVER", "");
  fclose(flog);

  logtalk("OVER", "");
  fclose(ftalk);

  sprintf(buf,"%s.old",CHAT_LOGFILE);
  f_mv(CHAT_LOGFILE,buf);
  sprintf(buf,"%s.old",CHAT_TALKFILE);
  f_mv(CHAT_TALKFILE,buf);

  flog = fopen(CHAT_LOGFILE, "a+");
  logit("START", "chat daemon");

  ftalk = fopen(CHAT_TALKFILE, "a+");
  logtalk("START", "chat room");

}



static void
main_signals()
{
  struct sigaction act;

  /* sigblock(sigmask(SIGPIPE)); */
  /* Thor.981206: 統一 POSIX 標準用法  */

  /* act.sa_mask = 0; */ /* Thor.981105: 標準用法 */
  sigemptyset(&act.sa_mask);      
  act.sa_flags = 0;

  act.sa_handler = sig_trap;
  sigaction(SIGBUS, &act, NULL);
  sigaction(SIGSEGV, &act, NULL);
  sigaction(SIGTERM, &act, NULL);


  /* 更新紀錄檔 */
  act.sa_handler = sig_log;
  sigaction(SIGUSR1, &act, NULL);
  sigaction(SIGHUP, &act, NULL);

  act.sa_handler = sig_over;
  sigaction(SIGXCPU, &act, NULL);

  act.sa_handler = reaper;
  sigaction(SIGCHLD, &act, NULL);

#ifdef  SERVER_USAGE
  act.sa_handler = server_usage;
  sigaction(SIGPROF, &act, NULL);
#endif

  /* Thor.981206: lkchu patch: 統一 POSIX 標準用法  */
  /* 在此借用 sigset_t act.sa_mask */
  sigaddset(&act.sa_mask, SIGPIPE);
  sigprocmask(SIG_BLOCK, &act.sa_mask, NULL);

}


/*
static void *
attach_shm(shmkey, shmsize)
  int shmkey, shmsize;
{
  void *shmptr;
  int shmid;   

  shmid = shmget(shmkey, shmsize, 0);
  if (shmid < 0)
  {
    shmid = shmget(shmkey, shmsize, IPC_CREAT | 0600);
  }
  else
  {
    shmsize = 0;
  }
   
  shmptr = (void *) shmat(shmid, NULL, 0);

  if (shmsize)
    memset(shmptr, 0, shmsize);

  return shmptr;
}
*/

/*
void
load_mud_like()
{
  if (mud == NULL)
  {
    //mud = attach_shm(MUD_KEY, sizeof(MUD));
    mud = (MUD *)malloc(sizeof(MUD));
    mudshm_init();
  }
}
*/

void
mudshm_init()
{
  ChatAction *head;
  int fw,size;   
  struct stat st;


  mud = &muddata;

  head = party_data = mud->chat_party;
  fw = open(FN_CHAT_PARTY_DB,O_RDONLY);
  fstat(fw, &st);
  
  if(!fstat(fw, &st) && (size = st.st_size) > 0)
  {
    if(size > PARTY_MAX * sizeof(ChatAction))
      size = PARTY_MAX * sizeof(ChatAction); 
    memset(head,0,PARTY_MAX * sizeof(ChatAction));
    if (size)
      read(fw, head, size);
  }
  close(fw);

  head = party_data2 = mud->chat_party2;
  fw = open(FN_CHAT_PARTY2_DB,O_RDONLY);
  fstat(fw, &st);

  if(!fstat(fw, &st) && (size = st.st_size) > 0)
  {
    if(size > PARTY_MAX * sizeof(ChatAction))
      size = PARTY_MAX * sizeof(ChatAction);
    memset(head,0,PARTY_MAX * sizeof(ChatAction));
    if (size)
      read(fw, head, size);
  }
  close(fw);  

  head = speak_data = mud->chat_speak;
  fw = open(FN_CHAT_SPEAK_DB,O_RDONLY);
  fstat(fw, &st);
  
  if(!fstat(fw, &st) && (size = st.st_size) > 0)
  {
    if(size > SPEAK_MAX * sizeof(ChatAction))
      size = SPEAK_MAX * sizeof(ChatAction);
    memset(head,0,SPEAK_MAX * sizeof(ChatAction));
    if (size)
      read(fw, head, size);
  }
  close(fw);

  head = condition_data = mud->chat_condition;
  fw = open(FN_CHAT_CONDITION_DB,O_RDONLY);
  fstat(fw, &st);
  
  if(!fstat(fw, &st) && (size = st.st_size) > 0)
  {
    if(size > CONDITION_MAX * sizeof(ChatAction))
      size = CONDITION_MAX * sizeof(ChatAction);
    memset(head,0,CONDITION_MAX * sizeof(ChatAction));
    if (size)
      read(fw, head, size);
  }
  close(fw);

  head = person_data = mud->chat_person;
  fw = open(FN_CHAT_PERSON_DB,O_RDONLY);
  fstat(fw, &st);

  if(!fstat(fw, &st) && (size = st.st_size) > 0)
  {
    if(size > CONDITION_MAX * sizeof(ChatAction))
      size = CONDITION_MAX * sizeof(ChatAction);
    memset(head,0,CONDITION_MAX * sizeof(ChatAction));
    if (size)
      read(fw, head, size);
  }
  close(fw);

}


int
main(argc, argv)
  int argc;
  char *argv[];
{
  int sock, nfds, maxfds=0, servo_sno;
  ChatUser *cu,/* *userpool,*/ **FBI;
  time_t uptime, tcheck;
  fd_set rset, xset;
  static struct timeval tv = {CHAT_INTERVAL, 0};
  struct timeval tv_tmp; /* Thor.981206: for future reservation bug */
  sock = 0;

  while ((nfds = getopt(argc, argv, "hid")) != -1)
  {
    switch (nfds)
    {
    case 'i':
      sock = 1;
      break;

    case 'd':
      break;

    case 'h':
    default:

      fprintf(stderr, "Usage: %s [options]\n"
        "\t-i  start from inetd with wait option\n"
        "\t-d  debug mode\n"
        "\t-h  help\n",
        argv[0]);
      exit(0);
    }       
  }

  servo_daemon(sock); 
  /* start_daemon(argc); */

  setgid(BBSGID);
  setuid(BBSUID);
  chdir(BBSHOME);
  umask(077);

  log_init();

  main_signals();

  /* --------------------------------------------------- */
  /* init variable : rooms & users			 */
  /* --------------------------------------------------- */

  userpool = NULL;
  strcpy(mainroom.name, MAIN_NAME);
  strcpy(mainroom.topic, MAIN_TOPIC);
  mainroom.rflag |= ROOM_OPENTOPIC;

  /* --------------------------------------------------- */
  /* main loop						 */
  /* --------------------------------------------------- */

  tcheck = 0;
  servo_sno = 0;

  mudshm_init();

  for (;;)
  {
    uptime = time(0);
    if (tcheck < uptime)
    {
      nfds = maxfds = 0;
      FD_ZERO(&mainfset);
      FD_SET(0, &mainfset);

      tcheck = uptime - CHAT_INTERVAL;

      for (FBI = &mainuser; (cu = *FBI);)
      {
#ifdef CHATROOM_KICK_TIMEOUT_USER 
	if (cu->uptime < tcheck)
	{
	  cuser_free(cu);

	  *FBI = cu->unext;

	  cu->unext = userpool;
	  userpool = cu;
	}
	else
	{
#endif
	  nfds++;
	  sock = cu->sock;
	  FD_SET(sock, &mainfset);
	  if (maxfds < sock)
	    maxfds = sock;

	  FBI = &(cu->unext);
#ifdef CHATROOM_KICK_TIMEOUT_USER 
	}
#endif
      }

      totaluser = nfds;
      fprintf(flog, "MAINTAIN %d user (%d)\n", nfds, maxfds++);
      fflush(flog);

      tcheck = uptime + CHAT_INTERVAL;
    }

    /* ------------------------------------------------- */
    /* Set up the fdsets				 */
    /* ------------------------------------------------- */

    rset = mainfset;
    xset = mainfset;

    /* Thor.981206: for future reservation bug */   
    tv_tmp = tv;
    nfds = select(maxfds, &rset, NULL, &xset, &tv_tmp);

#if 0
    {
      char buf[32];
      static int xxx;

      if ((++xxx & 8191) == 0)
      {
	sprintf(buf, "%d/%d", nfds, maxfds);
	logit("MAIN", buf);
      }
    }
#endif

    if (nfds == 0)
    {
      continue;
    }

    if (nfds < 0)
    {
      sock = errno;
      if (sock != EINTR)
      {
	logit("select", strerror(sock));
      }
      continue;
    }

    /* ------------------------------------------------- */
    /* serve active agents				 */
    /* ------------------------------------------------- */

    uptime = time(0);

    for (FBI = &mainuser; (cu = *FBI);)
    {
      sock = cu->sock;

      if (FD_ISSET(sock, &rset))
      {
	static int xxx, xno;

	nfds = cuser_serve(cu);

	if ((++xxx & 511) == 0)
	{
	  int sno;

	  sno = cu->sno;
	  fprintf(flog, "rset\t[%d] S%d R%d %d\n", sno, sock, nfds, xxx);
	  if (sno == xno)
	    nfds = -1;
	  else
	    xno = sno;
	}
      }
      else if (FD_ISSET(sock, &xset))
      {
	nfds = -1;
      }
      else
      {
	nfds = 0;
      }

      if (nfds < 0 || cu->uptime <= 0)	/* free this client */
      {
	cuser_free(cu);

	*FBI = cu->unext;

	cu->unext = userpool;
	userpool = cu;

	continue;
      }

      if (nfds > 0)
      {
	cu->uptime = uptime;
      }

      FBI = &(cu->unext);
    }

    /* ------------------------------------------------- */
    /* accept new connection				 */
    /* ------------------------------------------------- */

    if (FD_ISSET(0, &rset))
    {

      {
	static int yyy;

	if ((++yyy & 2047) == 0)
	  fprintf(flog, "conn\t%d\n", yyy);
      }

      for (;;)
      {
	int value;
	struct sockaddr_in sin;

	value = sizeof(sin);
	sock = accept(0, (struct sockaddr *) &sin, (socklen_t *) &value);
	
	if (sock > 0)
	{
	  if ((cu = userpool))
	  {
	    userpool = cu->unext;
	  }
	  else
	  {
	    cu = (ChatUser *) malloc(sizeof(ChatUser));
	  }

	  *FBI = cu;

	  /* variable initialization */

	  memset(cu, 0, sizeof(ChatUser));
	  cu->sock = sock;
	  cu->tbegin = uptime;
	  cu->uptime = uptime;
	  cu->sno = ++servo_sno;
	  cu->xdata = 0;
	  cu->retry = 0;
	  memcpy(cu->rhost, &sin.sin_addr, sizeof(struct in_addr));

	  totaluser++;

	  FD_SET(sock, &mainfset);
	  if (sock >= maxfds)
	    maxfds = sock + 1;

	  {
	    int value;

	    value = 1;
	    setsockopt(sock, IPPROTO_TCP, TCP_NODELAY,
	      (char *) &value, sizeof(value));
	  }

#if 1
	  fcntl(sock, F_SETFL, fcntl(sock, F_GETFL, 0) | O_NDELAY);
#endif

	  fprintf(flog, "CONN\t[%d] %d %s\n",
	    servo_sno, sock, str_time(&cu->tbegin));
	  break;
	}

	nfds = errno;
	if (nfds != EINTR)
	{
	  logit("accept", strerror(nfds));
	  break;
	}

#if 0
	while (waitpid(-1, NULL, WNOHANG | WUNTRACED) > 0);
#endif
      }
    }

    /* ------------------------------------------------- */
    /* tail of main loop				 */
    /* ------------------------------------------------- */

  }
}
