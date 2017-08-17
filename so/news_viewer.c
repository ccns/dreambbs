/*-------------------------------------------------------*/
/* news_viewer.c    (YZU WindTopBBS Ver 3.02 )           */
/*-------------------------------------------------------*/
/* author : Verit.bbs@bbs.yzu.edu.tw                     */
/* modify : visor.bbs@bbs.yzu.edu.tw                     */
/* target : Kimo News Menu                               */
/* create : 01/07/12                                     */
/* update : 01/07/13                                     */
/*-------------------------------------------------------*/

#include "bbs.h"

#undef	HYPER

#define MENU_LEFT       1
#define MENU_TOP        10
#define TITLE_LEFT      35
#define TITLE_TOP       6
#define MAX_TITLE       15
#define MAX_MENU        11
#define BANNER_TOP      1
#define BANNER_LEFT     2

#define MAIN_MENU_TOP   7
#define MAIN_MENU_LEFT  5
#define MAX_MAIN_MENU   5

#define MENU_BAR        "\033[44;37;1m"
#define MAIN_MENU_BAR   "\033[43;37;1m"
#define TITLE_BAR       "\033[41;37;1m"
#define ORG_BAR         "\033[40;37;0m"

#define M_PU            "*[500m"
#define M_DN            "[501m"
#define M_HOME          "[502m"
#define M_END           "[503m"
#define M_TOP           "[504m"
#define M_DOWN          "[505m"
#define M_LEFT          "[506m"
#define M_RIGHT         "[507m"

#if 1   //lale banner or kimo banner
#define BANNER1         "[0m[33;1m¢¡  ¢¡¢~שש¢{¢¡  ¢¡[32mשÝשש¢¡שß[36mשששששששששששששששששששששש[32m [37m¸ך®ֶ¨׃·½¡G©_¼¯·s»D[36m שששששש¢¡"
#define BANNER2         "[0m[33;1mשר  שר  ¢¡שרשר  שר[32mשר  שרשר¢~שש¢¡¢~  ¢¡¢~שש¢¡ [37m    http://tw.news.yahoo.com/[36mשר"
#define BANNER3         "[0m[33;1m¢¢¢s¢£¢~¢q¢£שר  שר[32mשר  שרשרשאשששושרשרשרשüשש¢¡            [37m Designed By Verit[36mשר"
#define BANNER4         "[0m[33;1m  ¢¢שששדשששש¢¢שש¢£[32mשדשש¢¢שהשהשששהשהשהשהששששש‎[36mששששששששששששששששששששששששששששששש‎[0m"
#endif

static char total_news[50][30];
static char total_menu[50][5];
static int total_all;
static int total_nall;

static char news[MAX_MENU][30];
char menu_list[MAX_MENU][5];

static char main_menu[50][15];

static int menu_count;

static HDR *hdr_spool;
static int news_count[MAX_MENU];
static struct tm *ptime;
static int site = 0;
static char site_name[18][15];


#ifdef HYPER
static char mouse_con[512];
#endif

int draw_banner(int);

