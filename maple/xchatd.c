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



#define SERVER_USAGE
#define WATCH_DOG
#undef  DEBUG                   /* �{���������� */
#undef  MONITOR                 /* �ʷ� chatroom ���ʥH�ѨM�ȯ� */
#undef  STAND_ALONE             /* ���f�t BBS �W�߰��� */


#ifdef  DEBUG
#define MONITOR
#endif

static int gline;

#ifdef  WATCH_DOG
#define MYDOG  gline = __LINE__
#else
#define MYDOG                   /* NOOP */
#endif


#define CHAT_PIDFILE    "run/chat.pid"
#define CHAT_LOGFILE    FN_CHAT_LOG
#define CHAT_TALKFILE   FN_CHATDATA_LOG
#define CHAT_INTERVAL   (60 * 30)
#define SOCK_QLEN       3


/* name of the main room (always exists) */


#define MAIN_NAME       "Lobby"
#define MAIN_TOPIC      "�H�K���..."




#define ROOM_LOCKED     1
#define ROOM_SECRET     2
#define ROOM_OPENTOPIC  4
#define ROOM_ALL        (NULL)


#define LOCKED(room)    (room->rflag & ROOM_LOCKED)
#define SECRET(room)    (room->rflag & ROOM_SECRET)
#define OPENTOPIC(room) (room->rflag & ROOM_OPENTOPIC)


#define RESTRICTED(usr) (usr->uflag == 0)       /* guest */
#define CHATSYSOP(usr)  (usr->uflag & (PERM_SYSOP | PERM_CHATROOM))
#define PERM_ROOMOP     PERM_CHAT       /* Thor: �� PERM_CHAT�� PERM_ROOMOP */
#define PERM_CHATOP     PERM_DENYCHAT   /* Thor: �� PERM_DENYCHAT�� PERM_CHATOP */
/* #define ROOMOP(usr)     (usr->uflag & ( PERM_ROOMOP | PERM_SYSOP | PERM_CHATROOM )) */
/* Thor.980603: PERM_CHATROOM�אּ default �S�� roomop, ���i�H�ۤv���o chatop*/
#define ROOMOP(usr)     (usr->uflag & (PERM_ROOMOP|PERM_CHATOP))
#define CLOAK(usr)      (usr->uflag & PERM_CLOAK)


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
void mudshm_init(void);


struct ChatUser
{
    ChatUser *unext;
    ChatRoom *room;
    UserList *ignore;
    int sock;                   /* user socket */
    int userno;
    int uflag;
    int clitype;                /* Xshadow: client type. 1 for common client,
                                 * 0 for bbs only client */
    time_t tbegin;
    time_t uptime;
    int sno;
    int xdata;
    int retry;

    int isize;                  /* current size of ibuf */
    char ibuf[128];             /* buffer for non-blocking receiving */
    char userid[IDLEN + 1];     /* real userid */
    char chatid[9];             /* chat id */
    char rhost[30];             /* host address */
};


struct ChatRoom
{
    ChatRoom *next, *prev;
    UserList *invite;
    char name[IDLEN];
    char topic[48];             /* Let the room op to define room topic */
    int rflag;                  /* ROOM_LOCKED, ROOM_SECRET, ROOM_OPENTOPIC */
    int occupants;              /* number of users in room */
};


struct UserList
{
    UserList *next;
    int userno;
    char userid[FLEX_SIZE];
};
#define UserList_FLEX_MEMBER    userid


struct ChatCmd
{
    const char *cmdstr;
    void (*cmdfunc) (ChatUser *cu, char *msg);
    int exact;
};


static ChatRoom mainroom, *roompool;
static ChatUser *mainuser, *userpool;
static fd_set mainfset;
static int totaluser;           /* current number of connections */
static struct timeval zerotv;   /* timeval for selecting */
static int common_client_command;


#ifdef STAND_ALONE
static int userno_inc = 0;      /* userno auto-incrementer */
#endif


static const char msg_not_op[] = "�� �z���O�o��" CHATROOMNAME "�� Op";
static const char msg_no_such_id[] = "�� �ثe�S���H�ϥ� [%s] �o�Ӳ�ѥN��";
static const char msg_not_here[] = "�� [%s] ���b�o��" CHATROOMNAME "�C";


#define FUZZY_USER      ((ChatUser *) -1)

void load_mud_like(void);

/* ----------------------------------------------------- */
/* operation log and debug information                   */
/* ----------------------------------------------------- */


static FILE *flog;
static FILE *ftalk;


/* Thor.990211: �Τ@�� dao library */
#define str_time(t) Btime(t)

static void
logtalk(
    const char *key,
    const char *msg)
{
    time_t now;
    struct tm *p;

    time(&now);
    p = localtime(&now);
    fprintf(ftalk, "%02d/%02d/%02d %02d:%02d:%02d %-*s %s\n",
        p->tm_year % 100, p->tm_mon + 1, p->tm_mday,
        p->tm_hour, p->tm_min, p->tm_sec, IDLEN, key, msg);
}


static void
logit(
    const char *key,
    const char *msg)
{
    time_t now;
    struct tm *p;

    time(&now);
    p = localtime(&now);
    fprintf(flog, "%02d/%02d/%02d %02d:%02d:%02d %-*s %s\n",
        p->tm_year % 100, p->tm_mon + 1, p->tm_mday,
        p->tm_hour, p->tm_min, p->tm_sec, IDLEN, key, msg);
}


