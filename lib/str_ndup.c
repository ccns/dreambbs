#include <stdlib.h>

char *
str_ndup(src, len)
  char *src;
  int len;
{
  char *dst, *str, *end;

  str = src;
  end = src + len;
  do
  {
    if (!*str++)
    {
      end = str;
      break;
    }
  } while (str < end);

  dst = (char *) malloc(end - src);

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
