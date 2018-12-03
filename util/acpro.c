/*-------------------------------------------------------*/
/* util/acpro.c         ( YZU WindTopBBS Ver 3.00 )      */
/*-------------------------------------------------------*/
/* author : visor.bbs@bbs.yzu.edu.tw                     */
/* target : 建立 [專業討論區] cache                      */
/* create : 95/03/29                                     */
/* update : 97/03/29                                     */
/*-------------------------------------------------------*/

#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "bbs.h"

static BCACHE *bshm;

static void
attach_err(
    int shmkey,
    char *name)
{
    fprintf(stderr, "[%s error] key = %x\n", name, shmkey);
    exit(1);
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
        if (shmid < 0)
            attach_err(shmkey, "shmget");
    }
    else
    {
        shmsize = 0;
    }

    shmptr = (void *) shmat(shmid, NULL, 0);
    if (shmptr == (void *) -1)
        attach_err(shmkey, "shmat");

    if (shmsize)
        memset(shmptr, 0, shmsize);

    return shmptr;
}

void
bshm_init(void)
{
    register BCACHE *xshm;
    register time_t *uptime;
    register int n, turn;

    turn = 0;
    xshm = bshm;
    if (xshm == NULL)
    {
        bshm = xshm = attach_shm(BRDSHM_KEY, sizeof(BCACHE));
    }

    uptime = &(xshm->uptime);

    for (;;)
    {
        n = *uptime;
        if (n > 0)
            return;

        if (n < 0)
        {
            if (++turn < 30)
            {
                sleep(2);
                continue;
            }
        }

        *uptime = -1;

        if ((n = open(FN_BRD, O_RDONLY)) >= 0)
        {
            xshm->number =
                read(n, xshm->bcache, MAXBOARD * sizeof(BRD)) / sizeof(BRD);
            close(n);
        }

        /* 等所有 boards 資料更新後再設定 uptime */

        time(uptime);
        fprintf(stderr, "[account]\tCACHE\treload bcache");

        return;
    }
}

/* ----------------------------------------------------- */
/* build Class image                                     */
/* ----------------------------------------------------- */


#define PROFESS_RUNFILE "run/profess.run"

#define CH_MAX  100


static ClassHeader *chx[CH_MAX];
static int chn;
static BRD *bhead, *btail;


static int
class_parse(
    char *key)
{
    char *str, *ptr, fpath[80];
    ClassHeader *chp;
    HDR hdr;
    int i, len, count;
    FILE *fp;

    strcpy(fpath, "gem/@/@");
    str = fpath + sizeof("gem/@/@") - 1;
    for (ptr = key;; ptr++)
    {
        i = *ptr;
        if (i == '/')
            i = 0;
        *str = i;
        if (!i)
            break;
        str++;
    }

    len = ptr - key;

    /* search classes */

    for (i = 1; i < chn; i++)
    {
        str = chx[i]->title;
        if (str[len] == '/' && !memcmp(key, str, len))
            return CH_END - i;
    }

    /* build classes */

    if ((fp = fopen(fpath, "r")))
    {
        int ans;
        struct stat st;

        if (fstat(fileno(fp), &st) || (count = st.st_size / sizeof(HDR)) <= 0)
        {
            fclose(fp);
            return CH_END;
        }

        chx[chn++] = chp =
            (ClassHeader *) malloc(sizeof(ClassHeader) + count * sizeof(short));
        memset(chp->title, 0, CH_TTLEN);
        strcpy(chp->title, key);

        ans = chn;
        count = 0;

        while (fread(&hdr, sizeof(hdr), 1, fp) == 1)
        {
            if (hdr.xmode & GEM_BOARD)
            {
                BRD *bp;

                i = -1;
                str = hdr.xname;
                bp = bhead;

                for (;;)
                {
                    i++;
                    if (!strcasecmp(str, bp->brdname))
                        break;

                    if (++bp >= btail)
                    {
                        i = -1;
                        break;
                    }
                }

                if (i < 0)
                    continue;
            }
            else
            {
                i = class_parse(hdr.title);

                if (i == CH_END)
                    continue;
            }

            chp->chno[count++] = i;
        }

        fclose(fp);

        chp->count = count;
        return -ans;
    }

    return CH_END;
}


static int
chno_cmp(
    short *i, short *j)
{
    return strcasecmp(bhead[*i].brdname, bhead[*j].brdname);
}


static void
class_sort(void)
{
    ClassHeader *chp;
    int i, j, max;
    BRD *bp;

    max = bshm->number;
    bhead = bp = bshm->bcache;
    btail = bp + max;

    chp = (ClassHeader *) malloc(sizeof(ClassHeader) + max * sizeof(short));

    for (i = j = 0; i < max; i++, bp++)
    {
        if (bp->brdname)
        {
            chp->chno[j++] = i;
        }
    }

    chp->count = j;

    qsort(chp->chno, j, sizeof(short), chno_cmp);

    memset(chp->title, 0, CH_TTLEN);
    strcpy(chp->title, "Boards");
    chx[chn++] = chp;
}


static void
profess_image(void)
{
    int i;
    FILE *fp;
    short len, pos[CH_MAX];
    ClassHeader *chp;

    class_sort();
    class_parse(PROFESS_INIFILE);

    if (chn < 2)  /* lkchu.990106: 尚沒有分類 */
        return;

    len = sizeof(short) * (chn + 1);

    for (i = 0; i < chn; i++)
    {
        pos[i] = len;
        len += CH_TTLEN + chx[i]->count * sizeof(short);
    }
    pos[i++] = len;
    if ((fp = fopen(PROFESS_RUNFILE, "w")))
    {
        fwrite(pos, sizeof(short), i, fp);
        for (i = 0; i < chn; i++)
        {
            chp = chx[i];
            fwrite(chp->title, 1, CH_TTLEN + chp->count * sizeof(short), fp);
            free(chp);
        }
        fclose(fp);
        rename(PROFESS_RUNFILE, PROFESS_IMGFILE);
    }
}

int
main(void)
{
    chdir(BBSHOME);
    umask(077);
    /* --------------------------------------------------- */
    /* build Class image                                   */
    /* --------------------------------------------------- */
    bshm_init();
    profess_image();
    exit(0);
}
