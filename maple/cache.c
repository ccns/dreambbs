/*-------------------------------------------------------*/
/* cache.c      ( NTHU CS MapleBBS Ver 3.02 )            */
/*-------------------------------------------------------*/
/* target : cache up data by shared memory               */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/


#include "bbs.h"

#include <sys/ipc.h>
#include <sys/shm.h>


/* IID.2021-02-25: For functions independent of bbsd globals, please refer to lib/shm.c */


#ifdef  HAVE_SEM
#include <sys/sem.h>

/* ----------------------------------------------------- */
/* semaphore : for critical section                      */
/* ----------------------------------------------------- */


static int ap_semid;


void
sem_init(void)
{
    int semid;

    union semun
    {
        int val;
        struct semid_ds *buf;
        unsigned short *array;
    }     arg =
    {
        1
    };

    semid = semget(BSEM_KEY, 1, 0);
    if (semid == -1)
    {
        semid = semget(BSEM_KEY, 1, IPC_CREAT | BSEM_FLG);
        if (semid == -1)
            attach_err(BSEM_KEY, "semget");
        semctl(semid, 0, SETVAL, arg);
    }
    ap_semid = semid;
}

static void
sem_lock(
    int op)                     /* op is BSEM_ENTER or BSEM_LEAVE */
{
    struct sembuf sops;

    sops.sem_num = 0;
    sops.sem_flg = SEM_UNDO;
    sops.sem_op = op;
    semop(ap_semid, &sops, 1);
}

#endif  /* HAVE_SEM */


/*-------------------------------------------------------*/
/* .UTMP cache                                           */
/*-------------------------------------------------------*/


UCACHE *ushm;


#ifndef _BBTP_


void
utmp_mode(
    int mode)
{
    if (bbsmode != mode)
    {

#ifdef MODE_STAT
        if (mode != M_XMODE && bbsmode != M_XMODE) { /* XMODE 被拿來做其它雜項, 不列入統計 */
            time_t now;

            time(&now);
            modelog.used_time[bbsmode] += (now - mode_lastchange);
            mode_lastchange = now;
        }
#endif

        cutmp->mode = bbsmode = mode;

    }
}


int
utmp_new(
    const UTMP *up)
{
    UCACHE *xshm;
    UTMP *uentp, *utail;

    /* --------------------------------------------------- */
    /* semaphore : critical section                        */
    /* --------------------------------------------------- */

#ifdef  HAVE_SEM
    sem_lock(BSEM_ENTER);
#endif

    xshm = ushm;

#ifdef  HAVE_MAXRESERVE
    if (HAS_PERM(PERM_ADMIN))
    {
#endif
        uentp = xshm->uslot;
        utail = uentp + MAXACTIVE;
#ifdef  HAVE_MAXRESERVE
    }
    else
    {
        uentp = xshm->uslot + MAX_RESERVE;
        utail = uentp + (MAXACTIVE - MAX_RESERVE);
    }
#endif

    /* uentp += (up->pid % xshm->count);        *//* hashing */

    do
    {
        if (!uentp->pid && !uentp->userno)
        {
            unsigned int offset;

            offset = (char *) uentp - (char *) xshm->uslot;
            memcpy(uentp, up, sizeof(UTMP));
            xshm->count++;
            if (xshm->offset < offset)
                xshm->offset = offset;
            cutmp = uentp;

#ifdef  HAVE_SEM
            sem_lock(BSEM_LEAVE);
#endif

            return 1;
        }
    } while (++uentp < utail);

    /* Thor:告訴user有人登先一步了 */

#ifdef  HAVE_SEM
    sem_lock(BSEM_LEAVE);
#endif

    return 0;
}


void
utmp_free(void)
{
    UTMP *uentp;

    uentp = cutmp;
    if (!uentp || !uentp->pid)
        return;

#ifdef  HAVE_SEM
    sem_lock(BSEM_ENTER);
#endif

    memset(uentp, 0, sizeof(UTMP));
/*  uentp->pid = uentp->userno = 0;*/
    ushm->count--;

#ifdef  HAVE_SEM
    sem_lock(BSEM_LEAVE);
#endif
}


GCC_PURE UTMP *
utmp_find(
    int userno)
{
    UTMP *uentp, *uceil;

    uentp = ushm->uslot;
    uceil = (UTMP *) ((char *) uentp + ushm->offset);
    do
    {
        if (uentp->userno == userno)
            return uentp;
    } while (++uentp <= uceil);

    return NULL;
}

GCC_PURE UTMP *
pid_find(
    int pid)
{
    UTMP *uentp, *uceil;

    uentp = ushm->uslot;
    uceil = (UTMP *) ((char *) uentp + ushm->offset);
    do
    {
        if (uentp->pid == pid && uentp->pid != 0)
            return uentp;
    } while (++uentp <= uceil);

    return NULL;
}


