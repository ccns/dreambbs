/*-------------------------------------------------------*/
/* talk.c	( NTHU CS MapleBBS Ver 3.00 )		 */
/*-------------------------------------------------------*/
/* target : talk/query/friend(pal) routines	 	 */
/* create : 95/03/29				 	 */
/* update : 2000/01/02				 	 */
/*-------------------------------------------------------*/

#define	_MODES_C_

#include "bbs.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#undef	APRIL_FIRST

typedef struct
{
  int pal_no;
  char ship[46];
}  PAL_SHIP;

#ifdef  HAVE_PIP_FIGHT
  void (*p)();
#endif



static int pal_count;
static int *pal_pool;
static int bmw_modetype;
#ifdef HAVE_BOARD_PAL
static int board_pals;
#endif
extern UCACHE *ushm;
extern XZ xz[];
static PAL_SHIP *pal_ship;
static int can_see(UTMP *up);


#ifdef EVERY_Z
extern int vio_fd;		/* Thor.0725: ¬°talk,chat¥i¥Î^z§@·Ç³Æ */
extern int holdon_fd;
#endif


typedef struct
{
  int curcol, curln;
  int sline, eline;
}      talk_win;


typedef struct
{
	UTMP *utmp;
	int type;
}	PICKUP;


void my_query();

static void
reset_utmp()
{
  cutmp->pid = currpid;
  cutmp->userno = cuser.userno;
  cutmp->mode = bbsmode;
  cutmp->ufo = cuser.ufo;
  strcpy(cutmp->userid, cuser.userid);
  strcpy(cutmp->username, cuser.username);
  strcpy(cutmp->realname, cuser.realname);
  str_ncpy(cutmp->from, fromhost, sizeof(cutmp->from));
}


/* ----------------------------------------------------- */
/* °O¿ý pal ªº user number				 */
/* ----------------------------------------------------- */

#ifdef	HAVE_BOARD_PAL
#define PICKUP_WAYS	(7)
int pickup_way=1;
#else
#define	PICKUP_WAYS	(6)
int pickup_way=1;
#endif

static char page_requestor[40];

#if 0
#ifdef	HAVE_SHOWNUMMSG
typedef struct
{
  int year;
  int month;
  int day;
  int weekday;
  char *sendmsg;
  char *recvmsg;
}       MESSAGE;

static MESSAGE *MessageTable[] =
{

};
#endif
#endif

char *
bmode(up, simple)
  UTMP *up;
  int simple;
{
  static char modestr[32];
#ifdef	HAVE_SHOWNUMMSG
  char *nums[9] = {"¤@","¤G","¤T","¥|","¤­","¤»","¤C","¤K","¤E"};
#endif
  int mode;
  char *word;
  
  if(!up)
    return "¤£¦b¯¸¤W";
    
#ifdef	HAVE_SHOWNUMMSG
  if(up->num_msg > 9)
  {
    strcpy(modestr,"³QÄéÃz¤F");
    return (modestr);
  }
  else if(up->num_msg > 0)
  {
    sprintf(modestr,"¦¬¨ì%s«Ê°T®§",nums[up->num_msg-1]);
    return (modestr);
  }
#endif  
  
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
    return (word); /* Thor.980805: µù¸Ñ: Áô¨­ªº¤H¤£·|³Qª¾¹D¸ò½Ötalk */
  
  sprintf(modestr, "%s %s", word, up->mateid);
  return (modestr);
}


/* ----------------------------------------------------- */
/* ¦n¤Í¦W³æ¡G¬d¸ßª¬ºA¡B¤Í½Ë´y­z				 */
/* ----------------------------------------------------- */


/* Thor: ¶Ç¦^­È -> 0 for good, 1 for normal, 2 for bad */


#if 0
static int
pal_belong(userid, uno)
  char *userid;
  int uno;
{
  PAL *head, *tail;
  char *fimage = NULL, fpath[64];
  int fsize, ans;

  ans = 1;			/* §ä¤£¨ì¬° normal */

  if (*userid)
  {
    usr_fpath(fpath, userid, FN_PAL);

    fimage = f_map(fpath, &fsize);
    if (fimage != (char *) -1)
    {
      head = (PAL *) fimage;
      tail = (PAL *) (fimage + fsize);
      do
      {
	if (uno == head->userno)
	{
	  ans = head->ftype;	/* Thor: ­ì¬° 2-, ²{ª½±µ¶Ç¦^ */
	  break;
	}
      } while (++head < tail);
      munmap(fimage, fsize);
    }
  }

  return ans;
}
#endif

static void
copyship(ship,userno)
  char *ship;
  int userno;
{
  PAL_SHIP *shead;
  shead = pal_ship;
  if(userno == cuser.userno)
  {
    strcpy(ship,"¨º´N¬O§Ú");
	return;
  }
  if (shead)
  {
    while(shead->pal_no)
    {
      if(shead->pal_no == userno)
      {
        strcpy(ship,shead->ship);
        return;
      }
      shead++;
    } 
  }
  return;

}

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

