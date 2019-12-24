/*-------------------------------------------------------*/
/* menu.c       ( NTHU CS MapleBBS Ver 3.00 )            */
/*-------------------------------------------------------*/
/* target : menu/help/movie routines                     */
/* create : 95/03/29                                     */
/* update : 2000/01/02                                   */
/*-------------------------------------------------------*/

#include "bbs.h"

extern BCACHE *bshm;
extern UCACHE *ushm;
int item_length[20]={0};

extern time_t brd_visit[MAXBOARD];

#ifdef  HAVE_INFO
#define INFO_EMPTY      "Info      【 \x1b[1;36m校方公告區\x1b[m 】"
#define INFO_HAVE       "Info      【 \x1b[41;33;1m快進來看看\x1b[m 】"
#endif

#ifdef  HAVE_STUDENT
#define STUDENT_EMPTY   "1Student  【 \x1b[1;36m學生公告區\x1b[m 】"
#define STUDENT_HAVE    "1Student  【 \x1b[41;33;1m快進來看看\x1b[m 】"
#endif

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
history(void)
{
    more(FN_ETC_COUNTER, 0);
    return 0;
}

static int
version(void)
{
    more(FN_ETC_VERSION, 0);
    return 0;
}

static int
today(void)
{
    more("gem/@/@-act", 0);
    return 0;
}

static int
popmax(void)
{
    more("gem/@/@pop", 0);
    return 0;
}

static int
yesterday(void)
{
    more("gem/@/@=act", 0);
    return 0;
}

static int
day(void)
{
    more("gem/@/@-day", 0);
    return 0;
}

static int
week(void)
{
    more("gem/@/@-week", 0);
    return 0;
}

static int
month(void)
{
    more("gem/@/@-month", 0);
    return 0;
}

static int
year(void)
{
    more("gem/@/@-year", 0);
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
    time_t tpad;
    char msg[356];
}      Pad;


int
pad_view(void)
{
    int fd, count;
    Pad *pad;

    if ((fd = open(FN_NOTE_PAD, O_RDONLY)) < 0)
        return XEASY;

    clear();
    move(0, 23);
    outs("\x1b[1;37;45m ●  " BOARDNAME " 留 言 板  ● \n\n");
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

    time(&(pad.tpad));

    str = pad.msg;

    sprintf(str, "\x1b[1;37;46mΥ\x1b[34;47m %s \x1b[33m(%s)", cuser.userid, cuser.username);
    len = strlen(str);
    strcat(str, & " \x1b[30;46m"[len & 1]);

    for (i = len >> 1; i < 41; i++)
        strcat(str, "▄");
    sprintf(str2, "\x1b[34;47m %.14s \x1b[37;46mΥ\x1b[m\n%-70.70s\n%-70.70s\n%-70.70s\n",
        Etime(&(pad.tpad)), buf[0], buf[1], buf[2]);
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
        return XEASY;
    default:
        return XEASY; /* 090911.cache: 不小心按錯不要趕走人家 ;( */
    }

#ifdef  LOG_BMW
    /*bmw_save();*/                   /* lkchu.981201: 熱訊記錄處理 */
#endif

    clear();
    prints("       \x1b[1;31m ●       \x1b[1;36m ┌─┐┌─┐┌─┐┌─╮ ┌─╮┌╮┐┌─┐\n"
        "      \x1b[1;31m●\x1b[1;37m○\x1b[1;33m●\x1b[1;37m═══\x1b[1;36m│  ┬│  ││  ││  │ │ ═ └  ┘│═╡\x1b[1;37m════\n"
        "       \x1b[1;33m ●        \x1b[1;34m└─┤└─┘└─┘└─╯ └─╯ └┘ └─┘\x1b[m\n");
    prints("Dear \x1b[32m%s(%s)\x1b[m，別忘了再度光臨【 %s 】\n"
        "以下是您在站內的註冊資料:\n",
        cuser.userid, cuser.username, str_site);
    acct_show(&cuser, 3);
    vmsg_body(NULL);
    u_exit("EXIT ");
    vkey();
    exit(0);
}


