/*-------------------------------------------------------*/
/* mailtoall.c          ( YZU WindTopBBS Ver 3.00 )      */
/*-------------------------------------------------------*/
/* author : visor.bbs@bbs.yzu.edu.tw                     */
/* target : 寄信給全站使用者或所有版主                   */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/


#include "bbs.h"
#include <sys/shm.h>

BCACHE *bshm;



typedef struct
{
    char id[IDLEN+1];
    char brd[IDLEN+1];
    int check;
} BM;


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

static int
check_in_memory(const char *bm, const char *id)
{
    const char *i;
    for (i=bm; strlen(i); i=i+IDLEN+1)
    if (!strcmp(i, id))
        return 0;
    return 1;
}


static void
send_to_all(const char *title, const char *fpath, const char *bm)
{
    char buf[128];
    const char *ptr;
    HDR mhdr;

    for (ptr=bm; strlen(ptr); ptr=ptr+IDLEN+1)
    {
        usr_fpath(buf, ptr, FN_DIR);
        hdr_stamp(buf, HDR_LINK, &mhdr, fpath);
        strcpy(mhdr.owner, "SYSOP");
        strcpy(mhdr.title, title);
        mhdr.xmode = MAIL_MULTI;
        rec_add(buf, &mhdr, sizeof(HDR));
    }
}

static int
to_bm(
    const char *fpath,
    const char *title)
{
    BRD *bhdr, *head, *tail;
    char *ptr, *bm;

    bm = (char *)malloc(MAXBOARD*(IDLEN+1)*3);
    memset(bm, 0, MAXBOARD*(IDLEN+1)*3);
    ptr = bm;

    head = bhdr = bshm->bcache;
    tail = bhdr + bshm->number;
    do                          /* 至少有sysop一版 */
    {
        char *c;
        char buf[BMLEN + 1];

        strcpy(buf, head->BM);
        c = buf;
        while (1)
        {
            char *d;
            d = strchr(c, '/');
            if (*c)
            {
                if (d)
                {
                    *d++ = 0;
                    if (check_in_memory(bm, c))
                    {
                        strcpy(ptr, c);
                        ptr+=IDLEN+1;
                    }
                    c = d;
                }
                else
                {
                    if (check_in_memory(bm, c))
                    {
                        strcpy(ptr, c);
                        ptr+=IDLEN+1;
                    }
                    break;
                }
            }
            else
                break;
        }
    } while (++head < tail);

    send_to_all(title, fpath, bm);

    unlink(fpath);
    free(bm);
    return 0;
}

static void
traverse(
    char *fpath,
    const char *path,
    const char *title)
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
        HDR mhdr;
        fname = de->d_name;
        if (fname[0] > ' ' && fname[0] != '.')
        {
            strcpy(str, fname);
            strcat(str, "/.DIR");
            hdr_stamp(fpath, HDR_LINK, &mhdr, path);
            strcpy(mhdr.owner, "SYSOP");
            strcpy(mhdr.title, title);
            mhdr.xmode = MAIL_MULTI;
            rec_add(fpath, &mhdr, sizeof(HDR));
        }
    }
    closedir(dirp);
}


static int
open_mail(
    const char *path,
    const char *title)
{
    int ch;
    char *fname, fpath[256];

    strcpy(fname = fpath, "usr/@");
    fname = (char *) strchr(fname, '@');

    for (ch = 'a'; ch <= 'z'; ch++)
    {
        fname[0] = ch;
        fname[1] = '\0';
        traverse(fpath, path, title);
    }
    return 1;
}

/* mailtoall mode title fpath */

int
main(
    int argc,
    char *argv[])
{
    int mode;

    bshm = attach_shm(BRDSHM_KEY, sizeof(BCACHE));

    if (argc>3)
    {
        mode = atoi(argv[1]);
        switch (mode)
        {
            case 1:
                open_mail(argv[2], argv[3]);
                break;
            case 2:
                to_bm(argv[2], argv[3]);
                break;
        }

    }
    unlink(argv[2]);

    return 0;
}
