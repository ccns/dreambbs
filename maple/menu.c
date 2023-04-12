/*-------------------------------------------------------*/
/* menu.c       ( NTHU CS MapleBBS Ver 3.00 )            */
/*-------------------------------------------------------*/
/* target : menu/help/movie routines                     */
/* create : 95/03/29                                     */
/* update : 2000/01/02                                   */
/*-------------------------------------------------------*/

#include "bbs.h"

#ifdef  HAVE_INFO
#define INFO_EMPTY      "Info      【 \x1b[1;36m校方公告區\x1b[m 】"
#define INFO_HAVE       "Info      【 \x1b[41;33;1;5m快進來看看\x1b[m 】"
#endif

#ifdef  HAVE_STUDENT
#define STUDENT_EMPTY   "1Student  【 \x1b[1;36m學生公告區\x1b[m 】"
#define STUDENT_HAVE    "1Student  【 \x1b[41;33;1;5m快進來看看\x1b[m 】"
#endif

#define GOODBYE_EXIT    "Goodbye   【再別" BOARDNAME "】"
#define GOODBYE_GOBACK  "GoBack    【 回上層選單 】"

#define MENU_HELP       NULL
#define MENU_HELP_NCUR  "(Tab) 開關選單移動模式；(Space) 切換使用中游標" // Multiple-cursor mode

static int
system_result(void)
{
    char fpath[128];
    sprintf(fpath, "brd/%s/@/@vote", brd_sysop);
    more(fpath, 0);
    return 0;
}


static int
view_login_log(void)
{
    char fpath[128];
    usr_fpath(fpath, cuser.userid, FN_LOG);
    more(fpath, 0);
    return 0;
}

static int
menumore(const void *arg)
{
    more((const char *)arg, 0);
    return 0;
}


static int
welcome(void)
{
    film_out(FILM_WELCOME, -1);
    return 0;
}

static int
resetbrd(void)
{
    board_main();
    return 0;
}

#if 0  // Unused
static int
x_sysload(void)
{
    long nproc;
    double load[3];
    char buf[80];

    /*load = ushm->sysload;*/
    nproc = sysconf(_SC_NPROCESSORS_ONLN);
    getloadavg(load, 3);
    sprintf(buf, "系統負載 %.2f %.2f %.2f / %ld", load[0], load[1], load[2], nproc);
    vmsg(buf);
    return XEASY;
}
#endif


/* ----------------------------------------------------- */
/* 離開 BBS 站                                           */
/* ----------------------------------------------------- */


#define FN_NOTE_PAD     "run/note.pad"
#define FN_NOTE_TMP     "run/note.tmp"
#define NOTE_MAX        100
#define NOTE_DUE        48


typedef struct
{
    time32_t tpad;
    char msg[356];
}      Pad;  /* DISKDATA(raw) */


int
pad_view(void)
{
    int fd, count;
    Pad *pad;

    if ((fd = open(FN_NOTE_PAD, O_RDONLY)) < 0)
        return XEASY;

    clear();
    move(0, 23);
    outs("\x1b[1;37;45m ●  " BOARDNAME " 留 言 板  ● \x1b[m\n\n");
    count = 0;

    mgets(-1);

    for (;;)
    {
        pad = (Pad *) mread(fd, sizeof(Pad));
        if (!pad)
        {
            vmsg(NULL);
            break;
        }

        outs(pad->msg);
        if (++count == 5)
        {
            move(b_lines, 0);
            outs("請按 [SPACE] 繼續觀賞，或按其他鍵結束：");
            if (vkey() != ' ')
                break;

            count = 0;
            move(2, 0);
            clrtobot();
        }
    }

    close(fd);
    return 0;
}


static void
pad_draw(void)
{
    int i, cc, fdr, len;
    FILE *fpw;
    Pad pad;
    char *str, str2[300], buf[3][80];

    do
    {
        buf[0][0] = buf[1][0] = buf[2][0] = '\0';
        move(12, 0);
        clrtobot();
        outs("\n請留言 (至多三行)，按[Enter]結束");
        for (i = 0; (i < 3) &&
            vget(16 + i, 0, "：", buf[i], 70, DOECHO); i++);
        cc = vans("(S)存檔觀賞 (E)重新來過 (Q)算了？[S] ");
        if (cc == 'q' || i == 0)
            return;
    } while (cc == 'e');

    time32(&(pad.tpad));

    str = pad.msg;

    sprintf(str, "\x1b[1;37;46mΥ\x1b[34;47m %s \x1b[33m(%s)", cuser.userid, cuser.username);
    len = strlen(str);
    strcat(str, & " \x1b[30;46m"[len % 2U]);

    for (i = len >> 1; i < 41; i++)
        strcat(str, "▄");
    sprintf(str2, "\x1b[34;47m %.14s \x1b[37;46mΥ\x1b[m\n%-70.70s\n%-70.70s\n%-70.70s\n",
        Etime_any(&(pad.tpad)), buf[0], buf[1], buf[2]);
    strcat(str, str2);

    f_cat(FN_NOTE_ALL, str);

    if (!(fpw = fopen(FN_NOTE_TMP, "w")))
        return;
    len = 342;  // strlen(str)
    memset(str + len + 1, 0, sizeof(pad.msg) - (len + 1));
    str[355] = '\n';

    fwrite(&pad, sizeof(pad), 1, fpw);

    if ((fdr = open(FN_NOTE_PAD, O_RDONLY)) >= 0)
    {
        Pad *pp;

        i = 0;
        cc = pad.tpad - NOTE_DUE * 60 * 60;
        mgets(-1);
        while ((pp = (Pad *) mread(fdr, sizeof(Pad))))
        {
            fwrite(pp, sizeof(Pad), 1, fpw);
            if ((++i > NOTE_MAX) || (pp->tpad < cc))
                break;
        }
        close(fdr);
    }

    fclose(fpw);

    rename(FN_NOTE_TMP, FN_NOTE_PAD);
    pad_view();
}


static int
goodbye(void)
{
    char ans;
    if (xo_stack_level > 0)
        return QUIT;

    bmw_save();
    if (cuser.ufo2 & UFO2_DEF_LEAVE)
    {
        if (!(ans = vans("G)再別" NICKNAME " M)報告站長 N)留言板 Q)取消？[Q] ")))
            ans = 'q';
    }
    else
        ans = vans("G)再別" NICKNAME " M)報告站長 N)留言板 Q)取消？[Q] ");

    switch (ans)
    {
    case 'g':
    case 'y':
        break;

    case 'm':
        mail_sysop();
        break;

    case 'n':
        /* if (cuser.userlevel) */
        if (HAS_PERM(PERM_POST)) /* Thor.990118: 要能post才能留言, 提高門檻 */
            pad_draw();
        break;

    case 'q':
    default: /* 090911.cache: 不小心按錯不要趕走人家 ;( */
        return XEASY;
    }

    return QUIT;
}


/* ----------------------------------------------------- */
/* help & menu processing                                */
/* ----------------------------------------------------- */

void
vs_mid(
    const char *mid)
{
    int spc, len, pad;
    unsigned int ufo;

    if (mid == NULL)
        mid = str_site;

    len = strlen(mid);
    ufo = cutmp->ufo;
    if (ufo & UFO_BIFF)
    {
        mid = NEWMAILMSG; // 你有新情書
        len = 15;
    }
    else if (ufo & UFO_BIFFN)
    {
        mid = NEWPASSMSG; // 你有新留言
        len = 15;
    }

    spc = b_cols - len; /* spc: 中間還剩下多長的空間 */
    pad = spc >> 1; /* pad: Spaces needed to center `mid` */

    prints("%*s%s%*s\x1b[m\n", pad, "", mid, spc - pad, "");
}

