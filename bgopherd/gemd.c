/*-------------------------------------------------------*/
/* util/gemd.c		( NTHU CS MapleBBS Ver 3.00 )	 */
/*-------------------------------------------------------*/
/* target : BBS gopher daemon				 */
/* create : 96/11/20				 	 */
/* update : 97/03/29				 	 */
/*-------------------------------------------------------*/
/* syntax : gemd					 */
/*-------------------------------------------------------*/
/* notice : single process concurrent server		 */
/*-------------------------------------------------------*/


#include "bbs.h"


#include <sys/wait.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/shm.h>



#define WATCH_DOG
#define	SERVER_USAGE


#define	GEMD_LOGFILE	"run/gemd.log"
#define	GEMD_PIDFILE	"run/gemd.pid"


#define GEMD_PORT	70
#define	GEMD_PERIOD	(60 * 30)
#define GEMD_TIMEOUT	(60 * 10)


#define	TCP_QLEN	3
#define	TCP_BUFSIZ	(256 * 14)
#define	TCP_LINSIZ	256
#define	TCP_RCVSIZ	512


/* ----------------------------------------------------- */
/* client connection structure				 */
/* ----------------------------------------------------- */


typedef struct Agent
{
  struct Agent *anext;
  int state;
  int sock;
  int sno;
  time_t tbegin;		/* 建立 connection 的時間 */
  time_t uptime;

  FILE *stream;
  char zone[64];

  char pool[TCP_BUFSIZ];	/* buffered I/O pool */
  int locus;
  int xdata;
}     Agent;


#define ap_log(key, sno, msg) fprintf(flog, "%s\t[%d] %s\n", key, sno, msg);


/* ----------------------------------------------------- */
/* connection state					 */
/* ----------------------------------------------------- */


#define	CS_FREE		0x00
#define	CS_READ		0x01	/* reading command */
#define	CS_FILE		0x02	/* writing file */
#define	CS_INDEX	0x03	/* writing index */
#define	CS_GPLUS	0x04	/* writing index (gopher 2.0+) */
#define	CS_FLUSH	0x05	/* flush data */


/* ----------------------------------------------------- */
/* operation log and debug information			 */
/* ----------------------------------------------------- */


extern int errno;


static FILE *flog;		/* log file descriptor */
static int gline;

static BCACHE *bshm;


#ifdef  WATCH_DOG
# define MYDOG  gline = __LINE__
#else
# define MYDOG			/* NOOP */
#endif




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
      xshm->number = read(n, xshm->bcache, MAXBOARD * sizeof(BRD)) / sizeof(BRD);
      close(n);
    }
    else
    {
      /* 開啟 .BRD 失敗  直接結束程式 */
      exit(1);
    }

    /* 等所有 boards 資料更新後再設定 uptime */

    time(uptime);
    fprintf(stderr, "[gemd]\tCACHE\treload bcache");

    return;
  }
}

static int
allow_brdname(brdname)
  char *brdname;
{
  BRD *bhdr, *tail;

  bhdr = bshm->bcache;
  tail = bhdr + bshm->number;

  do
  {
    if (!strcmp(bhdr->brdname, brdname))
    {
      if (!bhdr->readlevel)
        return 1;
      break;
    }
  } while (++bhdr < tail);

  return 0;
}

static int
allow_path(fpath)
  char *fpath;
{
  char *brdname, *str;
  int ret;

  if (!strncmp(fpath, "brd/", 4))
  {
    brdname = fpath + 4;
    if ((str = strchr(brdname, '/')))
    {
      *str = '\0';
      ret = allow_brdname(brdname);
      *str = '/';
      return ret;
    }
  }
  return 1;
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
  fprintf(flog, "%02d/%02d %02d:%02d:%02d %-7s%s\n",
    p->tm_mon + 1, p->tm_mday,
    p->tm_hour, p->tm_min, p->tm_sec, key, msg);
}


static char *
str_time(t)
  time_t *t;
{
  static char buf[20];
  struct tm *p;

  p = localtime(t);
  sprintf(buf, "%02d/%02d %02d:%02d:%02d",
    p->tm_mon + 1, p->tm_mday,
    p->tm_hour, p->tm_min, p->tm_sec);
  return buf;
}


static inline void
log_open()
{
  FILE *fp;

  umask(077);

  if ((fp = fopen(GEMD_PIDFILE, "w")))
  {
    fprintf(fp, "%d\n", getpid());
    fclose(fp);
  }

  flog = fopen(GEMD_LOGFILE, "w");
  logit("START", "gemd (gopher) daemon");
}


