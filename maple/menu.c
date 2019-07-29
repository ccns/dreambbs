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
#define INFO_EMPTY      "Info      �i \x1b[1;36m�դ褽�i��\x1b[m �j"
#define INFO_HAVE       "Info      �i \x1b[41;33;1m�ֶi�Ӭݬ�\x1b[m �j"
#endif

#ifdef  HAVE_STUDENT
#define STUDENT_EMPTY   "1Student  �i \x1b[1;36m�ǥͤ��i��\x1b[m �j"
#define STUDENT_HAVE    "1Student  �i \x1b[41;33;1m�ֶi�Ӭݬ�\x1b[m �j"
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
    double load[3];
    char buf[80];

    /*load = ushm->sysload;*/
    getloadavg(load, 3);
    sprintf(buf, "�t�έt�� %.2f %.2f %.2f", load[0], load[1], load[2]);
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
    outs("\x1b[1;37;45m ��  " BOARDNAME " �d �� �O  �� \n\n");
    count = 0;

    mgets(-1);

    for (;;)
    {
        pad = mread(fd, sizeof(Pad));
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

    time(&(pad.tpad));

    str = pad.msg;

    sprintf(str, "\x1b[1;37;46m�W\x1b[34;47m %s \x1b[33m(%s)", cuser.userid, cuser.username);
    len = strlen(str);
    strcat(str, & " \x1b[30;46m"[len & 1]);

    for (i = len >> 1; i < 41; i++)
        strcat(str, "�e");
    sprintf(str2, "\x1b[34;47m %.14s \x1b[37;46m�W\x1b[m\n%-70.70s\n%-70.70s\n%-70.70s\n",
        Etime(&(pad.tpad)), buf[0], buf[1], buf[2]);
    strcat(str, str2);

    f_cat(FN_NOTE_ALL, str);

    if (!(fpw = fopen(FN_NOTE_TMP, "w")))
        return;
    str[355] = '\n';

    fwrite(&pad, sizeof(pad), 1, fpw);

    if ((fdr = open(FN_NOTE_PAD, O_RDONLY)) >= 0)
    {
        Pad *pp;

        i = 0;
        cc = pad.tpad - NOTE_DUE * 60 * 60;
        mgets(-1);
        while ((pp = mread(fdr, sizeof(Pad))))
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
        return XEASY;
    default:
        return XEASY; /* 090911.cache: ���p�߫������n�����H�a ;( */
    }

#ifdef  LOG_BMW
    /*bmw_save();*/                   /* lkchu.981201: ���T�O���B�z */
#endif

    clear();
    prints("       \x1b[1;31m ��       \x1b[1;36m �z�w�{�z�w�{�z�w�{�z�w�� �z�w���z���{�z�w�{\n"
        "      \x1b[1;31m��\x1b[1;37m��\x1b[1;33m��\x1b[1;37m������\x1b[1;36m�x  �s�x  �x�x  �x�x  �x �x �� �|  �}�x����\x1b[1;37m��������\n"
        "       \x1b[1;33m ��        \x1b[1;34m�|�w�t�|�w�}�|�w�}�|�w�� �|�w�� �|�} �|�w�}\x1b[m\n");
    prints("Dear \x1b[32m%s(%s)\x1b[m�A�O�ѤF�A�ץ��{�i %s �j\n"
        "�H�U�O�z�b���������U���:\n",
        cuser.userid, cuser.username, str_site);
    acct_show(&cuser, 3);
    vmsg(NULL);
    u_exit("EXIT ");
    exit(0);
}


/* ----------------------------------------------------- */
/* help & menu processing                                */
/* ----------------------------------------------------- */

