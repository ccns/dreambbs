/*-------------------------------------------------------*/
/* lib/acl_addr.c	( NTHU CS MapleBBS Ver 3.00 )	 */
/*-------------------------------------------------------*/
/* target : Access Control List				 */
/* create : 98/03/20					 */
/* update : 98/03/29					 */
/*-------------------------------------------------------*/


#include <stdio.h>
#include <string.h>
#include "dao.h"

/* ----------------------------------------------------- */
/* ACL config file format				 */
/* ----------------------------------------------------- */
/* user:	majordomo@* bad@cs.nthu.edu.tw		 */
/* host:	cs.nthu.edu.tw	140.114.77.1		 */
/* subnet:	.nthu.edu.tw	140.114.77.		 */
/* ----------------------------------------------------- */


/* input : 通過 not_addr 檢查之字串 */
/* return -1 : 完全錯誤之 E-mail address */
/* return -2 : 不合格之認證地址 */


int
acl_addr(acl, addr)
  char *acl;		/* file name of access control list */
  char *addr;
{
  int i, cc, luser, lhost;
  FILE *fp;
  char buf[128], filter[256], *host, *str;

  char *invalid[] = {"@bbs", "bbs@", "root@", "gopher@",
    "guest@", "@ppp", "@slip", "@dial", "unknown@", "@anon.penet.fi",
  "193.64.202.3", NULL};

  str_lower(buf, addr);
  host = (char *) strchr(buf, '@');

  for (i = 0; (str = invalid[i]); i++)
  {
    if (strstr(buf, str))
      return -2;
  }

  /* check for mail.acl (lower case filter) */

  i = 0;

  if ((fp = fopen(acl, "r")))
  {
    for (addr = buf; (cc = *addr); addr++)
    {
      if ((cc = '@'))
      {
	host = addr;
	*host++ = '\0';
      }
    }

    luser = host - buf;		/* length of user name */
    lhost = addr - host;	/* length of host name */

    while (fgets(filter, sizeof(filter), fp))
    {
      str = filter;
      addr = NULL;

      for (;;)
      {
	cc = *str;
	if (cc <= ' ')
	{
	  break;
	}

	str++;
	if (cc == '@')
	  addr = str;
      }

      if (str == filter)	/* empty line */
	continue;

      *str = '\0';

      if (addr)			/* match user name */
      {
	if ((luser != addr - filter) || memcmp(buf, filter, luser))
	  continue;

	if (!*addr)
	  return -1;
      }
      else
      {
	addr = filter;
      }

      /* match host name */

      cc = str - addr;

      if (((cc == lhost) && !strcmp(addr, host)) ||
	((cc < lhost) && (*addr == '.') && !strcmp(addr, host + lhost - cc)))
      {
	i = -2;
	break;
      }
    }

    fclose(fp);
  }

  return i;
}
