/*-------------------------------------------------------*/
/* xyz.c        ( NTHU CS MapleBBS Ver 3.10 )            */
/*-------------------------------------------------------*/
/* target : 雜七雜八的外掛                               */
/* create : 09/04/08                                     */
/* author : cache                                        */
/* update :                                              */
/*-------------------------------------------------------*/


#include "bbs.h"

#include <netinet/tcp.h>
#include <sys/wait.h>
#include <sys/resource.h>

extern UCACHE *ushm;

/* cache.081017:系統資訊 */

int
x_siteinfo(void)
{
    double load[3];
    getloadavg(load, 3);

    clear();

    move(1, 0);
    prints("站    名： %s - %s\n", MYHOSTNAME, BBSIP);
    prints("程式版本： %s [%s]\x1b[m\n", BBSVERNAME, BBSVERSION);
    prints("站上人數： %d/%d\n", ushm->count, MAXACTIVE);
    prints("系統負載： %.2f %.2f %.2f [%s]\n",
        load[0], load[1], load[2], load[0] > 10 ? "\x1b[1;41;37m過高\x1b[m" : load[0] > 5 ?
        "\x1b[1;42;37m稍高\x1b[m" : "\x1b[1;44;37m正常\x1b[m");
    prints("索引資料： BRD %d KB, ACCT %d KB, HDR %d KB\n", sizeof(BRD), sizeof(ACCT), sizeof(HDR));
    prints("\n");
    prints("\x1b[1m本 BBS 版本是由 WindTop BBS 為起始，\x1b[m\n");
    prints("\x1b[1m並參考諸位前輩的智慧結晶改版而來，所有智慧財產均屬於原作者。\x1b[m\n");
    prints("\n");
    prints("\x1b[1mDreamBBS.2010 Modified: Pang-Wei Tsai(cache)\x1b[m\n");
    prints("\x1b[1;33mInternet Technology Lab\x1b[37m, Institute of CCE, National Cheng Kung University.\x1b[m\n");
    prints("\n");
#ifdef Modules
    prints("\x1b[1;30mModules & Plug-in: \x1b[m\n\n");

//模組化的放在這邊
#ifdef MultiRecommend
    prints("\x1b[1;32m  online \x1b[1;30m  Multi Recommend Control 多樣化推文控制系統\x1b[m\n");
#endif
#ifdef M3_USE_PMORE
    prints("\x1b[1;32m  online \x1b[1;30m  pmore (piaip's more) 2007+ w/Movie\x1b[m\n");
#endif
#ifdef M3_USE_PFTERM
    prints("\x1b[1;32m  online \x1b[1;30m  pfterm (piaip's flat terminal, Perfect Term)\x1b[m\n");
#endif
#ifdef GRAYOUT
    prints("\x1b[1;32m  online \x1b[1;30m  Grayout Advanced Control 淡入淡出特效系統\x1b[m\n");
#else
    prints("\x1b[1;31m  offline\x1b[1;30m  Grayout Advanced Control 淡入淡出特效系統\x1b[m\n");
#endif

/*
#ifdef SMerge
    prints("\x1b[1;32m  online \x1b[1;30m  Smart Merge 修文自動合併\x1b[m\n");
#else
    prints("\x1b[1;31m  offline\x1b[1;30m  Smart Merge 修文自動合併\x1b[m\n");
#endif
#ifdef BBSRuby
    prints("\x1b[1;32m  online \x1b[1;30m  (EXP) BBSRuby v0.3\x1b[m\n");
#else
    prints("\x1b[1;31m  offline\x1b[1;30m  (EXP) BBSRuby v0.3\x1b[m\n");
#endif
*/

#else
//    prints("\x1b[1;30mModules & Plug-in: None\x1b[m\n");
#endif  /* #ifdef Modules */
    vmsg(NULL);
    return 0;
}
