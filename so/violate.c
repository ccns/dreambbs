/*-------------------------------------------------------*/
/* viol.c   ( YZU_CSE WindTop BBS )                     */
/*-------------------------------------------------------*/
/* author : visor.bbs@bbs.yzu.edu.tw                     */
/* target : administrator routines                       */
/* create : 2000/01/02                                   */
/* update : NULL                                         */
/*-------------------------------------------------------*/

#undef  MODES_C
#include "bbs.h"

static int viol_add(XO *xo);


static void
viol_item(
int num,
const EMAIL *viol)
{
    char buf[5];
    int now;
    now = (viol->deny - time(0)) / 3600;

    if (viol->deny == -1)
        sprintf(buf, "%s", "永久");
    else
        sprintf(buf, "%4d", BMAX(now, 0));
    prints("%6d %4d %4s %-*.*s\n", num, viol->times, buf, d_cols + 62, d_cols + 62, viol->email);
}

static int
viol_body(
XO *xo)
{
    const EMAIL *viol;
    int num, max, tail;

    move(3, 0);
    clrtobot();
    max = xo->max;
    if (max <= 0)
    {
        if (vans("要新增資料嗎(y/N)？[N] ") == 'y')
            return viol_add(xo);
        return XO_QUIT;
    }

    num = xo->top;
    viol = (const EMAIL *) xo_pool_base + num;
    tail = num + XO_TALL;
    max = BMIN(max, tail);

    do
    {
        viol_item(++num, viol++);
    }
    while (num < max);

    return XO_NONE;
}


static int
viol_head(
XO *xo)
{
    vs_head("暫時禁止名單", str_site);
    prints(NECK_VIOL, d_cols, "");
    return viol_body(xo);
}


static int
viol_load(
XO *xo)
{
    xo_load(xo, sizeof(EMAIL));
    return viol_body(xo);
}


static int
viol_init(
XO *xo)
{
    xo_load(xo, sizeof(EMAIL));
    return viol_head(xo);
}


static int
viol_edit(
EMAIL *viol,
int echo)
{
    if (echo == DOECHO)
        memset(viol, 0, sizeof(EMAIL));
    if (vget(B_LINES_REF, 0, "E-mail：", viol->email, sizeof(viol->email), echo))
        return 1;
    else
        return 0;
}


static int
viol_add(
XO *xo)
{
    EMAIL viol;

    if (viol_edit(&viol, DOECHO))
    {
        rec_add(xo->dir, &viol, sizeof(EMAIL));
        xo->pos = XO_TAIL;
        xo_load(xo, sizeof(EMAIL));
    }
    return XO_HEAD;
}

static int
viol_delete(
XO *xo)
{

    if (vans(msg_del_ny) == 'y')
    {
        if (!rec_del(xo->dir, sizeof(EMAIL), xo->pos, NULL, NULL))
        {
            return XO_LOAD;
        }
    }
    return XO_FOOT;
}


static int
viol_change(
XO *xo)
{
    EMAIL *viol, mate;
    int pos, cur;

    pos = xo->pos;
    cur = pos - xo->top;
    viol = (EMAIL *) xo_pool_base + pos;

    mate = *viol;
    viol_edit(viol, GCARRY);
    if (memcmp(viol, &mate, sizeof(EMAIL)))
    {
        rec_put(xo->dir, viol, sizeof(EMAIL), pos);
        move(3 + cur, 0);
        viol_item(++pos, viol);
        cursor_show(3 + cur, 0);
    }

    return XO_FOOT;
}

static int
viol_find(
XO *xo)
{
    EMAIL viol;
    int pos, fd;
    char buf[64];

    if (!vget(B_LINES_REF, 0, "請輸入查詢字串:", buf, sizeof(buf), DOECHO))
        return XO_FOOT;

    fd = open(FN_VIOLATELAW_DB, O_RDONLY);

    pos = xo->pos + 1;

    while (fd >= 0)
    {
        lseek(fd, (off_t)(sizeof(EMAIL) * pos), SEEK_SET);
        if (read(fd, &viol, sizeof(EMAIL)) == sizeof(EMAIL))
        {
            if (str_str(viol.email, buf))
            {
                xo->pos = pos;
                close(fd);
                return XO_INIT;
            }
            pos++;
        }
        else
        {
            close(fd);
            break;
        }
    }

    return XO_INIT;
}


static int
viol_help(
XO *xo)
{
    /*film_out(FILM_EMAIL, -1);*/
    return XO_HEAD;
}


KeyFuncList viol_cb =
{
    {XO_INIT, {viol_init}},
    {XO_LOAD, {viol_load}},
    {XO_HEAD, {viol_head}},
    {XO_BODY, {viol_body}},

    {Ctrl('P'), {viol_add}},
    {'r', {viol_change}},
    {'f', {viol_find}},
    {'c', {viol_change}},
    {'s', {xo_cb_init}},
    {'d', {viol_delete}},
    {'h', {viol_help}}
};


int
Violate(void)
{
    DL_HOLD;
    XO *xo, *last;
    char fpath[64];

    last = xz[XZ_OTHER - XO_ZONE].xo;  /* record */

    utmp_mode(M_OMENU);
    sprintf(fpath, FN_VIOLATELAW_DB);
    xz[XZ_OTHER - XO_ZONE].xo = xo = xo_new(fpath);
    xo->cb = viol_cb;
    xo->recsiz = sizeof(EMAIL);
    xo->pos = 0;
    xover(XZ_OTHER);
    free(xo);

    xz[XZ_OTHER - XO_ZONE].xo = last;  /* restore */

    return DL_RELEASE(0);
}



