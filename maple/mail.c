/*-------------------------------------------------------*/
/* mail.c       ( NTHU CS MapleBBS Ver 3.02 )            */
/*-------------------------------------------------------*/
/* target : local/internet mail routines                 */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/

#include "bbs.h"

//static int m_count(void);


/* ----------------------------------------------------- */
/* Link List routines                                    */
/* ----------------------------------------------------- */


#define MSG_CC "\x1b[32m[群組名單]\x1b[m\n"


LinkList *ll_head;              /* head of link list */
static LinkList *ll_tail;       /* tail of link list */


void
ll_new(void)
{
    LinkList *list, *next;

    list = ll_head;

    while (list)
    {
        next = list->next;
        free(list);
        list = next;
    }

    ll_head = ll_tail = NULL;
}


void
ll_add(
    const char *name)
{
    LinkList *node;
    int len;

    len = strlen(name) + 1;
    node = (LinkList *) malloc(SIZEOF_FLEX(LinkList, len));
    node->next = NULL;
    strcpy(node->data, name);

    if (ll_head)
        ll_tail->next = node;
    else
        ll_head = node;
    ll_tail = node;
}


int
ll_del(
    const char *name)
{
    LinkList *list, *prev, *next;

    prev = NULL;
    for (list = ll_head; list; list = next)
    {
        next = list->next;
        if (!strcmp(list->data, name))
        {
            if (prev == NULL)
                ll_head = next;
            else
                prev->next = next;

            if (list == ll_tail)
                ll_tail = prev;

            free(list);
            return 1;
        }
        prev = list;
    }
    return 0;
}


GCC_PURE int
ll_has(
    const char *name)
{
    const LinkList *list;

    for (list = ll_head; list; list = list->next)
    {
        if (!strcmp(list->data, name))
            return 1;
    }
    return 0;
}


void
ll_out(
    int row, int column,
    const char *msg)
{
    LinkList *list;
    int cur_rownum, ch, crow;


    move(row, column);
    clrtobot();
    outs(msg);
    cur_rownum = row;

    column = 80;
    for (list = ll_head; list; list = list->next)
    {
        msg = list->data;
        crow = strlen(msg) + 1;
        if (column + crow > 78)
        {
            column = crow;
            outc('\n');
            cur_rownum++;
            if (cur_rownum >= b_lines-1)
            {
                outs("★ 列表(C)繼續 (Q)結束 ? [C]");
                ch = vkey();
                if (ch == 'q' || ch == 'Q')
                    break;
                else
                {
                    move(row+2, 0);
                    clrtobot();
                    cur_rownum = row+1;
                }
            }
        }
        else
        {
            column += crow;
            outc(' ');
        }
        outs(msg);
    }
}

/* ----------------------------------------------------- */
/* BBS (batch) SMTP                                      */
/* ----------------------------------------------------- */


#ifdef  BATCH_SMTP
int
bsmtp(
    const char *fpath, const char *title, const char *rcpt,
    int method)
{
    char buf[80];
    time_t chrono;
    MailQueue mqueue;

    chrono = time(NULL);

    /* 中途攔截 */

    if (method != MQ_JUSTIFY)
    {
        /* stamp the queue file */

        strcpy(buf, "out/");

        for (;;)
        {
            archiv32(chrono, buf + 4);
            if (!f_ln(fpath, buf))
                break;
            chrono++;
        }
        fpath = buf;

        strcpy(mqueue.filepath, fpath);
        strcpy(mqueue.subject, title);
    }

    /* setup mail queue */

    mqueue.mailtime = chrono;
    mqueue.method = method;
    strcpy(mqueue.sender, cuser.userid);
    strcpy(mqueue.username, cuser.username);
    strcpy(mqueue.rcpt, rcpt);
    if (rec_add(MAIL_QUEUE, &mqueue, sizeof(mqueue)) < 0)
        return -1;

    cuser.numemail++;           /* 記錄使用者共寄出幾封 Internet E-mail */
    return chrono;
}

#else

/* ----------------------------------------------------- */
/* (direct) SMTP                                         */
/* ----------------------------------------------------- */


int
bsmtp(
    const char *fpath, const char *title, char *rcpt,
    int method)
{
    int sock;
    time_t chrono, stamp;
    FILE *fp, *fr, *fw;
    char *str, buf[512], from[80], subject[80], msgid[80], keyfile[80], valid[10];
#ifdef HAVE_SIGNED_MAIL
    const char *signature = NULL;
    char prikey[PLAINPASSLEN];
    union {
        char str[PLAINPASSLEN];
        struct {
            unsigned int hash, hash2;
        } val;
    } sign = {{0}};

    *prikey = prikey[PLAINPASSLEN-1] = sign.str[PLAINPASSLEN-1] = '\0'; /* Thor.990413:註解: 字串結束 */
#endif

    cuser.numemail++;           /* 記錄使用者共寄出幾封 Internet E-mail */
    chrono = time(&stamp);

    /* --------------------------------------------------- */
    /* 身分認證信函                                        */
    /* --------------------------------------------------- */

    if (method == MQ_JUSTIFY)
    {
        fpath = FN_ETC_VALID;
        /* Thor.990125: MYHOSTNAME統一放入 str_host */
        sprintf(from, "bbsreg@%s", str_host);
        archiv32(str_hash(rcpt, chrono), buf);
        /* sprintf(subject, "[MapleBBS]To %s(%s) [VALID]", cuser.userid, buf); */
        /* Thor.981012: 集中在 config.h 管理 */
        sprintf(subject, TAG_VALID"%s(%s) [VALID]", cuser.userid, buf);
        title = subject;

        usr_fpath(keyfile, cuser.userid, FN_REGKEY);
        if ( ( fp = fopen(keyfile, "w") ) )
        {
            fprintf(fp, "%s\n", buf);
            fclose(fp);
        }
        strncpy(valid, buf, 10);

    }
    else
    {
        /* Thor.990125: MYHOSTNAME統一放入 str_host */
        sprintf(from, "%s.bbs@%s", cuser.userid, str_host);
    }

#ifdef HAVE_SMTP_SERVER
    {
        int i;
        const char *const alias[] = SMTP_SERVER, *str_alias;
        sock = -1;
        for (i=0; (str_alias = alias[i]); i++)
        {
            sock = dns_open(str_alias, 25);
            if (sock >= 0)
                break;
        }
        if (sock < 0)
        {
            str = strchr(rcpt, '@') + 1;
            sock = dns_smtp(str);
        }
    }
#else
    str = strchr(rcpt, '@') + 1;
    sock = dns_smtp(str);
#endif

    if (sock >= 0)
    {
        archiv32(chrono, msgid);

        move(b_lines, 0);
        clrtoeol();

        if (method)
        {
            prints("★ 寄信給 %s \x1b[5m...\x1b[m", rcpt);
            refresh();
        }
        else
        {
            prints("★ 寄信中 \x1b[5m...\x1b[m");
            refresh();
        }

        sleep(1);                       /* wait for mail server response */

        fr = fdopen(sock, "r");
        fw = fdopen(sock, "w");

        fgets(buf, sizeof(buf), fr);
        if (memcmp(buf, "220", 3))
            goto smtp_error;
        while (buf[3] == '-') /* maniac.bbs@WMStar.twbbs.org 2000.04.18 */
            fgets(buf, sizeof(buf), fr);

        /* Thor.990125: MYHOSTNAME統一放入 str_host */
        fprintf(fw, "HELO %s\r\n", str_host);
        fflush(fw);
        do
        {
            fgets(buf, sizeof(buf), fr);
            if (memcmp(buf, "250", 3))
                goto smtp_error;
        } while (buf[3] == '-');

        fprintf(fw, "MAIL FROM:<%s>\r\n", from);
        fflush(fw);
        do
        {
            fgets(buf, sizeof(buf), fr);
            if (memcmp(buf, "250", 3))
                goto smtp_error;
        } while (buf[3] == '-');

        fprintf(fw, "RCPT TO:<%s>\r\n", rcpt);
        fflush(fw);
        do
        {
            fgets(buf, sizeof(buf), fr);
            if (memcmp(buf, "250", 3))
                goto smtp_error;
        } while (buf[3] == '-');

/*      fprintf(fw, "DATA\r\n", rcpt);*/ /* statue.000713 */
        fprintf(fw, "DATA\r\n");
        fflush(fw);
        do
        {
            fgets(buf, sizeof(buf), fr);
            if (memcmp(buf, "354", 3))
                goto smtp_error;
        } while (buf[3] == '-');

        /* ------------------------------------------------- */
        /* begin of mail header                              */
        /* ------------------------------------------------- */

        /* Thor.990125: 儘可能的像 RFC822 & sendmail的作法, 免得別人不接:p */
        fprintf(fw, "From: %s\r\nTo: %s\r\nSubject: %s\r\nX-Sender: %s (%s)\r\n"
            "Date: %s\r\nMessage-Id: <%s@%s>\r\n"
            "X-Disclaimer: [%s] 對本信內容恕不負責\r\n\r\n",
            from, rcpt, title, cuser.userid, cuser.username,
            Atime(&stamp), msgid, str_host,
            str_site);

        if (method & MQ_JUSTIFY)        /* 身分認證信函 */
        {
            fprintf(fw, " ID: %s (%s)  E-mail: %s\r\n\r\n",
                cuser.userid, cuser.username, rcpt);
        }

        /* ------------------------------------------------- */
        /* begin of mail body                                */
        /* ------------------------------------------------- */

        if ((fp = fopen(fpath, "r")))
        {
            char *ptr;

            str = buf;
            *str++ = '.';
            while (fgets(str, sizeof(buf) - 3, fp))
            {
                if ((ptr = strchr(str, '\n')))
                {
                    *ptr++ = '\r';
                    *ptr++ = '\n';
                    *ptr = '\0';
                }
                fputs((*str == '.' ? buf : str), fw);
            }
            fclose(fp);
        }
#ifdef HAVE_SIGNED_MAIL
        if (!(method & MQ_JUSTIFY) && !rec_get(PRIVATE_KEY, prikey, PLAINPASSLEN-1, 0))
        /* Thor.990413: 除了認證函外, 其他信件都要加sign */
        {
            /* Thor.990413: buf用不到了, 借來用用 :P */
            sprintf(buf, "%s -> %s", cuser.userid, rcpt);
            sign.val.hash = str_hash(buf, stamp);
            sign.val.hash2 = str_hash2(buf, sign.val.hash);
            str_xor(sign.str, prikey);
            explicit_zero_bytes(prikey, sizeof(prikey));
            /* Thor.990413: 不加()的話, 時間尾空白會被吃掉(驗證時) */
            fprintf(fw, "\x1b[1;32m※ X-Info: \x1b[33m%s\x1b[m\r\n\x1b[1;32m※ X-Sign: \x1b[36m%s%s \x1b[37m(%s)\x1b[m\r\n",
                buf, msgid, signature = gensignature(sign.str), Btime(&stamp));
        }
#endif
        fputs("\r\n.\r\n", fw);
        fflush(fw);

        fgets(buf, sizeof(buf), fr);
        if (memcmp(buf, "250", 3))
            goto smtp_error;

        fputs("QUIT\r\n", fw);
        fflush(fw);
        fclose(fw);
        fclose(fr);
        goto smtp_log;

smtp_error:

        fclose(fr);
        fclose(fw);
        sprintf(msgid + 7, "\n\t%.70s", buf);
        chrono = -1;
    }
    else
    {
        chrono = -1;
        strcpy(msgid, "CONN");
    }

smtp_log:

    /* --------------------------------------------------- */
    /* 記錄寄信                                            */
    /* --------------------------------------------------- */

    sprintf(buf, "%s%-13s%c> %s %s %s\n\t%s\n\t%s\n", Btime(&stamp), cuser.userid,
        ((method == MQ_JUSTIFY) ? '=' : '-'), rcpt, msgid,
#ifdef HAVE_SIGNED_MAIL
            signature? signature: "NoPriKey",
#else
            "",
#endif
            title, fpath);
    f_cat(FN_MAIL_LOG, buf);

    return chrono;
}
#endif  /* #ifdef  BATCH_SMTP */

