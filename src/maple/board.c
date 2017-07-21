/*-------------------------------------------------------*/
/* board.c	( NTHU CS MapleBBS Ver 2.36 )		 */
/*-------------------------------------------------------*/
/* target : �ݪO�B�s�ե\��	 			 */
/* create : 95/03/29				 	 */
/* update : 95/12/15				 	 */
/*-------------------------------------------------------*/

#include "bbs.h"

extern BCACHE *bshm;
extern XZ xz[];


char brd_bits[MAXBOARD];
time_t brd_visit[MAXBOARD];	/* �̪��s���ɶ� */
static char *class_img;

#ifdef	HAVE_PROFESS
static char *profess_img = NULL;
#endif

#ifdef	HAVE_FAVORITE
static char *favorite_img = NULL;
#endif

static XO board_xo;
BRD *xbrd;
int boardmode=0;


/* ----------------------------------------------------- */
/* �ݪO�\Ū�O�� .BRH (Board Reading History)		 */
/* ----------------------------------------------------- */


typedef struct BoardReadingHistory
{
  time_t bstamp;		/* �إ߬ݪO���ɶ�, unique */ /* Thor.brh_tail*/
  time_t bvisit;		/* �W���\Ū�ɶ� */ /* Thor.980902:�S�Ψ�? */
                       /* Thor.980904:��Ū�ɩ�W��Ū���ɶ�, ��Ū�ɩ� bhno */
  int bcount;                                      /* Thor.980902:�S�Ψ�? */
                                                   /* Thor.980902:���ۤv�ݪ� */
  /* --------------------------------------------------- */
  /* time_t {final, begin} / {final | BRH_SIGN}		 */
  /* --------------------------------------------------- */
                           /* Thor.980904:����: BRH_SIGN�N��final begin �ۦP */
                           /* Thor.980904:����: �Ѥj��p�ƦC,�s��wŪinterval */
}                   BRH;


#define	BRH_EXPIRE	180          /* Thor.980902:����:�O�d�h�֤� */
#define BRH_MAX      	200          /* Thor.980902:����:�C���̦h���X�Ӽ��� */
#define BRH_PAGE	2048         /* Thor.980902:����:�C���h�t�q, �Τ���F */
#define	BRH_MASK	0x7fffffff   /* Thor.980902:����:�̤j�q��2038�~1�뤤*/
#define	BRH_SIGN	0x80000000   /* Thor.980902:����:zap����final�M�� */
#define	BRH_WINDOW	(sizeof(BRH) + sizeof(time_t) * BRH_MAX * 2)


static int *brh_base;		/* allocated memory */
static int *brh_tail;		/* allocated memory */
static int brh_size;		/* allocated memory size */
static time_t brh_expire;


static int *
brh_alloc(tail, size)
  int *tail;
  int size;
{
  int *base, n;

  base = brh_base;
  n = (char *) tail - (char *) base;
  size += n;
  if (size > brh_size)
  {
    /* size = (size & -BRH_PAGE) + BRH_PAGE; */
    size += n >> 4;		/* �h�w���@�ǰO���� */
    base = (int *) realloc((char *) base, size);

    if (base == NULL)
      abort_bbs();

    brh_base = base;
    brh_size = size;
    tail = (int *) ((char *) base + n);
  }

  return tail;
}


