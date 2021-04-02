/*-------------------------------------------------------*/
/* bbsd.c       ( NTHU CS MapleBBS Ver 3.00 )            */
/*-------------------------------------------------------*/
/* author : opus.bbs@bbs.cs.nthu.edu.tw                  */
/* target : BBS daemon/main/login/top-menu routines      */
/* create : 95/03/29                                     */
/* update : 96/10/10                                     */
/*-------------------------------------------------------*/

#define MAIN_C

#include "bbs.h"
#include "dns.h"

#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/telnet.h>
#include <sys/resource.h>


#define QLEN            3
#define PID_FILE        "run/bbs.pid"
#define PID_FILE_INET   "run/bbs_inet.pid"
#define PID_FILE_UNIXSOCKET "run/bbs_unixsocket.pid"
#define LOG_FILE        "run/bbs.log"
#undef  SERVER_USAGE

/* IID.2021-02-09: The global message logger */
Logger g_logger = {
    .file = NULL,
    .path = LOG_FILE,
    .lv_skip = LOGLV_WARN,
};

#define MAXPORTS        COUNTOF(myports)
static const int myports[] = {23, 3456, 3001, /* 3002, 3003 */};

/* Thor.990113: exports for anonymous log */
/* static */ char rusername[40];

/* static int mport; */ /* Thor.990325: ���ݭn�F:P */
static ip_addr tn_addr;

/* IID.20190903: The unix socket path for listening proxy connections */
static const char *unix_path;


#ifdef  TREAT
int treat=0;
#endif

/* ----------------------------------------------------- */
/* ���} BBS �{��                                         */
/* ----------------------------------------------------- */

void
blog_pid(
    const char *mode,
    const char *msg,
    pid_t pid
)
{
    char buf[512], data[256];
    time_t now;

    time(&now);
    if (!msg)
    {
        msg = data;
        sprintf(data, "Stay: %d (%d)", (int)(now - ap_start) / 60, pid);
    }

    sprintf(buf, "%s %-5.5s %-13s%s\n", Etime(&now), mode, cuser.userid, msg);
    f_cat(FN_USIES, buf);
}

void
blog(
    const char *mode,
    const char *msg
)
{
    blog_pid(mode, msg, currpid);
}


#ifdef MODE_STAT
void
log_modes(void)
{
    time(&modelog.logtime);
    rec_add(FN_MODE_CUR, &modelog, sizeof(UMODELOG));
}
#endif

#ifdef  TRANUFO
typedef struct
{
    unsigned int old;
    unsigned int new_;
}       TABLE;

const TABLE table[] = {
//  {UFO_COLOR, UFO2_COLOR},
//  {UFO_MOVIE, UFO2_MOVIE},
//  {UFO_BRDNEW, UFO2_BRDNEW},
//  {UFO_BNOTE, UFO2_BNOTE},
//  {UFO_VEDIT, UFO2_VEDIT},
//  {UFO_PAL, UFO2_PAL},
//  {UFO_MOTD, UFO2_MOTD},
//  {UFO_MIME, UFO2_MIME},
//  {UFO_SIGN, UFO2_SIGN},
//  {UFO_SHOWUSER, UFO2_SHOWUSER},
//  {UFO_REALNAME, UFO2_REALNAME},
//  {UFO_SHIP, UFO2_SHIP},
//  {UFO_NWLOG, UFO2_NWLOG},
//  {UFO_NTLOG, UFO2_NTLOG},
    {UFO_ACL, UFO2_ACL},
    {0, 0}
};

#endif  /* #ifdef  TRANUFO */

void
u_exit(
    const char *mode
)
{
    int fd, delta;
    ACCT tuser;
    char fpath[80];

    /* �h�X�̫�ݪ����ӪO */
    if (currbno >= 0)
    {
        //����H���ܦ��t�ơF�t�ƪ����k�s
        bshm->mantime[currbno] = BMAX(bshm->mantime[currbno]-1, 0);
    }

    utmp_free();                        /* ���� UTMP shm */
    blog(mode, NULL);

    if (cuser.userlevel)
    {
        ve_backup();            /* �s�边�۰ʳƥ� */
        brh_save();                     /* �x�s�\Ū�O���� */

#ifdef  HAVE_DETECT_CROSSPOST
        attr_put(cuser.userid, ATTR_CROSS_KEY, &cksum); /* �x�s CrossPost ���� */
#endif

    }

    /* Thor.980405: �����ɥ��w�R�����T */
#ifndef LOG_BMW        /* lkchu.981201: �i�b config.h �]�w */

    usr_fpath(fpath, cuser.userid, FN_BMW);
    unlink(fpath);

#endif

#ifdef MODE_STAT
    log_modes();
#endif

    usr_fpath(fpath, cuser.userid, FN_ACCT);
    fd = open(fpath, O_RDWR);
    if (fd >= 0)
    {
        if (read(fd, &tuser, sizeof(ACCT)) == sizeof(ACCT))
        {
            delta = time(&cuser.lastlogin) - ap_start;
            cuser.staytime += delta;
            /* lkchu.981201: �� delta, �C���W�����n�W�L�T�����~�� */
            if (delta > 3 * 60)
            {
                cuser.numlogins++;
            }
#ifdef HAVE_SONG
            cuser.request = tuser.request;
            if (cuser.request>500)
                cuser.request = 500;
            else if (cuser.request<=0)
                cuser.request = 0;
#endif
            cuser.money = tuser.money;
            cuser.userlevel = tuser.userlevel;
            cuser.tvalid = tuser.tvalid;
            cuser.vtime = tuser.vtime;
            cuser.deny = tuser.deny;
            if (tuser.numlogins > cuser.numlogins)
                cuser.numlogins = tuser.numlogins;
            if (tuser.numposts > cuser.numposts)
                cuser.numposts = tuser.numposts;
            if (tuser.staytime > cuser.staytime)
                cuser.staytime = tuser.staytime;

            strcpy(cuser.justify, tuser.justify);
            strcpy(cuser.vmail, tuser.vmail);
#ifdef  TRANUFO
            {
                const TABLE *ptr;
                cuser.ufo2 = 0;
                for (ptr = table; ptr->old; ptr++)
                {
                    if (cuser.ufo & ptr->old)
                        cuser.ufo2 |= ptr->new_;
                    else
                        cuser.ufo2 &= ~ptr->new_;
                }
            }
#endif
#ifdef  HAVE_CLASSTABLEALERT
            if (!utmp_find(cuser.userno))
                classtable_free();
//          vmsg("test");
#endif
            lseek(fd, (off_t) 0, SEEK_SET);
            write(fd, &cuser, sizeof(ACCT));
        }
        close(fd);
    }
}


GCC_NORETURN void
abort_bbs(void)
{
    if (bbstate)
        u_exit("AXED");

    exit(0);
}


static void
login_abort(
    const char *msg
)
{
    outs(msg);
    refresh();
    blog("LOGIN", currtitle);
    exit(0);
}


/* Thor.980903: lkchu patch: ���ϥΤW���ӽбb����, �h�U�C function������ */

#ifdef LOGINASNEW

/* ----------------------------------------------------- */
/* �ˬd user ���U���p                                    */
/* ----------------------------------------------------- */


