/* ----------------------------------- */
/* pip.c  �i�p���{��                   */
/* ��@��: dsyan   ��g��: fennet      */
/* �Ϲ� by tiball.bbs@bbs.nhctc.edu.tw */
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

#define	PIPNAME		"�d����"

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

/*�t�ο��*/
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

/*�C���D�{��*/
int p_pipple()
{
	FILE *fs;
	int pipkey;
	char genbuf[200];

	utmp_mode(M_CHICKEN);
	more("game/pipgame/pip.welcome", NULL);
	vs_head("�q�l�i�p��", BoardName);
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

/*�ɶ���ܪk*/
char*
dsyan_time(const time_t *t)
{
	struct tm *tp;
	static char ans[9];

	tp = localtime(t);
	sprintf(ans, "%02d/%02d/%02d", (tp->tm_year) % 100, tp->tm_mon + 1, tp->tm_mday);
	return ans;
}

/*�s�C�����]�w*/
void pip_new_game()
{
	char buf[256];
	time_t now;
	char *pipsex[3] = {"�H", "��", "��"};
	struct tm *ptime;
	ptime = localtime(&now);

	if (d.death == 1 && !(!d.name[0]))
	{
		clear();
		vs_head(NICKNAME PIPNAME, BoardName);
		move(4, 6);
		prints("�w��Ө� [1;5;33m" NICKNAME "�ͪ���ެ�s�|[0m");
		move(6, 6);
		prints("�g�ڭ̽լd���  ���e�A���i�L�p����  �i�O�Q�A�i���F...");
		move(8, 6);
		if (d.liveagain < 4)
		{
			prints("�ڭ̥i�H���A���p���_��  ���O�ݭn�I�X�@�I�N��");
			getdata(10, 6, "�A�n�ڭ����L���Ͷ�? [y/N]: ", buf, 2, DOECHO, 0);
			if (buf[0] == 'y' || buf[0] == 'Y')
			{
				pip_live_again();
			}
		}
		else if (d.liveagain >= 4)
		{
			prints("�i�O�A�_����N�Ӧh���F  �p�����W���O�}�M����");
			move(10, 6);
			prints("�ڭ̧䤣��i�H��N���a��F  �ҥH....");
			vmsg("���s�A�ӧa....��....");
		}
	}
	if (d.death != 0 || !d.name[0])
	{
		clear();
		vs_head(NICKNAME PIPNAME, BoardName);
		/*�p���R�W*/
		getdata(2, 0, "���p�����Ӧnť���W�r�a(�Ф��n���Ů�): ", buf, 11, DOECHO, 0);
		if (!buf[0]) 
			return;
		strcpy(d.name, buf);
		/*1:�� 2:�� */
		getdata(4, 3, "[Boy]�p������ or [Girl]�p������ [b/G]: ", buf, 2, DOECHO, 0);
		if (buf[0] == 'b' || buf[0] == 'B')
		{
			d.sex = 1;
		}
		else
		{
			d.sex = 2;
		}
		move(6, 3);
		prints(NICKNAME PIPNAME "���C���{��������ت��k");
		move(7, 3);
		prints("�靈�����|�b�p��20���ɵ����C���A�çi���p�����򪺵o�i");
		move(8, 3);
		prints("��S�������h�@���i��p�����`�~�����C��....");
		/*1:���n�B���B 4:�n�B���B */
		getdata(9, 3, "�A�Ʊ�p���C���O�_�n��20������? [Y/n]: ", buf, 2, DOECHO, 0);
		if (buf[0] == 'n' || buf[0] == 'N')
		{
			d.wantend = 1;
		}
		else
		{
			d.wantend = 4;
		}
		/*�}�Y�e��*/
		show_basic_pic(0);
		vmsg("�p���ש�ϥͤF�A�Цn�n�R�L....");

		/*�}�Y�]�w*/
		now = time(0);
		strcpy(d.birth, dsyan_time(&now));
		d.bbtime = 0;

		/*�򥻸��*/
		d.year = ptime->tm_year;
		d.month = ptime->tm_mon + 1;
		d.day = ptime->tm_mday;
		d.death = d.nodone = d.relation = 0;
		d.liveagain = d.level = d.exp = d.dataE = 0;
		d.chickenmode = 1;

		/*����Ѽ�*/
		d.hp = rand() % 15 + START_HP;
		d.maxhp = rand() % 20 + START_HP;
		if (d.hp > d.maxhp) d.hp = d.maxhp;
		d.weight = rand() % 10 + 50;
		d.tired = d.sick = d.shit = d.wrist = 0;
		d.bodyA = d.bodyB = d.bodyC = d.bodyD = d.bodyE = 0;

		/*�����Ѽ�*/
		d.social = d.family = d.hexp = d.mexp = 0;
		d.tmpA = d.tmpB = d.tmpC = d.tmpD = d.tmpE = 0;

		/*�԰��Ѽ�*/
		d.mp = d.maxmp = d.attack = d.resist = d.speed = d.hskill = d.mskill = d.mresist = 0;
		d.magicmode = d.specialmagic = d.fightC = d.fightD = d.fightE = 0;

		/*�Z���Ѽ�*/
		d.weaponhead = d.weaponrhand = d.weaponlhand = d.weaponbody = d.weaponfoot = 0;
		d.weaponA = d.weaponB = d.weaponC = d.weaponD = d.weaponE = 0;

		/*��O�Ѽ�*/
		d.toman = d.character = d.love = d.wisdom = d.art = d.etchics = 0;
		d.brave = d.homework = d.charm = d.manners = d.speech = d.cookskill = 0;
		d.learnA = d.learnB = d.learnC = d.learnD = d.learnE = 0;

		/*���A�ƭ�*/
		d.happy = rand() % 10 + START_HAPPY;
		d.satisfy = rand() % 10 + START_SATISFY;
		d.fallinlove = d.belief = d.offense = d.affect = 0;
		d.stateA = d.stateB = d.stateC = d.stateD = d.stateE = 0;

		/*�����Ѽ�:���� �s�� �ī~ �j�ɤY*/
		d.food = START_FOOD;
		d.medicine = d.cookie = d.bighp = 2;
		d.ginseng = d.snowgrass = d.eatC = d.eatD = d.eatE = 0;

		/*���~�Ѽ�:�� ����*/
		d.book = d.playtool = 0;
		d.money = START_MONEY;
		d.thingA = d.thingB = d.thingC = d.thingD = d.thingE = 0;

		/*�q���Ѽ�:Ĺ �t*/
		d.winn = d.losee = 0;

		/*�Ѩ�����*/
		d.royalA = d.royalB = d.royalC = d.royalD = d.royalE = 0;
		d.royalF = d.royalG = d.royalH = d.royalI = d.royalJ = 0;
		d.seeroyalJ = 1;
		d.seeA = d.seeB = d.seeC = d.seeD = d.seeE;
		/*�����D�B�R�H*/
		d.lover = 0;
		/*0:�S�� 1:�]�� 2:�s�� 3:A 4:B 5:C 6:D 7:E */
		d.classA = d.classB = d.classC = d.classD = d.classE = 0;
		d.classF = d.classG = d.classH = d.classI = d.classJ = 0;
		d.classK = d.classL = d.classM = d.classN = d.classO = 0;

		d.workA = d.workB = d.workC = d.workD = d.workE = 0;
		d.workF = d.workG = d.workH = d.workI = d.workJ = 0;
		d.workK = d.workL = d.workM = d.workN = d.workO = 0;
		d.workP = d.workQ = d.workR = d.workS = d.workT = 0;
		d.workU = d.workV = d.workW = d.workX = d.workY = d.workZ = 0;
		/*�i���O��*/
		now = time(0);
		sprintf(buf, "[1;36m%s %-11s�i�F�@���s [%s] �� %s �p�� [0m\n", Cdate(&now), cuser.userid, d.name, pipsex[d.sex]);
		pip_log_record(buf);
	}
	pip_write_file();
}

/*�p�����`�禡*/
void
pipdie(msg, mode)
char *msg;
int mode;
{
	char genbuf[200];
	time_t now;
	clear();
	vs_head("�q�l�i�p��", BoardName);
	if (mode == 1)
	{
		show_die_pic(1);
		vmsg("�����ӱa���p���F");
		clear();
		vs_head("�q�l�i�p��", BoardName);
		show_die_pic(2);
		move(14, 20);
		prints("�i�����p��[1;31m%s[m", msg);
		vmsg(NICKNAME "�s����....");
	}
	else if (mode == 2)
	{
		show_die_pic(3);
		vmsg("����..�ڳQ���F.....");
	}
	else if (mode == 3)
	{
		show_die_pic(0);
		vmsg("�C�������o..");
	}

	now = time(0);
	sprintf(genbuf, "[1;31m%s %-11s���p�� [%s] %s[m\n", Cdate(&now), cuser.userid, d.name, msg);
	pip_log_record(genbuf);
	pip_write_file();
}


/*pro:���v base:���� mode:���� mul:�[�v100=1 cal:�[��*/
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
/*�D�e���M���                                                               */
/*---------------------------------------------------------------------------*/

char *menuname[8][2] =
{
	{"             ",
		"[1;44;37m ��� [46m[1]�� [2]�}�� [3]�צ� [4]���� [5]���u [6]�S�� [7]�t�� [Q]���}          [m"},

	{"             ",
	 "[1;44;37m  �򥻿��  [46m[1]���� [2]�M�� [3]�� [4]�˿� [5]����         [Q]���X�G          [m"},

	{"[1;44;37m �}�� [46m�i��`�Ϋ~�j[1]�K�Q�ө� [2]" NICKNAME "�ľQ [3]�]�̮ѧ�                        [m",
	 "[1;44;37m ��� [46m�i�Z���ʳf�j[A]�Y���˳� [B]�k��˳� [C]����˳� [D]����˳� [E]�}���˳�  [m"},

	{"[1;44;37m �צ� [46m[A]���(%d) [B]�ֵ�(%d) [C]����(%d) [D]�x��(%d) [E]�C�N(%d)                   [m",
	 "[1;44;37m ��� [46m[F]�氫(%d) [G]�]�k(%d) [H]§��(%d) [I]ø�e(%d) [J]�R��(%d) [Q]���X�G         [m"},

	{"   ",
	 "[1;44;37m  ���ֿ��  [46m[1]���B [2]�B�� [3]���| [4]�q�� [5]�ȹC [6]���~ [7]�ۺq [Q]���X�G  [m"},

	{"[1;44;37m ���u [46m[A]�a�� [B]�O�i [C]���] [D]�A�� [E]�\\�U [F]�а� [G]�a�u [H]���          [m",
	 "[1;44;37m ��� [46m[I]���v [J]�y�H [K]�u�a [L]�u�� [M]�a�� [N]�s�a [O]�s�� [P]�]�`�| [Q]���X[m"},

	{"[1;44;37m �S�� [46m[1]" NICKNAME "��| [2]�A�n�p [3]�԰��צ� [4]���X�B�� [5]" NICKNAME "                [m",
	 "[1;44;37m ��� [46m                                                                  [Q]���X[m"},

	{"   ",
	 "[1;44;37m  �t�ο��  [46m[1]�ԲӸ�� [2]�p���ۥ� [3]�S�O�A�� [4]�x�s�i�� [5]Ū���i�� [Q]���X[m"}
};

/*�D���*/
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

/*�򥻿��*/
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

/*�ө����*/
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

/*�צ���*/
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

/*���ֿ��*/
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

/*���u���*/
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

/*�S����*/
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



/*����menu.c���\��*/
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
		/*�P�_�O�_���`  �����Y���^�W�@�h*/
		if (d.death == 1 || d.death == 2 || d.death == 3)
			return 0;
		/*�gpip_mainmenu�P�w��O�_���`*/
		if (pip_mainmenu(menumode))
			return 0;

		class1 = d.wisdom / 200 + 1;			/*���*/
		if (class1 > 5)  class1 = 5;
		class2 = (d.affect * 2 + d.wisdom + d.art * 2 + d.character) / 400 + 1; /*�ֵ�*/
		if (class2 > 5)  class2 = 5;
		class3 = (d.belief * 2 + d.wisdom) / 400 + 1;		/*����*/
		if (class3 > 5)  class3 = 5;
		class4 = (d.hskill * 2 + d.wisdom) / 400 + 1;		/*�x��*/
		if (class4 > 5)  class4 = 5;
		class5 = (d.hskill + d.attack) / 400 + 1;		/*�C�N*/
		if (class5 > 5)  class5 = 5;
		class6 = (d.hskill + d.resist) / 400 + 1;		/*�氫*/
		if (class6 > 5)  class6 = 5;
		class7 = (d.mskill + d.maxmp) / 400 + 1;		/*�]�k*/
		if (class7 > 5)  class7 = 5;
		class8 = (d.manners * 2 + d.character) / 400 + 1;	/*§��*/
		if (class8 > 5)  class8 = 5;
		class9 = (d.art * 2 + d.character) / 400 + 1; 		/*ø�e*/
		if (class9 > 5)  class9 = 5;
		class10 = (d.art * 2 + d.charm) / 400 + 1;		/*�R��*/
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
/* �򥻿��:���� �M�� �˿� ��                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/
int pip_main_menu()
{
	pip_do_menu(0, 0, pipmainlist);
	return 0;
}

/*---------------------------------------------------------------------------*/
/* �򥻿��:���� �M�� �˿� ��                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/
int pip_basic_menu()
{
	pip_do_menu(1, 0, pipbasiclist);
	return 0;
}

/*---------------------------------------------------------------------------*/
/* �ө����:���� �s�� �j�ɤY ���� �ѥ�                                       */
/*                                                                           */
/*---------------------------------------------------------------------------*/
int pip_store_menu()
{
	pip_do_menu(2, 1, pipstorelist);
	return 0;
}

/*---------------------------------------------------------------------------*/
/* �צ���:���� �m�Z �צ�                                                   */
/*                                                                           */
/*---------------------------------------------------------------------------*/
int pip_practice_menu()
{
	pip_do_menu(3, 3, pippracticelist);
	return 0;
}


/*---------------------------------------------------------------------------*/
/* ���ֿ��:���B �ȹC �B�� ���| �q��                                         */
/*                                                                           */
/*---------------------------------------------------------------------------*/
int pip_play_menu()
{
	pip_do_menu(4, 0, pipplaylist);
	return 0;
}

/*---------------------------------------------------------------------------*/
/* ���u���:�a�� �W�u �a�� �a�u                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/
int pip_job_menu()
{
	pip_do_menu(5, 2, pipjoblist);
	return 0;
}

/*---------------------------------------------------------------------------*/
/* �S����:�ݯf ��� �԰� ���X �¨�                                         */
/*                                                                           */
/*---------------------------------------------------------------------------*/
int pip_special_menu()
{
	pip_do_menu(6, 0, pipspeciallist);
	return 0;
}

/*---------------------------------------------------------------------------*/
/* �t�ο��:�ӤH���  �p�����  �S�O�A��                                     */
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
	char yo[12][5] = {"�ϥ�", "����", "����", "�ൣ", "�֦~", "�C�~",
					  "���~", "���~", "��~", "�Ѧ~", "�j�}", "���P"
					 };

	color1 = color2 = color3 = color4 = 37;
	move(1, 0);
	tm = (time(0) - start_time + d.bbtime) / 60 / 30; /* �@�� */
	tm1 = (time(0) - start_time + d.bbtime) / 60;
	m = d.bbtime / 60 / 30;
	m1 = d.bbtime / 60;
	/*���j�@���ɪ��W�[���ܭ�*/
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

		/*�O���}�l*/
		now = time(0);
		sprintf(genbuf, "[1;37m%s %-11s���p�� [%s] �� %d ���F [m\n", Cdate(&now), cuser.userid, d.name, m + 1);
		pip_log_record(genbuf);
		/*�O���פ�*/
		clear();
		vs_head("�q�l�i�p��", BoardName);
		show_basic_pic(20); /*�ͤ�ּ�*/
		vmsg("�p�����j�@���F..");
		/*����*/
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

	/*�Z�x*/
	if ((time(0) - start_time) >= 900)
	{
		d.seeroyalJ = 0;
	}

	if (m == 0) /*�ϥ�*/
		age = 0;
	else if (m == 1) /*����*/
		age = 1;
	else if (m >= 2 && m <= 5) /*����*/
		age = 2;
	else if (m >= 6 && m <= 12) /*�ൣ*/
		age = 3;
	else if (m >= 13 && m <= 15) /*�֦~*/
		age = 4;
	else if (m >= 16 && m <= 18) /*�C�~*/
		age = 5;
	else if (m >= 19 && m <= 35) /*���~*/
		age = 6;
	else if (m >= 36 && m <= 45) /*���~*/
		age = 7;
	else if (m >= 45 && m <= 60) /*��~*/
		age = 8;
	else if (m >= 60 && m <= 70) /*�Ѧ~*/
		age = 9;
	else if (m >= 70 && m <= 100) /*�j�}*/
		age = 10;
	else if (m > 100) /*���P*/
		age = 11;
	clear();
	/*vs_head("�q�l�i�p��", BoardName);*/
	move(0, 0);
	if (d.sex == 1)
		sprintf(buf, "[1;41m  " NICKNAME PIPNAME " �� [%s�N��] [32m�� [37m%-15s                                  [m", d.chickenmode ? "�G" : "�@", d.name);
	else if (d.sex == 2)
		sprintf(buf, "[1;41m  " NICKNAME PIPNAME " �� [%s�N��] [33m�� [37m%-15s                                  [m", d.chickenmode ? "�G" : "�@", d.name);
	else
		sprintf(buf, "[1;41m  " NICKNAME PIPNAME " �� [%s�N��] [34m�H [37m%-15s                                  [m", d.chickenmode ? "�G" : "�@", d.name);
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
			, " [1;32m[��  �A][37m %-5s     [32m[��  ��][37m %-9s [32m[�~  ��][37m %-5d     [32m[��  ��][%dm %-8d [m"
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
			, " [1;32m[��  �R][%dm %-10d[32m[�k  �O][%dm %-10d[32m[��  ��][37m %-5d     [32m[�h  ��][%dm %-4d[0m "
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
			, " [1;32m[�R MAX][37m %-10d[32m[�k MAX][37m %-10d[32m[ż���f][%dm %-4d[37m/[%dm%-4d [32m[�֡���][%dm %-4d[37m/[%dm%-4d[m"
			, d.maxhp, d.maxmp, color1, d.shit, color2, d.sick, color3, d.happy, color4, d.satisfy);
	prints(buf);
	if (mode == 0)  /*�D�n�e��*/
	{
		anynum = 0;
		anynum = rand() % 4;
		move(4, 0);
		if (anynum == 0)
			sprintf(buf, " [1;35m[������]:[31m����[36m��ܦM�I  [33m����[36m���ĵ�i  [37m�զ�[36m��ܦw��[0m");
		else if (anynum == 1)
			sprintf(buf, " [1;35m[������]:[37m�n�h�h�`�N�p�����h�ҫשM�f��  �H�K�֦��f��[0m");
		else if (anynum == 2)
			sprintf(buf, " [1;35m[������]:[37m�H�ɪ`�N�p�����ͩR�ƭȭ�![0m");
		else if (anynum == 3)
			sprintf(buf, " [1;35m[������]:[37m�ּּ֧֪��p���~�O���֪��p��.....[0m");
		prints(buf);
	}
	else if (mode == 1)/*����*/
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
				, " [1;36m[����][%dm%-7d[36m[�s��][%dm%-7d[36m[�ɤY][%dm%-7d[36m[�F��][%dm%-7d[36m[�H��][37m%-7d[36m[����][37m%-7d[0m"
				, color1, d.food, color2, d.cookie, color3, d.bighp, color4, d.medicine, d.ginseng, d.snowgrass);
		prints(buf);

	}
	else if (mode == 2)/*���u*/
	{
		move(4, 0);
		sprintf(buf
				, " [1;36m[�R��][37m%-5d[36m[���z][37m%-5d[36m[���][37m%-5d[36m[���N][37m%-5d[36m[�D�w][37m%-5d[36m[�i��][37m%-5d[36m[�a��][37m%-5d[0m"
				, d.love, d.wisdom, d.character, d.art, d.etchics, d.brave, d.homework);
		prints(buf);

	}
	else if (mode == 3)/*�צ�*/
	{
		move(4, 0);
		sprintf(buf
				, " [1;36m[���z][37m%-5d[36m[���][37m%-5d[36m[���N][37m%-5d[36m[�i��][37m%-5d[36m[����][37m%-5d[36m[���m][37m%-5d[36m[�t��][37m%-5d[0m"
				, d.wisdom, d.character, d.art, d.brave, d.attack, d.resist, d.speed);
		prints(buf);

	}
	move(5, 0);
	prints("[1;%dm�z�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�{[m", color);
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
	prints("[1;%dm�|�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�}[m", color);
	move(19, 0);
	prints(" [1;34m�w[37;44m  �� �A  [0;1;34m�w[0m");
	move(20, 0);
	prints(" ");
	if (d.shit == 0)
		prints("���b�p��  ");
	if (d.shit > 40 && d.shit < 60)
		prints("���I���  ");
	if (d.shit >= 60 && d.shit < 80)
		prints("[1;33m�ܯ�F��[m  ");
	if (d.shit >= 80 && d.shit < 100)
	{
		prints("[1;35m�֯䦺�F[m  ");
		d.sick += 4;
		d.character -= (rand() % 3 + 3);
	}
	if (d.shit >= 100)
	{
		d.death = 1;
		pipdie("[1;31m�z��䦺�F[m  ", 1);
		return -1;
	}

	if (d.hp <= 0)
		pc = 0;
	else
		pc = d.hp * 100 / d.maxhp;
	if (pc == 0)
	{
		d.death = 1;
		pipdie("[1;31m���j���F[m  ", 1);
		return -1;
	}
	if (pc < 20)
	{
		prints("[1;35m�־j���F[m  ");
		d.sick += 3;
		d.happy -= 5;
		d.satisfy -= 3;
	}
	if (pc < 40 && pc >= 20)
		prints("[1;33m�Q�Y�F��[m  ");
	if (pc <= 100 && pc >= 90)
		prints("�{�l����  ");
	if (pc < 110 && pc > 100)
		prints("[1;33m��������[m  ");

	pc = d.tired;
	if (pc < 20)
		prints("�믫�ܦn  ");
	if (pc < 80 && pc >= 60)
		prints("[1;33m���I�p��[m  ");
	if (pc < 100 && pc >= 80)
	{
		prints("[1;35m�u���ܲ�[m  ");
		d.sick += 5;
	}
	if (pc >= 100)
	{
		d.death = 1;
		pipdie("[1;31m����֦��F[m  ", 1);
		return -1;
	}

	pc = 60 + 10 * tm;
	if (d.weight < (pc + 30) && d.weight >= (pc + 10))
		prints("[1;33m���I�p�D[m  ");
	if (d.weight < (pc + 50) && d.weight >= (pc + 30))
	{
		prints("[1;35m�ӭD�F��[m  ");
		d.sick += 3;
		if (d.speed >= 2)
			d.speed -= 2;
		else
			d.speed = 0;

	}
	if (d.weight > (pc + 50))
	{
		d.death = 1;
		pipdie("[1;31m���Φ��F[m  ", 1);
		return -1;
	}

	if (d.weight < (pc - 50))
	{
		d.death = 1;
		pipdie("[1;31m:~~ �G���F[m  ", 1);
		return -1;
	}
	if (d.weight > (pc - 30) && d.weight <= (pc - 10))
		prints("[1;33m���I�p�G[m  ");
	if (d.weight > (pc - 50) && d.weight <= (pc - 30))
		prints("[1;35m�ӽG�F��[m ");

	if (d.sick < 75 && d.sick >= 50)
	{
		prints("[1;33m�ͯf�F��[m  ");
		count_tired(1, 8, "Y", 100, 1);
	}
	if (d.sick < 100 && d.sick >= 75)
	{
		prints("[1;35m���f����[m  ");
		d.sick += 5;
		count_tired(1, 15, "Y", 100, 1);
	}
	if (d.sick >= 100)
	{
		d.death = 1;
		pipdie("[1;31m�f���F�� :~~[m  ", 1);
		return -1;
	}

	pc = d.happy;
	if (pc < 20)
		prints("[1;35m�ܤ��ּ�[m  ");
	if (pc < 40 && pc >= 20)
		prints("[1;33m���ӧּ�[m  ");
	if (pc < 95 && pc >= 80)
		prints("�ְּ�..  ");
	if (pc <= 100 && pc >= 95)
		prints("�ܧּ�..  ");

	pc = d.satisfy;
	if (pc < 20) prints("[1;35m�ܤ�����..[m  ");
	if (pc < 40 && pc >= 20) prints("[1;33m���Ӻ���[m  ");
	if (pc < 95 && pc >= 80) prints("������..  ");
	if (pc <= 100 && pc >= 95) prints("�ܺ���..  ");

	prints("\n");

	pip_write_file();
	return 0;
}

