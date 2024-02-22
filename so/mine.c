/*
 * Mine: '99 All-rewritten version
 *
 * Author: piaip
 *
 * $Log: mine.c, v $
 * Revision 1.1  1999/05/19 16:09:22  bbs
 * /home/bbs/src/maple: source original version
 *
 * Revision 1.3  1999/01/30 02:09:29  piaip
 * Better interface, keys help, ... [beta]
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
#define random(x) ((void)0, rand() % (x))
static char *BoardName = currboard;

enum {  MAP_MAXY = 20,
        MAP_MAXX = 30,

        // These are flags for bitwise operators
        TILE_BLANK = 0,
        TILE_MINE  = 1,
        TILE_TAGGED = 0x10,
        TILE_EXPAND = 0x20,

        TILE_NOUSE,
     };

static int MAP_Y = MAP_MAXY, MAP_X = MAP_MAXX;
static int TotalMines = 0, TaggedMines = 0, currx = 0, curry = 0;
static char MineMap[ MAP_MAXY+2 ][ MAP_MAXX+2 ];
static int fasttime[4];

static void clrtokol(void)
{
    int n;
    int x;
    getyx(&SINKVAL(int), &x);
    for (n = x; n < 17; n++)
        outc(' ');
}

static void initMap(void)
{
    int x, y, i;
    for (y = 0; y < MAP_Y + 2; y++)
        for (x = 0; x < MAP_X + 2; x++)
        {
            MineMap[y][x] = TILE_BLANK;
            if (y == 0 || x == 0 || y == MAP_Y + 1 || x == MAP_X + 1)
                MineMap[y][x] |= TILE_EXPAND;
        }

    for (i = 0; i < TotalMines;)
    {
        x = random(MAP_X) + 1;
        if (rand() % 5 > 2)
            x = random(MAP_X) + 1;
        if (rand() % 5 > 2)
            y = random(MAP_Y) + 1;
        y = random(MAP_Y) + 1;
        if (MineMap[y][x] == TILE_BLANK)
        {
            MineMap[y][x] = TILE_MINE;
            i++;
        }
    }
}

static int
show_fasttime(void)
{
    int i;
    FILE *fp;
    char buf[40];
    static const char *const buf1[] = {"入門級:", "進階級:", "高  級:", "變態級:"};
    fp = fopen("game/mine/mine_fasttime", "r");
    clear();
    for (i = 0; i < COUNTOF(buf1); i++)
    {
        fscanf(fp, "%s%d", buf, &fasttime[i]);
        prints("\x1b[1;31m%s  \x1b[1;32m%s %d\x1b[m\n", buf1[i], buf, fasttime[i]);
    }
    vmsg(NULL);
    fclose(fp);
    return 0;
}

static int
load_fasttime(void)
{
    int i;
    char buf[40];
    FILE *fp;
    fp = fopen("game/mine/mine_fasttime", "r");
    for (i = 0; i < 4; i++)
    {
        fscanf(fp, "%s%d", buf, &fasttime[i]);
    }
    fclose(fp);
    return 0;
}

static int
change_fasttime(int n, int t)
{
    int i;
    char buf[4][40];
    FILE *fp;
    fp = fopen("game/mine/mine_fasttime", "r");
    for (i = 0; i < 4; i++)
    {
        fscanf(fp, "%s%d", buf[i], &fasttime[i]);
    }
    strcpy(buf[n], cuser.userid);
    fasttime[n] = t;
    fclose(fp);
    fp = fopen("game/mine/mine_fasttime", "w");
    for (i = 0; i < 4; i++)
    {
        fprintf(fp, "%s %d\n", buf[i], fasttime[i]);
    }
    fclose(fp);
    vmsg("你破記錄囉!!!!");
    return 0;
}

GCC_PURE static int countNeighbor(int y, int x, int bitmask)
{
    int sum = 0;
    if (MineMap[y-1][x+1] & bitmask)
        ++sum;
    if (MineMap[y-1][x  ] & bitmask)
        ++sum;
    if (MineMap[y-1][x-1] & bitmask)
        ++sum;
    if (MineMap[y  ][x+1] & bitmask)
        ++sum;
    if (MineMap[y  ][x  ] & bitmask)
        ++sum;
    if (MineMap[y  ][x-1] & bitmask)
        ++sum;
    if (MineMap[y+1][x+1] & bitmask)
        ++sum;
    if (MineMap[y+1][x  ] & bitmask)
        ++sum;
    if (MineMap[y+1][x-1] & bitmask)
        ++sum;
    return sum;
}

const char *const symTag = "\x1b[1;40;31mΨ\x1b[m";
const char *const symWrong = "\x1b[1;41;37m〤\x1b[m";
const char *const symBlank = "■";
const char *const strMines[] = {"　", "１", "２", "３", "４", "５",
                                "６", "７", "８", NULL,
                               };

enum {  MAP_START_X = 16 };             // Must be > Prompts

static time_t init_time = 0;

static void drawInfo(void)
{
    move(b_lines - 1, 0);
    clrtoeol();
    prints("所花時間: %.0lf 秒，剩下 %d 個地雷未標記.\n",
           difftime(time(0), init_time), TotalMines - TaggedMines);
}

static void drawPrompt(void)
{
    int i;
    for (i = 1; i <= 20; i++)
    {
        move(i, 0);
        clrtokol();
    }
    vs_bar("踩地雷");
    move(3, 0);
    outs("按鍵說明:"); clrtokol();
    move(4, 0);
    outs("移動     方向鍵"); clrtokol();
    move(5, 0);
    outs("翻開     空白鍵"); clrtokol();
    move(6, 0);
    outs("標記地雷   ｍ"); clrtokol();
    move(7, 0);
    outs("掃雷       ｔ"); clrtokol();
    move(9, 0);
    outs("離開  ESC-Esc/q"); clrtokol();
}

static void drawMapLine(int y, int flShow)
{
    int x = 0;
    drawInfo();
    move(y + 1, MAP_START_X);
    clrtokol();
    for (x = 1; x <= MAP_X; x++)
    {

        if (x == currx && y == curry)
            outs("\x1b[44;31m");

        if (MineMap[y][x] & TILE_TAGGED)
        {
            if (flShow && (MineMap[y][x] & TILE_MINE) == 0)
                outs(symWrong);
            else
                outs(symTag);
        }
        else if (MineMap[y][x] & TILE_EXPAND)
            outs(strMines[countNeighbor(y, x, TILE_MINE)]);
        else if (flShow && (MineMap[y][x] & TILE_MINE))
            outs(symTag);
        else
            outs(symBlank);

        if (x == currx && y == curry)
            outs("\x1b[m");
    }
    clrtoeol();

    move(curry + 1, currx*2 + MAP_START_X - 1);
}

static void drawMap(int flShow)
{
    int y;

    clear();
    drawPrompt();
    for (y = 1; y < MAP_Y + 1; y++)
    {
        drawMapLine(y, flShow);
    }
}

static int flLoseMine = 0;

static void loseMine(bool is_cheat)
{
    drawMap(1);
    game_log(1, "\x1b[31;1m在 %.01f 秒時%s啦!!!", difftime(time(0), init_time), (is_cheat) ? "自爆" : "踩到地雷");
    vmsg((is_cheat) ? "你自爆了" : "你輸了");
    flLoseMine = 1;
}

static void ExpandMap(int y, int x, int flTrace)
{
    if (!flTrace)
    {
        if (MineMap[y][x] & TILE_TAGGED || MineMap[y][x] & TILE_EXPAND)
            return;
        if ((MineMap[y][x] & TILE_MINE) && (!(MineMap[y][x] & TILE_TAGGED)))
            { loseMine(0); return; }
        MineMap[y][x] |= TILE_EXPAND;
        drawMapLine(y, 0);
    }
    if (flTrace || countNeighbor(y, x, TILE_MINE) == 0)
    {
        if (flTrace || (MineMap [y-1][x  ] & TILE_EXPAND) == 0)
            ExpandMap(y - 1, x, 0);
        if (flTrace || (MineMap [y  ][x-1] & TILE_EXPAND) == 0)
            ExpandMap(y, x - 1, 0);
        if (flTrace || (MineMap [y+1][x  ] & TILE_EXPAND) == 0)
            ExpandMap(y + 1, x, 0);
        if (flTrace || (MineMap [y  ][x+1] & TILE_EXPAND) == 0)
            ExpandMap(y, x + 1, 0);
        if (flTrace || (MineMap [y-1][x-1] & TILE_EXPAND) == 0)
            ExpandMap(y - 1, x - 1, 0);
        if (flTrace || (MineMap [y+1][x-1] & TILE_EXPAND) == 0)
            ExpandMap(y + 1, x - 1, 0);
        if (flTrace || (MineMap [y-1][x+1] & TILE_EXPAND) == 0)
            ExpandMap(y - 1, x + 1, 0);
        if (flTrace || (MineMap [y+1][x+1] & TILE_EXPAND) == 0)
            ExpandMap(y + 1, x + 1, 0);
    }
}

static void TraceMap(int y, int x)
{
    if (!(MineMap[y][x] & TILE_EXPAND))
        return;
    if (countNeighbor(y, x, TILE_MINE) == countNeighbor(y, x, TILE_TAGGED))
    {
        ExpandMap(y, x, 1);
    }
}

static void playMine(void)
{
    int ch;
    currx = MAP_X / 2 + 1, curry = MAP_Y / 2 + 1;
    flLoseMine = 0;
    move(2, 0);
    clrtobot();

    drawMap(0);
    while (!flLoseMine && ((ch = vkey()) != 'q'))
    {

        switch (ch)
        {
        case Meta(KEY_ESC):
            return;

        case KEY_UP:
            if (curry > 1)
            {
                drawMapLine(curry--, 0);
                drawMapLine(curry, 0);
            }
            break;

        case KEY_DOWN:
            if (curry < MAP_Y)
            {
                drawMapLine(curry++, 0);
            }
            break;

        case KEY_LEFT:
            if (currx > 1)
                currx--;
            break;

        case KEY_RIGHT:
            if (currx < MAP_X)
                currx++;
            break;

        case Ctrl('P'):
            drawMap(1);
            pressanykey(0);
            drawMap(0);
            loseMine(1);
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
            }

            MineMap[curry][currx] ^= TILE_TAGGED;
            if (TaggedMines == TotalMines)
                return;
            break;

        default:
            break;
        }
        drawMapLine(curry, 0);
    }
}

int Mine(void)
{
    DL_HOLD;
    int x, y, ti;
    char ans[5], buf[10];
    srand(time(0));
start:
    vs_bar("踩地雷");
    getdata(2, 0, "你要玩 [1]入門 [2]進階 [3]高級 [4]變態 [5]最快時間 [6]離開: ",
            ans, 4, DOECHO, "1");
    TotalMines = 0;
    switch (atoi(ans))
    {
    case 1:
        strcpy(buf, "入門級");
        MAP_X = 10;
        MAP_Y = 10;
        break;
    case 2:
        strcpy(buf, "進階級");
        MAP_X = 20;
        MAP_Y = 15;
        break;
    case 3:
        strcpy(buf, "高  級");
        MAP_X = 30;
        MAP_Y = 20;
        break;
    case 4:
        strcpy(buf, "變態級");
        MAP_X = 30;
        MAP_Y = 20;
        TotalMines = 40;
        break;
    case 5:
        show_fasttime();
        break;
    default:
        return DL_RELEASE(0);
    }
    if (atoi(ans) == 5)
        goto start;
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
                    { loseMine(0); y = MAP_Y + 1; break; }

        if (!flLoseMine)
        {
            move(b_lines - 1, 0);
            clrtoeol();
            ti = (int) difftime(time(0), init_time);
            outs("\x1b[1;37;44m你贏了!  ");
            prints("所花時間: %d 秒\x1b[m\n", ti);
            game_log(1, "\x1b[32;1m在 %s 花了 %d 秒清除地雷!!!", buf, ti);
            if (ti < fasttime[atoi(ans)-1])
                change_fasttime(atoi(ans) - 1, ti);
            vmsg(NULL);
        }
    }
    goto start;
}

