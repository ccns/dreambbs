/*-------------------------------------------------------*/
/* board.c      ( NTHU CS MapleBBS Ver 3.02 )            */
/*-------------------------------------------------------*/
/* target : 看板、群組功能                               */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/

#include "bbs.h"

extern BCACHE *bshm;
extern XZ xz[];

static int *brh_base;           /* allocated memory */
static int *brh_tail;           /* allocated memory */
static int brh_size;            /* allocated memory size */
static time_t brh_expire;

char brd_bits[MAXBOARD];
time_t brd_visit[MAXBOARD];     /* 最近瀏覽時間 */
static char *class_img;

#ifdef  HAVE_PROFESS
static char *profess_img = NULL;
#endif

static XO board_xo;
BRD *xbrd;
int boardmode=0;

static int *
brh_alloc(
    int *tail,
    int size)
{
    int *base, n;

    base = brh_base;
    n = (char *) tail - (char *) base;
    size += n;
    if (size > brh_size)
    {
        /* size = (size & -BRH_PAGE) + BRH_PAGE; */
        size += n >> 4;         /* 多預約一些記憶體 */
        base = (int *) realloc((char *) base, size);

        if (base == NULL)
            abort_bbs();

        brh_base = base;
        brh_size = size;
        tail = (int *) ((char *) base + n);
    }

    return tail;
}


static void
brh_put(void)
{
    int *list;

    /* compact the history list */

    list = brh_tail;

    if (*list)
    {
        int *head, *tail, n, item, chrono;

        n = *++list;   /* Thor.980904: 正讀時是bhno */
        brd_bits[n] |= BRD_H_BIT;
        time((time_t *) list);    /* Thor.980904: 註解: bvisit time */

        item = *++list;
        head = ++list;
        tail = head + item;

        while (head < tail)
        {
            chrono = *head++;
            n = *head++;
            if (n == chrono) /* Thor.980904: 註解: 相同的時候壓起來 */
            {
                n |= BRH_SIGN;
                item--;
            }
            else
            {
                *list++ = chrono;
            }
            *list++ = n;
        }

        list[-item - 1] = item;
        *list = 0;
        brh_tail = list;  /* Thor.980904:新的空brh */
    }
}


void
brh_get(
    time_t bstamp,              /* board stamp */
    int bhno)
{
    int *head, *tail;
    int size, bcnt, item;
    char buf[BRH_WINDOW];

    if (bstamp == *brh_tail) /* Thor.980904:註解:該版已在 brh_tail上 */
        return;

    brh_put();

    bcnt = 0;
    tail = brh_tail;

    if (brd_bits[bhno] & BRD_H_BIT)
    {
        head = brh_base;
        while (head < tail)
        {
            item = head[2];
            size = item * sizeof(time_t) + sizeof(BRH);

            if (bstamp == *head)
            {
                bcnt = item;
                memcpy(buf, head + 3, size - 3 * sizeof(time_t));
                tail = (int *) ((char *) tail - size);
                if ((item = (char *) tail - (char *) head))
                    memmove(head, (char *) head + size, item);
                break;
            }
            head = (int *) ((char *) head + size);
        }
    }

    brh_tail = tail = brh_alloc(tail, BRH_WINDOW);

    *tail++ = bstamp;
    *tail++ = bhno;

    if (bcnt)                   /* expand history list */
    {
        int *list;

        size = bcnt;
        list = tail;
        head = (int *) buf;

        do
        {
            item = *head++;
            if (item & BRH_SIGN)
            {
                item ^= BRH_SIGN;
                *++list = item;
                bcnt++;
            }
            *++list = item;
        } while (--size);
    }
    else if (brd_bits[bhno] & BRD_Z_BIT)
    {
        brh_visit(0);
    }

    *tail = bcnt;
}


GCC_PURE int
brh_unread(
    time_t chrono)
{
    int *head, *tail, item;

    if (chrono <= brh_expire)
        return 0;

    head = brh_tail + 2;
    if ((item = *head) > 0)
    {
        /* check {final, begin} history list */

        head++;
        tail = head + item;
        do
        {
            if (chrono > *head)
                return 1;

            head++;
            if (chrono >= *head)
                return 0;

        } while (++head < tail);
    }
    return 1;
}


void
brh_visit(
    int mode)                   /* 0 : visit, 1: un-visit */
{
    int *list;

    list = (int *) brh_tail + 2;
    *list++ = 2;
    if (mode)
    {
        *list = mode;
    }
    else
    {
        time((time_t *)list);
    }
    *++list = mode;
}

void
brh_add(time_t prev, time_t chrono, time_t next)
{
    int *base, *head, *tail, item, final, begin;

    head = base = brh_tail + 2;
    item = *head++;
    tail = head + item;

    begin = BRH_MASK;

    while (head < tail)
    {
        final = *head;
        if (chrono > final)
        {
            if (prev <= final)
            {
                if (next < begin)       /* increase */
                    *head = chrono;
                else
                {                       /* merge */
                    *base = item - 2;
                    base = head - 1;
                    do
                    {
                        *base++ = *++head;
                    } while (head < tail);
                }
                return;
            }

            break;
        }

        begin = *++head;
        head++;
    }

    if (next >= begin)
    {
        head[-1] = chrono;
        return;
    }

    /* insert or append */

    /* [21, 22, 23] ==> [32, 30] [15, 10] */

    if (item < BRH_MAX)
    {
        /* [32, 30] [22, 22] [15, 10] */

        *base = item + 2;
        tail += 2;
    }
    else
    {
        /* [32, 30] [22, 10] */  /* Thor.980923: how about [6, 7, 8] ? [15, 7]? */

        tail--;

        if (head >= tail)
            return;
    }

    prev = chrono;
    for (;;)
    {
        final = *head;
        *head++ = chrono;

        if (head >= tail)
            return;

        begin = *head;
        *head++ = prev;

        if (head >= tail)
            return;

        chrono = final;
        prev = begin;
    }
}

/* ----------------------------------------------------- */
/* 強迫 user login 時候讀取某公告看板                    */
/* ----------------------------------------------------- */

