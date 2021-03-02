/*-------------------------------------------------------*/
/* util/mailpost.c      ( NTHU CS MapleBBS Ver 3.02 )    */
/*-------------------------------------------------------*/
/* target : (1) general user E-mail post 到看板          */
/*          (2) BM E-mail post 到精華區                  */
/*          (3) 自動審核身份認證信函之回信               */
/* create : 95/03/29                                     */
/* update : 97/03/29                                     */
/*-------------------------------------------------------*/
/* notice : brdshm (board shared memory) synchronize     */
/*-------------------------------------------------------*/

#include "bbs.h"

//#define LOG_FILE        "run/mailog"
#define LOG_FILE        FN_BBSMAILPOST_LOG

#define JUNK            0
#define NET_SAVE        1
#define LOCAL_SAVE      2
#define DIGEST          3

static int mymode = JUNK;

static ACCT myacct;
static char myfrom[128], mysub[128], myname[128], mypasswd[128], myboard[128], mytitle[128];

static int
acct_fetch(
    const char *userid
)
{
    int fd;
    char fpath[80], buf[80];

    str_lower(buf, userid);
    sprintf(fpath, "usr/%c/%s/.ACCT", *buf, buf);
    fd = open(fpath, O_RDWR, 0600);
    if (fd >= 0)
    {
        if (read(fd, &myacct, sizeof(ACCT)) != sizeof(ACCT))
        {
            // IID.20190505: Always fails; seems like this is disabled.
            close(fd);
            fd = -1;
        }
    }
    return fd;
}

/* ----------------------------------------------------- */
/* .BOARDS shared memory (cache.c)                       */
/* ----------------------------------------------------- */

static int
brd_fetch(
    const char* bname,
    BRD* brd
)
{
    FILE *fp;

    fp = fopen(FN_BRD, "r");
    if (!fp)
        return -1;

    while (fread(brd, sizeof(BRD), 1, fp) == 1)
    {
        if (!strcasecmp(bname, brd->brdname))
        {
            fclose(fp);
            return 0;
        }
    }

    fclose(fp);
    return -1;
}

/* ----------------------------------------------------- */
/* buffered I/O for stdin                                */
/* ----------------------------------------------------- */

#define POOL_SIZE       4096
#define LINE_SIZE       512

static char pool[POOL_SIZE];
static char mybuf[LINE_SIZE];
static int pool_size = POOL_SIZE;
static int pool_ptr = POOL_SIZE;

static int
readline(
    char *buf
)
{
    int ch;
    int len, bytes;

    len = bytes = 0;
    do
    {
        if (pool_ptr >= pool_size)
        {
/*          pool_size = read(0, pool, POOL_SIZE); */
            ch = fread(pool, 1, POOL_SIZE, stdin);
            if (ch <= 0)
                return 0;

            pool_size = ch;
            pool_ptr = 0;
        }
        ch = pool[pool_ptr++];
        bytes++;

        if (ch == '\r')
            continue;

        buf[len++] = ch;
    } while (ch != '\n' && len < (LINE_SIZE - 1));

    buf[len] = '\0';

    if (buf[0] == '.' && (buf[1] == '\n' || buf[1] == '\0'))
        return 0;

    return bytes;
}

/* ----------------------------------------------------- */
/* record run/mailog for management                      */
/* ----------------------------------------------------- */

static void
mailog(
    const char* mode,
    const char* msg
)
{
    FILE *fp;

    if ((fp = fopen(LOG_FILE, "a")))
    {
        time_t now;
        struct tm *p;

        time(&now);
        p = localtime(&now);
        fprintf(fp, "%02d/%02d %02d:%02d:%02d <%s> %s\n",
            p->tm_mon + 1, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec,
            mode, msg);
        fclose(fp);
    }
}

#if 0  // Unused
static int
Link(
    const char* src,
    const char* dst
)
{
    int ret;

    if ((ret = link(src, dst)))
    {
        if (errno != EEXIST)
            ret = f_cp(src, dst, O_EXCL);
    }
    return ret;
}
#endif

static void
justify_user(void)
{
    char buf[128];
    HDR mhdr;
    int fd;

    sprintf(buf, "usr/%c/%s/.DIR", *myname, myname);
    if (!hdr_stamp(buf, HDR_LINK, &mhdr, "etc/justified"))
    {
        strcpy(mhdr.title, "您已經通過身分認證了！");
        strcpy(mhdr.owner, "SYSOP");
        mhdr.xmode = MAIL_NOREPLY;
        rec_append(buf, &mhdr, sizeof(mhdr));
    }

    sprintf(buf, "usr/%c/%s/email", *myname, myname);
    fd = open(buf, O_WRONLY | O_CREAT | O_APPEND, 0600);
    if (fd >= 0)
    {
        char *base, *str;
        int count;

        count = pool_ptr;
        base = pool;
        str = base + count;
        str = strstr(str, "\n\n");
        if (str != NULL)
        {
            count = str - pool + 1;
        }
        write(fd, pool, count);
        close(fd);
    }

    myacct.vtime = time32(&myacct.tvalid);
    strcpy(myacct.justify, myfrom);
    myfrom[sizeof(myacct.justify)-1] = 0;
    strcpy(myacct.vmail, myacct.email);
    myacct.userlevel |= PERM_VALID;
}