void
vs_head(
    const char *title, const char *mid)
{
    char buf[(T_COLS - 1) - 79 + 69 + 1];     /* d_cols �̤j�i��O (T_COLS - 1) */
    char ttl[(T_COLS - 1) - 79 + 69 + 1];
    int spc, len;
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

    spc = strlen(mid);
    ufo = cutmp->ufo;

    len = d_cols + 65 - strlen(title) - strlen(currboard); /* len: �����ٳѤU�h�����Ŷ� */
    if (ufo & UFO_BIFF)
    {
        mid = NEWMAILMSG; // �A���s����
        spc = 15;
    }
    else if (ufo & UFO_BIFFN)
    {
        mid = NEWPASSMSG; // �A���s�d��
        spc = 15;
    }
    else if ( spc > len )
    {
        spc = len;
    }
    memcpy(ttl, mid, spc);
    ttl[spc] = '\0';
    mid = ttl;

    spc = 2 + len - spc; /* �\�� mid �H��A�����٦� spc ��Ŷ��A�b mid ���k�U�� spc/2 �����ť� */
    len = (1 - spc) & 1;

    if (spc < 0)
    {
        ttl[strlen(mid)+spc]= '\0';
        spc = 0;
    }

    memset(buf, ' ', spc >>= 1);
    buf[spc] = '\0';

#ifdef  COLOR_HEADER
    prints("\x1b[1;%2d;37m�i%s�j%s\x1b[33m%s\x1b[1;%2d;37m%s\x1b[37m�ݪO�m%s�n\x1b[m\n",
        color, title, buf, mid, color, buf + len, currboard);
#else
    prints("\x1b[1;46;37m�i%s�j%s\x1b[33m%s\x1b[46m%s\x1b[37m�ݪO�m%s�n\x1b[m\n",
        title, buf, mid, buf + len, currboard);
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
            & "�Ѥ@�G�T�|����"[ptime->tm_wday << 1]);

        uptime = now + 86400 - ptime->tm_hour * 3600 -
            ptime->tm_min * 60 - ptime->tm_sec;
    }
    ufo = (now - (uptime - 86400)) / 60;

    /* Thor.980913: ����: �̱`���I�s movie()���ɾ��O�C����s film, �b 60��H�W,
                          �G���ݰw�� xx:yy �ӯS�O�@�@�r���x�s�H�[�t */

    sprintf(footer, "\x1b[0;34;46m%s%d:%02d] \x1b[30;47m �ثe���W��\x1b[31m%4d\x1b[30m �H�A�ڬO \x1b[31m%-12s\x1b[30m [�I�s/�T��]\x1b[31m%s",
        datemsg, ufo / 60, ufo % 60,
        /*ushm->count*/total_num, cuser.userid, flagmsg);
    outf(footer);
}


#define MENU_CHANG      0x80000000


#define PERM_MENU       PERM_PURGE


static MENU menu_main[];
static MENU menu_service[];
static MENU menu_xyz[];
static MENU menu_user[];
static MENU menu_song[];


/* ----------------------------------------------------- */
/* load menu                                             */
/* ----------------------------------------------------- */
static MENU menu_admin[];

static MENU menu_load[] =
{
    {BINARY_PREFIX"adminutil.so:top", PERM_SYSOP, - M_XMODE,
    "Top        �u�W Load"},

    {BINARY_PREFIX"adminutil.so:psaux", PERM_SYSOP, - M_XMODE,
    "Ps         �u�W����{��"},

    {BINARY_PREFIX"adminutil.so:dmesg", PERM_SYSOP, - M_XMODE,
    "Dmesg      �t�ΰT��"},

    {BINARY_PREFIX"adminutil.so:df", PERM_SYSOP, - M_XMODE,
    "FileSys    �ϺШt��"},

    {menu_admin, PERM_MENU + 'T', M_ADMIN,
    "�t�έt��"}
};

static MENU menu_boardadm[] =
{
    {m_newbrd, PERM_BOARD, M_SYSTEM,
    "NewBoard   �}�P�s�ݪO"},

    {a_editbrd, PERM_BOARD, M_SYSTEM,
    "ConfigBrd  �]�w�ݪO"},

    {m_bmset, PERM_BOARD, M_SYSTEM,
    "BMset      �]�w���D�v��"},

    {BanMail, PERM_BOARD|PERM_SYSOP, M_BANMAIL,
    "FireWall   �׫H�C��"},

    {BINARY_PREFIX"adminutil.so:bm_check", PERM_BOARD|PERM_SYSOP, - M_XMODE,
    "Manage     �O�D�T�{"},

    {BINARY_PREFIX"adminutil.so:m_expire", PERM_BOARD|PERM_SYSOP, - M_XMODE,
    "DExpire    �M���ݪO�R���峹"},

    {BINARY_PREFIX"adminutil.so:mail_to_bm", PERM_SYSOP, - M_XMODE,
    "ToBM       �H�H���O�D"},

    {BINARY_PREFIX"adminutil.so:mail_to_all", PERM_SYSOP, - M_XMODE,
    "Alluser    �t�γq�i"},

    {BINARY_PREFIX"personal.so:personal_admin", PERM_BOARD|PERM_SYSOP, - M_XMODE,
    "Personal   �ӤH�O�f��"},

    {menu_admin, PERM_MENU + 'N', M_ADMIN,
    "�ݪO�`��"}
};

