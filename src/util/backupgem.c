/*-------------------------------------------------------*/
/* util/backupgem.c     ( YZU WindTopBBS Ver 3.00 )      */
/*-------------------------------------------------------*/
/* author : visor.bbs@bbs.yzu.edu.tw			 */
/* target : 備份所有精華區資料                           */
/* create : 95/03/29                                     */
/* update : 97/03/29                                     */
/*-------------------------------------------------------*/
#include <strings.h>


#include "bbs.h"
char 	*bk_path;

static void
expire(brd)
  char *brd;
{
  char fpath[128];
  sprintf(fpath,"tar zcvf %s/gem/%s.tgz %s",bk_path,brd,brd);
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


  if (chdir("gem/brd") || !(dirp = opendir(".")))
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
