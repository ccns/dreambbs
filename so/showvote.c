/*-------------------------------------------------------*/
/* show.c    ( YZU_CSE WindTop BBS )                     */
/*-------------------------------------------------------*/
/* author : visor.bbs@bbs.yzu.edu.tw                     */
/* target : administrator routines                       */
/* create : 2000/01/02                                   */
/* update : NULL                                         */
/*-------------------------------------------------------*/

#undef  MODES_C
#include "bbs.h"

static int show_add(XO *xo);
typedef struct
{
    char email[60];
} LOG;  /* DISKDATA(raw) */

static int
show_item(
XO *xo,
int pos)
{
    const LOG *const show = (const LOG *) xo_pool_base + pos;
    const int num = pos + 1;
    prints("%6d     %s\n", num, show->email);
    return XO_NONE;
}

static int
show_cur(
XO *xo,
int pos)
{
    move(3 + pos - xo->top, 0);
    return show_item(xo, pos);
}

static int
show_body(
XO *xo)
{
    int num, max, tail;

    move(3, 0);
    clrtobot();
    max = xo->max;
    if (max <= 0)
    {
        outs("\n《已投票名單》目前沒有資料\n");
        outs("\n  (^P)新增資料\n");
        return XO_NONE;
    }

    num = xo->top;
    tail = num + XO_TALL;
    max = BMIN(max, tail);

    do
    {
        show_item(xo, num++);
    }
    while (num < max);

    return XO_NONE;
}


static int
show_head(
XO *xo)
{
    vs_head("已投票名單", str_site);
    prints(NECK_SHOW, d_cols, "");
    return show_body(xo);
}


static int
show_load(
XO *xo)
{
    xo_load(xo, sizeof(LOG));
    return show_body(xo);
}


static int
show_init(
XO *xo)
{
    xo_load(xo, sizeof(LOG));
    return show_head(xo);
}


static int
show_edit(
LOG *show,
int echo)
{
    if (!(echo & GCARRY))
        memset(show, 0, sizeof(LOG));
    if (vget(B_LINES_REF, 0, "E-mail：", show->email, sizeof(show->email), echo))
        return 1;
    else
        return 0;
}


static int
show_add(
XO *xo)
{
    LOG show;

    if (show_edit(&show, DOECHO))
    {
        rec_add(xo->dir, &show, sizeof(LOG));
        xo->pos[xo->cur_idx] = XO_TAIL;
        xo_load(xo, sizeof(LOG));
    }
    return XO_HEAD;
}

static int
show_delete(
XO *xo,
int pos)
{

    if (vans_xo(xo, msg_del_ny) == 'y')
    {
        if (!rec_del(xo->dir, sizeof(LOG), pos, NULL, NULL))
        {
            return XO_LOAD;
        }
    }
    return XO_FOOT;
}


static int
show_change(
XO *xo,
int pos)
{
    LOG *show, mate;

    show = (LOG *) xo_pool_base + pos;

    mate = *show;
    show_edit(show, GCARRY);
    if (memcmp(show, &mate, sizeof(LOG)))
    {
        rec_put(xo->dir, show, sizeof(LOG), pos);
        return XR_FOOT + XO_CUR;
    }

    return XO_FOOT;
}

static int
show_help(
XO *xo)
{
    return XO_NONE;
}


KeyFuncList show_cb =
{
    {XO_INIT, {show_init}},
    {XO_LOAD, {show_load}},
    {XO_HEAD, {show_head}},
    {XO_BODY, {show_body}},
    {XO_CUR | XO_POSF, {.posf = show_cur}},

    {Ctrl('P'), {show_add}},
    {'r' | XO_POSF, {.posf = show_change}},
    {'c' | XO_POSF, {.posf = show_change}},
    {'s', {xo_cb_init}},
    {'d' | XO_POSF, {.posf = show_delete}},
    {'h', {show_help}}
};


int
Showvote(
XO *xo,
int pos)
{
    DL_HOLD;
    XO *last;
    const VCH *vch;
    char fpath[128], *fname;
    if (!HAS_PERM(PERM_SYSOP))
        return DL_RELEASE(XO_NONE);

    last = xz[XZ_OTHER - XO_ZONE].xo;  /* record */

    vch = (const VCH *) xo_pool_base + pos;
    hdr_fpath(fpath, xo->dir, (const HDR *) vch);
    fname = strrchr(fpath, '@');
    *fname = 'E';

    utmp_mode(M_OMENU);

    xz[XZ_OTHER - XO_ZONE].xo = xo = xo_new(fpath);
    xo->cb = show_cb;
    xo->recsiz = sizeof(LOG);
    for (int i = 0; i < COUNTOF(xo->pos); ++i)
        xo->pos[i] = 0;
    xover(XZ_OTHER);
    free(xo);

    xz[XZ_OTHER - XO_ZONE].xo = last;  /* restore */

    return DL_RELEASE(XO_INIT);
}



