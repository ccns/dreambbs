#include <string.h>
#include "dao.h"

int
str_has(list, tag)
  char *list;
  char *tag;
{
  int cc, len;

    len = strlen(tag);
    for (;;)
    {
      cc = list[len];
      if ((!cc || cc == '/') && !str_ncmp(list, tag, len))
	return 1;

      for (;;)
      {
	cc = *list;
	if (!cc)
	  return 0;
	list++;
	if (cc == '/')
	  break;
      }
    }
}