static void
brh_put()
{
  int *list;

  /* compact the history list */

  list = brh_tail;

  if (*list)
  {
    int *head, *tail, n, item, chrono;

    n = *++list;   /* Thor.980904: ��Ū�ɬObhno */
    brd_bits[n] |= BRD_H_BIT;
    time((time_t *) list);    /* Thor.980904: ����: bvisit time */

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
brh_get(bstamp, bhno)
  time_t bstamp;		/* board stamp */
  int bhno;
{
  int *head, *tail;
  int size, bcnt, item;
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
      size = item * sizeof(time_t) + sizeof(BRH);

      if (bstamp == *head)
      {
	bcnt = item;
	memcpy(buf, head + 3, size);
	tail = (int *) ((char *) tail - size);
	if ((item = (char *) tail - (char *) head))
	  memcpy(head, (char *) head + size, item);
	break;
      }
      head = (int *) ((char *) head + size);
    }
  }

  brh_tail = tail = brh_alloc(tail, BRH_WINDOW);

  *tail++ = bstamp;
  *tail++ = bhno;

  if (bcnt)			/* expand history list */
  {
    int *list;

    size = bcnt;
    list = tail;
    head = (int *) buf;

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

  *tail = bcnt;
}


int
brh_unread(chrono)
  time_t chrono;
{
  int *head, *tail, item;

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
brh_visit(mode)
  int mode;			/* 0 : visit, 1: un-visit */
{
  int *list;

  list = (int *) brh_tail + 2;
  *list++ = 2;
  if (mode)
  {
    *list = mode;
  }
  else
  {
    time((time_t *)list);
  }
  *++list = mode;
}


void
brh_add(prev, chrono, next)
  time_t prev, chrono, next;
{
  int *base, *head, *tail, item, final, begin;

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
	if (next < begin)	/* increase */
	  *head = chrono;
	else
	{			/* merge */
	  *base = item - 2;
	  base = head - 1;
	  do
	  {
	    *base++ = *++head;
	  } while (head < tail);
	}
	return;
      }

      if (next >= begin)
      {
	head[-1] = chrono;
	return;
      }

      break;
    }

    begin = *++head;
    head++;
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
    /* [32, 30] [22, 10] */  /* Thor.980923: how about [6, 7, 8] ? [15,7]? */

    tail--;
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
/* �j�� user login �ɭ�Ū���Y���i�ݪO			 */
/* ----------------------------------------------------- */

/* cdchen.bbs@bbs.ntcic.edu.tw add by statue.000725 */
#ifdef	HAVE_FORCE_BOARD
extern void
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
/* board permission check				 */
/* ----------------------------------------------------- */


static inline int
is_bm(list)
  char *list;			/* �O�D�GBM list */
{
  int cc, len;
  char *userid;

  len = strlen(userid = cuser.userid);
  do
  {
    cc = list[len];
    if ((!cc || cc == '/') && !str_ncmp(list, userid, len))
    {
      return 1;
    }
    while ((cc = *list++))
    {
      if (cc == '/')
	break;
    }
  } while (cc);

  return 0;
}

#if	defined(HAVE_RESIST_WATER) || defined(HAVE_DETECT_CROSSPOST)
void
remove_perm()
{
  int i;
  for(i=0;i<(sizeof(brd_bits)/sizeof(int));i++)
    brd_bits[i] &= ~BRD_W_BIT;
}
#endif

inline int
Ben_Perm(bhdr, ulevel)
  BRD *bhdr;
  usint ulevel;
{
  usint readlevel, postlevel, bits;
  char *blist, *bname;

  bname = bhdr->brdname;
  if (!*bname)
    return 0;

  if (!str_cmp(bname, DEFAULT_BOARD))
  {
#ifdef  HAVE_MODERATED_BOARD	
    extern int bm_belong();
#ifdef  HAVE_WATER_LIST
#ifdef	HAVE_SYSOP_WATERLIST
	if(bm_belong(bname) == BRD_R_BIT)
      return BRD_R_BIT;
	else
#endif
#endif
#endif
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

    extern int bm_belong();
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
  else if(bm_belong(bname) == BRD_R_BIT)
      bits &= ~BRD_W_BIT;
  
#endif
#endif

  /* Thor.980813: ����: �S�O�� BM �Ҷq, bm ���Ӫ����Ҧ��v�� */

  blist = bhdr->BM;

  if ((ulevel & PERM_BM) && blist[0] > ' ' && is_bm(blist))
    return (BRD_R_BIT | BRD_W_BIT | BRD_X_BIT);

#ifndef HAVE_BOARD_SEE    
  if (ulevel & PERM_ALLBOARD)
    bits |= (BRD_W_BIT | BRD_X_BIT);  
#endif
  if (!str_cmp(cuser.userid, ELDER))
    bits = BRD_R_BIT | BRD_W_BIT | BRD_X_BIT;

  return bits;
}


/* ----------------------------------------------------- */
/* ���J currboard �i��Y�z�]�w				 */
/* ----------------------------------------------------- */


int
bstamp2bno(stamp)
  time_t stamp;
{
  BRD *brd;
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
brh_load()
{
  BRD *brdp, *bend;
  usint ulevel;
  int n, cbno;
  char *bits;

  int size, *base;
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
  /* �N .BRH ���J memory				 */
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
  /* �h�O�d BRH_WINDOW ���B�@�Ŷ�			 */
  /* --------------------------------------------------- */

  /* brh_size = n = ((size + BRH_WINDOW) & -BRH_PAGE) + BRH_PAGE; */
  
  
  brh_size = n = size + BRH_WINDOW;
  brh_base = base = (int *) malloc(n);

  if (size && ((n = open(fpath, O_RDONLY)) >= 0))
  {
    int *head, *tail, *list, bstamp, bhno;

    size = read(n, base, size);
    close(n);


    /* compact reading history : remove dummy/expired record */

    head = base;
    tail = (int *) ((char *) base + size);
    bits = brd_bits;
    while (head < tail && head >= brh_base)
    {
      bstamp = *head;

      if (bstamp & BRH_SIGN)	/* zap */
      {
	bstamp ^= BRH_SIGN;

	bhno = bstamp2bno(bstamp);

	if (bhno >= 0)
	{
	  bits[bhno] |= BRD_Z_BIT;
	}
	head++;
	continue;
      }

      bhno = bstamp2bno(bstamp);

      list = head + 2;

      if(list > tail)
        break;

      n = *list;
      size = n + 3;

      /* �o�ӬݪO�s�b�B�S���Q zap ���B�i�H read */

      if (bhno >= 0 && (bits[bhno] & BRD_R_BIT))
      {
      
	bits[bhno] |= BRD_H_BIT;/* �w���\Ū�O�� */
	bstp[bhno] = head[1];	/* �W���\Ū�ɶ� */
	cbno = bhno;

	if (n > 0)
	{

#if 0
	  if (n > BRH_MAX)
	    n = BRH_MAX;
#endif

	  list += n;   /* Thor.980904: ����: �̫�@��tag */
	  
	  if(list > tail)
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

	n = n * sizeof(time_t) + sizeof(BRH);
	if (base != head)
	  memcpy(base, head, n);
	base = (int *) ((char *) base + n);
      }
      head += size;
    }
  }

  *base = 0;
  brh_tail = base;

  /* --------------------------------------------------- */
  /* �]�w default board					 */
  /* --------------------------------------------------- */

  strcpy(currboard, "�|����w");
#ifdef HAVE_BOARD_PAL  
  cutmp->board_pal = brd_bno(currboard);
#endif
#ifdef	HAVE_RESIST_WATER
  if(checkqt > CHECK_QUOT_MAX)
    remove_perm();
#endif
}


void
brh_save()
{
  int *base, *head, *tail, bhno, size;
  BRD *bhdr, *bend;
  char *bits;

  /* Thor.980830: lkchu patch:  �٨S load �N���� save */ 
  if(!(base = brh_base))
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
    size = head[2] * sizeof(time_t) + sizeof(BRH);
    if (bhno >= 0 && !(bits[bhno] & BRD_Z_BIT))
    {
      if (base != head)
	memcpy(base, head, size);
      base = (int *) ((char *) base + size);
    }
    head = (int *) ((char *) head + size);
  }

  /* save zap record */

  tail = brh_alloc(base, sizeof(time_t) * MAXBOARD);

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
brd_usies_BMlog()
{
  char fpath[64], buf[256];

  brd_fpath(fpath, currboard, "usies");
  sprintf(buf, "%s %s\n", Now(), cuser.userid);
  f_cat(fpath, buf);
}

/* 081206.cache: �n�ͪO�ץ� */

int ok=1;

void
XoPost(bno)
  int bno;
{
  XO *xo;
  BRD *brd;
  int bits;
  char *str, fpath[64];
  static int LastBno = -1;

  /* 090823.cache: �ݪO�H�� */
  if (LastBno != bno)
  {
    if (LastBno >= 0)
      {
        if (bshm->mantime[LastBno] > 0 )//����H���ܦ��t�� 
          bshm->mantime[LastBno]--; /* �h�X�W�@�ӪO */
        else
          bshm->mantime[LastBno] = 0;//�t�ƪ����k�s 
      }

    bshm->mantime[bno]++;       /* �i�J�s���O */

    LastBno = bno;
  }

  brd = bshm->bcache + bno;
  strcpy(currboard, brd->brdname);
#ifdef HAVE_BOARD_PAL
  cutmp->board_pal = bno;
#endif
  currbno = bno;
  brh_get(brd->bstamp, bno);

  bbstate = /* (bbstate & STAT_DIRTY) | */ STAT_STARTED | brd->battr;
  bbstate &= ~STAT_POST;

  str = &brd_bits[bno];
  bits = *str;

/* cache.081206: ��n�ͬݪ����s�P�_�O���O�]�㦳 R �v�� */
  if (!(bits & BRD_R_BIT)/* && (bits & BRD_F_BIT)*/)
    {
      vmsg("���p���O�D�N�z�[�J�ݪO�n��");
      ok=0;
    }
  else
  {
  if (bits & BRD_X_BIT)
    bbstate |= (STAT_BOARD | STAT_POST);
  else if (bits & BRD_W_BIT)
    bbstate |= STAT_POST;

#ifdef  HAVE_MODERATED_BOARD
  if (brd->readlevel == PERM_SYSOP)
    bbstate |= STAT_MODERATED;
#endif

  if (/*!(bits & BRD_V_BIT) && */(cuser.ufo2 & UFO2_BNOTE))
  {
    *str = bits | BRD_V_BIT;
    brd_fpath(fpath, currboard, FN_NOTE);
    more(fpath, NULL);
  }

  brd_fpath(fpath, currboard, fn_dir);
  xz[XZ_POST - XO_ZONE].xo = xo = xo_get(fpath);
  xo->key = XZ_POST;
  xo->xyz = brd->bvote > 0 ? "���ݪO�i��벼��" : brd->title + 3;
  str = brd->BM;
  if (*str <= ' ')
    str = "�x�D��";
  sprintf(currBM, "�O�D�G%s", str);

#ifdef LOG_BRD_USIES
  /* lkchu.981201: �\Ū�ݪ��O�� */
  if(!(bbstate & BRD_NOLOGREAD))
    brd_usies();
#endif

  if (!(brd->battr & BRD_ANONYMOUS))
    brd_usies_BMlog();

#ifdef	HAVE_COUNT_BOARD
//  if(!(strcmp(brd->brdname,"Test")))
  if(!(bbstate & BRD_NOTOTAL) && !(bits & BRD_V_BIT))
    brd->n_reads++;
#endif
  ok=1;
  }
  
}


/* ----------------------------------------------------- */
/* Class [�����s��]					 */
/* ----------------------------------------------------- */

/* cache.090503: �Y�ɼ����ݪO */
static int
mantime_cmp(a, b)
  short *a;
  short *b;
{
  /* �Ѧh�ƨ�� */
  return bshm->mantime[*b] - bshm->mantime[*a];
}

static int class_hot = 0;
static int class_flag2 = 0;  /* 1:�C�X�n��/���K�O�A�B�ۤv�S���\Ū�v���� */
static int class_flag;


#define	BFO_YANK	0x01


static int
class_check(chn)
  int chn;
{
  short *cbase, *chead, *ctail;
  int pos, max, val, zap;
  int bnum = 0; 
  BRD *brd;
  char *bits;

  if (!class_img)
    return 0;          

  chn = CH_END - chn;
  switch(boardmode)
  {
    case 0:
      cbase = (short *) class_img; 
      break;
#ifdef	HAVE_PROFESS      
    case 1:
      cbase = (short *) profess_img;
      break;
#endif      
#ifdef  HAVE_FAVORITE      
    case 2:
      cbase = (short *) favorite_img;
      break;
#endif     
    default:
      cbase = (short *) class_img; 
  }

  chead = cbase + chn;

  pos = chead[0] + CH_TTLEN;
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
        (brd[chn].readlevel != PERM_BOARD &&
        brd[chn].readlevel != PERM_SYSOP )))
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
    else if(!class_check(chn))
       continue;
    max++;
  } while (chead < ctail);

  if (bnum > 0)
    qsort(cbase - bnum, bnum, sizeof(short), mantime_cmp);

  return max;
}

static int
class_load(xo)
  XO *xo;
{
  short *cbase, *chead, *ctail;
  int chn;			/* ClassHeader number */
  int pos, max, val, zap;
  int bnum = 0;
  BRD *brd;
  char *bits;

  chn = CH_END - xo->key;

  switch(boardmode)
  {
    case 0:
      cbase = (short *) class_img;
      break;
#ifdef	HAVE_PROFESS      
    case 1:
      cbase = (short *) profess_img;
      break;
#endif      
#ifdef  HAVE_FAVORITE
    case 2:
      cbase = (short *) favorite_img;
      break;
#endif
    default:
      cbase = (short *) class_img;
  }

  chead = cbase + chn;

  pos = chead[0] + CH_TTLEN;
  max = chead[1];

  chead = (short *) ((char *) cbase + pos);
  ctail = (short *) ((char *) cbase + max);

  max -= pos;

  if ((cbase = (short *) xo->xyz))
    cbase = (short *) realloc(cbase, max);
  else
    cbase = (short *) malloc(max);
  xo->xyz = (char *) cbase;

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
        (brd[chn].readlevel != PERM_BOARD &&
        brd[chn].readlevel != PERM_SYSOP )))
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
    else if(!class_check(chn))
    {
      continue;
    }

    max++;
    *cbase++ = chn;
  } while (chead < ctail);

  if (bnum > 0)
    qsort(cbase - bnum, bnum, sizeof(short), mantime_cmp);

  xo->max = max;
  if (xo->pos >= max)
    xo->pos = xo->top = 0;

  return max;
}