#ifdef	HAVE_BANMSG
static int
can_banmsg(up)
  UTMP *up;
{
  int count, *cache, datum, mid;

  if ((cache = up->banmsg_spool))
  {
    for (count = up->banmsg_max; count > 0;)
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
#endif


/*static */int
can_message(up)
  UTMP *up;
{
  int self, ufo, can;

  if (up->userno == (self = cuser.userno))
    return NA;

  ufo = up->ufo;

//  if(ufo & UFO_REJECT)
//    return NA;

  if (HAS_PERM(PERM_SYSOP | PERM_ACCOUNTS|PERM_CHATROOM))	/* ¯¸ªø¡B±b¸¹ */
    return YEA;

#ifdef	HAVE_BANMSG
  if(can_banmsg(up))		/* ©Ú¦¬°T®§ */
    return NA;  
#endif  

  if ( ufo & (UFO_MESSAGE))	/* »·Â÷¹ÐÄÛ */
    return NA;

  if (!(ufo & UFO_QUIET))
    return YEA;			/* ©I¥s¾¹¨SÃö±¼¡A¥çµL·l¤Í */

  can = 0;			/* §ä¤£¨ì¬° normal */
  can = can_see(up);
  return (ufo & UFO_QUIET)
    ? can == 1			/* ¥u¦³¦n¤Í¥i¥H */
    : can < 2 			/* ¥u­n¤£¬O·l¤Í */ ;
}


static int
can_override(up)
  UTMP *up;
{
  int self, ufo, can;

  if (up->userno == (self = cuser.userno))
    return NA;

  ufo = up->ufo;
    
  if(ufo & (UFO_REJECT|UFO_NET))
    return NA;

  if (HAS_PERM(PERM_SYSOP | PERM_ACCOUNTS|PERM_CHATROOM))	/* ¯¸ªø¡B±b¸¹ */
    return YEA;

  if (ufo & UFO_PAGER1)		/* »·Â÷¹ÐÄÛ */
    return NA;

  if (!(ufo & UFO_PAGER))
    return YEA;			/* ©I¥s¾¹¨SÃö±¼¡A¥çµL·l¤Í */

  can = 0;			/* §ä¤£¨ì¬° normal */

  can = can_see(up);
  return (ufo & UFO_PAGER)
    ? can == 1			/* ¥u¦³¦n¤Í¥i¥H */
    : can < 2 /* ¥u­n¤£¬O·l¤Í */ ;
}

/* ----------------------------------------------------- */
/* ¦n¤Í¦W³æ¡G·s¼W¡B§R°£¡B­×§ï¡B¸ü¤J¡B¦P¨B		 */
/* ----------------------------------------------------- */

#ifdef	HAVE_BOARD_PAL
int
is_boardpal(up)
  UTMP *up;
{
  return cutmp->board_pal == up->board_pal;
}
#endif

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

#ifdef	HAVE_BANMSG
int
is_banmsg(userno)
  int userno;
{
  int count, *cache, datum, mid;

  if ((cache = cutmp->banmsg_spool))
  {
    for (count = cutmp->banmsg_max; count > 0;)
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
#endif

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
  int count, fsize, ufo ,fd;
  int *plist, *cache;
  int ship_total;
  PAL *phead, *ptail;
  PAL_SHIP *shead;
  char *fimage, fpath[64];
  UTMP *up;

  up = cutmp;
  cutmp->userno = cuser.userno;

  cache = NULL;
  ship_total = count = 0;
  ufo = cuser.ufo & ~( UFO_BIFF | UFO_BIFFN | UFO_REJECT | UFO_FCACHE);
  /* Thor.980805: ¸ò BIFF¦n¹³¨S¦³¤Ó¤jÃö«Y */

  fsize = 0;
  usr_fpath(fpath, cuser.userid, FN_PAL);
  fimage = f_img(fpath, &fsize);
  if((fsize > (PAL_MAX * sizeof(PAL))) && (fd = open(fpath,O_RDWR)))
  {
    ftruncate(fd, PAL_MAX * sizeof(PAL));
    close(fd);
  }
  if (fimage != NULL)
  {
    if (fsize > (PAL_MAX * sizeof(PAL)))
    {
      sprintf(fpath, "%-13s%d > %d * %d\n", cuser.userid, fsize, PAL_MAX, sizeof(PAL));
      f_cat(FN_PAL_LOG, fpath);
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
	if (strlen(phead->ship) > 0)
	  ship_total++;
      } while (++phead < ptail);

      if (count > 0)
      {
	ufo |= UFO_FCACHE;
	if (count > 1)
	  xsort(cache, count, sizeof(int), (void *)int_cmp);
      }
      else
      {
	cache = NULL;
      }
      
    }
  }

  pal_pool = cache;
  up->pal_max = pal_count = count;
  if (cutmp)
  {
    ufo = (ufo & ~(UFO_UTMP_MASK | UFO_REJECT)) | (cutmp->ufo & UFO_UTMP_MASK);
    /* Thor.980805: ¸Ñ¨M cutmp.ufo©M cuser.ufoªº¦P¨B°ÝÃD */
    cutmp->ufo = ufo & (~UFO_REJECT);
  }
  if(pal_ship)
  {
    free(pal_ship);
    pal_ship = NULL;
  }
  
  
  if(ship_total)
  {
     shead = pal_ship = (PAL_SHIP *)malloc((ship_total+1)*sizeof(PAL_SHIP));  
     phead = (PAL *) fimage;
     ptail = (PAL *) (fimage + fsize);
     if(pal_ship)
     {
       memset(pal_ship,0,(ship_total+1)*sizeof(PAL_SHIP));
       do
       {
         if(strlen(phead->ship)>0)
         {
           strcpy(shead->ship,phead->ship);
           shead->pal_no = phead->userno;
           shead++;
         }
       } while (++phead < ptail);  
     }
     else
       pal_ship = NULL;
  }
  else
    pal_ship = NULL;
  
  if(fimage) free(fimage);
  cuser.ufo = ufo;
}

/* 2003.01.01 verit : ­«¾ã ¤W¯¸³qª¾¦W³æ */
void 
aloha_sync(void)
{
  char fpath[128];
  int fd,size=0;
  struct stat st;

  usr_fpath(fpath, cuser.userid, FN_FRIEND_BENZ);
  if ((fd = open(fpath, O_RDWR, 0600)) < 0)
      return;

  outz("¡¹ ¸ê®Æ¾ã²z½]®Ö¤¤¡A½Ðµy­Ô \033[5m...\033[m");
  refresh();

  if (!fstat(fd, &st) && (size = st.st_size) > 0)
  {
    BMW *pbase, *phead, *ptail;
    int userno;

    pbase = phead = (BMW *) malloc(size);
    size = read(fd, pbase, size);
    if (size >= sizeof(BMW))
    {
      ptail = (BMW *) ((char *) pbase + size);
      while (phead < ptail)
      {
        userno = phead->recver;
        if (userno > 0 && userno == acct_userno(phead->userid))
        {
          phead++;
          continue;
        }
        ptail--;
        if (phead >= ptail) break;
        memcpy(phead, ptail, sizeof(BMW));
      }

      size = (char *) ptail - (char *) pbase;
      if (size > 0)
      {
        //xsort(pbase, size / sizeof(BMW), sizeof(BMW), str_cmp);
        lseek(fd, 0, SEEK_SET);
        write(fd, pbase, size);
        ftruncate(fd, size);
      }
    }
    free(pbase);
  }
  close(fd);

  if (size <= 0)
    unlink(fpath);
}
      

void
pal_sync(fpath)
  char *fpath;
{
  int fd, size=0;
  struct stat st;
  char buf[64];

  if (!fpath)
  {
    fpath = buf;
    usr_fpath(fpath, cuser.userid, FN_PAL);
  }

  if ((fd = open(fpath, O_RDWR, 0600)) < 0)
    return;

  outz("¡¹ ¸ê®Æ¾ã²z½]®Ö¤¤¡A½Ðµy­Ô \033[5m...\033[m");
  refresh();

  if (!fstat(fd, &st) && (size = st.st_size) > 0)
  {
    PAL *pbase, *phead, *ptail;
    int userno;

    pbase = phead = (PAL *) malloc(size);
    size = read(fd, pbase, size);
    if (size >= sizeof(PAL))
    {
      ptail = (PAL *) ((char *) pbase + size);
      while (phead < ptail)
      {
	userno = phead->userno;
	if (userno > 0 && userno == acct_userno(phead->userid))
	{
	  phead++;
	  continue;
	}

	ptail--;
	if (phead >= ptail)
	  break;
	memcpy(phead, ptail, sizeof(PAL));
      }

      size = (char *) ptail - (char *) pbase;
      if (size > 0)
      {
	if (size > sizeof(PAL))
	{
	  if (size > PAL_ALMR * sizeof(PAL))
	    vmsg("±zªº¦n¤Í¦W³æ¤Ó¦h¡A½Ðµ½¥[¾ã²z");
	  xsort(pbase, size / sizeof(PAL), sizeof(PAL), (void *)str_cmp);
	}

	/* Thor.0709: ¬O§_­n¥[¤W®ø°£­«ÂÐªº¦n¤Íªº°Ê§@? */

	lseek(fd, 0, SEEK_SET);
	write(fd, pbase, size);
	ftruncate(fd, size);
      }
    }
    free(pbase);
  }
  close(fd);

  if (size <= 0)
    unlink(fpath);
}


/* ----------------------------------------------------- */
/* ¦n¤Í¦W³æ¡G¿ï³æ¦¡¾Þ§@¬É­±´y­z				 */
/* ----------------------------------------------------- */


static int pal_add();


static void
pal_item(num, pal)
  int num;
  PAL *pal;
{
  prints("%6d %-3s%-14s%s\n", num, pal->ftype & PAL_BAD ? "¢æ" : "",
    pal->userid, pal->ship);
}


static int
pal_body(xo)
  XO *xo;
{
  PAL *pal;
  int num, max, tail;

  max = xo->max;
  if (max <= 0)
  {
    if (vans("­n¥æ·sªB¤Í¶Ü(Y/N)¡H[N] ") == 'y')
      return pal_add(xo);
    return XO_QUIT;
  }

  pal = (PAL *) xo_pool;
  num = xo->top;
  tail = num + XO_TALL;
  if (max > tail)
    max = tail;

  move(3, 0);
  do
  {
    pal_item(++num, pal++);
  } while (num < max);
  clrtobot();

  return XO_NONE;
}


static int
pal_head(xo)
  XO *xo;
{
  vs_head("¦n¤Í¦W³æ", str_site);
  outs("\
  [¡ö]Â÷¶} a)·s¼W c)­×§ï d)§R°£ m)±H«H s)¾ã²z [/?]·j´M [q]¬d¸ß [h]elp\n\
\033[30;47m  ½s¸¹    ¥N ¸¹         ¤Í       ½Ë                                           \033[m");
  return pal_body(xo);
}


static int
pal_load(xo)
  XO *xo;
{
  xo_load(xo, sizeof(PAL));
  return pal_body(xo);
}


static int
pal_init(xo)
  XO *xo;
{
  xo_load(xo, sizeof(PAL));
  return pal_head(xo);
}


static void
pal_edit(pal, echo)
  PAL *pal;
  int echo;
{
  if (echo == DOECHO)
    memset(pal, 0, sizeof(PAL));
  vget(b_lines, 0, "¤Í½Ë¡G", pal->ship, sizeof(pal->ship), echo);
  pal->ftype = vans("·l¤Í(Y/N)¡H[N] ") == 'y' ? PAL_BAD : 0;
}


static int
pal_search(xo, step)
  XO *xo;
  int step;
{
  int num, pos, max;
  static char buf[IDLEN + 1];
  int fsize;
  PAL *phead;
  char *fimage = NULL, fpath[64];

  if (vget(b_lines, 0, msg_uid, buf, IDLEN + 1, GCARRY))
  {
    int buflen;
    char bufl[IDLEN + 1];

    usr_fpath(fpath, cuser.userid, FN_PAL);
    fimage = f_img(fpath, &fsize);
    phead = (PAL *) fimage;

    str_lower(bufl, buf);
    buflen = strlen(bufl);
    
    pos = num = xo->pos;
    max = xo->max;
    do
    {
      pos += step;
      if (pos < 0) 
        pos = max - 1;
      else if (pos >= max)
	    pos = 0;
   
      if (str_str((phead + pos)->userid, bufl) || str_str((phead + pos)->ship, bufl))
      {
		move(b_lines, 0);
		clrtoeol();
	    if(fimage) 
		  free(fimage);
		return pos + XO_MOVE;
      }

    } while (pos != num);
  }
  if(fimage) 
    free(fimage);

  return XO_FOOT;
}

static int
pal_search_forward(xo)
  XO *xo;
{
  return pal_search(xo, 1); /* step = +1 */
}

static int
pal_search_backward(xo)
  XO *xo;
{
  return pal_search(xo, -1); /* step = -1 */
}


static int
pal_add(xo)
  XO *xo;
{
  ACCT acct;
  int userno;

  if (xo->max >= PAL_MAX)
  {
    vmsg("±zªº¦n¤Í¦W³æ¤Ó¦h¡A½Ðµ½¥[¾ã²z");
    return XO_FOOT;
  }

  userno = acct_get(msg_uid, &acct);

  /* jhlin : for board_moderator, temporarily */

#if 1				/* Thor.0709: ¤£­«ÂÐ¥[¤J */
  /* if (is_pal(userno)) */
  if ((xo->dir[0] == 'u') && (is_pal(userno) || is_bad(userno)))
              /* lkchu.981201: ¥u¦³¦b moderator board ¤~¥i­«ÂÐ¥[¤J */
  {
    vmsg("¦W³æ¤¤¤w¦³¦¹¦n¤Í");
    return XO_FOOT;
  }
  else if (userno == cuser.userno)      /* lkchu.981201: ¦n¤Í¦W³æ¤£¥i¥[¦Û¤v */
  {
    vmsg("¦Û¤v¤£¶·¥[¤J¦n¤Í¦W³æ¤¤");
    return XO_FOOT;
  }
#endif

  if ((userno > 0) /* && !pal_state(cutmp, userno, NULL) */ )
  {
    PAL pal;

    pal_edit(&pal, DOECHO);
    strcpy(pal.userid, acct.userid);
    pal.userno = userno;
    rec_add(xo->dir, &pal, sizeof(PAL));
#if 0
    if ((xo->dir[0] == 'u') && (pal.userno != cuser.userno) && !(pal.ftype & PAL_BAD))
    {
      char fpath[64];
      BMW bmw;

      /* bmw.caller = cutmp; */		/* lkchu.981201: frienz ¤¤ utmp µL®Ä */
      bmw.recver = cuser.userno;
      strcpy(bmw.userid, cuser.userid);
      usr_fpath(fpath, pal.userid, FN_FRIEND_BENZ);
      rec_add(fpath, &bmw, sizeof(BMW));
    }
#endif
    xo->pos = XO_TAIL /* xo->max */ ;
    xo_load(xo, sizeof(PAL));
  }

#if 1				/* Thor.0709: ¦n¤Í¦W³æ¦P¨B */
  pal_cache();
#endif

  return pal_head(xo);
}





static int
pal_delete(xo)
  XO *xo;
{
  if (vans(msg_del_ny) == 'y')
  {
#if 0
    if (xo->dir[0] == 'u')
    {
      PAL *pal;

      pal = (PAL *) xo_pool + (xo->pos - xo->top);
      if (!(pal->ftype & PAL_BAD))
      {
        char fpath[64];

        usr_fpath(fpath, pal->userid, FN_FRIEND_BENZ);
        rec_del(fpath, sizeof(BMW), 0, cmpbmw, NULL);
      }
    }
#endif

    if (!rec_del(xo->dir, sizeof(PAL), xo->pos, NULL, NULL))
    {

#if 1				/* Thor.0709: ¦n¤Í¦W³æ¦P¨B */
      pal_cache();
#endif

      return pal_load(xo);
    }
  }
  return XO_FOOT;
}


static int
pal_change(xo)
  XO *xo;
{
  PAL *pal, mate;
  int pos, cur;
#if 0
  char old_ftype, fpath[64];
#endif
  
  pos = xo->pos;
  cur = pos - xo->top;
  pal = (PAL *) xo_pool + cur;

#if 0
  old_ftype = pal->ftype;
  usr_fpath(fpath, pal->userid, FN_FRIEND_BENZ);
#endif
    
  mate = *pal;
  pal_edit(pal, GCARRY);
  if (memcmp(pal, &mate, sizeof(PAL)))
  {
    rec_put(xo->dir, pal, sizeof(PAL), pos);
    move(3 + cur, 0);
    pal_item(++pos, pal);
  }

#if 0
  if (xo->dir[0] == 'b')	/* lkchu.981201: moderator board */
    return XO_FOOT;
  
  if ( !(old_ftype & PAL_BAD) && (pal->ftype & PAL_BAD) )
      /* lkchu.981201: ­ì¥»¬O¦n¤Í, §ï¦¨·l¤Í­n rec_del */
  {
    rec_del(fpath, sizeof(BMW), 0, cmpbmw, NULL);
  }
  else if ( (old_ftype & PAL_BAD) && !(pal->ftype & PAL_BAD) )
      /* lkchu.981201: ­ì¥»¬O·l¤Í, §ï¦¨¦n¤Í­n rec_add */
  {
    BMW bmw;

    /* bmw.caller = cutmp; */	/* lkchu.981201: frienz ¤¤ utmp µL®Ä */
    bmw.recver = cuser.userno;
    strcpy(bmw.userid, cuser.userid);
    rec_add(fpath, &bmw, sizeof(BMW));
  }  
#endif

  return XO_FOOT;
}


static int
pal_mail(xo)
  XO *xo;
{
  PAL *pal;
  char *userid;

  pal = (PAL *) xo_pool + (xo->pos - xo->top);
  userid = pal->userid;
  if (*userid)
  {
    vs_bar("±H  «H");
    prints("¦¬«H¤H¡G%s", userid);
    my_send(userid);
  }
  return pal_head(xo);
}


static int
pal_sort(xo)
  XO *xo;
{
  pal_sync(xo->dir);
  return pal_load(xo);
}


static int
pal_query(xo)
  XO *xo;
{
  PAL *pal;

  pal = (PAL *) xo_pool + (xo->pos - xo->top);
  move(1, 0);
  clrtobot();
  /* move(2, 0); *//* Thor.0810: ¥i¥H¤£¥[¶Ü? */
  /* if(*pal->userid) *//* Thor.0806:°¸º¸¦nª±¤@¤U, À³¸Ó¨S®t */
  my_query(pal->userid, 1);
  return pal_head(xo);
}


static int
pal_help(xo)
  XO *xo;
{
  film_out(FILM_PAL, -1);
  return pal_head(xo);
}


KeyFunc pal_cb[] =
{
  {XO_INIT, pal_init},
  {XO_LOAD, pal_load},
  {XO_HEAD, pal_head},
  {XO_BODY, pal_body},

  {'a', pal_add},
  {'c', pal_change},
  {'d', pal_delete},
  {'m', pal_mail},
  {'q', pal_query},
  {'s', pal_sort},
  {'/', pal_search_forward},
  {'?', pal_search_backward},
#if 0
  {'w' | XO_DL, (int (*)())"bin/bbcall.so:pal_bbc"},
#endif

  {'h', pal_help}
};


int
t_pal()
{
  XO *xo;
  char fpath[64];

  usr_fpath(fpath, cuser.userid, FN_PAL);
  xz[XZ_PAL - XO_ZONE].xo = xo = xo_new(fpath);
  xover(XZ_PAL);
  pal_cache();
  free(xo);

  return 0;
}


#if 0
int
t_pal()
{
  XO *xo;

  xo = pal_xo;
  if (xo == NULL)
  {
    char fpath[64];

    usr_fpath(fpath, cuser.userid, FN_PAL);
    pal_xo = xo = xo_new(fpath);
  }
  xover(XZ_PAL);
  pal_cache();
  return 0;
}
#endif


/* ----------------------------------------------------- */
/* °T®§¦Cªí: ¿ï³æ¦¡¾Þ§@¬É­±´y­z by lkchu                 */
/* ----------------------------------------------------- */


/*static */void bmw_edit();


static void
bmw_item(num, bmw)
  int num;
  BMW *bmw;
{
  struct tm *ptime = localtime(&bmw->btime);

  if(!(bmw_modetype & BMW_MODE))
  {
    if (bmw->sender == cuser.userno)
    {
      /* lkchu.990206: ¦n¤Í¼s¼· */
      if (!(*bmw->userid))
        strcpy(bmw->userid, "²³®a¦n¤Í");
      
      prints("%5d %02d:%02d %-13s¡¸%-50.50s\n", num, ptime->tm_hour, ptime->tm_min,
        bmw->userid, bmw->msg);
    }
    else
    {
      if(strstr(bmw->msg,"¡¹¼s¼½"))
        prints("%5d \033[36;1m%02d:%02d %-13s¡¹%-50.50s\033[m\n", num, ptime->tm_hour, ptime->tm_min,
          bmw->userid, (bmw->msg)+8);
      else
        prints("%5d \033[32m%02d:%02d %-13s¡¹%-50.50s\033[m\n", num, ptime->tm_hour, ptime->tm_min,
          bmw->userid, bmw->msg);
    }
  }
  else
  {
    if (bmw->sender == cuser.userno)
    {
      if (!(*bmw->userid))
        strcpy(bmw->userid, "²³®a¦n¤Í");

      prints("%5d %-13s¡¸%-57.57s\n", num,bmw->userid, bmw->msg);
    }
    else
    {
      if(strstr(bmw->msg,"¡¹¼s¼½"))
        prints("%5d \033[36;1m%-13s¡¹%-57.57s\033[m\n", num,
          bmw->userid, (bmw->msg)+8);
      else
        prints("%5d \033[32m%-13s¡¹%-57.57s\033[m\n", num,
          bmw->userid, bmw->msg);
    }  
  }
}


static int
bmw_body(xo)
  XO *xo;
{
  BMW *bmw;
  int num, max, tail;

  move(3, 0);
  clrtobot();
  max = xo->max;
  if (max <= 0)
  {
    vmsg("¥ý«e¨ÃµL¼ö°T©I¥s");
    return XO_QUIT;
  }

  bmw = (BMW *) xo_pool;
  num = xo->top;
  tail = num + XO_TALL;
  if (max > tail)
    max = tail;

  do
  {
    bmw_item(++num, bmw++);
  } while (num < max);

  return XO_NONE;
}


static int
bmw_head(xo)
  XO *xo;
{
  vs_head("¹î¬Ý°T®§", str_site);
  if(bmw_modetype & BMW_MODE)
  {
    outs("\
  [¡ö]Â÷¶}  [d]§R°£  [m]±H«H  [w]§Ö°T  [s]§ó·s  [¡÷]¬d¸ß  [h]elp\n\
\033[30;47m ½s¸¹ ¥N ¸¹        ¤º       ®e                                                \033[m");
  }
  else
  {
    outs("\
  [¡ö]Â÷¶}  [d]§R°£  [m]±H«H  [w]§Ö°T  [s]§ó·s  [¡÷]¬d¸ß  [h]elp\n\
\033[30;47m ½s¸¹ ®É ¶¡ ¥N ¸¹        ¤º       ®e                                          \033[m");
  }
  return bmw_body(xo);
}


static int
bmw_load(xo)
  XO *xo;
{
  xo_load(xo, sizeof(BMW));
  return bmw_body(xo);
}


static int
bmw_init(xo)
  XO *xo;
{
  xo_load(xo, sizeof(BMW));
  return bmw_head(xo);
}


static int
bmw_delete(xo)
  XO *xo;
{
  if (vans(msg_del_ny) == 'y')
    if (!rec_del(xo->dir, sizeof(BMW), xo->pos, NULL, NULL))
      return bmw_load(xo);

  return XO_FOOT;
}


static int
bmw_mail(xo)
  XO *xo;
{
  BMW *bmw;
  char *userid;

  bmw = (BMW *) xo_pool + (xo->pos - xo->top);
  userid = bmw->userid;
  if (*userid)
  {
    vs_bar("±H  «H");
    prints("¦¬«H¤H¡G%s", userid);
    my_send(userid);
  }
  return bmw_head(xo);
}


static int
bmw_query(xo)
  XO *xo;
{
  BMW *bmw;

  bmw = (BMW *) xo_pool + (xo->pos - xo->top);
  move(1, 0);
  clrtobot();
  /* move(2, 0); *//* Thor.0810: ¥i¥H¤£¥[¶Ü? */
  /* if(*pal->userid) *//* Thor.0806:°¸º¸¦nª±¤@¤U, À³¸Ó¨S®t */
  my_query(bmw->userid, 1);
  return bmw_head(xo);
}


static int
bmw_write(xo)
  XO *xo;
{
  if (HAS_PERM(PERM_PAGE))
  {
    UTMP *up = NULL;
    BMW *benz;

    benz = (BMW *) xo_pool + (xo->pos - xo->top);
    if( (benz->caller >= ushm->uslot && benz->caller < ushm->uslot + MAXACTIVE) && (benz->caller && benz->caller->userno == benz->sender) && can_message(benz->caller))
    {
      up = benz->caller;
    }
    
    if ((up || (up = utmp_find(benz->sender))) && can_message(up))
        /* lkchu.990104: Åý¥H«eªº bmw ¤]¥i¥H¦^ */
    {
      if (((up->ufo & UFO_CLOAK) && !HAS_PERM(PERM_SEECLOAK))||(can_see(up)==2 && !HAS_PERM(PERM_SYSOP))||benz->sender==0)
      {
        return XO_NONE;		/* lkchu.990111: Áô§ÎÅo */
      }
      else
      {
        BMW bmw;
        char buf[20];
#ifdef  HAVE_SHOWNUMMSG
        if(up->num_msg > 9 && (up->ufo & UFO_MAXMSG) && !HAS_PERM(PERM_SYSOP))
        {
          vmsg("¹ï¤è¤w¸g³Q¤ô²yÄéÃz¤F!!");
          return XO_INIT;
        }
#endif
        sprintf(buf, "¡¹[%s]", up->userid);
        bmw_edit(up, buf, &bmw, 0);
      }
    }
#ifdef	HAVE_BANMSG    
    else if(up && !(up->ufo & UFO_MESSAGE) && can_banmsg(up))
    {
      vmsg("¹ï¤è¤£·QÅ¥¨ì±zªºÁn­µ!!");
      return XO_INIT;
    }
#endif    
  }
  return XO_NONE;
}

static int
bmw_mode(xo)
  XO *xo;
{
  bmw_modetype ^= BMW_MODE;
  return bmw_init(xo);
}


static int
bmw_help(xo)
  XO *xo;
{
  film_out(FILM_BMW, -1);
  return bmw_head(xo);
}


KeyFunc bmw_cb[] =
{
  {XO_INIT, bmw_init},
  {XO_LOAD, bmw_load},
  {XO_HEAD, bmw_head},
  {XO_BODY, bmw_body},
  
  {'d', bmw_delete},
  {'m', bmw_mail},
  {'w', bmw_write},
  {'q', bmw_query},
  {Ctrl('Q'), bmw_query},
  {'s', bmw_init},
  {KEY_TAB, bmw_mode}, 
  {'h', bmw_help}
};


int
t_bmw()
{
  xover(XZ_BMW);
  return 0;
}


#ifdef HAVE_MODERATED_BOARD
/* ----------------------------------------------------- */
/* ªO¤Í¦W³æ¡Gmoderated board				 */
/* ----------------------------------------------------- */


#define FN_FIMAGE	"fimage"


static void
bm_image()
{
  int fd;
  char fpath[80];

  brd_fpath(fpath, currboard, FN_PAL);
  if ((fd = open(fpath, O_RDONLY)) >= 0)
  {
    struct stat st;
    PAL *pal, *up;
    int count;

    fstat(fd, &st);
    if ((pal = (PAL *) malloc(count = st.st_size)))
    {
      count = read(fd, pal, count) / sizeof(PAL);
      if (count > 0)
      {
	int *userno,*ubase;
	int c = count;
	
	ubase = userno = (int *)malloc(count*sizeof(int));

	up = pal;
	do
	{
#ifdef HAVE_WATER_LIST
	  *userno++ = (up->ftype == PAL_BAD) ? -(up->userno) : up->userno;
#else
	  *userno++ = up->userno;
#endif	  
	  up++;
	} while (--c);

	if (count > 1)		/* Thor: ¦h±Æ§Ç¦³¯q¨­Åé°·±d... */
	  xsort(ubase, count, sizeof(int), int_cmp);

	brd_fpath(fpath, currboard, FN_FIMAGE);
	if ((count = open(fpath, O_WRONLY | O_CREAT | O_TRUNC, 0600)) >= 0)
	{
	  write(count, ubase, (char *) userno - (char *) ubase);
	  close(count);
	  free(ubase);
	}
      }
      else  /* Thor.980811: lkchu patch: »P friend ¦P¨B */
      {
	brd_fpath(fpath, currboard, FN_FIMAGE);
        unlink(fpath);
      }
      free(pal);
    }
    close(fd);
  }
}


int
bm_belong(board)
  char *board;
{
  int fsize, count, result,wcount;
  char fpath[80];

  result = 0;

  brd_fpath(fpath, board, FN_FIMAGE);	/* Thor: fimage­n¥ý sort¹L! */
  board = f_img(fpath, &fsize);

  if (board != NULL)
  {
    wcount = count = fsize / sizeof(int);

    if (count > 0)
    {
      int userno, *up;
      int datum, mid;

      userno = cuser.userno;
      up = (int *) board;

      while (count > 0)
      {
	datum = up[mid = count >> 1];
	if (userno == datum)
	{
	  result = BRD_R_BIT | BRD_W_BIT;
	  break;
	}
	if (userno > datum)
	{
	  up += (++mid);
	  count -= mid;
	}
	else
	{
	  count = mid;
	}
      }
#ifdef HAVE_WATER_LIST
      up = (int *) board;
      while (wcount > 0)
      {
        datum = up[mid = wcount >> 1];
        if(-userno == datum)
        {
          result = BRD_R_BIT;
          break;
        }
        if (-userno > datum)
        {
          up += (++mid);
          wcount -= mid;
        }
        else
        {
          wcount = mid;
        }
      }
#endif
    }
    free(board);
  }
  return result;
}


int
XoBM(xo)
  XO *xo;
{
  if ((bbstate & STAT_BOARD) /*&& (bbstate & STAT_MODERATED)*/)
    /* && (cbhdr.readlevel == PERM_SYSOP)) */
    /*
     * Thor.1020: bbstate¦³ STAT_MODERATED´N¥Nªí²Å¦XMODERATED BOARD,
     * ¤£»Ý¦Acheck readlevel PERM_SYSOP
     */
  {
    XO *xt;
    char fpath[80];

    brd_fpath(fpath, currboard, FN_PAL);
    xz[XZ_PAL - XO_ZONE].xo = xt = xo_new(fpath);
    xover(XZ_PAL);		/* Thor: ¶ixover«e, pal_xo ¤@©w­n ready */

    /* build userno image to speed up, maybe upgreade to shm */

    bm_image();

    free(xt);

    return XO_INIT /* or post_init(xo) */ ;
  }
  return XO_NONE;
}
#endif


/* ------------------------------------- */
/* ¯u¹ê°Ê§@				 */
/* ------------------------------------- */


static void
showplans(userid)
  char *userid;
{
  int i;
  FILE *fp;
  char buf[256];

  usr_fpath(buf, userid, FN_PLANS);

  if ((fp = fopen(buf, "r")))
  {
    move(7,0);
//    outs(" [¦W¤ù]¡G\n\n");
    i = MAXQUERYLINES;

    while (i-- && fgets(buf, sizeof(buf), fp))
    {
      outx(buf);
    }
    fclose(fp);
  }
  else
  {
//    outs(" [¨S¦³¦W¤ù]\n\n");
  }
}


static void
do_query(acct, paling)
  ACCT *acct;
  int paling;			/* ¬O§_¥¿¦b³]©w¦n¤Í¦W³æ */
{
  UTMP *up;
  int userno,mail, rich;
  char *userid;

  utmp_mode(M_QUERY);
  userno = acct->userno;
  userid = acct->userid;
  strcpy(cutmp->mateid, userid);

  prints(" %s(%s) ¤W¯¸ %d ¦¸¡A¤å³¹ %d ½g¡A%s»{ÃÒ¡C\n",
    userid, (HAS_PERM(PERM_SYSOPX) || !(acct->userlevel & PERM_DENYNICK)) ? acct->username : GUEST_1, acct->numlogins, acct->numposts,
    acct->userlevel & PERM_VALID ? "¤w" : "¥¼");

  prints(" ¤W¦¸(\033[1;33m%s\033[m)¨Ó¦Û(%s)\n", 
  Ctime(&acct->lastlogin), ((acct->ufo & UFO_HIDEDN)&&!HAS_PERM(PERM_SYSOP)) ? 
  HIDEDN_SRC : acct->lasthost);

#if defined(REALINFO) && defined(QUERY_REALNAMES)
  if (HAS_PERM(PERM_BASIC))
    prints(" ¯u¹ê©m¦W: %s", acct->realname);
  else
#endif
    if (HAS_PERM(PERM_SYSOP))
      prints(" ¯u¹ê©m¦W: %s", acct->realname);
    
  /* °²³]¤W¦¸ login ¤w¹L 6 ¤p®É¡A«K¤£¦b¯¸¤W¡A´î¤Ö utmp_find */
  
  outs(" [°ÊºA] ");
  /*  up = (acct->lastlogin < time(0) - 6 * 3600) ? NULL : utmp_find(userno);  */
  up = utmp_find(userno);
  outs("\033[1;36m");
  outs((!up || (up && ((!HAS_PERM(PERM_SEECLOAK) && (up->ufo & UFO_CLOAK)) || (can_see(up)==2)) && !HAS_PERM(PERM_SYSOP))) ? "¤£¦b¯¸¤W":bmode(up, 1));
  outs("\033[m");
  /* Thor.981108: ¬°º¡¨¬ cigar ¹ý©³Áô¨­ªº­n¨D, ¤£¹L¥Î fingerÁÙ¬O¥i¥H¬Ý¨ì:p */
  
#if 0
  /* Query ®É¥i¦P®É¬Ý¨ä¤Í½Ë´y­z©Î´c¦æ */

  if ((!paling) && (state = pal_state(cutmp, userno, ship)))
  {
    prints(" [%s¤Í] %s", state > 0 ? "¦n" : "·l", ship);
  }
#endif

  mail = m_query(userid);
  if(mail)
    prints(" [«H½c] \033[1;31m¦³ %d «Ê·s±¡®Ñ\033[m\n",mail);
  else
    prints(" [«H½c] ³£¬Ý¹L¤F\n");

  char fortune[7][9] = {"½a§xªü¦v", "®a¹Ò´¶³q", "®a¹Ò¤p±d", "®a¹Ò´I¦³", "°]¤O¶¯«p", "´I¥i¼Ä°ê", "I'm Rich"};

  if     (acct->money >= 100000000)
    rich=6;
  else if(acct->money >=  10000000)
    rich=5;
  else if(acct->money >=   1000000)
    rich=4;    
  else if(acct->money >=    100000)
    rich=3;
  else if(acct->money >=     10000)
    rich=2; 
  else if(acct->money >=      1000)
    rich=1; 
  else
    rich=0;

  if (acct->point1 > 10) 
    prints(" [Àu¨}¿n¤À] \033[1;32m%d\033[m [¦H¤å] %d [¸gÀÙ] %s", acct->point1, acct->point2, fortune[rich]);
  else if (acct->point2 > 1) 
    prints(" [Àu¨}¿n¤À] %d [¦H¤å] \033[1;31m%d\033[m [¸gÀÙ] %s", acct->point1, acct->point2, fortune[rich]);
  else
    prints(" [Àu¨}¿n¤À] %d [¦H¤å] %d [¸gÀÙ] %s", acct->point1, acct->point2, fortune[rich]);

  if (paling == 2) 
    ;
  else
  {
    /* deny nick can't have plans. statue.2001.02.11 */
    if(HAS_PERM(PERM_SYSOPX)) 
      showplans(userid);
    else if(!(acct->userlevel & PERM_DENYNICK) && (acct->userlevel & PERM_VALID))
      showplans(userid);
  }   
  
  vmsg(NULL);

  move(b_lines, 0);
  clrtoeol();
}


void
my_query(userid, paling)
  char *userid;
  int paling;			/* ¬O§_¥¿¦b³]©w¦n¤Í¦W³æ */
{
  ACCT acct;

  if (acct_load(&acct, userid) >= 0)
  {
    do_query(&acct, paling);
  }
  else
  {
    vmsg(err_uid);
  }
}


/* ----------------------------------------------------- */
/* BMW : bbs message write routines			 */
/* ----------------------------------------------------- */


#define	BMW_FORMAT	"\033[1;33;46m¡¹%s \033[37;45m %s \033[m"
#define BMW_FORMAT_BC	"\033[1;37;45m¡¹%s \033[1;33;46m %s \033[m"
/* patch by visor : BMW_LOCAL_MAX >= BMW_PER_USER 
   ¥H§K¶i¤JµL½u°j°é                               */
#define	BMW_LOCAL_MAX	10	


static BMW bmw_lslot[BMW_LOCAL_MAX],bmw_sentlot[BMW_LOCAL_MAX];
//static BMW bmw_lslot[BMW_LOCAL_MAX];
static int bmw_locus;

static int
bmw_send(callee, bmw)
  UTMP *callee;
  BMW *bmw;
{
  BMW *mpool, *mhead, *mtail, **mslot;
  int i;
  pid_t pid;
  time_t texpire;

  if ((callee->userno != bmw->recver) || (pid = callee->pid) <= 0)
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
  /* Thor.981206: »Ýª`·N, ­Yushm mapping¤£¦P, 
                  «h¤£¦P°¦ bbsd ¤¬call·|core dump,
                  °£«D³o¤]¥Îoffset, ¤£¹L°£¤F -i, À³¸Ó¬O«D¥²­n */


  /* sem_lock(BSEM_LEAVE); */
  return kill(pid, SIGUSR2);
}


/*static */void
bmw_edit(up, hint, bmw, cc)
  UTMP *up;
  char *hint;
  BMW *bmw;
  int cc;
{
  uschar *str;
  screenline sl[2];

  if (up)
    bmw->recver = up->userno;	/* ¥ý°O¤U userno §@¬° check */

  if (!cc)
    save_foot(sl);

  str = bmw->msg;
  
  memset(str,0,sizeof(bmw->msg));
  
  str[0] = cc;
  str[1] = '\0';

  if (vget(b_lines - 1, 0, hint, str, 58 , GCARRY) &&
                                     /* lkchu.990103: ·s¤¶­±¥u¤¹³\ 48 ­Ó¦r¤¸ */
    vans("½T©w­n°e¥X¡m¼ö°T¡n¶Ü(Y/N)¡H[Y] ") != 'n')
  {
#if 0
    FILE *fp;
#endif
    char *userid, fpath[64];


    bmw->caller = cutmp;
    bmw->sender = cuser.userno;
    strcpy(bmw->userid, userid = cuser.userid);

    if (up)
    {
      if (bmw_send(up, bmw))
      {
	vmsg(MSG_USR_LEFT);
        if (!cc)
          restore_foot(sl);	
	return;
      }
    }

    /* lkchu.981230: §Q¥Î xover ¾ã¦X bmw */
    if (up)
      strcpy(bmw->userid, up->userid);	
				/* lkchu.990103: ­Y¬O¦Û¤v°e¥Xªº bmw, ¦s¹ï¤èªº userid */
    else
      *bmw->userid = '\0';	/* lkchu.990206: ¦n¤Í¼s¼·³]¬° NULL */
      
    time(&bmw->btime);
    usr_fpath(fpath, userid, FN_BMW);
    rec_add(fpath, bmw, sizeof(BMW));
    strcpy(bmw->userid, userid);

#if 0
    /* Thor.0722: msg file¥[¤W¦Û¤v»¡ªº¸Ü */

    usr_fpath(fpath, userid, FN_BMW);
    if (fp = fopen(fpath, "a"))
    {

#ifndef BMW_TIME
      fprintf(fp, "¡¸%s¡G%s\n", up ? up->userid : "²³®a¦n¤Í", str);
#else  
      /* Thor.980821: ¼ö°T°O¿ý¥[¤W®É¶¡ */
      fprintf(fp, "¡¸%s%s¡G%s\n", up ? up->userid : "²³®a¦n¤Í", bmw_timemsg(),  str);
#endif

      fclose(fp);
    }
#endif
  }

  if (!cc)
    restore_foot(sl);
}

#if 0
/* lkchu.981201: ¿ï¾Ü½u¤W¨Ï¥ÎªÌ¶Ç°T®§ */
static int
bmw_choose()
{
  UTMP *up, *ubase, *uceil;
  int self, seecloak;
  char userid[IDLEN + 1];

  ll_new();

  self = cuser.userno;
  seecloak = HAS_PERM(PERM_SEECLOAK);
  ubase = up = ushm->uslot;
  uceil = ubase + ushm->count;
  do
  {
    if (up->pid && up->userno != self && str_cmp(up->userid, STR_GUEST) &&
      ((seecloak || !(up->ufo & UFO_CLOAK)) || (can_see(up)==2 && !HAS_PERM(PERM_SYSOP))))
      ll_add(up->userid);
  } while (++up <= uceil);
#if 0
  if (!vget(1, 0, "½Ð¿é¤J¥N¸¹(«öªÅ¥ÕÁä¦C¥X¯¸¤W¨Ï¥ÎªÌ)¡G", userid, IDLEN + 1, GET_LIST))
  {
    vmsg(err_uid);
    return 0;
  }
#endif  
  
  up = ubase;
  do
  {
    if (!str_cmp(userid, up->userid))
    {
     if (HAS_PERM(PERM_PAGE) && can_message(up))
     {
       BMW bmw;
       char buf[20];

       sprintf(buf, "¡¹[%s]", up->userid);
       bmw_edit(up, buf, &bmw, 0);
     }
     
     return 0;
    
    }    
  } while (++up <= uceil);

  return 0;
}
#endif

/*¶W¬¯¦^ÅU*/
static void bmw_display(int max,int pos)
{
  int i=9, j, sent;
  BMW bmw,bmw2;
  char buf[128],color[10];

//  bmw_pos = pos;
  move(i, 0);
  clrtobot();
  i++;
  move(i,0);
  prints(" \033[1;36mùï¢w¢w¢w¢w¢w¢wùô\033[43;37m              ¹Ú¤j¶W¬¯¤ô²y¦^ÅU              \033[40;36mùò¢w¢w¢w¢w¢w¢wùñ\033[m");
  
  i++;
  for (max=0;max<8 ;max++)
  {    
    bmw = bmw_lslot[max];
    if (max == pos)
      bmw2 = bmw;
        

    if(max == pos)
      sprintf(color,"1;45");
    else
      sprintf(color,"0");
      
    if (strstr(bmw.msg,"¡¹¼s¼½"))
      sprintf(buf, "   \033[1;45;37m[%-12s]\033[%sm %-58s\033[m", bmw.userid,color,(bmw.msg+8));
    else
      sprintf(buf, "   \033[37;%sm[\033[33m%-12s\033[37m] %-58s\033[m",color, bmw.userid,bmw.msg);
    move(i,0);
    outs(buf);
    i++;
  }
  
  move(i, 0);
  outs(" \033[1;36m¢u¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t\033[m\n");
  sent = 0;
  for (j=0; j<BMW_LOCAL_MAX; j++)
  {
    bmw = bmw_sentlot[j];
    if (!strncmp(bmw.userid, bmw2.userid, IDLEN))
    {
      sent = 1;
      bmw = bmw_sentlot[j];
      sprintf(buf, "  [1;32mTo %-12s\033[m: \033[32m%-57s\033[m", bmw.userid, bmw.msg);
      outs(buf);
      break;
    }
  }
  if (!sent) 
    prints("  \033[32m©|¥¼¶Ç°Tµ¹ %s\033[m",bmw2.userid);
  move(i+2, 0);
  prints(" \033[1;36mùõ¢w¢w¢w¢w¢w¢wùô\033[43;37m ¾Þ§@ (¡ö¡BEnter)Â÷¶} (¡ô¡õ)¿ï¾Ü (¨ä¥L)¦^°T \033[40;36mùò¢w¢w¢w¢w¢w¢wù÷\033[m ");
  
}


void bmw_reply(int replymode)/* 0:¤@¦¸ctrl+r 1:¨â¦¸ctrl+r */
{
  int userno, max, pos, cc, mode;
  UTMP *up, *uhead;
  BMW bmw;
  screenline sl[2],slt[b_lines + 1];
  char buf[128];

  max = bmw_locus - 1;

  save_foot(sl); /* Thor.981222: ·Q²M±¼message */

  if (max < 0)
  {
    vmsg("¥ý«e¨ÃµL¼ö°T©I¥s");
    restore_foot(sl); /* Thor.981222: ·Q²M±¼message */
    return;
  }

  if (cuser.ufo2 & UFO2_REPLY || replymode)
    vs_save(slt);         /* °O¿ý bmd_display ¤§«eªº screen */
        
  mode = bbsmode;               /* lkchu.981201: save current mode */
  utmp_mode(M_BMW_REPLY);

#if 0  /* Thor.981222: ·Q²M±¼message */
  save_foot(sl);
#endif

  if (!(cuser.ufo2 & UFO2_REPLY) && !replymode)
  {
    move(b_lines - 1, 0);
    clrtoeol();
//    outs("\033[34;46m ¼ö°T¦^À³ \033[31;47m (¡ö)\033[30mÂ÷¶} \033[31m(¡ô¡õ¡÷)\033[30mÂsÄý \033[31m(Enter)\033[30m¿ï¾Ü½u¤W¨Ï¥ÎªÌ¦©À³ \033[31m(¨ä¥L)\033[30m¦^À³ \033[m");
    outs("\033[34;46m ¼ö°T¦^À³ \033[31;47m (¡ö Enter)\033[30mÂ÷¶} \033[31m(¡ô¡õ¡÷)\033[30mÂsÄý \033[31m(¨ä¥L)\033[30m¦^À³ \033[m");
  }

  cc = 12345;
  pos = max;

  uhead = ushm->uslot;

  for (;;)
  {
    if (cc == 12345)
    {
      bmw = bmw_lslot[pos];
      if (cuser.ufo2 & UFO2_REPLY || replymode)
        bmw_display(max, pos);
      else 
      {
        if (strstr(bmw.msg,"¡¹¼s¼½"))
          sprintf(buf, BMW_FORMAT_BC, bmw.userid, (bmw.msg)+8);
        else
          sprintf(buf, BMW_FORMAT, bmw.userid, bmw.msg);
        outz(buf);
      }
    }

    cc = vkey();

#if 0
    if (cc == '\n')	/* lkchu.981201: «ö Enter ¿ï¾Ü½u¤W user */
    {
      screenline slp[b_lines + 1];
      
      vs_save(slp);
      bmw_choose();
      memcpy(&slp[b_lines - 1], &sl, sizeof(screenline) * 2);
                         /* lkchu.981201: Åý foot ¤£¥Î­«ÂÐ restore */
      vs_restore(slp);
      if (cuser.ufo2 & UFO2_REPLY || replymode)
        vs_restore(slt);      /* ÁÙ­ì bmw_display ¤§«eªº screen */
      utmp_mode(mode);      /* lkchu.981201: restore bbsmode */
      return;
    }
#else
    if (cc == '\n')
    {
      if (cuser.ufo2 & UFO2_REPLY || replymode)
        vs_restore(slt);      /* ÁÙ­ì bmw_display ¤§«eªº screen */
      else
        restore_foot(sl);
      utmp_mode(mode);
      return;
    }
#endif

    if (cc == Ctrl('R') && !replymode)
    {
      bmw_reply(1);
      break;
    }

    if (cc == KEY_UP)
    {
      if (pos > 0)
      {
	pos--;
	cc = 12345;
      }
      continue;
    }

    if (cc == KEY_DOWN)
    {
      if (pos < max)
      {
	pos++;
	cc = 12345;
      }
      continue;
    }

    if (cc == KEY_RIGHT)
    {
      if (pos != max)
      {
	pos = max;
	cc = 12345;
      }
      continue;
    }

    if (cc == KEY_LEFT)
    {
      if (cuser.ufo2 & UFO2_REPLY || replymode)
        vs_restore(slt);      /* ÁÙ­ì bmw_display ¤§«eªº screen */
      break;
    }

    if (isprint2(cc))
    {
      userno = bmw.sender; /* Thor.980805: ¨¾¤î¨t²Î¨ó´M¦^¦© */
      if(!userno)
      {
        vmsg("¨t²Î°T®§µLªk¦^¦©"); 
//        vmsg("±z¨Ã¤£¬O¹ï¤èªº¦n¤Í¡AµLªk¦^¦©¤W¯¸³qª¾");
				/* lkchu.981201: ¦n¤Í¥i¦^¦© */
                                          
        break;
      }
      if(bmw.caller->ufo & UFO_REJECT)
      {
        vmsg("¹ï¤è¦³¨Æ¡A½Ðµy«Ý¤@·|¨à....");
        break;
      }

      up = bmw.caller;
#if 1    
      if ((up < uhead) || (up > uhead + MAXACTIVE /*ushm->offset*/))
				/* lkchu.981201: comparison of distinct pointer types */
      {
	vmsg(MSG_USR_LEFT);
	break;
      }
#endif
      /* userno = bmw.sender; */ /* Thor.980805: ¨¾¤î¨t²Î¦^¦© */
      if (up->userno != userno)
      {
	up = utmp_find(userno);
	if (!up)
	{
	  vmsg(MSG_USR_LEFT);
	  break;
	}
      }

#ifdef  HAVE_SHOWNUMMSG
      if(up->num_msg > 9 && (up->ufo & UFO_MAXMSG) && !HAS_PERM(PERM_SYSOP))
      {
        vmsg("¹ï¤è¤w¸g³Q¤ô²yÄéÃz¤F!!");
        break;
      }
#endif

#ifdef  HAVE_BANMSG
      if(!(up->ufo & UFO_MESSAGE) && can_banmsg(up))
      {
        vmsg("¹ï¤è¤£·QÅ¥¨ì±zªºÁn­µ!!");
        break;
      }
#endif

      /* bmw_edit(up, "¡¹¦^À³¡G", &bmw, cc); */
      /* Thor.981214: ¬°¨Ï¦^¦©¤£­P§Ë²V */
      sprintf(buf,"¡¹[%s]", up->userid);
      bmw_edit(up, buf, &bmw, cc); 
      break;
    }
  }

  if (cuser.ufo2 & UFO2_REPLY || replymode)
    vs_restore(slt);
  else
    restore_foot(sl);

  utmp_mode(mode);      /* lkchu.981201: restore bbsmode */
}





#if 0
void
bmw_reply()
{
  int userno, max, pos, cc, mode;
  UTMP *up, *uhead;
  BMW bmw;
  screenline sl[2];
  char buf[128];

  max = bmw_locus - 1;

  save_foot(sl); /* Thor.981222: ·Q²M±¼message */

  if (max < 0)
  {
    vmsg("¥ý«e¨ÃµL¼ö°T©I¥s");
    restore_foot(sl); /* Thor.981222: ·Q²M±¼message */
    return;
  }
  
  mode = bbsmode;               /* lkchu.981201: save current mode */
  utmp_mode(M_BMW_REPLY);

#if 0  /* Thor.981222: ·Q²M±¼message */
  save_foot(sl);
#endif

  move(b_lines - 1, 0);
  outs("\033[34;46m ¼ö°T¦^À³ \033[31;47m (¡ö)\033[30mÂ÷¶} \033[31m(¡ô¡õ¡÷)\033[30mÂsÄý \033[31m(Enter)\033[30m¿ï¾Ü½u¤W¨Ï¥ÎªÌ¦©À³ \033[31m(¨ä¥L)\033[30m¦^À³ \033[m");

  cc = 12345;
  pos = max;

  uhead = ushm->uslot;

  for (;;)
  {
    if (cc == 12345)
    {
      bmw = bmw_lslot[pos];
      if (strstr(bmw.msg,"¡¹¼s¼½"))
        sprintf(buf, BMW_FORMAT_BC, bmw.userid, (bmw.msg)+8);
      else
        sprintf(buf, BMW_FORMAT, bmw.userid, bmw.msg);
      outz(buf);
    }

    cc = vkey();

#if 0
    if (cc == '\n')	/* lkchu.981201: «ö Enter ¿ï¾Ü½u¤W user */
    {
      screenline slp[b_lines + 1];
      
      vs_save(slp);
      bmw_choose();
      memcpy(&slp[b_lines - 1], &sl, sizeof(screenline) * 2);
                         /* lkchu.981201: Åý foot ¤£¥Î­«ÂÐ restore */
      vs_restore(slp);
      utmp_mode(mode);      /* lkchu.981201: restore bbsmode */
      return;
    }
#else
    if (cc == '\n')
    {
      restore_foot(sl);
      utmp_mode(mode);
      return;
    }
#endif

    if (cc == KEY_UP)
    {
      if (pos > 0)
      {
	pos--;
	cc = 12345;
      }
      continue;
    }

    if (cc == KEY_DOWN)
    {
      if (pos < max)
      {
	pos++;
	cc = 12345;
      }
      continue;
    }

    if (cc == KEY_RIGHT)
    {
      if (pos != max)
      {
	pos = max;
	cc = 12345;
      }
      continue;
    }

    if (cc == KEY_LEFT)
      break;

    if (isprint2(cc))
    {
      userno = bmw.sender; /* Thor.980805: ¨¾¤î¨t²Î¨ó´M¦^¦© */
      if(!userno)
      {
        vmsg("¨t²Î°T®§µLªk¦^¦©"); 
/*        vmsg("±z¨Ã¤£¬O¹ï¤èªº¦n¤Í¡AµLªk¦^¦©¤W¯¸³qª¾");*/
				/* lkchu.981201: ¦n¤Í¥i¦^¦© */
                                          
        break;
      }
      if(bmw.caller->ufo & UFO_REJECT)
      {
        vmsg("¹ï¤è¦³¨Æ¡A½Ðµy«Ý¤@·|¨à....");
        break;
      }

      up = bmw.caller;
#if 1    
      if ((up < uhead) || (up > uhead + MAXACTIVE /*ushm->offset*/))
				/* lkchu.981201: comparison of distinct pointer types */
      {
	vmsg(MSG_USR_LEFT);
	break;
      }
#endif
      /* userno = bmw.sender; */ /* Thor.980805: ¨¾¤î¨t²Î¦^¦© */
      if (up->userno != userno)
      {
	up = utmp_find(userno);
	if (!up)
	{
	  vmsg(MSG_USR_LEFT);
	  break;
	}
      }

#ifdef  HAVE_SHOWNUMMSG
      if(up->num_msg > 9 && (up->ufo & UFO_MAXMSG) && !HAS_PERM(PERM_SYSOP))
      {
        vmsg("¹ï¤è¤w¸g³Q¤ô²yÄéÃz¤F!!");
        break;
      }
#endif

#ifdef  HAVE_BANMSG
      if(!(up->ufo & UFO_MESSAGE) && can_banmsg(up))
      {
        vmsg("¹ï¤è¤£·QÅ¥¨ì±zªºÁn­µ!!");
        break;
      }
#endif

      /* bmw_edit(up, "¡¹¦^À³¡G", &bmw, cc); */
      /* Thor.981214: ¬°¨Ï¦^¦©¤£­P§Ë²V */
      sprintf(buf,"¡¹[%s]", up->userid);
      bmw_edit(up, buf, &bmw, cc); 
      break;
    }
  }

  restore_foot(sl);
  utmp_mode(mode);      /* lkchu.981201: restore bbsmode */
}
#endif

/* ----------------------------------------------------- */
/* Thor.0607: system assist user login notify		 */
/* ----------------------------------------------------- */


#define MSG_CC "\033[32m[¸s²Õ¦W³æ]\033[m\n"


int
pal_list(reciper)
  int reciper;
{
  LIST list;
  int userno, fd;
  char buf[32], fpath[64],msg[128],temp[30];
  ACCT acct;
  sprintf(msg,"A)¼W¥[ D)§R°£ F)¦n¤Í G)±ø¥ó [1~%d]¸s²Õ¦W³æ M)©w®× Q)¨ú®ø¡H[M]",MAX_LIST);

  userno = 0;

  for (;;)
  {
    switch (vget(1, 0, msg , buf, 2, LCECHO))
    {
    case '1': case '2': case '3': case '4': case '5': case '6': case '7':
    case '8': case '9':
      sprintf(temp,"list.%c",*buf);
      usr_fpath(fpath,cuser.userid,temp);
      fd = open(fpath,O_RDONLY);
      while (fd)
      {
        if(read(fd,&list,sizeof(LIST)) == sizeof(LIST))
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
    case 'a':
      while (acct_get("½Ð¿é¤J¥N¸¹(¥u«ö ENTER µ²§ô·s¼W): ", &acct) > 0)
      {
	if (!ll_has(acct.userid))
	{
	  ll_add(acct.userid);
	  reciper++;
	  ll_out(3, 0, MSG_CC);
	}
      }
      break;

    case 'd':

      while (reciper)
      {
	if (!vget(1, 0, "½Ð¿é¤J¥N¸¹(¥u«ö ENTER µ²§ô§R°£): ",
	    buf, IDLEN + 1, GET_LIST))
	  break;
	if (ll_del(buf))
	  reciper--;
	ll_out(3, 0, MSG_CC);
      }
      break;
#if 1
    case 'g':
      if ((userno = vget(b_lines, 0, "¸s²Õ±ø¥ó¡G", buf, 16, DOECHO)))
	str_lower(buf, buf);
#endif
    case 'f':
      usr_fpath(fpath, cuser.userid, FN_PAL);
      if ((fd = open(fpath, O_RDONLY)) >= 0)
      {
	PAL *pal;
	char *userid;

	mgets(-1);
	while ((pal = mread(fd, sizeof(PAL))))
	{
	  userid = pal->userid;
	  if (!ll_has(userid) && (pal->userno != cuser.userno) &&
	    !(pal->ftype & PAL_BAD) &&
	    (!userno || str_str(pal->ship, buf)))
	  {
	    ll_add(userid);
	    reciper++;
	  }
	}
	close(fd);
      }
      ll_out(3, 0, MSG_CC);
      userno = 0;
      break;

    case 'q':
      return 0;

    default:
      return reciper;
    }
  }
}


#ifdef HAVE_ALOHA
void
aloha()
{
  UTMP *up, *ubase, *uceil;
  int fd;
  char fpath[64];
  BMW *bmw, benz;
  int userno;
  struct stat st;
  
  /* lkchu.981201: ¦n¤Í³qª¾ */

  usr_fpath(fpath, cuser.userid, FN_FRIEND_BENZ);
  if (((fd = open(fpath, O_RDONLY)) >= 0) && !fstat(fd, &st))
  {
    if(st.st_size <= 0)
    {
      close(fd);
      return;
    }
    benz.caller = cutmp;
    /* benz.sender = cuser.userno; */
    benz.sender = 0; /* Thor.980805: ¨t²Î¨ó´M¬°³æ¦V call in */
    strcpy(benz.userid, cuser.userid);
    sprintf(benz.msg, "¡· ¶i ¤J (%s) Åo!! ¡·", BOARDNAME);

    ubase = ushm->uslot;
    uceil = (void *) ubase + ushm->offset;

    mgets(-1);
    while ((bmw = mread(fd, sizeof(BMW))))
    {
      /* Thor.1030:¥u³qª¾ªB¤Í */

      userno = bmw->recver;
      /* up = bmw->caller; */
      up = utmp_find(userno);	/* lkchu.981201: frienz ¤¤ªº utmp µL®Ä */
      
      if (up >= ubase && up <= uceil &&
	up->userno == userno && !(cuser.ufo & UFO_CLOAK) && can_message(up))
                                 /* Thor.980804: Áô¨­¤W¯¸¤£³qª¾, ¯¸ªø¯SÅv */
                                 /* lkchu.981201: ¹ï¤è­n¥´¶}¡u¦n¤Í¤W¯¸³qª¾¡v¤~³qª¾ */
      {
        /*if (is_pal(userno))*/           /* lkchu.980806: ¦n¤Í¤~¥i¥H reply */
        /*  benz.sender = cuser.userno;
        else*/
        if(!is_bad(userno) || (up->userlevel & PERM_SYSOP))
        {
          benz.sender = 0;

  	  benz.recver = userno;
	  bmw_send(up, &benz);
	}
      }
    }
    close(fd);
  }
}
#endif


#ifdef LOGIN_NOTIFY
extern LinkList *ll_head;


int
t_loginNotify()
{
  LinkList *wp;
  BMW bmw;
  char fpath[64];

  /* ³]©w list ªº¦W³æ */

  vs_bar("¨t²Î¨ó´Mºô¤Í");

  ll_new();

  if (pal_list(0))
  {
    wp = ll_head;
    bmw.caller = cutmp;
    bmw.recver = cuser.userno;
    strcpy(bmw.userid, cuser.userid);

    /* Thor.980629: ¤pbug, ¥i¥H¨ó´M¦Û¤v, ¶i¦Ócall-in¦Û¤v:P */

    do
    {
      usr_fpath(fpath, wp->data, FN_BENZ);
      rec_add(fpath, &bmw, sizeof(BMW));
    } while ((wp = wp->next));

    vmsg("¨ó´M³]©w§¹¦¨, ªB¤Í¤W¯¸®É" SYSOPNICK "·|³qª¾±z,½Ð¤Å­«ÂÐ³]©w!");

    /* Thor.1030 : ­Y¤£¬OªB¤Í´N¤£³qª¾°Õ...... ¥H§Kcallin ¤£¹ïºÙ */
  }
  return 0;
}


void
loginNotify()
{
  UTMP *up, *ubase, *uceil;
  int fd;
  char fpath[64];
  BMW *bmw, benz;
  int userno;

  usr_fpath(fpath, cuser.userid, FN_BENZ);

  if ((fd = open(fpath, O_RDONLY)) >= 0)
  {
    vs_bar("¨t²Î¨ó´Mºô¤Í");

#if 0
    move(10, 0);
    clrtobot();
    outs("¡¹ " SYSOPNICK "¡G¡y´¿¸g¦³¤H·Q§ä±z®@¡K¡K\n");
#endif

    benz.caller = cutmp;
    /* benz.sender = cuser.userno; */
    benz.sender = 0; /* Thor.980805: ¨t²Î¨ó´M¬°³æ¦V call in */
    strcpy(benz.userid, cuser.userid);
    sprintf(benz.msg, "¡· ­è­è½ñ¶i%sªºªù [¨t²Î¨ó´M] ¡·", BOARDNAME);

    ubase = ushm->uslot;
    uceil = (void *) ubase + ushm->offset;

    mgets(-1);
    while ((bmw = mread(fd, sizeof(BMW))))
    {
      /* Thor.1030:¥u³qª¾ªB¤Í */

      up = bmw->caller;
      userno = bmw->recver;

      if (up >= ubase && up <= uceil &&
	up->userno == userno && !(cuser.ufo & UFO_CLOAK) && userno != cuser.userno  && can_see(up) !=2)
                                 /* Thor.980804: Áô¨­¤W¯¸¤£³qª¾, ¯¸ªø¯SÅv */
#if 0  
        /* Thor.980805: ¤£·|¦^¦©¤F, ³æ¦V call-in */
        && is_pal(userno) && 
        userno != cuser.userno && !(cuser.ufo & UFO_QUIET))
        /* Thor.980707: ¤£´M¦Û¤v */ /* Thor.980804: »·Â÷¹ÐÄÛ®É¤£³qª¾,¥H¨¾¦^¦© */
#endif
      {
/*        benz.sender = is_pal(userno) ? cuser.userno : 0; */
                      /* lkchu.980806: ¦n¤Í¤~¥i¥H reply */
        if(!is_bad(userno) || (up->userlevel & PERM_SYSOP))
        {              
	  benz.sender = 0;
	  benz.recver = userno;
	  bmw_send(up, &benz);

          prints("*");  /* Thor.980707: ¦³³qª¾¨ìªº¦³©Ò¤£¦P */
        }
      }

      prints("%-13s", bmw->userid);

    }
    close(fd);
    unlink(fpath);
    vmsg(NULL);
  }
}
#endif


/* Thor: for ask last call-in messages */

/* lkchu: ¼ö°T¦^ÅU·s¤¶­± */
int
t_recall()
{
  xover(XZ_BMW);
  return 0;
}


#ifdef LOG_TALK
void
talk_save()
{
  char fpath[64];
  struct stat st;
  
  usr_fpath(fpath, cuser.userid, FN_TALK_LOG);
  stat(fpath, &st);

  if (!(cuser.ufo2 & UFO2_NTLOG) && st.st_size > 0)
  {
    char buf[64];
    HDR fhdr;
    
    usr_fpath(buf, cuser.userid, fn_dir);
    hdr_stamp(buf, HDR_LINK, &fhdr, fpath);
    strcpy(fhdr.title, "[³Æ §Ñ ¿ý] ²á¤Ñ¬ö¿ý");
    strcpy(fhdr.owner, cuser.userid);
    fhdr.xmode = MAIL_READ | MAIL_NOREPLY;
    rec_add(buf, &fhdr, sizeof(fhdr)); 

  } 
  unlink(fpath);
  return;


#if 0
  /* lkchu.981201: ©ñ¶i¨p¤H«H½c¤º/²M°£/«O¯d */
  if (dashf(fpath))
  {
    
    switch (vans("¥»¦¸²á¤Ñ¬ö¿ý³B²z (M)³Æ§Ñ¿ý (C)²M°£¡H[M] "))
    {
    case 'c':
      unlink(fpath);
      break;
      
    default:
      {
        char buf[64];
        HDR fhdr;
        
        usr_fpath(buf, cuser.userid, fn_dir);
        hdr_stamp(buf, HDR_LINK, &fhdr, fpath);
        strcpy(fhdr.title, "[³Æ §Ñ ¿ý] ²á¤Ñ¬ö¿ý");
        strcpy(fhdr.owner, cuser.userid);
        fhdr.xmode = MAIL_READ | MAIL_NOREPLY;
        rec_add(buf, &fhdr, sizeof(fhdr)); 

        unlink(fpath);
      }
      break;
    }      
  }
#endif

}
#endif


#ifdef LOG_BMW
void
bmw_save()
{
  int fd,check_max;
  char ans;
  char fpath[64],buf[128];
  struct stat st;
    
  
  usr_fpath(fpath, cuser.userid, FN_BMW);
  fd = f_open(fpath);	/* lkchu.990428: ­Y size ¬° 0 ·|³Q unlink ±¼ */
  if (fd >= 0)
  {
    fstat(fd, &st);
    check_max = st.st_size;
  }
  else
    check_max = 0;


  /* lkchu.981201: ©ñ¶i¨p¤H«H½c¤º/²M°£/«O¯d */
  if (fd >= 0 && check_max > sizeof(BMW))
  {
    if(check_max >= BMW_MAX_SIZE * 1024)
      ans = 'm';
    else if(cuser.ufo2 & UFO2_NWLOG)
      ans = 'r';
    else
    {
      sprintf(buf,"¥»¦¸¤W¯¸¼ö°T³B²z (M)³Æ§Ñ¿ý (R)«O¯d (C)²M°£ [%dk/%dk] ¡H[R] ",check_max/1024,BMW_MAX_SIZE);
      ans = vans(buf);
    }
    switch (ans)
    {
    case 'c':
      close(fd);
      unlink(fpath);
      break;

    case 'r':
      close(fd);
      break;
      
    case 'm':
      {
        FILE *fout;
        char buf[80], folder[80];
        HDR fhdr;
        
        usr_fpath(folder, cuser.userid, fn_dir);
        if ((fout = fdopen(hdr_stamp(folder, 0, &fhdr, buf), "w")))
        {        
          BMW bmw;
          
          while (read(fd, &bmw, sizeof(BMW)) == sizeof(BMW)) 
          {
            struct tm *ptime = localtime(&bmw.btime);
            
            fprintf(fout, "%s%s(%02d:%02d)¡G%s\033[m\n", 
              bmw.sender == cuser.userno ? "¡¸" : "\033[32m¡¹",
              bmw.userid, ptime->tm_hour, ptime->tm_min, bmw.msg);
          }
          fclose(fout);
        }
        close(fd);
        
        fhdr.xmode = MAIL_READ | MAIL_NOREPLY;
	strcpy(fhdr.title, "[³Æ §Ñ ¿ý] ¼ö°T¬ö¿ý");
	strcpy(fhdr.owner, cuser.userid);
        rec_add(folder, &fhdr, sizeof(fhdr));

        unlink(fpath);
      }
      break;
      
    default:
      close(fd);
      break;
    }      
  }

}
#endif


void
bmw_rqst()
{
  int i, j, userno, locus;
  BMW bmw[BMW_PER_USER], *mptr, **mslot;

  /* download BMW slot first */

  i = j = 0;
  userno = cuser.userno;
  mslot = cutmp->mslot;

  while ((mptr = mslot[i]))
  {
    mslot[i] = NULL;
    if (mptr->recver == userno)
    {
      bmw[j++] = *mptr;
    }
    mptr->btime = 0;

    if (++i >= BMW_PER_USER)
      break;
  }

  /* process the request */

  if (j)
  {
    char buf[128];

    locus = bmw_locus;
    i = locus + j - BMW_LOCAL_MAX;
    if (i >= 0)
    {
      locus -= i;
      memcpy(bmw_lslot, bmw_lslot + i, locus * sizeof(BMW));
    }

    i = 0;
    do
    {
      mptr = &bmw[i];

      /* lkchu.981230: §Q¥Î xover ¾ã¦X bmw */
      usr_fpath(buf, cuser.userid, FN_BMW);
      rec_add(buf, &bmw[i], sizeof(BMW)); 
                  
      bmw_lslot[locus++] = *mptr;
    } while (++i < j);

    bmw_locus = locus;

    if(!(cutmp->ufo & (UFO_REJECT | UFO_NET)))
    {
      if (strstr(mptr->msg,"¡¹¼s¼½"))
         sprintf(buf, BMW_FORMAT_BC, mptr->userid, (mptr->msg)+8);
      else
         sprintf(buf, BMW_FORMAT, mptr->userid, mptr->msg);

      /* Thor.980827: ¬°¤F¨¾¤î¦C¦L¤@¥b(more)®É¼ö°T¦Ó«á¦C¦L¶W¹L½d³ò½ð¤H, 
                      ¬G¦s¤U´å¼Ð¦ì¸m */
      cursor_save(); 

      outz(buf);
      /* Thor.980827: ¬°¤F¨¾¤î¦C¦L¤@¥b(more)®É¼ö°T¦Ó«á¦C¦L¶W¹L½d³ò½ð¤H, 
                      ¬GÁÙ­ì´å¼Ð¦ì¸m */
      cursor_restore();

      refresh();
      bell();
#ifdef	HAVE_SHOWNUMMSG      
      cutmp->num_msg++;
#endif      
    }
  }
}


/* ----------------------------------------------------- */
/* talk sub-routines					 */
/* ----------------------------------------------------- */


static void
talk_nextline(twin)
  talk_win *twin;
{
  int curln;

  curln = twin->curln + 1;
  if (curln > twin->eline)
    curln = twin->sline;
  if (curln != twin->eline)
  {
    move(curln + 1, 0);
    clrtoeol();
  }
  move(curln, 0);
  clrtoeol();
  twin->curcol = 0;
  twin->curln = curln;
}


static void
talk_char(twin, ch)
  talk_win *twin;
  int ch;
{
  int col, ln;

  col = twin->curcol;
  ln = twin->curln;

  if (isprint2(ch))
  {
    if (col < 79)
    {
      move(ln, col);
      twin->curcol = ++col;
    }
    else
    {
      talk_nextline(twin);
      twin->curcol = 1;
    }
    outc(ch);
  }
  else if (ch == '\n')
  {
    talk_nextline(twin);
  }
  else if (ch == Ctrl('H'))
  {
    if (col)
    {
      twin->curcol = --col;
      move(ln, col);
      outc(' ');
      move(ln, col);
    }
  }
  else if (ch == Ctrl('G'))
  {
    bell();
  }
}


static void
talk_string(twin, str)
  talk_win *twin;
  uschar *str;
{
  int ch;

  while ((ch = *str))
  {
    talk_char(twin, ch);
    str++;
  }
}


static void
talk_speak(fd)
  int fd;
{
  talk_win mywin, itswin;
  uschar data[80];
  char buf[80];
  int i, ch;
#ifdef  LOG_TALK
  char mywords[80], itswords[80], itsuserid[40];
  FILE *fp;
    
  /* lkchu: make sure that's empty */
  mywords[0] = itswords[0] = '\0';
  
  strcpy(itsuserid, page_requestor);
  strtok(itsuserid, " (");
#endif

  utmp_mode(M_TALK);

  ch = 58 - strlen(page_requestor);

  sprintf(buf, "%s¡i%s", cuser.userid, cuser.username);

  i = ch - strlen(buf);
  if (i >= 0)
  {
    i = (i >> 1) + 1;
  }
  else
  {
    buf[ch] = '\0';
    i = 1;
  }
  memset(data, ' ', i);
  data[i] = '\0';

  memset(&mywin, 0, sizeof(mywin));
  memset(&itswin, 0, sizeof(itswin));

  i = b_lines >> 1;
  mywin.eline = i - 1;
  itswin.curln = itswin.sline = i + 1;
  itswin.eline = b_lines - 1;

  clear();
  move(i, 0);
  prints("\033[1;46;37m  ½Í¤Ñ»¡¦a  \033[45m%s%s¡j ¡»  %s%s\033[m",
    data, buf, page_requestor, data);
#if 1
  outz("\033[34;46m ¥æ½Í¼Ò¦¡ \033[31;47m (^A)\033[30m¹ï«³¼Ò¦¡ \033[31m(^B)\033[30m¶H´Ñ¼Ò¦¡ \033[31m(^C,^D)\033[30mµ²§ô¥æ½Í \033[31m(^Z)\033[30m§Ö±¶¦Cªí \033[31m(^G)\033[30m¹Í¹Í \033[m");
#endif
  move(0, 0);

#ifdef LOG_TALK                            /* lkchu.981201: ²á¤Ñ°O¿ý */
  usr_fpath(buf, cuser.userid, FN_TALK_LOG);
  if ((fp = fopen(buf, "a+")))
    fprintf(fp, "¡i %s »P %s ¤§²á¤Ñ°O¿ý ¡j\n", cuser.userid, page_requestor);
#endif

  add_io(fd, 60);

  for (;;)
  {
    ch = igetch();

    if (ch == KEY_ESC)
    {
      igetch();
      igetch();
      continue;
    }

#ifdef EVERY_Z
    /* Thor.0725: talk¤¤, ctrl-z */
    if (ch == Ctrl('Z'))
    {
      screenline sl[b_lines + 1];
      char buf[IDLEN + 1];
      /* Thor.0731: ¼È¦s mateid, ¦]¬°¦³¥i¯àquery§O¤H®É·|¥Î±¼mateid */
      strcpy(buf, cutmp->mateid);

      /* Thor.0727: ¼È¦s vio_fd */
      holdon_fd = vio_fd;
      vio_fd = 0;
      vs_save(sl);
      every_Z();
      vs_restore(sl);
      /* Thor.0727: ÁÙ­ì vio_fd */
      vio_fd = holdon_fd;
      holdon_fd = 0;

      /* Thor.0731: ÁÙ­ì mateid, ¦]¬°¦³¥i¯àquery§O¤H®É·|¥Î±¼mateid */
      strcpy(cutmp->mateid, buf);
      continue;
    }
#endif
    if (ch == Ctrl('U'))
    {
      char buf[IDLEN + 1];
      strcpy(buf, cutmp->mateid);

      holdon_fd = vio_fd;
      vio_fd = 0;
      every_U();
      vio_fd = holdon_fd;
      holdon_fd = 0;

      strcpy(cutmp->mateid, buf);
      continue;
    }
    if (ch == Ctrl('D') || ch == Ctrl('C'))
      break;

    if (ch == I_OTHERDATA)
    {
      ch = recv(fd, data, 80, 0);
      if (ch <= 0)
	break;
#if 1
      if (data[0] == Ctrl('A'))
      { /* Thor.990219: ©I¥s¥~±¾´Ñ½L */
        if(DL_func("bin/bwboard.so:vaBWboard",fd,1)==-2)
          break;
        continue;
      }
      if (data[0] == Ctrl('B'))
      {
        if(DL_func("bin/chess.so:vaChess",fd,1)==-2)
          break;
        continue;
      }
#endif
      for (i = 0; i < ch; i++)
      {
	talk_char(&itswin, data[i]);
#ifdef	LOG_TALK
        switch (data[i])
        {
        case '\n':
	  /* lkchu.981201: ¦³´«¦æ´N§â itswords ¦L¥X²M±¼ */
	  if (itswords[0] != '\0')
  	  {
  	    fprintf(fp, "\033[32m%s¡G%s\033[m\n", itsuserid, itswords);
	    itswords[0] = '\0';
	  }
	  break;

        case Ctrl('H'):	/* lkchu.981201: backspace */
          itswords[str_len(itswords) - 1] = '\0';
          break;
          
        default:
	  if (str_len(itswords) < sizeof(itswords))
  	  {
  	    strncat(itswords, (char *)&data[i], 1);
	  }
	  else	/* lkchu.981201: itswords ¸Ëº¡¤F */
	  {
  	    fprintf(fp, "\033[32m%s¡G%s%c\033[m\n", itsuserid, itswords, data[i]);
	    itswords[0] = '\0';
	  }
	  break;
	}
#endif

      }
    }
#if 1
    else if (ch == Ctrl('A'))
    { /* Thor.990219: ©I¥s¥~±¾´Ñ½L */
      /* extern int BWboard(); */
      data[0] = ch;
      if (send(fd, data, 1, 0) != 1)
	break;
      /* if (BWboard(fd,0)==-2) */
      if(DL_func("bin/bwboard.so:vaBWboard",fd,0)==-2)
        break;
    }
    else if (ch == Ctrl('B'))
    { /* Thor.990219: ©I¥s¥~±¾´Ñ½L */
      /* extern int BWboard(); */
      data[0] = ch;
      if (send(fd, data, 1, 0) != 1)
        break;
      /* if (BWboard(fd,0)==-2) */
      if(DL_func("bin/chess.so:vaChess",fd,0)==-2)
        break;
    }
#endif
    else if (isprint2(ch) || ch == '\n' || ch == Ctrl('H') || ch == Ctrl('G'))
    {
      data[0] = ch;
      if (send(fd, data, 1, 0) != 1)
	break;

#ifdef LOG_TALK				/* lkchu: ¦Û¤v»¡ªº¸Ü */
      switch (ch)
      {
      case '\n':
        if (mywords[0] != '\0')
        {
          fprintf(fp, "%s¡G%s\n", cuser.userid, mywords);
  	  mywords[0] = '\0';
        }
        break;
      
      case Ctrl('H'):
        mywords[str_len(mywords) - 1] = '\0';
        break;
      
      default:
        if (str_len(mywords) < sizeof(mywords))
	{
          strncat(mywords, (char *)&ch, 1);
        }
        else
        {
          fprintf(fp, "%s¡G%s%c\n", cuser.userid, mywords, ch);
	  mywords[0] = '\0';
	}
        break;
      }
#endif

      talk_char(&mywin, ch);

#ifdef EVERY_BIFF 
      /* Thor.980805: ¦³¤H¦b®ÇÃä«öenter¤~»Ý­ncheck biff */ 
      if(ch=='\n')
      {
        static int old_biff,old_biffn; 
        int biff = cutmp->ufo & UFO_BIFF; 
        if (biff && !old_biff) 
          talk_string(&mywin, "¡» ¾´! ¶l®t½Ä¶i¨Ó¤F!\n"); 
        old_biff = biff;
        biff = cutmp->ufo & UFO_BIFFN;                                       
        if (biff && !old_biffn)
          talk_string(&mywin, "¡» ¾´! ±z¦³¯«¯µ¯d¨¥!\n");
        old_biffn = biff; 
      }
#endif
    }

  }

#ifdef LOG_TALK
  fclose(fp);
#endif

  add_io(0, 60);
}


#if 0
static int
xsocket()
{
  int sock, val;

  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock >= 0)
  {
    val = 64;
    setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &val, sizeof(val));
    setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &val, sizeof(val));
  }
  return sock;
}
#endif


