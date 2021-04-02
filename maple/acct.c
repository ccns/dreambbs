/*-------------------------------------------------------*/
/* acct.c       ( NTHU CS MapleBBS Ver 3.00 )            */
/*-------------------------------------------------------*/
/* target : account / administration routines            */
/* create : 95/03/29                                     */
/* update : 96/04/05                                     */
/*-------------------------------------------------------*/

#define ADMIN_C

#include "bbs.h"

#undef  CHANGE_USERNO
#undef  CHANGE_SECOND

#define STR_PERM      "bctpjm#x--------PTCMSNL*B#KGACBS"

/* log admin command by statue@WindTop */
void logitfile(const char *file, const char *key, const char *msg)
{
    time_t now;
    struct tm *p;
    char buf[256];

    time(&now);
    p = localtime(&now);
    sprintf(buf, "%02d/%02d/%02d %02d:%02d:%02d %s %-14s %s\n",
            p->tm_year % 100, p->tm_mon + 1, p->tm_mday,
            p->tm_hour, p->tm_min, p->tm_sec, key, cuser.userid,
            msg ? msg : "");
    f_cat(file, buf);
}

/* ----------------------------------------------------- */
/* �W�[�ȹ�, �u�}�n��, �H�h                              */
/* ----------------------------------------------------- */

void addmoney(int addend, const char *userid)
{
    ACCT acct;
    if (acct_load(&acct, userid) >= 0)
    {
        double temp = (acct.money + addend);    /* �קK���� */
        if (temp < INT_MAX)
            acct.money += addend;
        else
        {
            acct.money = (INT_MAX - 1);
        }
        acct_save(&acct);
    }
}

void addpoint1(int addend, const char *userid)
{
    ACCT acct;
    if (acct_load(&acct, userid) >= 0)
    {
        double temp = (acct.point1 + addend);    /* �קK���� */
        if (temp < INT_MAX)
            acct.point1 += addend;
        acct_save(&acct);
    }
}

void addpoint2(int addend, const char *userid)
{
    ACCT acct;
    if (acct_load(&acct, userid) >= 0)
    {
        double temp = (acct.point2 + addend);    /* �קK���� */
        if (temp < INT_MAX)
            acct.point2 += addend;
        acct_save(&acct);
    }
}

/* ----------------------------------------------------- */
/* (.ACCT) �ϥΪ̱b�� (account) subroutines              */
/* ----------------------------------------------------- */

void keeplog(const char *fnlog, const char *board, const char *title, int mode    /* 0:load 1:rename 2:unlink 3:mark */
    )
{
    HDR hdr;
    char folder[128], fpath[128];
    int fd;
    FILE *fp;

    if (!board)
        board = BRD_SYSTEM;

    sprintf(folder, "brd/%s/.DIR", board);
    fd = hdr_stamp(folder, 'A', &hdr, fpath);
    if (fd < 0)
        return;

    if (mode == 1)
    {
        close(fd);
        /* rename(fnlog, fpath); */
        f_mv(fnlog, fpath);        /* Thor.990409:�i��partition */
    }
    else
    {
        fp = fdopen(fd, "w");
        fprintf(fp, "�@��: SYSOP (%s)\n���D: %s\n�ɶ�: %s\n",
                SYSOPNICK, title, ctime(&hdr.chrono));
        f_suck(fp, fnlog);
        fclose(fp);
        if (mode == 2)
            unlink(fnlog);
    }

    strcpy(hdr.title, title);
    strcpy(hdr.owner, "SYSOP");
    strcpy(hdr.nick, SYSOPNICK);
    if (mode == 3)
        hdr.xmode |= POST_MARKED;
    fd = open(folder, O_WRONLY | O_CREAT | O_APPEND, 0600);
    if (fd < 0)
    {
        unlink(fpath);
        return;
    }
    write(fd, &hdr, sizeof(HDR));
    close(fd);
}


int acct_load(ACCT * acct, const char *userid)
{
    int fd;

    usr_fpath((char *)acct, userid, FN_ACCT);
    fd = open((char *)acct, O_RDONLY);
    if (fd >= 0)
    {
        /* Thor.990416: �S�O�`�N, ���� .ACCT�����׷|�O0 */
        read(fd, acct, sizeof(ACCT));
        close(fd);
    }
    return fd;
}

void acct_save(const ACCT * acct)
{
    int fd;
    char fpath[80];

    usr_fpath(fpath, acct->userid, FN_ACCT);
    fd = open(fpath, O_WRONLY, 0600);    /* fpath �����w�g�s�b */
    if (fd >= 0)
    {
        write(fd, acct, sizeof(ACCT));
        close(fd);
    }
}


int acct_userno(const char *userid)
{
    int fd;
    int userno;
    char fpath[80];

    usr_fpath(fpath, userid, FN_ACCT);
    fd = open(fpath, O_RDONLY);
    if (fd >= 0)
    {
        read(fd, &userno, sizeof(userno));
        close(fd);
        return userno;
    }
    return 0;
}


/* ----------------------------------------------------- */
/* name complete for user ID                             */
/* ----------------------------------------------------- */
/* return value :                                        */
/* 0 : �ϥΪ����� enter ==> cancel                       */
/* -1 : bad user id                                      */
/* o.w.: �Ǧ^�� userid �� userno                         */
/* ----------------------------------------------------- */

int acct_get(const char *msg, ACCT * acct)
{
    if (!vget(1, 0, msg, acct->userid, IDLEN + 1, GET_USER))
        return 0;

    if (acct_load(acct, acct->userid) >= 0)
        return acct->userno;

    vmsg(err_uid);
    return -1;
}


/* ----------------------------------------------------- */
/* �]�w�t���ɮ�                                          */
/* ----------------------------------------------------- */

static int x_edit_file(const void *arg)
{
    const char *fpath = (const char *)arg;
    int n;
    char buf[80];

    n = vget(B_LINES_REF, 0, "(D)�R�� (E)�s�� (V)�s�� [Q]�����H", buf, 3, LCECHO);
    if (n != 'd' && n != 'e' && n != 'v')
        return 0;

    if (bbsmode == M_UFILES)
        usr_fpath(buf, cuser.userid, fpath);
    else                        /* M_XFILES */
        sprintf(buf, "%s", fpath);

    if (n == 'd')
    {
        unlink(buf);
    }
    else if (n == 'e')
    {
        if (bbsothermode & OTHERSTAT_EDITING)
            vmsg("�A�٦��ɮ��٨S�s���@�I");
        else
            vmsg(vedit(buf, false) ? "��ʤ���" : "��s����");
    }                            /* Thor.981020: �`�N�Qtalk�����D  */
    else if (n == 'v')
    {
        more(buf, NULL);
    }
    return 0;
}

void x_file(int mode,            /* M_XFILES / M_UFILES */
            const char *const xlist[], /* description list */
            const char *const flist[]  /* filename list */
    )
{
    const char *desc;
    char buf[ANSILINESIZE];

    MENU list[41];

    int n = 0;
    while ((desc = xlist[n]))
    {
        sprintf(buf, "%*d %s", (mode == M_UFILES) ? 1 : 2, n+1, desc);
        if (mode == M_XFILES)          /* Thor.980806: ����: �L�X�ɦW */
        {
            int len_strip = strip_ansi_len(buf);
            sprintf(buf + len_strip,
                "%*s%s", BMAX(24 - len_strip - 4, 0), "", flist[n] + 4);
                                       /* statue.000703: ����: +4 �h���ؿ� */
        }
        list[n] =
            LISTLIT(MENU){{.funcarg = {{x_edit_file}, flist[n]}}, 0, mode | M_ARG, strdup(buf)};
        n++;
    }
    sprintf(buf, "%s\n%s", (mode == M_XFILES) ? "�s�t���ɮ�" : "�s�ӤH�ɮ�",
            "��J�ɮ׽s���i���ܸӶ��F�� (Enter)(��) �T�w�F�� (Esc)(e)(��) �^�h");
    list[n] = LISTLIT(MENU){{NULL}, PERM_MENU, mode, buf};

    if (mode == M_UFILES)
        domenu(list, MENU_YPOS_REF, MENU_XPOS_REF, 0, 0, 1);
    else
        domenu(list, ((B_LINES_REF-19) >> 1), 0, 20, T_COLS_REF >> 1, 2);

    for (MENU *mptr = list; mptr->item.func; mptr++)
        free((char *)mptr->desc);
}

int check_admin(const char *name)
{
    ADMIN admin;
    int pos = 0, fd;

    if (!str_casecmp(cuser.userid, ELDER))
        return 1;

    fd = open(FN_ETC_ADMIN_DB, O_RDONLY);
    while (fd >= 0)
    {
        lseek(fd, (off_t) (sizeof(admin) * pos), SEEK_SET);
        if (read(fd, &admin, sizeof(admin)) == sizeof(admin))
        {
            if (!strcmp(admin.name, name))
            {
                close(fd);
                return 1;
            }
            pos++;
        }
        else
        {
            close(fd);
            return 0;
        }
    }
    return 0;
}

/* ----------------------------------------------------- */
/* bit-wise display and setup                            */
/* ----------------------------------------------------- */

void bitmsg(const char *msg, const char *str, int level)
{
    int cc;

    outs(msg);
    while ((cc = *str))
    {
        outc((level & 1) ? cc : '-');
        level >>= 1;
        str++;
    }

    outc('\n');
}


unsigned int bitset(unsigned int pbits, int count,    /* �@���X�ӿﶵ */
                    int maxon,    /* �̦h�i�H enable �X�� */
                    const char *msg, const char *const perms[])
{
    int i, j, on;

    move(2, 0);
    clrtobot();
    outs(msg);

    for (i = on = 0, j = 1; i < count; i++)
    {
        msg = "��";
        if (pbits & j)
        {
            if (perms[i])
            {
                on++;
                msg = "��";
            }
            else
                pbits ^= j;
        }
        move(5 + (i % 16U), (i < 16 ? 0 : (b_cols+1) >> 1));
        if (perms[i])
            prints("%c %s %s", radix32[i], msg, perms[i]);
        else
            prints("\x1b[1;30m%c\x1b[m", radix32[i]);
        j <<= 1;
    }

    while ((i = vans("�Ы�������]�w�A�Ϋ� [Return] �����G")))
    {
        i -= '0';
        if (i >= 10)
            i -= 'a' - '0' - 10;

        if (i >= 0 && i < count && perms[i])
        {
            j = 1 << i;
            if (pbits & j)
            {
                on--;
                msg = "��";
            }
            else
            {
                if (on >= maxon)
                    continue;
                on++;
                msg = "��";
            }

            pbits ^= j;
            move(5 + (i % 16U), (i < 16 ? 0 : (b_cols+1) >> 1) + 2);
            outs(msg);
        }
    }
    return (pbits);
}


static unsigned int setperm(unsigned int level)
{
    if (HAS_PERM(PERM_SYSOP))
        return bitset(level, NUMPERMS, NUMPERMS, MSG_USERPERM, perm_tbl);

    /* [�b���޲z��] ����� SYSOP */

    if (level & PERM_SYSOP)
        return level;

    return bitset(level, NUMPERMS - 5, NUMPERMS - 5, MSG_USERPERM, perm_tbl);
}


/* ----------------------------------------------------- */
/* �b���޲z                                              */
/* ----------------------------------------------------- */

/* BLACK SU */
static void acct_su(const ACCT * u)
{
    XO *xo;
    char path[80], id[20] GCC_UNUSED;
    int level, ufo;

    if (!supervisor)
    {
        vmsg("�� �A���O�W�ů��ȡI");
        return;
    }
    ufo = cuser.ufo;
    level = cuser.userlevel;
    memcpy(&cuser, u, sizeof(ACCT));
    cuser.userlevel = level;
    cuser.ufo = ufo;
    usr_fpath(path, u->userid, FN_DIR);
    // IID.20190507: Xover object for mailbox (`cmbox`) is statically allocated.
/*
    xo = xz[XZ_MBOX - XO_ZONE].xo;
    xz[XZ_MBOX - XO_ZONE].xo =  xo_new(path);
    xz[XZ_MBOX - XO_ZONE].xo->cb = mbox_cb;
    xz[XZ_MBOX - XO_ZONE].xo->recsiz = sizeof(HDR);
    xz[XZ_MBOX - XO_ZONE].xo->pos = 0;
    free(xo);
*/
    // IID.20190508: Use `mbox_main()` to update `cmbox`.
    mbox_main();
    usr_fpath(path, u->userid, FN_BMW);
    xo = xz[XZ_BMW - XO_ZONE].xo;
    xz[XZ_BMW - XO_ZONE].xo =  xo_new(path);
    xz[XZ_BMW - XO_ZONE].xo->cb = bmw_cb;
    xz[XZ_BMW - XO_ZONE].xo->recsiz = sizeof(BMW);
    xz[XZ_BMW - XO_ZONE].xo->pos = 0;
    free(xo);
    pal_cache();
}

