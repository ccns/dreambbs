/*-------------------------------------------------------*/
/* lib/dns.c		( NTHU CS MapleBBS Ver 3.00 )	 */
/*-------------------------------------------------------*/
/* target : included C file for DNS routines		 */
/* create : 96/11/20					 */
/* update : 96/12/15					 */
/*-------------------------------------------------------*/


#include "dns.h"


void
dns_init()
{
  res_init();
  /* _res.retrans = 5; */ /* DNS query timeout */
  _res.retry = 2;
//  _res.options |= RES_USEVC;
}


int
dns_query(name, qtype, ans)
  char *name;			/* domain name */
  int qtype;			/* type of query */
  querybuf *ans;		/* buffer to put answer */
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
