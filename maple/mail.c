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


#define MSG_CC "\x1b[32m[�s�զW��]\x1b[m\n"


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
                outs("�� �C��(C)�~�� (Q)���� ? [C]");
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
static int
bsmtp_batch(
    const char *fpath, const char *title, const char *rcpt,
    int method)
{
    char buf[80];
    time_t chrono;
    MailQueue mqueue;

    chrono = time(NULL);

    /* ���~�d�I */

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

    cuser.numemail++;           /* �O���ϥΪ̦@�H�X�X�� Internet E-mail */
    return chrono;
}
#endif

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
    char *str, buf[512], from[80], subject[80], msgid[80], keyfile[80], valid[10], boundary[256];
    char fname[256];
#ifdef HAVE_SIGNED_MAIL
    const char *signature = NULL;
    char prikey[PLAINPASSSIZE];
    union {
        char str[PLAINPASSSIZE];
        struct {
            unsigned int hash, hash2;
        } val;
    } sign = {{0}};

    *prikey = prikey[PLAINPASSSIZE-1] = sign.str[PLAINPASSSIZE-1] = '\0'; /* Thor.990413:����: �r�굲�� */
#endif

#ifdef BATCH_SMTP
    if (!(method & MQ_ATTACH))
        return bsmtp_batch(fpath, title, rcpt, method);
#endif

    cuser.numemail++;           /* �O���ϥΪ̦@�H�X�X�� Internet E-mail */
    chrono = time(&stamp);

    /* --------------------------------------------------- */
    /* �����{�ҫH��                                        */
    /* --------------------------------------------------- */

    if (method == MQ_JUSTIFY)
    {
        fpath = FN_ETC_VALID;
        /* Thor.990125: MYHOSTNAME�Τ@��J str_host */
        sprintf(from, "bbsreg@%s", str_host);
        archiv32(str_hash(rcpt, chrono), buf);
        /* sprintf(subject, "[MapleBBS]To %s(%s) [VALID]", cuser.userid, buf); */
        /* Thor.981012: �����b config.h �޲z */
        sprintf(subject, TAG_VALID"%s(%s) [VALID]", cuser.userid, buf);
        title = subject;

        usr_fpath(keyfile, cuser.userid, FN_REGKEY);
        if ( ( fp = fopen(keyfile, "w") ) )
        {
            fprintf(fp, "%s\n", buf);
            fclose(fp);
        }
        str_scpy(valid, buf, sizeof(valid));

    }
    else
    {
        if (method & MQ_ATTACH)
        {
            const struct tm *const xtime = localtime(&chrono);
            sprintf(fname, "mail_%04d%02d%02d.tgz", xtime->tm_year + 1900, xtime->tm_mon + 1, xtime->tm_mday);
        }

        /* Thor.990125: MYHOSTNAME�Τ@��J str_host */
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

        if (method & MQ_ATTACH)
            sprintf(boundary, "----=_NextPart_%s", msgid);

        move(b_lines, 0);
        clrtoeol();

        if (method)
        {
            prints("�� �H�H�� %s \x1b[5m...\x1b[m", rcpt);
            refresh();
        }
        else
        {
            prints("�� �H�H�� \x1b[5m...\x1b[m");
            refresh();
        }

        sleep(1);                       /* wait for mail server response */

        fr = fdopen(sock, "r");
        fw = fdopen(sock, "w");

        fgets(buf, sizeof(buf), fr);
        if (strncmp(buf, "220", 3))
            goto smtp_error;
        while (buf[3] == '-') /* maniac.bbs@WMStar.twbbs.org 2000.04.18 */
            fgets(buf, sizeof(buf), fr);

#define SMTP_TRY_SENDF(...) do { \
    fprintf(fw, __VA_ARGS__); \
    fflush(fw); \
    do \
    { \
        fgets(buf, sizeof(buf), fr); \
        if (strncmp(buf, "250", 3)) \
            goto smtp_error; \
    } while (buf[3] == '-'); \
} while (0)

        /* Thor.990125: MYHOSTNAME�Τ@��J str_host */
        SMTP_TRY_SENDF("HELO %s\r\n", str_host);
        SMTP_TRY_SENDF("MAIL FROM:<%s>\r\n", from);
        SMTP_TRY_SENDF("RCPT TO:<%s>\r\n", rcpt);
/*      SMTP_TRY_SENDF("DATA\r\n", rcpt);*/ /* statue.000713 */
        SMTP_TRY_SENDF("DATA\r\n");

