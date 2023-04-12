/*-------------------------------------------------------*/
/* menu.c       ( NTHU CS MapleBBS Ver 3.00 )            */
/*-------------------------------------------------------*/
/* target : menu/help/movie routines                     */
/* create : 95/03/29                                     */
/* update : 2000/01/02                                   */
/*-------------------------------------------------------*/

#include "bbs.h"

#ifdef  HAVE_INFO
#define INFO_EMPTY      "Info      �i \x1b[1;36m�դ褽�i��\x1b[m �j"
#define INFO_HAVE       "Info      �i \x1b[41;33;1;5m�ֶi�Ӭݬ�\x1b[m �j"
#endif

#ifdef  HAVE_STUDENT
#define STUDENT_EMPTY   "1Student  �i \x1b[1;36m�ǥͤ��i��\x1b[m �j"
#define STUDENT_HAVE    "1Student  �i \x1b[41;33;1;5m�ֶi�Ӭݬ�\x1b[m �j"
#endif

#define GOODBYE_EXIT    "Goodbye   �i�A�O" BOARDNAME "�j"
#define GOODBYE_GOBACK  "GoBack    �i �^�W�h��� �j"

#define MENU_HELP       NULL
#define MENU_HELP_NCUR  "(Tab) �}����沾�ʼҦ��F(Space) �����ϥΤ����" // Multiple-cursor mode

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
    sprintf(buf, "�t�έt�� %.2f %.2f %.2f / %ld", load[0], load[1], load[2], nproc);
    vmsg(buf);
    return XEASY;
}
#endif


/* ----------------------------------------------------- */
/* ���} BBS ��                                           */
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
    outs("\x1b[1;37;45m ��  " BOARDNAME " �d �� �O  �� \x1b[m\n\n");
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
            outs("�Ы� [SPACE] �~���[��A�Ϋ���L�䵲���G");
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
        outs("\n�Яd�� (�ܦh�T��)�A��[Enter]����");
        for (i = 0; (i < 3) &&
            vget(16 + i, 0, "�G", buf[i], 70, DOECHO); i++);
        cc = vans("(S)�s���[�� (E)���s�ӹL (Q)��F�H[S] ");
        if (cc == 'q' || i == 0)
            return;
    } while (cc == 'e');

    time32(&(pad.tpad));

    str = pad.msg;

    sprintf(str, "\x1b[1;37;46m�W\x1b[34;47m %s \x1b[33m(%s)", cuser.userid, cuser.username);
    len = strlen(str);
    strcat(str, & " \x1b[30;46m"[len % 2U]);

    for (i = len >> 1; i < 41; i++)
        strcat(str, "�e");
    sprintf(str2, "\x1b[34;47m %.14s \x1b[37;46m�W\x1b[m\n%-70.70s\n%-70.70s\n%-70.70s\n",
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
        if (!(ans = vans("G)�A�O" NICKNAME " M)���i���� N)�d���O Q)�����H[Q] ")))
            ans = 'q';
    }
    else
        ans = vans("G)�A�O" NICKNAME " M)���i���� N)�d���O Q)�����H[Q] ");

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
        if (HAS_PERM(PERM_POST)) /* Thor.990118: �n��post�~��d��, �������e */
            pad_draw();
        break;

    case 'q':
    default: /* 090911.cache: ���p�߫������n�����H�a ;( */
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
        mid = NEWMAILMSG; // �A���s����
        len = 15;
    }
    else if (ufo & UFO_BIFFN)
    {
        mid = NEWPASSMSG; // �A���s�d��
        len = 15;
    }

    spc = b_cols - len; /* spc: �����ٳѤU�h�����Ŷ� */
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
    int color = 44; //090911.cache: �Ӫ�F�T�w�@���C��
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
        mid = NEWMAILMSG; // �A���s����
        len = 15;
    }
    else if (ufo & UFO_BIFFN)
    {
        mid = NEWPASSMSG; // �A���s�d��
        len = 15;
    }

    spc = b_cols - 14 - len - strlen(currboard); /* spc: �����ٳѤU�h�����Ŷ� */
    len_ttl = BMIN(len_ttl, spc); /* Truncate `title` if too long */
    spc -= len_ttl; /* �\�� title �H��A�����٦� spc ��Ŷ� */
    pad = BMAX(((b_cols - len) >> 1) - (len_ttl + 5), 0); /* pad: Spaces needed to center `mid` */

