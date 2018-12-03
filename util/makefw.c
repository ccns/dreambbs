/*-------------------------------------------------------*/
/* util/makefw.c	( YZU WindTopBBS Ver 3.00 )	 */
/*-------------------------------------------------------*/
/* author : visor.bbs@bbs.yzu.edu.tw			 */
/* target : 更新各版面的檔信列表                         */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/

#include <strings.h>
#include "bbs.h"
#include <sys/shm.h>

FWOCACHE *fwoshm;
int total;
FW *cur;

static int
cmpfw(
    BANMAIL *ban)
{
    return !strcmp(ban->data, cur->data);
}

static void *
attach_shm(
    register int shmkey, register int shmsize)
{
    register void *shmptr;
    register int shmid;

    shmid = shmget(shmkey, shmsize, 0);
    if (shmid < 0)
    {
        shmid = shmget(shmkey, shmsize, IPC_CREAT | 0600);
    }
    else
    {
        shmsize = 0;
    }

    shmptr = (void *) shmat(shmid, NULL, 0);

    if (shmsize)
        memset(shmptr, 0, shmsize);

    return shmptr;
}


static void
expire(
    char *brd)
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
            printf("%-15s %6.6d %10.10ld %s\n", brd, head.usage, head.time, head.data);
            total++;
            if (total >= (MAXOFILEWALL-1))
                break;
        }
        close(fd);
        memset(&(fwoshm->fwocache[total]), 0, sizeof(FW));
    }
}

static void
rewrite(void)
{
    BANMAIL data;
    FW *head;
    char fpath[128];
    int pos;
    time_t now;

    now = time(0);

    head = fwoshm->fwocache;
    while (head->name[0])
    {
        brd_fpath(fpath, head->name, BANMAIL_ACL);
        cur = head;
        pos = rec_loc(fpath, sizeof(BANMAIL), cmpfw);
        if (pos >= 0)
        {
            rec_get(fpath, &data, sizeof(BANMAIL), pos);
            data.usage = head->usage;
            if ((now - head->time) > (BANMAIL_EXPIRE * 86400))
                rec_del(fpath, sizeof(BANMAIL), pos, NULL, NULL);
            else
            {
                data.time = head->time;
                rec_put(fpath, &data, sizeof(BANMAIL), pos);
            }
        }
        head++;
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

    fwoshm = attach_shm(FWOSHM_KEY, sizeof(FWOCACHE));

    rewrite();

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
