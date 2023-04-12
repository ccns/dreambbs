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


static int banmail_item(XO * xo, int pos)
{
    const BANMAIL *const ban = (const BANMAIL *) xo_pool_base + pos;
    const int num = pos + 1;

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

    return XO_NONE;
}

static int banmail_cur(XO *xo, int pos)
{
    move(3 + pos - xo->top, 0);
    banmail_item(xo, pos);
    return XO_NONE;
}

static int banmail_body(XO * xo)
{
    int num, max, tail;

    move(3, 0);

    max = xo->max;
    if (max <= 0)
    {
        outs("\n《擋信列表》目前沒有資料\n");
        outs("\n  (^P)新增資料\n");
        clrtobot();
        return XO_NONE;
    }

    num = xo->top;
    tail = num + XO_TALL;
    max = BMIN(max, tail);

    do
    {
        banmail_item(xo, num++);
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
        xo->pos[xo->cur_idx] = XO_TAIL /* xo->max */ ;
        xo_load(xo, sizeof(BANMAIL));
    }
    return XO_HEAD;
}

static int banmail_delete(XO * xo, int pos)
{

    if (vans_xo(xo, msg_del_ny) == 'y')
    {
        if (!rec_del(xo->dir, sizeof(BANMAIL), pos, NULL, NULL))
        {
            return XO_LOAD;
        }
    }
    return XO_FOOT;
}


static int banmail_change(XO * xo, int pos)
{
    BANMAIL *banmail, mate;

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
    {XO_CUR | XO_POSF, {.posf = banmail_cur}},

    {Ctrl('P'), {banmail_add}},
    {'S', {banmail_sync}},
    {'r' | XO_POSF, {.posf = banmail_change}},
    {'c' | XO_POSF, {.posf = banmail_change}},
    {'s', {xo_cb_init}},
    {'d' | XO_POSF, {.posf = banmail_delete}},
    {'h', {banmail_help}}
};


int BanMail(void)
{
    DL_HOLD;
    XO *xo, *last;
    char fpath[64];

    last = xz[XZ_BANMAIL - XO_ZONE].xo;  /* record */

    sprintf(fpath, FN_ETC_BANMAIL_ACL);
    xz[XZ_BANMAIL - XO_ZONE].xo = xo = xo_new(fpath);
    xo->cb = banmail_cb;
    xo->recsiz = sizeof(BANMAIL);
    for (int i = 0; i < COUNTOF(xo->pos); ++i)
        xo->pos[i] = 0;
    xover(XZ_BANMAIL);
    fwshm_load(fwshm);
    free(xo);

    xz[XZ_BANMAIL - XO_ZONE].xo = last;  /* restore */

    return DL_RELEASE(0);
}

void post_mail(void)
{
    DL_HOLD;
    XO *xx, *last;
    char fpath[64];

    last = xz[XZ_BANMAIL - XO_ZONE].xo;  /* record */

    sprintf(fpath, "brd/%s/banmail.acl", currboard);
    xz[XZ_BANMAIL - XO_ZONE].xo = xx = xo_new(fpath);
    xx->cb = banmail_cb;
    xx->recsiz = sizeof(BANMAIL);
    for (int i = 0; i < COUNTOF(xx->pos); ++i)
        xx->pos[i] = 0;
    xover(XZ_BANMAIL);
    free(xx);

    xz[XZ_BANMAIL - XO_ZONE].xo = last;  /* restore */

    DL_RELEASE_VOID();
}
