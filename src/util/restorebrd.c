/*-------------------------------------------------------*/
/* util/restoreuser.c   ( YZU_CSE WindTop 2000 )         */
/*-------------------------------------------------------*/
/* target : �N�ƥ�����Ƨ������Ҧ��ϥΪ̴_��             */
/*          ��Y���ϥΪ̵w�зl���ɩҫإ�                 */
/* create : 2000/06/08                                   */
/* update :                                              */
/*-------------------------------------------------------*/

#include "bbs.h"

static void
reaper(lowid)
  char *lowid;
{
  char buf[256];
  sprintf(buf,"tar zxvf /var/tape/brd/%s ",lowid);
  system(buf);
}

static void
traverse(fpath)
  char *fpath;
{
  DIR *dirp;
  struct dirent *de;
  char *fname;

  if (!(dirp = opendir(fpath)))
  {
    return;
  }

  while ((de = readdir(dirp)))
  {
    fname = de->d_name;
    if (fname[0] > ' ' && fname[0] != '.')
    {
      reaper(fname);
    }
  }
  closedir(dirp);
}

int
main(void)
{
  char fpath[256];

  strcpy(fpath, "/var/tape/brd");

  chdir("/home/bbs/brd");
  traverse(fpath);
  return 0;
}