static int
valid_ident(
    const char *ident
)
{
    static const char *const invalid[] = {"bbs@", "@bbs", "unknown@", "root@", "gopher@",
        "guest@", "@ppp", "@slip", NULL};
    char buf[128];
    const char *str;
    int i;

    str_lower(buf, ident);
    for (i = 0; (str = invalid[i]); i++)
    {
        if (strstr(buf, str))
            return 0;
    }
    return 1;
}

static void
verify_user(
    char *magic
)
{
    char *ptr, *next, buf[80];
    int fh;
    char buf2[512];
    int done;

/* printf("V1: %s\n", magic); */

    done = 0;
    strcpy(buf2, magic);

    if (valid_ident(myfrom) && (ptr = strchr(magic, '(')))
    {
        *ptr++ = '\0';

        fh = acct_fetch(magic);
        if (fh < 0)
        {
            sprintf(buf, "BBS user <%s> unknown: %s", magic, myfrom);
            mailog("verify", buf);
            puts(buf);
            return;
        }

        {
/*          printf("V2: %s\n", ptr); */

            if ((next = (char *) strchr(ptr, ')')))
            {
                *next = 0;

                if (strstr(next+1, "[VALID]"))
                {
                    if (str_hash(myacct.email, myacct.vtime) == chrono32(ptr))
                    {
                        str_lower(myname, magic);
                        justify_user();
                        lseek(fh, (off_t) 0, SEEK_SET);
                        write(fh, &myacct, sizeof(ACCT));
                        sprintf(buf, "[%s]%s", myacct.userid, myfrom);
                        mailog("valid", buf);
                        done = 1;
                    }
                }
                else
                {

                    str_lower(buf, myacct.email);
                /*  printf("V3: %x %x\n", hash32(buf), chrono32(ptr)); */
                    if (hash32(buf) == chrono32(ptr))
                    {
                        str_lower(myname, magic);
                        justify_user();
                        lseek(fh, (off_t) 0, SEEK_SET);
                        write(fh, &myacct, sizeof(ACCT));
                        sprintf(buf, "[%s]%s", myacct.userid, myfrom);
                        mailog("verify", buf);
                        done = 1;
                    }
                }
            }
        }
        close(fh);
    }

    if (!done)
    {
        sprintf(buf, "Invalid [%s] %s", buf2, myfrom);
        mailog("verify", buf);
    }
}


static int
post_article(void)
{
    int fd;
    FILE *fp;
    HDR hdr;
    char fpath[80], buf[128];

    if (mymode == JUNK)
    {
        pool_ptr = 0;
        if (!readline(mybuf))
            exit(0);

        if (!*myname)
            strcpy(myname, "<mailpost>");

        if (!*mytitle)
            strcpy(mytitle, *mysub ? mysub : "<< 原信照登 >>");
    }

    sprintf(fpath, "brd/%s/.DIR", mymode == JUNK ? BRD_JUNK : myboard);

#ifdef  DEBUG
    printf("dir: %s\n", fpath);
#endif

    fd = hdr_stamp(fpath, 'A', &hdr, buf);
    if (fd < 0)
    {
        sprintf(buf, "file error <%s>", fpath);
        mailog("mailpost", buf);
        return -1;
    }

    fp = fdopen(fd, "w");

#ifdef  DEBUG
    printf("post to %s\n", buf);
#endif

    if (mymode != JUNK)
    {
        fprintf(fp, "作者: %s (%s) %s: %s\n標題: %s\n時間: %s\n",
            myname, myacct.username, (mymode == LOCAL_SAVE ? "站內" : "看板"),
            myboard, mytitle, ctime_any(&hdr.chrono));

        hdr.xmode = (mymode == LOCAL_SAVE ? POST_EMAIL : POST_EMAIL | POST_OUTGO);
    }

    do
    {
        fputs(mybuf, fp);
    } while (readline(mybuf));
    fprintf(fp, "\n--\n※ Origin: %s ◆ Mail: %s\n", BOARDNAME, myfrom);
    fclose(fp);

    strcpy(hdr.owner, myname);
    if (mymode != JUNK)
        strcpy(hdr.nick, myacct.username);
    mytitle[TTLEN] = '\0';
    strcpy(hdr.title, mytitle);
    rec_bot(fpath, &hdr, sizeof(hdr));

    if ((mymode == NET_SAVE) && (fp = fopen("innd/out.bntp", "a")))
    {
        fprintf(fp, "%s\t%s\t%s\t%s\t%s\n",
            myboard, hdr.xname, hdr.owner, hdr.nick, hdr.title);
        fclose(fp);
    }

    if (mymode != JUNK)
    {
        sprintf(buf, "[%s]%s => %s", myname, myfrom, myboard);
        mailog("mailpost", buf);
    }

    return 0;
}


