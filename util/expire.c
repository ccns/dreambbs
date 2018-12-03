/*-------------------------------------------------------*/
/* util/expire.c	( NTHU CS MapleBBS Ver 3.00 )	 */
/*-------------------------------------------------------*/
/* target : 自動砍信工具程式				 */
/* create : 95/03/29				 	 */
/* update : 97/03/29				 	 */
/*-------------------------------------------------------*/
/* syntax : expire [day] [max] [min] [board]	 	 */
/* NOTICE : give board to sync				 */
/*-------------------------------------------------------*/

/* Thor.981027: 加上 board時, 可sync 某一board */

#include <strings.h>

#include "bbs.h"

#define	DEF_DAYS	9999
#define	DEF_MAXP	8000
#define	DEF_MINP	7500

#define	EXPIRE_CONF	FN_ETC_EXPIRE_CONF
#define	EXPIRE_LOG	FN_EXPIRE_LOG

typedef struct
{
    char bname[16];		/* board ID */
    int days;			/* expired days */
    int maxp;			/* max post */
    int minp;			/* min post */
}      life;


BRD table[MAXBOARD];

/* ----------------------------------------------------- */
/* synchronize folder & files				 */
/* ----------------------------------------------------- */


typedef struct
{
    time_t chrono;
    char prefix;
    char exotic;
} SyncData;


static SyncData *sync_pool;
static int sync_size, sync_head;


#define	SYNC_DB_SIZE	2048


static int
sync_cmp(
    SyncData *s1, SyncData *s2)
{
    return s1->chrono - s2->chrono;
}


static void
sync_init(
    char *fname,
    time_t uptime)
{
    int ch, prefix;
    time_t chrono;
    char *str, fpath[80];
    struct dirent *de;
    DIR *dirp;

    SyncData *xpool;
    int xsize, xhead;

    if ((xpool = sync_pool))
    {
        xsize = sync_size;
    }
    else
    {
        xpool = (SyncData *) malloc(SYNC_DB_SIZE * sizeof(SyncData));
        xsize = SYNC_DB_SIZE;
    }

    xhead = 0;

    ch = strlen(fname);
    memcpy(fpath, fname, ch);
    fname = fpath + ch;
    *fname++ = '/';
    fname[1] = '\0';

    ch = '0';
    for (;;)
    {
        *fname = ch++;

        if ((dirp = opendir(fpath)))
        {
            while ((de = readdir(dirp)))
            {
                str = de->d_name;
                prefix = *str;
                if (prefix == '.')
                    continue;

                chrono = chrono32(str);

                if (chrono > uptime)
                    continue;

                if (xhead >= xsize)
                {
                    xsize += (xsize >> 1);
                    xpool = (SyncData *) realloc(xpool, xsize * sizeof(SyncData));
                }

                xpool[xhead].chrono = chrono;
                xpool[xhead].prefix = prefix;
                xpool[xhead].exotic = 1;
                xhead++;
            }

            closedir(dirp);
        }

        if (ch == 'W')
            break;

        if (ch == '9' + 1)
            ch = 'A';
    }

    if (xhead > 1)
        qsort(xpool, xhead, sizeof(SyncData), sync_cmp);

    sync_pool = xpool;
    sync_size = xsize;
    sync_head = xhead;
}


static void
sync_check(
    FILE *flog,
    char *fname)
{
    char *str, fpath[80];
    SyncData *xpool, *xtail;
    time_t cc;

    if ((cc = sync_head) <= 0)
        return;

    xpool = sync_pool;
    xtail = xpool + cc;

    sprintf(fpath, "%s/ /", fname);
    str = strchr(fpath, ' ');
    fname = str + 3;

    do
    {
        if (xtail->exotic)
        {
            cc = xtail->chrono;
            fname[-1] = xtail->prefix;
            *str = radix32[cc & 31];
            archiv32(cc, fname);
            unlink(fpath);

            fprintf(flog, "-\t%s\n", fpath);
        }
    } while (--xtail >= xpool);
}