/*�T�w�ɶ��@���� */
int
pip_time_change(cnow)
time_t cnow;
{
	int stime = 60;
	int stired = 2;
	while ((time(0) - lasttime) >= stime) /* �T�w�ɶ������� */
	{
		/*������  �٬O�|��ż��*/
		if ((time(0) - cnow) >= stime)
			d.shit += (rand() % 3 + 3);
		/*������  �h�ҷ�M��C��*/
		if (d.tired >= stired) d.tired -= stired; else d.tired = 0;
		/*������  �{�l�]�|�j�� */
		d.hp -= rand() % 2 + 2;
		if (d.mexp < 0)
			d.mexp = 0;
		if (d.hexp < 0)
			d.hexp = 0;
		/*��O�|�]�ͯf���C�@�I*/
		d.hp -= d.sick / 10;
		/*�f��|�H���v�W�[��ֳ֤\*/
		if (rand() % 3 > 0)
		{
			d.sick -= rand() % 2;
			if (d.sick < 0)
				d.sick = 0;
		}
		else
			d.sick += rand() % 2;
		/*�H����ּ֫�*/
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
	/*�ּ֫׺��N�׳̤j�ȳ]�w*/
	if (d.happy > 100)
		d.happy = 100;
	else if (d.happy < 0)
		d.happy = 0;
	if (d.satisfy > 100)
		d.satisfy = 100;
	else if (d.satisfy < 0)
		d.satisfy = 0;
	/*����*/
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
/* �򥻿��:���� �M�� �˿� ��                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

int pip_basic_takeshower() /*�~��*/
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
		vmsg("�ڬO���b���p��  cccc....");
	}
	else if (lucky == 1)
	{
		show_usual_pic(7);
		vmsg("���� ����");
	}
	else
	{
		show_usual_pic(2);
		vmsg("�ڷR�~�� lalala....");
	}
	return 0;
}

int pip_basic_takerest() /*��*/
{
	count_tired(5, 20, "Y", 100, 0);
	if (d.hp > d.maxhp)
		d.hp = d.maxhp;
	d.shit += 1;
	move(4, 0);
	show_usual_pic(5);
	vmsg("�A���@�U�ڴN�_���o....");
	show_usual_pic(6);
	vmsg("�޳޳�..�Ӱ_���o......");
	return 0;
}

int pip_basic_kiss()/*�˿�*/
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
		vmsg("�ӹ�! �q�@��.....");
	}
	else
	{
		vmsg("�ˤӦh�]�O�|ż������....");
	}
	return 0;
}

int pip_basic_feed()     /* ����*/
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
		sprintf(buf, "%s�Ӱ�����ƩO?", d.name);
		prints(buf);
		now = time(0);
		move(b_lines, 0);
		clrtoeol();
		move(b_lines, 0);
		prints("[1;44;37m  �������  [46m[1]�Y�� [2]�s�� [3]�ɤY [4]�F�� [5]�H�x [6]���� [Q]���X�G         [m");
		pip_time_change(now);
		pipkey = vkey();
		pip_time_change(now);

		switch (pipkey)
		{
		case '1':
			if (d.food <= 0)
			{
				move(b_lines, 0);
				vmsg("�S�������o..�֥h�R�a�I");
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
			vmsg("�C�Y�@�������|��_��O50��!");
			break;

		case '2':
			if (d.cookie <= 0)
			{
				move(b_lines, 0);
				vmsg("�s���Y���o..�֥h�R�a�I");
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
			vmsg("�Y�s���e���D��...");
			break;

		case '3':
			if (d.bighp <= 0)
			{
				move(b_lines, 0);
				vmsg("�S���j�ɤY�F�C! �ֶR�a..");
				break;
			}
			d.bighp--;
			d.hp += 600;
			d.tired -= 20;
			d.weight += rand() % 2;
			move(4, 0);
			show_feed_pic(4);
			d.nodone = 0;
			vmsg("�ɤY..�W���Ϊ���...");
			break;

		case '4':
			if (d.medicine <= 0)
			{
				move(b_lines, 0);
				vmsg("�S���F���o..�֥h�R�a�I");
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
			vmsg("�C�Y�@���F�۷|��_�k�O50��!");
			break;

		case '5':
			if (d.ginseng <= 0)
			{
				move(b_lines, 0);
				vmsg("�S���d�~�H�x�C! �ֶR�a..");
				break;
			}
			d.ginseng--;
			d.mp += 500;
			d.tired -= 20;
			move(4, 0);
			show_feed_pic(1);
			d.nodone = 0;
			vmsg("�d�~�H�x..�W���Ϊ���...");
			break;

		case '6':
			if (d.snowgrass <= 0)
			{
				move(b_lines, 0);
				vmsg("�S���Ѥs�����C! �ֶR�a..");
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
			vmsg("�Ѥs����..�W���Ϊ���...");
			break;

		}
	}
	while ((pipkey != 'Q') && (pipkey != 'q') && (pipkey != KEY_LEFT));

	return 0;
}

/*�C���g��ƤJ�ɮ�*/
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

/*�C��Ū��ƥX�ɮ�*/
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
		vmsg("�ڨS���i�p���� !");
		return;
	}

	return;
}

/*�O����pip.log��*/
int
pip_log_record(msg)
char *msg;
{
	FILE *fs;

	fs = fopen(FN_PIP_LOG, "a+");
	fprintf(fs, "%s", msg);
	fclose(fs);
}

/*�p���i���x�s*/
int
pip_write_backup()
{
	char *files[4] = {"�S��", "�i�פ@", "�i�פG", "�i�פT"};
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
		prints("�x�s [1]�i�פ@ [2]�i�פG [3]�i�פT [Q]��� [1/2/3/Q]�G");
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
		vmsg("����x�s�C���i��");
		return 0;
	}
	move(b_lines -2, 1);
	prints("�x�s�ɮ׷|�л\\���x�s�� [%s] ���p�����ɮ׳�I�ЦҼ{�M��...", files[num]);
	sprintf(buf1, "�T�w�n�x�s�� [%s] �ɮ׶ܡH [y/N]: ", files[num]);
	getdata(b_lines - 1, 1, buf1, ans, 2, DOECHO, 0);
	if (ans[0] != 'y' && ans[0] != 'Y')
	{
		vmsg("����x�s�ɮ�");
		return 0;
	}

	move(b_lines -1, 0);
	clrtobot();
	sprintf(buf1, "�x�s [%s] �ɮק����F", files[num]);
	vmsg(buf1);
	sprintf(buf, "/bin/cp %s %s.bak%d", get_path(cuser.userid, "chicken"), get_path(cuser.userid, "chicken"), num);
	system(buf);
	return 0;
}

int
pip_read_backup()
{
	char buf[200], buf1[200], buf2[200];
	char *files[4] = {"�S��", "�i�פ@", "�i�פG", "�i�פT"};
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
		prints("Ū�� [1]�i�פ@ [2]�i�פG [3]�i�פT [Q]��� [1/2/3/Q]�G");
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
				sprintf(buf, "�ɮ� [%s] ���s�b", files[num]);
				vmsg(buf);
				ok = 0;
			}
			else
			{

				move(b_lines - 2, 1);
				prints("Ū���X�ɮ׷|�л\\�{�b���b�����p�����ɮ׳�I�ЦҼ{�M��...");
				sprintf(buf, "�T�w�nŪ���X [%s] �ɮ׶ܡH [y/N]: ", files[num]);
				getdata(b_lines - 1, 1, buf, ans, 2, DOECHO, 0);
				if (ans[0] != 'y' && ans[0] != 'Y')
					vmsg("���ڦA�M�w�@�U...");
				else ok = 1;
				fclose(fs);
			}
		}
	}
	while (pipkey != 'Q' && pipkey != 'q' && ok != 1);
	if (pipkey == 'q' || pipkey == 'Q')
	{
		vmsg("�٬O���쥻���C��");
		return 0;
	}

	move(b_lines -1, 0);
	clrtobot();
	sprintf(buf, "Ū�� [%s] �ɮק����F", files[num]);
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
	vs_head("�p���_����N��", BoardName);

	now = time(0);
	sprintf(genbuf, "[1;33m%s %-11s���p�� [%s�G�N] �_���F�I[m\n", Cdate(&now), cuser.userid, d.name);
	pip_log_record(genbuf);

	/*����W���]�w*/
	d.death = 0;
	d.maxhp = d.maxhp * ALIVE + 1;
	d.hp = d.maxhp;
	d.tired = 20;
	d.shit = 20;
	d.sick = 20;
	d.wrist = d.wrist * ALIVE;
	d.weight = 45 + 10 * tm;

	/*����줭�����@*/
	d.money = d.money * ALIVE;

	/*�԰���O���@�b*/
	d.attack = d.attack * ALIVE;
	d.resist = d.resist * ALIVE;
	d.maxmp = d.maxmp * ALIVE;
	d.mp = d.maxmp;

	/*�ܪ����ּ�*/
	d.happy = 0;
	d.satisfy = 0;

	/*������b*/
	d.social = d.social * ALIVE;
	d.family = d.family * ALIVE;
	d.hexp = d.hexp * ALIVE;
	d.mexp = d.mexp * ALIVE;

	/*�Z��������*/
	d.weaponhead = 0;
	d.weaponrhand = 0;
	d.weaponlhand = 0;
	d.weaponbody = 0;
	d.weaponfoot = 0;

	/*�����Ѥ@�b*/
	d.food = d.food * ALIVE;
	d.medicine = d.medicine * ALIVE;
	d.bighp = d.bighp * ALIVE;
	d.cookie = d.cookie * ALIVE;

	d.liveagain += 1;

	vmsg("�p�����x���ؤ��I");
	vmsg("�p������_���I");
	vmsg("�p����O�վ㤤�I");
	vmsg("���H�z�A�A���p���S�_���o�I");
	pip_write_file();
	return 0;
}

/*---------------------------------------------------------------------------*/
/* �p���ϧΰ�                                                                */
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
show_feed_pic(int i)  /*�Y�F��*/
{
	char buf[256];
	clrchyiuan(6, 18);
	sprintf(buf, BBSHOME"/game/pipgame/feed/pic%d", i);
	show_file(buf, 6, 12, ONLY_COLOR);
}

void
show_buy_pic(int i)  /*�ʶR�F��*/
{
	char buf[256];
	clrchyiuan(6, 18);
	sprintf(buf, BBSHOME"/game/pipgame/buy/pic%d", i);
	show_file(buf, 6, 12, ONLY_COLOR);
}

void
show_usual_pic(int i)  /* ���`���A */
{
	char buf[256];
	clrchyiuan(6, 18);
	sprintf(buf, BBSHOME"/game/pipgame/usual/pic%d", i);
	show_file(buf, 6, 12, ONLY_COLOR);

}

void
show_special_pic(int i)  /* �S���� */
{
	char buf[256];
	clrchyiuan(6, 18);
	sprintf(buf, BBSHOME"/game/pipgame/special/pic%d", i);
	show_file(buf, 6, 12, ONLY_COLOR);

}

void
show_practice_pic(int i)  /*�צ�Ϊ��� */
{
	char buf[256];
	clrchyiuan(6, 18);
	sprintf(buf, BBSHOME"/game/pipgame/practice/pic%d", i);
	show_file(buf, 6, 12, ONLY_COLOR);
}

void
show_job_pic(int i)    /* ���u��show�� */
{
	char buf[256];
	clrchyiuan(6, 18);
	sprintf(buf, BBSHOME"/game/pipgame/job/pic%d", i);
	show_file(buf, 6, 12, ONLY_COLOR);

}


void
show_play_pic(int i)  /*�𶢪���*/
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
show_guess_pic(int i)  /* �q���� */
{
	char buf[256];
	clrchyiuan(6, 18);
	sprintf(buf, BBSHOME"/game/pipgame/guess/pic%d", i);
	show_file(buf, 6, 12, ONLY_COLOR);
}

void
show_weapon_pic(int i)  /* �Z���� */
{
	char buf[256];
	clrchyiuan(1, 10);
	sprintf(buf, BBSHOME"/game/pipgame/weapon/pic%d", i);
	show_file(buf, 1, 10, ONLY_COLOR);
}

void
show_palace_pic(int i)  /* �Ѩ����ڥ� */
{
	char buf[256];
	clrchyiuan(0, 13);
	sprintf(buf, BBSHOME"/game/pipgame/palace/pic%d", i);
	show_file(buf, 0, 11, ONLY_COLOR);

}

void
show_badman_pic(int i)  /* �a�H */
{
	char buf[256];
	clrchyiuan(6, 18);
	sprintf(buf, BBSHOME"/game/pipgame/badman/pic%d", i);
	show_file(buf, 6, 14, ONLY_COLOR);
}

void
show_fight_pic(int i)  /* ���[ */
{
	char buf[256];
	clrchyiuan(6, 18);
	sprintf(buf, BBSHOME"/game/pipgame/fight/pic%d", i);
	show_file(buf, 6, 14, ONLY_COLOR);
}

void
show_die_pic(int i)  /*���`*/
{
	char buf[256];
	clrchyiuan(0, 23);
	sprintf(buf, BBSHOME"/game/pipgame/die/pic%d", i);
	show_file(buf, 0, 23, ONLY_COLOR);
}

void
show_system_pic(int i)  /*�t��*/
{
	char buf[256];
	clrchyiuan(1, 23);
	sprintf(buf, BBSHOME"/game/pipgame/system/pic%d", i);
	show_file(buf, 4, 16, ONLY_COLOR);
}

void
show_ending_pic(int i)  /*����*/
{
	char buf[256];
	clrchyiuan(1, 23);
	sprintf(buf, BBSHOME"/game/pipgame/ending/pic%d", i);
	show_file(buf, 4, 16, ONLY_COLOR);
}

void
show_resultshow_pic(int i)	/*��ì�u*/
{
	char buf[256];
	clrchyiuan(0, 24);
	sprintf(buf, BBSHOME"/game/pipgame/resultshow/pic%d", i);
	show_file(buf, 0, 24, ONLY_COLOR);
}

/*---------------------------------------------------------------------------*/
/* �ө����:���� �s�� �j�ɤY ���� �ѥ�                                       */
/*                                                                           */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* �ө����:���� �s�� �j�ɤY ���� �ѥ�                                       */
/* �禡�w                                                                    */
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

int pip_store_weapon_head()	/*�Y���Z��*/
{
	d.weaponhead = pip_weapon_doing_menu(d.weaponhead, 0, headlist);
	return 0;
}
int pip_store_weapon_rhand()	/*�k��Z��*/
{
	d.weaponrhand = pip_weapon_doing_menu(d.weaponrhand, 1, rhandlist);
	return 0;
}
int pip_store_weapon_lhand()    /*����Z��*/
{
	d.weaponlhand = pip_weapon_doing_menu(d.weaponlhand, 2, lhandlist);
	return 0;
}
int pip_store_weapon_body()	/*����Z��*/
{
	d.weaponbody = pip_weapon_doing_menu(d.weaponbody, 3, bodylist);
	return 0;
}
int pip_store_weapon_foot()     /*�����Z��*/
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
	char *shopname[4] = {"���W", "�K�Q�ө�", NICKNAME "�ľQ", "�]�̮ѧ�"};
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
		sprintf(inbuf, "[1;31m  �w[41;37m �s�� [0;1;31m�w[41;37m ��      �~ [0;1;31m�w�w[41;37m ��            �� [0;1;31m�w�w[41;37m ��     �� [0;1;31m�w[37;41m �֦��ƶq [0;1;31m�w[0m  ");
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
		sprintf(inbuf, "[1;44;37m  %8s���  [46m  [B]�R�J���~  [S]��X���~  [Q]���X�G                         [m", shopname[mode]);
		prints(inbuf);
		pipkey = vkey();
		switch (pipkey)
		{
		case 'B':
		case 'b':
			move(b_lines - 1, 1);
			sprintf(inbuf, "�Q�n�R�Jԣ�O? [0]���R�J [1��%d]���~�Ӹ�: ", oldnum[0]);
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
					sprintf(inbuf, "�A�n�R�J���~ [%s] �h�֭өO?(�W�� %d): ", p[choice].name, d.money / p[choice].money);
					getdata(b_lines - 1, 1, inbuf, genbuf, 6, DOECHO, 0);
					smoney = atoi(genbuf);
				}
				if (smoney < 0)
				{
					vmsg("���R�J...");
				}
				else if (d.money < smoney*p[choice].money)
				{
					vmsg("�A�����S������h��..");
				}
				else
				{
					sprintf(inbuf, "�T�w�R�J���~ [%s] �ƶq %d �Ӷ�?(���a��� %d) [y/N]: ", p[choice].name, smoney, smoney*p[choice].money);
					getdata(b_lines - 1, 1, inbuf, genbuf, 2, DOECHO, 0);
					if (genbuf[0] == 'y' || genbuf[0] == 'Y')
					{
						oldnum[choice] += smoney;
						d.money -= smoney * p[choice].money;
						sprintf(inbuf, "���󵹤F�A%d��%s", smoney, p[choice].name);
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
						vmsg("���R�J...");
					}
				}
			}
			else
			{
				sprintf(inbuf, "���R�J.....");
				vmsg(inbuf);
			}
			break;

		case 'S':
		case 's':
			if (mode == 3)
			{
				vmsg("�o�ǪF�褣����....");
				break;
			}
			move(b_lines - 1, 1);
			sprintf(inbuf, "�Q�n��Xԣ�O? [0]����X [1��%d]���~�Ӹ�: ", oldnum[0]);
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
				sprintf(inbuf, "�A�n��X���~ [%s] �h�֭өO?(�W�� %d): ", p[choice].name, oldnum[choice]);
				getdata(b_lines - 1, 1, inbuf, genbuf, 6, , 0);
				smoney = atoi(genbuf);
				if (smoney < 0)
				{
					vmsg("����X...");
				}
				else if (smoney > oldnum[choice])
				{
					sprintf(inbuf, "�A�� [%s] �S������h�ӳ�", p[choice].name);
					vmsg(inbuf);
				}
				else
				{
					sprintf(inbuf, "�T�w��X���~ [%s] �ƶq %d �Ӷ�?(���a�R�� %d) [y/N]: ", p[choice].name, smoney, smoney*p[choice].money*8 / 10);
					getdata(b_lines - 1, 1, inbuf, genbuf, 2, DOECHO, 0);
					if (genbuf[0] == 'y' || genbuf[0] == 'Y')
					{
						oldnum[choice] -= smoney;
						d.money += smoney * p[choice].money * 8 / 10;
						sprintf(inbuf, "���󮳨��F�A��%d��%s", smoney, p[choice].name);
						vmsg(inbuf);
					}
					else
					{
						vmsg("����X...");
					}
				}
			}
			else
			{
				sprintf(inbuf, "����X.....");
				vmsg(inbuf);
			}
			break;
		case 'Q':
		case 'q':
			sprintf(inbuf, "��������@ %d ��,���} %s ", oldmoney - d.money, shopname[mode]);
			vmsg(inbuf);
			break;
		}
	}
	while ((pipkey != 'Q') && (pipkey != 'q') && (pipkey != KEY_LEFT));
	return 0;
}

