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
    char *lowid)
{
    char buf[256];
    sprintf(buf, "tar zxvf /var/tape/brd/%s ", lowid);
    system(buf);
}

static void
traverse(
    char *fpath)
{
    DIR *dirp;
    struct dirent *de;
    char *fname;

    if (!(dirp = opendir(fpath)))
    {
        return;
    }

    while ((de = readdir(dirp)))
    {
        fname = de->d_name;
        if (fname[0] > ' ' && fname[0] != '.')
        {
            reaper(fname);
        }
    }
    closedir(dirp);
}

int
main(void)
{
    char fpath[256];

    strcpy(fpath, "/var/tape/brd");

    chdir("/home/bbs/brd");
    traverse(fpath);
    return 0;
}
