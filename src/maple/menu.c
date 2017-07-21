/*-------------------------------------------------------*/
/* menu.c	( NTHU CS MapleBBS Ver 3.00 )		 */
/*-------------------------------------------------------*/
/* target : menu/help/movie routines		 	 */
/* create : 95/03/29				 	 */
/* update : 2000/01/02				 	 */
/*-------------------------------------------------------*/

#include "bbs.h"

extern int t_aloha();
extern BCACHE *bshm;
extern UCACHE *ushm;
int item_lenth[20]={0};

extern time_t brd_visit[MAXBOARD];

#ifdef	HAVE_INFO
#define INFO_EMPTY	"Info      �i \033[1;36m�դ褽�i��\033[m �j"
#define	INFO_HAVE	"Info      �i \033[41;33;1m�ֶi�Ӭݬ�\033[m �j"
#endif

#ifdef	HAVE_STUDENT
#define	STUDENT_EMPTY	"1Student  �i \033[1;36m�ǥͤ��i��\033[m �j"
#define STUDENT_HAVE	"1Student  �i \033[41;33;1m�ֶi�Ӭݬ�\033[m �j"
#endif

#ifdef	HAVE_ACTIVITY
#define	ACTIVITY_EMPTY	"2Activity �i \033[1;36m���ʤ��i��\033[m �j"
#define	ACTIVITY_HAVE	"2Activity �i \033[41;33;1m�ֶi�Ӭݬ�\033[m �j"
#endif

static int
system_result()
{
  char fpath[128];
  sprintf(fpath,"brd/%s/@/@vote",brd_sysop);
  more(fpath,0);
  return 0;
}


static int
view_login_log()
{
  char fpath[128];
  usr_fpath(fpath,cuser.userid,FN_LOG);
  more(fpath,0);
  return 0;
}


static int
history()
{
  more(FN_ETC_COUNTER, 0);
  return 0;
}

static int
version()
{
  more(FN_ETC_VERSION, 0);
  return 0;
}

static int
today()
{
  more("gem/@/@-act", 0);
  return 0;
}

#ifdef	HAVE_PERSON_DATA
static int
count_birth()
{
  more("gem/@/@-birth", 0);
  return 0;
}

static int
count_age()
{
  more("gem/@/@-yearsold", 0);
  return 0;
}

static int
count_star()
{
  more("gem/@/@-star", 0);
  return 0;
}
#endif

static int
popmax()
{
  more("gem/@/@pop", 0);
  return 0;
}

static int
yesterday()
{
  more("gem/@/@=act", 0);
  return 0;
}

static int 
day()
{
  more("gem/@/@-day", 0);
  return 0;
}

static int
week()
{
  more("gem/@/@-week", 0);
  return 0;
}

static int
month()
{
  more("gem/@/@-month", 0);
  return 0;
}

static int
year()
{
  more("gem/@/@-year", 0);
  return 0;
}

static int
welcome()
{
  film_out(FILM_WELCOME, -1);
  return 0;
}

static int
resetbrd()
{
  board_main();
  return 0;
}

static int
x_sysload()
{
  double load[3];
  char buf[80];

  /*load = ushm->sysload;*/
  getloadavg(load,3);
  sprintf(buf, "�t�έt�� %.2f %.2f %.2f", load[0], load[1], load[2]);
  vmsg(buf);
  return XEASY;
}


/* ----------------------------------------------------- */
/* ���} BBS ��						 */
/* ----------------------------------------------------- */


#define	FN_NOTE_PAD	"run/note.pad"
#define	FN_NOTE_TMP	"run/note.tmp"
#define	NOTE_MAX	100
#define	NOTE_DUE	48


typedef struct
{
  time_t tpad;
  char msg[356];
}      Pad;


