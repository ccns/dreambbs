/*-------------------------------------------------------*/
/* util/bmtad.c	( NTHU CS MapleBBS Ver 3.00 )		 */
/*-------------------------------------------------------*/
/* target : Mail Transport Agent for BBS		 */
/* create : 96/11/20					 */
/* update : 96/12/15					 */
/*-------------------------------------------------------*/
/* syntax : bmtad					 */
/*-------------------------------------------------------*/
/* notice : brdshm (board shared memory) synchronize	 */
/*-------------------------------------------------------*/


#include "bbs.h"

#undef	TRACE
#define	TRACE  logit

#define	NO_SPAM

/* Thor.990221: 儘量 fflush出log */
#undef	DEBUG

#define ADM_ALIASES     {"root", "mailer-daemon", "postmaster", "bbs", NULL}

/* Thor.990411: sendmail.reject暴力法, 注意, 全小寫, 為bbs的id */
#define REJECT     {"destroy", "needforlove", NULL}

#define	REJECT_GB_ENCODING	/* 檔掉 GB 編碼的信件 */

/* lkchu.981201 */
#define	HOST_ALIASES	{MYHOSTNAME,BBSIP,"infopro.com.tw","140.138.2.236","es1.seed.net.tw","epaper.com.tw","msy.epaper.com.tw","msx.epaper.com.tw","msz.epaper.com.tw","free2.infopro.com.tw", NULL}

#ifdef	NO_SPAM
#define	NO_SPAM_HOST	{"bbs.cse.yzu.edu.tw","mail.yzu.edu.tw","cloud.yzu.edu.tw","infopro.com.tw","es1.seed.net.tw","epaper.com.tw","msy.epaper.com.tw","msx.epaper.com.tw","msz.epaper.com.tw",NULL}
#endif

#include <sys/wait.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/ipc.h>
#include <sys/shm.h>


#include "host_ht.ic"
#include "acl.ic"
#include "hash.ic"
#include "mailer.ic"


#define SERVER_USAGE
#define	WATCH_DOG
#define	HAVE_RLIMIT


#define BMTA_PIDFILE	"run/bmta.pid"
#define BMTA_LOGFILE	"run/bmta.log"


#define BMTA_PORT	25
#define BMTA_PERIOD	(60 * 15)
#define BMTA_TIMEOUT	(60 * 30)
#define BMTA_LOGTIME	(3 * 86400)
#define BMTA_FAULT	100


#define TCP_BACKLOG	3
#define TCP_BUFSIZ	4096
#define TCP_LINSIZ	256
#define TCP_RCVSIZ	2048


#define MIN_DATA_SIZE	8000
#define MAX_DATA_SIZE	65535
#define MAX_CMD_LEN	1024
#define	MAX_RCPT	7
#define MAX_HOST_CONN	2


#define	SPAM_TITLE_LIMIT	15
#define SPAM_MHOST_LIMIT	500
#define	SPAM_MFROM_LIMIT	500
#define SPAM_FORGE_LIMIT	10	/* 不是寫錯的，是故意的 */


/* ----------------------------------------------------- */
/* SMTP commands					 */
/* ----------------------------------------------------- */


typedef struct
{
  void (*func) ();
  char *cmd;
  char *help;
}      Command;


/* ----------------------------------------------------- */
/* client connection structure				 */
/* ----------------------------------------------------- */


typedef struct RCPT
{
  struct RCPT *rnext;
  char userid[0];
}    RCPT;


typedef struct Agent
{
  struct Agent *anext;
  int sock;
  int sno;
  int state;
  int mode;
  unsigned int ip_addr;

  time_t uptime;
  time_t tbegin;

  int xsize;
  int xrcpt;
  int xdata;
  int xerro;
  int xspam;

  char ident[80];
  char memo[80];
  char fpath[80];

  char from[80];
  char title[80];

  char addr[80];
  char nick[80];

  int nrcpt;			/* number of rcpt */
  RCPT *rcpt;

  char *data;
  int used;
  int size;
}     Agent;


static int servo_sno;


/* ----------------------------------------------------- */
/* connection state					 */
/* ----------------------------------------------------- */


#define CS_FREE 	0x00
#define CS_RECV 	0x01
#define CS_REPLY	0x02
#define CS_SEND		0x03
#define CS_FLUSH	0x04	/* flush data and quit */


/* ----------------------------------------------------- */
/* AM : Agent Mode					 */
/* ----------------------------------------------------- */


#define	AM_MAILPOST	0x001
#define	AM_VALID	0x002
#define	AM_BBSADM	0x004
#define	AM_LOCAL	0x008	/* local saved post */
#define	AM_GEM		0x010
#define	AM_DROP		0x020	/* swallow command */
#define	AM_SWALLOW	0x040	/* swallow data */
#define	AM_DATA		0x080	/* data mode */

#define	AM_SPAM		0x100

#ifdef DEBUG
#define AM_DEBUG	0x200	/* for tracing malicious connection */
#endif

/* ----------------------------------------------------- */
/* operation log and debug information			 */
/* ----------------------------------------------------- */
/* @START | ... | time					 */
/* @CONN | [sno] ident | time				 */
/* ----------------------------------------------------- */


static FILE *flog;
static int gline;


#ifdef	WATCH_DOG
#define MYDOG	gline = __LINE__
#else
#define MYDOG			/* NOOP */
#endif


extern int errno;
extern char *crypt();


static void
logitvalid(file, key, userid,msg)
  char *file;
  char *key;
  char *userid;
  char *msg;
{
  time_t now;
  struct tm *p;
  char buf[100];

  time(&now);
  p = localtime(&now);
  sprintf(buf, "%02d/%02d/%02d %02d:%02d:%02d %s %-14s %s\n",
    p->tm_year % 100, p->tm_mon + 1, p->tm_mday,
    p->tm_hour, p->tm_min, p->tm_sec, key, userid, msg);
  f_cat(file, buf);
}

static void
log_fresh(fpath)
  char *fpath;
{
  int count;
  char fsrc[80], fdst[80];

  if (flog)
    fclose(flog);

  count = 9;
  do
  {
    sprintf(fdst, "%s.%d", fpath, count);
    sprintf(fsrc, "%s.%d", fpath, --count);
    rename(fsrc, fdst);
  } while (count);

  rename(fpath, fsrc);
  flog = fopen(fpath, "a");
}


static void
logit(key, msg)
  char *key;
  char *msg;
{
  time_t now;
  struct tm *p;

  time(&now);
  p = localtime(&now);
  /* Thor.990329: y2k */
  fprintf(flog, "%s\t%s\t%02d/%02d/%02d %02d:%02d:%02d\n",
    key, msg, p->tm_year % 100, p->tm_mon + 1, p->tm_mday,
    p->tm_hour, p->tm_min, p->tm_sec);

#ifdef DEBUG
  fflush(flog);
#endif
  
}


static void
log_open()
{
  FILE *fp;

  umask(077);

  if (fp = fopen(BMTA_PIDFILE, "w"))
  {
    fprintf(fp, "%d\n", getpid());
    fclose(fp);
  }

  flog = fopen(BMTA_LOGFILE, "a");
  logit("START", "MTA daemon");
}


static inline void
agent_log(ap, key, msg)
  Agent *ap;
  char *key;
  char *msg;
{
  fprintf(flog, "%s\t[%d] %s\n", key, ap->sno, msg);
#ifdef DEBUG
  fflush(flog);
#endif
}


static void
agent_reply(ap, msg)
  Agent *ap;
  char *msg;
{
  int cc;
  char *base, *head;

  head = base = ap->data;
  while (cc = *msg++)
  {
    *head++ = cc;
  }
  *head++ = '\r';
  *head++ = '\n';
  ap->used = head - base;
  ap->state = CS_SEND;
}


/* ----------------------------------------------------- */
/* server side routines 				 */
/* ----------------------------------------------------- */


static int
acct_fetch(userid, acct)
  char *userid;
  ACCT *acct;
{
  int fd;
  char fpath[80];

  /* Thor.990111: 不在 a-z是不合理的 */
  if( *userid < 'a' || *userid > 'z') 
    return -1;

  sprintf(fpath, "usr/%c/%s/.ACCT", *userid, userid);
  fd = open(fpath, O_RDWR, 0600);
  if (fd >= 0)
  {
    if (read(fd, acct, sizeof(ACCT)) != sizeof(ACCT))
    {
      close(fd);
      fd = -1;
    }
  }
  return fd;
}


/* ----------------------------------------------------- */
/* board：shm 部份須與 cache.c 相容			 */
/* ----------------------------------------------------- */


static BCACHE *bshm;


static void
attach_err(shmkey, name)
  int shmkey;
  char *name;
{
  fprintf(flog, "BSHM\t[%s error] key = %x\n", name, shmkey);
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


/* static */
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

    /* 等所有 boards 資料更新後再設定 uptime */

    time(uptime);
    return;
  }
}


static BRD *
brd_get(bname)
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


/* ----------------------------------------------------- */
/* board：shm 部份須與 cache.c 相容			 */
/* ----------------------------------------------------- */


static UCACHE *ushm;


/* static */
inline void
ushm_init()
{
  ushm = attach_shm(UTMPSHM_KEY, sizeof(UCACHE));
}


static inline void
bbs_biff(userno)
  int userno;
{
  UTMP *utmp, *uceil;
  usint offset;

  offset = ushm->offset;
  if (offset > (MAXACTIVE - 1) * sizeof(UTMP)) 
    offset = (MAXACTIVE - 1) * sizeof(UTMP);

  utmp = ushm->uslot;
  uceil = (void *) utmp + offset;

  do
  {
    if (utmp->userno == userno)
      utmp->ufo |= UFO_BIFF;
  } while (++utmp <= uceil);
}


/* ----------------------------------------------------- */
/* anti-spam						 */
/* ----------------------------------------------------- */


static HashTable *mrcpt_ht;
static HashTable *mhost_ht;
static HashTable *mfrom_ht;
static HashTable *title_ht;


#ifdef	CHECK_FORGE
static HashTable *forge_ht;


static int
is_forge(host)
  char *host;
{
  HashEntry *he;
  int score;
  unsigned long addr;

  he = ht_add(forge_ht, host);
  if ((score = he->score) == 0)
    he->uptime = addr = dns_addr(host);
  else
    addr = he->uptime;
  he->score = ++score;
  if (score == SPAM_FORGE_LIMIT)
  {
    fprintf(flog, "FORGE\t%s\n", host);
  }
  return (addr == INADDR_NONE);
}

#else

static int
is_forge(host)
  char *host;
{
  int cc;
  char *str;

  str = NULL;

  while (cc = *host)
  {
    host++;
    if (cc == '.')
      str = host;
  }

  return ((str == NULL) || (host - str <= 1));
}


#endif	/* CHECK_FORGE */


static void
spam_add(he)
  HashEntry *he;
{
  FILE *fp;

  if (fp = fopen(FN_ETC_SPAMER_ACL, "a"))
  {
    struct tm *p;

    p = localtime(&he->uptime);
    str_lower(he->key, he->key);
  /* Thor.990329: y2k */
    fprintf(fp, "%s # %d %02d/%02d/%02d %02d:%02d:%02d\n",
      he->key, he->score,
      p->tm_year % 100, p->tm_mon + 1, p->tm_mday,
      p->tm_hour, p->tm_min, p->tm_sec);
    fclose(fp);
  }

  he->score = 0;		/* release this hashing entry */
}


#if 0
static void
spam_list(ht, key)
  HashTable *ht;
  char *key;
{
  int i;
  HashEntry *he;
  FILE *fp;

  fp = flog;
  fprintf(fp, "\n[%s] %d entries\n\n", key, ht->tale);

  for (i = ht->mask; i >= 0; i--)
  {
    he = ht->bucket[i];
    while (he)
    {
      fprintf(fp, "%d\t%s\n", he->score, he->key);
      he = he->next;
    }
  }
}


static void
spam_fresh(now)
  time_t now;
{
  spam_list(mfrom_ht, "MAIL FROM");
  spam_list(title_ht, "Subject");
  spam_list(mhost_ht, "relay host");

#ifdef	CHECK_FORGE
  spam_list(forge_ht, "forged");
#endif

  spam_list(mrcpt_ht, "RCPT TO");
}
#endif


/* ----------------------------------------------------- */
/* statistics of visitor				 */
/* ----------------------------------------------------- */


#include "splay.h"