int init()
{
	int i, j;
	char fpath[128], buf[256], *str;
	time_t now;
	FILE *fp;

	time(&now);
	ptime = localtime(&now);

	fp = fopen(FN_NEWS_CLASS, "r");
	i = total_all = 0;
	if (fp)
	{
		while (fgets(buf, 256, fp))
		{
			if (buf[0] == '#')
				continue;
			str = strtok(buf, "\t\n");
			if (str)
				strcpy(total_news[i], str);
			str = strtok(NULL, "\t\n");
			if (str)
				strcpy(total_menu[i], str);
			i++;
		}
		total_all = i - 1;
	}
	fclose(fp);

	sprintf(fpath, FN_NEWS"%s.ini", site_name[site]);
	fp = fopen(fpath, "r");
	i = 0;
	if (fp)
	{
		while (fgets(buf, 256, fp))
		{
			if (buf[0] == '#')
				continue;
			str = strtok(buf, "\t\n");
			if (str)
				if (strcmp("CLASS", str))
					continue;
			while ((str = strtok(NULL, "\t\n,")))
			{
				strcpy(news[i], str);
				i++;
			}
		}
		menu_count = i;
	}
	fclose(fp);
	for (i = 0;i < menu_count;i++)
	{
		for (j = 0;j < total_all;j++)
			if (!strcmp(news[i], total_news[j]))
			{
				strcpy(menu_list[i], total_menu[j]);
			}
	}

	return 1;
}
#ifdef HYPER
char *mouse_check(int index)
{
	int i;
	int tmp = index / MAX_TITLE;

	memset(mouse_con, 0, sizeof(mouse_con));
	strcpy(mouse_con, M_HOME);
	for (i = 0;i < tmp;i++)
		strcat(mouse_con, M_DN);
	tmp = index % MAX_TITLE;
	for (i = 0;i < tmp;i++)
		strcat(mouse_con, M_DOWN);
	strcat(mouse_con, M_RIGHT);
	return mouse_con;
}
#endif

int draw_main_menu(int now, int old)
{
	int now_x = now % MAX_MAIN_MENU ;
	int now_y = now / MAX_MAIN_MENU ;
	int old_x = old % MAX_MAIN_MENU ;
	int old_y = old / MAX_MAIN_MENU ;
	int page = (total_nall - 1) / MAX_MAIN_MENU ;
	int i;

	for (i = old_y;i < page + 1;i++)
	{
		move(MAIN_MENU_TOP + old_x, MAIN_MENU_LEFT + 37*i);
		prints("%s%12s  %s", ORG_BAR, main_menu[old_x+MAX_MAIN_MENU*i], ORG_BAR);
	}
	for (i = now_y;i < page + 1;i++)
	{
		if (i == now_y)
		{
			move(MAIN_MENU_TOP + now_x, MAIN_MENU_LEFT + 37*i);
			prints("%s%12s  %s", MAIN_MENU_BAR, main_menu[now], ORG_BAR);
		}
		else
		{
			move(MAIN_MENU_TOP + now_x, MAIN_MENU_LEFT + 37*i);
			prints("%s%12s  %s", ORG_BAR, main_menu[now_x+MAX_MAIN_MENU*i], ORG_BAR);
		}
	}
	return 1;
}

int fmain_menu()
{
	char c;
	int i, x, y;
	int now_main = site, old_main = 0;

	clear();
	draw_banner(MAIN_MENU_TOP - 6);

	move(MAIN_MENU_TOP - 2, MAIN_MENU_LEFT);
	prints("¢~שששששששששששששששששששששששששששששששששששששששששששששששששששששששששששששששששש¢¡");
	for (i = 0;i < total_nall;i++)
	{
		x = i % MAX_MAIN_MENU ;
		y = i / MAX_MAIN_MENU ;
		if (y == 0)
		{
			move(MAIN_MENU_TOP + x, MAIN_MENU_LEFT);
			clrtohol();
		}
		move(MAIN_MENU_TOP + x, MAIN_MENU_LEFT + y*37);
		prints("%s%12s  %s", ORG_BAR, main_menu[i], ORG_BAR);
	}
	move(MAIN_MENU_TOP + MAX_MAIN_MENU + 1, MAIN_MENU_LEFT);
	prints("¢¢שששששששששששששששששששששששששששששששששששששששששששששששששששששששששששששששששש¢£");
	outz("\033[46;44m ¡i¾Þ§@»¡©ת¡j \033[47;31m  (¡פ¡ץ¡צ¡ק) \033[30m¿ן¾Ü \033[31m(Enter) \033[30m½T©w\033[31m (q)\033[30m ֲק¶}                          \033[m");
	draw_main_menu(site, site);

	do
	{
		old_main = now_main;
		c = vkey();
		switch (c)
		{
		case KEY_DOWN:
			now_main ++;
			if (now_main == total_nall)
				now_main = 0;
			draw_main_menu(now_main, old_main);
			break;
		case KEY_UP:
			now_main --;
			if (now_main < 0)
				now_main = total_nall - 1;
			draw_main_menu(now_main, old_main);
			break;
		case KEY_HOME:
		case '0':
			now_main = 0;
			draw_main_menu(now_main, old_main);
			break;
		case KEY_END:
		case '$':
			now_main = total_nall - 1;
			draw_main_menu(now_main, old_main);
			break;
		case Ctrl('U'):
						every_U();
			break;
		case KEY_RIGHT:
			now_main += MAX_MAIN_MENU ;
			if (now_main > total_nall - 1)
				now_main %= MAX_MAIN_MENU ;
			draw_main_menu(now_main, old_main);
			break ;
		case KEY_LEFT:
			if (now_main < MAX_MAIN_MENU)
	{
				if (now_main <= (total_nall - 1) % MAX_MAIN_MENU)
				{
					now_main += MAX_MAIN_MENU * ((total_nall - 1) / MAX_MAIN_MENU);
				}
				else
				{
					now_main = total_nall - 1;
				}
			}
			else
				now_main -= MAX_MAIN_MENU ;
			draw_main_menu(now_main, old_main);
			break;
		case '\n':
		case '\r':
			return now_main;
		}
	}
	while (c != 'q' && c != 'Q' && c != KEY_ESC);
	return -1;
}

