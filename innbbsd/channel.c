/*-------------------------------------------------------*/
/* channel.c    ( NTHU CS MapleBBS Ver 3.10 )            */
/*-------------------------------------------------------*/
/* target : innbbsd main program                         */
/* create : 95/04/27                                     */
/* update :   /  /                                       */
/* author : skhuang@csie.nctu.edu.tw                     */
/* modify : itoc.bbs@bbs.tnfsh.tn.edu.tw                 */
/*-------------------------------------------------------*/


#include "innbbsconf.h"
#include "bbslib.h"
#include "inntobbs.h"
#include "nntp.h"
#include <time.h>
#include <sys/time.h>

#ifdef NoCeM
#include "nocem.h"
#endif


#define INNBBSD_PIDFILE "run/innbbsd.pid"


/* ----------------------------------------------------- */
/* my recv                                               */
/* ----------------------------------------------------- */

typedef struct ClientType ClientType;
typedef struct
{
    const char *name;
    const char *usage;
    int minargc;                /* argc �ִ̤X�� */
    int maxargc;                /* argc �̦h�X�� */
    int mode;                   /* 0:�ncommand-mode�~��]  1:�ndata-mode�~��]  2:�S������ */
    int errorcode;
    int normalcode;
    void (*main) (ClientType *client);
}       daemoncmd_t;


typedef struct
{
    FILE *in, *out;
    int argc;
    char **argv;
    daemoncmd_t *dc;
}       argv_t;


typedef struct
{
    char *data;
    int used;
    int left;
}       buffer_t;


struct ClientType
{
    char nodename[13];
    char hostname[128];         /* client hostname */
    char buffer[4096];

    int mode;                   /* 1:data mode  0:command mode */
    argv_t Argv;

    int fd;
    buffer_t in;
    buffer_t out;
};


#if 0   /* itoc.030109.����: my_recv ���y�{ */
            �z�� receive_article() �� bbspost_add()
  my_recv() �u�� receive_nocem()   �� �e�h nocem.c �B�z
            �|�� cancel_article()  �� bbspost_cancel()
#endif


static void
my_recv(
    ClientType *client)
{
    FILE *fout;
    int rel;
    const char *ptr;

    fout = client->Argv.out;

    rel = readlines(client->in.data + 2);

    if (rel > 0)
    {
        rel = 0;
        if ((ptr = CONTROL))
        {
            if (!str_ncasecmp(ptr, "cancel ", 7))
            {
                /* itoc.030127: cancel �����٬O�n�~�򦬨�L�ʫH */
                /* rel = cancel_article(ptr + 7); */
                cancel_article(ptr + 7);
            }
        }
        else
        {
#ifdef NoCeM
            if (strstr(SUBJECT, "@@") && strstr(BODY, "NCM") && strstr(BODY, "PGP"))
                rel = receive_nocem();
            else
#endif
                rel = receive_article();
        }

        if (rel == -1)
            fprintf(fout, "400 server side failed\r\n");
        else
            fprintf(fout, "235\r\n");
    }
    else if (rel == 0)          /* PATH�]�A�ۤv */
    {
        fprintf(fout, "235\r\n");
    }
    else /* if (rel < 0) */     /* ���Y��줣���� */
    {
        fputs("437\r\n", fout);
    }

    fflush(fout);
}


/* ----------------------------------------------------- */
/* command sets                                          */
/* ----------------------------------------------------- */


#ifdef __cplusplus
  #define INTERNAL extern  /* Used inside an anonymous namespace */
  #define INTERNAL_INIT /* Empty */
#else
  #define INTERNAL static
  #define INTERNAL_INIT static
#endif

#ifdef __cplusplus
namespace {
#endif
INTERNAL daemoncmd_t cmds[];
#ifdef __cplusplus
}  // namespace
#endif


