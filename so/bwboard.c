/*-------------------------------------------------------*/
/* bwboard.c    ( NTHU CS MapleBBS Ver 3.10 )            */
/*-------------------------------------------------------*/
/* author : thor.bbs@bbs.cs.nthu.edu.tw                  */
/* target : Black White Chess Board dynamic link module  */
/* create : 99/02/20                                     */
/* update : 99/12/30                                     */
/*-------------------------------------------------------*/

#include "bbs.h"

extern char lastcmd[MAXLASTCMD][80];

static int bwline;            /* Where to display message now */
static int cfd; /* socket number */
static int myColor; /* my chess color */
static int GameOver; /* end game */

/* Thor.990311: dynamic attribute key */
static int key_total; /* attribute key for total */
static int key_win; /* attribute key for win */

#define stop_line (b_lines - 3)

#ifdef EVERY_Z
extern int vio_fd, holdon_fd;
#endif
static KeyFunc Talk[];
static KeyFunc myTurn[], yourTurn[];

enum { Empty = 0, Black = 1, White = 2, Deny = 3};
static char *icon[] = {"¢q", "¡´", "¡³", "  " };
static int othAllow(void);

/* 19 * 19, standard chess board */
char Board[19][19];
char Allow[19][19];
int numWhite, numBlack;
int rulerRow, rulerCol;

static int yourPass, myPass;
static int yourStep, myStep;
char whatsay[128], noansi[80];

enum {DISCONNECT = -2, LEAVE = -1, NOTHING = 0};

void
log_chess()
{
	FILE *fp;
	char fpath[80];
	usr_fpath(fpath, cuser.userid, "chess.log");
	fp = fopen(fpath, "w");
	fprintf(fp, "\n    ³o¬O§Ú­Ìªº¾ÔÁZ ^^y\n");
	fprintf(fp, "    %s\n", whatsay);
	fclose(fp);
	keeplog(fpath, BRD_SPECIAL, noansi, 2);
}

#if 0

rules util include
0. Board initialize
(happen before start, and clear board action)
1. Board update by "color" down a "chess pos"(without update screen)
(screen by a main redraw routine, after call this)
return value, represent win side to end(for both turn)
	   (happen when state change)
	   2. Board allow step checking by "color"(for my turn)
	   return value, represent # possible step
			  (happen when state change)
			  if Game is over, won''t allow any step

#endif

			  enum{Binit = 0, Bupdate, Ballow};

static int (**rule)();

/* numWhite, numBlack maintained by rule, 0&1 */
/* Board[19][19] Allow[19][19] maintained by rule */

static inline void countBWnum()
{
	int i, j;
	numWhite = numBlack = 0;
	for (i = 0; i < 19;i++)
		for (j = 0; j < 19;j++)
			if (Board[i][j] == White)
				numWhite ++;
			else if (Board[i][j] == Black)
				numBlack ++;
}

/* Rules for othello, 8 x 8 */
static int othInit()
{
	int i, j;
	for (i = 0;i < 19;i++)
		for (j = 0;j < 19;j++)
			Board[i][j] = i < 8 && j < 8 ? Empty : Deny;
	Board[3][3] = Board[4][4] = Black;
	Board[3][4] = Board[4][3] = White;
	numWhite = numBlack = 2;
	rulerRow = rulerCol = 8;
	return 0;
}

static inline int othEatable(int Color, int row, int col, int rowstep, int colstep)
{
	int eat = 0;
	do
	{
		row += rowstep;
		col += colstep;
		/* check range */
		if (row < 0 || row >= 8 || col < 0 || col >= 8) return 0;
		if (Board[row][col] == Color) return eat;
		eat = 1;
	}
	while (Board[row][col] == Deny - Color);
	return 0;
}

static int othUpdate(int Color, int row, int col)
{
	int i, j, p, q;
	int winside = Empty;
	Board[row][col] = Color;
	for (i = -1; i <= 1; i++)
		for (j = -1; j <= 1; j++)
			if (i != 0 || j != 0)
				if (othEatable(Color, row, col, i, j))
				{
					p = row + i;
					q = col + j;
					for (;;)
					{
						if (Board[p][q] == Color) break;
						Board[p][q] = Color;
						p += i;
						q += j;
					}
				}
	/* count numWhite & numBlack */
	countBWnum();

	/* Thor.990329: µù¸Ñ, ¤Uº¡®É */
	/* if(numWhite + numBlack == 8*8) */
	{
		int my = myColor; /* Thor.990331: ¼È¦smyColor */
		int allowBlack, allowWhite;
		myColor = Black;
		allowBlack = othAllow();
		myColor = White;
		allowWhite = othAllow();
		myColor = my;
		if (allowBlack == 0 && allowWhite == 0)
		{

			if (numWhite > numBlack)
				winside = White;
			else if (numWhite < numBlack)
				winside = Black;
			else
				winside = Deny;
		}
	}
#if 0
	/* Thor.990329: ¨S¤l¤U®É (·PÁÂ ghoster ´£¨Ñ¦l©Û:P) */
	else if (numWhite == 0)
	{
		winside = Black;
	}
	else if (numBlack == 0)
	{
		winside = White;
	}
#endif
	return winside;
}

