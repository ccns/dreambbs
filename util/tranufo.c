/*-------------------------------------------------------*/
/* util/template.c      ( YZU WindTop 2000 )             */
/*-------------------------------------------------------*/
/* author : visor.bbs@bbs.yzu.edu.tw                     */
/* target : 標本用                                       */
/* create : 98/03/29                                     */
/* update : 98/05/29                                     */
/*-------------------------------------------------------*/
/* syntax :                                              */
/*-------------------------------------------------------*/

#include "bbs.h"

typedef struct
{
    unsigned int old;
    unsigned int new;
}       TABLE;

TABLE table[] = {
    {UFO_COLOR, UFO2_COLOR},
    {UFO_MOVIE, UFO2_MOVIE},
    {UFO_BRDNEW, UFO2_BRDNEW},
    {UFO_BNOTE, UFO2_BNOTE},
    {UFO_VEDIT, UFO2_VEDIT},
    {UFO_PAL, UFO2_PAL},
    {UFO_MOTD, UFO2_MOTD},
    {UFO_MIME, UFO2_MIME},
    {UFO_SIGN, UFO2_SIGN},
    {UFO_SHOWUSER, UFO2_SHOWUSER},
    {UFO_REALNAME, UFO2_REALNAME},
    {UFO_SHIP, UFO2_SHIP},
    {UFO_NWLOG, UFO2_NWLOG},
    {UFO_NTLOG, UFO2_NTLOG},
    {UFO_ACL, UFO2_ACL},
    {0, 0}
};

void
acct_save(
    ACCT *acct)
{
    int fd;
    char fpath[80];

    usr_fpath(fpath, acct->userid, FN_ACCT);
    fd = open(fpath, O_WRONLY, 0600);     /* fpath 必須已經存在 */
    if (fd >= 0)
    {
        write(fd, acct, sizeof(ACCT));
        close(fd);
    }
}

static void
reaper(
    char *fpath,
    char *lowid)
{
    int fd;

    char buf[256];
    ACCT acct;
    TABLE *ptr;

    sprintf(buf, "%s/.ACCT", fpath);
    fd = open(buf, O_RDWR, 0);
    if (fd < 0)
        return;

    if (read(fd, &acct, sizeof(acct))!=sizeof(acct))
    {
        close(fd);
        return;
    }
    close(fd);
    acct.ufo2 = 0;
    for (ptr = table; ptr->old; ptr++)
    {
        if (acct.ufo & ptr->old)
            acct.ufo2 |= ptr->new;
        else
            acct.ufo2 &= ~ptr->new;
    }
    printf("%-14.14s : ufo) %#18.18x ufo2) %#18.18x\n", acct.userid, acct.ufo, acct.ufo2);
    acct_save(&acct);

}

static void
traverse(
    char *fpath)
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

    while ((de = readdir(dirp)))
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
    return 0;
}