int
pip_weapon_doing_menu(variance, type, p)             /* �Z���ʶR�e�� */
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
	char menutitle[5][11] = {"�Y���˳ư�", "�k��˳ư�", "����˳ư�", "����˳ư�", "�����˳ư�"};
	int pipkey;
	char choicekey[5];
	int choice;

	do
	{
		clear();
		vs_head(menutitle[type], BoardName);
		show_weapon_pic(0);
		/*   move(10,2);
		   sprintf(buf,"[1;37m�{����O:��OMax:[36m%-5d[37m  �k�OMax:[36m%-5d[37m  ����:[36m%-5d[37m  ���m:[36m%-5d[37m  �t��:[36m%-5d [m",
		           d.maxhp,d.maxmp,d.attack,d.resist,d.speed);
		   prints(buf);*/
		move(11, 2);
		sprintf(buf, "[1;37;41m [NO]  [����W]  [��O]  [�k�O]  [�t��]  [����]  [���m]  [�t��]  [��  ��] [m");
		prints(buf);
		move(12, 2);
		sprintf(buf, " [1;31m�w�w[37m�զ� �i�H�ʶR[31m�w�w[32m��� �֦��˳�[31m�w�w[33m���� ��������[31m�w�w[35m���� ��O����[31m�w�w[m");
		prints(buf);

		n = 0;
		while ((s = p[n].name))
		{
			move(13 + n, 2);
			if (variance != 0 && variance == (n))/*��������*/
			{
				sprintf(buf,
						"[1;32m (%2d)  %-10s %4d    %4d    %4d    %4d    %4d    %4d    %6d[m",
						n, p[n].name, p[n].needmaxhp, p[n].needmaxmp, p[n].needspeed,
						p[n].attack, p[n].resist, p[n].speed, p[n].cost);
			}
			else if (d.maxhp < p[n].needmaxhp || d.maxmp < p[n].needmaxmp || d.speed < p[n].needspeed)/*��O����*/
			{
				sprintf(buf,
						"[1;35m (%2d)  %-10s %4d    %4d    %4d    %4d    %4d    %4d    %6d[m",
						n, p[n].name, p[n].needmaxhp, p[n].needmaxmp, p[n].needspeed,
						p[n].attack, p[n].resist, p[n].speed, p[n].cost);
			}

			else if (d.money < p[n].cost) /*��������*/
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
		sprintf(buf, "[1;44;37m  �Z���ʶR���  [46m  [B]�ʶR�Z��  [S]�汼�˳�  [W]�ӤH���  [Q]���X�G            [m");
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
			sprintf(shortbuf, "�Q�n�ʶRԣ�O? �A������[%d]��:[�Ʀr]: ", d.money);
			prints(shortbuf);
			getdata(b_lines - 1, 1, shortbuf, choicekey, 4, LCECHO, "0");
			choice = atoi(choicekey);
			if (choice >= 0 && choice <= n)
			{
				move(b_lines - 1, 0);
				clrtoeol();
				move(b_lines - 1, 1);
				if (choice == 0)  /*�Ѱ�*/
				{
					sprintf(shortbuf, "����ʶR...");
					vmsg(shortbuf);
				}

				else if (variance == choice)  /*���w�g����*/
				{
					sprintf(shortbuf, "�A���w�g�� %s �o", p[variance].name);
					vmsg(shortbuf);
				}

				else if (p[choice].cost >= (d.money + p[variance].sell))  /*������*/
				{
					sprintf(shortbuf, "�o�ӭn %d ���A�A����������!", p[choice].cost);
					vmsg(shortbuf);
				}

				else if (d.maxhp < p[choice].needmaxhp || d.maxmp < p[choice].needmaxmp
						 || d.speed < p[choice].needspeed)  /*��O����*/
				{
					sprintf(shortbuf, "�ݭnHP %d MP %d SPEED %d ��",
							p[choice].needmaxhp, p[choice].needmaxmp, p[choice].needspeed);
					vmsg(shortbuf);
				}
				else  /*���Q�ʶR*/
				{
					sprintf(shortbuf, "�A�T�w�n�ʶR %s ��?($%d) [y/N]: ", p[choice].name, p[choice].cost);
					getdata(b_lines - 1, 1, shortbuf, ans, 2, DOECHO, 0);
					if (ans[0] == 'y' || ans[0] == 'Y')
					{
						sprintf(shortbuf, "�p���w�g�˰t�W %s �F", p[choice].name);
						vmsg(shortbuf);
						d.attack += (p[choice].attack - p[variance].attack);
						d.resist += (p[choice].resist - p[variance].resist);
						d.speed += (p[choice].speed - p[variance].speed);
						d.money -= (p[choice].cost - p[variance].sell);
						variance = choice;
					}
					else
					{
						sprintf(shortbuf, "����ʶR.....");
						vmsg(shortbuf);
					}
				}
			}
			break;

		case 'S':
		case 's':
			if (variance != 0)
			{
				sprintf(shortbuf, "�A�T�w�n�汼%s��? ���:%d [y/N]: ", p[variance].name, p[variance].sell);
				getdata(b_lines - 1, 1, shortbuf, ans, 2, DOECHO, 0);
				if (ans[0] == 'y' || ans[0] == 'Y')
				{
					sprintf(shortbuf, "�˳� %s ��F %d", p[variance].name, p[variance].sell);
					d.attack -= p[variance].attack;
					d.resist -= p[variance].resist;
					d.speed -= p[variance].speed;
					d.money += p[variance].sell;
					vmsg(shortbuf);
					variance = 0;
				}
				else
				{
					sprintf(shortbuf, "ccc..�ڦ^����N�F...");
					vmsg(shortbuf);
				}
			}
			else if (variance == 0)
			{
				sprintf(shortbuf, "�A���ӴN�S���˳ƤF...");
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
/* ���u���:�a�� �W�u �a�� �a�u                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

int pip_job_workA()
{
	/*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	/*  �x�a�x�޲z�x�ݤH���� + N , ���a�~�� + N , �i���ޥ� + N  �x*/
	/*  �x        �x�M���˪����Y + N , �h�� + 1 , �P�� - 2      �x*/
	/*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	/*  �x�a�x�޲z�x�Y ��    �O - RND (�h��) >=   5 �h�u�@���\  �x*/
	/*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
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
		vmsg("�a�ƫܦ��\\��..�h�@�I�����A..");
	}
	else if (class < 75 && class >= 50)
	{
		d.cookskill += rand() % 2 + 5;
		d.homework += rand() % 2 + 5;
		d.family += rand() % 3 + 3;
		d.relation += rand() % 3 + 3;
		workmoney = 60 + (d.cookskill * 2 + d.homework + d.family) / 45;
		vmsg("�a�����Z���Q����..���..");
	}
	else if (class < 50 && class >= 25)
	{
		d.cookskill += rand() % 3 + 3;
		d.homework += rand() % 3 + 3;
		d.family += rand() % 3 + 2;
		d.relation += rand() % 3 + 2;
		workmoney = 40 + (d.cookskill * 2 + d.homework + d.family) / 50;
		vmsg("�a�ƴ����q�q��..�i�H��n��..�[�o..");
	}
	else if (class < 25)
	{
		d.cookskill += rand() % 3 + 1;
		d.homework += rand() % 3 + 1;
		d.family += rand() % 3 + 1;
		d.relation += rand() % 3 + 1;
		workmoney = 20 + (d.cookskill * 2 + d.homework + d.family) / 60;
		vmsg("�a�ƫ��V�|��..�o�ˤ����..");
	}
	d.money += workmoney * LEARN_ELVEL;
	d.workA += 1;
	return 0;
}

int pip_job_workB()
{
	/*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	/*  �x�|���|  �x���� + N , �P�� + 1 , �y�O - 1 , �h�� + 3   �x*/
	/*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	/*  �x�|���|  �x�Y ��    �O - RND (�h��) >=  20 �h�u�@���\  �x*/
	/*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
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
		vmsg("��O�i�ܦ��\\��..�U���A�ӳ�..");
	}
	else if (class < 90 && class >= 75)
	{
		d.love += rand() % 2 + 5;
		d.toman += rand() % 2 + 2;
		workmoney = 120 + (d.love + d.toman) / 50;
		vmsg("�O�i�ٷ�������..���..");
	}
	else if (class < 75 && class >= 50)
	{
		d.love += rand() % 2 + 3;
		d.toman += rand() % 2 + 1;
		workmoney = 100 + (d.love + d.toman) / 50;
		vmsg("�p�B�ͫܥֳ�..�[�o..");
	}
	else if (class < 50)
	{
		d.love += rand() % 2 + 1;
		d.toman += rand() % 2 + 1;
		workmoney = 80 + (d.love + d.toman) / 50;
		vmsg("���V�|��..�A�n����p�B�ͭC...");
	}
	d.money += workmoney * LEARN_ELVEL;
	d.workB += 1;
	return 0;
}

int pip_job_workC()
{
	/*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	/*  �x���]    �x���a�~�� + N , �԰��޳N - N , �h�� + 2      �x*/
	/*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	/*  �x���]    �x�Y ��    �O - RND (�h��) >=  30 �h�u�@���\  �x*/
	/*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
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
		vmsg("���]�Ʒ~�]�]��W..�Ʊ�A�A�L��...");
	}
	else if (class < 95 && class >= 80)
	{
		d.homework += rand() % 2 + 5;
		d.family += rand() % 2 + 3;
		d.hskill -= rand() % 2 + 5;
		if (d.hskill < 0)
			d.hskill = 0;
		workmoney = 200 + (d.cookskill * 2 + d.homework * 2) / 50;
		vmsg("���]���Z���Q����..���..");
	}
	else if (class < 80 && class >= 60)
	{
		d.homework += rand() % 2 + 3;
		d.family += rand() % 2 + 3;
		d.hskill -= rand() % 2 + 5;
		if (d.hskill < 0)
			d.hskill = 0;
		workmoney = 150 + (d.cookskill * 2 + d.homework * 2) / 50;
		vmsg("�����q�q��..�i�H��n��..�[�o..");
	}
	else if (class < 60)
	{
		d.homework += rand() % 2 + 1;
		d.family += rand() % 2 + 1;
		d.hskill -= rand() % 2 + 1;
		if (d.hskill < 0)
			d.hskill = 0;
		workmoney = 100 + (d.cookskill * 2 + d.homework * 2) / 50;
		vmsg("�o�ӫ��V�|��..�A�o�ˤ����..");
	}
	d.money += workmoney * LEARN_ELVEL;
	d.workC += 1;
	return 0;
}

int pip_job_workD()
{
	/*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	/*  �x�A��    �x��O + 1 , �äO + 1 , ��� - 1 , �h�� + 3   �x*/
	/*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	/*  �x�A��    �x�Y ��    �O - RND (�h��) >=  30 �h�u�@���\  �x*/
	/*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
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
		vmsg("���Ϫ����n�n��..�Ʊ�A�A������...");
	}
	else if (class < 95 && class >= 80)
	{
		workmoney = 210 + (d.wrist * 2 + d.hp * 2) / 80;
		vmsg("����..�٤�����..:)");
	}
	else if (class < 80 && class >= 60)
	{
		workmoney = 160 + (d.wrist * 2 + d.hp * 2) / 80;
		vmsg("�����q�q��..�i�H��n��..");
	}
	else if (class < 60)
	{
		workmoney = 120 + (d.wrist * 2 + d.hp * 2) / 80;
		vmsg("�A���ӾA�X�A�����u�@  -_-...");
	}
	d.money += workmoney * LEARN_ELVEL;
	d.workD += 1;
	return 0;
}

int pip_job_workE()
{
	/*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	/*  �x�\�U    �x�Ʋz + N , �԰��޳N - N , �h�� + 2          �x*/
	/*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	/*  �x�\�U    �x�Y �i���޳N - RND (�h��) >=  50 �h�u�@���\  �x*/
	/*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
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
		vmsg("�ȤH�����Ӧn�Y�F..�A�Ӥ@�L�a...");
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
		vmsg("�N���٤����Y��..:)");
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
		vmsg("�����q�q��..�i�H��n��..");
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
		vmsg("�A���p���ݥ[�j��...");
	}
	d.money += workmoney * LEARN_ELVEL;
	d.workE += 1;
	return 0;
}

int pip_job_workF()
{
	/*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	/*  �x�а�    �x�H�� + 2 , �D�w + 1 , �o�^ - 2 , �h�� + 1   �x*/
	/*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
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
		vmsg("���ܤ� ���ݧA�o��{�u ���A�h�@�I...");
	}
	else if (class < 75 && class >= 50)
	{
		workmoney = 75 + (d.belief + d.etchics - d.offense) / 20;
		vmsg("���§A����������..:)");
	}
	else if (class < 50 && class >= 25)
	{
		workmoney = 50 + (d.belief + d.etchics - d.offense) / 20;
		vmsg("�A�u���ܦ��R�߰�..���L���I�p�֪��ˤl...");
	}
	else if (class < 25)
	{
		workmoney = 25 + (d.belief + d.etchics - d.offense) / 20;
		vmsg("�ө^�m����..���]���ॴ�V��....:(");
	}
	d.money += workmoney * LEARN_ELVEL;
	d.workF += 1;
	return 0;
}

int pip_job_workG()
{
	/* �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	/* �x�a�u    �x��O + 2 , �y�O + 1 , �h�� + 3 ,�ͦR +1     �x*/
	/* �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
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
	vmsg("�\\�a�u�n��ĵ���..:p");
	d.money += workmoney * LEARN_ELVEL;
	d.workG += 1;
	return 0;
}

int pip_job_workH()
{
	/*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	/*  �x����  �x�äO + 2 , ��� - 2 , �h�� + 4              �x*/
	/*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	/*  �x����  �x�Y ��    �O - RND (�h��) >=  80 �h�u�@���\  �x*/
	/*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	float class;
	long workmoney;

	if ((d.bbtime / 60 / 30) < 1) /*�@���~��*/
	{
		vmsg("�p���Ӥp�F,�@���H��A�ӧa...");
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
		vmsg("�A�äO�ܦn��..:)");
	}
	else if (class < 70 && class >= 50)
	{
		workmoney = 300 + d.wrist / 20 + d.maxhp / 80;
		vmsg("��F���־��.....:)");
	}
	else if (class < 50 && class >= 20)
	{
		workmoney = 250 + d.wrist / 20 + d.maxhp / 80;
		vmsg("�����q�q��..�i�H��n��..");
	}
	else if (class < 20)
	{
		workmoney = 200 + d.wrist / 20 + d.maxhp / 80;
		vmsg("�ݥ[�j��..����A�ӧa....");
	}
	d.money += workmoney * LEARN_ELVEL;
	d.workH += 1;
	return 0;
}

int pip_job_workI()
{
	/*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	/*  �x���e�|  �x�P�� + 1 , �äO - 1 , �h�� + 3              �x*/
	/*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	/*  �x���e�|  �x�Y ���N�׾i - RND (�h��) >=  40 �h�u�@���\  �x*/
	/*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	float class;
	long workmoney;

	if ((d.bbtime / 60 / 30) < 1) /*�@���~��*/
	{
		vmsg("�p���Ӥp�F,�@���H��A�ӧa...");
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
		vmsg("�ȤH���ܳ��w���A���y����..:)");
	}
	else if (class < 80 && class >= 60)
	{
		workmoney = 360 + d.art / 10 + d.affect / 20;
		vmsg("����������..�ᦳ�ѥ�...:)");
	}
	else if (class < 60 && class >= 40)
	{
		workmoney = 320 + d.art / 10 + d.affect / 20;
		vmsg("��������..�A�[�o�@�I..");
	}
	else if (class < 40)
	{
		workmoney = 250 + d.art / 10 + d.affect / 20;
		vmsg("�ݥ[�j��..�H��A�ӧa....");
	}
	d.money += workmoney * LEARN_ELVEL;
	d.workI += 1;
	return 0;
}

int pip_job_workJ()
{
	/*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	/*  �x���y��  �x��O + 1 , ��� - 1 , ���� - 1 , �h�� + 3   �x*/
	/*  �x        �x�԰��޳N + N                                �x*/
	/*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	/*  �x���y��  �x�Y ��    �O - RND (�h��) >=  80 ��          �x*/
	/*  �x        �x�Y ��    �O - RND (�h��) >=  40 �h�u�@���\  �x*/
	/*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	float class;
	float class1;
	long workmoney;

	/*�ⷳ�H�W�~��*/
	if ((d.bbtime / 60 / 30) < 2)
	{
		vmsg("�p���Ӥp�F,�ⷳ�H��A�ӧa...");
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
		vmsg("�A�O�������y�H..");
	}
	else if ((class < 75 && class >= 50) && class1 >= 60)
	{
		d.hskill += rand() % 2 + 5;
		workmoney = 270 + d.maxhp / 45 + d.hskill / 20;
		vmsg("�����٤�����..�i�H���\\�@�y�F..:)");
	}
	else if ((class < 50 && class >= 25) && class1 >= 40)
	{
		d.hskill += rand() % 2 + 3;
		workmoney = 240 + d.maxhp / 40 + d.hskill / 20;
		vmsg("�޳N�t�j�H�N  �A�[�o��..");
	}
	else if ((class < 25 && class >= 0) && class1 >= 20)
	{
		d.hskill += rand() % 2 + 1;
		workmoney = 210 + d.maxhp / 30 + d.hskill / 20;
		vmsg("���y�O��O�P���O�����X....");
	}
	else if (class < 0)
	{
		d.hskill += rand() % 2;
		workmoney = 190 + d.hskill / 20;
		vmsg("�n�h�h����M�W�i���z��....");
	}
	d.money += workmoney * LEARN_ELVEL;
	d.workJ += 1;
	return 0;
}

int pip_job_workK()
{
	/* �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	/* �x�u�a    �x��O + 2 , �y�O - 1 , �h�� + 3              �x*/
	/* �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	float class;
	long workmoney;

	/*�ⷳ�H�W�~��*/
	if ((d.bbtime / 60 / 30) < 2)
	{
		vmsg("�p���Ӥp�F,�ⷳ�H��A�ӧa...");
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
		vmsg("�u�{�ܧ���  ���§A�F..");
	}
	else if (class < 75 && class >= 50)
	{
		workmoney = 220 + d.maxhp / 45;
		vmsg("�u�{�|�ٶ��Q  ���W�A�F..");
	}
	else if (class < 50 && class >= 25)
	{
		workmoney = 200 + d.maxhp / 40;
		vmsg("�u�{�t�j�H�N  �A�[�o��..");
	}
	else if (class < 25 && class >= 0)
	{
		workmoney = 180 + d.maxhp / 30;
		vmsg("��  �ݥ[�j�ݥ[�j....");
	}
	else
	{
		workmoney = 160;
		vmsg("�U����O�n�@�I..�h�ҫקC�@�I�A��....");
	}

	d.money += workmoney * LEARN_ELVEL;
	d.workK += 1;
	return 0;
}

int pip_job_workL()
{
	/*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	/*  �x�Ӷ�    �x���]��O + N , �P�� + 1 , �y�O - 1          �x*/
	/*  �x        �x�h�� + 2                                    �x*/
	/*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	float class;
	float class1;
	long workmoney;

	/*�T���~��*/
	if ((d.bbtime / 60 / 30) < 3)
	{
		vmsg("�p���{�b�٤Ӥp�F,�T���H��A�ӧa...");
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
		vmsg("�u�Ӧ��\\��  ���A�h�I��");
	}
	else if ((class < 75 && class >= 50) && class1 >= 50)
	{
		d.mresist += rand() % 2 + 5;
		workmoney = 150 + (d.affect + d.brave) / 50;
		vmsg("�u���ٺ⦨�\\��..�°�..");
	}
	else if ((class < 50 && class >= 25) && class1 >= 25)
	{
		d.mresist += rand() % 2 + 3;
		workmoney = 120 + (d.affect + d.brave) / 60;
		vmsg("�u���ٺ�t�j�H�N��..�[�o..");
	}
	else
	{
		d.mresist += rand() % 2 + 1;
		workmoney = 80 + (d.affect + d.brave) / 70;
		vmsg("�ڤ]����K��ԣ�F..�ЦA�[�o..");
	}
#if 0
	if (rand() % 10 == 5)
	{
		vmsg("�u�O�˷�  ���J�즺���]..");
		pip_fight_bad(12);
	}
#endif
	d.money += workmoney * LEARN_ELVEL;
	d.workL += 1;
	return 0;
}

int pip_job_workM()
{
	/*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	/*  �x�a�x�Юv�x�D�w + 1 , ���� + N , �y�O - 1 , �h�� + 7   �x*/
	/*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	float class;
	long workmoney;

	if ((d.bbtime / 60 / 30) < 4)
	{
		vmsg("�p���Ӥp�F,�|���H��A�ӧa...");
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
	vmsg("�a�л��P ��M���N�֤@�I�o");
	d.workM += 1;
	return 0;
}

int pip_job_workN()
{
	/*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	/*  �x�s��    �x�i���ޥ� + N , �͸ܧޥ� + N , ���O - 2      �x*/
	/*  �x        �x�h�� + 5                                    �x*/
	/*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	/*  �x�s��    �x�Y ��    �O - RND (�h��) >=  60 ��          �x*/
	/*  �x        �x�Y �y    �O - RND (�h��) >=  50 �h�u�@���\  �x*/
	/*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	float class;
	float class1;
	long workmoney;

	if ((d.bbtime / 60 / 30) < 5)
	{
		vmsg("�p���Ӥp�F,�����H��A�ӧa...");
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
		vmsg("�A�ܬ���  :)");
	}
	else if ((class < 75 && class >= 50) && class1 >= 50)
	{
		d.cookskill += rand() % 2 + 5;
		d.speech += rand() % 2 + 5;
		workmoney = 400 + (d.charm) / 5;
		vmsg("�Z���w�諸�C....");
	}
	else if ((class < 50 && class >= 25) && class1 >= 25)
	{
		d.cookskill += rand() % 2 + 4;
		d.speech += rand() % 2 + 3;
		workmoney = 300 + (d.charm) / 5;
		vmsg("�ܥ��Z��..���������...");
	}
	else
	{
		d.cookskill += rand() % 2 + 2;
		d.speech += rand() % 2 + 2;
		workmoney = 200 + (d.charm) / 5;
		vmsg("�A���A�O������..�Х[�o....");
	}
	d.money += workmoney * LEARN_ELVEL;
	d.workN += 1;
	return 0;
}

int pip_job_workO()
{
	/*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	/*  �x�s�a    �x�y�O + 2 , �o�^ + 2 , �D�w - 3 , �H�� - 3   �x*/
	/*  �x        �x�ݤH���� - N , �M���˪����Y - N , �h�� + 12 �x*/
	/*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	/*  �x�s�a    �x�Y �y    �O - RND (�h��) >=  70 �h�u�@���\  �x*/
	/*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	float class;
	long workmoney;

	if ((d.bbtime / 60 / 30) < 4)
	{
		vmsg("�p���Ӥp�F,�|���H��A�ӧa...");
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
		vmsg("�A�O���������P��  :)");
	}
	else if (class < 75 && class >= 50)
	{
		d.relation -= rand() % 5 + 8;
		d.toman -= rand() % 5 + 8;
		workmoney = 500 + (d.charm) / 5;
		vmsg("�A�Z���w�諸�C..:)");
	}
	else if (class < 50 && class >= 25)
	{
		d.relation -= rand() % 5 + 5;
		d.toman -= rand() % 5 + 5;
		workmoney = 400 + (d.charm) / 5;
		vmsg("�A�ܥ��Z..����������...");
	}
	else
	{
		d.relation -= rand() % 5 + 1;
		d.toman -= rand() % 5 + 1;
		workmoney = 300 + (d.charm) / 5;
		vmsg("��..�A���A�O������....");
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
	/*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	/*  �x�j�]�`�|�x�y�O + 3 , �o�^ + 1 , ��� - 2 , ���O - 1   �x*/
	/*  �x        �x�ݤH���� - N , �h�� + 8                     �x*/
	/*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	/*  �x�j�]�`�|�x�Y �y    �O - RND (�h��) >=  70 ��          �x*/
	/*  �x        �x�Y ���N�׾i - RND (�h��) >=  30 �h�u�@���\  �x*/
	/*  �|�w�w�w�w�r�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�}*/
	float class;
	float class1;
	long workmoney;

	if ((d.bbtime / 60 / 30) < 6)
	{
		vmsg("�p���Ӥp�F,�����H��A�ӧa...");
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
		vmsg("�A�O�]�`�|�̰{�G���P�P��  :)");
	}
	else if ((class < 75 && class >= 50) && class1 > 20)
	{
		d.speech += rand() % 5 + 8;
		d.toman -= rand() % 5 + 8;
		workmoney = 800 + (d.charm) / 5;
		vmsg("���..�A�Z���w�諸�C..:)");
	}
	else if ((class < 50 && class >= 25) && class1 > 10)
	{
		d.speech += rand() % 5 + 5;
		d.toman -= rand() % 5 + 5;
		workmoney = 600 + (d.charm) / 5;
		vmsg("�A�n�[�o�F��..��������...");
	}
	else
	{
		d.speech += rand() % 5 + 1;
		d.toman -= rand() % 5 + 1;
		workmoney = 400 + (d.charm) / 5;
		vmsg("��..�A�����....");
	}
	d.money += workmoney * LEARN_ELVEL;
	if (d.toman < 0)
		d.toman = 0;
	d.workP += 1;
	return 1;
}

/*---------------------------------------------------------------------------*/
/* ���ֿ��:���B �ȹC �B�� ���| �q��                                         */
/*                                                                           */
/*---------------------------------------------------------------------------*/

int pip_play_stroll()	/*���B*/
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
		vmsg("�J��B���o  �u�n.... ^_^");
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
		vmsg("�ߨ�F100���F..�C�C�C....");
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
				vmsg("���F50���F..����....");
			}
			else
			{
				d.money = 0;
				d.hp -= (rand() % 3 + 3);
				vmsg("���������F..����....");
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
				vmsg("�ΤF50���F..���i�H�|�ڳ�....");
			}
			else
			{
				d.money = 0;
				d.hp -= (rand() % 3 + 3);
				vmsg("���Q�ڰ��Υ����F..:p");
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
		vmsg("�n�γ�ߨ쪱��F��.....");
	}
	else if (lucky == 6)
	{
		d.happy -= (rand() % 3 + 10);
		d.shit += (rand() % 3 + 20);
		move(4, 0);
		show_play_pic(9);
		vmsg("�u�O�˷�  �i�H�h�R�R�����");
	}
	else
	{
		d.happy += rand() % 3 + 3;
		d.satisfy += rand() % 2 + 1;
		d.shit += rand() % 3 + 2;
		d.hp -= (rand() % 3 + 2);
		move(4, 0);
		show_play_pic(8);
		vmsg("�S���S�O���Ƶo�Ͱ�.....");
	}
	return 0;
}

