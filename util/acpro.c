/*-------------------------------------------------------*/
/* util/acpro.c         ( YZU WindTopBBS Ver 3.00 )      */
/*-------------------------------------------------------*/
/* author : visor.bbs@bbs.yzu.edu.tw                     */
/* target : 建立 [專業討論區] cache                      */
/* create : 95/03/29                                     */
/* update : 97/03/29                                     */
/*-------------------------------------------------------*/

#include "bbs.h"

static BCACHE *bshm;

/* ----------------------------------------------------- */
/* build Class image                                     */
/* ----------------------------------------------------- */


#define CLASS_RUNFILE   "run/class.run"
#define PROFESS_RUNFILE "run/profess.run"

#define CH_MAX  5000


static ClassHeader *chx[CH_MAX];
static int chn;
static BRD *bhead, *btail;


static int
class_parse(
    const char *key)
{
    char *str, fpath[80];
    const char *ptr;
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
            (ClassHeader *) malloc(SIZEOF_FLEX(ClassHeader, count));
        memset(chp->title, 0, CH_TTSIZE);
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
    const void *i, const void *j)
{
    return strcasecmp(bhead[* (const short *)i].brdname, bhead[* (const short *)j].brdname);
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

    chp = (ClassHeader *) malloc(SIZEOF_FLEX(ClassHeader, max));

    for (i = j = 0; i < max; i++, bp++)
    {
        if (bp->brdname[0])
        {
            chp->chno[j++] = i;
        }
    }

    chp->count = j;

    qsort(chp->chno, j, sizeof(short), chno_cmp);

    memset(chp->title, 0, CH_TTSIZE);
    strcpy(chp->title, "Boards");
    chx[chn++] = chp;
}


static void
class_image(
    const char *inifile,
    const char *runfile,
    const char *imgfile)
{
    int i;
    FILE *fp;
    short len, pos[CH_MAX];
    ClassHeader *chp;

    class_sort();
    class_parse(inifile);

    if (chn < 2)  /* lkchu.990106: 尚沒有分類 */
        goto cleanup;

    len = sizeof(short) * (chn + 1);

    for (i = 0; i < chn; i++)
    {
        pos[i] = len;
        len += CH_TTSIZE + chx[i]->count * sizeof(short);
    }
    pos[i++] = len;
    if ((fp = fopen(runfile, "w")))
    {
        fwrite(pos, sizeof(short), i, fp);
        for (i = 0; i < chn; i++)
        {
            chp = chx[i];
            fwrite(chp->title, 1, CH_TTSIZE + chp->count * sizeof(short), fp);
            free(chp);
        }
        fclose(fp);
        rename(runfile, imgfile);
    }
    else
    {
cleanup:
        for (i = 0; i < chn; i++)
        {
            free(chx[i]);
        }
    }
    chn = 0;
}

int
main(void)
{
    setgid(BBSGID);
    setuid(BBSUID);
    chdir(BBSHOME);
    umask(077);
    /* --------------------------------------------------- */
    /* build Class image                                   */
    /* --------------------------------------------------- */
    shm_logger_init(NULL);
    bshm_init(&bshm);
    class_image(CLASS_INIFILE, CLASS_RUNFILE, CLASS_IMGFILE);
    class_image(PROFESS_INIFILE, PROFESS_RUNFILE, PROFESS_IMGFILE);
    exit(0);
}
