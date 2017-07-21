/* ----------------------------------------------------- */
/* make directory hierarchy [0-9A-V] : 32-way interleave */
/* ----------------------------------------------------- */


#include <sys/stat.h>


void
mak_dirs(fpath)
  char *fpath;
{
  char *fname;
  int ch;

  if (mkdir(fpath, 0755))
    return;

  fname = fpath;
  while (*++fname);
  *fname++ = '/';
  fname[1] = '\0';

  ch = '0';
  for (;;)
  {
    *fname = ch++;
    mkdir(fpath, 0755);
    if (ch == 'W')
      break;
    if (ch == '9' + 1)
      ch = '@';			/* @ : for special purpose */
  }

  fname[-1] = '\0';
}
