/*-------------------------------------------------------*/
/* util/addpost.c   ( NTHU CS MapleBBS Ver 3.00 )        */
/*-------------------------------------------------------*/
/* target : �t�Φ۰ʵo���ݪO                           */
/* create : 03/12/23                                     */
/* update :   /  /                                       */
/* author : itoc.bbs@bbs.tnfsh.tn.edu.tw                 */
/*-------------------------------------------------------*/


#include "bbs.h"

static char *userid;
static char *username;

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
add_post(           /* �o���ݪO */
    const char *brdname,  /* �� post ���ݪO */
    const char *fpath,    /* �ɮ׸��| */
    const char *title)    /* �峹���D */
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
    const char *fpath;
    const char *brdname, *title, *fname;
    FILE *fp;

    if (argc != 6)
    {
        printf("Usage: %s brdname userid username title filepath\n", argv[0]);
        return -1;
    }

    chdir(BBSHOME);

    fpath = "tmp/addpost.tmp";    /* �Ȧs�� */
    brdname = argv[1];
    userid = argv[2];
    username = argv[3];
    title = argv[4];
    fname = argv[5];

    if ( ( fp = fopen(fpath, "w") ) )
    {
        fprintf(fp, "%s %s (%s) %s %s\n",
            STR_AUTHOR1, userid, username, STR_POST2, brdname);
        fprintf(fp, "���D: %s\n�ɶ�: %s\n\n", title, Now());

        f_suck(fp, fname);
        fprintf(fp, "\n--\n�� ����Ѧ۰ʵo��t�Ωҵo��\n");

        fclose(fp);

        add_post(brdname, fpath, title);
        unlink(fpath);
    }

    return 0;
}