/* ----------------------------------------------------- */
/* help & menu processing                                */
/* ----------------------------------------------------- */

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

    len_ttl = strlen(title);
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
    static int orig_flag = -1;
    static time_t uptime = -1;
    static char flagmsg[16];
    static char datemsg[16];

    int ufo;
    time_t now;

    ufo = cuser.ufo;
    time(&now);

    /* Thor: it is depend on which state */

    if ((bbsmode < M_CLASS) && (cuser.ufo2 & UFO2_MOVIE))
    {
        static int tag = FILM_MOVIE;

        tag = film_out(tag, 2);
    }


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
            & "天一二三四五六"[ptime->tm_wday << 1]);

        uptime = now + 86400 - ptime->tm_hour * 3600 -
            ptime->tm_min * 60 - ptime->tm_sec;
    }
    ufo = (now - (uptime - 86400)) / 60;

    /* Thor.980913: 註解: 最常見呼叫 movie()的時機是每次更新 film, 在 60秒以上,
                          故不需針對 xx:yy 來特別作一字串儲存以加速 */

    sprintf(footer, "\x1b[0;34;46m%s%d:%02d] \x1b[30;47m 目前站上有\x1b[31m%4d\x1b[30m 人，我是 \x1b[31m%-12s\x1b[30m [呼叫/訊息]\x1b[31m%s",
        datemsg, ufo / 60, ufo % 60,
        /*ushm->count*/total_num, cuser.userid, flagmsg);
    outf(footer);
}


#define MENU_CHANG      0x80000000


#define PERM_MENU       PERM_PURGE


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

    {{BanMail}, PERM_BOARD|PERM_SYSOP, M_BANMAIL,
    "FireWall   擋信列表"},

    {{.dlfunc = DL_NAME("adminutil.so", bm_check)}, PERM_BOARD|PERM_SYSOP, M_DL(M_XMODE),
    "Manage     板主確認"},

    {{.dlfunc = DL_NAME("adminutil.so", m_expire)}, PERM_BOARD|PERM_SYSOP, M_DL(M_XMODE),
    "DExpire    清除看板刪除文章"},

    {{.dlfunc = DL_NAME("adminutil.so", mail_to_bm)}, PERM_SYSOP, M_DL(M_XMODE),
    "ToBM       寄信給板主"},

    {{.dlfunc = DL_NAME("adminutil.so", mail_to_all)}, PERM_SYSOP, M_DL(M_XMODE),
    "Alluser    系統通告"},

    {{.dlfunc = DL_NAME("personal.so", personal_admin)}, PERM_BOARD|PERM_SYSOP, M_DL(M_XMODE),
    "Personal   個人板審核"},

    {{.menu = menu_admin}, PERM_MENU + 'N', M_ADMIN,
    "看板總管"}
};

INTERNAL_INIT MENU menu_accadm[] =
{
    {{m_user}, PERM_ACCOUNTS, M_SYSTEM,
    "User       使用者資料"},

    {{.dlfunc = DL_NAME("bank.so", money_back)}, PERM_ACCOUNTS, M_DL(M_XMODE),
    "GetMoney   匯入舊夢幣"},

#ifdef  HAVE_SONG
    {{.dlfunc = DL_NAME("song.so", AddRequestTimes)}, PERM_KTV, M_DL(M_XMODE),
    "Addsongs   增加點歌次數"},
#endif

    {{.dlfunc = DL_NAME("passwd.so", new_passwd)}, PERM_SYSOP, M_DL(M_XMODE),
    "Password   重送新密碼"},

#ifdef  HAVE_REGISTER_FORM
    {{.dlfunc = m_register}, PERM_ACCOUNTS, M_SYSTEM,
    "1Register  審核註冊表單"},
#endif

#ifdef HAVE_OBSERVE_LIST
    {{.dlfunc = DL_NAME("observe.so", Observe_list)}, PERM_SYSOP|PERM_BOARD, M_DL(M_XMODE),
    "2Observe   系統觀察名單"},
#endif

    {{.menu = menu_admin}, PERM_MENU + 'U', M_ADMIN,
    "註冊總管"}
};

