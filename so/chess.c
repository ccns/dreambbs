#include "bbs.h"

#define MSG_BAR "\033[34;46m [%s\033[34m][%s] \033[31;47m m:\033[30m翻,移動 \033[31mCtrl-S:\033[30m認輸 \033[31mCtrl-N:\033[30m重新 \033[31mCtrl-C:\033[30m離開 \033[m"

#ifdef EVERY_Z
extern int vio_fd, holdon_fd;
#endif

static int cfd;
static int Rule;
static int Row, Col;
static int TotalStep;
static int msgline;
static int GameOver;
static int myColor;
static int sideline;
static int Totalch; /* 加速用 :p */
static int Focus;
static int youreat_index;
static int myeat_index;

static int Board[19][17];
static int MyEat[16], YourEat[16];
static int Appear[14];

char *ruleStr[] = {"象棋", "暗棋"};
char *icon[] = {"┼", "●", "帥", "仕", "相", "硨", "傌", "炮", "兵",
				"將", "士", "象", "車", "馬", "包", "卒"
			   };
char *color[] = {"\033[31m紅", "\033[30m黑"};

enum{DISCONNECT = -2, LEAVE = -1, NOTHING = 0};
enum{Deny = -1, Empty = 0, Cover = 1};
enum{Red = 0, Black = 1};

static KeyFunc *mapTurn;
static KeyFunc myTurn[], yourTurn[];

static void
ch_printmsg(int index, char *msg)
{
	char buf[80];
	switch (index)
	{
	case 1:     /* for move uncover eat*/
		move(msgline + 10, 37); outs(msg); clrtoeol();
		if ((msgline += 1) > 9)
			msgline = 0;
		move(msgline + 10, 37); outs("→"); clrtoeol();
		break;
	case 2:     /* for select */
		sprintf(buf, "\033[1;33m◆您選取了 %s(%d, %c)\033[m", icon[Focus/1000]
				, Rule ? (Col - 1) / 2 : Col / 2, Rule ? (Row - 1) / 2 + 'A' : Row / 2 + 'A');
		move(1, 37); outs(buf); clrtoeol();
		break;
		//case 3: /* for eat */
	}
}

static void
ch_printeat()
{
	int i;
	/* my:4, your:7 */

	if (myeat_index)
	{
		for (i = 0; i < myeat_index; i++)
		{
			move(4, 37 + i * 2);
			outs(icon[MyEat[i]]);
		}
	}

	if (youreat_index)
	{
		for (i = 0; i < youreat_index; i++)
		{
			move(7, 37 + i * 2);
			outs(icon[YourEat[i]]);
		}
	}
}

static void
overgame()
{
	char buf[80];

	sprintf(buf, "%s獲勝!請按Ctrl-C重玩(測試中勝負不予紀錄)",
			GameOver == myColor ? "我方" : "對方");
	ch_printmsg(1, buf);
}

static char *
ch_brdline(int row)
{
	char *t;
	static char buf[512], river[] = "  楚      河          漢      界  ";
	static char side[] = " A B C D E F G H I J";
	int i;
	char *str = buf;

	for (i = 0; i < 17; i++)
	{
		t = icon[Board[row][i]];
		if (Board[row][i] == Empty)
		{
			if (row == 0)
			{
				if (i == 0)
					t = "┌";
				else if (i == 16)
					t = "┐";
				else if (i % 2)
					t = "─";
				else
					t = "┬";
			}
			else if (row == 18)
			{
				if (i == 0)
					t = "└";
				else if (i == 16)
					t = "┘";
				else if (i % 2)
					t = "─";
				else
					t = "┴";
			}
			else if (row == 9)
			{
				strcpy(buf, river);
				break;
			}
			else if (row % 2)
			{
				if (i % 2)
					t = "  ";
				else
					t = "│";
			}
			else
			{
				if (i == 0)
					t = "├";
				else if (i == 16)
					t = "┤";
				else if (i % 2)
					t = "─";
			}
		}
		strcpy(str, t); /* color */
		str += 2;
	} /* for loop */

	if (row != 9)
	{
		if (Rule)
		{
			if (row % 2 == 1)
			{
				i = row / 2;
				strncpy(str, side + i*2, 2);
			}
			else
				strcpy(str, "  ");
		}
		else
		{
			if (row % 2 == 0)
			{
				i = row / 2;
				strncpy(str, side + i*2, 2);
			}
			else
				strcpy(str, "  ");
		}
	}
	return buf;
}

