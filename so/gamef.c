/*-------------------------------------------------------*/
/* gamef.c      ( WindTopBBS Ver 3.02 )                  */
/*-------------------------------------------------------*/
/* target : WindTop Games' API                           */
/* create : 2001/04/10                                   */
/* update : 2001/04/10                                   */
/*-------------------------------------------------------*/
#include <stdarg.h>
#include "bbs.h"
#define getdata(x1, x2, x3, x4, x5, x6, x7) vget(x1, x2, x3, x4, x5, DOECHO)
static char Bdate[20];

static inline char *
Cdate(
const time_t *chrono)
{
    struct tm *ptime;

    ptime = localtime(chrono);
    /* Thor.990329: y2k */
    sprintf(Bdate, "%02d/%02d/%02d",
            ptime->tm_year % 100, ptime->tm_mon + 1, ptime->tm_mday);
    return Bdate;
}

GCC_FORMAT(1, 2) static inline void
pressanykey(const char *fmt, ...)
{
    va_list args;
    char buf[512], *ptr;
    int cc, ch;

    if (fmt)
    {
        va_start(args, fmt);
        vsprintf(buf, fmt, args);
        va_end(args);
    }
    else
    {
        sprintf(buf, "\x1b[37;45;1m%*s�� �Ы� \x1b[33m(Space/Return)\x1b[37m �~�� ��%*s\x1b[m", (d_cols >> 1) + 22 , "", ((d_cols + 1) >> 1) + 21, "");
    }
    move(b_lines, 0);
    for (ptr = buf; (cc = (unsigned char) *ptr); ptr++)
        outc(cc);
    do
    {
        ch = vkey();
    }
    while ((ch != ' ') && (ch != KEY_LEFT) && (ch != '\r') && (ch != '\n'));

    move(b_lines, 0);
    clrtoeol();
    refresh();
}

GCC_FORMAT(2, 3) static inline void
game_log(int file, const char *fmt, ...)
{
    va_list args;
    char buf[200], ff[40];
    time_t now;
    FILE *fs;

    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);

    switch (file)
    {
    case 1: strcpy(ff, FN_MINE_LOG); break;
    case 2: strcpy(ff, "run/bj.log"); break;
    }
    fs = fopen(ff, "a+");
    now = time(0);
    fprintf(fs, "\x1b[1;33m%s \x1b[32m%s \x1b[36m%s\x1b[m\n", Cdate(&now), cuser.userid, buf);
    fclose(fs);
}