int
pad_view()
{
  int fd, count;
  Pad *pad;

  if ((fd = open(FN_NOTE_PAD, O_RDONLY)) < 0)
    return XEASY;

  clear();
  move(0, 23);
  outs("\033[1;37;45m ��  " BOARDNAME " �d �� �O  �� \n\n");
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
pad_draw()
{
  int i, cc, fdr,len;
  FILE *fpw;
  Pad pad;
  char *str,str2[300], buf[3][80];

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

  sprintf(str, "\033[1;37;46m�W\033[34;47m %s \033[33m(%s)", cuser.userid, cuser.username);
  len = strlen(str);
  strcat(str, " \033[30;46m" + (len & 1));
  
  for (i = len >> 1; i < 41; i++)
    strcat(str,"�e");    
  sprintf(str2, "\033[34;47m %.14s \033[37;46m�W\033[m\n%-70.70s\n%-70.70s\n%-70.70s\n",
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
goodbye()
{
  char ans;
  bmw_save();
  if(cuser.ufo2 & UFO2_DEF_LEAVE)
  {
    if(!(ans = vans("G)�A�O" NICKNAME " M)���i���� N)�d���O Q)�����H[Q] ")))
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
    if(HAS_PERM(PERM_POST)) /* Thor.990118: �n��post�~��d��, �������e */
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
  prints("       \033[1;31m ��       \033[1;36m �z�w�{�z�w�{�z�w�{�z�w�� �z�w���z���{�z�w�{\n"
  "      \033[1;31m��\033[1;37m��\033[1;33m��\033[1;37m������\033[1;36m�x  �s�x  �x�x  �x�x  �x �x �� �|  �}�x����\033[1;37m��������\n"
  "       \033[1;33m ��        \033[1;34m�|�w�t�|�w�}�|�w�}�|�w�� �|�w�� �|�} �|�w�}\033[m\n");
  prints("Dear \033[32m%s(%s)\033[m�A�O�ѤF�A�ץ��{�i %s �j\n"
    "�H�U�O�z�b���������U���:\n",
    cuser.userid, cuser.username, str_site);
  acct_show(&cuser, 3);
  vmsg(NULL);
  u_exit("EXIT ");
  exit(0);
}


/* ----------------------------------------------------- */
/* help & menu processring				 */
/* ----------------------------------------------------- */


void
vs_head(title, mid)
  char *title, *mid;
{
  char buf[40], ttl[60];
  int spc;
  usint ufo;
#ifdef  COLOR_HEADER
/*  int color = (time(0) % 7) + 41;        lkchu.981201: random color */
  int color = 44; //090911.cache: �Ӫ�F�T�w�@���C�� 
#endif
#if 1
  char *newyear[6]={"�ڤ��j�a���j�a�s�~�ּ�        ",
                    "                              ",
                    "Dreambbs/NCKU.CCNS            ",
                    "Since 1995                    ",
                    "                              ",
                    "WindTop-cache modified (C)2010"};
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

  if (!*title)
  {
    title++;

    if (ufo & UFO_BIFF)
      cutmp->ufo = ufo ^ UFO_BIFF;     /* �ݹL�N�� */
    if (ufo & UFO_BIFFN)
      cutmp->ufo = ufo ^ UFO_BIFFN;     /* �ݹL�N�� */
  }
  else
  {
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
#if 1
	/* 2005 : 1104508800 */
    else if(time(0) < 1136044800)
    {
      time_t now,hour,min,sec;
      now = 1136044800 - time(0);
      if(now <= 0)
      {
        strcpy(ttl,newyear[time(0)%6]);
        spc = strlen(ttl);
        mid = ttl;
        mid[spc] = '\0';
      }
      else
      {
        hour = now/3600;
        min = now/60 - hour*60;
        sec = now % 60;
        sprintf(ttl,"�Z 2006 �~�ٳ� %02d �� %02d �� %02d ��",hour,min,sec);
        spc = strlen(ttl);
        mid = ttl;  
        mid[spc] = '\0';
      }

    }
#endif    
#if 0
    else if(aprilfirst)
    {
        sprintf(ttl,"�ڲέp�A�ثe�� %d �Ӷm�����F",ushm->avgload);
        spc = strlen(ttl);
        mid = ttl;
        mid[spc] = '\0';
    }
#endif
    else if (spc > (65 - strlen(title)-strlen(currboard)))
    {
      spc = (65 - strlen(title)-strlen(currboard));
      memcpy(ttl, mid, spc);
      mid = ttl;
      mid[spc] = '\0';
    }
  }

  spc = 67 - strlen(title) - spc - strlen(currboard);
  
  if(spc < 0)
  {
    mid[strlen(mid)+spc]= '\0';
    spc = 0;
  }
  
  ufo = 1 - (spc & 1);
  memset(buf, ' ', spc >>= 1);
  buf[spc] = '\0';

#ifdef	COLOR_HEADER
  prints("\033[1;%2d;37m�i%s�j%s\033[33m%s\033[1;%2d;37m%s\033[37m�ݪO�m%s�n\033[m\n",
    color, title, buf,mid , color, buf + ufo, currboard);
#else
  prints("\033[1;46;37m�i%s�j%s\033[33m%s\033[46m%s\033[37m�ݪO�m%s�n\033[m\n",
    title, buf, mid, buf + ufo, currboard);
#endif
}


/* ------------------------------------- */
/* �ʵe�B�z				 */
/* ------------------------------------- */


static char footer[160];


void
movie()
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
      "�Ѥ@�G�T�|����" + (ptime->tm_wday << 1));

    uptime = now + 86400 - ptime->tm_hour * 3600 -
      ptime->tm_min * 60 - ptime->tm_sec;
  }
  ufo = (now - (uptime - 86400)) / 60;

  /* Thor.980913: ����: �̱`���I�s movie()���ɾ��O�C����s film, �b 60��H�W,
                        �G���ݰw�� xx:yy �ӯS�O�@�@�r���x�s�H�[�t */

  sprintf(footer, "\033[0;34;46m%s%d:%02d] \033[30;47m �ثe���W��\033[31m%4d\033[30m �H, �ڬO \033[31m%-12s\033[30m [�I�s/�T��]\033[31m%s\033[m",
    datemsg, ufo / 60, ufo % 60,
    /*ushm->count*/total_num, cuser.userid, flagmsg);
  outz(footer);
}


#define	MENU_CHANG	0x80000000


#define	PERM_MENU	PERM_PURGE


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
  {"bin/adminutil.so:top", PERM_SYSOP, - M_XMODE,
  "Top        �u�W Load"},

  {"bin/adminutil.so:psaux", PERM_SYSOP, - M_XMODE,
  "Ps         �u�W����{��"},

  {"bin/adminutil.so:dmesg", PERM_SYSOP, - M_XMODE,
  "Dmesg      �t�ΰT��"},

  {"bin/adminutil.so:df", PERM_SYSOP, - M_XMODE,
  "Storage    �ϺШt��"},

  {x_sysload, PERM_ADMIN, M_SYSTEM,
  "Load       �t�έt��"},

  {menu_admin, PERM_MENU + 'T', M_XMENU,
  "�t�έt��"}
};

