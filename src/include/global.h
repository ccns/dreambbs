/*-------------------------------------------------------*/
/* global.h	( NTHU CS MapleBBS Ver 2.36 )		 */
/*-------------------------------------------------------*/
/* target : global definitions & variables		 */
/* create : 95/03/29				 	 */
/* update : 95/12/15				 	 */
/*-------------------------------------------------------*/
#ifndef	_GLOBAL_H_
#define	_GLOBAL_H_

#ifdef	_MAIN_C_
# define VAR
# define INI(x)		= x
#else
# define VAR		extern
# define INI(x)
#endif

/* ----------------------------------------------------- */
/* GLOBAL DEFINITION					 */
/* ----------------------------------------------------- */
#define	FN_INNBBS_LOG		"innd/innbbs.log"	/* �Ҧ� [��H] �O�� */

/* ----------------------------------------------------- */
/* �ӤH�ؿ��ɦW�]�w                                      */
/* ----------------------------------------------------- */
#define	FN_ACCT			".ACCT"			/* User account */
#define FN_BRD			".BRD"			/* board list */
#define FN_BRH			".BRH"			/* board reading history */
#define FN_DIR			".DIR"			/* index */
#define FN_GEM			".GEM"			/* folder backup */
#define FN_SCHEMA		".USR"			/* userid schema */
//#define FN_PRH			".PRH"			/* PostRecommendHistory */
#define	FN_ALOHA		"aloha"			/* �W���q�� */
#define FN_BMW			"bmw"			/* BBS Message Write */
#define	FN_BANMSG		"banmsg"		/* �ڦ��T���C�� */
#define FN_BENZ			"benz"			/* BBS login Notify */
#define	FN_CHICKEN		"chieken"		/* �p�������� (N) */
#define	FN_EMAIL		"email"			/* email reply�{�ҰO�� (N) */
#define FN_FAVORITE     	"favorite"      	/* �ڪ��̷R�ݪ� */
#define	FN_FAVORITE_IMG		"favorite.img"		/* �ڪ��̷R�ݪ� �v���� */
#define FN_MYFAVORITE     	"MF/favorite"      	/* �ڪ��̷R�ݪ� */
#define	FN_PAL			"friend"		/* �n�ͦW�� */
#define FN_FRIEND_BENZ  	"frienz"        	/* lkchu.981201: benz for friend */
#define	FN_JUSTIFY		"justify"		/* ���U������ */
#define	FN_LOG			"log"			/* �ӤH�W������ */
#define	FN_LOGINS_BAD		"logins.bad"		/* �K�X���~���� */
#define	FN_NEWBOARD		"newboard"		/* �s�p�t�ά����� (N) */
#define	FN_NOTE			"note"			/* �H�H�ƥ��� */
#define	FN_PNOTE_ANS		"p_note.ans"		/* �����������d�� (N) */
#define	FN_PNOTE_ANS_SAVE	"p_note.ans.save"	/* ���������x�s���d�� */
#define	FN_PNOTE_HINT		"p_note.hint"		/* �������D�H�d�� */
#define	FN_PLANS		"plans"			/* �W���� (N) */
#define	FN_PNOTE_DAT		"pnote.dat"		/* ��������Ʈw�s���y��  */
#define	FN_PNOTE_TMP		"pnote.tmp"		/*   */
#define	FN_PNOTE_DAT_SAVE	"pnote.dat.save"	/* ��������Ʈw�O�d�U�Ӫ��y�� */
#define	FN_SIGN			"sign"			/* ñ�W�� */
#define	FN_STOPPERM_LOG		"stopperm.log"		/* �ӤH���v���� */
#define	FN_VOTE			"vote"			/* �벼������Ʈw (N) */
#define FN_CLASSTABLE2		"classtable2"		/* �s�����\�Ҫ� */
#define FN_THRESHOLD       "threshold"    /* �ݪO�o����e */
#define FN_BANK		"run/bank.log"		/* �Ȧ� log */
#define FN_SHOP		"run/shop.log"		/* �ө� log */
#define	FN_REGKEY		"regkey"		/* �{�ҽX */
#define FN_FORWARD    "forward"       /* �۰���H�]�w */
/* ----------------------------------------------------- */
/* etc/ �U���ɦW�]�w                                     */
/* ----------------------------------------------------- */
#define	FN_CHAT_PARTY2_DB	"etc/chat.party2.db"
#define FN_CHAT_PARTY_DB	"etc/chat.party.db"
#define FN_CHAT_SPEAK_DB	"etc/chat.speak.db"
#define FN_CHAT_CONDITION_DB	"etc/chat.condition.db"
#define	FN_CHAT_PERSON_DB	"etc/chat.person.db"

