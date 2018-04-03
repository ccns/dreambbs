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
#define INFO_EMPTY	"Info      【 \033[1;36m校方公告區\033[m 】"
#define	INFO_HAVE	"Info      【 \033[41;33;1m快進來看看\033[m 】"
#endif

#ifdef	HAVE_STUDENT
#define	STUDENT_EMPTY	"1Student  【 \033[1;36m學生公告區\033[m 】"
#define STUDENT_HAVE	"1Student  【 \033[41;33;1m快進來看看\033[m 】"
#endif

#ifdef	HAVE_ACTIVITY
#define	ACTIVITY_EMPTY	"2Activity 【 \033[1;36m活動公告區\033[m 】"
#define	ACTIVITY_HAVE	"2Activity 【 \033[41;33;1m快進來看看\033[m 】"
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
  sprintf(buf, "系統負載 %.2f %.2f %.2f", load[0], load[1], load[2]);
  vmsg(buf);
  return XEASY;
}


/* ----------------------------------------------------- */
/* 離開 BBS 站						 */
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
  outs("\033[1;37;45m ●  " BOARDNAME " 留 言 板  ● \n\n");
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
    outs("\n請留言 (至多三行)，按[Enter]結束");
    for (i = 0; (i < 3) &&
      vget(16 + i, 0, "：", buf[i], 70, DOECHO); i++);
    cc = vans("(S)存檔觀賞 (E)重新來過 (Q)算了？[S] ");
    if (cc == 'q' || i == 0)
      return;
  } while (cc == 'e');

  time(&(pad.tpad));

  str = pad.msg;

  sprintf(str, "\033[1;37;46mΥ\033[34;47m %s \033[33m(%s)", cuser.userid, cuser.username);
  len = strlen(str);
  strcat(str, " \033[30;46m" + (len & 1));
  
  for (i = len >> 1; i < 41; i++)
    strcat(str,"▄");    
  sprintf(str2, "\033[34;47m %.14s \033[37;46mΥ\033[m\n%-70.70s\n%-70.70s\n%-70.70s\n",
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
    if(!(ans = vans("G)再別" NICKNAME " M)報告站長 N)留言板 Q)取消？[Q] ")))
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
    if(HAS_PERM(PERM_POST)) /* Thor.990118: 要能post才能留言, 提高門檻 */
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
  prints("       \033[1;31m ●       \033[1;36m ┌─┐┌─┐┌─┐┌─╮ ┌─╮┌╮┐┌─┐\n"
  "      \033[1;31m●\033[1;37m○\033[1;33m●\033[1;37m═══\033[1;36m│  ┬│  ││  ││  │ │ ═ └  ┘│═╡\033[1;37m════\n"
  "       \033[1;33m ●        \033[1;34m└─┤└─┘└─┘└─╯ └─╯ └┘ └─┘\033[m\n");
  prints("Dear \033[32m%s(%s)\033[m，別忘了再度光臨【 %s 】\n"
    "以下是您在站內的註冊資料:\n",
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
  int color = 44; //090911.cache: 太花了固定一種顏色 
#endif
#if 1
  char *newyear[6]={ BOARDNAME "祝大家新年快樂        ",
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
      cutmp->ufo = ufo ^ UFO_BIFF;     /* 看過就算 */
    if (ufo & UFO_BIFFN)
      cutmp->ufo = ufo ^ UFO_BIFFN;     /* 看過就算 */
  }
  else
  {
    if (ufo & UFO_BIFF)
    {
      mid = NEWMAILMSG; // 你有新情書 
      spc = 15;
    }
    else if (ufo & UFO_BIFFN) 
    {
      mid = NEWPASSMSG; // 你有新留言 
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
        sprintf(ttl,"距 2006 年還剩 %02d 時 %02d 分 %02d 秒",hour,min,sec);
        spc = strlen(ttl);
        mid = ttl;  
        mid[spc] = '\0';
      }

    }
#endif    
#if 0
    else if(aprilfirst)
    {
        sprintf(ttl,"據統計，目前有 %d 個鄉民受騙",ushm->avgload);
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
  prints("\033[1;%2d;37m【%s】%s\033[33m%s\033[1;%2d;37m%s\033[37m看板《%s》\033[m\n",
    color, title, buf,mid , color, buf + ufo, currboard);
#else
  prints("\033[1;46;37m【%s】%s\033[33m%s\033[46m%s\033[37m看板《%s》\033[m\n",
    title, buf, mid, buf + ufo, currboard);
#endif
}


/* ------------------------------------- */
/* 動畫處理				 */
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
      "天一二三四五六" + (ptime->tm_wday << 1));

    uptime = now + 86400 - ptime->tm_hour * 3600 -
      ptime->tm_min * 60 - ptime->tm_sec;
  }
  ufo = (now - (uptime - 86400)) / 60;

  /* Thor.980913: 註解: 最常見呼叫 movie()的時機是每次更新 film, 在 60秒以上,
                        故不需針對 xx:yy 來特別作一字串儲存以加速 */

  sprintf(footer, "\033[0;34;46m%s%d:%02d] \033[30;47m 目前站上有\033[31m%4d\033[30m 人, 我是 \033[31m%-12s\033[30m [呼叫/訊息]\033[31m%s\033[m",
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
  "Top        線上 Load"},

  {"bin/adminutil.so:psaux", PERM_SYSOP, - M_XMODE,
  "Ps         線上執行程式"},

  {"bin/adminutil.so:dmesg", PERM_SYSOP, - M_XMODE,
  "Dmesg      系統訊息"},

  {"bin/adminutil.so:df", PERM_SYSOP, - M_XMODE,
  "FileSys    磁碟系統"},

  {x_sysload, PERM_ADMIN, M_SYSTEM,
  "Load       系統負載"},

  {menu_admin, PERM_MENU + 'T', M_XMENU,
  "系統負載"}
};

