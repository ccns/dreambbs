/*-------------------------------------------------------*/
/* util/resetvmail.c    ( YZU WindTop 3.00 ) 		 */
/*-------------------------------------------------------*/
/* author : visor.bbs@bbs.yzu.edu.tw			 */
/* target : �M���S�w�w���U�� Email �������U              */
/* create : 95/03/29                                     */
/* update : 97/03/29                                     */
/*-------------------------------------------------------*/

#include "bbs.h"


void
acct_save(acct)
  ACCT *acct;
{
  int fd;
  char fpath[80];

  usr_fpath(fpath, acct->userid, ".ACCT");
  fd = open(fpath, O_WRONLY, 0600);     /* fpath �����w�g�s�b */
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

  char buf[256],*ptr;
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
  if(acct.userlevel & PERM_VALID)
  {
    
    if ((ptr = strstr(acct.email,"cloud.yzu.edu.tw")));
    else if ((ptr = strstr(acct.email,"mozart.yzu.edu.tw")));
    else if ((ptr = strstr(acct.email,"wind.yzu.edu.tw")));
    else if ((ptr = strstr(acct.email,"bach.yzu.edu.tw")));
    else if ((ptr = strstr(acct.email,"graduate.yzu.edu.tw")));
    else if ((ptr = strstr(acct.email,"moon.yzu.edu.tw")));
    else if ((ptr = strstr(acct.email,"mail86.yzu.edu.tw")));
    else if ((ptr = strstr(acct.email,"mail87.yzu.edu.tw")));
    else if ((ptr = strstr(acct.email,"mail88.yzu.edu.tw")));
    else if ((ptr = strstr(acct.email,"mail.yzu.edu.tw")));
    if(ptr)
    {
      acct.userlevel &= ~PERM_VALID;
/*      strcpy(ptr,"mail.yzu.edu.tw");
      strcpy(acct.vmail,acct.email);*/
    }
  }
  
  acct_save(&acct);

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