#define	FN_ETC_BIRTHDAY		"etc/birthday"		/* �ͤ�d */
#define	FN_ETC_MAILER_ACL	"etc/mailer.acl"	/* �o�H�n��׫H�C�� */
#define FN_ETC_ADMIN_DB		"etc/admin.db"		/* �W�ů��ȦC���Ʈw */
#define	FN_ETC_ALLOW_ACL	"etc/allow.acl"		/* �զW�� */
#define	FN_ETC_ANNOUNCE		"etc/announce"		/* �i�����i */
#define	FN_ETC_APPROVED		"etc/approved"		/* Email �q�L�����{�� */
#define	FN_ETC_BADID		"etc/badid"		/* �����N�� */
#define	FN_ETC_BANMAIL_ACL	"etc/banmail.acl"	/* �����ɫH��Ʈw */
#define	FN_ETC_BANIP_ACL	"etc/banip.acl"		/* �T��W���a�I */
#define FN_CHECKMAIL_MAIL	"etc/checkmail.mail"	/* �i���ˬd���� */
#define	FN_ETC_COUNTER		"etc/counter"		/* ���v�����P���� */
#define	FN_ETC_EMAIL		"etc/e-mail"		/* �ק� Email, �i�樭���{�� */
#define	FN_ETC_EXPIRE_CONF	"etc/expire.conf"	/* �������� */
#define	FN_ETC_JUSTIFIED_POP3	"etc/justified.pop3"	/* POP3 �q�L�����{�� */
#define	FN_ETC_JUSTIFIED_BMTA	"etc/justified.bmta"	/* BMTA �q�L�����{�� */
#define	FN_ETC_JUSTIFY		"etc/justify"		/* �����{�Ҫ���k */
#define	FN_ETC_LOVEPAPER	"etc/lovepaper"		/* ���Ѳ��;���Ʈw */
#define	FN_ETC_SPAMER_ACL	"etc/spamer.acl"	/* ���~ SPAM �W�� */
#define	FN_ETC_MAIL_OVER	"etc/mail.over"		/* �H��W�X�W�� */
#define	FN_ETC_MAILSERVICE	"etc/mailservice"	/* �q�l�l��A�� */
#define	FN_MATCH_MAIL		"etc/match.mail"	/* �i���S��j�M��s���� */
#define	FN_ETC_MQUOTA		"etc/mquota"		/* �W�L�O�s���� */
#define	FN_ETC_NEWUSER		"etc/newuser"		/* �s��W������ */
#define	FN_ETC_NOTIFY		"etc/notify"		/* �|���q�L�����{�� */
#define	FN_ETC_REREG		"etc/re-reg"		/* �C�b�~���s�{�� */
#define	FN_ETC_SYSOP		"etc/sysop"		/* ���ȦC�� */
#define	FN_ETC_VERSION		"etc/Version"		/* WindTOP���� */
#define	FN_ETC_EMAILADDR_ACL	"etc/same_email.acl"	/* �ۦP�H�c */
#define FN_STOPPERM_MAIL	"etc/stopperm.mail"	/* �i�����v���� */
#define	FN_ETC_UNTRUST_ACL	"etc/untrust.acl"	/* �T����U�W�� */
#define	FN_ETC_VALID		"etc/valid"		/* �{�ҫH�� */
#define FN_VIOLATELAW_DB	"etc/violatelaw.acl"	/* �H�k��Ʈw */
#define	FN_ETC_RFORM		"etc/rform"		/* ���U�满�� */
#define	FN_ETC_OBSERVE		"etc/observe.db"	/* �t���[��W�� */
#define	FN_ETC_PERSONAL		"etc/personal"		/* �ӤH�O��Ʈw */
#define FN_VCH         		".VCH"          /* vote control header */
#define FN_PAYCHECK    		"paycheck"      /* �䲼 */

/* ----------------------------------------------------- */
/* news �]�w��	                                     */
/* ----------------------------------------------------- */
#define	FN_NEWS				"etc/news/"				/* �]�w�ɦ�m */
#define	FN_NEWS_CLASS		"etc/news/class.ini"	/* Class �W�ٹ��� */
#define	FN_NEWS_NEWS		"etc/news/news.ini"		/* ���x�]�w */
#define	BRD_NEWS			"news/"					/* �峹��m */

