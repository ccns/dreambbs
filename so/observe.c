/*-------------------------------------------------------*/
/* observe.c    ( YZU_CSE WindTop BBS )                  */
/*-------------------------------------------------------*/
/* author : visor.bbs@bbs.yzu.edu.tw                     */
/* target : observe list                                 */
/* create :                                              */
/* update :                                              */
/*-------------------------------------------------------*/


#undef  MODES_C
#include "bbs.h"

/* ----------------------------------------------------- */
/* 擋信列表：選單式操作界面描述                          */
/* ----------------------------------------------------- */


static int observe_add(XO *xo);


static void
observe_item(
int num,
const OBSERVE *observe)
{
    prints("%6d %-*.*s %-*.*s\n", num, IDLEN, IDLEN, observe->userid, d_cols + 58, d_cols + 58, observe->title);
}

static int
observe_cur(
XO *xo)
{
    const OBSERVE *const observe = (const OBSERVE *) xo_pool_base + xo->pos;
    move(3 + xo->pos - xo->top, 0);
    observe_item(xo->pos + 1, observe);
    return XO_NONE;
}

static int
observe_body(
XO *xo)
{
    const OBSERVE *observe;
    int num, max, tail;

    move(3, 0);
    clrtobot();
    max = xo->max;
    if (max <= 0)
    {
        if (vans("要新增資料嗎(y/N)？[N] ") == 'y')
            return observe_add(xo);
        return XO_QUIT;
    }

    num = xo->top;
    observe = (const OBSERVE *) xo_pool_base + num;
    tail = num + XO_TALL;
    max = BMIN(max, tail);

    do
    {
        observe_item(++num, observe++);
    }
    while (num < max);

    return XO_NONE;
}


static int
observe_head(
XO *xo)
{
    vs_head("觀察名單列表", str_site);
    prints(NECK_OBSERVE, d_cols, "");
    return observe_body(xo);
}


static int
observe_load(
XO *xo)
{
    xo_load(xo, sizeof(OBSERVE));
    return observe_body(xo);
}


static int
observe_init(
XO *xo)
{
    xo_load(xo, sizeof(OBSERVE));
    return observe_head(xo);
}

static int
observe_sync(
XO *xo)
{
    char *fpath;
    int fd, size, total = 0, userno;
    struct stat st;

    fpath = xo->dir;

    if ((fd = open(fpath, O_RDWR, 0600)) < 0)
        return XO_NONE;

    outz("★ 資料整理稽核中，請稍候 \x1b[5m...\x1b[m");
    refresh();

    if (!fstat(fd, &st) && (size = st.st_size) > 0)
    {
        OBSERVE *pbase, *phead, *ptail;

        pbase = phead = (OBSERVE *) malloc(size);
        size = read(fd, pbase, size);
        if (size >= sizeof(OBSERVE))
        {
            ptail = (OBSERVE *)((char *) pbase + size);

            size = (char *) ptail - (char *) pbase;
            total = size / sizeof(OBSERVE);
            for (; phead < ptail; phead++)
            {
                userno = acct_userno(phead->userid);
                if (userno)
                    phead->userno = userno;
                else
                {
                    ptail--;
                    if (phead != ptail)
                        memcpy(phead, ptail, sizeof(OBSERVE));
                    total--;
                }
            }

            if (total > 0)
            {
                if (total > 1)
                    xsort(pbase, total, sizeof(OBSERVE), (int (*)(const void *lhs, const void *rhs))str_casecmp);

                lseek(fd, 0, SEEK_SET);
                write(fd, pbase, total * sizeof(OBSERVE));
                ftruncate(fd, total * sizeof(OBSERVE));
            }
        }
        free(pbase);
    }
    close(fd);
    if (total <= 0)
        unlink(fpath);
    return XO_INIT;
}


static int
observe_edit(
OBSERVE *observe,
int echo)
{
    ACCT acct;
    int userno;

    if (echo == DOECHO)
    {
        memset(observe, 0, sizeof(OBSERVE));
        userno = acct_get(msg_uid, &acct);

        if ((userno > 0))
        {
            observe->userno = acct.userno;
            strcpy(observe->userid, acct.userid);
            vget(B_LINES_REF, 0, "說明：", observe->title, sizeof(observe->title), echo);
            return 1;
        }
        else
            return 0;
    }
    else
    {
        vget(B_LINES_REF, 0, "說明：", observe->title, sizeof(observe->title), echo);
        return 1;
    }
}


static int
observe_add(
XO *xo)
{
    if (xo->max >= MAXOBSERVELIST)
    {
        vmsg("您的好友名單太多，請善加整理");
        return XO_FOOT;
    }
    else
    {
        OBSERVE observe;

        if (observe_edit(&observe, DOECHO))
        {
            rec_add(xo->dir, &observe, sizeof(OBSERVE));
            xo->pos = XO_TAIL;
            xo_load(xo, sizeof(OBSERVE));
        }
    }
    return XO_HEAD;
}

static int
observe_delete(
XO *xo)
{

    if (vans(msg_del_ny) == 'y')
    {
        if (!rec_del(xo->dir, sizeof(OBSERVE), xo->pos, NULL, NULL))
        {
            return XO_LOAD;
        }
    }
    return XO_FOOT;
}


static int
observe_change(
XO *xo)
{
    OBSERVE *observe, mate;
    int pos;

    pos = xo->pos;
    observe = (OBSERVE *) xo_pool_base + pos;

    //mate = *observe;
    memcpy(&mate, observe, sizeof(OBSERVE));
    observe_edit(observe, GCARRY);
    if (memcmp(observe, &mate, sizeof(OBSERVE)))
    {
        rec_put(xo->dir, observe, sizeof(OBSERVE), pos);
        return XR_FOOT + XO_CUR;
    }

    return XO_FOOT;
}

static int
observe_help(
XO *xo)
{
//  film_out(FILM_OBSERVE, -1);
    return XO_HEAD;
}


KeyFuncList observe_cb =
{
    {XO_INIT, {observe_init}},
    {XO_LOAD, {observe_load}},
    {XO_HEAD, {observe_head}},
    {XO_BODY, {observe_body}},
    {XO_CUR, {observe_cur}},

    {Ctrl('P'), {observe_add}},
    {'S', {observe_sync}},
    {'r', {observe_change}},
    {'c', {observe_change}},
    {'s', {xo_cb_init}},
    {'d', {observe_delete}},
    {'h', {observe_help}}
};


int
Observe_list(void)
{
    DL_HOLD;
    XO *xo, *last;
    char fpath[64];

    last = xz[XZ_OTHER - XO_ZONE].xo;  /* record */

    sprintf(fpath, FN_ETC_OBSERVE);
    xz[XZ_OTHER - XO_ZONE].xo = xo = xo_new(fpath);
    xo->cb = observe_cb;
    xo->recsiz = sizeof(OBSERVE);
    xo->pos = 0;
    xover(XZ_OTHER);
    observeshm_load();
    free(xo);

    xz[XZ_OTHER - XO_ZONE].xo = last;  /* restore */

    return DL_RELEASE(0);
}

