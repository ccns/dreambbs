#include "dao.h"
#include <string.h>

void
hdr_fpath(fpath, folder, hdr)
  char *fpath;
  char *folder;
  HDR *hdr;
{
  char *str = NULL;
  int cc, chrono;

  while ((cc = *folder++))
  {
    *fpath++ = cc;
    if (cc == '/')
      str = fpath;
  }

  chrono = hdr->chrono;
  folder = hdr->xname;
  cc = *folder;
  if (cc != '@')
    cc = radix32[chrono & 31];

  if (*str == '.')
  {
    *str++ = cc;
    *str++ = '/';
  }
  else
  {
    str[-2] = cc;
  }

  if (hdr->xmode & GEM_EXTEND)
  {
    *str++ = 'X';
    archiv32(chrono, str);
  }
  else
    strcpy(str, hdr->xname);
}
