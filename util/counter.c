/*-------------------------------------------------------*/
/* util/counter.c       ( YZU WindTopBBS Ver 3.00 )      */
/*-------------------------------------------------------*/
/* author : visor.bbs@bbs.yzu.edu.tw			 */
/* target : 歷史軌跡的紀錄                               */
/* create : 95/03/29                                     */
/* update : 97/03/29                                     */
/*-------------------------------------------------------*/
/* syntax : counter                                   	 */
/*-------------------------------------------------------*/

#include "bbs.h"
#include <sys/shm.h>

COUNTER *count;

static void *
attach_shm(
  register int shmkey, register int shmsize)
{
  register void *shmptr;
  register int shmid;

  shmid = shmget(shmkey, shmsize, 0);
  if (shmid < 0)
  {
    shmid = shmget(shmkey, shmsize, IPC_CREAT | 0600);
  }
  else
  {
    shmsize = 0;
  }

  shmptr = (void *) shmat(shmid, NULL, 0);

  if (shmsize)
    memset(shmptr, 0, shmsize);

  return shmptr;
}

int
main(
  int argc,
  char *argv[])
{
  int fd;
  time_t now;
  struct tm ntime, *xtime ,ptime;
  FILE *fp;
  char ymd[80],flag;
  
  if(argc > 1)
  {
    if(*argv[1] == 'r')  flag = 1;
    else if(*argv[1] == 'w')  flag = 2;
    else flag = 0;
  }
  else
    flag = 0;

  now = time(NULL);
  xtime = localtime(&now);
  ntime = *xtime;
  
  sprintf(ymd, "%02d/%02d/%02d",
    ntime.tm_year % 100, ntime.tm_mon + 1, ntime.tm_mday);
    
  fp = fopen("etc/counter","a+");  

  count = attach_shm(COUNT_KEY, sizeof(COUNTER));
  
  now = count->samehour_max_time;
  xtime = localtime(&now);
  ptime = *xtime;
    
  if (flag == 1)
  {
    printf("\nhour_max_login = %d \n",count->hour_max_login);
    printf("day_max_login = %d \n",count->day_max_login);
    printf("samehour_max_login = %d \n",count->samehour_max_login);
    printf("max_regist = %d \n",count->max_regist);
    printf("cur_hour_max_login = %d \n",count->cur_hour_max_login);
    printf("cur_day_max_login = %d \n",count->cur_day_max_login);
    printf("samehour_max_time = %d \n",(int) count->samehour_max_time);
    printf("samehour_max_login_old = %d \n",count->samehour_max_login_old);
    printf("max_regist_old = %d \n",count->max_regist_old);
  }
  if (flag == 2)
  {
    if(argc > 5)
    {
      count->samehour_max_login = atoi(argv[2]);
      count->max_regist = atoi(argv[3]);
      count->hour_max_login = atoi(argv[4]);
      count->day_max_login = atoi(argv[5]);
    }
    else
    {
      printf("bin/counter w SML MR MHL MHD \n");
      printf("SML = 同時在站內人數\n");
      printf("MR = 總註冊人數\n");
      printf("MHL = 單一小時上線人次\n");
      printf("MHD = 單日上線人次\n");
    }
  }

  if(flag == 0)
  {
    if(count->max_regist > count->max_regist_old)
    {
      fprintf(fp,"★ 【%s】\x1b[1;32m總註冊人數\x1b[m提升到 \x1b[31;1m%d\x1b[m 人\n",ymd,count->max_regist);
      count->max_regist_old = count->max_regist;
    }

    if(count->samehour_max_login > count->samehour_max_login_old)
    {
      fprintf(fp,"◎ 【%s %02d:%02d】\x1b[32m同時在站內人數\x1b[m首次達到 \x1b[1;36m%d\x1b[m 人次\n",ymd,ptime.tm_hour,ptime.tm_min,count->samehour_max_login);
      count->samehour_max_login_old = count->samehour_max_login;
    }

    if(count->cur_hour_max_login > count->hour_max_login)
    {
      fprintf(fp,"◇ 【%s %02d】\x1b[1;32m單一小時上線人次\x1b[m首次達到 \x1b[1;35m%d\x1b[m 人次\n",ymd,ntime.tm_hour,count->cur_hour_max_login);
      count->hour_max_login = count->cur_hour_max_login;
    }
    count->cur_hour_max_login = 0;

    if (ntime.tm_hour == 0)
    {
      if(count->cur_day_max_login > count->day_max_login)
      {
        fprintf(fp,"◆ 【%s】\x1b[1;32m單日上線人次\x1b[m首次達到 \x1b[1;33m%d\x1b[m 人次\n",ymd,count->cur_day_max_login);
        count->day_max_login = count->cur_day_max_login;
      }
      count->cur_day_max_login = 0;
    }
    if ((fd = open("run/var/counter", O_WRONLY | O_CREAT | O_TRUNC, 0600)))
    {
      write(fd,count,sizeof(COUNTER));
      close(fd);
    }
  }
  fclose(fp);
  return 0;
}
