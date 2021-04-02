/*-------------------------------------------------------*/
/* talk.c       ( NTHU CS MapleBBS Ver 3.00 )            */
/*-------------------------------------------------------*/
/* target : talk/query/friend(pal) routines              */
/* create : 95/03/29                                     */
/* update : 2000/01/02                                   */
/*-------------------------------------------------------*/

#define MODES_C

#include "bbs.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#undef  APRIL_FIRST

typedef struct
{
    int pal_no;
    char ship[46];
}  PAL_SHIP;

static int pal_count;
static int *pal_pool;
static int bmw_modetype;
#ifdef HAVE_BOARD_PAL
static int board_pals;
#endif
static PAL_SHIP *pal_ship;
GCC_PURE static int can_see(const UTMP *up);
static bool can_banmsg(const UTMP *up);


typedef struct
{
    int curcol, curln;
    int sline, eline;
}      talk_win;


typedef struct
{
    UTMP *utmp;
    int type;
}       PICKUP;


static void
reset_utmp(void)
{
    cutmp->pid = currpid;
    cutmp->userno = cuser.userno;
    cutmp->mode = bbsmode;
    cutmp->ufo = cuser.ufo;
    strcpy(cutmp->userid, cuser.userid);
    strcpy(cutmp->username, cuser.username);
    strcpy(cutmp->realname, cuser.realname);
    str_scpy(cutmp->from, fromhost, sizeof(cutmp->from));
}


/* ----------------------------------------------------- */
/* �O�� pal �� user number                               */
/* ----------------------------------------------------- */

#define PICKUP_WAYS     COUNTOF(msg_pickup_way)
int pickup_way=1;

static char page_requestor[40];

const char *
bmode(
    const UTMP *up,
    int simple)
{
    static char modestr[32];
#ifdef  HAVE_SHOWNUMMSG
    static const char *const nums[9] = {"�@", "�G", "�T", "�|", "��", "��", "�C", "�K", "�E"};
#endif
    int mode;
    const char *word;

    if (!up)
        return "���b���W";

#ifdef  HAVE_SHOWNUMMSG
    if (up->num_msg > 9)
    {
        strcpy(modestr, "�Q���z�F");
        return (modestr);
    }
    else if (up->num_msg > 0)
    {
        sprintf(modestr, "����%s�ʰT��", nums[up->num_msg-1]);
        return (modestr);
    }
#endif

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
        return (word); /* Thor.980805: ����: �������H���|�Q���D���talk */

    sprintf(modestr, "%s %s", word, up->mateid);
    return (modestr);
}


/* ----------------------------------------------------- */
/* �n�ͦW��G�d�ߪ��A�B�ͽ˴y�z                          */
/* ----------------------------------------------------- */


/* Thor: �Ǧ^�� -> 0 for good, 1 for normal, 2 for bad */

static void
copyship(
    char *ship,
    int userno)
{
    PAL_SHIP *shead;
    shead = pal_ship;
    if (userno == cuser.userno)
    {
        strcpy(ship, "���N�O��");
                return;
    }
    if (shead)
    {
        while (shead->pal_no)
        {
            if (shead->pal_no == userno)
            {
                strcpy(ship, shead->ship);
                return;
            }
            shead++;
        }
    }
    return;

}

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