static int
XoClass(chn)
  int chn;
{
  XO xo, *xt;

  /* Thor.980727: �ѨM XO xo�����T�w��, 
                  class_load�����| initial xo.max, ��L���T�w */
  xo.pos = xo.top = 0;

  xo.key = chn;
  xo.xyz = NULL;
  if (!class_load(&xo))
  {
    free(xo.xyz);
    return XO_NONE;
  }

  xt = xz[XZ_CLASS - XO_ZONE].xo;
  xz[XZ_CLASS - XO_ZONE].xo = &xo;
  xover(XZ_CLASS);
  free(xo.xyz);
  xz[XZ_CLASS - XO_ZONE].xo = xt;

  return XO_BODY;
}


#if 0
/* ----------------------------------------------------- */
/* �ˬd BRD �@���X�g post�A�γ̫�@�g post ���ɶ�	 */
/* ----------------------------------------------------- */


void
bcheck(brd)
  BRD *brd;
{
  char folder[64];
  struct stat st;
  int fd, size;

  brd_fpath(folder, brd->brdname, fn_dir);

#if 1
  if (!stat(folder, &st))
  {
    brd->bpost = st.st_size / sizeof(HDR);
    brd->blast = st.st_mtime;
  }
#else
  fd = open(folder, O_RDONLY);
  if (fd < 0)
    return;

  fstat(fd, &st);

  if (st.st_mtime > brd->btime)
  {
    brd->btime = time(0) + 45;	/* 45 �������������ˬd */
    if ((size = st.st_size) >= sizeof(HDR))
    {
      brd->bpost = size / sizeof(HDR);
      lseek(fd, size - sizeof(HDR), SEEK_SET);
      read(fd, &brd->blast, sizeof(time_t));
    }
    else
    {
      brd->blast = brd->bpost = 0;
    }
  }

  close(fd);
#endif
}
#endif