#ifdef  COLOR_HEADER
    prints("\x1b[1;%2d;37m�i%.*s�j%*s \x1b[33m%s\x1b[1;%2d;37m%*s \x1b[37m�ݪO�m%s�n\x1b[m\n",
        color, len_ttl, title, pad, "", mid, color, spc - pad, "", currboard);
#else
    prints("\x1b[1;46;37m�i%.*s�j%*s \x1b[33m%s\x1b[46m%*s \x1b[37m�ݪO�m%s�n\x1b[m\n",
        len_ttl, title, pad, "", mid, spc - pad, "", currboard);
#endif
}


void clear_notification(void)
{
    unsigned int ufo = cutmp->ufo;

    if (ufo & UFO_BIFF)
        cutmp->ufo = ufo ^ UFO_BIFF;     /* �ݹL�N�� */
    if (ufo & UFO_BIFFN)
        cutmp->ufo = ufo ^ UFO_BIFFN;     /* �ݹL�N�� */
}


/* ------------------------------------- */
/* �ʵe�B�z                              */
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

    /* Thor: �P�� ��� �I�s�� �n�ͤW�� ���� */

    ufo &= UFO_PAGER | UFO_CLOAK | UFO_QUIET | UFO_PAGER1 | UFO_MESSAGE;
    if (orig_flag != ufo)
    {
        orig_flag = ufo;
        sprintf(flagmsg,
            "%s/%s",
            (ufo & UFO_PAGER1) ? "����" : (ufo & UFO_PAGER) ? "�b��" : "���}",
            (ufo & UFO_MESSAGE) ? "����" : (ufo & UFO_QUIET) ? "�b��" : "���}");
    }

    if (now > uptime)
    {
        struct tm *ptime;

        ptime = localtime(&now);
        sprintf(datemsg, "[%d/%d �P��%.2s ",
            ptime->tm_mon + 1, ptime->tm_mday,
            & "�Ѥ@�G�T�|����"[2 * ptime->tm_wday]);

        uptime = now + 86400 - ptime->tm_hour * 3600 -
            ptime->tm_min * 60 - ptime->tm_sec;
    }
    ufo = (now - (uptime - 86400)) / 60;

    /* Thor.980913: ����: �̱`���I�s movie()���ɾ��O�C����s film, �b 60��H�W,
                          �G���ݰw�� xx:yy �ӯS�O�@�@�r���x�s�H�[�t */

    sprintf(footer, "\x1b[0;34;46m%s%d:%02d] \x1b[30;47m �ثe���W��\x1b[31m%4d\x1b[30m �H�A�ڬO \x1b[31m%-*s\t\x1b[30m [�I�s/�T��]\x1b[31m%s",
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
    "NewBoard   �}�P�s�ݪO"},

    {{a_editbrd}, PERM_BOARD, M_SYSTEM,
    "ConfigBrd  �]�w�ݪO"},

    {{m_bmset}, PERM_BOARD, M_SYSTEM,
    "BMset      �]�w���D�v��"},

    {{.dl = {DL_NAME("banmail.so", BanMail)}}, PERM_BOARD|PERM_SYSOP, M_DL(M_BANMAIL),
    "FireWall   �׫H�C��"},

    {{.dl = {DL_NAME("adminutil.so", bm_check)}}, PERM_BOARD|PERM_SYSOP, M_DL(M_XMODE),
    "Manage     �O�D�T�{"},

    {{.dl = {DL_NAME("adminutil.so", m_expire)}}, PERM_BOARD|PERM_SYSOP, M_DL(M_XMODE),
    "DExpire    �M���ݪO�R���峹"},

    {{.dl = {DL_NAME("adminutil.so", mail_to_bm)}}, PERM_SYSOP, M_DL(M_XMODE),
    "ToBM       �H�H���O�D"},

    {{.dl = {DL_NAME("adminutil.so", mail_to_all)}}, PERM_SYSOP, M_DL(M_XMODE),
    "Alluser    �t�γq�i"},

    {{.dl = {DL_NAME("personal.so", personal_admin)}}, PERM_BOARD|PERM_SYSOP, M_DL(M_XMODE),
    "Personal   �ӤH�O�f��"},

    {{.menu = menu_admin}, PERM_MENU + 'N', M_ADMIN,
    "�ݪO�`��"}
};

