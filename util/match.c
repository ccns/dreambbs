/*-------------------------------------------------------*/
/* util/match.c         ( YZU WindTopBBS Ver 3.00 )	 */
/*-------------------------------------------------------*/
/* author : visor.bbs@bbs.yzu.edu.tw			 */
/* target : ID, 真實姓名, Email對照表                    */
/* create : 2000/06/08                                   */
/* update :                                              */
/*-------------------------------------------------------*/

#include "bbs.h"

FILE *flog;
static int funo;

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

    close(fd);
    fprintf(flog,"%-13s %-20s %-40.40s\n",acct.userid,acct.realname,acct.email);

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
    flog = fopen(FN_MATCH_NEW,"w");

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
    fprintf(flog,"The end.\n");
    fclose(flog);
    rename(FN_MATCH_NEW,FN_MATCH_LOG);
    if (argc > 1)
    {
        sprintf(fpath,"mail %s.bbs@" MYHOSTNAME " < " FN_MATCH_MAIL,argv[1]);
        system(fpath);
    }
    else
        system("mail bbs@" MYHOSTNAME " < " FN_MATCH_MAIL);
    return 0;
}
