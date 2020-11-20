/*-------------------------------------------------------*/
/* banmail.c    ( YZU_CSE WindTop BBS )                  */
/*-------------------------------------------------------*/
/* author : visor.bbs@bbs.yzu.edu.tw                     */
/* target : ban_mail_list routines                       */
/* create : 99/12/20                                     */
/* update : 99/12/31                                     */
/*-------------------------------------------------------*/


#undef  MODES_C
#include "bbs.h"

/* ----------------------------------------------------- */
/* 擋信列表：選單式操作界面描述                          */
/* ----------------------------------------------------- */


static int banmail_add(XO * xo);


static void banmail_item(int num, const BANMAIL * ban)
{
    time_t now;
    char modes[7];
    sprintf(modes, "%c%c%c%c%c%c", (ban->mode & FW_OWNER) ? '1' : '0',
            (ban->mode & FW_TITLE) ? '1' : '0',
            (ban->mode & FW_TIME) ? '1' : '0',
            (ban->mode & FW_PATH) ? '1' : '0',
            (ban->mode & FW_ORIGIN) ? '1' : '0',
            (ban->mode & FW_CANCEL) ? '1' : '0');

    now = ((ban->time - time(0) + BANMAIL_EXPIRE * 86400) / 3600);
    prints("%6d  %6d %6ld %s  %-*.*s\n", num, ban->usage, BMAX(now, (time_t)0),
           modes, d_cols + 49, d_cols + 49, ban->data);
}

static int banmail_cur(XO *xo)
{
    const BANMAIL *const banmail = (const BANMAIL *) xo_pool_base + xo->pos;
    move(3 + xo->pos - xo->top, 0);
    banmail_item(xo->pos + 1, banmail);
    return XO_NONE;
}

static int banmail_body(XO * xo)
{
    const BANMAIL *banmail = NULL;
    int num, max, tail;

    max = xo->max;
    if (max <= 0)
    {
        if (vans("要新增資料嗎(y/N)？[N] ") == 'y')
            return banmail_add(xo);
        return XO_QUIT;
    }

    num = xo->top;
    banmail = (const BANMAIL *) xo_pool_base + num;
    tail = num + XO_TALL;
    max = BMIN(max, tail);

    move(3, 0);
    do
    {
        banmail_item(++num, banmail++);
    }
    while (num < max);
    clrtobot();

    return XO_NONE;
}


static int banmail_head(XO * xo)
{
    vs_head("擋信列表", str_site);
    prints(NECK_BANMAIL, d_cols, "");
    return banmail_body(xo);
}


static int banmail_load(XO * xo)
{
    xo_load(xo, sizeof(BANMAIL));
    return banmail_body(xo);
}


static int banmail_init(XO * xo)
{
    xo_load(xo, sizeof(BANMAIL));
    return banmail_head(xo);
}

static int banmail_sync(XO * xo)
{
    char *fpath;
    int fd, size = 0;
    struct stat st;

    fpath = xo->dir;

    if ((fd = open(fpath, O_RDWR, 0600)) < 0)
        return XO_NONE;

    outz("★ 資料整理稽核中，請稍候 \x1b[5m...\x1b[m");
    refresh();

    if (!fstat(fd, &st) && (size = st.st_size) > 0)
    {
        BANMAIL *pbase, *phead, *ptail;

        pbase = phead = (BANMAIL *) malloc(size);
        size = read(fd, pbase, size);
        if (size >= sizeof(BANMAIL))
        {
            ptail = (BANMAIL *) ((char *)pbase + size);

            size = (char *)ptail - (char *)pbase;
            if (size > 0)
            {
                if (size > sizeof(BANMAIL))
                    xsort(pbase, size / sizeof(BANMAIL), sizeof(BANMAIL),
                          (int (*)(const void *lhs, const void *rhs))str_casecmp);

                lseek(fd, 0, SEEK_SET);
                write(fd, pbase, size);
                ftruncate(fd, size);
            }
        }
        free(pbase);
    }
    close(fd);
    if (size <= 0)
        unlink(fpath);
    return XO_INIT;
}