#undef SMTP_TRY_SENDF

        /* ------------------------------------------------- */
        /* begin of mail header                              */
        /* ------------------------------------------------- */

        /* Thor.990125: ���i�઺�� RFC822 & sendmail���@�k, �K�o�O�H����:p */
        fprintf(fw, "From: %s\r\nTo: %s\r\nSubject: %s\r\nX-Sender: %s (%s)\r\n"
            "Date: %s\r\nMessage-Id: <%s@%s>\r\n"
            "X-Disclaimer: [%s] �糧�H���e�����t�d\r\n",
            from, rcpt, title, cuser.userid, cuser.username,
            Atime(&stamp), msgid, str_host,
            str_site);
        if (!(method & MQ_ATTACH))
            fputs("\r\n", fw);

        if (method & MQ_JUSTIFY)        /* �����{�ҫH�� */
        {
            fprintf(fw, " ID: %s (%s)  E-mail: %s\r\n\r\n",
                cuser.userid, cuser.username, rcpt);
        }

        if (method & MQ_ATTACH)
        {
            fprintf(fw, "MIME-Version: 1.0\r\nContent-Type: multipart/mixed;\r\n"
                    "\tboundary=\"%s\"\r\n\r\n", boundary);

            fprintf(fw, "This is a multi-part message in MIME format.\r\n");
            fprintf(fw, "--%s\r\nContent-Type: text/plain;\r\n\tcharset=\"big5\"\r\n"
                    "Content-Transfer-Encoding: 8bit\r\n\r\n����W�١G%s\r\n", boundary, fname);

            fprintf(fw, "--%s\r\nContent-Type: application/x-compressed;\r\n\tname=\"%s\"\r\n"
                    "Content-Transfer-Encoding: base64\r\nContent-Disposition: attachment;\r\n"
                    "\tfilename=\"%s\"\r\n\r\n", boundary, fname, fname);
        }

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
        if (method & MQ_ATTACH)
            fprintf(fw, "--%s--\r\n", boundary);

#ifdef HAVE_SIGNED_MAIL
        if (!(method & MQ_JUSTIFY) && !rec_get(PRIVATE_KEY, prikey, PLAINPASSSIZE-1, 0))
        /* Thor.990413: ���F�{�Ҩ�~, ��L�H�󳣭n�[sign */
        {
            /* Thor.990413: buf�Τ���F, �ɨӥΥ� :P */
            sprintf(buf, "%s -> %s", cuser.userid, rcpt);
            sign.val.hash = str_hash(buf, stamp);
            sign.val.hash2 = str_hash2(buf, sign.val.hash);
            str_xor(sign.str, prikey);
            explicit_zero_bytes(prikey, sizeof(prikey));
            /* Thor.990413: ���[()����, �ɶ����ťշ|�Q�Y��(���Ү�) */
            fprintf(fw, "\x1b[1;32m�� X-Info: \x1b[33m%s\x1b[m\r\n\x1b[1;32m�� X-Sign: \x1b[36m%s%s \x1b[37m(%s)\x1b[m\r\n",
                buf, msgid, signature = gensignature(sign.str), Btime(&stamp));
        }
