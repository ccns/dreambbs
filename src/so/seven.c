/*-------------------------------------------------------*/
/* so/seven.c        ( YZU WindTopBBS Ver 3.00 )         */
/*-------------------------------------------------------*/
/* author : verit.bbs@bbs.yzu.edu.tw                     */
/* target : ���s�C�� 1 		                         */
/* create : 2001/01/12                                   */
/*-------------------------------------------------------*/

#include "bbs.h"
#define DO_LOG

int cards[52];
int seven[7];
int points[7];

/***************************************
	�e�X�P
***************************************/
void
draw_card(card, x, y)
int card;
int x;
int y;
{
	char *flower[] = {"��", "��", "��", "��"};
	char *number[] = {"��", "��", "��", "��", "��", "��", "��", "��", "��", "10", "��", "��", "��"};
	move(x  , y);prints("�~�w�w�w��");
	move(x + 1, y);prints("�x%s�@�@�x", (card < 0) ? "�@" : number[card%13]);
	move(x + 2, y);prints("�x%s�@�@�x", (card < 0) ? "�@" : flower[card%4]);
	move(x + 3, y);prints("�x�@�@�@�x");
	move(x + 4, y);prints("�x�@�@�@�x");
	move(x + 5, y);prints("���w�w�w��");
	return ;
}

/***************************************
	�C������
***************************************/
int
draw_explain(void)
{
	int x = 4;
	clear();
	vs_head("���s�C��", str_site);
	move(x + 0, 10);prints("�i�C�������j");
	move(x + 2, 15);prints("(1) �b�ù��W�謰�P�[�A�i�H�Q�� j , k �����C");
	move(x + 4, 15);prints("(2) �b�ù����U�謰���P , �i�H�Q�� c �����C");
	move(x + 6, 15);prints("(3) ��P�[���P�O���P���U�@�i�ΤW�@�i�A�Y�i�Y�P�C");
	move(x + 7, 15);prints("   (�u���I�ơA���ݪ��)");
	move(x + 9, 15);prints("(4) ��P�[���P���Y���A�ιC����ӡC");
	move(x + 11, 15);prints("(5) ����P�������B�|���Y���P�[���P�A�Y�C�����ѡC");
	move(x + 13, 15);prints("(6) ���C�����v�ݩ� verit.bbs@bbs.yzu.edu.tw");
	vmsg(NULL);
}

/***************************************
	�e�X�C���P���t�m
***************************************/
void
draw_screen(void)
{
	int i, j;
	clear();
	vs_head("���s�C��", str_site);
	for (i = 0;i < 7;i++)
		for (j = 0;j < i + 1;j++)
		{
			draw_card((i == j) ? cards[i] : -1, 4 + j, 5 + i*10);
		}
}

/***************************************
	�e�X���
***************************************/
void
draw_cursor(location, mode)
int location;
int mode;
{
	move(8 + seven[location], 9 + location*10);
	prints("%s", (mode == 0) ? "�@" : "��");
}

/***************************************
	�M���ù��W���P
***************************************/
void clear_card(location)
int location;
{
	move(9 + seven[location], 5 + location*10);
	prints("�@�@�@�@�@");
}

/***************************************
	�C���Ѽƪ�l��
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
	�P�_�C���O�_����
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
	�C���D�{��
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
	prints("�A�٦� %2d ���|�i�H���P", have_card);
	move(18, 10);
	outz("\033[1;37;42m�i�ާ@�����j [j]���� [k]�k�� [enter]�Y�P [c]���P [q]���} \033[1;30mBy Verit.bbs@yzu.bbs \033[m");
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
			prints("�A�٦� %2d ���|�i�H���P", have_card);
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
						prints("�@�@�@�@�@");
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
			vmsg("���ߧA�L���� !!");
#ifdef DO_LOG
			sprintf(log, "%d\033[m", tmp);
			logitfile(FN_SEVEN_LOG , "\033[1;33m<WIN>", log);
#endif
		}
		else if (tmp == -1)
		{
			vmsg("�D�ԥ��� !!");
#ifdef DO_LOG
			logitfile(FN_SEVEN_LOG , "\033[1;31m<LOSE>\033[m", " ");
#endif
		}
		else if (tmp == -2)
		{
			vmsg("�U���A�Ӫ��� !!");
#ifdef DO_LOG
			logitfile(FN_SEVEN_LOG , "\033[1;34m<QUIT>\033[m", " ");
#endif
			return 0;
		}
		if (vans("�O�_�n�~��H [y/N]") != 'y')
			break;
	}
	while (1);
#ifdef DO_LOG
	logitfile(FN_SEVEN_LOG , "<EXIT>\033[m", " ");
#endif
	return 0;
}