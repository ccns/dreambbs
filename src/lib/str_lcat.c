#include <sys/types.h>
#include <string.h>

size_t  strlcat(char *, const char *, size_t);

size_t
strlcat(char *dst, const char *src, size_t siz)
{
  char *d = dst;
  const char *s = src;
  size_t n = siz;
  size_t dlen;

  while (n-- != 0 && *d != '\0')
    d++;
  dlen = d - dst;
  n = siz - dlen;

  if (n == 0)
    return(dlen + strlen(s));
  while (*s != '\0') 
  {
    if (n != 1) 
    {
      *d++ = *s;
      n--;
    }
    s++;
  }
  *d = '\0';

  return(dlen + (s - src));
}
