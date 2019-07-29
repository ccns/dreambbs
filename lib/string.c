#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include "dao.h"

#ifdef __linux__
  #include <sys/random.h>
#endif

char *str_add(char *dst, char *src)
{
    while ((*dst = *src))
    {
        src++;
        dst++;
    }
    return dst;
}

void str_ansi(                    /* strip ANSI code */
                 char *dst, const char *str, int max)
{
    int ch, ansi;
    char *tail;

    for (ansi = 0, tail = dst + max - 1; (ch = *str); str++)
    {
        if (ch == '\n')
        {
            break;
        }
        else if (ch == '\x1b')
        {
            ansi = 1;
        }
        else if (ansi)
        {
            if ((ch < '0' || ch > '9') && ch != ';' && ch != '[')
                ansi = 0;
        }
        else
        {
            *dst++ = ch;
            if (dst >= tail)
                break;
        }
    }
    *dst = '\0';
}

void str_cat(char *dst, const char *s1, const char *s2)
{
    while ((*dst = *s1))
    {
        s1++;
        dst++;
    }

    while ((*dst++ = *s2++))
        ;
}

int str_cmp(const char *s1, const char *s2)
{
    int c1, c2, diff;

    do
    {
        c1 = *s1++;
        c2 = *s2++;
        if (c1 >= 'A' && c1 <= 'Z')
            c1 |= 32;
        if (c2 >= 'A' && c2 <= 'Z')
            c2 |= 32;
        if ((diff = c1 - c2))
            return (diff);
    }
    while (c1);
    return 0;
}

void str_cut(char *dst, char *src)
{
    int cc;

    for (;;)
    {
        cc = *src++;
        if (!cc)
        {
            *dst = '\0';
            return;
        }

        if (cc == ' ')
        {
            while (*src == ' ')
                src++;

            while ((cc = *src++))
            {
                if (cc == ' ' || cc == '\n' || cc == '\r')
                    break;
                *dst++ = cc;
            }

            *dst = '\0';
            return;
        }
    }
}

/*-------------------------------------------------------*/
/* lib/str_decode.c     ( NTHU CS MapleBBS Ver 3.00 )    */
/*-------------------------------------------------------*/
/* target : included C for QP/BASE64 decoding            */
/* create : 95/03/29                                     */
/* update : 97/03/29                                     */
/*-------------------------------------------------------*/

/* ----------------------------------------------------- */
/* QP code : "0123456789ABCDEF"                          */
/* ----------------------------------------------------- */


int qp_code(register int x)
{
    if (x >= '0' && x <= '9')
        return x - '0';
    if (x >= 'a' && x <= 'f')
        return x - 'a' + 10;
    if (x >= 'A' && x <= 'F')
        return x - 'A' + 10;
    return -1;
}


/* ------------------------------------------------------------------ */
/* BASE64 :                                                           */
/* "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/" */
/* ------------------------------------------------------------------ */


int base64_code(register int x)
{
    if (x >= 'A' && x <= 'Z')
        return x - 'A';
    if (x >= 'a' && x <= 'z')
        return x - 'a' + 26;
    if (x >= '0' && x <= '9')
        return x - '0' + 52;
    if (x == '+')
        return 62;
    if (x == '/')
        return 63;
    return -1;
}


/* ----------------------------------------------------- */
/* �� encode / charset                                   */
/* ----------------------------------------------------- */


static inline int isreturn(const char c)
{
    return c == '\r' || c == '\n';
}


static inline int is_space(const char c)
{
    return c == ' ' || c == '\t' || isreturn(c);
}


/* ��Content-Transfer-Encode ���Ĥ@�Ӧr��, �̷Ӽзǥu�i��O q, b, 7, 8 �o�|�� */
char *mm_getencode(char *str, char *code)
{
    if (str)
    {
        /* skip leading space */
        while (is_space(*str))
            str++;

        if (!str_ncmp(str, "quoted-printable", 16))
        {
            *code = 'q';
            return str + 16;
        }
        if (!str_ncmp(str, "base64", 6))
        {
            *code = 'b';
            return str + 6;
        }
    }

    *code = 0;
    return str;
}


