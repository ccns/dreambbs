/*-------------------------------------------------------*/
/* util/makeuserno.c    ( YZU WindTopBBS Ver 3.00 )      */
/*-------------------------------------------------------*/
/* author : visor.bbs@bbs.yzu.edu.tw                     */
/* target : 自動重整 userno 重複的程式                   */
/* create : 2000/06/08                                   */
/* update :                                              */
/*-------------------------------------------------------*/
/* syntax : makeuserno                                   */
/* NOTICE : 轉換部分[水桶名單，好友名單，熱訊紀錄]       */
/*          好友上站通知無法轉換                         */
/*-------------------------------------------------------*/

#include "bbs.h"

#undef  FAKE_IO

typedef struct
{
    char userid[IDLEN+1];
}       MAP;
MAP map[60000];


int total;
static void pal_sync(const char *fpath);

static int
int_cmp(
    const void *a,
    const void *b)
{
    return *(const int *)a - *(const int *)b;
}

static void
bimage(
    const char *brd)
{
    int fd;
    char fpath[128];

    brd_fpath(fpath, brd, FN_PAL);
    if ((fd = open(fpath, O_RDONLY)) >= 0)
    {
        struct stat st;
        PAL *pal, *up;
        int count;

        fstat(fd, &st);
        if ((pal = (PAL *) malloc(count = st.st_size)))
        {
            count = read(fd, pal, count) / sizeof(PAL);
            if (count > 0)
            {
                int *userno, *ubase;
                int c = count;

                ubase = userno = (int *) malloc(count * sizeof(int));
                up = pal;

                do
                {
                    *userno++ = (up->ftype == PAL_BAD) ? -(up->userno) : up->userno;
                    up++;
                } while (--c);

                if (count > 1)
                    xsort(ubase, count, sizeof(int), (int (*)(const void *lhs, const void *rhs))int_cmp);

                brd_fpath(fpath, brd, "fimage");
#ifndef FAKE_IO
                if ((count = open(fpath, O_WRONLY | O_CREAT | O_TRUNC, 0600)) >= 0)
                {
                    write(count, ubase, (char *) userno - (char *) ubase);
                    close(count);
                }
#endif
                free(ubase);
                printf("BRD : %s  OK~\n", brd);
            }
            else
            {
#ifndef FAKE_IO
                brd_fpath(fpath, brd, "fimage");
                unlink(fpath);
#endif
                printf("BRD : %s  UNLINK~\n", brd);
            }
            free(pal);
        }
        close(fd);
    }
    else
    {
#ifndef FAKE_IO
        brd_fpath(fpath, brd, "fimage");
        unlink(fpath);
#endif
        printf("BRD : %s  UNLINK~\n", brd);
    }
}

void
resetbrd(void)
{
    struct dirent *de;
    DIR *dirp;
    char buf[128], *ptr;

    if (!(dirp = opendir(BBSHOME"/brd")))
    {
        return;
    }

    while ((de = readdir(dirp)))
    {
        ptr = de->d_name;
        if (ptr[0] > ' ' && ptr[0] != '.')
        {
            sprintf(buf, BBSHOME "/brd/%s/friend", ptr);
            pal_sync(buf);
            bimage(ptr);
        }
    }
    closedir(dirp);
}

static int
finduserno(
    const char *userid)
{
    int i;
    for (i=1; i<=total; i++)
    {
        if (!strcmp(map[i].userid, userid))
            return i;
    }
    return 0;
}

void
bmw_sync(
    const char *userid,
    int userno)
{
    int fd;
    char fpath[128];

    usr_fpath(fpath, userid, FN_BMW);
    fd = f_open(fpath);
    if (fd >= 0)
    {
        FILE *fout;
        char folder[80], buf[128];
        HDR fhdr;

        usr_fpath(folder, userid, FN_DIR);
#ifndef FAKE_IO
        if ((fout = fdopen(hdr_stamp(folder, 0, &fhdr, buf), "w")))
        {
            BMW bmw;

            while (read(fd, &bmw, sizeof(BMW)) == sizeof(BMW))
            {
                struct tm *ptime = localtime(&bmw.btime);

                fprintf(fout, "%s%s(%02d:%02d)：%s\x1b[m\n",
                    bmw.sender == userno ? "☆" : "\x1b[32m★",
                    bmw.userid, ptime->tm_hour, ptime->tm_min, bmw.msg);
            }
            fclose(fout);
        }
        close(fd);

        fhdr.xmode = MAIL_READ | MAIL_NOREPLY;
        strcpy(fhdr.title, "[備 忘 錄] 熱訊紀錄");
        strcpy(fhdr.owner, userid);
        rec_add(folder, &fhdr, sizeof(fhdr));
        unlink(fpath);
#endif
    }
}

