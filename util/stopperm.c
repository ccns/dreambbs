/*-------------------------------------------------------*/
/* stopperm.c   ( YZU WindTopBBS Ver 3.00 )              */
/*-------------------------------------------------------*/
/* target : 連坐處罰程式                                 */
/* create : 95/03/29                                     */
/* update : 96/04/05                                     */
/*-------------------------------------------------------*/


#undef  ADMIN_C
#include "bbs.h"

static int funo;
static const char *kmail;
static int total;
static FILE *flog;

static void
reaper(
    const char *fpath,
    const char *lowid)
{
    int fd;

    char buf[256];
    ACCT acct;

    sprintf(buf, "%s/.ACCT", fpath);
    fd = open(buf, O_RDONLY, 0);
    if (fd < 0)
        return;

    if (read(fd, &acct, sizeof(acct))!=sizeof(acct))
    {
        close(fd);
        return;
    }
    close(fd);

    if (!strcmp(acct.email, kmail))
    {
        fprintf(flog, "%-*s\n", IDLEN, acct.userid);
        total++;
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

static int
same_mail2(
    const char *mail,
    const char *file)
{
    int ch;
    char *fname, fpath[256];
    kmail = mail;
    total = 0;

    sprintf(fpath, "%s.%s", FN_SAMEEMAIL_LOG, file);

    flog = fopen(fpath, "w");
    if (flog == NULL)
        return 0;

    funo = open(FN_SCHEMA, O_RDWR | O_CREAT, 0600);

    if (funo < 0)
    {
        fclose(flog);
        return 0;
    }

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
    fclose(flog);
    close(funo);
    return total;
}


static void
setup(
    const char *id,
    const char *email,
    int mode,
    const char *exer,
    const char *file)
{
    ACCT x, *u;
    int i, num;
    FILE *flog;
    char buf[80];
    char fpath[128];

    u = &x;

    sprintf(fpath, "%s.%s", FN_SAMEEMAIL_LOG, file);

    num = same_mail2(email, file);
    flog = fopen(fpath, "r");

    if (!flog)
        return;

    for (i=1; i<=num; i++)
    {
        char fmt[13];
        sprintf(fmt, "%%%ds", IDLEN);
        fscanf(flog, fmt, buf);
        if (acct_load(u, buf) >= 0)
        {
            if (strcmp(u->userid, id))
                add_deny_exer(u, mode, 1, exer);
            else
                add_deny_exer(u, mode, 0, exer);
        }
    }

    if (mode & DENY_MODE_ALL)
        deny_log_email(email, (x.userlevel & PERM_DENYSTOP) ? -1 : x.deny);
    fclose(flog);
    unlink(fpath);
}

/* stopperm id email  mode  */


int
main(
    int argc,
    char *argv[])
{
    char buf[256];
    const char *userid = NULL, *email = NULL, *mode = NULL, *exer = NULL, *file = NULL;

    while (optind < argc)
    {
        switch (getopt(argc, argv, "+" "u:e:m:x:f:"))
        {
        case -1:  // Position arguments
            if (!(optarg = argv[optind++]))
                break;
            if (!userid)
        case 'u':
                userid = optarg;
            else if (!email)
        case 'e':
                email = optarg;
            else if (!mode)
        case 'm':
                mode = optarg;
            else if (!exer)
        case 'x':
                exer = optarg;
            else if (!file)
        case 'f':
                file = optarg;
            break;

        default:
            userid = email = mode = exer = file = NULL;  // Invalidate arguments
            optind = argc;  // Ignore remaining arguments
            break;
        }
    }

    if (userid || email || mode || exer || file)
    {
        if (!(userid && email && mode && exer && file))
        {
            fprintf(stderr, "Usage: %s\n", argv[0]);
            fprintf(stderr, "Do nothing but send a stopperm notification to bbs@" MYHOSTNAME "\n");
            fprintf(stderr, "Usage: %s [-u] <userid> [-e] <email> [-m] <deny_mode> [-x] <executor_userid> [-f] <tmpfile_suffix>\n", argv[0]);
            fprintf(stderr, "Remove permissions for users with mail <email> and then send a notification to the executor\n");
            return 2;
        }

        setup(userid, email, atoi(mode), exer, file);
        sprintf(buf, "mail %s.bbs@" MYHOSTNAME " < " FN_STOPPERM_MAIL, exer);
        system(buf);
    }
    else
        system("mail bbs@" MYHOSTNAME " < " FN_STOPPERM_MAIL);
    return 0;
}