static int othAllow(/*int Color,that is myColor*/)
{
	int i, j, p, q, num = 0;
	for (i = 0;i < 8;i++)
		for (j = 0;j < 8;j++)
		{
			Allow[i][j] = 0;
			if (Board[i][j] == Empty)
			{
				for (p = -1;p <= 1;p++)
					for (q = -1;q <= 1;q++)
						if (p != 0 || q != 0)
						{
							if (othEatable(myColor, i, j, p, q))
							{
								Allow[i][j] = 1;
								num++;
								goto next;
							}
						}
			}
next:
			;
		}
	return num;
}

static int (*othRule[])() = {othInit, othUpdate, othAllow};

/* Rules for five chess, 15 x 15 */
static int fivInit()
{
	int i, j;
	for (i = 0;i < 19;i++)
		for (j = 0;j < 19;j++)
			Board[i][j] = i < 15 && j < 15 ? Empty : Deny;
	numWhite = numBlack = 0;
	rulerRow = rulerCol = 15;
	return 0;
}

#if 0
#define LIVE_SIDE 0x10
#endif

static int fivCount(int Color, int row, int col, int rowstep, int colstep)
{
	int count = 0;
	for (;;)
	{
		row += rowstep;
		col += colstep;
		/* check range */
		if (row < 0 || row >= 15 || col < 0 || col >= 15) return count;
#if 0
		/* Thor.990415: §PÂ_¦³»~, ¨£¤U */
		/* Thor.990414: check live side */
		if (Board[row][col] == Empty) return count | LIVE_SIDE;
#endif
		if (Board[row][col] != Color) return count;
		count++;
	}
}

static int fivUpdate(int Color, int row, int col)
{
#if 0
	int cnt[4], n3, n4, n5, nL, i;
#endif

	int winside = Empty;
	Board[row][col] = Color;
	if (Color == Black) numBlack++;
	else if (Color == White) numWhite++;

#if 0
	¡° ¤Þ­z¡mDaimon(¦N¨Æ¬v½µ»Ä¶À¥Ê)¡n¤§»Ê¨¥¡G
	/* Thor.990414:µù:­Dªº¤HnickÁ`¬O¨ú³oºØ :p */
	¶Â´Ñ¡]¥ýµÛªÌ¡ ^ ¦³¤U¦C¤TµÛ¸TµÛ(¤SºÙ¸T¤â)ÂI¡G¤T¤T(Âù¬¡¤T)¡B¥ | ¥ | ¡Bªø³s¡C
	¦b³s¤­¤§«e§Î¦¨¸TµÛªÌ¡Aµô©w¬°¸TµÛ­t¡C
	¥Õ´Ñ¨S¦³¸TµÛÂI¡Aªø³s©ÎªÌ¤T¤T¤]³£¥i¥H³Ó¡C
	¤W­±³o¬q¤å³¹¨ú¦Û©ó¡§¤­¤l´Ñ¾Ç° | ¡§
http://www.tacocity.com.tw/shosho/index2.htm
	¸Ì­±¦³¸Ô²Óªº³W«h
#endif
#if 0
	cnt[0] = fivCount(Color, row, col, -1, -1) + fivCount(Color, row, col, + 1, + 1) + 1;
			 cnt[1] = fivCount(Color, row, col, -1, 0) + fivCount(Color, row, col, + 1, 0) + 1;
					  cnt[2] = fivCount(Color, row, col, 0, -1) + fivCount(Color, row, col, 0, + 1) + 1;
							   cnt[3] = fivCount(Color, row, col, -1, + 1) + fivCount(Color, row, col, + 1, -1) + 1;

										n3 = 0; /* Âù¬¡¤T */
											 n4 = 0; /* Âù¥| */
												  n5 = 0; /* ¤­ */
													   nL = 0; /* ªø³s */

															for (i = 0; i < 4; i++)
{
	if (cnt[i] == (3 | (LIVE_SIDE + LIVE_SIDE))) n3++;
		if ((cnt[i] % LIVE_SIDE) == 4) n4++;
		if ((cnt[i] % LIVE_SIDE) == 5) n5++;
		if ((cnt[i] % LIVE_SIDE) > 5) nL++;
	}

	if (n5 > 0)
	winside = Color;
			  else
	{
		if (Color == Black)
			{
				static void printbwline();
				if (n3 >= 2)
				{
					printbwline("¡» ¶Â¤èÂù¤T¸TµÛ");
					winside = White;
				}
				if (n4 >= 2)
				{
					printbwline("¡» ¶Â¤èÂù¥|¸TµÛ");
					winside = White;
				}
				if (nL > 0)
				{
					printbwline("¡» ¶Â¤èªø³s¸TµÛ");
					winside = White;
				}
			}
			else
			{
				if (nL > 0)
					winside = Color;
			}
		}
