/*-------------------------------------------------------*/
/* util/tranBRD.c       ( YZU CSE WindTop Ver 3.xx )     */
/*-------------------------------------------------------*/
/* target : 新舊版 BRD 轉換                              */
/* create :                                              */
/* update :                                              */
/*-------------------------------------------------------*/
/* Usage  : tranBRD                                      */
/*-------------------------------------------------------*/

#include "bbs.h"


/* ----------------------------------------------------- */
/* BOARDS struct : 128 bytes                             */
/* ----------------------------------------------------- */

#define EXPIRE_CONF     FN_ETC_EXPIRE_CONF

typedef struct BoardHeaderOld
{
    char brdname[IDLEN + 1];      /* board ID */
    char title[BTLEN + 1];
    char color;
    char class[5];
    char BM[BMLEN + 1];           /* BMs' uid, token '/' */

    unsigned char bvote;          /* 共有幾項投票舉行中 */

    time_t bstamp;                /* 建立看板的時間, unique */
    unsigned int readlevel;       /* 閱讀文章的權限 */
    unsigned int postlevel;       /* 發表文章的權限 */
    unsigned int battr;           /* 看板屬性 */
    time_t btime;                 /* .DIR 的 st_mtime */
    int bpost;                    /* 共有幾篇 post */
    time_t blast;                 /* 最後一篇 post 的時間 */
}           BRDOLD;


typedef struct
{
    char bname[16];               /* board ID */
    int days;                     /* expired days */
    int maxp;                     /* max post */
    int minp;                     /* min post */
}      life;

int
main(
    int argc,
    char *argv[])
{
    int inf, count, outf, num, number;
    life table[MAXBOARD], *key;
    char *ptr, *bname, buf[256];
    BRDOLD old;
    BRD brd;
    FILE *fp;

    setgid(BBSGID);
    setuid(BBSUID);
    chdir(BBSHOME);

    if (argc < 2)
    {
        printf("Usage:\t%s <.BRD>\n", argv[0]);
        exit(1);
    }


    count = 0;
    if ((fp = fopen(EXPIRE_CONF, "r")))
    {
        while (fgets(buf, sizeof(buf), fp) && buf[0])
        {
            if (buf[0] < '0')
                continue;

            bname = (char *) strtok(buf, " \t\r\n");
            if (bname && *bname)
            {
                ptr = (char *) strtok(NULL, " \t\r\n");
                if (ptr && (number = atoi(ptr)) > 0)
                {
                    key = &(table[count++]);
                    strcpy(key->bname, bname);
                    key->days = number;
                    key->maxp = 0;
                    key->minp = 0;

                    ptr = (char *) strtok(NULL, " \t\r\n");
                    if (ptr && (number = atoi(ptr)) > 0)
                    {
                        key->maxp = number;

                        ptr = (char *) strtok(NULL, " \t\r\n");
                        if (ptr && (number = atoi(ptr)) > 0)
                        {
                            key->minp = number;
                        }
                    }
                }
            }
        }
        fclose(fp);
    }

    if (count > 1)
    {
        qsort(table, count, sizeof(life), (int (*)(const void *lhs, const void *rhs))strcasecmp);
    }


    inf = open(argv[1], O_RDONLY);
    outf = open(".BRD.new", O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (inf == -1)
    {
        printf("error open old file\n");
        exit(1);
    }

    if (outf == -1)
    {
        printf("error open new file\n");
        exit(1);
    }

    num = 0;
    while (read(inf, &old, sizeof(BRDOLD)) == sizeof(BRDOLD))
    {
        num++;
        memset(&brd, 0, sizeof(BRD));
        memcpy(&brd, &old, sizeof(BRDOLD));

        key = (life *) bsearch(brd.brdname, table, count, sizeof(life), (int (*)(const void *lhs, const void *rhs))strcasecmp);
        if (key)
        {
            brd.expiremax = key->maxp;
            brd.expiremin = key->minp;
            brd.expireday = key->days;
        }

        printf("%04d %-13s %-5s %-40s %-20s %5u %5u %5u\n", num, brd.brdname, brd.class, brd.title, brd.BM, brd.expiremax, brd.expiremin, brd.expireday);

        write(outf, &brd, sizeof(BRD));
    }
    close(inf);
    close(outf);
    return 0;
}
