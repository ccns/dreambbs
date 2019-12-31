/*-------------------------------------------------------*/
/* lib/dns.c            ( NTHU CS MapleBBS Ver 3.00 )    */
/*-------------------------------------------------------*/
/* target : included C file for DNS routines             */
/* create : 96/11/20                                     */
/* update : 96/12/15                                     */
/*-------------------------------------------------------*/

#include "config.h"             /* lkchu.981201: 是否有 define HAVE_ETC_HOSTS */

#include "dao.h"
#include <signal.h>
#include <unistd.h>
#include <string.h>

void dns_init(void)
{
    res_init();
    /* _res.retrans = 5; *//* DNS query timeout */
    _res.retry = 2;
    //  _res.options |= RES_USEVC;
}


int dns_query(const char *name, /* domain name */
              int qtype,        /* type of query */
              querybuf * ans    /* buffer to put answer */
    )
{
    querybuf buf;

    qtype =
        res_mkquery(QUERY, name, C_IN, qtype, (unsigned char *)NULL, 0, NULL,
                    (unsigned char *)&buf, sizeof(buf));

    if (qtype >= 0)
    {
        qtype =
            res_send((unsigned char *)&buf, qtype, (unsigned char *)ans,
                     sizeof(querybuf));

        /* avoid problems after truncation in tcp packets */

        if (qtype > sizeof(querybuf))
            qtype = sizeof(querybuf);
    }

    return qtype;
}

/*-------------------------------------------------------*/
/* lib/dns_addr.c       ( NTHU CS MapleBBS Ver 3.00 )    */
/*-------------------------------------------------------*/
/* target : included C file for DNS routines             */
/* create : 96/11/20                                     */
/* update : 96/12/15                                     */
/*-------------------------------------------------------*/

/* ----------------------------------------------------- */
/* get IP address by host name                           */
/* ----------------------------------------------------- */