/* BLACK SU */

static void bm_list(            /* ��� userid �O���ǪO���O�D */
                       const char *userid)
{
    const BRD *bhdr = bshm->bcache;
    const BRD *const tail = bhdr + bshm->number;

    outs("����O�D�G");

    do
    {
        const char *const list = bhdr->BM;
        const int ch = *list;
        if ((ch > ' ') && (ch < 128))
        {
            if (str_has(list, userid))
            {
                outs(bhdr->brdname);
                outc(' ');
            }
        }
    }
    while (++bhdr < tail);

    outc('\n');
}

#ifdef LOG_ADMIN
/* Thor.990405: log permission modify */
static void perm_log(const ACCT * u, int oldl)
{
    int i;
    unsigned int level;
    char buf[128];

    for (i = 0, level = 1; i < NUMPERMS; i++, level <<= 1)
    {
        if ((u->userlevel & level) != (oldl & level))
        {
            sprintf(buf, "%s %s %s (%s) by %s\n", u->userid,
                    (u->userlevel & level) ? "��" : "��",
                    perm_tbl[i], Now(), cuser.userid);
            if (!str_casecmp(cuser.userid, ELDER))
                pmsg2("�O�D���ʤ��[�J��x");
            else
                f_cat(FN_SECURITY, buf);
        }
    }
}
#endif

/* 0: user config 1: admin 2: reg-form 3: normal user 4: bm setting */
void acct_show(const ACCT * u, int adm
    )
{
    time_t now;
    int diff;
    unsigned int ulevel;
    const char *uid;
    char buf[80];

    clrtobot();

    uid = u->userid;
    if (adm != 2)
        prints("\n�N    ���G%s        �ϥΪ̽s���G%d", uid, u->userno);

    prints("\n��    �١G%s\n"
           "�u��m�W�G%s\n"
           "�~���}�G%s\n"
           "�l��H�c�G%s\n",
           u->username,
           ((adm != 3) && (adm != 4)) ? u->realname : "��ƫO�K",
           ((adm != 3) && (adm != 4)) ? u->address : "��ƫO�K",
           (adm != 3) ? u->email : "��ƫO�K");

    prints("���U����G%s", ctime(&u->firstlogin));

    prints("���{����G%s", ctime(&u->lastlogin));

    diff = u->staytime / 60;
    prints("�W�����ơG%d �� (�@ %d �� %d ��)\n",
           u->numlogins, diff / 60, diff % 60);

    prints("�峹�ƥءG%d �g", u->numposts);

    prints(" (�u�}�n��:%d/�H��:%d/�ڹ�:%d)\n", u->point1, u->point2, u->money);

    usr_fpath(buf, uid, fn_dir);
    prints("�ӤH�H��G%d ��", rec_num(buf, sizeof(HDR)));

    prints(" �Ѿl�I�q���ơG%d ��\n", u->request);

    ulevel = u->userlevel;

    outs("�����{�ҡG\x1b[32m");
    if (ulevel & PERM_VALID)
    {
        outs(u->tvalid ? Ctime(&u->tvalid) : "���Ĵ����w�L�A�Э��s�{��");
    }
    else
    {
        outs("�аѦҥ������G��i��T�{�A�H���@�v��");
    }
    outs("\x1b[m\n");
    time(&now);
    if ((u->deny - now) > 0 || u->userlevel & PERM_DENYSTOP)
    {
        outs("�B�@�������G\x1b[1;31m");
        if (u->userlevel & PERM_DENYSTOP)
            outs("�L���{�D \x1b[m\n");
        else
        {
            outs(Ctime(&u->deny));
            outs("\x1b[m");
            prints("  �Z���ٳ� %ld �� %ld �� \n", (u->deny - now) / 86400,
                   (u->deny - now) / 3600 - ((u->deny - now) / 86400) * 24);
        }
    }

    if (adm)
    {
        if (adm != 3 && adm != 4)
        {
            prints("�{�Ҹ�ơG%s\n", u->justify);
            prints("�{�Ҧa�}�G%s\n", u->vmail);
            // IID.20190521: Currently unused; disabled for now.
            // prints("RFC 931 �G%s\n", u->ident);
        }
        prints("�W���a�I�G%s (���~�H %d )\n", u->lasthost, u->numemail);
        bitmsg(MSG_USERPERM, STR_PERM, ulevel);
        bitmsg("�X �� �@�G", "-----PMmane---b---Hh--Wm--p", u->ufo);
        bitmsg("�X �� �G�G", "0123456789ABCDEFGHIJKLMNOPQ", u->ufo2);
    }
    else
    {
        diff = (time(0) - ap_start) / 60;
        prints("���d�����G%d �p�� %d ��\n", diff / 60, diff % 60);
    }

    if (adm == 2)
        return;

    /* Thor: �Q�ݬݳo�� user �O���Ǫ������D */

    if (ulevel & PERM_BM)
        bm_list(uid);

#ifdef  NEWUSER_LIMIT
    if (u->lastlogin - u->firstlogin < 3 * 86400)
        outs("\n�s��W���G�T�ѫ�}���v��");
#endif
}

void bm_setup(ACCT * u, int adm)
{
    int y, x;

    getyx(&y, &x);
    acct_show(u, 3);

    if ((u->userlevel & PERM_SYSOP) && !(cuser.userlevel & PERM_SYSOP))
    {
        outs("���b���������������A�L�k����v���I");
        goto cleanup;
    }
    if (!str_casecmp(cuser.userid, ELDER))
        pmsg2("�O�D���ʤ��[�J��x");
    else
    {
        char tmp[80], why[80], buf[80] GCC_UNUSED;
        pmsg2("�O�D���ʤw�[�J������x");
        if (!vget(B_LINES_REF, 0, "�п�J���ʲz�ѡG", why, 40, DOECHO))
        {
            sprintf(why, "����J�z�ѡA�T���");
            pmsg2("�п�J���ʲz��");
            goto cleanup;
        }
        sprintf(tmp, "\n\n%s %-12s ��ϥΪ� %-12s ����O�D����\n�z��: ", Now(),
                cuser.userid, u->userid);
        f_cat(FN_BLACKSU_LOG, tmp);
        f_cat(FN_BLACKSU_LOG, why);
    }

    move(y, x);
    acct_show(u, adm);

    adm = vans("�]�w���D�v�� Y)�T�w N)���� Q)���} [Q] ");
    if (adm == 'y' || adm == 'Y')
        u->userlevel = u->userlevel | PERM_BM;
    else if (adm == 'n' || adm == 'N')
        u->userlevel = u->userlevel & (~PERM_BM);
    else
        goto cleanup;
    acct_save(u);

cleanup:
    move(1, 0);
    clrtobot();
}


static int seek_log_email(const char *mail, int mode)
{
    EMAIL email;
    int pos = 0, fd;
    fd = open(FN_VIOLATELAW_DB, O_RDONLY);
    while (fd >= 0)
    {
        lseek(fd, (off_t) (sizeof(email) * pos), SEEK_SET);
        if (read(fd, &email, sizeof(email)) == sizeof(email))
        {
            if (!strcmp(email.email, mail)
                && (mode ? (email.deny > time(0) || email.deny == -1) : 1))
            {
                close(fd);
                return pos;
            }
            pos++;
        }
        else
        {
            close(fd);
            break;
        }
    }
    return -1;
}

void deny_log_email(const char *mail, time_t deny)
{
    EMAIL email;
    int pos;
    pos = seek_log_email(mail, 0);
    if (pos >= 0)
    {
        rec_get(FN_VIOLATELAW_DB, &email, sizeof(EMAIL), pos);
        if (deny > email.deny || deny == -1)
            email.deny = deny;
        email.times++;
        rec_put(FN_VIOLATELAW_DB, &email, sizeof(EMAIL), pos);
    }
    else
    {
        memset(&email, 0, sizeof(email));
        strcpy(email.email, mail);
        email.deny = deny;
        rec_add(FN_VIOLATELAW_DB, &email, sizeof(EMAIL));
    }
}

static void deny_add_email(ACCT * he)
{
    char buf[128];
    time_t now;
    struct tm *p;

    time(&now);

    p = localtime(&now);
    str_lower(he->vmail, he->vmail);
    sprintf(buf, "%s # %02d/%02d/%02d %02d:%02d %s (%s)\n",
            he->vmail,
            p->tm_year % 100, p->tm_mon + 1, p->tm_mday,
            p->tm_hour, p->tm_min, "���v", cuser.userid);
    f_cat(FN_ETC_UNTRUST_ACL, buf);
}

static int select_mode(int adm)
{
    int select, days = 0, mode = 0;
    if (!adm)
    {
        select =
            vans
            ("�B�@ 1)������ 2)Cross Post 3)�s��H 4)�s�i�H 5)�c�� 6)�_�v 0)���� [0] ")
            - '0';
        if (select > 6 || select <= 0)
            return 0;
        if (select != 6)
        {
            days =
                vans
                ("�B�@���� 1)�@�P�� 2)��P�� 3)�T�P�� 4)�@�Ӥ� 5)�ä[ [1] ") -
                '0';
            mode =
                vans
                ("�B�@�覡 1)�T�� talk 2)��H�c 3)�T�� post 4)�ʺ� 5)���� 6)�P guest [3] ")
                - '0';
        }

        if (vans("�A�T�w�� [y/N] ") != 'y')
            return 0;

        if (days > 5 || days < 1)
            days = 1;
        if (mode > 6 || mode < 1)
            mode = 3;

        switch (select)
        {
        case 1:
            adm |= DENY_SEL_TALK;
            break;
        case 2:
            adm |= DENY_SEL_POST;
            break;
        case 3:
            adm |= DENY_SEL_MAIL;
            break;
        case 4:
            adm |= DENY_SEL_AD;
            break;
        case 5:
            adm |= DENY_SEL_SELL;
            break;
        case 6:
            adm |= DENY_SEL_OK;
            break;
        }
        switch (days)
        {
        case 1:
            adm |= DENY_DAYS_1;
            break;
        case 2:
            adm |= DENY_DAYS_2;
            break;
        case 3:
            adm |= DENY_DAYS_3;
            break;
        case 4:
            adm |= DENY_DAYS_4;
            break;
        case 5:
            adm |= DENY_DAYS_5;
            break;
        }
        switch (mode)
        {
        case 1:
            adm |= DENY_MODE_TALK;
            break;
        case 2:
            adm |= DENY_MODE_MAIL;
            break;
        case 3:
            adm |= DENY_MODE_POST;
            break;
        case 4:
            adm |= DENY_MODE_NICK;
            break;
        case 5:
            adm |= DENY_MODE_ALL;
            break;
        case 6:
            adm |= DENY_MODE_GUEST;
            break;
        }

    }
    return adm;
}

