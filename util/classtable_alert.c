/*-------------------------------------------------------*/
/* util/classtable_alert.c	( YZU WindTopBBS Ver 3.00 )	 */
/*-------------------------------------------------------*/
/* target : 		 	 			 */
/* create : 					 	 */
/* update : 					 	 */
/*-------------------------------------------------------*/


//           int tm_sec;     /* seconds (0 - 60) */
//           int tm_min;     /* minutes (0 - 59) */
//           int tm_hour;    /* hours (0 - 23) */
//           int tm_mday;    /* day of month (1 - 31) */
//           int tm_mon;     /* month of year (0 - 11) */
//           int tm_year;    /* year - 1900 */
//           int tm_wday;    /* day of week (Sunday = 0) */
//           int tm_yday;    /* day of year (0 - 365) */
//           int tm_isdst;   /* is summer time in effect? */
//           char *tm_zone;  /* abbreviation of timezone name */
//           long tm_gmtoff; /* offset from UTC in seconds */

#include <sys/shm.h>
#include "bbs.h"

static UCACHE *ushm;
static CLASS_TABLE_ALERT *cache;
static int cache_size;

static void *
attach_shm(
    int shmkey, int shmsize)
{
    void *shmptr;
    int shmid;

    shmid = shmget(shmkey, shmsize, 0);
    shmsize = 0;
    shmptr = (void *) shmat(shmid, NULL, 0);

    return shmptr;
}

static CLASS_TABLE_ALERT *
bfind(
    int userno)
{
    int count;
    CLASS_TABLE_ALERT *tmpcache,*datum;

    if ((tmpcache = cache))
    {
        for (count = 0; count < cache_size;count++)
        {
            datum = tmpcache + count;
            if (userno == datum->userno)
                return datum;
        }
    }
    return NULL;
}

int
bsend(
    UTMP *callee,
    BMW *bmw)
{
    BMW *mpool, *mhead, *mtail, **mslot;
    int i;
    pid_t pid;
    time_t texpire;

    if ((pid = callee->pid) <= 0)
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

    texpire = time(&bmw->btime) - BMW_EXPIRE;

    mpool = ushm->mpool;
    mhead = ushm->mbase;
    if (mhead < mpool)
        mhead = mpool;
    mtail = mpool + BMW_MAX;

    do
    {
        if (++mhead >= mtail)
            mhead = mpool;
    } while (mhead->btime > texpire);

    *mhead = *bmw;
    ushm->mbase = mslot[i] = mhead;

    /* sem_lock(BSEM_LEAVE); */
    return kill(pid, SIGUSR2);
}


void
bedit(
    UTMP *up,
    BMW *bmw)
{

    bmw->recver = up->userno;	/* 先記下 userno 作為 check */

    bmw->caller = NULL;
    bmw->sender = 0;
    strcpy(bmw->userid, "系統報時者");

    bsend(up, bmw);
}


static void
init(void)
{
    UTMP *up, *uceil;
    int userno;
    BMW bmw;
    CLASS_TABLE_ALERT *ptr;
    time_t now;
    struct tm *p;


    up = ushm->uslot;
    uceil = (void *) up + ushm->offset;

    do
    {
        userno = up->userno;
        if (up->pid <= 0)
            continue;
        if (!(up->ufo & UFO_CLASSTABLE))
            continue;
//      printf("%-13.13s  %6.6d  %6.6d\n",up->userid,up->userno,up->pid);
        if((ptr=bfind(up->userno)))
        {
            //strcpy(bmw.msg,"訊息測試,不便請見諒");

            time(&now);
            p = localtime(&now);
            if(p->tm_wday >= 1 && p->tm_hour >= 8 && p->tm_hour<=20)
            {
                if(ptr->item[(p->tm_wday-1)*13 + (p->tm_hour - 8)].used)
                {
                    CLASS_TABLE_ALERT_ITEM *cur;
                    cur = &(ptr->item[(p->tm_wday-1)*13 + (p->tm_hour - 8)]);
//		            cur = &(ptr->item[i]);
                    if(strlen(cur->room))
                    {
                        sprintf(bmw.msg, "◎您在 %-.16s 有一門 [%s] 課，上課不要遲到哦!!",cur->room,cur->condensation);
                        printf("%s:%s\n",up->userid,bmw.msg);
                    }
                    else
                    {
                        sprintf(bmw.msg, "◎您有一門 [%s] 課，上課不要遲到哦!!",cur->condensation);
                        printf("%s:%s\n",up->userid,bmw.msg);

                    }
                bedit(up,&bmw);
                }
            }
        }
    } while (++up <= uceil);

}
/*
static int
int1_cmp(
    CLASS_TABLE_ALERT **i, CLASS_TABLE_ALERT **j)
{
    return (*i)->userno - (*j)->userno;
}
*/
UTMP *
utmp_find(
    int userno)
{
    UTMP *uentp, *uceil;

    uentp = ushm->uslot;
    uceil = (void *) uentp + ushm->offset;
    do
    {
        if (uentp->userno == userno)
            return uentp;
    } while (++uentp <= uceil);

    return NULL;
}


void
bcache(
    char *fpath)
{
    int fd, size=0;
    struct stat st;
    int userno;

    if ((fd = open(fpath, O_RDWR, 0600)) < 0)
        return;


    if (!fstat(fd, &st) && (size = st.st_size) > 0)
    {
        CLASS_TABLE_ALERT *pbase, *phead, *ptail;

        pbase = phead = (CLASS_TABLE_ALERT *) malloc(size);
        size = read(fd, pbase, size);
                //printf("2bcache:%d\n",size);
        if (size >= sizeof(CLASS_TABLE_ALERT))
        {
            ptail = (CLASS_TABLE_ALERT *) ((char *) pbase + size);
        //  printf("2bcache\n");
            while (phead < ptail)
            {
                userno = phead->userno;
                if (utmp_find(userno))
                {
                    //printf("3bcache\n");
                    phead++;
                    continue;
                }
                //printf("4bcache\n");
                ptail--;
                if (phead >= ptail)
                    break;
                memcpy(phead, ptail, sizeof(CLASS_TABLE_ALERT));
            }

            size = (char *) ptail - (char *) pbase;
            if (size > 0)
            {
                //printf("6bcache:%d\n",size);

//		        if (size > sizeof(CLASS_TABLE_ALERT))
//			        xsort(pbase, size / sizeof(CLASS_TABLE_ALERT), sizeof(CLASS_TABLE_ALERT), int1_cmp);
                //printf("5bcache\n");

                lseek(fd, 0, SEEK_SET);
                write(fd, pbase, size);
                ftruncate(fd, size);
            }
            //printf("1bcache\n");
            cache = pbase;
            cache_size = size / sizeof(CLASS_TABLE_ALERT);
        }
    }
    close(fd);

    if (size <= 0)
    {
        unlink(fpath);
        cache = NULL;
        cache_size = 0;
    }
 // printf("bcache\n");
}

int
main(
    int argc,
    char *argv[])
{
    char fpath[128];

    chdir(BBSHOME);

    ushm = attach_shm(UTMPSHM_KEY, sizeof(UCACHE));

    if(!ushm)
    {
        exit(1);
    }


    sprintf(fpath,"%s.new",FN_CLASSTABLE_DB);
    f_mv(FN_CLASSTABLE_DB, fpath);
    bcache(fpath);


    init();

    if(cache)
        free(cache);
    f_cp(fpath, FN_CLASSTABLE_DB, O_APPEND);
    unlink(fpath);
    return 0;
}
