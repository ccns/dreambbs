/*-------------------------------------------------------*/
/* board.c      ( NTHU CS MapleBBS Ver 3.02 )            */
/*-------------------------------------------------------*/
/* target : �ݪO�B�s�ե\��                               */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/

#include "bbs.h"

static time32_t *brh_base;      /* allocated memory */
static time32_t *brh_tail;      /* allocated memory */
static int brh_size;            /* allocated memory size */
static time_t brh_expire;

char brd_bits[MAXBOARD];
time_t brd_visit[MAXBOARD];     /* �̪��s���ɶ� */
static char *class_img;

#ifdef  HAVE_PROFESS
static char *profess_img = NULL;
#endif

static XO board_xo;
BRD *xbrd;
int boardmode=0;

static time32_t *
brh_alloc(
    time32_t *tail,
    int size)
{
    time32_t *base;
    int n;

    base = brh_base;
    n = (char *) tail - (char *) base;
    size += n;
    if (size > brh_size)
    {
        /* size = (size & -BRH_PAGE) + BRH_PAGE; */
        size += n >> 4;         /* �h�w���@�ǰO���� */
        base = (time32_t *) realloc((char *) base, size);

        if (base == NULL)
            abort_bbs();

        brh_base = base;
        brh_size = size;
        tail = (time32_t *) ((char *) base + n);
    }

    return tail;
}


static void
brh_put(void)
{
    time32_t *list;

    /* compact the history list */

    list = brh_tail;

    if (*list)
    {
        time32_t *head, *tail;
        int n;
        time_t item, chrono;

        n = *++list;   /* Thor.980904: ��Ū�ɬObhno */
        brd_bits[n] |= BRD_H_BIT;
        time32(list);  /* Thor.980904: ����: bvisit time */

        item = *++list;
        head = ++list;
        tail = head + item;

        while (head < tail)
        {
            chrono = *head++;
            n = *head++;
            if (n == chrono) /* Thor.980904: ����: �ۦP���ɭ����_�� */
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
        brh_tail = list;  /* Thor.980904:�s����brh */
    }
}


void
brh_get(
    time_t bstamp,              /* board stamp */
    int bhno)
{
    time32_t *head, *tail;
    time_t item;
    int size, bcnt;
    char buf[BRH_WINDOW];

    if (bstamp == *brh_tail) /* Thor.980904:����:�Ӫ��w�b brh_tail�W */
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
            size = item * sizeof(time32_t) + sizeof(BRH);

            if (bstamp == *head)
            {
                bcnt = item;
                memcpy(buf, head + 3, size - sizeof(BRH));
                tail = (time32_t *) ((char *) tail - size);
                if ((item = (char *) tail - (char *) head))
                    memmove(head, (char *) head + size, item);
                break;
            }
            head = (time32_t *) ((char *) head + size);
        }
    }

    brh_tail = tail = brh_alloc(tail, BRH_WINDOW);

    *tail++ = bstamp;
    *tail++ = bhno;

    if (bcnt)                   /* expand history list */
    {
        time32_t *list;

        size = bcnt;
        list = tail;
        head = (time32_t *) buf;

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
    const time32_t *head, *tail;
    time_t item;

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
    time32_t *list;

    list = brh_tail + 2;
    *list++ = 2;
    if (mode)
    {
        *list = mode;
    }
    else
    {
        time32(list);
    }
    *++list = mode;
}