void
vs_head(
    const char *title, const char *mid)
{
    int spc, len, len_ttl, pad;
    unsigned int ufo;
#ifdef  COLOR_HEADER
/*  int color = (time(0) % 7) + 41;        lkchu.981201: random color */
    int color = 44; //090911.cache: 太花了固定一種顏色
#endif

    if (mid == NULL)
    {
        move(0, 0);
        clrtoeol();
        mid = str_site;
    }
    else
    {
        clear();
    }

    len_ttl = strcspn(title, "\n");
    len = strlen(mid);
    ufo = cutmp->ufo;
    if (ufo & UFO_BIFF)
    {
        mid = NEWMAILMSG; // 你有新情書
        len = 15;
    }
    else if (ufo & UFO_BIFFN)
    {
        mid = NEWPASSMSG; // 你有新留言
        len = 15;
    }

    spc = b_cols - 14 - len - strlen(currboard); /* spc: 中間還剩下多長的空間 */
    len_ttl = BMIN(len_ttl, spc); /* Truncate `title` if too long */
    spc -= len_ttl; /* 擺完 title 以後，中間還有 spc 格空間 */
    pad = BMAX(((b_cols - len) >> 1) - (len_ttl + 5), 0); /* pad: Spaces needed to center `mid` */

#ifdef  COLOR_HEADER
    prints("\x1b[1;%2d;37m【%.*s】%*s \x1b[33m%s\x1b[1;%2d;37m%*s \x1b[37m看板《%s》\x1b[m\n",
        color, len_ttl, title, pad, "", mid, color, spc - pad, "", currboard);
#else
    prints("\x1b[1;46;37m【%.*s】%*s \x1b[33m%s\x1b[46m%*s \x1b[37m看板《%s》\x1b[m\n",
        len_ttl, title, pad, "", mid, spc - pad, "", currboard);
#endif
}


void clear_notification(void)
{
    unsigned int ufo = cutmp->ufo;

    if (ufo & UFO_BIFF)
        cutmp->ufo = ufo ^ UFO_BIFF;     /* 看過就算 */
    if (ufo & UFO_BIFFN)
        cutmp->ufo = ufo ^ UFO_BIFFN;     /* 看過就算 */
}


/* ------------------------------------- */
/* 動畫處理                              */
/* ------------------------------------- */


static char footer[160];


void
movie(void)
{
    if (cuser.ufo2 & UFO2_MOVIE)
    {
        static int tag = FILM_MOVIE;

        tag = film_out(tag, 2);
    }
}

static void
menu_foot(void)
{
    static int orig_flag = -1;
    static time_t uptime = -1;
    static char flagmsg[16];
    static char datemsg[16];

    int ufo;
    time_t now;

    ufo = cuser.ufo;
    time(&now);

    /* Thor: 同時 顯示 呼叫器 好友上站 隱身 */

    ufo &= UFO_PAGER | UFO_CLOAK | UFO_QUIET | UFO_PAGER1 | UFO_MESSAGE;
    if (orig_flag != ufo)
    {
        orig_flag = ufo;
        sprintf(flagmsg,
            "%s/%s",
            (ufo & UFO_PAGER1) ? "全關" : (ufo & UFO_PAGER) ? "半關" : "全開",
            (ufo & UFO_MESSAGE) ? "全關" : (ufo & UFO_QUIET) ? "半關" : "全開");
    }

    if (now > uptime)
    {
        struct tm *ptime;

        ptime = localtime(&now);
        sprintf(datemsg, "[%d/%d 星期%.2s ",
            ptime->tm_mon + 1, ptime->tm_mday,
            & "天一二三四五六"[2 * ptime->tm_wday]);

        uptime = now + 86400 - ptime->tm_hour * 3600 -
            ptime->tm_min * 60 - ptime->tm_sec;
    }
    ufo = (now - (uptime - 86400)) / 60;

    /* Thor.980913: 註解: 最常見呼叫 movie()的時機是每次更新 film, 在 60秒以上,
                          故不需針對 xx:yy 來特別作一字串儲存以加速 */

    sprintf(footer, "\x1b[0;34;46m%s%d:%02d] \x1b[30;47m 目前站上有\x1b[31m%4d\x1b[30m 人，我是 \x1b[31m%-*s\t\x1b[30m [呼叫/訊息]\x1b[31m%s",
        datemsg, ufo / 60, ufo % 60,
        /*ushm->count*/total_num,
        12 + 14 - INT(strlen(datemsg)) + (ufo / 60 < 10),
        cuser.userid, flagmsg);
    outf(footer);
}


#define MENU_CHANG      0x80000000


#ifdef __cplusplus
  #define INTERNAL extern  /* Used inside an anonymous namespace */
  #define INTERNAL_INIT /* Empty */
#else
  #define INTERNAL static
  #define INTERNAL_INIT static
#endif

#ifdef __cplusplus
namespace {
#endif

INTERNAL MENU menu_main[];
INTERNAL MENU menu_service[];
INTERNAL MENU menu_xyz[];
INTERNAL MENU menu_user[];
INTERNAL MENU menu_song[];


/* ----------------------------------------------------- */
/* load menu                                             */
/* ----------------------------------------------------- */
INTERNAL MENU menu_admin[];

INTERNAL_INIT MENU menu_boardadm[] =
{
    {{m_newbrd}, PERM_BOARD, M_SYSTEM,
    "NewBoard   開闢新看板"},

    {{a_editbrd}, PERM_BOARD, M_SYSTEM,
    "ConfigBrd  設定看板"},

    {{m_bmset}, PERM_BOARD, M_SYSTEM,
    "BMset      設定版主權限"},

    {{.dl = {DL_NAME("banmail.so", BanMail)}}, PERM_BOARD|PERM_SYSOP, M_DL(M_BANMAIL),
    "FireWall   擋信列表"},

    {{.dl = {DL_NAME("adminutil.so", bm_check)}}, PERM_BOARD|PERM_SYSOP, M_DL(M_XMODE),
    "Manage     板主確認"},

    {{.dl = {DL_NAME("adminutil.so", m_expire)}}, PERM_BOARD|PERM_SYSOP, M_DL(M_XMODE),
    "DExpire    清除看板刪除文章"},

    {{.dl = {DL_NAME("adminutil.so", mail_to_bm)}}, PERM_SYSOP, M_DL(M_XMODE),
    "ToBM       寄信給板主"},

    {{.dl = {DL_NAME("adminutil.so", mail_to_all)}}, PERM_SYSOP, M_DL(M_XMODE),
    "Alluser    系統通告"},

    {{.dl = {DL_NAME("personal.so", personal_admin)}}, PERM_BOARD|PERM_SYSOP, M_DL(M_XMODE),
    "Personal   個人板審核"},

    {{.menu = menu_admin}, PERM_MENU + 'N', M_ADMIN,
    "看板總管"}
};

INTERNAL_INIT MENU menu_accadm[] =
{
    {{m_user}, PERM_ACCOUNTS, M_SYSTEM,
    "User       使用者資料"},

    {{.dl = {DL_NAME("bank.so", money_back)}}, PERM_ACCOUNTS, M_DL(M_XMODE),
    "GetMoney   匯入舊夢幣"},

#ifdef  HAVE_SONG
    {{.dl = {DL_NAME("song.so", AddRequestTimes)}}, PERM_KTV, M_DL(M_XMODE),
    "Addsongs   增加點歌次數"},
#endif

    {{.dl = {DL_NAME("passwd.so", new_passwd)}}, PERM_SYSOP, M_DL(M_XMODE),
    "Password   重送新密碼"},

#ifdef  HAVE_REGISTER_FORM
    {m_register}, PERM_ACCOUNTS, M_SYSTEM,
    "1Register  審核註冊表單"},
#endif

#ifdef HAVE_OBSERVE_LIST
    {{.dl = {DL_NAME("observe.so", Observe_list)}}, PERM_SYSOP|PERM_BOARD, M_DL(M_XMODE),
    "2Observe   系統觀察名單"},
#endif

    {{.menu = menu_admin}, PERM_MENU + 'U', M_ADMIN,
    "註冊總管"}
};

INTERNAL_INIT MENU menu_settingadm[] =
{

    {{.dl = {DL_NAME("adminutil.so", m_xfile)}}, PERM_SYSOP, M_DL(M_XFILES),
    "File       編輯系統檔案"},

    {{.dl = {DL_NAME("adminutil.so", m_xhlp)}}, PERM_SYSOP, M_DL(M_XFILES),
    "Hlp        編輯說明檔案"},

    {{.dl = {DL_NAME("admin.so", Admin)}}, PERM_SYSOP, M_DL(M_XMODE),
    "Operator   系統管理員列表"},

    {{.dl = {DL_NAME("chatmenu.so", Chatmenu)}}, PERM_CHATROOM|PERM_SYSOP, M_DL(M_XMODE),
    "Chatmenu   " CHATROOMNAME "動詞"},

    {{.dl = {DL_NAME("violate.so", Violate)}}, PERM_SYSOP, M_DL(M_XMODE),
    "Violate    處罰名單"},

    {{.dl = {DL_NAME("adminutil.so", special_search)}}, PERM_SYSOP, M_DL(M_XMODE),
    "XSpecial   特殊搜尋"},

    {{.dl = {DL_NAME("adminutil.so", update_all)}}, PERM_SYSOP, M_DL(M_XMODE),
    "Database   系統資料庫更新"},

    {{.menu = menu_admin}, PERM_MENU + 'X', M_ADMIN,
    "系統資料"}
};