static inline void
log_init(void)
{
    FILE *fp;

    /* --------------------------------------------------- */
    /* log daemon's PID                                    */
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


#ifdef  DEBUG
static char chatbuf[256];       /* general purpose buffer */


static void
debug_list(
    const UserList *list)
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
debug_user(void)
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
debug_room(void)
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
log_user(
    const ChatUser *cu)
{
    static int log_num;

    if (cu)
    {
        if (log_num > 100 && log_num < 150)
        {
            sprintf(chatbuf, "%d: %p <%d>", log_num, cu, gline);
            logit("traverse user ", chatbuf);
        }
        else if (log_num == 100)
        {
            sprintf(chatbuf, "BOOM !! at line %d", gline);
            logit("traverse user ", chatbuf);
        }
        log_num++;
    }
    else
        log_num = 0;
}
#endif                          /* DEBUG */


/* ----------------------------------------------------- */
/* string routines                                       */
/* ----------------------------------------------------- */


static int
valid_chatid(
    const char *id)
{
    int ch, len;

    for (len = 0; (ch = *id); id++)
    { /* Thor.980921: �ťլ����X�zchatid, ��getnext�P�_���~���� */
        if (ch == '/' || ch == '*' || ch == ':' || ch ==' ')
            return 0;
        if (++len > 8)
            return 0;
    }
    return len;
}


/* Case Independent strcmp : 1 ==> equal */


/* Thor.990211: �Τ@�ϥ�dao library */
#define str_equal(s1, s2) (!str_casecmp(s1, s2))

/* ----------------------------------------------------- */
/* match strings' similarity case-insensitively          */
/* ----------------------------------------------------- */
/* str_match(keyword, string)                            */
/* ----------------------------------------------------- */
/* 0 : equal            ("foo", "foo")                   */
/* -1 : mismatch        ("abc", "xyz")                   */
/* o.w. : similar       ("goo", "good")                  */
/* ----------------------------------------------------- */


GCC_PURE static int
str_match(
    const char *s1,             /* lower-case (sub)string */
    const char *s2)
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
cuser_by_userid(
    const char *userid)
{
    ChatUser *cu;
    char buf[80]; /* Thor.980727: �@���̪��~80 */

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
cuser_by_chatid(
    const char *chatid)
{
    ChatUser *cu;
    char buf[80]; /* Thor.980727: �@���̪��~80 */

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
fuzzy_cuser_by_chatid(
    const char *chatid)
{
    ChatUser *cu, *xuser;
    int mode;
    char buf[80]; /* Thor.980727: �@���̪��~80 */

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
                return FUZZY_USER;      /* �ŦX�̤j�� 2 �H */

            xuser = cu;
        }
    }
    return xuser;
}


static ChatRoom *
croom_by_roomid(
    const char *roomid)
{
    ChatRoom *room;
    char buf[80]; /* Thor.980727: �@���̪��~80 */

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
list_free(
    UserList **list)
{
    UserList *user, *next;

    for (user = *list, *list = NULL; user; user = next)
    {
        next = user->next;
        free(user);
    }
}


static void
list_add(
    UserList **list,
    const ChatUser *user)
{
    UserList *node;
    const char *userid;
    int len;

    len = strlen(userid = user->userid) + 1;
    if ((node = (UserList *) malloc(SIZEOF_FLEX(UserList, len))))
    {
        node->next = *list;
        node->userno = user->userno;
        memcpy(node->userid, userid, len);
        *list = node;
    }
}


static int
list_delete(
    UserList **list,
    const char *userid)
{
    UserList *node;
    char buf[80]; /* Thor.980727: ��J�@���̪��~ 80 */

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


GCC_PURE static int
list_belong(
    const UserList *list,
    int userno)
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
str_swap(
    char *str,
    const char *src,
    const char *des)
{
    char *ptr, *tmp;
    char buf[600];
    ptr = strstr(str, src);
    if (ptr)
    {
        *ptr = '\0';
        tmp = ptr + strlen(src);
        sprintf(buf, "%s%s%s", str, des, tmp);
        strcpy(str, buf);
        return 1;
    }
    else
        return 0;
}

static void
do_send(
    int nfds,
    fd_set *wset,
    char *msg)
{
    int len, sr;

#if 1
    /* Thor: for future reservation bug */
    zerotv.tv_sec = 0;
    zerotv.tv_usec = 0;
#endif
    while (str_swap(msg, "\\x1b", "\x1b"));

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
send_to_room(
    ChatRoom *room,
    const char *msg,
    int userno,
    int number)
{
    ChatUser *cu;
    fd_set wset;
    int sock, max;
    int clitype;                        /* ���� bbs client �� common client �⦸�B�z */
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

                max = BMAX(max, sock);
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
            str = buf;
            strcpy(str, msg);
        }

        do_send(max, &wset, str);
    }
}


static void
send_to_user(
    ChatUser *user,
    const char *msg,
    int userno,
    int number)
{
    int sock;

#if 0
    if (!user->userno || (!user->clitype && number && number != MSG_MESSAGE))
#endif
    /* Thor.980911: �p�G�duser->userno�h�blogin_user��error message�|�L�k�e�^ */
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
        else
            strcpy(buf, msg);

        do_send(sock, &wset, buf);
    }
}


/* ----------------------------------------------------- */


static void
room_changed(
    ChatRoom *room)
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
user_changed(
    ChatUser *cu)
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
exit_room(
    ChatUser *user,
    int mode,
    char *msg)
{
    ChatRoom *room;
    char buf[128];

    if (!(room = user->room))
        return;

    user->room = NULL;
    /* user->uflag &= ~(PERM_ROOMOP | PERM_SYSOP | PERM_CHATROOM); */
    user->uflag &= ~PERM_ROOMOP;
    /* Thor.980601: ���}�ж��ɥu�M room op, ���M sysop, chatroom, �]�ѥͨ㦳 */

    if (room->occupants -= (CLOAK(user)) ? 0 : 1 )
    {
        char *chatid;

        chatid = user->chatid;
        switch (mode)
        {
        case EXIT_LOGOUT:

            sprintf(buf, "�� %s ���}�F ...", chatid);
            if (msg && *msg)
            {
                strcat(buf, ": ");

                msg[79] = 0;            /* Thor:����Ӫ� */

                strncat(buf, msg, 79);
            }
            break;

        case EXIT_LOSTCONN:

            sprintf(buf, "�� %s ���F�_�u�������o", chatid);
            break;

        case EXIT_KICK:

            sprintf(buf, "�� �����I%s �Q��X�h�F", chatid);
            break;
        }

        if (!CLOAK(user))
            send_to_room(room, buf, 0, MSG_MESSAGE);

        sprintf(buf, "- %s", user->userid);
        send_to_room(room, buf, 0, MSG_USERNOTIFY);
/*      room->occupants += CLOAK(user) ? 1 : 0;*/
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

        /* �^�� */
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

/* Thor.990211: �Τ@�� dao library */
/* static */
int
acct_load(
    ACCT *acct,
    const char *userid)
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


/* Thor.990211: �Τ@�� dao library */

static void
chat_query(
    ChatUser *cu,
    char *msg)
{
    FILE *fp;
    ACCT acct;
    char buf[256];

    /* Thor.980617: �i���d�O�_���Ŧr�� */
    if (*msg && acct_load(&acct, msg) >= 0)
    {
        sprintf(buf, "%s(%s) �@�W�� %d ���A�峹 %d �g",
            acct.userid, acct.username, acct.numlogins, acct.numposts);
        send_to_user(cu, buf, 0, MSG_MESSAGE);

        sprintf(buf, "�̪�(%s)�q(%s)�W��", Ctime(&acct.lastlogin),
            (acct.lasthost[0] ? acct.lasthost : "�~�Ӫ�"));
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
#endif  /* #ifndef STAND_ALONE */


static void
chat_clear(
    ChatUser *cu,
    char *msg)
{
    if (cu->clitype)
        send_to_user(cu, "", 0, MSG_CLRSCR);
    else
        send_to_user(cu, "/c", 0, MSG_MESSAGE);
}


static void
chat_date(
    ChatUser *cu,
    char *msg)
{
    time_t thetime;
    char buf[128];

    time(&thetime);
    sprintf(buf, "�� �зǮɶ�: %s", Ctime(&thetime));
    send_to_user(cu, buf, 0, MSG_MESSAGE);
}


static void
chat_mud(
    ChatUser *cu,
    char *msg)
{
    ChatRoom *room;

    room = cu->room;

    if (!ROOMOP(cu))
    {
        return;
    }
    mudshm_init();
    send_to_room(room,  "�� ��s" CHATROOMNAME "�ʺA��", 0, MSG_MESSAGE);

}

static void
chat_topic(
    ChatUser *cu,
    char *msg)
{
    ChatRoom *room;
    char *topic, buf[128];

    room = cu->room;

    if (!ROOMOP(cu) && !OPENTOPIC(room))
    {
        send_to_user(cu, msg_not_op, 0, MSG_MESSAGE);
        return;
    }

    if (strstr(msg, "\\x1b"))
    {
        send_to_user(cu, "�� ���X�檺���D", 0, MSG_MESSAGE);
        return;
    }
    if (*msg == '\0')
    {
        send_to_user(cu, "�� �Ы��w���D", 0, MSG_MESSAGE);
        return;
    }

    topic = room->topic;
    str_scpy(topic, msg, sizeof(room->topic));

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
        sprintf(buf, "�� %s �N���D�אּ \x1b[1;32m%s\x1b[m", cu->chatid, topic);
        send_to_room(room, buf, 0, MSG_MESSAGE);
    }
}


static void
chat_version(
    ChatUser *cu,
    char *msg)
{
    char buf[80];

    sprintf(buf, "%d %d", XCHAT_VERSION_MAJOR, XCHAT_VERSION_MINOR);
    send_to_user(cu, buf, 0, MSG_VERSION);
}


static void
chat_nick(
    ChatUser *cu,
    char *msg)
{
    char *chatid, *str, buf[128];
    ChatUser *xuser;

    chatid = nextword(&msg);
    chatid[8] = '\0';
    if (!valid_chatid(chatid) || strstr(chatid, "\\x1b"))
    {
        send_to_user(cu, "�� �o�Ӳ�ѥN���O�����T��", 0, MSG_MESSAGE);
        return;
    }

    xuser = cuser_by_chatid(chatid);
    if (xuser != NULL && xuser != cu)
    {
        send_to_user(cu, "�� �w�g���H�������n�o", 0, MSG_MESSAGE);
        return;
    }

    str = cu->chatid;

    if (!CLOAK(cu))
    {
        sprintf(buf, "�� %s �N��ѥN���אּ \x1b[1;33m%s\x1b[m", str, chatid);
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
chat_list_rooms(
    ChatUser *cuser,
    char *msg)
{
    ChatRoom *cr, *room;
    char buf[128];
    int mode;

    if (RESTRICTED(cuser))
    {
        send_to_user(cuser, "�� �z�S���v���C�X�{����" CHATROOMNAME, 0, MSG_MESSAGE);
        return;
    }

    mode = common_client_command;

    if (mode)
        send_to_user(cuser, "", 0, MSG_ROOMLISTSTART);
    else
        send_to_user(cuser, "\x1b[7m  ��ѫǦW��  �x�H�Ƣx���D        \x1b[m", 0,
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
                sprintf(buf, "%-*s�x%4d�x%s", (int)(unsigned int)strlen(CHATROOMNAME)-6+12,
                    cr->name, cr->occupants, cr->topic);
                if (LOCKED(cr))
                    strcat(buf, " [���]");
                if (SECRET(cr))
                    strcat(buf, " [���K]");
                if (OPENTOPIC(cr))
                    strcat(buf, " [���D]");
                send_to_user(cuser, buf, 0, MSG_MESSAGE);
            }
        }
    } while ((cr = cr->next));

    if (mode)
        send_to_user(cuser, "", 0, MSG_ROOMLISTEND);
}


static void
chat_do_user_list(
    ChatUser *cu,
    char *msg,
    ChatRoom *theroom)
{
    ChatRoom *myroom, *room;
    ChatUser *user;
    int start, stop, curr, mode; /*, uflag; */
    char buf[128];

    curr = 0; /* Thor.980619: initialize curr */
    start = atoi(nextword(&msg));
    stop = atoi(nextword(&msg));

    mode = common_client_command;

    if (mode)
        send_to_user(cu, "", 0, MSG_USERLISTSTART);
    else
        send_to_user(cu, "\x1b[7m ��ѥN���x�ϥΪ̥N��  �x" CHATROOMNAME " \x1b[m", 0,
            MSG_MESSAGE);

    myroom = cu->room;

    /* Thor.980717: �ݭn���ư� cu->userno == 0 �����p��? */
    for (user = mainuser; user; user = user->unext)
    {
        /*
        if (!cu->userno)
            continue;
         */ /* Thor.980717: �J�M cu ���ŤF���ٶi�ӷF��? */
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
        /* Thor.980717:�W�zlogic���~ */

        /* Thor.980717: viewer check */
        if ((myroom != room) && (RESTRICTED(cu) ||
                                  (room && SECRET(room) && !CHATSYSOP(cu))))
            continue;

        /* Thor.980717: viewee check */
        if (CLOAK(user) && (user != cu) && !CHATSYSOP(cu))
            continue;
        /* Thor.980717:���g*/


        curr++;
        if (start && curr < start)
            continue;
        else if (stop && (curr > stop))
            break;

        if (mode)
        {
            if (!room)
                continue;               /* Xshadow: �٨S�i�J����ж����N���C�X */

            sprintf(buf, "%s %s %s %s",
                user->chatid, user->userid, room->name, user->rhost);
/* Thor.980603: PERM_CHATROOM�אּ default �S�� roomop, ���i�H�ۤv���o */
/*          if (uflag & (PERM_ROOMOP | PERM_SYSOP | PERM_CHATROOM)) */
            if (ROOMOP(user))
                strcat(buf, " Op");
        }
        else
        {
            sprintf(buf, " %-8s�x%-*s�x%s",
                user->chatid, IDLEN, user->userid, room ? room->name : "[�b���f�r��]");
/* Thor.980603: PERM_CHATROOM�אּ default �S�� roomop, ���i�H�ۤv���o */
/*          if (uflag & (PERM_ROOMOP | PERM_SYSOP | PERM_CHATROOM)) */
            /* if (uflag & (PERM_ROOMOP|PERM_CHATOP)) */
            if (ROOMOP(user))  /* Thor.980602: �Τ@�Ϊk */
                strcat(buf, " [Op]");
        }

        send_to_user(cu, buf, 0, mode ? MSG_USERLIST : MSG_MESSAGE);
    }

    if (mode)
        send_to_user(cu, "", 0, MSG_USERLISTEND);
}


static void
chat_list_by_room(
    ChatUser *cu,
    char *msg)
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
            sprintf(buf, "�� �S�� [%s] �o��" CHATROOMNAME, roomstr);
            send_to_user(cu, buf, 0, MSG_MESSAGE);
            return;
        }

        if (whichroom != cu->room && SECRET(whichroom) && !CHATSYSOP(cu))
        {
            send_to_user(cu, "�� �L�k�C�X�b���K" CHATROOMNAME "���ϥΪ�", 0, MSG_MESSAGE);
            return;
        }
    }
    chat_do_user_list(cu, msg, whichroom);
}


static void
chat_list_users(
    ChatUser *cu,
    char *msg)
{
    chat_do_user_list(cu, msg, ROOM_ALL);
}


static void
chat_chatroom(
    ChatUser *cu,
    char *msg)
{
    if (common_client_command)
        send_to_user(cu, NICKNAME CHATROOMNAME, 0, MSG_CHATROOM);
}


static void
chat_map_chatids(
    ChatUser *cu,                       /* Thor: �٨S���@���P���� */
    ChatRoom *whichroom)
{
    int c;
    ChatRoom *myroom, *room;
    ChatUser *user;
    char buf[128];

    myroom = cu->room;

    send_to_user(cu,
        "\x1b[7m ��ѥN�� �ϥΪ̥N��  �x ��ѥN�� �ϥΪ̥N��  �x ��ѥN�� �ϥΪ̥N�� \x1b[m", 0, MSG_MESSAGE);

    for (c = 0, user = mainuser; user; user = user->unext)
    {
        if (!cu->userno)
            continue;

        room = user->room;
        if (whichroom != ROOM_ALL && whichroom != room)
            continue;

        if (myroom != room)
        {
            if (RESTRICTED(cu) ||       /* Thor: �n��check room �O���O�Ū� */
                (room && SECRET(room) && !CHATSYSOP(cu)))
                continue;
        }

        if (CLOAK(user) && (user != cu) && !CHATSYSOP(cu))      /* Thor:�����N */
            continue;

        sprintf(buf + (c * 24), " %-8s%c%-*s%s",
            user->chatid, ROOMOP(user) ? '*' : ' ',
            IDLEN, user->userid, (c < 2 ? "�x" : "  "));

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
chat_map_chatids_thisroom(
    ChatUser *cu,
    char *msg)
{
    chat_map_chatids(cu, cu->room);
}


static void
chat_setroom(
    ChatUser *cu,
    char *msg)
{
    char *modestr;
    ChatRoom *room;
    char *chatid;
    int sign, flag;
    char buf[128];
    const char *fstr=NULL;

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
            "�� �Ы��w���A: {[+(�]�w)][-(����)]}{[L(���)][s(���K)][t(�}����D)}", 0, MSG_MESSAGE);
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
            fstr = "���";
            break;

        case 's':
        case 'S':
            flag = ROOM_SECRET;
            fstr = "���K";
            break;

        case 't':
        case 'T':
            flag = ROOM_OPENTOPIC;
            fstr = "�}����D";
            break;

        default:
            sprintf(buf, "�� ���A���~�G[%c]", *modestr);
            send_to_user(cu, buf, 0, MSG_MESSAGE);
        }

        /* Thor: check room �O���O�Ū�, ���Ӥ��O�Ū� */

        if (flag && (room->rflag & flag) != sign * flag)
        {
            room->rflag ^= flag;

            if (!CLOAK(cu))
            {
                sprintf(buf, "�� ��" CHATROOMNAME "�Q %s %s [%s] ���A",
                    chatid, sign ? "�]�w��" : MSG_CANCEL, fstr);
                send_to_room(room, buf, 0, MSG_MESSAGE);
            }
        }
        modestr++;
    }

    /* Thor.980602: ���� Main room ��_ or ���K, �o�������?
                    ���}���N�i����, �n�ݤ]�ݤ���?
                    �Q�n��H�]�𤣶i main room, ���|�ܩ_�Ƕ�? */
    if (str_equal(MAIN_NAME, room->name))
    {
        if (room->rflag & (ROOM_LOCKED | ROOM_SECRET))
        {
                send_to_room(room, "�� �� " SYSOPNICK " �I�F�y�_��z���]�k", 0, MSG_MESSAGE);
                room->rflag &= ~(ROOM_LOCKED|ROOM_SECRET);
        }
    }

    room_changed(room);
}