static int
class_body(xo)
  XO *xo;
{
  char *img, *bits, buf[16], buf2[20], brdtype, *str2;
  short *chp;
  BRD *bcache;
  int n, cnt, max, brdnew, bno;


  switch(boardmode)
  {
    case 0:
      img = class_img;
      break;
#ifdef	HAVE_PROFESS      
    case 1:
      img = profess_img;
      break;
#endif      
#ifdef  HAVE_FAVORITE       
    case 2:
      img = favorite_img;
      break;
#endif
    default:
      img = class_img;
  }

  bcache = bshm->bcache;
  brdnew = class_flag & UFO2_BRDNEW;
  bits = brd_bits;

  max = xo->max;
  if (max <= 0)
    return XO_QUIT;
  
  
  cnt = xo->top;
  chp = (short *) xo->xyz + cnt;
  n = 3;
  do
  {
    move(n, 0);
    if (cnt < max)
    {
      int chn;
      char *str;

      cnt++;
      chn = *chp++;
      if (chn >= 0)
      {
	BRD *brd;
	int num;

	brd = bcache + chn;

	if (!brdnew)
	{
	  int fd, fsize;
	  char folder[64];
	  struct stat st;

	  brd_fpath(folder, brd->brdname, fn_dir);
	  if ((fd = open(folder, O_RDONLY)) >= 0)
	  {
	    fstat(fd, &st);

          if(st.st_mtime > brd->btime)  // �W���έp��A�ɮצ��ק�L
          {
            if((fsize = st.st_size) >= sizeof(HDR))
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
              brd->blast = hdr.chrono;
            }
           else
              brd->blast = brd->bpost = 0;
          }

	    close(fd);
	  }

	  num = cnt;                       
	}
	else
	{
	  int fd, fsize;
	  char folder[64];
	  struct stat st;

	  brd_fpath(folder, brd->brdname, fn_dir);
	  if ((fd = open(folder, O_RDONLY)) >= 0)
	  {
	    fstat(fd, &st);

          if(st.st_mtime > brd->btime)  // �W���έp��A�ɮצ��ק�L
          {
            if((fsize = st.st_size) >= sizeof(HDR))
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
              brd->blast = (hdr.chrono > brd->blast) ? hdr.chrono : brd->blast;
            }
           else
              brd->blast = brd->bpost = 0;
          }

	    close(fd);
	  }

	  num = brd->bpost;   
	}

    str = brd->blast > brd_visit[chn] ? "\033[1;31m��\033[m" : "��";  

	char tmp[BTLEN + 1];

	strcpy(tmp, brd->title);
	if (tmp[31] & 0x80)
//	  tmp[33] = '\0';
//	else
	  tmp[31] = ' ';
	  tmp[32] = '\0';

/* 081122.cache:�ݪO�ʽ�,���q�\,���K,�n��,�@�� */
      if(bits[chn] & BRD_Z_BIT)
        brdtype = '-';
#ifdef HAVE_MODERATED_BOARD
      else if(brd->readlevel & PERM_BOARD)
        brdtype = '.';     
      else if(brd->readlevel & PERM_SYSOP)
        brdtype = ')';
#endif
      else
        brdtype = ' ';
 
/* �B�z �H�� */ /* cache.20090416: ��ptt�ܦ�*/
      bno = brd - bshm->bcache;
      bno = bshm->mantime[bno];
      if (brd->bvote)
        str2 = "\033[1;33m  ��\033[m ";      
      else if (bno > 999)
        str2 = "\033[1;32m  �q\033[m ";
      else if (bno > 799)
        str2 = "\033[1;35m  �q\033[m ";
      else if (bno > 699)
        str2 = "\033[1;33m  �q\033[m ";
      else if (bno > 599)
        str2 = "  \033[1;42m�z\033[m ";
      else if (bno > 499)
        str2 = "  \033[1;45m�z\033[m ";
      else if (bno > 449)
        str2 = "  \033[1;44m�z\033[m ";
      else if (bno > 399)
        str2 = "\033[1;32m  �z\033[m ";
      else if (bno > 349)
        str2 = "\033[1;35m  �z\033[m ";
      else if (bno > 299)
        str2 = "\033[1;33m  �z\033[m ";
      else if (bno > 249)
        str2 = "\033[1;36m  �z\033[m ";
      else if (bno > 199)
        str2 = "\033[1;31m  �z\033[m ";
      else if (bno > 149)
        str2 = "\033[1;37m  �z\033[m ";
      else if (bno > 99)
        str2 = "\033[1;31m HOT\033[m ";
      else if (bno > 49)
        str2 = "\033[1;37m HOT\033[m ";
      else if (bno > 5)
        sprintf(str2 = buf2, "  %2d ", bno);
      else
        str2 = "     ";
//�`�N���T��ť�, �]�� HOT �O�T�� char �G���ƪ�
//	  prints("\033[%d;4%d;37m%6d%s%s%c%-13s\033[%sm%-4s %s%-33.32s%s%s%.13s",mode,mode?cuser.barcolor:0, num, str, mode ? "\033[37m" : "\033[m",
//	  brdtype,brd->brdname, buf, brd->class, mode ? "\033[37m" : "\033[m", brd->title,brd->bvote ? "[1;33m  �� " : str2, mode ? "\033[37m" : "\033[m", brd->BM);

        sprintf(buf,"%d;3%d",brd->color/10,brd->color%10);
//	prints("%6d%s%c%-13s\033[%sm%-4s \033[m%-36s%c %.13s", num, str,
//	prints("%6d%s%c%-13s\033[%sm%-4s \033[m%s%c %.13s", num, str,

	  prints("%6d%s%c%-13s\033[%sm%-4s \033[m%-32s %s", num, str, brdtype, brd->brdname, buf, brd->class, tmp, str2);
    
        strcpy(tmp, brd->BM);
        if (tmp[13] & 0x80)
          tmp[12] = '\0';
        else
          tmp[13] = '\0';
	  
	prints("%-13s", tmp);

      }
      else
      {
	short *chx;

	chx = (short *) img + (CH_END - chn);
	prints("%6d   %s", cnt, img + *chx);
      }
    }
    clrtoeol();
  } while (++n < b_lines);

  return XO_NONE;
}