static bool
belong(
    const char *flist,
    const char *key
)
{
    int fd;
    bool rc;

    rc = false;
    fd = open(flist, O_RDONLY);
    if (fd >= 0)
    {
        char *str;
        mgets(-1);

        while ((str = mgets(fd)))
        {
            str_lower(str, str);
            if (str_casestr(key, str))
            {
                rc = true;
                break;
            }
        }

        close(fd);
    }
    return rc;
}


static bool
is_badid(
    const char *userid
)
{
    int ch;
    const char *str;

    if (strlen(userid) < 2)
        return true;

    if (!is_alpha(*userid))
        return true;

    if (!str_casecmp(userid, STR_NEW))
        return true;

    str = userid;
    while ((ch = *(++str)))
    {
        if (!is_alnum(ch))
            return true;
    }
    return (belong(FN_ETC_BADID, userid));
}


static int
uniq_userno(
    int fd
)
{
    char buf[4096];
    int userno, size;
    SCHEMA *sp;                 /* record length 16 �i�㰣 4096 */

    userno = 1;

    while ((size = read(fd, buf, sizeof(buf))) > 0)
    {
        sp = (SCHEMA *) buf;
        do
        {
            if (sp->userid[0] == '\0')
            {
                lseek(fd, -size, SEEK_CUR);
                return userno;
            }
            userno++;
            size -= sizeof(SCHEMA);
            sp++;
        } while (size);
    }

    return userno;
}


static void
acct_apply(void)
{
    SCHEMA slot;
    char buf[80];
    char *userid, *pw;
    int try_, fd;

    film_out(FILM_APPLY, 0);

    memset(&cuser, 0, sizeof(cuser));
    userid = cuser.userid;
    try_ = 0;
    for (;;)
    {
        if (!vget(17, 0, msg_uid, userid, IDLEN + 1, DOECHO))
            login_abort("\n�A�� ...");

        if (is_badid(userid))
        {
            vmsg("�L�k�����o�ӥN���A�Шϥέ^��r���A�åB���n�]�t�Ů�");
        }
        else
        {
            usr_fpath(buf, userid, NULL);
            if (dashd(buf))
                vmsg("���N���w�g���H�ϥ�");
            else
                break;
        }

        if (++try_ >= 10)
            login_abort("\n�z���տ��~����J�Ӧh�A�ФU���A�ӧa");
    }

    /* IID.20190530: For the forward compatibility of older versions */
    if (vget(18, 0, "�O�_�ϥηs���K�X�[�K(y/N)�H[N]", buf, 3, LCECHO) == 'y')
    {
        try_ = GENPASSWD_SHA256;
        fd = PLAINPASSLEN;
    }
    else
    {
        try_ = GENPASSWD_DES;
        fd = OLDPLAINPASSLEN;
    }

    for (;;)
    {
        vget(19, 0, "�г]�w�K�X�G", buf, fd, NOECHO | VGET_STEALTH_NOECHO);
        if ((strlen(buf) < 3) || !strcmp(buf, userid))
        {
            vmsg("�K�X��²����D�J�I�A�ܤ֭n 4 �Ӧr�ӥB���i�M�N���ۦ�");
            continue;
        }

        vget(20, 0, "���ˬd�K�X�G", buf + PLAINPASSLEN + 1, fd, NOECHO | VGET_STEALTH_NOECHO);
        if (!strcmp(buf, buf + PLAINPASSLEN + 1))
            break;

        vmsg("�K�X��J���~�A�Э��s��J�K�X");
    }

    str_scpy(cuser.passwd, pw = genpasswd(buf, try_), PASSLEN);
    str_scpy(cuser.passhash, pw + PASSLEN, sizeof(cuser.passhash));

    do
    {
        vget(20, 0, "��    �١G", cuser.username, sizeof(cuser.username), DOECHO);
    } while (strlen(cuser.username) < 2);

    do
    {
        vget(21, 0, "�u��m�W�G", cuser.realname, sizeof(cuser.realname), DOECHO);
    } while (strlen(cuser.realname) < 4);

    do
    {
        vget(22, 0, "�~��a�}�G", cuser.address, sizeof(cuser.address), DOECHO);
    } while (strlen(cuser.address) < 12);

    cuser.point1 = 0;
    cuser.point2 = 0;
    cuser.money = 0;

    cuser.userlevel = PERM_DEFAULT;
    cuser.ufo2 = UFO2_COLOR | UFO2_MOVIE | UFO2_BNOTE | UFO2_MENU_LIGHTBAR;
    /* Thor.980805: ����, �w�]�X��ufo */
    cuser.numlogins = 1;

    /* dispatch unique userno */

    cuser.firstlogin = cuser.lastlogin = cuser.tcheck = slot.uptime = time(0)/*ap_start*/;
    memcpy(slot.userid, userid, IDLEN);

    fd = open(FN_SCHEMA, O_RDWR | O_CREAT, 0600);
    {

        /* flock(fd, LOCK_EX); */
        /* Thor.981205: �� fcntl ���Nflock, POSIX�зǥΪk */
        f_exlock(fd);

        cuser.userno = try_ = uniq_userno(fd);
        write(fd, &slot, sizeof(slot));
        /* flock(fd, LOCK_UN); */
        /* Thor.981205: �� fcntl ���Nflock, POSIX�зǥΪk */
        f_unlock(fd);
    }
    close(fd);

    /* create directory */

    usr_fpath(buf, userid, NULL);
    mkdir(buf, 0755);
    strcat(buf, "/@");
    mkdir(buf, 0755);

    usr_fpath(buf, userid, FN_ACCT);
    fd = open(buf, O_WRONLY | O_CREAT, 0600);
    write(fd, &cuser, sizeof(cuser));
    close(fd);
    /* Thor.990416: �`�N: ���|�� .ACCT���׬O0��, �ӥB�u�� @�ؿ�, �����[� */

    sprintf(buf, "%d", try_);
    blog("APPLY", buf);
}

#endif /* LOGINASNEW */

/* ----------------------------------------------------- */
/* bad login                                             */
/* ----------------------------------------------------- */
static void
logattempt(
    int type                    /* '-' login failure   ' ' success */
)
{
    char buf[128], fpath[80];
    const char *const conn_type = (unix_path) ? "WSP" : "BBS";

//  time_t now = time(0);
    struct tm *p;
    p = localtime(&ap_start);
    sprintf(buf, "%02d/%02d/%02d %02d:%02d:%02d %c%s\t%s\n",
        p->tm_year % 100, p->tm_mon + 1, p->tm_mday,
        p->tm_hour, p->tm_min, p->tm_sec, type, conn_type, currtitle);

#if 0
    sprintf(buf, "%c%-12s[%s] %s\n", type, cuser.userid,
        Etime(&ap_start), currtitle);
    f_cat(FN_LOGIN_LOG, buf);
#endif

//  str_stamp(fpath, &ap_start);
//  sprintf(buf, "%s %cBBS\t%s\n", fpath, type, currtitle);
    /* Thor.990415: currtitle�w���tip */
    /* sprintf(buf, "%s %cBBS\t%s ip:%08x\n", fpath, type, currtitle, tn_addr); */
    /* Thor.980803: �l�� ip address */
    usr_fpath(fpath, cuser.userid, FN_LOG);
    f_cat(fpath, buf);

    if (type != ' ')
    {
        usr_fpath(fpath, cuser.userid, FN_LOGINS_BAD);
        sprintf(buf, "[%s] %s %s\n", Ctime(&ap_start), conn_type,
            (type == '*' ? currtitle : fromhost));
        f_cat(fpath, buf);
    }
}