static int banmail_edit(BANMAIL * banmail, int echo)
{
    int change = 0;
    char modes[8], buf[64];

    if (echo == DOECHO)
        memset(banmail, 0, sizeof(BANMAIL));

    sprintf(modes, "%c%c%c%c%c%c", (banmail->mode & FW_OWNER) ? '1' : '0',
            (banmail->mode & FW_TITLE) ? '1' : '0',
            (banmail->mode & FW_TIME) ? '1' : '0',
            (banmail->mode & FW_PATH) ? '1' : '0',
            (banmail->mode & FW_ORIGIN) ? '1' : '0',
            (banmail->mode & FW_CANCEL) ? '1' : '0');

    if (vget
        (b_lines, 0, "擋信列表：", banmail->data, sizeof(banmail->data), echo))
        change++;
    sprintf(buf, "擋信模式：(作者、標題、時間、路徑、來源、連線砍信)[%s]",
            modes);
    if (vget(B_LINES_REF, 0, buf, modes, 8, GCARRY))
    {
        banmail->mode = (modes[0] != '0') ? FW_OWNER : 0;
        banmail->mode |= (modes[1] != '0') ? FW_TITLE : 0;
        banmail->mode |= (modes[2] != '0') ? FW_TIME : 0;
        banmail->mode |= (modes[3] != '0') ? FW_PATH : 0;
        banmail->mode |= (modes[4] != '0') ? FW_ORIGIN : 0;
        banmail->mode |= (modes[5] != '0') ? FW_CANCEL : 0;
        change++;
    }

    if (change)
        return 1;
    else
        return 0;
}


static int banmail_add(XO * xo)
{
    BANMAIL banmail;

    if (banmail_edit(&banmail, DOECHO))
    {
        banmail.time = time(0);
        rec_add(xo->dir, &banmail, sizeof(BANMAIL));
        xo->pos = XO_TAIL /* xo->max */ ;
        xo_load(xo, sizeof(BANMAIL));
    }
    return XO_HEAD;
}

static int banmail_delete(XO * xo)
{

    if (vans(msg_del_ny) == 'y')
    {
        if (!rec_del(xo->dir, sizeof(BANMAIL), xo->pos, NULL, NULL))
        {
            return XO_LOAD;
        }
    }
    return XO_FOOT;
}


static int banmail_change(XO * xo)
{
    BANMAIL *banmail, mate;
    int pos;

    pos = xo->pos;
    banmail = (BANMAIL *) xo_pool_base + pos;

    mate = *banmail;
    banmail_edit(banmail, GCARRY);
    if (memcmp(banmail, &mate, sizeof(BANMAIL)))
    {
        banmail->time = time(0);
        rec_put(xo->dir, banmail, sizeof(BANMAIL), pos);
        return XR_FOOT + XO_CUR;
    }

    return XO_FOOT;
}

static int banmail_help(XO * xo)
{
    film_out(FILM_BANMAIL, -1);
    return XO_HEAD;
}


KeyFuncList banmail_cb = {
    {XO_INIT, {banmail_init}},
    {XO_LOAD, {banmail_load}},
    {XO_HEAD, {banmail_head}},
    {XO_BODY, {banmail_body}},
    {XO_CUR, {banmail_cur}},

    {Ctrl('P'), {banmail_add}},
    {'S', {banmail_sync}},
    {'r', {banmail_change}},
    {'c', {banmail_change}},
    {'s', {xo_cb_init}},
    {'d', {banmail_delete}},
    {'h', {banmail_help}}
};


int BanMail(void)
{
    XO *xo, *last;
    char fpath[64];

    last = xz[XZ_BANMAIL - XO_ZONE].xo;  /* record */

    sprintf(fpath, FN_ETC_BANMAIL_ACL);
    xz[XZ_BANMAIL - XO_ZONE].xo = xo = xo_new(fpath);
    xo->cb = banmail_cb;
    xo->recsiz = sizeof(BANMAIL);
    xo->pos = 0;
    xover(XZ_BANMAIL);
    fwshm_load();
    free(xo);

    xz[XZ_BANMAIL - XO_ZONE].xo = last;  /* restore */

    return 0;
}

void post_mail(void)
{
    XO *xx, *last;
    char fpath[64];

    last = xz[XZ_BANMAIL - XO_ZONE].xo;  /* record */

    sprintf(fpath, "brd/%s/banmail.acl", currboard);
    xz[XZ_BANMAIL - XO_ZONE].xo = xx = xo_new(fpath);
    xx->cb = banmail_cb;
    xx->recsiz = sizeof(BANMAIL);
    xx->pos = 0;
    xover(XZ_BANMAIL);
    free(xx);

    xz[XZ_BANMAIL - XO_ZONE].xo = last;  /* restore */
}