static const char *const chat_msg[] =
{
    "[//]help", "MUD-like ����ʵ�",
    "[/h]elp op", CHATROOMNAME "�޲z���M�Ϋ��O",
    "[/a]ct <msg>", "���@�Ӱʧ@",
    "[/b]ye [msg]", "�D�O",
    "[/c]lear  [/d]ate", "�M���ù�  �ثe�ɶ�",
    /* "[/d]ate", "�ثe�ɶ�", *//* Thor: ���O�Ӧh */

#if 0
    "[/f]ire <user> <msg>", "�o�e���T",         /* Thor.0727: �M flag ��key */
#endif

    "[/i]gnore [user]", "�����ϥΪ�",
    "[/j]oin <room>", "�إߩΥ[�J" CHATROOMNAME,
    "[/l]ist [start [stop]]", "�C�X" CHATROOMNAME "�ϥΪ�",
    "[/m]sg <id|user> <msg>", "�� <id> ��������",
    "[/n]ick <id>", "�N�ͤѥN������ <id>",
    "[/p]ager", "�����I�s��",
    "[/q]uery <user>", "�d�ߺ���",
    "[/r]oom", "�C�X�@��" CHATROOMNAME,
    "[/t]ape", "�}��������",
    "[/u]nignore <user>", "��������",

#if 0
    "[/u]sers", "�C�X���W�ϥΪ�",
#endif

    "[/w]ho", "�C�X��" CHATROOMNAME "�ϥΪ�",
    "[/w]hoin <room>", "�C�X" CHATROOMNAME "<room> ���ϥΪ�",
    NULL
};


