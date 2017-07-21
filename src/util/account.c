/*-------------------------------------------------------*/
/* util/account.c	( NTHU CS MapleBBS Ver 3.00 )	 */
/*-------------------------------------------------------*/
/* target : �W���H���έp�B�t�θ�Ƴƥ��B�}��		 */
/* create : 95/03/29				 	 */
/* update : 97/03/29				 	 */
/*-------------------------------------------------------*/
/* syntax : ���{���y�H crontab ����A�]�w�ɶ����C�p��	 */
/* 1-5 �� ����						 */
/*-------------------------------------------------------*/
/* notice : brdshm (board shared memory) synchronize	 */
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


#define	MAX_LINE	16
#define	ADJUST_M	10	/* adjust back 10 minutes */


static char fn_today[] = "gem/@/@-act";	/* ����W���H���έp */
static char fn_yesday[] = "gem/@/@=act";	/* �Q��W���H���έp */
extern int errno;


void
keeplog(fnlog, board, title, mode)
  char *fnlog;
  char *board;
  char *title;
  int mode;		/* 0:load 1: rename  2:unlink 3:mark*/
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
    f_mv(fnlog, fpath); /* Thor.990409:�i��partition */
  }
  else
  {
    fp = fdopen(fd, "w");
    fprintf(fp, "�@��: SYSOP (" SYSOPNICK ")\n���D: %s\n�ɶ�: %s\n",
      title, ctime(&hdr.chrono));
    f_suck(fp, fnlog);
    fclose(fp);
    close(fd);
    if (mode)
      unlink(fnlog);
  }
  if(mode == 3)
    hdr.xmode |= POST_MARKED;

  strcpy(hdr.title, title);
  strcpy(hdr.owner, "SYSOP");
  strcpy(hdr.nick, SYSOPNICK);
#if 0
  fd = open(folder, O_WRONLY | O_CREAT | O_APPEND, 0600);
  if (fd < 0)
  {
    unlink(fpath);
    return;
  }
  write(fd, &hdr, sizeof(HDR));
  close(fd);
#endif
  rec_bot(folder, &hdr, sizeof(HDR));
}


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
  register time_t *uptime;
  register int n, turn;

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
    fprintf(stderr, "[account]\tCACHE\treload bcache");

#if 0
    blog("CACHE", "reload bcache");
#endif

    return;
  }
}


struct Tchoice
{
  int count;
  vitem_t vitem;
};


static int
TchoiceCompare(struct Tchoice * i, struct Tchoice * j)
{
  return j->count - i->count;
}