/* cdchen.bbs@bbs.ntcic.edu.tw add by statue.000725 */
#ifdef  HAVE_FORCE_BOARD
extern void
force_board (void)
{
    BRD *brd;
    int bno;

    if (!cuser.userlevel)                /* guest 跳過 */
        return;
    bno = brd_bno(FORCE_BOARD);
    brd = bshm->bcache +  bno;
    brh_get(brd->bstamp, bno);

//  while (brd->blast > brd_visit[bno]) {
    while (brh_unread(brd->blast)) {
        vmsg("有新公告!! 請先閱\讀完新公告後再離開..");
        XoPost(bno);
        xover(XZ_POST);
        time(&brd_visit[bno]);

        strcpy(currboard, FORCE_BOARD);
        strcpy(currBM, "");
    }
}
#endif


/* ----------------------------------------------------- */
/* board permission check                                */
/* ----------------------------------------------------- */


GCC_PURE static inline bool
is_bm(
    const char *list)                 /* 板主：BM list */
{
    int cc, len;
    char *userid;

    len = strlen(userid = cuser.userid);
    do
    {
        cc = list[len];
        if ((!cc || cc == '/') && !str_ncmp(list, userid, len))
        {
            return true;
        }
        while ((cc = *list++))
        {
            if (cc == '/')
                break;
        }
    } while (cc);

    return false;
}

#if     defined(HAVE_RESIST_WATER) || defined(HAVE_DETECT_CROSSPOST)
void
remove_perm(void)
{
    int i;
    for (i=0; i<(sizeof(brd_bits)/sizeof(int)); i++)
        brd_bits[i] &= ~BRD_W_BIT;
}
#endif

int
Ben_Perm(
    const BRD *bhdr,
    unsigned int ulevel)
{
    unsigned int readlevel, postlevel, bits;
    const char *blist, *bname;

    bname = bhdr->brdname;
    if (!*bname)
        return 0;

    if (!str_cmp(bname, DEFAULT_BOARD))
    {
#ifdef  HAVE_MODERATED_BOARD
#if defined(HAVE_WATER_LIST) && defined(HAVE_SYSOP_WATERLIST)
        if (bm_belong(bname) == BRD_R_BIT)
            return BRD_R_BIT;
        else
#endif
#endif  /* #ifdef  HAVE_MODERATED_BOARD */
            return (BRD_R_BIT | BRD_W_BIT);
    }

    bits = 0;

    readlevel = bhdr->readlevel;
    if (!readlevel || (readlevel & ulevel))
    {
        bits = BRD_R_BIT;

        if (ulevel & PERM_POST)
        {
            postlevel = bhdr->postlevel;
            if (!postlevel || (postlevel & ulevel))
                bits |= BRD_W_BIT;
        }

    }

    /* (moderated) 祕密看板：核對看板之好友名單 */

#ifdef HAVE_MODERATED_BOARD
    if (readlevel & PERM_ADMIN)
    {
        bits = 0;

        bits = bm_belong(bname);  /* Thor.980813: 對秘密看版而言, 是重新判斷的 */

        if (readlevel & PERM_SYSOP)
        {
            if (readlevel & PERM_BOARD)
                ;
            else
                bits |= BRD_F_BIT;
        }

    }
#ifdef HAVE_WATER_LIST
    else if (bm_belong(bname) == BRD_R_BIT)
        bits &= ~BRD_W_BIT;

#endif
#endif  /* #ifdef HAVE_MODERATED_BOARD */

    /* Thor.980813: 註解: 特別為 BM 考量, bm 有該版的所有權限 */

    blist = bhdr->BM;

    if ((ulevel & PERM_BM) && blist[0] > ' ' && is_bm(blist))
        return (BRD_R_BIT | BRD_W_BIT | BRD_X_BIT);

#ifndef HAVE_BOARD_SEE
    if (ulevel & PERM_ALLBOARD)
        bits |= (BRD_W_BIT | BRD_X_BIT);
#endif
    if (!str_cmp(cuser.userid, ELDER))
        bits = BRD_R_BIT | BRD_W_BIT | BRD_X_BIT;

    return bits;
}


/* ----------------------------------------------------- */
/* 載入 currboard 進行若干設定                           */
/* ----------------------------------------------------- */

GCC_PURE int
bstamp2bno(
    time_t stamp)
{
    BRD *brd;
    int bno, max;

    bno = 0;
    brd = bshm->bcache;
    max = bshm->number;
    for (;;)
    {
        if (stamp == brd->bstamp)
            return bno;
        if (++bno >= max)
            return -1;
        brd++;
    }
}

void
brh_load(void)
{
    BRD *brdp, *bend;
    unsigned int ulevel;
    int n, cbno GCC_UNUSED;
    char *bits;

    int size, *base;
    time_t expire, *bstp;
    char fpath[64];

    ulevel = cuser.userlevel;
#ifdef HAVE_BOARD_SEE
    n = (ulevel & PERM_ALLBOARD) ? (BRD_R_BIT | BRD_W_BIT | BRD_X_BIT) : 0;
#else
    n = (ulevel & PERM_SYSOP) ? (BRD_R_BIT | BRD_W_BIT | BRD_X_BIT) : 0;
#endif
    memset(bits = brd_bits, n, sizeof(brd_bits));
    memset(bstp = brd_visit, 0, sizeof(brd_visit));

    if (n == 0)
    {
        brdp = bshm->bcache;
        bend = brdp + bshm->number;

        do
        {
            *bits++ = Ben_Perm(brdp, ulevel);
        } while (++brdp < bend);
    }

    /* --------------------------------------------------- */
    /* 將 .BRH 載入 memory                                 */
    /* --------------------------------------------------- */

    size = 0;
    cbno = -1;
    brh_expire = expire = time(0) - BRH_EXPIRE * 86400;

    if (ulevel)
    {
        struct stat st;

        usr_fpath(fpath, cuser.userid, FN_BRH);
        if (!stat(fpath, &st))
            size = st.st_size;
    }

    /* --------------------------------------------------- */
    /* 多保留 BRH_WINDOW 的運作空間                        */
    /* --------------------------------------------------- */

    /* brh_size = n = ((size + BRH_WINDOW) & -BRH_PAGE) + BRH_PAGE; */


    brh_size = n = size + BRH_WINDOW;
    brh_base = base = (int *) malloc(n);

    if (size && ((n = open(fpath, O_RDONLY)) >= 0))
    {
        int *head, *tail, *list, bstamp, bhno;

        size = read(n, base, size);
        close(n);


        /* compact reading history : remove dummy/expired record */

        head = base;
        tail = (int *) ((char *) base + size);
        bits = brd_bits;
        while (head < tail && head >= brh_base)
        {
            bstamp = *head;

            if (bstamp & BRH_SIGN)      /* zap */
            {
                bstamp ^= BRH_SIGN;

                bhno = bstamp2bno(bstamp);

                if (bhno >= 0)
                {
                    bits[bhno] |= BRD_Z_BIT;
                    time(&brd_visit[bhno]);
                }
                head++;
                continue;
            }

            bhno = bstamp2bno(bstamp);

            list = head + 2;

            if (list > tail)
                break;

            n = *list;
            size = n + 3;

            if (n < 0 || head + size > tail) {
                n = 0;
                size = 3;
            }

            /* 這個看板存在、沒有被 zap 掉、可以 read */

            if (bhno >= 0 && (bits[bhno] & BRD_R_BIT))
            {

                bits[bhno] |= BRD_H_BIT;/* 已有閱讀記錄 */
                bstp[bhno] = head[1];   /* 上次閱讀時間 */
                cbno = bhno;

                if (n > 0)
                {

#if 0
                    n = BMIN(n, BRH_MAX);
#endif

                    list += n;   /* Thor.980904: 註解: 最後一個tag */

                    if (list > tail)
                        break;

                    do
                    {
                        bhno = *list;
                        if ((bhno & BRH_MASK) > expire)
                            break;

                        if (!(bhno & BRH_SIGN))
                        {
                            if (*--list > expire)
                                break;
                            n--;
                        }

                        list--;
                        n--;
                    } while (n > 0);

                    head[2] = n;
                }

                n = n * sizeof(time_t) + sizeof(BRH);
                if (base != head)
                    memmove(base, head, n);
                base = (int *) ((char *) base + n);
            }
            head += size;
        }
    }

    *base = 0;
    brh_tail = base;

    /* --------------------------------------------------- */
    /* 設定 default board                                  */
    /* --------------------------------------------------- */

    strcpy(currboard, "尚未選定");
#ifdef HAVE_BOARD_PAL
    cutmp->board_pal = currbno;
#endif
#ifdef  HAVE_RESIST_WATER
    if (checkqt > CHECK_QUOT_MAX)
        remove_perm();
#endif
}

