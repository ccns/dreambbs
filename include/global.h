/*-------------------------------------------------------*/
/* global.h     ( NTHU CS MapleBBS Ver 3.02 )            */
/*-------------------------------------------------------*/
/* target : global definitions & variables               */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/
#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#ifdef  _MAIN_C_
# define VAR
# define INI(x)         = x
#else
# define VAR            extern
# define INI(x)
#endif

/* ----------------------------------------------------- */
/* GLOBAL DEFINITION                                     */
/* ----------------------------------------------------- */
#define FN_INNBBS_LOG           "innd/innbbs.log"       /* 所有 [轉信] 記錄 */

/* ----------------------------------------------------- */
/* 個人目錄檔名設定                                      */
/* ----------------------------------------------------- */
#define FN_ACCT                 ".ACCT"                 /* User account */
#define FN_BRD                  ".BRD"                  /* board list */
#define FN_BRH                  ".BRH"                  /* board reading history */
#define FN_DIR                  ".DIR"                  /* index */
#define FN_GEM                  ".GEM"                  /* folder backup */
#define FN_SCHEMA               ".USR"                  /* userid schema */
//#define FN_PRH                  ".PRH"                  /* PostRecommendHistory */
#define FN_ALOHA                "aloha"                 /* 上站通知 */
#define FN_BMW                  "bmw"                   /* BBS Message Write */
#define FN_BANMSG               "banmsg"                /* 拒收訊息列表 */
#define FN_BENZ                 "benz"                  /* BBS login Notify */
#define FN_CHICKEN              "chieken"               /* 小雞紀錄檔 (N) */
#define FN_EMAIL                "email"                 /* email reply認證記錄 (N) */
#define FN_FAVORITE             "favorite"              /* 我的最愛看版 */
#define FN_FAVORITE_IMG         "favorite.img"          /* 我的最愛看版 影像檔 */
#define FN_MYFAVORITE           "MF/favorite"           /* 我的最愛看版 */
#define FN_PAL                  "friend"                /* 好友名單 */
#define FN_FRIEND_BENZ          "frienz"                /* lkchu.981201: benz for friend */
#define FN_JUSTIFY              "justify"               /* 註冊紀錄檔 */
#define FN_LOG                  "log"                   /* 個人上站紀錄 */
#define FN_LOGINS_BAD           "logins.bad"            /* 密碼錯誤紀錄 */
#define FN_NEWBOARD             "newboard"              /* 連署系統紀錄檔 (N) */
#define FN_NOTE                 "note"                  /* 寄信備份用 */
#define FN_PNOTE_ANS            "p_note.ans"            /* 答錄機中的留言 (N) */
#define FN_PNOTE_ANS_SAVE       "p_note.ans.save"       /* 答錄機中儲存的留言 */
#define FN_PNOTE_HINT           "p_note.hint"           /* 答錄機主人留言 */
#define FN_PLANS                "plans"                 /* 名片檔 (N) */
#define FN_PNOTE_DAT            "pnote.dat"             /* 答錄機資料庫新的留言  */
#define FN_PNOTE_TMP            "pnote.tmp"             /*   */
#define FN_PNOTE_DAT_SAVE       "pnote.dat.save"        /* 答錄機資料庫保留下來的留言 */
#define FN_SIGN                 "sign"                  /* 簽名檔 */
#define FN_STOPPERM_LOG         "stopperm.log"          /* 個人停權紀錄 */
#define FN_VOTE                 "vote"                  /* 投票紀錄資料庫 (N) */
#define FN_CLASSTABLE2          "classtable2"           /* 新版的功課表 */
#define FN_THRESHOLD            "threshold"             /* 看板發文門檻 */
#define FN_BANK                 "run/bank.log"          /* 銀行 log */
#define FN_SHOP                 "run/shop.log"          /* 商店 log */
#define FN_REGKEY               "regkey"                /* 認證碼 */
#define FN_FORWARD              "forward"               /* 自動轉寄設定 */
/* ----------------------------------------------------- */
/* etc/ 下的檔名設定                                     */
/* ----------------------------------------------------- */
#define FN_CHAT_PARTY2_DB       "etc/chat.party2.db"
#define FN_CHAT_PARTY_DB        "etc/chat.party.db"
#define FN_CHAT_SPEAK_DB        "etc/chat.speak.db"
#define FN_CHAT_CONDITION_DB    "etc/chat.condition.db"
#define FN_CHAT_PERSON_DB       "etc/chat.person.db"