INTERNAL_INIT MENU menu_accadm[] =
{
    {{m_user}, PERM_ACCOUNTS, M_SYSTEM,
    "User       �ϥΪ̸��"},

    {{.dl = {DL_NAME("bank.so", money_back)}}, PERM_ACCOUNTS, M_DL(M_XMODE),
    "GetMoney   �פJ�¹ڹ�"},

#ifdef  HAVE_SONG
    {{.dl = {DL_NAME("song.so", AddRequestTimes)}}, PERM_KTV, M_DL(M_XMODE),
    "Addsongs   �W�[�I�q����"},
#endif

    {{.dl = {DL_NAME("passwd.so", new_passwd)}}, PERM_SYSOP, M_DL(M_XMODE),
    "Password   ���e�s�K�X"},

#ifdef  HAVE_REGISTER_FORM
    {m_register}, PERM_ACCOUNTS, M_SYSTEM,
    "1Register  �f�ֵ��U���"},
#endif

#ifdef HAVE_OBSERVE_LIST
    {{.dl = {DL_NAME("observe.so", Observe_list)}}, PERM_SYSOP|PERM_BOARD, M_DL(M_XMODE),
    "2Observe   �t���[��W��"},
#endif

    {{.menu = menu_admin}, PERM_MENU + 'U', M_ADMIN,
    "���U�`��"}
};

INTERNAL_INIT MENU menu_settingadm[] =
{

    {{.dl = {DL_NAME("adminutil.so", m_xfile)}}, PERM_SYSOP, M_DL(M_XFILES),
    "File       �s��t���ɮ�"},

    {{.dl = {DL_NAME("adminutil.so", m_xhlp)}}, PERM_SYSOP, M_DL(M_XFILES),
    "Hlp        �s�軡���ɮ�"},

    {{.dl = {DL_NAME("admin.so", Admin)}}, PERM_SYSOP, M_DL(M_XMODE),
    "Operator   �t�κ޲z���C��"},

    {{.dl = {DL_NAME("chatmenu.so", Chatmenu)}}, PERM_CHATROOM|PERM_SYSOP, M_DL(M_XMODE),
    "Chatmenu   " CHATROOMNAME "�ʵ�"},

    {{.dl = {DL_NAME("violate.so", Violate)}}, PERM_SYSOP, M_DL(M_XMODE),
    "Violate    �B�@�W��"},

    {{.dl = {DL_NAME("adminutil.so", special_search)}}, PERM_SYSOP, M_DL(M_XMODE),
    "XSpecial   �S��j�M"},

    {{.dl = {DL_NAME("adminutil.so", update_all)}}, PERM_SYSOP, M_DL(M_XMODE),
    "Database   �t�θ�Ʈw��s"},

    {{.menu = menu_admin}, PERM_MENU + 'X', M_ADMIN,
    "�t�θ��"}
};

/* ----------------------------------------------------- */
/* reset menu                                            */
/* ----------------------------------------------------- */
INTERNAL_INIT MENU menu_reset[] =
{
    {{DLFUNCARG(DL_NAME("adminutil.so", m_resetsys), 1)}, PERM_BOARD, M_DL(M_XMODE) | M_ARG,
    "Camera     �ʺA�ݪO"},

    {{DLFUNCARG(DL_NAME("adminutil.so", m_resetsys), 2)}, PERM_BOARD, M_DL(M_XMODE) | M_ARG,
    "Group      �����s��"},

    {{DLFUNCARG(DL_NAME("adminutil.so", m_resetsys), 3)}, PERM_SYSOP, M_DL(M_XMODE) | M_ARG,
    "Mail       �H�H���H��H"},

    {{DLFUNCARG(DL_NAME("adminutil.so", m_resetsys), 4)}, PERM_ADMIN, M_DL(M_XMODE) | M_ARG,
    "Killbbs    �M�������` BBS"},

    {{DLFUNCARG(DL_NAME("adminutil.so", m_resetsys), 5)}, PERM_BOARD, M_DL(M_XMODE) | M_ARG,
    "Firewall   �׫H�C��"},

    {{DLFUNCARG(DL_NAME("adminutil.so", m_resetsys), 6)}, PERM_CHATROOM, M_DL(M_XMODE) | M_ARG,
    "Xchatd     ���}��ѫ�"},

    {{DLFUNCARG(DL_NAME("adminutil.so", m_resetsys), 7)}, PERM_SYSOP, M_DL(M_XMODE) | M_ARG,
    "All        ����"},

    {{.menu = menu_admin}, PERM_MENU + 'K', M_ADMIN,
    "�t�έ��m"}
};


