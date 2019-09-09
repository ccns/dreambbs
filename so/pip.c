/* ----------------------------------- */
/* pip.c  養小雞程式                   */
/* 原作者: dsyan   改寫者: fennet      */
/* 圖圖 by tiball.bbs@bbs.nhctc.edu.tw */
/* ----------------------------------- */
#define ba 5
#define START_MONEY     (3000)
#define START_FOOD      (20)
#define START_HP        (50)
#define START_HAPPY     (20)
#define START_SATISFY   (20)

#define LEARN_LEVEL     ((d.happy+d.satisfy)/100)

#include "bbs.h"
#include "pipstruct.h"
#include <time.h>
#include "pip.h"

#define PIPNAME         "寵物雞"

static struct chicken d;
static time_t start_time;
static time_t lasttime;

#define getdata(x1, x2, x3, x4, x5, x6, x7)  vget(x1, x2, x3, x4, x5, DOECHO)

static int KEY_ESC_arg;
static const char *const BoardName = currboard;
#ifdef  HAVE_PIP_FIGHT
static UTMP *currutmp;
#endif
static char Bdate[128];
static char pippath[128];
static levelup ml[2];

#include "pipfun.c"

static int pip_money(void);
static void pip_load_mob(const char *fpath);
static void pip_load_mobset(const char *fpath);
static void pip_check_levelup(void);
static void pip_check_level(void);
static void pip_load_levelup(const char *fpath);
static int twice(int x, int max, int min);
static int pip_magic_menu(int mode, const UTMP *opt);
static int pip_magic_doing_menu(const struct magicset *p);
static int pip_data_list(const char *userid);
static void pip_read_file(const char *userid);
static void pip_write_file(void);
static void show_system_pic(int i);
static void pip_new_game(void);
static int pip_main_menu(void);
static int pip_live_again(void);
static void show_basic_pic(int i);
static void pip_log_record(const char *msg);
static void show_die_pic(int i);
static int pip_mainmenu(int mode);
static void pip_time_change(time_t cnow);
static int pip_go_palace_screen(const struct royalset *p);
static int pip_ending_screen(void);
static int pip_marriage_offer(void);
static void show_usual_pic(int i);
static void show_feed_pic(int i);
static int pip_buy_goods_new(int mode, const struct goodsofpip *p, int oldnum[]);
static int pip_weapon_doing_menu(int variance, int type, const struct weapon *p);
static int pip_vs_man(int n, const struct playrule *p, int mode);
static int pip_results_show_ending(int winorlost, int mode, int a, int b, int c);
static void pip_fight_bad(int n);
static void tie(void);
static void win(void);
static void situ(void);
static void lose(void);
static int pip_practice_function(int classnum, int classgrade, int pic1, int pic2, int *change1, int *change2, int *change3, int *change4, int *change5);
static int pip_ending_decide(char *eendbuf1, char *eendbuf2, char *eendbuf3, int *endmode, int *endgrade);
static int pip_game_over(int endgrade);
static int pip_practice_gradeup(int classnum, int classgrade, int data);
static int pip_read(const char *userid);

/*系統選單*/
static int pip_data_list_cuser(void), pip_system_freepip(void), pip_system_service(void);
static int pip_write_backup(void), pip_read_backup(void);
static int pip_divine(void), pip_results_show(void);

#ifdef  HAVE_PIP_FIGHT
static int pip_magic_fight_menu(void);
static int get_hurt(void);
static int pip_fight_feed(void);

#endif

static void
logit(
int money)
{
    char buf[100];
    time_t now;
    now = time(NULL);
    if (money < 100000)
        return;
    if (money > 300000)
        sprintf(buf, "\x1b[1;31m%s %s : %d\x1b[m\n", Cdate(&now), cuser.userid, money);
    else
        sprintf(buf, "%s %s : %d\n", Cdate(&now), cuser.userid, money);
    f_cat(FN_PIPMONEY_LOG, buf);
}

static char *
get_path(
const char *id, const char *file)
{
    usr_fpath(pippath, id, file);
    return pippath;
}

/*遊戲主程式*/
int p_pipple(void)
{
    FILE *fs;
    int pipkey;
    char genbuf[200];

    utmp_mode(M_CHICKEN);
    more("game/pipgame/pip.welcome", NULL);
    vs_head("電子養小雞", BoardName);
    srandom(time(0));
    /* sprintf(genbuf, "home/%s/chicken", cuser.userid);*/
    usr_fpath(genbuf, cuser.userid, "chicken");
    pip_load_levelup("game/pipdata/piplevel.dat");
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
    /*pip_do_menu(0, 0, pipmainlist);*/
    if (d.death != 0 || !d.name[0])  return 0;
    pip_load_mob("game/pipdata/pipmob.dat");
    pip_load_mobset("game/pipdata/pipmobset.dat");
    pip_main_menu();
    free(badmanlist);
    badmanlist = NULL;
    d.bbtime += time(0) - start_time;
    pip_write_file();
    logit(d.money);
    return 0;
}

/*時間表示法*/
static char*
dsyan_time(const time_t *t)
{
    struct tm *tp;
    static char ans[9];

    tp = localtime(t);
    sprintf(ans, "%02d/%02d/%02d", (tp->tm_year) % 100, tp->tm_mon + 1, tp->tm_mday);
    return ans;
}

/*新遊戲的設定*/
static void pip_new_game(void)
{
    char buf[256];
    time_t now;
    const char *pipsex[3] = {"？", "♂", "♀"};
    struct tm *ptime;
    ptime = localtime(&now);

    if (d.death == 1 && !(!d.name[0]))
    {
        clear();
        vs_head(NICKNAME PIPNAME, BoardName);
        move(4, 6);
        outs("歡迎來到 \x1b[1;5;33m" NICKNAME "生物科技研究院\x1b[0m");
        move(6, 6);
        outs("經我們調查顯示  先前你有養過小雞喔  可是被你養死了...");
        move(8, 6);
        if (d.liveagain < 4)
        {
            outs("我們可以幫你幫小雞復活  但是需要付出一點代價");
            getdata(10, 6, "你要我們讓他重生嗎? [y/N]: ", buf, 2, DOECHO, 0);
            if (buf[0] == 'y' || buf[0] == 'Y')
            {
                pip_live_again();
            }
        }
        else if (d.liveagain >= 4)
        {
            outs("可是你復活手術太多次了  小雞身上都是開刀痕跡");
            move(10, 6);
            outs("我們找不到可以手術的地方了  所以....");
            vmsg("重新再來吧....唉....");
        }
    }
    if (d.death != 0 || !d.name[0])
    {
        clear();
        vs_head(NICKNAME PIPNAME, BoardName);
        /*小雞命名*/
        getdata(2, 0, "幫小雞取個好聽的名字吧(請不要有空格): ", buf, 11, DOECHO, 0);
        if (!buf[0])
            return;
        strcpy(d.name, buf);
        /*1:公 2:母 */
        getdata(4, 3, "[Boy]小公雞♂ or [Girl]小母雞♀ [b/G]: ", buf, 2, DOECHO, 0);
        if (buf[0] == 'b' || buf[0] == 'B')
        {
            d.sex = 1;
        }
        else
        {
            d.sex = 2;
        }
        move(6, 3);
        outs(NICKNAME PIPNAME "的遊戲現今分成兩種玩法");
        move(7, 3);
        outs("選有結局會在小雞20歲時結束遊戲，並告知小雞後續的發展");
        move(8, 3);
        outs("選沒有結局則一直養到小雞死亡才結束遊戲....");
        /*1:不要且未婚 4:要且未婚 */
        getdata(9, 3, "你希望小雞遊戲是否要有20歲結局? [Y/n]: ", buf, 2, DOECHO, 0);
        if (buf[0] == 'n' || buf[0] == 'N')
        {
            d.wantend = 1;
        }
        else
        {
            d.wantend = 4;
        }
        /*開頭畫面*/
        show_basic_pic(0);
        vmsg("小雞終於誕生了，請好好愛他....");

        /*開頭設定*/
        now = time(0);
        strcpy(d.birth, dsyan_time(&now));
        d.bbtime = 0;

        /*基本資料*/
        d.year = ptime->tm_year;
        d.month = ptime->tm_mon + 1;
        d.day = ptime->tm_mday;
        d.death = d.nodone = d.relation = 0;
        d.liveagain = d.level = d.exp = d.dataE = 0;
        d.chickenmode = 1;

        /*身體參數*/
        d.hp = random() % 15 + START_HP;
        d.maxhp = random() % 20 + START_HP;
        if (d.hp > d.maxhp) d.hp = d.maxhp;
        d.weight = random() % 10 + 50;
        d.tired = d.sick = d.shit = d.wrist = 0;
        d.bodyA = d.bodyB = d.bodyC = d.bodyD = d.bodyE = 0;

        /*評價參數*/
        d.social = d.family = d.hexp = d.mexp = 0;
        d.tmpA = d.tmpB = d.tmpC = d.tmpD = d.tmpE = 0;

        /*戰鬥參數*/
        d.mp = d.maxmp = d.attack = d.resist = d.speed = d.hskill = d.mskill = d.mresist = 0;
        d.magicmode = d.specialmagic = d.fightC = d.fightD = d.fightE = 0;

        /*武器參數*/
        d.weaponhead = d.weaponrhand = d.weaponlhand = d.weaponbody = d.weaponfoot = 0;
        d.weaponA = d.weaponB = d.weaponC = d.weaponD = d.weaponE = 0;

        /*能力參數*/
        d.toman = d.character = d.love = d.wisdom = d.art = d.ethics = 0;
        d.brave = d.homework = d.charm = d.manners = d.speech = d.cookskill = 0;
        d.learnA = d.learnB = d.learnC = d.learnD = d.learnE = 0;

        /*狀態數值*/
        d.happy = random() % 10 + START_HAPPY;
        d.satisfy = random() % 10 + START_SATISFY;
        d.fallinlove = d.belief = d.offense = d.affect = 0;
        d.stateA = d.stateB = d.stateC = d.stateD = d.stateE = 0;

        /*食物參數:食物 零食 藥品 大補丸*/
        d.food = START_FOOD;
        d.medicine = d.cookie = d.bighp = 2;
        d.ginseng = d.snowgrass = d.eatC = d.eatD = d.eatE = 0;

        /*物品參數:書 玩具*/
        d.book = d.playtool = 0;
        d.money = START_MONEY;
        d.thingA = d.thingB = d.thingC = d.thingD = d.thingE = 0;

        /*猜拳參數:贏 負*/
        d.winn = d.losee = 0;

        /*參見王臣*/
        d.royalA = d.royalB = d.royalC = d.royalD = d.royalE = 0;
        d.royalF = d.royalG = d.royalH = d.royalI = d.royalJ = 0;
        d.seeroyalJ = 1;
        d.seeA = d.seeB = d.seeC = d.seeD = d.seeE;
        /*接受求婚愛人*/
        d.lover = 0;
        /*0:沒有 1:魔王 2:龍族 3:A 4:B 5:C 6:D 7:E */
        d.classA = d.classB = d.classC = d.classD = d.classE = 0;
        d.classF = d.classG = d.classH = d.classI = d.classJ = 0;
        d.classK = d.classL = d.classM = d.classN = d.classO = 0;

        d.workA = d.workB = d.workC = d.workD = d.workE = 0;
        d.workF = d.workG = d.workH = d.workI = d.workJ = 0;
        d.workK = d.workL = d.workM = d.workN = d.workO = 0;
        d.workP = d.workQ = d.workR = d.workS = d.workT = 0;
        d.workU = d.workV = d.workW = d.workX = d.workY = d.workZ = 0;
        /*養雞記錄*/
        now = time(0);
        sprintf(buf, "\x1b[1;36m%s %-11s養了一隻叫 [%s] 的 %s 小雞 \x1b[0m\n", Cdate(&now), cuser.userid, d.name, pipsex[d.sex]);
        pip_log_record(buf);
    }
    pip_write_file();
}

/*小雞死亡函式*/
static void
pipdie(
const char *msg,
int mode)
{
    char genbuf[200];
    time_t now;
    clear();
    vs_head("電子養小雞", BoardName);
    if (mode == 1)
    {
        show_die_pic(1);
        vmsg("死神來帶走小雞了");
        clear();
        vs_head("電子養小雞", BoardName);
        show_die_pic(2);
        move(14, (d_cols>>1) + 20);
        prints("可憐的小雞\x1b[1;31m%s\x1b[m", msg);
        vmsg(NICKNAME "哀悼中....");
    }
    else if (mode == 2)
    {
        show_die_pic(3);
        vmsg("嗚嗚嗚..我被丟棄了.....");
    }
    else if (mode == 3)
    {
        show_die_pic(0);
        vmsg("遊戲結束囉..");
    }

    now = time(0);
    sprintf(genbuf, "\x1b[1;31m%s %-11s的小雞 [%s] %s\x1b[m\n", Cdate(&now), cuser.userid, d.name, msg);
    pip_log_record(genbuf);
    pip_write_file();
}


/*pro:機率 base:底數 mode:類型 mul:加權100=1 cal:加減*/
static void
count_tired(
int prob, int base,
const char *mode,
int mul,
int cal)
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
                tiredvary = (random() % prob + base) * d.maxhp / (d.hp + 0.8 * d.hp) * 120 / 100;
            else if (cal == 0)
                tiredvary = (random() % prob + base) * 4 / 3;
        }
        else if (tm >= 4 && tm <= 7)
        {
            if (cal == 1)
                tiredvary = (random() % prob + base) * d.maxhp / (d.hp + 0.8 * d.hp);
            else if (cal == 0)
                tiredvary = (random() % prob + base) * 3 / 2;
        }
        else if (tm >= 8 && tm <= 10)
        {
            if (cal == 1)
                tiredvary = (random() % prob + base) * d.maxhp / (d.hp + 0.8 * d.hp) * 110 / 100;
            else if (cal == 0)
                tiredvary = (random() % prob + base) * 5 / 4;
        }
        else if (tm >= 11)
        {
            if (cal == 1)
                tiredvary = (random() % prob + base) * d.maxhp / (d.hp + 0.8 * d.hp) * 150 / 100;
            else if (cal == 0)
                tiredvary = (random() % prob + base) * 1;
        }
    }
    else if (!strcmp(mode, "N"))
    {
        tiredvary = random() % prob + base;
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
            { d.tired = 0; }
    }
    tiredvary = 0;
    return;
}

/*---------------------------------------------------------------------------*/
/*主畫面和選單                                                               */
/*---------------------------------------------------------------------------*/

static const char *menuname[8][2] =
{
    {"             ",
     "\x1b[1;44;37m 選單 \x1b[46m[1]基本 [2]逛街 [3]修行 [4]玩樂 [5]打工 [6]特殊 [7]系統 [Q]離開         %*s\x1b[m"},

    {"             ",
     "\x1b[1;44;37m  基本選單  \x1b[46m[1]餵食 [2]清潔 [3]休息 [4]親親 [5]換錢         %*s[Q]跳出           \x1b[m"},

    {"\x1b[1;44;37m 逛街 \x1b[46m【日常用品】[1]便利商店 [2]" NICKNAME "藥鋪 [3]夜裡書局         %*s\x1b[m",
     "\x1b[1;44;37m 選單 \x1b[46m【武器百貨】[A]頭部裝備 [B]右手裝備 [C]左手裝備 [D]身體裝備 [E]腳部裝備 %*s\x1b[m"},

    {"\x1b[1;44;37m 修行 \x1b[46m[A]科學(%d) [B]詩詞(%d) [C]神學(%d) [D]軍學(%d) [E]劍術(%d)                  %*s\x1b[m",
     "\x1b[1;44;37m 選單 \x1b[46m[F]格鬥(%d) [G]魔法(%d) [H]禮儀(%d) [I]繪畫(%d) [J]舞蹈(%d) [Q]跳出          %*s\x1b[m"},

    {"   ",
     "\x1b[1;44;37m  玩樂選單  \x1b[46m[1]散步 [2]運動 [3]約會 [4]猜拳 [5]旅遊 [6]郊外 [7]唱歌 [Q]跳出   %*s\x1b[m"},

    {"\x1b[1;44;37m 打工 \x1b[46m[A]家事 [B]保姆 [C]旅館 [D]農場 [E]餐\廳 [F]教堂 [G]地攤 [H]伐木 [I]美髮 %*s\x1b[m",
     "\x1b[1;44;37m 選單 \x1b[46m[J]獵人 [K]工地 [L]守墓 [M]家教 [N]酒家 [O]酒店 [P]夜總會       %*s[Q]跳出 \x1b[m"},

    {"\x1b[1;44;37m 特殊 \x1b[46m[1]" NICKNAME "醫院 [2]媚登峰 [3]戰鬥修行 [4]拜訪朋友 [5]" NICKNAME "       %*s\x1b[m",
     "\x1b[1;44;37m 選單 \x1b[46m                                                                %*s[Q]跳出 \x1b[m"},

    {"\x1b[1;44;37m 系統 \x1b[46m[1]詳細資料 [2]小雞自由 [3]特別服務 [4]儲存進度 [5]讀取進度             %*s\x1b[m",
     "\x1b[1;44;37m 選單 \x1b[46m                                                                %*s[Q]跳出 \x1b[m"}
};

/*主選單*/
static int pip_basic_menu(void), pip_store_menu(void), pip_practice_menu(void);
static int pip_play_menu(void), pip_job_menu(void), pip_special_menu(void), pip_system_menu(void);

static struct pipcommands pipmainlist[] =
{
    {pip_basic_menu,            '1',    '1'},
    {pip_store_menu,            '2',    '2'},
    {pip_practice_menu,         '3',    '3'},
    {pip_play_menu,             '4',    '4'},
    {pip_job_menu,              '5',    '5'},
    {pip_special_menu,          '6',    '6'},
    {pip_system_menu,           '7',    '7'},
    {NULL,                     '\0',   '\0'}
};

/*基本選單*/
static int pip_basic_feed(void), pip_basic_takeshower(void), pip_basic_takerest(void), pip_basic_kiss(void);
static struct pipcommands pipbasiclist[] =
{
    {pip_basic_feed,            '1',    '1'},
    {pip_basic_takeshower,      '2',    '2'},
    {pip_basic_takerest,        '3',    '3'},
    {pip_basic_kiss,            '4',    '4'},
    {pip_money,                 '5',    '5'},
    {NULL,                     '\0',   '\0'}
};

/*商店選單*/
static int pip_store_food(void), pip_store_medicine(void), pip_store_other(void);
static int pip_store_weapon_head(void), pip_store_weapon_rhand(void), pip_store_weapon_lhand(void);
static int pip_store_weapon_body(void), pip_store_weapon_foot(void);

static struct pipcommands pipstorelist[] =
{
    {pip_store_food,            '1',    '1'},
    {pip_store_medicine,        '2',    '2'},
    {pip_store_other,           '3',    '3'},
    {pip_store_weapon_head,     'a',    'A'},
    {pip_store_weapon_rhand,    'b',    'B'},
    {pip_store_weapon_lhand,    'c',    'C'},
    {pip_store_weapon_body,     'd',    'D'},
    {pip_store_weapon_foot,     'e',    'E'},
    {NULL,                     '\0',   '\0'}
};

/*修行選單*/
static int pip_practice_classA(void), pip_practice_classB(void), pip_practice_classC(void);
static int pip_practice_classD(void), pip_practice_classE(void), pip_practice_classF(void);
static int pip_practice_classG(void), pip_practice_classH(void), pip_practice_classI(void);
static int pip_practice_classJ(void);

static struct pipcommands pippracticelist[] =
{
    {pip_practice_classA,       'a',    'A'},
    {pip_practice_classB,       'b',    'B'},
    {pip_practice_classC,       'c',    'C'},
    {pip_practice_classD,       'd',    'D'},
    {pip_practice_classE,       'e',    'E'},
    {pip_practice_classF,       'f',    'F'},
    {pip_practice_classG,       'g',    'G'},
    {pip_practice_classH,       'h',    'H'},
    {pip_practice_classI,       'i',    'I'},
    {pip_practice_classJ,       'j',    'J'},
    {NULL,                     '\0',   '\0'}
};

/*玩樂選單*/
static int pip_play_stroll(void), pip_play_sport(void), pip_play_date(void), pip_play_guess(void);
static int pip_play_outing(void), pip_play_kite(void), pip_play_KTV(void);

static struct pipcommands pipplaylist[] =
{
    {pip_play_stroll,           '1',    '1'},
    {pip_play_sport,            '2',    '2'},
    {pip_play_date,             '3',    '3'},
    {pip_play_guess,            '4',    '4'},
    {pip_play_outing,           '5',    '5'},
    {pip_play_kite,             '6',    '6'},
    {pip_play_KTV,              '7',    '7'},
    {NULL,                     '\0',   '\0'}
};

/*打工選單*/
static int pip_job_workA(void), pip_job_workB(void), pip_job_workC(void), pip_job_workD(void);
static int pip_job_workE(void), pip_job_workF(void), pip_job_workG(void), pip_job_workH(void);
static int pip_job_workI(void), pip_job_workJ(void), pip_job_workK(void), pip_job_workL(void);
static int pip_job_workM(void), pip_job_workN(void), pip_job_workO(void), pip_job_workP(void);
static struct pipcommands pipjoblist[] =
{
    {pip_job_workA,             'a',    'A'},
    {pip_job_workB,             'b',    'B'},
    {pip_job_workC,             'c',    'C'},
    {pip_job_workD,             'd',    'D'},
    {pip_job_workE,             'e',    'E'},
    {pip_job_workF,             'f',    'F'},
    {pip_job_workG,             'g',    'G'},
    {pip_job_workH,             'h',    'H'},
    {pip_job_workI,             'i',    'I'},
    {pip_job_workJ,             'j',    'J'},
    {pip_job_workK,             'k',    'K'},
    {pip_job_workL,             'l',    'L'},
    {pip_job_workM,             'm',    'M'},
    {pip_job_workN,             'n',    'N'},
    {pip_job_workO,             'o',    'O'},
    {pip_job_workP,             'p',    'P'},
    {NULL,                     '\0',   '\0'}
};

/*特殊選單*/
static int pip_see_doctor(void), pip_change_weight(void), pip_meet_vs_man(void), pip_query(void), pip_go_palace(void);
/* static int pip_vs_fight(void); */
static struct pipcommands pipspeciallist[] =
{
    {pip_see_doctor,            '1',    '1'},
    {pip_change_weight,         '2',    '2'},
    {pip_meet_vs_man,           '3',    '3'},
    {pip_query,                 '4',    '4'},
    {pip_go_palace,             '5',    '5'},
    /*{pip_vs_fight,              'z',    'z'}, */
    {NULL,                     '\0',   '\0'}
};

static struct pipcommands pipsystemlist[] =
{
    {pip_data_list_cuser,       '1',    '1'},
    {pip_system_freepip,        '2',    '2'},
    {pip_system_service,        '3',    '3'},
    {pip_write_backup,          '4',    '4'},
    {pip_read_backup,           '5',    '5'},
    /*
    {pip_divine,                'o',    'O'},
    {pip_results_show,          's',    'S'},
    */
    {NULL,                     '\0',   '\0'}
};



/*類似menu.c的功能*/
static int
pip_do_menu(
int menunum, int menumode,
const struct pipcommands cmdtable[])
{
    time_t now;
    int key1, key2;
    int pipkey;
    int goback = 0, ok = 0;
    int class1 = 0, class2 = 0, class3 = 0, class4 = 0, class5 = 0;
    int class6 = 0, class7 = 0, class8 = 0, class9 = 0, class10 = 0;
    const struct pipcommands *cmd1;
    const struct pipcommands *cmd2;

    do
    {
        ok = 0;
        /*判斷是否死亡  死掉即跳回上一層*/
        if (d.death == 1 || d.death == 2 || d.death == 3)
            return 0;
        /*經pip_mainmenu判定後是否死亡*/
        if (pip_mainmenu(menumode))
            return 0;

        clrchyiuan(b_lines - 1, b_lines);
        if (menunum == 3)                                           /*修行*/
        {
            class1 = d.wisdom / 200 + 1;                            /*科學*/
            if (class1 > 5)  class1 = 5;
            class2 = (d.affect * 2 + d.wisdom + d.art * 2 + d.character) / 400 + 1; /*詩詞*/
            if (class2 > 5)  class2 = 5;
            class3 = (d.belief * 2 + d.wisdom) / 400 + 1;           /*神學*/
            if (class3 > 5)  class3 = 5;
            class4 = (d.hskill * 2 + d.wisdom) / 400 + 1;           /*軍學*/
            if (class4 > 5)  class4 = 5;
            class5 = (d.hskill + d.attack) / 400 + 1;               /*劍術*/
            if (class5 > 5)  class5 = 5;
            class6 = (d.hskill + d.resist) / 400 + 1;               /*格鬥*/
            if (class6 > 5)  class6 = 5;
            class7 = (d.mskill + d.maxmp) / 400 + 1;                /*魔法*/
            if (class7 > 5)  class7 = 5;
            class8 = (d.manners * 2 + d.character) / 400 + 1;       /*禮儀*/
            if (class8 > 5)  class8 = 5;
            class9 = (d.art * 2 + d.character) / 400 + 1;           /*繪畫*/
            if (class9 > 5)  class9 = 5;
            class10 = (d.art * 2 + d.charm) / 400 + 1;              /*舞蹈*/
            if (class10 > 5) class10 = 5;

            move(b_lines - 1, 0);
            prints(menuname[menunum][0], class1, class2, class3, class4, class5, d_cols, "");
            move(b_lines, 0);
            prints(menuname[menunum][1], class6, class7, class8, class9, class10, d_cols, "");
        }
        else
        {
            move(b_lines - 1, 0);

            int fill;
            switch (menunum)
            {
            case 2:
                fill = 20 + d_cols - (sizeof(NICKNAME) - 1);
                break;
            case 6:
                fill = 20 + d_cols - 2 * (sizeof(NICKNAME) - 1);
                break;
            default:
                fill = d_cols;
            }
            prints(menuname[menunum][0], fill, "");

            move(b_lines, 0);
            prints(menuname[menunum][1], d_cols, "");
        }

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
                /*if (key == tolower(pipkey))*/
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
/* 基本選單:餵食 清潔 親親 休息                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/
static int pip_main_menu(void)
{
    pip_do_menu(0, 0, pipmainlist);
    return 0;
}

/*---------------------------------------------------------------------------*/
/* 基本選單:餵食 清潔 親親 休息                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/
static int pip_basic_menu(void)
{
    pip_do_menu(1, 0, pipbasiclist);
    return 0;
}

/*---------------------------------------------------------------------------*/
/* 商店選單:食物 零食 大補丸 玩具 書本                                       */
/*                                                                           */
/*---------------------------------------------------------------------------*/
static int pip_store_menu(void)
{
    pip_do_menu(2, 1, pipstorelist);
    return 0;
}

/*---------------------------------------------------------------------------*/
/* 修行選單:念書 練武 修行                                                   */
/*                                                                           */
/*---------------------------------------------------------------------------*/
static int pip_practice_menu(void)
{
    pip_do_menu(3, 3, pippracticelist);
    return 0;
}


/*---------------------------------------------------------------------------*/
/* 玩樂選單:散步 旅遊 運動 約會 猜拳                                         */
/*                                                                           */
/*---------------------------------------------------------------------------*/
static int pip_play_menu(void)
{
    pip_do_menu(4, 0, pipplaylist);
    return 0;
}

/*---------------------------------------------------------------------------*/
/* 打工選單:家事 苦工 家教 地攤                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/
static int pip_job_menu(void)
{
    pip_do_menu(5, 2, pipjoblist);
    return 0;
}

/*---------------------------------------------------------------------------*/
/* 特殊選單:看病 減肥 戰鬥 拜訪 朝見                                         */
/*                                                                           */
/*---------------------------------------------------------------------------*/
static int pip_special_menu(void)
{
    pip_do_menu(6, 0, pipspeciallist);
    return 0;
}

/*---------------------------------------------------------------------------*/
/* 系統選單:個人資料  小雞放生  特別服務                                     */
/*                                                                           */
/*---------------------------------------------------------------------------*/
static int pip_system_menu(void)
{
    pip_do_menu(7, 0, pipsystemlist);
    return 0;
}


static int
pip_mainmenu(
int mode)
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
    char yo[12][5] = {"誕生", "嬰兒", "幼兒", "兒童", "少年", "青年",
                      "成年", "壯年", "更年", "老年", "古稀", "神仙"
                     };

    color1 = color2 = color3 = color4 = 37;
    move(1, 0);
    tm = (time(0) - start_time + d.bbtime) / 60 / 30; /* 一歲 */
    tm1 = (time(0) - start_time + d.bbtime) / 60;
    m = d.bbtime / 60 / 30;
    m1 = d.bbtime / 60;
    /*長大一歲時的增加改變值*/
    if (m != tm)
    {
        d.wisdom += 10;
        d.happy += random() % 5 + 5;
        if (d.happy > 100)
            d.happy = 100;
        d.satisfy += random() % 5;
        if (d.satisfy > 100)
            d.satisfy = 100;
        if (tm < 13) d.maxhp += random() % 5 + 5; else d.maxhp -= random() % 15;
        d.character += random() % 5;
        d.money += 500;
        d.seeroyalJ = 1;
        count_tired(1, 7, "N", 100, 0);
        d.bbtime += time(0) - start_time;
        start_time = time(0);
        pip_write_file();

        /*記錄開始*/
        now = time(0);
        sprintf(genbuf, "\x1b[1;37m%s %-11s的小雞 [%s] 滿 %d 歲了 \x1b[m\n", Cdate(&now), cuser.userid, d.name, m + 1);
        pip_log_record(genbuf);
        /*記錄終止*/
        clear();
        vs_head("電子養小雞", BoardName);
        show_basic_pic(20); /*生日快樂*/
        vmsg("小雞長大一歲了..");
        /*結局*/
        if (tm % 2 == 0)
            pip_results_show();
        if (tm >= 21 && (d.wantend == 4 || d.wantend == 5 || d.wantend == 6))
            pip_ending_screen();

        clrtobot();
        refresh();
    }
    color = 37;
    m = tm;

    if ((random() % 3000 == 29) && tm >= 15 && d.charm >= 300 && d.character >= 300)
        pip_marriage_offer();

    if (mode != 1 && random() % 4000 == 69)
        pip_divine();

    /*武官*/
    if ((time(0) - start_time) >= 900)
    {
        d.seeroyalJ = 0;
    }

    if (m == 0)                   /*誕生*/
        age = 0;
    else if (m == 1)              /*嬰兒*/
        age = 1;
    else if (m >= 2 && m <= 5)    /*幼兒*/
        age = 2;
    else if (m >= 6 && m <= 12)   /*兒童*/
        age = 3;
    else if (m >= 13 && m <= 15)  /*少年*/
        age = 4;
    else if (m >= 16 && m <= 18)  /*青年*/
        age = 5;
    else if (m >= 19 && m <= 35)  /*成年*/
        age = 6;
    else if (m >= 36 && m <= 45)  /*壯年*/
        age = 7;
    else if (m >= 45 && m <= 60)  /*更年*/
        age = 8;
    else if (m >= 60 && m <= 70)  /*老年*/
        age = 9;
    else if (m >= 70 && m <= 100) /*古稀*/
        age = 10;
    else  // m > 100              /*神仙*/
        age = 11;
    clear();
    /*vs_head("電子養小雞", BoardName);*/
    move(0, 0);
    if (d.sex == 1)
        sprintf(buf, "\x1b[1;41m  " NICKNAME PIPNAME " ～ [%s代雞] \x1b[32m♂ \x1b[37m%-15s     %*s\x1b[m", d.chickenmode ? "二" : "一", d.name, 40 + d_cols - ((int)(unsigned int)sizeof(NICKNAME PIPNAME) - 1), "");
    else if (d.sex == 2)
        sprintf(buf, "\x1b[1;41m  " NICKNAME PIPNAME " ～ [%s代雞] \x1b[33m♀ \x1b[37m%-15s     %*s\x1b[m", d.chickenmode ? "二" : "一", d.name, 40 + d_cols - ((int)(unsigned int)sizeof(NICKNAME PIPNAME) - 1), "");
    else
        sprintf(buf, "\x1b[1;41m  " NICKNAME PIPNAME " ～ [%s代雞] \x1b[34m？ \x1b[37m%-15s     %*s\x1b[m", d.chickenmode ? "二" : "一", d.name, 40 + d_cols - ((int)(unsigned int)sizeof(NICKNAME PIPNAME) - 1), "");
    outs(buf);

    move(1, 0);
    if (d.money <= 100)
        color1 = 31;
    else if (d.money > 100 && d.money <= 500)
        color1 = 33;
    else
        color1 = 37;
    sprintf(inbuf1, "%02d/%02d/%02d", (d.year - 11) % 100, d.month, d.day);
    sprintf(buf,
            " \x1b[1;32m[狀  態]\x1b[37m %-5s     \x1b[32m[生  日]\x1b[37m %-9s \x1b[32m[年  齡]\x1b[37m %-5d     \x1b[32m[金  錢]\x1b[%dm %-8d \x1b[m",
            yo[age], inbuf1, tm, color1, d.money);
    outs_centered(buf);

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

    sprintf(buf,
            " \x1b[1;32m[生  命]\x1b[%dm %-10d\x1b[32m[法  力]\x1b[%dm %-10d\x1b[32m[體  重]\x1b[37m %-5d     \x1b[32m[疲  勞]\x1b[%dm %-4d\x1b[0m ",
            color1, d.hp, color2, d.mp, d.weight, color3, d.tired);
    outs_centered(buf);

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
    sprintf(buf,
            " \x1b[1;32m[命 MAX]\x1b[37m %-10d\x1b[32m[法 MAX]\x1b[37m %-10d\x1b[32m[髒／病]\x1b[%dm %-4d\x1b[37m/\x1b[%dm%-4d \x1b[32m[快／滿]\x1b[%dm %-4d\x1b[37m/\x1b[%dm%-4d\x1b[m",
            d.maxhp, d.maxmp, color1, d.shit, color2, d.sick, color3, d.happy, color4, d.satisfy);
    outs_centered(buf);
    if (mode == 0)  /*主要畫面*/
    {
        anynum = 0;
        anynum = random() % 4;
        move(4, 0);
        if (anynum == 0)
            sprintf(buf, " \x1b[1;35m[站長曰]:\x1b[31m紅色\x1b[36m表示危險  \x1b[33m黃色\x1b[36m表示警告  \x1b[37m白色\x1b[36m表示安全\x1b[0m");
        else if (anynum == 1)
            sprintf(buf, " \x1b[1;35m[站長曰]:\x1b[37m要多多注意小雞的疲勞度和病氣  以免累死病死\x1b[0m");
        else if (anynum == 2)
            sprintf(buf, " \x1b[1;35m[站長曰]:\x1b[37m隨時注意小雞的生命數值唷!\x1b[0m");
        else if (anynum == 3)
            sprintf(buf, " \x1b[1;35m[站長曰]:\x1b[37m快快樂樂的小雞才是幸福的小雞.....\x1b[0m");
        outs_centered(buf);
    }
    else if (mode == 1)/*餵食*/
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
        sprintf(buf,
                " \x1b[1;36m[食物]\x1b[%dm%-7d\x1b[36m[零食]\x1b[%dm%-7d\x1b[36m[補丸]\x1b[%dm%-7d\x1b[36m[靈芝]\x1b[%dm%-7d\x1b[36m[人參]\x1b[37m%-7d\x1b[36m[雪蓮]\x1b[37m%-7d\x1b[0m",
                color1, d.food, color2, d.cookie, color3, d.bighp, color4, d.medicine, d.ginseng, d.snowgrass);
        outs_centered(buf);

    }
    else if (mode == 2)/*打工*/
    {
        move(4, 0);
        sprintf(buf,
                " \x1b[1;36m[愛心]\x1b[37m%-5d\x1b[36m[智慧]\x1b[37m%-5d\x1b[36m[氣質]\x1b[37m%-5d\x1b[36m[藝術]\x1b[37m%-5d\x1b[36m[道德]\x1b[37m%-5d\x1b[36m[勇敢]\x1b[37m%-5d\x1b[36m[家事]\x1b[37m%-5d\x1b[0m",
                d.love, d.wisdom, d.character, d.art, d.ethics, d.brave, d.homework);
        outs_centered(buf);

    }
    else if (mode == 3)/*修行*/
    {
        move(4, 0);
        sprintf(buf,
                " \x1b[1;36m[智慧]\x1b[37m%-5d\x1b[36m[氣質]\x1b[37m%-5d\x1b[36m[藝術]\x1b[37m%-5d\x1b[36m[勇敢]\x1b[37m%-5d\x1b[36m[攻擊]\x1b[37m%-5d\x1b[36m[防禦]\x1b[37m%-5d\x1b[36m[速度]\x1b[37m%-5d\x1b[0m",
                d.wisdom, d.character, d.art, d.brave, d.attack, d.resist, d.speed);
        outs_centered(buf);

    }
    move(5, 0);
    prints_centered("\x1b[1;%dm┌─────────────────────────────────────┐\x1b[m", color);
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


    move(b_lines - 5, 0);
    prints_centered("\x1b[1;%dm└─────────────────────────────────────┘\x1b[m", color);
    move(b_lines - 4, 0);
    outs_centered(" \x1b[1;34m─\x1b[37;44m  狀 態  \x1b[0;1;34m─\x1b[0m");
    move(b_lines - 3, 0);
    outs_centered(" ");
    if (d.shit == 0)
        outs("乾淨小雞  ");
    if (d.shit > 40 && d.shit < 60)
        outs("有點臭臭  ");
    if (d.shit >= 60 && d.shit < 80)
        outs("\x1b[1;33m很臭了說\x1b[m  ");
    if (d.shit >= 80 && d.shit < 100)
    {
        outs("\x1b[1;35m快臭死了\x1b[m  ");
        d.sick += 4;
        d.character -= (random() % 3 + 3);
    }
    if (d.shit >= 100)
    {
        d.death = 1;
        pipdie("\x1b[1;31m哇～臭死了\x1b[m  ", 1);
        return -1;
    }

    if (d.hp <= 0)
        pc = 0;
    else
        pc = d.hp * 100 / d.maxhp;
    if (pc == 0)
    {
        d.death = 1;
        pipdie("\x1b[1;31m嗚～餓死了\x1b[m  ", 1);
        return -1;
    }
    if (pc < 20)
    {
        outs("\x1b[1;35m快餓昏了\x1b[m  ");
        d.sick += 3;
        d.happy -= 5;
        d.satisfy -= 3;
    }
    if (pc < 40 && pc >= 20)
        outs("\x1b[1;33m想吃東西\x1b[m  ");
    if (pc <= 100 && pc >= 90)
        outs("肚子飽飽  ");
    if (pc < 110 && pc > 100)
        outs("\x1b[1;33m撐撐的說\x1b[m  ");

    pc = d.tired;
    if (pc < 20)
        outs("精神很好  ");
    if (pc < 80 && pc >= 60)
        outs("\x1b[1;33m有點小累\x1b[m  ");
    if (pc < 100 && pc >= 80)
    {
        outs("\x1b[1;35m真的很累\x1b[m  ");
        d.sick += 5;
    }
    if (pc >= 100)
    {
        d.death = 1;
        pipdie("\x1b[1;31mㄚ～累死了\x1b[m  ", 1);
        return -1;
    }

    pc = 60 + 10 * tm;
    if (d.weight < (pc + 30) && d.weight >= (pc + 10))
        outs("\x1b[1;33m有點小胖\x1b[m  ");
    if (d.weight < (pc + 50) && d.weight >= (pc + 30))
    {
        outs("\x1b[1;35m太胖了啦\x1b[m  ");
        d.sick += 3;
        if (d.speed >= 2)
            d.speed -= 2;
        else
            d.speed = 0;

    }
    if (d.weight > (pc + 50))
    {
        d.death = 1;
        pipdie("\x1b[1;31m嗚～肥死了\x1b[m  ", 1);
        return -1;
    }

    if (d.weight < (pc - 50))
    {
        d.death = 1;
        pipdie("\x1b[1;31m:~~ 瘦死了\x1b[m  ", 1);
        return -1;
    }
    if (d.weight > (pc - 30) && d.weight <= (pc - 10))
        outs("\x1b[1;33m有點小瘦\x1b[m  ");
    if (d.weight > (pc - 50) && d.weight <= (pc - 30))
        outs("\x1b[1;35m太瘦了喔\x1b[m ");

    if (d.sick < 75 && d.sick >= 50)
    {
        outs("\x1b[1;33m生病了啦\x1b[m  ");
        count_tired(1, 8, "Y", 100, 1);
    }
    if (d.sick < 100 && d.sick >= 75)
    {
        outs("\x1b[1;35m正病重中\x1b[m  ");
        d.sick += 5;
        count_tired(1, 15, "Y", 100, 1);
    }
    if (d.sick >= 100)
    {
        d.death = 1;
        pipdie("\x1b[1;31m病死了啦 :~~\x1b[m  ", 1);
        return -1;
    }

    pc = d.happy;
    if (pc < 20)
        outs("\x1b[1;35m很不快樂\x1b[m  ");
    if (pc < 40 && pc >= 20)
        outs("\x1b[1;33m不太快樂\x1b[m  ");
    if (pc < 95 && pc >= 80)
        outs("快樂啦..  ");
    if (pc <= 100 && pc >= 95)
        outs("很快樂..  ");

    pc = d.satisfy;
    if (pc < 20) outs("\x1b[1;35m很不滿足..\x1b[m  ");
    if (pc < 40 && pc >= 20) outs("\x1b[1;33m不太滿足\x1b[m  ");
    if (pc < 95 && pc >= 80) outs("滿足啦..  ");
    if (pc <= 100 && pc >= 95) outs("很滿足..  ");

    outs("\n");

    pip_write_file();
    return 0;
}

