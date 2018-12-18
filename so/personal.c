/*-------------------------------------------------------*/
/* personal.c   ( CCNS BBS )                             */
/*-------------------------------------------------------*/
/* author : cat@ccns.ncku.edu.tw                         */
/* target : personal                                     */
/* create : 2004/12/12                                   */
/* update : NULL                                         */
/*-------------------------------------------------------*/

#undef  _MODES_C_
#include "bbs.h"


extern XZ xz[];
extern BCACHE *bshm;
static int mode = 0;    /* 0:email 1:brdtitle */
char msg[60];

static int
personal_log(
    PB *personal,
    int admin)          /* 0:申請  1:開板  2:拒絕 */
{
    FILE *fp;
    time_t now;
    char tag[3][5] = {"申請", "開板", "拒絕"};

    if ((fp = fopen(FN_PERSONAL_LOG, "a+")))
    {
        time(&now);

        fprintf(fp, "%24.24s ", ctime(&now));
        fprintf(fp, "%s ", (admin) ? cuser.userid : personal->userid);
        fprintf(fp, "%s ", tag[admin]);
        if (!admin)
            fprintf(fp, "%s 板\n", personal->brdname);
        else
            fprintf(fp, "%s 的 %s 板\n", personal->userid, personal->brdname);
        fclose(fp);
    }

    return 0;
}

