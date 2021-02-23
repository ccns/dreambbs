/*-------------------------------------------------------*/
/* util/hdr-dump.c      ( NTHU CS MapleBBS Ver 3.02 )    */
/*-------------------------------------------------------*/
/* target : 看板標題表                                   */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/
/* Usage:       hdr-dump .DIR                            */
/*-------------------------------------------------------*/


#include "bbs.h"
int
main(
    int argc,
    char *argv[])
{
    const char *bmw_file = NULL, *userid = NULL;
    int inf, count;
    BMW bmw;
    ACCT acct;
    char fpath[80];

    while (optind < argc)
    {
        switch (getopt(argc, argv, "+" "b:u:"))
        {
        case -1:  // Position arguments
            if (!(optarg = argv[optind++]))
                break;
            if (!bmw_file)
        case 'b':
                bmw_file = optarg;
            else if (!userid)
        case 'u':
                userid = optarg;
            break;

        default:
            bmw_file = userid = NULL;  // Invalidate arguments
            optind = argc;  // Ignore remaining arguments
            break;
        }
    }

    if (!(bmw_file && userid))
    {
        fprintf(stderr, "Usage:\t%s [-b] <bmw file> [-u] <userid>\n", argv[0]);
        exit(2);
    }

    strcpy(fpath, BBSHOME"/");
    usr_fpath(fpath + strlen(BBSHOME) + 1, userid, FN_ACCT);
    inf = open(fpath, O_RDONLY);
    if (inf == -1)
    {
        printf("error open acct file %s\n", fpath);
        exit(1);
    }
    if (read(inf, &acct, sizeof(ACCT)) != sizeof(ACCT))
    {
        close(inf);
        printf("error open acct file\n");
        exit(1);
    }
    close(inf);


    inf = open(bmw_file, O_RDONLY);
    if (inf == -1)
    {
        printf("error open bmw file\n");
        exit(1);
    }

    count = 0;


    while (read(inf, &bmw, sizeof(BMW)) == sizeof(BMW))
    {
        struct tm *ptime = localtime_any(&bmw.btime);
        count++;
        printf("%s%s(%02d:%02d)：%s\x1b[m\n",
               bmw.sender == acct.userno ? "☆" : "\x1b[32m★",
               bmw.userid, ptime->tm_hour, ptime->tm_min, bmw.msg);
    }
    close(inf);

    exit(0);
}