GCC_PURE static daemoncmd_t *
searchcmd(
    const char *cmd)
{
    daemoncmd_t *p;
    const char *name;

    for (p = cmds; (name = p->name); p++)
    {
        if (!str_casecmp(name, cmd))
            return p;
    }
    return NULL;
}


#define MAX_ARG         16
#define MAX_ARG_SIZE    1024


static int
argify(
    const char *line, char ***argvp)
{
    static char *argvbuffer[MAX_ARG + 2];
    char **argv = argvbuffer;
    int i;
    static char argifybuffer[MAX_ARG_SIZE];
    char *p;

    while (strchr("\t\n\r ", *line))
        line++;
    p = argifybuffer;
    strncpy(p, line, sizeof(argifybuffer));
    for (*argvp = argv, i = 0; *p && i < MAX_ARG;)
    {
        for (*argv++ = p; *p && !strchr("\t\r\n ", *p); p++);
        if (*p == '\0')
            break;
        for (*p++ = '\0'; strchr("\t\r\n ", *p) && *p; p++);
    }
    *argv = NULL;
    return argv - *argvp;
}


/* ----------------------------------------------------- */
/* innd channel                                          */
/* ----------------------------------------------------- */


static fd_set rfd;              /* read fd_set */


static void
reapchild(
    int s)
{
    int state;
    while (waitpid(-1, &state, WNOHANG | WUNTRACED) > 0)
    {
        /* printf("reaping child\n"); */
    }
}


static void
dokill(
    int s)
{
    kill(0, SIGKILL);
}


static int                      /* -1:���� */
initinetserver(void)
{
    struct addrinfo *hosts;      /* host information entries */
    struct addrinfo hints = {0}; /* Internet endpoint hints */
    int fd, value;
    struct linger foobar;
    char port_str[12];

    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_V4MAPPED | AI_ADDRCONFIG | AI_NUMERICSERV | AI_PASSIVE;

    sprintf(port_str, "%d", INNBBS_PORT);

    if (getaddrinfo(NULL, port_str, &hints, &hosts))
        return -1;

    for (struct addrinfo *host = hosts; host; host = host->ai_next)
    {
        /* Allocate a socket */
        fd = socket(host->ai_family, host->ai_socktype, host->ai_protocol);
        if (fd < 0)
            continue;

        if (bind(fd, host->ai_addr, host->ai_addrlen) < 0)
        {
            printf("innbbsd �w�� inetd �ҰʤF�A�L�ݦA��ʰ���\n");
            close(fd);
            freeaddrinfo(hosts);
            return -1;
        }

        /* Success */
        listen(fd, 10);
        break;
    }
    freeaddrinfo(hosts);
    if (fd < 0)
    {
        printf("inet socket �}�ҥ���\n");
        return -1;
    }

    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *) &value, sizeof(value)) < 0)
        bbslog("setsockopt (SO_REUSEADDR)");
    foobar.l_onoff = 0;
    if (setsockopt(fd, SOL_SOCKET, SO_LINGER, (char *) &foobar, sizeof(foobar)) < 0)
        bbslog("setsockopt (SO_LINGER)");

    signal(SIGHUP, SIG_IGN);
    signal(SIGUSR1, SIG_IGN);
    signal(SIGCHLD, reapchild);
    signal(SIGINT, dokill);
    signal(SIGPIPE, dokill);
    signal(SIGTERM, dokill);

    return fd;
}


static int
tryaccept(
    int s)
{
    int ns;
    socklen_t fromlen = sizeof(struct sockaddr_storage);
    struct sockaddr_storage sockaddr;   /* Internet endpoint address */

    do
    {
        ns = accept(s, (struct sockaddr *)&sockaddr, &fromlen);
        errno = 0;
    } while (ns < 0 && errno == EINTR);
    return ns;
}