/* �� charset */
void mm_getcharset(const char *str, char *charset, int size    /* charset size */
    )
{
    char *src, *dst, *end;
    char delim;
    int ch;

    *charset = '\0';

    if (!str)
        return;

    if (!(src = (char *)strstr(str, "charset=")))
        return;

    src += 8;
    dst = charset;
    end = dst + size - 1;        /* �O�d�Ŷ��� '\0' */
    delim = '\0';

    while ((ch = *src++))
    {
        if (ch == delim)
            break;

        if (ch == '\"')
        {
            delim = '\"';
            continue;
        }

        if (!is_alnum(ch) && ch != '-')
            break;

        *dst = ch;

        if (++dst >= end)
            break;
    }

    *dst = '\0';

    if (!str_cmp(charset, "iso-8859-1"))    /* ���v�]�����i�� */
        *charset = '\0';
}


/* ----------------------------------------------------- */
/* judge & decode QP / BASE64                            */
/* ----------------------------------------------------- */


/* PaulLiu.030410:
   RFC 2047 (Header) QP �����A�̭��W�w '_' ��� ' ' (US_ASCII���ť�)
   �� RFC 2045 (Body) QP �����A'_' �٬O '_'�A�S���S��γ~
   �ҥH�b�� mmdecode ���G��g
 */

/* �� Header �� mmdecode */
static int mmdecode_header(char *src,    /* Thor.980901: src�Mdst�i�ۦP, ��src�@�w��?��\0���� */
                           char encode,    /* Thor.980901: �`�N, decode�X�����G���|�ۤv�[�W \0 */
                           char *dst)
{
    char *t;
    int pattern, bits;
    int ch;

    t = dst;
    encode |= 0x20;                 /* Thor: to lower */

    switch (encode)
    {
    case 'q':                       /* Thor: quoted-printable */

        while ((ch = *src) && ch != '?')    /* Thor: Header �̭� 0 �M '?' ���O delimiter */
        {
            if (ch == '=')
            {
                int x, y;

                x = *++src;
                y = x ? *++src : 0;
                if (isreturn(x))
                    continue;

                if ((x = qp_code(x)) < 0 || (y = qp_code(y)) < 0)
                    return -1;

                *t++ = (x << 4) + y;
            }
            else if (ch == '_')    /* Header �n�� '_' ���� ' ' */
            {
                *t++ = ' ';
            }
            else
            {
                *t++ = ch;
            }
            src++;
        }
        return t - dst;

    case 'b':                    /* Thor: base 64 */

        /* Thor: pattern & bits are cleared outside while () */
        pattern = 0;
        bits = 0;

        while ((ch = *src) && ch != '?')    /* Thor: Header �̭� 0 �M '?' ���O delimiter */
        {
            int x;

            x = base64_code(*src++);
            if (x < 0)            /* Thor: ignore everything not in the base64,=,.. */
                continue;

            pattern = (pattern << 6) | x;
            bits += 6;            /* Thor: 1 code gains 6 bits */
            if (bits >= 8)        /* Thor: enough to form a byte */
            {
                bits -= 8;
                *t++ = (pattern >> bits) & 0xff;
            }
        }
        return t - dst;
    }

    return -1;
}


int mmdecode(                       /* �� Header �� mmdecode */
                const char *src,    /* Thor.980901: src�Mdst�i�ۦP, ��src�@�w��?��\0���� */
                char encode,        /* Thor.980901: �`�N, decode�X�����G���|�ۤv�[�W \0 */
                char *dst)
{
    char *t;
    int pattern, bits;
    int ch;

    t = dst;
    encode |= 0x20;                 /* Thor: to lower */

    switch (encode)
    {
    case 'q':                       /* Thor: quoted-printable */

        while ((ch = *src))         /* Thor: 0 �O delimiter */
        {
            if (ch == '=')
            {
                int x, y;

                x = *++src;
                y = x ? *++src : 0;
                if (isreturn(x))
                    continue;

                if ((x = qp_code(x)) < 0 || (y = qp_code(y)) < 0)
                    return -1;

                *t++ = (x << 4) + y;
            }
            else
            {
                *t++ = ch;
            }
            src++;
        }
        return t - dst;

    case 'b':                       /* Thor: base 64 */

        /* Thor: pattern & bits are cleared outside while () */
        pattern = 0;
        bits = 0;

        while ((ch = *src))         /* Thor: 0 �O delimiter */
        {
            int x;

            x = base64_code(*src++);
            if (x < 0)              /* Thor: ignore everything not in the base64,=,.. */
                continue;

            pattern = (pattern << 6) | x;
            bits += 6;              /* Thor: 1 code gains 6 bits */
            if (bits >= 8)          /* Thor: enough to form a byte */
            {
                bits -= 8;
                *t++ = (pattern >> bits) & 0xff;
            }
        }
        return t - dst;
    }

    return -1;
}