static void
talk_hangup(sock)
  int sock;
{
  cutmp->sockport = 0;
  add_io(0, 60);
  close(sock);
}


static char *talk_reason[] =
{
  "¹ï¤£°_¡A§Ú¦³¨Æ±¡¤£¯à¸ò§A talk",
  "§Ú²{¦b«Ü¦£¡A½Ðµ¥¤@·|¨à¦A call §Ú",
  "²{¦b¦£¤£¹L¨Ó¡Aµ¥¤@¤U§Ú·|¥D°Ê page §A",
  "§Ú²{¦b¤£·Q talk °Õ ...",
  "§A¯uªº«Ü·Ð¡A§Ú¹ê¦b¤£·Q¸ò§A talk"

#ifdef EVERY_Z
  ,"§Úªº¼L¤Ú¥¿¦£µÛ©M§O¤HÁ¿¸Ü©O¡A¨S¦³ªÅªº¼L¤Ú¤F"
  /* Thor.0725: for chat&talk ¥Î^z §@·Ç³Æ */
#endif
};


/* return 0: ¨S¦³ talk, 1: ¦³ talk, -1: ¨ä¥L */


static int
talk_page(up)
  UTMP *up;
{
  int sock, msgsock;
  struct sockaddr_in sin;
  pid_t pid;
  int ans, length,myans;
  char buf[60];
#if     defined(__OpenBSD__)
  struct hostent *h;
#endif

#if 0
  /* Thor.0523: ´ú¸Õ 500¤H¥H¤W, ¸T¤î¥¼»{ÃÒuser talk */
  if (ushm->number > 500 && !HAS_PERM(PERM_VALID))
  {
    vmsg("¥Ø«e½u¤W¤H¼Æ¤Ó¦h¡A±z¥¼³q¹L»{ÃÒ¡AµLªk¨Ï¥Î¦¹¥\\¯à");
    return 0;
  }
#endif

#ifdef EVERY_Z
  /* Thor.0725: ¬° talk & chat ¥i¥Î ^z §@·Ç³Æ */
  if (holdon_fd)
  {
    vmsg("±zÁ¿¸ÜÁ¿¤@¥bÁÙ¨SÁ¿§¹­C");
    return 0;
  }
#endif

  pid = up->mode;

  if (pid >= M_BBTP && pid <= M_CHAT)
  {
    vmsg("¹ï¤èµL·v²á¤Ñ");
    return 0;
  }

  if (!(pid = up->pid) || kill(pid, 0))
  {
    vmsg(MSG_USR_LEFT);
    return 0;
  }

  /* showplans(up->userid); */
#ifdef  HAVE_PIP_FIGHT
  myans = vans("­n©M¥L/¦o½Í¤Ñ(Y)©Î¹ï¾Ô¤pÂû(C)¶Ü (Y/N/C)?[N] ");
#else
  myans = vans("½T©w­n©M¥L/¦o½Í¤Ñ¶Ü (Y/N)?[N] ");
#endif

#ifdef  HAVE_PIP_FIGHT
  if (myans != 'y' && myans != 'c')
#else
  if (myans != 'y')
#endif
    return 0;

#ifdef  HAVE_PIP_FIGHT
  if (myans == 'c')
  {
    usr_fpath(buf,up->userid,"chicken");
    if(access(buf,0))
    {
      vmsg("¹ï¤è¨S¦³¾i¤pÂû¡I");
      return 1;
    }
  }
#endif

  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0)
    return 0;