int draw_banner(int top)
{
	move(top + 0, BANNER_LEFT);outs(BANNER1);
	move(top + 1, BANNER_LEFT);outs(BANNER2);
	move(top + 2, BANNER_LEFT);outs(BANNER3);
	move(top + 3, BANNER_LEFT);outs(BANNER4);
	return 0;
}

int draw_menu(int now, int old, int mode)
{
	int i;

	if (old == -1)
	{
		for (i = 0;i < 22;i++)
		{
			move(i, MENU_LEFT);
			if (MENU_TOP - 5 == i)
			{
				move(i, 1);
				prints("\033[36;1;40m¢~¢w¢w¢w¢w¢¡%s¡»¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w\033[37m NewsTitle \033[32;1m¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¡»%s", "\033[32;1;40m", ORG_BAR);
			}
			else if (i == MENU_TOP - 2)
				prints("\033[32;1;40m %s"ORG_BAR, main_menu[site]);
			else if (i == MENU_TOP - 4)
				prints("\033[36;1;40m¢x%2d₪כ%2d₪י¢x"ORG_BAR, ptime->tm_mon + 1, ptime->tm_mday);
			else if (i == MENU_TOP - 3)
				prints("\033[36;1;40m¢¢¢w¢w¢w¢w¢£"ORG_BAR);
			else if (i == MENU_TOP - 1)
				prints("\033[32;1mשÝ¡ד\033[37mMemu\033[32;40m¡דשß");
			else if (i == MENU_TOP + MAX_MENU)
				prints("\033[32;1;40mשד¡ד¡ד¡ד¡דשו"ORG_BAR);//,"\033[32;1;40m");
			else
			{
				move(i, MENU_LEFT + 2);
				if (i >= MENU_TOP && i < MENU_TOP + menu_count)
					prints("%s%6s%s", ORG_BAR, menu_list[i-MENU_TOP], ORG_BAR);
				else
					prints("%s%6s%s", ORG_BAR, " ", ORG_BAR);
			}
		}
		draw_banner(BANNER_TOP);
	}
	else
	{
		move(MENU_TOP + old, MENU_LEFT + 2);
		prints("%s%6s  %s", ORG_BAR, menu_list[old], ORG_BAR);
	}

	move(MENU_TOP + now, MENU_LEFT + 2);
	prints("%s%6s  %s", MENU_BAR, menu_list[now], ORG_BAR);
	return 0;
}