void
pal_sync(
    const char *fpath)
{
    int fd, size=0;
    struct stat st;

    if ((fd = open(fpath, O_RDWR, 0600)) < 0)
        return;

    if (!fstat(fd, &st) && (size = st.st_size) > 0)
    {
        PAL *pbase, *phead, *ptail;
        int userno;

        pbase = phead = (PAL *) malloc(size);
        size = read(fd, pbase, size);
        if (size >= sizeof(PAL))
        {
            ptail = (PAL *) ((char *) pbase + size);
            while (phead < ptail)
            {
                if ((userno = finduserno(phead->userid)))
                {
                    phead->userno = userno;
                    phead++;
                    continue;
                }

                ptail--;
                if (phead >= ptail)
                    break;
                memcpy(phead, ptail, sizeof(PAL));
            }

            size = (char *) ptail - (char *) pbase;
            if (size > 0)
            {
                if (size > sizeof(PAL))
                    xsort(pbase, size / sizeof(PAL), sizeof(PAL), (int (*)(const void *lhs, const void *rhs))str_casecmp);
#ifndef FAKE_IO
                lseek(fd, 0, SEEK_SET);
                write(fd, pbase, size);
                ftruncate(fd, size);
#endif
                printf("PATH : %s  PAL : %zu\n", fpath, size/sizeof(PAL));
            }
        }
        free(pbase);
    }
    close(fd);

#ifndef FAKE_IO
    if (size <= 0)
        unlink(fpath);
#endif
}

static void
reaper(
    const char *fpath,
    const char *lowid)
{
    int fd;

    char buf[256];
    ACCT acct;

    sprintf(buf, "%s/.ACCT", fpath);
    fd = open(buf, O_RDWR, 0);
    if (fd < 0)
        return;

    if (read(fd, &acct, sizeof(acct))!=sizeof(acct))
    {
        close(fd);
        return;
    }
    bmw_sync(acct.userid, acct.userno);

    acct.userno = total;
    strcpy(map[total++].userid, acct.userid);

#ifndef FAKE_IO
    lseek(fd, 0, SEEK_SET);
    write(fd, &acct, sizeof(ACCT));
#endif
    printf("%-14s : %d\n", acct.userid, acct.userno);

    close(fd);
}

static void
optimize(
    const char *fpath,
    const char *lowid)
{

    char buf[256];
    sprintf(buf, "%s/friend", fpath);
    pal_sync(buf);
#ifndef FAKE_IO
    sprintf(buf, "%s/frienz", fpath);
    unlink(buf);
    sprintf(buf, "%s/benz", fpath);
    unlink(buf);
    sprintf(buf, "%s/aloha", fpath);
    unlink(buf);
#endif
}


static void
traverse(
    char *fpath,
    int mode)
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
            if (mode == 1)
                reaper(fpath, fname);
            else if (mode == 2)
                optimize(fpath, fname);
        }
    }
    closedir(dirp);
}

int
main(void)
{
    int ch, mode;
    char *fname, fpath[256];

    setgid(BBSGID);
    setuid(BBSUID);
    chdir(BBSHOME);

    total = 1;

    strcpy(fname = fpath, "usr/@");
    fname = (char *) strchr(fname, '@');

    for (mode = 1; mode <= 2; mode++)
    {
        for (ch = 'a'; ch <= 'z'; ch++)
        {
            fname[0] = ch;
            fname[1] = '\0';
            traverse(fpath, mode);
        }
        for (ch = '0'; ch <= '9'; ch++)
        {
            fname[0] = ch;
            fname[1] = '\0';
            traverse(fpath, mode);
        }
        if (mode == 2)
            break;
        total--;
    }
    resetbrd();
#ifndef FAKE_IO
    {
        SCHEMA slot;
        int fd, num;

        fd = open(".USR.new", O_CREAT | O_TRUNC | O_WRONLY, 0600);
        for (num = 1; num <= total; num++)
        {
            strncpy(slot.userid, map[num].userid, IDLEN);
            time(&slot.uptime);
            write(fd, &slot, sizeof(SCHEMA));
        }
        close(fd);
    }
#endif

    printf("total user %d\n", total);
    return 0;
}
