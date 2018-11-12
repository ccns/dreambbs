/*-------------------------------------------------------*/
/* util/countage.c      ( NTHU CS MapleBBS Ver 3.00 )    */
/*-------------------------------------------------------*/
/* author : wsyfish.bbs@fpg.m4.ntu.edu.tw		 */
/* Modify : visor.bbs@bbs.yzu.edu.tw			 */
/* target : 站上年齡統計                                 */
/* create : 97/09/05                                     */
/* update : 2000/07/27                                   */
/* 註: 修改 自原本 yearsold.c                		 */
/*-------------------------------------------------------*/


#include "bbs.h"
#define MAX_LINE        16
#define OUTFILE     BBSHOME"/gem/@/@-yearsold"

void
fouts(
  FILE *fp,
  char buf[], char mode)
{
  static char state = '0';

  if (state != mode)
    fprintf(fp, "\x1b[3%cm", state = mode);
  if (buf[0])
  {
    fprintf(fp, buf);
    buf[0] = 0;
  }
}

int
main(void)
{
  int  i, j;
  char buf[256],c;
  FILE *fp;
  int year, max, item, maxyear;
  int totalyear=0 ;
  int act[25];
  int ten=0;
  time_t now;
  struct tm *ptime;

  chdir(BBSHOME);

  now = time(NULL) ;
  ptime = localtime(&now);


  memset(act,0,sizeof(act));

  for (c = 'a'; c <= 'z'; c++)
  {
    char buf[64];
    struct dirent *de;
    DIR *dirp;

    sprintf(buf, BBSHOME "/usr/%c", c);

    if (!(dirp = opendir(buf)))
      continue;

    while ((de = readdir(dirp)))
    {
      ACCT cuser;
      USER_ATTR	attr;
      int fd;

      if (de->d_name[0] <= ' ' || de->d_name[0] == '.')
        continue;

      usr_fpath(buf,de->d_name,".ACCT"); 
      if ((fd = open(buf, O_RDONLY)) < 0)
        continue;

      read(fd, &cuser, sizeof(cuser));
      close(fd);
      memset(&attr,0,sizeof(USER_ATTR));
      if(attr_get(cuser.userid,ATTR_USER_KEY,&attr) < 0)
        continue;

    if(((ptime->tm_year - (attr.year -1900)) < 10) || ((ptime->tm_year - (attr.year-1900)) >33))
      continue;

    act[ptime->tm_year - (attr.year-1900) - 10]++;
    act[24]++;

     } /* while */
    closedir(dirp);
  } /* for */

  for (i = max  = totalyear = maxyear = 0; i < 24; i++)
  {
    totalyear += act[i] * (i + 10);
    if (act[i] > max)
    {
      max = act[i];
      maxyear = i;
    }
  }
  
  if(max >= 1000)
  {
    ten = 1;
  }

  item = max / MAX_LINE + 1;

  if ((fp = fopen(OUTFILE, "w")) == NULL)
  {
    printf("cann't open gem/@/@-yearsold\n");
    return 0;
  }
  
  fprintf(fp, "\t\t\t \x1b[1;33;45m 風之塔 年齡統計 [%02d/%02d/%02d] \x1b[m\n\n",
     (ptime->tm_year)%100,ptime->tm_mon+1,ptime->tm_mday);
  for (i = MAX_LINE + 1; i > 0; i--)
  {
    strcpy(buf, "   ");
    for (j = 0; j < 24; j++)
    {
      max = item * i;
      year = act[j];
      if (year && (max > year) && (max - item <= year))
      {
        fouts(fp, buf, '7');
        if(ten)
          fprintf(fp, "%-3d", year/10);
        else
          fprintf(fp, "%-3d", year);
      }
      else if (max <= year)
      {
        fouts(fp, buf, '4');
        fprintf(fp, "█ ");
      }
      else
        strcat(buf, "   ");
    }
    fprintf(fp, "\n");
  }

  fprintf(fp, "  \x1b[1;35m╒═══════════════════════════════════╕ \n"
    "   \x1b[1;32m10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33\n"
    "\t%s    \x1b[36m有效統計人次：\x1b[37m%-9d\x1b[36m平均年齡：\x1b[37m%d\x1b[40;0m\n"
    , ten ? "\033[36m單位：\033[37m10 人" : "\t",act[24],(int) totalyear / (act[24] ? act[24] : 1));
  fclose(fp);
  return 0;
}