/* ----------------------------------------------------- */
/* �n�� BBS �{��                                         */
/* ----------------------------------------------------- */

static void
utmp_setup(
    int mode
)
{
    UTMP utmp;
    const char *const guestname[GUESTNAME]={GUEST_NAMES};

    cutmp = NULL; /* Thor.980805: pal_cache���| check cutmp  */
    /*pal_cache();*/  /* by visor */
    /* cutmp = NULL; */
    memset(&utmp, 0, sizeof(utmp));
    utmp.pid = currpid;
    utmp.userno = cuser.userno;
    utmp.mode = bbsmode = mode;
    utmp.in_addr = tn_addr;
    utmp.ufo = cuser.ufo;
    utmp.flag = 0;
    utmp.userlevel = cuser.userlevel;
#ifdef  HAVE_SHOWNUMMSG
    utmp.num_msg = 0;
#endif
#ifdef HAVE_BOARD_PAL
    utmp.board_pal = -1;
#endif

    strcpy(utmp.userid, cuser.userid);
    time(&utmp.idle_time);
    srand(time(0));
    srandom(time(0));
    strcpy(utmp.username, ((!str_casecmp(cuser.userid, STR_GUEST)||!HAS_PERM(PERM_VALID)||HAS_PERM(PERM_DENYNICK))&&!HAS_PERM(PERM_SYSOP)) ? guestname[rand()%GUESTNAME] : cuser.username);
    strcpy(utmp.realname, cuser.realname);
    str_scpy(utmp.from, fromhost, sizeof(utmp.from));

    /* cache.20130407 �O�s�ϥΪ̵n�JIP��m(IPv4) */
    /* IID.20191228: Or IPv6 */
    getnameinfo((struct sockaddr *)&tn_addr, sizeof(tn_addr), ipv6addr, sizeof(ipv6addr), NULL, NI_MAXSERV, NI_NUMERICHOST);

    /* Thor: �i�DUser�w�g���F�񤣤U... */

    if (!utmp_new(&utmp))
    {
        login_abort("\n�z���諸��l�w�g�Q�H�������n�F�A�ФU���A�ӧa");
    }
    pal_cache();
#ifdef  HAVE_BANMSG
    banmsg_cache();
#endif
}


/* ----------------------------------------------------- */
/* user login                                            */
/* ----------------------------------------------------- */