/* ----------------------------------------------------- */
/* reset menu                                            */
/* ----------------------------------------------------- */
INTERNAL_INIT MENU menu_reset[] =
{
    {{DLFUNCARG(DL_NAME("adminutil.so", m_resetsys), 1)}, PERM_BOARD, M_DL(M_XMODE) | M_ARG,
    "Camera     動態看板"},

    {{DLFUNCARG(DL_NAME("adminutil.so", m_resetsys), 2)}, PERM_BOARD, M_DL(M_XMODE) | M_ARG,
    "Group      分類群組"},

    {{DLFUNCARG(DL_NAME("adminutil.so", m_resetsys), 3)}, PERM_SYSOP, M_DL(M_XMODE) | M_ARG,
    "Mail       寄信收信轉信"},

    {{DLFUNCARG(DL_NAME("adminutil.so", m_resetsys), 4)}, PERM_ADMIN, M_DL(M_XMODE) | M_ARG,
    "Killbbs    清除不正常 BBS"},

    {{DLFUNCARG(DL_NAME("adminutil.so", m_resetsys), 5)}, PERM_BOARD, M_DL(M_XMODE) | M_ARG,
    "Firewall   擋信列表"},

    {{DLFUNCARG(DL_NAME("adminutil.so", m_resetsys), 6)}, PERM_CHATROOM, M_DL(M_XMODE) | M_ARG,
    "Xchatd     重開聊天室"},

    {{DLFUNCARG(DL_NAME("adminutil.so", m_resetsys), 7)}, PERM_SYSOP, M_DL(M_XMODE) | M_ARG,
    "All        全部"},

    {{.menu = menu_admin}, PERM_MENU + 'K', M_ADMIN,
    "系統重置"}
};


/* ----------------------------------------------------- */
/* administrator's maintain menu                         */
/* ----------------------------------------------------- */


INTERNAL_INIT MENU menu_admin[] =
{

    {{.menu = menu_accadm}, PERM_ADMIN, M_ADMIN,
    "Acctadm    註冊總管功\能"},

    {{.menu = menu_boardadm}, PERM_BOARD, M_ADMIN,
    "Boardadm   看板總管功\能"},

    {{.menu = menu_settingadm}, PERM_ADMIN, M_ADMIN,
    "Data       系統資料庫設定"},

    {{.dl = {DL_NAME("innbbs.so", a_innbbs)}}, PERM_BOARD, M_DL(M_SYSTEM),
    "InnBBS     轉信設定"},

    {{.menu = menu_reset}, PERM_ADMIN, M_ADMIN,
    "ResetSys   重置系統"},

#ifdef  HAVE_ADM_SHELL
    {{x_csh}, PERM_SYSOP, M_SYSTEM,
    "Shell      執行系統 Shell"},
#endif

#ifdef  HAVE_REPORT
    {{m_trace}, PERM_SYSOP, M_SYSTEM,
    "Trace      設定是否記錄除錯資訊"},
#endif

    {{.menu = menu_main}, PERM_MENU + 'A', M_ADMIN,
    "系統維護"}
};

#ifdef __cplusplus
}  // namespace
#endif


/* ----------------------------------------------------- */
/* mail menu                                             */
/* ----------------------------------------------------- */


static int
XoMbox(void)
{
    if (HAS_PERM(PERM_DENYMAIL))
        vmsg("您的信箱被鎖了！");
    else
        xover(XZ_MBOX);
    return 0;
}

#ifdef HAVE_SIGNED_MAIL
int m_verify(void);
#endif
int m_setmboxdir(void);

#ifdef __cplusplus
namespace {
#endif
INTERNAL_INIT MENU menu_mail[] =
{
    {{XoMbox}, PERM_READMAIL, M_RMAIL,
    "Read       閱\讀信件"},

    {{m_send}, PERM_INTERNET, M_SMAIL,
    "MailSend   站內寄信"},

#ifdef MULTI_MAIL  /* Thor.981009: 防止愛情幸運信 */
    {{mail_list}, PERM_INTERNET, M_SMAIL,
    "Group      群組寄信"},
#endif

    {{.dl = {DL_NAME("contact.so", Contact)}}, PERM_INTERNET, M_DL(M_XMODE),
    "Contact    聯絡名單"},

    {{m_setforward}, PERM_INTERNET, M_SMAIL,
    "AutoFor    站內信自動轉寄"},

    {{m_setmboxdir}, PERM_INTERNET, M_SMAIL,
    "Fixdir     重建信箱索引"},

#ifdef HAVE_DOWNLOAD
    {{m_zip}, PERM_VALID, M_XMODE,
    "Zip        打包下載個人資料"},
#endif
/*
#ifdef HAVE_SIGNED_MAIL
    {{m_verify}, PERM_VALID, M_XMODE,
    "Verify     驗證信件電子簽章"},
#endif
*/
    {{mail_sysop}, PERM_BASIC, M_SMAIL,
    "Yes Sir!   寄信給站長"},

    {{.menu = menu_main}, PERM_MENU + 'R', M_MMENU,       /* itoc.020829: 怕 guest 沒選項 */
    "電子郵件"}
};
#ifdef __cplusplus
}  // namespace
#endif


/* ----------------------------------------------------- */
/* Talk menu                                             */
/* ----------------------------------------------------- */

static int
XoUlist(void)
{
    xover(XZ_ULIST);
    return 0;
}