#endif
        fputs("\r\n.\r\n", fw);
        fflush(fw);

        fgets(buf, sizeof(buf), fr);
        if (strncmp(buf, "250", 3))
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
    /* �O���H�H                                            */
    /* --------------------------------------------------- */

    sprintf(buf, "%s%-*s %c> %s %s %s\n\t%s\n\t%s\n", Btime(&stamp), IDLEN, cuser.userid,
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

#ifdef HAVE_SIGNED_MAIL
/* Thor.990413: �������ҥ\�� */

#define SIGNATURESIZE (PASSSIZE-1-3 + PASSHASHSIZE-1-1 + 1)
#define SIGNATURELEN  SIGNATURESIZE /* Alias for backward compatibility */
int
m_verify(void)
{
    time_t chrono;
    char info[79], *p;
    char sign[79], *q;
    char buf[160];

    char prikey[PLAINPASSSIZE];
    union {
        char str[PLAINPASSSIZE];
        struct {
            unsigned int hash, hash2;
        } val;
    } s = {{0}};

    prikey[PLAINPASSSIZE-1] = s.str[PLAINPASSSIZE-1] = '\0'; /* Thor.990413:����: �r�굲�� */

    if (rec_get(PRIVATE_KEY, prikey, PLAINPASSSIZE-1, 0))
    {
        zmsg("���t�ΨõL�q�lñ���A�Ь�SYSOP");
        return XEASY;
    }

    move(13, 0);
    clrtobot();
    move(15, 0);
    outs("�Ш̧ǿ�J�H����� X-Info X-Sign �H�i������");

    if (!vget(17, 0, ":", info, sizeof info, DOECHO) ||
        !vget(18, 0, ":", sign, sizeof sign, DOECHO))
        return 0;

    str_rtrim(info); /* Thor: �h����, for ptelnet�۰ʥ[�ť� */
    str_rtrim(sign);

    if (!strncmp("�� X-Info: ", p = info, 11))
        p += 11;
    while (*p == ' ') p++; /* Thor: �h�e�Y */

    if (!strncmp("�� X-Sign: ", q = sign, 11))
        q += 11;
    while (*q == ' ') q++;

    if (strlen(q) < 7 + PASSSIZE-1 || (isalnum(q[7+PASSSIZE-1]) && strlen(q) < 7 + SIGNATURESIZE-1))
    {
        vmsg("�q�lñ�����~");
        return 0;
    }

    str_scpy(s.str + 1, q, 8);  /* Thor: �ȭɤ@�U s.str */
    chrono = chrono32(s.str); /* prefix 1 char */

    q += 7; /* real sign */
    if (isalnum(q[PASSSIZE-1]))
        q[SIGNATURESIZE-1] = 0; /* �ɧ�0 */
    else
        q[PASSSIZE-1] = 0; /* �ɧ�0 */

    s.val.hash = str_hash(p, chrono);
    s.val.hash2 = str_hash2(p, s.val.hash);
    str_xor(s.str, prikey);
    explicit_zero_bytes(prikey, sizeof(prikey));

    sprintf(buf, "(%s)", Btime(&chrono));

    if (chksignature(q, s.str) || strcmp(q + ((isalnum(q[PASSSIZE-1])) ? SIGNATURESIZE : PASSSIZE), buf))
    {
        /* Thor.990413: log usage */
        sprintf(buf, "%s@%s - XInfo:%s", rusername, fromhost, p);
        blog("VRFY", buf);
        /* Thor: fake sign */
        move(20, 25);
        outs("\x1b[41;37;5m *�`�N* ���ҿ��~! \x1b[m");
        vmsg("���H�ëD�ѥ����ҵo�A�Ьd��");
        return 0;
    }

    sprintf(buf, "%s@%s + XInfo:%s", rusername, fromhost, p);
    blog("VRFY", buf);

    vmsg("���H�ѥ����ҵo�X");
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
        /* Thor.981205: �� fcntl ���Nflock, POSIX�зǥΪk */
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
        /* Thor.981205: �� fcntl ���Nflock, POSIX�зǥΪk */
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
        vmsg("�ϥ� BBS �H�c�@���{�ҫH�c/�ϥε��U��{�Ҫ̵L�k���]�I");
        return;
    }

    strcpy(addr, cuser.email);

/*
 *  if (!vget(B_LINES_REF, 0, "�п�J��H�a�}�G", addr, 60, GCARRY))
 *      return;
 *
 *  if (not_addr(addr))
 *  {
 *      zmsg("���X�檺 E-mail address");
 *      return;
 *  }
 *
 *  sprintf(fpath, "�T�w�H�� [%s] ��(y/N)�H[N] ", addr);
 */

    sprintf(fpath, "�T�w�H�� [%s] ��(y/N)�H[N] ", cuser.email);
    if (vans(fpath) != 'y')
        return;

    userid = strchr(addr, '@') + 1;
    if (dns_smtp(userid) >= 0)   /* itoc.����: ���M bsmtp() �]�|���A���O�o������A�H�K���Y���~���D�O�L�Ī��u�@���a�} */
    {
        userid = cuser.userid;

        if (mode == '1')                /* �ӤH�H�� */
        {
            /* usr_fpath(fpath, userid, "@"); */
            /* �]�� .DIR ���b @/ �̡A�n�@�_���] */
            usr_fpath(cmd, userid, fn_dir);
            usr_fpath(fpath, userid, "@");
            strcat(fpath, " ");
            strcat(fpath, cmd);
        }
        else if (mode == '2')   /* �ݪO�峹 */
        {
            brd_fpath(fpath, currboard, NULL);
        }
        else /* if (mode == '3') */     /* �ݪO��ذ� */
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
        rc = bsmtp(fpath, title, addr, MQ_ATTACH);
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
            vmsg("�H�w�H�X");
            return;
        }
    }

    vmsg("�H��L�k�H�F");
}