static void
tn_login(void)
{
    int fd, attempts;
    unsigned int level, ufo;
    time_t start, check_deny;
    char fpath[80], uid[IDLEN + 1];

    char passbuf[PLAINPASSLEN];

    /* �� currtitle �@�� */

    /* sprintf(currtitle, "%s@%s", rusername, fromhost); */
    /* Thor.990415: ����ip, �ȥ��d���� */
    getnameinfo((struct sockaddr *)&tn_addr, sizeof(tn_addr), fpath, sizeof(fpath), NULL, NI_MAXSERV, NI_NUMERICHOST);
    sprintf(currtitle, "%s@%s ip:%s (%d)", rusername, fromhost, fpath, currpid);


/* by visor */
#if 0
    move(20, 0);
    outs("�������Ш���~~~~");
    vkey();
    sleep(10);
    exit(0);
#endif
/* by visor */

    /* 081119.cache: ���`��ܶi���e�� */
    move(b_lines, 0);
    prints("\x1b[m���[�αb���G\x1b[1;32mguest\x1b[m  �ӽзs�b���G\x1b[1;31mnew\x1b[m");

    /*move(b_lines, 0);
    outs("�� �L�k�s�u�ɡA�ЧQ�� port 3456 �W��");*/

#ifdef loginAD

    /*090823.cache: �i���s�i*/
    FILE *fp;
    char buf[128];
    move(18, 0);
    if ( ( fp = fopen("gem/@/@AD", "r") ) )
    {
        while (fgets(buf, sizeof(buf), fp))
        outs(buf);
        fclose(fp);
    }

#endif

    attempts = 0;
    for (;;)
    {
        if (++attempts > LOGINATTEMPTS)
        {
            film_out(FILM_TRYOUT, 0);
            login_abort("\n�A�� ....");
        }

        vget(21, 0, msg_uid, uid, IDLEN + 1, DOECHO);

        if (str_casecmp(uid, STR_NEW) == 0)
        {

#ifdef LOGINASNEW
            acct_apply(); /* Thor.980917: ����: setup cuser ok */
            level = cuser.userlevel;
            ufo = cuser.ufo;
            /* cuser.userlevel = PERM_DEFAULT; */
            /* cuser.ufo = UFO_COLOR | UFO_MOVIE | UFO_BNOTE; */
            break;
#else
            outs("\n���t�Υثe�Ȱ��u�W���U�A�Х� guest �i�J");
            continue;
#endif
        }
        else if (!*uid || (acct_load(&cuser, uid) < 0))
        {
            vmsg(err_uid);
        }
        else if (str_casecmp(uid, STR_GUEST))
        {
            if (!vget(21, D_COLS_REF + 26, MSG_PASSWD, passbuf, PLAINPASSLEN, NOECHO | VGET_STEALTH_NOECHO))
            {
                continue;       /* �o�{ userid ��J���~�A�b��J passwd �ɪ������L */
            }

            passbuf[PLAINPASSLEN-1] = '\0';

            if (chkpasswd(cuser.passwd, cuser.passhash, passbuf))
            {
                logattempt('-');
                vmsg(ERR_PASSWD);
            }
            else
            {
                /* SYSOP gets all permission bits */

                if (!str_casecmp(cuser.userid, str_sysop))
                    cuser.userlevel = ~0 ^ (PERM_DENYPOST | PERM_DENYTALK |
                        PERM_DENYCHAT | PERM_DENYMAIL | PERM_DENYSTOP | PERM_DENYNICK |
                        PERM_DENYLOGIN | PERM_PURGE);
                /* else */  /* Thor.980521: everyone should have level */

                level = cuser.userlevel;

                ufo = cuser.ufo & ~(HAS_PERM(PERM_LOGINCLOAK) ?
                /* lkchu.981201: �C���W�����n 'c' ��·� :p */
                    (UFO_BIFF | UFO_BIFFN | UFO_SOCKET | UFO_NET) :
                    (UFO_BIFF | UFO_BIFFN | UFO_SOCKET | UFO_NET | UFO_CLOAK));


                if ((level & PERM_ADMIN) && (cuser.ufo2 & UFO2_ACL))
                {
                    char buf[256]; /* check ip */
                    getnameinfo((struct sockaddr *)&tn_addr, sizeof(tn_addr), buf, sizeof(buf), NULL, NI_MAXSERV, NI_NUMERICHOST);
                    usr_fpath(fpath, cuser.userid, "acl");
                    str_lower(rusername, rusername);      /* lkchu.981201: ���p�g */
                    str_lower(fromhost, fromhost);
                    if (acl_has(fpath, rusername, fromhost) == 0
                            || acl_has(fpath, rusername, buf) == 0)
                    {  /* Thor.980728: �`�N acl��, �M rusername, fromhost �n�����p�g */
                        logattempt('*');
                        login_abort("\n�A���W���a�I���ӹ�l�A�Юֹ� [�W���a�I�]�w��]");
                    }
                }

                logattempt(' ');

                /* check for multi-session */

                if (!(level & PERM_SYSOP))
                {
                    UTMP *ui;
                    pid_t pid;

                    if (level & (PERM_DENYLOGIN | PERM_PURGE))
                        login_abort("\n�o�ӱb���Ȱ��A�ȡA�Ա��ЦV�������ߡC");


                    if (!(ui = (UTMP *) utmp_find(cuser.userno)))
                        break;          /* user isn't logged in */


                    pid = ui->pid;
                    if (!pid || (kill(pid, 0) == -1))
                    {
                        memset(ui, 0, sizeof(UTMP));
                        break;          /* stale entry in utmp file */
                    }

                    if (vans("������h���n�J�A�z�Q�R����L���ƪ� login (Y/n)�ܡH[Y] ") != 'n')
                    {
                        kill(pid, SIGTERM);
                        blog_pid("MULTI", cuser.username, pid);
                        break;
                    }

                    if (utmp_count(cuser.userno, 0) > 2)
                    {
                        pmsg2_body("�z�w�g�F��h���n�J�W��");
                        login_abort("\n");
                    }
                }
                break;
            }
        }
        else
        {                               /* guest */
            logattempt(' ');
            cuser.userlevel = level = 0;
            /* Thor.981207: �ȤH�ê�, �j��g�^cuser.userlevel */
            ufo = UFO_PAGER | UFO_QUIET | UFO_MESSAGE | UFO_PAGER1 |UFO_HIDDEN;
            cuser.ufo = ufo;
            cuser.ufo2 = UFO2_COLOR | UFO2_BNOTE | UFO2_MOVIE | UFO2_MENU_LIGHTBAR;
            if (utmp_count(cuser.userno, 0) > MAXGUEST)
            {
                pmsg2_body("�ثe�b�u�W��guest�L�h");
                login_abort("\n");
            }
            break;
        }
    }

    /* --------------------------------------------------- */
    /* �n���t��                                          */
    /* --------------------------------------------------- */

    start = ap_start;


//  setproctitle("%s@%s", cuser.userid, fromhost);

    sprintf(fpath, "%s (%d)", currtitle, currpid);


    blog("ENTER", fpath);
    // IID.20190521: Currently unused; disabled for now.
    // if (rusername[0])
    //     strcpy(cuser.ident, currtitle);

    /* --------------------------------------------------- */
    /* ��l�� utmp�Bflag�Bmode                           */
    /* --------------------------------------------------- */

    cuser.ufo = ufo & (~UFO_WEB);
    ufo = cuser.ufo;
    utmp_setup(M_LOGIN); /* Thor.980917: ����: cutmp, cutmp-> setup ok */
    bbstate = STAT_STARTED;
    bbsothermode = 0;
    mbox_main();

    film_out(FILM_WELCOME, 0);


#ifdef MODE_STAT
    memset(&modelog, 0, sizeof(UMODELOG));
    mode_lastchange = ap_start;
#endif

    if (level)                  /* not guest */
    {
        /*
         * Thor: �������ܹL userlevel, �ҥH substitute_record �����n�i�g�J
         * userlevel. �u���b userlogin ��, ���i�H�� userlevel, ��L�ɭԧ��i�H.
         */
        /* Thor.990318: �{�b�u���b"�ϥΪ̸�ƽ]�֤�"�ɤ����userlevel, ��L���i */

        /* ------------------------------------------------- */
        /* �ֹ� user level                                       */
        /* ------------------------------------------------- */

#ifdef JUSTIFY_PERIODICAL
        /* if (level & PERM_VALID) */
        if ((level & PERM_VALID) && !(level & (PERM_SYSOP|PERM_XEMPT)))
        {  /* Thor.980819: �������@�w�������{��, ex. SysOp  */
            if (cuser.tvalid + VALID_PERIOD < start)
            {
                level ^= PERM_VALID;
                find_same_email(cuser.email, 3);
            }
        }
#endif


        time(&check_deny);

        level |= (PERM_POST | PERM_PAGE | PERM_CHAT);


        if (!(level & PERM_SYSOP))
        {
#ifdef NEWUSER_LIMIT
            /* Thor.980825: lkchu patch: �J�M�� NEWUSER_LIMIT, �٬O�[�@�U�n�F,
                                         �_�h�@���^�� FAQ ���֪�. :) */
            if (start - cuser.firstlogin < 3 * 86400)
            {
                /* if (!(level & PERM_VALID)) */
                level &= ~PERM_POST;    /* �Y�Ϥw�g�q�L�{�ҡA�٬O�n���ߤT�� */
            }
#endif

            /* Thor.980629: ���g�����{��, �T�� chat/talk/write */
            if (!(level & PERM_VALID))
            {
                level &= ~(PERM_CHAT | PERM_PAGE | PERM_POST);
            }

            if (level & (PERM_DENYPOST | PERM_DENYTALK | PERM_DENYCHAT | PERM_DENYMAIL | PERM_DENYNICK))
            {
                if (cuser.deny < check_deny && !(level & PERM_DENYSTOP))
                {
                    usr_fpath(fpath, cuser.userid, FN_STOPPERM_LOG);
                    unlink(fpath);
                    level &= ~(PERM_DENYPOST | PERM_DENYTALK | PERM_DENYCHAT | PERM_DENYMAIL | PERM_DENYNICK);
                }
            }
            else if (!(level & PERM_DENYSTOP) && cuser.deny > check_deny) cuser.deny = check_deny;

            if (!(level & PERM_CLOAK))
                ufo &= ~(UFO_CLOAK|UFO_HIDDEN);

            if (level & PERM_DENYPOST)
                level &= ~PERM_POST;

            if (level & PERM_DENYTALK)
            {
                ufo |= UFO_MESSAGE|UFO_PAGER1;
                level &= ~PERM_PAGE;
            }

            if (level & PERM_DENYCHAT)
                level &= ~PERM_CHAT;

/*
            if ((cuser.numemail >> 4) > (cuser.numlogins + cuser.numposts))
                level |= PERM_DENYMAIL;*/
        }

        supervisor = check_admin(cuser.userid);

        /* ------------------------------------------------- */
        /* �n�ͦW��P�B�B�M�z�L���H��                    */
        /* ------------------------------------------------- */

        if (start > cuser.tcheck + CHECK_PERIOD)
        {
#ifdef  HAVE_MAILGEM
            if (HAS_PERM(PERM_MBOX))
            {
                int (*p)(int level, char *fpath);
                char fpath[128];
                usr_fpath(fpath, cuser.userid, "gem");

                p = DL_NAME_GET("mailgem.so", gcheck);
                if (p)
                    (*p)(0, fpath);
            }
#endif

            pal_sync(NULL); /* Thor.990318: �b�o�^�{�ҫH�����v���j:P */
            aloha_sync();
#ifdef  HAVE_BANMSG
            banmsg_sync(NULL);  /* �ڦ��T�� */
#endif
            ufo |= m_quota(); /* Thor.980804: ����, ��ƾ�z�]�֦��]�t BIFF check */
            cuser.ufo = ufo;
            cuser.tcheck = start;
        }
        else
        {
            if (m_query(cuser.userid)>0)
                ufo |= UFO_BIFF;
        }
        if (check_personal_note(1, cuser.userid))
            ufo |= UFO_BIFFN;

        cutmp->ufo = cuser.ufo = ufo; /* Thor.980805: �ѨM ufo �P�B���D */

        /* ------------------------------------------------- */
        /* �N .ACCT �g�^                                         */
        /* ------------------------------------------------- */

#if 1
        /* Thor.990318: ������j���v ���H�bwelcome�e���^�{�ҫH, �G���ܦ� */
        move(b_lines - 3, 0);
        prints("�� �w��z�� \x1b[1;33m%d\x1b[m �׫��X�����A�ڰO�o���ѬO \x1b[1;33m%s\x1b[m\n",
            cuser.numlogins, Ctime(&cuser.lastlogin));
        prints("�� �Ӧ۩� \x1b[1;33m%s\x1b[m", cuser.lasthost);
        /* Thor.990321: �Nvmsg���ܫ��, ����H�b���ɦ^�{�ҫH */
#endif

        cuser.lastlogin = start;
        cuser.userlevel = level;
        str_scpy(cuser.lasthost, fromhost, sizeof(cuser.lasthost));
        usr_fpath(fpath, cuser.userid, FN_ACCT);
        fd = open(fpath, O_WRONLY);
        write(fd, &cuser, sizeof(ACCT));
        close(fd);
#if 1
        vmsg(NULL);
#endif

#if 1
        usr_fpath(fpath, cuser.userid, FN_LOGINS_BAD);
        /* Thor.990204: ���Ҽ{more �Ǧ^�� */
        if (more(fpath, (char *)-1) >= 0 && vans("������n�J���Ѫ��O���A�z�n�R���W�z��T��(Y/n)?[Y]") != 'n')
            unlink(fpath);
#endif

        usr_fpath(fpath, cuser.userid, FN_STOPPERM_LOG);
        if (more(fpath, (char *)-1)>= 0)
            vmsg(NULL);

#if 1
        if (!(level & PERM_VALID))
        {
            bell();
            more(FN_ETC_NOTIFY, NULL);
        }
        /* ���Įɶ��O�� 10 �ѫe���Xĵ�i */
#ifdef JUSTIFY_PERIODICAL
        else if (!(level & (PERM_ADMIN|PERM_XEMPT)))
        {
            if (cuser.tvalid + VALID_PERIOD - 10 * 86400 < start)
            {
                bell();
                more(FN_ETC_REREG, NULL);
            }
        }
#endif
#endif  /* #if 1 */

#ifdef NEWUSER_LIMIT
            /* Thor.980825: lkchu patch: �J�M�� NEWUSER_LIMIT, �٬O�[�@�U�n�F,
                                         �_�h�@���^�� FAQ ���֪�. :) */
            if (start - cuser.firstlogin < 3 * 86400)
            {
                /* if (!(level & PERM_VALID)) */
                /* �Y�Ϥw�g�q�L�{�ҡA�٬O�n���ߤT�� */
                more(FN_ETC_NEWUSER, NULL);
            }
#endif

#if 0
        if (ufo & UFO_MQUOTA)
        {
            bell();
            more(FN_ETC_MQUOTA, NULL);
        }
#endif

        ve_recover();

#ifdef LOGIN_NOTIFY
        if (!(ufo & UFO_CLOAK))     /* lkchu.981201: �ۤv�����N���� notify */
        {
            void loginNotify(void);
            loginNotify();
        }
        else
        {
            /* lkchu.981201: �M�� benz, ���q���F */
            usr_fpath(fpath, cuser.userid, FN_BENZ);
            unlink(fpath);
        }
#endif

#ifdef HAVE_ALOHA
        if (!(ufo & UFO_CLOAK))     /* lkchu.981201: �ۤv�����N���� notify */
        {
            void aloha(void);
            aloha();
        }

#endif
    }
    else
    {
        vmsg(NULL);                     /* Thor: for guest look welcome */
    }

    /* lkchu.990510: ���n���i��� announce, ���� welcome �� :p */
    more(FN_ETC_ANNOUNCE, NULL);
#if 0
    bell();
    sleep(1);
    bell();
#endif

    /* Thor.980917: ����: �즹�������ӳ]�w�n cuser. level? ufo? rusername bbstate cutmp cutmp-> */

    if (!(cuser.ufo2 & UFO2_MOTD))
    {
        more("gem/@/@-day", NULL);      /* ����������D */
        clear();
        pad_view();
    }

#if 0
    if (cuser.ufo2 & UFO2_APRIL1)
    {
        more(FN_APRIL_FIRST, NULL);
        bell();
        sleep(1);
        bell();
    }
#endif

/* 20100615.cache: 3000�H�H�W�|�y���į�W�����D */
//#ifdef        HAVE_CLASSTABLEALERT
//  classtable_free();
//  classtable_main();
//#endif

    showansi = cuser.ufo2 & UFO2_COLOR;
#ifdef M3_USE_PFTERM
    redrawwin();
    refresh();
#endif
}


