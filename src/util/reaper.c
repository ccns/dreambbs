/*-------------------------------------------------------*/
/* util/reaper.c	( NTHU CS MapleBBS Ver 3.00 )	 */
/*-------------------------------------------------------*/
/* target : �ϥΪ̱b���w���M�z				 */
/* create : 95/03/29				 	 */
/* update : 97/03/29				 	 */
/*-------------------------------------------------------*/
/* syntax : reaper					 */
/*-------------------------------------------------------*/
/* notice : ~bbs/usr/@/     - expired users's data	 */
/*          run/reaper.log  - list of expired users	 */
/*          run/manager.log - list of managers		 */
/*          run/lazybm.log - list of lazy bm		 */
/*          run/emailaddr.log - list of same email addr  */
/*-------------------------------------------------------*/


#include <stdio.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

#if (defined(SOLARIS) || defined(LINUX))
#include <sys/param.h>
#else
#include <machine/param.h>
#endif
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "config.h"
#include "struct.h"
#include "perm.h"
#include "bbs.h"

#define BANREGIST       "etc/banregist.acl"
#define CHECK_LAZYBM
#define	EADDR_GROUPING  /* Thor.980930: �Y�����ɥ� undef�Y�i */

#undef	DEBUG

#ifdef CHECK_LAZYBM
#define DAY_LAZYBM	30
#endif

#ifdef EADDR_GROUPING                         	
/* Thor.980930: 3�ӥH�Waccount�P�@email�Y�`�N */
#define EMAIL_REG_LIMIT 	3
#endif

#define DAY_NEWUSR	90	 /* login ���W�L 10 �� */
#define DAY_FORFUN	365	 /* �����������{�� */
#define DAY_OCCUPY	730  /* �w���������{�� */


static time_t due_newusr;
static time_t due_forfun;
static time_t due_occupy;



static int visit;
static int prune;
static int manager;
static int criminal;
static int invalid;
static int vacation;
static int bms;

static FILE *flog;
static FILE *flst;
static FILE *fcri;
static FILE *fbml;
static FILE *fmah;


#ifdef CHECK_LAZYBM
static time_t due_lazybm;
static int lazybm;
static FILE *fbm;
#endif


#ifdef EADDR_GROUPING
static FILE *faddr;
#endif

/* ----------------------------------------------------- */
/* ----------------------------------------------------- */


static int funo;
static int max_uno;
static SCHEMA schema;

REAPER_TIME reaper_time[] = 
{
  {701,1002},{117,229},{0,0}
};


COUNTER *counter;
BCACHE *bshm;


#ifdef DEBUG
static void
logit(msg)
  char *msg;
{
  FILE *fp;
  fp = fopen("reaper.debug.log","a+");
  fprintf(fp,"%s\n",msg);
  fclose(fp);
}
#endif


static void *
attach_shm(shmkey, shmsize)
  register int shmkey, shmsize;
{
  register void *shmptr;
  register int shmid;

  shmid = shmget(shmkey, shmsize, 0);
  if(shmid < 0)
    return NULL;

  shmptr = (void *) shmat(shmid, NULL, 0);
  return shmptr;
}

static void
userno_free(uno)
  int uno;
{
  off_t off;
  int fd;

  fd = funo;

  /* flock(fd, LOCK_EX); */
  /* Thor.981205: �� fcntl ���Nflock, POSIX�зǥΪk */
  f_exlock(fd);

  time(&schema.uptime);
  off = (uno - 1) * sizeof(schema);
  if (lseek(fd, off, SEEK_SET) < 0)
    exit(2);
  if (write(fd, &schema, sizeof(schema)) != sizeof(schema))
    exit(2);

  /* flock(fd, LOCK_UN); */
  /* Thor.981205: �� fcntl ���Nflock, POSIX�зǥΪk */
  f_unlock(fd);
}


/* ----------------------------------------------------- */
/* ----------------------------------------------------- */