#if 0
int
apply_ulist(
    int (*fptr) (user_info *uentp))
{
    UTMP *uentp;
    int i, state;

    uentp = ushm->uslot;
    for (i = 0; i < USHM_SIZE; i++, uentp++)
    {
        if (uentp->pid)
            if (state = (*fptr) (uentp))
                return state;
    }
    return 0;
}


UTMP *
utmp_search(
    int userno,
    int order)                  /* 第幾個 */
{
    UTMP *uentp, *uceil;

    uentp = ushm->uslot;
    uceil = (UTMP *) ((char *) uentp + ushm->offset);
    do
    {
        if (uentp->userno == userno)
        {
            if (--order <= 0)
                return uentp;
        }
    } while (++uentp <= uceil);
    return NULL;
}
#endif  /* #if 0 */


int
utmp_count(
    int userno,
    int show)
{
    UTMP *uentp, *uceil;
    int count;

    count = 0;
    uentp = ushm->uslot;
    uceil = (UTMP *) ((char *) uentp + ushm->offset);
    do
    {
        if (uentp->userno == userno)
        {
            count++;
            if (show)
            {
                prints("(%d) 目前狀態為: %-17.16s(來自 %s)\n",
                    count, bmode(uentp, 0), uentp->from);
            }
        }
    } while (++uentp <= uceil);
    return count;
}

#if 1 && defined(HAVE_CLASSTABLEALERT)
GCC_PURE int
cmpclasstable(
    const void *ptr)
{
    return ((const CLASS_TABLE_ALERT *)ptr)->userno == cuser.userno;
}

void
classtable_free(void)
{
    int pos;
    while ((pos = rec_loc(FN_CLASSTABLE_DB, sizeof(CLASS_TABLE_ALERT), cmpclasstable)) >= 0)
        rec_del(FN_CLASSTABLE_DB, sizeof(CLASS_TABLE_ALERT), pos, cmpclasstable, NULL);
}

void
classtable_main(void)
{
    int fd, size=0;
    struct stat st;
    int i;
    char fpath[128];
    CLASS_TABLE_ALERT tmp;

    memset(&tmp, 0, sizeof(CLASS_TABLE_ALERT));

    tmp.userno = cuser.userno;

    usr_fpath(fpath, cuser.userid, FN_CLASSTABLE2);

    if ((fd = open(fpath, O_RDWR, 0600)) < 0)
        return;

    if (!fstat(fd, &st) && (size = st.st_size) > 0)
    {
        CLASS_TABLE2 *pbase;

        pbase = (CLASS_TABLE2 *) malloc(size);
        size = read(fd, pbase, size);
        for (i=0; i<78; i++)
        {
            if (pbase[i].valid)
            {
                strcpy(tmp.item[i].condensation, pbase[i].condensation);
                strcpy(tmp.item[i].room, pbase[i].room);
                tmp.item[i].used = 1;
            }
        }
        rec_add(FN_CLASSTABLE_DB, &tmp, sizeof(CLASS_TABLE_ALERT));


        free(pbase);
    }
    close(fd);

}
#endif

/*-------------------------------------------------------*/
/* .BRD cache                                            */
/*-------------------------------------------------------*/


BCACHE *bshm;


#if 0
void
bsync(void)
{
    rec_put(FN_BRD, bshm->bcache, sizeof(BRD) * bshm->number, 0);
}
#endif


#if 0
int
apply_boards(
    int (*func) (BRD *bhdr))
{
    BRD *bhdr;
    int i;

    for (i = 0, bhdr = bshm->bcache; i < bshm->number; i++, bhdr++)
    {
        if (brd_bits[i])
        {
            if ((*func) (bhdr) == -1)
                return -1;
        }
    }
    return 0;
}
#endif


GCC_PURE int
brd_bno(
    const char *bname)
{
    const BRD *brdp, *bend;
    int bno;

    brdp = bshm->bcache;
    bend = brdp + bshm->number;
    bno = 0;

    do
    {
        if (!str_casecmp(bname, brdp->brdname))
            return bno;

        bno++;
    } while (++brdp < bend);

    return -1;
}


#if 0
BRD *
getbrd(
    char *bname)
{
    BRD *bhdr, *tail;

    bhdr = bshm->bcache;
    tail = bhdr + bshm->number;
    do
    {
        if (!str_casecmp(bname, bhdr->brdname))
            return bhdr;
    } while (++bhdr < tail);
    return NULL;
}
#endif

#ifdef  HAVE_OBSERVE_LIST
/*-------------------------------------------------------*/
/* etc/observe.db cache                                  */
/*-------------------------------------------------------*/
OCACHE *oshm;


