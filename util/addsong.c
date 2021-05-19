/*-------------------------------------------------------*/
/* util/addsong.c       ( YZU WindTopBBS Ver 3.00 )      */
/*-------------------------------------------------------*/
/* author : visor.bbs@bbs.yzu.edu.tw                     */
/* target : �C�ӬP���W�[�I�q����                         */
/* create : 2000/07/03                                   */
/* update :                                              */
/*-------------------------------------------------------*/
/* syntax : addsong [perm] [times]                       */
/* NOTICE : add users' request times                     */
/*-------------------------------------------------------*/
#include "bbs.h"

static unsigned int perm;
static int num;

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
    close(fd);

    if (acct.userlevel & perm)
    {
        acct.request += num;
        if (acct.request > 500)
            acct.request = 500;
        if (acct.request < 0)
            acct.request = 0;
        acct_save(&acct);
    }
}

static void
traverse(
    char *fpath)
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
            reaper(fpath, fname);
        }
    }
    closedir(dirp);
}

int
main(
    int argc,
    char *argv[])
{
    int ch;
    char *fname, fpath[256];

    setgid(BBSGID);
    setuid(BBSUID);
    chdir(BBSHOME);

    strcpy(fname = fpath, "usr/@");
    fname = (char *) strchr(fname, '@');

    while (optind < argc)
    {
        switch (getopt(argc, argv, "+" "p:t:"))
        {
        case -1:  // Position arguments
            if (!(optarg = argv[optind++]))
                break;
            if (!perm)
            {
        case 'p':
                if ((perm = strtoll(optarg, NULL, 0)))
                    break;
            }
            else if (!(num > 0))
            {
        case 't':
                if ((num = atoi(optarg)) > 0)
                    break;
            }
            else
                break;
            // Falls through
            // to handle invalid argument values
        default:
            perm = num = 0;  // Invalidate arguments
            optind = argc;  // Ignore remaining arguments
            break;
        }
    }

    if (!(perm && num > 0))
    {
        printf("Usage: %s [-p] perm [-t] times\n", argv[0]);
        printf("(perm != 0, times > 0)\n");
        return -1;
    }

    for (ch = 'a'; ch <= 'z'; ch++)
    {
        fname[0] = ch;
        fname[1] = '\0';
        traverse(fpath);
    }
    for (ch = '0'; ch <= '9'; ch++)
    {
        fname[0] = ch;
        fname[1] = '\0';
        traverse(fpath);
    }
    return 0;
}
