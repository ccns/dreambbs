/*-------------------------------------------------------*/
/* xyz.c        ( NTHU CS MapleBBS Ver 3.10 )            */
/*-------------------------------------------------------*/
/* target : ���C���K���~��                               */
/* create : 09/04/08                                     */
/* author : cache                                        */
/* update :                                              */
/*-------------------------------------------------------*/


#include "bbs.h"
#include "bbs_script.h"

#include <netinet/tcp.h>
#include <sys/wait.h>
#include <sys/resource.h>

/* cache.081017:�t�θ�T */

int
x_siteinfo(void)
{
    DL_HOLD;
    long nproc;
    double load[3], load_norm;
    nproc = sysconf(_SC_NPROCESSORS_ONLN);
    getloadavg(load, 3);
    load_norm = load[0] / ((nproc > 0) ? nproc : 2);

    clear();

    move(1, 0);
    prints("��    �W�G%s - %s\n", str_host, BBSIP);
    prints("�{�������G%s [%s] %s", bbsvername, bbsversion, build_head);
    if (strcmp(BUILD_HEAD, build_head))
        prints(" \x1b[1m�̷s�G%s\x1b[m\n", BUILD_HEAD);
    prints("���䪩���G%s %s %s\n",
        build_remote_url, build_branch_remote,
        (!strcmp(BUILD_BRANCH_REMOTE, build_branch_remote)) ? BUILD_HEAD_REMOTE : build_head_remote);
    prints("�sĶ���ҡG%s %s %s %s%d\n",
        build_make, build_arch, build_compiler, build_lang, build_langver / 100 % 100);
    prints("�t�έt���G%.2f %.2f %.2f / %ld [%s] ",
        load[0], load[1], load[2], nproc, load_norm > 5 ? "\x1b[1;41;37m�L��\x1b[m" : load_norm > 1 ?
        "\x1b[1;42;37m�y��\x1b[m" : "\x1b[1;44;37m���`\x1b[m");
    prints("���W�H�ơG%d/%d\n", ushm->count, MAXACTIVE);
    prints("���޸�ơGBRD %zu bytes, ACCT %zu bytes, HDR %zu bytes\n", sizeof(BRD), sizeof(ACCT), sizeof(HDR));
    prints("\n");
    prints("\x1b[1m�� BBS �����O�� WindTop BBS ���_�l�A\x1b[m\n");
    prints("\x1b[1m�ðѦҽѦ�e�������z�����睊�ӨӡA�Ҧ����z�]�����ݩ��@�̡C\x1b[m\n");
    prints("\n");
    prints("\x1b[1mDreamBBS.2010 Modified: Pang-Wei Tsai(cache)\x1b[m\n");
    prints("\x1b[1;33mInternet Technology Lab\x1b[37m, Institute of CCE, National Cheng Kung University.\x1b[m\n");
    prints("\n");
#ifdef Modules
#define CHECK_CONF(conf)  ((bool)(module_flags & (MODULE_ ## conf)))
    prints("Modules & Plug-in: %s\x1b[m\n\n",
        CHECK_CONF(DL_HOTSWAP) ? "[\x1b[1;32mHotswap enabled\x1b[m]" : "[\x1b[1;31mHotswap disabled\x1b[m]");

//�Ҳդƪ���b�o��
#define ONLINE_STR  "\x1b[1;32monline \x1b[m"
#define OFFLINE_STR "\x1b[1;31moffline\x1b[1;30m"
#define STATUS_STR(conf)  ((module_flags & (MODULE_ ## conf)) ? ONLINE_STR : OFFLINE_STR)
#define STATUS_FMT  "%s %s \x1b[1;30m%s %s\x1b[m\n"  /* status, name, version, author */

    prints(STATUS_FMT, STATUS_STR(MultiRecommend), "Multi Recommend Control �h�ˤƱ��山��t��", "\b", "\b");
    prints(STATUS_FMT, STATUS_STR(M3_USE_PMORE), "pmore (piaip's more)", "2007+ w/Movie", "by piaip");
    prints(STATUS_FMT, STATUS_STR(M3_USE_PFTERM), "pfterm (piaip's flat terminal, Perfect Term)", "\b", "by piaip");
    prints(STATUS_FMT, STATUS_STR(GRAYOUT), "Grayout Advanced Control �H�J�H�X�S�Ĩt��", "\b", "by hrs113355");
#ifdef HAVE_BBSLUA
    prints(STATUS_FMT, STATUS_STR(M3_USE_BBSLUA), "BBS-Lua", bbslua_version_str, "by piaip");
#endif
//    prints(STATUS_FMT, STATUS_STR(SMerge), "Smart Merge �פ�۰ʦX��", "", "by hrs113355 & cache");
#ifdef HAVE_BBSRUBY
    prints(STATUS_FMT, STATUS_STR(M3_USE_BBSRUBY), "(EXP) BBSRuby", bbsruby_version_str, "by zero");
#endif

#else
//    prints("\x1b[1;30mModules & Plug-in: None\x1b[m\n");
#endif  /* #ifdef Modules */
    vmsg(NULL);
    return DL_RELEASE(0);
}