int draw_title(int menu, int now, int old, int again, int mode)
{
	char buf[128];
	int i, fd;
	int page;
	char fpath[128];
	char bname[64];

	if (again)
	{
		if (hdr_spool)
			free(hdr_spool);

		strcpy(bname, news[menu]);
		bname[IDLEN-1] = '\0';
		sprintf(fpath, BRD_NEWS"%s/@/@%s", site_name[site], bname);
		news_count[menu] = rec_num(fpath, sizeof(HDR));
		hdr_spool = (HDR *)malloc(sizeof(HDR) * news_count[menu]);
		fd = open(fpath, O_RDONLY);
		read(fd, hdr_spool, (sizeof(HDR) * news_count[menu]));
		close(fd);

		memset(buf, 0, sizeof(buf));
		page = now / MAX_TITLE;
		for (i = page * MAX_TITLE ;i < (page + 1) * MAX_TITLE && i < news_count[menu];i++)
		{
			move(TITLE_TOP + (i % MAX_TITLE), TITLE_LEFT);
			clrtoeol();
#if 0
			prints("%s%2d) %-5.5s %-53.53s%s", ORG_BAR, i + 1, hdr_spool[i].date + 3, hdr_spool[i].title, ORG_BAR);
#else
#ifdef HYPER
			prints("%s[200m[441;1437m\033[508m\033[300m%2d\033[303m) %-59.55s%s[201m", ORG_BAR, i + 1, hdr_spool[i].title, ORG_BAR);
#else
			prints("%s%2d) %-59.55s%s", ORG_BAR, i + 1, hdr_spool[i].title, ORG_BAR);
#endif
#endif
		}
		if (page == news_count[menu] / MAX_TITLE)
			for (i = news_count[menu] % MAX_TITLE;i < MAX_TITLE;i++)
			{
				move(TITLE_TOP + i, TITLE_LEFT);
				prints("\033[m");
				clrtoeol();
			}
		if (news_count[menu] == 0)
		{
			move(TITLE_TOP + 4, TITLE_LEFT);
			prints("                      \033[37;41;1m  ¥»₪י©|µL·s»D  \033[37;40m");
			clrtoeol();
		}
	}
	else
	{
		move(TITLE_TOP + (old % MAX_TITLE), TITLE_LEFT);
#if 0
		prints("%s%2d) %-5.5s %-53.53s%s", ORG_BAR, old + 1, hdr_spool[old].date + 3, hdr_spool[old].title, ORG_BAR);
#else
#ifdef HYPER
		prints("%s[200m\033[441;1437m\033[508m\033[300m%2d\033[303m) %-59.58s%s[m[201m", ORG_BAR, old + 1, hdr_spool[old].title, ORG_BAR);
#else
		prints("%s%2d) %-59.55s%s", ORG_BAR, old + 1, hdr_spool[old].title, ORG_BAR);
#endif
#endif
	}
	if (mode == 1)
	{
		move(TITLE_TOP + (now % MAX_TITLE), TITLE_LEFT);
#if 0
		prints("%s%2d) %-5.5s %-53.53s%s", TITLE_BAR, now + 1, hdr_spool[now].date + 3, hdr_spool[now].title, ORG_BAR);
#else
#ifdef HYPER
		prints("%s[200m\033[441;1437m\033[508m\033[300m%2d\033[303m) %-59.58s%s[201m", TITLE_BAR, now + 1, hdr_spool[now].title, ORG_BAR);
#else
		prints("%s%2d) %-59.55s%s", TITLE_BAR, now + 1, hdr_spool[now].title, ORG_BAR);
#endif
#endif
	}
	return 0;
}