/* ----------------------------------------------------- */
/* server side stuff					 */
/* ----------------------------------------------------- */


static char *
str_copy(dst, src)
  char *dst;
  char *src;
{
  int ch;

  while ((ch = *src++))
    *dst++ = ch;
  return dst;
}


static inline FILE *
gem_open(fpath)
  char *fpath;
{
  FILE *fp;
  int fd;
  struct stat st;

  /* itoc.030727: 加上檢查看板權限的部分，以免有人亂踹 */
  if (!allow_path(fpath))
  {
    fprintf(flog,"PERM\t%s\n", fpath);
    return NULL;
  }


  fp = NULL;
  fd = open(fpath, O_RDONLY);
  if (fd >= 0)
  {
    if (fstat(fd, &st) || !S_ISREG(st.st_mode) ||
      (st.st_size <= 0) || !(fp = fdopen(fd, "r")))
    {
      close(fd);
    }
  }
  return fp;
}


/* ----------------------------------------------------- */
/* transform ASCII file to text-mail format		 */
/* ----------------------------------------------------- */


static void
gem_file(ap)
  Agent *ap;
{
  char *pool, *head, *tail;
  FILE *fp;
  int cc;

  fp = ap->stream;
  pool = ap->pool;
  head = pool + ap->locus;
  tail = pool + TCP_BUFSIZ - TCP_LINSIZ;

  while (head < tail)
  {
    if (!fgets(head, TCP_LINSIZ, fp))
    {
      fclose(fp);
      ap->stream = NULL;
      ap->state = CS_FLUSH;

      *head++ = '.';
      *head++ = '\r';
      *head++ = '\n';
      break;
    }

    while ((cc = *head))
    {
      if (cc == '\n')
      {
	*head++ = '\r';
	*head++ = '\n';
	break;
      }
      head++;
    }
  }
  ap->locus = head - pool;
}


/* ----------------------------------------------------- */
/* transform FOLDER to gopher index format		 */
/* ----------------------------------------------------- */


static void
gem_index(ap)
  Agent *ap;
{
  char *pool, *head, *tail, *str, *xname, *zone;
  int cc, state, xmode;
  FILE *fp;
  HDR hdr;

  fp = ap->stream;
  zone = ap->zone;
  pool = ap->pool;
  head = pool + ap->locus;
  tail = pool + TCP_BUFSIZ - TCP_LINSIZ;
  state = ap->state;

  while (head < tail)
  {
    if (fread(&hdr, sizeof(hdr), 1, fp) != 1)
    {
      fclose(fp);
      ap->stream = NULL;
      ap->state = CS_FLUSH;

      *head++ = '.';
      *head++ = '\r';
      *head++ = '\n';
      break;
    }

    xmode = hdr.xmode;
    if ((xmode & (GEM_RESTRICT|GEM_RESERVED|GEM_LOCK)) || (hdr.title[0] == '#'))
      continue;

    if (xmode & GEM_BOARD)
    {
       if(!allow_brdname(hdr.xname))
         continue;
    }


    if (state == CS_GPLUS)
      head = str_copy(head, "+INFO: ");

    *head++ = cc = (xmode & GEM_FOLDER) ? '1' : '0';

    head = str_copy(head, hdr.title);
    *head++ = '\t';

    xname = hdr.xname;

    if (xmode & GEM_GOPHER)
    {
      /* ----------------------------------------------- */
      /* 假設 HDR.xname 欄位長度夠，讓事情簡化		 */
      /* ----------------------------------------------- */

      str = strchr(xname, '/');
      *str++ = '\0';
      head = str_copy(head, str);
      *head++ = '\t';

      head = str_copy(head, xname);
      *head++ = '\t';

      sprintf(head, "%d\r\n", hdr.xid);
      head += strlen(head);
    }
    else
    {
      *head++ = cc;
      *head++ = '/';

      if (xmode & GEM_BOARD)
      {
	sprintf(head, "brd/%s/.DIR", xname);
	head += strlen(head);
      }
      else
      {
	head = str_copy(head, zone);

	cc = *xname;
	if (cc != '@')
	  cc = xname[7];
	*head++ = cc;
	*head++ = '/';

	head = str_copy(head, xname);
      }

      head = str_copy(head, "\t" MYHOSTNAME "\t70\r\n");
    }
  }
  ap->locus = head - pool;
}


/* ----------------------------------------------------- */
/* parse the command, check security & serve it		 */
/* ----------------------------------------------------- */


