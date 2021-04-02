/*-------------------------------------------------------*/
/* global_def.h     ( NCKU CCNS WindTop-DreamBBS 3.0 )   */
/*-------------------------------------------------------*/
/* Author: Wei-Cheng Yeh (IID) <iid@ccns.ncku.edu.tw>    */
/* Target: Global definitions                            */
/* Create: 2020-11-16                                    */
/*-------------------------------------------------------*/
#ifndef GLOBAL_DEF_H
#define GLOBAL_DEF_H

#include "config.h"
#include "cppdef.h"

/* ----------------------------------------------------- */
/* GLOBAL DEFINITION                                     */
/* ----------------------------------------------------- */
#define FN_INNBBS_LOG           "innd/innbbs.log"       /* �Ҧ� [��H] �O�� */

/* ----------------------------------------------------- */
/* �ӤH�ؿ��ɦW�]�w                                      */
/* ----------------------------------------------------- */
#define FN_ACCT                 ".ACCT"                 /* User account */
#define FN_BRD                  ".BRD"                  /* board list */
#define FN_BRH                  ".BRH"                  /* board reading history */
#define FN_DIR                  ".DIR"                  /* index */
#define FN_GEM                  ".GEM"                  /* folder backup */
#define FN_SCHEMA               ".USR"                  /* userid schema */
//#define FN_PRH                  ".PRH"                  /* PostRecommendHistory */
#define FN_ALOHA                "aloha"                 /* �W���q�� */
#define FN_BMW                  "bmw"                   /* BBS Message Write */
#define FN_BANMSG               "banmsg"                /* �ڦ��T���C�� */
#define FN_BENZ                 "benz"                  /* BBS login Notify */
#define FN_CHICKEN              "chieken"               /* �p�������� (N) */
#define FN_EMAIL                "email"                 /* email reply�{�ҰO�� (N) */
#define FN_FAVORITE             "favorite"              /* �ڪ��̷R�ݪ� */
#define FN_MYFAVORITE           "MF/favorite"           /* �ڪ��̷R�ݪ� */
#define FN_PAL                  "friend"                /* �n�ͦW�� */
#define FN_FRIEND_BENZ          "frienz"                /* lkchu.981201: benz for friend */
#define FN_JUSTIFY              "justify"               /* ���U������ */
#define FN_LOG                  "log"                   /* �ӤH�W������ */
#define FN_LOGINS_BAD           "logins.bad"            /* �K�X���~���� */
#define FN_NEWBOARD             "newboard"              /* �s�p�t�ά����� (N) */
#define FN_NOTE                 "note"                  /* �H�H�ƥ��� */
#define FN_PNOTE_ANS            "p_note.ans"            /* �����������d�� (N) */
#define FN_PNOTE_ANS_SAVE       "p_note.ans.save"       /* ���������x�s���d�� */
#define FN_PNOTE_HINT           "p_note.hint"           /* �������D�H�d�� */
#define FN_PLANS                "plans"                 /* �W���� (N) */
#define FN_PNOTE_DAT            "pnote.dat"             /* ��������Ʈw�s���d��  */
#define FN_PNOTE_TMP            "pnote.tmp"             /*   */
#define FN_PNOTE_DAT_SAVE       "pnote.dat.save"        /* ��������Ʈw�O�d�U�Ӫ��d�� */
#define FN_SIGN                 "sign"                  /* ñ�W�� */
#define FN_STOPPERM_LOG         "stopperm.log"          /* �ӤH���v���� */
#define FN_VOTE                 "vote"                  /* �벼������Ʈw (N) */
#define FN_CLASSTABLE2          "classtable2"           /* �s�����\�Ҫ� */
#define FN_THRESHOLD            "threshold"             /* �ݪO�o����e */
#define FN_BANK                 "run/bank.log"          /* �Ȧ� log */
#define FN_SHOP                 "run/shop.log"          /* �ө� log */
#define FN_REGKEY               "regkey"                /* �{�ҽX */
#define FN_FORWARD              "forward"               /* �۰���H�]�w */
/* ----------------------------------------------------- */
/* etc/ �U���ɦW�]�w                                     */
/* ----------------------------------------------------- */
#define FN_CHAT_PARTY2_DB       "etc/chat.party2.db"
#define FN_CHAT_PARTY_DB        "etc/chat.party.db"
#define FN_CHAT_SPEAK_DB        "etc/chat.speak.db"
#define FN_CHAT_CONDITION_DB    "etc/chat.condition.db"
#define FN_CHAT_PERSON_DB       "etc/chat.person.db"