#define FN_ETC_BIRTHDAY         "etc/birthday"          /* 生日卡 */
#define FN_ETC_MAILER_ACL       "etc/mailer.acl"        /* 發信軟體擋信列表 */
#define FN_ETC_ADMIN_DB         "etc/admin.db"          /* 超級站務列表資料庫 */
#define FN_ETC_ALLOW_ACL        "etc/allow.acl"         /* 白名單 */
#define FN_ETC_ANNOUNCE         "etc/announce"          /* 進站公告 */
#define FN_ETC_APPROVED         "etc/approved"          /* Email 通過身分認證 */
#define FN_ETC_BADID            "etc/badid"             /* 不雅代號 */
#define FN_ETC_BANMAIL_ACL      "etc/banmail.acl"       /* 全站檔信資料庫 */
#define FN_ETC_BANIP_ACL        "etc/banip.acl"         /* 禁止上站地點 */
#define FN_CHECKMAIL_MAIL       "etc/checkmail.mail"    /* 告知檢查完畢 */
#define FN_ETC_COUNTER          "etc/counter"           /* 歷史紀錄與成長 */
#define FN_ETC_EMAIL            "etc/e-mail"            /* 修改 Email, 進行身分認證 */
#define FN_ETC_EXPIRE_CONF      "etc/expire.conf"       /* 版面期限 */
#define FN_ETC_JUSTIFIED_POP3   "etc/justified.pop3"    /* POP3 通過身分認證 */
#define FN_ETC_JUSTIFIED_BMTA   "etc/justified.bmta"    /* BMTA 通過身分認證 */
#define FN_ETC_JUSTIFY          "etc/justify"           /* 身分認證的方法 */
#define FN_ETC_LOVEPAPER        "etc/lovepaper"         /* 情書產生器資料庫 */
#define FN_ETC_SPAMMER_ACL      "etc/spamer.acl"        /* 站外 SPAM 名單 */
#define FN_ETC_MAIL_OVER        "etc/mail.over"         /* 信件超出上限 */
#define FN_ETC_MAILSERVICE      "etc/mailservice"       /* 電子郵件服務 */
#define FN_MATCH_MAIL           "etc/match.mail"        /* 告知特殊搜尋更新完畢 */
#define FN_ETC_MQUOTA           "etc/mquota"            /* 超過保存期限 */
#define FN_ETC_NEWUSER          "etc/newuser"           /* 新手上路須知 */
#define FN_ETC_NOTIFY           "etc/notify"            /* 尚未通過身分認證 */
#define FN_ETC_REREG            "etc/re-reg"            /* 每半年重新認證 */
#define FN_ETC_SYSOP            "etc/sysop"             /* 站務列表 */
#define FN_ETC_VERSION          "etc/Version"           /* WindTOP版本 */
#define FN_ETC_EMAILADDR_ACL    "etc/same_email.acl"    /* 相同信箱 */
#define FN_STOPPERM_MAIL        "etc/stopperm.mail"     /* 告知停權完畢 */
#define FN_ETC_UNTRUST_ACL      "etc/untrust.acl"       /* 禁止註冊名單 */
#define FN_ETC_VALID            "etc/valid"             /* 認證信函 */
#define FN_VIOLATELAW_DB        "etc/violatelaw.acl"    /* 違法資料庫 */
#define FN_ETC_RFORM            "etc/rform"             /* 註冊單說明 */
#define FN_ETC_OBSERVE          "etc/observe.db"        /* 系統觀察名單 */
#define FN_ETC_PERSONAL         "etc/personal"          /* 個人板資料庫 */
#define FN_VCH                  ".VCH"                  /* vote control header */
#define FN_PAYCHECK             "paycheck"              /* 支票 */

/* ----------------------------------------------------- */
/* news 設定檔                                           */
/* ----------------------------------------------------- */
#define FN_NEWS                 "etc/news/"             /* 設定檔位置 */
#define FN_NEWS_CLASS           "etc/news/class.ini"    /* Class 名稱對應 */
#define FN_NEWS_NEWS            "etc/news/news.ini"     /* 站台設定 */
#define BRD_NEWS                "news/"                 /* 文章位置 */