static int
vx_cmp(x, y)
 HashEntry *x;
 HashEntry *y;
{
  int dif;

  dif = y->visit - x->visit;
  if (dif)
    return dif;
  return str_cmp(x->key, y->key);
}


static void
vx_out(fp, top)
  FILE *fp;
  SplayNode *top;
{
  HashEntry *he;

  if (top == NULL)
    return;

  vx_out(fp, top->left);

  he = (HashEntry *) top->data;
#if 0
  if (he->visit <= LOWER_BOUND)
    return;
#endif

  fprintf(fp, "%6d %s\n", he->visit, he->key);

  vx_out(fp, top->right);
}


static void
splay_free(SplayNode *top)
{
  SplayNode *node;

  if (node = top->left)
    splay_free(node);

  if (node = top->right)
    splay_free(node);

  MYDOG;
  free(top);
  MYDOG;
}


static void
vx_log(fp, tag, ht)
  FILE *fp;
  char *tag;
  HashTable *ht;
{
  int i, nentry, nvisit;
  SplayNode *top;
  HashEntry *he;

  fprintf(fp, "[%s]\n\n", tag);
  
  top = NULL;
  nentry = 0;
  nvisit = 0;

  /* splay sort */

  for (i = ht->mask; i >= 0; i--)
  {
    for (he = ht->bucket[i]; he; he = he->next)
    {
      nentry++;
      nvisit += he->visit;
      top = splay_in(top, he, vx_cmp);
    }
  }

  /* report */

  fprintf(fp, "%d entry, %d visit:\n\n", nentry, nvisit);
  vx_out(fp, top);

  /* free memory */

  splay_free(top);

  for (i = ht->mask; i >= 0; i--)
  {
    for (he = ht->bucket[i]; he; he = he->next)
    {
      he->visit = 0;
    }
  }
}


static void
visit_fresh()
{
  char folder[80], fpath[80];
  int fd;
  FILE *fp;
  HDR hdr;

/* add by statue */
  char title[80];
  time_t now;
  struct tm ptime, *xtime;

  now = time(NULL);
  now -= 10 * 60;         /* back to ancent */
  xtime = localtime(&now);
  ptime = *xtime;
/* add by statue */

  strcpy(folder, "brd/"BRD_SECRET"/.DIR");
  if ((fd = hdr_stamp(folder, 'A', &hdr, fpath)) < 0)
    return;

  fp = fdopen(fd, "w");
  if (!fp)
  {
    close(fd);
    return;
  }

  vx_log(fp, "Host", mhost_ht);
  vx_log(fp, "From", mfrom_ht);
  vx_log(fp, "Rcpt", mrcpt_ht);
  vx_log(fp, "標題", title_ht);
  fclose(fp);
  close(fd);

  hdr.xmode = POST_MARKED;
  strcpy(hdr.owner, "<BMTA>");
  sprintf(title, "[記錄] [%2d 月 %2d 日] SPAM統計資料", ptime.tm_mon + 1, ptime.tm_mday);
  strcpy(hdr.title, title);
  rec_add(folder, &hdr, sizeof(HDR));
}


/* ----------------------------------------------------- */
/* memo of mail header / body				 */
/* ----------------------------------------------------- */


static void
mta_memo(ap, mark)
  Agent *ap;
  int mark;
{
  char folder[80], fpath[80], nick[80], *memo;
  int fd;
  FILE *fp;
  HDR hdr;

  memo = ap->nick;
  if (*memo)
    sprintf(nick, " (%s)", memo);
  else
    nick[0] = '\0';

  memo = ap->memo;
  if (!*memo)
  {
    memo = ap->fpath;
    if (!*memo)
    {
      memo = ap->title;
    }
  }

  strcpy(folder, "brd/junk/.DIR");
  if ((fd = hdr_stamp(folder, 'A', &hdr, fpath)) < 0)
  {
    agent_log(ap, "MEMO", "[junk] board error");
    return;
  }

  fp = fdopen(fd, "w");
  if (!fp)
  {
    close(fd);
    return;
  }

  fprintf(fp, "From: %s%s\nSubj: %s\nDate: %s"
    "Host: %s\nMemo: %s\nFile: %s\nSize: %d\n%s",
    ap->addr, nick, ap->title, ctime(&hdr.chrono),
    ap->ident, ap->memo, ap->fpath, ap->used, ap->data);
  fclose(fp);
  close(fd);

  if (mark)
    hdr.xmode = POST_MARKED;

  strcpy(hdr.owner, "<BMTA>");
  strcpy(hdr.title, memo);
  rec_add(folder, &hdr, sizeof(HDR));
  
}


/* ----------------------------------------------------- */
/* mailers						 */
/* ----------------------------------------------------- */

#if 0
static void
str_ansi(dst, str, max)		/* strip ANSI code */
  char *dst, *str;
  int max;
{
  int ch, ansi;
  char *tail;

  for (ansi = 0, tail = dst + max - 2; ch = *str; str++)
  {
    if (ch == '\n')
    {
      break;
    }
    else if (ch == 27)
    {
      ansi = 1;
    }
    else if (ansi)
    {
      if ((ch < '0' || ch > '9') && ch != ';' && ch != '[')
	ansi = 0;
    }
    else
    {
      *dst++ = ch;
      if (dst >= tail)
	break;
    }
  }
  *dst = '\0';
}
#endif

static int
is_ident(ident)
  char *ident;
{
  static char *invalid[] = {"bbs@", "@bbs", "unknown@", "root@", "gopher@",
  "guest@", "@ppp", "@slip", NULL};
  char buf[128], *str;
  int i;

  str_lower(buf, ident);
  for (i = 0; str = invalid[i]; i++)
  {
    if (strstr(buf, str))
      return 0;
  }
  return 1;
}


static int
bbs_mail(ap, data, userid)
  Agent *ap;
  char *data;
  char *userid;
{
  HDR hdr;
  int fd, method, sno;
  FILE *fp;
  char folder[80], buf[256], from[256], *author, *fpath, *title;
  struct stat st;

  fp = flog;
  sno = ap->sno;

  /* check if the userid is in our bbs now */

  sprintf(folder, "usr/%c/%s/.DIR", *userid, userid);
  if (!fstat((int) folder, &st) && st.st_size > MAX_BBSMAIL * sizeof(HDR))
  {
    fprintf(fp, "MAIL-\t[%d] <%s> over-spammed\n", sno, userid);
    return -1;
  }

  /* allocate a file for the new mail */

  fpath = ap->fpath;
  method = *fpath ? HDR_LINK : 0;
  if ((fd = hdr_stamp(folder, method, &hdr, fpath)) < 0)
  {
    fprintf(fp, "MAIL-\t[%d] <%s> stamp error\n", sno, userid);
    return -2;
  }

  author = ap->addr;
  title = ap->nick;
  sprintf(ap->memo, "%s -> %s", author, userid);

  if (*title)
    sprintf(from, "%s (%s)", author, title);
  else
    strcpy(from, author);

  title = ap->title;
  if (!method)
  {
    if (!*title)
      sprintf(title, "來自 %.64s", author);

    sprintf(buf, "作者: %.72s\n標題: %.72s\n時間: %s\n",
      from, title, ctime(&hdr.chrono));

    write(fd, buf, strlen(buf));
    write(fd, data, ap->data + ap->used - data);
    close(fd);
  }

  hdr.xmode = MAIL_INCOME;
  str_ncpy(hdr.owner, author, sizeof(hdr.owner));
  str_ncpy(hdr.title, title, sizeof(hdr.title));
  rec_add(folder, &hdr, sizeof(HDR));

  fprintf(fp, "MAIL\t[%d]\t%s\t%s\t%s/%s\n", sno, author, title, userid, hdr.xname);
  ap->xrcpt++;
  ap->xdata += ap->used;

  /* --------------------------------------------------- */
  /* 通知 user 有新信件					 */
  /* --------------------------------------------------- */

  sprintf(folder, "usr/%c/%s/.ACCT", *userid, userid);
  fd = open(folder, O_RDONLY);
  if (fd >= 0)
  {
    ACCT acct;
    /* int userno; */

    /* if ((read(fd, &userno, sizeof userno) == sizeof userno) && (userno > 0)) */
    if ((read(fd, &acct, sizeof(ACCT)) == sizeof(ACCT)) && (acct.userno > 0))
      bbs_biff(acct.userno);		/* lkchu.990428: 為了 EMAIL_PAGE */

#ifdef EMAIL_PAGE
    /* --------------------------------------------------- */
    /* E-Mail 網路傳呼	by lkchu@dragon2.net		   */
    /* --------------------------------------------------- */

    if (acct.ufo & UFO_MPAGER)
    {
      char *p;
      
      if ((p = str_str(acct.address, "bbc")) != NULL)  /* 找 BBC 描述 */
        DL_func("bin/emailpage.so:vaEMailPager", p + 3, author, title);
    }    
#endif
    close(fd);
  }
  
  mta_memo(ap, 0);
  return 0;
}


int
find_same_email(mail,mode)    /* mode : 1.find 2.add 3.del */
  char *mail;
  int mode;
{
  int pos=0,fd;
  char *fpath;
  SAME_EMAIL email;

  fpath = "etc/same_email.acl";

  if(mode >= 1 && mode <= 3)
  {
    fd = open(fpath,O_RDONLY);
    pos=0;
    while(fd)
    {
      lseek(fd, (off_t) (sizeof(SAME_EMAIL) * pos), SEEK_SET);
        if (read(fd, &email, sizeof(SAME_EMAIL)) == sizeof(SAME_EMAIL))
        {
          if(!strcmp(mail,email.email))
            break;
          pos++;
        }
        else
        {
          pos = -1;
          break;
        }
    }
    if(fd)
      close(fd);
  }
  if(mode == 1)
  {
    if(pos>=0)
      return email.num;
    else
      return 0;
  }
  if(mode == 2)
  {
    if(pos == -1)
    {
      memset(&email,0,sizeof(SAME_EMAIL));
      strcpy(email.email,mail);
      email.num = 1;
      rec_add(fpath,&email,sizeof(SAME_EMAIL));
    }
    else
    {
      email.num++;
      rec_put(fpath,&email,sizeof(SAME_EMAIL),pos);
    }
  }
  return 0;
}

