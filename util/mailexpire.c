/*-------------------------------------------------------*/
/* util/mailexpire.c	( YZU WindTop Ver 3.02 )	 */
/*-------------------------------------------------------*/
/* target : 自動砍信工具程式				 */
/* create : 2000/07/14				 	 */
/* update : 					 	 */
/*-------------------------------------------------------*/
/* syntax : mailexpire [userid] 		 	 */
/* NOTICE : clean user's mbox				 */
/*-------------------------------------------------------*/

#include <strings.h>

#include "bbs.h"

FILE *flog;


static void
expire(
  char *userid)
{
  HDR hdr;
  char fpath[128], fnew[128], index[128], *fname;
  int done, keep, xmode;
  FILE *fpr, *fpw;


  usr_fpath(index,userid,FN_DIR);

  if (!(fpr = fopen(index, "r")))
  {
    fprintf(flog, "\tError open file: %s\n", index);
    return;
  }

  fpw = f_new(index, fnew);
  if (!fpw)
  {
    fprintf(flog, "\tExclusive lock: %s\n", fnew);
    fclose(fpr);
    return;
  }

  strcpy(fpath, index);
  fname = (char *) strrchr(fpath, '.');
  *fname++ = '@';
  *fname++ = '/';

  done = 1;

  while (fread(&hdr, sizeof(hdr), 1, fpr) == 1)
  {
    xmode = hdr.xmode;
    if (xmode & POST_DELETE )
      keep = 0;
    else
      keep = 1;

    if (keep)
    {
      if (fwrite(&hdr, sizeof(hdr), 1, fpw) != 1)
      {
	fprintf(flog, "\tError in write DIR.n: %s\n", hdr.xname);
	done = 0;
	break;
      }
    }
    else
    {
      strcpy(fname, hdr.xname);
      unlink(fpath);
      fprintf(flog, "\t%s\n", fpath);
    }
  }
  fclose(fpr);
  fclose(fpw);

  if (done)
  {
    sprintf(fpath, "%s.o", index);
    if (!rename(index, fpath))
    {
      if (rename(fnew, index))
        rename(fpath, index);		/* 換回來 */
    }
  }
  unlink(fnew);

}

static void
traverse(
  char *fpath)
{
  DIR *dirp;
  struct dirent *de;
  char *fname, *str;

  if (!(dirp = opendir(fpath)))
  {
    fprintf(flog, "## unable to enter hierarchy [%s]\n", fpath);
    return;
  }

  for (str = fpath; *str; str++);
  *str++ = '/';

  while ((de = readdir(dirp)))
  {
    fname = de->d_name;
    if (fname[0] > ' ' && fname[0] != '.')
    {
      expire(fname);
    }
  }
  closedir(dirp);
}


int
main(
  int argc,
  char *argv[])
{
  char fpath[128],*fname,ch;

  setgid(BBSGID);
  setuid(BBSUID);
  chdir(BBSHOME);

  /* --------------------------------------------------- */
  /* visit all users					 */
  /* --------------------------------------------------- */

  flog = fopen(FN_MAILEXPIRE_LOG, "w");

  strcpy(fname = fpath, "usr/@");
  fname = (char *) strchr(fname, '@');
  
  if(argc > 1)
  {
    expire(argv[1]);
  }
  else
  {
 
    for (ch = 'a'; ch <= 'z'; ch++)
    {
      fname[0] = ch;
      fname[1] = '\0';
      traverse(fpath);
    }
    for (ch = '0'; ch <= '9'; ch++)
    {
      fname[0] = ch;
      fname[1] = '\0';
      traverse(fpath);
    }
  }


  fclose(flog);
  exit(0);
}