#ifdef HAVE_DOWNLOAD
int
bsmtp_file(
    const char *fpath, const char *title, char *rcpt)
{
    int sock;
    time_t chrono, stamp;
    FILE *fp, *fr, *fw;
    char *str, buf[512], from[80], msgid[80], boundary[256];
    char fname[256];
    struct tm ntime, *xtime;



    cuser.numemail++;           /* 記錄使用者共寄出幾封 Internet E-mail */
    chrono = time(&stamp);
    xtime = localtime(&chrono);
    ntime = *xtime;

    sprintf(fname, "mail_%04d%02d%02d.tgz", ntime.tm_year + 1900, ntime.tm_mon + 1, ntime.tm_mday);

    /* --------------------------------------------------- */
    /* 身分認證信函                                        */
    /* --------------------------------------------------- */

    /* Thor.990125: MYHOSTNAME統一放入 str_host */
    sprintf(from, "%s.bbs@%s", cuser.userid, str_host);

#ifdef HAVE_SMTP_SERVER
    {
        int i;
        const char *const alias[] = SMTP_SERVER, *str_alias;
        for (i=0; (str_alias = alias[i]); i++)
        {
            sock = dns_open(str_alias, 25);
            if (sock >= 0)
                break;
        }
        if (sock < 0)
        {
            str = strchr(rcpt, '@') + 1;
            sock = dns_smtp(str);
        }
    }
#else
    str = strchr(rcpt, '@') + 1;
    sock = dns_smtp(str);
#endif

    if (sock >= 0)
    {
        archiv32(chrono, msgid);

        sprintf(boundary, "----=_NextPart_%s", msgid);

        move(b_lines, 0);
        clrtoeol();

        prints("★ 寄信給 %s \x1b[5m...\x1b[m", rcpt);
        refresh();

        sleep(1);                       /* wait for mail server response */

        fr = fdopen(sock, "r");
        fw = fdopen(sock, "w");

        fgets(buf, sizeof(buf), fr);
        if (memcmp(buf, "220", 3))
            goto smtp_file_error;
        while (buf[3] == '-') /* maniac.bbs@WMStar.twbbs.org 2000.04.18 */
            fgets(buf, sizeof(buf), fr);

        /* Thor.990125: MYHOSTNAME統一放入 str_host */
        fprintf(fw, "HELO %s\r\n", str_host);
        fflush(fw);
        do
        {
            fgets(buf, sizeof(buf), fr);
            if (memcmp(buf, "250", 3))
                goto smtp_file_error;
        } while (buf[3] == '-');

        fprintf(fw, "MAIL FROM:<%s>\r\n", from);
        fflush(fw);
        do
        {
            fgets(buf, sizeof(buf), fr);
            if (memcmp(buf, "250", 3))
                goto smtp_file_error;
        } while (buf[3] == '-');

        fprintf(fw, "RCPT TO:<%s>\r\n", rcpt);
        fflush(fw);
        do
        {
            fgets(buf, sizeof(buf), fr);
            if (memcmp(buf, "250", 3))
                goto smtp_file_error;
        } while (buf[3] == '-');

        fprintf(fw, "DATA\r\n");
        fflush(fw);
        do
        {
            fgets(buf, sizeof(buf), fr);
            if (memcmp(buf, "354", 3))
                goto smtp_file_error;
        } while (buf[3] == '-');

        /* ------------------------------------------------- */
        /* begin of mail header                              */
        /* ------------------------------------------------- */

        /* Thor.990125: 儘可能的像 RFC822 & sendmail的作法, 免得別人不接:p */
        fprintf(fw, "From: %s\r\nTo: %s\r\nSubject: %s\r\nX-Sender: %s (%s)\r\n"
            "Date: %s\r\nMessage-Id: <%s@%s>\r\n"
            "X-Disclaimer: [%s] 對本信內容恕不負責\r\n",
            from, rcpt, title, cuser.userid, cuser.username,
            Atime(&stamp), msgid, str_host,
            str_site);

        fprintf(fw, "MIME-Version: 1.0\r\nContent-Type: multipart/mixed;\r\n"
                "\tboundary=\"%s\"\r\n\r\n", boundary);

        fprintf(fw, "This is a multi-part message in MIME format.\r\n");
        fprintf(fw, "--%s\r\nContent-Type: text/plain;\r\n\tcharset=\"big5\"\r\n"
                "Content-Transfer-Encoding: 8bit\r\n\r\n附件名稱：%s\r\n", boundary, fname);

        fprintf(fw, "--%s\r\nContent-Type: application/x-compressed;\r\n\tname=\"%s\"\r\n"
                "Content-Transfer-Encoding: base64\r\nContent-Disposition: attachment;\r\n"
                "\tfilename=\"%s\"\r\n\r\n", boundary, fname, fname);

        /* ------------------------------------------------- */
        /* begin of mail body                                */
        /* ------------------------------------------------- */

        if ((fp = fopen(fpath, "r")))
        {
            char *ptr;

            str = buf;
            *str++ = '.';
            while (fgets(str, sizeof(buf) - 3, fp))
            {
                if ((ptr = strchr(str, '\n')))
                {
                    *ptr++ = '\r';
                    *ptr++ = '\n';
                    *ptr = '\0';
                }
                fputs((*str == '.' ? buf : str), fw);
            }
            fclose(fp);
        }
        fprintf(fw, "--%s--\r\n", boundary);

        fputs("\r\n.\r\n", fw);
        fflush(fw);

        fgets(buf, sizeof(buf), fr);
        if (memcmp(buf, "250", 3))
            goto smtp_file_error;

        fputs("QUIT\r\n", fw);
        fflush(fw);
        fclose(fw);
        fclose(fr);
        goto smtp_file_log;

smtp_file_error:

        fclose(fr);
        fclose(fw);
        sprintf(msgid + 7, "\n\t%.70s", buf);
        chrono = -1;
    }
    else
    {
        chrono = -1;
        strcpy(msgid, "CONN");
    }

smtp_file_log:

    /* --------------------------------------------------- */
    /* 記錄寄信                                            */
    /* --------------------------------------------------- */

    sprintf(buf, "%s%-13s> %s %s\n\t%s\n\t%s\n", Btime(&stamp), cuser.userid,
        rcpt, msgid, title, fpath);
    f_cat(FN_MAIL_LOG, buf);

    return chrono;
}

#endif  /* #ifdef HAVE_DOWNLOAD */

#ifdef HAVE_SIGNED_MAIL
/* Thor.990413: 提供驗證功能 */