static int
draw_vote(bdh, fpath, vch)
  BRD *bdh;			/* Thor: �ǤJ BRD, �i�d battr */
  char *fpath;
  VCH *vch;
{
  FILE *fp;
  char *bid, *fname, buf[80], bpath[80];
  struct Tchoice choice[MAX_CHOICES];
  int total, items, num, fd, ticket, bollt, *list, *head, *tail;
  struct stat st;

  bid = bdh->brdname;
  fname = strrchr(fpath, '@');

  /* vote item */

  *fname = 'I';
  items = 0;
  if ((fp = fopen(fpath, "r")))
  {
    while (fread(&choice[items].vitem, sizeof(vitem_t), 1, fp) == 1)
    {
      choice[items].count = 0;
      items++;
    }
    fclose(fp);
    unlink(fpath);
  }

  if (items == 0)
    return 0;

  /* vote ballots */

  *fname = 'V';

  if ((fd = open(fpath, O_RDONLY)) < 0)
    return 0;

  if (fstat(fd, &st) || (total = st.st_size) <= 0)
  {
    close(fd);
    unlink(fpath);
    return 0;
  }

  /* �֭p�벼���G */

  bollt = 0;			/* Thor: �`�����k0 */

  list = (int *) malloc(total);
  total = read(fd, list, total);
  close(fd);
  unlink(fpath);

  if (total <= 0)
  {
    free(list);
    return 0;
  }

  head = list;
  tail = (int *) ((char *) list + total);

  do
  {
    for (ticket = *head++, num = 0; ticket && num < items; ticket >>= 1, num++)
    {
      if (ticket & 1)
      {
	choice[num].count++;
	bollt++;
      }
    }
  } while (head < tail);

  free(list);

  /* ���Ͷ}�����G */

  *fname = 'Z';
  if ((fp = fopen(fpath, "w")) == NULL)
    return 0;

  memset(buf, '-', 74);
  buf[74] = '\0';
  fprintf(fp, "\n\n> %s <\n\n�� [%s] �ݪO�벼�G%s\n\n�|��H  �G%s\n\n�|�����G%s\n",
    buf, bid, vch->title, vch->owner, ctime(&vch->chrono));
  fprintf(fp, "�}������G%s\n�� �벼�D�D:\n\n", ctime(&vch->vclose));

  *fname = '@';
  f_suck(fp, fpath);
  unlink(fpath);

  fprintf(fp, "�� �벼���G�G�C�H�i�� %d ���A�@ %d �H�ѥ[�A��X %d ��\n\n",
    vch->maxblt, total / sizeof(int), bollt);

    if (vch->vsort == 's')
      qsort(choice, items, sizeof(struct Tchoice), (int (*)())TchoiceCompare);

    if (vch->vpercent == '%')
      fd = BMAX(1, total / sizeof(int));
      else
	fd = 0;

    for (num = 0; num < items; num++)
    {
      ticket = choice[num].count;
      if (fd)
	fprintf(fp, "\t%-42s%3d �� (%4.1f%%)\n",
	(char *)&choice[num].vitem, ticket, 100.0 * ticket / fd);
	else
	fprintf(fp, "\t%-42s%3d ��\n",
	(char *)&choice[num].vitem, ticket);
    }

    /* other opinions */

  *fname = 'O';
  fputs("\n�� �ڦ��ܭn���G\n\n", fp);
  f_suck(fp, fpath);
  fclose(fp);

  fp = fopen(fpath, "w");	/* Thor: �� O_ �Ȧs�@�U�U... */
  *fname = 'Z';
  f_suck(fp, fpath);
  sprintf(bpath, "brd/%s/@/@vote", bid);
  f_suck(fp, bpath);
  fclose(fp);
  *fname = 'O';
  rename(fpath, bpath);

  /* �N�}�����G post �� [system] �P ���ݪO */
  *fname = 'S';
  unlink(fpath);
  *fname = 'E';
  unlink(fpath);

  *fname = 'Z';

  /* Thor: �Y�ݪ��ݩʬ� BRD_NOVOTE, �h��post�� sysop */

  if (!(bdh->battr & BRD_NOVOTE))
  {
    sprintf(buf, "[�O��] %s <<�ݪO�ﱡ����>>", bid);
    keeplog(fpath, NULL, buf, 0);
  }

  keeplog(fpath, bid, "[�O��] �ﱡ����", 2);

  return 1;
}


static int
draw_board(brd)
  BRD *brd;
{
  int bvote, fd, fsize, alive;
  VCH *vch, *head, *tail;
  time_t now;
  char *fname, fpath[80], buf[80];
  struct stat st;

  /* �� account �� maintain brd->bvote */

  bvote = brd->bvote;

  sprintf(fpath, "brd/%s/.VCH", brd->brdname);

  if ((fd = open(fpath, O_RDWR | O_APPEND, 0600)) < 0
    || fstat(fd, &st)
    || (fsize = st.st_size) <= 0)
  {
    if (fd >= 0)
    {
      close(fd);
      unlink(fpath);
    }
    brd->bvote = 0;
    return bvote ? -1 : 0;
  }

  vch = (VCH *) malloc(fsize);

  /* flock(fd, LOCK_EX); */
  /* Thor.981205: �� fcntl ���Nflock, POSIX�зǥΪk */
  f_exlock(fd);

  read(fd, vch, fsize);

  strcpy(buf, fpath);
  fname = strrchr(buf, '.');
  *fname++ = '@';
  *fname++ = '/';

  head = vch;
  tail = (VCH *) ((char *) vch + fsize);

  alive = 0;

  now = time(NULL);

  do
  {
    if (head->vclose < now)
    {
      strcpy(fname, head->xname);
      draw_vote(brd, buf, head);/* Thor: �ǤJ BRD, �i�d battr */
      head->chrono = 0;
    }
    else
    {
      alive++;
    }
  } while (++head < tail);


  if (alive && alive != fsize / sizeof(VCH))
  {
    ftruncate(fd, 0);
    head = vch;
    do
    {
      if (head->chrono)
      {
	write(fd, head, sizeof(VCH));
      }
    } while (++head < tail);
  }

  /* flock(fd, LOCK_UN);  */
  /* Thor.981205: �� fcntl ���Nflock, POSIX�зǥΪk */
  f_unlock(fd);

  close(fd);
  if (!alive)
    unlink(fpath);

  free(vch);

  if (alive != bvote)
  {
    brd->bvote = alive;
    return 1;
  }

  return 0;
}