static void
channelcreate(
    ClientType *client,
    int sock,
    const char *nodename, const char *hostname)
{
    buffer_t *in, *out;

    str_scpy(client->nodename, nodename, 13);
    str_scpy(client->hostname, hostname, 128);

    client->fd = sock;
    FD_SET(sock, &rfd);
    client->Argv.in = fdopen(sock, "r");
    client->Argv.out = fdopen(sock, "w");

    client->buffer[0] = '\0';
    client->mode = 0;

    in = &client->in;
    free(in->data);
    in->data = (char *) malloc(ChannelSize * 4);
    in->left = ChannelSize * 4;
    in->used = 0;

    out = &client->out;
    free(out->data);
    out->data = (char *) malloc(ChannelSize);
    out->left = ChannelSize;
    out->used = 0;
}


static void
channeldestroy(
    ClientType *client)
{
    FD_CLR(client->fd, &rfd);
    fclose(client->Argv.in);
    fclose(client->Argv.out);
    close(client->fd);
    client->fd = -1;

    free(client->in.data);
    client->in.data = NULL;
    free(client->out.data);
    client->out.data = NULL;
}


static int
channelreader(
    ClientType *client)
{
    int len, used;
    char *data, *head;
    buffer_t *in;

    NODENAME = client->nodename;

    in = &client->in;
    len = in->left;
    used = in->used;
    data = in->data;
    if (len < ReadSize + 3)
    {
        len += (used + ReadSize);
        len += (len >> 3);

        in->data = data = (char *) realloc(data, len);
        len -= used;
        in->left = len;
    }

    head = data + used;
    len = recv(client->fd, head, ReadSize, 0);
    if (len <= 0)
        return len;

    head[len] = '\0';
    head[len + 1] = '\0';

    if (client->mode)           /* data mode */
    {
        char *dest;
        int cc;

        dest = head - 1;

        for (;;)
        {
            cc = *head;

            if (!cc)
            {
                used = dest - data + 1;
                in->left -= (used - in->used);
                in->used = used;
                return len;
            }

            head++;

            if (cc == '\r')
                continue;

            if (cc == '\n')
            {
                used = *dest;

                if (used == '.')
                {
                    if (dest[-1] == '\n')
                        break;          /* end of article body */
                }
                else
                {
                    /* strip the trailing space */
                    /* Thor.990110: ���� �Ŧ�\n �� space, for multi-line merge */
                    /* while (used == ' ' || used == '\t') */
                    while (dest[-1] != '\n' && (used == ' ' || used == '\t'))
                        used = *--dest;
                }
            }

            *++dest = cc;
        }

        /* strip the trailing empty lines */
        *data = '\0';
        while (*--dest == '\n')
            ;
        dest += 2;
        *dest = '\0';

        my_recv(client);
        client->mode = 0;
    }
    else                        /* command mode */
    {
        argv_t *argv;
        daemoncmd_t *dp;
        FILE *out;

        head = (char *) strchr(head, '\n');

        if (head == NULL)
        {
            in->used += len;
            in->left -= len;
            return len;
        }

        *head++ = '\0';

        argv = &client->Argv;
        argv->argc = argify(data, &argv->argv);
        argv->dc = dp = searchcmd(argv->argv[0]);
        out = argv->out;

        if (dp)
        {
            if ((argv->argc < dp->minargc) || (argv->argc > dp->maxargc))               /* �ˬd argc �O�_�����n�D */
            {
                fprintf(out, "%d Usage: %s\r\n", dp->errorcode, dp->usage);
                fflush(out);
            }
            else if ((dp->mode == 0 || dp->mode == 1) && (client->mode != dp->mode))    /* �ˬd data/command mode �O�_�����n�D */
            {
                fprintf(out, "%d %s error\r\n", dp->errorcode, dp->name);
                fflush(out);
            }
            else                /* �q�L�H�W�T�D�ˬd */
            {
                void (*Main) (ClientType *client);

                if ((Main = dp->main))
                    (*Main) (client);
            }
        }
        else
        {
            fprintf(out, "500 Syntax error or bad command\r\n");
            fflush(out);
        }
    }

    /* Thor.980825: gc patch: �p�G�w�g����L CMDquit, �U�����ʧ@�����ΰ��F(client destroyed) :) */
    /* if (client->mode == 0) */
    if (client->mode == 0 && client->in.data != NULL)
    {
        int left;

        left = in->left + in->used;

        if ((used = *head))
        {
            char *str;

            str = data;
            while ((*str++ = *head++))
                ;

            used = str - data;
        }

        in->left = left - used;
        in->used = used;
    }

    return len;
}