void str_decode(char *str)
{
    int adj;
    char *src, *dst;
    char buf[512];

    src = str;
    dst = buf;
    adj = 0;

    while (*src && (dst - buf) < sizeof(buf) - 1)
    {
        if (*src != '=')
        {                           /* Thor: not coded */
            char *tmp = src;
            while (adj && *tmp && is_space(*tmp))
                tmp++;
            if (adj && *tmp == '=')
            {                       /* Thor: jump over space */
                adj = 0;
                src = tmp;
            }
            else
                *dst++ = *src++;
        }
        else                        /* Thor: *src == '=' */
        {
            char *tmp = src + 1;
            if (*tmp == '?')        /* Thor: =? coded */
            {
                /* "=?%s?Q?" for QP, "=?%s?B?" for BASE64 */
                tmp++;
                while (*tmp && *tmp != '?')
                    tmp++;
                if (*tmp && tmp[1] && tmp[2] == '?')        /* Thor: *tmp == '?' */
                {
                    int i = mmdecode_header(tmp + 3, tmp[1], dst);
                    if (i >= 0)
                    {

                        tmp += 3;       /* Thor: decode's src */
                        while (*tmp && *tmp++ != '?');      /* Thor: no ? end, mmdecode_header -1 */
                        /* Thor.980901: 0 �]�� decode ���� */
                        if (*tmp == '=')
                            tmp++;
                        src = tmp;      /* Thor: decode over */
                        dst += i;
                        adj = 1;        /* Thor: adjacent */
                    }
                }
            }

            while (src != tmp)      /* Thor: not coded */
                *dst++ = *src++;
        }
    }
    *dst = 0;
    strcpy(str, buf);
}

char *str_dup(char *src, int pad)
{
    char *dst;

    dst = (char *)malloc(strlen(src) + pad);
    strcpy(dst, src);
    return dst;
}

void str_folder(char *fpath, char *folder, const char *fname)
{
    int ch;
    char *token = NULL;

    while ((ch = *folder++))
    {
        *fpath++ = ch;
        if (ch == '/')
            token = fpath;
    }
    if (*token != '.')
        token -= 2;
    strcpy(token, fname);
}

void setdirpath(char *fpath, char *direct, const char *fname)
{
    int ch;
    char *target = NULL;

    while ((ch = *direct))
    {
        *fpath++ = ch;
        if (ch == '/')
            target = fpath;
        direct++;
    }

    strcpy(target, fname);
}

/* ----------------------------------------------------  */
/* E-mail address format                                 */
/* ----------------------------------------------------  */
/* 1. user@domain                                        */
/* 2. <user@domain>                                      */
/* 3. user@domain (nick)                                 */
/* 4. user@domain ("nick")                               */
/* 5. nick <user@domain>                                 */
/* 6. "nick" <user@domain>                               */
/* ----------------------------------------------------  */

int str_from(char *from, char *addr, char *nick)
{
    char *str, *ptr, *langle;
    int cc;

    *nick = 0;

    langle = ptr = NULL;

    for (str = from; (cc = *str); str++)
    {
        if (cc == '<')
            langle = str;
        else if (cc == '@')
            ptr = str;
    }

    if (ptr == NULL)
    {
        strcpy(addr, from);
        return -1;
    }

    if (langle && langle < ptr && str[-1] == '>')
    {
        /* case 2/5/6 : name <mail_addr> */

        str[-1] = 0;
        if (langle > from)
        {
            ptr = langle - 2;
            if (*from == '"')
            {
                from++;
                if (*ptr == '"')
                    ptr--;
            }
            if (*from == '(')
            {
                from++;
                if (*ptr == ')')
                    ptr--;
            }
            ptr[1] = '\0';
            strcpy(nick, from);
            str_decode(nick);
        }

        from = langle + 1;
    }
    else
    {
        /* case 1/3/4 */

        if (*--str == ')')
        {
            if (str[-1] == '"')
                str--;
            *str = 0;

            if ((ptr = (char *)strchr(from, '(')))
            {
                ptr[-1] = 0;
                if (*++ptr == '"')
                    ptr++;

                strcpy(nick, ptr);
                str_decode(nick);
            }
        }
    }

    strcpy(addr, from);
    return 0;
}

