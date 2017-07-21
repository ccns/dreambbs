/*-------------------------------------------------------*/
/* util/msgall.c	( YZU WindTopBBS Ver 3.00 )	 */
/*-------------------------------------------------------*/
/* target : 		 	 			 */
/* create : 					 	 */
/* update : 					 	 */
/*-------------------------------------------------------*/

#include <sys/shm.h>
#include "bbs.h"

static UCACHE *ushm;

static void *
attach_shm(shmkey, shmsize)
  int shmkey, shmsize;
{
  void *shmptr;
  int shmid;

  shmid = shmget(shmkey, shmsize, 0);
  shmsize = 0;
  shmptr = (void *) shmat(shmid, NULL, 0);

  return shmptr;
}


int
bsend(callee, bmw)
  UTMP *callee;
  BMW *bmw;
{
  BMW *mpool, *mhead, *mtail, **mslot;
  int i;
  pid_t pid;
  time_t texpire;

  if ((pid = callee->pid) <= 0)
    return 1;

  mslot = callee->mslot;
  i = 0;

  for (;;)
  {
    if (mslot[i] == NULL)
      break;

    if (++i >= BMW_PER_USER)
    {
      return 1;
    }
  }

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

  return kill(pid, SIGUSR2);
}


void
bedit(up, bmw)
  UTMP *up;
  BMW *bmw;
{
  bmw->recver = up->userno;	/* ���O�U userno �@�� check */

  bmw->caller = NULL;
  bmw->sender = 0;
  strcpy(bmw->userid, "�t�γq�i");

  bsend(up, bmw);
}

int
main(argc, argv)
  int argc;
  char **argv;
{
  UTMP *up, *uceil;
  BMW bmw;
 
  chdir(BBSHOME);
  if(argc < 2)
  {
	printf("Usage: msgall <msg>\n");
	exit(0);
  }
 
  ushm = attach_shm(UTMPSHM_KEY, sizeof(UCACHE)); 
  strcpy(bmw.msg,argv[1]);

  up = ushm->uslot;
  uceil = (void *) up + ushm->offset;

  do
  {
    if (up->pid <= 0)
      continue;
    bedit(up, &bmw);
  } while (++up <= uceil);

  return 0;
}