/* ----------------------------------------------------- */
/* administrator's maintain menu                         */
/* ----------------------------------------------------- */


INTERNAL_INIT MENU menu_admin[] =
{

    {{.menu = menu_accadm}, PERM_ADMIN, M_ADMIN,
    "Acctadm    ���U�`�ޥ\\��"},

    {{.menu = menu_boardadm}, PERM_BOARD, M_ADMIN,
    "Boardadm   �ݪO�`�ޥ\\��"},

    {{.menu = menu_settingadm}, PERM_ADMIN, M_ADMIN,
    "Data       �t�θ�Ʈw�]�w"},

    {{.dl = {DL_NAME("innbbs.so", a_innbbs)}}, PERM_BOARD, M_DL(M_SYSTEM),
    "InnBBS     ��H�]�w"},

    {{.menu = menu_reset}, PERM_ADMIN, M_ADMIN,
    "ResetSys   ���m�t��"},

#ifdef  HAVE_ADM_SHELL
    {{x_csh}, PERM_SYSOP, M_SYSTEM,
    "Shell      ����t�� Shell"},
#endif

#ifdef  HAVE_REPORT
    {{m_trace}, PERM_SYSOP, M_SYSTEM,
    "Trace      �]�w�O�_�O��������T"},
#endif

    {{.menu = menu_main}, PERM_MENU + 'A', M_ADMIN,
    "�t�κ��@"}
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
        vmsg("�z���H�c�Q��F�I");
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
    "Read       �\\Ū�H��"},

    {{m_send}, PERM_INTERNET, M_SMAIL,
    "MailSend   �����H�H"},

#ifdef MULTI_MAIL  /* Thor.981009: ����R�����B�H */
    {{mail_list}, PERM_INTERNET, M_SMAIL,
    "Group      �s�ձH�H"},
#endif

    {{.dl = {DL_NAME("contact.so", Contact)}}, PERM_INTERNET, M_DL(M_XMODE),
    "Contact    �p���W��"},

    {{m_setforward}, PERM_INTERNET, M_SMAIL,
    "AutoFor    �����H�۰���H"},

    {{m_setmboxdir}, PERM_INTERNET, M_SMAIL,
    "Fixdir     ���ثH�c����"},

#ifdef HAVE_DOWNLOAD
    {{m_zip}, PERM_VALID, M_XMODE,
    "Zip        ���]�U���ӤH���"},
#endif
/*
#ifdef HAVE_SIGNED_MAIL
    {{m_verify}, PERM_VALID, M_XMODE,
    "Verify     ���ҫH��q�lñ��"},
#endif
*/
    {{mail_sysop}, PERM_BASIC, M_SMAIL,
    "Yes Sir!   �H�H������"},

    {{.menu = menu_main}, PERM_MENU + 'R', M_MMENU,       /* itoc.020829: �� guest �S�ﶵ */
    "�q�l�l��"}
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
    "Users      ������Ѥ�U"},

    {{t_pal}, PERM_BASIC, M_PAL,
    "Friend     �s��n�ͦW��"},

#ifdef  HAVE_BANMSG
    {{t_banmsg}, PERM_BASIC, M_XMODE,
    "Banmsg     �ڦ��T���W��"},
#endif
    {{.dl = {DL_NAME("aloha.so", t_aloha)}}, PERM_BASIC, M_DL(M_XMODE),
    "Aloha      �W���q���W��"},

    {{t_pager}, PERM_BASIC, M_XMODE,
    "Pager      �����I�s��"},

    {{t_message}, PERM_BASIC, M_XMODE,
    "Message    �����T��"},

    {{t_query}, 0, M_QUERY,
    "Query      �d�ߺ���"},

    /* Thor.990220: chatroom client ��ĥ~�� */
    {{.dl = {DL_NAME("chat.so", t_chat)}}, PERM_CHAT, M_DL(M_CHAT),
    "ChatRoom   " NICKNAME CHATROOMNAME},

    {{t_recall}, PERM_BASIC, M_XMODE,
    "Write      �^�U�e�X�����T"},