static void
ch_draw()
{
	int i;
	char buf[256];

	for (i = 0; i < 19; i++)
	{
		if (Rule && (i > 8))
			break;

		move(1 + i, 0);
		outs(ch_brdline(i));
	}
	sprintf(buf, "☆我方所吃之子");move(3, 37);outs(buf);
	sprintf(buf, "★對方所吃之子");move(6, 37);outs(buf);
	sprintf(buf, "====================================");move(9, 37);outs(buf);

	move(sideline, 0);
	if (Rule)
		outs("  0   1   2   3   4   5   6   7");
	else
		outs(" 0   1   2   3   4   5   6   7   8");

	sprintf(buf, MSG_BAR, color[myColor],
			(mapTurn == myTurn) ? "換我了" : "換對方");
	move(21, 0);
	outs(buf);
}

static void
ch_init()
{
	char buf[256];
	int i, j;

	Totalch = TotalStep = GameOver = msgline = youreat_index = myeat_index = 0;
	Row = Col = Rule;
	sideline = Rule ? 10 : 20;
	Focus = 0;

	for (i = 0; i < 19; i++)
		for (j = 0; j < 17; j++)
			Board[i][j] = Empty;

	for (i = 0; i < 16; i++)
	{
		MyEat[i] = YourEat[i] = 0;
	}

	if (Rule)
	{
		for (i = 9; i < 19; i++)
			for (j = 0; j < 17; j++)
				Board[i][j] = Deny;

		for (i = 1; i < 9; i += 2)
			for (j = 1; j < 17; j += 2)
				Board[i][j] = Cover;

		memset(Appear, 0, sizeof(Appear));
	}
	else
	{
		/* 紅方 */
		Board[0][0] = Board[0][16] = 5;
		Board[0][2] = Board[0][14] = 6;
		Board[0][4] = Board[0][12] = 4;
		Board[0][6] = Board[0][10] = 3;
		Board[0][8] = Board[0][8] = 2;
		Board[4][2] = Board[4][14] = 7;
		Board[6][0] = Board[6][4] = Board[6][8] = Board[6][12] = Board[6][16]
									= 8;

		/* 黑方 */
		Board[18][0] = Board[18][16] = 12;
		Board[18][2] = Board[18][14] = 13;
		Board[18][4] = Board[18][12] = 11;
		Board[18][6] = Board[18][10] = 10;
		Board[18][8] = Board[18][8] = 9;
		Board[14][2] = Board[14][14] = 14;
		Board[12][0] = Board[12][4] = Board[12][8] = Board[12][12] = Board[12][16]
									  = 15;
	}

	for (i = 0; i < 16; i++)
	{
		MyEat[i] = YourEat[i] = Empty;
	}

	if (myColor == Red)
		mapTurn = myTurn;
	else
		mapTurn = yourTurn;

	clear();

	/* sprintf(buf, "【 對奕%s 】  ☆%s(%d戰%d勝) vs ★%s(%d戰%d勝)", ); */
	sprintf(buf, "\033[1;33;44m【 對奕-%s 】\033[0m", ruleStr[Rule]);
	outs(buf);

	ch_draw();
}

static inline int
ch_send(char *buf)
{
	int len;

	len = strlen(buf) + 1;
	return (send(cfd, buf, len, 0) == len);
}