int add_deny(ACCT * u, int adm, int cross)
{
    FILE *fp;
    char buf[80];
    ACCT x;
    time_t now;
    bool check_time;
    const char *cselect = NULL, *cdays = NULL, *cmode = NULL;

    memcpy(&x, u, sizeof(ACCT));
    time(&now);
    check_time = (x.deny > now);


    fp = fopen(FN_STOP_LOG, "w");
    if (!adm)
    {
        adm = select_mode(adm);
    }
    if (!adm)
    {
        if (fp)
            fclose(fp);
        return 0;
    }

    if (!strncmp(u->justify, "reg:", 4))
        adm = (adm & ~DENY_MODE_ALL) | DENY_MODE_GUEST;

    if (adm & DENY_SEL_OK)
    {
        x.deny = now;
        memcpy(u, &x, sizeof(x));
        acct_save(u);
        return adm;
    }
    if (adm & DENY_SEL)
    {
        if (adm & DENY_SEL_TALK)
            cselect = "������";
        else if (adm & DENY_SEL_POST)
            cselect = " Cross Post";
        else if (adm & DENY_SEL_MAIL)
            cselect = "���o�s��H";
        else if (adm & DENY_SEL_AD)
            cselect = "���o�s�i�H";
        else if (adm & DENY_SEL_SELL)
            cselect = "�c��D�k�ƪ�";
        fprintf(fp, "�d %s �H�ϯ��W%s�A�̯��W����", u->userid, cselect);
    }
    if ((adm & (DENY_MODE_ALL)) && !(adm & DENY_MODE_GUEST))
    {
        if ((adm & DENY_MODE_ALL) == DENY_MODE_ALL)
        {
            x.userlevel |=
                (PERM_DENYPOST | PERM_DENYTALK | PERM_DENYCHAT | PERM_DENYMAIL
                 | PERM_DENYNICK);
            cmode = " Talk�BMail�B\nPost�B���ʺ�";
        }
        else if (adm & DENY_MODE_POST)
        {
            x.userlevel |= (PERM_DENYPOST);
            cmode = " Post ";
        }
        else if (adm & DENY_MODE_TALK)
        {
            x.userlevel |= (PERM_DENYTALK | PERM_DENYCHAT);
            cmode = " Talk ";
        }
        else if (adm & DENY_MODE_MAIL)
        {
            x.userlevel |= (PERM_DENYMAIL);
            cmode = " Mail ";
        }
        else if (adm & DENY_MODE_NICK)
        {
            x.userlevel |= (PERM_DENYNICK);
            cmode = "���ʺ�";
        }
        fprintf(fp, "%s�v��", cmode);
    }
    if (adm & DENY_MODE_GUEST)
    {
        x.userlevel |=
            (PERM_DENYPOST | PERM_DENYTALK | PERM_DENYCHAT | PERM_DENYMAIL |
             PERM_DENYNICK | PERM_DENYSTOP);
        x.userlevel &= ~(PERM_BASIC | PERM_VALID);
        x.deny += 86400 * 31;
        cmode = " Talk�BMail�B\nPost�B���ʺ�";
        fprintf(fp,
                "%s�v���A�v������ guest�A�ä��_�v�A�ëO�d�b���A\n�� E-mail�G%s �ä��o�b�������U�C\n\n",
                cmode, u->vmail[0] ? u->vmail : "[�L�{�ҫH�c]");
        deny_add_email(u);
    }
    if ((adm & DENY_DAYS) && !(adm & DENY_MODE_GUEST))
    {
        x.deny = ((now > x.deny) ? now : x.deny);
        if (adm & DENY_DAYS_1)
        {
            cdays = "�@�P��";
            x.deny += 86400 * 7;
        }
        else if (adm & DENY_DAYS_2)
        {
            cdays = "��P��";
            x.deny += 86400 * 14;
        }
        else if (adm & DENY_DAYS_3)
        {
            cdays = "�T�P��";
            x.deny += 86400 * 21;
        }
        else if (adm & DENY_DAYS_4)
        {
            cdays = "�@�Ӥ�";
            x.deny += 86400 * 31;
        }
        else if (adm & DENY_DAYS_5)
        {
            cdays = "";
            x.deny += 86400 * 31;
            x.userlevel |= PERM_DENYSTOP;
        }
        fprintf(fp, "%s�C\n", cdays);
        if (adm & DENY_DAYS_5)
            fprintf(fp, "����: �ä��_�v�C\n\n");
        else
            fprintf(fp, "����: %s%s�A�����@�L�۰ʴ_�v�C\n\n",
                    check_time ? "�W���B�@�����֥[" : "�q���Ѱ_", cdays);
    }
    fprintf(fp,
            "\x1b[1;32m�� Origin: \x1b[1;33m%s \x1b[1;37m<%s>\n\x1b[1;31m�� From: \x1b[1;36m%s\x1b[m\n",
            BOARDNAME, MYHOSTNAME, MYHOSTNAME);

    fclose(fp);
    sprintf(buf, "[%s�B�@] %s %s", cross ? "�s��" : "", u->userid, cselect);
    keeplog(FN_STOP_LOG, BRD_VIOLATELAW, buf, 3);
    usr_fpath(buf, x.userid, FN_STOPPERM_LOG);
    fp = fopen(buf, "a+");
    f_suck(fp, FN_STOP_LOG);
    fclose(fp);

    memcpy(u, &x, sizeof(x));
    acct_save(u);
    return adm;
}


void acct_setup(ACCT * u, int adm)
{
    ACCT x;
    int y, c;

    int i, num, tmp, mode;
    FILE *flog;
    char *str, buf[80], pass[PLAINPASSSIZE];
    char id[13];
    tmp = 0;

    getyx(&y, &c);
    acct_show(u, (adm) ? 3 : 0);

    memcpy(&x, u, sizeof(ACCT));

    if (((u->userlevel & PERM_SYSOP) && strcmp(cuser.userid, u->userid))
        && !check_admin(cuser.userid))
    {
        outs("���b���������������A�L�k���I");
        goto cleanup;
    }

    if (adm)
    {
        if (supervisor)
        {
            if (!str_casecmp(cuser.userid, ELDER))
                pmsg2("�d�߰ʧ@���[�J��x");
            else
            {
                char tmp[80], why[80];
                pmsg2("�d�߰ʧ@�w�[�J������x");
                if (!vget(B_LINES_REF, 0, "�п�J�z�ѡG", why, 40, DOECHO))
                {
                    sprintf(why, "����J�z�ѡA�T��d��");
                    pmsg2("�п�J�d�߲z��");
                    goto cleanup;
                }
                sprintf(tmp,
                        "\n\n%s %-12s ��ϥΪ� %-12s ����d�߰ʧ@\n�z��: ",
                        Now(), cuser.userid, u->userid);
                f_cat(FN_BLACKSU_LOG, tmp);
                f_cat(FN_BLACKSU_LOG, why);
            }

            move(y, c);
            acct_show(u, adm);

            adm =
                vans
                ("�]�w 1)��� 2)�v�� 3)�s���B�@ 4)��H�B�@ 5)���v�� 6)�X�� 7)SU Q)���� [Q] ");
        }
        else
        {
            if (!str_casecmp(cuser.userid, ELDER))
                pmsg2("�d�߰ʧ@���[�J��x");
            else
            {
                char tmp[80], why[80];
                pmsg2("�d�߰ʧ@�w�[�J������x");
                if (!vget(B_LINES_REF, 0, "�п�J�d�߲z�ѡG", why, 40, DOECHO))
                {
                    sprintf(why, "����J�z�ѡA�T��d��");
                    pmsg2("�п�J�d�߲z��");
                    goto cleanup;
                }
                sprintf(tmp,
                        "\n\n%s %-12s ��ϥΪ� %-12s ����d�߰ʧ@\n�z��: ",
                        Now(), cuser.userid, u->userid);
                f_cat(FN_BLACKSU_LOG, tmp);
                f_cat(FN_BLACKSU_LOG, why);
            }

            move(y, c);
            acct_show(u, adm);

            adm =
                vans
                ("�]�w 1)��� 2)�v�� 3)�s���B�@ 4)��H�B�@ 5)���v�� 6)�X�� Q)���� [Q] ");
        }
        if (adm == '6')
        {
            su_setup(u);
            acct_save(u);
        }
        /* BLACK SU */
        if (adm == '7' && supervisor)
            acct_su(u);
        /* BLACK SU */
        if (adm == '4')
        {
            tmp = add_deny(u, tmp, 0);
            if ((tmp & DENY_MODE_ALL) && (u->deny > time(0)))
                deny_log_email(u->vmail,
                               (u->userlevel & PERM_DENYSTOP) ? -1 : u->deny);
        }
        if (adm == '3')
        {
            DL_HOTSWAP_SCOPE int (*sm) (char *mail) = NULL;
            switch (vans("�ϥε{�� i)���� o)�~�� q)�����G[q]"))
            {
            case 'i':
                if (!sm)
                {
                    sm = DL_NAME_GET("same_mail.so", same_mail);
                }
                strcpy(id, u->userid);
                if (sm)
                {
                    num = (*sm) (u->vmail);
                }
                else
                    break;

                flog = fopen(FN_SAMEEMAIL_LOG, "r");
                tmp = 0;
                if (flog == NULL)
                    goto cleanup;

                for (i = 1; i <= num; i++)
                {
                    fscanf(flog, "%13s", buf);
                    acct_load(u, buf);
                    if (u != NULL)
                    {
                        if (strcmp(u->userid, id))
                            tmp = add_deny(u, tmp, 1);
                        else
                            tmp = add_deny(u, tmp, 0);
                    }
                    if (!tmp)
                        break;
                }

                if (tmp & DENY_MODE_ALL)
                    deny_log_email(u->vmail,
                                   (u->userlevel & PERM_DENYSTOP) ? -1 : u->
                                   deny);
                fclose(flog);
                break;
            case 'o':
                {
                    char buf_mode[12], buf_time[22];
                    mode = select_mode(0);
                    sprintf(buf_mode, "%d", mode);
                    sprintf(buf_time, "%lld", (long long)time(NULL));
                    PROC_CMD_BG(BINARY_SUFFIX"stopperm", u->userid, u->vmail, buf_mode, cuser.userid, buf_time);
                }
                break;
            }
        }
        if (adm == '5')
        {
            u->userlevel = PERM_BASIC;
            acct_save(u);
            goto cleanup;
        }
        if (adm == '2')
            goto set_perm;

        if (adm != '1')
            goto cleanup;
    }
    else
    {
        if (vans("�ק���(y/N)?[N] ") != 'y')
            goto cleanup;
    }

    move(i = 3, 0);
    clrtobot();

    if (adm)
    {
        str = x.userid;
        for (;;)
        {
            vget(i, 0, "�ϥΪ̥N��(����Ы� Enter)�G", str, IDLEN + 1, GCARRY);
            if (!str_casecmp(str, u->userid) || !acct_userno(str))
                break;
            vmsg("���~�I�w���ۦP ID ���ϥΪ�");
        }
    }
    else
    {
        /* pcbug.990813: �sPASSLEN�L��, �令�����g�� */
        /*    vget(i, 0, "�нT�{�K�X�G", buf, PASSSIZE, NOECHO); */
        vget(i, 0, "�нT�{�K�X�G", buf, PLAINPASSSIZE, NOECHO | VGET_STEALTH_NOECHO);
        if (chkpasswd(u->passwd, u->passhash, buf))
        {
            vmsg("�K�X���~");
            goto cleanup;
        }
    }

    i++;
    for (;;)
    {
        /* IID.20190530: For forward compatibility with older versions */
        if (vget(++i, 0, "�O�_�ϥηs���K�X�[�K(y/N)�H[N]", buf, 3, LCECHO) == 'y')
        {
            mode = GENPASSWD_SHA256;
            num = PLAINPASSSIZE;
        }
        else
        {
            mode = GENPASSWD_DES;
            num = OLDPLAINPASSSIZE;
        }

        if (!vget
            (++i, 0, "�]�w�s�K�X(����Ы� Enter)�G", buf, /*PASSSIZE*/ num,
             NOECHO | VGET_STEALTH_NOECHO))
            break;

        strcpy(pass, buf);
        vget(i + 1, 0, "�ˬd�s�K�X�G", buf, /*PASSSIZE*/ num, NOECHO | VGET_STEALTH_NOECHO);
        if (!strcmp(buf, pass))
        {
            buf[num-1] = '\0';
            str_scpy(x.passwd, str = genpasswd(buf, mode), PASSSIZE);
            str_scpy(x.passhash, str + PASSSIZE, sizeof(x.passhash));
            i++;
            logitfile(FN_PASS_LOG, cuser.userid, cuser.lasthost);
            break;
        }
    }

    i++;
    str = x.username;
    do
    {
        vget(i, 0, "��    �١G", str, sizeof(x.username), GCARRY);
    }
    while (str_len_nospace(str) < 1);

    i++;
    str = x.realname;
    do
    {
        vget(i, 0, "�u��m�W�G", str, sizeof(x.realname), GCARRY);
    }
    while (str_len_nospace(str) < 4);

    i++;
    str = x.address;
    do
    {
        vget(i, 0, "�~��a�}�G", str, sizeof(x.address), GCARRY);
    }
    while (str_len_nospace(str) < 8);

    if (adm)
    {
        i++;
        str = x.email;

        vget(i, 0, "E-mail �H�c�G", str, sizeof(x.email), GCARRY);

        vget(++i, 0, "�{�Ҹ�ơG", x.justify, 44, GCARRY);
        if (strlen(x.justify) > 4)
        {
            vget(++i, 0, "�W�[���Ĵ���(y/N)�G", buf, 2, DOECHO);
            if (buf[0] == 'y' || buf[0] == 'Y')
            {
                time(&x.tvalid);
                x.userlevel |=
                    (PERM_BASIC | PERM_CHAT | PERM_PAGE | PERM_POST |
                     PERM_VALID);
                /*by visor */
            }
        }
        sprintf(buf, "%d", u->numlogins);
        vget(++i, 0, "�W�u���ơG", buf, 10, GCARRY);
        if ((num = atoi(buf)) >= 0)
            x.numlogins = num;

#ifdef CHANGE_USERNO
        sprintf(buf, "%d", u->userno);
        vget(++i, 0, "�ϥΪ̽s���G", buf, 10, GCARRY);
        if ((num = atoi(buf)) >= 0)
            x.userno = num;
#endif

        sprintf(buf, "%d", u->numposts);
        vget(++i, 0, "�峹�g�ơG", buf, 10, GCARRY);
        if ((num = atoi(buf)) >= 0)
            x.numposts = num;

        sprintf(buf, "%d", u->money);
        vget(++i, 0, "�ڹ��G", buf, 10, GCARRY);
        if ((num = atoi(buf)) >= 0)
            x.money = num;

        sprintf(buf, "%d", u->point1);
        vget(++i, 0, "�u�}�n���G", buf, 10, GCARRY);
        if ((num = atoi(buf)) >= 0)
            x.point1 = num;

        sprintf(buf, "%d", u->point2);
        vget(++i, 0, "�H��G", buf, 10, GCARRY);
        if ((num = atoi(buf)) >= 0)
            x.point2 = num;

#ifdef  CHANGE_SECOND
        sprintf(buf, "%d", u->staytime);
        vget(++i, 0, "�W���`��ơG", buf, 20, GCARRY);
        if ((num = atoi(buf)) >= 0)
            x.staytime = num;
#endif

        sprintf(buf, "%d", u->request);
        vget(++i, 0, "�I�q���ơG", buf, 10, GCARRY);
        if ((num = atoi(buf)) >= 0)
            x.request = num;

        /* lkchu.981201: �S��γ~ :p */
        vget(++i, 0, "�{�Ҧa�}�G", x.vmail, 44, GCARRY);
        vget(++i, 0, "�W���a�I�G", x.lasthost, 30, GCARRY);
        // IID.20190521: Currently unused; disabled for now.
        // vget(++i, 0, "RFC 931 �G", x.ident, 44, GCARRY);

        if (vans("�]�w�v��(y/N)?[N] ") == 'y')
        {
          set_perm:

            i = setperm(num = x.userlevel);

            if (i == num)
            {
                vmsg("�����ק�");
                if (adm == '2')
                    goto cleanup;
            }
            else
            {
                x.userlevel = i;
            }
        }
    }

    if (vans(msg_sure_ny) != 'y')
        goto cleanup;

    if (adm)
    {
        if (str_casecmp(u->userid, x.userid))
        {                        /* Thor: 980806: �S�O�`�N�p�G usr�C�Ӧr�����b�P�@partition���ܷ|�����D */
            char dst[80];

            usr_fpath(buf, u->userid, NULL);
            usr_fpath(dst, x.userid, NULL);
            rename(buf, dst);
            /* Thor.990416: �S�O�`�N! .USR�å��@�֧�s, �i�঳�������D */
        }
#ifdef LOG_ADMIN
        /* lkchu.981201: security log */
        perm_log(&x, u->userlevel);
#endif
    }
    else
    {
        /* Thor: �o�˧Y�Ϧb�u�W, �]�i�H�� userlevel */

        if (acct_load(u, x.userid) >= 0)
            x.userlevel = u->userlevel;
    }

    memcpy(u, &x, sizeof(x));
    acct_save(u);

cleanup:
    move(1, 0);
    clrtobot();
}


