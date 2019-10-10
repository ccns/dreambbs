#include "dao.h"
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>

static int rm_dir(const char *fpath);

void f_cat(const char *fpath, const char *msg)
{
    int fd;

    if ((fd = open(fpath, O_WRONLY | O_CREAT | O_APPEND, 0600)) >= 0)
    {
        write(fd, msg, strlen(msg));
        close(fd);
    }
}

int f_cp(const char *src, const char *dst, int mode    /* O_EXCL / O_APPEND / O_TRUNC */
    )
{
    int fsrc, fdst, ret;

    ret = -1;

    if ((fsrc = open(src, O_RDONLY)) >= 0)
    {
        if ((fdst = open(dst, O_WRONLY | O_CREAT | mode, 0600)) >= 0)
        {
            char pool[BLK_SIZ];

            char *data = pool;
            do
            {
                ret = read(fsrc, data, BLK_SIZ);
                if (ret <= 0)
                    break;
            }
            while (write(fdst, data, ret) > 0);
            close(fdst);
        }
        close(fsrc);
    }
    return ret;
}


char *f_img(const char *fpath, int *fsize)
{
    int fd, size;
    struct stat st;
    char* data;

    if ((fd = open(fpath, O_RDONLY)) < 0)
        return NULL;

    data = NULL;

    if (!fstat(fd, &st) && S_ISREG(st.st_mode) && (size = st.st_size) > 0
        && (data = (char *)malloc(size)))
    {
        *fsize = size;
        if (read(fd, data, size) != size)
        {
            free(data);
            data = NULL;
        }
    }

    close(fd);
    return data;
}

/* ----------------------------------------------------- */
/* f_ln() : link() cross partition / disk                */
/* ----------------------------------------------------- */

int f_ln(const char *src, const char *dst)
{
    int ret;

    if ((ret = link(src, dst)))
    {
        if (errno != EEXIST)
            ret = f_cp(src, dst, O_EXCL);
    }
    return ret;
}

static struct flock fl = {
    .l_whence = SEEK_SET,
    .l_start = 0,
    .l_len = 0,
};

int f_exlock(int fd)
{
#if 0
    return flock(fd, LOCK_EX);
#endif
    /* Thor.981205: 用 fcntl 取代flock, POSIX標準用法 */
    fl.l_type = F_WRLCK;
    /* Thor.990309: with blocking */
    return fcntl(fd, F_SETLKW /*F_SETLK */, &fl);
}

int f_unlock(int fd)
{
#if 0
    return flock(fd, LOCK_UN);
#endif
    /* Thor.981205: 用 fcntl 取代flock, POSIX標準用法 */
    fl.l_type = F_UNLCK;
    return fcntl(fd, F_SETLKW /*F_SETLK */, &fl);
}

#ifdef MAP_FILE                    /* 44BSD defines this & requires it to mmap files */
#define DAO_MAP       (MAP_SHARED | MAP_FILE)
#else
#define DAO_MAP       (MAP_SHARED)
#endif

char *f_map(const char *fpath, int *fsize)
{
    int fd, size;
    struct stat st;
    char *data;

    if ((fd = open(fpath, O_RDONLY)) < 0)
        return (char *)-1;

    if (fstat(fd, &st) || !S_ISREG(st.st_mode) || (size = st.st_size) <= 0)
    {
        close(fd);
        return (char *)-1;
    }

    data = (char *)mmap(NULL, size, PROT_READ, DAO_MAP, fd, 0);
    close(fd);
    *fsize = size;
    return data;
}


int f_mode(const char *fpath)
{
    struct stat st;

    if (stat(fpath, &st))
        return 0;

    return st.st_mode;
}

int f_mv(const char *src, const char *dst)
{
    int ret;

    if ((ret = rename(src, dst)))
    {
        ret = f_cp(src, dst, O_TRUNC);
        if (!ret)
            unlink(src);
    }
    return ret;
}

/* ----------------------------------------------------- */
/* exclusively create file [*.n]                         */
/* ----------------------------------------------------- */

FILE *f_new(const char *fold, char *fnew)
{
    int fd, try;

    try = 0;
    str_cat(fnew, fold, ".n");

    for (;;)
    {
        fd = open(fnew, O_WRONLY | O_CREAT | O_EXCL, 0600);

        if (fd >= 0)
            return fdopen(fd, "w");

        if (errno != EEXIST)
            break;

        if (!try++)
        {
            struct stat st;

            if (stat(fnew, &st) < 0)
                break;
            if (st.st_mtime < time(NULL) - 20 * 60)    /* 假設 20 分鐘內應該處理完 */
                unlink(fnew);
        }
        else
        {
            if (try > 24)        /* 等待 120 秒鐘 */
                break;
            sleep(5);
        }
    }
    return NULL;
}

