/*-------------------------------------------------------*/
/* util/brdstat.c       ( YZU CSE WindTop BBS 3.02 )     */
/*-------------------------------------------------------*/
/* target : �ݪO��T�έp                                 */
/* create : 2003/08/17                                   */
/* update :                                              */
/*-------------------------------------------------------*/
/* syntax : brdstat                                      */
/*-------------------------------------------------------*/


#include "bbs.h"

#ifdef  HAVE_COUNT_BOARD


#define MAX_LINE        16
#define ADJUST_M        10      /* adjust back 10 minutes */

#define YEAR_HOUR       (365 * 24)
#define HALFYEAR_HOUR   (180 * 24)
#define THREEMONTH_HOUR (90 * 24)
#define MONTH_HOUR      (30 * 24)
#define TWOWEEK_HOUR    (14 * 24)
#define WEEK_HOUR       (7 * 24)
#define DAY_HOUR        (1 * 24)
#define HOUR            (1)


/* ----------------------------------------------------- */
/* �}���Gshm �������P cache.c �ۮe                       */
/* ----------------------------------------------------- */


static BCACHE *bshm;


/* ----------------------------------------------------- */


static void
count_board(
    const BRD *brd,
    time_t now)
{
    BSTATCOUNT bcount;
    char fpath[128];

    memset(&bcount, 0, sizeof(BSTATCOUNT));

    brd_fpath(fpath, brd->brdname, FN_BRD_STATCOUNT);

    rec_get(fpath, &bcount, sizeof(BSTATCOUNT), 0);
    printf("%-15s%10d %10d %10d %10d\n", brd->brdname,
        bcount.threemonth.n_reads, bcount.threemonth.n_posts,
        bcount.threemonth.n_news, bcount.threemonth.n_bans);

}


int
main(void)
{
    BRD *bcache, *head, *tail;

    setgid(BBSGID);
    setuid(BBSUID);
    chdir(BBSHOME);
    umask(077);

    /* --------------------------------------------------- */
    /* build Class image                                   */
    /* --------------------------------------------------- */

    shm_logger_init(NULL);
    bshm_attach(&bshm);
    if (!bshm) /* bshm ���]�w���� */
        exit(1);

    /* --------------------------------------------------- */
    /* �ݪO��T�έp                                        */
    /* --------------------------------------------------- */

    head = bcache = bshm->bcache;
    tail = head + bshm->number;
    do
    {
        if (!(head->battr & BRD_NOTOTAL) && head->brdname[0])
        {
            count_board(head, 0);
        }
    } while (++head < tail);

    return 0;
}
#else

GCC_CONSTEXPR int main(void)
{
    return 0;
}

#endif  /* #ifdef  HAVE_COUNT_BOARD */
