/*-------------------------------------------------------*/
/* util/addpost.c   ( NTHU CS MapleBBS Ver 3.00 )        */
/*-------------------------------------------------------*/
/* target : 系統自動發文到看板                           */
/* create : 03/12/23                                     */
/* update :   /  /                                       */
/* author : itoc.bbs@bbs.tnfsh.tn.edu.tw                 */
/*-------------------------------------------------------*/


#include "bbs.h"

static const char *userid;
static const char *username;

static void
out_post(
    HDR *hdr,
    const char *board)
{
    bntp_t bntp;

    memset(&bntp, 0, sizeof(bntp_t));
    strcpy(bntp.board, board);
    strcpy(bntp.xname, hdr->xname);
    strcpy(bntp.owner, hdr->owner);
    strcpy(bntp.nick, hdr->nick);
    strcpy(bntp.title, hdr->title);
    rec_add("innd/out.bntp", &bntp, sizeof(bntp_t));
}


static void
add_post(           /* 發文到看板 */
    const char *brdname,  /* 欲 post 的看板 */
    const char *fpath,    /* 檔案路徑 */
    const char *title)    /* 文章標題 */
{
    HDR hdr;
    char folder[64];

    brd_fpath(folder, brdname, FN_DIR);
    hdr_stamp(folder, HDR_LINK | 'A', &hdr, fpath);
    strcpy(hdr.owner, userid);
    strcpy(hdr.nick, username);
    strcpy(hdr.title, title);
    hdr.xmode = POST_OUTGO;
    rec_add(folder, &hdr, sizeof(HDR));

    out_post(&hdr, brdname);
}


int
main(
    int argc,
    char *argv[])
{
    const char *fpath = NULL;
    const char *brdname = NULL, *title = NULL, *fname = NULL;
    FILE *fp;

    while (optind < argc)
    {
        switch (getopt(argc, argv, "+" "b:u:n:t:f:"))
        {
        case -1:  // Position arguments
            if (!(optarg = argv[optind++]))
                break;
            if (!brdname)
        case 'b':
                brdname = optarg;
            else if (!userid)
        case 'u':
                userid = optarg;
            else if (!username)
        case 'n':
                username = optarg;
            else if (!title)
        case 't':
                title = optarg;
            else if (!fname)
        case 'f':
                fname = optarg;
            break;

        default:
            brdname = userid = username = title = fname = NULL;  // Invalidate arguments
            optind = argc;  // Ignore remaining arguments
            break;
        }
    }

    if (!(brdname && userid && username && title && fname))
    {
        fprintf(stderr, "Usage: %s [-b] <brdname> [-u] <userid> [-n] <username> [-t] <title> [-f] <filepath>\n", argv[0]);
        return 2;
    }

    chdir(BBSHOME);

    fpath = "tmp/addpost.tmp";    /* 暫存檔 */

    if ( ( fp = fopen(fpath, "w") ) )
    {
        fprintf(fp, "%s %s (%s) %s %s\n",
            STR_AUTHOR1, userid, username, STR_POST2, brdname);
        fprintf(fp, "標題: %s\n時間: %s\n\n", title, Now());

        f_suck(fp, fname);
        fprintf(fp, "\n--\n※ 本文由自動發文系統所發表\n");

        fclose(fp);

        add_post(brdname, fpath, title);
        unlink(fpath);
    }

    return 0;
}