/* ----------------------------------------------------- */
/* trap signals                                          */
/* ----------------------------------------------------- */

static void abort_bbs_signal(GCC_UNUSED int signum)
{
    abort_bbs();
}
void talk_rqst_signal(GCC_UNUSED int signum)
{
    talk_rqst();
}
static void bmw_rqst_signal(GCC_UNUSED int signum)
{
    bmw_rqst();
}

static void
tn_signals(void)
{
    struct sigaction act;

    /* act.sa_mask = 0; */ /* Thor.981105: �зǥΪk */
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;

    act.sa_handler = abort_bbs_signal;
    sigaction(SIGBUS, &act, NULL);
    sigaction(SIGSEGV, &act, NULL);
    sigaction(SIGTERM, &act, NULL);
    sigaction(SIGXCPU, &act, NULL);
#ifdef SIGSYS
    /* Thor.981221: easy for porting */
    sigaction(SIGSYS, &act, NULL);/* bad argument to system call */
#endif

    act.sa_handler = talk_rqst_signal;
    sigaction(SIGUSR1, &act, NULL);

    act.sa_handler = bmw_rqst_signal;
    sigaction(SIGUSR2, &act, NULL);

    /* sigblock(sigmask(SIGPIPE)); */
    /* Thor.981206: lkchu patch: �Τ@ POSIX �зǥΪk  */
    /* �b���ɥ� sigset_t act.sa_mask */
    sigaddset(&act.sa_mask, SIGPIPE);
    sigprocmask(SIG_BLOCK, &act.sa_mask, NULL);

}