static int
news_cross(xo)
XO *xo;
{
	char xboard[20], fpath[128], xfolder[80], buf[80], buf2[80];
	HDR xpost;
	int rc;
	FILE *fp;
	HDR *hdr;

	if (!HAS_PERM(PERM_POST))
		return 0;

	hdr = (HDR *) hdr_spool + xo->pos;

	if (ask_board(xboard, BRD_W_BIT,
				  "\n\n\033[1;33m½׀¬D¿ן¾A·ם×÷¬Ý×O¡A₪ֱ₪ֲֵא¶K¶W¹L₪T×O¡C\033[m\n\n")
		&& *xboard)
	{
		rc = vget(2, 0, "(S)¦sְֹ (Q)¨ת®ר¡H[Q] ", buf, 3, LCECHO);
		if (rc != 's' && rc != 'S')
			return 0;

		hdr_fpath(fpath, xo->dir, hdr);
		brd_fpath(xfolder, xboard, fn_dir);

		if (!(fp = fdopen(hdr_stamp(xfolder, 'A', &xpost, buf), "w")))
			return 0;

		strcpy(buf2, ve_title);
		strcpy(ve_title, hdr->title);
		strcpy(buf, currboard);
		strcpy(currboard, xboard);
		ve_header(fp);
		strcpy(currboard, buf);
		strcpy(ve_title, buf2);

		fprintf(fp, "¡° ¥»₪וֲא¿‎¦Û [­·₪§¶נ·s»D¾\\ֵ×¨t²־]\n\n");

		f_suck(fp, fpath);
		fclose(fp);

		strcpy(xpost.owner, cuser.userid);
		strcpy(xpost.nick, cuser.username);
		memcpy(xpost.date, hdr->date, sizeof(xpost.date));

		sprintf(xpost.title, "[ֲא¶K]%-54.54s", hdr->title);

		rec_add(xfolder, &xpost, sizeof(xpost));

		vmsg("ֲא¿‎§¹¦¨");
	}
	return 0;

}


