/*-------------------------------------------------------*/
/* xyz.c	( NTHU CS MapleBBS Ver 3.10 )	             */
/*-------------------------------------------------------*/
/* target : ���C���K���~��			                   	 */
/* create : 09/04/08					                 */
/* author : cache                                        */
/* update :      					                     */
/*-------------------------------------------------------*/


#include "bbs.h"

#include <netinet/tcp.h>
#include <sys/wait.h>
#include <sys/resource.h>

extern UCACHE *ushm;

/* cache.081017:�t�θ�T */

int
x_siteinfo()
{
    double load[3];
    getloadavg(load, 3);

    clear();

    move(1, 0);
    prints("��    �W�G %s - %s\n", MYHOSTNAME, BBSIP);
    prints("�{�������G %s [%s]\033[m\n", BBSNAME, BBSVERSION);
    prints("���W�H�ơG %d/%d\n", ushm->count, MAXACTIVE);
    prints("�t�έt���G %.2f %.2f %.2f [%s]\n"
	    , load[0], load[1], load[2], load[0] > 10 ? "\033[1;41;37m�L��\033[m" : load[0] > 5 ? 
        "\033[1;42;37m�y��\033[m" : "\033[1;44;37m���`\033[m");
    prints("���޸�ơG BRD %d KB, ACCT %d KB, HDR %d KB\n", sizeof(BRD), sizeof(ACCT), sizeof(HDR));
    prints("\n");
    prints("\033[1m�� BBS �����O�� WindTop BBS ���_�l�A\033[m\n");
    prints("\033[1m�ðѦҽѦ�e�������z�����睊�ӨӡA�Ҧ����z�]�����ݩ��@�̡C\033[m\n");
    prints("\n");
    prints("\033[1mDreamBBS.2010 Modified: Pang-Wei Tsai(cache)\033[m\n");
    prints("\033[1;33mInternet Technology Lab\033[37m, Institute of CCE, National Cheng Kung University.\033[m\n");
    prints("\n");
#ifdef Modules    
    prints("\033[1;30mModules & Plug-in: \033[m\n\n");

//�Ҳդƪ���b�o��
#ifdef MultiRecommend
    prints("\033[1;32m  online \033[1;30m  Multi Recommend Control �h�ˤƱ��山��t��\033[m\n");
#endif
#ifdef M3_USE_PMORE
    prints("\033[1;32m  online \033[1;30m  pmore (piaip's more) 2007 w/Movie\033[m\n");
#endif
#ifdef GRAYOUT
    prints("\033[1;32m  online \033[1;30m  Grayout Advanced Control �H�J�H�X�S�Ĩt��\033[m\n");
#else
    prints("\033[1;31m  offline\033[1;30m  Grayout Advanced Control �H�J�H�X�S�Ĩt��\033[m\n");
#endif

/*
#ifdef SMerge
    prints("\033[1;32m  online \033[1;30m  Smart Merge �פ�۰ʦX��\033[m\n");
#else
    prints("\033[1;31m  offline\033[1;30m  Smart Merge �פ�۰ʦX��\033[m\n");
#endif
#ifdef BBSRuby
    prints("\033[1;32m  online \033[1;30m  (EXP) BBSRuby v0.3\033[m\n");
#else
    prints("\033[1;31m  offline\033[1;30m  (EXP) BBSRuby v0.3\033[m\n");
#endif
*/

#else
//    prints("\033[1;30mModules & Plug-in: None\033[m\n");
#endif
    vmsg(NULL);
    return 0;
}
