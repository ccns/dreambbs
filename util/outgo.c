/*-------------------------------------------------------*/
/* util/outgo.c         ( YZU WindTopBBS Ver 3.00 )      */
/*-------------------------------------------------------*/
/* author : visor.bbs@bbs.yzu.edu.tw                     */
/* target : 自動送信程式                                 */
/* create : 2000/06/22                                   */
/* update :                                              */
/*-------------------------------------------------------*/
/* syntax : outgo [board] [start] [end]                  */
/* NOTICE :                                              */
/*-------------------------------------------------------*/
#include "bbs.h"

void
outgo_post(
    const HDR *hdr,
    const char *board)
{

    bntp_t bntp;
    memset(&bntp, 0, sizeof(bntp_t));
    bntp.chrono = hdr->chrono;
    strcpy(bntp.board, board);
    strcpy(bntp.xname, hdr->xname);
    strcpy(bntp.owner, hdr->owner);
    strcpy(bntp.nick, hdr->nick);
    strcpy(bntp.title, hdr->title);
    rec_add("innd/out.bntp", &bntp, sizeof(bntp_t));

    /*
    char *fpath, buf[256];

    fpath = "innd/out.bntp";

    sprintf(buf, "%s\t%s\t%s\t%s\t%s\n",
        board, hdr->xname, hdr->owner, hdr->nick, hdr->title);
    f_cat(fpath, buf);
    */
}

int
main(
    int argc,
    char *argv[])
{
    char fpath[128];
    int start = -1, end = -1, fd;
    char *board = NULL;
    HDR hdr;

    while (optind < argc)
    {
        switch (getopt(argc, argv, "+" "b:f:t:"))
        {
        case -1:  // Position arguments
            if (!(optarg = argv[optind++]))
                break;
            if (!board)
            {
        case 'b':
                board = optarg;
                break;
            }
            else if (!start)
            {
        case 'f':
                if ((start = atoi(optarg)) > 0)
                    break;
            }
            else if (!end)
            {
        case 't':
                if ((end = atoi(optarg)) > 0)
                    break;
            }
            else
                break;
            // Falls through
            // to handle invalid argument values
        default:
            board = NULL;  // Invalidate arguments
            optind = argc;  // Ignore remaining arguments
            break;
        }
    }

    if (board && start > 0 && end > 0)
    {
        brd_fpath(fpath, board, FN_DIR);
        if ((fd = open(fpath, O_RDONLY)) >= 0)
        {
            lseek(fd, (off_t)((start-1)*sizeof(HDR)), SEEK_SET);
            while (read(fd, &hdr, sizeof(HDR)) == sizeof(HDR) && start <= end)
            {
                outgo_post(&hdr, board);
                start++;
            }
            close(fd);
        }
    }
    else
    {
        fprintf(stderr, "\nSYNOPSIS : %s [-b] <看版> [-f] <起點> [-t] <終點>\n", argv[0]);
        fprintf(stderr, "(起點, 終點 > 0)\n");
        return 2;
    }
    return 0;
}