static int
bbs_valid(ap)
  Agent *ap;
{
  int fd, sno;
  FILE *fp;
  char pool[256], buf[256], justify[128], *str, *ptr, *userid;
  ACCT acct;
  HDR hdr;

  fp = flog;
  sno = ap->sno;

  str = strstr(ptr = ap->title, TAG_VALID);
  if (!str)
  {
    sprintf(ap->memo, "REG : %.64s", ptr);
    fprintf(fp, "REG-\t[%d] %s\n", sno, ptr);
    return -2;
  }

  userid = pool;
  strcpy(userid, str + sizeof(TAG_VALID) - 1);
  if (!(str = strchr(userid, '(')))
    return -1;

  *str++ = '\0';

#if 0
  /* Thor.981228: 認證信不可為退信 */
  if (!*ap->from)
  {
    sprintf(ap->memo, "REG - %s (noFROM)", userid);
    fprintf(fp, "REG-\t[%d] <%s> noFROM \n", sno, userid);
    return -1;
  }
#endif

  if (!is_ident(ptr = ap->addr))
  {
    sprintf(ap->memo, "REG - %s (ident)", userid);
    fprintf(fp, "REG-\t[%d] <%s> ident %s\n", sno, userid, ptr);
    return -1;
  }

  if (!(ptr = (char *) strchr(str, ')')) || !strstr(ptr, "[VALID]"))
  {
    sprintf(ap->memo, "REG - %s (format)", userid);
    fprintf(fp, "REG-\t[%d] <%s> format\n", sno, userid);
    return -1;
  }

  *ptr++ = 0;

  str_lower(userid, userid);
  fd = acct_fetch(userid, &acct);
  if (fd < 0)
  {
    sprintf(ap->memo, "REG - %s (not exist)", userid);
    fprintf(fp, "REG-\t[%d] <%s> not exist\n", sno, userid);
    logitvalid(FN_VERIFY_LOG, "-BMTA Verify ERR-", acct.userid , acct.email);
    return -1;
  }

  if (str_hash(acct.email, acct.vtime) != chrono32(str - 1))
  {
    close(fd);
    sprintf(ap->memo, "REG - %s (checksum)", userid);
    fprintf(fp, "REG-\t[%d] <%s>  check sum: %s\n", sno, acct.userid, str);
    return -1;
  }

  ptr = ap->nick;
  if (*ptr)
    sprintf(justify, "%s (%s)", ap->addr, ptr);
  else
    strcpy(justify, ap->addr);
  str_ncpy(acct.justify, justify, sizeof(acct.justify));
  strcpy(acct.vmail, acct.email);
  if( find_same_email(acct.vmail,1) < MAX_REGIST)
  {  
    find_same_email(acct.vmail,2);
    acct.vtime = time(&acct.tvalid);
    acct.userlevel |= PERM_VALID;
  }
  lseek(fd, 0, SEEK_SET);
  /* write(fd, &acct, sizeof(ACCT)); */
  /* close(fd); */
  { /* Thor.990318: 查為什麼寫不進去 */
    int ret = write(fd, &acct, sizeof(ACCT));
    close(fd);
    if(ret < 0)
    {
      sprintf(ap->memo, "REG - %s (write fail)", userid);
      fprintf(fp, "REG-\t[%d] <%s>  write fail(%s) %s\n", sno, acct.userid, strerror(errno), justify);
      logitvalid(FN_VERIFY_LOG, "-BMTA Verify ERR-", acct.userid , acct.email);
      return -1;
    }
  }

  sprintf(buf, "usr/%c/%s/.DIR", *userid, userid);
  if (!hdr_stamp(buf, HDR_LINK, &hdr, FN_ETC_JUSTIFIED_BMTA))
  {
    strcpy(hdr.title, "您已經通過身分認證了！");
    strcpy(hdr.owner, "SYSOP");
    hdr.xmode = MAIL_NOREPLY;
    rec_add(buf, &hdr, sizeof(hdr));
  }

  /* Thor.990414: 加印 Timestamp, 方便追查 */
  fprintf(fp, "REG\t[%d] <%s> %s %s\n", sno, acct.userid, justify, str);
  sprintf(ap->memo, "REG + %s", userid);
  ap->xrcpt++;

  sprintf(buf, "usr/%c/%s/email", *userid, userid);
  if (fp = fopen(buf, "w"))
  {
    fprintf(fp, "ID: %s\nVALID: %s\nHost: %s\nFrom: %s\n%s\n",
      userid, justify, ap->ident, ap->addr, ap->data);
    fclose(fp);
    logitvalid(FN_VERIFY_LOG, "-BMTA Verify OK -", acct.userid , acct.email);
  }

  return 0;
}


static int
bbs_post(ap, data)
  Agent *ap;
  char *data;
{
  int cc, fd, mode, sno, fx;
  FILE *fp;
  char *str, key[80], buf[256], folder[128];
  char myuserid[80], mypasswd[80], myboard[80], mytitle[128];
  ACCT acct;
  HDR hdr;
  BRD *brd;

  mode = ap->mode;

  if (strstr(ap->title, TAG_VALID))
  {
    ap->mode = mode | AM_VALID;
    return bbs_valid(ap);
  }

  fp = flog;
  sno = ap->sno;
  sprintf(ap->memo, "%s =>", ap->addr);

  /* parse the mail post header */

  *myuserid = *mypasswd = *myboard = *mytitle = '\0';

  for (;;)  /* Thor.981104: 註解: for parse poster header */
  {
    cc = *data++;

    if (cc == '\n' || cc == '\0')
      break; /* Thor.981104: 註解: 本文開始或信件結束 */

    if (cc == '#')
    {
      str = key;

      for (;;)
      {
        cc = *data;

#if 0
        if (!cc)
          return -1;
#endif

        /*if (cc == ':' || cc == ' ' || cc == '\n') */
	if (!is_alpha(cc)) 
	  break;

	*str++ = cc;
        data++;
      }

      *str = '\0';  /* Thor.981104: parse 完 key */

      for (;;) /* Thor.981104: 註解: 略過所有的空白和: */
      {
        cc = *data;
        if (cc != ' ' && cc != ':')
          break;
        data++;
      }

      str = buf;

      for (;;) /* Thor.981104: 抓到換行符號為止 (^M(\r) for M$) */
      { /* Thor.990221:怪了, 在agent_recv有略過所有^M\r, 為何抓key還會出現^M? */
        /* if (cc == '\n' || cc == '\r' || cc == '\0') */
        /* Thor.981104: 印不出來的就不管了...:P */
        if (!isprint2((unsigned char) cc))
          break;
	*str++ = cc;
	cc = *++data;
      }

      *str = '\0';  /* Thor.981104: parse完 buf */

      while (*data && *data++ != '\n')
        ;
      /* Thor.981104: 讀完一行 */

      if (!str_cmp(key, "name") || !str_cmp(key, "userid"))
      {
	strcpy(myuserid, buf);
      }
      else if (!str_cmp(key, "passwd") || !str_cmp(key, "password") || !str_cmp(key, "passward"))
      {
	strcpy(mypasswd, buf);
      }
      else if (!str_cmp(key, "board"))
      {
	strcpy(myboard, buf);
      }
      else if (!str_cmp(key, "title") || !str_cmp(key, "subject"))
      {
	str_ansi(mytitle, buf, sizeof(mytitle));
      }
      else if (!str_cmp(key, "digest") || !str_cmp(key, "gem"))
      {
	mode |= AM_GEM;
      }
      else if (!str_cmp(key, "local"))
      {
	mode |= AM_LOCAL;
      }
    }
  }

  if (!*myuserid || !*mypasswd || !*myboard || !*mytitle || !cc)
  {
    fprintf(fp, "POST-\t[%d] <%s> err %s\n",
      sno, myuserid, ap->from);
    return -1;
  }

  /* check if the board is in our bbs now */

  if (!(brd = brd_get(myboard)))
  {
    fprintf(fp, "POST-\t[%d] <%s> No such board [%s]\n",
      sno, myuserid, myboard);
    return -1;
  }

  /* Thor.981101: 修正 myboard為真正board name, 以防不能post */
  strcpy(myboard, brd->brdname);
 

  /* check user account */

  str_lower(myuserid, myuserid);
  fd = acct_fetch(myuserid, &acct);
  if (fd < 0)
  {
    fprintf(fp, "POST-\t[%d] <%s> not exist\n", sno, myuserid);
    return -1;
  }

  /* check password */

  str = crypt(mypasswd, acct.passwd);
  if (strncmp(str, acct.passwd, PASSLEN))
  {
    close(fd);
    fprintf(fp, "POST-\t[%d] <%s> password incorrect\n", sno, myuserid);
    return -1;
  }

  /* check permission */

  cc = acct.userlevel;
  if ((cc & PERM_DENYPOST) || !(cc & PERM_POST))
  {
    close(fd);
    fprintf(fp, "POST-\t[%d] <%s> permission denied\n", sno, myuserid);
    return -1;
  }

  /* Thor.981026: 未具備 Internet 權限者，只能在站內發表文章 */

  if (!(cc & PERM_INTERNET))
    mode |= AM_LOCAL;


  if (mode & AM_GEM)
  {
    if (!(cc & (PERM_SYSOP | PERM_ALLBOARD)) && !str_has(brd->BM, myuserid))
    {
      close(fd);
      fprintf(fp, "POST-\t[%d] <%s> not [%s] BM\n", sno, myuserid, myboard);
      return -1;
    }
    sprintf(folder, "gem/brd/%s/.GEM", myboard);
  }
  else
  {
    cc = brd->battr;
    if (cc & BRD_NOTRAN)
      mode |= AM_LOCAL;

    /* if (cc & BRD_NOCOUNT == 0) */
    if (!(cc & BRD_NOCOUNT)) /* Thor.981123: 看來比較清楚 */
    {
      acct.numposts++;
      cc = 1;
    }
    else
    {
      cc = 0;
    }
    sprintf(folder, "brd/%s/.DIR", myboard);
  }

  if ((fx = hdr_stamp(folder, 'A', &hdr, ap->fpath)) < 0)
  {
    close(fd);
    fprintf(fp, "POST-\t[%d] <%s> err %s\n", sno, myuserid, folder);
    return -1;
  }

  str_ncpy(ap->title, mytitle, sizeof(ap->title));
  sprintf(buf, "作者: %s (%s)\n標題: %.72s\n時間: %s\n",
    acct.userid, acct.username, mytitle, ctime(&hdr.chrono));
  write(fx, buf, strlen(buf));
  write(fx, data, ap->data + ap->used - data);
#if 1
  sprintf(buf, "--\n\033[1;32m※ Origin: \033[33m%s \033[37m<%s> \033[m\n\033[1;31m◆ From: \033[36m%s\033[m\n",
    BOARDNAME, MYHOSTNAME, ap->addr);
#else
  sprintf(buf, "\n--\n※ Origin: %s<%s> ◆ Mail: %s\n",
    BOARDNAME, MYHOSTNAME, ap->addr);
#endif
  write(fx, buf, strlen(buf));
  close(fx);

  hdr.xmode = POST_INCOME;
  strcpy(hdr.owner, acct.userid);
  str_ncpy(hdr.title, mytitle, sizeof(hdr.title));
  rec_add(folder, &hdr, sizeof(HDR));

  if (cc)
  {
    lseek(fd, (off_t) 0, SEEK_SET);
    write(fd, &acct, sizeof(ACCT));
  }
  close(fd);

  fprintf(fp, "POST\t[%d] <%s> => [%s]\n", sno, myuserid, myboard);
  ap->xrcpt++;

  /* 轉信 */

  if (!(mode & (AM_GEM | AM_LOCAL)) && (fp = fopen("innd/out.bntp", "a")))
  {
    fprintf(fp, "%s\t%s\t%s\t%s\t%s\n",
      myboard, hdr.xname, hdr.owner, acct.username, mytitle);
    fclose(fp);
  }

  sprintf(ap->memo, "%s => [%s]", hdr.owner, myboard);

  return 0;
}


/* ----------------------------------------------------- */
/* mail header routines 				 */
/* ----------------------------------------------------- */


/* ----------------------------------------------------- */
/* From xyz Wed Dec 24 17:05:37 1997			 */
/* From: xyz (nick)					 */
/* From user@domain  Wed Dec 24 18:00:26 1997		 */
/* From: user@domain (nick)				 */
/* ----------------------------------------------------- */


static char *
mta_from(ap, str)
  Agent *ap;
  char *str;
{
  int cc;
  char pool[512], *head, *tail;

  head = pool;
  tail = head + sizeof(pool) - 1;

  for (;;)
  {
    /* skip leading space */

    while (*str == ' ')
      str++;

    /* copy the <From> to buffer pool */

    for (;;)
    {
      cc = *str;

      if (cc == '\0')
      {
	*head = '\0';
	sprintf(head, "%s %s", pool, ap->addr);
	agent_log(ap, "From:", head);
	return str;
      }

      str++;
/*
說明: mta_from 只抓取一行的資料來解出 addr 和 nick, 但是一行是不夠的, 因為
      沒有考慮到 \n\t 的特殊情形!
      
      舉例:
      
      From: =?big5?B?TG9ja2UgR2VyICi4r6vCqHw7qHy90KVbr/OmcsBZKQ==?=
               <locke@SoftChina.com.tw>
*/
      if (cc == '\n' && *str!='\t' && *str!=' ')
	break;

      *head++ = cc;

      if (head >= tail)
	return str;
    }

    if (*str != ' ')
      break;

    /* go on to merge multi-line <From> */
  }

  *head = '\0';

  str_from(pool, head = ap->addr, ap->nick);

  if(!(tail = strchr(head, '@')))
    return NULL;

  if (str_cmp(head, ap->from))
  {
  
    *tail++ = '\0';
    /* Thor.981223: 不擋 bbsreg和bbs, 將 untrust.acl分離出來 */
    /* if ( is_forge(tail) || acl_match(tail, head)) */
    if((ap->mode & AM_VALID) && acl_has(FN_ETC_UNTRUST_ACL ,head ,tail) > 0)
    {
      fprintf(flog, "UNTRUST\t[%d] %s\n", ap->sno, ap->from);
      return NULL;
    }
    
    if (!(ap->mode & (AM_VALID | AM_MAILPOST)) && (is_forge(tail) || acl_match(tail, head) || !str_cmp(tail,MYHOSTNAME)))
      return NULL;

    if(strchr(pool,' '))
      return NULL;

    tail[-1] = '@';
  }

  return str;
}