void
brh_save(void)
{
    int *base, *head, *tail, bhno, size;
    BRD *bhdr, *bend;
    char *bits;

    /* Thor.980830: lkchu patch:  還沒 load 就不用 save */
    if (!(base = brh_base))
        return;

#if 0
    base = brh_base;
#endif

    brh_put();

    /* save history of un-zapped boards */

    bits = brd_bits;
    head = base;
    tail = brh_tail;
    while (head < tail)
    {
        bhno = bstamp2bno(*head);
        size = head[2] * sizeof(time_t) + sizeof(BRH);
        if (bhno >= 0 && !(bits[bhno] & BRD_Z_BIT))
        {
            if (base != head)
                memmove(base, head, size);
            base = (int *) ((char *) base + size);
        }
        head = (int *) ((char *) head + size);
    }

    /* save zap record */

    tail = brh_alloc(base, sizeof(time_t) * MAXBOARD);

    bhdr = bshm->bcache;
    bend = bhdr + bshm->number;
    do
    {
        if (*bits++ & BRD_Z_BIT)
        {
            *tail++ = bhdr->bstamp | BRH_SIGN;
        }
    } while (++bhdr < bend);

    /* OK, save it */

    base = brh_base;
    if ((size = ((char *) tail) - ((char *) base)) > 0)
    {
        char fpath[64];
        int fd;

        usr_fpath(fpath, cuser.userid, FN_BRH);
        if ((fd = open(fpath, O_WRONLY | O_CREAT | O_TRUNC, 0600)) >= 0)
        {
            write(fd, base, size);
            close(fd);
        }
    }

    // Unload
    free(brh_base);
    brh_base = NULL;
    brh_tail = NULL;
    brh_size = 0;
}

/*-------------------------------------------------------*/

#ifdef LOG_BRD_USIES
/* lkchu.981201: 看板閱讀記錄 */
void
brd_usies(void)
{
    char buf[256];

    sprintf(buf, "%s %s (%s)\n", currboard, cuser.userid, Now());
    f_cat(FN_BRD_USIES, buf);
}
#endif

static void
brd_usies_BMlog(void)
{
    char fpath[64], buf[256];

    brd_fpath(fpath, currboard, "usies");
    sprintf(buf, "%s %s\n", Now(), cuser.userid);
    f_cat(fpath, buf);
}

/* 081206.cache: 好友板修正 */
bool
XoPost(
    int bno)
{
    XO *xo;
    BRD *brd;
    int bits;
    char *str_bit, fpath[64];
    const char *str;

    str_bit = &brd_bits[bno];
    bits = *str_bit;

    /* cache.081206: 對好友看版重新判斷是不是也具有 R 權限 */
    if (!(bits & BRD_R_BIT)/* && (bits & BRD_F_BIT)*/)
    {
        vmsg("請聯絡板主將您加入看板好友");
        return false;
    }

    /* 090823.cache: 看板人氣 */
    if (currbno != bno)
    {
        /* 退出上一個板 */
        if (currbno >= 0)
        {
            //防止人氣變成負數；負數的話歸零
            bshm->mantime[currbno] = BMAX(bshm->mantime[currbno]-1, 0);
        }

        bshm->mantime[bno]++;       /* 進入新的板 */

        currbno = bno;
    }

    brd = bshm->bcache + bno;
    strcpy(currboard, brd->brdname);
#ifdef HAVE_BOARD_PAL
    cutmp->board_pal = bno;
#endif
    brh_get(brd->bstamp, bno);

    bbstate = /* (bbstate & STAT_DIRTY) | */ STAT_STARTED | brd->battr;
    bbstate &= ~STAT_POST;

    if (bits & BRD_X_BIT)
        bbstate |= (STAT_BOARD | STAT_POST);
    else if (bits & BRD_W_BIT)
        bbstate |= STAT_POST;

#ifdef  HAVE_MODERATED_BOARD
    if (brd->readlevel == PERM_SYSOP)
        bbstate |= STAT_MODERATED;
#endif

    if (/*!(bits & BRD_V_BIT) && */(cuser.ufo2 & UFO2_BNOTE))
    {
        *str_bit = bits | BRD_V_BIT;
        brd_fpath(fpath, currboard, FN_NOTE);
        more(fpath, NULL);
    }

    brd_fpath(fpath, currboard, fn_dir);
    xz[XZ_POST - XO_ZONE].xo = xo = xo_get(fpath);
    xo->key = XZ_POST;
    xo->xyz = brd->bvote > 0 ? (char *) "本看板進行投票中" : brd->title + 3;
    str = brd->BM;
    if (*str <= ' ')
        str = "徵求中";
    sprintf(currBM, "板主：%s", str);

#ifdef LOG_BRD_USIES
    /* lkchu.981201: 閱讀看版記錄 */
    if (!(bbstate & BRD_NOLOGREAD))
        brd_usies();
#endif

    if (!(brd->battr & BRD_ANONYMOUS))
        brd_usies_BMlog();

#ifdef  HAVE_COUNT_BOARD
//      if (!(strcmp(brd->brdname, "Test")))
    if (!(bbstate & BRD_NOTOTAL) && !(bits & BRD_V_BIT))
        brd->n_reads++;
#endif
    return true;
}