#if     defined(__OpenBSD__)                    /* lkchu */

  if (!(h = gethostbyname(MYHOSTNAME)))
    return -1;  
  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_port = 0;
  memcpy(&sin.sin_addr, h->h_addr, h->h_length);                

#else

  sin.sin_family = AF_INET;
  sin.sin_port = 0;
  sin.sin_addr.s_addr = INADDR_ANY;
  memset(sin.sin_zero, 0, sizeof(sin.sin_zero));

#endif

  length = sizeof(sin);
  if (bind(sock, (struct sockaddr *) &sin, length) < 0 || getsockname(sock, (struct sockaddr *) &sin, &length) < 0)
  {
    close(sock);
    return 0;
  }

  cutmp->sockport = sin.sin_port;
  strcpy(cutmp->mateid, up->userid);
  up->talker = cutmp;
#ifdef  HAVE_PIP_FIGHT
  if(myans == 'c')
    utmp_mode(M_CHICKEN);
  else
#endif
    utmp_mode(M_PAGE);
  kill(pid, SIGUSR1);

  clear();
  prints("­º«×©I¥s %s ...\n¥i«ö Ctrl-D ¤¤¤î", up->userid);

  listen(sock, 1);
  add_io(sock, 20);
  do
  {
    msgsock = igetch();

    if (msgsock == Ctrl('D'))
    {
      talk_hangup(sock);
      return -1;
    }

    if (msgsock == I_TIMEOUT)
    {
      move(0, 0);
      outs("¦A");
      bell();

      if (kill(pid, SIGUSR1)) 
      /* Thor.990201:µù¸Ñ:¨ä¹ê³oªºkill,¤]¥u¬O¬Ý¬Ý¹ï¤è¬O¤£¬OÁÙ¦b½u¤W¦Ó¤w:p
                          ­«µosignal¨ä¹ê¦ü¥G talk_rqst¤£·|¦A³Q¥s */
      {
	talk_hangup(sock);
	vmsg(MSG_USR_LEFT);
	return -1;
      }
    }
  } while (msgsock != I_OTHERDATA);

  msgsock = accept(sock, NULL, NULL);
  talk_hangup(sock);
  if (msgsock == -1)
    return -1;

  length = read(msgsock, buf, sizeof(buf));
  ans = buf[0];
  if (ans == 'y')
  {
    sprintf(page_requestor, "%s (%s)", up->userid, up->username);

    /*
     * Thor.0814: ª`·N! ¦b¦¹¦³¤@­ÓÂû¦PÀnÁ¿ªº¥i¯à±¡ªp, ¦pªG askia ¥ýpage
     * starlight, ¦ý¦b starlight ¦^À³«e«o°¨¤WÂ÷¶}, ´« page backspace,
     * backspace©|¥¼¦^À³«e, ¦pªG starlight ¦^À³¤F, starlight ´N·|³Q accept,
     * ¦Ó¤£¬O backspace.
     * 
     * ¦¹®É¦b¿Ã¹õ¤¤¥¡, ¬Ý¨ìªº page_requestor·|¬O backspace, ¥i¬O¨Æ¹ê¤W,
     * talkªº¹ï¶H¬O starlight, ³y¦¨Âû¦PÀnÁ¿!
     * 
     * ¼È®É¤£¤©­×¥¿, ¥H§@¬°ªá¤ßªÌªºÃg»@:P
     */

    talk_speak(msgsock);
  }
