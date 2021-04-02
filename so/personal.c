/*-------------------------------------------------------*/
/* personal.c   ( CCNS BBS )                             */
/*-------------------------------------------------------*/
/* author : cat@ccns.ncku.edu.tw                         */
/* target : personal                                     */
/* create : 2004/12/12                                   */
/* update : NULL                                         */
/*-------------------------------------------------------*/

#undef  MODES_C
#include "bbs.h"


static int mode = 0;    /* 0:email 1:brdtitle */
static char msg[60];

static int
personal_log(
    const PB *personal,
    int admin)          /* 0:�ӽ�  1:�}�O  2:�ڵ� */
{
    FILE *fp;
    time_t now;
    static const char *const tag[3] = {"�ӽ�", "�}�O", "�ڵ�"};

    if ((fp = fopen(FN_PERSONAL_LOG, "a+")))
    {
        time(&now);

        fprintf(fp, "%24.24s ", ctime(&now));
        fprintf(fp, "%s ", (admin) ? cuser.userid : personal->userid);
        fprintf(fp, "%s ", tag[admin]);
        if (!admin)
            fprintf(fp, "%s �O\n", personal->brdname);
        else
            fprintf(fp, "%s �� %s �O\n", personal->userid, personal->brdname);
        fclose(fp);
    }

    return 0;
}