int u_info(void)
{
    char *str, username[24];    /* Thor.980727:lkchu patch: username[20] -> 24 */

    move(2, 0);
    strcpy(username, str = cuser.username);
    acct_setup(&cuser, 0);
    if (strcmp(username, str) && HAS_PERM(PERM_VALID)
        && (!HAS_PERM(PERM_DENYNICK) || HAS_PERM(PERM_SYSOP)))
        memcpy(cutmp->username, str, sizeof(cuser.username));
    else if (HAS_PERM(PERM_DENYNICK))
        vmsg("�T��ק�ʺ�");
    return 0;
}


int m_user(void)
{
    int ans;
    ACCT acct;

    while ((ans = acct_get(msg_uid, &acct)))
    {
        if (ans > 0)
            acct_setup(&acct, 1);
    }
    return 0;
}

int m_bmset(void)
{
    int ans;
    ACCT acct;
    while ((ans = acct_get(msg_uid, &acct)))
    {
        if (ans > 0)
            bm_setup(&acct, 4);
    }
    return 0;
}


/* ----------------------------------------------------- */
/* �]�w E-mail address                                   */
/* ----------------------------------------------------- */


int ban_addr(const char *addr)
{
    int i;
    char *host, *str GCC_UNUSED;
    char foo[64];                /* SoC: ��m���ˬd�� email address */
    const char* str_invalid;

    static const char *const invalid[] = { "@bbs", "bbs@", "root@", "gopher@",
        "guest@", "@ppp", "@slip", "@dial", "unknown@", "@anon.penet.fi",
        "193.64.202.3", "brd@", NULL
    };

    /* SoC: �O���� email ���j�p�g */
    str_lower(foo, addr);

    for (i = 0; (str_invalid = invalid[i]); i++)
    {
        if (strstr(foo, str_invalid))
            return 1;
    }

    /* check for mail.acl (lower case filter) */

    host = (char *)strchr(foo, '@');
    *host = '\0';
    /* i = acl_has(FN_ETC_SPAMMER_ACL, foo, host + 1); */
    /* Thor.981223: �Nbbsreg�ڵ��������} */
    i = acl_has(FN_ETC_UNTRUST_ACL, foo, host + 1);
    /* *host = '@'; */
    if (i < 0)
        blog("NOACL", host);
    return i > 0;
}

#ifdef HAVE_WRITE
static int allow_addr(const char *addr)
{
    int i;
    char *host;
    char foo[64];

    str_lower(foo, addr);

    host = (char *)strchr(foo, '@');
    *host = '\0';
    i = acl_has(FN_ETC_ALLOW_ACL, foo, host + 1);
    return i > 0;
}
#endif

void                            /* gaod:���s:p */
check_nckuemail(char *email)
{
    char *ptr;
    ptr = strstr(email, DEFAULTSERVER);

    if (ptr)
    {
        strcpy(ptr, NCKUMAIL);
    }
}

/* ��M�O�_�����U�T�ӥH�W�� Email */
int find_same_email(            /* mode : 1.find 2.add 3.del */
                       const char *mail, int mode)
{
    int pos = 0, fd;
    const char *fpath;
    SAME_EMAIL email;


    fpath = FN_ETC_EMAILADDR_ACL;

    if (mode >= 1 && mode <= 3)
    {
        fd = open(fpath, O_RDONLY);
        pos = 0;
        while (fd >= 0)
        {
            lseek(fd, (off_t) (sizeof(SAME_EMAIL) * pos), SEEK_SET);
            if (read(fd, &email, sizeof(SAME_EMAIL)) == sizeof(SAME_EMAIL))
            {
                if (!strcmp(mail, email.email))
                    break;
                pos++;
            }
            else
            {
                pos = -1;
                break;
            }
        }
        if (fd >= 0)
            close(fd);
    }


    if (mode == 1)
    {
        if (pos >= 0)
            return email.num;
        else
            return 0;
    }
    if (mode == 2)
    {
        if (pos == -1)
        {
            memset(&email, 0, sizeof(SAME_EMAIL));
            strcpy(email.email, mail);
            email.num = 1;
            rec_add(fpath, &email, sizeof(SAME_EMAIL));
        }
        else
        {
            email.num++;
            rec_put(fpath, &email, sizeof(SAME_EMAIL), pos);
        }
    }
    if (mode == 3)
    {
        if (pos == -1)
            return 0;
        if (email.num == 1)
        {
            rec_del(fpath, sizeof(SAME_EMAIL), pos, NULL, NULL);
        }
        else
        {
            email.num--;
            rec_put(fpath, &email, sizeof(SAME_EMAIL), pos);
        }
    }
    return 0;
}