#define SIGNATURELEN  (PASSLEN-1-3 + PASSHASHLEN-1-1 + 1)
int
m_verify(void)
{
    time_t chrono;
    char info[79], *p;
    char sign[79], *q;
    char buf[160];

    char prikey[PLAINPASSLEN];
    union {
        char str[PLAINPASSLEN];
        struct {
            unsigned int hash, hash2;
        } val;
    } s = {{0}};

    prikey[PLAINPASSLEN-1] = s.str[PLAINPASSLEN-1] = '\0'; /* Thor.990413:註解: 字串結束 */

    if (rec_get(PRIVATE_KEY, prikey, PLAINPASSLEN-1, 0))
    {
        zmsg("本系統並無電子簽章，請洽SYSOP");
        return XEASY;
    }

    move(13, 0);
    clrtobot();
    move(15, 0);
    outs("請依序輸入信末兩行 X-Info X-Sign 以進行驗證");

    if (!vget(17, 0, ":", info, sizeof info, DOECHO) ||
        !vget(18, 0, ":", sign, sizeof sign, DOECHO))
        return 0;

    str_trim(info); /* Thor: 去尾巴, for ptelnet自動加空白 */
    str_trim(sign);

    if (!memcmp("※ X-Info: ", p = info, 11))
        p += 11;
    while (*p == ' ') p++; /* Thor: 去前頭 */

    if (!memcmp("※ X-Sign: ", q = sign, 11))
        q += 11;
    while (*q == ' ') q++;

    if (strlen(q) < 7 + PASSLEN-1 || (isalnum(q[7+PASSLEN-1]) && strlen(q) < 7 + SIGNATURELEN-1))
    {
        vmsg("電子簽章有誤");
        return 0;
    }

    str_ncpy(s.str + 1, q, 8);  /* Thor: 暫借一下 s.str */
    chrono = chrono32(s.str); /* prefix 1 char */

    q += 7; /* real sign */
    if (isalnum(q[PASSLEN-1]))
        q[SIGNATURELEN-1] = 0; /* 補尾0 */
    else
        q[PASSLEN-1] = 0; /* 補尾0 */

    s.val.hash = str_hash(p, chrono);
    s.val.hash2 = str_hash2(p, s.val.hash);
    str_xor(s.str, prikey);
    explicit_zero_bytes(prikey, sizeof(prikey));

    sprintf(buf, "(%s)", Btime(&chrono));

    if (chksignature(q, s.str) || strcmp(q + ((isalnum(q[PASSLEN-1])) ? SIGNATURELEN : PASSLEN), buf))
    {
        /* Thor.990413: log usage */
        sprintf(buf, "%s@%s - XInfo:%s", rusername, fromhost, p);
        blog("VRFY", buf);
        /* Thor: fake sign */
        move(20, 25);
        outs("\x1b[41;37;5m *注意* 驗證錯誤! \x1b[m");
        vmsg("此信並非由本站所發，請查照");
        return 0;
    }

    sprintf(buf, "%s@%s + XInfo:%s", rusername, fromhost, p);
    blog("VRFY", buf);

    vmsg("此信由本站所發出");
    return 0;
}
#endif  /* #ifdef HAVE_SIGNED_MAIL */

/* ----------------------------------------------------- */
/* mail routines                                         */
/* ----------------------------------------------------- */

static union
{
    XO mail_xo;
    char bytes[SIZEOF_FLEX(XO, 32)];
}      cmbox_union;
static XO *const cmbox = &cmbox_union.mail_xo;

int
m_total_size(void)
{
    int fd, fsize, total;
    struct stat st;
    HDR *head, *tail;
    char *base, *folder, fpath[80];
    int changed;

    if ((fd = open(folder = cmbox->dir, O_RDWR)) < 0)
        return 0;

    fsize = 0;
    total = 0;
    changed = 0;

    if (!fstat(fd, &st) && (fsize = st.st_size) >= sizeof(HDR) &&
        (base = (char *) malloc(fsize)))
    {

        f_exlock(fd);

        if ((fsize = read(fd, base, fsize)) >= sizeof(HDR))
        {
            head = (HDR *) base;
            tail = (HDR *) (base + fsize);

            do
            {
                if (head->xid > 0)
                {
                    total += head->xid;
                }
                else
                {
                    hdr_fpath(fpath, folder, head);
                    stat(fpath, &st);
                    total += st.st_size;
                    head->xid = st.st_size;
                    changed = 1;
                }
            } while (++head < tail);

        }

        if (changed == 1)
        {
            lseek(fd, (off_t) 0, SEEK_SET);
            write(fd, base, fsize);
            ftruncate(fd, fsize);
        }
        f_unlock(fd);

        free(base);
    }

    close(fd);

    return total;
}


unsigned int
m_quota(void)
{
    unsigned int ufo;
    int fd, count GCC_UNUSED, fsize, limit, xmode;
    GCC_UNUSED time_t mail_due, mark_due;
    struct stat st;
    HDR *head, *tail;
    char *base, *folder, date[9];

    if ((fd = open(folder = cmbox->dir, O_RDWR)) < 0)
        return 0;

    ufo = 0;
    fsize = 0;

    if (!fstat(fd, &st) && (fsize = st.st_size) >= sizeof(HDR) &&
        (base = (char *) malloc(fsize)))
    {

        /* flock(fd, LOCK_EX); */
        /* Thor.981205: 用 fcntl 取代flock, POSIX標準用法 */
        f_exlock(fd);

        if ((fsize = read(fd, base, fsize)) >= sizeof(HDR))
        {
            int prune;          /* number of pruned mail */

            limit = time(0);
            mail_due = limit - MAIL_DUE * 86400;
            mark_due = limit - MARK_DUE * 86400;
            st.st_mtime = limit + CHECK_PERIOD;
            str_stamp(date, &st.st_mtime);

            limit = cuser.userlevel;
            if (limit & (PERM_SYSOP | PERM_MBOX))
                limit = MAX_BBSMAIL;
            else
                limit = limit & PERM_VALID ? MAX_VALIDMAIL : MAX_NOVALIDMAIL;

            count = fsize / sizeof(HDR);

            head = (HDR *) base;
            tail = (HDR *) (base + fsize);

            prune = 0;
#if 1
            do
            {
                xmode = head->xmode;

#if 0
                if (xmode & MAIL_DELETE)
                {
                    char fpath[64];

                    hdr_fpath(fpath, folder, head);
                    unlink(fpath);
                    prune--;
                    continue;
                }
#endif
                if (!(xmode & MAIL_READ))
                    ufo |= UFO_BIFF;

#if 0
                if ((count > limit) ||
                    (head->chrono <= (xmode & MAIL_MARKED ? mark_due : mail_due)))
                {
                    count--;
                    head->xmode = xmode | MAIL_DELETE;
                    strcpy(head->date, date);
                    ufo |= UFO_MQUOTA;
                }
#else
                head->xmode = xmode & (~MAIL_DELETE);
#endif
                if (prune)
                    head[prune] = head[0];

            } while (++head < tail);
#endif  /* #if 1 */
            fsize += (prune * sizeof(HDR));
#if 0
            if ((fsize > 0) && (prune || (ufo & UFO_MQUOTA)))
            {
                lseek(fd, 0, SEEK_SET);
                write(fd, base, fsize);
                ftruncate(fd, fsize);
            }

#else
            ufo &= ~UFO_MQUOTA;
            if (fsize > 0)
            {
                lseek(fd, 0, SEEK_SET);
                write(fd, base, fsize);
                ftruncate(fd, fsize);
            }
#endif

        }

        /* flock(fd, LOCK_UN); */
        /* Thor.981205: 用 fcntl 取代flock, POSIX標準用法 */
        f_unlock(fd);

        free(base);
    }

    close(fd);
    if (fsize < sizeof(HDR))
        unlink(folder);

    return ufo;
}

#ifdef  HAVE_DOWNLOAD


#define PACKLOG     "run/mzip.log"

static void
packlog(
    const char *packmsg)
{
    FILE *fp;

    if ((fp = fopen(PACKLOG, "a")))
    {
        time_t now;
        struct tm *p;

        time(&now);
        p = localtime(&now);
        fprintf(fp, "%02d/%02d %02d:%02d:%02d <mzip> %s\n",
            p->tm_mon + 1, p->tm_mday,
            p->tm_hour, p->tm_min, p->tm_sec,
            packmsg);
        fclose(fp);
    }
}

/* ----------------------------------------------------- */
/* Zip mbox & board gem                                  */
/* ----------------------------------------------------- */