static int
belong(
    char *flist,
    char *key)
{
    int fd, rc;

    rc = 0;
    fd = open(flist, O_RDONLY);
    if (fd >= 0)
    {
        mgets(-1);

        while ((flist = mgets(fd)))
        {
            str_lower(flist, flist);
            if (str_str(key, flist))
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
    char *userid)
{
    int ch;
    char *str;

    if (strlen(userid) < 2)
        return 1;

    if (!is_alpha(*userid))
        return 1;

    if (!str_cmp(userid, "new"))
        return 1;

    str = userid;
    while ((ch = *(++str)))
    {
        if (!is_alnum(ch))
            return 1;
    }
    return (belong(FN_ETC_BADID, userid));
}


int
personal_apply(void)
{
    char validemail[2][20] = {"ccmail.ncku.edu.tw", "mail.ncku.edu.tw"};
    int i, num;
    char *c, /*buf[60], */brdname[IDLEN + 1];
    PB pb;
    struct tm *t;
    time_t now;
    int thisyear, enteryear;

    now = time(0);
    t = localtime(&now);



    if (cuser.numposts < 20 || cuser.numlogins < 500)
    {
        vmsg("資格不符無法申請個人板");
        return 0;
    }

    c = strchr(cuser.email, '@');
    if (c == NULL || (strcmp(c+1, validemail[0]) && strcmp(c+1, validemail[1])))
    {
        vmsg("您的 E-mail 不合格!");
        return 0;
    }

    thisyear = t->tm_year - 11;
    enteryear = (cuser.email[3]-'0') * 10 + (cuser.email[4]-'0');

    //百年蟲 ecchi float 2012/4/25
    if ( (thisyear - enteryear)%100 > 5)
    {
        vmsg("您的身份不合格!");
        return 0;
    }

    num = rec_num(FN_ETC_PERSONAL, sizeof(PB));
    for (i=0; i<num; i++)
    {
        rec_get(FN_ETC_PERSONAL, &pb, sizeof(PB), i);

        c = strchr(pb.email, '@');

        if (!strcmp(cuser.userid, pb.userid) || !strncmp(cuser.email, pb.email, c-pb.email))
        {
            if (pb.state & PB_OPEN)
                vmsg("您已經申請過個人板！");
            else
                vmsg("您的申請書正在審核中！");
            return 0;
        }
    }

    /* 資格審核通過 */

    memset(&pb, 0, sizeof(PB));

    clear();
    vs_head("個人板申請單", str_site);

    move(2, 0);
    prints("申請人：   %s\n", cuser.userid);
    prints("E-mail：   %s\n", cuser.email);
    prints("上站次數： %d\n", cuser.numlogins);
    prints("文章數：   %d", cuser.numposts);

    num = DOECHO;

    while (1)
    {
        while (1)
        {
            if (!vget(7, 0, "看板英文名稱： ", brdname, IDLEN - 1, num))
                return 0;

            if (is_badid(brdname))
                vmsg("無法接受這個板名，請使用英文字母，並且不要包含空格");
            else
            {
                sprintf(pb.brdname, "P_%s", brdname);
                if (brd_bno(pb.brdname) >= 0)
                    vmsg("此板名已經有人使用");
                else
                    break;
            }
        }

        while (1)
            if (vget(8, 0, "看板中文名稱： ", pb.brdtitle, BTLEN + 1, num))
                break;

        if (vans("確定資料都正確嗎？[y/N]") == 'y')
            break;
        else
            num = GCARRY;
    }

    strcpy(pb.userid, cuser.userid);
    strcpy(pb.email, cuser.email);
    pb.state = PB_APPLY;

    rec_add(FN_ETC_PERSONAL, &pb, sizeof(PB));
    personal_log(&pb, 0);

    vmsg("申請書填寫完成，請等待站務審核");
    return 0;

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

static void
personal_item(
    int num,
    PB *personal)
{
    if (!mode)
        prints("%6d %c %-12s %-12s %-40s\n", num, personal_attr(personal->state), personal->userid, personal->brdname, personal->email);
    else
        prints("%6d %c %-12s %-12s %-40s\n", num, personal_attr(personal->state), personal->userid, personal->brdname, personal->brdtitle);
}

static int
personal_body(
    XO *xo)
{
    PB *personal;
    int num, max, tail;

    move(3, 0);
    clrtobot();
    max = xo->max;
    if (max <= 0)
    {
        vmsg("目前沒有資料");
        return XO_QUIT;
    }
    personal = (PB *) xo_pool;
    num = xo->top;
    tail = num + XO_TALL;
    if (max > tail)
        max = tail;

    do
    {
        personal_item(++num, personal++);
    } while (num < max);

    return XO_NONE;
}


static int
personal_head(
    XO *xo)
{
    vs_head("個人板清單", str_site);
    outs(
        "  [←]離開 c)修改 d)刪除 s)重整 TAB)中文版名/E-mail O)開板 D)退件 [h]elp\n"
        "\033[44m  編號   申請人       看板名稱     ");
    if (!mode)
        outs("E-mail                                  \033[m");
    else
        outs("中文版名                                \033[m");
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
    if (vget(b_lines, 0, "申請人:", personal->userid, sizeof(personal->userid), echo)
     && vget(b_lines, 0, "看板名:", personal->brdname, sizeof(personal->brdname), echo)
     && vget(b_lines, 0, "看板標題:", personal->brdtitle, sizeof(personal->brdtitle), echo)
     && vget(b_lines, 0, "E-mail:", personal->email, sizeof(personal->email), echo))
        return 1;
    else
        return 0;
}


static int
personal_delete(
    XO *xo)
{

    if (vans(msg_del_ny) == 'y')
    {
        if (!rec_del(xo->dir, sizeof(PB), xo->pos, NULL, NULL))
        {
            return personal_load(xo);
        }
    }
    return XO_FOOT;
}


static int
personal_change(
    XO *xo)
{
    PB *personal, mate;
    int pos, cur;

    pos = xo->pos;
    cur = pos - xo->top;
    personal = (PB *) xo_pool + cur;

    mate = *personal;
    personal_edit(personal, GCARRY);
    if (memcmp(personal, &mate, sizeof(PB)))
    {
        rec_put(xo->dir, personal, sizeof(PB), pos);
        move(3 + cur, 0);
        personal_item(++pos, personal);
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
    PB *personal,
    int admin)          /* 0:open 1:deny */
{
    HDR hdr;
    time_t now;
    char folder[50], fpath[128];
    char title[2][30] = {"您的個人板通過申請囉！", "您的個人板申請書被退回！"};
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
        fprintf(fp, "作者: SYSOP (%s)\n標題: %s\n時間: %s\n", SYSOPNICK, title[admin], ctime(&now));
        fprintf(fp, "申請人:   %s\n", personal->userid);
        fprintf(fp, "英文板名: %s\n", personal->brdname);
        fprintf(fp, "中文板名: %s\n\n", personal->brdtitle);
        if (!admin)
            f_suck(fp, "gem/@/@accept");
        else
        {
            fprintf(fp, "\x1b[1;33m退件理由: %s\x1b[m\n\n", msg);
            f_suck(fp, "gem/@/@deny");
        }

        fclose(fp);
    }
    return 0;


}

int
sort_compare(
    const void *p1,
    const void *p2)
{
    HDR *a1, *a2;

    a1 = (HDR *) p1;
    a2 = (HDR *) p2;
    return str_cmp(a1->xname, a2->xname);

}

static int
personal_sort(
    char *gem)
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
    XO *xo)
{
    PB *personal;
    int cur, pos, index;
    char fpath[80];
    BRD newboard;
    HDR hdr;
    int bno;
    ACCT acct;
    char gem[5][20] = {"gem/@/@Person_A_E", "gem/@/@Person_F_J", "gem/@/@Person_K_O", "gem/@/@Person_P_T", "gem/@/@Person_U_Z"};

    pos = xo->pos;
    cur = pos - xo->top;
    personal = (PB *) xo_pool + cur;

    if (personal->state & PB_OPEN)
        return XO_NONE;

    if (brd_bno(personal->brdname) >= 0)
    {
        vmsg("板名雷同");
        return XO_NONE;
    }

    if ( bshm->number >= MAXBOARD)
    {
        vmsg("超過系統所能容納看版個數，請調整系統參數");
        return XO_NONE;
    }

    if (vans("確定要開設此看板嗎？[y/N]") != 'y')
        return XO_NONE;

    memset(&newboard, 0, sizeof(newboard));

    strcpy(newboard.brdname, personal->brdname);
    sprintf(newboard.title, "○ %s", personal->brdtitle);
    newboard.color = 7;
    strcpy(newboard.class, "個人");
    strcpy(newboard.BM, personal->userid);
    time(&newboard.bstamp);
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
        vmsg("無法建立新板");
        return XO_NONE;
    }

    sprintf(fpath, "gem/brd/%s", newboard.brdname);
    mak_dirs(fpath);
    mak_dirs(fpath + 4);

    bshm->uptime = 0;             /* force reload of bcache */
    bshm_init();

    /* 順便加進 NewBoard */

    brd2gem(&newboard, &hdr);
    rec_add("gem/@/@NewBoard", &hdr, sizeof(HDR));
    rec_add("gem/@/@Person_All", &hdr, sizeof(HDR));

    personal_sort("gem/@/@Person_All");

    if ((index = (tolower(newboard.brdname[2]) - 'a') / 5) == 5)
        index--;
    rec_add(gem[index], &hdr, sizeof(HDR));

    personal_sort(gem[index]);

    personal->state = PB_OPEN;

    rec_put(xo->dir, personal, sizeof(PB), pos);
    move(3 + cur, 0);
    personal_item(++pos, personal);

    mail2usr(personal, 0);
    if (acct_load(&acct, personal->userid) >= 0)
        if (!(acct.userlevel & PERM_BM))
        {
            acct.userlevel |= PERM_BM;
            acct_save(&acct);
        }

    personal_log(personal, 1);

    vmsg("新板成立");

    return XO_FOOT;
}

static int
personal_deny(
    XO *xo)
{
    PB *personal;
    int pos, cur;

    pos = xo->pos;
    cur = pos - xo->top;
    personal = (PB *) xo_pool + cur;

    if (personal->state & PB_OPEN)
        return XO_NONE;

    if (!vget(b_lines, 0, "拒絕開板理由: ", msg, sizeof(msg), DOECHO))
        return XO_NONE;

    if (vans("確定拒絕此申請嗎？[y/N]") != 'y')
        return XO_NONE;

    mail2usr(personal, 1);

    if (!rec_del(xo->dir, sizeof(PB), xo->pos, NULL, NULL))
    {
        personal_log(personal, 2);
        return personal_load(xo);
    }


    return XO_FOOT;
}

static int
personal_help(
    XO *xo)
{
//  film_out(FILM_PB, -1);
    return personal_head(xo);
}

KeyFunc personal_cb[] =
{
    {XO_INIT, personal_init},
    {XO_LOAD, personal_load},
    {XO_HEAD, personal_head},
    {XO_BODY, personal_body},

    {'c', personal_change},
    {'s', personal_init},
    {'d', personal_delete},
    {KEY_TAB, personal_switch},
    {'O', personal_open},
    {'D', personal_deny},
    {'h', personal_help}
};

int
personal_admin(void)
{
    XO *xo;
    utmp_mode(M_OMENU);
    xz[XZ_OTHER - XO_ZONE].xo = xo = xo_new(FN_ETC_PERSONAL);
    xz[XZ_OTHER - XO_ZONE].cb = personal_cb;
    xover(XZ_OTHER);
    free(xo);
    return 0;
}