static int
gem_serve(ap)
  Agent *ap;
{
  char *data, *str, *zone;
  int cc;
  FILE *fp;
  
  struct stat st;

  data = ap->pool;
  if (strstr(data, "..") || strstr(data, "//"))
    return CS_FREE;

  switch (*data)
  {
  case '0':
    data += 2;
    if (*data == '/')
      return CS_FREE;
      
    if((str = strrchr(data,'/')))
    {
      str++;
      if(strstr(str,".DIR") || *str == 'F')
      {
        ap_log("FOPEN_1", ap->sno, data);
        return CS_FREE;
      }
      else if(*str == '@' && !stat(data,&st) && (st.st_size % sizeof(HDR) == 0))
      {
        ap_log("FOPEN_2", ap->sno, data);
        return CS_FREE;
      }
    }
    else
    {
      ap_log("FOPEN_3", ap->sno, data);
      return CS_FREE;  
    }
      
      
    cc = CS_FILE;
    break;

  case '1':
    if (data[1])
    {
      data += 2;
      if (*data == '/')
	return CS_FREE;
      cc = CS_INDEX;
      break;
    }

  case 0:
    data = ".DIR";
    cc = CS_INDEX;
    break;

  case '\t':
    data = ".DIR";
    cc = CS_GPLUS;
    break;

  default:
    return CS_FREE;
  }

  fp = gem_open(data);

  zone = ap->zone;
  *zone = '\0';

  if (!fp && cc == CS_INDEX && *data != '.' && strlen(data) <= 12)
  {
    /* map "xyz" to "brd/xyz/.DIR" and try it once again */

    strcpy(zone, data);
    sprintf(data, "brd/%s/.DIR", zone);
    fprintf(flog, "MAP\t[%d] %s -> %s\n", ap->sno, zone, data);
    fp = gem_open(data);
  }

  if (!fp)
    return CS_FREE;

  ap->stream = fp;
  if (cc != CS_FILE)
  {
    ap_log("DIR", ap->sno, data);
    if ((str = strrchr(data, '/')))
    {
      if (str[1] != '.')
	str--;
      else
	str++;

      while (data < str)
      {
	*zone++ = *data++;
      }
    }
    *zone = '\0';
    ap_log("ZONE", ap->sno, ap->zone);
  }
  else
  {
    ap_log("FILE", ap->sno, data);
  }
  return cc;
}


/* ----------------------------------------------------- */
/* send output to client				 */
/* ----------------------------------------------------- */


static int
agent_send(ap)
  Agent *ap;
{
  int sock, len, cc;
  char *data;

  sock = ap->sock;
  len = ap->locus;
  data = ap->pool;
  cc = send(sock, data, len, 0);

  if (cc < 0)
  {
    cc = errno;
    if (cc != EWOULDBLOCK /* && cc != EPIPE */ )
    {
      ap_log("send", ap->sno, strerror(cc));
      return 0;
    }

    return -1;			/* would block, so leave it to do later */
  }

  if (cc == 0)
    return -1;

  len -= cc;
  ap->xdata += cc;
  ap->locus = len;

  if (len)
  {
    memcpy(data, data + cc, len);
    return cc;
  }

  if (ap->state == CS_FLUSH)
  {
    ap->state = CS_FREE;
    return 0;
  }

  return cc;
}


/* ----------------------------------------------------- */
/* receive request from client				 */
/* ----------------------------------------------------- */


static int
agent_recv(ap)
  Agent *ap;
{
  int cc;
  char *pool, *data;

  pool = ap->pool;
  data = pool + ap->locus;
  cc = recv(ap->sock, data, TCP_RCVSIZ, 0);

  if (cc <= 0)
  {
    cc = errno;
    if (cc != EWOULDBLOCK /* && cc != EPIPE */ )
    {
      ap_log("RECV", ap->sno, strerror(cc));
      return 0;
    }

    return -1;			/* would block, so leave it to do later */
  }

  data[cc] = '\0';

  while ((cc = *data))
  {
    if (cc == '\r' || cc == '\n')
    {
      /* remove tailing '/' from path */

      if ((data > pool + 1) && data[-1] == '/' && data[-2] != '/')
	data--;

      *data = '\0';

      ap_log("CMD", ap->sno, pool);

      ap->state = cc = gem_serve(ap);

      if (cc == CS_FREE)
	return 0;

      if (cc == CS_GPLUS)
      {
	strcpy(pool, "+-1\r\n");
	ap->locus = 5;
      }
      else
      {
	ap->locus = 0;
      }

      return 1;
    }
    data++;

  }

  if ((ap->locus = data - pool) > TCP_RCVSIZ)
  {
    ap_log("HACK", ap->sno, "buffer overflow");
    return 0;
  }

  return 1;
}


