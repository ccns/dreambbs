/*-------------------------------------------------------*/
/* lib/dns.c		( NTHU CS MapleBBS Ver 3.00 )	 */
/*-------------------------------------------------------*/
/* target : included C file for DNS routines		 */
/* create : 96/11/20					 */
/* update : 96/12/15					 */
/*-------------------------------------------------------*/

#include "dns.h"
#include "bbs.h"        /* lkchu.981201: 是否有 define HAVE_ETC_HOSTS */
#include <signal.h>
#include <unistd.h>
#include <string.h>

void
dns_init(void)
{
  res_init();
  /* _res.retrans = 5; */ /* DNS query timeout */
  _res.retry = 2;
//  _res.options |= RES_USEVC;
}


int
dns_query(
  char *name,			/* domain name */
  int qtype,			/* type of query */
  querybuf *ans 		/* buffer to put answer */
)
{
  querybuf buf;

  qtype = res_mkquery(QUERY, name, C_IN, qtype, (u_char *) NULL, 0, NULL,
    (u_char *) &buf, sizeof(buf));

  if (qtype >= 0)
  {
    qtype = res_send((u_char *) &buf, qtype, (u_char *) ans, sizeof(querybuf));

    /* avoid problems after truncation in tcp packets */

    if (qtype > sizeof(querybuf))
      qtype = sizeof(querybuf);
  }

  return qtype;
}
/*-------------------------------------------------------*/
/* lib/dns_addr.c	( NTHU CS MapleBBS Ver 3.00 )	 */
/*-------------------------------------------------------*/
/* target : included C file for DNS routines		 */
/* create : 96/11/20					 */
/* update : 96/12/15					 */
/*-------------------------------------------------------*/

/* ----------------------------------------------------- */
/* get IP address by host name				 */
/* ----------------------------------------------------- */

unsigned long
dns_addr(
  char *name
)
{
  ip_addr addr;
  u_char *cp, *eom;
  int cc, n, type, ancount, qdcount;
  querybuf ans;
  char hostbuf[MAXDNAME];

  /* disallow names consisting only of digits/dots, unless they end in a dot. */

  cc = name[0];
  if (cc >= '0' && cc <= '9')
  {
    for (cp = name;; ++cp)
    {
      cc = *cp;
      if (!cc)
      {
	/*
	 * All-numeric, no dot at the end. Fake up a hostent as if we'd
	 * actually done a lookup.  What if someone types 255.255.255.255?
	 * The test below will succeed spuriously... ???
	 */

	if (*--cp == '.')
	  break;

	return inet_addr(name);
      }
      if ((cc < '0' || cc > '9') && cc != '.')
	break;
    }
  }

  n = dns_query(name, T_A, &ans);
  if (n < 0)
    return INADDR_NONE;

  /* find first satisfactory answer */

  cp = (u_char *) & ans + sizeof(HEADER);
  eom = (u_char *) & ans + n;

  for (qdcount = ntohs(ans.hdr.qdcount); qdcount--; cp += n + QFIXEDSZ)
  {
    if ((n = dn_skipname(cp, eom)) < 0)
      return INADDR_NONE;
  }

  ancount = ntohs(ans.hdr.ancount);
  while (--ancount >= 0 && cp < eom)
  {
    if ((n = dn_expand((u_char *) &ans, eom, cp, hostbuf, MAXDNAME)) < 0)
      return INADDR_NONE;

    cp += n;
    type = getshort(cp);
    n = getshort(cp + 8);
    cp += 10;

    if (type == T_A)
    {

      addr.d[0] = cp[0];
      addr.d[1] = cp[1];
      addr.d[2] = cp[2];
      addr.d[3] = cp[3];
      return addr.addr;
    }

    cp += n;
  }
  return INADDR_NONE;
}

/*-------------------------------------------------------*/
/* lib/dns_ident.c      ( NTHU CS MapleBBS Ver 3.00 )    */
/*-------------------------------------------------------*/
/* target : included C file for DNS routines             */
/* create : 96/11/20                                     */
/* update : 96/12/15                                     */
/*-------------------------------------------------------*/

/* ----------------------------------------------------- */
/* get remote host  / user name                          */
/* ----------------------------------------------------- */