#ifdef __cplusplus
namespace {
#endif

INTERNAL_INIT MENU menu_talk[] =
{
    {{XoUlist}, 0, M_LUSERS,
    "Users      完全聊天手冊"},

    {{t_pal}, PERM_BASIC, M_PAL,
    "Friend     編輯好友名單"},

#ifdef  HAVE_BANMSG
    {{t_banmsg}, PERM_BASIC, M_XMODE,
    "Banmsg     拒收訊息名單"},
#endif
    {{.dl = {DL_NAME("aloha.so", t_aloha)}}, PERM_BASIC, M_DL(M_XMODE),
    "Aloha      上站通知名單"},

    {{t_pager}, PERM_BASIC, M_XMODE,
    "Pager      切換呼叫器"},

    {{t_message}, PERM_BASIC, M_XMODE,
    "Message    切換訊息"},

    {{t_query}, 0, M_QUERY,
    "Query      查詢網友"},

    /* Thor.990220: chatroom client 改採外掛 */
    {{.dl = {DL_NAME("chat.so", t_chat)}}, PERM_CHAT, M_DL(M_CHAT),
    "ChatRoom   " NICKNAME CHATROOMNAME},

    {{t_recall}, PERM_BASIC, M_XMODE,
    "Write      回顧前幾次熱訊"},

#ifdef LOGIN_NOTIFY
    {{t_loginNotify}, PERM_PAGE, M_XMODE,
    "Notify     設定系統網友協尋"},
#endif
    {{.menu = menu_main}, PERM_MENU + 'U', M_UMENU,
    "休閒聊天"}
};


/* ----------------------------------------------------- */
/* System menu                                           */
/* ----------------------------------------------------- */

INTERNAL_INIT MENU menu_information[] =
{

    {{FUNCARG(menumore, "gem/@/@pop")}, 0, M_READA | M_ARG,
    "Login      上站次數排行榜"},

    {{FUNCARG(menumore, "gem/@/@-act")}, 0, M_READA | M_ARG,
    "Today      今日上線人次統計"},

    {{FUNCARG(menumore, "gem/@/@=act")}, 0, M_READA | M_ARG,
    "Yesterday  昨日上線人次統計"},

    {{FUNCARG(menumore, "gem/@/@-day")}, 0, M_READA | M_ARG,
    "0Day       本日十大熱門話題"},

    {{FUNCARG(menumore, "gem/@/@-week")}, 0, M_READA | M_ARG,
    "1Week      本週五十大熱門話題"},

    {{FUNCARG(menumore, "gem/@/@-month")}, 0, M_READA | M_ARG,
    "2Month     本月百大熱門話題"},

    {{FUNCARG(menumore, "gem/@/@-year")}, 0, M_READA | M_ARG,
    "3Year      本年度百大熱門話題"},

    {{.menu = menu_xyz}, PERM_MENU + 'L', M_MMENU,
    "統計資料"}
};


INTERNAL_INIT MENU menu_xyz[] =
{
    {{.menu = menu_information}, 0, M_XMENU,
    "Tops       " NICKNAME "排行榜"},

    {{FUNCARG(menumore, FN_ETC_VERSION)}, 0, M_READA | M_ARG,
    "Version    源碼發展資訊"},

    {{.dl = {DL_NAME("xyz.so", x_siteinfo)}}, 0, M_DL(M_READA),
    "Xinfo      系統程式資訊"},

    {{pad_view}, 0, M_READA,
    "Note       觀看心情留言板"},

    {{welcome}, 0, M_READA,
    "Welcome    觀看歡迎畫面"},

    {{FUNCARG(menumore, FN_ETC_COUNTER)}, 0, M_READA | M_ARG,
    "History    本站歷史軌跡"},

    {{.menu = menu_main}, PERM_MENU + 'T', M_SMENU,
    "系統資訊"}
};

/* ----------------------------------------------------- */
/* User menu                                             */
/* ----------------------------------------------------- */

INTERNAL_INIT MENU menu_reg[] =
{

    {{u_info}, PERM_BASIC, M_XMODE,
    "Info       設定個人資料與密碼"},

    {{u_addr}, PERM_BASIC, M_XMODE,
    "Address    填寫電子信箱及認證"},

    {{u_verify}, PERM_BASIC, M_UFILES,
    "Verify     填寫《註冊認證碼》"},

#ifdef  HAVE_REGISTER_FORM
    {{u_register}, PERM_BASIC, M_UFILES,
    "Register   填寫《註冊申請單》"},
#endif

    {{u_setup}, PERM_VALID, M_UFILES,
    "Mode       設定操作模式"},

    {{ue_setup}, 0, M_UFILES,
    "Favorite   個人喜好設定"},

    {{u_xfile}, PERM_BASIC, M_UFILES,
    "Xfile      編輯個人檔案"},

    {{.dl = {DL_NAME("list.so", List)}}, PERM_VALID, M_DL(M_XMODE),
    "1List      群組名單"},

    {{.menu = menu_user}, PERM_MENU + 'I', M_MMENU,
    "註冊資訊"}
};


INTERNAL_INIT MENU menu_user[] =
{
    {{.menu = menu_reg}, 0, M_XMENU,
    "Configure  註冊及設定個人資訊"},

    {{u_lock}, PERM_BASIC, M_XMODE,
    "Lock       鎖定螢幕"},

    {{.dl = {DL_NAME("memorandum.so", Memorandum)}}, PERM_VALID, M_DL(M_XMODE),
    "Note       備忘錄"},

    {{.dl = {DL_NAME("pnote.so", main_note)}}, PERM_VALID, M_DL(M_XMODE),
    "PNote      個人答錄機"},

#ifdef  HAVE_CLASSTABLEALERT
    {{.dl = {DL_NAME("classtable2.so", main_classtable)}}, PERM_VALID, M_DL(M_XMODE),
    "2Table     新版個人功\課表"},
#endif

    {{view_login_log}, PERM_BASIC, M_READA,
    "ViewLog    檢視上站紀錄"},

    {{.menu = menu_service}, PERM_MENU + 'C', M_UMENU,
    "個人設定"}
};


/* ----------------------------------------------------- */
/* tool menu                                             */
/* ----------------------------------------------------- */

INTERNAL MENU menu_service[];

/* ----------------------------------------------------- */
/* game menu                                             */
/* ----------------------------------------------------- */

INTERNAL_INIT MENU menu_game[] =
{
    {{.dl = {DL_NAME("bj.so", BlackJack)}}, PERM_VALID, M_DL(M_XMODE),
    "BlackJack  " NICKNAME "黑傑克"},

    {{.dl = {DL_NAME("guessnum.so", fightNum)}}, PERM_VALID, M_DL(M_XMODE),
    "FightNum   數字大決戰"},

    {{.dl = {DL_NAME("guessnum.so", guessNum)}}, PERM_VALID, M_DL(M_XMODE),
    "GuessNum   傻瓜猜數字"},

    {{.dl = {DL_NAME("mine.so", Mine)}}, PERM_VALID, M_DL(M_XMODE),
    "Mine       " NICKNAME "踩地雷"},

    {{.dl = {DL_NAME("pip.so", p_pipple)}}, PERM_VALID, M_DL(M_XMODE),
    "Pip        " NICKNAME "戰鬥雞"},

    {{.menu = menu_service}, PERM_MENU + 'F', M_UMENU,
    "遊戲休閒"}

};

/* ----------------------------------------------------- */
/* yzu menu                                              */
/* ----------------------------------------------------- */

INTERNAL_INIT MENU menu_special[] =
{

    {{.dl = {DL_NAME("personal.so", personal_apply)}}, PERM_VALID, M_DL(M_XMODE),
    "PBApply      申請個人看板"},

    {{.dl = {DL_NAME("bank.so", bank_main)}}, PERM_VALID, M_DL(M_XMODE),
    "Bank       　銀行"},

    {{.dl = {DL_NAME("shop.so", shop_main)}}, PERM_VALID, M_DL(M_XMODE),
    "Pay        　商店"},

#ifdef HAVE_SONG
    {{.menu = menu_song}, PERM_VALID, M_XMENU,
    "Request      點歌系統"},
#endif

    {{resetbrd}, PERM_ADMIN, M_XMODE,
    "CameraReset  版面重設"},

    {{.menu = menu_service}, PERM_MENU + 'R', M_UMENU,
    "加值服務"}
};



/* ----------------------------------------------------- */
/* song menu                                             */
/* ----------------------------------------------------- */

#ifdef HAVE_SONG
INTERNAL_INIT MENU menu_song[] =
{
    {{.dl = {DL_NAME("song.so", XoSongMain)}}, PERM_VALID, M_DL(M_XMODE),
    "Request       點歌歌本"},

    {{.dl = {DL_NAME("song.so", XoSongLog)}}, PERM_VALID, M_DL(M_XMODE),
    "OrderSongs    點歌紀錄"},

    {{.dl = {DL_NAME("song.so", XoSongSub)}}, PERM_VALID, M_DL(M_XMODE),
    "Submit        投稿專區"},

    {{.menu = menu_special}, PERM_MENU + 'R', M_XMENU,
    "網呼點歌"}
};
#endif


/* ----------------------------------------------------- */
/* service menu                                          */
/* ----------------------------------------------------- */

/* Thor.990224: 開放外掛界面 */
INTERNAL_INIT MENU menu_service[] =
{

    {{.menu = menu_user}, 0, M_UMENU,
    "User      【 個人工具區 】"},

    {{.menu = menu_special}, PERM_VALID, M_XMENU,
    "Bonus     【 加值服務區 】"},

    {{.menu = menu_game}, PERM_VALID, M_XMENU,
    "Game      【 遊戲體驗區 】"},

#ifdef  HAVE_INFO
    {{Information}, 0, M_BOARD,
    INFO_EMPTY},
#endif

#ifdef  HAVE_STUDENT
    {{Student}, 0, M_BOARD,
    STUDENT_EMPTY},
#endif

/* 091007.cache: 拉人灌票沒意義... */

    {{.dl = {DL_NAME("newboard.so", XoNewBoard)}}, PERM_VALID, M_DL(M_XMODE),
    "Cosign    【 連署申請區 】"},

    {{.dl = {DL_NAME("vote.so", SystemVote)}}, PERM_POST, M_DL(M_XMODE),
    "Vote      【 系統投票區 】"},

    {{system_result}, 0, M_READA,
    "Result    【系統投票結果】"},

/*
#ifdef HAVE_SONG
    {{.menu = menu_song}, PERM_VALID, M_XMENU,
    "Song      【  點歌系統區  】"},
#endif
*/
    {{.menu = menu_main}, PERM_MENU + 'U', M_UMENU,
     NICKNAME "服務"}
};

#ifdef __cplusplus
}  // namespace
#endif

/* ----------------------------------------------------- */
/* main menu                                             */
/* ----------------------------------------------------- */

#ifdef  HAVE_CHANGE_SKIN
static int
sk_windtop_init(void)
{
    s_menu = menu_windtop;
    vmsg("DEBUG:DreamBBS");
    return SKIN;
}