void
brh_add(time_t prev, time_t chrono, time_t next)
{
    time32_t *base, *head, *tail;
    time_t item, final, begin;

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

    if (next >= begin && item > 0)
    {
        if (chrono < begin)
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
        /* [32, 30] [22, 10] */

        tail--;

        /* Thor.980923: how about [6, 7, 8] ? [15, 7]? */
        /* [6, 7, 8] ==> [32, 30] [15, 10] */  /* [32, 30] [15, 7] */
        if (head >= tail)
        {
            if (chrono < begin)
                head[-1] = chrono;
            return;
        }
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
/* �j�� user login �ɭ�Ū���Y���i�ݪO                    */
/* ----------------------------------------------------- */

/* cdchen.bbs@bbs.ntcic.edu.tw add by statue.000725 */
#ifdef  HAVE_FORCE_BOARD
void
force_board (void)
{
    BRD *brd;
    int bno;

    if (!cuser.userlevel)                /* guest ���L */
        return;
    bno = brd_bno(FORCE_BOARD);
    brd = bshm->bcache +  bno;
    brh_get(brd->bstamp, bno);

//  while (brd->blast > brd_visit[bno]) {
    while (brh_unread(brd->blast)) {
        vmsg("���s���i!! �Х��\\Ū���s���i��A���}..");
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
    const char *list)                 /* �O�D�GBM list */
{
    return str_has(list, cuser.userid);
}

#if     defined(HAVE_RESIST_WATER) || defined(HAVE_DETECT_CROSSPOST)
void
remove_perm(void)
{
    int i;
    for (i=0; i < COUNTOF(brd_bits); i++)
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

    if (!str_casecmp(bname, DEFAULT_BOARD))
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

    /* (moderated) ���K�ݪO�G�ֹ�ݪO���n�ͦW�� */

#ifdef HAVE_MODERATED_BOARD
    if (readlevel & PERM_ADMIN)
    {
        bits = 0;

        bits = bm_belong(bname);  /* Thor.980813: �ﯵ�K�ݪ��Ө�, �O���s�P�_�� */

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

    /* Thor.980813: ����: �S�O�� BM �Ҷq, bm ���Ӫ����Ҧ��v�� */

    blist = bhdr->BM;

    if ((ulevel & PERM_BM) && blist[0] > ' ' && is_bm(blist))
        return (BRD_R_BIT | BRD_W_BIT | BRD_X_BIT);

#ifndef HAVE_BOARD_SEE
    if (ulevel & PERM_ALLBOARD)
        bits |= (BRD_W_BIT | BRD_X_BIT);
#endif
    if (!str_casecmp(cuser.userid, ELDER))
        bits = BRD_R_BIT | BRD_W_BIT | BRD_X_BIT;

    return bits;
}


/* ----------------------------------------------------- */
/* ���J currboard �i��Y�z�]�w                           */
/* ----------------------------------------------------- */

GCC_PURE int
bstamp2bno(
    time_t stamp)
{
    const BRD *brd;
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

    int size;
    time32_t *base;
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
    /* �N .BRH ���J memory                                 */
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
    /* �h�O�d BRH_WINDOW ���B�@�Ŷ�                        */
    /* --------------------------------------------------- */

    /* brh_size = n = ((size + BRH_WINDOW) & -BRH_PAGE) + BRH_PAGE; */


    brh_size = n = size + BRH_WINDOW;
    brh_base = base = (time32_t *) malloc(n);

    if (size && ((n = open(fpath, O_RDONLY)) >= 0))
    {
        time32_t *head, *tail, *list;
        time_t bstamp, bhno;

        size = read(n, base, size);
        close(n);


        /* compact reading history : remove dummy/expired record */

        head = base;
        tail = (time32_t *) ((char *) base + size);
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

            /* �o�ӬݪO�s�b�B�S���Q zap ���B�i�H read */

            if (bhno >= 0 && (bits[bhno] & BRD_R_BIT))
            {

                bits[bhno] |= BRD_H_BIT;/* �w���\Ū�O�� */
                bstp[bhno] = head[1];   /* �W���\Ū�ɶ� */
                cbno = bhno;

                if (n > 0)
                {

#if 0
                    n = BMIN(n, BRH_MAX);
#endif

                    list += n;   /* Thor.980904: ����: �̫�@��tag */

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

                n = n * sizeof(time32_t) + sizeof(BRH);
                if (base != head)
                    memmove(base, head, n);
                base = (time32_t *) ((char *) base + n);
            }
            head += size;
        }
    }

    *base = 0;
    brh_tail = base;

    /* --------------------------------------------------- */
    /* �]�w default board                                  */
    /* --------------------------------------------------- */

    strcpy(currboard, "�|����w");
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
    time32_t *base, *head, *tail;
    time_t bhno;
    int size;
    BRD *bhdr, *bend;
    char *bits;

    /* Thor.980830: lkchu patch:  �٨S load �N���� save */
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
        size = head[2] * sizeof(time32_t) + sizeof(BRH);
        if (bhno >= 0 && !(bits[bhno] & BRD_Z_BIT))
        {
            if (base != head)
                memmove(base, head, size);
            base = (time32_t *) ((char *) base + size);
        }
        head = (time32_t *) ((char *) head + size);
    }

    /* save zap record */

    tail = brh_alloc(base, sizeof(time32_t) * MAXBOARD);

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
/* lkchu.981201: �ݪO�\Ū�O�� */
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

/* 081206.cache: �n�ͪO�ץ� */
bool
XoPostSimple(
    int bno)
{
    XO *xo;
    BRD *brd;
    int bits;
    char *str_bit, fpath[64];
    const char *str;

    str_bit = &brd_bits[bno];
    bits = *str_bit;

    /* cache.081206: ��n�ͬݪ����s�P�_�O���O�]�㦳 R �v�� */
    if (!(bits & BRD_R_BIT)/* && (bits & BRD_F_BIT)*/)
    {
        vmsg("���p���O�D�N�z�[�J�ݪO�n��");
        return false;
    }

    /* 090823.cache: �ݪO�H�� */
    if (currbno != bno)
    {
        /* �h�X�W�@�ӪO */
        if (currbno >= 0)
        {
            //����H���ܦ��t�ơF�t�ƪ����k�s
            bshm->mantime[currbno] = BMAX(bshm->mantime[currbno]-1, 0);
        }

        bshm->mantime[bno]++;       /* �i�J�s���O */

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

    if (!(bits & BRD_V_BIT))
        *str_bit = bits | BRD_V_BIT;

    brd_fpath(fpath, currboard, fn_dir);
    xz[XZ_POST - XO_ZONE].xo = xo = xo_get(fpath);
    xo->cb = post_cb;
    xo->recsiz = sizeof(HDR);
    xo->key = XZ_POST;
    xo->xyz = (void *) (brd->bvote > 0 ? (char *) "���ݪO�i��벼��" : brd->title + 3);
    str = brd->BM;
    if (*str <= ' ')
        str = "�x�D��";
    sprintf(currBM, "�O�D�G%s", str);

#ifdef LOG_BRD_USIES
    /* lkchu.981201: �\Ū�ݪ��O�� */
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

bool
XoPost(
    int bno)
{
    const bool res = XoPostSimple(bno);
    if (/*!(bits & BRD_V_BIT) && */(cuser.ufo2 & UFO2_BNOTE))
    {
        char fpath[64];
        brd_fpath(fpath, currboard, FN_NOTE);
        more(fpath, NULL);
    }
    return res;
}


/* ----------------------------------------------------- */
/* Class [�����s��]                                      */
/* ----------------------------------------------------- */

/* cache.090503: �Y�ɼ����ݪO */
static int
mantime_cmp(
    const void *a,
    const void *b)
{
    /* �Ѧh�ƨ�� */
    return bshm->mantime[* (const short *)b] - bshm->mantime[* (const short *)a];
}

static int class_flag2 = 0;  /* 1:�C�X�n��/���K�O�A�B�ۤv�S���\Ū�v���� */
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

    // "HOT/" �W�٥i�۩w�A�Y��W�]�n���K��᭱������ 4
    if (!strncmp((char *)cbase + cbase[chn], "HOT/", 4))
    {
        class_hot = 1;
        chn = 0;
    }

    chead = cbase + chn;

    pos = chead[0] + CH_TTSIZE;
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
/* cache.081207: �B�z���� board �n print */
/* thanks to gaod */
/* cache.090503: �Y�ɼ����ݪO */
/* cache.091225: �C�X�Ҧ����\Ū�v�����ݪO */

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

            // �Y�ɼ����ݪO�{�ɭȦ۩w
            if (class_hot)
            {
                if (bshm->mantime[chn] < CLASS_HOT) /* �u�C�X�H��W�L CLASS_HOT ���ݪO */
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

    // "HOT/" �W�٥i�۩w�A�Y��W�]�n���K��᭱������ 4
    if (!strncmp((char *)cbase + cbase[chn], "HOT/", 4))
    {
        class_hot = 1;
        chn = 0;
    }

    chead = cbase + chn;

    pos = chead[0] + CH_TTSIZE;
    max = chead[1];

    chead = (short *) ((char *) cbase + pos);
    ctail = (short *) ((char *) cbase + max);

    max -= pos;

    cbase = (short *) realloc(xo->xyz, max);
    xo->xyz = cbase;

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
/* cache.081207: �B�z���� board �n print */
/* thanks to gaod */
/* cache.090503: �Y�ɼ����ݪO */
/* cache.091225: �C�X�Ҧ����\Ū�v�����ݪO */
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
            // �Y�ɼ����ݪO�{�ɭȦ۩w
            if (class_hot)
            {
                if (bshm->mantime[chn] < CLASS_HOT) /* �u�C�X�H��W�L CLASS_HOT ���ݪO */
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

            if (st.st_mtime > brd->btime)  // �W���έp��A�ɮצ��ק�L
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
                {
                    brd->blast = 0;
                    brd->bpost = 0;
                }
            }

            close(fd);
        }
        if (brdnew)
            num = brd->bpost;
    }

    str = (!(bits[chn] & BRD_Z_BIT) && brd->blast > brd_visit[chn])
        ? (HAVE_UFO2_CONF(UFO2_MENU_LIGHTBAR) ? "\x1b[31m��\x1b[m" : "\x1b[1;31m��\x1b[m") : "��";

    char tmp[BTLEN + 1] = {0};
    int e_cols = (d_cols + 33 > BTLEN) ? BTLEN - 33 : d_cols;

    strcpy(tmp, brd->title);
    if (IS_DBCS_HI(tmp[e_cols + 32]))
//      tmp[e_cols + 33] = '\0';
//  else
        tmp[e_cols + 32] = ' ';
    tmp[e_cols + 33] = '\0';

/* 081122.cache:�ݪO�ʽ�, ���q�\, ���K, �n��, �@�� */
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

/* �B�z �H�� */ /* cache.20090416: ��ptt�ܦ�*/
    bno = bshm->mantime[chn];
    if (brd->bvote)
        str2 = "\x1b[1;33m  �� \x1b[m";
    else if (bno > 4999)
        str2 = "\x1b[1;32m TOP \x1b[m";
    else if (bno > 999)
        str2 = "\x1b[1;32m  �q \x1b[m";
    else if (bno > 799)
        str2 = "\x1b[1;35m  �q \x1b[m";
    else if (bno > 699)
        str2 = "\x1b[1;33m  �q \x1b[m";
    else if (bno > 599)
        str2 = "\x1b[1;42m  �z \x1b[m";
    else if (bno > 499)
        str2 = "\x1b[1;45m  �z \x1b[m";
    else if (bno > 449)
        str2 = "\x1b[1;44m  �z \x1b[m";
    else if (bno > 399)
        str2 = "\x1b[1;32m  �z \x1b[m";
    else if (bno > 349)
        str2 = "\x1b[1;35m  �z \x1b[m";
    else if (bno > 299)
        str2 = "\x1b[1;33m  �z \x1b[m";
    else if (bno > 249)
        str2 = "\x1b[1;36m  �z \x1b[m";
    else if (bno > 199)
        str2 = "\x1b[1;31m  �z \x1b[m";
    else if (bno > 149)
        str2 = "\x1b[1;37m  �z \x1b[m";
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
//�`�N���T��ť�, �]�� HOT �O�T�� char �G���ƪ�
//      prints("\x1b[%d;4%d;37m%6d%s%s%c%-*s \x1b[%sm%-4s %s%-33.32s%s%s%.13s", mode, mode?cuser.barcolor:0, num, str, mode ? "\x1b[37m" : "\x1b[m",

    sprintf(buf, "%d;3%d", (!brd->color) ? 1 : HAVE_UFO2_CONF(UFO2_MENU_LIGHTBAR) ? 0 : brd->color/10, brd->color%10);
//      prints("%6d%s%c%-*s \x1b[%sm%-4s \x1b[m%-36s%c %.13s", num, str,
//      prints("%6d%s%c%-*s \x1b[%sm%-4s \x1b[m%s%c %.13s", num, str,

//          brdtype, IDLEN, brd->brdname, buf, brd->class_, mode ? "\x1b[37m" : "\x1b[m", brd->title, brd->bvote ? "\x1b[1;33m  �� " : str2, mode ? "\x1b[37m" : "\x1b[m", brd->BM);

    prints("%6d%s%c%-*s \x1b[%sm%-4s \x1b[m%-*s%s", num, str, brdtype, IDLEN, brd->brdname, buf, brd->class_, d_cols + 33, tmp, str2);

    strcpy(tmp, brd->BM);
    if (IS_DBCS_HI(tmp[14]))
        tmp[13] = '\0';
    else
        tmp[14] = '\0';

    prints("%-14s\n", tmp);
}

void
class_outs(
    const char *title,
    int num)
{
    prints("%6d   %-*.*s     %.*s\n", num, IDLEN, IDLEN, title, d_cols + 53, title + IDLEN + 1);
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
class_cur(
    XO *xo,
    int pos)
{
    short *const chp = (short *) xo->xyz + pos;
    move(3 + pos - xo->top, 0);
    class_item(pos + 1, *chp);
    return XO_NONE;
}


static int
class_body(
    XO *xo)
{
    short *chp;
    int num, max, tail;

    move(3, 0);

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
            outs("\n�m�ݪO�C��n�ثe�S�����\n");
            clrtobot();
            return XO_NONE;
        }
        return XO_BODY;
    }

    chp = (short *) xo->xyz + xo->top;
    num = xo->top;
    tail = num + XO_TALL;
    max = BMIN(max, tail);

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
    prints(NECKBOARD, class_flag & UFO2_BRDNEW ? "�`��" : "�s��", d_cols + 33, "��   ��   ��   �z");

    return class_body(xo);
}

static int
class_head(
    XO *xo)
{
    vs_head("�ݪO�C��", str_site);
    return class_neck(xo);
}

static int
class_newmode(
    XO *xo)
{
    cuser.ufo2 ^= UFO2_BRDNEW;  /* Thor.980805: �S�O�`�N utmp.ufo���P�B���D */
    class_flag ^= UFO2_BRDNEW;
    return XO_NECK;
}

static int
class_help(
    XO *xo)
{
    film_out(FILM_CLASS, -1);
    return XO_HEAD;
}


static int
class_search(
    XO *xo,
    int pos)
{
    int num, max;
    char *ptr;
    char buf[IDLEN + 1];

    ptr = buf;
    pos = vget(B_LINES_REF, 0, MSG_BID, ptr, IDLEN + 1, DOECHO);
    move(b_lines, 0);
    clrtoeol();

    if (pos)
    {
        short *chp, chn;
        BRD *bcache, *brd;

        bcache = bshm->bcache;

        str_lower(ptr, ptr);
        num = pos;
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
                if (str_casestr(brd->brdname, ptr) || str_casestr(brd->title, ptr))
                    return XO_MOVE + pos;
            }
        } while (pos != num);
    }

    return XO_NONE;
}

/* cache.091125: �u�C�X�㦳�\Ū�v�������K/�n�ͬݪO */
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
        zmsg("�䤣��iŪ�����K/�n�ͬݪO");
        class_flag2 ^= 0x01;
        class_load(xo);
        xo->pos = pos;
        return XO_FOOT;
    }
    return XO_HEAD;
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
        zmsg("�䤣�쥼�Q zap �����ݪO");
        class_flag |= BFO_YANK;
        class_load(xo);
        xo->pos = pos;
        return XO_FOOT;
    }
    return XO_HEAD;
}

