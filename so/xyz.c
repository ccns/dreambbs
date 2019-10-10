/*-------------------------------------------------------*/
/* xyz.c        ( NTHU CS MapleBBS Ver 3.10 )            */
/*-------------------------------------------------------*/
/* target : 雜七雜八的外掛                               */
/* create : 09/04/08                                     */
/* author : cache                                        */
/* update :                                              */
/*-------------------------------------------------------*/


#include "bbs.h"
#include "bbs_script.h"

#include <netinet/tcp.h>
#include <sys/wait.h>
#include <sys/resource.h>

extern UCACHE *ushm;

/* cache.081017:系統資訊 */

int
x_siteinfo(void)
{
    long nproc;
    double load[3], load_norm;
    nproc = sysconf(_SC_NPROCESSORS_ONLN);
    getloadavg(load, 3);
    load_norm = load[0] / ((nproc > 0) ? nproc : 2);

    clear();

    move(1, 0);
    prints("站    名： %s - %s\n", MYHOSTNAME, BBSIP);
    prints("程式版本： %s [%s]\x1b[m\n", BBSVERNAME, BBSVERSION);
    prints("站上人數： %d/%d\n", ushm->count, MAXACTIVE);
    prints("系統負載： %.2f %.2f %.2f / %ld [%s]\n",
        load[0], load[1], load[2], nproc, load_norm > 5 ? "\x1b[1;41;37m過高\x1b[m" : load_norm > 1 ?
        "\x1b[1;42;37m稍高\x1b[m" : "\x1b[1;44;37m正常\x1b[m");
    prints("索引資料： BRD %zu bytes, ACCT %zu bytes, HDR %zu bytes\n", sizeof(BRD), sizeof(ACCT), sizeof(HDR));
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
#define ONLINE_STR(module)  "\x1b[1;32m  online \x1b[1;30m  " module "\x1b[m\n"
#define OFFLINE_STR(module) "\x1b[1;31m  offline\x1b[1;30m  " module "\x1b[m\n"

#ifdef MultiRecommend
    prints(ONLINE_STR("Multi Recommend Control 多樣化推文控制系統"));
#else
    prints(OFFLINE_STR("Multi Recommend Control 多樣化推文控制系統"));
#endif
#ifdef M3_USE_PMORE
    prints(ONLINE_STR("pmore (piaip's more) 2007+ w/Movie"));
#else
    prints(OFFLINE_STR("pmore (piaip's more) 2007+ w/Movie"));
#endif
#ifdef M3_USE_PFTERM
    prints(ONLINE_STR("pfterm (piaip's flat terminal, Perfect Term)"));
#else
    prints(OFFLINE_STR("pfterm (piaip's flat terminal, Perfect Term)"));
#endif
#ifdef GRAYOUT
    prints(ONLINE_STR("Grayout Advanced Control 淡入淡出特效系統"));
#else
    prints(OFFLINE_STR("Grayout Advanced Control 淡入淡出特效系統"));
#endif
#ifdef HAVE_BBSLUA
  #ifdef M3_USE_BBSLUA
    #ifdef BBSLUA_USE_LUAJIT
    prints(ONLINE_STR("BBS-Lua " BBSLUA_VERSION_STR " / " LUA_RELEASE " / " LUAJIT_VERSION));
    #else
    prints(ONLINE_STR("BBS-Lua " BBSLUA_VERSION_STR " / " LUA_RELEASE));
    #endif
  #else
    prints(OFFLINE_STR("BBS-Lua " BBSLUA_VERSION_STR));
  #endif
#endif  // #ifdef HAVE_BBSLUA

/*
#ifdef SMerge
    prints(ONLINE_STR("Smart Merge 修文自動合併));
#else
    prints(OFFLINE_STR("Smart Merge 修文自動合併));
#endif
*/
#ifdef HAVE_BBSRUBY
  #ifdef M3_USE_BBSRUBY
    prints(ONLINE_STR("(EXP) BBSRuby " BBSRUBY_VERSION_STR " Interface: " BBSRUBY_INTERFACE_VER_STR " / Ruby " RUBY_RELEASE_STR));
  #else
    prints(OFFLINE_STR("(EXP) BBSRuby " BBSRUBY_VERSION_STR " Interface: " BBSRUBY_INTERFACE_VER_STR));
  #endif
#endif

#else
//    prints("\x1b[1;30mModules & Plug-in: None\x1b[m\n");
#endif  /* #ifdef Modules */
    vmsg(NULL);
    return 0;
}
