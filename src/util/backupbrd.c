/*-------------------------------------------------------*/
/* util/backupbrd.c     ( YZU WindTopBBS Ver 3.00 )      */
/*-------------------------------------------------------*/
/* author : visor.bbs@bbs.yzu.edu.tw			 */
/* target : 備份所有版面資料                             */
/* create : 95/03/29                                     */
/* update : 97/03/29                                     */
/*-------------------------------------------------------*/
#include <strings.h>


#include "bbs.h"
char	*bk_path;

static void
expire(brd)
  char *brd;
{
  char fpath[128];
  time_t now;
  struct tm ntime, *xtime;
  now = time(NULL);
  xtime = localtime(&now);
  ntime = *xtime;
  if(!strcmp(brd,"LocalPosts"))
    sprintf(fpath,"tar zcvf %s/brd/%s%02d%02d.tgz %s",bk_path,brd,ntime.tm_mon + 1, ntime.tm_mday,brd);
  else
    sprintf(fpath,"tar zcvf %s/brd/%s.tgz %s",bk_path,brd,brd);
  system(fpath);
}


int
main(argc, argv)
  int argc;
  char *argv[];
{
  struct dirent *de;
  DIR *dirp;
  char *ptr;

  if(argc > 1)
    bk_path = argv[1];
  else
    bk_path = "/var/backup";

  setgid(BBSGID);
  setuid(BBSUID);
  chdir(BBSHOME);
  


  if (chdir("brd") || !(dirp = opendir(".")))
  {
    exit(-1);
  }

  while ((de = readdir(dirp)))
  {
    ptr = de->d_name;

    if (ptr[0] > ' ' && ptr[0] != '.')
      expire(ptr);
  }
  closedir(dirp);

  exit(0);
}