static MENU menu_accadm[] =
{
    {m_user, PERM_ACCOUNTS, M_SYSTEM,
    "User       �ϥΪ̸��"},

    {BINARY_PREFIX"bank.so:money_back", PERM_ACCOUNTS, - M_XMODE,
    "GetMoney   �פJ�¹ڹ�"},

#ifdef  HAVE_SONG
    {BINARY_PREFIX"song.so:AddRequestTimes", PERM_KTV, - M_XMODE,
    "Addsongs   �W�[�I�q����"},
#endif

    {BINARY_PREFIX"passwd.so:new_passwd", PERM_SYSOP, - M_XMODE,
    "Password   ���e�s�K�X"},

#ifdef  HAVE_REGISTER_FORM
    {m_register, PERM_ACCOUNTS, M_SYSTEM,
    "1Register  �f�ֵ��U���"},
#endif

#ifdef HAVE_OBSERVE_LIST
    {BINARY_PREFIX"observe.so:Observe_list", PERM_SYSOP|PERM_BOARD, - M_XMODE,
    "2Observe   �t���[��W��"},
#endif

    {menu_admin, PERM_MENU + 'U', M_ADMIN,
    "���U�`��"}
};

static MENU menu_settingadm[] =
{

    {BINARY_PREFIX"adminutil.so:m_xfile", PERM_SYSOP, - M_XFILES,
    "File       �s��t���ɮ�"},

    {BINARY_PREFIX"adminutil.so:m_xhlp", PERM_SYSOP, - M_XFILES,
    "Hlp        �s�軡���ɮ�"},

    {BINARY_PREFIX"admin.so:Admin", PERM_SYSOP, - M_XMODE,
    "Operator   �t�κ޲z���C��"},

    {BINARY_PREFIX"chatmenu.so:Chatmenu", PERM_CHATROOM|PERM_SYSOP, - M_XMODE,
    "Chatmenu   " CHATROOMNAME "�ʵ�"},

    {BINARY_PREFIX"violate.so:Violate", PERM_SYSOP, - M_XMODE,
    "Violate    �B�@�W��"},

    {BINARY_PREFIX"adminutil.so:special_search", PERM_SYSOP, - M_XMODE,
    "XSpecial   �S��j�M"},

    {BINARY_PREFIX"adminutil.so:update_all", PERM_SYSOP, - M_XMODE,
    "Database   �t�θ�Ʈw��s"},

    {menu_admin, PERM_MENU + 'X', M_ADMIN,
    "�t�θ��"}
};

/* ----------------------------------------------------- */
/* reset menu                                            */
/* ----------------------------------------------------- */
static MENU menu_reset[] =
{
    {menu_load, PERM_ADMIN, M_ADMIN,
    "Load       �t�έt��"},

    {BINARY_PREFIX"adminutil.so:reset1", PERM_BOARD, - M_XMODE,
    "Camera     �ʺA�ݪO"},

    {BINARY_PREFIX"adminutil.so:reset2", PERM_BOARD, - M_XMODE,
    "Group      �����s��"},

    {BINARY_PREFIX"adminutil.so:reset3", PERM_SYSOP, - M_XMODE,
    "Mail       �H�H���H��H"},

    {BINARY_PREFIX"adminutil.so:reset4", PERM_ADMIN, - M_XMODE,
    "Killbbs    �M�������` BBS"},

    {BINARY_PREFIX"adminutil.so:reset5", PERM_BOARD, - M_XMODE,
    "Firewall   �׫H�C��"},

    {BINARY_PREFIX"adminutil.so:reset6", PERM_CHATROOM, - M_XMODE,
    "Xchatd     ���}��ѫ�"},

    {BINARY_PREFIX"adminutil.so:reset7", PERM_SYSOP, - M_XMODE,
    "All        ����"},

    {menu_admin, PERM_MENU + 'K', M_ADMIN,
    "�t�έ��m"}
};