static void
do_forward(
    const char *title,
    int mode)
{
    int rc;
    char *userid;
    char addr[64], fpath[128], cmd[256];

    if (strstr(cuser.email, ".bbs@"))
    {
        vmsg("使用 BBS 信箱作為認證信箱/使用註冊單認證者無法打包！");
        return;
    }

    strcpy(addr, cuser.email);

/*
 *  if (!vget(B_LINES_REF, 0, "請輸入轉寄地址：", addr, 60, GCARRY))
 *      return;
 *
 *  if (not_addr(addr))
 *  {
 *      zmsg("不合格的 E-mail address");
 *      return;
 *  }
 *
 *  sprintf(fpath, "確定寄給 [%s] 嗎(y/N)？[N] ", addr);
 */

    sprintf(fpath, "確定寄給 [%s] 嗎(y/N)？[N] ", cuser.email);
    if (vans(fpath) != 'y')
        return;

    userid = strchr(addr, '@') + 1;
    if (dns_smtp(userid) >= 0)   /* itoc.註解: 雖然 bsmtp_file() 也會做，但是這邊先做，以免壓縮完才知道是無效的工作站地址 */
    {
        userid = cuser.userid;

        if (mode == '1')                /* 個人信件 */
        {
            /* usr_fpath(fpath, userid, "@"); */
            /* 因為 .DIR 不在 @/ 裡，要一起打包 */
            usr_fpath(cmd, userid, fn_dir);
            usr_fpath(fpath, userid, "@");
            strcat(fpath, " ");
            strcat(fpath, cmd);
        }
        else if (mode == '2')   /* 看板文章 */
        {
            brd_fpath(fpath, currboard, NULL);
        }
        else /* if (mode == '3') */     /* 看板精華區 */
        {
            gem_fpath(fpath, currboard, NULL);
        }

//      sprintf(cmd, "tar -zcv -f - %s | " BINARY_SUFFIX "base64encode > tmp/%s.tgz", fpath, userid);
//      r2.20180316: try system default util to excute base64 encoding
        #ifdef __linux__
            sprintf(cmd, "tar -zcf - %s | base64 > tmp/%s.tgz", fpath, userid);
        #else
            #ifdef __FreeBSD__
                sprintf(cmd, "tar -zcf - %s | b64encode -r %s > tmp/%s.tgz", fpath, userid, userid);
            #else
                sprintf(cmd, "tar -zcf - %s | " BINARY_SUFFIX "base64encode > tmp/%s.tgz", fpath, userid);
            #endif
        #endif  /* #ifdef __linux__ */

        system(cmd);

        sprintf(fpath, "tmp/%s.tgz", userid);
//      rc = bsmtp(fpath, title, addr, 0x02);
        rc = bsmtp_file(fpath, title, addr);
        unlink(fpath);

        if (rc >= 0)
        {
            char sentmsg[256];

            if (mode == '1')
            {
                sprintf(sentmsg, "user: %s , mailbox , sent to: %s", userid, addr);
            }
            else
            {
                sprintf(sentmsg, "user: %s , board: %s , sent to: %s", userid, currboard, addr);
            }
            packlog(sentmsg);
            vmsg("信已寄出");
            return;
        }
    }

    vmsg("信件無法寄達");
}


int
m_zip(void)                     /* itoc.010228: 打包資料 */
{
    int ans;
    const char *name, *item;
    char buf[80];

    ans = vans("打包資料 1)個人信件 2)看板文章 3)看板精華區 到註冊信箱 [Q] ");

    if (ans == '1')
    {
        name = cuser.userid;
        item = "個人信件";
    }
    else if (ans == '2' || ans == '3')
    {
        /* itoc.註解: 限定只能打包目前閱讀的看板，不能閱讀秘密看板的人就不能打包該板/精華區 */
        /* itoc.020612: 為了防止 POST_RESTRICT/GEM_RESTRICT 的文章外流，非板主就不能打包 */

        if (currbno < 0)
        {
            vmsg("請先進入您要打包的看板，再來此打包");
            return XEASY;
        }

        if ((ans == '2' && !(bbstate & STAT_BOARD)) || (ans == '3' && !(bbstate & STAT_BOARD)))
                                    /* tmp not STAT_BM */
        {
            vmsg("只有板主才能打包看板文章及看板精華區");
            return XEASY;
        }

        if (!(strstr(currboard, "P_")))
        {
            vmsg("非個人板暫不提供打包服務，特殊個人專板若需打包請洽站務");
            return XEASY;
        }

        name = currboard;
        item = (ans == '2') ? "看板文章" : "看板精華區";
    }
    else
    {
        return XEASY;
    }

    sprintf(buf, "確定要打包 %s %s嗎(y/N)？[N] ", name, item);
    if (vans(buf) == 'y')
    {
        sprintf(buf, "【" BOARDNAME "】%s %s", name, item);
        do_forward(buf, ans);
    }

    return XEASY;
}

#endif  /* #ifdef  HAVE_DOWNLOAD */

int
m_query(
    const char *userid)
{
    int fd, ans, fsize;
    HDR *head, *tail;
    char folder[64];
    struct stat st;

    ans = 0;
    usr_fpath(folder, userid, fn_dir);
    if ((fd = open(folder, O_RDONLY)) >= 0)
    {
        fsize = 0;

        if (!fstat(fd, &st) && (fsize = st.st_size) >= sizeof(HDR) &&
            (head = (HDR *) malloc(fsize)))
        {
            if ((fsize = read(fd, head, fsize)) >= sizeof(HDR))
            {
                tail = (HDR *) ((char *) head + fsize);

                while (--tail >= head)
                {
                    if (!(tail->xmode & MAIL_READ))
                    {
                        ans++;
                        /*break;*/
                    }
                }
            }
            free(head);
        }

        close(fd);
        if (fsize < sizeof(HDR))
            unlink(folder);
    }

    return ans;
}


void
m_biff(
    int userno)
{
    UTMP *utmp, *uceil;

    utmp = ushm->uslot;
    uceil = (UTMP *) ((char *) utmp + ushm->offset);
    do
    {
        if (utmp->userno == userno)
        {
            utmp->ufo |= UFO_BIFF;

#ifdef  BELL_ONCE_ONLY
            return;
#endif
        }
    } while (++utmp <= uceil);
}

#if 0
static int
m_count(void)
{
    int quota;
    unsigned int ulevel;
    struct stat st;

    ulevel = cuser.userlevel;

    /* Thor.980806: 註解: DENYMAIL在經驗值不夠時會在login自動設定 */
    if (ulevel & PERM_DENYMAIL)
    {
        vmsg("您的信箱被鎖了！");
        return 1;
    }

    if (stat(cmbox->dir, &st))
        return 0;

    if (ulevel & (PERM_SYSOP | PERM_ACCOUNTS | PERM_MBOX))
        quota = MAX_BBSMAIL * sizeof(HDR);
    else if (ulevel & PERM_VALID)
        quota = MAX_VALIDMAIL * sizeof(HDR);
    else
        quota = MAX_NOVALIDMAIL * sizeof(HDR);

    if (st.st_size <= quota)
        return 0;

    more(FN_ETC_MAIL_OVER, NULL);
    return 1;
}
#endif  /* #if 0 */

static void
mail_hold(
    const char *fpath,
    const char *rcpt)
{
    char *title, *folder, buf[256];
    HDR mhdr;

    if (vans("是否自存底稿(y/N)？[N] ") != 'y')
        return;

    folder = cmbox->dir;
    hdr_stamp(folder, HDR_LINK, &mhdr, (char *)fpath);

    mhdr.xmode = MAIL_READ | MAIL_HOLD /* | MAIL_NOREPLY */;
    strcpy(mhdr.owner, "[備 忘 錄]");
    strcpy(mhdr.nick, cuser.username);
    title = ve_title;
    if (rcpt)
    {
        sprintf(buf, "<%s> %s", rcpt, title);
        title = buf;
        title[TTLEN] = '\0';
    }
    strcpy(mhdr.title, title);
    rec_add(folder, &mhdr, sizeof(HDR));
}

/* cache.091209: 自動轉寄 */
int
m_setforward(void)
{
    char fpath[64], ip[50];
    FILE *fp;

    usr_fpath(fpath, cuser.userid, FN_FORWARD);
    if ((fp = fopen(fpath, "r")))
    {
        fscanf(fp, "%s", ip);
        fclose(fp);
    }
    else
    {
        ip[0] = '\0';
    }

    vget(B_LINES_REF - 1, 0, "請輸入信件自動轉寄的 E-mail：", ip, 50, GCARRY);

    if (ip[0] && !not_addr(ip) &&
        vans("確定開啟信件轉寄功\能(y/N)？[N] ") == 'y')
    {
        if ( ( fp = fopen(fpath, "w") ) )
        {
            fprintf(fp, "%s", ip);
            fclose(fp);
            pmsg2("設定完成");
            return 0;
        }
    }

    unlink(fpath);
    pmsg2("取消自動轉寄或無效 E-mail");
    return 0;
}

/* cache.100129: 重建信箱索引 */
int
m_setmboxdir(void)
{

    char upath[128], fpath1[128], id[5];

    pmsg2("警告：本功\能只能在信箱已毀損時使用");
    pmsg2("警告：重建索引並不能保證信箱的完整");


    if (vans("確定重建索引(y/N)？[N] ") == 'y')
    {

        vget(B_LINES_REF - 1, 0, "請輸入帳號的第一個英文字母(小寫)：", id, 5, GCARRY);

        usr_fpath(upath, cuser.userid, NULL);

        sprintf(fpath1, BBSHOME "/usr/%s/%s", id, cuser.userid);

        chdir(fpath1);
        proc_runl(BBSHOME "/" BINARY_SUFFIX "redir", "redir", NULL);
        f_mv(".DIR.@", FN_DIR);
        chdir(BBSHOME);

        pmsg2("重建完成");
        return 0;
    }

    pmsg2("取消重建信箱索引");
    return 0;

}

/* ----------------------------------------------------- */
/* in boards/mail 回信給原作者，轉信站亦可               */
/* ----------------------------------------------------- */


int
hdr_reply(
    int row,
    const HDR *hdr)
{
    char *title, *str;

    title = str = ve_title;

    if (hdr)
    {
        sprintf(title, "Re: %s", str_ttl(hdr->title));
        str += TTLEN;
    }
    *str = '\0';

    return vget(row, 0, "標題：", title, TTLEN + 1, GCARRY);
}


