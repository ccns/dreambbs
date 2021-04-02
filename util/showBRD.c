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
            printf("�ݪO�W�١G%-*s      �ݪO���D�G[%s] %s\n", IDLEN, brd.brdname, brd.class_, brd.title);
            printf("�벼���A�G%-13d     �ݪO�O�D�G%s\n", brd.bvote, brd.BM);
            printf("�峹�g�ơG%d\n", brd.bpost);
            printf("�}�O�ɶ��G%s\n", Btime_any(&brd.bstamp));
            printf(".DIR�ɶ��G%s\n", Btime_any(&brd.btime));
            printf("�̫�@�g�G%s\n", Btime_any(&brd.blast));

            if (!show_allbrd)
                break;
        }
    }

    fclose(fp);

    return 0;
}
