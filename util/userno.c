/*-------------------------------------------------------*/
/* util/rebuilduser.c   ( YZU_CSE WindTop 2000 )         */
/*-------------------------------------------------------*/
/* target : 檢查是否有相同的 userno                      */
/* create : 2000/06/08                                   */
/* update :                                              */
/*-------------------------------------------------------*/
/* syntax : rebuilduser                                  */
/*          自動輸出自 run/userno.log (FN_USERNO_LOG)    */
/*-------------------------------------------------------*/

#include "bbs.h"

typedef struct
{ 
  int userno;
  char userid[IDLEN+1];
}	MAP;
MAP map[60000];


static int funo;
FILE *flog;
int total;


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
  strcpy(map[total].userid,acct.userid);
  map[total].userno = acct.userno;
  total++;
  close(fd);
  fprintf(flog, "%14s  %5d\n",acct.userid,acct.userno);
  /*acct_save(&acct);*/
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

  while ( ( de = readdir(dirp) ) )
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
  int i,j,k;

  total = 1;
  funo = open(".USR", O_RDWR | O_CREAT, 0600);

  if (funo < 0)
    return 0;

  flog = fopen(FN_USERNO_LOG, "w");

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
  close(funo);
  total--;
  fprintf(flog, "total user %d\n",total);
  for(i=1;i<=total;i++)
  {
     k = 0;
     for(j=0;j<total;j++)
     {
        if(map[j].userno == i)
          k=1;
        if((map[i-1].userno == map[j].userno)&&((i-1)!=j))
          fprintf(flog, "userno %d is the same ~~\n",map[i-1].userno);
     }
     if(k==0)
       fprintf(flog, "userno %d is empty!\n", i);    
  }
  return 0;
}
