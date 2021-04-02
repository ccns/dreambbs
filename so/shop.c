/*-------------------------------------------------------*/
/* so/shop.c                                             */
/*-------------------------------------------------------*/
/* author : shiun.bbs@ccca.mksh.phc.edu.tw               */
/* modify : cat.bbs@ccca.mksh.phc.edu.tw                 */
/* update : cache.bbs@bbs.ee.ncku.edu.tw                 */
/* create : unknown                                      */
/* update : 09/10/11                                     */
/*-------------------------------------------------------*/
/* syntax :                                              */
/*-------------------------------------------------------*/

#include "bbs.h"

static int cloak_temp(void)
{
    time_t now;
    char c_time[25], c_buf[128];

    now = time(0);
    str_scpy(c_time, ctime(&now), sizeof(c_time));
    ACCT acct;
    int money;

    if (HAS_PERM(PERM_CLOAK))
    {
        pmsg2("�A���������v���I");
        return 0;
    }

    if (cutmp->ufo & UFO_CLOAK)
    {
        pmsg2("�z�w�g���ΤF�I");
        return 0;
    }

    if (acct_load(&acct, cuser.userid) >= 0)
        money = acct.money;
    else
    {
        pmsg2("�d�L�z���b���T...");
        return 0;
    }


    if (money < 65536)
    {
        pmsg2("�z���ڹ�����!!");
        return 0;
    }

    if (vans("�T�w�ʶR�ܡH [y/N]") != 'y')
        return 0;

    acct.money -= 65536;
    acct_save(&acct);

    cuser.ufo ^= UFO_CLOAK;
    cutmp->ufo ^= UFO_CLOAK;

//  acct_save(&cuser);
    pmsg2("�n�{���Э��s�W��");
    sprintf(c_buf, "%s %s �ʶR ���� �v��\n", c_time, cuser.userid);
    f_cat(FN_SHOP, c_buf);

    return 0;

}

static int hidefrom_temp(void)
{
    time_t now;
    char c_time[25], c_buf[128];

    now = time(0);
    str_scpy(c_time, ctime(&now), sizeof(c_time));
    ACCT acct;
    int money;

/*
    if (!HAS_PERM(PERM_12))
    {
        vmsg("���\\��Ȯ�����");
        return 0;
    }
*/

    if (HAS_PERM(PERM_CLOAK))
    {
        pmsg2("�A���������v���I");
        return 0;
    }

    if (cutmp->ufo & UFO_HIDDEN)
    {
        pmsg2("�z�w�g���ìG�m�F�I");
        return 0;
    }

    if (acct_load(&acct, cuser.userid) >= 0)
        money = acct.money;
    else
    {
        pmsg2("�d�L�z���b���T...");
        return 0;
    }

    if (money < 4194304)
    {
        pmsg2("�z���ڹ�����!!");
        return 0;
    }

    if (vans("�T�w�ʶR�ܡH [y/N]") != 'y')
        return 0;

    acct.money -= 4194304;
    acct_save(&acct);

    cuser.ufo ^= UFO_HIDDEN;
    cutmp->ufo ^= UFO_HIDDEN;

    acct_save(&cuser);
    pmsg2("�G�m�w����");
    sprintf(c_buf, "%s %s �ʶR ���ìG�m �v��\n", c_time, cuser.userid);
    f_cat(FN_SHOP, c_buf);

    return 0;

}

static int sysop(void)
{
    pmsg2("�O�ֻ��A�i�H�������I");
    return 0;
}

int shop_main(void)
{
    DL_HOLD;
    char buf[5];
    int money;

    ACCT acct;
    if (acct_load(&acct, cuser.userid) >= 0)
    {
        money = acct.money;
    }
    else
    {
        pmsg2("�d�L�z���b���T...");
        return DL_RELEASE(0);
    }

    clear();

    move(0, 0);
    prints("\x1b[1;33;42m");
    vs_mid(BOARDNAME "    �ө���");
    prints("\n");
    prints("                     �z���{���p�U�G\n\n                     �ڹ��G%d ��", money);
    move(10, 0);

    outs("          �w��i�J�ө���A�аݱz�ݭn����A�ȡG\n"
        "  \n"
        "            (1) �ʶR�Ȯ������N   (�ݭn�ڹ�   65536 ��/��)\n\n"
        "            (2) �ʶR�ä[�����N   (�ݭn�ڹ�  1048576 ��/��)\n\n"
        "            (3) �ʶR���ìG�m     (�ݭn�ڹ�  4194304 ��/��)\n\n"
        "            (4) �ʶR�����v��\x1b[1;31m HOT\x1b[m (�ݭn�ڹ� 10000000 ��)\n");

    if (!vget(B_LINES_REF, 0, "�п�ܱz�n���A�ȡG [Q] ���} ", buf, 2, DOECHO))
        return DL_RELEASE(0);

    if (*buf == '1')
        cloak_temp();
    else if (*buf == '2')
        pmsg2("���\\��|���}��");
    else if (*buf == '3')
        pmsg2("Sorry, we've closed this function...");
        //hidefrom_temp();
    else if (*buf == '4')
        sysop();
    else
        pmsg2("���¥��{");
    return DL_RELEASE(0);
}