static MENU menu_boardadm[] =
{
  {m_newbrd, PERM_BOARD, M_SYSTEM,
  "NewBoard   開闢新看板"},

  {a_editbrd, PERM_BOARD, M_SYSTEM,
  "ConfigBrd  設定看板"},

  {m_bmset, PERM_BOARD, M_SYSTEM,
  "BMset      設定版主權限"},

  {BanMail,PERM_BOARD|PERM_SYSOP, M_BANMAIL,
  "FireWall   擋信列表"},

  {"bin/adminutil.so:bm_check",PERM_BOARD|PERM_SYSOP, - M_XMODE,
  "Manage     板主確認"},

  {"bin/adminutil.so:m_expire", PERM_BOARD|PERM_SYSOP, - M_XMODE,
  "DExpire    清除看板刪除文章"},

  {"bin/adminutil.so:mail_to_bm", PERM_SYSOP, - M_XMODE,
  "ToBM       寄信給板主"},

  {"bin/adminutil.so:mail_to_all", PERM_SYSOP, - M_XMODE,
  "Alluser    系統通告"},
  
    "bin/personal.so:personal_admin", PERM_BOARD|PERM_SYSOP, - M_XMODE,
  "Personal   個人板審核",

  {menu_admin, PERM_MENU + 'N', M_XMENU,
  "看板總管"}
};

static MENU menu_accadm[] =
{
  {m_user, PERM_ACCOUNTS, M_SYSTEM,
  "User       使用者資料"},

  {"bin/bank.so:money_back", PERM_ACCOUNTS, - M_XMODE,
  "GetMoney   匯入舊夢幣"},

#ifdef	HAVE_SONG
  {"bin/song.so:AddRequestTimes",PERM_KTV , - M_XMODE,
  "Addsongs   增加點歌次數"},
#endif

  {"bin/passwd.so:new_passwd", PERM_SYSOP, - M_XMODE,
  "Password   重送新密碼"},

#ifdef  HAVE_REGISTER_FORM
  {m_register, PERM_ACCOUNTS, M_SYSTEM,
  "1Register  審核註冊表單"},
#endif

#ifdef HAVE_OBSERVE_LIST
  {"bin/observe.so:Observe_list", PERM_SYSOP|PERM_BOARD, - M_XMODE,
  "2Observe   系統觀察名單"},
#endif
 
  {menu_admin, PERM_MENU + 'U', M_XMENU,
  "註冊總管"}
};