/* ----------------------------------------------------- */
/* administrator's maintain menu                         */
/* ----------------------------------------------------- */


static MENU menu_admin[] =
{

    {menu_accadm, PERM_ADMIN, M_ADMIN,
    "Acctadm    ���U�`�ޥ\\��"},

    {menu_boardadm, PERM_BOARD, M_ADMIN,
    "Boardadm   �ݪO�`�ޥ\\��"},

    {menu_settingadm, PERM_ADMIN, M_ADMIN,
    "Data       �t�θ�Ʈw�]�w"},

    {BINARY_PREFIX"innbbs.so:a_innbbs", PERM_BOARD, - M_SYSTEM,
    "InnBBS     ��H�]�w"},

    {menu_reset, PERM_ADMIN, M_ADMIN,
    "ResetSys   ���m�t��"},

#ifdef  HAVE_ADM_SHELL
    {x_csh, PERM_SYSOP, M_SYSTEM,
    "Shell      ����t�� Shell"},
#endif

#ifdef  HAVE_REPORT
    {m_trace, PERM_SYSOP, M_SYSTEM,
    "Trace      �]�w�O�_�O��������T"},
#endif

    {menu_main, PERM_MENU + 'A', M_ADMIN,
    "�t�κ��@"}
};


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


static MENU menu_mail[] =
{
    {XoMbox, PERM_READMAIL, M_RMAIL,
    "Read       �\\Ū�H��"},

    {m_send, PERM_INTERNET, M_SMAIL,
    "MailSend   �����H�H"},

#ifdef MULTI_MAIL  /* Thor.981009: ����R�����B�H */
    {mail_list, PERM_INTERNET, M_SMAIL,
    "Group      �s�ձH�H"},
#endif

    {BINARY_PREFIX"contact.so:Contact", PERM_INTERNET, - M_XMODE,
    "Contact    �p���W��"},

    {m_setforward, PERM_INTERNET, M_SMAIL,
    "AutoFor    �����H�۰���H"},

    {m_setmboxdir, PERM_INTERNET, M_SMAIL,
    "Fixdir     ���ثH�c����"},

#ifdef HAVE_DOWNLOAD
    {m_zip, PERM_VALID, M_XMODE,
    "Zip        ���]�U���ӤH���"},
#endif
/*
#ifdef HAVE_SIGNED_MAIL
    {m_verify, PERM_VALID, M_XMODE,
    "Verify     ���ҫH��q�lñ��"},
#endif
*/
    {mail_sysop, PERM_BASIC, M_SMAIL,
    "Yes Sir!   �H�H������"},

    {menu_main, PERM_MENU + 'R', M_MMENU,       /* itoc.020829: �� guest �S�ﶵ */
    "�q�l�l��"}
};


/* ----------------------------------------------------- */
/* Talk menu                                             */
/* ----------------------------------------------------- */

static int
XoUlist(void)
{
    xover(XZ_ULIST);
    return 0;
}


static MENU menu_talk[] =
{
    {XoUlist, 0, M_LUSERS,
    "Users      ������Ѥ�U"},

    {t_pal, PERM_BASIC, M_PAL,
    "Friend     �s��n�ͦW��"},

#ifdef  HAVE_BANMSG
    {t_banmsg, PERM_BASIC, M_XMODE,
    "Banmsg     �ڦ��T���W��"},
#endif
    {BINARY_PREFIX"aloha.so:t_aloha", PERM_BASIC, - M_XMODE,
    "Aloha      �W���q���W��"},

    {t_pager, PERM_BASIC, M_XMODE,
    "Pager      �����I�s��"},

    {t_message, PERM_BASIC, M_XMODE,
    "Message    �����T��"},

    {t_query, 0, M_QUERY,
    "Query      �d�ߺ���"},

#ifdef HAVE_CHATROOM_CLIENT     /* r2.20180405: still fixing chatroom..... */

    /* Thor.990220: chatroom client ��ĥ~�� */
    {BINARY_PREFIX"chat.so:t_chat", PERM_CHAT, - M_CHAT,
    "ChatRoom   " NICKNAME CHATROOMNAME},

#endif

    {t_recall, PERM_BASIC, M_XMODE,
    "Write      �^�U�e�X�����T"},

#ifdef LOGIN_NOTIFY
    {t_loginNotify, PERM_PAGE, M_XMODE,
    "Notify     �]�w�t�κ��ͨ�M"},
#endif
    {menu_main, PERM_MENU + 'U', M_UMENU,
    "�𶢲��"}
};


