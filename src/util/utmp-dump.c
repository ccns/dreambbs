/*-------------------------------------------------------*/
/* util/utmp-dump.c	( YZU WindTopBBS Ver 3.00 )	 */
/*-------------------------------------------------------*/
/* target : 		 	 			 */
/* create : 					 	 */
/* update : 					 	 */
/*-------------------------------------------------------*/

#define	_MODES_C_


#include "bbs.h"
#include <sys/shm.h>

ACCT cuser;
UTMP *cutmp,utmp;

static int pal_count;
static int *pal_pool;
static UCACHE *ushm;
static int can_see(UTMP *up);
typedef UTMP *pickup;


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


/* ----------------------------------------------------- */
/* �O�� pal �� user number				 */
/* ----------------------------------------------------- */

#define PICKUP_WAYS	(6)

static int pickup_way;

char *
bmode(up, simple)
  UTMP *up;
  int simple;
{
  static char modestr[32];
  int mode;
  char *word;
  
  if(!up)
    return "���b���W";
  
  mode = up->mode;
  if(mode == M_IDLE)
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
    return (word); 
  
  sprintf(modestr, "%s %s", word, up->mateid);
  return (modestr);
}


/* ----------------------------------------------------- */
/* �n�ͦW��G�d�ߪ��A�B�ͽ˴y�z				 */
/* ----------------------------------------------------- */


