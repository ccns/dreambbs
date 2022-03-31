/*-------------------------------------------------------*/
/* util/utmp-dump.c     ( YZU WindTopBBS Ver 3.00 )      */
/*-------------------------------------------------------*/
/* target :                                              */
/* create :                                              */
/* update :                                              */
/*-------------------------------------------------------*/

#define MODES_C

#include "bbs.h"

ACCT cuser;
UTMP *cutmp, utmp;
int total_num;

static int pal_count;
static int *pal_pool;
static UCACHE *ushm;
static int can_see(const UTMP *up);
static bool can_message(const UTMP *up);
typedef UTMP *pickup;


/* ----------------------------------------------------- */
/* 記錄 pal 的 user number                               */
/* ----------------------------------------------------- */

#define PICKUP_WAYS     COUNTOF(msg_pickup_way)

static int pickup_way;

const char *
bmode(
    const UTMP *up,
    int simple)
{
    static char modestr[32];
    int mode;
    const char *word;

    if (!up)
        return "不在站上";

    mode = up->mode;
    if (mode == M_IDLE)
    {
        word = up->mateid;
    }
    else
    {
        word = ModeTypeTable[mode];
    }

    if (simple)
        return (word);

    if (mode < M_TALK || mode > M_QUERY)
        return (word);

    if ((mode != M_QUERY && !HAS_PERM(PERM_SEECLOAK) && (up->ufo & UFO_CLOAK))||(can_see(up)==2 && !HAS_PERM(PERM_SYSOP)))
        return (word);

    sprintf(modestr, "%s %s", word, up->mateid);
    return (modestr);
}


/* ----------------------------------------------------- */
/* 好友名單：查詢狀態、友誼描述                          */
/* ----------------------------------------------------- */


GCC_PURE static int
can_see(
    const UTMP *up)
{
    int count, datum, mid;
    const int *cache;

    if ((cache = up->pal_spool))
    {
        for (count = up->pal_max; count > 0;)
        {
            datum = cache[mid = count >> 1];
            if ((-cuser.userno) == datum)
                return 2;
            if ((-cuser.userno) > datum)
            {
                cache += (++mid);
                count -= mid;
            }
            else
            {
                count = mid;
            }
        }
    }
    if ((cache = up->pal_spool))
    {
        for (count = up->pal_max; count > 0;)
        {
            datum = cache[mid = count >> 1];
            if (cuser.userno == datum)
                return 1;
            if (cuser.userno > datum)
            {
                cache += (++mid);
                count -= mid;
            }
            else
            {
                count = mid;
            }
        }
    }
    return 0;

}

GCC_PURE static bool
is_bad(
    int userno)
{
    int count, datum, mid;
    const int *cache;

    if ((cache = pal_pool))
    {
        for (count = pal_count; count > 0;)
        {
            datum = cache[mid = count >> 1];
            if ((-userno) == datum)
                return true;
            if ((-userno) > datum)
            {
                cache += (++mid);
                count -= mid;
            }
            else
            {
                count = mid;
            }
        }
    }
    return false;
}

GCC_PURE static bool
can_message(
    const UTMP *up)
{
    int self, ufo, can;

    if (up->userno == (self = cuser.userno))
        return false;

    if (HAS_PERM(PERM_SYSOP | PERM_ACCOUNTS | PERM_CHATROOM))   /* 站長、帳號、聊天室 */
        return true;

    ufo = up->ufo;

    if (ufo & (UFO_MESSAGE))           /* 遠離塵囂 */
        return false;

    if (!(ufo & UFO_QUIET))
        return true;                     /* 呼叫器沒關掉，亦無損友 */

    can = 0;                    /* 找不到為 normal */
    can = can_see(up);
    return (ufo & UFO_QUIET)
        ? can == 1                      /* 只有好友可以 */
        : can < 2 /* 只要不是損友 */ ;
}


GCC_PURE static bool
can_override(
    const UTMP *up)
{
    int self, ufo, can;

    if (up->userno == (self = cuser.userno))
        return false;

    if (HAS_PERM(PERM_SYSOP | PERM_ACCOUNTS | PERM_CHATROOM))   /* 站長、帳號、聊天室 */
        return true;

    ufo = up->ufo;

    if (ufo & (UFO_PAGER1 | UFO_REJECT))                /* 遠離塵囂 */
        return false;

    if (!(ufo & UFO_PAGER))
        return true;                     /* 呼叫器沒關掉，亦無損友 */

    can = 0;                    /* 找不到為 normal */

    can = can_see(up);
    return (ufo & UFO_PAGER)
        ? can == 1                      /* 只有好友可以 */
        : can < 2 /* 只要不是損友 */ ;
}