static MENU menu_settingadm[] =
{

  {"bin/adminutil.so:m_xfile", PERM_SYSOP, - M_XFILES,
  "File       編輯系統檔案"},
  
  {"bin/adminutil.so:m_xhlp", PERM_SYSOP, - M_XFILES,
  "Hlp        編輯說明檔案"},

  {"bin/admin.so:Admin",PERM_SYSOP, - M_XMODE,
  "Operator   系統管理員列表"},

  {"bin/chatmenu.so:Chatmenu",PERM_CHATROOM|PERM_SYSOP,- M_XMODE,
  "Chatmenu   " CHATROOMNAME "動詞"},

  {"bin/violate.so:Violate",PERM_SYSOP,- M_XMODE,
  "Violate    處罰名單"},
  
  {"bin/adminutil.so:special_search",PERM_SYSOP, - M_XMODE,
  "XSpecial   特殊搜尋"},

  {"bin/adminutil.so:update_all",PERM_SYSOP, - M_XMODE,
  "Database   系統資料庫更新"},  
  
  {menu_admin, PERM_MENU + 'X', M_XMENU,
  "系統資料"}
};

/* ----------------------------------------------------- */
/* reset menu                                            */
/* ----------------------------------------------------- */
static MENU menu_reset[] =
{
  {menu_load, PERM_ADMIN, M_XMENU,
  "Load       系統負載"},

  {"bin/adminutil.so:reset1", PERM_BOARD, - M_XMODE,
  "Camera     動態看板"},

  {"bin/adminutil.so:reset2", PERM_BOARD, - M_XMODE,
  "Group      分類群組"},

  {"bin/adminutil.so:reset3", PERM_SYSOP, - M_XMODE,
  "Mail       寄信收信轉信"},

  {"bin/adminutil.so:reset4", PERM_ADMIN, - M_XMODE,
  "Killbbs    清除不正常 BBS"},

  {"bin/adminutil.so:reset5", PERM_BOARD, - M_XMODE,
  "Firewall   擋信列表"},

  {"bin/adminutil.so:reset6", PERM_CHATROOM, - M_XMODE,
  "Xchatd     重開聊天室"},

  {"bin/adminutil.so:reset7", PERM_SYSOP, - M_XMODE,
  "All        全部"},

  {menu_admin, PERM_MENU + 'K', M_XMENU,
  "系統重置"}
};


/* ----------------------------------------------------- */
/* administrator's maintain menu			 */
/* ----------------------------------------------------- */


static MENU menu_admin[] =
{

  {menu_accadm, PERM_ADMIN, M_XMENU,
  "Acctadm    註冊總管功\能"},

  {menu_boardadm, PERM_BOARD, M_XMENU,
  "Boardadm   看板總管功\能"},

  {menu_settingadm, PERM_ADMIN, M_XMENU,
  "Data       系統資料庫設定"},

  {"bin/innbbs.so:a_innbbs", PERM_BOARD, - M_SYSTEM,
  "InnBBS     轉信設定"},

  {menu_reset, PERM_ADMIN, M_XMENU,
  "ResetSys   重置系統"},

#ifdef	HAVE_ADM_SHELL
  {x_csh, PERM_SYSOP, M_SYSTEM,
  "Shell      執行系統 Shell"},
#endif

#ifdef	HAVE_REPORT
  {m_trace, PERM_SYSOP, M_SYSTEM,
  "Trace      設定是否記錄除錯資訊"},
#endif

  {menu_main, PERM_MENU + 'A', M_ADMIN,
  "系統維護"}
};


/* ----------------------------------------------------- */
/* mail menu						 */
/* ----------------------------------------------------- */


static int
XoMbox()
{
  if (HAS_PERM(PERM_DENYMAIL))
     vmsg("您的信箱被鎖了！");
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
  "Read       閱\讀信件"},

  {m_send, PERM_INTERNET, M_SMAIL,
  "MailSend   站內寄信"},

#ifdef MULTI_MAIL  /* Thor.981009: 防止愛情幸運信 */
  {mail_list, PERM_INTERNET, M_SMAIL,
  "Group      群組寄信"},
#endif

  {"bin/contact.so:Contact", PERM_INTERNET, - M_XMODE,
  "Contact    聯絡名單"},

  {m_internet, PERM_INTERNET, M_SMAIL,
  "Internet   寄信到 Internet"},

  {m_setforward, PERM_INTERNET, M_SMAIL,
  "AutoFor    站內信自動轉寄"},

  {m_setmboxdir, PERM_INTERNET, M_SMAIL,
  "Fixdir     重建信箱索引"},

/*  
  {"bin/lovepaper.so:lovepaper", PERM_INTERNET, - M_XMODE,
  "LovePager  \033[1;31m情書產生器\033[m"},
*/
#ifdef HAVE_DOWNLOAD
  {m_zip, PERM_VALID, M_XMODE,
  "Zip        打包下載個人資料"},