#define FN_ETC_BIRTHDAY         "etc/birthday"          /* �ͤ�d */
#define FN_ETC_MAILER_ACL       "etc/mailer.acl"        /* �o�H�n��׫H�C�� */
#define FN_ETC_ADMIN_DB         "etc/admin.db"          /* �W�ů��ȦC���Ʈw */
#define FN_ETC_ALLOW_ACL        "etc/allow.acl"         /* �զW�� */
#define FN_ETC_ANNOUNCE         "etc/announce"          /* �i�����i */
#define FN_ETC_APPROVED         "etc/approved"          /* Email �q�L�����{�� */
#define FN_ETC_BADID            "etc/badid"             /* �����N�� */
#define FN_ETC_BANMAIL_ACL      "etc/banmail.acl"       /* �����ɫH��Ʈw */
#define FN_ETC_BANIP_ACL        "etc/banip.acl"         /* �T��W���a�I */
#define FN_CHECKMAIL_MAIL       "etc/checkmail.mail"    /* �i���ˬd���� */
#define FN_ETC_COUNTER          "etc/counter"           /* ���v�����P���� */
#define FN_ETC_EMAIL            "etc/e-mail"            /* �ק� Email, �i�樭���{�� */
#define FN_ETC_EXPIRE_CONF      "etc/expire.conf"       /* �������� */
#define FN_ETC_JUSTIFIED_POP3   "etc/justified.pop3"    /* POP3 �q�L�����{�� */
#define FN_ETC_JUSTIFIED_BMTA   "etc/justified.bmta"    /* BMTA �q�L�����{�� */
#define FN_ETC_JUSTIFY          "etc/justify"           /* �����{�Ҫ���k */
#define FN_ETC_SPAMMER_ACL      "etc/spamer.acl"        /* ���~ SPAM �W�� */
#define FN_ETC_MAIL_OVER        "etc/mail.over"         /* �H��W�X�W�� */
#define FN_MATCH_MAIL           "etc/match.mail"        /* �i���S��j�M��s���� */
#define FN_ETC_MQUOTA           "etc/mquota"            /* �W�L�O�s���� */
#define FN_ETC_NEWUSER          "etc/newuser"           /* �s��W������ */
#define FN_ETC_NOTIFY           "etc/notify"            /* �|���q�L�����{�� */
#define FN_ETC_REREG            "etc/re-reg"            /* �C�b�~���s�{�� */
#define FN_ETC_SYSOP            "etc/sysop"             /* ���ȦC�� */
#define FN_ETC_VERSION          "etc/Version"           /* WindTOP���� */
#define FN_ETC_EMAILADDR_ACL    "etc/same_email.acl"    /* �ۦP�H�c */
#define FN_STOPPERM_MAIL        "etc/stopperm.mail"     /* �i�����v���� */
#define FN_ETC_UNTRUST_ACL      "etc/untrust.acl"       /* �T����U�W�� */
#define FN_ETC_VALID            "etc/valid"             /* �{�ҫH�� */
#define FN_VIOLATELAW_DB        "etc/violatelaw.acl"    /* �H�k��Ʈw */
#define FN_ETC_RFORM            "etc/rform"             /* ���U�满�� */
#define FN_ETC_OBSERVE          "etc/observe.db"        /* �t���[��W�� */
#define FN_ETC_PERSONAL         "etc/personal"          /* �ӤH�O��Ʈw */
#define FN_VCH                  ".VCH"                  /* vote control header */

/* ----------------------------------------------------- */
/* �U�Ӫ��U���ɦW�]�w                                    */
/* ----------------------------------------------------- */
#define CHECK_BAN               "check_ban"             /* �P�_�����O�_�׫H */
#define BANMAIL_ACL             "banmail.acl"           /* �׫H�]�w�� */
#define FN_BRD_STAT             "bstat"                 /* �ݪO��T */
#define FN_BRD_STATCOUNT        "bstatcount"            /* �ݪO��T�έp */

/* ----------------------------------------------------- */
/* Help �]�w��                                           */
/* ----------------------------------------------------- */
#define FN_HELP_LIST            "gem/@/@list.hlp"       /* �s�զW�� */
#define FN_NEWBOARD_HELP        "gem/@/@newboard.hlp"   /* �}���W�h */