int u_addr(void)
{
    const char *msg;
    char addr[60], buf[40], agent[128], temp[60];
    HDR fhdr;
    FILE *fout;
    int vtime;
    unsigned int tmp_perm;
    int popreturn;

    msg = NULL;
    more(FN_ETC_EMAIL, (char *)-1);
    strcpy(temp, cuser.email);
    tmp_perm = cuser.userlevel;
    /* lkchu.981201 */
    if (vget
        (B_LINES_REF - 1, 0, "E-Mail �a�}�G", addr, sizeof(cuser.email), DOECHO))
    {
#ifndef HAVE_SIMPLE_RFORM
        vtime = REG_REQUEST;
#endif
        str_lower(addr, addr);
        if (not_addr(addr))
        {
            msg = "���X�檺 E-mail address";
            pmsg(msg);
            return 0;
        }
        else if (ban_addr(addr))
        {
            msg = "�T����U�� E-mail address";
            pmsg(msg);
            return 0;
        }
#ifdef HAVE_WRITE
        else if (!allow_addr(addr))
        {
#ifndef HAVE_SIMPLE_RFORM
            attr_put(cuser.userid, ATTR_REG_KEY, &vtime);
#endif
            msg = "�|���ӽгq�L�� E-mail �D��";
            pmsg(msg);
            return 0;
        }
#endif  /* #ifdef HAVE_WRITE */
        else if (strcmp(temp, addr) && (seek_log_email(addr, 1) != -1))
        {
            msg = "�ȮɸT����U�� E-mail address";
            pmsg(msg);
            return 0;
        }
        else if (strcmp(temp, addr) ? (find_same_email(addr, 1) >= MAX_REGIST)
                 : (find_same_email(addr, 1) > MAX_REGIST))
        {
            msg = "���U�H�ƶW�L 3 �Ӫ� E-mail address";
            pmsg(msg);
            return 0;
        }

        /* pcbug.990522: pop3�{��. */
        vget(B_LINES_REF - 2, 0, "�O�_�ϥ� POP3 �{��?[Y]", buf, 2, LCECHO);

        if (buf[0] != 'n' && buf[0] != 'N')
        {
            char title[80], *ptr;
            int sock = 110;

            strcpy(title, addr);
            check_nckuemail(title);

            *(ptr = strchr(title, '@')) = 0;

            clear();
            move(2, 0);
            prints("�D��: %s\n�b��: %s\n", ptr + 1, title);
            prints("\x1b[1;5;36m�s�u���ݥD����...�еy��\x1b[m\n");
            refresh();
            if (!Get_Socket(ptr + 1, &sock))
            {
                close(sock);
                move(4, 0);
                clrtoeol();
                while (1)
                {
                    move(15, 0);
                    clrtobot();
                    vget(15, 0, "�п�J�H�W�ҦC�X���u�@���b�����K�X: ", buf,
                         PLAINPASSSIZE, NOECHO | VGET_STEALTH_NOECHO);
                    move(16, 0);
                    prints("\x1b[5;37m�����T�{��...�еy��\x1b[m\n\n");
                    refresh();

                    if (!(popreturn = POP3_Check(ptr + 1, title, buf)))
                    {
                        logitfile(FN_VERIFY_LOG, "-POP3 Verify OK -", addr);
                        cuser.userlevel |=
                            (PERM_VALID | PERM_POST | PERM_PAGE | PERM_CHAT);
                        if (cuser.userlevel & PERM_DENYPOST)
                            cuser.userlevel &= ~PERM_POST;

                        if (cuser.userlevel & PERM_DENYTALK)
                            cuser.userlevel &= ~PERM_PAGE;

                        if (cuser.userlevel & PERM_DENYCHAT)
                            cuser.userlevel &= ~PERM_CHAT;
                        str_scpy(cuser.vmail, addr, sizeof(cuser.vmail));
                        sprintf(agent, "pop3�{��:%s", addr);
                        str_scpy(cuser.justify, agent, sizeof(cuser.justify));
                        time(&cuser.tvalid);
                        strcpy(cuser.email, addr);
                        acct_save(&cuser);
                        find_same_email(addr, 2);
                        if (tmp_perm & PERM_VALID)
                            find_same_email(temp, 3);
                        usr_fpath(buf, cuser.userid, FN_JUSTIFY);
                        if ((fout = fopen(buf, "a")))
                        {
                            fprintf(fout, "%s\n", agent);
                            fclose(fout);
                        }
                        usr_fpath(buf, cuser.userid, fn_dir);
                        hdr_stamp(buf, HDR_LINK, &fhdr, FN_ETC_JUSTIFIED_POP3);
                        strcpy(fhdr.title,
                               "[���U���\\] �z�w�g�q�L�����{�ҤF�I");
                        strcpy(fhdr.owner, "SYSOP");
                        rec_add(buf, &fhdr, sizeof(fhdr));
                        board_main();
                        gem_main();
                        talk_main();
                        cutmp->ufo |= UFO_BIFF;
                        msg = "�����T�{���\\�A�ߨ责�@�v��";
                        break;
                    }
                    else
                    {
                        logitfile(FN_VERIFY_LOG, "-POP3 Verify ERR-", addr);
                        if (popreturn != 8)
                            vget(17, 0,
                                 "�����T�{���ѡA�O�_���s�T�{ (Y/n) ? [Y]", buf,
                                 3, LCECHO);

                        if (buf[0] == 'n' || buf[0] == 'N' || popreturn == 8)
                        {
                            cuser.userlevel &=
                                ~(PERM_VALID | PERM_POST | PERM_PAGE |
                                  PERM_CHAT);
                            cuser.vtime = -1;
                            strcpy(cuser.email, addr);
                            acct_save(&cuser);
                            board_main();
                            gem_main();
                            talk_main();

                            msg = "�����T�{���ѡA�v������....";
                            break;
                        }
                    }
                }
            }
            else
            {
                prints("POP3: ���䴩�A�����T�{\x1b[1;36m�ϥλ{�ҫH��\x1b[m"
                       "\n\n\x1b[1;36;5m�t�ΰe�H��...\x1b[m");
                refresh();
                sleep(1);
                buf[0] = 'n';
            }
        }



        if (buf[0] == 'n' || buf[0] == 'N')
        {

            if (tmp_perm & PERM_VALID)
                find_same_email(temp, 3);

            vtime = bsmtp(NULL, NULL, addr, MQ_JUSTIFY);

            if (vtime < 0)
            {
                msg = "�����{�ҫH��L�k�H�X�A�Х��T��g E-mail address";
                cuser.userlevel &=
                    ~(PERM_VALID | PERM_POST | PERM_PAGE | PERM_CHAT);
                cuser.vtime = vtime;
                strcpy(cuser.email, addr);
                acct_save(&cuser);
                board_main();
                gem_main();
                talk_main();
            }
            else
            {
                cuser.userlevel &= ~(PERM_VALID);
                cuser.vtime = vtime;
                strcpy(cuser.email, addr);
                acct_save(&cuser);

                more(FN_ETC_JUSTIFY, (char *)-1);
                /* lkchu.981201 */
                prints("\n%s(%s)�z�n�A�ѩ�z��s E-mail address ���]�w�A\n"
                       "�бz���֨� \x1b[44m%s\x1b[m �Ҧb���u�@���^�Сy�����{�ҫH��z�C",
                       cuser.userid, cuser.username, addr);
                msg = NULL;
            }
        }
    }
    vmsg(msg);
    return 0;
}

static const char *const UFO_FLAGS[] = {
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,

    /* PAGER */ "�����I�s��",
    /* QUITE */ "�����T��",
    /* MAXMSG */ "�T���W���ڦ��T��",
    /* FORWARD */ "�۰���H",
    /* CLASSTABLE */ ATTR_CONF_STR("�Ҫ�ɨ�q��", HAVE_CLASSTABLEALERT),
    /* MPAGER */ "�q�l�l��ǩI",
    NULL,
    NULL,
    NULL,
    /* REJECT */ "�ڦ��s��",
    NULL,
    NULL,
    NULL,
    /* HIDDEN */ "���èӷ�",

    /* CLOAK */ "�����N",
    NULL
};

static const char *const UFO2_FLAGS[] = {
    /* COLOR */ "�m��Ҧ�",
    /* MOVIE */ "�ʵe���",
    /* BRDNEW */ "�s����",
    /* BNOTE */ "��ܶi�O�e��",
    /* VEDIT */ "²�ƽs�边",
    /* PAL */ "�u��ܦn��",
    /* MOTD */ "²�ƶi���e��",
    /* MIME */ ATTR_CONF_STR("MIME �ѽX", HAVE_MIME_TRANSFER),
    /* SIGN */ "���ñ�W�� �}��:�� ����:�e",
    /* SHOWUSER */ "��ܦۤv ID �M�ʺ�",
    /* PRH */ ATTR_CONF_STR("�������ˤ峹����", HAVE_RECOMMEND),
    /* SHIP */ "�n�ʹy�z",
    /* NWLOG */ "���x�s���T����",
    /* NTLOG */ "���x�s��Ѭ���",
    /* CIRCLE */ "�`���\\Ū",
    /* ORIGUI */ "�����W������",
    /* DEF_ANONY */ "�w�]���ΦW",
    /* DEF_LEAVE */ "�w�]������",
    /* REPLY */ "�O�����y��T",
    /* DEF_LOCALMAIL */ "�u�������H",
    /* LIGHTBAR */ ATTR_CONF_STR("�ϥΥ��ο��t��", HAVE_MENU_LIGHTBAR),
    /* RESERVE */ NULL,
    /* RESERVE */ NULL,
    /* RESERVE */ NULL,
    /* ACL */ "ACL",
    /* RESERVE */ NULL,
    /* RESERVE */ NULL,
    /* RESERVE */ NULL,
    /* REALNAME */ "�u��m�W",
    /* RESERVE */ NULL,
    /* RESERVE */ NULL,
    /* RESERVE */ NULL
};


void su_setup(ACCT * u)
{
    int ufo, nflag, len;
    char fpath[80];
    UTMP *up;
    const char *const *flags = UFO_FLAGS;

    up = utmp_find(u->userno);
    len = 21;
    ufo = u->ufo;
    nflag = bitset(ufo, len, len, "�ާ@�Ҧ��]�w�G", flags);
    if (nflag != ufo)
    {
        if (up)
        {
            nflag = (nflag & ~UFO_UTMP_MASK) | (up->ufo & UFO_UTMP_MASK);
            up->ufo = u->ufo = nflag;
        }
        else
            u->ufo = nflag;
        //      showansi = nflag & UFO_COLOR;
        outs(str_ransi);
    }
    usr_fpath(fpath, u->userid, FN_FORWARD);
    if (u->ufo & UFO_FORWARD)
    {
        FILE *fd;
        fd = fopen(fpath, "w");
        fclose(fd);
    }
    else
        unlink(fpath);

}

int u_setup(void)
{
    int ufo, nflag, len;
    char fpath[80];

    const char *const *flags = UFO_FLAGS;

    nflag = cuser.userlevel;
    if (!nflag)
        len = 5;
    else if (nflag & (PERM_SYSOP | PERM_BOARD | PERM_ACCOUNTS | PERM_CHATROOM))
        len = 21;
    /* Thor.980910: �ݪ`�N��PERM_ADMIN���F�i��acl, �ٶ��K�]�i�H�������N�F:P */
    else if (nflag & PERM_CLOAK)
        len = 20;
    else
        len = 18;                /* lkchu.990428: �[�F �q�l�l��ǩI */

    ufo = cuser.ufo;
    nflag = bitset(ufo, len, len, "�ާ@�Ҧ��]�w�G", flags);
    if (nflag != ufo)
    {
        /* Thor.980805: �ѨM ufo BIFF���P�B���D */
        nflag = (nflag & ~UFO_UTMP_MASK) | (cutmp->ufo & UFO_UTMP_MASK);

        cutmp->ufo = cuser.ufo = nflag;
        /* Thor.980805: �n�S�O�`�N cuser.ufo�Mcutmp->ufo��UFO_BIFF���P�B���D, �A�� */

        //      showansi = nflag & UFO_COLOR;
        outs(str_ransi);
    }
    usr_fpath(fpath, cuser.userid, FN_FORWARD);
    if (cuser.ufo & UFO_FORWARD)
    {
        FILE *fd;
        fd = fopen(fpath, "w");
        fclose(fd);
    }
    else
        unlink(fpath);

    return 0;
}

int ue_setup(void)
{
    int ufo2, nflag, len;

    const char *const *flags = UFO2_FLAGS;

    nflag = cuser.userlevel;
    if (!nflag)
        len = 5;
    else if (nflag & (PERM_SYSOP | PERM_ACCOUNTS))
        len = 32;
    else if (nflag & PERM_ADMIN)
        len = 28;
    else
        len = 24;

    ufo2 = cuser.ufo2;
    nflag = bitset(ufo2, len, len, "�߷R�\\��]�w�G", flags);
    if (nflag != ufo2)
    {
        cuser.ufo2 = nflag;
        showansi = nflag & UFO2_COLOR;
        outs(str_ransi);
#ifdef M3_USE_PFTERM
        redrawwin();
        refresh();
#endif
    }
    return 0;
}

int u_lock(void)
{
    char buf[PLAINPASSSIZE];
    char swapmateid[IDLEN + 1] = "";
    const char IdleState[][IDLEN] = {
        "�۱j�[�P",
        "�ӫ��{",
        "���_���y",
        "�O�汴�I",
        "���\\���B",
        "�q�~�A�v"
    };

    strcpy(swapmateid, cutmp->mateid);
    vget(B_LINES_REF - 1, 0,
         "�z��:[0]�o�b (1)���q�� (2)�V�� (3)���O�� (4)�˦� (5)���� (6)��L (Q)�S��:",
         buf, 2, DOECHO);

    if (buf[0] <= '5' && buf[0] >= '0')
    {
        strcpy(cutmp->mateid, IdleState[buf[0] - '0']);
    }
    else if (buf[0] == '6')
    {
        vget(B_LINES_REF - 1, 0, "�o�b���z��:", cutmp->mateid, IDLEN, DOECHO);
    }
    else if (buf[0] == 'q' || buf[0] == 'Q')
    {
        strcpy(cutmp->mateid, swapmateid);
        return XEASY;
    }
    else
    {
        strcpy(cutmp->mateid, IdleState[0]);
    }
    cutmp->ufo |= UFO_REJECT;
    utmp_mode(M_IDLE);

    buf[0] = 'n';
    if (cuser.userlevel)
        vget(B_LINES_REF - 1, 0, "�O�_�n�i�J�ù���w���A(y/N)?[N]", buf, 2,
             DOECHO);

    clear();
    prints("\x1b[1;44;33m");
    vs_mid(BOARDNAME "    ���m/��w���A");
    move(4, 6);
    prints("���m���G%s", cutmp->mateid);
    if (buf[0] == 'y' || buf[0] == 'Y')
    {
        int check;
        blog("LOCK", "screen");
        bbstate |= STAT_LOCK;    /* lkchu.990513: ��w�ɤ��i�^�T�� */
        check = 0;
        do
        {
            vget(7, 0, "�� �п�J�K�X�A�H�Ѱ��ù���w�G", buf, PLAINPASSSIZE, NOECHO | VGET_STEALTH_NOECHO);
            check = chkpasswd(cuser.passwd, cuser.passhash, buf);
            if (check)
            {
                char fpath[80];
                char temp[80];
                usr_fpath(fpath, cuser.userid, FN_LOGINS_BAD);
                sprintf(temp, "[%s] BBS %s\n", Ctime(&ap_start), fromhost);
                f_cat(fpath, temp);
            }
        }
        while (check);
    }
    else
    {
        vkey();
    }

    strcpy(cutmp->mateid, swapmateid);
    bbstate ^= STAT_LOCK;
    cutmp->ufo &= ~UFO_REJECT;
    return 0;
}

