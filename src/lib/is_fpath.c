/* ----------------------------------------------------- */
/* transform to real path & security check		 */
/* ----------------------------------------------------- */


int
is_fpath(path)
  char *path;
{
  int ch, level;
  char *source, *target;

  level = 0;
  source = target = path;


  for (;;)
  {
    ch = *source;

    if (ch == '/')
    {
      int next;

      next = source[1];

      if (next == '/')
      {
	return 0;		/* [//] */
      }
      else if (next == '.')
      {
	next = source[2];

	if (next == '/')
	  return 0;		/* [/./] */

	if (next == '.' && source[3] == '/')
	{
	  /* -------------------------- */
	  /* abc/xyz/../def ==> abc/def */
	  /* -------------------------- */

	  for (;;)
	  {
	    if (target <= path)
	      return 0;

	    target--;
	    if (*target == '/')
	      break;
	  }

	  source += 3;
	  continue;
	}
      }

      level++;
    }

    *target = ch;

    if (ch == 0)
      return level;

    target++;
    source++;
  }
}