static int
ch_recv()
{
	static char buf[256];
	char msg[80];
	int len, tmp, check;

	len  = sizeof(buf) + 1;
	if ((len = recv(cfd, buf, len, 0)) <= 0)
		return DISCONNECT;

	switch (*buf)
	{
	case 'Q':
		return LEAVE;
	case 'N':
		ch_init();
		break;
	case 'D':
		tmp = atoi(buf + 1);
		Board[(tmp%1000)/19][(tmp%1000)%19] = tmp / 1000;
		Appear[tmp/1000 - 2] += 1;
		Totalch += 1;
		mapTurn = myTurn;
		ch_draw();
		sprintf(buf, "\033[1;32m△對方翻開 %s (%d, %c)\033[m", icon[tmp/1000],
				(tmp % 1000 % 19 - 1) / 2, tmp % 1000 / 19 / 2 + 'A');
		ch_printmsg(1, buf);
		break;
	case 'E':
		tmp = atoi(strtok(buf + 1, ":"));
		Board[(tmp%1000)/19][(tmp%1000)%19] = Empty;
		len = atoi(strtok(NULL, ":"));
		YourEat[youreat_index] = Board[len/19][len%19];
		youreat_index += 1;
		sprintf(buf, "\033[1;32m▽對方 %s(%d, %c) 吃掉我方 %s (%d, %c)\033[m",
				icon[tmp/1000], Rule ? (tmp % 1000 % 19 - 1) / 2 : tmp % 1000 % 19 / 2,
				Rule ? (tmp % 1000 / 19 - 1) / 2 + 'A' : tmp % 1000 / 19 / 2 + 'A',
				icon[Board[len/19][len%19]], Rule ? (len % 19 - 1) / 2 : (len % 19) / 2,
				Rule ? (len / 19 - 1) / 2 + 'A' : len / 19 / 2 + 'A');
		check = Board[len/19][len%19];
		Board[len/19][len%19] = tmp / 1000;
		mapTurn = myTurn;
		ch_draw();
		ch_printmsg(1, buf);
		ch_printeat();
		if (Rule)
		{
			if (youreat_index == 16)
			{
				GameOver = !myColor;
				overgame();
			}
		}
		else
		{
			if (check == 2 || check == 9)
			{
				GameOver = !myColor;
				overgame();
			}
		}
		break;
	case 'M':
		tmp = atoi(strtok(buf + 1, ":"));
		Board[(tmp%1000)/19][(tmp%1000)%19] = Empty;
		len = atoi(strtok(NULL, ":"));
		Board[len/19][len%19] = tmp / 1000;
		mapTurn = myTurn;
		ch_draw();
		sprintf(msg, "\033[1;37m▽對方將 %s(%d, %c) 移至 (%d, %c)\033[m",
				icon[tmp/1000], Rule ? (tmp % 1000 % 19 - 1) / 2 : tmp % 1000 % 19 / 2,
				Rule ? (tmp % 1000 / 19 - 1) / 2 + 'A' : tmp % 1000 / 19 / 2 + 'A', Rule ? (len % 19 - 1) / 2 : (len % 19) / 2,
				Rule ? (len / 19 - 1) / 2 + 'A' : len / 19 / 2 + 'A');
		ch_printmsg(1, msg);
		break;
	case 'S':
		GameOver = myColor;
		overgame();
		break;
	}

	return NOTHING;
}

int ch_rand()
{
	int rd, i;
	char *index[] = {"1", "2", "2", "2", "2", "2", "5", "1", "2", "2", "2", "2", "2", "5"};

	if (Totalch == 31)  /* 避免剩最後一個時還要 random */
	{
		for (i = 0; i < 14; i++)
		{
			if (Appear[i] < atoi(index[i]))
			{
				i += 2;
				return i;
			}
		}
	}

	for (;;)
	{
		rd = 0;
		srandom(time(NULL));
		rd = random() % 16;

		if (rd < 2)
			continue;
		if (Appear[rd - 2] < atoi(index[rd - 2]))
		{
			Appear[rd - 2] += 1;
			Totalch += 1;
			break;
		}
		else
			continue;
	}
	return rd;
}

static int
ch_count(int FoRow, int FoCol)
{
	int count, start, end;

	count = 0;
	if (Row == FoRow)
	{
		(Col > FoCol) ? (start = FoCol) : (start = Col);
		(Col > FoCol) ? (end = Col) : (end = FoCol);
		for (start += 2; start < end - 1; start += 2)
		{
			if (Board[Row][start] != Empty)
				count++;
		}
	}
	else if (Col == FoCol)
	{
		(Row > FoRow) ? (start = FoRow) : (start = Row);
		(Row > FoRow) ? (end = Row) : (end = FoRow);
		for (start += 2; start < end - 1; start += 2)
		{
			if (Board[start][Col] != Empty)
				count++;
		}
	}
	return count;
}

static int
ch_check()  /* 相同:0   不同:1 */
{
	if (myColor == Red && Board[Row][Col] < 9)
		return 0;
	else if (myColor == Black && Board[Row][Col] > 8)
		return 0;

	return 1;
}

