char *
str_add(dst, src)
  char *dst, *src;
{
  while ((*dst = *src))
  {
    src++;
    dst++;
  }
  return dst;
}