static int
class_zap(
    XO *xo,
    int pos)
{
    BRD *brd;
    short *chp;
    int num, chn;

    num = pos;
    chp = (short *) xo->xyz + num;
    chn = *chp;
    if (chn >= 0)
    {
        brd = bshm->bcache + chn;
        if (!(brd->battr & BRD_NOZAP) || (brd_bits[chn] & BRD_Z_BIT))
        {
            brd_bits[chn] ^= BRD_Z_BIT;
            return XO_CUR;
        }
    }

    return XO_NONE;
}


static int
class_edit(
    XO *xo,
    int pos)
{
    /* if (HAS_PERM(PERM_ALLBOARD)) */
    /* Thor.990119: �u�������i�H�ק� */
    if (HAS_PERM(PERM_SYSOP) || HAS_PERM(PERM_BOARD))
    {
        short *chp;
        int chn;

        chp = (short *) xo->xyz + pos;
        chn = *chp;
        if (chn >= 0)
        {
            brd_edit(chn);
            class_load(xo);
            return XO_BODY;
        }
    }
    return XO_NONE;
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
    return XO_HEAD;
}

#ifdef AUTHOR_EXTRACTION
/* Thor.0818: �Q�令�H�ثe���ݪ��C��Τ����ӧ�, ���n����� */