static void
levelmsg(str, level)
  char *str;
  int level;
{
  static char perm[] = "bctpjm#x--------PTCMSNL*B#KGACBS";
  int len = 32;
  char *p = perm;

  do
  {
    *str = (level & 1) ? *p : '-';
    p++;
    str++;
    level >>= 1;
  } while (--len);
  *str = '\0';
}


static void
datemsg(str, chrono)
  char *str;
  time_t *chrono;
{
  struct tm *t;

  t = localtime(chrono);
  /* Thor.990329: y2k */
  sprintf(str, "%02d/%02d/%02d%3d:%02d:%02d ",
    t->tm_year % 100, t->tm_mon + 1, t->tm_mday,
    t->tm_hour, t->tm_min, t->tm_sec);
}


/* ----------------------------------------------------- */
/* ----------------------------------------------------- */

#ifdef EADDR_GROUPING
/* Thor.980930: �N�P�@email addr��account, �����_�ӨæC��
      �u�@��z: 1.���� str_hash() �N email addr �ƭȤ�
                2.��binary search, ���happend userno, �䤣��h insert �sentry
                3.�N userno list ��z�C�X

      ��Ƶ��c: chain: int hash, int link_userno
                plist: int next_userno

      �Ȯɹw��email addr�`�Ƥ��W�L100000, 
      �Ȯɹw��user�`�Ƥ��W�L 100000

*/

typedef struct {
  int hash;
  int link;
} Chain;

static Chain *chain;
static int *plist;
static int numC;
   
static void
eaddr_group(userno, eaddr)
  int userno;
  char *eaddr;
{
  int left, right, mid, i;
  int hash = str_hash(eaddr,0);

  left = 0;
  right = numC - 1;
  for (;;)
  {
    int cmp;
    Chain *cptr;

    if (left > right)  /* Thor.980930: ��S */
    {
      for (i = numC; i > left; i--)
        chain[i] = chain[i - 1];

      cptr = &chain[left];
      cptr->hash = hash;
      cptr->link = userno;
      plist[userno] = 0; /* Thor: tail */
      numC++;
      break;
    }

    mid = (left + right) >> 1;
    cptr = &chain[mid];
    cmp = hash - cptr->hash;

    if (!cmp)
    {
      plist[userno] = cptr->link;
      cptr->link = userno;
      break;
    }

    if (cmp < 0)
      right = mid - 1;
    else
      left = mid + 1;
  }
}

static void
report_eaddr_group()
{
  int i, j, cnt;
  off_t off;
  int fd;

  SCHEMA s;
  /* funo */

  fprintf(faddr, "Email registration over %d times list\n\n", EMAIL_REG_LIMIT);

  for(i = 0; i < numC; i++)
  {
    for(cnt = 0, j = chain[i].link; j; cnt++, j = plist[j]);
    if (cnt > EMAIL_REG_LIMIT)
    {
      fprintf(faddr, "\n> %d\n", chain[i].hash);
      for(j = chain[i].link; j; j = plist[j])
      {
        off = (j - 1) * sizeof(schema);
        if (lseek(funo, off, SEEK_SET) < 0)
        {
          fprintf(faddr,"==> %d) can't lseek\n", j);
          continue;
        }
        if (read(funo, &s, sizeof(schema)) != sizeof(schema))
        {
          fprintf(faddr,"==> %d) can't read\n", j);
          continue;
        }
        else
        {
  	  ACCT acct;
          char buf[512];
          if (s.userid[0]<=' ') 
          {
            fprintf(faddr,"==> %d) has been reapered\n", j);
	    continue;
          }
	  usr_fpath(buf, s.userid, ".ACCT");
  	  fd = open(buf, O_RDONLY, 0);
  	  if (fd < 0)
          {
            fprintf(faddr,"==> %d)%-13s can't open\n", j, s.userid);
	    continue;
          }
          if (read(fd, &acct, sizeof(acct)) != sizeof(acct)) 
          {
            fprintf(faddr,"==> %d)%-13s can't read\n", j, s.userid);
            continue;
          }
          close(fd);

    	  datemsg(buf, &acct.lastlogin);
          fprintf(faddr,"%5d) %-13s%s[%d]\t%s\n", acct.userno, acct.userid, buf, acct.numlogins, acct.email);
        }
      }
    }
  }
}