static void
closepolls()
{
  BRD *bcache, *head, *tail;
  int state;

  state = 0;

  head = bcache = bshm->bcache;
  tail = head + bshm->number;
  do
  {
    state |= draw_board(head);
  } while (++head < tail);

  if (!state)
    return;

  /* write back the shm cache data */

  if ((state = open(FN_BRD, O_WRONLY | O_CREAT, 0600)) < 0)
    return;

  /* flock(state, LOCK_EX); */
  /* Thor.981205: �� fcntl ���Nflock, POSIX�зǥΪk */
  f_exlock(state);

  write(state, bcache, (char *) tail - (char *) bcache);

  /* flock(state, LOCK_UN); */
  /* Thor.981205: �� fcntl ���Nflock, POSIX�зǥΪk */
  f_unlock(state);

  close(state);
  time(&bshm->uptime);
}


/* ----------------------------------------------------- */
/* build Class image					 */
/* ----------------------------------------------------- */


#define CLASS_RUNFILE	"run/class.run"
#define PROFESS_RUNFILE	"run/profess.run"

#define	CH_MAX	5000


static ClassHeader *chx[CH_MAX];
static int chn;
static BRD *bhead, *btail;


static int
class_parse(key)
  char *key;
{
  char *str, *ptr, fpath[80];
  ClassHeader *chp;
  HDR hdr;
  int i, len, count;
  FILE *fp;

  strcpy(fpath, "gem/@/@");
  str = fpath + sizeof("gem/@/@") - 1;
  for (ptr = key;; ptr++)
  {
    i = *ptr;
    if (i == '/')
      i = 0;
    *str = i;
    if (!i)
      break;
    str++;
  }

  len = ptr - key;

  /* search classes */

  for (i = 1; i < chn; i++)
  {
    str = chx[i]->title;
    if (str[len] == '/' && !memcmp(key, str, len))
      return CH_END - i;
  }

  /* build classes */

  if ((fp = fopen(fpath, "r")))
  {
    int ans;
    struct stat st;

    if (fstat(fileno(fp), &st) || (count = st.st_size / sizeof(HDR)) <= 0)
    {
      fclose(fp);
      return CH_END;
    }

    chx[chn++] = chp =
      (ClassHeader *) malloc(sizeof(ClassHeader) + count * sizeof(short));
    memset(chp->title, 0, CH_TTLEN);
    strcpy(chp->title, key);

    ans = chn;
    count = 0;

    while (fread(&hdr, sizeof(hdr), 1, fp) == 1)
    {
      if (hdr.xmode & GEM_BOARD)
      {
	BRD *bp;

	i = -1;
	str = hdr.xname;
	bp = bhead;

	for (;;)
	{
	  i++;
	  if (!strcasecmp(str, bp->brdname))
	    break;

	  if (++bp >= btail)
	  {
	    i = -1;
	    break;
	  }
	}

	if (i < 0)
	  continue;
      }
      else
      {
	i = class_parse(hdr.title);

	if (i == CH_END)
	  continue;
      }

      chp->chno[count++] = i;
    }

    fclose(fp);

    chp->count = count;
    return -ans;
  }

  return CH_END;
}


static int
chno_cmp(i, j)
  short *i, *j;
{
  return strcasecmp(bhead[*i].brdname, bhead[*j].brdname);
}


static void
class_sort()
{
  ClassHeader *chp;
  int i, j, max;
  BRD *bp;

  max = bshm->number;
  bhead = bp = bshm->bcache;
  btail = bp + max;

  chp = (ClassHeader *) malloc(sizeof(ClassHeader) + max * sizeof(short));

  for (i = j = 0; i < max; i++, bp++)
  {
    if (bp->brdname)
    {
      chp->chno[j++] = i;
    }
  }

  chp->count = j;

  qsort(chp->chno, j, sizeof(short), chno_cmp);

  memset(chp->title, 0, CH_TTLEN);
  strcpy(chp->title, "Boards");
  chx[chn++] = chp;
}


static void
class_image()
{
  int i;
  FILE *fp;
  short len, pos[CH_MAX];
  ClassHeader *chp;

  class_sort();

  class_parse(CLASS_INIFILE);

  if (chn < 2)  /* lkchu.990106: �|�S������ */
    return;

  len = sizeof(short) * (chn + 1);

  for (i = 0; i < chn; i++)
  {
    pos[i] = len;
    len += CH_TTLEN + chx[i]->count * sizeof(short);
  }

  pos[i++] = len;

  if ((fp = fopen(CLASS_RUNFILE, "w")))
  {
    fwrite(pos, sizeof(short), i, fp);
    for (i = 0; i < chn; i++)
    {
      chp = chx[i];
      fwrite(chp->title, 1, CH_TTLEN + chp->count * sizeof(short), fp);
      free(chp);
    }
    fclose(fp);
    rename(CLASS_RUNFILE, CLASS_IMGFILE);
  }



}

