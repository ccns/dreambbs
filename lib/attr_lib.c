/* ----------------------------------------------------- */
/* author : thor.bbs@bbs.cs.nthu.edu.tw                  */
/* target : dynamic attribute library for maple bbs      */
/* create : 99/03/09                                     */
/* update :   /  /                                       */
/* ----------------------------------------------------- */

/* Thor.990311: 之所以用暴力而簡單的方式, 是為了考慮讓一般util也能用到此attr
                特別要注意, working directory必須為 BBSHOME */

#include "config.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "dao.h"

#if 0
  int key;
  key < 0 is reserved.
  key & 0xff == 0 is reserved.
  0x000000?? < key < 0x0000ff?? is reserved by maple.org
  sizeof(attr): key & 0xff

  file: $userhome/.ATTRP
#endif

#if 0
#define ATTR_OTHELLO_TOTAL 0x00001004
#define ATTR_FIVE_TOTAL 0x00001104
#define ATTR_OTHELLO_WIN 0x00001404
#define ATTR_FIVE_WIN 0x00001504
#endif

/* return value if exist, else no change (it can set to default value) */
int attr_get(const char *userid, int key, void *value)
{
    char fpath[64];
    int k;
    FILE *fp;

    usr_fpath(fpath, userid, ".ATTR");
    if ((fp = fopen(fpath, "rb")))
    {
        while (fread(&k, sizeof k, 1, fp))
        {
            if (k == key)
            {
                k = fread(value, (size_t) (k & 0xff), 1, fp);
                fclose(fp);
                return k - 1;
            }
            fseek(fp, (unsigned long)(k & 0xff), SEEK_CUR);
        }
        fclose(fp);
    }
    return -1;
}

/* set value if exist, else append new entry */
int attr_put(const char *userid, int key, const void *value)
{
    char fpath[64];
    int k, fd;
    FILE *fp;

    usr_fpath(fpath, userid, ".ATTR");
    if ((fd = open(fpath, O_RDWR | O_CREAT, 0600)) < 0)
        return -1;
    k = 0;
    if ((fp = fdopen(fd, "rb+")))
    {
        f_exlock(fd);
        for (;;)
        {
            if (fread(&k, sizeof k, 1, fp) <= 0)
            {
                fseek(fp, 0, SEEK_CUR);
                if (fwrite(&key, (size_t) sizeof key, 1, fp) <= 0)
                {
                    k = 0;        /* error code */
                    goto close_file;
                }
                break;
            }
            if (k == key)
            {
                fseek(fp, 0, SEEK_CUR);
                /* Thor.990311: for fwrite() at correct pos */
                break;
            }
            fseek(fp, (unsigned long)(k & 0xff), SEEK_CUR);
        }
        k = fwrite(value, (size_t) (key & 0xff), 1, fp);
      close_file:
        f_unlock(fd);
        fclose(fp);
    }
    else
        close(fd);
    return k - 1;
}

/* with file lock scheme */
/* for "int" ONLY */
/* set no default if dflt < 0 */
/* suppose value > 0 */
/* return value if success */
/* failed(-1) if value < 0, and no change to value */
/* no attr(-2) if set no default */
/* file fail or err key(-3) */
int attr_step(const char *userid, int key, int dflt, int step)
{
    char fpath[64];
    int fd, ret;
    int k, value;
    FILE *fp;

    if ((key & 0xff) != sizeof(int))
        return -3;
    usr_fpath(fpath, userid, ".ATTR");
    if ((fd = open(fpath, O_RDWR | O_CREAT, 0600)) < 0)
        return -3;
    if (!(fp = fdopen(fd, "rb+")))
    {
        close(fd);
        return -3;
    }

    ret = 0;
    f_exlock(fd);
    for (;;)
    {
        if (fread(&k, sizeof k, 1, fp) <= 0)
        {
            if (dflt < 0)
            {
                ret = -2;
                goto close_file;
            }
            fseek(fp, 0, SEEK_CUR);
            if (fwrite(&key, sizeof key, 1, fp) <= 0)
            {
                ret = -3;
                goto close_file;
            }
            value = dflt;
            break;
        }
        if (k == key)
        {
            fread(&value, sizeof value, 1, fp);
            fseek(fp, -(off_t) sizeof value, SEEK_CUR);
            /* Thor.990311: for fwrite() at correct pos */
            break;
        }
        fseek(fp, (unsigned long)(k & 0xff), SEEK_CUR);
    }

    value += step;
    if (value < 0)
    {
        ret = -1;
        goto close_file;
    }

    if (fwrite(&value, sizeof value, 1, fp) <= 0)
    {
        ret = -3;
        goto close_file;
    }

    ret = value;

  close_file:
    f_unlock(fd);
    fclose(fp);
    return ret;
}