#endif
#if 0

	/* Thor.990415: ¤W­±¨º¬q¤S¼g¿ù¤F, ¯d«Ý¦³¤ß¤H¤h¦AºCºC¼g§a:pp */

	§@ªÌ  tanx(¹G¹G¬°¼Z¸¨¤§¥»)                                ¬ÝªO  sysop
¼ÐÃD  Re: ¤­¤l´Ñ¦³bug
®É¶¡  Thu Apr 15 01: 37: 19 1999
	¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w

	¶â....§Ú­Ó¤H»{¬°¦bª±¤­¤l´Ñ®É, ³Ì¦n¥i¥H¦Û¥Ñ¿ï¾Ü¬O§_­n¦³³W«h
				  ¹ï·~¾lªº¤H¨Ó»¡, ¦³¤F¸T¤â¤Ï¦Ó· | ÅÜ±o¨S¦³¼Ö½ì, ¦³¤H´NÄ±±opttªº
				  ¤­¤l´Ñ¤£¦nª±(¦³¸T¤â).
				  ¦Ü©ó³W«h§Ú¨ì¤FDaimon´£¨Ñªººô¯¸¬Ý¤F¤@¤U, ¥u­n¬O¥ | ¥ | ¬O¥H¤U´X
				  ºØ±¡§Î

				  (¤@)¡´  ¡´  ¡´  ¡´
				  ¡ô
				  ¦A©ñ¶i¥h´Nºâ¥ | ¥ |
				  ¡õ
				  (¤G)¡´¡´¡´      ¡´¡´¡´(¤¤¶¡ªÅ¤T®æ)

				  (¤T)¡´  ¡´¡´¡³
				  ¡ô©ñ³o¸Ì¤]ºâ
	¡´
	¡´
	¡´

	(¥ |)¡´¡´  ¡´    ¡´¡´
	¡ô¦A©ñ´N¬O¥ | ¥ |


	ªø³sªº¸Ü, ¥u­n¤­¤l¥H¤W´Nºâ, ¤£ºÞ¦º¬¡


#endif
#if 1
	if (fivCount(Color, row, col, -1, -1) + fivCount(Color, row, col, + 1, + 1) >= 4
		|| fivCount(Color, row, col, -1, 0) + fivCount(Color, row, col, + 1, 0) >= 4
		|| fivCount(Color, row, col, 0, -1) + fivCount(Color, row, col, 0, + 1) >= 4
		|| fivCount(Color, row, col, -1, + 1) + fivCount(Color, row, col, + 1, -1) >= 4)
	winside = Color;
#endif
			  return winside;
				 }

					 static int fivAllow(/*int Color*/)
{
	int i, j, num = 0;
	for (i = 0;i < 19;i++)
			for (j = 0;j < 19;j++)
				num += Allow[i][j] = (Board[i][j] == Empty);
		return num;
	}

	static int (*fivRule[])() = { fivInit, fivUpdate, fivAllow};

								/* Rules for blocking chess, 19 x 19 */
								static int blkInit()
	{
		memset(Board, 0, sizeof Board);
		numWhite = numBlack = 0;
		rulerRow = rulerCol = 18;
		return 0;
	}

	/* borrow Allow for traversal, and return region */
	/* a recursive procedure, clear Allow before call it*/
	/* with row,col range check, return false if out */
	static int blkLive(int Color, int row, int col)
	{
		if (row < 0 || row >= 19 || col < 0 || col >= 19) return 0;
		if (Board[row][col] == Empty) return 1;
		if (Board[row][col] != Color) return 0;
		if (Allow[row][col]) return 0;
		Allow[row][col] = 1;
		return blkLive(Color, row - 1, col) |
			   blkLive(Color, row + 1, col) |
			   blkLive(Color, row, col - 1) |
			   blkLive(Color, row, col + 1);
	}

	static inline void blkClear()
	{
		int i, j;
		for (i = 0;i < 19;i++)
			for (j = 0;j < 19;j++)
				if (Allow[i][j]) Board[i][j] = Empty;
	}

	static int blkUpdate(int Color, int row, int col)
	{
		Board[row][col] = Color;

#if 0
		/* Thor.990321: a little mistake:p
		    XO
		   XO.O
		    XO   X can eat O in "."
		 */
		/* check for suiside */
		memset(Allow, 0, sizeof Allow);
		if (!blkLive(Color, row, col))
			blkClear();
		else
		{
			memset(Allow, 0, sizeof Allow);
			if (!blkLive(Deny - Color, row - 1, col))
				blkClear();

			memset(Allow, 0, sizeof Allow);
			if (!blkLive(Deny - Color, row + 1, col))
				blkClear();

			memset(Allow, 0, sizeof Allow);
			if (!blkLive(Deny - Color, row, col - 1))
				blkClear();

			memset(Allow, 0, sizeof Allow);
			if (!blkLive(Deny - Color, row, col + 1))
				blkClear();
		}
#endif
		memset(Allow, 0, sizeof Allow);
		if (!blkLive(Deny - Color, row - 1, col))
			blkClear();

		memset(Allow, 0, sizeof Allow);
		if (!blkLive(Deny - Color, row + 1, col))
			blkClear();

		memset(Allow, 0, sizeof Allow);
		if (!blkLive(Deny - Color, row, col - 1))
			blkClear();

		memset(Allow, 0, sizeof Allow);
		if (!blkLive(Deny - Color, row, col + 1))
			blkClear();

		/* check for suiside */
		memset(Allow, 0, sizeof Allow);
		if (!blkLive(Color, row, col))
			blkClear();

		/* count numWhite & numBlack */
		countBWnum();

		return Empty; /* Please check win side by your own */
	}

	/* borrow fivAllow as blkAllow */

	static int (*blkRule[])() = { blkInit, blkUpdate, fivAllow};

								/* rule set */
								static int (**ruleSet[])() = {othRule, fivRule, blkRule};
															 static char *ruleStrSet[] = {"¶Â¥Õ´Ñ", "¤­¤l´Ñ", "³ò´Ñ"};
																						 static char *ruleStr;

																						 /* board util */
