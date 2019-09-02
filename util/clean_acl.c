/*-------------------------------------------------------*/
/* util/clean_acl.c     ( YZU WindTopBBS Ver 3.00 )      */
/*-------------------------------------------------------*/
/* target : 清除 mail.acl 中重複之紀錄                   */
/* create : 95/03/29                                     */
/* update : 97/03/29                                     */
/*-------------------------------------------------------*/
/* syntax : clean_acl [acl file]                         */
/*-------------------------------------------------------*/

#include "bbs.h"

#define MAX_AC  (50000)

char map[MAX_AC][256];
int total;

static int
check_in(
    const char *email)
{
    int i;
    for (i=0; i<MAX_AC; i++)
        if (!strcmp(map[i], email))
            return 1;
    return 0;
}

int
main(
    int argc,
    char *argv[])
{
    FILE *fp, *fd;
    char buf[256], tmp[256], *ptr;
    const char *infile = NULL, *outfile = NULL;

    while (optind < argc)
    {
        switch (getopt(argc, argv, "+" "i:o:"))
        {
        case -1:  // Position arguments
            if (!(optarg = argv[optind++]))
                break;
            if (!infile)
        case 'i':
                infile = optarg;
            else if (!outfile)
        case 'o':
                outfile = optarg;
            break;

        default:
            infile = outfile = NULL;  // Invalidate arguments
            optind = argc;  // Ignore remaining arguments
            break;
        }
    }

    if (!(infile && outfile))
    {
        printf("Usage: %s [-i] <acl_file_in> [-o] <acl_file_out>\n", argv[0]);
        return 2;
    }


    fp = fopen(infile, "r");
    if (fp)
    {
        fd = fopen(outfile, "w");
        if (fd)
        {
            while (fgets(buf, 256, fp))
            {
                if (strstr(buf, ".epaper.com.tw"))
                    continue;
                if (strstr(buf, MYHOSTNAME))
                    continue;
                strcpy(tmp, buf);
                ptr = (char *)strchr(buf, '#');
                if (ptr)
                    *ptr = '\0';
                if (!check_in(buf))
                {
                    fprintf(fd, "%s", tmp);
                    strcpy(map[total++], buf);
                }
            }
            fclose(fd);
        }
        fclose(fp);
    }
    return 0;
}