#endif

static int
bm_list(userid,msg)                 /* ��� userid �O���ǪO���O�D */
  char *userid;
  char *msg;
{
  int len, ch;
  BRD *bhdr, *tail;
  char *list;

  int num;
  
  num = 0;

  len = strlen(userid);

  bhdr = bshm->bcache;
  tail = bhdr + bshm->number;
  *msg = '\0';

  do
  {
    list = bhdr->BM;
    ch = *list;
    if ((ch > ' ') && (ch < 128))
    {
      do
      {
        if (!str_ncmp(list, userid, len))
        {
          ch = list[len];
          if ((ch == 0) || (ch == '/'))
          {
            strcat(msg,bhdr->brdname);
            strcat(msg," ");
            num++;
            break;
          }
        }
        while ((ch = *list++))
        {
          if (ch == '/')
            break;
        }
      } while (ch);
    }
  } while (++bhdr < tail);
  return num;
}

static void
reaper(fpath, lowid)
  char *fpath;
  char *lowid;
{
  int fd, login;
  usint ulevel;
  time_t life;
  char buf[512], data[40];
  char bmlist[256];
  ACCT acct;
  int num;

  sprintf(buf, "%s/.ACCT", fpath);
  fd = open(buf, O_RDONLY, 0);
  if (fd < 0)
  { /* Thor.981001: �[�� log */
    fprintf(flog, "acct can't open %-13s ==> %s\n", lowid, buf);
    return;
  }

  if(read(fd, &acct, sizeof(acct))!=sizeof(acct))
  {
    fprintf(flog, "acct can't read %-13s ==> %s\n", lowid, buf);
    close(fd);
    return;
  }
  close(fd);
  fprintf(fmah,"%-13s %-20s %-40.40s\n",acct.userid,acct.realname,acct.email);

  fd = acct.userno;

  if ((fd <= 0) || (fd > max_uno))
  {
    fprintf(flog, "%5d) %-13s ==> %s\n", fd, acct.userid, buf);
    return;
  }

  ulevel = acct.userlevel;
  life = acct.lastlogin;
  login = acct.numlogins;

#ifdef EADDR_GROUPING
  if (ulevel & PERM_VALID) /* Thor.980930: �u�ݳq�L�{�Ҫ� email, ���� */
    eaddr_group(fd, acct.email);
#endif

  datemsg(buf, &acct.lastlogin);
  levelmsg(data, ulevel);

  if (ulevel & (PERM_MANAGE|PERM_BM))
  {
    if(ulevel != (PERM_BASIC|PERM_CHAT|PERM_PAGE|PERM_POST|PERM_VALID|PERM_BM))
    {
      fprintf(flst, "%5d) %-13s%s[%s] %d\n", fd, acct.userid, buf, data, login);
      manager++;
    }

    if((ulevel & PERM_BM))
    {
      if(bshm)
      {
        num = bm_list(acct.userid,bmlist);
        fprintf(fbml, "%5d) %-13s%s %-6d %-2d %s\n", fd, acct.userid, buf, login, num, bmlist);
        bms++;
#if 0        
        if(*bmlist == '\0')
        {
          char fl[256];

          acct.userlevel &= (~PERM_BM);
          sprintf(fl, "%s/.ACCT", fpath);
          fd = open(fl, O_WRONLY, 0);
          if (fd < 0)
          { /* Thor.981001: �[�� log */
            fprintf(flog, "acct can't open %-13s ==> %s\n", lowid, fl);
          }

          if(write(fd, &acct, sizeof(acct))!=sizeof(acct))
          {
            fprintf(flog, "acct can't read %-13s ==> %s\n", lowid, fl);
          }
          if(fd >= 0)
            close(fd);
        }
#endif        
        
      }
#ifdef CHECK_LAZYBM
      if(life < due_lazybm)
      {
        fprintf(fbm, "%5d) %-13s%s %d\n", fd, acct.userid, buf, login);
        lazybm++;
      }
#endif
    }
  }
  else if (ulevel)		/* guest.ulevel == 0, �û��O�d */
  {
    if(ulevel & PERM_CRIMINAL)
    {
        fprintf(fcri, "%5d) %-13s%s[%s] %d\n", fd, acct.userid, buf, data, login);
        criminal++;
    }
    
    if (login <= 3 && life < due_newusr)
      life = 0;

#ifdef	VACATION
    if (life < due_occupy)
      life = 0;
#else
    if (ulevel & PERM_PURGE)    /* lkchu.990221: �u�M���b���v */
    {
      life = 0;
    }
    else if (ulevel & (PERM_DENYSTOP|PERM_DENYLOGIN))
    {
      life = 1;
    }
    else if (ulevel & PERM_VALID)
    {
      if (life < due_occupy)
	life = 0;
    }
    else
    {
      if (life < due_forfun)
	life = 0;
      else
	invalid++;
    }
#endif

    if (!life && !vacation)
    {
      sprintf(buf, "usr/@/%s", lowid);

      while (rename(fpath, buf))
      {
        extern int errno;

        fprintf(flog, "rename %s ==> %s : %d\n", fpath, buf, errno);
        sprintf(buf, "usr/@/%s.%d", lowid, (int)++life);
      }

      userno_free(fd);
      fprintf(flog, "%5d) %-13s%s%d\n", fd, acct.userid, buf, login);
      prune++;
    }
#ifdef	HAVE_MAILGEM
    else if(!(ulevel & PERM_MBOX))
    {
      char fph[128];
      usr_fpath(fph,acct.userid,"gem");
      f_rm(fph);
    }
#endif    
  }

  visit++;
}