/* ----------------------------------------------------- */
/* Class [分類群組]                                      */
/* ----------------------------------------------------- */

/* cache.090503: 即時熱門看板 */
static int
mantime_cmp(
    const void *a,
    const void *b)
{
    /* 由多排到少 */
    return bshm->mantime[* (const short *)b] - bshm->mantime[* (const short *)a];
}

static int class_flag2 = 0;  /* 1:列出好友/秘密板，且自己又有閱讀權限的 */
static int class_flag;

#define BFO_YANK        0x01

static int
class_check(
    int chn)
{
    short *cbase, *chead, *ctail;
    int pos, max, val, zap;
    int bnum = 0;
    int class_hot = 0;
    BRD *brd;
    char *bits;

    if (!class_img)
        return 0;

    chn = CH_END - chn;
    switch (boardmode)
    {
        case 0:
            cbase = (short *) class_img;
            break;
#ifdef  HAVE_PROFESS
        case 1:
            cbase = (short *) profess_img;
            break;
#endif
        default:
            cbase = (short *) class_img;
    }

    // "HOT/" 名稱可自定，若改名也要順便改後面的長度 4
    if (!strncmp((char *)cbase + cbase[chn], "HOT/", 4))
    {
        class_hot = 1;
        chn = 0;
    }

    chead = cbase + chn;

    pos = chead[0] + CH_TTLEN;
    max = chead[1];

    chead = (short *) ((char *) cbase + pos);
    ctail = (short *) ((char *) cbase + max);

    max = 0;
    brd = bshm->bcache;
    bits = brd_bits;
    zap = (class_flag & BFO_YANK) ? 0 : BRD_Z_BIT;

    do
    {
        chn = *chead++;
        if (chn >= 0)
        {
            val = bits[chn];
/* cache.081207: 處理哪些 board 要 print */
/* thanks to gaod */
/* cache.090503: 即時熱門看板 */
/* cache.091225: 列出所有有閱讀權限的看板 */

            if (brd[chn].readlevel != PERM_CHATROOM)
            {
                if (class_flag2 &&
                    (!(val & BRD_R_BIT) || !(brd[chn].brdname[0]) ||
                    !(brd[chn].readlevel) || (brd[chn].readlevel & ~(PERM_SYSOP | PERM_BOARD)) ))

                        continue;

                if ((val & BRD_F_BIT) && !(val & zap))
                    ;
                else
                {
                    if ( !(val & BRD_R_BIT) || (val & zap) || !(brd[chn].brdname[0]) )
                        continue;
                }
            }

            // 即時熱門看板臨界值自定
            if (class_hot)
            {
                if (bshm->mantime[chn] < CLASS_HOT) /* 只列出人氣超過 CLASS_HOT 的看板 */
                    continue;
                bnum++;
            }
        }
        else if (!class_check(chn))
            continue;
        max++;
    } while (chead < ctail);

    if (class_hot && bnum > 0)
        qsort(cbase - bnum, bnum, sizeof(short), mantime_cmp);

    return max;
}

static int
class_load(
    XO *xo)
{
    short *cbase, *chead, *ctail;
    int chn;                    /* ClassHeader number */
    int pos, max, val, zap;
    int bnum = 0;
    int class_hot = 0;
    BRD *brd;
    char *bits;

    chn = CH_END - xo->key;

    switch (boardmode)
    {
        case 0:
            cbase = (short *) class_img;
            break;
#ifdef  HAVE_PROFESS
        case 1:
            cbase = (short *) profess_img;
            break;
#endif
        default:
            cbase = (short *) class_img;
    }

    // "HOT/" 名稱可自定，若改名也要順便改後面的長度 4
    if (!strncmp((char *)cbase + cbase[chn], "HOT/", 4))
    {
        class_hot = 1;
        chn = 0;
    }

    chead = cbase + chn;

    pos = chead[0] + CH_TTLEN;
    max = chead[1];

    chead = (short *) ((char *) cbase + pos);
    ctail = (short *) ((char *) cbase + max);

    max -= pos;

    cbase = (short *) realloc(xo->xyz, max);
    xo->xyz = (char *) cbase;

    max = 0;
    brd = bshm->bcache;
    bits = brd_bits;
    zap = (class_flag & BFO_YANK) ? 0 : BRD_Z_BIT;

    do
    {
        chn = *chead++;
        if (chn >= 0)
        {
            val = bits[chn];
/* cache.081207: 處理哪些 board 要 print */
/* thanks to gaod */
/* cache.090503: 即時熱門看板 */
/* cache.091225: 列出所有有閱讀權限的看板 */
            if (brd[chn].readlevel != PERM_CHATROOM)
            {
                if (class_flag2 &&
                    (!(val & BRD_R_BIT) || !(brd[chn].brdname[0]) ||
                    !(brd[chn].readlevel) || (brd[chn].readlevel & ~(PERM_SYSOP | PERM_BOARD)) ))
                        continue;

                if ((val & BRD_F_BIT) && !(val & zap))
                    ;
                else
                {
                    if ( !(val & BRD_R_BIT) || (val & zap) || !(brd[chn].brdname[0]) )
                        continue;
                }
            }
            // 即時熱門看板臨界值自定
            if (class_hot)
            {
                if (bshm->mantime[chn] < CLASS_HOT) /* 只列出人氣超過 CLASS_HOT 的看板 */
                    continue;
                bnum++;
            }
        }
        else if (!class_check(chn))
        {
            continue;
        }

        max++;
        *cbase++ = chn;
    } while (chead < ctail);

    if (class_hot && bnum > 0)
        qsort(cbase - bnum, bnum, sizeof(short), mantime_cmp);

    xo->max = max;
    if (xo->pos >= max)
        xo->pos = xo->top = 0;

    return max;
}

