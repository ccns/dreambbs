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

extern XZ xz[];

static int ways;

static int list_add(XO *xo);

static int mode;


static void
list_item(
int num,
LIST *list)
{
    prints("%6d     %s\n", num, list->userid);
}

static int
list_body(
XO *xo)
{
    LIST *list;
    int num, max, tail;

    move(3, 0);
    clrtobot();
    max = xo->max;
    if (max <= 0)
    {
        if (vans("�n�s�W��ƶ�(Y/N)�H[N] ") == 'y')
            return list_add(xo);
        return XO_QUIT;
    }

    list = (LIST *) xo_pool;
    num = xo->top;
    tail = num + XO_TALL;
    if (max > tail)
        max = tail;

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
    sprintf(buf, "�s�զW��.%d ", ways);
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
ACCT *acct,
char *dir)
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
        vmsg("�z���s�զW��Ӧh�A�е��[��z");
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
            vmsg("�s�զW�椤�w�����H");
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
    return list_head(xo);
}

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
    free(xo);
    xz[XZ_OTHER - XO_ZONE].xo = xo = xo_new(fpath);
    xo->pos = 0;
    return XO_INIT;
}

static int
list_search(
XO *xo)
{
    LIST list;
    int max, cur;
    char buf[IDLEN+1];

    cur=xo->pos;
    max=xo->max;

    if (!vget(b_lines, 0, "����r�G", buf, sizeof(buf), DOECHO))
        return XO_NONE;

    str_lower(buf, buf);

    while (++cur<max)
    {
        rec_get(xo->dir, &list, sizeof(LIST), cur);

        if (str_str(list.userid, buf))
        {
            xo->pos = xo->top + cur;
            return list_init(xo);

        }
    }



    return XO_NONE;
}

static int
list_delete(
XO *xo)
{

    if (vans(msg_del_ny) == 'y')
    {
        if (!rec_del(xo->dir, sizeof(LIST), xo->pos, NULL, NULL))
        {
            return list_load(xo);
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
        vget(b_lines, 0, "�п�J�s�զW�١G", list.title[ways-1], 41, GCARRY);
    }

    if ((fd = open(fpath, O_WRONLY | O_CREAT | O_TRUNC, 0600)) >= 0
        && vans("�T�w�� [Y/n]�G") != 'n')
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
    return list_head(xo);
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
    free(xo);
    xz[XZ_OTHER - XO_ZONE].xo = xo = xo_new(fpath);
    xo->pos = 0;
    return list_init(xo);
}
#endif

static int
list_browse(
XO *xo)
{
    return XO_NONE;
}


KeyFunc list_cb[] =
{
    {XO_INIT, list_init},
    {XO_LOAD, list_load},
    {XO_HEAD, list_head},
    {XO_BODY, list_body},

    {'r', list_browse},
    {'T', list_title},
    {'a', list_add},
    {'r', list_help},
#ifdef HAVE_MULTI_CROSSPOST
    {'F', list_board},
#endif
    {'s', list_init},
    {'d', list_delete},
    {KEY_TAB, list_mode},
    {'/', list_search},
    {'h', list_help}
};


int
List(void)
{
    XO *xo;
    char fpath[128], msg[100];
    char buf[10];
    int i;

#ifdef HAVE_MULTI_CROSSPOST
    if (HAS_PERM(PERM_ALLBOARD))
    {
        i = vans("�п�ܸs�պ��� 1)�ϥΪ� 2)�ݪO [1]") - '0';
        if (i == 2)
            mode = 1;
        else
            mode = 0;
    }
#endif

    move(11, 0);
    clrtobot();
    prints("\n      %4s   %-18s\n%s\n", "�s��", "�s�զW��", msg_separator);
    for (i = 1; i <= MAX_LIST; i++)
    {
        get_title(msg, i - 1);
        prints("      %4d   %s..\n", i, msg);
    }



    sprintf(msg, "�ĴX�Ӹs�զW�� [1~%d]�G", MAX_LIST);
    if (!vget(b_lines, 0, msg, buf, 3, DOECHO))
        return 0;
    ways = atoi(buf);
    if (ways > MAX_LIST || ways < 1)
        return 0;
    utmp_mode(M_OMENU);
    sprintf(buf, mode ? "board.%d" : "list.%d", ways);
    usr_fpath(fpath, cuser.userid, buf);
    xz[XZ_OTHER - XO_ZONE].xo = xo = xo_new(fpath);
    xz[XZ_OTHER - XO_ZONE].cb = list_cb;
    xo->pos = 0;
    xover(XZ_OTHER);
    free(xo);
    return 0;
}