/* opus.1127 : �p�e���g, �i extract author/title */

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

    if (!HAS_PERM(PERM_VALID))
        return XO_NONE;

    if (!vget(B_LINES_REF, 0, "�п�J�@�̡G", author, IDLEN + 1, DOECHO))
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
        if ((chn = *chead++) >= 0)      /* Thor.0818: ���� group */
        {
            /* Thor.0701: �M����w�@�̤峹, ���h����m, �é�J */

            int fsize;
            char *fimage;
            char folder[80];
            HDR *head, *tail;

            sprintf(folder, "�m�M����w�@�̡n�ݪ��G%s \x1b[5m...\x1b[m",
                brd[chn].brdname);
            outz(folder);
            refresh();
            brd_fpath(folder, brd[chn].brdname, fn_dir);

            fimage = f_map(folder, &fsize);

            if (fimage == (char *) -1)
                continue;

            head = (HDR *) fimage;
            tail = (HDR *) (fimage + fsize);

            while (head <= --tail)
            {
                if (tail->xmode & (POST_CANCEL | POST_DELETE | POST_MDELETE | POST_LOCK))
                    continue;

                /* if (str_casestr(temp, author)) *//* Thor.0818:�Ʊ����� */

                if (!str_ncasecmp(tail->owner, author, len))
                {
                    XO *xt = xo_get(folder);
                    xt->pos = tail - head;
                    xt->cb = post_cb;
                    xt->recsiz = sizeof(HDR);
                    chp[tag++] = chn;
                    break;
                }
            }

            munmap(fimage, fsize);
        }
    } while (chead < ctail);

    if (!tag)
    {
        free(chp);
        vmsg("�ŵL�@��");
        return XO_FOOT;
    }

    xo_a.cb = class_cb;
    xo_a.recsiz = sizeof(short);
    xo_a.pos = xo_a.top = 0;
    xo_a.max = tag;
    xo_a.key = 1;                       /* all boards */
    xo_a.xyz = chp;

    xoTmp = xz[XZ_CLASS - XO_ZONE].xo;  /* Thor.0701: �O�U��Ӫ�class_xo */

    xz[XZ_CLASS - XO_ZONE].xo = &xo_a;
    xover(XZ_CLASS);

    free(chp);

    xz[XZ_CLASS - XO_ZONE].xo = xoTmp;          /* Thor.0701: �٭� class_xo */

    return XO_BODY;
}
#endif  // #if 0  // Unused
#endif  /* #ifdef AUTHOR_EXTRACTION */