/* static inline */ int
mail_external(
    char *addr)
{
    char *str;

    str = strchr(addr, '@');
    if (!str)
        return 0;

        /* Thor.990125: MYHOSTNAME統一放入 str_host */
    if (str_cmp(str_host, str + 1))
        return 1;

    /* 攔截 xyz@domain 或 xyz.bbs@domain */

    *str = '\0';
    if ((str = strchr(addr, '.')))
        *str = '\0';
    return 0;
}

/* cache.091209: 自動轉寄信件*/
/* cuser.userid 將「標題 title、檔案在 fpath」的信件寄給 userid 的外部信箱 */
static void
forward_mail(
    const char *fpath, const char *userid, const char *title)
{
    FILE *fp;
    char ip[80];

    usr_fpath(ip, userid, FN_FORWARD);
    if ( ( fp = fopen(ip, "r") ) )
    {
        fscanf(fp, "%s", ip);
        fclose(fp);

        if (ip[0])
            bsmtp(fpath, title, ip, 0);
    }
}

int
mail_send(
    char *rcpt, const char *title)
{
    HDR mhdr;
    char fpath[80], folder[80], ckforward[80];
    int rc, userno=0;
    ACCT acct;

    int internet_mail;

    if (!(internet_mail = mail_external(rcpt)))
    {
        if ((userno = acct_userno(rcpt)) <= 0)
            return -1;

        if (!title)
            vget(2, 0, "主題：", ve_title, TTLEN, DOECHO);

    }

    utmp_mode(M_SMAIL);
    fpath[0] = '\0';

    curredit = EDIT_MAIL;               /* Thor.1105: 直接指定寫信 */

    if (vedit(fpath, internet_mail ? 2 : 1) == -1)
    {
        unlink(fpath);
        clear();
        return -2;
    }

    if (internet_mail)
    {
        clear();
        prints("信件即將寄給 %s\n標題為：%s\n確定要寄出嗎? (Y/n) [Y]",
            rcpt, title);
        switch (vkey())
        {
        case 'n':
        case 'N':
            outs("N\n信件已取消");
            refresh();
            rc = -2;
            break;

        default:
            outs("Y\n請稍候，信件傳遞中...\n");
            refresh();
            rc = bsmtp(fpath, title, rcpt, 0);
            if (rc < 0)
                vmsg("信件無法寄達");
            mail_hold(fpath, rcpt);
        }
        unlink(fpath);
        return rc;
    }
    usr_fpath(ckforward, rcpt, FN_FORWARD);
    if (access(ckforward, 0))
    {
        usr_fpath(folder, rcpt, fn_dir);
        hdr_stamp(folder, HDR_LINK, &mhdr, fpath);
        strcpy(mhdr.owner, cuser.userid);
        strcpy(mhdr.nick, cuser.username);      /* :chuan: 加入 nick */
        strcpy(mhdr.title, ve_title);
        rc = rec_add(folder, &mhdr, sizeof(mhdr));
        forward_mail(fpath, rcpt, ve_title);
    }
    else
    {
        if (acct_load(&acct, rcpt) >= 0)
        {
            if (!title)
                title = ve_title;
            rc = bsmtp(fpath, title, acct.email, 0);
            if (rc < 0)
                vmsg("信件無法寄達");
            mail_hold(fpath, acct.email);
            unlink(fpath);
            return rc;
        }
        else
            vmsg("信件無法寄達");
                return -1;
    }
    if (!rc) {
        mail_hold(fpath, rcpt);
    }

#if 0
    prints("\n收信人: %s (%s)\n標  題: %s\n", mhdr.owner, mhdr.nick, mhdr.title);
    refresh();
#endif

    unlink(fpath);

    m_biff(userno);

#ifdef EMAIL_PAGE
    /* --------------------------------------------------- */
    /* E-Mail 網路傳呼  by lkchu@dragon2.net               */
    /* --------------------------------------------------- */

    if ((acct_load(&acct, rcpt) >= 0) && (acct.ufo & UFO_MPAGER))
    {
        char *p;

        if ((p = str_str(acct.address, "bbc")) != NULL)  /* 找 BBC 描述 */
            DL_NAME_CALL("emailpage.so", EMailPager)(p + 3, cuser.userid, ve_title);
    }
#endif

    return rc;
}


void
mail_reply(
    HDR *hdr)
{
    int xmode, prefix;
    char buf[80];
    const char *msg;

    if (bbsothermode & OTHERSTAT_EDITING)
    {
        vmsg("你還有檔案還沒編完哦！");
        return;
    }


    if (!(HAS_PERM(PERM_INTERNET)))
        return;

//  if ((hdr->xmode & MAIL_NOREPLY) || m_count())
//      return;

    if ((hdr->xmode & MAIL_NOREPLY) || mail_stat(CHK_MAIL_NOMSG))
    {
        vmsg("你的信箱容量超過上限，請整理！");
        chk_mailstat = 1;
        return;
    }
    else
        chk_mailstat = 0;

    vs_bar("回  信");

    /* find the author */

    strcpy(quote_user, hdr->owner);
    strcpy(quote_nick, hdr->nick);

    /* make the title */

    if (!hdr_reply(3, hdr))
        return;

    prints("\n收信人: %s (%s)\n標  題: %s\n", quote_user, quote_nick, ve_title);

    /* Thor: 為了省一次 rec_put 回信則假設看過內容 */

    xmode = hdr->xmode | MAIL_READ;
    prefix = quote_file[0];

    /* edit, then send the mail */

    switch (mail_send(quote_user, ve_title))
    {
    case -1:
        msg = err_uid;
        break;

    case -2:
        msg = msg_cancel;
        break;

    case -3:  /* Thor.980707: 有此情況嗎 ?*/
        sprintf(buf, "[%s] 無法收信", quote_user);
        msg = buf;
        break;

    default:
        xmode |= MAIL_REPLIED;
        msg = "信已寄出";  /* Thor.980705: mail_send()已經顯示過一次了.. 拿掉嗎? */
        break;
    }

    if (prefix == 'u')  /* user mail 看信時才標 r */
    {
        hdr->xmode = xmode;
    }

    vmsg(msg);
}


void
my_send(
    char *rcpt)
{
    int result;
    const char *msg;

    if (bbsothermode & OTHERSTAT_EDITING)
    {
        vmsg("你還有檔案還沒編完哦！");
        return;
    }



    if (mail_stat(CHK_MAIL_NOMSG))
    {
        vmsg("你的信箱容量超過上限，請整理！");
        chk_mailstat = 1;
        return;
    }
    else
        chk_mailstat = 0;

//  if (m_count())
//      return;

    msg = "信已寄出";

    if ((result = mail_send(rcpt, NULL)))
    {
        switch (result)
        {
        case -1:
            msg = err_uid;
            break;

        case -2:
            msg = msg_cancel;
            break;

        case -3:  /* Thor.980707: 有此情況嗎 ?*/
            msg = "使用者無法收信";
        }
    }
    vmsg(msg);
}


int
m_send(void)
{
    if (HAS_PERM(PERM_DENYMAIL))
        vmsg("您的信箱被鎖了！");
    else
    {
        ACCT muser;
        vs_bar("寄  信");
        if (acct_get(msg_uid, &muser) > 0)
            my_send(muser.userid);
    }
    return 0;
}


int
mail_sysop(void)
{
    int fd;
    if (bbsothermode & OTHERSTAT_EDITING)
    {
        vmsg("你還有檔案還沒編完哦！");
        return 0;
    }


    if ((fd = open(FN_ETC_SYSOP, O_RDONLY)) >= 0)
    {
        int i, j;
        char *ptr, *str;

        struct SYSOPLIST  /* DISKDATA(format) */
        {
            char userid[IDLEN + 1];
            char duty[40];
        }         sysoplist[9]; /* 假設 9 個足矣 */

        j = 0;
        mgets(-1);
        while ((str = mgets(fd)))
        {
            if ((ptr = strchr(str, ':')))
            {
                *ptr = '\0';
                do
                {
                    i = *++ptr;
                } while (i == ' ' || i == '\t');

                if (i)
                {
                    strcpy(sysoplist[j].userid, str);
                    strcpy(sysoplist[j++].duty, ptr);
                }
            }
        }
        close(fd);

        move(11, 0);
        clrtobot();
        prints("%16s   %-16s權責劃分\n", "編號", "站長 ID");
        outsep(b_cols, msg_separator);
        outc('\n');

        for (i = 0; i < j; i++)
        {
            prints("%15d.   \x1b[1;%dm%-16s%s\x1b[m\n",
                i + 1, 31 + i, sysoplist[i].userid, sysoplist[i].duty);
        }
        prints("%-14s0.   \x1b[1;%dm離開\x1b[m", "", 31 + j);

        i = vans("請輸入代碼[0]：") - '0' - 1;
        if (i >= 0 && i < j)
        {
            clear();
            mail_send(sysoplist[i].userid, NULL);
        }
    }
    return 0;
}


/* ----------------------------------------------------- */
/* 群組寄信、回信 : multi_send, multi_reply              */
/* ----------------------------------------------------- */

#ifndef MULTI_MAIL

#define multi_reply(x) mail_reply(x)

#else  /* Thor.981009: 防止愛情幸運信 */