#ifdef __cplusplus
namespace {
#endif
INTERNAL_INIT MENU skin_main[] =
{
    {{sk_windtop_init}, PERM_SYSOP, M_XMODE,
    "DreamBBS   預設的系統"},

    {{.menu = menu_main}, PERM_MENU + 'W', M_MMENU,
    "介面選單"}
};
#ifdef __cplusplus
}  // namespace
#endif
#endif  /* #ifdef  HAVE_CHANGE_SKIN */

static int
Gem(void)
{
    XoGem("gem/.DIR", "", ((HAS_PERM(PERM_SYSOP|PERM_BOARD|PERM_GEM)) ? GEM_SYSOP : GEM_USER));
    return 0;
}

#ifdef __cplusplus
namespace {
#endif

INTERNAL_INIT MENU menu_main[] =
{
    {{.menu = menu_admin}, PERM_ADMIN, M_ADMIN,
    "0Admin    【 系統維護區 】"},

    {{Gem}, 0, M_GEM,
    "Announce  【 精華公佈欄 】"},

    {{Boards}, 0, M_BOARD,
    "Boards    【 \x1b[1;33m佈告討論區\x1b[m 】"},

    {{Class}, 0, M_CLASS,
    "Class     【 \x1b[1;33m分組討論區\x1b[m 】"},

#ifdef  HAVE_PROFESS
    {{Profess}, 0, M_PROFESS,
    "Profession【 \x1b[1;33m專業討論區\x1b[m 】"},
#endif

#ifdef  HAVE_FAVORITE
    {{MyFavorite}, PERM_BASIC, M_CLASS,
    "Favorite  【 \x1b[1;32m我的最愛區\x1b[m 】"},
#endif

#ifdef HAVE_SIGNED_MAIL
    {{.menu = menu_mail}, PERM_BASIC, M_MAIL, /* Thor.990413: 若不用m_verify, guest就沒有選單內容囉 */
    "Mail      【 信件工具區 】"},
#else
    {{.menu = menu_mail}, PERM_BASIC, M_MAIL,
    "Mail      【 私人信件區 】"},
#endif

    {{.menu = menu_talk}, 0, M_TMENU,
    "Talk      【 休閒聊天區 】"},

    {{.menu = menu_service}, PERM_BASIC, M_XMENU,
    "DService  【 " NICKNAME "服務區 】"},

    /* lkchu.990428: 不要都塞在個人工具區 */
    {{.menu = menu_xyz}, 0, M_SMENU,
    "Xyz       【 系統資訊區 】"},

#ifdef  HAVE_CHANGE_SKIN
    {{.menu = *skin_main}, PERM_SYSOP, M_XMENU,
    "2Skin     【 選擇介面區 】"},
#endif

#ifndef MENU_NO_GOODBYE
    {{goodbye}, 0, M_XMODE,
    GOODBYE_EXIT},
#endif

    {{NULL}, PERM_MENU + 'B', M_MMENU,
    "主功\能表"}
};

#ifdef __cplusplus
}  // namespace
#endif

#ifdef  TREAT
static int
goodbye1(void)
{
    switch (vans("G)再別" NICKNAME " Q)取消？[Q] "))
    {
    case 'g':
    case 'y':
        return QUIT:
        break;

    case 'q':
    default:
        break;
    }

    clear();
    outs("※ 偵測到您的電腦試圖攻擊伺服器 ※\n");
    bell();
    vkey();
    outs("※ 哈哈  騙你的啦  ^O^ ，" BOARDNAME "祝您愚人節快樂 ※\n");
    bell();
    vkey();
    return QUIT:
}


#ifdef __cplusplus
namespace {
#endif
INTERNAL_INIT MENU menu_treat[] =
{
    {{goodbye1}, 0, M_XMODE,
    GOODBYE_EXIT},

    {{NULL}, PERM_MENU + 'G', M_MMENU,
    "主功\能表"}
};
#ifdef __cplusplus
}  // namespace
#endif
#endif  /* #ifdef  TREAT */

GCC_PURE
int strip_ansi_n_len(
    const char *str,
    int maxlen)
{
    int len;
    const char *ptr, *tmp;
    ptr = str;
    len = strnlen(str, maxlen);

    while (ptr && maxlen-- >= 0)
    {
        ptr = strstr(ptr, "\x1b");
        if (ptr)
        {
            for (tmp=ptr; *tmp!='m'; tmp++);
            len -= (tmp-ptr+1);
            ptr = tmp+1;
        }
    }
    return len;
}

GCC_PURE
int strip_ansi_len(
    const char *str)
{
    return strip_ansi_n_len(str, strlen(str));
}


const char *
check_info(const void *func, const char *input)
{
#if defined(HAVE_INFO) || defined(HAVE_STUDENT)
    const BRD *brd;
#endif
    const char *name = NULL;
    name = input;
#ifdef  HAVE_INFO
    if (func == (const void *)Information)
    {
        brd = bshm->bcache + brd_bno(BRD_BULLETIN);
        if (brd)
        {
            check_new(brd);
            if (brd->blast > brd_visit[brd_bno(BRD_BULLETIN)])
                name = INFO_HAVE;
        }
    }
#endif
#ifdef  HAVE_STUDENT
    if (func == (const void *)Student)
    {
        brd = bshm->bcache + brd_bno(BRD_SBULLETIN);
        if (brd)
        {
            check_new(brd);
            if (brd->blast > brd_visit[brd_bno(BRD_SBULLETIN)])
                name = STUDENT_HAVE;
        }
    }
#endif
    if (func == (const void *)goodbye)
    {
        if (xo_stack_level > 0)
            name = GOODBYE_GOBACK;
    }

    return name;
}


void
main_menu(void)
{
#ifdef  TREAT
    domenu(menu_treat, MENU_YPOS_REF, MENU_XPOS_REF, 0, 0, 1);
#endif
    domenu(menu_main, MENU_YPOS_REF, MENU_XPOS_REF, 0, 0, 1);
}

/* IID.20200331: Restructure with xover commands */

typedef struct {
    int y_ref, x_ref, y_ref_prev, x_ref_prev, height_ref, width_ref;
    int y, x, height, width;
    MENU *menu, *mtail;
    MENU **table;
    int table_size;
    int pos_prev; /* previous cursor positions */
    int max_prev; /* previous menu max */

    /* IID.20200107: Match input sequence. */
    int cmdcur_max;
    int *cmdcur;  /* Command matching cursor */
    int *cmdlen;  /* Command matching length (no spaces) */
    bool keep_cmd;
    bool keyboard_cmd;
    bool default_cmd;
    int *item_length;
    int max_item_length;
    int explan_len_prev;
    bool is_moving;
    bool is_moving_prev;
} DomenuXyz;

GCC_PURE static int
domenu_gety(
    int num,
    const DomenuXyz *xyz)
{
    if (xyz->height > 0 && xyz->width > 0)
        return xyz->y + (num % xyz->height);
    else
        return xyz->y + num;
}

GCC_PURE static int
domenu_getx(
    int num,
    const DomenuXyz *xyz)
{
    if (xyz->height > 0 && xyz->width > 0)
        return xyz->x + (num / xyz->height * xyz->width);
    else
        return xyz->x;
}

GCC_NONNULLS
GCC_PURE static int domenu_geth(int max, const DomenuXyz *xyz)
{
    return (xyz->height) ? xyz->height : max;
}

GCC_NONNULLS
GCC_PURE static int domenu_getw(int max, const DomenuXyz *xyz)
{
    return ((xyz->width) ? xyz->width : xyz->max_item_length) * ((xyz->height) ? (max - 1)/xyz->height + 1 : 1);
}

/* returns whether the menu moved successfully */
GCC_NONNULLS
static bool domenu_move(XO *xo, int cmd)
{
    DomenuXyz *const xyz = (DomenuXyz *)xo->xyz;
    const int h = domenu_geth(xo->max, xyz);
    const int w = domenu_getw(xo->max, xyz);
    const int y_ref = gety_bound_move(cmd, xyz->y_ref, 1, 1 + ((B_LINES_REF - h) >> 1), B_LINES_REF - h);
    const int x_ref = getx_bound_move(cmd, xyz->x_ref, 0, (B_COLS_REF - w) >> 1, B_COLS_REF - w);
    const bool diff = (y_ref != xyz->y_ref) || (x_ref != xyz->x_ref);
    xyz->y_ref = y_ref;
    xyz->x_ref = x_ref;
    return diff;
}

