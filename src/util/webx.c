/*-------------------------------------------------------*/
/* util/webx.c		( NTHU CS MapleBBS Ver 3.00 )	 */
/*-------------------------------------------------------*/
/* target : WEB client (command-line mode)		 */
/* create : 95/03/29				 	 */
/* update : 97/03/29				 	 */
/*-------------------------------------------------------*/
/* syntax : webx url_file [period duration]		 */
/*-------------------------------------------------------*/

#include "bbs.h"


#define	WEBC_PIDFILE	"run/webc.pid"
#define	WEBC_LOGFILE	"run/webc.log"

static int curpid;

/* ----------------------------------------------------- */
/* ----------------------------------------------------- */


static char pool[4096];
/* Thor.980828: 共享一下 pool */

/* ----------------------------------------------------- */
/* log routines						 */
/* ----------------------------------------------------- */

static void
logit(key, msg)
  char *key;
  char *msg;
{
  time_t now;
  struct tm *p;
  char buf[256];

  time(&now);
  p = localtime(&now);
  /* Thor.990329: y2k */
  sprintf(buf, "%s\t%s\t%02d/%02d/%02d %02d:%02d:%02d [%d]\n",
    key, msg, p->tm_year % 100, p->tm_mon + 1, p->tm_mday,
    p->tm_hour, p->tm_min, p->tm_sec, curpid);
  f_cat(WEBC_LOGFILE,buf);
}


/* ----------------------------------------------------- */
/* web client						 */
/* ----------------------------------------------------- */


static int
webc(file, host, path, port)
  char *file;
  char *host;
  char *path;
  int port;
{
  int cc, sock, tlen , ignore = 0;
  FILE *fp;
  char *xhead, *xtail, buf[120], tag[8];

  sock = dns_open(host, port);
  if (sock < 0)
  {
    logit("ERROR","dns_open error in webc()");
    return -1;
  }

  sprintf(buf, "GET %s HTTP/1.0\n\n", path);
  /* Thor.981108: HTTP 標準用法, 以便從 proxy 出去 query (squid) */
  /* sprintf(buf, "GET %s HTTP/1.0\n\n", path); */
  /* Thor.981108: 不過還要 parse header */
  cc = strlen(buf);
  if (send(sock, buf, cc, 0) != cc)
  {
    close(sock);
    logit("ERROR","send error in webc()");
    return -1;
  }

  xhead = pool;
  xtail = pool;
  tlen = 0;

  strcpy(buf, file);
  strcat(buf, "-");
  fp = fopen(buf, "w");

  for (;;)
  {
    if (xhead >= xtail)
    {
      xhead = pool;
      cc = recv(sock, xhead, sizeof(pool), 0);
      if (cc <= 0)
	break;
      xtail = xhead + cc;
    }

    cc = *xhead++;
    if (cc == '<')
    {
      tlen = 1;
      continue;
    }

    if (tlen)
    {
      /* support <br> and <P> */

      if (cc == '>')
      {
	if (tlen == 3 && !str_ncmp(tag, "br", 2))
	{
	  fputc('\n', fp);
	}
	else if (tlen == 2 && !str_ncmp(tag, "P", 1))
	{
	  fputc('\n', fp);
	  fputc('\n', fp);
	}

	tlen = 0;
	continue;
      }

      if (tlen <= 2)
      {
	tag[tlen - 1] = cc;
      }
      tlen++;
      continue;
    }
    if (cc == '\n')
      ignore++;
    if (cc != '\r')
      if (ignore >10)
        fputc(cc, fp);
  }

  close(sock);

  fputc('\n', fp);
  fclose(fp);
  rename(buf, file);
  return cc;
}

/* ----------------------------------------------------- */
/* main routines					 */
/* ----------------------------------------------------- */


typedef struct webX
{
  struct webX *next;
  char *file;
  char *host;
  char *path;
  int port;
}    webX;


#define	STR_SPACE	" \t\n\r"


static webX *
webx_parse(file)
  char *file;
{
  FILE *fp;
  webX *web;
  char *host, *path, buf[256];

  web = NULL;

  if ((fp = fopen(file, "r")))
  {
    while (fgets(buf, sizeof(buf), fp))
    {
      if (buf[0] <= '#')
	continue;

      if ((file = strtok(buf, STR_SPACE)))
      {
	if ((host = strtok(NULL, STR_SPACE)))
	{
	  if ((path = strtok(NULL, STR_SPACE)))
	  {
	    int port;
	    char *str;
	    webX *tmp;

	    if ((str = strtok(NULL, STR_SPACE)))
	    {
	      port = atoi(str);
	      if (port <= 0 || port >= 65535)
		continue;
	    }
	    else
	    {
	      port = 80;
	    }

	    tmp = (webX *) malloc(sizeof(webX));
	    tmp->next = web;
	    tmp->file = strdup(file);
	    tmp->host = strdup(host);
	    tmp->path = strdup(path);
	    tmp->port = port;

	    web = tmp;

	  }
	}
      }
    }

    fclose(fp);
  }
  logit("PARSE", web ? "webx_parse finish" : "webx_parse empty");
  return web;
}

int
main(argc, argv)
  int argc;
  char **argv;
{
  webX *wlist, *web;
  time_t now, uptime;
  int period, duration;

  if (argc < 2 || argc > 4)
  {
    fprintf(stderr, "%s: web client\n\nusage:\t %s file [period] [duration]\n",
      argv[0], argv[0]);
  }

  close(0);
  close(1);
  close(2);

  wlist = webx_parse(argv[1]);
  if (wlist == NULL)
    exit(0);

  period = duration = 0;
  if (argc > 2)
  {
    if (fork())
      exit(0);

    setsid();

    if (fork())
      exit(0);
 
    curpid = getpid();

    period = atoi(argv[2]);

    if (argc > 3)
      duration = atoi(argv[3]);
  }


  dns_init();
  umask(077);

  /* Thor.980828: Start webx, write pid */
  sprintf(pool, "webx[%d] conf:%s period:%d duration:%d\n", curpid, argv[1],period, duration); 
  f_cat(WEBC_PIDFILE, pool);
  logit("START", "WEBX client");

  now = time(0);
  duration += now;

  for (;;)
  {
    logit("REFRESH", argv[1]);
    web = wlist;
    do
    {
      webc(web->file, web->host, web->path, web->port);
    } while ((web = web->next));

    if (argc <= 2)
      break;

    uptime = time(0);
    if (uptime > duration)
      break;

    now += period;
    if (uptime < now)
    {
      sleep(now - uptime);
    }
    else
    {
      now = period;
    }
  }

  logit("FINISH", argv[1]);
  exit(0);
}