#if 0
static void
profess_image()
{
  int i;
  FILE *fp;
  short len, pos[CH_MAX];
  ClassHeader *chp;

  class_sort();
  class_parse(PROFESS_INIFILE);

  if (chn < 2)  /* lkchu.990106: �|�S������ */
    return;

  len = sizeof(short) * (chn + 1);

  for (i = 0; i < chn; i++)
  {
    pos[i] = len;
    len += CH_TTLEN + chx[i]->count * sizeof(short);
  }
  pos[i++] = len;
  if (fp = fopen(PROFESS_RUNFILE, "w"))
  {
     fwrite(pos, sizeof(short), i, fp);
     for (i = 0; i < chn; i++)
     {
     chp = chx[i];
     fwrite(chp->title, 1, CH_TTLEN + chp->count * sizeof(short), fp);
     free(chp);
     }
     fclose(fp);
     rename(PROFESS_RUNFILE, PROFESS_IMGFILE);
   }
}
#endif



/* ----------------------------------------------------- */


static void
ansi_puts(fp, buf, mode)
  FILE *fp;
  char buf[], mode;
{
  static char state = '0';

  if (state != mode)
    fprintf(fp, "\033[3%cm", state = mode);
  if (buf[0])
  {
    fprintf(fp, buf);
    buf[0] = 0;
  }
}


static void
gzip(source, target, stamp)
  char *source, *target, *stamp;
{
  char buf[128];

  sprintf(buf, "`which gzip` -n log/%s%s", target, stamp);
  /* rename(source, &buf[13]); */
  f_mv(source, &buf[17]); /* Thor.990409: �i�� partition */
  system(buf);
}


static void
gtar(source, target, stamp, prune)
  char *source, *target, *stamp;
  int prune;
{
  char buf[128];

  sprintf(buf, "`which tar` cfz log/%s%s.tgz %s", target, stamp, source);
  system(buf);

  if (prune)
  {
    f_rm(source);
#if 0
    sprintf(buf, "/bin/rm -fr %s", source);
    system(buf);
    mkdir(source, 0755);
#endif
  }
}


#if 0
static void
gem_backup()
{
  int op;
  DIR *dirp;
  struct dirent *de;
  char *fname, buf[80], tgz[80];

  if (chdir("gem"))
    return;

  op = (time(0) / 86400);
  if (!(op & 7))
  {
    system("`which tar` cfz /usr/data/gem/gem.tgz .??? ?");
  }

  if (chdir("brd"))
    return;

  if (dirp = opendir("."))
  {
    while (de = readdir(dirp))
    {
      fname = de->d_name;
      if (*fname == '.' || *fname <= ' ')
	continue;

      if ((++op) & 15)
	continue;

      /* Thor.980328: gem��@���N�n, ���M /usr����HD�񤣤U */
      sprintf(buf, "/usr/data/gem/%s.tz0", fname);
      sprintf(tgz, "/usr/data/gem/%s.tgz", fname);
      rename(tgz, buf);
      sprintf(buf, "`which gtar` cfz %s %s", tgz, fname);
      system(buf);
    }
    closedir(dirp);
  }
}
#endif


static void
error(fpath)
  char *fpath;
{
  printf("can not open [%s]\n", fpath);
  exit(1);
}


#if 0
static void
out_clean()
{
  struct stat st;
  struct dirent *de;
  DIR *dirp;
  time_t due;
  char *fname;

  if (chdir("/home/bbs/out"))
    return;

  dirp = opendir(".");
  if (!dirp)
    return;

  due = time(0) - 3 * 24 * 60 * 60;

  while (de = readdir(dirp))
  {
    fname = de->d_name;
    if ((*fname != '.') && !stat(fname, &st) && (st.st_mtime < due))
    {
      unlink(fname);
    }
  }

  closedir(dirp);
}
#endif