static const char *const room_msg[] =
{
    "[/f]lag [+-][lst]", "�]�w��w�B���K�B�}����D",
    "[/i]nvite <id>", "�ܽ� <id> �[�J" CHATROOMNAME,
    "[/kick] <id>", "�N <id> ��X" CHATROOMNAME,
    "[/o]p [<id>]", "�N Op ���v�O�ಾ�� <id>",
    "[/topic] <text>", "���Ӹ��D",
    "[/mud]", "��s" CHATROOMNAME "�ʺA��",
    "[/w]all", "�s�� (�����M��)",
    NULL
};


static void
chat_help(
    ChatUser *cu,
    char *msg)
{
    char buf[128];
    const char *const *table, *str;

    if (str_equal("op", nextword(&msg)))
    {
        send_to_user(cu, CHATROOMNAME "�޲z���M�Ϋ��O", 0, MSG_MESSAGE);
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
chat_private(
    ChatUser *cu,
    char *msg)
{
    ChatUser *xuser;
    int userno;
    char *recipient, buf[128];

    userno = 0;
    recipient = nextword(&msg);
    xuser = (ChatUser *) fuzzy_cuser_by_chatid(recipient);
    if (xuser == NULL)          /* Thor.0724: �� userid�]�i�Ǯ����� */
    {
        xuser = cuser_by_userid(recipient);
    }

    if (xuser == NULL)
    {
        sprintf(buf, msg_no_such_id, recipient);
    }
    else if (xuser == FUZZY_USER)
    {                           /* ambiguous */
        strcpy(buf, "�� �Ы�����ѥN��");
    }
    else if (*msg)
    {
        userno = cu->userno;
        sprintf(buf, "\x1b[1m*%s*\x1b[m ", cu->chatid);
        msg[79] = 0;            /* Thor:����Ӫ� */
        strncat(buf, msg, 80);
        send_to_user(xuser, buf, userno, MSG_MESSAGE);

        if (xuser->clitype)
        {                               /* Xshadow: �p�G���O�� client �W�Ӫ� */
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
        sprintf(buf, "�� �z�Q�� %s ������ܩO�H", xuser->chatid);
    }

    send_to_user(cu, buf, userno, MSG_MESSAGE);

    /* Thor: userno �n�令 0 ��? */
}


static void
chat_cloak(
    ChatUser *cu,
    char *msg)
{
    if (CHATSYSOP(cu))
    {
        char buf[128];
        ChatRoom *room;


        cu->uflag ^= PERM_CLOAK;
        room = cu->room;
        room->occupants += CLOAK(cu) ? -1 : 1;
        sprintf(buf, "�� %s", CLOAK(cu) ? MSG_CLOAKED : MSG_UNCLOAK);
        send_to_user(cu, buf, 0, MSG_MESSAGE);
    }
}


/* ----------------------------------------------------- */


static void
arrive_room(
    ChatUser *cuser,
    ChatRoom *room)
{
    char *rname, buf[256];

    /* Xshadow: �����e���ۤv, �ϥ����ж��N�|���s build user list */

    sprintf(buf, "+ %s %s %s %s",
        cuser->userid, cuser->chatid, room->name, cuser->rhost);
    if (ROOMOP(cuser))
        strcat(buf, " Op");
    send_to_room(room, buf, 0, MSG_USERNOTIFY);

    if (!CLOAK(cuser))
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

    sprintf(buf, "�� \x1b[32;1m%s\x1b[m �i�J \x1b[33;1m[%s]\x1b[m �]�[",
        cuser->chatid, rname);

    if (!CLOAK(cuser))
        send_to_room(room, buf, cuser->userno, MSG_MESSAGE);
    else
        send_to_user(cuser, buf, 0, MSG_MESSAGE);
}


static int
enter_room(
    ChatUser *cuser,
    const char *rname,
    char *msg)
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
            send_to_user(cuser, "�� �L�k�A�s�P�]�[�F", 0, MSG_MESSAGE);
            return 0;
        }

        memset(room, 0, sizeof(ChatRoom));
        str_scpy(room->name, rname, IDLEN);
        strcpy(room->topic, "�o�O�@�ӷs�Ѧa");

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
            sprintf(buf, "�� �z���ӴN�b [%s] " CHATROOMNAME "�o :)", rname);
            send_to_user(cuser, buf, 0, MSG_MESSAGE);
            return 0;
        }

        if (!CHATSYSOP(cuser) && LOCKED(room) &&
            !list_belong(room->invite, cuser->userno))
        {
            send_to_user(cuser, "�� �����c���A�D�в��J", 0, MSG_MESSAGE);
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
cuser_free(
    ChatUser *cuser)
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
print_user_counts(
    ChatUser *cuser)
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

    sprintf(buf, "�� �w����{�i" NICKNAME CHATROOMNAME "�j�A�� \x1b[1;33m/h\x1b[m ��ܻ����A�� \x1b[1;33m/b\x1b[m ���}");
    send_to_user(cuser, buf, 0, number);

    sprintf(buf, "�� �ثe�}�F \x1b[1;31m%d\x1b[m ���]�[�A�@�� \x1b[1;36m%d\x1b[m �H���\\�s���}", roomc, userc);
    if (suserc)
        sprintf(buf + strlen(buf), " [ %d �H�b���K�]�[]", suserc);
    send_to_user(cuser, buf, 0, number);

//  load_mud_like();
}


static int
login_user(
    ChatUser *cu,
    char *msg)
{
    int utent;

    char *userid;
    char *chatid, *passwd;
    ChatUser *xuser;
    int level;

#ifndef STAND_ALONE
    ACCT acct;
#endif

    /* Xshadow.0915: common client support : /-! userid chatid password */
    /* client/server �����̾� userid �� .PASSWDS �P�_ userlevel */

    userid = nextword(&msg);
    chatid = nextword(&msg);

#ifdef  DEBUG
    logit("ENTER", userid);
#endif

#ifndef STAND_ALONE
    /* Thor.0730: parse space before passwd */

    passwd = msg;

    /* Thor.0813: ���L�@�Ů�Y�i, �]���ϥ��p�Gchatid���Ů�, �K�X�]���� */
    /* �N��K�X��, �]���|����:p */
    /* �i�O�p�G�K�X�Ĥ@�Ӧr�O�Ů�, �����Ӧh�Ů�|�i����... */
    /* Thor.980910: �ѩ� nextword�קאּ�ᱵ�Ů��0, �ǤJ�ȫh�����Ჾ��0��,
                    �ҥH���ݧ@���ʧ@ */
#if 0
    if (*passwd == ' ')
        passwd++;
#endif

    /* Thor.0729: load acct */

    if (!*userid || (acct_load(&acct, userid) < 0))
    {

#ifdef  DEBUG
        logit("noexist", userid);
#endif

        if (cu->clitype)
            send_to_user(cu, ERR_UID, 0, ERR_LOGIN_NOSUCHUSER);
        else
            send_to_user(cu, CHAT_LOGIN_INVALID, 0, MSG_MESSAGE);

        return -1;
    }

    /* Thor.0813: ��ίu�� password check, for C/S bbs */

    /* Thor.990214: �`�N, daolib�� �D0�N���� */
    /* if (!chkpasswd(acct.passwd, acct.passhash, passwd)) */
    if ((strncmp(passwd, acct.passwd, PASSSIZE-1)
          || (strlen(passwd) >= PASSSIZE
              && strncmp(passwd + PASSSIZE - 1, acct.passhash, sizeof(acct.passhash))))
        && chkpasswd(acct.passwd, acct.passhash, passwd))
    {

#ifdef  DEBUG
        logit("fake", userid);
#endif

        if (cu->clitype)
            send_to_user(cu, "�K�X���~", 0, ERR_LOGIN_PASSERROR);
        else
            send_to_user(cu, CHAT_LOGIN_INVALID, 0, MSG_MESSAGE);

        return -1;
    }

    level = acct.userlevel;
    utent = acct.userno;

#else                           /* STAND_ALONE */
    level = 1;
    utent = ++userno_inc;
#endif                          /* STAND_ALONE */

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

#ifdef  DEBUG
            logit("enter", "bogus");
#endif

            if (cu->clitype)
                send_to_user(cu, "�ФŬ��������i�J" CHATROOMNAME " !!", 0,
                    ERR_LOGIN_USERONLINE);
            else
                send_to_user(cu, CHAT_LOGIN_BOGUS, 0, MSG_MESSAGE);
            return -1;          /* Thor: �άO0�����ۤv�F�_? */
        }
    }


