/*-------------------------------------------------------*/
/* util/brdstat.c	( YZU CSE WindTop BBS 3.02 )	 */
/*-------------------------------------------------------*/
/* target : 看板資訊統計				 */
/* create : 2003/08/17				 	 */
/* update : 					 	 */
/*-------------------------------------------------------*/
/* syntax : brdstat					 */
/*-------------------------------------------------------*/


#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/ipc.h>
#include <sys/shm.h>


#include "bbs.h"

#ifdef	HAVE_COUNT_BOARD


#define	MAX_LINE	16
#define	ADJUST_M	10	/* adjust back 10 minutes */

#define	YEAR_HOUR	(365 * 24)
#define HALFYEAR_HOUR	(180 * 24)
#define	THREEMONTH_HOUR	(90 * 24)
#define	MONTH_HOUR	(30 * 24)
#define	TWOWEEK_HOUR	(14 * 24)
#define	WEEK_HOUR	(7 * 24)
#define	DAY_HOUR	(1 * 24)
#define	HOUR		(1)


HDR		hotboard[MAX_HOTBOARD];
int		hotcount;


void
keeplog(
    char *fnlog,
    char *board,
    char *title,
    int mode)		/* 0:load 1: rename  2:unlink 3:mark*/
{
    HDR hdr;
    char folder[128], fpath[128];
    int fd;
    FILE *fp;

    if (!board)
        board = BRD_SYSTEM;

    sprintf(folder, "brd/%s/.DIR", board);
    fd = hdr_stamp(folder, 'A', &hdr, fpath);
    if (fd < 0)
        return;

    if (mode == 1 || mode == 3)
    {
        close(fd);
        /* rename(fnlog, fpath); */
        f_mv(fnlog, fpath); /* Thor.990409:可跨partition */
    }
    else
    {
        fp = fdopen(fd, "w");
        fprintf(fp, "作者: SYSOP (" SYSOPNICK ")\n標題: %s\n時間: %s\n",
            title, ctime(&hdr.chrono));
        f_suck(fp, fnlog);
        fclose(fp);
        close(fd);
        if (mode)
            unlink(fnlog);
    }
    if (mode == 3)
        hdr.xmode |= POST_MARKED;

    strcpy(hdr.title, title);
    strcpy(hdr.owner, "SYSOP");
    strcpy(hdr.nick, SYSOPNICK);
    fd = open(folder, O_WRONLY | O_CREAT | O_APPEND, 0600);
    if (fd < 0)
    {
        unlink(fpath);
        return;
    }
    write(fd, &hdr, sizeof(HDR));
    close(fd);
}


/* ----------------------------------------------------- */
/* 開票：shm 部份須與 cache.c 相容			 */
/* ----------------------------------------------------- */


static BCACHE *bshm;


static void
attach_err(
    int shmkey,
    char *name)
{
    fprintf(stderr, "[%s error] key = %x\n", name, shmkey);
    exit(1);
}


static void *
attach_shm(
    register int shmkey, register int shmsize)
{
    register void *shmptr;
    register int shmid;

    shmid = shmget(shmkey, shmsize, 0);
    if (shmid < 0)
    {
        shmid = shmget(shmkey, shmsize, IPC_CREAT | 0600);
        if (shmid < 0)
            attach_err(shmkey, "shmget");
    }
    else
    {
        shmsize = 0;
    }

    shmptr = (void *) shmat(shmid, NULL, 0);
    if (shmptr == (void *) -1)
        attach_err(shmkey, "shmat");

    if (shmsize)
        memset(shmptr, 0, shmsize);

    return shmptr;
}


/* static */
void
bshm_init(void)
{
    register BCACHE *xshm;

    xshm = bshm;
    if (xshm == NULL)
    {
        bshm = xshm = attach_shm(BRDSHM_KEY, sizeof(BCACHE));
    }

    if (bshm->uptime < 0)
        exit(1);

}


/* ----------------------------------------------------- */

static void
add_log(
    BSTAT *des,
    BSTAT *src)
{
    des->n_reads += src->n_reads;
    des->n_posts += src->n_posts;
    des->n_news += src->n_news;
    des->n_bans += src->n_bans;
}

