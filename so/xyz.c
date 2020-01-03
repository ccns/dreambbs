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
    prints("站    名： %s - %s\n", str_host, BBSIP);
    prints("程式版本： %s [%s] %s\n", bbsvername, bbsversion, build_head);
    prints("分支版本： %s %s %s\n", build_remote_url, build_branch_remote, build_head_remote);
    prints("編譯環境： %s %s %s %s%d\n",
        build_make, build_arch, build_compiler, build_lang, build_langver / 100 % 100);
    prints("系統負載： %.2f %.2f %.2f / %ld [%s] ",
        load[0], load[1], load[2], nproc, load_norm > 5 ? "\x1b[1;41;37m過高\x1b[m" : load_norm > 1 ?
        "\x1b[1;42;37m稍高\x1b[m" : "\x1b[1;44;37m正常\x1b[m");
    prints("站上人數： %d/%d\n", ushm->count, MAXACTIVE);
    prints("索引資料： BRD %zu bytes, ACCT %zu bytes, HDR %zu bytes\n", sizeof(BRD), sizeof(ACCT), sizeof(HDR));
    prints("\n");
    prints("\x1b[1m本 BBS 版本是由 WindTop BBS 為起始，\x1b[m\n");
    prints("\x1b[1m並參考諸位前輩的智慧結晶改版而來，所有智慧財產均屬於原作者。\x1b[m\n");
    prints("\n");
    prints("\x1b[1mDreamBBS.2010 Modified: Pang-Wei Tsai(cache)\x1b[m\n");
    prints("\x1b[1;33mInternet Technology Lab\x1b[37m, Institute of CCE, National Cheng Kung University.\x1b[m\n");
    prints("\n");
#ifdef Modules
#define CHECK_CONF(conf)  ((bool)(module_flags & (MODULE_ ## conf)))
    prints("Modules & Plug-in: %s\x1b[m\n\n",
        CHECK_CONF(DL_HOTSWAP) ? "[\x1b[1;32mHotswap enabled\x1b[m]" : "[\x1b[1;31mHotswap disabled\x1b[m]");

//模組化的放在這邊
#define ONLINE_STR  "\x1b[1;32monline \x1b[m"
#define OFFLINE_STR "\x1b[1;31moffline\x1b[1;30m"
#define STATUS_STR(conf)  ((module_flags & (MODULE_ ## conf)) ? ONLINE_STR : OFFLINE_STR)
#define STATUS_FMT  "%s %s \x1b[1;30m%s %s\x1b[m\n"  /* status, name, version, author */

    prints(STATUS_FMT, STATUS_STR(MultiRecommend), "Multi Recommend Control 多樣化推文控制系統", "\b", "\b");
    prints(STATUS_FMT, STATUS_STR(M3_USE_PMORE), "pmore (piaip's more)", "2007+ w/Movie", "by piaip");
    prints(STATUS_FMT, STATUS_STR(M3_USE_PFTERM), "pfterm (piaip's flat terminal, Perfect Term)", "\b", "by piaip");
    prints(STATUS_FMT, STATUS_STR(GRAYOUT), "Grayout Advanced Control 淡入淡出特效系統", "\b", "by hrs113355");
#ifdef HAVE_BBSLUA
    prints(STATUS_FMT, STATUS_STR(M3_USE_BBSLUA), "BBS-Lua", bbslua_version_str, "by piaip");
#endif
//    prints(STATUS_FMT, STATUS_STR(SMerge), "Smart Merge 修文自動合併", "", "by hrs113355 & cache");
#ifdef HAVE_BBSRUBY
    prints(STATUS_FMT, STATUS_STR(M3_USE_BBSRUBY), "(EXP) BBSRuby", bbsruby_version_str, "by zero");
#endif

#else
//    prints("\x1b[1;30mModules & Plug-in: None\x1b[m\n");
#endif  /* #ifdef Modules */
    vmsg(NULL);
    return 0;
}