static int
domenu_item(
    XO *xo,
    int pos)
{
    DomenuXyz *const xyz = (DomenuXyz *)xo->xyz;
    char item[ANSILINESIZE];
    const MENU *const mptr = xyz->table[pos];
    const char *const str = check_info((const void *)mptr->item.func, mptr->desc);
    const int item_str_len = strcspn(str, "\n");
    const int match_max = BMIN(xyz->cmdcur_max, item_str_len);
    const int y = domenu_gety(pos, xyz);

    sprintf(item, "\x1b[m(\x1b[1;36m%-*.*s\x1b[m)%.*s",
            xyz->cmdcur_max, match_max, str, BMAX(0, item_str_len - match_max), str + match_max);
    outs(item);

    if (HAVE_UFO2_CONF(UFO2_MENU_LIGHTBAR))
        grayout(y, y + 1, GRAYOUT_COLORNORM);

    xyz->item_length[pos] = 4 + strip_ansi_n_len(str, item_str_len);
    xyz->max_item_length = BMAX(xyz->max_item_length, xyz->item_length[pos]);
    clrtoeol();

    return XO_NONE;
}

static int
domenu_redo(
    XO *xo,
    int cmd)
{
    DomenuXyz *const xyz = (DomenuXyz *)xo->xyz;
    int cmd_res = cmd & XO_MOVE_MASK;

    if (cmd & XZ_ZONE)
        return cmd;  /* Cannot handle this */

    if (cmd & XR_PART_LOAD)
    {
        const unsigned int level = cuser.userlevel;
        unsigned int mlevel;
        unsigned int umode;
        int max;

#ifdef  MENU_VERBOSE
domenu_redo_reload:
#endif
        max = 0;
        for (xyz->mtail = xyz->menu;; xyz->mtail++)
        {
            mlevel = xyz->mtail->level;
            umode = xyz->mtail->umode;
            if ((umode & M_TAIL_MASK) || (mlevel & PERM_MENU))
            {

#ifdef  MENU_VERBOSE
                if (max <= 0)                /* 找不到適合權限之功能，回上一層功能表 */
                {
                    xyz->menu = xyz->mtail->menu;
                    goto domenu_redo_reload;
                }
#endif

                break;
            }
            if (mlevel && !(mlevel & level))
                continue;
            if (!strncmp(xyz->mtail->desc, OPT_OPERATOR, strlen(OPT_OPERATOR)) && !(supervisor || !str_casecmp(cuser.userid, ELDER) || !str_casecmp(cuser.userid, STR_SYSOP)))
                continue;

            xyz->table[max++] = xyz->mtail;
        }

        xo->max = max;
        xyz->max_prev = BMAX(xyz->max_prev, max);

        if (cmd_res == XO_NONE)
        {
            if ((xyz->menu == menu_main) && (cutmp->ufo & UFO_BIFF))
                cmd_res = 'M';
            else if ((xyz->menu == menu_main) && (cutmp->ufo & UFO_BIFFN))
                cmd_res = 'U';
            else
                cmd_res = mlevel ^ PERM_MENU;  /* default command */
            xyz->keyboard_cmd = false;
            xyz->default_cmd = true;
        }
        utmp_mode(xyz->mtail->umode);
    }
    if (cmd & XR_PART_HEAD)
    {
        /* Keep menu in bound after resizing */
        domenu_move(xo, cmd);

        /* Resize */
        xyz->y = gety_ref(xyz->y_ref);
        xyz->x = getx_ref(xyz->x_ref);
        xyz->height = gety_ref(xyz->height_ref);
        xyz->width = getx_ref(xyz->width_ref);

        /* Reset the previous state */
        xyz->y_ref_prev = xyz->y_ref;
        xyz->x_ref_prev = xyz->x_ref;
        xyz->max_prev = xo->max;

        clear();
        vs_head(xyz->mtail->desc, NULL);
        //prints("\n\x1b[30;47m     選項         選項說明                         動態看板                   \x1b[m\n");
    }
    if (cmd & XR_PART_NECK)
    {
        if (!MENU_NOMOVIE_POS(xyz->y, xyz->x))
            movie();
    }
    if (cmd & XR_PART_BODY)
    {
        const int y_prev = gety_ref(xyz->y_ref_prev);
        const int x_prev = getx_ref(xyz->x_ref_prev);
        const int h_prev = domenu_geth(xyz->max_prev, xyz);
        const int h = domenu_geth(xo->max, xyz);

        /* Clear the screen area for the menu item first */
        for (int i = 0; i < h_prev; ++i)
        {
            move_ansi(y_prev + i, BMAX(BMIN(x_prev, xyz->x) - 1, 0));
            clrtoeol();
        }
        if (xyz->y + (h - 1) >= b_lines - 1 && xyz->explan_len_prev > b_cols - xyz->x)
        {
            /* Clean the previous explanation if the explanation is overlapped in the middle */
            move_ansi(b_lines - 1, b_cols - xyz->explan_len_prev);
            clrtoeol();
            xyz->explan_len_prev = 0;
            cmd |= XR_PART_KNEE;
        }

        /* Then draw the menu item */
        xyz->max_item_length = 0;
        for (int i = 0; i < xo->max; i++)
        {
            move_ansi(domenu_gety(i, xyz), domenu_getx(i, xyz) + 2);
            domenu_item(xo, i);
        }
        xyz->y_ref_prev = xyz->y_ref;
        xyz->x_ref_prev = xyz->x_ref;
        xyz->pos_prev = -1;
        xyz->max_prev = xo->max;

        /* Ensure that all items are inside the screen; otherwise move the screen and then redraw again */
        const int w = domenu_getw(xo->max, xyz);
        if (xyz->x > 1 && xyz->x + w > b_cols)
        {
            xyz->x_ref -= xyz->x + w - b_cols;
            /* Clean straying text caused by auto line wrapping */
            xyz->x_ref_prev = 0;
            xyz->max_prev = xo->max + 1;
            cmd_res |= XR_HEAD;
        }
    }
    if (cmd & XR_PART_KNEE)
    {
        const MENU *const mptr = xyz->table[xo->pos[xo->cur_idx]];
        const char *const desc = check_info((const void *)mptr->item.func, mptr->desc);
        const char *explan = strchr(desc, '\n');
        if (!explan)
            explan = strchr(xyz->mtail->desc, '\n');
        if (!explan && xo_ncur > 1)
            explan = MENU_HELP_NCUR;
        if (!explan)
            explan = MENU_HELP;

        move_ansi(b_lines - 1, b_cols - xyz->explan_len_prev);
        clrtoeol();
        if (explan)
        {
            const int h = domenu_geth(xo->max, xyz);
            const int w = domenu_getw(xo->max, xyz);
            while (explan[0] == '\n')
                ++explan;
            int explan_len = strip_ansi_len(explan);

            /* Ensure there is room to display the explanation */
            if (xyz->y + (h - 1) < b_lines - 1 || b_cols - (xyz->x + w) > explan_len)
            {
                move_ansi(b_lines - 1, b_cols - explan_len);
                outs(explan);
                xyz->explan_len_prev = explan_len;
            }
        }
        else
        {
            xyz->explan_len_prev = 0;
        }

    }
    if (cmd & XR_PART_FOOT)
    {
        menu_foot();
    }

    return cmd_res;
}

static int
domenu_cur(XO *xo, int pos)
{
    DomenuXyz *const xyz = (DomenuXyz *)xo->xyz;

    const int i = pos;
    move_ansi(domenu_gety(i, xyz), domenu_getx(i, xyz) + 2);
    if (i < xo->max)
        return domenu_item(xo, i);
    clrtoeol();
    return XO_NONE;
}

static int domenu_init(XO *xo) { return domenu_redo(xo, XO_INIT); }
static int domenu_load(XO *xo) { return domenu_redo(xo, XO_LOAD); }
static int domenu_head(XO *xo) { return domenu_redo(xo, XO_HEAD); }
static int domenu_neck(XO *xo) { return domenu_redo(xo, XO_NECK); }
static int domenu_body(XO *xo) { return domenu_redo(xo, XO_BODY); }
static int domenu_foot(XO *xo) { return domenu_redo(xo, XO_FOOT); }

KeyFuncList domenu_cb =
{
    {XO_INIT, {domenu_init}},
    {XO_LOAD, {domenu_load}},
    {XO_HEAD, {domenu_head}},
    {XO_NECK, {domenu_neck}},
    {XO_BODY, {domenu_body}},
    {XO_FOOT, {domenu_foot}},
    {XO_CUR | XO_POSF, {.posf = domenu_cur}},
};

