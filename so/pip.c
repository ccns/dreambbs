/* ----------------------------------- */
/* pip.c  �i�p���{��                   */
/* ��@��: dsyan   ��g��: fennet      */
/* �Ϲ� by tiball.bbs@bbs.nhctc.edu.tw */
/* ----------------------------------- */
#define ba 5
#define START_MONEY     (3000)
#define START_FOOD      (20)
#define START_HP        (50)
#define START_HAPPY     (20)
#define START_SATISFY   (20)

#define LEARN_LEVEL     ((d.state[STATE_HAPPY]+d.state[STATE_SATISFY])/100)

#include "bbs.h"
#include "pipstruct.h"
#include <time.h>
#include "pip.h"

#define PIPNAME         "�d����"

static struct chicken d;
static time_t start_time;
static time_t lasttime;

#define getdata(x1, x2, x3, x4, x5, x6, x7)  vget(x1, x2, x3, x4, x5, DOECHO)

GCC_UNUSED static int KEY_ESC_arg;
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
static bool pip_read_file(struct chicken *ck, const char *userid);
static bool pip_write_file(const struct chicken *ck, const char *userid);
static void show_system_pic(int i);
static void pip_new_game(void);
static int pip_main_menu(void);
static int pip_live_again(void);
static void show_basic_pic(int i);
static void pip_log_record(const char *msg);
static void show_die_pic(int i);
static int pip_mainmenu(enum pipmenumode mode);
static void pip_time_change(time_t cnow);
static int pip_ending_screen(void);
static int pip_marriage_offer(void);
static void show_usual_pic(int i);
static void show_feed_pic(int i);
static int pip_buy_goods_new(enum pipshopidx mode, int item_list[]);
static int pip_weapon_doing_menu(enum pipweapon type, int item_list[WEAPON_COUNT]);
static int pip_vs_man(int n, const struct playrule *p, int mode);
static int pip_results_show_ending(int winorlost, int mode, int a, int b, int c);
static void pip_fight_bad(int n);
static void tie(void);
static void win(void);
static void situ(void);
static void lose(void);
static int pip_practice_function(enum pipclass classnum, int classgrade, int pic1, int pic2, int *change1, int *change2, int *change3, int *change4, int *change5);
static int pip_ending_decide(char *eendbuf1, char *eendbuf2, char *eendbuf3, int *endmode, int *endgrade);
static int pip_game_over(int endgrade);
static int pip_practice_gradeup(enum pipclass classnum, int classgrade, int data);
static int pip_read(const char *userid);

/*�t�ο��*/
static int pip_data_list_cuser(void), pip_system_freepip(void), pip_system_service(void);
static int pip_write_backup(void), pip_read_backup(void);
static int pip_divine(void), pip_results_show(void);

#ifdef  HAVE_PIP_FIGHT
static int pip_magic_fight_menu(const struct magicset *p, const UTMP *opt);
static int get_hurt(int hurt, int mexp);
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

/*�C���D�{��*/
int p_pipple(void)
{
    DL_HOLD;
    FILE *fs;
    int pipkey;
    char genbuf[200];

    utmp_mode(M_CHICKEN);
    more("game/pipgame/pip.welcome", NULL);
    vs_head("�q�l�i�p��", BoardName);
    srandom(time(0));
    /* sprintf(genbuf, "home/%s/chicken", cuser.userid);*/
    usr_fpath(genbuf, cuser.userid, "chicken");
    pip_load_levelup("game/pipdata/piplevel.dat");
    if (!pip_read_file(&d, cuser.userid))
        vmsg("�ڨS���i�p���� !");
    if ((fs = fopen(genbuf, "r")) == NULL)
    {
        show_system_pic(11);
        move(b_lines, 0);
        pipkey = vkey();
        if (pipkey == 'Q' || pipkey == 'q')
            return DL_RELEASE(0);
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
            return DL_RELEASE(0);
        }
        if (d.death != 0 || !d.name[0])
        {
            pip_new_game();
        }
        fclose(fs);
    }

    lasttime = time(0);
    start_time = time(0);
    /*pip_do_menu(PIPMENU_MAIN);*/
    if (d.death != 0 || !d.name[0])  return DL_RELEASE(0);
    pip_load_mob("game/pipdata/pipmob.dat");
    pip_load_mobset("game/pipdata/pipmobset.dat");
    pip_main_menu();
    free(badmanlist);
    badmanlist = NULL;
    d.bbtime += time(0) - start_time;
    pip_write_file(&d, cuser.userid);
    logit(d.thing[THING_MONEY]);
    return DL_RELEASE(0);
}

/*�ɶ���ܪk*/
static char*
dsyan_time(const time_t *t)
{
    struct tm *tp;
    static char ans[9];

    tp = localtime(t);
    sprintf(ans, "%02d/%02d/%02d", (tp->tm_year) % 100, tp->tm_mon + 1, tp->tm_mday);
    return ans;
}

/*�s�C�����]�w*/
static void pip_new_game(void)
{
    char buf[256];
    time_t now;
    static const char *const pipsex[] = {"�H", "��", "��"};
    struct tm *ptime;
    ptime = localtime(&now);

    if (d.death == 1 && !(!d.name[0]))
    {
        clear();
        vs_head(NICKNAME PIPNAME, BoardName);
        move(4, 6);
        outs("�w��Ө� \x1b[1;5;33m" NICKNAME "�ͪ���ެ�s�|\x1b[0m");
        move(6, 6);
        outs("�g�ڭ̽լd���  ���e�A���i�L�p����  �i�O�Q�A�i���F...");
        move(8, 6);
        if (d.liveagain < 4)
        {
            outs("�ڭ̥i�H���A���p���_��  ���O�ݭn�I�X�@�I�N��");
            getdata(10, 6, "�A�n�ڭ����L���Ͷ�? [y/N]: ", buf, 2, DOECHO, 0);
            if (buf[0] == 'y' || buf[0] == 'Y')
            {
                pip_live_again();
            }
        }
        else
        {
            outs("�i�O�A�_����N�Ӧh���F  �p�����W���O�}�M����");
            move(10, 6);
            outs("�ڭ̧䤣��i�H��N���a��F  �ҥH....");
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
        outs(NICKNAME PIPNAME "���C���{��������ت��k");
        move(7, 3);
        outs("�靈�����|�b�p��20���ɵ����C���A�çi���p�����򪺵o�i");
        move(8, 3);
        outs("��S�������h�@���i��p�����`�~�����C��....");
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
        d.bbtime = 0;

        /*�򥻸��*/
        d.year = ptime->tm_year;
        d.month = ptime->tm_mon + 1;
        d.day = ptime->tm_mday;
        d.death = d.nodone = d.relation = 0;
        d.liveagain = d.level = d.exp = d.dataL = 0;
        d.chickenmode = 1;

        /*����Ѽ�*/
        memset(d.body, 0, sizeof(d.body));
        d.body[BODY_HP] = random() % 15 + START_HP;
        d.body[BODY_MAXHP] = random() % 20 + START_HP;
        if (d.body[BODY_HP] > d.body[BODY_MAXHP]) d.body[BODY_HP] = d.body[BODY_MAXHP];
        d.body[BODY_WEIGHT] = random() % 10 + 50;

        /*�����Ѽ�*/
        memset(d.tmp, 0, sizeof(d.tmp));

        /*�԰��Ѽ�*/
        memset(d.fight, 0, sizeof(d.fight));

        /*�Z���Ѽ�*/
        memset(d.weapon, 0, sizeof(d.weapon));

        /*��O�Ѽ�*/
        memset(d.learn, 0, sizeof(d.learn));

        /*���A�ƭ�*/
        memset(d.state, 0, sizeof(d.state));
        d.state[STATE_HAPPY] = random() % 10 + START_HAPPY;
        d.state[STATE_SATISFY] = random() % 10 + START_SATISFY;

        /*�����Ѽ�:���� �s�� �ī~ �j�ɤY*/
        memset(d.eat, 0, sizeof(d.eat));
        d.eat[EAT_FOOD] = START_FOOD;
        d.eat[EAT_MEDICINE] = d.eat[EAT_COOKIE] = d.eat[EAT_BIGHP] = 2;

        /*���~�Ѽ�:�� ����*/
        memset(d.thing, 0, sizeof(d.thing));
        d.thing[THING_MONEY] = START_MONEY;

        /*�q���Ѽ�:Ĺ �t*/
        d.winn = d.losee = 0;

        /*�Ѩ�����*/
        memset(d.royal, 0, sizeof(d.royal));
        memset(d.see, 0, sizeof(d.see));
        d.see[SEE_ROYAL_J] = 1;

        /*�����D�B�R�H*/
        d.lover = 0;
        /*0:�S�� 1:�]�� 2:�s�� 3:A 4:B 5:C 6:D 7:E */

        memset(d.class_, 0, sizeof(d.class_));
        memset(d.work, 0, sizeof(d.work));

        /*�i���O��*/
        now = time(0);
        sprintf(buf, "\x1b[1;36m%s %-11s�i�F�@���s [%s] �� %s �p�� \x1b[0m\n", Cdate(&now), cuser.userid, d.name, pipsex[(d.sex > 0 && d.sex < COUNTOF(pipsex)) ? d.sex : 0]);
        pip_log_record(buf);
    }
    pip_write_file(&d, cuser.userid);
}

/*�p�����`�禡*/
static void
pipdie(
const char *msg,
int mode)
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
        move(14, (d_cols>>1) + 20);
        prints("�i�����p��\x1b[1;31m%s\x1b[m", msg);
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
    sprintf(genbuf, "\x1b[1;31m%s %-11s���p�� [%s] %s\x1b[m\n", Cdate(&now), cuser.userid, d.name, msg);
    pip_log_record(genbuf);
    pip_write_file(&d, cuser.userid);
}


/*pro:���v base:���� mode:���� mul:�[�v100=1 cal:�[��*/
static void
count_tired(
int prob, int base,
bool mode,
int mul,
int cal)
{
    int tiredvary = 0;
    int tm;
    /*time_t now;*/
    tm = (time(0) - start_time + d.bbtime) / 60 / 30;
    if (mode)
    {
        if (tm <= 3)
        {
            if (cal)
                tiredvary = (random() % prob + base) * d.body[BODY_MAXHP] / (d.body[BODY_HP] + 0.8 * d.body[BODY_HP]) * 120 / 100;
            else
                tiredvary = (random() % prob + base) * 4 / 3;
        }
        else if (tm <= 7)
        {
            if (cal)
                tiredvary = (random() % prob + base) * d.body[BODY_MAXHP] / (d.body[BODY_HP] + 0.8 * d.body[BODY_HP]);
            else
                tiredvary = (random() % prob + base) * 3 / 2;
        }
        else if (tm <= 10)
        {
            if (cal)
                tiredvary = (random() % prob + base) * d.body[BODY_MAXHP] / (d.body[BODY_HP] + 0.8 * d.body[BODY_HP]) * 110 / 100;
            else
                tiredvary = (random() % prob + base) * 5 / 4;
        }
        else
        {
            if (cal)
                tiredvary = (random() % prob + base) * d.body[BODY_MAXHP] / (d.body[BODY_HP] + 0.8 * d.body[BODY_HP]) * 150 / 100;
            else
                tiredvary = (random() % prob + base) * 1;
        }
    }
    else
    {
        tiredvary = random() % prob + base;
    }

    if (cal)
    {
        d.body[BODY_TIRED] += (tiredvary * mul / 100);
        if (d.body[BODY_TIRED] > 100)
            d.body[BODY_TIRED] = 100;
    }
    else
    {
        d.body[BODY_TIRED] = d.body[BODY_TIRED] - tiredvary;
        if (d.body[BODY_TIRED] <= 0)
            { d.body[BODY_TIRED] = 0; }
    }
    tiredvary = 0;
    return;
}

/*---------------------------------------------------------------------------*/
/*�D�e���M���                                                               */
/*---------------------------------------------------------------------------*/

/*�D���*/
static int pip_basic_menu(void), pip_store_menu(void), pip_practice_menu(void);
static int pip_play_menu(void), pip_job_menu(void), pip_special_menu(void), pip_system_menu(void);

static const struct pipcommands pipmainlist[] =
{
    { '1', pip_basic_menu   },
    { '2', pip_store_menu   },
    { '3', pip_practice_menu},
    { '4', pip_play_menu    },
    { '5', pip_job_menu     },
    { '6', pip_special_menu },
    { '7', pip_system_menu  },
    {'\0', NULL             }
};

/*�򥻿��*/
static int pip_basic_feed(void), pip_basic_takeshower(void), pip_basic_takerest(void), pip_basic_kiss(void);
static const struct pipcommands pipbasiclist[] =
{
    { '1', pip_basic_feed      },
    { '2', pip_basic_takeshower},
    { '3', pip_basic_takerest  },
    { '4', pip_basic_kiss      },
    { '5', pip_money           },
    {'\0', NULL                }
};

/*�ө����*/
static int pip_store_food(void), pip_store_medicine(void), pip_store_other(void);
static int pip_store_weapon_head(void), pip_store_weapon_rhand(void), pip_store_weapon_lhand(void);
static int pip_store_weapon_body(void), pip_store_weapon_foot(void);

static const struct pipcommands pipstorelist[] =
{
    { '1', pip_store_food        },
    { '2', pip_store_medicine    },
    { '3', pip_store_other       },
    { 'a', pip_store_weapon_head },
    { 'b', pip_store_weapon_rhand},
    { 'c', pip_store_weapon_lhand},
    { 'd', pip_store_weapon_body },
    { 'e', pip_store_weapon_foot },
    {'\0', NULL                  }
};

/*�צ���*/
static int pip_practice_classA(void), pip_practice_classB(void), pip_practice_classC(void);
static int pip_practice_classD(void), pip_practice_classE(void), pip_practice_classF(void);
static int pip_practice_classG(void), pip_practice_classH(void), pip_practice_classI(void);
static int pip_practice_classJ(void);

static const struct pipcommands pippracticelist[] =
{
    { 'a', pip_practice_classA},
    { 'b', pip_practice_classB},
    { 'c', pip_practice_classC},
    { 'd', pip_practice_classD},
    { 'e', pip_practice_classE},
    { 'f', pip_practice_classF},
    { 'g', pip_practice_classG},
    { 'h', pip_practice_classH},
    { 'i', pip_practice_classI},
    { 'j', pip_practice_classJ},
    {'\0', NULL               }
};

/*���ֿ��*/
static int pip_play_stroll(void), pip_play_sport(void), pip_play_date(void), pip_play_guess(void);
static int pip_play_outing(void), pip_play_kite(void), pip_play_KTV(void);

static const struct pipcommands pipplaylist[] =
{
    { '1', pip_play_stroll},
    { '2', pip_play_sport },
    { '3', pip_play_date  },
    { '4', pip_play_guess },
    { '5', pip_play_outing},
    { '6', pip_play_kite  },
    { '7', pip_play_KTV   },
    {'\0', NULL           }
};

/*���u���*/
static int pip_job_workA(void), pip_job_workB(void), pip_job_workC(void), pip_job_workD(void);
static int pip_job_workE(void), pip_job_workF(void), pip_job_workG(void), pip_job_workH(void);
static int pip_job_workI(void), pip_job_workJ(void), pip_job_workK(void), pip_job_workL(void);
static int pip_job_workM(void), pip_job_workN(void), pip_job_workO(void), pip_job_workP(void);
static const struct pipcommands pipjoblist[] =
{
    { 'a', pip_job_workA},
    { 'b', pip_job_workB},
    { 'c', pip_job_workC},
    { 'd', pip_job_workD},
    { 'e', pip_job_workE},
    { 'f', pip_job_workF},
    { 'g', pip_job_workG},
    { 'h', pip_job_workH},
    { 'i', pip_job_workI},
    { 'j', pip_job_workJ},
    { 'k', pip_job_workK},
    { 'l', pip_job_workL},
    { 'm', pip_job_workM},
    { 'n', pip_job_workN},
    { 'o', pip_job_workO},
    { 'p', pip_job_workP},
    {'\0', NULL         }
};

/*�S����*/
static int pip_see_doctor(void), pip_change_weight(void), pip_meet_vs_man(void), pip_query(void), pip_go_palace(void);
/* static int pip_vs_fight(void); */
static const struct pipcommands pipspeciallist[] =
{
    { '1', pip_see_doctor   },
    { '2', pip_change_weight},
    { '3', pip_meet_vs_man  },
    { '4', pip_query        },
    { '5', pip_go_palace    },
    /*{ 'z', pip_vs_fight     }, */
    {'\0', NULL             }
};

static const struct pipcommands pipsystemlist[] =
{
    { '1', pip_data_list_cuser},
    { '2', pip_system_freepip },
    { '3', pip_system_service },
    { '4', pip_write_backup   },
    { '5', pip_read_backup    },
    /*
    { 'o', pip_divine         },
    { 's', pip_results_show   },
    */
    {'\0', NULL               }
};

struct pipmenu {
    const struct pipcommands *cmdtable;
    enum pipmenumode mode;
    const char *name[2];
};

static const struct pipmenu pip_menu_list[PIPMENU_COUNT] =
{
    {pipmainlist, MODE_MAIN, {
        "             ",
        "\x1b[1;44;37m ��� \x1b[46m[1]�� [2]�}�� [3]�צ� [4]���� [5]���u [6]�S�� [7]�t�� [Q]���}          %*s\x1b[m"}
    },
    {pipbasiclist, MODE_MAIN, {
        "             ",
        "\x1b[1;44;37m  �򥻿��  \x1b[46m[1]���� [2]�M�� [3]�� [4]�˿� [5]����        %*s[Q]���X             \x1b[m"}
    },
    {pipstorelist, MODE_FEED, {
        "\x1b[1;44;37m �}�� \x1b[46m�i��`�Ϋ~�j[1]�K�Q�ө� [2]" NICKNAME "�ľQ [3]�]�̮ѧ�          %*s\x1b[m",
        "\x1b[1;44;37m ��� \x1b[46m�i�Z���ʳf�j[A]�Y���˳� [B]�k��˳� [C]����˳� [D]����˳� [E]�}���˳�  %*s\x1b[m"}
    },
    {pippracticelist, MODE_FIGHT, {
        "\x1b[1;44;37m �צ� \x1b[46m[A]���(%d) [B]�ֵ�(%d) [C]����(%d) [D]�x��(%d) [E]�C�N(%d)                   %*s\x1b[m",
        "\x1b[1;44;37m ��� \x1b[46m[F]�氫(%d) [G]�]�k(%d) [H]§��(%d) [I]ø�e(%d) [J]�R��(%d) [Q]���X           %*s\x1b[m"}
    },
    {pipplaylist, MODE_MAIN, {
        "   ",
        "\x1b[1;44;37m  ���ֿ��  \x1b[46m[1]���B [2]�B�� [3]���| [4]�q�� [5]�ȹC [6]���~ [7]�ۺq [Q]���X    %*s\x1b[m"}
    },
    {pipjoblist, MODE_WORK, {
        "\x1b[1;44;37m ���u \x1b[46m[A]�a�� [B]�O�i [C]���] [D]�A�� [E]�\\�U [F]�а� [G]�a�u [H]��� [I]���v  %*s\x1b[m",
        "\x1b[1;44;37m ��� \x1b[46m[J]�y�H [K]�u�a [L]�u�� [M]�a�� [N]�s�a [O]�s�� [P]�]�`�|       %*s[Q]���X  \x1b[m"}
    },
    {pipspeciallist, MODE_MAIN, {
        "\x1b[1;44;37m �S�� \x1b[46m[1]" NICKNAME "��| [2]�A�n�p [3]�԰��צ� [4]���X�B�� [5]" NICKNAME "        %*s\x1b[m",
        "\x1b[1;44;37m ��� \x1b[46m                                                                %*s[Q]���X  \x1b[m"}
    },
    {pipsystemlist, MODE_MAIN, {
        "",
        "\x1b[1;44;37m  �t�ο��  \x1b[46m[1]�ԲӸ�� [2]�p���ۥ� [3]�S�O�A�� [4]�x�s�i�� [5]Ū���i�� [Q]���X%*s\x1b[m"}
    },
};


/*����menu.c���\��*/
static int
pip_do_menu(
enum pipmenuidx menunum)
{
    time_t now;
    int pipkey;
    int goback = 0;
    int class1 = 0, class2 = 0, class3 = 0, class4 = 0, class5 = 0;
    int class6 = 0, class7 = 0, class8 = 0, class9 = 0, class10 = 0;

    const struct pipmenu *const menu = &pip_menu_list[menunum];

    do
    {
        /*�P�_�O�_���`  �����Y���^�W�@�h*/
        if (d.death == 1 || d.death == 2 || d.death == 3)
            return 0;
        /*�gpip_mainmenu�P�w��O�_���`*/
        if (pip_mainmenu(menu->mode))
            return 0;

        clrchyiuan(b_lines - 1, b_lines);
        if (menunum == PIPMENU_PRACTICE)                            /*�צ�*/
        {
            class1 = BMIN(d.learn[LEARN_WISDOM] / 200 + 1, 5);                   /*���*/
            class2 = BMIN((d.state[STATE_AFFECT] * 2 + d.learn[LEARN_WISDOM] + d.learn[LEARN_ART] * 2 + d.learn[LEARN_CHARACTER]) / 400 + 1, 5); /*�ֵ�*/
            class3 = BMIN((d.state[STATE_BELIEF] * 2 + d.learn[LEARN_WISDOM]) / 400 + 1, 5);  /*����*/
            class4 = BMIN((d.fight[FIGHT_HSKILL] * 2 + d.learn[LEARN_WISDOM]) / 400 + 1, 5);  /*�x��*/
            class5 = BMIN((d.fight[FIGHT_HSKILL] + d.fight[FIGHT_ATTACK]) / 400 + 1, 5);      /*�C�N*/
            class6 = BMIN((d.fight[FIGHT_HSKILL] + d.fight[FIGHT_RESIST]) / 400 + 1, 5);      /*�氫*/
            class7 = BMIN((d.fight[FIGHT_MSKILL] + d.fight[FIGHT_MAXMP]) / 400 + 1, 5);       /*�]�k*/
            class8 = BMIN((d.learn[LEARN_MANNERS] * 2 + d.learn[LEARN_CHARACTER]) / 400 + 1, 5); /*§��*/
            class9 = BMIN((d.learn[LEARN_ART] * 2 + d.learn[LEARN_CHARACTER]) / 400 + 1, 5);  /*ø�e*/
            class10 = BMIN((d.learn[LEARN_ART] * 2 + d.learn[LEARN_CHARM]) / 400 + 1, 5);     /*�R��*/

            move(b_lines - 1, 0);
            prints(menu->name[0], class1, class2, class3, class4, class5, d_cols, "");
            move(b_lines, 0);
            prints(menu->name[1], class6, class7, class8, class9, class10, d_cols, "");
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
            prints(menu->name[0], fill, "");

            move(b_lines, 0);
            prints(menu->name[1], d_cols, "");
        }

        now = time(0);
        pip_time_change(now);
        pipkey = vkey();
        now = time(0);
        pip_time_change(now);

        switch (pipkey)
        {
        case KEY_LEFT:
        case 'q':
        case 'Q':
            goback = 1;
            break;

        default:
            {
                int key;
                for (const struct pipcommands *cmd = menu->cmdtable; (key = cmd->key); cmd++)
                {
                    if (key == pipkey
                        || (key >= 0 && key <= 0xff && (key == tolower(pipkey))))
                    {
                        cmd->fptr();
                        break;
                    }
                }
                pip_check_levelup();
                break;
            }
        }
    }
    while (goback == 0);

    return 0;
}

/* �򥻿��:���� �M�� �˿� ��                                              */
static int pip_main_menu(void) { return pip_do_menu(PIPMENU_MAIN); }

/* �򥻿��:���� �M�� �˿� ��                                              */
static int pip_basic_menu(void) { return pip_do_menu(PIPMENU_BASIC); }

/* �ө����:���� �s�� �j�ɤY ���� �ѥ�                                       */
static int pip_store_menu(void) { return pip_do_menu(PIPMENU_STORE); }

/* �צ���:���� �m�Z �צ�                                                   */
static int pip_practice_menu(void) { return pip_do_menu(PIPMENU_PRACTICE); }

/* ���ֿ��:���B �ȹC �B�� ���| �q��                                         */
static int pip_play_menu(void) { return pip_do_menu(PIPMENU_PLAY); }

/* ���u���:�a�� �W�u �a�� �a�u                                              */
static int pip_job_menu(void) { return pip_do_menu(PIPMENU_JOB); }

/* �S����:�ݯf ��� �԰� ���X �¨�                                         */
static int pip_special_menu(void) { return pip_do_menu(PIPMENU_SPECIAL); }

/* �t�ο��:�ӤH���  �p�����  �S�O�A��                                     */
static int pip_system_menu(void) { return pip_do_menu(PIPMENU_SYSTEM); }

struct pipage {
    int age;
    const char *name;
};

/*static cosnt char yo[][5]={"�ϥ�", "����", "����", "�ൣ", "�C�~", "�֦~", "���~",
                             "���~", "���~", "���~", "��~", "�Ѧ~", "�Ѧ~", "�j�}"};*/
static const struct pipage pip_age_list[] = {
    {0, "�ϥ�"}, {1, "����"}, {5, "����"}, {12, "�ൣ"}, {15, "�֦~"}, {18, "�C�~"},
    {35, "���~"}, {45, "���~"}, {60, "��~"}, {70, "�Ѧ~"}, {100, "�j�}"}, {-1, "���P"},
};

static int pip_age_grade(int age)
{
    for (int i = 0; i < COUNTOF(pip_age_list) - 1; ++i)
        if (age <= pip_age_list[i].age)
            return i;
    return COUNTOF(pip_age_list) - 1;
}

static const char *pip_age_name(int age)
{
    return pip_age_list[pip_age_grade(age)].name;
}

static void pip_show_age_pic(int age, int weight)
{
    int weight_grade;
    int weight_std = 60 + 10 * age;
    if (weight <= (weight_std - 30))
        weight_grade = 0;
    else if (weight < (weight_std + 30))
        weight_grade = 1;
    else
        weight_grade = 2;

    switch (pip_age_grade(age))
    {
    case 0:
    case 1:
    case 2:
        show_basic_pic(1 + weight_grade);
        break;
    case 3:
    case 4:
        show_basic_pic(4 + weight_grade);
        break;
    case 5:
    case 6:
        show_basic_pic(7 + weight_grade);
        break;
    case 7:
    case 8:
        show_basic_pic(10 + weight_grade);
        break;
    case 9:
        show_basic_pic(13);
        break;
    case 10:
    case 11:
        show_basic_pic(16);
        break;
    }
}

static int
pip_mainmenu(
enum pipmenumode mode)
{
    char genbuf[200];
    time_t now;

    int tm, m, color, tm1 GCC_UNUSED, m1 GCC_UNUSED;
    int color1, color2, color3, color4;
    int anynum;
    float pc;

    color1 = color2 = color3 = color4 = 37;
    move(1, 0);
    tm = (time(0) - start_time + d.bbtime) / 60 / 30; /* �@�� */
    tm1 = (time(0) - start_time + d.bbtime) / 60;
    m = d.bbtime / 60 / 30;
    m1 = d.bbtime / 60;
    /*���j�@���ɪ��W�[���ܭ�*/
    if (m != tm)
    {
        d.learn[LEARN_WISDOM] += 10;
        d.state[STATE_HAPPY] += random() % 5 + 5;
        if (d.state[STATE_HAPPY] > 100)
            d.state[STATE_HAPPY] = 100;
        d.state[STATE_SATISFY] += random() % 5;
        if (d.state[STATE_SATISFY] > 100)
            d.state[STATE_SATISFY] = 100;
        if (tm < 13) d.body[BODY_MAXHP] += random() % 5 + 5; else d.body[BODY_MAXHP] -= random() % 15;
        d.learn[LEARN_CHARACTER] += random() % 5;
        d.thing[THING_MONEY] += 500;
        d.see[SEE_ROYAL_J] = 1;
        count_tired(1, 7, false, 100, 0);
        d.bbtime += time(0) - start_time;
        start_time = time(0);
        pip_write_file(&d, cuser.userid);

        /*�O���}�l*/
        now = time(0);
        sprintf(genbuf, "\x1b[1;37m%s %-11s���p�� [%s] �� %d ���F \x1b[m\n", Cdate(&now), cuser.userid, d.name, m + 1);
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

    if ((random() % 3000 == 29) && tm >= 15 && d.learn[LEARN_CHARM] >= 300 && d.learn[LEARN_CHARACTER] >= 300)
        pip_marriage_offer();

    if (mode != MODE_FEED && random() % 4000 == 69)
        pip_divine();

    /*�Z�x*/
    if ((time(0) - start_time) >= 900)
    {
        d.see[SEE_ROYAL_J] = 0;
    }

    clear();
    /*vs_head("�q�l�i�p��", BoardName);*/
    move(0, 0);
    if (d.sex == 1)
        prints("\x1b[1;41m  " NICKNAME PIPNAME " �� [%s�N��] \x1b[32m�� \x1b[37m%-15s      %*s\x1b[m", d.chickenmode ? "�G" : "�@", d.name, 40 + d_cols - ((int)(unsigned int)sizeof(NICKNAME PIPNAME) - 1), "");
    else if (d.sex == 2)
        prints("\x1b[1;41m  " NICKNAME PIPNAME " �� [%s�N��] \x1b[33m�� \x1b[37m%-15s      %*s\x1b[m", d.chickenmode ? "�G" : "�@", d.name, 40 + d_cols - ((int)(unsigned int)sizeof(NICKNAME PIPNAME) - 1), "");
    else
        prints("\x1b[1;41m  " NICKNAME PIPNAME " �� [%s�N��] \x1b[34m�H \x1b[37m%-15s      %*s\x1b[m", d.chickenmode ? "�G" : "�@", d.name, 40 + d_cols - ((int)(unsigned int)sizeof(NICKNAME PIPNAME) - 1), "");

    move(1, 0);
    if (d.thing[THING_MONEY] <= 100)
        color1 = 31;
    else if (d.thing[THING_MONEY] <= 500)
        color1 = 33;
    else
        color1 = 37;
    prints_centered(" \x1b[1;32m[��  �A]\x1b[37m %-5s     \x1b[32m[��  ��]\x1b[37m %02d/%02d/%02d  \x1b[32m[�~  ��]\x1b[37m %-5d     \x1b[32m[��  ��]\x1b[%dm %-8d \x1b[m",
            pip_age_name(m), (d.year - 11) % 100, d.month, d.day, tm, color1, d.thing[THING_MONEY]);

    move(2, 0);

    if ((d.body[BODY_HP]*100 / d.body[BODY_MAXHP]) <= 20)
        color1 = 31;
    else if ((d.body[BODY_HP]*100 / d.body[BODY_MAXHP]) <= 40)
        color1 = 33;
    else
        color1 = 37;

    if (d.fight[FIGHT_MAXMP] <= 0)
        color2 = 37;
    else if ((d.fight[FIGHT_MP]*100 / d.fight[FIGHT_MAXMP]) <= 20)
        color2 = 31;
    else if ((d.fight[FIGHT_MP]*100 / d.fight[FIGHT_MAXMP]) <= 40)
        color2 = 33;
    else
        color2 = 37;

    if (d.body[BODY_TIRED] >= 80)
        color3 = 31;
    else if (d.body[BODY_TIRED] >= 60)
        color3 = 33;
    else
        color3 = 37;

    prints_centered(" \x1b[1;32m[��  �R]\x1b[%dm %-10d\x1b[32m[�k  �O]\x1b[%dm %-10d\x1b[32m[��  ��]\x1b[37m %-5d     \x1b[32m[�h  ��]\x1b[%dm %-4d\x1b[0m ",
            color1, d.body[BODY_HP], color2, d.fight[FIGHT_MP], d.body[BODY_WEIGHT], color3, d.body[BODY_TIRED]);

    move(3, 0);
    if (d.body[BODY_SHIT] >= 80)
        color1 = 31;
    else if (d.body[BODY_SHIT] >= 60)
        color1 = 33;
    else
        color1 = 37;
    if (d.body[BODY_SICK] >= 75)
        color2 = 31;
    else if (d.body[BODY_SICK] >= 50)
        color2 = 33;
    else
        color2 = 37;
    if (d.state[STATE_HAPPY] <= 20)
        color3 = 31;
    else if (d.state[STATE_HAPPY] <= 40)
        color3 = 33;
    else
        color3 = 37;
    if (d.state[STATE_SATISFY] <= 20)
        color4 = 31;
    else if (d.state[STATE_SATISFY] <= 40)
        color4 = 33;
    else
        color4 = 37;
    prints_centered(" \x1b[1;32m[�R MAX]\x1b[37m %-10d\x1b[32m[�k MAX]\x1b[37m %-10d\x1b[32m[ż���f]\x1b[%dm %-4d\x1b[37m/\x1b[%dm%-4d \x1b[32m[�֡���]\x1b[%dm %-4d\x1b[37m/\x1b[%dm%-4d\x1b[m",
            d.body[BODY_MAXHP], d.fight[FIGHT_MAXMP], color1, d.body[BODY_SHIT], color2, d.body[BODY_SICK], color3, d.state[STATE_HAPPY], color4, d.state[STATE_SATISFY]);
    switch (mode)
    {
    case MODE_MAIN:
        anynum = random() % 4;
        move(4, 0);
        if (anynum == 0)
            prints_centered(" \x1b[1;35m[������]:\x1b[31m����\x1b[36m��ܦM�I  \x1b[33m����\x1b[36m���ĵ�i  \x1b[37m�զ�\x1b[36m��ܦw��\x1b[0m");
        else if (anynum == 1)
            prints_centered(" \x1b[1;35m[������]:\x1b[37m�n�h�h�`�N�p�����h�ҫשM�f��  �H�K�֦��f��\x1b[0m");
        else if (anynum == 2)
            prints_centered(" \x1b[1;35m[������]:\x1b[37m�H�ɪ`�N�p�����ͩR�ƭȭ�!\x1b[0m");
        else if (anynum == 3)
            prints_centered(" \x1b[1;35m[������]:\x1b[37m�ּּ֧֪��p���~�O���֪��p��.....\x1b[0m");
        break;

    case MODE_FEED:
        move(4, 0);
        if (d.eat[EAT_FOOD] <= 0)
            color1 = 31;
        else if (d.eat[EAT_FOOD] <= 5)
            color1 = 33;
        else
            color1 = 37;
        if (d.eat[EAT_COOKIE] <= 0)
            color2 = 31;
        else if (d.eat[EAT_COOKIE] <= 5)
            color2 = 33;
        else
            color2 = 37;
        if (d.eat[EAT_BIGHP] <= 0)
            color3 = 31;
        else if (d.eat[EAT_BIGHP] <= 2)
            color3 = 33;
        else
            color3 = 37;
        if (d.eat[EAT_MEDICINE] <= 0)
            color4 = 31;
        else if (d.eat[EAT_MEDICINE] <= 5)
            color4 = 33;
        else
            color4 = 37;
        prints_centered(" \x1b[1;36m[����]\x1b[%dm%-7d\x1b[36m[�s��]\x1b[%dm%-7d\x1b[36m[�ɤY]\x1b[%dm%-7d\x1b[36m[�F��]\x1b[%dm%-7d\x1b[36m[�H��]\x1b[37m%-7d\x1b[36m[����]\x1b[37m%-7d\x1b[0m",
                color1, d.eat[EAT_FOOD], color2, d.eat[EAT_COOKIE], color3, d.eat[EAT_BIGHP], color4, d.eat[EAT_MEDICINE], d.eat[EAT_GINSENG], d.eat[EAT_SNOWGRASS]);
        break;

    case MODE_WORK:
        move(4, 0);
        prints_centered(" \x1b[1;36m[�R��]\x1b[37m%-5d\x1b[36m[���z]\x1b[37m%-5d\x1b[36m[���]\x1b[37m%-5d\x1b[36m[���N]\x1b[37m%-5d\x1b[36m[�D�w]\x1b[37m%-5d\x1b[36m[�i��]\x1b[37m%-5d\x1b[36m[�a��]\x1b[37m%-5d\x1b[0m",
                d.learn[LEARN_LOVE], d.learn[LEARN_WISDOM], d.learn[LEARN_CHARACTER], d.learn[LEARN_ART], d.learn[LEARN_ETHICS], d.learn[LEARN_BRAVE], d.learn[LEARN_HOMEWORK]);
        break;

    case MODE_FIGHT:
        move(4, 0);
        prints_centered(" \x1b[1;36m[���z]\x1b[37m%-5d\x1b[36m[���]\x1b[37m%-5d\x1b[36m[���N]\x1b[37m%-5d\x1b[36m[�i��]\x1b[37m%-5d\x1b[36m[����]\x1b[37m%-5d\x1b[36m[���m]\x1b[37m%-5d\x1b[36m[�t��]\x1b[37m%-5d\x1b[0m",
                d.learn[LEARN_WISDOM], d.learn[LEARN_CHARACTER], d.learn[LEARN_ART], d.learn[LEARN_BRAVE], d.fight[FIGHT_ATTACK], d.fight[FIGHT_RESIST], d.fight[FIGHT_SPEED]);
        break;
    }

    move(5, 0);
    prints_centered("\x1b[1;%dm�z�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�{\x1b[m", color);

    move(6, 0);
    pip_show_age_pic(tm, d.body[BODY_WEIGHT]);

    move(b_lines - 5, 0);
    prints_centered("\x1b[1;%dm�|�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�}\x1b[m", color);
    move(b_lines - 4, 0);
    outs_centered(" \x1b[1;34m�w\x1b[37;44m  �� �A  \x1b[0;1;34m�w\x1b[0m");
    move(b_lines - 3, 0);
    outs_centered(" ");

    if (d.body[BODY_SHIT] <= 0)
        outs("���b�p��  ");
    else if (d.body[BODY_SHIT] <= 40)
        ;
    else if (d.body[BODY_SHIT] < 60)
        outs("���I���  ");
    else if (d.body[BODY_SHIT] < 80)
        outs("\x1b[1;33m�ܯ�F��\x1b[m  ");
    else if (d.body[BODY_SHIT] < 100)
    {
        outs("\x1b[1;35m�֯䦺�F\x1b[m  ");
        d.body[BODY_SICK] += 4;
        d.learn[LEARN_CHARACTER] -= (random() % 3 + 3);
    }
    else
    {
        d.death = 1;
        pipdie("\x1b[1;31m�z��䦺�F\x1b[m  ", 1);
        return -1;
    }

    if (d.body[BODY_HP] <= 0)
        pc = 0;
    else
        pc = d.body[BODY_HP] * 100 / d.body[BODY_MAXHP];
    if (pc <= 0)
    {
        d.death = 1;
        pipdie("\x1b[1;31m���j���F\x1b[m  ", 1);
        return -1;
    }
    else if (pc < 20)
    {
        outs("\x1b[1;35m�־j���F\x1b[m  ");
        d.body[BODY_SICK] += 3;
        d.state[STATE_HAPPY] -= 5;
        d.state[STATE_SATISFY] -= 3;
    }
    else if (pc < 40)
        outs("\x1b[1;33m�Q�Y�F��\x1b[m  ");
    else if (pc < 90)
        ;
    else if (pc <= 100)
        outs("�{�l����  ");
    else
        outs("\x1b[1;33m��������\x1b[m  ");

    pc = d.body[BODY_TIRED];
    if (pc < 20)
        outs("�믫�ܦn  ");
    else if (pc < 60)
        ;
    else if (pc < 80)
        outs("\x1b[1;33m���I�p��\x1b[m  ");
    else if (pc < 100)
    {
        outs("\x1b[1;35m�u���ܲ�\x1b[m  ");
        d.body[BODY_SICK] += 5;
    }
    else
    {
        d.death = 1;
        pipdie("\x1b[1;31m����֦��F\x1b[m  ", 1);
        return -1;
    }

    pc = 60 + 10 * tm;
    if (d.body[BODY_WEIGHT] < (pc - 50))
    {
        d.death = 1;
        pipdie("\x1b[1;31m:~~ �G���F\x1b[m  ", 1);
        return -1;
    }
    else if (d.body[BODY_WEIGHT] <= (pc - 30))
        outs("\x1b[1;35m�ӽG�F��\x1b[m ");
    else if (d.body[BODY_WEIGHT] <= (pc - 10))
        outs("\x1b[1;33m���I�p�G\x1b[m  ");
    else if (d.body[BODY_WEIGHT] < (pc + 10))
        ;
    else if (d.body[BODY_WEIGHT] < (pc + 30))
        outs("\x1b[1;33m���I�p�D\x1b[m  ");
    else if (d.body[BODY_WEIGHT] < (pc + 50))
    {
        outs("\x1b[1;35m�ӭD�F��\x1b[m  ");
        d.body[BODY_SICK] += 3;
        if (d.fight[FIGHT_SPEED] >= 2)
            d.fight[FIGHT_SPEED] -= 2;
        else
            d.fight[FIGHT_SPEED] = 0;

    }
    else
    {
        d.death = 1;
        pipdie("\x1b[1;31m���Φ��F\x1b[m  ", 1);
        return -1;
    }

    if (d.body[BODY_SICK] < 50)
        ;
    else if (d.body[BODY_SICK] < 75)
    {
        outs("\x1b[1;33m�ͯf�F��\x1b[m  ");
        count_tired(1, 8, true, 100, 1);
    }
    else if (d.body[BODY_SICK] < 100)
    {
        outs("\x1b[1;35m���f����\x1b[m  ");
        d.body[BODY_SICK] += 5;
        count_tired(1, 15, true, 100, 1);
    }
    else
    {
        d.death = 1;
        pipdie("\x1b[1;31m�f���F�� :~~\x1b[m  ", 1);
        return -1;
    }

    pc = d.state[STATE_HAPPY];
    if (pc < 20)
        outs("\x1b[1;35m�ܤ��ּ�\x1b[m  ");
    else if (pc < 40)
        outs("\x1b[1;33m���ӧּ�\x1b[m  ");
    else if (pc < 80)
        ;
    else if (pc < 95)
        outs("�ְּ�..  ");
    else
        outs("�ܧּ�..  ");

    pc = d.state[STATE_SATISFY];
    if (pc < 20)
        outs("\x1b[1;35m�ܤ�����..\x1b[m  ");
    else if (pc < 40)
        outs("\x1b[1;33m���Ӻ���\x1b[m  ");
    else if (pc < 80)
        ;
    else if (pc < 95)
        outs("������..  ");
    else
        outs("�ܺ���..  ");

    outs("\n");

    pip_write_file(&d, cuser.userid);
    return 0;
}

/*�T�w�ɶ��@���� */
static void
pip_time_change(
time_t cnow)
{
    int stime = 60;
    int stired = 2;
    while ((time(0) - lasttime) >= stime) /* �T�w�ɶ������� */
    {
        /*������  �٬O�|��ż��*/
        if ((time(0) - cnow) >= stime)
            d.body[BODY_SHIT] += (random() % 3 + 3);
        /*������  �h�ҷ�M��C��*/
        if (d.body[BODY_TIRED] >= stired) d.body[BODY_TIRED] -= stired; else d.body[BODY_TIRED] = 0;
        /*������  �{�l�]�|�j�� */
        d.body[BODY_HP] -= random() % 2 + 2;
        if (d.tmp[TMP_MEXP] < 0)
            d.tmp[TMP_MEXP] = 0;
        if (d.tmp[TMP_HEXP] < 0)
            d.tmp[TMP_HEXP] = 0;
        /*��O�|�]�ͯf���C�@�I*/
        d.body[BODY_HP] -= d.body[BODY_SICK] / 10;
        /*�f��|�H���v�W�[��ֳ֤\*/
        if (random() % 3 > 0)
        {
            d.body[BODY_SICK] -= random() % 2;
            if (d.body[BODY_SICK] < 0)
                d.body[BODY_SICK] = 0;
        }
        else
            d.body[BODY_SICK] += random() % 2;
        /*�H����ּ֫�*/
        if (random() % 4 > 0)
        {
            d.state[STATE_HAPPY] -= random() % 2 + 2;
        }
        else
            d.state[STATE_HAPPY] += 2;
        if (random() % 4 > 0)
        {
            d.state[STATE_SATISFY] -= (random() % 4 + 5);
        }
        else
            d.state[STATE_SATISFY] += 2;
        lasttime += stime;
    }
    /*�ּ֫׺��N�׳̤j�ȳ]�w*/
    if (d.state[STATE_HAPPY] > 100)
        d.state[STATE_HAPPY] = 100;
    else if (d.state[STATE_HAPPY] < 0)
        d.state[STATE_HAPPY] = 0;
    if (d.state[STATE_SATISFY] > 100)
        d.state[STATE_SATISFY] = 100;
    else if (d.state[STATE_SATISFY] < 0)
        d.state[STATE_SATISFY] = 0;
    /*����*/
    if (d.tmp[TMP_SOCIAL] < 0)
        d.tmp[TMP_SOCIAL] = 0;
    if (d.body[BODY_TIRED] < 0)
        d.body[BODY_TIRED] = 0;
    if (d.body[BODY_HP] > d.body[BODY_MAXHP])
        d.body[BODY_HP] = d.body[BODY_MAXHP];
    if (d.fight[FIGHT_MP] > d.fight[FIGHT_MAXMP])
        d.fight[FIGHT_MP] = d.fight[FIGHT_MAXMP];
    if (d.thing[THING_MONEY] < 0)
        d.thing[THING_MONEY] = 0;
    if (d.learn[LEARN_CHARM] < 0)
        d.learn[LEARN_CHARM] = 0;
}

/*---------------------------------------------------------------------------*/
/* �򥻿��:���� �M�� �˿� ��                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static int pip_basic_takeshower(void) /*�~��*/
{
    int lucky;
    d.body[BODY_SHIT] -= 20;
    if (d.body[BODY_SHIT] < 0) d.body[BODY_SHIT] = 0;
    d.body[BODY_HP] -= random() % 2 + 3;
    move(4, 0);
    lucky = random() % 3;
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

static int pip_basic_takerest(void) /*��*/
{
    count_tired(5, 20, true, 100, 0);
    if (d.body[BODY_HP] > d.body[BODY_MAXHP])
        d.body[BODY_HP] = d.body[BODY_MAXHP];
    d.body[BODY_SHIT] += 1;
    move(4, 0);
    show_usual_pic(5);
    vmsg("�A���@�U�ڴN�_���o....");
    show_usual_pic(6);
    vmsg("�޳޳�..�Ӱ_���o......");
    return 0;
}

static int pip_basic_kiss(void)/*�˿�*/
{
    if (random() % 2 > 0)
    {
        d.state[STATE_HAPPY] += random() % 3 + 4;
        d.state[STATE_SATISFY] += random() % 2 + 1;
    }
    else
    {
        d.state[STATE_HAPPY] += random() % 2 + 1;
        d.state[STATE_SATISFY] += random() % 3 + 4;
    }
    count_tired(1, 2, false, 100, 1);
    d.body[BODY_SHIT] += random() % 5 + 4;
    d.relation += random() % 2;
    move(4, 0);
    show_usual_pic(3);
    if (d.body[BODY_SHIT] < 60)
    {
        vmsg("�ӹ�! �q�@��.....");
    }
    else
    {
        vmsg("�ˤӦh�]�O�|ż������....");
    }
    return 0;
}

static int pip_basic_feed(void)     /* ����*/
{
    time_t now;
    int pipkey;

    d.nodone = 1;

    do
    {
        if (d.death == 1 || d.death == 2 || d.death == 3)
            return 0;
        if (pip_mainmenu(MODE_FEED)) return 0;
        move(b_lines -2, 0);
        clrtoeol();
        move(b_lines - 2, 1);
        prints_centered("%s�Ӱ�����ƩO?", d.name);
        now = time(0);
        move(b_lines, 0);
        clrtoeol();
        move(b_lines, 0);
        prints("\x1b[1;44;37m  �������  \x1b[46m[1]�Y�� [2]�s�� [3]�ɤY [4]�F�� [5]�H�x [6]���� [Q]���X            %*s\x1b[m", d_cols, "");
        pip_time_change(now);
        pipkey = vkey();
        pip_time_change(now);

        switch (pipkey)
        {
        case '1':
            if (d.eat[EAT_FOOD] <= 0)
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
            d.eat[EAT_FOOD]--;
            d.body[BODY_HP] += 50;
            if (d.body[BODY_HP] >= d.body[BODY_MAXHP])
            {
                d.body[BODY_HP] = d.body[BODY_MAXHP];
                d.body[BODY_WEIGHT] += random() % 2;
            }
            d.nodone = 0;
            vmsg("�C�Y�@�������|��_��O50��!");
            break;

        case '2':
            if (d.eat[EAT_COOKIE] <= 0)
            {
                move(b_lines, 0);
                vmsg("�s���Y���o..�֥h�R�a�I");
                break;
            }
            move(4, 0);
            d.eat[EAT_COOKIE]--;
            d.body[BODY_HP] += 100;
            if (d.body[BODY_HP] >= d.body[BODY_MAXHP])
            {
                d.body[BODY_HP] = d.body[BODY_MAXHP];
                d.body[BODY_WEIGHT] += (random() % 2 + 2);
            }
            else
            {
                d.body[BODY_WEIGHT] += (random() % 2 + 1);
            }
            if (random() % 2 > 0)
                show_feed_pic(2);
            else
                show_feed_pic(3);
            d.state[STATE_HAPPY] += (random() % 3 + 4);
            d.state[STATE_SATISFY] += random() % 3 + 2;
            d.nodone = 0;
            vmsg("�Y�s���e���D��...");
            break;

        case '3':
            if (d.eat[EAT_BIGHP] <= 0)
            {
                move(b_lines, 0);
                vmsg("�S���j�ɤY�F�C! �ֶR�a..");
                break;
            }
            d.eat[EAT_BIGHP]--;
            d.body[BODY_HP] += 600;
            d.body[BODY_TIRED] -= 20;
            d.body[BODY_WEIGHT] += random() % 2;
            move(4, 0);
            show_feed_pic(4);
            d.nodone = 0;
            vmsg("�ɤY..�W�ŴΪ���...");
            break;

        case '4':
            if (d.eat[EAT_MEDICINE] <= 0)
            {
                move(b_lines, 0);
                vmsg("�S���F���o..�֥h�R�a�I");
                break;
            }
            move(4, 0);
            show_feed_pic(1);
            d.eat[EAT_MEDICINE]--;
            d.fight[FIGHT_MP] += 50;
            if (d.fight[FIGHT_MP] >= d.fight[FIGHT_MAXMP])
            {
                d.fight[FIGHT_MP] = d.fight[FIGHT_MAXMP];
            }
            d.nodone = 0;
            vmsg("�C�Y�@���F�۷|��_�k�O50��!");
            break;

        case '5':
            if (d.eat[EAT_GINSENG] <= 0)
            {
                move(b_lines, 0);
                vmsg("�S���d�~�H�x�C! �ֶR�a..");
                break;
            }
            d.eat[EAT_GINSENG]--;
            d.fight[FIGHT_MP] += 500;
            d.body[BODY_TIRED] -= 20;
            move(4, 0);
            show_feed_pic(1);
            d.nodone = 0;
            vmsg("�d�~�H�x..�W�ŴΪ���...");
            break;

        case '6':
            if (d.eat[EAT_SNOWGRASS] <= 0)
            {
                move(b_lines, 0);
                vmsg("�S���Ѥs�����C! �ֶR�a..");
                break;
            }
            d.eat[EAT_SNOWGRASS]--;
            d.fight[FIGHT_MP] = d.fight[FIGHT_MAXMP];
            d.body[BODY_HP] = d.body[BODY_MAXHP];
            d.body[BODY_TIRED] -= 0;
            d.body[BODY_SICK] = 0;
            move(4, 0);
            show_feed_pic(1);
            d.nodone = 0;
            vmsg("�Ѥs����..�W�ŴΪ���...");
            break;

        }
    }
    while ((pipkey != 'Q') && (pipkey != 'q') && (pipkey != KEY_LEFT));

    return 0;
}

/*�C���g��ƤJ�ɮ�*/
static void pip_save_array(FILE *ff, const int *arr, int count)
{
    for (int i = 0; i < count; i++)
        fprintf(ff, "%d ", arr[i]);
    fprintf(ff, "\n");
}

static bool pip_write_file(const struct chicken *ck, const char *userid)
{
    FILE *ff;
    char buf[200];
    /* sprintf(buf, "home/%s/chicken", userid);*/
    usr_fpath(buf, userid, "chicken");

    if ((ff = fopen(buf, "w")))
    {
        fprintf(ff, "%ld\n", ck->bbtime);
        fprintf(ff,
            "%d %d %d %d \n" "%d %d %d %d \n" "%d %d %d %d \n",
            ck->year, ck->month, ck->day, ck->sex,
            ck->death, ck->nodone, ck->relation, ck->liveagain,
            ck->chickenmode, ck->level, ck->exp, ck->dataL);

        pip_save_array(ff, ck->body, COUNTOF(ck->body));
        pip_save_array(ff, ck->tmp, COUNTOF(ck->tmp));
        pip_save_array(ff, ck->fight, COUNTOF(ck->fight));
        pip_save_array(ff, ck->weapon, COUNTOF(ck->weapon));
        pip_save_array(ff, ck->learn, COUNTOF(ck->learn));
        pip_save_array(ff, ck->state, COUNTOF(ck->state));
        pip_save_array(ff, ck->eat, COUNTOF(ck->eat));
        pip_save_array(ff, ck->thing, COUNTOF(ck->thing));

        fprintf(ff, "%d %d \n", ck->winn, ck->losee);

        pip_save_array(ff, ck->royal, COUNTOF(ck->royal));
        pip_save_array(ff, ck->see, COUNTOF(ck->see));

        fprintf(ff, "%d %d %s\n", ck->wantend, ck->lover, ck->name);

        pip_save_array(ff, ck->class_, COUNTOF(ck->class_));
        pip_save_array(ff, ck->work, COUNTOF(ck->work));

        fclose(ff);
        return true;
    }
    return false;
}

/*�C��Ū��ƥX�ɮ�*/
static void pip_read_array(FILE *fs, int *arr, int count)
{
    for (int i = 0; i < count; i++)
        fscanf(fs, "%d", &arr[i]);
}

static bool pip_read_file(struct chicken *ck, const char *userid)
{
    FILE *fs;
    char buf[200];
    /* sprintf(buf, "home/%s/chicken", userid);*/
    usr_fpath(buf, userid, "chicken");

    if ((fs = fopen(buf, "r")))
    {
        fgets(buf, 80, fs);
        ck->bbtime = (time_t) atol(buf);

        fscanf(fs,
            "%d%d%d%d" "%d%d%d%d" "%d%d%d%d",
            &ck->year, &ck->month, &ck->day, &ck->sex,
            &ck->death, &ck->nodone, &ck->relation, &ck->liveagain,
            &ck->chickenmode, &ck->level, &ck->exp, &ck->dataL);

        pip_read_array(fs, ck->body, COUNTOF(ck->body));
        pip_read_array(fs, ck->tmp, COUNTOF(ck->tmp));
        pip_read_array(fs, ck->fight, COUNTOF(ck->fight));
        pip_read_array(fs, ck->weapon, COUNTOF(ck->weapon));
        pip_read_array(fs, ck->learn, COUNTOF(ck->learn));
        pip_read_array(fs, ck->state, COUNTOF(ck->state));
        pip_read_array(fs, ck->eat, COUNTOF(ck->eat));
        pip_read_array(fs, ck->thing, COUNTOF(ck->thing));

        fscanf(fs, "%d%d", &ck->winn, &ck->losee);

        pip_read_array(fs, ck->royal, COUNTOF(ck->royal));
        pip_read_array(fs, ck->see, COUNTOF(ck->see));

        fscanf(fs, "%d%d%s", &ck->wantend, &ck->lover, ck->name);

        pip_save_array(fs, ck->class_, COUNTOF(ck->class_));
        pip_save_array(fs, ck->work, COUNTOF(ck->work));

        fclose(fs);
        return true;
    }
    return false;
}

/*�O����pip.log��*/
static void
pip_log_record(
const char *msg)
{
    FILE *fs;

    fs = fopen(FN_PIP_LOG, "a+");
    fprintf(fs, "%s", msg);
    fclose(fs);
}

/*�p���i���x�s*/
static int
pip_write_backup(void)
{
    static const char *const files[] = {"�i�פ@", "�i�פG", "�i�פT"};
    char buf[200], buf1[200];
    char ans[3];
    int num = 0;
    int pipkey;

    show_system_pic(21);
    pip_write_file(&d, cuser.userid);
    do
    {
        move(b_lines - 2, 0);
        clrtoeol();
        move(b_lines - 1, 0);
        clrtoeol();
        move(b_lines - 1, 1);
        outs("�x�s [1]�i�פ@ [2]�i�פG [3]�i�פT [Q]��� [1/2/3/Q]�G");
        pipkey = vkey();

        num = pipkey - '0';
    }
    while (pipkey != 'Q' && pipkey != 'q' && (num < 1 || num > COUNTOF(files)));
    if (pipkey == 'q' || pipkey == 'Q')
    {
        vmsg("����x�s�C���i��");
        return 0;
    }
    move(b_lines -2, 1);
    prints("�x�s�ɮ׷|�л\\���x�s�� [%s] ���p�����ɮ׳�I�ЦҼ{�M��...", files[num - 1]);
    sprintf(buf1, "�T�w�n�x�s�� [%s] �ɮ׶ܡH [y/N]: ", files[num - 1]);
    getdata(B_LINES_REF - 1, 1, buf1, ans, 2, DOECHO, 0);
    if (ans[0] != 'y' && ans[0] != 'Y')
    {
        vmsg("����x�s�ɮ�");
        return 0;
    }

    move(b_lines -1, 0);
    clrtobot();
    sprintf(buf1, "�x�s [%s] �ɮק����F", files[num - 1]);
    vmsg(buf1);
    sprintf(buf, "%s%d", get_path(cuser.userid, "chicken.bak"), num);
    PROC_CMD("/bin/cp", get_path(cuser.userid, "chicken"), buf);
    return 0;
}

static int
pip_read_backup(void)
{
    char buf[200], buf1[200], buf2[200];
    static const char *const files[] = {"�i�פ@", "�i�פG", "�i�פT"};
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
        outs("Ū�� [1]�i�פ@ [2]�i�פG [3]�i�פT [Q]��� [1/2/3/Q]�G");
        pipkey = vkey();

        num = pipkey - '0';

        if (num > 0 && num <= COUNTOF(files))
        {
            usr_fpath(buf, cuser.userid, "chicken.bak");
            sprintf(buf + strlen(buf), "%d", num);
            if ((fs = fopen(buf, "r")) == NULL)
            {
                sprintf(buf, "�ɮ� [%s] ���s�b", files[num - 1]);
                vmsg(buf);
                ok = 0;
            }
            else
            {

                move(b_lines - 2, 1);
                outs("Ū���X�ɮ׷|�л\\�{�b���b�����p�����ɮ׳�I�ЦҼ{�M��...");
                sprintf(buf, "�T�w�nŪ���X [%s] �ɮ׶ܡH [y/N]: ", files[num - 1]);
                getdata(B_LINES_REF - 1, 1, buf, ans, 2, DOECHO, 0);
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
    sprintf(buf, "Ū�� [%s] �ɮק����F", files[num - 1]);
    vmsg(buf);

    sprintf(buf1, "%s%d", get_path(cuser.userid, "chicken.bak"), num);
    PROC_CMD("/bin/touch", buf1);
    PROC_CMD("/bin/cp", buf1, get_path(cuser.userid, "chicken"));
    pip_read_file(&d, cuser.userid);
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
    vs_head("�p���_����N��", BoardName);

    now = time(0);
    sprintf(genbuf, "\x1b[1;33m%s %-11s���p�� [%s�G�N] �_���F�I\x1b[m\n", Cdate(&now), cuser.userid, d.name);
    pip_log_record(genbuf);

    /*����W���]�w*/
    d.death = 0;
    d.body[BODY_MAXHP] = d.body[BODY_MAXHP] * ALIVE + 1;
    d.body[BODY_HP] = d.body[BODY_MAXHP];
    d.body[BODY_TIRED] = 20;
    d.body[BODY_SHIT] = 20;
    d.body[BODY_SICK] = 20;
    d.body[BODY_WRIST] = d.body[BODY_WRIST] * ALIVE;
    d.body[BODY_WEIGHT] = 45 + 10 * tm;

    /*����줭�����@*/
    d.thing[THING_MONEY] = d.thing[THING_MONEY] * ALIVE;

    /*�԰���O���@�b*/
    d.fight[FIGHT_ATTACK] = d.fight[FIGHT_ATTACK] * ALIVE;
    d.fight[FIGHT_RESIST] = d.fight[FIGHT_RESIST] * ALIVE;
    d.fight[FIGHT_MAXMP] = d.fight[FIGHT_MAXMP] * ALIVE;
    d.fight[FIGHT_MP] = d.fight[FIGHT_MAXMP];

    /*�ܱo���ּ�*/
    d.state[STATE_HAPPY] = 0;
    d.state[STATE_SATISFY] = 0;

    /*������b*/
    d.tmp[TMP_SOCIAL] = d.tmp[TMP_SOCIAL] * ALIVE;
    d.tmp[TMP_FAMILY] = d.tmp[TMP_FAMILY] * ALIVE;
    d.tmp[TMP_HEXP] = d.tmp[TMP_HEXP] * ALIVE;
    d.tmp[TMP_MEXP] = d.tmp[TMP_MEXP] * ALIVE;

    /*�Z��������*/
    d.weapon[WEAPON_HEAD] = 0;
    d.weapon[WEAPON_RHAND] = 0;
    d.weapon[WEAPON_LHAND] = 0;
    d.weapon[WEAPON_BODY] = 0;
    d.weapon[WEAPON_FOOT] = 0;

    /*�����Ѥ@�b*/
    d.eat[EAT_FOOD] = d.eat[EAT_FOOD] * ALIVE;
    d.eat[EAT_MEDICINE] = d.eat[EAT_MEDICINE] * ALIVE;
    d.eat[EAT_BIGHP] = d.eat[EAT_BIGHP] * ALIVE;
    d.eat[EAT_COOKIE] = d.eat[EAT_COOKIE] * ALIVE;

    d.liveagain += 1;

    vmsg("�p�����x���ؤ��I");
    vmsg("�p������_���I");
    vmsg("�p����O�վ㤤�I");
    vmsg("���߱z�A�A���p���S�_���o�I");
    pip_write_file(&d, cuser.userid);
    return 0;
}

/*---------------------------------------------------------------------------*/
/* �p���ϧΰ�                                                                */
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
show_feed_pic(int i)  /*�Y�F��*/
{
    char buf[256];
    clrchyiuan(6, b_lines - 6);
    clrchyiuan(b_lines - 1, b_lines);
    sprintf(buf, BBSHOME"/game/pipgame/feed/pic%d", i);
    show_file(buf, 6, 12, ONLY_COLOR);
}

static void
show_buy_pic(int i)  /*�ʶR�F��*/
{
    char buf[256];
    clrchyiuan(6, b_lines - 6);
    clrchyiuan(b_lines - 1, b_lines);
    sprintf(buf, BBSHOME"/game/pipgame/buy/pic%d", i);
    show_file(buf, 6, 12, ONLY_COLOR);
}

static void
show_usual_pic(int i)  /* ���`���A */
{
    char buf[256];
    clrchyiuan(6, b_lines - 6);
    clrchyiuan(b_lines - 1, b_lines);
    sprintf(buf, BBSHOME"/game/pipgame/usual/pic%d", i);
    show_file(buf, 6, 12, ONLY_COLOR);

}

static void
show_special_pic(int i)  /* �S���� */
{
    char buf[256];
    clrchyiuan(6, b_lines - 6);
    clrchyiuan(b_lines - 1, b_lines);
    sprintf(buf, BBSHOME"/game/pipgame/special/pic%d", i);
    show_file(buf, 6, 12, ONLY_COLOR);

}

static void
show_practice_pic(int i)  /*�צ�Ϊ��� */
{
    char buf[256];
    clrchyiuan(6, b_lines - 6);
    clrchyiuan(b_lines - 1, b_lines);
    sprintf(buf, BBSHOME"/game/pipgame/practice/pic%d", i);
    show_file(buf, 6, 12, ONLY_COLOR);
}

static void
show_job_pic(int i)    /* ���u��show�� */
{
    char buf[256];
    clrchyiuan(6, b_lines - 6);
    clrchyiuan(b_lines - 1, b_lines);
    sprintf(buf, BBSHOME"/game/pipgame/job/pic%d", i);
    show_file(buf, 6, 12, ONLY_COLOR);

}


static void
show_play_pic(int i)  /*�𶢪���*/
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
show_guess_pic(int i)  /* �q���� */
{
    char buf[256];
    clrchyiuan(6, b_lines - 6);
    clrchyiuan(b_lines - 1, b_lines);
    sprintf(buf, BBSHOME"/game/pipgame/guess/pic%d", i);
    show_file(buf, 6, 12, ONLY_COLOR);
}

static void
show_weapon_pic(int i)  /* �Z���� */
{
    char buf[256];
    clrchyiuan(1, 10);
    clrchyiuan(b_lines - 1, b_lines);
    sprintf(buf, BBSHOME"/game/pipgame/weapon/pic%d", i);
    show_file(buf, 1, 10, ONLY_COLOR);
}

static void
show_palace_pic(int i)  /* �Ѩ����ڥ� */
{
    char buf[256];
    clrchyiuan(0, 13);
    clrchyiuan(b_lines - 1, b_lines);
    sprintf(buf, BBSHOME"/game/pipgame/palace/pic%d", i);
    show_file(buf, 0, 11, ONLY_COLOR);

}

static void
show_badman_pic(int i)  /* �a�H */
{
    char buf[256];
    clrchyiuan(6, b_lines - 6);
    clrchyiuan(b_lines - 1, b_lines);
    sprintf(buf, BBSHOME"/game/pipgame/badman/pic%d", i);
    show_file(buf, 6, 14, ONLY_COLOR);
}

static void
show_fight_pic(int i)  /* ���[ */
{
    char buf[256];
    clrchyiuan(6, b_lines - 6);
    clrchyiuan(b_lines - 1, b_lines);
    sprintf(buf, BBSHOME"/game/pipgame/fight/pic%d", i);
    show_file(buf, 6, 14, ONLY_COLOR);
}

static void
show_die_pic(int i)  /*���`*/
{
    char buf[256];
    clrchyiuan(0, b_lines);
    sprintf(buf, BBSHOME"/game/pipgame/die/pic%d", i);
    show_file(buf, 0, b_lines, ONLY_COLOR);
}

static void
show_system_pic(int i)  /*�t��*/
{
    char buf[256];
    clrchyiuan(1, b_lines);
    sprintf(buf, BBSHOME"/game/pipgame/system/pic%d", i);
    show_file(buf, 4, 16, ONLY_COLOR);
}

static void
show_ending_pic(int i)  /*����*/
{
    char buf[256];
    clrchyiuan(1, b_lines);
    sprintf(buf, BBSHOME"/game/pipgame/ending/pic%d", i);
    show_file(buf, 4, 16, ONLY_COLOR);
}

static void
show_resultshow_pic(int i)      /*��ì�u*/
{
    char buf[256];
    clrchyiuan(0, b_lines);
    sprintf(buf, BBSHOME"/game/pipgame/resultshow/pic%d", i);
    show_file(buf, 0, b_lines, ONLY_COLOR);
}

/*---------------------------------------------------------------------------*/
/* �ө����:���� �s�� �j�ɤY ���� �ѥ�                                       */
/* �禡�w                                                                    */
/*---------------------------------------------------------------------------*/

static int pip_store_food(void) { return pip_buy_goods_new(SHOP_FOOD, d.eat); }

static int pip_store_medicine(void) { return pip_buy_goods_new(SHOP_MEDICINE, d.eat); }

static int pip_store_other(void) { return pip_buy_goods_new(SHOP_OTHER, d.thing); }

/*�Y���Z��*/
static int pip_store_weapon_head(void) { return pip_weapon_doing_menu(WEAPON_HEAD, d.weapon); }
/*�k��Z��*/
static int pip_store_weapon_rhand(void) { return pip_weapon_doing_menu(WEAPON_RHAND, d.weapon); }
/*����Z��*/
static int pip_store_weapon_lhand(void) { return pip_weapon_doing_menu(WEAPON_LHAND, d.weapon); }
/*����Z��*/
static int pip_store_weapon_body(void) { return pip_weapon_doing_menu(WEAPON_BODY, d.weapon); }
/*�����Z��*/
static int pip_store_weapon_foot(void) { return pip_weapon_doing_menu(WEAPON_FOOT, d.weapon); }


static int
pip_buy_goods_new(
enum pipshopidx mode,
int item_list[])
{
    char inbuf[256];
    char genbuf[20];
    long smoney;
    int oldmoney;
    int i, pipkey, choice;
    int numlen;

    const struct pipshop *const shop = &pip_shop_list[mode];

    oldmoney = d.thing[THING_MONEY];
    do
    {
        clrchyiuan(6, b_lines - 6);
        move(6, 0);
        prints_centered("\x1b[1;31m  �w\x1b[41;37m �s�� \x1b[0;1;31m�w\x1b[41;37m ��      �~ \x1b[0;1;31m�w�w\x1b[41;37m ��            �� \x1b[0;1;31m�w�w\x1b[41;37m ��     �� \x1b[0;1;31m�w\x1b[37;41m �֦��ƶq \x1b[0;1;31m�w\x1b[0m  ");
        for (i = 0; shop->list[i].name; i++)
        {
            const struct goodsofpip *const pi = &shop->list[i];

            move(7 + i, 0);
            prints_centered("     \x1b[1;35m[\x1b[37m%2d\x1b[35m]     \x1b[36m%-10s      \x1b[37m%-14s        \x1b[1;33m%-10d   \x1b[1;32m%-9d    \x1b[0m",
                    i+1, pi->name, pi->msgbuy, pi->money, item_list[pi->id]);
        }
        numlen = i;
        clrchyiuan(b_lines - 4, b_lines);
        move(b_lines, 0);
        prints("\x1b[1;44;37m  %8s���  \x1b[46m  [B]�R�J���~  [S]��X���~  [Q]���X      %*s\x1b[m", shop->name, 30 + d_cols - (int)(unsigned int)strlen(shop->name), "");
        pipkey = vkey();
        switch (pipkey)
        {
        case 'B':
        case 'b':
            move(b_lines - 1, 1);
            sprintf(inbuf, "�Q�n�R�Jԣ�O? [0]���R�J [1��%d]���~�Ӹ�: ", numlen);
            getdata(B_LINES_REF - 1, 1, inbuf, genbuf, 3, LCECHO, "0");
            choice = atoi(genbuf)-1;
            if (choice >= 0 && choice < numlen)
            {
                const struct goodsofpip *const pchoice = &shop->list[choice];

                clrchyiuan(6, b_lines - 6);
                if (random() % 2 > 0)
                    show_buy_pic(pchoice->pic1);
                else
                    show_buy_pic(pchoice->pic2);
                move(b_lines - 1, 0);
                clrtoeol();
                move(b_lines - 1, 1);
                smoney = 0;
                if (mode == SHOP_OTHER)
                    smoney = 1;
                else
                {
                    sprintf(inbuf, "�A�n�R�J���~ [%s] �h�֭өO?(�W�� %d): ", pchoice->name, d.thing[THING_MONEY] / pchoice->money);
                    getdata(B_LINES_REF - 1, 1, inbuf, genbuf, 6, DOECHO, 0);
                    smoney = atoi(genbuf);
                }
                if (smoney < 0)
                {
                    vmsg("���R�J...");
                }
                else if (d.thing[THING_MONEY] < smoney*pchoice->money)
                {
                    vmsg("�A�����S������h��..");
                }
                else
                {
                    sprintf(inbuf, "�T�w�R�J���~ [%s] �ƶq %ld �Ӷ�?(���a��� %ld) [y/N]: ", pchoice->name, smoney, smoney*pchoice->money);
                    getdata(B_LINES_REF - 1, 1, inbuf, genbuf, 2, DOECHO, 0);
                    if (genbuf[0] == 'y' || genbuf[0] == 'Y')
                    {
                        item_list[pchoice->id] += smoney;
                        d.thing[THING_MONEY] -= smoney * pchoice->money;
                        sprintf(inbuf, "���󵹤F�A%ld��%s", smoney, pchoice->name);
                        vmsg(inbuf);
                        vmsg(pchoice->msguse);
                        if (mode == SHOP_OTHER && choice == 0)
                        {
                            d.state[STATE_HAPPY] += random() % 10 + 20 * smoney;
                            d.state[STATE_SATISFY] += random() % 10 + 20 * smoney;
                        }
                        else if (mode == SHOP_OTHER && choice == 1)
                        {
                            d.state[STATE_HAPPY] += (random() % 2 + 2) * smoney;
                            d.learn[LEARN_WISDOM] += (2 + 10 / (d.learn[LEARN_WISDOM] / 100 + 1)) * smoney;
                            d.learn[LEARN_CHARACTER] += (random() % 4 + 2) * smoney;
                            d.learn[LEARN_ART] += (random() % 2 + 1) * smoney;
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
            if (mode == SHOP_OTHER)
            {
                vmsg("�o�ǪF�褣����....");
                break;
            }
            move(b_lines - 1, 1);
            sprintf(inbuf, "�Q�n��Xԣ�O? [0]����X [1��%d]���~�Ӹ�: ", numlen);
            getdata(B_LINES_REF - 1, 1, inbuf, genbuf, 3, LCECHO, "0");
            choice = atoi(genbuf)-1;
            if (choice >= 0 && choice < numlen)
            {
                const struct goodsofpip *const pchoice = &shop->list[choice];

                clrchyiuan(6, b_lines - 6);
                if (random() % 2 > 0)
                    show_buy_pic(pchoice->pic1);
                else
                    show_buy_pic(pchoice->pic2);
                move(b_lines - 1, 0);
                clrtoeol();
                move(b_lines - 1, 1);
                smoney = 0;
                sprintf(inbuf, "�A�n��X���~ [%s] �h�֭өO?(�W�� %d): ", pchoice->name, item_list[pchoice->id]);
                getdata(B_LINES_REF - 1, 1, inbuf, genbuf, 6,, 0);
                smoney = atoi(genbuf);
                if (smoney < 0)
                {
                    vmsg("����X...");
                }
                else if (smoney > item_list[pchoice->id])
                {
                    sprintf(inbuf, "�A�� [%s] �S������h�ӳ�", pchoice->name);
                    vmsg(inbuf);
                }
                else
                {
                    sprintf(inbuf, "�T�w��X���~ [%s] �ƶq %ld �Ӷ�?(���a�R�� %ld) [y/N]: ", pchoice->name, smoney, smoney*pchoice->money*8 / 10);
                    getdata(B_LINES_REF - 1, 1, inbuf, genbuf, 2, DOECHO, 0);
                    if (genbuf[0] == 'y' || genbuf[0] == 'Y')
                    {
                        item_list[pchoice->id] -= smoney;
                        d.thing[THING_MONEY] += smoney * pchoice->money * 8 / 10;
                        sprintf(inbuf, "���󮳨��F�A��%ld��%s", smoney, pchoice->name);
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
            sprintf(inbuf, "��������@ %d ���A���} %s ", oldmoney - d.thing[THING_MONEY], shop->name);
            vmsg(inbuf);
            break;
        }
    }
    while ((pipkey != 'Q') && (pipkey != 'q') && (pipkey != KEY_LEFT));
    return 0;
}

static int
pip_weapon_doing_menu(             /* �Z���ʶR�e�� */
enum pipweapon type,
int item_list[WEAPON_COUNT])
{
    time_t now;
    int n = 0;
    const char *s;
    char ans[5];
    char shortbuf[100];
    int pipkey;
    char choicekey[5];
    int choice;

    int variance = item_list[type];
    const struct weaponlist *const weapons = &pip_weapon_list[type];

    do
    {
        const struct weapon *const pvariance = &weapons->list[variance];

        clear();
        vs_head(weapons->menutitle, BoardName);
        show_weapon_pic(0);
   /*   move(10, 2);
        prints_centered("\x1b[1;37m�{����O:��OMax:\x1b[36m%-5d\x1b[37m  �k�OMax:\x1b[36m%-5d\x1b[37m  ����:\x1b[36m%-5d\x1b[37m  ���m:\x1b[36m%-5d\x1b[37m  �t��:\x1b[36m%-5d \x1b[m",
                d.body[BODY_MAXHP], d.fight[FIGHT_MAXMP], d.fight[FIGHT_ATTACK], d.fight[FIGHT_RESIST], d.fight[FIGHT_SPEED]);
   */
        move(11, 2);
        prints_centered("\x1b[1;37;41m [NO]  [����W]  [��O]  [�k�O]  [�t��]  [����]  [���m]  [�t��]  [��  ��] \x1b[m");
        move(12, 2);
        prints_centered(" \x1b[1;31m�w�w\x1b[37m�զ� �i�H�ʶR\x1b[31m�w�w\x1b[32m��� �֦��˳�\x1b[31m�w�w\x1b[33m���� ��������\x1b[31m�w�w\x1b[35m���� ��O����\x1b[31m�w�w\x1b[m");

        n = 1;
        while ((s = weapons->list[n].name))
        {
            const struct weapon *const pn = &weapons->list[n];

            move(12 + n, 2);
            if (variance == (n))/*��������*/
            {
                prints_centered("\x1b[1;32m (%2d)  %-10s %4d    %4d    %4d    %4d    %4d    %4d    %6d\x1b[m",
                        n, pn->name, pn->needmaxhp, pn->needmaxmp, pn->needspeed,
                        pn->attack, pn->resist, pn->speed, pn->cost);
            }
            else if (d.body[BODY_MAXHP] < pn->needmaxhp || d.fight[FIGHT_MAXMP] < pn->needmaxmp || d.fight[FIGHT_SPEED] < pn->needspeed)/*��O����*/
            {
                prints_centered("\x1b[1;35m (%2d)  %-10s %4d    %4d    %4d    %4d    %4d    %4d    %6d\x1b[m",
                        n, pn->name, pn->needmaxhp, pn->needmaxmp, pn->needspeed,
                        pn->attack, pn->resist, pn->speed, pn->cost);
            }

            else if (d.thing[THING_MONEY] < pn->cost) /*��������*/
            {
                prints_centered("\x1b[1;33m (%2d)  %-10s %4d    %4d    %4d    %4d    %4d    %4d    %6d\x1b[m",
                        n, pn->name, pn->needmaxhp, pn->needmaxmp, pn->needspeed,
                        pn->attack, pn->resist, pn->speed, pn->cost);
            }
            else
            {
                prints_centered("\x1b[1;37m (%2d)  %-10s %4d    %4d    %4d    %4d    %4d    %4d    %6d\x1b[m",
                        n, pn->name, pn->needmaxhp, pn->needmaxmp, pn->needspeed,
                        pn->attack, pn->resist, pn->speed, pn->cost);
            }
            n++;
        }
        move(b_lines, 0);
        prints("\x1b[1;44;37m  �Z���ʶR���  \x1b[46m  [B]�ʶR�Z��  [S]�汼�˳�  [W]�ӤH���  [Q]���X               %*s\x1b[m", d_cols, "");
        now = time(0);
        pip_time_change(now);
        pipkey = vkey();
        pip_time_change(now);

        switch (pipkey)
        {
        case 'B':
        case 'b':
            move(b_lines - 1, 1);
            sprintf(shortbuf, "�Q�n�ʶRԣ�O? �A������[%d]��:[�Ʀr]: ", d.thing[THING_MONEY]);
            getdata(B_LINES_REF - 1, 1, shortbuf, choicekey, 4, LCECHO, "0");
            choice = atoi(choicekey);
            if (choice >= 0 && choice <= n)
            {
                const struct weapon *const pchoice = &weapons->list[choice];

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
                    sprintf(shortbuf, "�A���w�g�� %s �o", pvariance->name);
                    vmsg(shortbuf);
                }

                else if (pchoice->cost >= (d.thing[THING_MONEY] + pvariance->sell))  /*������*/
                {
                    sprintf(shortbuf, "�o�ӭn %d ���A�A����������!", pchoice->cost);
                    vmsg(shortbuf);
                }

                else if (d.body[BODY_MAXHP] < pchoice->needmaxhp || d.fight[FIGHT_MAXMP] < pchoice->needmaxmp
                          || d.fight[FIGHT_SPEED] < pchoice->needspeed)  /*��O����*/
                {
                    sprintf(shortbuf, "�ݭnHP %d MP %d SPEED %d ��",
                            pchoice->needmaxhp, pchoice->needmaxmp, pchoice->needspeed);
                    vmsg(shortbuf);
                }
                else  /*���Q�ʶR*/
                {
                    sprintf(shortbuf, "�A�T�w�n�ʶR %s ��?($%d) [y/N]: ", pchoice->name, pchoice->cost);
                    getdata(B_LINES_REF - 1, 1, shortbuf, ans, 2, DOECHO, 0);
                    if (ans[0] == 'y' || ans[0] == 'Y')
                    {
                        sprintf(shortbuf, "�p���w�g�˳ƤW %s �F", pchoice->name);
                        vmsg(shortbuf);
                        d.fight[FIGHT_ATTACK] += (pchoice->attack - pvariance->attack);
                        d.fight[FIGHT_RESIST] += (pchoice->resist - pvariance->resist);
                        d.fight[FIGHT_SPEED] += (pchoice->speed - pvariance->speed);
                        d.thing[THING_MONEY] -= (pchoice->cost - pvariance->sell);
                        item_list[type] = variance = choice;
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
                sprintf(shortbuf, "�A�T�w�n�汼%s��? ���:%d [y/N]: ", pvariance->name, pvariance->sell);
                getdata(B_LINES_REF - 1, 1, shortbuf, ans, 2, DOECHO, 0);
                if (ans[0] == 'y' || ans[0] == 'Y')
                {
                    sprintf(shortbuf, "�˳� %s ��F %d", pvariance->name, pvariance->sell);
                    d.fight[FIGHT_ATTACK] -= pvariance->attack;
                    d.fight[FIGHT_RESIST] -= pvariance->resist;
                    d.fight[FIGHT_SPEED] -= pvariance->speed;
                    d.thing[THING_MONEY] += pvariance->sell;
                    vmsg(shortbuf);
                    item_list[type] = variance = 0;
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
            }
            break;

        case 'W':
        case 'w':
            pip_data_list(cuser.userid);
            break;
        }
    }
    while ((pipkey != 'Q') && (pipkey != 'q') && (pipkey != KEY_LEFT));

    return 0;
}

/*---------------------------------------------------------------------------*/
/* ���u���:�a�� �W�u �a�� �a�u                                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static int pip_job_workA(void)
{
    /*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    /*  �x�a�x�޲z�x�ݤH���� + N, ���a�~�� + N, �i���ޥ� + N    �x*/
    /*  �x        �x�M���˪����Y + N, �h�� + 1, �P�� - 2        �x*/
    /*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    /*  �x�a�x�޲z�x�Y ��    �O - RND (�h��) >=   5 �h�u�@���\  �x*/
    /*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    float class_;
    long workmoney;

    workmoney = 0;
    class_ = ((d.body[BODY_HP] * 100 / d.body[BODY_MAXHP]) - d.body[BODY_TIRED]) * LEARN_LEVEL;
    d.body[BODY_MAXHP] += random() % 2 * LEARN_LEVEL;
    d.body[BODY_SHIT] += random() % 3 + 5;
    count_tired(3, 7, true, 100, 1);
    d.body[BODY_HP] -= (random() % 2 + 4);
    d.state[STATE_HAPPY] -= (random() % 3 + 4);
    d.state[STATE_SATISFY] -= random() % 3 + 4;
    d.state[STATE_AFFECT] -= 7 + random() % 7;
    if (d.state[STATE_AFFECT] <= 0)
        d.state[STATE_AFFECT] = 0;
    show_job_pic(11);
    if (class_ >= 75)
    {
        d.learn[LEARN_COOKSKILL] += random() % 2 + 7;
        d.learn[LEARN_HOMEWORK] += random() % 2 + 7;
        d.tmp[TMP_FAMILY] += random() % 3 + 4;
        d.relation += random() % 3 + 4;
        workmoney = 80 + (d.learn[LEARN_COOKSKILL] * 2 + d.learn[LEARN_HOMEWORK] + d.tmp[TMP_FAMILY]) / 40;
        vmsg("�a�ƫܦ��\\��..�h�@�I�����A..");
    }
    else if (class_ >= 50)
    {
        d.learn[LEARN_COOKSKILL] += random() % 2 + 5;
        d.learn[LEARN_HOMEWORK] += random() % 2 + 5;
        d.tmp[TMP_FAMILY] += random() % 3 + 3;
        d.relation += random() % 3 + 3;
        workmoney = 60 + (d.learn[LEARN_COOKSKILL] * 2 + d.learn[LEARN_HOMEWORK] + d.tmp[TMP_FAMILY]) / 45;
        vmsg("�a�����Z���Q����..���..");
    }
    else if (class_ >= 25)
    {
        d.learn[LEARN_COOKSKILL] += random() % 3 + 3;
        d.learn[LEARN_HOMEWORK] += random() % 3 + 3;
        d.tmp[TMP_FAMILY] += random() % 3 + 2;
        d.relation += random() % 3 + 2;
        workmoney = 40 + (d.learn[LEARN_COOKSKILL] * 2 + d.learn[LEARN_HOMEWORK] + d.tmp[TMP_FAMILY]) / 50;
        vmsg("�a�ƴ����q�q��..�i�H��n��..�[�o..");
    }
    else
    {
        d.learn[LEARN_COOKSKILL] += random() % 3 + 1;
        d.learn[LEARN_HOMEWORK] += random() % 3 + 1;
        d.tmp[TMP_FAMILY] += random() % 3 + 1;
        d.relation += random() % 3 + 1;
        workmoney = 20 + (d.learn[LEARN_COOKSKILL] * 2 + d.learn[LEARN_HOMEWORK] + d.tmp[TMP_FAMILY]) / 60;
        vmsg("�a�ƫ��V�|��..�o�ˤ����..");
    }
    d.thing[THING_MONEY] += workmoney * LEARN_LEVEL;
    d.work[A] += 1;
    return 0;
}

static int pip_job_workB(void)
{
    /*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    /*  �x�|���|  �x���� + N, �P�� + 1, �y�O - 1, �h�� + 3      �x*/
    /*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    /*  �x�|���|  �x�Y ��    �O - RND (�h��) >=  20 �h�u�@���\  �x*/
    /*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    float class_;
    long workmoney;

    workmoney = 0;
    class_ = ((d.body[BODY_HP] * 100 / d.body[BODY_MAXHP]) - d.body[BODY_TIRED]) * LEARN_LEVEL;
    d.body[BODY_MAXHP] += (random() % 2 + 1) * LEARN_LEVEL;
    d.body[BODY_SHIT] += random() % 3 + 5;
    d.state[STATE_AFFECT] += random() % 3 + 4;

    count_tired(3, 9, true, 100, 1);
    d.body[BODY_HP] -= (random() % 3 + 6);
    d.state[STATE_HAPPY] -= (random() % 3 + 4);
    d.state[STATE_SATISFY] -= random() % 3 + 4;
    d.learn[LEARN_CHARM] -= random() % 3 + 4;
    if (d.learn[LEARN_CHARM] <= 0)
        d.learn[LEARN_CHARM] = 0;
    show_job_pic(21);
    if (class_ >= 90)
    {
        d.learn[LEARN_LOVE] += random() % 2 + 7;
        d.learn[LEARN_TOMAN] += random() % 2 + 2;
        workmoney = 150 + (d.learn[LEARN_LOVE] + d.learn[LEARN_TOMAN]) / 50;
        vmsg("��O�i�ܦ��\\��..�U���A�ӳ�..");
    }
    else if (class_ >= 75)
    {
        d.learn[LEARN_LOVE] += random() % 2 + 5;
        d.learn[LEARN_TOMAN] += random() % 2 + 2;
        workmoney = 120 + (d.learn[LEARN_LOVE] + d.learn[LEARN_TOMAN]) / 50;
        vmsg("�O�i�ٷ�������..���..");
    }
    else if (class_ >= 50)
    {
        d.learn[LEARN_LOVE] += random() % 2 + 3;
        d.learn[LEARN_TOMAN] += random() % 2 + 1;
        workmoney = 100 + (d.learn[LEARN_LOVE] + d.learn[LEARN_TOMAN]) / 50;
        vmsg("�p�B�ͫܥֳ�..�[�o..");
    }
    else
    {
        d.learn[LEARN_LOVE] += random() % 2 + 1;
        d.learn[LEARN_TOMAN] += random() % 2 + 1;
        workmoney = 80 + (d.learn[LEARN_LOVE] + d.learn[LEARN_TOMAN]) / 50;
        vmsg("���V�|��..�A�n����p�B�ͭC...");
    }
    d.thing[THING_MONEY] += workmoney * LEARN_LEVEL;
    d.work[B] += 1;
    return 0;
}

static int pip_job_workC(void)
{
    /*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    /*  �x���]    �x���a�~�� + N, �԰��޳N - N, �h�� + 2        �x*/
    /*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    /*  �x���]    �x�Y ��    �O - RND (�h��) >=  30 �h�u�@���\  �x*/
    /*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    float class_;
    long workmoney;

    workmoney = 0;
    class_ = ((d.body[BODY_HP] * 100 / d.body[BODY_MAXHP]) - d.body[BODY_TIRED]) * LEARN_LEVEL;
    d.body[BODY_MAXHP] += (random() % 2 + 2) * LEARN_LEVEL;
    d.body[BODY_SHIT] += random() % 3 + 5;
    count_tired(5, 12, true, 100, 1);
    d.body[BODY_HP] -= (random() % 4 + 8);
    d.state[STATE_HAPPY] -= (random() % 3 + 4);
    d.state[STATE_SATISFY] -= random() % 3 + 4;
    show_job_pic(31);
    if (class_ >= 95)
    {
        d.learn[LEARN_HOMEWORK] += random() % 2 + 7;
        d.tmp[TMP_FAMILY] += random() % 2 + 4;
        d.fight[FIGHT_HSKILL] -= random() % 2 + 7;
        if (d.fight[FIGHT_HSKILL] < 0)
            d.fight[FIGHT_HSKILL] = 0;
        workmoney = 250 + (d.learn[LEARN_COOKSKILL] * 2 + d.learn[LEARN_HOMEWORK] * 2) / 40;
        vmsg("���]�Ʒ~�]�]��W..�Ʊ�A�A�L��...");
    }
    else if (class_ >= 80)
    {
        d.learn[LEARN_HOMEWORK] += random() % 2 + 5;
        d.tmp[TMP_FAMILY] += random() % 2 + 3;
        d.fight[FIGHT_HSKILL] -= random() % 2 + 5;
        if (d.fight[FIGHT_HSKILL] < 0)
            d.fight[FIGHT_HSKILL] = 0;
        workmoney = 200 + (d.learn[LEARN_COOKSKILL] * 2 + d.learn[LEARN_HOMEWORK] * 2) / 50;
        vmsg("���]���Z���Q����..���..");
    }
    else if (class_ >= 60)
    {
        d.learn[LEARN_HOMEWORK] += random() % 2 + 3;
        d.tmp[TMP_FAMILY] += random() % 2 + 3;
        d.fight[FIGHT_HSKILL] -= random() % 2 + 5;
        if (d.fight[FIGHT_HSKILL] < 0)
            d.fight[FIGHT_HSKILL] = 0;
        workmoney = 150 + (d.learn[LEARN_COOKSKILL] * 2 + d.learn[LEARN_HOMEWORK] * 2) / 50;
        vmsg("�����q�q��..�i�H��n��..�[�o..");
    }
    else
    {
        d.learn[LEARN_HOMEWORK] += random() % 2 + 1;
        d.tmp[TMP_FAMILY] += random() % 2 + 1;
        d.fight[FIGHT_HSKILL] -= random() % 2 + 1;
        if (d.fight[FIGHT_HSKILL] < 0)
            d.fight[FIGHT_HSKILL] = 0;
        workmoney = 100 + (d.learn[LEARN_COOKSKILL] * 2 + d.learn[LEARN_HOMEWORK] * 2) / 50;
        vmsg("�o�ӫ��V�|��..�A�o�ˤ����..");
    }
    d.thing[THING_MONEY] += workmoney * LEARN_LEVEL;
    d.work[C] += 1;
    return 0;
}

static int pip_job_workD(void)
{
    /*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    /*  �x�A��    �x��O + 1, �äO + 1, ��� - 1, �h�� + 3      �x*/
    /*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    /*  �x�A��    �x�Y ��    �O - RND (�h��) >=  30 �h�u�@���\  �x*/
    /*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    float class_;
    long workmoney;

    workmoney = 0;
    class_ = ((d.body[BODY_HP] * 100 / d.body[BODY_MAXHP]) - d.body[BODY_TIRED]) * LEARN_LEVEL;
    d.body[BODY_MAXHP] += (random() % 3 + 2) * LEARN_LEVEL;
    d.body[BODY_WRIST] += random() % 2 + 2;
    d.body[BODY_SHIT] += random() % 5 + 10;
    count_tired(5, 15, true, 100, 1);
    d.body[BODY_HP] -= (random() % 4 + 10);
    d.state[STATE_HAPPY] -= (random() % 3 + 4);
    d.state[STATE_SATISFY] -= random() % 3 + 4;
    d.learn[LEARN_CHARACTER] -= random() % 3 + 4;
    if (d.learn[LEARN_CHARACTER] < 0)
        d.learn[LEARN_CHARACTER] = 0;
    show_job_pic(41);
    if (class_ >= 95)
    {
        workmoney = 250 + (d.body[BODY_WRIST] * 2 + d.body[BODY_HP] * 2) / 80;
        vmsg("���Ϫ����n�n��..�Ʊ�A�A������...");
    }
    else if (class_ >= 80)
    {
        workmoney = 210 + (d.body[BODY_WRIST] * 2 + d.body[BODY_HP] * 2) / 80;
        vmsg("����..�٤�����..:)");
    }
    else if (class_ >= 60)
    {
        workmoney = 160 + (d.body[BODY_WRIST] * 2 + d.body[BODY_HP] * 2) / 80;
        vmsg("�����q�q��..�i�H��n��..");
    }
    else
    {
        workmoney = 120 + (d.body[BODY_WRIST] * 2 + d.body[BODY_HP] * 2) / 80;
        vmsg("�A���ӾA�X�A�����u�@  -_-...");
    }
    d.thing[THING_MONEY] += workmoney * LEARN_LEVEL;
    d.work[D] += 1;
    return 0;
}

static int pip_job_workE(void)
{
    /*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    /*  �x�\�U    �x�Ʋz + N, �԰��޳N - N, �h�� + 2            �x*/
    /*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    /*  �x�\�U    �x�Y �i���޳N - RND (�h��) >=  50 �h�u�@���\  �x*/
    /*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    float class_;
    long workmoney;

    workmoney = 0;
    class_ = (d.learn[LEARN_COOKSKILL] - d.body[BODY_TIRED]) * LEARN_LEVEL;
    d.body[BODY_MAXHP] += (random() % 2 + 1) * LEARN_LEVEL;
    d.body[BODY_SHIT] += random() % 4 + 12;
    count_tired(5, 9, true, 100, 1);
    d.body[BODY_HP] -= (random() % 4 + 8);
    d.state[STATE_HAPPY] -= (random() % 3 + 4);
    d.state[STATE_SATISFY] -= random() % 3 + 4;
    show_job_pic(51);
    if (class_ >= 80)
    {
        d.learn[LEARN_HOMEWORK] += random() % 2 + 1;
        d.tmp[TMP_FAMILY] += random() % 2 + 1;
        d.fight[FIGHT_HSKILL] -= random() % 2 + 5;
        if (d.fight[FIGHT_HSKILL] < 0)
            d.fight[FIGHT_HSKILL] = 0;
        d.learn[LEARN_COOKSKILL] += random() % 2 + 6;
        workmoney = 250 + (d.learn[LEARN_COOKSKILL] * 2 + d.learn[LEARN_HOMEWORK] * 2 + d.tmp[TMP_FAMILY] * 2) / 80;
        vmsg("�ȤH�����Ӧn�Y�F..�A�Ӥ@�L�a...");
    }
    else if (class_ >= 60)
    {
        d.learn[LEARN_HOMEWORK] += random() % 2 + 1;
        d.tmp[TMP_FAMILY] += random() % 2 + 1;
        d.fight[FIGHT_HSKILL] -= random() % 2 + 5;
        if (d.fight[FIGHT_HSKILL] < 0)
            d.fight[FIGHT_HSKILL] = 0;
        d.learn[LEARN_COOKSKILL] += random() % 2 + 4;
        workmoney = 200 + (d.learn[LEARN_COOKSKILL] * 2 + d.learn[LEARN_HOMEWORK] * 2 + d.tmp[TMP_FAMILY] * 2) / 80;
        vmsg("�N���٤����Y��..:)");
    }
    else if (class_ >= 30)
    {
        d.learn[LEARN_HOMEWORK] += random() % 2 + 1;
        d.tmp[TMP_FAMILY] += random() % 2 + 1;
        d.fight[FIGHT_HSKILL] -= random() % 2 + 5;
        if (d.fight[FIGHT_HSKILL] < 0)
            d.fight[FIGHT_HSKILL] = 0;
        d.learn[LEARN_COOKSKILL] += random() % 2 + 2;
        workmoney = 150 + (d.learn[LEARN_COOKSKILL] * 2 + d.learn[LEARN_HOMEWORK] * 2 + d.tmp[TMP_FAMILY] * 2) / 80;
        vmsg("�����q�q��..�i�H��n��..");
    }
    else
    {
        d.learn[LEARN_HOMEWORK] += random() % 2 + 1;
        d.tmp[TMP_FAMILY] += random() % 2 + 1;
        d.fight[FIGHT_HSKILL] -= random() % 2 + 5;
        if (d.fight[FIGHT_HSKILL] < 0)
            d.fight[FIGHT_HSKILL] = 0;
        d.learn[LEARN_COOKSKILL] += random() % 2 + 1;
        workmoney = 100 + (d.learn[LEARN_COOKSKILL] * 2 + d.learn[LEARN_HOMEWORK] * 2 + d.tmp[TMP_FAMILY] * 2) / 80;
        vmsg("�A���p���ݥ[�j��...");
    }
    d.thing[THING_MONEY] += workmoney * LEARN_LEVEL;
    d.work[E] += 1;
    return 0;
}

static int pip_job_workF(void)
{
    /*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    /*  �x�а�    �x�H�� + 2, �D�w + 1, �o�^ - 2, �h�� + 1      �x*/
    /*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    float class_;
    long workmoney;

    workmoney = 0;
    class_ = ((d.body[BODY_HP] * 100 / d.body[BODY_MAXHP]) - d.body[BODY_TIRED]) * LEARN_LEVEL;
    count_tired(5, 7, true, 100, 1);
    d.learn[LEARN_LOVE] += (random() % 3 + 4) * LEARN_LEVEL;
    d.state[STATE_BELIEF] += (random() % 4 + 7) * LEARN_LEVEL;
    d.learn[LEARN_ETHICS] += (random() % 3 + 7) * LEARN_LEVEL;
    d.body[BODY_SHIT] += random() % 3 + 3;
    d.body[BODY_HP] -= random() % 3 + 5;
    d.state[STATE_OFFENSE] -= random() % 4 + 7;
    if (d.state[STATE_OFFENSE] < 0)
        d.state[STATE_OFFENSE] = 0;
    show_job_pic(61);
    if (class_ >= 75)
    {
        workmoney = 100 + (d.state[STATE_BELIEF] + d.learn[LEARN_ETHICS] - d.state[STATE_OFFENSE]) / 20;
        vmsg("���ܤ� ���ݧA�o��{�u ���A�h�@�I...");
    }
    else if (class_ >= 50)
    {
        workmoney = 75 + (d.state[STATE_BELIEF] + d.learn[LEARN_ETHICS] - d.state[STATE_OFFENSE]) / 20;
        vmsg("���§A����������..:)");
    }
    else if (class_ >= 25)
    {
        workmoney = 50 + (d.state[STATE_BELIEF] + d.learn[LEARN_ETHICS] - d.state[STATE_OFFENSE]) / 20;
        vmsg("�A�u���ܦ��R�߰�..���L���I�p�֪��ˤl...");
    }
    else
    {
        workmoney = 25 + (d.state[STATE_BELIEF] + d.learn[LEARN_ETHICS] - d.state[STATE_OFFENSE]) / 20;
        vmsg("�ө^�m����..���]���ॴ�V��....:(");
    }
    d.thing[THING_MONEY] += workmoney * LEARN_LEVEL;
    d.work[F] += 1;
    return 0;
}

static int pip_job_workG(void)
{
    /* �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    /* �x�a�u    �x��O + 2, �y�O + 1, �h�� + 3, �ͦR +1       �x*/
    /* �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    long workmoney;

    workmoney = 0;
    workmoney = 200 + (d.learn[LEARN_CHARM] * 3 + d.learn[LEARN_SPEECH] * 2 + d.learn[LEARN_TOMAN]) / 50;
    count_tired(3, 12, true, 100, 1);
    d.body[BODY_SHIT] += random() % 3 + 8;
    d.fight[FIGHT_SPEED] += (random() % 2) * LEARN_LEVEL;
    d.body[BODY_WEIGHT] -= random() % 2;
    d.state[STATE_HAPPY] -= (random() % 3 + 7);
    d.state[STATE_SATISFY] -= random() % 3 + 5;
    d.body[BODY_HP] -= (random() % 6 + 6);
    d.learn[LEARN_CHARM] += (random() % 2 + 3) * LEARN_LEVEL;
    d.learn[LEARN_SPEECH] += (random() % 2 + 3) * LEARN_LEVEL;
    d.learn[LEARN_TOMAN] += (random() % 2 + 3) * LEARN_LEVEL;
    move(4, 0);
    show_job_pic(71);
    vmsg("�\\�a�u�n��ĵ���..:p");
    d.thing[THING_MONEY] += workmoney * LEARN_LEVEL;
    d.work[G] += 1;
    return 0;
}

static int pip_job_workH(void)
{
    /*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    /*  �x����  �x�äO + 2, ��� - 2, �h�� + 4                �x*/
    /*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    /*  �x����  �x�Y ��    �O - RND (�h��) >=  80 �h�u�@���\  �x*/
    /*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    float class_;
    long workmoney;

    if ((d.bbtime / 60 / 30) < 1) /*�@���~��*/
    {
        vmsg("�p���Ӥp�F�A�@���H��A�ӧa...");
        return 0;
    }
    workmoney = 0;
    class_ = (d.body[BODY_WRIST] - d.body[BODY_TIRED]) * LEARN_LEVEL;
    d.body[BODY_MAXHP] += (random() % 2 + 3) * LEARN_LEVEL;
    d.body[BODY_SHIT] += random() % 7 + 15;
    d.body[BODY_WRIST] += (random() % 3 + 4) * LEARN_LEVEL;
    count_tired(5, 15, true, 100, 1);
    d.body[BODY_HP] -= (random() % 4 + 10);
    d.state[STATE_HAPPY] -= (random() % 3 + 4);
    d.state[STATE_SATISFY] -= random() % 3 + 4;
    d.learn[LEARN_CHARACTER] -= random() % 3 + 7;
    if (d.learn[LEARN_CHARACTER] < 0)
        d.learn[LEARN_CHARACTER] = 0;
    show_job_pic(81);
    if (class_ >= 70)
    {
        workmoney = 350 + d.body[BODY_WRIST] / 20 + d.body[BODY_MAXHP] / 80;
        vmsg("�A�äO�ܦn��..:)");
    }
    else if (class_ >= 50)
    {
        workmoney = 300 + d.body[BODY_WRIST] / 20 + d.body[BODY_MAXHP] / 80;
        vmsg("��F���־��.....:)");
    }
    else if (class_ >= 20)
    {
        workmoney = 250 + d.body[BODY_WRIST] / 20 + d.body[BODY_MAXHP] / 80;
        vmsg("�����q�q��..�i�H��n��..");
    }
    else
    {
        workmoney = 200 + d.body[BODY_WRIST] / 20 + d.body[BODY_MAXHP] / 80;
        vmsg("�ݥ[�j��..����A�ӧa....");
    }
    d.thing[THING_MONEY] += workmoney * LEARN_LEVEL;
    d.work[H] += 1;
    return 0;
}

static int pip_job_workI(void)
{
    /*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    /*  �x���e�|  �x�P�� + 1, �äO - 1, �h�� + 3                �x*/
    /*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    /*  �x���e�|  �x�Y ���N�׾i - RND (�h��) >=  40 �h�u�@���\  �x*/
    /*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    float class_;
    long workmoney;

    if ((d.bbtime / 60 / 30) < 1) /*�@���~��*/
    {
        vmsg("�p���Ӥp�F�A�@���H��A�ӧa...");
        return 0;
    }
    workmoney = 0;
    class_ = (d.learn[LEARN_ART] - d.body[BODY_TIRED]) * LEARN_LEVEL;
    d.body[BODY_MAXHP] += (random() % 2) * LEARN_LEVEL;
    d.state[STATE_AFFECT] += (random() % 2 + 3) * LEARN_LEVEL;
    count_tired(3, 11, true, 100, 1);
    d.body[BODY_SHIT] += random() % 4 + 8;
    d.body[BODY_HP] -= (random() % 4 + 10);
    d.state[STATE_HAPPY] -= (random() % 3 + 4);
    d.state[STATE_SATISFY] -= random() % 3 + 4;
    d.body[BODY_WRIST] -= random() % + 3;
    if (d.body[BODY_WRIST] < 0)
        d.body[BODY_WRIST] = 0;
    /*show_job_pic(4);*/
    if (class_ >= 80)
    {
        workmoney = 400 + d.learn[LEARN_ART] / 10 + d.state[STATE_AFFECT] / 20;
        vmsg("�ȤH���ܳ��w���A���y����..:)");
    }
    else if (class_ >= 60)
    {
        workmoney = 360 + d.learn[LEARN_ART] / 10 + d.state[STATE_AFFECT] / 20;
        vmsg("����������..�ᦳ�ѥ�...:)");
    }
    else if (class_ >= 40)
    {
        workmoney = 320 + d.learn[LEARN_ART] / 10 + d.state[STATE_AFFECT] / 20;
        vmsg("��������..�A�[�o�@�I..");
    }
    else
    {
        workmoney = 250 + d.learn[LEARN_ART] / 10 + d.state[STATE_AFFECT] / 20;
        vmsg("�ݥ[�j��..�H��A�ӧa....");
    }
    d.thing[THING_MONEY] += workmoney * LEARN_LEVEL;
    d.work[I] += 1;
    return 0;
}

static int pip_job_workJ(void)
{
    /*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    /*  �x���y��  �x��O + 1, ��� - 1, ���� - 1, �h�� + 3      �x*/
    /*  �x        �x�԰��޳N + N                                �x*/
    /*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    /*  �x���y��  �x�Y ��    �O - RND (�h��) >=  80 ��          �x*/
    /*  �x        �x�Y ��    �O - RND (�h��) >=  40 �h�u�@���\  �x*/
    /*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    float class_;
    float class1;
    long workmoney;

    /*�ⷳ�H�W�~��*/
    if ((d.bbtime / 60 / 30) < 2)
    {
        vmsg("�p���Ӥp�F�A�ⷳ�H��A�ӧa...");
        return 0;
    }
    workmoney = 0;
    class_ = ((d.body[BODY_HP] * 100 / d.body[BODY_MAXHP]) - d.body[BODY_TIRED]) * LEARN_LEVEL;
    class1 = (d.learn[LEARN_WISDOM] - d.body[BODY_TIRED]) * LEARN_LEVEL;
    count_tired(5, 15, true, 100, 1);
    d.body[BODY_SHIT] += random() % 4 + 13;
    d.body[BODY_WEIGHT] -= (random() % 2 + 1);
    d.body[BODY_MAXHP] += (random() % 2 + 3) * LEARN_LEVEL;
    d.fight[FIGHT_SPEED] += (random() % 2 + 3) * LEARN_LEVEL;
    d.body[BODY_HP] -= (random() % 6 + 8);
    d.learn[LEARN_CHARACTER] -= random() % 3 + 4;
    d.state[STATE_HAPPY] -= random() % 5 + 8;
    d.state[STATE_SATISFY] -= random() % 5 + 6;
    d.learn[LEARN_LOVE] -= random() % 3 + 4;
    if (d.learn[LEARN_CHARACTER] < 0)
        d.learn[LEARN_CHARACTER] = 0;
    if (d.learn[LEARN_LOVE] < 0)
        d.learn[LEARN_LOVE] = 0;
    move(4, 0);
    show_job_pic(101);
    if (class_ >= 80 && class1 >= 80)
    {
        d.fight[FIGHT_HSKILL] += random() % 2 + 7;
        workmoney = 300 + d.body[BODY_MAXHP] / 50 + d.fight[FIGHT_HSKILL] / 20;
        vmsg("�A�O�������y�H..");
    }
    else if (class_ >= 50 && class1 >= 60)
    {
        d.fight[FIGHT_HSKILL] += random() % 2 + 5;
        workmoney = 270 + d.body[BODY_MAXHP] / 45 + d.fight[FIGHT_HSKILL] / 20;
        vmsg("�����٤�����..�i�H���\\�@�y�F..:)");
    }
    else if (class_ >= 25 && class1 >= 40)
    {
        d.fight[FIGHT_HSKILL] += random() % 2 + 3;
        workmoney = 240 + d.body[BODY_MAXHP] / 40 + d.fight[FIGHT_HSKILL] / 20;
        vmsg("�޳N�t�j�H�N  �A�[�o��..");
    }
    else if (class_ >= 0 && class1 >= 20)
    {
        d.fight[FIGHT_HSKILL] += random() % 2 + 1;
        workmoney = 210 + d.body[BODY_MAXHP] / 30 + d.fight[FIGHT_HSKILL] / 20;
        vmsg("���y�O��O�P���O�����X....");
    }
    else
    {
        d.fight[FIGHT_HSKILL] += random() % 2;
        workmoney = 190 + d.fight[FIGHT_HSKILL] / 20;
        vmsg("�n�h�h����M�W�i���z��....");
    }
    d.thing[THING_MONEY] += workmoney * LEARN_LEVEL;
    d.work[J] += 1;
    return 0;
}

static int pip_job_workK(void)
{
    /* �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    /* �x�u�a    �x��O + 2, �y�O - 1, �h�� + 3                �x*/
    /* �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    float class_;
    long workmoney;

    /*�ⷳ�H�W�~��*/
    if ((d.bbtime / 60 / 30) < 2)
    {
        vmsg("�p���Ӥp�F�A�ⷳ�H��A�ӧa...");
        return 0;
    }
    workmoney = 0;
    class_ = ((d.body[BODY_HP] * 100 / d.body[BODY_MAXHP]) - d.body[BODY_TIRED]) * LEARN_LEVEL;
    count_tired(5, 15, true, 100, 1);
    d.body[BODY_SHIT] += random() % 4 + 16;
    d.body[BODY_WEIGHT] -= (random() % 2 + 2);
    d.body[BODY_MAXHP] += (random() % 2 + 1) * LEARN_LEVEL;
    d.fight[FIGHT_SPEED] += (random() % 2 + 2) * LEARN_LEVEL;
    d.body[BODY_HP] -= (random() % 6 + 10);
    d.learn[LEARN_CHARM] -= random() % 3 + 6;
    d.state[STATE_HAPPY] -= (random() % 5 + 10);
    d.state[STATE_SATISFY] -= random() % 5 + 6;
    if (d.learn[LEARN_CHARM] < 0)
        d.learn[LEARN_CHARM] = 0;
    move(4, 0);
    show_job_pic(111);
    if (class_ >= 75)
    {
        workmoney = 250 + d.body[BODY_MAXHP] / 50;
        vmsg("�u�{�ܧ���  ���§A�F..");
    }
    else if (class_ >= 50)
    {
        workmoney = 220 + d.body[BODY_MAXHP] / 45;
        vmsg("�u�{�|�ٶ��Q  ���W�A�F..");
    }
    else if (class_ >= 25)
    {
        workmoney = 200 + d.body[BODY_MAXHP] / 40;
        vmsg("�u�{�t�j�H�N  �A�[�o��..");
    }
    else if (class_ >= 0)
    {
        workmoney = 180 + d.body[BODY_MAXHP] / 30;
        vmsg("��  �ݥ[�j�ݥ[�j....");
    }
    else
    {
        workmoney = 160;
        vmsg("�U����O�n�@�I..�h�ҫקC�@�I�A��....");
    }

    d.thing[THING_MONEY] += workmoney * LEARN_LEVEL;
    d.work[K] += 1;
    return 0;
}

static int pip_job_workL(void)
{
    /*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    /*  �x�Ӷ�    �x���]��O + N, �P�� + 1, �y�O - 1            �x*/
    /*  �x        �x�h�� + 2                                    �x*/
    /*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    float class_;
    float class1;
    long workmoney;

    /*�T���~��*/
    if ((d.bbtime / 60 / 30) < 3)
    {
        vmsg("�p���{�b�٤Ӥp�F�A�T���H��A�ӧa...");
        return 0;
    }
    workmoney = 0;
    class_ = ((d.body[BODY_HP] * 100 / d.body[BODY_MAXHP]) - d.body[BODY_TIRED]) * LEARN_LEVEL;
    class1 = (d.state[STATE_BELIEF] - d.body[BODY_TIRED]) * LEARN_LEVEL;
    d.body[BODY_SHIT] += random() % 5 + 8;
    d.fight[FIGHT_MAXMP] += (random() % 2) * LEARN_LEVEL;
    d.state[STATE_AFFECT] += (random() % 2 + 2) * LEARN_LEVEL;
    d.learn[LEARN_BRAVE] += (random() % 2 + 2) * LEARN_LEVEL;
    count_tired(5, 12, true, 100, 1);
    d.body[BODY_HP] -= (random() % 3 + 7);
    d.state[STATE_HAPPY] -= (random() % 4 + 6);
    d.state[STATE_SATISFY] -= random() % 3 + 5;
    d.learn[LEARN_CHARM] -= random() % 3 + 6;
    if (d.learn[LEARN_CHARM] < 0)
        d.learn[LEARN_CHARM] = 0;
    show_job_pic(121);
    if (class_ >= 75 && class1 >= 75)
    {
        d.fight[FIGHT_MRESIST] += random() % 2 + 7;
        workmoney = 200 + (d.state[STATE_AFFECT] + d.learn[LEARN_BRAVE]) / 40;
        vmsg("�u�Ӧ��\\��  ���A�h�I��");
    }
    else if (class_ >= 50 && class1 >= 50)
    {
        d.fight[FIGHT_MRESIST] += random() % 2 + 5;
        workmoney = 150 + (d.state[STATE_AFFECT] + d.learn[LEARN_BRAVE]) / 50;
        vmsg("�u���ٺ⦨�\\��..�°�..");
    }
    else if (class_ >= 25 && class1 >= 25)
    {
        d.fight[FIGHT_MRESIST] += random() % 2 + 3;
        workmoney = 120 + (d.state[STATE_AFFECT] + d.learn[LEARN_BRAVE]) / 60;
        vmsg("�u���ٺ�t�j�H�N��..�[�o..");
    }
    else
    {
        d.fight[FIGHT_MRESIST] += random() % 2 + 1;
        workmoney = 80 + (d.state[STATE_AFFECT] + d.learn[LEARN_BRAVE]) / 70;
        vmsg("�ڤ]����K��ԣ�F..�ЦA�[�o..");
    }

    d.thing[THING_MONEY] += workmoney * LEARN_LEVEL;
    d.work[L] += 1;
    return 0;
}

static int pip_job_workM(void)
{
    /*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    /*  �x�a�x�Юv�x�D�w + 1, ���� + N, �y�O - 1, �h�� + 7      �x*/
    /*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    GCC_UNUSED float class_;
    long workmoney;

    if ((d.bbtime / 60 / 30) < 4)
    {
        vmsg("�p���Ӥp�F�A�|���H��A�ӧa...");
        return 0;
    }
    workmoney = 0;
    class_ = ((d.body[BODY_HP] * 100 / d.body[BODY_MAXHP]) - d.body[BODY_TIRED]) * LEARN_LEVEL;
    workmoney = 50 + d.learn[LEARN_WISDOM] / 20 + d.learn[LEARN_CHARACTER] / 20;
    count_tired(5, 10, true, 100, 1);
    d.body[BODY_SHIT] += random() % 3 + 8;
    d.learn[LEARN_CHARACTER] += (random() % 2) * LEARN_LEVEL;
    d.learn[LEARN_WISDOM] += (random() % 2) * LEARN_LEVEL;
    d.state[STATE_HAPPY] -= (random() % 3 + 6);
    d.state[STATE_SATISFY] -= random() % 3 + 5;
    d.body[BODY_HP] -= (random() % 3 + 8);
    d.thing[THING_MONEY] += workmoney * LEARN_LEVEL;
    move(4, 0);
    show_job_pic(131);
    vmsg("�a�л��P ��M���N�֤@�I�o");
    d.work[M] += 1;
    return 0;
}

static int pip_job_workN(void)
{
    /*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    /*  �x�s��    �x�i���ޥ� + N, �͸ܧޥ� + N, ���O - 2        �x*/
    /*  �x        �x�h�� + 5                                    �x*/
    /*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    /*  �x�s��    �x�Y ��    �O - RND (�h��) >=  60 ��          �x*/
    /*  �x        �x�Y �y    �O - RND (�h��) >=  50 �h�u�@���\  �x*/
    /*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    float class_;
    float class1;
    long workmoney;

    if ((d.bbtime / 60 / 30) < 5)
    {
        vmsg("�p���Ӥp�F�A�����H��A�ӧa...");
        return 0;
    }
    workmoney = 0;
    class_ = ((d.body[BODY_HP] * 100 / d.body[BODY_MAXHP]) - d.body[BODY_TIRED]) * LEARN_LEVEL;
    class1 = (d.learn[LEARN_CHARM] - d.body[BODY_TIRED]) * LEARN_LEVEL;
    d.body[BODY_SHIT] += random() % 5 + 5;
    count_tired(5, 14, true, 100, 1);
    d.body[BODY_HP] -= (random() % 3 + 5);
    d.tmp[TMP_SOCIAL] -= random() % 5 + 6;
    d.state[STATE_HAPPY] -= (random() % 4 + 6);
    d.state[STATE_SATISFY] -= random() % 3 + 5;
    d.learn[LEARN_WISDOM] -= random() % 3 + 4;
    if (d.learn[LEARN_WISDOM] < 0)
        d.learn[LEARN_WISDOM] = 0;
    /*show_job_pic(6);*/
    if (class_ >= 75 && class1 >= 75)
    {
        d.learn[LEARN_COOKSKILL] += random() % 2 + 7;
        d.learn[LEARN_SPEECH] += random() % 2 + 5;
        workmoney = 500 + (d.learn[LEARN_CHARM]) / 5;
        vmsg("�A�ܬ���  :)");
    }
    else if (class_ >= 50 && class1 >= 50)
    {
        d.learn[LEARN_COOKSKILL] += random() % 2 + 5;
        d.learn[LEARN_SPEECH] += random() % 2 + 5;
        workmoney = 400 + (d.learn[LEARN_CHARM]) / 5;
        vmsg("�Z���w�諸�C....");
    }
    else if (class_ >= 25 && class1 >= 25)
    {
        d.learn[LEARN_COOKSKILL] += random() % 2 + 4;
        d.learn[LEARN_SPEECH] += random() % 2 + 3;
        workmoney = 300 + (d.learn[LEARN_CHARM]) / 5;
        vmsg("�ܥ��Z��..���������...");
    }
    else
    {
        d.learn[LEARN_COOKSKILL] += random() % 2 + 2;
        d.learn[LEARN_SPEECH] += random() % 2 + 2;
        workmoney = 200 + (d.learn[LEARN_CHARM]) / 5;
        vmsg("�A���y�O������..�Х[�o....");
    }
    d.thing[THING_MONEY] += workmoney * LEARN_LEVEL;
    d.work[N] += 1;
    return 0;
}

static int pip_job_workO(void)
{
    /*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    /*  �x�s�a    �x�y�O + 2, �o�^ + 2, �D�w - 3, �H�� - 3      �x*/
    /*  �x        �x�ݤH���� - N, �M���˪����Y - N, �h�� + 12   �x*/
    /*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    /*  �x�s�a    �x�Y �y    �O - RND (�h��) >=  70 �h�u�@���\  �x*/
    /*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    float class_;
    long workmoney;

    if ((d.bbtime / 60 / 30) < 4)
    {
        vmsg("�p���Ӥp�F�A�|���H��A�ӧa...");
        return 0;
    }
    workmoney = 0;
    class_ = (d.learn[LEARN_CHARM] - d.body[BODY_TIRED]) * LEARN_LEVEL;
    d.body[BODY_SHIT] += random() % 5 + 14;
    d.learn[LEARN_CHARM] += (random() % 3 + 8) * LEARN_LEVEL;
    d.state[STATE_OFFENSE] += (random() % 3 + 8) * LEARN_LEVEL;
    count_tired(5, 22, true, 100, 1);
    d.body[BODY_HP] -= (random() % 3 + 8);
    d.tmp[TMP_SOCIAL] -= random() % 6 + 12;
    d.state[STATE_HAPPY] -= (random() % 4 + 8);
    d.state[STATE_SATISFY] -= random() % 3 + 8;
    d.learn[LEARN_ETHICS] -= random() % 6 + 10;
    d.state[STATE_BELIEF] -= random() % 6 + 10;
    if (d.learn[LEARN_ETHICS] < 0)
        d.learn[LEARN_ETHICS] = 0;
    if (d.state[STATE_BELIEF] < 0)
        d.state[STATE_BELIEF] = 0;

    /*show_job_pic(6);*/
    if (class_ >= 75)
    {
        d.relation -= random() % 5 + 12;
        d.learn[LEARN_TOMAN] -= random() % 5 + 12;
        workmoney = 600 + (d.learn[LEARN_CHARM]) / 5;
        vmsg("�A�O���������P��  :)");
    }
    else if (class_ >= 50)
    {
        d.relation -= random() % 5 + 8;
        d.learn[LEARN_TOMAN] -= random() % 5 + 8;
        workmoney = 500 + (d.learn[LEARN_CHARM]) / 5;
        vmsg("�A�Z���w�諸�C..:)");
    }
    else if (class_ >= 25)
    {
        d.relation -= random() % 5 + 5;
        d.learn[LEARN_TOMAN] -= random() % 5 + 5;
        workmoney = 400 + (d.learn[LEARN_CHARM]) / 5;
        vmsg("�A�ܥ��Z..����������...");
    }
    else
    {
        d.relation -= random() % 5 + 1;
        d.learn[LEARN_TOMAN] -= random() % 5 + 1;
        workmoney = 300 + (d.learn[LEARN_CHARM]) / 5;
        vmsg("��..�A���y�O������....");
    }
    d.thing[THING_MONEY] += workmoney * LEARN_LEVEL;
    if (d.relation < 0)
        d.relation = 0;
    if (d.learn[LEARN_TOMAN] < 0)
        d.learn[LEARN_TOMAN] = 0;
    d.work[O] += 1;
    return 0;
}

static int pip_job_workP(void)
{
    /*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    /*  �x�j�]�`�|�x�y�O + 3, �o�^ + 1, ��� - 2, ���O - 1      �x*/
    /*  �x        �x�ݤH���� - N, �h�� + 8                      �x*/
    /*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    /*  �x�j�]�`�|�x�Y �y    �O - RND (�h��) >=  70 ��          �x*/
    /*  �x        �x�Y ���N�׾i - RND (�h��) >=  30 �h�u�@���\  �x*/
    /*  �|�w�w�w�w�r�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�}*/
    float class_;
    float class1;
    long workmoney;

    if ((d.bbtime / 60 / 30) < 6)
    {
        vmsg("�p���Ӥp�F�A�����H��A�ӧa...");
        return 0;
    }
    workmoney = 0;
    class_ = (d.learn[LEARN_CHARM] - d.body[BODY_TIRED]) * LEARN_LEVEL;
    class1 = (d.learn[LEARN_ART] - d.body[BODY_TIRED]) * LEARN_LEVEL;
    d.body[BODY_SHIT] += random() % 5 + 7;
    d.learn[LEARN_CHARM] += (random() % 3 + 8) * LEARN_LEVEL;
    d.state[STATE_OFFENSE] += (random() % 3 + 8) * LEARN_LEVEL;
    count_tired(5, 22, true, 100, 1);
    d.body[BODY_HP] -= (random() % 3 + 8);
    d.tmp[TMP_SOCIAL] -= random() % 6 + 12;
    d.state[STATE_HAPPY] -= (random() % 4 + 8);
    d.state[STATE_SATISFY] -= random() % 3 + 8;
    d.learn[LEARN_CHARACTER] -= random() % 3 + 8;
    d.learn[LEARN_WISDOM] -= random() % 3 + 5;
    if (d.learn[LEARN_CHARACTER] < 0)
        d.learn[LEARN_CHARACTER] = 0;
    if (d.learn[LEARN_WISDOM] < 0)
        d.learn[LEARN_WISDOM] = 0;
    /*show_job_pic(6);*/
    if (class_ >= 75 && class1 > 30)
    {
        d.learn[LEARN_SPEECH] += random() % 5 + 12;
        d.learn[LEARN_TOMAN] -= random() % 5 + 12;
        workmoney = 1000 + (d.learn[LEARN_CHARM]) / 5;
        vmsg("�A�O�]�`�|�̰{�G���P�P��  :)");
    }
    else if (class_ >= 50 && class1 > 20)
    {
        d.learn[LEARN_SPEECH] += random() % 5 + 8;
        d.learn[LEARN_TOMAN] -= random() % 5 + 8;
        workmoney = 800 + (d.learn[LEARN_CHARM]) / 5;
        vmsg("���..�A�Z���w�諸�C..:)");
    }
    else if (class_ >= 25 && class1 > 10)
    {
        d.learn[LEARN_SPEECH] += random() % 5 + 5;
        d.learn[LEARN_TOMAN] -= random() % 5 + 5;
        workmoney = 600 + (d.learn[LEARN_CHARM]) / 5;
        vmsg("�A�n�[�o�F��..��������...");
    }
    else
    {
        d.learn[LEARN_SPEECH] += random() % 5 + 1;
        d.learn[LEARN_TOMAN] -= random() % 5 + 1;
        workmoney = 400 + (d.learn[LEARN_CHARM]) / 5;
        vmsg("��..�A�����....");
    }
    d.thing[THING_MONEY] += workmoney * LEARN_LEVEL;
    if (d.learn[LEARN_TOMAN] < 0)
        d.learn[LEARN_TOMAN] = 0;
    d.work[P] += 1;
    return 1;
}

/*---------------------------------------------------------------------------*/
/* ���ֿ��:���B �ȹC �B�� ���| �q��                                         */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static int pip_play_stroll(void)       /*���B*/
{
    int lucky;
    count_tired(3, 3, true, 100, 0);
    lucky = random() % 7;
    if (lucky == 2)
    {
        d.state[STATE_HAPPY] += random() % 3 + random() % 3 + 9;
        d.state[STATE_SATISFY] += random() % 3 + random() % 3 + 3;
        d.body[BODY_SHIT] += random() % 3 + 3;
        d.body[BODY_HP] -= (random() % 3 + 5);
        move(4, 0);
        if (random() % 2 > 0)
            show_play_pic(1);
        else
            show_play_pic(2);
        vmsg("�J��B���o  �u�n.... ^_^");
    }
    else if (lucky == 3)
    {
        d.thing[THING_MONEY] += 100;
        d.state[STATE_HAPPY] += random() % 3 + 6;
        d.state[STATE_SATISFY] += random() % 3 + 4;
        d.body[BODY_SHIT] += random() % 3 + 3;
        d.body[BODY_HP] -= (random() % 3 + 4);
        move(4, 0);
        show_play_pic(3);
        vmsg("�ߨ�F100���F..�C�C�C....");
    }

    else if (lucky == 4)
    {
        if (random() % 2 > 0)
        {
            d.state[STATE_HAPPY] -= (random() % 2 + 5);
            move(4, 0);
            d.body[BODY_HP] -= (random() % 3 + 3);
            show_play_pic(4);
            if (d.thing[THING_MONEY] >= 50)
            {
                d.thing[THING_MONEY] -= 50;
                vmsg("���F50���F..����....");
            }
            else
            {
                d.thing[THING_MONEY] = 0;
                d.body[BODY_HP] -= (random() % 3 + 3);
                vmsg("���������F..����....");
            }
            d.body[BODY_SHIT] += random() % 3 + 2;
        }
        else
        {
            d.state[STATE_HAPPY] += random() % 3 + 5;
            move(4, 0);
            show_play_pic(5);
            if (d.thing[THING_MONEY] >= 50)
            {
                d.thing[THING_MONEY] -= 50;
                d.body[BODY_HP] -= (random() % 3 + 3);
                vmsg("�ΤF50���F..���i�H�|�ڳ�....");
            }
            else
            {
                d.thing[THING_MONEY] = 0;
                d.body[BODY_HP] -= (random() % 3 + 3);
                vmsg("���Q�ڰ��Υ����F..:p");
            }
            d.body[BODY_SHIT] += random() % 3 + 2;
        }
    }
    else if (lucky == 5)
    {
        d.state[STATE_HAPPY] += random() % 3 + 6;
        d.state[STATE_SATISFY] += random() % 3 + 5;
        d.body[BODY_SHIT] += 2;
        move(4, 0);
        if (random() % 2 > 0)
            show_play_pic(6);
        else
            show_play_pic(7);
        vmsg("�n�γ�ߨ쪱��F��.....");
    }
    else if (lucky == 6)
    {
        d.state[STATE_HAPPY] -= (random() % 3 + 10);
        d.body[BODY_SHIT] += (random() % 3 + 20);
        move(4, 0);
        show_play_pic(9);
        vmsg("�u�O�˷�  �i�H�h�R�R�����");
    }
    else
    {
        d.state[STATE_HAPPY] += random() % 3 + 3;
        d.state[STATE_SATISFY] += random() % 2 + 1;
        d.body[BODY_SHIT] += random() % 3 + 2;
        d.body[BODY_HP] -= (random() % 3 + 2);
        move(4, 0);
        show_play_pic(8);
        vmsg("�S���S�O���Ƶo�Ͱ�.....");
    }
    return 0;
}

static int pip_play_sport(void)        /*�B��*/
{
    count_tired(3, 8, true, 100, 1);
    d.body[BODY_WEIGHT] -= (random() % 3 + 2);
    d.state[STATE_SATISFY] += random() % 2 + 3;
    if (d.state[STATE_SATISFY] > 100)
        d.state[STATE_SATISFY] = 100;
    d.body[BODY_SHIT] += random() % 5 + 10;
    d.body[BODY_HP] -= (random() % 2 + 8);
    d.body[BODY_MAXHP] += random() % 2;
    d.fight[FIGHT_SPEED] += (2 + random() % 3);
    move(4, 0);
    show_play_pic(10);
    vmsg("�B�ʦn�B�h�h��...");
    return 0;
}

static int pip_play_date(void) /*���|*/
{
    if (d.thing[THING_MONEY] < 150)
    {
        vmsg("�A�������h��! ���|�`�o���I����");
    }
    else
    {
        count_tired(3, 6, true, 100, 1);
        d.state[STATE_HAPPY] += random() % 5 + 12;
        d.body[BODY_SHIT] += random() % 3 + 5;
        d.body[BODY_HP] -= random() % 4 + 8;
        d.state[STATE_SATISFY] += random() % 5 + 7;
        d.learn[LEARN_CHARACTER] += random() % 3 + 1;
        d.thing[THING_MONEY] = d.thing[THING_MONEY] - 150;
        move(4, 0);
        show_play_pic(11);
        vmsg("���|�h  �I�I");
    }
    return 0;
}
static int pip_play_outing(void)       /*���C*/
{
    int lucky;

    if (d.thing[THING_MONEY] < 250)
    {
        vmsg("�A�������h��! �ȹC�`�o���I����");
    }
    else
    {
        d.body[BODY_WEIGHT] += random() % 2 + 1;
        d.thing[THING_MONEY] -= 250;
        count_tired(10, 45, false, 100, 0);
        d.body[BODY_HP] -= random() % 10 + 20;
        if (d.body[BODY_HP] >= d.body[BODY_MAXHP])
            d.body[BODY_HP] = d.body[BODY_MAXHP];
        d.state[STATE_HAPPY] += random() % 10 + 12;
        d.learn[LEARN_CHARACTER] += random() % 5 + 5;
        d.state[STATE_SATISFY] += random() % 10 + 10;
        lucky = random() % 4;
        if (lucky == 0)
        {
            d.fight[FIGHT_MAXMP] += random() % 3;
            d.learn[LEARN_ART] += random() % 2;
            show_play_pic(12);
            if (random() % 2 > 0)
                vmsg("�ߤ����@�ѲH�H���Pı  �n�ΪA��....");
            else
                vmsg("���� �~�� �߱��n�h�F.....");
        }
        else if (lucky == 1)
        {
            d.learn[LEARN_ART] += random() % 3;
            d.fight[FIGHT_MAXMP] += random() % 2;
            show_play_pic(13);
            if (random() % 2 > 0)
                vmsg("���s����������  �Φ��@�T���R���e..");
            else
                vmsg("�ݵ۬ݵ�  �����h�γ������o..");
        }
        else if (lucky == 2)
        {
            d.learn[LEARN_LOVE] += random() % 3;
            show_play_pic(14);
            if (random() % 2 > 0)
                vmsg("��  �Ӷ��֨S�J�����o...");
            else
                vmsg("ť���o�O�����  �A���O?");
        }
        else if (lucky == 3)
        {
            d.body[BODY_MAXHP] += random() % 3;
            show_play_pic(15);
            if (random() % 2 > 0)
                vmsg("���ڭ̺ƨg�b�]�̪����y�a....�I�I..");
            else
                vmsg("�D�n�������ﭱŧ��  �̳��w�o�طPı�F....");
        }
        if ((random() % 301 + random() % 200) % 100 == 12)
        {
            lucky = 0;
            clear();
            prints("\x1b[1;41m  " NICKNAME PIPNAME " �� %-10s                                                  \x1b[0m", d.name);
            show_play_pic(0);
            move(b_lines - 6, (d_cols>>1) + 10);
            prints("\x1b[1;36m�˷R�� \x1b[1;33m%s ��\x1b[0m", d.name);
            move(b_lines - 5, (d_cols>>1) + 10);
            outs("\x1b[1;37m�ݨ�A�o�˧V�O�����i�ۤv����O  ���ڤߤ��Q����������..\x1b[m");
            move(b_lines - 4, (d_cols>>1) + 10);
            outs("\x1b[1;36m�p�ѨϧڨM�w���A���๪�y���y  �����a���U�A�@�U....^_^\x1b[0m");
            move(b_lines - 3, (d_cols>>1) + 10);
            lucky = random() % 7;
            if (lucky >= 6)
            {
                outs("\x1b[1;33m�ڱN���A���U����O�������ɦʤ�������......\x1b[0m");
                d.body[BODY_MAXHP] = d.body[BODY_MAXHP] * 105 / 100;
                d.body[BODY_HP] = d.body[BODY_MAXHP];
                d.fight[FIGHT_MAXMP] = d.fight[FIGHT_MAXMP] * 105 / 100;
                d.fight[FIGHT_MP] = d.fight[FIGHT_MAXMP];
                d.fight[FIGHT_ATTACK] = d.fight[FIGHT_ATTACK] * 105 / 100;
                d.fight[FIGHT_RESIST] = d.fight[FIGHT_RESIST] * 105 / 100;
                d.fight[FIGHT_SPEED] = d.fight[FIGHT_SPEED] * 105 / 100;
                d.learn[LEARN_CHARACTER] = d.learn[LEARN_CHARACTER] * 105 / 100;
                d.learn[LEARN_LOVE] = d.learn[LEARN_LOVE] * 105 / 100;
                d.learn[LEARN_WISDOM] = d.learn[LEARN_WISDOM] * 105 / 100;
                d.learn[LEARN_ART] = d.learn[LEARN_ART] * 105 / 100;
                d.learn[LEARN_BRAVE] = d.learn[LEARN_BRAVE] * 105 / 100;
                d.learn[LEARN_HOMEWORK] = d.learn[LEARN_HOMEWORK] * 105 / 100;
            }

            else if (lucky >= 4)
            {
                outs("\x1b[1;33m�ڱN���A���԰���O�������ɦʤ����Q��.......\x1b[0m");
                d.fight[FIGHT_ATTACK] = d.fight[FIGHT_ATTACK] * 110 / 100;
                d.fight[FIGHT_RESIST] = d.fight[FIGHT_RESIST] * 110 / 100;
                d.fight[FIGHT_SPEED] = d.fight[FIGHT_SPEED] * 110 / 100;
                d.learn[LEARN_BRAVE] = d.learn[LEARN_BRAVE] * 110 / 100;
            }

            else if (lucky >= 2)
            {
                outs("\x1b[1;33m�ڱN���A���]�k��O�M�ͩR�O�������ɦʤ����Q��.......\x1b[0m");
                d.body[BODY_MAXHP] = d.body[BODY_MAXHP] * 110 / 100;
                d.body[BODY_HP] = d.body[BODY_MAXHP];
                d.fight[FIGHT_MAXMP] = d.fight[FIGHT_MAXMP] * 110 / 100;
                d.fight[FIGHT_MP] = d.fight[FIGHT_MAXMP];
            }
            else
            {
                outs("\x1b[1;33m�ڱN���A���P����O�������ɦʤ����G�Q��....\x1b[0m");
                d.learn[LEARN_CHARACTER] = d.learn[LEARN_CHARACTER] * 110 / 100;
                d.learn[LEARN_LOVE] = d.learn[LEARN_LOVE] * 110 / 100;
                d.learn[LEARN_WISDOM] = d.learn[LEARN_WISDOM] * 110 / 100;
                d.learn[LEARN_ART] = d.learn[LEARN_ART] * 110 / 100;
                d.learn[LEARN_HOMEWORK] = d.learn[LEARN_HOMEWORK] * 110 / 100;
            }

            vmsg("���~��[�o��...");
        }
    }
    return 0;
}

static int pip_play_kite(void) /*����*/
{
    count_tired(4, 4, true, 100, 0);
    d.body[BODY_WEIGHT] += (random() % 2 + 2);
    d.state[STATE_SATISFY] += random() % 3 + 12;
    if (d.state[STATE_SATISFY] > 100)
        d.state[STATE_SATISFY] = 100;
    d.state[STATE_HAPPY] += random() % 5 + 10;
    d.body[BODY_SHIT] += random() % 5 + 6;
    d.body[BODY_HP] -= (random() % 2 + 7);
    d.state[STATE_AFFECT] += random() % 4 + 6;
    move(4, 0);
    show_play_pic(16);
    vmsg("�񭷺�u�n����...");
    return 0;
}

static int pip_play_KTV(void)  /*KTV*/
{
    if (d.thing[THING_MONEY] < 250)
    {
        vmsg("�A�������h��! �ۺq�`�o���I����");
    }
    else
    {
        count_tired(10, 10, true, 100, 0);
        d.state[STATE_SATISFY] += random() % 2 + 20;
        if (d.state[STATE_SATISFY] > 100)
            d.state[STATE_SATISFY] = 100;
        d.state[STATE_HAPPY] += random() % 3 + 20;
        d.body[BODY_SHIT] += random() % 5 + 6;
        d.thing[THING_MONEY] -= 250;
        d.body[BODY_HP] += (random() % 2 + 6);
        d.learn[LEARN_ART] += random() % 4 + 3;
        move(4, 0);
        show_play_pic(17);
        vmsg("�A���A  �Q�n�k...");
    }
    return 0;
}

static int pip_play_guess(void)   /* �q���{�� */
{
    int com;
    int pipkey;
    GCC_UNUSED struct tm *qtime;
    time_t now;

    time(&now);
    qtime = localtime(&now);
    d.state[STATE_SATISFY] += (random() % 3 + 2);
    count_tired(2, 2, true, 100, 1);
    d.body[BODY_SHIT] += random() % 3 + 2;
    do
    {
        if (d.death == 1 || d.death == 2 || d.death == 3)
            return 0;
        if (pip_mainmenu(MODE_MAIN)) return 0;
        move(b_lines -2, 0);
        clrtoeol();
        move(b_lines, 0);
        clrtoeol();
        move(b_lines, 0);
        prints("\x1b[1;44;37m  �q�����  \x1b[46m[1]�ڥX�ŤM [2]�ڥX���Y [3]�ڥX���� [4]�q���O�� [Q]���X            %*s\x1b[m", d_cols, "");
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
        outs("�p���G�ŤM\n");
        break;
    case 1:
        outs("�p���G���Y\n");
        break;
    case 2:
        outs("�p���G��\n");
        break;
    }

    move(b_lines - 6, 0);

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

    return 0;
}

static void win(void)
{
    d.winn++;
    d.body[BODY_HP] -= random() % 2 + 3;
    move(4, 0);
    show_guess_pic(2);
    move(b_lines, 0);
    vmsg("�p����F....~>_<~");
    return;
}

static void tie(void)
{
    d.body[BODY_HP] -= random() % 2 + 3;
    d.state[STATE_HAPPY] += random() % 3 + 5;
    move(4, 0);
    show_guess_pic(3);
    move(b_lines, 0);
    vmsg("����........-_-");
    return;
}

static void lose(void)
{
    d.losee++;
    d.state[STATE_HAPPY] += random() % 3 + 5;
    d.body[BODY_HP] -= random() % 2 + 3;
    move(4, 0);
    show_guess_pic(1);
    move(b_lines, 0);
    vmsg("�p��Ĺ�o....*^_^*");
    return;
}

static void situ(void)
{
    clrchyiuan(b_lines - 4, b_lines - 2);
    move(b_lines - 4, 0);
    prints("�A:\x1b[44m %d�� %d�t\x1b[m                     \n", d.winn, d.losee);
    move(b_lines - 3, 0);
    prints("��:\x1b[44m %d�� %d�t\x1b[m                     \n", d.losee, d.winn);

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

static const char *const classrank[] = {"���", "����", "����", "�i��", "�M�~"};

struct classdata_money {
    int mul;
    int base;
};
struct classdata_basic_effect {
    int hp_dec;
    int happy_dec;
    int satisfy_dec;
    int shit_inc;
};

struct classdata {
    const char *name;
    struct classdata_money money;
    struct classdata_basic_effect effect;
    const char *word[4];
};

//   �ҦW, ��O, �ĪG, ���\�@, ���\�G, ���Ѥ@, ���ѤG
static const struct classdata pip_class_list[CLASS_COUNT] =
{
    {"�۵M���", {60, 170}, {5, 5, 4, 4}, {
        "���b�Υ\\Ū�Ѥ�..", "�ڬO�o���� cccc...",
        "�o�D���ݤ�����..�ǤF", "�ᤣ���F :~~~~~~"}
    },
    {"��֧���", {70, 190}, {5, 7, 6, 4}, {
        "�ɫe�����...�ìO�a�W��...", "�����ͫn��..�K�ӵo�X�K..",
        "��..�W�Ҥ��n�y�f��", "�A�ٲV��..�@�A�I�|��֤T�ʭ�"}
    },
    {"���ǱШ|", {70, 190}, {5, 7, 6, 4}, {
        "���p����  ���p����", "���ڭ̪ﱵ�Ѱ󤧪�",
        "��..�A�b�F����? �٤��n�n��", "���ǫ��Y�ª�..�Цn�n��..:("}
    },
    {"�x�ǱШ|", {80, 210}, {5, 6, 5, 4}, {
        "�]�l�L�k�O����L�k��..", "�q�x����A�ڭn�a�L�h���M",
        "����}�Σ�?�V�ð}��?? @_@", "�A�٥H���A�b���T��ӣ�?"}
    },
    {"�C�D�޳N", {70, 190}, {7, 5, 4, 6}, {
        "�ݧڪ��F�`  �W�t�E�C....", "�ڨ� �ڨ� �ڨ���..",
        "�C�n��í�@�I��..", "�A�b��a����? �C�����@�I"}
    },
    {"�氫�ԧ�", {60, 170}, {7, 5, 4, 6}, {
        "�٦׬O�٦�  �I�I..", "�Q�K�ɤH���..",
        "�}�A�𰪤@�I��...", "���Y���o��S�O��.."}
    },
    {"�]�k�Ш|", {90, 230}, {6, 5, 4, 6}, {
        "���� ���� ��������..", "�D�x+���i��+����+����=??",
        "�p�ߧA��������  ���n�ô�..", "����f�����n�y������y�W.."}
    },
    {"§���Ш|", {70, 190}, {6, 6, 5, 4}, {
        "�n����§������...", "�ڶ٭�..��������..",
        "���Ǥ��|��??�ѧr..", "���_���ӨS����..�ѣ�.."}
    },
    {"ø�e�ޥ�", {70, 190}, {5, 5, 4, 7}, {
        "�ܤ�����..�����N�ѥ�..", "�o�T�e���C��f�t���ܦn..",
        "���n���e�Ű�..�n�[�o..", "���n�r�e����..�a�a�p����.."}
    },
    {"�R�Чޥ�", {80, 210}, {7, 5, 4, 7}, {
        "�A�N���@�����Z��..", "�R�вӭM�ܦn��..",
        "����A�X�n�@�I..", "���U�A�u���@�I..���n�o��ʾ|.."}
    },
};

/*---------------------------------------------------------------------------*/
/* �צ���:���� �m�Z �צ�                                                   */
/* �禡�w                                                                    */
/*---------------------------------------------------------------------------*/

static int pip_practice_classA(void)
{
    /*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    /*  �x�۵M��Ǣx���O + 1~ 4, �H�� - 0~0, ���]��O - 0~0     �x*/
    /*  �x        �u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    /*  �x        �x���O + 2~ 6, �H�� - 0~1, ���]��O - 0~1     �x*/
    /*  �x        �u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    /*  �x        �x���O + 3~ 8, �H�� - 0~2, ���]��O - 0~1     �x*/
    /*  �x        �u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    /*  �x        �x���O + 4~12, �H�� - 1~3, ���]��O - 0~1     �x*/
    /*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    int body, class_;
    int change1, change2, change3, change4, change5;

    class_ = BMIN(d.learn[LEARN_WISDOM] / 200, COUNTOF(classrank) - 1); /*���*/

    body = pip_practice_function(CLASS_A, class_, 11, 12, &change1, &change2, &change3, &change4, &change5);
    if (body == -1) return 0;
    d.learn[LEARN_WISDOM] += change4 * LEARN_LEVEL;
    if (body == 0)
    {
        d.state[STATE_BELIEF] -= random() % (4 + class_ * 2);
        d.fight[FIGHT_MRESIST] -= random() % 4;
    }
    else
    {
        d.state[STATE_BELIEF] -= random() % (4 + class_ * 2);
        d.fight[FIGHT_MRESIST] -= random() % 3;
    }
    pip_practice_gradeup(CLASS_A, class_, d.learn[LEARN_WISDOM] / 200);
    if (d.state[STATE_BELIEF] < 0)  d.state[STATE_BELIEF] = 0;
    if (d.fight[FIGHT_MRESIST] < 0) d.fight[FIGHT_MRESIST] = 0;
    d.class_[A] += 1;
    return 0;
}

static int pip_practice_classB(void)
{
    /*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    /*  �x�ֵ�    �x�P�� + 1~1, ���O + 0~1, ���N�׾i + 0~1      �x*/
    /*  �x        �x��� + 0~1                                  �x*/
    /*  �x        �u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    /*  �x        �x�P�� + 1~2, ���O + 0~2, ���N�׾i + 0~1      �x*/
    /*  �x        �x��� + 0~1                                  �x*/
    /*  �x        �u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    /*  �x        �x�P�� + 1~4, ���O + 0~3, ���N�׾i + 0~1      �x*/
    /*  �x        �x��� + 0~1                                  �x*/
    /*  �x        �u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    /*  �x        �x�P�� + 2~5, ���O + 0~4, ���N�׾i + 0~1      �x*/
    /*  �x        �x��� + 0~1                                  �x*/
    /*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    int body, class_;
    int change1, change2, change3, change4, change5;

    class_ = BMIN((d.state[STATE_AFFECT] * 2 + d.learn[LEARN_WISDOM] + d.learn[LEARN_ART] * 2 + d.learn[LEARN_CHARACTER]) / 400, COUNTOF(classrank) - 1); /*�ֵ�*/

    body = pip_practice_function(CLASS_B, class_, 21, 21, &change1, &change2, &change3, &change4, &change5);
    if (body == -1) return 0;
    d.state[STATE_AFFECT] += change3 * LEARN_LEVEL;
    if (body == 0)
    {
        d.learn[LEARN_WISDOM] += random() % (class_ + 4) * LEARN_LEVEL;
        d.learn[LEARN_CHARACTER] += random() % (class_ + 4) * LEARN_LEVEL;
        d.learn[LEARN_ART] += random() % (class_ + 4) * LEARN_LEVEL;
    }
    else
    {
        d.learn[LEARN_WISDOM] += random() % (class_ + 3) * LEARN_LEVEL;
        d.learn[LEARN_CHARACTER] += random() % (class_ + 3) * LEARN_LEVEL;
        d.learn[LEARN_ART] += random() % (class_ + 3) * LEARN_LEVEL;
    }
    body = (d.state[STATE_AFFECT] * 2 + d.learn[LEARN_WISDOM] + d.learn[LEARN_ART] * 2 + d.learn[LEARN_CHARACTER]) / 400;
    pip_practice_gradeup(CLASS_B, class_, body);
    d.class_[B] += 1;
    return 0;
}

static int pip_practice_classC(void)
{
    /*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    /*  �x����    �x���O + 1~1, �H�� + 1~2, ���]��O + 0~1      �x*/
    /*  �x        �u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    /*  �x        �x���O + 1~1, �H�� + 1~3, ���]��O + 0~1      �x*/
    /*  �x        �u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    /*  �x        �x���O + 1~2, �H�� + 1~4, ���]��O + 0~1      �x*/
    /*  �x        �u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    /*  �x        �x���O + 1~3, �H�� + 1~5, ���]��O + 0~1      �x*/
    /*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    int body, class_;
    int change1, change2, change3, change4, change5;

    class_ = BMIN((d.state[STATE_BELIEF] * 2 + d.learn[LEARN_WISDOM]) / 400, COUNTOF(classrank) - 1); /*����*/

    body = pip_practice_function(CLASS_C, class_, 31, 31, &change1, &change2, &change3, &change4, &change5);
    if (body == -1) return 0;
    d.learn[LEARN_WISDOM] += change2 * LEARN_LEVEL;
    d.state[STATE_BELIEF] += change3 * LEARN_LEVEL;
    if (body == 0)
    {
        d.fight[FIGHT_MRESIST] += random() % 5 * LEARN_LEVEL;
    }
    else
    {
        d.fight[FIGHT_MRESIST] += random() % 3 * LEARN_LEVEL;
    }
    body = (d.state[STATE_BELIEF] * 2 + d.learn[LEARN_WISDOM]) / 400;
    pip_practice_gradeup(CLASS_C, class_, body);
    d.class_[C] += 1;
    return 0;
}

static int pip_practice_classD(void)
{
    /*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    /*  �x�x��    �x���O + 1~2, �԰��޳N + 0~1, �P�� - 0~1      �x*/
    /*  �x        �u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    /*  �x        �x���O + 2~4, �԰��޳N + 0~1, �P�� - 0~1      �x*/
    /*  �x        �u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    /*  �x        �x���O + 3~4, �԰��޳N + 0~1, �P�� - 0~1      �x*/
    /*  �x        �u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    /*  �x        �x���O + 4~5, �԰��޳N + 0~1, �P�� - 0~1      �x*/
    /*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    int body, class_;
    int change1, change2, change3, change4, change5;

    class_ = BMIN((d.fight[FIGHT_HSKILL] * 2 + d.learn[LEARN_WISDOM]) / 400, COUNTOF(classrank) - 1);
    body = pip_practice_function(CLASS_D, class_, 41, 41, &change1, &change2, &change3, &change4, &change5);
    if (body == -1) return 0;
    d.learn[LEARN_WISDOM] += change2 * LEARN_LEVEL;
    if (body == 0)
    {
        d.fight[FIGHT_HSKILL] += (random() % 3 + 4) * LEARN_LEVEL;
        d.state[STATE_AFFECT] -= random() % 3 + 6;
    }
    else
    {
        d.fight[FIGHT_HSKILL] += (random() % 3 + 2) * LEARN_LEVEL;
        d.state[STATE_AFFECT] -= random() % 3 + 6;
    }
    body = (d.fight[FIGHT_HSKILL] * 2 + d.learn[LEARN_WISDOM]) / 400;
    pip_practice_gradeup(CLASS_D, class_, body);
    if (d.state[STATE_AFFECT] < 0)  d.state[STATE_AFFECT] = 0;
    d.class_[D] += 1;
    return 0;
}

static int pip_practice_classE(void)
{
    /*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    /*  �x�C�N    �x�԰��޳N + 0~1, ������O + 1~1              �x*/
    /*  �x        �u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    /*  �x        �x�԰��޳N + 0~1, ������O + 1~2              �x*/
    /*  �x        �u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    /*  �x        �x�԰��޳N + 0~1, ������O + 1~3              �x*/
    /*  �x        �u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    /*  �x        �x�԰��޳N + 0~1, ������O + 1~4              �x*/
    /*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    int body, class_;
    int change1, change2, change3, change4, change5;

    class_ = BMIN((d.fight[FIGHT_HSKILL] + d.fight[FIGHT_ATTACK]) / 400, COUNTOF(classrank) - 1);

    body = pip_practice_function(CLASS_E, class_, 51, 51, &change1, &change2, &change3, &change4, &change5);
    if (body == -1) return 0;
    d.fight[FIGHT_SPEED] += (random() % 3 + 2) * LEARN_LEVEL;
    d.tmp[TMP_HEXP] += (random() % 2 + 2) * LEARN_LEVEL;
    d.fight[FIGHT_ATTACK] += change4 * LEARN_LEVEL;
    if (body == 0)
    {
        d.fight[FIGHT_HSKILL] += (random() % 3 + 5) * LEARN_LEVEL;
    }
    else
    {
        d.fight[FIGHT_HSKILL] += (random() % 3 + 3) * LEARN_LEVEL;
    }
    body = (d.fight[FIGHT_HSKILL] + d.fight[FIGHT_ATTACK]) / 400;
    pip_practice_gradeup(CLASS_E, class_, body);
    d.class_[E] += 1;
    return 0;
}

static int pip_practice_classF(void)
{
    /*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    /*  �x�氫�N  �x�԰��޳N + 1~1, ���m��O + 0~0              �x*/
    /*  �x        �u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    /*  �x        �x�԰��޳N + 1~1, ���m��O + 0~1              �x*/
    /*  �x        �u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    /*  �x        �x�԰��޳N + 1~2, ���m��O + 0~1              �x*/
    /*  �x        �u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    /*  �x        �x�԰��޳N + 1~3, ���m��O + 0~1              �x*/
    /*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    int body, class_;
    int change1, change2, change3, change4, change5;

    class_ = BMIN((d.fight[FIGHT_HSKILL] + d.fight[FIGHT_RESIST]) / 400, COUNTOF(classrank) - 1);

    body = pip_practice_function(CLASS_F, class_, 61, 61, &change1, &change2, &change3, &change4, &change5);
    if (body == -1) return 0;
    d.tmp[TMP_HEXP] += (random() % 2 + 2) * LEARN_LEVEL;
    d.fight[FIGHT_SPEED] += (random() % 3 + 2) * LEARN_LEVEL;
    d.fight[FIGHT_RESIST] += change2 * LEARN_LEVEL;
    if (body == 0)
    {
        d.fight[FIGHT_HSKILL] += (random() % 3 + 5) * LEARN_LEVEL;
    }
    else
    {
        d.fight[FIGHT_HSKILL] += (random() % 3 + 3) * LEARN_LEVEL;
    }
    body = (d.fight[FIGHT_HSKILL] + d.fight[FIGHT_RESIST]) / 400;
    pip_practice_gradeup(CLASS_F, class_, body);
    d.class_[F] += 1;
    return 0;
}

static int pip_practice_classG(void)
{
    /*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    /*  �x�]�k    �x�]�k�޳N + 1~1, �]�k��O + 0~2              �x*/
    /*  �x        �u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    /*  �x        �x�]�k�޳N + 1~2, �]�k��O + 0~3              �x*/
    /*  �x        �u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    /*  �x        �x�]�k�޳N + 1~3, �]�k��O + 0~4              �x*/
    /*  �x        �u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    /*  �x        �x�]�k�޳N + 2~4, �]�k��O + 0~5              �x*/
    /*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    int body, class_;
    int change1, change2, change3, change4, change5;

    class_ = BMIN((d.fight[FIGHT_MSKILL] + d.fight[FIGHT_MAXMP]) / 400, COUNTOF(classrank) - 1);

    body = pip_practice_function(CLASS_G, class_, 71, 72, &change1, &change2, &change3, &change4, &change5);
    if (body == -1) return 0;
    d.fight[FIGHT_MAXMP] += change3 * LEARN_LEVEL;
    d.tmp[TMP_MEXP] += (random() % 2 + 2) * LEARN_LEVEL;
    if (body == 0)
    {
        d.fight[FIGHT_MSKILL] += (random() % 3 + 7) * LEARN_LEVEL;
    }
    else
    {
        d.fight[FIGHT_MSKILL] += (random() % 3 + 4) * LEARN_LEVEL;
    }

    body = (d.fight[FIGHT_MSKILL] + d.fight[FIGHT_MAXMP]) / 400;
    pip_practice_gradeup(CLASS_G, class_, body);
    d.class_[G] += 1;
    return 0;
}

static int pip_practice_classH(void)
{
    /*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    /*  �x§��    �x§����{ + 1~1, ��� + 1~1                  �x*/
    /*  �x        �u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    /*  �x        �x§����{ + 1~2, ��� + 1~2                  �x*/
    /*  �x        �u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    /*  �x        �x§����{ + 1~3, ��� + 1~3                  �x*/
    /*  �x        �u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    /*  �x        �x§����{ + 2~4, ��� + 1~4                  �x*/
    /*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    int body, class_;
    int change1, change2, change3, change4, change5;

    class_ = BMIN((d.learn[LEARN_MANNERS] * 2 + d.learn[LEARN_CHARACTER]) / 400, COUNTOF(classrank) - 1);

    body = pip_practice_function(CLASS_H, class_, 0, 0, &change1, &change2, &change3, &change4, &change5);
    if (body == -1) return 0;
    d.tmp[TMP_SOCIAL] += (random() % 2 + 2) * LEARN_LEVEL;
    d.learn[LEARN_MANNERS] += (change1 + random() % 2) * LEARN_LEVEL;
    d.learn[LEARN_CHARACTER] += (change1 + random() % 2) * LEARN_LEVEL;
    body = (d.learn[LEARN_CHARACTER] + d.learn[LEARN_MANNERS]) / 400;
    pip_practice_gradeup(CLASS_H, class_, body);
    d.class_[H] += 1;
    return 0;
}

static int pip_practice_classI(void)
{
    /*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    /*  �xø�e    �x���N�׾i + 1~1, �P�� + 0~1                  �x*/
    /*  �x        �u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    /*  �x        �x���N�׾i + 1~2, �P�� + 0~1                  �x*/
    /*  �x        �u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    /*  �x        �x���N�׾i + 1~3, �P�� + 0~1                  �x*/
    /*  �x        �u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    /*  �x        �x���N�׾i + 2~4, �P�� + 0~1                  �x*/
    /*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    int body, class_;
    int change1, change2, change3, change4, change5;

    class_ = BMIN((d.learn[LEARN_ART] * 2 + d.learn[LEARN_CHARACTER]) / 400, COUNTOF(classrank) - 1);

    body = pip_practice_function(CLASS_I, class_, 91, 91, &change1, &change2, &change3, &change4, &change5);
    if (body == -1) return 0;
    d.learn[LEARN_ART] += change4 * LEARN_LEVEL;
    d.state[STATE_AFFECT] += change2 * LEARN_LEVEL;
    body = (d.state[STATE_AFFECT] + d.learn[LEARN_ART]) / 400;
    pip_practice_gradeup(CLASS_I, class_, body);
    d.class_[I] += 1;
    return 0;
}

static int pip_practice_classJ(void)
{
    /*  �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    /*  �x�R��    �x���N�׾i + 0~1, �y�O + 0~1, ��O + 1~1      �x*/
    /*  �x        �u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    /*  �x        �x���N�׾i + 1~1, �y�O + 0~1, ��O + 1~1      �x*/
    /*  �x        �u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    /*  �x        �x���N�׾i + 1~2, �y�O + 0~2, ��O + 1~1      �x*/
    /*  �x        �u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t*/
    /*  �x        �x���N�׾i + 1~3, �y�O + 1~2, ��O + 1~1      �x*/
    /*  �|�w�w�w�w�r�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�}*/
    int body, class_;
    int change1, change2, change3, change4, change5;

    class_ = BMIN((d.learn[LEARN_ART] * 2 + d.learn[LEARN_CHARM]) / 400, COUNTOF(classrank) - 1);

    body = pip_practice_function(CLASS_J, class_, 0, 0, &change1, &change2, &change3, &change4, &change5);
    if (body == -1) return 0;
    d.learn[LEARN_ART] += change2 * LEARN_LEVEL;
    d.body[BODY_MAXHP] += (random() % 3 + 2) * LEARN_LEVEL;
    if (body == 0)
    {
        d.learn[LEARN_CHARM] += random() % (5 + class_) * LEARN_LEVEL;
    }
    else if (body == 1)
    {
        d.learn[LEARN_CHARM] += random() % (3 + class_) * LEARN_LEVEL;
    }
    body = (d.learn[LEARN_ART] * 2 + d.learn[LEARN_CHARM]) / 400;
    pip_practice_gradeup(CLASS_J, class_, body);
    d.class_[J] += 1;
    return 0;
}

/*�ǤJ:�Ҹ� ���� �ͩR �ּ� ���� żż �Ǧ^:�ܼ�12345 return:body(-1~1)*/
static int
pip_practice_function(
enum pipclass classnum, int classgrade, int pic1, int pic2,
int *change1, int *change2, int *change3, int *change4, int *change5)
{
    int  a, b, body, health;
    char inbuf[256], ans[5];
    long smoney;

    const struct classdata *const clsdata = &pip_class_list[classnum];

    /*������k*/
    smoney = classgrade * clsdata->money.mul + clsdata->money.base;
    move(b_lines - 2, 0);
    clrtoeol();
    sprintf(inbuf, "[%8s%4s�ҵ{]�n�� $%ld�A�T�w�n��??[y/N]: ", clsdata->name, classrank[classgrade], smoney);
    getdata(B_LINES_REF - 2, 1, inbuf, ans, 2, DOECHO, 0);
    if (ans[0] != 'y' && ans[0] != 'Y')  return -1;
    if (d.thing[THING_MONEY] < smoney)
    {
        vmsg("�ܩ�p��...�A����������");
        return -1;
    }
    count_tired(4, 5, true, 100, 1);
    d.thing[THING_MONEY] = d.thing[THING_MONEY] - smoney;
    /*���\�P�_���P�_*/
    health = d.body[BODY_HP] * 1 / 2 + random() % 20 - d.body[BODY_TIRED];
    if (health > 0) body = 0;
    else body = 1;

    a = random() % 3 + 2;
    b = (random() % 12 + random() % 13) % 2;
    d.body[BODY_HP] -= random() % (3 + random() % 3) + clsdata->effect.hp_dec;
    d.state[STATE_HAPPY] -= random() % (3 + random() % 3) + clsdata->effect.happy_dec;
    d.state[STATE_SATISFY] -= random() % (3 + random() % 3) + clsdata->effect.satisfy_dec;
    d.body[BODY_SHIT] += random() % (3 + random() % 3) + clsdata->effect.shit_inc;
    *change1 = random() % a + 4 + classgrade * 2 / (body + 2);    /* random()%3+3 */
    *change2 = random() % a + 6 + classgrade * 2 / (body + 2);    /* random()%3+5 */
    *change3 = random() % a + 8 + classgrade * 3 / (body + 2);    /* random()%3+7 */
    *change4 = random() % a + 10 + classgrade * 3 / (body + 2);   /* random()%3+9 */
    *change5 = random() % a + 12 + classgrade * 3 / (body + 2);   /* random()%3+11 */
    if (random() % 2 > 0 && pic1 > 0)
        show_practice_pic(pic1);
    else if (pic2 > 0)
        show_practice_pic(pic2);
    vmsg(clsdata->word[body+b]);
    return body;
}

static int pip_practice_gradeup(
enum pipclass classnum, int classgrade, int data)
{
    char inbuf[256];

    if (data > classgrade && classgrade < COUNTOF(classrank) - 1)
    {
        sprintf(inbuf, "�U�����W [%8s%4s�ҵ{]",
                pip_class_list[classnum].name, classrank[BMIN(data, COUNTOF(classrank) - 1)]);
        vmsg(inbuf);
    }
    return 0;
}

/*---------------------------------------------------------------------------*/
/* �S����:�ݯf ��� �԰� ���X �¨�                                         */
/*                                                                           */
/*---------------------------------------------------------------------------*/


static int pip_see_doctor(void)        /*�����*/
{
    char buf[256];
    long savemoney;
    clrchyiuan(b_lines - 2, b_lines);
    savemoney = d.body[BODY_SICK] * 25;
    if (d.body[BODY_SICK] <= 0)
    {
        vmsg("�z��..�S�f����|�F��..�Q�|�F..��~~");
        d.learn[LEARN_CHARACTER] -= (random() % 3 + 1);
        if (d.learn[LEARN_CHARACTER] < 0)
            d.learn[LEARN_CHARACTER] = 0;
        d.state[STATE_HAPPY] -= (random() % 3 + 3);
        d.state[STATE_SATISFY] -= random() % 3 + 2;
    }
    else if (d.thing[THING_MONEY] < savemoney)
    {
        sprintf(buf, "�A���f�n�� %ld ����....�A��������...", savemoney);
        vmsg(buf);
    }
    else if (d.body[BODY_SICK] > 0 && d.thing[THING_MONEY] >= savemoney)
    {
        d.body[BODY_TIRED] -= random() % 10 + 20;
        if (d.body[BODY_TIRED] < 0)
            d.body[BODY_TIRED] = 0;
        d.body[BODY_SICK] = 0;
        d.thing[THING_MONEY] = d.thing[THING_MONEY] - savemoney;
        move(4, 0);
        show_special_pic(1);
        vmsg("�Ĩ�f��..�S���Ƨ@��!!");
    }
    return 0;
}

/*���*/
static int pip_change_weight(void)
{
    char genbuf[5];
    char inbuf[256];
    int weightmp;

    move(b_lines -1, 0);
    clrtoeol();
    show_special_pic(2);
    getdata(B_LINES_REF - 1, 1, "�A����ܬO? [Q]���}: ", genbuf, 2, 1, 0);
    if (genbuf[0] == '1' || genbuf[0] == '2' || genbuf[0] == '3' || genbuf[0] == '4')
    {
        switch (genbuf[0])
        {
        case '1':
            if (d.thing[THING_MONEY] < 80)
            {
                vmsg("�ǲμW�D�n80����....�A��������...");
            }
            else
            {
                getdata(B_LINES_REF - 1, 1, "�ݪ�O80��(3��5����)�A�A�T�w��? [y/N]: ", genbuf, 2, 1, 0);
                if (genbuf[0] == 'Y' || genbuf[0] == 'y')
                {
                    weightmp = 3 + random() % 3;
                    d.body[BODY_WEIGHT] += weightmp;
                    d.thing[THING_MONEY] -= 80;
                    d.body[BODY_MAXHP] -= random() % 2;
                    d.body[BODY_HP] -= random() % 2 + 3;
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
            getdata(B_LINES_REF - 1, 1, "�W�@����n30���A�A�n�W�h�֤���O? [�ж�Ʀr]: ", genbuf, 4, 1, 0);
            weightmp = atoi(genbuf);
            if (weightmp <= 0)
            {
                vmsg("��J���~..����o...");
            }
            else if (d.thing[THING_MONEY] > (weightmp*30))
            {
                sprintf(inbuf, "�W�[%d����A�`�@�ݪ�O%d���A�T�w��? [y/N]: ", weightmp, weightmp*30);
                getdata(B_LINES_REF - 1, 1, inbuf, genbuf, 2, 1, 0);
                if (genbuf[0] == 'Y' || genbuf[0] == 'y')
                {
                    d.thing[THING_MONEY] -= weightmp * 30;
                    d.body[BODY_WEIGHT] += weightmp;
                    d.body[BODY_MAXHP] -= (random() % 2 + 2);
                    count_tired(5, 8, false, 100, 1);
                    d.body[BODY_HP] -= (random() % 2 + 3);
                    d.body[BODY_SICK] += random() % 10 + 5;
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
            if (d.thing[THING_MONEY] < 80)
            {
                vmsg("�ǲδ�έn80����....�A��������...");
            }
            else
            {
                getdata(B_LINES_REF - 1, 1, "�ݪ�O80��(3��5����)�A�A�T�w��? [y/N]: ", genbuf, 2, 1, 0);
                if (genbuf[0] == 'Y' || genbuf[0] == 'y')
                {
                    weightmp = 3 + random() % 3;
                    d.body[BODY_WEIGHT] -= weightmp;
                    if (d.body[BODY_WEIGHT] < 0)
                        d.body[BODY_WEIGHT] = 0;
                    d.thing[THING_MONEY] -= 100;
                    d.body[BODY_MAXHP] += random() % 2;
                    d.body[BODY_HP] -= random() % 2 + 3;
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
            getdata(B_LINES_REF - 1, 1, "��@����n30���A�A�n��h�֤���O? [�ж�Ʀr]: ", genbuf, 4, 1, 0);
            weightmp = atoi(genbuf);
            if (weightmp <= 0)
            {
                vmsg("��J���~..����o...");
            }
            else if (d.body[BODY_WEIGHT] <= weightmp)
            {
                vmsg("�A�S���򭫳�.....");
            }
            else if (d.thing[THING_MONEY] > (weightmp*30))
            {
                sprintf(inbuf, "���%d����A�`�@�ݪ�O%d���A�T�w��? [y/N]: ", weightmp, weightmp*30);
                getdata(B_LINES_REF - 1, 1, inbuf, genbuf, 2, 1, 0);
                if (genbuf[0] == 'Y' || genbuf[0] == 'y')
                {
                    d.thing[THING_MONEY] -= weightmp * 30;
                    d.body[BODY_WEIGHT] -= weightmp;
                    d.body[BODY_MAXHP] -= (random() % 2 + 2);
                    count_tired(5, 8, false, 100, 1);
                    d.body[BODY_HP] -= (random() % 2 + 3);
                    d.body[BODY_SICK] += random() % 10 + 5;
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

static int
pip_go_palace(void)
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
    static const char *const needmode[3] = {"      ", "§����{��", "�ͦR�ޥ���"};
    int save[COUNTOF(d.royal)] = {0};

    const struct royalset *const p = royallist;

    d.nodone = 0;
    do
    {
        clear();
        show_palace_pic(0);
        move(13, 4);
        prints_centered("\x1b[1;31m�z�w�w�w�w�w�w�t\x1b[37;41m �Ө��`�q�O���F  �п�ܧA�����X����H \x1b[0;1;31m�u�w�w�w�w�w�w�{\x1b[0m");
        move(14, 4);
        prints_centered("\x1b[1;31m�x                                                                  �x\x1b[0m");

        for (n = 0; n < ROYAL_COUNT / 2; n++)
        {
            a = 2 * n;
            b = 2 * n + 1;
            move(15 + n, 4);
            sprintf(inbuf1, "%-10s%3d", needmode[p[a].needmode], p[a].needvalue);
            if (n == ROYAL_J / 2)
            {
                sprintf(inbuf2, "%-10s", needmode[p[b].needmode]);
            }
            else
            {
                sprintf(inbuf2, "%-10s%3d", needmode[p[b].needmode], p[b].needvalue);
            }
            if (n != ROYAL_J / 2 || d.see[SEE_ROYAL_J])
                prints_centered("\x1b[1;31m�x \x1b[36m(\x1b[37m%c\x1b[36m) \x1b[33m%-10s  \x1b[37m%-14s     \x1b[36m(\x1b[37m%c\x1b[36m) \x1b[33m%-10s  \x1b[37m%-14s\x1b[31m�x\x1b[0m",
                        p[a].num, p[a].name, inbuf1, p[b].num, p[b].name, inbuf2);
            else
                prints_centered("\x1b[1;31m�x \x1b[36m(\x1b[37m%c\x1b[36m) \x1b[33m%-10s  \x1b[37m%-14s                                   \x1b[31m�x\x1b[0m",
                        p[a].num, p[a].name, inbuf1);
        }
        move(20, 4);
        prints_centered("\x1b[1;31m�x                                                                  �x\x1b[0m");
        move(21, 4);
        prints_centered("\x1b[1;31m�|�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�}\x1b[0m");


        if (d.death == 1 || d.death == 2 || d.death == 3)
            return 0;
        /*�N�U�H���w�g���P���ƭȥs�^��*/
        /*from {�u��, ���, �N�x, �j��, ���q, �d�m, ���m, ���, �p��, ���l}*/
        for (int i = 0; i < COUNTOF(d.royal); i++)
            save[i] = d.royal[i];

        move(b_lines - 1, 0);
        clrtoeol();
        move(b_lines - 1, 0);
        prints("\x1b[1;33m [�ͩR�O] %d/%d  [�h�ҫ�] %d \x1b[0m", d.body[BODY_HP], d.body[BODY_MAXHP], d.body[BODY_TIRED]);

        move(b_lines, 0);
        clrtoeol();
        move(b_lines, 0);
        prints(
            "\x1b[1;37;46m  �Ѩ����  \x1b[44m [�r��]��ܱ����X���H��  [Q]���}" NICKNAME "�`�q�O��       %*s\x1b[0m", 20 + d_cols - ((int)(unsigned int)sizeof(NICKNAME) - 1), "");
        pipkey = vkey();
        choice = pipkey - 'A';
        if (choice < 0 || choice >= ROYAL_COUNT)
            choice = pipkey - 'a';

        if ((choice >= 0 && choice < ROYAL_COUNT) && (choice != ROYAL_J || d.see[SEE_ROYAL_J]))
        {
            d.tmp[TMP_SOCIAL] += random() % 3 + 3;
            d.body[BODY_HP] -= random() % 5 + 6;
            d.body[BODY_TIRED] += random() % 5 + 8;
            if (d.body[BODY_TIRED] >= 100)
            {
                d.death = 1;
                pipdie("\x1b[1;31m�֦��F...\x1b[m  ", 1);
            }
            if (d.body[BODY_HP] < 0)
            {
                d.death = 1;
                pipdie("\x1b[1;31m�j���F...\x1b[m  ", 1);
            }
            if (d.death == 1)
            {
                sprintf(buf, "�T�T�F...�u�O�d��..");
            }
            else
            {
                if ((p[choice].needmode == 0) ||
                    (p[choice].needmode == 1 && d.learn[LEARN_MANNERS] >= p[choice].needvalue) ||
                    (p[choice].needmode == 2 && d.learn[LEARN_SPEECH] >= p[choice].needvalue))
                {
                    if (choice != ROYAL_J && save[choice] >= p[choice].maxtoman)
                    {
                        if (random() % 2 > 0)
                            sprintf(buf, "��M�o�򰶤j���A���ܯu�O�a����...");
                        else
                            sprintf(buf, "�ܰ����A�ӫ��X�ڡA���ڤ��൹�A����F..");
                    }
                    else
                    {
                        change = 0;
                        switch (choice)
                        {
                        default:
                            switch (choice)
                            {
                            case ROYAL_A:
                                change = d.learn[LEARN_CHARACTER] / 5;
                                break;
                            case ROYAL_B:
                                change = d.learn[LEARN_CHARACTER] / 8;
                                break;
                            case ROYAL_C:
                                change = d.learn[LEARN_CHARM] / 5;
                                break;
                            case ROYAL_D:
                                change = d.learn[LEARN_WISDOM] / 10;
                                break;
                            case ROYAL_E:
                                change = d.state[STATE_BELIEF] / 10;
                                break;
                            case ROYAL_F:
                                change = d.learn[LEARN_SPEECH] / 10;
                                break;
                            case ROYAL_G:
                                change = d.tmp[TMP_SOCIAL] / 10;
                                break;
                            case ROYAL_H:
                                change = d.tmp[TMP_HEXP] / 10;
                                break;
                            }
                            /*�p�G�j��C�����W�[�̤j�q*/
                            if (change > p[choice].addtoman)
                                change = p[choice].addtoman;
                            /*�p�G�[�W���������j��ү൹���Ҧ��Ȯ�*/
                            if ((change + save[choice]) >= p[choice].maxtoman)
                                change = p[choice].maxtoman - save[choice];
                            save[choice] += change;
                            d.learn[LEARN_TOMAN] += change;
                            break;

                        case ROYAL_I:
                            save[ROYAL_I] = 0;
                            d.tmp[TMP_SOCIAL] -= 13 + random() % 4;
                            d.state[STATE_AFFECT] += 13 + random() % 4;
                            break;

                        case ROYAL_J:
                            save[9] += 15 + random() % 4;
                            d.see[SEE_ROYAL_J] = 0;
                            break;
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
                        sprintf(buf, "�ڤ��M�A�o�˪����͸�....");
                    else
                        sprintf(buf, "�A�o���S�оi�����A�A�h�Ǿ�§���a....");

                }
            }
            vmsg(buf);
        }
        for (int i = 0; i < COUNTOF(d.royal); i++)
            d.royal[i] = save[i];
    }
    while ((pipkey != 'Q') && (pipkey != 'q') && (pipkey != KEY_LEFT));

    vmsg("���}" NICKNAME "�`�q�O��.....");
    return 0;
}
/*--------------------------------------------------------------------------*/
/* pip_vs_fight.c �p����Ե{��                                              */
/* �@��:chyiuan   �P��SiEpthero���޳N����                                   */
/*--------------------------------------------------------------------------*/
#ifdef  HAVE_PIP_FIGHT
static void
pip_set_currutmp(void)
{
    currutmp->pip->hp = d.body[BODY_HP];
    currutmp->pip->mp = d.fight[FIGHT_MP];
    currutmp->pip->maxhp = d.body[BODY_MAXHP];
    currutmp->pip->maxmp = d.fight[FIGHT_MAXMP];
    currutmp->pip->attack = d.fight[FIGHT_ATTACK];
    currutmp->pip->resist = d.fight[FIGHT_RESIST];
    currutmp->pip->mresist = d.fight[FIGHT_MRESIST];
    currutmp->pip->speed = d.fight[FIGHT_SPEED];
}

static void
pip_get_currutmp(void)
{
    d.body[BODY_HP] = currutmp->pip->hp;
    d.fight[FIGHT_MP] = currutmp->pip->mp;
}

int
pip_vf_fight(
int fd,
int first)
{
    DL_HOLD;

    pipdata temp;
    struct chicken chickentemp;
    int ch, datac, dinjure, oldtired, oldhp;
    int oldhexp, oldmexp, oldhskill, oldmskill, oldbrave;
    int gameover = 0;
    int i;
    int notyou = 0;                     /*chyiuan:�H�K�T���Q�˿�*/
    float mresist;
    UTMP *opponent;
    char data[200], buf1[256], buf2[256], mymsg[8][150];

    memcpy(&temp, &(cutmp->pip), sizeof(pipdata));
    memcpy(&chickentemp, &d, sizeof(d));


    currutmp = cutmp;
    utmp_mode(M_CHICKEN);
    clear();
    pip_read_file(&d, cuser.userid);
    currutmp->pip->pipmode = 0; /*1:��F 2:Ĺ�F 3:�����F */
    currutmp->pip->leaving = 1;
    currutmp->pip->mode = d.chickenmode;
    pip_set_currutmp();         /*��p����data  down load for�Q�I�s��*/
    currutmp->pip->nodone = first;      /*�M�w�֥�����*/
    currutmp->pip->msgcount = 0;        /*�԰��T���k�s*/
    currutmp->pip->chatcount = 0;       /*��ѰT���k�s*/
    currutmp->pip->msg[0] = '\0';
    strcpy(currutmp->pip->name, d.name);


    /*�s�U�¤p��data*/
    oldmexp = d.tmp[TMP_MEXP];
    oldhexp = d.tmp[TMP_HEXP];
    oldbrave = d.learn[LEARN_BRAVE];
    oldhskill = d.fight[FIGHT_HSKILL];
    oldmskill = d.fight[FIGHT_MSKILL];
    opponent = cutmp->talker;
    add_io(fd, 2);
    /*��襼�ǳƧ���  �����@�U  ���F������ */
    while (gameover == 0 && (opponent->pip == NULL || opponent->pip->leaving == 0))
    {
        move(b_lines, 0);
        prints("\x1b[1;46m ����٦b�ǳƤ�                                                                %*s\x1b[m", d_cols, "");
        ch = vkey();
    }
    if (currutmp->pip->mode != opponent->pip->mode)
    {
        vmsg("�@�N���P�G�N�����ब�� PK !!");
        add_io(0, 60);
        return DL_RELEASE(0);
    }
    for (i = 0; i < 8; i++)
        mymsg[i][0] = '\0';
    for (i = 0; i < 10; i++)
        currutmp->pip->chat[i][0] = '\0';
    /*�}�l���T��*/
    sprintf(mymsg[0], "\x1b[1;37m%s �M %s ���԰��}�l�F..\x1b[m",
            opponent->pip->name, currutmp->pip->name);
    strcpy(currutmp->pip->msg, mymsg[0]);
    currutmp->pip->msgcount = 0;
    /*msgcount�Mcharcount����k���P*/
    add_io(fd, 1);
    /*  currutmp->pip->mode=0;*/
    while (!(opponent->pip || currutmp->pip->leaving == 0 || opponent->pip->leaving == 0))
    {
        clear();
        /*���F�@�Ǩ�L����]  ���������O�I�s�ª�  �ҥHreload*/
        pip_get_currutmp();
        /*              pip_set_currutmp();*/

        if (opponent->pip->nodone != 1)
            strcpy(mymsg[currutmp->pip->msgcount%8], currutmp->pip->msg);
        move(0, 0);
        outs_centered("\x1b[1;34m����\x1b[44;37m �ۤv��� \x1b[0;1;34m����������������������������������������������������������������\x1b[m\n");
        prints_centered("\x1b[1m   \x1b[33m�m  �W:\x1b[37m%-20s                                              \x1b[31m  \x1b[m\n",
               d.name);
        sprintf(buf1, "%d/%d", d.body[BODY_HP], d.body[BODY_MAXHP]);
        sprintf(buf2, "%d/%d", d.fight[FIGHT_MP], d.fight[FIGHT_MAXMP]);
        prints_centered("\x1b[1m   \x1b[33m��  �O:\x1b[37m%-24s       \x1b[33m�k  �O:\x1b[37m%-24s\x1b[33m\x1b[m\n",
               buf1, buf2);
        prints_centered("\x1b[1m   \x1b[33m��  ��:\x1b[37m%-12d\x1b[33m��  �m:\x1b[37m%-12d\x1b[33m�t  ��:\x1b[37m%-12d\x1b[33m��  �]:\x1b[37m%-9d  \x1b[m\n",
               d.fight[FIGHT_ATTACK], d.fight[FIGHT_RESIST], d.fight[FIGHT_SPEED], d.fight[FIGHT_MRESIST]);
        prints_centered("\x1b[1m   \x1b[33m�԰���:\x1b[37m%-12d\x1b[33m�]�k��:\x1b[37m%-12d\x1b[33m�]����:\x1b[37m%-12d\x1b[33m�Z����:\x1b[37m%-9d  \x1b[m\n",
               d.fight[FIGHT_HSKILL], d.fight[FIGHT_MSKILL], d.tmp[TMP_MEXP], d.tmp[TMP_HEXP]);
        prints_centered("\x1b[1m   \x1b[33m��  ��:\x1b[37m%-12d\x1b[33m��  �Y:\x1b[37m%-12d\x1b[33m�s  ��:\x1b[37m%-12d\x1b[33m�F  ��:\x1b[37m%-9d  \x1b[m\n",
               d.eat[EAT_FOOD], d.eat[EAT_BIGHP], d.eat[EAT_COOKIE], d.eat[EAT_MEDICINE]);
        prints_centered("\x1b[1m   \x1b[33m�H  �x:\x1b[37m%-12d\x1b[33m��  ��:\x1b[37m%-12d\x1b[33m�h  ��:\x1b[37m%-15d               \x1b[m\n",
               d.eat[EAT_GINSENG], d.eat[EAT_SNOWGRASS], d.body[BODY_TIRED]);
        move(7, 0);
        outs_centered("\x1b[1;34m����\x1b[44;37m �԰��T�� \x1b[0;1;34m����������������������������������������������������������������\x1b[m\n");
        for (i = 0; i < 8; i++)
        {
            move(8 + i, 1);

            if (currutmp->pip->msgcount < 8)
            {
                outs_centered(mymsg[i]);
                /*�A��pip.msgcount�b8�椺*/
            }
            else
            {
                outs_centered(mymsg[(currutmp->pip->msgcount-8+i)%8]);
                /*pip.msgcount=8:��ܤw�g��9�� �ҥH�q0->7*/
            }
        }
        move(16, 0);
        outs_centered("\x1b[1;34m����\x1b[44;37m �͸ܰT�� \x1b[0;1;34m����������������������������������������������������������������\x1b[m\n");
        for (i = 0; i < 2; i++)
        {
            move(17 + i, 0);
            if (currutmp->pip->chatcount < 3)
            {
                outs_centered(currutmp->pip->chat[i]);
                /*�A��pip.chatcount�b2�椺*/
            }
            else
            {
                prints_centered("%s", currutmp->pip->chat[(currutmp->pip->chatcount-2+i)%10]);
                /*pip.chatcount=3:��ܤw�g��2�� �ҥH�q0->1*/
            }
        }
        move(19, 0);
        outs_centered("\x1b[1;34m����\x1b[1;37;44m ����� \x1b[0;1;34m����������������������������������������������������������������\x1b[m\n");
        prints_centered("\x1b[1m   \x1b[33m�m  �W:\x1b[37m%-20s                                                \x1b[m\n",
               opponent->pip->name);
        sprintf(buf1, "%d/%d", opponent->pip->hp, opponent->pip->maxhp);
        sprintf(buf2, "%d/%d", opponent->pip->mp, opponent->pip->maxmp);
        prints_centered("\x1b[1m   \x1b[33m��  �O:\x1b[37m%-24s       \x1b[33m�k  �O:\x1b[37m%-24s\x1b[m\n",
               buf1, buf2);
        outs_centered("\x1b[1;34m������������������������������������������������������������������������������\x1b[m\n");
        if (opponent->pip->nodone == 1)
        {
            notyou = 1;
            prints("\x1b[1;37;44m  ���X�ۤ��A�еy�ݤ@�|.....                                [T/^T]CHAT/�^�U  \x1b[m");
        }
        else
        {
            notyou = 0;
            prints("\x1b[1;44;37m  �԰��R�O  \x1b[46m [1]���q [2]���O [3]�]�k [4]���m [5]�ɥR [6]�k�R [T/^T]CHAT/�^�U  \x1b[m");
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
            len = getdata(B_LINES_REF, 0, "�Q��: ", buf, 60, 1, 0);
            if (len && buf[0] != ' ')
            {
                sprintf(msg, "\x1b[1;46;33m��%s\x1b[37;45m %s \x1b[m", cuser.userid, buf);
                strcpy(opponent->pip->chat[currutmp->pip->chatcount%10], msg);
                strcpy(currutmp->pip->chat[currutmp->pip->chatcount%10], msg);
                opponent->pip->chatcount++;
                currutmp->pip->chatcount++;
            }

        }
        else if (ch == Ctrl('T') || ch == Meta('T'))
        {
            add_io(fd, 30);
            clrchyiuan(7, b_lines - 4);
            move(7, 0);
            outs_centered("\x1b[1;31m����\x1b[41;37m �^�U�͸� \x1b[0;1;31m����������������������������������������������������������������\x1b[m\n");
            for (i = 0; i < 10; i++)
            {
                move(8 + i, 0);
                if (currutmp->pip->chatcount < 10)
                {
                    outs_centered(currutmp->pip->chat[i]);
                    /*�A��pip.msgcount�b�C�椺*/
                }
                else
                {
                    prints_centered("%s", currutmp->pip->chat[(currutmp->pip->chatcount-10+i)%10]);
                    /*pip.chatcount=10:��ܤw�g��11�� �ҥH�q0->9*/
                }
            }
            move(18, 0);
            outs_centered("\x1b[1;31m����\x1b[41;37m �즹���� \x1b[0;1;31m����������������������������������������������������������������\x1b[m");
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
                if (random() % 9 == 0)
                {
                    vmsg("���M�S����..:~~~");
                    sprintf(buf, "\x1b[1;33m%s \x1b[37m�� \x1b[33m%s\x1b[37m �I�i���q�����A���O�S������...",
                            d.name, opponent->pip->name);
                }
                else
                {
                    if (opponent->pip->resistmode == 0)
                        dinjure = (d.fight[FIGHT_HSKILL] / 100 + d.tmp[TMP_HEXP] / 100 + d.fight[FIGHT_ATTACK] / 9 - opponent->pip->resist / 12 + random() % 20 - opponent->pip->speed / 30 + d.fight[FIGHT_SPEED] / 30);
                    else
                        dinjure = (d.fight[FIGHT_HSKILL] / 100 + d.tmp[TMP_HEXP] / 100 + d.fight[FIGHT_ATTACK] / 9 - opponent->pip->resist / 6 + random() % 20 - opponent->pip->speed / 10 + d.fight[FIGHT_SPEED] / 30);
                    dinjure = BMAX(dinjure, 10);
                    opponent->pip->hp -= dinjure;
                    d.tmp[TMP_HEXP] += random() % 2 + 2;
                    d.fight[FIGHT_HSKILL] += random() % 2 + 1;
                    sprintf(buf, "���q�����A�����O��C%d", dinjure);
                    vmsg(buf);
                    sprintf(buf, "\x1b[1;33m%s \x1b[37m�I�i�F���q�����A\x1b[33m%s \x1b[37m����O��C \x1b[31m%d \x1b[37m�I\x1b[m",
                            d.name, opponent->pip->name, dinjure);
                }
                opponent->pip->resistmode = 0;
                opponent->pip->msgcount++;
                currutmp->pip->msgcount++;
                strcpy(opponent->pip->msg, buf);
                strcpy(mymsg[currutmp->pip->msgcount%8], buf);
                currutmp->pip->nodone = 2;      /*����*/
                opponent->pip->nodone = 1;
                break;

            case '2':
                show_fight_pic(2);
                if (random() % 11 == 0)
                {
                    vmsg("���M�S����..:~~~");
                    sprintf(buf, "\x1b[1;33m%s \x1b[37m�� \x1b[33m%s\x1b[37m �I�i���O�����A���O�S������...",
                            d.name, opponent->pip->name);
                }
                else
                {
                    if (opponent->pip->resistmode == 0)
                        dinjure = (d.fight[FIGHT_HSKILL] / 100 + d.tmp[TMP_HEXP] / 100 + d.fight[FIGHT_ATTACK] / 5 - opponent->pip->resist / 12 + random() % 30 - opponent->pip->speed / 50 + d.fight[FIGHT_SPEED] / 30);
                    else
                        dinjure = (d.fight[FIGHT_HSKILL] / 100 + d.tmp[TMP_HEXP] / 100 + d.fight[FIGHT_ATTACK] / 5 - opponent->pip->resist / 6 + random() % 30 - opponent->pip->speed / 30 + d.fight[FIGHT_SPEED] / 30);
                    dinjure = BMAX(dinjure, 20);
                    if (d.body[BODY_HP] > 5)
                    {
                        opponent->pip->hp -= dinjure;
                        d.body[BODY_HP] -= 5;
                        d.tmp[TMP_HEXP] += random() % 3 + 3;
                        d.fight[FIGHT_HSKILL] += random() % 2 + 2;
                        sprintf(buf, "���O�����A�����O��C%d", dinjure);
                        vmsg(buf);
                        sprintf(buf, "\x1b[1;33m%s \x1b[37m�I�i�F���O�����A\x1b[33m%s \x1b[37m����O��C \x1b[31m%d \x1b[37m�I\x1b[m",
                                d.name, opponent->pip->name, dinjure);
                    }
                    else
                    {
                        d.nodone = 1;
                        vmsg("�A��HP�p��5��..�����...");
                    }
                }
                opponent->pip->resistmode = 0;
                opponent->pip->msgcount++;
                currutmp->pip->msgcount++;
                strcpy(opponent->pip->msg, buf);
                strcpy(mymsg[currutmp->pip->msgcount%8], buf);
                currutmp->pip->nodone = 2;      /*����*/
                opponent->pip->nodone = 1;
                break;

            case '3':
                clrchyiuan(8, b_lines - 4);
                oldtired = d.body[BODY_TIRED];
                oldhp = d.body[BODY_HP];
                d.fight[FIGHT_MAGICMODE] = 0;
                add_io(fd, 60);
                dinjure = pip_magic_menu(1, opponent);
                add_io(fd, 1);
                if (dinjure < 0)
                    dinjure = 5;
                if (d.nodone == 0)
                {
                    if (d.fight[FIGHT_MAGICMODE] == 1)
                    {
                        oldtired = oldtired - d.body[BODY_TIRED];
                        oldhp = d.body[BODY_HP] - oldhp;
                        sprintf(buf, "�v����A��O����%d�A�h�ҭ��C%d", oldhp, oldtired);
                        vmsg(buf);
                        sprintf(buf, "\x1b[1;33m%s \x1b[37m�ϥ��]�k�v������A��O���� \x1b[36m%d \x1b[37m�I�A�h�ҭ��C \x1b[36m%d \x1b[37m�I\x1b[m", d.name, oldhp, oldtired);
                    }
                    else
                    {
                        if (random() % 15 == 0)
                        {
                            vmsg("���M�S����..:~~~");
                            sprintf(buf, "\x1b[1;33m%s \x1b[37m�� \x1b[33m%s\x1b[37m �I�i�]�k�����A���O�S������...",
                                    d.name, opponent->pip->name);
                        }
                        else
                        {
                            dinjure = get_hurt(dinjure, d.tmp[TMP_MEXP]);
                            mresist = TCLAMP((d.tmp[TMP_MEXP]) / (opponent->pip->mresist + 1), 0.3, 3);
                            dinjure = (int)dinjure * mresist;

                            opponent->pip->hp -= dinjure;
                            d.fight[FIGHT_MSKILL] += random() % 2 + 2;
                            sprintf(buf, "�]�k�����A�����O��C%d", dinjure);
                            vmsg(buf);
                            sprintf(buf, "\x1b[1;33m%s \x1b[37m�I�i�F�]�k�����A\x1b[33m%s \x1b[37m����O��C \x1b[31m%d \x1b[37m�I\x1b[m",
                                    d.name, opponent->pip->name, dinjure);
                        }
                    }

                    opponent->pip->msgcount++;
                    currutmp->pip->msgcount++;
                    strcpy(opponent->pip->msg, buf);
                    strcpy(mymsg[currutmp->pip->msgcount%8], buf);
                    /*��_��O�O��d.body[BODY_HP]�Md.body[BODY_MAXHP]�h �ҥH�o��s*/
                    currutmp->pip->hp = d.body[BODY_HP];
                    currutmp->pip->mp = d.fight[FIGHT_MP];
                    currutmp->pip->nodone = 2;  /*����*/
                    opponent->pip->nodone = 1;
                    pip_set_currutmp();
                }
                break;

            case '4':
                currutmp->pip->resistmode = 1;
                vmsg("�p���[�j���m��....");
                sprintf(buf, "\x1b[1;33m%s \x1b[37m�[�j���m�A�ǳƥ��O��� \x1b[33m%s \x1b[37m���U�@��\x1b[m",
                        d.name, opponent->pip->name);
                opponent->pip->msgcount++;
                currutmp->pip->msgcount++;
                strcpy(opponent->pip->msg, buf);
                strcpy(mymsg[currutmp->pip->msgcount%8], buf);
                currutmp->pip->nodone = 2;      /*����*/
                opponent->pip->nodone = 1;
                break;
            case '5':
                add_io(fd, 60);
                pip_fight_feed();
                add_io(fd, 1);
                if (d.nodone != 1)
                {
                    sprintf(buf, "\x1b[1;33m%s \x1b[37m�ɥR�F���W����q�A��O�Ϊk�O����۪�����\x1b[m", d.name);
                    opponent->pip->msgcount++;
                    currutmp->pip->msgcount++;
                    strcpy(opponent->pip->msg, buf);
                    strcpy(mymsg[currutmp->pip->msgcount%8], buf);
                    /*��_��O�O��d.body[BODY_HP]�Md.body[BODY_MAXHP]�h �ҥH�o��s*/
                    currutmp->pip->hp = d.body[BODY_HP];
                    currutmp->pip->mp = d.fight[FIGHT_MP];
                    currutmp->pip->nodone = 2;  /*����*/
                    opponent->pip->nodone = 1;
                    pip_set_currutmp();
                }
                break;
            case '6':
                opponent->pip->msgcount++;
                currutmp->pip->msgcount++;
                if (random() % 20 >= 18 || (random() % 20 > 13 && d.fight[FIGHT_SPEED] <= opponent->pip->speed))
                {
                    vmsg("�Q�k�]�A�o���ѤF...");
                    sprintf(buf, "\x1b[1;33m%s \x1b[37m�Q���k�]�A��...���o���ѤF...\x1b[m", d.name);
                    strcpy(opponent->pip->msg, buf);
                    strcpy(mymsg[currutmp->pip->msgcount%8], buf);
                }
                else
                {
                    sprintf(buf, "\x1b[1;33m%s \x1b[37m��ı�����L���A�ҥH�M�w���k�]�A��...\x1b[m", d.name);
                    strcpy(opponent->pip->msg, buf);
                    strcpy(mymsg[currutmp->pip->msgcount%8], buf);
                    currutmp->pip->pipmode = 3;
                    clear();
                    vs_head("�q�l�i�p��", BoardName);
                    move(10, 0);
                    outs_centered("            \x1b[1;31m�z�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�{\x1b[m\n");
                    prints_centered("            \x1b[1;31m�x  \x1b[37m��O���j���p�� \x1b[33m%-10s                 \x1b[31m�x\x1b[m\n", d.name);
                    prints_centered("            \x1b[1;31m�x  \x1b[37m�b�P��� \x1b[32m%-10s \x1b[37m�԰��Ḩ�]��          \x1b[31m�x\x1b[m\n", opponent->pip->name);
                    outs_centered("            \x1b[1;31m�|�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�}\x1b[m\n");
                    currutmp->pip->leaving = 0;
                    add_io(fd, 60);
                    vmsg("�T�Q���p �����W��...");
                }
                currutmp->pip->nodone = 2;      /*����*/
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
            outs_centered("            \x1b[1;31m�z�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�{\x1b[m\n");
            prints_centered("            \x1b[1;31m�x  \x1b[37m�^�i���p�� \x1b[33m%-10s                     \x1b[31m�x\x1b[m\n", d.name);
            prints_centered("            \x1b[1;31m�x  \x1b[37m���ѤF���p�� \x1b[32m%-10s                 \x1b[31m�x\x1b[m\n", opponent->pip->name);
            outs_centered("            \x1b[1;31m�|�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�}\x1b[m");
            currutmp->pip->leaving = 0;
            add_io(fd, 60);
            if (opponent->pip->hp <= 0)
                vmsg("��覺���o..�ҥH�AĹ�o..");
            else
                vmsg("��踨�]�o..�ҥH��AĹ�o.....");
        }
        if (gameover != 1 && (opponent->pip->pipmode == 2 || currutmp->pip->pipmode == 1))
        {
            clear();
            vs_head("�q�l�i�p��", BoardName);
            move(10, 0);
            outs_centered("            \x1b[1;31m�z�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�{\x1b[m\n");
            prints_centered("            \x1b[1;31m�x  \x1b[37m�i�����p�� \x1b[33m%-10s                     \x1b[31m�x\x1b[m\n", d.name);
            prints_centered("            \x1b[1;31m�x  \x1b[37m�b�P \x1b[32m%-10s \x1b[37m���԰����A                \x1b[31m�x\x1b[m\n", opponent->pip->name);
            outs_centered("            \x1b[1;31m�x  \x1b[37m�����a����F�A�O�̲{���S�O����.........   \x1b[31m�x\x1b[m\n");
            outs_centered("            \x1b[1;31m�|�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�}\x1b[m\n");
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
    return DL_RELEASE(0);
}
#endif  /* #ifdef  HAVE_PIP_FIGHT */

/*---------------------------------------------------------------------------*/
/* �����禡                                                                  */
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
/*  �����ѼƳ]�w                                                            */
/*--------------------------------------------------------------------------*/

static int /*�����e��*/
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
    outs("\x1b[1;33m������������������������������������������������������������\x1b[0m");
    move(2, (d_cols>>1) + 9);
    outs("\x1b[1;37m��      ����    ������      ����      ����    ������      ��\x1b[0m");
    move(3, (d_cols>>1) + 9);
    outs("\x1b[0;37m��    ������    ������  ������������������    ������  ������\x1b[0m");
    move(4, (d_cols>>1) + 9);
    outs("\x1b[0;37m��    ������  ��  ����  ������������������  ��  ����  ������\x1b[0m");
    move(5, (d_cols>>1) + 9);
    outs("\x1b[1;37m��      ����  ��  ����      ����      ����  ��  ����      ��\x1b[0m");
    move(6, (d_cols>>1) + 9);
    outs("\x1b[1;35m������������������������������������������������������������\x1b[0m");
    move(b_lines - 16, (d_cols>>1) + 8);
    outs("\x1b[1;31m�w�w�w�w�w�w�w�w�w�w\x1b[41;37m " NICKNAME PIPNAME "�������i \x1b[0;1;31m�w�w�w�w�w�w�w�w�w�w\x1b[0m");
    move(b_lines - 14, (d_cols>>1) + 10);
    outs("\x1b[1;36m�o�Ӯɶ�������ı�a�٬O���{�F...\x1b[0m");
    move(b_lines - 12, (d_cols>>1) + 10);
    prints("\x1b[1;37m\x1b[33m%s\x1b[37m �o���}�A���ŷx�h��A�ۤv�@�����b�~���D�ͦs�F.....\x1b[0m", d.name);
    move(b_lines - 10, (d_cols>>1) + 10);
    outs("\x1b[1;36m�b�A���U�оɥL���o�q�ɥ��A���L��Ĳ�F�ܦh���A���i�F�ܦh����O....\x1b[0m");
    move(b_lines - 8, (d_cols>>1) + 10);
    prints("\x1b[1;37m�]���o�ǡA���p�� \x1b[33m%s\x1b[37m ���᪺�ͬ��A�ܱo��h���h���F........\x1b[0m", d.name);
    move(b_lines - 6, (d_cols>>1) + 10);
    outs("\x1b[1;36m���A�����ߡA�A���I�X�A�A�Ҧ����R......\x1b[0m");
    move(b_lines - 4, (d_cols>>1) + 10);
    prints("\x1b[1;37m\x1b[33m%s\x1b[37m �|�û����ʰO�b�ߪ�....\x1b[0m", d.name);
    vmsg("���U�Ӭݥ��ӵo�i");
    clrchyiuan(b_lines - 16, b_lines - 4);
    move(b_lines - 16, (d_cols>>1) + 8);
    outs("\x1b[1;34m�w�w�w�w�w�w�w�w�w�w\x1b[44;37m " NICKNAME PIPNAME "���ӵo�i \x1b[0;1;34m�w�w�w�w�w�w�w�w�w�w\x1b[0m");
    move(b_lines - 14, (d_cols>>1) + 10);
    prints("\x1b[1;36m�z�L�����y�A���ڭ̤@�_�Ӭ� \x1b[33m%s\x1b[36m �����ӵo�i�a.....\x1b[0m", d.name);
    move(b_lines - 12, (d_cols>>1) + 10);
    prints("\x1b[1;37m�p�� \x1b[33m%s\x1b[37m ���%s....\x1b[0m", d.name, endbuf1);
    move(b_lines - 10, (d_cols>>1) + 10);
    prints("\x1b[1;36m�]���L�����e���V�O�A�ϱo�L�b�o�@�譱%s....\x1b[0m", endbuf2);
    move(b_lines - 8, (d_cols>>1) + 10);
    prints("\x1b[1;37m�ܩ�p�����B�ê��p�A�L���%s�A�B�ú�O�ܬ���.....\x1b[0m", endbuf3);
    move(b_lines - 6, (d_cols>>1) + 10);
    outs("\x1b[1;36m��..�o�O�@�Ӥ�����������..........\x1b[0m");
    vmsg("�ڷQ  �A�@�w�ܷP�ʧa.....");
    show_ending_pic(0);
    vmsg("�ݤ@�ݤ����o");
    endgrade = pip_game_over(endgrade);
    /* inmoney(endgrade*10*ba);
      inexp(endgrade*ba);*/
    PROC_CMD("/bin/rm", get_path(cuser.userid, "chicken"));
    sprintf(buf, "�o�� %d ���A%d �I�g���", endgrade*10*ba, endgrade*10);
    vmsg(buf);
    vmsg("�U�@���O�p�����  ����copy�U�ӧ@����");
    pip_data_list(cuser.userid);
    vmsg("�w��A�ӬD��....");
    /*�O���}�l*/
    now = time(0);
    sprintf(buf, "\x1b[1;35m�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w\x1b[0m\n");
    pip_log_record(buf);
    sprintf(buf, "\x1b[1;37m�b \x1b[33m%s \x1b[37m���ɭԡA\x1b[36m%s \x1b[37m���p�� \x1b[32m%s\x1b[37m �X�{�F����\x1b[0m\n", Cdate(&now), cuser.userid, d.name);
    pip_log_record(buf);
    sprintf(buf, "\x1b[1;37m�p�� \x1b[32m%s \x1b[37m�V�O�[�j�ۤv�A���%s\x1b[0m\n\x1b[1;37m�]�����e���V�O�A�ϱo�b�o�@�譱%s\x1b[0m\n", d.name, endbuf1, endbuf2);
    pip_log_record(buf);
    sprintf(buf, "\x1b[1;37m�ܩ�B�ê��p�A�L���%s�A�B�ú�O�ܬ���.....\x1b[0m\n\n\x1b[1;37m�p�� \x1b[32n%s\x1b[37m ���`�n���� \x1b[33m%d\x1b[0m\n", endbuf3, d.name, endgrade);
    pip_log_record(buf);
    sprintf(buf, "\x1b[1;35m�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w\x1b[0m\n");
    pip_log_record(buf);
    /*�O���פ�*/
    d.death = 3;
    pipdie("\x1b[1;31m�C�������o...\x1b[m  ", 3);
    return 0;
}

static int
pip_ending_decide(
char *endbuf1, char *endbuf2, char *endbuf3,
int *endmode, int *endgrade)
{
    //   �k��, �k��
    static const char *const name[][2] = {
        {"�����F�P�檺�k��", "���F�P�檺�k��"},
        {"�������l",   "���F���D"},
        {"�����A",     "���F�A"},
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
        /*0:�t�� 1:���N 2:�U�� 3:�Ԥh 4:�]�k 5:���� 6:�a��*/
    case 0:
        pip_endingblack(buf1, &m, &n, &grade);
        break;
    case 1:
        pip_endingart(buf1, &m, &n, &grade);
        break;
    case 2:
        pip_endingall_purpose(buf1, &m, &n, &grade, modeall_purpose);
        break;
    case 3:
        pip_endingcombat(buf1, &m, &n, &grade);
        break;
    case 4:
        pip_endingmagic(buf1, &m, &n, &grade);
        break;
    case 5:
        pip_endingsocial(buf1, &m, &n, &grade);
        break;
    case 6:
        pip_endingfamily(buf1, &m, &n, &grade);
        break;
    }

    grade += pip_marry_decide();
    strcpy(endbuf1, buf1);
    if (n == 0)
    {
        *endgrade = grade + 300;
        sprintf(buf2, "�D�`�����Q..");
    }
    else if (n == 1)
    {
        *endgrade = grade + 100;
        sprintf(buf2, "��{�٤���..");
    }
    else if (n == 2)
    {
        *endgrade = grade - 10;
        sprintf(buf2, "�`�J��ܦh���D....");
    }
    strcpy(endbuf2, buf2);
    if (d.lover >= 1 && d.lover < COUNTOF(name))
    {
        if (d.sex == 1)
            sprintf(buf2, "%s", name[d.lover][1]);
        else
            sprintf(buf2, "%s", name[d.lover][0]);
    }
    else if (d.lover == 10)
        sprintf(buf2, "%s", buf1);
    else
    {
        if (d.sex == 1)
            sprintf(buf2, "%s", name[0][1]);
        else
            sprintf(buf2, "%s", name[0][0]);
    }
    strcpy(endbuf3, buf2);
    return 0;
}
/*�����P�_*/
/*0:�t�� 1:���N 2:�U�� 3:�Ԥh 4:�]�k 5:���� 6:�a��*/
static int
pip_future_decide(
int *modeall_purpose)
{
    int endmode;
    /*�t��*/
    if ((d.learn[LEARN_ETHICS] == 0 && d.state[STATE_OFFENSE] >= 100) || (d.learn[LEARN_ETHICS] > 0 && d.learn[LEARN_ETHICS] < 50 && d.state[STATE_OFFENSE] >= 250))
        endmode = 0;
    /*���N*/
    else if (d.learn[LEARN_ART] > d.tmp[TMP_HEXP] && d.learn[LEARN_ART] > d.tmp[TMP_MEXP] && d.learn[LEARN_ART] > d.fight[FIGHT_HSKILL] && d.learn[LEARN_ART] > d.fight[FIGHT_MSKILL] &&
             d.learn[LEARN_ART] > d.tmp[TMP_SOCIAL] && d.learn[LEARN_ART] > d.tmp[TMP_FAMILY] && d.learn[LEARN_ART] > d.learn[LEARN_HOMEWORK] && d.learn[LEARN_ART] > d.learn[LEARN_WISDOM] &&
             d.learn[LEARN_ART] > d.learn[LEARN_CHARM] && d.learn[LEARN_ART] > d.state[STATE_BELIEF] && d.learn[LEARN_ART] > d.learn[LEARN_MANNERS] && d.learn[LEARN_ART] > d.learn[LEARN_SPEECH] &&
             d.learn[LEARN_ART] > d.learn[LEARN_COOKSKILL] && d.learn[LEARN_ART] > d.learn[LEARN_LOVE])
        endmode = 1;
    /*�԰�*/
    else if (d.tmp[TMP_HEXP] >= d.tmp[TMP_SOCIAL] && d.tmp[TMP_HEXP] >= d.tmp[TMP_MEXP] && d.tmp[TMP_HEXP] >= d.tmp[TMP_FAMILY])
    {
        *modeall_purpose = 0;
        if (d.tmp[TMP_HEXP] > d.tmp[TMP_SOCIAL] + 50 || d.tmp[TMP_HEXP] > d.tmp[TMP_MEXP] + 50 || d.tmp[TMP_HEXP] > d.tmp[TMP_FAMILY] + 50)
            endmode = 3;
        else
            endmode = 2;
    }
    /*�]�k*/
    else if (d.tmp[TMP_MEXP] >= d.tmp[TMP_HEXP] && d.tmp[TMP_MEXP] >= d.tmp[TMP_SOCIAL] && d.tmp[TMP_MEXP] >= d.tmp[TMP_FAMILY])
    {
        *modeall_purpose = 1;
        if (d.tmp[TMP_MEXP] > d.tmp[TMP_HEXP] || d.tmp[TMP_MEXP] > d.tmp[TMP_SOCIAL] || d.tmp[TMP_MEXP] > d.tmp[TMP_FAMILY])
            endmode = 4;
        else
            endmode = 2;
    }
    else if (d.tmp[TMP_SOCIAL] >= d.tmp[TMP_HEXP] && d.tmp[TMP_SOCIAL] >= d.tmp[TMP_MEXP] && d.tmp[TMP_SOCIAL] >= d.tmp[TMP_FAMILY])
    {
        *modeall_purpose = 2;
        if (d.tmp[TMP_SOCIAL] > d.tmp[TMP_HEXP] + 50 || d.tmp[TMP_SOCIAL] > d.tmp[TMP_MEXP] + 50 || d.tmp[TMP_SOCIAL] > d.tmp[TMP_FAMILY] + 50)
            endmode = 5;
        else
            endmode = 2;
    }

    else
    {
        *modeall_purpose = 3;
        if (d.tmp[TMP_FAMILY] > d.tmp[TMP_HEXP] + 50 || d.tmp[TMP_FAMILY] > d.tmp[TMP_MEXP] + 50 || d.tmp[TMP_FAMILY] > d.tmp[TMP_SOCIAL] + 50)
            endmode = 6;
        else
            endmode = 2;
    }
    return endmode;
}
/*���B���P�_*/
static int
pip_marry_decide(void)
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
        if (d.royal[J] >= d.relation && d.royal[J] >= 100)
        {
            d.lover = 1;  /*���l*/
            grade = 200;
        }
        else if (d.relation > d.royal[J] && d.relation >= 100)
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


static int
pip_endingblack( /*�t��*/
char *buf,
int *m, int *n, int *grade)
{
    if (d.state[STATE_OFFENSE] >= 500 && d.tmp[TMP_MEXP] >= 500) /*�]��*/
    {
        *m = 0;
        if (d.tmp[TMP_MEXP] >= 1000)
            *n = 0;
        else if (d.tmp[TMP_MEXP] < 1000 && d.tmp[TMP_MEXP] >= 800)
            *n = 1;
        else
            *n = 2;
    }

    else if (d.tmp[TMP_HEXP] >= 600)  /*�y�]*/
    {
        *m = 1;
        if (d.learn[LEARN_WISDOM] >= 350)
            *n = 0;
        else if (d.learn[LEARN_WISDOM] < 350 && d.learn[LEARN_WISDOM] >= 300)
            *n = 1;
        else
            *n = 2;
    }
    else if (d.learn[LEARN_SPEECH] >= 100 && d.learn[LEARN_ART] >= 80) /*SM*/
    {
        *m = 2;
        if (d.learn[LEARN_SPEECH] > 150 && d.learn[LEARN_ART] >= 120)
            *n = 0;
        else if (d.learn[LEARN_SPEECH] > 120 && d.learn[LEARN_ART] >= 100)
            *n = 1;
        else
            *n = 2;
    }
    else if (d.tmp[TMP_HEXP] >= 320 && d.learn[LEARN_CHARACTER] > 200 && d.learn[LEARN_CHARM] < 200)       /*�µ�Ѥj*/
    {
        *m = 3;
        if (d.tmp[TMP_HEXP] >= 400)
            *n = 0;
        else if (d.tmp[TMP_HEXP] < 400 && d.tmp[TMP_HEXP] >= 360)
            *n = 1;
        else
            *n = 2;
    }
    else if (d.learn[LEARN_CHARACTER] >= 200 && d.learn[LEARN_CHARM] >= 200 && d.learn[LEARN_SPEECH] > 70 && d.learn[LEARN_TOMAN] > 70)  /*���ű@��*/
    {
        *m = 4;
        if (d.learn[LEARN_CHARM] >= 300)
            *n = 0;
        else if (d.learn[LEARN_CHARM] < 300 && d.learn[LEARN_CHARM] >= 250)
            *n = 1;
        else
            *n = 2;
    }

    else if (d.learn[LEARN_WISDOM] >= 450)  /*�B�F�v*/
    {
        *m = 5;
        if (d.learn[LEARN_WISDOM] >= 550)
            *n = 0;
        else if (d.learn[LEARN_WISDOM] < 550 && d.learn[LEARN_WISDOM] >= 500)
            *n = 1;
        else
            *n = 2;
    }

    else /*�y�a*/
    {
        *m = 6;
        if (d.learn[LEARN_CHARM] >= 350)
            *n = 0;
        else if (d.learn[LEARN_CHARM] < 350 && d.learn[LEARN_CHARM] >= 300)
            *n = 1;
        else
            *n = 2;
    }
    if (d.sex == 1)
        strcpy(buf, endmodeblack[*m].boy);
    else
        strcpy(buf, endmodeblack[*m].girl);
    *grade = endmodeblack[*m].grade;
    return 0;
}


static int
pip_endingsocial( /*����*/
char *buf,
int *m, int *n, int *grade)
{
    int class_;
    if (d.tmp[TMP_SOCIAL] > 600) class_ = 0;
    else if (d.tmp[TMP_SOCIAL] > 450) class_ = 1;
    else if (d.tmp[TMP_SOCIAL] > 380) class_ = 2;
    else if (d.tmp[TMP_SOCIAL] > 250) class_ = 3;
    else class_ = 4;

    switch (class_)
    {
    case 0:
        if (d.learn[LEARN_CHARM] > 500)
        {
            *m = 0;
            d.lover = 10;
            if (d.learn[LEARN_CHARACTER] >= 700)
                *n = 0;
            else if (d.learn[LEARN_CHARACTER] >= 500)
                *n = 1;
            else
                *n = 2;
        }
        else
        {
            *m = 1;
            d.lover = 10;
            if (d.learn[LEARN_CHARACTER] >= 700)
                *n = 0;
            else if (d.learn[LEARN_CHARACTER] >= 500)
                *n = 1;
            else
                *n = 2;
        }
        break;

    case 1:
        *m = 0;
        d.lover = 10;
        if (d.learn[LEARN_CHARACTER] >= 700)
            *n = 0;
        else if (d.learn[LEARN_CHARACTER] >= 500)
            *n = 1;
        else
            *n = 2;
        break;

    case 2:
        if (d.learn[LEARN_CHARACTER] >= d.learn[LEARN_CHARM])
        {
            *m = 2;
            d.lover = 10;
            if (d.learn[LEARN_TOMAN] >= 250)
                *n = 0;
            else if (d.learn[LEARN_TOMAN] >= 200)
                *n = 1;
            else
                *n = 2;
        }
        else
        {
            *m = 3;
            d.lover = 10;
            if (d.learn[LEARN_CHARACTER] >= 400)
                *n = 0;
            else if (d.learn[LEARN_CHARACTER] >= 300)
                *n = 1;
            else
                *n = 2;
        }
        break;

    case 3:
        if (d.learn[LEARN_WISDOM] >= d.state[STATE_AFFECT])
        {
            *m = 4;
            d.lover = 10;
            if (d.learn[LEARN_TOMAN] > 120 && d.learn[LEARN_COOKSKILL] > 300 && d.learn[LEARN_HOMEWORK] > 300)
                *n = 0;
            else if (d.learn[LEARN_TOMAN] > 100 && d.learn[LEARN_COOKSKILL] > 250 && d.learn[LEARN_HOMEWORK] > 250)
                *n = 1;
            else
                *n = 2;
        }
        else
        {
            *m = 5;
            d.lover = 10;
            if (d.body[BODY_HP] >= 400)
                *n = 0;
            else if (d.body[BODY_HP] >= 300)
                *n = 1;
            else
                *n = 2;
        }
        break;

    case 4:
        *m = 6;
        d.lover = 10;
        if (d.learn[LEARN_CHARM] >= 200)
            *n = 0;
        else if (d.learn[LEARN_CHARM] >= 100)
            *n = 1;
        else
            *n = 2;
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
pip_endingmagic( /*�]�k*/
char *buf,
int *m, int *n, int *grade)
{
    int class_;
    if (d.tmp[TMP_MEXP] > 800) class_ = 0;
    else if (d.tmp[TMP_MEXP] > 600) class_ = 1;
    else if (d.tmp[TMP_MEXP] > 500) class_ = 2;
    else if (d.tmp[TMP_MEXP] > 300) class_ = 3;
    else class_ = 4;

    switch (class_)
    {
    case 0:
        if (d.state[STATE_AFFECT] > d.learn[LEARN_WISDOM] && d.state[STATE_AFFECT] > d.state[STATE_BELIEF] && d.learn[LEARN_ETHICS] > 100)
        {
            *m = 0;
            if (d.learn[LEARN_ETHICS] >= 800)
                *n = 0;
            else if (d.learn[LEARN_ETHICS] >= 400)
                *n = 1;
            else
                *n = 2;
        }
        else if (d.learn[LEARN_ETHICS] < 50)
        {
            *m = 3;
            if (d.body[BODY_HP] >= 400)
                *n = 0;
            else if (d.body[BODY_HP] >= 200)
                *n = 1;
            else
                *n = 2;
        }
        else
        {
            *m = 1;
            if (d.learn[LEARN_WISDOM] >= 800)
                *n = 0;
            else if (d.learn[LEARN_WISDOM] >= 400)
                *n = 1;
            else
                *n = 2;
        }
        break;

    case 1:
        if (d.learn[LEARN_ETHICS] >= 50)
        {
            *m = 2;
            if (d.learn[LEARN_WISDOM] >= 500)
                *n = 0;
            else if (d.learn[LEARN_WISDOM] >= 200)
                *n = 1;
            else
                *n = 2;
        }
        else
        {
            *m = 3;
            if (d.body[BODY_HP] >= 400)
                *n = 0;
            else if (d.body[BODY_HP] >= 200)
                *n = 1;
            else
                *n = 2;
        }
        break;

    case 2:
        *m = 4;
        if (d.fight[FIGHT_MSKILL] >= 300)
            *n = 0;
        else if (d.fight[FIGHT_MSKILL] >= 150)
            *n = 1;
        else
            *n = 2;
        break;

    case 3:
        *m = 5;
        if (d.learn[LEARN_SPEECH] >= 150)
            *n = 0;
        else if (d.learn[LEARN_SPEECH] >= 60)
            *n = 1;
        else
            *n = 2;
        break;

    case 4:
        if (d.learn[LEARN_CHARACTER] >= 200)
        {
            *m = 6;
            if (d.learn[LEARN_SPEECH] >= 150)
                *n = 0;
            else if (d.learn[LEARN_SPEECH] >= 60)
                *n = 1;
            else
                *n = 2;
        }
        else
        {
            *m = 7;
            if (d.learn[LEARN_SPEECH] >= 150)
                *n = 0;
            else if (d.learn[LEARN_SPEECH] >= 60)
                *n = 1;
            else
                *n = 2;
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
pip_endingcombat( /*�԰�*/
char *buf,
int *m, int *n, int *grade)
{
    int class_;
    if (d.tmp[TMP_HEXP] > 1500) class_ = 0;
    else if (d.tmp[TMP_HEXP] > 1000) class_ = 1;
    else if (d.tmp[TMP_HEXP] > 800) class_ = 2;
    else class_ = 3;

    switch (class_)
    {
    case 0:
        if (d.state[STATE_AFFECT] > d.learn[LEARN_WISDOM] && d.state[STATE_AFFECT] > d.state[STATE_BELIEF] && d.learn[LEARN_ETHICS] > 100)
        {
            *m = 0;
            if (d.learn[LEARN_ETHICS] >= 800)
                *n = 0;
            else if (d.learn[LEARN_ETHICS] >= 400)
                *n = 1;
            else
                *n = 2;
        }
        else if (d.learn[LEARN_ETHICS] < 50)
        {
            *m = 3;
            if (d.body[BODY_HP] >= 400)
                *n = 0;
            else if (d.body[BODY_HP] >= 200)
                *n = 1;
            else
                *n = 2;
        }
        else
        {
            *m = 1;
            if (d.learn[LEARN_WISDOM] >= 800)
                *n = 0;
            else if (d.learn[LEARN_WISDOM] >= 400)
                *n = 1;
            else
                *n = 2;
        }
        break;

    case 1:
        if (d.learn[LEARN_CHARACTER] >= 300 && d.learn[LEARN_ETHICS] > 50)
        {
            *m = 2;
            if (d.learn[LEARN_ETHICS] >= 300 && d.learn[LEARN_CHARM] >= 300)
                *n = 0;
            else if (d.learn[LEARN_ETHICS] >= 250 && d.learn[LEARN_CHARM] >= 250)
                *n = 1;
            else
                *n = 2;
        }
        else if (d.learn[LEARN_ETHICS] > 50)
        {
            *m = 3;
            if (d.learn[LEARN_SPEECH] >= 200)
                *n = 0;
            else if (d.learn[LEARN_SPEECH] >= 80)
                *n = 1;
            else
                *n = 2;
        }
        else
        {
            *m = 6;
            if (d.body[BODY_HP] >= 400)
                *n = 0;
            else if (d.body[BODY_HP] >= 200)
                *n = 1;
            else
                *n = 2;
        }
        break;

    case 2:
        if (d.learn[LEARN_CHARACTER] >= 400 && d.learn[LEARN_ETHICS] > 50)
        {
            *m = 4;
            if (d.learn[LEARN_ETHICS] >= 300)
                *n = 0;
            else if (d.learn[LEARN_ETHICS] >= 150)
                *n = 1;
            else
                *n = 2;
        }
        else if (d.learn[LEARN_ETHICS] > 50)
        {
            *m = 3;
            if (d.learn[LEARN_SPEECH] >= 200)
                *n = 0;
            else if (d.learn[LEARN_SPEECH] >= 80)
                *n = 1;
            else
                *n = 2;
        }
        else
        {
            *m = 6;
            if (d.body[BODY_HP] >= 400)
                *n = 0;
            else if (d.body[BODY_HP] >= 200)
                *n = 1;
            else
                *n = 2;
        }
        break;

    case 3:
        if (d.learn[LEARN_ETHICS] >= 50)
        {
            *m = 5;
        }
        else
        {
            *m = 7;
        }
        if (d.fight[FIGHT_HSKILL] >= 100)
            *n = 0;
        else if (d.fight[FIGHT_HSKILL] >= 80)
            *n = 1;
        else
            *n = 2;
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
pip_endingfamily( /*�a��*/
char *buf,
int *m, int *n, int *grade)
{
    *m = 0;
    if (d.learn[LEARN_CHARM] >= 200)
        *n = 0;
    else if (d.learn[LEARN_CHARM] > 100)
        *n = 1;
    else
        *n = 2;

    if (d.sex == 1)
        strcpy(buf, endmodefamily[*m].boy);
    else
        strcpy(buf, endmodefamily[*m].girl);
    *grade = endmodefamily[*m].grade;
    return 0;
}


static int
pip_endingall_purpose( /*�U��*/
char *buf,
int *m, int *n, int *grade,
int mode)
{
    int data;
    int class_;
    int num = 0;

    if (mode == 0)
        data = d.tmp[TMP_HEXP];
    else if (mode == 1)
        data = d.tmp[TMP_MEXP];
    else if (mode == 2)
        data = d.tmp[TMP_SOCIAL];
    else  // mode == 3
        data = d.tmp[TMP_FAMILY];
    if (data > 1000) class_ = 0;
    else if (data > 800) class_ = 1;
    else if (data > 500) class_ = 2;
    else if (data > 300) class_ = 3;
    else class_ = 4;

    data = pip_max_worktime(&num);
    switch (class_)
    {
    case 0:
        if (d.learn[LEARN_CHARACTER] >= 1000)
        {
            *m = 0;
            if (d.learn[LEARN_ETHICS] >= 900)
                *n = 0;
            else if (d.learn[LEARN_ETHICS] >= 600)
                *n = 1;
            else
                *n = 2;
        }
        else
        {
            *m = 1;
            if (d.learn[LEARN_ETHICS] >= 650)
                *n = 0;
            else if (d.learn[LEARN_ETHICS] >= 400)
                *n = 1;
            else
                *n = 2;
        }
        break;

    case 1:
        if (d.state[STATE_BELIEF] > d.learn[LEARN_ETHICS] && d.state[STATE_BELIEF] > d.learn[LEARN_WISDOM])
        {
            *m = 2;
            if (d.learn[LEARN_ETHICS] >= 500)
                *n = 0;
            else if (d.learn[LEARN_ETHICS] >= 250)
                *n = 1;
            else
                *n = 2;
        }
        else if (d.learn[LEARN_ETHICS] > d.state[STATE_BELIEF] && d.learn[LEARN_ETHICS] > d.learn[LEARN_WISDOM])
        {
            *m = 3;
            if (d.learn[LEARN_WISDOM] >= 800)
                *n = 0;
            else if (d.learn[LEARN_WISDOM] >= 600)
                *n = 1;
            else
                *n = 2;
        }
        else
        {
            *m = 4;
            if (d.state[STATE_AFFECT] >= 800)
                *n = 0;
            else if (d.state[STATE_AFFECT] >= 400)
                *n = 1;
            else
                *n = 2;
        }
        break;

    case 2:
        if (d.state[STATE_BELIEF] > d.learn[LEARN_ETHICS] && d.state[STATE_BELIEF] > d.learn[LEARN_WISDOM])
        {
            *m = 5;
            if (d.state[STATE_BELIEF] >= 400)
                *n = 0;
            else if (d.state[STATE_BELIEF] >= 150)
                *n = 1;
            else
                *n = 2;
        }
        else if (d.learn[LEARN_ETHICS] > d.state[STATE_BELIEF] && d.learn[LEARN_ETHICS] > d.learn[LEARN_WISDOM])
        {
            *m = 6;
            if (d.learn[LEARN_WISDOM] >= 700)
                *n = 0;
            else if (d.learn[LEARN_WISDOM] >= 400)
                *n = 1;
            else
                *n = 2;
        }
        else
        {
            *m = 7;
            if (d.state[STATE_AFFECT] >= 800)
                *n = 0;
            else if (d.state[STATE_AFFECT] >= 400)
                *n = 1;
            else
                *n = 2;
        }
        break;

    case 3:
        *m = 8 + num;
        switch (num)
        {
        default:
            *m = 8;
        case 0:
            if (d.learn[LEARN_ETHICS] > 400) *n = 0;
            else if (d.learn[LEARN_ETHICS] > 200) *n = 1;
            else *n = 2;
            break;
        case 1:
            if (d.learn[LEARN_LOVE] > 100) *n = 0;
            else if (d.learn[LEARN_LOVE] > 50) *n = 1;
            else *n = 2;
            break;
        case 2:
            if (d.learn[LEARN_HOMEWORK] > 100) *n = 0;
            else if (d.learn[LEARN_HOMEWORK] > 50) *n = 1;
            else *n = 2;
            break;
        case 3:
            if (d.body[BODY_HP] > 600) *n = 0;
            else if (d.body[BODY_HP] > 300) *n = 1;
            else *n = 2;
            break;
        case 4:
            if (d.learn[LEARN_COOKSKILL] > 200) *n = 0;
            else if (d.learn[LEARN_COOKSKILL] > 100) *n = 1;
            else *n = 2;
            break;
        case 5:
            if ((d.state[STATE_BELIEF] + d.learn[LEARN_ETHICS]) > 600) *n = 0;
            else if ((d.state[STATE_BELIEF] + d.learn[LEARN_ETHICS]) > 200) *n = 1;
            else *n = 2;
            break;
        case 6:
            if (d.learn[LEARN_SPEECH] > 150) *n = 0;
            else if (d.learn[LEARN_SPEECH] > 50) *n = 1;
            else *n = 2;
            break;
        case 7:
            if ((d.body[BODY_HP] + d.body[BODY_WRIST]) > 900) *n = 0;
            else if ((d.body[BODY_HP] + d.body[BODY_WRIST]) > 600) *n = 1;
            else *n = 2;
            break;
        case 8:
        case 10:
            if (d.learn[LEARN_ART] > 250) *n = 0;
            else if (d.learn[LEARN_ART] > 100) *n = 1;
            else *n = 2;
            break;
        case 9:
            if (d.fight[FIGHT_HSKILL] > 250) *n = 0;
            else if (d.fight[FIGHT_HSKILL] > 100) *n = 1;
            else *n = 2;
            break;
        case 11:
            if (d.state[STATE_BELIEF] > 500) *n = 0;
            else if (d.state[STATE_BELIEF] > 200) *n = 1;
            else *n = 2;
            break;
        case 12:
            if (d.learn[LEARN_WISDOM] > 500) *n = 0;
            else if (d.learn[LEARN_WISDOM] > 200) *n = 1;
            else *n = 2;
            break;
        case 13:
        case 15:
            if (d.learn[LEARN_CHARM] > 1000) *n = 0;
            else if (d.learn[LEARN_CHARM] > 500) *n = 1;
            else *n = 2;
            break;
        case 14:
            if (d.learn[LEARN_CHARM] > 700) *n = 0;
            else if (d.learn[LEARN_CHARM] > 300) *n = 1;
            else *n = 2;
            break;
        }
        break;
    case 4:
        *m = 24 + num;
        switch (num)
        {
        default:
            *m = 24;
        case 0:
            if (d.relation > 100) *n = 0;
            else if (d.relation > 50) *n = 1;
            else *n = 2;
            break;
        case 1:
        case 2:
            if (d.body[BODY_HP] > 400) *n = 0;
            else if (d.body[BODY_HP] > 150) *n = 1;
            else *n = 2;
            break;
        case 3:
        case 9:
        case 10:
            if (d.body[BODY_HP] > 600) *n = 0;
            else if (d.body[BODY_HP] > 300) *n = 1;
            else *n = 2;
            break;
        case 4:
            if (d.learn[LEARN_COOKSKILL] > 150) *n = 0;
            else if (d.learn[LEARN_COOKSKILL] > 80) *n = 1;
            else *n = 2;
            break;
        case 5:
            if ((d.state[STATE_BELIEF] + d.learn[LEARN_ETHICS]) > 600) *n = 0;
            else if ((d.state[STATE_BELIEF] + d.learn[LEARN_ETHICS]) > 200) *n = 1;
            else *n = 2;
            break;
        case 6:
            if (d.learn[LEARN_SPEECH] > 150) *n = 0;
            else if (d.learn[LEARN_SPEECH] > 50) *n = 1;
            else *n = 2;
            break;
        case 7:
            if ((d.body[BODY_HP] + d.body[BODY_WRIST]) > 700) *n = 0;
            else if ((d.body[BODY_HP] + d.body[BODY_WRIST]) > 300) *n = 1;
            else *n = 2;
            break;
        case 8:
            if (d.learn[LEARN_ART] > 100) *n = 0;
            else if (d.learn[LEARN_ART] > 50) *n = 1;
            else *n = 2;
            break;
        case 11:
            if (d.body[BODY_HP] > 300) *n = 0;
            else if (d.body[BODY_HP] > 150) *n = 1;
            else *n = 2;
            break;
        case 12:
            if (d.learn[LEARN_SPEECH] > 100) *n = 0;
            else if (d.learn[LEARN_SPEECH] > 40) *n = 1;
            else *n = 2;
            break;
        case 13:
        case 15:
            if (d.learn[LEARN_CHARM] > 1000) *n = 0;
            else if (d.learn[LEARN_CHARM] > 500) *n = 1;
            else *n = 2;
            break;
        case 14:
            if (d.learn[LEARN_CHARM] > 700) *n = 0;
            else if (d.learn[LEARN_CHARM] > 300) *n = 1;
            else *n = 2;
            break;
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
pip_endingart( /*���N*/
char *buf,
int *m, int *n, int *grade)
{
    if (d.learn[LEARN_SPEECH] >= 100)
    {
        *m = 0;
        if (d.body[BODY_HP] >= 300 && d.state[STATE_AFFECT] >= 350)
            *n = 0;
        else if (d.body[BODY_HP] >= 250 && d.state[STATE_AFFECT] >= 300)
            *n = 1;
        else
            *n = 2;
    }
    else if (d.learn[LEARN_WISDOM] >= 400)
    {
        *m = 1;
        if (d.state[STATE_AFFECT] >= 500)
            *n = 0;
        else if (d.state[STATE_AFFECT] >= 450)
            *n = 1;
        else
            *n = 2;
    }
    else if (d.class_[I] >= d.class_[J])
    {
        *m = 2;
        if (d.state[STATE_AFFECT] >= 350)
            *n = 0;
        else if (d.state[STATE_AFFECT] >= 300)
            *n = 1;
        else
            *n = 2;
    }
    else
    {
        *m = 3;
        if (d.state[STATE_AFFECT] >= 200 && d.body[BODY_HP] > 150)
            *n = 0;
        else if (d.state[STATE_AFFECT] >= 180 && d.body[BODY_HP] > 150)
            *n = 1;
        else
            *n = 2;
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
    *num = -1;
    for (int i = 0; i < COUNTOF(d.work); i++)
    {
        if (d.work[i] > data)
        {
            data = d.work[i];
            *num = i;
        }
    }
    return data;
}

static int pip_game_over(
int endgrade)
{
    long gradebasic;
    long gradeall;

    gradebasic = (d.body[BODY_MAXHP] + d.body[BODY_WRIST] + d.learn[LEARN_WISDOM] + d.learn[LEARN_CHARACTER] + d.learn[LEARN_CHARM] + d.learn[LEARN_ETHICS] + d.state[STATE_BELIEF] + d.state[STATE_AFFECT]) / 10 - d.state[STATE_OFFENSE];
    clrchyiuan(1, b_lines);
    gradeall = gradebasic + endgrade;
    move(8, (d_cols>>1) + 17);
    outs("\x1b[1;36m�P�±z�������" NICKNAME "�p�����C��.....\x1b[0m");
    move(10, (d_cols>>1) + 17);
    outs("\x1b[1;37m�g�L�t�έp�⪺���G�G\x1b[0m");
    move(12, (d_cols>>1) + 17);
    prints("\x1b[1;36m�z���p�� \x1b[37m%s \x1b[36m�`�o���� \x1b[1;5;33m%ld \x1b[0m", d.name, gradeall);
    return gradeall;
}

static int pip_divine(void) /*�e�R�v�ӳX*/
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
    outs("\x1b[1;33;5m�n�n�n...\x1b[0;1;37m��M�ǨӰ}�}���V���n.........\x1b[0m");
    vmsg("�h�@�@�O�֧a......");
    clrchyiuan(6, b_lines - 6);
    move(10, (d_cols>>1) + 14);
    outs("\x1b[1;37;46m    ��ӬO���C�|�����e�R�v�ӳX�F.......    \x1b[0m");
    vmsg("�}�����L�i�ӧa....");
    if (d.thing[THING_MONEY] >= money)
    {
        randvalue = random() % 5;
        sprintf(buf, "�A�n�e�R��? �n��%ld����...[y/N]: ", money);
        getdata(12, 14, buf, ans, 2, 1, 0);
        if (ans[0] == 'y' || ans[0] == 'Y')
        {
            pip_ending_decide(endbuf1, endbuf2, endbuf3, &endmode, &endgrade);
            if (randvalue == 0)
                sprintf(buf, "\x1b[1;37m  �A���p��%s�H��i�઺�����O%s  \x1b[m", d.name, endmodemagic[2+random()%5].girl);
            else if (randvalue == 1)
                sprintf(buf, "\x1b[1;37m  �A���p��%s�H��i�઺�����O%s  \x1b[m", d.name, endmodecombat[2+random()%6].girl);
            else if (randvalue == 2)
                sprintf(buf, "\x1b[1;37m  �A���p��%s�H��i�઺�����O%s  \x1b[m", d.name, endmodeall_purpose[6+random()%15].girl);
            else if (randvalue == 3)
                sprintf(buf, "\x1b[1;37m  �A���p��%s�H��i�઺�����O%s  \x1b[m", d.name, endmodeart[2+random()%6].girl);
            else if (randvalue == 4)
                sprintf(buf, "\x1b[1;37m  �A���p��%s�H��i�઺�����O%s  \x1b[m", d.name, endbuf1);
            d.thing[THING_MONEY] -= money;
            clrchyiuan(6, b_lines - 6);
            move(10, (d_cols>>1) + 14);
            outs("\x1b[1;33m�b�ڥe�R���G�ݨ�....\x1b[m");
            move(12, (d_cols>>1) + 14);
            outs(buf);
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

static int
pip_money(void)
{
    char buf[100], ans[10];

    int money = -1;
    if (!d.name[0] || d.death) return 0;
    clrchyiuan(6, b_lines - 6);
    /* move(12, 0);
    clrtobot();*/
    prints("�A���W�� %d ���I�q���ơA���� %d ��\n", cuser.request, d.thing[THING_MONEY]);
    outs("\n�@�����@�d������!!\n");
    while (money < 0 || money > cuser.request)
    {
        getdata(10, 0, "�n���h�֦�? ", ans, 10, LCECHO, 0);
        if (!ans[0]) return 0;
        money = atol(ans);
    }
    sprintf(buf, "�O�_�n�ഫ %d �� �� %d ����? [y/N]: ", money, money*1000);
    getdata(11, 0, buf, ans, 3, LCECHO, 0);
    if (ans[0] == 'y' || ans[0] == 'Y')
    {
        ACCT acct;
        acct_load(&acct, cuser.userid);
        /* demoney(money);*/
        d.thing[THING_MONEY] += (money * 1000);
        cuser.request -= money;
        acct.request = cuser.request;
        acct_save(&acct);
        pip_write_file(&d, cuser.userid);
        sprintf(buf, "�A���W�� %d ���I�q���ơA���� %d ��", cuser.request, d.thing[THING_MONEY]);
    }
    else
        sprintf(buf, "����.....");

    vmsg(buf);
    return 0;
}

static int pip_query(void)  /*���X�p��*/
{
    int id;
    char genbuf[STRSIZE];

    vs_bar("���X�P��");
    usercomplete(msg_uid, genbuf);
    if (genbuf[0])
    {
        move(2, 0);
        if ( ( id = acct_userno(genbuf) ) )
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

static int
pip_read(
const char *userid)
{
    int pc1, age = 0;
    struct chicken ck;

    if (pip_read_file(&ck, userid))
    {
        age = ck.bbtime / 60 / 30;

        move(1, 0);
        clrtobot();
        prints("�o�O%s�i���p���G\n", userid);

        if (ck.death == 0)
        {
            prints("\x1b[1;32mName�G%-10s\x1b[m  �ͤ�G%2d�~%2d��%2d��   �~�֡G%2d��  ���A�G%s  �����G%d\n"
                   "�ͩR�G%3d/%-3d  �ּ֡G%-4d  ���N�G%-4d  ���G%-4d  ���z�G%-4d  �魫�G%-4d\n"
                   "�j�ɤY�G%-4d   �����G%-4d  �s���G%-4d  �h�ҡG%-4d  żż�G%-4d  �f��G%-4d\n",
                   ck.name, ck.year - 11, ck.month, ck.day, age, pip_age_name(age), ck.thing[THING_MONEY],
                   ck.body[BODY_HP], ck.body[BODY_MAXHP], ck.state[STATE_HAPPY], ck.state[STATE_SATISFY], ck.learn[LEARN_CHARACTER], ck.learn[LEARN_WISDOM], ck.body[BODY_WEIGHT],
                   ck.eat[EAT_BIGHP], ck.eat[EAT_FOOD], ck.eat[EAT_COOKIE], ck.body[BODY_TIRED], ck.body[BODY_SHIT], ck.body[BODY_SICK]);

            move(5, 0);
            pip_show_age_pic(age, ck.body[BODY_WEIGHT]);

            move(b_lines - 5, 0);
            if (ck.body[BODY_SHIT] <= 0) outs("�ܰ��b..");
            else if (ck.body[BODY_SHIT] <= 40) { }
            else if (ck.body[BODY_SHIT] < 60) outs("��䪺..");
            else if (ck.body[BODY_SHIT] < 80) outs("�n���..");
            else if (ck.body[BODY_SHIT] < 100) outs("\x1b[1;34m�֯䦺�F..\x1b[m");
            else { outs("\x1b[1;31m�䦺�F..\x1b[m"); return -1; }

            pc1 = ck.body[BODY_HP] * 100 / ck.body[BODY_MAXHP];
            if (pc1 <= 0) { outs("�j���F.."); return -1; }
            else if (pc1 < 20) outs("\x1b[1;35m�����L�O��.�־j���F.\x1b[m");
            else if (pc1 < 40) outs("��O���Ӱ�..�Q�Y�I�F��..");
            else if (pc1 < 80) { }
            else if (pc1 < 100) outs("���{�l��������O..");
            else outs("\x1b[1;34m�ּ����F..\x1b[m");

            pc1 = ck.body[BODY_TIRED];
            if (pc1 < 20) outs("�믫���]��..");
            else if (pc1 < 60) { }
            else if (pc1 < 80) outs("\x1b[1;34m���I�p��..\x1b[m");
            else if (pc1 < 100) { outs("\x1b[1;31m�n�ֳ�A�֤���F..\x1b[m"); }
            else { outs("�֦��F..."); return -1; }

            pc1 = 60 + 10 * age;
            if (ck.body[BODY_WEIGHT] < (pc1 - 50)) { outs("�G���F.."); return -1; }
            else if (ck.body[BODY_WEIGHT] <= (pc1 - 30)) outs("�ӽG�F..");
            else if (ck.body[BODY_WEIGHT] <= (pc1 - 10)) outs("���I�p�G..");
            else if (ck.body[BODY_WEIGHT] < (pc1 + 10)) { }
            else if (ck.body[BODY_WEIGHT] < (pc1 + 30)) outs("���I�p�D..");
            else if (ck.body[BODY_WEIGHT] < (pc1 + 50)) outs("�ӭD�F..");
            else { outs("�D���F..."); return -1; }

            if (ck.body[BODY_SICK] < 50) { }
            else if (ck.body[BODY_SICK] < 75) outs("\x1b[1;34m�ͯf�F..\x1b[m");
            else if (ck.body[BODY_SICK] < 100) { outs("\x1b[1;31m�f��!!..\x1b[m"); }
            else { outs("�f���F.!."); return -1; }

            pc1 = ck.state[STATE_HAPPY];
            if (pc1 < 20) outs("\x1b[1;31m�ܤ��ּ�..\x1b[m");
            else if (pc1 < 40) outs("���ּ�..");
            else if (pc1 < 80) { }
            else if (pc1 < 95) outs("�ּ�..");
            else outs("�ܧּ�..");

            pc1 = ck.state[STATE_SATISFY];
            if (pc1 < 40) outs("\x1b[ck.3;1m������..\x1b[m");
            else if (pc1 < 80) { }
            else if (pc1 < 95) outs("����..");
            else outs("�ܺ���..");
        }
        else if (ck.death == 1)
        {
            show_die_pic(2);
            move(14, (d_cols>>1) + 20);
            outs("�i�����p����I�s�v�F");
        }
        else if (ck.death == 2)
        {
            show_die_pic(3);
        }
        else if (ck.death == 3)
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

    return 0;
}

/*---------------------------------------------------------------------------*/
/* �t�ο��:�ӤH���  �p�����  �S�O�A��                                     */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static int
pip_system_freepip(void)
{
    char buf[256];
    clrchyiuan(b_lines - 2, b_lines);
    getdata(B_LINES_REF - 1, 1, "�u���n��ͶܡH(y/N): ", buf, 2, 1, 0);
    if (buf[0] != 'y' && buf[0] != 'Y') return 0;
    sprintf(buf, "%s �Q���ߪ� %s �ᱼ�F~", d.name, cuser.userid);
    vmsg(buf);
    d.death = 2;
    pipdie("\x1b[1;31m�Q���ߥ��:~~\x1b[0m", 2);
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
    prints("\x1b[1;44m  �A�ȶ���  \x1b[46m[1]�R�W�j�v [2]�ܩʤ�N [3]�����]��                                %*s\x1b[0m", d_cols, "");
    pipkey = vkey();

    switch (pipkey)
    {
    case '1':
        move(b_lines - 1, 0);
        clrtobot();
        getdata(B_LINES_REF - 1, 1, "���p�����s���Ӧn�W�r�G ", buf, 11, DOECHO, NULL);
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
            sprintf(buf, "\x1b[1;37m%s %-11s��p�� [%s] ��W�� [%s] \x1b[0m\n", Cdate(&now), cuser.userid, oldname, d.name);
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
            outs("\x1b[1;33m�N�p����\x1b[32m��\x1b[33m�ܩʦ�\x1b[35m��\x1b[33m���ܡH \x1b[37m[y/N]\x1b[0m");
        }
        else
        {
            oldchoice = 1; /*��-->��*/
            move(b_lines - 1, 0);
            outs("\x1b[1;33m�N�p����\x1b[35m��\x1b[33m�ܩʦ�\x1b[35m��\x1b[33m���ܡH \x1b[37m[y/N]\x1b[0m");
        }
        move(b_lines, 0);
        prints("\x1b[1;44m  �A�ȶ���  \x1b[46m[1]�R�W�j�v [2]�ܩʤ�N [3]�����]��                                %*s\x1b[0m", d_cols, "");
        pipkey = vkey();
        if (pipkey == 'Y' || pipkey == 'y')
        {
            /*��W�O��*/
            now = time(0);
            if (d.sex == 1)
                sprintf(buf, "\x1b[1;37m%s %-11s��p�� [%s] �ѡ��ܩʦ���F\x1b[0m\n", Cdate(&now), cuser.userid, d.name);
            else
                sprintf(buf, "\x1b[1;37m%s %-11s��p�� [%s] �ѡ��ܩʦ���F\x1b[0m\n", Cdate(&now), cuser.userid, d.name);
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
            outs("\x1b[1;33m�N�p���C���令\x1b[32m[��20������]\x1b[33m? \x1b[37m[y/N]\x1b[0m");
            sprintf(buf, "�p���C���]�w��[��20������]..");
        }
        else
        {
            oldchoice -= 3; /*��-->�S��*/
            move(b_lines - 1, 0);
            outs("\x1b[1;33m�N�p���C���令\x1b[32m[�S��20������]\x1b[33m? \x1b[37m[y/N]\x1b[0m");
            sprintf(buf, "�p���C���]�w��[�S��20������]..");
        }
        move(b_lines, 0);
        prints("\x1b[1;44m  �A�ȶ���  \x1b[46m[1]�R�W�j�v [2]�ܩʤ�N [3]�����]��                                %*s\x1b[0m", d_cols, "");
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
    GCC_UNUSED const char *userid;

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
pip_data_list(  /*�ݤp���ӤH�ԲӸ��*/
const char *userid)
{
    char inbuf1[20];
    char inbuf2[20];
    int tm;
    int pipkey;
    int page = 1;
    struct chicken chicken;

    if (!pip_read_file(&chicken, userid))
    {
        vmsg("�ڨS���i�p���� !");
        return 0;
    }


//  tm=(time(0)-start_time+chicken.bbtime)/60/30;
    tm = chicken.bbtime / 60 / 30;

    clear();
    move(1, 0);
    outs_centered("       \x1b[1;33m����������������������������������������\x1b[m\n");
    outs_centered("       \x1b[0;37m������  ���� ��   �������������� ��   ��\x1b[m\n");
    outs_centered("       \x1b[1;37m������  ��������  ��  ����    ������  ��\x1b[m\n");
    outs_centered("       \x1b[1;34m��������������������  ����    ����������\x1b[32m......................\x1b[m");
    do
    {
        clrchyiuan(5, b_lines);
        switch (page)
        {
        case 1:
            move(5, 0);
            prints_centered("\x1b[1;31m �~�t\x1b[41;37m �򥻸�� \x1b[0;1;31m�u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w��\x1b[m\n");

            prints_centered("\x1b[1;31m �x\x1b[33m�̩m    �W :\x1b[37m %-10s \x1b[33m�̥�    �� :\x1b[37m %02d/%02d/%02d   \x1b[33m�̦~    �� :\x1b[37m %-2d         \x1b[31m�x\x1b[m\n",
                    chicken.name, (chicken.year) % 100, chicken.month, chicken.day, tm);

            sprintf(inbuf1, "%d%s/%d%s", chicken.body[BODY_HP] > 1000 ? chicken.body[BODY_HP] / 1000 : chicken.body[BODY_HP], chicken.body[BODY_HP] > 1000 ? "K" : "", chicken.body[BODY_MAXHP] > 1000 ? chicken.body[BODY_MAXHP] / 1000 : chicken.body[BODY_MAXHP], chicken.body[BODY_MAXHP] > 1000 ? "K" : "");
            sprintf(inbuf2, "%d%s/%d%s", chicken.fight[FIGHT_MP] > 1000 ? chicken.fight[FIGHT_MP] / 1000 : chicken.fight[FIGHT_MP], chicken.fight[FIGHT_MP] > 1000 ? "K" : "", chicken.fight[FIGHT_MAXMP] > 1000 ? chicken.fight[FIGHT_MAXMP] / 1000 : chicken.fight[FIGHT_MAXMP], chicken.fight[FIGHT_MAXMP] > 1000 ? "K" : "");

            prints_centered("\x1b[1;31m �x\x1b[33m����    �� :\x1b[37m %-5d(�̧J)\x1b[33m����    �O :\x1b[37m %-11s\x1b[33m�̪k    �O :\x1b[37m %-11s\x1b[31m�x\x1b[m\n",
                    chicken.body[BODY_WEIGHT], inbuf1, inbuf2);

            prints_centered("\x1b[1;31m �x\x1b[33m�̯h    �� :\x1b[37m %-3d        \x1b[33m�̯f    �� :\x1b[37m %-3d        \x1b[33m��ż    ż :\x1b[37m %-3d        \x1b[31m�x\x1b[m\n",
                    chicken.body[BODY_TIRED], chicken.body[BODY_SICK], chicken.body[BODY_SHIT]);

            prints_centered("\x1b[1;31m �x\x1b[33m�̵�    �O :\x1b[37m %-7d    \x1b[33m�̿ˤl���Y :\x1b[37m %-7d    \x1b[33m�̪�    �� :\x1b[37m %-11d\x1b[31m�x\x1b[m\n",
                    chicken.body[BODY_WRIST], chicken.relation, chicken.thing[THING_MONEY]);

            prints_centered("\x1b[1;31m �u�t\x1b[41;37m ��O��� \x1b[0;1;31m�u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t\x1b[m\n");

            prints_centered("\x1b[1;31m �x\x1b[33m�̮�    �� :\x1b[37m %-10d \x1b[33m�̴�    �O :\x1b[37m %-10d \x1b[33m�̷R    �� :\x1b[37m %-10d \x1b[31m�x\x1b[m\n",
                    chicken.learn[LEARN_CHARACTER], chicken.learn[LEARN_WISDOM], chicken.learn[LEARN_LOVE]);

            prints_centered("\x1b[1;31m �x\x1b[33m����    �N :\x1b[37m %-10d \x1b[33m�̹D    �w :\x1b[37m %-10d \x1b[33m�̮a    �� :\x1b[37m %-10d \x1b[31m�x\x1b[m\n",
                    chicken.learn[LEARN_ART], chicken.learn[LEARN_ETHICS], chicken.learn[LEARN_HOMEWORK]);

            prints_centered("\x1b[1;31m �x\x1b[33m��§    �� :\x1b[37m %-10d \x1b[33m����    �� :\x1b[37m %-10d \x1b[33m�̲i    �� :\x1b[37m %-10d \x1b[31m�x\x1b[m\n",
                    chicken.learn[LEARN_MANNERS], chicken.learn[LEARN_SPEECH], chicken.learn[LEARN_COOKSKILL]);

            prints_centered("\x1b[1;31m �u�t\x1b[41;37m ���A��� \x1b[0;1;31m�u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t\x1b[m\n");

            prints_centered("\x1b[1;31m �x\x1b[33m�̧�    �� :\x1b[37m %-10d \x1b[33m�̺�    �N :\x1b[37m %-10d \x1b[33m�̤H    �� :\x1b[37m %-10d \x1b[31m�x\x1b[m\n",
                    chicken.state[STATE_HAPPY], chicken.state[STATE_SATISFY], chicken.learn[LEARN_TOMAN]);

            prints_centered("\x1b[1;31m �x\x1b[33m�̾y    �O :\x1b[37m %-10d \x1b[33m�̫i    �� :\x1b[37m %-10d \x1b[33m�̫H    �� :\x1b[37m %-10d \x1b[31m�x\x1b[m\n",
                    chicken.learn[LEARN_CHARM], chicken.learn[LEARN_BRAVE], chicken.state[STATE_BELIEF]);

            prints_centered("\x1b[1;31m �x\x1b[33m�̸o    �^ :\x1b[37m %-10d \x1b[33m�̷P    �� :\x1b[37m %-10d \x1b[33m            \x1b[37m            \x1b[31m�x\x1b[m\n",
                    chicken.state[STATE_OFFENSE], chicken.state[STATE_AFFECT]);

            prints_centered("\x1b[1;31m �u�t\x1b[41;37m ������� \x1b[0;1;31m�u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t\x1b[m\n");

            prints_centered("\x1b[1;31m �x\x1b[33m�̪������ :\x1b[37m %-10d \x1b[33m�̾԰����� :\x1b[37m %-10d \x1b[33m���]�k���� :\x1b[37m %-10d \x1b[31m�x\x1b[m\n",
                    chicken.tmp[TMP_SOCIAL], chicken.tmp[TMP_HEXP], chicken.tmp[TMP_MEXP]);

            prints_centered("\x1b[1;31m �x\x1b[33m�̮a�Ƶ��� :\x1b[37m %-10d                                                 \x1b[31m�x\x1b[m\n",
                    chicken.tmp[TMP_FAMILY]);

            prints_centered("\x1b[1;31m ���w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w��\x1b[m\n");

            move(b_lines - 1, 0);
            prints_centered("                                                              \x1b[1;36m�Ĥ@��\x1b[37m/\x1b[36m�@�G��\x1b[m\n");
            break;

        case 2:
            move(5, 0);
            prints_centered("\x1b[1;31m �~�t\x1b[41;37m ���~��� \x1b[0;1;31m�u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w��\x1b[m\n");

            prints_centered("\x1b[1;31m �x\x1b[33m�̭�    �� :\x1b[37m %-10d \x1b[33m�̹s    �� :\x1b[37m %-10d \x1b[33m�̤j �� �Y :\x1b[37m %-10d \x1b[31m�x\x1b[m\n",
                    chicken.eat[EAT_FOOD], chicken.eat[EAT_COOKIE], chicken.eat[EAT_BIGHP]);

            prints_centered("\x1b[1;31m �x\x1b[33m���F    �� :\x1b[37m %-10d \x1b[33m�̮�    �� :\x1b[37m %-10d \x1b[33m�̪�    �� :\x1b[37m %-10d \x1b[31m�x\x1b[m\n",
                    chicken.eat[EAT_MEDICINE], chicken.thing[THING_BOOK], chicken.thing[THING_PLAYTOOL]);

            prints_centered("\x1b[1;31m �u�t\x1b[41;37m �C����� \x1b[0;1;31m�u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t\x1b[m\n");

            prints_centered("\x1b[1;31m �x\x1b[33m�̲q �� Ĺ :\x1b[37m %-10d \x1b[33m�̲q �� �� :\x1b[37m %-10d                         \x1b[31m�x\x1b[m\n",
                    chicken.winn, chicken.losee);

            prints_centered("\x1b[1;31m �u�t\x1b[41;37m �Z�O��� \x1b[0;1;31m�u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t\x1b[m\n");

            prints_centered("\x1b[1;31m �x\x1b[33m�̧� �� �O :\x1b[37m %-10d \x1b[33m�̨� �m �O :\x1b[37m %-10d \x1b[33m�̳t �� �� :\x1b[37m %-10d \x1b[31m�x\x1b[m\n",
                    chicken.fight[FIGHT_ATTACK], chicken.fight[FIGHT_RESIST], chicken.fight[FIGHT_SPEED]);
            prints_centered("\x1b[1;31m �x\x1b[33m�̧��]��O :\x1b[37m %-10d \x1b[33m�̾԰��޳N :\x1b[37m %-10d \x1b[33m���]�k�޳N :\x1b[37m %-10d \x1b[31m�x\x1b[m\n",
                    chicken.fight[FIGHT_MRESIST], chicken.fight[FIGHT_HSKILL], chicken.fight[FIGHT_MSKILL]);

            prints_centered("\x1b[1;31m �x\x1b[33m���Y���˳� :\x1b[37m %-10s \x1b[33m�̥k��˳� :\x1b[37m %-10s \x1b[33m�̥���˳� :\x1b[37m %-10s \x1b[31m�x\x1b[m\n",
                    headlist[chicken.weapon[WEAPON_HEAD]].name, rhandlist[chicken.weapon[WEAPON_RHAND]].name, lhandlist[chicken.weapon[WEAPON_LHAND]].name);

            prints_centered("\x1b[1;31m �x\x1b[33m�̨���˳� :\x1b[37m %-10s \x1b[33m�̸}���˳� :\x1b[37m %-10s \x1b[33m            \x1b[37m            \x1b[31m�x\x1b[m\n",
                    bodylist[chicken.weapon[WEAPON_BODY]].name, footlist[chicken.weapon[WEAPON_FOOT]].name);

            prints_centered("\x1b[1;31m �u�t\x1b[41;37m ���Ÿ�� \x1b[0;1;31m�u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t\x1b[m\n");

            prints_centered("\x1b[1;31m �x\x1b[33m�̵�    �� :\x1b[37m %-10d \x1b[33m�̸g �� �� :\x1b[37m %-10d \x1b[33m�̤U���ɯ� :\x1b[37m %-10d \x1b[31m�x\x1b[m\n",
                    chicken.level, chicken.exp, twice(d.level, 10000, 100));

            prints_centered("\x1b[1;31m ���w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w��\x1b[m\n");

            move(b_lines - 1, 0);
            prints_centered("                                                              \x1b[1;36m�ĤG��\x1b[37m/\x1b[36m�@�G��\x1b[m\n");
            break;
        }
        move(b_lines, 0);
        prints("\x1b[1;44;37m  ��ƿ��  \x1b[46m  [��/PAGE UP]���W�@�� [��/PAGE DOWN]���U�@�� [Q]���}             %*s\x1b[m", d_cols, "");
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

static int pip_fight_main(int n, const struct playrule list[], int mode);

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
static int
get_man(
int class_, int mob, int plus)
{
    int lucky, man;
    lucky = random() % (class_ * 5 + 5);
    if (lucky <= (class_*2 + 2))
    {
        man = random() % mob + plus;
    }
    else if (lucky <= (class_*4 + 4))
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
    int class_;
    int man, lucky;
    char ans;
    class_ = BMAX((d.body[BODY_MAXHP] * 30 + d.fight[FIGHT_MAXMP] * 20 + d.fight[FIGHT_ATTACK] * 20 + d.fight[FIGHT_RESIST] * 15 + d.tmp[TMP_MEXP] * 5 + d.tmp[TMP_HEXP] * 5 + d.fight[FIGHT_SPEED] * 10) / 8500, 0);

    move(b_lines - 1, 0);
    prints("\x1b[1;44;37m �ϰ� \x1b[46m[1]�����}�]  [2]�_��B��  [3]�j�N���  [4]�H�u�q  [5]�a������            %*s\x1b[m\n", d_cols, "");
    prints("\x1b[1;44;37m ��� \x1b[46m                                                                 %*s[Q]�^�a \x1b[m", d_cols, "");
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

    while (d.body[BODY_HP] > 0)
    {
        move(b_lines - 1, 0);
        clrtoeol();
        move(b_lines, 0);
        prints("\x1b[1;44;37m ��V \x1b[46m[R]�^�a [F]���� (E/W/S/N)�F��n�_                                        %*s\x1b[m", d_cols, "");
        ans = vkey();
        if (ans == 'r' || ans == 'R')
            return 0;

        lucky = random() % 2000;
        if (ans != 'e' && ans != 'w' && ans != 's' && ans != 'n' && ans != 'E' && ans != 'W' && ans != 'S' && ans != 'N' &&
            ans != 'F' && ans != 'f')
            continue;
        if (ans == 'f' || ans == 'F')
            pip_basic_feed();
        else if (lucky >= 1999)
        {
            vmsg("�J��j�]���աI");
        }
        else if (lucky >= 1000)
        {
            vmsg("�S�o�ͥ���ơI");
        }
        else
        {
            if (class_ < 20)
            {
                man = get_man(class_, mob[1+class_][0], mob[1+class_][1]);
            }
            else
            {
                man = get_man(class_, mob[21][0], mob[21][1]);
            }
            pip_fight_bad(man);
        }
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
/* �԰��԰��禡                                                              */
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
    int dinjure = 0;            /*�p���ˮ`�O*/
    int minjure = 0;            /*���ˮ`�O*/
    int dresistmode = 0;        /*�p���[�j���m*/
    int mresistmode = 0;        /*���[�j���m*/
    int oldhexp;                /*���԰��e�氫�g��*/
    int oldmexp;                /*���԰��e�]�k�g��*/
    int oldbrave;               /*���԰��e�i��*/
    int oldhskill;              /*���԰��e�԰��޳N*/
    int oldmskill;              /*���԰��e�]�k�޳N*/
    GCC_UNUSED int oldethics;   /*���԰��e�D�w*/
    int oldmoney;               /*���԰��e����*/
    int oldtired;
    int oldhp;
    int oldexp;
    int winorlose = 0;          /*1:you win 0:you loss*/

    /*�H�����ͤH�� �åB�s�n�԰��e���@�Ǽƭ�*/
    oldhexp = d.tmp[TMP_HEXP];
    oldmexp = d.tmp[TMP_MEXP];
    oldbrave = d.learn[LEARN_BRAVE];
    oldhskill = d.fight[FIGHT_HSKILL];
    oldmskill = d.fight[FIGHT_MSKILL];
    oldethics = d.learn[LEARN_ETHICS];
    oldmoney = d.thing[THING_MONEY];
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
        m.maxhp = d.body[BODY_MAXHP] * (80 + random() % 50) / 100 + 20;;
        m.hp = m.maxhp - random() % 10 + 20;
        m.maxmp = d.fight[FIGHT_MAXMP] * (80 + random() % 50) / 100 + 10;
        m.mp = m.maxmp - random() % 20 + 10;
        m.speed = d.fight[FIGHT_SPEED] * (80 + random() % 50) / 100 + 10;
        m.attack = d.fight[FIGHT_ATTACK] * (80 + random() % 50) / 100 + 10;
        m.resist = d.fight[FIGHT_RESIST] * (80 + random() % 50) / 100 + 10;
        m.money = 0;
        m.death = 0;
    }
    /*d.body[BODY_TIRED]+=random()%(n+1)/4+2;*/
    /*d.body[BODY_SHIT]+=random()%(n+1)/4+2;*/
    do
    {
        if (m.hp <= 0) /*�ĤH�����F*/
        {
            m.hp = 0;
            d.thing[THING_MONEY] += m.money;
            m.death = 1;
            d.learn[LEARN_BRAVE] += random() % 4 + 3;
        }
        if (d.body[BODY_HP] <= 0 || d.body[BODY_TIRED] >= 100)  /*�p���}�`*/
        {
            if (mode == 1)
            {
                d.body[BODY_HP] = 0;
                d.body[BODY_TIRED] = 0;
                d.death = 1;
            }
            else
            {
                d.body[BODY_HP] = d.body[BODY_MAXHP] / 3 + 10;
                d.tmp[TMP_HEXP] -= random() % 3 + 2;
                d.tmp[TMP_MEXP] -= random() % 3 + 2;
                d.body[BODY_TIRED] = 50;
                d.death = 1;
            }
        }
        clear();
        /*vs_head("�q�l�i�p��", BoardName);*/
        move(0, 0);
        if (d.sex == 1)
            prints("\x1b[1;41m  " NICKNAME PIPNAME " �� \x1b[32m�� \x1b[37m%-10s         %*s\x1b[0m", d.name, 50 + d_cols - ((int)(unsigned int)sizeof(NICKNAME PIPNAME) - 1), "");
        else if (d.sex == 2)
            prints("\x1b[1;41m  " NICKNAME PIPNAME " �� \x1b[33m�� \x1b[37m%-10s         %*s\x1b[0m", d.name, 50 + d_cols - ((int)(unsigned int)sizeof(NICKNAME PIPNAME) - 1), "");
        else
            prints("\x1b[1;41m  " NICKNAME PIPNAME " �� \x1b[34m�H \x1b[37m%-10s         %*s\x1b[0m", d.name, 50 + d_cols - ((int)(unsigned int)sizeof(NICKNAME PIPNAME) - 1), "");
        move(6, 0);
        if (mode == 1)
            show_badman_pic(m.map/*n*/);
        move(1, 0);
        prints_centered("\x1b[1;31m�z�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�{\x1b[m");
        move(2, 0);
        /* lucky���ӷ�color��*/
        if (d.body[BODY_TIRED] >= 80)
            lucky = 31;
        else if (d.body[BODY_TIRED] >= 60 && d.body[BODY_TIRED] < 80)
            lucky = 33;
        else
            lucky = 37;
        sprintf(inbuf1, "%d%s/%d%s", d.body[BODY_HP] > 1000 ? d.body[BODY_HP] / 1000 : d.body[BODY_HP], d.body[BODY_HP] > 1000 ? "K" : "", d.body[BODY_MAXHP] > 1000 ? d.body[BODY_MAXHP] / 1000 : d.body[BODY_MAXHP], d.body[BODY_MAXHP] > 1000 ? "K" : "");
        sprintf(inbuf2, "%d%s/%d%s", d.fight[FIGHT_MP] > 1000 ? d.fight[FIGHT_MP] / 1000 : d.fight[FIGHT_MP], d.fight[FIGHT_MP] > 1000 ? "K" : "", d.fight[FIGHT_MAXMP] > 1000 ? d.fight[FIGHT_MAXMP] / 1000 : d.fight[FIGHT_MAXMP], d.fight[FIGHT_MAXMP] > 1000 ? "K" : "");

        prints_centered("\x1b[1;31m�x\x1b[33m��  �R:\x1b[37m%-12s\x1b[33m�k  �O:\x1b[37m%-12s\x1b[33m�h  ��:\x1b[%dm%-12d\x1b[33m��  ��:\x1b[37m%-10d\x1b[31m�x\x1b[m",
                inbuf1, inbuf2, lucky, d.body[BODY_TIRED], d.thing[THING_MONEY]);
        move(3, 0);
        prints_centered("\x1b[1;31m�x\x1b[33m��  ��:\x1b[37m%-10d  \x1b[33m��  �m:\x1b[37m%-10d  \x1b[33m�t  ��:\x1b[37m%-10d  \x1b[33m�g  ��:\x1b[37m%-10d\x1b[31m�x\x1b[m",
                d.fight[FIGHT_ATTACK], d.fight[FIGHT_RESIST], d.fight[FIGHT_SPEED], d.exp);
        move(4, 0);
        prints_centered("\x1b[1;31m�x\x1b[33m��  ��:\x1b[37m%-5d       \x1b[33m�j�ɤY:\x1b[37m%-5d       \x1b[33m�s  ��:\x1b[37m%-5d       \x1b[33m�F  ��:\x1b[37m%-5d     \x1b[31m�x\x1b[m",
                d.eat[EAT_FOOD], d.eat[EAT_BIGHP], d.eat[EAT_COOKIE], d.eat[EAT_MEDICINE]);
        move(5, 0);
        prints_centered("\x1b[1;31m�|�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�}\x1b[m");
        move(b_lines - 4, 0);
        prints_centered("\x1b[1;34m�z�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�{\x1b[m");
        move(b_lines - 3, 0);
        sprintf(inbuf1, "%d%s/%d%s", m.hp > 1000 ? m.hp / 1000 : m.hp, m.hp > 1000 ? "K" : "", m.maxhp > 1000 ? m.maxhp / 1000 : m.maxhp, m.maxhp > 1000 ? "K" : "");
        sprintf(inbuf2, "%d%s/%d%s", m.mp > 1000 ? m.mp / 1000 : m.mp, m.mp > 1000 ? "K" : "", m.maxmp > 1000 ? m.maxmp / 1000 : m.maxmp, m.maxmp > 1000 ? "K" : "");

        prints_centered("\x1b[1;34m�x\x1b[32m�m  �W:\x1b[37m%-10s  \x1b[32m��  �R:\x1b[37m%-11s \x1b[32m�k  �O:\x1b[37m%-11s                  \x1b[34m�x\x1b[m",
                p[n].name, inbuf1, inbuf2);
        move(b_lines - 2, 0);
        prints_centered("\x1b[1;34m�x\x1b[32m��  ��:\x1b[37m%-6d      \x1b[32m��  �m:\x1b[37m%-6d      \x1b[32m�t  ��:\x1b[37m%-6d      \x1b[32m��  ��:\x1b[37m%-6d    \x1b[34m�x\x1b[m",
                m.attack, m.resist, m.speed, m.money);
        move(b_lines - 1, 0);
        prints_centered("\x1b[1;34m�|�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�}\x1b[m");
        move(b_lines, 0);
        prints("\x1b[1;44;37m  �԰��R�O  \x1b[46m  [1]���q  [2]���O  [3]�]�k  [4]���m  [5]�ɥR  [6]�k�R             %*s\x1b[m", d_cols, "");

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
                    vmsg("���M�S����..:~~~");
                }
                else
                {
                    if (mresistmode == 0)
                        dinjure = (d.fight[FIGHT_HSKILL] / 100 + d.tmp[TMP_HEXP] / 100 + d.fight[FIGHT_ATTACK] / 9 - m.resist / 8 + random() % 12 + 2 - m.speed / 30 + d.fight[FIGHT_SPEED] / 30);
                    else
                        dinjure = (d.fight[FIGHT_HSKILL] / 100 + d.tmp[TMP_HEXP] / 100 + d.fight[FIGHT_ATTACK] / 9 - m.resist / 6 + random() % 12 + 2 - m.speed / 30 + d.fight[FIGHT_SPEED] / 30);
                    if (dinjure <= 0)
                        dinjure = 9;
                    m.hp -= dinjure;
                    d.tmp[TMP_HEXP] += random() % 2 + 2;
                    d.fight[FIGHT_HSKILL] += random() % 2 + 1;
                    sprintf(buf, "���q�����A���ͩR�O��C%d", dinjure);
                    vmsg(buf);
                }
                d.body[BODY_TIRED] += random() % (n + 1) / 15 + 2;
                break;

            case '2':
                show_fight_pic(2);
                if (random() % 11 == 0)
                {
                    vmsg("���M�S����..:~~~");
                }
                else
                {
                    if (mresistmode == 0)
                        dinjure = (d.fight[FIGHT_HSKILL] / 100 + d.tmp[TMP_HEXP] / 100 + d.fight[FIGHT_ATTACK] / 5 - m.resist / 12 + random() % 12 + 6 - m.speed / 50 + d.fight[FIGHT_SPEED] / 30);
                    else
                        dinjure = (d.fight[FIGHT_HSKILL] / 100 + d.tmp[TMP_HEXP] / 100 + d.fight[FIGHT_ATTACK] / 5 - m.resist / 8 + random() % 12 + 6 - m.speed / 40 + d.fight[FIGHT_SPEED] / 30);
                    if (dinjure <= 15)
                        dinjure = 20;
                    if (d.body[BODY_HP] > 5)
                    {
                        m.hp -= dinjure;
                        d.body[BODY_HP] -= 5;
                        d.tmp[TMP_HEXP] += random() % 3 + 3;
                        d.fight[FIGHT_HSKILL] += random() % 2 + 2;
                        d.body[BODY_TIRED] += random() % (n + 1) / 10 + 3;
                        sprintf(buf, "���O�����A���ͩR�O��C%d", dinjure);
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
                oldtired = d.body[BODY_TIRED];
                oldhp = d.body[BODY_HP];
                d.fight[FIGHT_MAGICMODE] = 0;
                dinjure = pip_magic_menu(0, NULL);
                if (dinjure < 0)
                    dinjure = 5;
                if (d.nodone == 0)
                {
                    if (d.fight[FIGHT_MAGICMODE] == 1)
                    {
                        oldtired = oldtired - d.body[BODY_TIRED];
                        oldhp = d.body[BODY_HP] - oldhp;
                        sprintf(buf, "�v����A�ͩR�O����%d�A�h�ҭ��C%d", oldhp, oldtired);
                        vmsg(buf);
                    }
                    else
                    {
                        if (random() % 15 == 0)
                            vmsg("���M�S����..:~~~");
                        else
                        {
                            if (d.tmp[TMP_MEXP] <= 100)
                            {
                                if (random() % 4 > 0)
                                    dinjure = dinjure * 60 / 100;
                                else
                                    dinjure = dinjure * 80 / 100;
                            }
                            else if (d.tmp[TMP_MEXP] <= 250 && d.tmp[TMP_MEXP] > 100)
                            {
                                if (random() % 4 > 0)
                                    dinjure = dinjure * 70 / 100;
                                else
                                    dinjure = dinjure * 85 / 100;
                            }
                            else if (d.tmp[TMP_MEXP] <= 500 && d.tmp[TMP_MEXP] > 250)
                            {
                                if (random() % 4 > 0)
                                    dinjure = dinjure * 85 / 100;
                                else
                                    dinjure = dinjure * 95 / 100;
                            }
                            else if (d.tmp[TMP_MEXP] > 500)
                            {
                                if (random() % 10 > 0)
                                    dinjure = dinjure * 90 / 100;
                                else
                                    dinjure = dinjure * 99 / 100;
                            }
                            if ((p[n].special[d.fight[FIGHT_MAGICMODE]-2] - 48) == 1)
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
                            d.body[BODY_TIRED] += random() % (n + 1) / 12 + 2;
                            m.hp -= dinjure;
                            /*d.tmp[TMP_MEXP]+=random()%2+2;*/
                            d.fight[FIGHT_MSKILL] += random() % 2 + 2;
                            sprintf(buf, "�]�k�����A���ͩR�O��C%d", dinjure);
                            vmsg(buf);
                        }
                    }
                }
                break;
            case '4':
                dresistmode = 1;
                d.body[BODY_TIRED] += random() % (n + 1) / 20 + 1;
                vmsg("�p���[�j���m��....");
                break;

            case '5':

                pip_basic_feed();
                break;

            case '6':
                d.thing[THING_MONEY] -= (random() % 100 + 30);
                d.learn[LEARN_BRAVE] -= (random() % 3 + 2);
                if (d.thing[THING_MONEY] < 0)
                    d.thing[THING_MONEY] = 0;
                if (d.fight[FIGHT_HSKILL] < 0)
                    d.fight[FIGHT_HSKILL] = 0;
                if (d.learn[LEARN_BRAVE] < 0)
                    d.learn[LEARN_BRAVE] = 0;
                clear();
                vs_head("�q�l�i�p��", BoardName);
                move(10, 0);
                outs_centered("            \x1b[1;31m�z�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�{\x1b[m\n");
                prints_centered("            \x1b[1;31m�x  \x1b[37m��O���j���p�� \x1b[33m%-10s                 \x1b[31m�x\x1b[m\n", d.name);
                prints_centered("            \x1b[1;31m�x  \x1b[37m�b�P��� \x1b[32m%-10s \x1b[37m�԰��Ḩ�]��          \x1b[31m�x\x1b[m\n", p[n].name);
                sprintf(inbuf1, "%d/%d", d.tmp[TMP_HEXP] - oldhexp, d.tmp[TMP_MEXP] - oldmexp);
                prints_centered("            \x1b[1;31m�x  \x1b[37m�����W�[�F \x1b[36m%-5s \x1b[37m�I  �޳N�W�[�F \x1b[36m%-2d/%-2d \x1b[37m�I  \x1b[31m�x\x1b[m\n", inbuf1, d.fight[FIGHT_HSKILL] - oldhskill, d.fight[FIGHT_MSKILL] - oldmskill);
                sprintf(inbuf1, "%d \x1b[37m��", oldmoney - d.thing[THING_MONEY]);
                prints_centered("            \x1b[1;31m�x  \x1b[37m�i�����C�F \x1b[36m%-5d \x1b[37m�I  ������֤F \x1b[36m%-13s  \x1b[31m�x\x1b[m\n", oldbrave - d.learn[LEARN_BRAVE], inbuf1);
                outs_centered("            \x1b[1;31m�|�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�}\x1b[m");
                vmsg("�T�Q���p �����W��...");
                winorlose = 0;
                break;
            }
        }
        clear();
        move(0, 0);
        if (d.sex == 1)
            prints("\x1b[1;41m  " NICKNAME PIPNAME " �� \x1b[32m�� \x1b[37m%-10s         %*s\x1b[0m", d.name, 50 + d_cols - ((int)(unsigned int)sizeof(NICKNAME PIPNAME) - 1), "");
        else if (d.sex == 2)
            prints("\x1b[1;41m  " NICKNAME PIPNAME " �� \x1b[33m�� \x1b[37m%-10s         %*s\x1b[0m", d.name, 50 + d_cols - ((int)(unsigned int)sizeof(NICKNAME PIPNAME) - 1), "");
        else
            prints("\x1b[1;41m  " NICKNAME PIPNAME " �� \x1b[34m�H \x1b[37m%-10s         %*s\x1b[0m", d.name, 50 + d_cols - ((int)(unsigned int)sizeof(NICKNAME PIPNAME) - 1), "");
        move(1, 0);
        prints_centered("\x1b[1;31m�z�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�{\x1b[m");
        move(2, 0);
        /* lucky���ӷ�color��*/
        if (d.body[BODY_TIRED] >= 80)
            lucky = 31;
        else if (d.body[BODY_TIRED] >= 60 && d.body[BODY_TIRED] < 80)
            lucky = 33;
        else
            lucky = 37;

        sprintf(inbuf1, "%d%s/%d%s", d.body[BODY_HP] > 1000 ? d.body[BODY_HP] / 1000 : d.body[BODY_HP], d.body[BODY_HP] > 1000 ? "K" : "", d.body[BODY_MAXHP] > 1000 ? d.body[BODY_MAXHP] / 1000 : d.body[BODY_MAXHP], d.body[BODY_MAXHP] > 1000 ? "K" : "");
        sprintf(inbuf2, "%d%s/%d%s", d.fight[FIGHT_MP] > 1000 ? d.fight[FIGHT_MP] / 1000 : d.fight[FIGHT_MP], d.fight[FIGHT_MP] > 1000 ? "K" : "", d.fight[FIGHT_MAXMP] > 1000 ? d.fight[FIGHT_MAXMP] / 1000 : d.fight[FIGHT_MAXMP], d.fight[FIGHT_MAXMP] > 1000 ? "K" : "");

        prints_centered("\x1b[1;31m�x\x1b[33m��  �R:\x1b[37m%-12s\x1b[33m�k  �O:\x1b[37m%-12s\x1b[33m�h  ��:\x1b[%dm%-12d\x1b[33m��  ��:\x1b[37m%-10d\x1b[31m�x\x1b[m",
                inbuf1, inbuf2, lucky, d.body[BODY_TIRED], d.thing[THING_MONEY]);

        move(3, 0);
        prints_centered("\x1b[1;31m�x\x1b[33m��  ��:\x1b[37m%-10d  \x1b[33m��  �m:\x1b[37m%-10d  \x1b[33m�t  ��:\x1b[37m%-10d  \x1b[33m�g  ��:\x1b[37m%-10d\x1b[31m�x\x1b[m",
                d.fight[FIGHT_ATTACK], d.fight[FIGHT_RESIST], d.fight[FIGHT_SPEED], d.exp);
        move(4, 0);
        prints_centered("\x1b[1;31m�x\x1b[33m��  ��:\x1b[37m%-5d       \x1b[33m�j�ɤY:\x1b[37m%-5d       \x1b[33m�s  ��:\x1b[37m%-5d       \x1b[33m�F  ��:\x1b[37m%-5d     \x1b[31m�x\x1b[m",
                d.eat[EAT_FOOD], d.eat[EAT_BIGHP], d.eat[EAT_COOKIE], d.eat[EAT_MEDICINE]);
        move(5, 0);
        prints_centered("\x1b[1;31m�|�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�}\x1b[m");
        move(6, 0);
        if (mode == 1)
            show_badman_pic(m.map/*n*/);
        move(b_lines - 4, 0);
        prints_centered("\x1b[1;34m�z�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�{\x1b[m");
        move(b_lines - 3, 0);
        sprintf(inbuf1, "%d/%d", m.hp, m.maxhp);
        sprintf(inbuf2, "%d/%d", m.mp, m.maxmp);
        prints_centered("\x1b[1;34m�x\x1b[32m�m  �W:\x1b[37m%-10s  \x1b[32m��  �R:\x1b[37m%-11s \x1b[32m�k  �O:\x1b[37m%-11s                  \x1b[34m�x\x1b[m",
                p[n].name, inbuf1, inbuf2);
        move(b_lines - 2, 0);
        prints_centered("\x1b[1;34m�x\x1b[32m��  ��:\x1b[37m%-6d      \x1b[32m��  �m:\x1b[37m%-6d      \x1b[32m�t  ��:\x1b[37m%-6d      \x1b[32m��  ��:\x1b[37m%-6d    \x1b[34m�x\x1b[m",
                m.attack, m.resist, m.speed, m.money);
        move(b_lines - 1, 0);
        prints_centered("\x1b[1;34m�|�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�}\x1b[m");
        move(b_lines, 0);
        prints("\x1b[1;41;37m  \x1b[37m�����R�O  \x1b[47m  \x1b[31m[1]\x1b[30m���q  \x1b[31m[2]\x1b[30m���O  \x1b[31m[3]\x1b[30m�]�k  \x1b[31m[4]\x1b[30m���m  \x1b[31m[5]\x1b[30m�k�R                      %*s\x1b[m", d_cols, "");

        if ((m.hp > 0) && (pipkey != '6') && (pipkey == '1' || pipkey == '2' || pipkey == '3' || pipkey == '4' || pipkey == '5') && (d.death == 0) && (d.nodone == 0))
        {
            mresistmode = 0;
            lucky = random() % 100;
            if (lucky <= 50)
                mankey = 1;
            else if (lucky <= 84)
                mankey = 2;
            else if (lucky <= 97)
                mankey = 3;
            else
                mankey = 4;
            switch (mankey)
            {
            case 1:
                if (random() % 6 == 5)
                {
                    vmsg("���S����..:~~~");
                }
                else
                {
                    if (dresistmode == 0)
                        minjure = (m.attack / 9 - d.fight[FIGHT_RESIST] / 12 + random() % 15 + 4 - d.fight[FIGHT_SPEED] / 30 + m.speed / 30 - d.fight[FIGHT_HSKILL] / 200 - d.tmp[TMP_HEXP] / 200);
                    else
                        minjure = (m.attack / 9 - d.fight[FIGHT_RESIST] / 8 + random() % 12 + 4 - d.fight[FIGHT_SPEED] / 50 + m.speed / 20 - d.fight[FIGHT_HSKILL] / 200 - d.tmp[TMP_HEXP] / 200);
                    if (minjure <= 0)
                        minjure = 8;
                    d.body[BODY_HP] -= minjure;
                    d.body[BODY_TIRED] += random() % 3 + 2;
                    sprintf(buf, "��贶�q�����A�ͩR�O��C%d", minjure);
                    vmsg(buf);
                }
                break;

            case 2:
                if (random() % 11 == 10)
                {
                    vmsg("���S����..:~~~");
                }
                else
                {
                    if (m.hp > 5)
                    {
                        if (dresistmode == 0)
                            minjure = (m.attack / 5 - d.fight[FIGHT_RESIST] / 12 + random() % 12 + 6 - d.fight[FIGHT_SPEED] / 30 + m.speed / 30 - d.fight[FIGHT_HSKILL] / 200 - d.tmp[TMP_HEXP] / 200);
                        else
                            minjure = (m.attack / 5 - d.fight[FIGHT_RESIST] / 8 + random() % 12 + 6 - d.fight[FIGHT_SPEED] / 30 + m.speed / 30 - d.fight[FIGHT_HSKILL] / 200 - d.tmp[TMP_HEXP] / 200);
                        if (minjure <= 15)
                            minjure = 20;
                        d.body[BODY_HP] -= minjure;
                        m.hp -= 5;
                        sprintf(buf, "�����O�����A�ͩR�O��C%d", minjure);
                        d.body[BODY_TIRED] += random() % 4 + 4;
                        vmsg(buf);
                    }
                    else
                    {
                        if (dresistmode == 0)
                            minjure = (m.attack / 9 - d.fight[FIGHT_RESIST] / 12 + random() % 12 + 4 - d.fight[FIGHT_SPEED] / 30 + m.speed / 25 - d.tmp[TMP_HEXP] / 200 - d.fight[FIGHT_HSKILL] / 200);
                        else
                            minjure = (m.attack / 9 - d.fight[FIGHT_RESIST] / 8 + random() % 12 + 3 - d.fight[FIGHT_SPEED] / 30 + m.speed / 25 - d.tmp[TMP_HEXP] / 200 - d.fight[FIGHT_HSKILL] / 200);
                        if (minjure <= 0)
                            minjure = 4;
                        d.body[BODY_HP] -= minjure;
                        d.body[BODY_TIRED] += random() % 3 + 2;
                        sprintf(buf, "��贶�q�����A�ͩR�O��C%d", minjure);
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
                                sprintf(inbuf1, "�����]");
                            else
                                sprintf(inbuf1, "�H��");
                        }
                        else if (m.mp >= (m.maxmp / 4))
                        {
                            minjure = m.maxmp / 5;
                            m.mp -= (300 + random() % 200);
                            if (random() % 2)
                                sprintf(inbuf1, "�g����");
                            else
                                sprintf(inbuf1, "��g��");
                        }
                        else if (m.mp >= (m.maxmp / 6))
                        {
                            minjure = m.maxmp / 6;
                            m.mp -= (100 + random() % 100);
                            if (random() % 2)
                                sprintf(inbuf1, "�g��t");
                            else
                                sprintf(inbuf1, "�۩�");
                        }
                        else
                        {
                            minjure = m.maxmp / 8;
                            m.mp -= 50;
                            if (random() % 2)
                                sprintf(inbuf1, "�����");
                            else
                                sprintf(inbuf1, "����");
                        }
                        minjure = minjure - d.fight[FIGHT_RESIST] / 50 - d.fight[FIGHT_MRESIST] / 10 - d.fight[FIGHT_MSKILL] / 200 - d.tmp[TMP_MEXP] / 200 + random() % 10;
                        if (minjure < 0)
                            minjure = 15;
                        d.body[BODY_HP] -= minjure;
                        if (m.mp < 0) m.mp = 0;
                        d.fight[FIGHT_MRESIST] += random() % 2 + 1;
                        sprintf(buf, "���l��F%s�A�A���ˤF%d�I", inbuf1, minjure);
                        vmsg(buf);
                    }
                    else
                    {
                        m.mp -= 20;
                        m.hp += (m.maxmp / 6) + random() % 20;
                        if (m.hp > m.maxhp)
                            m.hp = m.maxhp;
                        vmsg("���ϥ��]�k�v���F�ۤv...");
                    }
                }
                else
                {
                    mresistmode = 1;
                    vmsg("���[�j���m....");
                }
                break;

            case 4:
                d.thing[THING_MONEY] += (m.money + m.money / 2) / 3 + random() % 10;
                d.fight[FIGHT_HSKILL] += random() % 4 + 3;
                d.learn[LEARN_BRAVE] += random() % 3 + 2;
                m.death = 1;
                sprintf(buf, "�����{�F..�����F�@�ǿ����A...");
                vmsg(buf);
                break;
            }
        }

        if (m.death == 1)
        {
            clear();
            oldexp = ((d.tmp[TMP_HEXP] - oldhexp) + (d.tmp[TMP_MEXP] - oldmexp) + random() % 10) * (d.level + 1) + random() % (d.level + 1);
            d.exp += oldexp;
            vs_head("�q�l�i�p��", BoardName);
            if (mode == 1)
            {
                move(10, 0);
                outs_centered("            \x1b[1;31m�z�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�{\x1b[m\n");
                prints_centered("            \x1b[1;31m�x  \x1b[37m�^�i���p�� \x1b[33m%-10s                     \x1b[31m�x\x1b[m\n", d.name);
                prints_centered("            \x1b[1;31m�x  \x1b[37m���ѤF���c���Ǫ� \x1b[32m%-10s               \x1b[31m�x\x1b[m\n", p[n].name);
            }
            else
            {
                move(10, 0);
                outs_centered("            \x1b[1;31m�z�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�{\x1b[m\n");
                prints_centered("            \x1b[1;31m�x  \x1b[37m�Z�N�j�|���p�� \x1b[33m%-10s                 \x1b[31m�x\x1b[m\n", d.name);
                prints_centered("            \x1b[1;31m�x  \x1b[37m���ѤF�j�l����� \x1b[32m%-10s               \x1b[31m�x\x1b[m\n", p[n].name);
            }
            sprintf(inbuf1, "%d/%d", d.tmp[TMP_HEXP] - oldhexp, d.tmp[TMP_MEXP] - oldmexp);
            prints_centered("            \x1b[1;31m�x  \x1b[37m�������ɤF %-5s �I  �޳N�W�[�F %-2d/%-2d �I  \x1b[31m�x\x1b[m\n", inbuf1, d.fight[FIGHT_HSKILL] - oldhskill, d.fight[FIGHT_MSKILL] - oldmskill);
            sprintf(inbuf1, "%d ��", d.thing[THING_MONEY] - oldmoney);
            prints_centered("            \x1b[1;31m�x  \x1b[37m�i�����ɤF %-5d �I  �����W�[�F %-9s \x1b[31m�x\x1b[m\n", d.learn[LEARN_BRAVE] - oldbrave, inbuf1);
            prints_centered("            \x1b[1;31m�x  \x1b[37m�g��ȼW�[�F %-6d �I  �ɯũ|�� %-6d �I\x1b[31m�x\x1b[m\n", oldexp, twice(d.level, 10000, 100) - d.exp);
            outs_centered("            \x1b[1;31m�|�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�}\x1b[m\n");

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
            outs_centered("            \x1b[1;31m�z�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�{\x1b[m\n");
            prints_centered("            \x1b[1;31m�x  \x1b[37m�i�����p�� \x1b[33m%-10s                     \x1b[31m�x\x1b[m\n", d.name);
            prints_centered("            \x1b[1;31m�x  \x1b[37m�b�P \x1b[32m%-10s \x1b[37m���԰����A                \x1b[31m�x\x1b[m\n", p[n].name);
            outs_centered("            \x1b[1;31m�x  \x1b[37m�����a�}�`�F�A�b���S�O�q�s..........      \x1b[31m�x\x1b[m\n");
            outs_centered("            \x1b[1;31m�|�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�}\x1b[m\n");
            vmsg("�p���}�`�F....");
            pipdie("\x1b[1;31m�԰����Q�����F...\x1b[m  ", 1);
        }
        else if (d.death == 1 && mode == 2)
        {
            clear();
            vs_head("�q�l�i�p��", BoardName);
            move(10, 0);
            outs_centered("            \x1b[1;31m�z�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�{\n\x1b[m");
            prints_centered("            \x1b[1;31m�x  \x1b[37m�i�����p�� \x1b[33m%-10s                     \x1b[31m�x\x1b[m\n", d.name);
            prints_centered("            \x1b[1;31m�x  \x1b[37m�b�P \x1b[32m%-10s \x1b[37m���԰����A                \x1b[31m�x\x1b[m\n", p[n].name);
            outs_centered("            \x1b[1;31m�x  \x1b[37m�����a����F�A�O�̲{���S�O����.........   \x1b[31m�x\x1b[m\n");
            outs_centered("            \x1b[1;31m�|�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�}\x1b[m\n");
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
get_hurt(int hurt, int mexp)
{
    int dinjure;
    mexp = (int)BMIN(mexp, 14000) / 100;
    if (random() % 5 > 0)
        dinjure = (int)hurt * (60 + mexp) / 100;
    else
        dinjure = (int)hurt * (70 + mexp) / 100;
    return dinjure;
}
#endif

/*�i�J�ϥ��]�k���*/
static int
pip_magic_menu(  /*�԰����k�N������*/
int mode,
const UTMP *opt)
{
    int injure;         /*�ˮ`�O*/
    int pipkey;

    do
    {
        move(b_lines, 0);
        clrtoeol();
        move(b_lines, 0);
        if (mode)
        {
            prints("\x1b[1;44;37m  �]�k���  \x1b[46m  [1]�v�� [2]�p�t [3]�B�t [4]���t [5]�g�t [6]���t [7]�S�� [Q]���  %*s\x1b[m", d_cols, "");
        }
        else
        {
            prints("\x1b[1;44;37m  �]�k���  \x1b[46m  [1]�v�� [2]�p�t [3]�B�t [4]���t [5]�g�t [6]���t [Q]���          %*s\x1b[m", d_cols, "");
        }
        pipkey = vkey();
        switch (pipkey)
        {
        case '1':  /*�v���k�N*/
            d.fight[FIGHT_MAGICMODE] = 1;
            injure = pip_magic_doing_menu(treatmagiclist);
            break;

        case '2':  /*�p�t�k�N*/
            d.fight[FIGHT_MAGICMODE] = 2;
            injure = pip_magic_doing_menu(thundermagiclist);
            break;

        case '3': /*�B�t�k�N*/
            d.fight[FIGHT_MAGICMODE] = 3;
            injure = pip_magic_doing_menu(icemagiclist);
            break;

        case '4': /*���t�k�N*/
            d.fight[FIGHT_MAGICMODE] = 4;
            injure = pip_magic_doing_menu(firemagiclist);
            show_fight_pic(341);
            vmsg("�p���ϥΤF���t�k�N");
            break;

        case '5': /*�g�t�k�N*/
            d.fight[FIGHT_MAGICMODE] = 5;
            injure = pip_magic_doing_menu(earthmagiclist);
            break;

        case '6': /*���t�k�N*/
            d.fight[FIGHT_MAGICMODE] = 6;
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
                d.fight[FIGHT_MAGICMODE] = 7;
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
pip_magic_doing_menu(   /*�]�k�e��*/
const struct magicset *p)
{
    int n = 1;
    const char *s;
    char buf[256];
    char ans[5];
    int pipkey;
    int injure = 0;

    d.nodone = 0;

    clrchyiuan(6, b_lines - 6);
    move(7, 0);
    prints_centered("\x1b[1;31m�t\x1b[37;41m   �i��[%s]�@����   \x1b[0;1;31m�u�w�w�w�w�w�w�w�w�w�w�w�w\x1b[m", p[0].name);
    while ((s = p[n].name) && (p[n].needmp <= d.fight[FIGHT_MP]))
    {
        move(7 + n, 4);
        if (p[n].hpmode == 1)
        {
            prints_centered("\x1b[1;37m[\x1b[36m%d\x1b[37m] \x1b[33m%-12s  \x1b[37m�ݭn�k�O: \x1b[32m%-6d  \x1b[37m��_��O: \x1b[32m%-6d \x1b[37m��_�h��: \x1b[32m%-6d\x1b[m   ", n, p[n].name, p[n].needmp, p[n].hp, p[n].tired);
        }
        else if (p[n].hpmode == 2)
        {
            prints_centered("\x1b[1;37m[\x1b[36m%d\x1b[37m] \x1b[33m%-12s  \x1b[37m�ݭn�k�O: \x1b[32m%-6d  \x1b[37m��_��O��\x1b[35m�̤j��\x1b[37m ��_�h�Ҩ�\x1b[35m�̤p��\x1b[m  ", n, p[n].name, p[n].needmp);
        }
        else
        {
            prints_centered("\x1b[1;37m[\x1b[36m%d\x1b[37m] \x1b[33m%-12s  \x1b[37m�ݭn�k�O: \x1b[32m%-6d \x1b[m             ", n, p[n].name, p[n].needmp);
        }
        n++;
    }
    n -= 1;

    do
    {
        move(16, 4);
        sprintf(buf, "�A�Q�ϥέ��@��%8s�O?  [Q]���: ", p[0].name);
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
                d.body[BODY_HP] += p[pipkey].hp;
                d.body[BODY_TIRED] -= p[pipkey].tired;
                d.fight[FIGHT_MP] -= p[pipkey].needmp;
                if (d.body[BODY_HP] > d.body[BODY_MAXHP])
                    d.body[BODY_HP] = d.body[BODY_MAXHP];
                if (d.body[BODY_TIRED] < 0)
                    d.body[BODY_TIRED] = 0;
                injure = 0;
            }
            else if (p[pipkey].hpmode == 2)
            {
                d.body[BODY_HP] = d.body[BODY_MAXHP];
                d.fight[FIGHT_MP] -= p[pipkey].needmp;
                d.body[BODY_TIRED] = 0;
                injure = 0;
            }
            else
            {
                injure = (p[pipkey].hp + (d.fight[FIGHT_MAXMP] / 8) - random() % 5);
                d.fight[FIGHT_MP] -= p[pipkey].needmp;
            }
            d.tmp[TMP_MEXP] += random() % 3 + pipkey;
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
pip_magic_fight_menu(  /*�]�k�e��*/
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
    prints_centered("\x1b[1;31m�t\x1b[37;41m   �i��[%s]�@����   \x1b[0;1;31m�u�w�w�w�w�w�w�w�w�w�w�w�w\x1b[m", s->name);
    s++;
    while (s->name)
    {
        move(7 + n, 4);
        if ((d.fight[FIGHT_SPECIALMAGIC] & s->map) && (s->needmp <= d.fight[FIGHT_MP]))
        {
            sprintf(buf,
                    "\x1b[1;37m[\x1b[36m%d\x1b[37m] \x1b[33m%-12s  \x1b[37m�ݭn�k�O: \x1b[32m%-6d \x1b[m             ", n, s->name, s->needmp);
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
        sprintf(buf, "�A�Q�ϥέ��@��%8s�O?  [Q]���: ", p[0].name);
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
            injure = (opt->pip->hp * p[mg[pipkey]].hp / 100 - random() % 300);
            d.fight[FIGHT_MP] -= p[mg[pipkey]].needmp;
            d.tmp[TMP_MEXP] += random() % 30 + pipkey + 100;
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
/* �禡�S��                                                                  */
/*                                                                           */
/*---------------------------------------------------------------------------*/

/*�D�B*/
static int
pip_marriage_offer(void)
{
    time_t now;
    char buf[256];
    char ans[4];
    int money;
    int who;
    static const char *const name[][2] = {
        {"�k�ӤH��", "�ӤH��"},
        {"�k�ӤH��", "�ӤH��"},
        {"�k�ӤH��", "�ӤH��"},
        {"�k�ӤH��", "�ӤH��"},
        {"�k�ӤH��", "�ӤH��"}
    };
    do
    {
        who = random() % COUNTOF(name);
    }
    while (d.lover == (who + 3));

    money = random() % 2000 + random() % 3000 + 4000;
    sprintf(buf, "%s�a�ӤF����%d�A�n�V�A���p���D�B�A�z�@�N�ܡH[y/N]: ", name[who][d.sex != 1], money);
    getdata(B_LINES_REF - 1, 1, buf, ans, 2, 1, 0);
    if (ans[0] == 'y' || ans[0] == 'Y')
    {
        if (d.wantend != 1 && d.wantend != 4)
        {
            sprintf(buf, "���㤧�e�w�g���B���F�A�z�T�w�n�Ѱ��±B���A��w�߱B���ܡH[y/N]: ");
            getdata(B_LINES_REF - 1, 1, buf, ans, 2, 1, 0);
            if (ans[0] != 'y' && ans[0] != 'Y')
            {
                d.tmp[TMP_SOCIAL] += 10;
                vmsg("�٬O�����±B���n�F..");
                return 0;
            }
            d.tmp[TMP_SOCIAL] -= random() % 50 + 100;
        }
        d.learn[LEARN_CHARM] -= random() % 5 + 20;
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
        sprintf(buf, "\x1b[1;37m%s %-11s���p�� [%s] �����F %s ���D�B\x1b[0m\n", Cdate(&now), cuser.userid, d.name, name[who][d.sex != 1]);
        pip_log_record(buf);
    }
    else
    {
        d.learn[LEARN_CHARM] += random() % 5 + 20;
        d.relation += 20;
        if (d.wantend == 1 || d.wantend == 4)
            vmsg("���٦~��  �߱��٤��w...");
        else
            vmsg("�ڦ��w���B���F..�藍�_...");
    }
    d.thing[THING_MONEY] += money;
    return 0;
}

static int pip_results_show(void)  /*��ì�u*/
{
    static const char *const showname[4] = {"�Z���j�|", "���N�j�i", "�Ӯa�R�|", "�i���j��"};
    char buf[256];
    int pipkey, i = 0;
    int winorlost = 0;
    int a, b[3][2], c[3] = {0, 0, 0};

    clear();
    move(10, (d_cols>>1) + 14);
    outs("\x1b[1;33m�m�N�m�N�� ���W���l�t���ڭ̰e�H�ӤF��...\x1b[0m");
    vmsg("��  ��H���}�ݬݧa...");
    clear();
    show_resultshow_pic(0);
    move(b_lines, 0);
    prints("[A]%s [B]%s [C]%s [D]%s [Q]���:", showname[0], showname[1], showname[2], showname[3]);
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
        vmsg("���~�@���|�H���ɡ�{�b���ɶ}�l");
        for (i = 0; i < 3; i++)
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
            pip_results_show_ending(3, 0, b[1][0], b[0][0], b[2][0]);
            d.tmp[TMP_HEXP] += random() % 10 + 50;
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
            pip_results_show_ending(2, 0, c[0], c[1], c[2]);
            d.tmp[TMP_HEXP] += random() % 10 + 30;
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
            pip_results_show_ending(1, 0, c[0], c[1], c[2]);
            d.tmp[TMP_HEXP] += random() % 10 + 10;
            break;
        case 0:
            pip_results_show_ending(0, 0, b[0][0], b[1][0], b[2][0]);
            d.tmp[TMP_HEXP] -= random() % 10 + 10;
            break;
        }
        break;
    case 'B':
    case 'b':
        vmsg("���~�@���|�H���ɡ�{�b���ɶ}�l");
        show_resultshow_pic(21);
        vmsg("���ɱ���");
        if ((d.learn[LEARN_ART]*2 + d.learn[LEARN_CHARACTER]) / 400 >= 5)
        {
            winorlost = 3;
        }
        else if ((d.learn[LEARN_ART]*2 + d.learn[LEARN_CHARACTER]) / 400 >= 4)
        {
            winorlost = 2;
        }
        else if ((d.learn[LEARN_ART]*2 + d.learn[LEARN_CHARACTER]) / 400 >= 3)
        {
            winorlost = 1;
        }
        else
        {
            winorlost = 0;
        }
        pip_results_show_ending(winorlost, 1, random() % 2, random() % 2 + 2, random() % 2 + 4);
        d.learn[LEARN_ART] += random() % 10 + 20 * winorlost;
        d.learn[LEARN_CHARACTER] += random() % 10 + 20 * winorlost;
        break;
    case 'C':
    case 'c':
        vmsg("���~�@���|�H���ɡ�{�b���ɶ}�l");
        if ((d.learn[LEARN_ART]*2 + d.learn[LEARN_CHARM]) / 400 >= 5)
        {
            winorlost = 3;
        }
        else if ((d.learn[LEARN_ART]*2 + d.learn[LEARN_CHARM]) / 400 >= 4)
        {
            winorlost = 2;
        }
        else if ((d.learn[LEARN_ART]*2 + d.learn[LEARN_CHARM]) / 400 >= 3)
        {
            winorlost = 1;
        }
        else
        {
            winorlost = 0;
        }
        d.learn[LEARN_ART] += random() % 10 + 20 * winorlost;
        d.learn[LEARN_CHARM] += random() % 10 + 20 * winorlost;
        pip_results_show_ending(winorlost, 2, random() % 2, random() % 2 + 4, random() % 2 + 2);
        break;
    case 'D':
    case 'd':
        vmsg("���~�@���|�H���ɡ�{�b���ɶ}�l");
        if ((d.state[STATE_AFFECT] + d.learn[LEARN_COOKSKILL]*2) / 200 >= 4)
        {
            winorlost = 3;
        }
        else if ((d.state[STATE_AFFECT] + d.learn[LEARN_COOKSKILL]*2) / 200 >= 3)
        {
            winorlost = 2;
        }
        else if ((d.state[STATE_AFFECT] + d.learn[LEARN_COOKSKILL]*2) / 200 >= 2)
        {
            winorlost = 1;
        }
        else
        {
            winorlost = 0;
        }
        d.learn[LEARN_COOKSKILL] += random() % 10 + 20 * winorlost;
        d.tmp[TMP_FAMILY] += random() % 10 + 20 * winorlost;
        pip_results_show_ending(winorlost, 3, random() % 2 + 2, random() % 2, random() % 2 + 4);
        break;
    case 'Q':
    case 'q':
        vmsg("���~���ѥ[��.....:(");
        d.state[STATE_HAPPY] -= random() % 10 + 10;
        d.state[STATE_SATISFY] -= random() % 10 + 10;
        d.relation -= random() % 10;
        break;
    }
    if (pipkey != 'Q' && pipkey != 'q')
    {
        d.body[BODY_TIRED] = 0;
        d.body[BODY_HP] = d.body[BODY_MAXHP];
        d.state[STATE_HAPPY] += random() % 20;
        d.state[STATE_SATISFY] += random() % 20;
        d.relation += random() % 10;
    }
    return 0;
}

static int pip_results_show_ending(
int winorlost, int mode, int a, int b, int c)
{
    static const char *const gamename[4] = {"�Z���j�|", "���N�j�i", "�Ӯa�R�|", "�i���j��"};
    static const int resultmoney[COUNTOF(gamename)] = {0, 3000, 5000, 8000};
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
    prints("\x1b[1;37m���� \x1b[32m���� %s ���G���� \x1b[37m����\x1b[0m", gamename[mode]);
    move(8, (d_cols>>1) + 15);
    prints("\x1b[1;41m �a�x \x1b[0;1m�� \x1b[1;33m%-10s\x1b[36m  ���� %d\x1b[0m", name1, resultmoney[3]);
    move(10, (d_cols>>1) + 15);
    prints("\x1b[1;41m �ȭx \x1b[0;1m�� \x1b[1;33m%-10s\x1b[36m  ���� %d\x1b[0m", name2, resultmoney[2]);
    move(12, (d_cols>>1) + 15);
    prints("\x1b[1;41m �u�x \x1b[0;1m�� \x1b[1;33m%-10s\x1b[36m  ���� %d\x1b[0m", name3, resultmoney[1]);
    move(14, (d_cols>>1) + 15);
    prints("\x1b[1;41m �̫� \x1b[0;1m�� \x1b[1;33m%-10s\x1b[36m \x1b[0m", name4);
    sprintf(buf, "���~��%s�����o ��~�A�ӧa..", gamename[mode]);
    d.thing[THING_MONEY] += resultmoney[winorlost];
    vmsg(buf);
    return 0;
}

/*---------------------------------------------------------------------------*/
/* �G�N���禡�S��                                                            */
/*---------------------------------------------------------------------------*/
static int
twice(
int x, int max, int min)
{
    float a, b;
    int y;
    a = (2 * max + 3 * min) / 21000.0;
    b = (max - min - 10000 * a) / 100.0;
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
        levelswap(&d.body[BODY_MAXHP], twice(lv, ml[1].maxhp, ml[0].maxhp));
        levelswap(&d.body[BODY_WRIST], twice(lv, ml[1].wrist, ml[0].wrist));
        levelswap(&d.fight[FIGHT_ATTACK], twice(lv, ml[1].attack, ml[0].attack));
        levelswap(&d.fight[FIGHT_RESIST], twice(lv, ml[1].resist, ml[0].resist));
        levelswap(&d.fight[FIGHT_SPEED], twice(lv, ml[1].speed, ml[0].speed));
        levelswap(&d.fight[FIGHT_MAXMP], twice(lv, ml[1].maxmp, ml[0].maxmp));
        levelswap(&d.fight[FIGHT_HSKILL], twice(lv, ml[1].hskill, ml[0].hskill));
        levelswap(&d.fight[FIGHT_MSKILL], twice(lv, ml[1].mskill, ml[0].mskill));
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
        for (i = 0; i < COUNTOF(ml); i++)
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
    int color1, color2, color3, color4;

    color1 = color2 = color3 = color4 = 37;
    move(1, 0);
    m = (time(0) - start_time + d.bbtime) / 60 / 30; /* �@�� */
    /*���j�@���ɪ��W�[���ܭ�*/
    color = 37;

    clear();
    move(0, 0);
    if (d.sex == 1)
        prints("\x1b[1;41m  " NICKNAME PIPNAME " �� \x1b[32m�� \x1b[37m%-15s     %*s\x1b[0m", d.name, 55 + d_cols - (sizeof(NICKNAME PIPNAME) - 1), "");
    else if (d.sex == 2)
        prints("\x1b[1;41m  " NICKNAME PIPNAME " �� \x1b[33m�� \x1b[37m%-15s     %*s\x1b[0m", d.name, 55 + d_cols - (sizeof(NICKNAME PIPNAME) - 1), "");
    else
        prints("\x1b[1;41m  " NICKNAME PIPNAME " �� \x1b[34m�H \x1b[37m%-15s     %*s\x1b[0m", d.name, 55 + d_cols - (sizeof(NICKNAME PIPNAME) - 1), "");

    move(1, 0);
    if (d.thing[THING_MONEY] <= 100)
        color1 = 31;
    else if (d.thing[THING_MONEY] <= 500)
        color1 = 33;
    else
        color1 = 37;
    sprintf(inbuf1, "%02d/%02d/%02d", (d.year - 11) % 100, d.month, d.day);
    prints_centered(" \x1b[1;32m[��  �A]\x1b[37m %-5s     \x1b[32m[��  ��]\x1b[37m %-9s \x1b[32m[�~  ��]\x1b[37m %-5d     \x1b[32m[��  ��]\x1b[%dm %-8d \x1b[m",
            pip_age_name(m), inbuf1, m, color1, d.thing[THING_MONEY]);

    move(2, 0);

    if ((d.body[BODY_HP]*100 / d.body[BODY_MAXHP]) <= 20)
        color1 = 31;
    else if ((d.body[BODY_HP]*100 / d.body[BODY_MAXHP]) <= 40)
        color1 = 33;
    else
        color1 = 37;
    if (d.fight[FIGHT_MAXMP] == 0)
        color2 = 37;
    else if ((d.fight[FIGHT_MP]*100 / d.fight[FIGHT_MAXMP]) <= 20)
        color2 = 31;
    else if ((d.fight[FIGHT_MP]*100 / d.fight[FIGHT_MAXMP]) <= 40)
        color2 = 33;
    else
        color2 = 37;

    if (d.body[BODY_TIRED] >= 80)
        color3 = 31;
    else if (d.body[BODY_TIRED] >= 60)
        color3 = 33;
    else
        color3 = 37;

    sprintf(inbuf1, "%d/%d", d.body[BODY_HP], d.body[BODY_MAXHP]);
    sprintf(inbuf2, "%d/%d", d.fight[FIGHT_MP], d.fight[FIGHT_MAXMP]);
    prints_centered(" \x1b[1;32m[��  �R]\x1b[%dm %-10s\x1b[32m[�k  �O]\x1b[%dm %-10s\x1b[32m[��  ��]\x1b[37m %-5d     \x1b[32m[�h  ��]\x1b[%dm %-4d\x1b[0m ",
            color1, inbuf1, color2, inbuf2, d.body[BODY_WEIGHT], color3, d.body[BODY_TIRED]);

    move(3, 0);
    if (d.body[BODY_SHIT] >= 80)
        color1 = 31;
    else if (d.body[BODY_SHIT] >= 60)
        color1 = 33;
    else
        color1 = 37;
    if (d.body[BODY_SICK] >= 75)
        color2 = 31;
    else if (d.body[BODY_SICK] >= 50)
        color2 = 33;
    else
        color2 = 37;
    if (d.state[STATE_HAPPY] <= 20)
        color3 = 31;
    else if (d.state[STATE_HAPPY] <= 40)
        color3 = 33;
    else
        color3 = 37;
    if (d.state[STATE_SATISFY] <= 20)
        color4 = 31;
    else if (d.state[STATE_SATISFY] <= 40)
        color4 = 33;
    else
        color4 = 37;
    prints_centered(" \x1b[1;32m[ż  ż]\x1b[%dm %-4d      \x1b[32m[�f  ��]\x1b[%dm %-4d      \x1b[32m[�ּ֫�]\x1b[%dm %-4d      \x1b[32m[���N��]\x1b[%dm %-4d\x1b[0m",
            color1, d.body[BODY_SHIT], color2, d.body[BODY_SICK], color3, d.state[STATE_HAPPY], color4, d.state[STATE_SATISFY]);
    if (mode == 1)/*����*/
    {
        move(4, 0);
        if (d.eat[EAT_FOOD] <= 0)
            color1 = 31;
        else if (d.eat[EAT_FOOD] <= 5)
            color1 = 33;
        else
            color1 = 37;
        if (d.eat[EAT_COOKIE] <= 0)
            color2 = 31;
        else if (d.eat[EAT_COOKIE] <= 5)
            color2 = 33;
        else
            color2 = 37;
        if (d.eat[EAT_BIGHP] <= 0)
            color3 = 31;
        else if (d.eat[EAT_BIGHP] <= 2)
            color3 = 33;
        else
            color3 = 37;
        if (d.eat[EAT_MEDICINE] <= 0)
            color4 = 31;
        else if (d.eat[EAT_MEDICINE] <= 5)
            color4 = 33;
        else
            color4 = 37;
        prints_centered(" \x1b[1;36m[����]\x1b[%dm%-7d\x1b[36m[�s��]\x1b[%dm%-7d\x1b[36m[�ɤY]\x1b[%dm%-7d\x1b[36m[�F��]\x1b[%dm%-7d\x1b[36m[�H��]\x1b[37m%-7d\x1b[36m[����]\x1b[37m%-7d\x1b[0m",
                color1, d.eat[EAT_FOOD], color2, d.eat[EAT_COOKIE], color3, d.eat[EAT_BIGHP], color4, d.eat[EAT_MEDICINE], d.eat[EAT_GINSENG], d.eat[EAT_SNOWGRASS]);

    }
    move(5, 0);
    prints_centered("\x1b[1;%dm�z�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�{\x1b[m", color);
    move(6, 0);
    pip_show_age_pic(m, d.body[BODY_WEIGHT]);

    move(b_lines - 5, 0);
    prints_centered("\x1b[1;%dm�|�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�}\x1b[m", color);
    move(b_lines - 4, 0);
    outs_centered(" \x1b[1;34m�w\x1b[37;44m  �� �A  \x1b[0;1;34m�w\x1b[0m");
    move(b_lines - 3, 0);
    outs_centered(" �԰���.............\n");

}
#endif  /* #ifdef HAVE_PIP_FIGHT */
#ifdef  HAVE_PIP_FIGHT
static int pip_fight_feed(void)     /* ����*/
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
        prints_centered("%s�Ӱ�����ƩO?", d.name);
        now = time(0);
        move(b_lines, 0);
        clrtoeol();
        move(b_lines, 0);
        prints("\x1b[1;44;37m  �������  \x1b[46m[1]�Y�� [2]�s�� [3]�ɤY [4]�F�� [5]�H�x [6]���� [Q]���X            %*s\x1b[m", d_cols, "");
        pipkey = vkey();

        switch (pipkey)
        {
        case '1':
            if (d.eat[EAT_FOOD] <= 0)
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
            d.eat[EAT_FOOD]--;
            d.body[BODY_HP] += 50;
            if (d.body[BODY_HP] >= d.body[BODY_MAXHP])
            {
                d.body[BODY_HP] = d.body[BODY_MAXHP];
                d.body[BODY_WEIGHT] += random() % 2;
            }
            d.nodone = 0;
            vmsg("�C�Y�@�������|��_��O50��!");
            break;

        case '2':
            if (d.eat[EAT_COOKIE] <= 0)
            {
                move(b_lines, 0);
                vmsg("�s���Y���o..�֥h�R�a�I");
                break;
            }
            move(4, 0);
            d.eat[EAT_COOKIE]--;
            d.body[BODY_HP] += 100;
            if (d.body[BODY_HP] >= d.body[BODY_MAXHP])
            {
                d.body[BODY_HP] = d.body[BODY_MAXHP];
                d.body[BODY_WEIGHT] += (random() % 2 + 2);
            }
            else
            {
                d.body[BODY_WEIGHT] += (random() % 2 + 1);
            }
            if (random() % 2 > 0)
                show_feed_pic(2);
            else
                show_feed_pic(3);
            d.state[STATE_HAPPY] += (random() % 3 + 4);
            d.state[STATE_SATISFY] += random() % 3 + 2;
            d.nodone = 0;
            vmsg("�Y�s���e���D��...");
            break;

        case '3':
            if (d.eat[EAT_BIGHP] <= 0)
            {
                move(b_lines, 0);
                vmsg("�S���j�ɤY�F�C! �ֶR�a..");
                break;
            }
            d.eat[EAT_BIGHP]--;
            d.body[BODY_HP] += 600;
            d.body[BODY_TIRED] -= 20;
            d.body[BODY_WEIGHT] += random() % 2;
            move(4, 0);
            show_feed_pic(4);
            d.nodone = 0;
            vmsg("�ɤY..�W�ŴΪ���...");
            break;

        case '4':
            if (d.eat[EAT_MEDICINE] <= 0)
            {
                move(b_lines, 0);
                vmsg("�S���F���o..�֥h�R�a�I");
                break;
            }
            move(4, 0);
            show_feed_pic(1);
            d.eat[EAT_MEDICINE]--;
            d.fight[FIGHT_MP] += 50;
            if (d.fight[FIGHT_MP] >= d.fight[FIGHT_MAXMP])
            {
                d.fight[FIGHT_MP] = d.fight[FIGHT_MAXMP];
            }
            d.nodone = 0;
            vmsg("�C�Y�@���F�۷|��_�k�O50��!");
            break;

        case '5':
            if (d.eat[EAT_GINSENG] <= 0)
            {
                move(b_lines, 0);
                vmsg("�S���d�~�H�x�C! �ֶR�a..");
                break;
            }
            d.eat[EAT_GINSENG]--;
            d.fight[FIGHT_MP] += 500;
            d.body[BODY_TIRED] -= 20;
            move(4, 0);
            show_feed_pic(1);
            d.nodone = 0;
            vmsg("�d�~�H�x..�W�ŴΪ���...");
            break;

        case '6':
            if (d.eat[EAT_SNOWGRASS] <= 0)
            {
                move(b_lines, 0);
                vmsg("�S���Ѥs�����C! �ֶR�a..");
                break;
            }
            d.eat[EAT_SNOWGRASS]--;
            d.fight[FIGHT_MP] = d.fight[FIGHT_MAXMP];
            d.body[BODY_HP] = d.body[BODY_MAXHP];
            d.body[BODY_TIRED] -= 0;
            d.body[BODY_SICK] = 0;
            move(4, 0);
            show_feed_pic(1);
            d.nodone = 0;
            vmsg("�Ѥs����..�W�ŴΪ���...");
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