#ifdef  HAVE_PIP_FIGHT
  else if(ans == 'c')
  {
    if(!p)
      p = DL_get("bin/pip.so:pip_vf_fight");
    if(p)
	{
	  up->pip = NULL;
      (*p)(msgsock,2);
	  cutmp->pip = NULL;
	}
    add_io(0,60);
/*    pip_vf_fight(msgsock,1);*/
  }
  else if(ans == 'C')
  {
    outs("¡i¦^­µ¡j¹ï¤è¤£·Q PK ¤pÂû¡I");
  }
#endif
  else
  {
    char *reply;

    if (ans == ' ')
    {
      reply = buf;
      reply[length] = '\0';
    }
    else
      reply = talk_reason[ans - '1'];

    move(4, 0);
    outs("¡i¦^­µ¡j");
    outs(reply);
  }

  close(msgsock);
  cutmp->talker = NULL;
#ifdef  LOG_TALK
  if (ans == 'y' && cutmp->mode != M_CHICKEN)    /* mat.991011: ¨¾¤îTalk³Q©Úµ´®É¡A²£¥Í²á¤Ñ°O¿ýªºrecord */
    talk_save();          /* lkchu.981201: talk °O¿ý³B²z */
#endif
  vmsg("²á¤Ñµ²§ô");
  return 1;
}


