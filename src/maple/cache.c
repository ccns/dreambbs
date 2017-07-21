/*-------------------------------------------------------*/
/* cache.c	( NTHU CS MapleBBS Ver 2.36 )		 */
/*-------------------------------------------------------*/
/* target : cache up data by shared memory		 */
/* create : 95/03/29				 	 */
/* update : 95/12/15				 	 */
/*-------------------------------------------------------*/


#include "bbs.h"

#include <sys/ipc.h>
#include <sys/shm.h>

#ifdef	HAVE_SEM
#include <sys/sem.h>
#endif


#ifdef MODE_STAT 
UMODELOG modelog; 
time_t mode_lastchange; 
#endif
extern int item_lenth[20];

static void
attach_err(shmkey, name)
  int shmkey;
  char *name;
{
  char buf[80];

  sprintf(buf, "key = %x", shmkey);
  blog(name, buf);
  exit(1);
}


static void *
attach_shm(shmkey, shmsize)
  int shmkey, shmsize;
{
  void *shmptr;
  int shmid;

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


#ifdef	HAVE_SEM


/* ----------------------------------------------------- */
/* semaphore : for critical section			 */
/* ----------------------------------------------------- */


static int ap_semid;


void
sem_init()
{
  int semid;

  union semun
  {
    int val;
    struct semid_ds *buf;
    ushort *array;
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
sem_lock(op)
  int op;			/* op is BSEM_ENTER or BSEM_LEAVE */
{
  struct sembuf sops;

  sops.sem_num = 0;
  sops.sem_flg = SEM_UNDO;
  sops.sem_op = op;
  semop(ap_semid, &sops, 1);
}

#endif	/* HAVE_SEM */


/*-------------------------------------------------------*/
/* .UTMP cache						 */
/*-------------------------------------------------------*/


UCACHE *ushm;


void
ushm_init()
{
  UCACHE *xshm;

  ushm = xshm = attach_shm(UTMPSHM_KEY, sizeof(UCACHE));

#if 0
  if (xshm->mbase < xshm->mpool)
    xshm->mbase = xshm->mpool;
#endif
}


#ifndef	_BBTP_


void
utmp_mode(mode)
  int mode;
{
  if (bbsmode != mode)
  {

#ifdef MODE_STAT
    if (mode != M_XMODE && bbsmode != M_XMODE) { /* XMODE �Q���Ӱ��䥦����, ���C�J�έp */
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
utmp_new(up)
  UTMP *up;
{
  UCACHE *xshm;
  UTMP *uentp, *utail;

  /* --------------------------------------------------- */
  /* semaphore : critical section			 */
  /* --------------------------------------------------- */

#ifdef	HAVE_SEM
  sem_lock(BSEM_ENTER);
#endif

  xshm = ushm;

#ifdef	HAVE_MAXRESERVE  
  if(HAS_PERM(PERM_ADMIN))
  {
#endif  
  	uentp = xshm->uslot;
  	utail = uentp + MAXACTIVE;
#ifdef	HAVE_MAXRESERVE  
  }
  else
  {
  	uentp = xshm->uslot + MAX_RESERVE;
  	utail = uentp + (MAXACTIVE - MAX_RESERVE);
  }
#endif

  /* uentp += (up->pid % xshm->count);	*//* hashing */

  do
  {
    if (!uentp->pid && !uentp->userno )
    {
      usint offset;

      offset = (void *) uentp - (void *) xshm->uslot;
      memcpy(uentp, up, sizeof(UTMP));
      xshm->count++;
      if (xshm->offset < offset)
	xshm->offset = offset;
      cutmp = uentp;

#ifdef	HAVE_SEM
      sem_lock(BSEM_LEAVE);
#endif

      return 1;
    }
  } while (++uentp < utail);

  /* Thor:�i�Duser���H�n���@�B�F */

#ifdef	HAVE_SEM
  sem_lock(BSEM_LEAVE);
#endif

  return 0;
}


void
utmp_free()
{
  UTMP *uentp;

  uentp = cutmp;
  if (!uentp || !uentp->pid)
    return;

#ifdef	HAVE_SEM
  sem_lock(BSEM_ENTER);
#endif

  memset(uentp,0,sizeof(UTMP));
/*  uentp->pid = uentp->userno = 0;*/
  ushm->count--;

#ifdef	HAVE_SEM
  sem_lock(BSEM_LEAVE);
#endif
}


UTMP *
utmp_find(userno)
  int userno;
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

UTMP *
pid_find(pid)
  int pid;
{
  UTMP *uentp, *uceil;

  uentp = ushm->uslot;
  uceil = (void *) uentp + ushm->offset;
  do
  {
    if (uentp->pid == pid && uentp->pid != 0)
      return uentp;
  } while (++uentp <= uceil);

  return NULL;
}


#if 0
int
apply_ulist(fptr)
  int (*fptr) ();
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
utmp_search(userno, order)
  int userno;
  int order;			/* �ĴX�� */
{
  UTMP *uentp, *uceil;

  uentp = ushm->uslot;
  uceil = (void *) uentp + ushm->offset;
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
#endif


int
utmp_count(userno, show)
  int userno;
  int show;
{
  UTMP *uentp, *uceil;
  int count;

  count = 0;
  uentp = ushm->uslot;
  uceil = (void *) uentp + ushm->offset;
  do
  {
    if (uentp->userno == userno)
    {
      count++;
      if (show)
      {
	prints("(%d) �ثe���A��: %-17.16s(�Ӧ� %s)\n",
	  count, bmode(uentp, 0), uentp->from);
      }
    }
  } while (++uentp <= uceil);
  return count;
}

#if 1
#ifdef	HAVE_CLASSTABLEALERT
int
cmpclasstable(ptr)
  CLASS_TABLE_ALERT *ptr;
{
  return ptr->userno == cuser.userno;
}

void  
classtable_free(void)
{
  int pos;
  while( (pos = rec_loc(FN_CLASSTABLE_DB,sizeof(CLASS_TABLE_ALERT), (void *)cmpclasstable)) >= 0)
    rec_del(FN_CLASSTABLE_DB, sizeof(CLASS_TABLE_ALERT), pos, (void *)cmpclasstable, NULL);
}

void 
classtable_main(void)
{
  int fd, size=0;
  struct stat st;
  int i;
  char fpath[128];
  CLASS_TABLE_ALERT tmp;

  memset(&tmp,0,sizeof(CLASS_TABLE_ALERT));

  tmp.userno = cuser.userno;

  usr_fpath(fpath,cuser.userid,FN_CLASSTABLE2);

  if ((fd = open(fpath, O_RDWR, 0600)) < 0)
    return;

  if (!fstat(fd, &st) && (size = st.st_size) > 0)
  {
    CLASS_TABLE2 *pbase;

    pbase = (CLASS_TABLE2 *) malloc(size);
    size = read(fd, pbase, size);
	for(i=0;i<78;i++)
	{
		if(pbase[i].valid)
		{
			strcpy(tmp.item[i].condensation,pbase[i].condensation);
			strcpy(tmp.item[i].room,pbase[i].room);
			tmp.item[i].used = 1;
		}
	}
	rec_add(FN_CLASSTABLE_DB, &tmp, sizeof(CLASS_TABLE_ALERT));


	free(pbase);
  }
  close(fd);

}
#endif
#endif

/*-------------------------------------------------------*/
/* .BRD cache						 */
/*-------------------------------------------------------*/


BCACHE *bshm;


#if 0
void
bsync()
{
  rec_put(FN_BRD, bshm->bcache, sizeof(BRD) * bshm->number, 0);
}
#endif


void
bshm_init()
{
  BCACHE *xshm;
  time_t *uptime;
  int n, turn;

  turn = 0;
  xshm = bshm;
  if (xshm == NULL)
  {
    bshm = xshm = attach_shm(BRDSHM_KEY, sizeof(BCACHE));
  }

  uptime = &(xshm->uptime);

  for (;;)
  {
    n = *uptime;
    if (n > 0)
      return;

    if (n < 0)
    {
      if (++turn < 30)
      {
	sleep(2);
	continue;
      }
    }

    *uptime = -1;

    if ((n = open(FN_BRD, O_RDONLY)) >= 0)
    {
      xshm->number =
	read(n, xshm->bcache, MAXBOARD * sizeof(BRD)) / sizeof(BRD);
      close(n);
    }

    /* ���Ҧ� boards ��Ƨ�s��A�]�w uptime */

    time(uptime);
    blog("CACHE", "reload bcache");
    return;
  }
}


#if 0
int
apply_boards(func)
  int (*func) ();
{
  extern char brd_bits[];
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


int
brd_bno(bname)
  char *bname;
{
  BRD *brdp, *bend;
  int bno;

  brdp = bshm->bcache;
  bend = brdp + bshm->number;
  bno = 0;

  do
  {
    if (!str_cmp(bname, brdp->brdname))
      return bno;

    bno++;
  } while (++brdp < bend);

  return -1;
}


#if 0
BRD *
getbrd(bname)
  char *bname;
{
  BRD *bhdr, *tail;

  bhdr = bshm->bcache;
  tail = bhdr + bshm->number;
  do
  {
    if (!str_cmp(bname, bhdr->brdname))
      return bhdr;
  } while (++bhdr < tail);
  return NULL;
}
#endif

#ifdef	HAVE_OBSERVE_LIST
/*-------------------------------------------------------*/
/* etc/observe.db cache                                 */
/*-------------------------------------------------------*/
OCACHE *oshm;

static int
int_cmp(a, b)
  int *a;
  int *b;
{
  return *a - *b;
}


int
observeshm_find(userno)
  int userno;
{
  int count, *cache, datum, mid;

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


void
observeshm_load()
{
  OBSERVE *head,*tail;
  int size;
  char *fimage;

  size = 0;
  oshm->total = 0;
  memset(oshm->userno,0,sizeof(int)*MAXOBSERVELIST);
  fimage = f_img(FN_ETC_OBSERVE, &size);
  if(fimage)
  {
    if(size > 0)
    {
    	head = (OBSERVE *) fimage;
    	tail = (OBSERVE *) (fimage + size);
	for(;head<tail;head++)
	{
	  oshm->userno[oshm->total++] = head->userno;
	}
	if(oshm->total>1)
  	  xsort(oshm->userno, oshm->total, sizeof(int), int_cmp);
    }
    free(fimage);
  }

}

void
observeshm_init()
{
    oshm = attach_shm(OBSERVE_KEY, sizeof(OCACHE));
	observeshm_load();
}
#endif

/*-------------------------------------------------------*/
/* run/var/counter cache                                 */
/*-------------------------------------------------------*/
COUNTER *curcount;

void
count_update()
{
  if((ushm->count) > (curcount->samehour_max_login))
  {
    curcount->samehour_max_login = ushm->count;
    curcount->samehour_max_time = time(0);
  }
  curcount->cur_hour_max_login++;
  curcount->cur_day_max_login++;
}

void
count_load()
{
  COUNTER *head;
  int fw,size;
  struct stat st;

  head = curcount;
  if((fw = open(FN_VAR_SYSHISTORY,O_RDONLY)))
  {

    if(!fstat(fw, &st) && (size = st.st_size) > 0)
    {
      read(fw, head, sizeof(COUNTER));
    }
    close(fw);
  }
}

void
count_init()
{
  if (curcount == NULL)
  {
    curcount = attach_shm(COUNT_KEY, sizeof(COUNTER));
    if(curcount->hour_max_login == 0)
      count_load();
  }
}

/*-------------------------------------------------------*/
/* etc/banmail cache                                     */
/*-------------------------------------------------------*/
FWCACHE *fwshm;
BANMAIL *curfw;

static int
cmpban(ban)
  BANMAIL *ban;
{
  return !strcmp(ban->data,curfw->data);
}

void
fwshm_load()
{
  BANMAIL *head,data;
  int fw,size,pos;
  struct stat st;

  head = fwshm->fwcache;

  while(*head->data)
  {
    curfw = head;
    pos = rec_loc(FN_ETC_BANMAIL_ACL,sizeof(BANMAIL),cmpban);
    if(pos >= 0)
    {
      rec_get(FN_ETC_BANMAIL_ACL, &data, sizeof(BANMAIL), pos);
      data.usage = head->usage;
      data.time = head->time;
      rec_put(FN_ETC_BANMAIL_ACL, &data, sizeof(BANMAIL), pos);
    }
    head++;
  }  
  
  head = fwshm->fwcache;  
  fw = open(FN_ETC_BANMAIL_ACL,O_RDONLY);
  fstat(fw, &st);
  
  if(!fstat(fw, &st) && (size = st.st_size) > 0)
  {
    if(size > MAXFIREWALL * sizeof(BANMAIL))
      size = MAXFIREWALL * sizeof(BANMAIL);
      
    if (size)
      read(fw, head, size);
    fwshm->number = size / sizeof(BANMAIL);
  }
  close(fw);
}

void
fwshm_init()
{
  if (fwshm == NULL)
  {
    fwshm = attach_shm(FWSHM_KEY, sizeof(FWCACHE));
    if(fwshm->number == 0)
      fwshm_load();
  }
}

/*-------------------------------------------------------*/
/* etc/movie cache					 */
/*-------------------------------------------------------*/


static FCACHE *fshm;


void
fshm_init()
{
  if (fshm == NULL)
    fshm = attach_shm(FILMSHM_KEY, sizeof(FCACHE));
}


static inline void
out_rle(str,film)
  uschar *str;
  int film;
{
#ifdef SHOW_USER_IN_TEXT
  uschar *t_name = cuser.userid;
  uschar *t_nick = cuser.username;
#endif
  int x,y/*,count=0*/;
  int cc, rl;

  if(film)
    move(1,0/*item_lenth[count++]*/);
    //move(3,36+item_lenth[count++]);
  while ((cc = *str))
  {
    str++;
    switch (cc)
    {
    case 8:  /* Thor.980804: ���Ѥ@�U, opus���Y�L�F */
      rl = *str++;
      cc = *str++;

      while (--rl >= 0)
      {
        if(cc=='\n' && film) 
        {
          getyx(&y,&x);    
          outs("\033[m\0");
          clrtoeol();
          move(y + 1, 0/*item_lenth[count++]*/);
        }
        else
          outc(cc);
          
      }
      continue;

#ifdef SHOW_USER_IN_TEXT
    case 1:
      if ((cc = *t_name) && (cuser.ufo2 & UFO2_SHOWUSER))
	t_name++;
      else
	cc = (cuser.ufo2 & UFO2_SHOWUSER) ? ' ': '#';
      break;

    case 2:
      if ((cc = *t_nick) && (cuser.ufo2 & UFO2_SHOWUSER))
	t_nick++;
      else
	cc = (cuser.ufo2 & UFO2_SHOWUSER) ? ' ' : '%';
#endif
    }
    if(cc=='\n' && film)
    {
      getyx(&y,&x);
      outs("\033[m\0");
      clrtoeol();
      move(y + 1, 0/*item_lenth[count++]*/);
      
    }
    else
      outc(cc);
  }
/*  while(count>=0) item_lenth[count--]=0;*/
}


int
film_out(tag, row)
  int tag;
  int row;			/* -1 : help */
{
  int fmax, len, *shot;
  char *film, buf[FILM_SIZ];
  
  if (row <= 0)
    clear();
  else
    move(row, 0);

  len = 0;
  shot = fshm->shot;
  film = fshm->film;

  while (!(fmax = *shot))	/* util/camera.c ���b���� */
  {
    sleep(5);
    if (++len > 10)
      return FILM_MOVIE;
  }
  
  if (tag >= FILM_MOVIE)	/* random select */
  {
    tag += (time(0) & 7);	/* 7 steps forward */
    if (tag >= fmax)
      tag = FILM_MOVIE;
  }    /* Thor.980804: �i��O�G�N���a? �Ĥ@�i random select�e�K�Ө䤤�@�� */
  
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
  
  if (tag > FILM_MOVIE)          /* FILM_MOVIE */
  {
    out_rle(buf,1);
  }
  else
  {
    out_rle(buf,0);
  }  

  if (row < 0)			/* help screen */
    vmsg(NULL);

  return tag;
}

UTMP *
utmp_check(userid)        /* �ˬd�ϥΪ̬O�_�b���W */
  char *userid;
{
  UTMP *uentp, *uceil;

  uentp = ushm->uslot;
  uceil = (void *) uentp + ushm->offset;
  do
  {
    if (uentp->pid)
    {           /* �w�g���������ˬd */
      if (!strcmp(userid, uentp->userid)
        && (!(uentp->ufo & UFO_CLOAK) || HAS_PERM(PERM_SEECLOAK))
        /*&& ((can_see(uentp) != 2) || HAS_PERM(PERM_ACCOUNTS))*/)
        return uentp;
    }
  } while (++uentp <= uceil);

  return NULL;
}

#endif	/* _BBTP_ */
