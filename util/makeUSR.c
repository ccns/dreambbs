/*-------------------------------------------------------*/
/* util/makeUSR.c    ( YZU WindTopBBS Ver 3.00 )         */
/*-------------------------------------------------------*/
/* author : visor.bbs@bbs.yzu.edu.tw                     */
/* target : 重建 .USR                                    */
/* create : 2000/11/05                                   */
/* update :                                              */
/*-------------------------------------------------------*/
/* syntax : makeUSR                                      */
/* NOTICE : 重建 .USR                                    */
/*-------------------------------------------------------*/
#include "bbs.h"

#undef	FAKE_IO

typedef struct
{
    char userid[IDLEN+1];
}	MAP;
MAP map[100000];


int total;

static void
reaper(
    char *fpath,
    char *lowid)
{
    int fd;

    char buf[256];
    ACCT acct;

    sprintf(buf, "%s/.ACCT", fpath);
    fd = open(buf, O_RDONLY, 0);
    if (fd < 0)
        return;

    if (read(fd, &acct, sizeof(acct))!=sizeof(acct))
    {
        close(fd);
        return;
    }
    strcpy(map[acct.userno].userid,acct.userid);
    if (total<acct.userno)
        total = acct.userno;

    printf("%-14s : %d\n",acct.userid,acct.userno);

    close(fd);
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
    int ch, mode = 0;
    char *fname, fpath[256];

    setgid(BBSGID);
    setuid(BBSUID);
    chdir(BBSHOME);

//  memset(map,0,sizeof(MAP)60000);
    memset(map,0,sizeof(map));

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
#ifndef FAKE_IO
    {
        SCHEMA slot;
        int fd,num;

        fd = open(".USR.new", O_CREAT | O_TRUNC | O_WRONLY, 0600);
        for (num = 1; num <= total; num++)
        {
            memset(&slot, 0, sizeof(SCHEMA));
            strcpy(slot.userid, map[num].userid);
            time(&slot.uptime);
            write(fd, &slot, sizeof(SCHEMA));
        }
        close(fd);
    }
#endif

    return 0;
}