int pip_play_sport()	/*�B��*/
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
	vmsg("�B�ʦn�B�h�h��...");
	return 0;
}

int pip_play_date()	/*���|*/
{
	if (d.money < 150)
	{
		vmsg("�A�������h��! ���|�`�o���I����");
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
		vmsg("���|�h  �I�I");
	}
	return 0;
}
int pip_play_outing()	/*���C*/
{
	int lucky;
	char buf[256];

	if (d.money < 250)
	{
		vmsg("�A�������h��! �ȹC�`�o���I����");
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
				vmsg("�ߤ����@�ѲH�H���Pı  �n�ΪA��....");
			else
				vmsg("���� �~�� �߱��n�h�F.....");
		}
		else if (lucky == 1)
		{
			d.art += rand() % 3;
			d.maxmp += rand() % 2;
			show_play_pic(13);
			if (rand() % 2 > 0)
				vmsg("���s����������  �Φ��@�T���R���e..");
			else
				vmsg("�ݵ۬ݵ�  �����h�γ������o..");
		}
		else if (lucky == 2)
		{
			d.love += rand() % 3;
			show_play_pic(14);
			if (rand() % 2 > 0)
				vmsg("��  �Ӷ��֨S�J�����o...");
			else
				vmsg("ť���o�O�����  �A���O?");
		}
		else if (lucky == 3)
		{
			d.maxhp += rand() % 3;
			show_play_pic(15);
			if (rand() % 2 > 0)
				vmsg("���ڭ̺ƨg�b�]�̪����y�a....�I�I..");
			else
				vmsg("�D�n�������ﭱŧ��  �̳��w�o�طPı�F....");
		}
		if ((rand() % 301 + rand() % 200) % 100 == 12)
		{
			lucky = 0;
			clear();
			sprintf(buf, "[1;41m  " NICKNAME PIPNAME " �� %-10s                                                  [0m", d.name);
			show_play_pic(0);
			move(17, 10);
			prints("[1;36m�˷R�� [1;33m%s ��[0m", d.name);
			move(18, 10);
			prints("[1;37m�ݨ�A�o�˧V�O�����i�ۤv����O  ���ڤߤ��Q����������..[m");
			move(19, 10);
			prints("[1;36m�p�ѨϧڨM�w���A���๪�y���y  �����a���U�A�@�U....^_^[0m");
			move(20, 10);
			lucky = rand() % 7;
			if (lucky == 6)
			{
				prints("[1;33m�ڱN���A���U����O�������ɦʤ�������......[0m");
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
				prints("[1;33m�ڱN���A���԰���O�������ɦʤ����Q��.......[0m");
				d.attack = d.attack * 110 / 100;
				d.resist = d.resist * 110 / 100;
				d.speed = d.speed * 110 / 100;
				d.brave = d.brave * 110 / 100;
			}

			else if (lucky <= 3 && lucky >= 2)
			{
				prints("[1;33m�ڱN���A���]�k��O�M�ͩR�O�������ɦʤ����Q��.......[0m");
				d.maxhp = d.maxhp * 110 / 100;
				d.hp = d.maxhp;
				d.maxmp = d.maxmp * 110 / 100;
				d.mp = d.maxmp;
			}
			else if (lucky <= 1 && lucky >= 0)
			{
				prints("[1;33m�ڱN���A���P����O�������ɦʤ����G�Q��....[0m");
				d.character = d.character * 110 / 100;
				d.love = d.love * 110 / 100;
				d.wisdom = d.wisdom * 110 / 100;
				d.art = d.art * 110 / 100;
				d.homework = d.homework * 110 / 100;
			}

			vmsg("���~��[�o��...");
		}
	}
	return 0;
}

int pip_play_kite()	/*����*/
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
	vmsg("�񭷺�u�n����...");
	return 0;
}

int pip_play_KTV()	/*KTV*/
{
	if (d.money < 250)
	{
		vmsg("�A�������h��! �ۺq�`�o���I����");
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
		vmsg("�A���A  �Q�n�k...");
	}
	return 0;
}

int pip_play_guess()   /* �q���{�� */
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
		prints("[1;44;37m  �q�����  [46m[1]�ڥX�ŤM [2]�ڥX���Y [3]�ڥX���� [4]�q���O�� [Q]���X�G         [m");
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
		outs("�p���G�ŤM\n");
		break;
	case 1:
		outs("�p���G���Y\n");
		break;
	case 2:
		outs("�p���G��\n");
		break;
	}

	move(17, 0);

	switch (pipkey)
	{
	case '1':
		outs("�A  �G�ŤM\n");
		if (com == 0)
			tie();
		else  if (com == 1)
			lose();
		else if (com == 2)
			win();
		break;
	case '2':
		outs("�A�@�G���Y\n");
		if (com == 0)
			win();
		else if (com == 1)
			tie();
		else if (com == 2)
			lose();
		break;
	case '3':
		outs("�A�@�G��\n");
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
	vmsg("�p����F....~>_<~");
	return;
}

void tie()
{
	d.hp -= rand() % 2 + 3;
	d.happy += rand() % 3 + 5;
	move(4, 0);
	show_guess_pic(3);
	move(b_lines, 0);
	vmsg("����........-_-");
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
	vmsg("�p��Ĺ�o....*^_^*");
	return;
}

void situ()
{
	clrchyiuan(19, 21);
	move(19, 0);
	prints("�A:[44m %d�� %d�t[m                     \n", d.winn, d.losee);
	move(20, 0);
	prints("��:[44m %d�� %d�t[m                     \n", d.losee, d.winn);

	if (d.winn >= d.losee)
	{
		move(b_lines, 0);
		vmsg("��..Ĺ�p���]�S�h���a");
	}
	else
	{
		move(b_lines, 0);
		vmsg("�³J..���鵹�F��....��...");
	}
	return;
}

/*---------------------------------------------------------------------------*/
/* �צ���:���� �m�Z �צ�                                                   */
/*                                                                           */
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/* ��Ʈw                                                                    */
/*---------------------------------------------------------------------------*/
char *classrank[6] = {"�S��", "���", "����", "����", "�i��", "�M�~"};
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
	{"�ҦW", "���\\�@", "���\\�G", "���Ѥ@", "���ѤG"},

	{"�۵M���", "���b�Υ\\Ū�Ѥ�..", "�ڬO�o���� cccc...",
	 "�o�D���ݤ�����..�ǤF", "�ᤣ���F :~~~~~~"},

	{"��֧���", "�ɫe�����...�ìO�a�W��...", "�����ͫn��..�K�ӵo�X�K..",
	 "��..�W�Ҥ��n�y�f��", "�A�ٲV��..�@�A�I�|��֤T�ʭ�"},

	{"���ǱШ|", "���p����  ���p����", "���ڭ̪ﱵ�Ѱ󤧪�",
	 "��..�A�b�F����? �٤��n�n��", "���ǫ��Y�ª�..�Цn�n��..:("},

	{"�x�ǱШ|", "�]�l�L�k�O����L�k��..", "�q�x����A�ڭn�a�L�h���M",
	 "����}�Σ�?�V�ð}��?? @_@", "�A�٥H���A�b���T��ӣ�?"},

	{"�C�D�޳N", "�ݧڪ��F�`  �W�t�E�C....", "�ڨ� �ڨ� �ڨ���..",
	 "�C�n��í�@�I��..", "�A�b��a����? �C�����@�I"},

	{"�氫�ԧ�", "�٦׬O�٦�  �I�I..", "�Q�K�ɤH���..",
	 "�}�A�𰪤@�I��...", "���Y���o��S�O��.."},

	{"�]�k�Ш|", "���� ���� ��������..", "�D�x+���i��+����+����=??",
	 "�p�ߧA��������  ���n�ô�..", "����f�����n�y������y�W.."},

	{"§���Ш|", "�n����§������...", "�ڶ٭�..��������..",
	 "���Ǥ��|��??�ѧr..", "���_���ӨS����..�ѣ�.."},

	{"ø�e�ޥ�", "�ܤ�����..�����N�ѥ�..", "�o�T�e���C��f�t���ܦn..",
	 "���n���e�Ű�..�n�[�o..", "���n�r�e����..�a�a�p����.."},

	{"�R�Чޥ�", "�A�N���@�����Z��..", "�R�вӭM�ܦn��..",
	 "����A�X�n�@�I..", "���U�A�u���@�I..���n�o��ʾ|.."}
};
/*---------------------------------------------------------------------------*/
/* �צ���:���� �m�Z �צ�                                                   */
/* �禡�w                                                                    */
/*---------------------------------------------------------------------------*/

int pip_practice_classA()
{
	/*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	/*  �x�۵M��Ǣx���O + 1~ 4 , �H�� - 0~0 , ���]��O - 0~0   �x*/
	/*  �x        �u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	/*  �x        �x���O + 2~ 6 , �H�� - 0~1 , ���]��O - 0~1   �x*/
	/*  �x        �u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	/*  �x        �x���O + 3~ 8 , �H�� - 0~2 , ���]��O - 0~1   �x*/
	/*  �x        �u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	/*  �x        �x���O + 4~12 , �H�� - 1~3 , ���]��O - 0~1   �x*/
	/*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	int body, class;
	int change1, change2, change3, change4, change5;

	class = d.wisdom / 200 + 1; /*���*/
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
	/*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	/*  �x�ֵ�    �x�P�� + 1~1 , ���O + 0~1 , ���N�׾i + 0~1    �x*/
	/*  �x        �x��� + 0~1                                  �x*/
	/*  �x        �u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	/*  �x        �x�P�� + 1~2 , ���O + 0~2 , ���N�׾i + 0~1    �x*/
	/*  �x        �x��� + 0~1                                  �x*/
	/*  �x        �u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	/*  �x        �x�P�� + 1~4 , ���O + 0~3 , ���N�׾i + 0~1    �x*/
	/*  �x        �x��� + 0~1                                  �x*/
	/*  �x        �u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	/*  �x        �x�P�� + 2~5 , ���O + 0~4 , ���N�׾i + 0~1    �x*/
	/*  �x        �x��� + 0~1                                  �x*/
	/*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	int body, class;
	int change1, change2, change3, change4, change5;

	class = (d.affect * 2 + d.wisdom + d.art * 2 + d.character) / 400 + 1; /*�ֵ�*/
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
	/*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	/*  �x����    �x���O + 1~1 , �H�� + 1~2 , ���]��O + 0~1    �x*/
	/*  �x        �u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	/*  �x        �x���O + 1~1 , �H�� + 1~3 , ���]��O + 0~1    �x*/
	/*  �x        �u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	/*  �x        �x���O + 1~2 , �H�� + 1~4 , ���]��O + 0~1    �x*/
	/*  �x        �u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	/*  �x        �x���O + 1~3 , �H�� + 1~5 , ���]��O + 0~1    �x*/
	/*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	int body, class;
	int change1, change2, change3, change4, change5;

	class = (d.belief * 2 + d.wisdom) / 400 + 1; /*����*/
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
	/*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	/*  �x�x��    �x���O + 1~2 , �԰��޳N + 0~1 , �P�� - 0~1    �x*/
	/*  �x        �u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	/*  �x        �x���O + 2~4 , �԰��޳N + 0~1 , �P�� - 0~1    �x*/
	/*  �x        �u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	/*  �x        �x���O + 3~4 , �԰��޳N + 0~1 , �P�� - 0~1    �x*/
	/*  �x        �u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	/*  �x        �x���O + 4~5 , �԰��޳N + 0~1 , �P�� - 0~1    �x*/
	/*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
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
	/*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	/*  �x�C�N    �x�԰��޳N + 0~1 , ������O + 1~1             �x*/
	/*  �x        �u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	/*  �x        �x�԰��޳N + 0~1 , ������O + 1~2             �x*/
	/*  �x        �u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	/*  �x        �x�԰��޳N + 0~1 , ������O + 1~3             �x*/
	/*  �x        �u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	/*  �x        �x�԰��޳N + 0~1 , ������O + 1~4             �x*/
	/*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
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
	/*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	/*  �x�氫�N  �x�԰��޳N + 1~1 , ���m��O + 0~0             �x*/
	/*  �x        �u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	/*  �x        �x�԰��޳N + 1~1 , ���m��O + 0~1             �x*/
	/*  �x        �u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	/*  �x        �x�԰��޳N + 1~2 , ���m��O + 0~1             �x*/
	/*  �x        �u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	/*  �x        �x�԰��޳N + 1~3 , ���m��O + 0~1             �x*/
	/*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
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
	/*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	/*  �x�]�k    �x�]�k�޳N + 1~1 , �]�k��O + 0~2             �x*/
	/*  �x        �u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	/*  �x        �x�]�k�޳N + 1~2 , �]�k��O + 0~3             �x*/
	/*  �x        �u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	/*  �x        �x�]�k�޳N + 1~3 , �]�k��O + 0~4             �x*/
	/*  �x        �u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	/*  �x        �x�]�k�޳N + 2~4 , �]�k��O + 0~5             �x*/
	/*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
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
	/*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	/*  �x§��    �x§����{ + 1~1 , ��� + 1~1                 �x*/
	/*  �x        �u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	/*  �x        �x§����{ + 1~2 , ��� + 1~2                 �x*/
	/*  �x        �u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	/*  �x        �x§����{ + 1~3 , ��� + 1~3                 �x*/
	/*  �x        �u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	/*  �x        �x§����{ + 2~4 , ��� + 1~4                 �x*/
	/*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
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
	/*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	/*  �xø�e    �x���N�׾i + 1~1 , �P�� + 0~1                 �x*/
	/*  �x        �u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	/*  �x        �x���N�׾i + 1~2 , �P�� + 0~1                 �x*/
	/*  �x        �u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	/*  �x        �x���N�׾i + 1~3 , �P�� + 0~1                 �x*/
	/*  �x        �u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	/*  �x        �x���N�׾i + 2~4 , �P�� + 0~1                 �x*/
	/*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
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
	/*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	/*  �x�R��    �x���N�׾i + 0~1 , �y�O + 0~1 , ��O + 1~1    �x*/
	/*  �x        �u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	/*  �x        �x���N�׾i + 1~1 , �y�O + 0~1 , ��O + 1~1    �x*/
	/*  �x        �u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	/*  �x        �x���N�׾i + 1~2 , �y�O + 0~2 , ��O + 1~1    �x*/
	/*  �x        �u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
	/*  �x        �x���N�׾i + 1~3 , �y�O + 1~2 , ��O + 1~1    �x*/
	/*  �|�w�w�w�w�r�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�}*/
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

/*�ǤJ:�Ҹ� ���� �ͩR �ּ� ���� żż �Ǧ^:�ܼ�12345 return:body*/
int
pip_practice_function(classnum, classgrade, pic1, pic2, change1, change2, change3, change4, change5)
int classnum, classgrade, pic1, pic2;
int *change1, *change2, *change3, *change4, *change5;
{
	int  a, b, body, health;
	char inbuf[256], ans[5];
	long smoney;

	/*������k*/
	smoney = classgrade * classmoney[classnum][0] + classmoney[classnum][1];
	move(b_lines - 2, 0);
	clrtoeol();
	sprintf(inbuf, "[%8s%4s�ҵ{]�n�� $%d ,�T�w�n��??[y/N]: ", classword[classnum][0], classrank[classgrade], smoney);
	getdata(b_lines - 2, 1, inbuf, ans, 2, DOECHO, 0);
	if (ans[0] != 'y' && ans[0] != 'Y')  return 0;
	if (d.money < smoney)
	{
		vmsg("�ܩ�p��...�A����������");
		return 0;
	}
	count_tired(4, 5, "Y", 100, 1);
	d.money = d.money - smoney;
	/*���\�P�_���P�_*/
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
		sprintf(inbuf, "�U�����W [%8s%4s�ҵ{]",
				classword[classnum][0], classrank[classgrade+1]);
		vmsg(inbuf);
	}
	return 0;
}

/*---------------------------------------------------------------------------*/
/* �S����:�ݯf ��� �԰� ���X �¨�                                         */
/*                                                                           */
/*---------------------------------------------------------------------------*/


int pip_see_doctor()	/*�����*/
{
	char buf[256];
	long savemoney;
	savemoney = d.sick * 25;
	if (d.sick <= 0)
	{
		vmsg("�z��..�S�f����|�F��..�Q�|�F..��~~");
		d.character -= (rand() % 3 + 1);
		if (d.character < 0)
			d.character = 0;
		d.happy -= (rand() % 3 + 3);
		d.satisfy -= rand() % 3 + 2;
	}
	else if (d.money < savemoney)
	{
		sprintf(buf, "�A���f�n�� %d ����....�A��������...", savemoney);
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
		vmsg("�Ĩ�f��..�S���Ƨ@��!!");
	}
	return 0;
}

