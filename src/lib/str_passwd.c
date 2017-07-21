#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "dao.h"

#ifndef	PASSLEN
#define	PASSLEN 14
#endif


/* ----------------------------------------------------- */
/* password encryption					 */
/* ----------------------------------------------------- */


char *crypt();
static char pwbuf[PASSLEN];


char *
genpasswd(pw)
  char *pw;
{
  char saltc[2];
  int i, c;

  if (!*pw)
    return pw;

  i = 9 * getpid();
  saltc[0] = i & 077;
  saltc[1] = (i >> 6) & 077;

  for (i = 0; i < 2; i++)
  {
    c = saltc[i] + '.';
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
int
chkpasswd(passwd, test)
  char *passwd, *test;
{
  char *pw;
  
  /* if(!*passwd) return -1 */ /* Thor.990416: �Ȧ���passwd�O�Ū� */
  str_ncpy(pwbuf, test, PASSLEN);
  pw = crypt(pwbuf, passwd);
  return (strncmp(pw, passwd, PASSLEN));
}