#ifndef STAND_ALONE
    /* Thor.980629: �Ȯɭɥ� invalid_chatid �o�� �S��PERM_CHAT���H */

    if (!valid_chatid(chatid) || !(level & PERM_CHAT) || (level & PERM_DENYCHAT))
    { /* Thor.981012: �����@��, �s denychat�]BAN��, �K�o client�@�� */

#ifdef  DEBUG
        logit("enter", chatid);
#endif

        if (cu->clitype)
            send_to_user(cu, "���X�k��" CHATROOMNAME "�N�� !!", 0, ERR_LOGIN_NICKERROR);
        else
            send_to_user(cu, CHAT_LOGIN_INVALID, 0, MSG_MESSAGE);
        return 0;
    }
#endif  /* #ifndef STAND_ALONE */

#ifdef  DEBUG
    debug_user();
#endif

    if (cuser_by_chatid(chatid) != NULL)
    {
        /* chatid in use */

#ifdef  DEBUG
        logit("enter", "duplicate");
#endif

        if (cu->clitype)
            send_to_user(cu, "�o�ӥN���w�g���H�ϥ�", 0, ERR_LOGIN_NICKINUSE);
        else
            send_to_user(cu, CHAT_LOGIN_EXISTS, 0, MSG_MESSAGE);
        return 0;
    }

#ifdef DEBUG                    /* CHATSYSOP �@�i�ӴN���� */
    cu->uflag = level & ~(PERM_ROOMOP | PERM_CHATOP | (CHATSYSOP(cu) ? 0 : PERM_CLOAK));
#else
    cu->uflag = level & ~(PERM_ROOMOP | PERM_CHATOP | PERM_CLOAK);
#endif

    /* Thor: �i�ӥ��M�� ROOMOP (�PPERM_CHAT) */

    strcpy(cu->userid, userid);
    str_scpy(cu->chatid, chatid, sizeof(cu->chatid));

    fprintf(flog, "ENTER\t[%d] %s\n", cu->sno, userid);

    /* Xshadow: ���o client ���ӷ� */

    /* IID.2021-02-12: After `command_execute()` is done, `cu->ibuf` will be reset by setting `cu->isize` to `0`,
     *     so it can be used as a temporary buffer here without problems. */
#ifdef NOIDENT
    memcpy(cu->ibuf, cu->rhost, sizeof(ip_addr));
    getnameinfo((struct sockaddr *)cu->rhost, sizeof(cu->rhost), cu->ibuf, sizeof(ip_addr), NULL, NI_MAXSERV, NI_NUMERICHOST);
#else
    dns_name((ip_addr *)cu->rhost, cu->ibuf, sizeof(cu->ibuf));
    str_scpy(cu->rhost, cu->ibuf, sizeof(cu->rhost));
  #if 0
    memcpy(cu->ibuf, cu->rhost, sizeof(ip_addr));
    getnameinfo((struct sockaddr *)cu->rhost, sizeof(cu->rhost), cu->ibuf, sizeof(ip_addr), NULL, NI_MAXSERV, 0);
  #endif
#endif

    cu->userno = utent;

    if (cu->clitype)
        send_to_user(cu, "���Q", 0, MSG_LOGINOK);
    else
        send_to_user(cu, CHAT_LOGIN_OK, 0, MSG_MESSAGE);

    arrive_room(cu, &mainroom);

    send_to_user(cu, "", 0, MSG_MOTDSTART);
    print_user_counts(cu);
    send_to_user(cu, "", 0, MSG_MOTDEND);

#ifdef  DEBUG
    logit("enter", "OK");
#endif

    return 0;
}


static void
chat_act(
    ChatUser *cu,
    char *msg)
{
    if (*msg)
    {
        char buf[256];

        sprintf(buf, "%s \x1b[36m%s\x1b[m", cu->chatid, msg);
        send_to_room(cu->room, buf, cu->userno, MSG_MESSAGE);
    }
}


static void
chat_ignore(
    ChatUser *cu,
    char *msg)
{
    char buf[256];
    const char *str;

    if (RESTRICTED(cu))
    {
        str = "�� �z�S�� ignore �O�H���v�Q";
    }
    else
    {
        char *ignoree;

        ignoree = nextword(&msg);
        if (*ignoree)
        {
            ChatUser *xuser;

            xuser = cuser_by_userid(ignoree);

            if (xuser == NULL)
            {
                sprintf(buf, msg_no_such_id, ignoree);
            }
            else if (xuser == cu || CHATSYSOP(xuser) ||
                (ROOMOP(xuser) && (xuser->room == cu->room)))
            {
                sprintf(buf, "�� ���i�H ignore [%s]", ignoree);
            }
            else
            {
                if (list_belong(cu->ignore, xuser->userno))
                {
                    sprintf(buf, "�� %s �w�g�Q�ᵲ�F", xuser->chatid);
                }
                else
                {
                    list_add(&(cu->ignore), xuser);
                    sprintf(buf, "�� �N [%s] ���J�N�c�F :p", xuser->chatid);
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

                send_to_user(cu, "�� �o�ǤH�Q���J�N�c�F�G", 0, MSG_MESSAGE);
                len = 0;
                do
                {
                    sprintf(userid, "%-*s ", IDLEN, list->userid);
                    strcpy(buf + len, userid);
                    len += 13;
                    if (len >= 78)
                    {
                        send_to_user(cu, buf, 0, MSG_MESSAGE);
                        len = 0;
                    }
                } while ((list = list->next));

                if (len == 0)
                    return;
            }
            else
            {
                str = "�� �z�ثe�èS�� ignore ����H";
            }
        }
        str = buf;
    }

    send_to_user(cu, str, 0, MSG_MESSAGE);
}


static void
chat_unignore(
    ChatUser *cu,
    char *msg)
{
    char *ignoree, buf[80];
    const char *str;

    ignoree = nextword(&msg);

    if (*ignoree)
    {
        sprintf(buf, (list_delete(&(cu->ignore), ignoree)) ?
            "�� [%s] ���A�Q�A�N���F" :
            "�� �z�å� ignore [%s] �o���H��", ignoree);
        str = buf;
    }
    else
    {
        str = "�� �Ы��� user ID";
    }
    send_to_user(cu, str, 0, MSG_MESSAGE);
}


static void
chat_join(
    ChatUser *cu,
    char *msg)
{
    if (RESTRICTED(cu))
    {
        send_to_user(cu, "�� �z�S���[�J��L" CHATROOMNAME "���v��", 0, MSG_MESSAGE);
    }
    else
    {
        char *roomid = nextword(&msg);

        if (*roomid)
            enter_room(cu, roomid, msg);
        else
            send_to_user(cu, "�� �Ы��w" CHATROOMNAME, 0, MSG_MESSAGE);
    }
}


static void
chat_kick(
    ChatUser *cu,
    char *msg)
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
    {                       /* Thor.980604: �� userid�]���q */
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
        sprintf(buf, "�� ���i�H kick [%s]", twit);
        send_to_user(cu, buf, 0, MSG_MESSAGE);
        return;
    }

    exit_room(xuser, EXIT_KICK, (char *) NULL);

    if (room == &mainroom)
        xuser->uptime = 0;              /* logout_user(xuser); */
    else
        enter_room(xuser, MAIN_NAME, (char *) NULL);
        /* Thor.980602: ����N��, ���nshow�Xxxx���}�F���T������n */
}