/* ----------------------------------------------------- */
/* 好友名單：新增、刪除、修改、載入、同步                */
/* ----------------------------------------------------- */


bool
is_pal(
    int userno)
{
    int count, *cache, datum, mid;

    if ((cache = pal_pool))
    {
        for (count = pal_count; count > 0;)
        {
            datum = cache[mid = count >> 1];
            if (userno == datum)
                return true;
            if (userno > datum)
            {
                cache += (++mid);
                count -= mid;
            }
            else
            {
                count = mid;
            }
        }
    }
    return false;
}


void
pal_cache(void)
{
    int count, fsize, ufo;
    int *plist, *cache;
    PAL *phead, *ptail;
    char *fimage, fpath[64];
    UTMP *up;

    up = cutmp;
    cutmp->userno = cuser.userno;

    cache = NULL;
    count = 0;
    ufo = cuser.ufo & ~( UFO_BIFF | UFO_BIFFN | UFO_REJECT | UFO_FCACHE );

    fsize = 0;
    usr_fpath(fpath, cuser.userid, FN_PAL);
    fimage = f_img(fpath, &fsize);
    if (fimage != NULL)
    {
        if (fsize > (PAL_MAX * sizeof(PAL)))
        {
            fsize = PAL_MAX * sizeof(PAL);
        }

        count = fsize / sizeof(PAL);
        if (count)
        {
            cache = plist = up->pal_spool;
            phead = (PAL *) fimage;
            ptail = (PAL *) (fimage + fsize);
            ufo |= UFO_REJECT;
            do
            {
                if (phead->ftype & PAL_BAD)
                {
                    *plist++ = -(phead->userno);
                }
                else
                {
                    *plist++ = phead->userno;
                }
            } while (++phead < ptail);

            if (count > 0)
            {
                ufo |= UFO_FCACHE;
                if (count > 1)
                    xsort(cache, count, sizeof(int), int_cmp);
            }
            else
            {
                cache = NULL;
            }

        }
    }

    pal_pool = cache;
    up->pal_max = pal_count = count;

    free(fimage);
    cuser.ufo = ufo;
}


/*-------------------------------------------------------*/
/* 選單式聊天介面                                        */
/*-------------------------------------------------------*/


static pickup ulist_pool[MAXACTIVE];
static int friend_num, ofriend_num, pfriend_num, bfriend_num;
static int ulist_head(XO *xo);
static int ulist_init(XO *xo);


static const char *const msg_pickup_way[] =
{
    "任意",
    "代號",
    "故鄉",
    "動態",
    "暱稱",
    "閒置"
};

GCC_PURE static char
ck_state(
    int in1,
    int in2,
    const UTMP *up,
    int mode)
{
    if (up->ufo & in2)
        return '#';
    else if (up->ufo & in1)
    {
        if (mode)
            return can_override(up) ? 'o' : '*';
        else
            return can_message(up) ? 'o' : '*';
    }
    else
        return ' ';
}