static char *
mta_xmailer(ap, str)
  Agent *ap;
  char *str;
{
  int cc;
  char pool[512], *head;

  head = pool;

  for (;;)
  {
    /* skip leading space */

    while (*str == ' ')
      str++;


    for (;;)
    {
      cc = *str;

      if (cc == '\0')
      {
	*head = '\0';
	return str;
      }

      str++;

      if (cc == '\n')
	break;

      *head++ = cc;

      if (head >= pool + sizeof(pool))
	return str;
    }

    if (*str != ' ')
      break;

  }

  *head = '\0';

  if (*pool)
  {
    if (mailer_match(pool))
      return NULL;
  }

  return str;
}


#if 0


/* ----------------------------------------------------- */
/* Return-Path: <root>		 			 */
/* Return-Path: <user@domain>				 */
/* ----------------------------------------------------- */


static char *
mta_reply(ap, str)
  Agent *ap;
  char *str;
{
  int cc;
  char *ptr, user[128], domain[128];

  ptr = user;

  while (cc = *str)
  {
    str++;

    /* skip leading space */

    if (cc == ' ' || cc == '<')
      continue;

    if (cc == '@')
    {
      *ptr = '\0';
      ptr = domain;
    }
    else if (cc == '>')
    {
      *ptr++ = '\0';
    }
    else if (cc == '\n')
    {
      if ( is_forge(domain) || acl_match(domain, user))
	return NULL;
      break;
    }
    else
    {
      *ptr++ = cc;
    }
  }

  return str;
}
#endif


static char *
mta_subject(ap, str)
  Agent *ap;
  unsigned char *str;
{
  int cc;
  char pool[640], *head, *tail; /*, *line ;  */

  head = pool;
  tail = head + sizeof(pool) - 128;

#if 1
  /* Thor.980831: modified for multi-line header */

  /* skip leading space */

  while (*str == ' ' || *str == '\n' || *str == '\t') str++;
  

  /* copy the <Subject> to buffer pool */

  for (;;)
  {
    cc = *str;

    if (cc == '\0')
    {
      sprintf(head, "%s %s", ap->from, ap->addr);
      agent_log(ap, "Subj:", pool /*head*/); /* Thor.980904:想看subj全貌 */
      return str;
    }

    str++;
       /* Thor.980906: |no body |header       |body seperator */
    if (cc == '\n' && (!*str || (*str != ' ' && *str != '\n' && *str != '\t')))
      break;

    *head++ = cc;

    if (head >= tail)		/* line too long */
    {
      agent_log(ap, "Subj:", pool); /* Xshadow.980906:really line too long ? */
      return str;
    }
  }

  *head = 0;

#ifdef  REJECT_GB_ENCODING
  if(!str_ncmp(pool,"=?GB2312?",9))
    return NULL;
#endif



  str_decode(pool);
#endif


#if 0
  for (;;)
  {
    /* skip leading space */

    while (*str == ' ')
      str++;

    /* copy the <Subject> to buffer pool */

    line = head;

    for (;;)
    {
      cc = *str;

      if (cc == '\0')
      {
	sprintf(head, "%s %s", ap->from, ap->addr);
	agent_log(ap, "Subj:", head);
	return str;
      }

      str++;
      *head++ = cc;
      if (cc == '\n')
	break;
    }

    str_decode(line);

    if (*str != ' ')
      break;

    /* go on to merge multi-line <Subject> */

    head = str_tail(line);

    if (head >= tail)		/* line too long */
      return str;
  }
#endif

  str_ansi(ap->title, pool, sizeof(ap->title));
  return str;
}


static inline int
is_almost(size, used)
  int *size;
  int used;
{
  int delta;

  delta = (*size) - used;
  *size = used;

  return (delta >= -16 && delta <= 16);
}


/* lkchu.981201: check for alias hosts */
static int
is_alias(from)
  char *from;
{
  int len;
  char *str;
  static char *alias[] = HOST_ALIASES;

  /* check the aliases */

  for (len = 0; str = alias[len]; len++)
  {
    if (*str && !str_ncmp(from, str, strlen(str) - 1))
      return 1;
  }
  return 0;
}

#ifdef	NO_SPAM
static int
is_no_spam(from)
  char *from;
{
  int len;
  char *str;
  static char *alias_nospam[] = NO_SPAM_HOST;

  for (len = 0; str = alias_nospam[len]; len++)
  {
    if (*str && !str_ncmp(from, str, strlen(str)))
      return 1;
  }
  return 0;
}
#else
static int
is_no_spam(from)
  char *from;
{
  return 0;
}
#endif


static char *
mta_to(ap, str)
  Agent *ap;
  char *str;
{
  int cc, found;
#if 0
  char pool[1024], *head, *tail;
  head = pool;
  tail = head + sizeof(pool) - 1;
#endif

  found = 0;

  for (;;)
  {
    cc = *str;

    if (cc == '\0')
    {
	fprintf(flog, "TO:\t[%d]\n", ap->sno);
	return str;
    }

    str++;

    if (cc == '@')
    {
      if (!found)
      {
	/* 有些 spammer 只假造 @host 部份 */
	/* 須是 user@host 而不是 @host 而已 */

	/* if (!str_ncmp(str, MYHOSTNAME, sizeof(MYHOSTNAME) - 1)) */
        if (is_alias(str))      /* lkchu.981201: for alias hostname */
	  found = str[-2] > ' ' ? 1 : -1;
      }
    }
    else if (cc == '\n')
    {
      if (*str != ' ')
	break;
    }

    /* go on to merge multi-line <To> */

#if 0
    /* skip leading space */

    while (*str == ' ')
      str++;

    /* copy the <To> to buffer pool */

    for (;;)
    {
      cc = *str;

      if (cc == '\0')
      {
	*head = pool[60] = '\0';
	fprintf(flog, "TO:\t[%d] %s\n", ap->sno, pool);
	return str;
      }

      str++;
      if (cc == '\n')
	break;

      if (cc >= 'A' && cc <= 'Z')
	cc += 0x20;

      *head++ = cc;

      if (head >= tail)
	return str;
    }

    if (*str != ' ')
      break;

    /* go on to merge multi-line <From> */
#endif
  }

  if (found > 0)
    return str;

  fprintf(flog, "TO-\t[%d]\n", ap->sno);
  return NULL;
}


/* Thor.980901: mail decode, only for ONE part, not for 
                 "This is a multi-part message in MIME format." */
static char *
mta_decode(ap, str, code)
  Agent *ap;
  unsigned char *str;
  char *code;
{
  /* skip leading space */
  while (*str == ' ') str++;

  *code = 0;
  if(!str_ncmp(str,"quoted-printable",16))
  {
    TRACE("DECODE", "quoted printable");

    *code = 'q';
  }
  else if (!str_ncmp(str,"base64",6))
  {
    TRACE("DECODE", "base64");
    *code = 'b';
  }
  
  /* skip whole line */
  while (*str && *str++ != '\n');

  return str;
}

static char *
mta_boundary(ap, str, boundary)
  Agent *ap;
  unsigned char *str;
  char *boundary;
{
  int cc;

#if 1
  char *base = boundary;
#endif

  *boundary = 0;
  /* skip leading space */
  while (*str == ' ') str++;

  if(!str_ncmp(str,"multipart",9))
  {
    char *tmp;
    
    if(tmp = str_str(str, "boundary="))
    {
#if 0
      tmp += 9 + 1 /* " */;
#endif
      /* Thor.990221: 居然有的人不用 " */
      tmp += 9;
      if(*tmp=='"') tmp++;

      while(*tmp && *tmp != '"' && *tmp !='\n')
        *boundary++=*tmp++;
      /* *boundary++ = '\n'; */ /* Thor.980907: 之後會被無意間跳過 */
      *boundary = 0;
    }
    TRACE("MULTI", base );
  }
  else if (!str_ncmp(str,"text",4))
  {
    char *tmp;

    if(!str_ncmp(str+4, "/html",5))
    {
      *base = 0;
      return NULL;
    }
#ifdef REJECT_GB_ENCODING
    else if(tmp = str_str(str, "charset="))
    {
      char buf2[512];
      char *ptr;
      ptr = buf2;
      
      tmp += 8;
      if(*tmp=='"') tmp++;

      while(*tmp && *tmp != '"' && *tmp !='\n')
        *ptr++=*tmp++;
      *ptr = 0;

      if(!str_ncmp(buf2,"GB2312",6))
      {
        *base = 0;
        return NULL;
      }
    }
#endif  
    /* TRACE("TEXT", base ); */
  }
  
  while (cc = *str)
  { 
    str ++;
    if (cc == '\n' && (!*str || *str > ' ' || *str == '\n'))
      break;
  }

  return str;
}

static char *
mail_boundary(ap, str, boundary)
  Agent *ap;
  unsigned char *str;
  char *boundary;
{
  int cc;

#if 1
  char *base = boundary;
#endif

  *boundary = 0;
  /* skip leading space */
  while (*str == ' ') str++;

  if(!str_ncmp(str,"multipart",9))
  {
    char *tmp;
    
    if(tmp = str_str(str, "boundary="))
    {
#if 0
      tmp += 9 + 1 /* " */;
#endif
      /* Thor.990221: 居然有的人不用 " */
      tmp += 9;
      if(*tmp=='"') tmp++;

      while(*tmp && *tmp != '"' && *tmp !='\n')
        *boundary++=*tmp++;
      /* *boundary++ = '\n'; */ /* Thor.980907: 之後會被無意間跳過 */
      *boundary = 0;
    }
    TRACE("MULTI", base );
  }
  
  while (cc = *str)
  { 
    str ++;
    if (cc == '\n' && (!*str || *str > ' ' || *str == '\n'))
      break;
  }

  return str;
}


/* Thor.980907: support multipart mime */
/* ----------------------------------------------------- */
/* multipart decoder	 				 */
/* ----------------------------------------------------- */
static int
multipart(src,dst,boundary) 
  unsigned char *src;
  unsigned char *dst;      /* Thor: no ending 0 */
  unsigned char *boundary; /* Thor: should include "--" */
{
  unsigned char *base = dst;
  char *bound;
  unsigned char *tmp;
  char decode = 0;
  char buf[512] = "--"; /* Thor: sub mime boundary */
  int cc;
  int boundlen = strlen(boundary);
  /* if (!*boundary) return 0; */
  if(boundlen < 6) return 0; /* Thor: boundary too small, "\n>  <\n" */

  while(*src)
  {

    bound = strstr(src, boundary);

    if(bound) *bound = 0; 

    if(buf[2])  /* Thor: multipart & encoded can't be happened simutaneously */
    {
       cc = multipart(src,dst,buf);
       /* buf[2] = 0; */
       if(cc > 0)
         dst += cc;
       else
         goto bypass;
    }
    else if (decode) 
    {
      cc = mmdecode2(src,decode,dst);
      /* decode = 0; */
      if(cc > 0)
        dst += cc;
      else
        goto bypass;
    }
    else
    {
bypass:
      while(*src)
        *dst++=*src++;
    }
#if 1 /* Thor: reset */
    buf[2] = 0;
    decode = 0;
#endif


    if(!bound) break;

    src = bound + boundlen; /* Thor: src over boundary */

    tmp = dst + boundlen - 3 /* " <\n" */;

    *dst++ = '\n';
    *dst++ = '>';
    *dst++ = ' ';

    while(dst < tmp) 
      *dst++ = '-';

    *dst++ = ' ';
    *dst++ = '<';
    *dst++ = '\n';
  
    for(;;)  /* Thor: processing sub-header */
    {
      if (!str_ncmp(src, "Content-Transfer-Encoding:",26))
      { /* Thor.980901: 解 rfc1522 body code */
        src = mta_decode(NULL, src + 26, &decode);
      }
      else if(!str_ncmp(src,"Content-Type:",13))
      {
        src = mail_boundary(NULL, src + 13, buf + 2);
      }
      else 
      {
        while (cc = *src)
        /* Thor.980907: 剛好跳了boundary後的 \n,不是故意的 :p */
        {
	  src++;
	  if (cc == '\n')
	    break;
        }
      }
      if (!src || !*src || *src == '\n')
        break;			/* null body or end of mail header */
    }
  
  }
  return dst - base;
}

/* ----------------------------------------------------- */
/* mailer 						 */
/* ----------------------------------------------------- */

