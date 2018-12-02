/*-------------------------------------------------------*/
/* talk.c	( NTHU CS MapleBBS Ver 3.00 )		 */
/*-------------------------------------------------------*/
/* target : talk/query/friend(pal) routines	 	 */
/* create : 95/03/29				 	 */
/* update : 2000/01/02				 	 */
/*-------------------------------------------------------*/

#undef	_MODES_C_


#include "bbs.h"

static int aloha_add(XO *xo);
static int aloha_loadpal(XO *xo);
extern XZ xz[];

/* ----------------------------------------------------- */
/* 上站通知名單			           Jerics 2k.01  */
/* ----------------------------------------------------- */

int
cmpbmw(
BMW *benz)
{
    return benz->recver == cuser.userno;
}

static int
aloha_find(
char *fpath,
ALOHA *aloha)
{
    ALOHA new;
    int pos = 0, fd;
    fd = open(fpath, O_RDONLY);
    while (fd)
    {
        lseek(fd, (off_t)(sizeof(ALOHA) * pos), SEEK_SET);
        if (read(fd, &new, sizeof(ALOHA)) == sizeof(ALOHA))
        {
            if (aloha->userno == new.userno)
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
    return 0;
}

static void
aloha_item(
int num,
ALOHA *aloha)
{
    prints("%6d     %s\n", num, aloha->userid);
}

static int
aloha_body(
XO *xo)
{
    ALOHA *aloha;
    int num, max, tail;

    move(3, 0);
    clrtobot();
    max = xo->max;
    if (max <= 0)
    {
        if (vans("要新增資料嗎(Y/N)？[N] ") == 'y')
        {
            if (vans("新增單人或引入好友名單(A/F)？[A] ") == 'f')
                return aloha_loadpal(xo);
            else
                return aloha_add(xo);
        }
        return XO_QUIT;
    }

    aloha = (ALOHA *) xo_pool;
    num = xo->top;
    tail = num + XO_TALL;
    if (max > tail)
        max = tail;

    do
    {
        aloha_item(++num, aloha++);
    }
    while (num < max);

    return XO_NONE;
}


static int
aloha_head(
XO *xo)
{
    vs_head("上站通知名單", str_site);
    outs(
        "  [←]離開 a)新增 d)刪除 s)重整 f)引入好友名單 [h]elp\n"
        "\033[34;47m  編號     上 站 通 知 名 單                                                  \033[m");
    return aloha_body(xo);
}


static int
aloha_load(
XO *xo)
{
    xo_load(xo, sizeof(ALOHA));
    return aloha_body(xo);
}


static int
aloha_init(
XO *xo)
{
    xo_load(xo, sizeof(ALOHA));
    return aloha_head(xo);
}

static int
aloha_loadpal(
XO *xo)
{
    int pos, i, max;
    char fpath[64], path[64];
    BMW bmw;
    PAL pal;
    ALOHA aloha;
    pos = xo->pos;
    max = xo->max;
    if (vans("要引入好友名單嗎(Y/N)？[N] ") == 'y')
    {
        usr_fpath(fpath, cuser.userid, FN_PAL);
        for (i = 0; max < MAX_ALOHA; i++)
        {
            if (rec_get(fpath, &pal, sizeof(PAL), i) >= 0)
            {
                strcpy(aloha.userid, pal.userid);
                aloha.userno = pal.userno;
                if (!aloha_find(xo->dir, &aloha))
                {
                    bmw.recver = cuser.userno;
                    strcpy(bmw.userid, cuser.userid);
                    usr_fpath(path, aloha.userid, FN_FRIEND_BENZ);
                    rec_add(path, &bmw, sizeof(BMW));
                    rec_add(xo->dir, &aloha, sizeof(ALOHA));
                    xo->pos = XO_TAIL;
                    max++;
                }
            }
            else break;
        }
        xo_load(xo, sizeof(ALOHA));
    }
    return aloha_head(xo);
}

static int
aloha_add(
XO *xo)
{
    char path[64];
    BMW bmw;
    ACCT acct;
    ALOHA aloha;
    if ((aloha.userno = acct_get(msg_uid, &acct)) <= 0)
        return XO_HEAD;
    strcpy(aloha.userid, acct.userid);

    if (aloha.userno == cuser.userno)
    {
        vmsg("自己不須加入上站通知名單中");
        return XO_HEAD;
    }
    if (xo->max >= MAX_ALOHA)
    {
        vmsg("已超過最大上限嘍 !!");
        return XO_HEAD;
    }

    bmw.recver = cuser.userno;
    strcpy(bmw.userid, cuser.userid);
    usr_fpath(path, aloha.userid, FN_FRIEND_BENZ);
    if (!aloha_find(xo->dir, &aloha))
    {
        rec_add(path, &bmw, sizeof(BMW));
        rec_add(xo->dir, &aloha, sizeof(ALOHA));
    }
    xo->pos = XO_TAIL /* xo->max */ ;
    xo_load(xo, sizeof(ALOHA));

    return aloha_head(xo);
}

static int
aloha_rangedel(
XO *xo)
{
    char buf[8];
    int head, tail, fd;

    vget(b_lines, 0, "[設定刪除範圍] 起點：", buf, 6, DOECHO);
    head = atoi(buf);
    if (head <= 0)
    {
        zmsg("起點有誤");
        return XO_FOOT;
    }

    vget(b_lines, 28, "終點：", buf, 6, DOECHO);
    tail = atoi(buf);
    if (tail < head)
    {
        zmsg("終點有誤");
        return XO_FOOT;
    }
    if (tail > xo->max)
        tail = xo->max;


    if (vget(b_lines, 41, msg_sure_ny, buf, 3, LCECHO) == 'y')
    {
        char fpath[64];
        int size;
        ALOHA *aloha, *ahead, *atail, *abase;
        struct stat st;

        fd = open(xo->dir, O_RDONLY);
        fstat(fd, &st);
        size = st.st_size;
        abase = (ALOHA *) malloc(size);
        size = read(fd, abase, size);
        close(fd);

        ahead = abase + (head - 1);
        atail = abase + (tail - 1);

        for (aloha = ahead; aloha <= atail; aloha++)
        {
            usr_fpath(fpath, aloha->userid, FN_FRIEND_BENZ);
            while (rec_loc(fpath, sizeof(BMW), cmpbmw) >= 0)
                rec_del(fpath, sizeof(BMW), 0, cmpbmw, NULL);
            rec_del(xo->dir, sizeof(ALOHA), head - 1, NULL, NULL);
        }
        free(abase);
        return aloha_init(xo);
    }
    return XO_FOOT;
}

static int
aloha_delete(
XO *xo)
{
    if (vans(msg_del_ny) == 'y')
    {
        char fpath[64];
        ALOHA *aloha;
        aloha = (ALOHA *) xo_pool + (xo->pos - xo->top);

        usr_fpath(fpath, aloha->userid, FN_FRIEND_BENZ);
        while (rec_loc(fpath, sizeof(BMW), cmpbmw) >= 0)
            rec_del(fpath, sizeof(BMW), 0, cmpbmw, NULL);
        rec_del(xo->dir, sizeof(ALOHA), xo->pos, NULL, NULL);
        return aloha_init(xo);
    }
    return XO_FOOT;
}

static int
aloha_help(
XO *xo)
{
    film_out(FILM_ALOHA, -1);
    return aloha_head(xo);
}


KeyFunc aloha_cb[] =
{
    {XO_INIT, aloha_init},
    {XO_LOAD, aloha_load},
    {XO_HEAD, aloha_head},
    {XO_BODY, aloha_body},

    {'a', aloha_add},
    {'D', aloha_rangedel},
    {'f', aloha_loadpal},
#if 0
    {'r', aloha_change},
    {'c', aloha_change},
#endif
    {'s', aloha_init},
    {'d', aloha_delete},
    {'h', aloha_help}
};


int
t_aloha(void)
{
    XO *xo;
    char fpath[64];

    utmp_mode(M_OMENU);
    usr_fpath(fpath, cuser.userid, FN_ALOHA);
    xz[XZ_OTHER - XO_ZONE].xo = xo = xo_new(fpath);
    xz[XZ_OTHER - XO_ZONE].cb = aloha_cb;
    xover(XZ_OTHER);
    free(xo);
    return 0;
}

