/*-------------------------------------------------------*/
/* rfc2047.c		( NTHU CS MapleBBS Ver 3.10 )	 */
/*-------------------------------------------------------*/
/* target : RFC 2047 QP/base64 encode			 */
/* create : 03/04/11					 */
/* update : 03/05/19					 */
/* author : PaulLiu.bbs@bbs.cis.nctu.edu.tw		 */
/*-------------------------------------------------------*/


#include <stdio.h>


#if 0	/* itoc.030411: �S���g RFC 2047 encode */
void
output_str(fp, prefix, str, charset, suffix)
  FILE *fp;
  char *prefix;
  char *str;
  char *charset;
  char *suffix;
{
  fprintf(fp, "%s%s%s", prefix, str, suffix);
}
#endif


/*-------------------------------------------------------*/
/* RFC2047 QP encode					 */
/*-------------------------------------------------------*/


void
output_rfc2047_qp(fp, prefix, str, charset, suffix)
  FILE *fp;
  char *prefix;
  char *str;
  char *charset;
  char *suffix;
{
  int i, ch;
  int blank = 1;	/* 1:���Ѫťղզ� */
  static char tbl[16] = {'0','1','2','3','4','5','6','7','8','9', 'A','B','C','D','E','F'};

  fputs(prefix, fp);

  /* �p�G�r��}�Y�� US_ASCII printable characters�A�i�����X�A�o�ˤ���n�ݡA�]����ۮe */
  for (i = 0; ch = str[i]; i++)
  {
    if (ch != '=' && ch != '?' && ch != '_' && ch > '\x1f' && ch < '\x7f')
    {
      if (blank)
      {
	if (ch != ' ')
	  blank = 0;
	else if (str[i + 1] == '\0')	/* �Y���O�ťաA�̫�@�ӭn��X */
	  break;
      }
      fprintf(fp, "%c", ch);
    }
    else
      break;
  }

  if (ch != '\0')	/* �p�G���S���S��r���N���� */
  {
    /* �}�l encode */
    fprintf(fp, "=?%s?Q?", charset);	/* ���w�r�� */
    for (; ch = str[i]; i++)
    {
      /* �p�G�O non-printable �r���N�n��X */
      /* �d��: '\x20' ~ '\x7e' �� printable, �䤤 =, ?, _, �ť�, ���S��Ÿ��]�n��X */

      if (ch == '=' || ch == '?' || ch == '_' || ch <= '\x1f' || ch >= '\x7f')
	fprintf(fp, "=%c%c", tbl[(ch >> 4) & '\x0f'], tbl[ch & '\x0f']);
      else if (ch == ' ')	/* �ťդ���S��, �ন '_' �� "=20" */
	fprintf(fp, "=20");
      else
	fprintf(fp, "%c", ch);
    }
    fputs("?=", fp);
  }

  fputs(suffix, fp);
}


#if 0

/* output_rfc2047_qp() �i�H�������� output_rfc2047_base64()�A�p�G�Q�� encode ���� */

/*-------------------------------------------------------*/
/* RFC2047 base64 encode				 */
/*-------------------------------------------------------*/


static int 
output_rfc2047_prefix(fp, str)
  FILE *fp;
  const unsigned char *str;
{
  int i, lastspace;

  /* output prefix US_ASCII printable characters */
  lastspace = -1;

  /* step 1: find the last space */
  for (i = 0; str[i] != '\0' && str[i] != '=' && str[i] != '?'
    && str[i] != '_' && str[i] > '\x1f' && str[i] < '\x7f'; i++)
  {
    if (str[i] == ' ')
      lastspace = i;
  }
  if (str[i] == '\0')		/* if non special char then outout directly */
  {
    fprintf(fp, "%s", str);
    return i;
  }

  /* step 2: output the prefix with last space */
  for (i = 0; i <= lastspace; i++)
    fprintf(fp, "%c", str[i]);
  return i;
}


static void 
output_rfc2047_base64_3to4(a, b, c, oa, ob, oc, od)
  unsigned char a, b, c;
  char *oa, *ob, *oc, *od;
{
  static char tbl[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  int i;

  *oa = '=';
  *ob = '=';
  *oc = '=';
  *od = '=';

  i = (int)((a >> 2) & '\x3f');
  *oa = tbl[i];
  i = (int)((a << 4) & '\x30') | (int)((b >> 4) & '\x0f');
  *ob = tbl[i];
  if (b == '\0')
    return;
  i = (int)((b << 2) & '\x3c') | (int)((c >> 6) & '\x03');
  *oc = tbl[i];
  if (c == '\0')
    return;
  i = (int)(c & '\x3f');
  *od = tbl[i];
}


void 
output_rfc2047_base64(fp, prefix, str, charset, suffix)
  FILE *fp;
  char *prefix;
  const unsigned char *str;
  const unsigned char *charset;
  char *suffix;
{
  int i, j;
  char a[3], oa[5];

  fputs(prefix, fp);

  /* output prefix US_ASCII printable characters */
  i = output_rfc2047_prefix(fp, str);
  if (str[i] == '\0')
    return;

  /* start encoding */
  fprintf(fp, "=?%s?B?", charset);
  for (; str[i] != '\0';)
  {
    memset(a, 0, sizeof(a));
    for (j = 0; j < 3; j++)
    {
      a[j] = str[i];
      if (str[i] != '\0')
	i++;
    }
    output_rfc2047_base64_3to4(a[0], a[1], a[2],
      &(oa[0]), &(oa[1]), &(oa[2]), &(oa[3]));
    oa[4] = '\0';
    fprintf(fp, "%s", oa);
  }
  fprintf(fp, "?=");

  fputs(suffix, fp);  
}
#endif
