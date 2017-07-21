/*-------------------------------------------------------*/
/* lib/dns_smtp.c	( NTHU CS MapleBBS Ver 3.00 )	 */
/*-------------------------------------------------------*/
/* target : included C file for DNS routines		 */
/* create : 96/11/20					 */
/* update : 96/12/15					 */
/*-------------------------------------------------------*/


#include "dns.h"


static inline void
dns_mx(domain, mxlist)
  char *domain;
  char *mxlist;
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

      /* Thor.980820: �N�ƭ� MX entry �� : ��_�ӥH�K�@�Ӥ@�Ӹ� */
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
dns_smtp(host)
  char *host;
{
  int sock;
  char *str, *ptr, mxlist[MAX_MXLIST];

  dns_mx(host, str = mxlist);
  if (!*str)
    str = host;

  for (;;)
  { /* Thor.980820: ����: �U�@host�榡�� xxx:yyy:zzz, �h���� xxx,����A�� yyy */
    ptr = str;
    while (sock = *ptr)
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
