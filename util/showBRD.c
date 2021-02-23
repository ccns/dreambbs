/*-------------------------------------------------------*/
/* util/showBRD.c       ( NTHU CS MapleBBS Ver 3.10 )    */
/*-------------------------------------------------------*/
/* target : show board info                              */
/* create : 01/10/05                                     */
/* update :   /  /                                       */
/* author : itoc.bbs@bbs.tnfsh.tn.edu.tw                 */
/*-------------------------------------------------------*/
/* syntax : showBRD [target_board]                       */
/*-------------------------------------------------------*/


#include "bbs.h"


int
main(
    int argc,
    char *argv[])
{
    int show_allbrd;
    BRD brd;
    FILE *fp;

    if (argc < 2)
        show_allbrd = 1;
    else
        show_allbrd = 0;

    setgid(BBSGID);
    setuid(BBSUID);
    chdir(BBSHOME);

    if (!(fp = fopen(FN_BRD, "r")))
        return -1;

    while (fread(&brd, sizeof(BRD), 1, fp) == 1)
    {
        if (show_allbrd || !str_casecmp(brd.brdname, argv[1]))
        {
            printf("看板名稱：%-*s      看板標題：[%s] %s\n", IDLEN, brd.brdname, brd.class_, brd.title);
            printf("投票狀態：%-13d     看板板主：%s\n", brd.bvote, brd.BM);
            printf("文章篇數：%d\n", brd.bpost);
            printf("開板時間：%s\n", Btime_any(&brd.bstamp));
            printf(".DIR時間：%s\n", Btime_any(&brd.btime));
            printf("最後一篇：%s\n", Btime_any(&brd.blast));

            if (!show_allbrd)
                break;
        }
    }

    fclose(fp);

    return 0;
}
