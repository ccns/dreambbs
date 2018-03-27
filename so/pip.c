/* ----------------------------------- */
/* pip.c  ¾i¤pÂûµ{¦¡                   */
/* ­ì§@ªÌ: dsyan   §ï¼gªÌ: fennet      */
/* ¹Ï¹Ï by tiball.bbs@bbs.nhctc.edu.tw */
/* ----------------------------------- */
#define ba 5
#define START_MONEY	(3000)
#define START_FOOD	(20)
#define START_HP	(50)
#define	START_HAPPY	(20)
#define START_SATISFY	(20)

#define LEARN_ELVEL	((d.happy+d.satisfy)/100)

#include "bbs.h"
#include "pipstruct.h"
#include <time.h>
#include "pip.h"

#define	PIPNAME		"Ãdª«Âû"

struct chicken d;
time_t start_time;
time_t lasttime;

#define getdata(x1,x2,x3,x4,x5,x6,x7)  vget(x1,x2,x3,x4,x5,DOECHO)

int KEY_ESC_arg;
char *BoardName = currboard;
UTMP *currutmp;
char Bdate[128];
char pippath[128];
levelup ml[2];

#include "pipfun.c"

int pip_money();
static void pip_load_mob(char *fpath);
static void pip_load_mobset(char *fpath);
static void pip_check_levelup();
static void pip_check_level();
static void pip_load_levelup(char *fpath);
static int twice();
static int pip_magic_menu();
static int pip_magic_doing_menu();
void pip_read_file(char *userid);
void pip_write_file();
void show_system_pic(int i);
void pip_new_game();
int pip_main_menu();
int pip_live_again();
void show_basic_pic(int i);
int pip_log_record(char *msg);
void show_die_pic(int i);
int pip_mainmenu(int mode);
int pip_time_change(time_t cnow);
int pip_ending_screen();
int pip_marriage_offer();
void show_usual_pic(int i);
void show_feed_pic(int i);
int pip_buy_goods_new(int mode, struct goodsofpip *p, int oldnum[]);
int pip_weapon_doing_menu(int variance, int type, struct weapon *p);
int pip_vs_man(int n, struct playrule *p, int mode);
int pip_results_show_ending(int winorlost, int mode, int a, int b, int c);
void pip_fight_bad(int n);
void tie();
void win();
void situ();
void lose();
int pip_practice_function(int classnum, int classgrade, int pic1, int pic2, int *change1, int *change2, int *change3, int *change4, int *change5);
int pip_ending_decide(char *eendbuf1, char *eendbuf2, char *eendbuf3, int *endmode, int *endgrade);
int pip_game_over(int endgrade);
int pip_practice_gradeup(int classnum, int classgrade, int data);

/*¨t²Î¿ï³æ*/
int pip_data_list(), pip_system_freepip(), pip_system_service();
int pip_write_backup(), pip_read_backup();
int pip_divine(), pip_results_show();

#ifdef	HAVE_PIP_FIGHT
static int pip_magic_fight_menu();
static int get_hurt();
static int pip_fight_feed();

#endif
/*int pip_request();*/

static void
logit(money)
int money;
{
	char buf[100];
	time_t now;
	now = time(NULL);
	if (money < 100000)
		return;
	if (money > 300000)
		sprintf(buf, "\033[1;31m%s %s : %d\033[m\n", Cdate(&now), cuser.userid, money);
	else
		sprintf(buf, "%s %s : %d\n", Cdate(&now), cuser.userid, money);
	f_cat(FN_PIPMONEY_LOG, buf);
}

char *
get_path(id, file)
char *id, *file;
{
	usr_fpath(pippath, id, file);
	return pippath;
}

/*¹CÀ¸¥Dµ{¦¡*/
int p_pipple()
{
	FILE *fs;
	int pipkey;
	char genbuf[200];

	utmp_mode(M_CHICKEN);
	more("game/pipgame/pip.welcome", NULL);
	vs_head("¹q¤l¾i¤pÂû", BoardName);
	srandom(time(0));
	/* sprintf(genbuf,"home/%s/chicken",cuser.userid);*/
	usr_fpath(genbuf, cuser.userid, "chicken");
	pip_load_levelup("game/pipdata/piplevel.dat");
	pip_load_mob("game/pipdata/pipmob.dat");
	pip_load_mobset("game/pipdata/pipmobset.dat");
	pip_read_file(cuser.userid);
	if ((fs = fopen(genbuf, "r")) == NULL)
	{
		show_system_pic(11);
		move(b_lines, 0);
		pipkey = vkey();
		if (pipkey == 'Q' || pipkey == 'q')
			return 0;
		if (d.death != 0 || !d.name[0])
		{
			pip_new_game();
		}
	}
	else
	{
		show_system_pic(12);
		move(b_lines, 0);
		pipkey = vkey();
		if (pipkey == 'R' || pipkey == 'r')
			pip_read_backup();
		else if (pipkey == 'Q' || pipkey == 'q')
		{
			fclose(fs);
			return 0;
		}
		if (d.death != 0 || !d.name[0])
		{
			pip_new_game();
		}
		fclose(fs);
	}

	lasttime = time(0);
	start_time = time(0);
	/*pip_do_menu(0,0,pipmainlist);*/
	if (d.death != 0 || !d.name[0])  return 0;
	pip_main_menu();
	d.bbtime += time(0) - start_time;
	pip_write_file();
	logit(d.money);
	return 0;
}

/*®É¶¡ªí¥Üªk*/
char*
dsyan_time(const time_t *t)
{
	struct tm *tp;
	static char ans[9];

	tp = localtime(t);
	sprintf(ans, "%02d/%02d/%02d", (tp->tm_year) % 100, tp->tm_mon + 1, tp->tm_mday);
	return ans;
}

/*·s¹CÀ¸ªº³]©w*/
void pip_new_game()
{
	char buf[256];
	time_t now;
	char *pipsex[3] = {"¡H", "¡ñ", "¡ð"};
	struct tm *ptime;
	ptime = localtime(&now);

	if (d.death == 1 && !(!d.name[0]))
	{
		clear();
		vs_head(NICKNAME PIPNAME, BoardName);
		move(4, 6);
		prints("Åwªï¨Ó¨ì [1;5;33m" NICKNAME "¥Íª«¬ì§Þ¬ã¨s°|[0m");
		move(6, 6);
		prints("¸g§Ú­Ì½Õ¬dÅã¥Ü  ¥ý«e§A¦³¾i¹L¤pÂû³á  ¥i¬O³Q§A¾i¦º¤F...");
		move(8, 6);
		if (d.liveagain < 4)
		{
			prints("§Ú­Ì¥i¥HÀ°§AÀ°¤pÂû´_¬¡  ¦ý¬O»Ý­n¥I¥X¤@ÂI¥N»ù");
			getdata(10, 6, "§A­n§Ú­ÌÅý¥L­«¥Í¶Ü? [y/N]: ", buf, 2, DOECHO, 0);
			if (buf[0] == 'y' || buf[0] == 'Y')
			{
				pip_live_again();
			}
		}
		else if (d.liveagain >= 4)
		{
			prints("¥i¬O§A´_¬¡¤â³N¤Ó¦h¦¸¤F  ¤pÂû¨­¤W³£¬O¶}¤M²ª¸ñ");
			move(10, 6);
			prints("§Ú­Ì§ä¤£¨ì¥i¥H¤â³Nªº¦a¤è¤F  ©Ò¥H....");
			vmsg("­«·s¦A¨Ó§a....­ü....");
		}
	}
	if (d.death != 0 || !d.name[0])
	{
		clear();
		vs_head(NICKNAME PIPNAME, BoardName);
		/*¤pÂû©R¦W*/
		getdata(2, 0, "À°¤pÂû¨ú­Ó¦nÅ¥ªº¦W¦r§a(½Ð¤£­n¦³ªÅ®æ): ", buf, 11, DOECHO, 0);
		if (!buf[0]) 
			return;
		strcpy(d.name, buf);
		/*1:¤½ 2:¥À */
		getdata(4, 3, "[Boy]¤p¤½Âû¡ñ or [Girl]¤p¥ÀÂû¡ð [b/G]: ", buf, 2, DOECHO, 0);
		if (buf[0] == 'b' || buf[0] == 'B')
		{
			d.sex = 1;
		}
		else
		{
			d.sex = 2;
		}
		move(6, 3);
		prints(NICKNAME PIPNAME "ªº¹CÀ¸²{¤µ¤À¦¨¨âºØª±ªk");
		move(7, 3);
		prints("¿ï¦³µ²§½·|¦b¤pÂû20·³®Éµ²§ô¹CÀ¸¡A¨Ã§iª¾¤pÂû«áÄòªºµo®i");
		move(8, 3);
		prints("¿ï¨S¦³µ²§½«h¤@ª½¾i¨ì¤pÂû¦º¤`¤~µ²§ô¹CÀ¸....");
		/*1:¤£­n¥B¥¼±B 4:­n¥B¥¼±B */
		getdata(9, 3, "§A§Æ±æ¤pÂû¹CÀ¸¬O§_­n¦³20·³µ²§½? [Y/n]: ", buf, 2, DOECHO, 0);
		if (buf[0] == 'n' || buf[0] == 'N')
		{
			d.wantend = 1;
		}
		else
		{
			d.wantend = 4;
		}
		/*¶}ÀYµe­±*/
		show_basic_pic(0);
		vmsg("¤pÂû²×©ó½Ï¥Í¤F¡A½Ð¦n¦n·R¥L....");

		/*¶}ÀY³]©w*/
		now = time(0);
		strcpy(d.birth, dsyan_time(&now));
		d.bbtime = 0;

		/*°ò¥»¸ê®Æ*/
		d.year = ptime->tm_year;
		d.month = ptime->tm_mon + 1;
		d.day = ptime->tm_mday;
		d.death = d.nodone = d.relation = 0;
		d.liveagain = d.level = d.exp = d.dataE = 0;
		d.chickenmode = 1;

		/*¨­Åé°Ñ¼Æ*/
		d.hp = rand() % 15 + START_HP;
		d.maxhp = rand() % 20 + START_HP;
		if (d.hp > d.maxhp) d.hp = d.maxhp;
		d.weight = rand() % 10 + 50;
		d.tired = d.sick = d.shit = d.wrist = 0;
		d.bodyA = d.bodyB = d.bodyC = d.bodyD = d.bodyE = 0;

		/*µû»ù°Ñ¼Æ*/
		d.social = d.family = d.hexp = d.mexp = 0;
		d.tmpA = d.tmpB = d.tmpC = d.tmpD = d.tmpE = 0;

		/*¾Ô°«°Ñ¼Æ*/
		d.mp = d.maxmp = d.attack = d.resist = d.speed = d.hskill = d.mskill = d.mresist = 0;
		d.magicmode = d.specialmagic = d.fightC = d.fightD = d.fightE = 0;

		/*ªZ¾¹°Ñ¼Æ*/
		d.weaponhead = d.weaponrhand = d.weaponlhand = d.weaponbody = d.weaponfoot = 0;
		d.weaponA = d.weaponB = d.weaponC = d.weaponD = d.weaponE = 0;

		/*¯à¤O°Ñ¼Æ*/
		d.toman = d.character = d.love = d.wisdom = d.art = d.etchics = 0;
		d.brave = d.homework = d.charm = d.manners = d.speech = d.cookskill = 0;
		d.learnA = d.learnB = d.learnC = d.learnD = d.learnE = 0;

		/*ª¬ºA¼Æ­È*/
		d.happy = rand() % 10 + START_HAPPY;
		d.satisfy = rand() % 10 + START_SATISFY;
		d.fallinlove = d.belief = d.offense = d.affect = 0;
		d.stateA = d.stateB = d.stateC = d.stateD = d.stateE = 0;

		/*­¹ª«°Ñ¼Æ:­¹ª« ¹s­¹ ÃÄ«~ ¤j¸É¤Y*/
		d.food = START_FOOD;
		d.medicine = d.cookie = d.bighp = 2;
		d.ginseng = d.snowgrass = d.eatC = d.eatD = d.eatE = 0;

		/*ª««~°Ñ¼Æ:®Ñ ª±¨ã*/
		d.book = d.playtool = 0;
		d.money = START_MONEY;
		d.thingA = d.thingB = d.thingC = d.thingD = d.thingE = 0;

		/*²q®±°Ñ¼Æ:Ä¹ ­t*/
		d.winn = d.losee = 0;

		/*°Ñ¨£¤ý¦Ú*/
		d.royalA = d.royalB = d.royalC = d.royalD = d.royalE = 0;
		d.royalF = d.royalG = d.royalH = d.royalI = d.royalJ = 0;
		d.seeroyalJ = 1;
		d.seeA = d.seeB = d.seeC = d.seeD = d.seeE;
		/*±µ¨ü¨D±B·R¤H*/
		d.lover = 0;
		/*0:¨S¦³ 1:Å]¤ý 2:Às±Ú 3:A 4:B 5:C 6:D 7:E */
		d.classA = d.classB = d.classC = d.classD = d.classE = 0;
		d.classF = d.classG = d.classH = d.classI = d.classJ = 0;
		d.classK = d.classL = d.classM = d.classN = d.classO = 0;

		d.workA = d.workB = d.workC = d.workD = d.workE = 0;
		d.workF = d.workG = d.workH = d.workI = d.workJ = 0;
		d.workK = d.workL = d.workM = d.workN = d.workO = 0;
		d.workP = d.workQ = d.workR = d.workS = d.workT = 0;
		d.workU = d.workV = d.workW = d.workX = d.workY = d.workZ = 0;
		/*¾iÂû°O¿ý*/
		now = time(0);
		sprintf(buf, "[1;36m%s %-11s¾i¤F¤@°¦¥s [%s] ªº %s ¤pÂû [0m\n", Cdate(&now), cuser.userid, d.name, pipsex[d.sex]);
		pip_log_record(buf);
	}
	pip_write_file();
}

/*¤pÂû¦º¤`¨ç¦¡*/
void
pipdie(msg, mode)
char *msg;
int mode;
{
	char genbuf[200];
	time_t now;
	clear();
	vs_head("¹q¤l¾i¤pÂû", BoardName);
	if (mode == 1)
	{
		show_die_pic(1);
		vmsg("¦º¯«¨Ó±a¨«¤pÂû¤F");
		clear();
		vs_head("¹q¤l¾i¤pÂû", BoardName);
		show_die_pic(2);
		move(14, 20);
		prints("¥i¼¦ªº¤pÂû[1;31m%s[m", msg);
		vmsg(NICKNAME "«s±¥¤¤....");
	}
	else if (mode == 2)
	{
		show_die_pic(3);
		vmsg("¶ã¶ã¶ã..§Ú³Q¥á±ó¤F.....");
	}
	else if (mode == 3)
	{
		show_die_pic(0);
		vmsg("¹CÀ¸µ²§ôÅo..");
	}

	now = time(0);
	sprintf(genbuf, "[1;31m%s %-11sªº¤pÂû [%s] %s[m\n", Cdate(&now), cuser.userid, d.name, msg);
	pip_log_record(genbuf);
	pip_write_file();
}


/*pro:¾÷²v base:©³¼Æ mode:Ãþ«¬ mul:¥[Åv100=1 cal:¥[´î*/
void
count_tired(prob, base, mode, mul, cal)
int prob, base;
char *mode;
int mul;
int cal;
{
	int tiredvary = 0;
	int tm;
	/*time_t now;*/
	tm = (time(0) - start_time + d.bbtime) / 60 / 30;
	if (!strcmp(mode, "Y"))
	{
		if (tm >= 0 && tm <= 3)
		{
			if (cal == 1)
				tiredvary = (rand() % prob + base) * d.maxhp / (d.hp + 0.8 * d.hp) * 120 / 100;
			else if (cal == 0)
				tiredvary = (rand() % prob + base) * 4 / 3;
		}
		else if (tm >= 4 && tm <= 7)
		{
			if (cal == 1)
				tiredvary = (rand() % prob + base) * d.maxhp / (d.hp + 0.8 * d.hp);
			else if (cal == 0)
				tiredvary = (rand() % prob + base) * 3 / 2;
		}
		else if (tm >= 8 && tm <= 10)
		{
			if (cal == 1)
				tiredvary = (rand() % prob + base) * d.maxhp / (d.hp + 0.8 * d.hp) * 110 / 100;
			else if (cal == 0)
				tiredvary = (rand() % prob + base) * 5 / 4;
		}
		else if (tm >= 11)
		{
			if (cal == 1)
				tiredvary = (rand() % prob + base) * d.maxhp / (d.hp + 0.8 * d.hp) * 150 / 100;
			else if (cal == 0)
				tiredvary = (rand() % prob + base) * 1;
		}
	}
	else if (!strcmp(mode, "N"))
	{
		tiredvary = rand() % prob + base;
	}

	if (cal == 1)
	{
		d.tired += (tiredvary * mul / 100);
		if (d.tired > 100)
			d.tired = 100;
	}
	else if (cal == 0)
	{
		d.tired = d.tired - tiredvary;
		if (d.tired <= 0)
			{d.tired = 0;}
	}
	tiredvary = 0;
	return;
}

/*---------------------------------------------------------------------------*/
/*¥Dµe­±©M¿ï³æ                                                               */
/*---------------------------------------------------------------------------*/

char *menuname[8][2] =
{
	{"             ",
		"[1;44;37m ¿ï³æ [46m[1]°ò¥» [2]³}µó [3]­×¦æ [4]ª±¼Ö [5]¥´¤u [6]¯S®í [7]¨t²Î [Q]Â÷¶}          [m"},

	{"             ",
	 "[1;44;37m  °ò¥»¿ï³æ  [46m[1]Áý­¹ [2]²M¼ä [3]¥ð®§ [4]¿Ë¿Ë [5]´«¿ú         [Q]¸õ¥X¡G          [m"},

	{"[1;44;37m ³}µó [46m¡i¤é±`¥Î«~¡j[1]«K§Q°Ó©± [2]" NICKNAME "ÃÄ¾Q [3]©]¸Ì®Ñ§½                        [m",
	 "[1;44;37m ¿ï³æ [46m¡iªZ¾¹¦Ê³f¡j[A]ÀY³¡¸Ë³Æ [B]¥k¤â¸Ë³Æ [C]¥ª¤â¸Ë³Æ [D]¨­Åé¸Ë³Æ [E]¸}³¡¸Ë³Æ  [m"},

	{"[1;44;37m ­×¦æ [46m[A]¬ì¾Ç(%d) [B]¸Öµü(%d) [C]¯«¾Ç(%d) [D]­x¾Ç(%d) [E]¼C³N(%d)                   [m",
	 "[1;44;37m ¿ï³æ [46m[F]®æ°«(%d) [G]Å]ªk(%d) [H]Â§»ö(%d) [I]Ã¸µe(%d) [J]»RÁÐ(%d) [Q]¸õ¥X¡G         [m"},

	{"   ",
	 "[1;44;37m  ª±¼Ö¿ï³æ  [46m[1]´²¨B [2]¹B°Ê [3]¬ù·| [4]²q®± [5]®È¹C [6]­¥¥~ [7]°Ûºq [Q]¸õ¥X¡G  [m"},

	{"[1;44;37m ¥´¤u [46m[A]®a¨Æ [B]«O©i [C]®ÈÀ] [D]¹A³õ [E]À\\ÆU [F]±Ð°ó [G]¦aÅu [H]¥ï¤ì          [m",
	 "[1;44;37m ¿ï³æ [46m[I]¬ü¾v [J]Ây¤H [K]¤u¦a [L]¦u¹Ó [M]®a±Ð [N]°s®a [O]°s©± [P]©]Á`·| [Q]¸õ¥X[m"},

	{"[1;44;37m ¯S®í [46m[1]" NICKNAME "Âå°| [2]´Aµn®p [3]¾Ô°«­×¦æ [4]«ô³XªB¤Í [5]" NICKNAME "                [m",
	 "[1;44;37m ¿ï³æ [46m                                                                  [Q]¸õ¥X[m"},

	{"   ",
	 "[1;44;37m  ¨t²Î¿ï³æ  [46m[1]¸Ô²Ó¸ê®Æ [2]¤pÂû¦Û¥Ñ [3]¯S§OªA°È [4]Àx¦s¶i«× [5]Åª¨ú¶i«× [Q]¸õ¥X[m"}
};

/*¥D¿ï³æ*/
int pip_basic_menu(), pip_store_menu(), pip_practice_menu();
int pip_play_menu(), pip_job_menu(), pip_special_menu(), pip_system_menu();

static struct pipcommands pipmainlist[] =
{
	{pip_basic_menu,	'1',	'1'},
	{pip_store_menu,	'2',	'2'},
	{pip_practice_menu,	'3',	'3'},
	{pip_play_menu,		'4',	'4'},
	{pip_job_menu,		'5',	'5'},
	{pip_special_menu,	'6',	'6'},
	{pip_system_menu,	'7',	'7'},
	{NULL,			'\0',	'\0'}
};

/*°ò¥»¿ï³æ*/
int pip_basic_feed(), pip_basic_takeshower(), pip_basic_takerest(), pip_basic_kiss();
static struct pipcommands pipbasiclist[] =
{
	{pip_basic_feed,		'1',	'1'},
	{pip_basic_takeshower,	'2',	'2'},
	{pip_basic_takerest,	'3',	'3'},
	{pip_basic_kiss,		'4',	'4'},
	{pip_money,		'5',	'5'},
	/*{pip_request,		'6',	'6'},*/
	{NULL,			'\0',	'\0'}
};

/*°Ó©±¿ï³æ*/
int pip_store_food(), pip_store_medicine(), pip_store_other();
int pip_store_weapon_head(), pip_store_weapon_rhand(), pip_store_weapon_lhand();
int pip_store_weapon_body(), pip_store_weapon_foot();

static struct pipcommands pipstorelist[] =
{
	{pip_store_food,		'1',	'1'},
	{pip_store_medicine,	'2',	'2'},
	{pip_store_other,	'3',	'3'},
	{pip_store_weapon_head,	'a',	'A'},
	{pip_store_weapon_rhand,	'b',	'B'},
	{pip_store_weapon_lhand,	'c',	'C'},
	{pip_store_weapon_body,	'd',	'D'},
	{pip_store_weapon_foot,	'e',	'E'},
	{NULL,			'\0',	'\0'}
};

/*­×¦æ¿ï³æ*/
int pip_practice_classA(), pip_practice_classB(), pip_practice_classC();
int pip_practice_classD(), pip_practice_classE(), pip_practice_classF();
int pip_practice_classG(), pip_practice_classH(), pip_practice_classI();
int pip_practice_classJ();

static struct pipcommands pippracticelist[] =
{
	{pip_practice_classA,	'a',	'A'},
	{pip_practice_classB,	'b',	'B'},
	{pip_practice_classC,	'c',	'C'},
	{pip_practice_classD,	'd',	'D'},
	{pip_practice_classE,	'e',	'E'},
	{pip_practice_classF,	'f',	'F'},
	{pip_practice_classG,	'g',	'G'},
	{pip_practice_classH,	'h',	'H'},
	{pip_practice_classI,	'i',	'I'},
	{pip_practice_classJ,	'j',	'J'},
	{NULL,			'\0',	'\0'}
};

/*ª±¼Ö¿ï³æ*/
int pip_play_stroll(), pip_play_sport(), pip_play_date(), pip_play_guess();
int pip_play_outing(), pip_play_kite(), pip_play_KTV();

static struct pipcommands pipplaylist[] =
{
	{pip_play_stroll,	'1',	'1'},
	{pip_play_sport,	'2',	'2'},
	{pip_play_date,		'3',	'3'},
	{pip_play_guess,	'4',	'4'},
	{pip_play_outing,	'5',	'5'},
	{pip_play_kite,		'6',	'6'},
	{pip_play_KTV,		'7',	'7'},
	{NULL,			'\0',	'\0'}
};

/*¥´¤u¿ï³æ*/
int pip_job_workA(), pip_job_workB(), pip_job_workC(), pip_job_workD();
int pip_job_workE(), pip_job_workF(), pip_job_workG(), pip_job_workH();
int pip_job_workI(), pip_job_workJ(), pip_job_workK(), pip_job_workL();
int pip_job_workM(), pip_job_workN(), pip_job_workO(), pip_job_workP();
static struct pipcommands pipjoblist[] =
{
	{pip_job_workA,		'a',	'A'},
	{pip_job_workB,		'b',	'B'},
	{pip_job_workC,		'c',	'C'},
	{pip_job_workD,		'd',	'D'},
	{pip_job_workE,		'e',	'E'},
	{pip_job_workF,		'f',	'F'},
	{pip_job_workG,		'g',	'G'},
	{pip_job_workH,		'h',	'H'},
	{pip_job_workI,		'i',	'I'},
	{pip_job_workJ,		'j',	'J'},
	{pip_job_workK,		'k',	'K'},
	{pip_job_workL,		'l',	'L'},
	{pip_job_workM,		'm',	'M'},
	{pip_job_workN,		'n',	'N'},
	{pip_job_workO,		'o',	'O'},
	{pip_job_workP,		'p',	'P'},
	{NULL,			'\0',	'\0'}
};

/*¯S®í¿ï³æ*/
int pip_see_doctor(), pip_change_weight(), pip_meet_vs_man(), pip_query(), pip_go_palace();
int pip_vs_fight();
static struct pipcommands pipspeciallist[] =
{
	{pip_see_doctor,		'1',	'1'},
	{pip_change_weight,	'2',	'2'},
	{pip_meet_vs_man,	'3',	'3'},
	{pip_query,		'4',	'4'},
	{pip_go_palace,		'5',	'5'},
	/*{pip_vs_fight,		'z',	'z'},*/
	{NULL,			'\0',	'\0'}
};

static struct pipcommands pipsystemlist[] =
{
	{pip_data_list,		'1',	'1'},
	{pip_system_freepip,	'2',	'2'},
	{pip_system_service,	'3',	'3'},
	{pip_write_backup,	'4',	'4'},
	{pip_read_backup,	'5',	'5'},
	/*
		{pip_divine,		'o',	'O'},
		{pip_results_show,	's',	'S'},
	*/
	{NULL,			'\0',	'\0'}
};



/*Ãþ¦ümenu.cªº¥\¯à*/
int
pip_do_menu(menunum, menumode, cmdtable)
int menunum, menumode;
struct pipcommands cmdtable[];
{
	time_t now;
	int key1, key2;
	int pipkey;
	int goback = 0, ok = 0;
	int class1 = 0, class2 = 0, class3 = 0, class4 = 0, class5 = 0;
	int class6 = 0, class7 = 0, class8 = 0, class9 = 0, class10 = 0;
	struct pipcommands *cmd1;
	struct pipcommands *cmd2;

	do
	{
		ok = 0;
		/*§PÂ_¬O§_¦º¤`  ¦º±¼§Y¸õ¦^¤W¤@¼h*/
		if (d.death == 1 || d.death == 2 || d.death == 3)
			return 0;
		/*¸gpip_mainmenu§P©w«á¬O§_¦º¤`*/
		if (pip_mainmenu(menumode))
			return 0;

		class1 = d.wisdom / 200 + 1;			/*¬ì¾Ç*/
		if (class1 > 5)  class1 = 5;
		class2 = (d.affect * 2 + d.wisdom + d.art * 2 + d.character) / 400 + 1; /*¸Öµü*/
		if (class2 > 5)  class2 = 5;
		class3 = (d.belief * 2 + d.wisdom) / 400 + 1;		/*¯«¾Ç*/
		if (class3 > 5)  class3 = 5;
		class4 = (d.hskill * 2 + d.wisdom) / 400 + 1;		/*­x¾Ç*/
		if (class4 > 5)  class4 = 5;
		class5 = (d.hskill + d.attack) / 400 + 1;		/*¼C³N*/
		if (class5 > 5)  class5 = 5;
		class6 = (d.hskill + d.resist) / 400 + 1;		/*®æ°«*/
		if (class6 > 5)  class6 = 5;
		class7 = (d.mskill + d.maxmp) / 400 + 1;		/*Å]ªk*/
		if (class7 > 5)  class7 = 5;
		class8 = (d.manners * 2 + d.character) / 400 + 1;	/*Â§»ö*/
		if (class8 > 5)  class8 = 5;
		class9 = (d.art * 2 + d.character) / 400 + 1; 		/*Ã¸µe*/
		if (class9 > 5)  class9 = 5;
		class10 = (d.art * 2 + d.charm) / 400 + 1;		/*»RÁÐ*/
		if (class10 > 5) class10 = 5;

		clrchyiuan(22, 24);
		move(b_lines - 1, 0);
		prints(menuname[menunum][0], class1, class2, class3, class4, class5);
		move(b_lines, 0);
		prints(menuname[menunum][1], class6, class7, class8, class9, class10);

		now = time(0);
		pip_time_change(now);
		pipkey = vkey();
		now = time(0);
		pip_time_change(now);

		cmd1 = cmdtable;
		cmd2 = cmdtable;
		switch (pipkey)
		{
		case KEY_LEFT:
		case 'q':
		case 'Q':
			goback = 1;
			break;

		default:
			for (; (key1 = cmd1->key1); cmd1++)
				/*if(key == tolower(pipkey))*/
				if (key1 == pipkey)
				{
					cmd1->fptr();
					ok = 1;
				}
			for (; (key2 = cmd2->key2); cmd2++)
				if (ok == 0 && key2 == pipkey)
				{
					cmd2->fptr();
				}
			pip_check_levelup();
			break;
		}
	}
	while (goback == 0);

	return 0;
}


/*---------------------------------------------------------------------------*/
/* °ò¥»¿ï³æ:Áý­¹ ²M¼ä ¿Ë¿Ë ¥ð®§                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/
int pip_main_menu()
{
	pip_do_menu(0, 0, pipmainlist);
	return 0;
}

/*---------------------------------------------------------------------------*/
/* °ò¥»¿ï³æ:Áý­¹ ²M¼ä ¿Ë¿Ë ¥ð®§                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/
int pip_basic_menu()
{
	pip_do_menu(1, 0, pipbasiclist);
	return 0;
}

/*---------------------------------------------------------------------------*/
/* °Ó©±¿ï³æ:­¹ª« ¹s­¹ ¤j¸É¤Y ª±¨ã ®Ñ¥»                                       */
/*                                                                           */
/*---------------------------------------------------------------------------*/
int pip_store_menu()
{
	pip_do_menu(2, 1, pipstorelist);
	return 0;
}

/*---------------------------------------------------------------------------*/
/* ­×¦æ¿ï³æ:©À®Ñ ½mªZ ­×¦æ                                                   */
/*                                                                           */
/*---------------------------------------------------------------------------*/
int pip_practice_menu()
{
	pip_do_menu(3, 3, pippracticelist);
	return 0;
}


/*---------------------------------------------------------------------------*/
/* ª±¼Ö¿ï³æ:´²¨B ®È¹C ¹B°Ê ¬ù·| ²q®±                                         */
/*                                                                           */
/*---------------------------------------------------------------------------*/
int pip_play_menu()
{
	pip_do_menu(4, 0, pipplaylist);
	return 0;
}

/*---------------------------------------------------------------------------*/
/* ¥´¤u¿ï³æ:®a¨Æ ­W¤u ®a±Ð ¦aÅu                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/
int pip_job_menu()
{
	pip_do_menu(5, 2, pipjoblist);
	return 0;
}

/*---------------------------------------------------------------------------*/
/* ¯S®í¿ï³æ:¬Ý¯f ´îªÎ ¾Ô°« «ô³X ´Â¨£                                         */
/*                                                                           */
/*---------------------------------------------------------------------------*/
int pip_special_menu()
{
	pip_do_menu(6, 0, pipspeciallist);
	return 0;
}

/*---------------------------------------------------------------------------*/
/* ¨t²Î¿ï³æ:­Ó¤H¸ê®Æ  ¤pÂû©ñ¥Í  ¯S§OªA°È                                     */
/*                                                                           */
/*---------------------------------------------------------------------------*/
int pip_system_menu()
{
	pip_do_menu(7, 0, pipsystemlist);
	return 0;
}


int
pip_mainmenu(mode)
int mode;
{
	char genbuf[200];
	char inbuf1[20];
	char buf[256];
	time_t now;

	int tm, m, color, tm1, m1;
	int age;
	int color1, color2, color3, color4;
	int anynum;
	float pc;
	char yo[12][5] = {"½Ï¥Í", "À¦¨à", "¥®¨à", "¨àµ£", "¤Ö¦~", "«C¦~",
					  "¦¨¦~", "§§¦~", "§ó¦~", "¦Ñ¦~", "¥jµ}", "¯«¥P"
					 };

	color1 = color2 = color3 = color4 = 37;
	move(1, 0);
	tm = (time(0) - start_time + d.bbtime) / 60 / 30; /* ¤@·³ */
	tm1 = (time(0) - start_time + d.bbtime) / 60;
	m = d.bbtime / 60 / 30;
	m1 = d.bbtime / 60;
	/*ªø¤j¤@·³®Éªº¼W¥[§ïÅÜ­È*/
	if (m != tm)
	{
		d.wisdom += 10;
		d.happy += rand() % 5 + 5;
		if (d.happy > 100)
			d.happy = 100;
		d.satisfy += rand() % 5;
		if (d.satisfy > 100)
			d.satisfy = 100;
		if (tm < 13) d.maxhp += rand() % 5 + 5; else d.maxhp -= rand() % 15;
		d.character += rand() % 5;
		d.money += 500;
		d.seeroyalJ = 1;
		count_tired(1, 7, "N", 100, 0);
		d.bbtime += time(0) - start_time;
		start_time = time(0);
		pip_write_file();

		/*°O¿ý¶}©l*/
		now = time(0);
		sprintf(genbuf, "[1;37m%s %-11sªº¤pÂû [%s] º¡ %d ·³¤F [m\n", Cdate(&now), cuser.userid, d.name, m + 1);
		pip_log_record(genbuf);
		/*°O¿ý²×¤î*/
		clear();
		vs_head("¹q¤l¾i¤pÂû", BoardName);
		show_basic_pic(20); /*¥Í¤é§Ö¼Ö*/
		vmsg("¤pÂûªø¤j¤@·³¤F..");
		/*µ²§½*/
		if (tm % 2 == 0)
			pip_results_show();
		if (tm >= 21 && (d.wantend == 4 || d.wantend == 5 || d.wantend == 6))
			pip_ending_screen();

		clrtobot();
		refresh();
	}
	color = 37;
	m = tm;

	if ((rand() % 3000 == 29) && tm >= 15 && d.charm >= 300 && d.character >= 300)
		pip_marriage_offer();

	if (mode != 1 && rand() % 4000 == 69)
		pip_divine();

	/*ªZ©x*/
	if ((time(0) - start_time) >= 900)
	{
		d.seeroyalJ = 0;
	}

	if (m == 0) /*½Ï¥Í*/
		age = 0;
	else if (m == 1) /*À¦¨à*/
		age = 1;
	else if (m >= 2 && m <= 5) /*¥®¨à*/
		age = 2;
	else if (m >= 6 && m <= 12) /*¨àµ£*/
		age = 3;
	else if (m >= 13 && m <= 15) /*¤Ö¦~*/
		age = 4;
	else if (m >= 16 && m <= 18) /*«C¦~*/
		age = 5;
	else if (m >= 19 && m <= 35) /*¦¨¦~*/
		age = 6;
	else if (m >= 36 && m <= 45) /*§§¦~*/
		age = 7;
	else if (m >= 45 && m <= 60) /*§ó¦~*/
		age = 8;
	else if (m >= 60 && m <= 70) /*¦Ñ¦~*/
		age = 9;
	else if (m >= 70 && m <= 100) /*¥jµ}*/
		age = 10;
	else if (m > 100) /*¯«¥P*/
		age = 11;
	clear();
	/*vs_head("¹q¤l¾i¤pÂû", BoardName);*/
	move(0, 0);
	if (d.sex == 1)
		sprintf(buf, "[1;41m  " NICKNAME PIPNAME " ¡ã [%s¥NÂû] [32m¡ñ [37m%-15s                                  [m", d.chickenmode ? "¤G" : "¤@", d.name);
	else if (d.sex == 2)
		sprintf(buf, "[1;41m  " NICKNAME PIPNAME " ¡ã [%s¥NÂû] [33m¡ð [37m%-15s                                  [m", d.chickenmode ? "¤G" : "¤@", d.name);
	else
		sprintf(buf, "[1;41m  " NICKNAME PIPNAME " ¡ã [%s¥NÂû] [34m¡H [37m%-15s                                  [m", d.chickenmode ? "¤G" : "¤@", d.name);
	prints(buf);

	move(1, 0);
	if (d.money <= 100)
		color1 = 31;
	else if (d.money > 100 && d.money <= 500)
		color1 = 33;
	else
		color1 = 37;
	sprintf(inbuf1, "%02d/%02d/%02d", (d.year - 11) % 100, d.month, d.day);
	sprintf(buf
			, " [1;32m[ª¬  ºA][37m %-5s     [32m[¥Í  ¤é][37m %-9s [32m[¦~  ÄÖ][37m %-5d     [32m[ª÷  ¿ú][%dm %-8d [m"
			, yo[age], inbuf1, tm, color1, d.money);
	prints(buf);

	move(2, 0);

	if ((d.hp*100 / d.maxhp) <= 20)
		color1 = 31;
	else if ((d.hp*100 / d.maxhp) <= 40 && (d.hp*100 / d.maxhp) > 20)
		color1 = 33;
	else
		color1 = 37;
	if (d.maxmp == 0)
		color2 = 37;
	else if ((d.mp*100 / d.maxmp) <= 20)
		color2 = 31;
	else if ((d.mp*100 / d.maxmp) <= 40 && (d.mp*100 / d.maxmp) > 20)
		color2 = 33;
	else
		color2 = 37;

	if (d.tired >= 80)
		color3 = 31;
	else if (d.tired < 80 && d.tired >= 60)
		color3 = 33;
	else
		color3 = 37;

	sprintf(buf
			, " [1;32m[¥Í  ©R][%dm %-10d[32m[ªk  ¤O][%dm %-10d[32m[Åé  ­«][37m %-5d     [32m[¯h  ³Ò][%dm %-4d[0m "
			, color1, d.hp, color2, d.mp, d.weight, color3, d.tired);
	prints(buf);

	move(3, 0);
	if (d.shit >= 80)
		color1 = 31;
	else if (d.shit < 80 && d.shit >= 60)
		color1 = 33;
	else
		color1 = 37;
	if (d.sick >= 75)
		color2 = 31;
	else if (d.sick < 75 && d.sick >= 50)
		color2 = 33;
	else
		color2 = 37;
	if (d.happy <= 20)
		color3 = 31;
	else if (d.happy > 20 && d.happy <= 40)
		color3 = 33;
	else
		color3 = 37;
	if (d.satisfy <= 20)
		color4 = 31;
	else if (d.satisfy > 20 && d.satisfy <= 40)
		color4 = 33;
	else
		color4 = 37;
	sprintf(buf
			, " [1;32m[©R MAX][37m %-10d[32m[ªk MAX][37m %-10d[32m[Å¼¡þ¯f][%dm %-4d[37m/[%dm%-4d [32m[§Ö¡þº¡][%dm %-4d[37m/[%dm%-4d[m"
			, d.maxhp, d.maxmp, color1, d.shit, color2, d.sick, color3, d.happy, color4, d.satisfy);
	prints(buf);
	if (mode == 0)  /*¥D­nµe­±*/
	{
		anynum = 0;
		anynum = rand() % 4;
		move(4, 0);
		if (anynum == 0)
			sprintf(buf, " [1;35m[¯¸ªø¤ê]:[31m¬õ¦â[36mªí¥Ü¦MÀI  [33m¶À¦â[36mªí¥ÜÄµ§i  [37m¥Õ¦â[36mªí¥Ü¦w¥þ[0m");
		else if (anynum == 1)
			sprintf(buf, " [1;35m[¯¸ªø¤ê]:[37m­n¦h¦hª`·N¤pÂûªº¯h³Ò«×©M¯f®ð  ¥H§K²Ö¦º¯f¦º[0m");
		else if (anynum == 2)
			sprintf(buf, " [1;35m[¯¸ªø¤ê]:[37mÀH®Éª`·N¤pÂûªº¥Í©R¼Æ­È­ò![0m");
		else if (anynum == 3)
			sprintf(buf, " [1;35m[¯¸ªø¤ê]:[37m§Ö§Ö¼Ö¼Öªº¤pÂû¤~¬O©¯ºÖªº¤pÂû.....[0m");
		prints(buf);
	}
	else if (mode == 1)/*Áý­¹*/
	{
		move(4, 0);
		if (d.food == 0)
			color1 = 31;
		else if (d.food <= 5 && d.food > 0)
			color1 = 33;
		else
			color1 = 37;
		if (d.cookie == 0)
			color2 = 31;
		else if (d.cookie <= 5 && d.cookie > 0)
			color2 = 33;
		else
			color2 = 37;
		if (d.bighp == 0)
			color3 = 31;
		else if (d.bighp <= 2 && d.bighp > 0)
			color3 = 33;
		else
			color3 = 37;
		if (d.medicine == 0)
			color4 = 31;
		else if (d.medicine <= 5 && d.medicine > 0)
			color4 = 33;
		else
			color4 = 37;
		sprintf(buf
				, " [1;36m[­¹ª«][%dm%-7d[36m[¹s­¹][%dm%-7d[36m[¸É¤Y][%dm%-7d[36m[ÆFªÛ][%dm%-7d[36m[¤H°Ñ][37m%-7d[36m[³·½¬][37m%-7d[0m"
				, color1, d.food, color2, d.cookie, color3, d.bighp, color4, d.medicine, d.ginseng, d.snowgrass);
		prints(buf);

	}
	else if (mode == 2)/*¥´¤u*/
	{
		move(4, 0);
		sprintf(buf
				, " [1;36m[·R¤ß][37m%-5d[36m[´¼¼z][37m%-5d[36m[®ð½è][37m%-5d[36m[ÃÀ³N][37m%-5d[36m[¹D¼w][37m%-5d[36m[«i´±][37m%-5d[36m[®a¨Æ][37m%-5d[0m"
				, d.love, d.wisdom, d.character, d.art, d.etchics, d.brave, d.homework);
		prints(buf);

	}
	else if (mode == 3)/*­×¦æ*/
	{
		move(4, 0);
		sprintf(buf
				, " [1;36m[´¼¼z][37m%-5d[36m[®ð½è][37m%-5d[36m[ÃÀ³N][37m%-5d[36m[«i´±][37m%-5d[36m[§ðÀ»][37m%-5d[36m[¨¾¿m][37m%-5d[36m[³t«×][37m%-5d[0m"
				, d.wisdom, d.character, d.art, d.brave, d.attack, d.resist, d.speed);
		prints(buf);

	}
	move(5, 0);
	prints("[1;%dm¢z¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢{[m", color);
	move(6, 0);
	switch (age)
	{
	case 0:
	case 1:
	case 2:
		if (d.weight <= (60 + 10*tm - 30))
			show_basic_pic(1);
		else if (d.weight > (60 + 10*tm - 30) && d.weight < (60 + 10*tm + 30))
			show_basic_pic(2);
		else if (d.weight >= (60 + 10*tm + 30))
			show_basic_pic(3);
		break;
	case 3:
	case 4:
		if (d.weight <= (60 + 10*tm - 30))
			show_basic_pic(4);
		else if (d.weight > (60 + 10*tm - 30) && d.weight < (60 + 10*tm + 30))
			show_basic_pic(5);
		else if (d.weight >= (60 + 10*tm + 30))
			show_basic_pic(6);
		break;
	case 5:
	case 6:
		if (d.weight <= (60 + 10*tm - 30))
			show_basic_pic(7);
		else if (d.weight > (60 + 10*tm - 30) && d.weight < (60 + 10*tm + 30))
			show_basic_pic(8);
		else if (d.weight >= (60 + 10*tm + 30))
			show_basic_pic(9);
		break;
	case 7:
	case 8:
		if (d.weight <= (60 + 10*tm - 30))
			show_basic_pic(10);
		else if (d.weight > (60 + 10*tm - 30) && d.weight < (60 + 10*tm + 30))
			show_basic_pic(11);
		else if (d.weight >= (60 + 10*tm + 30))
			show_basic_pic(12);
		break;
	case 9:
		show_basic_pic(13);
		break;
	case 10:
	case 11:
		show_basic_pic(16);
		break;
	}


	move(18, 0);
	prints("[1;%dm¢|¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢}[m", color);
	move(19, 0);
	prints(" [1;34m¢w[37;44m  ª¬ ºA  [0;1;34m¢w[0m");
	move(20, 0);
	prints(" ");
	if (d.shit == 0)
		prints("°®²b¤pÂû  ");
	if (d.shit > 40 && d.shit < 60)
		prints("¦³ÂI¯ä¯ä  ");
	if (d.shit >= 60 && d.shit < 80)
		prints("[1;33m«Ü¯ä¤F»¡[m  ");
	if (d.shit >= 80 && d.shit < 100)
	{
		prints("[1;35m§Ö¯ä¦º¤F[m  ");
		d.sick += 4;
		d.character -= (rand() % 3 + 3);
	}
	if (d.shit >= 100)
	{
		d.death = 1;
		pipdie("[1;31m«z¡ã¯ä¦º¤F[m  ", 1);
		return -1;
	}

	if (d.hp <= 0)
		pc = 0;
	else
		pc = d.hp * 100 / d.maxhp;
	if (pc == 0)
	{
		d.death = 1;
		pipdie("[1;31m¶ã¡ã¾j¦º¤F[m  ", 1);
		return -1;
	}
	if (pc < 20)
	{
		prints("[1;35m§Ö¾j©ü¤F[m  ");
		d.sick += 3;
		d.happy -= 5;
		d.satisfy -= 3;
	}
	if (pc < 40 && pc >= 20)
		prints("[1;33m·Q¦YªF¦è[m  ");
	if (pc <= 100 && pc >= 90)
		prints("¨{¤l¹¡¹¡  ");
	if (pc < 110 && pc > 100)
		prints("[1;33m¼µ¼µªº»¡[m  ");

	pc = d.tired;
	if (pc < 20)
		prints("ºë¯««Ü¦n  ");
	if (pc < 80 && pc >= 60)
		prints("[1;33m¦³ÂI¤p²Ö[m  ");
	if (pc < 100 && pc >= 80)
	{
		prints("[1;35m¯uªº«Ü²Ö[m  ");
		d.sick += 5;
	}
	if (pc >= 100)
	{
		d.death = 1;
		pipdie("[1;31m£«¡ã²Ö¦º¤F[m  ", 1);
		return -1;
	}

	pc = 60 + 10 * tm;
	if (d.weight < (pc + 30) && d.weight >= (pc + 10))
		prints("[1;33m¦³ÂI¤p­D[m  ");
	if (d.weight < (pc + 50) && d.weight >= (pc + 30))
	{
		prints("[1;35m¤Ó­D¤F°Õ[m  ");
		d.sick += 3;
		if (d.speed >= 2)
			d.speed -= 2;
		else
			d.speed = 0;

	}
	if (d.weight > (pc + 50))
	{
		d.death = 1;
		pipdie("[1;31m¶ã¡ãªÎ¦º¤F[m  ", 1);
		return -1;
	}

	if (d.weight < (pc - 50))
	{
		d.death = 1;
		pipdie("[1;31m:~~ ½G¦º¤F[m  ", 1);
		return -1;
	}
	if (d.weight > (pc - 30) && d.weight <= (pc - 10))
		prints("[1;33m¦³ÂI¤p½G[m  ");
	if (d.weight > (pc - 50) && d.weight <= (pc - 30))
		prints("[1;35m¤Ó½G¤F³á[m ");

	if (d.sick < 75 && d.sick >= 50)
	{
		prints("[1;33m¥Í¯f¤F°Õ[m  ");
		count_tired(1, 8, "Y", 100, 1);
	}
	if (d.sick < 100 && d.sick >= 75)
	{
		prints("[1;35m¥¿¯f­«¤¤[m  ");
		d.sick += 5;
		count_tired(1, 15, "Y", 100, 1);
	}
	if (d.sick >= 100)
	{
		d.death = 1;
		pipdie("[1;31m¯f¦º¤F°Õ :~~[m  ", 1);
		return -1;
	}

	pc = d.happy;
	if (pc < 20)
		prints("[1;35m«Ü¤£§Ö¼Ö[m  ");
	if (pc < 40 && pc >= 20)
		prints("[1;33m¤£¤Ó§Ö¼Ö[m  ");
	if (pc < 95 && pc >= 80)
		prints("§Ö¼Ö°Õ..  ");
	if (pc <= 100 && pc >= 95)
		prints("«Ü§Ö¼Ö..  ");

	pc = d.satisfy;
	if (pc < 20) prints("[1;35m«Ü¤£º¡¨¬..[m  ");
	if (pc < 40 && pc >= 20) prints("[1;33m¤£¤Óº¡¨¬[m  ");
	if (pc < 95 && pc >= 80) prints("º¡¨¬°Õ..  ");
	if (pc <= 100 && pc >= 95) prints("«Üº¡¨¬..  ");

	prints("\n");

	pip_write_file();
	return 0;
}

/*©T©w®É¶¡§@ªº¨Æ */
int
pip_time_change(cnow)
time_t cnow;
{
	int stime = 60;
	int stired = 2;
	while ((time(0) - lasttime) >= stime) /* ©T©w®É¶¡°µªº¨Æ */
	{
		/*¤£°µ¨Æ  ÁÙ¬O·|ÅÜÅ¼ªº*/
		if ((time(0) - cnow) >= stime)
			d.shit += (rand() % 3 + 3);
		/*¤£°µ¨Æ  ¯h³Ò·íµM´î§C°Õ*/
		if (d.tired >= stired) d.tired -= stired; else d.tired = 0;
		/*¤£°µ¨Æ  ¨{¤l¤]·|¾j«§ */
		d.hp -= rand() % 2 + 2;
		if (d.mexp < 0)
			d.mexp = 0;
		if (d.hexp < 0)
			d.hexp = 0;
		/*Åé¤O·|¦]¥Í¯f­°§C¤@ÂI*/
		d.hp -= d.sick / 10;
		/*¯f®ð·|ÀH¾÷²v¼W¥[´î¤Ö¤Ö³\*/
		if (rand() % 3 > 0)
		{
			d.sick -= rand() % 2;
			if (d.sick < 0)
				d.sick = 0;
		}
		else
			d.sick += rand() % 2;
		/*ÀH¾÷´î§Ö¼Ö«×*/
		if (rand() % 4 > 0)
		{
			d.happy -= rand() % 2 + 2;
		}
		else
			d.happy += 2;
		if (rand() % 4 > 0)
		{
			d.satisfy -= (rand() % 4 + 5);
		}
		else
			d.satisfy += 2;
		lasttime += stime;
	};
	/*§Ö¼Ö«×º¡·N«×³Ì¤j­È³]©w*/
	if (d.happy > 100)
		d.happy = 100;
	else if (d.happy < 0)
		d.happy = 0;
	if (d.satisfy > 100)
		d.satisfy = 100;
	else if (d.satisfy < 0)
		d.satisfy = 0;
	/*µû»ù*/
	if (d.social < 0)
		d.social = 0;
	if (d.tired < 0)
		d.tired = 0;
	if (d.hp > d.maxhp)
		d.hp = d.maxhp;
	if (d.mp > d.maxmp)
		d.mp = d.maxmp;
	if (d.money < 0)
		d.money = 0;
	if (d.charm < 0)
		d.charm = 0;
}

/*---------------------------------------------------------------------------*/
/* °ò¥»¿ï³æ:Áý­¹ ²M¼ä ¿Ë¿Ë ¥ð®§                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

int pip_basic_takeshower() /*¬~¾þ*/
{
	int lucky;
	d.shit -= 20;
	if (d.shit < 0) d.shit = 0;
	d.hp -= rand() % 2 + 3;
	move(4, 0);
	lucky = rand() % 3;
	if (lucky == 0)
	{
		show_usual_pic(1);
		vmsg("§Ú¬O°®²bªº¤pÂû  cccc....");
	}
	else if (lucky == 1)
	{
		show_usual_pic(7);
		vmsg("°¨±í ¶â¡ã¡ã");
	}
	else
	{
		show_usual_pic(2);
		vmsg("§Ú·R¬~¾þ lalala....");
	}
	return 0;
}

int pip_basic_takerest() /*¥ð®§*/
{
	count_tired(5, 20, "Y", 100, 0);
	if (d.hp > d.maxhp)
		d.hp = d.maxhp;
	d.shit += 1;
	move(4, 0);
	show_usual_pic(5);
	vmsg("¦A«ö¤@¤U§Ú´N°_§ÉÅo....");
	show_usual_pic(6);
	vmsg("³Þ³Þ³Þ..¸Ó°_§ÉÅo......");
	return 0;
}

int pip_basic_kiss()/*¿Ë¿Ë*/
{
	if (rand() % 2 > 0)
	{
		d.happy += rand() % 3 + 4;
		d.satisfy += rand() % 2 + 1;
	}
	else
	{
		d.happy += rand() % 2 + 1;
		d.satisfy += rand() % 3 + 4;
	}
	count_tired(1, 2, "N", 100, 1);
	d.shit += rand() % 5 + 4;
	d.relation += rand() % 2;
	move(4, 0);
	show_usual_pic(3);
	if (d.shit < 60)
	{
		vmsg("¨Ó¹À! Ôq¤@­Ó.....");
	}
	else
	{
		vmsg("¿Ë¤Ó¦h¤]¬O·|Å¼¦ºªº³á....");
	}
	return 0;
}

int pip_basic_feed()     /* Áý­¹*/
{
	time_t now;
	char buf[256];
	int pipkey;

	d.nodone = 1;

	do
	{
		if (d.death == 1 || d.death == 2 || d.death == 3)
			return 0;
		if (pip_mainmenu(1)) return 0;
		move(b_lines -2, 0);
		clrtoeol();
		move(b_lines - 2, 1);
		sprintf(buf, "%s¸Ó°µ¤°»ò¨Æ©O?", d.name);
		prints(buf);
		now = time(0);
		move(b_lines, 0);
		clrtoeol();
		move(b_lines, 0);
		prints("[1;44;37m  ¶¼­¹¿ï³æ  [46m[1]¦Y¶º [2]¹s­¹ [3]¸É¤Y [4]ÆFªÛ [5]¤Hçx [6]³·½¬ [Q]¸õ¥X¡G         [m");
		pip_time_change(now);
		pipkey = vkey();
		pip_time_change(now);

		switch (pipkey)
		{
		case '1':
			if (d.food <= 0)
			{
				move(b_lines, 0);
				vmsg("¨S¦³­¹ª«Åo..§Ö¥h¶R§a¡I");
				break;
			}
			move(4, 0);
			if ((d.bbtime / 60 / 30) < 3)
				show_feed_pic(0);
			else
				show_feed_pic(1);
			d.food--;
			d.hp += 50;
			if (d.hp >= d.maxhp)
			{
				d.hp = d.maxhp;
				d.weight += rand() % 2;
			}
			d.nodone = 0;
			vmsg("¨C¦Y¤@¦¸­¹ª«·|«ì´_Åé¤O50³á!");
			break;

		case '2':
			if (d.cookie <= 0)
			{
				move(b_lines, 0);
				vmsg("¹s­¹¦Y¥úÅo..§Ö¥h¶R§a¡I");
				break;
			}
			move(4, 0);
			d.cookie--;
			d.hp += 100;
			if (d.hp >= d.maxhp)
			{
				d.hp = d.maxhp;
				d.weight += (rand() % 2 + 2);
			}
			else
			{
				d.weight += (rand() % 2 + 1);
			}
			if (rand() % 2 > 0)
				show_feed_pic(2);
			else
				show_feed_pic(3);
			d.happy += (rand() % 3 + 4);
			d.satisfy += rand() % 3 + 2;
			d.nodone = 0;
			vmsg("¦Y¹s­¹®e©ö­D³á...");
			break;

		case '3':
			if (d.bighp <= 0)
			{
				move(b_lines, 0);
				vmsg("¨S¦³¤j¸É¤Y¤F­C! §Ö¶R§a..");
				break;
			}
			d.bighp--;
			d.hp += 600;
			d.tired -= 20;
			d.weight += rand() % 2;
			move(4, 0);
			show_feed_pic(4);
			d.nodone = 0;
			vmsg("¸É¤Y..¶W·¥´Îªº­ò...");
			break;

		case '4':
			if (d.medicine <= 0)
			{
				move(b_lines, 0);
				vmsg("¨S¦³ÆFªÛÅo..§Ö¥h¶R§a¡I");
				break;
			}
			move(4, 0);
			show_feed_pic(1);
			d.medicine--;
			d.mp += 50;
			if (d.mp >= d.maxmp)
			{
				d.mp = d.maxmp;
			}
			d.nodone = 0;
			vmsg("¨C¦Y¤@¦¸ÆFªÛ·|«ì´_ªk¤O50³á!");
			break;

		case '5':
			if (d.ginseng <= 0)
			{
				move(b_lines, 0);
				vmsg("¨S¦³¤d¦~¤Hçx­C! §Ö¶R§a..");
				break;
			}
			d.ginseng--;
			d.mp += 500;
			d.tired -= 20;
			move(4, 0);
			show_feed_pic(1);
			d.nodone = 0;
			vmsg("¤d¦~¤Hçx..¶W·¥´Îªº­ò...");
			break;

		case '6':
			if (d.snowgrass <= 0)
			{
				move(b_lines, 0);
				vmsg("¨S¦³¤Ñ¤s³·½¬­C! §Ö¶R§a..");
				break;
			}
			d.snowgrass--;
			d.mp = d.maxmp;
			d.hp = d.maxhp;
			d.tired -= 0;
			d.sick = 0;
			move(4, 0);
			show_feed_pic(1);
			d.nodone = 0;
			vmsg("¤Ñ¤s³·½¬..¶W·¥´Îªº­ò...");
			break;

		}
	}
	while ((pipkey != 'Q') && (pipkey != 'q') && (pipkey != KEY_LEFT));

	return 0;
}

/*¹CÀ¸¼g¸ê®Æ¤JÀÉ®×*/
void pip_write_file()
{
	FILE *ff;
	char buf[200];
	/* sprintf(buf,"home/%s/chicken",cuser.userid);*/
	usr_fpath(buf, cuser.userid, "chicken");

	if ((ff = fopen(buf, "w")))
	{
		fprintf(ff, "%lu\n", d.bbtime);
		fprintf(ff,
				"%d %d %d %d %d %d %d %d %d \n\
				%d %d %d %d %d %d %d %d %d %d \n\
				%d %d %d %d %d %d %d %d %d %d \n\
				%d %d %d %d %d %d %d %d %d %d \n\
				%d %d %d %d %d %d %d %d %d %d \n\
				%d %d %d %d %d %d %d %d %d %d \n\
				%d %d %d %d %d %d %d %d %d %d \n\
				%d %d %d %d %d %d %d %d %d %d \n\
				%d %d %d %d %d %d %d %d %d %d \n\
				%d %d %d %d %d %d %d %d %d %d \n\
				%d %d %d %d %d %d %d %d %d %d \n\
				%d %d %d %d %d %d %d %d %d %d \n\
				%d %d %s %d %d %d %d %d %d %d \n\
				%d %d %d %d %d %d %d %d %d %d \n\
				%d %d %d %d %d %d %d %d %d %d \n\
				%d %d %d %d %d %d %d %d %d %d %d %d %d %d",
				d.year, d.month, d.day, d.sex, d.death, d.nodone, d.relation, d.liveagain, d.chickenmode, d.level, d.exp, d.dataE,
				d.hp, d.maxhp, d.weight, d.tired, d.sick, d.shit, d.wrist, d.bodyA, d.bodyB, d.bodyC, d.bodyD, d.bodyE,
				d.social, d.family, d.hexp, d.mexp, d.tmpA, d.tmpB, d.tmpC, d.tmpD, d.tmpE,
				d.mp, d.maxmp, d.attack, d.resist, d.speed, d.hskill, d.mskill, d.mresist, d.magicmode, d.specialmagic, d.fightC, d.fightD, d.fightE,
				d.weaponhead, d.weaponrhand, d.weaponlhand, d.weaponbody, d.weaponfoot, d.weaponA, d.weaponB, d.weaponC, d.weaponD, d.weaponE,
				d.toman, d.character, d.love, d.wisdom, d.art, d.etchics, d.brave, d.homework, d.charm, d.manners, d.speech, d.cookskill, d.learnA, d.learnB, d.learnC, d.learnD, d.learnE,
				d.happy, d.satisfy, d.fallinlove, d.belief, d.offense, d.affect, d.stateA, d.stateB, d.stateC, d.stateD, d.stateE,
				d.food, d.medicine, d.bighp, d.cookie, d.ginseng, d.snowgrass, d.eatC, d.eatD, d.eatE,
				d.book, d.playtool, d.money, d.thingA, d.thingB, d.thingC, d.thingD, d.thingE,
				d.winn, d.losee,
				d.royalA, d.royalB, d.royalC, d.royalD, d.royalE, d.royalF, d.royalG, d.royalH, d.royalI, d.royalJ, d.seeroyalJ, d.seeA, d.seeB, d.seeC, d.seeD, d.seeE,
				d.wantend, d.lover, d.name,
				d.classA, d.classB, d.classC, d.classD, d.classE,
				d.classF, d.classG, d.classH, d.classI, d.classJ,
				d.classK, d.classL, d.classM, d.classN, d.classO,
				d.workA, d.workB, d.workC, d.workD, d.workE,
				d.workF, d.workG, d.workH, d.workI, d.workJ,
				d.workK, d.workL, d.workM, d.workN, d.workO,
				d.workP, d.workQ, d.workR, d.workS, d.workT,
				d.workU, d.workV, d.workW, d.workX, d.workY, d.workZ
			   );
		fclose(ff);
	}
}

/*¹CÀ¸Åª¸ê®Æ¥XÀÉ®×*/
void pip_read_file(char *userid)
{
	FILE *fs;
	char buf[200];
	/* sprintf(buf,"home/%s/chicken",userid);*/
	usr_fpath(buf, userid, "chicken");

	if ((fs = fopen(buf, "r")))
	{
		fgets(buf, 80, fs);
		d.bbtime = (time_t) atol(buf);

		fscanf(fs,
			   "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %s %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
			   &(d.year), &(d.month), &(d.day), &(d.sex), &(d.death), &(d.nodone), &(d.relation), &(d.liveagain), &(d.chickenmode), &(d.level), &(d.exp), &(d.dataE),
			   &(d.hp), &(d.maxhp), &(d.weight), &(d.tired), &(d.sick), &(d.shit), &(d.wrist), &(d.bodyA), &(d.bodyB), &(d.bodyC), &(d.bodyD), &(d.bodyE),
			   &(d.social), &(d.family), &(d.hexp), &(d.mexp), &(d.tmpA), &(d.tmpB), &(d.tmpC), &(d.tmpD), &(d.tmpE),
			   &(d.mp), &(d.maxmp), &(d.attack), &(d.resist), &(d.speed), &(d.hskill), &(d.mskill), &(d.mresist), &(d.magicmode), &(d.specialmagic), &(d.fightC), &(d.fightD), &(d.fightE),
			   &(d.weaponhead), &(d.weaponrhand), &(d.weaponlhand), &(d.weaponbody), &(d.weaponfoot), &(d.weaponA), &(d.weaponB), &(d.weaponC), &(d.weaponD), &(d.weaponE),
			   &(d.toman), &(d.character), &(d.love), &(d.wisdom), &(d.art), &(d.etchics), &(d.brave), &(d.homework), &(d.charm), &(d.manners), &(d.speech), &(d.cookskill), &(d.learnA), &(d.learnB), &(d.learnC), &(d.learnD), &(d.learnE),
			   &(d.happy), &(d.satisfy), &(d.fallinlove), &(d.belief), &(d.offense), &(d.affect), &(d.stateA), &(d.stateB), &(d.stateC), &(d.stateD), &(d.stateE),
			   &(d.food), &(d.medicine), &(d.bighp), &(d.cookie), &(d.ginseng), &(d.snowgrass), &(d.eatC), &(d.eatD), &(d.eatE),
			   &(d.book), &(d.playtool), &(d.money), &(d.thingA), &(d.thingB), &(d.thingC), &(d.thingD), &(d.thingE),
			   &(d.winn), &(d.losee),
			   &(d.royalA), &(d.royalB), &(d.royalC), &(d.royalD), &(d.royalE), &(d.royalF), &(d.royalG), &(d.royalH), &(d.royalI), &(d.royalJ), &(d.seeroyalJ), &(d.seeA), &(d.seeB), &(d.seeC), &(d.seeD), &(d.seeE),
			   &(d.wantend), &(d.lover), d.name,
			   &(d.classA), &(d.classB), &(d.classC), &(d.classD), &(d.classE),
			   &(d.classF), &(d.classG), &(d.classH), &(d.classI), &(d.classJ),
			   &(d.classK), &(d.classL), &(d.classM), &(d.classN), &(d.classO),
			   &(d.workA), &(d.workB), &(d.workC), &(d.workD), &(d.workE),
			   &(d.workF), &(d.workG), &(d.workH), &(d.workI), &(d.workJ),
			   &(d.workK), &(d.workL), &(d.workM), &(d.workN), &(d.workO),
			   &(d.workP), &(d.workQ), &(d.workR), &(d.workS), &(d.workT),
			   &(d.workU), &(d.workV), &(d.workW), &(d.workX), &(d.workY), &(d.workZ)
			  );

		fclose(fs);
	}
	else
	{
		vmsg("§Ú¨S¦³¾i¤pÂû°Õ !");
		return;
	}

	return;
}

/*°O¿ý¨ìpip.logÀÉ*/
int
pip_log_record(msg)
char *msg;
{
	FILE *fs;

	fs = fopen(FN_PIP_LOG, "a+");
	fprintf(fs, "%s", msg);
	fclose(fs);
}

/*¤pÂû¶i«×Àx¦s*/
int
pip_write_backup()
{
	char *files[4] = {"¨S¦³", "¶i«×¤@", "¶i«×¤G", "¶i«×¤T"};
	char buf[200], buf1[200];
	char ans[3];
	int num = 0;
	int pipkey;

	show_system_pic(21);
	pip_write_file();
	do
	{
		move(b_lines - 2, 0);
		clrtoeol();
		move(b_lines - 1, 0);
		clrtoeol();
		move(b_lines - 1, 1);
		prints("Àx¦s [1]¶i«×¤@ [2]¶i«×¤G [3]¶i«×¤T [Q]©ñ±ó [1/2/3/Q]¡G");
		pipkey = vkey();

		if (pipkey == '1')
			num = 1;
		else if (pipkey == '2')
			num = 2;
		else if (pipkey == '3')
			num = 3;
		else
			num = 0;

	}
	while (pipkey != 'Q' && pipkey != 'q' && num != 1 && num != 2 && num != 3);
	if (pipkey == 'q' || pipkey == 'Q')
	{
		vmsg("©ñ±óÀx¦s¹CÀ¸¶i«×");
		return 0;
	}
	move(b_lines -2, 1);
	prints("Àx¦sÀÉ®×·|ÂÐ»\\­ìÀx¦s©ó [%s] ªº¤pÂûªºÀÉ®×³á¡I½Ð¦Ò¼{²M·¡...", files[num]);
	sprintf(buf1, "½T©w­nÀx¦s©ó [%s] ÀÉ®×¶Ü¡H [y/N]: ", files[num]);
	getdata(b_lines - 1, 1, buf1, ans, 2, DOECHO, 0);
	if (ans[0] != 'y' && ans[0] != 'Y')
	{
		vmsg("©ñ±óÀx¦sÀÉ®×");
		return 0;
	}

	move(b_lines -1, 0);
	clrtobot();
	sprintf(buf1, "Àx¦s [%s] ÀÉ®×§¹¦¨¤F", files[num]);
	vmsg(buf1);
	sprintf(buf, "/bin/cp %s %s.bak%d", get_path(cuser.userid, "chicken"), get_path(cuser.userid, "chicken"), num);
	system(buf);
	return 0;
}

int
pip_read_backup()
{
	char buf[200], buf1[200], buf2[200];
	char *files[4] = {"¨S¦³", "¶i«×¤@", "¶i«×¤G", "¶i«×¤T"};
	char ans[3];
	int pipkey;
	int num = 0;
	int ok = 0;
	FILE *fs;
	show_system_pic(22);
	do
	{
		move(b_lines - 2, 0);
		clrtoeol();
		move(b_lines - 1, 0);
		clrtoeol();
		move(b_lines - 1, 1);
		prints("Åª¨ú [1]¶i«×¤@ [2]¶i«×¤G [3]¶i«×¤T [Q]©ñ±ó [1/2/3/Q]¡G");
		pipkey = vkey();

		if (pipkey == '1')
			num = 1;
		else if (pipkey == '2')
			num = 2;
		else if (pipkey == '3')
			num = 3;
		else
			num = 0;

		if (num > 0)
		{
			usr_fpath(buf, cuser.userid, "chicken.bak");
			sprintf(buf, "%s%d", buf, num);
			if ((fs = fopen(buf, "r")) == NULL)
			{
				sprintf(buf, "ÀÉ®× [%s] ¤£¦s¦b", files[num]);
				vmsg(buf);
				ok = 0;
			}
			else
			{

				move(b_lines - 2, 1);
				prints("Åª¨ú¥XÀÉ®×·|ÂÐ»\\²{¦b¥¿¦bª±ªº¤pÂûªºÀÉ®×³á¡I½Ð¦Ò¼{²M·¡...");
				sprintf(buf, "½T©w­nÅª¨ú¥X [%s] ÀÉ®×¶Ü¡H [y/N]: ", files[num]);
				getdata(b_lines - 1, 1, buf, ans, 2, DOECHO, 0);
				if (ans[0] != 'y' && ans[0] != 'Y')
					vmsg("Åý§Ú¦A¨M©w¤@¤U...");
				else ok = 1;
				fclose(fs);
			}
		}
	}
	while (pipkey != 'Q' && pipkey != 'q' && ok != 1);
	if (pipkey == 'q' || pipkey == 'Q')
	{
		vmsg("ÁÙ¬Oª±­ì¥»ªº¹CÀ¸");
		return 0;
	}

	move(b_lines -1, 0);
	clrtobot();
	sprintf(buf, "Åª¨ú [%s] ÀÉ®×§¹¦¨¤F", files[num]);
	vmsg(buf);

	sprintf(buf1, "/bin/touch %s%d", get_path(cuser.userid, "chicken.bak"), num);
	sprintf(buf2, "/bin/cp %s.bak%d %s", get_path(cuser.userid, "chicken"), num, get_path(cuser.userid, "chicken"));
	system(buf1);
	system(buf2);
	pip_read_file(cuser.userid);
	return 0;
}



int
pip_live_again()
{
	char genbuf[80];
	time_t now;
	int tm;

	tm = (d.bbtime) / 60 / 30;

	clear();
	vs_head("¤pÂû´_¬¡¤â³N¤¤", BoardName);

	now = time(0);
	sprintf(genbuf, "[1;33m%s %-11sªº¤pÂû [%s¤G¥N] ´_¬¡¤F¡I[m\n", Cdate(&now), cuser.userid, d.name);
	pip_log_record(genbuf);

	/*¨­Åé¤Wªº³]©w*/
	d.death = 0;
	d.maxhp = d.maxhp * ALIVE + 1;
	d.hp = d.maxhp;
	d.tired = 20;
	d.shit = 20;
	d.sick = 20;
	d.wrist = d.wrist * ALIVE;
	d.weight = 45 + 10 * tm;

	/*¿ú´î¨ì¤­¤À¤§¤@*/
	d.money = d.money * ALIVE;

	/*¾Ô°«¯à¤O­°¤@¥b*/
	d.attack = d.attack * ALIVE;
	d.resist = d.resist * ALIVE;
	d.maxmp = d.maxmp * ALIVE;
	d.mp = d.maxmp;

	/*ÅÜªº¤£§Ö¼Ö*/
	d.happy = 0;
	d.satisfy = 0;

	/*µû»ù´î¥b*/
	d.social = d.social * ALIVE;
	d.family = d.family * ALIVE;
	d.hexp = d.hexp * ALIVE;
	d.mexp = d.mexp * ALIVE;

	/*ªZ¾¹±¼¥ú¥ú*/
	d.weaponhead = 0;
	d.weaponrhand = 0;
	d.weaponlhand = 0;
	d.weaponbody = 0;
	d.weaponfoot = 0;

	/*­¹ª«³Ñ¤@¥b*/
	d.food = d.food * ALIVE;
	d.medicine = d.medicine * ALIVE;
	d.bighp = d.bighp * ALIVE;
	d.cookie = d.cookie * ALIVE;

	d.liveagain += 1;

	vmsg("¤pÂû¾¹©x­««Ø¤¤¡I");
	vmsg("¤pÂûÅé½è«ì´_¤¤¡I");
	vmsg("¤pÂû¯à¤O½Õ¾ã¤¤¡I");
	vmsg("®¥ÁH±z¡A§Aªº¤pÂû¤S´_¬¡Åo¡I");
	pip_write_file();
	return 0;
}

/*---------------------------------------------------------------------------*/
/* ¤pÂû¹Ï§Î°Ï                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

void
show_basic_pic(int i)
{
	char buf[256];
	clrchyiuan(6, 18);
	sprintf(buf, BBSHOME"/game/pipgame/basic/pic%d", i);
	show_file(buf, 6, 12, ONLY_COLOR);
}

void
show_feed_pic(int i)  /*¦YªF¦è*/
{
	char buf[256];
	clrchyiuan(6, 18);
	sprintf(buf, BBSHOME"/game/pipgame/feed/pic%d", i);
	show_file(buf, 6, 12, ONLY_COLOR);
}

void
show_buy_pic(int i)  /*ÁÊ¶RªF¦è*/
{
	char buf[256];
	clrchyiuan(6, 18);
	sprintf(buf, BBSHOME"/game/pipgame/buy/pic%d", i);
	show_file(buf, 6, 12, ONLY_COLOR);
}

void
show_usual_pic(int i)  /* ¥­±`ª¬ºA */
{
	char buf[256];
	clrchyiuan(6, 18);
	sprintf(buf, BBSHOME"/game/pipgame/usual/pic%d", i);
	show_file(buf, 6, 12, ONLY_COLOR);

}

void
show_special_pic(int i)  /* ¯S®í¿ï³æ */
{
	char buf[256];
	clrchyiuan(6, 18);
	sprintf(buf, BBSHOME"/game/pipgame/special/pic%d", i);
	show_file(buf, 6, 12, ONLY_COLOR);

}

void
show_practice_pic(int i)  /*­×¦æ¥Îªº¹Ï */
{
	char buf[256];
	clrchyiuan(6, 18);
	sprintf(buf, BBSHOME"/game/pipgame/practice/pic%d", i);
	show_file(buf, 6, 12, ONLY_COLOR);
}

void
show_job_pic(int i)    /* ¥´¤uªºshow¹Ï */
{
	char buf[256];
	clrchyiuan(6, 18);
	sprintf(buf, BBSHOME"/game/pipgame/job/pic%d", i);
	show_file(buf, 6, 12, ONLY_COLOR);

}


void
show_play_pic(int i)  /*¥ð¶¢ªº¹Ï*/
{
	char buf[256];
	clrchyiuan(6, 18);
	sprintf(buf, BBSHOME"/game/pipgame/play/pic%d", i);
	if (i == 0)
		show_file(buf, 2, 16, ONLY_COLOR);
	else
		show_file(buf, 6, 12, ONLY_COLOR);
}

void
show_guess_pic(int i)  /* ²q®±¥Î */
{
	char buf[256];
	clrchyiuan(6, 18);
	sprintf(buf, BBSHOME"/game/pipgame/guess/pic%d", i);
	show_file(buf, 6, 12, ONLY_COLOR);
}

void
show_weapon_pic(int i)  /* ªZ¾¹¥Î */
{
	char buf[256];
	clrchyiuan(1, 10);
	sprintf(buf, BBSHOME"/game/pipgame/weapon/pic%d", i);
	show_file(buf, 1, 10, ONLY_COLOR);
}

void
show_palace_pic(int i)  /* °Ñ¨£¤ý¦Ú¥Î */
{
	char buf[256];
	clrchyiuan(0, 13);
	sprintf(buf, BBSHOME"/game/pipgame/palace/pic%d", i);
	show_file(buf, 0, 11, ONLY_COLOR);

}

void
show_badman_pic(int i)  /* Ãa¤H */
{
	char buf[256];
	clrchyiuan(6, 18);
	sprintf(buf, BBSHOME"/game/pipgame/badman/pic%d", i);
	show_file(buf, 6, 14, ONLY_COLOR);
}

void
show_fight_pic(int i)  /* ¥´¬[ */
{
	char buf[256];
	clrchyiuan(6, 18);
	sprintf(buf, BBSHOME"/game/pipgame/fight/pic%d", i);
	show_file(buf, 6, 14, ONLY_COLOR);
}

void
show_die_pic(int i)  /*¦º¤`*/
{
	char buf[256];
	clrchyiuan(0, 23);
	sprintf(buf, BBSHOME"/game/pipgame/die/pic%d", i);
	show_file(buf, 0, 23, ONLY_COLOR);
}

void
show_system_pic(int i)  /*¨t²Î*/
{
	char buf[256];
	clrchyiuan(1, 23);
	sprintf(buf, BBSHOME"/game/pipgame/system/pic%d", i);
	show_file(buf, 4, 16, ONLY_COLOR);
}

void
show_ending_pic(int i)  /*µ²§ô*/
{
	char buf[256];
	clrchyiuan(1, 23);
	sprintf(buf, BBSHOME"/game/pipgame/ending/pic%d", i);
	show_file(buf, 4, 16, ONLY_COLOR);
}

void
show_resultshow_pic(int i)	/*¦¬Ã¬©u*/
{
	char buf[256];
	clrchyiuan(0, 24);
	sprintf(buf, BBSHOME"/game/pipgame/resultshow/pic%d", i);
	show_file(buf, 0, 24, ONLY_COLOR);
}

/*---------------------------------------------------------------------------*/
/* °Ó©±¿ï³æ:­¹ª« ¹s­¹ ¤j¸É¤Y ª±¨ã ®Ñ¥»                                       */
/*                                                                           */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* °Ó©±¿ï³æ:­¹ª« ¹s­¹ ¤j¸É¤Y ª±¨ã ®Ñ¥»                                       */
/* ¨ç¦¡®w                                                                    */
/*---------------------------------------------------------------------------*/

int pip_store_food()
{
	int num[3];
	num[0] = 2;
	num[1] = d.food;
	num[2] = d.cookie;
	pip_buy_goods_new(1, pipfoodlist, num);
	d.food = num[1];
	d.cookie = num[2];
	return 0;
}

int pip_store_medicine()
{
	int num[5];
	num[0] = 4;
	num[1] = d.bighp;
	num[2] = d.medicine;
	num[3] = d.ginseng;
	num[4] = d.snowgrass;
	pip_buy_goods_new(2, pipmedicinelist, num);
	d.bighp = num[1];
	d.medicine = num[2];
	d.ginseng = num[3];
	d.snowgrass = num[4];
	return 0;
}

int pip_store_other()
{
	int num[3];
	num[0] = 2;
	num[1] = d.playtool;
	num[2] = d.book;
	pip_buy_goods_new(3, pipotherlist, num);
	d.playtool = num[1];
	d.book = num[2];
	return 0;
}

int pip_store_weapon_head()	/*ÀY³¡ªZ¾¹*/
{
	d.weaponhead = pip_weapon_doing_menu(d.weaponhead, 0, headlist);
	return 0;
}
int pip_store_weapon_rhand()	/*¥k¤âªZ¾¹*/
{
	d.weaponrhand = pip_weapon_doing_menu(d.weaponrhand, 1, rhandlist);
	return 0;
}
int pip_store_weapon_lhand()    /*¥ª¤âªZ¾¹*/
{
	d.weaponlhand = pip_weapon_doing_menu(d.weaponlhand, 2, lhandlist);
	return 0;
}
int pip_store_weapon_body()	/*¨­ÅéªZ¾¹*/
{
	d.weaponbody = pip_weapon_doing_menu(d.weaponbody, 3, bodylist);
	return 0;
}
int pip_store_weapon_foot()     /*¨¬³¡ªZ¾¹*/
{
	d.weaponfoot = pip_weapon_doing_menu(d.weaponfoot, 4, footlist);
	return 0;
}


int
pip_buy_goods_new(mode, p, oldnum)
int mode;
int oldnum[];
struct goodsofpip *p;
{
	char *shopname[4] = {"©±¦W", "«K§Q°Ó©±", NICKNAME "ÃÄ¾Q", "©]¸Ì®Ñ§½"};
	char inbuf[256];
	char genbuf[20];
	long smoney;
	int oldmoney;
	int i, pipkey, choice;
	oldmoney = d.money;
	do
	{
		clrchyiuan(6, 18);
		move(6, 0);
		sprintf(inbuf, "[1;31m  ¢w[41;37m ½s¸¹ [0;1;31m¢w[41;37m °Ó      «~ [0;1;31m¢w¢w[41;37m ®Ä            ¯à [0;1;31m¢w¢w[41;37m »ù     ®æ [0;1;31m¢w[37;41m ¾Ö¦³¼Æ¶q [0;1;31m¢w[0m  ");
		prints(inbuf);
		for (i = 1;i <= oldnum[0];i++)
		{
			move(7 + i, 0);
			sprintf(inbuf, "     [1;35m[[37m%2d[35m]     [36m%-10s      [37m%-14s        [1;33m%-10d   [1;32m%-9d    [0m",
					p[i].num, p[i].name, p[i].msgbuy, p[i].money, oldnum[i]);
			prints(inbuf);
		}
		clrchyiuan(19, 24);
		move(b_lines, 0);
		sprintf(inbuf, "[1;44;37m  %8s¿ï³æ  [46m  [B]¶R¤Jª««~  [S]½æ¥Xª««~  [Q]¸õ¥X¡G                         [m", shopname[mode]);
		prints(inbuf);
		pipkey = vkey();
		switch (pipkey)
		{
		case 'B':
		case 'b':
			move(b_lines - 1, 1);
			sprintf(inbuf, "·Q­n¶R¤JÔ£©O? [0]©ñ±ó¶R¤J [1¡ã%d]ª««~°Ó¸¹: ", oldnum[0]);
			getdata(b_lines - 1, 1, inbuf, genbuf, 3, LCECHO, "0");
			choice = atoi(genbuf);
			if (choice >= 1 && choice <= oldnum[0])
			{
				clrchyiuan(6, 18);
				if (rand() % 2 > 0)
					show_buy_pic(p[choice].pic1);
				else
					show_buy_pic(p[choice].pic2);
				move(b_lines - 1, 0);
				clrtoeol();
				move(b_lines - 1, 1);
				smoney = 0;
				if (mode == 3)
					smoney = 1;
				else
				{
					sprintf(inbuf, "§A­n¶R¤Jª««~ [%s] ¦h¤Ö­Ó©O?(¤W­­ %d): ", p[choice].name, d.money / p[choice].money);
					getdata(b_lines - 1, 1, inbuf, genbuf, 6, DOECHO, 0);
					smoney = atoi(genbuf);
				}
				if (smoney < 0)
				{
					vmsg("©ñ±ó¶R¤J...");
				}
				else if (d.money < smoney*p[choice].money)
				{
					vmsg("§Aªº¿ú¨S¦³¨º»ò¦h³á..");
				}
				else
				{
					sprintf(inbuf, "½T©w¶R¤Jª««~ [%s] ¼Æ¶q %d ­Ó¶Ü?(©±®a½æ»ù %d) [y/N]: ", p[choice].name, smoney, smoney*p[choice].money);
					getdata(b_lines - 1, 1, inbuf, genbuf, 2, DOECHO, 0);
					if (genbuf[0] == 'y' || genbuf[0] == 'Y')
					{
						oldnum[choice] += smoney;
						d.money -= smoney * p[choice].money;
						sprintf(inbuf, "¦ÑÁóµ¹¤F§A%d­Ó%s", smoney, p[choice].name);
						vmsg(inbuf);
						vmsg(p[choice].msguse);
						if (mode == 3 && choice == 1)
						{
							d.happy += rand() % 10 + 20 * smoney;
							d.satisfy += rand() % 10 + 20 * smoney;
						}
						if (mode == 3 && choice == 2)
						{
							d.happy += (rand() % 2 + 2) * smoney;
							d.wisdom += (2 + 10 / (d.wisdom / 100 + 1)) * smoney;
							d.character += (rand() % 4 + 2) * smoney;
							d.art += (rand() % 2 + 1) * smoney;
						}
					}
					else
					{
						vmsg("©ñ±ó¶R¤J...");
					}
				}
			}
			else
			{
				sprintf(inbuf, "©ñ±ó¶R¤J.....");
				vmsg(inbuf);
			}
			break;

		case 'S':
		case 's':
			if (mode == 3)
			{
				vmsg("³o¨ÇªF¦è¤£¯à½æ³á....");
				break;
			}
			move(b_lines - 1, 1);
			sprintf(inbuf, "·Q­n½æ¥XÔ£©O? [0]©ñ±ó½æ¥X [1¡ã%d]ª««~°Ó¸¹: ", oldnum[0]);
			getdata(b_lines - 1, 1, inbuf, genbuf, 3, LCECHO, "0");
			choice = atoi(genbuf);
			if (choice >= 1 && choice <= oldnum[0])
			{
				clrchyiuan(6, 18);
				if (rand() % 2 > 0)
					show_buy_pic(p[choice].pic1);
				else
					show_buy_pic(p[choice].pic2);
				move(b_lines - 1, 0);
				clrtoeol();
				move(b_lines - 1, 1);
				smoney = 0;
				sprintf(inbuf, "§A­n½æ¥Xª««~ [%s] ¦h¤Ö­Ó©O?(¤W­­ %d): ", p[choice].name, oldnum[choice]);
				getdata(b_lines - 1, 1, inbuf, genbuf, 6, , 0);
				smoney = atoi(genbuf);
				if (smoney < 0)
				{
					vmsg("©ñ±ó½æ¥X...");
				}
				else if (smoney > oldnum[choice])
				{
					sprintf(inbuf, "§Aªº [%s] ¨S¦³¨º»ò¦h­Ó³á", p[choice].name);
					vmsg(inbuf);
				}
				else
				{
					sprintf(inbuf, "½T©w½æ¥Xª««~ [%s] ¼Æ¶q %d ­Ó¶Ü?(©±®a¶R»ù %d) [y/N]: ", p[choice].name, smoney, smoney*p[choice].money*8 / 10);
					getdata(b_lines - 1, 1, inbuf, genbuf, 2, DOECHO, 0);
					if (genbuf[0] == 'y' || genbuf[0] == 'Y')
					{
						oldnum[choice] -= smoney;
						d.money += smoney * p[choice].money * 8 / 10;
						sprintf(inbuf, "¦ÑÁó®³¨«¤F§Aªº%d­Ó%s", smoney, p[choice].name);
						vmsg(inbuf);
					}
					else
					{
						vmsg("©ñ±ó½æ¥X...");
					}
				}
			}
			else
			{
				sprintf(inbuf, "©ñ±ó½æ¥X.....");
				vmsg(inbuf);
			}
			break;
		case 'Q':
		case 'q':
			sprintf(inbuf, "ª÷¿ú¥æ©ö¦@ %d ¤¸,Â÷¶} %s ", oldmoney - d.money, shopname[mode]);
			vmsg(inbuf);
			break;
		}
	}
	while ((pipkey != 'Q') && (pipkey != 'q') && (pipkey != KEY_LEFT));
	return 0;
}

int
pip_weapon_doing_menu(variance, type, p)             /* ªZ¾¹ÁÊ¶Rµe­± */
int variance;
int type;
struct weapon *p;
{
	time_t now;
	register int n = 0;
	register char *s;
	char buf[256];
	char ans[5];
	char shortbuf[100];
	char menutitle[5][11] = {"ÀY³¡¸Ë³Æ°Ï", "¥k¤â¸Ë³Æ°Ï", "¥ª¤â¸Ë³Æ°Ï", "¨­Åé¸Ë³Æ°Ï", "¨¬³¡¸Ë³Æ°Ï"};
	int pipkey;
	char choicekey[5];
	int choice;

	do
	{
		clear();
		vs_head(menutitle[type], BoardName);
		show_weapon_pic(0);
		/*   move(10,2);
		   sprintf(buf,"[1;37m²{¤µ¯à¤O:Åé¤OMax:[36m%-5d[37m  ªk¤OMax:[36m%-5d[37m  §ðÀ»:[36m%-5d[37m  ¨¾¿m:[36m%-5d[37m  ³t«×:[36m%-5d [m",
		           d.maxhp,d.maxmp,d.attack,d.resist,d.speed);
		   prints(buf);*/
		move(11, 2);
		sprintf(buf, "[1;37;41m [NO]  [¾¹¨ã¦W]  [Åé¤O]  [ªk¤O]  [³t«×]  [§ðÀ»]  [¨¾¿m]  [³t«×]  [°â  »ù] [m");
		prints(buf);
		move(12, 2);
		sprintf(buf, " [1;31m¢w¢w[37m¥Õ¦â ¥i¥HÁÊ¶R[31m¢w¢w[32mºñ¦â ¾Ö¦³¸Ë³Æ[31m¢w¢w[33m¶À¦â ¿ú¿ú¤£°÷[31m¢w¢w[35mµµ¦â ¯à¤O¤£¨¬[31m¢w¢w[m");
		prints(buf);

		n = 0;
		while ((s = p[n].name))
		{
			move(13 + n, 2);
			if (variance != 0 && variance == (n))/*¥»¨­¦³ªº*/
			{
				sprintf(buf,
						"[1;32m (%2d)  %-10s %4d    %4d    %4d    %4d    %4d    %4d    %6d[m",
						n, p[n].name, p[n].needmaxhp, p[n].needmaxmp, p[n].needspeed,
						p[n].attack, p[n].resist, p[n].speed, p[n].cost);
			}
			else if (d.maxhp < p[n].needmaxhp || d.maxmp < p[n].needmaxmp || d.speed < p[n].needspeed)/*¯à¤O¤£¨¬*/
			{
				sprintf(buf,
						"[1;35m (%2d)  %-10s %4d    %4d    %4d    %4d    %4d    %4d    %6d[m",
						n, p[n].name, p[n].needmaxhp, p[n].needmaxmp, p[n].needspeed,
						p[n].attack, p[n].resist, p[n].speed, p[n].cost);
			}

			else if (d.money < p[n].cost) /*¿ú¤£°÷ªº*/
			{
				sprintf(buf,
						"[1;33m (%2d)  %-10s %4d    %4d    %4d    %4d    %4d    %4d    %6d[m",
						n, p[n].name, p[n].needmaxhp, p[n].needmaxmp, p[n].needspeed,
						p[n].attack, p[n].resist, p[n].speed, p[n].cost);
			}
			else
			{
				sprintf(buf,
						"[1;37m (%2d)  %-10s %4d    %4d    %4d    %4d    %4d    %4d    %6d[m",
						n, p[n].name, p[n].needmaxhp, p[n].needmaxmp, p[n].needspeed,
						p[n].attack, p[n].resist, p[n].speed, p[n].cost);
			}
			prints(buf);
			n++;
		}
		move(b_lines, 0);
		sprintf(buf, "[1;44;37m  ªZ¾¹ÁÊ¶R¿ï³æ  [46m  [B]ÁÊ¶RªZ¾¹  [S]½æ±¼¸Ë³Æ  [W]­Ó¤H¸ê®Æ  [Q]¸õ¥X¡G            [m");
		prints(buf);
		now = time(0);
		pip_time_change(now);
		pipkey = vkey();
		pip_time_change(now);

		switch (pipkey)
		{
		case 'B':
		case 'b':
			move(b_lines - 1, 1);
			sprintf(shortbuf, "·Q­nÁÊ¶RÔ£©O? §Aªº¿ú¿ú[%d]¤¸:[¼Æ¦r]: ", d.money);
			prints(shortbuf);
			getdata(b_lines - 1, 1, shortbuf, choicekey, 4, LCECHO, "0");
			choice = atoi(choicekey);
			if (choice >= 0 && choice <= n)
			{
				move(b_lines - 1, 0);
				clrtoeol();
				move(b_lines - 1, 1);
				if (choice == 0)  /*¸Ñ°£*/
				{
					sprintf(shortbuf, "©ñ±óÁÊ¶R...");
					vmsg(shortbuf);
				}

				else if (variance == choice)  /*¦­¤w¸g¦³°Õ*/
				{
					sprintf(shortbuf, "§A¦­¤w¸g¦³ %s Åo", p[variance].name);
					vmsg(shortbuf);
				}

				else if (p[choice].cost >= (d.money + p[variance].sell))  /*¿ú¤£°÷*/
				{
					sprintf(shortbuf, "³o­Ó­n %d ¤¸¡A§Aªº¿ú¤£°÷°Õ!", p[choice].cost);
					vmsg(shortbuf);
				}

				else if (d.maxhp < p[choice].needmaxhp || d.maxmp < p[choice].needmaxmp
						 || d.speed < p[choice].needspeed)  /*¯à¤O¤£¨¬*/
				{
					sprintf(shortbuf, "»Ý­nHP %d MP %d SPEED %d ³á",
							p[choice].needmaxhp, p[choice].needmaxmp, p[choice].needspeed);
					vmsg(shortbuf);
				}
				else  /*¶¶§QÁÊ¶R*/
				{
					sprintf(shortbuf, "§A½T©w­nÁÊ¶R %s ¶Ü?($%d) [y/N]: ", p[choice].name, p[choice].cost);
					getdata(b_lines - 1, 1, shortbuf, ans, 2, DOECHO, 0);
					if (ans[0] == 'y' || ans[0] == 'Y')
					{
						sprintf(shortbuf, "¤pÂû¤w¸g¸Ë°t¤W %s ¤F", p[choice].name);
						vmsg(shortbuf);
						d.attack += (p[choice].attack - p[variance].attack);
						d.resist += (p[choice].resist - p[variance].resist);
						d.speed += (p[choice].speed - p[variance].speed);
						d.money -= (p[choice].cost - p[variance].sell);
						variance = choice;
					}
					else
					{
						sprintf(shortbuf, "©ñ±óÁÊ¶R.....");
						vmsg(shortbuf);
					}
				}
			}
			break;

		case 'S':
		case 's':
			if (variance != 0)
			{
				sprintf(shortbuf, "§A½T©w­n½æ±¼%s¶Ü? ½æ»ù:%d [y/N]: ", p[variance].name, p[variance].sell);
				getdata(b_lines - 1, 1, shortbuf, ans, 2, DOECHO, 0);
				if (ans[0] == 'y' || ans[0] == 'Y')
				{
					sprintf(shortbuf, "¸Ë³Æ %s ½æ¤F %d", p[variance].name, p[variance].sell);
					d.attack -= p[variance].attack;
					d.resist -= p[variance].resist;
					d.speed -= p[variance].speed;
					d.money += p[variance].sell;
					vmsg(shortbuf);
					variance = 0;
				}
				else
				{
					sprintf(shortbuf, "ccc..§Ú¦^¤ßÂà·N¤F...");
					vmsg(shortbuf);
				}
			}
			else if (variance == 0)
			{
				sprintf(shortbuf, "§A¥»¨Ó´N¨S¦³¸Ë³Æ¤F...");
				vmsg(shortbuf);
				variance = 0;
			}
			break;

		case 'W':
		case 'w':
			pip_data_list(cuser.userid);
			break;
		}
	}
	while ((pipkey != 'Q') && (pipkey != 'q') && (pipkey != KEY_LEFT));

	return variance;
}

/*---------------------------------------------------------------------------*/
/* ¥´¤u¿ï³æ:®a¨Æ ­W¤u ®a±Ð ¦aÅu                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

int pip_job_workA()
{
	/*  ¢u¢w¢w¢w¢w¢q¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	/*  ¢x®a®xºÞ²z¢x«Ý¤H±µª« + N , ±½¦a¬~¦ç + N , ²i¶¹§Þ¥© + N  ¢x*/
	/*  ¢x        ¢x©M¤÷¿ËªºÃö«Y + N , ¯h³Ò + 1 , ·P¨ü - 2      ¢x*/
	/*  ¢u¢w¢w¢w¢w¢q¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	/*  ¢x®a®xºÞ²z¢x­Y Åé    ¤O - RND (¯h³Ò) >=   5 «h¤u§@¦¨¥\  ¢x*/
	/*  ¢u¢w¢w¢w¢w¢q¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	float class;
	long workmoney;

	workmoney = 0;
	class = ((d.hp * 100 / d.maxhp) - d.tired) * LEARN_ELVEL;
	d.maxhp += rand() % 2 * LEARN_ELVEL;
	d.shit += rand() % 3 + 5;
	count_tired(3, 7, "Y", 100, 1);
	d.hp -= (rand() % 2 + 4);
	d.happy -= (rand() % 3 + 4);
	d.satisfy -= rand() % 3 + 4;
	d.affect -= 7 + rand() % 7;
	if (d.affect <= 0)
		d.affect = 0;
	show_job_pic(11);
	if (class >= 75)
	{
		d.cookskill += rand() % 2 + 7;
		d.homework += rand() % 2 + 7;
		d.family += rand() % 3 + 4;
		d.relation += rand() % 3 + 4;
		workmoney = 80 + (d.cookskill * 2 + d.homework + d.family) / 40;
		vmsg("®a¨Æ«Ü¦¨¥\\³á..¦h¤@ÂI¿úµ¹§A..");
	}
	else if (class < 75 && class >= 50)
	{
		d.cookskill += rand() % 2 + 5;
		d.homework += rand() % 2 + 5;
		d.family += rand() % 3 + 3;
		d.relation += rand() % 3 + 3;
		workmoney = 60 + (d.cookskill * 2 + d.homework + d.family) / 45;
		vmsg("®a¨ÆÁÙÆZ¶¶§Qªº­ò..¶â¶â..");
	}
	else if (class < 50 && class >= 25)
	{
		d.cookskill += rand() % 3 + 3;
		d.homework += rand() % 3 + 3;
		d.family += rand() % 3 + 2;
		d.relation += rand() % 3 + 2;
		workmoney = 40 + (d.cookskill * 2 + d.homework + d.family) / 50;
		vmsg("®a¨Æ´¶´¶³q³q°Õ..¥i¥H§ó¦nªº..¥[ªo..");
	}
	else if (class < 25)
	{
		d.cookskill += rand() % 3 + 1;
		d.homework += rand() % 3 + 1;
		d.family += rand() % 3 + 1;
		d.relation += rand() % 3 + 1;
		workmoney = 20 + (d.cookskill * 2 + d.homework + d.family) / 60;
		vmsg("®a¨Æ«ÜÁV¿|³á..³o¼Ë¤£¦æ°Õ..");
	}
	d.money += workmoney * LEARN_ELVEL;
	d.workA += 1;
	return 0;
}

int pip_job_workB()
{
	/*  ¢u¢w¢w¢w¢w¢q¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	/*  ¢x¨|¥®°|  ¢x¥À©Ê + N , ·P¨ü + 1 , ¾y¤O - 1 , ¯h³Ò + 3   ¢x*/
	/*  ¢u¢w¢w¢w¢w¢q¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	/*  ¢x¨|¥®°|  ¢x­Y Åé    ¤O - RND (¯h³Ò) >=  20 «h¤u§@¦¨¥\  ¢x*/
	/*  ¢u¢w¢w¢w¢w¢q¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	float class;
	long workmoney;

	workmoney = 0;
	class = ((d.hp * 100 / d.maxhp) - d.tired) * LEARN_ELVEL;
	d.maxhp += (rand() % 2 + 1) * LEARN_ELVEL;
	d.shit += rand() % 3 + 5;
	d.affect += rand() % 3 + 4;

	count_tired(3, 9, "Y", 100, 1);
	d.hp -= (rand() % 3 + 6);
	d.happy -= (rand() % 3 + 4);
	d.satisfy -= rand() % 3 + 4;
	d.charm -= rand() % 3 + 4;
	if (d.charm <= 0)
		d.charm = 0;
	show_job_pic(21);
	if (class >= 90)
	{
		d.love += rand() % 2 + 7;
		d.toman += rand() % 2 + 2;
		workmoney = 150 + (d.love + d.toman) / 50;
		vmsg("·í«O©i«Ü¦¨¥\\³á..¤U¦¸¦A¨Ó³á..");
	}
	else if (class < 90 && class >= 75)
	{
		d.love += rand() % 2 + 5;
		d.toman += rand() % 2 + 2;
		workmoney = 120 + (d.love + d.toman) / 50;
		vmsg("«O©iÁÙ·íªº¤£¿ù­ò..¶â¶â..");
	}
	else if (class < 75 && class >= 50)
	{
		d.love += rand() % 2 + 3;
		d.toman += rand() % 2 + 1;
		workmoney = 100 + (d.love + d.toman) / 50;
		vmsg("¤pªB¤Í«Ü¥Ö³á..¥[ªo..");
	}
	else if (class < 50)
	{
		d.love += rand() % 2 + 1;
		d.toman += rand() % 2 + 1;
		workmoney = 80 + (d.love + d.toman) / 50;
		vmsg("«ÜÁV¿|³á..§A¸n¤£¦í¤pªB¤Í­C...");
	}
	d.money += workmoney * LEARN_ELVEL;
	d.workB += 1;
	return 0;
}

int pip_job_workC()
{
	/*  ¢u¢w¢w¢w¢w¢q¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	/*  ¢x®ÈÀ]    ¢x±½¦a¬~¦ç + N , ¾Ô°«§Þ³N - N , ¯h³Ò + 2      ¢x*/
	/*  ¢u¢w¢w¢w¢w¢q¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	/*  ¢x®ÈÀ]    ¢x­Y Åé    ¤O - RND (¯h³Ò) >=  30 «h¤u§@¦¨¥\  ¢x*/
	/*  ¢u¢w¢w¢w¢w¢q¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	float class;
	long workmoney;

	workmoney = 0;
	class = ((d.hp * 100 / d.maxhp) - d.tired) * LEARN_ELVEL;
	d.maxhp += (rand() % 2 + 2) * LEARN_ELVEL;
	d.shit += rand() % 3 + 5;
	count_tired(5, 12, "Y", 100, 1);
	d.hp -= (rand() % 4 + 8);
	d.happy -= (rand() % 3 + 4);
	d.satisfy -= rand() % 3 + 4;
	show_job_pic(31);
	if (class >= 95)
	{
		d.homework += rand() % 2 + 7;
		d.family += rand() % 2 + 4;
		d.hskill -= rand() % 2 + 7;
		if (d.hskill < 0)
			d.hskill = 0;
		workmoney = 250 + (d.cookskill * 2 + d.homework * 2) / 40;
		vmsg("®ÈÀ]¨Æ·~»]»]¤é¤W..§Æ±æ§A¦A¹L¨Ó...");
	}
	else if (class < 95 && class >= 80)
	{
		d.homework += rand() % 2 + 5;
		d.family += rand() % 2 + 3;
		d.hskill -= rand() % 2 + 5;
		if (d.hskill < 0)
			d.hskill = 0;
		workmoney = 200 + (d.cookskill * 2 + d.homework * 2) / 50;
		vmsg("®ÈÀ]ÁÙÆZ¶¶§Qªº­ò..¶â¶â..");
	}
	else if (class < 80 && class >= 60)
	{
		d.homework += rand() % 2 + 3;
		d.family += rand() % 2 + 3;
		d.hskill -= rand() % 2 + 5;
		if (d.hskill < 0)
			d.hskill = 0;
		workmoney = 150 + (d.cookskill * 2 + d.homework * 2) / 50;
		vmsg("´¶´¶³q³q°Õ..¥i¥H§ó¦nªº..¥[ªo..");
	}
	else if (class < 60)
	{
		d.homework += rand() % 2 + 1;
		d.family += rand() % 2 + 1;
		d.hskill -= rand() % 2 + 1;
		if (d.hskill < 0)
			d.hskill = 0;
		workmoney = 100 + (d.cookskill * 2 + d.homework * 2) / 50;
		vmsg("³o­Ó«ÜÁV¿|³á..§A³o¼Ë¤£¦æ°Õ..");
	}
	d.money += workmoney * LEARN_ELVEL;
	d.workC += 1;
	return 0;
}

int pip_job_workD()
{
	/*  ¢u¢w¢w¢w¢w¢q¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	/*  ¢x¹A³õ    ¢xÅé¤O + 1 , µÃ¤O + 1 , ®ð½è - 1 , ¯h³Ò + 3   ¢x*/
	/*  ¢u¢w¢w¢w¢w¢q¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	/*  ¢x¹A³õ    ¢x­Y Åé    ¤O - RND (¯h³Ò) >=  30 «h¤u§@¦¨¥\  ¢x*/
	/*  ¢u¢w¢w¢w¢w¢q¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	float class;
	long workmoney;

	workmoney = 0;
	class = ((d.hp * 100 / d.maxhp) - d.tired) * LEARN_ELVEL;
	d.maxhp += (rand() % 3 + 2) * LEARN_ELVEL;
	d.wrist += rand() % 2 + 2;
	d.shit += rand() % 5 + 10;
	count_tired(5, 15, "Y", 100, 1);
	d.hp -= (rand() % 4 + 10);
	d.happy -= (rand() % 3 + 4);
	d.satisfy -= rand() % 3 + 4;
	d.character -= rand() % 3 + 4;
	if (d.character < 0)
		d.character = 0;
	show_job_pic(41);
	if (class >= 95)
	{
		workmoney = 250 + (d.wrist * 2 + d.hp * 2) / 80;
		vmsg("¤û¦Ïªøªº¦n¦n³á..§Æ±æ§A¦A¨ÓÀ°¦£...");
	}
	else if (class < 95 && class >= 80)
	{
		workmoney = 210 + (d.wrist * 2 + d.hp * 2) / 80;
		vmsg("¨þ¨þ..ÁÙ¤£¿ù³á..:)");
	}
	else if (class < 80 && class >= 60)
	{
		workmoney = 160 + (d.wrist * 2 + d.hp * 2) / 80;
		vmsg("´¶´¶³q³q°Õ..¥i¥H§ó¦nªº..");
	}
	else if (class < 60)
	{
		workmoney = 120 + (d.wrist * 2 + d.hp * 2) / 80;
		vmsg("§A¤£¤Ó¾A¦X¹A³õªº¤u§@  -_-...");
	}
	d.money += workmoney * LEARN_ELVEL;
	d.workD += 1;
	return 0;
}

int pip_job_workE()
{
	/*  ¢u¢w¢w¢w¢w¢q¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	/*  ¢xÀ\ÆU    ¢x®Æ²z + N , ¾Ô°«§Þ³N - N , ¯h³Ò + 2          ¢x*/
	/*  ¢u¢w¢w¢w¢w¢q¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	/*  ¢xÀ\ÆU    ¢x­Y ²i¶¹§Þ³N - RND (¯h³Ò) >=  50 «h¤u§@¦¨¥\  ¢x*/
	/*  ¢u¢w¢w¢w¢w¢q¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	float class;
	long workmoney;

	workmoney = 0;
	class = (d.cookskill - d.tired) * LEARN_ELVEL;
	d.maxhp += (rand() % 2 + 1) * LEARN_ELVEL;
	d.shit += rand() % 4 + 12;
	count_tired(5, 9, "Y", 100, 1);
	d.hp -= (rand() % 4 + 8);
	d.happy -= (rand() % 3 + 4);
	d.satisfy -= rand() % 3 + 4;
	show_job_pic(51);
	if (class >= 80)
	{
		d.homework += rand() % 2 + 1;
		d.family += rand() % 2 + 1;
		d.hskill -= rand() % 2 + 5;
		if (d.hskill < 0)
			d.hskill = 0;
		d.cookskill += rand() % 2 + 6;
		workmoney = 250 + (d.cookskill * 2 + d.homework * 2 + d.family * 2) / 80;
		vmsg("«È¤H³£»¡¤Ó¦n¦Y¤F..¦A¨Ó¤@½L§a...");
	}
	else if (class < 80 && class >= 60)
	{
		d.homework += rand() % 2 + 1;
		d.family += rand() % 2 + 1;
		d.hskill -= rand() % 2 + 5;
		if (d.hskill < 0)
			d.hskill = 0;
		d.cookskill += rand() % 2 + 4;
		workmoney = 200 + (d.cookskill * 2 + d.homework * 2 + d.family * 2) / 80;
		vmsg("µNªºÁÙ¤£¿ù¦Y­ò..:)");
	}
	else if (class < 60 && class >= 30)
	{
		d.homework += rand() % 2 + 1;
		d.family += rand() % 2 + 1;
		d.hskill -= rand() % 2 + 5;
		if (d.hskill < 0)
			d.hskill = 0;
		d.cookskill += rand() % 2 + 2;
		workmoney = 150 + (d.cookskill * 2 + d.homework * 2 + d.family * 2) / 80;
		vmsg("´¶´¶³q³q°Õ..¥i¥H§ó¦nªº..");
	}
	else if (class < 30)
	{
		d.homework += rand() % 2 + 1;
		d.family += rand() % 2 + 1;
		d.hskill -= rand() % 2 + 5;
		if (d.hskill < 0)
			d.hskill = 0;
		d.cookskill += rand() % 2 + 1;
		workmoney = 100 + (d.cookskill * 2 + d.homework * 2 + d.family * 2) / 80;
		vmsg("§Aªº¼pÃÀ«Ý¥[±j³á...");
	}
	d.money += workmoney * LEARN_ELVEL;
	d.workE += 1;
	return 0;
}

int pip_job_workF()
{
	/*  ¢u¢w¢w¢w¢w¢q¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	/*  ¢x±Ð°ó    ¢x«H¥õ + 2 , ¹D¼w + 1 , ¸oÄ^ - 2 , ¯h³Ò + 1   ¢x*/
	/*  ¢u¢w¢w¢w¢w¢q¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	float class;
	long workmoney;

	workmoney = 0;
	class = ((d.hp * 100 / d.maxhp) - d.tired) * LEARN_ELVEL;
	count_tired(5, 7, "Y", 100, 1);
	d.love += (rand() % 3 + 4) * LEARN_ELVEL;
	d.belief += (rand() % 4 + 7) * LEARN_ELVEL;
	d.etchics += (rand() % 3 + 7) * LEARN_ELVEL;
	d.shit += rand() % 3 + 3;
	d.hp -= rand() % 3 + 5;
	d.offense -= rand() % 4 + 7;
	if (d.offense < 0)
		d.offense = 0;
	show_job_pic(61);
	if (class >= 75)
	{
		workmoney = 100 + (d.belief + d.etchics - d.offense) / 20;
		vmsg("¿ú«Ü¤Ö ¦ý¬Ý§A³o»ò»{¯u µ¹§A¦h¤@ÂI...");
	}
	else if (class < 75 && class >= 50)
	{
		workmoney = 75 + (d.belief + d.etchics - d.offense) / 20;
		vmsg("ÁÂÁÂ§Aªº¼ö¤ßÀ°¦£..:)");
	}
	else if (class < 50 && class >= 25)
	{
		workmoney = 50 + (d.belief + d.etchics - d.offense) / 20;
		vmsg("§A¯uªº«Ü¦³·R¤ß°Õ..¤£¹L¦³ÂI¤p²Öªº¼Ë¤l...");
	}
	else if (class < 25)
	{
		workmoney = 25 + (d.belief + d.etchics - d.offense) / 20;
		vmsg("¨Ó©^Äm¤£¿ù..¦ý¤]¤£¯à¥´²V£«....:(");
	}
	d.money += workmoney * LEARN_ELVEL;
	d.workF += 1;
	return 0;
}

int pip_job_workG()
{
	/* ¢u¢w¢w¢w¢w¢q¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	/* ¢x¦aÅu    ¢xÅé¤O + 2 , ¾y¤O + 1 , ¯h³Ò + 3 ,½Í¦R +1     ¢x*/
	/* ¢u¢w¢w¢w¢w¢q¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	long workmoney;

	workmoney = 0;
	workmoney = 200 + (d.charm * 3 + d.speech * 2 + d.toman) / 50;
	count_tired(3, 12, "Y", 100, 1);
	d.shit += rand() % 3 + 8;
	d.speed += (rand() % 2) * LEARN_ELVEL;
	d.weight -= rand() % 2;
	d.happy -= (rand() % 3 + 7);
	d.satisfy -= rand() % 3 + 5;
	d.hp -= (rand() % 6 + 6);
	d.charm += (rand() % 2 + 3) * LEARN_ELVEL;
	d.speech += (rand() % 2 + 3) * LEARN_ELVEL;
	d.toman += (rand() % 2 + 3) * LEARN_ELVEL;
	move(4, 0);
	show_job_pic(71);
	vmsg("Â\\¦aÅu­n¸úÄµ¹î°Õ..:p");
	d.money += workmoney * LEARN_ELVEL;
	d.workG += 1;
	return 0;
}

int pip_job_workH()
{
	/*  ¢u¢w¢w¢w¢w¢q¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	/*  ¢x¥ï¤ì³õ  ¢xµÃ¤O + 2 , ®ð½è - 2 , ¯h³Ò + 4              ¢x*/
	/*  ¢u¢w¢w¢w¢w¢q¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	/*  ¢x¥ï¤ì³õ  ¢x­Y µÃ    ¤O - RND (¯h³Ò) >=  80 «h¤u§@¦¨¥\  ¢x*/
	/*  ¢u¢w¢w¢w¢w¢q¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	float class;
	long workmoney;

	if ((d.bbtime / 60 / 30) < 1) /*¤@·³¤~¦æ*/
	{
		vmsg("¤pÂû¤Ó¤p¤F,¤@·³¥H«á¦A¨Ó§a...");
		return 0;
	}
	workmoney = 0;
	class = (d.wrist - d.tired) * LEARN_ELVEL;
	d.maxhp += (rand() % 2 + 3) * LEARN_ELVEL;
	d.shit += rand() % 7 + 15;
	d.wrist += (rand() % 3 + 4) * LEARN_ELVEL;
	count_tired(5, 15, "Y", 100, 1);
	d.hp -= (rand() % 4 + 10);
	d.happy -= (rand() % 3 + 4);
	d.satisfy -= rand() % 3 + 4;
	d.character -= rand() % 3 + 7;
	if (d.character < 0)
		d.character = 0;
	show_job_pic(81);
	if (class >= 70)
	{
		workmoney = 350 + d.wrist / 20 + d.maxhp / 80;
		vmsg("§AµÃ¤O«Ü¦n­ò..:)");
	}
	else if (class < 70 && class >= 50)
	{
		workmoney = 300 + d.wrist / 20 + d.maxhp / 80;
		vmsg("¬å¤F¤£¤Ö¾ð³á.....:)");
	}
	else if (class < 50 && class >= 20)
	{
		workmoney = 250 + d.wrist / 20 + d.maxhp / 80;
		vmsg("´¶´¶³q³q°Õ..¥i¥H§ó¦nªº..");
	}
	else if (class < 20)
	{
		workmoney = 200 + d.wrist / 20 + d.maxhp / 80;
		vmsg("«Ý¥[±j³á..ÁëÁå¦A¨Ó§a....");
	}
	d.money += workmoney * LEARN_ELVEL;
	d.workH += 1;
	return 0;
}

int pip_job_workI()
{
	/*  ¢u¢w¢w¢w¢w¢q¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	/*  ¢x¬ü®e°|  ¢x·P¨ü + 1 , µÃ¤O - 1 , ¯h³Ò + 3              ¢x*/
	/*  ¢u¢w¢w¢w¢w¢q¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	/*  ¢x¬ü®e°|  ¢x­Y ÃÀ³N­×¾i - RND (¯h³Ò) >=  40 «h¤u§@¦¨¥\  ¢x*/
	/*  ¢u¢w¢w¢w¢w¢q¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	float class;
	long workmoney;

	if ((d.bbtime / 60 / 30) < 1) /*¤@·³¤~¦æ*/
	{
		vmsg("¤pÂû¤Ó¤p¤F,¤@·³¥H«á¦A¨Ó§a...");
		return 0;
	}
	workmoney = 0;
	class = (d.art - d.tired) * LEARN_ELVEL;
	d.maxhp += (rand() % 2) * LEARN_ELVEL;
	d.affect += (rand() % 2 + 3) * LEARN_ELVEL;
	count_tired(3, 11, "Y", 100, 1);
	d.shit += rand() % 4 + 8;
	d.hp -= (rand() % 4 + 10);
	d.happy -= (rand() % 3 + 4);
	d.satisfy -= rand() % 3 + 4;
	d.wrist -= rand() % + 3;
	if (d.wrist < 0)
		d.wrist = 0;
	/*show_job_pic(4);*/
	if (class >= 80)
	{
		workmoney = 400 + d.art / 10 + d.affect / 20;
		vmsg("«È¤H³£«Ü³ßÅwÅý§A°µ³y«¬­ò..:)");
	}
	else if (class < 80 && class >= 60)
	{
		workmoney = 360 + d.art / 10 + d.affect / 20;
		vmsg("°µªº¤£¿ù³á..»á¦³¤Ñ¥÷...:)");
	}
	else if (class < 60 && class >= 40)
	{
		workmoney = 320 + d.art / 10 + d.affect / 20;
		vmsg("°¨°¨ªêªê°Õ..¦A¥[ªo¤@ÂI..");
	}
	else if (class < 40)
	{
		workmoney = 250 + d.art / 10 + d.affect / 20;
		vmsg("«Ý¥[±j³á..¥H«á¦A¨Ó§a....");
	}
	d.money += workmoney * LEARN_ELVEL;
	d.workI += 1;
	return 0;
}

int pip_job_workJ()
{
	/*  ¢u¢w¢w¢w¢w¢q¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	/*  ¢x¬¼Ây°Ï  ¢xÅé¤O + 1 , ®ð½è - 1 , ¥À©Ê - 1 , ¯h³Ò + 3   ¢x*/
	/*  ¢x        ¢x¾Ô°«§Þ³N + N                                ¢x*/
	/*  ¢u¢w¢w¢w¢w¢q¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	/*  ¢x¬¼Ây°Ï  ¢x­Y Åé    ¤O - RND (¯h³Ò) >=  80 ¡®          ¢x*/
	/*  ¢x        ¢x­Y ´¼    ¤O - RND (¯h³Ò) >=  40 «h¤u§@¦¨¥\  ¢x*/
	/*  ¢u¢w¢w¢w¢w¢q¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	float class;
	float class1;
	long workmoney;

	/*¨â·³¥H¤W¤~¦æ*/
	if ((d.bbtime / 60 / 30) < 2)
	{
		vmsg("¤pÂû¤Ó¤p¤F,¨â·³¥H«á¦A¨Ó§a...");
		return 0;
	}
	workmoney = 0;
	class = ((d.hp * 100 / d.maxhp) - d.tired) * LEARN_ELVEL;
	class1 = (d.wisdom - d.tired) * LEARN_ELVEL;
	count_tired(5, 15, "Y", 100, 1);
	d.shit += rand() % 4 + 13;
	d.weight -= (rand() % 2 + 1);
	d.maxhp += (rand() % 2 + 3) * LEARN_ELVEL;
	d.speed += (rand() % 2 + 3) * LEARN_ELVEL;
	d.hp -= (rand() % 6 + 8);
	d.character -= rand() % 3 + 4;
	d.happy -= rand() % 5 + 8;
	d.satisfy -= rand() % 5 + 6;
	d.love -= rand() % 3 + 4;
	if (d.character < 0)
		d.character = 0;
	if (d.love < 0)
		d.love = 0;
	move(4, 0);
	show_job_pic(101);
	if (class >= 80 && class1 >= 80)
	{
		d.hskill += rand() % 2 + 7;
		workmoney = 300 + d.maxhp / 50 + d.hskill / 20;
		vmsg("§A¬O§¹¬üªºÂy¤H..");
	}
	else if ((class < 75 && class >= 50) && class1 >= 60)
	{
		d.hskill += rand() % 2 + 5;
		workmoney = 270 + d.maxhp / 45 + d.hskill / 20;
		vmsg("¦¬ÀòÁÙ¤£¿ù³á..¥i¥H¹¡À\\¤@¹y¤F..:)");
	}
	else if ((class < 50 && class >= 25) && class1 >= 40)
	{
		d.hskill += rand() % 2 + 3;
		workmoney = 240 + d.maxhp / 40 + d.hskill / 20;
		vmsg("§Þ³N®t±j¤H·N  ¦A¥[ªo³á..");
	}
	else if ((class < 25 && class >= 0) && class1 >= 20)
	{
		d.hskill += rand() % 2 + 1;
		workmoney = 210 + d.maxhp / 30 + d.hskill / 20;
		vmsg("¬¼Ây¬OÅé¤O»P´¼¤Oªºµ²¦X....");
	}
	else if (class < 0)
	{
		d.hskill += rand() % 2;
		workmoney = 190 + d.hskill / 20;
		vmsg("­n¦h¦hÁëÁå©M¼W¶i´¼¼z°Õ....");
	}
	d.money += workmoney * LEARN_ELVEL;
	d.workJ += 1;
	return 0;
}

int pip_job_workK()
{
	/* ¢u¢w¢w¢w¢w¢q¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	/* ¢x¤u¦a    ¢xÅé¤O + 2 , ¾y¤O - 1 , ¯h³Ò + 3              ¢x*/
	/* ¢u¢w¢w¢w¢w¢q¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	float class;
	long workmoney;

	/*¨â·³¥H¤W¤~¦æ*/
	if ((d.bbtime / 60 / 30) < 2)
	{
		vmsg("¤pÂû¤Ó¤p¤F,¨â·³¥H«á¦A¨Ó§a...");
		return 0;
	}
	workmoney = 0;
	class = ((d.hp * 100 / d.maxhp) - d.tired) * LEARN_ELVEL;
	count_tired(5, 15, "Y", 100, 1);
	d.shit += rand() % 4 + 16;
	d.weight -= (rand() % 2 + 2);
	d.maxhp += (rand() % 2 + 1) * LEARN_ELVEL;
	d.speed += (rand() % 2 + 2) * LEARN_ELVEL;
	d.hp -= (rand() % 6 + 10);
	d.charm -= rand() % 3 + 6;
	d.happy -= (rand() % 5 + 10);
	d.satisfy -= rand() % 5 + 6;
	if (d.charm < 0)
		d.charm = 0;
	move(4, 0);
	show_job_pic(111);
	if (class >= 75)
	{
		workmoney = 250 + d.maxhp / 50;
		vmsg("¤uµ{«Ü§¹¬ü  ÁÂÁÂ§A¤F..");
	}
	else if (class < 75 && class >= 50)
	{
		workmoney = 220 + d.maxhp / 45;
		vmsg("¤uµ{©|ºÙ¶¶§Q  ¨¯­W§A¤F..");
	}
	else if (class < 50 && class >= 25)
	{
		workmoney = 200 + d.maxhp / 40;
		vmsg("¤uµ{®t±j¤H·N  ¦A¥[ªo³á..");
	}
	else if (class < 25 && class >= 0)
	{
		workmoney = 180 + d.maxhp / 30;
		vmsg("£­  «Ý¥[±j«Ý¥[±j....");
	}
	else
	{
		workmoney = 160;
		vmsg("¤U¦¸Åé¤O¦n¤@ÂI..¯h³Ò«×§C¤@ÂI¦A¨Ó....");
	}

	d.money += workmoney * LEARN_ELVEL;
	d.workK += 1;
	return 0;
}

int pip_job_workL()
{
	/*  ¢u¢w¢w¢w¢w¢q¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	/*  ¢x¹Ó¶é    ¢x§ÜÅ]¯à¤O + N , ·P¨ü + 1 , ¾y¤O - 1          ¢x*/
	/*  ¢x        ¢x¯h³Ò + 2                                    ¢x*/
	/*  ¢u¢w¢w¢w¢w¢q¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	float class;
	float class1;
	long workmoney;

	/*¤T·³¤~¦æ*/
	if ((d.bbtime / 60 / 30) < 3)
	{
		vmsg("¤pÂû²{¦bÁÙ¤Ó¤p¤F,¤T·³¥H«á¦A¨Ó§a...");
		return 0;
	}
	workmoney = 0;
	class = ((d.hp * 100 / d.maxhp) - d.tired) * LEARN_ELVEL;
	class1 = (d.belief - d.tired) * LEARN_ELVEL;
	d.shit += rand() % 5 + 8;
	d.maxmp += (rand() % 2) * LEARN_ELVEL;
	d.affect += (rand() % 2 + 2) * LEARN_ELVEL;
	d.brave += (rand() % 2 + 2) * LEARN_ELVEL;
	count_tired(5, 12, "Y", 100, 1);
	d.hp -= (rand() % 3 + 7);
	d.happy -= (rand() % 4 + 6);
	d.satisfy -= rand() % 3 + 5;
	d.charm -= rand() % 3 + 6;
	if (d.charm < 0)
		d.charm = 0;
	show_job_pic(121);
	if (class >= 75 && class1 >= 75)
	{
		d.mresist += rand() % 2 + 7;
		workmoney = 200 + (d.affect + d.brave) / 40;
		vmsg("¦u¹Ó¦¨¥\\³á  µ¹§A¦hÂI¿ú");
	}
	else if ((class < 75 && class >= 50) && class1 >= 50)
	{
		d.mresist += rand() % 2 + 5;
		workmoney = 150 + (d.affect + d.brave) / 50;
		vmsg("¦u¹ÓÁÙºâ¦¨¥\\³á..ÁÂ°Õ..");
	}
	else if ((class < 50 && class >= 25) && class1 >= 25)
	{
		d.mresist += rand() % 2 + 3;
		workmoney = 120 + (d.affect + d.brave) / 60;
		vmsg("¦u¹ÓÁÙºâ®t±j¤H·N³á..¥[ªo..");
	}
	else
	{
		d.mresist += rand() % 2 + 1;
		workmoney = 80 + (d.affect + d.brave) / 70;
		vmsg("§Ú¤]¤£¤è«K»¡Ô£¤F..½Ð¦A¥[ªo..");
	}
#if 0
	if (rand() % 10 == 5)
	{
		vmsg("¯u¬O­Ë·°  ³º¹J¨ì¦º¯«Å]..");
		pip_fight_bad(12);
	}
#endif
	d.money += workmoney * LEARN_ELVEL;
	d.workL += 1;
	return 0;
}

int pip_job_workM()
{
	/*  ¢u¢w¢w¢w¢w¢q¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	/*  ¢x®a®x±Ð®v¢x¹D¼w + 1 , ¥À©Ê + N , ¾y¤O - 1 , ¯h³Ò + 7   ¢x*/
	/*  ¢u¢w¢w¢w¢w¢q¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	float class;
	long workmoney;

	if ((d.bbtime / 60 / 30) < 4)
	{
		vmsg("¤pÂû¤Ó¤p¤F,¥|·³¥H«á¦A¨Ó§a...");
		return 0;
	}
	workmoney = 0;
	class = ((d.hp * 100 / d.maxhp) - d.tired) * LEARN_ELVEL;
	workmoney = 50 + d.wisdom / 20 + d.character / 20;
	count_tired(5, 10, "Y", 100, 1);
	d.shit += rand() % 3 + 8;
	d.character += (rand() % 2) * LEARN_ELVEL;
	d.wisdom += (rand() % 2) * LEARN_ELVEL;
	d.happy -= (rand() % 3 + 6);
	d.satisfy -= rand() % 3 + 5;
	d.hp -= (rand() % 3 + 8);
	d.money += workmoney * LEARN_ELVEL;
	move(4, 0);
	show_job_pic(131);
	vmsg("®a±Ð»´ÃP ·íµM¿ú´N¤Ö¤@ÂIÅo");
	d.workM += 1;
	return 0;
}

int pip_job_workN()
{
	/*  ¢u¢w¢w¢w¢w¢q¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	/*  ¢x°s©±    ¢x²i¶¹§Þ¥© + N , ½Í¸Ü§Þ¥© + N , ´¼¤O - 2      ¢x*/
	/*  ¢x        ¢x¯h³Ò + 5                                    ¢x*/
	/*  ¢u¢w¢w¢w¢w¢q¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	/*  ¢x°s©±    ¢x­Y Åé    ¤O - RND (¯h³Ò) >=  60 ¡®          ¢x*/
	/*  ¢x        ¢x­Y ¾y    ¤O - RND (¯h³Ò) >=  50 «h¤u§@¦¨¥\  ¢x*/
	/*  ¢u¢w¢w¢w¢w¢q¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	float class;
	float class1;
	long workmoney;

	if ((d.bbtime / 60 / 30) < 5)
	{
		vmsg("¤pÂû¤Ó¤p¤F,¤­·³¥H«á¦A¨Ó§a...");
		return 0;
	}
	workmoney = 0;
	class = ((d.hp * 100 / d.maxhp) - d.tired) * LEARN_ELVEL;
	class1 = (d.charm - d.tired) * LEARN_ELVEL;
	d.shit += rand() % 5 + 5;
	count_tired(5, 14, "Y", 100, 1);
	d.hp -= (rand() % 3 + 5);
	d.social -= rand() % 5 + 6;
	d.happy -= (rand() % 4 + 6);
	d.satisfy -= rand() % 3 + 5;
	d.wisdom -= rand() % 3 + 4;
	if (d.wisdom < 0)
		d.wisdom = 0;
	/*show_job_pic(6);*/
	if (class >= 75 && class1 >= 75)
	{
		d.cookskill += rand() % 2 + 7;
		d.speech += rand() % 2 + 5;
		workmoney = 500 + (d.charm) / 5;
		vmsg("§A«Ü¬õ­ò  :)");
	}
	else if ((class < 75 && class >= 50) && class1 >= 50)
	{
		d.cookskill += rand() % 2 + 5;
		d.speech += rand() % 2 + 5;
		workmoney = 400 + (d.charm) / 5;
		vmsg("ÆZ¨üÅwªïªº­C....");
	}
	else if ((class < 50 && class >= 25) && class1 >= 25)
	{
		d.cookskill += rand() % 2 + 4;
		d.speech += rand() % 2 + 3;
		workmoney = 300 + (d.charm) / 5;
		vmsg("«Ü¥­¤Z°Õ..¦ý°¨°¨ªêªê...");
	}
	else
	{
		d.cookskill += rand() % 2 + 2;
		d.speech += rand() % 2 + 2;
		workmoney = 200 + (d.charm) / 5;
		vmsg("§Aªº´A¤O¤£°÷°Õ..½Ð¥[ªo....");
	}
	d.money += workmoney * LEARN_ELVEL;
	d.workN += 1;
	return 0;
}

int pip_job_workO()
{
	/*  ¢u¢w¢w¢w¢w¢q¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	/*  ¢x°s®a    ¢x¾y¤O + 2 , ¸oÄ^ + 2 , ¹D¼w - 3 , «H¥õ - 3   ¢x*/
	/*  ¢x        ¢x«Ý¤H±µª« - N , ©M¤÷¿ËªºÃö«Y - N , ¯h³Ò + 12 ¢x*/
	/*  ¢u¢w¢w¢w¢w¢q¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	/*  ¢x°s®a    ¢x­Y ¾y    ¤O - RND (¯h³Ò) >=  70 «h¤u§@¦¨¥\  ¢x*/
	/*  ¢u¢w¢w¢w¢w¢q¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	float class;
	long workmoney;

	if ((d.bbtime / 60 / 30) < 4)
	{
		vmsg("¤pÂû¤Ó¤p¤F,¥|·³¥H«á¦A¨Ó§a...");
		return 0;
	}
	workmoney = 0;
	class = (d.charm - d.tired) * LEARN_ELVEL;
	d.shit += rand() % 5 + 14;
	d.charm += (rand() % 3 + 8) * LEARN_ELVEL;
	d.offense += (rand() % 3 + 8) * LEARN_ELVEL;
	count_tired(5, 22, "Y", 100, 1);
	d.hp -= (rand() % 3 + 8);
	d.social -= rand() % 6 + 12;
	d.happy -= (rand() % 4 + 8);
	d.satisfy -= rand() % 3 + 8;
	d.etchics -= rand() % 6 + 10;
	d.belief -= rand() % 6 + 10;
	if (d.etchics < 0)
		d.etchics = 0;
	if (d.belief < 0)
		d.belief = 0;

	/*show_job_pic(6);*/
	if (class >= 75)
	{
		d.relation -= rand() % 5 + 12;
		d.toman -= rand() % 5 + 12;
		workmoney = 600 + (d.charm) / 5;
		vmsg("§A¬O¥»©±ªº¬õµP­ò  :)");
	}
	else if (class < 75 && class >= 50)
	{
		d.relation -= rand() % 5 + 8;
		d.toman -= rand() % 5 + 8;
		workmoney = 500 + (d.charm) / 5;
		vmsg("§AÆZ¨üÅwªïªº­C..:)");
	}
	else if (class < 50 && class >= 25)
	{
		d.relation -= rand() % 5 + 5;
		d.toman -= rand() % 5 + 5;
		workmoney = 400 + (d.charm) / 5;
		vmsg("§A«Ü¥­¤Z..¦ý°¨°¨ªêªê°Õ...");
	}
	else
	{
		d.relation -= rand() % 5 + 1;
		d.toman -= rand() % 5 + 1;
		workmoney = 300 + (d.charm) / 5;
		vmsg("­ü..§Aªº´A¤O¤£°÷°Õ....");
	}
	d.money += workmoney * LEARN_ELVEL;
	if (d.relation < 0)
		d.relation = 0;
	if (d.toman < 0)
		d.toman = 0;
	d.workO += 1;
	return 0;
}

int pip_job_workP()
{
	/*  ¢u¢w¢w¢w¢w¢q¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	/*  ¢x¤j©]Á`·|¢x¾y¤O + 3 , ¸oÄ^ + 1 , ®ð½è - 2 , ´¼¤O - 1   ¢x*/
	/*  ¢x        ¢x«Ý¤H±µª« - N , ¯h³Ò + 8                     ¢x*/
	/*  ¢u¢w¢w¢w¢w¢q¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	/*  ¢x¤j©]Á`·|¢x­Y ¾y    ¤O - RND (¯h³Ò) >=  70 ¡®          ¢x*/
	/*  ¢x        ¢x­Y ÃÀ³N­×¾i - RND (¯h³Ò) >=  30 «h¤u§@¦¨¥\  ¢x*/
	/*  ¢|¢w¢w¢w¢w¢r¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢}*/
	float class;
	float class1;
	long workmoney;

	if ((d.bbtime / 60 / 30) < 6)
	{
		vmsg("¤pÂû¤Ó¤p¤F,¤»·³¥H«á¦A¨Ó§a...");
		return 0;
	}
	workmoney = 0;
	class = (d.charm - d.tired) * LEARN_ELVEL;
	class1 = (d.art - d.tired) * LEARN_ELVEL;
	d.shit += rand() % 5 + 7;
	d.charm += (rand() % 3 + 8) * LEARN_ELVEL;
	d.offense += (rand() % 3 + 8) * LEARN_ELVEL;
	count_tired(5, 22, "Y", 100, 1);
	d.hp -= (rand() % 3 + 8);
	d.social -= rand() % 6 + 12;
	d.happy -= (rand() % 4 + 8);
	d.satisfy -= rand() % 3 + 8;
	d.character -= rand() % 3 + 8;
	d.wisdom -= rand() % 3 + 5;
	if (d.character < 0)
		d.character = 0;
	if (d.wisdom < 0)
		d.wisdom = 0;
	/*show_job_pic(6);*/
	if (class >= 75 && class1 > 30)
	{
		d.speech += rand() % 5 + 12;
		d.toman -= rand() % 5 + 12;
		workmoney = 1000 + (d.charm) / 5;
		vmsg("§A¬O©]Á`·|³Ì°{«Gªº¬P¬P­ò  :)");
	}
	else if ((class < 75 && class >= 50) && class1 > 20)
	{
		d.speech += rand() % 5 + 8;
		d.toman -= rand() % 5 + 8;
		workmoney = 800 + (d.charm) / 5;
		vmsg("¶â¶â..§AÆZ¨üÅwªïªº­C..:)");
	}
	else if ((class < 50 && class >= 25) && class1 > 10)
	{
		d.speech += rand() % 5 + 5;
		d.toman -= rand() % 5 + 5;
		workmoney = 600 + (d.charm) / 5;
		vmsg("§A­n¥[ªo¤F°Õ..¦ý´¶´¶°Õ...");
	}
	else
	{
		d.speech += rand() % 5 + 1;
		d.toman -= rand() % 5 + 1;
		workmoney = 400 + (d.charm) / 5;
		vmsg("­ü..§A¤£¦æ°Õ....");
	}
	d.money += workmoney * LEARN_ELVEL;
	if (d.toman < 0)
		d.toman = 0;
	d.workP += 1;
	return 1;
}

/*---------------------------------------------------------------------------*/
/* ª±¼Ö¿ï³æ:´²¨B ®È¹C ¹B°Ê ¬ù·| ²q®±                                         */
/*                                                                           */
/*---------------------------------------------------------------------------*/

int pip_play_stroll()	/*´²¨B*/
{
	int lucky;
	count_tired(3, 3, "Y", 100, 0);
	lucky = rand() % 7;
	if (lucky == 2)
	{
		d.happy += rand() % 3 + rand() % 3 + 9;
		d.satisfy += rand() % 3 + rand() % 3 + 3;
		d.shit += rand() % 3 + 3;
		d.hp -= (rand() % 3 + 5);
		move(4, 0);
		if (rand() % 2 > 0)
			show_play_pic(1);
		else
			show_play_pic(2);
		vmsg("¹J¨ìªB¤ÍÅo  ¯u¦n.... ^_^");
	}
	else if (lucky == 3)
	{
		d.money += 100;
		d.happy += rand() % 3 + 6;
		d.satisfy += rand() % 3 + 4;
		d.shit += rand() % 3 + 3;
		d.hp -= (rand() % 3 + 4);
		move(4, 0);
		show_play_pic(3);
		vmsg("¾ß¨ì¤F100¤¸¤F..­C­C­C....");
	}

	else if (lucky == 4)
	{
		if (rand() % 2 > 0)
		{
			d.happy -= (rand() % 2 + 5);
			move(4, 0);
			d.hp -= (rand() % 3 + 3);
			show_play_pic(4);
			if (d.money >= 50)
			{
				d.money -= 50;
				vmsg("±¼¤F50¤¸¤F..¶ã¶ã¶ã....");
			}
			else
			{
				d.money = 0;
				d.hp -= (rand() % 3 + 3);
				vmsg("¿ú±¼¥ú¥ú¤F..¶ã¶ã¶ã....");
			}
			d.shit += rand() % 3 + 2;
		}
		else
		{
			d.happy += rand() % 3 + 5;
			move(4, 0);
			show_play_pic(5);
			if (d.money >= 50)
			{
				d.money -= 50;
				d.hp -= (rand() % 3 + 3);
				vmsg("¥Î¤F50¤¸¤F..¤£¥i¥H½|§Ú³á....");
			}
			else
			{
				d.money = 0;
				d.hp -= (rand() % 3 + 3);
				vmsg("¿ú³Q§Ú°½¥Î¥ú¥ú¤F..:p");
			}
			d.shit += rand() % 3 + 2;
		}
	}
	else if (lucky == 5)
	{
		d.happy += rand() % 3 + 6;
		d.satisfy += rand() % 3 + 5;
		d.shit += 2;
		move(4, 0);
		if (rand() % 2 > 0)
			show_play_pic(6);
		else
			show_play_pic(7);
		vmsg("¦n´Î³á¾ß¨ìª±¨ã¤F»¡.....");
	}
	else if (lucky == 6)
	{
		d.happy -= (rand() % 3 + 10);
		d.shit += (rand() % 3 + 20);
		move(4, 0);
		show_play_pic(9);
		vmsg("¯u¬O­Ë·°  ¥i¥H¥h¶R·R°ê¼ú¨é");
	}
	else
	{
		d.happy += rand() % 3 + 3;
		d.satisfy += rand() % 2 + 1;
		d.shit += rand() % 3 + 2;
		d.hp -= (rand() % 3 + 2);
		move(4, 0);
		show_play_pic(8);
		vmsg("¨S¦³¯S§Oªº¨Æµo¥Í°Õ.....");
	}
	return 0;
}

int pip_play_sport()	/*¹B°Ê*/
{
	count_tired(3, 8, "Y", 100, 1);
	d.weight -= (rand() % 3 + 2);
	d.satisfy += rand() % 2 + 3;
	if (d.satisfy > 100)
		d.satisfy = 100;
	d.shit += rand() % 5 + 10;
	d.hp -= (rand() % 2 + 8);
	d.maxhp += rand() % 2;
	d.speed += (2 + rand() % 3);
	move(4, 0);
	show_play_pic(10);
	vmsg("¹B°Ê¦n³B¦h¦h°Õ...");
	return 0;
}

int pip_play_date()	/*¬ù·|*/
{
	if (d.money < 150)
	{
		vmsg("§A¿ú¤£°÷¦h°Õ! ¬ù·|Á`±oªáÂI¿ú¿ú");
	}
	else
	{
		count_tired(3, 6, "Y", 100, 1);
		d.happy += rand() % 5 + 12;
		d.shit += rand() % 3 + 5;
		d.hp -= rand() % 4 + 8;
		d.satisfy += rand() % 5 + 7;
		d.character += rand() % 3 + 1;
		d.money = d.money - 150;
		move(4, 0);
		show_play_pic(11);
		vmsg("¬ù·|¥h  ©I©I");
	}
	return 0;
}
int pip_play_outing()	/*­¥¹C*/
{
	int lucky;
	char buf[256];

	if (d.money < 250)
	{
		vmsg("§A¿ú¤£°÷¦h°Õ! ®È¹CÁ`±oªáÂI¿ú¿ú");
	}
	else
	{
		d.weight += rand() % 2 + 1;
		d.money -= 250;
		count_tired(10, 45, "N", 100, 0);
		d.hp -= rand() % 10 + 20;
		if (d.hp >= d.maxhp)
			d.hp = d.maxhp;
		d.happy += rand() % 10 + 12;
		d.character += rand() % 5 + 5;
		d.satisfy += rand() % 10 + 10;
		lucky = rand() % 4;
		if (lucky == 0)
		{
			d.maxmp += rand() % 3;
			d.art += rand() % 2;
			show_play_pic(12);
			if (rand() % 2 > 0)
				vmsg("¤ß¤¤¦³¤@ªÑ²H²Hªº·PÄ±  ¦nµÎªA³á....");
			else
				vmsg("¶³¤ô ¶~±¡ ¤ß±¡¦n¦h¤F.....");
		}
		else if (lucky == 1)
		{
			d.art += rand() % 3;
			d.maxmp += rand() % 2;
			show_play_pic(13);
			if (rand() % 2 > 0)
				vmsg("¦³¤s¦³¤ô¦³¸¨¤é  §Î¦¨¤@´T¬üÄRªºµe..");
			else
				vmsg("¬ÝµÛ¬ÝµÛ  ¥þ¨­¯h¾Î³£¤£¨£Åo..");
		}
		else if (lucky == 2)
		{
			d.love += rand() % 3;
			show_play_pic(14);
			if (rand() % 2 > 0)
				vmsg("¬Ý  ¤Ó¶§§Ö¨S¤J¤ô¤¤Åo...");
			else
				vmsg("Å¥»¡³o¬O®üÃä°Õ  §A»¡©O?");
		}
		else if (lucky == 3)
		{
			d.maxhp += rand() % 3;
			show_play_pic(15);
			if (rand() % 2 > 0)
				vmsg("Åý§Ú­ÌºÆ¨g¦b©]¸Ìªº®üÅy§a....©I©I..");
			else
				vmsg("²D²nªº®ü­·ªï­±Å§¨Ó  ³Ì³ßÅw³oºØ·PÄ±¤F....");
		}
		if ((rand() % 301 + rand() % 200) % 100 == 12)
		{
			lucky = 0;
			clear();
			sprintf(buf, "[1;41m  " NICKNAME PIPNAME " ¡ã %-10s                                                  [0m", d.name);
			show_play_pic(0);
			move(17, 10);
			prints("[1;36m¿Ë·Rªº [1;33m%s ¡ã[0m", d.name);
			move(18, 10);
			prints("[1;37m¬Ý¨ì§A³o¼Ë§V¤Oªº°ö¾i¦Û¤vªº¯à¤O  Åý§Ú¤ß¤¤¤Q¤Àªº°ª¿³³á..[m");
			move(19, 10);
			prints("[1;36m¤p¤Ñ¨Ï§Ú¨M©wµ¹§A¼ú½à¹ªÀy¹ªÀy  °½°½¦aÀ°§U§A¤@¤U....^_^[0m");
			move(20, 10);
			lucky = rand() % 7;
			if (lucky == 6)
			{
				prints("[1;33m§Ú±NÀ°§Aªº¦U¶µ¯à¤O¥þ³¡´£¤É¦Ê¤À¤§¤­³á......[0m");
				d.maxhp = d.maxhp * 105 / 100;
				d.hp = d.maxhp;
				d.maxmp = d.maxmp * 105 / 100;
				d.mp = d.maxmp;
				d.attack = d.attack * 105 / 100;
				d.resist = d.resist * 105 / 100;
				d.speed = d.speed * 105 / 100;
				d.character = d.character * 105 / 100;
				d.love = d.love * 105 / 100;
				d.wisdom = d.wisdom * 105 / 100;
				d.art = d.art * 105 / 100;
				d.brave = d.brave * 105 / 100;
				d.homework = d.homework * 105 / 100;
			}

			else if (lucky <= 5 && lucky >= 4)
			{
				prints("[1;33m§Ú±NÀ°§Aªº¾Ô°«¯à¤O¥þ³¡´£¤É¦Ê¤À¤§¤Q³á.......[0m");
				d.attack = d.attack * 110 / 100;
				d.resist = d.resist * 110 / 100;
				d.speed = d.speed * 110 / 100;
				d.brave = d.brave * 110 / 100;
			}

			else if (lucky <= 3 && lucky >= 2)
			{
				prints("[1;33m§Ú±NÀ°§AªºÅ]ªk¯à¤O©M¥Í©R¤O¥þ³¡´£¤É¦Ê¤À¤§¤Q³á.......[0m");
				d.maxhp = d.maxhp * 110 / 100;
				d.hp = d.maxhp;
				d.maxmp = d.maxmp * 110 / 100;
				d.mp = d.maxmp;
			}
			else if (lucky <= 1 && lucky >= 0)
			{
				prints("[1;33m§Ú±NÀ°§Aªº·P¨ü¯à¤O¥þ³¡´£¤É¦Ê¤À¤§¤G¤Q³á....[0m");
				d.character = d.character * 110 / 100;
				d.love = d.love * 110 / 100;
				d.wisdom = d.wisdom * 110 / 100;
				d.art = d.art * 110 / 100;
				d.homework = d.homework * 110 / 100;
			}

			vmsg("½ÐÄ~Äò¥[ªo³á...");
		}
	}
	return 0;
}

int pip_play_kite()	/*­·ºå*/
{
	count_tired(4, 4, "Y", 100, 0);
	d.weight += (rand() % 2 + 2);
	d.satisfy += rand() % 3 + 12;
	if (d.satisfy > 100)
		d.satisfy = 100;
	d.happy += rand() % 5 + 10;
	d.shit += rand() % 5 + 6;
	d.hp -= (rand() % 2 + 7);
	d.affect += rand() % 4 + 6;
	move(4, 0);
	show_play_pic(16);
	vmsg("©ñ­·ºå¯u¦nª±°Õ...");
	return 0;
}

int pip_play_KTV()	/*KTV*/
{
	if (d.money < 250)
	{
		vmsg("§A¿ú¤£°÷¦h°Õ! °ÛºqÁ`±oªáÂI¿ú¿ú");
	}
	else
	{
		count_tired(10, 10, "Y", 100, 0);
		d.satisfy += rand() % 2 + 20;
		if (d.satisfy > 100)
			d.satisfy = 100;
		d.happy += rand() % 3 + 20;
		d.shit += rand() % 5 + 6;
		d.money -= 250;
		d.hp += (rand() % 2 + 6);
		d.art += rand() % 4 + 3;
		move(4, 0);
		show_play_pic(17);
		vmsg("§A»¡§A  ·Q­n°k...");
	}
	return 0;
}

int pip_play_guess()   /* ²q®±µ{¦¡ */
{
	int com;
	int pipkey;
	struct tm *qtime;
	time_t now;

	time(&now);
	qtime = localtime(&now);
	d.satisfy += (rand() % 3 + 2);
	count_tired(2, 2, "Y", 100, 1);
	d.shit += rand() % 3 + 2;
	do
	{
		if (d.death == 1 || d.death == 2 || d.death == 3)
			return 0;
		if (pip_mainmenu(0)) return 0;
		move(b_lines -2, 0);
		clrtoeol();
		move(b_lines, 0);
		clrtoeol();
		move(b_lines, 0);
		prints("[1;44;37m  ²q®±¿ï³æ  [46m[1]§Ú¥X°Å¤M [2]§Ú¥X¥ÛÀY [3]§Ú¥X¥¬°Õ [4]²q®±°O¿ý [Q]¸õ¥X¡G         [m");
		move(b_lines - 1, 0);
		clrtoeol();
		pipkey = vkey();
		switch (pipkey)
		{
		case '4':
			situ();
			break;
		}
	}
	while ((pipkey != '1') && (pipkey != '2') && (pipkey != '3') && (pipkey != 'q') && (pipkey != 'Q'));

	com = rand() % 3;
	move(18, 0);
	clrtobot();
	switch (com)
	{
	case 0:
		outs("¤pÂû¡G°Å¤M\n");
		break;
	case 1:
		outs("¤pÂû¡G¥ÛÀY\n");
		break;
	case 2:
		outs("¤pÂû¡G¥¬\n");
		break;
	}

	move(17, 0);

	switch (pipkey)
	{
	case '1':
		outs("§A  ¡G°Å¤M\n");
		if (com == 0)
			tie();
		else  if (com == 1)
			lose();
		else if (com == 2)
			win();
		break;
	case '2':
		outs("§A¡@¡G¥ÛÀY\n");
		if (com == 0)
			win();
		else if (com == 1)
			tie();
		else if (com == 2)
			lose();
		break;
	case '3':
		outs("§A¡@¡G¥¬\n");
		if (com == 0)
			lose();
		else if (com == 1)
			win();
		else if (com == 2)
			tie();
		break;
	case 'q':
		break;
	}

}

void win()
{
	d.winn++;
	d.hp -= rand() % 2 + 3;
	move(4, 0);
	show_guess_pic(2);
	move(b_lines, 0);
	vmsg("¤pÂû¿é¤F....~>_<~");
	return;
}

void tie()
{
	d.hp -= rand() % 2 + 3;
	d.happy += rand() % 3 + 5;
	move(4, 0);
	show_guess_pic(3);
	move(b_lines, 0);
	vmsg("¥­¤â........-_-");
	return;
}

void lose()
{
	d.losee++;
	d.happy += rand() % 3 + 5;
	d.hp -= rand() % 2 + 3;
	move(4, 0);
	show_guess_pic(1);
	move(b_lines, 0);
	vmsg("¤pÂûÄ¹Åo....*^_^*");
	return;
}

void situ()
{
	clrchyiuan(19, 21);
	move(19, 0);
	prints("§A:[44m %d³Ó %d­t[m                     \n", d.winn, d.losee);
	move(20, 0);
	prints("Âû:[44m %d³Ó %d­t[m                     \n", d.losee, d.winn);

	if (d.winn >= d.losee)
	{
		move(b_lines, 0);
		vmsg("«¢..Ä¹¤pÂû¤]¨S¦h¥úºa");
	}
	else
	{
		move(b_lines, 0);
		vmsg("²Â³J..³º¿éµ¹¤FÂû....£­...");
	}
	return;
}

/*---------------------------------------------------------------------------*/
/* ­×¦æ¿ï³æ:©À®Ñ ½mªZ ­×¦æ                                                   */
/*                                                                           */
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/* ¸ê®Æ®w                                                                    */
/*---------------------------------------------------------------------------*/
char *classrank[6] = {"¨S¦³", "ªì¯Å", "¤¤¯Å", "°ª¯Å", "¶i¶¥", "±M·~"};
int classmoney[11][2] = {{ 0,  0},
	{60, 110}, {70, 120}, {70, 120}, {80, 130}, {70, 120},
	{60, 110}, {90, 140}, {70, 120}, {70, 120}, {80, 130}
};
int classvariable[11][4] =
{
	{0, 0, 0, 0},
	{5, 5, 4, 4}, {5, 7, 6, 4}, {5, 7, 6, 4}, {5, 6, 5, 4}, {7, 5, 4, 6},
	{7, 5, 4, 6}, {6, 5, 4, 6}, {6, 6, 5, 4}, {5, 5, 4, 7}, {7, 5, 4, 7}
};


char *classword[11][5] =
{
	{"½Ò¦W", "¦¨¥\\¤@", "¦¨¥\\¤G", "¥¢±Ñ¤@", "¥¢±Ñ¤G"},

	{"¦ÛµM¬ì¾Ç", "¥¿¦b¥Î¥\\Åª®Ñ¤¤..", "§Ú¬OÁo©úÂû cccc...",
	 "³oÃD«ç»ò¬Ý¤£À´«¨..©Ç¤F", "°á¤£§¹¤F :~~~~~~"},

	{"­ð¸Ö§ºµü", "§É«e©ú¤ë¥ú...ºÃ¬O¦a¤WÁ÷...", "¬õ¨§¥Í«n°ê..¬K¨Óµo´XªK..",
	 "£°..¤W½Ò¤£­n¬y¤f¤ô", "§AÁÙ²V³á..»@§A­I·|­ð¸Ö¤T¦Ê­º"},

	{"¯«¾Ç±Ð¨|", "«¢¹p¸ô¨È  «¢¹p¸ô¨È", "Åý§Ú­Ìªï±µ¤Ñ°ó¤§ªù",
	 "£°..§A¦b·F¹À£«? ÁÙ¤£¦n¦n°á", "¯«¾Ç«ÜÄYµÂªº..½Ð¦n¦n¾Ç..:("},

	{"­x¾Ç±Ð¨|", "®]¤l§Lªk¬O¤¤°ê§Lªk®Ñ..", "±q­x³ø°ê¡A§Ú­n±a§L¥h¥´¥M",
	 "¤°»ò°}§Î£«?²V¶Ã°}§Î?? @_@", "§AÁÙ¥H¬°§A¦bª±¤T°ê§Ó£«?"},

	{"¼C¹D§Þ³N", "¬Ý§Úªº¼F®`  ¿W©t¤E¼C....", "§Ú¨ë §Ú¨ë §Ú¨ë¨ë¨ë..",
	 "¼C­n®³Ã­¤@ÂI°Õ..", "§A¦b¨ë¦a¹«£«? ¼C®³°ª¤@ÂI"},

	{"®æ°«¾Ô§Þ", "¦Ù¦×¬O¦Ù¦×  ©I©I..", "¤Q¤K»É¤H¦æ®ð´²..",
	 "¸}¦A½ð°ª¤@ÂI°Õ...", "®±ÀY«ç»ò³o»ò¨S¤O£«.."},

	{"Å]ªk±Ð¨|", "§ÚÅÜ §ÚÅÜ §ÚÅÜÅÜÅÜ..", "³DÁx+Áµ»i§À+¹«¤ú+ÃÊßï=??",
	 "¤p¤ß§Aªº±½©ª°Õ  ¤£­n¶Ã´§..", "£°¡ã¤f¤ô¤£­n¬y¨ì¤ô´¹²y¤W.."},

	{"Â§»ö±Ð¨|", "­n·í°¦¦³Â§»ªªºÂû...", "¼Ú¶Ù­ò..£«­ù£«¨§..",
	 "«ç»ò¾Ç¤£·|£«??¤Ñ§r..", "¨«°_¸ô¨Ó¨S¨«¼Ë..¤Ñ£«.."},

	{"Ã¸µe§Þ¥©", "«Ü¤£¿ù­ò..¦³¬ü³N¤Ñ¥÷..", "³o´TµeªºÃC¦â·f°tªº«Ü¦n..",
	 "¤£­n°­µe²Å°Õ..­n¥[ªo..", "¤£­n«rµeµ§°Õ..ÃaÃa¤pÂû³á.."},

	{"»RÁÐ§Þ¥©", "§A´N¹³¤@°¦¤ÑÃZ³á..", "»RÁÐ²Ó­M«Ü¦n³á..",
	 "¨­Åé¦A¬X³n¤@ÂI..", "«ô°U§AÀu¬ü¤@ÂI..¤£­n³o»ò²Ê¾|.."}
};
/*---------------------------------------------------------------------------*/
/* ­×¦æ¿ï³æ:©À®Ñ ½mªZ ­×¦æ                                                   */
/* ¨ç¦¡®w                                                                    */
/*---------------------------------------------------------------------------*/

int pip_practice_classA()
{
	/*  ¢u¢w¢w¢w¢w¢q¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	/*  ¢x¦ÛµM¬ì¾Ç¢x´¼¤O + 1~ 4 , «H¥õ - 0~0 , §ÜÅ]¯à¤O - 0~0   ¢x*/
	/*  ¢x        ¢u¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	/*  ¢x        ¢x´¼¤O + 2~ 6 , «H¥õ - 0~1 , §ÜÅ]¯à¤O - 0~1   ¢x*/
	/*  ¢x        ¢u¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	/*  ¢x        ¢x´¼¤O + 3~ 8 , «H¥õ - 0~2 , §ÜÅ]¯à¤O - 0~1   ¢x*/
	/*  ¢x        ¢u¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	/*  ¢x        ¢x´¼¤O + 4~12 , «H¥õ - 1~3 , §ÜÅ]¯à¤O - 0~1   ¢x*/
	/*  ¢u¢w¢w¢w¢w¢q¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	int body, class;
	int change1, change2, change3, change4, change5;

	class = d.wisdom / 200 + 1; /*¬ì¾Ç*/
	if (class > 5) class = 5;

	body = pip_practice_function(1, class, 11, 12, &change1, &change2, &change3, &change4, &change5);
	if (body == 0) return 0;
	d.wisdom += change4 * LEARN_ELVEL;
	if (body == 1)
	{
		d.belief -= rand() % (2 + class * 2);
		d.mresist -= rand() % 4;
	}
	else
	{
		d.belief -= rand() % (2 + class * 2);
		d.mresist -= rand() % 3;
	}
	pip_practice_gradeup(1, class, d.wisdom / 200 + 1);
	if (d.belief < 0)  d.belief = 0;
	if (d.mresist < 0) d.mresist = 0;
	d.classA += 1;
	return 0;
}

int pip_practice_classB()
{
	/*  ¢u¢w¢w¢w¢w¢q¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	/*  ¢x¸Öµü    ¢x·P¨ü + 1~1 , ´¼¤O + 0~1 , ÃÀ³N­×¾i + 0~1    ¢x*/
	/*  ¢x        ¢x®ð½è + 0~1                                  ¢x*/
	/*  ¢x        ¢u¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	/*  ¢x        ¢x·P¨ü + 1~2 , ´¼¤O + 0~2 , ÃÀ³N­×¾i + 0~1    ¢x*/
	/*  ¢x        ¢x®ð½è + 0~1                                  ¢x*/
	/*  ¢x        ¢u¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	/*  ¢x        ¢x·P¨ü + 1~4 , ´¼¤O + 0~3 , ÃÀ³N­×¾i + 0~1    ¢x*/
	/*  ¢x        ¢x®ð½è + 0~1                                  ¢x*/
	/*  ¢x        ¢u¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	/*  ¢x        ¢x·P¨ü + 2~5 , ´¼¤O + 0~4 , ÃÀ³N­×¾i + 0~1    ¢x*/
	/*  ¢x        ¢x®ð½è + 0~1                                  ¢x*/
	/*  ¢u¢w¢w¢w¢w¢q¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	int body, class;
	int change1, change2, change3, change4, change5;

	class = (d.affect * 2 + d.wisdom + d.art * 2 + d.character) / 400 + 1; /*¸Öµü*/
	if (class > 5) class = 5;

	body = pip_practice_function(2, class, 21, 21, &change1, &change2, &change3, &change4, &change5);
	if (body == 0) return 0;
	d.affect += change3 * LEARN_ELVEL;
	if (body == 1)
	{
		d.wisdom += rand() % (class + 3) * LEARN_ELVEL;
		d.character += rand() % (class + 3) * LEARN_ELVEL;
		d.art += rand() % (class + 3) * LEARN_ELVEL;
	}
	else
	{
		d.wisdom += rand() % (class + 2) * LEARN_ELVEL;
		d.character += rand() % (class + 2) * LEARN_ELVEL;
		d.art += rand() % (class + 2) * LEARN_ELVEL;
	}
	body = (d.affect * 2 + d.wisdom + d.art * 2 + d.character) / 400 + 1;
	pip_practice_gradeup(2, class, body);
	d.classB += 1;
	return 0;
}

int pip_practice_classC()
{
	/*  ¢u¢w¢w¢w¢w¢q¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	/*  ¢x¯«¾Ç    ¢x´¼¤O + 1~1 , «H¥õ + 1~2 , §ÜÅ]¯à¤O + 0~1    ¢x*/
	/*  ¢x        ¢u¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	/*  ¢x        ¢x´¼¤O + 1~1 , «H¥õ + 1~3 , §ÜÅ]¯à¤O + 0~1    ¢x*/
	/*  ¢x        ¢u¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	/*  ¢x        ¢x´¼¤O + 1~2 , «H¥õ + 1~4 , §ÜÅ]¯à¤O + 0~1    ¢x*/
	/*  ¢x        ¢u¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	/*  ¢x        ¢x´¼¤O + 1~3 , «H¥õ + 1~5 , §ÜÅ]¯à¤O + 0~1    ¢x*/
	/*  ¢u¢w¢w¢w¢w¢q¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	int body, class;
	int change1, change2, change3, change4, change5;

	class = (d.belief * 2 + d.wisdom) / 400 + 1; /*¯«¾Ç*/
	if (class > 5) class = 5;

	body = pip_practice_function(3, class, 31, 31, &change1, &change2, &change3, &change4, &change5);
	if (body == 0) return 0;
	d.wisdom += change2 * LEARN_ELVEL;
	d.belief += change3 * LEARN_ELVEL;
	if (body == 1)
	{
		d.mresist += rand() % 5 * LEARN_ELVEL;
	}
	else
	{
		d.mresist += rand() % 3 * LEARN_ELVEL;
	}
	body = (d.belief * 2 + d.wisdom) / 400 + 1;
	pip_practice_gradeup(3, class, body);
	d.classC += 1;
	return 0;
}

int pip_practice_classD()
{
	/*  ¢u¢w¢w¢w¢w¢q¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	/*  ¢x­x¾Ç    ¢x´¼¤O + 1~2 , ¾Ô°«§Þ³N + 0~1 , ·P¨ü - 0~1    ¢x*/
	/*  ¢x        ¢u¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	/*  ¢x        ¢x´¼¤O + 2~4 , ¾Ô°«§Þ³N + 0~1 , ·P¨ü - 0~1    ¢x*/
	/*  ¢x        ¢u¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	/*  ¢x        ¢x´¼¤O + 3~4 , ¾Ô°«§Þ³N + 0~1 , ·P¨ü - 0~1    ¢x*/
	/*  ¢x        ¢u¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	/*  ¢x        ¢x´¼¤O + 4~5 , ¾Ô°«§Þ³N + 0~1 , ·P¨ü - 0~1    ¢x*/
	/*  ¢u¢w¢w¢w¢w¢q¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	int body, class;
	int change1, change2, change3, change4, change5;

	class = (d.hskill * 2 + d.wisdom) / 400 + 1;
	if (class > 5) class = 5;
	body = pip_practice_function(4, class, 41, 41, &change1, &change2, &change3, &change4, &change5);
	if (body == 0) return 0;
	d.wisdom += change2 * LEARN_ELVEL;
	if (body == 1)
	{
		d.hskill += (rand() % 3 + 4) * LEARN_ELVEL;
		d.affect -= rand() % 3 + 6;
	}
	else
	{
		d.hskill += (rand() % 3 + 2) * LEARN_ELVEL;
		d.affect -= rand() % 3 + 6;
	}
	body = (d.hskill * 2 + d.wisdom) / 400 + 1;
	pip_practice_gradeup(4, class, body);
	if (d.affect < 0)  d.affect = 0;
	d.classD += 1;
	return 0;
}

int pip_practice_classE()
{
	/*  ¢u¢w¢w¢w¢w¢q¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	/*  ¢x¼C³N    ¢x¾Ô°«§Þ³N + 0~1 , §ðÀ»¯à¤O + 1~1             ¢x*/
	/*  ¢x        ¢u¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	/*  ¢x        ¢x¾Ô°«§Þ³N + 0~1 , §ðÀ»¯à¤O + 1~2             ¢x*/
	/*  ¢x        ¢u¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	/*  ¢x        ¢x¾Ô°«§Þ³N + 0~1 , §ðÀ»¯à¤O + 1~3             ¢x*/
	/*  ¢x        ¢u¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	/*  ¢x        ¢x¾Ô°«§Þ³N + 0~1 , §ðÀ»¯à¤O + 1~4             ¢x*/
	/*  ¢u¢w¢w¢w¢w¢q¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	int body, class;
	int change1, change2, change3, change4, change5;

	class = (d.hskill + d.attack) / 400 + 1;
	if (class > 5) class = 5;

	body = pip_practice_function(5, class, 51, 51, &change1, &change2, &change3, &change4, &change5);
	if (body == 0) return 0;
	d.speed += (rand() % 3 + 2) * LEARN_ELVEL;
	d.hexp += (rand() % 2 + 2) * LEARN_ELVEL;
	d.attack += change4 * LEARN_ELVEL;
	if (body == 1)
	{
		d.hskill += (rand() % 3 + 5) * LEARN_ELVEL;
	}
	else
	{
		d.hskill += (rand() % 3 + 3) * LEARN_ELVEL;
	}
	body = (d.hskill + d.attack) / 400 + 1;
	pip_practice_gradeup(5, class, body);
	d.classE += 1;
	return 0;
}

int pip_practice_classF()
{
	/*  ¢u¢w¢w¢w¢w¢q¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	/*  ¢x®æ°«³N  ¢x¾Ô°«§Þ³N + 1~1 , ¨¾¿m¯à¤O + 0~0             ¢x*/
	/*  ¢x        ¢u¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	/*  ¢x        ¢x¾Ô°«§Þ³N + 1~1 , ¨¾¿m¯à¤O + 0~1             ¢x*/
	/*  ¢x        ¢u¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	/*  ¢x        ¢x¾Ô°«§Þ³N + 1~2 , ¨¾¿m¯à¤O + 0~1             ¢x*/
	/*  ¢x        ¢u¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	/*  ¢x        ¢x¾Ô°«§Þ³N + 1~3 , ¨¾¿m¯à¤O + 0~1             ¢x*/
	/*  ¢u¢w¢w¢w¢w¢q¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	int body, class;
	int change1, change2, change3, change4, change5;

	class = (d.hskill + d.resist) / 400 + 1;
	if (class > 5) class = 5;

	body = pip_practice_function(6, class, 61, 61, &change1, &change2, &change3, &change4, &change5);
	if (body == 0) return 0;
	d.hexp += (rand() % 2 + 2) * LEARN_ELVEL;
	d.speed += (rand() % 3 + 2) * LEARN_ELVEL;
	d.resist += change2 * LEARN_ELVEL;
	if (body == 1)
	{
		d.hskill += (rand() % 3 + 5) * LEARN_ELVEL;
	}
	else
	{
		d.hskill += (rand() % 3 + 3) * LEARN_ELVEL;
	}
	body = (d.hskill + d.resist) / 400 + 1;
	pip_practice_gradeup(6, class, body);
	d.classF += 1;
	return 0;
}

int pip_practice_classG()
{
	/*  ¢u¢w¢w¢w¢w¢q¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	/*  ¢xÅ]ªk    ¢xÅ]ªk§Þ³N + 1~1 , Å]ªk¯à¤O + 0~2             ¢x*/
	/*  ¢x        ¢u¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	/*  ¢x        ¢xÅ]ªk§Þ³N + 1~2 , Å]ªk¯à¤O + 0~3             ¢x*/
	/*  ¢x        ¢u¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	/*  ¢x        ¢xÅ]ªk§Þ³N + 1~3 , Å]ªk¯à¤O + 0~4             ¢x*/
	/*  ¢x        ¢u¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	/*  ¢x        ¢xÅ]ªk§Þ³N + 2~4 , Å]ªk¯à¤O + 0~5             ¢x*/
	/*  ¢u¢w¢w¢w¢w¢q¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	int body, class;
	int change1, change2, change3, change4, change5;

	class = (d.mskill + d.maxmp) / 400 + 1;
	if (class > 5) class = 5;

	body = pip_practice_function(7, class, 71, 72, &change1, &change2, &change3, &change4, &change5);
	if (body == 0) return 0;
	d.maxmp += change3 * LEARN_ELVEL;
	d.mexp += (rand() % 2 + 2) * LEARN_ELVEL;
	if (body == 1)
	{
		d.mskill += (rand() % 3 + 7) * LEARN_ELVEL;
	}
	else
	{
		d.mskill += (rand() % 3 + 4) * LEARN_ELVEL;
	}

	body = (d.mskill + d.maxmp) / 400 + 1;
	pip_practice_gradeup(7, class, body);
	d.classG += 1;
	return 0;
}

int pip_practice_classH()
{
	/*  ¢u¢w¢w¢w¢w¢q¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	/*  ¢xÂ§»ö    ¢xÂ§»öªí²{ + 1~1 , ®ð½è + 1~1                 ¢x*/
	/*  ¢x        ¢u¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	/*  ¢x        ¢xÂ§»öªí²{ + 1~2 , ®ð½è + 1~2                 ¢x*/
	/*  ¢x        ¢u¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	/*  ¢x        ¢xÂ§»öªí²{ + 1~3 , ®ð½è + 1~3                 ¢x*/
	/*  ¢x        ¢u¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	/*  ¢x        ¢xÂ§»öªí²{ + 2~4 , ®ð½è + 1~4                 ¢x*/
	/*  ¢u¢w¢w¢w¢w¢q¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	int body, class;
	int change1, change2, change3, change4, change5;

	class = (d.manners * 2 + d.character) / 400 + 1;
	if (class > 5) class = 5;

	body = pip_practice_function(8, class, 0, 0, &change1, &change2, &change3, &change4, &change5);
	if (body == 0) return 0;
	d.social += (rand() % 2 + 2) * LEARN_ELVEL;
	d.manners += (change1 + rand() % 2) * LEARN_ELVEL;
	d.character += (change1 + rand() % 2) * LEARN_ELVEL;
	body = (d.character + d.manners) / 400 + 1;
	pip_practice_gradeup(8, class, body);
	d.classH += 1;
	return 0;
}

int pip_practice_classI()
{
	/*  ¢u¢w¢w¢w¢w¢q¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	/*  ¢xÃ¸µe    ¢xÃÀ³N­×¾i + 1~1 , ·P¨ü + 0~1                 ¢x*/
	/*  ¢x        ¢u¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	/*  ¢x        ¢xÃÀ³N­×¾i + 1~2 , ·P¨ü + 0~1                 ¢x*/
	/*  ¢x        ¢u¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	/*  ¢x        ¢xÃÀ³N­×¾i + 1~3 , ·P¨ü + 0~1                 ¢x*/
	/*  ¢x        ¢u¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	/*  ¢x        ¢xÃÀ³N­×¾i + 2~4 , ·P¨ü + 0~1                 ¢x*/
	/*  ¢u¢w¢w¢w¢w¢q¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	int body, class;
	int change1, change2, change3, change4, change5;

	class = (d.art * 2 + d.character) / 400 + 1;
	if (class > 5) class = 5;

	body = pip_practice_function(9, class, 91, 91, &change1, &change2, &change3, &change4, &change5);
	if (body == 0) return 0;
	d.art += change4 * LEARN_ELVEL;
	d.affect += change2 * LEARN_ELVEL;
	body = (d.affect + d.art) / 400 + 1;
	pip_practice_gradeup(9, class, body);
	d.classI += 1;
	return 0;
}

int pip_practice_classJ()
{
	/*  ¢u¢w¢w¢w¢w¢q¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	/*  ¢x»RÁÐ    ¢xÃÀ³N­×¾i + 0~1 , ¾y¤O + 0~1 , Åé¤O + 1~1    ¢x*/
	/*  ¢x        ¢u¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	/*  ¢x        ¢xÃÀ³N­×¾i + 1~1 , ¾y¤O + 0~1 , Åé¤O + 1~1    ¢x*/
	/*  ¢x        ¢u¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	/*  ¢x        ¢xÃÀ³N­×¾i + 1~2 , ¾y¤O + 0~2 , Åé¤O + 1~1    ¢x*/
	/*  ¢x        ¢u¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t*/
	/*  ¢x        ¢xÃÀ³N­×¾i + 1~3 , ¾y¤O + 1~2 , Åé¤O + 1~1    ¢x*/
	/*  ¢|¢w¢w¢w¢w¢r¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢}*/
	int body, class;
	int change1, change2, change3, change4, change5;

	class = (d.art * 2 + d.charm) / 400 + 1;
	if (class > 5) class = 5;

	body = pip_practice_function(10, class, 0, 0, &change1, &change2, &change3, &change4, &change5);
	if (body == 0) return 0;
	d.art += change2 * LEARN_ELVEL;
	d.maxhp += (rand() % 3 + 2) * LEARN_ELVEL;
	if (body == 1)
	{
		d.charm += rand() % (4 + class) * LEARN_ELVEL;
	}
	else if (body == 2)
	{
		d.charm += rand() % (2 + class) * LEARN_ELVEL;
	}
	body = (d.art * 2 + d.charm) / 400 + 1;
	pip_practice_gradeup(10, class, body);
	d.classJ += 1;
	return 0;
}

/*¶Ç¤J:½Ò¸¹ µ¥¯Å ¥Í©R §Ö¼Ö º¡¨¬ Å¼Å¼ ¶Ç¦^:ÅÜ¼Æ12345 return:body*/
int
pip_practice_function(classnum, classgrade, pic1, pic2, change1, change2, change3, change4, change5)
int classnum, classgrade, pic1, pic2;
int *change1, *change2, *change3, *change4, *change5;
{
	int  a, b, body, health;
	char inbuf[256], ans[5];
	long smoney;

	/*¿úªººâªk*/
	smoney = classgrade * classmoney[classnum][0] + classmoney[classnum][1];
	move(b_lines - 2, 0);
	clrtoeol();
	sprintf(inbuf, "[%8s%4s½Òµ{]­nªá $%d ,½T©w­n¶Ü??[y/N]: ", classword[classnum][0], classrank[classgrade], smoney);
	getdata(b_lines - 2, 1, inbuf, ans, 2, DOECHO, 0);
	if (ans[0] != 'y' && ans[0] != 'Y')  return 0;
	if (d.money < smoney)
	{
		vmsg("«Ü©êºp³á...§Aªº¿ú¤£°÷³á");
		return 0;
	}
	count_tired(4, 5, "Y", 100, 1);
	d.money = d.money - smoney;
	/*¦¨¥\»P§_ªº§PÂ_*/
	health = d.hp * 1 / 2 + rand() % 20 - d.tired;
	if (health > 0) body = 1;
	else body = 2;

	a = rand() % 3 + 2;
	b = (rand() % 12 + rand() % 13) % 2;
	d.hp -= rand() % (3 + rand() % 3) + classvariable[classnum][0];
	d.happy -= rand() % (3 + rand() % 3) + classvariable[classnum][1];
	d.satisfy -= rand() % (3 + rand() % 3) + classvariable[classnum][2];
	d.shit += rand() % (3 + rand() % 3) + classvariable[classnum][3];
	*change1 = rand() % a + 2 + classgrade * 2 / (body + 1);	/* rand()%3+3 */
	*change2 = rand() % a + 4 + classgrade * 2 / (body + 1);	/* rand()%3+5 */
	*change3 = rand() % a + 5 + classgrade * 3 / (body + 1);	/* rand()%3+7 */
	*change4 = rand() % a + 7 + classgrade * 3 / (body + 1);	/* rand()%3+9 */
	*change5 = rand() % a + 9 + classgrade * 3 / (body + 1);	/* rand()%3+11 */
	if (rand() % 2 > 0 && pic1 > 0)
		show_practice_pic(pic1);
	else if (pic2 > 0)
		show_practice_pic(pic2);
	vmsg(classword[classnum][body+b]);
	return body;
}

int pip_practice_gradeup(classnum, classgrade, data)
int classnum, classgrade, data;
{
	char inbuf[256];

	if ((data == (classgrade + 1)) && classgrade < 5)
	{
		sprintf(inbuf, "¤U¦¸´«¤W [%8s%4s½Òµ{]",
				classword[classnum][0], classrank[classgrade+1]);
		vmsg(inbuf);
	}
	return 0;
}

/*---------------------------------------------------------------------------*/
/* ¯S®í¿ï³æ:¬Ý¯f ´îªÎ ¾Ô°« «ô³X ´Â¨£                                         */
/*                                                                           */
/*---------------------------------------------------------------------------*/


int pip_see_doctor()	/*¬ÝÂå¥Í*/
{
	char buf[256];
	long savemoney;
	savemoney = d.sick * 25;
	if (d.sick <= 0)
	{
		vmsg("«z­ù..¨S¯f¨ÓÂå°|·F¹À..³Q½|¤F..¶ã~~");
		d.character -= (rand() % 3 + 1);
		if (d.character < 0)
			d.character = 0;
		d.happy -= (rand() % 3 + 3);
		d.satisfy -= rand() % 3 + 2;
	}
	else if (d.money < savemoney)
	{
		sprintf(buf, "§Aªº¯f­nªá %d ¤¸³á....§A¤£°÷¿ú°Õ...", savemoney);
		vmsg(buf);
	}
	else if (d.sick > 0 && d.money >= savemoney)
	{
		d.tired -= rand() % 10 + 20;
		if (d.tired < 0)
			d.tired = 0;
		d.sick = 0;
		d.money = d.money - savemoney;
		move(4, 0);
		show_special_pic(1);
		vmsg("ÃÄ¨ì¯f°£..¨S¦³°Æ§@¥Î!!");
	}
	return 0;
}

/*´îªÎ*/
int pip_change_weight()
{
	char genbuf[5];
	char inbuf[256];
	int weightmp;

	move(b_lines -1, 0);
	clrtoeol();
	show_special_pic(2);
	getdata(b_lines - 1, 1, "§Aªº¿ï¾Ü¬O? [Q]Â÷¶}: ", genbuf, 2, 1, 0);
	if (genbuf[0] == '1' || genbuf[0] == '2' || genbuf[0] == '3' || genbuf[0] == '4')
	{
		switch (genbuf[0])
		{
		case '1':
			if (d.money < 80)
			{
				vmsg("¶Ç²Î¼W­D­n80¤¸³á....§A¤£°÷¿ú°Õ...");
			}
			else
			{
				getdata(b_lines - 1, 1, "»Ýªá¶O80¤¸(3¡ã5¤½¤ç)¡A§A½T©w¶Ü? [y/N]: ", genbuf, 2, 1, 0);
				if (genbuf[0] == 'Y' || genbuf[0] == 'y')
				{
					weightmp = 3 + rand() % 3;
					d.weight += weightmp;
					d.money -= 80;
					d.maxhp -= rand() % 2;
					d.hp -= rand() % 2 + 3;
					show_special_pic(3);
					sprintf(inbuf, "Á`¦@¼W¥[¤F%d¤½¤ç", weightmp);
					vmsg(inbuf);
				}
				else
				{
					vmsg("¦^¤ßÂà·NÅo.....");
				}
			}
			break;

		case '2':
			getdata(b_lines - 1, 1, "¼W¤@¤½¤ç­n30¤¸¡A§A­n¼W¦h¤Ö¤½¤ç©O? [½Ð¶ñ¼Æ¦r]: ", genbuf, 4, 1, 0);
			weightmp = atoi(genbuf);
			if (weightmp <= 0)
			{
				vmsg("¿é¤J¦³»~..©ñ±óÅo...");
			}
			else if (d.money > (weightmp*30))
			{
				sprintf(inbuf, "¼W¥[%d¤½¤ç¡AÁ`¦@»Ýªá¶O¤F%d¤¸¡A½T©w¶Ü? [y/N]: ", weightmp, weightmp*30);
				getdata(b_lines - 1, 1, inbuf, genbuf, 2, 1, 0);
				if (genbuf[0] == 'Y' || genbuf[0] == 'y')
				{
					d.money -= weightmp * 30;
					d.weight += weightmp;
					d.maxhp -= (rand() % 2 + 2);
					count_tired(5, 8, "N", 100, 1);
					d.hp -= (rand() % 2 + 3);
					d.sick += rand() % 10 + 5;
					show_special_pic(3);
					sprintf(inbuf, "Á`¦@¼W¥[¤F%d¤½¤ç", weightmp);
					vmsg(inbuf);
				}
				else
				{
					vmsg("¦^¤ßÂà·NÅo.....");
				}
			}
			else
			{
				vmsg("§A¿ú¨S¨º»ò¦h°Õ.......");
			}
			break;

		case '3':
			if (d.money < 80)
			{
				vmsg("¶Ç²Î´îªÎ­n80¤¸³á....§A¤£°÷¿ú°Õ...");
			}
			else
			{
				getdata(b_lines - 1, 1, "»Ýªá¶O80¤¸(3¡ã5¤½¤ç)¡A§A½T©w¶Ü? [y/N]: ", genbuf, 2, 1, 0);
				if (genbuf[0] == 'Y' || genbuf[0] == 'y')
				{
					weightmp = 3 + rand() % 3;
					d.weight -= weightmp;
					if (d.weight < 0)
						d.weight = 0;
					d.money -= 100;
					d.maxhp += rand() % 2;
					d.hp -= rand() % 2 + 3;
					show_special_pic(4);
					sprintf(inbuf, "Á`¦@´î¤Ö¤F%d¤½¤ç", weightmp);
					vmsg(inbuf);
				}
				else
				{
					vmsg("¦^¤ßÂà·NÅo.....");
				}
			}
			break;
		case '4':
			getdata(b_lines - 1, 1, "´î¤@¤½¤ç­n30¤¸¡A§A­n´î¦h¤Ö¤½¤ç©O? [½Ð¶ñ¼Æ¦r]: ", genbuf, 4, 1, 0);
			weightmp = atoi(genbuf);
			if (weightmp <= 0)
			{
				vmsg("¿é¤J¦³»~..©ñ±óÅo...");
			}
			else if (d.weight <= weightmp)
			{
				vmsg("§A¨S¨º»ò­«³á.....");
			}
			else if (d.money > (weightmp*30))
			{
				sprintf(inbuf, "´î¤Ö%d¤½¤ç¡AÁ`¦@»Ýªá¶O¤F%d¤¸¡A½T©w¶Ü? [y/N]: ", weightmp, weightmp*30);
				getdata(b_lines - 1, 1, inbuf, genbuf, 2, 1, 0);
				if (genbuf[0] == 'Y' || genbuf[0] == 'y')
				{
					d.money -= weightmp * 30;
					d.weight -= weightmp;
					d.maxhp -= (rand() % 2 + 2);
					count_tired(5, 8, "N", 100, 1);
					d.hp -= (rand() % 2 + 3);
					d.sick += rand() % 10 + 5;
					show_special_pic(4);
					sprintf(inbuf, "Á`¦@´î¤Ö¤F%d¤½¤ç", weightmp);
					vmsg(inbuf);
				}
				else
				{
					vmsg("¦^¤ßÂà·NÅo.....");
				}
			}
			else
			{
				vmsg("§A¿ú¨S¨º»ò¦h°Õ.......");
			}
			break;
		}
	}
	return 0;
}


/*°Ñ¨£*/

int
pip_go_palace()
{
	pip_go_palace_screen(royallist);
	return 0;
}

int
pip_go_palace_screen(p)
struct royalset *p;
{
	int n;
	int a;
	int b;
	int choice;
	int pipkey;
	int change;
	char buf[256];
	char inbuf1[20];
	char inbuf2[20];
	char *needmode[3] = {"      ", "Â§»öªí²{¡Ö", "½Í¦R§Þ¥©¡Ö"};
	int save[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	d.nodone = 0;
	do
	{
		clear();
		show_palace_pic(0);
		move(13, 4);
		sprintf(buf, "[1;31m¢z¢w¢w¢w¢w¢w¢w¢t[37;41m ¨Ó¨ìÁ`¥q¥O³¡¤F  ½Ð¿ï¾Ü§A±ý«ô³Xªº¹ï¶H [0;1;31m¢u¢w¢w¢w¢w¢w¢w¢{[0m");
		prints(buf);
		move(14, 4);
		sprintf(buf, "[1;31m¢x                                                                  ¢x[0m");
		prints(buf);

		for (n = 0;n < 5;n++)
		{
			a = 2 * n + 1;
			b = 2 * n + 2;
			move(15 + n, 4);
			sprintf(inbuf1, "%-10s%3d", needmode[p[a].needmode], p[a].needvalue);
			if (n == 4)
			{
				sprintf(inbuf2, "%-10s", needmode[p[b].needmode]);
			}
			else
			{
				sprintf(inbuf2, "%-10s%3d", needmode[p[b].needmode], p[b].needvalue);
			}
			if ((d.seeroyalJ == 1 && n == 4) || (n != 4))
				sprintf(buf, "[1;31m¢x [36m([37m%s[36m) [33m%-10s  [37m%-14s     [36m([37m%s[36m) [33m%-10s  [37m%-14s[31m¢x[0m",
						p[a].num, p[a].name, inbuf1, p[b].num, p[b].name, inbuf2);
			else
				sprintf(buf, "[1;31m¢x [36m([37m%s[36m) [33m%-10s  [37m%-14s                                   [31m¢x[0m",
						p[a].num, p[a].name, inbuf1);
			prints(buf);
		}
		move(20, 4);
		sprintf(buf, "[1;31m¢x                                                                  ¢x[0m");
		prints(buf);
		move(21, 4);
		sprintf(buf, "[1;31m¢|¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢}[0m");
		prints(buf);


		if (d.death == 1 || d.death == 2 || d.death == 3)
			return 0;
		/*±N¦U¤H°È¤w¸gµ¹»Pªº¼Æ­È¥s¦^¨Ó*/
		save[1] = d.royalA;          /*from¦u½Ã*/
		save[2] = d.royalB;          /*fromªñ½Ã*/
		save[3] = d.royalC;		/*from±N­x*/
		save[4] = d.royalD;          /*from¤j¦Ú*/
		save[5] = d.royalE;          /*from²½¥q*/
		save[6] = d.royalF;          /*fromÃd¦m*/
		save[7] = d.royalG;          /*from¤ý¦m*/
		save[8] = d.royalH;          /*from°ê¤ý*/
		save[9] = d.royalI;          /*from¤p¤¡*/
		save[10] = d.royalJ;         /*from¤ý¤l*/

		move(b_lines - 1, 0);
		clrtoeol();
		move(b_lines - 1, 0);
		prints("[1;33m [¥Í©R¤O] %d/%d  [¯h³Ò«×] %d [0m", d.hp, d.maxhp, d.tired);

		move(b_lines, 0);
		clrtoeol();
		move(b_lines, 0);
		prints(
			"[1;37;46m  °Ñ¨£¿ï³æ  [44m [¦r¥À]¿ï¾Ü±ý«ô³Xªº¤Hª«  [Q]Â÷¶}" NICKNAME "Á`¥q¥O³¡¡G                  [0m");
		pipkey = vkey();
		choice = pipkey - 64;
		if (choice < 1 || choice > 10)
			choice = pipkey - 96;

		if ((choice >= 1 && choice <= 10 && d.seeroyalJ == 1) || (choice >= 1 && choice <= 9 && d.seeroyalJ == 0))
		{
			d.social += rand() % 3 + 3;
			d.hp -= rand() % 5 + 6;
			d.tired += rand() % 5 + 8;
			if (d.tired >= 100)
			{
				d.death = 1;
				pipdie("[1;31m²Ö¦º¤F...[m  ", 1);
			}
			if (d.hp < 0)
			{
				d.death = 1;
				pipdie("[1;31m¾j¦º¤F...[m  ", 1);
			}
			if (d.death == 1)
			{
				sprintf(buf, "ÙTÙT¤F...¯u¬O´d±¡..");
			}
			else
			{
				if ((p[choice].needmode == 0) ||
					(p[choice].needmode == 1 && d.manners >= p[choice].needvalue) ||
					(p[choice].needmode == 2 && d.speech >= p[choice].needvalue))
				{
					if (choice >= 1 && choice <= 9 && save[choice] >= p[choice].maxtoman)
					{
						if (rand() % 2 > 0)
							sprintf(buf, "¯à©M³o»ò°¶¤jªº§AÁ¿¸Ü¯u¬Oºa©¯£«...");
						else
							sprintf(buf, "«Ü°ª¿³§A¨Ó«ô³X§Ú¡A¦ý§Ú¤£¯àµ¹§A¤°»ò¤F..");
					}
					else
					{
						change = 0;
						if (choice >= 1 && choice <= 8)
						{
							switch (choice)
							{
							case 1:
								change = d.character / 5;
								break;
							case 2:
								change = d.character / 8;
								break;
							case 3:
								change = d.charm / 5;
								break;
							case 4:
								change = d.wisdom / 10;
								break;
							case 5:
								change = d.belief / 10;
								break;
							case 6:
								change = d.speech / 10;
								break;
							case 7:
								change = d.social / 10;
								break;
							case 8:
								change = d.hexp / 10;
								break;
							}
							/*¦pªG¤j©ó¨C¦¸ªº¼W¥[³Ì¤j¶q*/
							if (change > p[choice].addtoman)
								change = p[choice].addtoman;
							/*¦pªG¥[¤W­ì¥ýªº¤§«á¤j©ó©Ò¯àµ¹ªº©Ò¦³­È®É*/
							if ((change + save[choice]) >= p[choice].maxtoman)
								change = p[choice].maxtoman - save[choice];
							save[choice] += change;
							d.toman += change;
						}
						else if (choice == 9)
						{
							save[9] = 0;
							d.social -= 13 + rand() % 4;
							d.affect += 13 + rand() % 4;
						}
						else if (choice == 10 && d.seeroyalJ == 1)
						{
							save[10] += 15 + rand() % 4;
							d.seeroyalJ = 0;
						}
						if (rand() % 2 > 0)
							sprintf(buf, "%s", p[choice].words1);
						else
							sprintf(buf, "%s", p[choice].words2);
					}
				}
				else
				{
					if (rand() % 2 > 0)
						sprintf(buf, "§Ú¤£©M©p³o¼ËªºÂû½Í¸Ü....");
					else
						sprintf(buf, "§A³o°¦¨S±Ð¾iªºÂû¡A¦A¥h¾Ç¾ÇÂ§»ö§a....");

				}
			}
			vmsg(buf);
		}
		d.royalA = save[1];
		d.royalB = save[2];
		d.royalC = save[3];
		d.royalD = save[4];
		d.royalE = save[5];
		d.royalF = save[6];
		d.royalG = save[7];
		d.royalH = save[8];
		d.royalI = save[9];
		d.royalJ = save[10];
	}
	while ((pipkey != 'Q') && (pipkey != 'q') && (pipkey != KEY_LEFT));

	vmsg("Â÷¶}" NICKNAME "Á`¥q¥O³¡.....");
	return 0;
}
/*--------------------------------------------------------------------------*/
/* pip_vs_fight.c ¤pÂû¹ï¾Ôµ{¦¡				                    */
/* §@ªÌ:chyiuan   ·PÁÂSiEptheroªº§Þ³N«ü¾É				    */
/*--------------------------------------------------------------------------*/
#ifdef	HAVE_PIP_FIGHT
static int
pip_set_currutmp()
{
	currutmp->pip->hp = d.hp;
	currutmp->pip->mp = d.mp;
	currutmp->pip->maxhp = d.maxhp;
	currutmp->pip->maxmp = d.maxmp;
	currutmp->pip->attack = d.attack;
	currutmp->pip->resist = d.resist;
	currutmp->pip->mresist = d.mresist;
	currutmp->pip->speed = d.speed;
}

static int
pip_get_currutmp()
{
	d.hp = currutmp->pip->hp;
	d.mp = currutmp->pip->mp;
}

int
pip_vf_fight(fd, first)
int fd;
int first;
{
	pipdata temp;
	struct chicken chickentemp;
	int ch, datac, dinjure, oldtired, oldhp;
	int oldhexp, oldmexp, oldhskill, oldmskill, oldbrave;
	int gameover = 0;
	int i;
	int notyou = 0;			/*chyiuan:¥H§K°T®§³Q§Ë¿ù*/
	float mresist;
	UTMP *opponent;
	char data[200], buf1[256], buf2[256], mymsg[8][150];

	memcpy(&temp, &(cutmp->pip), sizeof(pipdata));
	memcpy(&chickentemp, &d, sizeof(d));


	currutmp = cutmp;
	utmp_mode(M_CHICKEN);
	clear();
	pip_read_file(cuser.userid);
	currutmp->pip->pipmode = 0;	/*1:¿é¤F 2:Ä¹¤F 3:¤£ª±¤F */
	currutmp->pip->leaving = 1;
	currutmp->pip->mode = d.chickenmode;
	pip_set_currutmp();		/*§â¤pÂûªºdata  down load for³Q©I¥sªÌ*/
	currutmp->pip->nodone = first;	/*¨M©w½Ö¥ý§ðÀ»*/
	currutmp->pip->msgcount = 0;	/*¾Ô°«°T®§Âk¹s*/
	currutmp->pip->chatcount = 0;	/*²á¤Ñ°T®§Âk¹s*/
	currutmp->pip->msg[0] = '\0';
	strcpy(currutmp->pip->name, d.name);


	/*¦s¤UÂÂ¤pÂûdata*/
	oldmexp = d.mexp;
	oldhexp = d.hexp;
	oldbrave = d.brave;
	oldhskill = d.hskill;
	oldmskill = d.mskill;
	opponent = cutmp->talker;
	add_io(fd, 2);
	/*¹ï¤è¥¼·Ç³Æ§´·í  ¥ýµ¥¤@¤U  ¬°¤F¨¾¤î·í¾÷ */
	while (gameover == 0 && (opponent->pip == NULL || opponent->pip->leaving == 0))
	{
		move(b_lines, 0);
		prints("[1;46m ¹ï¤èÁÙ¦b·Ç³Æ¤¤                                                        [m");
		ch = vkey();
	}
	if (currutmp->pip->mode != opponent->pip->mode)
	{
		vmsg("¤@¥NÂû»P¤G¥NÂû¤£¯à¤¬¬Û PK !!");
		add_io(0, 60);
		return 0;
	}
	for (i = 0;i < 8;i++)
		mymsg[i][0] = '\0';
	for (i = 0;i < 10;i++)
		currutmp->pip->chat[i][0] = '\0';
	/*¶}©lªº°T®§*/
	sprintf(mymsg[0], "[1;37m%s ©M %s ªº¾Ô°«¶}©l¤F..[m",
			opponent->pip->name, currutmp->pip->name);
	strcpy(currutmp->pip->msg, mymsg[0]);
	currutmp->pip->msgcount = 0;
	/*msgcount©Mcharcountªººâªk¤£¦P*/
	add_io(fd, 1);
	/*	currutmp->pip->mode=0;*/
	while (!(opponent->pip || currutmp->pip->leaving == 0 || opponent->pip->leaving == 0))
	{
		clear();
		/*¬°¤F¤@¨Ç¨ä¥Lªº­ì¦]  ¹³Áý­¹µ¥¬O©I¥sÂÂªº  ©Ò¥Hreload*/
		pip_get_currutmp();
		/*		pip_set_currutmp();*/

		if (opponent->pip->nodone != 1)
			strcpy(mymsg[currutmp->pip->msgcount%8], currutmp->pip->msg);
		move(0, 0);
		prints("[1;34mùùùâ[44;37m ¦Û¤v¸ê®Æ [0;1;34mùàùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùù[m\n");
		prints("[1m   [33m©m  ¦W:[37m%-20s                                              [31m  [m\n",
			   d.name);
		sprintf(buf1, "%d/%d", d.hp, d.maxhp);
		sprintf(buf2, "%d/%d", d.mp, d.maxmp);
		prints("[1m   [33mÅé  ¤O:[37m%-24s       [33mªk  ¤O:[37m%-24s[33m[m\n",
			   buf1, buf2);
		prints("[1m   [33m§ð  À»:[37m%-12d[33m¨¾  ¿m:[37m%-12d[33m³t  «×:[37m%-12d[33m§Ü  Å]:[37m%-9d  [m\n",
			   d.attack, d.resist, d.speed, d.mresist);
		prints("[1m   [33m¾Ô°«§Þ:[37m%-12d[33mÅ]ªk§Þ:[37m%-12d[33mÅ]µû»ù:[37m%-12d[33mªZµû»ù:[37m%-9d  [m\n",
			   d.hskill, d.mskill, d.mexp, d.hexp);
		prints("[1m   [33m­¹  ª«:[37m%-12d[33m¸É  ¤Y:[37m%-12d[33m¹s  ­¹:[37m%-12d[33mÆF  ªÛ:[37m%-9d  [m\n",
			   d.food, d.bighp, d.cookie, d.medicine);
		prints("[1m   [33m¤H  çx:[37m%-12d[33m³·  ½¬:[37m%-12d[33m¯h  ³Ò:[37m%-15d               [m\n",
			   d.ginseng, d.snowgrass, d.tired);
		move(7, 0);
		prints("[1;34mùùùâ[44;37m ¾Ô°«°T®§ [0;1;34mùàùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùù[m\n");
		for (i = 0;i < 8;i++)
		{
			move(8 + i, 1);

			if (currutmp->pip->msgcount < 8)
			{
				prints(mymsg[i]);
				/*¾A¥Îpip.msgcount¦b8¦æ¤º*/
			}
			else
			{
				prints(mymsg[(currutmp->pip->msgcount-8+i)%8]);
				/*pip.msgcount=8:ªí¥Ü¤w¸g¦³9­Ó ©Ò¥H±q0->7*/
			}
		}
		move(16, 0);
		prints("[1;34mùùùâ[44;37m ½Í¸Ü°T®§ [0;1;34mùàùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùù[m\n");
		for (i = 0;i < 2;i++)
		{
			move(17 + i, 0);
			if (currutmp->pip->chatcount < 3)
			{
				prints(currutmp->pip->chat[i]);
				/*¾A¥Îpip.chatcount¦b2¦æ¤º*/
			}
			else
			{
				prints("%s", currutmp->pip->chat[(currutmp->pip->chatcount-2+i)%10]);
				/*pip.chatcount=3:ªí¥Ü¤w¸g¦³2­Ó ©Ò¥H±q0->1*/
			}
		}
		move(19, 0);
		prints("[1;34mùùùâ[1;37;44m ¹ï¤â¸ê®Æ [0;1;34mùàùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùù[m\n");
		prints("[1m   [33m©m  ¦W:[37m%-20s                                                [m\n",
			   opponent->pip->name);
		sprintf(buf1, "%d/%d", opponent->pip->hp, opponent->pip->maxhp);
		sprintf(buf2, "%d/%d", opponent->pip->mp, opponent->pip->maxmp);
		prints("[1m   [33mÅé  ¤O:[37m%-24s       [33mªk  ¤O:[37m%-24s[m\n",
			   buf1, buf2);
		prints("[1;34mùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùù[m\n");
		if (opponent->pip->nodone == 1)
		{
			notyou = 1;
			prints("[1;37;44m  ¹ï¤è¥X©Û¤¤¡A½Ðµy«Ý¤@·|.....                                [T/^T]CHAT/¦^ÅU  [m");
		}
		else
		{
			notyou = 0;
			prints("[1;44;37m  ¾Ô°«©R¥O  [46m [1]´¶³q [2]¥þ¤O [3]Å]ªk [4]¨¾¿m [5]¸É¥R [6]°k©R [T/^T]CHAT/¦^ÅU  [m");
		}

		while ((ch = vkey()) == I_TIMEOUT)
		{
			if (opponent->pip->nodone != 1 && notyou == 1)
				break;
		}

		if (ch == I_OTHERDATA)
		{
			datac = recv(fd, data, sizeof(data), 0);
			if (datac <= 0)
				break;
		}
		else if (ch == 'T' || ch == 't')
		{
			int len;
			char msg[120];
			char buf[80];
			len = getdata(b_lines, 0, "·Q»¡: ", buf, 60, 1, 0);
			if (len && buf[0] != ' ')
			{
				sprintf(msg, "[1;46;33m¡¹%s[37;45m %s [m", cuser.userid, buf);
				strcpy(opponent->pip->chat[currutmp->pip->chatcount%10], msg);
				strcpy(currutmp->pip->chat[currutmp->pip->chatcount%10], msg);
				opponent->pip->chatcount++;
				currutmp->pip->chatcount++;
			}

		}
		else if (ch == Ctrl('T'))
		{
			add_io(fd, 30);
			clrchyiuan(7, 19);
			move(7, 0);
			prints("[1;31mùùùâ[41;37m ¦^ÅU½Í¸Ü [0;1;31mùàùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùù[m\n");
			for (i = 0;i < 10;i++)
			{
				move(8 + i, 0);
				if (currutmp->pip->chatcount < 10)
				{
					prints(currutmp->pip->chat[i]);
					/*¾A¥Îpip.msgcount¦b¤C¦æ¤º*/
				}
				else
				{
					prints("%s", currutmp->pip->chat[(currutmp->pip->chatcount-10+i)%10]);
					/*pip.chatcount=10:ªí¥Ü¤w¸g¦³11­Ó ©Ò¥H±q0->9*/
				}
			}
			move(18, 0);
			prints("[1;31mùùùâ[41;37m ¨ì¦¹¬°¤î [0;1;31mùàùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùù[m");
			vmsg("¦^ÅU¤§«eªº½Í¸Ü ¥u¦³10³q");
			add_io(fd, 1);
		}
		else if (currutmp->pip->nodone == 1 && opponent->pip->leaving == 1 && notyou == 0)
		{
			d.nodone = 1;
			switch (ch)
			{
				char buf[256];
			case '1':
				if (rand() % 9 == 0)
				{
					vmsg("³ºµM¨S¥´¤¤..:~~~");
					sprintf(buf, "[1;33m%s [37m¹ï [33m%s[37m ¬I®i´¶³q§ðÀ»¡A¦ý¬O¨S¦³¥´¤¤...",
							d.name, opponent->pip->name);
				}
				else
				{
					if (opponent->pip->resistmore == 0)
						dinjure = (d.hskill / 100 + d.hexp / 100 + d.attack / 9 - opponent->pip->resist / 12 + rand() % 20 - opponent->pip->speed / 30 + d.speed / 30);
					else
						dinjure = (d.hskill / 100 + d.hexp / 100 + d.attack / 9 - opponent->pip->resist / 6 + rand() % 20 - opponent->pip->speed / 10 + d.speed / 30);
					if (dinjure <= 10)	dinjure = 10;
					opponent->pip->hp -= dinjure;
					d.hexp += rand() % 2 + 2;
					d.hskill += rand() % 2 + 1;
					sprintf(buf, "´¶³q§ðÀ»,¹ï¤èÅé¤O´î§C%d", dinjure);
					vmsg(buf);
					sprintf(buf, "[1;33m%s [37m¬I®i¤F´¶³q§ðÀ»,[33m%s [37mªºÅé¤O´î§C [31m%d [37mÂI[m"
							, d.name, opponent->pip->name, dinjure);
				}
				opponent->pip->resistmore = 0;
				opponent->pip->msgcount++;
				currutmp->pip->msgcount++;
				strcpy(opponent->pip->msg, buf);
				strcpy(mymsg[currutmp->pip->msgcount%8], buf);
				currutmp->pip->nodone = 2;	/*°µ§¹*/
				opponent->pip->nodone = 1;
				break;

			case '2':
				show_fight_pic(2);
				if (rand() % 11 == 0)
				{
					vmsg("³ºµM¨S¥´¤¤..:~~~");
					sprintf(buf, "[1;33m%s [37m¹ï [33m%s[37m ¬I®i¥þ¤O§ðÀ»¡A¦ý¬O¨S¦³¥´¤¤...",
							d.name, opponent->pip->name);
				}
				else
				{
					if (opponent->pip->resistmore == 0)
						dinjure = (d.hskill / 100 + d.hexp / 100 + d.attack / 5 - opponent->pip->resist / 12 + rand() % 30 - opponent->pip->speed / 50 + d.speed / 30);
					else
						dinjure = (d.hskill / 100 + d.hexp / 100 + d.attack / 5 - opponent->pip->resist / 6 + rand() % 30 - opponent->pip->speed / 30 + d.speed / 30);
					if (dinjure <= 20) dinjure = 20;
					if (d.hp > 5)
					{
						opponent->pip->hp -= dinjure;
						d.hp -= 5;
						d.hexp += rand() % 3 + 3;
						d.hskill += rand() % 2 + 2;
						sprintf(buf, "¥þ¤O§ðÀ»,¹ï¤èÅé¤O´î§C%d", dinjure);
						vmsg(buf);
						sprintf(buf, "[1;33m%s [37m¬I®i¤F¥þ¤O§ðÀ»,[33m%s [37mªºÅé¤O´î§C [31m%d [37mÂI[m"
								, d.name, opponent->pip->name, dinjure);
					}
					else
					{
						d.nodone = 1;
						vmsg("§AªºHP¤p©ó5°Õ..¤£¦æ°Õ...");
					}
				}
				opponent->pip->resistmore = 0;
				opponent->pip->msgcount++;
				currutmp->pip->msgcount++;
				strcpy(opponent->pip->msg, buf);
				strcpy(mymsg[currutmp->pip->msgcount%8], buf);
				currutmp->pip->nodone = 2;	/*°µ§¹*/
				opponent->pip->nodone = 1;
				break;

			case '3':
				clrchyiuan(8, 19);
				oldtired = d.tired;
				oldhp = d.hp;
				d.magicmode = 0;
				add_io(fd, 60);
				dinjure = pip_magic_menu(1, opponent);
				add_io(fd, 1);
				if (dinjure < 0)
					dinjure = 5;
				if (d.nodone == 0)
				{
					if (d.magicmode == 1)
					{
						oldtired = oldtired - d.tired;
						oldhp = d.hp - oldhp;
						sprintf(buf, "ªvÀø«á,Åé¤O´£°ª%d ¯h³Ò­°§C%d", oldhp, oldtired);
						vmsg(buf);
						sprintf(buf, "[1;33m%s [37m¨Ï¥ÎÅ]ªkªvÀø¤§«á,Åé¤O´£°ª [36m%d [37mÂI¡A¯h³Ò­°§C [36m%d [37mÂI[m", d.name, oldhp, oldtired);
					}
					else
					{
						if (rand() % 15 == 0)
						{
							vmsg("³ºµM¨S¥´¤¤..:~~~");
							sprintf(buf, "[1;33m%s [37m¹ï [33m%s[37m ¬I®iÅ]ªk§ðÀ»¡A¦ý¬O¨S¦³¥´¤¤...",
									d.name, opponent->pip->name);
						}
						else
						{
							dinjure = get_hurt(dinjure, d.mexp);

							mresist = (d.mexp) / (opponent->pip->mresist + 1);
							if (mresist > 3)
								mresist = 3;
							if (mresist < 0.3)
								mresist = 0.3;

							dinjure = (int)dinjure * mresist;

							opponent->pip->hp -= dinjure;
							d.mskill += rand() % 2 + 2;
							sprintf(buf, "Å]ªk§ðÀ»,¹ï¤èÅé¤O´î§C%d", dinjure);
							vmsg(buf);
							sprintf(buf, "[1;33m%s [37m¬I®i¤FÅ]ªk§ðÀ»,[33m%s [37mªºÅé¤O´î§C [31m%d [37mÂI[m"
									, d.name, opponent->pip->name, dinjure);
						}
					}

					opponent->pip->msgcount++;
					currutmp->pip->msgcount++;
					strcpy(opponent->pip->msg, buf);
					strcpy(mymsg[currutmp->pip->msgcount%8], buf);
					/*«ì´_Åé¤O¬O¥Îd.hp©Md.maxhp¥h ©Ò¥H±o§ó·s*/
					currutmp->pip->hp = d.hp;
					currutmp->pip->mp = d.mp;
					currutmp->pip->nodone = 2;	/*°µ§¹*/
					opponent->pip->nodone = 1;
					pip_set_currutmp();
				}
				break;

			case '4':
				currutmp->pip->resistmore = 1;
				vmsg("¤pÂû¥[±j¨¾¿m°Õ....");
				sprintf(buf, "[1;33m%s [37m¥[±j¨¾¿m¡A·Ç³Æ¥þ¤O©è¾× [33m%s [37mªº¤U¤@©Û[m",
						d.name, opponent->pip->name);
				opponent->pip->msgcount++;
				currutmp->pip->msgcount++;
				strcpy(opponent->pip->msg, buf);
				strcpy(mymsg[currutmp->pip->msgcount%8], buf);
				currutmp->pip->nodone = 2;	/*°µ§¹*/
				opponent->pip->nodone = 1;
				break;
			case '5':
				add_io(fd, 60);
				pip_fight_feed();
				add_io(fd, 1);
				if (d.nodone != 1)
				{
					sprintf(buf, "[1;33m%s [37m¸É¥R¤F¨­¤Wªº¯à¶q¡AÅé¤O©Îªk¤O¦³ÅãµÛªº´£¤É[m", d.name);
					opponent->pip->msgcount++;
					currutmp->pip->msgcount++;
					strcpy(opponent->pip->msg, buf);
					strcpy(mymsg[currutmp->pip->msgcount%8], buf);
					/*«ì´_Åé¤O¬O¥Îd.hp©Md.maxhp¥h ©Ò¥H±o§ó·s*/
					currutmp->pip->hp = d.hp;
					currutmp->pip->mp = d.mp;
					currutmp->pip->nodone = 2;	/*°µ§¹*/
					opponent->pip->nodone = 1;
					pip_set_currutmp();
				}
				break;
			case '6':
				opponent->pip->msgcount++;
				currutmp->pip->msgcount++;
				if (rand() % 20 >= 18 || (rand() % 20 > 13 && d.speed <= opponent->pip->speed))
				{
					vmsg("·Q°k¶]¡A«o¥¢±Ñ¤F...");
					sprintf(buf, "[1;33m%s [37m·Q¥ý°k¶]¦A»¡...¦ý«o¥¢±Ñ¤F...[m", d.name);
					strcpy(opponent->pip->msg, buf);
					strcpy(mymsg[currutmp->pip->msgcount%8], buf);
				}
				else
				{
					sprintf(buf, "[1;33m%s [37m¦ÛÄ±¥´¤£¹L¹ï¤è¡A©Ò¥H¨M©w¥ý°k¶]¦A»¡...[m", d.name);
					strcpy(opponent->pip->msg, buf);
					strcpy(mymsg[currutmp->pip->msgcount%8], buf);
					currutmp->pip->pipmode = 3;
					clear();
					vs_head("¹q¤l¾i¤pÂû", BoardName);
					move(10, 0);
					prints("            [1;31m¢z¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢{[m\n");
					prints("            [1;31m¢x  [37m¹ê¤O¤£±jªº¤pÂû [33m%-10s                 [31m¢x[m\n", d.name);
					prints("            [1;31m¢x  [37m¦b»P¹ï¤â [32m%-10s [37m¾Ô°««á¸¨¶]°Õ          [31m¢x[m\n", opponent->pip->name);
					prints("            [1;31m¢|¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢}[m\n");
					currutmp->pip->leaving = 0;
					add_io(fd, 60);
					vmsg("¤T¤Q¤»­p ¨«¬°¤Wµ¦...");
				}
				currutmp->pip->nodone = 2;	/*°µ§¹*/
				opponent->pip->nodone = 1;
				break;


			}
		}
		if (currutmp->pip->hp < 0)
		{
			currutmp->pip->pipmode = 1;
			opponent->pip->pipmode = 2;
		}
		if (currutmp->pip->pipmode == 2 || opponent->pip->pipmode == 1 || opponent->pip->pipmode == 3)
		{
			clear();
			vs_head("¹q¤l¾i¤pÂû", BoardName);
			move(10, 0);
			prints("            [1;31m¢z¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢{[m\n");
			prints("            [1;31m¢x  [37m­^«iªº¤pÂû [33m%-10s                     [31m¢x[m\n", d.name);
			prints("            [1;31m¢x  [37m¥´±Ñ¤F¹ï¤è¤pÂû [32m%-10s                 [31m¢x[m\n", opponent->pip->name);
			prints("            [1;31m¢|¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢}[m");
			currutmp->pip->leaving = 0;
			add_io(fd, 60);
			if (opponent->pip->hp <= 0)
				vmsg("¹ï¤è¦º±¼Åo..©Ò¥H§AÄ¹Åo..");
			else if (opponent->pip->hp > 0)
				vmsg("¹ï¤è¸¨¶]Åo..©Ò¥Hºâ§AÄ¹Åo.....");
		}
		if (gameover != 1 && (opponent->pip->pipmode == 2 || currutmp->pip->pipmode == 1))
		{
			clear();
			vs_head("¹q¤l¾i¤pÂû", BoardName);
			move(10, 0);
			prints("            [1;31m¢z¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢{[m\n");
			prints("            [1;31m¢x  [37m¥i¼¦ªº¤pÂû [33m%-10s                     [31m¢x[m\n", d.name);
			prints("            [1;31m¢x  [37m¦b»P [32m%-10s [37mªº¾Ô°«¤¤¡A                [31m¢x[m\n", opponent->pip->name);
			prints("            [1;31m¢x  [37m¤£©¯¦a¥´¿é¤F¡A°OªÌ²{³õ¯S§O³ø¾É.........   [31m¢x[m\n");
			prints("            [1;31m¢|¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢}[m\n");
			currutmp->pip->leaving = 0;
			add_io(fd, 60);
			vmsg("¤pÂû¥´¿é¤F....");
		}
		if (opponent->pip->pipmode != 0 || currutmp->pip->pipmode != 0)
		{
			currutmp->pip->leaving = 0;
		}

	}
	add_io(0, 60);
	close(fd);
	vmsg(NULL);
	utmp_mode(M_CHICKEN);
	memcpy(&(cutmp->pip), &temp, sizeof(pipdata));
	memcpy(&d, &chickentemp, sizeof(d));
	return 0;
}
#endif

/*---------------------------------------------------------------------------*/
/* µ²§½¨ç¦¡                                                                  */
/*                                                                           */
/*---------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/*  µ²§½°Ñ¼Æ³]©w                                                            */
/*--------------------------------------------------------------------------*/

int /*µ²§½µe­±*/
pip_ending_screen()
{
	time_t now;
	char buf[256];
	char endbuf1[50];
	char endbuf2[50];
	char endbuf3[50];
	int endgrade = 0;
	int endmode = 0;
	clear();
	pip_ending_decide(endbuf1, endbuf2, endbuf3, &endmode, &endgrade);
	move(1, 9);
	prints("[1;33mùÝùùùùùùùßùÝùùùùùûùßùÝùùùùùùùûùÝùùùùùùùßùÝùùùùùûùßùúùùùùùùùû[0m");
	move(2, 9);
	prints("[1;37mùø      ùøùø    ùøùøùø      ùøùø      ùøùø    ùøùøùø      ùø[0m");
	move(3, 9);
	prints("[0;37mùø    ùùùâùø    ùøùøùø  ùúùûùøùãùùùßùÝùåùø    ùøùøùø  ùÝùùùß[0m");
	move(4, 9);
	prints("[0;37mùø    ùùùâùø  ùø  ùøùø  ùüùýùøùÝùùùåùãùßùø  ùø  ùøùø  ùüùýùø[0m");
	move(5, 9);
	prints("[1;37mùø      ùøùø  ùø  ùøùø      ùøùø      ùøùø  ùø  ùøùø      ùø[0m");
	move(6, 9);
	prints("[1;35mùãùùùùùùùåùãùùùüùùùåùãùùùùùùùýùãùùùùùùùåùãùùùüùùùåùüùùùùùùùý[0m");
	move(7, 8);
	prints("[1;31m¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w[41;37m " NICKNAME PIPNAME "µ²§½³ø§i [0;1;31m¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w[0m");
	move(9, 10);
	prints("[1;36m³o­Ó®É¶¡¤£ª¾¤£Ä±¦aÁÙ¬O¨ìÁ{¤F...[0m");
	move(11, 10);
	prints("[1;37m[33m%s[37m ±oÂ÷¶}§Aªº·Å·xÃh©ê¡A¦Û¤v¤@°¦Âû¦b¥~­±¨D¥Í¦s¤F.....[0m", d.name);
	move(13, 10);
	prints("[1;36m¦b§A·ÓÅU±Ð¾É¥Lªº³o¬q®É¥ú¡AÅý¥L±µÄ²¤F«Ü¦h»â°ì¡A°ö¾i¤F«Ü¦hªº¯à¤O....[0m");
	move(15, 10);
	prints("[1;37m¦]¬°³o¨Ç¡AÅý¤pÂû [33m%s[37m ¤§«áªº¥Í¬¡¡AÅÜ±o§ó¦hªö¦h«º¤F........[0m", d.name);
	move(17, 10);
	prints("[1;36m¹ï©ó§AªºÃö¤ß¡A§Aªº¥I¥X¡A§A©Ò¦³ªº·R......[0m");
	move(19, 10);
	prints("[1;37m[33m%s[37m ·|¥Ã»·³£»Ê°O¦b¤ßªº....[0m", d.name);
	vmsg("±µ¤U¨Ó¬Ý¥¼¨Óµo®i");
	clrchyiuan(7, 19);
	move(7, 8);
	prints("[1;34m¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w[44;37m " NICKNAME PIPNAME "¥¼¨Óµo®i [0;1;34m¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w[0m");
	move(9, 10);
	prints("[1;36m³z¹L¤ô´¹²y¡AÅý§Ú­Ì¤@°_¨Ó¬Ý [33m%s[36m ªº¥¼¨Óµo®i§a.....[0m", d.name);
	move(11, 10);
	prints("[1;37m¤pÂû [33m%s[37m «á¨Ó%s....[0m", d.name, endbuf1);
	move(13, 10);
	prints("[1;36m¦]¬°¥Lªº¤§«eªº§V¤O¡A¨Ï±o¥L¦b³o¤@¤è­±%s....[0m", endbuf2);
	move(15, 10);
	prints("[1;37m¦Ü©ó¤pÂûªº±B«Ãª¬ªp¡A¥L«á¨Ó%s¡A±B«Ãºâ¬O«Ü¬üº¡.....[0m", endbuf3);
	move(17, 10);
	prints("[1;36m¶â..³o¬O¤@­Ó¤£¿ùªºµ²§½­ò..........[0m");
	vmsg("§Ú·Q  §A¤@©w«Ü·P°Ê§a.....");
	show_ending_pic(0);
	vmsg("¬Ý¤@¬Ý¤À¼ÆÅo");
	endgrade = pip_game_over(endgrade);
	/*  inmoney(endgrade*10*ba);
	  inexp(endgrade*ba);*/
	sprintf(buf, "/bin/rm %s", get_path(cuser.userid, "chicken"));
	system(buf);
	sprintf(buf, "±o¨ì %d ¤¸,%d ÂI¸gÅç­È", endgrade*10*ba, endgrade*10);
	vmsg(buf);
	vmsg("¤U¤@­¶¬O¤pÂû¸ê®Æ  »°§Öcopy¤U¨Ó°µ¬ö©À");
	pip_data_list(cuser.userid);
	vmsg("Åwªï¦A¨Ó¬D¾Ô....");
	/*°O¿ý¶}©l*/
	now = time(0);
	sprintf(buf, "[1;35m¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w[0m\n");
	pip_log_record(buf);
	sprintf(buf, "[1;37m¦b [33m%s [37mªº®É­Ô¡A[36m%s [37mªº¤pÂû [32m%s[37m ¥X²{¤Fµ²§½[0m\n", Cdate(&now), cuser.userid, d.name);
	pip_log_record(buf);
	sprintf(buf, "[1;37m¤pÂû [32m%s [37m§V¤O¥[±j¦Û¤v¡A«á¨Ó%s[0m\n[1;37m¦]¬°¤§«eªº§V¤O¡A¨Ï±o¦b³o¤@¤è­±%s[0m\n", d.name, endbuf1, endbuf2);
	pip_log_record(buf);
	sprintf(buf, "[1;37m¦Ü©ó±B«Ãª¬ªp¡A¥L«á¨Ó%s¡A±B«Ãºâ¬O«Ü¬üº¡.....[0m\n\n[1;37m¤pÂû [32n%s[37m ªºÁ`¿n¤À¡× [33m%d[0m\n", endbuf3, d.name, endgrade);
	pip_log_record(buf);
	sprintf(buf, "[1;35m¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w[0m\n");
	pip_log_record(buf);
	/*°O¿ý²×¤î*/
	d.death = 3;
	pipdie("[1;31m¹CÀ¸µ²§ôÅo...[m  ", 3);
	return 0;
}

int
pip_ending_decide(endbuf1, endbuf2, endbuf3, endmode, endgrade)
char *endbuf1, *endbuf2, *endbuf3;
int *endmode, *endgrade;
{
	char *name[8][2] = {{"¨kªº", "¤kªº"},
		{"¶ùµ¹¤ý¤l", "°ù¤F¤½¥D"},
		{"¶ùµ¹§A", "°ù¤F§A"},
		{"¶ùµ¹°Ó¤H¢Ï", "°ù¤F¤k°Ó¤H¢Ï"},
		{"¶ùµ¹°Ó¤H¢Ð", "°ù¤F¤k°Ó¤H¢Ð"},
		{"¶ùµ¹°Ó¤H¢Ñ", "°ù¤F¤k°Ó¤H¢Ñ"},
		{"¶ùµ¹°Ó¤H¢Ò", "°ù¤F¤k°Ó¤H¢Ò"},
		{"¶ùµ¹°Ó¤H¢Ó", "°ù¤F¤k°Ó¤H¢Ó"}
	};
	int m = 0, n = 0, grade = 0;
	int modeall_purpose = 0;
	char buf1[256];
	char buf2[256];

	*endmode = pip_future_decide(&modeall_purpose);
	switch (*endmode)
	{
		/*1:·t¶Â 2:ÃÀ³N 3:¸U¯à 4:¾Ô¤h 5:Å]ªk 6:ªÀ¥æ 7:®a¨Æ*/
	case 1:
		pip_endingblack(buf1, &m, &n, &grade);
		break;
	case 2:
		pip_endingart(buf1, &m, &n, &grade);
		break;
	case 3:
		pip_endingall_purpose(buf1, &m, &n, &grade, modeall_purpose);
		break;
	case 4:
		pip_endingcombat(buf1, &m, &n, &grade);
		break;
	case 5:
		pip_endingmagic(buf1, &m, &n, &grade);
		break;
	case 6:
		pip_endingsocial(buf1, &m, &n, &grade);
		break;
	case 7:
		pip_endingfamily(buf1, &m, &n, &grade);
		break;
	}

	grade += pip_marry_decide();
	strcpy(endbuf1, buf1);
	if (n == 1)
	{
		*endgrade = grade + 300;
		sprintf(buf2, "«D±`ªº¶¶§Q..");
	}
	else if (n == 2)
	{
		*endgrade = grade + 100;
		sprintf(buf2, "ªí²{ÁÙ¤£¿ù..");
	}
	else if (n == 3)
	{
		*endgrade = grade - 10;
		sprintf(buf2, "±`¹J¨ì«Ü¦h°ÝÃD....");
	}
	strcpy(endbuf2, buf2);
	if (d.lover >= 1 && d.lover <= 7)
	{
		if (d.sex == 1)
			sprintf(buf2, "%s", name[d.lover][1]);
		else
			sprintf(buf2, "%s", name[d.lover][0]);
	}
	else if (d.lover == 10)
		sprintf(buf2, "%s", buf1);
	else if (d.lover == 0)
	{
		if (d.sex == 1)
			sprintf(buf2, "°ù¤F¦P¦æªº¤k«Ä");
		else
			sprintf(buf2, "¶ùµ¹¤F¦P¦æªº¨k¥Í");
	}
	strcpy(endbuf3, buf2);
	return 0;
}
/*µ²§½§PÂ_*/
/*1:·t¶Â 2:ÃÀ³N 3:¸U¯à 4:¾Ô¤h 5:Å]ªk 6:ªÀ¥æ 7:®a¨Æ*/
int
pip_future_decide(modeall_purpose)
int *modeall_purpose;
{
	int endmode;
	/*·t¶Â*/
	if ((d.etchics == 0 && d.offense >= 100) || (d.etchics > 0 && d.etchics < 50 && d.offense >= 250))
		endmode = 1;
	/*ÃÀ³N*/
	else if (d.art > d.hexp && d.art > d.mexp && d.art > d.hskill && d.art > d.mskill &&
			 d.art > d.social && d.art > d.family && d.art > d.homework && d.art > d.wisdom &&
			 d.art > d.charm && d.art > d.belief && d.art > d.manners && d.art > d.speech &&
			 d.art > d.cookskill && d.art > d.love)
		endmode = 2;
	/*¾Ô°«*/
	else if (d.hexp >= d.social && d.hexp >= d.mexp && d.hexp >= d.family)
	{
		*modeall_purpose = 1;
		if (d.hexp > d.social + 50 || d.hexp > d.mexp + 50 || d.hexp > d.family + 50)
			endmode = 4;
		else
			endmode = 3;
	}
	/*Å]ªk*/
	else if (d.mexp >= d.hexp && d.mexp >= d.social && d.mexp >= d.family)
	{
		*modeall_purpose = 2;
		if (d.mexp > d.hexp || d.mexp > d.social || d.mexp > d.family)
			endmode = 5;
		else
			endmode = 3;
	}
	else if (d.social >= d.hexp && d.social >= d.mexp && d.social >= d.family)
	{
		*modeall_purpose = 3;
		if (d.social > d.hexp + 50 || d.social > d.mexp + 50 || d.social > d.family + 50)
			endmode = 6;
		else
			endmode = 3;
	}

	else
	{
		*modeall_purpose = 4;
		if (d.family > d.hexp + 50 || d.family > d.mexp + 50 || d.family > d.social + 50)
			endmode = 7;
		else
			endmode = 3;
	}
	return endmode;
}
/*µ²±Bªº§PÂ_*/
int
pip_marry_decide()
{
	int grade;
	if (d.lover != 0)
	{
		/* 3 4 5 6 7:°Ó¤H */
		d.lover = d.lover;
		grade = 80;
	}
	else
	{
		if (d.royalJ >= d.relation && d.royalJ >= 100)
		{
			d.lover = 1;  /*¤ý¤l*/
			grade = 200;
		}
		else if (d.relation > d.royalJ && d.relation >= 100)
		{
			d.lover = 2;  /*¤÷¿Ë©Î¥À¿Ë*/
			grade = 0;
		}
		else
		{
			d.lover = 0;
			grade = 40;
		}
	}
	return grade;
}


int
pip_endingblack(buf, m, n, grade) /*·t¶Â*/
char *buf;
int *m, *n, *grade;
{
	if (d.offense >= 500 && d.mexp >= 500) /*Å]¤ý*/
	{
		*m = 1;
		if (d.mexp >= 1000)
			*n = 1;
		else if (d.mexp < 1000 && d.mexp >= 800)
			*n = 2;
		else
			*n = 3;
	}

	else if (d.hexp >= 600)  /*¬yª]*/
	{
		*m = 2;
		if (d.wisdom >= 350)
			*n = 1;
		else if (d.wisdom < 350 && d.wisdom >= 300)
			*n = 2;
		else
			*n = 3;
	}
	else if (d.speech >= 100 && d.art >= 80) /*SM*/
	{
		*m = 3;
		if (d.speech > 150 && d.art >= 120)
			*n = 1;
		else if (d.speech > 120 && d.art >= 100)
			*n = 2;
		else
			*n = 3;
	}
	else if (d.hexp >= 320 && d.character > 200 && d.charm < 200)	/*¶Âµó¦Ñ¤j*/
	{
		*m = 4;
		if (d.hexp >= 400)
			*n = 1;
		else if (d.hexp < 400 && d.hexp >= 360)
			*n = 2;
		else
			*n = 3;
	}
	else if (d.character >= 200 && d.charm >= 200 && d.speech > 70 && d.toman > 70)  /*°ª¯Å±@°ü*/
	{
		*m = 5;
		if (d.charm >= 300)
			*n = 1;
		else if (d.charm < 300 && d.charm >= 250)
			*n = 2;
		else
			*n = 3;
	}

	else if (d.wisdom >= 450)  /*¶BÄF®v*/
	{
		*m = 6;
		if (d.wisdom >= 550)
			*n = 1;
		else if (d.wisdom < 550 && d.wisdom >= 500)
			*n = 2;
		else
			*n = 3;
	}

	else /*¬yÅa*/
	{
		*m = 7;
		if (d.charm >= 350)
			*n = 1;
		else if (d.charm < 350 && d.charm >= 300)
			*n = 2;
		else
			*n = 3;
	}
	if (d.sex == 1)
		strcpy(buf, endmodeblack[*m].boy);
	else
		strcpy(buf, endmodeblack[*m].girl);
	*grade = endmodeblack[*m].grade;
	return 0;
}


int
pip_endingsocial(buf, m, n, grade) /*ªÀ¥æ*/
char *buf;
int *m, *n, *grade;
{
	int class;
	if (d.social > 600) class = 1;
	else if (d.social > 450) class = 2;
	else if (d.social > 380) class = 3;
	else if (d.social > 250) class = 4;
	else class = 5;

	switch (class)
	{
	case 1:
		if (d.charm > 500)
		{
			*m = 1;
			d.lover = 10;
			if (d.character >= 700)
				*n = 1;
			else if (d.character < 700 && d.character >= 500)
				*n = 2;
			else
				*n = 3;
		}
		else
		{
			*m = 2;
			d.lover = 10;
			if (d.character >= 700)
				*n = 1;
			else if (d.character < 700 && d.character >= 500)
				*n = 2;
			else
				*n = 3;
		}
		break;

	case 2:
		*m = 1;
		d.lover = 10;
		if (d.character >= 700)
			*n = 1;
		else if (d.character < 700 && d.character >= 500)
			*n = 2;
		else
			*n = 3;
		break;

	case 3:
		if (d.character >= d.charm)
		{
			*m = 3;
			d.lover = 10;
			if (d.toman >= 250)
				*n = 1;
			else if (d.toman < 250 && d.toman >= 200)
				*n = 2;
			else
				*n = 3;
		}
		else
		{
			*m = 4;
			d.lover = 10;
			if (d.character >= 400)
				*n = 1;
			else if (d.character < 400 && d.character >= 300)
				*n = 2;
			else
				*n = 3;
		}
		break;

	case 4:
		if (d.wisdom >= d.affect)
		{
			*m = 5;
			d.lover = 10;
			if (d.toman > 120 && d.cookskill > 300 && d.homework > 300)
				*n = 1;
			else if (d.toman < 120 && d.cookskill < 300 && d.homework < 300 && d.toman > 100 && d.cookskill > 250 && d.homework > 250)
				*n = 2;
			else
				*n = 3;
		}
		else
		{
			*m = 6;
			d.lover = 10;
			if (d.hp >= 400)
				*n = 1;
			else if (d.hp < 400 && d.hp >= 300)
				*n = 2;
			else
				*n = 3;
		}
		break;

	case 5:
		*m = 7;
		d.lover = 10;
		if (d.charm >= 200)
			*n = 1;
		else if (d.charm < 200 && d.charm >= 100)
			*n = 2;
		else
			*n = 3;
		break;
	}
	if (d.sex == 1)
		strcpy(buf, endmodesocial[*m].boy);
	else
		strcpy(buf, endmodesocial[*m].girl);
	*grade = endmodesocial[*m].grade;
	return 0;
}

int
pip_endingmagic(buf, m, n, grade) /*Å]ªk*/
char *buf;
int *m, *n, *grade;
{
	int class;
	if (d.mexp > 800) class = 1;
	else if (d.mexp > 600) class = 2;
	else if (d.mexp > 500) class = 3;
	else if (d.mexp > 300) class = 4;
	else class = 5;

	switch (class)
	{
	case 1:
		if (d.affect > d.wisdom && d.affect > d.belief && d.etchics > 100)
		{
			*m = 1;
			if (d.etchics >= 800)
				*n = 1;
			else if (d.etchics < 800 && d.etchics >= 400)
				*n = 2;
			else
				*n = 3;
		}
		else if (d.etchics < 50)
		{
			*m = 4;
			if (d.hp >= 400)
				*n = 1;
			else if (d.hp < 400 && d.hp >= 200)
				*n = 2;
			else
				*n = 3;
		}
		else
		{
			*m = 2;
			if (d.wisdom >= 800)
				*n = 1;
			else if (d.wisdom < 800 && d.wisdom >= 400)
				*n = 2;
			else
				*n = 3;
		}
		break;

	case 2:
		if (d.etchics >= 50)
		{
			*m = 3;
			if (d.wisdom >= 500)
				*n = 1;
			else if (d.wisdom < 500 && d.wisdom >= 200)
				*n = 2;
			else
				*n = 3;
		}
		else
		{
			*m = 4;
			if (d.hp >= 400)
				*n = 1;
			else if (d.hp < 400 && d.hp >= 200)
				*n = 2;
			else
				*n = 3;
		}
		break;

	case 3:
		*m = 5;
		if (d.mskill >= 300)
			*n = 1;
		else if (d.mskill < 300 && d.mskill >= 150)
			*n = 2;
		else
			*n = 3;
		break;

	case 4:
		*m = 6;
		if (d.speech >= 150)
			*n = 1;
		else if (d.speech < 150 && d.speech >= 60)
			*n = 2;
		else
			*n = 3;
		break;

	case 5:
		if (d.character >= 200)
		{
			*m = 7;
			if (d.speech >= 150)
				*n = 1;
			else if (d.speech < 150 && d.speech >= 60)
				*n = 2;
			else
				*n = 3;
		}
		else
		{
			*m = 8;
			if (d.speech >= 150)
				*n = 1;
			else if (d.speech < 150 && d.speech >= 60)
				*n = 2;
			else
				*n = 3;
		}
		break;

	}

	if (d.sex == 1)
		strcpy(buf, endmodemagic[*m].boy);
	else
		strcpy(buf, endmodemagic[*m].girl);
	*grade = endmodemagic[*m].grade;
	return 0;
}

int
pip_endingcombat(buf, m, n, grade) /*¾Ô°«*/
char *buf;
int *m, *n, *grade;
{
	int class;
	if (d.hexp > 1500) class = 1;
	else if (d.hexp > 1000) class = 2;
	else if (d.hexp > 800) class = 3;
	else class = 4;

	switch (class)
	{
	case 1:
		if (d.affect > d.wisdom && d.affect > d.belief && d.etchics > 100)
		{
			*m = 1;
			if (d.etchics >= 800)
				*n = 1;
			else if (d.etchics < 800 && d.etchics >= 400)
				*n = 2;
			else
				*n = 3;
		}
		else if (d.etchics < 50)
		{

		}
		else
		{
			*m = 2;
			if (d.wisdom >= 800)
				*n = 1;
			else if (d.wisdom < 800 && d.wisdom >= 400)
				*n = 2;
			else
				*n = 3;
		}
		break;

	case 2:
		if (d.character >= 300 && d.etchics > 50)
		{
			*m = 3;
			if (d.etchics >= 300 && d.charm >= 300)
				*n = 1;
			else if (d.etchics < 300 && d.charm < 300 && d.etchics >= 250 && d.charm >= 250)
				*n = 2;
			else
				*n = 3;
		}
		else if (d.character < 300 && d.etchics > 50)
		{
			*m = 4;
			if (d.speech >= 200)
				*n = 1;
			else if (d.speech < 150 && d.speech >= 80)
				*n = 2;
			else
				*n = 3;
		}
		else
		{
			*m = 7;
			if (d.hp >= 400)
				*n = 1;
			else if (d.hp < 400 && d.hp >= 200)
				*n = 2;
			else
				*n = 3;
		}
		break;

	case 3:
		if (d.character >= 400 && d.etchics > 50)
		{
			*m = 5;
			if (d.etchics >= 300)
				*n = 1;
			else if (d.etchics < 300 && d.etchics >= 150)
				*n = 2;
			else
				*n = 3;
		}
		else if (d.character < 400 && d.etchics > 50)
		{
			*m = 4;
			if (d.speech >= 200)
				*n = 1;
			else if (d.speech < 150 && d.speech >= 80)
				*n = 2;
			else
				*n = 3;
		}
		else
		{
			*m = 7;
			if (d.hp >= 400)
				*n = 1;
			else if (d.hp < 400 && d.hp >= 200)
				*n = 2;
			else
				*n = 3;
		}
		break;

	case 4:
		if (d.etchics >= 50)
		{
			*m = 6;
		}
		else
		{
			*m = 8;
		}
		if (d.hskill >= 100)
			*n = 1;
		else if (d.hskill < 100 && d.hskill >= 80)
			*n = 2;
		else
			*n = 3;
		break;
	}

	if (d.sex == 1)
		strcpy(buf, endmodecombat[*m].boy);
	else
		strcpy(buf, endmodecombat[*m].girl);
	*grade = endmodecombat[*m].grade;
	return 0;
}


int
pip_endingfamily(buf, m, n, grade) /*®a¨Æ*/
char *buf;
int *m, *n, *grade;
{
	*m = 1;
	if (d.charm >= 200)
		*n = 1;
	else if (d.charm < 200 && d.charm > 100)
		*n = 2;
	else
		*n = 3;

	if (d.sex == 1)
		strcpy(buf, endmodefamily[*m].boy);
	else
		strcpy(buf, endmodefamily[*m].girl);
	*grade = endmodefamily[*m].grade;
	return 0;
}


int
pip_endingall_purpose(buf, m, n, grade, mode) /*¸U¯à*/
char *buf;
int *m, *n, *grade;
int mode;
{
	int data;
	int class;
	int num = 0;

	if (mode == 1)
		data = d.hexp;
	else if (mode == 2)
		data = d.mexp;
	else if (mode == 3)
		data = d.social;
	else if (mode == 4)
		data = d.family;
	if (class > 1000) class = 1;
	else if (class > 800) class = 2;
	else if (class > 500) class = 3;
	else if (class > 300) class = 4;
	else class = 5;

	data = pip_max_worktime(&num);
	switch (class)
	{
	case 1:
		if (d.character >= 1000)
		{
			*m = 1;
			if (d.etchics >= 900)
				*n = 1;
			else if (d.etchics < 900 && d.etchics >= 600)
				*n = 2;
			else
				*n = 3;
		}
		else
		{
			*m = 2;
			if (d.etchics >= 650)
				*n = 1;
			else if (d.etchics < 650 && d.etchics >= 400)
				*n = 2;
			else
				*n = 3;
		}
		break;

	case 2:
		if (d.belief > d.etchics && d.belief > d.wisdom)
		{
			*m = 3;
			if (d.etchics >= 500)
				*n = 1;
			else if (d.etchics < 500 && d.etchics >= 250)
				*n = 2;
			else
				*n = 3;
		}
		else if (d.etchics > d.belief && d.etchics > d.wisdom)
		{
			*m = 4;
			if (d.wisdom >= 800)
				*n = 1;
			else if (d.wisdom < 800 && d.wisdom >= 600)
				*n = 2;
			else
				*n = 3;
		}
		else
		{
			*m = 5;
			if (d.affect >= 800)
				*n = 1;
			else if (d.affect < 800 && d.affect >= 400)
				*n = 2;
			else
				*n = 3;
		}
		break;

	case 3:
		if (d.belief > d.etchics && d.belief > d.wisdom)
		{
			*m = 6;
			if (d.belief >= 400)
				*n = 1;
			else if (d.belief < 400 && d.belief >= 150)
				*n = 2;
			else
				*n = 3;
		}
		else if (d.etchics > d.belief && d.etchics > d.wisdom)
		{
			*m = 7;
			if (d.wisdom >= 700)
				*n = 1;
			else if (d.wisdom < 700 && d.wisdom >= 400)
				*n = 2;
			else
				*n = 3;
		}
		else
		{
			*m = 8;
			if (d.affect >= 800)
				*n = 1;
			else if (d.affect < 800 && d.affect >= 400)
				*n = 2;
			else
				*n = 3;
		}
		break;

	case 4:
		if (num >= 2)
		{
			*m = 8 + num;
			switch (num)
			{
			case 2:
				if (d.love > 100)	*n = 1;
				else if (d.love > 50) *n = 2;
				else *n = 3;
				break;
			case 3:
				if (d.homework > 100) *n = 1;
				else if (d.homework > 50) *n = 2;
				else *n = 3;
				break;
			case 4:
				if (d.hp > 600) *n = 1;
				else if (d.hp > 300) *n = 2;
				else *n = 3;
				break;
			case 5:
				if (d.cookskill > 200) *n = 1;
				else if (d.cookskill > 100) *n = 2;
				else *n = 3;
				break;
			case 6:
				if ((d.belief + d.etchics) > 600) *n = 1;
				else if ((d.belief + d.etchics) > 200) *n = 2;
				else *n = 3;
				break;
			case 7:
				if (d.speech > 150) *n = 1;
				else if (d.speech > 50) *n = 2;
				else *n = 3;
				break;
			case 8:
				if ((d.hp + d.wrist) > 900) *n = 1;
				else if ((d.hp + d.wrist) > 600) *n = 2;
				else *n = 3;
				break;
			case 9:
			case 11:
				if (d.art > 250) *n = 1;
				else if (d.art > 100) *n = 2;
				else *n = 3;
				break;
			case 10:
				if (d.hskill > 250) *n = 1;
				else if (d.hskill > 100) *n = 2;
				else *n = 3;
				break;
			case 12:
				if (d.belief > 500) *n = 1;
				else if (d.belief > 200) *n = 2;
				else *n = 3;
				break;
			case 13:
				if (d.wisdom > 500) *n = 1;
				else if (d.wisdom > 200) *n = 2;
				else *n = 3;
				break;
			case 14:
			case 16:
				if (d.charm > 1000) *n = 1;
				else if (d.charm > 500) *n = 2;
				else *n = 3;
				break;
			case 15:
				if (d.charm > 700) *n = 1;
				else if (d.charm > 300) *n = 2;
				else *n = 3;
				break;
			}
		}
		else
		{
			*m = 9;
			if (d.etchics > 400)
				*n = 1;
			else if (d.etchics > 200)
				*n = 2;
			else
				*n = 3;
		}
		break;
	case 5:
		if (num >= 2)
		{
			*m = 24 + num;
			switch (num)
			{
			case 2:
			case 3:
				if (d.hp > 400) *n = 1;
				else if (d.hp > 150) *n = 2;
				else *n = 3;
				break;
			case 4:
			case 10:
			case 11:
				if (d.hp > 600) *n = 1;
				else if (d.hp > 300) *n = 2;
				else *n = 3;
				break;
			case 5:
				if (d.cookskill > 150) *n = 1;
				else if (d.cookskill > 80) *n = 2;
				else *n = 3;
				break;
			case 6:
				if ((d.belief + d.etchics) > 600) *n = 1;
				else if ((d.belief + d.etchics) > 200) *n = 2;
				else *n = 3;
				break;
			case 7:
				if (d.speech > 150) *n = 1;
				else if (d.speech > 50) *n = 2;
				else *n = 3;
				break;
			case 8:
				if ((d.hp + d.wrist) > 700) *n = 1;
				else if ((d.hp + d.wrist) > 300) *n = 2;
				else *n = 3;
				break;
			case 9:
				if (d.art > 100) *n = 1;
				else if (d.art > 50) *n = 2;
				else *n = 3;
				break;
			case 12:
				if (d.hp > 300) *n = 1;
				else if (d.hp > 150) *n = 2;
				else *n = 3;
				break;
			case 13:
				if (d.speech > 100) *n = 1;
				else if (d.speech > 40) *n = 2;
				else *n = 3;
				break;
			case 14:
			case 16:
				if (d.charm > 1000) *n = 1;
				else if (d.charm > 500) *n = 2;
				else *n = 3;
				break;
			case 15:
				if (d.charm > 700) *n = 1;
				else if (d.charm > 300) *n = 2;
				else *n = 3;
				break;
			}
		}
		else
		{
			*m = 25;
			if (d.relation > 100)
				*n = 1;
			else if (d.relation > 50)
				*n = 2;
			else
				*n = 3;
		}
		break;
	}

	if (d.sex == 1)
		strcpy(buf, endmodeall_purpose[*m].boy);
	else
		strcpy(buf, endmodeall_purpose[*m].girl);
	*grade = endmodeall_purpose[*m].grade;
	return 0;
}

int
pip_endingart(buf, m, n, grade) /*ÃÀ³N*/
char *buf;
int *m, *n, *grade;
{
	if (d.speech >= 100)
	{
		*m = 1;
		if (d.hp >= 300 && d.affect >= 350)
			*n = 1;
		else if (d.hp < 300 && d.affect < 350 && d.hp >= 250 && d.affect >= 300)
			*n = 2;
		else
			*n = 3;
	}
	else if (d.wisdom >= 400)
	{
		*m = 2;
		if (d.affect >= 500)
			*n = 1;
		else if (d.affect < 500 && d.affect >= 450)
			*n = 2;
		else
			*n = 3;
	}
	else if (d.classI >= d.classJ)
	{
		*m = 3;
		if (d.affect >= 350)
			*n = 1;
		else if (d.affect < 350 && d.affect >= 300)
			*n = 2;
		else
			*n = 3;
	}
	else
	{
		*m = 4;
		if (d.affect >= 200 && d.hp > 150)
			*n = 1;
		else if (d.affect < 200 && d.affect >= 180 && d.hp > 150)
			*n = 2;
		else
			*n = 3;
	}
	if (d.sex == 1)
		strcpy(buf, endmodeart[*m].boy);
	else
		strcpy(buf, endmodeart[*m].girl);
	*grade = endmodeart[*m].grade;
	return 0;
}

int
pip_max_worktime(num)
int *num;
{
	int data = 20;
	if (d.workA > data)
	{
		data = d.workA;
		*num = 1;
	}
	if (d.workB > data)
	{
		data = d.workB;
		*num = 2;
	}
	if (d.workC > data)
	{
		data = d.workC;
		*num = 3;
	}
	if (d.workD > data)
	{
		data = d.workD;
		*num = 4;
	}
	if (d.workE > data)
	{
		data = d.workE;
		*num = 5;
	}

	if (d.workF > data)
	{
		data = d.workF;
		*num = 6;
	}
	if (d.workG > data)
	{
		data = d.workG;
		*num = 7;
	}
	if (d.workH > data)
	{
		data = d.workH;
		*num = 8;
	}
	if (d.workI > data)
	{
		data = d.workI;
		*num = 9;
	}
	if (d.workJ > data)
	{
		data = d.workJ;
		*num = 10;
	}
	if (d.workK > data)
	{
		data = d.workK;
		*num = 11;
	}
	if (d.workL > data)
	{
		data = d.workL;
		*num = 12;
	}
	if (d.workM > data)
	{
		data = d.workM;
		*num = 13;
	}
	if (d.workN > data)
	{
		data = d.workN;
		*num = 14;
	}
	if (d.workO > data)
	{
		data = d.workO;
		*num = 16;
	}
	if (d.workP > data)
	{
		data = d.workP;
		*num = 16;
	}

	return data;
}

int pip_game_over(endgrade)
int endgrade;
{
	long gradebasic;
	long gradeall;

	gradebasic = (d.maxhp + d.wrist + d.wisdom + d.character + d.charm + d.etchics + d.belief + d.affect) / 10 - d.offense;
	clrchyiuan(1, 23);
	gradeall = gradebasic + endgrade;
	move(8, 17);
	prints("[1;36m·PÁÂ±zª±§¹¾ã­Ó" NICKNAME "¤pÂûªº¹CÀ¸.....[0m");
	move(10, 17);
	prints("[1;37m¸g¹L¨t²Î­pºâªºµ²ªG¡G[0m");
	move(12, 17);
	prints("[1;36m±zªº¤pÂû [37m%s [36mÁ`±o¤À¡× [1;5;33m%d [0m", d.name, gradeall);
	return gradeall;
}

int pip_divine() /*¥e¤R®v¨Ó³X*/
{
	char buf[256];
	char ans[4];
	char endbuf1[50];
	char endbuf2[50];
	char endbuf3[50];
	int endgrade = 0;
	int endmode = 0;
	long money;
	int tm;
	int randvalue;

	tm = d.bbtime / 60 / 30;
	move(b_lines - 2, 0);
	money = 300 * (tm + 1);
	clrchyiuan(6, 18);
	move(10, 14);
	prints("[1;33;5m¥n¥n¥n...[0;1;37m¬ðµM¶Ç¨Ó°}°}ªººVªùÁn.........[0m");
	vmsg("¥hÁ@Á@¬O½Ö§a......");
	clrchyiuan(6, 18);
	move(10, 14);
	prints("[1;37;46m    ­ì¨Ó¬O¶³¹C¥|®üªº¥e¤R®v¨Ó³X¤F.......    [0m");
	vmsg("¶}ªùÅý¥L¶i¨Ó§a....");
	if (d.money >= money)
	{
		randvalue = rand() % 5;
		sprintf(buf, "§A­n¥e¤R¶Ü? ­nªá%d¤¸³á...[y/N]: ", money);
		getdata(12, 14, buf, ans, 2, 1, 0);
		if (ans[0] == 'y' || ans[0] == 'Y')
		{
			pip_ending_decide(endbuf1, endbuf2, endbuf3, &endmode, &endgrade);
			if (randvalue == 0)
				sprintf(buf, "[1;37m  §Aªº¤pÂû%s¥H«á¥i¯àªº¨­¥÷¬O%s  [m", d.name, endmodemagic[2+rand()%5].girl);
			else if (randvalue == 1)
				sprintf(buf, "[1;37m  §Aªº¤pÂû%s¥H«á¥i¯àªº¨­¥÷¬O%s  [m", d.name, endmodecombat[2+rand()%6].girl);
			else if (randvalue == 2)
				sprintf(buf, "[1;37m  §Aªº¤pÂû%s¥H«á¥i¯àªº¨­¥÷¬O%s  [m", d.name, endmodeall_purpose[6+rand()%15].girl);
			else if (randvalue == 3)
				sprintf(buf, "[1;37m  §Aªº¤pÂû%s¥H«á¥i¯àªº¨­¥÷¬O%s  [m", d.name, endmodeart[2+rand()%6].girl);
			else if (randvalue == 4)
				sprintf(buf, "[1;37m  §Aªº¤pÂû%s¥H«á¥i¯àªº¨­¥÷¬O%s  [m", d.name, endbuf1);
			d.money -= money;
			clrchyiuan(6, 18);
			move(10, 14);
			prints("[1;33m¦b§Ú¥e¤Rµ²ªG¬Ý¨Ó....[m");
			move(12, 14);
			prints(buf);
			vmsg("ÁÂÁÂ´fÅU¡A¦³½t¦A¨£­±¤F.(¤£·Ç¤£¯à©Ç§Ú³á)");
		}
		else
		{
			vmsg("§A¤£·Q¥e¤R°Ú?..¯u¥i±¤..¨º¥u¦³µ¥¤U¦¸§a...");
		}
	}
	else
	{
		vmsg("§Aªº¿ú¤£°÷³á..¯u¬O¥i±¤..µ¥¤U¦¸§a...");
	}
	return 0;
}

int
pip_money()
{
	char buf[100], ans[10];

	int money = -1;
	if (!d.name[0] || d.death) return 0;
	clrchyiuan(6, 18);
	/*  move(12,0);
	  clrtobot();*/
	prints("§A¨­¤W¦³ %d ¦¸ÂIºq¦¸¼Æ,Âûª÷ %d ¤¸\n", cuser.request, d.money);
	outs("\n¤@¦¸´«¤@¤dÂûª÷­ò!!\n");
	while (money < 0 || money > cuser.request)
	{
		getdata(10, 0, "­n´«¦h¤Ö¦¸? ", ans, 10, LCECHO, 0);
		if (!ans[0]) return 0;
		money = atol(ans);
	}
	sprintf(buf, "¬O§_­nÂà´« %d ¦¸ ¬° %d Âûª÷? [y/N]: ", money, money*1000);
	getdata(11, 0, buf, ans, 3, LCECHO, 0);
	if (ans[0] == 'y' || ans[0] == 'Y')
	{
		ACCT acct;
		acct_load(&acct, cuser.userid);
		/*    demoney(money);*/
		d.money += (money * 1000);
		cuser.request -= money;
		acct.request = cuser.request;
		acct_save(&acct);
		pip_write_file();
		sprintf(buf, "§A¨­¤W¦³ %d ¦¸ÂIºq¦¸¼Æ,Âûª÷ %d ¤¸", cuser.request, d.money);
	}
	else
		sprintf(buf, "¨ú®ø.....");

	vmsg(buf);
	return 0;
}

/* ±NÂûª÷´«¦¨ÂIºq¦¸¼Æ by statue */
#if 0
int
pip_request()
{
	char buf[100], ans[10];

	int money = -1;
	if (!d.name[0] || d.death) return;
	clrchyiuan(6, 18);
	/*  move(12,0);
	  clrtobot();*/
	prints("§A¨­¤W¦³ %d ¦¸ÂIºq¦¸¼Æ,Âûª÷ %d ¤¸\n", cuser.request, d.money);

	if (d.money <= 5000)
	{
		sprintf(buf, "Âûª÷¥²¶·¤j©ó 5000 ¤~¥iÂà´«......");
		vmsg(buf);
		return 0;
	}

	outs("\n¤­¤dÂûª÷´«¤@¦¸­ò!!\n");
	while (money < 0 || (money)*5000 > d.money - START_MONEY)
	{
		getdata(10, 0, "­n´«¦h¤Ö¦¸? ", ans, 10, LCECHO, 0);
		if (!ans[0]) return;
		money = atol(ans);
	}
	sprintf(buf, "¬O§_­nÂà´« %d Âûª÷ ¬° %d ¦¸? [y/N]: ", money*5000, money);
	getdata(11, 0, buf, ans, 3, LCECHO, 0);
	if (ans[0] == 'y' || ans[0] == 'Y')
	{
		ACCT acct;
		acct_load(&acct, cuser.userid);
		/*    demoney(money);*/
		d.money -= (money * 5000);
		cuser.request += money;
		acct.request = cuser.request;
		acct_save(&acct);
		pip_write_file();
		sprintf(buf, "§A¨­¤W¦³ %d ¦¸ÂIºq¦¸¼Æ,Âûª÷ %d ¤¸", cuser.request, d.money);
	}
	else
		sprintf(buf, "¨ú®ø.....");

	vmsg(buf);
	return 0;
}
#endif

int pip_query()  /*«ô³X¤pÂû*/
{
	int id;
	char genbuf[STRLEN];

	vs_bar("«ô³X¦P¦ñ");
	usercomplete(msg_uid, genbuf);
	if (genbuf[0])
	{
		move(2, 0);
		if (id = acct_userno(genbuf))
		{
			pip_read(genbuf);
			vmsg("Æ[¼¯¤@¤U§O¤Hªº¤pÂû...:p");
		}
		else
		{
			outs(err_uid);
			clrtoeol();
		}
	}
	return 0;
}

int
pip_read(genbuf)
char *genbuf;
{
	FILE *fs;
	char buf[200];
	/*char yo[14][5]={"½Ï¥Í","À¦¨à","¥®¨à","¨àµ£","«C¦~","¤Ö¦~","¦¨¦~",
	                "§§¦~","§§¦~","§§¦~","§ó¦~","¦Ñ¦~","¦Ñ¦~","¥jµ}"};*/
	char yo[12][5] = {"½Ï¥Í", "À¦¨à", "¥®¨à", "¨àµ£", "¤Ö¦~", "«C¦~",
					  "¦¨¦~", "§§¦~", "§ó¦~", "¦Ñ¦~", "¥jµ}", "¯«¥P"
					 };
	int pc1, age1, age = 0;

	int year1, month1, day1, sex1, death1, nodone1, relation1, liveagain1, chickenmode1, level1, exp1, dataE1;
	int hp1, maxhp1, weight1, tired1, sick1, shit1, wrist1, bodyA1, bodyB1, bodyC1, bodyD1, bodyE1;
	int social1, family1, hexp1, mexp1, tmpA1, tmpB1, tmpC1, tmpD1, tmpE1;
	int mp1, maxmp1, attack1, resist1, speed1, hskill1, mskill1, mresist1, magicmode1, specialmagic1, fightC1, fightD1, fightE1;
	int weaponhead1, weaponrhand1, weaponlhand1, weaponbody1, weaponfoot1, weaponA1, weaponB1, weaponC1, weaponD1, weaponE1;
	int toman1, character1, love1, wisdom1, art1, etchics1, brave1, homework1, charm1, manners1, speech1, cookskill1, learnA1, learnB1, learnC1, learnD1, learnE1;
	int happy1, satisfy1, fallinlove1, belief1, offense1, affect1, stateA1, stateB1, stateC1, stateD1, stateE1;
	int food1, medicine1, bighp1, cookie1, ginseng1, snowgrass1, eatC1, eatD1, eatE1;
	int book1, playtool1, money1, thingA1, thingB1, thingC1, thingD1, thingE1;
	int winn1, losee1;
	int royalA1, royalB1, royalC1, royalD1, royalE1, royalF1, royalG1, royalH1, royalI1, royalJ1, seeroyalJ1, seeA1, seeB1, seeC1, seeD1, seeE1;
	int wantend1, lover1;
	char name1[200];
	int classA1, classB1, classC1, classD1, classE1;
	int classF1, classG1, classH1, classI1, classJ1;
	int classK1, classL1, classM1, classN1, classO1;
	int workA1, workB1, workC1, workD1, workE1;
	int workF1, workG1, workH1, workI1, workJ1;
	int workK1, workL1, workM1, workN1, workO1;
	int workP1, workQ1, workR1, workS1, workT1;
	int workU1, workV1, workW1, workX1, workY1, workZ1;

	/*  sprintf(buf,"home/%s/chicken",genbuf);*/
	usr_fpath(buf, genbuf, "chicken");
	/*  currutmp->destuid = genbuf;*/

	if (fs = fopen(buf, "r"))
	{
		fgets(buf, 80, fs);
		age = ((time_t) atol(buf)) / 60 / 30;

		if (age == 0) /*½Ï¥Í*/
			age1 = 0;
		else if (age == 1) /*À¦¨à*/
			age1 = 1;
		else if (age >= 2 && age <= 5) /*¥®¨à*/
			age1 = 2;
		else if (age >= 6 && age <= 12) /*¨àµ£*/
			age1 = 3;
		else if (age >= 13 && age <= 15) /*¤Ö¦~*/
			age1 = 4;
		else if (age >= 16 && age <= 18) /*«C¦~*/
			age1 = 5;
		else if (age >= 19 && age <= 35) /*¦¨¦~*/
			age1 = 6;
		else if (age >= 36 && age <= 45) /*§§¦~*/
			age1 = 7;
		else if (age >= 45 && age <= 60) /*§ó¦~*/
			age1 = 8;
		else if (age >= 60 && age <= 70) /*¦Ñ¦~*/
			age1 = 9;
		else if (age >= 70 && age <= 100) /*¥jµ}*/
			age1 = 10;
		else if (age > 100) /*¯«¥P*/
			age1 = 11;

		fscanf(fs,
			   "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %s %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
			   &(year1), &(month1), &(day1), &(sex1), &(death1), &(nodone1), &(relation1), &(liveagain1), &(chickenmode1), &(level1), &(exp1), &(dataE1),
			   &(hp1), &(maxhp1), &(weight1), &(tired1), &(sick1), &(shit1), &(wrist1), &(bodyA1), &(bodyB1), &(bodyC1), &(bodyD1), &(bodyE1),
			   &(social1), &(family1), &(hexp1), &(mexp1), &(tmpA1), &(tmpB1), &(tmpC1), &(tmpD1), &(tmpE1),
			   &(mp1), &(maxmp1), &(attack1), &(resist1), &(speed1), &(hskill1), &(mskill1), &(mresist1), &(magicmode1), &(specialmagic1), &(fightC1), &(fightD1), &(fightE1),
			   &(weaponhead1), &(weaponrhand1), &(weaponlhand1), &(weaponbody1), &(weaponfoot1), &(weaponA1), &(weaponB1), &(weaponC1), &(weaponD1), &(weaponE1),
			   &(toman1), &(character1), &(love1), &(wisdom1), &(art1), &(etchics1), &(brave1), &(homework1), &(charm1), &(manners1), &(speech1), &(cookskill1), &(learnA1), &(learnB1), &(learnC1), &(learnD1), &(learnE1),
			   &(happy1), &(satisfy1), &(fallinlove1), &(belief1), &(offense1), &(affect1), &(stateA1), &(stateB1), &(stateC1), &(stateD1), &(stateE1),
			   &(food1), &(medicine1), &(bighp1), &(cookie1), &(ginseng1), &(snowgrass1), &(eatC1), &(eatD1), &(eatE1),
			   &(book1), &(playtool1), &(money1), &(thingA1), &(thingB1), &(thingC1), &(thingD1), &(thingE1),
			   &(winn1), &(losee1),
			   &(royalA1), &(royalB1), &(royalC1), &(royalD1), &(royalE1), &(royalF1), &(royalG1), &(royalH1), &(royalI1), &(royalJ1), &(seeroyalJ1), &(seeA1), &(seeB1), &(seeC1), &(seeD1), &(seeE1),
			   &(wantend1), &(lover1),
			   name1,
			   &(classA1), &(classB1), &(classC1), &(classD1), &(classE1),
			   &(classF1), &(classG1), &(classH1), &(classI1), &(classJ1),
			   &(classK1), &(classL1), &(classM1), &(classN1), &(classO1),
			   &(workA1), &(workB1), &(workC1), &(workD1), &(workE1),
			   &(workF1), &(workG1), &(workH1), &(workI1), &(workJ1),
			   &(workK1), &(workL1), &(workM1), &(workN1), &(workO1),
			   &(workP1), &(workQ1), &(workR1), &(workS1), &(workT1),
			   &(workU1), &(workV1), &(workW1), &(workX1), &(workY1), &(workZ1)
			  );
		fclose(fs);

		move(1, 0);
		clrtobot();
		prints("³o¬O%s¾iªº¤pÂû¡G\n", genbuf);

		if (death1 == 0)
		{
			prints("[1;32mName¡G%-10s[m  ¥Í¤é¡G%2d¦~%2d¤ë%2d¤é   ¦~ÄÖ¡G%2d·³  ª¬ºA¡G%s  ¿ú¿ú¡G%d\n"
				   "¥Í©R¡G%3d/%-3d  §Ö¼Ö¡G%-4d  º¡·N¡G%-4d  ®ð½è¡G%-4d  ´¼¼z¡G%-4d  Åé­«¡G%-4d\n"
				   "¤j¸É¤Y¡G%-4d   ­¹ª«¡G%-4d  ¹s­¹¡G%-4d  ¯h³Ò¡G%-4d  Å¼Å¼¡G%-4d  ¯f®ð¡G%-4d\n",
				   name1, year1 - 11, month1, day1, age, yo[age1], money1,
				   hp1, maxhp1, happy1, satisfy1, character1, wisdom1, weight1,
				   bighp1, food1, cookie1, tired1, shit1, sick1);

			move(5, 0);
			switch (age1)
			{
			case 0:
			case 1:
			case 2:
				if (weight1 <= (60 + 10*age - 30))
					show_basic_pic(1);
				else if (weight1 > (60 + 10*age - 30) && weight1 < (60 + 10*age + 30))
					show_basic_pic(2);
				else if (weight1 >= (60 + 10*age + 30))
					show_basic_pic(3);
				break;
			case 3:
			case 4:
				if (weight1 <= (60 + 10*age - 30))
					show_basic_pic(4);
				else if (weight1 > (60 + 10*age - 30) && weight1 < (60 + 10*age + 30))
					show_basic_pic(5);
				else if (weight1 >= (60 + 10*age + 30))
					show_basic_pic(6);
				break;
			case 5:
			case 6:
				if (weight1 <= (60 + 10*age - 30))
					show_basic_pic(7);
				else if (weight1 > (60 + 10*age - 30) && weight1 < (60 + 10*age + 30))
					show_basic_pic(8);
				else if (weight1 >= (60 + 10*age + 30))
					show_basic_pic(9);
				break;
			case 7:
			case 8:
				if (weight1 <= (60 + 10*age - 30))
					show_basic_pic(10);
				else if (weight1 > (60 + 10*age - 30) && weight1 < (60 + 10*age + 30))
					show_basic_pic(11);
				else if (weight1 >= (60 + 10*age + 30))
					show_basic_pic(12);
				break;
			case 9:
				show_basic_pic(13);
				break;
			case 10:
			case 11:
				show_basic_pic(13);
				break;
			}
			move(18, 0);
			if (shit1 == 0) prints("«Ü°®²b..");
			if (shit1 > 40 && shit1 < 60) prints("¯ä¯äªº..");
			if (shit1 >= 60 && shit1 < 80) prints("¦n¯ä³á..");
			if (shit1 >= 80 && shit1 < 100) prints("[1;34m§Ö¯ä¦º¤F..[m");
			if (shit1 >= 100) {prints("[1;31m¯ä¦º¤F..[m"); return -1;}

			pc1 = hp1 * 100 / maxhp1;
			if (pc1 == 0) {prints("¾j¦º¤F.."); return -1;}
			if (pc1 < 20) prints("[1;35m¥þ¨­µL¤O¤¤.§Ö¾j¦º¤F.[m");
			if (pc1 < 40 && pc1 >= 20) prints("Åé¤O¤£¤Ó°÷..·Q¦YÂIªF¦è..");
			if (pc1 < 100 && pc1 >= 80) prints("¶â¡ã¨{¤l¹¡¹¡¦³Åé¤O..");
			if (pc1 >= 100) prints("[1;34m§Ö¼µ¦º¤F..[m");

			pc1 = tired1;
			if (pc1 < 20) prints("ºë¯«§Ý§Ý¤¤..");
			if (pc1 < 80 && pc1 >= 60) prints("[1;34m¦³ÂI¤p²Ö..[m");
			if (pc1 < 100 && pc1 >= 80) {prints("[1;31m¦n²Ö³á¡A§Ö¤£¦æ¤F..[m"); }
			if (pc1 >= 100) {prints("²Ö¦º¤F..."); return -1;}

			pc1 = 60 + 10 * age;
			if (weight1 < (pc1 + 30) && weight1 >= (pc1 + 10)) prints("¦³ÂI¤p­D..");
			if (weight1 < (pc1 + 50) && weight1 >= (pc1 + 30)) prints("¤Ó­D¤F..");
			if (weight1 > (pc1 + 50)) {prints("­D¦º¤F..."); return -1;}

			if (weight1 < (pc1 - 50)) {prints("½G¦º¤F.."); return -1;}
			if (weight1 > (pc1 - 30) && weight1 <= (pc1 - 10)) prints("¦³ÂI¤p½G..");
			if (weight1 > (pc1 - 50) && weight1 <= (pc1 - 30)) prints("¤Ó½G¤F..");

			if (sick1 < 75 && sick1 >= 50) prints("[1;34m¥Í¯f¤F..[m");
			if (sick1 < 100 && sick1 >= 75) {prints("[1;31m¯f­«!!..[m"); }
			if (sick1 >= 100) {prints("¯f¦º¤F.!."); return -1;}

			pc1 = happy1;
			if (pc1 < 20) prints("[1;31m«Ü¤£§Ö¼Ö..[m");
			if (pc1 < 40 && pc1 >= 20) prints("¤£§Ö¼Ö..");
			if (pc1 < 95 && pc1 >= 80) prints("§Ö¼Ö..");
			if (pc1 <= 100 && pc1 >= 95) prints("«Ü§Ö¼Ö..");

			pc1 = satisfy1;
			if (pc1 < 40) prints("[31;1m¤£º¡¨¬..[m");
			if (pc1 < 95 && pc1 >= 80) prints("º¡¨¬..");
			if (pc1 <= 100 && pc1 >= 95) prints("«Üº¡¨¬..");
		}
		else if (death1 == 1)
		{
			show_die_pic(2);
			move(14, 20);
			prints("¥i¼¦ªº¤pÂû¶ã©I«s«v¤F");
		}
		else if (death1 == 2)
		{
			show_die_pic(3);
		}
		else if (death1 == 3)
		{
			move(5, 0);
			outs("¹CÀ¸¤w¸gª±¨ìµ²§½Åo....");
		}
		else
		{
			vmsg("ÀÉ®×·l·´¤F....");
		}
	}   /* ¦³¾i¤pÂû */
	else
	{
		move(1, 0);
		clrtobot();
		vmsg("³o¤@®aªº¤H¨S¦³¾i¤pÂû......");
	}
}

/*---------------------------------------------------------------------------*/
/* ¨t²Î¿ï³æ:­Ó¤H¸ê®Æ  ¤pÂû©ñ¥Í  ¯S§OªA°È                                     */
/*                                                                           */
/*---------------------------------------------------------------------------*/

char weaponhead[7][10] =
{
	"¨S¦³¸Ë³Æ",
	"¶ì½¦´U¤l",
	"¤û¥Ö¤p´U",
	"¡À¦w¥þ´U",
	"¿ûÅKÀY²¯",
	"Å]ªk¾vãT",
	"¶Àª÷¸t²¯"
};


char weaponrhand[10][10] =
{
	"¨S¦³¸Ë³Æ",
	"¤j¤ì´Î",
	"ª÷ÄÝ§æ¤â",
	"«C»É¼C",
	"´¸¹p¼C",
	"ÂÍÁl¤M",
	"§Ñ±¡¼C",
	"·àÀYÄ_¤M",
	"±OÀs¤M",
	"¶Àª÷¸t§ú"
};

char weaponlhand[8][10] =
{
	"¨S¦³¸Ë³Æ",
	"¤j¤ì´Î",
	"ª÷ÄÝ§æ¤â",
	"¤ì¬Þ",
	"¤£ÄÃ¿û¬Þ",
	"¥Õª÷¤§¬Þ",
	"Å]ªk¬Þ",
	"¶Àª÷¸t¬Þ"
};


char weaponbody[7][10] =
{
	"¨S¦³¸Ë³Æ",
	"¶ì½¦«`¥Ò",
	"¯S¯Å¥Ö¥Ò",
	"¿ûÅK²¯¥Ò",
	"Å]ªk©Ü­·",
	"¥Õª÷²¯¥Ò",
	"¶Àª÷¸t¦ç"
};

char weaponfoot[8][12] =
{
	"¨S¦³¸Ë³Æ",
	"¶ì½¦©ì¾c",
	"ªF¬v¤ì®j",
	"¯S¯Å«B¾c",
	"NIKE¹B°Ê¾c",
	"Æs³½¥Ö¹u",
	"­¸¤ÑÅ]¹u",
	"¶Àª÷¸t¹u"
};

int
pip_system_freepip()
{
	char buf[256];
	move(b_lines - 1, 0);
	clrtoeol();
	getdata(b_lines - 1, 1, "¯uªº­n©ñ¥Í¶Ü¡H(y/N): ", buf, 2, 1, 0);
	if (buf[0] != 'y' && buf[0] != 'Y') return 0;
	sprintf(buf, "%s ³Q¬½¤ßªº %s ¥á±¼¤F~", d.name, cuser.userid);
	vmsg(buf);
	d.death = 2;
	pipdie("[1;31m³Q¬½¤ß¥á±ó:~~[0m", 2);
	return 0;
}


int
pip_system_service()
{
	int pipkey;
	int oldchoice;
	char buf[200];
	char oldname[21];
	time_t now;

	move(b_lines, 0);
	clrtoeol();
	move(b_lines, 0);
	prints("[1;44m  ªA°È¶µ¥Ø  [46m[1]©R¦W¤j®v [2]ÅÜ©Ê¤â³N [3]µ²§½³]§½                                [0m");
	pipkey = vkey();

	switch (pipkey)
	{
	case '1':
		move(b_lines - 1, 0);
		clrtobot();
		getdata(b_lines - 1, 1, "À°¤pÂû­«·s¨ú­Ó¦n¦W¦r¡G ", buf, 11, DOECHO, NULL);
		if (!buf[0])
		{
			vmsg("µ¥¤@¤U·Q¦n¦A¨Ó¦n¤F  :)");
			break;
		}
		else
		{
			strcpy(oldname, d.name);
			strcpy(d.name, buf);
			/*§ï¦W°O¿ý*/
			now = time(0);
			sprintf(buf, "[1;37m%s %-11s§â¤pÂû [%s] §ï¦W¦¨ [%s] [0m\n", Cdate(&now), cuser.userid, oldname, d.name);
			pip_log_record(buf);
			vmsg("¶â¶â  ´«¤@­Ó·sªº¦W¦r³á...");
		}
		break;

	case '2':  /*ÅÜ©Ê*/
		move(b_lines - 1, 0);
		clrtobot();
		/*1:¤½ 2:¥À */
		if (d.sex == 1)
		{
			oldchoice = 2; /*¤½-->¥À*/
			move(b_lines - 1, 0);
			prints("[1;33m±N¤pÂû¥Ñ[32m¡ñ[33mÅÜ©Ê¦¨[35m¡ð[33mªº¶Ü¡H [37m[y/N][0m");
		}
		else
		{
			oldchoice = 1; /*¥À-->¤½*/
			move(b_lines - 1, 0);
			prints("[1;33m±N¤pÂû¥Ñ[35m¡ð[33mÅÜ©Ê¦¨[35m¡ñ[33mªº¶Ü¡H [37m[y/N][0m");
		}
		move(b_lines, 0);
		prints("[1;44m  ªA°È¶µ¥Ø  [46m[1]©R¦W¤j®v [2]ÅÜ©Ê¤â³N [3]µ²§½³]§½                                [0m");
		pipkey = vkey();
		if (pipkey == 'Y' || pipkey == 'y')
		{
			/*§ï¦W°O¿ý*/
			now = time(0);
			if (d.sex == 1)
				sprintf(buf, "[1;37m%s %-11s§â¤pÂû [%s] ¥Ñ¡ñÅÜ©Ê¦¨¡ð¤F[0m\n", Cdate(&now), cuser.userid, d.name);
			else
				sprintf(buf, "[1;37m%s %-11s§â¤pÂû [%s] ¥Ñ¡ðÅÜ©Ê¦¨¡ñ¤F[0m\n", Cdate(&now), cuser.userid, d.name);
			pip_log_record(buf);
			vmsg("ÅÜ©Ê¤â³N§¹²¦...");
			d.sex = oldchoice;
		}
		break;

	case '3':
		move(b_lines - 1, 0);
		clrtobot();
		/*1:¤£­n¥B¥¼±B 4:­n¥B¥¼±B */
		oldchoice = d.wantend;
		if (d.wantend == 1 || d.wantend == 2 || d.wantend == 3)
		{
			oldchoice += 3; /*¨S¦³-->¦³*/
			move(b_lines - 1, 0);
			prints("[1;33m±N¤pÂû¹CÀ¸§ï¦¨[32m[¦³20·³µ²§½][33m? [37m[y/N][0m");
			sprintf(buf, "¤pÂû¹CÀ¸³]©w¦¨[¦³20·³µ²§½]..");
		}
		else
		{
			oldchoice -= 3; /*¦³-->¨S¦³*/
			move(b_lines - 1, 0);
			prints("[1;33m±N¤pÂû¹CÀ¸§ï¦¨[32m[¨S¦³20·³µ²§½][33m? [37m[y/N][0m");
			sprintf(buf, "¤pÂû¹CÀ¸³]©w¦¨[¨S¦³20·³µ²§½]..");
		}
		move(b_lines, 0);
		prints("[1;44m  ªA°È¶µ¥Ø  [46m[1]©R¦W¤j®v [2]ÅÜ©Ê¤â³N [3]µ²§½³]§½                                [0m");
		pipkey = vkey();
		if (pipkey == 'Y' || pipkey == 'y')
		{
			d.wantend = oldchoice;
			vmsg(buf);
		}
		break;
	}
	return 0;
}

#include<stdarg.h>
void
pip_data_list_va(va_list pvar)
{
	char *userid;

	userid = va_arg(pvar, char *);
	pip_data_list(cuser.userid);
}


int
pip_data_list(userid)  /*¬Ý¤pÂû­Ó¤H¸Ô²Ó¸ê®Æ*/
char *userid;
{
	char buf[256];
	char inbuf1[20];
	char inbuf2[20];
	int tm;
	int pipkey;
	int page = 1;
	struct chicken chicken;
	FILE *fs;

	/*  if (!isprint(userid[0]))*/
	usr_fpath(buf, cuser.userid, "chicken");
	/*  else
	    usr_fpath(buf,userid,"chicken");*/

	if (fs = fopen(buf, "r"))
	{
		fgets(buf, 80, fs);
		chicken.bbtime = (time_t) atol(buf);

		fscanf(fs,
			   "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %s %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
			   &(chicken.year), &(chicken.month), &(chicken.day), &(chicken.sex), &(chicken.death), &(chicken.nodone), &(chicken.relation), &(chicken.liveagain), &(chicken.chickenmode), &(chicken.level), &(chicken.exp), &(chicken.dataE),
			   &(chicken.hp), &(chicken.maxhp), &(chicken.weight), &(chicken.tired), &(chicken.sick), &(chicken.shit), &(chicken.wrist), &(chicken.bodyA), &(chicken.bodyB), &(chicken.bodyC), &(chicken.bodyD), &(chicken.bodyE),
			   &(chicken.social), &(chicken.family), &(chicken.hexp), &(chicken.mexp), &(chicken.tmpA), &(chicken.tmpB), &(chicken.tmpC), &(chicken.tmpD), &(chicken.tmpE),
			   &(chicken.mp), &(chicken.maxmp), &(chicken.attack), &(chicken.resist), &(chicken.speed), &(chicken.hskill), &(chicken.mskill), &(chicken.mresist), &(chicken.magicmode), &(chicken.specialmagic), &(chicken.fightC), &(chicken.fightD), &(chicken.fightE),
			   &(chicken.weaponhead), &(chicken.weaponrhand), &(chicken.weaponlhand), &(chicken.weaponbody), &(chicken.weaponfoot), &(chicken.weaponA), &(chicken.weaponB), &(chicken.weaponC), &(chicken.weaponD), &(chicken.weaponE),
			   &(chicken.toman), &(chicken.character), &(chicken.love), &(chicken.wisdom), &(chicken.art), &(chicken.etchics), &(chicken.brave), &(chicken.homework), &(chicken.charm), &(chicken.manners), &(chicken.speech), &(chicken.cookskill), &(chicken.learnA), &(chicken.learnB), &(chicken.learnC), &(chicken.learnD), &(chicken.learnE),
			   &(chicken.happy), &(chicken.satisfy), &(chicken.fallinlove), &(chicken.belief), &(chicken.offense), &(chicken.affect), &(chicken.stateA), &(chicken.stateB), &(chicken.stateC), &(chicken.stateD), &(chicken.stateE),
			   &(chicken.food), &(chicken.medicine), &(chicken.bighp), &(chicken.cookie), &(chicken.ginseng), &(chicken.snowgrass), &(chicken.eatC), &(chicken.eatD), &(chicken.eatE),
			   &(chicken.book), &(chicken.playtool), &(chicken.money), &(chicken.thingA), &(chicken.thingB), &(chicken.thingC), &(chicken.thingD), &(chicken.thingE),
			   &(chicken.winn), &(chicken.losee),
			   &(chicken.royalA), &(chicken.royalB), &(chicken.royalC), &(chicken.royalD), &(chicken.royalE), &(chicken.royalF), &(chicken.royalG), &(chicken.royalH), &(chicken.royalI), &(chicken.royalJ), &(chicken.seeroyalJ), &(chicken.seeA), &(chicken.seeB), &(chicken.seeC), &(chicken.seeD), &(chicken.seeE),
			   &(chicken.wantend), &(chicken.lover), chicken.name,
			   &(chicken.classA), &(chicken.classB), &(chicken.classC), &(chicken.classD), &(chicken.classE),
			   &(chicken.classF), &(chicken.classG), &(chicken.classH), &(chicken.classI), &(chicken.classJ),
			   &(chicken.classK), &(chicken.classL), &(chicken.classM), &(chicken.classN), &(chicken.classO),
			   &(chicken.workA), &(chicken.workB), &(chicken.workC), &(chicken.workD), &(chicken.workE),
			   &(chicken.workF), &(chicken.workG), &(chicken.workH), &(chicken.workI), &(chicken.workJ),
			   &(chicken.workK), &(chicken.workL), &(chicken.workM), &(chicken.workN), &(chicken.workO),
			   &(chicken.workP), &(chicken.workQ), &(chicken.workR), &(chicken.workS), &(chicken.workT),
			   &(chicken.workU), &(chicken.workV), &(chicken.workW), &(chicken.workX), &(chicken.workY), &(chicken.workZ)
			  );

		fclose(fs);
	}
	else
	{
		vmsg("§Ú¨S¦³¾i¤pÂû°Õ !");
		return 0;
	}


//  tm=(time(0)-start_time+chicken.bbtime)/60/30;
	tm = chicken.bbtime / 60 / 30;

	clear();
	move(1, 0);
	prints("       [1;33mùÝùùùùùùùûùúùùùùùùùûùÝùùùùùùùßùúùùùùùùùû[m\n");
	prints("       [0;37mùøùúùû  ùøùø ùù   ùøùãùßùÝùùùåùø ùù   ùø[m\n");
	prints("       [1;37mùøùüùý  ùøùøùÝùß  ùø  ùøùø    ùøùÝùß  ùø[m\n");
	prints("       [1;34mùãùùùùùùùýùãùåùãùùùå  ùãùå    ùãùåùãùùùå[32m......................[m");
	do
	{
		clrchyiuan(5, 23);
		switch (page)
		{
		case 1:
			move(5, 0);
			sprintf(buf,
					"[1;31m ¢~¢t[41;37m °ò¥»¸ê®Æ [0;1;31m¢u¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢¡[m\n");
			prints(buf);

			sprintf(buf,
					"[1;31m ¢x[33m¡Ì©m    ¦W :[37m %-10s [33m¡Ì¥Í    ¤é :[37m %02d/%02d/%02d   [33m¡Ì¦~    ¬ö :[37m %-2d         [31m¢x[m\n",
					chicken.name, (chicken.year) % 100, chicken.month, chicken.day, tm);
			prints(buf);

			sprintf(inbuf1, "%d%s/%d%s", chicken.hp > 1000 ? chicken.hp / 1000 : chicken.hp, chicken.hp > 1000 ? "K" : "", chicken.maxhp > 1000 ? chicken.maxhp / 1000 : chicken.maxhp, chicken.maxhp > 1000 ? "K" : "");
			sprintf(inbuf2, "%d%s/%d%s", chicken.mp > 1000 ? chicken.mp / 1000 : chicken.mp, chicken.mp > 1000 ? "K" : "", chicken.maxmp > 1000 ? chicken.maxmp / 1000 : chicken.maxmp, chicken.maxmp > 1000 ? "K" : "");

			sprintf(buf,
					"[1;31m ¢x[33m¡ÌÅé    ­« :[37m %-5d(¦Ì§J)[33m¡ÌÅé    ¤O :[37m %-11s[33m¡Ìªk    ¤O :[37m %-11s[31m¢x[m\n",
					chicken.weight, inbuf1, inbuf2);
			prints(buf);

			sprintf(buf,
					"[1;31m ¢x[33m¡Ì¯h    ³Ò :[37m %-3d        [33m¡Ì¯f    ®ð :[37m %-3d        [33m¡ÌÅ¼    Å¼ :[37m %-3d        [31m¢x[m\n",
					chicken.tired, chicken.sick, chicken.shit);
			prints(buf);

			sprintf(buf,
					"[1;31m ¢x[33m¡ÌµÃ    ¤O :[37m %-7d    [33m¡Ì¿Ë¤lÃö«Y :[37m %-7d    [33m¡Ìª÷    ¿ú :[37m %-11d[31m¢x[m\n",
					chicken.wrist, chicken.relation, chicken.money);
			prints(buf);

			sprintf(buf,
					"[1;31m ¢u¢t[41;37m ¯à¤O¸ê®Æ [0;1;31m¢u¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t[m\n");
			prints(buf);

			sprintf(buf,
					"[1;31m ¢x[33m¡Ì®ð    ½è :[37m %-10d [33m¡Ì´¼    ¤O :[37m %-10d [33m¡Ì·R    ¤ß :[37m %-10d [31m¢x[m\n",
					chicken.character, chicken.wisdom, chicken.love);
			prints(buf);

			sprintf(buf,
					"[1;31m ¢x[33m¡ÌÃÀ    ³N :[37m %-10d [33m¡Ì¹D    ¼w :[37m %-10d [33m¡Ì®a    ¨Æ :[37m %-10d [31m¢x[m\n",
					chicken.art, chicken.etchics, chicken.homework);
			prints(buf);

			sprintf(buf,
					"[1;31m ¢x[33m¡ÌÂ§    »ö :[37m %-10d [33m¡ÌÀ³    ¹ï :[37m %-10d [33m¡Ì²i    ¶¹ :[37m %-10d [31m¢x[m\n",
					chicken.manners, chicken.speech, chicken.cookskill);
			prints(buf);

			sprintf(buf,
					"[1;31m ¢u¢t[41;37m ª¬ºA¸ê®Æ [0;1;31m¢u¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t[m\n");
			prints(buf);

			sprintf(buf,
					"[1;31m ¢x[33m¡Ì§Ö    ¼Ö :[37m %-10d [33m¡Ìº¡    ·N :[37m %-10d [33m¡Ì¤H    »Ú :[37m %-10d [31m¢x[m\n",
					chicken.happy, chicken.satisfy, chicken.toman);
			prints(buf);

			sprintf(buf,
					"[1;31m ¢x[33m¡Ì¾y    ¤O :[37m %-10d [33m¡Ì«i    ´± :[37m %-10d [33m¡Ì«H    ¥õ :[37m %-10d [31m¢x[m\n",
					chicken.charm, chicken.brave, chicken.belief);
			prints(buf);

			sprintf(buf,
					"[1;31m ¢x[33m¡Ì¸o    Ä^ :[37m %-10d [33m¡Ì·P    ¨ü :[37m %-10d [33m            [37m            [31m¢x[m\n",
					chicken.offense, chicken.affect);
			prints(buf);

			sprintf(buf,
					"[1;31m ¢u¢t[41;37m µû»ù¸ê®Æ [0;1;31m¢u¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t[m\n");
			prints(buf);

			sprintf(buf,
					"[1;31m ¢x[33m¡ÌªÀ¥æµû»ù :[37m %-10d [33m¡Ì¾Ô°«µû»ù :[37m %-10d [33m¡ÌÅ]ªkµû»ù :[37m %-10d [31m¢x[m\n",
					chicken.social, chicken.hexp, chicken.mexp);
			prints(buf);

			sprintf(buf,
					"[1;31m ¢x[33m¡Ì®a¨Æµû»ù :[37m %-10d                                                 [31m¢x[m\n",
					chicken.family);
			prints(buf);

			sprintf(buf,
					"[1;31m ¢¢¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢£[m\n");
			prints(buf);

			move(b_lines - 1, 0);
			sprintf(buf,
					"                                                              [1;36m²Ä¤@­¶[37m/[36m¦@¤G­¶[m\n");
			prints(buf);
			break;

		case 2:
			move(5, 0);
			sprintf(buf,
					"[1;31m ¢~¢t[41;37m ª««~¸ê®Æ [0;1;31m¢u¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢¡[m\n");
			prints(buf);

			sprintf(buf,
					"[1;31m ¢x[33m¡Ì­¹    ª« :[37m %-10d [33m¡Ì¹s    ­¹ :[37m %-10d [33m¡Ì¤j ¸É ¤Y :[37m %-10d [31m¢x[m\n",
					chicken.food, chicken.cookie, chicken.bighp);
			prints(buf);

			sprintf(buf,
					"[1;31m ¢x[33m¡ÌÆF    ªÛ :[37m %-10d [33m¡Ì®Ñ    ¥» :[37m %-10d [33m¡Ìª±    ¨ã :[37m %-10d [31m¢x[m\n",
					chicken.medicine, chicken.book, chicken.playtool);
			prints(buf);

			sprintf(buf,
					"[1;31m ¢u¢t[41;37m ¹CÀ¸¸ê®Æ [0;1;31m¢u¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t[m\n");
			prints(buf);

			sprintf(buf,
					"[1;31m ¢x[33m¡Ì²q ®± Ä¹ :[37m %-10d [33m¡Ì²q ®± ¿é :[37m %-10d                         [31m¢x[m\n",
					chicken.winn, chicken.losee);
			prints(buf);

			sprintf(buf,
					"[1;31m ¢u¢t[41;37m ªZ¤O¸ê®Æ [0;1;31m¢u¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t[m\n");
			prints(buf);

			sprintf(buf,
					"[1;31m ¢x[33m¡Ì§ð À» ¤O :[37m %-10d [33m¡Ì¨¾ ¿m ¤O :[37m %-10d [33m¡Ì³t «× ­È :[37m %-10d [31m¢x[m\n",
					chicken.attack, chicken.resist, chicken.speed);
			prints(buf);
			sprintf(buf,
					"[1;31m ¢x[33m¡Ì§ÜÅ]¯à¤O :[37m %-10d [33m¡Ì¾Ô°«§Þ³N :[37m %-10d [33m¡ÌÅ]ªk§Þ³N :[37m %-10d [31m¢x[m\n",
					chicken.mresist, chicken.hskill, chicken.mskill);
			prints(buf);

			sprintf(buf,
					"[1;31m ¢x[33m¡ÌÀY³¡¸Ë³Æ :[37m %-10s [33m¡Ì¥k¤â¸Ë³Æ :[37m %-10s [33m¡Ì¥ª¤â¸Ë³Æ :[37m %-10s [31m¢x[m\n",
					weaponhead[chicken.weaponhead], weaponrhand[chicken.weaponrhand], weaponlhand[chicken.weaponlhand]);
			prints(buf);

			sprintf(buf,
					"[1;31m ¢x[33m¡Ì¨­Åé¸Ë³Æ :[37m %-10s [33m¡Ì¸}³¡¸Ë³Æ :[37m %-10s [33m            [37m            [31m¢x[m\n",
					weaponbody[chicken.weaponbody], weaponfoot[chicken.weaponfoot]);
			prints(buf);

			sprintf(buf,
					"[1;31m ¢u¢t[41;37m µ¥¯Å¸ê®Æ [0;1;31m¢u¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢t[m\n");
			prints(buf);

			sprintf(buf,
					"[1;31m ¢x[33m¡Ìµ¥    ¯Å :[37m %-10d [33m¡Ì¸g Åç ­È :[37m %-10d [33m¡Ì¤U¦¸¤É¯Å :[37m %-10d [31m¢x[m\n",
					chicken.level, chicken.exp, twice(d.level, 10000, 100));
			prints(buf);

			sprintf(buf,
					"[1;31m ¢¢¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢£[m\n");
			prints(buf);

			move(b_lines - 1, 0);
			sprintf(buf,
					"                                                              [1;36m²Ä¤G­¶[37m/[36m¦@¤G­¶[m\n");
			prints(buf);
			break;
		}
		move(b_lines, 0);
		sprintf(buf, "[1;44;37m  ¸ê®Æ¿ï³æ  [46m  [¡ô/PAGE UP]©¹¤W¤@­¶ [¡õ/PAGE DOWN]©¹¤U¤@­¶ [Q]Â÷¶}:            [m");
		prints(buf);
		pipkey = vkey();
		switch (pipkey)
		{
		case KEY_UP:
		case KEY_PGUP:
		case KEY_DOWN:
		case KEY_PGDN:
			if (page == 1)
				page = 2;
			else if (page == 2)
				page = 1;
			break;
		}
	}
	while ((pipkey != 'Q') && (pipkey != 'q') && (pipkey != KEY_LEFT));
	return 0;
}

/*---------------------------------------------------------------------------*/
/* ¾Ô°«¯S°Ï                                                                  */
/*                                                                           */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* ¾Ô°«¤Hª«¨M©w¨ç¦¡                                                          */
/*                                                                           */
/*---------------------------------------------------------------------------*/
/*0~1:2
  2~4:3
  5~9:5
  10~12:3
  13~14:1
  15~19:5
  20~23:4
  24~27:4
  28~33:6
  34~45:12
*/
int
get_man(class, mob, plus)
int class, mob, plus;
{
	int lucky, man;
	lucky = rand() % (class * 5);
	if (lucky <= (class*2))
	{
		man = rand() % mob + plus;
	}
	else if (lucky <= (class*4) && lucky > (class*2))
	{
		man = rand() % (mob + plus / 2) + (plus / 2);
	}
	else
	{
		man = rand() % (mob + plus);
	}
	return man;
}

int
pip_meet_vs_man()
{
	int class;
	int man, lucky;
	char ans;
	class = (d.maxhp * 30 + d.maxmp * 20 + d.attack * 20 + d.resist * 15 + d.mexp * 5 + d.hexp * 5 + d.speed * 10) / 8500 + 1;

	move(b_lines - 1, 0);
	outs("\033[1;44;37m °Ï°ì \033[46m[1]ª¢¤§¬}¸]  [2]¥_¤è¦B­ì  [3]¥j¥N¿ò¸ñ  [4]¤H¤u®q  [5]¦aº»¤§ªù            \033[m\n");
	outs("\033[1;44;37m °Ï°ì \033[46m                                                                  [Q]¦^®a\033[m");
	while (1)
	{
		char buf[128];
		ans = vkey();
		if (ans == 'q' || ans == 'Q')
			return 0;
		if (ans >= '1' && ans <= '5')
		{
			sprintf(buf, "game/pipdata/pipmob%c.dat", ans);
			pip_load_mob(buf);
			sprintf(buf, "game/pipdata/pipmobset%c.dat", ans);
			pip_load_mobset(buf);
			break;
		}
	}

	while (d.hp > 0)
	{
		move(b_lines, 0);
		outs("[1;44;37m ¤è¦V [46m[R]¦^®a [F]Áý­¹ (E/W/S/N)ªF¦è«n¥_                                        \033[m");
		ans = vkey();
		if (ans == 'r' || ans == 'R')
			return 0;

		lucky = rand() % 2000;
		if (ans != 'e' && ans != 'w' && ans != 's' && ans != 'n' && ans != 'E' && ans != 'W' && ans != 'S' && ans != 'N' &&
			ans != 'F' && ans != 'f')
			continue;
		if (ans == 'f' || ans == 'F')
			pip_basic_feed();
		else if (lucky == 1999)
		{
			vmsg("¹J¨ì¤jÅ]¤ý°Õ¡I");
		}
		else if (lucky < 1000)
		{
			if (class >= 1 && class <= 20)
			{
				man = get_man(class, mob[class][0], mob[class][1]);
			}
			else if (class > 20)
			{
				man = get_man(class, mob[21][0], mob[21][1]);
			}
			pip_fight_bad(man);
		}
		else
			vmsg("¨Sµo¥Í¥ô¦ó¨Æ¡I");
	}
	return 0;
}

void
pip_fight_bad(n)
int n;
{
	pip_fight_main(n, badmanlist, 1);
	return;
}


int
pip_fight_main(n, list, mode)
int n;
struct playrule list[];
int mode;
{
	pip_vs_man(n, list, mode);
	return 0;
}

/*---------------------------------------------------------------------------*/
/* ¾Ô°«¾Ô°«¨ç¦¡                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/
int
pip_vs_man(n, p, mode)
int n;
struct playrule *p;
int mode;
{
	/* p[n].name hp mp speed attack resist money special map */
	struct playrule m;
	char buf[256];
	char inbuf1[20];
	char inbuf2[20];
	int pipkey;
	int mankey;
	int lucky;
	int dinjure = 0;		/*¤pÂû¶Ë®`¤O*/
	int minjure = 0;		/*¹ï¤è¶Ë®`¤O*/
	int dresistmore = 0;	/*¤pÂû¥[±j¨¾¿m*/
	int mresistmore = 0;	/*¹ï¤è¥[±j¨¾¿m*/
	int oldhexp;		/*¥¼¾Ô°««e®æ°«¸gÅç*/
	int oldmexp;		/*¥¼¾Ô°««eÅ]ªk¸gÅç*/
	int oldbrave;		/*¥¼¾Ô°««e«i´±*/
	int oldhskill;		/*¥¼¾Ô°««e¾Ô°«§Þ³N*/
	int oldmskill;		/*¥¼¾Ô°««eÅ]ªk§Þ³N*/
	int oldetchics;	/*¥¼¾Ô°««e¹D¼w*/
	int oldmoney;		/*¥¼¾Ô°««eª÷¿ú*/
	int oldtired;
	int oldhp;
	int oldexp;
	int winorlose = 0;		/*1:you win 0:you loss*/

	/*ÀH¾÷²£¥Í¤Hª« ¨Ã¥B¦s¦n¾Ô°««eªº¤@¨Ç¼Æ­È*/
	oldhexp = d.hexp;
	oldmexp = d.mexp;
	oldbrave = d.brave;
	oldhskill = d.hskill;
	oldmskill = d.mskill;
	oldetchics = d.etchics;
	oldmoney = d.money;
	if (mode == 1)
	{
		m.hp = p[n].hp - rand() % 10;
		m.maxhp = (m.hp + p[n].hp) / 2;
		m.mp = p[n].mp - rand() % 10;
		m.maxmp = (m.mp + p[n].mp) / 2;
		m.speed = p[n].speed - rand() % 4 - 1;
		m.attack = p[n].attack - rand() % 10;
		m.resist = p[n].resist - rand() % 10;
		m.money = p[n].money - rand() % 50;
		m.death = p[n].death;
		m.map = p[n].map;
	}
	else
	{
		m.maxhp = d.maxhp * (80 + rand() % 50) / 100 + 20;;
		m.hp = m.maxhp - rand() % 10 + 20;
		m.maxmp = d.maxmp * (80 + rand() % 50) / 100 + 10;
		m.mp = m.maxmp - rand() % 20 + 10;
		m.speed = d.speed * (80 + rand() % 50) / 100 + 10;
		m.attack = d.attack * (80 + rand() % 50) / 100 + 10;
		m.resist = d.resist * (80 + rand() % 50) / 100 + 10;
		m.money = 0;
		m.death = 0;
	}
	/*d.tired+=rand()%(n+1)/4+2;*/
	/*d.shit+=rand()%(n+1)/4+2;*/
	do
	{
		if (m.hp <= 0) /*¼Ä¤H¦º±¼¤F*/
		{
			m.hp = 0;
			d.money += m.money;
			m.death = 1;
			d.brave += rand() % 4 + 3;
		}
		if (d.hp <= 0 || d.tired >= 100)  /*¤pÂû°}¤`*/
		{
			if (mode == 1)
			{
				d.hp = 0;
				d.tired = 0;
				d.death = 1;
			}
			else
			{
				d.hp = d.maxhp / 3 + 10;
				d.hexp -= rand() % 3 + 2;
				d.mexp -= rand() % 3 + 2;
				d.tired = 50;
				d.death = 1;
			}
		}
		clear();
		/*vs_head("¹q¤l¾i¤pÂû", BoardName);*/
		move(0, 0);
		if (d.sex == 1)
			sprintf(buf, "[1;41m  " NICKNAME PIPNAME " ¡ã [32m¡ñ [37m%-10s                                                [0m", d.name);
		else if (d.sex == 2)
			sprintf(buf, "[1;41m  " NICKNAME PIPNAME " ¡ã [33m¡ð [37m%-10s                                                [0m", d.name);
		else
			sprintf(buf, "[1;41m  " NICKNAME PIPNAME " ¡ã [34m¡H [37m%-10s                                                [0m", d.name);
		prints(buf);
		move(6, 0);
		if (mode == 1)
			show_badman_pic(m.map/*n*/);
		move(1, 0);
		sprintf(buf, "[1;31m¢z¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢{[m");
		prints(buf);
		move(2, 0);
		/* lucky®³¨Ó·ícolor¥Î*/
		if (d.tired >= 80)
			lucky = 31;
		else if (d.tired >= 60 && d.tired < 80)
			lucky = 33;
		else
			lucky = 37;
		sprintf(inbuf1, "%d%s/%d%s", d.hp > 1000 ? d.hp / 1000 : d.hp, d.hp > 1000 ? "K" : "", d.maxhp > 1000 ? d.maxhp / 1000 : d.maxhp, d.maxhp > 1000 ? "K" : "");
		sprintf(inbuf2, "%d%s/%d%s", d.mp > 1000 ? d.mp / 1000 : d.mp, d.mp > 1000 ? "K" : "", d.maxmp > 1000 ? d.maxmp / 1000 : d.maxmp, d.maxmp > 1000 ? "K" : "");

		sprintf(buf, "[1;31m¢x[33m¥Í  ©R:[37m%-12s[33mªk  ¤O:[37m%-12s[33m¯h  ³Ò:[%dm%-12d[33mª÷  ¿ú:[37m%-10d[31m¢x[m",
				inbuf1, inbuf2, lucky, d.tired, d.money);
		prints(buf);
		move(3, 0);
		sprintf(buf, "[1;31m¢x[33m§ð  À»:[37m%-10d  [33m¨¾  ¿m:[37m%-10d  [33m³t  «×:[37m%-10d  [33m¸g  Åç:[37m%-10d[31m¢x[m",
				d.attack, d.resist, d.speed, d.exp);
		prints(buf);
		move(4, 0);
		sprintf(buf, "[1;31m¢x[33m­¹  ª«:[37m%-5d       [33m¤j¸É¤Y:[37m%-5d       [33m¹s  ­¹:[37m%-5d       [33mÆF  ªÛ:[37m%-5d     [31m¢x[m",
				d.food, d.bighp, d.cookie, d.medicine);
		prints(buf);
		move(5, 0);
		sprintf(buf, "[1;31m¢|¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢}[m");
		prints(buf);
		move(19, 0);
		sprintf(buf, "[1;34m¢z¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢{[m");
		prints(buf);
		move(20, 0);
		sprintf(inbuf1, "%d%s/%d%s", m.hp > 1000 ? m.hp / 1000 : m.hp, m.hp > 1000 ? "K" : "", m.maxhp > 1000 ? m.maxhp / 1000 : m.maxhp, m.maxhp > 1000 ? "K" : "");
		sprintf(inbuf2, "%d%s/%d%s", m.mp > 1000 ? m.mp / 1000 : m.mp, m.mp > 1000 ? "K" : "", m.maxmp > 1000 ? m.maxmp / 1000 : m.maxmp, m.maxmp > 1000 ? "K" : "");

		sprintf(buf, "[1;34m¢x[32m©m  ¦W:[37m%-10s  [32m¥Í  ©R:[37m%-11s [32mªk  ¤O:[37m%-11s                  [34m¢x[m",
				p[n].name, inbuf1, inbuf2);
		prints(buf);
		move(21, 0);
		sprintf(buf, "[1;34m¢x[32m§ð  À»:[37m%-6d      [32m¨¾  ¿m:[37m%-6d      [32m³t  «×:[37m%-6d      [32mª÷  ¿ú:[37m%-6d    [34m¢x[m",
				m.attack, m.resist, m.speed, m.money);
		prints(buf);
		move(22, 0);
		sprintf(buf, "[1;34m¢|¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢}[m");
		prints(buf);
		move(b_lines, 0);
		sprintf(buf, "[1;44;37m  ¾Ô°«©R¥O  [46m  [1]´¶³q  [2]¥þ¤O  [3]Å]ªk  [4]¨¾¿m  [5]¸É¥R  [6]°k©R         [m");
		prints(buf);

		if (m.death == 0 && d.death == 0)
		{
			dresistmore = 0;
			d.nodone = 0;
			pipkey = vkey();
			switch (pipkey)
			{
			case '1':
				if (rand() % 9 == 0)
				{
					vmsg("³ºµM¨S¥´¤¤..:~~~");
				}
				else
				{
					if (mresistmore == 0)
						dinjure = (d.hskill / 100 + d.hexp / 100 + d.attack / 9 - m.resist / 8 + rand() % 12 + 2 - m.speed / 30 + d.speed / 30);
					else
						dinjure = (d.hskill / 100 + d.hexp / 100 + d.attack / 9 - m.resist / 6 + rand() % 12 + 2 - m.speed / 30 + d.speed / 30);
					if (dinjure <= 0)
						dinjure = 9;
					m.hp -= dinjure;
					d.hexp += rand() % 2 + 2;
					d.hskill += rand() % 2 + 1;
					sprintf(buf, "´¶³q§ðÀ»,¹ï¤è¥Í©R¤O´î§C%d", dinjure);
					vmsg(buf);
				}
				d.tired += rand() % (n + 1) / 15 + 2;
				break;

			case '2':
				show_fight_pic(2);
				if (rand() % 11 == 0)
				{
					vmsg("³ºµM¨S¥´¤¤..:~~~");
				}
				else
				{
					if (mresistmore == 0)
						dinjure = (d.hskill / 100 + d.hexp / 100 + d.attack / 5 - m.resist / 12 + rand() % 12 + 6 - m.speed / 50 + d.speed / 30);
					else
						dinjure = (d.hskill / 100 + d.hexp / 100 + d.attack / 5 - m.resist / 8 + rand() % 12 + 6 - m.speed / 40 + d.speed / 30);
					if (dinjure <= 15)
						dinjure = 20;
					if (d.hp > 5)
					{
						m.hp -= dinjure;
						d.hp -= 5;
						d.hexp += rand() % 3 + 3;
						d.hskill += rand() % 2 + 2;
						d.tired += rand() % (n + 1) / 10 + 3;
						sprintf(buf, "¥þ¤O§ðÀ»,¹ï¤è¥Í©R¤O´î§C%d", dinjure);
						vmsg(buf);
					}
					else
					{
						d.nodone = 1;
						vmsg("§AªºHP¤p©ó5°Õ..¤£¦æ°Õ...");
					}
				}
				break;

			case '3':
				oldtired = d.tired;
				oldhp = d.hp;
				d.magicmode = 0;
				dinjure = pip_magic_menu(0, NULL);
				if (dinjure < 0)
					dinjure = 5;
				if (d.nodone == 0)
				{
					if (d.magicmode == 1)
					{
						oldtired = oldtired - d.tired;
						oldhp = d.hp - oldhp;
						sprintf(buf, "ªvÀø«á,¥Í©R¤O´£°ª%d ¯h³Ò­°§C%d", oldhp, oldtired);
						vmsg(buf);
					}
					else
					{
						if (rand() % 15 == 0)
							vmsg("³ºµM¨S¥´¤¤..:~~~");
						else
						{
							if (d.mexp <= 100)
							{
								if (rand() % 4 > 0)
									dinjure = dinjure * 60 / 100;
								else
									dinjure = dinjure * 80 / 100;
							}
							else if (d.mexp <= 250 && d.mexp > 100)
							{
								if (rand() % 4 > 0)
									dinjure = dinjure * 70 / 100;
								else
									dinjure = dinjure * 85 / 100;
							}
							else if (d.mexp <= 500 && d.mexp > 250)
							{
								if (rand() % 4 > 0)
									dinjure = dinjure * 85 / 100;
								else
									dinjure = dinjure * 95 / 100;
							}
							else if (d.mexp > 500)
							{
								if (rand() % 10 > 0)
									dinjure = dinjure * 90 / 100;
								else
									dinjure = dinjure * 99 / 100;
							}
							if ((p[n].special[d.magicmode-2] - 48) == 1)
							{
								if (rand() % 2 > 0)
								{
									dinjure = dinjure * 125 / 100;
								}
								else
								{
									dinjure = dinjure * 110 / 100;
								}
							}
							else
							{
								if (rand() % 2 > 0)
								{
									dinjure = dinjure * 60 / 100;
								}
								else
								{
									dinjure = dinjure * 75 / 100;
								}
							}
							d.tired += rand() % (n + 1) / 12 + 2;
							m.hp -= dinjure;
							/*d.mexp+=rand()%2+2;*/
							d.mskill += rand() % 2 + 2;
							sprintf(buf, "Å]ªk§ðÀ»,¹ï¤è¥Í©R¤O´î§C%d", dinjure);
							vmsg(buf);
						}
					}
				}
				break;
			case '4':
				dresistmore = 1;
				d.tired += rand() % (n + 1) / 20 + 1;
				vmsg("¤pÂû¥[±j¨¾¿m°Õ....");
				break;

			case '5':

				pip_basic_feed();
				break;

			case '6':
				d.money -= (rand() % 100 + 30);
				d.brave -= (rand() % 3 + 2);
				if (d.money < 0)
					d.money = 0;
				if (d.hskill < 0)
					d.hskill = 0;
				if (d.brave < 0)
					d.brave = 0;
				clear();
				vs_head("¹q¤l¾i¤pÂû", BoardName);
				move(10, 0);
				prints("            [1;31m¢z¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢{[m\n");
				prints("            [1;31m¢x  [37m¹ê¤O¤£±jªº¤pÂû [33m%-10s                 [31m¢x[m\n", d.name);
				prints("            [1;31m¢x  [37m¦b»P¹ï¤â [32m%-10s [37m¾Ô°««á¸¨¶]°Õ          [31m¢x[m\n", p[n].name);
				sprintf(inbuf1, "%d/%d", d.hexp - oldhexp, d.mexp - oldmexp);
				prints("            [1;31m¢x  [37mµû»ù¼W¥[¤F [36m%-5s [37mÂI  §Þ³N¼W¥[¤F [36m%-2d/%-2d [37mÂI  [31m¢x[m\n", inbuf1, d.hskill - oldhskill, d.mskill - oldmskill);
				sprintf(inbuf1, "%d [37m¤¸", oldmoney - d.money);
				prints("            [1;31m¢x  [37m«i´±­°§C¤F [36m%-5d [37mÂI  ª÷¿ú´î¤Ö¤F [36m%-13s  [31m¢x[m\n", oldbrave - d.brave, inbuf1);
				prints("            [1;31m¢|¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢}[m");
				vmsg("¤T¤Q¤»­p ¨«¬°¤Wµ¦...");
				winorlose = 0;
				break;
			}
		}
		clear();
		move(0, 0);
		if (d.sex == 1)
			sprintf(buf, "[1;41m  " NICKNAME PIPNAME " ¡ã [32m¡ñ [37m%-10s                                                  [0m", d.name);
		else if (d.sex == 2)
			sprintf(buf, "[1;41m  " NICKNAME PIPNAME " ¡ã [33m¡ð [37m%-10s                                                  [0m", d.name);
		else
			sprintf(buf, "[1;41m  " NICKNAME PIPNAME " ¡ã [34m¡H [37m%-10s                                                  [0m", d.name);
		prints(buf);
		move(1, 0);
		sprintf(buf, "[1;31m¢z¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢{[m");
		prints(buf);
		move(2, 0);
		/* lucky®³¨Ó·ícolor¥Î*/
		if (d.tired >= 80)
			lucky = 31;
		else if (d.tired >= 60 && d.tired < 80)
			lucky = 33;
		else
			lucky = 37;

		sprintf(inbuf1, "%d%s/%d%s", d.hp > 1000 ? d.hp / 1000 : d.hp, d.hp > 1000 ? "K" : "", d.maxhp > 1000 ? d.maxhp / 1000 : d.maxhp, d.maxhp > 1000 ? "K" : "");
		sprintf(inbuf2, "%d%s/%d%s", d.mp > 1000 ? d.mp / 1000 : d.mp, d.mp > 1000 ? "K" : "", d.maxmp > 1000 ? d.maxmp / 1000 : d.maxmp, d.maxmp > 1000 ? "K" : "");

		sprintf(buf, "[1;31m¢x[33m¥Í  ©R:[37m%-12s[33mªk  ¤O:[37m%-12s[33m¯h  ³Ò:[%dm%-12d[33mª÷  ¿ú:[37m%-10d[31m¢x[m",
				inbuf1, inbuf2, lucky, d.tired, d.money);
		prints(buf);

		move(3, 0);
		sprintf(buf, "[1;31m¢x[33m§ð  À»:[37m%-10d  [33m¨¾  ¿m:[37m%-10d  [33m³t  «×:[37m%-10d  [33m¸g  Åç:[37m%-10d[31m¢x[m",
				d.attack, d.resist, d.speed, d.exp);
		prints(buf);
		move(4, 0);
		sprintf(buf, "[1;31m¢x[33m­¹  ª«:[37m%-5d       [33m¤j¸É¤Y:[37m%-5d       [33m¹s  ­¹:[37m%-5d       [33mÆF  ªÛ:[37m%-5d     [31m¢x[m",
				d.food, d.bighp, d.cookie, d.medicine);
		prints(buf);
		move(5, 0);
		sprintf(buf, "[1;31m¢|¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢}[m");
		prints(buf);
		move(6, 0);
		if (mode == 1)
			show_badman_pic(m.map/*n*/);
		move(19, 0);
		sprintf(buf, "[1;34m¢z¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢{[m");
		prints(buf);
		move(20, 0);
		sprintf(inbuf1, "%d/%d", m.hp, m.maxhp);
		sprintf(inbuf2, "%d/%d", m.mp, m.maxmp);
		sprintf(buf, "[1;34m¢x[32m©m  ¦W:[37m%-10s  [32m¥Í  ©R:[37m%-11s [32mªk  ¤O:[37m%-11s                  [34m¢x[m",
				p[n].name, inbuf1, inbuf2);
		prints(buf);
		move(21, 0);
		sprintf(buf, "[1;34m¢x[32m§ð  À»:[37m%-6d      [32m¨¾  ¿m:[37m%-6d      [32m³t  «×:[37m%-6d      [32mª÷  ¿ú:[37m%-6d    [34m¢x[m",
				m.attack, m.resist, m.speed, m.money);
		prints(buf);
		move(22, 0);
		sprintf(buf, "[1;34m¢|¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢}[m");
		prints(buf);
		move(b_lines, 0);
		sprintf(buf, "[1;41;37m  [37m§ðÀ»©R¥O  [47m  [31m[1][30m´¶³q  [31m[2][30m¥þ¤O  [31m[3][30mÅ]ªk  [31m[4][30m¨¾¿m  [31m[5][30m°k©R                     [m");
		prints(buf);

		if ((m.hp > 0) && (pipkey != '6') && (pipkey == '1' || pipkey == '2' || pipkey == '3' || pipkey == '4' || pipkey == '5') && (d.death == 0) && (d.nodone == 0))
		{
			mresistmore = 0;
			lucky = rand() % 100;
			if (lucky >= 0 && lucky <= 50)
				mankey = 1;
			else if (lucky >= 51 && lucky <= 84)
				mankey = 2;
			else if (lucky >= 85 && lucky <= 97)
				mankey = 3;
			else if (lucky >= 98)
				mankey = 4;
			switch (mankey)
			{
			case 1:
				if (rand() % 6 == 5)
				{
					vmsg("¹ï¤è¨S¥´¤¤..:~~~");
				}
				else
				{
					if (dresistmore == 0)
						minjure = (m.attack / 9 - d.resist / 12 + rand() % 15 + 4 - d.speed / 30 + m.speed / 30 - d.hskill / 200 - d.hexp / 200);
					else
						minjure = (m.attack / 9 - d.resist / 8 + rand() % 12 + 4 - d.speed / 50 + m.speed / 20 - d.hskill / 200 - d.hexp / 200);
					if (minjure <= 0)
						minjure = 8;
					d.hp -= minjure;
					d.tired += rand() % 3 + 2;
					sprintf(buf, "¹ï¤è´¶³q§ðÀ»,¥Í©R¤O´î§C%d", minjure);
					vmsg(buf);
				}
				break;

			case 2:
				if (rand() % 11 == 10)
				{
					vmsg("¹ï¤è¨S¥´¤¤..:~~~");
				}
				else
				{
					if (m.hp > 5)
					{
						if (dresistmore == 0)
							minjure = (m.attack / 5 - d.resist / 12 + rand() % 12 + 6 - d.speed / 30 + m.speed / 30 - d.hskill / 200 - d.hexp / 200);
						else
							minjure = (m.attack / 5 - d.resist / 8 + rand() % 12 + 6 - d.speed / 30 + m.speed / 30 - d.hskill / 200 - d.hexp / 200);
						if (minjure <= 15)
							minjure = 20;
						d.hp -= minjure;
						m.hp -= 5;
						sprintf(buf, "¹ï¤è¥þ¤O§ðÀ», ¥Í©R¤O´î§C%d", minjure);
						d.tired += rand() % 4 + 4;
						vmsg(buf);
					}
					else
					{
						if (dresistmore == 0)
							minjure = (m.attack / 9 - d.resist / 12 + rand() % 12 + 4 - d.speed / 30 + m.speed / 25 - d.hexp / 200 - d.hskill / 200);
						else
							minjure = (m.attack / 9 - d.resist / 8 + rand() % 12 + 3 - d.speed / 30 + m.speed / 25 - d.hexp / 200 - d.hskill / 200);
						if (minjure <= 0)
							minjure = 4;
						d.hp -= minjure;
						d.tired += rand() % 3 + 2;
						sprintf(buf, "¹ï¤è´¶³q§ðÀ»,¥Í©R¤O´î§C%d", minjure);
						vmsg(buf);
					}
				}
				break;

			case 3:
				if (rand() % 5 > 3 && m.mp > 20)
				{
					if (rand() % 6 > 0 && m.mp >= 50 && m.hp > (m.maxhp / 10))
					{
						if (m.mp >= (m.maxmp / 2))
						{
							minjure = m.maxmp / 4;
							m.mp -= (500 - rand() % 300);
							if (rand() % 2)
								sprintf(inbuf1, "¼ö¤õÅ]");
							else
								sprintf(inbuf1, "´H®ð°­");
						}
						else if (m.mp < (m.maxmp / 2) && m.mp >= (m.maxmp / 4))
						{
							minjure = m.maxmp / 5;
							m.mp -= (300 + rand() % 200);
							if (rand() % 2)
								sprintf(inbuf1, "¨g¤ô©Ç");
							else
								sprintf(inbuf1, "«ã¤g¦ä");
						}
						else if (m.mp < (m.maxmp / 4) && m.mp >= (m.maxmp / 6))
						{
							minjure = m.maxmp / 6;
							m.mp -= (100 + rand() % 100);
							if (rand() % 2)
								sprintf(inbuf1, "°g»î°­®t");
							else
								sprintf(inbuf1, "¥Û©Ç");
						}
						else if (m.mp < (m.maxmp / 6) && m.mp >= 0)
						{
							minjure = m.maxmp / 8;
							m.mp -= 50;
							if (rand() % 2)
								sprintf(inbuf1, "°­¤ì»î");
							else
								sprintf(inbuf1, "­·§¯");
						}
						minjure = minjure - d.resist / 50 - d.mresist / 10 - d.mskill / 200 - d.mexp / 200 + rand() % 10;
						if (minjure < 0)
							minjure = 15;
						d.hp -= minjure;
						if (m.mp < 0) m.mp = 0;
						d.mresist += rand() % 2 + 1;
						sprintf(buf, "¹ï¤è©Û´«¤F%s,§A¨ü¶Ë¤F%dÂI", inbuf1, minjure);
						vmsg(buf);
					}
					else
					{
						m.mp -= 20;
						m.hp += (m.maxmp / 6) + rand() % 20;
						if (m.hp > m.maxhp)
							m.hp = m.maxhp;
						vmsg("¹ï¤è¨Ï¥ÎÅ]ªkªvÀø¤F¦Û¤v...");
					}
				}
				else
				{
					mresistmore = 1;
					vmsg("¹ï¤è¥[±j¨¾¿m....");
				}
				break;

			case 4:
				d.money += (m.money + m.money / 2) / 3 + rand() % 10;
				d.hskill += rand() % 4 + 3;
				d.brave += rand() % 3 + 2;
				m.death = 1;
				sprintf(buf, "¹ï¤è¥ý°{¤F..¦ý±¼¤F¤@¨Ç¿úµ¹§A...");
				vmsg(buf);
				break;
			}
		}

		if (m.death == 1)
		{
			clear();
			oldexp = ((d.hexp - oldhexp) + (d.mexp - oldmexp) + rand() % 10) * (d.level + 1) + rand() % (d.level + 1);
			d.exp += oldexp;
			vs_head("¹q¤l¾i¤pÂû", BoardName);
			if (mode == 1)
			{
				move(10, 0);
				prints("            [1;31m¢z¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢{[m\n");
				prints("            [1;31m¢x  [37m­^«iªº¤pÂû [33m%-10s                     [31m¢x[m\n", d.name);
				prints("            [1;31m¢x  [37m¥´±Ñ¤F¨¸´cªº©Çª« [32m%-10s               [31m¢x[m\n", p[n].name);
			}
			else
			{
				move(10, 0);
				prints("            [1;31m¢z¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢{[m\n");
				prints("            [1;31m¢x  [37mªZ³N¤j·|ªº¤pÂû [33m%-10s                 [31m¢x[m\n", d.name);
				prints("            [1;31m¢x  [37m¥´±Ñ¤F±j«lªº¹ï¤â [32m%-10s               [31m¢x[m\n", p[n].name);
			}
			sprintf(inbuf1, "%d/%d", d.hexp - oldhexp, d.mexp - oldmexp);
			prints("            [1;31m¢x  [37mµû»ù´£¤É¤F %-5s ÂI  §Þ³N¼W¥[¤F %-2d/%-2d ÂI  [31m¢x[m\n", inbuf1, d.hskill - oldhskill, d.mskill - oldmskill);
			sprintf(inbuf1, "%d ¤¸", d.money - oldmoney);
			prints("            [1;31m¢x  [37m«i´±´£¤É¤F %-5d ÂI  ª÷¿ú¼W¥[¤F %-9s [31m¢x[m\n", d.brave - oldbrave, inbuf1);
			prints("            [1;31m¢x  [37m¸gÅç­È¼W¥[¤F %-6d ÂI  ¤É¯Å©|»Ý %-6d ÂI[31m¢x[m\n", oldexp, twice(d.level, 10000, 100) - d.exp);
			prints("            [1;31m¢|¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢}[m\n");

			if (m.hp <= 0)
				vmsg("¹ï¤è¦º±¼Åo..©Ò¥H§AÄ¹Åo..");
			else if (m.hp > 0)
				vmsg("¹ï¤è¸¨¶]Åo..©Ò¥Hºâ§AÄ¹Åo.....");
			winorlose = 1;
		}
		if (d.death == 1 && mode == 1)
		{
			clear();
			vs_head("¹q¤l¾i¤pÂû", BoardName);
			move(10, 0);
			prints("            [1;31m¢z¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢{[m\n");
			prints("            [1;31m¢x  [37m¥i¼¦ªº¤pÂû [33m%-10s                     [31m¢x[m\n", d.name);
			prints("            [1;31m¢x  [37m¦b»P [32m%-10s [37mªº¾Ô°«¤¤¡A                [31m¢x[m\n", p[n].name);
			prints("            [1;31m¢x  [37m¤£©¯¦a°}¤`¤F¡A¦b¦¹¯S§OÀq«s..........      [31m¢x[m\n");
			prints("            [1;31m¢|¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢}[m\n");
			vmsg("¤pÂû°}¤`¤F....");
			pipdie("[1;31m¾Ô°«¤¤³Q¥´¦º¤F...[m  ", 1);
		}
		else if (d.death == 1 && mode == 2)
		{
			clear();
			vs_head("¹q¤l¾i¤pÂû", BoardName);
			move(10, 0);
			prints("            [1;31m¢z¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢{\n[m");
			prints("            [1;31m¢x  [37m¥i¼¦ªº¤pÂû [33m%-10s                     [31m¢x[m\n", d.name);
			prints("            [1;31m¢x  [37m¦b»P [32m%-10s [37mªº¾Ô°«¤¤¡A                [31m¢x[m\n", p[n].name);
			prints("            [1;31m¢x  [37m¤£©¯¦a¥´¿é¤F¡A°OªÌ²{³õ¯S§O³ø¾É.........   [31m¢x[m\n");
			prints("            [1;31m¢|¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢}[m\n");
			vmsg("¤pÂû¥´¿é¤F....");
		}
	}
	while ((pipkey != '6') && (d.death != 1) && (m.death != 1) && (mankey != 8));
	pip_check_level();
	return winorlose;
}


/*---------------------------------------------------------------------------*/
/* ¾Ô°«Å]ªk¨ç¦¡                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/
#ifdef HAVE_PIP_FIGHT
static int
get_hurt(hurt, mexp)
{
	int dinjure;
	if (mexp > 14000)
		mexp = 14000;
	mexp = (int)mexp / 100;
	if (rand() % 5 > 0)
		dinjure = (int)hurt * (60 + mexp) / 100;
	else
		dinjure = (int)hurt * (70 + mexp) / 100;
	return dinjure;
}
#endif

/*¶i¤J¨Ï¥ÎÅ]ªk¿ï³æ*/
static int
pip_magic_menu(mode, opt)  /*¾Ô°«¤¤ªk³NªºÀ³¥Î*/
int mode;
UTMP *opt;
{
	char buf[256];
	int injure;		/*¶Ë®`¤O*/
	int pipkey;

	do
	{
		move(b_lines, 0);
		clrtoeol();
		move(b_lines, 0);
		if (mode)
		{
			sprintf(buf,
					"[1;44;37m  Å]ªk¿ï³æ  [46m  [1]ªvÀø [2]¹p¨t [3]¦B¨t [4]¤õ¨t [5]¤g¨t [6]­·¨t [7]¯S®í [Q]©ñ±ó: [m");
		}
		else
		{
			sprintf(buf,
					"[1;44;37m  Å]ªk¿ï³æ  [46m  [1]ªvÀø [2]¹p¨t [3]¦B¨t [4]¤õ¨t [5]¤g¨t [6]­·¨t [Q]©ñ±ó: [m");
		}
		move(b_lines, 0);
		prints(buf);
		pipkey = vkey();
		switch (pipkey)
		{
		case '1':  /*ªvÀøªk³N*/
			d.magicmode = 1;
			injure = pip_magic_doing_menu(treatmagiclist);
			break;

		case '2':  /*¹p¨tªk³N*/
			d.magicmode = 2;
			injure = pip_magic_doing_menu(thundermagiclist);
			break;

		case '3': /*¦B¨tªk³N*/
			d.magicmode = 3;
			injure = pip_magic_doing_menu(icemagiclist);
			break;

		case '4': /*ª¢¨tªk³N*/
			d.magicmode = 4;
			injure = pip_magic_doing_menu(firemagiclist);
			show_fight_pic(341);
			vmsg("¤pÂû¨Ï¥Î¤Fª¢¨tªk³N");
			break;

		case '5': /*¤g¨tªk³N*/
			d.magicmode = 5;
			injure = pip_magic_doing_menu(earthmagiclist);
			break;

		case '6': /*­·¨tªk³N*/
			d.magicmode = 6;
			injure = pip_magic_doing_menu(windmagiclist);
			break;
#ifdef HAVE_PIP_FIGHT
		case '7':
			if (!mode)
			{
				pipkey = ' ';
				break;
			}
			else
			{
				d.magicmode = 7;
				injure = pip_magic_fight_menu(specialmagiclist, opt);
				break;
			}
#endif
		}
	}
	while ((pipkey != '1') && (pipkey != '2') && (pipkey != '3') && (pipkey != '4') && (pipkey != '5') && (pipkey != '6') && (pipkey != '7') && (pipkey != 'Q') && (pipkey != 'q') && (d.nodone == 0));

	if ((pipkey == 'Q') || (pipkey == 'q'))
	{
		d.nodone = 1;
	}
	return injure;
}

/*Å]ªkµøµ¡*/
static int
pip_magic_doing_menu(p)   /*Å]ªkµe­±*/
struct magicset *p;
{
	register int n = 1;
	register char *s;
	char buf[256];
	char ans[5];
	int pipkey;
	int injure = 0;

	d.nodone = 0;

	clrchyiuan(6, 18);
	move(7, 0);
	sprintf(buf, "[1;31m¢t[37;41m   ¥i¥Î[%s]¤@Äýªí   [0;1;31m¢u¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w[m", p[0].name);
	prints(buf);
	while ((s = p[n].name) && (p[n].needmp <= d.mp))
	{
		move(7 + n, 4);
		if (p[n].hpmode == 1)
		{
			sprintf(buf,
					"[1;37m[[36m%d[37m] [33m%-12s  [37m»Ý­nªk¤O: [32m%-6d  [37m«ì´_Åé¤O: [32m%-6d [37m«ì´_¯h³Ò: [32m%-6d[m   ", n, p[n].name, p[n].needmp, p[n].hp, p[n].tired);
			prints(buf);
		}
		else if (p[n].hpmode == 2)
		{
			sprintf(buf,
					"[1;37m[[36m%d[37m] [33m%-12s  [37m»Ý­nªk¤O: [32m%-6d  [37m«ì´_Åé¤O¨ì[35m³Ì¤j­È[37m «ì´_¯h³Ò¨ì[35m³Ì¤p­È[m  ", n, p[n].name, p[n].needmp);
			prints(buf);
		}
		else if (p[n].hpmode == 0)
		{
			sprintf(buf,
					"[1;37m[[36m%d[37m] [33m%-12s  [37m»Ý­nªk¤O: [32m%-6d [m             ", n, p[n].name, p[n].needmp);
			prints(buf);
		}
		n++;
	}
	n -= 1;

	do
	{
		move(16, 4);
		sprintf(buf, "§A·Q¨Ï¥Î¨º¤@­Ó%8s©O?  [Q]©ñ±ó: ", p[0].name);
		getdata(16, 4, buf, ans, 2, 1, 0);
		if (ans[0] != 'q' && ans[0] != 'Q')
		{
			pipkey = atoi(ans);
		}
	}
	while (ans[0] != 'q' && ans[0] != 'Q' && (pipkey > n || pipkey <= 0));

	if (ans[0] != 'q' && ans[0] != 'Q')
	{
		getdata(17, 4, "½T©w¨Ï¥Î¶Ü? [Y/n]: ", ans, 2, 1, 0);
		if (ans[0] != 'n' && ans[0] != 'N')
		{
			if (p[pipkey].hpmode == 1)
			{
				d.hp += p[pipkey].hp;
				d.tired -= p[pipkey].tired;
				d.mp -= p[pipkey].needmp;
				if (d.hp > d.maxhp)
					d.hp = d.maxhp;
				if (d.tired < 0)
					d.tired = 0;
				injure = 0;
			}
			else if (p[pipkey].hpmode == 2)
			{
				d.hp = d.maxhp;
				d.mp -= p[pipkey].needmp;
				d.tired = 0;
				injure = 0;
			}
			else
			{
				injure = (p[pipkey].hp + (d.maxmp / 8) - rand() % 5);
				d.mp -= p[pipkey].needmp;
			}
			d.mexp += rand() % 3 + pipkey;
		}
		else
		{
			d.nodone = 1;
			injure = 0;
		}
	}
	else
	{
		d.nodone = 1;
		injure = 0;
	}
	return injure;
}

#ifdef	HAVE_PIP_FIGHT
static int
pip_magic_fight_menu(p, opt)  /*Å]ªkµe­±*/
struct magicset *p;
UTMP *opt;
{
	int n = 1, cur = 1, mg[16];
	char buf[256];
	char ans[5];
	int pipkey;
	int injure = 0;
	struct magicset *s;

	s = p;
	d.nodone = 0;

	clrchyiuan(6, 18);
	move(7, 0);
	sprintf(buf, "[1;31m¢t[37;41m   ¥i¥Î[%s]¤@Äýªí   [0;1;31m¢u¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w[m", s->name);
	prints(buf);
	s++;
	while (s->name)
	{
		move(7 + n, 4);
		if ((d.specialmagic & s->map) && (s->needmp <= d.mp))
		{
			sprintf(buf,
					"[1;37m[[36m%d[37m] [33m%-12s  [37m»Ý­nªk¤O: [32m%-6d [m             ", n, s->name, s->needmp);
			prints(buf);
			mg[n] = cur;
			n++;
		}
		cur++;
		s++;
	}
	n -= 1;

	do
	{
		move(16, 4);
		sprintf(buf, "§A·Q¨Ï¥Î¨º¤@­Ó%8s©O?  [Q]©ñ±ó: ", p[0].name);
		getdata(16, 4, buf, ans, 2, 1, 0);
		if (ans[0] != 'q' && ans[0] != 'Q')
		{
			pipkey = atoi(ans);
		}
	}
	while (ans[0] != 'q' && ans[0] != 'Q' && (pipkey > n || pipkey <= 0));

	if (ans[0] != 'q' && ans[0] != 'Q')
	{
		getdata(17, 4, "½T©w¨Ï¥Î¶Ü? [Y/n]: ", ans, 2, 1, 0);
		if (ans[0] != 'n' && ans[0] != 'N')
		{
			injure = (opt->pip->hp * p[mg[pipkey]].hp / 100 - rand() % 300);
			d.mp -= p[mg[pipkey]].needmp;
			d.mexp += rand() % 30 + pipkey + 100;
			cutmp->pip->mode = - mg[pipkey];
		}
		else
		{
			d.nodone = 1;
			injure = 0;
		}
	}
	else
	{
		d.nodone = 1;
		injure = 0;
	}
	return injure;
}
#endif
/*---------------------------------------------------------------------------*/
/* ¨ç¦¡¯S°Ï                                                                  */
/*                                                                           */
/*---------------------------------------------------------------------------*/

/*¨D±B*/
int
pip_marriage_offer()
{
	time_t now;
	char buf[256];
	char ans[4];
	int money;
	int who;
	char *name[5][2] = {{"¤k°Ó¤H¢Ï", "°Ó¤H¢Ï"},
		{"¤k°Ó¤H¢Ð", "°Ó¤H¢Ð"},
		{"¤k°Ó¤H¢Ñ", "°Ó¤H¢Ñ"},
		{"¤k°Ó¤H¢Ò", "°Ó¤H¢Ò"},
		{"¤k°Ó¤H¢Ó", "°Ó¤H¢Ó"}
	};
	do
	{
		who = rand() % 5;
	}
	while (d.lover == (who + 3));

	money = rand() % 2000 + rand() % 3000 + 4000;
	sprintf(buf, "%s±a¨Ó¤Fª÷¿ú%d¡A­n¦V§Aªº¤pÂû¨D±B¡A±zÄ@·N¶Ü¡H[y/N]: ", name[who][d.sex-1], money);
	getdata(b_lines - 1, 1, buf, ans, 2, 1, 0);
	if (ans[0] == 'y' || ans[0] == 'Y')
	{
		if (d.wantend != 1 && d.wantend != 4)
		{
			sprintf(buf, "£«¡ã¤§«e¤w¸g¦³±B¬ù¤F¡A±z½T©w­n¸Ñ°£ÂÂ±B¬ù¡A§ï©w¥ß±B¬ù¶Ü¡H[y/N]: ");
			getdata(b_lines - 1, 1, buf, ans, 2, 1, 0);
			if (ans[0] != 'y' && ans[0] != 'Y')
			{
				d.social += 10;
				vmsg("ÁÙ¬Oºû«ùÂÂ±B¬ù¦n¤F..");
				return 0;
			}
			d.social -= rand() % 50 + 100;
		}
		d.charm -= rand() % 5 + 20;
		d.lover = who + 3;
		d.relation -= 20;
		if (d.relation < 0)
			d.relation = 0;
		if (d.wantend < 4)
			d.wantend = 2;
		else
			d.wantend = 5;
		vmsg("§Ú·Q¹ï¤è¬O¤@­Ó«Ü¦nªº¦ñ«Q..");
		now = time(0);
		sprintf(buf, "[1;37m%s %-11sªº¤pÂû [%s] ±µ¨ü¤F %s ªº¨D±B[0m\n", Cdate(&now), cuser.userid, d.name, name[who][d.sex-1]);
		pip_log_record(buf);
	}
	else
	{
		d.charm += rand() % 5 + 20;
		d.relation += 20;
		if (d.wantend == 1 || d.wantend == 4)
			vmsg("§ÚÁÙ¦~»´  ¤ß±¡ÁÙ¤£©w...");
		else
			vmsg("§Ú¦­¤w¦³±B¬ù¤F..¹ï¤£°_...");
	}
	d.money += money;
	return 0;
}

int pip_results_show()  /*¦¬Ã¬©u*/
{
	char *showname[5] = {"  ", "ªZ°«¤j·|", "ÃÀ³N¤j®i", "¬Ó®a»R·|", "²i¶¹¤jÁÉ"};
	char buf[256];
	int pipkey, i = 0;
	int winorlost = 0;
	int a, b[3][2], c[3] = {0, 0, 0};

	clear();
	move(10, 14);
	prints("[1;33m¥m©N¥m©N¡ã ¨¯­Wªº¶l®tÀ°§Ú­Ì°e«H¨Ó¤F³á...[0m");
	vmsg("¶â  §â«H¥´¶}¬Ý¬Ý§a...");
	clear();
	show_resultshow_pic(0);
	sprintf(buf, "[A]%s [B]%s [C]%s [D]%s [Q]©ñ±ó:", showname[1], showname[2], showname[3], showname[4]);
	move(b_lines, 0);
	prints(buf);
	do
	{
		pipkey = vkey();
	}
	while (pipkey != 'q' && pipkey != 'Q' && pipkey != 'A' && pipkey != 'a' &&
		   pipkey != 'B' && pipkey != 'b' && pipkey != 'C' && pipkey != 'c' &&
		   pipkey != 'D' && pipkey != 'd');
	a = rand() % 4 + 1;
	b[0][0] = a - 1;
	b[1][0] = a + 1;
	b[2][0] = a;
	switch (pipkey)
	{
	case 'A':
	case 'a':
		vmsg("¤µ¦~¦@¦³¥|¤H°ÑÁÉ¡ã²{¦b¤ñÁÉ¶}©l");
		for (i = 0;i < 3;i++)
		{
			a = 0;
			b[i][1] = 0;
			sprintf(buf, "§Aªº²Ä%d­Ó¹ï¤â¬O%s", i + 1, resultmanlist[b[i][0]].name);
			vmsg(buf);
			a = pip_vs_man(b[i][0], resultmanlist, 2);
			if (a == 1)
				b[i][1] = 1;/*¹ï¤è¿é¤F*/
			winorlost += a;
			d.death = 0;
		}
		switch (winorlost)
		{
		case 3:
			pip_results_show_ending(3, 1, b[1][0], b[0][0], b[2][0]);
			d.hexp += rand() % 10 + 50;
			break;
		case 2:
			if (b[0][1] != 1)
			{
				c[0] = b[0][0];
				c[1] = b[1][0];
				c[2] = b[2][0];
			}
			else if (b[1][1] != 1)
			{
				c[0] = b[1][0];
				c[1] = b[2][0];
				c[2] = b[0][0];
			}
			else if (b[2][1] != 1)
			{
				c[0] = b[2][0];
				c[1] = b[0][0];
				c[2] = b[1][0];
			}
			pip_results_show_ending(2, 1, c[0], c[1], c[2]);
			d.hexp += rand() % 10 + 30;
			break;
		case 1:
			if (b[0][1] == 1)
			{
				c[0] = b[2][0];
				c[1] = b[1][0];
				c[2] = b[0][0];
			}
			else if (b[1][1] == 1)
			{
				c[0] = b[0][0];
				c[1] = b[2][0];
				c[2] = b[1][0];
			}
			else if (b[2][1] == 1)
			{
				c[0] = b[1][0];
				c[1] = b[0][0];
				c[2] = b[2][0];
			}
			pip_results_show_ending(1, 1, c[0], c[1], c[2]);
			d.hexp += rand() % 10 + 10;
			break;
		case 0:
			pip_results_show_ending(0, 1, b[0][0], b[1][0], b[2][0]);
			d.hexp -= rand() % 10 + 10;
			break;
		}
		break;
	case 'B':
	case 'b':
		vmsg("¤µ¦~¦@¦³¥|¤H°ÑÁÉ¡ã²{¦b¤ñÁÉ¶}©l");
		show_resultshow_pic(21);
		vmsg("¤ñÁÉ±¡§Î");
		if ((d.art*2 + d.character) / 400 >= 5)
		{
			winorlost = 3;
		}
		else if ((d.art*2 + d.character) / 400 >= 4)
		{
			winorlost = 2;
		}
		else if ((d.art*2 + d.character) / 400 >= 3)
		{
			winorlost = 1;
		}
		else
		{
			winorlost = 0;
		}
		pip_results_show_ending(winorlost, 2, rand() % 2, rand() % 2 + 2, rand() % 2 + 4);
		d.art += rand() % 10 + 20 * winorlost;
		d.character += rand() % 10 + 20 * winorlost;
		break;
	case 'C':
	case 'c':
		vmsg("¤µ¦~¦@¦³¥|¤H°ÑÁÉ¡ã²{¦b¤ñÁÉ¶}©l");
		if ((d.art*2 + d.charm) / 400 >= 5)
		{
			winorlost = 3;
		}
		else if ((d.art*2 + d.charm) / 400 >= 4)
		{
			winorlost = 2;
		}
		else if ((d.art*2 + d.charm) / 400 >= 3)
		{
			winorlost = 1;
		}
		else
		{
			winorlost = 0;
		}
		d.art += rand() % 10 + 20 * winorlost;
		d.charm += rand() % 10 + 20 * winorlost;
		pip_results_show_ending(winorlost, 3, rand() % 2, rand() % 2 + 4, rand() % 2 + 2);
		break;
	case 'D':
	case 'd':
		vmsg("¤µ¦~¦@¦³¥|¤H°ÑÁÉ¡ã²{¦b¤ñÁÉ¶}©l");
		if ((d.affect + d.cookskill*2) / 200 >= 4)
		{
			winorlost = 3;
		}
		else if ((d.affect + d.cookskill*2) / 200 >= 3)
		{
			winorlost = 2;
		}
		else if ((d.affect + d.cookskill*2) / 200 >= 2)
		{
			winorlost = 1;
		}
		else
		{
			winorlost = 0;
		}
		d.cookskill += rand() % 10 + 20 * winorlost;
		d.family += rand() % 10 + 20 * winorlost;
		pip_results_show_ending(winorlost, 4, rand() % 2 + 2, rand() % 2, rand() % 2 + 4);
		break;
	case 'Q':
	case 'q':
		vmsg("¤µ¦~¤£°Ñ¥[°Õ.....:(");
		d.happy -= rand() % 10 + 10;
		d.satisfy -= rand() % 10 + 10;
		d.relation -= rand() % 10;
		break;
	}
	if (pipkey != 'Q' && pipkey != 'q')
	{
		d.tired = 0;
		d.hp = d.maxhp;
		d.happy += rand() % 20;
		d.satisfy += rand() % 20;
		d.relation += rand() % 10;
	}
	return 0;
}

int pip_results_show_ending(winorlost, mode, a, b, c)
int winorlost, mode, a, b, c;
{
	char *gamename[5] = {"  ", "ªZ°«¤j·|", "ÃÀ³N¤j®i", "¬Ó®a»R·|", "²i¶¹¤jÁÉ"};
	int resultmoney[4] = {0, 3000, 5000, 8000};
	char name1[25], name2[25], name3[25], name4[25];
	char buf[256];

	if (winorlost == 3)
	{
		strcpy(name1, d.name);
		strcpy(name2, resultmanlist[a].name);
		strcpy(name3, resultmanlist[b].name);
		strcpy(name4, resultmanlist[c].name);
	}
	else if (winorlost == 2)
	{
		strcpy(name1, resultmanlist[a].name);
		strcpy(name2, d.name);
		strcpy(name3, resultmanlist[b].name);
		strcpy(name4, resultmanlist[c].name);
	}
	else if (winorlost == 1)
	{
		strcpy(name1, resultmanlist[a].name);
		strcpy(name2, resultmanlist[b].name);
		strcpy(name3, d.name);
		strcpy(name4, resultmanlist[c].name);
	}
	else
	{
		strcpy(name1, resultmanlist[a].name);
		strcpy(name2, resultmanlist[b].name);
		strcpy(name3, resultmanlist[c].name);
		strcpy(name4, d.name);
	}
	clear();
	move(6, 13);
	prints("[1;37m¡ã¡ã¡ã [32m¥»©¡ %s µ²ªG´¦¾å [37m¡ã¡ã¡ã[0m", gamename[mode]);
	move(8, 15);
	prints("[1;41m «a­x [0;1m¡ã [1;33m%-10s[36m  ¼úª÷ %d[0m", name1, resultmoney[3]);
	move(10, 15);
	prints("[1;41m ¨È­x [0;1m¡ã [1;33m%-10s[36m  ¼úª÷ %d[0m", name2, resultmoney[2]);
	move(12, 15);
	prints("[1;41m ©u­x [0;1m¡ã [1;33m%-10s[36m  ¼úª÷ %d[0m", name3, resultmoney[1]);
	move(14, 15);
	prints("[1;41m ³Ì«á [0;1m¡ã [1;33m%-10s[36m [0m", name4);
	sprintf(buf, "¤µ¦~ªº%sµ²§ôÅo «á¦~¦A¨Ó§a..", gamename[mode]);
	d.money += resultmoney[winorlost];
	vmsg(buf);
	return 0;
}

/*---------------------------------------------------------------------------*/
/* ¤G¥NÂû¨ç¦¡¯S°Ï                                                            */
/*---------------------------------------------------------------------------*/
static int
twice(x, max, min)
int x, max, min;
{
	float a, b;
	int y;
	a = (2 * max + 3 * min) / 21000;
	b = (max - min - 10000 * a) / 100;
	y = (int) a * x * x + b * x + min;
	return y;
}

static void
pip_load_mob(fpath)
char *fpath;
{
	FILE *fp;
	int max, i;
	char buf[128];
	playrule *p;
	if (badmanlist)
		free(badmanlist);
	if (fp = fopen(fpath, "r"))
	{
		fscanf(fp, "%d%s", &max, buf);
		p = badmanlist = (playrule *)malloc((max + 1) * sizeof(playrule));
		memset(badmanlist, 0, (max + 1)*sizeof(playrule));
		for (i = 0;i < max;i++)
		{
			fscanf(fp, "%s%d%d%d%d%d%d%s%d", p[i].name, &p[i].hp, &p[i].mp, &p[i].attack,
				   &p[i].resist, &p[i].speed, &p[i].money, p[i].special, &p[i].map);
			p[i].maxhp = p[i].maxmp = p[i].death = 0;
		}
		fclose(fp);
	}
}

static void
pip_load_mobset(fpath)
char *fpath;
{
	FILE *fp;
	int i;
	if (fp = fopen(fpath, "r"))
	{
		for (i = 0;i <= 21;i++)
		{
			fscanf(fp, "%d%d", &mob[i][0], &mob[i][1]);
		}
		fclose(fp);
	}
}

static void
levelswap(cur, num)
int *cur, num;
{
	if (*cur > num) *cur = num;
}

static void
pip_check_levelup()
{
	int lv;
	if (d.chickenmode == 1)
	{
		lv = d.level;
		levelswap(&d.maxhp, twice(lv, ml[1].maxhp, ml[0].maxhp));
		levelswap(&d.wrist, twice(lv, ml[1].wrist, ml[0].wrist));
		levelswap(&d.attack, twice(lv, ml[1].attack, ml[0].attack));
		levelswap(&d.resist, twice(lv, ml[1].resist, ml[0].resist));
		levelswap(&d.speed, twice(lv, ml[1].speed, ml[0].speed));
		levelswap(&d.maxmp, twice(lv, ml[1].maxmp, ml[0].maxmp));
		levelswap(&d.hskill, twice(lv, ml[1].hskill, ml[0].hskill));
		levelswap(&d.mskill, twice(lv, ml[1].mskill, ml[0].mskill));
	}
}

static void
pip_check_level()
{
	if (d.exp >= twice(d.level, 10000000, 100))
	{
		d.level++;
		d.exp = 0;
	}
	if (d.level > 100)
		d.level = 100;
}

static void
pip_load_levelup(fpath)
char *fpath;
{
	FILE *fp;
	int i, temp;
	char buf[128];
	if (fp = fopen(fpath, "r"))
	{
		fscanf(fp, "%s", buf);
		for (i = 0;i <= 1;i++)
		{
			fscanf(fp, "%d%d%d%d%d%d%d%d%d", &temp, &ml[i].maxhp, &ml[i].wrist,
				   &ml[i].maxmp, &ml[i].attack, &ml[i].resist, &ml[i].speed,
				   &ml[i].hskill, &ml[i].mskill);
		}
		fclose(fp);
	}
}

#ifdef HAVE_PIP_FIGHT
static void
pip_fight_menu(mode)
int mode;
{
	char inbuf1[20];
	char inbuf2[20];
	char buf[256];

	int m, color;
	int age;
	int color1, color2, color3, color4;
	char yo[12][5] = {"½Ï¥Í", "À¦¨à", "¥®¨à", "¨àµ£", "¤Ö¦~", "«C¦~",
					  "¦¨¦~", "§§¦~", "§ó¦~", "¦Ñ¦~", "¥jµ}", "¯«¥P"
					 };

	color1 = color2 = color3 = color4 = 37;
	move(1, 0);
	m = (time(0) - start_time + d.bbtime) / 60 / 30; /* ¤@·³ */
	/*ªø¤j¤@·³®Éªº¼W¥[§ïÅÜ­È*/
	color = 37;

	if (m == 0) /*½Ï¥Í*/
		age = 0;
	else if (m == 1) /*À¦¨à*/
		age = 1;
	else if (m >= 2 && m <= 5) /*¥®¨à*/
		age = 2;
	else if (m >= 6 && m <= 12) /*¨àµ£*/
		age = 3;
	else if (m >= 13 && m <= 15) /*¤Ö¦~*/
		age = 4;
	else if (m >= 16 && m <= 18) /*«C¦~*/
		age = 5;
	else if (m >= 19 && m <= 35) /*¦¨¦~*/
		age = 6;
	else if (m >= 36 && m <= 45) /*§§¦~*/
		age = 7;
	else if (m >= 45 && m <= 60) /*§ó¦~*/
		age = 8;
	else if (m >= 60 && m <= 70) /*¦Ñ¦~*/
		age = 9;
	else if (m >= 70 && m <= 100) /*¥jµ}*/
		age = 10;
	else if (m > 100) /*¯«¥P*/
		age = 11;
	clear();
	move(0, 0);
	if (d.sex == 1)
		sprintf(buf, "[1;41m  " NICKNAME PIPNAME " ¡ã [32m¡ñ [37m%-15s                                           [0m", d.name);
	else if (d.sex == 2)
		sprintf(buf, "[1;41m  " NICKNAME PIPNAME " ¡ã [33m¡ð [37m%-15s                                           [0m", d.name);
	else
		sprintf(buf, "[1;41m  " NICKNAME PIPNAME " ¡ã [34m¡H [37m%-15s                                           [0m", d.name);
	prints(buf);

	move(1, 0);
	if (d.money <= 100)
		color1 = 31;
	else if (d.money > 100 && d.money <= 500)
		color1 = 33;
	else
		color1 = 37;
	sprintf(inbuf1, "%02d/%02d/%02d", (d.year - 11) % 100, d.month, d.day);
	sprintf(buf
			, " [1;32m[ª¬  ºA][37m %-5s     [32m[¥Í  ¤é][37m %-9s [32m[¦~  ÄÖ][37m %-5d     [32m[ª÷  ¿ú][%dm %-8d [m"
			, yo[age], inbuf1, m, color1, d.money);
	prints(buf);

	move(2, 0);

	if ((d.hp*100 / d.maxhp) <= 20)
		color1 = 31;
	else if ((d.hp*100 / d.maxhp) <= 40 && (d.hp*100 / d.maxhp) > 20)
		color1 = 33;
	else
		color1 = 37;
	if (d.maxmp == 0)
		color2 = 37;
	else if ((d.mp*100 / d.maxmp) <= 20)
		color2 = 31;
	else if ((d.mp*100 / d.maxmp) <= 40 && (d.mp*100 / d.maxmp) > 20)
		color2 = 33;
	else
		color2 = 37;

	if (d.tired >= 80)
		color3 = 31;
	else if (d.tired < 80 && d.tired >= 60)
		color3 = 33;
	else
		color3 = 37;

	sprintf(inbuf1, "%d/%d", d.hp, d.maxhp);
	sprintf(inbuf2, "%d/%d", d.mp, d.maxmp);
	sprintf(buf
			, " [1;32m[¥Í  ©R][%dm %-10s[32m[ªk  ¤O][%dm %-10s[32m[Åé  ­«][37m %-5d     [32m[¯h  ³Ò][%dm %-4d[0m "
			, color1, inbuf1, color2, inbuf2, d.weight, color3, d.tired);
	prints(buf);

	move(3, 0);
	if (d.shit >= 80)
		color1 = 31;
	else if (d.shit < 80 && d.shit >= 60)
		color1 = 33;
	else
		color1 = 37;
	if (d.sick >= 75)
		color2 = 31;
	else if (d.sick < 75 && d.sick >= 50)
		color2 = 33;
	else
		color2 = 37;
	if (d.happy <= 20)
		color3 = 31;
	else if (d.happy > 20 && d.happy <= 40)
		color3 = 33;
	else
		color3 = 37;
	if (d.satisfy <= 20)
		color4 = 31;
	else if (d.satisfy > 20 && d.satisfy <= 40)
		color4 = 33;
	else
		color4 = 37;
	sprintf(buf
			, " [1;32m[Å¼  Å¼][%dm %-4d      [32m[¯f  ®ð][%dm %-4d      [32m[§Ö¼Ö«×][%dm %-4d      [32m[º¡·N«×][%dm %-4d[0m"
			, color1, d.shit, color2, d.sick, color3, d.happy, color4, d.satisfy);
	prints(buf);
	if (mode == 1)/*Áý­¹*/
	{
		move(4, 0);
		if (d.food == 0)
			color1 = 31;
		else if (d.food <= 5 && d.food > 0)
			color1 = 33;
		else
			color1 = 37;
		if (d.cookie == 0)
			color2 = 31;
		else if (d.cookie <= 5 && d.cookie > 0)
			color2 = 33;
		else
			color2 = 37;
		if (d.bighp == 0)
			color3 = 31;
		else if (d.bighp <= 2 && d.bighp > 0)
			color3 = 33;
		else
			color3 = 37;
		if (d.medicine == 0)
			color4 = 31;
		else if (d.medicine <= 5 && d.medicine > 0)
			color4 = 33;
		else
			color4 = 37;
		sprintf(buf
				, " [1;36m[­¹ª«][%dm%-7d[36m[¹s­¹][%dm%-7d[36m[¸É¤Y][%dm%-7d[36m[ÆFªÛ][%dm%-7d[36m[¤H°Ñ][37m%-7d[36m[³·½¬][37m%-7d[0m"
				, color1, d.food, color2, d.cookie, color3, d.bighp, color4, d.medicine, d.ginseng, d.snowgrass);
		prints(buf);

	}
	move(5, 0);
	prints("[1;%dm¢z¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢{[m", color);
	move(6, 0);
	switch (age)
	{
	case 0:
	case 1:
	case 2:
		if (d.weight <= (60 + 10*m - 30))
			show_basic_pic(1);
		else if (d.weight > (60 + 10*m - 30) && d.weight < (60 + 10*m + 30))
			show_basic_pic(2);
		else if (d.weight >= (60 + 10*m + 30))
			show_basic_pic(3);
		break;
	case 3:
	case 4:
		if (d.weight <= (60 + 10*m - 30))
			show_basic_pic(4);
		else if (d.weight > (60 + 10*m - 30) && d.weight < (60 + 10*m + 30))
			show_basic_pic(5);
		else if (d.weight >= (60 + 10*m + 30))
			show_basic_pic(6);
		break;
	case 5:
	case 6:
		if (d.weight <= (60 + 10*m - 30))
			show_basic_pic(7);
		else if (d.weight > (60 + 10*m - 30) && d.weight < (60 + 10*m + 30))
			show_basic_pic(8);
		else if (d.weight >= (60 + 10*m + 30))
			show_basic_pic(9);
		break;
	case 7:
	case 8:
		if (d.weight <= (60 + 10*m - 30))
			show_basic_pic(10);
		else if (d.weight > (60 + 10*m - 30) && d.weight < (60 + 10*m + 30))
			show_basic_pic(11);
		else if (d.weight >= (60 + 10*m + 30))
			show_basic_pic(12);
		break;
	case 9:
		show_basic_pic(13);
		break;
	case 10:
	case 11:
		show_basic_pic(16);
		break;
	}


	move(18, 0);
	prints("[1;%dm¢|¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢}[m", color);
	move(19, 0);
	prints(" [1;34m¢w[37;44m  ª¬ ºA  [0;1;34m¢w[0m");
	move(20, 0);
	prints(" ¾Ô°«¤¤.............\n");

}
#endif
#ifdef	HAVE_PIP_FIGHT
static int pip_fight_feed()     /* Áý­¹*/
{
	time_t now;
	char buf[256];
	int pipkey;

	d.nodone = 1;

	do
	{
		if (d.death == 1 || d.death == 2 || d.death == 3)
			return 0;
		pip_fight_menu(1);
		move(b_lines -2, 0);
		clrtoeol();
		move(b_lines - 2, 1);
		sprintf(buf, "%s¸Ó°µ¤°»ò¨Æ©O?", d.name);
		prints(buf);
		now = time(0);
		move(b_lines, 0);
		clrtoeol();
		move(b_lines, 0);
		prints("[1;44;37m  ¶¼­¹¿ï³æ  [46m[1]¦Y¶º [2]¹s­¹ [3]¸É¤Y [4]ÆFªÛ [5]¤Hçx [6]³·½¬ [Q]¸õ¥X¡G         [m");
		pipkey = vkey();

		switch (pipkey)
		{
		case '1':
			if (d.food <= 0)
			{
				move(b_lines, 0);
				vmsg("¨S¦³­¹ª«Åo..§Ö¥h¶R§a¡I");
				break;
			}
			move(4, 0);
			if ((d.bbtime / 60 / 30) < 3)
				show_feed_pic(0);
			else
				show_feed_pic(1);
			d.food--;
			d.hp += 50;
			if (d.hp >= d.maxhp)
			{
				d.hp = d.maxhp;
				d.weight += rand() % 2;
			}
			d.nodone = 0;
			vmsg("¨C¦Y¤@¦¸­¹ª«·|«ì´_Åé¤O50³á!");
			break;

		case '2':
			if (d.cookie <= 0)
			{
				move(b_lines, 0);
				vmsg("¹s­¹¦Y¥úÅo..§Ö¥h¶R§a¡I");
				break;
			}
			move(4, 0);
			d.cookie--;
			d.hp += 100;
			if (d.hp >= d.maxhp)
			{
				d.hp = d.maxhp;
				d.weight += (rand() % 2 + 2);
			}
			else
			{
				d.weight += (rand() % 2 + 1);
			}
			if (rand() % 2 > 0)
				show_feed_pic(2);
			else
				show_feed_pic(3);
			d.happy += (rand() % 3 + 4);
			d.satisfy += rand() % 3 + 2;
			d.nodone = 0;
			vmsg("¦Y¹s­¹®e©ö­D³á...");
			break;

		case '3':
			if (d.bighp <= 0)
			{
				move(b_lines, 0);
				vmsg("¨S¦³¤j¸É¤Y¤F­C! §Ö¶R§a..");
				break;
			}
			d.bighp--;
			d.hp += 600;
			d.tired -= 20;
			d.weight += rand() % 2;
			move(4, 0);
			show_feed_pic(4);
			d.nodone = 0;
			vmsg("¸É¤Y..¶W·¥´Îªº­ò...");
			break;

		case '4':
			if (d.medicine <= 0)
			{
				move(b_lines, 0);
				vmsg("¨S¦³ÆFªÛÅo..§Ö¥h¶R§a¡I");
				break;
			}
			move(4, 0);
			show_feed_pic(1);
			d.medicine--;
			d.mp += 50;
			if (d.mp >= d.maxmp)
			{
				d.mp = d.maxmp;
			}
			d.nodone = 0;
			vmsg("¨C¦Y¤@¦¸ÆFªÛ·|«ì´_ªk¤O50³á!");
			break;

		case '5':
			if (d.ginseng <= 0)
			{
				move(b_lines, 0);
				vmsg("¨S¦³¤d¦~¤Hçx­C! §Ö¶R§a..");
				break;
			}
			d.ginseng--;
			d.mp += 500;
			d.tired -= 20;
			move(4, 0);
			show_feed_pic(1);
			d.nodone = 0;
			vmsg("¤d¦~¤Hçx..¶W·¥´Îªº­ò...");
			break;

		case '6':
			if (d.snowgrass <= 0)
			{
				move(b_lines, 0);
				vmsg("¨S¦³¤Ñ¤s³·½¬­C! §Ö¶R§a..");
				break;
			}
			d.snowgrass--;
			d.mp = d.maxmp;
			d.hp = d.maxhp;
			d.tired -= 0;
			d.sick = 0;
			move(4, 0);
			show_feed_pic(1);
			d.nodone = 0;
			vmsg("¤Ñ¤s³·½¬..¶W·¥´Îªº­ò...");
			break;
case 'Q': case 'q': case KEY_LEFT:
			pipkey = '7';
			break;
		}
	}
	while (pipkey > '7' || pipkey < '0');

	return 0;
}
#endif
