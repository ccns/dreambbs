#include <sys/types.h>
#include <string.h>

size_t  strlcpy(char *, const char *, size_t);

size_t
strlcpy(char *dst, const char *src, size_t siz)
{
  char *d = dst;
  const char *s = src;
  size_t n = siz;

  /* Copy as many bytes as will fit */
  if (n != 0 && --n != 0) 
  {
    do 
    {
      if ((*d++ = *s++) == 0)
        break;
    } while (--n != 0);
  }

  /* Not enough room in dst, add NUL and traverse rest of src */
  if (n == 0) 
  {
    if (siz != 0)
      *d = '\0';                /* NUL-terminate dst */
    while (*s++)
      ;
  }

  return(s - src - 1);        /* count does not include NUL */
}