#if defined(HAVE_COUNT_BOARD) && 0
static int
class_stat(
    XO *xo,
    int pos)
{
    BRD *brd;
    short *chp;
    int num, chn;
    char msg[80];

    num = pos;
    chp = (short *) xo->xyz + num;
    chn = *chp;
    if (chn >= 0)
    {
        brd = bshm->bcache + chn;
        sprintf(msg, "�ثe�ֿn�\\Ū�ơG%d�A�ֿn�o��ơG%d", brd->n_reads, brd->n_posts);
        pmsg(msg);
    }

    return XO_NONE;
}
#endif

static int
class_visit(
    XO *xo,
    int pos)
{
    short *chp;
    int chn;

    chp = (short *) xo->xyz + pos;
    chn = *chp;
    if (chn >= 0)
    {
        BRD *brd;
        brd = bshm->bcache + chn;
        brh_get(brd->bstamp, chn);
        brh_visit(0);
        time(&brd_visit[chn]);
    }
    return XO_BODY;
}

static int class_browse(XO *xo, int pos);

static KeyFuncList class_cb =
{
    {XO_INIT, {class_head}},
    {XO_LOAD, {class_body}},
    {XO_HEAD, {class_head}},
    {XO_NECK, {class_neck}},
    {XO_BODY, {class_body}},
    {XO_CUR | XO_POSF, {.posf = class_cur}},

    {'r' | XO_POSF, {.posf = class_browse}},
    {'/' | XO_POSF, {.posf = class_search}},
    {'c', {class_newmode}},

    {'s', {class_switch}},

    {'y', {class_yank}},
    {'i', {class_yank2}}, //�C�X�Ҧ����\Ū�v�������K/�n�ͬݪO
    {'z' | XO_POSF, {.posf = class_zap}},
    {'E' | XO_POSF, {.posf = class_edit}},
    {'v' | XO_POSF, {.posf = class_visit}},
#ifdef  HAVE_COUNT_BOARD
    {'S' | XO_POSF | XO_DL, {.dlposf = DL_NAME("brdstat.so", main_bstat)}},
#endif

#ifdef  HAVE_FAVORITE
    {'a' | XO_POSF, {.posf = class_add}},
#endif

#ifdef AUTHOR_EXTRACTION
    //{'A', {XoAuthor}},
#endif

    {'h', {class_help}}
};