static void
save_hot(void)
{
    int i, fd;
    for (i=0; i<hotcount; i++)
        hotboard[i].xid = 0;
    fd = open(FN_HOTBOARD, O_WRONLY);
    write(fd, hotboard, sizeof(HDR)*hotcount);
    close(fd);

}

static void
count_hot(
    BRD *brd)
{
    int i;
    for (i=0; i<hotcount; i++)
    {
        if (brd->n_reads >= hotboard[i].xid)
            break;
    }
    if (i < MAX_HOTBOARD)
    {
        if (i < MAX_HOTBOARD-1)
            memmove(hotboard + i + 1, hotboard + i, sizeof(HDR)*(MAX_HOTBOARD-i-1));
        memset(hotboard + i, 0, sizeof(HDR));
        time(&(hotboard[i].chrono));
        strcpy(hotboard[i].xname, brd->brdname);
        sprintf(hotboard[i].title, "%-16s%s", brd->brdname, brd->title );
        hotboard[i].xmode = GEM_BOARD | GEM_FOLDER;
        hotboard[i].xid = brd->n_reads;
        if (hotcount < MAX_HOTBOARD)
            hotcount++;
    }
}

static void
count_board(
    BRD *brd,
    time_t now)
{
    struct tm ntime, *xtime;
    BSTATCOUNT bcount;
    char fpath[128], flog[128];
    BSTAT tmp[24], *base, *head, *tail;

    int fd, count, size;
    struct stat st;

    int pos;

    int hour;
    int day;
    int week;
    int twoweek;
    int month;
    int threemonth;
    int halfyear;
    int year;



    xtime = localtime(&now);
    ntime = *xtime;
    memset(&bcount, 0, sizeof(BSTATCOUNT));

    brd_fpath(fpath, brd->brdname, FN_BRD_STATCOUNT);
    brd_fpath(flog, brd->brdname, FN_BRD_STAT);

    fd = open(flog, O_RDONLY);
    if (fd < 0)
        return;

    if (fstat(fd, &st) || (count = st.st_size / sizeof(BSTAT)) <= 0)
    {
        close(fd);
        return;
    }
    head = base = (BSTAT *)malloc(sizeof(BSTAT) * count);
    size = read(fd, base, sizeof(BSTAT) * count);
    count = size / sizeof(BSTAT);
    tail = head + count;

    hour = count;
    day = (count > DAY_HOUR) ? (count - DAY_HOUR + 1) : 1;
    week = (count > WEEK_HOUR) ? (count - WEEK_HOUR + 1) : 1;
    twoweek = (count > TWOWEEK_HOUR) ? (count - TWOWEEK_HOUR + 1) : 1;
    month = (count > MONTH_HOUR) ? (count - MONTH_HOUR + 1) : 1;
    threemonth = (count > THREEMONTH_HOUR) ? (count - THREEMONTH_HOUR + 1) : 1;
    halfyear = (count > HALFYEAR_HOUR) ? (count - HALFYEAR_HOUR + 1) : 1;
    year = (count > YEAR_HOUR) ? (count - YEAR_HOUR + 1) : 1;

    pos = 1;

    rec_get(fpath, &bcount, sizeof(BSTATCOUNT), 0);


    memset(&bcount, 0, sizeof(BSTAT) * 8);

    do
    {
        if (hour <= pos)
            add_log(&(bcount.hour), head);
        if (day <= pos)
            add_log(&(bcount.day), head);
        if (week <= pos)
            add_log(&(bcount.week), head);
        if (twoweek <= pos)
            add_log(&(bcount.twoweek), head);
        if (month <= pos)
            add_log(&(bcount.month), head);
        if (threemonth <= pos)
            add_log(&(bcount.threemonth), head);
        if (halfyear <= pos)
            add_log(&(bcount.halfyear), head);
        if (year <= pos)
            add_log(&(bcount.year), head);
        pos++;
    } while (++head < tail);

    strcpy(bcount.hour.type, "近一小時");
    strcpy(bcount.day.type, "近一天");
    strcpy(bcount.week.type, "近一週");
    strcpy(bcount.twoweek.type, "近兩週");
    strcpy(bcount.month.type, "近一月");
    strcpy(bcount.threemonth.type, "近三月");
    strcpy(bcount.halfyear.type, "近半年");
    strcpy(bcount.year.type, "近一年");

    bcount.hour.chrono = now;
    bcount.day.chrono = now;
    bcount.week.chrono = now;
    bcount.twoweek.chrono = now;
    bcount.month.chrono = now;
    bcount.threemonth.chrono = now;
    bcount.halfyear.chrono = now;
    bcount.year.chrono = now;

    memcpy(tmp, &(bcount.lhour[1]), sizeof(BSTAT) * 23);
    memcpy(bcount.lhour, tmp, sizeof(BSTAT) * 23);
    memcpy(&(bcount.lhour[23]), &(bcount.hour), sizeof(BSTAT));

    if (ntime.tm_hour == 0)
    {
        memcpy(tmp, &(bcount.lday[1]), sizeof(BSTAT) * 23);
        memcpy(bcount.lday, tmp, sizeof(BSTAT) * 23);
        memcpy(&(bcount.lday[23]), &(bcount.day), sizeof(BSTAT));

        if (ntime.tm_wday == 0)
        {
            memcpy(tmp, &(bcount.lweek[1]), sizeof(BSTAT) * 23);
            memcpy(bcount.lweek, tmp, sizeof(BSTAT) * 23);
            memcpy(&(bcount.lweek[23]), &(bcount.week), sizeof(BSTAT));
        }

        if (ntime.tm_mday == 1)
        {
            memcpy(tmp, &(bcount.lmonth[1]), sizeof(BSTAT) * 23);
            memcpy(bcount.lmonth, tmp, sizeof(BSTAT) * 23);
            memcpy(&(bcount.lmonth[23]), &(bcount.month), sizeof(BSTAT));
        }


        head = base + (year - 1);
        size = ((count <= YEAR_HOUR) ? count : YEAR_HOUR) * sizeof(BSTAT);
        lseek(fd, (off_t) 0, SEEK_SET);
        write(fd, head, size);
        ftruncate(fd, size);

    }

    rec_put(fpath, &bcount, sizeof(BSTATCOUNT), 0);

    free(base);
    close(fd);
}


