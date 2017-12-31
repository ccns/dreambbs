#include <string.h>

void
setdirpath(fpath, direct, fname)
  char *fpath, *direct, *fname;
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
