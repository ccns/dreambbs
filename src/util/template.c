/*-------------------------------------------------------*/
/* util/template.c      ( YZU WindTop 2000)    		 */
/*-------------------------------------------------------*/
/* author : visor.bbs@bbs.yzu.edu.tw			 */
/* target : 標本用			                 */
/* create : 98/03/29                                     */
/* update : 98/05/29                                     */
/*-------------------------------------------------------*/
/* syntax :                                              */
/*-------------------------------------------------------*/

#include "bbs.h"

void
acct_save(acct)
  ACCT *acct;
{
  int fd;
  char fpath[80];

  usr_fpath(fpath, acct->userid, ".ACCT");
  fd = open(fpath, O_WRONLY, 0600);     /* fpath 必須已經存在 */
  if (fd >= 0)
  {
    write(fd, acct, sizeof(ACCT));
    close(fd);
  }
}

static void
reaper(fpath, lowid)
  char *fpath;
  char *lowid;
{
  int fd;

  char buf[256];
  ACCT acct;

  sprintf(buf, "%s/.ACCT", fpath);
  fd = open(buf, O_RDWR, 0);
  if (fd < 0)
     return;

  if(read(fd, &acct, sizeof(acct))!=sizeof(acct))
  {
    close(fd);
    return;
  }
  close(fd);
/*  acct.request = 10;*/
/*  acct.stay_min = 0;*/
  if(acct.request > 22)
    printf("%s : %d\n",acct.userid,acct.request);
/*  acct_save(&acct);*/

}

static void
traverse(fpath)
  char *fpath;
{
  DIR *dirp;
  struct dirent *de;
  char *fname, *str;

  if (!(dirp = opendir(fpath)))
  {
    return;
  }
  for (str = fpath; *str; str++);
  *str++ = '/';

  while ((de = readdir(dirp)))
  {
    fname = de->d_name;
    if (fname[0] > ' ' && fname[0] != '.')
    {
      strcpy(str, fname);
      reaper(fpath, fname);
    }
  }
  closedir(dirp);
}

int
main(void)
{
  int ch;
  char *fname, fpath[256];

  strcpy(fname = fpath, "usr/@");
  fname = (char *) strchr(fname, '@');

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
  return 0;
}
