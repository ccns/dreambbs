/*-------------------------------------------------------*/
/* util/outgo.c		( YZU WindTopBBS Ver 3.00 )	 */
/*-------------------------------------------------------*/
/* author : visor.bbs@bbs.yzu.edu.tw			 */
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
    HDR *hdr,
    char *board)
{

    bntp_t bntp;
    memset(&bntp,0,sizeof(bntp_t));
    bntp.chrono = hdr->chrono;
    strcpy(bntp.board,board);
    strcpy(bntp.xname,hdr->xname);
    strcpy(bntp.owner,hdr->owner);
    strcpy(bntp.nick,hdr->nick);
    strcpy(bntp.title,hdr->title);
    rec_add("innd/out.bntp",&bntp,sizeof(bntp_t));

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
    int start,end,fd;
    char *board;
    HDR hdr;

    if (argc > 3)
    {
        board = argv[1];
        start = atoi(argv[2]);
        end = atoi(argv[3]);
        brd_fpath(fpath,board,FN_DIR);
        if ((fd = open(fpath,O_RDONLY)))
        {
            lseek(fd,(off_t)((start-1)*sizeof(HDR)),SEEK_SET);
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
        printf("\nSYNOPSIS : outgo 看版 起點 終點\n");
    }
    return 0;
}