INTERNAL_INIT MENU menu_settingadm[] =
{

    {{.dlfunc = DL_NAME("adminutil.so", m_xfile)}, PERM_SYSOP, M_DL(M_XFILES),
    "File       編輯系統檔案"},

    {{.dlfunc = DL_NAME("adminutil.so", m_xhlp)}, PERM_SYSOP, M_DL(M_XFILES),
    "Hlp        編輯說明檔案"},

    {{.dlfunc = DL_NAME("admin.so", Admin)}, PERM_SYSOP, M_DL(M_XMODE),
    "Operator   系統管理員列表"},

    {{.dlfunc = DL_NAME("chatmenu.so", Chatmenu)}, PERM_CHATROOM|PERM_SYSOP, M_DL(M_XMODE),
    "Chatmenu   " CHATROOMNAME "動詞"},

    {{.dlfunc = DL_NAME("violate.so", Violate)}, PERM_SYSOP, M_DL(M_XMODE),
    "Violate    處罰名單"},

    {{.dlfunc = DL_NAME("adminutil.so", special_search)}, PERM_SYSOP, M_DL(M_XMODE),
    "XSpecial   特殊搜尋"},

    {{.dlfunc = DL_NAME("adminutil.so", update_all)}, PERM_SYSOP, M_DL(M_XMODE),
    "Database   系統資料庫更新"},

    {{.menu = menu_admin}, PERM_MENU + 'X', M_ADMIN,
    "系統資料"}
};