int
m_zip(void)                     /* itoc.010228: ���]��� */
{
    int ans;
    const char *name, *item;
    char buf[80];

    ans = vans("���]��� 1)�ӤH�H�� 2)�ݪO�峹 3)�ݪO��ذ� ����U�H�c [Q] ");

    if (ans == '1')
    {
        name = cuser.userid;
        item = "�ӤH�H��";
    }
    else if (ans == '2' || ans == '3')
    {
        /* itoc.����: ���w�u�ॴ�]�ثe�\Ū���ݪO�A����\Ū���K�ݪO���H�N���ॴ�]�ӪO/��ذ� */
        /* itoc.020612: ���F���� POST_RESTRICT/GEM_RESTRICT ���峹�~�y�A�D�O�D�N���ॴ�] */

        if (currbno < 0)
        {
            vmsg("�Х��i�J�z�n���]���ݪO�A�A�Ӧ����]");
            return XEASY;
        }

        if ((ans == '2' && !(bbstate & STAT_BOARD)) || (ans == '3' && !(bbstate & STAT_BOARD)))
                                    /* tmp not STAT_BM */
        {
            vmsg("�u���O�D�~�ॴ�]�ݪO�峹�άݪO��ذ�");
            return XEASY;
        }

        if (!(strstr(currboard, "P_")))
        {
            vmsg("�D�ӤH�O�Ȥ����ѥ��]�A�ȡA�S��ӤH�M�O�Y�ݥ��]�Ь�����");
            return XEASY;
        }

        name = currboard;
        item = (ans == '2') ? "�ݪO�峹" : "�ݪO��ذ�";
    }
    else
    {
        return XEASY;
    }

    sprintf(buf, "�T�w�n���] %s %s��(y/N)�H[N] ", name, item);
    if (vans(buf) == 'y')
    {
        sprintf(buf, "�i" BOARDNAME "�j%s %s", name, item);
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
    uceil = utmp + ushm->ubackidx;
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

    /* Thor.980806: ����: DENYMAIL�b�g��Ȥ����ɷ|�blogin�۰ʳ]�w */
    if (ulevel & PERM_DENYMAIL)
    {
        vmsg("�z���H�c�Q��F�I");
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

    if (vans("�O�_�ۦs���Z(y/N)�H[N] ") != 'y')
        return;

    folder = cmbox->dir;
    hdr_stamp(folder, HDR_LINK, &mhdr, (char *)fpath);

    mhdr.xmode = MAIL_READ | MAIL_HOLD /* | MAIL_NOREPLY */;
    strcpy(mhdr.owner, "[�� �� ��]");
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

/* cache.091209: �۰���H */
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

    vget(B_LINES_REF - 1, 0, "�п�J�H��۰���H�� E-mail�G", ip, 50, GCARRY);

    if (ip[0] && !not_addr(ip) &&
        vans("�T�w�}�ҫH����H�\\��(y/N)�H[N] ") == 'y')
    {
        if ( ( fp = fopen(fpath, "w") ) )
        {
            fprintf(fp, "%s", ip);
            fclose(fp);
            pmsg2("�]�w����");
            return 0;
        }
    }

    unlink(fpath);
    pmsg2("�����۰���H�εL�� E-mail");
    return 0;
}

/* cache.100129: ���ثH�c���� */
int
m_setmboxdir(void)
{

    char upath[128], fpath1[128], id[5];

    pmsg2("ĵ�i�G���\\��u��b�H�c�w���l�ɨϥ�");
    pmsg2("ĵ�i�G���د��ިä���O�ҫH�c������");


    if (vans("�T�w���د���(y/N)�H[N] ") == 'y')
    {

        vget(B_LINES_REF - 1, 0, "�п�J�b�����Ĥ@�ӭ^��r��(�p�g)�G", id, 5, GCARRY);

        usr_fpath(upath, cuser.userid, NULL);

        sprintf(fpath1, BBSHOME "/usr/%s/%s", id, cuser.userid);

        chdir(fpath1);
        PROC_CMD(BBSHOME "/" BINARY_SUFFIX "redir", NULL);
        f_mv(".DIR.@", FN_DIR);
        chdir(BBSHOME);

        pmsg2("���ا���");
        return 0;
    }

    pmsg2("�������ثH�c����");
    return 0;

}

/* ----------------------------------------------------- */
/* in boards/mail �^�H����@�̡A��H����i               */
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

    return vget(row, 0, "���D�G", title, TTLEN + 1, GCARRY);
}


/* static inline */ int
mail_external(
    char *addr)
{
    char *str;

    str = strchr(addr, '@');
    if (!str)
        return 0;

        /* Thor.990125: MYHOSTNAME�Τ@��J str_host */
    if (str_casecmp(str_host, str + 1))
        return 1;

    /* �d�I xyz@domain �� xyz.bbs@domain */

    *str = '\0';
    if ((str = strchr(addr, '.')))
        *str = '\0';
    return 0;
}

/* cache.091209: �۰���H�H��*/
/* cuser.userid �N�u���D title�B�ɮצb fpath�v���H��H�� userid ���~���H�c */
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
            vget(2, 0, "�D�D�G", ve_title, TTLEN + 1, DOECHO);

    }

    utmp_mode(M_SMAIL);
    fpath[0] = '\0';

    curredit = EDIT_MAIL;               /* Thor.1105: �������w�g�H */

    if (vedit(fpath, internet_mail ? 2 : 1) == -1)
    {
        unlink(fpath);
        clear();
        return -2;
    }

    if (internet_mail)
    {
        clear();
        prints("�H��Y�N�H�� %s\n���D���G%s\n�T�w�n�H�X��? (Y/n) [Y]",
            rcpt, title);
        switch (vkey())
        {
        case 'n':
        case 'N':
            outs("N\n�H��w����");
            refresh();
            rc = -2;
            break;

        default:
            outs("Y\n�еy�ԡA�H��ǻ���...\n");
            refresh();
            rc = bsmtp(fpath, title, rcpt, 0);
            if (rc < 0)
                vmsg("�H��L�k�H�F");
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
        strcpy(mhdr.nick, cuser.username);      /* :chuan: �[�J nick */
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
                vmsg("�H��L�k�H�F");
            mail_hold(fpath, acct.email);
            unlink(fpath);
            return rc;
        }
        else
            vmsg("�H��L�k�H�F");
                return -1;
    }
    if (!rc) {
        mail_hold(fpath, rcpt);
    }