static int
multi_send(
    const char *title)
{
    FILE *fp;
    HDR mhdr;
    char buf[128], fpath[64], *userid;
    int userno, reciper, listing;
    LinkList *wp;

    if (bbsothermode & OTHERSTAT_EDITING)
    {
        vmsg("你還有檔案還沒編完哦！");
        return -1;
    }

    vs_bar(title ? "群組回信" : "群組寄信");

    ll_new();
    listing = reciper = 0;

    /* 回信時讀取 mail list 名單 */

    if (*quote_file)
    {
        ll_add(quote_user);
        reciper = 1;

        fp = fopen(quote_file, "r");
        while (fgets(buf, sizeof(buf), fp))
        {
            if (memcmp(buf, "※ ", 3))
            {
                if (listing)
                    break;
            }
            else
            {
                userid = buf + 3;
                if (listing)
                {
                    strtok(userid, " \n\r");
                    do
                    {
                        if ((userno = acct_userno(userid)) && (userno != cuser.userno) &&
                            !ll_has(userid))
                        {
                            ll_add(userid);
                            reciper++;
                        }
                    } while ((userid = (char *) strtok(NULL, " \n\r")));
                }
                else if (!memcmp(userid, "[通告]", 6))
                    listing = 1;
            }
        }
        fclose(fp);
        ll_out(3, 0, MSG_CC);
    }

    /* 設定 mail list 的名單 */

    reciper = pal_list(reciper);

    /* 開始寄信 */

    move(1, 0);
    clrtobot();

    if (reciper == 1)
    {
        mail_send(ll_head->data, title);
    }
    else if (reciper >= 2 && ve_subject(2, title, "[通告] "))
    {
        usr_fpath(fpath, cuser.userid, FN_NOTE);

        if ((fp = fopen(fpath, "w")))
        {
            fprintf(fp, "※ [通告] 共 %d 人收件", reciper);
            listing = 80;
            wp = ll_head;

            do
            {
                userid = wp->data;
                reciper = strlen(userid) + 1;

                if (listing + reciper > 75)
                {
                    listing = reciper;
                    fprintf(fp, "\n※");
                }
                else
                {
                    listing += reciper;
                }

                fprintf(fp, " %s", userid);
            } while ((wp = wp->next));

            memset(buf, '-', 75);
            buf[75] = '\0';
            fprintf(fp, "\n%s\n\n", buf);
            fclose(fp);
        }

        utmp_mode(M_SMAIL);
        curredit = EDIT_MAIL | EDIT_LIST;

        if (vedit(fpath, true) == -1)
        {
            outs(msg_cancel);
        }
        else
        {
            vs_bar("寄信中...");

            listing = 80;
            wp = ll_head;
            title = ve_title;

            do
            {
                ACCT cacct;
                userid = wp->data;
                acct_load(&cacct, userid);
                reciper = strlen(userid) + 1;
                if (listing + reciper > 75)
                {
                    listing = reciper;
                    outc('\n');
                }
                else
                {
                    listing += reciper;
                    outc(' ');
                }
                outs(userid);

                usr_fpath(buf, userid, fn_dir);
                hdr_stamp(buf, HDR_LINK, &mhdr, fpath);
                strcpy(mhdr.owner, cuser.userid);
                strcpy(mhdr.title, title);
                mhdr.xmode = MAIL_MULTI;
                rec_add(buf, &mhdr, sizeof(HDR));
                forward_mail(fpath, userid, title);
                m_biff(cacct.userno);
            } while ((wp = wp->next));

            mail_hold(fpath, NULL);
        }
        unlink(fpath);

#if 0
        curredit = 0;           /* Thor.1105: 其實是可以不加了 */
#endif
    }
    else
    {
        vmsg(msg_cancel);
        return -1;
    }
    vmsg(NULL);
    return 0;
}


static void
multi_reply(
    HDR *mhdr)
{
    if (!HAS_PERM(PERM_INTERNET))
        return;
    strcpy(quote_user, mhdr->owner);
    strcpy(quote_nick, mhdr->nick);
    if (!multi_send(mhdr->title))
        mhdr->xmode |= (MAIL_REPLIED | MAIL_READ);
}


int
mail_list(void)
{

    if (mail_stat(CHK_MAIL_NOMSG))
    {
        vmsg("你的信箱容量超過上限，請整理！");
        chk_mailstat = 1;
        return 0;
    }
    else
        chk_mailstat = 0;

//  if (m_count())
//      return 0;

    multi_send(NULL);
    return 0;
}

#endif  /* #ifndef MULTI_MAIL */

/* ----------------------------------------------------- */
/* Mail Box call-back routines                           */
/* ----------------------------------------------------- */


static inline int
mbox_attr(
    int type)
{
    if (type & MAIL_DELETE)
        return 'D';

    if (type & MAIL_REPLIED)
        return (type & MAIL_MARKED) ? 'R' : 'r';

    return "+ Mm"[type % 4U];
}


int
tag_char(
    int chrono)
{
    return TagNum && !Tagger(chrono, 0, TAG_NIN) ? '*' : ' ';
}


void
hdr_outs(               /* print HDR's subject */
    const HDR *hdr,
    int cc)
{
    static const char *const type[4] =
    {"Re", "◇", "=>", "◆"};
    static const char *const type_reset[][4] = {
        {"\x1b[m", "\x1b[m", "\x1b[1;33m", "\x1b[1;32m"},
#ifdef HAVE_MENU_LIGHTBAR
        {"\x1b[m", "\x1b[m", "\x1b[0;33m", "\x1b[0;32m"},
#endif
    };
#ifdef  HAVE_DECLARE
    static const char *const type_dec[4] =
    {"\x1b[1;37m", "\x1b[1;37m", "\x1b[1;37m", "\x1b[1;33m"};
#endif
    const char *title, *mark;
    int ch, len;
    UTMP *online;

    if (cc)
    {
#if 0
        if (tag_char(hdr->chrono) == '*')
        {
            outc(' ');
            outc('*');
        }
#ifdef  HAVE_RECOMMEND
        else if (!(hdr->xmode &(POST_LOCK | POST_CANCEL | POST_DELETE | POST_MDELETE)) && hdr->recommend >= (MIN_RECOMMEND) && !(cuser.ufo2 & UFO2_PRH))
        {
            if (hdr->recommend <=30)
                prints("\x1b[36m%02.2d\x1b[m", hdr->recommend);
            else if (hdr->recommend > 30 && hdr->recommend <= 60)
                prints("\x1b[1;33m%02.2d\x1b[m", hdr->recommend);
            else
                prints("\x1b[1;31m%02.2d\x1b[m", hdr->recommend);
        }
#endif
        else
        {
            outc(' ');
            outc(' ');
        }
#endif  /* #if 0 */
        outs("\x1b[m ");
        outs(hdr->date + 3);
        outc(' ');

        mark = hdr->owner;
        len = 13;

        online = utmp_check(mark);  /* 使用者在站上就變色 */
        if (online != NULL)
            outs("\x1b[1;37m");

        while ((ch = (unsigned char) *mark))
        {
            if ((--len == 0) || (ch == '@'))
                ch = '.';
            outc(ch);

            if (ch == '.')
                break;

            mark++;
        }

        while (len--)
        {
            outc(' ');
        }

        if (online != NULL)
            outs("\x1b[m");
    }
    else
    {
        cc = 64;
    }

    title = str_ttl(mark = hdr->title);
    ch = title == mark;
    if (!strcmp(currtitle, title))
        ch += 2;
    outs(type_reset[HAVE_UFO2_CONF(UFO2_MENU_LIGHTBAR)][ch]);
    outs(type[ch]);

    mark = title + cc;
    cc = ' ';

    if (hdr->xmode & POST_LOCK && !HAS_PERM(PERM_SYSOP))
    {
        outs(" 此文章已加密鎖定！\x1b[m");
        outc('\n');
        return;
    }


    {

#ifdef  HAVE_DECLARE            /* Thor.0508: Declaration, 嘗試使某些title更明顯 */
                                /* IID.20191204: Match balanced brackets. */
        int square = -1;        /* -1: No square brackets / no more processing.
                                 * 0: The first character is `[` and has not been output
                                 *       or all `[` are closed.
                                 * N: N non-closed `[`s are meeted. */
        int angle = 0;
        int curly = 0;
        bool dbcs_hi = false;

        if (*title == '[')
        {
            square = 0;
        }
#endif

        do
        {

#ifdef  HAVE_DECLARE
            int *pcnt = NULL;
            switch ((dbcs_hi) ? '\0' : cc)
            {
            case '[':
            case ']':
                if (square >= 0)
                    pcnt = &square;
                break;
            case '<':
            case '>':
                if (angle >= 0)
                    pcnt = &angle;
                break;
            case '{':
            case '}':
                if (curly >= 0)
                    pcnt = &curly;
            }
            if (pcnt && *pcnt < 1 && (square >= 1 || angle >= 1 || curly >= 1))
                pcnt = NULL;

            if (dbcs_hi)
                dbcs_hi = false;
            else if (IS_DBCS_HI(cc))
                dbcs_hi = true;

            switch ((pcnt) ? cc : '\0')
            {
            case '[':
            case '<':
            case '{':
                if (++*pcnt == 1)
                {
                    outs(type_dec[ch]);
                    outc(cc);
                    continue;
                }
                break;

            case ']':
            case '>':
            case '}':
                if (--*pcnt == 0)
                {
                    if (pcnt == &square)
                        square = -1;
                    outc(cc);
                    outs(type_reset[HAVE_UFO2_CONF(UFO2_MENU_LIGHTBAR)][ch]);
                    continue;
                }
            }

#endif  /* #ifdef  HAVE_DECLARE */

            outc(cc);
        } while ((cc = (unsigned char) *title++) && (title < mark));

        if (ch >= 2
#ifdef  HAVE_DECLARE
          || (square >= 1 || angle >= 1 || curly >= 1)
#endif
        )
            outs("\x1b[m");
    }

    outc('\n');
}