#ifdef  HAVE_BANMSG
GCC_PURE static bool
can_banmsg(
    const UTMP *up)
{
    int count, datum, mid;
    const int *cache;

    if ((cache = up->banmsg_spool))
    {
        for (count = up->banmsg_max; count > 0;)
        {
            datum = cache[mid = count >> 1];
            if (cuser.userno == datum)
                return true;
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
    return false;

}
#endif


GCC_PURE bool
can_message(
    const UTMP *up)
{
    int self, ufo, can;

    if (up->userno == (self = cuser.userno))
        return false;

    ufo = up->ufo;

//  if (ufo & UFO_REJECT)
//      return false;

    if (HAS_PERM(PERM_SYSOP | PERM_ACCOUNTS|PERM_CHATROOM))     /* �����B�b�� */
        return true;

#ifdef  HAVE_BANMSG
    if (can_banmsg(up))         /* �ڦ��T�� */
        return false;
#endif

    if (ufo & (UFO_MESSAGE))   /* �������� */
        return false;

    if (!(ufo & UFO_QUIET))
        return true;                     /* �I�s���S�����A��L�l�� */

    can = 0;                    /* �䤣�쬰 normal */
    can = can_see(up);
    return (ufo & UFO_QUIET)
        ? can == 1                      /* �u���n�ͥi�H */
        : can < 2                       /* �u�n���O�l�� */ ;
}


GCC_PURE static bool
can_override(
    const UTMP *up)
{
    int self, ufo, can;

    if (up->userno == (self = cuser.userno))
        return false;

    ufo = up->ufo;

    if (ufo & (UFO_REJECT|UFO_NET))
        return false;

    if (HAS_PERM(PERM_SYSOP | PERM_ACCOUNTS|PERM_CHATROOM))     /* �����B�b�� */
        return true;

    if (ufo & UFO_PAGER1)               /* �������� */
        return false;

    if (!(ufo & UFO_PAGER))
        return true;                     /* �I�s���S�����A��L�l�� */

    can = 0;                    /* �䤣�쬰 normal */

    can = can_see(up);
    return (ufo & UFO_PAGER)
        ? can == 1                      /* �u���n�ͥi�H */
        : can < 2 /* �u�n���O�l�� */ ;
}

/* ----------------------------------------------------- */
/* �n�ͦW��G�s�W�B�R���B�ק�B���J�B�P�B                */
/* ----------------------------------------------------- */

#ifdef  HAVE_BOARD_PAL
GCC_PURE bool
is_boardpal(
    const UTMP *up)
{
    return cutmp->board_pal == up->board_pal;
}
#endif

GCC_PURE bool
is_pal(
    int userno)
{
    int count, datum, mid;
    const int *cache;

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

#ifdef  HAVE_BANMSG
GCC_PURE bool
is_banmsg(
    int userno)
{
    int count, datum, mid;
    const int *cache;

    if ((cache = cutmp->banmsg_spool))
    {
        for (count = cutmp->banmsg_max; count > 0;)
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
#endif


void
pal_cache(void)
{
    int count, fsize, ufo, fd;
    int *plist, *cache;
    int ship_total;
    PAL *phead, *ptail;
    PAL_SHIP *shead;
    char *fimage, fpath[64];
    UTMP *up;

    up = cutmp;
    cutmp->userno = cuser.userno;

    cache = NULL;
    ship_total = count = 0;
    ufo = cuser.ufo & ~( UFO_BIFF | UFO_BIFFN | UFO_REJECT | UFO_FCACHE );
    /* Thor.980805: �� BIFF�n���S���Ӥj���Y */

    fsize = 0;
    usr_fpath(fpath, cuser.userid, FN_PAL);
    fimage = f_img(fpath, &fsize);
    if ((fsize > (PAL_MAX * sizeof(PAL))) && (fd = open(fpath, O_RDWR)) >= 0)
    {
        ftruncate(fd, PAL_MAX * sizeof(PAL));
        close(fd);
    }
    if (fimage != NULL)
    {
        if (fsize > (PAL_MAX * sizeof(PAL)))
        {
            sprintf(fpath, "%-*s %d > %d * %zu\n", IDLEN, cuser.userid, fsize, PAL_MAX, sizeof(PAL));
            f_cat(FN_PAL_LOG, fpath);
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
                if (strlen(phead->ship) > 0)
                    ship_total++;
            } while (++phead < ptail);

            if (count > 0)
            {
                ufo |= UFO_FCACHE;
                if (count > 1)
                    xsort(cache, count, sizeof(int), (int (*)(const void *lhs, const void *rhs))int_cmp);
            }
            else
            {
                cache = NULL;
            }

        }
    }

    pal_pool = cache;
    up->pal_max = pal_count = count;
    if (cutmp)
    {
        ufo = (ufo & ~(UFO_UTMP_MASK | UFO_REJECT)) | (cutmp->ufo & UFO_UTMP_MASK);
        /* Thor.980805: �ѨM cutmp.ufo�M cuser.ufo���P�B���D */
        cutmp->ufo = ufo & (~UFO_REJECT);
    }
    free(pal_ship);
    pal_ship = NULL;


    if (ship_total)
    {
        shead = pal_ship = (PAL_SHIP *)malloc((ship_total+1)*sizeof(PAL_SHIP));
        phead = (PAL *) fimage;
        ptail = (PAL *) (fimage + fsize);
        if (pal_ship)
        {
            memset(pal_ship, 0, (ship_total+1)*sizeof(PAL_SHIP));
            do
            {
                if (strlen(phead->ship)>0)
                {
                    strcpy(shead->ship, phead->ship);
                    shead->pal_no = phead->userno;
                    shead++;
                }
            } while (++phead < ptail);
        }
        else
            pal_ship = NULL;
    }
    else
        pal_ship = NULL;

    free(fimage);
    cuser.ufo = ufo;
}

/* 2003.01.01 verit : ���� �W���q���W�� */
void
aloha_sync(void)
{
    char fpath[128];
    int fd, size=0;
    struct stat st;

    usr_fpath(fpath, cuser.userid, FN_FRIEND_BENZ);
    if ((fd = open(fpath, O_RDWR, 0600)) < 0)
            return;

    outz("�� ��ƾ�z�]�֤��A�еy�� \x1b[5m...\x1b[m");
    refresh();

    if (!fstat(fd, &st) && (size = st.st_size) > 0)
    {
        BMW *pbase, *phead, *ptail;
        int userno;

        pbase = phead = (BMW *) malloc(size);
        size = read(fd, pbase, size);
        if (size >= sizeof(BMW))
        {
            ptail = (BMW *) ((char *) pbase + size);
            while (phead < ptail)
            {
                userno = phead->recver;
                if (userno > 0 && userno == acct_userno(phead->userid))
                {
                    phead++;
                    continue;
                }
                ptail--;
                if (phead >= ptail) break;
                memcpy(phead, ptail, sizeof(BMW));
            }

            size = (char *) ptail - (char *) pbase;
            if (size > 0)
            {
                //xsort(pbase, size / sizeof(BMW), sizeof(BMW), str_casecmp);
                lseek(fd, 0, SEEK_SET);
                write(fd, pbase, size);
                ftruncate(fd, size);
            }
        }
        free(pbase);
    }
    close(fd);

    if (size <= 0)
        unlink(fpath);
}


void
pal_sync(
    const char *fpath)
{
    int fd, size=0;
    struct stat st;
    char buf[64];

    if (!fpath)
    {
        usr_fpath(buf, cuser.userid, FN_PAL);
        fpath = buf;
    }

    if ((fd = open(fpath, O_RDWR, 0600)) < 0)
        return;

    outz("�� ��ƾ�z�]�֤��A�еy�� \x1b[5m...\x1b[m");
    refresh();

    if (!fstat(fd, &st) && (size = st.st_size) > 0)
    {
        PAL *pbase, *phead, *ptail;
        int userno;

        pbase = phead = (PAL *) malloc(size);
        size = read(fd, pbase, size);
        if (size >= sizeof(PAL))
        {
            ptail = (PAL *) ((char *) pbase + size);
            while (phead < ptail)
            {
                userno = phead->userno;
                if (userno > 0 && userno == acct_userno(phead->userid))
                {
                    phead++;
                    continue;
                }

                ptail--;
                if (phead >= ptail)
                    break;
                memcpy(phead, ptail, sizeof(PAL));
            }

            size = (char *) ptail - (char *) pbase;
            if (size > 0)
            {
                if (size > sizeof(PAL))
                {
                    if (size > PAL_ALMR * sizeof(PAL))
                        vmsg("�z���n�ͦW��Ӧh�A�е��[��z");
                    xsort(pbase, size / sizeof(PAL), sizeof(PAL), (int (*)(const void *lhs, const void *rhs))str_casecmp);
                }

                /* Thor.0709: �O�_�n�[�W�������Ъ��n�ͪ��ʧ@? */

                lseek(fd, 0, SEEK_SET);
                write(fd, pbase, size);
                ftruncate(fd, size);
            }
        }
        free(pbase);
    }
    close(fd);

    if (size <= 0)
        unlink(fpath);
}


/* ----------------------------------------------------- */
/* �n�ͦW��G��榡�ާ@�ɭ��y�z                          */
/* ----------------------------------------------------- */


static int pal_add(XO *xo);


static void
pal_item(
    int num,
    const PAL *pal)
{
    prints("%6d %-3s%-14s%s\n", num, pal->ftype & PAL_BAD ? "��" : "",
        pal->userid, pal->ship);
}

static int
pal_cur(
    XO *xo)
{
    const PAL *const pal = (const PAL *) xo_pool_base + xo->pos;
    move(3 + xo->pos - xo->top, 0);
    pal_item(xo->pos + 1, pal);
    return XO_NONE;
}


static int
pal_body(
    XO *xo)
{
    const PAL *pal;
    int num, max, tail;

    max = xo->max;
    if (max <= 0)
    {
        if (vans("�n��s�B�Ͷ�(y/N)�H[N] ") == 'y')
            return pal_add(xo);
        return XO_QUIT;
    }

    num = xo->top;
    pal = (const PAL *) xo_pool_base + num;
    tail = num + XO_TALL;
    max = BMIN(max, tail);

    move(3, 0);
    do
    {
        pal_item(++num, pal++);
    } while (num < max);
    clrtobot();

    return XO_NONE;
}


static int
pal_head(
    XO *xo)
{
    vs_head("�n�ͦW��", str_site);
    prints(NECK_PAL, d_cols, "");
    return pal_body(xo);
}


static int
pal_load(
    XO *xo)
{
    xo_load(xo, sizeof(PAL));
    return pal_body(xo);
}


static int
pal_init(
    XO *xo)
{
    xo_load(xo, sizeof(PAL));
    return pal_head(xo);
}


static void
pal_edit(
    PAL *pal,
    int echo)
{
    if (echo == DOECHO)
        memset(pal, 0, sizeof(PAL));
    vget(B_LINES_REF, 0, "�ͽˡG", pal->ship, sizeof(pal->ship), echo);
    pal->ftype = vans("�l��(y/N)�H[N] ") == 'y' ? PAL_BAD : 0;
}


static int
pal_search(
    XO *xo,
    int step)
{
    int num, pos, max;
    static char buf[IDLEN + 1];
    int fsize;
    PAL *phead;
    char *fimage = NULL, fpath[64];

    if (vget(B_LINES_REF, 0, msg_uid, buf, IDLEN + 1, GCARRY))
    {
        GCC_UNUSED int buflen;
        char bufl[IDLEN + 1];

        usr_fpath(fpath, cuser.userid, FN_PAL);
        fimage = f_img(fpath, &fsize);
        phead = (PAL *) fimage;

        str_lower(bufl, buf);
        buflen = strlen(bufl);

        pos = num = xo->pos;
        max = xo->max;
        do
        {
            pos += step;
            if (pos < 0)
                pos = max - 1;
            else if (pos >= max)
                        pos = 0;

            if (str_casestr((phead + pos)->userid, bufl) || str_casestr((phead + pos)->ship, bufl))
            {
                move(b_lines, 0);
                clrtoeol();
                free(fimage);
                return XR_FOOT + XO_MOVE + pos;
            }

        } while (pos != num);
    }
    free(fimage);

    return XO_FOOT;
}

static int
pal_search_forward(
    XO *xo)
{
    return pal_search(xo, 1); /* step = +1 */
}

static int
pal_search_backward(
    XO *xo)
{
    return pal_search(xo, -1); /* step = -1 */
}


static int
pal_add(
    XO *xo)
{
    ACCT acct;
    int userno;

    if (xo->max >= PAL_MAX)
    {
        vmsg("�z���n�ͦW��Ӧh�A�е��[��z");
        return XO_FOOT;
    }

    userno = acct_get(msg_uid, &acct);

    /* jhlin : for board_moderator, temporarily */

#if 1                           /* Thor.0709: �����Х[�J */
    /* if (is_pal(userno)) */
    if ((xo->dir[0] == 'u') && (is_pal(userno) || is_bad(userno)))
                /* lkchu.981201: �u���b moderator board �~�i���Х[�J */
    {
        vmsg("�W�椤�w�����n��");
        return XO_FOOT;
    }
    else if (userno == cuser.userno)      /* lkchu.981201: �n�ͦW�椣�i�[�ۤv */
    {
        vmsg("�ۤv�����[�J�n�ͦW�椤");
        return XO_FOOT;
    }
#endif

    if ((userno > 0) /* && !pal_state(cutmp, userno, NULL) */)
    {
        PAL pal;

        pal_edit(&pal, DOECHO);
        strcpy(pal.userid, acct.userid);
        pal.userno = userno;
        rec_add(xo->dir, &pal, sizeof(PAL));
        xo->pos = XO_TAIL /* xo->max */ ;
        xo_load(xo, sizeof(PAL));
    }

#if 1                           /* Thor.0709: �n�ͦW��P�B */
    pal_cache();
#endif

    return XO_HEAD;
}





static int
pal_delete(
    XO *xo)
{
    if (vans(msg_del_ny) == 'y')
    {
        if (!rec_del(xo->dir, sizeof(PAL), xo->pos, NULL, NULL))
        {

#if 1                           /* Thor.0709: �n�ͦW��P�B */
            pal_cache();
#endif

            return XO_LOAD;
        }
    }
    return XO_FOOT;
}


static int
pal_change(
    XO *xo)
{
    PAL *pal, mate;
    int pos;

    pos = xo->pos;
    pal = (PAL *) xo_pool_base + pos;

    mate = *pal;
    pal_edit(pal, GCARRY);
    if (memcmp(pal, &mate, sizeof(PAL)))
    {
        rec_put(xo->dir, pal, sizeof(PAL), pos);
        return XR_FOOT + XO_CUR;
    }

    return XO_FOOT;
}


static int
pal_mail(
    XO *xo)
{
    PAL *pal;
    char *userid;

    pal = (PAL *) xo_pool_base + xo->pos;
    userid = pal->userid;
    if (*userid)
    {
        vs_bar("�H  �H");
        prints("���H�H�G%s", userid);
        my_send(userid);
    }
    return XO_HEAD;
}


static int
pal_sort(
    XO *xo)
{
    pal_sync(xo->dir);
    return XO_LOAD;
}


static int
pal_query(
    XO *xo)
{
    const PAL *pal;

    pal = (const PAL *) xo_pool_base + xo->pos;
    move(1, 0);
    clrtobot();
    /* move(2, 0); *//* Thor.0810: �i�H���[��? */
    /* if (*pal->userid) *//* Thor.0806:�����n���@�U, ���ӨS�t */
    my_query(pal->userid, 1);
    return XO_HEAD;
}


static int
pal_help(
    XO *xo)
{
    film_out(FILM_PAL, -1);
    return XO_HEAD;
}


KeyFuncList pal_cb =
{
    {XO_INIT, {pal_init}},
    {XO_LOAD, {pal_load}},
    {XO_HEAD, {pal_head}},
    {XO_BODY, {pal_body}},
    {XO_CUR, {pal_cur}},

    {'a', {pal_add}},
    {'c', {pal_change}},
    {'d', {pal_delete}},
    {'m', {pal_mail}},
    {'q', {pal_query}},
    {'s', {pal_sort}},
    {'/', {pal_search_forward}},
    {'?', {pal_search_backward}},
    {'h', {pal_help}}
};


int
t_pal(void)
{
    XO *xo, *last;
    char fpath[64];

    last = xz[XZ_PAL - XO_ZONE].xo;  /* record */

    usr_fpath(fpath, cuser.userid, FN_PAL);
    xz[XZ_PAL - XO_ZONE].xo = xo = xo_new(fpath);
    xo->cb = pal_cb;
    xo->recsiz = sizeof(PAL);
    xo->pos = 0;
    xover(XZ_PAL);
    pal_cache();
    free(xo);

    xz[XZ_PAL - XO_ZONE].xo = last;  /* restore */

    return 0;
}


/* ----------------------------------------------------- */
/* �T���C��: ��榡�ާ@�ɭ��y�z by lkchu                 */
/* ----------------------------------------------------- */


/*static */void bmw_edit(UTMP *up, const char *hint, BMW *bmw, int cc);


static void
bmw_item(
    int num,
    const BMW *bmw)
{
    struct tm *ptime = localtime_any(&bmw->btime);

    if (!(bmw_modetype & BMW_MODE))
    {
        if (bmw->sender == cuser.userno)
        {
            /* lkchu.990206: �n�ͼs�� */
            const char *userid = bmw->userid;
            if (!*userid)
                userid = "���a�n��";

            prints("%6d %02d:%02d %-*s ��%-*.*s\n", num, ptime->tm_hour, ptime->tm_min,
                IDLEN, userid, d_cols + 50, d_cols + 50, bmw->msg);
        }
        else
        {
            if (strstr(bmw->msg, "���s��"))
                prints("%6d \x1b[36;1m%02d:%02d %-*s ��%-*.*s\x1b[m\n", num, ptime->tm_hour, ptime->tm_min,
                    IDLEN, bmw->userid, d_cols + 50, d_cols + 50, (bmw->msg)+8);
            else
                prints("%6d \x1b[32m%02d:%02d %-*s ��%-*.*s\x1b[m\n", num, ptime->tm_hour, ptime->tm_min,
                    IDLEN, bmw->userid, d_cols + 50, d_cols + 50, bmw->msg);
        }
    }
    else
    {
        if (bmw->sender == cuser.userno)
        {
            /* lkchu.990206: �n�ͼs�� */
            const char *userid = bmw->userid;
            if (!*userid)
                userid = "���a�n��";

            prints("%6d %-*s ��%-*.*s\n", num, IDLEN, userid, d_cols + 57, d_cols + 57, bmw->msg);
        }
        else
        {
            if (strstr(bmw->msg, "���s��"))
                prints("%6d \x1b[36;1m%-*s ��%-*.*s\x1b[m\n", num,
                    IDLEN, bmw->userid, d_cols + 57, d_cols + 57, (bmw->msg)+8);
            else
                prints("%6d \x1b[32m%-*s ��%-*.*s\x1b[m\n", num,
                    IDLEN, bmw->userid, d_cols + 57, d_cols + 57, bmw->msg);
        }
    }
}

static int
bmw_cur(
    XO *xo)
{
    const BMW *const bmw = (const BMW *) xo_pool_base + xo->pos;
    move(3 + xo->pos - xo->top, 0);
    bmw_item(xo->pos + 1, bmw);
    return XO_NONE;
}


static int
bmw_body(
    XO *xo)
{
    const BMW *bmw;
    int num, max, tail;

    move(3, 0);
    clrtobot();
    max = xo->max;
    if (max <= 0)
    {
        vmsg("���e�õL���T�I�s");
        return XO_QUIT;
    }

    num = xo->top;
    bmw = (const BMW *) xo_pool_base + num;
    tail = num + XO_TALL;
    max = BMIN(max, tail);

    do
    {
        bmw_item(++num, bmw++);
    } while (num < max);

    return XO_NONE;
}


static int
bmw_head(
    XO *xo)
{
    vs_head("�d�ݰT��", str_site);
    if (bmw_modetype & BMW_MODE)
    {
        prints(NECK_BMW, d_cols, "");
    }
    else
    {
        prints(NECK_BMWTIME, d_cols, "");
    }
    return bmw_body(xo);
}


static int
bmw_load(
    XO *xo)
{
    xo_load(xo, sizeof(BMW));
    return bmw_body(xo);
}


static int
bmw_init(
    XO *xo)
{
    xo_load(xo, sizeof(BMW));
    return bmw_head(xo);
}


static int
bmw_delete(
    XO *xo)
{
    if (vans(msg_del_ny) == 'y')
        if (!rec_del(xo->dir, sizeof(BMW), xo->pos, NULL, NULL))
            return XO_LOAD;

    return XO_FOOT;
}


static int
bmw_mail(
    XO *xo)
{
    BMW *bmw;
    char *userid;

    bmw = (BMW *) xo_pool_base + xo->pos;
    userid = bmw->userid;
    if (*userid)
    {
        vs_bar("�H  �H");
        prints("���H�H�G%s", userid);
        my_send(userid);
    }
    return XO_HEAD;
}


static int
bmw_query(
    XO *xo)
{
    const BMW *bmw;

    bmw = (const BMW *) xo_pool_base + xo->pos;
    move(1, 0);
    clrtobot();
    /* move(2, 0); *//* Thor.0810: �i�H���[��? */
    /* if (*pal->userid) *//* Thor.0806:�����n���@�U, ���ӨS�t */
    my_query(bmw->userid, 1);
    return XO_HEAD;
}


static int
bmw_write(
    XO *xo)
{
    if (HAS_PERM(PERM_PAGE))
    {
        UTMP *up = NULL;
        const BMW *benz;

        benz = (const BMW *) xo_pool_base + xo->pos;
        if ((benz->caller >= ushm->uslot && benz->caller < ushm->uslot + MAXACTIVE) && (benz->caller->userno == benz->sender) && can_message(benz->caller))
        {
            up = benz->caller;
        }

        if ((up || (up = utmp_find(benz->sender))) && can_message(up))
            /* lkchu.990104: ���H�e�� bmw �]�i�H�^ */
        {
            if (((up->ufo & UFO_CLOAK) && !HAS_PERM(PERM_SEECLOAK))||(can_see(up)==2 && !HAS_PERM(PERM_SYSOP))||benz->sender==0)
            {
                return XO_NONE;         /* lkchu.990111: �����o */
            }
            else
            {
                BMW bmw;
                char buf[20];
#ifdef  HAVE_SHOWNUMMSG
                if (up->num_msg > 9 && (up->ufo & UFO_MAXMSG) && !HAS_PERM(PERM_SYSOP))
                {
                    vmsg("���w�g�Q���y���z�F!!");
                    return XO_INIT;
                }
#endif
                sprintf(buf, "��[%s]", up->userid);
                bmw_edit(up, buf, &bmw, 0);
            }
        }
#ifdef  HAVE_BANMSG
        else if (up && !(up->ufo & UFO_MESSAGE) && can_banmsg(up))
        {
            vmsg("��褣�Qť��z���n��!!");
            return XO_INIT;
        }
#endif
    }
    return XO_NONE;
}

static int
bmw_mode(
    XO *xo)
{
    bmw_modetype ^= BMW_MODE;
    return XO_INIT;
}


static int
bmw_help(
    XO *xo)
{
    film_out(FILM_BMW, -1);
    return XO_HEAD;
}


KeyFuncList bmw_cb =
{
    {XO_INIT, {bmw_init}},
    {XO_LOAD, {bmw_load}},
    {XO_HEAD, {bmw_head}},
    {XO_BODY, {bmw_body}},
    {XO_CUR, {bmw_cur}},

    {'d', {bmw_delete}},
    {'m', {bmw_mail}},
    {'w', {bmw_write}},
    {'q', {bmw_query}},
    {Ctrl('Q'), {bmw_query}},
    {'s', {xo_cb_init}},
    {KEY_TAB, {bmw_mode}},
    {'h', {bmw_help}}
};


int
t_bmw(void)
{
    xover(XZ_BMW);
    return 0;
}


#ifdef HAVE_MODERATED_BOARD
/* ----------------------------------------------------- */
/* �O�ͦW��Gmoderated board                             */
/* ----------------------------------------------------- */


#define FN_FIMAGE       "fimage"


static void
bm_image(void)
{
    int fd;
    char fpath[80];

    brd_fpath(fpath, currboard, FN_PAL);
    if ((fd = open(fpath, O_RDONLY)) >= 0)
    {
        struct stat st;
        PAL *pal, *up;
        int count;

        fstat(fd, &st);
        if ((pal = (PAL *) malloc(count = st.st_size)))
        {
            count = read(fd, pal, count) / sizeof(PAL);
            if (count > 0)
            {
                int *userno, *ubase;
                int c = count;

                ubase = userno = (int *)malloc(count*sizeof(int));

                up = pal;
                do
                {
#ifdef HAVE_WATER_LIST
                    *userno++ = (up->ftype == PAL_BAD) ? -(up->userno) : up->userno;
#else
                    *userno++ = up->userno;
#endif
                    up++;
                } while (--c);

                if (count > 1)          /* Thor: �h�ƧǦ��q���鰷�d... */
                    xsort(ubase, count, sizeof(int), int_cmp);

                brd_fpath(fpath, currboard, FN_FIMAGE);
                if ((count = open(fpath, O_WRONLY | O_CREAT | O_TRUNC, 0600)) >= 0)
                {
                    write(count, ubase, (char *) userno - (char *) ubase);
                    close(count);
                }
                free(ubase);
            }
            else  /* Thor.980811: lkchu patch: �P friend �P�B */
            {
                brd_fpath(fpath, currboard, FN_FIMAGE);
                unlink(fpath);
            }
            free(pal);
        }
        close(fd);
    }
}


int
bm_belong(
    const char *board)
{
    int fsize, count, result, wcount;
    char fpath[80], *board_img;

    result = 0;

    brd_fpath(fpath, board, FN_FIMAGE); /* Thor: fimage�n�� sort�L! */
    board_img = f_img(fpath, &fsize);

    if (board_img != NULL)
    {
        wcount = count = fsize / sizeof(int);

        if (count > 0)
        {
            int userno, *up;
            int datum, mid;

            userno = cuser.userno;
            up = (int *) board_img;

            while (count > 0)
            {
                datum = up[mid = count >> 1];
                if (userno == datum)
                {
                    result = BRD_R_BIT | BRD_W_BIT;
                    break;
                }
                if (userno > datum)
                {
                    up += (++mid);
                    count -= mid;
                }
                else
                {
                    count = mid;
                }
            }
#ifdef HAVE_WATER_LIST
            up = (int *) board_img;
            while (wcount > 0)
            {
                datum = up[mid = wcount >> 1];
                if (-userno == datum)
                {
                    result = BRD_R_BIT;
                    break;
                }
                if (-userno > datum)
                {
                    up += (++mid);
                    wcount -= mid;
                }
                else
                {
                    wcount = mid;
                }
            }
#endif
        }
        free(board_img);
    }
    return result;
}


int
XoBM(
    XO *xo)
{
    if ((bbstate & STAT_BOARD) /*&& (bbstate & STAT_MODERATED)*/)
        /* && (cbhdr.readlevel == PERM_SYSOP)) */
        /*
         * Thor.1020: bbstate�� STAT_MODERATED�N�N��ŦXMODERATED BOARD,
         * ���ݦAcheck readlevel PERM_SYSOP
         */
    {
        XO *xt, *last;
        char fpath[80];

        last = xz[XZ_PAL - XO_ZONE].xo;  /* record */

        brd_fpath(fpath, currboard, FN_PAL);
        xz[XZ_PAL - XO_ZONE].xo = xt = xo_new(fpath);
        xt->cb = pal_cb;
        xt->recsiz = sizeof(PAL);
        xt->pos = 0;
        xover(XZ_PAL);          /* Thor: �ixover�e, pal_xo �@�w�n ready */

        /* build userno image to speed up, maybe upgrade to shm */

        bm_image();

        free(xt);

        xz[XZ_PAL - XO_ZONE].xo = last;  /* restore */

        return XO_INIT /* or post_init(xo) */ ;
    }
    return XO_NONE;
}
#endif  /* #ifdef HAVE_MODERATED_BOARD */


/* ------------------------------------- */
/* �u��ʧ@                              */
/* ------------------------------------- */


static void
showplans(
    const char *userid)
{
    int i;
    FILE *fp;
    char buf[256];

    usr_fpath(buf, userid, FN_PLANS);

    if ((fp = fopen(buf, "r")))
    {
        move(7, 0);
//      outs(" [�W��]�G\n\n");
        i = MAXQUERYLINES;

        while (i-- && fgets(buf, sizeof(buf), fp))
        {
            outx(buf);
        }
        fclose(fp);
    }
    else
    {
//      outs(" [�S���W��]\n\n");
    }
}


static void
do_query(
    const ACCT *acct,
    int paling)                 /* �O�_���b�]�w�n�ͦW�� */
{
    UTMP *up;
    int userno, mail, rich;
    const char *userid;

    utmp_mode(M_QUERY);
    userno = acct->userno;
    userid = acct->userid;
    strcpy(cutmp->mateid, userid);

    prints(" %s(%s) �W�� %d ���A�峹 %d �g�A%s�{�ҡC\n",
        userid, (HAS_PERM(PERM_SYSOPX) || !(acct->userlevel & PERM_DENYNICK)) ? acct->username : GUEST_1, acct->numlogins, acct->numposts,
        acct->userlevel & PERM_VALID ? "�w" : "��");

    prints(" �W��(\x1b[1;33m%s\x1b[m)�Ӧ�(%s)\n",
    Ctime_any(&acct->lastlogin), ((acct->ufo & UFO_HIDDEN)&&!HAS_PERM(PERM_SYSOP)) ?
    HIDDEN_SRC : acct->lasthost);

#if defined(REALINFO) && defined(QUERY_REALNAMES)
    if (HAS_PERM(PERM_BASIC))
        prints(" �u��m�W: %s", acct->realname);
    else
#endif
    if (HAS_PERM(PERM_SYSOP))
        prints(" �u��m�W: %s", acct->realname);

    /* ���]�W�� login �w�L 6 �p�ɡA�K���b���W�A��� utmp_find */

    outs(" [�ʺA] ");
    /*  up = (acct->lastlogin < time(0) - 6 * 3600) ? NULL : utmp_find(userno);  */
    up = utmp_find(userno);
    outs("\x1b[1;36m");
    outs((!up || (up && ((!HAS_PERM(PERM_SEECLOAK) && (up->ufo & UFO_CLOAK)) || (can_see(up)==2)) && !HAS_PERM(PERM_SYSOP))) ? "���b���W":bmode(up, 1));
    outs("\x1b[m");
    /* Thor.981108: ������ cigar �����������n�D, ���L�� finger�٬O�i�H�ݨ�:p */

    mail = m_query(userid);
    if (mail)
        prints(" [�H�c] \x1b[1;31m�� %d �ʷs����\x1b[m\n", mail);
    else
        prints(" [�H�c] ���ݹL�F\n");

    static const char *const fortune[] = {"�a�x���v", "�a�Ҵ��q", "�a�Ҥp�d", "�a�ҴI��", "�]�O���p", "�I�i�İ�", "I'm Rich"};

    {
        int money = acct->money / 1000;
        for (rich = 0; rich < COUNTOF(fortune) - 1; ++rich)
        {
            if (money <= 0)
                break;
            money /= 10;
        }
    }

    if (acct->point1 > 10)
        prints(" [�u�}�n��] \x1b[1;32m%d\x1b[m [�H��] %d [�g��] %s", acct->point1, acct->point2, fortune[rich]);
    else if (acct->point2 > 1)
        prints(" [�u�}�n��] %d [�H��] \x1b[1;31m%d\x1b[m [�g��] %s", acct->point1, acct->point2, fortune[rich]);
    else
        prints(" [�u�}�n��] %d [�H��] %d [�g��] %s", acct->point1, acct->point2, fortune[rich]);

    if (paling == 2)
        ;
    else
    {
        /* deny nick can't have plans. statue.2001.02.11 */
        if (HAS_PERM(PERM_SYSOPX))
            showplans(userid);
        else if (!(acct->userlevel & PERM_DENYNICK) && (acct->userlevel & PERM_VALID))
            showplans(userid);
    }

    vmsg(NULL);

    move(b_lines, 0);
    clrtoeol();
}


void
my_query(
    const char *userid,
    int paling)                 /* �O�_���b�]�w�n�ͦW�� */
{
    ACCT acct;

    if (acct_load(&acct, userid) >= 0)
    {
        do_query(&acct, paling);
    }
    else
    {
        vmsg(err_uid);
    }
}


/* ----------------------------------------------------- */
/* BMW : bbs message write routines                      */
/* ----------------------------------------------------- */


#define BMW_FORMAT      "\x1b[1;33;46m��%s \x1b[37;45m %s \x1b[m"
#define BMW_FORMAT_BC   "\x1b[1;37;45m��%s \x1b[1;33;46m %s \x1b[m"
/* patch by visor : BMW_LOCAL_MAX >= BMW_PER_USER
    �H�K�i�J�L���j��                               */
#define BMW_LOCAL_MAX   10


static BMW bmw_lslot[BMW_LOCAL_MAX], bmw_sentlot[BMW_LOCAL_MAX];
//static BMW bmw_lslot[BMW_LOCAL_MAX];
static int bmw_locus;

static int
bmw_send(
    UTMP *callee,
    BMW *bmw)
{
    BMW *mpool, *mhead, *mtail, **mslot;
    int i;
    pid_t pid;
    time_t texpire;

    if ((callee->userno != bmw->recver) || (pid = callee->pid) <= 0)
        return 1;

    /* sem_lock(BSEM_ENTER); */

    /* find callee's available slot */

    mslot = callee->mslot;
    i = 0;

    for (;;)
    {
        if (mslot[i] == NULL)
            break;

        if (++i >= BMW_PER_USER)
        {
            /* sem_lock(BSEM_LEAVE); */
            return 1;
        }
    }

    /* find available BMW slot in pool */

    texpire = time32(&bmw->btime) - BMW_EXPIRE;

    mpool = ushm->mpool;
    mhead = BMAX(ushm->mbase, mpool);
    mtail = mpool + BMW_MAX;

    do
    {
        if (++mhead >= mtail)
            mhead = mpool;
    } while (mhead->btime > texpire);

    *mhead = *bmw;
    ushm->mbase = mslot[i] = mhead;
    /* Thor.981206: �ݪ`�N, �Yushm mapping���P,
                    �h���P�� bbsd ��call�|core dump,
                    ���D�o�]��offset, ���L���F -i, ���ӬO�D���n */


    /* sem_lock(BSEM_LEAVE); */
    return kill(pid, SIGUSR2);
}


/*static */void
bmw_edit(
    UTMP *up,
    const char *hint,
    BMW *bmw,
    int cc)
{
    char *str;
    footer_backup_t old_footer;

    if (up)
        bmw->recver = up->userno;       /* ���O�U userno �@�� check */

    if (!cc)
        foot_dump(&old_footer);

    str = bmw->msg;

    memset(str, 0, sizeof(bmw->msg));

    str[0] = cc;
    str[1] = '\0';

    if (vget(B_LINES_REF - 1, 0, hint, str, BMIN(58UL, sizeof(bmw->msg)), GCARRY) &&
                                        /* lkchu.990103: �s�����u���\ 48 �Ӧr�� */
        vans("�T�w�n�e�X�m���T�n��(Y/n)�H[Y] ") != 'n')
    {
        //FILE *fp;
        char *userid, fpath[64];


        bmw->caller = cutmp;
        bmw->sender = cuser.userno;
        strcpy(bmw->userid, userid = cuser.userid);

        if (up)
        {
            if (bmw_send(up, bmw))
            {
                vmsg(MSG_USR_LEFT);
                if (!cc)
                    foot_restore_free(&old_footer);
                return;
            }
        }

        /* lkchu.981230: �Q�� xover ��X bmw */
        if (up)
            strcpy(bmw->userid, up->userid);
            /* lkchu.990103: �Y�O�ۤv�e�X�� bmw, �s��誺 userid */
        else
            *bmw->userid = '\0';        /* lkchu.990206: �n�ͼs���]�� NULL */

        time32(&bmw->btime);
        usr_fpath(fpath, userid, FN_BMW);
        rec_add(fpath, bmw, sizeof(BMW));
        strcpy(bmw->userid, userid);
    }

    if (!cc)
        foot_restore_free(&old_footer);
}

/*�W���^�U*/
static void bmw_display(int max, int pos)
{
    int i=9, j, sent;
    BMW bmw, bmw2;
    char buf[128], color[10];

//  bmw_pos = pos;
    move(i, 0);
    clrtobot();
    i++;
    move(i, 0);
    prints(" \x1b[1;36m��w�w�w�w�w�w��\x1b[43;37m              �ڤj�W�����y�^�U              \x1b[40;36m��w�w�w�w�w�w��\x1b[m");

    i++;
    for (max=0; max<8; max++)
    {
        bmw = bmw_lslot[max];
        if (max == pos)
            bmw2 = bmw;


        if (max == pos)
            sprintf(color, "1;45");
        else
            sprintf(color, "0");

        if (strstr(bmw.msg, "���s��"))
            sprintf(buf, "   \x1b[1;45;37m[%-*s]\x1b[%sm %-58s\x1b[m", IDLEN, bmw.userid, color, (bmw.msg+8));
        else
            sprintf(buf, "   \x1b[37;%sm[\x1b[33m%-*s\x1b[37m] %-58s\x1b[m", color, IDLEN, bmw.userid, bmw.msg);
        move(i, 0);
        outs(buf);
        i++;
    }

    move(i, 0);
    outs(" \x1b[1;36m�u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t\x1b[m\n");
    sent = 0;
    for (j=0; j<BMW_LOCAL_MAX; j++)
    {
        bmw = bmw_sentlot[j];
        if (!strncmp(bmw.userid, bmw2.userid, IDLEN))
        {
            sent = 1;
            bmw = bmw_sentlot[j];
            sprintf(buf, "  \x1b[1;32mTo %-*s\x1b[m: \x1b[32m%-57s\x1b[m", IDLEN, bmw.userid, bmw.msg);
            outs(buf);
            break;
        }
    }
    if (!sent)
        prints("  \x1b[32m�|���ǰT�� %s\x1b[m", bmw2.userid);
    move(i+2, 0);
    prints(" \x1b[1;36m���w�w�w�w�w�w��\x1b[43;37m �ާ@ (���BEnter)���} (����)��� (��L)�^�T \x1b[40;36m��w�w�w�w�w�w��\x1b[m ");

}


void bmw_reply(int replymode)/* 0:�@��ctrl+r 1:�⦸ctrl+r */
{
    int userno, max, pos, cc, mode;
    UTMP *up, *uhead;
    BMW bmw;
    footer_backup_t old_footer;
    screen_backup_t old_screen;
    char buf[128];
    bool reload = false;

    max = bmw_locus - 1;

    foot_dump(&old_footer);  /* Thor.981222: �Q�M��message */

    if (max < 0)
    {
        vmsg("���e�õL���T�I�s");
        foot_restore_free(&old_footer); /* Thor.981222: �Q�M��message */
        return;
    }

    if (cuser.ufo2 & UFO2_REPLY || replymode)
        scr_dump(&old_screen);  /* �O�� bmd_display ���e�� screen */

    mode = bbsmode;               /* lkchu.981201: save current mode */
    utmp_mode(M_BMW_REPLY);

    if (!(cuser.ufo2 & UFO2_REPLY) && !replymode)
    {
        move(b_lines - 1, 0);
        clrtoeol();
        prints(FOOTER_BMW_REPLY, d_cols, "");
    }

    reload = true;
    pos = max;

    uhead = ushm->uslot;

    for (;;)
    {
        if (reload)
        {
            reload = false;
            bmw = bmw_lslot[pos];
            if (cuser.ufo2 & UFO2_REPLY || replymode)
                bmw_display(max, pos);
            else
            {
                if (strstr(bmw.msg, "���s��"))
                    sprintf(buf, BMW_FORMAT_BC, bmw.userid, (bmw.msg)+8);
                else
                    sprintf(buf, BMW_FORMAT, bmw.userid, bmw.msg);
                outz(buf);
            }
        }

        cc = vkey();

        if (cc == '\n')
        {
            if (cuser.ufo2 & UFO2_REPLY || replymode)
            {
                scr_restore_free(&old_screen);  /* �٭� bmw_display ���e�� screen */
                foot_free(&old_footer);
            }
            else
                foot_restore_free(&old_footer);
            utmp_mode(mode);
            return;
        }

        if ((cc == Ctrl('R') || cc == Meta('R')) && !replymode)
        {
            bmw_reply(1);
            break;
        }

        if (cc == KEY_UP)
        {
            if (pos > 0)
            {
                pos--;
                reload = true;
            }
            continue;
        }

        if (cc == KEY_DOWN)
        {
            if (pos < max)
            {
                pos++;
                reload = true;
            }
            continue;
        }

        if (cc == KEY_RIGHT)
        {
            if (pos != max)
            {
                pos = max;
                reload = true;
            }
            continue;
        }

        if (cc == KEY_LEFT)
        {
            if (cuser.ufo2 & UFO2_REPLY || replymode)
                scr_restore_keep(&old_screen);  /* �٭� bmw_display ���e�� screen */
            break;
        }

        if (isprint2(cc))
        {
            userno = bmw.sender; /* Thor.980805: ����t�Ψ�M�^�� */
            if (!userno)
            {
                vmsg("�t�ΰT���L�k�^��");
//              vmsg("�z�ä��O��誺�n�͡A�L�k�^���W���q��");
                /* lkchu.981201: �n�ͥi�^�� */

                break;
            }

            up = bmw.caller;
#if 1
            if ((up < uhead) || (up > uhead + MAXACTIVE /*ushm->ubackidx*/))
                /* lkchu.981201: comparison of distinct pointer types */
            {
                vmsg(MSG_USR_LEFT);
                break;
            }
#endif
            /* userno = bmw.sender; */ /* Thor.980805: ����t�Φ^�� */
            if (up->userno != userno)
            {
                up = utmp_find(userno);
                if (!up)
                {
                    vmsg(MSG_USR_LEFT);
                    break;
                }
            }

            if (up->ufo & UFO_REJECT)
            {
                vmsg("��観�ơA�еy�ݤ@�|��....");
                break;
            }

#ifdef  HAVE_SHOWNUMMSG
            if (up->num_msg > 9 && (up->ufo & UFO_MAXMSG) && !HAS_PERM(PERM_SYSOP))
            {
                vmsg("���w�g�Q���y���z�F!!");
                break;
            }
#endif

#ifdef  HAVE_BANMSG
            if (!(up->ufo & UFO_MESSAGE) && can_banmsg(up))
            {
                vmsg("��褣�Qť��z���n��!!");
                break;
            }
#endif

            /* bmw_edit(up, "���^���G", &bmw, cc); */
            /* Thor.981214: ���Ϧ^�����P�˲V */
            sprintf(buf, "��[%s]", up->userid);
            bmw_edit(up, buf, &bmw, cc);
            break;
        }
    }

    if (cuser.ufo2 & UFO2_REPLY || replymode)
    {
        scr_restore_free(&old_screen);
        foot_free(&old_footer);
    }
    else
        foot_restore_free(&old_footer);

    utmp_mode(mode);      /* lkchu.981201: restore bbsmode */
}

/* ----------------------------------------------------- */
/* Thor.0607: system assist user login notify            */
/* ----------------------------------------------------- */


#define MSG_CC "\x1b[32m[�s�զW��]\x1b[m\n"

int
pal_list(
    int reciper)
{
    LIST list;
    int userno, fd;
    char buf[32], fpath[64], msg[128], temp[30];
    ACCT acct;
    sprintf(msg, "A)�W�[ D)�R�� F)�n�� G)���� [1~%d]�s�զW�� M)�w�� Q)�����H[M]", MAX_LIST);

    userno = 0;

    for (;;)
    {
        switch (vget(1, 0, msg, buf, 2, LCECHO))
        {
        case '1': case '2': case '3': case '4': case '5': case '6': case '7':
        case '8': case '9':
            sprintf(temp, "list.%c", *buf);
            usr_fpath(fpath, cuser.userid, temp);
            fd = open(fpath, O_RDONLY);
            while (fd >= 0)
            {
                if (read(fd, &list, sizeof(LIST)) == sizeof(LIST))
                {
                    if (!ll_has(list.userid))
                    {
                        ll_add(list.userid);
                        reciper++;
                        ll_out(3, 0, MSG_CC);
                    }
                }
                else
                {
                    close(fd);
                    break;
                }
            }

            break;
        case 'a':
            while (acct_get("�п�J�N��(�u�� ENTER �����s�W): ", &acct) > 0)
            {
                if (!ll_has(acct.userid))
                {
                    ll_add(acct.userid);
                    reciper++;
                    ll_out(3, 0, MSG_CC);
                }
            }
            break;

        case 'd':

            while (reciper)
            {
                if (!vget(1, 0, "�п�J�N��(�u�� ENTER �����R��): ",
                        buf, IDLEN + 1, GET_LIST))
                    break;
                if (ll_del(buf))
                    reciper--;
                ll_out(3, 0, MSG_CC);
            }
            break;
#if 1
        case 'g':
            if ((userno = vget(B_LINES_REF, 0, "�s�ձ���G", buf, 16, DOECHO)))
                str_lower(buf, buf);
            // Falls through
#endif
        case 'f':
            usr_fpath(fpath, cuser.userid, FN_PAL);
            if ((fd = open(fpath, O_RDONLY)) >= 0)
            {
                PAL *pal;
                char *userid;

                mgets(-1);
                while ((pal = (PAL *) mread(fd, sizeof(PAL))))
                {
                    userid = pal->userid;
                    if (!ll_has(userid) && (pal->userno != cuser.userno) &&
                        !(pal->ftype & PAL_BAD) &&
                        (!userno || str_casestr(pal->ship, buf)))
                    {
                        ll_add(userid);
                        reciper++;
                    }
                }
                close(fd);
            }
            ll_out(3, 0, MSG_CC);
            userno = 0;
            break;

        case 'q':
            return 0;

        default:
            return reciper;
        }
    }
}


#ifdef HAVE_ALOHA
void
aloha(void)
{
    UTMP *up, *ubase, *uceil;
    int fd;
    char fpath[64];
    BMW *bmw, benz;
    int userno;
    struct stat st;

    /* lkchu.981201: �n�ͳq�� */

    usr_fpath(fpath, cuser.userid, FN_FRIEND_BENZ);
    if (((fd = open(fpath, O_RDONLY)) >= 0) && !fstat(fd, &st))
    {
        if (st.st_size <= 0)
        {
            close(fd);
            return;
        }
        benz.caller = cutmp;
        /* benz.sender = cuser.userno; */
        benz.sender = 0; /* Thor.980805: �t�Ψ�M����V call in */
        strcpy(benz.userid, cuser.userid);
        sprintf(benz.msg, "�� �i �J (%s) �o!! ��", BOARDNAME);

        ubase = ushm->uslot;
        uceil = ubase + ushm->ubackidx;

        mgets(-1);
        while ((bmw = (BMW *) mread(fd, sizeof(BMW))))
        {
            /* Thor.1030:�u�q���B�� */

            userno = bmw->recver;
            /* up = bmw->caller; */
            up = utmp_find(userno);     /* lkchu.981201: frienz ���� utmp �L�� */

            if (up >= ubase && up <= uceil &&
                up->userno == userno && !(cuser.ufo & UFO_CLOAK) && can_message(up))
                                        /* Thor.980804: �����W�����q��, �����S�v */
                                        /* lkchu.981201: ���n���}�u�n�ͤW���q���v�~�q�� */
            {
                /*if (is_pal(userno))*/           /* lkchu.980806: �n�ͤ~�i�H reply */
                /*    benz.sender = cuser.userno;
                else*/
                if (!is_bad(userno) || (up->userlevel & PERM_SYSOP))
                {
                    benz.sender = 0;

                    benz.recver = userno;
                    bmw_send(up, &benz);
                }
            }
        }
        close(fd);
    }
}
#endif  /* #ifdef HAVE_ALOHA */


#ifdef LOGIN_NOTIFY
int
t_loginNotify(void)
{
    LinkList *wp;
    BMW bmw;
    char fpath[64];

    /* �]�w list ���W�� */

    vs_bar("�t�Ψ�M����");

    ll_new();

    if (pal_list(0))
    {
        wp = ll_head;
        bmw.caller = cutmp;
        bmw.recver = cuser.userno;
        strcpy(bmw.userid, cuser.userid);

        /* Thor.980629: �pbug, �i�H��M�ۤv, �i��call-in�ۤv:P */

        do
        {
            usr_fpath(fpath, wp->data, FN_BENZ);
            rec_add(fpath, &bmw, sizeof(BMW));
        } while ((wp = wp->next));

        vmsg("��M�]�w�����A�B�ͤW����" SYSOPNICK "�|�q���z�A�Фŭ��г]�w!");

        /* Thor.1030 : �Y���O�B�ʹN���q����...... �H�Kcallin ����� */
    }
    return 0;
}


void
loginNotify(void)
{
    UTMP *up, *ubase, *uceil;
    int fd;
    char fpath[64];
    BMW *bmw, benz;
    int userno;

    usr_fpath(fpath, cuser.userid, FN_BENZ);

    if ((fd = open(fpath, O_RDONLY)) >= 0)
    {
        vs_bar("�t�Ψ�M����");

        benz.caller = cutmp;
        /* benz.sender = cuser.userno; */
        benz.sender = 0; /* Thor.980805: �t�Ψ�M����V call in */
        strcpy(benz.userid, cuser.userid);
        sprintf(benz.msg, "�� ����i%s���� [�t�Ψ�M] ��", BOARDNAME);

        ubase = ushm->uslot;
        uceil = ubase + ushm->ubackidx;

        mgets(-1);
        while ((bmw = (BMW *) mread(fd, sizeof(BMW))))
        {
            /* Thor.1030:�u�q���B�� */

            up = bmw->caller;
            userno = bmw->recver;

            if (up >= ubase && up <= uceil &&
                up->userno == userno && !(cuser.ufo & UFO_CLOAK) && userno != cuser.userno  && can_see(up) !=2)
                                        /* Thor.980804: �����W�����q��, �����S�v */
            {
/*              benz.sender = is_pal(userno) ? cuser.userno : 0; */
                      /* lkchu.980806: �n�ͤ~�i�H reply */
                if (!is_bad(userno) || (up->userlevel & PERM_SYSOP))
                {
                    benz.sender = 0;
                    benz.recver = userno;
                    bmw_send(up, &benz);

                    prints("*");  /* Thor.980707: ���q���쪺���Ҥ��P */
                }
            }

            prints("%-*s ", IDLEN, bmw->userid);

        }
        close(fd);
        unlink(fpath);
        vmsg(NULL);
    }
}
#endif  /* #ifdef LOGIN_NOTIFY */


/* Thor: for ask last call-in messages */

/* lkchu: ���T�^�U�s���� */
int
t_recall(void)
{
    xover(XZ_BMW);
    return 0;
}


#ifdef LOG_TALK
void
talk_save(void)
{
    char fpath[64];
    struct stat st;

    usr_fpath(fpath, cuser.userid, FN_TALK_LOG);
    stat(fpath, &st);

    if (!(cuser.ufo2 & UFO2_NTLOG) && st.st_size > 0)
    {
        char buf[64];
        HDR fhdr;

        usr_fpath(buf, cuser.userid, fn_dir);
        hdr_stamp(buf, HDR_LINK, &fhdr, fpath);
        strcpy(fhdr.title, "[�� �� ��] ��Ѭ���");
        strcpy(fhdr.owner, cuser.userid);
        fhdr.xmode = MAIL_READ | MAIL_NOREPLY;
        rec_add(buf, &fhdr, sizeof(fhdr));

    }
    unlink(fpath);
    return;

}
#endif


#ifdef LOG_BMW
void
bmw_save(void)
{
    int fd, check_max;
    char ans;
    char fpath[64], buf[128];
    struct stat st;


    usr_fpath(fpath, cuser.userid, FN_BMW);
    fd = f_open(fpath); /* lkchu.990428: �Y size �� 0 �|�Q unlink �� */
    if (fd >= 0)
    {
        fstat(fd, &st);
        check_max = st.st_size;
    }
    else
        check_max = 0;


    /* lkchu.981201: ��i�p�H�H�c��/�M��/�O�d */
    if (fd >= 0 && check_max > sizeof(BMW))
    {
        if (check_max >= BMW_MAX_SIZE * 1024)
            ans = 'm';
        else if (cuser.ufo2 & UFO2_NWLOG)
            ans = 'r';
        else
        {
            sprintf(buf, "�����W�����T�B�z (M)�Ƨѿ� (R)�O�d (C)�M�� [%dk/%dk] �H[R] ", check_max/1024, BMW_MAX_SIZE);
            ans = vans(buf);
        }
        switch (ans)
        {
        case 'c':
            close(fd);
            unlink(fpath);
            break;

        case 'r':
            close(fd);
            break;

        case 'm':
            {
                FILE *fout;
                char buf[80], folder[80];
                HDR fhdr;

                usr_fpath(folder, cuser.userid, fn_dir);
                if ((fout = fdopen(hdr_stamp(folder, 0, &fhdr, buf), "w")))
                {
                    BMW bmw;

                    while (read(fd, &bmw, sizeof(BMW)) == sizeof(BMW))
                    {
                        struct tm *ptime = localtime_any(&bmw.btime);

                        fprintf(fout, "%s%s(%02d:%02d)�G%s\x1b[m\n",
                            bmw.sender == cuser.userno ? "��" : "\x1b[32m��",
                            bmw.userid, ptime->tm_hour, ptime->tm_min, bmw.msg);
                    }
                    fclose(fout);
                }
                close(fd);

                fhdr.xmode = MAIL_READ | MAIL_NOREPLY;
                strcpy(fhdr.title, "[�� �� ��] ���T����");
                strcpy(fhdr.owner, cuser.userid);
                rec_add(folder, &fhdr, sizeof(fhdr));

                unlink(fpath);
            }
            break;

        default:
            close(fd);
            break;
        }
    }

}
#endif  /* #ifdef LOG_BMW */


void
bmw_rqst(void)
{
    int i, j, userno, locus;
    BMW bmw[BMW_PER_USER], *mptr, **mslot;

    /* download BMW slot first */

    i = j = 0;
    userno = cuser.userno;
    mslot = cutmp->mslot;

    while ((mptr = mslot[i]))
    {
        mslot[i] = NULL;
        if (mptr->recver == userno)
        {
            bmw[j++] = *mptr;
        }
        mptr->btime = 0;

        if (++i >= BMW_PER_USER)
            break;
    }

    /* process the request */

    if (j)
    {
        char buf[128];

        locus = bmw_locus;
        i = locus + j - BMW_LOCAL_MAX;
        if (i >= 0)
        {
            locus -= i;
            /* IID.20191222: Large `locus` && small `j` => `locus` > `i` => overlap */
            memmove(bmw_lslot, bmw_lslot + i, locus * sizeof(BMW));
        }

        i = 0;
        do
        {
            mptr = &bmw[i];

            /* lkchu.981230: �Q�� xover ��X bmw */
            usr_fpath(buf, cuser.userid, FN_BMW);
            rec_add(buf, &bmw[i], sizeof(BMW));

            bmw_lslot[locus++] = *mptr;
        } while (++i < j);

        bmw_locus = locus;

        if (!(cutmp->ufo & (UFO_REJECT | UFO_NET)))
        {
            if (strstr(mptr->msg, "���s��"))
                sprintf(buf, BMW_FORMAT_BC, mptr->userid, (mptr->msg)+8);
            else
                sprintf(buf, BMW_FORMAT, mptr->userid, mptr->msg);

            /* Thor.980827: ���F����C�L�@�b(more)�ɼ��T�ӫ�C�L�W�L�d���H,
                            �G�s�U��Ц�m */
            cursor_save();

            outz(buf);
            /* Thor.980827: ���F����C�L�@�b(more)�ɼ��T�ӫ�C�L�W�L�d���H,
                            �G�٭��Ц�m */
            cursor_restore();

            refresh();
            bell();
#ifdef  HAVE_SHOWNUMMSG
            cutmp->num_msg++;
#endif
        }
    }
}


/* ----------------------------------------------------- */
/* talk sub-routines                                     */
/* ----------------------------------------------------- */


static void
talk_nextline(
    talk_win *twin)
{
    int curln;

    curln = twin->curln + 1;
    if (curln > twin->eline)
        curln = twin->sline;
    if (curln != twin->eline)
    {
        move(curln + 1, 0);
        clrtoeol();
    }
    move(curln, 0);
    clrtoeol();
    twin->curcol = 0;
    twin->curln = curln;
}


static void
talk_char(
    talk_win *twin,
    int ch)
{
    int col, ln;

    col = twin->curcol;
    ln = twin->curln;

    if (isprint2(ch))
    {
        if (col < 79)
        {
            move(ln, col);
            twin->curcol = ++col;
        }
        else
        {
            talk_nextline(twin);
            twin->curcol = 1;
        }
        outc(ch);
    }
    else if (ch == '\n')
    {
        talk_nextline(twin);
    }
    else if (ch == Ctrl('H'))
    {
        if (col)
        {
            twin->curcol = --col;
            move(ln, col);
            outc(' ');
            move(ln, col);
        }
    }
    else if (ch == Ctrl('G'))
    {
        bell();
    }
}


static void
talk_string(
    talk_win *twin,
    const char *str)
{
    int ch;

    while ((ch = (unsigned char) *str))
    {
        talk_char(twin, ch);
        str++;
    }
}


static void
talk_speak(
    int fd)
{
    talk_win mywin, itswin;
    unsigned char data[80];
    char buf[80];
    int i, ch;
#ifdef  LOG_TALK
    char mywords[80], itswords[80], itsuserid[40];
    FILE *fp;

    /* lkchu: make sure that's empty */
    mywords[0] = itswords[0] = '\0';

    strcpy(itsuserid, page_requestor);
    strtok(itsuserid, " (");
#endif

    utmp_mode(M_TALK);

    ch = 60 + d_cols - strlen(page_requestor);

    sprintf(buf, "%s�i%s", cuser.userid, cuser.username);

    i = ch - strlen(buf);
    if (i < 0)
    {
        buf[ch] = '\0';
        i = 0;
    }

    memset(&mywin, 0, sizeof(mywin));
    memset(&itswin, 0, sizeof(itswin));

    ch = b_lines >> 1;
    mywin.eline = ch - 1;
    itswin.curln = itswin.sline = ch + 1;
    itswin.eline = b_lines - 1;

    clear();
    move(ch, 0);
    prints("\x1b[1;46;37m  �ͤѻ��a  \x1b[45m%*s%s�j ��  %s%*s\x1b[m",
        i>>1, "", buf, page_requestor, (i+1)>>1, "");
#if 1
    outf(FOOTER_TALK);
#endif
    move(0, 0);

#ifdef LOG_TALK                            /* lkchu.981201: ��ѰO�� */
    usr_fpath(buf, cuser.userid, FN_TALK_LOG);
    if ((fp = fopen(buf, "a+")))
        fprintf(fp, "�i %s �P %s ����ѰO�� �j\n", cuser.userid, page_requestor);
#endif

    add_io(fd, 60);

    for (;;)
    {
        ch = vkey();

#ifdef EVERY_Z
        /* Thor.0725: talk��, ctrl-z */
        if (ch == Ctrl('Z'))
        {
            screen_backup_t old_screen;
            char buf[IDLEN + 1];
            /* Thor.0731: �Ȧs mateid, �]�����i��query�O�H�ɷ|�α�mateid */
            strcpy(buf, cutmp->mateid);

            /* Thor.0727: �Ȧs vio_fd */
            holdon_fd = vio_fd;
            vio_fd = 0;
            scr_dump(&old_screen);
            every_Z(NULL);
            scr_restore_free(&old_screen);
            /* Thor.0727: �٭� vio_fd */
            vio_fd = holdon_fd;
            holdon_fd = 0;

            /* Thor.0731: �٭� mateid, �]�����i��query�O�H�ɷ|�α�mateid */
            strcpy(cutmp->mateid, buf);
            continue;
        }
#endif  /* #ifdef EVERY_Z */
        if (ch == Ctrl('U'))
        {
            char buf[IDLEN + 1];
            strcpy(buf, cutmp->mateid);

            holdon_fd = vio_fd;
            vio_fd = 0;
            every_U();
            vio_fd = holdon_fd;
            holdon_fd = 0;

            strcpy(cutmp->mateid, buf);
            continue;
        }
        if (ch == Ctrl('D') || ch == Ctrl('C'))
            break;

        if (ch == I_OTHERDATA)
        {
            ch = recv(fd, data, 80, 0);
            if (ch <= 0)
                break;
#if 0   // IID.20190508: `bwboard.so` and `chess.so` do not exist anymore.
            if (data[0] == Ctrl('A') || data[0] == Meta('A'))
            { /* Thor.990219: �I�s�~���ѽL */
                if (DL_NAME_CALL("bwboard.so", BWboard)(fd, 1)==-2)
                    break;
                continue;
            }
            if (data[0] == Ctrl('B'))
            {
                if (DL_NAME_CALL("chess.so", Chess)(fd, 1)==-2)
                    break;
                continue;
            }
#endif
            for (i = 0; i < ch; i++)
            {
                talk_char(&itswin, data[i]);
#ifdef  LOG_TALK
                switch (data[i])
                {
                case '\n':
                    /* lkchu.981201: ������N�� itswords �L�X�M�� */
                    if (itswords[0] != '\0')
                    {
                        fprintf(fp, "\x1b[32m%s�G%s\x1b[m\n", itsuserid, itswords);
                        itswords[0] = '\0';
                    }
                    break;

                case Ctrl('H'): /* lkchu.981201: backspace */
                    itswords[strlen(itswords) - 1] = '\0';
                    break;

                default:
                    if (strlen(itswords) + 1 < sizeof(itswords))
                    {
                        strncat(itswords, (char *)&data[i], 1);
                    }
                    else        /* lkchu.981201: itswords �˺��F */
                    {
                        fprintf(fp, "\x1b[32m%s�G%s%c\x1b[m\n", itsuserid, itswords, data[i]);
                        itswords[0] = '\0';
                    }
                    break;
                }
#endif

            }
        }
#if 0   // IID.20190508: `bwboard.so` and `chess.so` do not exist anymore.
        else if (ch == Ctrl('A') || ch == Meta('A'))
        { /* Thor.990219: �I�s�~���ѽL */
            /* int BWboard(int sock, int later); */
            data[0] = ch;
            if (send(fd, data, 1, 0) != 1)
                break;
            /* if (BWboard(fd, 0)==-2) */
            if (DL_NAME_CALL("bwboard.so", BWboard)(fd, 0)==-2)
                break;
        }
        else if (ch == Ctrl('B'))
        { /* Thor.990219: �I�s�~���ѽL */
            /* int BWboard(int sock, int later); */
            data[0] = ch;
            if (send(fd, data, 1, 0) != 1)
                break;
            /* if (BWboard(fd, 0)==-2) */
            if (DL_NAME_CALL("chess.so", Chess)(fd, 0)==-2)
                break;
        }
#endif
        else if (isprint2(ch) || ch == '\n' || ch == Ctrl('H') || ch == Ctrl('G'))
        {
            data[0] = ch;
            if (send(fd, data, 1, 0) != 1)
                break;

#ifdef LOG_TALK                         /* lkchu: �ۤv������ */
            switch (ch)
            {
            case '\n':
                if (mywords[0] != '\0')
                {
                    fprintf(fp, "%s�G%s\n", cuser.userid, mywords);
                    mywords[0] = '\0';
                }
                break;

            case Ctrl('H'):
                mywords[strlen(mywords) - 1] = '\0';
                break;

            default:
                if (strlen(mywords) + 1 < sizeof(mywords))
                {
                    strncat(mywords, (char *)&ch, 1);
                }
                else
                {
                    fprintf(fp, "%s�G%s%c\n", cuser.userid, mywords, ch);
                    mywords[0] = '\0';
                }
                break;
            }
#endif

            talk_char(&mywin, ch);

#ifdef EVERY_BIFF
            /* Thor.980805: ���H�b�����enter�~�ݭncheck biff */
            if (ch=='\n')
            {
                static int old_biff, old_biffn;
                int biff = cutmp->ufo & UFO_BIFF;
                if (biff && !old_biff)
                    talk_string(&mywin, "�� ��! �l�t�Ķi�ӤF!\n");
                old_biff = biff;
                biff = cutmp->ufo & UFO_BIFFN;
                if (biff && !old_biffn)
                    talk_string(&mywin, "�� ��! �z�������d��!\n");
                old_biffn = biff;
            }
#endif
        }

    }

#ifdef LOG_TALK
    if (fp)
        fclose(fp);
#endif

    add_io(0, 60);
}


static void
talk_hangup(
    int sock)
{
    cutmp->sockport = 0;
    add_io(0, 60);
    close(sock);
}


static const char *const talk_reason[] =
{
    "�藍�_�A�ڦ��Ʊ������A talk",
    "�ڲ{�b�ܦ��A�е��@�|��A call ��",
    "�{�b�����L�ӡA���@�U�ڷ|�D�� page �A",
    "�ڲ{�b���Q talk �� ...",
    "�A�u���ܷСA�ڹ�b���Q��A talk",

#ifdef EVERY_Z
    "�ڪ��L�ڥ����۩M�O�H���ܩO�A�S���Ū��L�ڤF",
    /* Thor.0725: for chat&talk ��^z �@�ǳ� */
#endif
};


/* return 0: �S�� talk, 1: �� talk, -1: ��L */


static int
talk_page(
    UTMP *up)
{
    int sock = -1;
    int msgsock;
    pid_t pid;
    struct sockaddr_storage sin;
    int ans, length, myans;
    char buf[60];
    struct addrinfo hints = {0};
    struct addrinfo *hs;

    /* IID.2021-02-19: Fallback to native IPv4 when IPv6 is not available */
    static const sa_family_t ai_family[] = {AF_INET6, AF_INET};

#ifdef EVERY_Z
    /* Thor.0725: �� talk & chat �i�� ^z �@�ǳ� */
    if (holdon_fd)
    {
        vmsg("�z�������@�b�٨S�����C");
        return 0;
    }
#endif

    pid = up->mode;

    if (pid >= M_BBTP && pid <= M_CHAT)
    {
        vmsg("���L�v���");
        return 0;
    }

    if (!(pid = up->pid) || kill(pid, 0))
    {
        vmsg(MSG_USR_LEFT);
        return 0;
    }

    /* showplans(up->userid); */
#ifdef  HAVE_PIP_FIGHT
    myans = vans("�n�M�L/�o�ͤ�(y)�ι�Ԥp��(c)�� (y/N/c)?[N] ");
#else
    myans = vans("�T�w�n�M�L/�o�ͤѶ� (y/N)?[N] ");
#endif

#ifdef  HAVE_PIP_FIGHT
    if (myans != 'y' && myans != 'c')
#else
    if (myans != 'y')
#endif
        return 0;

#ifdef  HAVE_PIP_FIGHT
    if (myans == 'c')
    {
        usr_fpath(buf, up->userid, "chicken");
        if (access(buf, 0))
        {
            vmsg("���S���i�p���I");
            return 1;
        }
    }
#endif

    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_ADDRCONFIG | AI_NUMERICSERV | AI_PASSIVE;
    for (int i = 0; i < COUNTOF(ai_family); ++i)
    {
        hints.ai_family = ai_family[i];
        if (getaddrinfo(NULL, "0", &hints, &hs))
            continue;

        for (struct addrinfo *h = hs; h; h = h ->ai_next)
        {
            sock = socket(h->ai_family, h->ai_socktype, 0);
            if (sock < 0)
                continue;

            length = sizeof(sin);
            if (bind(sock, h->ai_addr, h->ai_addrlen) < 0 || getsockname(sock, (struct sockaddr *) &sin, (socklen_t *) &length) < 0)
            {
                close(sock);
                sock = -1;
                continue;
            }

            /* Success */
            break;
        }
        freeaddrinfo(hs);

        if (sock >= 0)
            break; /* Success */
    }
    if (sock < 0)
        return 0;

    {
        char port_str[12];
        getnameinfo((struct sockaddr *)&sin, (socklen_t)length, NULL, NI_MAXHOST, port_str, sizeof(port_str), NI_NUMERICSERV);
        cutmp->sockport = atoi(port_str);
    }
    strcpy(cutmp->mateid, up->userid);
    up->talker = cutmp;
#ifdef  HAVE_PIP_FIGHT
    if (myans == 'c')
        utmp_mode(M_CHICKEN);
    else
#endif
        utmp_mode(M_PAGE);
    kill(pid, SIGUSR1);

    clear();
    prints("���שI�s %s ...\n�i�� Ctrl-D ����", up->userid);

    listen(sock, 1);
    add_io(sock, 20);
    do
    {
        msgsock = vkey();

        if (msgsock == Ctrl('D'))
        {
            talk_hangup(sock);
            return -1;
        }

        if (msgsock == I_TIMEOUT)
        {
            move(0, 0);
            outs("�A");
            bell();

            if (kill(pid, SIGUSR1))
            /* Thor.990201:����:���o��kill, �]�u�O�ݬݹ��O���O�٦b�u�W�Ӥw:p
                                ���osignal�����G talk_rqst���|�A�Q�s */
            {
                talk_hangup(sock);
                vmsg(MSG_USR_LEFT);
                return -1;
            }
        }
    } while (msgsock != I_OTHERDATA);

    msgsock = accept(sock, NULL, NULL);
    talk_hangup(sock);
    if (msgsock == -1)
        return -1;

    length = read(msgsock, buf, sizeof(buf));
    ans = buf[0];
    if (ans == 'y')
    {
        sprintf(page_requestor, "%s (%s)", up->userid, up->username);

        /*
        * Thor.0814: �`�N! �b�����@�����P�n�����i�ౡ�p, �p�G askia ��page
        * starlight, ���b starlight �^���e�o���W���}, �� page backspace,
        * backspace�|���^���e, �p�G starlight �^���F, starlight �N�|�Q accept,
        * �Ӥ��O backspace.
        *
        * ���ɦb�ù�����, �ݨ쪺 page_requestor�|�O backspace, �i�O�ƹ�W,
        * talk����H�O starlight, �y�����P�n��!
        *
        * �Ȯɤ����ץ�, �H�@����ߪ̪��g�@:P
        */

        talk_speak(msgsock);
    }
#ifdef  HAVE_PIP_FIGHT
    else if (ans == 'c')
    {
        DL_HOTSWAP_SCOPE int (*p)(int, int) = NULL;
        if (!p)
            p = DL_NAME_GET("pip.so", pip_vf_fight);
        if (p)
        {
            up->pip = NULL;
            (*p)(msgsock, 2);
            cutmp->pip = NULL;
        }
        add_io(0, 60);
/*      pip_vf_fight(msgsock, 1);*/
    }
    else if (ans == 'C')
    {
        outs("�i�^���j��褣�Q PK �p���I");
    }
#endif
    else
    {
        const char *reply;

        if (ans == ' ')
        {
            buf[length] = '\0';
            reply = buf;
        }
        else
            reply = talk_reason[ans - '1'];

        move(4, 0);
        outs("�i�^���j");
        outs(reply);
    }

    close(msgsock);
    cutmp->talker = NULL;
#ifdef  LOG_TALK
    if (ans == 'y' && cutmp->mode != M_CHICKEN)    /* mat.991011: ����Talk�Q�ڵ��ɡA���Ͳ�ѰO����record */
        talk_save();          /* lkchu.981201: talk �O���B�z */
#endif
    vmsg("��ѵ���");
    return 1;
}


/*-------------------------------------------------------*/
/* ��榡��Ѥ���                                        */
/*-------------------------------------------------------*/


static PICKUP ulist_pool[MAXACTIVE];
#ifdef  FRIEND_FIRST
static int friend_num, ofriend_num, pfriend_num, bfriend_num;/* visor.991103: �O���ثe���W�n�ͼ� */
#endif
static int ulist_head(XO *xo);
static int ulist_init(XO *xo);
static XO ulist_xo;


static const char *const msg_pickup_way[] =
{
    "���N",
    "�N��",
    "�G�m",
    "�ʺA",
    "�ʺ�",
    "���m",
#ifdef  HAVE_BOARD_PAL
    "�O��",
#endif
};

static char
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
    PICKUP *pp;
    UTMP *up;
    int paltmp;
    int n, cnt, max, ufo, self, userno, sysop GCC_UNUSED, diff, diffmsg, fcolor, colortmp;
    char buf[16], color[20], ship[80];
    static const char *const wcolor[7] = {"\x1b[m", COLOR_PAL, COLOR_BAD, COLOR_BOTH, COLOR_OPAL, COLOR_CLOAK, COLOR_BOARDPAL};
    time_t now;

#ifdef HAVE_BOARD_PAL
    bool isbpal;

    isbpal = (cutmp->board_pal != -1);
#endif

    max = xo->max;
    if (max <= 0)
    {
        return XO_QUIT;
    }

    cnt = xo->top;
    pp = &ulist_pool[cnt];
    self = cuser.userno;
    sysop = HAS_PERM(PERM_SYSOP | PERM_ACCOUNTS);
    time(&now);

    n = 2;
    while (++n < b_lines)
    {
        move(n, 0);
        if (cnt++ < max)
        {
            up = pp->utmp;

            if ((userno = up->userno) && (up->userid[0]) && !((up->ufo & UFO_CLOAK) && !HAS_PERM(PERM_SEECLOAK) && (up->userno != cuser.userno)))
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

                paltmp = (pp->type == 1 || pp->type == 2);
                fcolor = (userno == self) ? 3 : paltmp;

#ifdef  HAVE_BOARD_PAL
                if (isbpal && is_boardpal(up) && userno != self)
                    fcolor = 6;
#else
                if (userno != self)
                    fcolor = 6;
#endif

                colortmp = (pp->type == 1 || pp->type == 3);

                if (paltmp && colortmp == 1)
                    fcolor = 3;
                else if (!paltmp && colortmp == 1)
                    fcolor = 4;

                if (is_bad(userno))
                    fcolor = 2;

                if (cuser.ufo2 & UFO2_SHIP)
                {
                    strcpy(ship, " ");
                    copyship(ship, userno);
                }

                ufo = up->ufo;


                diff = ck_state(UFO_PAGER, UFO_PAGER1, up, 1);
                diffmsg = ck_state(UFO_QUIET, UFO_MESSAGE, up, 0);

                colortmp = 1;

                if (ufo & UFO_CLOAK)
                    fcolor = 5;
                else if (fcolor == 0)
                    colortmp = 0;

                strcpy(color, wcolor[fcolor]);

                prints("%6d%c%s%-*s %-*.*s%s%-*.*s%c%c %-*.*s %5.5s",
                    cnt, (up->ufo & UFO_WEB)?'*':' ',
                    color, IDLEN, up->userid,
                    (d_cols >> 1) + 22, (d_cols >> 1) + 21, (HAS_PERM(PERM_SYSOP) && (cuser.ufo2 & UFO2_REALNAME))? up->realname : up->username,
                    colortmp > 0 ? "\x1b[m" : "",
                    ((d_cols+1) >> 1) + 16, ((d_cols+1) >> 1) + 15,
                    (cuser.ufo2 & UFO2_SHIP) ? ship : ((up->ufo & UFO_HIDDEN)&&!HAS_PERM(PERM_SYSOP)) ?
                    HIDDEN_SRC : up->from, diff, diffmsg,
                    IDLEN, IDLEN, bmode(up, 0), buf);
            }
            else
            {
                outs("      < ������ͥ������} >");
            }
            pp++;
        }
        clrtoeol();
    }

    return XO_NONE;
}


static int
ulist_cmp_userid(
    const void *i, const void *j)
{
    const PICKUP *a = (const PICKUP *)i;
    const PICKUP *b = (const PICKUP *)j;
    if (a->type == b->type)
                return str_casecmp(a->utmp->userid, b->utmp->userid);
    else
        return a->type - b->type;
}

static int
ulist_cmp_host(
    const void *i, const void *j)
{
    return str_casecmp(((const PICKUP *)i)->utmp->from, ((const PICKUP *)j)->utmp->from);
}

static int
ulist_cmp_idle(
    const void *i, const void *j)
{
    return ((const PICKUP *)i)->utmp->idle_time - ((const PICKUP *)j)->utmp->idle_time;
}

static int
ulist_cmp_mode(
    const void *i, const void *j)
{
    return ((const PICKUP *)i)->utmp->mode - ((const PICKUP *)j)->utmp->mode;
}

static int
ulist_cmp_nick(
    const void *i, const void *j)
{
    return str_casecmp(((const PICKUP *)i)->utmp->username, ((const PICKUP *)j)->utmp->username);
}

#ifdef  HAVE_BOARD_PAL
static int
ulist_cmp_board(
    const void *i, const void *j)
{
    return ((const PICKUP *)i)->utmp->board_pal - ((const PICKUP *)j)->utmp->board_pal;
}
#endif

static int (*const ulist_cmp[]) (const void *i, const void *j) =
{
    ulist_cmp_userid,
    ulist_cmp_host,
    ulist_cmp_mode,
    ulist_cmp_nick,
    ulist_cmp_idle,
#ifdef  HAVE_BOARD_PAL
    ulist_cmp_board,
#endif
};


static int
ulist_init(
    XO *xo)
{
    UTMP *up, *uceil;
    PICKUP *pp;
    int max, filter, seecloak, userno, self, nf_num, tmp;
    bool ispal, bad;
#ifdef HAVE_BOARD_PAL
    bool isbpal;
#endif

    pp = ulist_pool;

    self = cuser.userno;
    filter = cuser.ufo2 & UFO2_PAL;

    if (cutmp->pid <= 0 || cutmp->userno <= 0)
        reset_utmp();

    seecloak = HAS_PERM(PERM_SEECLOAK);

    up = ushm->uslot;
    uceil = up + ushm->ubackidx;

    max = 0;
    bad = false;
#ifdef HAVE_BOARD_PAL
    board_pals = 0;
    isbpal = (cutmp->board_pal != -1);
#endif
#ifdef  FRIEND_FIRST    /* by visor : ���g�n�͸m�e */

    friend_num = ofriend_num = pfriend_num = nf_num = bfriend_num = 0;

    do                                  /* ����n�� */
    {
        userno = up->userno;
        if (userno <= 0 || (up->pid <= 0 && !HAS_PERM(PERM_SYSOP|PERM_SEECLOAK)))
            continue;
        if (!seecloak && (up->ufo & UFO_CLOAK))
            continue;
        tmp = can_see(up);
        if (is_bad(userno))
        {
            bfriend_num++;
            bad = true;
        }
        else
            bad = true;

        if (((seecloak || !(up->ufo & UFO_CLOAK)) && (tmp != 2)) || HAS_PERM(PERM_SYSOP|PERM_SEECLOAK) || up->userno == cuser.userno)
        {
#ifdef HAVE_BOARD_PAL
            if (isbpal && is_boardpal(up))
                board_pals++;
#endif
            ispal = is_pal(userno);

            if ((!bad && (ispal && (tmp == 1))) || (userno == self))
            {
                pp->utmp = up;
                pp->type = 1;
                friend_num++;
                pp++;
            }
            else if (ispal && !(tmp == 1) && !filter && !bad)
            {
                pp->utmp = up;
                pp->type = 2;
                pfriend_num++;
                pp++;
            }
            else if (!ispal && (tmp == 1) && !filter && !bad)
            {
                pp->utmp = up;
                pp->type = 3;
                ofriend_num++;
                pp++;
            }
            else if (!filter)
            {
                pp->utmp = up;
                pp->type = 4;
                nf_num++;
                pp++;
            }
        }
    } while (++up <= uceil);
#else
    do
    {
        userno = up->userno;
        if (userno <= 0)
            continue;
        if ((userno == self) || ((seecloak || !(up->ufo & UFO_CLOAK))&&(can_see(up)!=2 || HAS_PERM(PERM_SYSOP)) && (!filter || is_pal(userno))))
        {
            *pp++ = up;
        }
    } while (++up <= uceil);

#endif  /* #ifdef  FRIEND_FIRST */

    xo->max = max = pp - ulist_pool;

    if (xo->pos >= max)
        xo->pos = xo->top = 0;

    if ((max > 1) && (pickup_way))
    {
        xsort(ulist_pool, max, sizeof(PICKUP), ulist_cmp[pickup_way - 1]);
    }
    total_num = max;

/* cache.101023: shm�z���y���H�ƶñ��᪺�۰ʭץ� */
#ifdef AUTO_FIX_INFO
    ushm->count = total_num;

    if (currbno >= 0)
        bshm->mantime[currbno] = board_pals;    /* �̫�ݪ����ӪO�H�Ƨ�s */
#endif

    return ulist_head(xo);
}



static int
ulist_neck(
    XO *xo)
{
    move(1, 0);
#ifdef HAVE_BOARD_PAL
    prints("  �ƦC�覡�G[\x1b[1m%s\x1b[m] �W���H�ơG%d %s�ڪ��B�͡G%d %s�P�ڬ��͡G%d %s�a�H�G%d \x1b[0;36m�O�͡G%d\x1b[m",
        msg_pickup_way[pickup_way], total_num, COLOR_PAL, friend_num+pfriend_num, COLOR_OPAL, friend_num+ofriend_num, COLOR_BAD, bfriend_num, board_pals);
    prints(NECK_ULIST,
        (d_cols >> 1) + 22, (HAS_PERM(PERM_SYSOP) && (cuser.ufo2 & UFO2_REALNAME)) ? "�u��m�W" : "��  ��",
        ((d_cols+1) >> 1) + 13, (cuser.ufo2 & UFO2_SHIP) ? "�n�ʹy�z" :"�G�m", "�ʺA");
#else
    prints("  �ƦC�覡�G[\x1b[1m%s\x1b[m] �W���H�ơG%d %s�ڪ��B�͡G%d %s�P�ڬ��͡G%d %s�a�H�G%d\x1b[m",
        msg_pickup_way[pickup_way], total_num, COLOR_PAL, friend_num+pfriend_num, COLOR_OPAL, friend_num+ofriend_num, COLOR_BAD, bfriend_num);
    prints(NECK_ULIST,
        (d_cols >> 1) + 22, (HAS_PERM(PERM_SYSOP) && (cuser.ufo & UFO_REALNAME)) ? "�u��m�W" : "��  ��",
        ((d_cols+1) >> 1) + 13, (cuser.ufo2 & UFO2_SHIP) ? "�n�ʹy�z" :"�G�m", "�ʺA");
#endif

    return ulist_body(xo);
}


static int
ulist_head(
    XO *xo)
{
    vs_head((cuser.ufo2 & UFO2_PAL)?"�n�ͦC��":"���ͦC��", str_site);
    return ulist_neck(xo);
}


static int
ulist_toggle(
    XO *xo)
{
    int ans, max;
    ans = pickup_way + 1;
    ans %= PICKUP_WAYS;

    pickup_way = ans;
    max = xo->max;
    if (max <= 1)
        return XO_FOOT;
    return XO_INIT;
}


static int
ulist_pal(
    XO *xo)
{
    cuser.ufo2 ^= UFO2_PAL;
    /* Thor.980805: �`�N ufo �P�B���D */
    return XO_INIT;
}


static int
ulist_search(
    XO *xo,
    int step)
{
    int num, pos, max;
    PICKUP *pp;
    static char buf[IDLEN + 1];

    if (vget(B_LINES_REF, 0, msg_uid, buf, IDLEN + 1, GCARRY))
    {
        GCC_UNUSED int buflen;
        char bufl[IDLEN + 1];

        str_lower(bufl, buf);
        buflen = strlen(bufl); /* Thor: ���w�j��0 */

        pos = num = xo->pos;
        max = xo->max;
        pp = ulist_pool;
        do
        {
            pos += step;
            if (pos < 0) /* Thor.990124: ���] max ����0 */
                pos = max - 1;
            else if (pos >= max)
                pos = 0;

            /* Thor.990124: id �h�q�Y match */
            /* if (str_ncasecmp(pp[pos]->userid, bufl, buflen)==0 */

            if (str_casestr(pp[pos].utmp->userid, bufl) /* lkchu.990127: �䳡�� id �n������n�� :p */
            || str_casestr(pp[pos].utmp->username, bufl)) /* Thor.990124: �i�H�� ���� nickname */
            {
                move(b_lines, 0);
                clrtoeol();
                return XR_FOOT + XO_MOVE + pos;
            }

        } while (pos != num);
    }

    return XO_FOOT;
}

static int
ulist_search_forward(
    XO *xo)
{
    return ulist_search(xo, 1); /* step = +1 */
}

static int
ulist_search_backward(
    XO *xo)
{
    return ulist_search(xo, -1); /* step = -1 */
}



static int
ulist_makepal(
    XO *xo)
{
    if (cuser.userlevel)
    {
        UTMP *up;
        int userno;

        up = ulist_pool[xo->pos].utmp;
        userno = up->userno;
        if (userno > 0 && !is_pal(userno) && !is_bad(userno)   /* �|���C�J�n�ͦW�� */
                && (userno != cuser.userno))    /* lkchu.981217: �ۤv���i���n�� */

        {
            PAL pal;
            char buf[80];

            strcpy(buf, up->userid);

            vget(B_LINES_REF, 0, "�n�ʹy�z�G", pal.ship, sizeof(pal.ship), DOECHO);
            pal.ftype = 0;
            pal.userno = userno;
            strcpy(pal.userid, buf);
            usr_fpath(buf, cuser.userid, FN_PAL);
            if (rec_num(buf, sizeof(PAL)) < PAL_MAX)
            {
                rec_add(buf, &pal, sizeof(PAL));
                pal_cache();
                return XO_INIT;
            }
            else
            {
                vmsg("�z���n�ͦW��Ӧh�A�е��[��z");
            }
        }
    }
    return XO_FOOT;
}

static int
ulist_makebad(
    XO *xo)
{
    if (cuser.userlevel)
    {
        UTMP *up;
        int userno;

        up = ulist_pool[xo->pos].utmp;
        userno = up->userno;
        if (userno > 0 && !is_pal(userno) && !is_bad(userno)  /* �|���C�J�n�ͦW�� */
                && (userno != cuser.userno))    /* lkchu.981217: �ۤv���i���n�� */

        {
            PAL pal;
            char buf[80];

            strcpy(buf, up->userid);

            vget(B_LINES_REF, 0, "�c��c���G", pal.ship, sizeof(pal.ship), DOECHO);
            pal.ftype = PAL_BAD;
            pal.userno = userno;
            strcpy(pal.userid, buf);
            usr_fpath(buf, cuser.userid, FN_PAL);
            if (rec_num(buf, sizeof(PAL)) < PAL_MAX)
            {
                rec_add(buf, &pal, sizeof(PAL));
                pal_cache();
                return XO_INIT;
            }
            else
                vmsg("�z���n�ͦW��Ӧh�A�е��[��z");
        }
    }
    return XO_NONE;
}


static int
ulist_mail(
    XO *xo)
{
    char userid[IDLEN + 1];

    /* Thor.981022:�����S���v���H�H */
    if (!HAS_PERM(PERM_INTERNET) || HAS_PERM(PERM_DENYMAIL) || !cuser.userlevel)
        return XO_NONE;

    strcpy(userid, ulist_pool[xo->pos].utmp->userid);
    if (*userid)
    {
        vs_bar("�H  �H");
        prints("���H�H�G%s", userid);
        my_send(userid);
        return XO_INIT;
    }

    vmsg(MSG_USR_LEFT);
    return XO_FOOT;
}


static int
ulist_query(
    XO *xo)
{
    move(1, 0);
    clrtobot();
    my_query(ulist_pool[xo->pos].utmp->userid, 0);
    /*return XO_NECK;*/
    return XO_INIT;
}


static int
ulist_broadcast(
    XO *xo)
{
    int num;
    PICKUP *pp;
    UTMP *up;
    BMW bmw;
    char buf[80], ans, admin;

    num = cuser.userlevel;
    if (!(num & (PERM_SYSOP)) &&
        (!(num & PERM_PAGE) || !(cuser.ufo2 & UFO2_PAL)))
        return XO_NONE;

    num = xo->max;
    if (num < 1)
        return XO_NONE;

    bmw.caller = 0;
    bmw_edit(NULL, "���s���G", &bmw, 0);
    sprintf(buf, "���s���G%s", bmw.msg);
    strcpy(bmw.msg, buf);
    admin = check_admin(cuser.userid);
    if (admin && !(cuser.ufo2 & UFO2_PAL))
    {
        if ((ans = vans("�� �ϥ� SYSOP �s���ܡH [y/N]")) != 'Y' && ans != 'y')
            admin = 0;
        if ((ans = vans("�� �T�w�����s���ܡH [y/N]")) != 'Y' && ans != 'y')
            return XO_INIT;
    }
    if (!(cuser.ufo2 & UFO2_PAL) && admin)
    {
        strcpy(bmw.userid, "SYSOP");
        /*bmw.sender = 1;*/
    }
    if (bmw.caller)
    {
        pp = ulist_pool;
        while (--num >= 0)
        {
            up = pp[num].utmp;
            if (can_message(up) && (!(up->ufo & UFO_BROADCAST)||
                (HAS_PERM(PERM_SYSOP|PERM_CHATROOM) && !(cuser.ufo2 & UFO2_PAL))))
            {
                bmw.recver = up->userno;
                bmw_send(up, &bmw);
            }
        }
    }
    return XO_INIT;
}


static int
ulist_talk(
    XO *xo)
{
    if (HAS_PERM(PERM_PAGE))
    {
        UTMP *up;

        up = ulist_pool[xo->pos].utmp;
        if (can_override(up))
            return talk_page(up) ? XO_INIT : XO_FOOT;
    }
    return XO_NONE;
}


static int
ulist_write(
    XO *xo)
{
    if (HAS_PERM(PERM_PAGE))
    {
        UTMP *up;

        up = ulist_pool[xo->pos].utmp;
        if (can_message(up))
        {
            BMW bmw;
            char buf[20];

#ifdef  HAVE_SHOWNUMMSG
            if (up->num_msg > 9 && (up->ufo & UFO_MAXMSG) && !HAS_PERM(PERM_SYSOP))
            {
                vmsg("���w�g�Q���y���z�F!!");
                return XO_INIT;
            }
#endif

            sprintf(buf, "��[%s]", up->userid);
            bmw_edit(up, buf, &bmw, 0);
        }
#ifdef  HAVE_BANMSG
        else if (!(up->ufo & UFO_MESSAGE) && can_banmsg(up))
        {
            vmsg("��褣�Qť��z���n��!!");
        }
#endif
        return XO_INIT;
    }
    return XO_NONE;
}


static int
ulist_edit(                     /* Thor: �i�u�W�d�ݤέק�ϥΪ� */
    XO *xo)
{
    ACCT acct;

    if (!HAS_PERM(PERM_SYSOP) ||
        acct_load(&acct, ulist_pool[xo->pos].utmp->userid) < 0)
        return XO_NONE;

    vs_bar("�ϥΪ̳]�w");
    acct_setup(&acct, 1);
    return XO_LOAD;
}

/* BLACK SU */
static int
ulist_su(
    XO *xo)
{
    XO *tmp;
    ACCT acct;
    char path[80];
    int level, ufo;
    ufo = cuser.ufo;
    level = cuser.userlevel;
    if (!supervisor ||
        acct_load(&acct, ulist_pool[xo->pos].utmp->userid) < 0)
        return XO_NONE;

    memcpy(&cuser, &acct, sizeof(ACCT));
    cuser.userlevel = level;
    cuser.ufo = ufo;
    usr_fpath(path, acct.userid, FN_DIR);
    // IID.20190507: Xover object for mailbox (`cmbox`) is statically allocated.
/*
    tmp = xz[XZ_MBOX - XO_ZONE].xo;
    xz[XZ_MBOX - XO_ZONE].xo =  xo_new(path);
    xz[XZ_MBOX - XO_ZONE].xo->cb = mbox_cb;
    xz[XZ_MBOX - XO_ZONE].xo->recsiz = sizeof(HDR);
    xz[XZ_MBOX - XO_ZONE].xo->pos = 0;
    free(tmp);
*/
    // IID.20190508: Use `mbox_main()` to update `cmbox`.
    mbox_main();
    usr_fpath(path, acct.userid, FN_BMW);
    tmp = xz[XZ_BMW - XO_ZONE].xo;
    xz[XZ_BMW - XO_ZONE].xo =  xo_new(path);
    xz[XZ_BMW - XO_ZONE].xo->cb = bmw_cb;
    xz[XZ_BMW - XO_ZONE].xo->recsiz = sizeof(BMW);
    xz[XZ_BMW - XO_ZONE].xo->pos = 0;
    free(tmp);
    pal_cache();
    return XO_INIT;
}
/* BLACK SU */

static int
ulist_kick(
    XO *xo)
{
    ACCT u;
    acct_load(&u, ulist_pool[xo->pos].utmp->userid);
    if ((HAS_PERM(PERM_SYSOP)&& (!(u.userlevel & PERM_SYSOP) || !strcmp(cuser.userid, u.userid)))||check_admin(cuser.userid))
    {
        UTMP *up;
        pid_t pid;
        char buf[80];

        up = ulist_pool[xo->pos].utmp;
        if ((pid = up->pid))
        {
            if (vans(msg_sure_ny) != 'y' || pid != up->pid)
                return XO_FOOT;

            sprintf(buf, "%s (%s)", up->userid, up->username);

            if ((kill(pid, SIGTERM) == -1) && (errno == ESRCH))
            /*  memset(up, 0, sizeof(UTMP)); */
            {
                up->pid = up -> userno = 0;
                ushm->count--;            /*r2.20170803: try to fix counting error*/
            }
            else
                sleep(3);               /* �Q�𪺤H�o�ɭԥ��b�ۧڤF�_ */
            blog("KICK", buf);
            return XO_INIT;
        }
        else
        {
            if (vans(msg_sure_ny) != 'y')
                return XO_FOOT;
            memset(up, 0, sizeof(UTMP));
            return XO_INIT;
        }
    }
    return XO_NONE;
}


#ifdef  HAVE_CHANGE_FROM
static int
ulist_fromchange(
    XO *xo)
{
    char *str, buf[34];

    if (!HAS_PERM(PERM_ADMIN))
        return XO_NONE;

    strcpy(buf, str = cutmp->from);
    vget(B_LINES_REF, 0, "�п�J�s���G�m�G", buf, sizeof(cutmp->from), GCARRY);
    if (strcmp(buf, str))
    {
        strcpy(str, buf);
        strcpy(cutmp->from, buf);
        return XO_INIT;
        /*return XO_LOAD;*/
    }

    return XO_FOOT;
}
#endif


static int
ulist_nickchange(
    XO *xo)
{
    char *str, buf[24];

    if (!HAS_PERM(PERM_VALID) || (HAS_PERM(PERM_DENYNICK)&&!HAS_PERM(PERM_SYSOP)))
        return XO_NONE;

    strcpy(buf, str = cuser.username);
    vget(B_LINES_REF, 0, "�п�J�s���ʺ١G", buf, sizeof(cuser.username), GCARRY);

    if (strcmp(buf, str) && str_len_nospace(buf) > 0)
    {
        strcpy(str, buf);
        strcpy(cutmp->username, buf);
        return XO_INIT;
        /*return XO_BODY;*/
    }
    return XO_FOOT;
}


static int
ulist_help(
    XO *xo)
{
    film_out(FILM_ULIST, -1);
    return XO_INIT;
}


static int
ulist_pager(
    XO *xo)
{
    if (!HAS_PERM(PERM_PAGE))
        return XO_NONE;
    if ((cuser.ufo & UFO_PAGER) && (cuser.ufo & UFO_PAGER1))
    {
        cuser.ufo ^= UFO_PAGER;
        cutmp->ufo ^= UFO_PAGER;
        cuser.ufo ^= UFO_PAGER1;
        cutmp->ufo ^= UFO_PAGER1;
    }
    else if (cuser.ufo & UFO_PAGER)
    {
        cuser.ufo ^= UFO_PAGER1;
        cutmp->ufo ^= UFO_PAGER1;
    }
    else
    {
        cuser.ufo ^= UFO_PAGER;
        cutmp->ufo ^= UFO_PAGER;
    }
    return XO_INIT;
}

static int
ulist_message(
    XO *xo)
{

    if (!HAS_PERM(PERM_PAGE))
        return XO_NONE;
    if ((cuser.ufo & UFO_QUIET) && (cuser.ufo & UFO_MESSAGE))
    {
        cuser.ufo ^= UFO_QUIET;
        cutmp->ufo ^= UFO_QUIET;
        cuser.ufo ^= UFO_MESSAGE;
        cutmp->ufo ^= UFO_MESSAGE;
    }
    else if (cuser.ufo & UFO_QUIET)
    {
        cuser.ufo ^= UFO_MESSAGE;
        cutmp->ufo ^= UFO_MESSAGE;
    }
    else
    {
        cuser.ufo ^= UFO_QUIET;
        cutmp->ufo ^= UFO_QUIET;
    }
    return XO_INIT;
}

static int
ulist_recall(
    XO *xo)
{
    t_recall();
    return XO_INIT;
}

static int
ulist_realname(
    XO *xo)
{
    if (HAS_PERM(PERM_SYSOP))
    {
        cuser.ufo2 ^= UFO2_REALNAME;
//      cutmp->ufo ^= UFO_REALNAME;
    }
    return XO_INIT;
}

static int
ulist_ship(
    XO *xo)
{
    cuser.ufo2 ^= UFO2_SHIP;
//  cutmp->ufo ^= UFO_SHIP;
    return XO_INIT;
}

static int
ulist_mp(
    XO *xo)
{
    GCC_UNUSED int tmp;

    if (!HAS_PERM(PERM_VALID))
        return XO_NONE;
    tmp = t_pal();
    return XO_INIT;
}

static int
ulist_readmail(
    XO *xo)
{
    if (cuser.userlevel)
    {
        if (HAS_PERM(PERM_DENYMAIL))
            vmsg("�z���H�c�Q��F�I");
        else
            xover(XZ_MBOX);
        return XO_INIT;
    }
    else
        return XO_NONE;
}

static int
ulist_del(
    XO *xo)
{
    UTMP *up;
    char ans;
    int userno;
    int  fd, tmp=0;
    char fpath[64];
    PAL *pal;

    ans = vans("�O�_�R��(y/N)�G");
    if (ans == 'y' || ans == 'Y')
    {
        up = ulist_pool[xo->pos].utmp;
        userno = up->userno;

        usr_fpath(fpath, cuser.userid, FN_PAL);
        if ((fd = open(fpath, O_RDONLY)) >= 0)
        {
            mgets(-1);
            while ((pal = (PAL *) mread(fd, sizeof(PAL))))
            {
                if (pal->userno!=userno)
                    tmp = tmp + 1;
                else
                {
                    rec_del(fpath, sizeof(PAL), tmp, NULL, NULL);
                    break;
                }
            }
            close(fd);
        }
        pal_cache();
        return XO_INIT;
    }
    else
        return XO_FOOT;
}

static int
ulist_changeship(
    XO *xo)
{
    UTMP *up;
    int userno;
    int  fd, tmp=0;
    char fpath[64], buf[46];
    PAL *pal;
    int check;

    up = ulist_pool[xo->pos].utmp;
    userno = up->userno;


    check = is_pal(userno) ? 1 : is_bad(userno) ? 2 : 0;
    if (!check)
        return XO_NONE;
    strcpy(buf, "");
    copyship(buf, userno);

    if (vget(B_LINES_REF, 0, check == 1 ?"�ͽˡG":"�c��c���G", buf, sizeof(buf), GCARRY))
    {
        usr_fpath(fpath, cuser.userid, FN_PAL);
        if ((fd = open(fpath, O_RDONLY)) >= 0)
        {
            mgets(-1);
            while ((pal = (PAL *) mread(fd, sizeof(PAL))))
            {
                if (pal->userno!=userno)
                    tmp = tmp + 1;
                else
                {
                    strcpy(pal->ship, buf);
                    rec_put(fpath, pal, sizeof(PAL), tmp);
                    break;
                }
            }
            close(fd);
        }
        pal_cache();
        return XO_INIT;
    }
    else
        return XO_HEAD;
}

#if 1
static int
ulist_state(
    XO *xo)
{
    char buf[128];
    if (!HAS_PERM(PERM_SYSOP))
        return XO_NONE;
    sprintf(buf, "PID : %d", ulist_pool[xo->pos].utmp->pid);
    vmsg(buf);
    return XO_INIT;
}
#endif  /* #if 1 */

#ifdef  APRIL_FIRST
static int
ulist_april1(
    XO *xo)
{
    char buf[256];
    more(FN_APRIL_FIRST, NULL);
    sprintf(buf, "�z�O�� %d �ӳQ�F���ϥΪ� ^^y ", ushm->avgload);
    if (!aprilfirst)
        ushm->avgload++;
    vmsg(buf);
    aprilfirst = 1;
    return XO_INIT;
}
#endif

KeyFuncList ulist_cb =
{
    {XO_INIT, {ulist_init}},
    {XO_LOAD, {ulist_body}},
    {XO_HEAD, {ulist_head}},
    {XO_BODY, {ulist_body}},
#if 1
    {'S', {ulist_state}},
#endif
    {'y', {ulist_readmail}},
/* BLACK SU */
    {'u', {ulist_su}},
/* BLACK SU */
    {'m', {ulist_message}},
    {'Z', {ulist_ship}},
    {'f', {ulist_pal}},
    {'a', {ulist_makepal}},
    {'A', {ulist_makebad}},
    {'t', {ulist_talk}},
    {'w', {ulist_write}},
    {'l', {ulist_recall}},                /* Thor: ���T�^�U */
    {'j', {ulist_changeship}},
    {'q', {ulist_query}},
    {'b', {ulist_broadcast}},
    {'s', {xo_cb_init}},          /* refresh status Thor: ��user�n�D */
    {'c', {t_cloak}},
    {'R', {ulist_realname}},
    {'o', {ulist_mp}},
    {'d', {ulist_del}},
    {'p', {ulist_pager}},
    {Ctrl('Q'), {ulist_query}},
    {Ctrl('K'), {ulist_kick}},
    {Ctrl('X'), {ulist_edit}},
    {'g', {ulist_nickchange}},
#ifdef HAVE_CHANGE_FROM
    {Ctrl('F'), {ulist_fromchange}},
#endif

    /* Thor.990125: �i�e��j�M, id or nickname */
    {'/', {ulist_search_forward}},
    {'?', {ulist_search_backward}},

#ifdef  APRIL_FIRST
    {'X', {ulist_april1}},
#endif

    {'M', {ulist_mail}},
    {KEY_TAB, {ulist_toggle}},
    {'h', {ulist_help}}
};


/* ----------------------------------------------------- */
/* talk main-routines                                    */
/* ----------------------------------------------------- */

int
t_message(void)
{
    if ((cuser.ufo & UFO_QUIET) && (cuser.ufo & UFO_MESSAGE))
    {
        cuser.ufo ^= UFO_QUIET;
        cutmp->ufo ^= UFO_QUIET;
        cuser.ufo ^= UFO_MESSAGE;
        cutmp->ufo ^= UFO_MESSAGE;
    }
    else if (cuser.ufo & UFO_QUIET)
    {
        cuser.ufo ^= UFO_MESSAGE;
        cutmp->ufo ^= UFO_MESSAGE;
    }
    else
    {
        cuser.ufo ^= UFO_QUIET;
        cutmp->ufo ^= UFO_QUIET;
    }
    return 0;
}


int
t_pager(void)
{
    /* cuser.ufo = (cutmp->ufo ^= UFO_PAGER); */
    if ((cuser.ufo & UFO_PAGER) && (cuser.ufo & UFO_PAGER1))
    {
        cuser.ufo ^= UFO_PAGER;
        cutmp->ufo ^= UFO_PAGER;
        cuser.ufo ^= UFO_PAGER1;
        cutmp->ufo ^= UFO_PAGER1;
    }
    else if (cuser.ufo & UFO_PAGER)
    {
        cuser.ufo ^= UFO_PAGER1;
        cutmp->ufo ^= UFO_PAGER1;
    }
    else
    {
        cuser.ufo ^= UFO_PAGER;
        cutmp->ufo ^= UFO_PAGER;
    }
    return 0;
}

int
t_cloak(
    XO *xo)
{
    if (HAS_PERM(PERM_CLOAK))
    {
        cuser.ufo ^= UFO_CLOAK;
        cutmp->ufo ^= UFO_CLOAK;
    } /* Thor.980805: �ѨM ufo���P�B���D */
    return XO_INIT;
}




int
t_query(void)
{
    ACCT acct;

    vs_bar("�d�ߺ���");
    if (acct_get(msg_uid, &acct) > 0)
    {
        move(2, 0);
        clrtobot();
        do_query(&acct, 0);
    }

    return 0;
}


/* ------------------------------------- */
/* ���H�Ӧ���l�F�A�^���I�s��            */
/* ------------------------------------- */

void
talk_rqst(void)
{
    UTMP *up;
    int mode, sock, ans, len, port;
    char buf[80];
    screen_backup_t old_screen;
    struct addrinfo hints = {0};
    struct addrinfo *hs;

    up = cutmp->talker;
    if (!up)
        return;
    up->talker = cutmp;

    port = up->sockport;
    if (!port)
        return;

    mode = bbsmode;
#ifdef  HAVE_PIP_FIGHT
    if (up->mode == M_CHICKEN)
        utmp_mode(M_CHICKEN);
    else
#endif
        utmp_mode(M_TRQST);


    scr_dump(&old_screen);

    clear();
    sprintf(page_requestor, "%s (%s)", up->userid, up->username);

#ifdef EVERY_Z
    /* Thor.0725: �� talk & chat �i�� ^z �@�ǳ� */

    if (holdon_fd)
    {
        sprintf(buf, "%s �Q�M�z��A���L�z�u���@�i�L", page_requestor);
        vmsg(buf);
        buf[0] = ans = '6';             /* Thor.0725:�u���@�i�L */
        len = 1;
        goto over_for;
    }
#endif

    bell();
#ifdef  HAVE_PIP_FIGHT
    if (up->mode != M_CHICKEN)
#endif
    {
        prints("�z�Q�� %s ��ѶܡH(�Ӧ� %s)", page_requestor, up->from);
        for (;;)
        {
            ans = vget(1, 0, "==> Yes, No, Query�H[Y] ", buf, 3, LCECHO);
            if (ans == 'q')
            {
                my_query(up->userid, 0);
            }
            else
                break;
        }
    }
#ifdef  HAVE_PIP_FIGHT
    else
    {
        prints("�z�Q�� %s PK �p���ܡH(�Ӧ� %s)", page_requestor, up->from);
        ans = vget(1, 0, "==> Yes, No�H[Y] ", buf, 3, LCECHO);
    }
#endif

    len = 1;

    if (ans == 'n')
    {
        move(2, 0);
        clrtobot();
#ifdef  HAVE_PIP_FIGHT
        if (up->mode != M_CHICKEN)
#endif
        {
            for (ans = 0; ans < 5; ans++)
                prints("\n (%d) %s", ans + 1, talk_reason[ans]);
            ans = vget(10, 0, "�п�J�ﶵ�Ψ�L���� [1]�G\n==> ",
                buf + 1, sizeof(buf) - 1, DOECHO);

            if (ans == 0)
                ans = '1';

            if (ans >= '1' && ans <= '5')
            {
                buf[0] = ans;
            }
            else
            {
                buf[0] = ans = ' ';
                len = strlen(buf);
            }
        }
#ifdef  HAVE_PIP_FIGHT
        else
            buf[0] = ans = 'C';
#endif
    }
    else
    {
#ifdef  HAVE_PIP_FIGHT
        if (up->mode != M_CHICKEN)
#endif
            buf[0] = ans = 'y';
#ifdef  HAVE_PIP_FIGHT
        else
            buf[0] = ans = 'c';
#endif
    }

#ifdef EVERY_Z
over_for:
#endif

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_ADDRCONFIG | AI_NUMERICSERV;
    {
        char port_str[12];
        sprintf(port_str, "%d", port);
        if (getaddrinfo(NULL, port_str, &hints, &hs))
            return;
    }

    for (struct addrinfo *h = hs; h; h = h->ai_next)
    {
        sock = socket(h->ai_family, h->ai_socktype, 0);
        if (sock < 0)
            continue;

        if (!connect(sock, h->ai_addr, h->ai_addrlen))
        {
            send(sock, buf, len, 0);

            if (ans == 'y')
            {
                strcpy(cutmp->mateid, up->userid);
                talk_speak(sock);
            }
#ifdef  HAVE_PIP_FIGHT
            else if (ans == 'c')
            {
                DL_HOTSWAP_SCOPE int (*p)(int, int) = NULL;
                if (!p)
                    p = DL_NAME_GET("pip.so", pip_vf_fight);
                strcpy(cutmp->mateid, up->userid);
                if (p)
                {
                    cutmp->pip = NULL;
                    (*p)(sock, 1);
                    cutmp->pip = NULL;

                }
                add_io(0, 60);
/*              pip_vf_fight(sock, 2);*/
            }
#endif
            close(sock);
            break;
        }
        close(sock);
    }
    freeaddrinfo(hs);

#ifdef  LOG_TALK
    if (ans == 'y' && up->mode != M_CHICKEN) /* mat.991011: ����Talk�ڵ��ɡA���Ͳ�ѰO����record */
        talk_save();          /* lkchu.981201: talk �O���B�z */
#endif
    scr_restore_free(&old_screen);
    utmp_mode(mode);
}


void
talk_main(void)
{
    char fpath[64];

    xz[XZ_ULIST - XO_ZONE].xo = &ulist_xo;
    ulist_xo.cb = ulist_cb;
    ulist_xo.recsiz = sizeof(UTMP);

    /* lkchu.981230: �Q�� xover ��X bmw */
    usr_fpath(fpath, cuser.userid, FN_BMW);
    free(xz[XZ_BMW - XO_ZONE].xo);
    xz[XZ_BMW - XO_ZONE].xo = xo_new(fpath);
    xz[XZ_BMW - XO_ZONE].xo->cb = bmw_cb;
    xz[XZ_BMW - XO_ZONE].xo->recsiz = sizeof(BMW);
    xz[XZ_BMW - XO_ZONE].xo->pos = 0;
}

int
check_personal_note(
    int newflag,
    const char *userid)
{
    char fpath[256];
    int  fd, total = 0;
    notedata myitem;
    const char *fn_note_dat = FN_PNOTE_DAT;

    if (userid == NULL)
        usr_fpath(fpath, cuser.userid, fn_note_dat);
    else
        usr_fpath(fpath, userid, fn_note_dat);

    if ((fd = open(fpath, O_RDONLY)) >=0)
    {
        while (read(fd, &myitem, sizeof(myitem)) == sizeof(myitem))
        {
            if (newflag)
            {
                if (myitem.mode == 0) total++;
            }
            else
                total++;
        }
        close(fd);
        return total;
    }
    return 0;
}

#ifdef  HAVE_BANMSG
void
banmsg_cache(void)
{
    int count, fsize, fd;
    int *plist, *cache;
    BANMSG *phead, *ptail;
    char *fimage, fpath[64];
    UTMP *up;

    up = cutmp;
    cutmp->userno = cuser.userno;

    cache = NULL;
    count = 0;

    fsize = 0;
    usr_fpath(fpath, cuser.userid, FN_BANMSG);
    fimage = f_img(fpath, &fsize);
    if ((fsize > (BANMSG_MAX * sizeof(BANMSG))) && (fd = open(fpath, O_RDWR)) >= 0)
    {
        ftruncate(fd, BANMSG_MAX * sizeof(BANMSG));
        close(fd);
    }
    if (fimage != NULL)
    {
        if (fsize > (BANMSG_MAX * sizeof(BANMSG)))
        {
            fsize = BANMSG_MAX * sizeof(BANMSG);
        }

        count = fsize / sizeof(BANMSG);
        if (count)
        {
            cache = plist = up->banmsg_spool;
            phead = (BANMSG *) fimage;
            ptail = (BANMSG *) (fimage + fsize);
            do
            {
                *plist++ = phead->userno;
            } while (++phead < ptail);

            if (count > 0)
            {
                if (count > 1)
                    xsort(cache, count, sizeof(int), int_cmp);
            }
        }
    }

    up->banmsg_max = count;

    free(fimage);
}


void
banmsg_sync(
    const char *fpath)
{
    int fd, size=0;
    struct stat st;
    char buf[64];

    if (!fpath)
    {
        usr_fpath(buf, cuser.userid, FN_BANMSG);
        fpath = buf;
    }

    if ((fd = open(fpath, O_RDWR, 0600)) < 0)
        return;

    outz("�� ��ƾ�z�]�֤��A�еy�� \x1b[5m...\x1b[m");
    refresh();

    if (!fstat(fd, &st) && (size = st.st_size) > 0)
    {
        BANMSG *pbase, *phead, *ptail;
        int userno;

        pbase = phead = (BANMSG *) malloc(size);
        size = read(fd, pbase, size);
        if (size >= sizeof(BANMSG))
        {
            ptail = (BANMSG *) ((char *) pbase + size);
            while (phead < ptail)
            {
                userno = phead->userno;
                if (userno > 0 && userno == acct_userno(phead->userid))
                {
                    phead++;
                    continue;
                }

                ptail--;
                if (phead >= ptail)
                    break;
                memcpy(phead, ptail, sizeof(BANMSG));
            }

            size = (char *) ptail - (char *) pbase;
            if (size > 0)
            {
                if (size > sizeof(BANMSG))
                {
                    xsort(pbase, size / sizeof(BANMSG), sizeof(BANMSG), (int (*)(const void *lhs, const void *rhs))str_casecmp);
                }

                lseek(fd, 0, SEEK_SET);
                write(fd, pbase, size);
                ftruncate(fd, size);
            }
        }
        free(pbase);
    }
    close(fd);

    if (size <= 0)
        unlink(fpath);
}


/* ----------------------------------------------------- */
/* �ڦ��T���W��G��榡�ާ@�ɭ��y�z                      */
/* ----------------------------------------------------- */


static int banmsg_add(XO *xo);


static void
banmsg_item(
    int num,
    const BANMSG *banmsg)
{
    prints("%6d    %-14s%s\n", num, banmsg->userid, banmsg->ship);
}

static int
banmsg_cur(
    XO *xo)
{
    const BANMSG *const banmsg = (const BANMSG *) xo_pool_base + xo->pos;
    move(3 + xo->pos - xo->top, 0);
    banmsg_item(xo->pos + 1, banmsg);
    return XO_NONE;
}


static int
banmsg_body(
    XO *xo)
{
    const BANMSG *banmsg;
    int num, max, tail;

    move(3, 0);
    clrtobot();
    max = xo->max;
    if (max <= 0)
    {
        if (vans("�n�s�W��(y/N)�H[N] ") == 'y')
            return banmsg_add(xo);
        return XO_QUIT;
    }

    num = xo->top;
    banmsg = (const BANMSG *) xo_pool_base + num;
    tail = num + XO_TALL;
    if (max > tail)
        max = tail;

    do
    {
        banmsg_item(++num, banmsg++);
    } while (num < max);

    return XO_NONE;
}


static int
banmsg_head(
    XO *xo)
{
    vs_head("�ڦ��W��", str_site);
    prints(NECK_BANMSG, d_cols, "");
    return banmsg_body(xo);
}


static int
banmsg_load(
    XO *xo)
{
    xo_load(xo, sizeof(BANMSG));
    return banmsg_body(xo);
}


static int
banmsg_init(
    XO *xo)
{
    xo_load(xo, sizeof(BANMSG));
    return banmsg_head(xo);
}


static void
banmsg_edit(
    BANMSG *banmsg,
    int echo)
{
    if (echo == DOECHO)
        memset(banmsg, 0, sizeof(BANMSG));
    vget(B_LINES_REF, 0, "�y�z�G", banmsg->ship, sizeof(banmsg->ship), echo);
}


static int
banmsg_add(
    XO *xo)
{
    ACCT acct;
    int userno;

    if (xo->max >= BANMSG_MAX)
    {
        vmsg("�z���ڦ��T���W��Ӧh�A�е��[��z");
        return XO_FOOT;
    }

    userno = acct_get(msg_uid, &acct);

#if 1                           /* Thor.0709: �����Х[�J */
    if ((xo->dir[0] == 'u') && is_banmsg(userno))
    {
        vmsg("�W�椤�w�����H");
        return XO_FOOT;
    }
    else if (userno == cuser.userno)
    {
        vmsg("�ۤv�����[�J�ڦ��T���W�椤");
        return XO_FOOT;
    }
#endif

    if (userno > 0)
    {
        BANMSG banmsg;

        banmsg_edit(&banmsg, DOECHO);
        strcpy(banmsg.userid, acct.userid);
        banmsg.userno = userno;
        rec_add(xo->dir, &banmsg, sizeof(BANMSG));
        xo->pos = XO_TAIL;
        xo_load(xo, sizeof(BANMSG));
    }

    banmsg_cache();

    return XO_HEAD;
}


static int
banmsg_delete(
    XO *xo)
{
    if (vans(msg_del_ny) == 'y')
    {

        if (!rec_del(xo->dir, sizeof(BANMSG), xo->pos, NULL, NULL))
        {

            banmsg_cache();
            return XO_LOAD;
        }
    }
    return XO_FOOT;
}


static int
banmsg_change(
    XO *xo)
{
    BANMSG *banmsg, mate;
    int pos;

    pos = xo->pos;
    banmsg = (BANMSG *) xo_pool_base + pos;

    mate = *banmsg;
    banmsg_edit(banmsg, GCARRY);
    if (memcmp(banmsg, &mate, sizeof(BANMSG)))
    {
        rec_put(xo->dir, banmsg, sizeof(BANMSG), pos);
        return XR_FOOT + XO_CUR;
    }

    return XO_FOOT;
}


static int
banmsg_mail(
    XO *xo)
{
    BANMSG *banmsg;
    char *userid;

    banmsg = (BANMSG *) xo_pool_base + xo->pos;
    userid = banmsg->userid;
    if (*userid)
    {
        vs_bar("�H  �H");
        prints("���H�H�G%s", userid);
        my_send(userid);
    }
    return XO_HEAD;
}


static int
banmsg_sort(
    XO *xo)
{
    banmsg_sync(xo->dir);
    return XO_LOAD;
}


static int
banmsg_query(
    XO *xo)
{
    const BANMSG *banmsg;

    banmsg = (const BANMSG *) xo_pool_base + xo->pos;
    move(1, 0);
    clrtobot();
    my_query(banmsg->userid, 1);
    return XO_HEAD;
}


static int
banmsg_help(
    XO *xo)
{
//  film_out(FILM_BANMSG, -1);
    return XO_HEAD;
}


KeyFuncList banmsg_cb =
{
    {XO_INIT, {banmsg_init}},
    {XO_LOAD, {banmsg_load}},
    {XO_HEAD, {banmsg_head}},
    {XO_BODY, {banmsg_body}},
    {XO_CUR, {banmsg_cur}},

    {'a', {banmsg_add}},
    {'c', {banmsg_change}},
    {'d', {banmsg_delete}},
    {'m', {banmsg_mail}},
    {'q', {banmsg_query}},
    {'s', {banmsg_sort}},
    {'h', {banmsg_help}}
};


int
t_banmsg(void)
{
    XO *xo, *last;
    char fpath[64];

    last = xz[XZ_OTHER - XO_ZONE].xo;  /* record */

    usr_fpath(fpath, cuser.userid, FN_BANMSG);
    xz[XZ_OTHER - XO_ZONE].xo = xo = xo_new(fpath);
    xo->cb = banmsg_cb;
    xo->recsiz = sizeof(BANMSG);
    xo->pos = 0;
    xover(XZ_OTHER);
    banmsg_cache();
    free(xo);

    xz[XZ_OTHER - XO_ZONE].xo = last;  /* restore */

    return 0;
}
#endif  /* #ifdef  HAVE_BANMSG */