#endif
/* 
#ifdef HAVE_SIGNED_MAIL
  {m_verify, PERM_VALID, M_XMODE,
  "Verify     驗證信件電子簽章"},
#endif
*/ 
  {mail_sysop, PERM_BASIC, M_SMAIL,
  "Yes Sir!   寄信給站長"},
/* 
#ifdef HAVE_MIME_TRANSFER
  {"bin/imap4mail.so:Imap4mail", PERM_INTERNET, - M_XMODE,
  "Mail       IMAP4郵件系統服務"},
#endif

  {"bin/pop3mail.so:Pop3mail", PERM_SYSOP, -M_XMODE,
  "Pop3       POP3 郵件系統服務"},
*/ 
  {menu_main, PERM_MENU + 'R', M_MMENU,	/* itoc.020829: 怕 guest 沒選項 */
  "電子郵件"}
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
  "Users      完全聊天手冊"},

  {t_pal, PERM_BASIC, M_PAL,
  "Friend     編輯好友名單"},

#ifdef	HAVE_BANMSG
  {t_banmsg, PERM_BASIC, M_XMODE,
  "Banmsg     拒收訊息名單"},
#endif
  {"bin/aloha.so:t_aloha", PERM_BASIC, - M_XMODE,
  "Aloha      上站通知名單"},

  {t_pager, PERM_BASIC, M_XMODE,
  "Pager      切換呼叫器"},

  {t_message, PERM_BASIC, M_XMODE,
  "Message    切換訊息"},

  {t_query, 0, M_QUERY,
  "Query      查詢網友"},

#if 0
  /* Thor.990220: 改採外掛 */
  {"bin/chat.so:t_chat", PERM_CHAT, - M_CHAT,
  "Chat       " NICKNAME CHATROOMNAME},
#endif

  {t_recall, PERM_BASIC, M_XMODE,
  "Write      回顧前幾次熱訊"},

#ifdef LOGIN_NOTIFY
  {t_loginNotify, PERM_PAGE, M_XMODE,
  "Notify     設定系統網友協尋"},
#endif
  {menu_main, PERM_MENU + 'U', M_UMENU,
  "休閒聊天"}
};


/* ----------------------------------------------------- */
/* System menu						 */
/* ----------------------------------------------------- */

static MENU menu_information[] = 
{

  {popmax, 0 , M_READA,
  "Login      上站次數排行榜"},

  {today, 0, M_READA,
  "Today      今日上線人次統計"},

  {yesterday, 0, M_READA,
  "Yesterday  昨日上線人次統計"},

  {day, 0, M_READA,
  "0Day       本日十大熱門話題"},

  {week, 0, M_READA,
  "1Week      本週五十大熱門話題"},

  {month, 0, M_READA,
  "2Month     本月百大熱門話題"},

  {year, 0, M_READA,
  "3Year      本年度百大熱門話題"},
/*
#ifdef  HAVE_PERSON_DATA
  {count_birth, 0, M_READA,
  "4Birthday  本月壽星統計"},

  {count_age, 0, M_READA,
  "5YearsOld  使用者年齡統計"},

  {count_star, 0, M_READA,
  "6Star      使用者星座統計"},
#endif
*/
  menu_xyz, PERM_MENU + 'L', M_MMENU,
  "統計資料"
};


static MENU menu_xyz[] =
{
  menu_information, 0, M_XMENU,
  "Tops       " NICKNAME "排行榜",

  {version,0,M_READA,
  "Version    源碼發展資訊"},

  {"bin/xyz.so:x_siteinfo", 0, - M_READA,
  "Xinfo      系統程式資訊"},

  {pad_view, 0, M_READA,
  "Note       觀看心情留言板"},

  {welcome, 0, M_READA,
  "Welcome    觀看歡迎畫面"},

  {history, 0, M_READA,
  "History    本站歷史軌跡"},

  {menu_main, PERM_MENU + 'T', M_SMENU,
  "系統資訊"}
};

/* ----------------------------------------------------- */
/* User menu                                             */
/* ----------------------------------------------------- */

