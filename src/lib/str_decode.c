/*-------------------------------------------------------*/
/* lib/str_decode.c	( NTHU CS MapleBBS Ver 3.00 )	 */
/*-------------------------------------------------------*/
/* target : included C for QP/BASE64 decoding		 */
/* create : 95/03/29				 	 */
/* update : 97/03/29				 	 */
/*-------------------------------------------------------*/


#include <string.h>


/* ----------------------------------------------------- */
/* QP code : "0123456789ABCDEF"				 */
/* ----------------------------------------------------- */


static int
qp_code(x)
  register int x;
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
/* BASE64 :							      */
/* "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/" */
/* ------------------------------------------------------------------ */


static int
base64_code(x)
  register int x;
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
/* �� encode / charset					 */
/* ----------------------------------------------------- */


static inline int
isreturn(c)
  unsigned char c;
{
  return c == '\r' || c == '\n';
}


static inline int 
isspace(c)
  unsigned char c;
{
  return c == ' ' || c == '\t' || isreturn(c);
}


/* ��Content-Transfer-Encode ���Ĥ@�Ӧr��, �̷Ӽзǥu�i��O q,b,7,8 �o�|�� */
char *
mm_getencode(str, code)
  unsigned char *str;
  char *code;
{
  if (str)
  {
    /* skip leading space */
    while (isspace(*str))
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
void
mm_getcharset(str, charset, size)
  const char *str;
  char *charset;
  int size;		/* charset size */
{
  char *src, *dst, *end;
  char delim;
  int ch;

  *charset = '\0';

  if (!str)
    return;

  if (!(src = (char *) strstr(str, "charset=")))
    return;

  src += 8;
  dst = charset;
  end = dst + size - 1;	/* �O�d�Ŷ��� '\0' */
  delim = '\0';

  while (ch = *src++)
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

  if (!str_cmp(charset, "iso-8859-1"))	/* ���v�]�񤣥i�� */
    *charset = '\0';
}


/* ----------------------------------------------------- */
/* judge & decode QP / BASE64				 */
/* ----------------------------------------------------- */


/* PaulLiu.030410: 
   RFC 2047 (Header) QP �����A�̭��W�w '_' ��� ' ' (US_ASCII���ť�)
   �� RFC 2045 (Body) QP �����A'_' �٬O '_'�A�S���S��γ~
   �ҥH�b�� mmdecode ���G���g
*/

static int
mmdecode_header(src, encode, dst)	/* �� Header �� mmdecode */
  unsigned char *src;		/* Thor.980901: src�Mdst�i�ۦP, ��src�@�w��?��\0���� */
  unsigned char encode;		/* Thor.980901: �`�N, decode�X�����G���|�ۤv�[�W \0 */
  unsigned char *dst;
{
  unsigned char *t;
  int pattern, bits;
  int ch;

  t = dst;
  encode |= 0x20;		/* Thor: to lower */

  switch (encode)
  {
  case 'q':			/* Thor: quoted-printable */

    while ((ch = *src) && ch != '?')	/* Thor: Header �̭� 0 �M '?' ���O delimiter */
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
      else if (ch == '_')	/* Header �n�� '_' ���� ' ' */
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

  case 'b':			/* Thor: base 64 */

    /* Thor: pattern & bits are cleared outside while() */
    pattern = 0;
    bits = 0;

    while ((ch = *src) && ch != '?')	/* Thor: Header �̭� 0 �M '?' ���O delimiter */
    {
      int x;

      x = base64_code(*src++);
      if (x < 0)		/* Thor: ignore everything not in the base64,=,.. */
	continue;

      pattern = (pattern << 6) | x;
      bits += 6;		/* Thor: 1 code gains 6 bits */
      if (bits >= 8)		/* Thor: enough to form a byte */
      {
	bits -= 8;
	*t++ = (pattern >> bits) & 0xff;
      }
    }
    return t - dst;
  }

  return -1;
}


int
mmdecode(src, encode, dst)	/* �� Header �� mmdecode */
  unsigned char *src;		/* Thor.980901: src�Mdst�i�ۦP, ��src�@�w��?��\0���� */
  unsigned char encode;		/* Thor.980901: �`�N, decode�X�����G���|�ۤv�[�W \0 */
  unsigned char *dst;
{
  unsigned char *t;
  int pattern, bits;
  int ch;

  t = dst;
  encode |= 0x20;		/* Thor: to lower */

  switch (encode)
  {
  case 'q':			/* Thor: quoted-printable */

    while (ch = *src)		/* Thor: 0 �O delimiter */
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

  case 'b':			/* Thor: base 64 */

    /* Thor: pattern & bits are cleared outside while() */
    pattern = 0;
    bits = 0;

    while (ch = *src)		/* Thor: 0 �O delimiter */
    {
      int x;

      x = base64_code(*src++);
      if (x < 0)		/* Thor: ignore everything not in the base64,=,.. */
	continue;

      pattern = (pattern << 6) | x;
      bits += 6;		/* Thor: 1 code gains 6 bits */
      if (bits >= 8)		/* Thor: enough to form a byte */
      {
	bits -= 8;
	*t++ = (pattern >> bits) & 0xff;
      }
    }
    return t - dst;
  }

  return -1;
}


void
str_decode(str)
  unsigned char *str;
{
  int adj;
  unsigned char *src, *dst;
  unsigned char buf[512];

  src = str;
  dst = buf;
  adj = 0;

  while (*src && (dst - buf) < sizeof(buf) - 1)
  {
    if (*src != '=')
    {				/* Thor: not coded */
      unsigned char *tmp = src;
      while (adj && *tmp && isspace(*tmp))
	tmp++;
      if (adj && *tmp == '=')
      {				/* Thor: jump over space */
	adj = 0;
	src = tmp;
      }
      else
	*dst++ = *src++;
    }
    else			/* Thor: *src == '=' */
    {
      unsigned char *tmp = src + 1;
      if (*tmp == '?')		/* Thor: =? coded */
      {
	/* "=?%s?Q?" for QP, "=?%s?B?" for BASE64 */
	tmp++;
	while (*tmp && *tmp != '?')
	  tmp++;
	if (*tmp && tmp[1] && tmp[2] == '?')	/* Thor: *tmp == '?' */
	{
	  int i = mmdecode_header(tmp + 3, tmp[1], dst);
	  if (i >= 0)
	  {
	    tmp += 3;		/* Thor: decode's src */
	    while (*tmp && *tmp++ != '?');	/* Thor: no ? end, mmdecode_header -1 */
	    /* Thor.980901: 0 �]�� decode ���� */
	    if (*tmp == '=')
	      tmp++;
	    src = tmp;		/* Thor: decode over */
	    dst += i;
	    adj = 1;		/* Thor: adjcent */
	  }
	}
      }

      while (src != tmp)	/* Thor: not coded */
	*dst++ = *src++;
    }
  }
  *dst = 0;
  strcpy(str, buf);
}


#if 0
int
main()
{
  char buf[1024] = "=?Big5?B?pl7C0CA6IFtNYXBsZUJCU11UbyB5dW5sdW5nKDE4SzRGTE0pIFtWQUxJ?=\n\t=?Big5?B?RF0=?=";

  str_decode(buf);
  puts(buf);

  buf[mmdecode("=A7=DA=A4@=AA=BD=B8I=A4=A3=A8=EC=A7=DA=BE=C7=AA=F8", 'q', buf)] = '\0';
  puts(buf);
}
#endif