int news_menu(int choose)
{
	int c;
	XO *xo;
	int now_menu, now_title, old_menu, old_title;
	int state = 0;
	char buf[128], site_path[128];

	clear();
	init();
	now_menu = now_title = old_menu = old_title = 0;
	draw_menu(now_menu, -1, state);
	draw_title(0, 0, 0, 1, state);
	do
	{
#if 1
		outz("\033[46;44m ¡i¾Þ§@»¡©ת¡j \033[47;31m  (¡פ¡ץ) \033[30m¿ן¾Ü \033[31m(¡ק Enter) \033[30m½T©w\033[31m (¡צ)\033[30m ֲק¶}                           \033[m");
#else
		outz("[200m[506m[¡צ]ֲק¶}[201m");
		/*//      outz("\033[200m\033[506m[¡צ]ֲק¶}\033[201m");*/
#endif

		old_menu = now_menu ;
		old_title = now_title;
		c = vkey();
		if (c >= '1' && c <= '9' && state)
		{
			char buf1[6];
			buf1[0] = c;
			buf1[1] = '\0';
			vget(23, 0, "¸ץ¦Ü²ִ´X¶µ¡G", buf1, sizeof(buf1), GCARRY);
			now_title = atoi(buf1) - 1;
			if (now_title >= news_count[now_menu])
				now_title = news_count[now_menu] - 1;
			draw_title(now_menu, now_title, old_title, 1, state);
		}
		switch (c)
		{
		case 'x':
			if (state)
			{
				sprintf(site_path, BRD_NEWS"%s/.DIR", site_name[site]);
				xo = xo_new(site_path);
				xo->pos = now_title;
				news_cross(xo);
				free(xo);
				clear();
				draw_menu(now_menu, -1, state);
				draw_title(now_menu, now_title, old_title, 1, state);
			}
			break;
		case KEY_TAB :
			now_menu++;
			if (now_menu == menu_count)
				now_menu = 0;
			draw_menu(now_menu, old_menu, state);
			now_title = 0;
			state = news_count[now_menu] ? 1 : 0;
			draw_title(now_menu, 0, 0, 1, state);
			break;
		case KEY_DOWN :
			if (state)
			{
				now_title++;
				if (now_title == news_count[now_menu])
					now_title = 0;
				draw_title(now_menu, now_title, old_title, (now_title % MAX_TITLE) ? 0 : 1, state);
			}
			else
			{
				now_menu++;
				if (now_menu == menu_count)
					now_menu = 0;
				draw_menu(now_menu, old_menu, state);
				now_title = 0;
				draw_title(now_menu, 0, 0, 1, state);
			}
			break;
		case KEY_UP :
			if (state)
			{
				now_title--;
				if (now_title < 0)
				{
					now_title = news_count[now_menu] - 1;
					draw_title(now_menu, now_title, old_title, 1, state);
					break;
				}
				draw_title(now_menu, now_title, old_title, ((now_title + 1) % MAX_TITLE) ? 0 : 1, state);
			}
			else
			{
				now_menu--;
				if (now_menu < 0)
					now_menu = menu_count - 1;
				draw_menu(now_menu, old_menu, state);
				now_title = 0;
				draw_title(now_menu, 0, 0, 1, state);
			}
			break;
		case KEY_LEFT:
			if (state)
			{
				state = 0;
				draw_menu(now_menu, old_menu, state);
				draw_title(now_menu, now_title, old_title, 0, state);
			}
			else
			{
				return 1;
			}
			break;
		case KEY_PGUP:
			if (state)
			{
				now_title -= MAX_TITLE;
				if (now_title < 0)
					now_title = 0;
				draw_title(now_menu, now_title, old_title, 1, state);
			}
			break;
		case KEY_PGDN:
		case ' ':
			if (state)
			{
				now_title += MAX_TITLE;
				if (now_title >= news_count[now_menu])
					now_title = news_count[now_menu] - 1;
				draw_title(now_menu, now_title, old_title, 1, state);
			}
			break;
		case KEY_HOME:
		case '0':
			if (state)
			{
				now_title = 0;
				draw_title(now_menu, now_title, old_title, 1, state);
			}
			break;
		case KEY_END:
		case '$':
			if (state)
			{
				now_title = news_count[now_menu] - 1;
				draw_title(now_menu, now_title, old_title, 1, state);
			}
			break;
		case KEY_INS:
			state = 1;
			break;
		case KEY_RIGHT:
		case '\n':
		case '\r':
		case 'r':
			if (state)
			{
				sprintf(site_path, BRD_NEWS"%s/.DIR", site_name[site]);
				hdr_fpath(buf, site_path, &hdr_spool[now_title]);
				more(buf, NULL);
				clear();
				draw_menu(now_menu, -1, state);
				draw_title(now_menu, now_title, old_title, 1, state);
			}
			else
			{
				state = news_count[now_menu] ? 1 : 0;
				draw_menu(now_menu, old_menu, state);
				if (state)
					draw_title(now_menu, now_title, old_title, 0, state);
			}
			break;
		case Ctrl('B'):
						every_B();
			break;
		case Ctrl('U'):
						every_U();
			break;
		case Ctrl('Z'):
						every_Z();
			break;
		}
	}
	while (1);
	return 1;
}

int main_news()
{
	FILE *fp;
	int i;
	char buf[256], *str;

	fp = fopen(FN_NEWS_NEWS, "r");
	i = 0;
	if (fp)
	{
		while (fgets(buf, 256, fp))
		{
			if (buf[0] == '#')
				continue;
			str = strtok(buf, "\t\n");
			if (str)
				strcpy(site_name[i], str);
			str = strtok(NULL, "\t\n");
			str = strtok(NULL, "\t\n");
			if (str)
				strcpy(main_menu[i], str);
			i++;
		}
		strcpy(main_menu[i++], "ֲק      ¶}");
		total_nall = i;
	}
	fclose(fp);

	site = 0;

	do
	{
		site = fmain_menu();
		if (site == -1 || site == (total_nall - 1))
			return 1;
		if (site < total_nall - 1)
			news_menu(site);
	}
	while (1);
	return 0;
}