/*固定時間作的事 */
static void
pip_time_change(
time_t cnow)
{
    int stime = 60;
    int stired = 2;
    while ((time(0) - lasttime) >= stime) /* 固定時間做的事 */
    {
        /*不做事  還是會變髒的*/
        if ((time(0) - cnow) >= stime)
            d.shit += (random() % 3 + 3);
        /*不做事  疲勞當然減低啦*/
        if (d.tired >= stired) d.tired -= stired; else d.tired = 0;
        /*不做事  肚子也會餓咩 */
        d.hp -= random() % 2 + 2;
        if (d.mexp < 0)
            d.mexp = 0;
        if (d.hexp < 0)
            d.hexp = 0;
        /*體力會因生病降低一點*/
        d.hp -= d.sick / 10;
        /*病氣會隨機率增加減少少許*/
        if (random() % 3 > 0)
        {
            d.sick -= random() % 2;
            if (d.sick < 0)
                d.sick = 0;
        }
        else
            d.sick += random() % 2;
        /*隨機減快樂度*/
        if (random() % 4 > 0)
        {
            d.happy -= random() % 2 + 2;
        }
        else
            d.happy += 2;
        if (random() % 4 > 0)
        {
            d.satisfy -= (random() % 4 + 5);
        }
        else
            d.satisfy += 2;
        lasttime += stime;
    }
    /*快樂度滿意度最大值設定*/
    if (d.happy > 100)
        d.happy = 100;
    else if (d.happy < 0)
        d.happy = 0;
    if (d.satisfy > 100)
        d.satisfy = 100;
    else if (d.satisfy < 0)
        d.satisfy = 0;
    /*評價*/
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
/* 基本選單:餵食 清潔 親親 休息                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static int pip_basic_takeshower(void) /*洗澡*/
{
    int lucky;
    d.shit -= 20;
    if (d.shit < 0) d.shit = 0;
    d.hp -= random() % 2 + 3;
    move(4, 0);
    lucky = random() % 3;
    if (lucky == 0)
    {
        show_usual_pic(1);
        vmsg("我是乾淨的小雞  cccc....");
    }
    else if (lucky == 1)
    {
        show_usual_pic(7);
        vmsg("馬桶 嗯～～");
    }
    else
    {
        show_usual_pic(2);
        vmsg("我愛洗澡 lalala....");
    }
    return 0;
}

static int pip_basic_takerest(void) /*休息*/
{
    count_tired(5, 20, "Y", 100, 0);
    if (d.hp > d.maxhp)
        d.hp = d.maxhp;
    d.shit += 1;
    move(4, 0);
    show_usual_pic(5);
    vmsg("再按一下我就起床囉....");
    show_usual_pic(6);
    vmsg("喂喂喂..該起床囉......");
    return 0;
}

static int pip_basic_kiss(void)/*親親*/
{
    if (random() % 2 > 0)
    {
        d.happy += random() % 3 + 4;
        d.satisfy += random() % 2 + 1;
    }
    else
    {
        d.happy += random() % 2 + 1;
        d.satisfy += random() % 3 + 4;
    }
    count_tired(1, 2, "N", 100, 1);
    d.shit += random() % 5 + 4;
    d.relation += random() % 2;
    move(4, 0);
    show_usual_pic(3);
    if (d.shit < 60)
    {
        vmsg("來嘛! 啵一個.....");
    }
    else
    {
        vmsg("親太多也是會髒死的喔....");
    }
    return 0;
}

static int pip_basic_feed(void)     /* 餵食*/
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
        sprintf(buf, "%s該做什麼事呢?", d.name);
        outs_centered(buf);
        now = time(0);
        move(b_lines, 0);
        clrtoeol();
        move(b_lines, 0);
        prints("\x1b[1;44;37m  飲食選單  \x1b[46m[1]吃飯 [2]零食 [3]補丸 [4]靈芝 [5]人蔘 [6]雪蓮 [Q]跳出           %*s\x1b[m", d_cols, "");
        pip_time_change(now);
        pipkey = vkey();
        pip_time_change(now);

        switch (pipkey)
        {
        case '1':
            if (d.food <= 0)
            {
                move(b_lines, 0);
                vmsg("沒有食物囉..快去買吧！");
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
                d.weight += random() % 2;
            }
            d.nodone = 0;
            vmsg("每吃一次食物會恢復體力50喔!");
            break;

        case '2':
            if (d.cookie <= 0)
            {
                move(b_lines, 0);
                vmsg("零食吃光囉..快去買吧！");
                break;
            }
            move(4, 0);
            d.cookie--;
            d.hp += 100;
            if (d.hp >= d.maxhp)
            {
                d.hp = d.maxhp;
                d.weight += (random() % 2 + 2);
            }
            else
            {
                d.weight += (random() % 2 + 1);
            }
            if (random() % 2 > 0)
                show_feed_pic(2);
            else
                show_feed_pic(3);
            d.happy += (random() % 3 + 4);
            d.satisfy += random() % 3 + 2;
            d.nodone = 0;
            vmsg("吃零食容易胖喔...");
            break;

        case '3':
            if (d.bighp <= 0)
            {
                move(b_lines, 0);
                vmsg("沒有大補丸了耶! 快買吧..");
                break;
            }
            d.bighp--;
            d.hp += 600;
            d.tired -= 20;
            d.weight += random() % 2;
            move(4, 0);
            show_feed_pic(4);
            d.nodone = 0;
            vmsg("補丸..超級棒的唷...");
            break;

        case '4':
            if (d.medicine <= 0)
            {
                move(b_lines, 0);
                vmsg("沒有靈芝囉..快去買吧！");
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
            vmsg("每吃一次靈芝會恢復法力50喔!");
            break;

        case '5':
            if (d.ginseng <= 0)
            {
                move(b_lines, 0);
                vmsg("沒有千年人蔘耶! 快買吧..");
                break;
            }
            d.ginseng--;
            d.mp += 500;
            d.tired -= 20;
            move(4, 0);
            show_feed_pic(1);
            d.nodone = 0;
            vmsg("千年人蔘..超級棒的唷...");
            break;

        case '6':
            if (d.snowgrass <= 0)
            {
                move(b_lines, 0);
                vmsg("沒有天山雪蓮耶! 快買吧..");
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
            vmsg("天山雪蓮..超級棒的唷...");
            break;

        }
    }
    while ((pipkey != 'Q') && (pipkey != 'q') && (pipkey != KEY_LEFT));

    return 0;
}

/*遊戲寫資料入檔案*/
static void pip_write_file(void)
{
    FILE *ff;
    char buf[200];
    /* sprintf(buf, "home/%s/chicken", cuser.userid);*/
    usr_fpath(buf, cuser.userid, "chicken");

    if ((ff = fopen(buf, "w")))
    {
        fprintf(ff, "%ld\n", d.bbtime);
        fprintf(ff,
                "%d %d %d %d %d %d %d %d %d \n"
                "%d %d %d %d %d %d %d %d %d %d \n"
                "%d %d %d %d %d %d %d %d %d %d \n"
                "%d %d %d %d %d %d %d %d %d %d \n"
                "%d %d %d %d %d %d %d %d %d %d \n"
                "%d %d %d %d %d %d %d %d %d %d \n"
                "%d %d %d %d %d %d %d %d %d %d \n"
                "%d %d %d %d %d %d %d %d %d %d \n"
                "%d %d %d %d %d %d %d %d %d %d \n"
                "%d %d %d %d %d %d %d %d %d %d \n"
                "%d %d %d %d %d %d %d %d %d %d \n"
                "%d %d %d %d %d %d %d %d %d %d \n"
                "%d %d %s %d %d %d %d %d %d %d \n"
                "%d %d %d %d %d %d %d %d %d %d \n"
                "%d %d %d %d %d %d %d %d %d %d \n"
                "%d %d %d %d %d %d %d %d %d %d %d %d %d %d",
                d.year, d.month, d.day, d.sex, d.death, d.nodone, d.relation, d.liveagain, d.chickenmode, d.level, d.exp, d.dataE,
                d.hp, d.maxhp, d.weight, d.tired, d.sick, d.shit, d.wrist, d.bodyA, d.bodyB, d.bodyC, d.bodyD, d.bodyE,
                d.social, d.family, d.hexp, d.mexp, d.tmpA, d.tmpB, d.tmpC, d.tmpD, d.tmpE,
                d.mp, d.maxmp, d.attack, d.resist, d.speed, d.hskill, d.mskill, d.mresist, d.magicmode, d.specialmagic, d.fightC, d.fightD, d.fightE,
                d.weaponhead, d.weaponrhand, d.weaponlhand, d.weaponbody, d.weaponfoot, d.weaponA, d.weaponB, d.weaponC, d.weaponD, d.weaponE,
                d.toman, d.character, d.love, d.wisdom, d.art, d.ethics, d.brave, d.homework, d.charm, d.manners, d.speech, d.cookskill, d.learnA, d.learnB, d.learnC, d.learnD, d.learnE,
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

/*遊戲讀資料出檔案*/
static void pip_read_file(const char *userid)
{
    FILE *fs;
    char buf[200];
    /* sprintf(buf, "home/%s/chicken", userid);*/
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
               &(d.toman), &(d.character), &(d.love), &(d.wisdom), &(d.art), &(d.ethics), &(d.brave), &(d.homework), &(d.charm), &(d.manners), &(d.speech), &(d.cookskill), &(d.learnA), &(d.learnB), &(d.learnC), &(d.learnD), &(d.learnE),
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
        vmsg("我沒有養小雞啦 !");
        return;
    }

    return;
}

/*記錄到pip.log檔*/
static void
pip_log_record(
const char *msg)
{
    FILE *fs;

    fs = fopen(FN_PIP_LOG, "a+");
    fprintf(fs, "%s", msg);
    fclose(fs);
}

/*小雞進度儲存*/
static int
pip_write_backup(void)
{
    const char *files[4] = {"沒有", "進度一", "進度二", "進度三"};
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
        outs("儲存 [1]進度一 [2]進度二 [3]進度三 [Q]放棄 [1/2/3/Q]：");
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
        vmsg("放棄儲存遊戲進度");
        return 0;
    }
    move(b_lines -2, 1);
    prints("儲存檔案會覆蓋\原儲存於 [%s] 的小雞的檔案喔！請考慮清楚...", files[num]);
    sprintf(buf1, "確定要儲存於 [%s] 檔案嗎？ [y/N]: ", files[num]);
    getdata(b_lines - 1, 1, buf1, ans, 2, DOECHO, 0);
    if (ans[0] != 'y' && ans[0] != 'Y')
    {
        vmsg("放棄儲存檔案");
        return 0;
    }

    move(b_lines -1, 0);
    clrtobot();
    sprintf(buf1, "儲存 [%s] 檔案完成了", files[num]);
    vmsg(buf1);
    sprintf(buf, "/bin/cp %s %s.bak%d", get_path(cuser.userid, "chicken"), get_path(cuser.userid, "chicken"), num);
    system(buf);
    return 0;
}

static int
pip_read_backup(void)
{
    char buf[200], buf1[200], buf2[200];
    const char *files[4] = {"沒有", "進度一", "進度二", "進度三"};
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
        outs("讀取 [1]進度一 [2]進度二 [3]進度三 [Q]放棄 [1/2/3/Q]：");
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
            sprintf(buf + strlen(buf), "%d", num);
            if ((fs = fopen(buf, "r")) == NULL)
            {
                sprintf(buf, "檔案 [%s] 不存在", files[num]);
                vmsg(buf);
                ok = 0;
            }
            else
            {

                move(b_lines - 2, 1);
                outs("讀取出檔案會覆蓋\現在正在玩的小雞的檔案喔！請考慮清楚...");
                sprintf(buf, "確定要讀取出 [%s] 檔案嗎？ [y/N]: ", files[num]);
                getdata(b_lines - 1, 1, buf, ans, 2, DOECHO, 0);
                if (ans[0] != 'y' && ans[0] != 'Y')
                    vmsg("讓我再決定一下...");
                else ok = 1;
                fclose(fs);
            }
        }
    }
    while (pipkey != 'Q' && pipkey != 'q' && ok != 1);
    if (pipkey == 'q' || pipkey == 'Q')
    {
        vmsg("還是玩原本的遊戲");
        return 0;
    }

    move(b_lines -1, 0);
    clrtobot();
    sprintf(buf, "讀取 [%s] 檔案完成了", files[num]);
    vmsg(buf);

    sprintf(buf1, "/bin/touch %s%d", get_path(cuser.userid, "chicken.bak"), num);
    sprintf(buf2, "/bin/cp %s.bak%d %s", get_path(cuser.userid, "chicken"), num, get_path(cuser.userid, "chicken"));
    system(buf1);
    system(buf2);
    pip_read_file(cuser.userid);
    return 0;
}



static int
pip_live_again(void)
{
    char genbuf[80];
    time_t now;
    int tm;

    tm = (d.bbtime) / 60 / 30;

    clear();
    vs_head("小雞復活手術中", BoardName);

    now = time(0);
    sprintf(genbuf, "\x1b[1;33m%s %-11s的小雞 [%s二代] 復活了！\x1b[m\n", Cdate(&now), cuser.userid, d.name);
    pip_log_record(genbuf);

    /*身體上的設定*/
    d.death = 0;
    d.maxhp = d.maxhp * ALIVE + 1;
    d.hp = d.maxhp;
    d.tired = 20;
    d.shit = 20;
    d.sick = 20;
    d.wrist = d.wrist * ALIVE;
    d.weight = 45 + 10 * tm;

    /*錢減到五分之一*/
    d.money = d.money * ALIVE;

    /*戰鬥能力降一半*/
    d.attack = d.attack * ALIVE;
    d.resist = d.resist * ALIVE;
    d.maxmp = d.maxmp * ALIVE;
    d.mp = d.maxmp;

    /*變得不快樂*/
    d.happy = 0;
    d.satisfy = 0;

    /*評價減半*/
    d.social = d.social * ALIVE;
    d.family = d.family * ALIVE;
    d.hexp = d.hexp * ALIVE;
    d.mexp = d.mexp * ALIVE;

    /*武器掉光光*/
    d.weaponhead = 0;
    d.weaponrhand = 0;
    d.weaponlhand = 0;
    d.weaponbody = 0;
    d.weaponfoot = 0;

    /*食物剩一半*/
    d.food = d.food * ALIVE;
    d.medicine = d.medicine * ALIVE;
    d.bighp = d.bighp * ALIVE;
    d.cookie = d.cookie * ALIVE;

    d.liveagain += 1;

    vmsg("小雞器官重建中！");
    vmsg("小雞體質恢復中！");
    vmsg("小雞能力調整中！");
    vmsg("恭喜您，你的小雞又復活囉！");
    pip_write_file();
    return 0;
}

/*---------------------------------------------------------------------------*/
/* 小雞圖形區                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void
show_basic_pic(int i)
{
    char buf[256];
    clrchyiuan(6, b_lines - 6);
    clrchyiuan(b_lines - 1, b_lines);
    sprintf(buf, BBSHOME"/game/pipgame/basic/pic%d", i);
    show_file(buf, 6, 12, ONLY_COLOR);
}

static void
show_feed_pic(int i)  /*吃東西*/
{
    char buf[256];
    clrchyiuan(6, b_lines - 6);
    clrchyiuan(b_lines - 1, b_lines);
    sprintf(buf, BBSHOME"/game/pipgame/feed/pic%d", i);
    show_file(buf, 6, 12, ONLY_COLOR);
}

static void
show_buy_pic(int i)  /*購買東西*/
{
    char buf[256];
    clrchyiuan(6, b_lines - 6);
    clrchyiuan(b_lines - 1, b_lines);
    sprintf(buf, BBSHOME"/game/pipgame/buy/pic%d", i);
    show_file(buf, 6, 12, ONLY_COLOR);
}

static void
show_usual_pic(int i)  /* 平常狀態 */
{
    char buf[256];
    clrchyiuan(6, b_lines - 6);
    clrchyiuan(b_lines - 1, b_lines);
    sprintf(buf, BBSHOME"/game/pipgame/usual/pic%d", i);
    show_file(buf, 6, 12, ONLY_COLOR);

}

static void
show_special_pic(int i)  /* 特殊選單 */
{
    char buf[256];
    clrchyiuan(6, b_lines - 6);
    clrchyiuan(b_lines - 1, b_lines);
    sprintf(buf, BBSHOME"/game/pipgame/special/pic%d", i);
    show_file(buf, 6, 12, ONLY_COLOR);

}

static void
show_practice_pic(int i)  /*修行用的圖 */
{
    char buf[256];
    clrchyiuan(6, b_lines - 6);
    clrchyiuan(b_lines - 1, b_lines);
    sprintf(buf, BBSHOME"/game/pipgame/practice/pic%d", i);
    show_file(buf, 6, 12, ONLY_COLOR);
}

static void
show_job_pic(int i)    /* 打工的show圖 */
{
    char buf[256];
    clrchyiuan(6, b_lines - 6);
    clrchyiuan(b_lines - 1, b_lines);
    sprintf(buf, BBSHOME"/game/pipgame/job/pic%d", i);
    show_file(buf, 6, 12, ONLY_COLOR);

}


static void
show_play_pic(int i)  /*休閒的圖*/
{
    char buf[256];
    clrchyiuan(6, b_lines - 6);
    clrchyiuan(b_lines - 1, b_lines);
    sprintf(buf, BBSHOME"/game/pipgame/play/pic%d", i);
    if (i == 0)
        show_file(buf, 2, b_lines - 7, ONLY_COLOR);
    else
        show_file(buf, 6, 12, ONLY_COLOR);
}

static void
show_guess_pic(int i)  /* 猜拳用 */
{
    char buf[256];
    clrchyiuan(6, b_lines - 6);
    clrchyiuan(b_lines - 1, b_lines);
    sprintf(buf, BBSHOME"/game/pipgame/guess/pic%d", i);
    show_file(buf, 6, 12, ONLY_COLOR);
}

static void
show_weapon_pic(int i)  /* 武器用 */
{
    char buf[256];
    clrchyiuan(1, 10);
    clrchyiuan(b_lines - 1, b_lines);
    sprintf(buf, BBSHOME"/game/pipgame/weapon/pic%d", i);
    show_file(buf, 1, 10, ONLY_COLOR);
}

static void
show_palace_pic(int i)  /* 參見王臣用 */
{
    char buf[256];
    clrchyiuan(0, 13);
    clrchyiuan(b_lines - 1, b_lines);
    sprintf(buf, BBSHOME"/game/pipgame/palace/pic%d", i);
    show_file(buf, 0, 11, ONLY_COLOR);

}

static void
show_badman_pic(int i)  /* 壞人 */
{
    char buf[256];
    clrchyiuan(6, b_lines - 6);
    clrchyiuan(b_lines - 1, b_lines);
    sprintf(buf, BBSHOME"/game/pipgame/badman/pic%d", i);
    show_file(buf, 6, 14, ONLY_COLOR);
}

static void
show_fight_pic(int i)  /* 打架 */
{
    char buf[256];
    clrchyiuan(6, b_lines - 6);
    clrchyiuan(b_lines - 1, b_lines);
    sprintf(buf, BBSHOME"/game/pipgame/fight/pic%d", i);
    show_file(buf, 6, 14, ONLY_COLOR);
}

static void
show_die_pic(int i)  /*死亡*/
{
    char buf[256];
    clrchyiuan(0, b_lines);
    sprintf(buf, BBSHOME"/game/pipgame/die/pic%d", i);
    show_file(buf, 0, b_lines, ONLY_COLOR);
}

static void
show_system_pic(int i)  /*系統*/
{
    char buf[256];
    clrchyiuan(1, b_lines);
    sprintf(buf, BBSHOME"/game/pipgame/system/pic%d", i);
    show_file(buf, 4, 16, ONLY_COLOR);
}

static void
show_ending_pic(int i)  /*結束*/
{
    char buf[256];
    clrchyiuan(1, b_lines);
    sprintf(buf, BBSHOME"/game/pipgame/ending/pic%d", i);
    show_file(buf, 4, 16, ONLY_COLOR);
}

static void
show_resultshow_pic(int i)      /*收穫季*/
{
    char buf[256];
    clrchyiuan(0, b_lines);
    sprintf(buf, BBSHOME"/game/pipgame/resultshow/pic%d", i);
    show_file(buf, 0, b_lines, ONLY_COLOR);
}

/*---------------------------------------------------------------------------*/
/* 商店選單:食物 零食 大補丸 玩具 書本                                       */
/*                                                                           */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* 商店選單:食物 零食 大補丸 玩具 書本                                       */
/* 函式庫                                                                    */
/*---------------------------------------------------------------------------*/

static int pip_store_food(void)
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

static int pip_store_medicine(void)
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

static int pip_store_other(void)
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

static int pip_store_weapon_head(void)         /*頭部武器*/
{
    d.weaponhead = pip_weapon_doing_menu(d.weaponhead, 0, headlist);
    return 0;
}
static int pip_store_weapon_rhand(void)        /*右手武器*/
{
    d.weaponrhand = pip_weapon_doing_menu(d.weaponrhand, 1, rhandlist);
    return 0;
}
static int pip_store_weapon_lhand(void)        /*左手武器*/
{
    d.weaponlhand = pip_weapon_doing_menu(d.weaponlhand, 2, lhandlist);
    return 0;
}
static int pip_store_weapon_body(void)         /*身體武器*/
{
    d.weaponbody = pip_weapon_doing_menu(d.weaponbody, 3, bodylist);
    return 0;
}
static int pip_store_weapon_foot(void)         /*足部武器*/
{
    d.weaponfoot = pip_weapon_doing_menu(d.weaponfoot, 4, footlist);
    return 0;
}


static int
pip_buy_goods_new(
int mode,
const struct goodsofpip *p,
int oldnum[])
{
    const char *shopname[4] = {"店名", "便利商店", NICKNAME "藥鋪", "夜裡書局"};
    char inbuf[256];
    char genbuf[20];
    long smoney;
    int oldmoney;
    int i, pipkey, choice;
    oldmoney = d.money;
    do
    {
        clrchyiuan(6, b_lines - 6);
        move(6, 0);
        sprintf(inbuf, "\x1b[1;31m  ─\x1b[41;37m 編號 \x1b[0;1;31m─\x1b[41;37m 商      品 \x1b[0;1;31m──\x1b[41;37m 效            能 \x1b[0;1;31m──\x1b[41;37m 價     格 \x1b[0;1;31m─\x1b[37;41m 擁有數量 \x1b[0;1;31m─\x1b[0m  ");
        outs_centered(inbuf);
        for (i = 1; i <= oldnum[0]; i++)
        {
            move(7 + i, 0);
            sprintf(inbuf, "     \x1b[1;35m[\x1b[37m%2d\x1b[35m]     \x1b[36m%-10s      \x1b[37m%-14s        \x1b[1;33m%-10d   \x1b[1;32m%-9d    \x1b[0m",
                    p[i].num, p[i].name, p[i].msgbuy, p[i].money, oldnum[i]);
            outs_centered(inbuf);
        }
        clrchyiuan(b_lines - 4, b_lines);
        move(b_lines, 0);
        sprintf(inbuf, "\x1b[1;44;37m  %8s選單  \x1b[46m  [B]買入物品  [S]賣出物品  [Q]跳出     %*s\x1b[m", shopname[mode], 30 + d_cols - (int)(unsigned int)strlen(shopname[mode]), "");
        outs(inbuf);
        pipkey = vkey();
        switch (pipkey)
        {
        case 'B':
        case 'b':
            move(b_lines - 1, 1);
            sprintf(inbuf, "想要買入啥呢? [0]放棄買入 [1～%d]物品商號: ", oldnum[0]);
            getdata(b_lines - 1, 1, inbuf, genbuf, 3, LCECHO, "0");
            choice = atoi(genbuf);
            if (choice >= 1 && choice <= oldnum[0])
            {
                clrchyiuan(6, b_lines - 6);
                if (random() % 2 > 0)
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
                    sprintf(inbuf, "你要買入物品 [%s] 多少個呢?(上限 %d): ", p[choice].name, d.money / p[choice].money);
                    getdata(b_lines - 1, 1, inbuf, genbuf, 6, DOECHO, 0);
                    smoney = atoi(genbuf);
                }
                if (smoney < 0)
                {
                    vmsg("放棄買入...");
                }
                else if (d.money < smoney*p[choice].money)
                {
                    vmsg("你的錢沒有那麼多喔..");
                }
                else
                {
                    sprintf(inbuf, "確定買入物品 [%s] 數量 %ld 個嗎?(店家賣價 %ld) [y/N]: ", p[choice].name, smoney, smoney*p[choice].money);
                    getdata(b_lines - 1, 1, inbuf, genbuf, 2, DOECHO, 0);
                    if (genbuf[0] == 'y' || genbuf[0] == 'Y')
                    {
                        oldnum[choice] += smoney;
                        d.money -= smoney * p[choice].money;
                        sprintf(inbuf, "老闆給了你%ld個%s", smoney, p[choice].name);
                        vmsg(inbuf);
                        vmsg(p[choice].msguse);
                        if (mode == 3 && choice == 1)
                        {
                            d.happy += random() % 10 + 20 * smoney;
                            d.satisfy += random() % 10 + 20 * smoney;
                        }
                        if (mode == 3 && choice == 2)
                        {
                            d.happy += (random() % 2 + 2) * smoney;
                            d.wisdom += (2 + 10 / (d.wisdom / 100 + 1)) * smoney;
                            d.character += (random() % 4 + 2) * smoney;
                            d.art += (random() % 2 + 1) * smoney;
                        }
                    }
                    else
                    {
                        vmsg("放棄買入...");
                    }
                }
            }
            else
            {
                sprintf(inbuf, "放棄買入.....");
                vmsg(inbuf);
            }
            break;

        case 'S':
        case 's':
            if (mode == 3)
            {
                vmsg("這些東西不能賣喔....");
                break;
            }
            move(b_lines - 1, 1);
            sprintf(inbuf, "想要賣出啥呢? [0]放棄賣出 [1～%d]物品商號: ", oldnum[0]);
            getdata(b_lines - 1, 1, inbuf, genbuf, 3, LCECHO, "0");
            choice = atoi(genbuf);
            if (choice >= 1 && choice <= oldnum[0])
            {
                clrchyiuan(6, b_lines - 6);
                if (random() % 2 > 0)
                    show_buy_pic(p[choice].pic1);
                else
                    show_buy_pic(p[choice].pic2);
                move(b_lines - 1, 0);
                clrtoeol();
                move(b_lines - 1, 1);
                smoney = 0;
                sprintf(inbuf, "你要賣出物品 [%s] 多少個呢?(上限 %d): ", p[choice].name, oldnum[choice]);
                getdata(b_lines - 1, 1, inbuf, genbuf, 6,, 0);
                smoney = atoi(genbuf);
                if (smoney < 0)
                {
                    vmsg("放棄賣出...");
                }
                else if (smoney > oldnum[choice])
                {
                    sprintf(inbuf, "你的 [%s] 沒有那麼多個喔", p[choice].name);
                    vmsg(inbuf);
                }
                else
                {
                    sprintf(inbuf, "確定賣出物品 [%s] 數量 %ld 個嗎?(店家買價 %ld) [y/N]: ", p[choice].name, smoney, smoney*p[choice].money*8 / 10);
                    getdata(b_lines - 1, 1, inbuf, genbuf, 2, DOECHO, 0);
                    if (genbuf[0] == 'y' || genbuf[0] == 'Y')
                    {
                        oldnum[choice] -= smoney;
                        d.money += smoney * p[choice].money * 8 / 10;
                        sprintf(inbuf, "老闆拿走了你的%ld個%s", smoney, p[choice].name);
                        vmsg(inbuf);
                    }
                    else
                    {
                        vmsg("放棄賣出...");
                    }
                }
            }
            else
            {
                sprintf(inbuf, "放棄賣出.....");
                vmsg(inbuf);
            }
            break;
        case 'Q':
        case 'q':
            sprintf(inbuf, "金錢交易共 %d 元，離開 %s ", oldmoney - d.money, shopname[mode]);
            vmsg(inbuf);
            break;
        }
    }
    while ((pipkey != 'Q') && (pipkey != 'q') && (pipkey != KEY_LEFT));
    return 0;
}