static void
traverse(fpath)
  char *fpath;
{
  DIR *dirp;
  struct dirent *de;
  char *fname, *str;

  /* visit the second hierarchy */

  if (!(dirp = opendir(fpath)))
  {
    fprintf(flog, "## unable to enter hierarchy [%s]\n", fpath);
    return;
  }

  for (str = fpath; *str; str++);
  *str++ = '/';

  while ((de = readdir(dirp)))
  {
    fname = de->d_name;
    if (fname[0] > ' ' && fname[0] != '.')
    {
      strcpy(str, fname);
      reaper(fpath, fname);
    }
  }
  closedir(dirp);
}

static int
check_vacation()
{
  struct tm ptime,*xtime;
  int now,i;
  
  now = time(NULL);
  xtime = localtime((time_t *)&now);
  ptime = *xtime;
  now = (ptime.tm_mon+1)*100 + ptime.tm_mday;
  for(i=0;reaper_time[i].start;i++)
  {
    if(now < reaper_time[i].end && now >= reaper_time[i].start)
      return 1;
  }
  return 0;
}

int
main()
{
  int ch;
  time_t start, end;
  struct stat st;
  char *fname, fpath[256];
  
  
  setuid(BBSUID);
  setgid(BBSGID);
  chdir(BBSHOME);

  bshm = attach_shm(BRDSHM_KEY, sizeof(BCACHE));
      
  if(bshm->uptime < 0)
    bshm = NULL;
            
  vacation = check_vacation();
  flog = fopen(FN_REAPER_LOG, "w");
  if (flog == NULL)
    exit(1);
  fprintf(flog,"\n  UNO) ID                                     \n\n");

  flst = fopen(FN_MANAGER_LOG, "w");
  if (flst == NULL)
    exit(1);

  fcri = fopen(FN_CRIMINAL_LOG, "w");
  if (fcri == NULL)
    exit(1);

  fbml = fopen(FN_BMLIST_LOG, "w");
  if (fbml == NULL)
    exit(1);

  fmah = fopen(FN_MATCH_LOG, "w");
  if (fmah == NULL)
    exit(1);
    
#ifdef CHECK_LAZYBM
  fbm = fopen(FN_LAZYBM_LOG, "w");
  if (fbm == NULL)
    exit(1);
#endif

#ifdef EADDR_GROUPING
  faddr = fopen(FN_EMAILADDR_LOG, "w");
  if (faddr == NULL)
    exit(1);
#endif

#ifdef EADDR_GROUPING
  funo = open(".USR", O_RDWR | O_CREAT, 0600); /* Thor.980930: for read name */
#else
  funo = open(".USR", O_WRONLY | O_CREAT, 0600);
#endif
  if (funo < 0)
    exit(1);

  /* ���]�M���b�������A�s���U�H�Ƥ��|�W�L 300 �H */

  fstat(funo, &st);
  max_uno = st.st_size / sizeof(SCHEMA) + 300;

  time(&start);

  due_newusr = start - DAY_NEWUSR * 86400;
  due_forfun = start - DAY_FORFUN * 86400;
  due_occupy = start - DAY_OCCUPY * 86400;

#ifdef CHECK_LAZYBM
  due_lazybm = start - DAY_LAZYBM * 86400;
#endif

#ifdef EADDR_GROUPING
  chain = (Chain *) malloc(max_uno * sizeof(Chain));
  plist = (int *) malloc((max_uno + 1) * sizeof(int));
  if (!chain || !plist) 
  {
    fprintf(faddr,"out of memory....\n");
    exit(1);
  }
#endif

  strcpy(fname = fpath, "usr/@");
  mkdir(fname, 0700);
  fname = (char *) strchr(fname, '@');

  /* visit the first hierarchy */

  for (ch = 'a'; ch <= 'z'; ch++)
  {
    fname[0] = ch;
    fname[1] = '\0';
    traverse(fpath);
  }
  for (ch = '0'; ch <= '9'; ch++)
  {
    fname[0] = ch;
    fname[1] = '\0';
    traverse(fpath);
  }
  unlink(BANREGIST);

#ifdef DEBUG
    logit("report_eaddr_group");
#endif

#ifdef EADDR_GROUPING
  report_eaddr_group(); /* Thor.980930: before close funo */
#endif

  close(funo);

  fprintf(flst, "\n�ثe�t�κ޲z�̤H��: %d\n\n", manager);
  fclose(flst);
  fprintf(fcri, "\n�ثe�t�ΰ��v�H��: %d\n\n", criminal);
  fclose(fcri);
  fprintf(fbml, "\n�ثe����ڤ��j�a���O�D�H��: %d\n\n",bms);
  fclose(fbml);
  
  fclose(fmah);

  time(&end);
  fprintf(flog, "\n\n# �}�l�M���ɶ�: %s", ctime(&start));
  fprintf(flog, "\n# �M�������ɶ�: %s", ctime(&end));
  end -= start;
  start = end % 60;
  end /= 60;
  fprintf(flog, "# �@��ɶ� : %d:%d:%d\n", (int)end / 60, (int)end % 60, (int)start);
  fprintf(flog, "# ���U�H��(�]�t���{��) : %d\n", visit);
  fprintf(flog, "# �R���H�� : %d\n", prune);
  fprintf(flog, "# ���{�ҤH�� : %d\n", invalid);
  fclose(flog);

#ifdef CHECK_LAZYBM
  fprintf(fbm, "\n�ڤ��j�a %d �餺���n�J���O�D�H��: %d\n\n", DAY_LAZYBM, lazybm);
  fclose(fbm);
#endif


#ifdef DEBUG
    logit("counter");
#endif
  counter = attach_shm(COUNT_KEY, sizeof(COUNTER));
  if(counter)
    counter->max_regist = visit;

#ifdef EADDR_GROUPING
  free(chain);
  free(plist);
  fclose(faddr);
#endif

  exit(0);
}