static inline void
tn_main(void)
{
    long nproc;
    double load[3], load_norm;
    char buf[128], buf2[128];

    getnameinfo((struct sockaddr *)&tn_addr, sizeof(tn_addr), buf2, sizeof(buf2), NULL, NI_MAXSERV, NI_NUMERICHOST);
    str_lower(buf, fromhost);

    if (acl_has(FN_ETC_BANIP_ACL, "", buf) > 0
        || acl_has(FN_ETC_BANIP_ACL, "", buf2) > 0)
        exit(0);

    /* �N login �������M�� */
    add_io(0, 0);
    while (igetch() != I_TIMEOUT);
    add_io(0, 60);


    clear();
/*cache.080510: --���ץ�, �}���ɭn�O�o�]�w�o�q�ܦ�����--*/
/*cache.080714: power user �i�H��Jbusy�}��login�e������*/
#if 0
    more("gem/@/@close", (char *) -1);    /* �T��W���e�� */
    move(b_lines -1, 0);
    outs("�Ы����N�����}...");
    if ( vkey() != 'c' || vkey() != 'c' || vkey() != 'n' || vkey() != 's' )
        login_abort("�w���_�s�u");
#endif
#if 0
    sprintf(buf, BBSHOME "/gem/@/@close");
    more(buf, (char *) -1);
    login_abort("\n�ڤ��j�a�D���h�E��...");
    return;
    sleep(10);
    login_abort("\n");
#endif


    //�t������e�����o
    nproc = sysconf(_SC_NPROCESSORS_ONLN);
    getloadavg(load, 3);
    load_norm = load[0] / ((nproc > 0) ? nproc : 2);

    //�t���L���T��login
    if (load_norm>5)
    {
        prints("\n�藍�_...\n\n�ѩ�ثe�t���L���A�еy��A��...");
        //pcman�|�۰ʭ��s�ɶ��]�ӵu�|�ܦ� DOS �ܥi�� :P
        sleep(3);
        login_abort("\n");
    }

    //�קK�ݪO�H��t��
    currbno = -1;

    //getloadavg(load, 3);
    prints( MYHOSTNAME " �� " OWNER " �� " BBSIP " [" BBSVERNAME " " BBSVERSION "]\n"
"�w����{�i\x1b[1;33;46m %s \x1b[m�j�C�t�έt���G%.2f %.2f %.2f / %ld [%s] �u�W�H�� [%d/%d]",
        str_site, load[0], load[1], load[2], nproc, load_norm>5?"\x1b[1;37;41m�L��\x1b[m":load_norm>1?"\x1b[1;37;42m����\x1b[m":"\x1b[1;37;44m���`\x1b[m", ushm->count, MAXACTIVE);

    film_out(FILM_INCOME, 2);

    total_num = ushm->count+1;
    currpid = getpid();
    cutmp = NULL;


    tn_signals();  /* Thor.980806: ��� tn_login�e, �H�K call in���|�Q�� */
    tn_login();

    /* cache.090823 ����H�ƿ��~�z�� */
    if (ushm->count < 0)
        ushm->count = 0;

    /*tn_signals(); */
    brh_load();

#ifdef  HAVE_DETECT_CROSSPOST
    if (attr_get(cuser.userid, ATTR_CROSS_KEY, &cksum)<0)
        memset(&cksum, 0, sizeof(CHECKSUMCOUNT));
#endif

    board_main();

    gem_main();

    talk_main();

#ifdef  HAVE_FORCE_BOARD
    force_board();  /* CdChen.990417: �j��\Ū���i�O */
#endif

    count_update();
    time(&ap_start);
#ifdef  HAVE_CHK_MAILSIZE
    if (!HAS_PERM(PERM_DENYMAIL) && HAS_PERM(PERM_BASIC))
    {
        if (mail_stat(CHK_MAIL_VALID))
        {
            chk_mailstat = 1;
            vmsg("�z���H�c�w�W�X�e�q�A�L�k�ϥΥ��\\��A�вM�z�z���H�c�I");
            xover(XZ_MBOX);
        }
    }
#endif

    main_menu();

    /* IID.20200102: Log out. */

#ifdef  LOG_BMW
    /*bmw_save();*/                   /* lkchu.981201: ���T�O���B�z */
#endif

    clear();
    prints("       \x1b[1;31m ��       \x1b[1;36m �z�w�{�z�w�{�z�w�{�z�w�� �z�w���z���{�z�w�{\n"
        "      \x1b[1;31m��\x1b[1;37m��\x1b[1;33m��\x1b[1;37m������\x1b[1;36m�x  �s�x  �x�x  �x�x  �x �x �� �|  �}�x����\x1b[1;37m��������\n"
        "       \x1b[1;33m ��        \x1b[1;34m�|�w�t�|�w�}�|�w�}�|�w�� �|�w�� �|�} �|�w�}\x1b[m\n");
    prints("Dear \x1b[32m%s(%s)\x1b[m�A�O�ѤF�A�ץ��{�i %s �j\n"
        "�H�U�O�z�b���������U���:\n",
        cuser.userid, cuser.username, str_site);
    acct_show(&cuser, 3);
    vmsg_body(NULL);
    u_exit("EXIT ");
    vkey();
    exit(0);
}


/* ----------------------------------------------------- */
/* FSA (finite state automata) for telnet protocol       */
/* ----------------------------------------------------- */

static void
telnet_init(void)
{
    static const unsigned char svr[] = {
        IAC, DO, TELOPT_TTYPE,
        IAC, SB, TELOPT_TTYPE, TELQUAL_SEND, IAC, SE,
        IAC, WILL, TELOPT_ECHO,
        IAC, WILL, TELOPT_SGA,
        IAC, DO, TELOPT_BINARY
    };

    int n, len;
    const unsigned char *cmd;
    int rset;
    struct timeval to;
    char buf[64];

    /* --------------------------------------------------- */
    /* init telnet protocol                                */
    /* --------------------------------------------------- */

#if 0
    to.tv_sec = 1;
    to.tv_usec = 1;
#endif
    cmd = svr;

    for (n = 0; n < 4; n++)
    {
        len = (n == 1 ? 6 : 3);
        send(0, cmd, len, 0);
        cmd += len;

        rset = 1;
        /* Thor.981221: for future reservation bug */
        to.tv_sec = 1;
        to.tv_usec = 1;
        if (select(1, (fd_set *) & rset, NULL, NULL, &to) > 0)
            recv(0, buf, sizeof(buf), 0);
    }
}


/* ----------------------------------------------------- */
/* �䴩�W�L 24 �C���e��                                  */
/* ----------------------------------------------------- */

static void
term_init(void)
{
/* fuse.030518: ���� */
//  server�ݡG�A�|���ܦ�C�ƶܡH(TN_NAWS, Negotiate About Window Size)
//  client���GYes, I do. (TNCH_DO)
//
//  ����b�s�u�ɡA��TERM�ܤƦ�C�ƮɴN�|�o�X�G
//  TNCH_IAC + TNCH_SB + TN_NAWS + ��ƦC�� + TNCH_IAC + TNCH_SE;

    /* ask client to report it's term size */
    static const unsigned char svr[] =      /* server */
    {
        IAC, DO, TELOPT_NAWS
    };

    int rset;
    char buf[64], *rcv;
    struct timeval to;

#ifdef M3_USE_PFTERM
    initscr();
#endif

    memset(buf, 0, sizeof(buf));

    /* �ݹ�� (telnet client) ���S���䴩���P���ù��e�� */
    send(0, svr, 3, 0);

    rset = 1;
    to.tv_sec = 1;
    to.tv_usec = 1;
    if (select(1, (fd_set *) & rset, NULL, NULL, &to) > 0)
        recv(0, buf, sizeof(buf), 0);

    rcv = NULL;
    if ((unsigned char) buf[0] == IAC && buf[2] == TELOPT_NAWS)
    {
        /* gslin: Unix �� telnet �靈�L�[ port �Ѽƪ��欰���Ӥ@�� */
        if ((unsigned char) buf[1] == SB)
        {
            rcv = buf + 3;
        }
        else if ((unsigned char) buf[1] == WILL)
        {
            if ((unsigned char) buf[3] != IAC)
            {
                rset = 1;
                to.tv_sec = 1;
                to.tv_usec = 1;
                if (select(1, (fd_set *) & rset, NULL, NULL, &to) > 0)
                    recv(0, buf + 3, sizeof(buf) - 3, 0);
            }
                if ((unsigned char) buf[3] == IAC && (unsigned char) buf[4] == SB && buf[5] == TELOPT_NAWS)
                    rcv = buf + 6;
        }
    }

    if (rcv)
    {
        b_lines = ntohs(* (short *) (rcv + 2)) - 1;
        b_cols = ntohs(* (short *) rcv) - 1;

        /* b_lines �ܤ֭n 23�A�̦h����W�L T_LINES - 1 */
        b_lines = TCLAMP(b_lines, 23, T_LINES - 1);
        /* b_cols �ܤ֭n 79�A�̦h����W�L T_COLS - 1 */
        b_cols = TCLAMP(b_cols, 79, T_COLS - 1);
    }
    else
    {
        b_lines = 23;
        b_cols = 79;
    }

#ifdef M3_USE_PFTERM
    resizeterm(b_lines + 1, b_cols + 1);
#endif

    d_cols = b_cols - 79;
}