static int
mbox_foot(
    XO* xo)
{
    outf(MSG_MAILER);
    return XO_NONE;
}

static inline void
mbox_item(
    int pos,                    /* sequence number */
    const HDR *hdr)
{

#if 0                           /* Thor.0508: 變色看看 */
    prints("%5d %c%", pos, mbox_attr(hdr->xmode));
#endif

    int xmode = hdr->xmode;
    prints("%6d%c%s%c%s",
        pos, tag_char(hdr->chrono),
        (xmode & MAIL_DELETE) ? (HAVE_UFO2_CONF(UFO2_MENU_LIGHTBAR) ? "\x1b[1;37;41m" : "\x1b[1;5;37;41m")
            : (xmode & MAIL_MARKED) ? (HAVE_UFO2_CONF(UFO2_MENU_LIGHTBAR) ? "\x1b[36m" : "\x1b[1;36m")
            : "",
        mbox_attr(hdr->xmode),
        (xmode & (MAIL_DELETE | MAIL_MARKED)) ? "\x1b[m" : "");

    hdr_outs(hdr, d_cols + 47);
}

static int
mbox_cur(
    XO *xo)
{
    const HDR *const mhdr = (const HDR *) xo_pool_base + xo->pos;
    move(3 + xo->pos - xo->top, 0);
    mbox_item(xo->pos + 1, mhdr);
    return XO_NONE;
}


static int
mbox_body(
    XO *xo)
{
    const HDR *mhdr;
    int num, max, tail;

    max = xo->max;

    if (max <= 0)
    {
        vmsg("您沒有來信");
        return XO_QUIT;
    }

    num = xo->top;
    mhdr = (const HDR *) xo_pool_base + num;
    tail = num + XO_TALL;
    max = BMIN(max, tail);

    move(3, 0);
    do
    {
        mbox_item(++num, mhdr++);
    } while (num < max);
    clrtobot();

    return mbox_foot(xo);
}


static int
mbox_head(
    XO *xo)
{
    clear_notification();
    vs_head("郵件選單", str_site);

    prints(NECKMAIL, d_cols, "");

    return mbox_body(xo);
}


static int
mbox_load(
    XO *xo)
{
    xo_load(xo, sizeof(HDR));
    return mbox_body(xo);
}


static int
mbox_init(
    XO *xo)
{
    xo_load(xo, sizeof(HDR));
    return mbox_head(xo);
}


static int
mbox_delete(
    XO *xo)
{
    int pos, xmode;
    HDR *hdr;
    char *dir;
#ifndef HAVE_MAILUNDELETE
    char fpath[64];
#endif

    pos = xo->pos;
    hdr = (HDR *) xo_pool_base + pos;

    xmode = hdr->xmode;
    if ((xmode & (MAIL_MARKED | MAIL_DELETE)) == MAIL_MARKED)
        return XO_NONE; /* Thor.980901: mark後若被'D'起來, 則一樣可以delete,
                                        只有 MARK & no delete才會無效 */

    if (vans(msg_del_ny) == 'y')
    {
        dir = xo->dir;
        currchrono = hdr->chrono;
#ifdef HAVE_MAILUNDELETE
        hdr->xmode |= POST_DELETE;
        if (!rec_put(dir, hdr, sizeof(HDR), pos))
        {
            return XO_LOAD;
        }
#else
        if (!rec_del(dir, sizeof(HDR), pos, cmpchrono, NULL))
        {
            hdr_fpath(fpath, dir, hdr);
            unlink(fpath);
            return XO_LOAD;
        }
#endif
    }
    return XO_FOOT;
}

static int
mbox_forward(
    XO *xo)
{
    ACCT muser;
    int pos;
    const HDR *hdr;
    if (bbsothermode & OTHERSTAT_EDITING)
    {
        vmsg("你還有檔案還沒編完哦！");
        return XO_FOOT;
    }


    if (mail_stat(CHK_MAIL_NOMSG))
    {
        vmsg("你的信箱容量超過上限，請整理！");
        chk_mailstat = 1;
        return XO_HEAD;
    }
    else
        chk_mailstat = 0;

    if (acct_get("轉達信件給：", &muser) > 0)
    {
        pos = xo->pos;
        hdr = (const HDR *) xo_pool_base + pos;

        strcpy(quote_user, hdr->owner);
        hdr_fpath(quote_file, xo->dir, hdr);
        sprintf(ve_title, "%.64s (fwd)", hdr->title);
        move(1, 0);
        clrtobot();
        prints("轉信給: %s (%s)\n標  題: %s\n", muser.userid, muser.username,
            ve_title);

        switch (mail_send(muser.userid, ve_title))
        {
        case -1:
            outs(err_uid);
            break;

        case -2:
            outs(msg_cancel);
            break;

        case -3:  /* Thor.980707: 有此情況嗎 ?*/
            prints("使用者 [%s] 無法收信", muser.userid);
        }
        *quote_file = '\0';
        vmsg(NULL);
    }
    return XO_HEAD;
}


static int
mbox_browse(
    XO *xo)
{
    HDR *mhdr, hdr;
    int pos, xmode, nmode, mode;
    char *dir, *fpath;

    dir = xo->dir;
    pos = xo->pos;
    fpath = quote_file;
    mhdr = (HDR *) xo_pool_base + pos;
    memcpy(&hdr, mhdr, sizeof(HDR));
    mhdr = &hdr;
    strcpy(currtitle, str_ttl(mhdr->title));
    xmode = mhdr->xmode;

#ifdef  HAVE_CHK_MAILSIZE
 if (!(mhdr->xmode & MAIL_READ) && chk_mailstat == 1)
 {
    if (mail_stat(CHK_MAIL_NOMSG))
    {
        vmsg("您的信箱已超出容量，無法閱\讀新信件，請清理您的信箱！");
        return XO_FOOT;
    }
    else
        chk_mailstat = 0;
 }
#endif

    hdr_fpath(fpath, dir, mhdr);

    if ((mode = more(fpath, MSG_MAILER)) == -1)
    {
        *fpath = '\0';
        return XO_INIT;
    }
    else if (mode == -2)
    {
        mhdr->xmode |= MAIL_READ;
        rec_put(dir, mhdr, sizeof(HDR), pos);
        *fpath = '\0';
        return XO_INIT;
        /*nmode = vkey();*/
    }
    else if (mode > 0)
        nmode = mode;
    else
        nmode = vkey();

    if (nmode == 'd')
    {
        if (mbox_delete(xo) != XO_FOOT)
        {
            *fpath = '\0';
            return XO_INIT;
        }
    }
    else if (nmode == 'y' || nmode == 'r')
    {
        if (!(xmode & MAIL_NOREPLY) && !mail_stat(CHK_MAIL_NOMSG) /* m_count() */)
        {
            if ((nmode == 'y') && (xmode & MAIL_MULTI))
                multi_reply(mhdr);
            else
                mail_reply(mhdr);
        }
    }
    else if (nmode == 'm')
    {
        mhdr->xmode |= MAIL_MARKED;
    }
    else if (nmode == 'x')
    {
        mbox_forward(xo);
    }

    nmode = mhdr->xmode | MAIL_READ;

    if (xmode != nmode)
    {
        mhdr->xmode = nmode;
        rec_put(dir, mhdr, sizeof(HDR), pos);
    }
    *fpath = '\0';

    return XO_INIT;
}

static int
mbox_reply(
    XO *xo)
{
    int pos, xmode;
    HDR *mhdr, hdr;

//  if (m_count())
//      return XO_HEAD;

    if (mail_stat(CHK_MAIL_NOMSG))
    {
        vmsg("你的信箱容量超過上限，請整理！");
        chk_mailstat = 1;
        return XO_HEAD;
    }
    else
        chk_mailstat = 0;

    pos = xo->pos;
    mhdr = (HDR *) xo_pool_base + pos;
    memcpy(&hdr, mhdr, sizeof(HDR));
    mhdr = &hdr;

    xmode = mhdr->xmode;
    if (xmode & MAIL_NOREPLY)
        return XO_NONE;
    hdr_fpath(quote_file, xo->dir, mhdr);
    if (xmode & MAIL_MULTI)
        multi_reply(mhdr);
    else
        mail_reply(mhdr);
    *quote_file = '\0';

    if (mhdr->xmode != xmode)
        rec_put(xo->dir, mhdr, sizeof(HDR), pos);

    return XO_INIT;
}


static int
mbox_mark(
    XO *xo)
{
    HDR *mhdr;
    int cur, pos;

    pos = xo->pos;
    cur = pos - xo->top;
    mhdr = (HDR *) xo_pool_base + pos;

    mhdr->xmode ^= MAIL_MARKED;
    rec_put(xo->dir, mhdr, sizeof(HDR), pos);
    return XO_CUR;
}


static int
mbox_tag(
    XO *xo)
{
    const HDR *hdr;
    int tag, pos, cur;

    pos = xo->pos;
    cur = pos - xo->top;
    hdr = (const HDR *) xo_pool_base + pos;

    if ((tag = Tagger(hdr->chrono, pos, TAG_TOGGLE)))
    {
        return XO_CUR + 1;
    }

    /* return XO_NONE; */
    return XO_MOVE + XO_REL + 1; /* lkchu.981201: 跳至下一項 */
}


int
mbox_send(
    XO *xo)
{
    m_send();
    return XO_HEAD;
}