static int
ulist_body(
    XO *xo)
{
    pickup *pp;
    UTMP *up;
    int cnt, max, ufo, self, userno, sysop GCC_UNUSED, diff, diffmsg, fcolor, colortmp;
    char buf[16], color[20], ship[80];
    static const char *const wcolor[7] = {"\x1b[m", COLOR_PAL, COLOR_BAD, COLOR_BOTH, COLOR_OPAL, COLOR_CLOAK, COLOR_BOARDPAL};
    time_t now;

//  pal = cuser.ufo;

    max = xo->max;
    if (max <= 0)
    {
        return 0;
    }

//  pal &= UFO_PAL;
    cnt = 0;
    pp = &ulist_pool[0];
    self = cuser.userno;
    sysop = HAS_PERM(PERM_SYSOP | PERM_ACCOUNTS);
    time(&now);

    while (cnt++ < max)
    {
        up = *pp++;
        if ((userno = up->userno) && !((up->ufo & UFO_CLOAK) && !HAS_PERM(PERM_SEECLOAK) && (up->userno != cuser.userno)) )
        {
            if ((diff = now - up->idle_time))
            {
                if (diff < 60 * 60)
                    sprintf(buf, "%2d'%02d", diff / 60, diff % 60);
                else if ((diff /= 60) < 24 * 60)
                    sprintf(buf, "%2dh%02d", diff / 60, diff % 60);
                else if ((diff /= 60) < 100 * 24)
                    sprintf(buf, "%2dd%02d", diff / 24, diff % 24);
                else if ((diff /= 24) < 10000)
                    sprintf(buf, "%4dd", diff);
                else if ((diff /= 10000) < 1000)
                    sprintf(buf, "%3dkd", diff);
                else  /* Not possible for 32-bit integer `time_t` */
                    sprintf(buf, "---Md");
            }
            else
                buf[0] = '\0';

            fcolor = (userno == self) ? 3 : is_pal(userno);

            colortmp = can_see(up);
            if (is_pal(userno) && colortmp == 1) fcolor = 3;
            else if (!is_pal(userno) && colortmp == 1) fcolor = 4;

            if (is_bad(userno)) fcolor = 2;

            ufo = up->ufo;


            diff = ck_state(UFO_PAGER, UFO_PAGER1, up, 1);
            diffmsg = ck_state(UFO_QUIET, UFO_MESSAGE, up, 0);

            colortmp = 1;
            if (ufo & UFO_CLOAK) fcolor = 5;
            else if (fcolor == 0)
                colortmp = 0;
            strcpy(color, wcolor[fcolor]);

            printf("%5d %s%-*s %-22.21s%s%-16.15s%c%c %-14.14s%s\n",
                cnt,
                color, IDLEN, up->userid,
                (HAS_PERM(PERM_SYSOP) && (cuser.ufo2 & UFO2_REALNAME))? up->realname : up->username,
                colortmp > 0 ? "\x1b[m" : "",
                (cuser.ufo2 & UFO2_SHIP) ? ship : ((up->ufo & UFO_HIDDEN)&&!HAS_PERM(PERM_SYSOP)) ?
                HIDDEN_SRC : up->from, diff, diffmsg,
                bmode(up, 0), buf);
        }
    }

    return 0;
}


static int
ulist_cmp_userid(
    const void *i, const void *j)
{
    return str_casecmp((*(const UTMP *const *)i) -> userid, (*(const UTMP *const *)j) -> userid);
}


static int
ulist_cmp_host(
    const void *i, const void *j)
{
    return str_casecmp((*(const UTMP *const *)i) -> from, (*(const UTMP *const *)j) -> from);
}

static int
ulist_cmp_idle(
    const void *i, const void *j)
{
    return (*(const UTMP *const *)i)->idle_time - (*(const UTMP *const *)j)->idle_time;
}

static int
ulist_cmp_mode(
    const void *i, const void *j)
{
    return (*(const UTMP *const *)i)->mode - (*(const UTMP *const *)j)->mode;
}

static int
ulist_cmp_nick(
    const void *i, const void *j)
{
    return str_casecmp((*(const UTMP *const *)i) -> username, (*(const UTMP *const *)j) -> username);
}

static int (*const ulist_cmp[]) (const void *i, const void *j) =
{
    ulist_cmp_userid,
    ulist_cmp_host,
    ulist_cmp_mode,
    ulist_cmp_nick,
    ulist_cmp_idle
};