static int
XoClass(
    int chn)
{
    XO xo, *xt;

    /* Thor.980727: 解決 XO xo的不確定性,
                    class_load內部會 initial xo.max, 其他不確定 */
    xo.pos = xo.top = 0;

    xo.key = chn;
    xo.xyz = NULL;
    if (!class_load(&xo))
    {
        int ret = 0;
        if (!ret && (class_flag2 & 0x01))
        {
            class_flag2 ^= 0x01;
            ret = class_load(&xo);
        }
        if (!ret && !(class_flag & BFO_YANK))
        {
            class_flag |= BFO_YANK;
            ret = class_load(&xo);
        }
        if (!ret)
        {
            free(xo.xyz);
            return XO_NONE;
        }
    }

    xt = xz[XZ_CLASS - XO_ZONE].xo;
    xz[XZ_CLASS - XO_ZONE].xo = &xo;
    xover(XZ_CLASS);
    free(xo.xyz);
    xz[XZ_CLASS - XO_ZONE].xo = xt;

    return XO_BODY;
}

void
board_outs(
    int chn,
    int num)
{
    BRD *brd;
    char *bits, buf[16], buf2[20], brdtype;
    const char *str, *str2;
    int brdnew, bno;

    brd = bshm->bcache + chn;

    brdnew = class_flag & UFO2_BRDNEW;
    bits = brd_bits;

    {
        int fd, fsize;
        char folder[64];
        struct stat st;

        brd_fpath(folder, brd->brdname, fn_dir);
        if ((fd = open(folder, O_RDONLY)) >= 0)
        {
            fstat(fd, &st);

            if (st.st_mtime > brd->btime)  // 上次統計後，檔案有修改過
            {
                if ((fsize = st.st_size) >= sizeof(HDR))
                {
                    HDR hdr;

                    brd->bpost = fsize / sizeof(HDR);
                    while ((fsize -= sizeof(HDR)) >= 0)
                    {
                        lseek(fd, fsize, SEEK_SET);
                        read(fd, &hdr, sizeof(HDR));
                        if (!(hdr.xmode & (POST_LOCK | POST_BOTTOM)))
                            break;
                    }
                    brd->blast = (brdnew) ? BMAX(hdr.chrono, brd->blast) : hdr.chrono;
                }
                else
                    brd->blast = brd->bpost = 0;
            }

            close(fd);
        }
        if (brdnew)
            num = brd->bpost;
    }

    str = (!(bits[chn] & BRD_Z_BIT) && brd->blast > brd_visit[chn]) ? "\x1b[1;31m★\x1b[m" : "☆";

    char tmp[BTLEN + 1] = {0};
    int e_cols = (d_cols + 32 > BTLEN) ? BTLEN - 32 : d_cols;

    strcpy(tmp, brd->title);
    if (tmp[e_cols + 31] & 0x80)
//      tmp[e_cols + 33] = '\0';
//  else
        tmp[e_cols + 31] = ' ';
    tmp[e_cols + 32] = '\0';

/* 081122.cache:看板性質, 不訂閱, 秘密, 好友, 一般 */
    if (bits[chn] & BRD_Z_BIT)
        brdtype = '-';
#ifdef HAVE_MODERATED_BOARD
    else if (brd->readlevel & PERM_BOARD)
        brdtype = '.';
    else if (brd->readlevel & PERM_SYSOP)
        brdtype = ')';
#endif
    else
        brdtype = ' ';

/* 處理 人氣 */ /* cache.20090416: 仿ptt變色*/
    bno = bshm->mantime[chn];
    if (brd->bvote)
        str2 = "\x1b[1;33m  投 \x1b[m";
    else if (bno > 4999)
        str2 = "\x1b[1;32m TOP \x1b[m";
    else if (bno > 999)
        str2 = "\x1b[1;32m  夯 \x1b[m";
    else if (bno > 799)
        str2 = "\x1b[1;35m  夯 \x1b[m";
    else if (bno > 699)
        str2 = "\x1b[1;33m  夯 \x1b[m";
    else if (bno > 599)
        str2 = "\x1b[1;42m  爆 \x1b[m";
    else if (bno > 499)
        str2 = "\x1b[1;45m  爆 \x1b[m";
    else if (bno > 449)
        str2 = "\x1b[1;44m  爆 \x1b[m";
    else if (bno > 399)
        str2 = "\x1b[1;32m  爆 \x1b[m";
    else if (bno > 349)
        str2 = "\x1b[1;35m  爆 \x1b[m";
    else if (bno > 299)
        str2 = "\x1b[1;33m  爆 \x1b[m";
    else if (bno > 249)
        str2 = "\x1b[1;36m  爆 \x1b[m";
    else if (bno > 199)
        str2 = "\x1b[1;31m  爆 \x1b[m";
    else if (bno > 149)
        str2 = "\x1b[1;37m  爆 \x1b[m";
    else if (bno > 99)
        str2 = "\x1b[1;31m HOT \x1b[m";
    else if (bno > 49)
        str2 = "\x1b[1;37m HOT \x1b[m";
    else if (bno > 1) {  /* r2.170810: let somebody know which board is still "alive" :P */
        sprintf(buf2, "  %2d ", bno);
        str2 = buf2;
    }
    else
        str2 = "     ";
//注意有三格空白, 因為 HOT 是三個 char 故更改排版
//      prints("\x1b[%d;4%d;37m%6d%s%s%c%-13s\x1b[%sm%-4s %s%-33.32s%s%s%.13s", mode, mode?cuser.barcolor:0, num, str, mode ? "\x1b[37m" : "\x1b[m",
//          brdtype, brd->brdname, buf, brd->class_, mode ? "\x1b[37m" : "\x1b[m", brd->title, brd->bvote ? "\x1b[1;33m  投 " : str2, mode ? "\x1b[37m" : "\x1b[m", brd->BM);

    sprintf(buf, "%d;3%d", brd->color/10, brd->color%10);
//      prints("%6d%s%c%-13s\x1b[%sm%-4s \x1b[m%-36s%c %.13s", num, str,
//      prints("%6d%s%c%-13s\x1b[%sm%-4s \x1b[m%s%c %.13s", num, str,

    prints("%6d%s%c%-13s\x1b[%sm%-4s \x1b[m%-*s %s", num, str, brdtype, brd->brdname, buf, brd->class_, d_cols + 32, tmp, str2);

    strcpy(tmp, brd->BM);
    if (tmp[13] & 0x80)
        tmp[12] = '\0';
    else
        tmp[13] = '\0';

    prints("%-13s\n", tmp);
}