/* ----------------------------------------------------- */
/* �U�Ӫ��U���ɦW�]�w                                    */
/* ----------------------------------------------------- */
#define CHECK_BAN		"check_ban"		/* �P�_�����O�_�׫H */
#define	BANMAIL_ACL		"banmail.acl"		/* �׫H�]�w�� */
#define	FN_BRD_STAT		"bstat"			/* �ݪO��T */
#define	FN_BRD_STATCOUNT	"bstatcount"		/* �ݪO��T�έp */

/* ----------------------------------------------------- */
/* Help �]�w��						 */
/* ----------------------------------------------------- */
#define	FN_HELP_LIST		"gem/@/@list.hlp"	/* �s�զW�� */
#define FN_NEWBOARD_HELP	"gem/@/@newboard.hlp"	/* �}���W�h */

/* ----------------------------------------------------- */
/* run/ �U���ɦW�]�w                                     */
/* ----------------------------------------------------- */
#define FN_BROWSE_LOG   "run/browse.log"
#define	FN_NEWSEXPIRE_LOG	"run/newsexpire.log"
#define	FN_RESET_LOG		"run/reset.log"		/* �t�έ��m */
#define	FN_CHAT_LOG		"run/chat.log"
#define	FN_CHATDATA_LOG		"run/chatdata.log"
#define FN_VAR_SYSHISTORY	"run/var/counter"	/* ���v���� */
#ifdef  HAVE_ANONYMOUS
/* Thor.980727:lkchu patch: anonymouse post log */
#define FN_ANONYMOUS_LOG	"run/anonymous.log"	/* �ΦW�O���� */
#endif
#define FN_BANMAIL_LOG	"run/banmail.log"	/* �׫H���� */
#define FN_BGUARD_LOG	"run/bguard.log"	/* �Ҧ� [load] �O�� */
#define FN_BLACKSU_LOG	"run/blacksu.log"	/* �W�ů��Ȩϥά��� */
#define FN_BSMTP_LOG	"run/bsmtp.log"		/* �Ҧ� [�H�H] �O�� */
#ifdef LOG_BRD_USIES
#define	FN_BRD_USIES	"run/brd_usies"	/* �����\Ū���� */
#endif
#define	FN_CAMERA_LOG	"run/camera.log"	/* �ʺA�ݪO��s���� */
#define	FN_CHECKBM_LOG	"run/checkbm.log"	/* �T�{���D������ */
#define FN_EXPIRE_LOG	"run/expire.log"	/* �۰ʬ�H�u��{�� */
#define	FN_EXPIRED_LOG	"run/expired.log"	/* ��ʬ�H���� */
#define	FN_FAVORITE_LOG	"run/favorite.log"	/* �ڪ��̷R���� */
#define FN_MAILEXPIRE_LOG	"run/mailexpire.log"	/* �۰ʬ�ϥΪ̧R�����H���� */
#define	FN_GCHECK_LOG	"run/gcheck.log"	/* ��ذϾ�z�{�� */
#define FN_GEMD_LOG	"run/gemd.log"		/* �Ҧ� [�a��] �O�� */
#define	FN_GINDEX_LOG	"run/gindex.log"	/* ��ذϯ��޵{�� */
#define	FN_LAZYBM_LOG	"run/lazybm.log"	/* ���i���D���� */
#define FN_MAIL_LOG	"run/mail.log"		/* �Ҧ� [�H��] �O�� */
#define FN_BBSMAILPOST_LOG	"run/bbsmailpost.log"
                                                /* bbsmail mailpost ���� */
#define	FN_MAILSERVICE_LOG	"run/mailservice.log"	

#ifdef  HAVE_RECOMMEND
#define	FN_RECOMMEND_LOG	"run/recommend.log"	/* ���ˤ峹 */
#endif
						/* MailSevice �ϥά��� */