static int
XoClass(
    int chn)
{
    XO xo, *xt;

    /* Thor.980727: �ѨM XO xo�����T�w��,
                    class_load�����| initial xo.max, ��L���T�w */
    xo.cb = class_cb;
    xo.recsiz = sizeof(short);
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

static int
class_browse(
    XO *xo,
    int pos)
{
    short *chp;
    int chn;

    chp = (short *) xo->xyz + pos;
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

    return XO_HEAD;     /* Thor.0701: �L�k�M�֤@�I, �]�� XoPost */
}

int
Class(void)
{
    boardmode = 0;
    if (!class_img || XoClass(CH_END-1) == XO_NONE)
    {
        vmsg("���w�q���հQ�װ�");
        return XEASY;
    } /* Thor.980804: ���� ���� account �y�X class.img �ΨS�� class�����p */
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
            brd->btime = time(0) + 45;        /* 45 �������������ˬd */
            if ((fsize = st.st_size) >= sizeof(HDR))
            {
                brd->bpost = fsize / sizeof(HDR);
                lseek(fd, fsize - sizeof(HDR), SEEK_SET);
                read(fd, &brd->blast, sizeof(time32_t));
            }
            else
            {
                brd->blast = 0;
                brd->bpost = 0;
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
        vmsg("���w�q���հQ�װ�");
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

    /* �p�G�S�� .BRD �ɡA�Τj�p���s�ɡA�N run/class.img ����
       �o�ˤ~���|�i���N���I visor.000705 */
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

    board_xo.cb = class_cb;
    board_xo.recsiz = sizeof(short);
    board_xo.key = CH_END;
    if (class_img)
        class_load(&board_xo);

    xz[XZ_CLASS - XO_ZONE].xo = &board_xo;      /* Thor: default class_xo */
}

int
Boards(void)
{
    /* class_xo = &board_xo; *//* Thor: �w�� default, ���ݧ@�� */
    boardmode = -1;
    xover(XZ_CLASS);
    return 0;
}

#ifdef HAVE_MULTI_CROSSPOST
#define MSG_CC "\x1b[32m[�ݪO�s�զW��]\x1b[m\n"

int
brd_list(
    int reciper)
{
    LIST list;
    int userno, fd, select;
    char buf[32], fpath[64], msg[128], temp[30];
    BRD brd, *ptr;
    sprintf(msg, "A)�W�[ D)�R�� B)�����ݪ� C)���� G)���� [1~%d]�s�զW�� M)�w�� Q)�����H[M]", MAX_LIST);

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
        case 'c':
            if (!userno && vget(B_LINES_REF, 0, "�����G", buf, 16, DOECHO))
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
                            (select == 'g' && (str_casestr(brd.brdname, buf) || str_casestr(brd.title, buf)))||
                            (select == 'c' && str_casestr(brd.class_, buf))))
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
