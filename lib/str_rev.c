/* reverse the string */


char *
str_rev(dst, src)
  char *dst, *src;
{
  int cc;

  *dst = '\0';

  while ((cc = *src))
  {
    *--dst = cc;
    src++;
  }
  return dst;
}
