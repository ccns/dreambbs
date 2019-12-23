#ifndef DNS_H

#define DNS_H
/*-------------------------------------------------------*/
/* lib/dns.h            ( NTHU CS MapleBBS Ver 3.00 )    */
/*-------------------------------------------------------*/
/* target : header file for DNS routines                 */
/* create : 96/11/20                                     */
/* update : 96/12/15                                     */
/*-------------------------------------------------------*/

#define _DEFAULT_SOURCE 1

#include "config.h"

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
#define INADDR_NONE     0xffffffff
#endif
#define INADDR_FMT      "%u.%u.%u.%u"

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
 * untruncated answer in case the supplied answer buffer it not big enough
 * to accommodate the entire answer.
 */

#if     PACKETSZ > 1024
#define MAXPACKET       PACKETSZ
#else
#define MAXPACKET       1024    /* max packet size used internally by BIND */
#endif

#define MAX_MXLIST      1024

typedef union
{
    HEADER hdr;
    unsigned char buf[MAXPACKET];
} querybuf;                     /* response of DNS query */

static inline unsigned short
getshort(const unsigned char *c)
{
    unsigned short u;

    u = c[0];
    return (u << 8) + c[1];
}

#endif  /* DNS_H */