#define	FN_MANAGER_LOG	"run/manager.log"	/* ���ȦC�� */
#define	FN_BMLIST_LOG	"run/bmlist.log"	/* �O�D�C�� */
#define	FN_MATCH_LOG	"run/match.log"		/* �S��j�M */
#define	FN_MATCH_NEW	"run/match.new"		/* �S��j�M��s�� */
#define	FN_SONG_LOG	"run/ordersongs.log"	/* �I�q���� */
#define	FN_PAL_LOG	"run/pal.log"		/* �n�ͦW��L�h���� */
#define	FN_PASS_LOG	"run/pass.log"		/* �K�X�ק�O�� */
#define FN_PIP_LOG	"run/pip.log"		/* �d���� */
#define FN_PIPMONEY_LOG	"run/pipmoney.log"	/* �d�����]�Ȫ��p */
#define FN_MINE_LOG	"run/mine.log"		/* ��a�p */
#define FN_POP3_LOG	"run/pop3.log"		/* �Ҧ� [POP3] ���� */
#define	FN_POP3MAIL_LOG	"run/pop3mail.log"	/* �����~�� POP3 �l��A�� */
#define	FN_POST_LOG	"run/post.log"		/* �峹�g�Ʋέp */
#define	FN_POST_DB	"run/post.db"		/* �峹�g�Ʋέp��Ʈw */
#define	FN_POSTEDIT_LOG	"run/postedit.log"	/* �ק�峹���� */
#define FN_PYDICT_LOG	"run/pydict.log"	/* �^�~�~�^�r�� */
#define	FN_REAPER_LOG	"run/reaper.log"	/* �����b������ */
#define	FN_LOGIN_LOG	"run/login.log"		/* �Ҧ� [�n��] �O�� */
#define	FN_SPAM_LOG	"run/spam.log"		/* ���� [SPAM] ���� */
#define FN_STOP_LOG	"run/stop.log"		/* ���v���� */
#define	FN_SPAMPATH_LOG	"run/spampath.log"	/* junk �O spam ���R path */
#define	FN_CLASSTABLE_DB	"run/class_alert.db"	/* �Ҫ�ɨ�q�� */
#define	FN_SAMEEMAIL_LOG	"tmp/sameemail.log"
#define	FN_EMAILADDR_LOG	"run/emailaddr.log"	/* �W�L�{�ҫH�c�W�� */	
                                                /* ���U�H�c�ϥΦ��Ʋέp */

#ifdef LOG_TALK
#define FN_TALK_LOG	"talk.log"		/* lkchu.981201: talk �O���� */
#endif
#ifdef	LOG_CHAT
#define	FN_UCHAT_LOG	"chat.log"		/* ��ѫ� log */
#endif
#define	FN_USIES	"run/usies"		/* BBS log */
#define	FN_USERNO_LOG	"run/userno.log"	/* �ϥΪ̽s������ in crontab */
#define FN_LOGIN_LOG	"run/login.log"		/* �ϥΪ̤W���O���� */
#define FN_NOTE_ALL	"run/note.all"
#ifdef LOG_ADMIN
#define FN_SECURITY     "run/secure.log" 	/* lkchu.981201: �t�Φw���O�� */
#endif
#define	FN_VERIFY_LOG	"run/verify.log" 	/* �{�ҰO�� */
#define	FN_CRIMINAL_LOG	"run/criminal.log"	/* ���v�W�� */	
#define	FN_UEEQUERY_LOG	"run/ueequery.log"	/* �p�Ҭd�]���� */
#define	FN_RFORM_R	"run/rform_r.db"	/* ���U��ӽи�Ʈw */

#define	FN_BBSNET_LOG	"run/bbsnet.log"	/* BBSNET log */
#define FN_SEVEN_LOG	"run/seven.log"		/* ���s log */
#define FN_PERSONAL_LOG	"run/personal.log"	/* �ӤH�O���� log */
#define	FN_YZUSERVICE_LOG	"run/yzuservice.log"	/* YzuService Log */

/* ----------------------------------------------------- */
/* ��L���ɦW�]�w                                     	 */
/* ----------------------------------------------------- */ 
#define	FN_ERROR_CAMERA	"gem/@/@error-camera"	/* ���~���ʺA�ݪ� */
#define	FN_HOTBOARD		"gem/@/@HotBoard"		/* �����ݪO */

#define	FN_GAME_BBSNET	"game/bbsnet/bbsdata.db"	/* BBSNET ���x */

/* ----------------------------------------------------- */
/* �U�Ӫ����ɦW�]�w                                      */
/* ----------------------------------------------------- */
#define	BRD_ANNOUNCE	"0_Announce"
#define	BRD_BANPOSTLOG	"BanPostLog"
#define BRD_VIOLATELAW	"ViolateLaw"
#define BRD_LOCALPOSTS	"LocalPosts"
#define BRD_TRASHCAN	"TrashCan"
#define	BRD_SECRET	    "Secret"
#define	BRD_SYSTEM	    "SYSTEM"
#define	BRD_JUNK	    "junk"
#define	BRD_DELETED	    "deleted"
#define	BRD_BULLETIN	"Bulletin"
#define	BRD_SBULLETIN	"SBulletin"
#define	BRD_MODIFIED	"Modify"
#define	BRD_SPECIAL	    "Special"
#define	BRD_CROSSPOST	"CrossPost"