static MENU menu_boardadm[] =
{
  {m_newbrd, PERM_BOARD, M_SYSTEM,
  "NewBoard   �}�P�s�ݪO"},

  {a_editbrd, PERM_BOARD, M_SYSTEM,
  "SetBoard   �]�w�ݪO"},

  {m_bmset, PERM_BOARD, M_SYSTEM,
  "BMset      �]�w���D�v��"},

  {BanMail,PERM_BOARD|PERM_SYSOP, M_BANMAIL,
  "FireWall   �׫H�C��"},

  {"bin/adminutil.so:bm_check",PERM_BOARD|PERM_SYSOP, - M_XMODE,
  "Manage     ���D�T��"},

  {"bin/adminutil.so:m_expire", PERM_BOARD|PERM_SYSOP, - M_XMODE,
  "Clean      �M���ݪO�R���峹"},

  {"bin/adminutil.so:mail_to_bm", PERM_SYSOP, - M_XMODE,
  "ToBM       �H�H���O�D"},

  {"bin/adminutil.so:mail_to_all", PERM_SYSOP, - M_XMODE,
  "Alluser    �t�γq�i"},
  
    "bin/personal.so:personal_admin", PERM_BOARD|PERM_SYSOP, - M_XMODE,
  "Personal   �ӤH�O�f��",

  {menu_admin, PERM_MENU + 'N', M_XMENU,
  "�ݪO�`��"}
};

static MENU menu_accadm[] =
{
  {m_user, PERM_ACCOUNTS, M_SYSTEM,
  "User       �ϥΪ̸��"},

  {"bin/bank.so:money_back", PERM_ACCOUNTS, - M_XMODE,
  "GetMoney   �פJ�¹ڹ�"},

#ifdef	HAVE_SONG
  {"bin/song.so:AddRequestTimes",PERM_KTV , - M_XMODE,
  "Addsongs   �W�[�I�q����"},
#endif

  {"bin/passwd.so:new_passwd", PERM_SYSOP, - M_XMODE,
  "Password   ���e�s�K�X"},

#ifdef  HAVE_REGISTER_FORM
  {m_register, PERM_ACCOUNTS, M_SYSTEM,
  "1Register  �f�ֵ��U���"},
#endif

#ifdef HAVE_OBSERVE_LIST
  {"bin/observe.so:Observe_list", PERM_SYSOP|PERM_BOARD, - M_XMODE,
  "2Observe   �t���[��W��"},
#endif
 
  {menu_admin, PERM_MENU + 'U', M_XMENU,
  "���U�`��"}
};

static MENU menu_settingadm[] =
{

  {"bin/adminutil.so:m_xfile", PERM_SYSOP, - M_XFILES,
  "XFile      �s��t���ɮ�"},
  
  {"bin/adminutil.so:m_xhlp", PERM_SYSOP, - M_XFILES,
  "Hlp        �s�軡���ɮ�"},

  {"bin/admin.so:Admin",PERM_SYSOP, - M_XMODE,
  "Operator   �t�κ޲z���C��"},

  {"bin/chatmenu.so:Chatmenu",PERM_CHATROOM|PERM_SYSOP,- M_XMODE,
  "Chatmenu   " CHATROOMNAME "�ʵ�"},

  {"bin/violate.so:Violate",PERM_SYSOP,- M_XMODE,
  "Violate    �B�@�W��"},
  
  {"bin/adminutil.so:special_search",PERM_SYSOP, - M_XMODE,
  "Special    �S��j�M"},

  {"bin/adminutil.so:update_all",PERM_SYSOP, - M_XMODE,
  "Database   �t�θ�Ʈw��s"},  
  
  {menu_admin, PERM_MENU + 'X', M_XMENU,
  "�t�θ��"}
};