static bool
belong(
    const char *flist,
    const char *key)
{
    int fd;
    bool rc;
    char *str;

    rc = false;
    fd = open(flist, O_RDONLY);
    if (fd >= 0)
    {
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
    const char *userid)
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


int
personal_apply(void)
{
    DL_HOLD;
    static const char *const validemail[] = PB_MAIL_DOMAINS;
    int i, num;
    char *c, /*buf[60], */brdname[IDLEN + 1];
    PB pb;
    struct tm *t;
    time_t now;
    int thisyear, enteryear;

    now = time(0);
    t = localtime(&now);



    if (cuser.numposts < PB_NUMPOST_MIN || cuser.numlogins < PB_NUMLOGIN_MIN)
    {
        vmsg("��椣�ŵL�k�ӽЭӤH�O");
        return DL_RELEASE(0);
    }

    {
        bool valid = false;
        c = strchr(cuser.email, '@');
        if (c)
        {
            for (int i = 0; i < COUNTOF(validemail); ++i)
            {
                if (!strcmp(c+1, validemail[i]))
                    valid = true;
            }
        }
        if (!valid)
        {
            vmsg("�z�� E-mail ���X��!");
            return DL_RELEASE(0);
        }
    }

    thisyear = t->tm_year - 11;
    enteryear = (cuser.email[3]-'0') * 10 + (cuser.email[4]-'0');

    //�ʦ~�� ecchi float 2012/4/25
    if ((thisyear - enteryear)%100 > PB_SCHOOL_LEN_YEAR_MAX)
    {
        vmsg("�z���������X��!");
        return DL_RELEASE(0);
    }

    num = rec_num(FN_ETC_PERSONAL, sizeof(PB));
    for (i=0; i<num; i++)
    {
        rec_get(FN_ETC_PERSONAL, &pb, sizeof(PB), i);

        c = strchr(pb.email, '@');

        if (!strcmp(cuser.userid, pb.userid) || !strncmp(cuser.email, pb.email, c-pb.email))
        {
            if (pb.state & PB_OPEN)
                vmsg("�z�w�g�ӽйL�ӤH�O�I");
            else
                vmsg("�z���ӽЮѥ��b�f�֤��I");
            return DL_RELEASE(0);
        }
    }

    /* ���f�ֳq�L */

    memset(&pb, 0, sizeof(PB));

    clear();
    vs_head("�ӤH�O�ӽг�", str_site);

    move(2, 0);
    prints("�ӽФH�G   %s\n", cuser.userid);
    prints("E-mail�G   %s\n", cuser.email);
    prints("�W�����ơG %d\n", cuser.numlogins);
    prints("�峹�ơG   %d", cuser.numposts);

    num = DOECHO;

    while (1)
    {
        while (1)
        {
            if (!vget(7, 0, "�ݪO�^��W�١G ", brdname, IDLEN - 2 + 1, num))
                return DL_RELEASE(0);

            if (is_badid(brdname))
                vmsg("�L�k�����o�ӪO�W�A�Шϥέ^��r���A�åB���n�]�t�Ů�");
            else
            {
                sprintf(pb.brdname, "P_%s", brdname);
                if (brd_bno(pb.brdname) >= 0)
                    vmsg("���O�W�w�g���H�ϥ�");
                else
                    break;
            }
        }

        while (1)
            if (vget(8, 0, "�ݪO����W�١G ", pb.brdtitle, BTLEN + 1, num))
                break;

        if (vans("�T�w��Ƴ����T�ܡH[y/N]") == 'y')
            break;
        else
            num = GCARRY;
    }

    strcpy(pb.userid, cuser.userid);
    strcpy(pb.email, cuser.email);
    pb.state = PB_APPLY;

    rec_add(FN_ETC_PERSONAL, &pb, sizeof(PB));
    personal_log(&pb, 0);

    vmsg("�ӽЮѶ�g�����A�е��ݯ��ȼf��");
    return DL_RELEASE(0);

}

static char
personal_attr(unsigned int state)
{
    if (state & PB_APPLY)
        return 'A';

    if (state & PB_OPEN)
        return 'O';

    return ' ';
}

static int
personal_item(
    XO *xo,
    int pos)
{
    const PB *const personal = (const PB *) xo_pool_base + pos;
    const int num = pos + 1;
    if (!mode)
        prints("%6d %c %-*s %-*s %-*s\n", num, personal_attr(personal->state), IDLEN, personal->userid, IDLEN, personal->brdname, d_cols + 44, personal->email);
    else
        prints("%6d %c %-*s %-*s %-*s\n", num, personal_attr(personal->state), IDLEN, personal->userid, IDLEN, personal->brdname, d_cols + 44, personal->brdtitle);
    return XO_NONE;
}

static int
personal_cur(
    XO *xo,
    int pos)
{
    move(3 + pos - xo->top, 0);
    return personal_item(xo, pos);
}

static int
personal_body(
    XO *xo)
{
    int num, max, tail;

    move(3, 0);
    clrtobot();
    max = xo->max;
    if (max <= 0)
    {
        outs("\n�m�ӤH�O�M��n�ثe�S�����\n");
        return XO_NONE;
    }
    num = xo->top;
    tail = num + XO_TALL;
    max = BMIN(max, tail);

    do
    {
        personal_item(xo, num++);
    } while (num < max);

    return XO_NONE;
}


static int
personal_head(
    XO *xo)
{
    vs_head("�ӤH�O�M��", str_site);
    outs(NECK_PERSONAL1);
    if (!mode)
        prints(NECK_PERSONALEMAIL2, d_cols, "");
    else
        prints(NECK_PERSONALTITLE2, d_cols, "");
    return personal_body(xo);
}


static int
personal_load(
    XO *xo)
{
    xo_load(xo, sizeof(PB));
    return personal_body(xo);
}


static int
personal_init(
    XO *xo)
{
    xo_load(xo, sizeof(PB));
    return personal_head(xo);
}


static int
personal_edit(
    PB *personal,
    int echo)
{
    if (echo == DOECHO)
        memset(personal, 0, sizeof(PB));
    if (vget(B_LINES_REF, 0, "�ӽФH:", personal->userid, sizeof(personal->userid), echo)
     && vget(B_LINES_REF, 0, "�ݪO�W:", personal->brdname, sizeof(personal->brdname), echo)
     && vget(B_LINES_REF, 0, "�ݪO���D:", personal->brdtitle, sizeof(personal->brdtitle), echo)
     && vget(B_LINES_REF, 0, "E-mail:", personal->email, sizeof(personal->email), echo))
        return 1;
    else
        return 0;
}


static int
personal_delete(
    XO *xo,
    int pos)
{

    if (vans(msg_del_ny) == 'y')
    {
        if (!rec_del(xo->dir, sizeof(PB), pos, NULL, NULL))
        {
            return XO_LOAD;
        }
    }
    return XO_FOOT;
}


static int
personal_change(
    XO *xo,
    int pos)
{
    PB *personal, mate;

    personal = (PB *) xo_pool_base + pos;

    mate = *personal;
    personal_edit(personal, GCARRY);
    if (memcmp(personal, &mate, sizeof(PB)))
    {
        rec_put(xo->dir, personal, sizeof(PB), pos);
        return XR_FOOT + XO_CUR;
    }

    return XO_FOOT;
}

static int
personal_switch(
    XO *xo)
{
    mode++;
    mode %= 2;
    return XO_INIT;
}

static int
mail2usr(
    const PB *personal,
    int admin)          /* 0:open 1:deny */
{
    HDR hdr;
    time_t now;
    char folder[50], fpath[128];
    static const char *const title[2] = {"�z���ӤH�O�q�L�ӽ��o�I", "�z���ӤH�O�ӽЮѳQ�h�^�I"};
    FILE *fp;

    now = time(0);

    memset(&hdr, 0, sizeof(hdr));

    usr_fpath(folder, personal->userid, fn_dir);
    hdr_stamp(folder, 0, &hdr, fpath);
    strcpy(hdr.owner, "SYSOP");
    strcpy(hdr.nick, SYSOPNICK);
    strcpy(hdr.title, title[admin]);
    rec_add(folder, &hdr, sizeof(HDR));

    if ((fp = fopen(fpath, "w")))
    {
        fprintf(fp, "�@��: SYSOP (%s)\n���D: %s\n�ɶ�: %s\n", SYSOPNICK, title[admin], ctime(&now));
        fprintf(fp, "�ӽФH:   %s\n", personal->userid);
        fprintf(fp, "�^��O�W: %s\n", personal->brdname);
        fprintf(fp, "����O�W: %s\n\n", personal->brdtitle);
        if (!admin)
            f_suck(fp, "gem/@/@accept");
        else
        {
            fprintf(fp, "\x1b[1;33m�h��z��: %s\x1b[m\n\n", msg);
            f_suck(fp, "gem/@/@deny");
        }

        fclose(fp);
    }
    return 0;


}

GCC_PURE static int
sort_compare(
    const void *p1,
    const void *p2)
{
    const HDR *a1, *a2;

    a1 = (const HDR *) p1;
    a2 = (const HDR *) p2;
    return str_casecmp(a1->xname, a2->xname);

}

static int
personal_sort(
    const char *gem)
{
    HDR *sort;
    int max, fd, total;
    struct stat st;

    if ((fd = open(gem, O_RDWR, 0600)) < 0)
        return 0;

    if (fstat(fd, &st) || (total = st.st_size) <= 0)
    {
        close(fd);
        return 0;
    }
    f_exlock(fd);

    sort = (HDR *) malloc(total);
    read(fd, sort, total);

    max = total / sizeof(HDR);

    qsort(sort, max, sizeof(HDR), sort_compare);

    lseek(fd, (off_t) 0, SEEK_SET);
    write(fd, sort, total);
    f_unlock(fd);
    close(fd);
    free(sort);

    return 0;


}

static int
personal_open(
    XO *xo,
    int pos)
{
    PB *personal;
    int cur, index;
    char fpath[80];
    BRD newboard;
    HDR hdr;
    int bno;
    ACCT acct;
    static const char *const gem[] = {"gem/@/@Person_A_E", "gem/@/@Person_F_J", "gem/@/@Person_K_O", "gem/@/@Person_P_T", "gem/@/@Person_U_Z"};

    cur = pos - xo->top;
    personal = (PB *) xo_pool_base + pos;

    if (personal->state & PB_OPEN)
        return XO_NONE;

    if (brd_bno(personal->brdname) >= 0)
    {
        vmsg("�O�W�p�P");
        return XO_FOOT;
    }

    if (bshm->number >= MAXBOARD)
    {
        vmsg("�W�L�t�Ωү�e�Ǭݪ��ӼơA�нվ�t�ΰѼ�");
        return XO_FOOT;
    }

    if (vans("�T�w�n�}�]���ݪO�ܡH[y/N]") != 'y')
        return XO_FOOT;

    memset(&newboard, 0, sizeof(newboard));

    strcpy(newboard.brdname, personal->brdname);
    sprintf(newboard.title, "�� %s", personal->brdtitle);
    newboard.color = 7;
    strcpy(newboard.class_, "�ӤH");
    strcpy(newboard.BM, personal->userid);
    time32(&newboard.bstamp);
//  newboard.readlevel |= PERM_SYSOP;
    newboard.postlevel |= (PERM_POST|PERM_VALID|PERM_BASIC);
    newboard.battr |= BRD_NOTRAN;
    newboard.expiremax = 8000;
    newboard.expiremin = 7500;

    if ((bno = brd_bno("")) >= 0)
    {
        rec_put(FN_BRD, &newboard, sizeof(newboard), bno);
    }
    else if (rec_add(FN_BRD, &newboard, sizeof(newboard)) < 0)
    {
        vmsg("�L�k�إ߷s�O");
        return XO_FOOT;
    }

    sprintf(fpath, "gem/brd/%s", newboard.brdname);
    mak_dirs(fpath);
    mak_dirs(fpath + 4);

    bshm->uptime = 0;             /* force reload of bcache */
    bshm_init(&bshm);

    /* ���K�[�i NewBoard */

    brd2gem(&newboard, &hdr);
    rec_add("gem/@/@NewBoard", &hdr, sizeof(HDR));
    rec_add("gem/@/@Person_All", &hdr, sizeof(HDR));

    personal_sort("gem/@/@Person_All");

    if ((index = (tolower(newboard.brdname[2]) - 'a') / COUNTOF(gem)) >= COUNTOF(gem))
        index = COUNTOF(gem) - 1;
    rec_add(gem[index], &hdr, sizeof(HDR));

    personal_sort(gem[index]);

    personal->state = PB_OPEN;

    rec_put(xo->dir, personal, sizeof(PB), pos);
    move(3 + cur, 0);
    personal_item(xo, pos);
    cursor_show(3 + cur, 0);

    mail2usr(personal, 0);
    if (acct_load(&acct, personal->userid) >= 0)
        if (!(acct.userlevel & PERM_BM))
        {
            acct.userlevel |= PERM_BM;
            acct_save(&acct);
        }

    personal_log(personal, 1);

    vmsg("�s�O����");

    return XO_FOOT;
}

static int
personal_deny(
    XO *xo,
    int pos)
{
    const PB *personal;

    personal = (const PB *) xo_pool_base + pos;

    if (personal->state & PB_OPEN)
        return XO_NONE;

    if (!vget(B_LINES_REF, 0, "�ڵ��}�O�z��: ", msg, sizeof(msg), DOECHO))
        return XO_FOOT;

    if (vans("�T�w�ڵ����ӽжܡH[y/N]") != 'y')
        return XO_FOOT;

    mail2usr(personal, 1);

    {
        const PB personal_orig = *personal;
        if (!rec_del(xo->dir, sizeof(PB), pos, NULL, NULL))
        {
            personal_log(&personal_orig, 2);
            return XO_LOAD;
        }
    }


    return XO_FOOT;
}

static int
personal_help(
    XO *xo)
{
//  film_out(FILM_PB, -1);
    return XO_HEAD;
}

KeyFuncList personal_cb =
{
    {XO_INIT, {personal_init}},
    {XO_LOAD, {personal_load}},
    {XO_HEAD, {personal_head}},
    {XO_BODY, {personal_body}},
    {XO_CUR | XO_POSF, {.posf = personal_cur}},

    {'c' | XO_POSF, {.posf = personal_change}},
    {'s', {xo_cb_init}},
    {'d' | XO_POSF, {.posf = personal_delete}},
    {KEY_TAB, {personal_switch}},
    {'O' | XO_POSF, {.posf = personal_open}},
    {'D' | XO_POSF, {.posf = personal_deny}},
    {'h', {personal_help}}
};

int
personal_admin(void)
{
    DL_HOLD;
    XO *xo, *last;

    last = xz[XZ_OTHER - XO_ZONE].xo;  /* record */

    utmp_mode(M_OMENU);
    xz[XZ_OTHER - XO_ZONE].xo = xo = xo_new(FN_ETC_PERSONAL);
    xo->cb = personal_cb;
    xo->recsiz = sizeof(PB);
    xo->pos = 0;
    xover(XZ_OTHER);
    free(xo);

    xz[XZ_OTHER - XO_ZONE].xo = last;  /* restore */

    return DL_RELEASE(0);
}

