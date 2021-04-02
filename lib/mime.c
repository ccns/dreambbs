/*-------------------------------------------------------*/
/* lib/mime.c   ( NCKU CCNS WindTop-DreamBBS 3.20 )      */
/*-------------------------------------------------------*/
/* Author: Wei-Cheng Yeh (IID) <iid@ccns.ncku.edu.tw>    */
/* Target: MIME code processing library for DreamBBS     */
/* Create: 2020/11/21                                    */
/*-------------------------------------------------------*/

#include "config.h"

#include <stdio.h>
#include <string.h>
#include "dao.h"

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


GCC_CONSTEXPR int qp_code(int x)
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


GCC_CONSTEXPR int base64_code(int x)
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


static inline bool isreturn(const char c)
{
    return c == '\r' || c == '\n';
}


static inline bool is_space(const char c)
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

        if (!str_ncasecmp(str, "quoted-printable", 16))
        {
            *code = 'q';
            return str + 16;
        }
        if (!str_ncasecmp(str, "base64", 6))
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

    if (!str_casecmp(charset, "iso-8859-1"))    /* ���v�]�����i�� */
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
static int mmdecode_header(const char *src,  /* Thor.980901: src�Mdst�i�ۦP, ��src�@�w��?��\0���� */
                           char encode,      /* Thor.980901: �`�N, decode�X�����G���|�ۤv�[�W \0 */
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


int mmdecode(                       /* �� Body �� mmdecode */
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


/* Decode string `str` with embedded encoded texts in-place for MIME header (formerly `str_decode`)
 * `str` of arbitrary length is supported */
void mmdecode_str(char *str)
{
    const char *src = str;
    char *dst = str;
    bool adj = false;

    while (*src)
    {
        if (*src != '=')
        {                           /* Thor: not coded */
            const char *tmp = src;
            while (adj && *tmp && is_space(*tmp))
                tmp++;
            if (adj && *tmp == '=')
            {                       /* Thor: jump over space */
                adj = false;
                src = tmp;
            }
            else
                *dst++ = *src++;
        }
        else                        /* Thor: *src == '=' */
        {
            const char *tmp = src + 1;
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
                        /* IID.2020-12-14: Skip over the decoded string, which is never longer than the source string */
                        tmp += 3 + i;   /* Thor: decode's src */
                        while (*tmp && *tmp++ != '?');      /* Thor: no ? end, mmdecode_header -1 */
                        /* Thor.980901: 0 �]�� decode ���� */
                        if (*tmp == '=')
                            tmp++;
                        src = tmp;      /* Thor: decode over */
                        dst += i;
                        adj = true;     /* Thor: adjacent */
                    }
                }
            }

            while (src != tmp)      /* Thor: not coded */
                *dst++ = *src++;
        }
    }
    *dst = 0;
}


/*-------------------------------------------------------*/
/* rfc2047.c            ( NTHU CS MapleBBS Ver 3.10 )    */
/*-------------------------------------------------------*/
/* target : RFC 2047 QP/base64 encode                    */
/* create : 03/04/11                                     */
/* update : 03/05/19                                     */
/* author : PaulLiu.bbs@bbs.cis.nctu.edu.tw              */
/*-------------------------------------------------------*/

/*-------------------------------------------------------*/
/* RFC2047 QP encode                                     */
/*-------------------------------------------------------*/

void
output_rfc2047_qp(FILE * fp,
                  const char *prefix, const char *str, const char *charset, const char *suffix)
{
    int i, ch;
    int blank = 1;                /* 1:���Ѫťղզ� */
    static const char tbl[16] =
        { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D',
'E', 'F' };

    fputs(prefix, fp);

    /* �p�G�r��}�Y�� US_ASCII printable characters�A�i�����X�A�o�ˤ���n�ݡA�]����ۮe */
    for (i = 0; (ch = str[i]); i++)
    {
        if (ch != '=' && ch != '?' && ch != '_' && ch > '\x1f' && ch < '\x7f')
        {
            if (blank)
            {
                if (ch != ' ')
                    blank = 0;
                else if (str[i + 1] == '\0')    /* �Y���O�ťաA�̫�@�ӭn��X */
                    break;
            }
            fprintf(fp, "%c", ch);
        }
        else
            break;
    }

    if (ch != '\0')                /* �p�G���S���S��r���N���� */
    {
        /* �}�l encode */
        fprintf(fp, "=?%s?Q?", charset);    /* ���w�r�� */
        for (; (ch = str[i]); i++)
        {
            /* �p�G�O non-printable �r���N�n��X */
            /* �d��: '\x20' ~ '\x7e' �� printable, �䤤 =, ?, _, �ť�, ���S��Ÿ��]�n��X */

            if (ch == '=' || ch == '?' || ch == '_' || ch <= '\x1f'
                || ch >= '\x7f')
                fprintf(fp, "=%c%c", tbl[(ch >> 4) & '\x0f'],
                        tbl[ch & '\x0f']);
            else if (ch == ' ')    /* �ťդ���S��, �ন '_' �� "=20" */
                fprintf(fp, "=20");
            else
                fprintf(fp, "%c", ch);
        }
        fputs("?=", fp);
    }

    fputs(suffix, fp);
}