/* ----------------------------------------------------- */
/* System menu                                           */
/* ----------------------------------------------------- */

static MENU menu_information[] =
{

    {popmax, 0, M_READA,
    "Login      �W�����ƱƦ�]"},

    {today, 0, M_READA,
    "Today      ����W�u�H���έp"},

    {yesterday, 0, M_READA,
    "Yesterday  �Q��W�u�H���έp"},

    {day, 0, M_READA,
    "0Day       ����Q�j�������D"},

    {week, 0, M_READA,
    "1Week      ���g���Q�j�������D"},

    {month, 0, M_READA,
    "2Month     ����ʤj�������D"},

    {year, 0, M_READA,
    "3Year      ���~�צʤj�������D"},

    {menu_xyz, PERM_MENU + 'L', M_MMENU,
    "�έp���"}
};


static MENU menu_xyz[] =
{
    {menu_information, 0, M_XMENU,
    "Tops       " NICKNAME "�Ʀ�]"},

    {version, 0, M_READA,
    "Version    ���X�o�i��T"},

    {BINARY_PREFIX"xyz.so:x_siteinfo", 0, - M_READA,
    "Xinfo      �t�ε{����T"},

    {pad_view, 0, M_READA,
    "Note       �[�ݤ߱��d���O"},

    {welcome, 0, M_READA,
    "Welcome    �[���w��e��"},

    {history, 0, M_READA,
    "History    �������v�y��"},

    {menu_main, PERM_MENU + 'T', M_SMENU,
    "�t�θ�T"}
};

/* ----------------------------------------------------- */
/* User menu                                             */
/* ----------------------------------------------------- */

static MENU menu_reg[] =
{

    {u_info, PERM_BASIC, M_XMODE,
    "Info       �]�w�ӤH��ƻP�K�X"},

    {u_addr, PERM_BASIC, M_XMODE,
    "Address    ��g�q�l�H�c�λ{��"},

    {u_verify, PERM_BASIC, M_UFILES,
    "Verify     ��g�m���U�{�ҽX�n"},

#ifdef  HAVE_REGISTER_FORM
    {u_register, PERM_BASIC, M_UFILES,
    "Register   ��g�m���U�ӽг�n"},
#endif

    {u_setup, PERM_VALID, M_UFILES,
    "Mode       �]�w�ާ@�Ҧ�"},

    {ue_setup, 0, M_UFILES,
    "Favorite   �ӤH�ߦn�]�w"},

    {u_xfile, PERM_BASIC, M_UFILES,
    "Xfile      �s��ӤH�ɮ�"},

    {BINARY_PREFIX"list.so:List", PERM_VALID, - M_XMODE,
    "1List      �s�զW��"},

    {menu_user, PERM_MENU + 'I', M_MMENU,
    "���U��T"}
};


static MENU menu_user[] =
{
    {menu_reg, 0, M_XMENU,
    "Configure  ���U�γ]�w�ӤH��T"},

    {u_lock, PERM_BASIC, M_XMODE,
    "Lock       ��w�ù�"},

    {BINARY_PREFIX"memorandum.so:Memorandum", PERM_VALID, -M_XMODE,
    "Note       �Ƨѿ�"},

    {BINARY_PREFIX"pnote.so:main_note", PERM_VALID, - M_XMODE,
    "PNote      �ӤH������"},

#ifdef  HAVE_CLASSTABLEALERT
    {BINARY_PREFIX"classtable2.so:main_classtable", PERM_VALID, -M_XMODE,
    "2Table     �s���ӤH�\\�Ҫ�"},
#endif

    {view_login_log, PERM_BASIC, M_READA,
    "ViewLog    �˵��W������"},

    {menu_service, PERM_MENU + 'C', M_UMENU,
    "�ӤH�]�w"}
};


/* ----------------------------------------------------- */
/* tool menu                                             */
/* ----------------------------------------------------- */

static MENU menu_service[];

/* ----------------------------------------------------- */
/* game menu                                             */
/* ----------------------------------------------------- */