/* ----------------------------------------------------- */
/* run/ �U���ɦW�]�w                                     */
/* ----------------------------------------------------- */
#define FN_BROWSE_LOG           "run/browse.log"
#define FN_RESET_LOG            "run/reset.log"         /* �t�έ��m */
#define FN_CHAT_LOG             "run/chat.log"
#define FN_CHATDATA_LOG         "run/chatdata.log"
#define FN_VAR_SYSHISTORY       "run/var/counter"       /* ���v���� */
#ifdef  HAVE_ANONYMOUS
/* Thor.980727:lkchu patch: anonymous post log */
#define FN_ANONYMOUS_LOG        "run/anonymous.log"     /* �ΦW�O���� */
#endif  /* #ifdef  HAVE_ANONYMOUS */
#define FN_BANMAIL_LOG          "run/banmail.log"       /* �׫H���� */
#define FN_BLACKSU_LOG          "run/blacksu.log"       /* �W�ů��Ȩϥά��� */
#define FN_BSMTP_LOG            "run/bsmtp.log"         /* �Ҧ� [�H�H] �O�� */
#ifdef LOG_BRD_USIES
#define FN_BRD_USIES            "run/brd_usies"         /* �����\Ū���� */
#endif  /* #ifdef LOG_BRD_USIES */
#define FN_CAMERA_LOG           "run/camera.log"        /* �ʺA�ݪO��s���� */
#define FN_CHECKBM_LOG          "run/checkbm.log"       /* �T�{���D������ */
#define FN_EXPIRE_LOG           "run/expire.log"        /* �۰ʬ�H�u��{�� */
#define FN_EXPIRED_LOG          "run/expired.log"       /* ��ʬ�H���� */
#define FN_FAVORITE_LOG         "run/favorite.log"      /* �ڪ��̷R���� */
#define FN_MAILEXPIRE_LOG       "run/mailexpire.log"    /* �۰ʬ�ϥΪ̧R�����H���� */
#define FN_GCHECK_LOG           "run/gcheck.log"        /* ��ذϾ�z�{�� */
#define FN_GEMD_LOG             "run/gemd.log"          /* �Ҧ� [�a��] �O�� */
#define FN_GINDEX_LOG           "run/gindex.log"        /* ��ذϯ��޵{�� */
#define FN_LAZYBM_LOG           "run/lazybm.log"        /* ���i���D���� */
#define FN_MAIL_LOG             "run/mail.log"          /* �Ҧ� [�H��] �O�� */
#define FN_BBSMAILPOST_LOG      "run/bbsmailpost.log"   /* bbsmail mailpost ���� */

#ifdef  HAVE_RECOMMEND
#define FN_RECOMMEND_LOG        "run/recommend.log"     /* ���ˤ峹 */
#endif
                                                        /* MailService �ϥά��� */
#define FN_MANAGER_LOG          "run/manager.log"       /* ���ȦC�� */
#define FN_BMLIST_LOG           "run/bmlist.log"        /* �O�D�C�� */
#define FN_MATCH_LOG            "run/match.log"         /* �S��j�M */
#define FN_MATCH_NEW            "run/match.new"         /* �S��j�M��s�� */
#define FN_SONG_LOG             "run/ordersongs.log"    /* �I�q���� */
#define FN_PAL_LOG              "run/pal.log"           /* �n�ͦW��L�h���� */
#define FN_PASS_LOG             "run/pass.log"          /* �K�X�ק�O�� */
#define FN_PIP_LOG              "run/pip.log"           /* �d���� */
#define FN_PIPMONEY_LOG         "run/pipmoney.log"      /* �d�����]�Ȫ��p */
#define FN_MINE_LOG             "run/mine.log"          /* ��a�p */
#define FN_POP3_LOG             "run/pop3.log"          /* �Ҧ� [POP3] ���� */
#define FN_POST_LOG             "run/post.log"          /* �峹�g�Ʋέp */
#define FN_POST_DB              "run/post.db"           /* �峹�g�Ʋέp��Ʈw */
#define FN_POSTEDIT_LOG         "run/postedit.log"      /* �ק�峹���� */