int str_has(char *list, char *tag)
{
    int cc, len;

    len = strlen(tag);
    for (;;)
    {
        cc = list[len];
        if ((!cc || cc == '/') && !str_ncmp(list, tag, len))
            return 1;

        for (;;)
        {
            cc = *list;
            if (!cc)
                return 0;
            list++;
            if (cc == '/')
                break;
        }
    }
}

int str_hash2(char *str, int seed)
{
    int cc;

    while ((cc = *str++))
    {
        seed = (seed << 7) - seed + cc;    /* 127 * seed + cc */
    }
    return (seed & 0x7fffffff);
}

int str_hash(const char *str, int seed)
{
    int cc;

    while ((cc = *str++))
    {
        seed = (seed << 5) - seed + cc;    /* 31 * seed + cc */
    }
    return (seed & 0x7fffffff);
}

int str_len(char *str)
{
    int cc, len;

    for (len = 0; (cc = *str); str++)
    {
        if (cc != ' ')
            len++;
    }

    return len;
}

void str_lower(char *dst, const char *src)
{
    int ch;

    do
    {
        ch = *src++;
        if (ch >= 'A' && ch <= 'Z')
            ch |= 0x20;
        *dst++ = ch;
    }
    while (ch);
}

void str_lowest(char *dst, char *src)
{
    int ch;
    int in_chi = 0;                /* 1: �e�@�X�O����r */

    do
    {
        ch = *src++;
        if (in_chi || ch & 0x80)
            in_chi ^= 1;
        else if (ch >= 'A' && ch <= 'Z')
            ch |= 0x20;
        *dst++ = ch;
    }
    while (ch);
}

int str_ncmp(const char *s1, const char *s2, int n)
{
    int c1, c2;

    while (n--)
    {
        c1 = *s1++;
        if (c1 >= 'A' && c1 <= 'Z')
            c1 |= 32;

        c2 = *s2++;
        if (c2 >= 'A' && c2 <= 'Z')
            c2 |= 32;

        if (c1 -= c2)
            return (c1);

        if (!c2)
            break;
    }

    return 0;
}

void str_strip(                    /* remove trailing space */
                  char *str)
{
    int ch;

    do
    {
        ch = *(--str);
    }
    while (ch == ' ' || ch == '\t');
    str[1] = '\0';
}

/*
 * str_ncpy() - similar to strncpy(3) but terminates string always with '\0'
 * if n != 0, and doesn't do padding
 */
void str_ncpy(char *dst, const char *src, int n)
{
    char *end;

    end = dst + n - 1;

    do
    {
        n = (dst >= end) ? 0 : *src++;
        *dst++ = n;
    }
    while (n);
}

char *str_ndup(char *src, int len)
{
    char *dst, *str, *end;

    str = src;
    end = src + len;
    do
    {
        if (!*str++)
        {
            end = str;
            break;
        }
    }
    while (str < end);

    dst = (char *)malloc(end - src);

    for (str = dst; (*str = *src); src++)
    {
        str++;
        if (src >= end)
        {
            *str = '\0';
            break;
        }
    }

    return dst;
}

#ifndef PASSLEN
#define PASSLEN 14
#endif

#ifndef PLAINPASSLEN
#define PLAINPASSLEN 9
#endif


/* ----------------------------------------------------- */
/* password encryption                                   */
/* ----------------------------------------------------- */

/* IID.20190524: Get bytes from the system PRNG device. */
char *getrandom_bytes(char *buf, size_t buflen)
{
#if defined(__linux__)
    if (getrandom(buf, buflen, GRND_NONBLOCK) == -1)
        return NULL;
#elif OpenBSD >= 201311 /* 5.4 */ || __FreeBSD_version >= 1200000 /* 12.0 */
    arc4random_buf(buf, buflen);
#else
    int fd;
    if ((fd = open("/dev/urandom", O_RDONLY)) < 0)
        return NULL;
    read(fd, buf, buflen);
    close(fd);
#endif

    return buf;
}

