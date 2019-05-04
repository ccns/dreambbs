#include <stdarg.h>

static inline void
clrchyiuan(
int i, int j)
{
    while (i <= j)
    {
        move(i, 0);
        clrtoeol();
        i++;
    }
}

static inline void
outs_centered(const char *str)
{
    prints("%*s", d_cols>>1, "");
    outs(str);
}

static inline void
prints_centered(const char *fmt, ...)
{
    va_list args;
    char buf[512], *str;
    int cc;

    prints("%*s", d_cols>>1, "");

    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);
    for (str = buf; (cc = (unsigned char) *str); str++)
        outc(cc);
}

static inline int
show_file(char *filename, int y, int lines, int mode)
{
    FILE *fp;
    char buf[256];
    clrchyiuan(y, y + lines);
    move(y, d_cols>>1);
    if ((fp = fopen(filename, "r")))
    {
        while (fgets(buf, 256, fp) && lines--) {
            move(++y, d_cols>>1);
            outs(buf);
        }
        fclose(fp);
    }
    else
        return 0;

    return 1;
}

static inline void
usercomplete(
const char *msg, char *buf)
{
    vget(1, 0, msg, buf, IDLEN + 1, GET_USER);
}

static inline char *
Cdate(
time_t *chrono)
{
    struct tm *ptime;

    ptime = localtime(chrono);
    /* Thor.990329: y2k */
    sprintf(Bdate, "%02d/%02d/%02d",
            ptime->tm_year % 100, ptime->tm_mon + 1, ptime->tm_mday);
    return Bdate;
}
