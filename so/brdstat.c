/*-------------------------------------------------------*/
/* brdstat.c   ( YZU_CSE WindTop BBS )                   */
/*-------------------------------------------------------*/
/* author : visor.bbs@bbs.yzu.edu.tw                     */
/* target : board status                                 */
/* create : 2003/08/17                                   */
/* update : NULL                                         */
/*-------------------------------------------------------*/

#undef  MODES_C
#include "bbs.h"

#ifdef  HAVE_COUNT_BOARD

static void
bstat_item(
int num,
const BSTAT *bstat)
{
    prints("%6d   %-8.8s    %6d    %6d    %6d    %6d\n", num,
           bstat->type, bstat->n_reads, bstat->n_posts, bstat->n_news, bstat->n_bans);
}

static int
bstat_cur(
XO *xo,
int pos)
{
    const BSTAT *const bstat = (const BSTAT *) xo_pool_base + pos;
    move(3 + pos - xo->top, 0);
    bstat_item(pos + 1, bstat);
    return XO_NONE;
}

static int
bstat_body(
XO *xo)
{
    const BSTAT *bstat;
    int num, max, tail;

    move(3, 0);
    clrtobot();
    max = xo->max;
    if (max <= 0)
    {
        outs("\n�m�ݪO�ϥθ�T�n�L����έp���\n");
        return XO_NONE;
    }

    num = xo->top;
    bstat = (const BSTAT *) xo_pool_base + num;
    tail = num + XO_TALL;
    max = BMIN(max, tail);

    do
    {
        bstat_item(++num, bstat++);
    }
    while (num < max);

    return XO_NONE;
}


static int
bstat_head(
XO *xo)
{
    BRD *brd;
    int chn;
    char buf[80];


    chn = xo->key;
    brd = bshm->bcache + chn;

    sprintf(buf, "�ݪO�ϥθ�T�G%s", brd->brdname);

    vs_head(buf, str_site);
    prints(NECK_BRDSTAT, d_cols, "")
    return bstat_body(xo);
}


static int
bstat_load(
XO *xo)
{
    xo_load(xo, sizeof(BSTAT));
    return bstat_body(xo);
}


static int
bstat_init(
XO *xo)
{
    xo_load(xo, sizeof(BSTAT));
    return bstat_head(xo);
}


static int
bstat_stat(
XO *xo)
{
    BRD *brd;
    int chn;
    char msg[80];

    chn = xo->key;
    if (chn >= 0)
    {
        brd = bshm->bcache + chn;
        sprintf(msg, "�ֿn�\\Ū�G%d�A�o��G%d�A��H�G%d�A�׫H�G%d", brd->n_reads, brd->n_posts, brd->n_news, brd->n_bans);
        pmsg(msg);
    }

    return XO_NONE;
}

static int
bstat_clear(
XO *xo)
{
    BRD *brd;
    int chn;
    char fpath[128];

    if (!HAS_PERM(PERM_ALLBOARD))
        return XO_NONE;

    if (vans("�T�w�M���Ҧ������ܡH[y/N]") == 'y')
    {

        chn = xo->key;
        if (chn >= 0)
        {
            brd = bshm->bcache + chn;
            brd_fpath(fpath, brd->brdname, FN_BRD_STATCOUNT);
            unlink(fpath);
            brd_fpath(fpath, brd->brdname, FN_BRD_STAT);
            unlink(fpath);
        }
    }

    return XO_INIT;
}

static int
bstat_help(
XO *xo)
{
    //film_out(FILM_BSTAT, -1);
    return XO_HEAD;
}


KeyFuncList bstat_cb =
{
    {XO_INIT, {bstat_init}},
    {XO_LOAD, {bstat_load}},
    {XO_HEAD, {bstat_head}},
    {XO_BODY, {bstat_body}},
    {XO_CUR | XO_POSF, {.posf = bstat_cur}},

    {'s', {xo_cb_init}},
    {'S', {bstat_stat}},
    {'c', {bstat_clear}},
    {'h', {bstat_help}}
};


int
main_bstat(
XO *xo,
int pos)
{
    DL_HOLD;
    XO *xx, last;
    char fpath[64];
    BRD *brd;
    short *chp;
    int num, chn;


    num = pos;
    chp = (short *) xo->xyz + num;
    chn = *chp;
    if (chn >= 0)
    {
        last = xz[XZ_OTHER - XO_ZONE].xo;  /* record */

        brd = bshm->bcache + chn;
        utmp_mode(M_OMENU);
        brd_fpath(fpath, brd->brdname, FN_BRD_STATCOUNT);
        xz[XZ_OTHER - XO_ZONE].xo = xx = xo_new(fpath);
        xx->cb = bstat_cb;
        xx->recsiz = sizeof(BSTAT);
        xx->pos = 0;
        xx->key = chn;
        xover(XZ_OTHER);
        free(xx);

        xz[XZ_OTHER - XO_ZONE].xo = last;  /* restore */
    }

    return DL_RELEASE(XO_HEAD);
}



#endif  /* #ifdef  HAVE_COUNT_BOARD */