ip_addr dns_addr(const char *name)
{
    static const int qtypes[2] = {T_A, T_AAAA};
    struct addrinfo hints = {0};
    struct addrinfo *hosts;
    ip_addr addr = {0};

    /* disallow names consisting only of digits/dots, unless they end in a dot. */
    hints.ai_family = AF_UNSPEC;
    hints.ai_flags = AI_V4MAPPED | AI_ADDRCONFIG | AI_NUMERICHOST | AI_NUMERICSERV;
    {
        int status = getaddrinfo(name, NULL, &hints, &hosts);
        if (status)
        {
            switch (addr.family = hosts->ai_family)
            {
            case AF_INET:
                addr.v4 = *(const struct sockaddr_in *)hosts->ai_addr;
                break;
            case AF_INET6:
                addr.v6 = *(const struct sockaddr_in6 *)hosts->ai_addr;
                break;
            default:;
            }
            freeaddrinfo(hosts);
            return addr;
        }
        else if (status != EAI_NONAME)
            return IPADDR_NONE;
    }

    for (int i = 0; i < COUNTOF(qtypes); ++i)
    {
        querybuf ans;
        unsigned char *cp, *eom;
        {
            int n = dns_query(name, qtypes[i], &ans);
            if (n < 0)
                return IPADDR_NONE;

            /* find first satisfactory answer */

            cp = (unsigned char *)&ans + sizeof(HEADER);
            eom = (unsigned char *)&ans + n;
        }

        for (int qdcount = ntohs(ans.hdr.qdcount); qdcount--;)
        {
            int n = dn_skipname(cp, eom);
            if (n < 0)
                return IPADDR_NONE;
            cp += n + QFIXEDSZ;
        }

        for (int ancount = ntohs(ans.hdr.ancount); --ancount >= 0 && cp < eom;)
        {
            char hostbuf[MAXDNAME];
            int type;
            int n = dn_expand((unsigned char *)&ans, eom, cp, hostbuf, MAXDNAME);
            if (n < 0)
                return IPADDR_NONE;

            cp += n;
            type = getshort(cp);
            n = getshort(cp + 8);
            cp += 10;

            if (type == qtypes[i])
            {
                ip_addr addr = {0};
                switch (type)
                {
                case T_A:
                    addr.family = AF_INET;
                    addr.v4.sin_addr = *(struct in_addr *) cp;
                    return addr;
                case T_AAAA:
                    addr.family = AF_INET6;
                    addr.v6.sin6_addr = *(struct in6_addr *) cp;
                    return addr;
                default:
                    return IPADDR_NONE;
                }
            }

            cp += n;
        }
    }
    return IPADDR_NONE;
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

#define RFC931_PORT     113        /* Semi-well-known port */
#define ANY_PORT        0        /* Any old port will do */


#define RFC931_TIMEOUT            /* Thor.991215: 是否提供 timeout */

#ifdef RFC931_TIMEOUT
static const int timeout = 1;    /* 若 1 秒後連線未完成，則放棄 */

static void pseudo_handler(GCC_UNUSED int signum) /* Thor.991215: for timeout */
{
    /* connect time out */
}
#endif

/* Thor.990325: 為了讓反查時能確定查出，來自哪個interface就從那連回，以防反查不到 */

void dns_ident(int sock,        /* Thor.990330: 負數保留給, 用getsock無法抓出正確port的時候.
                                   代表 Port, 不過不太可能用到 */
               const ip_addr *from, char *rhost, int rhost_sz, char *ruser, int ruser_sz)
{
    struct addrinfo rmt_hints, *rmt_hosts;
    struct addrinfo our_hints, *our_hosts;
    char rmt_pt[NI_MAXSERV];
    char our_pt[NI_MAXSERV];

#ifdef RFC931_TIMEOUT
    unsigned old_alarm;            /* Thor.991215: for timeout */
    struct sigaction oact;
#endif

    *ruser = '\0';

    /* get remote host name */

    if (dns_name(from, rhost, rhost_sz))
        return;                    /* 假設沒有 FQDN 就沒有跑 identd */

#ifdef RFC931_TIMEOUT
    {
        struct sigaction act;
        /* Thor.991215: set for timeout */
        sigemptyset(&act.sa_mask);
        act.sa_flags = 0;
        act.sa_handler = pseudo_handler;    /* SIG_IGN; */
        sigaction(SIGALRM, &act, &oact);

        old_alarm = alarm(timeout);
    }
#endif

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
        struct sockaddr_storage our_sin;
        int sz = sizeof(our_sin);
        char our_lport[12];

        if (getsockname(sock, (struct sockaddr *)&our_sin, (socklen_t *) &sz) < 0)
            return;

        /* Thor.990325: 為了讓反查時能確定查出，來自哪個interface就從那連回 */
        our_hints.ai_family = our_sin.ss_family;
        our_hints.ai_flags = AI_V4MAPPED | AI_ADDRCONFIG | AI_NUMERICSERV | AI_PASSIVE;
        getnameinfo((struct sockaddr *)&our_sin, sz, NULL, NI_MAXHOST, our_pt, sizeof(our_pt), 0);
        sprintf(our_lport, "%d", ANY_PORT);

        if (getaddrinfo(NULL, our_lport, &our_hints, &our_hosts))
            goto cleanup_alarm;
    }
    else
    {
        our_hints.ai_next = NULL;
        our_hosts = &our_hints;
        sprintf(our_pt, "%d", -sock);
    }

    /*
     * Bind the local and remote ends of the query socket to the same IP
     * addresses as the connection under investigation. We go through all this
     * trouble because the local or remote system might have more than one
     * network address. The RFC931 etc. client sends only port numbers; the
     * server takes the IP addresses from the query socket.
     */

    {
        char host_str[NI_MAXHOST];
        char rmt_lport[12];
        rmt_hints.ai_family = from->family;
        rmt_hints.ai_flags = AI_V4MAPPED | AI_ADDRCONFIG | AI_NUMERICSERV;
        getnameinfo((const struct sockaddr *)from, sizeof(*from), host_str, sizeof(host_str), rmt_pt, sizeof(rmt_pt), NI_NUMERICHOST);
        sprintf(rmt_lport, "%d", RFC931_PORT);

        if (getaddrinfo(host_str, rmt_lport, &rmt_hints, &rmt_hosts))
            goto cleanup_our;
    }

    for (struct addrinfo *our_h = our_hosts; our_h; our_h = our_h->ai_next)
    {
        for (struct addrinfo *rmt_h = rmt_hosts; rmt_h; rmt_h = our_h->ai_next)
        {
            int s = socket(rmt_h->ai_family, rmt_h->ai_socktype, 0);
            if (s < 0)
                goto cleanup_rmt;

            /* Thor.990325: 為了讓反查時能確定查出，來自哪個interface就從那連回 */
            if ((sock < 0 || !bind(s, our_h->ai_addr, our_h->ai_addrlen))
                && !connect(s, rmt_h->ai_addr, rmt_h->ai_addrlen))
            {
                /*
                 * Send query to server. Neglect the risk that a 13-byte write would have
                 * to be fragmented by the local system and cause trouble with buggy
                 * System V stdio libraries.
                 */

                char buf[512];
                int cc;
                sprintf(buf, "%s,%s\r\n", rmt_pt, our_pt);
                write(s, buf, strlen(buf));

                cc = read(s, buf, sizeof(buf));
                if (cc > 0)
                {
                    char rmt_port[NI_MAXSERV];
                    char our_port[NI_MAXSERV];
                    char ruser_buf[80];

                    buf[cc] = '\0';

                    if (sscanf
                        (buf, "%s , %s : USERID :%*[^:]:%79s", rmt_port, our_port,
                         ruser_buf) == 3 && !strcmp(rmt_pt, rmt_port) && !strcmp(our_pt, our_port))
                    {
                        str_ncpy(ruser, ruser_buf, ruser_sz);

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
                                goto cleanup_rmt;
                            }

                            if (cc == '\0')
                                goto cleanup_rmt;

                            ruser++;
                        }
                    }
                }
            }
            else
            {
                close(s);
                s = -1;
            }

            close(s);
        }
    }