static int
can_see(up)
  UTMP *up;
{
  int count, *cache, datum, mid;

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

static int
is_bad(userno)
  int userno;
{
  int count, *cache, datum, mid;

  if ((cache = pal_pool))
  {
    for (count = pal_count; count > 0;)
    {
      datum = cache[mid = count >> 1];
      if ((-userno) == datum)
        return 1;
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
  return 0;
}

static int
can_message(up)
  UTMP *up;
{
  int self, ufo, can;

  if (up->userno == (self = cuser.userno))
    return NA;

  if (HAS_PERM(PERM_SYSOP | PERM_ACCOUNTS | PERM_CHATROOM))	/* �����B�b���B��ѫ� */
    return YEA;

  ufo = up->ufo;

  if ( ufo & (UFO_MESSAGE))		/* �������� */
    return NA;

  if (!(ufo & UFO_QUIET))
    return YEA;			/* �I�s���S�����A��L�l�� */

  can = 0;			/* �䤣�쬰 normal */
  can = can_see(up);
  return (ufo & UFO_QUIET)
    ? can == 1			/* �u���n�ͥi�H */
    : can < 2 /* �u�n���O�l�� */ ;
}


static int
can_override(up)
  UTMP *up;
{
  int self, ufo, can;

  if (up->userno == (self = cuser.userno))
    return NA;

  if (HAS_PERM(PERM_SYSOP | PERM_ACCOUNTS | PERM_CHATROOM))	/* �����B�b���B��ѫ� */
    return YEA;

  ufo = up->ufo;

  if (ufo & (UFO_PAGER1 | UFO_REJECT))		/* �������� */
    return NA;

  if (!(ufo & UFO_PAGER))
    return YEA;			/* �I�s���S�����A��L�l�� */

  can = 0;			/* �䤣�쬰 normal */

  can = can_see(up);
  return (ufo & UFO_PAGER)
    ? can == 1			/* �u���n�ͥi�H */
    : can < 2 /* �u�n���O�l�� */ ;
}

/* ----------------------------------------------------- */
/* �n�ͦW��G�s�W�B�R���B�ק�B���J�B�P�B		 */
/* ----------------------------------------------------- */


int
is_pal(userno)
  int userno;
{
  int count, *cache, datum, mid;

  if ((cache = pal_pool))
  {
    for (count = pal_count; count > 0;)
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


static int
int_cmp(a, b)
  int *a;
  int *b;
{
  return *a - *b;
}


void
pal_cache()
{
  int count, fsize, ufo ;
  int *plist, *cache;
  PAL *phead, *ptail;
  char *fimage, fpath[64];
  UTMP *up;

  up = cutmp;
  cutmp->userno = cuser.userno;

  cache = NULL;
  count = 0;
  ufo = cuser.ufo & ~( UFO_BIFF | UFO_BIFFN | UFO_REJECT | UFO_FCACHE);

  fsize = 0;
  usr_fpath(fpath, cuser.userid, FN_PAL);
  fimage = f_img(fpath, &fsize);
  if (fimage != NULL)
  {
    if (fsize > (PAL_MAX * sizeof(PAL)))
    {
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
      } while (++phead < ptail);

      if (count > 0)
      {
	ufo |= UFO_FCACHE;
	if (count > 1)
	  xsort(cache, count, sizeof(int), int_cmp);
      }
      else
      {
	cache = NULL;
      }
      
    }
  }

  pal_pool = cache;
  up->pal_max = pal_count = count;
  
  if(fimage) 
    free(fimage);
  cuser.ufo = ufo;
}


/*-------------------------------------------------------*/
/* ��榡��Ѥ���					 */
/*-------------------------------------------------------*/


static pickup ulist_pool[MAXACTIVE];
static int friend_num,ofriend_num,pfriend_num,bfriend_num;
static int ulist_head();
static int ulist_init();


static char *msg_pickup_way[PICKUP_WAYS] =
{
  "���N",
  "�N��",
  "�G�m",
  "�ʺA",
  "�ʺ�",
  "���m"
};

static char 
ck_state(in1,in2,up,mode)
  int in1;
  int in2;
  UTMP *up;
  int mode;
{
	if (up->ufo & in2)
	  return '#';
	else if (up->ufo & in1)
	{
	  if(mode)
	    return can_override(up) ? 'o' : '*';
	  else
	    return can_message(up) ? 'o' : '*';
	}
	else
	  return ' ';
}	



static int
ulist_body(xo)
  XO *xo;
{
  pickup *pp;
  UTMP *up;
  int cnt, max, ufo, self, userno, sysop, diff, diffmsg, fcolor,colortmp;
  char buf[8],color[20],ship[80],*wcolor[7] = {"\033[m",COLOR_PAL,COLOR_BAD,COLOR_BOTH,COLOR_OPAL,COLOR_CLOAK,COLOR_BOARDPAL};

//  pal = cuser.ufo;
   
  max = xo->max;
  if (max <= 0)
  {
    return 0;
  }

//  pal &= UFO_PAL;
  cnt = 0;
  pp = &ulist_pool[0];
  self = cuser.userno;
  sysop = HAS_PERM(PERM_SYSOP | PERM_ACCOUNTS);

  while (cnt++ < max)
  {
      up = *pp++;
      if ((userno = up->userno) && !((up->ufo & UFO_CLOAK) && !HAS_PERM(PERM_SEECLOAK) && (up->userno != cuser.userno)) )
      {
	if ((diff = up->idle_time))
	  sprintf(buf, "%2d", diff);
	else
	  buf[0] = '\0';
	
	fcolor = (userno == self) ? 3 : is_pal(userno);

        colortmp = can_see(up);
	if(is_pal(userno) && colortmp == 1) fcolor = 3;
        else if (!is_pal(userno) && colortmp == 1) fcolor = 4;
	
        if (is_bad(userno)) fcolor = 2;

	ufo = up->ufo;


        diff = ck_state(UFO_PAGER,UFO_PAGER1,up,1);
        diffmsg = ck_state(UFO_QUIET,UFO_MESSAGE,up,0);

        colortmp = 1;
        if(ufo & UFO_CLOAK) fcolor = 5;
        else if(fcolor == 0)
          colortmp = 0;  
        strcpy(color,wcolor[fcolor]);

	printf("%5d %s%-13s%-22.21s%s%-16.15s%c%c %-16.16s%s\n",
	  cnt,
	  color, up->userid,
	  (HAS_PERM(PERM_SYSOP) && (cuser.ufo2 & UFO2_REALNAME))? up->realname : up->username, 
	  colortmp > 0 ? "\033[m" : "",
	  (cuser.ufo2 & UFO2_SHIP) ? ship : ((up->ufo & UFO_HIDEDN)&&!HAS_PERM(PERM_SYSOP)) ? 
	  HIDEDN_SRC : up->from , diff,diffmsg,
	  bmode(up, 0), buf);
      }
  }

  return 0;
}


static int
ulist_cmp_userid(i, j)
  UTMP **i, **j;
{
  return str_cmp((*i)->userid, (*j)->userid);
}


static int
ulist_cmp_host(i, j)
  UTMP **i, **j;
{
  return str_cmp((*i)->from, (*j)->from);
}

static int
ulist_cmp_idle(i, j)
  UTMP **i, **j;
{
  return (*i)->idle_time - (*j)->idle_time;
}

static int
ulist_cmp_mode(i, j)
  UTMP **i, **j;
{
  return (*i)->mode - (*j)->mode;
}

static int
ulist_cmp_nick(i, j)
  UTMP **i, **j;
{
  return str_cmp((*i)->username, (*j)->username);
}

static int (*ulist_cmp[]) () =
{
  ulist_cmp_userid,
  ulist_cmp_host,
  ulist_cmp_mode,
  ulist_cmp_nick,
  ulist_cmp_idle
};


static int
ulist_init(xo)
  XO *xo;
{
  UTMP *up, *uceil;
  pickup *pp;
  int max, filter, seecloak, userno, self,i,nf_num,tmp;
  pickup pf[MAXACTIVE],of[MAXACTIVE],nf[MAXACTIVE];
  pp = ulist_pool;

  self = cuser.userno;
  filter = cuser.ufo2 & UFO2_PAL;
  
  seecloak = HAS_PERM(PERM_SEECLOAK);

  up = ushm->uslot;
  uceil = (void *) up + ushm->offset;

  max = 0;
  friend_num = ofriend_num = pfriend_num = nf_num = bfriend_num = 0;
  do					/* ����n�� */
  {
    userno = up->userno;
    if (userno <= 0 || (up->pid <= 0 && !HAS_PERM(PERM_SYSOP|PERM_SEECLOAK)))
      continue;
    if (!seecloak && (up->ufo & UFO_CLOAK))
      continue;      
    tmp = can_see(up);
    if(is_bad(userno)) bfriend_num++;
    if (((seecloak || !(up->ufo & UFO_CLOAK)) && (tmp != 2)) || HAS_PERM(PERM_SYSOP|PERM_SEECLOAK) || up->userno == cuser.userno)
    {
      if ((is_pal(userno) && (tmp == 1)) || (userno == self))
	  {
		  *pp++ = up;
	      friend_num++;
	  }
	else if(is_pal(userno) && !(tmp == 1) && !filter)
	  	  pf[pfriend_num++] = up;
	else if(!is_pal(userno) && (tmp == 1) && !filter)
		  of[ofriend_num++] = up;
	else if(!filter)
		  nf[nf_num++] = up;
	}
  } while (++up <= uceil);
  for(i=0;i<pfriend_num;i++)
	  *pp++ = pf[i];
  for(i=0;i<ofriend_num;i++)
	  *pp++ = of[i];
  for(i=0;i<nf_num;i++)
	  *pp++ = nf[i];

  xo->max = max = pp - ulist_pool;;

  if (xo->pos >= max)
    xo->pos = xo->top = 0;

  if ((max > 1) && (pickup_way))
  {
    if(pickup_way==1)
    {
      xsort(ulist_pool, friend_num, sizeof(pickup), ulist_cmp[pickup_way - 1]);
      xsort((ulist_pool+friend_num),pfriend_num , sizeof(pickup), ulist_cmp[pickup_way - 1]);
      xsort((ulist_pool+friend_num+pfriend_num), ofriend_num , sizeof(pickup), ulist_cmp[pickup_way - 1]);
      xsort((ulist_pool+friend_num+pfriend_num+ofriend_num), max - friend_num - pfriend_num - ofriend_num, sizeof(pickup), ulist_cmp[pickup_way - 1]);
    }
    else
      xsort(ulist_pool, max, sizeof(pickup), ulist_cmp[pickup_way - 1]);
  }
  total_num = max;
  return ulist_head(xo);
}


static int
ulist_neck(xo)
  XO *xo;
{
  printf("  �ƦC�覡�G[\033[1m%s\033[m] �W���H�ơG%d %s�ڪ��B�͡G%d %s�P�ڬ��͡G%d %s�a�H�G%d\033[m\n"
    "\033[30;47m No.  �N��         %-22s%-13s   PM %-14s���m \033[m\n",
    msg_pickup_way[pickup_way],total_num,COLOR_PAL,friend_num+pfriend_num,COLOR_OPAL,friend_num+ofriend_num,COLOR_BAD,bfriend_num,
    "��  ��","�G�m", "�ʺA");
  return ulist_body(xo);
}


static int
ulist_head(xo)
  XO *xo;
{
  return ulist_neck(xo);
}

static void
reset_utmp()
{
  cutmp->userno = cuser.userno;
  cutmp->ufo = cuser.ufo;
  strcpy(cutmp->userid, cuser.userid);
  strcpy(cutmp->username, cuser.username);
  strcpy(cutmp->realname, cuser.realname);
}


int
main(argc, argv)
  int argc;
  char *argv[];
{
  XO xo;
  char fpath[128];
  int fd;
 
   chdir(BBSHOME);
 
  ushm = attach_shm(UTMPSHM_KEY, sizeof(UCACHE)); 
  cutmp = &utmp;
  usr_fpath(fpath,argv[1],".ACCT");
  fd = open(fpath,O_RDONLY);
  if(fd>=0)
  {
    if(read(fd,&cuser,sizeof(ACCT)) == sizeof(ACCT))
    {
      reset_utmp();
      pickup_way = atoi(argv[2]);
      pal_cache();
      ulist_init(&xo);
    }
    close(fd);
  }
  return 0;
}