#define BRD_REQUEST	    "SongBook"
#define	BRD_CAMERA	    "note"
#define BRD_ORDERSONGS	"SongToCamera"

/* Thor.981223: �N bbsreg untrust.acl �����X�� */
#define NEWS_ACLFILE    "etc/news.acl"	/* news access control list */

#ifdef HAVE_SIGNED_MAIL
#define PRIVATE_KEY	"etc/prikey"	/* Thor.990409: key file name */
#endif

#ifdef	HAVE_REGISTER_FORM
#define FN_RFORM	"run/rform"	/* ���U��� */
#define FN_RFORM_LOG	"run/rform.log"	/* ���U���f�ְO���� */
#endif

#ifdef	MODE_STAT
#define FN_MODE_LOG     "run/mode.log"  /* �ϥΪ̰ʺA�έp - record per hour */
#define FN_MODE_CUR     "run/mode.cur"
#define FN_MODE_TMP     "run/mode.tmp"
#endif

#define	MAIL_QUEUE	"run/.MQ"

#define	DEFAULT_BOARD	str_sysop
#define	OPT_OPERATOR	"Operator"

/* ��L�]�w */
#define KEY_TAB		9
#define KEY_ENTER	10
#define KEY_ESC		27
#define KEY_UP		-1
#define KEY_DOWN	-2
#define KEY_RIGHT	-3
#define KEY_LEFT	-4
#define KEY_HOME	-21
#define KEY_INS		-22
#define KEY_DEL		-23
#define KEY_END		-24
#define KEY_PGUP	-25
#define KEY_PGDN	-26

#define I_TIMEOUT	-31
#define I_OTHERDATA	-32

#define Ctrl(c)		( c & 037 )
#define Meta(c)		( c + 0x0200 )
#define	isprint2(c)	((c >= ' ')) /* ((c & 0x80 || isprint(c))) */

/* ----------------------------------------------------- */
/* �ѼƳ]�w                                              */
/* ----------------------------------------------------- */
                                                                                
#define GRAYOUT_DARK    0
#define GRAYOUT_BOLD    1
#define GRAYOUT_NORMAL  2

/* ----------------------------------------------------- */
/* �T���r��G�W�ߥX�ӡA�H�Q�䴩�U�ػy��			 */
/* ----------------------------------------------------- */

#define QUOTE_CHAR1	'>'
#define QUOTE_CHAR2	':'


#define STR_CURSOR	"��"
#define STR_UNCUR	"  "
#define	STR_SPACE	" \t\n\r"

#define	STR_AUTHOR1	"�@��:"
#define	STR_AUTHOR2	"�o�H�H:"
#define	STR_POST1	"�ݪO:"
#define	STR_POST2	"����:"
#define	STR_REPLY	"Re: "
#define	STR_FORWARD	"Fw: "

#define STR_LINE	"\n\
> -------------------------------------------------------------------------- <\n\n"
#define	LEN_AUTHOR1	(sizeof(STR_AUTHOR1) - 1)
#define	LEN_AUTHOR2	(sizeof(STR_AUTHOR2) - 1)

#define STR_SYSOP	"sysop"
#define SYSOPNAME	"sysop"
#define ELDER	    "cache"
#define STR_GUEST	"guest"
#define STR_NEW		"new"

#define	MSG_SEPERATOR	"\
�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w"
#define MSG_BLINE	"\
______________________________________________________________________________"
#define MSG_ULINE	"\
�áááááááááááááááááááááááááááááááááááááá�"
#define	MSG_CANCEL	"����"
#define	MSG_USR_LEFT	"���w�g���h"

#define	MSG_USERPERM	"�v�����šG"
#define	MSG_READPERM	"�\\Ū�v���G"
#define	MSG_POSTPERM	"�o���v���G"
#define	MSG_BRDATTR	"�ݪO�ݩʡG"

#define	MSG_DEL_OK	"�R������"
#define	MSG_DEL_CANCEL	"�����R��"
#define	MSG_DEL_ERROR	"�R�����~"
#define	MSG_DEL_NY	"�нT�w�R��(Y/N)?[N] "