#if 0
    prints("\n���H�H: %s (%s)\n��  �D: %s\n", mhdr.owner, mhdr.nick, mhdr.title);
    refresh();
#endif

    unlink(fpath);

    m_biff(userno);

#ifdef EMAIL_PAGE
    /* --------------------------------------------------- */
    /* E-Mail �����ǩI  by lkchu@dragon2.net               */
    /* --------------------------------------------------- */

    if ((acct_load(&acct, rcpt) >= 0) && (acct.ufo & UFO_MPAGER))
    {
        char *p;

        if ((p = str_casestr(acct.address, "bbc")) != NULL)  /* �� BBC �y�z */
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
        vmsg("�A�٦��ɮ��٨S�s���@�I");
        return;
    }


    if (!(HAS_PERM(PERM_INTERNET)))
        return;

//  if ((hdr->xmode & MAIL_NOREPLY) || m_count())
//      return;

    if ((hdr->xmode & MAIL_NOREPLY) || mail_stat(CHK_MAIL_NOMSG))
    {
        vmsg("�A���H�c�e�q�W�L�W���A�о�z�I");
        chk_mailstat = 1;
        return;
    }
    else
        chk_mailstat = 0;

    vs_bar("�^  �H");

    /* find the author */

    strcpy(quote_user, hdr->owner);
    strcpy(quote_nick, hdr->nick);

    /* make the title */

    if (!hdr_reply(3, hdr))
        return;

    prints("\n���H�H: %s (%s)\n��  �D: %s\n", quote_user, quote_nick, ve_title);

    /* Thor: ���F�٤@�� rec_put �^�H�h���]�ݹL���e */

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

    case -3:  /* Thor.980707: �������p�� ?*/
        sprintf(buf, "[%s] �L�k���H", quote_user);
        msg = buf;
        break;

    default:
        xmode |= MAIL_REPLIED;
        msg = "�H�w�H�X";  /* Thor.980705: mail_send()�w�g��ܹL�@���F.. ������? */
        break;
    }

    if (prefix == 'u')  /* user mail �ݫH�ɤ~�� r */
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
        vmsg("�A�٦��ɮ��٨S�s���@�I");
        return;
    }



    if (mail_stat(CHK_MAIL_NOMSG))
    {
        vmsg("�A���H�c�e�q�W�L�W���A�о�z�I");
        chk_mailstat = 1;
        return;
    }
    else
        chk_mailstat = 0;

//  if (m_count())
//      return;

    msg = "�H�w�H�X";

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

        case -3:  /* Thor.980707: �������p�� ?*/
            msg = "�ϥΪ̵L�k���H";
        }
    }
    vmsg(msg);
}


int
m_send(void)
{
    if (HAS_PERM(PERM_DENYMAIL))
        vmsg("�z���H�c�Q��F�I");
    else
    {
        ACCT muser;
        vs_bar("�H  �H");
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
        vmsg("�A�٦��ɮ��٨S�s���@�I");
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
        }         sysoplist[9]; /* ���] 9 �Ө��o */

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
        prints("%16s   %-16s�v�d����\n", "�s��", "���� ID");
        outsep(b_cols, msg_separator);
        outc('\n');

        for (i = 0; i < j; i++)
        {
            prints("%15d.   \x1b[1;%dm%-16s%s\x1b[m\n",
                i + 1, 31 + i, sysoplist[i].userid, sysoplist[i].duty);
        }
        prints("%-14s0.   \x1b[1;%dm���}\x1b[m", "", 31 + j);

        i = vans("�п�J�N�X[0]�G") - '0' - 1;
        if (i >= 0 && i < j)
        {
            clear();
            mail_send(sysoplist[i].userid, NULL);
        }
    }
    return 0;
}


/* ----------------------------------------------------- */
/* �s�ձH�H�B�^�H : multi_send, multi_reply              */
/* ----------------------------------------------------- */

#ifndef MULTI_MAIL

#define multi_reply(x) mail_reply(x)

