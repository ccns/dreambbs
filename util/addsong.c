/*-------------------------------------------------------*/
/* util/addsong.c       ( YZU WindTopBBS Ver 3.00 )      */
/*-------------------------------------------------------*/
/* author : visor.bbs@bbs.yzu.edu.tw                     */
/* target : 每個星期增加點歌次數                         */
/* create : 20000/07/03                                  */
/* update :                                              */
/*-------------------------------------------------------*/
/* syntax : addsong [prem] [times]                       */
/* NOTICE : add users' request times                     */
/*-------------------------------------------------------*/
#include "bbs.h"

static int perm;
static int num;

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

  if(acct.userlevel & perm)
  {
    acct.request += num;
    if(acct.request > 500)
      acct.request = 500;
    if(acct.request < 0)
      acct.request = 0;
    acct_save(&acct);
  }
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
main(argc,argv)
  int argc;
  char *argv[];
{
  int ch;
  char *fname, fpath[256];

  chdir(BBSHOME);

  strcpy(fname = fpath, "usr/@");
  fname = (char *) strchr(fname, '@');

  if(argc > 2)
  {
    perm = atoi(argv[1]);
    num  = atoi(argv[2]);

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
  return 0;
}