int f_open(const char *fpath)
{
    int fd;
    struct stat st;

    if ((fd = open(fpath, O_RDONLY)) >= 0)
    {
        if (fstat(fd, &st) || st.st_size <= 0 || !S_ISREG(st.st_mode))
        {
            close(fd);
            unlink(fpath);
            fd = -1;
        }
    }

    return fd;
}

/* ----------------------------------------------------- */
/* file structure : set file path for boards/user home   */
/* ----------------------------------------------------- */

static void mak_fpath(char *str, const char *key, const char *name)
{
    int cc;

    cc = '/';
    for (;;)
    {
        *str = cc;
        if (!cc)
            break;
        str++;
        cc = *key++;

#if 0
        if (cc >= 'A' && cc <= 'Z')
            cc |= 0x20;
#endif
    }

    if (name)
    {
        *str++ = '/';
        strcpy(str, name);
    }
}


void brd_fpath(char *fpath, const char *board, const char *fname)
{
    *fpath++ = 'b';
    *fpath++ = 'r';
    *fpath++ = 'd';
    mak_fpath(fpath, board, fname);
}


void gem_fpath(char *fpath, const char *board, const char *fname)
{
    *fpath++ = 'g';
    *fpath++ = 'e';
    *fpath++ = 'm';
    *fpath++ = '/';
    *fpath++ = 'b';
    *fpath++ = 'r';
    *fpath++ = 'd';
    mak_fpath(fpath, board, fname);
}


void usr_fpath(char *fpath, const char *user, const char *fname)
{
#if 0
    char buf[16];
#endif

#define IDLEN    12                /* Length of board / user id, copy from  struct.h */

    char buf[IDLEN + 1];

    *fpath++ = 'u';
    *fpath++ = 's';
    *fpath++ = 'r';
    *fpath++ = '/';

#if 0
    str_lower(buf, user);        /* lower case */
#endif
    /* Thor.981027: 防止 buffer overflow, 雖然 SunOS 4.1.x上無此情況,
       以後再想好的改法 */
    str_ncpy(buf, user, sizeof(buf));
    str_lower(buf, buf);

    *fpath++ = *buf;
    mak_fpath(fpath, buf, fname);
}


int f_rm(const char *fpath)
{
    struct stat st;

    if (stat(fpath, &st))
        return -1;

    if (!S_ISDIR(st.st_mode))
        return unlink(fpath);

    return rm_dir(fpath);
}

static int rm_dir(const char *fpath)
{
    struct stat st;
    DIR *dirp;
    struct dirent *de;
    char buf[256], *fname;

    if (!(dirp = opendir(fpath)))
        return -1;

    for (fname = buf; (*fname = *fpath); fname++, fpath++)
        ;

    *fname++ = '/';

    readdir(dirp);
    readdir(dirp);

    while ((de = readdir(dirp)))
    {
        fpath = de->d_name;
        if (*fpath)
        {
            strcpy(fname, fpath);
            if (!stat(buf, &st))
            {
                if (S_ISDIR(st.st_mode))
                    rm_dir(buf);
                else
                    unlink(buf);
            }
        }
    }
    closedir(dirp);

    *--fname = '\0';
    return rmdir(buf);
}

void f_suck(FILE * fp, const char *fpath)
{
    int fd;

    if ((fd = open(fpath, O_RDONLY)) >= 0)
    {
        char pool[BLK_SIZ];
        int size;

        char *data = pool;
        while ((size = read(fd, data, BLK_SIZ)) > 0)
        {
            fwrite(data, size, 1, fp);
        }
        close(fd);
    }
}

/* ----------------------------------------------------- */
/* make directory hierarchy [0-9A-V] : 32-way interleave */
/* ----------------------------------------------------- */

void mak_dirs(char *fpath)
{
    char *fname;
    int ch;

    if (mkdir(fpath, 0755))
        return;

    fname = fpath;
    while (*++fname);
    *fname++ = '/';
    fname[1] = '\0';

    ch = '0';
    for (;;)
    {
        *fname = ch++;
        mkdir(fpath, 0755);
        if (ch == 'W')
            break;
        if (ch == '9' + 1)
            ch = '@';            /* @ : for special purpose */
    }

    fname[-1] = '\0';
}
