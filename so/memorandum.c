/*-------------------------------------------------------*/
/* memorandum.c   ( YZU_CSE Train BBS )                  */
/*-------------------------------------------------------*/
/* author : Jerics.bbs@bbs.yzu.edu.tw                    */
/* target : memorandum                                   */
/* create : 2000/01/12                                   */
/* update : NULL                                         */
/*-------------------------------------------------------*/

#undef  MODES_C
#include "bbs.h"

static int memorandum_add(XO *xo);
#if 0
char *
get_date(
const time_t *clock)
{
    static char foo[24];
    static const char myweek[] = "天一二三四五六";
    struct tm *mytm = localtime(clock);
    sprintf(foo, "[%d/%d]星期%c%c[%d:%02d]",
            mytm->tm_mon + 1, mytm->tm_mday, myweek[2 * mytm->tm_wday],
            myweek[2 * mytm->tm_wday + 1], mytm->tm_hour, mytm->tm_min);

    return (foo);
}

time_t
get_sch_time(void)
{
    const char *const t_max = "19392959";
    char ch, buf[8];
    time_t sch_time;
    struct tm ptime;
    int i;
    move(23, 0);
    clrtobot();
    outs("請輸入時間(24小時制)：  月  日  時  分");

    for (i = 0; i < 8; i++)
    {
        move(23, 23 + i / 2*4 + i % 2);
        ch = vkey();
        if (ch == 'q' || ch == 'Q') return 0;
        else if (ch == '\r')
        {
            i -= 1;
            continue;
        }
        else if (ch == '\177')
        {
            if (i)i -= 2;
            else i = -1;
            continue;
        }
        if (ch >= '0' && ch <= t_max[i])
        {
            outc(ch);
            buf[i] = ch - '0';
        }
        else
        {
            bell();
            i--;
        }
    }
    ptime.tm_year = 0;
    ptime.tm_mon  = buf[0] * 10 + buf[1] - 1;
    ptime.tm_mday = buf[2] * 10 + buf[3];
    ptime.tm_hour = buf[4] * 10 + buf[5] + 1;
    ptime.tm_min  = buf[6] * 10 + buf[7];
    ptime.tm_sec = 0;
    sch_time  = mktime(&ptime);
    prints("您剛剛輸入的時間是:%s", get_date(&sch_time));
    return sch_time;
}
#endif  /* #if 0 */

static int
memorandum_item(
XO *xo,
int pos)
{
    const MEMORANDUM *const memorandum = (const MEMORANDUM *) xo_pool_base + pos;
    const int num = pos + 1;
    prints("%6d  %-8s  %-8s  %-*s\n", num, memorandum->date, memorandum->time, d_cols + 51, memorandum->work);
    return XO_NONE;
}

static int
memorandum_cur(
XO *xo,
int pos)
{
    move(3 + pos - xo->top, 0);
    return memorandum_item(xo, pos);
}

static int
memorandum_body(
XO *xo)
{
    int num, max, tail;

    move(3, 0);
    clrtobot();
    max = xo->max;
    if (max <= 0)
    {
        outs("\n《備忘錄》目前沒有資料\n");
        outs("\n  (^P)新增資料\n");
        return XO_NONE;
    }

    num = xo->top;
    tail = num + XO_TALL;
    max = BMIN(max, tail);

    do
    {
        memorandum_item(xo, num++);
    }
    while (num < max);

    return XO_NONE;
}


static int
memorandum_head(
XO *xo)
{
    vs_head("備忘錄", str_site);
    prints(NECK_MEMORANDUM, d_cols, "");
    return memorandum_body(xo);
}


static int
memorandum_load(
XO *xo)
{
    xo_load(xo, sizeof(MEMORANDUM));
    return memorandum_body(xo);
}


static int
memorandum_init(
XO *xo)
{
    xo_load(xo, sizeof(MEMORANDUM));
    return memorandum_head(xo);
}


static int
memorandum_edit(
MEMORANDUM *memorandum,
int echo)
{
    if (!(echo & GCARRY))
        memset(memorandum, 0, sizeof(MEMORANDUM));
    if (vget(B_LINES_REF, 0, "日期：", memorandum->date, sizeof(memorandum->date), echo)
        && vget(B_LINES_REF, 0, "時間:", memorandum->time, sizeof(memorandum->time), echo)
        && vget(B_LINES_REF, 0, "工作或行程:", memorandum->work, sizeof(memorandum->work), echo))
        return 1;
    else
        return 0;
}


static int
memorandum_add(
XO *xo)
{
    MEMORANDUM memorandum;
    if (xo->max >= MAX_MEMORANDUM)
        vmsg_xo(xo, "你的備忘錄已到達上限!!");
    else if (memorandum_edit(&memorandum, DOECHO))
    {
        rec_add(xo->dir, &memorandum, sizeof(MEMORANDUM));
        xo->pos[xo->cur_idx] = XO_TAIL /* xo->max */ ;
        return XO_INIT;
    }
    return XO_HEAD;
}

static int
memorandum_delete(
XO *xo,
int pos)
{

    if (vans_xo(xo, msg_del_ny) == 'y')
    {
        if (!rec_del(xo->dir, sizeof(MEMORANDUM), pos, NULL, NULL))
        {
            return XO_LOAD;
        }
    }
    return XO_FOOT;
}


static int
memorandum_change(
XO *xo,
int pos)
{
    MEMORANDUM *memorandum, mate;

    memorandum = (MEMORANDUM *) xo_pool_base + pos;

    mate = *memorandum;
    memorandum_edit(memorandum, GCARRY);
    if (memcmp(memorandum, &mate, sizeof(MEMORANDUM)))
    {
        rec_put(xo->dir, memorandum, sizeof(MEMORANDUM), pos);
        return XR_FOOT + XO_CUR;
    }

    return XO_FOOT;
}

static int
memorandum_help(
XO *xo)
{
    film_out(FILM_MEMORANDUM, -1);
    return XO_HEAD;
}

KeyFuncList memorandum_cb =
{
    {XO_INIT, {memorandum_init}},
    {XO_LOAD, {memorandum_load}},
    {XO_HEAD, {memorandum_head}},
    {XO_BODY, {memorandum_body}},
    {XO_CUR | XO_POSF, {.posf = memorandum_cur}},

    {Ctrl('P'), {memorandum_add}},
    {'r' | XO_POSF, {.posf = memorandum_change}},
    {'c' | XO_POSF, {.posf = memorandum_change}},
    {'s', {xo_cb_init}},
    {'d' | XO_POSF, {.posf = memorandum_delete}},
    {'h', {memorandum_help}}
};

int
Memorandum(void)
{
    DL_HOLD;
    XO *xo, *last;
    char fpath[80];

    last = xz[XZ_OTHER - XO_ZONE].xo;  /* record */

    utmp_mode(M_OMENU);
    usr_fpath(fpath, cuser.userid, "memorandum");
    xz[XZ_OTHER - XO_ZONE].xo = xo = xo_new(fpath);
    xo->cb = memorandum_cb;
    xo->recsiz = sizeof(MEMORANDUM);
    for (int i = 0; i < COUNTOF(xo->pos); ++i)
        xo->pos[i] = 0;
    xover(XZ_OTHER);
    free(xo);

    xz[XZ_OTHER - XO_ZONE].xo = last;  /* restore */

    return DL_RELEASE(0);
}