static int
pip_weapon_doing_menu(             /* 武器購買畫面 */
int variance,
int type,
const struct weapon *p)
{
    time_t now;
    register int n = 0;
    register const char *s;
    char buf[256];
    char ans[5];
    char shortbuf[100];
    char menutitle[5][11] = {"頭部裝備區", "右手裝備區", "左手裝備區", "身體裝備區", "足部裝備區"};
    int pipkey;
    char choicekey[5];
    int choice;

    do
    {
        clear();
        vs_head(menutitle[type], BoardName);
        show_weapon_pic(0);
   /*   move(10, 2);
        sprintf(buf, "\x1b[1;37m現今能力:體力Max:\x1b[36m%-5d\x1b[37m  法力Max:\x1b[36m%-5d\x1b[37m  攻擊:\x1b[36m%-5d\x1b[37m  防禦:\x1b[36m%-5d\x1b[37m  速度:\x1b[36m%-5d \x1b[m",
                d.maxhp, d.maxmp, d.attack, d.resist, d.speed);
        outs_centered(buf);*/
        move(11, 2);
        sprintf(buf, "\x1b[1;37;41m [NO]  [器具名]  [體力]  [法力]  [速度]  [攻擊]  [防禦]  [速度]  [售  價] \x1b[m");
        outs_centered(buf);
        move(12, 2);
        sprintf(buf, " \x1b[1;31m──\x1b[37m白色 可以購買\x1b[31m──\x1b[32m綠色 擁有裝備\x1b[31m──\x1b[33m黃色 錢錢不夠\x1b[31m──\x1b[35m紫色 能力不足\x1b[31m──\x1b[m");
        outs_centered(buf);

        n = 0;
        while ((s = p[n].name))
        {
            move(13 + n, 2);
            if (variance != 0 && variance == (n))/*本身有的*/
            {
                sprintf(buf,
                        "\x1b[1;32m (%2d)  %-10s %4d    %4d    %4d    %4d    %4d    %4d    %6d\x1b[m",
                        n, p[n].name, p[n].needmaxhp, p[n].needmaxmp, p[n].needspeed,
                        p[n].attack, p[n].resist, p[n].speed, p[n].cost);
            }
            else if (d.maxhp < p[n].needmaxhp || d.maxmp < p[n].needmaxmp || d.speed < p[n].needspeed)/*能力不足*/
            {
                sprintf(buf,
                        "\x1b[1;35m (%2d)  %-10s %4d    %4d    %4d    %4d    %4d    %4d    %6d\x1b[m",
                        n, p[n].name, p[n].needmaxhp, p[n].needmaxmp, p[n].needspeed,
                        p[n].attack, p[n].resist, p[n].speed, p[n].cost);
            }

            else if (d.money < p[n].cost) /*錢不夠的*/
            {
                sprintf(buf,
                        "\x1b[1;33m (%2d)  %-10s %4d    %4d    %4d    %4d    %4d    %4d    %6d\x1b[m",
                        n, p[n].name, p[n].needmaxhp, p[n].needmaxmp, p[n].needspeed,
                        p[n].attack, p[n].resist, p[n].speed, p[n].cost);
            }
            else
            {
                sprintf(buf,
                        "\x1b[1;37m (%2d)  %-10s %4d    %4d    %4d    %4d    %4d    %4d    %6d\x1b[m",
                        n, p[n].name, p[n].needmaxhp, p[n].needmaxmp, p[n].needspeed,
                        p[n].attack, p[n].resist, p[n].speed, p[n].cost);
            }
            outs_centered(buf);
            n++;
        }
        move(b_lines, 0);
        sprintf(buf, "\x1b[1;44;37m  武器購買選單  \x1b[46m  [B]購買武器  [S]賣掉裝備  [W]個人資料  [Q]跳出              %*s\x1b[m", d_cols, "");
        outs(buf);
        now = time(0);
        pip_time_change(now);
        pipkey = vkey();
        pip_time_change(now);

        switch (pipkey)
        {
        case 'B':
        case 'b':
            move(b_lines - 1, 1);
            sprintf(shortbuf, "想要購買啥呢? 你的錢錢[%d]元:[數字]: ", d.money);
            outs(shortbuf);
            getdata(b_lines - 1, 1, shortbuf, choicekey, 4, LCECHO, "0");
            choice = atoi(choicekey);
            if (choice >= 0 && choice <= n)
            {
                move(b_lines - 1, 0);
                clrtoeol();
                move(b_lines - 1, 1);
                if (choice == 0)  /*解除*/
                {
                    sprintf(shortbuf, "放棄購買...");
                    vmsg(shortbuf);
                }

                else if (variance == choice)  /*早已經有啦*/
                {
                    sprintf(shortbuf, "你早已經有 %s 囉", p[variance].name);
                    vmsg(shortbuf);
                }

                else if (p[choice].cost >= (d.money + p[variance].sell))  /*錢不夠*/
                {
                    sprintf(shortbuf, "這個要 %d 元，你的錢不夠啦!", p[choice].cost);
                    vmsg(shortbuf);
                }

                else if (d.maxhp < p[choice].needmaxhp || d.maxmp < p[choice].needmaxmp
                          || d.speed < p[choice].needspeed)  /*能力不足*/
                {
                    sprintf(shortbuf, "需要HP %d MP %d SPEED %d 喔",
                            p[choice].needmaxhp, p[choice].needmaxmp, p[choice].needspeed);
                    vmsg(shortbuf);
                }
                else  /*順利購買*/
                {
                    sprintf(shortbuf, "你確定要購買 %s 嗎?($%d) [y/N]: ", p[choice].name, p[choice].cost);
                    getdata(b_lines - 1, 1, shortbuf, ans, 2, DOECHO, 0);
                    if (ans[0] == 'y' || ans[0] == 'Y')
                    {
                        sprintf(shortbuf, "小雞已經裝備上 %s 了", p[choice].name);
                        vmsg(shortbuf);
                        d.attack += (p[choice].attack - p[variance].attack);
                        d.resist += (p[choice].resist - p[variance].resist);
                        d.speed += (p[choice].speed - p[variance].speed);
                        d.money -= (p[choice].cost - p[variance].sell);
                        variance = choice;
                    }
                    else
                    {
                        sprintf(shortbuf, "放棄購買.....");
                        vmsg(shortbuf);
                    }
                }
            }
            break;

        case 'S':
        case 's':
            if (variance != 0)
            {
                sprintf(shortbuf, "你確定要賣掉%s嗎? 賣價:%d [y/N]: ", p[variance].name, p[variance].sell);
                getdata(b_lines - 1, 1, shortbuf, ans, 2, DOECHO, 0);
                if (ans[0] == 'y' || ans[0] == 'Y')
                {
                    sprintf(shortbuf, "裝備 %s 賣了 %d", p[variance].name, p[variance].sell);
                    d.attack -= p[variance].attack;
                    d.resist -= p[variance].resist;
                    d.speed -= p[variance].speed;
                    d.money += p[variance].sell;
                    vmsg(shortbuf);
                    variance = 0;
                }
                else
                {
                    sprintf(shortbuf, "ccc..我回心轉意了...");
                    vmsg(shortbuf);
                }
            }
            else if (variance == 0)
            {
                sprintf(shortbuf, "你本來就沒有裝備了...");
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
/* 打工選單:家事 苦工 家教 地攤                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static int pip_job_workA(void)
{
    /*  ├────┼──────────────────────┤*/
    /*  │家庭管理│待人接物 + N, 掃地洗衣 + N, 烹飪技巧 + N    │*/
    /*  │        │和父親的關係 + N, 疲勞 + 1, 感受 - 2        │*/
    /*  ├────┼──────────────────────┤*/
    /*  │家庭管理│若 體    力 - RND (疲勞) >=   5 則工作成功  │*/
    /*  ├────┼──────────────────────┤*/
    float class;
    long workmoney;

    workmoney = 0;
    class = ((d.hp * 100 / d.maxhp) - d.tired) * LEARN_LEVEL;
    d.maxhp += random() % 2 * LEARN_LEVEL;
    d.shit += random() % 3 + 5;
    count_tired(3, 7, "Y", 100, 1);
    d.hp -= (random() % 2 + 4);
    d.happy -= (random() % 3 + 4);
    d.satisfy -= random() % 3 + 4;
    d.affect -= 7 + random() % 7;
    if (d.affect <= 0)
        d.affect = 0;
    show_job_pic(11);
    if (class >= 75)
    {
        d.cookskill += random() % 2 + 7;
        d.homework += random() % 2 + 7;
        d.family += random() % 3 + 4;
        d.relation += random() % 3 + 4;
        workmoney = 80 + (d.cookskill * 2 + d.homework + d.family) / 40;
        vmsg("家事很成功\喔..多一點錢給你..");
    }
    else if (class < 75 && class >= 50)
    {
        d.cookskill += random() % 2 + 5;
        d.homework += random() % 2 + 5;
        d.family += random() % 3 + 3;
        d.relation += random() % 3 + 3;
        workmoney = 60 + (d.cookskill * 2 + d.homework + d.family) / 45;
        vmsg("家事還蠻順利的唷..嗯嗯..");
    }
    else if (class < 50 && class >= 25)
    {
        d.cookskill += random() % 3 + 3;
        d.homework += random() % 3 + 3;
        d.family += random() % 3 + 2;
        d.relation += random() % 3 + 2;
        workmoney = 40 + (d.cookskill * 2 + d.homework + d.family) / 50;
        vmsg("家事普普通通啦..可以更好的..加油..");
    }
    else if (class < 25)
    {
        d.cookskill += random() % 3 + 1;
        d.homework += random() % 3 + 1;
        d.family += random() % 3 + 1;
        d.relation += random() % 3 + 1;
        workmoney = 20 + (d.cookskill * 2 + d.homework + d.family) / 60;
        vmsg("家事很糟糕喔..這樣不行啦..");
    }
    d.money += workmoney * LEARN_LEVEL;
    d.workA += 1;
    return 0;
}

static int pip_job_workB(void)
{
    /*  ├────┼──────────────────────┤*/
    /*  │育幼院  │母性 + N, 感受 + 1, 魅力 - 1, 疲勞 + 3      │*/
    /*  ├────┼──────────────────────┤*/
    /*  │育幼院  │若 體    力 - RND (疲勞) >=  20 則工作成功  │*/
    /*  ├────┼──────────────────────┤*/
    float class;
    long workmoney;

    workmoney = 0;
    class = ((d.hp * 100 / d.maxhp) - d.tired) * LEARN_LEVEL;
    d.maxhp += (random() % 2 + 1) * LEARN_LEVEL;
    d.shit += random() % 3 + 5;
    d.affect += random() % 3 + 4;

    count_tired(3, 9, "Y", 100, 1);
    d.hp -= (random() % 3 + 6);
    d.happy -= (random() % 3 + 4);
    d.satisfy -= random() % 3 + 4;
    d.charm -= random() % 3 + 4;
    if (d.charm <= 0)
        d.charm = 0;
    show_job_pic(21);
    if (class >= 90)
    {
        d.love += random() % 2 + 7;
        d.toman += random() % 2 + 2;
        workmoney = 150 + (d.love + d.toman) / 50;
        vmsg("當保姆很成功\喔..下次再來喔..");
    }
    else if (class < 90 && class >= 75)
    {
        d.love += random() % 2 + 5;
        d.toman += random() % 2 + 2;
        workmoney = 120 + (d.love + d.toman) / 50;
        vmsg("保姆還當的不錯唷..嗯嗯..");
    }
    else if (class < 75 && class >= 50)
    {
        d.love += random() % 2 + 3;
        d.toman += random() % 2 + 1;
        workmoney = 100 + (d.love + d.toman) / 50;
        vmsg("小朋友很皮喔..加油..");
    }
    else if (class < 50)
    {
        d.love += random() % 2 + 1;
        d.toman += random() % 2 + 1;
        workmoney = 80 + (d.love + d.toman) / 50;
        vmsg("很糟糕喔..你罩不住小朋友耶...");
    }
    d.money += workmoney * LEARN_LEVEL;
    d.workB += 1;
    return 0;
}

static int pip_job_workC(void)
{
    /*  ├────┼──────────────────────┤*/
    /*  │旅館    │掃地洗衣 + N, 戰鬥技術 - N, 疲勞 + 2        │*/
    /*  ├────┼──────────────────────┤*/
    /*  │旅館    │若 體    力 - RND (疲勞) >=  30 則工作成功  │*/
    /*  ├────┼──────────────────────┤*/
    float class;
    long workmoney;

    workmoney = 0;
    class = ((d.hp * 100 / d.maxhp) - d.tired) * LEARN_LEVEL;
    d.maxhp += (random() % 2 + 2) * LEARN_LEVEL;
    d.shit += random() % 3 + 5;
    count_tired(5, 12, "Y", 100, 1);
    d.hp -= (random() % 4 + 8);
    d.happy -= (random() % 3 + 4);
    d.satisfy -= random() % 3 + 4;
    show_job_pic(31);
    if (class >= 95)
    {
        d.homework += random() % 2 + 7;
        d.family += random() % 2 + 4;
        d.hskill -= random() % 2 + 7;
        if (d.hskill < 0)
            d.hskill = 0;
        workmoney = 250 + (d.cookskill * 2 + d.homework * 2) / 40;
        vmsg("旅館事業蒸蒸日上..希望你再過來...");
    }
    else if (class < 95 && class >= 80)
    {
        d.homework += random() % 2 + 5;
        d.family += random() % 2 + 3;
        d.hskill -= random() % 2 + 5;
        if (d.hskill < 0)
            d.hskill = 0;
        workmoney = 200 + (d.cookskill * 2 + d.homework * 2) / 50;
        vmsg("旅館還蠻順利的唷..嗯嗯..");
    }
    else if (class < 80 && class >= 60)
    {
        d.homework += random() % 2 + 3;
        d.family += random() % 2 + 3;
        d.hskill -= random() % 2 + 5;
        if (d.hskill < 0)
            d.hskill = 0;
        workmoney = 150 + (d.cookskill * 2 + d.homework * 2) / 50;
        vmsg("普普通通啦..可以更好的..加油..");
    }
    else if (class < 60)
    {
        d.homework += random() % 2 + 1;
        d.family += random() % 2 + 1;
        d.hskill -= random() % 2 + 1;
        if (d.hskill < 0)
            d.hskill = 0;
        workmoney = 100 + (d.cookskill * 2 + d.homework * 2) / 50;
        vmsg("這個很糟糕喔..你這樣不行啦..");
    }
    d.money += workmoney * LEARN_LEVEL;
    d.workC += 1;
    return 0;
}

static int pip_job_workD(void)
{
    /*  ├────┼──────────────────────┤*/
    /*  │農場    │體力 + 1, 腕力 + 1, 氣質 - 1, 疲勞 + 3      │*/
    /*  ├────┼──────────────────────┤*/
    /*  │農場    │若 體    力 - RND (疲勞) >=  30 則工作成功  │*/
    /*  ├────┼──────────────────────┤*/
    float class;
    long workmoney;

    workmoney = 0;
    class = ((d.hp * 100 / d.maxhp) - d.tired) * LEARN_LEVEL;
    d.maxhp += (random() % 3 + 2) * LEARN_LEVEL;
    d.wrist += random() % 2 + 2;
    d.shit += random() % 5 + 10;
    count_tired(5, 15, "Y", 100, 1);
    d.hp -= (random() % 4 + 10);
    d.happy -= (random() % 3 + 4);
    d.satisfy -= random() % 3 + 4;
    d.character -= random() % 3 + 4;
    if (d.character < 0)
        d.character = 0;
    show_job_pic(41);
    if (class >= 95)
    {
        workmoney = 250 + (d.wrist * 2 + d.hp * 2) / 80;
        vmsg("牛羊長的好好喔..希望你再來幫忙...");
    }
    else if (class < 95 && class >= 80)
    {
        workmoney = 210 + (d.wrist * 2 + d.hp * 2) / 80;
        vmsg("呵呵..還不錯喔..:)");
    }
    else if (class < 80 && class >= 60)
    {
        workmoney = 160 + (d.wrist * 2 + d.hp * 2) / 80;
        vmsg("普普通通啦..可以更好的..");
    }
    else if (class < 60)
    {
        workmoney = 120 + (d.wrist * 2 + d.hp * 2) / 80;
        vmsg("你不太適合農場的工作  -_-...");
    }
    d.money += workmoney * LEARN_LEVEL;
    d.workD += 1;
    return 0;
}

static int pip_job_workE(void)
{
    /*  ├────┼──────────────────────┤*/
    /*  │餐廳    │料理 + N, 戰鬥技術 - N, 疲勞 + 2            │*/
    /*  ├────┼──────────────────────┤*/
    /*  │餐廳    │若 烹飪技術 - RND (疲勞) >=  50 則工作成功  │*/
    /*  ├────┼──────────────────────┤*/
    float class;
    long workmoney;

    workmoney = 0;
    class = (d.cookskill - d.tired) * LEARN_LEVEL;
    d.maxhp += (random() % 2 + 1) * LEARN_LEVEL;
    d.shit += random() % 4 + 12;
    count_tired(5, 9, "Y", 100, 1);
    d.hp -= (random() % 4 + 8);
    d.happy -= (random() % 3 + 4);
    d.satisfy -= random() % 3 + 4;
    show_job_pic(51);
    if (class >= 80)
    {
        d.homework += random() % 2 + 1;
        d.family += random() % 2 + 1;
        d.hskill -= random() % 2 + 5;
        if (d.hskill < 0)
            d.hskill = 0;
        d.cookskill += random() % 2 + 6;
        workmoney = 250 + (d.cookskill * 2 + d.homework * 2 + d.family * 2) / 80;
        vmsg("客人都說太好吃了..再來一盤吧...");
    }
    else if (class < 80 && class >= 60)
    {
        d.homework += random() % 2 + 1;
        d.family += random() % 2 + 1;
        d.hskill -= random() % 2 + 5;
        if (d.hskill < 0)
            d.hskill = 0;
        d.cookskill += random() % 2 + 4;
        workmoney = 200 + (d.cookskill * 2 + d.homework * 2 + d.family * 2) / 80;
        vmsg("煮的還不錯吃唷..:)");
    }
    else if (class < 60 && class >= 30)
    {
        d.homework += random() % 2 + 1;
        d.family += random() % 2 + 1;
        d.hskill -= random() % 2 + 5;
        if (d.hskill < 0)
            d.hskill = 0;
        d.cookskill += random() % 2 + 2;
        workmoney = 150 + (d.cookskill * 2 + d.homework * 2 + d.family * 2) / 80;
        vmsg("普普通通啦..可以更好的..");
    }
    else if (class < 30)
    {
        d.homework += random() % 2 + 1;
        d.family += random() % 2 + 1;
        d.hskill -= random() % 2 + 5;
        if (d.hskill < 0)
            d.hskill = 0;
        d.cookskill += random() % 2 + 1;
        workmoney = 100 + (d.cookskill * 2 + d.homework * 2 + d.family * 2) / 80;
        vmsg("你的廚藝待加強喔...");
    }
    d.money += workmoney * LEARN_LEVEL;
    d.workE += 1;
    return 0;
}

static int pip_job_workF(void)
{
    /*  ├────┼──────────────────────┤*/
    /*  │教堂    │信仰 + 2, 道德 + 1, 罪孽 - 2, 疲勞 + 1      │*/
    /*  ├────┼──────────────────────┤*/
    float class;
    long workmoney;

    workmoney = 0;
    class = ((d.hp * 100 / d.maxhp) - d.tired) * LEARN_LEVEL;
    count_tired(5, 7, "Y", 100, 1);
    d.love += (random() % 3 + 4) * LEARN_LEVEL;
    d.belief += (random() % 4 + 7) * LEARN_LEVEL;
    d.ethics += (random() % 3 + 7) * LEARN_LEVEL;
    d.shit += random() % 3 + 3;
    d.hp -= random() % 3 + 5;
    d.offense -= random() % 4 + 7;
    if (d.offense < 0)
        d.offense = 0;
    show_job_pic(61);
    if (class >= 75)
    {
        workmoney = 100 + (d.belief + d.ethics - d.offense) / 20;
        vmsg("錢很少 但看你這麼認真 給你多一點...");
    }
    else if (class < 75 && class >= 50)
    {
        workmoney = 75 + (d.belief + d.ethics - d.offense) / 20;
        vmsg("謝謝你的熱心幫忙..:)");
    }
    else if (class < 50 && class >= 25)
    {
        workmoney = 50 + (d.belief + d.ethics - d.offense) / 20;
        vmsg("你真的很有愛心啦..不過有點小累的樣子...");
    }
    else if (class < 25)
    {
        workmoney = 25 + (d.belief + d.ethics - d.offense) / 20;
        vmsg("來奉獻不錯..但也不能打混ㄚ....:(");
    }
    d.money += workmoney * LEARN_LEVEL;
    d.workF += 1;
    return 0;
}

static int pip_job_workG(void)
{
    /* ├────┼──────────────────────┤*/
    /* │地攤    │體力 + 2, 魅力 + 1, 疲勞 + 3, 談吐 +1       │*/
    /* ├────┼──────────────────────┤*/
    long workmoney;

    workmoney = 0;
    workmoney = 200 + (d.charm * 3 + d.speech * 2 + d.toman) / 50;
    count_tired(3, 12, "Y", 100, 1);
    d.shit += random() % 3 + 8;
    d.speed += (random() % 2) * LEARN_LEVEL;
    d.weight -= random() % 2;
    d.happy -= (random() % 3 + 7);
    d.satisfy -= random() % 3 + 5;
    d.hp -= (random() % 6 + 6);
    d.charm += (random() % 2 + 3) * LEARN_LEVEL;
    d.speech += (random() % 2 + 3) * LEARN_LEVEL;
    d.toman += (random() % 2 + 3) * LEARN_LEVEL;
    move(4, 0);
    show_job_pic(71);
    vmsg("擺\地攤要躲警察啦..:p");
    d.money += workmoney * LEARN_LEVEL;
    d.workG += 1;
    return 0;
}

static int pip_job_workH(void)
{
    /*  ├────┼──────────────────────┤*/
    /*  │伐木場  │腕力 + 2, 氣質 - 2, 疲勞 + 4                │*/
    /*  ├────┼──────────────────────┤*/
    /*  │伐木場  │若 腕    力 - RND (疲勞) >=  80 則工作成功  │*/
    /*  ├────┼──────────────────────┤*/
    float class;
    long workmoney;

    if ((d.bbtime / 60 / 30) < 1) /*一歲才行*/
    {
        vmsg("小雞太小了，一歲以後再來吧...");
        return 0;
    }
    workmoney = 0;
    class = (d.wrist - d.tired) * LEARN_LEVEL;
    d.maxhp += (random() % 2 + 3) * LEARN_LEVEL;
    d.shit += random() % 7 + 15;
    d.wrist += (random() % 3 + 4) * LEARN_LEVEL;
    count_tired(5, 15, "Y", 100, 1);
    d.hp -= (random() % 4 + 10);
    d.happy -= (random() % 3 + 4);
    d.satisfy -= random() % 3 + 4;
    d.character -= random() % 3 + 7;
    if (d.character < 0)
        d.character = 0;
    show_job_pic(81);
    if (class >= 70)
    {
        workmoney = 350 + d.wrist / 20 + d.maxhp / 80;
        vmsg("你腕力很好唷..:)");
    }
    else if (class < 70 && class >= 50)
    {
        workmoney = 300 + d.wrist / 20 + d.maxhp / 80;
        vmsg("砍了不少樹喔.....:)");
    }
    else if (class < 50 && class >= 20)
    {
        workmoney = 250 + d.wrist / 20 + d.maxhp / 80;
        vmsg("普普通通啦..可以更好的..");
    }
    else if (class < 20)
    {
        workmoney = 200 + d.wrist / 20 + d.maxhp / 80;
        vmsg("待加強喔..鍛鍊再來吧....");
    }
    d.money += workmoney * LEARN_LEVEL;
    d.workH += 1;
    return 0;
}

static int pip_job_workI(void)
{
    /*  ├────┼──────────────────────┤*/
    /*  │美容院  │感受 + 1, 腕力 - 1, 疲勞 + 3                │*/
    /*  ├────┼──────────────────────┤*/
    /*  │美容院  │若 藝術修養 - RND (疲勞) >=  40 則工作成功  │*/
    /*  ├────┼──────────────────────┤*/
    float class;
    long workmoney;

    if ((d.bbtime / 60 / 30) < 1) /*一歲才行*/
    {
        vmsg("小雞太小了，一歲以後再來吧...");
        return 0;
    }
    workmoney = 0;
    class = (d.art - d.tired) * LEARN_LEVEL;
    d.maxhp += (random() % 2) * LEARN_LEVEL;
    d.affect += (random() % 2 + 3) * LEARN_LEVEL;
    count_tired(3, 11, "Y", 100, 1);
    d.shit += random() % 4 + 8;
    d.hp -= (random() % 4 + 10);
    d.happy -= (random() % 3 + 4);
    d.satisfy -= random() % 3 + 4;
    d.wrist -= random() % + 3;
    if (d.wrist < 0)
        d.wrist = 0;
    /*show_job_pic(4);*/
    if (class >= 80)
    {
        workmoney = 400 + d.art / 10 + d.affect / 20;
        vmsg("客人都很喜歡讓你做造型唷..:)");
    }
    else if (class < 80 && class >= 60)
    {
        workmoney = 360 + d.art / 10 + d.affect / 20;
        vmsg("做的不錯喔..頗有天份...:)");
    }
    else if (class < 60 && class >= 40)
    {
        workmoney = 320 + d.art / 10 + d.affect / 20;
        vmsg("馬馬虎虎啦..再加油一點..");
    }
    else if (class < 40)
    {
        workmoney = 250 + d.art / 10 + d.affect / 20;
        vmsg("待加強喔..以後再來吧....");
    }
    d.money += workmoney * LEARN_LEVEL;
    d.workI += 1;
    return 0;
}

static int pip_job_workJ(void)
{
    /*  ├────┼──────────────────────┤*/
    /*  │狩獵區  │體力 + 1, 氣質 - 1, 母性 - 1, 疲勞 + 3      │*/
    /*  │        │戰鬥技術 + N                                │*/
    /*  ├────┼──────────────────────┤*/
    /*  │狩獵區  │若 體    力 - RND (疲勞) >=  80 ＆          │*/
    /*  │        │若 智    力 - RND (疲勞) >=  40 則工作成功  │*/
    /*  ├────┼──────────────────────┤*/
    float class;
    float class1;
    long workmoney;

    /*兩歲以上才行*/
    if ((d.bbtime / 60 / 30) < 2)
    {
        vmsg("小雞太小了，兩歲以後再來吧...");
        return 0;
    }
    workmoney = 0;
    class = ((d.hp * 100 / d.maxhp) - d.tired) * LEARN_LEVEL;
    class1 = (d.wisdom - d.tired) * LEARN_LEVEL;
    count_tired(5, 15, "Y", 100, 1);
    d.shit += random() % 4 + 13;
    d.weight -= (random() % 2 + 1);
    d.maxhp += (random() % 2 + 3) * LEARN_LEVEL;
    d.speed += (random() % 2 + 3) * LEARN_LEVEL;
    d.hp -= (random() % 6 + 8);
    d.character -= random() % 3 + 4;
    d.happy -= random() % 5 + 8;
    d.satisfy -= random() % 5 + 6;
    d.love -= random() % 3 + 4;
    if (d.character < 0)
        d.character = 0;
    if (d.love < 0)
        d.love = 0;
    move(4, 0);
    show_job_pic(101);
    if (class >= 80 && class1 >= 80)
    {
        d.hskill += random() % 2 + 7;
        workmoney = 300 + d.maxhp / 50 + d.hskill / 20;
        vmsg("你是完美的獵人..");
    }
    else if ((class < 75 && class >= 50) && class1 >= 60)
    {
        d.hskill += random() % 2 + 5;
        workmoney = 270 + d.maxhp / 45 + d.hskill / 20;
        vmsg("收獲還不錯喔..可以飽餐\一頓了..:)");
    }
    else if ((class < 50 && class >= 25) && class1 >= 40)
    {
        d.hskill += random() % 2 + 3;
        workmoney = 240 + d.maxhp / 40 + d.hskill / 20;
        vmsg("技術差強人意  再加油喔..");
    }
    else if ((class < 25 && class >= 0) && class1 >= 20)
    {
        d.hskill += random() % 2 + 1;
        workmoney = 210 + d.maxhp / 30 + d.hskill / 20;
        vmsg("狩獵是體力與智力的結合....");
    }
    else if (class < 0)
    {
        d.hskill += random() % 2;
        workmoney = 190 + d.hskill / 20;
        vmsg("要多多鍛鍊和增進智慧啦....");
    }
    d.money += workmoney * LEARN_LEVEL;
    d.workJ += 1;
    return 0;
}

static int pip_job_workK(void)
{
    /* ├────┼──────────────────────┤*/
    /* │工地    │體力 + 2, 魅力 - 1, 疲勞 + 3                │*/
    /* ├────┼──────────────────────┤*/
    float class;
    long workmoney;

    /*兩歲以上才行*/
    if ((d.bbtime / 60 / 30) < 2)
    {
        vmsg("小雞太小了，兩歲以後再來吧...");
        return 0;
    }
    workmoney = 0;
    class = ((d.hp * 100 / d.maxhp) - d.tired) * LEARN_LEVEL;
    count_tired(5, 15, "Y", 100, 1);
    d.shit += random() % 4 + 16;
    d.weight -= (random() % 2 + 2);
    d.maxhp += (random() % 2 + 1) * LEARN_LEVEL;
    d.speed += (random() % 2 + 2) * LEARN_LEVEL;
    d.hp -= (random() % 6 + 10);
    d.charm -= random() % 3 + 6;
    d.happy -= (random() % 5 + 10);
    d.satisfy -= random() % 5 + 6;
    if (d.charm < 0)
        d.charm = 0;
    move(4, 0);
    show_job_pic(111);
    if (class >= 75)
    {
        workmoney = 250 + d.maxhp / 50;
        vmsg("工程很完美  謝謝你了..");
    }
    else if (class < 75 && class >= 50)
    {
        workmoney = 220 + d.maxhp / 45;
        vmsg("工程尚稱順利  辛苦你了..");
    }
    else if (class < 50 && class >= 25)
    {
        workmoney = 200 + d.maxhp / 40;
        vmsg("工程差強人意  再加油喔..");
    }
    else if (class < 25 && class >= 0)
    {
        workmoney = 180 + d.maxhp / 30;
        vmsg("ㄜ  待加強待加強....");
    }
    else
    {
        workmoney = 160;
        vmsg("下次體力好一點..疲勞度低一點再來....");
    }

    d.money += workmoney * LEARN_LEVEL;
    d.workK += 1;
    return 0;
}

static int pip_job_workL(void)
{
    /*  ├────┼──────────────────────┤*/
    /*  │墓園    │抗魔能力 + N, 感受 + 1, 魅力 - 1            │*/
    /*  │        │疲勞 + 2                                    │*/
    /*  ├────┼──────────────────────┤*/
    float class;
    float class1;
    long workmoney;

    /*三歲才行*/
    if ((d.bbtime / 60 / 30) < 3)
    {
        vmsg("小雞現在還太小了，三歲以後再來吧...");
        return 0;
    }
    workmoney = 0;
    class = ((d.hp * 100 / d.maxhp) - d.tired) * LEARN_LEVEL;
    class1 = (d.belief - d.tired) * LEARN_LEVEL;
    d.shit += random() % 5 + 8;
    d.maxmp += (random() % 2) * LEARN_LEVEL;
    d.affect += (random() % 2 + 2) * LEARN_LEVEL;
    d.brave += (random() % 2 + 2) * LEARN_LEVEL;
    count_tired(5, 12, "Y", 100, 1);
    d.hp -= (random() % 3 + 7);
    d.happy -= (random() % 4 + 6);
    d.satisfy -= random() % 3 + 5;
    d.charm -= random() % 3 + 6;
    if (d.charm < 0)
        d.charm = 0;
    show_job_pic(121);
    if (class >= 75 && class1 >= 75)
    {
        d.mresist += random() % 2 + 7;
        workmoney = 200 + (d.affect + d.brave) / 40;
        vmsg("守墓成功\喔  給你多點錢");
    }
    else if ((class < 75 && class >= 50) && class1 >= 50)
    {
        d.mresist += random() % 2 + 5;
        workmoney = 150 + (d.affect + d.brave) / 50;
        vmsg("守墓還算成功\喔..謝啦..");
    }
    else if ((class < 50 && class >= 25) && class1 >= 25)
    {
        d.mresist += random() % 2 + 3;
        workmoney = 120 + (d.affect + d.brave) / 60;
        vmsg("守墓還算差強人意喔..加油..");
    }
    else
    {
        d.mresist += random() % 2 + 1;
        workmoney = 80 + (d.affect + d.brave) / 70;
        vmsg("我也不方便說啥了..請再加油..");
    }

    d.money += workmoney * LEARN_LEVEL;
    d.workL += 1;
    return 0;
}

static int pip_job_workM(void)
{
    /*  ├────┼──────────────────────┤*/
    /*  │家庭教師│道德 + 1, 母性 + N, 魅力 - 1, 疲勞 + 7      │*/
    /*  ├────┼──────────────────────┤*/
    float class;
    long workmoney;

    if ((d.bbtime / 60 / 30) < 4)
    {
        vmsg("小雞太小了，四歲以後再來吧...");
        return 0;
    }
    workmoney = 0;
    class = ((d.hp * 100 / d.maxhp) - d.tired) * LEARN_LEVEL;
    workmoney = 50 + d.wisdom / 20 + d.character / 20;
    count_tired(5, 10, "Y", 100, 1);
    d.shit += random() % 3 + 8;
    d.character += (random() % 2) * LEARN_LEVEL;
    d.wisdom += (random() % 2) * LEARN_LEVEL;
    d.happy -= (random() % 3 + 6);
    d.satisfy -= random() % 3 + 5;
    d.hp -= (random() % 3 + 8);
    d.money += workmoney * LEARN_LEVEL;
    move(4, 0);
    show_job_pic(131);
    vmsg("家教輕鬆 當然錢就少一點囉");
    d.workM += 1;
    return 0;
}

static int pip_job_workN(void)
{
    /*  ├────┼──────────────────────┤*/
    /*  │酒店    │烹飪技巧 + N, 談話技巧 + N, 智力 - 2        │*/
    /*  │        │疲勞 + 5                                    │*/
    /*  ├────┼──────────────────────┤*/
    /*  │酒店    │若 體    力 - RND (疲勞) >=  60 ＆          │*/
    /*  │        │若 魅    力 - RND (疲勞) >=  50 則工作成功  │*/
    /*  ├────┼──────────────────────┤*/
    float class;
    float class1;
    long workmoney;

    if ((d.bbtime / 60 / 30) < 5)
    {
        vmsg("小雞太小了，五歲以後再來吧...");
        return 0;
    }
    workmoney = 0;
    class = ((d.hp * 100 / d.maxhp) - d.tired) * LEARN_LEVEL;
    class1 = (d.charm - d.tired) * LEARN_LEVEL;
    d.shit += random() % 5 + 5;
    count_tired(5, 14, "Y", 100, 1);
    d.hp -= (random() % 3 + 5);
    d.social -= random() % 5 + 6;
    d.happy -= (random() % 4 + 6);
    d.satisfy -= random() % 3 + 5;
    d.wisdom -= random() % 3 + 4;
    if (d.wisdom < 0)
        d.wisdom = 0;
    /*show_job_pic(6);*/
    if (class >= 75 && class1 >= 75)
    {
        d.cookskill += random() % 2 + 7;
        d.speech += random() % 2 + 5;
        workmoney = 500 + (d.charm) / 5;
        vmsg("你很紅唷  :)");
    }
    else if ((class < 75 && class >= 50) && class1 >= 50)
    {
        d.cookskill += random() % 2 + 5;
        d.speech += random() % 2 + 5;
        workmoney = 400 + (d.charm) / 5;
        vmsg("蠻受歡迎的耶....");
    }
    else if ((class < 50 && class >= 25) && class1 >= 25)
    {
        d.cookskill += random() % 2 + 4;
        d.speech += random() % 2 + 3;
        workmoney = 300 + (d.charm) / 5;
        vmsg("很平凡啦..但馬馬虎虎...");
    }
    else
    {
        d.cookskill += random() % 2 + 2;
        d.speech += random() % 2 + 2;
        workmoney = 200 + (d.charm) / 5;
        vmsg("你的魅力不夠啦..請加油....");
    }
    d.money += workmoney * LEARN_LEVEL;
    d.workN += 1;
    return 0;
}

static int pip_job_workO(void)
{
    /*  ├────┼──────────────────────┤*/
    /*  │酒家    │魅力 + 2, 罪孽 + 2, 道德 - 3, 信仰 - 3      │*/
    /*  │        │待人接物 - N, 和父親的關係 - N, 疲勞 + 12   │*/
    /*  ├────┼──────────────────────┤*/
    /*  │酒家    │若 魅    力 - RND (疲勞) >=  70 則工作成功  │*/
    /*  ├────┼──────────────────────┤*/
    float class;
    long workmoney;

    if ((d.bbtime / 60 / 30) < 4)
    {
        vmsg("小雞太小了，四歲以後再來吧...");
        return 0;
    }
    workmoney = 0;
    class = (d.charm - d.tired) * LEARN_LEVEL;
    d.shit += random() % 5 + 14;
    d.charm += (random() % 3 + 8) * LEARN_LEVEL;
    d.offense += (random() % 3 + 8) * LEARN_LEVEL;
    count_tired(5, 22, "Y", 100, 1);
    d.hp -= (random() % 3 + 8);
    d.social -= random() % 6 + 12;
    d.happy -= (random() % 4 + 8);
    d.satisfy -= random() % 3 + 8;
    d.ethics -= random() % 6 + 10;
    d.belief -= random() % 6 + 10;
    if (d.ethics < 0)
        d.ethics = 0;
    if (d.belief < 0)
        d.belief = 0;

    /*show_job_pic(6);*/
    if (class >= 75)
    {
        d.relation -= random() % 5 + 12;
        d.toman -= random() % 5 + 12;
        workmoney = 600 + (d.charm) / 5;
        vmsg("你是本店的紅牌唷  :)");
    }
    else if (class < 75 && class >= 50)
    {
        d.relation -= random() % 5 + 8;
        d.toman -= random() % 5 + 8;
        workmoney = 500 + (d.charm) / 5;
        vmsg("你蠻受歡迎的耶..:)");
    }
    else if (class < 50 && class >= 25)
    {
        d.relation -= random() % 5 + 5;
        d.toman -= random() % 5 + 5;
        workmoney = 400 + (d.charm) / 5;
        vmsg("你很平凡..但馬馬虎虎啦...");
    }
    else
    {
        d.relation -= random() % 5 + 1;
        d.toman -= random() % 5 + 1;
        workmoney = 300 + (d.charm) / 5;
        vmsg("唉..你的魅力不夠啦....");
    }
    d.money += workmoney * LEARN_LEVEL;
    if (d.relation < 0)
        d.relation = 0;
    if (d.toman < 0)
        d.toman = 0;
    d.workO += 1;
    return 0;
}

static int pip_job_workP(void)
{
    /*  ├────┼──────────────────────┤*/
    /*  │大夜總會│魅力 + 3, 罪孽 + 1, 氣質 - 2, 智力 - 1      │*/
    /*  │        │待人接物 - N, 疲勞 + 8                      │*/
    /*  ├────┼──────────────────────┤*/
    /*  │大夜總會│若 魅    力 - RND (疲勞) >=  70 ＆          │*/
    /*  │        │若 藝術修養 - RND (疲勞) >=  30 則工作成功  │*/
    /*  └────┴──────────────────────┘*/
    float class;
    float class1;
    long workmoney;

    if ((d.bbtime / 60 / 30) < 6)
    {
        vmsg("小雞太小了，六歲以後再來吧...");
        return 0;
    }
    workmoney = 0;
    class = (d.charm - d.tired) * LEARN_LEVEL;
    class1 = (d.art - d.tired) * LEARN_LEVEL;
    d.shit += random() % 5 + 7;
    d.charm += (random() % 3 + 8) * LEARN_LEVEL;
    d.offense += (random() % 3 + 8) * LEARN_LEVEL;
    count_tired(5, 22, "Y", 100, 1);
    d.hp -= (random() % 3 + 8);
    d.social -= random() % 6 + 12;
    d.happy -= (random() % 4 + 8);
    d.satisfy -= random() % 3 + 8;
    d.character -= random() % 3 + 8;
    d.wisdom -= random() % 3 + 5;
    if (d.character < 0)
        d.character = 0;
    if (d.wisdom < 0)
        d.wisdom = 0;
    /*show_job_pic(6);*/
    if (class >= 75 && class1 > 30)
    {
        d.speech += random() % 5 + 12;
        d.toman -= random() % 5 + 12;
        workmoney = 1000 + (d.charm) / 5;
        vmsg("你是夜總會最閃亮的星星唷  :)");
    }
    else if ((class < 75 && class >= 50) && class1 > 20)
    {
        d.speech += random() % 5 + 8;
        d.toman -= random() % 5 + 8;
        workmoney = 800 + (d.charm) / 5;
        vmsg("嗯嗯..你蠻受歡迎的耶..:)");
    }
    else if ((class < 50 && class >= 25) && class1 > 10)
    {
        d.speech += random() % 5 + 5;
        d.toman -= random() % 5 + 5;
        workmoney = 600 + (d.charm) / 5;
        vmsg("你要加油了啦..但普普啦...");
    }
    else
    {
        d.speech += random() % 5 + 1;
        d.toman -= random() % 5 + 1;
        workmoney = 400 + (d.charm) / 5;
        vmsg("唉..你不行啦....");
    }
    d.money += workmoney * LEARN_LEVEL;
    if (d.toman < 0)
        d.toman = 0;
    d.workP += 1;
    return 1;
}

/*---------------------------------------------------------------------------*/
/* 玩樂選單:散步 旅遊 運動 約會 猜拳                                         */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static int pip_play_stroll(void)       /*散步*/
{
    int lucky;
    count_tired(3, 3, "Y", 100, 0);
    lucky = random() % 7;
    if (lucky == 2)
    {
        d.happy += random() % 3 + random() % 3 + 9;
        d.satisfy += random() % 3 + random() % 3 + 3;
        d.shit += random() % 3 + 3;
        d.hp -= (random() % 3 + 5);
        move(4, 0);
        if (random() % 2 > 0)
            show_play_pic(1);
        else
            show_play_pic(2);
        vmsg("遇到朋友囉  真好.... ^_^");
    }
    else if (lucky == 3)
    {
        d.money += 100;
        d.happy += random() % 3 + 6;
        d.satisfy += random() % 3 + 4;
        d.shit += random() % 3 + 3;
        d.hp -= (random() % 3 + 4);
        move(4, 0);
        show_play_pic(3);
        vmsg("撿到了100元了..耶耶耶....");
    }

    else if (lucky == 4)
    {
        if (random() % 2 > 0)
        {
            d.happy -= (random() % 2 + 5);
            move(4, 0);
            d.hp -= (random() % 3 + 3);
            show_play_pic(4);
            if (d.money >= 50)
            {
                d.money -= 50;
                vmsg("掉了50元了..嗚嗚嗚....");
            }
            else
            {
                d.money = 0;
                d.hp -= (random() % 3 + 3);
                vmsg("錢掉光光了..嗚嗚嗚....");
            }
            d.shit += random() % 3 + 2;
        }
        else
        {
            d.happy += random() % 3 + 5;
            move(4, 0);
            show_play_pic(5);
            if (d.money >= 50)
            {
                d.money -= 50;
                d.hp -= (random() % 3 + 3);
                vmsg("用了50元了..不可以罵我喔....");
            }
            else
            {
                d.money = 0;
                d.hp -= (random() % 3 + 3);
                vmsg("錢被我偷用光光了..:p");
            }
            d.shit += random() % 3 + 2;
        }
    }
    else if (lucky == 5)
    {
        d.happy += random() % 3 + 6;
        d.satisfy += random() % 3 + 5;
        d.shit += 2;
        move(4, 0);
        if (random() % 2 > 0)
            show_play_pic(6);
        else
            show_play_pic(7);
        vmsg("好棒喔撿到玩具了說.....");
    }
    else if (lucky == 6)
    {
        d.happy -= (random() % 3 + 10);
        d.shit += (random() % 3 + 20);
        move(4, 0);
        show_play_pic(9);
        vmsg("真是倒楣  可以去買愛國獎券");
    }
    else
    {
        d.happy += random() % 3 + 3;
        d.satisfy += random() % 2 + 1;
        d.shit += random() % 3 + 2;
        d.hp -= (random() % 3 + 2);
        move(4, 0);
        show_play_pic(8);
        vmsg("沒有特別的事發生啦.....");
    }
    return 0;
}

static int pip_play_sport(void)        /*運動*/
{
    count_tired(3, 8, "Y", 100, 1);
    d.weight -= (random() % 3 + 2);
    d.satisfy += random() % 2 + 3;
    if (d.satisfy > 100)
        d.satisfy = 100;
    d.shit += random() % 5 + 10;
    d.hp -= (random() % 2 + 8);
    d.maxhp += random() % 2;
    d.speed += (2 + random() % 3);
    move(4, 0);
    show_play_pic(10);
    vmsg("運動好處多多啦...");
    return 0;
}

static int pip_play_date(void) /*約會*/
{
    if (d.money < 150)
    {
        vmsg("你錢不夠多啦! 約會總得花點錢錢");
    }
    else
    {
        count_tired(3, 6, "Y", 100, 1);
        d.happy += random() % 5 + 12;
        d.shit += random() % 3 + 5;
        d.hp -= random() % 4 + 8;
        d.satisfy += random() % 5 + 7;
        d.character += random() % 3 + 1;
        d.money = d.money - 150;
        move(4, 0);
        show_play_pic(11);
        vmsg("約會去  呼呼");
    }
    return 0;
}
static int pip_play_outing(void)       /*郊遊*/
{
    int lucky;
    char buf[256];

    if (d.money < 250)
    {
        vmsg("你錢不夠多啦! 旅遊總得花點錢錢");
    }
    else
    {
        d.weight += random() % 2 + 1;
        d.money -= 250;
        count_tired(10, 45, "N", 100, 0);
        d.hp -= random() % 10 + 20;
        if (d.hp >= d.maxhp)
            d.hp = d.maxhp;
        d.happy += random() % 10 + 12;
        d.character += random() % 5 + 5;
        d.satisfy += random() % 10 + 10;
        lucky = random() % 4;
        if (lucky == 0)
        {
            d.maxmp += random() % 3;
            d.art += random() % 2;
            show_play_pic(12);
            if (random() % 2 > 0)
                vmsg("心中有一股淡淡的感覺  好舒服喔....");
            else
                vmsg("雲水 閑情 心情好多了.....");
        }
        else if (lucky == 1)
        {
            d.art += random() % 3;
            d.maxmp += random() % 2;
            show_play_pic(13);
            if (random() % 2 > 0)
                vmsg("有山有水有落日  形成一幅美麗的畫..");
            else
                vmsg("看著看著  全身疲憊都不見囉..");
        }
        else if (lucky == 2)
        {
            d.love += random() % 3;
            show_play_pic(14);
            if (random() % 2 > 0)
                vmsg("看  太陽快沒入水中囉...");
            else
                vmsg("聽說這是海邊啦  你說呢?");
        }
        else if (lucky == 3)
        {
            d.maxhp += random() % 3;
            show_play_pic(15);
            if (random() % 2 > 0)
                vmsg("讓我們瘋狂在夜裡的海灘吧....呼呼..");
            else
                vmsg("涼爽的海風迎面襲來  最喜歡這種感覺了....");
        }
        if ((random() % 301 + random() % 200) % 100 == 12)
        {
            lucky = 0;
            clear();
            sprintf(buf, "\x1b[1;41m  " NICKNAME PIPNAME " ～ %-10s                                                  \x1b[0m", d.name);
            show_play_pic(0);
            move(b_lines - 6, (d_cols>>1) + 10);
            prints("\x1b[1;36m親愛的 \x1b[1;33m%s ～\x1b[0m", d.name);
            move(b_lines - 5, (d_cols>>1) + 10);
            outs("\x1b[1;37m看到你這樣努力的培養自己的能力  讓我心中十分的高興喔..\x1b[m");
            move(b_lines - 4, (d_cols>>1) + 10);
            outs("\x1b[1;36m小天使我決定給你獎賞鼓勵鼓勵  偷偷地幫助你一下....^_^\x1b[0m");
            move(b_lines - 3, (d_cols>>1) + 10);
            lucky = random() % 7;
            if (lucky == 6)
            {
                outs("\x1b[1;33m我將幫你的各項能力全部提升百分之五喔......\x1b[0m");
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
                outs("\x1b[1;33m我將幫你的戰鬥能力全部提升百分之十喔.......\x1b[0m");
                d.attack = d.attack * 110 / 100;
                d.resist = d.resist * 110 / 100;
                d.speed = d.speed * 110 / 100;
                d.brave = d.brave * 110 / 100;
            }

            else if (lucky <= 3 && lucky >= 2)
            {
                outs("\x1b[1;33m我將幫你的魔法能力和生命力全部提升百分之十喔.......\x1b[0m");
                d.maxhp = d.maxhp * 110 / 100;
                d.hp = d.maxhp;
                d.maxmp = d.maxmp * 110 / 100;
                d.mp = d.maxmp;
            }
            else if (lucky <= 1 && lucky >= 0)
            {
                outs("\x1b[1;33m我將幫你的感受能力全部提升百分之二十喔....\x1b[0m");
                d.character = d.character * 110 / 100;
                d.love = d.love * 110 / 100;
                d.wisdom = d.wisdom * 110 / 100;
                d.art = d.art * 110 / 100;
                d.homework = d.homework * 110 / 100;
            }

            vmsg("請繼續加油喔...");
        }
    }
    return 0;
}

static int pip_play_kite(void) /*風箏*/
{
    count_tired(4, 4, "Y", 100, 0);
    d.weight += (random() % 2 + 2);
    d.satisfy += random() % 3 + 12;
    if (d.satisfy > 100)
        d.satisfy = 100;
    d.happy += random() % 5 + 10;
    d.shit += random() % 5 + 6;
    d.hp -= (random() % 2 + 7);
    d.affect += random() % 4 + 6;
    move(4, 0);
    show_play_pic(16);
    vmsg("放風箏真好玩啦...");
    return 0;
}

static int pip_play_KTV(void)  /*KTV*/
{
    if (d.money < 250)
    {
        vmsg("你錢不夠多啦! 唱歌總得花點錢錢");
    }
    else
    {
        count_tired(10, 10, "Y", 100, 0);
        d.satisfy += random() % 2 + 20;
        if (d.satisfy > 100)
            d.satisfy = 100;
        d.happy += random() % 3 + 20;
        d.shit += random() % 5 + 6;
        d.money -= 250;
        d.hp += (random() % 2 + 6);
        d.art += random() % 4 + 3;
        move(4, 0);
        show_play_pic(17);
        vmsg("你說你  想要逃...");
    }
    return 0;
}

static int pip_play_guess(void)   /* 猜拳程式 */
{
    int com;
    int pipkey;
    struct tm *qtime;
    time_t now;

    time(&now);
    qtime = localtime(&now);
    d.satisfy += (random() % 3 + 2);
    count_tired(2, 2, "Y", 100, 1);
    d.shit += random() % 3 + 2;
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
        prints("\x1b[1;44;37m  猜拳選單  \x1b[46m[1]我出剪刀 [2]我出石頭 [3]我出布啦 [4]猜拳記錄 [Q]跳出           %*s\x1b[m", d_cols, "");
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

    com = random() % 3;
    move(b_lines - 5, 0);
    clrtobot();
    switch (com)
    {
    case 0:
        outs("小雞：剪刀\n");
        break;
    case 1:
        outs("小雞：石頭\n");
        break;
    case 2:
        outs("小雞：布\n");
        break;
    }

    move(b_lines - 6, 0);

    switch (pipkey)
    {
    case '1':
        outs("你  ：剪刀\n");
        if (com == 0)
            tie();
        else  if (com == 1)
            lose();
        else if (com == 2)
            win();
        break;
    case '2':
        outs("你　：石頭\n");
        if (com == 0)
            win();
        else if (com == 1)
            tie();
        else if (com == 2)
            lose();
        break;
    case '3':
        outs("你　：布\n");
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

    return 0;
}

static void win(void)
{
    d.winn++;
    d.hp -= random() % 2 + 3;
    move(4, 0);
    show_guess_pic(2);
    move(b_lines, 0);
    vmsg("小雞輸了....~>_<~");
    return;
}

static void tie(void)
{
    d.hp -= random() % 2 + 3;
    d.happy += random() % 3 + 5;
    move(4, 0);
    show_guess_pic(3);
    move(b_lines, 0);
    vmsg("平手........-_-");
    return;
}

static void lose(void)
{
    d.losee++;
    d.happy += random() % 3 + 5;
    d.hp -= random() % 2 + 3;
    move(4, 0);
    show_guess_pic(1);
    move(b_lines, 0);
    vmsg("小雞贏囉....*^_^*");
    return;
}

static void situ(void)
{
    clrchyiuan(b_lines - 4, b_lines - 2);
    move(b_lines - 4, 0);
    prints("你:\x1b[44m %d勝 %d負\x1b[m                     \n", d.winn, d.losee);
    move(b_lines - 3, 0);
    prints("雞:\x1b[44m %d勝 %d負\x1b[m                     \n", d.losee, d.winn);

    if (d.winn >= d.losee)
    {
        move(b_lines, 0);
        vmsg("哈..贏小雞也沒多光榮");
    }
    else
    {
        move(b_lines, 0);
        vmsg("笨蛋..竟輸給了雞....ㄜ...");
    }
    return;
}

/*---------------------------------------------------------------------------*/
/* 修行選單:念書 練武 修行                                                   */
/*                                                                           */
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/* 資料庫                                                                    */
/*---------------------------------------------------------------------------*/
static const char *classrank[6] = {"沒有", "初級", "中級", "高級", "進階", "專業"};
static int classmoney[11][2] = {{ 0,  0},
    {60, 110}, {70, 120}, {70, 120}, {80, 130}, {70, 120},
    {60, 110}, {90, 140}, {70, 120}, {70, 120}, {80, 130}
};
static int classvariable[11][4] =
{
    {0, 0, 0, 0},
    {5, 5, 4, 4}, {5, 7, 6, 4}, {5, 7, 6, 4}, {5, 6, 5, 4}, {7, 5, 4, 6},
    {7, 5, 4, 6}, {6, 5, 4, 6}, {6, 6, 5, 4}, {5, 5, 4, 7}, {7, 5, 4, 7}
};


static const char *classword[11][5] =
{
    {"課名", "成功\一", "成功\二", "失敗一", "失敗二"},

    {"自然科學", "正在用功\讀書中..", "我是聰明雞 cccc...",
     "這題怎麼看不懂咧..怪了", "唸不完了 :~~~~~~"},

    {"唐詩宋詞", "床前明月光...疑是地上霜...", "紅豆生南國..春來發幾枝..",
     "ㄟ..上課不要流口水", "你還混喔..罰你背會唐詩三百首"},

    {"神學教育", "哈雷路亞  哈雷路亞", "讓我們迎接天堂之門",
     "ㄟ..你在幹嘛ㄚ? 還不好好唸", "神學很嚴肅的..請好好學..:("},

    {"軍學教育", "孫子兵法是中國兵法書..", "從軍報國，我要帶兵去打仗",
     "什麼陣形ㄚ?混亂陣形?? @_@", "你還以為你在玩三國志ㄚ?"},

    {"劍道技術", "看我的厲害  獨孤九劍....", "我刺 我刺 我刺刺刺..",
     "劍要拿穩一點啦..", "你在刺地鼠ㄚ? 劍拿高一點"},

    {"格鬥戰技", "肌肉是肌肉  呼呼..", "十八銅人行氣散..",
     "腳再踢高一點啦...", "拳頭怎麼這麼沒力ㄚ.."},

    {"魔法教育", "我變 我變 我變變變..", "蛇膽+蟋蜴尾+鼠牙+蟾蜍=??",
     "小心你的掃帚啦  不要亂揮..", "ㄟ～口水不要流到水晶球上.."},

    {"禮儀教育", "要當隻有禮貌的雞...", "歐嗨唷..ㄚ哩ㄚ豆..",
     "怎麼學不會ㄚ??天呀..", "走起路來沒走樣..天ㄚ.."},

    {"繪畫技巧", "很不錯唷..有美術天份..", "這幅畫的顏色搭配的很好..",
     "不要鬼畫符啦..要加油..", "不要咬畫筆啦..壞壞小雞喔.."},

    {"舞蹈技巧", "你就像一隻天鵝喔..", "舞蹈細胞很好喔..",
     "身體再柔軟一點..", "拜託你優美一點..不要這麼粗魯.."}
};
/*---------------------------------------------------------------------------*/
/* 修行選單:念書 練武 修行                                                   */
/* 函式庫                                                                    */
/*---------------------------------------------------------------------------*/

static int pip_practice_classA(void)
{
    /*  ├────┼──────────────────────┤*/
    /*  │自然科學│智力 + 1~ 4, 信仰 - 0~0, 抗魔能力 - 0~0     │*/
    /*  │        ├──────────────────────┤*/
    /*  │        │智力 + 2~ 6, 信仰 - 0~1, 抗魔能力 - 0~1     │*/
    /*  │        ├──────────────────────┤*/
    /*  │        │智力 + 3~ 8, 信仰 - 0~2, 抗魔能力 - 0~1     │*/
    /*  │        ├──────────────────────┤*/
    /*  │        │智力 + 4~12, 信仰 - 1~3, 抗魔能力 - 0~1     │*/
    /*  ├────┼──────────────────────┤*/
    int body, class;
    int change1, change2, change3, change4, change5;

    class = d.wisdom / 200 + 1; /*科學*/
    if (class > 5) class = 5;

    body = pip_practice_function(1, class, 11, 12, &change1, &change2, &change3, &change4, &change5);
    if (body == 0) return 0;
    d.wisdom += change4 * LEARN_LEVEL;
    if (body == 1)
    {
        d.belief -= random() % (2 + class * 2);
        d.mresist -= random() % 4;
    }
    else
    {
        d.belief -= random() % (2 + class * 2);
        d.mresist -= random() % 3;
    }
    pip_practice_gradeup(1, class, d.wisdom / 200 + 1);
    if (d.belief < 0)  d.belief = 0;
    if (d.mresist < 0) d.mresist = 0;
    d.classA += 1;
    return 0;
}

static int pip_practice_classB(void)
{
    /*  ├────┼──────────────────────┤*/
    /*  │詩詞    │感受 + 1~1, 智力 + 0~1, 藝術修養 + 0~1      │*/
    /*  │        │氣質 + 0~1                                  │*/
    /*  │        ├──────────────────────┤*/
    /*  │        │感受 + 1~2, 智力 + 0~2, 藝術修養 + 0~1      │*/
    /*  │        │氣質 + 0~1                                  │*/
    /*  │        ├──────────────────────┤*/
    /*  │        │感受 + 1~4, 智力 + 0~3, 藝術修養 + 0~1      │*/
    /*  │        │氣質 + 0~1                                  │*/
    /*  │        ├──────────────────────┤*/
    /*  │        │感受 + 2~5, 智力 + 0~4, 藝術修養 + 0~1      │*/
    /*  │        │氣質 + 0~1                                  │*/
    /*  ├────┼──────────────────────┤*/
    int body, class;
    int change1, change2, change3, change4, change5;

    class = (d.affect * 2 + d.wisdom + d.art * 2 + d.character) / 400 + 1; /*詩詞*/
    if (class > 5) class = 5;

    body = pip_practice_function(2, class, 21, 21, &change1, &change2, &change3, &change4, &change5);
    if (body == 0) return 0;
    d.affect += change3 * LEARN_LEVEL;
    if (body == 1)
    {
        d.wisdom += random() % (class + 3) * LEARN_LEVEL;
        d.character += random() % (class + 3) * LEARN_LEVEL;
        d.art += random() % (class + 3) * LEARN_LEVEL;
    }
    else
    {
        d.wisdom += random() % (class + 2) * LEARN_LEVEL;
        d.character += random() % (class + 2) * LEARN_LEVEL;
        d.art += random() % (class + 2) * LEARN_LEVEL;
    }
    body = (d.affect * 2 + d.wisdom + d.art * 2 + d.character) / 400 + 1;
    pip_practice_gradeup(2, class, body);
    d.classB += 1;
    return 0;
}

static int pip_practice_classC(void)
{
    /*  ├────┼──────────────────────┤*/
    /*  │神學    │智力 + 1~1, 信仰 + 1~2, 抗魔能力 + 0~1      │*/
    /*  │        ├──────────────────────┤*/
    /*  │        │智力 + 1~1, 信仰 + 1~3, 抗魔能力 + 0~1      │*/
    /*  │        ├──────────────────────┤*/
    /*  │        │智力 + 1~2, 信仰 + 1~4, 抗魔能力 + 0~1      │*/
    /*  │        ├──────────────────────┤*/
    /*  │        │智力 + 1~3, 信仰 + 1~5, 抗魔能力 + 0~1      │*/
    /*  ├────┼──────────────────────┤*/
    int body, class;
    int change1, change2, change3, change4, change5;

    class = (d.belief * 2 + d.wisdom) / 400 + 1; /*神學*/
    if (class > 5) class = 5;

    body = pip_practice_function(3, class, 31, 31, &change1, &change2, &change3, &change4, &change5);
    if (body == 0) return 0;
    d.wisdom += change2 * LEARN_LEVEL;
    d.belief += change3 * LEARN_LEVEL;
    if (body == 1)
    {
        d.mresist += random() % 5 * LEARN_LEVEL;
    }
    else
    {
        d.mresist += random() % 3 * LEARN_LEVEL;
    }
    body = (d.belief * 2 + d.wisdom) / 400 + 1;
    pip_practice_gradeup(3, class, body);
    d.classC += 1;
    return 0;
}

static int pip_practice_classD(void)
{
    /*  ├────┼──────────────────────┤*/
    /*  │軍學    │智力 + 1~2, 戰鬥技術 + 0~1, 感受 - 0~1      │*/
    /*  │        ├──────────────────────┤*/
    /*  │        │智力 + 2~4, 戰鬥技術 + 0~1, 感受 - 0~1      │*/
    /*  │        ├──────────────────────┤*/
    /*  │        │智力 + 3~4, 戰鬥技術 + 0~1, 感受 - 0~1      │*/
    /*  │        ├──────────────────────┤*/
    /*  │        │智力 + 4~5, 戰鬥技術 + 0~1, 感受 - 0~1      │*/
    /*  ├────┼──────────────────────┤*/
    int body, class;
    int change1, change2, change3, change4, change5;

    class = (d.hskill * 2 + d.wisdom) / 400 + 1;
    if (class > 5) class = 5;
    body = pip_practice_function(4, class, 41, 41, &change1, &change2, &change3, &change4, &change5);
    if (body == 0) return 0;
    d.wisdom += change2 * LEARN_LEVEL;
    if (body == 1)
    {
        d.hskill += (random() % 3 + 4) * LEARN_LEVEL;
        d.affect -= random() % 3 + 6;
    }
    else
    {
        d.hskill += (random() % 3 + 2) * LEARN_LEVEL;
        d.affect -= random() % 3 + 6;
    }
    body = (d.hskill * 2 + d.wisdom) / 400 + 1;
    pip_practice_gradeup(4, class, body);
    if (d.affect < 0)  d.affect = 0;
    d.classD += 1;
    return 0;
}

static int pip_practice_classE(void)
{
    /*  ├────┼──────────────────────┤*/
    /*  │劍術    │戰鬥技術 + 0~1, 攻擊能力 + 1~1              │*/
    /*  │        ├──────────────────────┤*/
    /*  │        │戰鬥技術 + 0~1, 攻擊能力 + 1~2              │*/
    /*  │        ├──────────────────────┤*/
    /*  │        │戰鬥技術 + 0~1, 攻擊能力 + 1~3              │*/
    /*  │        ├──────────────────────┤*/
    /*  │        │戰鬥技術 + 0~1, 攻擊能力 + 1~4              │*/
    /*  ├────┼──────────────────────┤*/
    int body, class;
    int change1, change2, change3, change4, change5;

    class = (d.hskill + d.attack) / 400 + 1;
    if (class > 5) class = 5;

    body = pip_practice_function(5, class, 51, 51, &change1, &change2, &change3, &change4, &change5);
    if (body == 0) return 0;
    d.speed += (random() % 3 + 2) * LEARN_LEVEL;
    d.hexp += (random() % 2 + 2) * LEARN_LEVEL;
    d.attack += change4 * LEARN_LEVEL;
    if (body == 1)
    {
        d.hskill += (random() % 3 + 5) * LEARN_LEVEL;
    }
    else
    {
        d.hskill += (random() % 3 + 3) * LEARN_LEVEL;
    }
    body = (d.hskill + d.attack) / 400 + 1;
    pip_practice_gradeup(5, class, body);
    d.classE += 1;
    return 0;
}

static int pip_practice_classF(void)
{
    /*  ├────┼──────────────────────┤*/
    /*  │格鬥術  │戰鬥技術 + 1~1, 防禦能力 + 0~0              │*/
    /*  │        ├──────────────────────┤*/
    /*  │        │戰鬥技術 + 1~1, 防禦能力 + 0~1              │*/
    /*  │        ├──────────────────────┤*/
    /*  │        │戰鬥技術 + 1~2, 防禦能力 + 0~1              │*/
    /*  │        ├──────────────────────┤*/
    /*  │        │戰鬥技術 + 1~3, 防禦能力 + 0~1              │*/
    /*  ├────┼──────────────────────┤*/
    int body, class;
    int change1, change2, change3, change4, change5;

    class = (d.hskill + d.resist) / 400 + 1;
    if (class > 5) class = 5;

    body = pip_practice_function(6, class, 61, 61, &change1, &change2, &change3, &change4, &change5);
    if (body == 0) return 0;
    d.hexp += (random() % 2 + 2) * LEARN_LEVEL;
    d.speed += (random() % 3 + 2) * LEARN_LEVEL;
    d.resist += change2 * LEARN_LEVEL;
    if (body == 1)
    {
        d.hskill += (random() % 3 + 5) * LEARN_LEVEL;
    }
    else
    {
        d.hskill += (random() % 3 + 3) * LEARN_LEVEL;
    }
    body = (d.hskill + d.resist) / 400 + 1;
    pip_practice_gradeup(6, class, body);
    d.classF += 1;
    return 0;
}

static int pip_practice_classG(void)
{
    /*  ├────┼──────────────────────┤*/
    /*  │魔法    │魔法技術 + 1~1, 魔法能力 + 0~2              │*/
    /*  │        ├──────────────────────┤*/
    /*  │        │魔法技術 + 1~2, 魔法能力 + 0~3              │*/
    /*  │        ├──────────────────────┤*/
    /*  │        │魔法技術 + 1~3, 魔法能力 + 0~4              │*/
    /*  │        ├──────────────────────┤*/
    /*  │        │魔法技術 + 2~4, 魔法能力 + 0~5              │*/
    /*  ├────┼──────────────────────┤*/
    int body, class;
    int change1, change2, change3, change4, change5;

    class = (d.mskill + d.maxmp) / 400 + 1;
    if (class > 5) class = 5;

    body = pip_practice_function(7, class, 71, 72, &change1, &change2, &change3, &change4, &change5);
    if (body == 0) return 0;
    d.maxmp += change3 * LEARN_LEVEL;
    d.mexp += (random() % 2 + 2) * LEARN_LEVEL;
    if (body == 1)
    {
        d.mskill += (random() % 3 + 7) * LEARN_LEVEL;
    }
    else
    {
        d.mskill += (random() % 3 + 4) * LEARN_LEVEL;
    }

    body = (d.mskill + d.maxmp) / 400 + 1;
    pip_practice_gradeup(7, class, body);
    d.classG += 1;
    return 0;
}

static int pip_practice_classH(void)
{
    /*  ├────┼──────────────────────┤*/
    /*  │禮儀    │禮儀表現 + 1~1, 氣質 + 1~1                  │*/
    /*  │        ├──────────────────────┤*/
    /*  │        │禮儀表現 + 1~2, 氣質 + 1~2                  │*/
    /*  │        ├──────────────────────┤*/
    /*  │        │禮儀表現 + 1~3, 氣質 + 1~3                  │*/
    /*  │        ├──────────────────────┤*/
    /*  │        │禮儀表現 + 2~4, 氣質 + 1~4                  │*/
    /*  ├────┼──────────────────────┤*/
    int body, class;
    int change1, change2, change3, change4, change5;

    class = (d.manners * 2 + d.character) / 400 + 1;
    if (class > 5) class = 5;

    body = pip_practice_function(8, class, 0, 0, &change1, &change2, &change3, &change4, &change5);
    if (body == 0) return 0;
    d.social += (random() % 2 + 2) * LEARN_LEVEL;
    d.manners += (change1 + random() % 2) * LEARN_LEVEL;
    d.character += (change1 + random() % 2) * LEARN_LEVEL;
    body = (d.character + d.manners) / 400 + 1;
    pip_practice_gradeup(8, class, body);
    d.classH += 1;
    return 0;
}

static int pip_practice_classI(void)
{
    /*  ├────┼──────────────────────┤*/
    /*  │繪畫    │藝術修養 + 1~1, 感受 + 0~1                  │*/
    /*  │        ├──────────────────────┤*/
    /*  │        │藝術修養 + 1~2, 感受 + 0~1                  │*/
    /*  │        ├──────────────────────┤*/
    /*  │        │藝術修養 + 1~3, 感受 + 0~1                  │*/
    /*  │        ├──────────────────────┤*/
    /*  │        │藝術修養 + 2~4, 感受 + 0~1                  │*/
    /*  ├────┼──────────────────────┤*/
    int body, class;
    int change1, change2, change3, change4, change5;

    class = (d.art * 2 + d.character) / 400 + 1;
    if (class > 5) class = 5;

    body = pip_practice_function(9, class, 91, 91, &change1, &change2, &change3, &change4, &change5);
    if (body == 0) return 0;
    d.art += change4 * LEARN_LEVEL;
    d.affect += change2 * LEARN_LEVEL;
    body = (d.affect + d.art) / 400 + 1;
    pip_practice_gradeup(9, class, body);
    d.classI += 1;
    return 0;
}

static int pip_practice_classJ(void)
{
    /*  ├────┼──────────────────────┤*/
    /*  │舞蹈    │藝術修養 + 0~1, 魅力 + 0~1, 體力 + 1~1      │*/
    /*  │        ├──────────────────────┤*/
    /*  │        │藝術修養 + 1~1, 魅力 + 0~1, 體力 + 1~1      │*/
    /*  │        ├──────────────────────┤*/
    /*  │        │藝術修養 + 1~2, 魅力 + 0~2, 體力 + 1~1      │*/
    /*  │        ├──────────────────────┤*/
    /*  │        │藝術修養 + 1~3, 魅力 + 1~2, 體力 + 1~1      │*/
    /*  └────┴──────────────────────┘*/
    int body, class;
    int change1, change2, change3, change4, change5;

    class = (d.art * 2 + d.charm) / 400 + 1;
    if (class > 5) class = 5;

    body = pip_practice_function(10, class, 0, 0, &change1, &change2, &change3, &change4, &change5);
    if (body == 0) return 0;
    d.art += change2 * LEARN_LEVEL;
    d.maxhp += (random() % 3 + 2) * LEARN_LEVEL;
    if (body == 1)
    {
        d.charm += random() % (4 + class) * LEARN_LEVEL;
    }
    else if (body == 2)
    {
        d.charm += random() % (2 + class) * LEARN_LEVEL;
    }
    body = (d.art * 2 + d.charm) / 400 + 1;
    pip_practice_gradeup(10, class, body);
    d.classJ += 1;
    return 0;
}

/*傳入:課號 等級 生命 快樂 滿足 髒髒 傳回:變數12345 return:body*/
static int
pip_practice_function(
int classnum, int classgrade, int pic1, int pic2,
int *change1, int *change2, int *change3, int *change4, int *change5)
{
    int  a, b, body, health;
    char inbuf[256], ans[5];
    long smoney;

    /*錢的算法*/
    smoney = classgrade * classmoney[classnum][0] + classmoney[classnum][1];
    move(b_lines - 2, 0);
    clrtoeol();
    sprintf(inbuf, "[%8s%4s課程]要花 $%ld，確定要嗎??[y/N]: ", classword[classnum][0], classrank[classgrade], smoney);
    getdata(b_lines - 2, 1, inbuf, ans, 2, DOECHO, 0);
    if (ans[0] != 'y' && ans[0] != 'Y')  return 0;
    if (d.money < smoney)
    {
        vmsg("很抱歉喔...你的錢不夠喔");
        return 0;
    }
    count_tired(4, 5, "Y", 100, 1);
    d.money = d.money - smoney;
    /*成功與否的判斷*/
    health = d.hp * 1 / 2 + random() % 20 - d.tired;
    if (health > 0) body = 1;
    else body = 2;

    a = random() % 3 + 2;
    b = (random() % 12 + random() % 13) % 2;
    d.hp -= random() % (3 + random() % 3) + classvariable[classnum][0];
    d.happy -= random() % (3 + random() % 3) + classvariable[classnum][1];
    d.satisfy -= random() % (3 + random() % 3) + classvariable[classnum][2];
    d.shit += random() % (3 + random() % 3) + classvariable[classnum][3];
    *change1 = random() % a + 2 + classgrade * 2 / (body + 1);    /* random()%3+3 */
    *change2 = random() % a + 4 + classgrade * 2 / (body + 1);    /* random()%3+5 */
    *change3 = random() % a + 5 + classgrade * 3 / (body + 1);    /* random()%3+7 */
    *change4 = random() % a + 7 + classgrade * 3 / (body + 1);    /* random()%3+9 */
    *change5 = random() % a + 9 + classgrade * 3 / (body + 1);    /* random()%3+11 */
    if (random() % 2 > 0 && pic1 > 0)
        show_practice_pic(pic1);
    else if (pic2 > 0)
        show_practice_pic(pic2);
    vmsg(classword[classnum][body+b]);
    return body;
}

static int pip_practice_gradeup(
int classnum, int classgrade, int data)
{
    char inbuf[256];

    if ((data == (classgrade + 1)) && classgrade < 5)
    {
        sprintf(inbuf, "下次換上 [%8s%4s課程]",
                classword[classnum][0], classrank[classgrade+1]);
        vmsg(inbuf);
    }
    return 0;
}

/*---------------------------------------------------------------------------*/
/* 特殊選單:看病 減肥 戰鬥 拜訪 朝見                                         */
/*                                                                           */
/*---------------------------------------------------------------------------*/


static int pip_see_doctor(void)        /*看醫生*/
{
    char buf[256];
    long savemoney;
    clrchyiuan(b_lines - 2, b_lines);
    savemoney = d.sick * 25;
    if (d.sick <= 0)
    {
        vmsg("哇哩..沒病來醫院幹嘛..被罵了..嗚~~");
        d.character -= (random() % 3 + 1);
        if (d.character < 0)
            d.character = 0;
        d.happy -= (random() % 3 + 3);
        d.satisfy -= random() % 3 + 2;
    }
    else if (d.money < savemoney)
    {
        sprintf(buf, "你的病要花 %ld 元喔....你不夠錢啦...", savemoney);
        vmsg(buf);
    }
    else if (d.sick > 0 && d.money >= savemoney)
    {
        d.tired -= random() % 10 + 20;
        if (d.tired < 0)
            d.tired = 0;
        d.sick = 0;
        d.money = d.money - savemoney;
        move(4, 0);
        show_special_pic(1);
        vmsg("藥到病除..沒有副作用!!");
    }
    return 0;
}

/*減肥*/
static int pip_change_weight(void)
{
    char genbuf[5];
    char inbuf[256];
    int weightmp;

    move(b_lines -1, 0);
    clrtoeol();
    show_special_pic(2);
    getdata(b_lines - 1, 1, "你的選擇是? [Q]離開: ", genbuf, 2, 1, 0);
    if (genbuf[0] == '1' || genbuf[0] == '2' || genbuf[0] == '3' || genbuf[0] == '4')
    {
        switch (genbuf[0])
        {
        case '1':
            if (d.money < 80)
            {
                vmsg("傳統增胖要80元喔....你不夠錢啦...");
            }
            else
            {
                getdata(b_lines - 1, 1, "需花費80元(3～5公斤)，你確定嗎? [y/N]: ", genbuf, 2, 1, 0);
                if (genbuf[0] == 'Y' || genbuf[0] == 'y')
                {
                    weightmp = 3 + random() % 3;
                    d.weight += weightmp;
                    d.money -= 80;
                    d.maxhp -= random() % 2;
                    d.hp -= random() % 2 + 3;
                    show_special_pic(3);
                    sprintf(inbuf, "總共增加了%d公斤", weightmp);
                    vmsg(inbuf);
                }
                else
                {
                    vmsg("回心轉意囉.....");
                }
            }
            break;

        case '2':
            getdata(b_lines - 1, 1, "增一公斤要30元，你要增多少公斤呢? [請填數字]: ", genbuf, 4, 1, 0);
            weightmp = atoi(genbuf);
            if (weightmp <= 0)
            {
                vmsg("輸入有誤..放棄囉...");
            }
            else if (d.money > (weightmp*30))
            {
                sprintf(inbuf, "增加%d公斤，總共需花費%d元，確定嗎? [y/N]: ", weightmp, weightmp*30);
                getdata(b_lines - 1, 1, inbuf, genbuf, 2, 1, 0);
                if (genbuf[0] == 'Y' || genbuf[0] == 'y')
                {
                    d.money -= weightmp * 30;
                    d.weight += weightmp;
                    d.maxhp -= (random() % 2 + 2);
                    count_tired(5, 8, "N", 100, 1);
                    d.hp -= (random() % 2 + 3);
                    d.sick += random() % 10 + 5;
                    show_special_pic(3);
                    sprintf(inbuf, "總共增加了%d公斤", weightmp);
                    vmsg(inbuf);
                }
                else
                {
                    vmsg("回心轉意囉.....");
                }
            }
            else
            {
                vmsg("你錢沒那麼多啦.......");
            }
            break;

        case '3':
            if (d.money < 80)
            {
                vmsg("傳統減肥要80元喔....你不夠錢啦...");
            }
            else
            {
                getdata(b_lines - 1, 1, "需花費80元(3～5公斤)，你確定嗎? [y/N]: ", genbuf, 2, 1, 0);
                if (genbuf[0] == 'Y' || genbuf[0] == 'y')
                {
                    weightmp = 3 + random() % 3;
                    d.weight -= weightmp;
                    if (d.weight < 0)
                        d.weight = 0;
                    d.money -= 100;
                    d.maxhp += random() % 2;
                    d.hp -= random() % 2 + 3;
                    show_special_pic(4);
                    sprintf(inbuf, "總共減少了%d公斤", weightmp);
                    vmsg(inbuf);
                }
                else
                {
                    vmsg("回心轉意囉.....");
                }
            }
            break;
        case '4':
            getdata(b_lines - 1, 1, "減一公斤要30元，你要減多少公斤呢? [請填數字]: ", genbuf, 4, 1, 0);
            weightmp = atoi(genbuf);
            if (weightmp <= 0)
            {
                vmsg("輸入有誤..放棄囉...");
            }
            else if (d.weight <= weightmp)
            {
                vmsg("你沒那麼重喔.....");
            }
            else if (d.money > (weightmp*30))
            {
                sprintf(inbuf, "減少%d公斤，總共需花費%d元，確定嗎? [y/N]: ", weightmp, weightmp*30);
                getdata(b_lines - 1, 1, inbuf, genbuf, 2, 1, 0);
                if (genbuf[0] == 'Y' || genbuf[0] == 'y')
                {
                    d.money -= weightmp * 30;
                    d.weight -= weightmp;
                    d.maxhp -= (random() % 2 + 2);
                    count_tired(5, 8, "N", 100, 1);
                    d.hp -= (random() % 2 + 3);
                    d.sick += random() % 10 + 5;
                    show_special_pic(4);
                    sprintf(inbuf, "總共減少了%d公斤", weightmp);
                    vmsg(inbuf);
                }
                else
                {
                    vmsg("回心轉意囉.....");
                }
            }
            else
            {
                vmsg("你錢沒那麼多啦.......");
            }
            break;
        }
    }
    return 0;
}


/*參見*/

static int
pip_go_palace(void)
{
    pip_go_palace_screen(royallist);
    return 0;
}

static int
pip_go_palace_screen(
const struct royalset *p)
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
    const char *needmode[3] = {"      ", "禮儀表現＞", "談吐技巧＞"};
    int save[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    d.nodone = 0;
    do
    {
        clear();
        show_palace_pic(0);
        move(13, 4);
        sprintf(buf, "\x1b[1;31m┌──────┤\x1b[37;41m 來到總司令部了  請選擇你欲拜訪的對象 \x1b[0;1;31m├──────┐\x1b[0m");
        outs_centered(buf);
        move(14, 4);
        sprintf(buf, "\x1b[1;31m│                                                                  │\x1b[0m");
        outs_centered(buf);

        for (n = 0; n < 5; n++)
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
                sprintf(buf, "\x1b[1;31m│ \x1b[36m(\x1b[37m%s\x1b[36m) \x1b[33m%-10s  \x1b[37m%-14s     \x1b[36m(\x1b[37m%s\x1b[36m) \x1b[33m%-10s  \x1b[37m%-14s\x1b[31m│\x1b[0m",
                        p[a].num, p[a].name, inbuf1, p[b].num, p[b].name, inbuf2);
            else
                sprintf(buf, "\x1b[1;31m│ \x1b[36m(\x1b[37m%s\x1b[36m) \x1b[33m%-10s  \x1b[37m%-14s                                   \x1b[31m│\x1b[0m",
                        p[a].num, p[a].name, inbuf1);
            outs_centered(buf);
        }
        move(20, 4);
        sprintf(buf, "\x1b[1;31m│                                                                  │\x1b[0m");
        outs_centered(buf);
        move(21, 4);
        sprintf(buf, "\x1b[1;31m└─────────────────────────────────┘\x1b[0m");
        outs_centered(buf);


        if (d.death == 1 || d.death == 2 || d.death == 3)
            return 0;
        /*將各人物已經給與的數值叫回來*/
        save[1] = d.royalA;          /*from守衛*/
        save[2] = d.royalB;          /*from近衛*/
        save[3] = d.royalC;          /*from將軍*/
        save[4] = d.royalD;          /*from大臣*/
        save[5] = d.royalE;          /*from祭司*/
        save[6] = d.royalF;          /*from寵妃*/
        save[7] = d.royalG;          /*from王妃*/
        save[8] = d.royalH;          /*from國王*/
        save[9] = d.royalI;          /*from小丑*/
        save[10] = d.royalJ;         /*from王子*/

        move(b_lines - 1, 0);
        clrtoeol();
        move(b_lines - 1, 0);
        prints("\x1b[1;33m [生命力] %d/%d  [疲勞度] %d \x1b[0m", d.hp, d.maxhp, d.tired);

        move(b_lines, 0);
        clrtoeol();
        move(b_lines, 0);
        prints(
            "\x1b[1;37;46m  參見選單  \x1b[44m [字母]選擇欲拜訪的人物  [Q]離開" NICKNAME "總司令部      %*s\x1b[0m", 20 + d_cols - ((int)(unsigned int)sizeof(NICKNAME) - 1), "");
        pipkey = vkey();
        choice = pipkey - 64;
        if (choice < 1 || choice > 10)
            choice = pipkey - 96;

        if ((choice >= 1 && choice <= 10 && d.seeroyalJ == 1) || (choice >= 1 && choice <= 9 && d.seeroyalJ == 0))
        {
            d.social += random() % 3 + 3;
            d.hp -= random() % 5 + 6;
            d.tired += random() % 5 + 8;
            if (d.tired >= 100)
            {
                d.death = 1;
                pipdie("\x1b[1;31m累死了...\x1b[m  ", 1);
            }
            if (d.hp < 0)
            {
                d.death = 1;
                pipdie("\x1b[1;31m餓死了...\x1b[m  ", 1);
            }
            if (d.death == 1)
            {
                sprintf(buf, "掰掰了...真是悲情..");
            }
            else
            {
                if ((p[choice].needmode == 0) ||
                    (p[choice].needmode == 1 && d.manners >= p[choice].needvalue) ||
                    (p[choice].needmode == 2 && d.speech >= p[choice].needvalue))
                {
                    if (choice >= 1 && choice <= 9 && save[choice] >= p[choice].maxtoman)
                    {
                        if (random() % 2 > 0)
                            sprintf(buf, "能和這麼偉大的你講話真是榮幸ㄚ...");
                        else
                            sprintf(buf, "很高興你來拜訪我，但我不能給你什麼了..");
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
                            /*如果大於每次的增加最大量*/
                            if (change > p[choice].addtoman)
                                change = p[choice].addtoman;
                            /*如果加上原先的之後大於所能給的所有值時*/
                            if ((change + save[choice]) >= p[choice].maxtoman)
                                change = p[choice].maxtoman - save[choice];
                            save[choice] += change;
                            d.toman += change;
                        }
                        else if (choice == 9)
                        {
                            save[9] = 0;
                            d.social -= 13 + random() % 4;
                            d.affect += 13 + random() % 4;
                        }
                        else if (choice == 10 && d.seeroyalJ == 1)
                        {
                            save[10] += 15 + random() % 4;
                            d.seeroyalJ = 0;
                        }
                        if (random() % 2 > 0)
                            sprintf(buf, "%s", p[choice].words1);
                        else
                            sprintf(buf, "%s", p[choice].words2);
                    }
                }
                else
                {
                    if (random() % 2 > 0)
                        sprintf(buf, "我不和你這樣的雞談話....");
                    else
                        sprintf(buf, "你這隻沒教養的雞，再去學學禮儀吧....");

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

    vmsg("離開" NICKNAME "總司令部.....");
    return 0;
}
/*--------------------------------------------------------------------------*/
/* pip_vs_fight.c 小雞對戰程式                                              */
/* 作者:chyiuan   感謝SiEpthero的技術指導                                   */
/*--------------------------------------------------------------------------*/
#ifdef  HAVE_PIP_FIGHT
static int
pip_set_currutmp(void)
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
pip_get_currutmp(void)
{
    d.hp = currutmp->pip->hp;
    d.mp = currutmp->pip->mp;
}

int
pip_vf_fight(
int fd,
int first)
{
    pipdata temp;
    struct chicken chickentemp;
    int ch, datac, dinjure, oldtired, oldhp;
    int oldhexp, oldmexp, oldhskill, oldmskill, oldbrave;
    int gameover = 0;
    int i;
    int notyou = 0;                     /*chyiuan:以免訊息被弄錯*/
    float mresist;
    UTMP *opponent;
    char data[200], buf1[256], buf2[256], mymsg[8][150];

    memcpy(&temp, &(cutmp->pip), sizeof(pipdata));
    memcpy(&chickentemp, &d, sizeof(d));


    currutmp = cutmp;
    utmp_mode(M_CHICKEN);
    clear();
    pip_read_file(cuser.userid);
    currutmp->pip->pipmode = 0; /*1:輸了 2:贏了 3:不玩了 */
    currutmp->pip->leaving = 1;
    currutmp->pip->mode = d.chickenmode;
    pip_set_currutmp();         /*把小雞的data  down load for被呼叫者*/
    currutmp->pip->nodone = first;      /*決定誰先攻擊*/
    currutmp->pip->msgcount = 0;        /*戰鬥訊息歸零*/
    currutmp->pip->chatcount = 0;       /*聊天訊息歸零*/
    currutmp->pip->msg[0] = '\0';
    strcpy(currutmp->pip->name, d.name);


    /*存下舊小雞data*/
    oldmexp = d.mexp;
    oldhexp = d.hexp;
    oldbrave = d.brave;
    oldhskill = d.hskill;
    oldmskill = d.mskill;
    opponent = cutmp->talker;
    add_io(fd, 2);
    /*對方未準備妥當  先等一下  為了防止當機 */
    while (gameover == 0 && (opponent->pip == NULL || opponent->pip->leaving == 0))
    {
        move(b_lines, 0);
        prints("\x1b[1;46m 對方還在準備中                                                               %*s\x1b[m", d_cols, "");
        ch = vkey();
    }
    if (currutmp->pip->mode != opponent->pip->mode)
    {
        vmsg("一代雞與二代雞不能互相 PK !!");
        add_io(0, 60);
        return 0;
    }
    for (i = 0; i < 8; i++)
        mymsg[i][0] = '\0';
    for (i = 0; i < 10; i++)
        currutmp->pip->chat[i][0] = '\0';
    /*開始的訊息*/
    sprintf(mymsg[0], "\x1b[1;37m%s 和 %s 的戰鬥開始了..\x1b[m",
            opponent->pip->name, currutmp->pip->name);
    strcpy(currutmp->pip->msg, mymsg[0]);
    currutmp->pip->msgcount = 0;
    /*msgcount和charcount的算法不同*/
    add_io(fd, 1);
    /*  currutmp->pip->mode=0;*/
    while (!(opponent->pip || currutmp->pip->leaving == 0 || opponent->pip->leaving == 0))
    {
        clear();
        /*為了一些其他的原因  像餵食等是呼叫舊的  所以reload*/
        pip_get_currutmp();
        /*              pip_set_currutmp();*/

        if (opponent->pip->nodone != 1)
            strcpy(mymsg[currutmp->pip->msgcount%8], currutmp->pip->msg);
        move(0, 0);
        outs_centered("\x1b[1;34m═╣\x1b[44;37m 自己資料 \x1b[0;1;34m╠═══════════════════════════════\x1b[m\n");
        prints_centered("\x1b[1m   \x1b[33m姓  名:\x1b[37m%-20s                                              \x1b[31m  \x1b[m\n",
               d.name);
        sprintf(buf1, "%d/%d", d.hp, d.maxhp);
        sprintf(buf2, "%d/%d", d.mp, d.maxmp);
        prints_centered("\x1b[1m   \x1b[33m體  力:\x1b[37m%-24s       \x1b[33m法  力:\x1b[37m%-24s\x1b[33m\x1b[m\n",
               buf1, buf2);
        prints_centered("\x1b[1m   \x1b[33m攻  擊:\x1b[37m%-12d\x1b[33m防  禦:\x1b[37m%-12d\x1b[33m速  度:\x1b[37m%-12d\x1b[33m抗  魔:\x1b[37m%-9d  \x1b[m\n",
               d.attack, d.resist, d.speed, d.mresist);
        prints_centered("\x1b[1m   \x1b[33m戰鬥技:\x1b[37m%-12d\x1b[33m魔法技:\x1b[37m%-12d\x1b[33m魔評價:\x1b[37m%-12d\x1b[33m武評價:\x1b[37m%-9d  \x1b[m\n",
               d.hskill, d.mskill, d.mexp, d.hexp);
        prints_centered("\x1b[1m   \x1b[33m食  物:\x1b[37m%-12d\x1b[33m補  丸:\x1b[37m%-12d\x1b[33m零  食:\x1b[37m%-12d\x1b[33m靈  芝:\x1b[37m%-9d  \x1b[m\n",
               d.food, d.bighp, d.cookie, d.medicine);
        prints_centered("\x1b[1m   \x1b[33m人  蔘:\x1b[37m%-12d\x1b[33m雪  蓮:\x1b[37m%-12d\x1b[33m疲  勞:\x1b[37m%-15d               \x1b[m\n",
               d.ginseng, d.snowgrass, d.tired);
        move(7, 0);
        outs_centered("\x1b[1;34m═╣\x1b[44;37m 戰鬥訊息 \x1b[0;1;34m╠═══════════════════════════════\x1b[m\n");
        for (i = 0; i < 8; i++)
        {
            move(8 + i, 1);

            if (currutmp->pip->msgcount < 8)
            {
                outs_centered(mymsg[i]);
                /*適用pip.msgcount在8行內*/
            }
            else
            {
                outs_centered(mymsg[(currutmp->pip->msgcount-8+i)%8]);
                /*pip.msgcount=8:表示已經有9個 所以從0->7*/
            }
        }
        move(16, 0);
        outs_centered("\x1b[1;34m═╣\x1b[44;37m 談話訊息 \x1b[0;1;34m╠═══════════════════════════════\x1b[m\n");
        for (i = 0; i < 2; i++)
        {
            move(17 + i, 0);
            if (currutmp->pip->chatcount < 3)
            {
                outs_centered(currutmp->pip->chat[i]);
                /*適用pip.chatcount在2行內*/
            }
            else
            {
                prints_centered("%s", currutmp->pip->chat[(currutmp->pip->chatcount-2+i)%10]);
                /*pip.chatcount=3:表示已經有2個 所以從0->1*/
            }
        }
        move(19, 0);
        outs_centered("\x1b[1;34m═╣\x1b[1;37;44m 對手資料 \x1b[0;1;34m╠═══════════════════════════════\x1b[m\n");
        prints_centered("\x1b[1m   \x1b[33m姓  名:\x1b[37m%-20s                                                \x1b[m\n",
               opponent->pip->name);
        sprintf(buf1, "%d/%d", opponent->pip->hp, opponent->pip->maxhp);
        sprintf(buf2, "%d/%d", opponent->pip->mp, opponent->pip->maxmp);
        prints_centered("\x1b[1m   \x1b[33m體  力:\x1b[37m%-24s       \x1b[33m法  力:\x1b[37m%-24s\x1b[m\n",
               buf1, buf2);
        outs_centered("\x1b[1;34m═══════════════════════════════════════\x1b[m\n");
        if (opponent->pip->nodone == 1)
        {
            notyou = 1;
            prints("\x1b[1;37;44m  對方出招中，請稍待一會.....                                [T/^T]CHAT/回顧  \x1b[m");
        }
        else
        {
            notyou = 0;
            prints("\x1b[1;44;37m  戰鬥命令  \x1b[46m [1]普通 [2]全力 [3]魔法 [4]防禦 [5]補充 [6]逃命 [T/^T]CHAT/回顧  \x1b[m");
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
            len = getdata(b_lines, 0, "想說: ", buf, 60, 1, 0);
            if (len && buf[0] != ' ')
            {
                sprintf(msg, "\x1b[1;46;33m★%s\x1b[37;45m %s \x1b[m", cuser.userid, buf);
                strcpy(opponent->pip->chat[currutmp->pip->chatcount%10], msg);
                strcpy(currutmp->pip->chat[currutmp->pip->chatcount%10], msg);
                opponent->pip->chatcount++;
                currutmp->pip->chatcount++;
            }

        }
        else if (ch == Ctrl('T'))
        {
            add_io(fd, 30);
            clrchyiuan(7, b_lines - 4);
            move(7, 0);
            outs_centered("\x1b[1;31m═╣\x1b[41;37m 回顧談話 \x1b[0;1;31m╠═══════════════════════════════\x1b[m\n");
            for (i = 0; i < 10; i++)
            {
                move(8 + i, 0);
                if (currutmp->pip->chatcount < 10)
                {
                    outs_centered(currutmp->pip->chat[i]);
                    /*適用pip.msgcount在七行內*/
                }
                else
                {
                    prints_centered("%s", currutmp->pip->chat[(currutmp->pip->chatcount-10+i)%10]);
                    /*pip.chatcount=10:表示已經有11個 所以從0->9*/
                }
            }
            move(18, 0);
            outs_centered("\x1b[1;31m═╣\x1b[41;37m 到此為止 \x1b[0;1;31m╠═══════════════════════════════\x1b[m");
            vmsg("回顧之前的談話 只有10通");
            add_io(fd, 1);
        }
        else if (currutmp->pip->nodone == 1 && opponent->pip->leaving == 1 && notyou == 0)
        {
            d.nodone = 1;
            switch (ch)
            {
                char buf[256];
            case '1':
                if (random() % 9 == 0)
                {
                    vmsg("竟然沒打中..:~~~");
                    sprintf(buf, "\x1b[1;33m%s \x1b[37m對 \x1b[33m%s\x1b[37m 施展普通攻擊，但是沒有打中...",
                            d.name, opponent->pip->name);
                }
                else
                {
                    if (opponent->pip->resistmode == 0)
                        dinjure = (d.hskill / 100 + d.hexp / 100 + d.attack / 9 - opponent->pip->resist / 12 + random() % 20 - opponent->pip->speed / 30 + d.speed / 30);
                    else
                        dinjure = (d.hskill / 100 + d.hexp / 100 + d.attack / 9 - opponent->pip->resist / 6 + random() % 20 - opponent->pip->speed / 10 + d.speed / 30);
                    if (dinjure <= 10)  dinjure = 10;
                    opponent->pip->hp -= dinjure;
                    d.hexp += random() % 2 + 2;
                    d.hskill += random() % 2 + 1;
                    sprintf(buf, "普通攻擊，對方體力減低%d", dinjure);
                    vmsg(buf);
                    sprintf(buf, "\x1b[1;33m%s \x1b[37m施展了普通攻擊，\x1b[33m%s \x1b[37m的體力減低 \x1b[31m%d \x1b[37m點\x1b[m",
                            d.name, opponent->pip->name, dinjure);
                }
                opponent->pip->resistmode = 0;
                opponent->pip->msgcount++;
                currutmp->pip->msgcount++;
                strcpy(opponent->pip->msg, buf);
                strcpy(mymsg[currutmp->pip->msgcount%8], buf);
                currutmp->pip->nodone = 2;      /*做完*/
                opponent->pip->nodone = 1;
                break;

            case '2':
                show_fight_pic(2);
                if (random() % 11 == 0)
                {
                    vmsg("竟然沒打中..:~~~");
                    sprintf(buf, "\x1b[1;33m%s \x1b[37m對 \x1b[33m%s\x1b[37m 施展全力攻擊，但是沒有打中...",
                            d.name, opponent->pip->name);
                }
                else
                {
                    if (opponent->pip->resistmode == 0)
                        dinjure = (d.hskill / 100 + d.hexp / 100 + d.attack / 5 - opponent->pip->resist / 12 + random() % 30 - opponent->pip->speed / 50 + d.speed / 30);
                    else
                        dinjure = (d.hskill / 100 + d.hexp / 100 + d.attack / 5 - opponent->pip->resist / 6 + random() % 30 - opponent->pip->speed / 30 + d.speed / 30);
                    if (dinjure <= 20) dinjure = 20;
                    if (d.hp > 5)
                    {
                        opponent->pip->hp -= dinjure;
                        d.hp -= 5;
                        d.hexp += random() % 3 + 3;
                        d.hskill += random() % 2 + 2;
                        sprintf(buf, "全力攻擊，對方體力減低%d", dinjure);
                        vmsg(buf);
                        sprintf(buf, "\x1b[1;33m%s \x1b[37m施展了全力攻擊，\x1b[33m%s \x1b[37m的體力減低 \x1b[31m%d \x1b[37m點\x1b[m",
                                d.name, opponent->pip->name, dinjure);
                    }
                    else
                    {
                        d.nodone = 1;
                        vmsg("你的HP小於5啦..不行啦...");
                    }
                }
                opponent->pip->resistmode = 0;
                opponent->pip->msgcount++;
                currutmp->pip->msgcount++;
                strcpy(opponent->pip->msg, buf);
                strcpy(mymsg[currutmp->pip->msgcount%8], buf);
                currutmp->pip->nodone = 2;      /*做完*/
                opponent->pip->nodone = 1;
                break;

            case '3':
                clrchyiuan(8, b_lines - 4);
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
                        sprintf(buf, "治療後，體力提高%d，疲勞降低%d", oldhp, oldtired);
                        vmsg(buf);
                        sprintf(buf, "\x1b[1;33m%s \x1b[37m使用魔法治療之後，體力提高 \x1b[36m%d \x1b[37m點，疲勞降低 \x1b[36m%d \x1b[37m點\x1b[m", d.name, oldhp, oldtired);
                    }
                    else
                    {
                        if (random() % 15 == 0)
                        {
                            vmsg("竟然沒打中..:~~~");
                            sprintf(buf, "\x1b[1;33m%s \x1b[37m對 \x1b[33m%s\x1b[37m 施展魔法攻擊，但是沒有打中...",
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
                            d.mskill += random() % 2 + 2;
                            sprintf(buf, "魔法攻擊，對方體力減低%d", dinjure);
                            vmsg(buf);
                            sprintf(buf, "\x1b[1;33m%s \x1b[37m施展了魔法攻擊，\x1b[33m%s \x1b[37m的體力減低 \x1b[31m%d \x1b[37m點\x1b[m",
                                    d.name, opponent->pip->name, dinjure);
                        }
                    }

                    opponent->pip->msgcount++;
                    currutmp->pip->msgcount++;
                    strcpy(opponent->pip->msg, buf);
                    strcpy(mymsg[currutmp->pip->msgcount%8], buf);
                    /*恢復體力是用d.hp和d.maxhp去 所以得更新*/
                    currutmp->pip->hp = d.hp;
                    currutmp->pip->mp = d.mp;
                    currutmp->pip->nodone = 2;  /*做完*/
                    opponent->pip->nodone = 1;
                    pip_set_currutmp();
                }
                break;

            case '4':
                currutmp->pip->resistmode = 1;
                vmsg("小雞加強防禦啦....");
                sprintf(buf, "\x1b[1;33m%s \x1b[37m加強防禦，準備全力抵擋 \x1b[33m%s \x1b[37m的下一招\x1b[m",
                        d.name, opponent->pip->name);
                opponent->pip->msgcount++;
                currutmp->pip->msgcount++;
                strcpy(opponent->pip->msg, buf);
                strcpy(mymsg[currutmp->pip->msgcount%8], buf);
                currutmp->pip->nodone = 2;      /*做完*/
                opponent->pip->nodone = 1;
                break;
            case '5':
                add_io(fd, 60);
                pip_fight_feed();
                add_io(fd, 1);
                if (d.nodone != 1)
                {
                    sprintf(buf, "\x1b[1;33m%s \x1b[37m補充了身上的能量，體力或法力有顯著的提升\x1b[m", d.name);
                    opponent->pip->msgcount++;
                    currutmp->pip->msgcount++;
                    strcpy(opponent->pip->msg, buf);
                    strcpy(mymsg[currutmp->pip->msgcount%8], buf);
                    /*恢復體力是用d.hp和d.maxhp去 所以得更新*/
                    currutmp->pip->hp = d.hp;
                    currutmp->pip->mp = d.mp;
                    currutmp->pip->nodone = 2;  /*做完*/
                    opponent->pip->nodone = 1;
                    pip_set_currutmp();
                }
                break;
            case '6':
                opponent->pip->msgcount++;
                currutmp->pip->msgcount++;
                if (random() % 20 >= 18 || (random() % 20 > 13 && d.speed <= opponent->pip->speed))
                {
                    vmsg("想逃跑，卻失敗了...");
                    sprintf(buf, "\x1b[1;33m%s \x1b[37m想先逃跑再說...但卻失敗了...\x1b[m", d.name);
                    strcpy(opponent->pip->msg, buf);
                    strcpy(mymsg[currutmp->pip->msgcount%8], buf);
                }
                else
                {
                    sprintf(buf, "\x1b[1;33m%s \x1b[37m自覺打不過對方，所以決定先逃跑再說...\x1b[m", d.name);
                    strcpy(opponent->pip->msg, buf);
                    strcpy(mymsg[currutmp->pip->msgcount%8], buf);
                    currutmp->pip->pipmode = 3;
                    clear();
                    vs_head("電子養小雞", BoardName);
                    move(10, 0);
                    outs_centered("            \x1b[1;31m┌──────────────────────┐\x1b[m\n");
                    prints_centered("            \x1b[1;31m│  \x1b[37m實力不強的小雞 \x1b[33m%-10s                 \x1b[31m│\x1b[m\n", d.name);
                    prints_centered("            \x1b[1;31m│  \x1b[37m在與對手 \x1b[32m%-10s \x1b[37m戰鬥後落跑啦          \x1b[31m│\x1b[m\n", opponent->pip->name);
                    outs_centered("            \x1b[1;31m└──────────────────────┘\x1b[m\n");
                    currutmp->pip->leaving = 0;
                    add_io(fd, 60);
                    vmsg("三十六計 走為上策...");
                }
                currutmp->pip->nodone = 2;      /*做完*/
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
            vs_head("電子養小雞", BoardName);
            move(10, 0);
            outs_centered("            \x1b[1;31m┌──────────────────────┐\x1b[m\n");
            prints_centered("            \x1b[1;31m│  \x1b[37m英勇的小雞 \x1b[33m%-10s                     \x1b[31m│\x1b[m\n", d.name);
            prints_centered("            \x1b[1;31m│  \x1b[37m打敗了對方小雞 \x1b[32m%-10s                 \x1b[31m│\x1b[m\n", opponent->pip->name);
            outs_centered("            \x1b[1;31m└──────────────────────┘\x1b[m");
            currutmp->pip->leaving = 0;
            add_io(fd, 60);
            if (opponent->pip->hp <= 0)
                vmsg("對方死掉囉..所以你贏囉..");
            else if (opponent->pip->hp > 0)
                vmsg("對方落跑囉..所以算你贏囉.....");
        }
        if (gameover != 1 && (opponent->pip->pipmode == 2 || currutmp->pip->pipmode == 1))
        {
            clear();
            vs_head("電子養小雞", BoardName);
            move(10, 0);
            outs_centered("            \x1b[1;31m┌──────────────────────┐\x1b[m\n");
            prints_centered("            \x1b[1;31m│  \x1b[37m可憐的小雞 \x1b[33m%-10s                     \x1b[31m│\x1b[m\n", d.name);
            prints_centered("            \x1b[1;31m│  \x1b[37m在與 \x1b[32m%-10s \x1b[37m的戰鬥中，                \x1b[31m│\x1b[m\n", opponent->pip->name);
            outs_centered("            \x1b[1;31m│  \x1b[37m不幸地打輸了，記者現場特別報導.........   \x1b[31m│\x1b[m\n");
            outs_centered("            \x1b[1;31m└──────────────────────┘\x1b[m\n");
            currutmp->pip->leaving = 0;
            add_io(fd, 60);
            vmsg("小雞打輸了....");
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
#endif  /* #ifdef  HAVE_PIP_FIGHT */

/*---------------------------------------------------------------------------*/
/* 結局函式                                                                  */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static int pip_future_decide(int *modeall_purpose);
static int pip_marry_decide(void);

static int pip_endingblack(char *buf, int *m, int *n, int *grade);
static int pip_endingsocial(char *buf, int *m, int *n, int *grade);
static int pip_endingmagic(char *buf, int *m, int *n, int *grade);
static int pip_endingcombat(char *buf, int *m, int *n, int *grade);
static int pip_endingfamily(char *buf, int *m, int *n, int *grade);
static int pip_endingall_purpose(char *buf, int *m, int *n, int *grade, int mode);
static int pip_endingart(char *buf, int *m, int *n, int *grade);

static int pip_max_worktime(int *num);

/*--------------------------------------------------------------------------*/
/*  結局參數設定                                                            */
/*--------------------------------------------------------------------------*/

static int /*結局畫面*/
pip_ending_screen(void)
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
    move(1, (d_cols>>1) + 9);
    outs("\x1b[1;33m╔═══╗╔══╮╗╔═══╮╔═══╗╔══╮╗╭═══╮\x1b[0m");
    move(2, (d_cols>>1) + 9);
    outs("\x1b[1;37m║      ║║    ║║║      ║║      ║║    ║║║      ║\x1b[0m");
    move(3, (d_cols>>1) + 9);
    outs("\x1b[0;37m║    ═╣║    ║║║  ╭╮║╚═╗╔╝║    ║║║  ╔═╗\x1b[0m");
    move(4, (d_cols>>1) + 9);
    outs("\x1b[0;37m║    ═╣║  ║  ║║  ╰╯║╔═╝╚╗║  ║  ║║  ╰╯║\x1b[0m");
    move(5, (d_cols>>1) + 9);
    outs("\x1b[1;37m║      ║║  ║  ║║      ║║      ║║  ║  ║║      ║\x1b[0m");
    move(6, (d_cols>>1) + 9);
    outs("\x1b[1;35m╚═══╝╚═╰═╝╚═══╯╚═══╝╚═╰═╝╰═══╯\x1b[0m");
    move(b_lines - 16, (d_cols>>1) + 8);
    outs("\x1b[1;31m──────────\x1b[41;37m " NICKNAME PIPNAME "結局報告 \x1b[0;1;31m──────────\x1b[0m");
    move(b_lines - 14, (d_cols>>1) + 10);
    outs("\x1b[1;36m這個時間不知不覺地還是到臨了...\x1b[0m");
    move(b_lines - 12, (d_cols>>1) + 10);
    prints("\x1b[1;37m\x1b[33m%s\x1b[37m 得離開你的溫暖懷抱，自己一隻雞在外面求生存了.....\x1b[0m", d.name);
    move(b_lines - 10, (d_cols>>1) + 10);
    outs("\x1b[1;36m在你照顧教導他的這段時光，讓他接觸了很多領域，培養了很多的能力....\x1b[0m");
    move(b_lines - 8, (d_cols>>1) + 10);
    prints("\x1b[1;37m因為這些，讓小雞 \x1b[33m%s\x1b[37m 之後的生活，變得更多采多姿了........\x1b[0m", d.name);
    move(b_lines - 6, (d_cols>>1) + 10);
    outs("\x1b[1;36m對於你的關心，你的付出，你所有的愛......\x1b[0m");
    move(b_lines - 4, (d_cols>>1) + 10);
    prints("\x1b[1;37m\x1b[33m%s\x1b[37m 會永遠都銘記在心的....\x1b[0m", d.name);
    vmsg("接下來看未來發展");
    clrchyiuan(b_lines - 16, b_lines - 4);
    move(b_lines - 16, (d_cols>>1) + 8);
    outs("\x1b[1;34m──────────\x1b[44;37m " NICKNAME PIPNAME "未來發展 \x1b[0;1;34m──────────\x1b[0m");
    move(b_lines - 14, (d_cols>>1) + 10);
    prints("\x1b[1;36m透過水晶球，讓我們一起來看 \x1b[33m%s\x1b[36m 的未來發展吧.....\x1b[0m", d.name);
    move(b_lines - 12, (d_cols>>1) + 10);
    prints("\x1b[1;37m小雞 \x1b[33m%s\x1b[37m 後來%s....\x1b[0m", d.name, endbuf1);
    move(b_lines - 10, (d_cols>>1) + 10);
    prints("\x1b[1;36m因為他的之前的努力，使得他在這一方面%s....\x1b[0m", endbuf2);
    move(b_lines - 8, (d_cols>>1) + 10);
    prints("\x1b[1;37m至於小雞的婚姻狀況，他後來%s，婚姻算是很美滿.....\x1b[0m", endbuf3);
    move(b_lines - 6, (d_cols>>1) + 10);
    outs("\x1b[1;36m嗯..這是一個不錯的結局唷..........\x1b[0m");
    vmsg("我想  你一定很感動吧.....");
    show_ending_pic(0);
    vmsg("看一看分數囉");
    endgrade = pip_game_over(endgrade);
    /* inmoney(endgrade*10*ba);
      inexp(endgrade*ba);*/
    sprintf(buf, "/bin/rm %s", get_path(cuser.userid, "chicken"));
    system(buf);
    sprintf(buf, "得到 %d 元，%d 點經驗值", endgrade*10*ba, endgrade*10);
    vmsg(buf);
    vmsg("下一頁是小雞資料  趕快copy下來作紀念");
    pip_data_list(cuser.userid);
    vmsg("歡迎再來挑戰....");
    /*記錄開始*/
    now = time(0);
    sprintf(buf, "\x1b[1;35m───────────────────────────────────────\x1b[0m\n");
    pip_log_record(buf);
    sprintf(buf, "\x1b[1;37m在 \x1b[33m%s \x1b[37m的時候，\x1b[36m%s \x1b[37m的小雞 \x1b[32m%s\x1b[37m 出現了結局\x1b[0m\n", Cdate(&now), cuser.userid, d.name);
    pip_log_record(buf);
    sprintf(buf, "\x1b[1;37m小雞 \x1b[32m%s \x1b[37m努力加強自己，後來%s\x1b[0m\n\x1b[1;37m因為之前的努力，使得在這一方面%s\x1b[0m\n", d.name, endbuf1, endbuf2);
    pip_log_record(buf);
    sprintf(buf, "\x1b[1;37m至於婚姻狀況，他後來%s，婚姻算是很美滿.....\x1b[0m\n\n\x1b[1;37m小雞 \x1b[32n%s\x1b[37m 的總積分＝ \x1b[33m%d\x1b[0m\n", endbuf3, d.name, endgrade);
    pip_log_record(buf);
    sprintf(buf, "\x1b[1;35m───────────────────────────────────────\x1b[0m\n");
    pip_log_record(buf);
    /*記錄終止*/
    d.death = 3;
    pipdie("\x1b[1;31m遊戲結束囉...\x1b[m  ", 3);
    return 0;
}

static int
pip_ending_decide(
char *endbuf1, char *endbuf2, char *endbuf3,
int *endmode, int *endgrade)
{
    const char *name[8][2] = {{"男的", "女的"},
        {"嫁給王子",   "娶了公主"},
        {"嫁給你",     "娶了你"},
        {"嫁給商人Ａ", "娶了女商人Ａ"},
        {"嫁給商人Ｂ", "娶了女商人Ｂ"},
        {"嫁給商人Ｃ", "娶了女商人Ｃ"},
        {"嫁給商人Ｄ", "娶了女商人Ｄ"},
        {"嫁給商人Ｅ", "娶了女商人Ｅ"}
    };
    int m = 0, n = 0, grade = 0;
    int modeall_purpose = 0;
    char buf1[256];
    char buf2[256];

    *endmode = pip_future_decide(&modeall_purpose);
    switch (*endmode)
    {
        /*1:暗黑 2:藝術 3:萬能 4:戰士 5:魔法 6:社交 7:家事*/
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
        sprintf(buf2, "非常的順利..");
    }
    else if (n == 2)
    {
        *endgrade = grade + 100;
        sprintf(buf2, "表現還不錯..");
    }
    else if (n == 3)
    {
        *endgrade = grade - 10;
        sprintf(buf2, "常遇到很多問題....");
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
            sprintf(buf2, "娶了同行的女孩");
        else
            sprintf(buf2, "嫁給了同行的男生");
    }
    strcpy(endbuf3, buf2);
    return 0;
}
/*結局判斷*/
/*1:暗黑 2:藝術 3:萬能 4:戰士 5:魔法 6:社交 7:家事*/
static int
pip_future_decide(
int *modeall_purpose)
{
    int endmode;
    /*暗黑*/
    if ((d.ethics == 0 && d.offense >= 100) || (d.ethics > 0 && d.ethics < 50 && d.offense >= 250))
        endmode = 1;
    /*藝術*/
    else if (d.art > d.hexp && d.art > d.mexp && d.art > d.hskill && d.art > d.mskill &&
             d.art > d.social && d.art > d.family && d.art > d.homework && d.art > d.wisdom &&
             d.art > d.charm && d.art > d.belief && d.art > d.manners && d.art > d.speech &&
             d.art > d.cookskill && d.art > d.love)
        endmode = 2;
    /*戰鬥*/
    else if (d.hexp >= d.social && d.hexp >= d.mexp && d.hexp >= d.family)
    {
        *modeall_purpose = 1;
        if (d.hexp > d.social + 50 || d.hexp > d.mexp + 50 || d.hexp > d.family + 50)
            endmode = 4;
        else
            endmode = 3;
    }
    /*魔法*/
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
/*結婚的判斷*/
static int
pip_marry_decide(void)
{
    int grade;
    if (d.lover != 0)
    {
        /* 3 4 5 6 7:商人 */
        d.lover = d.lover;
        grade = 80;
    }
    else
    {
        if (d.royalJ >= d.relation && d.royalJ >= 100)
        {
            d.lover = 1;  /*王子*/
            grade = 200;
        }
        else if (d.relation > d.royalJ && d.relation >= 100)
        {
            d.lover = 2;  /*父親或母親*/
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


static int
pip_endingblack( /*暗黑*/
char *buf,
int *m, int *n, int *grade)
{
    if (d.offense >= 500 && d.mexp >= 500) /*魔王*/
    {
        *m = 1;
        if (d.mexp >= 1000)
            *n = 1;
        else if (d.mexp < 1000 && d.mexp >= 800)
            *n = 2;
        else
            *n = 3;
    }

    else if (d.hexp >= 600)  /*流氓*/
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
    else if (d.hexp >= 320 && d.character > 200 && d.charm < 200)       /*黑街老大*/
    {
        *m = 4;
        if (d.hexp >= 400)
            *n = 1;
        else if (d.hexp < 400 && d.hexp >= 360)
            *n = 2;
        else
            *n = 3;
    }
    else if (d.character >= 200 && d.charm >= 200 && d.speech > 70 && d.toman > 70)  /*高級娼婦*/
    {
        *m = 5;
        if (d.charm >= 300)
            *n = 1;
        else if (d.charm < 300 && d.charm >= 250)
            *n = 2;
        else
            *n = 3;
    }

    else if (d.wisdom >= 450)  /*詐騙師*/
    {
        *m = 6;
        if (d.wisdom >= 550)
            *n = 1;
        else if (d.wisdom < 550 && d.wisdom >= 500)
            *n = 2;
        else
            *n = 3;
    }

    else /*流鶯*/
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


static int
pip_endingsocial( /*社交*/
char *buf,
int *m, int *n, int *grade)
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

static int
pip_endingmagic( /*魔法*/
char *buf,
int *m, int *n, int *grade)
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
        if (d.affect > d.wisdom && d.affect > d.belief && d.ethics > 100)
        {
            *m = 1;
            if (d.ethics >= 800)
                *n = 1;
            else if (d.ethics < 800 && d.ethics >= 400)
                *n = 2;
            else
                *n = 3;
        }
        else if (d.ethics < 50)
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
        if (d.ethics >= 50)
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

static int
pip_endingcombat( /*戰鬥*/
char *buf,
int *m, int *n, int *grade)
{
    int class;
    if (d.hexp > 1500) class = 1;
    else if (d.hexp > 1000) class = 2;
    else if (d.hexp > 800) class = 3;
    else class = 4;

    switch (class)
    {
    case 1:
        if (d.affect > d.wisdom && d.affect > d.belief && d.ethics > 100)
        {
            *m = 1;
            if (d.ethics >= 800)
                *n = 1;
            else if (d.ethics < 800 && d.ethics >= 400)
                *n = 2;
            else
                *n = 3;
        }
        else if (d.ethics < 50)
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
        if (d.character >= 300 && d.ethics > 50)
        {
            *m = 3;
            if (d.ethics >= 300 && d.charm >= 300)
                *n = 1;
            else if (d.ethics < 300 && d.charm < 300 && d.ethics >= 250 && d.charm >= 250)
                *n = 2;
            else
                *n = 3;
        }
        else if (d.character < 300 && d.ethics > 50)
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
        if (d.character >= 400 && d.ethics > 50)
        {
            *m = 5;
            if (d.ethics >= 300)
                *n = 1;
            else if (d.ethics < 300 && d.ethics >= 150)
                *n = 2;
            else
                *n = 3;
        }
        else if (d.character < 400 && d.ethics > 50)
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
        if (d.ethics >= 50)
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


static int
pip_endingfamily( /*家事*/
char *buf,
int *m, int *n, int *grade)
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


static int
pip_endingall_purpose( /*萬能*/
char *buf,
int *m, int *n, int *grade,
int mode)
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
    else  // mode == 4
        data = d.family;
    if (data > 1000) class = 1;
    else if (data > 800) class = 2;
    else if (data > 500) class = 3;
    else if (data > 300) class = 4;
    else class = 5;

    data = pip_max_worktime(&num);
    switch (class)
    {
    case 1:
        if (d.character >= 1000)
        {
            *m = 1;
            if (d.ethics >= 900)
                *n = 1;
            else if (d.ethics < 900 && d.ethics >= 600)
                *n = 2;
            else
                *n = 3;
        }
        else
        {
            *m = 2;
            if (d.ethics >= 650)
                *n = 1;
            else if (d.ethics < 650 && d.ethics >= 400)
                *n = 2;
            else
                *n = 3;
        }
        break;

    case 2:
        if (d.belief > d.ethics && d.belief > d.wisdom)
        {
            *m = 3;
            if (d.ethics >= 500)
                *n = 1;
            else if (d.ethics < 500 && d.ethics >= 250)
                *n = 2;
            else
                *n = 3;
        }
        else if (d.ethics > d.belief && d.ethics > d.wisdom)
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
        if (d.belief > d.ethics && d.belief > d.wisdom)
        {
            *m = 6;
            if (d.belief >= 400)
                *n = 1;
            else if (d.belief < 400 && d.belief >= 150)
                *n = 2;
            else
                *n = 3;
        }
        else if (d.ethics > d.belief && d.ethics > d.wisdom)
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
                if (d.love > 100)       *n = 1;
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
                if ((d.belief + d.ethics) > 600) *n = 1;
                else if ((d.belief + d.ethics) > 200) *n = 2;
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
            if (d.ethics > 400)
                *n = 1;
            else if (d.ethics > 200)
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
                if ((d.belief + d.ethics) > 600) *n = 1;
                else if ((d.belief + d.ethics) > 200) *n = 2;
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

static int
pip_endingart( /*藝術*/
char *buf,
int *m, int *n, int *grade)
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

static int
pip_max_worktime(
int *num)
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

static int pip_game_over(
int endgrade)
{
    long gradebasic;
    long gradeall;

    gradebasic = (d.maxhp + d.wrist + d.wisdom + d.character + d.charm + d.ethics + d.belief + d.affect) / 10 - d.offense;
    clrchyiuan(1, b_lines);
    gradeall = gradebasic + endgrade;
    move(8, (d_cols>>1) + 17);
    outs("\x1b[1;36m感謝您玩完整個" NICKNAME "小雞的遊戲.....\x1b[0m");
    move(10, (d_cols>>1) + 17);
    outs("\x1b[1;37m經過系統計算的結果：\x1b[0m");
    move(12, (d_cols>>1) + 17);
    prints("\x1b[1;36m您的小雞 \x1b[37m%s \x1b[36m總得分＝ \x1b[1;5;33m%ld \x1b[0m", d.name, gradeall);
    return gradeall;
}

static int pip_divine(void) /*占卜師來訪*/
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
    clrchyiuan(6, b_lines - 6);
    move(10, (d_cols>>1) + 14);
    outs("\x1b[1;33;5m叩叩叩...\x1b[0;1;37m突然傳來陣陣的敲門聲.........\x1b[0m");
    vmsg("去瞧瞧是誰吧......");
    clrchyiuan(6, b_lines - 6);
    move(10, (d_cols>>1) + 14);
    outs("\x1b[1;37;46m    原來是雲遊四海的占卜師來訪了.......    \x1b[0m");
    vmsg("開門讓他進來吧....");
    if (d.money >= money)
    {
        randvalue = random() % 5;
        sprintf(buf, "你要占卜嗎? 要花%ld元喔...[y/N]: ", money);
        getdata(12, 14, buf, ans, 2, 1, 0);
        if (ans[0] == 'y' || ans[0] == 'Y')
        {
            pip_ending_decide(endbuf1, endbuf2, endbuf3, &endmode, &endgrade);
            if (randvalue == 0)
                sprintf(buf, "\x1b[1;37m  你的小雞%s以後可能的身份是%s  \x1b[m", d.name, endmodemagic[2+random()%5].girl);
            else if (randvalue == 1)
                sprintf(buf, "\x1b[1;37m  你的小雞%s以後可能的身份是%s  \x1b[m", d.name, endmodecombat[2+random()%6].girl);
            else if (randvalue == 2)
                sprintf(buf, "\x1b[1;37m  你的小雞%s以後可能的身份是%s  \x1b[m", d.name, endmodeall_purpose[6+random()%15].girl);
            else if (randvalue == 3)
                sprintf(buf, "\x1b[1;37m  你的小雞%s以後可能的身份是%s  \x1b[m", d.name, endmodeart[2+random()%6].girl);
            else if (randvalue == 4)
                sprintf(buf, "\x1b[1;37m  你的小雞%s以後可能的身份是%s  \x1b[m", d.name, endbuf1);
            d.money -= money;
            clrchyiuan(6, b_lines - 6);
            move(10, (d_cols>>1) + 14);
            outs("\x1b[1;33m在我占卜結果看來....\x1b[m");
            move(12, (d_cols>>1) + 14);
            outs(buf);
            vmsg("謝謝惠顧，有緣再見面了.(不準不能怪我喔)");
        }
        else
        {
            vmsg("你不想占卜啊?..真可惜..那只有等下次吧...");
        }
    }
    else
    {
        vmsg("你的錢不夠喔..真是可惜..等下次吧...");
    }
    return 0;
}

static int
pip_money(void)
{
    char buf[100], ans[10];

    int money = -1;
    if (!d.name[0] || d.death) return 0;
    clrchyiuan(6, b_lines - 6);
    /* move(12, 0);
    clrtobot();*/
    prints("你身上有 %d 次點歌次數，雞金 %d 元\n", cuser.request, d.money);
    outs("\n一次換一千雞金唷!!\n");
    while (money < 0 || money > cuser.request)
    {
        getdata(10, 0, "要換多少次? ", ans, 10, LCECHO, 0);
        if (!ans[0]) return 0;
        money = atol(ans);
    }
    sprintf(buf, "是否要轉換 %d 次 為 %d 雞金? [y/N]: ", money, money*1000);
    getdata(11, 0, buf, ans, 3, LCECHO, 0);
    if (ans[0] == 'y' || ans[0] == 'Y')
    {
        ACCT acct;
        acct_load(&acct, cuser.userid);
        /* demoney(money);*/
        d.money += (money * 1000);
        cuser.request -= money;
        acct.request = cuser.request;
        acct_save(&acct);
        pip_write_file();
        sprintf(buf, "你身上有 %d 次點歌次數，雞金 %d 元", cuser.request, d.money);
    }
    else
        sprintf(buf, "取消.....");

    vmsg(buf);
    return 0;
}

static int pip_query(void)  /*拜訪小雞*/
{
    int id;
    char genbuf[STRLEN];

    vs_bar("拜訪同伴");
    usercomplete(msg_uid, genbuf);
    if (genbuf[0])
    {
        move(2, 0);
        if ( ( id = acct_userno(genbuf) ) )
        {
            pip_read(genbuf);
            vmsg("觀摩一下別人的小雞...:p");
        }
        else
        {
            outs(err_uid);
            clrtoeol();
        }
    }
    return 0;
}

static int
pip_read(
const char *userid)
{
    FILE *fs;
    char buf[200];
    /*char yo[14][5]={"誕生", "嬰兒", "幼兒", "兒童", "青年", "少年", "成年",
                      "壯年", "壯年", "壯年", "更年", "老年", "老年", "古稀"};*/
    char yo[12][5] = {"誕生", "嬰兒", "幼兒", "兒童", "少年", "青年",
                      "成年", "壯年", "更年", "老年", "古稀", "神仙"
                     };
    int pc1, age1, age = 0;

    int year1, month1, day1, sex1, death1, nodone1, relation1, liveagain1, chickenmode1, level1, exp1, dataE1;
    int hp1, maxhp1, weight1, tired1, sick1, shit1, wrist1, bodyA1, bodyB1, bodyC1, bodyD1, bodyE1;
    int social1, family1, hexp1, mexp1, tmpA1, tmpB1, tmpC1, tmpD1, tmpE1;
    int mp1, maxmp1, attack1, resist1, speed1, hskill1, mskill1, mresist1, magicmode1, specialmagic1, fightC1, fightD1, fightE1;
    int weaponhead1, weaponrhand1, weaponlhand1, weaponbody1, weaponfoot1, weaponA1, weaponB1, weaponC1, weaponD1, weaponE1;
    int toman1, character1, love1, wisdom1, art1, ethics1, brave1, homework1, charm1, manners1, speech1, cookskill1, learnA1, learnB1, learnC1, learnD1, learnE1;
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

    /* sprintf(buf, "home/%s/chicken", userid);*/
    usr_fpath(buf, userid, "chicken");
    /* currutmp->destuid = userid;*/

    if ( ( fs = fopen(buf, "r") ) )
    {
        fgets(buf, 80, fs);
        age = ((time_t) atol(buf)) / 60 / 30;

        if (age == 0)                     /*誕生*/
            age1 = 0;
        else if (age == 1)                /*嬰兒*/
            age1 = 1;
        else if (age >= 2 && age <= 5)    /*幼兒*/
            age1 = 2;
        else if (age >= 6 && age <= 12)   /*兒童*/
            age1 = 3;
        else if (age >= 13 && age <= 15)  /*少年*/
            age1 = 4;
        else if (age >= 16 && age <= 18)  /*青年*/
            age1 = 5;
        else if (age >= 19 && age <= 35)  /*成年*/
            age1 = 6;
        else if (age >= 36 && age <= 45)  /*壯年*/
            age1 = 7;
        else if (age >= 45 && age <= 60)  /*更年*/
            age1 = 8;
        else if (age >= 60 && age <= 70)  /*老年*/
            age1 = 9;
        else if (age >= 70 && age <= 100) /*古稀*/
            age1 = 10;
        else if (age > 100)               /*神仙*/
            age1 = 11;

        fscanf(fs,
               "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %s %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
               &(year1), &(month1), &(day1), &(sex1), &(death1), &(nodone1), &(relation1), &(liveagain1), &(chickenmode1), &(level1), &(exp1), &(dataE1),
               &(hp1), &(maxhp1), &(weight1), &(tired1), &(sick1), &(shit1), &(wrist1), &(bodyA1), &(bodyB1), &(bodyC1), &(bodyD1), &(bodyE1),
               &(social1), &(family1), &(hexp1), &(mexp1), &(tmpA1), &(tmpB1), &(tmpC1), &(tmpD1), &(tmpE1),
               &(mp1), &(maxmp1), &(attack1), &(resist1), &(speed1), &(hskill1), &(mskill1), &(mresist1), &(magicmode1), &(specialmagic1), &(fightC1), &(fightD1), &(fightE1),
               &(weaponhead1), &(weaponrhand1), &(weaponlhand1), &(weaponbody1), &(weaponfoot1), &(weaponA1), &(weaponB1), &(weaponC1), &(weaponD1), &(weaponE1),
               &(toman1), &(character1), &(love1), &(wisdom1), &(art1), &(ethics1), &(brave1), &(homework1), &(charm1), &(manners1), &(speech1), &(cookskill1), &(learnA1), &(learnB1), &(learnC1), &(learnD1), &(learnE1),
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
        prints("這是%s養的小雞：\n", userid);

        if (death1 == 0)
        {
            prints("\x1b[1;32mName：%-10s\x1b[m  生日：%2d年%2d月%2d日   年齡：%2d歲  狀態：%s  錢錢：%d\n"
                   "生命：%3d/%-3d  快樂：%-4d  滿意：%-4d  氣質：%-4d  智慧：%-4d  體重：%-4d\n"
                   "大補丸：%-4d   食物：%-4d  零食：%-4d  疲勞：%-4d  髒髒：%-4d  病氣：%-4d\n",
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
            move(b_lines - 5, 0);
            if (shit1 == 0) outs("很乾淨..");
            if (shit1 > 40 && shit1 < 60) outs("臭臭的..");
            if (shit1 >= 60 && shit1 < 80) outs("好臭喔..");
            if (shit1 >= 80 && shit1 < 100) outs("\x1b[1;34m快臭死了..\x1b[m");
            if (shit1 >= 100) { outs("\x1b[1;31m臭死了..\x1b[m"); return -1; }

            pc1 = hp1 * 100 / maxhp1;
            if (pc1 == 0) { outs("餓死了.."); return -1; }
            if (pc1 < 20) outs("\x1b[1;35m全身無力中.快餓死了.\x1b[m");
            if (pc1 < 40 && pc1 >= 20) outs("體力不太夠..想吃點東西..");
            if (pc1 < 100 && pc1 >= 80) outs("嗯～肚子飽飽有體力..");
            if (pc1 >= 100) outs("\x1b[1;34m快撐死了..\x1b[m");

            pc1 = tired1;
            if (pc1 < 20) outs("精神抖擻中..");
            if (pc1 < 80 && pc1 >= 60) outs("\x1b[1;34m有點小累..\x1b[m");
            if (pc1 < 100 && pc1 >= 80) { outs("\x1b[1;31m好累喔，快不行了..\x1b[m"); }
            if (pc1 >= 100) { outs("累死了..."); return -1; }

            pc1 = 60 + 10 * age;
            if (weight1 < (pc1 + 30) && weight1 >= (pc1 + 10)) outs("有點小胖..");
            if (weight1 < (pc1 + 50) && weight1 >= (pc1 + 30)) outs("太胖了..");
            if (weight1 > (pc1 + 50)) { outs("胖死了..."); return -1; }

            if (weight1 < (pc1 - 50)) { outs("瘦死了.."); return -1; }
            if (weight1 > (pc1 - 30) && weight1 <= (pc1 - 10)) outs("有點小瘦..");
            if (weight1 > (pc1 - 50) && weight1 <= (pc1 - 30)) outs("太瘦了..");

            if (sick1 < 75 && sick1 >= 50) outs("\x1b[1;34m生病了..\x1b[m");
            if (sick1 < 100 && sick1 >= 75) { outs("\x1b[1;31m病重!!..\x1b[m"); }
            if (sick1 >= 100) { outs("病死了.!."); return -1; }

            pc1 = happy1;
            if (pc1 < 20) outs("\x1b[1;31m很不快樂..\x1b[m");
            if (pc1 < 40 && pc1 >= 20) outs("不快樂..");
            if (pc1 < 95 && pc1 >= 80) outs("快樂..");
            if (pc1 <= 100 && pc1 >= 95) outs("很快樂..");

            pc1 = satisfy1;
            if (pc1 < 40) outs("\x1b[31;1m不滿足..\x1b[m");
            if (pc1 < 95 && pc1 >= 80) outs("滿足..");
            if (pc1 <= 100 && pc1 >= 95) outs("很滿足..");
        }
        else if (death1 == 1)
        {
            show_die_pic(2);
            move(14, (d_cols>>1) + 20);
            outs("可憐的小雞嗚呼哀哉了");
        }
        else if (death1 == 2)
        {
            show_die_pic(3);
        }
        else if (death1 == 3)
        {
            move(5, 0);
            outs("遊戲已經玩到結局囉....");
        }
        else
        {
            vmsg("檔案損毀了....");
        }
    }   /* 有養小雞 */
    else
    {
        move(1, 0);
        clrtobot();
        vmsg("這一家的人沒有養小雞......");
    }

    return 0;
}

/*---------------------------------------------------------------------------*/
/* 系統選單:個人資料  小雞放生  特別服務                                     */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static char weaponhead[7][10] =
{
    "沒有裝備",
    "塑膠帽子",
    "牛皮小帽",
    "㊣安全帽",
    "鋼鐵頭盔",
    "魔法髮箍",
    "黃金聖盔"
};


static char weaponrhand[10][10] =
{
    "沒有裝備",
    "大木棒",
    "金屬扳手",
    "青銅劍",
    "晴雷劍",
    "蟬翼刀",
    "忘情劍",
    "獅頭寶刀",
    "屠龍刀",
    "黃金聖杖"
};

static char weaponlhand[8][10] =
{
    "沒有裝備",
    "大木棒",
    "金屬扳手",
    "木盾",
    "不鏽鋼盾",
    "白金之盾",
    "魔法盾",
    "黃金聖盾"
};


static char weaponbody[7][10] =
{
    "沒有裝備",
    "塑膠冑甲",
    "特級皮甲",
    "鋼鐵盔甲",
    "魔法披風",
    "白金盔甲",
    "黃金聖衣"
};

static char weaponfoot[8][12] =
{
    "沒有裝備",
    "塑膠拖鞋",
    "東洋木屐",
    "特級雨鞋",
    "NIKE運動鞋",
    "鱷魚皮靴",
    "飛天魔靴",
    "黃金聖靴"
};

static int
pip_system_freepip(void)
{
    char buf[256];
    clrchyiuan(b_lines - 2, b_lines);
    getdata(b_lines - 1, 1, "真的要放生嗎？(y/N): ", buf, 2, 1, 0);
    if (buf[0] != 'y' && buf[0] != 'Y') return 0;
    sprintf(buf, "%s 被狠心的 %s 丟掉了~", d.name, cuser.userid);
    vmsg(buf);
    d.death = 2;
    pipdie("\x1b[1;31m被狠心丟棄:~~\x1b[0m", 2);
    return 0;
}


static int
pip_system_service(void)
{
    int pipkey;
    int oldchoice;
    char buf[200];
    char oldname[21];
    time_t now;

    move(b_lines - 1, 0);
    clrtoeol();
    move(b_lines, 0);
    prints("\x1b[1;44m  服務項目  \x1b[46m[1]命名大師 [2]變性手術 [3]結局設局                               %*s\x1b[0m", d_cols, "");
    pipkey = vkey();

    switch (pipkey)
    {
    case '1':
        move(b_lines - 1, 0);
        clrtobot();
        getdata(b_lines - 1, 1, "幫小雞重新取個好名字： ", buf, 11, DOECHO, NULL);
        if (!buf[0])
        {
            vmsg("等一下想好再來好了  :)");
            break;
        }
        else
        {
            strcpy(oldname, d.name);
            strcpy(d.name, buf);
            /*改名記錄*/
            now = time(0);
            sprintf(buf, "\x1b[1;37m%s %-11s把小雞 [%s] 改名成 [%s] \x1b[0m\n", Cdate(&now), cuser.userid, oldname, d.name);
            pip_log_record(buf);
            vmsg("嗯嗯  換一個新的名字喔...");
        }
        break;

    case '2':  /*變性*/
        move(b_lines - 1, 0);
        clrtobot();
        /*1:公 2:母 */
        if (d.sex == 1)
        {
            oldchoice = 2; /*公-->母*/
            move(b_lines - 1, 0);
            outs("\x1b[1;33m將小雞由\x1b[32m♂\x1b[33m變性成\x1b[35m♀\x1b[33m的嗎？ \x1b[37m[y/N]\x1b[0m");
        }
        else
        {
            oldchoice = 1; /*母-->公*/
            move(b_lines - 1, 0);
            outs("\x1b[1;33m將小雞由\x1b[35m♀\x1b[33m變性成\x1b[35m♂\x1b[33m的嗎？ \x1b[37m[y/N]\x1b[0m");
        }
        move(b_lines, 0);
        prints("\x1b[1;44m  服務項目  \x1b[46m[1]命名大師 [2]變性手術 [3]結局設局                               %*s\x1b[0m", d_cols, "");
        pipkey = vkey();
        if (pipkey == 'Y' || pipkey == 'y')
        {
            /*改名記錄*/
            now = time(0);
            if (d.sex == 1)
                sprintf(buf, "\x1b[1;37m%s %-11s把小雞 [%s] 由♂變性成♀了\x1b[0m\n", Cdate(&now), cuser.userid, d.name);
            else
                sprintf(buf, "\x1b[1;37m%s %-11s把小雞 [%s] 由♀變性成♂了\x1b[0m\n", Cdate(&now), cuser.userid, d.name);
            pip_log_record(buf);
            vmsg("變性手術完畢...");
            d.sex = oldchoice;
        }
        break;

    case '3':
        move(b_lines - 1, 0);
        clrtobot();
        /*1:不要且未婚 4:要且未婚 */
        oldchoice = d.wantend;
        if (d.wantend == 1 || d.wantend == 2 || d.wantend == 3)
        {
            oldchoice += 3; /*沒有-->有*/
            move(b_lines - 1, 0);
            outs("\x1b[1;33m將小雞遊戲改成\x1b[32m[有20歲結局]\x1b[33m? \x1b[37m[y/N]\x1b[0m");
            sprintf(buf, "小雞遊戲設定成[有20歲結局]..");
        }
        else
        {
            oldchoice -= 3; /*有-->沒有*/
            move(b_lines - 1, 0);
            outs("\x1b[1;33m將小雞遊戲改成\x1b[32m[沒有20歲結局]\x1b[33m? \x1b[37m[y/N]\x1b[0m");
            sprintf(buf, "小雞遊戲設定成[沒有20歲結局]..");
        }
        move(b_lines, 0);
        prints("\x1b[1;44m  服務項目  \x1b[46m[1]命名大師 [2]變性手術 [3]結局設局                               %*s\x1b[0m", d_cols, "");
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

#if 0  // Unused
#include<stdarg.h>
static void
pip_data_list_va(va_list pvar)
{
    const char *userid;

    userid = va_arg(pvar, const char *);
    pip_data_list(cuser.userid);
}
#endif  // #if 0  // Unused

static int
pip_data_list_cuser(void)
{
    return pip_data_list(cuser.userid);
}

static int
pip_data_list(  /*看小雞個人詳細資料*/
const char *userid)
{
    char buf[256];
    char inbuf1[20];
    char inbuf2[20];
    int tm;
    int pipkey;
    int page = 1;
    struct chicken chicken;
    FILE *fs;

    /* if (!isprint(userid[0]))*/
        usr_fpath(buf, cuser.userid, "chicken");
    /* else
        usr_fpath(buf, userid, "chicken");*/

    if ( ( fs = fopen(buf, "r") ) )
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
               &(chicken.toman), &(chicken.character), &(chicken.love), &(chicken.wisdom), &(chicken.art), &(chicken.ethics), &(chicken.brave), &(chicken.homework), &(chicken.charm), &(chicken.manners), &(chicken.speech), &(chicken.cookskill), &(chicken.learnA), &(chicken.learnB), &(chicken.learnC), &(chicken.learnD), &(chicken.learnE),
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
        vmsg("我沒有養小雞啦 !");
        return 0;
    }


//  tm=(time(0)-start_time+chicken.bbtime)/60/30;
    tm = chicken.bbtime / 60 / 30;

    clear();
    move(1, 0);
    outs_centered("       \x1b[1;33m╔═══╮╭═══╮╔═══╗╭═══╮\x1b[m\n");
    outs_centered("       \x1b[0;37m║╭╮  ║║ ═   ║╚╗╔═╝║ ═   ║\x1b[m\n");
    outs_centered("       \x1b[1;37m║╰╯  ║║╔╗  ║  ║║    ║╔╗  ║\x1b[m\n");
    outs_centered("       \x1b[1;34m╚═══╯╚╝╚═╝  ╚╝    ╚╝╚═╝\x1b[32m......................\x1b[m");
    do
    {
        clrchyiuan(5, b_lines);
        switch (page)
        {
        case 1:
            move(5, 0);
            sprintf(buf,
                    "\x1b[1;31m ╭┤\x1b[41;37m 基本資料 \x1b[0;1;31m├─────────────────────────────╮\x1b[m\n");
            outs_centered(buf);

            sprintf(buf,
                    "\x1b[1;31m │\x1b[33m﹟姓    名 :\x1b[37m %-10s \x1b[33m﹟生    日 :\x1b[37m %02d/%02d/%02d   \x1b[33m﹟年    紀 :\x1b[37m %-2d         \x1b[31m│\x1b[m\n",
                    chicken.name, (chicken.year) % 100, chicken.month, chicken.day, tm);
            outs_centered(buf);

            sprintf(inbuf1, "%d%s/%d%s", chicken.hp > 1000 ? chicken.hp / 1000 : chicken.hp, chicken.hp > 1000 ? "K" : "", chicken.maxhp > 1000 ? chicken.maxhp / 1000 : chicken.maxhp, chicken.maxhp > 1000 ? "K" : "");
            sprintf(inbuf2, "%d%s/%d%s", chicken.mp > 1000 ? chicken.mp / 1000 : chicken.mp, chicken.mp > 1000 ? "K" : "", chicken.maxmp > 1000 ? chicken.maxmp / 1000 : chicken.maxmp, chicken.maxmp > 1000 ? "K" : "");

            sprintf(buf,
                    "\x1b[1;31m │\x1b[33m﹟體    重 :\x1b[37m %-5d(米克)\x1b[33m﹟體    力 :\x1b[37m %-11s\x1b[33m﹟法    力 :\x1b[37m %-11s\x1b[31m│\x1b[m\n",
                    chicken.weight, inbuf1, inbuf2);
            outs_centered(buf);

            sprintf(buf,
                    "\x1b[1;31m │\x1b[33m﹟疲    勞 :\x1b[37m %-3d        \x1b[33m﹟病    氣 :\x1b[37m %-3d        \x1b[33m﹟髒    髒 :\x1b[37m %-3d        \x1b[31m│\x1b[m\n",
                    chicken.tired, chicken.sick, chicken.shit);
            outs_centered(buf);

            sprintf(buf,
                    "\x1b[1;31m │\x1b[33m﹟腕    力 :\x1b[37m %-7d    \x1b[33m﹟親子關係 :\x1b[37m %-7d    \x1b[33m﹟金    錢 :\x1b[37m %-11d\x1b[31m│\x1b[m\n",
                    chicken.wrist, chicken.relation, chicken.money);
            outs_centered(buf);

            sprintf(buf,
                    "\x1b[1;31m ├┤\x1b[41;37m 能力資料 \x1b[0;1;31m├─────────────────────────────┤\x1b[m\n");
            outs_centered(buf);

            sprintf(buf,
                    "\x1b[1;31m │\x1b[33m﹟氣    質 :\x1b[37m %-10d \x1b[33m﹟智    力 :\x1b[37m %-10d \x1b[33m﹟愛    心 :\x1b[37m %-10d \x1b[31m│\x1b[m\n",
                    chicken.character, chicken.wisdom, chicken.love);
            outs_centered(buf);

            sprintf(buf,
                    "\x1b[1;31m │\x1b[33m﹟藝    術 :\x1b[37m %-10d \x1b[33m﹟道    德 :\x1b[37m %-10d \x1b[33m﹟家    事 :\x1b[37m %-10d \x1b[31m│\x1b[m\n",
                    chicken.art, chicken.ethics, chicken.homework);
            outs_centered(buf);

            sprintf(buf,
                    "\x1b[1;31m │\x1b[33m﹟禮    儀 :\x1b[37m %-10d \x1b[33m﹟應    對 :\x1b[37m %-10d \x1b[33m﹟烹    飪 :\x1b[37m %-10d \x1b[31m│\x1b[m\n",
                    chicken.manners, chicken.speech, chicken.cookskill);
            outs_centered(buf);

            sprintf(buf,
                    "\x1b[1;31m ├┤\x1b[41;37m 狀態資料 \x1b[0;1;31m├─────────────────────────────┤\x1b[m\n");
            outs_centered(buf);

            sprintf(buf,
                    "\x1b[1;31m │\x1b[33m﹟快    樂 :\x1b[37m %-10d \x1b[33m﹟滿    意 :\x1b[37m %-10d \x1b[33m﹟人    際 :\x1b[37m %-10d \x1b[31m│\x1b[m\n",
                    chicken.happy, chicken.satisfy, chicken.toman);
            outs_centered(buf);

            sprintf(buf,
                    "\x1b[1;31m │\x1b[33m﹟魅    力 :\x1b[37m %-10d \x1b[33m﹟勇    敢 :\x1b[37m %-10d \x1b[33m﹟信    仰 :\x1b[37m %-10d \x1b[31m│\x1b[m\n",
                    chicken.charm, chicken.brave, chicken.belief);
            outs_centered(buf);

            sprintf(buf,
                    "\x1b[1;31m │\x1b[33m﹟罪    孽 :\x1b[37m %-10d \x1b[33m﹟感    受 :\x1b[37m %-10d \x1b[33m            \x1b[37m            \x1b[31m│\x1b[m\n",
                    chicken.offense, chicken.affect);
            outs_centered(buf);

            sprintf(buf,
                    "\x1b[1;31m ├┤\x1b[41;37m 評價資料 \x1b[0;1;31m├─────────────────────────────┤\x1b[m\n");
            outs_centered(buf);

            sprintf(buf,
                    "\x1b[1;31m │\x1b[33m﹟社交評價 :\x1b[37m %-10d \x1b[33m﹟戰鬥評價 :\x1b[37m %-10d \x1b[33m﹟魔法評價 :\x1b[37m %-10d \x1b[31m│\x1b[m\n",
                    chicken.social, chicken.hexp, chicken.mexp);
            outs_centered(buf);

            sprintf(buf,
                    "\x1b[1;31m │\x1b[33m﹟家事評價 :\x1b[37m %-10d                                                 \x1b[31m│\x1b[m\n",
                    chicken.family);
            outs_centered(buf);

            sprintf(buf,
                    "\x1b[1;31m ╰────────────────────────────────────╯\x1b[m\n");
            outs_centered(buf);

            move(b_lines - 1, 0);
            sprintf(buf,
                    "                                                              \x1b[1;36m第一頁\x1b[37m/\x1b[36m共二頁\x1b[m\n");
            outs_centered(buf);
            break;

        case 2:
            move(5, 0);
            sprintf(buf,
                    "\x1b[1;31m ╭┤\x1b[41;37m 物品資料 \x1b[0;1;31m├─────────────────────────────╮\x1b[m\n");
            outs_centered(buf);

            sprintf(buf,
                    "\x1b[1;31m │\x1b[33m﹟食    物 :\x1b[37m %-10d \x1b[33m﹟零    食 :\x1b[37m %-10d \x1b[33m﹟大 補 丸 :\x1b[37m %-10d \x1b[31m│\x1b[m\n",
                    chicken.food, chicken.cookie, chicken.bighp);
            outs_centered(buf);

            sprintf(buf,
                    "\x1b[1;31m │\x1b[33m﹟靈    芝 :\x1b[37m %-10d \x1b[33m﹟書    本 :\x1b[37m %-10d \x1b[33m﹟玩    具 :\x1b[37m %-10d \x1b[31m│\x1b[m\n",
                    chicken.medicine, chicken.book, chicken.playtool);
            outs_centered(buf);

            sprintf(buf,
                    "\x1b[1;31m ├┤\x1b[41;37m 遊戲資料 \x1b[0;1;31m├─────────────────────────────┤\x1b[m\n");
            outs_centered(buf);

            sprintf(buf,
                    "\x1b[1;31m │\x1b[33m﹟猜 拳 贏 :\x1b[37m %-10d \x1b[33m﹟猜 拳 輸 :\x1b[37m %-10d                         \x1b[31m│\x1b[m\n",
                    chicken.winn, chicken.losee);
            outs_centered(buf);

            sprintf(buf,
                    "\x1b[1;31m ├┤\x1b[41;37m 武力資料 \x1b[0;1;31m├─────────────────────────────┤\x1b[m\n");
            outs_centered(buf);

            sprintf(buf,
                    "\x1b[1;31m │\x1b[33m﹟攻 擊 力 :\x1b[37m %-10d \x1b[33m﹟防 禦 力 :\x1b[37m %-10d \x1b[33m﹟速 度 值 :\x1b[37m %-10d \x1b[31m│\x1b[m\n",
                    chicken.attack, chicken.resist, chicken.speed);
            outs_centered(buf);
            sprintf(buf,
                    "\x1b[1;31m │\x1b[33m﹟抗魔能力 :\x1b[37m %-10d \x1b[33m﹟戰鬥技術 :\x1b[37m %-10d \x1b[33m﹟魔法技術 :\x1b[37m %-10d \x1b[31m│\x1b[m\n",
                    chicken.mresist, chicken.hskill, chicken.mskill);
            outs_centered(buf);

            sprintf(buf,
                    "\x1b[1;31m │\x1b[33m﹟頭部裝備 :\x1b[37m %-10s \x1b[33m﹟右手裝備 :\x1b[37m %-10s \x1b[33m﹟左手裝備 :\x1b[37m %-10s \x1b[31m│\x1b[m\n",
                    weaponhead[chicken.weaponhead], weaponrhand[chicken.weaponrhand], weaponlhand[chicken.weaponlhand]);
            outs_centered(buf);

            sprintf(buf,
                    "\x1b[1;31m │\x1b[33m﹟身體裝備 :\x1b[37m %-10s \x1b[33m﹟腳部裝備 :\x1b[37m %-10s \x1b[33m            \x1b[37m            \x1b[31m│\x1b[m\n",
                    weaponbody[chicken.weaponbody], weaponfoot[chicken.weaponfoot]);
            outs_centered(buf);

            sprintf(buf,
                    "\x1b[1;31m ├┤\x1b[41;37m 等級資料 \x1b[0;1;31m├─────────────────────────────┤\x1b[m\n");
            outs_centered(buf);

            sprintf(buf,
                    "\x1b[1;31m │\x1b[33m﹟等    級 :\x1b[37m %-10d \x1b[33m﹟經 驗 值 :\x1b[37m %-10d \x1b[33m﹟下次升級 :\x1b[37m %-10d \x1b[31m│\x1b[m\n",
                    chicken.level, chicken.exp, twice(d.level, 10000, 100));
            outs_centered(buf);

            sprintf(buf,
                    "\x1b[1;31m ╰────────────────────────────────────╯\x1b[m\n");
            outs_centered(buf);

            move(b_lines - 1, 0);
            sprintf(buf,
                    "                                                              \x1b[1;36m第二頁\x1b[37m/\x1b[36m共二頁\x1b[m\n");
            outs_centered(buf);
            break;
        }
        move(b_lines, 0);
        sprintf(buf, "\x1b[1;44;37m  資料選單  \x1b[46m  [↑/PAGE UP]往上一頁 [↓/PAGE DOWN]往下一頁 [Q]離開            %*s\x1b[m", d_cols, "");
        outs(buf);
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
/* 戰鬥特區                                                                  */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static int pip_fight_main(int n, const struct playrule list[], int mode);

/*---------------------------------------------------------------------------*/
/* 戰鬥人物決定函式                                                          */
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
static int
get_man(
int class, int mob, int plus)
{
    int lucky, man;
    lucky = random() % (class * 5);
    if (lucky <= (class*2))
    {
        man = random() % mob + plus;
    }
    else if (lucky <= (class*4) && lucky > (class*2))
    {
        man = random() % (mob + plus / 2) + (plus / 2);
    }
    else
    {
        man = random() % (mob + plus);
    }
    return man;
}

static int
pip_meet_vs_man(void)
{
    int class;
    int man, lucky;
    char ans;
    class = (d.maxhp * 30 + d.maxmp * 20 + d.attack * 20 + d.resist * 15 + d.mexp * 5 + d.hexp * 5 + d.speed * 10) / 8500 + 1;

    move(b_lines - 1, 0);
    prints("\x1b[1;44;37m 區域 \x1b[46m[1]炎之洞窟  [2]北方冰原  [3]古代遺跡  [4]人工島  [5]地獄之門           %*s\x1b[m\n", d_cols, "");
    prints("\x1b[1;44;37m 選單 \x1b[46m                                                                %*s[Q]回家 \x1b[m", d_cols, "");
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
        move(b_lines - 1, 0);
        clrtoeol();
        move(b_lines, 0);
        prints("\x1b[1;44;37m 方向 \x1b[46m[R]回家 [F]餵食 (E/W/S/N)東西南北                                       %*s\x1b[m", d_cols, "");
        ans = vkey();
        if (ans == 'r' || ans == 'R')
            return 0;

        lucky = random() % 2000;
        if (ans != 'e' && ans != 'w' && ans != 's' && ans != 'n' && ans != 'E' && ans != 'W' && ans != 'S' && ans != 'N' &&
            ans != 'F' && ans != 'f')
            continue;
        if (ans == 'f' || ans == 'F')
            pip_basic_feed();
        else if (lucky == 1999)
        {
            vmsg("遇到大魔王啦！");
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
            vmsg("沒發生任何事！");
    }
    return 0;
}

static void
pip_fight_bad(
int n)
{
    pip_fight_main(n, badmanlist, 1);
    return;
}


static int
pip_fight_main(
int n,
const struct playrule list[],
int mode)
{
    pip_vs_man(n, list, mode);
    return 0;
}

/*---------------------------------------------------------------------------*/
/* 戰鬥戰鬥函式                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/
static int
pip_vs_man(
int n,
const struct playrule *p,
int mode)
{
    /* p[n].name hp mp speed attack resist money special map */
    struct playrule m;
    char buf[256];
    char inbuf1[20];
    char inbuf2[20];
    int pipkey;
    int mankey;
    int lucky;
    int dinjure = 0;            /*小雞傷害力*/
    int minjure = 0;            /*對方傷害力*/
    int dresistmode = 0;        /*小雞加強防禦*/
    int mresistmode = 0;        /*對方加強防禦*/
    int oldhexp;                /*未戰鬥前格鬥經驗*/
    int oldmexp;                /*未戰鬥前魔法經驗*/
    int oldbrave;               /*未戰鬥前勇敢*/
    int oldhskill;              /*未戰鬥前戰鬥技術*/
    int oldmskill;              /*未戰鬥前魔法技術*/
    int oldethics;      /*未戰鬥前道德*/
    int oldmoney;               /*未戰鬥前金錢*/
    int oldtired;
    int oldhp;
    int oldexp;
    int winorlose = 0;          /*1:you win 0:you loss*/

    /*隨機產生人物 並且存好戰鬥前的一些數值*/
    oldhexp = d.hexp;
    oldmexp = d.mexp;
    oldbrave = d.brave;
    oldhskill = d.hskill;
    oldmskill = d.mskill;
    oldethics = d.ethics;
    oldmoney = d.money;
    if (mode == 1)
    {
        m.hp = p[n].hp - random() % 10;
        m.maxhp = (m.hp + p[n].hp) / 2;
        m.mp = p[n].mp - random() % 10;
        m.maxmp = (m.mp + p[n].mp) / 2;
        m.speed = p[n].speed - random() % 4 - 1;
        m.attack = p[n].attack - random() % 10;
        m.resist = p[n].resist - random() % 10;
        m.money = p[n].money - random() % 50;
        m.death = p[n].death;
        m.map = p[n].map;
    }
    else
    {
        m.maxhp = d.maxhp * (80 + random() % 50) / 100 + 20;;
        m.hp = m.maxhp - random() % 10 + 20;
        m.maxmp = d.maxmp * (80 + random() % 50) / 100 + 10;
        m.mp = m.maxmp - random() % 20 + 10;
        m.speed = d.speed * (80 + random() % 50) / 100 + 10;
        m.attack = d.attack * (80 + random() % 50) / 100 + 10;
        m.resist = d.resist * (80 + random() % 50) / 100 + 10;
        m.money = 0;
        m.death = 0;
    }
    /*d.tired+=random()%(n+1)/4+2;*/
    /*d.shit+=random()%(n+1)/4+2;*/
    do
    {
        if (m.hp <= 0) /*敵人死掉了*/
        {
            m.hp = 0;
            d.money += m.money;
            m.death = 1;
            d.brave += random() % 4 + 3;
        }
        if (d.hp <= 0 || d.tired >= 100)  /*小雞陣亡*/
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
                d.hexp -= random() % 3 + 2;
                d.mexp -= random() % 3 + 2;
                d.tired = 50;
                d.death = 1;
            }
        }
        clear();
        /*vs_head("電子養小雞", BoardName);*/
        move(0, 0);
        if (d.sex == 1)
            sprintf(buf, "\x1b[1;41m  " NICKNAME PIPNAME " ～ \x1b[32m♂ \x1b[37m%-10s        %*s\x1b[0m", d.name, 50 + d_cols - ((int)(unsigned int)sizeof(NICKNAME PIPNAME) - 1), "");
        else if (d.sex == 2)
            sprintf(buf, "\x1b[1;41m  " NICKNAME PIPNAME " ～ \x1b[33m♀ \x1b[37m%-10s        %*s\x1b[0m", d.name, 50 + d_cols - ((int)(unsigned int)sizeof(NICKNAME PIPNAME) - 1), "");
        else
            sprintf(buf, "\x1b[1;41m  " NICKNAME PIPNAME " ～ \x1b[34m？ \x1b[37m%-10s        %*s\x1b[0m", d.name, 50 + d_cols - ((int)(unsigned int)sizeof(NICKNAME PIPNAME) - 1), "");
        outs(buf);
        move(6, 0);
        if (mode == 1)
            show_badman_pic(m.map/*n*/);
        move(1, 0);
        sprintf(buf, "\x1b[1;31m┌─────────────────────────────────────┐\x1b[m");
        outs_centered(buf);
        move(2, 0);
        /* lucky拿來當color用*/
        if (d.tired >= 80)
            lucky = 31;
        else if (d.tired >= 60 && d.tired < 80)
            lucky = 33;
        else
            lucky = 37;
        sprintf(inbuf1, "%d%s/%d%s", d.hp > 1000 ? d.hp / 1000 : d.hp, d.hp > 1000 ? "K" : "", d.maxhp > 1000 ? d.maxhp / 1000 : d.maxhp, d.maxhp > 1000 ? "K" : "");
        sprintf(inbuf2, "%d%s/%d%s", d.mp > 1000 ? d.mp / 1000 : d.mp, d.mp > 1000 ? "K" : "", d.maxmp > 1000 ? d.maxmp / 1000 : d.maxmp, d.maxmp > 1000 ? "K" : "");

        sprintf(buf, "\x1b[1;31m│\x1b[33m生  命:\x1b[37m%-12s\x1b[33m法  力:\x1b[37m%-12s\x1b[33m疲  勞:\x1b[%dm%-12d\x1b[33m金  錢:\x1b[37m%-10d\x1b[31m│\x1b[m",
                inbuf1, inbuf2, lucky, d.tired, d.money);
        outs_centered(buf);
        move(3, 0);
        sprintf(buf, "\x1b[1;31m│\x1b[33m攻  擊:\x1b[37m%-10d  \x1b[33m防  禦:\x1b[37m%-10d  \x1b[33m速  度:\x1b[37m%-10d  \x1b[33m經  驗:\x1b[37m%-10d\x1b[31m│\x1b[m",
                d.attack, d.resist, d.speed, d.exp);
        outs_centered(buf);
        move(4, 0);
        sprintf(buf, "\x1b[1;31m│\x1b[33m食  物:\x1b[37m%-5d       \x1b[33m大補丸:\x1b[37m%-5d       \x1b[33m零  食:\x1b[37m%-5d       \x1b[33m靈  芝:\x1b[37m%-5d     \x1b[31m│\x1b[m",
                d.food, d.bighp, d.cookie, d.medicine);
        outs_centered(buf);
        move(5, 0);
        sprintf(buf, "\x1b[1;31m└─────────────────────────────────────┘\x1b[m");
        outs_centered(buf);
        move(b_lines - 4, 0);
        sprintf(buf, "\x1b[1;34m┌─────────────────────────────────────┐\x1b[m");
        outs_centered(buf);
        move(b_lines - 3, 0);
        sprintf(inbuf1, "%d%s/%d%s", m.hp > 1000 ? m.hp / 1000 : m.hp, m.hp > 1000 ? "K" : "", m.maxhp > 1000 ? m.maxhp / 1000 : m.maxhp, m.maxhp > 1000 ? "K" : "");
        sprintf(inbuf2, "%d%s/%d%s", m.mp > 1000 ? m.mp / 1000 : m.mp, m.mp > 1000 ? "K" : "", m.maxmp > 1000 ? m.maxmp / 1000 : m.maxmp, m.maxmp > 1000 ? "K" : "");

        sprintf(buf, "\x1b[1;34m│\x1b[32m姓  名:\x1b[37m%-10s  \x1b[32m生  命:\x1b[37m%-11s \x1b[32m法  力:\x1b[37m%-11s                  \x1b[34m│\x1b[m",
                p[n].name, inbuf1, inbuf2);
        outs_centered(buf);
        move(b_lines - 2, 0);
        sprintf(buf, "\x1b[1;34m│\x1b[32m攻  擊:\x1b[37m%-6d      \x1b[32m防  禦:\x1b[37m%-6d      \x1b[32m速  度:\x1b[37m%-6d      \x1b[32m金  錢:\x1b[37m%-6d    \x1b[34m│\x1b[m",
                m.attack, m.resist, m.speed, m.money);
        outs_centered(buf);
        move(b_lines - 1, 0);
        sprintf(buf, "\x1b[1;34m└─────────────────────────────────────┘\x1b[m");
        outs_centered(buf);
        move(b_lines, 0);
        sprintf(buf, "\x1b[1;44;37m  戰鬥命令  \x1b[46m  [1]普通  [2]全力  [3]魔法  [4]防禦  [5]補充  [6]逃命            %*s\x1b[m", d_cols, "");
        outs(buf);

        if (m.death == 0 && d.death == 0)
        {
            dresistmode = 0;
            d.nodone = 0;
            pipkey = vkey();
            switch (pipkey)
            {
            case '1':
                if (random() % 9 == 0)
                {
                    vmsg("竟然沒打中..:~~~");
                }
                else
                {
                    if (mresistmode == 0)
                        dinjure = (d.hskill / 100 + d.hexp / 100 + d.attack / 9 - m.resist / 8 + random() % 12 + 2 - m.speed / 30 + d.speed / 30);
                    else
                        dinjure = (d.hskill / 100 + d.hexp / 100 + d.attack / 9 - m.resist / 6 + random() % 12 + 2 - m.speed / 30 + d.speed / 30);
                    if (dinjure <= 0)
                        dinjure = 9;
                    m.hp -= dinjure;
                    d.hexp += random() % 2 + 2;
                    d.hskill += random() % 2 + 1;
                    sprintf(buf, "普通攻擊，對方生命力減低%d", dinjure);
                    vmsg(buf);
                }
                d.tired += random() % (n + 1) / 15 + 2;
                break;

            case '2':
                show_fight_pic(2);
                if (random() % 11 == 0)
                {
                    vmsg("竟然沒打中..:~~~");
                }
                else
                {
                    if (mresistmode == 0)
                        dinjure = (d.hskill / 100 + d.hexp / 100 + d.attack / 5 - m.resist / 12 + random() % 12 + 6 - m.speed / 50 + d.speed / 30);
                    else
                        dinjure = (d.hskill / 100 + d.hexp / 100 + d.attack / 5 - m.resist / 8 + random() % 12 + 6 - m.speed / 40 + d.speed / 30);
                    if (dinjure <= 15)
                        dinjure = 20;
                    if (d.hp > 5)
                    {
                        m.hp -= dinjure;
                        d.hp -= 5;
                        d.hexp += random() % 3 + 3;
                        d.hskill += random() % 2 + 2;
                        d.tired += random() % (n + 1) / 10 + 3;
                        sprintf(buf, "全力攻擊，對方生命力減低%d", dinjure);
                        vmsg(buf);
                    }
                    else
                    {
                        d.nodone = 1;
                        vmsg("你的HP小於5啦..不行啦...");
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
                        sprintf(buf, "治療後，生命力提高%d，疲勞降低%d", oldhp, oldtired);
                        vmsg(buf);
                    }
                    else
                    {
                        if (random() % 15 == 0)
                            vmsg("竟然沒打中..:~~~");
                        else
                        {
                            if (d.mexp <= 100)
                            {
                                if (random() % 4 > 0)
                                    dinjure = dinjure * 60 / 100;
                                else
                                    dinjure = dinjure * 80 / 100;
                            }
                            else if (d.mexp <= 250 && d.mexp > 100)
                            {
                                if (random() % 4 > 0)
                                    dinjure = dinjure * 70 / 100;
                                else
                                    dinjure = dinjure * 85 / 100;
                            }
                            else if (d.mexp <= 500 && d.mexp > 250)
                            {
                                if (random() % 4 > 0)
                                    dinjure = dinjure * 85 / 100;
                                else
                                    dinjure = dinjure * 95 / 100;
                            }
                            else if (d.mexp > 500)
                            {
                                if (random() % 10 > 0)
                                    dinjure = dinjure * 90 / 100;
                                else
                                    dinjure = dinjure * 99 / 100;
                            }
                            if ((p[n].special[d.magicmode-2] - 48) == 1)
                            {
                                if (random() % 2 > 0)
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
                                if (random() % 2 > 0)
                                {
                                    dinjure = dinjure * 60 / 100;
                                }
                                else
                                {
                                    dinjure = dinjure * 75 / 100;
                                }
                            }
                            d.tired += random() % (n + 1) / 12 + 2;
                            m.hp -= dinjure;
                            /*d.mexp+=random()%2+2;*/
                            d.mskill += random() % 2 + 2;
                            sprintf(buf, "魔法攻擊，對方生命力減低%d", dinjure);
                            vmsg(buf);
                        }
                    }
                }
                break;
            case '4':
                dresistmode = 1;
                d.tired += random() % (n + 1) / 20 + 1;
                vmsg("小雞加強防禦啦....");
                break;

            case '5':

                pip_basic_feed();
                break;

            case '6':
                d.money -= (random() % 100 + 30);
                d.brave -= (random() % 3 + 2);
                if (d.money < 0)
                    d.money = 0;
                if (d.hskill < 0)
                    d.hskill = 0;
                if (d.brave < 0)
                    d.brave = 0;
                clear();
                vs_head("電子養小雞", BoardName);
                move(10, 0);
                outs_centered("            \x1b[1;31m┌──────────────────────┐\x1b[m\n");
                prints_centered("            \x1b[1;31m│  \x1b[37m實力不強的小雞 \x1b[33m%-10s                 \x1b[31m│\x1b[m\n", d.name);
                prints_centered("            \x1b[1;31m│  \x1b[37m在與對手 \x1b[32m%-10s \x1b[37m戰鬥後落跑啦          \x1b[31m│\x1b[m\n", p[n].name);
                sprintf(inbuf1, "%d/%d", d.hexp - oldhexp, d.mexp - oldmexp);
                prints_centered("            \x1b[1;31m│  \x1b[37m評價增加了 \x1b[36m%-5s \x1b[37m點  技術增加了 \x1b[36m%-2d/%-2d \x1b[37m點  \x1b[31m│\x1b[m\n", inbuf1, d.hskill - oldhskill, d.mskill - oldmskill);
                sprintf(inbuf1, "%d \x1b[37m元", oldmoney - d.money);
                prints_centered("            \x1b[1;31m│  \x1b[37m勇敢降低了 \x1b[36m%-5d \x1b[37m點  金錢減少了 \x1b[36m%-13s  \x1b[31m│\x1b[m\n", oldbrave - d.brave, inbuf1);
                outs_centered("            \x1b[1;31m└──────────────────────┘\x1b[m");
                vmsg("三十六計 走為上策...");
                winorlose = 0;
                break;
            }
        }
        clear();
        move(0, 0);
        if (d.sex == 1)
            sprintf(buf, "\x1b[1;41m  " NICKNAME PIPNAME " ～ \x1b[32m♂ \x1b[37m%-10s        %*s\x1b[0m", d.name, 50 + d_cols - ((int)(unsigned int)sizeof(NICKNAME PIPNAME) - 1), "");
        else if (d.sex == 2)
            sprintf(buf, "\x1b[1;41m  " NICKNAME PIPNAME " ～ \x1b[33m♀ \x1b[37m%-10s        %*s\x1b[0m", d.name, 50 + d_cols - ((int)(unsigned int)sizeof(NICKNAME PIPNAME) - 1), "");
        else
            sprintf(buf, "\x1b[1;41m  " NICKNAME PIPNAME " ～ \x1b[34m？ \x1b[37m%-10s        %*s\x1b[0m", d.name, 50 + d_cols - ((int)(unsigned int)sizeof(NICKNAME PIPNAME) - 1), "");
        outs(buf);
        move(1, 0);
        sprintf(buf, "\x1b[1;31m┌─────────────────────────────────────┐\x1b[m");
        outs_centered(buf);
        move(2, 0);
        /* lucky拿來當color用*/
        if (d.tired >= 80)
            lucky = 31;
        else if (d.tired >= 60 && d.tired < 80)
            lucky = 33;
        else
            lucky = 37;

        sprintf(inbuf1, "%d%s/%d%s", d.hp > 1000 ? d.hp / 1000 : d.hp, d.hp > 1000 ? "K" : "", d.maxhp > 1000 ? d.maxhp / 1000 : d.maxhp, d.maxhp > 1000 ? "K" : "");
        sprintf(inbuf2, "%d%s/%d%s", d.mp > 1000 ? d.mp / 1000 : d.mp, d.mp > 1000 ? "K" : "", d.maxmp > 1000 ? d.maxmp / 1000 : d.maxmp, d.maxmp > 1000 ? "K" : "");

        sprintf(buf, "\x1b[1;31m│\x1b[33m生  命:\x1b[37m%-12s\x1b[33m法  力:\x1b[37m%-12s\x1b[33m疲  勞:\x1b[%dm%-12d\x1b[33m金  錢:\x1b[37m%-10d\x1b[31m│\x1b[m",
                inbuf1, inbuf2, lucky, d.tired, d.money);
        outs_centered(buf);

        move(3, 0);
        sprintf(buf, "\x1b[1;31m│\x1b[33m攻  擊:\x1b[37m%-10d  \x1b[33m防  禦:\x1b[37m%-10d  \x1b[33m速  度:\x1b[37m%-10d  \x1b[33m經  驗:\x1b[37m%-10d\x1b[31m│\x1b[m",
                d.attack, d.resist, d.speed, d.exp);
        outs_centered(buf);
        move(4, 0);
        sprintf(buf, "\x1b[1;31m│\x1b[33m食  物:\x1b[37m%-5d       \x1b[33m大補丸:\x1b[37m%-5d       \x1b[33m零  食:\x1b[37m%-5d       \x1b[33m靈  芝:\x1b[37m%-5d     \x1b[31m│\x1b[m",
                d.food, d.bighp, d.cookie, d.medicine);
        outs_centered(buf);
        move(5, 0);
        sprintf(buf, "\x1b[1;31m└─────────────────────────────────────┘\x1b[m");
        outs_centered(buf);
        move(6, 0);
        if (mode == 1)
            show_badman_pic(m.map/*n*/);
        move(b_lines - 4, 0);
        sprintf(buf, "\x1b[1;34m┌─────────────────────────────────────┐\x1b[m");
        outs_centered(buf);
        move(b_lines - 3, 0);
        sprintf(inbuf1, "%d/%d", m.hp, m.maxhp);
        sprintf(inbuf2, "%d/%d", m.mp, m.maxmp);
        sprintf(buf, "\x1b[1;34m│\x1b[32m姓  名:\x1b[37m%-10s  \x1b[32m生  命:\x1b[37m%-11s \x1b[32m法  力:\x1b[37m%-11s                  \x1b[34m│\x1b[m",
                p[n].name, inbuf1, inbuf2);
        outs_centered(buf);
        move(b_lines - 2, 0);
        sprintf(buf, "\x1b[1;34m│\x1b[32m攻  擊:\x1b[37m%-6d      \x1b[32m防  禦:\x1b[37m%-6d      \x1b[32m速  度:\x1b[37m%-6d      \x1b[32m金  錢:\x1b[37m%-6d    \x1b[34m│\x1b[m",
                m.attack, m.resist, m.speed, m.money);
        outs_centered(buf);
        move(b_lines - 1, 0);
        sprintf(buf, "\x1b[1;34m└─────────────────────────────────────┘\x1b[m");
        outs_centered(buf);
        move(b_lines, 0);
        sprintf(buf, "\x1b[1;41;37m  \x1b[37m攻擊命令  \x1b[47m  \x1b[31m[1]\x1b[30m普通  \x1b[31m[2]\x1b[30m全力  \x1b[31m[3]\x1b[30m魔法  \x1b[31m[4]\x1b[30m防禦  \x1b[31m[5]\x1b[30m逃命                     %*s\x1b[m", d_cols, "");
        outs(buf);

        if ((m.hp > 0) && (pipkey != '6') && (pipkey == '1' || pipkey == '2' || pipkey == '3' || pipkey == '4' || pipkey == '5') && (d.death == 0) && (d.nodone == 0))
        {
            mresistmode = 0;
            lucky = random() % 100;
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
                if (random() % 6 == 5)
                {
                    vmsg("對方沒打中..:~~~");
                }
                else
                {
                    if (dresistmode == 0)
                        minjure = (m.attack / 9 - d.resist / 12 + random() % 15 + 4 - d.speed / 30 + m.speed / 30 - d.hskill / 200 - d.hexp / 200);
                    else
                        minjure = (m.attack / 9 - d.resist / 8 + random() % 12 + 4 - d.speed / 50 + m.speed / 20 - d.hskill / 200 - d.hexp / 200);
                    if (minjure <= 0)
                        minjure = 8;
                    d.hp -= minjure;
                    d.tired += random() % 3 + 2;
                    sprintf(buf, "對方普通攻擊，生命力減低%d", minjure);
                    vmsg(buf);
                }
                break;

            case 2:
                if (random() % 11 == 10)
                {
                    vmsg("對方沒打中..:~~~");
                }
                else
                {
                    if (m.hp > 5)
                    {
                        if (dresistmode == 0)
                            minjure = (m.attack / 5 - d.resist / 12 + random() % 12 + 6 - d.speed / 30 + m.speed / 30 - d.hskill / 200 - d.hexp / 200);
                        else
                            minjure = (m.attack / 5 - d.resist / 8 + random() % 12 + 6 - d.speed / 30 + m.speed / 30 - d.hskill / 200 - d.hexp / 200);
                        if (minjure <= 15)
                            minjure = 20;
                        d.hp -= minjure;
                        m.hp -= 5;
                        sprintf(buf, "對方全力攻擊，生命力減低%d", minjure);
                        d.tired += random() % 4 + 4;
                        vmsg(buf);
                    }
                    else
                    {
                        if (dresistmode == 0)
                            minjure = (m.attack / 9 - d.resist / 12 + random() % 12 + 4 - d.speed / 30 + m.speed / 25 - d.hexp / 200 - d.hskill / 200);
                        else
                            minjure = (m.attack / 9 - d.resist / 8 + random() % 12 + 3 - d.speed / 30 + m.speed / 25 - d.hexp / 200 - d.hskill / 200);
                        if (minjure <= 0)
                            minjure = 4;
                        d.hp -= minjure;
                        d.tired += random() % 3 + 2;
                        sprintf(buf, "對方普通攻擊，生命力減低%d", minjure);
                        vmsg(buf);
                    }
                }
                break;

            case 3:
                if (random() % 5 > 3 && m.mp > 20)
                {
                    if (random() % 6 > 0 && m.mp >= 50 && m.hp > (m.maxhp / 10))
                    {
                        if (m.mp >= (m.maxmp / 2))
                        {
                            minjure = m.maxmp / 4;
                            m.mp -= (500 - random() % 300);
                            if (random() % 2)
                                sprintf(inbuf1, "熱火魔");
                            else
                                sprintf(inbuf1, "寒氣鬼");
                        }
                        else if (m.mp < (m.maxmp / 2) && m.mp >= (m.maxmp / 4))
                        {
                            minjure = m.maxmp / 5;
                            m.mp -= (300 + random() % 200);
                            if (random() % 2)
                                sprintf(inbuf1, "狂水怪");
                            else
                                sprintf(inbuf1, "怒土虫");
                        }
                        else if (m.mp < (m.maxmp / 4) && m.mp >= (m.maxmp / 6))
                        {
                            minjure = m.maxmp / 6;
                            m.mp -= (100 + random() % 100);
                            if (random() % 2)
                                sprintf(inbuf1, "迷魂鬼差");
                            else
                                sprintf(inbuf1, "石怪");
                        }
                        else if (m.mp < (m.maxmp / 6) && m.mp >= 0)
                        {
                            minjure = m.maxmp / 8;
                            m.mp -= 50;
                            if (random() % 2)
                                sprintf(inbuf1, "鬼木魂");
                            else
                                sprintf(inbuf1, "風妖");
                        }
                        minjure = minjure - d.resist / 50 - d.mresist / 10 - d.mskill / 200 - d.mexp / 200 + random() % 10;
                        if (minjure < 0)
                            minjure = 15;
                        d.hp -= minjure;
                        if (m.mp < 0) m.mp = 0;
                        d.mresist += random() % 2 + 1;
                        sprintf(buf, "對方召喚了%s，你受傷了%d點", inbuf1, minjure);
                        vmsg(buf);
                    }
                    else
                    {
                        m.mp -= 20;
                        m.hp += (m.maxmp / 6) + random() % 20;
                        if (m.hp > m.maxhp)
                            m.hp = m.maxhp;
                        vmsg("對方使用魔法治療了自己...");
                    }
                }
                else
                {
                    mresistmode = 1;
                    vmsg("對方加強防禦....");
                }
                break;

            case 4:
                d.money += (m.money + m.money / 2) / 3 + random() % 10;
                d.hskill += random() % 4 + 3;
                d.brave += random() % 3 + 2;
                m.death = 1;
                sprintf(buf, "對方先閃了..但掉了一些錢給你...");
                vmsg(buf);
                break;
            }
        }

        if (m.death == 1)
        {
            clear();
            oldexp = ((d.hexp - oldhexp) + (d.mexp - oldmexp) + random() % 10) * (d.level + 1) + random() % (d.level + 1);
            d.exp += oldexp;
            vs_head("電子養小雞", BoardName);
            if (mode == 1)
            {
                move(10, 0);
                outs_centered("            \x1b[1;31m┌──────────────────────┐\x1b[m\n");
                prints_centered("            \x1b[1;31m│  \x1b[37m英勇的小雞 \x1b[33m%-10s                     \x1b[31m│\x1b[m\n", d.name);
                prints_centered("            \x1b[1;31m│  \x1b[37m打敗了邪惡的怪物 \x1b[32m%-10s               \x1b[31m│\x1b[m\n", p[n].name);
            }
            else
            {
                move(10, 0);
                outs_centered("            \x1b[1;31m┌──────────────────────┐\x1b[m\n");
                prints_centered("            \x1b[1;31m│  \x1b[37m武術大會的小雞 \x1b[33m%-10s                 \x1b[31m│\x1b[m\n", d.name);
                prints_centered("            \x1b[1;31m│  \x1b[37m打敗了強勁的對手 \x1b[32m%-10s               \x1b[31m│\x1b[m\n", p[n].name);
            }
            sprintf(inbuf1, "%d/%d", d.hexp - oldhexp, d.mexp - oldmexp);
            prints_centered("            \x1b[1;31m│  \x1b[37m評價提升了 %-5s 點  技術增加了 %-2d/%-2d 點  \x1b[31m│\x1b[m\n", inbuf1, d.hskill - oldhskill, d.mskill - oldmskill);
            sprintf(inbuf1, "%d 元", d.money - oldmoney);
            prints_centered("            \x1b[1;31m│  \x1b[37m勇敢提升了 %-5d 點  金錢增加了 %-9s \x1b[31m│\x1b[m\n", d.brave - oldbrave, inbuf1);
            prints_centered("            \x1b[1;31m│  \x1b[37m經驗值增加了 %-6d 點  升級尚需 %-6d 點\x1b[31m│\x1b[m\n", oldexp, twice(d.level, 10000, 100) - d.exp);
            outs_centered("            \x1b[1;31m└──────────────────────┘\x1b[m\n");

            if (m.hp <= 0)
                vmsg("對方死掉囉..所以你贏囉..");
            else if (m.hp > 0)
                vmsg("對方落跑囉..所以算你贏囉.....");
            winorlose = 1;
        }
        if (d.death == 1 && mode == 1)
        {
            clear();
            vs_head("電子養小雞", BoardName);
            move(10, 0);
            outs_centered("            \x1b[1;31m┌──────────────────────┐\x1b[m\n");
            prints_centered("            \x1b[1;31m│  \x1b[37m可憐的小雞 \x1b[33m%-10s                     \x1b[31m│\x1b[m\n", d.name);
            prints_centered("            \x1b[1;31m│  \x1b[37m在與 \x1b[32m%-10s \x1b[37m的戰鬥中，                \x1b[31m│\x1b[m\n", p[n].name);
            outs_centered("            \x1b[1;31m│  \x1b[37m不幸地陣亡了，在此特別默哀..........      \x1b[31m│\x1b[m\n");
            outs_centered("            \x1b[1;31m└──────────────────────┘\x1b[m\n");
            vmsg("小雞陣亡了....");
            pipdie("\x1b[1;31m戰鬥中被打死了...\x1b[m  ", 1);
        }
        else if (d.death == 1 && mode == 2)
        {
            clear();
            vs_head("電子養小雞", BoardName);
            move(10, 0);
            outs_centered("            \x1b[1;31m┌──────────────────────┐\n\x1b[m");
            prints_centered("            \x1b[1;31m│  \x1b[37m可憐的小雞 \x1b[33m%-10s                     \x1b[31m│\x1b[m\n", d.name);
            prints_centered("            \x1b[1;31m│  \x1b[37m在與 \x1b[32m%-10s \x1b[37m的戰鬥中，                \x1b[31m│\x1b[m\n", p[n].name);
            outs_centered("            \x1b[1;31m│  \x1b[37m不幸地打輸了，記者現場特別報導.........   \x1b[31m│\x1b[m\n");
            outs_centered("            \x1b[1;31m└──────────────────────┘\x1b[m\n");
            vmsg("小雞打輸了....");
        }
    }
    while ((pipkey != '6') && (d.death != 1) && (m.death != 1) && (mankey != 8));
    pip_check_level();
    return winorlose;
}


/*---------------------------------------------------------------------------*/
/* 戰鬥魔法函式                                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/
#ifdef HAVE_PIP_FIGHT
static int
get_hurt(int hurt, int mexp)
{
    int dinjure;
    if (mexp > 14000)
        mexp = 14000;
    mexp = (int)mexp / 100;
    if (random() % 5 > 0)
        dinjure = (int)hurt * (60 + mexp) / 100;
    else
        dinjure = (int)hurt * (70 + mexp) / 100;
    return dinjure;
}
#endif

/*進入使用魔法選單*/
static int
pip_magic_menu(  /*戰鬥中法術的應用*/
int mode,
const UTMP *opt)
{
    char buf[256];
    int injure;         /*傷害力*/
    int pipkey;

    do
    {
        move(b_lines, 0);
        clrtoeol();
        move(b_lines, 0);
        if (mode)
        {
            sprintf(buf,
                    "\x1b[1;44;37m  魔法選單  \x1b[46m  [1]治療 [2]雷系 [3]冰系 [4]火系 [5]土系 [6]風系 [7]特殊 [Q]放棄 %*s\x1b[m", d_cols, "");
        }
        else
        {
            sprintf(buf,
                    "\x1b[1;44;37m  魔法選單  \x1b[46m  [1]治療 [2]雷系 [3]冰系 [4]火系 [5]土系 [6]風系 [Q]放棄         %*s\x1b[m", d_cols, "");
        }
        move(b_lines, 0);
        outs(buf);
        pipkey = vkey();
        switch (pipkey)
        {
        case '1':  /*治療法術*/
            d.magicmode = 1;
            injure = pip_magic_doing_menu(treatmagiclist);
            break;

        case '2':  /*雷系法術*/
            d.magicmode = 2;
            injure = pip_magic_doing_menu(thundermagiclist);
            break;

        case '3': /*冰系法術*/
            d.magicmode = 3;
            injure = pip_magic_doing_menu(icemagiclist);
            break;

        case '4': /*炎系法術*/
            d.magicmode = 4;
            injure = pip_magic_doing_menu(firemagiclist);
            show_fight_pic(341);
            vmsg("小雞使用了炎系法術");
            break;

        case '5': /*土系法術*/
            d.magicmode = 5;
            injure = pip_magic_doing_menu(earthmagiclist);
            break;

        case '6': /*風系法術*/
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

/*魔法視窗*/
static int
pip_magic_doing_menu(   /*魔法畫面*/
const struct magicset *p)
{
    register int n = 1;
    register const char *s;
    char buf[256];
    char ans[5];
    int pipkey;
    int injure = 0;

    d.nodone = 0;

    clrchyiuan(6, b_lines - 6);
    move(7, 0);
    sprintf(buf, "\x1b[1;31m┤\x1b[37;41m   可用[%s]一覽表   \x1b[0;1;31m├────────────\x1b[m", p[0].name);
    outs_centered(buf);
    while ((s = p[n].name) && (p[n].needmp <= d.mp))
    {
        move(7 + n, 4);
        if (p[n].hpmode == 1)
        {
            sprintf(buf,
                    "\x1b[1;37m[\x1b[36m%d\x1b[37m] \x1b[33m%-12s  \x1b[37m需要法力: \x1b[32m%-6d  \x1b[37m恢復體力: \x1b[32m%-6d \x1b[37m恢復疲勞: \x1b[32m%-6d\x1b[m   ", n, p[n].name, p[n].needmp, p[n].hp, p[n].tired);
            outs_centered(buf);
        }
        else if (p[n].hpmode == 2)
        {
            sprintf(buf,
                    "\x1b[1;37m[\x1b[36m%d\x1b[37m] \x1b[33m%-12s  \x1b[37m需要法力: \x1b[32m%-6d  \x1b[37m恢復體力到\x1b[35m最大值\x1b[37m 恢復疲勞到\x1b[35m最小值\x1b[m  ", n, p[n].name, p[n].needmp);
            outs_centered(buf);
        }
        else if (p[n].hpmode == 0)
        {
            sprintf(buf,
                    "\x1b[1;37m[\x1b[36m%d\x1b[37m] \x1b[33m%-12s  \x1b[37m需要法力: \x1b[32m%-6d \x1b[m             ", n, p[n].name, p[n].needmp);
            outs_centered(buf);
        }
        n++;
    }
    n -= 1;

    do
    {
        move(16, 4);
        sprintf(buf, "你想使用哪一個%8s呢?  [Q]放棄: ", p[0].name);
        getdata(16, 4, buf, ans, 2, 1, 0);
        if (ans[0] != 'q' && ans[0] != 'Q')
        {
            pipkey = atoi(ans);
        }
    }
    while (ans[0] != 'q' && ans[0] != 'Q' && (pipkey > n || pipkey <= 0));

    if (ans[0] != 'q' && ans[0] != 'Q')
    {
        getdata(17, 4, "確定使用嗎? [Y/n]: ", ans, 2, 1, 0);
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
                injure = (p[pipkey].hp + (d.maxmp / 8) - random() % 5);
                d.mp -= p[pipkey].needmp;
            }
            d.mexp += random() % 3 + pipkey;
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

#ifdef  HAVE_PIP_FIGHT
static int
pip_magic_fight_menu(  /*魔法畫面*/
const struct magicset *p,
const UTMP *opt)
{
    int n = 1, cur = 1, mg[16];
    char buf[256];
    char ans[5];
    int pipkey;
    int injure = 0;
    const struct magicset *s;

    s = p;
    d.nodone = 0;

    clrchyiuan(6, b_lines - 6);
    move(7, 0);
    sprintf(buf, "\x1b[1;31m┤\x1b[37;41m   可用[%s]一覽表   \x1b[0;1;31m├────────────\x1b[m", s->name);
    outs_centered(buf);
    s++;
    while (s->name)
    {
        move(7 + n, 4);
        if ((d.specialmagic & s->map) && (s->needmp <= d.mp))
        {
            sprintf(buf,
                    "\x1b[1;37m[\x1b[36m%d\x1b[37m] \x1b[33m%-12s  \x1b[37m需要法力: \x1b[32m%-6d \x1b[m             ", n, s->name, s->needmp);
            outs_centered(buf);
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
        sprintf(buf, "你想使用哪一個%8s呢?  [Q]放棄: ", p[0].name);
        getdata(16, 4, buf, ans, 2, 1, 0);
        if (ans[0] != 'q' && ans[0] != 'Q')
        {
            pipkey = atoi(ans);
        }
    }
    while (ans[0] != 'q' && ans[0] != 'Q' && (pipkey > n || pipkey <= 0));

    if (ans[0] != 'q' && ans[0] != 'Q')
    {
        getdata(17, 4, "確定使用嗎? [Y/n]: ", ans, 2, 1, 0);
        if (ans[0] != 'n' && ans[0] != 'N')
        {
            injure = (opt->pip->hp * p[mg[pipkey]].hp / 100 - random() % 300);
            d.mp -= p[mg[pipkey]].needmp;
            d.mexp += random() % 30 + pipkey + 100;
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
#endif  /* #ifdef  HAVE_PIP_FIGHT */
/*---------------------------------------------------------------------------*/
/* 函式特區                                                                  */
/*                                                                           */
/*---------------------------------------------------------------------------*/

/*求婚*/
static int
pip_marriage_offer(void)
{
    time_t now;
    char buf[256];
    char ans[4];
    int money;
    int who;
    const char *name[5][2] = {{"女商人Ａ", "商人Ａ"},
        {"女商人Ｂ", "商人Ｂ"},
        {"女商人Ｃ", "商人Ｃ"},
        {"女商人Ｄ", "商人Ｄ"},
        {"女商人Ｅ", "商人Ｅ"}
    };
    do
    {
        who = random() % 5;
    }
    while (d.lover == (who + 3));

    money = random() % 2000 + random() % 3000 + 4000;
    sprintf(buf, "%s帶來了金錢%d，要向你的小雞求婚，您願意嗎？[y/N]: ", name[who][d.sex-1], money);
    getdata(b_lines - 1, 1, buf, ans, 2, 1, 0);
    if (ans[0] == 'y' || ans[0] == 'Y')
    {
        if (d.wantend != 1 && d.wantend != 4)
        {
            sprintf(buf, "ㄚ～之前已經有婚約了，您確定要解除舊婚約，改定立婚約嗎？[y/N]: ");
            getdata(b_lines - 1, 1, buf, ans, 2, 1, 0);
            if (ans[0] != 'y' && ans[0] != 'Y')
            {
                d.social += 10;
                vmsg("還是維持舊婚約好了..");
                return 0;
            }
            d.social -= random() % 50 + 100;
        }
        d.charm -= random() % 5 + 20;
        d.lover = who + 3;
        d.relation -= 20;
        if (d.relation < 0)
            d.relation = 0;
        if (d.wantend < 4)
            d.wantend = 2;
        else
            d.wantend = 5;
        vmsg("我想對方是一個很好的伴侶..");
        now = time(0);
        sprintf(buf, "\x1b[1;37m%s %-11s的小雞 [%s] 接受了 %s 的求婚\x1b[0m\n", Cdate(&now), cuser.userid, d.name, name[who][d.sex-1]);
        pip_log_record(buf);
    }
    else
    {
        d.charm += random() % 5 + 20;
        d.relation += 20;
        if (d.wantend == 1 || d.wantend == 4)
            vmsg("我還年輕  心情還不定...");
        else
            vmsg("我早已有婚約了..對不起...");
    }
    d.money += money;
    return 0;
}

static int pip_results_show(void)  /*收穫季*/
{
    const char *showname[5] = {"  ", "武鬥大會", "藝術大展", "皇家舞會", "烹飪大賽"};
    char buf[256];
    int pipkey, i = 0;
    int winorlost = 0;
    int a, b[3][2], c[3] = {0, 0, 0};

    clear();
    move(10, (d_cols>>1) + 14);
    outs("\x1b[1;33m叮咚叮咚～ 辛苦的郵差幫我們送信來了喔...\x1b[0m");
    vmsg("嗯  把信打開看看吧...");
    clear();
    show_resultshow_pic(0);
    sprintf(buf, "[A]%s [B]%s [C]%s [D]%s [Q]放棄:", showname[1], showname[2], showname[3], showname[4]);
    move(b_lines, 0);
    outs(buf);
    do
    {
        pipkey = vkey();
    }
    while (pipkey != 'q' && pipkey != 'Q' && pipkey != 'A' && pipkey != 'a' &&
           pipkey != 'B' && pipkey != 'b' && pipkey != 'C' && pipkey != 'c' &&
           pipkey != 'D' && pipkey != 'd');
    a = random() % 4 + 1;
    b[0][0] = a - 1;
    b[1][0] = a + 1;
    b[2][0] = a;
    switch (pipkey)
    {
    case 'A':
    case 'a':
        vmsg("今年共有四人參賽～現在比賽開始");
        for (i = 0; i < 3; i++)
        {
            a = 0;
            b[i][1] = 0;
            sprintf(buf, "你的第%d個對手是%s", i + 1, resultmanlist[b[i][0]].name);
            vmsg(buf);
            a = pip_vs_man(b[i][0], resultmanlist, 2);
            if (a == 1)
                b[i][1] = 1;/*對方輸了*/
            winorlost += a;
            d.death = 0;
        }
        switch (winorlost)
        {
        case 3:
            pip_results_show_ending(3, 1, b[1][0], b[0][0], b[2][0]);
            d.hexp += random() % 10 + 50;
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
            d.hexp += random() % 10 + 30;
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
            d.hexp += random() % 10 + 10;
            break;
        case 0:
            pip_results_show_ending(0, 1, b[0][0], b[1][0], b[2][0]);
            d.hexp -= random() % 10 + 10;
            break;
        }
        break;
    case 'B':
    case 'b':
        vmsg("今年共有四人參賽～現在比賽開始");
        show_resultshow_pic(21);
        vmsg("比賽情形");
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
        pip_results_show_ending(winorlost, 2, random() % 2, random() % 2 + 2, random() % 2 + 4);
        d.art += random() % 10 + 20 * winorlost;
        d.character += random() % 10 + 20 * winorlost;
        break;
    case 'C':
    case 'c':
        vmsg("今年共有四人參賽～現在比賽開始");
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
        d.art += random() % 10 + 20 * winorlost;
        d.charm += random() % 10 + 20 * winorlost;
        pip_results_show_ending(winorlost, 3, random() % 2, random() % 2 + 4, random() % 2 + 2);
        break;
    case 'D':
    case 'd':
        vmsg("今年共有四人參賽～現在比賽開始");
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
        d.cookskill += random() % 10 + 20 * winorlost;
        d.family += random() % 10 + 20 * winorlost;
        pip_results_show_ending(winorlost, 4, random() % 2 + 2, random() % 2, random() % 2 + 4);
        break;
    case 'Q':
    case 'q':
        vmsg("今年不參加啦.....:(");
        d.happy -= random() % 10 + 10;
        d.satisfy -= random() % 10 + 10;
        d.relation -= random() % 10;
        break;
    }
    if (pipkey != 'Q' && pipkey != 'q')
    {
        d.tired = 0;
        d.hp = d.maxhp;
        d.happy += random() % 20;
        d.satisfy += random() % 20;
        d.relation += random() % 10;
    }
    return 0;
}

static int pip_results_show_ending(
int winorlost, int mode, int a, int b, int c)
{
    const char *gamename[5] = {"  ", "武鬥大會", "藝術大展", "皇家舞會", "烹飪大賽"};
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
    move(6, (d_cols>>1) + 13);
    prints("\x1b[1;37m～～～ \x1b[32m本屆 %s 結果揭曉 \x1b[37m～～～\x1b[0m", gamename[mode]);
    move(8, (d_cols>>1) + 15);
    prints("\x1b[1;41m 冠軍 \x1b[0;1m～ \x1b[1;33m%-10s\x1b[36m  獎金 %d\x1b[0m", name1, resultmoney[3]);
    move(10, (d_cols>>1) + 15);
    prints("\x1b[1;41m 亞軍 \x1b[0;1m～ \x1b[1;33m%-10s\x1b[36m  獎金 %d\x1b[0m", name2, resultmoney[2]);
    move(12, (d_cols>>1) + 15);
    prints("\x1b[1;41m 季軍 \x1b[0;1m～ \x1b[1;33m%-10s\x1b[36m  獎金 %d\x1b[0m", name3, resultmoney[1]);
    move(14, (d_cols>>1) + 15);
    prints("\x1b[1;41m 最後 \x1b[0;1m～ \x1b[1;33m%-10s\x1b[36m \x1b[0m", name4);
    sprintf(buf, "今年的%s結束囉 後年再來吧..", gamename[mode]);
    d.money += resultmoney[winorlost];
    vmsg(buf);
    return 0;
}

/*---------------------------------------------------------------------------*/
/* 二代雞函式特區                                                            */
/*---------------------------------------------------------------------------*/
static int
twice(
int x, int max, int min)
{
    float a, b;
    int y;
    a = (2 * max + 3 * min) / 21000;
    b = (max - min - 10000 * a) / 100;
    y = (int) a * x * x + b * x + min;
    return y;
}

static void
pip_load_mob(
const char *fpath)
{
    FILE *fp;
    int max, i;
    char buf[128];
    playrule *p;
    free(badmanlist);
    if ( ( fp = fopen(fpath, "r") ) )
    {
        fscanf(fp, "%d%s", &max, buf);
        p = badmanlist = (playrule *)malloc((max + 1) * sizeof(playrule));
        memset(badmanlist, 0, (max + 1)*sizeof(playrule));
        for (i = 0; i < max; i++)
        {
            fscanf(fp, "%s%d%d%d%d%d%d%s%d", p[i].name, &p[i].hp, &p[i].mp, &p[i].attack,
                   &p[i].resist, &p[i].speed, &p[i].money, p[i].special, &p[i].map);
            p[i].maxhp = p[i].maxmp = p[i].death = 0;
        }
        fclose(fp);
    }
}

static void
pip_load_mobset(
const char *fpath)
{
    FILE *fp;
    int i;
    if ( ( fp = fopen(fpath, "r") ) )
    {
        for (i = 0; i <= 21; i++)
        {
            fscanf(fp, "%d%d", &mob[i][0], &mob[i][1]);
        }
        fclose(fp);
    }
}

static void
levelswap(
int *cur, int num)
{
    if (*cur > num) *cur = num;
}

static void
pip_check_levelup(void)
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
pip_check_level(void)
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
pip_load_levelup(
const char *fpath)
{
    FILE *fp;
    int i, temp;
    char buf[128];
    if ( ( fp = fopen(fpath, "r") ) )
    {
        fscanf(fp, "%s", buf);
        for (i = 0; i <= 1; i++)
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
pip_fight_menu(
int mode)
{
    char inbuf1[20];
    char inbuf2[20];
    char buf[256];

    int m, color;
    int age;
    int color1, color2, color3, color4;
    char yo[12][5] = {"誕生", "嬰兒", "幼兒", "兒童", "少年", "青年",
                      "成年", "壯年", "更年", "老年", "古稀", "神仙"
                     };

    color1 = color2 = color3 = color4 = 37;
    move(1, 0);
    m = (time(0) - start_time + d.bbtime) / 60 / 30; /* 一歲 */
    /*長大一歲時的增加改變值*/
    color = 37;

    if (m == 0)                   /*誕生*/
        age = 0;
    else if (m == 1)              /*嬰兒*/
        age = 1;
    else if (m >= 2 && m <= 5)    /*幼兒*/
        age = 2;
    else if (m >= 6 && m <= 12)   /*兒童*/
        age = 3;
    else if (m >= 13 && m <= 15)  /*少年*/
        age = 4;
    else if (m >= 16 && m <= 18)  /*青年*/
        age = 5;
    else if (m >= 19 && m <= 35)  /*成年*/
        age = 6;
    else if (m >= 36 && m <= 45)  /*壯年*/
        age = 7;
    else if (m >= 45 && m <= 60)  /*更年*/
        age = 8;
    else if (m >= 60 && m <= 70)  /*老年*/
        age = 9;
    else if (m >= 70 && m <= 100) /*古稀*/
        age = 10;
    else if (m > 100)             /*神仙*/
        age = 11;
    clear();
    move(0, 0);
    if (d.sex == 1)
        sprintf(buf, "\x1b[1;41m  " NICKNAME PIPNAME " ～ \x1b[32m♂ \x1b[37m%-15s    %*s\x1b[0m", d.name, 55 + d_cols - (sizeof(NICKNAME PIPNAME) - 1), "");
    else if (d.sex == 2)
        sprintf(buf, "\x1b[1;41m  " NICKNAME PIPNAME " ～ \x1b[33m♀ \x1b[37m%-15s    %*s\x1b[0m", d.name, 55 + d_cols - (sizeof(NICKNAME PIPNAME) - 1), "");
    else
        sprintf(buf, "\x1b[1;41m  " NICKNAME PIPNAME " ～ \x1b[34m？ \x1b[37m%-15s    %*s\x1b[0m", d.name, 55 + d_cols - (sizeof(NICKNAME PIPNAME) - 1), "");
    outs(buf);

    move(1, 0);
    if (d.money <= 100)
        color1 = 31;
    else if (d.money > 100 && d.money <= 500)
        color1 = 33;
    else
        color1 = 37;
    sprintf(inbuf1, "%02d/%02d/%02d", (d.year - 11) % 100, d.month, d.day);
    sprintf(buf,
            " \x1b[1;32m[狀  態]\x1b[37m %-5s     \x1b[32m[生  日]\x1b[37m %-9s \x1b[32m[年  齡]\x1b[37m %-5d     \x1b[32m[金  錢]\x1b[%dm %-8d \x1b[m",
            yo[age], inbuf1, m, color1, d.money);
    outs_centered(buf);

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
    sprintf(buf,
            " \x1b[1;32m[生  命]\x1b[%dm %-10s\x1b[32m[法  力]\x1b[%dm %-10s\x1b[32m[體  重]\x1b[37m %-5d     \x1b[32m[疲  勞]\x1b[%dm %-4d\x1b[0m ",
            color1, inbuf1, color2, inbuf2, d.weight, color3, d.tired);
    outs_centered(buf);

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
    sprintf(buf,
            " \x1b[1;32m[髒  髒]\x1b[%dm %-4d      \x1b[32m[病  氣]\x1b[%dm %-4d      \x1b[32m[快樂度]\x1b[%dm %-4d      \x1b[32m[滿意度]\x1b[%dm %-4d\x1b[0m",
            color1, d.shit, color2, d.sick, color3, d.happy, color4, d.satisfy);
    outs_centered(buf);
    if (mode == 1)/*餵食*/
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
        sprintf(buf,
                " \x1b[1;36m[食物]\x1b[%dm%-7d\x1b[36m[零食]\x1b[%dm%-7d\x1b[36m[補丸]\x1b[%dm%-7d\x1b[36m[靈芝]\x1b[%dm%-7d\x1b[36m[人參]\x1b[37m%-7d\x1b[36m[雪蓮]\x1b[37m%-7d\x1b[0m",
                color1, d.food, color2, d.cookie, color3, d.bighp, color4, d.medicine, d.ginseng, d.snowgrass);
        outs_centered(buf);

    }
    move(5, 0);
    prints_centered("\x1b[1;%dm┌─────────────────────────────────────┐\x1b[m", color);
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


    move(b_lines - 5, 0);
    prints_centered("\x1b[1;%dm└─────────────────────────────────────┘\x1b[m", color);
    move(b_lines - 4, 0);
    outs_centered(" \x1b[1;34m─\x1b[37;44m  狀 態  \x1b[0;1;34m─\x1b[0m");
    move(b_lines - 3, 0);
    outs_centered(" 戰鬥中.............\n");

}
#endif  /* #ifdef HAVE_PIP_FIGHT */
#ifdef  HAVE_PIP_FIGHT
static int pip_fight_feed(void)     /* 餵食*/
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
        sprintf(buf, "%s該做什麼事呢?", d.name);
        outs_centered(buf);
        now = time(0);
        move(b_lines, 0);
        clrtoeol();
        move(b_lines, 0);
        prints("\x1b[1;44;37m  飲食選單  \x1b[46m[1]吃飯 [2]零食 [3]補丸 [4]靈芝 [5]人蔘 [6]雪蓮 [Q]跳出           %*s\x1b[m", d_cols, "");
        pipkey = vkey();

        switch (pipkey)
        {
        case '1':
            if (d.food <= 0)
            {
                move(b_lines, 0);
                vmsg("沒有食物囉..快去買吧！");
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
                d.weight += random() % 2;
            }
            d.nodone = 0;
            vmsg("每吃一次食物會恢復體力50喔!");
            break;

        case '2':
            if (d.cookie <= 0)
            {
                move(b_lines, 0);
                vmsg("零食吃光囉..快去買吧！");
                break;
            }
            move(4, 0);
            d.cookie--;
            d.hp += 100;
            if (d.hp >= d.maxhp)
            {
                d.hp = d.maxhp;
                d.weight += (random() % 2 + 2);
            }
            else
            {
                d.weight += (random() % 2 + 1);
            }
            if (random() % 2 > 0)
                show_feed_pic(2);
            else
                show_feed_pic(3);
            d.happy += (random() % 3 + 4);
            d.satisfy += random() % 3 + 2;
            d.nodone = 0;
            vmsg("吃零食容易胖喔...");
            break;

        case '3':
            if (d.bighp <= 0)
            {
                move(b_lines, 0);
                vmsg("沒有大補丸了耶! 快買吧..");
                break;
            }
            d.bighp--;
            d.hp += 600;
            d.tired -= 20;
            d.weight += random() % 2;
            move(4, 0);
            show_feed_pic(4);
            d.nodone = 0;
            vmsg("補丸..超級棒的唷...");
            break;

        case '4':
            if (d.medicine <= 0)
            {
                move(b_lines, 0);
                vmsg("沒有靈芝囉..快去買吧！");
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
            vmsg("每吃一次靈芝會恢復法力50喔!");
            break;

        case '5':
            if (d.ginseng <= 0)
            {
                move(b_lines, 0);
                vmsg("沒有千年人蔘耶! 快買吧..");
                break;
            }
            d.ginseng--;
            d.mp += 500;
            d.tired -= 20;
            move(4, 0);
            show_feed_pic(1);
            d.nodone = 0;
            vmsg("千年人蔘..超級棒的唷...");
            break;

        case '6':
            if (d.snowgrass <= 0)
            {
                move(b_lines, 0);
                vmsg("沒有天山雪蓮耶! 快買吧..");
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
            vmsg("天山雪蓮..超級棒的唷...");
            break;
case 'Q': case 'q': case KEY_LEFT:
            pipkey = '7';
            break;
        }
    }
    while (pipkey > '7' || pipkey < '0');

    return 0;
}
#endif  /* #ifdef  HAVE_PIP_FIGHT */