GCC_PURE int
observeshm_find(
    int userno)
{
    unsigned int *cache, datum;
    int count, mid;

    if ((cache = oshm->userno))
    {
        for (count = oshm->total; count > 0;)
        {
            datum = cache[mid = count >> 1];
            if (userno == datum)
                return 1;
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
    return 0;
}
#endif  /* #ifdef  HAVE_OBSERVE_LIST */

/*-------------------------------------------------------*/
/* run/var/counter cache                                 */
/*-------------------------------------------------------*/
COUNTER *countshm;

void
count_update(void)
{
    if ((ushm->count) > (countshm->samehour_max_login))
    {
        countshm->samehour_max_login = ushm->count;
        countshm->samehour_max_time = time(0);
    }
    countshm->cur_hour_max_login++;
    countshm->cur_day_max_login++;
}

/*-------------------------------------------------------*/
/* etc/banmail cache                                     */
/*-------------------------------------------------------*/
FWCACHE *fwshm;

/*-------------------------------------------------------*/
/* etc/movie cache                                       */
/*-------------------------------------------------------*/


FCACHE *fshm;


static inline void
out_rle(
    char *str,
    bool is_movie)
{
#ifdef SHOW_USER_IN_TEXT
    char *t_name = cuser.userid;
    char *t_nick = cuser.username;
#endif
    int y/*, count=0*/;
    int cc, rl;

    if (is_movie)
        y = 1;
        //move(3, 36+item_length[count++]);
    else
        getyx(&y, &SINKVAL(int));

    move(y, d_cols >> 1/*item_length[count++]*/);
    while ((cc = (unsigned char) *str))
    {
        str++;
        switch (cc)
        {
        case 8:  /* Thor.980804: 註解一下, opus壓縮過了 */
            rl = *str++;
            cc = *str++;

            while (--rl >= 0)
            {
                if (cc=='\n')
                {
                    if (is_movie)
                    {
                        outs("\x1b[m");
                        clrtoeol();
                    }
                    move(++y, d_cols >> 1/*item_length[count++]*/);
                }
                else
                    outc(cc);

            }
            continue;

#ifdef SHOW_USER_IN_TEXT
        case 1:
            if ((cc = (unsigned char) *t_name) && (cuser.ufo2 & UFO2_SHOWUSER))
                t_name++;
            else
                cc = (cuser.ufo2 & UFO2_SHOWUSER) ? ' ': '#';
            break;

        case 2:
            if ((cc = (unsigned char) *t_nick) && (cuser.ufo2 & UFO2_SHOWUSER))
                t_nick++;
            else
                cc = (cuser.ufo2 & UFO2_SHOWUSER) ? ' ' : '%';
#endif
        }
        if (cc=='\n')
        {
            getyx(&y, &SINKVAL(int));
            if (is_movie)
            {
                outs("\x1b[m");
                clrtoeol();
            }
            move(++y, d_cols >> 1/*item_length[count++]*/);

        }
        else
            outc(cc);
    }
#ifndef M3_USE_PFTERM
    // XXX(IID.20190415): Workaround for broken visio `move()`.
    //    Without the workaround, the next line after the film will be offset
    clrtoeol();
#endif
/*  while (count>=0) item_length[count--]=0;*/
}


int
film_out(
    int tag,
    int row)                    /* -1 : help */
{
    int fmax, len, *shot;
    char *film, buf[FILM_SIZ];
    bool is_movie = (tag >= FILM_MOVIE);

    if (row <= 0)
        clear();
    else
        move(row, 0);

    len = 0;
    shot = fshm->shot;
    film = fshm->film;

    while (!(fmax = *shot))     /* util/camera.c 正在換片 */
    {
        sleep(5);
        if (++len > 10)
            return FILM_MOVIE;
    }

    if (is_movie)               /* random select */
    {
        tag += (time(0) % 8U);  /* 7 steps forward */
        if (tag >= fmax)
            tag = FILM_MOVIE;
    }    /* Thor.980804: 可能是故意的吧? 第一張 random select前八個其中一個 */

    if (tag)
    {
        len = shot[tag];
        film += len;
        len = shot[++tag] - len;
    }
    else
    {
        len = shot[1];
    }

    if (len >= FILM_SIZ - 10)
        return tag;

    memcpy(buf, film, len);
    buf[len] = '\0';

    out_rle(buf, is_movie);

    if (row < 0)                  /* help screen */
        vmsg(NULL);

    return tag;
}

GCC_PURE UTMP *
utmp_check(        /* 檢查使用者是否在站上 */
    const char *userid)
{
    UTMP *uentp, *uceil;

    uentp = ushm->uslot;
    uceil = (UTMP *) ((char *) uentp + ushm->offset);
    do
    {
        if (uentp->pid)
        {           /* 已經離站的不檢查 */
            if (!strcmp(userid, uentp->userid)
                  && (!(uentp->ufo & UFO_CLOAK) || HAS_PERM(PERM_SEECLOAK))
                /*&& ((can_see(uentp) != 2) || HAS_PERM(PERM_ACCOUNTS))*/)
                return uentp;
        }
    } while (++uentp <= uceil);

    return NULL;
}

#endif  /* _BBTP_ */