static int
ulist_init(
    XO *xo)
{
    UTMP *up, *uceil;
    pickup *pp;
    int max, filter, seecloak, userno, self, i, nf_num, tmp;
    pickup pf[MAXACTIVE], of[MAXACTIVE], nf[MAXACTIVE];
    pp = ulist_pool;

    self = cuser.userno;
    filter = cuser.ufo2 & UFO2_PAL;

    seecloak = HAS_PERM(PERM_SEECLOAK);

    up = ushm->uslot;
    uceil = up + ushm->ubackidx;

    max = 0;
    friend_num = ofriend_num = pfriend_num = nf_num = bfriend_num = 0;
    do                                  /* 先找好友 */
    {
        userno = up->userno;
        if (userno <= 0 || (up->pid <= 0 && !HAS_PERM(PERM_SYSOP|PERM_SEECLOAK)))
            continue;
        if (!seecloak && (up->ufo & UFO_CLOAK))
            continue;
        tmp = can_see(up);
        if (is_bad(userno)) bfriend_num++;
        if (((seecloak || !(up->ufo & UFO_CLOAK)) && (tmp != 2)) || HAS_PERM(PERM_SYSOP|PERM_SEECLOAK) || up->userno == cuser.userno)
        {
            if ((is_pal(userno) && (tmp == 1)) || (userno == self))
            {
                *pp++ = up;
                friend_num++;
            }
            else if (is_pal(userno) && !(tmp == 1) && !filter)
                pf[pfriend_num++] = up;
            else if (!is_pal(userno) && (tmp == 1) && !filter)
                of[ofriend_num++] = up;
            else if (!filter)
                nf[nf_num++] = up;
        }
    } while (++up <= uceil);
    for (i=0; i<pfriend_num; i++)
        *pp++ = pf[i];
    for (i=0; i<ofriend_num; i++)
        *pp++ = of[i];
    for (i=0; i<nf_num; i++)
        *pp++ = nf[i];

    xo->max = max = pp - ulist_pool;

    for (int i = 0; i < COUNTOF(xo->pos); ++i)
    {
        if (xo->pos[i] >= max)
            xo->pos[i] = xo->top = 0;
    }
    xo->cur_idx = 0;

    if ((max > 1) && (pickup_way))
    {
        if (pickup_way==1)
        {
            xsort(ulist_pool, friend_num, sizeof(pickup), ulist_cmp[pickup_way - 1]);
            xsort((ulist_pool+friend_num), pfriend_num, sizeof(pickup), ulist_cmp[pickup_way - 1]);
            xsort((ulist_pool+friend_num+pfriend_num), ofriend_num, sizeof(pickup), ulist_cmp[pickup_way - 1]);
            xsort((ulist_pool+friend_num+pfriend_num+ofriend_num), max - friend_num - pfriend_num - ofriend_num, sizeof(pickup), ulist_cmp[pickup_way - 1]);
        }
        else
            xsort(ulist_pool, max, sizeof(pickup), ulist_cmp[pickup_way - 1]);
    }
    total_num = max;
    return ulist_head(xo);
}


static int
ulist_neck(
    XO *xo)
{
    printf("  排列方式：[\x1b[1m%s\x1b[m] 上站人數：%d %s我的朋友：%d %s與我為友：%d %s壞人：%d\x1b[m",
        msg_pickup_way[pickup_way], total_num, COLOR_PAL, friend_num+pfriend_num, COLOR_OPAL, friend_num+ofriend_num, COLOR_BAD, bfriend_num);
    printf(NECK_ULIST "\n",
        22, "暱  稱", 13, "故鄉", "動態");
    return ulist_body(xo);
}


static int
ulist_head(
    XO *xo)
{
    return ulist_neck(xo);
}

static void
reset_utmp(void)
{
    cutmp->userno = cuser.userno;
    cutmp->ufo = cuser.ufo;
    strcpy(cutmp->userid, cuser.userid);
    strcpy(cutmp->username, cuser.username);
    strcpy(cutmp->realname, cuser.realname);
}


int
main(
    int argc,
    char *argv[])
{
    XO xo;
    char fpath[128];
    int fd;
    const char *userid = NULL;

    setgid(BBSGID);
    setuid(BBSUID);
    chdir(BBSHOME);

    pickup_way = -1;
    while (optind < argc)
    {
        switch (getopt(argc, argv, "+" "u:p:"))
        {
        case -1:  // Position arguments
            if (!(optarg = argv[optind++]))
                break;
            if (!userid)
            {
        case 'u':
                userid = optarg;
                break;
            }
            else if (!(pickup_way >= 0))
            {
        case 'p':
                if ((pickup_way = atoi(optarg)) >= 0 && pickup_way < PICKUP_WAYS)
                    break;
            }
            else
                break;
            // Falls through
            // to handle invalid argument values
        default:
            userid = NULL;  // Invalidate arguments
            optind = argc;  // Ignore remaining arguments
            break;
        }
    }

    if (!(userid && pickup_way >= 0))
    {
        fprintf(stderr, "Usage: %s [-u] <userid> [-p] <pickup_way>\n", argv[0]);
        fprintf(stderr, "pickup_ways:\n");
        for (pickup_way = 0; pickup_way < PICKUP_WAYS; pickup_way++)
            fprintf(stderr, "\t%d: Sorted by %s\n", pickup_way, msg_pickup_way[pickup_way]);
        return 2;
    }

    ushm_attach(&ushm);
    cutmp = &utmp;
    usr_fpath(fpath, userid, FN_ACCT);
    fd = open(fpath, O_RDONLY);
    if (fd>=0)
    {
        if (read(fd, &cuser, sizeof(ACCT)) == sizeof(ACCT))
        {
            reset_utmp();
            pal_cache();
            ulist_init(&xo);
        }
        close(fd);
    }
    return 0;
}