static int
ch_Mv0()    /* for 軍棋 */
{
	int mych, yourch, way;    /* way: 0. NOTHING  1.move  2.eat */
	int FoRow, FoCol, Rpos, Cpos;
	char buf[80];

	way = 0;
	FoRow = (Focus % 1000) / 19;
	FoCol = (Focus % 1000) % 19;
	if (Row == FoRow && Col == FoCol)
		return NOTHING;
	mych = Focus / 1000;
	yourch = Board[Row][Col];
	Rpos = FoRow > Row ? (FoRow - Row) : (Row - FoRow);
	Cpos = FoCol > Col ? (FoCol - Col) : (Col - FoCol);

	switch (mych)
	{
	case 2:
	case 9:
		if ((Col > 6 && Col < 10) && ((Rpos == 2 && FoCol == Col) ||
									  (Cpos == 2 && FoRow == Row)) && (Row < 5 || Row > 13))
		{
			if (yourch == Empty)
				way = 1;
			else if (ch_check())
				way = 2;
		}
		break;
	case 3:
	case 10:
		if ((Col > 5 && Col < 11) && (Rpos == 2) && (Cpos == 2) &&
			(Row < 5 || Row > 13))
		{
			if (yourch == Empty)
				way = 1;
			else if (ch_check())
				way = 2;
		}
		break;
	case 4:
	case 11:
		if (((Row < 9 && myColor == Red) || (Row > 9 && myColor == Black)) &&
			Rpos == 4 && Cpos == 4 && Board[(Row+FoRow)/2][(Col+FoCol)/2] == Empty)
		{
			if (yourch == Empty)
				way = 1;
			else if (ch_check())
				way = 2;
		}
		break;
	case 5:
	case 12:
		if (!ch_count(FoRow, FoCol) && (FoRow == Row || FoCol == Col))
		{
			if (Board[Row][Col] == Empty)
				way = 1;
			else if (ch_check())
				way = 2;
		}
		break;
	case 6:
	case 13:
		if ((Rpos == 4 && Cpos == 2 && Board[(Row+FoRow)/2][FoCol] == Empty) ||
			(Rpos == 2 && Cpos == 4 && Board[FoRow][(Col+FoCol)/2] == Empty))
		{
			if (Board[Row][Col] == Empty)
				way = 1;
			else if (ch_check())
				way = 2;
		}
		break;
	case 7:
	case 14:
		if (FoRow == Row || FoCol == Col)
		{
			if (ch_count(FoRow, FoCol) == 1 && ch_check())
				way = 2;
			else if (!ch_count(FoRow, FoCol) && Board[Row][Col] == Empty)
				way = 1;
		}
		break;
	case 8:
	case 15:
		if (myColor == Red)
		{
			if (Row < 9 && (Row - FoRow == 2) && FoCol == Col)
			{
				if (Board[Row][Col] == Empty)
					way = 1;
				else if (ch_check())
					way = 2;
			}
			else if (Row > 9 && ((Rpos == 2 && FoCol == Col) ||
								 (Cpos == 2 && FoRow == Row)))
			{
				if (Board[Row][Col] == Empty)
					way = 1;
				else if (ch_check())
					way = 2;
			}
		}
		else
		{
			if (Row > 9 && (FoRow - Row == 2) && FoCol == Col)
			{
				if (Board[Row][Col] == Empty)
					way = 1;
				else if (ch_check())
					way = 2;
			}
			else if (Row < 9 && ((Rpos == 2 && FoCol == Col) ||
								 (Cpos == 2 && FoRow == Row)))
			{
				if (Board[Row][Col] == Empty)
					way = 1;
				else if (ch_check())
					way = 2;
			}
		}
		break;
	}

	if (way)
	{
		if (way == 1)
		{
			Board[Row][Col] = mych;
			sprintf(buf, "M%d:%d", Focus, (Row * 19 + Col));
			if (!ch_send(buf))
				return DISCONNECT;
			Board[FoRow][FoCol] = Empty;
			mapTurn = yourTurn;
			ch_draw();
			sprintf(buf, "\033[1;36m▼您移動 %s(%d, %c) 至 (%d, %c)\033[m",
					icon[Focus/1000], Focus % 1000 % 19 / 2, Focus % 1000 / 19 / 2 + 'A',
					Col / 2, Row / 2 + 'A');
			ch_printmsg(1, buf);
		}
		else
		{
			MyEat[myeat_index] = Board[Row][Col];
			myeat_index += 1;
			Board[Row][Col] = mych;
			sprintf(buf, "E%d:%d", Focus, (Row * 19 + Col));
			if (!ch_send(buf))
				return DISCONNECT;
			Board[FoRow][FoCol] = Empty;
			mapTurn = yourTurn;
			ch_draw();
			sprintf(buf, "\033[1;32m▼您移動 %s(%d, %c) 吃掉 %s(%d, %c)\033[m",
					icon[Focus/1000], Focus % 1000 % 19 / 2, Focus % 1000 / 19 / 2 + 'A', icon[yourch],
					Col / 2, Row / 2 + 'A');
			ch_printmsg(1, buf);
			ch_printeat();
			if (yourch == 2 || yourch == 9)
			{
				GameOver = myColor;
				overgame();
			}
		}
	}
	Focus = 0;

	return NOTHING;
}