#define FN_REAPER_LOG           "run/reaper.log"        /* �����b������ */
#define FN_LOGIN_LOG            "run/login.log"         /* �Ҧ� [�n��] �O�� */
#define FN_SPAM_LOG             "run/spam.log"          /* ���� [SPAM] ���� */
#define FN_STOP_LOG             "run/stop.log"          /* ���v���� */
#define FN_SPAMPATH_LOG         "run/spampath.log"      /* junk �O spam ���R path */
#define FN_CLASSTABLE_DB        "run/class_alert.db"    /* �Ҫ�ɨ�q�� */
#define FN_SAMEEMAIL_LOG        "tmp/sameemail.log"
#define FN_EMAILADDR_LOG        "run/emailaddr.log"     /* �W�L�{�ҫH�c�W�� */
                                                        /* ���U�H�c�ϥΦ��Ʋέp */

#ifdef LOG_TALK
#define FN_TALK_LOG             "talk.log"              /* lkchu.981201: talk �O���� */
#endif  /* #ifdef LOG_TALK */
#ifdef  LOG_CHAT
#define FN_UCHAT_LOG            "chat.log"              /* ��ѫ� log */
#endif  /* #ifdef  LOG_CHAT */
#define FN_USIES                "run/usies"             /* BBS log */
#define FN_USERNO_LOG           "run/userno.log"        /* �ϥΪ̽s������ in crontab */
#define FN_LOGIN_LOG            "run/login.log"         /* �ϥΪ̤W���O���� */
#define FN_NOTE_ALL             "run/note.all"
#ifdef LOG_ADMIN
#define FN_SECURITY             "run/secure.log"        /* lkchu.981201: �t�Φw���O�� */
#endif  /* #ifdef LOG_ADMIN */
#define FN_VERIFY_LOG           "run/verify.log"        /* �{�ҰO�� */
#define FN_CRIMINAL_LOG         "run/criminal.log"      /* ���v�W�� */
#define FN_RFORM_R              "run/rform_r.db"        /* ���U��ӽи�Ʈw */

#define FN_PERSONAL_LOG         "run/personal.log"      /* �ӤH�O���� log */

/* ----------------------------------------------------- */
/* ��L���ɦW�]�w                                        */
/* ----------------------------------------------------- */
#define FN_ERROR_CAMERA         "gem/@/@error-camera"   /* ���~���ʺA�ݪ� */
#define FN_HOTBOARD             "gem/@/@HotBoard"       /* �����ݪO */
#define FN_APRIL_FIRST          "gem/brd/Admin/J/A106LL7J"
                                                        /* IID.20190822: The April first article (unused) */

/* ----------------------------------------------------- */
/* �U�Ӫ����ɦW�]�w                                      */
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

/* Thor.981223: �N bbsreg untrust.acl �����X�� */
#ifdef HAVE_SIGNED_MAIL
#define PRIVATE_KEY             "etc/prikey"            /* Thor.990409: key file name */
#endif

#ifdef  HAVE_REGISTER_FORM
#define FN_RFORM                "run/rform"             /* ���U��� */
#define FN_RFORM_LOG            "run/rform.log"         /* ���U���f�ְO���� */
#endif

#ifdef  MODE_STAT
#define FN_MODE_LOG             "run/mode.log"          /* �ϥΪ̰ʺA�έp - record per hour */
#define FN_MODE_CUR             "run/mode.cur"
#define FN_MODE_TMP             "run/mode.tmp"
#endif

#define MAIL_QUEUE      "run/.MQ"

#define DEFAULT_BOARD   str_sysop
#define OPT_OPERATOR    "Operator"

/* ��L�]�w */

/* Normal keys */
#define KEY_TAB         '\t'
#define KEY_ENTER       '\n'
#define KEY_ESC         '\x1b'

/* Control sequence keys */
#define KEY_UP          0x0141  /* A */
#define KEY_DOWN        0x0142  /* B */
#define KEY_RIGHT       0x0143  /* C */
#define KEY_LEFT        0x0144  /* D */
#define KEY_BTAB        0x015A  /* Z; Shift-Tab */
#define KEY_STAB        KEY_BTAB  /* Key alias for compatibility */

/* Control sequence keys with final byte being `~` */
/* Should be consecutive for compatibility */
#define KEY_HOME        0x0241
#define KEY_INS         0x0242
#define KEY_DEL         0x0243
#define KEY_END         0x0244
#define KEY_PGUP        0x0245
#define KEY_PGDN        0x0246