int u_xfile(void)
{
    int i;

    static const char *const desc[] = {
        "�W���a�I�]�w��",
        "�W����",
        "ñ�W�� (�C�� 6 ��A�@ 3 ��)",
        "�Ȧs��.1",
        "�Ȧs��.2",
        "�Ȧs��.3",
        "�Ȧs��.4",
        "�Ȧs��.5",
        NULL
    };

    static const char *const path[] = {
        "acl",
        FN_PLANS,
        FN_SIGN,
        "buf.1",
        "buf.2",
        "buf.3",
        "buf.4",
        "buf.5"
    };

    i = (HAS_PERM(PERM_ADMIN)) ? 0 : 1;
    x_file(M_UFILES, &desc[i], &path[i]);
    return 0;
}


/* ----------------------------------------------------- */
/* �ݪO�޲z                                              */
/* ----------------------------------------------------- */


GCC_PURE static bool valid_brdname(const char *brd)
{
    int ch;

    if (!is_alnum(*brd))
        return false;

    while ((ch = *++brd))
    {
        if (!is_alnum(ch) && ch != '.' && ch != '-' && ch != '_')
            return false;
    }
    return true;
}

static int m_setbrd(BRD * brd)
{
    int i;
    char *data, buf[16], old_brdname[IDLEN + 1];
    FILE *fp;
    char fpath[80];

    if (!str_casecmp(cuser.userid, ELDER))
        pmsg2("�ק�ʧ@���[�J��x");
    else
    {
        char tmp[80], why[80];
        pmsg2("�ק�ʧ@�w�[�J������x");
        if (!vget(B_LINES_REF, 0, "�п�J�ק�z�ѡG", why, 40, DOECHO))
        {
            sprintf(why, "����J�z�ѡA�T��ק�");
            pmsg2("�п�J�ק�z��");
            return 0;
        }
        sprintf(tmp, "\n\n%s %-12s ��ݪO %-12s ����ק�ʧ@\n�z��: ", Now(),
                cuser.userid, brd->brdname);
        f_cat(FN_BLACKSU_LOG, tmp);
        f_cat(FN_BLACKSU_LOG, why);
    }

    data = brd->brdname;
    i = *data ? 11 : 1;
    strcpy(old_brdname, data);

    for (;;)
    {
        if (!vget(i, 0, MSG_BID, data, IDLEN + 1, GCARRY))
        {
            if (i == 1)
                return -1;

            strcpy(data, old_brdname);    /* Thor:�Y�O�M�ūh�]����W�� */
            continue;
        }

        if (!strcmp(old_brdname, data) && valid_brdname(data))
        {                        /* Thor: �P��W�P�h���L */
            break;
        }

        if (brd_bno(data) >= 0)
        {
            outs("\n���~! �O�W�p�P");
        }
        else if (valid_brdname(data))
        {
            break;
        }
    }

    data = buf;
    vget(++i, 0, "�ݪO�D�D�G", brd->title, BTLEN + 1, GCARRY);
    vget(++i, 0, "���O�G", brd->class_, 5, GCARRY);
    sprintf(data, "%02d", brd->color);
    vget(++i, 0, "�C��榡 X.�G��(0~1) Y.�C��(0~7) [XY]�G", data, 3, GCARRY);
    if (data[0] < '0' || data[0] > '1')
        data[0] = '0';
    if (data[1] < '0' || data[1] > '7')
        data[1] = '7';
    brd->color = (char)atoi(data);
    vget(++i, 0, "�O�D�W��G", brd->BM, BMLEN + 1, GCARRY);

    sprintf(data, "%u", brd->expiremax);
    vget(++i, 0, "�̤j�峹�ƶq ( [0] ���w�])�G", data, 6, GCARRY);
    brd->expiremax = atoi(data);

    sprintf(data, "%u", brd->expiremin);
    vget(++i, 0, "�̤p�峹�ƶq ( [0] ���w�])�G", data, 6, GCARRY);
    brd->expiremin = atoi(data);

    sprintf(data, "%u", brd->expireday);
    vget(++i, 0, "�峹�O�d�Ѽ� ( [0] ���w�])�G", data, 6, GCARRY);
    brd->expireday = atoi(data);

#ifdef HAVE_MODERATED_BOARD
    /* cache.091124: ��θ��K�Q���ݪO�v���]�w */
    switch (vget
            (++i, 0, "�ݪO�v�� A)�@�� B)�۩w C)���K D)�n�͡H[Q] ", buf, 3,
             LCECHO))
    {
    case 'c':
        brd->readlevel = (PERM_SYSOP | PERM_BOARD);    /* ���K�ݪO */
        brd->postlevel = PERM_POST;
        brd->battr |= (BRD_NOSTAT | BRD_NOVOTE);
        break;

    case 'd':
        brd->readlevel = PERM_SYSOP;    /* �n�ͬݪO */
        brd->postlevel = PERM_POST;
        brd->battr |= (BRD_NOSTAT | BRD_NOVOTE);
        break;
#else
    switch (vget(++i, 0, "�ݪO�v�� A)�@�� B)�۩w�H[Q] ", buf, 3, LCECHO))
    {
#endif

    case 'a':
        brd->readlevel = 0;
        brd->postlevel = PERM_POST;    /* �@��ݪO�o���v���� PERM_POST */
        brd->battr &= ~(BRD_NOSTAT | BRD_NOVOTE);    /* �����n�͡����K�O�ݩ� */
        break;

    case 'b':
        if (vget(++i, 0, "�\\Ū�v��(y/N)�H[N] ", buf, 3, LCECHO) == 'y')
        {
            brd->readlevel =
                bitset(brd->readlevel, NUMPERMS, NUMPERMS, MSG_READPERM,
                       perm_tbl);
            move(2, 0);
            clrtobot();
            i = 1;
        }

        if (vget(++i, 0, "�o���v��(y/N)�H[N] ", buf, 3, LCECHO) == 'y')
        {
            brd->postlevel =
                bitset(brd->postlevel, NUMPERMS, NUMPERMS, MSG_POSTPERM,
                       perm_tbl);
            move(2, 0);
            clrtobot();
            i = 1;
        }
        break;

    default:                    /* �w�]���ܰ� */
        break;
    }

    if (vget(++i, 0, "�]�w�ݩ�(y/N)�H[N] ", data, 3, LCECHO) == 'y')
    {
        brd->battr =
            bitset(brd->battr, NUMATTRS, NUMATTRS, MSG_BRDATTR, battrs);
    }
    brd_fpath(fpath, brd->brdname, CHECK_BAN);

    /* cache.090928 �ݪO�����ݩ� - ������M�۩w���N��(�w�]) */
    if ((brd->battr & BRD_PUSHSNEER) && (brd->battr & BRD_PUSHDEFINE))
        brd->battr &= ~BRD_PUSHSNEER;

    /* cache.090928 �ݪO�����ݩ� - ��ݪO��Ū�ɤ]�T����� */
    if (brd->battr & BRD_NOREPLY)
        brd->battr |= BRD_PRH;

    /* cache.090928 �ݪO�����ݩ� - ���i����ɲM���Ҧ�����Ҧ��M���� */
    if (brd->battr & BRD_PRH)
    {
        brd->battr &= ~BRD_PUSHDISCON;
        brd->battr &= ~BRD_PUSHTIME;
        brd->battr &= ~BRD_PUSHSNEER;
        brd->battr &= ~BRD_PUSHDEFINE;
    }

    if (brd->battr & BRD_NOBAN)
    {
        fp = fopen(fpath, "w+");
        if (fp)
            fclose(fp);
    }
    else
        unlink(fpath);
    return 0;
}


int m_newbrd(void)
{
    BRD newboard;
    int bno;
    char fpath[80];
    HDR hdr;

    vs_bar("�إ߷s�O");
    memset(&newboard, 0, sizeof(newboard));
    strcpy(newboard.title, "������������������");
    if (m_setbrd(&newboard))
        return -1;

    if (vans(msg_sure_ny) != 'y')
        return 0;

    time(&newboard.bstamp);
    if ((bno = brd_bno("")) >= 0)
    {
        rec_put(FN_BRD, &newboard, sizeof(newboard), bno);
    }
    /* Thor.981102: ����W�Lshm�ݪO�Ӽ� */
    else if (bshm->number >= MAXBOARD)
    {
        vmsg("�W�L�t�Ωү�e�Ǭݪ��ӼơA�нվ�t�ΰѼ�");
        return -1;
    }
    else if (rec_add(FN_BRD, &newboard, sizeof(newboard)) < 0)
    {
        vmsg("�L�k�إ߷s�O");
        return -1;
    }

    sprintf(fpath, "gem/brd/%s", newboard.brdname);
    mak_dirs(fpath);
    mak_dirs(fpath + 4);

    bshm->uptime = 0;            /* force reload of bcache */
    bshm_init();

    /* ���K�[�i NewBoard */

    if (vans("�O�_�[�J [NewBoard] �s��(Y/n)?[Y] ") != 'n')
    {
        brd2gem(&newboard, &hdr);
        rec_add("gem/@/@NewBoard", &hdr, sizeof(HDR));
    }

    vmsg("�s�O����");
    return 0;
}

void brd_edit(int bno)
{
    BRD *bhdr, newbh;
    char buf[80];

    vs_bar("�ݪO�]�w");
    bhdr = bshm->bcache + bno;
    memcpy(&newbh, bhdr, sizeof(BRD));
    prints("�ݪO�W�١G%s\n�ݪO�����G%s\n�O�D�W��G%s\n",
           newbh.brdname, newbh.title, newbh.BM);
    prints("�ݪO���O�G[%4s] ���O�C��G%d\n", newbh.class_, newbh.color);
    prints("�峹�ƥءG[�̤j] %u [�̤p] %u [�Ѽ�] %u\n", newbh.expiremax,
           newbh.expiremin, newbh.expireday);

    bitmsg(MSG_READPERM, STR_PERM, newbh.readlevel);
    bitmsg(MSG_POSTPERM, STR_PERM, newbh.postlevel);
    bitmsg(MSG_BRDATTR, "zTcsvAPEblpRLWiePSt", newbh.battr);

    switch (vget
            (9, 0, "(D)�R�� (E)�]�w (V)�w�]�v�� (Q)�����H[Q] ", buf, 3,
             LCECHO))
    {
    case 'v':
        bhdr->postlevel |= PERM_VALID;
        rec_put(FN_BRD, bhdr, sizeof(BRD), bno);
        vmsg("�]�w����");
        break;
    case 'd':

        if (vget(10, 0, msg_sure_ny, buf, 3, LCECHO) != 'y')
        {
            vmsg(MSG_DEL_CANCEL);
        }
        else
        {
            char *bname = bhdr->brdname;
            /* �H�K�y��*bname��NULL�ɡA�|��� gem/brd and brd�C statue.000728 */
            if (*bname)
            {
                sprintf(buf, "gem/brd/%s", bname);
                //f_rm(buf);
                //f_rm(buf + 4);
                /* 100721.cache: f_rm is buggy... */
                PROC_CMD("/bin/rm", "-rf", buf);
                PROC_CMD("/bin/rm", "-rf", buf + 4);
                memset(&newbh, 0, sizeof(newbh));
                sprintf(newbh.title, "[%s] deleted by %s", bname,
                        cuser.userid);
                memcpy(bhdr, &newbh, sizeof(BRD));
                rec_put(FN_BRD, &newbh, sizeof(newbh), bno);
                blog("Admin", newbh.title);
                vmsg("�R�O����");
            }
        }
        break;

    case 'e':

        move(9, 0);
        clrtoeol();
        outs("������ [Return] ���ק�Ӷ��]�w");

        if (!m_setbrd(&newbh))
        {
            if ((vans(msg_sure_ny) == 'y') &&
                memcmp(&newbh, bhdr, sizeof(newbh)))
            {
                if (strcmp(bhdr->brdname, newbh.brdname))
                {
                    char src[80], dst[80];
                    /* Thor.980806: �S�O�`�N�p�G board���b�P�@partition�ت��ܷ|�����D */
                    sprintf(src, "gem/brd/%s", bhdr->brdname);
                    sprintf(dst, "gem/brd/%s", newbh.brdname);
                    rename(src, dst);
                    rename(src + 4, dst + 4);
                }
                memcpy(bhdr, &newbh, sizeof(BRD));
                rec_put(FN_BRD, &newbh, sizeof(BRD), bno);
            }
        }
        vmsg("�]�w����");
        break;
    }
}

int a_editbrd(void)                /* cache.100618: �ק�ݪO�ﶵ */
{
    int bno;
    BRD *brd;
    char bname[IDLEN + 1];

    if ((brd = ask_board(bname, BRD_R_BIT, NULL)))
    {
        bno = brd - bshm->bcache;
        brd_edit(bno);
    }
    else
    {
        vmsg("�䤣��o�ӬݪO");
    }

    return 0;
}

#ifdef  HAVE_REGISTER_FORM
/* ----------------------------------------------------- */
/* �ϥΪ̶�g���U���                                    */
/* ----------------------------------------------------- */


static void getfield(int line, int len, char *buf, const char *desc, const char *hint)
{
    move(line, 0);
    prints("%s:%s\n", desc, hint);
    vget(line + 1, 0, "         ", buf, len, GCARRY);
}

int check_idno(const char *s)
{
    const char *p, *const LEAD = "ABCDEFGHJKLMNPQRSTUVXYWZIO";
    int x, i;

    if (strlen(s) != 10 || (p = strchr(LEAD, toupper(*s))) == NULL)
        return 0;
    x = p - LEAD;
    x = x / 10 + x % 10 * 9;
    p = s + 1;
    if (*p != '1' && *p != '2')
        return 0;
    for (i = 1; i < 9; i++)
    {
        if (isdigit(*p))
            x += (*p++ - '0') * (9 - i);
        else
            return 0;
    }
    x = 9 - x % 10;
    return (x == *p - '0');
}                                /* CheckID */

#ifndef HAVE_SIMPLE_RFORM
static void send_request(void)
{
    RFORM_R form = {0};
    int check;

    check = REG_SENT;

    strcpy(form.userid, cuser.userid);
    form.userno = cuser.userno;
    if (!vget(B_LINES_REF, 0, "�п�J��] :", form.msg, 80, DOECHO))
    {
        vmsg("�e�楢��");
        return;
    }
    rec_add(FN_RFORM_R, &form, sizeof(RFORM_R));
    attr_put(cuser.userid, ATTR_REG_KEY, &check);
    vmsg("�e�榨�\\");
}

#endif

int u_register(void)
{
    FILE *fn;
    int ans;
    RFORM rform;
#ifndef HAVE_SIMPLE_RFORM
    int formstate = 0;

    if (attr_get(cuser.userid, ATTR_REG_KEY, &formstate) >= 0)
    {
        if (!(formstate))
        {
            vmsg("�е��� POP3 �� E-mail �{�Ҥ覡");
            return 0;
        }
        else if (formstate == REG_REQUEST)
        {
            if (vans("�O�_�ШD���U��ӽФ覡 ? [N]") == 'y')
                send_request();
            return 0;
        }
        else if (formstate == REG_SENT)
        {
            vmsg("�w�g�ШD�L�F�A�Э@�ߵ��� !");
            return 0;
        }
        else if (formstate == REG_FAULT)
        {
            vmsg("�z����ϥε��U�� !");
            return 0;
        }
    }
    else
    {
        vmsg("�е��� POP3 �� E-mail �{�Ҥ覡");
        return 0;
    }
#endif  /* #ifndef HAVE_SIMPLE_RFORM */

    if (HAS_PERM(PERM_VALID))
    {
        zmsg("�z�������T�{�w�g�����A���ݶ�g�ӽЪ�");
        return XEASY;
    }

    if ((fn = fopen(FN_RFORM, "rb")))
    {
        while (fread(&rform, sizeof(RFORM), 1, fn))
        {
            if ((rform.userno == cuser.userno) &&
                !str_casecmp(rform.userid, cuser.userid))
            {
                fclose(fn);
                zmsg("�z�����U�ӽг�|�b�B�z���A�Э@�ߵ���");
                return XEASY;
            }
        }
        fclose(fn);
    }

    if (vans("�z�T�w�n��g���U���(y/N)�H[N] ") != 'y')
        return XEASY;

    clear();
    more(FN_ETC_RFORM, (char *)-1);
    for (ans = 0; ans <= 1; ans++)
    {
        char msg[128];
        sprintf(msg, "%s�A�@�N��u���W�w�� ? [N]",
                ans ? "�ЦA�ԲӬݤ@�M�A" : "");
        if (vans(msg) != 'y')
            return 0;
        bell();
    }
    move(2, 0);
    clrtobot();
    prints("%s(%s) �z�n�A�оڹ��g�H�U����ơG\n(�� [Enter] ������l�]�w)",
           cuser.userid, cuser.username);

    memset(&rform, 0, sizeof(RFORM));
    strcpy(rform.realname, cuser.realname);
    strcpy(rform.address, cuser.address);
    rform.career[0] = rform.phone[0] = '\0';
    for (;;)
    {
        getfield(5, 20, rform.realname, "�u��m�W", "�ХΤ���");
        getfield(7, 50, rform.career, "�A�ȳ��", "�Ǯըt�ũγ��¾��");
        getfield(9, 60, rform.address, "�ثe��}", "�]�A��ǩΪ��P���X");
        getfield(11, 20, rform.phone, "�s���q��", "�]�A���~�����ϰ�X");
        getfield(13, 11, rform.idno, "�����Ҹ��X", "�иԲӶ�g");
        ans = vans("�H�W��ƬO�_���T(y/N)�H(q)���� [N] ");
        if (ans == 'q')
            return 0;
        if (ans == 'y')
            break;
    }
    strcpy(cuser.realname, rform.realname);
    strcpy(cuser.address, rform.address);

    rform.userno = cuser.userno;
    strcpy(rform.userid, cuser.userid);
    (void)time(&rform.rtime);
    rec_add(FN_RFORM, &rform, sizeof(RFORM));
    return 0;
}


/* ----------------------------------------------------- */
/* �B�z Register Form                                    */
/* ----------------------------------------------------- */


static int scan_register_form(int fd)
{
    static const char logfile[] = FN_RFORM_LOG;
    static const char *const reason[] = { "��J�u��m�W", "�Թ��g�ӽЪ�",
        "�Զ��}���", "�Զ�s���q��", "�Զ�A�ȳ��B�ξǮըt��",
        "�Τ����g�ӽг�", "�ĥ� E-mail �{��", "��g�����Ҹ��X", NULL
    };

    ACCT muser;
    RFORM rform;
    HDR fhdr;
    FILE *fout;

    int op, n;
    char buf[128], msg[128], *agent, *userid, *str;

    vs_bar("�f�֨ϥΪ̵��U���");
    agent = cuser.userid;

    while (read(fd, &rform, sizeof(RFORM)) == sizeof(RFORM))
    {
        userid = rform.userid;
        move(2, 0);
        prints("�ӽХN��: %s (�ӽЮɶ��G%s)\n", userid, Ctime(&rform.rtime));
        prints("�u��m�W: %s    �����Ҹ��X: %s(%s)\n", rform.realname,
               rform.idno, check_idno(rform.idno) ? "���T" : "���~");
        prints("�A�ȳ��: %s\n", rform.career);
        prints("�ثe��}: %s\n", rform.address);
        prints("�s���q��: %s\n", rform.phone);
        outsep(b_cols, msg_separator);
        outc('\n');
        clrtobot();

        if ((acct_load(&muser, userid) < 0) || (muser.userno != rform.userno))
        {
            vmsg("�d�L���H");
            op = 'd';
        }
        else
        {
            acct_show(&muser, 2);
            if (muser.userlevel & PERM_VALID)
            {
                vmsg("���b���w�g�������U");
                op = 'd';
            }
            else
            {
                op = vans("�O�_����(Y/N/Q/Del/Skip)�H[S] ");
            }
        }

        switch (op)
        {
        case 'y':

            muser.userlevel |= PERM_VALID;
            strcpy(muser.realname, rform.realname);
            strcpy(muser.address, rform.address);
            sprintf(muser.email, "%s.bbs@%s", muser.userid, MYHOSTNAME);
            strcpy(muser.vmail, muser.email);
            sprintf(msg, "reg:%s:%s:%s", rform.phone, rform.career, agent);
            str_scpy(muser.justify, msg, sizeof(muser.justify));
            /* Thor.980921: �O�I�_�� */

            /* Thor.981022: ��ʻ{�Ҥ]��{�Үɶ�, �C�b�~�|�A�۰ʻ{�Ҥ@�� */
            time(&muser.tvalid);

            acct_save(&muser);

            usr_fpath(buf, userid, FN_JUSTIFY);
            if ((fout = fopen(buf, "a+")))
            {
                fprintf(fout, "%s\n", msg);
                fclose(fout);
            }

            usr_fpath(buf, userid, fn_dir);
            hdr_stamp(buf, HDR_LINK, &fhdr, FN_ETC_APPROVED);
            strcpy(fhdr.title, "[���U���\\] �z�w�g�q�L�����{�ҤF�I");
            strcpy(fhdr.owner, cuser.userid);
            rec_add(buf, &fhdr, sizeof(fhdr));

            strcpy(rform.agent, agent);
            rec_add(logfile, &rform, sizeof(RFORM));

            break;

        case 'q':                /* �Ӳ֤F�A������ */

            do
            {
                rec_add(FN_RFORM, &rform, sizeof(RFORM));
            }
            while (read(fd, &rform, sizeof(RFORM)) == sizeof(RFORM));

        case 'd':
            break;

        case 'n':

            move(9, 0);
            prints("�д��X�h�^�ӽЪ��]�A�� <enter> ����\n\n");
            for (n = 0; (str = reason[n]); n++)
                prints("%d) ��%s\n", n, str);
            clrtobot();

            if ((op = vget(B_LINES_REF, 0, "�h�^��]�G", buf, 60, DOECHO)))
            {
                int i;
                char folder[80], fpath[80], boardbuf[IDLEN + 1];
                HDR fhdr;

                i = op - '0';
                if (i >= 0 && i < n)
                    strcpy(buf, reason[i]);

                usr_fpath(folder, muser.userid, fn_dir);
                if ((fout = fdopen(hdr_stamp(folder, 0, &fhdr, fpath), "w")))
                {
                    strcpy(ve_title, "[" SYSOPNICK "] �бz���s��g���U���");
                    strcpy(boardbuf, currboard);
                    *currboard = 0;
                    ve_header(fout);
                    *ve_title = 0;
                    strcpy(currboard, boardbuf);

                    fprintf(fout, "\t�ѩ�z���Ѫ���Ƥ����Թ�A�L�k�T�{�����A"
                            "\n\n\t�Э��s��g���U���G%s�C\n", buf);
                    fclose(fout);

                    strcpy(fhdr.owner, agent);
                    strcpy(fhdr.title, "[" SYSOPNICK "] �бz���s��g���U���");
                    rec_add(folder, &fhdr, sizeof(fhdr));
                }

                strcpy(rform.reply, buf);    /* �z�� */
                strcpy(rform.agent, agent);
                rec_add(logfile, &rform, sizeof(RFORM));

                break;
            }

        default:                /* put back to regfile */

            rec_add(FN_RFORM, &rform, sizeof(RFORM));
        }
    }
    return 0;
}

#ifndef HAVE_SIMPLE_RFORM
static int ans_request(void)
{
    int num, fd;
    RFORM_R form;
    ACCT muser;
    char fpath[128], op, buf[128];
    sprintf(fpath, "%s.tmp", FN_RFORM_R);

    rename(FN_RFORM_R, fpath);
    num = rec_num(fpath, sizeof(RFORM_R));
    if (num <= 0)
    {
        zmsg("�ثe�õL�n�D���U���ϥΪ�");
        return XEASY;
    }
    vs_bar("�f�ֵ��U��ШD");
    if ((fd = open(fpath, O_RDONLY)) >= 0)
    {
        while (read(fd, &form, sizeof(RFORM_R)) == sizeof(RFORM_R))
        {
            move(2, 0);
            prints("�ӽХN��: %s\n", form.userid);
            prints("�ӽвz��: %s\n", form.msg);
            outsep(b_cols, msg_separator);
            outc('\n');
            clrtobot();
            if ((acct_load(&muser, form.userid) < 0)
                || (muser.userno != form.userno))
            {
                vmsg("�d�L���H");
                op = 'd';
            }
            else
            {
                acct_show(&muser, 2);
                if (muser.userlevel & PERM_VALID)
                {
                    vmsg("���b���w�g�������U");
                    op = 'd';
                }
                else
                {
                    op = vans("�O�_����(yes/no/quit/Skip)�H");
                }
            }
            switch (op)
            {
            case 'd':
                break;
            case 'y':
            case 'Y':
                num = REG_OPEN;
                attr_put(muser.userid, ATTR_REG_KEY, &num);
                break;
            case 'n':
            case 'N':
                move(9, 0);
                prints("�д��X�h�^�ӽЪ��]�A�� <enter> ����\n\n");
                clrtobot();

                if ((op = vget(B_LINES_REF, 0, "�h�^��]�G", buf, 60, DOECHO)))
                {
                    char folder[80], fpath[80], boardbuf[IDLEN + 1];
                    HDR fhdr;
                    FILE *fout;

                    usr_fpath(folder, muser.userid, fn_dir);
                    if ((fout =
                         fdopen(hdr_stamp(folder, 0, &fhdr, fpath), "w")))
                    {
                        strcpy(ve_title, "[" SYSOPNICK "]���U���ӽаh��");
                        strcpy(boardbuf, currboard);
                        *currboard = 0;
                        ve_header(fout);
                        *ve_title = 0;
                        strcpy(currboard, boardbuf);

                        fprintf(fout, "\n\n\t�h�^��]�G%s�C\n", buf);
                        fclose(fout);

                        strcpy(fhdr.owner, cuser.userid);
                        strcpy(fhdr.title, "[" SYSOPNICK "] ���U���ӽаh��");
                        rec_add(folder, &fhdr, sizeof(fhdr));
                    }
                    num = REG_FAULT;
                    attr_put(muser.userid, ATTR_REG_KEY, &num);
                }
                else
                    rec_add(FN_RFORM_R, &form, sizeof(RFORM_R));

                break;
            case 'q':
            case 'Q':
                do
                {
                    rec_add(FN_RFORM_R, &form, sizeof(RFORM_R));
                }
                while (read(fd, &form, sizeof(RFORM_R)) == sizeof(RFORM_R));
                break;
            default:
                rec_add(FN_RFORM_R, &form, sizeof(RFORM_R));
            }
        }
        close(fd);
        unlink(fpath);
    }

    return 0;
}
#endif  /* #ifndef HAVE_SIMPLE_RFORM */

int m_register(void)
{
    int num;
#ifndef HAVE_SIMPLE_RFORM
    int num2;
    char msg[128];
#endif
    char buf[80];

    num = rec_num(FN_RFORM, sizeof(RFORM));
#ifndef HAVE_SIMPLE_RFORM
    num2 = rec_num(FN_RFORM_R, sizeof(RFORM_R));
    sprintf(msg, "�f�� : 1)���U��< %d ��> 2)���U��ШD< %d ��> [1]", num,
            num2);
#endif

#ifndef HAVE_SIMPLE_RFORM
    if (vans(msg) == '2')
    {
        return ans_request();
    }
#endif
    if (num <= 0)
    {
        clrtoeol();
        zmsg("�ثe�õL�s���U���");
        return XEASY;
    }

    sprintf(buf, "�@�� %d ����ơA�}�l�f�ֶ�(y/N)�H[N] ", num);
    num = XEASY;

    if (vans(buf) == 'y')
    {
        sprintf(buf, "%s.tmp", FN_RFORM);
        if (dashf(buf))
        {
            vmsg("��L SYSOP �]�b�f�ֵ��U�ӽг�");
            if (vans("�O�_�ϥέץ��\\��H") == 'y')
            {
                clear();
                prints
                    ("\n\n\x1b[1;33m�нT�w�S����L���Ȧb�f�֡A�_�h�N�y��\x1b[1;31;5m�ϥΪ̸���Y�����~!\x1b[m\n\n\n");
                if (vans("�T�w�L��L���ȼf�֤��H") == 'y')
                {
                    system("/bin/cat run/" FN_RFORM ".tmp >> run/" FN_RFORM);
                    PROC_CMD("/bin/rm", "-f", BBS_HOME "/run/" FN_RFORM ".tmp");
                    vmsg("�ץ������A�U���Фp�߼f��! �����N�䭫�s�}�l.");
                }
            }
        }
        else
        {
            int fd;

            rename(FN_RFORM, buf);
            fd = open(buf, O_RDONLY);
            if (fd >= 0)
            {
                scan_register_form(fd);
                close(fd);
                unlink(buf);
                num = 0;
            }
            else
            {
                vmsg("�L�k�}�ҵ��U��Ƥu�@��");
            }
        }
    }
    return num;
}
#endif  /* #ifdef  HAVE_REGISTER_FORM */


/* ----------------------------------------------------- */
/* ���Ͱl�ܰO���G��ĳ��� log_usies()�Bblog()            */
/* ----------------------------------------------------- */


#ifdef  HAVE_REPORT
void report(const char *s)
{
    static bool disable = false;
    int fd;

    if (disable)
        return;

    if ((fd = open("trace", O_WRONLY, 0600)) != -1)
    {
        char buf[256];
        char *thetime;
        time_t dtime;

        time(&dtime);
        thetime = Etime(&dtime);

        /* flock(fd, LOCK_EX); */
        /* Thor.981205: �� fcntl ���Nflock, POSIX�зǥΪk */
        f_exlock(fd);

        lseek(fd, 0, L_XTND);
        sprintf(buf, "%s %s %s\n", cuser.userid, thetime, s);
        write(fd, buf, strlen(buf));

        /* flock(fd, LOCK_UN); */
        /* Thor.981205: �� fcntl ���Nflock, POSIX�зǥΪk */
        f_unlock(fd);

        close(fd);
    }
    else
        disable = true;
}


int m_trace(void)
{
    struct stat bstatb, ostatb, cstatb;
    int btflag, otflag, ctflag, done = 0;
    char ans[2];
    char *msg;

    clear();
    move(0, 0);
    outs("Set Trace Options");
    clrtobot();
    while (!done)
    {
        move(2, 0);
        otflag = stat("trace", &ostatb);
        ctflag = stat("trace.chatd", &cstatb);
        btflag = stat("trace.bvote", &bstatb);
        outs("Current Trace Settings:\n");
        if (otflag)
            outs("Normal tracing is OFF\n");
        else
            prints("Normal tracing is ON (size = %d)\n", ostatb.st_size);
        if (ctflag)
            outs("Chatd  tracing is OFF\n");
        else
            prints("Chatd  tracing is ON (size = %d)\n", cstatb.st_size);
        if (btflag)
            outs("BVote  tracing is OFF\n");
        else
            prints("BVote  tracing is ON (size = %d)\n", bstatb.st_size);

        move(8, 0);
        outs("Enter:\n");
        prints("<1> to %s Normal tracing\n", otflag ? "enable " : "disable");
        prints("<2> to %s Chatd  tracing\n", ctflag ? "enable " : "disable");
        prints("<3> to %s BVote  tracing\n", btflag ? "enable " : "disable");
        vget(12, 0, "Anything else to exit�G", ans, 2, DOECHO);

        switch (ans[0])
        {
        case '1':
            if (otflag)
            {
                PROC_CMD("/bin/touch", "trace");
                msg = "BBS   tracing enabled.";
                report("opened report log");
            }
            else
            {
                report("closed report log");
                f_mv("trace", "trace.old");
                msg = "BBS   tracing disabled; log is in trace.old";
            }
            break;

        case '2':
            if (ctflag)
            {
                PROC_CMD("/bin/touch", "trace.chatd");
                msg = "Chat  tracing enabled.";
                report("chatd trace log opened");
            }
            else
            {
                f_mv("trace.chatd", "trace.chatd.old");
                msg = "Chat  tracing disabled; log is in trace.chatd.old";
                report("chatd trace log closed");
            }
            break;

        case '3':
            if (btflag)
            {
                PROC_CMD("/bin/touch", "trace.bvote");
                msg = "BVote tracing enabled.";
                report("BVote trace log opened");
            }
            else
            {
                f_mv("trace.bvote", "trace.bvote.old");
                msg = "BVote tracing disabled; log is in trace.bvote.old";
                report("BoardVote trace log closed");
            }
            break;

        default:
            msg = NULL;
            done = 1;
        }
        move(b_lines - 1, 0);
        if (msg)
            prints("%s\n", msg);
    }
    clear();
}
#endif /* HAVE_REPORT */

int u_verify(void)
{
    char keyfile[80], buf[80], /* buf2[80], */ inbuf[8], *key;
    FILE *fp;
    HDR fhdr;

    if (HAS_PERM(PERM_VALID))
    {
        zmsg("�z�������T�{�w�g�����A���ݶ�g�{�ҽX");
        return XEASY;
    }

    usr_fpath(keyfile, cuser.userid, FN_REGKEY);
    if (!dashf(keyfile))
    {
        zmsg("�S���z���{�ҽX��A�Э��� email address");
        return XEASY;
    }

    if (!(fp = fopen(keyfile, "r")))
    {
        zmsg("�}���ɮצ����D�A�гq������");
        return XEASY;
    }
    // FIXME(IID.20190415): What is the intent of the loop? Fetch last line?
    while (fgets(buf, 80, fp))
    {
        // FIXME(IID.20190415): The `strtok()` does absolutely nothing other than `key = buf`.
        //    To skip initial null characters? This does not work
        key = strtok(buf, "");
    }
    fclose(fp);

    if (vget(B_LINES_REF, 0, "�п�J�{�ҽX�G", inbuf, 8, DOECHO))
    {
        if (strncmp(key, inbuf, 7))
        {
            zmsg("��p�A�z���{�ҽX���~�C");

            /* jasonmel 20030731 : Hide the keys... */
            /*sprintf(buf2, "KEY: %s", key);
               vmsg(buf3);
               sprintf(buf2, "INPUT: %s", inbuf);
               vmsg(buf2); */
            return XEASY;
        }
        else
        {
            unlink(keyfile);

            cuser.userlevel |=
                (PERM_VALID | PERM_POST | PERM_PAGE | PERM_CHAT);
            strcpy(cuser.vmail, cuser.email);
            sprintf(buf, "key�{��:%s", cuser.email);
            str_scpy(cuser.justify, buf, sizeof(cuser.justify));
            time(&cuser.tvalid);
            acct_save(&cuser);
            usr_fpath(buf, cuser.userid, fn_dir);
            hdr_stamp(buf, HDR_LINK, &fhdr, "etc/justified");
            strcpy(fhdr.title, "[���U���\\] �z�w�g�q�L�����{�ҤF�I");
            strcpy(fhdr.owner, str_sysop);
            rec_add(buf, &fhdr, sizeof(fhdr));
            board_main();
            gem_main();
            talk_main();
            cutmp->ufo |= UFO_BIFF;
            vmsg("�����T�{���\\�A�ߨ责�@�v��");
        }
    }

    return 0;
}