static int
ch_Mv1()    /* for 暗棋 */
{
	int mych, yourch;
	int FoRow, FoCol;
	char buf[80];

	FoRow = (Focus % 1000) / 19;
	FoCol = (Focus % 1000) % 19;
	mych = Focus / 1000;
	yourch = Board[Row][Col];

	if ((yourch != Empty) && ((mych == 7) || (mych == 14)) && ch_check())
	{
		if (ch_count(FoRow, FoCol) == 1)
		{
			MyEat[myeat_index] = Board[Row][Col];
			myeat_index += 1;
			Board[Row][Col] = mych;
			sprintf(buf, "E%d:%d", Focus, (Row * 19 + Col));
			if (!ch_send(buf))
				return DISCONNECT;
			Board[FoRow][FoCol] = Empty;
			mapTurn = yourTurn;
			ch_draw();
			sprintf(buf, "\033[1;32m▼您移動 %s(%d, %c) 吃掉 %s(%d, %c)\033[m",
					icon[Focus/1000], Focus % 1000 % 19 / 2, Focus % 1000 / 19 / 2 + 'A', icon[yourch],
					Col / 2, Row / 2 + 'A');
			ch_printmsg(1, buf);
			ch_printeat();
		}
	}
	else if ((((Row - FoRow == 2) || (FoRow - Row == 2)) && (Col == FoCol)) ||
			 (((Col - FoCol == 2) || (FoCol - Col == 2)) && (Row == FoRow)))
	{
		if (yourch == Empty)
		{
			Board[Row][Col] = mych;
			sprintf(buf, "M%d:%d", Focus, (Row * 19 + Col));
			if (!ch_send(buf))
				return DISCONNECT;
			Board[FoRow][FoCol] = Empty;
			mapTurn = yourTurn;
			ch_draw();
			sprintf(buf, "\033[1;36m▼您將 %s(%d, %c) 移至 (%d, %c)\033[m",
					icon[Focus/1000], (Focus % 1000 % 19 - 1) / 2, (Focus % 1000 / 19 - 1) / 2 + 'A',
					(Col - 1) / 2, (Row - 1) / 2 + 'A');
			ch_printmsg(1, buf);
		}
		else if (ch_check())
		{
			if (myColor == Black)
				mych -= 7;
			else
				yourch -= 7;
			if (((mych == 8) && (yourch == 2)) || (mych <= yourch))
			{
				if ((mych == 7) && (yourch == 7))
					return NOTHING;
				if ((mych == 2) && (yourch == 8))
					return NOTHING;
				MyEat[myeat_index] = Board[Row][Col];
				myeat_index += 1;
				Board[Row][Col] = Focus / 1000;
				sprintf(buf, "E%d:%d", Focus, (Row * 19 + Col));
				if (!ch_send(buf))
					return DISCONNECT;
				Board[FoRow][FoCol] = Empty;
				mapTurn = yourTurn;
				ch_draw();
				sprintf(buf, "\033[1;32m▼您移動 %s(%d, %c) 吃掉 %s(%d, %c)\033[m",
						icon[Focus/1000], Focus % 1000 % 19 / 2, Focus % 1000 / 19 / 2 + 'A',
						icon[MyEat[myeat_index-1]], Row / 2, Col / 2 + 'A');
				ch_printmsg(1, buf);
				ch_printeat();
			}
		}
	}
	Focus = 0;

	if (myeat_index == 16)
	{
		GameOver = myColor;
		overgame();
	}
	return NOTHING;
}

static int
chCtrlN()
{
	if (!ch_send("N"))
		return DISCONNECT;
	ch_init();
	return NOTHING;
}

static int
chCtrlC()
{
	if (!ch_send("Q"))
		return DISCONNECT;
	return LEAVE;
}

static int
chCtrlS()
{
	if (!ch_send("S"))
		return DISCONNECT;
	GameOver = !myColor;
	overgame();
	return NOTHING;
}

