#include "config.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dao.h"
#include "global_def.h"

/* Copy string `src` to buffer `dst` and return the new string end of `dst` (formerly `str_add`) */
char *str_pcpy(char *dst, const char *src)
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

/* Compare two strings `s1` and `s2`, ignoring case differences (formerly `str_cmp`)
 * Returns the value of difference between the first different bytes of the two strings or `0` if the strings do not differ */
GCC_PURE int str_casecmp(const char *s1, const char *s2)
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

/* Split string `src` with spaces and copy the first line of the second splitted item to `dst` (formerly `str_cut`) */
void str_split_2nd(char *dst, const char *src)
{
    for (;;)
    {
        int cc = *src++;
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

char *str_dup(const char *src, int pad)
{
    char *dst;

    dst = (char *)malloc(strlen(src) + pad);
    strcpy(dst, src);
    return dst;
}

/* Similar to the `setdirpath` below, but for some `folder`, use the parent directory as the base directory instead (formerly `str_folder`)
 * When `folder` is not a path to a `.DIR` or `.GEM` file, the parent directory is used as the base directory */
void setdirpath_root(char *fpath, const char *folder, const char *fname)
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

/* Output the path string to `fname` with the containing directory of the file specified with `direct` as the base directory to `fpath` */
void setdirpath(char *fpath, const char *direct, const char *fname)
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

/* Parse string `from` and copy the email address into buffer `addr`, and the nickname into buffer `nick` (formerly `str_from`)
 * Returns `-1` if `from` does not contain any email addresses; `from` is copied into `addr` and an empty string is copied into `nick`
 * Returns `0` otherwise */
int from_parse(const char *from, char *addr, char *nick)
{
    const char *at = NULL;
    const char *langle = NULL;
    const char *from_end;

    *nick = '\0';

    {
        int cc;
        for (from_end = from; (cc = *from_end); from_end++)
        {
            if (cc == '<')
                langle = from_end;
            else if (cc == '@')
                at = from_end;
        }
    }

    if (at == NULL)
    {
        strcpy(addr, from);
        return -1;
    }

    if (langle && langle < at && from_end[-1] == '>')
    {
        /* case 2/5/6 : name <mail_addr> */

        if (langle > from)
        {
            const char *nick_end = langle - 1;
            const char *nick_head = from;
            if (*nick_head == '(' && nick_end[-1] == ')')
            {
                nick_head++;
                nick_end--;
            }
            if (*nick_head == '"' && nick_end[-1] == '"')
            {
                nick_head++;
                nick_end--;
            }
            strlcpy(nick, nick_head, nick_end - nick_head + 1);
            mmdecode_str(nick);
        }

        --from_end;
        from = langle + 1;
    }
    else
    {
        /* case 1/3/4 */

        const char *nick_end = from_end - 1;
        if (*nick_end == ')')
        {
            const char *nick_head = strchr(from, '(');
            if (nick_head)
            {
                from_end = nick_head - 1;

                nick_head++;
                if (*nick_head == '"' && nick_end[-1] == '"')
                {
                    nick_head++;
                    nick_end--;
                }
                strlcpy(nick, nick_head, nick_end - nick_head + 1);
                mmdecode_str(nick);
            }
        }
    }

    strlcpy(addr, from, from_end - from + 1);
    return 0;
}

GCC_PURE bool str_has(const char *list, const char *tag)
{
    int len = strlen(tag);
    for (;;)
    {
        int cc = list[len];
        if ((!cc || cc == '/') && !str_ncasecmp(list, tag, len))
            return true;

        for (;;)
        {
            cc = *list;
            if (!cc)
                return false;
            list++;
            if (cc == '/')
                break;
        }
    }
}

GCC_PURE int str_hash_mult(const char *str, unsigned int seed, unsigned int mult_base)
{
    unsigned int cc;

    while ((cc = *str++))
    {
        seed = mult_base * seed + cc;
    }
    return (seed & 0x7fffffff);
}

GCC_PURE int str_hash(const char *str, unsigned int seed)
{
    return str_hash_mult(str, seed, 31);
}

GCC_PURE int str_hash2(const char *str, unsigned int seed)
{
    return str_hash_mult(str, seed, 127);
}

GCC_PURE int hash32(const char *str)
{
    const unsigned int seed = 1048583; /* a big prime number */
    return str_hash(str, seed);
}

/* Return the length of string `str` without spaces (formerly `str_len) */
GCC_PURE int str_len_nospace(const char *str)
{
    int cc, len;

    for (len = 0; (cc = *str); str++)
    {
        if (cc != ' ')
            len++;
    }

    return len;
}

/* Convert string `str` to lowercase and output the result to buffer `dst` */
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

/* Convert string `str` to lowercase with DBCS characters handled and output the result to buffer `dst` (formerly `str_lowest`) */
void str_lower_dbcs(char *dst, const char *src)
{
    int ch;
    int in_dbcs = 0;               /* 1: 前一碼是中文字 */

    do
    {
        ch = *src++;
        if (in_dbcs || IS_DBCS_HI(ch))
            in_dbcs ^= 1;
        else if (ch >= 'A' && ch <= 'Z')
            ch |= 0x20;
        *dst++ = ch;
    }
    while (ch);
}

/* Compare two strings `s1` and `s2` for at most `n` bytes, ignoring case differences (formerly `str_ncmp`)
 * Returns the value of difference between the first different bytes of the two strings or `0` if the first `n` bytes of the two strings do not differ */
GCC_PURE int str_ncasecmp(const char *s1, const char *s2, int n)
{
    while (n--)
    {
        int c1, c2;

        c1 = *s1++;
        if (c1 >= 'A' && c1 <= 'Z')
            c1 |= 0x20;

        c2 = *s2++;
        if (c2 >= 'A' && c2 <= 'Z')
            c2 |= 0x20;

        if (c1 -= c2)
            return (c1);

        if (!c2)
            break;
    }

    return 0;
}

/* Remove trailing spaces and tabs from the string end `str` in-place (formerly `str_strip`) */
void str_rstrip_tail(                    /* remove trailing space */
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

/* Thor.980921: str_ncpy 含 0 */
/* Thor.980921: str_ncpy 已含 0之空間 */
/* Thor.980921: 已包含 0 */
/* Thor.980921: str_ncpy與一般 strncpy有所不同, 特別注意 */
/*
 * str_scpy() - similar to strncpy(3) but terminates string always with '\0' if n != 0,
 * and doesn't do padding (formerly `str_ncpy`)
 * It behaviors like `strlcpy`, except that it does not read more than `n` bytes from `src`.
 * It behaviors the same as `strscpy` from the Linux Kernel API, except that it does not return any values.
 */
void str_scpy(char *dst, const char *src, int n)
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

char *str_ndup(const char *src, int len)
{
    char *dst, *str;
    const char *str_src, *end;

    str_src = src;
    end = src + len;
    do
    {
        if (!*str_src++)
        {
            end = str_src;
            break;
        }
    }
    while (str_src < end);

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

/* Return the length of string `str` or `maxlen` if `str` contains more than `maxlen` non-'\0' characters */
GCC_PURE size_t str_nlen(const char *str, size_t maxlen)
{
    const char *const end = str + maxlen;
    for (const char *p = str; p < end; ++p)
    {
        if (!*p)
            return p - str;
    }
    return maxlen;
}

/* str_pat : wild card string pattern match support ? * \ */

GCC_PURE bool str_pat(const char *str, const char *pat)
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
                    return true;

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
            return false;

        pat = xpat;
        str = ++xstr;
    }

    while ((cp = *pat))
    {
        if (cp != '*')
            return false;

        pat++;
    }

    return true;
}


/* Reverse the string `src`, output the result to a buffer from the buffer end `dst`, and return the string head of `dst` (formerly `str_rev`) */
char *str_rev_tail(char *dst, const char *src)
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

/* Encode string `str` in-place using a form of run-length encoding and return the encoded string length (formerly `str_rle`)
 * Encoding scheme: 4+ repeated bytes (value > `'\b'`) => `'\b' <repeat length in 8-bit binary>` */
int rle_encode(                    /* run-length encoding */
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
#define NULL    (char*) 0
#endif

/* Find and return the first occurrence of lowercase pattern `tag` in string `str` or `NULL` if not found, ignoring the case of `str` (formerly `str_str`) */
GCC_PURE char *str_casestr(const char *str, const char *tag      /* non-empty lower case pattern */
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

/* Find and return the first occurrence of lowercase pattern `tag` in string `str` or `NULL` if not found, ignoring the case of `str` and handling DBCS characters (formerly `str_sub`) */
GCC_PURE char *str_casestr_dbcs(const char *str, const char *tag
                                        /* non-empty lowest case pattern */
    )
{
    int cc, c1, c2;
    const char *p1, *p2;
    int in_dbcs = 0;                    /* 1: 前一碼是中文字 */
    int in_dbcsi;                       /* 1: 前一碼是中文字 */

    cc = *tag++;

    while ((c1 = *str))
    {
        if (in_dbcs)
        {
            in_dbcs ^= 1;
        }
        else
        {
            if (IS_DBCS_HI(c1))
                in_dbcs ^= 1;
            else if (c1 >= 'A' && c1 <= 'Z')
                c1 |= 0x20;

            if (c1 == cc)
            {
                p1 = str;
                p2 = tag;
                in_dbcsi = in_dbcs;

                do
                {
                    c2 = *p2;
                    if (!c2)
                        return (char *)str;

                    p2++;
                    c1 = *++p1;
                    if (in_dbcsi || IS_DBCS_HI(c1))
                        in_dbcsi ^= 1;
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

GCC_PURE char *str_tail(const char *str)
{
    while (*str)
    {
        str++;
    }
    return (char *)str;
}

/* Remove trailing spaces from string `buf` in-place (formerly `str_trim`) */
void str_rtrim(                    /* remove trailing space */
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

char *str_ttl_hdrmode(const char *title, enum HdrMode *pmode)
{
    static const char *const title_marks[] = {STR_REPLY, STR_FORWARD};
    if (pmode)
        *pmode = HDRMODE_NORMAL;

    for (size_t i = 0; i < COUNTOF(title_marks); ++i)
    {
        if (!str_ncasecmp(title, title_marks[i], strlen(title_marks[i])))
        {
            title += strlen(title_marks[i]);
            if (*title == ' ')
                title++;
            if (pmode)
            {
                switch (i)
                {
                case 0:
                    *pmode = HDRMODE_REPLY;
                    break;
                case 1:
                    *pmode = HDRMODE_FORWARD;
                    break;
                default:
                    ;
                }
            }
            break;
        }
    }

    return (char *)title;
}

GCC_PURE char *str_ttl(const char *title)
{
    return str_ttl_hdrmode(title, NULL);
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
void str_xor(char *dst,         /* Thor.990409: 任意長度任意binary seq, 至少要 src那麼長 */
             const char *src    /* Thor.990409: 任意長度str, 不含 \0 */
             /* Thor: 結果是將src xor到dst上, 若有0結果, 則不變,
                      所以dst長度必大於等於 src(以字串而言) */
    )
{
    for (; *src; src++, dst++)
    {
        int cc;
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