/* ----------------------------------------------------- */
/* E-mail post to gem                                    */
/* ----------------------------------------------------- */

static int
digest_article(void)
{
    /* return post_article();*/ /* quick & dirty */
    /* Thor.0606: post 到 精華區資源回收筒 */
    int fd;
    FILE *fp;
    HDR hdr;
    char fpath[80], buf[128];

    if (mymode == JUNK)
    {
/*      pool_ptr = 0;
        if (!readline(mybuf)) */
            exit(0);
/*
        if (!*myname)
            strcpy(myname, "<mailpost>");

        if (!*mytitle)
            strcpy(mytitle, *mysub ? mysub : "<< 原信照登 >>");
*/
    }

    sprintf(fpath, "gem/brd/%s/.GEM", myboard);

#ifdef  DEBUG
    printf("dir: %s\n", fpath);
#endif

    fd = hdr_stamp(fpath, 'A', &hdr, buf);
    if (fd < 0)
    {
        sprintf(buf, "file error <%s>", fpath);
        mailog("mailpost", buf);
        return -1;
    }

    fp = fdopen(fd, "w");

#ifdef  DEBUG
    printf("gem to %s\n", buf);
#endif

/*
    if (mymode != JUNK)
    {
*/
        fprintf(fp, "作者: %s (%s) %s: %s\n標題: %s\n時間: %s\n",
            myname, myacct.username, (mymode == LOCAL_SAVE ? "站內" : "看板"),
            myboard, mytitle, ctime_any(&hdr.chrono));

        hdr.xmode =  POST_EMAIL;
/*
        hdr.xmode = (mymode == LOCAL_SAVE ? POST_EMAIL : POST_EMAIL | POST_OUTGO);
    }
*/

    do
    {
        fputs(mybuf, fp);
    } while (readline(mybuf));
    fprintf(fp, "\n--\n※ Origin: %s ◆ Mail: %s\n", BOARDNAME, myfrom);
    fclose(fp);

    strcpy(hdr.owner, myname);
/*
    if (mymode != JUNK)
*/
        strcpy(hdr.nick, myacct.username);
    mytitle[TTLEN] = '\0';
    strcpy(hdr.title, mytitle);
    rec_append(fpath, &hdr, sizeof(hdr));
/*
    if ((mymode == NET_SAVE) && (fp = fopen("innd/out.bntp", "a")))
    {
        fprintf(fp, "%s\t%s\t%s\t%s\t%s\n",
            myboard, hdr.xname, hdr.owner, hdr.nick, hdr.title);
        fclose(fp);
    }
*/
/*
    if (mymode != JUNK)
    {
*/
        sprintf(buf, "[%s]%s => %s", myname, myfrom, myboard);
        mailog("mailpost", buf);
/*
    }
*/
    return 0;
}


