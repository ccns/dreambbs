/*
 * Mine: '99 All-rewritten version
 *
 * Author: piaip
 *
 * $Log: mine.c,v $
 * Revision 1.1  1999/05/19 16:09:22  bbs
 * /home/bbs/src/maple: source original version
 *
 * Revision 1.3  1999/01/30 02:09:29  piaip
 * Better interface, keys help,... [beta]
 *
 * Revision 1.2  1999/01/29 13:23:28  piaip
 * For bbs version [alpha]
 *
 * Revision 1.1  1999/01/29 12:25:53  piaip
 * Initial revision
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "bbs.h"
#include "gamef.c"
#define random(x) (rand() % x)
char *BoardName = currboard;

enum {  MAP_MAXY = 20,
		MAP_MAXX = 30,

		// These are flags for bitwise operators
		TILE_BLANK = 0,
		TILE_MINE  = 1,
		TILE_TAGGED = 0x10,
		TILE_EXPAND = 0x20,

		TILE_NOUSE
	 };

static int MAP_Y = MAP_MAXY, MAP_X = MAP_MAXX;
static int TotalMines = 0, TaggedMines = 0, currx = 0, curry = 0;
static char MineMap[ MAP_MAXY+2 ][ MAP_MAXX+2 ];
extern int cur_col;
int fasttime[4];

void clrtokol(void)
{
	int n;
	for (n = cur_col;n < 17;n++)
		outc(' ');
}

void initMap(void)
{
	int x, y, i;
	for (y = 0; y < MAP_Y + 2; y++)
		for (x = 0; x < MAP_X + 2; x++)
		{
			MineMap[y][x] = TILE_BLANK;
			if (y == 0 || x == 0 || y == MAP_Y + 1 || x == MAP_X + 1)
				MineMap[y][x] |= TILE_EXPAND;
		};

	for (i = 0; i < TotalMines;)
	{
		x = random(MAP_X) + 1;
		if (rand() % 5 > 2)x = random(MAP_X) + 1;
		if (rand() % 5 > 2)y = random(MAP_Y) + 1;
		y = random(MAP_Y) + 1;
		if (MineMap[y][x] == TILE_BLANK)
		{
			MineMap[y][x] = TILE_MINE;
			i++;
		};
	};
};

int
show_fasttime(void)
{
	int i;
	FILE *fp;
	char buf[40], *buf1[4] = {"¤Jªù¯Å:", "¶i¶¥¯Å:", "°ª  ¯Å:", "ÅÜºA¯Å:"};
	fp = fopen("game/mine/mine_fasttime", "r");
	clear();
	for (i = 0;i < 4;i++)
	{
		fscanf(fp, "%s%d", buf, &fasttime[i]);
		prints("[1;31m%s  [1;32m%s %d[m\n", buf1[i], buf, fasttime[i]);
	}
	vmsg(NULL);
	fclose(fp);
	return 0;
}

int
load_fasttime(void)
{
	int i;
	char buf[40];
	FILE *fp;
	fp = fopen("game/mine/mine_fasttime", "r");
	for (i = 0;i < 4;i++)
	{
		fscanf(fp, "%s%d", buf, &fasttime[i]);
	}
	fclose(fp);
	return 0;
}

int
change_fasttime(int n, int t)
{
	int i;
	char buf[4][40];
	FILE *fp;
	fp = fopen("game/mine/mine_fasttime", "r");
	for (i = 0;i < 4;i++)
	{
		fscanf(fp, "%s%d", buf[i], &fasttime[i]);
	}
	strcpy(buf[n], cuser.userid);
	fasttime[n] = t;
	fclose(fp);
	fp = fopen("game/mine/mine_fasttime", "w");
	for (i = 0;i < 4;i++)
	{
		fprintf(fp, "%s %d\n", buf[i], fasttime[i]);
	}
	fclose(fp);
	vmsg("§A¯}°O¿ýÅo!!!!");
	return 0;
}

int countNeighbor(int y, int x, int bitmask)
{
	int sum = 0;
	if (MineMap[y-1][x+1] & bitmask) ++sum;
	if (MineMap[y-1][x  ] & bitmask) ++sum;
	if (MineMap[y-1][x-1] & bitmask) ++sum;
	if (MineMap[y  ][x+1] & bitmask) ++sum;
	if (MineMap[y  ][x  ] & bitmask) ++sum;
	if (MineMap[y  ][x-1] & bitmask) ++sum;
	if (MineMap[y+1][x+1] & bitmask) ++sum;
	if (MineMap[y+1][x  ] & bitmask) ++sum;
	if (MineMap[y+1][x-1] & bitmask) ++sum;
	return sum;
};

char *symTag = "[1;40;31m£Z[m";
char *symWrong = "[1;41;37m¢Æ[m";
char *symBlank = "¡½";
char *strMines[] = {"¡@", "¢°", "¢±", "¢²", "¢³", "¢´",
					"¢µ", "¢¶", "¢·", NULL
				   };

enum {  MAP_START_X = 16 };             // Must be > Prompts

static time_t init_time = 0;

void drawInfo(void)
{
	move(b_lines - 1, 0);
	clrtoeol();
	prints("©Òªá®É¶¡: %.0lf ¬í, ³Ñ¤U %d ­Ó¦a¹p¥¼¼Ð°O.\n",
		   difftime(time(0), init_time) , TotalMines - TaggedMines);
};

void drawPrompt(void)
{
	int i;
	for (i = 1;i <= 20;i++)
	{
		move(i, 0);
		clrtokol();
	}
	vs_bar("½ò¦a¹p");
	move(3, 0);
	outs("«öÁä»¡©ú:");clrtokol();
	move(4, 0);
	outs("²¾°Ê     ¤è¦VÁä");clrtokol();
	move(5, 0);
	outs("Â½¶}     ªÅ¥ÕÁä");clrtokol();
	move(6, 0);
	outs("¼Ð°O¦a¹p   ¢õ");clrtokol();
	move(7, 0);
	outs("±½¹p       ¢ü");clrtokol();
	move(9, 0);
	outs("Â÷¶}    Esc / q");clrtokol();
};

void drawMapLine(int y, int flShow)
{
	int x = 0;
	drawInfo();
	move(y + 1, MAP_START_X);
	clrtokol();
	for (x = 1; x <= MAP_X; x++)
	{

		if (x == currx && y == curry) outs("[44;31m");

		if (MineMap[y][x] & TILE_TAGGED)
		{
			if (flShow && (MineMap[y][x] & TILE_MINE) == 0)
				outs(symWrong);
			else outs(symTag);
		}
		else if (MineMap[y][x] & TILE_EXPAND)
			outs(strMines[countNeighbor(y, x, TILE_MINE)]);
		else if (flShow && (MineMap[y][x] & TILE_MINE))
			outs(symTag);
		else outs(symBlank);

		if (x == currx && y == curry) outs("[m");
	};
	clrtoeol();

	move(curry + 1, currx*2 + MAP_START_X - 1);
};

void drawMap(int flShow)
{
	int y;

	clear();
	drawPrompt();
	for (y = 1; y < MAP_Y + 1; y++)
	{
		drawMapLine(y, flShow);
	};
};

static int flLoseMine = 0;

static void loseMine(void)
{
	drawMap(1);
	game_log(1, "[31;1m¦b %.01f ¬í®É½ò¨ì¦a¹p°Õ!!!", difftime(time(0), init_time));
	vmsg("§A¿é¤F");
	flLoseMine = 1;
};

void ExpandMap(int y, int x, int flTrace)
{
	if (!flTrace)
	{
		if (MineMap[y][x] & TILE_TAGGED || MineMap[y][x] & TILE_EXPAND) return;
		if ((MineMap[y][x] & TILE_MINE) && (!(MineMap[y][x] & TILE_TAGGED)))
			{ loseMine(); return; };
		MineMap[y][x] |= TILE_EXPAND;
		drawMapLine(y, 0);
	};
	if (flTrace || countNeighbor(y, x, TILE_MINE) == 0)
	{
		if (flTrace || (MineMap [y-1][x  ] & TILE_EXPAND) == 0)
			ExpandMap(y - 1, x  , 0);
		if (flTrace || (MineMap [y  ][x-1] & TILE_EXPAND) == 0)
			ExpandMap(y  , x - 1, 0);
		if (flTrace || (MineMap [y+1][x  ] & TILE_EXPAND) == 0)
			ExpandMap(y + 1, x  , 0);
		if (flTrace || (MineMap [y  ][x+1] & TILE_EXPAND) == 0)
			ExpandMap(y  , x + 1, 0);
		if (flTrace || (MineMap [y-1][x-1] & TILE_EXPAND) == 0)
			ExpandMap(y - 1, x - 1, 0);
		if (flTrace || (MineMap [y+1][x-1] & TILE_EXPAND) == 0)
			ExpandMap(y + 1, x - 1, 0);
		if (flTrace || (MineMap [y-1][x+1] & TILE_EXPAND) == 0)
			ExpandMap(y - 1, x + 1, 0);
		if (flTrace || (MineMap [y+1][x+1] & TILE_EXPAND) == 0)
			ExpandMap(y + 1, x + 1, 0);
	};
};

void TraceMap(int y, int x)
{
	if (!(MineMap[y][x] & TILE_EXPAND)) return;
	if (countNeighbor(y, x, TILE_MINE) == countNeighbor(y, x, TILE_TAGGED))
	{
		ExpandMap(y, x, 1);
	};
};

void playMine(void)
{
	int ch ;
	currx = MAP_X / 2 + 1, curry = MAP_Y / 2 + 1;
	flLoseMine = 0;
	move(2, 0);
	clrtobot();

	drawMap(0);
	while (!flLoseMine && ((ch = vkey()) != 'q'))
	{

		switch (ch)
		{
		case KEY_ESC:
			return;
			break;

		case KEY_UP:
			if (curry > 1)
			{
				drawMapLine(curry--, 0);
				drawMapLine(curry, 0);
			};
			break;

		case KEY_DOWN:
			if (curry < MAP_Y)
			{
				drawMapLine(curry++, 0);
			};
			break;

		case KEY_LEFT:
			if (currx > 1) currx--;
			break;

		case KEY_RIGHT:
			if (currx < MAP_X) currx++;
			break;

		case Ctrl('P'):
						drawMap(1);
			pressanykey(0);
			drawMap(0);
			break;

		case '\r':
		case 't':
		case '\n':
			TraceMap(curry, currx);
			break;

		case ' ':
			ExpandMap(curry, currx, 0);
			break;

		case 'm':
			if ((MineMap[curry][currx] & TILE_EXPAND))
	{
				if (MineMap[curry][currx] & TILE_TAGGED)
				{
					TaggedMines--;
					MineMap[curry][currx] ^= TILE_EXPAND;
				}
				else
					break;
			}
			else
			{
				TaggedMines++;
				MineMap[curry][currx] ^= TILE_EXPAND;
			};

			MineMap[curry][currx] ^= TILE_TAGGED;
			if (TaggedMines == TotalMines) return;
			break;

		default:
			break;
		};
		drawMapLine(curry, 0);
	}
};

int Mine(void)
{
	int x, y, ti;
	char ans[5], buf[10];
	srand(time(0));
start:
	vs_bar("½ò¦a¹p");
	getdata(2, 0, "§A­nª± [1]¤Jªù [2]¶i¶¥ [3]°ª¯Å [4]ÅÜºA [5]³Ì§Ö®É¶¡ [6]Â÷¶}: ",
			ans, 4, DOECHO, "1");
	TotalMines = 0;
	switch (atoi(ans))
	{
	case 1:
		strcpy(buf, "¤Jªù¯Å");
		MAP_X = 10;
		MAP_Y = 10;
		break;
	case 2:
		strcpy(buf, "¶i¶¥¯Å");
		MAP_X = 20;
		MAP_Y = 15;
		break;
	case 3:
		strcpy(buf, "°ª  ¯Å");
		MAP_X = 30;
		MAP_Y = 20;
		break;
	case 4:
		strcpy(buf, "ÅÜºA¯Å");
		MAP_X = 30;
		MAP_Y = 20;
		TotalMines = 40;
		break;
	case 5:
		show_fasttime();
		break;
	default:
		return 0;
	};
	if (atoi(ans) == 5) goto start;
	load_fasttime();
	TotalMines += (MAP_X / 10) * (MAP_Y);
	TaggedMines = 0;
	initMap();
	init_time = time(0);
	playMine();
	if (!flLoseMine)
	{
		for (y = 1; y < MAP_Y + 1; y++)
			for (x = 1; x < MAP_X + 1; x++)
				if ((MineMap[y][x] & TILE_MINE) &&
					!(MineMap[y][x] & TILE_TAGGED))
					{ loseMine(); y = MAP_Y + 1; break; }

		if (!flLoseMine)
		{
			move(b_lines - 1, 0);
			clrtoeol();
			ti = (int) difftime(time(0), init_time);
			outs("[1;37;44m§AÄ¹¤F!  ");
			prints("©Òªá®É¶¡: %d ¬í[m\n", ti);
			game_log(1, "[32;1m¦b %s ªá¤F %d ¬í²M°£¦a¹p!!!", buf, ti);
			if (ti < fasttime[atoi(ans)-1]) change_fasttime(atoi(ans) - 1, ti);
			vmsg(NULL);
		};
	};
	goto start;
};