#else  /* Thor.981009: ����R�����B�H */

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
        vmsg("�A�٦��ɮ��٨S�s���@�I");
        return -1;
    }

    vs_bar(title ? "�s�զ^�H" : "�s�ձH�H");

    ll_new();
    listing = reciper = 0;

    /* �^�H��Ū�� mail list �W�� */

    if (*quote_file)
    {
        ll_add(quote_user);
        reciper = 1;

        fp = fopen(quote_file, "r");
        while (fgets(buf, sizeof(buf), fp))
        {
            if (strncmp(buf, "�� ", 3))
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
                else if (!strncmp(userid, "[�q�i]", 6))
                    listing = 1;
            }
        }
        fclose(fp);
        ll_out(3, 0, MSG_CC);
    }

    /* �]�w mail list ���W�� */

    reciper = pal_list(reciper);

    /* �}�l�H�H */

    move(1, 0);
    clrtobot();

    if (reciper == 1)
    {
        mail_send(ll_head->data, title);
    }
    else if (reciper >= 2 && ve_subject(2, title, "[�q�i] "))
    {
        usr_fpath(fpath, cuser.userid, FN_NOTE);

        if ((fp = fopen(fpath, "w")))
        {
            fprintf(fp, "�� [�q�i] �@ %d �H����", reciper);
            listing = 80;
            wp = ll_head;

            do
            {
                userid = wp->data;
                reciper = strlen(userid) + 1;

                if (listing + reciper > 75)
                {
                    listing = reciper;
                    fprintf(fp, "\n��");
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
            vs_bar("�H�H��...");

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
        curredit = 0;           /* Thor.1105: ���O�i�H���[�F */
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
        vmsg("�A���H�c�e�q�W�L�W���A�о�z�I");
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

typedef struct {
    const char *mark;
    const char *undec[2]; /* Use `[1]` when lightbar is enabled, or `[0]` otherwise */
    const char *dec[2];
    const char *reset[2]; /* Use `[1]` when the title is currently declared, or `[0]` otherwise */
} HdrStyle;

static const HdrStyle hdr_style[HDRMODE_COUNT] = {
    {"��", {"\x1b[m", "\x1b[m"}, {"\x1b[1;37m", "\x1b[36m"}, {"", "\x1b[m"}},
    {"��", {"\x1b[1;32m", "\x1b[32m"}, {"\x1b[1;33m", "\x1b[33m"}, {"\x1b[m", "\x1b[m"}},
    {"��", {"\x1b[m", "\x1b[m"}, {"\x1b[1;37m", "\x1b[36m"}, {"", "\x1b[m"}},
    {"��", {"\x1b[1;36m", "\x1b[36m"}, {"\x1b[1;33m", "\x1b[33m"}, {"\x1b[m", "\x1b[m"}},
    {"Re", {"\x1b[m", "\x1b[m"}, {"\x1b[1;37m", "\x1b[36m"}, {"", "\x1b[m"}},
    {"=>", {"\x1b[1;33m", "\x1b[33m"}, {"\x1b[1;37m", "\x1b[37m"}, {"\x1b[m", "\x1b[m"}},
    {"��", {"\x1b[m", "\x1b[m"}, {"\x1b[1;33m", "\x1b[33m"}, {"", "\x1b[m"}},
    {"��", {"\x1b[1;32m", "\x1b[32m"}, {"\x1b[1;33m", "\x1b[33m"}, {"\x1b[m", "\x1b[m"}},
    {"��", {"\x1b[1;35m", "\x1b[0;35m"}, {"\x1b[1;31m", "\x1b[0;31m"}, {"\x1b[m", "\x1b[m"}},
    {"��", {"\x1b[1;30m", "\x1b[m"}, {"\x1b[0;37m", "\x1b[m"}, {"\x1b[m", "\x1b[m"}},
};

void
hdr_outs(               /* print HDR's subject */
    const HDR *hdr,
    int width)
{
    const HdrStyle *style;
    const char *title;

    const bool has_lightbar = HAVE_UFO2_CONF(UFO2_MENU_LIGHTBAR);

    if (width)
    {
        const char *owner = hdr->owner;
        const UTMP *const online = utmp_check(owner);  /* �ϥΪ̦b���W�N�ܦ� */

        int len = 13;
        int ch;

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

        if (online != NULL)
            outs(has_lightbar ? "\x1b[36m" : "\x1b[1;37m");

        while ((ch = (unsigned char) *owner))
        {
            if ((--len == 0) || (ch == '@'))
                ch = '.';
            outc(ch);

            if (ch == '.')
                break;

            owner++;
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
        width = d_cols + 64;
    }

    if (hdr->xmode & (POST_DELETE | POST_MDELETE))
    {
        title = hdr->title;
        style = &hdr_style[HDRMODE_DELETED];
    }
    else if (hdr->xmode & POST_LOCK)
    {
        title = (HAS_PERM(PERM_SYSOP)) ? hdr->title : "���峹�w�[�K��w�I";
        style = &hdr_style[HDRMODE_LOCKED];
    }
    else
    {
        enum HdrMode mode;
        title = str_ttl_hdrmode(hdr->title, &mode);
        style = &hdr_style[mode];
        if (!strcmp(currtitle, title))
            ++style;
    }
    outs(style->undec[has_lightbar]);
    outs(style->mark);

    {
        const char *const title_end = title + width;
        int ch = ' ';

#ifdef  HAVE_DECLARE            /* Thor.0508: Declaration, ���ըϬY��title����� */
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
            switch ((dbcs_hi) ? '\0' : ch)
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
            else if (IS_DBCS_HI(ch))
                dbcs_hi = true;

            switch ((pcnt) ? ch : '\0')
            {
            case '[':
            case '<':
            case '{':
                if (++*pcnt == 1)
                {
                    outs(style->dec[has_lightbar]);
                    outc(ch);
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
                    outc(ch);
                    outs(style->undec[has_lightbar]);
                    continue;
                }
            }

#endif  /* #ifdef  HAVE_DECLARE */

            outc(ch);
        } while ((ch = (unsigned char) *title++) && (title < title_end));

        outs(style->reset[
#ifdef  HAVE_DECLARE
            (square >= 1 || angle >= 1 || curly >= 1)
#else
            0
#endif
        ]);
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

#if 0                           /* Thor.0508: �ܦ�ݬ� */
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
    XO *xo,
    int pos)
{
    const HDR *const mhdr = (const HDR *) xo_pool_base + pos;
    move(3 + pos - xo->top, 0);
    mbox_item(pos + 1, mhdr);
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
        vmsg("�z�S���ӫH");
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
    vs_head("�l����", str_site);

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
    XO *xo,
    int pos)
{
    int xmode;
    HDR *hdr;
    char *dir;
#ifndef HAVE_MAILUNDELETE
    char fpath[64];
#endif

    hdr = (HDR *) xo_pool_base + pos;

    xmode = hdr->xmode;
    if ((xmode & (MAIL_MARKED | MAIL_DELETE)) == MAIL_MARKED)
        return XO_NONE; /* Thor.980901: mark��Y�Q'D'�_��, �h�@�˥i�Hdelete,
                                        �u�� MARK & no delete�~�|�L�� */

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
        const HDR hdr_orig = *hdr;
        if (!rec_del(dir, sizeof(HDR), pos, cmpchrono, NULL))
        {
            hdr_fpath(fpath, dir, &hdr_orig);
            unlink(fpath);
            return XO_LOAD;
        }
#endif
    }
    return XO_FOOT;
}

static int
mbox_forward(
    XO *xo,
    int pos)
{
    ACCT muser;
    const HDR *hdr;
    if (bbsothermode & OTHERSTAT_EDITING)
    {
        vmsg("�A�٦��ɮ��٨S�s���@�I");
        return XO_FOOT;
    }


    if (mail_stat(CHK_MAIL_NOMSG))
    {
        vmsg("�A���H�c�e�q�W�L�W���A�о�z�I");
        chk_mailstat = 1;
        return XO_HEAD;
    }
    else
        chk_mailstat = 0;

    if (acct_get("��F�H�󵹡G", &muser) > 0)
    {
        hdr = (const HDR *) xo_pool_base + pos;

        strcpy(quote_user, hdr->owner);
        hdr_fpath(quote_file, xo->dir, hdr);
        sprintf(ve_title, "%.64s (fwd)", hdr->title);
        move(1, 0);
        clrtobot();
        prints("��H��: %s (%s)\n��  �D: %s\n", muser.userid, muser.username,
            ve_title);

        switch (mail_send(muser.userid, ve_title))
        {
        case -1:
            outs(err_uid);
            break;

        case -2:
            outs(msg_cancel);
            break;

        case -3:  /* Thor.980707: �������p�� ?*/
            prints("�ϥΪ� [%s] �L�k���H", muser.userid);
        }
        *quote_file = '\0';
        vmsg(NULL);
    }
    return XO_HEAD;
}


static int
mbox_browse(
    XO *xo,
    int pos)
{
    HDR *mhdr, hdr;
    int xmode, nmode, mode;
    char *dir, *fpath;

    dir = xo->dir;
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
        vmsg("�z���H�c�w�W�X�e�q�A�L�k�\\Ū�s�H��A�вM�z�z���H�c�I");
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
        if (mbox_delete(xo, pos) != XO_FOOT)
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
        mbox_forward(xo, pos);
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
    XO *xo,
    int pos)
{
    int xmode;
    HDR *mhdr, hdr;

//  if (m_count())
//      return XO_HEAD;

    if (mail_stat(CHK_MAIL_NOMSG))
    {
        vmsg("�A���H�c�e�q�W�L�W���A�о�z�I");
        chk_mailstat = 1;
        return XO_HEAD;
    }
    else
        chk_mailstat = 0;

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
    XO *xo,
    int pos)
{
    HDR *mhdr;

    mhdr = (HDR *) xo_pool_base + pos;

    mhdr->xmode ^= MAIL_MARKED;
    rec_put(xo->dir, mhdr, sizeof(HDR), pos);
    return XO_CUR;
}


static int
mbox_tag(
    XO *xo,
    int pos)
{
    const HDR *hdr;
    int tag;

    hdr = (const HDR *) xo_pool_base + pos;

    if ((tag = Tagger(hdr->chrono, pos, TAG_TOGGLE)))
    {
        return XO_CUR + 1;
    }

    /* return XO_NONE; */
    return XO_MOVE + XO_REL + 1; /* lkchu.981201: ���ܤU�@�� */
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
    sprintf(buf, "�H��� %d/%d �ʡA�e�q�j�p %d/%d K�I", total_e, limit_e, total_k, limit_k);
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
    XO *xo,
    int pos)
{
    HDR *hdr;
    char fpath[128];
    if (bbsothermode & OTHERSTAT_EDITING)
    {
        vmsg("�A�٦��ɮ��٨S�s���@�I");
        return XO_FOOT;
    }

    hdr = (HDR *) xo_pool_base + pos;
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
    XO *xo,
    int pos)
{
    const HDR *hdr;
    char *dir, fpath[80], buf[128];
    struct stat st;

    dir = xo->dir;
    hdr = (const HDR *) xo_pool_base + pos;
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
        sprintf(buf, "�e�q�j�p: %lld K", (long long)st.st_size/1024);
        vmsg(buf);
    }


    return XO_HEAD;
}

#ifdef  HAVE_MAIL_FIX
static int
mbox_title(
    XO *xo,
    int pos)
{
    HDR *hdr, mhdr;

    if (!supervisor)
        return XO_NONE;


    hdr = (HDR *) xo_pool_base + pos;
    mhdr = *hdr;

    vget(B_LINES_REF, 0, "���D�G", mhdr.title, sizeof(mhdr.title), GCARRY);
    vget(B_LINES_REF, 0, "�@�̡G", mhdr.owner, 74, GCARRY);
    vget(B_LINES_REF, 0, "����G", mhdr.date, sizeof(mhdr.date), GCARRY);
    vget(B_LINES_REF, 0, "�ɦW�G", mhdr.xname, sizeof(mhdr.date), GCARRY);
    if (mhdr.xid > 1000)
        mhdr.xid = 0;
    if (vans(msg_sure_ny) == 'y' &&
        memcmp(hdr, &mhdr, sizeof(HDR)))
    {
        *hdr = mhdr;
        rec_put(xo->dir, hdr, sizeof(HDR), pos);
        return XO_INIT;
    }

    return XO_FOOT;
}
#endif /* HAVE_MAIL_FIX */


#ifdef HAVE_MAILUNDELETE

static int
mbox_undelete(
    XO *xo,
    int pos)
{
    HDR *hdr;

    hdr = (HDR *) xo_pool_base + pos;

    hdr->xmode &= ~POST_DELETE;

    if (!rec_put(xo->dir, hdr, sizeof(HDR), pos))
        return XO_INIT;
    return XO_NONE;
}

static int
mbox_clean(
    XO *xo)
{
    if (vans("\x1b[1;5;41;33mĵ�i�G\x1b[m�M�����ᤣ��Ϧ^�C�T�w�n�M���ܡH(y/N)") == 'y')
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
            vmsg("�ʺA�s�����ѡA���p���t�κ޲z���I");
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
    {XO_CUR | XO_POSF, {.posf = mbox_cur}},

#ifdef  HAVE_MAIL_FIX
    {'T' | XO_POSF, {.posf = mbox_title}},
#endif
    {'r' | XO_POSF, {.posf = mbox_browse}},
    {'E' | XO_POSF, {.posf = mbox_edit}},
    {'s', {mbox_send}},
    {'d' | XO_POSF, {.posf = mbox_delete}},
    {Ctrl('X') | XO_POSF, {.posf = mbox_forward}},
    {'m' | XO_POSF, {.posf = mbox_mark}},
#ifdef  HAVE_MAILGEM
    {'z', {mbox_gem}},
#endif
    {'R' | XO_POSF, {.posf = mbox_reply}},
    {'y' | XO_POSF, {.posf = mbox_reply}},
    {'c', {mbox_stat}},
#ifdef HAVE_MAILUNDELETE
    {'U', {mbox_clean}},
    {'u' | XO_POSF, {.posf = mbox_undelete}},
#endif

    {KEY_TAB, {mbox_sysop}},
    {'I', {mbox_other}},
    {'t' | XO_POSF, {.posf = mbox_tag}},
    {'S' | XO_POSF, {.posf = mbox_size}},
    {'D', {xo_delete}},

    {Ctrl('Q') | XO_POSF, {.posf = xo_uquery}},
    {'X' | XO_POSF, {.posf = xo_usetup}},
    {'x' | XO_POSF, {.posf = post_cross}},

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