static int
chLEFT()
{
	if ((Col - 2) > -1)
		Col -= 2;
	return NOTHING;
}

static int
chRIGHT()
{
	if ((Col + 2) < 17)
		Col += 2;
	return NOTHING;
}

static int
chUP()
{
	if ((Row - 2) > -1)
		Row -= 2;
	return NOTHING;
}

static int
chDOWN()
{
	if (Board[Row + 2][Col] != Deny)
		if ((Row + 2) < 19)
			Row += 2;
	return NOTHING;
}

static int
chSpace()
{
	char buf[40];

	if (Board[Row][Col] == Cover)
	{
		Board[Row][Col] = ch_rand();
		sprintf(buf, "D%d", Board[Row][Col] * 1000 + Row * 19 + Col);
		if (!ch_send(buf))
			return DISCONNECT;

		mapTurn = yourTurn;
		Focus = Empty;
		ch_draw();
		sprintf(buf, "\033[1;32m▲您翻開 %s(%d, %c)\033[m",
				icon[Board[Row][Col]], (Col - 1) / 2, (Row - 1) / 2 + 'A');
		ch_printmsg(1, buf);
	}
	else
	{
		if (Focus)
			return Rule ? ch_Mv1() : ch_Mv0();
		else
		{
			if (Board[Row][Col] != Empty && !ch_check())
			{
				Focus = Board[Row][Col] * 1000 + Row * 19 + Col;
				ch_printmsg(2, NULL);
			}
		}
	}
	return NOTHING;
}

static int
NoOp()
{
	return NOTHING;
}

static KeyFunc
yourTurn[] =
{
	{Ctrl('N'), chCtrlN},
	{Ctrl('C'), chCtrlC},
	{KEY_LEFT, chLEFT},
	{KEY_RIGHT, chRIGHT},
	{KEY_UP, chUP},
	{KEY_DOWN, chDOWN},
	{0, NoOp}
},
myTurn[] =
{
	{Ctrl('N'), chCtrlN},
	{Ctrl('C'), chCtrlC},
	{Ctrl('S'), chCtrlS},
	{KEY_LEFT, chLEFT},
	{KEY_RIGHT, chRIGHT},
	{KEY_UP, chUP},
	{KEY_DOWN, chDOWN},
	{'m', chSpace},
	{' ', chSpace},
	{0, NoOp}
};

int MainChess(int sock, int later)
{
	screenline sl[b_lines + 1];
	int ch, tmpmode;
	char c;
	KeyFunc *k;

	vs_save(sl);
	cfd = sock;
	if (!later)
	{
		c = vans("想下哪種棋? 0)取消 1)象棋 2)暗棋 [0]");
		if (c) c -= '0';

		vs_restore(sl);

		if (send(cfd, &c, 1, 0) != 1)
			return -2;

		myColor = Red;
	}
	else
	{
		outz("對方要求進入對奕模式,選擇中請稍候.....");
		refresh();
		if (recv(cfd, &c, 1, 0) != 1)
			return -2;

		vs_restore(sl);
		myColor = Black;
	}

	if (c < 1 || c > 2)
		return -1;
	else
		c--;

	tmpmode = bbsmode;
	utmp_mode(M_CHESS);
	Rule = c;
	ch_init();

	for (;;)
	{
		move(Row + 1, Col * 2);

		k = mapTurn;

		ch = vkey();
		if (ch == I_OTHERDATA)
		{
			if ((ch = ch_recv()) >= NOTHING)
				continue;
			vs_restore(sl);
			return ch;
		}
#ifdef EVERY_Z
		else if (ch == Ctrl('Z'))
		{
			char buf[IDLEN + 1];

			strcpy(buf, cutmp->mateid);
			holdon_fd = vio_fd;
			vio_fd = 0;
			every_Z();
			vio_fd = holdon_fd;
			holdon_fd = 0;

			strcpy(cutmp->mateid, buf);
			continue;
		}
#endif
		for (;;k++)
		{
			if (!k->key || ch == k->key)
				break;
		}

		if ((ch = k->key ? (*k->func)() : (*k->func)(ch)) >= NOTHING)
			continue;
		vs_restore(sl);
		utmp_mode(tmpmode);
		return 0;
	}
}

#include<stdarg.h>
int vaChess(va_list pvar)
{
	int sock, later;
	sock = va_arg(pvar, int);
	later = va_arg(pvar, int);
	return MainChess(sock, later);
}