/* ----------------------------------------------------- */
/* reset menu                                            */
/* ----------------------------------------------------- */
static MENU menu_reset[] =
{
  {menu_load, PERM_ADMIN, M_XMENU,
  "Load       �t�έt��"},

  {"bin/adminutil.so:reset1", PERM_BOARD, - M_XMODE,
  "Camera     �ʺA�ݪO"},

  {"bin/adminutil.so:reset2", PERM_BOARD, - M_XMODE,
  "Group      �����s��"},

  {"bin/adminutil.so:reset3", PERM_SYSOP, - M_XMODE,
  "Mail       �H�H���H��H"},

  {"bin/adminutil.so:reset4", PERM_ADMIN, - M_XMODE,
  "Killbbs    �M�������` BBS"},

  {"bin/adminutil.so:reset5", PERM_BOARD, - M_XMODE,
  "Firewall   �׫H�C��"},

  {"bin/adminutil.so:reset6", PERM_CHATROOM, - M_XMODE,
  "Xchatd     ���}��ѫ�"},

  {"bin/adminutil.so:reset7", PERM_SYSOP, - M_XMODE,
  "All        ����"},

  {menu_admin, PERM_MENU + 'K', M_XMENU,
  "�t�έ��m"}
};


/* ----------------------------------------------------- */
/* administrator's maintain menu			 */
/* ----------------------------------------------------- */


static MENU menu_admin[] =
{

  {menu_accadm, PERM_ADMIN, M_XMENU,
  "Acctadm    ���U�`�ޥ\\��"},

  {menu_boardadm, PERM_BOARD, M_XMENU,
  "Boardadm   �ݪO�`�ޥ\\��"},

  {menu_settingadm, PERM_ADMIN, M_XMENU,
  "Data       �t�θ�Ʈw�]�w"},

  {"bin/innbbs.so:a_innbbs", PERM_BOARD, - M_SYSTEM,
  "InnBBS     ��H�]�w"},

  {menu_reset, PERM_ADMIN, M_XMENU,
  "ResetSys   ���m�t��"},

#ifdef	HAVE_ADM_SHELL
  {x_csh, PERM_SYSOP, M_SYSTEM,
  "Shell      ����t�� Shell"},
#endif

#ifdef	HAVE_REPORT
  {m_trace, PERM_SYSOP, M_SYSTEM,
  "Trace      �]�w�O�_�O��������T"},
#endif

  {menu_main, PERM_MENU + 'A', M_ADMIN,
  "�t�κ��@"}
};


/* ----------------------------------------------------- */
/* mail menu						 */
/* ----------------------------------------------------- */


static int
XoMbox()
{
  if (HAS_PERM(PERM_DENYMAIL))
     vmsg("�z���H�c�Q��F�I");
  else
     xover(XZ_MBOX);
  return 0;
}

#ifdef HAVE_SIGNED_MAIL
int m_verify();
#endif
int m_setmboxdir();


static MENU menu_mail[] =
{
  {XoMbox, PERM_READMAIL, M_RMAIL,
  "Read       �\\Ū�H��"},

  {m_send, PERM_INTERNET, M_SMAIL,
  "Send       �����H�H"},

#ifdef MULTI_MAIL  /* Thor.981009: ����R�����B�H */
  {mail_list, PERM_INTERNET, M_SMAIL,
  "Group      �s�ձH�H"},
#endif

  {"bin/contact.so:Contact", PERM_INTERNET, - M_XMODE,
  "Contact    �p���W��"},

  {m_internet, PERM_INTERNET, M_SMAIL,
  "Internet   �H�H�� Internet"},

  {m_setforward, PERM_INTERNET, M_SMAIL,
  "AutoFor    �����H�۰���H"},

  {m_setmboxdir, PERM_INTERNET, M_SMAIL,
  "Fixdir     ���ثH�c����"},

/*  
  {"bin/lovepaper.so:lovepaper", PERM_INTERNET, - M_XMODE,
  "LovePager  \033[1;31m���Ѳ��;�\033[m"},
*/
#ifdef HAVE_DOWNLOAD
  {m_zip, PERM_VALID, M_XMODE,
  "Zip        ���]�U������U�H�c"},
#endif
/* 
#ifdef HAVE_SIGNED_MAIL
  {m_verify, PERM_VALID, M_XMODE,
  "Verify     ���ҫH��q�lñ��"},
#endif
*/ 
  {mail_sysop, PERM_BASIC, M_SMAIL,
  "Yes Sir!   �H�H������"},
/* 
#ifdef HAVE_MIME_TRANSFER
  {"bin/imap4mail.so:Imap4mail", PERM_INTERNET, - M_XMODE,
  "Mail       IMAP4�l��t�ΪA��"},
#endif

  {"bin/pop3mail.so:Pop3mail", PERM_SYSOP, -M_XMODE,
  "Pop3       POP3 �l��t�ΪA��"},
*/ 
  {menu_main, PERM_MENU + 'R', M_MMENU,	/* itoc.020829: �� guest �S�ﶵ */
  "�q�l�l��"}
};


/* ----------------------------------------------------- */
/* Talk menu						 */
/* ----------------------------------------------------- */

static int
XoUlist()
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

#ifdef	HAVE_BANMSG
  {t_banmsg, PERM_BASIC, M_XMODE,
  "Banmsg     �ڦ��T���W��"},