#if 0

																					 screen:
																						 [maple BWboard]   xxxx vs yyyy
																						 ++++++++  talkline(you color, yellow)(40 chars)
																						 ++++++++  talkline(my color, cryn)
																						 ++++++++
																						 ++++++++
																						 ++++++++
																						 1 line for simple help, press key to ......
																						 1 line for nth turn, myColor, num, pass < - youcolor, num, pass(35)input talk
																							 2 line for write msg

																					 state:
																								 one step
																								 [my turn]--------[your turn]      mapTalk = NULL
																										 tab |             | tab
																										 + -- +       + -- +
																										 [talk mode]                mapTalk = map

#endif

																																			  static KeyFunc * mapTalk, *mapTurn;

																																			  static int bwRow, bwCol;
																																			  static int cmdCol, cmdPos;
																																			  static char talkBuf[42] = "T";
																																										static char *cmdBuf = &talkBuf[1];

																																															  static char *
																																															  bw_brdline(int row)
		{
			static char buf[80] = "\033[30;43m";
				static char rTxtY[] = " A B C D E F G H I J K L M N O P Q R S";
				static char rTxtX[] = " 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9";
				char txt[3];
				char *ptr = buf + 8, *t;
				int i;

				for (i = 0; i < 19; i++)
				{
					t = icon[(int)Board[row][i]];
					if (t == icon[Empty])
					{
						if (row == 0)
						{
							if (i == 0)
								t = "ùÝ";
							else if (i >= 18 || Board[row][i+1] == Deny)
								t = "ùß";
							else
								t = "ùç";
						}
						else if (row >= 18 || Board[row+1][i] == Deny)
						{
							if (i == 0)
								t = "ùã";
							else if (i >= 18 || Board[row][i+1] == Deny)
								t = "ùå";
							else
								t = "ùí";
						}
						else
						{
							if (i == 0)
								t = "ùò";
							else if (i >= 18 || Board[row][i+1] == Deny)
								t = "ùô";
						}
					}
					if (t != icon[Black] && t != icon[White])
					{
						if (row == rulerRow && i < rulerCol)
						{
							str_ncpy(txt, rTxtX + 2 *i , 3);
							t = txt;
						}
						else if (i == rulerCol && row < rulerRow)
						{
							str_ncpy(txt, rTxtY + 2 * row , 3);
							t = txt;
						}
					}
					strcpy(ptr, t);
					ptr += 2;
				}
				strcpy(ptr, "\033[m");
				return buf;
			}

	static char*
	bw_coorstr(row, col)
	int row;
	int col;
	{
		static char coor[10];
		sprintf(coor, "(%c,%d)", row + 'A', col + 1);
		return coor;
	}

	static void
	printbwline(msg)
	char *msg;
	{
		int line;

		line = bwline;
		move(line, 0);
		outs(bw_brdline(line - 1)); outs(msg); clrtoeol();
		if (++line == stop_line)
			line = 1;
		move(line, 0);
		outs(bw_brdline(line - 1)); outs("¡÷"); clrtoeol();
		bwline = line;
	}

	static void
	bw_draw()
	{
		int i, myNum, yourNum;
		char buf[80];
		for (i = 0;i < 19;i++)
		{
			move(1 + i, 0);
			outs(bw_brdline(i));
		}
		myNum = myColor == Black ? numBlack : numWhite;
		yourNum = myColor == Black ? numWhite : numBlack;

		sprintf(buf, "²Ä%d¦^ %s%d¤l%dÅý"
				" (%s) %s%d¤l%dÅý",
				BMIN(myStep, yourStep) + 1, icon[Deny - myColor], myNum, myPass,
				(mapTurn == myTurn ? "¡ö" : "¡÷"), icon[myColor], yourNum, yourPass);
		/* Thor.990219: ¯S§Oª`·N, ¦b¦¹¦]ÃC¦âÃö«Y, ¬G¥Î¤lªºicon, ­è¦n»P­ì¥»¬Û¤Ï */
		move(21, 0); outs(buf);
		/* nth turn,myColor,num,pass <- youcolor, num,pass */
	}

	static void
	bw_init()
	{
		char *t, msg[160], buf[80];
		int i, myTotal = 0, yourTotal = 0, myWin = 0, yourWin = 0;

		/* Initialize BWboard */
		(*rule[Binit])();

		yourPass = myPass = yourStep = myStep = 0;
		/* Initialize state */
		mapTalk = Talk;

		if (myColor == Black)
		{
			(*rule[Ballow])();
			mapTurn = myTurn;
		}
		else
		{
			mapTurn = yourTurn;
		}
		/* Initialize screen */
		clear();

		/* sprintf(buf,"¡¸%s vs ¡¹%s \033[m",cuser.userid, cutmp->mateid); */
		/* Thor.990311: dyanmic attribute */
		attr_get(cuser.userid, key_total, &myTotal);
		attr_get(cuser.userid, key_win, &myWin);
		attr_get(cutmp->mateid, key_total, &yourTotal);
		attr_get(cutmp->mateid, key_win, &yourWin);

		sprintf(buf, "¡¸%s(%d¾Ô%d³Ó) vs ¡¹%s(%d¾Ô%d³Ó) \033[m", cuser.userid, myTotal, myWin, cutmp->mateid, yourTotal, yourWin);
		sprintf(noansi, "¡i ¹ï«³%s ¡j %s vs %s", ruleStr, cuser.userid, cutmp->mateid);
		strcpy(whatsay, buf);
		sprintf(msg, "\033[1;33;44m¡i ¹ï«³%s ¡j", ruleStr);
		i = 80 - strlen(buf) + 3 - strlen(msg) + 10;
		t = str_tail(msg);
		for (;i;i--) *t++ = ' ';
		strcpy(t, buf);
		outs(msg);

#if 1
		sprintf(msg, "(sock:%d,icon:%s)%s %s", cfd, icon[myColor], ruleStr, buf);
		blog("BWBOARD", msg);
#endif

		move(20, 0);
		outs("\033[34;46m ¹ï«³¼Ò¦¡ \033[31;47m ( ,Enter)\033[30m¸¨¤l \033[31m(TAB)\033[30m¤Á´«´Ñ½L/¥æ½Í \033[31m(^P)\033[30mÅý¤â \033[31m(^C)\033[30m­«ª± \033[31m(^D)\033[30mÂ÷¶} \033[m");
		bw_draw();

		bwRow = bwCol = 0;
		cmdCol = 0;
		*cmdBuf = 0;

		bwline = 1;
		GameOver = Empty;
		cmdPos = -1;
	}

	static inline void
	overgame()
	{
		if (GameOver == Black)
			printbwline("\033[1;32m¡» ¶Â¤èÀò³Ó\033[m");
		else if (GameOver == White)
			printbwline("\033[1;32m¡» ¥Õ¤èÀò³Ó\033[m");
		else if (GameOver == Deny)
			printbwline("\033[1;32m¡» Âù¤è¥­¤â\033[m");

		/* Thor.990311: dyanmic attribute */
		attr_step(cuser.userid, key_total, 0, + 1);
#if 0
		myTotal++;
		attr_put(cuser.userid, key_total, &myTotal);
#endif
		if (myColor == GameOver)
		{
			attr_step(cuser.userid, key_win, 0, + 1);
#if 0
			myWin++;
			attr_put(cuser.userid, key_win, &myWin);
#endif
		}
	}

	static inline int
	bw_send(buf)
	char *buf;
	{
		int len;

		len = strlen(buf) + 1; /* with trail 0 */
		return (send(cfd, buf, len, 0) == len);
	}