static int domenu_exec(XO *xo, int cmd);

GCC_PURE int
gety_bound_move(
    int cmd, int y_ref, int min_ref, int mid_ref, int max_ref)
{
    int y = gety_ref(y_ref);
    int min = gety_ref(min_ref);
    int mid = gety_ref(mid_ref);
    int max = gety_ref(max_ref);

    switch (cmd)
    {
    case KEY_PGUP:
    case KEY_PGDN:
        if ((cmd == KEY_PGUP) ? y > mid : y < mid)
            return mid_ref;
        else
            return (cmd == KEY_PGUP) ? min_ref : max_ref;
    case KEY_UP:
    case KEY_DOWN:
        return y_ref + TCLAMP(y + ((cmd == KEY_DOWN) ? 1 : -1), min, BMAX(min, max)) - y;
    default:;
        return y_ref + TCLAMP(y, min, BMAX(min, max)) - y;
    }
}

GCC_PURE int
getx_bound_move(
    int cmd, int x_ref, int min_ref, int mid_ref, int max_ref)
{
    const int x = getx_ref(x_ref);
    const int min = getx_ref(min_ref);
    const int mid = getx_ref(mid_ref);
    const int max = getx_ref(max_ref);

    switch (cmd)
    {
    case KEY_HOME:
    case KEY_END:
        if ((cmd == KEY_HOME) ? x > mid : x < mid)
            return mid_ref;
        else
            return (cmd == KEY_HOME) ? min_ref : max_ref;
    case KEY_LEFT:
    case KEY_RIGHT:
        return x_ref + TCLAMP(x + ((cmd == KEY_RIGHT) ? 1 : -1), min, BMAX(min, max)) - x;
    default:;
        return x_ref + TCLAMP(x, min, BMAX(min, max)) - x;
    }
}

void
domenu(
    MENU *menu, int y_ref, int x_ref, int height_ref, int width_ref, int cmdcur_max)
{
    MENU *table[41];

    /* IID.20200107: Match input sequence. */
    int cmdcur[COUNTOF(table)];  /* Command matching cursor */
    int cmdlen[COUNTOF(table)];  /* Command matching length (no spaces) */
    int item_length[COUNTOF(table)] = {0};

    DomenuXyz xyz =
    {
        .y_ref = y_ref,
        .x_ref = x_ref,
        .y_ref_prev = y_ref,
        .x_ref_prev = x_ref,
        .height_ref = height_ref,
        .width_ref = width_ref,
        .menu = menu,
        .table = table,
        .table_size = COUNTOF(table),
        .pos_prev = 0, /* previous cursor position */
        .max_prev = 0, /* previous menu max */

        .cmdcur_max = cmdcur_max,
        .cmdcur = cmdcur,
        .cmdlen = cmdlen,
        .keep_cmd = false,
        .keyboard_cmd = true,
        .default_cmd = false,
        .item_length = item_length,
        .max_item_length = 0,
        .explan_len_prev = 0,
        .is_moving = false,
        .is_moving_prev = false,
    };

    XO xo =
    {
        .pos = {0},
        .cur_idx = 0,
        .top = 0,
        .max = 0,
        .xyz = &xyz,
        .cb = domenu_cb,
        .recsiz = sizeof(MENU),
    };

    int cmd = XO_INIT;

    for (;;)
    {
        cmd = domenu_exec(&xo, cmd);
        if ((cmd & ~XO_MOVE_MASK) == (XZ_ZONE | XZ_QUIT))
            break;

        /* Redraw on movement */
        if (xyz.y_ref != xyz.y_ref_prev || xyz.x_ref != xyz.x_ref_prev)
        {
            /* Update the current state */
            xyz.x = getx_ref(xyz.x_ref);
            xyz.y = gety_ref(xyz.y_ref);

            /* Set up redraw flags */
            cmd |= XR_BODY;
            if (MENU_NOMOVIE_POS(xyz.y, xyz.x) != MENU_NOMOVIE_POS(gety_ref(xyz.y_ref_prev), getx_ref(xyz.x_ref_prev)))
                cmd |= XR_PART_HEAD | XR_PART_NECK;

            continue;
        }

        cmd = vkey();
        xyz.keyboard_cmd = true;
        xyz.default_cmd = false;

        if (cmd == I_RESIZETERM)
            cmd = XO_HEAD;
    }
}

void domenu_cursor_show(XO *xo)
{
    DomenuXyz *const xyz = (DomenuXyz *)xo->xyz;
    xo->cur_idx %= xo_ncur + 1;

    int ycc, xcc, ycx, xcx;
    if (xyz->height > 0 && xyz->width > 0)
    {
        ycc = xyz->y + (xo->pos[xo->cur_idx] % xyz->height);
        xcc = xyz->x + (xo->pos[xo->cur_idx] / xyz->height * xyz->width);
        ycx = xyz->y + (xyz->pos_prev % xyz->height);
        xcx = xyz->x + (xyz->pos_prev / xyz->height * xyz->width);
    }
    else
    {
        ycc = xyz->y + xo->pos[xo->cur_idx];
        xcc = xyz->x;
        ycx = xyz->y + xyz->pos_prev;
        xcx = xyz->x;
    }
    if (xo->pos[xo->cur_idx] != xyz->pos_prev || xyz->is_moving != xyz->is_moving_prev)
    {
        domenu_redo(xo, XR_PART_KNEE + XO_NONE);
        if (xyz->pos_prev == -1)
        {
            for (int i = 0; i < xo_ncur; ++i)
            {
                if (i == xo->cur_idx)
                    continue;
                int yi, xi;
                if (xyz->height > 0 && xyz->width > 0)
                {
                    yi = xyz->y + (xo->pos[i] % xyz->height);
                    xi = xyz->x + (xo->pos[i] / xyz->height * xyz->width);
                }
                else
                {
                    yi = xyz->y + xo->pos[i];
                    xi = xyz->x;
                }
                cursor_show_mark(xo, yi, xi, xo->pos[i]);
            }
        }
        else
        {
            cursor_bar_clear(xo, ycx, xcx, xyz->width, xo->pos[xo->cur_idx], xyz->pos_prev);
        }
        const int cur_idx = xo->cur_idx;
        if (xyz->is_moving) // HACK: disable highlighting
        {
            xo->cur_idx = XO_NCUR; // an invalid index
            cursor_show_mark(xo, ycc, xcc, xo->pos[cur_idx]);
            xo->cur_idx = cur_idx;
        }
        else
            cursor_bar_show(xo, ycc, xcc, xyz->width, xo->pos[cur_idx]);
        xyz->pos_prev = xo->pos[xo->cur_idx];
        xyz->is_moving_prev = xyz->is_moving;
    }
    else
    {
        move(ycc, xcc + 1);
    }
}