char *crypt(const char *key, const char *salt);
static char pwbuf[PASSLEN];

char *genpasswd(char *pw)
{
    char saltc[2];
    int i, c;

    if (!*pw)
        return pw;

    /* IID.20190524: Get salt from the system PRNG device. */
    if (!getrandom_bytes(saltc, 2))
        return NULL;

    for (i = 0; i < 2; i++)
    {
        c = (saltc[i] & 0x3f) + '.';
        if (c > '9')
            c += 7;
        if (c > 'Z')
            c += 6;
        saltc[i] = c;
    }
    strcpy(pwbuf, pw);
    return crypt(pwbuf, saltc);
}


/* Thor.990214: ����: �X�K�X��, �Ǧ^0 */
int chkpasswd(char *passwd, char *test)
{
    char *pw;

    /* if (!*passwd) return -1 *//* Thor.990416: �Ȧ���passwd�O�Ū� */
    str_ncpy(pwbuf, test, PLAINPASSLEN);
    pw = crypt(pwbuf, passwd);
    return (strncmp(pw, passwd, PASSLEN));
}

/* str_pat : wild card string pattern match support ? * \ */

int str_pat(const char *str, const char *pat)
{
    const char *xstr = NULL, *xpat;
    int cs, cp;

    xpat = NULL;

    while ((cs = *str))
    {
        cp = *pat++;
        if (cp == '*')
        {
            for (;;)
            {
                cp = *pat;

                if (cp == '\0')
                    return 1;

                pat++;

                if (cp != '*')
                {
                    xpat = pat;
                    xstr = str;
                    break;
                }
            }
        }

        str++;

        if (cp == '?')
            continue;

        if (cp == '\\')
            cp = *pat++;

#ifdef  CASE_IN_SENSITIVE
        if (cp >= 'A' && cp <= 'Z')
            cp += 0x20;

        if (cs >= 'A' && cs <= 'Z')
            cs += 0x20;
#endif

        if (cp == cs)
            continue;

        if (xpat == NULL)
            return 0;

        pat = xpat;
        str = ++xstr;
    }

    while ((cp = *pat))
    {
        if (cp != '*')
            return 0;

        pat++;
    }

    return 1;
}


/* reverse the string */

char *str_rev(char *dst, char *src)
{
    int cc;

    *dst = '\0';

    while ((cc = *src))
    {
        *--dst = cc;
        src++;
    }
    return dst;
}

int str_rle(                    /* run-length encoding */
               char *str)
{
    char *src;
    char *dst;
    int cc, rl;

    dst = src = str;
    while ((cc = *src++))
    {
        if (cc > 8 && cc == src[0] && cc == src[1] && cc == src[2])
        {
            src += 2;
            rl = 4;
            while (*++src == cc)
            {
                if (++rl >= 255)
                    break;
            }

            *dst++ = 8;
            *dst++ = rl;
        }
        *dst++ = cc;
    }

    *dst = '\0';
    return dst - str;
}

#ifndef NULL
#define NULL    (char* ) 0
#endif

char *str_str(const char *str, const char *tag      /* non-empty lower case pattern */
    )
{
    int cc, c1, c2;
    const char *p1, *p2;

    cc = *tag++;

    while ((c1 = *str))
    {
        if (c1 >= 'A' && c1 <= 'Z')
            c1 |= 0x20;

        if (c1 == cc)
        {
            p1 = str;
            p2 = tag;

            do
            {
                c2 = *p2;
                if (!c2)
                    return (char *)str;

                p2++;
                c1 = *++p1;
                if (c1 >= 'A' && c1 <= 'Z')
                    c1 |= 0x20;
            }
            while (c1 == c2);
        }

        str++;
    }

    return NULL;
}