static int
class_neck(xo)
  XO *xo;
{
  move(1, 0);
  prints(NECKBOARD, class_flag & UFO2_BRDNEW ? "�`��" : "�s��", "��   ��   ��   �z");

  return class_body(xo);
}


static int
class_head(xo)
  XO *xo;
{
  vs_head("�ݪO�C��", str_site);
  return class_neck(xo);
}


static int
class_init(xo)			/* re-init */
  XO *xo;
{
  class_load(xo);
  return class_head(xo);
}


static int
class_newmode(xo)
  XO *xo;
{
  cuser.ufo2 ^= UFO2_BRDNEW;  /* Thor.980805: �S�O�`�N utmp.ufo���P�B���D */
  class_flag ^= UFO2_BRDNEW;
  return class_neck(xo);
}


static int
class_help(xo)
  XO *xo;
{
  film_out(FILM_CLASS, -1);
  return class_head(xo);
}


static int
class_search(xo)
  XO *xo;
{
  int num, pos, max;
  char *ptr;
  char buf[IDLEN + 1];

  ptr = buf;
  pos = vget(b_lines, 0, "�п�J�ݪO�W�١G", ptr, IDLEN + 1, DOECHO);
  move(b_lines, 0);
  clrtoeol();

  if (pos)
  {
    short *chp, chn;
    BRD *bcache, *brd;

    bcache = bshm->bcache;

    str_lower(ptr, ptr);
    pos = num = xo->pos;
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
	if (str_str(brd->brdname, ptr) || str_str(brd->title, ptr))
	  return pos + XO_MOVE;
      }
    } while (pos != num);
  }

  return XO_NONE;
}

/* cache.091125: �u�C�X�㦳�\Ū�v�������K/�n�ͬݪO */
static int
class_yank2(xo)
  XO *xo;
{
  if (xo->key >= 0)
    return XO_NONE;

  class_flag2 ^= 0x01;
  return class_init(xo);
}

static int
class_yank(xo)
  XO *xo;
{
  class_flag ^= BFO_YANK;
  return class_init(xo);
}


static int
class_zap(xo)
  XO *xo;
{
  BRD *brd;
  short *chp;
  int num, chn;

  num = xo->pos;
  chp = (short *) xo->xyz + num;
  chn = *chp;
  if (chn >= 0)
  {
    brd = bshm->bcache + chn;
    if (!(brd->battr & BRD_NOZAP) || ( brd_bits[chn] & BRD_Z_BIT))
    {
      move(3 + num - xo->top, 8);
      num = brd_bits[chn] ^= BRD_Z_BIT;
      outc(num & BRD_Z_BIT ? '-' : ' ');
    }
  }

  return XO_NONE;
}


static int
class_edit(xo)
  XO *xo;
{
  /* if (HAS_PERM(PERM_ALLBOARD)) */
  /* Thor.990119: �u�������i�H�ק� */
  if (HAS_PERM(PERM_SYSOP) || HAS_PERM(PERM_BOARD))
  {
    short *chp;
    int chn;

    chp = (short *) xo->xyz + xo->pos;
    chn = *chp;
    if (chn >= 0)
    {
      brd_edit(chn);
      return class_init(xo);
    }
  }
  return XO_NONE;
}


static int
class_browse(xo)
  XO *xo;
{
  short *chp;
  int chn;

  chp = (short *) xo->xyz + xo->pos;
  chn = *chp;
  if (chn < 0)
  {
   short *chx;
   char *img, *str;

   img = class_img;
   chx = (short *) img + (CH_END - chn);
   str = img + *chx;

   // "HOT/" �W�٥i�۩w�A�Y��W�]�n���K��᭱������ 4
//   if (!strncmp(str, "HOT/", 4))
//   {
//     class_hot = 1;
//     chn = CH_END;
//   }

    if (XoClass(chn) == XO_NONE)
      return XO_NONE;
      
//    if (class_hot)
//#      class_hot = 0;      /* ���} HOT Class �A�M�� class_hot �аO */
      
  }
  else
  {
    XoPost(chn);
    if(ok==1)
    {
    xover(XZ_POST);
    time(&brd_visit[chn]);
    }
  }

  return class_head(xo);	/* Thor.0701: �L�k�M�֤@�I, �]�� XoPost */
}