int
main()
{
  int fact, hour, max, item, total, i, j, over;
  char date[16];
  char title[80];

  static char act_file[] = "run/var/act";
  static char run_file[] = "run/usies";
  static char tmp_file[] = "run/tmp";
  static char log_file[] = "run/usies=";
#if 0
  static char brd_file[] = FN_BRD_USIES;
#endif
  char buf[256], ymd[16];
  FILE *fp, *fpw;

  int act[26];			/* ����/�֭p�ɶ� */
  time_t now;
  struct tm ntime, ptime, *xtime;

  now = time(NULL);
  xtime = localtime(&now);
  ntime = *xtime;

  now -= ADJUST_M * 60;		/* back to ancent */
  xtime = localtime(&now);
  ptime = *xtime;

  chdir(BBSHOME);
  umask(077);

  /* --------------------------------------------------- */
  /* �W���H���έp					 */
  /* --------------------------------------------------- */

  memset(act, 0, sizeof(act));
  fact = open(act_file, O_RDWR | O_CREAT, 0600);
  if (fact < 0)
    error(act_file);

  if (ptime.tm_hour != 0)
  {
    read(fact, act, sizeof(act));
    lseek(fact, 0, SEEK_SET);
  }

  if(rename(run_file, tmp_file))
  {
    sprintf(buf,"touch %s",tmp_file);
    system(buf);
  }
  if ((fp = fopen(tmp_file, "r")) == NULL)
    error(tmp_file);

  if ((fpw = fopen(log_file, "a")) == NULL)
    error(log_file);

  while (fgets(buf, sizeof(buf), fp))
  {
    fputs(buf, fpw);

    if (!memcmp(buf + 22, "ENTER", 5))
    {
      hour = atoi(buf + 9);
      if (hour >= 0 && hour <= 23)
	act[hour]++;
      continue;
    }

    if (!memcmp(buf + 41, "Stay:", 5))
    {
      if ((hour = atoi(buf + 47)))
      {
	act[24] += hour;
	act[25]++;
      }
      continue;
    }
  }
  fclose(fp);
  fclose(fpw);
  unlink(tmp_file);

  write(fact, act, sizeof(act));
  close(fact);

  for (i = max = total = 0; i < 24; i++)
  {
    total += act[i];
    if (act[i] > max)
      max = act[i];
  }

  item = max / MAX_LINE + 1;
  over = max > 1000;

  if (!ptime.tm_hour)
    keeplog(fn_today, NULL, "[�O��] �W���H���έp", 1);

  if ((fp = fopen(fn_today, "w")) == NULL)
    error(fn_today);

  /* Thor.990329: y2k */
  fprintf(fp, "\t\t\t   \033[1;33;46m [%02d/%02d/%02d] �W���H���έp \033[40m\n",
    ptime.tm_year % 100, ptime.tm_mon + 1, ptime.tm_mday);
  for (i = MAX_LINE + 1; i > 0; i--)
  {
    strcpy(buf, "   ");
    for (j = 0; j < 24; j++)
    {
      max = item * i;
      hour = act[j];
      if (hour && (max > hour) && (max - item <= hour))
      {
	ansi_puts(fp, buf, '3');
	if (over)
	  hour = (hour + 5) / 10;
	fprintf(fp, "%-3d", hour);
      }
      else if (max <= hour)
      {
	ansi_puts(fp, buf, '1');
	fprintf(fp, "�i ");
      }
      else
	strcat(buf, "   ");
    }
    fprintf(fp, "\n");
  }

  if (act[25] == 0) act[25]=1; /* Thor.980928: lkchu patch: ����Ƭ�0 */

  fprintf(fp, "\033[34m"
    "  ��������������������������������������������������������������������������\n  \033[32m"
    "0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23\n\n"
    "\t%s\t\033[35m�`�@�W���H���G\033[37m%-9d\033[35m�����ϥήɶ��G\033[37m%d\033[m\n",
    over ? "\033[35m���G\033[37m10 �H" : "", total, act[24] / act[25] + 1);
  fclose(fp);

  /* --------------------------------------------------- */
  /* build Class image					 */
  /* --------------------------------------------------- */

  bshm_init();
  class_image();
  /*bshm_init();
  profess_image();*/
  /* --------------------------------------------------- */
  /* �t�ζ}��						 */
  /* --------------------------------------------------- */

  closepolls();

  /* --------------------------------------------------- */
  /* ������Y�ƥ��B�������D�έp				 */
  /* --------------------------------------------------- */

  /* Thor.990329: y2k */
  sprintf(ymd, "-%02d%02d%02d",
    ptime.tm_year % 100, ptime.tm_mon + 1, ptime.tm_mday);

  /* if (ptime.tm_hour == 23) */
  if (ntime.tm_hour == 0)
  {

  sprintf(date, "[%2d �� %2d ��] ", ptime.tm_mon + 1, ptime.tm_mday);

      /* �H�U�O�ثe�S���b�ϥΪ����� */

#if 0

//    sprintf(title, "[�O��] %s�ϥΦ��Ʋέp", date);
//    system("bin/spss.sh");
//    keeplog("run/spss.log", NULL, title, 2);

//    sprintf(title, "[�O��] %s�����\\Ū���Ʋέp", date);
//    system("bin/brd_usies.sh");
//    keeplog(FN_BRD_USIES".log", NULL, title, 2); /* ��z�L�᪺ log */
//    keeplog(FN_BRD_USIES, BRD_SECRET, title, 2); /* ����z�e�� log */
//    gzip(FN_BRD_USIES,"brdusies/brdusies",ymd);   /* ����z�e�� log */

//    sprintf(title, "[�O��] %s�ק�峹����", date);
//    keeplog(FN_POSTEDIT_LOG, BRD_SECRET, title, 2);

//    sprintf(title, "[�O��] %s�j���p�Ҭd�]����", date);
//    keeplog(FN_UEEQUERY_LOG, BRD_SECRET, title, 2);

//    sprintf(title, "[�O��] %s�q�l�u�W�r�����", date);
//    keeplog(FN_PYDICT_LOG, BRD_SECRET, title, 2);

//    sprintf(title, "[�O��] %sbbsmail mailpost����", date);
//    keeplog(FN_BBSMAILPOST_LOG, BRD_SECRET, title, 2);

//    sprintf(title, "[�O��] %s�׫H����", date);
//    keeplog(FN_BANMAIL_LOG, NULL, title, 2);
//    gzip(FN_BANMAIL_LOG,"banmail/banmail",ymd);  /* �׫H���� */

/*
    sprintf(title, "[�O��] %s�H�H����", date);
    keeplog(FN_MAIL_LOG, BRD_SECRET, title, 2);
*/
//    sprintf(title, "[�O��] %s��H����", date);
//    keeplog(FN_INNBBS_LOG, BRD_SECRET, title, 2);
//    gzip(FN_INNBBS_LOG,"innbbsd/innbbsd",ymd);  /* ��H���� */

//    sprintf(title, "[�O��] %sMailService�ϥά���", date);
//    keeplog(FN_MAILSERVICE_LOG, BRD_SECRET, title, 2);

//    sprintf(title, "[�O��] %s�ݪ��H��R������", date);
//    keeplog(FN_EXPIRE_LOG, BRD_SECRET, title, 2);
//    gzip(FN_EXPIRE_LOG,"expire/expire",ymd);  /* �ݪ��H��R������ */

//    sprintf(title, "%sE-Mail �y�q�έp", date);
//    keeplog("run/unlog.log", NULL, title, 2);

     /* Thor.990403: �έpOVER */
//    system("grep OVER run/bmta.log | cut -f2 | cut -d' ' -f2- | sort | uniq -c > run/over.log");
//    sprintf(title, "%sE-Mail over max connection �έp", date);
//    keeplog("run/over.log", NULL, title, 2);

#endif

    /* �H�U�O���K���� */ 
    sprintf(title, "[�O��] %s�峹�[�ݬ���", date);
    keeplog(FN_BROWSE_LOG, BRD_SECRET, title, 3);

    sprintf(title, "[�O��] %s�ΦW�O����", date);
    keeplog(FN_ANONYMOUS_LOG, BRD_SECRET, title, 3);    

    /*  �ѩ�@���B��}�ɪ��A, �O�G�� log by statue
        �ק� SIG_USR1 �i�N log dump �X�� by visor */
    system("kill -USR1 `ps -auxwww | grep xchatd | awk '{print $2}'`");
    sprintf(title, "[�O��] %s��ѫǶi�X����", date);
    keeplog(FN_CHAT_LOG".old", BRD_SECRET, title, 2);

    sprintf(title, "[�O��] %spop3�{�Ҭ���", date);
    keeplog(FN_VERIFY_LOG, BRD_SECRET, title, 2);

    /* �ѩ� FN_POP3_LOG �@���B��}�ɪ��A, �O�G���N�� pop3d ����
       �ɮצ۵M�N�|�g�J, ���� log �~�|���ɮ� by statue */
    sprintf(title, "[�O��] %sPOP3D����", date);
    system("kill `ps -auxwww | grep bpop3d | awk '{print $2}'`");
    keeplog(FN_POP3_LOG, BRD_SECRET, title, 2);

    sprintf(title, "[�O��] %s���� E-mail �{�Ҭ���", date);
    keeplog("run/emailaddr.log", BRD_SECRET, title, 2);

    sprintf(title, "[�O��] %s���Ȧ欰����", date);
    keeplog(FN_BLACKSU_LOG, BRD_SECRET, title, 2);

    sprintf(title, "[�O��] %s�I�q����", date);
    keeplog(FN_SONG_LOG, BRD_SECRET, title, 2);

    sprintf(title, "[�O��] %s���ˤ峹����", date);
    keeplog(FN_RECOMMEND_LOG, BRD_SECRET, title, 2);

    sprintf(title, "[�O��] %s�Ȧ�������", date);
    keeplog(FN_BANK, BRD_SECRET, title, 2);

    sprintf(title, "[�O��] %s�ө��������", date);
    keeplog("run/shop.log", BRD_SECRET, title, 2);

    system("cat run/usies | grep APPLY > run/apply.log");
    sprintf(title, "[�O��] %s�C����U�ϥΪ̬���", date);
    keeplog("run/apply.log", BRD_SECRET, title, 2);

    sprintf(title, "[�O��] %s���ȦC��", date);
    keeplog(FN_MANAGER_LOG, BRD_SECRET, title, 2);

    sprintf(title, "[�O��] %s���v�C��", date);
    keeplog(FN_CRIMINAL_LOG, BRD_SECRET, title, 2);

    sprintf(title, "[�O��] %s�O�D�C��", date);
    keeplog(FN_BMLIST_LOG, BRD_SECRET, title, 2);

    sprintf(title, "[�O��] %s�v���ק����", date);
    keeplog(FN_SECURITY, BRD_SECRET, title, 2);

    sprintf(title, "[�O��] %s��ذϯ��ެ���", date);
    keeplog(FN_GINDEX_LOG, BRD_SECRET, title, 2);

    sprintf(title, "[�O��] %s��ذ��ˬd����", date);
    keeplog(FN_GCHECK_LOG, BRD_SECRET, title, 2);

    sprintf(title, "[�O��] %s�ݪO�H���ʧR������", date);
    keeplog(FN_EXPIRED_LOG, BRD_SECRET, title, 2);

    sprintf(title, "[�O��] %s�ϥΪ̿� ACCT ����", date);
    keeplog("run/NOACCT.log", BRD_SECRET, title, 2);

    sprintf(title, "[�O��] %s�ϥΪ̿� .DIR ����", date);
    keeplog("run/NOUSRDIR.log", BRD_SECRET, title, 2);

    sprintf(title, "[�O��] %s�ݪO�� .DIR ����", date);
    keeplog("run/NOBRDDIR.log", BRD_SECRET, title, 2);

#ifdef	HAVE_FAVORITE
    sprintf(title, "[�O��] %s�ڪ��̷R����", date);
    keeplog(FN_FAVORITE_LOG, BRD_SECRET, title, 2);
#endif

#ifdef  HAVE_BBSNET
    sprintf(title, "[�O��] %sBBSNET �ϥά���", date);
    keeplog(FN_BBSNET_LOG, BRD_SECRET, title, 2);
#endif

    /* �H�U�O���}���� */ 

    //sprintf(date, "[%2d �� %2d ��] ", ptime.tm_mon + 1, ptime.tm_mday);
    sprintf(title, "[�O��] %s�峹�g�Ʋέp", date);
    keeplog(FN_POST_LOG, NULL, title, 2);

    sprintf(title, "[�O��] %s�p����������", date);
    keeplog(FN_PIPMONEY_LOG, NULL, title, 2);

    sprintf(title, "[�O��] %s" BOARDNAME "�d����", date);
    keeplog(FN_PIP_LOG, NULL, title, 2);

    sprintf(title, "[�O��] %s" BOARDNAME "��a�p", date);
    keeplog(FN_MINE_LOG, NULL, title, 2);

    sprintf(title, "[�O��] %s�Ĳ��W���d���O", date);
    keeplog(FN_NOTE_ALL, NULL, title, 2);

    sprintf(title, "[�O��] %s�����b������", date);
    keeplog(FN_REAPER_LOG, NULL, title, 2);
    
    sprintf(title, "[�O��] %s���i�O�D����", date);
    keeplog(FN_LAZYBM_LOG, NULL, title, 2);

    sprintf(title, "[�O��] %s����Q�j�������D", date);
    keeplog("gem/@/@-day", NULL, title, 0);

    if ((fp = fopen(fn_yesday, "w")))
    {
      f_suck(fp, fn_today);
      fclose(fp);
    }

    if (ntime.tm_wday == 0)
    {
      sprintf(title, "[�O��] %s���g���Q�j�������D", date);
      keeplog("gem/@/@-week", NULL, title, 0);
    }

    if (ntime.tm_mday == 1)
    {
      sprintf(title, "[�O��] %s����ʤj�������D", date);
      keeplog("gem/@/@-month", NULL, title, 0);
    }

    if (ntime.tm_yday == 1)
    {
      sprintf(title, "[�O��] %s�~�צʤj�������D", date);
      keeplog("gem/@/@-year", NULL, title, 0);
    }

    gzip(log_file, "usies/usies", ymd);

    gzip(FN_CHATDATA_LOG".old","chat/chat",ymd);

#if 0
#ifdef  LOG_BRD_USIES
    gzip(brd_file, FN_BRD_USIES, ymd);   /* lkchu.981201: �ƥ��ݪ��\Ū�O�� */
#endif
#endif

  }
  else if (ntime.tm_hour == 1)
  {
    sprintf(date, "[%2d �� %2d ��] ", ptime.tm_mon + 1, ptime.tm_mday);

    sprintf(title, "[�O��] %s�ϥΪ̽s������", date);
    system("bin/userno");
    keeplog(FN_USERNO_LOG, BRD_SECRET, title, 2);
    gzip(FN_USERNO_LOG, "userno/userno", ymd);        /* �Ҧ� [�ϥΪ̽s������] �O�� */


/* ������� Secret �ݪO���d�\ by statue */
    gzip(FN_MAIL_LOG, "mail/mail", ymd);	/* �Ҧ� [�H�H] �O�� */
#if 0
    gzip(FN_INNBBS_LOG, "innbbs", ymd);  /* �Ҧ� [��H] �O�� */
    gzip(FN_PIPMONEY_LOG, "chicken", ymd);  /* �Ҧ� [�p��] �O�� */
    gzip(FN_SONG_LOG, "ordersongs", ymd);  /* �Ҧ� [�I�q] �O�� */
    gzip(FN_POP3_LOG, "pop3", ymd);	/* �Ҧ� [POP3] ���� */
#endif

#if 0
    if (ntime.tm_mday == 1)
    {
      gzip(FN_MAIL_LOG, "mailog", ymd);	/* �Ҧ� [�H��] �O�� */
      gzip(FN_BSMTP_LOG, "bsmtp", ymd);	/* �Ҧ� [�H�H] �O�� */
      gzip(FN_BGUARD_LOG, "bguard", ymd);	/* �Ҧ� [load] �O�� */
      gzip(FN_GEMD_LOG, "gemd", ymd);	/* �Ҧ� [    ] �O�� */
      gzip(FN_INNBBS_LOG, "innbbs", ymd);	/* �Ҧ� [��H] �O�� */
      system("bin/ctlinnbbsd reload");
      gzip(FN_RFORM_LOG, "rform", ymd);	/* �Ҧ� [���U] �O�� */
    }
#endif

    if (ntime.tm_wday == 6)
    {
#if 0
      gzip(FN_LOGIN_LOG, "login", ymd);	/* �Ҧ� [�n��] �O�� */
      gzip("run/mode.cur", "mode", ymd);
      gzip("run/post.all", "post", ymd);
#endif
//��091205����
//      gtar("usr/@", "reaper/reaper", ymd, 1);

      /* Thor.981221: ����: �U�����Ш̦U�H�ݨD�אּ�{�����a, �H�Ƥ��{��,
                            ���γƥ��{�����N comment out�a */
#if 0
      gtar("/usr/bbssrc/maple", "maple", ymd, 0);
      gtar("/usr/bbssrc/util", "util", ymd, 0);
#endif
    }
  }

#ifdef HAVE_SIGNED_MAIL
    srandom(time(NULL));
#if (PRIVATE_KEY_PERIOD == 0)
    if(!dashf(PRIVATE_KEY))
#else
    if(!dashf(PRIVATE_KEY) || (random() % PRIVATE_KEY_PERIOD) == 0)
#endif
    {
      sprintf(title,"log/prikey%s", ymd);
      f_mv(PRIVATE_KEY,title);
      i = 8;
      for(;;)
      {
        j = random() & 0xff;
        if(!j) continue;
        title[--i] = j;
        if(i == 0) break;
      }
      rec_add(PRIVATE_KEY, title, 8);
    }
#endif

  exit(0);
}