/* ----------------------------------------------------- */
/* reset menu                                            */
/* ----------------------------------------------------- */
INTERNAL_INIT MENU menu_reset[] =
{
    {{.dlfunc = DL_NAME("adminutil.so", reset1)}, PERM_BOARD, M_DL(M_XMODE),
    "Camera     動態看板"},

    {{.dlfunc = DL_NAME("adminutil.so", reset2)}, PERM_BOARD, M_DL(M_XMODE),
    "Group      分類群組"},

    {{.dlfunc = DL_NAME("adminutil.so", reset3)}, PERM_SYSOP, M_DL(M_XMODE),
    "Mail       寄信收信轉信"},

    {{.dlfunc = DL_NAME("adminutil.so", reset4)}, PERM_ADMIN, M_DL(M_XMODE),
    "Killbbs    清除不正常 BBS"},

    {{.dlfunc = DL_NAME("adminutil.so", reset5)}, PERM_BOARD, M_DL(M_XMODE),
    "Firewall   擋信列表"},

    {{.dlfunc = DL_NAME("adminutil.so", reset6)}, PERM_CHATROOM, M_DL(M_XMODE),
    "Xchatd     重開聊天室"},

    {{.dlfunc = DL_NAME("adminutil.so", reset7)}, PERM_SYSOP, M_DL(M_XMODE),
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

    {{.dlfunc = DL_NAME("innbbs.so", a_innbbs)}, PERM_BOARD, M_DL(M_SYSTEM),
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

    {{.dlfunc = DL_NAME("contact.so", Contact)}, PERM_INTERNET, M_DL(M_XMODE),
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
    {{.dlfunc = DL_NAME("aloha.so", t_aloha)}, PERM_BASIC, M_DL(M_XMODE),
    "Aloha      上站通知名單"},

    {{t_pager}, PERM_BASIC, M_XMODE,
    "Pager      切換呼叫器"},

    {{t_message}, PERM_BASIC, M_XMODE,
    "Message    切換訊息"},

    {{t_query}, 0, M_QUERY,
    "Query      查詢網友"},

    /* Thor.990220: chatroom client 改採外掛 */
    {{.dlfunc = DL_NAME("chat.so", t_chat)}, PERM_CHAT, M_DL(M_CHAT),
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

    {{popmax}, 0, M_READA,
    "Login      上站次數排行榜"},

    {{today}, 0, M_READA,
    "Today      今日上線人次統計"},

    {{yesterday}, 0, M_READA,
    "Yesterday  昨日上線人次統計"},

    {{day}, 0, M_READA,
    "0Day       本日十大熱門話題"},

    {{week}, 0, M_READA,
    "1Week      本週五十大熱門話題"},

    {{month}, 0, M_READA,
    "2Month     本月百大熱門話題"},

    {{year}, 0, M_READA,
    "3Year      本年度百大熱門話題"},

    {{.menu = menu_xyz}, PERM_MENU + 'L', M_MMENU,
    "統計資料"}
};


INTERNAL_INIT MENU menu_xyz[] =
{
    {{.menu = menu_information}, 0, M_XMENU,
    "Tops       " NICKNAME "排行榜"},

    {{version}, 0, M_READA,
    "Version    源碼發展資訊"},

    {{.dlfunc = DL_NAME("xyz.so", x_siteinfo)}, 0, M_DL(M_READA),
    "Xinfo      系統程式資訊"},

    {{pad_view}, 0, M_READA,
    "Note       觀看心情留言板"},

    {{welcome}, 0, M_READA,
    "Welcome    觀看歡迎畫面"},

    {{history}, 0, M_READA,
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

    {{.dlfunc = DL_NAME("list.so", List)}, PERM_VALID, M_DL(M_XMODE),
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

    {{.dlfunc = DL_NAME("memorandum.so", Memorandum)}, PERM_VALID, M_DL(M_XMODE),
    "Note       備忘錄"},

    {{.dlfunc = DL_NAME("pnote.so", main_note)}, PERM_VALID, M_DL(M_XMODE),
    "PNote      個人答錄機"},

#ifdef  HAVE_CLASSTABLEALERT
    {{.dlfunc = DL_NAME("classtable2.so", main_classtable)}, PERM_VALID, M_DL(M_XMODE),
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
    {{.dlfunc = DL_NAME("bj.so", BlackJack)}, PERM_VALID, M_DL(M_XMODE),
    "BlackJack  " NICKNAME "黑傑克"},

    {{.dlfunc = DL_NAME("guessnum.so", fightNum)}, PERM_VALID, M_DL(M_XMODE),
    "FightNum   數字大決戰"},

    {{.dlfunc = DL_NAME("guessnum.so", guessNum)}, PERM_VALID, M_DL(M_XMODE),
    "GuessNum   傻瓜猜數字"},

    {{.dlfunc = DL_NAME("mine.so", Mine)}, PERM_VALID, M_DL(M_XMODE),
    "Mine       " NICKNAME "踩地雷"},

    {{.dlfunc = DL_NAME("pip.so", p_pipple)}, PERM_VALID, M_DL(M_XMODE),
    "Pip        " NICKNAME "戰鬥雞"},

    {{.menu = menu_service}, PERM_MENU + 'F', M_UMENU,
    "遊戲休閒"}

};

/* ----------------------------------------------------- */
/* yzu menu                                              */
/* ----------------------------------------------------- */

INTERNAL_INIT MENU menu_special[] =
{

    {{.dlfunc = DL_NAME("personal.so", personal_apply)}, PERM_VALID, M_DL(M_XMODE),
    "PBApply      申請個人看板"},

    {{.dlfunc = DL_NAME("bank.so", bank_main)}, PERM_VALID, M_DL(M_XMODE),
    "Bank       　銀行"},

    {{.dlfunc = DL_NAME("shop.so", shop_main)}, PERM_VALID, M_DL(M_XMODE),
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
    {{.dlfunc = DL_NAME("song.so", XoSongMain)}, PERM_VALID, M_DL(M_XMODE),
    "Request       點歌歌本"},

    {{.dlfunc = DL_NAME("song.so", XoSongLog)}, PERM_VALID, M_DL(M_XMODE),
    "OrderSongs    點歌紀錄"},

    {{.dlfunc = DL_NAME("song.so", XoSongSub)}, PERM_VALID, M_DL(M_XMODE),
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

    {{.dlfunc = DL_NAME("newboard.so", XoNewBoard)}, PERM_VALID, M_DL(M_XMODE),
    "Cosign    【 連署申請區 】"},

    {{.dlfunc = DL_NAME("vote.so", SystemVote)}, PERM_POST, M_DL(M_XMODE),
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
    skin = 1;
    vmsg("DEBUG:DreamBBS");
    return 0;
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

    {{goodbye}, 0, M_XMODE,
    "Goodbye   【再別" BOARDNAME "】"},

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
        return KEY_NONE;
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
    return KEY_NONE;
}


#ifdef __cplusplus
namespace {
#endif
INTERNAL_INIT MENU menu_treat[] =
{
    {{goodbye1}, 0, M_XMODE,
    "Goodbye   【再別" NICKNAME "】"},

    {{NULL}, PERM_MENU + 'G', M_MMENU,
    "主功\能表"}
};
#ifdef __cplusplus
}  // namespace
#endif
#endif  /* #ifdef  TREAT */

GCC_PURE static
int count_len(
    const char *data)
{
    int len;
    const char *ptr, *tmp;
    ptr = data;
    len = strlen(data);

    while (ptr)
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


const char *
check_info(const char *input)
{
#if defined(HAVE_INFO) || defined(HAVE_STUDENT)
    const BRD *brd;
#endif
    const char *name = NULL;
    name = input;
#ifdef  HAVE_INFO
    if (!strcmp(input, INFO_EMPTY))
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
    if (!strcmp(input, STUDENT_EMPTY))
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

    return name;
}


void
menu(void)
{
    MENU *menu, *mptr, *table[17];
    unsigned int level, mode;
    int cc=0, cx=0, refilm=0;   /* current / previous cursor position */
    int max=0, mmx;                     /* current / previous menu max */
    int cmd=0, depth, count;
    char item[60];
    const char *str;

    mode = MENU_LOAD | MENU_DRAW | MENU_FILM;
#ifdef  TREAT
    menu = menu_treat;
#else
    menu = menu_main;
#endif
    depth = mmx = 0;

    for (;;)
    {
        level = cuser.userlevel;

        if (mode & MENU_LOAD)
        {
            for (max = -1;; menu++)
            {
                cc = menu->level;
                if (cc & PERM_MENU)
                {

#ifdef  MENU_VERBOSE
                    if (max < 0)                /* 找不到適合權限之功能，回上一層功能表 */
                    {
                        menu = menu->menu;
                        continue;
                    }
#endif

                    break;
                }
                if (cc && !(cc & level))
                    continue;
                if (!strncmp(menu->desc, OPT_OPERATOR, strlen(OPT_OPERATOR)) && !(supervisor || !str_cmp(cuser.userid, ELDER) || !str_cmp(cuser.userid, STR_SYSOP)))
                    continue;

                table[++max] = menu;
            }

            mmx = BMAX(mmx, max);

#ifndef TREAT
            if ((depth == 0) && (cutmp->ufo & UFO_BIFF))
                cmd = 'M';
            else if ((depth == 0) && (cutmp->ufo & UFO_BIFFN))
                cmd = 'U';
            else
#endif
                cmd = cc ^ PERM_MENU;   /* default command */
            utmp_mode(menu->umode);
        }

        if (mode & MENU_DRAW)
        {
            if (mode & MENU_FILM)
            {
                clear();
                refilm = 1;
            }
            vs_head(menu->desc, NULL);
            //prints("\n\x1b[30;47m     選項         選項說明                         動態看板                   \x1b[m\n");
            mode = 0;
            count = 0;
            while (count<20)
                item_length[count++] = 0;
            do
            {
                move(MENU_YPOS + mode, MENU_XPOS + 2);
                if (mode <= max)
                {
                    mptr = table[mode];
                    str = check_info(mptr->desc);
                    sprintf(item, "\x1b[m(\x1b[1;36m%c\x1b[m)%s", *str, str+1);
                    outs(item);
                    item_length[mode]=(cuser.ufo2 & UFO2_COLOR) ? strlen(item)-count_len(str)-2 : 0;
                    /*outs("(\x1b[1;36m");
                    outc(*str++);
                    outs("\x1b[m)");
                    outs(str);*/
                }
                clrtoeol();
            } while (++mode <= mmx);
            if (refilm)
            {
                movie();
                cx = -1;
                refilm = 0;
            }

            mmx = max;
            mode = 0;
        }

        switch (cmd)
        {
        case KEY_PGUP:
            cc = (cc == 0) ? max : 0;
        break;

        case KEY_PGDN:
            cc = (cc == max) ? 0 : max;
        break;

        case KEY_DOWN:
            if (++cc <= max)
                break;
            // Else falls through
            //    to wrap around cursor

        case KEY_HOME:
            cc = 0;
            break;

        case KEY_UP:
            if (--cc >= 0)
                break;
            // Else falls through
            //    to wrap around cursor

        case KEY_END:
            cc = max;
            break;

        case '\n':
        case KEY_RIGHT:
            mptr = table[cc];
            cmd = mptr->umode;
#if !NO_SO
            /* Thor.990212: dynamic load, with negative umode */
            if (cmd < 0)
            {
                void *p = DL_GET(mptr->dlfunc);
                if (!p) break;
                mptr->func = (int (*)(void))p;
                cmd = -cmd;
                mptr->umode = cmd;
            }
#endif
            utmp_mode(cmd /* = mptr->umode*/);

            if (cmd <= M_XMENU)
            {
                menu->level = PERM_MENU + mptr->desc[0];
                menu = mptr->menu;
                mode = MENU_LOAD | MENU_DRAW | MENU_FILM;
                depth++;
                continue;
            }

            mode = (*mptr->func) ();

#ifdef  HAVE_CHANGE_SKIN
            if (skin)
            {
                vmsg("DEBUG:SKIN");
                vmsg("123");
                //(*s_menu)();
                return;
                vmsg("1234");
            }

#endif

#ifdef  TREAT
            if (mode == KEY_NONE)
            {
                menu = menu_main;
                mode = MENU_LOAD | MENU_DRAW | MENU_FILM;
                continue;
            }
#endif

            utmp_mode(menu->umode);


            if (mode == XEASY)
            {
#if 1
                /* Thor.980826: 用 outz 就不用 move + clrtoeol了 */
                outf(footer);
#endif

                mode = 0;
            }
            else
            {
                mode = MENU_DRAW | MENU_FILM;
            }

            cmd = mptr->desc[0];
            continue;

#ifdef EVERY_Z
        case Ctrl('Z'):
            every_Z();          /* Thor: ctrl-Z everywhere */
            goto menu_key;
#endif
        case Ctrl('U'):
            every_U();
            break;
        case Ctrl('B'):
            every_B();
            break;
        case Ctrl('S'):
            every_S();
            break;
        case 's':
            every_S();
            break;
        case KEY_LEFT:
        case 'e':
            if (depth > 0)
            {
                menu->level = PERM_MENU + table[cc]->desc[0];
                menu = menu->menu;
                mode = MENU_LOAD | MENU_DRAW | MENU_FILM;
                depth--;
                continue;
            }

            cmd = 'G';

            // Falls through
            //    to move the cursor to option 'G' ('Goodbye'; exiting BBS)

        default:

            if (cmd >= 'a' && cmd <= 'z')
                cmd -= 0x20;

            cc = 0;
            for (;;)
            {
                if (table[cc]->desc[0] == cmd)
                    break;
                if (++cc > max)
                {
                    cc = cx;
                    goto menu_key;
                }
            }
        }

        if (cc != cx)
        {
            if (cx >= 0)
            {
                move(MENU_YPOS + cx, MENU_XPOS);
                outc(' ');
            }
            move(MENU_YPOS + cc, MENU_XPOS);
            outc('>');
            cx = cc;
        }
        else
        {
            move(MENU_YPOS + cc, MENU_XPOS + 1);
        }

menu_key:

        cmd = vkey();
    }
}

