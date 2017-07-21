#include <string.h>
#include "dao.h"

int
is_fname(str)
  char *str;
{
  int ch;

  ch = *str;
  if (ch == '/')
    return 0;

  do
  {
    if (!is_alnum(ch) && !strchr("-._/+@", ch))
      return 0;
  } while ((ch = *++str));
  return 1;
}
