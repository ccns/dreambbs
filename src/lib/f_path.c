/* ----------------------------------------------------- */
/* file structure : set file path for boards/user home	 */
/* ----------------------------------------------------- */

#include <string.h>
#include <dao.h>

static void
mak_fpath(str, key, name)
  char *str;
  char *key;
  char *name;
{
  int cc;

  cc = '/';
  for (;;)
  {
    *str = cc;
    if (!cc)
      break;
    str++;
    cc = *key++;

#if 0
    if (cc >= 'A' && cc <= 'Z')
      cc |= 0x20;
#endif
  }

  if (name)
  {
    *str++ = '/';
    strcpy(str, name);
  }
}


void
brd_fpath(fpath, board, fname)
  char *fpath;
  char *board;
  char *fname;
{
  *fpath++ = 'b';
  *fpath++ = 'r';
  *fpath++ = 'd';
  mak_fpath(fpath, board, fname);
}


void
gem_fpath(fpath, board, fname)
  char *fpath;
  char *board;
  char *fname;
{
  *fpath++ = 'g';
  *fpath++ = 'e';
  *fpath++ = 'm';
  mak_fpath(fpath, board, fname);
}


void
usr_fpath(fpath, user, fname)
  char *fpath;
  char *user;
  char *fname;
{
#if 0
  char buf[16];
#endif

#define IDLEN    12 /* Length of board / user id, copy from  struct.h */

  char buf[IDLEN + 1];

  *fpath++ = 'u';
  *fpath++ = 's';
  *fpath++ = 'r';
  *fpath++ = '/';

#if 0
  str_lower(buf, user);		/* lower case */
#endif
  /* Thor.981027: ���� buffer overflow, ���M SunOS 4.1.x�W�L�����p, 
                  �H��A�Q�n����k */
  str_ncpy(buf, user, sizeof(buf));
  str_lower(buf, buf);
    
  *fpath++ = *buf;
  mak_fpath(fpath, buf, fname);
}