cleanup_rmt:
    freeaddrinfo(rmt_hosts);

cleanup_our:
    if (sock >= 0)
        freeaddrinfo(our_hosts);

cleanup_alarm:
#ifdef RFC931_TIMEOUT
    /* Thor.991215: recover for timeout */
    alarm(old_alarm);
    sigaction(SIGALRM, &oact, NULL);
#endif
}

/*-------------------------------------------------------*/
/* lib/dns_name.c       ( NTHU CS MapleBBS Ver 3.00 )    */
/*-------------------------------------------------------*/
/* target : included C file for DNS routines             */
/* create : 96/11/20                                     */
/* update : 96/12/15                                     */
/*-------------------------------------------------------*/

/* ----------------------------------------------------- */
/* get host name by IP address                           */
/* ----------------------------------------------------- */

int dns_name(const ip_addr *addr, char *name, int name_sz)
{
    querybuf ans;
    unsigned char *cp, *eom;
#ifdef  HAVE_ETC_HOSTS
    FILE *fp;
#endif

    if (getnameinfo((const struct sockaddr *)addr, sizeof(*addr), name, name_sz, NULL, NI_MAXSERV, NI_NUMERICHOST))
        return -1;

#ifdef  HAVE_ETC_HOSTS
    /* lkchu: reference to etc/hosts */

    if ((fp = fopen("etc/hosts", "r")))
    {
        char abuf[256];
        while (fgets(abuf, sizeof(abuf), fp))
        {
            if (abuf[0] != '#')
            {
                /* if (!strcmp(name, (char *) strtok(abuf, " \t\r\n"))) */
                if (strstr(name, (char *)strtok(abuf, " \t\r\n")))
                {
                    str_ncpy(name, (char *)strtok(NULL, " \t\r\n"), name_sz);
                    fclose(fp);
                    return 0;
                }
            }
        }
        fclose(fp);
    }
#endif

    {
        const unsigned char *ad;
        char qbuf[MAXDNAME];
        int n;

        switch (addr->family)
        {
        case AF_INET:
            ad = (const unsigned char *) &addr->v4.sin_addr;
            sprintf(qbuf, INADDR_FMT ".in-addr.arpa",
                    ad[3], ad[2], ad[1], ad[0]);
            break;
        case AF_INET6:
            ad = (const unsigned char *) &addr->v6.sin6_addr;
            sprintf(qbuf, INADDR6_FMT ".ip6.arpa",
                    ad[15] >> 4, ad[15] & 0xf, ad[14] >> 4, ad[14] & 0xf,
                    ad[13] >> 4, ad[13] & 0xf, ad[12] >> 4, ad[12] & 0xf,
                    ad[11] >> 4, ad[11] & 0xf, ad[10] >> 4, ad[10] & 0xf,
                    ad[9] >> 4, ad[9] & 0xf, ad[8] >> 4, ad[8] & 0xf,
                    ad[7] >> 4, ad[7] & 0xf, ad[6] >> 4, ad[6] & 0xf,
                    ad[5] >> 4, ad[5] & 0xf, ad[4] >> 4, ad[4] & 0xf,
                    ad[3] >> 4, ad[3] & 0xf, ad[2] >> 4, ad[2] & 0xf,
                    ad[1] >> 4, ad[1] & 0xf, ad[0] >> 4, ad[0] & 0xf);
            break;
        default:
            return -1;
        }

        n = dns_query(qbuf, T_PTR, &ans);
        if (n < 0)
            return n;

        /* find first satisfactory answer */

        cp = (unsigned char *)&ans + sizeof(HEADER);
        eom = (unsigned char *)&ans + n;
    }

    for (int qdcount = ntohs(ans.hdr.qdcount); qdcount--;)
    {
        int n = dn_skipname(cp, eom);
        if (n < 0)
            return n;
        cp += n + QFIXEDSZ;
    }

    for (int ancount = ntohs(ans.hdr.ancount); --ancount >= 0 && cp < eom;)
    {
        char hostbuf[MAXDNAME];
        int type;
        int n = dn_expand((unsigned char *)&ans, eom, cp, hostbuf, MAXDNAME);
        if (n < 0)
            return n;

        cp += n;
        type = getshort(cp);
        n = getshort(cp + 8);
        cp += 10;

        if (type == T_PTR)
        {
            int n = dn_expand((unsigned char *)&ans, eom, cp, hostbuf, MAXDNAME);
            if (n >= 0)
            {
                str_ncpy(name, hostbuf, name_sz);
                return 0;
            }
        }

#if 0
        if (type == T_CNAME)
        {
            str_ncpy(name, hostbuf, name_sz);
            return 0;
        }
#endif
        cp += n;
    }

    return -1;
}

