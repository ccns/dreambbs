/*-------------------------------------------------------*/
/* mailtoall.c          ( YZU WindTopBBS Ver 3.00 )      */
/*-------------------------------------------------------*/
/* author : visor.bbs@bbs.yzu.edu.tw                     */
/* target : �H�H�������ϥΪ̩ΩҦ����D                   */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/


#include "bbs.h"

BCACHE *bshm;



typedef struct
{
    char id[IDLEN+1];
    char brd[IDLEN+1];
    int check;
} BM;


GCC_PURE static int
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
        hdr_stamp(buf, HDR_LINK, &mhdr, (char *)fpath);
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
    do                          /* �ܤ֦�sysop�@�� */
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
            hdr_stamp(fpath, HDR_LINK, &mhdr, (char *)path);
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
    char *path = NULL, *title = NULL;

    setgid(BBSGID);
    setuid(BBSUID);
    chdir(BBSHOME);

    shm_logger_init(NULL);
    bshm_init(&bshm);

    mode = (argc > 1) ? atoi(argv[1]) : 0;
    optind++;
    while (optind < argc)
    {
        switch (getopt(argc, argv, "+" "f:t:"))
        {
        case -1:  // Position arguments
            if (!(optarg = argv[optind++]))
                break;
            if (!path)
        case 'f':
                path = optarg;
            else if (!title)
        case 't':
                title = optarg;
            break;

        default:
            path = title = NULL;  // Invalidate arguments
            optind = argc;  // Ignore remaining arguments
            break;
        }
    }

    if (!(path && title))
        mode = 0;

    switch (mode)
    {
    case 1:
        open_mail(path, title);
        break;
    case 2:
        to_bm(path, title);
        break;

    default:
        fprintf(stderr, "Usage: %s {1|2} [-f] <path> [-t] <title>\n", argv[0]);
        return 2;
    }
    unlink(path);

    return 0;
}