static MENU menu_reg[] = 
{

  {u_info, PERM_BASIC, M_XMODE,
  "Info       設定個人資料與密碼"},

  {u_addr, PERM_BASIC, M_XMODE,
  "Address    填寫電子信箱及認證"},

  u_verify, PERM_BASIC, M_UFILES,
  "Verify     填寫《註冊認證碼》",

#ifdef	HAVE_REGISTER_FORM
  {u_register, PERM_BASIC, M_UFILES,
  "Register   填寫《註冊申請單》"},
#endif

  {u_setup, PERM_VALID, M_UFILES,
  "Mode       設定操作模式"},

  {ue_setup, 0, M_UFILES,
  "Favorite   個人喜好設定"},

  {u_xfile, PERM_BASIC, M_UFILES,
  "Xfile      編輯個人檔案"},

  {"bin/list.so:List",PERM_VALID,- M_XMODE,
  "1List      群組名單"},

  menu_user, PERM_MENU + 'I', M_MMENU,
  "註冊資訊"
};


static MENU menu_user[] =
{
  menu_reg, 0, M_XMENU,
  "Configure  註冊及設定個人資訊",

  {u_lock, PERM_BASIC, M_XMODE,
  "Lock       鎖定螢幕"},

  {"bin/memorandum.so:Memorandum", PERM_VALID, -M_XMODE,
  "Note       備忘錄"},

  {"bin/pnote.so:main_note", PERM_VALID, - M_XMODE,
  "PNote      個人答錄機"},
/*
  {"bin/adminutil.so:user_check_bm", PERM_BM, - M_XMODE,
  "CheckBM    版主確認"},

#ifdef	HAVE_CLASSTABLEALERT
  {"bin/classtable.so:ClassTable",PERM_VALID,-M_XMODE,
  "Table      個人功\課表"},
#endif

*/
#ifdef	HAVE_CLASSTABLEALERT
  {"bin/classtable2.so:main_classtable",PERM_VALID,-M_XMODE,
  "2Table     新版個人功\課表"},
#endif

  {view_login_log, PERM_BASIC, M_READA,
  "ViewLog    檢視上站紀錄"},

  {menu_service, PERM_MENU + 'C', M_UMENU,
  "個人設定"}
};


/* ----------------------------------------------------- */
/* tool menu					         */
/* ----------------------------------------------------- */

static MENU menu_service[];

/* ----------------------------------------------------- */
/* game menu					         */
/* ----------------------------------------------------- */

static MENU menu_game[] =
{
  {"bin/bj.so:BlackJack",PERM_VALID, - M_XMODE,
  "BlackJack  " NICKNAME "黑傑克"},

  {"bin/guessnum.so:fightNum", PERM_VALID, - M_XMODE,
  "FightNum   數字大決戰"},

  {"bin/guessnum.so:guessNum", PERM_VALID, - M_XMODE,
  "GuessNum   傻瓜猜數字"},

  {"bin/mine.so:Mine",PERM_VALID, - M_XMODE,
  "Mine       " NICKNAME "踩地雷"},

  {"bin/pip.so:p_pipple",PERM_VALID,- M_XMODE,
  "Pip        " NICKNAME "戰鬥雞"},

  {menu_service, PERM_MENU + 'F', M_UMENU,
  "遊戲休閒"}

};

/* ----------------------------------------------------- */
/* yzu menu					         */
/* ----------------------------------------------------- */

static MENU menu_special[] =
{

  {"bin/personal.so:personal_apply",PERM_VALID, - M_XMODE,
  "PBApply      申請個人看板"}, 

  {"bin/bank.so:bank_main", PERM_VALID, - M_XMODE,
  "Bank       　銀行"},

  {"bin/shop.so:shop_main", PERM_VALID, - M_XMODE,
  "Pay        　商店"},

#ifdef HAVE_SONG
  {menu_song, PERM_VALID, M_XMENU,
  "Request      點歌系統"},
#endif

  {resetbrd, PERM_ADMIN, M_XMODE,
  "CameraReset  版面重設"},

  {menu_service, PERM_MENU + 'R', M_UMENU,
  "加值服務"}
};



/* ----------------------------------------------------- */
/* song menu                                             */
/* ----------------------------------------------------- */
#ifdef HAVE_SONG
static MENU menu_song[] =
{
  {"bin/song.so:XoSongMain", PERM_VALID, - M_XMODE,
  "Request       點歌歌本"},

  {"bin/song.so:XoSongLog", PERM_VALID, - M_XMODE,
  "OrderSongs    點歌紀錄"},

  {"bin/song.so:XoSongSub", PERM_VALID, - M_XMODE,
  "Submit        投稿專區"},

  {menu_special, PERM_MENU + 'R', M_XMENU,
  "網呼點歌"}
};
#endif


