#ifndef	_DNS_H_

#define _DNS_H_

/*-------------------------------------------------------*/
/* lib/dns.h		( NTHU CS MapleBBS Ver 3.00 )	 */
/*-------------------------------------------------------*/
/* target : header file for DNS routines		 */
/* create : 96/11/20					 */
/* update : 96/12/15					 */
/*-------------------------------------------------------*/

#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ctype.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include "hdr.h"
#include "dao.h"

#ifndef INADDR_NONE
#define	INADDR_NONE	0xffffffff
#endif
#define	INADDR_FMT	"%u.%u.%u.%u"


typedef union
{
  unsigned char d[4];
  unsigned long addr;
}     ip_addr;


/*
 * The standard udp packet size PACKETSZ (512) is not sufficient for some
 * nameserver answers containing very many resource records. The resolver may
 * switch to tcp and retry if it detects udp packet overflow. Also note that
 * the resolver routines res_query and res_search return the size of the
 * un*truncated answer in case the supplied answer buffer it not big enough
 * to accommodate the entire answer.
 */


#if	PACKETSZ > 1024
#define MAXPACKET       PACKETSZ
#else
#define	MAXPACKET	1024	/* max packet size used internally by BIND */
#endif


#define MAX_MXLIST      1024


typedef union
{
  HEADER hdr;
  u_char buf[MAXPACKET];
}     querybuf;			/* response of DNS query */

static inline unsigned short
getshort(c)
  unsigned char *c;
{
  unsigned short u;

  u = c[0];
  return (u << 8) + c[1];
}

/* dns.c */
void dns_init(void);
int dns_query(char *name, int qtype, querybuf *ans);
/* dns_addr.c */
unsigned long dns_addr(char *name);
/* dns_name.c */
int dns_name(unsigned char *addr, char *name);
/* dns_smtp.c */
int dns_smtp(char *host);
/* dns_ident.c */
void dns_ident(int sock, struct sockaddr_in *from, char *rhost, char *ruser);
/* dns_open.c */
int dns_open(char *host, int port);

#endif	/* _DNS_H_ */
