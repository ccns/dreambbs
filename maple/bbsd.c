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


#define MAXPORTS        3
static const int myports[MAXPORTS] = {23, 3456, 3001, /* 3002, 3003 */};

extern BCACHE *bshm;
extern UCACHE *ushm;
#ifdef  HAVE_DETECT_CROSSPOST
extern CHECKSUMCOUNT cksum;
#endif

/* Thor.990113: exports for anonymous log */
/* static */ char rusername[40];

/* static int mport; */ /* Thor.990325: 不需要了:P */
static u_long tn_addr;

/* IID.20190903: The unix socket path for listening proxy connections */
static const char *unix_path;


#ifdef  TREAT
int treat=0;
#endif


#ifdef MODE_STAT
extern UMODELOG modelog;
extern time_t mode_lastchange;
#endif

/* ----------------------------------------------------- */
/* 離開 BBS 程式                                         */
/* ----------------------------------------------------- */

void
blog(
    const char *mode,
    const char *msg
)
{
    char buf[512], data[256];
    time_t now;

    time(&now);
    if (!msg)
    {
        msg = data;
        sprintf(data, "Stay: %d (%d)", (int)(now - ap_start) / 60, currpid);
    }

    sprintf(buf, "%s %-5.5s %-13s%s\n", Etime(&now), mode, cuser.userid, msg);
    f_cat(FN_USIES, buf);
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

    if (currbno >= 0)
    {
        if (bshm->mantime[currbno] > 0)//防止人氣變成負數
            bshm->mantime[currbno]--; /* 退出最後看的那個板 */
        else
            bshm->mantime[currbno] = 0;//負數的話歸零
    }

    utmp_free();                        /* 釋放 UTMP shm */
    blog(mode, NULL);

    if (cuser.userlevel)
    {
        ve_backup();            /* 編輯器自動備份 */
        brh_save();                     /* 儲存閱讀記錄檔 */

#ifdef  HAVE_DETECT_CROSSPOST
        attr_put(cuser.userid, ATTR_CROSS_KEY, &cksum); /* 儲存 CrossPost 紀錄 */
#endif

    }

    /* Thor.980405: 離站時必定刪除熱訊 */
#ifndef LOG_BMW        /* lkchu.981201: 可在 config.h 設定 */

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
            /* lkchu.981201: 用 delta, 每次上站都要超過三分鐘才算 */
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


void
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


/* Thor.980903: lkchu patch: 不使用上站申請帳號時, 則下列 function均不用 */

#ifdef LOGINASNEW

/* ----------------------------------------------------- */
/* 檢查 user 註冊情況                                    */
/* ----------------------------------------------------- */


static int
belong(
    const char *flist,
    const char *key
)
{
    int fd, rc;

    rc = 0;
    fd = open(flist, O_RDONLY);
    if (fd >= 0)
    {
        char *str;
        mgets(-1);

        while ((str = mgets(fd)))
        {
            str_lower(str, str);
            if (str_str(key, str))
            {
                rc = 1;
                break;
            }
        }

        close(fd);
    }
    return rc;
}


static int
is_badid(
    const char *userid
)
{
    int ch;
    const char *str;

    if (strlen(userid) < 2)
        return 1;

    if (!is_alpha(*userid))
        return 1;

    if (!str_cmp(userid, STR_NEW))
        return 1;

    str = userid;
    while ((ch = *(++str)))
    {
        if (!is_alnum(ch))
            return 1;
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
    SCHEMA *sp;                 /* record length 16 可整除 4096 */

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
            login_abort("\n再見 ...");

        if (is_badid(userid))
        {
            vmsg("無法接受這個代號，請使用英文字母，並且不要包含空格");
        }
        else
        {
            usr_fpath(buf, userid, NULL);
            if (dashd(buf))
                vmsg("此代號已經有人使用");
            else
                break;
        }

        if (++try_ >= 10)
            login_abort("\n您嘗試錯誤的輸入太多，請下次再來吧");
    }

    /* IID.20190530: For the forward compatibility of older versions */
    if (vget(18, 0, "是否使用新式密碼加密(y/N)？[N]", buf, 3, LCECHO) == 'y')
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
        vget(19, 0, "請設定密碼：", buf, fd, NOECHO | VGET_STEALTH_NOECHO);
        if ((strlen(buf) < 3) || !strcmp(buf, userid))
        {
            vmsg("密碼太簡單易遭入侵，至少要 4 個字而且不可和代號相似");
            continue;
        }

        vget(20, 0, "請檢查密碼：", buf + PLAINPASSLEN + 1, fd, NOECHO | VGET_STEALTH_NOECHO);
        if (!strcmp(buf, buf + PLAINPASSLEN + 1))
            break;

        vmsg("密碼輸入錯誤，請重新輸入密碼");
    }

    str_ncpy(cuser.passwd, pw = genpasswd(buf, try_), PASSLEN);
    str_ncpy(cuser.passhash, pw + PASSLEN, sizeof(cuser.passhash));

    do
    {
        vget(20, 0, "暱    稱：", cuser.username, sizeof(cuser.username), DOECHO);
    } while (strlen(cuser.username) < 2);

    do
    {
        vget(21, 0, "真實姓名：", cuser.realname, sizeof(cuser.realname), DOECHO);
    } while (strlen(cuser.realname) < 4);

    do
    {
        vget(22, 0, "居住地址：", cuser.address, sizeof(cuser.address), DOECHO);
    } while (strlen(cuser.address) < 12);

    cuser.point1 = 0;
    cuser.point2 = 0;
    cuser.money = 0;

    cuser.userlevel = PERM_DEFAULT;
    cuser.ufo2 = UFO2_COLOR | UFO2_MOVIE | UFO2_BNOTE;
    /* Thor.980805: 註解, 預設旗標ufo */
    cuser.numlogins = 1;

    /* dispatch unique userno */

    cuser.firstlogin = cuser.lastlogin = cuser.tcheck = slot.uptime = time(0)/*ap_start*/;
    memcpy(slot.userid, userid, IDLEN);

    fd = open(FN_SCHEMA, O_RDWR | O_CREAT, 0600);
    {

        /* flock(fd, LOCK_EX); */
        /* Thor.981205: 用 fcntl 取代flock, POSIX標準用法 */
        f_exlock(fd);

        cuser.userno = try_ = uniq_userno(fd);
        write(fd, &slot, sizeof(slot));
        /* flock(fd, LOCK_UN); */
        /* Thor.981205: 用 fcntl 取代flock, POSIX標準用法 */
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
    /* Thor.990416: 注意: 怎麼會有 .ACCT長度是0的, 而且只有 @目錄, 持續觀察中 */

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

//  time_t now = time(0);
    struct tm *p;
    p = localtime(&ap_start);
    sprintf(buf, "%02d/%02d/%02d %02d:%02d:%02d %cBBS\t%s\n",
        p->tm_year % 100, p->tm_mon + 1, p->tm_mday,
        p->tm_hour, p->tm_min, p->tm_sec, type, currtitle);

#if 0
    sprintf(buf, "%c%-12s[%s] %s\n", type, cuser.userid,
        Etime(&ap_start), currtitle);
    f_cat(FN_LOGIN_LOG, buf);
#endif

//  str_stamp(fpath, &ap_start);
//  sprintf(buf, "%s %cBBS\t%s\n", fpath, type, currtitle);
    /* Thor.990415: currtitle已內含ip */
    /* sprintf(buf, "%s %cBBS\t%s ip:%08x\n", fpath, type, currtitle, tn_addr); */
    /* Thor.980803: 追蹤 ip address */
    usr_fpath(fpath, cuser.userid, FN_LOG);
    f_cat(fpath, buf);

    if (type != ' ')
    {
        usr_fpath(fpath, cuser.userid, FN_LOGINS_BAD);
        sprintf(buf, "[%s] BBS %s\n", Ctime(&ap_start),
            (type == '*' ? currtitle : fromhost));
        f_cat(fpath, buf);
    }
}


/* ----------------------------------------------------- */
/* 登錄 BBS 程式                                         */
/* ----------------------------------------------------- */

static void
utmp_setup(
    int mode
)
{
    UTMP utmp;
    const char *const guestname[GUESTNAME]={GUEST_NAMES};

    cutmp = NULL; /* Thor.980805: pal_cache中會 check cutmp  */
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
    srand(time(0));
    srandom(time(0));
    strcpy(utmp.username, ((!str_cmp(cuser.userid, STR_GUEST)||!HAS_PERM(PERM_VALID)||HAS_PERM(PERM_DENYNICK))&&!HAS_PERM(PERM_SYSOP)) ? guestname[rand()%GUESTNAME] : cuser.username);
    strcpy(utmp.realname, cuser.realname);
    /* str_ncpy(utmp.from, fromhost, sizeof(utmp.from) - 1); */
    str_ncpy(utmp.from, fromhost, sizeof(utmp.from));
    /* Thor.980921: str_ncpy 含 0 */

    /* cache.20130407 保存使用者登入IP位置(IPv4) */
    unsigned char *addr;
    addr = (unsigned char *) &tn_addr;
    sprintf(ipv4addr, "%d.%d.%d.%d", addr[0], addr[1], addr[2], addr[3]);

    /* Thor: 告訴User已經滿了放不下... */

    if (!utmp_new(&utmp))
    {
        login_abort("\n您剛剛選的位子已經被人捷足先登了，請下次再來吧");
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

    /* 借 currtitle 一用 */

    /* sprintf(currtitle, "%s@%s", rusername, fromhost); */
    /* Thor.990415: 紀錄ip, 怕正查不到 */
    sprintf(currtitle, "%s@%s ip:%08lx (%d)", rusername, fromhost, tn_addr, currpid);


/* by visor */
#if 0
    move(20, 0);
    outs("關站中請見諒~~~~");
    vkey();
    sleep(10);
    exit(0);
#endif
/* by visor */

    /* 081119.cache: 正常顯示進站畫面 */
    move(b_lines, 0);
    prints("\x1b[m參觀用帳號：\x1b[1;32mguest\x1b[m  申請新帳號：\x1b[1;31mnew\x1b[m");

    /*move(b_lines, 0);
    outs("※ 無法連線時，請利用 port 3456 上站");*/

#ifdef loginAD

    /*090823.cache: 進站廣告*/
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
            login_abort("\n再見 ....");
        }

        vget(21, 0, msg_uid, uid, IDLEN + 1, DOECHO);

        if (str_cmp(uid, STR_NEW) == 0)
        {

#ifdef LOGINASNEW
            acct_apply(); /* Thor.980917: 註解: setup cuser ok */
            level = cuser.userlevel;
            ufo = cuser.ufo;
            /* cuser.userlevel = PERM_DEFAULT; */
            /* cuser.ufo = UFO_COLOR | UFO_MOVIE | UFO_BNOTE; */
            break;
#else
            outs("\n本系統目前暫停線上註冊，請用 guest 進入");
            continue;
#endif
        }
        else if (!*uid || (acct_load(&cuser, uid) < 0))
        {
            vmsg(err_uid);
        }
        else if (str_cmp(uid, STR_GUEST))
        {
            if (!vget(21, d_cols + 26, MSG_PASSWD, passbuf, PLAINPASSLEN, NOECHO | VGET_STEALTH_NOECHO))
            {
                continue;       /* 發現 userid 輸入錯誤，在輸入 passwd 時直接跳過 */
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

                if (!str_cmp(cuser.userid, str_sysop))
                    cuser.userlevel = ~0 ^ (PERM_DENYPOST | PERM_DENYTALK |
                        PERM_DENYCHAT | PERM_DENYMAIL | PERM_DENYSTOP | PERM_DENYNICK |
                        PERM_DENYLOGIN | PERM_PURGE);
                /* else */  /* Thor.980521: everyone should have level */

                level = cuser.userlevel;

                ufo = cuser.ufo & ~(HAS_PERM(PERM_LOGINCLOAK) ?
                /* lkchu.981201: 每次上站都要 'c' 頗麻煩 :p */
                    (UFO_BIFF | UFO_BIFFN | UFO_SOCKET | UFO_NET) :
                    (UFO_BIFF | UFO_BIFFN | UFO_SOCKET | UFO_NET | UFO_CLOAK));


                if ((level & PERM_ADMIN) && (cuser.ufo2 & UFO2_ACL))
                {
                    char buf[40]; /* check ip */
                    unsigned char *addr = (unsigned char*) &tn_addr;
                    sprintf(buf, "%d.%d.%d.%d", addr[0], addr[1], addr[2], addr[3]);
                    usr_fpath(fpath, cuser.userid, "acl");
                    str_lower(rusername, rusername);      /* lkchu.981201: 換小寫 */
                    str_lower(fromhost, fromhost);
                    if (acl_has(fpath, rusername, fromhost) == 0
                            || acl_has(fpath, rusername, buf) == 0)
                    {  /* Thor.980728: 注意 acl檔, 和 rusername, fromhost 要全部小寫 */
                        logattempt('*');
                        login_abort("\n你的上站地點不太對勁，請核對 [上站地點設定檔]");
                    }
                }

                logattempt(' ');

                /* check for multi-session */

                if (!(level & PERM_SYSOP))
                {
                    UTMP *ui;
                    pid_t pid;

                    if (level & (PERM_DENYLOGIN | PERM_PURGE))
                        login_abort("\n這個帳號暫停服務，詳情請向站長洽詢。");


                    if (!(ui = (UTMP *) utmp_find(cuser.userno)))
                        break;          /* user isn't logged in */


                    pid = ui->pid;
                    if (!pid || (kill(pid, 0) == -1))
                    {
                        memset(ui, 0, sizeof(UTMP));
                        break;          /* stale entry in utmp file */
                    }

                    if (vans("偵測到多重登入，您想刪除其他重複的 login (Y/n)嗎？[Y] ") != 'n')
                    {
                        kill(pid, SIGTERM);
                        blog("MULTI", cuser.username);
                        break;
                    }

                    if (utmp_count(cuser.userno, 0) > 2)
                    {
                        pmsg2_body("您已經達到多重登入上限");
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
            /* Thor.981207: 怕人亂玩, 強制寫回cuser.userlevel */
            ufo = UFO_PAGER | UFO_QUIET | UFO_MESSAGE | UFO_PAGER1 |UFO_HIDDEN;
            cuser.ufo = ufo;
            cuser.ufo2 = UFO2_COLOR | UFO2_BNOTE | UFO2_MOVIE;
            if (utmp_count(cuser.userno, 0) > MAXGUEST)
            {
                pmsg2_body("目前在線上的guest過多");
                login_abort("\n");
            }
            break;
        }
    }

    /* --------------------------------------------------- */
    /* 登錄系統                                          */
    /* --------------------------------------------------- */

    start = ap_start;


//  setproctitle("%s@%s", cuser.userid, fromhost);

    sprintf(fpath, "%s (%d)", currtitle, currpid);


    blog("ENTER", fpath);
    // IID.20190521: Currently unused; disabled for now.
    // if (rusername[0])
    //     strcpy(cuser.ident, currtitle);

    /* --------------------------------------------------- */
    /* 初始化 utmp、flag、mode                           */
    /* --------------------------------------------------- */

    cuser.ufo = ufo & (~UFO_WEB);
    ufo = cuser.ufo;
    utmp_setup(M_LOGIN); /* Thor.980917: 註解: cutmp, cutmp-> setup ok */
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
         * Thor: 內部改變過 userlevel, 所以 substitute_record 必須要可寫入
         * userlevel. 只有在 userlogin 時, 不可以改 userlevel, 其他時候均可以.
         */
        /* Thor.990318: 現在只有在"使用者資料稽核中"時不能改userlevel, 其他都可 */

        /* ------------------------------------------------- */
        /* 核對 user level                                       */
        /* ------------------------------------------------- */

#ifdef JUSTIFY_PERIODICAL
        /* if (level & PERM_VALID) */
        if ((level & PERM_VALID) && !(level & (PERM_SYSOP|PERM_XEMPT)))
        {  /* Thor.980819: 站長不作定期身份認證, ex. SysOp  */
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
            /* Thor.980825: lkchu patch: 既然有 NEWUSER_LIMIT, 還是加一下好了,
                                         否則一直回答 FAQ 挺累的. :) */
            if (start - cuser.firstlogin < 3 * 86400)
            {
                /* if (!(level & PERM_VALID)) */
                level &= ~PERM_POST;    /* 即使已經通過認證，還是要見習三天 */
            }
#endif

            /* Thor.980629: 未經身分認證, 禁止 chat/talk/write */
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
        /* 好友名單同步、清理過期信件                    */
        /* ------------------------------------------------- */

        if (start > cuser.tcheck + CHECK_PERIOD)
        {
#ifdef  HAVE_MAILGEM
            if (HAS_PERM(PERM_MBOX))
            {
                int (*p)(int level, char *fpath);
                char fpath[128];
                usr_fpath(fpath, cuser.userid, "gem");

                p = DL_GET(DL_NAME("mailgem.so", gcheck));
                if (p)
                    (*p)(0, fpath);
            }
#endif

            pal_sync(NULL); /* Thor.990318: 在這回認證信的機率不大:P */
            aloha_sync();
#ifdef  HAVE_BANMSG
            banmsg_sync(NULL);  /* 拒收訊息 */
#endif
            ufo |= m_quota(); /* Thor.980804: 註解, 資料整理稽核有包含 BIFF check */
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

        cutmp->ufo = cuser.ufo = ufo; /* Thor.980805: 解決 ufo 同步問題 */

        /* ------------------------------------------------- */
        /* 將 .ACCT 寫回                                         */
        /* ------------------------------------------------- */

#if 1
        /* Thor.990318: 為防止有大機率 有人在welcome畫面回認證信, 故移至此 */
        move(b_lines - 3, 0);
        prints("★ 歡迎您第 \x1b[1;33m%d\x1b[m 度拜訪本站，我記得那天是 \x1b[1;33m%s\x1b[m\n",
            cuser.numlogins, Ctime(&cuser.lastlogin));
        prints("★ 來自於 \x1b[1;33m%s\x1b[m", cuser.lasthost);
        /* Thor.990321: 將vmsg移至後方, 防止有人在此時回認證信 */
#endif

        cuser.lastlogin = start;
        cuser.userlevel = level;
        str_ncpy(cuser.lasthost, fromhost, sizeof(cuser.lasthost));
        usr_fpath(fpath, cuser.userid, FN_ACCT);
        fd = open(fpath, O_WRONLY);
        write(fd, &cuser, sizeof(ACCT));
        close(fd);
#if 1
        vmsg(NULL);
#endif

#if 1
        usr_fpath(fpath, cuser.userid, FN_LOGINS_BAD);
        /* Thor.990204: 為考慮more 傳回值 */
        if (more(fpath, (char *)-1) >= 0 && vans("偵測到登入失敗的記錄，您要刪除上述資訊嗎(Y/n)?[Y]") != 'n')
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
        /* 有效時間逾期 10 天前提出警告 */
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
            /* Thor.980825: lkchu patch: 既然有 NEWUSER_LIMIT, 還是加一下好了,
                                         否則一直回答 FAQ 挺累的. :) */
            if (start - cuser.firstlogin < 3 * 86400)
            {
                /* if (!(level & PERM_VALID)) */
                /* 即使已經通過認證，還是要見習三天 */
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
        if (!(ufo & UFO_CLOAK))     /* lkchu.981201: 自己隱身就不用 notify */
        {
            void loginNotify(void);
            loginNotify();
        }
        else
        {
            /* lkchu.981201: 清掉 benz, 不通知了 */
            usr_fpath(fpath, cuser.userid, FN_BENZ);
            unlink(fpath);
        }
#endif

#ifdef HAVE_ALOHA
        if (!(ufo & UFO_CLOAK))     /* lkchu.981201: 自己隱身就不用 notify */
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

    /* lkchu.990510: 重要公告放到 announce, 不跟 welcome 擠 :p */
    more(FN_ETC_ANNOUNCE, NULL);
#if 0
    bell();
    sleep(1);
    bell();
#endif

    /* Thor.980917: 註解: 到此為止應該設定好 cuser. level? ufo? rusername bbstate cutmp cutmp-> */

    if (!(cuser.ufo2 & UFO2_MOTD))
    {
        more("gem/@/@-day", NULL);      /* 今日熱門話題 */
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

/* 20100615.cache: 3000人以上會造成效能上的問題 */
//#ifdef        HAVE_CLASSTABLEALERT
//  classtable_free();
//  classtable_main();
//#endif

    showansi = cuser.ufo2 & UFO2_COLOR;
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

    /* act.sa_mask = 0; */ /* Thor.981105: 標準用法 */
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
    /* Thor.981206: lkchu patch: 統一 POSIX 標準用法  */
    /* 在此借用 sigset_t act.sa_mask */
    sigaddset(&act.sa_mask, SIGPIPE);
    sigprocmask(SIG_BLOCK, &act.sa_mask, NULL);

}


static inline void
tn_main(void)
{
    long nproc;
    double load[3], load_norm;
    char buf[128], buf2[40];
    unsigned char *addr = (unsigned char*) &tn_addr;

    sprintf(buf2, "%d.%d.%d.%d", addr[0], addr[1], addr[2], addr[3]);
    str_lower(buf, fromhost);

    if (acl_has(FN_ETC_BANIP_ACL, "", buf) > 0
        || acl_has(FN_ETC_BANIP_ACL, "", buf2) > 0)
        exit(0);

    /* 將 login 的部分清空 */
    add_io(0, 0);
    while (igetch() != I_TIMEOUT);
    add_io(0, 60);


    clear();
/*cache.080510: --維修用, 開站時要記得設定這段變成註解--*/
/*cache.080714: power user 可以輸入busy開啟login畫面測試*/
#if 0
    more("gem/@/@close", (char *) -1);    /* 禁止上站畫面 */
    move(b_lines -1, 0);
    outs("請按任意鍵離開...");
    if ( vkey() != 'c' || vkey() != 'c' || vkey() != 'n' || vkey() != 's' )
        login_abort("已中斷連線");
#endif
#if 0
    sprintf(buf, BBSHOME "/gem/@/@close");
    more(buf, (char *) -1);
    login_abort("\n夢之大地主機搬遷中...");
    return;
    sleep(10);
    login_abort("\n");
#endif


    //負載提到前面取得
    nproc = sysconf(_SC_NPROCESSORS_ONLN);
    getloadavg(load, 3);
    load_norm = load[0] / ((nproc > 0) ? nproc : 2);

    //負載過高禁止login
    if (load_norm>5)
    {
        prints("\n對不起...\n\n由於目前負載過高，請稍後再來...");
        //pcman會自動重連時間設太短會變成 DOS 很可怕 :P
        sleep(3);
        login_abort("\n");
    }

    //避免看板人氣負數
    currbno = -1;

    //getloadavg(load, 3);
    prints( MYHOSTNAME " ⊙ " OWNER " ⊙ " BBSIP " [" BBSVERNAME " " BBSVERSION "]\n"
"歡迎光臨【\x1b[1;33;46m %s \x1b[m】。系統負載：%.2f %.2f %.2f / %ld [%s] 線上人數 [%d/%d]",
        str_site, load[0], load[1], load[2], nproc, load_norm>5?"\x1b[1;37;41m過高\x1b[m":load_norm>1?"\x1b[1;37;42m偏高\x1b[m":"\x1b[1;37;44m正常\x1b[m", ushm->count, MAXACTIVE);

    film_out(FILM_INCOME, 2);

    total_num = ushm->count+1;
    currpid = getpid();
    cutmp = NULL;


    tn_signals();  /* Thor.980806: 放於 tn_login前, 以便 call in不會被踢 */
    tn_login();

    /* cache.090823 防止人數錯誤爆炸 */
    if (ushm->count < 0)
        ushm->count = 0;

    /*tn_signals(); */
    brh_load();
#ifdef  HAVE_FAVORITE
    if (HAS_PERM(PERM_VALID))
        favorite_main();
#endif

#ifdef  HAVE_DETECT_CROSSPOST
    if (attr_get(cuser.userid, ATTR_CROSS_KEY, &cksum)<0)
        memset(&cksum, 0, sizeof(CHECKSUMCOUNT));
#endif

    board_main();

    gem_main();

    talk_main();

#ifdef  HAVE_FORCE_BOARD
    force_board();  /* CdChen.990417: 強制閱讀公告板 */
#endif

    count_update();
    time(&ap_start);
#ifdef  HAVE_CHK_MAILSIZE
    if (!HAS_PERM(PERM_DENYMAIL) && HAS_PERM(PERM_BASIC))
    {
        if (mail_stat(CHK_MAIL_VALID))
        {
            chk_mailstat = 1;
            vmsg("您的信箱已超出容量，無法使用本功\能，請清理您的信箱！");
            xover(XZ_MBOX);
        }
    }
#endif

    menu();
    /* IID.20190929: `menu()` never returns. */
    /* abort_bbs(); */                  /* to make sure it will terminate */
}


/* ----------------------------------------------------- */
/* FSA (finite state automata) for telnet protocol       */
/* ----------------------------------------------------- */

static void
telnet_init(void)
{
    static const char svr[] = {
        IAC, DO, TELOPT_TTYPE,
        IAC, SB, TELOPT_TTYPE, TELQUAL_SEND, IAC, SE,
        IAC, WILL, TELOPT_ECHO,
        IAC, WILL, TELOPT_SGA,
        IAC, DO, TELOPT_BINARY
    };

    int n, len;
    const char *cmd;
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
/* 支援超過 24 列的畫面                                  */
/* ----------------------------------------------------- */

static void
term_init(void)
{
/* fuse.030518: 註解 */
//  server問：你會改變行列數嗎？(TN_NAWS, Negotiate About Window Size)
//  client答：Yes, I do. (TNCH_DO)
//
//  那麼在連線時，當TERM變化行列數時就會發出：
//  TNCH_IAC + TNCH_SB + TN_NAWS + 行數列數 + TNCH_IAC + TNCH_SE;

    /* ask client to report it's term size */
    static char svr[] =       /* server */
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

    /* 問對方 (telnet client) 有沒有支援不同的螢幕寬高 */
    send(0, svr, 3, 0);

    rset = 1;
    to.tv_sec = 1;
    to.tv_usec = 1;
    if (select(1, (fd_set *) & rset, NULL, NULL, &to) > 0)
        recv(0, buf, sizeof(buf), 0);

    rcv = NULL;
    if ((unsigned char) buf[0] == IAC && buf[2] == TELOPT_NAWS)
    {
        /* gslin: Unix 的 telnet 對有無加 port 參數的行為不太一樣 */
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

        /* b_lines 至少要 23，最多不能超過 T_LINES - 1 */
        if (b_lines >= T_LINES)
            b_lines = T_LINES - 1;
        else if (b_lines < 23)
            b_lines = 23;
        /* b_cols 至少要 79，最多不能超過 T_COLS - 1 */
        if (b_cols >= T_COLS)
            b_cols = T_COLS - 1;
        else if (b_cols < 79)
            b_cols = 79;
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
    int port /* Thor.981206: 取 0 代表 *沒有參數*, -1 代表 -i (inetd) */
             /* IID.20190903: `-2` represents `-u` <unix_socket>`. */ )
{
    int n;
    struct linger ld;
    struct sockaddr_un sun;
    struct sockaddr_in sin;
    struct sockaddr *psock;
    size_t sock_size;
#ifdef RLIMIT
    struct rlimit rl;
#endif
    char buf[80], data[80];
    time_t val;

    /*
     * More idiot speed-hacking --- the first time conversion makes the C
     * library open the files containing the locale definition and time zone.
     * If this hasn't happened in the parent process, it happens in the
     * children, once per connection --- and it does add up.
     */

    time(&val);
    strftime(buf, 80, "%d/%b/%Y %H:%M:%S", localtime(&val));

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
        /* Give up root privileges: no way back from here        */
        setgid(BBSGID);
        setuid(BBSUID);
#if 1
        n = sizeof(sin);
        if (getsockname(0, (struct sockaddr *) &sin, (socklen_t *) &n) >= 0)
            /* mport = */ port = ntohs(sin.sin_port);
#endif
        /* mport = port; */ /* Thor.990325: 不需要了:P */

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

    sun.sun_family = AF_UNIX;
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;

    if (port == 0) /* Thor.981206: port 0 代表沒有參數 */
    {
        n = MAXPORTS - 1;
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
        n = socket(PF_UNIX, SOCK_STREAM, 0);
    else
        n = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    val = 1;
    setsockopt(n, SOL_SOCKET, SO_REUSEADDR, (char *) &val, sizeof(val));

#if 0
    setsockopt(n, SOL_SOCKET, SO_KEEPALIVE, (char *) &val, sizeof(val));

    setsockopt(n, IPPROTO_TCP, TCP_NODELAY, (char *) &val, sizeof(val));
#endif

    ld.l_onoff = ld.l_linger = 0;
    setsockopt(n, SOL_SOCKET, SO_LINGER, (char *) &ld, sizeof(ld));

    /* mport = port; */ /* Thor.990325: 不需要了:P */

    if (port == -2)
    {
        psock = (struct sockaddr *) &sun;
        sock_size = sizeof(sun);

        strlcpy(sun.sun_path, unix_path, sizeof(sun.sun_path));
        // remove the file first if it exists.
        unlink(sun.sun_path);
    }
    else
    {
        psock = (struct sockaddr *) &sin;
        sock_size = sizeof(sin);

        sin.sin_port = htons(port);
    }
    if ((bind(n, psock, sock_size) < 0) || (listen(n, QLEN) < 0))
        exit(1);

    if (port == -2)
    {
        chown(unix_path, BBSUID, BBSGID);
        chmod(unix_path, 0600);
    }

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

    /* act.sa_mask = 0; */ /* Thor.981105: 標準用法 */
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
    int is_proxy = 0;           /* Whether connection data passing is enabled */
    struct sockaddr_in sin;

    /* --------------------------------------------------- */
    /* setup standalone daemon                             */
    /* --------------------------------------------------- */

    /* Thor.990325: usage, bbsd, or bbsd -i, or bbsd 1234 */
    /* Thor.981206: 取 0 代表 *沒有參數*, -1 代表 -i */

    switch (getopt(argc, argv, "p:u:i"))
    {
    case -1:
        if (!(optarg = argv[optind++]))
            break;
        // Falls through
        // to handle omitted `-p`
    case 'p':  /* IID.20190902: `bbsd [-p] 3456`. */
        if ((value = atoi(optarg)) > 0)
            break;

        if (0)  // Falls over the case
        {
    case 'u':  /* IID.20190903: `bbsd -u <unix_socket>` */
            unix_path = optarg;
            is_proxy = 1;
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
        value = 1;
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
                || cdata.cb != sizeof(cdata) || cdata.raddr_len != 4)
            {
                close(csock);
                continue;
            }

            sin.sin_addr.s_addr = *(u_long *)&cdata.raddr;
            sin.sin_port = cdata.rport;
        }

        time(&ap_start);
        argc = *totaluser;
        if (argc >= MAXACTIVE - 5 /* || *avgload > THRESHOLD */)
        {
            sprintf(currtitle,
                "目前線上人數 [%d] 人，系統滿載，請稍後再來\n", argc);
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
        /* Thor.990121: 免得反查時讓人久等 */

        telnet_init();

        sprintf(currtitle, "正進入%s...\n", str_site);
        send(0, currtitle, strlen(currtitle), 0);
#endif

        /* ------------------------------------------------- */
        /* ident remote host / user name via RFC931          */
        /* ------------------------------------------------- */

        /* rfc931(&sin, fromhost, rusername); */

        tn_addr = sin.sin_addr.s_addr;
        /* Thor.990325: 修改dns_ident定義, 來自哪if連那 */
        /* dns_ident(mport, &sin, fromhost, rusername); */

        /* cache.090728: 連線不反查, 增加速度 */
        unsigned char *addr;
        addr = (unsigned char *) &tn_addr;
        sprintf(fromhost, "%d.%d.%d.%d", addr[0], addr[1], addr[2], addr[3]);

        telnet_init();
        term_init();
        tn_main();
    }
}