static int
mta_mailer(ap)
  Agent *ap;
{
  char *data, *str, *addr, *delimiter, decode = 0;
  char boundary[512] = "--";
  int cc, mode;
  RCPT *rcpt;
  HashEntry *he, *hx;
  time_t uptime;

#define	MTA_CMP(x, y)	str_ncmp(x, y, sizeof(y) - 1)

  mode = ap->mode;
  data = ap->data + 1;		/* skip leading stuff */

  /* --------------------------------------------------- */
  /* parse the mail header and filter spam mails	 */
  /* --------------------------------------------------- */

  for (;;)
  {
    if (!MTA_CMP(data, "From"))
    {
      data += 5;

#if 0
      /* 攔截 MAILDER-DAEMON 之問題信件，以便追蹤 */

      if (str_str(data, "mailer-daemon"))
      {
	/* JUNK mail */

	strcpy(ap->memo, "JUNK");
	mta_memo(ap, 0);
	return 0;
      }

      /* 若不攔則可退信給使用者 */
#endif

      data = mta_from(ap, data);
      if (!data)
      {
	agent_reply(ap, "550 spam mail");
	return -1;
      }
    }
    else if (!MTA_CMP(data, "Subject:"))
    {
      data = mta_subject(ap, data + 8);
      if (!data)
      {
	agent_reply(ap, "550 spam mail");
	return -1;
      }
    }
    else if (!MTA_CMP(data, "X-Mailer:"))
    {
      data = mta_xmailer(ap, data + 9);
      if (!data)
      {
	agent_reply(ap, "550 spam mail");
	return -1;
      }
    }
    else if (!MTA_CMP(data, "To:"))
    {
      data = mta_to(ap, data + 3);
      if (!data)
      {
	agent_reply(ap, "550 fake mail");
	return -1;
      }
    }

#if 0
    else if (!MTA_CMP(data, "Return-Path:"))
    {
      data = mta_reply(ap, data + 12);
      if (!data)
      {
	agent_reply(ap, "550 spam mail");
	return -1;
      }
    }
    else if (!MTA_CMP(data, "Reply-To:"))
    {
      data = mta_reply(ap, data + 9);
      if (!data)
      {
	agent_reply(ap, "550 spam mail");
	return -1;
      }
    }
#endif
    else if (!MTA_CMP(data, "Content-Transfer-Encoding:"))
    { /* Thor.980901: 解 rfc1522 body code */
      data = mta_decode(ap, data + 26, &decode);
#if 0  /* Thor.980907: 不會發生 */
      if (!data)
      {
	agent_reply(ap, "550 error encoded mail");
	return -1;
      }
#endif
    }
    else if(!MTA_CMP(data,"Content-Type:"))
    { /* Thor.980907: 解 multi-part */
      data = mta_boundary(ap, data + 13, boundary + 2);
      if (!data)
      {
	agent_reply(ap, "550 error content-type mail");
	return -1;
      }
    }

    /* skip this line */

    else
    {
#if 0  /* Thor.980902: 計算 multipart使用量 */
      /* if(!MTA_CMP(data,"Content-Type: multi")) */
      if(boundary[2])
      {
        TRACE("MULTI",data);
      }
#endif
      for (;;)
      {
	cc = *data;

	if (cc == '\0')		/* null body */
	{
	  data--;
	  goto mta_mail_body;
	  /* return 0; */
	}

	data++;

	if (cc == '\n')
	  break;
      }
    }

    if (*data == '\n')
      break;			/* end of mail header */
  }

  /* --------------------------------------------------- */
  /* process the mail body				 */
  /* --------------------------------------------------- */

mta_mail_body:

  addr = ap->addr;
  if (!*addr)
  {
    str = ap->from;
    cc = *str;
    if (cc == ' ' || cc == '\0')
    {
      strcpy(ap->memo, "FROM: null");
      mta_memo(ap, 0);
      return 0;
    }
    strcpy(addr, str);
  }

  if (mode & AM_BBSADM)
  {
    sprintf(ap->memo, "ADM: %.64s", ap->from);
    /* mta_memo(ap, 0); */ /* Thor.980730: mark起來的確比較明顯方便 */
    mta_memo(ap, 1);            /* lkchu: mark ADM's letter */
 
    return 0;
  }

  *data = '\0';

  /* --------------------------------------------------- */
  /* validate user's character				 */
  /* --------------------------------------------------- */

  rcpt = ap->rcpt;

  if (mode & AM_VALID)
  {
    char user[80],*host;
    strcpy(user,ap->from);
    if((host = strchr(user,'@')) != NULL)
      *host++ = '\0';

    if(!host || acl_has(FN_ETC_UNTRUST_ACL ,user ,host) > 0)
    {
      fprintf(flog, "UNTRUST\t[%d] %s\n", ap->sno, ap->from);
       return 0;
    }          
  
    if (bbs_valid(ap) == -2)
      *data = '\n';

    mta_memo(ap, 0);

    if ((rcpt == NULL) && !(mode & AM_MAILPOST))
      return 0;
  }

  /* --------------------------------------------------- */
  /* check mail body					 */
  /* --------------------------------------------------- */

  delimiter = data;

  for (;;)
  {
    cc = *++data;
    if (cc == '\0')		/* null mail body */
      return 0;

    if (cc != '\n')		/* skip empty lines in mail body */
      break;
  }

  /* --------------------------------------------------- */
  /* decode mail body					 */
  /* --------------------------------------------------- */
  /* Thor.980901: decode multipart body */
  if (boundary[2])
  {
    /* TRACE("BOUNDARY",boundary); */
    /* TRACE("MULTIDATA",data); */
    cc = multipart(data,data,boundary);
    if(cc > 0)
      ap->used = (data - ap->data) + cc;
                /**header 長度****/ /*信長*/
  }
  /* Thor.980901: decode mail body */
  else if (decode) 
  {
    /* data[mmdecode2(data,decode,data)]=0; */
    /* Thor.980901: 因 decode必為 b or q, 故 mmdecode不為-1, 必定成功,
                    又因 mmdecode不自動加 0, 故手動加上 */

    /* Thor.980901: 不用 0 作結束, 而用長度, 計算方式如下 
                     write(fd, data, ap->data + ap->used - data);
                    故修改 ap->used, 以截掉過長 data */
    cc = mmdecode2(data,decode,data); 
    if(cc > 0) 
      ap->used = (data - ap->data) + cc;
                /**header 長度****/ /*信長*/

    /* TRACE("DECODEDATA", data); */
  }
  
  /* --------------------------------------------------- */
  /* check E-mail address for anti-spam first		 */
  /* --------------------------------------------------- */

  uptime = ap->uptime;

  addr = ap->addr;
  /* visor : no spam (is_no_spam) 20000727 */
  if ((str = strchr(addr, '@')) && str_ncmp("mailer-daemon@", addr, 14) && !is_no_spam(str+1))
  {
    int aScore, hScore;

    he = ht_add(mfrom_ht, addr);
    he->uptime = uptime;

    aScore = ap->nrcpt;
    if (aScore > 0)
    {
      hScore = aScore;
      hx = ht_add(title_ht, str_ttl(ap->title));
      hx->uptime = uptime;
      hx->visit += aScore - 1;
      hx->score += aScore;
      if (hx->score >= SPAM_TITLE_LIMIT)
      {
	fprintf(flog, "TITLE\t[%d] %s\n", ap->sno, ap->title);
	aScore += aScore;
      }

      if (is_almost(hx->xyz, ap->used))	/* 標題雷同、長度亦類似 */
      {
	aScore += SPAM_MFROM_LIMIT >> 4;
      }

      if (he->xyz == hx)	/* title / from 皆同 */
      {
	aScore += SPAM_MFROM_LIMIT >> 3;
      }
      else
      {
	he->xyz = hx;		/* title_ht 指向目前之 From */
      }
    }
    else
    {
      hScore = aScore = 1;
    }

    if ((he->score += aScore) >= SPAM_MFROM_LIMIT)
    {
      acl_add(addr);
      spam_add(he);
      fprintf(flog, "SPAM-F\t[%d] %s\n", ap->sno, addr);

      sprintf(ap->memo, "SPAM : %s", addr);
      *delimiter = '\n';
    }

    /* ------------------------------------------------- */

    he = ht_add(mhost_ht, ++str);
    he->uptime = uptime;

    if ((he->score += hScore) >= SPAM_MHOST_LIMIT)
    {
      acl_add(str);
      spam_add(he);
      fprintf(flog, "SPAM-H\t[%d] %s\n", ap->sno, str);

      sprintf(ap->memo, "SPAM : %s", str);
      *delimiter = '\n';
    }

    if (*delimiter)
    {
      mta_memo(ap, 0);
      return 0;
    }
  }

  /* --------------------------------------------------- */
  /* mail post						 */
  /* --------------------------------------------------- */

  if (mode & AM_MAILPOST)
  {
    bbs_post(ap, data);
    mta_memo(ap, 0);
  }

  if (rcpt)
  {
    char *dot;

    addr = ap->addr;
    if (dot = strchr(addr, '.'))
    {
      if (!str_cmp(dot, ".bbs@" MYHOSTNAME))
	*dot = '\0';
      else
	dot = NULL;
    }

    do
    {
      str = rcpt->userid;
      bbs_mail(ap, data, str);
    } while (rcpt = rcpt->rnext);

    if (dot)
      *dot = '.';
  }

  return 0;
}


/* ----------------------------------------------------- */
/* command dispatch					 */
/* ----------------------------------------------------- */


static void
agent_free_rcpt(ap)
  Agent *ap;
{
  RCPT *rcpt, *next;

  ap->nrcpt = 0;

  if (rcpt = ap->rcpt)
  {
    ap->rcpt = NULL;
    do
    {
      next = rcpt->rnext;
      MYDOG;
      free(rcpt);
      MYDOG;
    } while (rcpt = next);
  }
}


static void
agent_spam(ap)
  Agent *ap;
{
  int cc;
  char *from;
  FILE *fp;

  from = ap->from;
  cc = *from;
  if (cc == ' ' || cc == '\0')
    return;

  acl_add(from);

  if (fp = fopen(FN_ETC_SPAMER_ACL, "a"))
  {
    struct tm *p;

    p = localtime(&ap->uptime);
  /* Thor.990329: y2k */
    fprintf(fp, "%s # 100 %02d/%02d/%02d %02d:%02d:%02d\n",
      from,
      p->tm_year % 100, p->tm_mon + 1, p->tm_mday,
      p->tm_hour, p->tm_min, p->tm_sec);
    fclose(fp);
  }
}


/* ----------------------------------------------------- */
/* command dispatch					 */
/* ----------------------------------------------------- */


static void
cmd_what(ap)
  Agent *ap;
{
  ap->xerro++;
  agent_reply(ap, "500 Command unrecognized");
}


static void
cmd_help(ap)
  Agent *ap;
{
  agent_reply(ap, "\
214-Commands:\r\n\
214-	HELO	MAIL	RCPT	DATA\r\n\
214-	NOOP	QUIT	RSET	HELP\r\n\
214-See RFC-821 for more info.\r\n\
214 End of HELP info");
}


static void
cmd_noop(ap)
  Agent *ap;
{
  agent_reply(ap, "250 OK");
}


static void
agent_reset(ap)
  Agent *ap;
{
  ap->mode = 0;
  ap->memo[0] = '\0';
  ap->fpath[0] = '\0';
  ap->from[0] = '\0';
  ap->title[0] = '\0';
  ap->addr[0] = '\0';
  ap->nick[0] = '\0';
  agent_free_rcpt(ap);
}


static void
cmd_rset(ap)
  Agent *ap;
{
  agent_reset(ap);
  agent_reply(ap, "250 Reset state");
}


/* ----------------------------------------------------- */
/* 0 : OK , -1 : error, -2 : <>, 1 : relayed		 */
/* ----------------------------------------------------- */


static int
parse_addr(addr, user, domain)
  char *addr, **user, **domain;
{
  int ch, relay;
  char *ptr, *str;

  /* <[@domain_list:]user@doamin> */

  addr = strchr(addr, '<');
  if (!addr)
    return -1;

  ptr = strrchr(++addr, '>');
  if (!ptr)
    return -1;

  if (ptr == addr)
    return -2;			/* <> null domain */

  *ptr = '\0';
  if (ptr = strrchr(addr, ':'))
  {
    relay = 1;
    addr = ptr + 1;
  }
  else
  {
    relay = 0;
  }

  /* check the E-mail address format */

  *user = str = addr;

  for (ptr = NULL; ch = *addr; addr++)
  {
    if (ch == '@')
    {
      if (ptr)
	return -1;

      ptr = addr;
      continue;
    }

    if (!is_alnum(ch) && !strchr(".-_[]%!", ch))
      return -1;
  }

  if (!ptr)
    return -1;

  *ptr++ = '\0';
  *domain = ptr;

  if (!*str)			/* more ... */
  {
    return -1;
  }

  return relay;
}


