/*-------------------------------------------------------*/
/* util/brdstat.c	( YZU CSE WindTop BBS 3.02 )	 */
/*-------------------------------------------------------*/
/* target : �ݪO��T�έp				 */
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
#define HERFYEAR_HOUR	(180 * 24)
#define	THREEMONTH_HOUR	(90 * 24)
#define	MONTH_HOUR	(30 * 24)
#define	TWOWEEK_HOUR	(14 * 24)
#define	WEEK_HOUR	(7 * 24)
#define	DAY_HOUR	(1 * 24)
#define	HOUR		(1)		


extern int errno;


/* ----------------------------------------------------- */
/* �}���Gshm �������P cache.c �ۮe			 */
/* ----------------------------------------------------- */


static BCACHE *bshm;


static void
attach_err(shmkey, name)
  int shmkey;
  char *name;
{
  fprintf(stderr, "[%s error] key = %x\n", name, shmkey);
  exit(1);
}


static void *
attach_shm(shmkey, shmsize)
  register int shmkey, shmsize;
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
bshm_init()
{
  register BCACHE *xshm;

  xshm = bshm;
  if (xshm == NULL)
  {
    bshm = xshm = attach_shm(BRDSHM_KEY, sizeof(BCACHE));
  }

  if(bshm->uptime < 0)
    exit(1);

}


/* ----------------------------------------------------- */


static void
count_board(brd,now)
  BRD *brd;
  time_t now;
{
  BSTATCOUNT bcount;
  char fpath[128];

  memset(&bcount,0,sizeof(BSTATCOUNT));
  
  brd_fpath(fpath,brd->brdname,FN_BRD_STATCOUNT);
  
  rec_get(fpath,&bcount,sizeof(BSTATCOUNT),0);
  printf("%-15s%10d %10d %10d %10d\n",brd->brdname,
  	bcount.threemonth.n_reads,bcount.threemonth.n_posts,
  	bcount.threemonth.n_news,bcount.threemonth.n_bans);

}


int
main()
{
  BRD *bcache, *head, *tail;
  
  chdir(BBSHOME);
  umask(077);

  /* --------------------------------------------------- */
  /* build Class image					 */
  /* --------------------------------------------------- */

  bshm_init();

  /* --------------------------------------------------- */
  /* �ݪO��T�έp					 */
  /* --------------------------------------------------- */

  head = bcache = bshm->bcache;
  tail = head + bshm->number;
  do
  {
    if(!(head->battr & BRD_NOTOTAL) && head->brdname[0])
    {
        count_board(head,0);
    }
  } while (++head < tail);

  return 0;
}
#else

int main()
{
  return 0;
}

#endif