/* ----------------------------------------------------- */
/* 各個版下的檔名設定                                    */
/* ----------------------------------------------------- */
#define CHECK_BAN               "check_ban"             /* 判斷此版是否擋信 */
#define BANMAIL_ACL             "banmail.acl"           /* 擋信設定擋 */
#define FN_BRD_STAT             "bstat"                 /* 看板資訊 */
#define FN_BRD_STATCOUNT        "bstatcount"            /* 看板資訊統計 */

/* ----------------------------------------------------- */
/* Help 設定檔                                           */
/* ----------------------------------------------------- */
#define FN_HELP_LIST            "gem/@/@list.hlp"       /* 群組名單 */
#define FN_NEWBOARD_HELP        "gem/@/@newboard.hlp"   /* 開版規則 */

/* ----------------------------------------------------- */
/* run/ 下的檔名設定                                     */
/* ----------------------------------------------------- */
#define FN_BROWSE_LOG           "run/browse.log"
#define FN_NEWSEXPIRE_LOG       "run/newsexpire.log"
#define FN_RESET_LOG            "run/reset.log"         /* 系統重置 */
#define FN_CHAT_LOG             "run/chat.log"
#define FN_CHATDATA_LOG         "run/chatdata.log"
#define FN_VAR_SYSHISTORY       "run/var/counter"       /* 歷史紀錄 */
#ifdef  HAVE_ANONYMOUS
/* Thor.980727:lkchu patch: anonymous post log */
#define FN_ANONYMOUS_LOG        "run/anonymous.log"     /* 匿名板紀錄 */
#endif  /* #ifdef  HAVE_ANONYMOUS */
#define FN_BANMAIL_LOG          "run/banmail.log"       /* 擋信紀錄 */
#define FN_BLACKSU_LOG          "run/blacksu.log"       /* 超級站務使用紀錄 */
#define FN_BSMTP_LOG            "run/bsmtp.log"         /* 所有 [寄信] 記錄 */
#ifdef LOG_BRD_USIES
#define FN_BRD_USIES            "run/brd_usies"         /* 版面閱讀紀錄 */
#endif  /* #ifdef LOG_BRD_USIES */
#define FN_CAMERA_LOG           "run/camera.log"        /* 動態看板更新紀錄 */
#define FN_CHECKBM_LOG          "run/checkbm.log"       /* 確認版主紀錄檔 */
#define FN_EXPIRE_LOG           "run/expire.log"        /* 自動砍信工具程式 */
#define FN_EXPIRED_LOG          "run/expired.log"       /* 手動砍信紀錄 */
#define FN_FAVORITE_LOG         "run/favorite.log"      /* 我的最愛紀錄 */
#define FN_MAILEXPIRE_LOG       "run/mailexpire.log"    /* 自動砍使用者刪除的信紀錄 */
#define FN_GCHECK_LOG           "run/gcheck.log"        /* 精華區整理程式 */
#define FN_GEMD_LOG             "run/gemd.log"          /* 所有 [地鼠] 記錄 */
#define FN_GINDEX_LOG           "run/gindex.log"        /* 精華區索引程式 */
#define FN_LAZYBM_LOG           "run/lazybm.log"        /* 偷懶版主紀錄 */
#define FN_MAIL_LOG             "run/mail.log"          /* 所有 [信件] 記錄 */
#define FN_BBSMAILPOST_LOG      "run/bbsmailpost.log"   /* bbsmail mailpost 紀錄 */
#define FN_MAILSERVICE_LOG      "run/mailservice.log"

#ifdef  HAVE_RECOMMEND
#define FN_RECOMMEND_LOG        "run/recommend.log"     /* 推薦文章 */
#endif
                                                        /* MailService 使用紀錄 */
#define FN_MANAGER_LOG          "run/manager.log"       /* 站務列表 */
#define FN_BMLIST_LOG           "run/bmlist.log"        /* 板主列表 */
#define FN_MATCH_LOG            "run/match.log"         /* 特殊搜尋 */
#define FN_MATCH_NEW            "run/match.new"         /* 特殊搜尋更新中 */
#define FN_SONG_LOG             "run/ordersongs.log"    /* 點歌紀錄 */
#define FN_PAL_LOG              "run/pal.log"           /* 好友名單過多紀錄 */
#define FN_PASS_LOG             "run/pass.log"          /* 密碼修改記錄 */
#define FN_PIP_LOG              "run/pip.log"           /* 寵物雞 */
#define FN_PIPMONEY_LOG         "run/pipmoney.log"      /* 寵物雞財務狀況 */
#define FN_MINE_LOG             "run/mine.log"          /* 踩地雷 */
#define FN_POP3_LOG             "run/pop3.log"          /* 所有 [POP3] 紀錄 */
#define FN_POST_LOG             "run/post.log"          /* 文章篇數統計 */
#define FN_POST_DB              "run/post.db"           /* 文章篇數統計資料庫 */
#define FN_POSTEDIT_LOG         "run/postedit.log"      /* 修改文章紀錄 */
#define FN_PYDICT_LOG           "run/pydict.log"        /* 英漢漢英字典 */D