int
main(void)
{
    BRD *bcache, *head, *tail;
    BSTAT bstat;

    char fpath[128];

    time_t now;
    struct tm ntime, *xtime;

    now = time(NULL);
    xtime = localtime(&now);
    ntime = *xtime;

    chdir(BBSHOME);
    umask(077);

    /* --------------------------------------------------- */
    /* build Class image					 */
    /* --------------------------------------------------- */

    bshm_init();

    /* --------------------------------------------------- */
    /* 看板資訊統計					 */
    /* --------------------------------------------------- */

    head = bcache = bshm->bcache;
    tail = head + bshm->number;
    do
    {
        if (((head->readlevel & (PERM_BASIC|PERM_VALID|PERM_POST)) || head->readlevel == 0) && head->brdname[0])
        {
            count_hot(head);
        }
        if (!(head->battr & BRD_NOTOTAL) && head->brdname[0])
        {


//          if (!str_cmp(head->brdname, "Test") || !str_cmp(head->brdname, "WindTop"))
//          {
//              printf("DEBUG: bname:%s  reads:%d  posts:%d\n", head->brdname, head->n_reads, head->n_posts);
                memset(&bstat, 0, sizeof(BSTAT));
                bstat.chrono = now;
                bstat.n_reads = head->n_reads;
                bstat.n_posts = head->n_posts;
                bstat.n_news = head->n_news;
                bstat.n_bans = head->n_bans;
                head->n_reads = 0;
                head->n_posts = 0;
                head->n_news = 0;
                head->n_bans = 0;
                strcpy(bstat.type, "每小時");

                brd_fpath(fpath, head->brdname, FN_BRD_STAT);
                rec_add(fpath, &bstat, sizeof(BSTAT));

                count_board(head, now);
//          }
        }
    } while (++head < tail);

    save_hot();
    return 0;
}
#else

int main(void)
{
    return 0;
}

#endif
