/*-------------------------------------------------------*/
/* util/countstar.c     ( NTHU CS MapleBBS Ver 2.36 )    */
/*-------------------------------------------------------*/
/* author : wsyfish.bbs@fpg.m4.ntu.edu.tw                */
/* Modify : visor.bbs@bbs.yzu.edu.tw                     */
/* target : 站上星座統計                                 */
/* create : 97/09/05                                     */
/* update : 2000/07/27                                   */
/*-------------------------------------------------------*/

#include "bbs.h"
#define     OUTFILE     BBSHOME"/gem/@/@-star"

int
main(void)
{
  int i, j;
  FILE *fp;
  int max, item, maxhoroscope;
  char c;

  int act[12];

  char *name[13] = {"牡羊",
                    "金牛",
                    "雙子",
                    "巨蟹",
                    "獅子",
                    "處女",
                    "天秤",
                    "天蠍",
                    "射手",
                    "摩羯",
                    "水瓶",
                    "雙魚",
                    ""
                           };
  char    *blk[10] =
  {
      "  ","▏", "▎", "▍", "▌",
      "▋","▊", "▉", "█", "█",
  };

  chdir(BBSHOME);
  memset(act, 0, sizeof(act));

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

    switch (attr.month)
    {
      case 1:
        if (attr.day <=19)
          act[9]++;
        else
          act[10]++;
        break;
      case 2:
        if (attr.day <=18)
          act[10]++;
        else
          act[11]++;
        break;
      case 3:
        if (attr.day <=20)
          act[11]++;
        else
          act[0]++;
        break;
      case 4:
        if (attr.day <=19)
          act[0]++;
        else
          act[1]++;
        break;
      case 5:
        if (attr.day <=20)
          act[1]++;
        else
          act[2]++;
        break;
      case 6:
        if (attr.day <=21)
          act[2]++;
        else
          act[3]++;
        break;
      case 7:
        if (attr.day <=22)
          act[3]++;
        else
          act[4]++;
        break;
      case 8:
        if (attr.day <=22)
          act[4]++;
        else
          act[5]++;
        break;
      case 9:
        if (attr.day <=22)
          act[5]++;
        else
          act[6]++;
       break;
      case 10:
        if (attr.day <=23)
          act[6]++;
        else
          act[7]++;
       break;
      case 11:
        if (attr.day <=22)
          act[7]++;
        else
          act[8]++;
       break;
      case 12:
        if (attr.day <=21)
          act[8]++;
        else
          act[9]++;
       break;
    }


    } /* while */
    closedir(dirp);
  } /* for */



  for (i = max  = maxhoroscope = 0; i < 12; i++)
  {
    if (act[i] > max)
    {
      max = act[i];
      maxhoroscope = i;
    }
  }

  item = max / 30 + 1;

  if ((fp = fopen(OUTFILE, "w")) == NULL)
  {
    printf("cann't open %s\n",OUTFILE);
    return 0;
  }

  for(i = 0;i < 12;i++)
  {
    fprintf(fp," \x1b[1;37m%s座 \x1b[0;36m", name[i]);
    for(j = 0; j < act[i]/item; j++)
    {
      fprintf(fp,"%2s",blk[9]);
    }
    /* 為了剛好一頁 */
    if (i != 11)
      fprintf(fp,"%2s \x1b[1;37m%d\x1b[m\n\n",blk[(act[i] % item) * 10 / item],
            act[i]);
    else
      fprintf(fp,"%2s \x1b[1;37m%d\x1b[m\n",blk[(act[i] % item) * 10 / item],
            act[i]);
  }
  fclose(fp);
  return 0;
}