/*
 * dns_ident() speaks a common subset of the RFC 931, AUTH, TAP, IDENT and
 * RFC 1413 protocols. It queries an RFC 931 etc. compatible daemon on a
 * remote host to look up the owner of a connection. The information should
 * not be used for authentication purposes.
 */

#define RFC931_PORT     113     /* Semi-well-known port */
#define ANY_PORT        0       /* Any old port will do */


#define RFC931_TIMEOUT          /* Thor.991215: 是否提供 timeout */

#ifdef RFC931_TIMEOUT
static const int timeout = 1;   /* 若 1 秒後連線未完成，則放棄 */

static void
pseudo_handler()        /* Thor.991215: for timeout */
{
  /* connect time out */
}
#endif

/* Thor.990325: 為了讓反查時能確定查出，來自哪個interface就從那連回，以防反查不到 */

void
dns_ident(
  int sock,  /* Thor.990330: 負數保留給, 用getsock無法抓出正確port的時候.
                             代表 Port, 不過不太可能用到 */
  struct sockaddr_in *from,
  char *rhost,
  char *ruser
)
{
  struct sockaddr_in rmt_sin;
  struct sockaddr_in our_sin;
  unsigned rmt_port, rmt_pt;
  unsigned our_port, our_pt;
  int s, cc;
  char buf[512];

#ifdef RFC931_TIMEOUT
  unsigned old_alarm; /* Thor.991215: for timeout */
  struct sigaction act, oact;
#endif

  *ruser = '\0';

  /* get remote host name */

  if (dns_name((char *) &from->sin_addr, rhost))
    return;                     /* 假設沒有 FQDN 就沒有跑 identd */

  /*
   * Use one unbuffered stdio stream for writing to and for reading from the
   * RFC931 etc. server. This is done because of a bug in the SunOS 4.1.x
   * stdio library. The bug may live in other stdio implementations, too.
   * When we use a single, buffered, bidirectional stdio stream ("r+" or "w+"
   * mode) we read our own output. Such behaviour would make sense with
   * resources that support random-access operations, but not with sockets.
   */

  /* Thor.990325: 為了讓反查時能確定查出，來自哪個interface就從那連回，以防反查不到  */
  /* Thor.990330: 負數保留給，用getsock無法抓出正確port的時候，不太可能 */
  if (sock >= 0)
  {
    s = sizeof(our_sin);

    if (getsockname(sock, (struct sockaddr *) &our_sin, &s) < 0)
      return;

    /* Thor.990325: 為了讓反查時能確定查出，來自哪個interface就從那連回 */
    our_pt = ntohs(our_sin.sin_port);
    our_sin.sin_port = htons(ANY_PORT);
  }
  else
  {
    our_pt = -sock;
  }

  if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    return;

  /*
   * Bind the local and remote ends of the query socket to the same IP
   * addresses as the connection under investigation. We go through all this
   * trouble because the local or remote system might have more than one
   * network address. The RFC931 etc. client sends only port numbers; the
   * server takes the IP addresses from the query socket.
   */

#ifdef RFC931_TIMEOUT
  /* Thor.991215: set for timeout */
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
  act.sa_handler = pseudo_handler; /* SIG_IGN; */
  sigaction(SIGALRM, &act, &oact);

  old_alarm = alarm(timeout);
#endif

  rmt_sin = *from;
  rmt_pt = ntohs(rmt_sin.sin_port);
  rmt_sin.sin_port = htons(RFC931_PORT);

  /* Thor.990325: 為了讓反查時能確定查出，來自哪個interface就從那連回 */
  if ((sock < 0 || !bind(s, (struct sockaddr *) & our_sin, sizeof(our_sin)))
    && !connect(s, (struct sockaddr *) & rmt_sin, sizeof(rmt_sin)))
  {
    /*
     * Send query to server. Neglect the risk that a 13-byte write would have
     * to be fragmented by the local system and cause trouble with buggy
     * System V stdio libraries.
     */

    sprintf(buf, "%u,%u\r\n", rmt_pt, our_pt);
    write(s, buf, strlen(buf));

    cc = read(s, buf, sizeof(buf));
    if (cc > 0)
    {
      buf[cc] = '\0';

      if (sscanf(buf, "%u , %u : USERID :%*[^:]:%79s", &rmt_port, &our_port, ruser) == 3 &&
        rmt_pt == rmt_port && our_pt == our_port)
      {
        /*
         * Strip trailing carriage return. It is part of the protocol, not
         * part of the data.
         */

        for (;;)
        {
          cc = *ruser;

          if (cc == '\r')
          {
            *ruser = '\0';
            break;
          }

          if (cc == '\0')
            break;

          ruser++;
        }
      }
    }
  }

#ifdef RFC931_TIMEOUT
  /* Thor.991215: recover for timeout */
  alarm(old_alarm);
  sigaction(SIGALRM, &oact, NULL);
#endif

  close(s);
}