/* ----------------------------------------------------- */
/* close a connection & release its resource		 */
/* ----------------------------------------------------- */


static void
agent_fire(ap)
  Agent *ap;
{
  int sock;
  FILE *fp;

  sock = ap->sock;
  shutdown(sock, 2);
  close(sock);

  if ((fp = ap->stream))
    fclose(fp);

  fprintf(flog, "%s\t[%d] T%d D%d\n",
    ap->state == CS_FREE ? "BYE" : "END",
    ap->sno, (int) (time(0) - ap->tbegin), ap->xdata);
}


/* ----------------------------------------------------- */
/* accept a new connection				 */
/* ----------------------------------------------------- */


static inline int
agent_accept(n)
  int n;
{
/*  Agent *ap;*/ /* statue.000712 */
  int sock;
  int value;

  /* gopher do not care remote host / user name */

  for (;;)
  {
    sock = accept(0, NULL, NULL);
    if (sock > 0)
      break;

    sock = errno;
    if (sock != EINTR)
    {
      logit("ACCEPT", strerror(sock));
      return -1;
    }

    while (waitpid(-1, NULL, WNOHANG | WUNTRACED) > 0);
  }

  value = 1;
  setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char *) &value, sizeof(value));
  return sock;
}


/* ----------------------------------------------------- */
/* server core routines					 */
/* ----------------------------------------------------- */


static void
servo_daemon(inetd)
  int inetd;
{
  int fd, value;
  char buf[80];
  struct sockaddr_in fsin;
  struct linger ld;
  struct rlimit limit;

  /*
   * More idiot speed-hacking --- the first time conversion makes the C
   * library open the files containing the locale definition and time zone.
   * If this hasn't happened in the parent process, it happens in the
   * children, once per connection --- and it does add up.
   */

  time((time_t *) &value);
  gmtime((time_t *) &value);
  strftime(buf, 80, "%d/%b/%Y:%H:%M:%S", localtime((time_t *) &value));

  /* --------------------------------------------------- */
  /* adjust the resource limit				 */
  /* --------------------------------------------------- */

  limit.rlim_cur = limit.rlim_max = 4 * 1024 * 1024;
  setrlimit(RLIMIT_FSIZE, &limit);
  setrlimit(RLIMIT_DATA, &limit);

#ifdef SOLARIS
#define RLIMIT_RSS RLIMIT_AS
  /* Thor.981206: port for solaris 2.6 */
#endif

  setrlimit(RLIMIT_RSS, &limit);

  limit.rlim_cur = limit.rlim_max = 0;
  setrlimit(RLIMIT_CORE, &limit);

  /* --------------------------------------------------- */
  /* detach process					 */
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
  /* setup socket                                        */
  /* --------------------------------------------------- */

  fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  value = 1;
  setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *) &value, sizeof(value));

#if 0
  setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (char *) &value, sizeof(value));
#endif

  ld.l_onoff = ld.l_linger = 0;
  setsockopt(fd, SOL_SOCKET, SO_LINGER, (char *) &ld, sizeof(ld));

  memset((char *) &fsin, 0, sizeof(fsin));
  fsin.sin_family = AF_INET;
  fsin.sin_port = htons(GEMD_PORT);
  fsin.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(fd, (struct sockaddr *) & fsin, sizeof(fsin)) ||
    listen(fd, TCP_QLEN))
    exit(1);
}