#ifdef EVERY_BIFF
	static void check_biff()
	{
		/* Thor.980805: ¦³¤H¦b®ÇÃä«öenter¤~»Ý­ncheck biff */
		static int old_biff;
		int biff = cutmp->ufo & UFO_BIFF;
		if (biff && !old_biff)
			printbwline("¡» ¾´! ¶l®t¨Ó«ö¹a¤F!");
		old_biff = biff;
	}
#endif

#if 0

communication protocol:
	Ctrl('A') enter BWboard mode, (pass to another)
	first hand specify rule set(pass #Rule later)
	then start

	clear chess board, C.....\0
	talk line by line, T.....\0
	specify down pos, Dxxx\0 , y = xxx / 19, x = xxx % 19
								   pass one turn, P.....\0
								   leave BWboard mode, Q.....\0

#endif

								   static inline int
								   bw_recv()
	{
		static char buf[512];
		static int bufstart = 0;
		int cc, len;
		char *bptr, *str;
		char msg[80];
		int i;

		bptr = buf;
		cc = bufstart;
		len = sizeof(buf) - cc - 1;

		if ((len = recv(cfd, bptr + cc, len, 0)) <= 0)
			return DISCONNECT;

		cc += len;

		for (;;)
		{
			len = strlen(bptr);

			if (len >= cc)
			{
				/* wait for trailing data */
				memcpy(buf, bptr, len);
				bufstart = len;
				break;
			}
			str = bptr + 1;
			switch (*bptr)
			{
				/* clear chess board, C.....\0 */
			case 'C':
				bw_init();
				break;

				/* talk line by line, T.....\0 */
			case 'T':
				sprintf(msg, "\033[1;33m¡¹%s\033[m", str);
				printbwline(msg);
				break;

				/* specify down pos, Dxxx\0 , y = xxx / 19, x = xxx % 19 */
			case 'D':
				yourStep++;
				/* get pos */
				i = atoi(str);
				sprintf(msg, "¡» ¹ï¤è¸¨¤l %s", bw_coorstr(i / 19, i % 19));
				/* update board */
				GameOver = (*rule[Bupdate])(Deny - myColor, i / 19, i % 19);

				mapTurn = myTurn;

				bw_draw();

				printbwline(msg);

				if (GameOver)
				{
					overgame();
					memset(Allow, 0, sizeof Allow);
				}
				else
				{
					if ((*rule[Ballow])() <= 0)
						printbwline("¡» ±z¨«§ëµL¸ô¤F");
				}
				break;

				/* pass one turn, P.....\0 */
			case 'P':
				yourPass++; yourStep++;

				mapTurn = myTurn;
				bw_draw();
				printbwline("¡» ¹ï¤èÅý¤â");
				if (GameOver)
				{
					/* overgame(); */
					memset(Allow, 0, sizeof Allow);
				}
				else
				{
					if ((*rule[Ballow])() <= 0)
						printbwline("¡» ±z¨«§ëµL¸ô¤F"); /* Thor.990329: ending game? */
				}
				break;

				/* leave BWboard mode, Q.....\0 */
			case 'Q':
				log_chess();
				return LEAVE;
			}

			cc -= ++len;
			if (cc <= 0)
			{
				bufstart = 0;
				break;
			}
			bptr += len;
		}

		return NOTHING;
	}

	static int ftkCtrlC()
	{
		*cmdBuf = '\0';
		cmdCol = 0;
		move(b_lines - 2, 35);
		clrtoeol();
		return NOTHING;
	}

	static int fCtrlD()
	{
		/* send Q.....\0 cmd */
		if (!bw_send("Q"))
			return DISCONNECT;
		log_chess();
		return LEAVE;
	}

	static int ftkCtrlH()
	{
		if (cmdCol)
		{
			int ch = cmdCol--;
			memcpy(&cmdBuf[cmdCol], &cmdBuf[ch], sizeof talkBuf - ch - 1);
			move(b_lines - 2, cmdCol + 35);
			outs(&cmdBuf[cmdCol]);
			clrtoeol();
		}
		return NOTHING;
	}

	static int ftkEnter()
	{
		char msg[80];
#ifdef EVERY_BIFF
		check_biff();
#endif

		if (*cmdBuf)
		{
#if 0
			/* Thor.990218: reserve for "/" command */
			if (ch == '/')
				ch = chat_cmd(cfd, ptr);
#endif
			for (cmdPos = MAXLASTCMD - 1; cmdPos; cmdPos--)
				strcpy(lastcmd[cmdPos], lastcmd[cmdPos - 1]);
			strcpy(lastcmd[0], cmdBuf);

			if (!bw_send(talkBuf))
				return DISCONNECT;

			sprintf(msg, "\033[1;36m¡¸%s\033[m", cmdBuf);
			printbwline(msg);

			*cmdBuf = '\0';
			cmdCol = 0;
			cmdPos = -1;
			move(b_lines - 2, 35);
			clrtoeol();
		}
		return NOTHING;
	}

	static int ftkLEFT()
	{
		if (cmdCol)
			--cmdCol;
		return NOTHING;
	}

	static int ftkRIGHT()
	{
		if (cmdBuf[cmdCol])
			++cmdCol;
		return NOTHING;
	}

	static int ftkUP()
	{
		cmdPos++;
		cmdPos %= MAXLASTCMD;
		str_ncpy(cmdBuf, lastcmd[cmdPos], 41);
		move(b_lines - 2, 35);
		outs(cmdBuf);
		clrtoeol();
		cmdCol = strlen(cmdBuf);
		return NOTHING;
	}

	static int ftkDOWN()
	{
		cmdPos += MAXLASTCMD - 2;
		return ftkUP();
	}

	static int ftkDefault(int ch)
	{
		if (isprint2(ch))
		{
			if (cmdCol < 40)
			{
				if (cmdBuf[cmdCol])
				{
					/* insert */
					int i;
					for (i = cmdCol; cmdBuf[i] && i < 39; i++);
					cmdBuf[i + 1] = '\0';
					for (; i > cmdCol; i--)
						cmdBuf[i] = cmdBuf[i - 1];
				}
				else
				{
					/* append */
					cmdBuf[cmdCol + 1] = '\0';
				}
				cmdBuf[cmdCol] = ch;
				move(b_lines - 2, cmdCol + 35);
				outs(&cmdBuf[cmdCol++]);
			}
			return NOTHING;
		}
		return 0;
	}

	static int ftnCtrlC()
	{
#if 0
		int ch;
		/* Thor.980602: ¥Ñ©ó vans·|¥Î¨ìigetch,
		                ¬°¨¾ I_OTHERDATA³y¦¨·í¦í, ¦b¦¹¥Î ctrlZ_everywhere¤è¦¡,
		                «O¦svio_fd, «Ý°Ý§¹«á¦AÁÙ­ì */
		/* Thor.980602: ¼È¦s vio_fd */
		holdon_fd = vio_fd;
		vio_fd = 0;

		ch = vans("do you want clear brd?[y/N]");

		/* Thor.980602: ÁÙ­ì vio_fd */
		vio_fd = holdon_fd;
		holdon_fd = 0;

		if (ch == 'y')
		{
			if (!bw_send("C"))
				return DISCONNECT;
			bw_init();

		}
#endif
		if (!bw_send("C"))
			return DISCONNECT;
		bw_init();
		return NOTHING;
	}

	static int ftnUP()
	{
		if (bwRow)
			bwRow--;
		return NOTHING;
	}

	static int ftnDOWN()
	{
		if (bwRow < 18)
			if (Board[bwRow+1][bwCol] != Deny)
				bwRow ++;
		return NOTHING;
	}

	static int ftnLEFT()
	{
		if (bwCol)
			bwCol--;
		return NOTHING;
	}

	static int ftnRIGHT()
	{
		if (bwCol < 18)
			if (Board[bwRow][bwCol+1] != Deny)
				bwCol++;
		return NOTHING;
	}

	static int ftnPass()
	{
		/* Thor.990220: for chat mode to enter ^P pass */
		if (mapTurn == myTurn)
		{
			myPass++; myStep++;
			if (!bw_send("P"))
				return DISCONNECT;
			mapTurn = yourTurn;
			bw_draw();
			printbwline("¡» §Ú¤èÅý¤â");
		}
		return NOTHING;
	}

	static int ftnEnter()
	{
		char msg[80];
		char buf[20];

		if (!Allow[bwRow][bwCol]) return NOTHING;

		sprintf(msg, "¡» §Ú¤è¸¨¤l %s", bw_coorstr(bwRow, bwCol));

		myStep++;
		sprintf(buf, "D%d", bwRow * 19 + bwCol);

		if (!bw_send(buf))
			return DISCONNECT;

		/* update board */
		GameOver = (*rule[Bupdate])(myColor, bwRow, bwCol);

		mapTurn = yourTurn;

		bw_draw();

		printbwline(msg);

		if (GameOver)
			overgame();

		return NOTHING;
	}

	static int fTAB()
	{
		mapTalk = mapTalk ? NULL : Talk;
		return NOTHING;
	}

	static int fNoOp()
	{
		return NOTHING;
	}

	static KeyFunc
	Talk[] =
	{
		{Ctrl('C'), ftkCtrlC},
		{Ctrl('D'), fCtrlD},
		{Ctrl('H'), ftkCtrlH},
		{Ctrl('P'), ftnPass},
		{'\n', ftkEnter},
		{KEY_LEFT, ftkLEFT},
		{KEY_RIGHT, ftkRIGHT},
		{KEY_UP, ftkUP},
		{KEY_DOWN, ftkDOWN},
		{KEY_TAB, fTAB},
		{0, ftkDefault}
	},
	yourTurn[] =
	{
		{Ctrl('C'), ftnCtrlC},
		{Ctrl('D'), fCtrlD},
		{KEY_LEFT, ftnLEFT},
		{KEY_RIGHT, ftnRIGHT},
		{KEY_UP, ftnUP},
		{KEY_DOWN, ftnDOWN},
		{KEY_TAB, fTAB},
		{0, fNoOp}
	},
	myTurn[] =
	{
		{Ctrl('C'), ftnCtrlC},
		{' ', ftnEnter},
		{'\n', ftnEnter},
		{Ctrl('P'), ftnPass},
		{Ctrl('D'), fCtrlD},
		{KEY_LEFT, ftnLEFT},
		{KEY_RIGHT, ftnRIGHT},
		{KEY_UP, ftnUP},
		{KEY_DOWN, ftnDOWN},
		{KEY_TAB, fTAB},
		{0, fNoOp}
	};

	/* bwboard main */
	int BWboard(int sock, int later)
	{
		screenline sl[b_lines + 1];
		int ch;
		char c;
		KeyFunc *k;

		vs_save(sl);

		cfd = sock;
		if (!later)
		{
			/* ask for which rule set */
			/* assume: peer won't send char until setup */
			c = vans("·Q¤U­þºØ´Ñ? 0)¨ú®ø 1)¶Â¥Õ´Ñ 2)¤­¤l´Ñ 3)³ò´Ñ [0]");
			if (c) c -= '0';
			else
				vs_restore(sl);	/* lkchu.990428: §â foot restore ¦^¨Ó */

			/* transmit rule set */
			if (send(cfd, &c , 1, 0) != 1)
				return -2;

			myColor = Black;
		}
		else
		{
			/* prompt for waiting rule set */
			outz("¹ï¤è­n¨D¶i¤J¹ï«³¼Ò¦¡,¿ï¾Ü¤¤½Ðµy­Ô.....");
			refresh();
			/* receive rule set */
			if (recv(cfd, &c , 1, 0) != 1)
				return -2;

			vs_restore(sl);		/* lkchu.990428: §â foot restore ¦^¨Ó */
			myColor = White;
		}

		if (!c--) return -1;
		rule = ruleSet[(int)c];
		ruleStr = ruleStrSet[(int)c];

		/* Thor.990311: dynamic attribute */
		key_total = ATTR_OTHELLO_TOTAL + ((unsigned)c << 8);
		key_win = ATTR_OTHELLO_WIN + ((unsigned)c << 8);

		/* initialize all */
		bw_init();

		for (;;)
		{
			if (mapTalk)
			{
				move(b_lines - 2, cmdCol + 35);
				k = mapTalk;
			}
			else
			{
#if 0
				move(bwRow + 1, bwCol * 2);
#endif
				/* Thor.990222: for crxvt */
				move(bwRow + 1, bwCol * 2 + 1);
				k = mapTurn;
			}

			ch = vkey();
			if (ch == I_OTHERDATA)
			{
				/* incoming */
				if ((ch = bw_recv()) >= NOTHING) /* -1 for exit bwboard, -2 for exit talk */
					continue;
				vs_restore(sl);
				return ch;
			}
#ifdef EVERY_Z
			/* Thor: Chat ¤¤«ö ctrl-z */
			else if (ch == Ctrl('Z'))
			{
				char buf[IDLEN + 1];

				/* Thor.0731: ¼È¦s mateid, ¦]¬°¥X¥h®É¥i¯à·|¥Î±¼ mateid(like query) */
				strcpy(buf, cutmp->mateid);

				/* Thor.0727: ¼È¦s vio_fd */
				holdon_fd = vio_fd;
				vio_fd = 0;
				every_Z();
				/* Thor.0727: ÁÙ­ì vio_fd */
				vio_fd = holdon_fd;
				holdon_fd = 0;

				/* Thor.0731: ÁÙ­ì mateid, ¦]¬°¥X¥h®É¥i¯à·|¥Î±¼ mateid(like query) */
				strcpy(cutmp->mateid, buf);
				continue;
			}
#endif
			else if (ch == Ctrl('U'))
			{
				char buf[IDLEN + 1];

				/* Thor.0731: ¼È¦s mateid, ¦]¬°¥X¥h®É¥i¯à·|¥Î±¼ mateid(like query) */
				strcpy(buf, cutmp->mateid);

				/* Thor.0727: ¼È¦s vio_fd */
				holdon_fd = vio_fd;
				vio_fd = 0;
				every_U();
				/* Thor.0727: ÁÙ­ì vio_fd */
				vio_fd = holdon_fd;
				holdon_fd = 0;

				/* Thor.0731: ÁÙ­ì mateid, ¦]¬°¥X¥h®É¥i¯à·|¥Î±¼ mateid(like query) */
				strcpy(cutmp->mateid, buf);
				continue;
			}

			for (;;k++)
			{
				if (!k->key || ch == k->key)
					break;
			}

			/* -1 for exit bwboard, -2 for exit talk */
			if ((ch = k->key ? (*k->func)() : (*k->func)(ch)) >= NOTHING)
				continue;
			vs_restore(sl);
			return ch;
		}
	}

#include<stdarg.h>
	int vaBWboard(va_list pvar)
	{
		int sock, later;
		sock = va_arg(pvar, int);
		later = va_arg(pvar, int);
		return BWboard(sock, later);
	}