static void
expire(
    FILE *flog,
    life *brd,
    int sync)
{
    HDR hdr;
    struct stat st;
    char fpath[128], fnew[128], index[128], *fname, *bname, *str;
    int done, keep, total, xmode;
    FILE *fpr, *fpw;

    int days, maxp, minp;
    int duetime,now;

    SyncData *xpool=NULL, *xsync;
    int xhead=0;
    time_t uptime = 0;

    days = brd->days;
    maxp = brd->maxp;
    minp = brd->minp;
    bname = brd->bname;

    fprintf(flog, "%s\n", bname);

    if (sync)
    {
        uptime = time(0) - 10 * 60;		/* 太新的不 sync */
        sync_init(bname, uptime);
        xpool = sync_pool;
        xhead = sync_head;
        if (xhead <= 0)
            sync = 0;

        else
            fprintf(flog, "\t%d files to sync\n\n", xhead);
    }

/* cache.091125: 刪除 usies */
    {
        struct tm *ptime;
        time_t now;
        char buf[20];

        time(&now);
        ptime = localtime(&now);

        if (ptime->tm_wday == 0)      /* 每週一砍 usies */
        {
            sprintf(buf, "%s/%s", bname, "usies");
            unlink(buf);
        }
    }

    sprintf(index, "%s/.DIR", bname);
    if (!(fpr = fopen(index, "r")))
    {
        fprintf(flog, "\tError open file: %s\n", index);
        return;
    }

    fpw = f_new(index, fnew);
    if (!fpw)
    {
        fprintf(flog, "\tExclusive lock: %s\n", fnew);
        fclose(fpr);
        return;
    }

    strcpy(fpath, index);
    str = (char *) strrchr(fpath, '.');
    fname = str + 1;
    *fname++ = '/';

    done = 1;
    duetime = time(NULL) - days * 24 * 60 * 60;
    now = time(NULL);

    fstat(fileno(fpr), &st);
    total = st.st_size / sizeof(hdr);

    while (fread(&hdr, sizeof(hdr), 1, fpr) == 1)
    {
        xmode = hdr.xmode;
        if (xmode & (POST_CANCEL | POST_DELETE | POST_MDELETE))
            keep = 0;
        else if ((xmode & POST_EXPIRE) && hdr.expire < now && hdr.expire != 0)
            keep = 0;
        else if (xmode & (POST_MARKED | POST_LOCK) || total <= minp)
            keep = 1;
        else if (hdr.chrono < duetime || total > maxp)
            keep = 0;
        else
            keep = 1;

        if (sync && (hdr.chrono < uptime))
        {
            if ((xsync = (SyncData *) bsearch(&hdr.chrono,
                xpool, xhead, sizeof(SyncData), sync_cmp)))
            {
                xsync->exotic = 0;
            }
            else
            {
                if (keep)
                    keep = 0;
            }
        }

        if (keep)
        {
            if (fwrite(&hdr, sizeof(hdr), 1, fpw) != 1)
            {
                fprintf(flog, "\tError in write DIR.n: %s\n", hdr.xname);
                done = 0;
                sync = 0; /* Thor.990127: 沒作成, 就別砍了吧 */
                break;
            }
        }
        else
        {
            *str = hdr.xname[7];
            strcpy(fname, hdr.xname);
            unlink(fpath);
            fprintf(flog, "\t%s\n", fname);
            total--;
        }
    }
    fclose(fpr);
    fclose(fpw);

    if (done)
    {
        sprintf(fpath, "%s.o", index);
        if (!rename(index, fpath))
        {
            if (rename(fnew, index))
                rename(fpath, index);		/* 換回來 */
        }
    }
    unlink(fnew);

    if (sync)
        sync_check(flog, bname);
}


static int
brdbno(
    char *bname,
    int count)
{
    BRD *brdp, *bend;
    int bno;

    brdp = table;
    bend = brdp + count;
    bno = 0;

    do
    {
        if (!str_cmp(bname, brdp->brdname))
            return bno;

        bno++;
    } while (++brdp < bend);

    return -1;
}

int
main(
    int argc,
    char *argv[])
{
    FILE *fp;
    int number, count;
//  life db, table[MAXBOARD], key;
    life db,key;
    struct dirent *de;
    DIR *dirp;
    char *ptr;
    int fd;
    BRD *brd;

    db.days = ((argc > 1) && (number = atoi(argv[1])) > 0) ? number : DEF_DAYS;
    db.maxp = ((argc > 2) && (number = atoi(argv[2])) > 0) ? number : DEF_MAXP;
    db.minp = ((argc > 3) && (number = atoi(argv[3])) > 0) ? number : DEF_MINP;

    memset(&key,0,sizeof(key));

    /* --------------------------------------------------- */
    /* load expire.ctl					 */
    /* --------------------------------------------------- */

    setgid(BBSGID);
    setuid(BBSUID);
    chdir(BBSHOME);

    fd = open(".BRD",O_RDONLY);
    if(fd < 0)
    {
        fprintf(stderr,"Error open .BRD file\n");
        exit(1);
    }

    count = read(fd,table,sizeof(BRD)*MAXBOARD);
    count/=sizeof(BRD);
    close(fd);

    /* --------------------------------------------------- */
    /* visit all boards					 */
    /* --------------------------------------------------- */

    fp = fopen(EXPIRE_LOG, "w");

    if (chdir("brd") || !(dirp = opendir(".")))
    {
        fprintf(fp, ":Err: unable to visit boards \n"); /* statue.000713 */
        fclose(fp);
        exit(-1);
    }

    number = time(0) / 86400;

    while ((de = readdir(dirp)))
    {
        ptr = de->d_name;
        /* Thor.981027: 加上 board時, 可sync 某一board. 加得很醜, 有空再改 */
        if(argc > 4)
        {
            if(str_cmp(argv[4],ptr))
                continue;
            else
                number=0;
        }

        if (ptr[0] > ' ' && ptr[0] != '.')
        {
            if(count > 0)
            {
                fd = brdbno(ptr,count);
                if(fd >= 0)
                {
                    brd = &table[fd];
                    key.maxp = (brd->expiremax == 0 ? db.maxp : brd->expiremax);
                    key.minp = (brd->expiremin == 0 ? db.minp : brd->expiremin);
                    key.days = (brd->expireday == 0 ? db.days : brd->expireday);
                }
                else
                {
                    key.maxp = db.maxp;
                    key.minp = db.minp;
                    key.days = db.days;
                }
            }
            else
            {
                key.maxp = db.maxp;
                key.minp = db.minp;
                key.days = db.days;
            }
            strcpy(key.bname, ptr);
//          printf("Expire Board: %s\n",ptr);
            expire(fp, &key, !(number & 31));	/* 每隔 32 天 sync 一次 */
            ++number;
        }
    }
    closedir(dirp);

    fclose(fp);
    exit(0);
}