/*-------------------------------------------------------*/
/* ¿ï³æ¦¡²á¤Ñ¤¶­±					 */
/*-------------------------------------------------------*/


static PICKUP ulist_pool[MAXACTIVE];
#ifdef	FRIEND_FIRST
static int friend_num,ofriend_num,pfriend_num,bfriend_num;/* visor.991103: °O¿ý¥Ø«e¯¸¤W¦n¤Í¼Æ */
#endif
static int ulist_head();
static int ulist_init();
static XO ulist_xo;


static char *msg_pickup_way[PICKUP_WAYS] =
{
  "¥ô·N",
  "¥N¸¹",
  "¬G¶m",
  "°ÊºA",
  "¼ÊºÙ",
  "¶¢¸m"
#ifdef	HAVE_BOARD_PAL  
  ,"ªO¤Í"
#endif
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
  PICKUP *pp;
  UTMP *up;
  int paltmp;
  int n, cnt, max, ufo, self, userno, sysop, diff, diffmsg, fcolor,colortmp;
  char buf[8],color[20],ship[80],*wcolor[7] = {"\033[m",COLOR_PAL,COLOR_BAD,COLOR_BOTH,COLOR_OPAL,COLOR_CLOAK,COLOR_BOARDPAL};

#ifdef HAVE_BOARD_PAL
  int isbpal;
  
  isbpal = (cutmp->board_pal != -1);
#endif
   
  max = xo->max;
  if (max <= 0)
  {
    return XO_QUIT;
  }

  cnt = xo->top;
  pp = &ulist_pool[cnt];
  self = cuser.userno;
  sysop = HAS_PERM(PERM_SYSOP | PERM_ACCOUNTS);

  n = 2;
  while (++n < b_lines)
  {
    move(n, 0);
    if (cnt++ < max)
    {
      up = pp->utmp;

#if 0      
      if(supervisor)
      {
        sprintf(ship,"%s,%d,%d",up->userid,pp->type,up->userno);
        vmsg(ship);
      }
#endif
	  
      if ((userno = up->userno) && (up->userid[0]) && !((up->ufo & UFO_CLOAK) && !HAS_PERM(PERM_SEECLOAK) && (up->userno != cuser.userno)) )
      {
	if ((diff = up->idle_time))
	  sprintf(buf, "%2d", diff);
	else
	  buf[0] = '\0';
	
	paltmp = (pp->type == 1 || pp->type == 2);
	fcolor = (userno == self) ? 3 : paltmp;

#ifdef	HAVE_BOARD_PAL	
	if(isbpal && is_boardpal(up) && userno != self) 
	  fcolor = 6;
#else
	if(userno != self) 
	  fcolor = 6;
#endif

        colortmp = (pp->type == 1 || pp->type == 3);

	if(paltmp && colortmp == 1) 
	  fcolor = 3;
        else if (!paltmp && colortmp == 1) 
          fcolor = 4;
	
        if (is_bad(userno)) 
          fcolor = 2;

        if(cuser.ufo2 & UFO2_SHIP)
        {
          strcpy(ship," ");
          copyship(ship,userno);
        }
        
	ufo = up->ufo;


        diff = ck_state(UFO_PAGER,UFO_PAGER1,up,1);
        diffmsg = ck_state(UFO_QUIET,UFO_MESSAGE,up,0);

        colortmp = 1;

        if(ufo & UFO_CLOAK) 
          fcolor = 5;
        else if(fcolor == 0)
          colortmp = 0;  

        strcpy(color,wcolor[fcolor]);

	prints("%5d%c%s%-13s%-22.21s%s%-16.15s%c%c %-16.16s%s",
	  cnt,(up->ufo & UFO_WEB)?'*':' ',
	  color, up->userid,
	  (HAS_PERM(PERM_SYSOP) && (cuser.ufo2 & UFO2_REALNAME))? up->realname : up->username , 
	  colortmp > 0 ? "\033[m" : "",
	  (cuser.ufo2 & UFO2_SHIP) ? ship : ((up->ufo & UFO_HIDEDN)&&!HAS_PERM(PERM_SYSOP)) ? 
	  HIDEDN_SRC : up->from , diff,diffmsg,
	  bmode(up, 0), buf);
      }
      else
      {
		outs("      < ¦¹¦ìºô¤Í¥¿¥©Â÷¶} >");
      }
      pp++;
    }
    clrtoeol();
  }

  return XO_NONE;
}


static int
ulist_cmp_userid(i, j)
  PICKUP *i, *j;
{
  if(i->type == j->type)
	return str_cmp(i->utmp->userid, j->utmp->userid);
  else
    return i->type - j->type;
}

static int
ulist_cmp_host(i, j)
  PICKUP *i, *j;
{
  return str_cmp(i->utmp->from, j->utmp->from);
}

static int
ulist_cmp_idle(i, j)
  PICKUP *i, *j;
{
  return i->utmp->idle_time - j->utmp->idle_time;
}

static int
ulist_cmp_mode(i, j)
  PICKUP *i, *j;
{
  return i->utmp->mode - j->utmp->mode;
}

static int
ulist_cmp_nick(i, j)
  PICKUP *i, *j;
{
  return str_cmp(i->utmp->username, j->utmp->username);
}

#ifdef	HAVE_BOARD_PAL
static int
ulist_cmp_board(i,j)
  PICKUP *i, *j;
{
  return i->utmp->board_pal - j->utmp->board_pal;
}
#endif

static int (*ulist_cmp[]) () =
{
  ulist_cmp_userid,
  ulist_cmp_host,
  ulist_cmp_mode,
  ulist_cmp_nick,
  ulist_cmp_idle
#ifdef	HAVE_BOARD_PAL  
  ,ulist_cmp_board
#endif
};


static int
ulist_init(xo)
  XO *xo;
{
  UTMP *up, *uceil;
  PICKUP *pp;
  int max, filter, seecloak, userno, self, nf_num,tmp;
  int ispal,bad;
#ifdef HAVE_BOARD_PAL
  int isbpal;
#endif

  pp = ulist_pool;

  self = cuser.userno;
  filter = cuser.ufo2 & UFO2_PAL;
  
  if(cutmp->pid <= 0 || cutmp->userno <= 0)
    reset_utmp();

  seecloak = HAS_PERM(PERM_SEECLOAK);

  up = ushm->uslot;
  uceil = (void *) up + ushm->offset;

  max = 0;
  bad = 0;
#ifdef HAVE_BOARD_PAL
  board_pals = 0;
  isbpal = (cutmp->board_pal != -1);
#endif
#ifdef  FRIEND_FIRST	/* by visor : ­«¼g¦n¤Í¸m«e */

  friend_num = ofriend_num = pfriend_num = nf_num = bfriend_num = 0;

  do					/* ¥ý§ä¦n¤Í */
  {
    userno = up->userno;
    if (userno <= 0 || (up->pid <= 0 && !HAS_PERM(PERM_SYSOP|PERM_SEECLOAK)))
      continue;
    if (!seecloak && (up->ufo & UFO_CLOAK))
      continue;      
    tmp = can_see(up);
    if(is_bad(userno)) 
    {
      bfriend_num++;
      bad = 1;
    }
    else
      bad = 0;
      
    if (((seecloak || !(up->ufo & UFO_CLOAK)) && (tmp != 2)) || HAS_PERM(PERM_SYSOP|PERM_SEECLOAK) || up->userno == cuser.userno)
    {
#ifdef HAVE_BOARD_PAL
      if(isbpal && is_boardpal(up)) 
        board_pals++;     
#endif
      ispal = is_pal(userno);

      if (!bad && (ispal && (tmp == 1)) || (userno == self))
      {
         pp->utmp = up;
         pp->type = 1;
         friend_num++;
         pp++;
      }
      else if(ispal && !(tmp == 1) && !filter && !bad)
      {
         pp->utmp = up;
         pp->type = 2;
         pfriend_num++;
         pp++;
      }
      else if(!ispal && (tmp == 1) && !filter && !bad)
      {
         pp->utmp = up;
         pp->type = 3;
         ofriend_num++;
         pp++;
      }
      else if(!filter)
      {
         pp->utmp = up;
         pp->type = 4;
         nf_num++;
         pp++;
      }
    }
  } while (++up <= uceil);
#else
  do
  {
    userno = up->userno;
    if (userno <= 0)
      continue;
    if ((userno == self) || ((seecloak || !(up->ufo & UFO_CLOAK))&&(can_see(up)!=2 || HAS_PERM(PERM_SYSOP)) && (!filter || is_pal(userno))))
    {    
      *pp++ = up;
    }
  } while (++up <= uceil);

#endif

  xo->max = max = pp - ulist_pool;

  if (xo->pos >= max)
    xo->pos = xo->top = 0;

  if ((max > 1) && (pickup_way))
  {
     xsort(ulist_pool, max, sizeof(PICKUP), ulist_cmp[pickup_way - 1]);
  }
  total_num = max;
  
/* cache.101023: shmÃz¬µ³y¦¨¤H¼Æ¶Ã±¼«áªº¦Û°Ê­×¥¿ */
#ifdef AUTO_FIX_INFO
  ushm->count = total_num;
  
  extern BCACHE *bshm;
  if ((fd = brd_bno(currboard)) >= 0)
    bshm->mantime[fd] =   board_pals;       /* ³Ì«á¬Ýªº¨º­ÓªO¤H¼Æ§ó·s */
#endif  
  
  return ulist_head(xo);
}



static int
ulist_neck(xo)
  XO *xo;
{
  move(1, 0);
#ifdef HAVE_BOARD_PAL
  prints("  ±Æ¦C¤è¦¡¡G[\033[1m%s\033[m] ¤W¯¸¤H¼Æ¡G%d %s§ÚªºªB¤Í¡G%d %s»P§Ú¬°¤Í¡G%d %sÃa¤H¡G%d \033[0;36mªO¤Í¡G%d\033[m\n"
    "\033[30;47m No.  ¥N¸¹         %-22s%-13s   PM %-14s¶¢¸m\033[m",
    msg_pickup_way[pickup_way],total_num,COLOR_PAL,friend_num+pfriend_num,COLOR_OPAL,friend_num+ofriend_num,COLOR_BAD,bfriend_num,board_pals,
    (HAS_PERM(PERM_SYSOP) && (cuser.ufo2 & UFO2_REALNAME)) ? "¯u¹ê©m¦W" : "¼Ê  ºÙ",
    (cuser.ufo2 & UFO2_SHIP) ? "¦n¤Í´y­z" :"¬G¶m", "°ÊºA");
#else
  prints("  ±Æ¦C¤è¦¡¡G[\033[1m%s\033[m] ¤W¯¸¤H¼Æ¡G%d %s§ÚªºªB¤Í¡G%d %s»P§Ú¬°¤Í¡G%d %sÃa¤H¡G%d\033[m\n"
    "\033[30;47m No.  ¥N¸¹         %-22s%-13s   PM %-14s¶¢¸m \033[m",
    msg_pickup_way[pickup_way],total_num,COLOR_PAL,friend_num+pfriend_num,COLOR_OPAL,friend_num+ofriend_num,COLOR_BAD,bfriend_num,
    (HAS_PERM(PERM_SYSOP) && (cuser.ufo & UFO_REALNAME)) ? "¯u¹ê©m¦W" : "¼Ê  ºÙ",
    (cuser.ufo2 & UFO2_SHIP) ? "¦n¤Í´y­z" :"¬G¶m", "°ÊºA");
#endif

  return ulist_body(xo);
}


static int
ulist_head(xo)
  XO *xo;
{
  vs_head((cuser.ufo2 & UFO2_PAL)?"¦n¤Í¦Cªí":"ºô¤Í¦Cªí", str_site);
  return ulist_neck(xo);
}


static int
ulist_toggle(xo)
  XO *xo;
{
  int ans, max;
  ans = pickup_way + 1;
  ans %= PICKUP_WAYS;
  
  pickup_way = ans;
  max = xo->max;
  if (max <= 1)
    return XO_FOOT;
  return ulist_init(xo);
}


static int
ulist_pal(xo)
  XO *xo;
{
  cuser.ufo2 ^= UFO2_PAL;
  /* Thor.980805: ª`·N ufo ¦P¨B°ÝÃD */
  return ulist_init(xo);
}


static int
ulist_search(xo, step)
  XO *xo;
  int step;
{
  int num, pos, max;
  PICKUP *pp;
  static char buf[IDLEN + 1];

  if (vget(b_lines, 0, msg_uid, buf, IDLEN + 1, GCARRY))
  {
    int buflen;
    char bufl[IDLEN + 1];

    str_lower(bufl, buf);
    buflen = strlen(bufl); /* Thor: ¥²©w¤j©ó0 */
    
    pos = num = xo->pos;
    max = xo->max;
    pp = ulist_pool;
    do
    {
      pos += step;
      if (pos < 0) /* Thor.990124: °²³] max ¤£¬°0 */
        pos = max - 1;
      else if (pos >= max)
	pos = 0;

      /* Thor.990124: id «h±qÀY match */
      /* if (str_ncmp(pp[pos]->userid, bufl, buflen)==0 */
      
      if (str_str(pp[pos].utmp->userid, bufl) /* lkchu.990127: §ä³¡¥÷ id ¦n¹³¤ñ¸û¦n¥Î :p */
       || str_str(pp[pos].utmp->username, bufl)) /* Thor.990124: ¥i¥H§ä ³¡¤À nickname */
      {
	move(b_lines, 0);
	clrtoeol();
	return pos + XO_MOVE;
      }

    } while (pos != num);
  }

  return XO_FOOT;
}

static int
ulist_search_forward(xo)
  XO *xo;
{
  return ulist_search(xo, 1); /* step = +1 */
}

static int
ulist_search_backward(xo)
  XO *xo;
{
  return ulist_search(xo, -1); /* step = -1 */
}



static int
ulist_makepal(xo)
  XO *xo;
{
  if (cuser.userlevel)
  {
    UTMP *up;
    int userno;

    up = ulist_pool[xo->pos].utmp;
    userno = up->userno;
    if (userno > 0 && !is_pal(userno) && !is_bad(userno)   /* ©|¥¼¦C¤J¦n¤Í¦W³æ */
         && (userno != cuser.userno))	/* lkchu.981217: ¦Û¤v¤£¥i¬°¦n¤Í */
					
    {
      PAL pal;
      char buf[80];

      strcpy(buf, up->userid);

      vget(b_lines, 0, "¦n¤Í´y­z¡G", pal.ship, sizeof(pal.ship), DOECHO);
      pal.ftype = 0;
      pal.userno = userno;
      strcpy(pal.userid, buf);
      usr_fpath(buf, cuser.userid, FN_PAL);
      if(rec_num(buf,sizeof(PAL)) < PAL_MAX)
      {
        rec_add(buf, &pal, sizeof(PAL));
        pal_cache();
        return ulist_init(xo);
      }
      else
      {
        vmsg("±zªº¦n¤Í¦W³æ¤Ó¦h¡A½Ðµ½¥[¾ã²z");
      }
    }
  }
  return XO_FOOT;
}

static int
ulist_makebad(xo)
  XO *xo;
{
  if (cuser.userlevel)
  {
    UTMP *up;
    int userno;

    up = ulist_pool[xo->pos].utmp;
    userno = up->userno;
    if (userno > 0 && !is_pal(userno) && !is_bad(userno)  /* ©|¥¼¦C¤J¦n¤Í¦W³æ */
         && (userno != cuser.userno))	/* lkchu.981217: ¦Û¤v¤£¥i¬°¦n¤Í */
					
    {
      PAL pal;
      char buf[80];

      strcpy(buf, up->userid);

      vget(b_lines, 0, "´c¦æ´cª¬¡G", pal.ship, sizeof(pal.ship), DOECHO);
      pal.ftype = PAL_BAD;
      pal.userno = userno;
      strcpy(pal.userid, buf);
      usr_fpath(buf, cuser.userid, FN_PAL);
      if(rec_num(buf,sizeof(PAL)) < PAL_MAX)
      {
        rec_add(buf, &pal, sizeof(PAL));
        pal_cache();
        return ulist_init(xo);
      }
      else
        vmsg("±zªº¦n¤Í¦W³æ¤Ó¦h¡A½Ðµ½¥[¾ã²z");
    }
  }
  return XO_NONE;
}


