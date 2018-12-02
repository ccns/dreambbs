/*-------------------------------------------------------*/
/* util/checkemail.c      ( YZU WindTopBBS Ver 3.00 )    */
/*-------------------------------------------------------*/
/* author : visor.bbs@bbs.yzu.edu.tw                     */
/* target : 檢查是否為核可之 Email                       */
/* create : 95/03/29                                     */
/* update : 97/03/29                                     */
/*-------------------------------------------------------*/
/* syntax : checkemail                                   */
/*-------------------------------------------------------*/

#include "bbs.h"

#define MAX_AC	(60000)

typedef struct
{
    char email[60];
    int num;
}	MAP;
MAP map[MAX_AC];


static int funo;
int total;

static int
check_in(
    char *email)
{
    int i;
    for (i=0; i<MAX_AC; i++)
        if (!strcmp(map[i].email, email))
        {
            map[i].num++;
            return 1;
        }
    return 0;
}


static void
reaper(
    char *fpath,
    char *lowid)
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
    if (acct.userlevel & PERM_VALID)
    {
        str_lower(buf, acct.email);
        if (!check_in(buf))
        {
            strcpy(map[total].email, buf);
            map[total].num = 1;
            total++;
        }
    }
    close(fd);
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
    char *fname, fpath[256], bpath[256];
    int i, fd;

    fd = open(FN_ETC_EMAILADDR_ACL".new", O_WRONLY | O_CREAT | O_TRUNC, 0600);

    memset(map, 0, MAX_AC * sizeof(MAP));
    total = 0;
    funo = open(".USR", O_RDWR | O_CREAT, 0600);

    if (funo < 0)
        return 0;

    strcpy(fname = fpath, "usr/@");
    fname = (char *) strchr(fname, '@');

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
    close(funo);


    printf("total user %d\n", total);
    for (i=0; i<total; i++)
    {
        write(fd, &map[i], sizeof(MAP));
        printf("%-60s# %d\n", map[i].email, map[i].num);
    }
    close(fd);
    sprintf(bpath, FN_ETC_EMAILADDR_ACL);
    rename(FN_ETC_EMAILADDR_ACL".new", bpath);
    if (argc > 1)
    {
        sprintf(fpath, "mail %s.bbs@"MYHOSTNAME" < " FN_CHECKMAIL_MAIL, argv[1]);
        system("mail SYSOP.bbs@" MYHOSTNAME " < " FN_CHECKMAIL_MAIL);
        system("mail Dream_log.brd@" MYHOSTNAME " < " FN_CHECKMAIL_MAIL);
        system(fpath);
    }
    else
        system("mail bbs@" MYHOSTNAME " < " FN_CHECKMAIL_MAIL);
        system("mail SYSOP.bbs@" MYHOSTNAME " < " FN_CHECKMAIL_MAIL);
        system("mail Dream_log.brd@" MYHOSTNAME " < " FN_CHECKMAIL_MAIL);
    return 0;
}