/* ----------------------------------------------------- */
/* command set                                           */
/* ----------------------------------------------------- */


static int inetdstart = 0;


static void
CMDhelp(
    ClientType *client)
{
    argv_t *argv = &client->Argv;
    daemoncmd_t *p = argv->dc;
    FILE *out = argv->out;

    client->mode = 0;
    fprintf(out, "%d Available Commands\r\n", p->normalcode);
    for (p = cmds; p->name; p++)
        fprintf(out, "  %s\r\n", p->usage);
    fprintf(out, "Report problems to " STR_SYSOP ".bbs@" MYHOSTNAME "\r\n");
    fputs(".\r\n", out);
    fflush(out);
}


static void
CMDihave(
    ClientType *client)
{
    argv_t *argv = &client->Argv;
    daemoncmd_t *p = argv->dc;
    FILE *out = argv->out;
    char *data = argv->argv[1];

    if (data[0] != '<')
    {
        fprintf(out, "%d Bad Message-ID\r\n", p->errorcode);
    }
    else
    {
        fprintf(out, "%d\r\n", p->normalcode);
        client->mode = 1;

        strcpy(client->in.data, "\n\n\n");
        client->in.left += client->in.used - 3;
        client->in.used = 3;
    }
    fflush(out);
}


static void
CMDstat(
    ClientType *client)
{
    argv_t *argv = &client->Argv;
    daemoncmd_t *p = argv->dc;
    FILE *out = argv->out;
    char *data = argv->argv[1];

    if (data[0] != '<')         /* �u�䴩 <msgid> �d�� */
    {
        fprintf(out, "%d We does NOT support article number stating\r\n", p->errorcode);
    }
    else                        /* �d�߬O�_�w�g���� <msgid> */
    {
        if (HISfetch(data, NULL, NULL))
            fprintf(out, "%d 0 %s article received\r\n", p->normalcode, data);
        else
            fprintf(out, "%d No such article\r\n", p->errorcode);
    }
    fflush(out);
}


static void
CMDquit(
    ClientType *client)
{
    argv_t *argv = &client->Argv;
    daemoncmd_t *p = argv->dc;
    FILE *out = argv->out;

    client->mode = 0;
    fprintf(out, "%d quit\r\n", p->normalcode);
    fflush(out);

    channeldestroy(client);
}


#ifdef __cplusplus
namespace {
#endif
INTERNAL_INIT daemoncmd_t cmds[] =
{
    /* cmd-name, cmd-usage, min-argc, max-argc, mode, errorcode,             normalcode,           cmd-func */
    {"help",     "help [cmd]",            1, 2, 2, 0,                        NNTP_HELPOK_VAL,      CMDhelp},
    {"quit",     "quit",                  1, 1, 2, 0,                        NNTP_GOODBYE_ACK_VAL, CMDquit},
    {"ihave",    "ihave mid",             2, 2, 0, NNTP_HAVEIT_VAL,          NNTP_SENDIT_VAL,      CMDihave},
    {"stat",     "stat <mid>",            2, 2, 0, NNTP_NOTHING_FOLLOWS_VAL, NNTP_DONTHAVEIT_VAL,  CMDstat},
    {NULL,       NULL,                    0, 0, 2, 0,                        0,                    NULL}
};
#ifdef __cplusplus
}  // namespace
#endif


/* ----------------------------------------------------- */
/* �D�{��                                                */
/* ----------------------------------------------------- */


