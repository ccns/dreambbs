#ifndef	NULL
#define	NULL	(char *) 0
#endif


char * 
str_str(str, tag) 
  char *str; 
  char *tag;                  /* non-empty lower case pattern */ 
{ 
  int cc, c1, c2;
  char *p1, *p2;

  cc = *tag++;
 
  while ((c1 = *str))
  {
    if (c1 >= 'A' && c1 <= 'Z')
      c1 |= 0x20;

    if (c1 == cc)
    {
      p1 = str;
      p2 = tag;

      do
      {
        c2 = *p2;
        if (!c2)
          return str;
 
        p2++;
        c1 = *++p1;
        if (c1 >= 'A' && c1 <= 'Z')
          c1 |= 0x20;
      } while (c1 == c2);
    }
 
    str++;
  }

  return NULL;
}


#if 0
int
str_str(str, tag)
  char *str, *tag;		/* tag : lower-case string */
{
  int ch, key, chk;
  char *s, *t;

  key = *tag++;
  if (!key)
    return 0;

  while (ch = *str)
  {
    str++;
    if (ch >= 'A' && ch <= 'Z')
      ch |= 0x20;
    if (ch == key)
    {
      s = str;
      t = tag;
      str = NULL;
      for (;;)
      {
	chk = *t;
	if (!chk)
	  return 1;

	ch = *s;
	if (ch >= 'A' && ch <= 'Z')
	  ch |= 0x20;

	if (ch != chk)
	  break;

	if (str == NULL)
	{
	  if (ch == key)
	    str = s;
	}

	s++;
	t++;
      }
      if (str == NULL)
	str = s;
    }
  }
  return 0;
}
#endif


#if 0
int
str_str(str, tag)
  char *str, *tag;		/* tag : lower-case string */
{
  char buf[STRLEN];

  str_lower(buf, str);
  return (int) strstr(buf, tag);
}
#endif