#define FN_REAPER_LOG           "run/reaper.log"        /* 本日砍帳號紀錄 */
#define FN_LOGIN_LOG            "run/login.log"         /* 所有 [登錄] 記錄 */
#define FN_SPAM_LOG             "run/spam.log"          /* 本日 [SPAM] 紀錄 */
#define FN_STOP_LOG             "run/stop.log"          /* 停權紀錄 */
#define FN_SPAMPATH_LOG         "run/spampath.log"      /* junk 板 spam 分析 path */
#define FN_CLASSTABLE_DB        "run/class_alert.db"    /* 課表時刻通知 */
#define FN_SAMEEMAIL_LOG        "tmp/sameemail.log"
#define FN_EMAILADDR_LOG        "run/emailaddr.log"     /* 超過認證信箱上限 */
                                                        /* 註冊信箱使用次數統計 */

#ifdef LOG_TALK
#define FN_TALK_LOG             "talk.log"              /* lkchu.981201: talk 記錄檔 */
#endif  /* #ifdef LOG_TALK */
#ifdef  LOG_CHAT
#define FN_UCHAT_LOG            "chat.log"              /* 聊天室 log */
#endif  /* #ifdef  LOG_CHAT */
#define FN_USIES                "run/usies"             /* BBS log */
#define FN_USERNO_LOG           "run/userno.log"        /* 使用者編號紀錄 in crontab */
#define FN_LOGIN_LOG            "run/login.log"         /* 使用者上站記錄檔 */
#define FN_NOTE_ALL             "run/note.all"
#ifdef LOG_ADMIN
#define FN_SECURITY             "run/secure.log"        /* lkchu.981201: 系統安全記錄 */
#endif  /* #ifdef LOG_ADMIN */
#define FN_VERIFY_LOG           "run/verify.log"        /* 認證記錄 */
#define FN_CRIMINAL_LOG         "run/criminal.log"      /* 停權名單 */
#define FN_UEEQUERY_LOG         "run/ueequery.log"      /* 聯考查榜紀錄 */
#define FN_RFORM_R              "run/rform_r.db"        /* 註冊單申請資料庫 */

#define FN_BBSNET_LOG           "run/bbsnet.log"        /* BBSNET log */
#define FN_SEVEN_LOG            "run/seven.log"         /* 接龍 log */
#define FN_PERSONAL_LOG         "run/personal.log"      /* 個人板相關 log */
#define FN_YZUSERVICE_LOG       "run/yzuservice.log"    /* YzuService Log */

/* ----------------------------------------------------- */
/* 其他的檔名設定                                        */
/* ----------------------------------------------------- */
#define FN_ERROR_CAMERA         "gem/@/@error-camera"   /* 錯誤的動態看版 */
#define FN_HOTBOARD             "gem/@/@HotBoard"       /* 熱門看板 */

#define FN_GAME_BBSNET          "game/bbsnet/bbsdata.db"
                                                        /* BBSNET 站台 */

/* ----------------------------------------------------- */
/* 各個版的檔名設定                                      */
/* ----------------------------------------------------- */
#define BRD_ANNOUNCE            "0_Announce"
#define BRD_BANPOSTLOG          "BanPostLog"
#define BRD_VIOLATELAW          "ViolateLaw"
#define BRD_LOCALPOSTS          "LocalPosts"
#define BRD_TRASHCAN            "TrashCan"
#define BRD_SECRET              "Secret"
#define BRD_SYSTEM              "SYSTEM"
#define BRD_JUNK                "junk"
#define BRD_DELETED             "deleted"
#define BRD_BULLETIN            "Bulletin"
#define BRD_SBULLETIN           "SBulletin"
#define BRD_MODIFIED            "Modify"
#define BRD_SPECIAL             "Special"
#define BRD_CROSSPOST           "CrossPost"

