char *
str_ttl(title)
  char *title;
{
  if (title[0] == 'R' && title[1] == 'e' && title[2] == ':')
  {
    title += 3;
    if (*title == ' ')
      title++;
  }

  return title;
}
