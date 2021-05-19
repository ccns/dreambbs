/*-------------------------------------------------------*/
/* util/makefw.c        ( YZU WindTopBBS Ver 3.00 )      */
/*-------------------------------------------------------*/
/* author : visor.bbs@bbs.yzu.edu.tw                     */
/* target : 更新各版面的檔信列表                         */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/

#include "bbs.h"

static FWOCACHE *fwoshm;
int total;

static void
expire(
    const char *brd)
{
    char buf[256];
    int fd;
    BANMAIL head;

    sprintf(buf, "%s/%s", brd, BANMAIL_ACL);
    fd = open(buf, O_RDONLY);
    if (fd>=0 && total < (MAXOFILEWALL-1))
    {
        while (read(fd, &head, sizeof(BANMAIL)) == sizeof(BANMAIL))
        {
            if (strlen(head.data) <=0)
                continue;
            strcpy(fwoshm->fwocache[total].name, brd);
            strcpy(fwoshm->fwocache[total].data, head.data);
            fwoshm->fwocache[total].usage = head.usage;
            fwoshm->fwocache[total].time = head.time;
            fwoshm->fwocache[total].mode = head.mode;
            printf("%-15s %6.6d %10.10ld %s\n", brd, head.usage, (long)head.time, head.data);
            total++;
            if (total >= (MAXOFILEWALL-1))
                break;
        }
        close(fd);
        memset(&(fwoshm->fwocache[total]), 0, sizeof(FW));
    }
}

int
main(
    int argc,
    char *argv[])
{
    struct dirent *de;
    DIR *dirp;
    char *ptr;


    setgid(BBSGID);
    setuid(BBSUID);
    chdir(BBSHOME);

    fwoshm_init(&fwoshm);
    fwoshm_load(fwoshm);

    if (chdir("brd") || !(dirp = opendir(".")))
    {
        exit(-1);
    }

    while ((de = readdir(dirp)))
    {
        ptr = de->d_name;
        if (ptr[0] > ' ' && ptr[0] != '.')
            expire(ptr);
    }
    closedir(dirp);
    printf("usage : %d (%d)\n", total, MAXOFILEWALL);
    if (total >= MAXOFILEWALL)
        printf("out of memory!");


    exit(0);
}