/* ----------------------------------------------------- */
/* service menu					         */
/* ----------------------------------------------------- */

/* Thor.990224: 開放外掛界面 */
static MENU menu_service[] =
{

/* 090923.cache: 反正裡面都註解掉了 */
/*
*  {menu_tool, PERM_BASIC, M_XMENU,
*  "Tool      【 超值工具區 】"},
*/
  {menu_user, 0, M_UMENU,
  "User      【 個人工具區 】"},

  {menu_special, PERM_VALID, M_XMENU,
  "Bonus     【 加值服務區 】"},

  {menu_game, PERM_VALID, M_XMENU,
  "Game      【 遊戲體驗區 】"},

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

/* 091007.cache: 拉人灌票沒意義... */
  
  {"bin/newboard.so:XoNewBoard", PERM_VALID, - M_XMODE,
  "Cosign    【 連署申請區 】"},

  {"bin/vote.so:SystemVote" , PERM_POST, - M_XMODE,
  "Vote      【 系統投票區 】"},

  {system_result, 0, M_READA,
  "Result    【系統投票結果】"},

/*
#ifdef HAVE_SONG
  {menu_song, PERM_VALID, M_XMENU,
  "Song      【  點歌系統區  】"},
#endif
*/
  {menu_main, PERM_MENU + 'U', M_UMENU,
   NICKNAME "服務"}
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
  "DreamBBS   預設的系統"},

  {menu_main, PERM_MENU + 'W', M_MMENU,
  "介面選單"}
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
  "0Admin    【 系統維護區 】"},

  Gem, 0, M_GEM,
  "Announce  【 精華公佈欄 】",

  {Boards, 0, M_BOARD,
  "Boards    【 \033[1;33m佈告討論區\033[m 】"},

  {Class, 0, M_CLASS,
  "Class     【 \033[1;33m分組討論區\033[m 】"},

#ifdef	HAVE_PROFESS
  {Profess, 0, M_PROFESS,
  "Profession【 \033[1;33m專業討論區\033[m 】"},
#endif

#ifdef	HAVE_FAVORITE
  {MyFavorite, PERM_BASIC, M_CLASS,
  "Favorite  【 \033[1;32m我的最愛區\033[m 】"},
#endif

#ifdef HAVE_SIGNED_MAIL
  {menu_mail, PERM_BASIC , M_MAIL, /* Thor.990413: 若不用m_verify, guest就沒有選單內容囉 */
  "Mail      【 信件工具區 】"},
#else
  {menu_mail, PERM_BASIC , M_MAIL,
  "Mail      【 私人信件區 】"},
#endif

  {menu_talk, 0, M_TMENU,
  "Talk      【 休閒聊天區 】"},

  {menu_service, PERM_BASIC, M_XMENU,
  "DService  【 " NICKNAME "服務區 】"},

  /* lkchu.990428: 不要都塞在個人工具區 */
  {menu_xyz, 0, M_SMENU,
  "Xyz       【 系統資訊區 】"},

#ifdef	HAVE_CHANGE_SKIN  
  {skin_main, PERM_SYSOP,M_XMENU,
  "2Skin     【 選擇介面區 】"},
#endif

  {goodbye, 0, M_XMODE,
  "Goodbye   【再別" BOARDNAME "】"},

  {NULL, PERM_MENU + 'B', M_MMENU,
  "主功\能表"}
};

#ifdef	TREAT
static int
goodbye1()
{
  switch (vans("G)再別" NICKNAME " Q)取消？[Q] "))
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
  outs("※ 偵測到您的電腦試圖攻擊伺服器 ※\n");
  bell();
  vkey();
  outs("※ 哈哈  騙你的啦  ^O^ ，" BOARDNAME "祝您愚人節快樂 ※\n");
  bell();
  vkey();
  return 12345;
}



static MENU menu_treat[] =
{
  goodbye1, 0, M_XMODE,
  "Goodbye   【再別" NICKNAME "】",

  NULL, PERM_MENU + 'G', M_MMENU,
  "主功\能表"
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
	  if (max < 0)		/* 找不到適合權限之功能，回上一層功能表 */
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
      //prints("\n\033[30;47m     選項         選項說明                         動態看板                   \033[m\n");
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
        /* Thor.980826: 用 outz 就不用 move + clrtoeol了 */
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