static const char *             /* �Ǧ^�O�ѭ��@�ӯ��x���H�i�Ӫ� */
search_nodelist_byhost(
    const char *hostname)
{
    nodelist_t *find;
    struct addrinfo *hes;
    int i;

    /* itoc.021216: �� NODELIST ������ host�A�o�˪��ܡA
       �p�G nodelist.bbs �̭��񪺤��O���ѡA����٬O�i�H access */

    for (i = 0; i < NLCOUNT; i++)
    {
        find = NODELIST + i;
        if (!getaddrinfo(find->host, NULL, NULL, &hes))
        {
            for (struct addrinfo *he = hes; he; he = he->ai_next)
            {
                char client[128];
                getnameinfo(he->ai_addr, he->ai_addrlen, client, sizeof(client), NULL, NI_MAXSERV, NI_NUMERICHOST);
                if (!strcmp(hostname, client))
                {
                    freeaddrinfo(hes);
                    return find->name;
                }
            }
            freeaddrinfo(hes);
        }
    }

    return NULL;
}


static time_t
filetime(                       /* �Ǧ^ fpath ���ɮ׮ɶ� */
    const char *fpath)
{
    struct stat st;

    if (!stat(fpath, &st))
        return st.st_mtime;
    return 0;
}


static void
inndchannel(void)
{
    int i, fd, sock;
    const char *nodename;
    time_t uptime1;             /* time to maintain history */
    time_t uptime2;             /* time in initial_bbs */
    time_t now;
    struct tm *ptime;
    struct timeval to;
    fd_set orfd;                /* temp read fd_set */
    struct sockaddr_storage sin;
    ClientType *clientp;
    ClientType Channel[MAXCLIENT];

    /* --------------------------------------------------- */
    /* initial server                                      */
    /* --------------------------------------------------- */

    if (inetdstart)             /* inetd �Ұ� */
    {
        sock = 0;
    }
    else                        /* standalone */
    {
        if ((sock = initinetserver()) < 0)
            return;
    }

    FD_ZERO(&rfd);
    FD_SET(sock, &rfd);

    /* --------------------------------------------------- */
    /* initial history maintain time                       */
    /* --------------------------------------------------- */

    time(&uptime1);
    ptime = localtime(&uptime1);
    i = (HIS_MAINT_HOUR - ptime->tm_hour) * 3600 + (HIS_MAINT_MIN - ptime->tm_min) * 60;
    uptime1 += i;

    uptime2 = 0;                /* force to initial_bbs in the first time */

    /* --------------------------------------------------- */
    /* initial channel                                     */
    /* --------------------------------------------------- */

    memset(Channel, 0, sizeof(Channel));

    for (i = 0; i < MAXCLIENT; i++)
    {
        clientp = Channel + i;
        clientp->fd = -1;
    }

    /* --------------------------------------------------- */
    /* main loop                                           */
    /* --------------------------------------------------- */

    for (;;)
    {
        time(&now);

        /* When to maintain history files. */
        if (now > uptime1)
        {
            HISmaint();
            uptime1 += 86400;
        }

        /* �Y�ɮפ� uptime2 �ٷs���ܡA���򭫷s���J */
        if (filetime("innd/nodelist.bbs") > uptime2 ||
            filetime("innd/newsfeeds.bbs") > uptime2 ||
#ifdef NoCeM
            filetime("innd/ncmperm.bbs") > uptime2 ||
#endif
            filetime("innd/spamrule.bbs") > uptime2)
        {
            if (!initial_bbs())
                return;
            uptime2 = now;
        }

        /* in order to maintain history, timeout every 20 minutes in case no connections */
        to.tv_sec = 60 * 20;
        to.tv_usec = 0;
        orfd = rfd;
        if (select(FD_SETSIZE, &orfd, NULL, NULL, &to) <= 0)
            continue;

        /* ���H�ӳX�ݤF */

        if (FD_ISSET(sock, &orfd))              /* ��W�� */
        {
            char hostname[256];
            if ((fd = tryaccept(sock)) < 0)
                continue;

            /* �ˬd���S���b nodelist.bbs �̭� */
            i = sizeof(sin);            /* �ɥ� i */
            if (getpeername(fd, (struct sockaddr *) &sin, (socklen_t *) &i) < 0)
            {
                close(fd);
                continue;
            }
            getnameinfo((struct sockaddr *) &sin, i, hostname, sizeof(hostname), NULL, NI_MAXSERV, NI_NUMERICHOST);
            if (!(nodename = search_nodelist_byhost(hostname)))
            {
                char buf[256];

                bbslog("<channel> :Warn: %s �չϳs�������A���䤣�b nodelist.bbs �W�椤\n", hostname);
                sprintf(buf, "502 �z���b������ nodelist.bbs �W�椤 (%s).\r\n", hostname);
                write(fd, buf, strlen(buf));
                close(fd);
                continue;
            }

            /* ��@�ӪŪ� ClientType */
            for (i = 0; i < MAXCLIENT; i++)
            {
                clientp = Channel + i;
                if (clientp->fd == -1)
                    break;
            }
            if (i == MAXCLIENT)
            {
                static const char msg_no_desc[] = "502 �ثe�s�u�H�ƹL�h�A�еy��A��\r\n";

                write(fd, msg_no_desc, sizeof(msg_no_desc)-1);
                close(fd);
                continue;
            }

            channelcreate(clientp, fd, nodename, hostname);

            fprintf(clientp->Argv.out, "200 INNBBSD %s (%s)\r\n", VERSION, hostname);
            fflush(clientp->Argv.out);
        }

        /* ����Ҧ� ClientType ���ШD�A�òM���S�b�Ϊ� ClientType */
        for (i = 0; i < MAXCLIENT; i++)
        {
            clientp = Channel + i;
            fd = clientp->fd;

            if ((fd >= 0) && FD_ISSET(fd, &orfd) && (channelreader(clientp) <= 0))
                channeldestroy(clientp);
        }
    }
}