#define BRD_REQUEST             "SongBook"
#define BRD_CAMERA              "note"
#define BRD_ORDERSONGS          "SongToCamera"

/* Thor.981223: 將 bbsreg untrust.acl 分離出來 */
#define NEWS_ACLFILE            "etc/news.acl"          /* news access control list */

#ifdef HAVE_SIGNED_MAIL
#define PRIVATE_KEY             "etc/prikey"            /* Thor.990409: key file name */
#endif

#ifdef  HAVE_REGISTER_FORM
#define FN_RFORM                "run/rform"             /* 註冊表單 */
#define FN_RFORM_LOG            "run/rform.log"         /* 註冊表單審核記錄檔 */
#endif

#ifdef  MODE_STAT
#define FN_MODE_LOG             "run/mode.log"          /* 使用者動態統計 - record per hour */
#define FN_MODE_CUR             "run/mode.cur"
#define FN_MODE_TMP             "run/mode.tmp"
#endif

#define MAIL_QUEUE      "run/.MQ"

#define DEFAULT_BOARD   str_sysop
#define OPT_OPERATOR    "Operator"

/* 鍵盤設定 */
#define KEY_TAB         9
#define KEY_ENTER       10
#define KEY_ESC         27
#define KEY_UP          -1
#define KEY_DOWN        -2
#define KEY_RIGHT       -3
#define KEY_LEFT        -4
#define KEY_HOME        -21
#define KEY_INS         -22
#define KEY_DEL         -23
#define KEY_END         -24
#define KEY_PGUP        -25
#define KEY_PGDN        -26

#define I_TIMEOUT       -31
#define I_OTHERDATA     -32

#define Ctrl(c)         ( c & 037 )
#define Meta(c)         ( c + 0x0200 )
#define isprint2(c)     ((c >= ' ')) /* ((c & 0x80 || isprint(c))) */

/* ----------------------------------------------------- */
/* 參數設定                                              */
/* ----------------------------------------------------- */

/* ----------------------------------------------------- */
/* Register Form Log                                     */
/* ----------------------------------------------------- */
#define RFORM_PASS             (0)
#define RFORM_NOPASS           (1)
#define RFORM_DEL              (2)
#define RFORM_CANCELREG        (3)

/* ----------------------------------------------------- */
/* Grayout Levels                                        */
/* ----------------------------------------------------- */
#define GRAYOUT_COLORBOLD      (-2)
#define GRAYOUT_BOLD           (-1)
#define GRAYOUT_DARK           (0)
#define GRAYOUT_NORM           (1)
#define GRAYOUT_COLORNORM      (+2)

/* ----------------------------------------------------- */
/* 訊息字串：獨立出來，以利支援各種語言                  */
/* ----------------------------------------------------- */

#define QUOTE_CHAR1     '>'
#define QUOTE_CHAR2     ':'


#define STR_CURSOR      "●"
#define STR_UNCUR       "  "
#define STR_SPACE       " \t\n\r"

#define STR_AUTHOR1     "作者:"
#define STR_AUTHOR2     "發信人:"
#define STR_POST1       "看板:"
#define STR_POST2       "站內:"
#define STR_REPLY       "Re: "
#define STR_FORWARD     "Fw: "

#define STR_LINE        "\n" \
"> -------------------------------------------------------------------------- <\n\n"
#define LEN_AUTHOR1     (sizeof(STR_AUTHOR1) - 1)
#define LEN_AUTHOR2     (sizeof(STR_AUTHOR2) - 1)

#define STR_SYSOP       "sysop"
#define SYSOPNAME       "sysop"
#define ELDER           "cache"
#define STR_GUEST       "guest"
#define STR_NEW         "new"

#define MSG_SEPARATOR   \
"───────────────────────────────────────"
#define MSG_BLINE       \
"______________________________________________________________________________"
#define MSG_ULINE       \
"￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣"
#define MSG_CANCEL      "取消"
#define MSG_USR_LEFT    "對方已經離去"

#define MSG_USERPERM    "權限等級："
#define MSG_READPERM    "閱\讀權限："
#define MSG_POSTPERM    "發表權限："
#define MSG_BRDATTR     "看板屬性："

#define MSG_DEL_OK      "刪除完畢"
#define MSG_DEL_CANCEL  "取消刪除"
#define MSG_DEL_ERROR   "刪除錯誤"
#define MSG_DEL_NY      "請確定刪除(Y/N)?[N] "

