#include "config.h"

#include <stdio.h>
#include <time.h>
#include "dao.h"

/* static char datemsg[32]; */
static char datemsg[40];

void syncnow(void);

/* ------------------------------------------ */
/* mail / post �ɡA�̾ڮɶ��إ��ɮסA�[�W�l�W */
/* ------------------------------------------ */
/* Input: fpath = directory;                  */
/* Output: fpath = full path;                 */
/* ------------------------------------------ */

void str_stamp(char *str, const time_t * chrono)
{
    struct tm *ptime;

    ptime = localtime(chrono);
    /* Thor.990329: y2k */
    sprintf(str, "%02d/%02d/%02d",
            ptime->tm_year % 100, ptime->tm_mon + 1, ptime->tm_mday);
}

char *Btime(const time_t * clock)
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


char *Ctime(const time_t * clock)
{
    struct tm *t = localtime(clock);
    static const char week[] = "��@�G�T�|����";

    sprintf(datemsg, "%d�~%2d��%2d��%3d:%02d:%02d �P��%.2s",
            t->tm_year - 11, t->tm_mon + 1, t->tm_mday,
            t->tm_hour, t->tm_min, t->tm_sec, &week[2 * t->tm_wday]);
    return (datemsg);
}

char *Etime(const time_t * clock)
{
    strftime(datemsg, 22, "%D %T %a", localtime(clock));
    return (datemsg);
}

char *Atime(                    /* Thor.990125: ����ARPANET�ɶ��榡 */
               const time_t * clock)
{
    /* https://tools.ietf.org/html/rfc822  5.  DATE AND TIME SPECIFICATION*/
    /* https://tools.ietf.org/html/rfc1123 5.2.14  RFC-822 Date and Time Specification */

    /* ARPANET format: Thu, 11 Feb 1999 06:00:37 +0800 */
    /* strftime(datemsg, 40, "%a, %d %b %Y %T %Z", localtime(clock)); */
    /* Thor.990125: time zone���Ǧ^�Ȥ����MARPANET�榡�O�_�@��, ���w��, �Psendmail */
    strftime(datemsg, 40, "%a, %d %b %Y %T " BBSUTCZONE, localtime(clock));
    return (datemsg);
}

char *Now(void)
{
    time_t now = time(NULL);
    return Btime(&now);
}
