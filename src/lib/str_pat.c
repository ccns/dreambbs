/* str_pat : wild card string pattern match support ? * \ */


#include "dao.h"


int
str_pat(str, pat)
  const char *str;
  const char *pat;
{
  const char *xstr = NULL, *xpat;
  int cs, cp;

  xpat = NULL;

  while ((cs = *str))
  {
    cp = *pat++;
    if (cp == '*')
    {
      for (;;)
      {
	cp = *pat;

	if (cp == '\0')
	  return 1;

	pat++;

	if (cp != '*')
	{
	  xpat = pat;
	  xstr = str;
	  break;
	}
      }
    }

    str++;

    if (cp == '?')
      continue;

    if (cp == '\\')
      cp = *pat++;

#ifdef	CASE_IN_SENSITIVE
    if (cp >= 'A' && cp <= 'Z')
      cp += 0x20;

    if (cs >= 'A' && cs <= 'Z')
      cs += 0x20;
#endif

    if (cp == cs)
      continue;

    if (xpat == NULL)
      return 0;

    pat = xpat;
    str = ++xstr;
  }

  while ((cp = *pat))
  {
    if (cp != '*')
      return 0;

    pat++;
  }

  return 1;
}


#ifdef	TEST
#define	STR_PAT(x, y)	printf("<%s, %s> : %d\n", x, y, str_pat(x, y))


main()
{
  STR_PAT("a", "a*");
  STR_PAT("abc", "a*");
  STR_PAT("abc", "a*c");
  STR_PAT("abc", "a?c");
  STR_PAT("level", "l*l");
  STR_PAT("level", "l*e*l");
  STR_PAT("lelelelel", "l*l*l*l");
}
#endif	/* TEST */