static int
ulist_mail(xo)
  XO *xo;
{
  char userid[IDLEN + 1];

  /* Thor.981022:¤£µ¹¨S°ò¥»Åvªº±H«H */
  if (!HAS_PERM(PERM_INTERNET) || HAS_PERM(PERM_DENYMAIL) || !cuser.userlevel)
    return XO_NONE;

  strcpy(userid, ulist_pool[xo->pos].utmp->userid);
  if (*userid)
  {
    vs_bar("±H  «H");
    prints("¦¬«H¤H¡G%s", userid);
    my_send(userid);
    return ulist_init(xo);
  }

  vmsg(MSG_USR_LEFT);
  return XO_FOOT;
}


static int
ulist_query(xo)
  XO *xo;
{
  move(1, 0);
  clrtobot();
  my_query(ulist_pool[xo->pos].utmp->userid, 0);
  /*ulist_neck(xo);*/
  return XO_INIT;
}


static int
ulist_broadcast(xo)
  XO *xo;
{
  int num;
  PICKUP *pp;
  UTMP *up;
  BMW bmw;
  char buf[80],ans,admin;

  num = cuser.userlevel;
  if (!(num & (PERM_SYSOP)) &&
    (!(num & PERM_PAGE) || !(cuser.ufo2 & UFO2_PAL)))
    return XO_NONE;

  num = xo->max;
  if (num < 1)
    return XO_NONE;

  bmw.caller = 0;
  bmw_edit(NULL, "¡¹¼s¼½¡G", &bmw, 0);
  sprintf(buf,"¡¹¼s¼½¡G%s",bmw.msg);
  strcpy(bmw.msg,buf);
  admin = check_admin(cuser.userid);
  if(admin && !(cuser.ufo2 & UFO2_PAL))
  {
    if((ans = vans("¡· ¨Ï¥Î SYSOP ¼s¼½¶Ü¡H [y/N]")) != 'Y' && ans != 'y')
      admin = 0;
    if((ans = vans("¡· ½T©w¥þ¯¸¼s¼½¶Ü¡H [y/N]")) != 'Y' && ans != 'y')
      return XO_INIT;
  }
  if (!(cuser.ufo2 & UFO2_PAL) && admin)
  {
    strcpy(bmw.userid,"SYSOP");
    /*bmw.sender = 1;*/
  }
  if (bmw.caller)
  {
    pp = ulist_pool;
    while (--num >= 0)
    {
      up = pp[num].utmp;
      if (can_message(up) && (!(up->ufo & UFO_BROADCAST)||
         (HAS_PERM(PERM_SYSOP|PERM_CHATROOM) && !(cuser.ufo2 & UFO2_PAL))))
      {
	bmw.recver = up->userno;
	bmw_send(up, &bmw);
      }
    }
  }
  return XO_INIT;
}


static int
ulist_talk(xo)
  XO *xo;
{
  if (HAS_PERM(PERM_PAGE))
  {
    UTMP *up;

    up = ulist_pool[xo->pos].utmp;
    if (can_override(up))
      return talk_page(up) ? ulist_init(xo) : XO_FOOT;
  }
  return XO_NONE;
}


static int
ulist_write(xo)
  XO *xo;
{
  if (HAS_PERM(PERM_PAGE))
  {
    UTMP *up;

    up = ulist_pool[xo->pos].utmp;
    if (can_message(up))
    {
      BMW bmw;
      char buf[20];

#ifdef	HAVE_SHOWNUMMSG
      if(up->num_msg > 9 && (up->ufo & UFO_MAXMSG) && !HAS_PERM(PERM_SYSOP))
      {
        vmsg("¹ï¤è¤w¸g³Q¤ô²yÄéÃz¤F!!");
        return XO_INIT;
      }
#endif

      sprintf(buf, "¡¹[%s]", up->userid);
      bmw_edit(up, buf, &bmw, 0);
    }
#ifdef	HAVE_BANMSG    
    else if(!(up->ufo & UFO_MESSAGE) && can_banmsg(up))
    {
      vmsg("¹ï¤è¤£·QÅ¥¨ì±zªºÁn­µ!!");
    }
#endif    
    return XO_INIT;
  }
  return XO_NONE;
}


static int
ulist_edit(xo)			/* Thor: ¥i½u¤W¬d¬Ý¤Î­×§ï¨Ï¥ÎªÌ */
  XO *xo;
{
  ACCT acct;

  if (!HAS_PERM(PERM_SYSOP) ||
    acct_load(&acct, ulist_pool[xo->pos].utmp->userid) < 0)
    return XO_NONE;

  vs_bar("¨Ï¥ÎªÌ³]©w");
  acct_setup(&acct, 1);
  return ulist_head(xo);
}

/* BLACK SU */
static int
ulist_su(xo)
  XO *xo;
{
  XO *tmp;
  ACCT acct;
  char path[80];  
  int level,ufo;
  ufo = cuser.ufo;
  level = cuser.userlevel;
  if (!supervisor ||
      acct_load(&acct, ulist_pool[xo->pos].utmp->userid) < 0)
      return XO_NONE;

  memcpy(&cuser,&acct,sizeof(ACCT));
  cuser.userlevel = level;
  cuser.ufo = ufo;
  usr_fpath(path,acct.userid,".DIR");  
  tmp = xz[XZ_MBOX - XO_ZONE].xo;
  xz[XZ_MBOX - XO_ZONE].xo =  xo_new(path);
  free(tmp);
  usr_fpath(path, acct.userid, FN_BMW);
  tmp = xz[XZ_BMW - XO_ZONE].xo;
  xz[XZ_BMW - XO_ZONE].xo =  xo_new(path);
  free(tmp);
  pal_cache();  
  return ulist_init(xo);
}
/* BLACK SU */

static int
ulist_kick(xo)
  XO *xo;
{
  ACCT u;
  acct_load(&u, ulist_pool[xo->pos].utmp->userid);
  if ((HAS_PERM(PERM_SYSOP)&& (!(u.userlevel & PERM_SYSOP) || !strcmp(cuser.userid,u.userid)) )||check_admin(cuser.userid))
  {
    UTMP *up;
    pid_t pid;
    char buf[80];

    up = ulist_pool[xo->pos].utmp;
    if ((pid = up->pid))
    {
      if (vans(msg_sure_ny) != 'y' || pid != up->pid)
	return XO_FOOT;

      sprintf(buf, "%s (%s)", up->userid, up->username);

      if ((kill(pid, SIGTERM) == -1) && (errno == ESRCH))
        memset(up,0,sizeof(UTMP));

      blog("KICK ", buf);
      return ulist_init(xo);
    }
    else
    {
      if (vans(msg_sure_ny) != 'y')
        return XO_FOOT;
      memset(up,0,sizeof(UTMP));
      return ulist_init(xo);
    }
  }
  return XO_NONE;
}


#ifdef	HAVE_CHANGE_FROM
static int
ulist_fromchange(xo)
  XO *xo;
{
  char *str, buf[34];
  
  if (!HAS_PERM(PERM_ADMIN))
    return XO_NONE;
  
  strcpy(buf, str = cutmp->from);
  vget(b_lines, 0, "½Ð¿é¤J·sªº¬G¶m¡G", buf, sizeof(cutmp->from), GCARRY);
  if (strcmp(buf, str))
  {
    strcpy(str, buf);
    strcpy(cutmp->from, buf);
    return XO_INIT;
    /*ulist_body(xo);*/
  }

  return XO_FOOT;
}
#endif


static int
ulist_nickchange(xo)
  XO *xo;
{
  char *str, buf[24];

  if (!HAS_PERM(PERM_VALID) || (HAS_PERM(PERM_DENYNICK)&&!HAS_PERM(PERM_SYSOP)))
    return XO_NONE;

  strcpy(buf, str = cuser.username);
  vget(b_lines, 0, "½Ð¿é¤J·sªº¼ÊºÙ¡G", buf, sizeof(cuser.username), GCARRY);

  if (strcmp(buf, str) && str_len(buf) > 0 )
  {
    strcpy(str, buf);
    strcpy(cutmp->username, buf);
    return XO_INIT;
    /*ulist_body(xo);*/
  }
  return XO_FOOT;
}


static int
ulist_help(xo)
  XO *xo;
{
  film_out(FILM_ULIST, -1);
  return ulist_init(xo);
}


#if 0
static int
ulist_exotic(xo)
  XO *xo;
{
  UTMP *uhead, *utail, *uceil;
  char buf[80];

  if (!HAS_PERM(PERM_SYSOP))
    return XO_NONE;

  uhead = ushm->uslot;
  uceil = ushm->uceil;
  utail = uhead + MAXACTIVE - 1;
  sprintf(buf, "%s %p [%p, %p]\n", Now(), uceil, uhead, utail);
  f_cat("run/ushm.log", buf);

  ushm->uceil = utail;
  return ulist_init(xo);
}
#endif

static int
ulist_pager(xo)
  XO *xo;
{
  if (!HAS_PERM(PERM_PAGE))
      return XO_NONE;
   if((cuser.ufo & UFO_PAGER) && (cuser.ufo & UFO_PAGER1))
   {
   cuser.ufo ^= UFO_PAGER;
   cutmp->ufo ^= UFO_PAGER;
   cuser.ufo ^= UFO_PAGER1;
   cutmp->ufo ^= UFO_PAGER1;
   }
   else if(cuser.ufo & UFO_PAGER)
   {
   cuser.ufo ^= UFO_PAGER1;
   cutmp->ufo ^= UFO_PAGER1;
   }
   else 
   {
   cuser.ufo ^= UFO_PAGER;
   cutmp->ufo ^= UFO_PAGER;
   }
   return ulist_init(xo);
}

static int
ulist_message(xo)
  XO *xo;
{

  if (!HAS_PERM(PERM_PAGE))
        return XO_NONE;
   if((cuser.ufo & UFO_QUIET) && (cuser.ufo & UFO_MESSAGE))
   {
   cuser.ufo ^= UFO_QUIET;
   cutmp->ufo ^= UFO_QUIET;
   cuser.ufo ^= UFO_MESSAGE;
   cutmp->ufo ^= UFO_MESSAGE;
   }
   else if(cuser.ufo & UFO_QUIET)
   {
   cuser.ufo ^= UFO_MESSAGE;
   cutmp->ufo ^= UFO_MESSAGE;
   }
   else 
   {
   cuser.ufo ^= UFO_QUIET;
   cutmp->ufo ^= UFO_QUIET;
   }
   return ulist_init(xo);
}

static int
ulist_recall(xo)
  XO *xo;
{
  t_recall();
  return ulist_init(xo);
}

static int
ulist_realname(xo)
  XO *xo;
{
  if(HAS_PERM(PERM_SYSOP))
  {
     cuser.ufo2 ^= UFO2_REALNAME;
//     cutmp->ufo ^= UFO_REALNAME;
  } 
  return ulist_init(xo);
}

static int
ulist_ship(xo)
  XO *xo;
{
  cuser.ufo2 ^= UFO2_SHIP;
//  cutmp->ufo ^= UFO_SHIP;
  return ulist_init(xo);
}

static int
ulist_mp(xo)
  XO *xo;
{
  int tmp;
  
  if(!HAS_PERM(PERM_VALID))
    return XO_NONE;
  tmp = t_pal();
  return ulist_init(xo);
}

static int
ulist_readmail(xo)
  XO *xo;
{
  if(cuser.userlevel)
  {
    if (HAS_PERM(PERM_DENYMAIL))
      vmsg("±zªº«H½c³QÂê¤F¡I");
    else 
      xover(XZ_MBOX);
    return ulist_init(xo);
  }
  else
    return XO_NONE;
}

static int
ulist_del(xo)
  XO *xo;
{
  UTMP *up;
  char ans;
  int userno;
  int  fd,tmp=0;
  char fpath[64];
  PAL *pal;

  ans = vans("¬O§_§R°£(y/N)¡G");
  if (ans == 'y' || ans == 'Y')
  { 
    up = ulist_pool[xo->pos].utmp;
    userno = up->userno;

    usr_fpath(fpath, cuser.userid, FN_PAL);
    if ((fd = open(fpath, O_RDONLY)) >= 0)
    {
      mgets(-1);
      while ((pal = mread(fd, sizeof(PAL))))
      {
          if(pal->userno!=userno)
            tmp = tmp + 1;
          else
          { 
            rec_del(fpath,sizeof(PAL),tmp,NULL,NULL);
            break;
          }
      }
      close(fd);
    }
    pal_cache();
    return ulist_init(xo);
  }
  else
    return XO_FOOT;
}

static int
ulist_changeship(xo)
  XO *xo;
{
  UTMP *up;
  int userno;
  int  fd,tmp=0;
  char fpath[64],buf[46];
  PAL *pal;
  int check;

  up = ulist_pool[xo->pos].utmp;
  userno = up->userno;


  check = is_pal(userno) ? 1 : is_bad(userno) ? 2 : 0;
  if(!check)
      return XO_NONE;
  strcpy(buf,"");
  copyship(buf,userno);

  if(vget(b_lines, 0, check == 1 ?"¤Í½Ë¡G":"´c¦æ´cª¬¡G", buf, sizeof(buf), GCARRY))
  {
    usr_fpath(fpath, cuser.userid, FN_PAL);
    if ((fd = open(fpath, O_RDONLY)) >= 0)
    {
      mgets(-1);
      while ((pal = mread(fd, sizeof(PAL))))
      {
          if(pal->userno!=userno)
            tmp = tmp + 1;
          else
          {
            strcpy(pal->ship,buf);
            rec_put(fpath, pal, sizeof(PAL), tmp);
            break;
          }
      }
      close(fd);
    }
    pal_cache();
    return ulist_init(xo);
  }
  else
    return XO_HEAD;
}

#if 1
static int
ulist_test(xo)
  XO *xo;
{
  int fd;
  char buf[128];
  fd = open(FN_ETC_SYSOP, O_RDONLY);
  sprintf(buf,"ÀÉ®×½s¸¹ %d:%d:%d",fd,Ctrl('F'),'@');
  if(fd)
    close(fd);
  pmsg(buf);
/*  vget(b_lines,0,"¦h¤Ö¤H:",buf,6,DOECHO);
  ushm->count = atoi(buf);*/
  return XO_INIT;
}

static int
ulist_state(xo)
  XO *xo;
{
  char buf[128];
  if(!HAS_PERM(PERM_SYSOP))
    return XO_NONE;
  sprintf(buf,"PID : %d",ulist_pool[xo->pos].utmp->pid);
  vmsg(buf);
  return XO_INIT;
}
#endif

#ifdef	APRIL_FIRST
static int
ulist_april1(xo)
  XO *xo;
{
  char buf[256];
  more("gem/brd/Admin/J/A106LL7J",NULL);
  sprintf(buf,"±z¬O²Ä %d ­Ó³QÄFªº¨Ï¥ÎªÌ ^^y ",ushm->avgload);
  if(!aprilfirst)
    ushm->avgload++;
  vmsg(buf);
  aprilfirst = 1;
  return XO_INIT;
}
#endif

KeyFunc ulist_cb[] =
{
  {XO_INIT, ulist_init},
  {XO_LOAD, ulist_body},
  {XO_NONE, ulist_init},
#if 1
  {'V', ulist_test},
  {'S', ulist_state},
#endif
  {'y', ulist_readmail},
/* BLACK SU */
  {'u', ulist_su},
/* BLACK SU */
  {'m', ulist_message},
  {'Z', ulist_ship},
  {'f', ulist_pal},
  {'a', ulist_makepal},
  {'A', ulist_makebad},
  {'t', ulist_talk},
  {'w', ulist_write},
  {'l', ulist_recall},		/* Thor: ¼ö°T¦^ÅU */
  {'j', ulist_changeship},
  {'q', ulist_query},
  {'b', ulist_broadcast},
  {'s', ulist_init},		/* refresh status Thor: À³user­n¨D */
  {'c', t_cloak},
  {'R', ulist_realname},
  {'o', ulist_mp},
  {'d', ulist_del},
  {'p', ulist_pager},
  {Ctrl('Q'),ulist_query},
#if 0
  {'x', ulist_exotic},
#endif

  {Ctrl('K'), ulist_kick},
  {Ctrl('X'), ulist_edit},
  {'g', ulist_nickchange},
#ifdef HAVE_CHANGE_FROM
  {Ctrl('F'), ulist_fromchange},
#endif
  
#if 0
  {'/', ulist_search},
#endif
  /* Thor.990125: ¥i«e«á·j´M, id or nickname */
  {'/', ulist_search_forward},
  {'?', ulist_search_backward},

#ifdef  APRIL_FIRST
  {'X', ulist_april1},
#endif

  {'M', ulist_mail},
  {KEY_TAB, ulist_toggle},
  {'h', ulist_help}
};


/* ----------------------------------------------------- */
/* talk main-routines					 */
/* ----------------------------------------------------- */

int
t_message()
{
   if((cuser.ufo & UFO_QUIET) && (cuser.ufo & UFO_MESSAGE))
   {
   cuser.ufo ^= UFO_QUIET;
   cutmp->ufo ^= UFO_QUIET;
   cuser.ufo ^= UFO_MESSAGE;
   cutmp->ufo ^= UFO_MESSAGE;
   }
   else if(cuser.ufo & UFO_QUIET)
   {
   cuser.ufo ^= UFO_MESSAGE;
   cutmp->ufo ^= UFO_MESSAGE;
   }
   else 
   {
   cuser.ufo ^= UFO_QUIET;
   cutmp->ufo ^= UFO_QUIET;
   }
  return 0;
}


int
t_pager()
{
  /* cuser.ufo = (cutmp->ufo ^= UFO_PAGER); */
   if((cuser.ufo & UFO_PAGER) && (cuser.ufo & UFO_PAGER1))
   {
   cuser.ufo ^= UFO_PAGER;
   cutmp->ufo ^= UFO_PAGER;
   cuser.ufo ^= UFO_PAGER1;
   cutmp->ufo ^= UFO_PAGER1;
   }
   else if(cuser.ufo & UFO_PAGER)
   {
   cuser.ufo ^= UFO_PAGER1;
   cutmp->ufo ^= UFO_PAGER1;
   }
   else 
   {
   cuser.ufo ^= UFO_PAGER;
   cutmp->ufo ^= UFO_PAGER;
   }
  return 0;
}

int
t_cloak(xo)
  XO *xo;
{
  if(HAS_PERM(PERM_CLOAK))
  {
    cuser.ufo ^= UFO_CLOAK;
    cutmp->ufo ^= UFO_CLOAK;
  } /* Thor.980805: ¸Ñ¨M ufo¤£¦P¨B°ÝÃD */
  return ulist_init(xo);
}