/*-------------------------------------------------------*/
/* lib/dns_open.c       ( NTHU CS MapleBBS Ver 3.00 )    */
/*-------------------------------------------------------*/
/* target : included C file for DNS routines             */
/* create : 96/11/20                                     */
/* update : 96/12/15                                     */
/*-------------------------------------------------------*/

int dns_openip(const ip_addr *addr, int port)
{
    int sock;
    struct addrinfo hints = {0};
    struct addrinfo *hosts;
    char addr_str[INET6_ADDRSTRLEN], port_str[12];

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_V4MAPPED | AI_ADDRCONFIG | AI_NUMERICHOST | AI_NUMERICSERV;

    getnameinfo((const struct sockaddr *)addr, sizeof(*addr), addr_str, sizeof(port_str), NULL, NI_MAXSERV, NI_NUMERICHOST);
    sprintf(port_str, "%d", port);

    if (getaddrinfo(addr_str, port_str, &hints, &hosts))
        return -1;

    for (struct addrinfo *host = hosts; host; host = host->ai_next)
    {
        sock = socket(host->ai_family, host->ai_socktype, 0);
        if (sock < 0)
            continue;

        if (!connect(sock, host->ai_addr, host->ai_addrlen))
            break;;

        sock = -1;
        close(sock);
    }

    return sock;
}