int
Select()
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
class_switch(xo)
  XO *xo;
{
  Select();
  return class_head(xo);
}


#ifdef AUTHOR_EXTRACTION
/* Thor.0818: �Q�令�H�ثe���ݪ��C��Τ����ӧ�, ���n����� */


/* opus.1127 : �p�e���g, �i extract author/title */


static int
XoAuthor(xo)
  XO *xo;
{
  int chn, len, max, tag;
  short *chp, *chead, *ctail;
  BRD *brd;
  char author[IDLEN + 1];
  XO xo_a, *xoTmp;
#ifndef HAVE_MMAP  
  XO *xo_t;
#endif
  if (!HAS_PERM(PERM_VALID))
    return XO_NONE;

  if (!vget(b_lines, 0, "�п�J�@�̡G", author, IDLEN + 1, DOECHO))
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
    if ((chn = *chead++) >= 0)	/* Thor.0818: ���� group */
    {
      /* Thor.0701: �M����w�@�̤峹, ���h����m, �é�J */

#ifdef HAVE_MMAP
      int fsize;
      char *fimage;
#endif       
      char folder[80];
      HDR *head, *tail;

      sprintf(folder, "�m�M����w�@�̡n�ݪ��G%s \033[5m...\033[m",
	brd[chn].brdname);
      outz(folder);
      refresh();
      brd_fpath(folder, brd[chn].brdname, fn_dir);

#ifdef HAVE_MMAP
      fimage = f_map(folder, &fsize);

      if (fimage == (char *) -1)
	continue;

      head = (HDR *) fimage;
      tail = (HDR *) (fimage + fsize);

      while (head <= --tail)
      {
	if (tail->xmode & (POST_CANCEL | POST_DELETE | POST_MDELETE | POST_LOCK))
	  continue;

	/* if(str_str(temp,author)) *//* Thor.0818:�Ʊ����� */

	if (!str_ncmp(tail->owner, author, len))
	{
	  xo_get(folder)->pos = tail - head;
	  chp[tag++] = chn;
	  break;
	}
      }

      munmap(fimage, fsize);

#else
      xo_t = xo_new(folder);
      xo_t->pos = XO_TAIL;	/* �Ĥ@���i�J�ɡA�N��Щ�b�̫᭱ */
      xo_load(xo_t, sizeof(HDR));
      if (xo_t->max <= 0)
	continue;
      head = (HDR *) xo_pool;
      tail = (HDR *) xo_pool + (xo_t->pos - xo_t->top);
      for (;;)
      {
	if (!(tail->xmode & (POST_CANCEL | POST_DELETE | POST_MDELETE | POST_LOCK)))
	{
	  /* check condition */
	  if (!str_ncmp(tail->owner, author, len))	/* Thor.0818:�Ʊ����� */
	  {
	    xo_get(folder)->pos = tail - head;
	    chp[tag++] = chn;
	    break;
	  }
	}
	tail--;
	if (tail < head)
	{
	  if (xo_t->top <= 0)
	    break;
	  xo_t->pos -= XO_TALL;
	  xo_load(xo_t, sizeof(HDR));
	  tail = (HDR *) xo_pool + XO_TALL - 1;
	}
      }

      free(xo_t);
#endif

    }
  } while (chead < ctail);

  if (!tag)
  {
    free(chp);
    vmsg("�ŵL�@��");
    return XO_FOOT;
  }

  xo_a.pos = xo_a.top = 0;
  xo_a.max = tag;
  xo_a.key = 1;			/* all boards */
  xo_a.xyz = (char *) chp;

  xoTmp = xz[XZ_CLASS - XO_ZONE].xo;	/* Thor.0701: �O�U��Ӫ�class_xo */

  xz[XZ_CLASS - XO_ZONE].xo = &xo_a;
  xover(XZ_CLASS);

  free(chp);

  xz[XZ_CLASS - XO_ZONE].xo = xoTmp;		/* Thor.0701: �٭� class_xo */

  return class_body(xo);
}
#endif

#ifdef  HAVE_FAVORITE
static int
class_find_same(src)
  HDR *src;
{
  char fpath[128];
  int fd,i;
  HDR hdr;
  
  i = 0;
  usr_fpath(fpath,cuser.userid,FN_FAVORITE);
  if ((fd = open(fpath,O_RDONLY)))
  {
    while ((read(fd,&hdr, sizeof(HDR)) == sizeof(HDR)))
    {
      if(!str_cmp(hdr.xname,src->xname))
      {
        close(fd);
        return i;
      }
      i++;
    }
    close(fd);
  }
  return -1;
}

#if 0
static int
class_add(xo)
  XO *xo;
{
  short *chp;
  BRD *brd;
  HDR hdr;
  int chn,fasize;
  char fpath[128];
  if(boardmode == 2 || !HAS_PERM(PERM_VALID))
    return XO_NONE;    

  usr_fpath(fpath,cuser.userid,FN_FAVORITE);
  chp = (short *) xo->xyz + xo->pos;
  chn = *chp;
  if(chn < 0)
  {
    return XO_NONE;
  }  
  brd = bshm->bcache + chn;
  memset(&hdr,0,sizeof(HDR));
  brd2gem(brd,&hdr);
  if(class_find_same(&hdr) < 0 )
  {
    rec_add(fpath,&hdr,sizeof(HDR));
    favorite_main();
    usr_fpath(fpath,cuser.userid,FN_FAVORITE_IMG);
    if(favorite_img)
      free(favorite_img);
    favorite_img = f_img(fpath,&fasize);
    logitfile(FN_FAVORITE_LOG,"< ADD >",hdr.xname);
    vmsg("�w���\\�[�J�ڪ��̷R�I");
  }
  else
  {
    vmsg("�ڪ��̷R�w�����ݪO�I");
  }

  return XO_FOOT;
}
#endif