/* ----------------------------------------------------- */
/* stand-alone daemon                                    */
/* ----------------------------------------------------- */

static void
start_daemon(
    int port /* Thor.981206: �� 0 �N�� *�S���Ѽ�*, -1 �N�� -i (inetd) */
             /* IID.20190903: `-2` represents `-u` <unix_socket>`. */ )
{
    struct addrinfo hints = {0};
    struct addrinfo *hosts;
    struct sockaddr_un sun = {0};
#ifdef RLIMIT
    struct rlimit rl;
#endif
    char buf[80], data[80];
    int fd;
    bool listen_success = false;

    {
        /*
         * More idiot speed-hacking --- the first time conversion makes the C
         * library open the files containing the locale definition and time zone.
         * If this hasn't happened in the parent process, it happens in the
         * children, once per connection --- and it does add up.
         */

        time_t val;
        time(&val);
        strftime(buf, 80, "%d/%b/%Y %H:%M:%S", localtime(&val));
    }
#ifndef NOIDENT
    dns_init();
#endif

#ifdef RLIMIT
    /* --------------------------------------------------- */
    /* adjust resource : 16 mega is enough                 */
    /* --------------------------------------------------- */

    rl.rlim_cur = rl.rlim_max = 16 * 1024 * 1024;
    /* setrlimit(RLIMIT_FSIZE, &rl); */
    setrlimit(RLIMIT_DATA, &rl);

#ifdef SOLARIS
#define RLIMIT_RSS RLIMIT_AS
    /* Thor.981206: port for solaris 2.6 */
#endif

    setrlimit(RLIMIT_RSS, &rl);

    rl.rlim_cur = rl.rlim_max = RLIM_INFINITY;
    setrlimit(RLIMIT_CORE, &rl);

    rl.rlim_cur = rl.rlim_max = 60 * 20;
    setrlimit(RLIMIT_CPU, &rl);

#endif //RLIMIT

    /* --------------------------------------------------- */
    /* change directory to bbshome                         */
    /* --------------------------------------------------- */

    chdir(BBSHOME);
    umask(077);

    /* --------------------------------------------------- */
    /* detach daemon process                               */
    /* --------------------------------------------------- */

    close(1);
    close(2);

    if (port == -1) /* Thor.981206: inetd -i */
    {
        int n;
        struct sockaddr_storage sin;
        /* Give up root privileges: no way back from here        */
        setgid(BBSGID);
        setuid(BBSUID);
#if 1
        n = sizeof(sin);
        if (getsockname(0, (struct sockaddr *) &sin, (socklen_t *) &n) >= 0)
        {
            char port_str[NI_MAXSERV];
            getnameinfo((struct sockaddr *) &sin, n, NULL, NI_MAXHOST, port_str, sizeof(port_str), NI_NUMERICSERV);
            /* mport = */ port = atoi(port_str);
        }
#endif
        /* mport = port; */ /* Thor.990325: ���ݭn�F:P */

        sprintf(data, "%d\t%s\t%d\tinetd -i\n", getpid(), buf, port);
        f_cat(PID_FILE_INET, data);
        return;
    }

    close(0);

    if (fork())
        exit(0);

    setsid();

    if (fork())
        exit(0);

    /* --------------------------------------------------- */
    /* fork daemon process                                 */
    /* --------------------------------------------------- */

    if (port == -2)
    {
        hints.ai_family = sun.sun_family = AF_UNIX;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_addr = (struct sockaddr *)&sun;
        hints.ai_addrlen = sizeof(sun);
        hints.ai_next = NULL;
        hosts = &hints;
    }
    else
    {
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        hints.ai_flags = AI_V4MAPPED | AI_ADDRCONFIG | AI_NUMERICSERV | AI_PASSIVE;
    }

    if (port == 0) /* Thor.981206: port 0 �N��S���Ѽ� */
    {
        int n = MAXPORTS - 1;
        while (n)
        {
            if (fork() == 0)
                break;

            sleep(1);
            n--;
        }
        port = myports[n];
    }

    if (port == -2)
    {
        strlcpy(sun.sun_path, unix_path, sizeof(sun.sun_path));
        // remove the file first if it exists.
        unlink(sun.sun_path);
    }
    else
    {
        char port_str[12];
        sprintf(port_str, "%d", port);
        if (getaddrinfo(NULL, port_str, &hints, &hosts))
            exit(1);
    }

    for (struct addrinfo *host = hosts; host; host = host->ai_next)
    {
        struct linger ld;
        int val;

        fd = socket(host->ai_family, host->ai_socktype, host->ai_protocol);
        if (fd < 0)
            continue;

        val = 1;
        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *) &val, sizeof(val));

#if 0
        setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (char *) &val, sizeof(val));

        if (port != -2)
            setsockopt(fd, host->ai_socktype, TCP_NODELAY, (char *) &val, sizeof(val));
#endif

        ld.l_onoff = ld.l_linger = 0;
        setsockopt(fd, SOL_SOCKET, SO_LINGER, (char *) &ld, sizeof(ld));

        /* mport = port; */ /* Thor.990325: ���ݭn�F:P */

        if ((bind(fd, host->ai_addr, host->ai_addrlen) < 0) || (listen(fd, QLEN) < 0))
            continue;

        if (port == -2)
        {
            chown(unix_path, BBSUID, WWWGID);
            chmod(unix_path, 0660);
        }
        listen_success = true;
    }
    if (port != -2)
        freeaddrinfo(hosts);
    if (!listen_success)
        exit(1);

    /* --------------------------------------------------- */
    /* Give up root privileges: no way back from here      */
    /* --------------------------------------------------- */

    setgid(BBSGID);
    setuid(BBSUID);

    //sprintf(data, "%d\t%s\t%d\n", getpid(), buf, port);
    sprintf(data, "%d\n", getpid());

    if (port == -2)
    {
        f_cat(PID_FILE_UNIXSOCKET, data);
    }
    else
    {
        f_cat(PID_FILE, data);
    }
}


/* ----------------------------------------------------- */
/* reaper - clean up zombie children                     */
/* ----------------------------------------------------- */


static inline void
reaper(GCC_UNUSED int signum)
{
    while (waitpid(-1, NULL, WNOHANG | WUNTRACED) > 0);
}


