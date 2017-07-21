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
  /* Thor.990413: �̫᪺�Ů�O�Φb mail.c��bsmtp��, �b�ɶ��Muser���Ť@���,
                  ��... �����D��b�o���n�B�O���O�s�Ů�]�@�_�@��:P */
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
  static char week[] = "��@�G�T�|����";

  sprintf(datemsg, "%d�~%2d��%2d��%3d:%02d:%02d �P��%.2s",
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
Atime(clock) /* Thor.990125: ����ARPANET�ɶ��榡 */
  time_t *clock;
{
  /* ARPANET format: Thu, 11 Feb 1999 06:00:37 +0800 (CST) */
  /* strftime(datemsg, 40, "%a, %d %b %Y %T %Z", localtime(clock)); */
  /* Thor.990125: time zone���Ǧ^�Ȥ����MARPANET�榡�O�_�@��,���w��,�Psendmail*/
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
