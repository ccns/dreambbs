int
str_hash2(str, seed)
  char *str;
  int seed;
{
  int cc;

  while ((cc = *str++))
  {
    seed = (seed << 7) - seed + cc;	/* 127 * seed + cc */
  }
  return (seed & 0x7fffffff);
}