#ifdef  SERVER_USAGE
static void
servo_usage(GCC_UNUSED int signum)
{
    struct rusage ru;
    FILE *fp;

    fp = fopen("run/bbs.usage", "a");

    if (!getrusage(RUSAGE_CHILDREN, &ru))
    {
        fprintf(fp, "\n[Server Usage] %d: %d\n\n"
            "user time: %.6f\n"
            "system time: %.6f\n"
            "maximum resident set size: %lu P\n"
            "integral resident set size: %lu\n"
            "page faults not requiring physical I/O: %ld\n"
            "page faults requiring physical I/O: %ld\n"
            "swaps: %ld\n"
            "block input operations: %ld\n"
            "block output operations: %ld\n"
            "messages sent: %ld\n"
            "messages received: %ld\n"
            "signals received: %ld\n"
            "voluntary context switches: %ld\n"
            "involuntary context switches: %ld\n\n",

            getpid(), ap_start,
            (double) ru.ru_utime.tv_sec + (double) ru.ru_utime.tv_usec / 1000000.0,
            (double) ru.ru_stime.tv_sec + (double) ru.ru_stime.tv_usec / 1000000.0,
            ru.ru_maxrss,
            ru.ru_idrss,
            ru.ru_minflt,
            ru.ru_majflt,
            ru.ru_nswap,
            ru.ru_inblock,
            ru.ru_oublock,
            ru.ru_msgsnd,
            ru.ru_msgrcv,
            ru.ru_nsignals,
            ru.ru_nvcsw,
            ru.ru_nivcsw);
    }

    fclose(fp);
}
#endif  /* #ifdef  SERVER_USAGE */


static void
main_term(GCC_UNUSED int signum)
{
#ifdef  SERVER_USAGE
    servo_usage();
#endif
    exit(0);
}


static inline void
main_signals(void)
{
    struct sigaction act;

    /* act.sa_mask = 0; */ /* Thor.981105: �зǥΪk */
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;

    act.sa_handler = reaper;
    sigaction(SIGCHLD, &act, NULL);

    act.sa_handler = main_term;
    sigaction(SIGTERM, &act, NULL);

#ifdef  SERVER_USAGE
    act.sa_handler = servo_usage;
    sigaction(SIGPROF, &act, NULL);
#endif

    /* sigblock(sigmask(SIGPIPE)); */
}

static void usage(char *argv0)
{
    fprintf(stderr,
            "Usage: %s\n"
            "Listen on pre-defined ports\n"
            "\n",
            argv0);
    fprintf(stderr,
            "       %s -i\n"
            "Listen on inetd port when run by inetd\n"
            "\n",
            argv0);
    fprintf(stderr,
            "       %s <listen_spec>\n"
            "listen_specs:\n"
            "\t[-p] <port>        listen on port; -p can be omitted (port > 0)\n"
            "\t-u <unix_socket>   listen on unix_socket, with connection data passing enabled\n"
            "\n",
            argv0);
}

int main(int argc, char *argv[])
{
    int csock;                  /* socket for Master and Child */
    int *totaluser;
    int value = 0;
    bool is_proxy = false;      /* Whether connection data passing is enabled */

    /* --------------------------------------------------- */
    /* setup standalone daemon                             */
    /* --------------------------------------------------- */

    /* Thor.990325: usage, bbsd, or bbsd -i, or bbsd 1234 */
    /* Thor.981206: �� 0 �N�� *�S���Ѽ�*, -1 �N�� -i */

    switch (getopt(argc, argv, "p:u:i"))
    {
    case -1:
        if (!(optarg = argv[optind++]))
            break;
        // Falls through
        // to handle omitted `-p`
    case 'p':  /* IID.20190902: `bbsd [-p] 3456`. */
        if ((value = atoi(optarg)) > 0)
        {
            unix_path = NULL;
            break;
        }

        if (0)  // Falls over the case
        {
    case 'u':  /* IID.20190903: `bbsd -u <unix_socket>` */
            unix_path = optarg;
            is_proxy = true;
            value = -2;
            break;
        }
        else if (0)
        {
    case 'i':
            value = -1;
            break;
        }

    default:
        usage(argv[0]);
        return 2;
    }

    start_daemon(value);

    main_signals();

    /* --------------------------------------------------- */
    /* attach shared memory & semaphore                    */
    /* --------------------------------------------------- */
#ifdef  HAVE_SEM
    sem_init();
#endif
    ushm_init();
    bshm_init();
    fshm_init();
    fwshm_init();
    count_init();
#ifdef  HAVE_OBSERVE_LIST
    observeshm_init();
#endif

    /* --------------------------------------------------- */
    /* main loop                                           */
    /* --------------------------------------------------- */

    totaluser = &ushm->count;
    /* avgload = &ushm->avgload; */

    for (;;)
    {
        struct sockaddr_storage sin;
        int value = 1;
        if (select(1, (fd_set *) & value, NULL, NULL, NULL) < 0)
            continue;
        value = sizeof(sin);
        csock = accept(0, (struct sockaddr *) &sin, (socklen_t *) &value);
        if (csock < 0)
        {
            reaper(0);
            continue;
        }

        if (is_proxy)
        {
            conn_data_t cdata;
            if (read(csock, &cdata, sizeof(cdata)) != sizeof(cdata)
                || cdata.cb != sizeof(cdata))
            {
                close(csock);
                continue;
            }

            switch (value = cdata.raddr_len)
            {
            case 4:
                sin.ss_family = AF_INET;
                ((struct sockaddr_in *)&sin)->sin_addr = *(struct in_addr *)&cdata.raddr;
                ((struct sockaddr_in *)&sin)->sin_port = cdata.rport;
                break;
            case 16:
                sin.ss_family = AF_INET6;
                ((struct sockaddr_in6 *)&sin)->sin6_addr = *(struct in6_addr *)&cdata.raddr;
                ((struct sockaddr_in6 *)&sin)->sin6_port = cdata.rport;
                break;
            default:; /* Unsupported address family */
            }
            /* mport = cdata.lport; */
        }

        switch (sin.ss_family)
        {
        case AF_INET:
        case AF_INET6:
            break;

        case AF_UNIX: /* Unsupported address family of connections accepted from Unix socket */
        default: /* Other unsupported address family */
            close(csock);
            continue;
        }

        time(&ap_start);
        argc = *totaluser;
        if (argc >= MAXACTIVE - 5 /* || *avgload > THRESHOLD */)
        {
            sprintf(currtitle,
                "�ثe�u�W�H�� [%d] �H�A�t�κ����A�еy��A��\n", argc);
            send(csock, currtitle, strlen(currtitle), 0);
            close(csock);
            continue;
        }

        if (fork())
        {
            close(csock);
            continue;
        }

        dup2(csock, 0);
        close(csock);

#if 0
        /* Thor.990121: �K�o�Ϭd�����H�[�� */

        telnet_init();

        sprintf(currtitle, "���i�J%s...\n", str_site);
        send(0, currtitle, strlen(currtitle), 0);
#endif

        /* ------------------------------------------------- */
        /* ident remote host / user name via RFC931          */
        /* ------------------------------------------------- */

        /* rfc931((struct sockaddr *)&sin, fromhost, rusername); */

        tn_addr = *(ip_addr *)&sin;

#ifdef NOIDENT
        /* cache.090728: �s�u���Ϭd, �W�[�t�� */
        getnameinfo((struct sockaddr *)&tn_addr, sizeof(tn_addr), fromhost, sizeof(fromhost), NULL, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV);
#else
        /* Thor.990325: �ק�dns_ident�w�q, �Ӧۭ�if�s�� */
        dns_ident(0 /* mport */, &tn_addr, fromhost, sizeof(fromhost), rusername, sizeof(rusername));
#endif

        telnet_init();
        term_init();
        tn_main();
    }
}
