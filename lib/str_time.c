#include <stdio.h>
#include <time.h>


/* static char datemsg[32]; */
static char datemsg[40];


char *
Btime(clock)
  time_t *clock;
{
  struct tm *t = localtime(clock);

  /* Thor.990329: y2k */
  /* Thor.990413: 最後的空格是用在 mail.c的bsmtp末, 在時間和user間空一格用,
                  嗯... 不知道放在這的好處是不是連空格也一起共用:P */
  sprintf(datemsg, "%02d/%02d/%02d%3d:%02d:%02d ",
    t->tm_year % 100, t->tm_mon + 1, t->tm_mday,
    t->tm_hour, t->tm_min, t->tm_sec);
  return (datemsg);
}


char *
Ctime(clock)
  time_t *clock;
{
  struct tm *t = localtime(clock);
  static char week[] = "日一二三四五六";

  sprintf(datemsg, "%d年%2d月%2d日%3d:%02d:%02d 星期%.2s",
    t->tm_year - 11, t->tm_mon + 1, t->tm_mday,
    t->tm_hour, t->tm_min, t->tm_sec, &week[t->tm_wday << 1]);
  return (datemsg);
}


char *
Etime(clock)
  time_t *clock;
{
  strftime(datemsg, 22, "%D %T %a", localtime(clock));
  return (datemsg);
}

char *
Atime(clock) /* Thor.990125: 假裝ARPANET時間格式 */
  time_t *clock;
{
  /* ARPANET format: Thu, 11 Feb 1999 06:00:37 +0800 (CST) */
  /* strftime(datemsg, 40, "%a, %d %b %Y %T %Z", localtime(clock)); */
  /* Thor.990125: time zone的傳回值不知和ARPANET格式是否一樣,先硬給,同sendmail*/
  strftime(datemsg, 40, "%a, %d %b %Y %T +0800 (CST)", localtime(clock));
  return (datemsg);
}

char *
Now()
{
  time_t now;

  time(&now);
  return Btime(&now);
}