#ifdef	SERVER_USAGE
static void
servo_usage()
{
  struct rusage ru;

  if (getrusage(RUSAGE_SELF, &ru))
    return;

  fprintf(flog, "\n[Server Usage]\n\n"
    "user time: %.6f\n"
    "system time: %.6f\n"
    "maximum resident set size: %lu P\n"
    "integral resident set size: %lu\n"
    "page faults not requiring physical I/O: %d\n"
    "page faults requiring physical I/O: %d\n"
    "swaps: %d\n"
    "block input operations: %d\n"
    "block output operations: %d\n"
    "messages sent: %d\n"
    "messages received: %d\n"
    "signals received: %d\n"
    "voluntary context switches: %d\n"
    "involuntary context switches: %d\n"
    "gline: %d\n\n",

    (double) ru.ru_utime.tv_sec + (double) ru.ru_utime.tv_usec / 1000000.0,
    (double) ru.ru_stime.tv_sec + (double) ru.ru_stime.tv_usec / 1000000.0,
    ru.ru_maxrss,
    ru.ru_idrss,
    (int) ru.ru_minflt,
    (int) ru.ru_majflt,
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


static void
reaper()
{
  while (waitpid(-1, NULL, WNOHANG | WUNTRACED) > 0);
}


static void
sig_trap(sig)
  int sig;
{
  char buf[80];

  sprintf(buf, "[%d] at %d", sig, gline);
  logit("EXIT", buf);
  fclose(flog);
  exit(0);
}


/* ----------------------------------------------------- */
/* signal routines					 */
/* ----------------------------------------------------- */


static void
servo_signal()
{
  struct sigaction act;

  /* sigblock(sigmask(SIGPIPE)); */ /* Thor.981206: 統一 POSIX 標準用法  */ 

  /* act.sa_mask = 0; */ /* Thor.981105: 標準用法 */
  sigemptyset(&act.sa_mask);      
  act.sa_flags = 0;

  act.sa_handler = sig_trap;
  sigaction(SIGBUS, &act, NULL);
  sigaction(SIGSEGV, &act, NULL);
  sigaction(SIGTERM, &act, NULL);

  act.sa_handler = reaper;
  sigaction(SIGCHLD, &act, NULL);

#ifdef  SERVER_USAGE
  act.sa_handler = servo_usage;
  sigaction(SIGPROF, &act, NULL);
#endif

  /* Thor.981206: lkchu patch: 統一 POSIX 標準用法  */
  /* 在此借用 sigset_t act.sa_mask */
  sigaddset(&act.sa_mask, SIGPIPE);
  sigprocmask(SIG_BLOCK, &act.sa_mask, NULL);

}


int
main(argc, argv)
  int argc;
  char *argv[];
{
  int sock, nfds, state, servo_sno;
  time_t uptime, tcheck;
  Agent **FBI, *Scully, *Mulder, *agent;
  fd_set rset, wset, xset;
  static struct timeval tv = {GEMD_PERIOD, 0};

  state = 0;

  while ((nfds = getopt(argc, argv, "hid")) != -1)
  {
    switch (nfds)
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
  log_open();

  bshm_init();

  chdir("gem");
  servo_signal();

  tcheck = time(0) + GEMD_PERIOD;
  Scully = Mulder = NULL;
  servo_sno = 0;

  for (;;)
  {
    uptime = time(0);
    if (tcheck < uptime)
    {
      tcheck = uptime - GEMD_TIMEOUT;

      for (FBI = &Scully; (agent = *FBI);)
      {
	if (agent->uptime < tcheck)
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

      fflush(flog);
      tcheck = uptime + GEMD_PERIOD;
    }

    /* ------------------------------------------------- */
    /* Set up the fdsets				 */
    /* ------------------------------------------------- */

    FD_ZERO(&rset);
    FD_ZERO(&wset);
    FD_ZERO(&xset);

    FD_SET(0, &rset);
    nfds = 0;

    for (agent = Scully; agent; agent = agent->anext)
    {
      sock = agent->sock;
      state = agent->state;

      if (nfds < sock)
	nfds = sock;

      if (state == CS_READ)
      {
	FD_SET(sock, &rset);
      }
      else
      {
	FD_SET(sock, &wset);

	if (state == CS_FILE)
	  gem_file(agent);
	else if (state == CS_INDEX || state == CS_GPLUS)
	  gem_index(agent);
      }

      FD_SET(sock, &xset);
    }

    {
      struct timeval tv_tmp = tv;
      /* Thor.981206: for reservation future bug */
      nfds = select(nfds + 1, &rset, &wset, &xset, &tv_tmp);
    }

    if (nfds == 0)
    {
      continue;
    }

    if (nfds < 0)
    {
      sock = errno;
      if (sock != EINTR)
	logit("select", strerror(sock));
      continue;
    }

    /* ------------------------------------------------- */
    /* serve active agents				 */
    /* ------------------------------------------------- */

    uptime = time(0);

    for (FBI = &Scully; (agent = *FBI);)
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
      sock = agent_accept();
      if (sock <= 0)
	continue;

      if ((agent = Mulder))
      {
	Mulder = agent->anext;
      }
      else
      {
	agent = (Agent *) malloc(sizeof(Agent));
      }

      *FBI = agent;

      /* variable initialization */

      agent->anext = NULL;
      agent->state = CS_READ;
      agent->sock = sock;
      agent->sno = ++servo_sno;
      agent->tbegin = uptime;
      agent->uptime = uptime;
      agent->stream = NULL;
      agent->locus = 0;
      agent->xdata = 0;

      fprintf(flog, "CONN\t[%d] %s\n", servo_sno, str_time(&agent->tbegin));
    }

    /* ------------------------------------------------- */
    /* tail of main loop				 */
    /* ------------------------------------------------- */
  }
}