/*���*/
int pip_change_weight()
{
	char genbuf[5];
	char inbuf[256];
	int weightmp;

	move(b_lines -1, 0);
	clrtoeol();
	show_special_pic(2);
	getdata(b_lines - 1, 1, "�A����ܬO? [Q]���}: ", genbuf, 2, 1, 0);
	if (genbuf[0] == '1' || genbuf[0] == '2' || genbuf[0] == '3' || genbuf[0] == '4')
	{
		switch (genbuf[0])
		{
		case '1':
			if (d.money < 80)
			{
				vmsg("�ǲμW�D�n80����....�A��������...");
			}
			else
			{
				getdata(b_lines - 1, 1, "�ݪ�O80��(3��5����)�A�A�T�w��? [y/N]: ", genbuf, 2, 1, 0);
				if (genbuf[0] == 'Y' || genbuf[0] == 'y')
				{
					weightmp = 3 + rand() % 3;
					d.weight += weightmp;
					d.money -= 80;
					d.maxhp -= rand() % 2;
					d.hp -= rand() % 2 + 3;
					show_special_pic(3);
					sprintf(inbuf, "�`�@�W�[�F%d����", weightmp);
					vmsg(inbuf);
				}
				else
				{
					vmsg("�^����N�o.....");
				}
			}
			break;

		case '2':
			getdata(b_lines - 1, 1, "�W�@����n30���A�A�n�W�h�֤���O? [�ж�Ʀr]: ", genbuf, 4, 1, 0);
			weightmp = atoi(genbuf);
			if (weightmp <= 0)
			{
				vmsg("��J���~..����o...");
			}
			else if (d.money > (weightmp*30))
			{
				sprintf(inbuf, "�W�[%d����A�`�@�ݪ�O�F%d���A�T�w��? [y/N]: ", weightmp, weightmp*30);
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
					sprintf(inbuf, "�`�@�W�[�F%d����", weightmp);
					vmsg(inbuf);
				}
				else
				{
					vmsg("�^����N�o.....");
				}
			}
			else
			{
				vmsg("�A���S����h��.......");
			}
			break;

		case '3':
			if (d.money < 80)
			{
				vmsg("�ǲδ�έn80����....�A��������...");
			}
			else
			{
				getdata(b_lines - 1, 1, "�ݪ�O80��(3��5����)�A�A�T�w��? [y/N]: ", genbuf, 2, 1, 0);
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
					sprintf(inbuf, "�`�@��֤F%d����", weightmp);
					vmsg(inbuf);
				}
				else
				{
					vmsg("�^����N�o.....");
				}
			}
			break;
		case '4':
			getdata(b_lines - 1, 1, "��@����n30���A�A�n��h�֤���O? [�ж�Ʀr]: ", genbuf, 4, 1, 0);
			weightmp = atoi(genbuf);
			if (weightmp <= 0)
			{
				vmsg("��J���~..����o...");
			}
			else if (d.weight <= weightmp)
			{
				vmsg("�A�S���򭫳�.....");
			}
			else if (d.money > (weightmp*30))
			{
				sprintf(inbuf, "���%d����A�`�@�ݪ�O�F%d���A�T�w��? [y/N]: ", weightmp, weightmp*30);
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
					sprintf(inbuf, "�`�@��֤F%d����", weightmp);
					vmsg(inbuf);
				}
				else
				{
					vmsg("�^����N�o.....");
				}
			}
			else
			{
				vmsg("�A���S����h��.......");
			}
			break;
		}
	}
	return 0;
}


