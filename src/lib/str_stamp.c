/* ------------------------------------------ */
/* mail / post �ɡA�̾ڮɶ��إ��ɮסA�[�W�l�W */
/* ------------------------------------------ */
/* Input: fpath = directory;		      */
/* Output: fpath = full path;		      */
/* ------------------------------------------ */


#include <time.h>
#include <stdlib.h>
#include <stdio.h>

void
str_stamp(str, chrono)
  char *str;
  time_t *chrono;
{
  struct tm *ptime;

  ptime = localtime(chrono);
  /* Thor.990329: y2k */
  sprintf(str, "%02d/%02d/%02d",
    ptime->tm_year % 100, ptime->tm_mon + 1, ptime->tm_mday);
}
