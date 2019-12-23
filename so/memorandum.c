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

extern XZ xz[];
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
            mytm->tm_mon + 1, mytm->tm_mday, myweek[mytm->tm_wday<<1],
            myweek[(mytm->tm_wday<<1)+1], mytm->tm_hour, mytm->tm_min);

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
        ch = igetch();
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

static void
memorandum_item(
int num,
const MEMORANDUM *memorandum)
{
    prints("%6d  %-8s  %-8s  %-*s\n", num, memorandum->date, memorandum->time, d_cols + 51, memorandum->work);
}

static int
memorandum_body(
XO *xo)
{
    MEMORANDUM *memorandum;
    int num, max, tail;

    move(3, 0);
    clrtobot();
    max = xo->max;
    if (max <= 0)
    {
        if (vans("要新增資料嗎(y/N)？[N] ") == 'y')
            return memorandum_add(xo);
        return XO_QUIT;
    }

    memorandum = (MEMORANDUM *) xo_pool;
    num = xo->top;
    tail = num + XO_TALL;
    max = BMIN(max, tail);

    do
    {
        memorandum_item(++num, memorandum++);
    }
    while (num < max);

    return XO_NONE;
}


static int
memorandum_head(
XO *xo)
{
    vs_head("聯絡名單", str_site);
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
    if (echo == DOECHO)
        memset(memorandum, 0, sizeof(MEMORANDUM));
    if (vget(b_lines, 0, "日期：", memorandum->date, sizeof(memorandum->date), echo)
        && vget(b_lines, 0, "時間:", memorandum->time, sizeof(memorandum->time), echo)
        && vget(b_lines, 0, "工作或行程:", memorandum->work, sizeof(memorandum->work), echo))
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
        vmsg("你的聯絡名單已到達上限!!");
    else if (memorandum_edit(&memorandum, DOECHO))
    {
        rec_add(xo->dir, &memorandum, sizeof(MEMORANDUM));
        xo->pos = XO_TAIL /* xo->max */ ;
        return memorandum_init(xo);
    }
    return memorandum_head(xo);
}

static int
memorandum_delete(
XO *xo)
{

    if (vans(msg_del_ny) == 'y')
    {
        if (!rec_del(xo->dir, sizeof(MEMORANDUM), xo->pos, NULL, NULL))
        {
            return memorandum_load(xo);
        }
    }
    return XO_FOOT;
}


static int
memorandum_change(
XO *xo)
{
    MEMORANDUM *memorandum, mate;
    int pos, cur;

    pos = xo->pos;
    cur = pos - xo->top;
    memorandum = (MEMORANDUM *) xo_pool + cur;

    mate = *memorandum;
    memorandum_edit(memorandum, GCARRY);
    if (memcmp(memorandum, &mate, sizeof(MEMORANDUM)))
    {
        rec_put(xo->dir, memorandum, sizeof(MEMORANDUM), pos);
        move(3 + cur, 0);
        memorandum_item(++pos, memorandum);
    }

    return XO_FOOT;
}

static int
memorandum_help(
XO *xo)
{
    film_out(FILM_MEMORANDUM, -1);
    return memorandum_head(xo);
}

KeyFunc memorandum_cb[] =
{
    {XO_INIT, {memorandum_init}},
    {XO_LOAD, {memorandum_load}},
    {XO_HEAD, {memorandum_head}},
    {XO_BODY, {memorandum_body}},

    {Ctrl('P'), {memorandum_add}},
    {'r', {memorandum_change}},
    {'c', {memorandum_change}},
    {'s', {memorandum_init}},
    {'d', {memorandum_delete}},
    {'h', {memorandum_help}}
};

int
Memorandum(void)
{
    XO *xo;
    char fpath[80];
    utmp_mode(M_OMENU);
    usr_fpath(fpath, cuser.userid, "memorandum");
    xz[XZ_OTHER - XO_ZONE].xo = xo = xo_new(fpath);
    xz[XZ_OTHER - XO_ZONE].cb = memorandum_cb;
    xo->pos = 0;
    xover(XZ_OTHER);
    free(xo);
    return 0;
}