void
class_outs(
    const char *title,
    int num)
{
    prints("%6d   %s\n", num, title);
}

static void
class_item(
    int num,
    int chn)
{

    if (chn >= 0)
        board_outs(chn, num);
    else
    {
        char *img;
        short *chx;

        switch (boardmode)
        {
            case 0:
                img = class_img;
                break;
#ifdef  HAVE_PROFESS
            case 1:
                img = profess_img;
                break;
#endif
            default:
                img = class_img;
        }

        chx = (short *) img + (CH_END - chn);
        class_outs(img + *chx, num);
    }
}


static int
class_body(
    XO *xo)
{
    short *chp;
    int num, max, tail;

    max = xo->max;
    if (max <= 0)
    {
        int ret = class_load(xo);
        if (!ret && (class_flag2 & 0x01))
        {
            class_flag2 ^= 0x01;
            ret = class_load(xo);
        }
        if (!ret && !(class_flag & BFO_YANK))
        {
            class_flag |= BFO_YANK;
            ret = class_load(xo);
        }
        if (!ret)
        {
            return XO_QUIT;
        }
        return XO_BODY;
    }

    chp = (short *) xo->xyz + xo->top;
    num = xo->top;
    tail = num + XO_TALL;
    max = BMIN(max, tail);

    move(3, 0);
    do
    {
        class_item(++num, *chp++);
    } while (num < max);

    clrtobot();
    return XO_NONE;
}

static int
class_neck(
    XO *xo)
{
    move(1, 0);
    prints(NECKBOARD, class_flag & UFO2_BRDNEW ? "總數" : "編號", d_cols + 33, "中   文   敘   述");

    return class_body(xo);
}

static int
class_head(
    XO *xo)
{
    vs_head("看板列表", str_site);
    return class_neck(xo);
}

static int
class_init(                     /* re-init */
    XO *xo)
{
    class_load(xo);
    return class_head(xo);
}

static int
class_newmode(
    XO *xo)
{
    cuser.ufo2 ^= UFO2_BRDNEW;  /* Thor.980805: 特別注意 utmp.ufo的同步問題 */
    class_flag ^= UFO2_BRDNEW;
    return class_neck(xo);
}

static int
class_help(
    XO *xo)
{
    film_out(FILM_CLASS, -1);
    return class_head(xo);
}


static int
class_search(
    XO *xo)
{
    int num, pos, max;
    char *ptr;
    char buf[IDLEN + 1];

    ptr = buf;
    pos = vget(b_lines, 0, MSG_BID, ptr, IDLEN + 1, DOECHO);
    move(b_lines, 0);
    clrtoeol();

    if (pos)
    {
        short *chp, chn;
        BRD *bcache, *brd;

        bcache = bshm->bcache;

        str_lower(ptr, ptr);
        pos = num = xo->pos;
        max = xo->max;
        chp = (short *) xo->xyz;
        do
        {
            if (++pos >= max)
                pos = 0;
            chn = chp[pos];
            if (chn >= 0)
            {
                brd = bcache + chn;
                if (str_str(brd->brdname, ptr) || str_str(brd->title, ptr))
                    return pos + XO_MOVE;
            }
        } while (pos != num);
    }

    return XO_NONE;
}

/* cache.091125: 只列出具有閱讀權限的秘密/好友看板 */
static int
class_yank2(
    XO *xo)
{
    int pos = xo->pos;
    if (xo->key >= 0)
        return XO_NONE;

    class_flag2 ^= 0x01;
    if (!class_load(xo) && (class_flag2 & 0x01))
    {
        zmsg("找不到可讀的秘密/好友看板");
        class_flag2 ^= 0x01;
        class_load(xo);
        xo->pos = pos;
        return XO_NONE;
    }
    return class_head(xo);
}

static int
class_yank(
    XO *xo)
{
    int pos = xo->pos;
    if (xo->key >= 0)
        return XO_NONE;

    class_flag ^= BFO_YANK;
    if (!class_load(xo) && !(class_flag & BFO_YANK))
    {
        zmsg("找不到未被 zap 掉的看板");
        class_flag |= BFO_YANK;
        class_load(xo);
        xo->pos = pos;
        return XO_NONE;
    }
    return class_head(xo);
}

static int
class_zap(
    XO *xo)
{
    BRD *brd;
    short *chp;
    int num, chn;

    num = xo->pos;
    chp = (short *) xo->xyz + num;
    chn = *chp;
    if (chn >= 0)
    {
        brd = bshm->bcache + chn;
        if (!(brd->battr & BRD_NOZAP) || (brd_bits[chn] & BRD_Z_BIT))
        {
            move(3 + num - xo->top, 0);
            //move(3 + num - xo->top, 8);
            brd_bits[chn] ^= BRD_Z_BIT;
            //outc(num & BRD_Z_BIT ? '-' : ' ');
            class_item(num + 1, chn);
        }
    }

    return XO_NONE;
}


static int
class_edit(
    XO *xo)
{
    /* if (HAS_PERM(PERM_ALLBOARD)) */
    /* Thor.990119: 只有站長可以修改 */
    if (HAS_PERM(PERM_SYSOP) || HAS_PERM(PERM_BOARD))
    {
        short *chp;
        int chn;

        chp = (short *) xo->xyz + xo->pos;
        chn = *chp;
        if (chn >= 0)
        {
            brd_edit(chn);
            return class_init(xo);
        }
    }
    return XO_NONE;
}

static int
class_browse(
    XO *xo)
{
    short *chp;
    int chn;

    chp = (short *) xo->xyz + xo->pos;
    chn = *chp;
    if (chn < 0)
    {
        short *chx;
        char *img, *str;

        img = class_img;
        chx = (short *) img + (CH_END - chn);
        str = img + *chx;

        if (XoClass(chn) == XO_NONE)
            return XO_NONE;
    }
    else
    {
        if (XoPost(chn))
        {
            xover(XZ_POST);
            time(&brd_visit[chn]);
        }
    }

    return class_head(xo);      /* Thor.0701: 無法清少一點, 因為 XoPost */
}