static int
class_add2(xo)          /* gaod: �ڪ��̷R�������s�W�s�ݪO */
  XO *xo;
{
  short *chp;
  BRD *brd;
  HDR hdr;
  int chn, fasize;
  char fpath[128];
  char bname[IDLEN + 1];

  if(boardmode != 2 || !HAS_PERM(PERM_VALID))
    return XO_NONE;

  if(!(brd = ask_board(bname, BRD_R_BIT, NULL)))
  {
    vmsg(err_bid);
    return XO_HEAD;
  }

  if (!(brd->brdname[0]))
  {
    vmsg(err_bid);
    return XO_HEAD;
  }

  usr_fpath(fpath, cuser.userid, FN_FAVORITE);
  chp = (short *) xo->xyz + xo->pos;
  chn = *chp;
  memset(&hdr,0,sizeof(HDR));
  brd2gem(brd,&hdr);
  if(class_find_same(&hdr) < 0 )
  {
    rec_add(fpath,&hdr,sizeof(HDR));
    favorite_main();
    usr_fpath(fpath,cuser.userid,FN_FAVORITE_IMG);
    if(favorite_img)
      free(favorite_img);
    favorite_img = f_img(fpath,&fasize);
    logitfile(FN_FAVORITE_LOG,"< ADD >",hdr.xname);
    vmsg("�w���\\�[�J�ڪ��̷R�I");
  }
  else
  {
    vmsg("�ڪ��̷R�w�����ݪO�I");
  }
  return class_init(xo);
}

static int
class_del(xo)
  XO *xo;
{
  short *chp;
  BRD *brd;
  HDR hdr;
  int chn,fasize,pos;
  char fpath[128];
  if(boardmode != 2 || !HAS_PERM(PERM_VALID))
    return XO_NONE;

  usr_fpath(fpath,cuser.userid,FN_FAVORITE);
  chp = (short *) xo->xyz + xo->pos;
  chn = *chp;
  brd = bshm->bcache + chn;
  memset(&hdr,0,sizeof(HDR));
  brd2gem(brd,&hdr);
  if((pos = class_find_same(&hdr)) >= 0)
  {
    rec_del(fpath, sizeof(HDR), pos, NULL, NULL);
    favorite_main();
    usr_fpath(fpath,cuser.userid,FN_FAVORITE_IMG);
    if(favorite_img)
      free(favorite_img);
    favorite_img = f_img(fpath,&fasize);
    
    logitfile(FN_FAVORITE_LOG,"< DEL >",hdr.xname);
    vmsg("�w���\\�q�ڪ��̷R�����I");
    if(!favorite_img)
      return XO_QUIT;
  }
  else
  {
    vmsg("�ڪ��̷R�L���ݪO�I");
    return XO_FOOT;
  }

  return class_init(xo);
}

static int
class_mov(xo)
  XO *xo;
{
  short *chp;
  BRD *brd;
  HDR hdr;
  int chn,fasize,pos,newOrder;
  char fpath[128],buf[128];
  if(boardmode != 2 || !HAS_PERM(PERM_VALID))
    return XO_NONE;

  usr_fpath(fpath,cuser.userid,FN_FAVORITE);
  chp = (short *) xo->xyz + xo->pos;
  chn = *chp;
  brd = bshm->bcache + chn;
  memset(&hdr,0,sizeof(HDR));
  brd2gem(brd,&hdr);

  pos = xo->pos;
  sprintf(buf + 5, "�п�J�� %d �ﶵ���s��m�G", pos + 1);
  if (!vget(b_lines, 0, buf + 5, buf, 5, DOECHO))
    return XO_FOOT;

  newOrder = atoi(buf) - 1;
  if (newOrder < 0)
    newOrder = 0;
  else if (newOrder >= xo->max)
    newOrder = xo->max - 1;

  if((pos = class_find_same(&hdr)) >= 0)
  {
    if(newOrder != pos)
    {
      if (!rec_del(fpath, sizeof(HDR), pos, NULL, NULL))
      {
        rec_ins(fpath, &hdr, sizeof(HDR), newOrder, 1);
        xo->pos = newOrder;
      }


      favorite_main();
      usr_fpath(fpath,cuser.userid,FN_FAVORITE_IMG);
      favorite_img = f_img(fpath,&fasize);

      logitfile(FN_FAVORITE_LOG,"< MOV >",hdr.xname);
    }
    else
      return XO_FOOT;
  }
  else
  {
    vmsg("�ڪ��̷R�L���ݪO�I");
    return XO_FOOT;
  }

  return class_init(xo);
}
   
#endif


#ifdef  HAVE_COUNT_BOARD
#if 0
static int
class_stat(xo)
  XO *xo;
{
  BRD *brd;
  short *chp;
  int num, chn;
  char msg[80];

  num = xo->pos;
  chp = (short *) xo->xyz + num;
  chn = *chp;
  if (chn >= 0)
  {
    brd = bshm->bcache + chn;
    sprintf(msg,"�ثe�ֿn�\\Ū�ơG%d�A�ֿn�o��ơG%d",brd->n_reads,brd->n_posts);
    pmsg(msg);
  }

  return XO_NONE;
}
#endif
#endif

static int
class_visit(xo)
  XO *xo;
{
  short *chp;
  int chn;

  chp = (short *) xo->xyz + xo->pos;
  chn = *chp;
  if (chn >= 0)
  {
    BRD *brd;
    brd = bshm->bcache + chn;
    brh_get(brd->bstamp, chn);
    brh_visit(0);
    time(&brd_visit[chn]);
  }
  return class_body(xo);
}