static void
chat_makeop(
    ChatUser *cu,
    char *msg)
{
    char *newop, buf[80];
    ChatUser *xuser;
    ChatRoom *room;

    /* Thor.980603: PERM_CHATROOM�אּ default �S�� roomop, ���i�H�ۤv���o */

    newop = nextword(&msg);

    room = cu->room;

    if (!*newop && CHATSYSOP(cu))
    {
        /* Thor.980603: PERM_CHATROOM�אּ default �S�� roomop, ���i�H�ۤv���o */
        cu->uflag ^= PERM_CHATOP;

        user_changed(cu);
        if (!CLOAK(cu))
        {
            sprintf(buf, ROOMOP(cu) ? "�� " SYSOPNICK " �N Op �v�O�¤� %s"
                                    : "�� " SYSOPNICK " �N %s �� Op �v�O���^", cu->chatid);
            send_to_room(room, buf, 0, MSG_MESSAGE);
        }

        return;
    }

    /* if (!ROOMOP(cu)) */
    if (!(cu->uflag & (PERM_ROOMOP|PERM_CHATOP))) /* Thor.980603: chat room�`�ޤ����ಾ Op �v�O */
    {
        send_to_user(cu, "�� �z�����ಾ Op ���v�O" /* msg_not_op */, 0, MSG_MESSAGE);
        return;
    }

    xuser = cuser_by_chatid(newop);

#if 0
    if (xuser == NULL)
    {                       /* Thor.980604: �� userid�]���q */
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
        send_to_user(cu, "�� �z���N�w�g�O Op �F��", 0, MSG_MESSAGE);
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
        sprintf(buf, "�� %s �N Op �v�O�ಾ�� %s",
            cu->chatid, xuser->chatid);
        send_to_room(room, buf, 0, MSG_MESSAGE);
    }
}


static void
chat_invite(
    ChatUser *cu,
    char *msg)
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
    {                       /* Thor.980604: �� userid�]���q */
        xuser = cuser_by_userid(invitee);
    }
#endif

    if (xuser == NULL)
    {
        sprintf(buf, msg_no_such_id, invitee);
        send_to_user(cu, buf, 0, MSG_MESSAGE);
        return;
    }

    room = cu->room;            /* Thor: �O�_�n check room �O�_ NULL ? */
    list = &(room->invite);

    if (list_belong(*list, xuser->userno))
    {
        sprintf(buf, "�� %s �w�g�����L�ܽФF", xuser->chatid);
        send_to_user(cu, buf, 0, MSG_MESSAGE);
        return;
    }
    list_add(list, xuser);

    sprintf(buf, "�� %s �ܽбz�� [%s] " CHATROOMNAME,
        cu->chatid, room->name);
    send_to_user(xuser, buf, 0, MSG_MESSAGE);
    sprintf(buf, "�� %s ����z���ܽФF", xuser->chatid);
    send_to_user(cu, buf, 0, MSG_MESSAGE);
}


static void
chat_broadcast(
    ChatUser *cu,
    char *msg)
{
    char buf[80];

    if (!CHATSYSOP(cu))
    {
        send_to_user(cu, "�� �z�S���b" CHATROOMNAME "�s�����v�O!", 0, MSG_MESSAGE);
        return;
    }

    if (*msg == '\0')
    {
        send_to_user(cu, "�� �Ы��w�s�����e", 0, MSG_MESSAGE);
        return;
    }

    sprintf(buf, "\x1b[1m�� " BOARDNAME CHATROOMNAME "�s���� [%s].....\x1b[m",
        cu->chatid);
    send_to_room(ROOM_ALL, buf, 0, MSG_MESSAGE);
    sprintf(buf, "�� %s", msg);
    send_to_room(ROOM_ALL, buf, 0, MSG_MESSAGE);
}