char *str_sub(char *str, char *tag      /* non-empty lowest case pattern */
    )
{
    int cc, c1, c2;
    char *p1, *p2;
    int in_chi = 0;                     /* 1: �e�@�X�O����r */
    int in_chii;                        /* 1: �e�@�X�O����r */

    cc = *tag++;

    while ((c1 = *str))
    {
        if (in_chi)
        {
            in_chi ^= 1;
        }
        else
        {
            if (c1 & 0x80)
                in_chi ^= 1;
            else if (c1 >= 'A' && c1 <= 'Z')
                c1 |= 0x20;

            if (c1 == cc)
            {
                p1 = str;
                p2 = tag;
                in_chii = in_chi;

                do
                {
                    c2 = *p2;
                    if (!c2)
                        return str;

                    p2++;
                    c1 = *++p1;
                    if (in_chii || c1 & 0x80)
                        in_chii ^= 1;
                    else if (c1 >= 'A' && c1 <= 'Z')
                        c1 |= 0x20;
                }
                while (c1 == c2);
            }
        }

        str++;
    }

    return NULL;
}

char *str_tail(char *str)
{
    while (*str)
    {
        str++;
    }
    return str;
}

void str_trim(                    /* remove trailing space */
                 char *buf)
{
    char *p = buf;

    while (*p)
        p++;
    while (--p >= buf)
    {
        if (*p == ' ')
            *p = '\0';
        else
            break;
    }
}

const char *str_ttl(const char *title)
{
    if (title[0] == 'R' && title[1] == 'e' && title[2] == ':')
    {
        title += 3;
        if (*title == ' ')
            title++;
    }

    return title;
}

/*-------------------------------------------------------*/
/* lib/str_xor.c     ( NTHU CS MapleBBS Ver 3.10 )       */
/*-------------------------------------------------------*/
/* author : thor.bbs@bbs.cs.nthu.edu.tw                  */
/* target : included C for str xor-ing (signed mail)     */
/* create : 99/03/30                                     */
/* update :   /  /                                       */
/*-------------------------------------------------------*/

//const char*
void str_xor(char *dst,         /* Thor.990409: ���N���ץ��Nbinary seq, �ܤ֭n src����� */
             const char *src    /* Thor.990409: ���N����str, ���t \0 */
             /* Thor: ���G�O�Nsrc xor��dst�W, �Y��0���G, �h����,
                      �ҥHdst���ץ��j�󵥩� src(�H�r��Ө�) */
    )
{
    register int cc;
    for (; *src; src++, dst++)
    {
        if ((cc = *src ^ *dst))
            *dst = cc;
    }
}

/* strlcat based on OpenBSDs strlcat */

/*
 * Appends src to string dst of size siz (unlike strncat, siz is the
 * full size of dst, not space left).  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz <= strlen(dst)).
 * Returns strlen(src) + MIN(siz, strlen(initial dst)).
 * If retval >= siz, truncation occurred.
 */
size_t strlcat(char *dst, const char *src, size_t siz)
{
    char *d = dst;
    const char *s = src;
    size_t n = siz;
    size_t dlen;

    /* Find the end of dst and adjust bytes left but don't go past end */
    while (n-- != 0 && *d != '\0')
        d++;
    dlen = d - dst;
    n = siz - dlen;

    if (n == 0)
        return (dlen + strlen(s));
    while (*s != '\0')
    {
        if (n != 1)
        {
            *d++ = *s;
            n--;
        }
        s++;
    }
    *d = '\0';

    return (dlen + (s - src));    /* count does not include NUL */
}

/* strlcpy based on OpenBSDs strlcpy */
/*
 * Copy src to string dst of size siz.  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz == 0).
 * Returns strlen(src); if retval >= siz, truncation occurred.
 */

size_t strlcpy(char *dst, const char *src, size_t siz)
{
    char *d = dst;
    const char *s = src;
    size_t n = siz;

    /* Copy as many bytes as will fit */
    if (n != 0 && --n != 0)
    {
        do
        {
            if ((*d++ = *s++) == 0)
                break;
        }
        while (--n != 0);
    }

    /* Not enough room in dst, add NUL and traverse rest of src */
    if (n == 0)
    {
        if (siz != 0)
            *d = '\0';          /* NUL-terminate dst */
        while (*s++)
            ;
    }

    return (s - src - 1);       /* count does not include NUL */
}

int hash32(const char *str)
{
    int xo, cc;

    xo = 1048583;               /* a big prime number */
    while ((cc = *str++))
    {
        xo = (xo << 5) - xo + cc;    /* 31 * xo + cc */
    }
    return (xo & 0x7fffffff);
}
