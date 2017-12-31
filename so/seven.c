/*-------------------------------------------------------*/
/* so/seven.c        ( YZU WindTopBBS Ver 3.00 )         */
/*-------------------------------------------------------*/
/* author : verit.bbs@bbs.yzu.edu.tw                     */
/* target : 接龍遊戲 1 		                         */
/* create : 2001/01/12                                   */
/*-------------------------------------------------------*/

#include "bbs.h"
#define DO_LOG

int cards[52];
int seven[7];
int points[7];

/***************************************
	畫出牌
***************************************/
void
draw_card(card, x, y)
int card;
int x;
int y;
{
	char *flower[] = {"Ｃ", "Ｄ", "Ｈ", "Ｓ"};
	char *number[] = {"Ａ", "２", "３", "４", "５", "６", "７", "８", "９", "10", "Ｊ", "Ｑ", "Ｋ"};
	move(x  , y);prints("╭───╮");
	move(x + 1, y);prints("│%s　　│", (card < 0) ? "　" : number[card%13]);
	move(x + 2, y);prints("│%s　　│", (card < 0) ? "　" : flower[card%4]);
	move(x + 3, y);prints("│　　　│");
	move(x + 4, y);prints("│　　　│");
	move(x + 5, y);prints("╰───╯");
	return ;
}

/***************************************
	遊戲說明
***************************************/
int
draw_explain(void)
{
	int x = 4;
	clear();
	vs_head("接龍遊戲", str_site);
	move(x + 0, 10);prints("【遊戲說明】");
	move(x + 2, 15);prints("(1) 在螢幕上方為牌墩，可以利用 j , k 切換。");
	move(x + 4, 15);prints("(2) 在螢幕左下方為持牌 , 可以利用 c 切換。");
	move(x + 6, 15);prints("(3) 當牌墩的牌是持牌的下一張或上一張，即可吃牌。");
	move(x + 7, 15);prints("   (只看點數，不看花色)");
	move(x + 9, 15);prints("(4) 當牌墩的牌都吃完，及遊戲獲勝。");
	move(x + 11, 15);prints("(5) 當持牌切換完且尚未吃完牌墩的牌，即遊戲失敗。");
	move(x + 13, 15);prints("(6) 此遊戲版權屬於 verit.bbs@bbs.yzu.edu.tw");
	vmsg(NULL);
}

/***************************************
	畫出遊戲牌的配置
***************************************/
void
draw_screen(void)
{
	int i, j;
	clear();
	vs_head("接龍遊戲", str_site);
	for (i = 0;i < 7;i++)
		for (j = 0;j < i + 1;j++)
		{
			draw_card((i == j) ? cards[i] : -1, 4 + j, 5 + i*10);
		}
}

/***************************************
	畫出游標
***************************************/
void
draw_cursor(location, mode)
int location;
int mode;
{
	move(8 + seven[location], 9 + location*10);
	prints("%s", (mode == 0) ? "　" : "●");
}

/***************************************
	清除螢幕上的牌
***************************************/
void clear_card(location)
int location;
{
	move(9 + seven[location], 5 + location*10);
	prints("　　　　　");
}

/***************************************
	遊戲參數初始化
***************************************/
int
init(void)
{
	int i, num, tmp;
	for (i = 0;i < 52;i++)
		cards[i] = i;

	srand(time(0));
	for (i = 0;i < 52;i++)
	{
		num = cards[i] ;
		tmp = rand() % 52 ;
		cards[i] = cards[tmp] ;
		cards[tmp] = num;
	}

	for (i = 0;i < 7;i++)
	{
		seven[i] = i;
		points[i] = cards[i] ;
	}

	return 0;
}

/***************************************
	判斷遊戲是否結束
***************************************/
int
gameover(void)
{
	int i;
	for (i = 0;i < 7;i++)
		if (seven[i] != -1)
			return 0;
	return 1;
}

/***************************************
	遊戲主程式
***************************************/
int
play(void)
{
	int location = 0;
	int c, i;
	int now_card = 7;
	int have_card = 22;
	int point;

	draw_screen();
	draw_cursor(location, 1);
	point = cards[now_card] ;
	draw_card(cards[now_card++], 14, 5);
	move(19, 40);
	prints("你還有 %2d 機會可以換牌", have_card);
	move(18, 10);
	outz("\033[1;37;42m【操作說明】 [j]左移 [k]右移 [enter]吃牌 [c]換牌 [q]離開 \033[1;30mBy Verit.bbs@yzu.bbs \033[m");
	do
	{
		c = vkey();
		switch (c)
		{
		case 'c' :
			if (have_card == 0)
				return -1;
			have_card--;
			move(19, 40);
			prints("你還有 %2d 機會可以換牌", have_card);
			point = cards[now_card];
			draw_card(cards[now_card++], 14, 5);
			break;
		case 'k' :
		case KEY_RIGHT :
			draw_cursor(location, 0);
			do
			{
				location = (location + 1) % 7;
			}
			while (seven[location] == -1);
			draw_cursor(location, 1);
			break;
		case 'j' :
		case KEY_LEFT :
			draw_cursor(location, 0);
			do
			{
				location = (location == 0) ? 6 : location - 1;
			}
			while (seven[location] == -1);
			draw_cursor(location, 1);
			break;
		case '\n' :
		case ' ' :
			if (points[location] % 13 - point % 13 == 1 ||
				points[location] % 13 - point % 13 == -1 ||
				points[location] % 13 - point % 13 == 12 ||
				points[location] % 13 - point % 13 == -12)
			{
				point = points[location];
				draw_card(point, 14, 5);
				clear_card(location);
				draw_cursor(location, 0);
				seven[location]--;
				if (seven[location] >= 0)
				{
					points[location] = cards[now_card];
					draw_card(cards[now_card++], 4 + seven[location], 5 + location*10);
					draw_cursor(location, 1);
				}
				else
				{
					for (i = 0;i < 5;i++)
					{
						move(4 + i, 5 + location*10);
						prints("　　　　　");
					}
					if (gameover() == 1) return have_card;
					do
					{
						location = (location + 1) % 7;
					}
					while (seven[location] == -1);
					draw_cursor(location, 1);
				}
			}
			break;
		case 'q' :
			return -2 ;
		}
	}
	while (1);
	return 0;
}

int
seven_main(void)
{
	int tmp;
	char log[80];
#ifdef DO_LOG
	logitfile(FN_SEVEN_LOG , "<ENTER>\033[m", " ");
#endif
	draw_explain();
	do
	{
		init();
		clear();
		tmp = play();
		if (tmp >= 0)
		{
			vmsg("恭喜你過關啦 !!");
#ifdef DO_LOG
			sprintf(log, "%d\033[m", tmp);
			logitfile(FN_SEVEN_LOG , "\033[1;33m<WIN>", log);
#endif
		}
		else if (tmp == -1)
		{
			vmsg("挑戰失敗 !!");
#ifdef DO_LOG
			logitfile(FN_SEVEN_LOG , "\033[1;31m<LOSE>\033[m", " ");
#endif
		}
		else if (tmp == -2)
		{
			vmsg("下次再來玩唷 !!");
#ifdef DO_LOG
			logitfile(FN_SEVEN_LOG , "\033[1;34m<QUIT>\033[m", " ");
#endif
			return 0;
		}
		if (vans("是否要繼續？ [y/N]") != 'y')
			break;
	}
	while (1);
#ifdef DO_LOG
	logitfile(FN_SEVEN_LOG , "<EXIT>\033[m", " ");
#endif
	return 0;
}