int
Select(void)
{
    int bno;
    BRD *brd;
    char bname[16];

    if ((brd = ask_board(bname, BRD_R_BIT, NULL)))
    {
        bno = brd - bshm->bcache;
        if (*bname)
        {
            XoPost(bno);
        }
        xover(XZ_POST);
        time(&brd_visit[bno]);
    }
    else
    {
        vmsg(err_bid);
    }

    return 0;
}

static int
class_switch(
    XO *xo)
{
    Select();
    return class_head(xo);
}

#ifdef AUTHOR_EXTRACTION
/* Thor.0818: 想改成以目前的看版列表或分類來找, 不要找全部 */

/* opus.1127 : 計畫重寫, 可 extract author/title */

#if 0  // Unused
static int
XoAuthor(
    XO *xo)
{
    int chn, len, max, tag;
    short *chp, *chead, *ctail;
    BRD *brd;
    char author[IDLEN + 1];
    XO xo_a, *xoTmp;
#ifndef HAVE_MMAP
    XO *xo_t;
#endif

    if (!HAS_PERM(PERM_VALID))
        return XO_NONE;

    if (!vget(b_lines, 0, "請輸入作者：", author, IDLEN + 1, DOECHO))
        return XO_FOOT;

    str_lower(author, author);
    len = strlen(author);

    chead = (short *) xo->xyz;
    max = xo->max;
    ctail = chead + max;

    tag = 0;
    chp = (short *) malloc(max * sizeof(short));
    brd = bshm->bcache;

    do
    {
        if ((chn = *chead++) >= 0)      /* Thor.0818: 不為 group */
        {
            /* Thor.0701: 尋找指定作者文章, 有則移位置, 並放入 */

#ifdef HAVE_MMAP
            int fsize;
            char *fimage;
#endif
            char folder[80];
            HDR *head, *tail;

            sprintf(folder, "《尋找指定作者》看版：%s \x1b[5m...\x1b[m",
                brd[chn].brdname);
            outz(folder);
            refresh();
            brd_fpath(folder, brd[chn].brdname, fn_dir);

#ifdef HAVE_MMAP
            fimage = f_map(folder, &fsize);

            if (fimage == (char *) -1)
                continue;

            head = (HDR *) fimage;
            tail = (HDR *) (fimage + fsize);

            while (head <= --tail)
            {
                if (tail->xmode & (POST_CANCEL | POST_DELETE | POST_MDELETE | POST_LOCK))
                    continue;

                /* if (str_str(temp, author)) *//* Thor.0818:希望比較快 */

                if (!str_ncmp(tail->owner, author, len))
                {
                    xo_get(folder)->pos = tail - head;
                    chp[tag++] = chn;
                    break;
                }
            }

            munmap(fimage, fsize);

#else
            xo_t = xo_new(folder);
            xo_t->pos = XO_TAIL;        /* 第一次進入時，將游標放在最後面 */
            xo_load(xo_t, sizeof(HDR));
            if (xo_t->max <= 0)
                continue;
            head = (HDR *) xo_pool;
            tail = (HDR *) xo_pool + (xo_t->pos - xo_t->top);
            for (;;)
            {
                if (!(tail->xmode & (POST_CANCEL | POST_DELETE | POST_MDELETE | POST_LOCK)))
                {
                    /* check condition */
                    if (!str_ncmp(tail->owner, author, len))    /* Thor.0818:希望比較快 */
                    {
                        xo_get(folder)->pos = tail - head;
                        chp[tag++] = chn;
                        break;
                    }
                }
                tail--;
                if (tail < head)
                {
                    if (xo_t->top <= 0)
                        break;
                    xo_t->pos -= XO_TALL;
                    xo_load(xo_t, sizeof(HDR));
                    tail = (HDR *) xo_pool + XO_TALL - 1;
                }
            }

            free(xo_t);
#endif  /* #ifdef HAVE_MMAP */

        }
    } while (chead < ctail);

    if (!tag)
    {
        free(chp);
        vmsg("空無一物");
        return XO_FOOT;
    }

    xo_a.pos = xo_a.top = 0;
    xo_a.max = tag;
    xo_a.key = 1;                       /* all boards */
    xo_a.xyz = (char *) chp;

    xoTmp = xz[XZ_CLASS - XO_ZONE].xo;  /* Thor.0701: 記下原來的class_xo */

    xz[XZ_CLASS - XO_ZONE].xo = &xo_a;
    xover(XZ_CLASS);

    free(chp);

    xz[XZ_CLASS - XO_ZONE].xo = xoTmp;          /* Thor.0701: 還原 class_xo */

    return class_body(xo);
}
#endif  // #if 0  // Unused
#endif  /* #ifdef AUTHOR_EXTRACTION */

#if defined(HAVE_COUNT_BOARD) && 0
static int
class_stat(xo)
    XO *xo;
{
    BRD *brd;
    short *chp;
    int num, chn;
    char msg[80];

    num = xo->pos;
    chp = (short *) xo->xyz + num;
    chn = *chp;
    if (chn >= 0)
    {
        brd = bshm->bcache + chn;
        sprintf(msg, "目前累積閱\讀數：%d，累積發文數：%d", brd->n_reads, brd->n_posts);
        pmsg(msg);
    }

    return XO_NONE;
}
#endif

static int
class_visit(
    XO *xo)
{
    short *chp;
    int chn;

    chp = (short *) xo->xyz + xo->pos;
    chn = *chp;
    if (chn >= 0)
    {
        BRD *brd;
        brd = bshm->bcache + chn;
        brh_get(brd->bstamp, chn);
        brh_visit(0);
        time(&brd_visit[chn]);
    }
    return class_body(xo);
}

static KeyFunc class_cb[] =
{
    {XO_INIT, {class_head}},
    {XO_LOAD, {class_body}},
    {XO_HEAD, {class_head}},
    {XO_BODY, {class_body}},

    {'r', {class_browse}},
    {'/', {class_search}},
    {'c', {class_newmode}},

    {'s', {class_switch}},

    {'y', {class_yank}},
    {'i', {class_yank2}}, //列出所有有閱讀權限的秘密/好友看板
    {'z', {class_zap}},
    {'E', {class_edit}},
    {'v', {class_visit}},
#ifdef  HAVE_COUNT_BOARD
    {'S' | XO_DL, {.dlfunc = DL_NAME("brdstat.so", main_bstat)}},
#endif

#ifdef  HAVE_FAVORITE
    {'a', {class_add}},
#endif

#ifdef AUTHOR_EXTRACTION
    //{'A', {XoAuthor}},
#endif

    {'h', {class_help}}
};

