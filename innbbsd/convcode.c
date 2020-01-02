/* ----------------------------------------------------- */
/* 簡繁體漢字轉換                                        */
/* ----------------------------------------------------- */
/* create :   /  /                                       */
/* update : 03/05/16                                     */
/* author : kcn@cic.tsinghua.edu.cn                      */
/* modify : itoc.bbs@bbs.tnfsh.tn.edu.tw                 */
/* ----------------------------------------------------- */


#include "innbbsconf.h"
#include "bbslib.h"


#define BtoG_count      13973
#define GtoB_count      7614

#define BtoG_bad1       0xa1
#define BtoG_bad2       0xf5
#define GtoB_bad1       0xa1
#define GtoB_bad2       0xbc


static unsigned char *BtoG = NULL;
static unsigned char *GtoB = NULL;


static void
conv_init(void)
{
    int fd, size, BGsize, GBsize;
    struct stat st;

    if (BtoG != NULL)
        return;

    BGsize = 2 * BtoG_count;    /* 每個漢字 2-byte */
    GBsize = 2 * GtoB_count;
    BtoG = (unsigned char *) malloc(BGsize + GBsize);
    GtoB = BtoG + BGsize;

    if ((fd = open("etc/b2g_table", O_RDONLY)) >= 0)
    {
        fstat(fd, &st);
        size = BGsize <= st.st_size ? BGsize : st.st_size;
        read(fd, BtoG, size);
        close(fd);
    }
    if ((fd = open("etc/g2b_table", O_RDONLY)) >= 0)
    {
        fstat(fd, &st);
        size = GBsize <= st.st_size ? GBsize : st.st_size;
        read(fd, GtoB, size);
        close(fd);
    }
}


#define c1      (unsigned char)(src[0])
#define c2      (unsigned char)(src[1])


static void
b2g(
    const char *src, char *dst)
{
    int i;

    if ((c1 >= 0xa1) && (c1 <= 0xf9))
    {
        if ((c2 >= 0x40) && (c2 <= 0x7e))
        {
            i = ((c1 - 0xa1) * 157 + (c2 - 0x40)) * 2;
            dst[0] = BtoG[i++];
            dst[1] = BtoG[i];
            return;
        }
        else if ((c2 >= 0xa1) && (c2 <= 0xfe))
        {
            i = ((c1 - 0xa1) * 157 + (c2 - 0xa1) + 63) * 2;
            dst[0] = BtoG[i++];
            dst[1] = BtoG[i];
            return;
        }
    }
    dst[0] = BtoG_bad1;
    dst[1] = BtoG_bad2;
}


static void
g2b(
    const char *src, char *dst)
{
    int i;

    if ((c2 >= 0xa1) && (c2 <= 0xfe))
    {
        if ((c1 >= 0xa1) && (c1 <= 0xa9))
        {
            i = ((c1 - 0xa1) * 94 + (c2 - 0xa1)) * 2;
            dst[0] = GtoB[i++];
            dst[1] = GtoB[i];
            return;
        }
        else if ((c1 >= 0xb0) && (c1 <= 0xf7))
        {
            i = ((c1 - 0xb0 + 9) * 94 + (c2 - 0xa1)) * 2;
            dst[0] = GtoB[i++];
            dst[1] = GtoB[i];
            return;
        }
    }
    dst[0] = GtoB_bad1;
    dst[1] = GtoB_bad2;
}


static char *
hzconvert(
    const char *src,            /* source char buffer pointer */
    char *dst,                  /* destination char buffer pointer */
    void (*dbcvrt) (const char *src, char *dst))      /* 漢字 2-byte conversion function */
{
    int len;
    char *p;
    const char *end;

    conv_init();

    p = dst;
    len = strlen(src);
    end = src + len;
    while (src < end)
    {
        if (IS_DBCS_HI((unsigned char)*src))   /* hi-bit on 表示是漢字 */
        {
            dbcvrt(src, p);
            src += 2;           /* 一次轉二碼 */
            p += 2;
        }
        else
        {
            /* *p = *src; */    /* 不需要，因為在 b52gb()、gb2b5() 的應用裡 src == dst */
            src++;
            p++;
        }
    }
    /* dst[len] = '\0'; */      /* 不需要，因為在 b52gb()、gb2b5() 的應用裡 src == dst */

    return dst;
}


void
b52gb(
    char *str)
{
    hzconvert(str, str, b2g);
}


void
gb2b5(
    char *str)
{
    hzconvert(str, str, g2b);
}