#define MSG_SURE_NY     "請您確定(Y/N)？[N] "
#define MSG_SURE_YN     "請您確定(Y/N)？[Y] "

#define MSG_BID         "請輸入看板名稱："
#define MSG_UID         "請輸入代號："
#define MSG_PASSWD      "請輸入密碼："

#define ERR_BID         "錯誤的看板名稱"
#define ERR_UID         "錯誤的使用者代號"
#define ERR_PASSWD      "密碼輸入錯誤"

#define MSG_POST        "\033[34;46m 文章選讀 \033[31;47m (y)\033[30m回應 \033[31m(=\\[]<>-+;'`jk)\033[30m相關主題 \033[31m(/?)\033[30m搜尋標題 \033[31m(aA)\033[30m搜尋作者 \033[m"

#define MSG_MAILER      "\033[34;46m 魚雁往返 \033[31;47m (r)\033[30m回信 \033[31m(x)\033[30m轉達 \033[31m(y)\033[30m群組回信 \033[31m(d)\033[30m刪除 \033[31m[m]\033[30m標記 \033[m"

#define MSG_GEM         "\033[34;46m 閱\讀精華 \033[31;47m (=\\[]<>-+jk)\033[30m相關主題 \033[31m(/?aA)\033[30m搜尋標題作者 \033[31m(↑↓)\033[30m上下 \033[31m(←)\033[30m離開 \033[m"

#define MSG_CHAT_ULIST  "\033[7m 使用者代號    目前狀態  │ 使用者代號    目前狀態  │ 使用者代號    目前狀態 \033[m"

/* ----------------------------------------------------- */
/* GLOBAL VARIABLE                                       */
/* ----------------------------------------------------- */
VAR pid_t currpid;              /* current process ID */
VAR unsigned int bbsmode;       /* bbs operating mode, see modes.h */
VAR int bbstate;                /* bbs operating state */
VAR int bbsothermode;
VAR int supervisor;
VAR UTMP *cutmp;
VAR int curredit;
VAR int checkqt;
VAR int showansi INI(1);
VAR time_t ap_start;
VAR ACCT cuser;                 /* current user structure */
VAR time_t currchrono;          /* current file timestamp @ bbs.c mail.c */

VAR int b_lines;                /* bottom line */
VAR int t_lines;
VAR int b_cols;                 /* bottom columns */
VAR int d_cols;                 /* difference columns from standard */

VAR char fromhost[48];

VAR char quote_file[80];
VAR char quote_user[80];
VAR char quote_nick[80];
VAR char currtitle[80];

VAR char hunt[32];              /* hunt keyword */

VAR char ve_title[80];
VAR char currboard[IDLEN + 2];          /* name of currently selected board */
VAR char currBM[BMLEN + 7];             /* BM of currently selected board */
VAR int  currbno        INI(-1);
VAR char str_ransi[4]   INI("\033[m");
VAR unsigned int currbattr;            /* currently selected board battr */
VAR char ipv4addr[15];                 /* MAX_LEN: strlen(aaa.bbb.ccc.ddd) => 15 */

VAR int  chk_mailstat   INI(0);

/* filename */
VAR char *fn_dir        INI(FN_DIR);

/* message */
VAR char *msg_separator INI(MSG_SEPARATOR);

VAR char *msg_cancel    INI(MSG_CANCEL);

VAR char *msg_sure_ny   INI(MSG_SURE_NY);

VAR char *msg_uid       INI(MSG_UID);

VAR char *msg_del_ny    INI(MSG_DEL_NY);

VAR char *err_bid       INI(ERR_BID);
VAR char *err_uid       INI(ERR_UID);

VAR char *str_sysop     INI("sysop");
VAR char *brd_sysop     INI("SYSOP");
VAR char *str_author1   INI(STR_AUTHOR1);
VAR char *str_author2   INI(STR_AUTHOR2);
VAR char *str_post1     INI(STR_POST1);
VAR char *str_post2     INI(STR_POST2);
VAR char *str_host      INI(MYHOSTNAME);
VAR char *str_site      INI(BOARDNAME);

#ifdef  HAVE_RECOMMEND
VAR int recommend_time  INI(0);
#endif

#if 0
VAR int aprilfirst      INI(0);
#endif

#undef  VAR
#undef  INI

extern char xo_pool[];          /* XO's data I/O pool */
int total_num;                  /* 重設 站上人數*/

#endif                          /* _GLOBAL_H_ */