static int
mbox_help(
    XO *xo)
{
    film_out(FILM_MAIL, -1);
    return XO_HEAD;
}

int
mail_stat(
    int mode)
{
    int limit_e, total_e;
    int limit_k, total_k;
    char buf[128];

    limit_e = cuser.userlevel;
    if (limit_e & (PERM_SYSOP | PERM_MBOX))
        limit_e = MAX_BBSMAIL;
    else if (mode & CHK_MAIL_VALID)
        limit_e = MAX_VALIDMAIL;
    else
        limit_e = limit_e & PERM_VALID ? MAX_VALIDMAIL : MAX_NOVALIDMAIL;

    limit_k = cuser.userlevel;
    if (limit_k & (PERM_SYSOP | PERM_MBOX))
        limit_k = MAX_MAIL_SIZE;
    else if (mode & CHK_MAIL_VALID)
        limit_k = MAIL_SIZE;
    else
        limit_k = limit_k & PERM_VALID ? MAIL_SIZE : MAIL_SIZE_NO;

    usr_fpath(buf, cuser.userid, fn_dir);
    total_e = rec_num(buf, sizeof(HDR));
    total_k = m_total_size() / 1024;
    sprintf(buf, "信件數 %d/%d 封，容量大小 %d/%d K！", total_e, limit_e, total_k, limit_k);
    if (mode & CHK_MAIL_NORMAL)
        vmsg(buf);
    return (total_k>=limit_k)||(total_e>limit_e);
}

static int
mbox_stat(
    XO *xo)
{
    mail_stat(CHK_MAIL_NORMAL);
    return XO_HEAD;
}

static int
mbox_edit(
    XO *xo)
{
    HDR *hdr;
    char fpath[128];
    if (bbsothermode & OTHERSTAT_EDITING)
    {
        vmsg("你還有檔案還沒編完哦！");
        return XO_FOOT;
    }

    hdr = (HDR *) xo_pool_base + xo->pos;
    hdr_fpath(fpath, xo->dir, hdr);
    if (HAS_PERM(PERM_SYSOP))
    {
        vedit(fpath, false);
        return XO_HEAD;
    }
    return XO_NONE;
}

static int
mbox_size(
    XO *xo)
{
    const HDR *hdr;
    char *dir, fpath[80], buf[128];
    struct stat st;

    dir = xo->dir;
    hdr = (const HDR *) xo_pool_base + xo->pos;
    hdr_fpath(fpath, dir, hdr);

    if (HAS_PERM(PERM_SYSOP))
    {
        move(12, 0);
        clrtobot();
        outs("\nDir : ");
        outs(dir);
        outs("\nName: ");
        outs(hdr->xname);
        outs("\nFile: ");
        outs(fpath);

        if (!stat(fpath, &st))
            prints("\nTime: %s\nSize: %lld", Ctime(&st.st_mtime), (long long)st.st_size);
        vmsg(NULL);
    }
    else
    {
        stat(fpath, &st);
        sprintf(buf, "容量大小: %lld K", (long long)st.st_size/1024);
        vmsg(buf);
    }


    return XO_HEAD;
}

#ifdef  HAVE_MAIL_FIX
static int
mbox_title(
    XO *xo)
{
    HDR *hdr, mhdr;

    if (!supervisor)
        return XO_NONE;


    hdr = (HDR *) xo_pool_base + xo->pos;
    mhdr = *hdr;

    vget(B_LINES_REF, 0, "標題：", mhdr.title, sizeof(mhdr.title), GCARRY);
    vget(B_LINES_REF, 0, "作者：", mhdr.owner, 74, GCARRY);
    vget(B_LINES_REF, 0, "日期：", mhdr.date, sizeof(mhdr.date), GCARRY);
    vget(B_LINES_REF, 0, "檔名：", mhdr.xname, sizeof(mhdr.date), GCARRY);
    if (mhdr.xid > 1000)
        mhdr.xid = 0;
    if (vans(msg_sure_ny) == 'y' &&
        memcmp(hdr, &mhdr, sizeof(HDR)))
    {
        *hdr = mhdr;
        rec_put(xo->dir, hdr, sizeof(HDR), xo->pos);
        return XO_INIT;
    }

    return XO_FOOT;
}
#endif /* HAVE_MAIL_FIX */


#ifdef HAVE_MAILUNDELETE

static int
mbox_undelete(
    XO *xo)
{
    HDR *hdr;

    hdr = (HDR *) xo_pool_base + xo->pos;

    hdr->xmode &= ~POST_DELETE;

    if (!rec_put(xo->dir, hdr, sizeof(HDR), xo->pos))
        return XO_INIT;
    return XO_NONE;
}

static int
mbox_clean(
    XO *xo)
{
    if (vans("\x1b[1;5;41;33m警告：\x1b[m清除之後不能救回。確定要清除嗎？(y/N)") == 'y')
    {
        hdr_prune(xo->dir, 0, 0, 3);
        return XO_INIT;
    }
    return XO_HEAD;
}

int
mbox_check(void)
{
    HDR hdr;
    int fd, total;
    char fpath[256];

    usr_fpath(fpath, cuser.userid, FN_DIR);
    total = 0;
    if ((fd = open(fpath, O_RDONLY)) >= 0)
    {
        while (read(fd, &hdr, sizeof(HDR)) == sizeof(HDR))
        {
            if ( hdr.xmode & POST_DELETE )
                total++;
        }
        close(fd);
    }
    return total;
}
#endif  /* HAVE_MAILUNDELETE */

#ifdef  HAVE_MAILGEM
static int
mbox_gem(
    XO *xo)
{
    DL_HOTSWAP_SCOPE void (*mgp)(void) = NULL;
    if (!HAS_PERM(PERM_MBOX))
        return XO_NONE;
    if (!mgp)
    {
        mgp = DL_NAME_GET("mailgem.so", mailgem_main);
        if (mgp)
            (*mgp)();
        else
            vmsg("動態連結失敗，請聯絡系統管理員！");
    }
    else
        (*mgp)();
    return XO_INIT;
}
#endif

static int mbox_sysop(XO *xo);
static int mbox_other(XO *xo);

static KeyFuncList mbox_cb =
{
    {XO_INIT, {mbox_init}},
    {XO_LOAD, {mbox_load}},
    {XO_HEAD, {mbox_head}},
    {XO_BODY, {mbox_body}},
    {XO_FOOT, {mbox_foot}},
    {XO_CUR, {mbox_cur}},

#ifdef  HAVE_MAIL_FIX
    {'T', {mbox_title}},
#endif
    {'r', {mbox_browse}},
    {'E', {mbox_edit}},
    {'s', {mbox_send}},
    {'d', {mbox_delete}},
    {Ctrl('X'), {mbox_forward}},
    {'m', {mbox_mark}},
#ifdef  HAVE_MAILGEM
    {'z', {mbox_gem}},
#endif
    {'R', {mbox_reply}},
    {'y', {mbox_reply}},
    {'c', {mbox_stat}},
#ifdef HAVE_MAILUNDELETE
    {'U', {mbox_clean}},
    {'u', {mbox_undelete}},
#endif

    {KEY_TAB, {mbox_sysop}},
    {'I', {mbox_other}},
    {'t', {mbox_tag}},
    {'S', {mbox_size}},
    {'D', {xo_delete}},

    {Ctrl('Q'), {xo_uquery}},
    {'X', {xo_usetup}},
    {'x', {post_cross}},

    {'h', {mbox_help}}
};


/*by visor*/
static int
mbox_sysop(
    XO *xo)
{
    if (/*(xo == cmbox) &&*/ (HAS_PERM(PERM_SYSOP)))
    {
        XO *xx, *last;

        last = xz[XZ_MBOX - XO_ZONE].xo;  /* record */

        xz[XZ_MBOX - XO_ZONE].xo = xx = xo_new("usr/s/sysop/.DIR");
        xx->cb = mbox_cb;
        xx->recsiz = sizeof(HDR);
        xx->pos = 0;
        xover(XZ_MBOX);
        free(xx);

        xz[XZ_MBOX - XO_ZONE].xo = last;  /* restore */
        return XO_INIT;
    }

    return XO_NONE;
}

static int
mbox_other(
    XO *xo)
{

    ACCT acct;
    char path[80];

    if (!supervisor)
        return XO_NONE;

    while (acct_get(msg_uid, &acct) > 0)
    {
        XO *xx, *last;

        last = xz[XZ_MBOX - XO_ZONE].xo;  /* record */

        //str_lower(id, acct.userid);
        //sprintf(path, "usr/%c/%s/.DIR", *id, id);

        usr_fpath(path, acct.userid, fn_dir);
        usr_fpath(cmbox->dir, acct.userid, fn_dir);

        xz[XZ_MBOX - XO_ZONE].xo = xx = xo_new(path);
        xx->cb = mbox_cb;
        xx->recsiz = sizeof(HDR);
        xx->pos = 0;
        xover(XZ_MBOX);
        free(xx);

        usr_fpath(cmbox->dir, cuser.userid, fn_dir);

        xz[XZ_MBOX - XO_ZONE].xo = last;  /* restore */
        return XO_INIT;

    }
    return XO_HEAD;
}

void
mbox_main(void)
{
    cmbox->pos = XO_TAIL;
    usr_fpath(cmbox->dir, cuser.userid, fn_dir);
    xz[XZ_MBOX - XO_ZONE].xo = cmbox;
    cmbox->cb = mbox_cb;
    cmbox->recsiz = sizeof(HDR);
}