static void
cmd_mail(ap)
  Agent *ap;
{
  char *data, *from, *user, *domain, *ptr;
  int cc;

  from = ap->from;
  if (*from)
  {
    ap->xerro++;
    agent_reply(ap, "503 Sender already specified");
    return;
  }

  /* mail from:<[@domain_list:]user@doamin> */

  data = ap->data;
  cc = parse_addr(data, &user, &domain);

  if (cc)
  {
    if (cc == -2)		/* null domain */
    {
      ap->xerro++;
      agent_reply(ap, "550 null domain");
      return;  /* Thor.980816: 擋null domain */
#if 0
      from[0] = ' ';
      from[1] = '\0';

      strcpy(data, "250 Sender ok\r\n");
      ap->used = strlen(data);
      ap->state = CS_SEND;
      return;
#endif
    }

    ap->xerro++;
    agent_reply(ap, cc < 0 ? "501 Syntax error" :
      "551 we dont accept relayed mail");
    return;
  }

  ptr = data + MAX_CMD_LEN;
  sprintf(ptr, "%s@%s", user, domain);
  ht_add(mfrom_ht, ptr);

#ifdef	NO_SPAM
  if ( (is_forge(domain) || acl_match(domain, user)) && !is_no_spam(domain))
#else
  if (is_forge(domain) || acl_match(domain, user))
#endif
  {
    ap->xspam++;

    ap->mode |= AM_SPAM;
    agent_log(ap, "SPAM-M", ptr);

#if 0
    agent_log(ap, "DENY", ptr);
    agent_reply(ap, "550 Access denied");
    return;
#endif
  }

  str_ncpy(from, ptr, sizeof(ap->from));
  sprintf(data, "250 <%s> Sender ok\r\n", ptr);
  ap->used = strlen(data);
  ap->state = CS_SEND;
}


static int
is_rcpt(rcpt)
  char *rcpt;
{
  int len;
  char *str, fpath[80];
  struct stat st;
#if 0
  static char *alias[] = {"root", "mailer-daemon", "postmaster",
  "bbsadm", "chuan", "thor", "opus", NULL};
#endif  /* Thor.980730: 集中定義於最前 */

  static char *alias[] = ADM_ALIASES;

  /* Thor.990411: sendmail.reject暴力法 */
  static char *reject[] = REJECT;

  if (!str_cmp(rcpt, "bbsreg"))
    return AM_VALID;

  if (!str_cmp(rcpt, "bbs"))
    return AM_MAILPOST;

  /* check the aliases */

  for (len = 0; str = alias[len]; len++)
  {
    if (!str_cmp(rcpt, str))
      return AM_BBSADM;
  }

  /* check the users */

  len = strlen(rcpt);
  if (len > 4 && len <= IDLEN + 4)
  {
    str = rcpt + len - 4;
    if (!str_cmp(str, ".bbs"))
    {
      *str = '\0';
      str_lower(rcpt, rcpt);
#if 1
      /* Thor.990411: sendmail.reject暴力法 */
      for (len = 0; str = reject[len]; len++)
      {
        if (!strcmp(rcpt, str)) /* Thor.990411: 注意, 全小寫 */
          return -1;
      }
#endif
      sprintf(fpath, "usr/%c/%s/@", *rcpt, rcpt);
      if (!stat(fpath, &st) && S_ISDIR(st.st_mode))
	return 0;
    }
  }

  return -1;
}


static void
cmd_rcpt(ap)
  Agent *ap;
{
  char *data, *user, *domain;
  int cc;
  RCPT *rcpt;

  if (!ap->from[0])
  {
    ap->xerro++;
    agent_reply(ap, "503 MAIL first");
    return;
  }

#if 0
  /* Thor.981223: 不擋 bbsreg, 將 untrust.acl 分離出來 */
  if (ap->mode & AM_SPAM)
  {
    agent_reply(ap, "550 no spam mail");
    return;
  }
#endif

  if (ap->nrcpt > MAX_RCPT)
  {
    /* maybe spammer */

    ap->mode |= AM_SPAM;
    ap->xspam += ap->nrcpt;
    agent_spam(ap);
    agent_log(ap, "SPAM-R", "too many recipients");
    agent_reply(ap, "552 Too many recipients");
    return;
  }

  /* rcpt to:<[@domain_list:]user@doamin> */

  data = ap->data;
  cc = parse_addr(data, &user, &domain);
  if (cc)
  {
    ap->xerro++;
    agent_reply(ap, cc < 0 ? "501 Syntax error" :
      "551 we dont accept relayed mail");
    return;
  }

  if (domain == NULL)
  {
    agent_reply(ap, "550 null domain");
    return;
  }

  /* 只接受 FQDN 不接受數字格式 domain name */

  /* if (str_cmp(domain, MYHOSTNAME)) */
  if (!is_alias(domain))        /* lkchu.981201: for alias hostname */
  {
    ap->xerro++;
    agent_reply(ap, "550 we dont relay mail");
    return;
  }

  ht_add(mrcpt_ht, user);

  cc = is_rcpt(user);
  if (cc < 0)
  {
    ap->xerro++;
    agent_reply(ap, "550 no such user");
    return;
  }

  if (cc)
  {
    ap->mode |= cc;
  }
  else
  {
#if 1
    /* Thor.981227: 確定不是 bbsreg才擋，delay擋連線 */
    /* format: sprintf(ident = servo_ident, "[%d] %s ip:%08x", ++servo_sno, rhost, csin.sin_addr.s_addr); */
    char rhost[256], *s;
    s = strstr(ap->ident," ");
    strcpy(rhost,s+1);
    if(s=strstr(rhost," ")) *s=0;
    if (acl_match(rhost, "*")) 
    { 
      char buf[256]; 
 
      /* sprintf(buf, "421 %s deny %s\r\n", MYHOSTNAME, ident);  */
      sprintf(buf, "421 %s deny %s\r\n", MYHOSTNAME, ap->ident); 
      agent_reply(ap, buf);
      /* send(csock, buf, strlen(buf), 0); */ /* Thor.981206: 補個0上來 */ 
      /* shutdown(csock, 2);  */
      /* close(csock);  */
      ap->mode |= AM_SPAM;
      /* logit("DENY", ident);  */
      agent_log(ap, "DENY", ap->ident);
      return /*-1*/; 
    } 
#endif
#if 1
    /* Thor.981223: 確定不是 bbsreg才擋，將 untrust.acl分離出來 */
    if (ap->mode & AM_SPAM)
    {
      MYDOG;
      agent_reply(ap, "550 no spam mail");
      MYDOG;
      return;
    }
#endif
    ap->nrcpt++;
    cc = strlen(user) + 1;
    MYDOG;
    rcpt = (RCPT *) malloc(sizeof(RCPT) + cc);
    MYDOG;
    if(!rcpt) /* Thor.990205: 記錄空間不夠 */
      TRACE("ERROR","Not enough space in cmd_rcpt()");

    rcpt->rnext = ap->rcpt;
    memcpy(rcpt->userid, user, cc);
    ap->rcpt = rcpt;
  }

  agent_reply(ap, "250 Recipient ok");
}


static void
cmd_helo(ap)
  Agent *ap;
{
  char *data;

  data = ap->data;
  sprintf(data, "250 Hello %s\r\n", ap->ident);
  ap->used = strlen(data);
  ap->state = CS_SEND;
}


#if 0
static void
cmd_ehlo(ap)
  Agent *ap;
{
  char *data;

  data = ap->data;
  sprintf(data, "250-Hello %s\r\n250 8BITMIME\r\n", ap->ident);
  ap->used = strlen(data);
  ap->state = CS_SEND;
}
#endif


static void
cmd_data(ap)
  Agent *ap;
{
  int mode;

  mode = ap->mode;
  if (!ap->rcpt && !(mode & (AM_MAILPOST | AM_VALID | AM_BBSADM)))
  {
    ap->xerro++;
    agent_reply(ap, "503 RCPT first");
    return;
  }

  ap->mode = mode | AM_DATA;
  agent_reply(ap, "354 go ahead");
}


static void
cmd_quit(ap)
  Agent *ap;
{
  char *data;

  data = ap->data;
  strcpy(data, "221 bye\r\n");
  ap->used = strlen(data);
  ap->state = CS_FLUSH;
}


static void
cmd_nogo(ap)
  Agent *ap;
{
  agent_reply(ap, "502 operation not allowed");
  return;
}


static Command cmd_table[] =
{
  cmd_helo, "helo", "HELO <hostname> - Introduce yourself",
#if 0
  cmd_ehlo, "ehlo", "EHLO <hostname> - Introduce yourself",
#endif
  cmd_nogo, "ehlo", "", /* Thor.980929: 不支援enhanced smtp */
  cmd_mail, "mail", "MAIL FROM: <sender> - Specifies the sender",
  cmd_rcpt, "rcpt", "RCPT TO: <recipient> - Specifies the recipient",
  cmd_data, "data", "",
  cmd_quit, "quit", "QUIT -     Exit sendmail (SMTP)",
  cmd_noop, "noop", "NOOP -     Do nothing",
  cmd_rset, "rset", "RSET -     Resets the system",
  cmd_help, "help", "HELP [ <topic> ] - gives help info",
  cmd_nogo, "expn", "",
  cmd_nogo, "vrfy", "",
  cmd_what, NULL, NULL
};


/* ----------------------------------------------------- */
/* send output to client				 */
/* ----------------------------------------------------- */
/* return value :					 */
/* > 0 : bytes sent					 */
/* = 0 : close this agent				 */
/* < 0 : there are some error, but keep trying		 */
/* ----------------------------------------------------- */


static int
agent_send(ap)
  Agent *ap;
{
  int csock, len, cc;
  char *data;

  csock = ap->sock;
  data = ap->data;
  len = ap->used;
  cc = send(csock, data, len, 0);

#ifdef AM_DEBUG
  if(ap->mode & AM_DEBUG)
  {
    char buf[1024];

    sprintf(buf,"%s\t[%d]\tbmtad>>>\n",Now(),ap->sno);
    f_cat("run/bmta.debug",buf);
    str_ncpy(buf,data,len+1);
    f_cat("run/bmta.debug",buf);
    sprintf(buf,"\t[%d]\tbmtad<<<\n",ap->sno);
    f_cat("run/bmta.debug",buf);
  }
#endif
  if (cc < 0)
  {
    cc = errno;
    if (cc != EWOULDBLOCK)
    {
      agent_log(ap, "SEND", strerror(cc));
      return 0;
    }

    /* would block, so leave it to do later */
    return -1;
  }

  if (cc == 0)
    return -1;

  len -= cc;
  ap->used = len;
  if (len)
  {
    memcpy(data, data + cc, len);
    return cc;
  }

  if (ap->state == CS_FLUSH)
  {
    shutdown(csock, 2);
    close(csock);
    ap->sock = -1;
    return 0;
  }

  ap->state = CS_RECV;
  return cc;
}


/* ----------------------------------------------------- */
/* receive request from client				 */
/* ----------------------------------------------------- */