#ifdef LOGIN_NOTIFY
    {{t_loginNotify}, PERM_PAGE, M_XMODE,
    "Notify     �]�w�t�κ��ͨ�M"},
#endif
    {{.menu = menu_main}, PERM_MENU + 'U', M_UMENU,
    "�𶢲��"}
};


/* ----------------------------------------------------- */
/* System menu                                           */
/* ----------------------------------------------------- */

INTERNAL_INIT MENU menu_information[] =
{

    {{FUNCARG(menumore, "gem/@/@pop")}, 0, M_READA | M_ARG,
    "Login      �W�����ƱƦ�]"},

    {{FUNCARG(menumore, "gem/@/@-act")}, 0, M_READA | M_ARG,
    "Today      ����W�u�H���έp"},

    {{FUNCARG(menumore, "gem/@/@=act")}, 0, M_READA | M_ARG,
    "Yesterday  �Q��W�u�H���έp"},

    {{FUNCARG(menumore, "gem/@/@-day")}, 0, M_READA | M_ARG,
    "0Day       ����Q�j�������D"},

    {{FUNCARG(menumore, "gem/@/@-week")}, 0, M_READA | M_ARG,
    "1Week      ���g���Q�j�������D"},

    {{FUNCARG(menumore, "gem/@/@-month")}, 0, M_READA | M_ARG,
    "2Month     ����ʤj�������D"},

    {{FUNCARG(menumore, "gem/@/@-year")}, 0, M_READA | M_ARG,
    "3Year      ���~�צʤj�������D"},

    {{.menu = menu_xyz}, PERM_MENU + 'L', M_MMENU,
    "�έp���"}
};


INTERNAL_INIT MENU menu_xyz[] =
{
    {{.menu = menu_information}, 0, M_XMENU,
    "Tops       " NICKNAME "�Ʀ�]"},

    {{FUNCARG(menumore, FN_ETC_VERSION)}, 0, M_READA | M_ARG,
    "Version    ���X�o�i��T"},

    {{.dl = {DL_NAME("xyz.so", x_siteinfo)}}, 0, M_DL(M_READA),
    "Xinfo      �t�ε{����T"},

    {{pad_view}, 0, M_READA,
    "Note       �[�ݤ߱��d���O"},

    {{welcome}, 0, M_READA,
    "Welcome    �[���w��e��"},

    {{FUNCARG(menumore, FN_ETC_COUNTER)}, 0, M_READA | M_ARG,
    "History    �������v�y��"},

    {{.menu = menu_main}, PERM_MENU + 'T', M_SMENU,
    "�t�θ�T"}
};

/* ----------------------------------------------------- */
/* User menu                                             */
/* ----------------------------------------------------- */

INTERNAL_INIT MENU menu_reg[] =
{

    {{u_info}, PERM_BASIC, M_XMODE,
    "Info       �]�w�ӤH��ƻP�K�X"},

    {{u_addr}, PERM_BASIC, M_XMODE,
    "Address    ��g�q�l�H�c�λ{��"},

    {{u_verify}, PERM_BASIC, M_UFILES,
    "Verify     ��g�m���U�{�ҽX�n"},

#ifdef  HAVE_REGISTER_FORM
    {{u_register}, PERM_BASIC, M_UFILES,
    "Register   ��g�m���U�ӽг�n"},
#endif

    {{u_setup}, PERM_VALID, M_UFILES,
    "Mode       �]�w�ާ@�Ҧ�"},

    {{ue_setup}, 0, M_UFILES,
    "Favorite   �ӤH�ߦn�]�w"},

    {{u_xfile}, PERM_BASIC, M_UFILES,
    "Xfile      �s��ӤH�ɮ�"},

    {{.dl = {DL_NAME("list.so", List)}}, PERM_VALID, M_DL(M_XMODE),
    "1List      �s�զW��"},

    {{.menu = menu_user}, PERM_MENU + 'I', M_MMENU,
    "���U��T"}
};


