/*-------------------------------------------------------*/
/* admin.c   ( YZU_CSE WindTop BBS )                     */
/*-------------------------------------------------------*/
/* author : visor.bbs@bbs.yzu.edu.tw                     */
/* target : administrator routines                       */
/* create : 2000/01/02                                   */
/* update : NULL                                         */
/*-------------------------------------------------------*/

#undef  MODES_C
#include "bbs.h"

static int admin_add(XO *xo);


static void
admin_item(
int num,
const ADMIN *admin)
{
    prints("%6d     %s\n", num, admin->name);
}

static int
admin_cur(
XO *xo,
int pos)
{
    const ADMIN *const admin = (const ADMIN *) xo_pool_base + pos;
    move(3 + pos - xo->top, 0);
    admin_item(pos + 1, admin);
    return XO_NONE;
}

static int
admin_body(
XO *xo)
{
    const ADMIN *admin;
    int num, max, tail;

    move(3, 0);
    clrtobot();
    max = xo->max;
    if (max <= 0)
    {
        if (vans("要新增資料嗎(y/N)？[N] ") == 'y')
            return admin_add(xo);
        return XO_QUIT;
    }

    num = xo->top;
    admin = (const ADMIN *) xo_pool_base + num;
    tail = num + XO_TALL;
    max = BMIN(max, tail);

    do
    {
        admin_item(++num, admin++);
    }
    while (num < max);

    return XO_NONE;
}


static int
admin_head(
XO *xo)
{
    vs_head("超級站務", str_site);
    prints(NECK_ADMIN, d_cols, "");
    return admin_body(xo);
}


static int
admin_load(
XO *xo)
{
    xo_load(xo, sizeof(ADMIN));
    return admin_body(xo);
}


static int
admin_init(
XO *xo)
{
    xo_load(xo, sizeof(ADMIN));
    return admin_head(xo);
}


static int
admin_edit(
ADMIN *admin,
int echo)
{
    if (echo == DOECHO)
        memset(admin, 0, sizeof(ADMIN));
    if (vget(B_LINES_REF, 0, "超級站務列表：", admin->name, sizeof(admin->name), echo))
        return 1;
    else
        return 0;
}


static int
admin_add(
XO *xo)
{
    ADMIN admin;

    if (admin_edit(&admin, DOECHO))
    {
        rec_add(xo->dir, &admin, sizeof(ADMIN));
        xo->pos = XO_TAIL /* xo->max */ ;
        xo_load(xo, sizeof(ADMIN));
    }
    return XO_HEAD;
}

static int
admin_delete(
XO *xo,
int pos)
{

    if (vans(msg_del_ny) == 'y')
    {
        if (!rec_del(xo->dir, sizeof(ADMIN), pos, NULL, NULL))
        {
            return XO_LOAD;
        }
    }
    return XO_FOOT;
}


static int
admin_change(
XO *xo,
int pos)
{
    ADMIN *admin, mate;

    admin = (ADMIN *) xo_pool_base + pos;

    mate = *admin;
    admin_edit(admin, GCARRY);
    if (memcmp(admin, &mate, sizeof(ADMIN)))
    {
        rec_put(xo->dir, admin, sizeof(ADMIN), pos);
        return XR_FOOT + XO_CUR;
    }

    return XO_FOOT;
}

static int
admin_help(
XO *xo)
{
    film_out(FILM_ADMIN, -1);
    return XO_HEAD;
}


KeyFuncList admin_cb =
{
    {XO_INIT, {admin_init}},
    {XO_LOAD, {admin_load}},
    {XO_HEAD, {admin_head}},
    {XO_BODY, {admin_body}},
    {XO_CUR | XO_POSF, {.posf = admin_cur}},

    {Ctrl('P'), {admin_add}},
    {'r' | XO_POSF, {.posf = admin_change}},
    {'c' | XO_POSF, {.posf = admin_change}},
    {'s', {xo_cb_init}},
    {'d' | XO_POSF, {.posf = admin_delete}},
    {'h', {admin_help}}
};


int
Admin(void)
{
    DL_HOLD;
    XO *xo, *last;
    char fpath[64];
    if (!check_admin(cuser.userid) && str_casecmp(cuser.userid, SYSOPNAME))
    {
        vmsg("◎ 你不是系統管理員！");
        return DL_RELEASE(0);
    }

    last = xz[XZ_OTHER - XO_ZONE].xo;  /* record */

    utmp_mode(M_OMENU);
    sprintf(fpath, FN_ETC_ADMIN_DB);
    xz[XZ_OTHER - XO_ZONE].xo = xo = xo_new(fpath);
    xo->cb = admin_cb;
    xo->recsiz = sizeof(ADMIN);
    xo->pos = 0;
    xover(XZ_OTHER);
    free(xo);

    xz[XZ_OTHER - XO_ZONE].xo = last;  /* restore */

    return DL_RELEASE(0);
}



