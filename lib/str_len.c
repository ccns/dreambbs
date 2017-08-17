int
str_len(str)
  char *str;
{
  int cc, len;

  for (len = 0; (cc = *str); str++)
  {
    if (cc != ' ')
      len++;
  }

  return len;
}