INTERNAL_INIT MENU menu_user[] =
{
    {{.menu = menu_reg}, 0, M_XMENU,
    "Configure  ���U�γ]�w�ӤH��T"},

    {{u_lock}, PERM_BASIC, M_XMODE,
    "Lock       ��w�ù�"},

    {{.dl = {DL_NAME("memorandum.so", Memorandum)}}, PERM_VALID, M_DL(M_XMODE),
    "Note       �Ƨѿ�"},

    {{.dl = {DL_NAME("pnote.so", main_note)}}, PERM_VALID, M_DL(M_XMODE),
    "PNote      �ӤH������"},

#ifdef  HAVE_CLASSTABLEALERT
    {{.dl = {DL_NAME("classtable2.so", main_classtable)}}, PERM_VALID, M_DL(M_XMODE),
    "2Table     �s���ӤH�\\�Ҫ�"},
#endif

    {{view_login_log}, PERM_BASIC, M_READA,
    "ViewLog    �˵��W������"},

    {{.menu = menu_service}, PERM_MENU + 'C', M_UMENU,
    "�ӤH�]�w"}
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
    "BlackJack  " NICKNAME "�³ǧJ"},

    {{.dl = {DL_NAME("guessnum.so", fightNum)}}, PERM_VALID, M_DL(M_XMODE),
    "FightNum   �Ʀr�j�M��"},

    {{.dl = {DL_NAME("guessnum.so", guessNum)}}, PERM_VALID, M_DL(M_XMODE),
    "GuessNum   �̥ʲq�Ʀr"},

    {{.dl = {DL_NAME("mine.so", Mine)}}, PERM_VALID, M_DL(M_XMODE),
    "Mine       " NICKNAME "��a�p"},

    {{.dl = {DL_NAME("pip.so", p_pipple)}}, PERM_VALID, M_DL(M_XMODE),
    "Pip        " NICKNAME "�԰���"},

    {{.menu = menu_service}, PERM_MENU + 'F', M_UMENU,
    "�C����"}

};

/* ----------------------------------------------------- */
/* yzu menu                                              */
/* ----------------------------------------------------- */

INTERNAL_INIT MENU menu_special[] =
{

    {{.dl = {DL_NAME("personal.so", personal_apply)}}, PERM_VALID, M_DL(M_XMODE),
    "PBApply      �ӽЭӤH�ݪO"},

    {{.dl = {DL_NAME("bank.so", bank_main)}}, PERM_VALID, M_DL(M_XMODE),
    "Bank       �@�Ȧ�"},

    {{.dl = {DL_NAME("shop.so", shop_main)}}, PERM_VALID, M_DL(M_XMODE),
    "Pay        �@�ө�"},

#ifdef HAVE_SONG
    {{.menu = menu_song}, PERM_VALID, M_XMENU,
    "Request      �I�q�t��"},
#endif

    {{resetbrd}, PERM_ADMIN, M_XMODE,
    "CameraReset  �������]"},

    {{.menu = menu_service}, PERM_MENU + 'R', M_UMENU,
    "�[�ȪA��"}
};



/* ----------------------------------------------------- */
/* song menu                                             */
/* ----------------------------------------------------- */

#ifdef HAVE_SONG
INTERNAL_INIT MENU menu_song[] =
{
    {{.dl = {DL_NAME("song.so", XoSongMain)}}, PERM_VALID, M_DL(M_XMODE),
    "Request       �I�q�q��"},

    {{.dl = {DL_NAME("song.so", XoSongLog)}}, PERM_VALID, M_DL(M_XMODE),
    "OrderSongs    �I�q����"},

    {{.dl = {DL_NAME("song.so", XoSongSub)}}, PERM_VALID, M_DL(M_XMODE),
    "Submit        ��Z�M��"},

    {{.menu = menu_special}, PERM_MENU + 'R', M_XMENU,
    "���I�I�q"}
};
#endif


/* ----------------------------------------------------- */
/* service menu                                          */
/* ----------------------------------------------------- */