/* Should be consecutive for compatibility */
#define KEY_F1          0x0251
#define KEY_F2          0x0252
#define KEY_F3          0x0253
#define KEY_F4          0x0254
#define KEY_F5          0x0255
#define KEY_F6          0x0256
#define KEY_F7          0x0257
#define KEY_F8          0x0258
#define KEY_F9          0x0259
#define KEY_F10         0x025A
#define KEY_F11         0x025B
#define KEY_F12         0x025C

/* Special key values */
#define KEY_INVALID     0x03FE

#define I_TIMEOUT       0x04FD
#define I_OTHERDATA     0x04FE

#define I_RESIZETERM    Meta(Ctrl('L'))

#define KEY_NONE        0x4000  /* All key values should `< KEY_NONE` */

#define Ctrl(c)         ( c & ~0x0060 )
#define Meta(c)         ( c | 0x2000 )  /* All unmodified key values should `< 0x2000` */
#define Shift(c)        ( c ^ 0x0020 )  /* Only works on 'A'-']', 'a'-'}', and special keys */
/* If needed, apply `Shift()` after applying `Ctrl()`, e.g., `Shift(Ctrl(c))`. Do not do the reverse */
/* For normal keys, do not apply `Shift()` on `Ctrl()`ed keys. */
#define isprint2(c)     ((c >= ' ') && (c <= 0xff)) /* ((c & 0x80 || isprint(c))) */

#define IS_DBCS_HI(x)   ((x) >= 0x80U)

/* ----------------------------------------------------- */
/* �ѼƳ]�w                                              */
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
#ifdef M3_USE_PFTERM
#define GRAYOUT_COLORBOLD      (-2)
#define GRAYOUT_BOLD           (-1)
#define GRAYOUT_DARK           (0)
#define GRAYOUT_NORM           (1)
#define GRAYOUT_COLORNORM      (+2)
#else  // #ifdef M3_USE_PFTERM
#define GRAYOUT_COLORBOLD      (-1)
#define GRAYOUT_BOLD           (1)
#define GRAYOUT_DARK           (0)
#define GRAYOUT_NORM           (2)
#define GRAYOUT_COLORNORM      (-2)
#endif  // #ifdef M3_USE_PFTERM

/* ----------------------------------------------------- */
/* �T���r��G�W�ߥX�ӡA�H�Q�䴩�U�ػy��                  */
/* ----------------------------------------------------- */

#define QUOTE_CHAR1     '>'
#define QUOTE_CHAR2     ':'


//#define STR_CURSOR      "��"
//#define STR_UNCUR       "  "  // Needs to be as wide as `STR_CURSOR`
#define STR_CURSOR      ">"
#define STR_UNCUR       " "
#define STR_SPACE       " \t\n\r"

#define STR_AUTHOR1     "�@��:"
#define STR_AUTHOR2     "�o�H�H:"
#define STR_POST1       "�ݪO:"
#define STR_POST2       "����:"
#define STR_REPLY       "Re:"
#define STR_FORWARD     "Fw:"

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
"�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w"
#define MSG_BLINE       \
"______________________________________________________________________________"
#define MSG_ULINE       \
"�áááááááááááááááááááááááááááááááááááááá�"
#define MSG_CANCEL      "����"
#define MSG_USR_LEFT    "���w�g���h"

#define MSG_USERPERM    "�v�����šG"
#define MSG_READPERM    "�\\Ū�v���G"
#define MSG_POSTPERM    "�o���v���G"
#define MSG_BRDATTR     "�ݪO�ݩʡG"

#define MSG_DEL_OK      "�R������"
#define MSG_DEL_CANCEL  "�����R��"
#define MSG_DEL_ERROR   "�R�����~"
#define MSG_DEL_NY      "�нT�w�R��(y/N)?[N] "

#define MSG_SURE_NY     "�бz�T�w(y/N)�H[N] "
#define MSG_SURE_YN     "�бz�T�w(Y/n)�H[Y] "

#define MSG_BID         "�п�J�ݪO�W�١G"
#define MSG_UID         "�п�J�N���G"
#define MSG_PASSWD      "�п�J�K�X�G"

#define ERR_BID         "���~���ݪO�W��"
#define ERR_UID         "���~���ϥΪ̥N��"
#define ERR_PASSWD      "�K�X��J���~"

#define MSG_CHAT_ULIST  "\x1b[7m �ϥΪ̥N��    �ثe���A  �x �ϥΪ̥N��    �ثe���A  �x �ϥΪ̥N��    �ثe���A \x1b[m"

#endif                          /* GLOBAL_DEF_H */