#endif
  {"bin/aloha.so:t_aloha", PERM_BASIC, - M_XMODE,
  "Aloha      �W���q���W��"},

  {t_pager, PERM_BASIC, M_XMODE,
  "Pager      �����I�s��"},

  {t_message, PERM_BASIC, M_XMODE,
  "Message    �����T��"},

  {t_query, 0, M_QUERY,
  "Query      �d�ߺ���"},

#if 0
  /* Thor.990220: ��ĥ~�� */
  {"bin/chat.so:t_chat", PERM_CHAT, - M_CHAT,
  "Chat       " NICKNAME CHATROOMNAME},
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
/* System menu						 */
/* ----------------------------------------------------- */

static MENU menu_information[] = 
{

  {popmax, 0 , M_READA,
  "Users      �W�����ƱƦ�]"},

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
/*
#ifdef  HAVE_PERSON_DATA
  {count_birth, 0, M_READA,
  "4Birthday  ����جP�έp"},

  {count_age, 0, M_READA,
  "5YearsOld  �ϥΪ̦~�ֲέp"},

  {count_star, 0, M_READA,
  "6Star      �ϥΪ̬P�y�έp"},
#endif
*/
  menu_xyz, PERM_MENU + 'U', M_MMENU,
  "�έp���"
};


static MENU menu_xyz[] =
{
  menu_information, 0, M_XMENU,
  "Dream      �ڤ��j�a�Ʀ�]",

  {version,0,M_READA,
  "Version    ���X�o�i��T"},

  {"bin/xyz.so:x_siteinfo", 0, - M_READA,
  "Source     �t�ε{����T"},

  {pad_view, 0, M_READA,
  "Note       �[�ݤ߱��d���O"},

  {welcome, 0, M_READA,
  "Welcome    �[���w��e��"},

  {history, 0, M_READA,
  "History    �������v�y��"},

  {menu_main, PERM_MENU + 'D', M_SMENU,
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

  u_verify, PERM_BASIC, M_UFILES,
  "Verify     ��g�m���U�{�ҽX�n",

#ifdef	HAVE_REGISTER_FORM
  {u_register, PERM_BASIC, M_UFILES,
  "Register   ��g�m���U�ӽг�n"},
#endif

  {u_setup, PERM_VALID, M_UFILES,
  "Setup      �]�w�ާ@�Ҧ�"},

  {ue_setup, 0, M_UFILES,
  "Favorite   �ӤH�ߦn�]�w"},

  {u_xfile, PERM_BASIC, M_UFILES,
  "Xfile      �s��ӤH�ɮ�"},

  {"bin/list.so:List",PERM_VALID,- M_XMODE,
  "1List      �s�զW��"},

  menu_user, PERM_MENU + 'I', M_MMENU,
  "���U��T"
};


static MENU menu_user[] =
{
  menu_reg, 0, M_XMENU,
  "Setting    ���U�γ]�w�ӤH��T",

  {u_lock, PERM_BASIC, M_XMODE,
  "Lock       ��w�ù�"},

  {"bin/memorandum.so:Memorandum", PERM_VALID, -M_XMODE,
  "Note       �Ƨѿ�"},

  {"bin/pnote.so:main_note", PERM_VALID, - M_XMODE,
  "PNote      �ӤH������"},
/*
  {"bin/adminutil.so:user_check_bm", PERM_BM, - M_XMODE,
  "CheckBM    ���D�T�{"},

#ifdef	HAVE_CLASSTABLEALERT
  {"bin/classtable.so:ClassTable",PERM_VALID,-M_XMODE,
  "Table      �ӤH�\\�Ҫ�"},
#endif

*/
#ifdef	HAVE_CLASSTABLEALERT
  {"bin/classtable2.so:main_classtable",PERM_VALID,-M_XMODE,
  "2Table     �s���ӤH�\\�Ҫ�"},
#endif

  {view_login_log, PERM_BASIC, M_READA,
  "ViewLog    �˵��W������"},

  {menu_service, PERM_MENU + 'S', M_UMENU,
  "�ӤH�]�w"}
};


/* ----------------------------------------------------- */
/* tool menu					         */
/* ----------------------------------------------------- */

static MENU menu_service[];

/*
static MENU menu_tool[] =
{

  {"bin/bbcall.so:main_bbcall", PERM_INTERNET, - M_XMODE,
  "BBcall     �}�k�Y�}�����"},

  {"bin/pydict.so:main_pydict", PERM_INTERNET, - M_XMODE,
  "PyDict     PyDict �^�~�~�^�r��"},

  {"bin/dictd.so:main_dictd", PERM_INTERNET, - M_XMODE,
  "Dictd      Dictd ���^��r��"},

   {"bin/dreye.so:main_dreye", PERM_INTERNET, - M_XMODE,
  "DrEye      Dr.Eye �^�~�~�^�r��"},

  {"bin/railway.so:main_railway", PERM_INTERNET, - M_XMODE,
  "Railway    �x�K�����ɨ��d��"},


  {"bin/news_viewer.so:main_news",PERM_VALID, - M_XMODE,
  "NewsReader �s�D�\\Ū�u��"},

  {"bin/ueequery.so:x_ueequery", 0, - M_XMODE,
  "Ueequery   �p�ҭ���ҩm�W�d�]"},

  {"bin/ueequery2.so:x_ueequery2", 0, - M_XMODE,
  "ClassUee   �p�ҾǮլ�t�d�]"},

  {"bin/icq.so:main_icq", PERM_VALID, - M_XMODE,
  "ICQMsg     ICQ²�T�ǰe"},

  {"bin/star.so:main_star", 0, - M_XMODE,
  "Horoscope  �P�y�B�դ��R"},

#ifdef	HAVE_PERSON_DATA
  {"bin/fortune.so:main_fortune", PERM_VALID, - M_XMODE,
  "Lucky      �B�չw���t��"},
#endif


  {menu_service, PERM_MENU + 'D', M_UMENU,
  "�W�Ȥu��"}

};

*/

/* ----------------------------------------------------- */
/* game menu					         */
/* ----------------------------------------------------- */

static MENU menu_game[] =
{
  {"bin/bj.so:BlackJack",PERM_VALID, - M_XMODE,
  "BlackJack  " NICKNAME "�³ǧJ"},

  {"bin/guessnum.so:fightNum", 0, - M_XMODE,
  "FightNum   �Ʀr�j�M��"},

  {"bin/guessnum.so:guessNum", 0, - M_XMODE,
  "GuessNum   �̥ʲq�Ʀr"},

  {"bin/mine.so:Mine",PERM_VALID, - M_XMODE,
  "Mine       " NICKNAME "��a�p"},

  {"bin/pip.so:p_pipple",PERM_VALID,- M_XMODE,
  "Pip        " NICKNAME "�԰���"},

  {"bin/km.so:main_km", PERM_VALID, - M_XMODE,
  "Km         �թ���"},

/*
  {"bin/puzzle.so:NumPad",PERM_SYSOP,- M_XMODE,
  "NumPad     ���z���L"},
*/

#ifdef  HAVE_GAME_QKMJ
  {"bin/qkmj.so:main_qkmj", PERM_VALID, - M_XMODE,
  "Qkmj       ���ꪺ���"},
#endif

  {menu_service, PERM_MENU + 'F', M_UMENU,
  "�C����"}

};

/* ----------------------------------------------------- */
/* yzu menu					         */
/* ----------------------------------------------------- */

static MENU menu_special[] =
{
/*
  {"bin/grade.so:main_grade",PERM_BASIC, - M_XMODE,
  "Grade        ���Z�d�ߨt��"},

  {"bin/graduate.so:main_gpoint",PERM_BASIC, - M_XMODE,
  "CheckGrade   ���~�Ǥ��ˬd"},

  {"bin/sec_hand.so:sec_hand", PERM_VALID , - M_XMODE,
  "SecondHand   �դ��G�⥫��"},
*/
  {"bin/netwhois.so:x_whois", PERM_VALID , - M_XMODE,
  "Netwhois     IPDN�d��"},

#ifdef	HAVE_BBSNET
  {"bin/bbsnet.so:Bbsnet", PERM_SYSOP, - M_XMODE,
  "Jump         ���i�ǰe"},
#endif

  {"bin/personal.so:personal_apply",PERM_VALID, - M_XMODE,
  "PBApply      �ӽЭӤH�ݪO"}, 

  {"bin/bank.so:bank_main", PERM_VALID, - M_XMODE,
  "Bank       �@�Ȧ�"},

  {"bin/shop.so:shop_main", PERM_VALID, - M_XMODE,
  "Shop       �@�ө�"},

#ifdef HAVE_SONG
  {menu_song, PERM_VALID, M_XMENU,
  "Request      �I�q�t��"},
#endif

  {resetbrd, PERM_BASIC, M_XMODE,
  "CameraReset  �������]"},

  {menu_service, PERM_MENU + 'N', M_UMENU,
  "�[�ȪA��"}
};



/* ----------------------------------------------------- */
/* song menu                                             */
/* ----------------------------------------------------- */
#ifdef HAVE_SONG
static MENU menu_song[] =
{
  {"bin/song.so:XoSongMain", PERM_VALID, - M_XMODE,
  "Request       �I�q�q��"},

  {"bin/song.so:XoSongLog", PERM_VALID, - M_XMODE,
  "OrderSongs    �I�q����"},

  {"bin/song.so:XoSongSub", PERM_VALID, - M_XMODE,
  "Submit        ��Z�M��"},

  {menu_special, PERM_MENU + 'R', M_XMENU,
  "���I�I�q"}
};
#endif


/* ----------------------------------------------------- */
/* service menu					         */
/* ----------------------------------------------------- */

/* Thor.990224: �}��~���ɭ� */
static MENU menu_service[] =
{

/* 090923.cache: �ϥ��̭������ѱ��F */
/*
  {menu_tool, PERM_BASIC, M_XMENU,
  "Tool      �i �W�Ȥu��� �j"},
*/
  {menu_user, 0, M_UMENU,
  "User      �i �ӤH�u��� �j"},

  {menu_special, PERM_VALID, M_XMENU,
  "Cache     �i �[�ȪA�Ȱ� �j"},

  {menu_game, PERM_BASIC, M_XMENU,
  "Game      �i �C������� �j"},

#ifdef	HAVE_INFO
  {Information, 0, M_BOARD,
  INFO_EMPTY},
#endif

#ifdef	HAVE_STUDENT  
  {Student, 0, M_BOARD,
  STUDENT_EMPTY},
#endif

#ifdef	HAVE_ACTIVITY
  {Activity, 0, M_BOARD,
  ACTIVITY_EMPTY},
#endif

/* 091007.cache: �ԤH�鲼�S�N�q... */
  
  {"bin/newboard.so:XoNewBoard", PERM_VALID, - M_XMODE,
  "Cosign    �i �s�p�ӽа� �j"},

  {"bin/vote.so:SystemVote" , PERM_POST, - M_XMODE,
  "SystemVote�i �t�Χ벼�� �j"},

  {system_result, 0, M_READA,
  "Voteresult�i�t�Χ벼���G�j"},

/*
#ifdef HAVE_SONG
  {menu_song, PERM_VALID, M_XMENU,
  "Song      �i �I�q�t�ΰ� �j"},
#endif
*/
  {menu_main, PERM_MENU + 'G', M_UMENU,
  "�ڤj�A��"}
};

/* ----------------------------------------------------- */
/* main menu						 */
/* ----------------------------------------------------- */

#ifdef	HAVE_CHANGE_SKIN	
static int
sk_windtop_init()
{
  s_menu = menu_windtop;
  skin = 1;
  vmsg("DEBUG:DreamBBS");
  return 0;
}

MENU skin_main[] =
{
  {sk_windtop_init,PERM_SYSOP, M_XMODE,
  "DreamBBS   �w�]���t��"},

  {menu_main, PERM_MENU + 'W', M_MMENU,
  "�������"}
};
#endif

static int
Gem()
{
  XoGem("gem/.DIR", "", ((HAS_PERM(PERM_SYSOP|PERM_BOARD|PERM_GEM)) ? GEM_SYSOP : GEM_USER));
  return 0;
}

static MENU menu_main[] =
{
  {menu_admin, PERM_ADMIN, M_ADMIN,
  "0Admin    �i �t�κ��@�� �j"},

  Gem, 0, M_GEM,
  "Announce  �i ��ؤ��G�� �j",

  {Boards, 0, M_BOARD,
  "Boards    �i \033[1;33m�G�i�Q�װ�\033[m �j"},

  {Class, 0, M_CLASS,
  "Class     �i \033[1;33m���հQ�װ�\033[m �j"},

#ifdef	HAVE_PROFESS
  {Profess, 0, M_PROFESS,
  "Profession�i \033[1;33m�M�~�Q�װ�\033[m �j"},
#endif

#ifdef	HAVE_FAVORITE
  {MyFavorite,PERM_VALID,M_CLASS,
  "Favorite  �i \033[1;32m�ڪ��̷R��\033[m �j"},
#endif

#ifdef HAVE_SIGNED_MAIL
  {menu_mail, PERM_BASIC , M_MAIL, /* Thor.990413: �Y����m_verify, guest�N�S����椺�e�o */
  "Mail      �i �H��u��� �j"},
#else
  {menu_mail, PERM_BASIC , M_MAIL,
  "Mail      �i �p�H�H��� �j"},
#endif

  {menu_talk, 0, M_TMENU,
  "Talk      �i �𶢲�Ѱ� �j"},

  {menu_service, PERM_BASIC, M_XMENU,
  "Dream     �i �ڤj�A�Ȱ� �j"},

  /* lkchu.990428: ���n����b�ӤH�u��� */
  {menu_xyz, 0, M_SMENU,
  "Xyz       �i �t�θ�T�� �j"},

#ifdef	HAVE_CHANGE_SKIN  
  {skin_main, PERM_SYSOP,M_XMENU,
  "2Skin     �i ��ܤ����� �j"},
#endif

  {goodbye, 0, M_XMODE,
  "Goodbye   �i�A�O" NICKNAME "�j"},

  {NULL, PERM_MENU + 'B', M_MMENU,
  "�D�\\���"}
};

#ifdef	TREAT
static int
goodbye1()
{
  switch (vans("G)�A�O" NICKNAME " Q)�����H[Q] "))
  {
  case 'g':
  case 'y':
    return 12345;
    break;    

  case 'q':
  default:
    break;
  }

  clear();
  outs("�� ������z���q���չϧ������A�� ��\n");
  bell();
  vkey();
  outs("�� ����  �F�A����  ^O^ �A�ڤ��j�a���z�M�H�`�ּ� ��\n");
  bell();
  vkey();
  return 12345;
}



static MENU menu_treat[] =
{
  goodbye1, 0, M_XMODE,
  "Goodbye   �i�A�O" NICKNAME "�j",

  NULL, PERM_MENU + 'G', M_MMENU,
  "�D�\\���"
};
#endif

static
int count_len(data)
  char *data;
{
  int len;
  char *ptr,*tmp;
  ptr = data;
  len = strlen(data);

  while(ptr)
  {
    ptr = strstr(ptr,"\033");
    if(ptr)
    {
      for(tmp=ptr;*tmp!='m';tmp++);
      len -= (tmp-ptr+1);
      ptr = tmp+1;
    }
  }
  return len;
}


char *
check_info(char *input)
{
#if defined(HAVE_INFO) || defined(HAVE_STUDENT) || defined(HAVE_ACTIVITY)
  BRD *brd;
#endif  
  char *name = NULL;
  name = input;
#ifdef	HAVE_INFO  
  if(!strcmp(input,INFO_EMPTY))
  {
     brd = bshm->bcache + brd_bno(BRD_BULLETIN);
     if(brd)
     {
       check_new(brd);
       if(brd->blast > brd_visit[brd_bno(BRD_BULLETIN)])
         name = INFO_HAVE;
     }
  }
#endif  
#ifdef	HAVE_STUDENT
  if(!strcmp(input,STUDENT_EMPTY))
  {
     brd = bshm->bcache + brd_bno(BRD_SBULLETIN);
     if(brd)
     {
       check_new(brd);
       if(brd->blast > brd_visit[brd_bno(BRD_SBULLETIN)])
         name = STUDENT_HAVE;
     }
  }
#endif
#ifdef	HAVE_ACTIVITY
  if(!strcmp(input,ACTIVITY_EMPTY))
  {
    brd = bshm->bcache + brd_bno(BRD_ABULLETIN);
    if(brd)
    {
      check_new(brd);
      if(brd->blast > brd_visit[brd_bno(BRD_ABULLETIN)])
        name = ACTIVITY_HAVE;
    }
  }
#endif
  return name;
}


void
menu()
{
  MENU *menu, *mptr, *table[17];
  usint level, mode;
  int cc=0, cx=0 , refilm=0;	/* current / previous cursor position */
  int max=0, mmx;			/* current / previous menu max */
  int cmd=0, depth,count;
  char *str,item[60];

  mode = MENU_LOAD | MENU_DRAW | MENU_FILM;
#ifdef	TREAT
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

#ifdef	MENU_VERBOSE
	  if (max < 0)		/* �䤣��A�X�v�����\��A�^�W�@�h�\��� */
	  {
	    menu = (MENU *) menu->func;
	    continue;
	  }
#endif

	  break;
	}
	if (cc && !(cc & level))
	  continue;
	if(!strncmp(menu->desc,OPT_OPERATOR,strlen(OPT_OPERATOR)) && !(supervisor || !str_cmp(cuser.userid,ELDER) || !str_cmp(cuser.userid,STR_SYSOP)))
	  continue;

	table[++max] = menu;
      }

      if (mmx < max)
	mmx = max;

