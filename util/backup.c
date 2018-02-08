/*-------------------------------------------------------*/
/* util/backup.c     ( ES BBS )				 */
/*-------------------------------------------------------*/
/* author : catalyst.bbs@bbs.es.ncku.edu.tw		 */
/* target : 排程備份使用者/看板/精華區                   */
/* create : 2007/05/28                                   */
/* comment: Sun usr a-d					 */
/*	    Mon usr e-k					 */
/*	    Tue usr l-r					 */
/*	    Wed usr s-z					 */
/*	    Thr brd a-k 0-9				 */
/*	    Fri brd l-z					 */
/*	    Sat gem					 */
/*	    everyday src system				 */
/*-------------------------------------------------------*/

#include "bbs.h"

char	*bk_path;

struct tm *t;
time_t now;
int mon, mday;

static void
log_backup(msg)
  char *msg;
{
  time_t now;
  FILE *fp;

  time(&now);

  chdir(BBSHOME);

  fp = fopen("run/backup.log_backup","a+");
  fprintf(fp,"%24.24s %s\n",ctime(&now),msg);
  
  fclose(fp);
  
}

static void
proceed(fpath)
  char *fpath;
{
  DIR *dirp;
  struct dirent *de;
  char *fname, *str;
  char cmd[256];

  if (!(dirp = opendir(".")))
    return;

  for (str = fpath; *str; str++);
  *str++ = '/';

  while (de = readdir(dirp))
  {
    fname = de->d_name;
    if (fname[0] > ' ' && fname[0] != '.')
    {
      //strcpy(str, fname);
      sprintf(cmd,"tar zcf %s/usr/usr%02d%02d/%c/%s.tgz %s", bk_path, mon, mday, *fname, fname, fname);
      system(cmd);
    }
  }
  closedir(dirp);
}

static void
bk_usr(day)
  int day;
{
  char start[4] = {'a','e','l','s'}, end[4] = {'d','k','r','z'};
  char buf[256], fpath[256], *fname;
  char ch;
  
  chdir(BBSHOME);

  sprintf(buf,"usr %c-%c backup start", start[day], end[day]);
  log_backup(buf);

  strcpy(fname = fpath, "usr/@");
  fname = (char *) strchr(fname, '@');

  sprintf(buf, "%s/usr/usr%02d%02d",bk_path, mon, mday);
  mkdir (buf, 0700);

  for (ch = start[day]; ch <= end[day]; ch++)
  {
    fname[0] = ch;
    fname[1] = '\0';
    sprintf(buf,"%s/usr/usr%02d%02d/%c",bk_path, mon, mday, ch);
    mkdir(buf, 0700);
    sprintf(buf,"%s/%s",BBSHOME,fpath);
    chdir(buf);
    proceed(fpath);
  }

  sprintf(buf,"usr %c-%c backup complete", start[day], end[day]);
  log_backup(buf);

}


static void
bk_brd(day)
  int day;
{
  struct dirent *de;
  DIR *dirp;
  char *ptr, cmd[256];

  //chdir(BBSHOME);
  
  if(day != 4 && day != 5)
    return;

  
  if (chdir(BBSHOME "/brd") || !(dirp = opendir(".")))
    return;

  sprintf(cmd,"brd %s backup start",(day == 4) ? "#-k" : "l-z");
  log_backup(cmd);

  sprintf(cmd, "%s/brd/brd%02d%02d", bk_path, mon, mday);
  mkdir(cmd, 0700);
  
  chdir(BBSHOME "/brd");
  
  while (de = readdir(dirp))
  {
    ptr = de->d_name;

    if (ptr[0] > ' ' && ptr[0] != '.')
    {
      if((day == 4 && ((ptr[0] >= '0' && ptr[0] <= '9') || (ptr[0] >= 'a' && ptr[0] <= 'k') || (ptr[0] >= 'A' && ptr[0] <= 'K'))) ||
         (day == 5 && ((ptr[0] >= 'l' && ptr[0] <= 'z') || (ptr[0] >= 'L' && ptr[0] <= 'Z'))))
      {
        sprintf(cmd,"tar zvcf %s/brd/brd%02d%02d/%s.tgz %s",bk_path, mon, mday, ptr, ptr);
        system(cmd);
      }
      else
        continue;
    }
  }
  closedir(dirp);

  sprintf(cmd,"brd %s backup complete",(day == 4) ? "#-k" : "l-z");
  log_backup(cmd);
}

static void
bk_gem()
{
  struct dirent *de;
  DIR *dirp;
  char *ptr, cmd[256];

  //chdir(BBSHOME);
  
  if (chdir(BBSHOME "/gem/brd") || !(dirp = opendir(".")))
    return;

  log_backup("gem backup start");

  sprintf(cmd, "%s/gem/gem%02d%02d", bk_path, mon, mday);
  mkdir(cmd, 0700);
  
  chdir(BBSHOME "/gem/brd");
   
  while (de = readdir(dirp))
  {
    ptr = de->d_name;

    if (ptr[0] > ' ' && ptr[0] != '.')
    {
        sprintf(cmd,"tar zcf %s/gem/gem%02d%02d/%s.tgz %s",bk_path, mon, mday, ptr, ptr);
        system(cmd);
    }
  }
  closedir(dirp);
  log_backup("gem backup complete");
}

static void
bk_system_src()
{
  char system_folders[5][9] = {"bin","etc","innd","newboard","dreambbs"};
  char path[64],cmd[256];
  int i;
  
  chdir(BBSHOME);
  
  log_backup("system backup start");

  sprintf(path,"%s/system/system%02d%02d",bk_path, mon, mday);
  mkdir(path, 0755);
  
  sprintf(cmd,"gzip -c .USR > %s/USR.gz", path);
  system(cmd);
  
  sprintf(cmd,"gzip -c .BRD > %s/BRD.gz", path);
  system(cmd);

  for(i=0;i<5;i++)
  {
    sprintf(cmd,"tar zcf %s/%s.tgz %s", path, system_folders[i], system_folders[i]);
    system(cmd);
  }
  
  sprintf(cmd,"touch %s/gem.tar", path);
  system(cmd);
  sprintf(cmd,"tar rvf %s/gem.tar gem/.DIR", path);
  system(cmd);
  sprintf(cmd,"tar rvf %s/gem.tar gem/.GEM", path);
  system(cmd);
  for(i = '0' ; i <= '9' ; i++)
  {
    sprintf(cmd,"tar rvf %s/gem.tar gem/%c", path,i);
    system(cmd);
  }
  for(i = 'A' ; i <= 'V' ; i++)
  {
    sprintf(cmd,"tar rvf %s/gem.tar gem/%c", path,i);
    system(cmd);
  }
  sprintf(cmd,"tar rvf %s/gem.tar gem/@",path);
  system(cmd);
  
  sprintf(cmd,"gzip -9 %s/gem.tar",path);
  system(cmd);

  log_backup("system backup complete");
  
}


int 
main(argc, argv)
  int argc;
  char *argv[];       
{
  int day;

  setgid(BBSGID);
  setuid(BBSUID);
  chdir(BBSHOME);
  
  time(&now);
  t = localtime(&now);

  day = t->tm_wday;

  if(argc>1)
    day = atoi(argv[1]);
  if(day > 6 || day < 0)
  {
    printf("Something error!\n");
    return 0;
  }
  

  bk_path = "/var/backup";

  mon = t->tm_mon+1;
  mday = t->tm_mday;
  
  bk_system_src();
  if(day >= 0 && day <= 3)
    bk_usr(day);
  else if(day >= 4 && day <= 5)
    bk_brd(day);
  else
    bk_gem();

}