int
Class(void)
{
    boardmode = 0;
    if (!class_img || XoClass(CH_END-1) == XO_NONE)
    {
        vmsg("未定義分組討論區");
        return XEASY;
    } /* Thor.980804: 防止 未用 account 造出 class.img 或沒有 class的情況 */
    return 0;
}

void
check_new(
    BRD *brd)
{
    int fd, fsize;
    char folder[64];
    struct stat st;

    brd_fpath(folder, brd->brdname, fn_dir);
    if ((fd = open(folder, O_RDONLY)) >= 0)
    {
        fstat(fd, &st);
        if (st.st_mtime > brd->btime)
        {
            brd->btime = time(0) + 45;        /* 45 秒鐘內不重複檢查 */
            if ((fsize = st.st_size) >= sizeof(HDR))
            {
                brd->bpost = fsize / sizeof(HDR);
                lseek(fd, fsize - sizeof(HDR), SEEK_SET);
                read(fd, &brd->blast, sizeof(time_t));
            }
            else
            {
                brd->blast = brd->bpost = 0;
            }
        }
        close(fd);
    }
}

#ifdef  HAVE_INFO
int
Information(void)
{
    int chn;
    chn = brd_bno(BRD_BULLETIN);
    XoPost(chn);
    xover(XZ_POST);
    time(&brd_visit[chn]);
    return 0;
}
#endif

#ifdef  HAVE_STUDENT
int
Student(void)
{
    int chn;
    chn = brd_bno(BRD_SBULLETIN);
    XoPost(chn);
    xover(XZ_POST);
    time(&brd_visit[chn]);
    return 0;
}
#endif

#ifdef  HAVE_PROFESS
int
Profess(void)
{
    boardmode = 1;
    if (!profess_img || XoClass(CH_END-1) == XO_NONE)
    {
        vmsg("未定義分組討論區");
        return XEASY;
    }
    return 0;
}
#endif

void
board_main(void)
{
    int fsize;
#ifdef  HAVE_PROFESS
    int psize;
#endif
    struct stat st;
#ifdef  HAVE_FAVORITE
    int fasize;
    char fpath[128];
#endif
//  brh_load();

#ifdef  HAVE_RECOMMEND
    recommend_time = time(0);
#endif

    /* 如果沒有 .BRD 時，或大小為零時，將 run/class.img 移除
       這樣才不會進選單就掛點 visor.000705 */
    if (stat(FN_BRD, &st) || st.st_size <= 0)
        unlink(CLASS_IMGFILE);

    free(class_img);
    class_img = f_img(CLASS_IMGFILE, &fsize);
#ifdef  HAVE_PROFESS
    free(profess_img);
    profess_img = f_img(PROFESS_IMGFILE, &psize);
#endif

    if (class_img == NULL)
    {
        blog("CACHE", CLASS_IMGFILE);
    }
#ifdef  HAVE_PROFESS
    if (profess_img == NULL)
    {
        blog("CACHE", PROFESS_IMGFILE);
    }
#endif

    if (!cuser.userlevel)               /* guest yank all boards */
    {
        class_flag = BFO_YANK;
    }
    else
    {

#if 0
        class_flag = 0;         /* to speed up */
#endif

        class_flag = cuser.ufo2 & UFO2_BRDNEW;
    }

    board_xo.key = CH_END;
    if (class_img)
        class_load(&board_xo);

    xz[XZ_CLASS - XO_ZONE].xo = &board_xo;      /* Thor: default class_xo */
    xz[XZ_CLASS - XO_ZONE].cb = class_cb;               /* Thor: default class_xo */
}

int
Boards(void)
{
    /* class_xo = &board_xo; *//* Thor: 已有 default, 不需作此 */
    boardmode = -1;
    xover(XZ_CLASS);
    return 0;
}

#ifdef HAVE_MULTI_CROSSPOST
#define MSG_CC "\x1b[32m[看板群組名單]\x1b[m\n"

int
brd_list(
    int reciper)
{
    LIST list;
    int userno, fd, select;
    char buf[32], fpath[64], msg[128], temp[30];
    BRD brd, *ptr;
    sprintf(msg, "A)增加 D)刪除 B)全部看版 C)分類 G)條件 [1~%d]群組名單 M)定案 Q)取消？[M]", MAX_LIST);

    userno = 0;

    for (;;)
    {
        select = vget(1, 0, msg, buf, 2, LCECHO);
        switch (select)
        {
#if 1
        case '1': case '2': case '3': case '4': case '5': case '6': case '7':
        case '8': case '9':
            sprintf(temp, "board.%c", *buf);
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
#endif  /* #if 1 */
        case 'a':
            while ((ptr = ask_board(buf, BRD_W_BIT, NULL)))
            {
                if (!ll_has(ptr->brdname))
                {
                    ll_add(ptr->brdname);
                    reciper++;
                    ll_out(3, 0, MSG_CC);
                }
            }
            break;

        case 'd':

            while (reciper)
            {
                if (!vget(1, 0, "請輸入代號(只按 ENTER 結束刪除): ",
                        buf, IDLEN + 1, GET_LIST))
                    break;
                if (ll_del(buf))
                    reciper--;
                ll_out(3, 0, MSG_CC);
            }
            break;
#if 1
        case 'g':
            if ((userno = vget(b_lines, 0, "群組條件：", buf, 16, DOECHO)))
                str_lower(buf, buf);
            // Falls through
        case 'c':
            if (!userno && vget(b_lines, 0, "分類：", buf, 16, DOECHO))
                str_lower(buf, buf);
            // Falls through
        case 'b':
            if ((fd = open(FN_BRD, O_RDONLY)) >= 0)
            {
                char *name;

                while (read(fd, &brd, sizeof(BRD)) == sizeof(BRD))
                {
                    name = brd.brdname;
                    if (!ll_has(name) && (
                            (select == 'b') ||
                            (select == 'g' && (str_str(brd.brdname, buf) || str_str(brd.title, buf)))||
                            (select == 'c' && str_str(brd.class_, buf))))
                    {
                        ll_add(name);
                        reciper++;
                    }
                }
                close(fd);
            }
            ll_out(3, 0, MSG_CC);
            userno = 0;
            break;
#endif  /* #if 1 */
        case 'q':
            return 0;

        default:
            return reciper;
        }
    }
}

#endif  /* #ifdef HAVE_MULTI_CROSSPOST */