/*�Ѩ�*/

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
	char *needmode[3] = {"      ", "§����{��", "�ͦR�ޥ���"};
	int save[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	d.nodone = 0;
	do
	{
		clear();
		show_palace_pic(0);
		move(13, 4);
		sprintf(buf, "[1;31m�z�w�w�w�w�w�w�t[37;41m �Ө��`�q�O���F  �п�ܧA�����X����H [0;1;31m�u�w�w�w�w�w�w�{[0m");
		prints(buf);
		move(14, 4);
		sprintf(buf, "[1;31m�x                                                                  �x[0m");
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
				sprintf(buf, "[1;31m�x [36m([37m%s[36m) [33m%-10s  [37m%-14s     [36m([37m%s[36m) [33m%-10s  [37m%-14s[31m�x[0m",
						p[a].num, p[a].name, inbuf1, p[b].num, p[b].name, inbuf2);
			else
				sprintf(buf, "[1;31m�x [36m([37m%s[36m) [33m%-10s  [37m%-14s                                   [31m�x[0m",
						p[a].num, p[a].name, inbuf1);
			prints(buf);
		}
		move(20, 4);
		sprintf(buf, "[1;31m�x                                                                  �x[0m");
		prints(buf);
		move(21, 4);
		sprintf(buf, "[1;31m�|�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�}[0m");
		prints(buf);


		if (d.death == 1 || d.death == 2 || d.death == 3)
			return 0;
		/*�N�U�H�Ȥw�g���P���ƭȥs�^��*/
		save[1] = d.royalA;          /*from�u��*/
		save[2] = d.royalB;          /*from���*/
		save[3] = d.royalC;		/*from�N�x*/
		save[4] = d.royalD;          /*from�j��*/
		save[5] = d.royalE;          /*from���q*/
		save[6] = d.royalF;          /*from�d�m*/
		save[7] = d.royalG;          /*from���m*/
		save[8] = d.royalH;          /*from���*/
		save[9] = d.royalI;          /*from�p��*/
		save[10] = d.royalJ;         /*from���l*/

		move(b_lines - 1, 0);
		clrtoeol();
		move(b_lines - 1, 0);
		prints("[1;33m [�ͩR�O] %d/%d  [�h�ҫ�] %d [0m", d.hp, d.maxhp, d.tired);

		move(b_lines, 0);
		clrtoeol();
		move(b_lines, 0);
		prints(
			"[1;37;46m  �Ѩ����  [44m [�r��]��ܱ����X���H��  [Q]���}" NICKNAME "�`�q�O���G                  [0m");
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
				pipdie("[1;31m�֦��F...[m  ", 1);
			}
			if (d.hp < 0)
			{
				d.death = 1;
				pipdie("[1;31m�j���F...[m  ", 1);
			}
			if (d.death == 1)
			{
				sprintf(buf, "�T�T�F...�u�O�d��..");
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
							sprintf(buf, "��M�o�򰶤j���A���ܯu�O�a����...");
						else
							sprintf(buf, "�ܰ����A�ӫ��X�ڡA���ڤ��൹�A����F..");
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
							/*�p�G�j��C�����W�[�̤j�q*/
							if (change > p[choice].addtoman)
								change = p[choice].addtoman;
							/*�p�G�[�W���������j��ү൹���Ҧ��Ȯ�*/
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
						sprintf(buf, "�ڤ��M�p�o�˪����͸�....");
					else
						sprintf(buf, "�A�o���S�оi�����A�A�h�Ǿ�§���a....");

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

	vmsg("���}" NICKNAME "�`�q�O��.....");
	return 0;
}
/*--------------------------------------------------------------------------*/
/* pip_vs_fight.c �p����Ե{��				                    */
/* �@��:chyiuan   �P��SiEpthero���޳N����				    */
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
	int notyou = 0;			/*chyiuan:�H�K�T���Q�˿�*/
	float mresist;
	UTMP *opponent;
	char data[200], buf1[256], buf2[256], mymsg[8][150];

	memcpy(&temp, &(cutmp->pip), sizeof(pipdata));
	memcpy(&chickentemp, &d, sizeof(d));


	currutmp = cutmp;
	utmp_mode(M_CHICKEN);
	clear();
	pip_read_file(cuser.userid);
	currutmp->pip->pipmode = 0;	/*1:��F 2:Ĺ�F 3:�����F */
	currutmp->pip->leaving = 1;
	currutmp->pip->mode = d.chickenmode;
	pip_set_currutmp();		/*��p����data  down load for�Q�I�s��*/
	currutmp->pip->nodone = first;	/*�M�w�֥�����*/
	currutmp->pip->msgcount = 0;	/*�԰��T���k�s*/
	currutmp->pip->chatcount = 0;	/*��ѰT���k�s*/
	currutmp->pip->msg[0] = '\0';
	strcpy(currutmp->pip->name, d.name);


	/*�s�U�¤p��data*/
	oldmexp = d.mexp;
	oldhexp = d.hexp;
	oldbrave = d.brave;
	oldhskill = d.hskill;
	oldmskill = d.mskill;
	opponent = cutmp->talker;
	add_io(fd, 2);
	/*��襼�ǳƧ���  �����@�U  ���F������ */
	while (gameover == 0 && (opponent->pip == NULL || opponent->pip->leaving == 0))
	{
		move(b_lines, 0);
		prints("[1;46m ����٦b�ǳƤ�                                                        [m");
		ch = vkey();
	}
	if (currutmp->pip->mode != opponent->pip->mode)
	{
		vmsg("�@�N���P�G�N�����ब�� PK !!");
		add_io(0, 60);
		return 0;
	}
	for (i = 0;i < 8;i++)
		mymsg[i][0] = '\0';
	for (i = 0;i < 10;i++)
		currutmp->pip->chat[i][0] = '\0';
	/*�}�l���T��*/
	sprintf(mymsg[0], "[1;37m%s �M %s ���԰��}�l�F..[m",
			opponent->pip->name, currutmp->pip->name);
	strcpy(currutmp->pip->msg, mymsg[0]);
	currutmp->pip->msgcount = 0;
	/*msgcount�Mcharcount����k���P*/
	add_io(fd, 1);
	/*	currutmp->pip->mode=0;*/
	while (!(opponent->pip || currutmp->pip->leaving == 0 || opponent->pip->leaving == 0))
	{
		clear();
		/*���F�@�Ǩ�L����]  ���������O�I�s�ª�  �ҥHreload*/
		pip_get_currutmp();
		/*		pip_set_currutmp();*/

		if (opponent->pip->nodone != 1)
			strcpy(mymsg[currutmp->pip->msgcount%8], currutmp->pip->msg);
		move(0, 0);
		prints("[1;34m����[44;37m �ۤv��� [0;1;34m����������������������������������������������������������������[m\n");
		prints("[1m   [33m�m  �W:[37m%-20s                                              [31m  [m\n",
			   d.name);
		sprintf(buf1, "%d/%d", d.hp, d.maxhp);
		sprintf(buf2, "%d/%d", d.mp, d.maxmp);
		prints("[1m   [33m��  �O:[37m%-24s       [33m�k  �O:[37m%-24s[33m[m\n",
			   buf1, buf2);
		prints("[1m   [33m��  ��:[37m%-12d[33m��  �m:[37m%-12d[33m�t  ��:[37m%-12d[33m��  �]:[37m%-9d  [m\n",
			   d.attack, d.resist, d.speed, d.mresist);
		prints("[1m   [33m�԰���:[37m%-12d[33m�]�k��:[37m%-12d[33m�]����:[37m%-12d[33m�Z����:[37m%-9d  [m\n",
			   d.hskill, d.mskill, d.mexp, d.hexp);
		prints("[1m   [33m��  ��:[37m%-12d[33m��  �Y:[37m%-12d[33m�s  ��:[37m%-12d[33m�F  ��:[37m%-9d  [m\n",
			   d.food, d.bighp, d.cookie, d.medicine);
		prints("[1m   [33m�H  �x:[37m%-12d[33m��  ��:[37m%-12d[33m�h  ��:[37m%-15d               [m\n",
			   d.ginseng, d.snowgrass, d.tired);
		move(7, 0);
		prints("[1;34m����[44;37m �԰��T�� [0;1;34m����������������������������������������������������������������[m\n");
		for (i = 0;i < 8;i++)
		{
			move(8 + i, 1);

			if (currutmp->pip->msgcount < 8)
			{
				prints(mymsg[i]);
				/*�A��pip.msgcount�b8�椺*/
			}
			else
			{
				prints(mymsg[(currutmp->pip->msgcount-8+i)%8]);
				/*pip.msgcount=8:��ܤw�g��9�� �ҥH�q0->7*/
			}
		}
		move(16, 0);
		prints("[1;34m����[44;37m �͸ܰT�� [0;1;34m����������������������������������������������������������������[m\n");
		for (i = 0;i < 2;i++)
		{
			move(17 + i, 0);
			if (currutmp->pip->chatcount < 3)
			{
				prints(currutmp->pip->chat[i]);
				/*�A��pip.chatcount�b2�椺*/
			}
			else
			{
				prints("%s", currutmp->pip->chat[(currutmp->pip->chatcount-2+i)%10]);
				/*pip.chatcount=3:��ܤw�g��2�� �ҥH�q0->1*/
			}
		}
		move(19, 0);
		prints("[1;34m����[1;37;44m ����� [0;1;34m����������������������������������������������������������������[m\n");
		prints("[1m   [33m�m  �W:[37m%-20s                                                [m\n",
			   opponent->pip->name);
		sprintf(buf1, "%d/%d", opponent->pip->hp, opponent->pip->maxhp);
		sprintf(buf2, "%d/%d", opponent->pip->mp, opponent->pip->maxmp);
		prints("[1m   [33m��  �O:[37m%-24s       [33m�k  �O:[37m%-24s[m\n",
			   buf1, buf2);
		prints("[1;34m������������������������������������������������������������������������������[m\n");
		if (opponent->pip->nodone == 1)
		{
			notyou = 1;
			prints("[1;37;44m  ���X�ۤ��A�еy�ݤ@�|.....                                [T/^T]CHAT/�^�U  [m");
		}
		else
		{
			notyou = 0;
			prints("[1;44;37m  �԰��R�O  [46m [1]���q [2]���O [3]�]�k [4]���m [5]�ɥR [6]�k�R [T/^T]CHAT/�^�U  [m");
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
			len = getdata(b_lines, 0, "�Q��: ", buf, 60, 1, 0);
			if (len && buf[0] != ' ')
			{
				sprintf(msg, "[1;46;33m��%s[37;45m %s [m", cuser.userid, buf);
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
			prints("[1;31m����[41;37m �^�U�͸� [0;1;31m����������������������������������������������������������������[m\n");
			for (i = 0;i < 10;i++)
			{
				move(8 + i, 0);
				if (currutmp->pip->chatcount < 10)
				{
					prints(currutmp->pip->chat[i]);
					/*�A��pip.msgcount�b�C�椺*/
				}
				else
				{
					prints("%s", currutmp->pip->chat[(currutmp->pip->chatcount-10+i)%10]);
					/*pip.chatcount=10:��ܤw�g��11�� �ҥH�q0->9*/
				}
			}
			move(18, 0);
			prints("[1;31m����[41;37m �즹���� [0;1;31m����������������������������������������������������������������[m");
			vmsg("�^�U���e���͸� �u��10�q");
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
					vmsg("���M�S����..:~~~");
					sprintf(buf, "[1;33m%s [37m�� [33m%s[37m �I�i���q�����A���O�S������...",
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
					sprintf(buf, "���q����,�����O��C%d", dinjure);
					vmsg(buf);
					sprintf(buf, "[1;33m%s [37m�I�i�F���q����,[33m%s [37m����O��C [31m%d [37m�I[m"
							, d.name, opponent->pip->name, dinjure);
				}
				opponent->pip->resistmore = 0;
				opponent->pip->msgcount++;
				currutmp->pip->msgcount++;
				strcpy(opponent->pip->msg, buf);
				strcpy(mymsg[currutmp->pip->msgcount%8], buf);
				currutmp->pip->nodone = 2;	/*����*/
				opponent->pip->nodone = 1;
				break;

			case '2':
				show_fight_pic(2);
				if (rand() % 11 == 0)
				{
					vmsg("���M�S����..:~~~");
					sprintf(buf, "[1;33m%s [37m�� [33m%s[37m �I�i���O�����A���O�S������...",
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
						sprintf(buf, "���O����,�����O��C%d", dinjure);
						vmsg(buf);
						sprintf(buf, "[1;33m%s [37m�I�i�F���O����,[33m%s [37m����O��C [31m%d [37m�I[m"
								, d.name, opponent->pip->name, dinjure);
					}
					else
					{
						d.nodone = 1;
						vmsg("�A��HP�p��5��..�����...");
					}
				}
				opponent->pip->resistmore = 0;
				opponent->pip->msgcount++;
				currutmp->pip->msgcount++;
				strcpy(opponent->pip->msg, buf);
				strcpy(mymsg[currutmp->pip->msgcount%8], buf);
				currutmp->pip->nodone = 2;	/*����*/
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
						sprintf(buf, "�v����,��O����%d �h�ҭ��C%d", oldhp, oldtired);
						vmsg(buf);
						sprintf(buf, "[1;33m%s [37m�ϥ��]�k�v������,��O���� [36m%d [37m�I�A�h�ҭ��C [36m%d [37m�I[m", d.name, oldhp, oldtired);
					}
					else
					{
						if (rand() % 15 == 0)
						{
							vmsg("���M�S����..:~~~");
							sprintf(buf, "[1;33m%s [37m�� [33m%s[37m �I�i�]�k�����A���O�S������...",
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
							sprintf(buf, "�]�k����,�����O��C%d", dinjure);
							vmsg(buf);
							sprintf(buf, "[1;33m%s [37m�I�i�F�]�k����,[33m%s [37m����O��C [31m%d [37m�I[m"
									, d.name, opponent->pip->name, dinjure);
						}
					}

					opponent->pip->msgcount++;
					currutmp->pip->msgcount++;
					strcpy(opponent->pip->msg, buf);
					strcpy(mymsg[currutmp->pip->msgcount%8], buf);
					/*��_��O�O��d.hp�Md.maxhp�h �ҥH�o��s*/
					currutmp->pip->hp = d.hp;
					currutmp->pip->mp = d.mp;
					currutmp->pip->nodone = 2;	/*����*/
					opponent->pip->nodone = 1;
					pip_set_currutmp();
				}
				break;

			case '4':
				currutmp->pip->resistmore = 1;
				vmsg("�p���[�j���m��....");
				sprintf(buf, "[1;33m%s [37m�[�j���m�A�ǳƥ��O��� [33m%s [37m���U�@��[m",
						d.name, opponent->pip->name);
				opponent->pip->msgcount++;
				currutmp->pip->msgcount++;
				strcpy(opponent->pip->msg, buf);
				strcpy(mymsg[currutmp->pip->msgcount%8], buf);
				currutmp->pip->nodone = 2;	/*����*/
				opponent->pip->nodone = 1;
				break;
			case '5':
				add_io(fd, 60);
				pip_fight_feed();
				add_io(fd, 1);
				if (d.nodone != 1)
				{
					sprintf(buf, "[1;33m%s [37m�ɥR�F���W����q�A��O�Ϊk�O����۪�����[m", d.name);
					opponent->pip->msgcount++;
					currutmp->pip->msgcount++;
					strcpy(opponent->pip->msg, buf);
					strcpy(mymsg[currutmp->pip->msgcount%8], buf);
					/*��_��O�O��d.hp�Md.maxhp�h �ҥH�o��s*/
					currutmp->pip->hp = d.hp;
					currutmp->pip->mp = d.mp;
					currutmp->pip->nodone = 2;	/*����*/
					opponent->pip->nodone = 1;
					pip_set_currutmp();
				}
				break;
			case '6':
				opponent->pip->msgcount++;
				currutmp->pip->msgcount++;
				if (rand() % 20 >= 18 || (rand() % 20 > 13 && d.speed <= opponent->pip->speed))
				{
					vmsg("�Q�k�]�A�o���ѤF...");
					sprintf(buf, "[1;33m%s [37m�Q���k�]�A��...���o���ѤF...[m", d.name);
					strcpy(opponent->pip->msg, buf);
					strcpy(mymsg[currutmp->pip->msgcount%8], buf);
				}
				else
				{
					sprintf(buf, "[1;33m%s [37m��ı�����L���A�ҥH�M�w���k�]�A��...[m", d.name);
					strcpy(opponent->pip->msg, buf);
					strcpy(mymsg[currutmp->pip->msgcount%8], buf);
					currutmp->pip->pipmode = 3;
					clear();
					vs_head("�q�l�i�p��", BoardName);
					move(10, 0);
					prints("            [1;31m�z�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�{[m\n");
					prints("            [1;31m�x  [37m��O���j���p�� [33m%-10s                 [31m�x[m\n", d.name);
					prints("            [1;31m�x  [37m�b�P��� [32m%-10s [37m�԰��Ḩ�]��          [31m�x[m\n", opponent->pip->name);
					prints("            [1;31m�|�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�}[m\n");
					currutmp->pip->leaving = 0;
					add_io(fd, 60);
					vmsg("�T�Q���p �����W��...");
				}
				currutmp->pip->nodone = 2;	/*����*/
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
			vs_head("�q�l�i�p��", BoardName);
			move(10, 0);
			prints("            [1;31m�z�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�{[m\n");
			prints("            [1;31m�x  [37m�^�i���p�� [33m%-10s                     [31m�x[m\n", d.name);
			prints("            [1;31m�x  [37m���ѤF���p�� [32m%-10s                 [31m�x[m\n", opponent->pip->name);
			prints("            [1;31m�|�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�}[m");
			currutmp->pip->leaving = 0;
			add_io(fd, 60);
			if (opponent->pip->hp <= 0)
				vmsg("��覺���o..�ҥH�AĹ�o..");
			else if (opponent->pip->hp > 0)
				vmsg("��踨�]�o..�ҥH��AĹ�o.....");
		}
		if (gameover != 1 && (opponent->pip->pipmode == 2 || currutmp->pip->pipmode == 1))
		{
			clear();
			vs_head("�q�l�i�p��", BoardName);
			move(10, 0);
			prints("            [1;31m�z�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�{[m\n");
			prints("            [1;31m�x  [37m�i�����p�� [33m%-10s                     [31m�x[m\n", d.name);
			prints("            [1;31m�x  [37m�b�P [32m%-10s [37m���԰����A                [31m�x[m\n", opponent->pip->name);
			prints("            [1;31m�x  [37m�����a����F�A�O�̲{���S�O����.........   [31m�x[m\n");
			prints("            [1;31m�|�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�}[m\n");
			currutmp->pip->leaving = 0;
			add_io(fd, 60);
			vmsg("�p������F....");
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
/* �����禡                                                                  */
/*                                                                           */
/*---------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/*  �����ѼƳ]�w                                                            */
/*--------------------------------------------------------------------------*/

int /*�����e��*/
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
	prints("[1;33m������������������������������������������������������������[0m");
	move(2, 9);
	prints("[1;37m��      ����    ������      ����      ����    ������      ��[0m");
	move(3, 9);
	prints("[0;37m��    ������    ������  ������������������    ������  ������[0m");
	move(4, 9);
	prints("[0;37m��    ������  ��  ����  ������������������  ��  ����  ������[0m");
	move(5, 9);
	prints("[1;37m��      ����  ��  ����      ����      ����  ��  ����      ��[0m");
	move(6, 9);
	prints("[1;35m������������������������������������������������������������[0m");
	move(7, 8);
	prints("[1;31m�w�w�w�w�w�w�w�w�w�w[41;37m " NICKNAME PIPNAME "�������i [0;1;31m�w�w�w�w�w�w�w�w�w�w[0m");
	move(9, 10);
	prints("[1;36m�o�Ӯɶ�������ı�a�٬O���{�F...[0m");
	move(11, 10);
	prints("[1;37m[33m%s[37m �o���}�A���ŷx�h��A�ۤv�@�����b�~���D�ͦs�F.....[0m", d.name);
	move(13, 10);
	prints("[1;36m�b�A���U�оɥL���o�q�ɥ��A���L��Ĳ�F�ܦh���A���i�F�ܦh����O....[0m");
	move(15, 10);
	prints("[1;37m�]���o�ǡA���p�� [33m%s[37m ���᪺�ͬ��A�ܱo��h���h���F........[0m", d.name);
	move(17, 10);
	prints("[1;36m���A�����ߡA�A���I�X�A�A�Ҧ����R......[0m");
	move(19, 10);
	prints("[1;37m[33m%s[37m �|�û����ʰO�b�ߪ�....[0m", d.name);
	vmsg("���U�Ӭݥ��ӵo�i");
	clrchyiuan(7, 19);
	move(7, 8);
	prints("[1;34m�w�w�w�w�w�w�w�w�w�w[44;37m " NICKNAME PIPNAME "���ӵo�i [0;1;34m�w�w�w�w�w�w�w�w�w�w[0m");
	move(9, 10);
	prints("[1;36m�z�L�����y�A���ڭ̤@�_�Ӭ� [33m%s[36m �����ӵo�i�a.....[0m", d.name);
	move(11, 10);
	prints("[1;37m�p�� [33m%s[37m ���%s....[0m", d.name, endbuf1);
	move(13, 10);
	prints("[1;36m�]���L�����e���V�O�A�ϱo�L�b�o�@�譱%s....[0m", endbuf2);
	move(15, 10);
	prints("[1;37m�ܩ�p�����B�ê��p�A�L���%s�A�B�ú�O�ܬ���.....[0m", endbuf3);
	move(17, 10);
	prints("[1;36m��..�o�O�@�Ӥ�����������..........[0m");
	vmsg("�ڷQ  �A�@�w�ܷP�ʧa.....");
	show_ending_pic(0);
	vmsg("�ݤ@�ݤ����o");
	endgrade = pip_game_over(endgrade);
	/*  inmoney(endgrade*10*ba);
	  inexp(endgrade*ba);*/
	sprintf(buf, "/bin/rm %s", get_path(cuser.userid, "chicken"));
	system(buf);
	sprintf(buf, "�o�� %d ��,%d �I�g���", endgrade*10*ba, endgrade*10);
	vmsg(buf);
	vmsg("�U�@���O�p�����  ����copy�U�Ӱ�����");
	pip_data_list(cuser.userid);
	vmsg("�w��A�ӬD��....");
	/*�O���}�l*/
	now = time(0);
	sprintf(buf, "[1;35m�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w[0m\n");
	pip_log_record(buf);
	sprintf(buf, "[1;37m�b [33m%s [37m���ɭԡA[36m%s [37m���p�� [32m%s[37m �X�{�F����[0m\n", Cdate(&now), cuser.userid, d.name);
	pip_log_record(buf);
	sprintf(buf, "[1;37m�p�� [32m%s [37m�V�O�[�j�ۤv�A���%s[0m\n[1;37m�]�����e���V�O�A�ϱo�b�o�@�譱%s[0m\n", d.name, endbuf1, endbuf2);
	pip_log_record(buf);
	sprintf(buf, "[1;37m�ܩ�B�ê��p�A�L���%s�A�B�ú�O�ܬ���.....[0m\n\n[1;37m�p�� [32n%s[37m ���`�n���� [33m%d[0m\n", endbuf3, d.name, endgrade);
	pip_log_record(buf);
	sprintf(buf, "[1;35m�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w[0m\n");
	pip_log_record(buf);
	/*�O���פ�*/
	d.death = 3;
	pipdie("[1;31m�C�������o...[m  ", 3);
	return 0;
}

int
pip_ending_decide(endbuf1, endbuf2, endbuf3, endmode, endgrade)
char *endbuf1, *endbuf2, *endbuf3;
int *endmode, *endgrade;
{
	char *name[8][2] = {{"�k��", "�k��"},
		{"�������l", "���F���D"},
		{"�����A", "���F�A"},
		{"�����ӤH��", "���F�k�ӤH��"},
		{"�����ӤH��", "���F�k�ӤH��"},
		{"�����ӤH��", "���F�k�ӤH��"},
		{"�����ӤH��", "���F�k�ӤH��"},
		{"�����ӤH��", "���F�k�ӤH��"}
	};
	int m = 0, n = 0, grade = 0;
	int modeall_purpose = 0;
	char buf1[256];
	char buf2[256];

	*endmode = pip_future_decide(&modeall_purpose);
	switch (*endmode)
	{
		/*1:�t�� 2:���N 3:�U�� 4:�Ԥh 5:�]�k 6:���� 7:�a��*/
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
		sprintf(buf2, "�D�`�����Q..");
	}
	else if (n == 2)
	{
		*endgrade = grade + 100;
		sprintf(buf2, "��{�٤���..");
	}
	else if (n == 3)
	{
		*endgrade = grade - 10;
		sprintf(buf2, "�`�J��ܦh���D....");
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
			sprintf(buf2, "���F�P�檺�k��");
		else
			sprintf(buf2, "�����F�P�檺�k��");
	}
	strcpy(endbuf3, buf2);
	return 0;
}
/*�����P�_*/
/*1:�t�� 2:���N 3:�U�� 4:�Ԥh 5:�]�k 6:���� 7:�a��*/
int
pip_future_decide(modeall_purpose)
int *modeall_purpose;
{
	int endmode;
	/*�t��*/
	if ((d.etchics == 0 && d.offense >= 100) || (d.etchics > 0 && d.etchics < 50 && d.offense >= 250))
		endmode = 1;
	/*���N*/
	else if (d.art > d.hexp && d.art > d.mexp && d.art > d.hskill && d.art > d.mskill &&
			 d.art > d.social && d.art > d.family && d.art > d.homework && d.art > d.wisdom &&
			 d.art > d.charm && d.art > d.belief && d.art > d.manners && d.art > d.speech &&
			 d.art > d.cookskill && d.art > d.love)
		endmode = 2;
	/*�԰�*/
	else if (d.hexp >= d.social && d.hexp >= d.mexp && d.hexp >= d.family)
	{
		*modeall_purpose = 1;
		if (d.hexp > d.social + 50 || d.hexp > d.mexp + 50 || d.hexp > d.family + 50)
			endmode = 4;
		else
			endmode = 3;
	}
	/*�]�k*/
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
/*���B���P�_*/
int
pip_marry_decide()
{
	int grade;
	if (d.lover != 0)
	{
		/* 3 4 5 6 7:�ӤH */
		d.lover = d.lover;
		grade = 80;
	}
	else
	{
		if (d.royalJ >= d.relation && d.royalJ >= 100)
		{
			d.lover = 1;  /*���l*/
			grade = 200;
		}
		else if (d.relation > d.royalJ && d.relation >= 100)
		{
			d.lover = 2;  /*���˩Υ���*/
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
pip_endingblack(buf, m, n, grade) /*�t��*/
char *buf;
int *m, *n, *grade;
{
	if (d.offense >= 500 && d.mexp >= 500) /*�]��*/
	{
		*m = 1;
		if (d.mexp >= 1000)
			*n = 1;
		else if (d.mexp < 1000 && d.mexp >= 800)
			*n = 2;
		else
			*n = 3;
	}

	else if (d.hexp >= 600)  /*�y�]*/
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
	else if (d.hexp >= 320 && d.character > 200 && d.charm < 200)	/*�µ�Ѥj*/
	{
		*m = 4;
		if (d.hexp >= 400)
			*n = 1;
		else if (d.hexp < 400 && d.hexp >= 360)
			*n = 2;
		else
			*n = 3;
	}
	else if (d.character >= 200 && d.charm >= 200 && d.speech > 70 && d.toman > 70)  /*���ű@��*/
	{
		*m = 5;
		if (d.charm >= 300)
			*n = 1;
		else if (d.charm < 300 && d.charm >= 250)
			*n = 2;
		else
			*n = 3;
	}

	else if (d.wisdom >= 450)  /*�B�F�v*/
	{
		*m = 6;
		if (d.wisdom >= 550)
			*n = 1;
		else if (d.wisdom < 550 && d.wisdom >= 500)
			*n = 2;
		else
			*n = 3;
	}

	else /*�y�a*/
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
pip_endingsocial(buf, m, n, grade) /*����*/
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
pip_endingmagic(buf, m, n, grade) /*�]�k*/
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
pip_endingcombat(buf, m, n, grade) /*�԰�*/
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
pip_endingfamily(buf, m, n, grade) /*�a��*/
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
pip_endingall_purpose(buf, m, n, grade, mode) /*�U��*/
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
pip_endingart(buf, m, n, grade) /*���N*/
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
	prints("[1;36m�P�±z�������" NICKNAME "�p�����C��.....[0m");
	move(10, 17);
	prints("[1;37m�g�L�t�έp�⪺���G�G[0m");
	move(12, 17);
	prints("[1;36m�z���p�� [37m%s [36m�`�o���� [1;5;33m%d [0m", d.name, gradeall);
	return gradeall;
}

int pip_divine() /*�e�R�v�ӳX*/
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
	prints("[1;33;5m�n�n�n...[0;1;37m��M�ǨӰ}�}���V���n.........[0m");
	vmsg("�h�@�@�O�֧a......");
	clrchyiuan(6, 18);
	move(10, 14);
	prints("[1;37;46m    ��ӬO���C�|�����e�R�v�ӳX�F.......    [0m");
	vmsg("�}�����L�i�ӧa....");
	if (d.money >= money)
	{
		randvalue = rand() % 5;
		sprintf(buf, "�A�n�e�R��? �n��%d����...[y/N]: ", money);
		getdata(12, 14, buf, ans, 2, 1, 0);
		if (ans[0] == 'y' || ans[0] == 'Y')
		{
			pip_ending_decide(endbuf1, endbuf2, endbuf3, &endmode, &endgrade);
			if (randvalue == 0)
				sprintf(buf, "[1;37m  �A���p��%s�H��i�઺�����O%s  [m", d.name, endmodemagic[2+rand()%5].girl);
			else if (randvalue == 1)
				sprintf(buf, "[1;37m  �A���p��%s�H��i�઺�����O%s  [m", d.name, endmodecombat[2+rand()%6].girl);
			else if (randvalue == 2)
				sprintf(buf, "[1;37m  �A���p��%s�H��i�઺�����O%s  [m", d.name, endmodeall_purpose[6+rand()%15].girl);
			else if (randvalue == 3)
				sprintf(buf, "[1;37m  �A���p��%s�H��i�઺�����O%s  [m", d.name, endmodeart[2+rand()%6].girl);
			else if (randvalue == 4)
				sprintf(buf, "[1;37m  �A���p��%s�H��i�઺�����O%s  [m", d.name, endbuf1);
			d.money -= money;
			clrchyiuan(6, 18);
			move(10, 14);
			prints("[1;33m�b�ڥe�R���G�ݨ�....[m");
			move(12, 14);
			prints(buf);
			vmsg("���´f�U�A���t�A�����F.(���Ǥ���ǧڳ�)");
		}
		else
		{
			vmsg("�A���Q�e�R��?..�u�i��..���u�����U���a...");
		}
	}
	else
	{
		vmsg("�A����������..�u�O�i��..���U���a...");
	}
	return 0;
}

int
pip_money()
{
	char buf[100], ans[10];

	int money = -1;
	if (!d.name[0] || d.death) return;
	clrchyiuan(6, 18);
	/*  move(12,0);
	  clrtobot();*/
	prints("�A���W�� %d ���I�q����,���� %d ��\n", cuser.request, d.money);
	outs("\n�@�����@�d������!!\n");
	while (money < 0 || money > cuser.request)
	{
		getdata(10, 0, "�n���h�֦�? ", ans, 10, LCECHO, 0);
		if (!ans[0]) return;
		money = atol(ans);
	}
	sprintf(buf, "�O�_�n�ഫ %d �� �� %d ����? [y/N]: ", money, money*1000);
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
		sprintf(buf, "�A���W�� %d ���I�q����,���� %d ��", cuser.request, d.money);
	}
	else
		sprintf(buf, "����.....");

	vmsg(buf);
	return 0;
}

/* �N���������I�q���� by statue */
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
	prints("�A���W�� %d ���I�q����,���� %d ��\n", cuser.request, d.money);

	if (d.money <= 5000)
	{
		sprintf(buf, "���������j�� 5000 �~�i�ഫ......");
		vmsg(buf);
		return 0;
	}

	outs("\n���d�������@����!!\n");
	while (money < 0 || (money)*5000 > d.money - START_MONEY)
	{
		getdata(10, 0, "�n���h�֦�? ", ans, 10, LCECHO, 0);
		if (!ans[0]) return;
		money = atol(ans);
	}
	sprintf(buf, "�O�_�n�ഫ %d ���� �� %d ��? [y/N]: ", money*5000, money);
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
		sprintf(buf, "�A���W�� %d ���I�q����,���� %d ��", cuser.request, d.money);
	}
	else
		sprintf(buf, "����.....");

	vmsg(buf);
	return 0;
}
#endif

int pip_query()  /*���X�p��*/
{
	int id;
	char genbuf[STRLEN];

	vs_bar("���X�P��");
	usercomplete(msg_uid, genbuf);
	if (genbuf[0])
	{
		move(2, 0);
		if (id = acct_userno(genbuf))
		{
			pip_read(genbuf);
			vmsg("�[���@�U�O�H���p��...:p");
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
	/*char yo[14][5]={"�ϥ�","����","����","�ൣ","�C�~","�֦~","���~",
	                "���~","���~","���~","��~","�Ѧ~","�Ѧ~","�j�}"};*/
	char yo[12][5] = {"�ϥ�", "����", "����", "�ൣ", "�֦~", "�C�~",
					  "���~", "���~", "��~", "�Ѧ~", "�j�}", "���P"
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

		if (age == 0) /*�ϥ�*/
			age1 = 0;
		else if (age == 1) /*����*/
			age1 = 1;
		else if (age >= 2 && age <= 5) /*����*/
			age1 = 2;
		else if (age >= 6 && age <= 12) /*�ൣ*/
			age1 = 3;
		else if (age >= 13 && age <= 15) /*�֦~*/
			age1 = 4;
		else if (age >= 16 && age <= 18) /*�C�~*/
			age1 = 5;
		else if (age >= 19 && age <= 35) /*���~*/
			age1 = 6;
		else if (age >= 36 && age <= 45) /*���~*/
			age1 = 7;
		else if (age >= 45 && age <= 60) /*��~*/
			age1 = 8;
		else if (age >= 60 && age <= 70) /*�Ѧ~*/
			age1 = 9;
		else if (age >= 70 && age <= 100) /*�j�}*/
			age1 = 10;
		else if (age > 100) /*���P*/
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
		prints("�o�O%s�i���p���G\n", genbuf);

		if (death1 == 0)
		{
			prints("[1;32mName�G%-10s[m  �ͤ�G%2d�~%2d��%2d��   �~�֡G%2d��  ���A�G%s  �����G%d\n"
				   "�ͩR�G%3d/%-3d  �ּ֡G%-4d  ���N�G%-4d  ���G%-4d  ���z�G%-4d  �魫�G%-4d\n"
				   "�j�ɤY�G%-4d   �����G%-4d  �s���G%-4d  �h�ҡG%-4d  żż�G%-4d  �f��G%-4d\n",
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
			if (shit1 == 0) prints("�ܰ��b..");
			if (shit1 > 40 && shit1 < 60) prints("��䪺..");
			if (shit1 >= 60 && shit1 < 80) prints("�n���..");
			if (shit1 >= 80 && shit1 < 100) prints("[1;34m�֯䦺�F..[m");
			if (shit1 >= 100) {prints("[1;31m�䦺�F..[m"); return -1;}

			pc1 = hp1 * 100 / maxhp1;
			if (pc1 == 0) {prints("�j���F.."); return -1;}
			if (pc1 < 20) prints("[1;35m�����L�O��.�־j���F.[m");
			if (pc1 < 40 && pc1 >= 20) prints("��O���Ӱ�..�Q�Y�I�F��..");
			if (pc1 < 100 && pc1 >= 80) prints("���{�l��������O..");
			if (pc1 >= 100) prints("[1;34m�ּ����F..[m");

			pc1 = tired1;
			if (pc1 < 20) prints("�믫�ݧݤ�..");
			if (pc1 < 80 && pc1 >= 60) prints("[1;34m���I�p��..[m");
			if (pc1 < 100 && pc1 >= 80) {prints("[1;31m�n�ֳ�A�֤���F..[m"); }
			if (pc1 >= 100) {prints("�֦��F..."); return -1;}

			pc1 = 60 + 10 * age;
			if (weight1 < (pc1 + 30) && weight1 >= (pc1 + 10)) prints("���I�p�D..");
			if (weight1 < (pc1 + 50) && weight1 >= (pc1 + 30)) prints("�ӭD�F..");
			if (weight1 > (pc1 + 50)) {prints("�D���F..."); return -1;}

			if (weight1 < (pc1 - 50)) {prints("�G���F.."); return -1;}
			if (weight1 > (pc1 - 30) && weight1 <= (pc1 - 10)) prints("���I�p�G..");
			if (weight1 > (pc1 - 50) && weight1 <= (pc1 - 30)) prints("�ӽG�F..");

			if (sick1 < 75 && sick1 >= 50) prints("[1;34m�ͯf�F..[m");
			if (sick1 < 100 && sick1 >= 75) {prints("[1;31m�f��!!..[m"); }
			if (sick1 >= 100) {prints("�f���F.!."); return -1;}

			pc1 = happy1;
			if (pc1 < 20) prints("[1;31m�ܤ��ּ�..[m");
			if (pc1 < 40 && pc1 >= 20) prints("���ּ�..");
			if (pc1 < 95 && pc1 >= 80) prints("�ּ�..");
			if (pc1 <= 100 && pc1 >= 95) prints("�ܧּ�..");

			pc1 = satisfy1;
			if (pc1 < 40) prints("[31;1m������..[m");
			if (pc1 < 95 && pc1 >= 80) prints("����..");
			if (pc1 <= 100 && pc1 >= 95) prints("�ܺ���..");
		}
		else if (death1 == 1)
		{
			show_die_pic(2);
			move(14, 20);
			prints("�i�����p����I�s�v�F");
		}
		else if (death1 == 2)
		{
			show_die_pic(3);
		}
		else if (death1 == 3)
		{
			move(5, 0);
			outs("�C���w�g���쵲���o....");
		}
		else
		{
			vmsg("�ɮ׷l���F....");
		}
	}   /* ���i�p�� */
	else
	{
		move(1, 0);
		clrtobot();
		vmsg("�o�@�a���H�S���i�p��......");
	}
}

/*---------------------------------------------------------------------------*/
/* �t�ο��:�ӤH���  �p�����  �S�O�A��                                     */
/*                                                                           */
/*---------------------------------------------------------------------------*/

char weaponhead[7][10] =
{
	"�S���˳�",
	"�콦�U�l",
	"���֤p�U",
	"���w���U",
	"���K�Y��",
	"�]�k�v�T",
	"�����t��"
};


char weaponrhand[10][10] =
{
	"�S���˳�",
	"�j���",
	"���ݧ��",
	"�C�ɼC",
	"���p�C",
	"���l�M",
	"�ѱ��C",
	"���Y�_�M",
	"�O�s�M",
	"�����t��"
};

char weaponlhand[8][10] =
{
	"�S���˳�",
	"�j���",
	"���ݧ��",
	"���",
	"���ÿ���",
	"�ժ�����",
	"�]�k��",
	"�����t��"
};


char weaponbody[7][10] =
{
	"�S���˳�",
	"�콦�`��",
	"�S�ť֥�",
	"���K����",
	"�]�k�ܭ�",
	"�ժ�����",
	"�����t��"
};

char weaponfoot[8][12] =
{
	"�S���˳�",
	"�콦��c",
	"�F�v��j",
	"�S�ūB�c",
	"NIKE�B�ʾc",
	"�s���ֹu",
	"�����]�u",
	"�����t�u"
};

int
pip_system_freepip()
{
	char buf[256];
	move(b_lines - 1, 0);
	clrtoeol();
	getdata(b_lines - 1, 1, "�u���n��ͶܡH(y/N): ", buf, 2, 1, 0);
	if (buf[0] != 'y' && buf[0] != 'Y') return 0;
	sprintf(buf, "%s �Q���ߪ� %s �ᱼ�F~", d.name, cuser.userid);
	vmsg(buf);
	d.death = 2;
	pipdie("[1;31m�Q���ߥ��:~~[0m", 2);
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
	prints("[1;44m  �A�ȶ���  [46m[1]�R�W�j�v [2]�ܩʤ�N [3]�����]��                                [0m");
	pipkey = vkey();

	switch (pipkey)
	{
	case '1':
		move(b_lines - 1, 0);
		clrtobot();
		getdata(b_lines - 1, 1, "���p�����s���Ӧn�W�r�G ", buf, 11, DOECHO, NULL);
		if (!buf[0])
		{
			vmsg("���@�U�Q�n�A�Ӧn�F  :)");
			break;
		}
		else
		{
			strcpy(oldname, d.name);
			strcpy(d.name, buf);
			/*��W�O��*/
			now = time(0);
			sprintf(buf, "[1;37m%s %-11s��p�� [%s] ��W�� [%s] [0m\n", Cdate(&now), cuser.userid, oldname, d.name);
			pip_log_record(buf);
			vmsg("���  ���@�ӷs���W�r��...");
		}
		break;

	case '2':  /*�ܩ�*/
		move(b_lines - 1, 0);
		clrtobot();
		/*1:�� 2:�� */
		if (d.sex == 1)
		{
			oldchoice = 2; /*��-->��*/
			move(b_lines - 1, 0);
			prints("[1;33m�N�p����[32m��[33m�ܩʦ�[35m��[33m���ܡH [37m[y/N][0m");
		}
		else
		{
			oldchoice = 1; /*��-->��*/
			move(b_lines - 1, 0);
			prints("[1;33m�N�p����[35m��[33m�ܩʦ�[35m��[33m���ܡH [37m[y/N][0m");
		}
		move(b_lines, 0);
		prints("[1;44m  �A�ȶ���  [46m[1]�R�W�j�v [2]�ܩʤ�N [3]�����]��                                [0m");
		pipkey = vkey();
		if (pipkey == 'Y' || pipkey == 'y')
		{
			/*��W�O��*/
			now = time(0);
			if (d.sex == 1)
				sprintf(buf, "[1;37m%s %-11s��p�� [%s] �ѡ��ܩʦ���F[0m\n", Cdate(&now), cuser.userid, d.name);
			else
				sprintf(buf, "[1;37m%s %-11s��p�� [%s] �ѡ��ܩʦ���F[0m\n", Cdate(&now), cuser.userid, d.name);
			pip_log_record(buf);
			vmsg("�ܩʤ�N����...");
			d.sex = oldchoice;
		}
		break;

	case '3':
		move(b_lines - 1, 0);
		clrtobot();
		/*1:���n�B���B 4:�n�B���B */
		oldchoice = d.wantend;
		if (d.wantend == 1 || d.wantend == 2 || d.wantend == 3)
		{
			oldchoice += 3; /*�S��-->��*/
			move(b_lines - 1, 0);
			prints("[1;33m�N�p���C���令[32m[��20������][33m? [37m[y/N][0m");
			sprintf(buf, "�p���C���]�w��[��20������]..");
		}
		else
		{
			oldchoice -= 3; /*��-->�S��*/
			move(b_lines - 1, 0);
			prints("[1;33m�N�p���C���令[32m[�S��20������][33m? [37m[y/N][0m");
			sprintf(buf, "�p���C���]�w��[�S��20������]..");
		}
		move(b_lines, 0);
		prints("[1;44m  �A�ȶ���  [46m[1]�R�W�j�v [2]�ܩʤ�N [3]�����]��                                [0m");
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
pip_data_list(userid)  /*�ݤp���ӤH�ԲӸ��*/
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
		vmsg("�ڨS���i�p���� !");
		return 0;
	}


//  tm=(time(0)-start_time+chicken.bbtime)/60/30;
	tm = chicken.bbtime / 60 / 30;

	clear();
	move(1, 0);
	prints("       [1;33m����������������������������������������[m\n");
	prints("       [0;37m������  ���� ��   �������������� ��   ��[m\n");
	prints("       [1;37m������  ��������  ��  ����    ������  ��[m\n");
	prints("       [1;34m��������������������  ����    ����������[32m......................[m");
	do
	{
		clrchyiuan(5, 23);
		switch (page)
		{
		case 1:
			move(5, 0);
			sprintf(buf,
					"[1;31m �~�t[41;37m �򥻸�� [0;1;31m�u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w��[m\n");
			prints(buf);

			sprintf(buf,
					"[1;31m �x[33m�̩m    �W :[37m %-10s [33m�̥�    �� :[37m %02d/%02d/%02d   [33m�̦~    �� :[37m %-2d         [31m�x[m\n",
					chicken.name, (chicken.year) % 100, chicken.month, chicken.day, tm);
			prints(buf);

			sprintf(inbuf1, "%d%s/%d%s", chicken.hp > 1000 ? chicken.hp / 1000 : chicken.hp, chicken.hp > 1000 ? "K" : "", chicken.maxhp > 1000 ? chicken.maxhp / 1000 : chicken.maxhp, chicken.maxhp > 1000 ? "K" : "");
			sprintf(inbuf2, "%d%s/%d%s", chicken.mp > 1000 ? chicken.mp / 1000 : chicken.mp, chicken.mp > 1000 ? "K" : "", chicken.maxmp > 1000 ? chicken.maxmp / 1000 : chicken.maxmp, chicken.maxmp > 1000 ? "K" : "");

			sprintf(buf,
					"[1;31m �x[33m����    �� :[37m %-5d(�̧J)[33m����    �O :[37m %-11s[33m�̪k    �O :[37m %-11s[31m�x[m\n",
					chicken.weight, inbuf1, inbuf2);
			prints(buf);

			sprintf(buf,
					"[1;31m �x[33m�̯h    �� :[37m %-3d        [33m�̯f    �� :[37m %-3d        [33m��ż    ż :[37m %-3d        [31m�x[m\n",
					chicken.tired, chicken.sick, chicken.shit);
			prints(buf);

			sprintf(buf,
					"[1;31m �x[33m�̵�    �O :[37m %-7d    [33m�̿ˤl���Y :[37m %-7d    [33m�̪�    �� :[37m %-11d[31m�x[m\n",
					chicken.wrist, chicken.relation, chicken.money);
			prints(buf);

			sprintf(buf,
					"[1;31m �u�t[41;37m ��O��� [0;1;31m�u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t[m\n");
			prints(buf);

			sprintf(buf,
					"[1;31m �x[33m�̮�    �� :[37m %-10d [33m�̴�    �O :[37m %-10d [33m�̷R    �� :[37m %-10d [31m�x[m\n",
					chicken.character, chicken.wisdom, chicken.love);
			prints(buf);

			sprintf(buf,
					"[1;31m �x[33m����    �N :[37m %-10d [33m�̹D    �w :[37m %-10d [33m�̮a    �� :[37m %-10d [31m�x[m\n",
					chicken.art, chicken.etchics, chicken.homework);
			prints(buf);

			sprintf(buf,
					"[1;31m �x[33m��§    �� :[37m %-10d [33m����    �� :[37m %-10d [33m�̲i    �� :[37m %-10d [31m�x[m\n",
					chicken.manners, chicken.speech, chicken.cookskill);
			prints(buf);

			sprintf(buf,
					"[1;31m �u�t[41;37m ���A��� [0;1;31m�u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t[m\n");
			prints(buf);

			sprintf(buf,
					"[1;31m �x[33m�̧�    �� :[37m %-10d [33m�̺�    �N :[37m %-10d [33m�̤H    �� :[37m %-10d [31m�x[m\n",
					chicken.happy, chicken.satisfy, chicken.toman);
			prints(buf);

			sprintf(buf,
					"[1;31m �x[33m�̾y    �O :[37m %-10d [33m�̫i    �� :[37m %-10d [33m�̫H    �� :[37m %-10d [31m�x[m\n",
					chicken.charm, chicken.brave, chicken.belief);
			prints(buf);

			sprintf(buf,
					"[1;31m �x[33m�̸o    �^ :[37m %-10d [33m�̷P    �� :[37m %-10d [33m            [37m            [31m�x[m\n",
					chicken.offense, chicken.affect);
			prints(buf);

			sprintf(buf,
					"[1;31m �u�t[41;37m ������� [0;1;31m�u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t[m\n");
			prints(buf);

			sprintf(buf,
					"[1;31m �x[33m�̪������ :[37m %-10d [33m�̾԰����� :[37m %-10d [33m���]�k���� :[37m %-10d [31m�x[m\n",
					chicken.social, chicken.hexp, chicken.mexp);
			prints(buf);

			sprintf(buf,
					"[1;31m �x[33m�̮a�Ƶ��� :[37m %-10d                                                 [31m�x[m\n",
					chicken.family);
			prints(buf);

			sprintf(buf,
					"[1;31m ���w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w��[m\n");
			prints(buf);

			move(b_lines - 1, 0);
			sprintf(buf,
					"                                                              [1;36m�Ĥ@��[37m/[36m�@�G��[m\n");
			prints(buf);
			break;

		case 2:
			move(5, 0);
			sprintf(buf,
					"[1;31m �~�t[41;37m ���~��� [0;1;31m�u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w��[m\n");
			prints(buf);

			sprintf(buf,
					"[1;31m �x[33m�̭�    �� :[37m %-10d [33m�̹s    �� :[37m %-10d [33m�̤j �� �Y :[37m %-10d [31m�x[m\n",
					chicken.food, chicken.cookie, chicken.bighp);
			prints(buf);

			sprintf(buf,
					"[1;31m �x[33m���F    �� :[37m %-10d [33m�̮�    �� :[37m %-10d [33m�̪�    �� :[37m %-10d [31m�x[m\n",
					chicken.medicine, chicken.book, chicken.playtool);
			prints(buf);

			sprintf(buf,
					"[1;31m �u�t[41;37m �C����� [0;1;31m�u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t[m\n");
			prints(buf);

			sprintf(buf,
					"[1;31m �x[33m�̲q �� Ĺ :[37m %-10d [33m�̲q �� �� :[37m %-10d                         [31m�x[m\n",
					chicken.winn, chicken.losee);
			prints(buf);

			sprintf(buf,
					"[1;31m �u�t[41;37m �Z�O��� [0;1;31m�u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t[m\n");
			prints(buf);

			sprintf(buf,
					"[1;31m �x[33m�̧� �� �O :[37m %-10d [33m�̨� �m �O :[37m %-10d [33m�̳t �� �� :[37m %-10d [31m�x[m\n",
					chicken.attack, chicken.resist, chicken.speed);
			prints(buf);
			sprintf(buf,
					"[1;31m �x[33m�̧��]��O :[37m %-10d [33m�̾԰��޳N :[37m %-10d [33m���]�k�޳N :[37m %-10d [31m�x[m\n",
					chicken.mresist, chicken.hskill, chicken.mskill);
			prints(buf);

			sprintf(buf,
					"[1;31m �x[33m���Y���˳� :[37m %-10s [33m�̥k��˳� :[37m %-10s [33m�̥���˳� :[37m %-10s [31m�x[m\n",
					weaponhead[chicken.weaponhead], weaponrhand[chicken.weaponrhand], weaponlhand[chicken.weaponlhand]);
			prints(buf);

			sprintf(buf,
					"[1;31m �x[33m�̨���˳� :[37m %-10s [33m�̸}���˳� :[37m %-10s [33m            [37m            [31m�x[m\n",
					weaponbody[chicken.weaponbody], weaponfoot[chicken.weaponfoot]);
			prints(buf);

			sprintf(buf,
					"[1;31m �u�t[41;37m ���Ÿ�� [0;1;31m�u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t[m\n");
			prints(buf);

			sprintf(buf,
					"[1;31m �x[33m�̵�    �� :[37m %-10d [33m�̸g �� �� :[37m %-10d [33m�̤U���ɯ� :[37m %-10d [31m�x[m\n",
					chicken.level, chicken.exp, twice(d.level, 10000, 100));
			prints(buf);

			sprintf(buf,
					"[1;31m ���w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w��[m\n");
			prints(buf);

			move(b_lines - 1, 0);
			sprintf(buf,
					"                                                              [1;36m�ĤG��[37m/[36m�@�G��[m\n");
			prints(buf);
			break;
		}
		move(b_lines, 0);
		sprintf(buf, "[1;44;37m  ��ƿ��  [46m  [��/PAGE UP]���W�@�� [��/PAGE DOWN]���U�@�� [Q]���}:            [m");
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
/* �԰��S��                                                                  */
/*                                                                           */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* �԰��H���M�w�禡                                                          */
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
	outs("\033[1;44;37m �ϰ� \033[46m[1]�����}�]  [2]�_��B��  [3]�j�N���  [4]�H�u�q  [5]�a������            \033[m\n");
	outs("\033[1;44;37m �ϰ� \033[46m                                                                  [Q]�^�a\033[m");
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
		outs("[1;44;37m ��V [46m[R]�^�a [F]���� (E/W/S/N)�F��n�_                                        \033[m");
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
			vmsg("�J��j�]���աI");
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
			vmsg("�S�o�ͥ���ơI");
	}
	return;
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
/* �԰��԰��禡                                                              */
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
	int dinjure = 0;		/*�p���ˮ`�O*/
	int minjure = 0;		/*���ˮ`�O*/
	int dresistmore = 0;	/*�p���[�j���m*/
	int mresistmore = 0;	/*���[�j���m*/
	int oldhexp;		/*���԰��e�氫�g��*/
	int oldmexp;		/*���԰��e�]�k�g��*/
	int oldbrave;		/*���԰��e�i��*/
	int oldhskill;		/*���԰��e�԰��޳N*/
	int oldmskill;		/*���԰��e�]�k�޳N*/
	int oldetchics;	/*���԰��e�D�w*/
	int oldmoney;		/*���԰��e����*/
	int oldtired;
	int oldhp;
	int oldexp;
	int winorlose = 0;		/*1:you win 0:you loss*/

	/*�H�����ͤH�� �åB�s�n�԰��e���@�Ǽƭ�*/
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
		if (m.hp <= 0) /*�ĤH�����F*/
		{
			m.hp = 0;
			d.money += m.money;
			m.death = 1;
			d.brave += rand() % 4 + 3;
		}
		if (d.hp <= 0 || d.tired >= 100)  /*�p���}�`*/
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
		/*vs_head("�q�l�i�p��", BoardName);*/
		move(0, 0);
		if (d.sex == 1)
			sprintf(buf, "[1;41m  " NICKNAME PIPNAME " �� [32m�� [37m%-10s                                                [0m", d.name);
		else if (d.sex == 2)
			sprintf(buf, "[1;41m  " NICKNAME PIPNAME " �� [33m�� [37m%-10s                                                [0m", d.name);
		else
			sprintf(buf, "[1;41m  " NICKNAME PIPNAME " �� [34m�H [37m%-10s                                                [0m", d.name);
		prints(buf);
		move(6, 0);
		if (mode == 1)
			show_badman_pic(m.map/*n*/);
		move(1, 0);
		sprintf(buf, "[1;31m�z�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�{[m");
		prints(buf);
		move(2, 0);
		/* lucky���ӷ�color��*/
		if (d.tired >= 80)
			lucky = 31;
		else if (d.tired >= 60 && d.tired < 80)
			lucky = 33;
		else
			lucky = 37;
		sprintf(inbuf1, "%d%s/%d%s", d.hp > 1000 ? d.hp / 1000 : d.hp, d.hp > 1000 ? "K" : "", d.maxhp > 1000 ? d.maxhp / 1000 : d.maxhp, d.maxhp > 1000 ? "K" : "");
		sprintf(inbuf2, "%d%s/%d%s", d.mp > 1000 ? d.mp / 1000 : d.mp, d.mp > 1000 ? "K" : "", d.maxmp > 1000 ? d.maxmp / 1000 : d.maxmp, d.maxmp > 1000 ? "K" : "");

		sprintf(buf, "[1;31m�x[33m��  �R:[37m%-12s[33m�k  �O:[37m%-12s[33m�h  ��:[%dm%-12d[33m��  ��:[37m%-10d[31m�x[m",
				inbuf1, inbuf2, lucky, d.tired, d.money);
		prints(buf);
		move(3, 0);
		sprintf(buf, "[1;31m�x[33m��  ��:[37m%-10d  [33m��  �m:[37m%-10d  [33m�t  ��:[37m%-10d  [33m�g  ��:[37m%-10d[31m�x[m",
				d.attack, d.resist, d.speed, d.exp);
		prints(buf);
		move(4, 0);
		sprintf(buf, "[1;31m�x[33m��  ��:[37m%-5d       [33m�j�ɤY:[37m%-5d       [33m�s  ��:[37m%-5d       [33m�F  ��:[37m%-5d     [31m�x[m",
				d.food, d.bighp, d.cookie, d.medicine);
		prints(buf);
		move(5, 0);
		sprintf(buf, "[1;31m�|�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�}[m");
		prints(buf);
		move(19, 0);
		sprintf(buf, "[1;34m�z�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�{[m");
		prints(buf);
		move(20, 0);
		sprintf(inbuf1, "%d%s/%d%s", m.hp > 1000 ? m.hp / 1000 : m.hp, m.hp > 1000 ? "K" : "", m.maxhp > 1000 ? m.maxhp / 1000 : m.maxhp, m.maxhp > 1000 ? "K" : "");
		sprintf(inbuf2, "%d%s/%d%s", m.mp > 1000 ? m.mp / 1000 : m.mp, m.mp > 1000 ? "K" : "", m.maxmp > 1000 ? m.maxmp / 1000 : m.maxmp, m.maxmp > 1000 ? "K" : "");

		sprintf(buf, "[1;34m�x[32m�m  �W:[37m%-10s  [32m��  �R:[37m%-11s [32m�k  �O:[37m%-11s                  [34m�x[m",
				p[n].name, inbuf1, inbuf2);
		prints(buf);
		move(21, 0);
		sprintf(buf, "[1;34m�x[32m��  ��:[37m%-6d      [32m��  �m:[37m%-6d      [32m�t  ��:[37m%-6d      [32m��  ��:[37m%-6d    [34m�x[m",
				m.attack, m.resist, m.speed, m.money);
		prints(buf);
		move(22, 0);
		sprintf(buf, "[1;34m�|�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�}[m");
		prints(buf);
		move(b_lines, 0);
		sprintf(buf, "[1;44;37m  �԰��R�O  [46m  [1]���q  [2]���O  [3]�]�k  [4]���m  [5]�ɥR  [6]�k�R         [m");
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
					vmsg("���M�S����..:~~~");
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
					sprintf(buf, "���q����,���ͩR�O��C%d", dinjure);
					vmsg(buf);
				}
				d.tired += rand() % (n + 1) / 15 + 2;
				break;

			case '2':
				show_fight_pic(2);
				if (rand() % 11 == 0)
				{
					vmsg("���M�S����..:~~~");
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
						sprintf(buf, "���O����,���ͩR�O��C%d", dinjure);
						vmsg(buf);
					}
					else
					{
						d.nodone = 1;
						vmsg("�A��HP�p��5��..�����...");
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
						sprintf(buf, "�v����,�ͩR�O����%d �h�ҭ��C%d", oldhp, oldtired);
						vmsg(buf);
					}
					else
					{
						if (rand() % 15 == 0)
							vmsg("���M�S����..:~~~");
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
							sprintf(buf, "�]�k����,���ͩR�O��C%d", dinjure);
							vmsg(buf);
						}
					}
				}
				break;
			case '4':
				dresistmore = 1;
				d.tired += rand() % (n + 1) / 20 + 1;
				vmsg("�p���[�j���m��....");
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
				vs_head("�q�l�i�p��", BoardName);
				move(10, 0);
				prints("            [1;31m�z�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�{[m\n");
				prints("            [1;31m�x  [37m��O���j���p�� [33m%-10s                 [31m�x[m\n", d.name);
				prints("            [1;31m�x  [37m�b�P��� [32m%-10s [37m�԰��Ḩ�]��          [31m�x[m\n", p[n].name);
				sprintf(inbuf1, "%d/%d", d.hexp - oldhexp, d.mexp - oldmexp);
				prints("            [1;31m�x  [37m�����W�[�F [36m%-5s [37m�I  �޳N�W�[�F [36m%-2d/%-2d [37m�I  [31m�x[m\n", inbuf1, d.hskill - oldhskill, d.mskill - oldmskill);
				sprintf(inbuf1, "%d [37m��", oldmoney - d.money);
				prints("            [1;31m�x  [37m�i�����C�F [36m%-5d [37m�I  ������֤F [36m%-13s  [31m�x[m\n", oldbrave - d.brave, inbuf1);
				prints("            [1;31m�|�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�}[m");
				vmsg("�T�Q���p �����W��...");
				winorlose = 0;
				break;
			}
		}
		clear();
		move(0, 0);
		if (d.sex == 1)
			sprintf(buf, "[1;41m  " NICKNAME PIPNAME " �� [32m�� [37m%-10s                                                  [0m", d.name);
		else if (d.sex == 2)
			sprintf(buf, "[1;41m  " NICKNAME PIPNAME " �� [33m�� [37m%-10s                                                  [0m", d.name);
		else
			sprintf(buf, "[1;41m  " NICKNAME PIPNAME " �� [34m�H [37m%-10s                                                  [0m", d.name);
		prints(buf);
		move(1, 0);
		sprintf(buf, "[1;31m�z�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�{[m");
		prints(buf);
		move(2, 0);
		/* lucky���ӷ�color��*/
		if (d.tired >= 80)
			lucky = 31;
		else if (d.tired >= 60 && d.tired < 80)
			lucky = 33;
		else
			lucky = 37;

		sprintf(inbuf1, "%d%s/%d%s", d.hp > 1000 ? d.hp / 1000 : d.hp, d.hp > 1000 ? "K" : "", d.maxhp > 1000 ? d.maxhp / 1000 : d.maxhp, d.maxhp > 1000 ? "K" : "");
		sprintf(inbuf2, "%d%s/%d%s", d.mp > 1000 ? d.mp / 1000 : d.mp, d.mp > 1000 ? "K" : "", d.maxmp > 1000 ? d.maxmp / 1000 : d.maxmp, d.maxmp > 1000 ? "K" : "");

		sprintf(buf, "[1;31m�x[33m��  �R:[37m%-12s[33m�k  �O:[37m%-12s[33m�h  ��:[%dm%-12d[33m��  ��:[37m%-10d[31m�x[m",
				inbuf1, inbuf2, lucky, d.tired, d.money);
		prints(buf);

		move(3, 0);
		sprintf(buf, "[1;31m�x[33m��  ��:[37m%-10d  [33m��  �m:[37m%-10d  [33m�t  ��:[37m%-10d  [33m�g  ��:[37m%-10d[31m�x[m",
				d.attack, d.resist, d.speed, d.exp);
		prints(buf);
		move(4, 0);
		sprintf(buf, "[1;31m�x[33m��  ��:[37m%-5d       [33m�j�ɤY:[37m%-5d       [33m�s  ��:[37m%-5d       [33m�F  ��:[37m%-5d     [31m�x[m",
				d.food, d.bighp, d.cookie, d.medicine);
		prints(buf);
		move(5, 0);
		sprintf(buf, "[1;31m�|�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�}[m");
		prints(buf);
		move(6, 0);
		if (mode == 1)
			show_badman_pic(m.map/*n*/);
		move(19, 0);
		sprintf(buf, "[1;34m�z�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�{[m");
		prints(buf);
		move(20, 0);
		sprintf(inbuf1, "%d/%d", m.hp, m.maxhp);
		sprintf(inbuf2, "%d/%d", m.mp, m.maxmp);
		sprintf(buf, "[1;34m�x[32m�m  �W:[37m%-10s  [32m��  �R:[37m%-11s [32m�k  �O:[37m%-11s                  [34m�x[m",
				p[n].name, inbuf1, inbuf2);
		prints(buf);
		move(21, 0);
		sprintf(buf, "[1;34m�x[32m��  ��:[37m%-6d      [32m��  �m:[37m%-6d      [32m�t  ��:[37m%-6d      [32m��  ��:[37m%-6d    [34m�x[m",
				m.attack, m.resist, m.speed, m.money);
		prints(buf);
		move(22, 0);
		sprintf(buf, "[1;34m�|�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�}[m");
		prints(buf);
		move(b_lines, 0);
		sprintf(buf, "[1;41;37m  [37m�����R�O  [47m  [31m[1][30m���q  [31m[2][30m���O  [31m[3][30m�]�k  [31m[4][30m���m  [31m[5][30m�k�R                     [m");
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
					vmsg("���S����..:~~~");
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
					sprintf(buf, "��贶�q����,�ͩR�O��C%d", minjure);
					vmsg(buf);
				}
				break;

			case 2:
				if (rand() % 11 == 10)
				{
					vmsg("���S����..:~~~");
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
						sprintf(buf, "�����O����, �ͩR�O��C%d", minjure);
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
						sprintf(buf, "��贶�q����,�ͩR�O��C%d", minjure);
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
								sprintf(inbuf1, "�����]");
							else
								sprintf(inbuf1, "�H��");
						}
						else if (m.mp < (m.maxmp / 2) && m.mp >= (m.maxmp / 4))
						{
							minjure = m.maxmp / 5;
							m.mp -= (300 + rand() % 200);
							if (rand() % 2)
								sprintf(inbuf1, "�g����");
							else
								sprintf(inbuf1, "��g��");
						}
						else if (m.mp < (m.maxmp / 4) && m.mp >= (m.maxmp / 6))
						{
							minjure = m.maxmp / 6;
							m.mp -= (100 + rand() % 100);
							if (rand() % 2)
								sprintf(inbuf1, "�g��t");
							else
								sprintf(inbuf1, "�۩�");
						}
						else if (m.mp < (m.maxmp / 6) && m.mp >= 0)
						{
							minjure = m.maxmp / 8;
							m.mp -= 50;
							if (rand() % 2)
								sprintf(inbuf1, "�����");
							else
								sprintf(inbuf1, "����");
						}
						minjure = minjure - d.resist / 50 - d.mresist / 10 - d.mskill / 200 - d.mexp / 200 + rand() % 10;
						if (minjure < 0)
							minjure = 15;
						d.hp -= minjure;
						if (m.mp < 0) m.mp = 0;
						d.mresist += rand() % 2 + 1;
						sprintf(buf, "���۴��F%s,�A���ˤF%d�I", inbuf1, minjure);
						vmsg(buf);
					}
					else
					{
						m.mp -= 20;
						m.hp += (m.maxmp / 6) + rand() % 20;
						if (m.hp > m.maxhp)
							m.hp = m.maxhp;
						vmsg("���ϥ��]�k�v���F�ۤv...");
					}
				}
				else
				{
					mresistmore = 1;
					vmsg("���[�j���m....");
				}
				break;

			case 4:
				d.money += (m.money + m.money / 2) / 3 + rand() % 10;
				d.hskill += rand() % 4 + 3;
				d.brave += rand() % 3 + 2;
				m.death = 1;
				sprintf(buf, "�����{�F..�����F�@�ǿ����A...");
				vmsg(buf);
				break;
			}
		}

		if (m.death == 1)
		{
			clear();
			oldexp = ((d.hexp - oldhexp) + (d.mexp - oldmexp) + rand() % 10) * (d.level + 1) + rand() % (d.level + 1);
			d.exp += oldexp;
			vs_head("�q�l�i�p��", BoardName);
			if (mode == 1)
			{
				move(10, 0);
				prints("            [1;31m�z�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�{[m\n");
				prints("            [1;31m�x  [37m�^�i���p�� [33m%-10s                     [31m�x[m\n", d.name);
				prints("            [1;31m�x  [37m���ѤF���c���Ǫ� [32m%-10s               [31m�x[m\n", p[n].name);
			}
			else
			{
				move(10, 0);
				prints("            [1;31m�z�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�{[m\n");
				prints("            [1;31m�x  [37m�Z�N�j�|���p�� [33m%-10s                 [31m�x[m\n", d.name);
				prints("            [1;31m�x  [37m���ѤF�j�l����� [32m%-10s               [31m�x[m\n", p[n].name);
			}
			sprintf(inbuf1, "%d/%d", d.hexp - oldhexp, d.mexp - oldmexp);
			prints("            [1;31m�x  [37m�������ɤF %-5s �I  �޳N�W�[�F %-2d/%-2d �I  [31m�x[m\n", inbuf1, d.hskill - oldhskill, d.mskill - oldmskill);
			sprintf(inbuf1, "%d ��", d.money - oldmoney);
			prints("            [1;31m�x  [37m�i�����ɤF %-5d �I  �����W�[�F %-9s [31m�x[m\n", d.brave - oldbrave, inbuf1);
			prints("            [1;31m�x  [37m�g��ȼW�[�F %-6d �I  �ɯũ|�� %-6d �I[31m�x[m\n", oldexp, twice(d.level, 10000, 100) - d.exp);
			prints("            [1;31m�|�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�}[m\n");

			if (m.hp <= 0)
				vmsg("��覺���o..�ҥH�AĹ�o..");
			else if (m.hp > 0)
				vmsg("��踨�]�o..�ҥH��AĹ�o.....");
			winorlose = 1;
		}
		if (d.death == 1 && mode == 1)
		{
			clear();
			vs_head("�q�l�i�p��", BoardName);
			move(10, 0);
			prints("            [1;31m�z�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�{[m\n");
			prints("            [1;31m�x  [37m�i�����p�� [33m%-10s                     [31m�x[m\n", d.name);
			prints("            [1;31m�x  [37m�b�P [32m%-10s [37m���԰����A                [31m�x[m\n", p[n].name);
			prints("            [1;31m�x  [37m�����a�}�`�F�A�b���S�O�q�s..........      [31m�x[m\n");
			prints("            [1;31m�|�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�}[m\n");
			vmsg("�p���}�`�F....");
			pipdie("[1;31m�԰����Q�����F...[m  ", 1);
		}
		else if (d.death == 1 && mode == 2)
		{
			clear();
			vs_head("�q�l�i�p��", BoardName);
			move(10, 0);
			prints("            [1;31m�z�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�{\n[m");
			prints("            [1;31m�x  [37m�i�����p�� [33m%-10s                     [31m�x[m\n", d.name);
			prints("            [1;31m�x  [37m�b�P [32m%-10s [37m���԰����A                [31m�x[m\n", p[n].name);
			prints("            [1;31m�x  [37m�����a����F�A�O�̲{���S�O����.........   [31m�x[m\n");
			prints("            [1;31m�|�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�}[m\n");
			vmsg("�p������F....");
		}
	}
	while ((pipkey != '6') && (d.death != 1) && (m.death != 1) && (mankey != 8));
	pip_check_level();
	return winorlose;
}