static int
mailpost(void)
{
    int fh, dirty;
    char *ip, *ptr, *token, *key, buf[80];
    BRD brd;

    /* parse header */

    if (!readline(mybuf))
        return 0;

    if (strncasecmp(mybuf, "From ", 5))
        return post_article();  /* junk */

    dirty = *myfrom = *mysub = *myname = *mypasswd = *myboard = *mytitle = 0;

    while (!*myname || !*mypasswd || !*myboard || !*mytitle)
    {
        if (mybuf[0] == '#')
        {
            key = mybuf + 1;

            /* remove trailing space */

            if ((ptr = strchr(key, '\n')))
            {
                str_rstrip_tail(ptr);
            }

            /* split token & skip leading space */

            if ((token = strchr(key, ':')))
            {
                str_rstrip_tail(token);

                do
                {
                    fh = *(++token);
                } while (fh == ' ' || fh == '\t');
            }

            if (!str_casecmp(key, "name"))
            {
                strcpy(myname, token);
            }
            else if (!str_casecmp(key, "passwd") || !str_casecmp(key, "password") || !str_casecmp(key, "passward"))
            {
                strcpy(mypasswd, token);
            }
            else if (!str_casecmp(key, "board"))
            {
                strcpy(myboard, token);
            }
            else if (!str_casecmp(key, "title") || !str_casecmp(key, "subject"))
            {
                str_ansi(mytitle, token, sizeof(mytitle));
            }
            else if (!str_casecmp(key, "digest"))
            {
                mymode = DIGEST;
            }
            else if (!str_casecmp(key, "local"))
            {
                mymode = LOCAL_SAVE;
            }
        }
        else if (!strncasecmp(mybuf, "From", 4))
        {
            str_lower(myfrom, mybuf + 4);
            if (strstr(myfrom, "mailer-daemon"))        /* junk */
            {
                strcpy(mytitle, "<< 系統退信 >>");
                return post_article();
            }

            if ((ip = strchr(mybuf, '<')) && (ptr = strrchr(ip, '>')))
            {
                *ptr = '\0';
                if (ip[-1] == ' ')
                    ip[-1] = '\0';
                ptr = (char *) strchr(mybuf, ' ');
                while (*++ptr == ' ');
                sprintf(myfrom, "%s (%s)", ip + 1, ptr);
            }
            else
            {
#if 0
                strtok(mybuf, " ");
                strcpy(myfrom, (char *) strtok(NULL, " "));
#endif
                str_split_2nd(myfrom, mybuf);
            }
        }
        else if (!strncmp(mybuf, "Subject: ", 9))
        {
            /* audit justify mail */

            mmdecode_str(mybuf);
            /* if (ptr = strstr(mybuf, "[MapleBBS]To ")) */
            /* Thor.981012: 集中於 config.h 定義 */
            if ((ptr = strstr(mybuf, TAG_VALID)))
            {
                /* gslin.990101: TAG_VALID 長度不一定 */
                verify_user(ptr + STRLITLEN(TAG_VALID));
                /* verify_user(ptr + 13); */
                return 1;               /* eat mail queue */
            }

            if ((ptr = strchr(token = mybuf + 9, '\n')))
                *ptr = '\0';
            str_ansi(mysub, token, sizeof(mytitle));
        }

        if ((++dirty > 70) || !readline(mybuf))
        {
            mymode = JUNK;
            return post_article();      /* junk */
        }
    }

    dirty = 0;

    /* check if the userid is in our bbs now */

    fh = acct_fetch(myname);
    if (fh < 0)
    {
        sprintf(buf, "BBS user <%s> not existed", myname);
        mailog("mailpost", buf);
        puts(buf);
        return -1;
    }

    /* check password */

    if (chkpasswd(myacct.passwd, myacct.passhash, mypasswd))
    {
        close(fh);
        sprintf(buf, "BBS user <%s> password incorrect", myname);
        mailog("mailpost", buf);
        puts(buf);
        return -1;
    }

#ifdef  MAIL_POST_VALID
    if (!(myacct.userlevel & PERM_VALID) && valid_ident(myfrom))
    {

        /* ------------------------------ */
        /* 順便記錄 user's E-mail address */
        /* ------------------------------ */

        str_lower(myname, myname);
        justify_user();
        dirty = true;
    }
#endif

    /* check if the board is in our bbs now */

    if (brd_fetch(myboard, &brd))
    {
        close(fh);
        sprintf(buf, "No such board [%s]", myboard);
        mailog("mailpost", buf);
        puts(buf);
        return -1;
    }

    strcpy(myboard, brd.brdname);

    /* check permission */

    if (mymode != DIGEST)
    {
        if (mymode != LOCAL_SAVE)
            mymode = NET_SAVE;

        /* Thor.981123: lkchu patch: mailpost 文章數不增加問題 */
        /* IID.2021-03-02: Explanation: `&` has lower precedence than `==`; it should be enclosed in `()` */
        if (!(brd.battr & BRD_NOCOUNT))
        {
            myacct.numposts++;
            dirty = true;
        }
    }

    while (mybuf[0] && mybuf[0] != '\n')
    {
        if (!readline(mybuf))
            return 0;
    }

    while (mybuf[0] == '\n')
    {
        if (!readline(mybuf))
            return 0;
    }

    if (dirty && mybuf[0])
    {
        lseek(fh, (off_t) 0, SEEK_SET);
        write(fh, &myacct, sizeof(ACCT));
    }
    close(fh);

    strcpy(myname, myacct.userid);
    if (mybuf[0])
        return (mymode == DIGEST) ? digest_article() : post_article();

    mymode = JUNK;
    return post_article();
}


static void
sig_catch(
    int sig)
{
    char buf[40];

    sprintf(buf, "signal [%d]", sig);
    mailog("mailpost", buf);
    exit(0);
}

int
main(void)
{
    setgid(BBSGID);
    setuid(BBSUID);
    chdir(BBSHOME);

    signal(SIGBUS, sig_catch);
    signal(SIGSEGV, sig_catch);
    signal(SIGPIPE, sig_catch);

/*
*/

    if (mailpost())
    {
        /* eat mail queue */
        while (fread(pool, 1, POOL_SIZE, stdin) > 0)
        {
            sleep(10);
        }
        /* exit(-1); */
    }
    exit(0);
}
