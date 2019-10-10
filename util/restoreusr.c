/*-------------------------------------------------------*/
/* util/restoreuser.c   ( YZU_CSE WindTop 2000 )         */
/*-------------------------------------------------------*/
/* target : 將備份的資料夾中的所有使用者復原             */
/*          於某次使用者硬碟損毀時所建立                 */
/* create : 2000/06/08                                   */
/* update :                                              */
/*-------------------------------------------------------*/

#include "bbs.h"

static void
reaper(
    const char *fpath,
    const char *lowid)
{
    char buf[256];
    sprintf(buf, "tar zxvf /var/tape/usr/%c/%s -C \"" BBSHOME "/usr/%c\"", *lowid, lowid, *lowid);
    system(buf);
}

static void
traverse(
    char *fpath)
{
    DIR *dirp;
    struct dirent *de;
    char *fname, *str;

    if (!(dirp = opendir(".")))
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

    strcpy(fname = fpath, "/var/tape/usr/@");
    fname = (char *) strchr(fname, '@');

    for (ch = 'a'; ch <= 'z'; ch++)
    {
        fname[0] = ch;
        fname[1] = '\0';
        chdir(fpath);
        traverse(fpath);
    }
    for (ch = '0'; ch <= '9'; ch++)
    {
        fname[0] = ch;
        fname[1] = '\0';
        chdir(fpath);
        traverse(fpath);
    }
    return 0;
}
