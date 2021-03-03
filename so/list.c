/*-------------------------------------------------------*/
/* list.c   ( YZU_CSE WindTop BBS )                     */
/*-------------------------------------------------------*/
/* author : visor.bbs@bbs.yzu.edu.tw                     */
/* target : administrator routines                       */
/* create : 2000/01/02                                   */
/* update : NULL                                         */
/*-------------------------------------------------------*/

#undef  MODES_C
#include "bbs.h"

static int ways;

static int list_add(XO *xo);

static int mode;


static void
list_item(
int num,
const LIST *list)
{
    prints("%6d     %s\n", num, list->userid);
}

static int
list_cur(
XO *xo,
int pos)
{
    const LIST *const list = (const LIST *) xo_pool_base + pos;
    move(3 + pos - xo->top, 0);
    list_item(pos + 1, list);
    return XO_NONE;
}

static int
list_body(
XO *xo)
{
    const LIST *list;
    int num, max, tail;

    move(3, 0);
    clrtobot();
    max = xo->max;
    if (max <= 0)
    {
        if (vans("要新增資料嗎(y/N)？[N] ") == 'y')
            return list_add(xo);
        return XO_QUIT;
    }

    num = xo->top;
    list = (const LIST *) xo_pool_base + num;
    tail = num + XO_TALL;
    max = BMIN(max, tail);

    do
    {
        list_item(++num, list++);
    }
    while (num < max);

    return XO_NONE;
}

static void
get_title(
char *title,
int item)
{
    LIST_TITLE list;
    int fd;
    char fpath[80];
    memset(&list, 0, sizeof(LIST_TITLE));
    usr_fpath(fpath, cuser.userid, mode ? "board" : "list");
    fd = open(fpath, O_RDONLY);
    if (fd >= 0)
    {
        read(fd, &list, sizeof(LIST_TITLE));
        close(fd);
    }
    strcpy(title, list.title[item]);
}


static int
list_head(
XO *xo)
{
    char buf[64], mid[41];
    sprintf(buf, "群組名單.%d ", ways);
    *mid = 0;
    get_title(mid, ways - 1);
    vs_head(buf, mid);
#ifdef HAVE_MULTI_CROSSPOST
    if (mode)
        prints(NECK_LISTBRD, d_cols, "");
    else
#endif
        prints(NECK_LISTUSR, d_cols, "");
    return list_body(xo);
}


static int
list_load(
XO *xo)
{
    xo_load(xo, sizeof(LIST));
    return list_body(xo);
}

static int
have_it(
const ACCT *acct,
const char *dir)
{
    LIST clist;
    int pos = 0, fd;
    fd = open(dir, O_RDONLY);
    while (fd >= 0)
    {
        lseek(fd, (off_t)(sizeof(LIST) * pos), SEEK_SET);
        if (read(fd, &clist, sizeof(LIST)) == sizeof(LIST))
        {
            if (!strcmp(clist.userid, acct->userid))
            {
                close(fd);
                return 1;
            }
            pos++;
        }
        else
        {
            close(fd);
            return 0;
        }
    }
    if (fd >= 0)
        close(fd);
    return 0;
}


static int
list_init(
XO *xo)
{
    xo_load(xo, sizeof(LIST));
    return list_head(xo);
}


static int
list_add(
XO *xo)
{
    LIST list;
    ACCT acct;
    int userno;
    BRD *brd = NULL;
#ifdef HAVE_MULTI_CROSSPOST
    char buf[20];
#endif

    if (xo->max >= PAL_MAX)
    {
        vmsg("您的群組名單太多，請善加整理");
        return XO_FOOT;
    }

#ifdef HAVE_MULTI_CROSSPOST
    if (mode)
    {
        brd = ask_board(buf, BRD_W_BIT, NULL);
        if (brd)
            userno = 1;
        else
            userno = 0;
    }
    else
#endif
    {
        userno = acct_get(msg_uid, &acct);

        if (have_it(&acct, xo->dir))
        {
            vmsg("群組名單中已有此人");
            return XO_FOOT;
        }
    }

    if (userno > 0)
    {
        strcpy(list.userid, mode ? brd->brdname : acct.userid);
        rec_add(xo->dir, &list, sizeof(LIST));
        xo->pos = XO_TAIL;
        xo_load(xo, sizeof(LIST));
    }
    return XO_HEAD;
}

static int
list_search(
XO *xo,
int pos)
{
    LIST list;
    int max, cur;
    char buf[IDLEN+1];

    cur=pos;
    max=xo->max;

    if (!vget(B_LINES_REF, 0, "關鍵字：", buf, sizeof(buf), DOECHO))
        return XO_FOOT;

    str_lower(buf, buf);

    while (++cur<max)
    {
        rec_get(xo->dir, &list, sizeof(LIST), cur);

        if (str_casestr(list.userid, buf))
        {
            return XR_FOOT + XO_MOVE + cur;

        }
    }



    return XO_FOOT;
}

static int
list_delete(
XO *xo,
int pos)
{

    if (vans(msg_del_ny) == 'y')
    {
        if (!rec_del(xo->dir, sizeof(LIST), pos, NULL, NULL))
        {
            return XO_LOAD;
        }
    }
    return XO_FOOT;
}