/*-------------------------------------------------------*/
/* lib/dns_name.c	( NTHU CS MapleBBS Ver 3.00 )	 */
/*-------------------------------------------------------*/
/* target : included C file for DNS routines		 */
/* create : 96/11/20					 */
/* update : 96/12/15					 */
/*-------------------------------------------------------*/

/* ----------------------------------------------------- */
/* get host name by IP address				 */
/* ----------------------------------------------------- */

int
dns_name(
  unsigned char *addr,
  char *name
)
{
  querybuf ans;
  char qbuf[MAXDNAME];
  char hostbuf[MAXDNAME];
  int n, type, ancount, qdcount;
  u_char *cp, *eom;
#ifdef  HAVE_ETC_HOSTS
  char abuf[256];
  FILE *fp;
#endif     

  sprintf(name, INADDR_FMT, addr[0], addr[1], addr[2], addr[3]);

#ifdef	HAVE_ETC_HOSTS
  /* lkchu: reference to etc/hosts */

  if ( ( fp = fopen("etc/hosts", "r") ) )
  {
    while (fgets(abuf, sizeof(abuf), fp))
    {
      if (abuf[0] != '#')
      {
        /* if (!strcmp(name, (char *) strtok(abuf, " \t\r\n"))) */
        if (strstr(name, (char *) strtok(abuf, " \t\r\n")))
        {
          strcpy(name, (char *) strtok(NULL, " \t\r\n"));
          fclose(fp);
          return 0;
        }
      }
    }
    fclose(fp);
  }
#endif

  sprintf(qbuf, INADDR_FMT ".in-addr.arpa",
    addr[3], addr[2], addr[1], addr[0]);

  n = dns_query(qbuf, T_PTR, &ans);
  if (n < 0)
    return n;

  /* find first satisfactory answer */

  cp = (u_char *) & ans + sizeof(HEADER);
  eom = (u_char *) & ans + n;

  for (qdcount = ntohs(ans.hdr.qdcount); qdcount--; cp += n + QFIXEDSZ)
  {
    if ((n = dn_skipname(cp, eom)) < 0)
      return n;
  }

  for (ancount = ntohs(ans.hdr.ancount); --ancount >= 0 && cp < eom; cp += n)
  {
    if ((n = dn_expand((u_char *) &ans, eom, cp, hostbuf, MAXDNAME)) < 0)
      return n;

    cp += n;
    type = getshort(cp);
    n = getshort(cp + 8);
    cp += 10;

    if (type == T_PTR)
    {
      if ((n = dn_expand((u_char *) &ans, eom, cp, hostbuf, MAXDNAME)) >= 0)
      {
	strcpy(name, hostbuf);
	return 0;
      }
    }

#if 0
    if (type == T_CNAME)
    {
      strcpy(name, hostbuf);
      return 0;
    }
#endif
  }

  return -1;
}
/*-------------------------------------------------------*/
/* lib/dns_open.c	( NTHU CS MapleBBS Ver 3.00 )	 */
/*-------------------------------------------------------*/
/* target : included C file for DNS routines		 */
/* create : 96/11/20					 */
/* update : 96/12/15					 */
/*-------------------------------------------------------*/