static int
domenu_exec(
    XO *xo,
    int cmd)
{
    DomenuXyz *const xyz = (DomenuXyz *)xo->xyz;

    while (cmd != XO_NONE || (cmd & XO_ZONE_MASK))
    {
        if (!xyz->menu)
        {
            return XO_QUIT;
        }
        if (!xyz->keep_cmd)
        {
            memset(xyz->cmdcur, 0, sizeof(xyz->cmdcur[0]) * xyz->table_size);
            memset(xyz->cmdlen, 0, sizeof(xyz->cmdlen[0]) * xyz->table_size);
        }
        xyz->keep_cmd = false;

        cmd = domenu_redo(xo, cmd);

        /* Menu moving hotkeys */
        switch ((xyz->is_moving) ? cmd : KEY_NONE)
        {
        case KEY_PGUP:
        case KEY_PGDN:
        case KEY_UP:
        case KEY_DOWN:
        case KEY_HOME:
        case KEY_END:
        case KEY_LEFT:
        case KEY_RIGHT:
            if (domenu_move(xo, cmd))
                goto key_handle_end;
            xyz->is_moving = false; // exit the moving mode when the menu failed to move
            break;
        default:
            ;
        }

        /* Invoke hotkey functions for keyboard input only */
        switch ((xyz->keyboard_cmd) ? cmd : KEY_NONE)
        {
        case KEY_KONAMI:
            for (int i = xo_ncur; i < XO_NCUR; ++i)
                xo->pos[i] = xo->pos[xo_ncur - 1];
            xo_ncur = xo_ncur % XO_NCUR + 1;
            xo->cur_idx %= xo_ncur;
            cmd = XO_BODY; // Cursor count changed; redraw
            continue;

        case KEY_TAB:
            if (xo_ncur == 1) // Plain mode
                break;
            xyz->is_moving = !xyz->is_moving;
            break;

        case ' ':
            xyz->is_moving = false;
            xo->cur_idx = (xo->cur_idx + 1) % xo_ncur;
            break;

        case KEY_PGUP:
            if (xyz->height > 0 && xo->pos[xo->cur_idx] - xyz->height >= 0)
                xo->pos[xo->cur_idx] -= xyz->height;
            else
                xo->pos[xo->cur_idx] = (xo->pos[xo->cur_idx] == 0) ? xo->max - 1 : 0;
        break;

        case KEY_PGDN:
            if (xyz->height > 0 && xo->pos[xo->cur_idx] + xyz->height < xo->max)
                xo->pos[xo->cur_idx] += xyz->height;
            else
                xo->pos[xo->cur_idx] = (xo->pos[xo->cur_idx] == xo->max - 1) ? 0 : xo->max - 1;
        break;

        case KEY_DOWN:
            if (++xo->pos[xo->cur_idx] < xo->max)
                break;
            // Else falls through
            //    to wrap around cursor

        case KEY_HOME:
            xo->pos[xo->cur_idx] = 0;
            break;

        case KEY_UP:
            if (--xo->pos[xo->cur_idx] >= 0)
                break;
            // Else falls through
            //    to wrap around cursor

        case KEY_END:
            xo->pos[xo->cur_idx] = xo->max - 1;
            break;

        case KEY_RIGHT:
            if (xyz->height > 0 && xo->pos[xo->cur_idx] + xyz->height < xo->max)
            {
                xo->pos[xo->cur_idx] += xyz->height;
                break;
            }
            // Else falls through
            //    to execute the function

        case '\n':
            {
                MENU *const mptr = xyz->table[xo->pos[xo->cur_idx]];
                unsigned int mmode = mptr->umode;
                int res;
                /* IID.2021-12-02: Helper union for proper handling of `M_ARG` */
                typedef union {
                    MenuItem item;
                    FuncArg funcarg;
                    DlFuncArg dlfuncarg;
                } MItem;
                MItem m =
                    (mmode & M_ARG) ? LISTLIT(MItem){.funcarg = *mptr->item.funcarg} /* Make a copy of the `FuncArg` object */
                    : LISTLIT(MItem){.item = mptr->item};
#if !NO_SO
                /* Thor.990212: dynamic load, with negative umode */
                if (mmode & M_DL(0))
                {
                    if (mmode & M_ARG)
                    {
                        m.funcarg.func = (int (*)(const void *)) DL_GET(m.dlfuncarg.func);
                        if (!m.funcarg.func) break;
  #ifndef DL_HOTSWAP
                        /* Update the `FuncArg` object */
                        mptr->item.funcarg->func = m.funcarg.func;
  #endif
                    }
                    else
                    {
                        m.item.func = (int (*)(void)) DL_GET(m.item.dl.func);
                        if (!m.item.func) break;
  #ifndef DL_HOTSWAP
                        mptr->item.func = m.item.func;
  #endif
                    }
                    mmode &= ~M_DL(0);
  #ifndef DL_HOTSWAP
                    mptr->umode = mmode;
  #endif
                }
#endif
                utmp_mode(mmode & M_MASK /* = mptr->umode*/);

                if ((mmode & M_MASK) >= M_MENU && (mmode & M_MASK) < M_FUN)
                {
                    if (xyz->cmdcur_max == 1)
                        xyz->mtail->level = PERM_MENU + mptr->desc[0];
                    xyz->menu = m.item.menu;
                    cmd = XO_INIT;
                    continue;
                }

                if (mmode & M_ARG)
                {
                    if (mmode & M_XO)
                        res = m.funcarg.xofunc(xo, m.funcarg.arg);
                    else
                        res = m.funcarg.func(m.funcarg.arg);
                }
                else
                {
                    if (mmode & M_XO)
                        res = m.item.xofunc(xo);
                    else
                        res = m.item.func();
                }

                utmp_mode(xyz->mtail->umode);

                if (res >= -1 && res < '\b')  /* Non-xover return value */
                {
                    res = XO_HEAD;
                }
                else if ((res & ~XO_MOVE_MASK) == XR_FOOT)
                {
#if 1
                    /* Thor.980826: 用 outz 就不用 move + clrtoeol了 */
                    outf(footer);
#endif
                    res &= ~XO_MOVE_MASK;
                }

                cmd = res & XO_MOVE_MASK;
                res = (res & ~XO_MOVE_MASK) + XO_NONE;

                switch (res)
                {
                default:
                case XEASY:
                    break;

                case QUIT:
                    return XO_QUIT;

#ifdef  HAVE_CHANGE_SKIN
                case XZ_SKIN + XO_NONE:
                    vmsg("DEBUG:SKIN");
                    vmsg("123");
                    //(*s_menu)();
                    return XO_QUIT;
                    vmsg("1234");
                    break;
#endif
                }

                if (cmd == KEY_NONE)
                    cmd = ' ';
                cmd |= res & ~XO_MOVE_MASK;
                xyz->keyboard_cmd = false;
                continue;
            }

#ifdef EVERY_Z
        case Ctrl('Z'):
            every_Z(xo);       /* Thor: ctrl-Z everywhere */
            cmd = XO_HEAD;
            continue;
#endif
        case Ctrl('U'):
            every_U();
            cmd = XO_HEAD;
            continue;
        case Ctrl('B'):
            every_B();
            cmd = XO_HEAD;
            continue;
        case Ctrl('S'):
        case 'S':
        case 's':
        case '/':
            every_S();
            cmd = XO_HEAD;
            continue;
        case KEY_LEFT:
            if (xyz->height > 0 && xo->pos[xo->cur_idx] - xyz->height >= 0)
            {
                xo->pos[xo->cur_idx] -= xyz->height;
                break;
            }
            // Else falls through
            //    to enter the parent menu

        case KEY_ESC:
        case Meta(KEY_ESC):
        case 'e':
            if (xyz->menu != menu_main || xo_stack_level > 0)
            {
                xyz->mtail->level = PERM_MENU + xyz->table[xo->pos[xo->cur_idx]]->desc[0];
                xyz->menu = xyz->mtail->item.menu;
                cmd = XO_INIT;
                continue;
            }

            cmd = 'G';
            xyz->keyboard_cmd = false;

            // Falls through
            //    to move the cursor to option 'G' ('Goodbye'; exiting BBS)

            /* Command matching */
        default:
            switch (cmd)
            {
            default:
                {
                    int maxlen = 0;

                    cmd = tolower(cmd);

                    /* IID.20200107: Match input sequence. */
                    for (int i = 0; i < xo->max; i++)
                    {
                        const char *const mdesc = xyz->table[i]->desc;
                        const int match_max = str_nlen(mdesc, xyz->cmdcur_max);
                        /* Skip spaces */
                        xyz->cmdcur[i] += strspn(mdesc + xyz->cmdcur[i], " ");
                        /* Not matched or cursor reached the end */
                        if (xyz->cmdcur[i] >= match_max
                            || tolower(mdesc[xyz->cmdcur[i]]) != cmd)
                        {
                            /* Reset and skip spaces */
                            xyz->cmdcur[i] = strspn(mdesc + xyz->cmdcur[0], " ");
                            xyz->cmdlen[i] = 0;
                        }
                        if (tolower(mdesc[xyz->cmdcur[i]]) == cmd)
                        {
                            xyz->cmdcur[i]++;
                            xyz->cmdlen[i]++;
                        }
                        if (xyz->cmdlen[i] > maxlen)
                        {
                            maxlen = xyz->cmdlen[i];
                            if (xyz->default_cmd)
                            {
                                for (int j = 0; j < COUNTOF(xo->pos); ++j)
                                    xo->pos[j] = i;
                            }
                            else
                            {
                                xo->pos[xo->cur_idx] = i;
                            }
                        }
                    }
                }

                // Falls through
                //    to keep the input

            case ' ':  /* Ignore space for matching */
                if (xyz->keyboard_cmd)  /* `cmd` is from keyboard */
                    xyz->keep_cmd = true;
                break;
            case KEY_NONE:
                ; /* Do nothing */
            }
        }
key_handle_end:

        cmd = XO_NONE;
        domenu_cursor_show(xo);
    }
    return XO_NONE;
}