static int
agent_recv(ap)
  Agent *ap;
{
  int cc, mode, size, used;
  char *data, *head, *dest;

  mode = ap->mode;
  used = ap->used;
  data = ap->data;

  if (mode & AM_DATA)
  {
    if (used <= 0)
    {
      /* pre-set data */

      *data = '\n';
      used = 1;
    }
    else
    {
      /* check the available space */

      size = ap->size;
      cc = size - used;

      if (cc < TCP_RCVSIZ + 3)
      {
	if (size < MAX_DATA_SIZE)
	{
	  size += TCP_RCVSIZ + (size >> 2);

	  if (data = (char *) realloc(data, size))
	  {
	    ap->data = data;
	    ap->size = size;
	  }
	  else
	  {
	    fprintf(flog, "ERROR\t[%d] malloc: %d\n", ap->sno, size);
#ifdef DEBUG
  	    fflush(flog);
#endif
	    return 0;
	  }
	}
	else
	{
	  /* DATA 太長了，通通吃下來，並假設 mail header 不會超過 HEADER_SIZE */

#define HEADER_SIZE   8192
	  data[HEADER_SIZE - 2] = data[used - 2];
	  data[HEADER_SIZE - 1] = data[used - 1];
	  used = HEADER_SIZE;
#undef	HEADER_SIZE
	  ap->mode = (mode |= AM_SWALLOW);
	  ap->xerro++;
	  fprintf(flog, "ERROR\t[%d] data too long ip:%08x\n", ap->sno,ap->ip_addr);
#ifdef DEBUG
          fflush(flog);
#endif
	}
      }
    }
  }

  head = data + used;
  cc = recv(ap->sock, head, TCP_RCVSIZ, 0);

  if (cc <= 0)
  {
    cc = errno;
    if (cc != EWOULDBLOCK)
    {
      agent_log(ap, "RECV", strerror(cc));
      return 0;
    }

    /* would block, so leave it to do later */

    return -1;
  }

  head[cc] = '\0';
  ap->xsize += cc;

#ifdef AM_DEBUG
  if(mode & AM_DEBUG)
  {
    char buf[80];
    sprintf(buf,"%s\t[%d]\tpeer>>>\n",Now(),ap->sno);
    f_cat("run/bmta.debug",buf);
    f_cat("run/bmta.debug",head);
    sprintf(buf,"\t[%d]\tpeer<<<\n",ap->sno);
    f_cat("run/bmta.debug",buf);
  }
#endif
  /* --------------------------------------------------- */
  /* DATA mode						 */
  /* --------------------------------------------------- */

  if (mode & AM_DATA)
  {
    dest = head - 1;

    for (;;)
    {
      cc = *head;

      if (!cc)
      {
	used = dest - data + 1;
	if (used > 2 && *dest == '\n' && dest[-1] == '.' && dest[-2] == '\n')
	  break;		/* end of mail body */

	ap->used = used;
	return 1;
      }

      head++;

      if (cc == '\r')
	continue;

      if (cc == '\n')
      {
	for (;;)
	{
	  used = *dest;

	  if (used == ' ' || used == '\t')
	  {
	    dest--;		/* strip the trailing space */
	    continue;
	  }

	  if (used == '.' && dest - data >= 2 &&
	    dest[-1] == '.' && dest[-2] == '\n')
	  {
	    *dest = ' ';	/* strip leading ".." to ". " */
	  }

	  break;
	}
      }

      *++dest = cc;
    }

    if (mode & AM_SWALLOW)
    {
      agent_reset(ap);
      agent_reply(ap, "552 To much mail data");
      return -1;
    }

    /* strip the trailing empty lines */

    dest -= 3;
    while (*dest == '\n' && dest > data)
      dest--;
    dest += 2;
    *dest = '\0';

    ap->used = dest - data;

    /* Thor.981223: 不擋 bbsreg和bbs, 將 untrust.acl 分離出來 */
    /* if (mode & AM_SPAM) */
    if (!(mode & (AM_VALID | AM_MAILPOST)) && (mode & AM_SPAM))
    {
      sprintf(ap->memo, "SPAM : %s", ap->from);
      /* mta_memo(ap, 0); */    /* Thor.980730: 再check看看 */

      mta_memo(ap, 0);          /* lkchu: mark SPAM mail */

      agent_reply(ap, "250 Message droped");
      agent_log(ap, "SPAM", "mail");
    }
    else
    {
      cc = mta_mailer(ap);
      if (!cc)
      {
	agent_reply(ap, "250 Message accepted");
      }
      else if (cc < 0)
      {
	ap->xspam++;
	agent_log(ap, "SPAM", "mail");
      }
    }

    agent_reset(ap);
    return 1;
  }

  /* --------------------------------------------------- */
  /* command mode					 */
  /* --------------------------------------------------- */

  used += cc;

  if (used >= MAX_CMD_LEN)
  {
    fprintf(flog, "CMD\t[%d] too long (%d) %.32s\n",
      ap->sno, used, data);
#ifdef DEBUG
    fflush(flog);
#endif
    ap->mode = (mode |= AM_DROP);
    ap->xerro += 10;		/* are you hacker ? */
    used = 32;
  }

  while (cc = *head)
  {
    if (cc == '\r' || cc == '\n')
    {
      Command *cmd;

      *head = '\0';

      if (mode & AM_DROP)
      {
	agent_reset(ap);
	agent_reply(ap, "552 command too long");
	return -1;
      }

      for (cmd = cmd_table; head = cmd->cmd; cmd++)
      {
	if (!str_ncmp(data, head, 4))
	  break;
      }

      ap->used = 0;
      (*cmd->func) (ap);
      return 1;
    }

    if (cc == '\t')
      *head = ' ';

    head++;
  }

  ap->used = used;
  return 1;
}


/* ----------------------------------------------------- */
/* close a connection & release its resource		 */
/* ----------------------------------------------------- */


static void
agent_fire(ap)
  Agent *ap;
{
  int num;
  char *data, *key, xerro[32], xspam[32];
  char data2[256]; /* 修正 logit 有可能因為指標的關係, 造成 segmentation fault */

  num = ap->sock;
  if (num > 0)
  {

#ifdef	SUNOS
#define M_NONBLOCK	FNDELAY
#else
#define M_NONBLOCK	O_NONBLOCK
#endif

    fcntl(num, F_SETFL, M_NONBLOCK);

#define	MSG_ABORT	"\r\n450 buggy, closing ...\r\n"
    send(num, MSG_ABORT, sizeof(MSG_ABORT) - 1, 0);
#undef	MSG_ABORT
    shutdown(num, 2);
    close(num);

    key = "END";
  }
  else
  {
    key = "BYE";
  }

  agent_free_rcpt(ap);

  /* log */

  data = ap->data;

  *xerro = *xspam = '\0';
  if ((num = ap->xerro) > 0)
    sprintf(xerro, " X%d", num);
  if ((num = ap->xspam) > 0)
    sprintf(xspam, " Z%d", num);

  sprintf(data2, "[%d] T%d R%d D%d S%d%s%s", ap->sno, 
    (int) (time(0) - ap->tbegin), ap->xrcpt, ap->xdata, 
    ap->xsize, xerro, xspam);
  logit(key, data2);

  MYDOG;
  free(data);
  MYDOG;
}


/* ----------------------------------------------------- */
/* accept a new connection				 */
/* ----------------------------------------------------- */


static char servo_ident[128];


static int
agent_accept()
{
  int csock;
  int value;
  struct sockaddr_in csin;
  char rhost[80], *ident;

  for (;;)
  {
    value = sizeof(csin);
    csock = accept(0, (struct sockaddr *) &csin, &value);
    if (csock > 0)
      break;

    csock = errno;
    if (csock != EINTR)
    {
      logit("ACCEPT", strerror(csock));
      return -1;
    }

    while (waitpid(-1, NULL, WNOHANG | WUNTRACED) > 0);
  }

  value = 1;
  setsockopt(csock, IPPROTO_TCP, TCP_NODELAY, (char *) &value, sizeof(value));

  /* --------------------------------------------------- */
  /* check remote host / user name			 */
  /* --------------------------------------------------- */

#if 1
  hht_look((HostAddr *) &csin.sin_addr, rhost);
  /* Thor.981207: 追蹤 ip address */
  sprintf(ident = servo_ident, "[%d] %s ip:%08x", ++servo_sno, rhost, csin.sin_addr.s_addr);
#else
  /* dns_ident(BMTA_PORT, &csin, rhost, ruser); */
  /* Thor.990325: 定義修改, 不再固定死為BMTA_PORT, 且從來自的interface連回 */
  dns_ident(csock, &csin, rhost, ruser);
  sprintf(ident = servo_ident, "[%d] %s@%s ip:%08x", ++servo_sno, ruser, rhost, csin.sin_addr.s_addr);
#endif

  ht_add(mhost_ht, rhost);

#if 0
  if (acl_match(rhost, "*"))
  {
    char buf[256];

    sprintf(buf, "421 %s deny %s\r\n", MYHOSTNAME, ident);
    send(csock, buf, strlen(buf), 0); /* Thor.981206: 補個0上來 */
    shutdown(csock, 2);
    close(csock);
    logit("DENY", ident);
    return -1;
  }
#endif

  logit("CONN", ident);
  return csock;
}


/* ----------------------------------------------------- */
/* signal routines					 */
/* ----------------------------------------------------- */


#ifdef	SERVER_USAGE
static void
servo_usage()
{
  struct rusage ru;

  if (getrusage(RUSAGE_SELF, &ru))
    return;

  fprintf(flog, "\n[Server Usage]\n\n"
    " user time: %.6f\n"
    " system time: %.6f\n"
    " maximum resident set size: %lu P\n"
    " integral resident set size: %lu\n"
    " page faults not requiring physical I/O: %d\n"
    " page faults requiring physical I/O: %d\n"
    " swaps: %d\n"
    " block input operations: %d\n"
    " block output operations: %d\n"
    " messages sent: %d\n"
    " messages received: %d\n"
    " signals received: %d\n"
    " voluntary context switches: %d\n"
    " involuntary context switches: %d\ngline: %d\n\n",

    (double) ru.ru_utime.tv_sec + (double) ru.ru_utime.tv_usec / 1000000.0,
    (double) ru.ru_stime.tv_sec + (double) ru.ru_stime.tv_usec / 1000000.0,
    ru.ru_maxrss,
    ru.ru_idrss,
    ru.ru_minflt,
    ru.ru_majflt,
    (int) ru.ru_nswap,
    (int) ru.ru_inblock,
    (int) ru.ru_oublock,
    (int) ru.ru_msgsnd,
    (int) ru.ru_msgrcv,
    (int) ru.ru_nsignals,
    (int) ru.ru_nvcsw,
    (int) ru.ru_nivcsw, 
    (int) gline);

  fflush(flog);
}
#endif


#define SS_CONFIG	1
#define SS_SHUTDOWN	2


static int servo_state;


static void
sig_hup()
{
  servo_state |= SS_CONFIG;
}


static void
sig_term()			/* graceful termination */
{
  servo_state |= SS_SHUTDOWN;
}


static void
sig_abort(sig)
  int sig;
{
  char buf[80];

  sprintf(buf, "abort: %d, errno: %d, gline: %d", sig, errno, gline);
  logit("EXIT", buf);
  fclose(flog);
  exit(0);
}


static void
reaper()
{
  while (waitpid(-1, NULL, WNOHANG | WUNTRACED) > 0);
}


static void
servo_signal()
{
  struct sigaction act;

  /* sigblock(sigmask(SIGPIPE)); */ /* Thor.981206: 統一 POSIX 標準用法  */ 

  /* act.sa_mask = 0; */ /* Thor.981105: 標準用法 */
  sigemptyset(&act.sa_mask);  
  act.sa_flags = 0;

  act.sa_handler = sig_term;	/* forced termination */
  sigaction(SIGTERM, &act, NULL);

  act.sa_handler = sig_abort;	/* forced termination */
  sigaction(SIGSEGV, &act, NULL);	/* if rlimit violate */
  sigaction(SIGBUS, &act, NULL);
#if 1
  /* Thor.990203: 抓 signal */
  sigaction(SIGURG, &act, NULL);
  sigaction(SIGXCPU, &act, NULL);
  sigaction(SIGXFSZ, &act, NULL);
#ifdef SOLARIS
  sigaction(SIGLOST, &act, NULL);
  sigaction(SIGPOLL, &act, NULL);
  sigaction(SIGPWR, &act, NULL);
#endif
  sigaction(SIGFPE, &act, NULL);
#ifndef LINUX
  sigaction(SIGSYS, &act, NULL);
  sigaction(SIGEMT, &act, NULL);
#endif
  sigaction(SIGWINCH, &act, NULL);
  sigaction(SIGINT, &act, NULL);
  sigaction(SIGQUIT, &act, NULL);
  sigaction(SIGILL, &act, NULL);
  sigaction(SIGTRAP, &act, NULL);
  sigaction(SIGABRT, &act, NULL);
  sigaction(SIGTSTP, &act, NULL);
  sigaction(SIGTTIN, &act, NULL);
  sigaction(SIGTTOU, &act, NULL);
  sigaction(SIGVTALRM, &act, NULL);
#endif

  act.sa_handler = sig_hup;	/* restart config */
  sigaction(SIGHUP, &act, NULL);

  act.sa_handler = reaper;
  sigaction(SIGCHLD, &act, NULL);

#ifdef	SERVER_USAGE
  act.sa_handler = servo_usage;
  sigaction(SIGPROF, &act, NULL);
#endif

  /* Thor.981206: lkchu patch: 統一 POSIX 標準用法  */
  /* 在此借用 sigset_t act.sa_mask */
  sigaddset(&act.sa_mask, SIGPIPE);
  sigprocmask(SIG_BLOCK, &act.sa_mask, NULL);

}