int
dns_open(
  char *host,
  int port
)
{
  querybuf ans;
  int n, ancount, qdcount;
  unsigned char *cp, *eom;
  char buf[MAXDNAME];
  struct sockaddr_in sin;
  int type;

#if 1
  /* Thor.980707: 因gem.c呼叫時可能將host用ip放入，故作特別處理 */
  if(*host>='0' && *host<='9')
  {
    for(n=0, cp=host; n<4; n++, cp++)
    {
      buf[n]=0;
      while(*cp >= '0' && *cp <='9')
      {
        buf[n] *= 10;
        buf[n] += *cp++ - '0';
      }
      if (!*cp) 
        break;
    }
    if (n==3) 
    {
      cp = buf;
      goto ip;
    }
  }
  /* Thor.980707: 隨便寫寫，要講究完全match再說:p */ 
#endif 

  n = dns_query(host, T_A, &ans);
  if (n < 0)
    return n;

  /* find first satisfactory answer */

  cp = (u_char *) & ans + sizeof(HEADER);
  eom = (u_char *) & ans + n;

  for (qdcount = ntohs(ans.hdr.qdcount); qdcount--; cp += n + QFIXEDSZ)
  {
    if ((n = dn_skipname(cp, eom)) < 0)
      return n;
  }

  ancount = ntohs(ans.hdr.ancount);

  while (--ancount >= 0 && cp < eom)
  {
    if ((n = dn_expand((u_char *) &ans, eom, cp, buf, MAXDNAME)) < 0)
      return -1;

    cp += n;

    type = getshort(cp);
    n = getshort(cp + 8);
    cp += 10;

    if (type == T_A)
    {
      int sock;
      ip_addr *ip;
#if 1
ip:
#endif

      sin.sin_family = AF_INET;
      sin.sin_port = htons(port);
      memset(sin.sin_zero, 0, sizeof(sin.sin_zero));

      ip = (ip_addr *) & (sin.sin_addr.s_addr);

      ip->d[0] = cp[0];
      ip->d[1] = cp[1];
      ip->d[2] = cp[2];
      ip->d[3] = cp[3];

      sock = socket(AF_INET, SOCK_STREAM, 0);
      if (sock < 0)
	return sock;

      if (!connect(sock, (struct sockaddr *) & sin, sizeof(sin)))
	return sock;

      close(sock);
    }

    cp += n;
  }

  return -1;
}
/*-------------------------------------------------------*/
/* lib/dns_smtp.c	( NTHU CS MapleBBS Ver 3.00 )	 */
/*-------------------------------------------------------*/
/* target : included C file for DNS routines		 */
/* create : 96/11/20					 */
/* update : 96/12/15					 */
/*-------------------------------------------------------*/

static inline void
dns_mx(
  char *domain,
  char *mxlist
)
{
  querybuf ans;
  int n, ancount, qdcount;
  unsigned char *cp, *eom;
  int type;

  *mxlist = 0;

  n = dns_query(domain, T_MX, &ans);

  if (n < 0)
    return;

  /* find first satisfactory answer */

  cp = (u_char *) & ans + sizeof(HEADER);
  eom = (u_char *) & ans + n;

  for (qdcount = ntohs(ans.hdr.qdcount); qdcount--; cp += n + QFIXEDSZ)
  {
    if ((n = dn_skipname(cp, eom)) < 0)
      return;
  }

  ancount = ntohs(ans.hdr.ancount);
  domain = mxlist + MAX_MXLIST - MAXDNAME - 2;

  while (--ancount >= 0 && cp < eom)
  {
    if ((n = dn_expand((u_char *) &ans, eom, cp, mxlist, MAXDNAME)) < 0)
      break;

    cp += n;

    type = getshort(cp);
    n = getshort(cp + 8);
    cp += 10;

    if (type == T_MX)
    {
      /* pref = getshort(cp); */
      *mxlist = '\0';
      if ((dn_expand((u_char *) &ans, eom, cp + 2, mxlist, MAXDNAME)) < 0)
	break;

      if (!*mxlist)
	return;

      /* Thor.980820: 將數個 MX entry 用 : 串起來以便一個一個試 */
      while (*mxlist)
	mxlist++;
      *mxlist++ = ':';

      if (mxlist >= domain)
	break;
    }

    cp += n;
  }

  *mxlist = '\0';
}


int
dns_smtp(
  char *host
)
{
  int sock;
  char *str, *ptr, mxlist[MAX_MXLIST];

  dns_mx(host, str = mxlist);
  if (!*str)
    str = host;

  for (;;)
  { /* Thor.980820: 註解: 萬一host格式為 xxx:yyy:zzz, 則先試 xxx,不行再試 yyy */
    ptr = str;
    while ( ( sock = *ptr ) )
    {
      if (sock == ':')
      {
	*ptr++ = '\0';
	break;
      }
      ptr++;
    }

    if (!*str)
      return -1;

    sock = dns_open(str, 25);
    if (sock >= 0)
      return sock;

    str = ptr;
  }
}
