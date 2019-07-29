/*-------------------------------------------------------*/
/* lib/acl_addr.c       ( NTHU CS MapleBBS Ver 3.00 )    */
/*-------------------------------------------------------*/
/* target : Access Control List                          */
/* create : 98/03/20                                     */
/* update : 98/03/29                                     */
/*-------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include "dao.h"

/* ----------------------------------------------------- */
/* ACL config file format                                */
/* ----------------------------------------------------- */
/* user:        majordomo@* bad@cs.nthu.edu.tw           */
/* host:        cs.nthu.edu.tw  140.114.77.1             */
/* subnet:      .nthu.edu.tw    140.114.77.              */
/* ----------------------------------------------------- */


/* input : �q�L not_addr �ˬd���r�� */
/* return -1 : �������~�� E-mail address */
/* return -2 : ���X�椧�{�Ҧa�} */


int acl_addr(char *acl,            /* file name of access control list */
             char *addr)
{
    int i, cc, luser, lhost;
    FILE *fp;
    char buf[128], filter[256], *host, *str;
    const char *str_invalid;

    const char *invalid[] = { "@bbs", "bbs@", "root@", "gopher@",
        "guest@", "@ppp", "@slip", "@dial", "unknown@", "@anon.penet.fi",
        "193.64.202.3", NULL
    };

    str_lower(buf, addr);
    host = (char *)strchr(buf, '@');

    for (i = 0; (str_invalid = invalid[i]); i++)
    {
        if (strstr(buf, str_invalid))
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

        luser = host - buf;     /* length of user name */
        lhost = addr - host;    /* length of host name */

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

            if (str == filter)    /* empty line */
                continue;

            *str = '\0';

            if (addr)            /* match user name */
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
                ((cc < lhost) && (*addr == '.')
                 && !strcmp(addr, host + lhost - cc)))
            {
                i = -2;
                break;
            }
        }

        fclose(fp);
    }

    return i;
}

/*-------------------------------------------------------*/
/* lib/acl_has.c        ( NTHU CS MapleBBS Ver 3.00 )    */
/*-------------------------------------------------------*/
/* target : Access Control List                          */
/* create : 98/03/20                                     */
/* update : 98/03/29                                     */
/*-------------------------------------------------------*/

/* ----------------------------------------------------- */
/* ACL config file format                                */
/* ----------------------------------------------------- */
/* user:        majordomo@* bad@cs.nthu.edu.tw           */
/*                        ^ Thor.980825: �����ť�        */
/* host:        cs.nthu.edu.tw  140.114.77.1             */
/* subnet:      .nthu.edu.tw    140.114.77.              */
/* ----------------------------------------------------- */


/* return -1 : ACL file ���s�b */
/* return 0 : ACL ���]�t�� pattern */
/* return 1 : ACL �ŦX�� pattern */


int acl_has(const char *acl,      /* file name of access control list */
            const char *user,     /* lower-case string */
            char *host            /* lower-case string */
    )
{
    int i, cc, luser, lhost;
    FILE *fp;
    char filter[256], *addr, *str;

    if (!(fp = fopen(acl, "r")))
        return -1;

    i = 0;
    luser = strlen(user);        /* length of user name */
    lhost = strlen(host);        /* length of host name */

    while (fgets(filter, sizeof(filter), fp))
    {
        addr = NULL;

        for (str = filter; (cc = *str) > ' '; str++)
        {                        /* Thor.980825: ����: �J�� �ť� �N�⦹�浲�� */
            if (cc == '@')
                addr = str;
        }

        if (str == filter)       /* empty line */
            continue;

        *str = '\0';             /* Thor.980825: ����: �N�����B��0, �K�ͪK�` */
        str_lower(filter, filter);   /* lkchu.981201: lower-case string */

        if (addr)                /* match user name */
        {
            if ((luser != addr - filter) || memcmp(user, filter, luser))
                continue;

            if (!*++addr)
            {
                i = 1;
                break;
            }
        }
        else
        {
            addr = filter;
        }

        /* match host name */

        cc = str - addr;

        if (cc > lhost)
            continue;

        if (cc == lhost)
        {
            if (memcmp(addr, host, lhost))
                continue;
        }
        else
        {
            if (((*addr != '.') || memcmp(addr, host + lhost - cc, cc)) &&
                ((addr[cc - 1] != '.') || memcmp(addr, host, cc)))
                continue;
        }

        i = 1;
        break;
    }

    fclose(fp);
    return i;
}