#define	MSG_SURE_NY	"�бz�T�w(Y/N)�H[N] "
#define	MSG_SURE_YN	"�бz�T�w(Y/N)�H[Y] "

#define	MSG_BID		"�п�J�ݪO�W�١G"
#define	MSG_UID		"�п�J�N���G"
#define	MSG_PASSWD	"�п�J�K�X�G"

#define	ERR_BID		"���~���ݪO�W��"
#define	ERR_UID		"���~���ϥΪ̥N��"
#define	ERR_PASSWD	"�K�X��J���~"

#define	MSG_POST	"\033[34;46m �峹��Ū \033[31;47m (y)\033[30m�^�� \033[31m(=\\[]<>-+;'`jk)\033[30m�����D�D \033[31m(/?)\033[30m�j�M���D \033[31m(aA)\033[30m�j�M�@�� \033[m"

#define	MSG_MAILER	"\033[34;46m �������� \033[31;47m (r)\033[30m�^�H \033[31m(x)\033[30m��F \033[31m(y)\033[30m�s�զ^�H \033[31m(d)\033[30m�R�� \033[31m[m]\033[30m�аO \033[m"

#define	MSG_GEM	"\033[34;46m �\\Ū��� \033[31;47m (=\\[]<>-+jk)\033[30m�����D�D \033[31m(/?aA)\033[30m�j�M���D�@�� \033[31m(����)\033[30m�W�U \033[31m(��)\033[30m���} \033[m"

#define	MSG_CHAT_ULIST	"\033[7m �ϥΪ̥N��    �ثe���A  �x �ϥΪ̥N��    �ثe���A  �x �ϥΪ̥N��    �ثe���A \033[m"

/* ----------------------------------------------------- */
/* GLOBAL VARIABLE					 */
/* ----------------------------------------------------- */
VAR pid_t currpid;		/* current process ID */
VAR usint bbsmode;		/* bbs operating mode, see modes.h */
VAR int bbstate;		/* bbs operatine state */
VAR int bbsothermode;
VAR int supervisor;
VAR UTMP *cutmp;
VAR int curredit;
VAR int checkqt;
VAR int showansi INI(1);
VAR time_t ap_start;
VAR ACCT cuser;			/* current user structure */
VAR time_t currchrono;		/* current file timestamp @ bbs.c mail.c */

VAR char fromhost[48];

VAR char quote_file[80];
VAR char quote_user[80];
VAR char quote_nick[80];
VAR char currtitle[80];

VAR char hunt[32];		/* hunt keyword */

VAR char ve_title[80];
VAR char currboard[IDLEN + 2];	/* name of currently selected board */
VAR char currBM[BMLEN + 7];	/* BM of currently selected board */
VAR int  currbno	INI(-1);
VAR char str_ransi[4] 	INI("\033[m");
VAR usint currbattr;            /* currently selected board battr */
VAR char ipv4addr[15]; /* MAX_LEN: strlen(aaa.bbb.ccc.ddd) => 15 */

VAR int  chk_mailstat	INI(0);

/* filename */
VAR char *fn_dir	INI(FN_DIR);

/* message */
VAR char *msg_seperator	INI(MSG_SEPERATOR);

VAR char *msg_cancel	INI(MSG_CANCEL);

VAR char *msg_sure_ny	INI(MSG_SURE_NY);

VAR char *msg_uid	INI(MSG_UID);

VAR char *msg_del_ny	INI(MSG_DEL_NY);

VAR char *err_bid	INI(ERR_BID);
VAR char *err_uid	INI(ERR_UID);

VAR char *str_sysop	INI("sysop");
VAR char *brd_sysop	INI("SYSOP");
VAR char *str_author1	INI(STR_AUTHOR1);
VAR char *str_author2	INI(STR_AUTHOR2);
VAR char *str_post1	INI(STR_POST1);
VAR char *str_post2	INI(STR_POST2);
VAR char *str_host	INI(MYHOSTNAME);
VAR char *str_site	INI(BOARDNAME);

#ifdef	HAVE_RECOMMEND
VAR int recommand_time	INI(0);
#endif

#if 0
VAR int aprilfirst	INI(0);
#endif

#undef	VAR
#undef	INI

extern char xo_pool[];		/* XO's data I/O pool */
int total_num;			/* ���] ���W�H��*/

#endif				/* _GLOBAL_H_ */