static MENU menu_game[] =
{
    {BINARY_PREFIX"bj.so:BlackJack", PERM_VALID, - M_XMODE,
    "BlackJack  " NICKNAME "�³ǧJ"},

    {BINARY_PREFIX"guessnum.so:fightNum", PERM_VALID, - M_XMODE,
    "FightNum   �Ʀr�j�M��"},

    {BINARY_PREFIX"guessnum.so:guessNum", PERM_VALID, - M_XMODE,
    "GuessNum   �̥ʲq�Ʀr"},

    {BINARY_PREFIX"mine.so:Mine", PERM_VALID, - M_XMODE,
    "Mine       " NICKNAME "��a�p"},

    {BINARY_PREFIX"pip.so:p_pipple", PERM_VALID, - M_XMODE,
    "Pip        " NICKNAME "�԰���"},

    {menu_service, PERM_MENU + 'F', M_UMENU,
    "�C����"}

};

/* ----------------------------------------------------- */
/* yzu menu                                              */
/* ----------------------------------------------------- */

static MENU menu_special[] =
{

    {BINARY_PREFIX"personal.so:personal_apply", PERM_VALID, - M_XMODE,
    "PBApply      �ӽЭӤH�ݪO"},

    {BINARY_PREFIX"bank.so:bank_main", PERM_VALID, - M_XMODE,
    "Bank       �@�Ȧ�"},

    {BINARY_PREFIX"shop.so:shop_main", PERM_VALID, - M_XMODE,
    "Pay        �@�ө�"},

#ifdef HAVE_SONG
    {menu_song, PERM_VALID, M_XMENU,
    "Request      �I�q�t��"},
#endif

    {resetbrd, PERM_ADMIN, M_XMODE,
    "CameraReset  �������]"},

    {menu_service, PERM_MENU + 'R', M_UMENU,
    "�[�ȪA��"}
};



/* ----------------------------------------------------- */
/* song menu                                             */
/* ----------------------------------------------------- */

#ifdef HAVE_SONG
static MENU menu_song[] =
{
    {BINARY_PREFIX"song.so:XoSongMain", PERM_VALID, - M_XMODE,
    "Request       �I�q�q��"},

    {BINARY_PREFIX"song.so:XoSongLog", PERM_VALID, - M_XMODE,
    "OrderSongs    �I�q����"},

    {BINARY_PREFIX"song.so:XoSongSub", PERM_VALID, - M_XMODE,
    "Submit        ��Z�M��"},

    {menu_special, PERM_MENU + 'R', M_XMENU,
    "���I�I�q"}
};
#endif


/* ----------------------------------------------------- */
/* service menu                                          */
/* ----------------------------------------------------- */

/* Thor.990224: �}��~���ɭ� */
static MENU menu_service[] =
{

    {menu_user, 0, M_UMENU,
    "User      �i �ӤH�u��� �j"},

    {menu_special, PERM_VALID, M_XMENU,
    "Bonus     �i �[�ȪA�Ȱ� �j"},

    {menu_game, PERM_VALID, M_XMENU,
    "Game      �i �C������� �j"},

#ifdef  HAVE_INFO
    {Information, 0, M_BOARD,
    INFO_EMPTY},
#endif

#ifdef  HAVE_STUDENT
    {Student, 0, M_BOARD,
    STUDENT_EMPTY},
#endif

/* 091007.cache: �ԤH�鲼�S�N�q... */

    {BINARY_PREFIX"newboard.so:XoNewBoard", PERM_VALID, - M_XMODE,
    "Cosign    �i �s�p�ӽа� �j"},

    {BINARY_PREFIX"vote.so:SystemVote", PERM_POST, - M_XMODE,
    "Vote      �i �t�Χ벼�� �j"},

    {system_result, 0, M_READA,
    "Result    �i�t�Χ벼���G�j"},

/*
#ifdef HAVE_SONG
    {menu_song, PERM_VALID, M_XMENU,
    "Song      �i  �I�q�t�ΰ�  �j"},
#endif
*/
    {menu_main, PERM_MENU + 'U', M_UMENU,
     NICKNAME "�A��"}
};

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

MENU skin_main[] =
{
    {sk_windtop_init, PERM_SYSOP, M_XMODE,
    "DreamBBS   �w�]���t��"},

    {menu_main, PERM_MENU + 'W', M_MMENU,
    "�������"}
};
#endif  /* #ifdef  HAVE_CHANGE_SKIN */

