/*-------------------------------------------------------*/
/* util/backupusr.c     ( YZU WindTopBBS Ver 3.00 )      */
/*-------------------------------------------------------*/
/* author : visor.bbs@bbs.yzu.edu.tw			 */
/* target : 備份所有使用者資料                           */
/* create : 95/03/29                                     */
/* update : 97/03/29                                     */
/*-------------------------------------------------------*/

#include "bbs.h"

char	*bk_path;

static void
reaper(fpath, lowid)
  char *fpath;
  char *lowid;
{
  char buf[256];
  sprintf(buf,"tar zcvf %s/usr/%c/%s.tgz %s",bk_path,*lowid,lowid,lowid);
  system(buf);
}

static void
traverse(fpath)
  char *fpath;
{
  DIR *dirp;
  struct dirent *de;
  char *fname, *str;

  if (!(dirp = opendir(".")))
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
  char *fname, fpath[256],buf[256];
  if(argc>1)
    bk_path = argv[1];
  else
    bk_path = "/var/backup";

  strcpy(fname = fpath, "usr/@");
  fname = (char *) strchr(fname, '@');

  for (ch = 'a'; ch <= 'z'; ch++)
  {
    fname[0] = ch;
    fname[1] = '\0';
    sprintf(buf,"%s/usr/%c",bk_path,ch);
    mkdir(buf, 0755);
    sprintf(buf,"%s/%s",BBSHOME,fpath);
    chdir(buf);
    traverse(fpath);
  }
  for (ch = '0'; ch <= '9'; ch++)
  {
    fname[0] = ch;
    fname[1] = '\0';
    sprintf(buf,"%s/usr/%c",bk_path,ch);
    mkdir(buf, 0755);
    sprintf(buf,"%s/%s",BBSHOME,fpath);
    chdir(buf);
    traverse(fpath);
  }
  return 0;
}