int dns_open(const char *host, int port)
{
    static const int qtypes[2] = {T_A, T_AAAA};
    querybuf ans;
    unsigned char *cp, *eom;
    char buf[MAXDNAME];

#if 1
    /* Thor.980707: 因gem.c呼叫時可能將host用ip放入，故作特別處理 */
    if (*host >= '0' && *host <= '9')
    {
        int n;
        const unsigned char *ccp = (const unsigned char *)host;
        for (n = 0; n < 4; n++, ccp++)
        {
            buf[n] = 0;
            while (*ccp >= '0' && *ccp <= '9')
            {
                buf[n] *= 10;
                buf[n] += *ccp++ - '0';
            }
            if (!*ccp)
                break;
        }
        if (n == 3)
        {
            ip_addr addr = {0};
            addr.family = AF_INET;
            addr.v4.sin_addr = *(struct in_addr *)buf;
            return dns_openip(&addr, port);
        }
    }
    /* Thor.980707: 隨便寫寫，要講究完全match再說:p */
#endif

    for (int i = 0; i < COUNTOF(qtypes); ++i)
    {
        {
            int n = dns_query(host, qtypes[i], &ans);
            if (n < 0)
                return n;

            /* find first satisfactory answer */

            cp = (unsigned char *)&ans + sizeof(HEADER);
            eom = (unsigned char *)&ans + n;
        }

        for (int qdcount = ntohs(ans.hdr.qdcount); qdcount--;)
        {
            int n = dn_skipname(cp, eom);
            if (n < 0)
                return n;
            cp += n + QFIXEDSZ;
        }

        for (int ancount = ntohs(ans.hdr.ancount); --ancount >= 0 && cp < eom;)
        {
            int type;
            int n = dn_expand((unsigned char *)&ans, eom, cp, buf, MAXDNAME);
            if (n < 0)
                return -1;

            cp += n;

            type = getshort(cp);
            n = getshort(cp + 8);
            cp += 10;

            if (type == qtypes[i])
            {
                ip_addr addr = {0};
                int sock;
                switch (type)
                {
                case T_A:
                    addr.family = AF_INET;
                    addr.v4.sin_addr = *(struct in_addr *)buf;
                    break;
                case T_AAAA:
                    addr.family = AF_INET6;
                    addr.v6.sin6_addr = *(struct in6_addr *)buf;
                    break;
                default:
                    return -1;
                }
                sock = dns_openip(&addr, port);
                if (sock >= 0)
                    return sock;
            }

            cp += n;
        }
    }

    return -1;
}

/*-------------------------------------------------------*/
/* lib/dns_smtp.c       ( NTHU CS MapleBBS Ver 3.00 )    */
/*-------------------------------------------------------*/
/* target : included C file for DNS routines             */
/* create : 96/11/20                                     */
/* update : 96/12/15                                     */
/*-------------------------------------------------------*/

#define DNS_MXLIST_DELIMITER  ';'

static inline void dns_mx(const char *domain, char *mxlist)
{
    querybuf ans;
    unsigned char *cp, *eom;

    *mxlist = 0;

    {
        int n = dns_query(domain, T_MX, &ans);

        if (n < 0)
            return;

        /* find first satisfactory answer */

        cp = (unsigned char *)&ans + sizeof(HEADER);
        eom = (unsigned char *)&ans + n;
    }

    for (int qdcount = ntohs(ans.hdr.qdcount); qdcount--;)
    {
        int n = dn_skipname(cp, eom);
        if (n < 0)
            return;
        cp += n + QFIXEDSZ;
    }

    domain = mxlist + MAX_MXLIST - MAXDNAME - 2;

    for (int ancount = ntohs(ans.hdr.ancount); --ancount >= 0 && cp < eom;)
    {
        int type;
        int n = dn_expand((unsigned char *)&ans, eom, cp, mxlist, MAXDNAME);
        if (n < 0)
            break;

        cp += n;

        type = getshort(cp);
        n = getshort(cp + 8);
        cp += 10;

        if (type == T_MX)
        {
            /* pref = getshort(cp); */
            *mxlist = '\0';
            if ((dn_expand
                 ((unsigned char *)&ans, eom, cp + 2, mxlist, MAXDNAME)) < 0)
                break;

            if (!*mxlist)
                return;

            /* Thor.980820: 將數個 MX entry 用 : 串起來以便一個一個試 */
            /* IID.20191229: `:` is valid in IPv6 address; change the delimiter to ';' */

            while (*mxlist)
                mxlist++;
            *mxlist++ = DNS_MXLIST_DELIMITER;

            if (mxlist >= domain)
                break;
        }

        cp += n;
    }

    *mxlist = '\0';
}


int dns_smtp(char *host)
{
    char mxlist[MAX_MXLIST], *str = mxlist;

    dns_mx(host, str);
    if (!*str)
        str = host;

    for (;;)
    {                            /* Thor.980820: 註解: 萬一host格式為 xxx:yyy:zzz, 則先試 xxx, 不行再試 yyy */
        /* IID.20191229: `:` is valid in IPv6 address; change the delimiter to ';' */
        char *ptr = str, ch;
        int sock;
        while ((ch = *ptr))
        {
            if (ch == DNS_MXLIST_DELIMITER)
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