/*---------------------------------------------------------------------------*/
/* �԰��]�k�禡                                                              */
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

/*�i�J�ϥ��]�k���*/
static int
pip_magic_menu(mode, opt)  /*�԰����k�N������*/
int mode;
UTMP *opt;
{
	char buf[256];
	int injure;		/*�ˮ`�O*/
	int pipkey;

	do
	{
		move(b_lines, 0);
		clrtoeol();
		move(b_lines, 0);
		if (mode)
		{
			sprintf(buf,
					"[1;44;37m  �]�k���  [46m  [1]�v�� [2]�p�t [3]�B�t [4]���t [5]�g�t [6]���t [7]�S�� [Q]���: [m");
		}
		else
		{
			sprintf(buf,
					"[1;44;37m  �]�k���  [46m  [1]�v�� [2]�p�t [3]�B�t [4]���t [5]�g�t [6]���t [Q]���: [m");
		}
		move(b_lines, 0);
		prints(buf);
		pipkey = vkey();
		switch (pipkey)
		{
		case '1':  /*�v���k�N*/
			d.magicmode = 1;
			injure = pip_magic_doing_menu(treatmagiclist);
			break;

		case '2':  /*�p�t�k�N*/
			d.magicmode = 2;
			injure = pip_magic_doing_menu(thundermagiclist);
			break;

		case '3': /*�B�t�k�N*/
			d.magicmode = 3;
			injure = pip_magic_doing_menu(icemagiclist);
			break;

		case '4': /*���t�k�N*/
			d.magicmode = 4;
			injure = pip_magic_doing_menu(firemagiclist);
			show_fight_pic(341);
			vmsg("�p���ϥΤF���t�k�N");
			break;

		case '5': /*�g�t�k�N*/
			d.magicmode = 5;
			injure = pip_magic_doing_menu(earthmagiclist);
			break;

		case '6': /*���t�k�N*/
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

/*�]�k����*/
static int
pip_magic_doing_menu(p)   /*�]�k�e��*/
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
	sprintf(buf, "[1;31m�t[37;41m   �i��[%s]�@����   [0;1;31m�u�w�w�w�w�w�w�w�w�w�w�w�w[m", p[0].name);
	prints(buf);
	while ((s = p[n].name) && (p[n].needmp <= d.mp))
	{
		move(7 + n, 4);
		if (p[n].hpmode == 1)
		{
			sprintf(buf,
					"[1;37m[[36m%d[37m] [33m%-12s  [37m�ݭn�k�O: [32m%-6d  [37m��_��O: [32m%-6d [37m��_�h��: [32m%-6d[m   ", n, p[n].name, p[n].needmp, p[n].hp, p[n].tired);
			prints(buf);
		}
		else if (p[n].hpmode == 2)
		{
			sprintf(buf,
					"[1;37m[[36m%d[37m] [33m%-12s  [37m�ݭn�k�O: [32m%-6d  [37m��_��O��[35m�̤j��[37m ��_�h�Ҩ�[35m�̤p��[m  ", n, p[n].name, p[n].needmp);
			prints(buf);
		}
		else if (p[n].hpmode == 0)
		{
			sprintf(buf,
					"[1;37m[[36m%d[37m] [33m%-12s  [37m�ݭn�k�O: [32m%-6d [m             ", n, p[n].name, p[n].needmp);
			prints(buf);
		}
		n++;
	}
	n -= 1;

	do
	{
		move(16, 4);
		sprintf(buf, "�A�Q�ϥΨ��@��%8s�O?  [Q]���: ", p[0].name);
		getdata(16, 4, buf, ans, 2, 1, 0);
		if (ans[0] != 'q' && ans[0] != 'Q')
		{
			pipkey = atoi(ans);
		}
	}
	while (ans[0] != 'q' && ans[0] != 'Q' && (pipkey > n || pipkey <= 0));

	if (ans[0] != 'q' && ans[0] != 'Q')
	{
		getdata(17, 4, "�T�w�ϥζ�? [Y/n]: ", ans, 2, 1, 0);
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
pip_magic_fight_menu(p, opt)  /*�]�k�e��*/
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
	sprintf(buf, "[1;31m�t[37;41m   �i��[%s]�@����   [0;1;31m�u�w�w�w�w�w�w�w�w�w�w�w�w[m", s->name);
	prints(buf);
	s++;
	while (s->name)
	{
		move(7 + n, 4);
		if ((d.specialmagic & s->map) && (s->needmp <= d.mp))
		{
			sprintf(buf,
					"[1;37m[[36m%d[37m] [33m%-12s  [37m�ݭn�k�O: [32m%-6d [m             ", n, s->name, s->needmp);
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
		sprintf(buf, "�A�Q�ϥΨ��@��%8s�O?  [Q]���: ", p[0].name);
		getdata(16, 4, buf, ans, 2, 1, 0);
		if (ans[0] != 'q' && ans[0] != 'Q')
		{
			pipkey = atoi(ans);
		}
	}
	while (ans[0] != 'q' && ans[0] != 'Q' && (pipkey > n || pipkey <= 0));

	if (ans[0] != 'q' && ans[0] != 'Q')
	{
		getdata(17, 4, "�T�w�ϥζ�? [Y/n]: ", ans, 2, 1, 0);
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
/* �禡�S��                                                                  */
/*                                                                           */
/*---------------------------------------------------------------------------*/

/*�D�B*/
int
pip_marriage_offer()
{
	time_t now;
	char buf[256];
	char ans[4];
	int money;
	int who;
	char *name[5][2] = {{"�k�ӤH��", "�ӤH��"},
		{"�k�ӤH��", "�ӤH��"},
		{"�k�ӤH��", "�ӤH��"},
		{"�k�ӤH��", "�ӤH��"},
		{"�k�ӤH��", "�ӤH��"}
	};
	do
	{
		who = rand() % 5;
	}
	while (d.lover == (who + 3));

	money = rand() % 2000 + rand() % 3000 + 4000;
	sprintf(buf, "%s�a�ӤF����%d�A�n�V�A���p���D�B�A�z�@�N�ܡH[y/N]: ", name[who][d.sex-1], money);
	getdata(b_lines - 1, 1, buf, ans, 2, 1, 0);
	if (ans[0] == 'y' || ans[0] == 'Y')
	{
		if (d.wantend != 1 && d.wantend != 4)
		{
			sprintf(buf, "���㤧�e�w�g���B���F�A�z�T�w�n�Ѱ��±B���A��w�߱B���ܡH[y/N]: ");
			getdata(b_lines - 1, 1, buf, ans, 2, 1, 0);
			if (ans[0] != 'y' && ans[0] != 'Y')
			{
				d.social += 10;
				vmsg("�٬O�����±B���n�F..");
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
		vmsg("�ڷQ���O�@�ӫܦn����Q..");
		now = time(0);
		sprintf(buf, "[1;37m%s %-11s���p�� [%s] �����F %s ���D�B[0m\n", Cdate(&now), cuser.userid, d.name, name[who][d.sex-1]);
		pip_log_record(buf);
	}
	else
	{
		d.charm += rand() % 5 + 20;
		d.relation += 20;
		if (d.wantend == 1 || d.wantend == 4)
			vmsg("���٦~��  �߱��٤��w...");
		else
			vmsg("�ڦ��w���B���F..�藍�_...");
	}
	d.money += money;
	return 0;
}

int pip_results_show()  /*��ì�u*/
{
	char *showname[5] = {"  ", "�Z���j�|", "���N�j�i", "�Ӯa�R�|", "�i���j��"};
	char buf[256];
	int pipkey, i = 0;
	int winorlost = 0;
	int a, b[3][2], c[3] = {0, 0, 0};

	clear();
	move(10, 14);
	prints("[1;33m�m�N�m�N�� ���W���l�t���ڭ̰e�H�ӤF��...[0m");
	vmsg("��  ��H���}�ݬݧa...");
	clear();
	show_resultshow_pic(0);
	sprintf(buf, "[A]%s [B]%s [C]%s [D]%s [Q]���:", showname[1], showname[2], showname[3], showname[4]);
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
		vmsg("���~�@���|�H���ɡ�{�b���ɶ}�l");
		for (i = 0;i < 3;i++)
		{
			a = 0;
			b[i][1] = 0;
			sprintf(buf, "�A����%d�ӹ��O%s", i + 1, resultmanlist[b[i][0]].name);
			vmsg(buf);
			a = pip_vs_man(b[i][0], resultmanlist, 2);
			if (a == 1)
				b[i][1] = 1;/*����F*/
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
		vmsg("���~�@���|�H���ɡ�{�b���ɶ}�l");
		show_resultshow_pic(21);
		vmsg("���ɱ���");
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
		vmsg("���~�@���|�H���ɡ�{�b���ɶ}�l");
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
		vmsg("���~�@���|�H���ɡ�{�b���ɶ}�l");
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
		vmsg("���~���ѥ[��.....:(");
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
	char *gamename[5] = {"  ", "�Z���j�|", "���N�j�i", "�Ӯa�R�|", "�i���j��"};
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
	prints("[1;37m���� [32m���� %s ���G���� [37m����[0m", gamename[mode]);
	move(8, 15);
	prints("[1;41m �a�x [0;1m�� [1;33m%-10s[36m  ���� %d[0m", name1, resultmoney[3]);
	move(10, 15);
	prints("[1;41m �ȭx [0;1m�� [1;33m%-10s[36m  ���� %d[0m", name2, resultmoney[2]);
	move(12, 15);
	prints("[1;41m �u�x [0;1m�� [1;33m%-10s[36m  ���� %d[0m", name3, resultmoney[1]);
	move(14, 15);
	prints("[1;41m �̫� [0;1m�� [1;33m%-10s[36m [0m", name4);
	sprintf(buf, "���~��%s�����o ��~�A�ӧa..", gamename[mode]);
	d.money += resultmoney[winorlost];
	vmsg(buf);
	return 0;
}

/*---------------------------------------------------------------------------*/
/* �G�N���禡�S��                                                            */
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
	char yo[12][5] = {"�ϥ�", "����", "����", "�ൣ", "�֦~", "�C�~",
					  "���~", "���~", "��~", "�Ѧ~", "�j�}", "���P"
					 };

	color1 = color2 = color3 = color4 = 37;
	move(1, 0);
	m = (time(0) - start_time + d.bbtime) / 60 / 30; /* �@�� */
	/*���j�@���ɪ��W�[���ܭ�*/
	color = 37;

	if (m == 0) /*�ϥ�*/
		age = 0;
	else if (m == 1) /*����*/
		age = 1;
	else if (m >= 2 && m <= 5) /*����*/
		age = 2;
	else if (m >= 6 && m <= 12) /*�ൣ*/
		age = 3;
	else if (m >= 13 && m <= 15) /*�֦~*/
		age = 4;
	else if (m >= 16 && m <= 18) /*�C�~*/
		age = 5;
	else if (m >= 19 && m <= 35) /*���~*/
		age = 6;
	else if (m >= 36 && m <= 45) /*���~*/
		age = 7;
	else if (m >= 45 && m <= 60) /*��~*/
		age = 8;
	else if (m >= 60 && m <= 70) /*�Ѧ~*/
		age = 9;
	else if (m >= 70 && m <= 100) /*�j�}*/
		age = 10;
	else if (m > 100) /*���P*/
		age = 11;
	clear();
	move(0, 0);
	if (d.sex == 1)
		sprintf(buf, "[1;41m  " NICKNAME PIPNAME " �� [32m�� [37m%-15s                                           [0m", d.name);
	else if (d.sex == 2)
		sprintf(buf, "[1;41m  " NICKNAME PIPNAME " �� [33m�� [37m%-15s                                           [0m", d.name);
	else
		sprintf(buf, "[1;41m  " NICKNAME PIPNAME " �� [34m�H [37m%-15s                                           [0m", d.name);
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
			, " [1;32m[��  �A][37m %-5s     [32m[��  ��][37m %-9s [32m[�~  ��][37m %-5d     [32m[��  ��][%dm %-8d [m"
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
			, " [1;32m[��  �R][%dm %-10s[32m[�k  �O][%dm %-10s[32m[��  ��][37m %-5d     [32m[�h  ��][%dm %-4d[0m "
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
			, " [1;32m[ż  ż][%dm %-4d      [32m[�f  ��][%dm %-4d      [32m[�ּ֫�][%dm %-4d      [32m[���N��][%dm %-4d[0m"
			, color1, d.shit, color2, d.sick, color3, d.happy, color4, d.satisfy);
	prints(buf);
	if (mode == 1)/*����*/
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
				, " [1;36m[����][%dm%-7d[36m[�s��][%dm%-7d[36m[�ɤY][%dm%-7d[36m[�F��][%dm%-7d[36m[�H��][37m%-7d[36m[����][37m%-7d[0m"
				, color1, d.food, color2, d.cookie, color3, d.bighp, color4, d.medicine, d.ginseng, d.snowgrass);
		prints(buf);

	}
	move(5, 0);
	prints("[1;%dm�z�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�{[m", color);
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
	prints("[1;%dm�|�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�}[m", color);
	move(19, 0);
	prints(" [1;34m�w[37;44m  �� �A  [0;1;34m�w[0m");
	move(20, 0);
	prints(" �԰���.............\n");

}
#endif
#ifdef	HAVE_PIP_FIGHT
static int pip_fight_feed()     /* ����*/
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
		sprintf(buf, "%s�Ӱ�����ƩO?", d.name);
		prints(buf);
		now = time(0);
		move(b_lines, 0);
		clrtoeol();
		move(b_lines, 0);
		prints("[1;44;37m  �������  [46m[1]�Y�� [2]�s�� [3]�ɤY [4]�F�� [5]�H�x [6]���� [Q]���X�G         [m");
		pipkey = vkey();

		switch (pipkey)
		{
		case '1':
			if (d.food <= 0)
			{
				move(b_lines, 0);
				vmsg("�S�������o..�֥h�R�a�I");
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
			vmsg("�C�Y�@�������|��_��O50��!");
			break;

		case '2':
			if (d.cookie <= 0)
			{
				move(b_lines, 0);
				vmsg("�s���Y���o..�֥h�R�a�I");
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
			vmsg("�Y�s���e���D��...");
			break;

		case '3':
			if (d.bighp <= 0)
			{
				move(b_lines, 0);
				vmsg("�S���j�ɤY�F�C! �ֶR�a..");
				break;
			}
			d.bighp--;
			d.hp += 600;
			d.tired -= 20;
			d.weight += rand() % 2;
			move(4, 0);
			show_feed_pic(4);
			d.nodone = 0;
			vmsg("�ɤY..�W���Ϊ���...");
			break;

		case '4':
			if (d.medicine <= 0)
			{
				move(b_lines, 0);
				vmsg("�S���F���o..�֥h�R�a�I");
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
			vmsg("�C�Y�@���F�۷|��_�k�O50��!");
			break;

		case '5':
			if (d.ginseng <= 0)
			{
				move(b_lines, 0);
				vmsg("�S���d�~�H�x�C! �ֶR�a..");
				break;
			}
			d.ginseng--;
			d.mp += 500;
			d.tired -= 20;
			move(4, 0);
			show_feed_pic(1);
			d.nodone = 0;
			vmsg("�d�~�H�x..�W���Ϊ���...");
			break;

		case '6':
			if (d.snowgrass <= 0)
			{
				move(b_lines, 0);
				vmsg("�S���Ѥs�����C! �ֶR�a..");
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
			vmsg("�Ѥs����..�W���Ϊ���...");
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