static void
chat_bye(
    ChatUser *cu,
    char *msg)
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
party_action(
    ChatUser *cu,
    char *cmd,
    const char *party,
    int mode)
{
    ChatAction *cap;
    char *verb, buf[256];
    for (cap = mode ? party_data : party_data2; strlen(verb = cap->verb); cap++)
    {
        if (str_equal(verb, cmd))
        {
            if (*party == '\0')
            {
                party = "�j�a";
            }
            else
            {
                ChatUser *xuser;

                xuser = fuzzy_cuser_by_chatid(party);
                if (xuser == NULL)
                {                       /* Thor.0724: �� userid�]���q */
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
                    send_to_user(cu, "�� �Ы�����ѥN��", 0, MSG_MESSAGE);
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
            return 0;                   /* Thor: cu->room �O�_�� NULL? */
        }
    }
    return 1;
}


/* --------------------------------------------- */
/* MUD-like social commands : speak              */
/* --------------------------------------------- */

ChatAction *speak_data;

static int
speak_action(
    ChatUser *cu,
    char *cmd,
    char *msg)
{
    ChatAction *cap;
    char *verb, buf[256];

    for (cap = speak_data; strlen(verb = cap->verb); cap++)
    {
        if (str_equal(verb, cmd))
        {
            sprintf(buf, "\x1b[1;32m%s \x1b[31m%s�G\x1b[33m %s\x1b[m",
                cu->chatid, cap->part1_msg, msg);
            send_to_room(cu->room, buf, cu->userno, MSG_MESSAGE);
            return 0;
        }
    }
    return 1;
}


/* ----------------------------------------------------- */
/* MUD-like social commands : condition                  */
/* ----------------------------------------------------- */

ChatAction *condition_data, *person_data;

static int
condition_action(
    ChatUser *cu,
    char *cmd)
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
person_action(
    ChatUser *cu,
    char *cmd,
    const char *party)
{
    ChatAction *cap;
    char *verb, buf[256];

    for (cap = person_data; strlen(verb = cap->verb); cap++)
    {
        if (str_equal(verb, cmd))
        {
            if (!*(cap->part2_msg))
            {
                sprintf(buf, "\x1b[1;32m%s \x1b[31m%s\x1b[m",
                    cu->chatid, cap->part1_msg);
            }
            else
            {

                if (*party == '\0')
                {
                    party = "�j�a";
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
                        send_to_user(cu, "�� �Ы�����ѥN��", 0, MSG_MESSAGE);
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


static const char *const dscrb[] =
{
    "\x1b[1;37m�i Verb + Nick�G   �ʵ� + ���W�r �j\x1b[36m  �ҡG//kick piggy\x1b[m",
    "\x1b[1;37m�i Verb + Message�G�ʵ� + �n������ �j\x1b[36m  �ҡG//sing �ѤѤ���\x1b[m",
    "\x1b[1;37m�i Verb�G�ʵ� �j   �����G�¸ܭ���\x1b[m",
    "\x1b[1;37m�i Verb + Nick�G   �ʵ� + ���W�r �j\x1b[36m  �ҡG//kick piggy\x1b[m",
    "\x1b[1;37m�i Verb�G�ʵ� �j   �����G�ӤH���ʵ�\x1b[m",
    NULL
};


static ChatAction *catbl(int in)
{
    if (in==0) return party_data;
    else if (in==1) return speak_data;
    else if (in==2) return condition_data;
    else if (in==3) return party_data2;
    else if (in==4) return person_data;
    else return NULL;
}

static void
chat_partyinfo(
    ChatUser *cu,
    char *msg)
{
    if (common_client_command)
    {
        send_to_user(cu, "3 �ʧ@  ���  ���A", 0, MSG_PARTYINFO);
    }
}


static void
chat_party(
    ChatUser *cu,
    char *msg)
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

    /* Xshadow: �u�� condition �~�O immediate mode */
    send_to_user(cu, buf, 0, MSG_PARTYLISTSTART);

    cap = catbl(kind);
    for (i = 0; cap[i].verb[0]; i++)
    {
        sprintf(buf, "%-10s %-20s", cap[i].verb, cap[i].brief_desc);
        send_to_user(cu, buf, 0, MSG_PARTYLIST);
    }

    sprintf(buf, "%d", kind);
    send_to_user(cu, buf, 0, MSG_PARTYLISTEND);
}


#define SCREEN_WIDTH    80
#define MAX_VERB_LEN    8
#define VERB_NO         10


static void
view_action_verb(       /* Thor.0726: �s�[�ʵ�������� */
    ChatUser *cu,
    int cmd)
{
    int i;
    char *q, *data, *expn, buf[256];
    const char *p;
    ChatAction *cap;

    send_to_user(cu, "/c", 0, MSG_CLRSCR);

    data = buf;

    if (cmd < '1' || cmd > '5')
    {                           /* Thor.0726: �g�o���n, �Q��k��i... */
        for (i = 0; (p = dscrb[i]); i++)
        {
            sprintf(data, "  [//]help %d          - MUD-like ����ʵ�   �� %d ��", i + 1, i + 1);
            send_to_user(cu, data, 0, MSG_MESSAGE);
            send_to_user(cu, p, 0, MSG_MESSAGE);
            send_to_user(cu, " ", 0, MSG_MESSAGE);      /* Thor.0726: ���� */
        }
    }
    else
    {
        i = cmd - '1';

        send_to_user(cu, dscrb[i], 0, MSG_MESSAGE);

        expn = buf + 100;               /* Thor.0726: ���Ӥ��|overlap�a? */

        *data = '\0';
        *expn = '\0';

        cap = catbl(i);

        for (i = 0; strlen(p = cap[i].verb); i++)
        {
            q = cap[i].brief_desc;

            strcat(data, p);
            strcat(expn, q);

            if (((i + 1) % VERB_NO) == 0)
            {
                send_to_user(cu, data, 0, MSG_MESSAGE);
                send_to_user(cu, expn, 0, MSG_MESSAGE); /* Thor.0726: ��ܤ������ */
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
            send_to_user(cu, expn, 0, MSG_MESSAGE);     /* Thor.0726: ��ܤ������ */
        }
    }
    /* send_to_user(cu, " ", 0); *//* Thor.0726: ����, �ݭn " " ��? */
}


/* ----------------------------------------------------- */
/* chat user service routines                            */
/* ----------------------------------------------------- */


static const ChatCmd chatcmdlist[] =
{
    {"act", chat_act, 0},
    {"bye", chat_bye, 0},
    {"chatroom", chat_chatroom, 1},     /* Xshadow: for common client */
    {"clear", chat_clear, 0},
    {"cloak", chat_cloak, 2},
    {"mud", chat_mud, 2},
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
    {"party", chat_party, 1},           /* Xshadow: party data for common client */
    {"partyinfo", chat_partyinfo, 1},   /* Xshadow: party info for common
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


/* Thor: 0 ���� exact, 1 �n exactly equal, 2 ���K���O */


static int
command_execute(
    ChatUser *cu)
{
    char *cmd, *msg, buf[128];
    /* Thor.981108: lkchu patch: chatid + msg �u�� 80 bytes ����, �אּ 128 */
    const ChatCmd *cmdrec;
    int match, ch, check;

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
            if (cu->room && !CLOAK(cu)) /* �������H�]���໡�ܮ@ */
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
        if (*(cmd+1) <= '5' && *(cmd+1) >= '1')
        {
            view_action_verb(cu, *(cmd+1));
            check = 1;
        }

    if (*cmd == '/'&&!(*(cmd+1) <= '5' && *(cmd+1) >= '1'))
    {
        cmd++;
        if (!*cmd || str_equal("help", cmd))
        {
            cmd = nextword(&msg);       /* Thor.0726: �ʵ����� */
            view_action_verb(cu, *cmd);
            match = 1;
        }
        else if (party_action(cu, cmd, msg, 0) == 0)
            match = 1;
        else if (party_action(cu, cmd, msg, 1) == 0)
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
        const char *str;

        common_client_command = 0;
        if (*cmd == '-')
        {
            if (cu->clitype)
            {
                cmd++;                  /* Xshadow: ���O�q�U�@�Ӧr���~�}�l */
                common_client_command = 1;
            }
            else
            {
                /* ���O common client ���e�X common client ���O -> ���˨S�ݨ� */
            }
        }

        str_lower(buf, cmd);

        for (cmdrec = chatcmdlist; (str = cmdrec->cmdstr); cmdrec++)
        {
            switch (cmdrec->exact)
            {
            case 1:                     /* exactly equal */
                match = str_equal(str, buf);
                break;

            case 2:                     /* Thor: secret command */
                if (CHATSYSOP(cu))
                    match = str_equal(str, buf);
                break;

            default:                    /* not necessary equal */
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
        sprintf(buf, "�� ���O���~�G/%s", cmd);
        send_to_user(cu, buf, 0, MSG_MESSAGE);
    }

    return 0;
}


/* ----------------------------------------------------- */
/* serve chat_user's connection                          */
/* ----------------------------------------------------- */


static int
cuser_serve(
    ChatUser *cu)
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

        return 0;                       /* would block, so leave it to do later */
#endif
    }

    if (len == 0)
    {
        if (++cu->retry > 100)
            return -1;
        return 0;
    }

#if 1
    /* Xshadow: �N�e�F����Ʃ�������U�� */
    memcpy(logbuf, buf, sizeof(buf));
    for (ch = 0; ch < sizeof(buf); ch++)
        if (!logbuf[ch] || logbuf[ch] == '\r' || logbuf[ch] == '\n')
            logbuf[ch] = '$';

    logbuf[len + 1] = '\0';
    if (strncmp(logbuf, "/! ", 3))
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
servo_daemon(
    int inetd)
{
    int fd, value;
    char buf[80];
    struct addrinfo hints = {0};
    struct addrinfo *hosts;
    char port_str[12];
    struct linger ld;
#ifdef RLIMIT
    struct rlimit limit;
#endif //RLIMIT

    /* IID.2021-02-19: Fallback to native IPv4 when IPv6 is not available */
    static const sa_family_t ai_family[] = {AF_INET6, AF_INET};

    /*
     * More idiot speed-hacking --- the first time conversion makes the C
     * library open the files containing the locale definition and time zone.
     * If this hasn't happened in the parent process, it happens in the
     * children, once per connection --- and it does add up.
     */

    time((time_t *) &value);
    gmtime((time_t *) &value);
    strftime(buf, 80, "%d/%b/%Y:%H:%M:%S", localtime((time_t *) &value));

#ifndef NOIDENT
    dns_init();
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
    if (inetd)
        return 0;

    close(0);

    if (fork())
        exit(0);

    setsid();

    if (fork())
        exit(0);

    /* --------------------------------------------------- */
    /* bind the service port                               */
    /* --------------------------------------------------- */

    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_ADDRCONFIG | AI_NUMERICSERV | AI_PASSIVE;
    sprintf(port_str, "%d", CHAT_PORT);

    for (int i = 0; i < COUNTOF(ai_family); ++i)
    {
        hints.ai_family = ai_family[i];
        if (getaddrinfo(NULL, port_str, &hints, &hosts))
            continue;

        for (struct addrinfo *host = hosts; host; host = host->ai_next)
        {
            fd = socket(host->ai_family, host->ai_socktype, host->ai_protocol);
            if (fd < 0)
                continue;

            /*
             * timeout �譱, �N socket �令 O_NDELAY (no delay, non-blocking),
             * �p�G�බ�Q�e�X��ƴN�e�X, ����e�X�N��F, ���A���� TCP_TIMEOUT �ɶ��C
             * (default �O 120 ��, �åB�� 3-way handshaking ����, ���i��@���A��)�C
             */

#if 1
            fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NDELAY);
#endif

            value = 1;
            setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *) &value, sizeof(value));

            value = 1;
            setsockopt(fd, host->ai_protocol, TCP_NODELAY, (char *) &value, sizeof(value));

            ld.l_onoff = ld.l_linger = 0;
            setsockopt(fd, SOL_SOCKET, SO_LINGER, (char *) &ld, sizeof(ld));

            if ((bind(fd, host->ai_addr, host->ai_addrlen) < 0) ||
                (listen(fd, SOCK_QLEN) < 0))
            {
                close(fd);
                fd = -1;
                continue;
            }

            /* Success */
            break;
        }
        freeaddrinfo(hosts);

        if (fd >= 0)
            break; /* Success */
    }
    if (fd < 0)
        exit(1);

    return fd;
}


#ifdef  SERVER_USAGE
static void
server_usage(GCC_UNUSED int signum)
{
    struct rusage ru;

    if (getrusage(RUSAGE_SELF, &ru))
        return;

    fprintf(flog, "\n[Server Usage]\n\n"
        "user time: %.6f\n"
        "system time: %.6f\n"
        "maximum resident set size: %ld P\n"
        "integral resident set size: %ld\n"
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
#endif  /* #ifdef  SERVER_USAGE */


static void
reaper(GCC_UNUSED int signum)
{
    while (waitpid(-1, NULL, WNOHANG | WUNTRACED) > 0);
}


static void
sig_trap(
    int sig)
{
    char buf[80];

    sprintf(buf, "signal [%d] at line %d (errno: %d)", sig, gline, errno);
    logit("EXIT", buf);
    fclose(flog);
    exit(1);
}


static void
sig_over(int signum)
{
    int fd;

    server_usage(signum);
    logit("OVER", "");
    fclose(flog);
    for (fd = 0; fd < 64; fd++)
        close(fd);
    execl(BINARY_SUFFIX"xchatd", BINARY_SUFFIX"xchatd", (const char *)NULL);
}

static void
sig_log(GCC_UNUSED int signum)
{
    char buf[128];

    logit("OVER", "");
    fclose(flog);

    logtalk("OVER", "");
    fclose(ftalk);

    sprintf(buf, "%s.old", CHAT_LOGFILE);
    f_mv(CHAT_LOGFILE, buf);
    sprintf(buf, "%s.old", CHAT_TALKFILE);
    f_mv(CHAT_TALKFILE, buf);

    flog = fopen(CHAT_LOGFILE, "a+");
    logit("START", "chat daemon");

    ftalk = fopen(CHAT_TALKFILE, "a+");
    logtalk("START", "chat room");

}



static void
main_signals(void)
{
    struct sigaction act;

    /* sigblock(sigmask(SIGPIPE)); */
    /* Thor.981206: �Τ@ POSIX �зǥΪk  */

    /* act.sa_mask = 0; */ /* Thor.981105: �зǥΪk */
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;

    act.sa_handler = sig_trap;
    sigaction(SIGBUS, &act, NULL);
    sigaction(SIGSEGV, &act, NULL);
    sigaction(SIGTERM, &act, NULL);


    /* ��s������ */
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

    /* Thor.981206: lkchu patch: �Τ@ POSIX �зǥΪk  */
    /* �b���ɥ� sigset_t act.sa_mask */
    sigaddset(&act.sa_mask, SIGPIPE);
    sigprocmask(SIG_BLOCK, &act.sa_mask, NULL);

}


/*
static void *
attach_shm(
    int shmkey, int shmsize)
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
load_mud_like(void)
{
    if (mud == NULL)
    {
        //mud = (MUD *) attach_shm(MUD_KEY, sizeof(MUD));
        mud = (MUD *)malloc(sizeof(MUD));
        mudshm_init();
    }
}
*/

void
mudshm_init(void)
{
    ChatAction *head;
    int fw, size;
    struct stat st;


    mud = &muddata;

    head = party_data = mud->chat_party;
    fw = open(FN_CHAT_PARTY_DB, O_RDONLY);
    fstat(fw, &st);

    if (!fstat(fw, &st) && (size = st.st_size) > 0)
    {
        if (size > PARTY_MAX * sizeof(ChatAction))
            size = PARTY_MAX * sizeof(ChatAction);
        memset(head, 0, PARTY_MAX * sizeof(ChatAction));
        if (size)
            read(fw, head, size);
    }
    close(fw);

    head = party_data2 = mud->chat_party2;
    fw = open(FN_CHAT_PARTY2_DB, O_RDONLY);
    fstat(fw, &st);

    if (!fstat(fw, &st) && (size = st.st_size) > 0)
    {
        if (size > PARTY_MAX * sizeof(ChatAction))
            size = PARTY_MAX * sizeof(ChatAction);
        memset(head, 0, PARTY_MAX * sizeof(ChatAction));
        if (size)
            read(fw, head, size);
    }
    close(fw);

    head = speak_data = mud->chat_speak;
    fw = open(FN_CHAT_SPEAK_DB, O_RDONLY);
    fstat(fw, &st);

    if (!fstat(fw, &st) && (size = st.st_size) > 0)
    {
        if (size > SPEAK_MAX * sizeof(ChatAction))
            size = SPEAK_MAX * sizeof(ChatAction);
        memset(head, 0, SPEAK_MAX * sizeof(ChatAction));
        if (size)
            read(fw, head, size);
    }
    close(fw);

    head = condition_data = mud->chat_condition;
    fw = open(FN_CHAT_CONDITION_DB, O_RDONLY);
    fstat(fw, &st);

    if (!fstat(fw, &st) && (size = st.st_size) > 0)
    {
        if (size > CONDITION_MAX * sizeof(ChatAction))
            size = CONDITION_MAX * sizeof(ChatAction);
        memset(head, 0, CONDITION_MAX * sizeof(ChatAction));
        if (size)
            read(fw, head, size);
    }
    close(fw);

    head = person_data = mud->chat_person;
    fw = open(FN_CHAT_PERSON_DB, O_RDONLY);
    fstat(fw, &st);

    if (!fstat(fw, &st) && (size = st.st_size) > 0)
    {
        if (size > CONDITION_MAX * sizeof(ChatAction))
            size = CONDITION_MAX * sizeof(ChatAction);
        memset(head, 0, CONDITION_MAX * sizeof(ChatAction));
        if (size)
            read(fw, head, size);
    }
    close(fw);

}


int
main(
    int argc,
    char *argv[])
{
    int sock, nfds, maxfds=0, servo_sno;
    ChatUser *cu, /* *userpool, */ **FBI;
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
            exit(2);
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
    /* init variable : rooms & users                       */
    /* --------------------------------------------------- */

    userpool = NULL;
    strcpy(mainroom.name, MAIN_NAME);
    strcpy(mainroom.topic, MAIN_TOPIC);
    mainroom.rflag |= ROOM_OPENTOPIC;

    /* --------------------------------------------------- */
    /* main loop                                           */
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
                    maxfds = BMAX(maxfds, sock);

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
        /* Set up the fdsets                                 */
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

            if ((++xxx % 0x2000U) == 0)
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
        /* serve active agents                               */
        /* ------------------------------------------------- */

        uptime = time(0);

        for (FBI = &mainuser; (cu = *FBI);)
        {
            sock = cu->sock;

            if (FD_ISSET(sock, &rset))
            {
                static int xxx, xno;

                nfds = cuser_serve(cu);

                if ((++xxx % 512U) == 0)
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

            if (nfds < 0 || cu->uptime <= 0)    /* free this client */
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
        /* accept new connection                             */
        /* ------------------------------------------------- */

        if (FD_ISSET(0, &rset))
        {

            {
                static int yyy;

                if ((++yyy % 2048U) == 0)
                    fprintf(flog, "conn\t%d\n", yyy);
            }

            for (;;)
            {
                int value;
                ip_addr sin;

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
                    memcpy(cu->rhost, &sin, sizeof(ip_addr));

                    totaluser++;

                    FD_SET(sock, &mainfset);
                    maxfds = BMAX(maxfds, sock + 1);

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
        /* tail of main loop                                 */
        /* ------------------------------------------------- */

    }
}