static KeyFunc class_cb[] =
{
  {XO_INIT, class_head},
  {XO_LOAD, class_body},
  {XO_HEAD, class_head},
  {XO_BODY, class_body},

  {'r', class_browse},
  {'/', class_search},
  {'c', class_newmode},

  {'s', class_switch},

  {'y', class_yank},
  {'i', class_yank2}, //�C�X�Ҧ����\Ū�v�������K/�n���ݪO 
  {'z', class_zap},
  {'E', class_edit},
  {'v', class_visit},
#ifdef  HAVE_COUNT_BOARD
  {'S' | XO_DL, (int (*)())"bin/brdstat.so:main_bstat"},
#endif

#ifdef  HAVE_FAVORITE
  {'a', class_add},
  {Ctrl('P'), class_add2},
  {'d', class_del},
  {'M', class_mov},
#endif

#ifdef AUTHOR_EXTRACTION
  //{'A', XoAuthor},
#endif

  {'h', class_help}
};


int
Class()
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
check_new(brd)
  BRD *brd;
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
                read(fd, &brd->blast, sizeof(time_t));
              }
              else
              {
                brd->blast = brd->bpost = 0;
              }
            }
            close(fd);
          }
}

#ifdef	HAVE_INFO
int 
Information()
{
  int chn;
  chn = brd_bno(BRD_BULLETIN);
  XoPost(chn);
  xover(XZ_POST);
  time(&brd_visit[chn]);
  return 0;
}
#endif

#ifdef	HAVE_STUDENT
int 
Student()
{
  int chn;
  chn = brd_bno(BRD_SBULLETIN);
  XoPost(chn);
  xover(XZ_POST);
  time(&brd_visit[chn]);
  return 0;
}
#endif

#ifdef	HAVE_ACTIVITY
int
Activity()
{
  int chn;
  chn = brd_bno(BRD_ABULLETIN);
  XoPost(chn);
  xover(XZ_POST);
  time(&brd_visit[chn]);
  return 0;
}
#endif

#ifdef	HAVE_PROFESS
int
Profess()
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

#ifdef  HAVE_FAVORITE 
int
Favorite()
{
  boardmode = 2;
  if (!favorite_img || XoClass(CH_END-1) == XO_NONE)
  {
    vmsg("�ڪ��̷R�|�L����ݪO�I�ЦܬݪO�C��e�� a �s�W�C");
    return XEASY;
  }
  return 0;
}
#endif

void
board_main()
{
  int fsize;
#ifdef	HAVE_PROFESS  
  int psize;
#endif  
  struct stat st;
#ifdef  HAVE_FAVORITE
  int fasize;
  char fpath[128];
#endif
//  brh_load();


#ifdef	HAVE_RECOMMEND
  recommand_time = time(0);
#endif

  /* �p�G�S�� .BRD �ɡA�Τj�p���s�ɡA�N run/class.img ����
     �o�ˤ~���|�i���N���I visor.000705 */
  if (stat(FN_BRD, &st) || st.st_size <= 0)
    unlink(CLASS_IMGFILE);

  class_img = f_img(CLASS_IMGFILE, &fsize);
#ifdef	HAVE_PROFESS
  profess_img = f_img(PROFESS_IMGFILE, &psize);
#endif

#ifdef  HAVE_FAVORITE
  if(HAS_PERM(PERM_VALID))
  {
    usr_fpath(fpath,cuser.userid,FN_FAVORITE_IMG);
    favorite_img = f_img(fpath,&fasize);
  }
#endif
  
  if (class_img == NULL)
  {
    blog("CACHE", CLASS_IMGFILE);
  }
#ifdef	HAVE_PROFESS  
  if (profess_img == NULL)                                                        
  {
    blog("CACHE", PROFESS_IMGFILE);
  }
#endif  

  if (!cuser.userlevel)		/* guest yank all boards */
  {
    class_flag = BFO_YANK;
  }
  else
  {

#if 0
    class_flag = 0;		/* to speed up */
#endif

    class_flag = cuser.ufo2 & UFO2_BRDNEW;
  }

  board_xo.key = CH_END;
  if(class_img)
    class_load(&board_xo);

  xz[XZ_CLASS - XO_ZONE].xo = &board_xo;	/* Thor: default class_xo */
  xz[XZ_CLASS - XO_ZONE].cb = class_cb;		/* Thor: default class_xo */
}


int
Boards()
{
  /* class_xo = &board_xo; *//* Thor: �w�� default, ���ݧ@�� */
  boardmode = -1;
  xover(XZ_CLASS);
  return 0;
}

#ifdef HAVE_MULTI_CROSSPOST
#define MSG_CC "\033[32m[�ݪO�s�զW��]\033[m\n"

int
brd_list(reciper)
  int reciper;
{
  LIST list;
  int userno, fd ,select;
  char buf[32], fpath[64],msg[128],temp[30];
  BRD brd,*ptr;
  sprintf(msg,"A)�W�[ D)�R�� B)�����ݪ� C)���� G)���� [1~%d]�s�զW�� M)�w�� Q)�����H[M]",MAX_LIST);

  userno = 0;

  for (;;)
  {
    select = vget(1, 0, msg , buf, 2, LCECHO);
    switch (select)
    {
#if 1
    case '1': case '2': case '3': case '4': case '5': case '6': case '7':
    case '8': case '9':
      sprintf(temp,"board.%c",*buf);
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
#endif
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
      if ((userno = vget(b_lines, 0, "�s�ձ���G", buf, 16, DOECHO)))
        str_lower(buf, buf);
    case 'c':
       if (!userno && vget(b_lines, 0, "�����G", buf, 16, DOECHO))
         str_lower(buf, buf);
    case 'b':
      if ((fd = open(FN_BRD, O_RDONLY)) >= 0)
      {
        char *name;

        while (read(fd,&brd, sizeof(BRD)) == sizeof(BRD))
        {
          name = brd.brdname;
          if (!ll_has(name) && (
              (select == 'b') || 
              (select == 'g' && (str_str(brd.brdname, buf) || str_str(brd.title, buf)))||
              (select == 'c' && str_str(brd.class, buf))))
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
#endif
    case 'q':
      return 0;

    default:
      return reciper;
    }
  }
}

#endif