#ifndef	TREAT
      if ((depth == 0) && (cutmp->ufo & UFO_BIFF))
	cmd = 'M';
      else if((depth == 0) && (cutmp->ufo & UFO_BIFFN))
        cmd = 'U';
      else
#endif
        cmd = cc ^ PERM_MENU;	/* default command */
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
      //prints("\n\033[30;47m     �ﶵ         �ﶵ����                         �ʺA�ݪO                   \033[m\n");
      mode = 0;
      count = 0;
      while(count<20) 
	item_lenth[count++] = 0;
      do
      {                
	move(MENU_YPOS + mode, MENU_XPOS + 2);
	if (mode <= max)
	{
	  mptr = table[mode];
	  str = check_info(mptr->desc);
          sprintf(item,"\033[m(\033[1;36m%c\033[m)%s",*str,str+1);
          outs(item);
          item_lenth[mode]=(cuser.ufo2 & UFO2_COLOR) ? strlen(item)-count_len(str)-2 : 0;
	  /*outs("(\033[1;36m");
	  outc(*str++);
	  outs("\033[m)");
	  outs(str);*/
	}
	clrtohol();
      } while (++mode <= mmx);
      if(refilm)
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
      
    case KEY_HOME:
      cc = 0;
      break;

    case KEY_UP:
      if (--cc >= 0)
	break;

    case KEY_END:
      cc = max;
      break;

    case '\n':
    case KEY_RIGHT:
      mptr = table[cc];
      cmd = mptr->umode;
#if 1
     /* Thor.990212: dynamic load , with negative umode */
      if(cmd < 0)
      {
        void *p = DL_get(mptr->func);
        if(!p) break;
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
	int (*func) ();

	func = mptr->func;
	mode = (*func) ();
      }

#ifdef	HAVE_CHANGE_SKIN
      if (skin)
      {
        vmsg("DEBUG:SKIN");
        vmsg("123");
        //(*s_menu)();
        return;
        vmsg("1234");
      }

#endif

#ifdef	TREAT
      if(mode == 12345)
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
	outz(footer);
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
      every_Z();		/* Thor: ctrl-Z everywhere */
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