static int
Gem(void)
{
    XoGem("gem/.DIR", "", ((HAS_PERM(PERM_SYSOP|PERM_BOARD|PERM_GEM)) ? GEM_SYSOP : GEM_USER));
    return 0;
}

static MENU menu_main[] =
{
    {menu_admin, PERM_ADMIN, M_ADMIN,
    "0Admin    �i �t�κ��@�� �j"},

    {Gem, 0, M_GEM,
    "Announce  �i ��ؤ��G�� �j"},

    {Boards, 0, M_BOARD,
    "Boards    �i \x1b[1;33m�G�i�Q�װ�\x1b[m �j"},

    {Class, 0, M_CLASS,
    "Class     �i \x1b[1;33m���հQ�װ�\x1b[m �j"},

#ifdef  HAVE_PROFESS
    {Profess, 0, M_PROFESS,
    "Profession�i \x1b[1;33m�M�~�Q�װ�\x1b[m �j"},
#endif

#ifdef  HAVE_FAVORITE
    {MyFavorite, PERM_BASIC, M_CLASS,
    "Favorite  �i \x1b[1;32m�ڪ��̷R��\x1b[m �j"},
#endif

#ifdef HAVE_SIGNED_MAIL
    {menu_mail, PERM_BASIC, M_MAIL, /* Thor.990413: �Y����m_verify, guest�N�S����椺�e�o */
    "Mail      �i �H��u��� �j"},
#else
    {menu_mail, PERM_BASIC, M_MAIL,
    "Mail      �i �p�H�H��� �j"},
#endif

    {menu_talk, 0, M_TMENU,
    "Talk      �i �𶢲�Ѱ� �j"},

    {menu_service, PERM_BASIC, M_XMENU,
    "DService  �i " NICKNAME "�A�Ȱ� �j"},

    /* lkchu.990428: ���n����b�ӤH�u��� */
    {menu_xyz, 0, M_SMENU,
    "Xyz       �i �t�θ�T�� �j"},

#ifdef  HAVE_CHANGE_SKIN
    {skin_main, PERM_SYSOP, M_XMENU,
    "2Skin     �i ��ܤ����� �j"},
#endif

    {goodbye, 0, M_XMODE,
    "Goodbye   �i�A�O" BOARDNAME "�j"},

    {NULL, PERM_MENU + 'B', M_MMENU,
    "�D�\\���"}
};

#ifdef  TREAT
static int
goodbye1(void)
{
    switch (vans("G)�A�O" NICKNAME " Q)�����H[Q] "))
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
    outs("�� ������z���q���չϧ������A�� ��\n");
    bell();
    vkey();
    outs("�� ����  �F�A����  ^O^ �A" BOARDNAME "���z�M�H�`�ּ� ��\n");
    bell();
    vkey();
    return KEY_NONE;
}



static MENU menu_treat[] =
{
    {goodbye1, 0, M_XMODE,
    "Goodbye   �i�A�O" NICKNAME "�j"},

    {NULL, PERM_MENU + 'G', M_MMENU,
    "�D�\\���"}
};
#endif  /* #ifdef  TREAT */

static
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
    BRD *brd;
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
                    if (max < 0)                /* �䤣��A�X�v�����\��A�^�W�@�h�\��� */
                    {
                        menu = (MENU *) menu->func;
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

            if (mmx < max)
                mmx = max;

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
            //prints("\n\x1b[30;47m     �ﶵ         �ﶵ����                         �ʺA�ݪO                   \x1b[m\n");
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
#if 1
            /* Thor.990212: dynamic load, with negative umode */
            if (cmd < 0)
            {
                void *p = DL_get(mptr->func);
                if (!p) break;
                mptr->func = p;
                cmd = -cmd;
                mptr->umode = cmd;
            }
#endif
            utmp_mode(cmd /* = mptr->umode*/);

            if (cmd <= M_XMENU)
            {
                menu->level = PERM_MENU + mptr->desc[0];
                menu = (MENU *) mptr->func;
                mode = MENU_LOAD | MENU_DRAW | MENU_FILM;
                depth++;
                continue;
            }

            {
                int (*func) (void);

                func = mptr->func;
                mode = (*func) ();
            }

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
                /* Thor.980826: �� outz �N���� move + clrtoeol�F */
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
                menu = (MENU *) menu->func;
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