/* Thor.990224: �}��~���ɭ� */
INTERNAL_INIT MENU menu_service[] =
{

    {{.menu = menu_user}, 0, M_UMENU,
    "User      �i �ӤH�u��� �j"},

    {{.menu = menu_special}, PERM_VALID, M_XMENU,
    "Bonus     �i �[�ȪA�Ȱ� �j"},

    {{.menu = menu_game}, PERM_VALID, M_XMENU,
    "Game      �i �C������� �j"},

#ifdef  HAVE_INFO
    {{Information}, 0, M_BOARD,
    INFO_EMPTY},
#endif

#ifdef  HAVE_STUDENT
    {{Student}, 0, M_BOARD,
    STUDENT_EMPTY},
#endif

/* 091007.cache: �ԤH�鲼�S�N�q... */

    {{.dl = {DL_NAME("newboard.so", XoNewBoard)}}, PERM_VALID, M_DL(M_XMODE),
    "Cosign    �i �s�p�ӽа� �j"},

    {{.dl = {DL_NAME("vote.so", SystemVote)}}, PERM_POST, M_DL(M_XMODE),
    "Vote      �i �t�Χ벼�� �j"},

    {{system_result}, 0, M_READA,
    "Result    �i�t�Χ벼���G�j"},

/*
#ifdef HAVE_SONG
    {{.menu = menu_song}, PERM_VALID, M_XMENU,
    "Song      �i  �I�q�t�ΰ�  �j"},
#endif
*/
    {{.menu = menu_main}, PERM_MENU + 'U', M_UMENU,
     NICKNAME "�A��"}
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
    "DreamBBS   �w�]���t��"},

    {{.menu = menu_main}, PERM_MENU + 'W', M_MMENU,
    "�������"}
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
    "0Admin    �i �t�κ��@�� �j"},

    {{Gem}, 0, M_GEM,
    "Announce  �i ��ؤ��G�� �j"},

    {{Boards}, 0, M_BOARD,
    "Boards    �i \x1b[1;33m�G�i�Q�װ�\x1b[m �j"},

    {{Class}, 0, M_CLASS,
    "Class     �i \x1b[1;33m���հQ�װ�\x1b[m �j"},

#ifdef  HAVE_PROFESS
    {{Profess}, 0, M_PROFESS,
    "Profession�i \x1b[1;33m�M�~�Q�װ�\x1b[m �j"},
#endif

#ifdef  HAVE_FAVORITE
    {{MyFavorite}, PERM_BASIC, M_CLASS,
    "Favorite  �i \x1b[1;32m�ڪ��̷R��\x1b[m �j"},
#endif

#ifdef HAVE_SIGNED_MAIL
    {{.menu = menu_mail}, PERM_BASIC, M_MAIL, /* Thor.990413: �Y����m_verify, guest�N�S����椺�e�o */
    "Mail      �i �H��u��� �j"},
#else
    {{.menu = menu_mail}, PERM_BASIC, M_MAIL,
    "Mail      �i �p�H�H��� �j"},
#endif

    {{.menu = menu_talk}, 0, M_TMENU,
    "Talk      �i �𶢲�Ѱ� �j"},

    {{.menu = menu_service}, PERM_BASIC, M_XMENU,
    "DService  �i " NICKNAME "�A�Ȱ� �j"},

    /* lkchu.990428: ���n����b�ӤH�u��� */
    {{.menu = menu_xyz}, 0, M_SMENU,
    "Xyz       �i �t�θ�T�� �j"},

#ifdef  HAVE_CHANGE_SKIN
    {{.menu = *skin_main}, PERM_SYSOP, M_XMENU,
    "2Skin     �i ��ܤ����� �j"},
#endif

#ifndef MENU_NO_GOODBYE
    {{goodbye}, 0, M_XMODE,
    GOODBYE_EXIT},
#endif

    {{NULL}, PERM_MENU + 'B', M_MMENU,
    "�D�\\���"}
};

#ifdef __cplusplus
}  // namespace
#endif

#ifdef  TREAT
static int
goodbye1(void)
{
    switch (vans("G)�A�O" NICKNAME " Q)�����H[Q] "))
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
    outs("�� ������z���q���չϧ������A�� ��\n");
    bell();
    vkey();
    outs("�� ����  �F�A����  ^O^ �A" BOARDNAME "���z�M�H�`�ּ� ��\n");
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
    "�D�\\���"}
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
                if (max <= 0)                /* �䤣��A�X�v�����\��A�^�W�@�h�\��� */
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
        //prints("\n\x1b[30;47m     �ﶵ         �ﶵ����                         �ʺA�ݪO                   \x1b[m\n");
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
                    /* Thor.980826: �� outz �N���� move + clrtoeol�F */
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