#ifdef SOLARIS

#include <sys/resource.h>

#ifdef RLIMIT
static int
getdtablesize(void)
{
    struct rlimit limit;

    if (getrlimit(RLIMIT_NOFILE, &limit) >= 0)
        return limit.rlim_cur;
    return -1;
}
#endif //RLIMIT
#endif  /* #ifdef SOLARIS */


static void
standaloneinit(void)
{
    int ndescriptors, s;
    FILE *fp;

    ndescriptors = getdtablesize();
    if (!inetdstart)
    {
        if (fork())
            exit(0);
    }

    for (s = 3; s < ndescriptors; s++)
        close(s);

    if ((fp = fopen(INNBBSD_PIDFILE, "w")))
    {
        fprintf(fp, "%d\n", getpid());
        fclose(fp);
    }
}


static void
usage(
    const char *argv)
{
    fprintf(stderr, "Usage: %s [options]\n", argv);
    fprintf(stderr, "       -i        �H inetd wait option �Ұ�\n");
}


int
main(
    int argc,
    char *argv[])
{
    int c;
    struct sockaddr_storage sin;

    setgid(BBSGID);
    setuid(BBSUID);
    chdir(BBSHOME);
    umask(077);

    while ((c = getopt(argc, argv, "i")) != -1)
    {
        switch (c)
        {
        case 'i':
            c = sizeof(sin);
            if (getsockname(0, (struct sockaddr *) &sin, (socklen_t *) &c) < 0)
            {
                printf("�z���O�q inetd �ҰʡA�L�ݨϥ� -i\n");
                exit(0);
            }
            inetdstart = 1;
            break;

        default:
            usage(argv[0]);
            exit(2);
        }
    }

    init_bshm();
    standaloneinit();
    inndchannel();

    exit(0);
}
