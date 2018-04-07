/*-------------------------------------------------------*/
/* lib/dns_open.c	( NTHU CS MapleBBS Ver 3.00 )	 */
/*-------------------------------------------------------*/
/* target : included C file for DNS routines		 */
/* create : 96/11/20					 */
/* update : 96/12/15					 */
/*-------------------------------------------------------*/


#include "dns.h"
#include <unistd.h>
#include <string.h>

int
dns_open(host, port)
  char *host;
  int port;
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