/* ----------------------------------------------------- */
/* server core routines 				 */
/* ----------------------------------------------------- */


static void
servo_daemon(inetd)
  int inetd;
{
  int fd, value;
  char buf[80];
  struct linger ld;
  struct sockaddr_in sin;

  /*
   * More idiot speed-hacking --- the first time conversion makes the C
   * library open the files containing the locale definition and time zone.
   * If this hasn't happened in the parent process, it happens in the
   * children, once per connection --- and it does add up.
   */

  time((time_t *) &value);
  gmtime((time_t *) &value);
  strftime(buf, 80, "%d/%b/%Y:%H:%M:%S", localtime((time_t *) &value));

#ifdef	HAVE_RLIMIT

{
  struct rlimit limit;

  /* --------------------------------------------------- */
  /* adjust the resource limit				 */
  /* --------------------------------------------------- */

  getrlimit(RLIMIT_NOFILE, &limit);
  limit.rlim_cur = limit.rlim_max;
  setrlimit(RLIMIT_NOFILE, &limit);

  limit.rlim_cur = limit.rlim_max = 16 * 1024 * 1024;
  setrlimit(RLIMIT_FSIZE, &limit);

  /* limit.rlim_cur = limit.rlim_max = 4 * 1024 * 1024; */
  /* Thor.990207: 怕空間不夠, 暫改為 8MB */
  limit.rlim_cur = limit.rlim_max = 8 * 1024 * 1024;
  setrlimit(RLIMIT_DATA, &limit);

#ifdef SOLARIS
#define RLIMIT_RSS RLIMIT_AS
  /* Thor.981206: port for solaris 2.6 */
#endif

  setrlimit(RLIMIT_RSS, &limit);

  limit.rlim_cur = limit.rlim_max = 0;
  setrlimit(RLIMIT_CORE, &limit);
}

#endif

  /* --------------------------------------------------- */
  /* detach daemon process				 */
  /* --------------------------------------------------- */

  close(1);
  close(2);

  if (inetd)
    return;

  close(0);

  if (fork())
    exit(0);

  setsid();

  if (fork())
    exit(0);

  /* --------------------------------------------------- */
  /* setup socket					 */
  /* --------------------------------------------------- */

  fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  value = 1;
  setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *) &value, sizeof(value));

  ld.l_onoff = ld.l_linger = 0;
  setsockopt(fd, SOL_SOCKET, SO_LINGER, (char *) &ld, sizeof(ld));

  sin.sin_family = AF_INET;
  sin.sin_port = htons(BMTA_PORT);
  sin.sin_addr.s_addr = htonl(INADDR_ANY);
  memset((char *) &sin.sin_zero, 0, sizeof(sin.sin_zero));

  if (bind(fd, (struct sockaddr *) & sin, sizeof(sin)) ||
    listen(fd, TCP_BACKLOG))
    exit(1);
}


/* Thor.990204: 註解: 每天早上 5:30 整理一次 */
#define	SERVO_HOUR	5
#define	SERVO_MIN	30


static time_t
fresh_time(uptime)
  time_t uptime;
{
  struct tm *local;
  int i;

  local = localtime(&uptime);
  i = (SERVO_HOUR - local->tm_hour) * 3600 +
    (SERVO_MIN - local->tm_min) * 60;
  if (i < 120)			/* 保留時間差 120 秒 */
    i += 86400;
  return (uptime + i);
}

int
main(argc, argv)
  int argc;
  char *argv[];
{
  int n, sock, state;
  time_t uptime, tcheck, tfresh, tscore;
  Agent **FBI, *Scully, *Mulder, *agent;
  fd_set rset, wset, xset;
  static struct timeval tv = {BMTA_PERIOD, 0};

  state = 0;

  while ((n = getopt(argc, argv, "hid")) != -1)
  {
    switch (n)
    {
    case 'i':
      state = 1;
      break;

    case 'd':
      break;

    case 'h':
    default:

      fprintf(stderr, "Usage: %s [options]\n"
	"\t-i  start from inetd with wait option\n"
	"\t-d  debug mode\n"
	"\t-h  help\n",
	argv[0]);
      exit(0);
    }
  }

  servo_daemon(state);

  setgid(BBSGID);
  setuid(BBSUID);
  chdir(BBSHOME);
  servo_signal();

  acl_load(FN_ETC_SPAMER_ACL);
  mailer_load(FN_ETC_MAILER_ACL);

  log_open();
  bshm_init();
  ushm_init();
  dns_init();

  mrcpt_ht = ht_new(128, 0);
  mhost_ht = ht_new(256, 0);
  mfrom_ht = ht_new(256, 0);
  title_ht = ht_new(512, 0);

#ifdef	CHECK_FORGE
  forge_ht = ht_new(256, 0);
#endif

  uptime = time(0);
  tcheck = uptime + BMTA_PERIOD;
  tfresh = fresh_time(uptime);
  tscore = uptime + 2 * 60 * 60;

  Scully = Mulder = NULL;

  for (;;)
  {
    /* maintain : resource and garbage collection */

    uptime = time(0);
    if (tcheck < uptime)
    {
      /* ----------------------------------------------- */
      /* agent_audit(uptime - BMTA_TIMEOUT)		 */
      /* ----------------------------------------------- */

      tcheck = uptime - BMTA_TIMEOUT;

      for (FBI = &Scully; agent = *FBI;)
      {
	if ((agent->uptime < tcheck) || (agent->xerro > BMTA_FAULT))
	{
	  agent_fire(agent);

	  *FBI = agent->anext;

	  agent->anext = Mulder;
	  Mulder = agent;
	}
	else
	{
	  FBI = &(agent->anext);
	}
      }

      /* ----------------------------------------------- */
      /* expire SPAM HashTable				 */
      /* ----------------------------------------------- */

      if (uptime > tscore)
      {
	tscore = uptime + 120 * 60;

	/* 超過 120 分鐘沒新進記錄，就開始 expire */

	tcheck = uptime - 120 * 60;
	ht_expire(mfrom_ht, tcheck);
	ht_expire(mhost_ht, tcheck);
	ht_expire(mrcpt_ht, tcheck);
	ht_expire(title_ht, tcheck);

	/* --------------------------------------------- */
	/* expire DNS HostHashTable cache		 */
	/* --------------------------------------------- */

	hht_expire(uptime - 3 * 60 * 60);

	/* ht_expire(forge_ht, tcheck);*/ /* never expire */

      }

      /* ----------------------------------------------- */
      /* maintain SPAM & server log			 */
      /* ----------------------------------------------- */

      if (tfresh < uptime)
      {
	tfresh = uptime + 86400;

#if 0
	spam_fresh(uptime);
#endif
	visit_fresh();
	servo_usage();
	log_fresh(BMTA_LOGFILE);
      }
      else
      {
	fflush(flog);
      }

      tcheck = uptime + BMTA_PERIOD;
    }

    /* ------------------------------------------------- */
    /* check servo operation state			 */
    /* ------------------------------------------------- */

    n = 0;

    if (state = servo_state)
    {
      if (state & SS_CONFIG)
      {
	state ^= SS_CONFIG;

	acl_reset();
	acl_load(FN_ETC_SPAMER_ACL);
      }

      if (state & SS_SHUTDOWN)	/* graceful shutdown */
      {
	n = -1;
	close(0);
      }

      servo_state = state;
    }

    /* ------------------------------------------------- */
    /* Set up the fdsets				 */
    /* ------------------------------------------------- */

    FD_ZERO(&rset);
    FD_ZERO(&wset);
    FD_ZERO(&xset);

    if (n == 0)
      FD_SET(0, &rset);

    for (agent = Scully; agent; agent = agent->anext)
    {
      sock = agent->sock;
      state = agent->state;

      if (n < sock)
	n = sock;

      if (state == CS_RECV)
      {
	FD_SET(sock, &rset);
      }
      else
      {
	FD_SET(sock, &wset);
      }

      FD_SET(sock, &xset);
    }

    /* no active agent and ready to die */

    if (n < 0)
    {
      break;
    }

    {
    struct timeval tv_tmp=tv;
    /* Thor.981221: for future reservation bug */
    n = select(n + 1, &rset, &wset, &xset, &tv_tmp);
    }

    if (n == 0)
    {
      continue;
    }

    if (n < 0)
    {
      n = errno;
      if (n != EINTR)
      {
	logit("SELECT", strerror(n));
      }
      continue;
    }

    /* ------------------------------------------------- */
    /* serve active agents				 */
    /* ------------------------------------------------- */

    uptime = time(0);

    for (FBI = &Scully; agent = *FBI;)
    {
      sock = agent->sock;

      if (FD_ISSET(sock, &wset))
      {
	state = agent_send(agent);
      }
      else if (FD_ISSET(sock, &rset))
      {
	state = agent_recv(agent);
      }
      else if (FD_ISSET(sock, &xset))
      {
	state = 0;
      }
      else
      {
	state = -1;
      }

      if (state == 0)		/* fire this agent */
      {
	agent_fire(agent);

	*FBI = agent->anext;

	agent->anext = Mulder;
	Mulder = agent;

	continue;
      }

      if (state > 0)
      {
	agent->uptime = uptime;
      }

      FBI = &(agent->anext);
    }

    /* ------------------------------------------------- */
    /* serve new connection				 */
    /* ------------------------------------------------- */

    if (FD_ISSET(0, &rset))
    {
      /* Thor.990319: check maximum connection number */
      unsigned int ip_addr;
      sock = agent_accept();

      if (sock > 0)
      {
#if 1
        /* Thor.990319: check maximum connection number */
        int num = 0;

        sscanf(strstr(servo_ident,"ip:")+3,"%x",&ip_addr);
        for (agent = Scully; agent; agent = agent->anext) 
        {
          if(agent->ip_addr == ip_addr)
            num ++;
        }
        if(num >= MAX_HOST_CONN)
        {
          char buf[256]; 
 
          sprintf(buf, "421 %s over max connection\r\n",MYHOSTNAME); 
          send(sock, buf, strlen(buf), 0); /* Thor.981206: 補個0上來 */ 
          shutdown(sock, 2); 
          close(sock); 
          logit("OVER", servo_ident); 
          continue; 
        }
        
#endif
	if (agent = Mulder)
	{
	  Mulder = agent->anext;
	}
	else
	{
          MYDOG;
	  agent = (Agent *) malloc(sizeof(Agent));
          MYDOG;
          if(!agent) /* Thor.990205: 記錄空間不夠 */
            TRACE("ERROR","Not enough space in main()");
	}

	*FBI = agent;

	/* variable initialization */

	memset(agent, 0, sizeof(Agent));

	agent->sock = sock;
	agent->sno = servo_sno;
	agent->state = CS_SEND;
	agent->tbegin = agent->uptime = uptime;
	strcpy(agent->ident, servo_ident);

        /* Thor.990319: check maximum connection number */
        agent->ip_addr = ip_addr;

#ifdef AM_DEBUG
        if(strstr(agent->ident,"8c7a4133"))
        { /* Thor.990221: that's mail.cc.ntnu.edu.tw */
          agent->mode |= AM_DEBUG;
	}
#endif

        MYDOG;
	agent->data = (char *) malloc(MIN_DATA_SIZE);
        MYDOG;
        if(!agent->data) /* Thor.990205: 記錄空間不夠 */
          TRACE("ERROR","Not enough space in agent->data");
#if 0
	sprintf(agent->data, "220 " MYHOSTNAME " ESMTP ready %s\r\n",
	  servo_ident);
#endif
	sprintf(agent->data, "220 " MYHOSTNAME " SMTP ready %s\r\n",
	  servo_ident); /* Thor.981001: 不用 enhanced SMTP */
	agent->used = strlen(agent->data);
	agent->size = MIN_DATA_SIZE;
      }
    }

    /* ------------------------------------------------- */
    /* tail of main loop				 */
    /* ------------------------------------------------- */
  }

  logit("EXIT", "shutdown");
  fclose(flog);

  exit(0);
}