static int
list_title(
XO *xo)
{
    LIST_TITLE list;
    int fd;
    char fpath[80];

    memset(&list, 0, sizeof(LIST_TITLE));
    usr_fpath(fpath, cuser.userid, mode ? "board" : "list");
    fd = open(fpath, O_RDONLY);
    if (fd >= 0)
    {
        read(fd, &list, sizeof(LIST_TITLE));
        close(fd);
        vget(B_LINES_REF, 0, "請輸入群組名稱：", list.title[ways-1], 41, GCARRY);
    }

    if ((fd = open(fpath, O_WRONLY | O_CREAT | O_TRUNC, 0600)) >= 0
        && vans("確定嗎 [Y/n]：") != 'n')
    {
        write(fd, &list, sizeof(LIST_TITLE));
        close(fd);
    }

    return XO_INIT;
}

static int
list_help(
XO *xo)
{
    /*film_out(FILM_LIST, -1);*/
    more(FN_HELP_LIST, NULL);
    return XO_HEAD;
}

static int
list_browse(
XO *xo)
{
    return XO_NONE;
}

static int list_mode(XO *xo);
#ifdef HAVE_MULTI_CROSSPOST
static int list_board(XO *xo);
#endif

KeyFuncList list_cb =
{
    {XO_INIT, {list_init}},
    {XO_LOAD, {list_load}},
    {XO_HEAD, {list_head}},
    {XO_BODY, {list_body}},
    {XO_CUR | XO_POSF, {.posf = list_cur}},

    {'r', {list_browse}},
    {'T', {list_title}},
    {'a', {list_add}},
    {'r', {list_help}},
#ifdef HAVE_MULTI_CROSSPOST
    {'F', {list_board}},
#endif
    {'s', {xo_cb_init}},
    {'d' | XO_POSF, {.posf = list_delete}},
    {KEY_TAB, {list_mode}},
    {'/' | XO_POSF, {.posf = list_search}},
    {'h', {list_help}}
};


static int
list_mode(
XO *xo)
{
    char fpath[128];
    char buf[32];
    if (++ways > MAX_LIST)
        ways = 1;
    sprintf(buf, mode ? "board.%d" : "list.%d", ways);
    usr_fpath(fpath, cuser.userid, buf);
    free(xz[XZ_OTHER - XO_ZONE].xo);
    xz[XZ_OTHER - XO_ZONE].xo = xo = xo_new(fpath);
    xo->cb = list_cb;
    xo->recsiz = sizeof(LIST);
    xo->pos = 0;
    return XO_INIT;
}

#ifdef HAVE_MULTI_CROSSPOST
static int
list_board(
XO *xo)
{
    char buf[32], fpath[128];
    if (!HAS_PERM(PERM_ALLBOARD))
        return XO_NONE;
    if (mode)
        mode = 0;
    else
        mode = 1;
    sprintf(buf, mode ? "board.%d" : "list.%d", ways);
    usr_fpath(fpath, cuser.userid, buf);
    free(xz[XZ_OTHER - XO_ZONE].xo);
    xz[XZ_OTHER - XO_ZONE].xo = xo = xo_new(fpath);
    xo->cb = list_cb;
    xo->recsiz = sizeof(LIST);
    xo->pos = 0;
    return XO_INIT;
}
#endif

int
List(void)
{
    DL_HOLD;
    XO *xo, *last;
    char fpath[128], msg[100];
    char buf[10];
    int i;

#ifdef HAVE_MULTI_CROSSPOST
    if (HAS_PERM(PERM_ALLBOARD))
    {
        i = vans("請選擇群組種類 1)使用者 2)看板 [1]") - '0';
        if (i == 2)
            mode = 1;
        else
            mode = 0;
    }
#endif

    move(11, 0);
    clrtobot();
    prints("\n      %4s   %-18s\n", "編號", "群組名稱");
    outsep(b_cols, msg_separator);
    outc('\n');
    for (i = 1; i <= MAX_LIST; i++)
    {
        get_title(msg, i - 1);
        prints("      %4d   %s..\n", i, msg);
    }



    sprintf(msg, "第幾個群組名單 [1~%d]：", MAX_LIST);
    if (!vget(B_LINES_REF, 0, msg, buf, 3, DOECHO))
        return DL_RELEASE(0);
    ways = atoi(buf);
    if (ways > MAX_LIST || ways < 1)
        return DL_RELEASE(0);
    utmp_mode(M_OMENU);
    sprintf(buf, mode ? "board.%d" : "list.%d", ways);
    usr_fpath(fpath, cuser.userid, buf);

    last = xz[XZ_OTHER - XO_ZONE].xo;  /* record */

    xz[XZ_OTHER - XO_ZONE].xo = xo = xo_new(fpath);
    xo->cb = list_cb;
    xo->recsiz = sizeof(LIST);
    xo->pos = 0;
    xover(XZ_OTHER);
    free(xo);

    xz[XZ_OTHER - XO_ZONE].xo = last;  /* restore */

    return DL_RELEASE(0);
}