int
t_query()
{
  ACCT acct;

  vs_bar("¬d¸ßºô¤Í");
  if (acct_get(msg_uid, &acct) > 0)
  {
    move(2, 0);
    clrtobot();
    do_query(&acct, 0);
  }

  return 0;
}


#if 0
static int
talk_choose()
{
  UTMP *up, *ubase, *uceil;
  int self, seecloak;
  char userid[IDLEN + 1];

  ll_new();

  self = cuser.userno;
  seecloak = HAS_PERM(PERM_SEECLOAK);
  ubase = up = ushm->uslot;
  uceil = ushm->uceil;
  do
  {
    if (up->pid && up->userno != self &&
      ((seecloak || !(up->ufo & UFO_CLOAK))&&(can_see(up)!=2 || HAS_PERM(PERM_SYSOP))))
      AddLinkList(up->userid);
  } while (++up <= uceil);

  if (!vget(1, 0, "½Ð¿é¤J¥N¸¹¡G", userid, IDLEN + 1, GET_LIST))
    return 0;

  up = ubase;
  do
  {
    if (!str_cmp(userid, up->userid))
      return up->userno;
  } while (++up <= uceil);

  return 0;
}


int
t_talk()
{
  int tuid, unum, ucount;
  UTMP *up;
  char ans[4];
  BMW bmw;

  if (ushm->count <= 1)
  {
    outs("¥Ø«e½u¤W¥u¦³±z¤@¤H¡A§ÖÁÜ½ÐªB¤Í¨Ó¥úÁ{¡i" BOARDNAME "¡j§a¡I");
    return XEASY;
  }

  tuid = talk_choose();
  if (!tuid)
    return 0;

  /* ----------------- */
  /* multi-login check */
  /* ----------------- */

  move(3, 0);
  unum = 1;
  while ((ucount = utmp_count(tuid, 0)) > 1)
  {
    outs("(0) ¤£·Q talk ¤F...\n");
    utmp_count(tuid, 1);
    vget(1, 33, "½Ð¿ï¾Ü¤@­Ó²á¤Ñ¹ï¶H [0]¡G", ans, 3, DOECHO);
    unum = atoi(ans);
    if (unum == 0)
      return 0;
    move(3, 0);
    clrtobot();
    if (unum > 0 && unum <= ucount)
      break;
  }

  if (up = utmp_search(tuid, unum))
  {
    if (can_override(up))
    {
      ucount = vget(3, 0, "½T©w­n²á¤Ñ¶Ü¡H T)alk W)rite Q)uit [T] ",
	ans, 3, LCECHO);

      if (ucount == 'q')
	return 0;

      if (tuid != up->userno)
      {
	vmsg(MSG_USR_LEFT);
	return 0;
      }

      if (ucount == 'w')
      {
	bmw_edit(up, "¡¹¼ö°T¡G", &bmw, 0);
      }
      else
      {
	talk_page(up);
      }
    }
    else
    {
      vmsg("¹ï¤èÃö±¼©I¥s¾¹¤F");
    }
  }

  return 0;
}
#endif


/* ------------------------------------- */
/* ¦³¤H¨Ó¦êªù¤l¤F¡A¦^À³©I¥s¾¹		 */
/* ------------------------------------- */


void
talk_rqst()
{
  UTMP *up;
  int mode, sock, ans, len, port;
  char buf[80];
  struct sockaddr_in sin;
  screenline sl[b_lines + 1];
#if     defined(__OpenBSD__)
  struct hostent *h;
#endif

  up = cutmp->talker;
  if (!up)
    return;
  up->talker = cutmp;

  port = up->sockport;
  if (!port)
    return;

  mode = bbsmode;
#ifdef  HAVE_PIP_FIGHT
  if(up->mode == M_CHICKEN)
    utmp_mode(M_CHICKEN);
  else
#endif
    utmp_mode(M_TRQST);

  vs_save(sl);

  clear();
  sprintf(page_requestor, "%s (%s)", up->userid, up->username);

#ifdef EVERY_Z
  /* Thor.0725: ¬° talk & chat ¥i¥Î ^z §@·Ç³Æ */

  if (holdon_fd)
  {
    sprintf(buf, "%s ·Q©M±z²á¡A¤£¹L±z¥u¦³¤@±i¼L", page_requestor);
    vmsg(buf);
    buf[0] = ans = '6';		/* Thor.0725:¥u¦³¤@±i¼L */
    len = 1;
    goto over_for;
  }
#endif

  bell();
#ifdef  HAVE_PIP_FIGHT
  if(up->mode != M_CHICKEN)
#endif
  {
    prints("±z·Q¸ò %s ²á¤Ñ¶Ü¡H(¨Ó¦Û %s)", page_requestor, up->from);
    for (;;)
    {
      ans = vget(1, 0, "==> Yes, No, Query¡H[Y] ", buf, 3, LCECHO);
      if (ans == 'q')
      {
        my_query(up->userid, 0);
      }
      else
        break;
    }
  }
#ifdef  HAVE_PIP_FIGHT
  else
  {
    prints("±z·Q¸ò %s PK ¤pÂû¶Ü¡H(¨Ó¦Û %s)", page_requestor, up->from);
    ans = vget(1, 0, "==> Yes, No¡H[Y] ", buf, 3, LCECHO);
  }
#endif

  len = 1;

  if (ans == 'n')
  {
    move(2, 0);
    clrtobot();
#ifdef  HAVE_PIP_FIGHT
    if(up->mode != M_CHICKEN)
#endif
    {
      for (ans = 0; ans < 5; ans++)
        prints("\n (%d) %s", ans + 1, talk_reason[ans]);
      ans = vget(10, 0, "½Ð¿é¤J¿ï¶µ©Î¨ä¥L±¡¥Ñ [1]¡G\n==> ",
        buf + 1, sizeof(buf) - 1, DOECHO);

      if (ans == 0)
        ans = '1';

      if (ans >= '1' && ans <= '5')
      {
        buf[0] = ans;
      }
      else
      {
        buf[0] = ans = ' ';
        len = strlen(buf);
      }
    }
#ifdef  HAVE_PIP_FIGHT
    else
      buf[0] = ans = 'C';
#endif
  }
  else
  {
#ifdef  HAVE_PIP_FIGHT
    if(up->mode != M_CHICKEN)
#endif
      buf[0] = ans = 'y';
#ifdef  HAVE_PIP_FIGHT
    else
      buf[0] = ans = 'c';
#endif
  }

#ifdef EVERY_Z
over_for:
#endif

  sock = socket(AF_INET, SOCK_STREAM, 0);

#if     defined(__OpenBSD__)                    /* lkchu */

  if (!(h = gethostbyname(MYHOSTNAME)))
    return;
  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_port = port;
  memcpy(&sin.sin_addr, h->h_addr, h->h_length);
                
#else

  sin.sin_family = AF_INET;
  sin.sin_port = port;
  sin.sin_addr.s_addr = INADDR_ANY /* INADDR_LOOPBACK */;
  memset(sin.sin_zero, 0, sizeof(sin.sin_zero));

#endif

  if (!connect(sock, (struct sockaddr *) & sin, sizeof(sin)))
  {
    send(sock, buf, len, 0);

    if (ans == 'y')
    {
      strcpy(cutmp->mateid, up->userid);
      talk_speak(sock);
    }
#ifdef  HAVE_PIP_FIGHT
    else if(ans == 'c')
    {
      if(!p)
        p = DL_get("bin/pip.so:pip_vf_fight");
      strcpy(cutmp->mateid, up->userid);
      if(p)
	  {
	    cutmp->pip = NULL;
        (*p)(sock,1);
	    cutmp->pip = NULL;

	  }
      add_io(0,60);
/*      pip_vf_fight(sock,2);*/
    }
#endif
  }

  close(sock);
#ifdef  LOG_TALK
  if (ans == 'y' && up->mode != M_CHICKEN) /* mat.991011: ¨¾¤îTalk©Úµ´®É¡A²£¥Í²á¤Ñ°O¿ýªºrecord */
    talk_save();          /* lkchu.981201: talk °O¿ý³B²z */
#endif
  vs_restore(sl);
  utmp_mode(mode);
}


void
talk_main()
{
  char fpath[64];
  
  xz[XZ_ULIST - XO_ZONE].xo = &ulist_xo;
  xz[XZ_ULIST - XO_ZONE].cb = ulist_cb;

  xz[XZ_PAL - XO_ZONE].cb = pal_cb;
  
  /* lkchu.981230: §Q¥Î xover ¾ã¦X bmw */
  usr_fpath(fpath, cuser.userid, FN_BMW);
  xz[XZ_BMW - XO_ZONE].xo = xo_new(fpath);
  xz[XZ_BMW - XO_ZONE].cb = bmw_cb;
}

int
check_personal_note(newflag,userid)
  int newflag;
  char *userid;
{
 char fpath[256];
 int  fd,total = 0;
 notedata myitem;
 char *fn_note_dat      = "pnote.dat";

 if (userid == NULL)
   usr_fpath(fpath, cuser.userid, fn_note_dat);
 else
   usr_fpath(fpath, userid, fn_note_dat);

 if ((fd = open(fpath, O_RDONLY)) >=0)
 {
   while (read(fd,&myitem, sizeof(myitem)) == sizeof(myitem))
   {
     if (newflag)
     {
       if (myitem.mode == 0) total++;
     }
     else
       total++;
   }
   close(fd);
   return total;
 }
 return 0;
}

#ifdef	HAVE_BANMSG
void
banmsg_cache()
{
  int count, fsize, fd;
  int *plist, *cache;
  BANMSG *phead, *ptail;
  char *fimage, fpath[64];
  UTMP *up;

  up = cutmp;
  cutmp->userno = cuser.userno;

  cache = NULL;
  count = 0;

  fsize = 0;
  usr_fpath(fpath, cuser.userid, FN_BANMSG);
  fimage = f_img(fpath, &fsize);
  if((fsize > (BANMSG_MAX * sizeof(BANMSG))) && (fd = open(fpath,O_RDWR)))
  {
    ftruncate(fd, BANMSG_MAX * sizeof(BANMSG));
    close(fd);
  }
  if (fimage != NULL)
  {
    if (fsize > (BANMSG_MAX * sizeof(BANMSG)))
    {
      fsize = BANMSG_MAX * sizeof(BANMSG);
    }

    count = fsize / sizeof(BANMSG);
    if (count)
    {
      cache = plist = up->banmsg_spool;
      phead = (BANMSG *) fimage;
      ptail = (BANMSG *) (fimage + fsize);
      do
      {
	*plist++ = phead->userno;
      } while (++phead < ptail);

      if (count > 0)
      {
	if (count > 1)
	  xsort(cache, count, sizeof(int), int_cmp);
      }
    }
  }

  up->banmsg_max = count;
  
  if(fimage) 
    free(fimage);
}


void
banmsg_sync(fpath)
  char *fpath;
{
  int fd, size=0;
  struct stat st;
  char buf[64];

  if (!fpath)
  {
    fpath = buf;
    usr_fpath(fpath, cuser.userid, FN_BANMSG);
  }

  if ((fd = open(fpath, O_RDWR, 0600)) < 0)
    return;

  outz("¡¹ ¸ê®Æ¾ã²z½]®Ö¤¤¡A½Ðµy­Ô \033[5m...\033[m");
  refresh();

  if (!fstat(fd, &st) && (size = st.st_size) > 0)
  {
    BANMSG *pbase, *phead, *ptail;
    int userno;

    pbase = phead = (BANMSG *) malloc(size);
    size = read(fd, pbase, size);
    if (size >= sizeof(BANMSG))
    {
      ptail = (BANMSG *) ((char *) pbase + size);
      while (phead < ptail)
      {
	userno = phead->userno;
	if (userno > 0 && userno == acct_userno(phead->userid))
	{
	  phead++;
	  continue;
	}

	ptail--;
	if (phead >= ptail)
	  break;
	memcpy(phead, ptail, sizeof(BANMSG));
      }

      size = (char *) ptail - (char *) pbase;
      if (size > 0)
      {
	if (size > sizeof(BANMSG))
	{
	  xsort(pbase, size / sizeof(BANMSG), sizeof(BANMSG), (void *)str_cmp);
	}

	lseek(fd, 0, SEEK_SET);
	write(fd, pbase, size);
	ftruncate(fd, size);
      }
    }
    free(pbase);
  }
  close(fd);

  if (size <= 0)
    unlink(fpath);
}


/* ----------------------------------------------------- */
/* ©Ú¦¬°T®§¦W³æ¡G¿ï³æ¦¡¾Þ§@¬É­±´y­z		 	 */
/* ----------------------------------------------------- */


static int banmsg_add();


static void
banmsg_item(num, banmsg)
  int num;
  BANMSG *banmsg;
{
  prints("%6d    %-14s%s\n", num, banmsg->userid, banmsg->ship);
}


static int
banmsg_body(xo)
  XO *xo;
{
  BANMSG *banmsg;
  int num, max, tail;

  move(3, 0);
  clrtobot();
  max = xo->max;
  if (max <= 0)
  {
    if (vans("­n·s¼W¶Ü(Y/N)¡H[N] ") == 'y')
      return banmsg_add(xo);
    return XO_QUIT;
  }

  banmsg = (BANMSG *) xo_pool;
  num = xo->top;
  tail = num + XO_TALL;
  if (max > tail)
    max = tail;

  do
  {
    banmsg_item(++num, banmsg++);
  } while (num < max);

  return XO_NONE;
}


static int
banmsg_head(xo)
  XO *xo;
{
  vs_head("©Ú¦¬¦W³æ", str_site);
  outs("\
  [¡ö]Â÷¶} a)·s¼W c)­×§ï d)§R°£ m)±H«H s)¾ã²z [q]¬d¸ß [h]elp\n\
\033[30;47m  ½s¸¹    ¥N ¸¹         ´y       ­z                                           \033[m");
  return banmsg_body(xo);
}


static int
banmsg_load(xo)
  XO *xo;
{
  xo_load(xo, sizeof(BANMSG));
  return banmsg_body(xo);
}


static int
banmsg_init(xo)
  XO *xo;
{
  xo_load(xo, sizeof(BANMSG));
  return banmsg_head(xo);
}


static void
banmsg_edit(banmsg, echo)
  BANMSG *banmsg;
  int echo;
{
  if (echo == DOECHO)
    memset(banmsg, 0, sizeof(BANMSG));
  vget(b_lines, 0, "´y­z¡G", banmsg->ship, sizeof(banmsg->ship), echo);
}


static int
banmsg_add(xo)
  XO *xo;
{
  ACCT acct;
  int userno;

  if (xo->max >= BANMSG_MAX)
  {
    vmsg("±zªº©Ú¦¬°T®§¦W³æ¤Ó¦h¡A½Ðµ½¥[¾ã²z");
    return XO_FOOT;
  }

  userno = acct_get(msg_uid, &acct);

#if 1				/* Thor.0709: ¤£­«ÂÐ¥[¤J */
  if ((xo->dir[0] == 'u') && is_banmsg(userno))
  {
    vmsg("¦W³æ¤¤¤w¦³¦¹¤H");
    return XO_FOOT;
  }
  else if (userno == cuser.userno)
  {
    vmsg("¦Û¤v¤£¶·¥[¤J©Ú¦¬°T®§¦W³æ¤¤");
    return XO_FOOT;
  }
#endif

  if (userno > 0)
  {
    BANMSG banmsg;

    banmsg_edit(&banmsg, DOECHO);
    strcpy(banmsg.userid, acct.userid);
    banmsg.userno = userno;
    rec_add(xo->dir, &banmsg, sizeof(BANMSG));
    xo->pos = XO_TAIL;
    xo_load(xo, sizeof(BANMSG));
  }

  banmsg_cache();

  return banmsg_head(xo);
}


static int
banmsg_delete(xo)
  XO *xo;
{
  if (vans(msg_del_ny) == 'y')
  {

    if (!rec_del(xo->dir, sizeof(BANMSG), xo->pos, NULL, NULL))
    {

      banmsg_cache();
      return banmsg_load(xo);
    }
  }
  return XO_FOOT;
}


static int
banmsg_change(xo)
  XO *xo;
{
  BANMSG *banmsg, mate;
  int pos, cur;
  
  pos = xo->pos;
  cur = pos - xo->top;
  banmsg = (BANMSG *) xo_pool + cur;

  mate = *banmsg;
  banmsg_edit(banmsg, GCARRY);
  if (memcmp(banmsg, &mate, sizeof(BANMSG)))
  {
    rec_put(xo->dir, banmsg, sizeof(BANMSG), pos);
    move(3 + cur, 0);
    banmsg_item(++pos, banmsg);
  }

  return XO_FOOT;
}


static int
banmsg_mail(xo)
  XO *xo;
{
  BANMSG *banmsg;
  char *userid;

  banmsg = (BANMSG *) xo_pool + (xo->pos - xo->top);
  userid = banmsg->userid;
  if (*userid)
  {
    vs_bar("±H  «H");
    prints("¦¬«H¤H¡G%s", userid);
    my_send(userid);
  }
  return banmsg_head(xo);
}


static int
banmsg_sort(xo)
  XO *xo;
{
  banmsg_sync(xo->dir);
  return banmsg_load(xo);
}


static int
banmsg_query(xo)
  XO *xo;
{
  BANMSG *banmsg;

  banmsg = (BANMSG *) xo_pool + (xo->pos - xo->top);
  move(1, 0);
  clrtobot();
  my_query(banmsg->userid, 1);
  return banmsg_head(xo);
}


static int
banmsg_help(xo)
  XO *xo;
{
//  film_out(FILM_BANMSG, -1);
  return banmsg_head(xo);
}


KeyFunc banmsg_cb[] =
{
  {XO_INIT, banmsg_init},
  {XO_LOAD, banmsg_load},
  {XO_HEAD, banmsg_head},
  {XO_BODY, banmsg_body},

  {'a', banmsg_add},
  {'c', banmsg_change},
  {'d', banmsg_delete},
  {'m', banmsg_mail},
  {'q', banmsg_query},
  {'s', banmsg_sort},
  {'h', banmsg_help}
};


int
t_banmsg()
{
  XO *xo;
  char fpath[64];

  usr_fpath(fpath, cuser.userid, FN_BANMSG);
  xz[XZ_OTHER - XO_ZONE].xo = xo = xo_new(fpath);
  xz[XZ_OTHER - XO_ZONE].cb = banmsg_cb;
  xover(XZ_OTHER);
  banmsg_cache();
  free(xo);

  return 0;
}
#endif
