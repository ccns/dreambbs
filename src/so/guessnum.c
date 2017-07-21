/*-------------------------------------------------------*/
/* guessnum.c   ( NTHU CS MapleBBS Ver 3.10 )            */
/*-------------------------------------------------------*/
/* author : thor.bbs@bbs.cs.nthu.edu.tw			 */
/* target : Guess Number tool dynamic link module        */
/* create : 99/02/16                                     */
/* update :   /  /                                       */
/*-------------------------------------------------------*/

#include "bbs.h"


typedef char Num[4];

typedef struct
{
	Num n;
	int A, B;
} His;

static int hisNum;
static His *hisList;

static int numNum;
static char *numSet;

#if 1
void _init()
{
	hisList = (His *)malloc(sizeof(His)); /* pseudo */
	numSet = (char *)malloc(10 * 9 * 8 * 7 * sizeof(char));
}

void _fini()
{
	free(hisList);
	free(numSet);
}
#endif

static void AB(Num p, Num q, int *A, int *B)
{
	/* compare p and q, return ?A?B */
	int i, j;
	*A = *B = 0;
	for (i = 0;i < 4;i++)
	{
		for (j = 0;j < 4;j++)
		{
			if (p[i] == q[j])
			{
				if (i == j)
					++*A;
				else
					++*B;
			}
		}
	}
}

static int getth(int o, char a[])
{
	/* return "o"th element index in a[], base 0 */
	int i = -1;
	o++;
	while (o)
		if (a[++i])
			continue;
		else
			o--;
	return i;
}

static void ord2Num(int o, Num p)
{
	/* return "o"th filtered number */
	char digit[10];
	int i, j, k;

	memset(digit, 0, sizeof digit);

	for (j = 0, k = 10; j < 4; j++, k--)
	{
		i = o % k; o /= k;
		i = getth(i, digit);
		p[j] = i; digit[i] = 1;
	}
}

static int matchHis(Num n)
{
	int i, A, B;
	for (i = 0;i < hisNum;i++)
	{
		AB(n, hisList[i].n, &A, &B);
		if (A != hisList[i].A || B != hisList[i].B)
			return 0;
	}
	return 1;
}

int mainNum(int fighting /* Thor.990317: ��ԼҦ� */)
{
	Num myNumber;
	/* initialize variables */
#if 0
	if (!hisList) hisList = (His *)malloc(sizeof(His)); /* pseudo */
	if (!numSet) numSet = (char *)malloc(10 * 9 * 8 * 7 * sizeof(char));
#endif

	hisNum = 0;

	numNum = 10 * 9 * 8 * 7;
	memset(numSet, 0, 10*9*8*7*sizeof(char));

	srand(time(NULL));

	/* Thor.990317:��ԼҦ� */
	vs_bar(fighting ? "�q�Ʀr�j��" : "�̥ʲq�Ʀr"); /* clear(); */

	/* Thor.990221: ���H���������} */
	if (vans("�Q�n�z���Ʀr�F��?[y/N]") != 'y')
	{
		vmsg("�����F��? �U���A�Ӯ@! ^_^");
		return 0;
	}

	if (fighting)
		ord2Num(rand() % numNum, myNumber); /* Thor.990317:��ԼҦ� */

	/* while there is possibility */
	for (;;)
	{
		Num myGuess, yourGuess;
		int youA, youB, myA, myB;

		if (fighting) /* Thor.990317:��ԼҦ� */
		{
			int i;
			char tmp[50];
			vget(b_lines - 3, 0, "�z�q�ڪ��Ʀr�O[????]:", tmp, 5, DOECHO);
			/* Thor.990317: ��²��, ���@checking */
			if (!tmp[0]) goto abort_game;

			for (i = 0;i < 4;i++)
				yourGuess[i] = tmp[i] - '0';
			AB(myNumber, yourGuess, &myA, &myB);
			move(b_lines - 2, 0);
			prints("�ڻ� \033[1m%dA%dB \033[m", myA, myB);

			if (myA == 4)
			{
				/* you win  */
				vmsg("�zĹ�F! �n�R�� ^O^");
#if 1
				sprintf(tmp, "HisNum:%d, user win game!", hisNum);
				blog(fighting ? "FIGHTNUM" : "GUESSNUM", tmp);
#endif
				return 0;
			}
		}

		/* pickup a candidate number */
		for (;;)
		{
			int i;
			/* pickup by random */
			if (numNum <= 0)
				goto foolme;
			i = rand() % numNum;
			i = getth(i, numSet); /* i-th ordering num */
			numSet[i] = 1; numNum--; /* filtered out */
			ord2Num(i, myGuess); /* convert ordering num to Num */

			/* check history */
			if (matchHis(myGuess))
				break;
		}

		/* show the picked number */
		move(b_lines - 1, 0);
		prints("�ڲq�z���Ʀr�O \033[1;37m%d%d%d%d\033[m", myGuess[0], myGuess[1], myGuess[2], myGuess[3]);

		/* get ?A?B */
		for (;;)
		{
			char buf[5];
			/* get response */
			vget(b_lines, 0, "�z���^��[?A?B]:", buf, 5, DOECHO);

			if (!buf[0])
			{
				char msg[40];
abort_game:
				/* abort */
				vmsg("�����F��? �U���A�Ӯ@! ^_^");
#if 1
				sprintf(msg, "HisNum:%d, abort game!", hisNum);
				blog(fighting ? "FIGHTNUM" : "GUESSNUM", msg);
#endif
				return 0;
			}
			if (isdigit(buf[0]) && (buf[1] | 0x20) == 'a'
				&& isdigit(buf[2]) && (buf[3] | 0x20) == 'b')
			{
				youA = buf[0] - '0';
				youB = buf[2] - '0';
				/* check legimate */
				if (youA >= 0 && youA <= 4
					&& youB >= 0 && youB <= 4
					&& youA + youB <= 4)
				{
					/* if 4A, end the game */
					if (youA == 4)
					{
						char msg[40];
						/* I win  */
						vmsg("��Ĺ�F! �F�`�a ^O^");
#if 1
						sprintf(msg, "HisNum:%d, win game!", hisNum);
						blog(fighting ? "FIGHTNUM" : "GUESSNUM", msg);
#endif
						return 0;
					}
					else
						break;
				}
			}
			/* err A B */
			zmsg("��J�榡���~");
		}
		/* put in history */
		hisNum ++;
		hisList = (His *) realloc(hisList, hisNum * sizeof(His)); /* assume must succeeded */
		memcpy(hisList[hisNum - 1].n, myGuess, sizeof(Num));
		hisList[hisNum - 1].A = youA;
		hisList[hisNum - 1].B = youB;

		move(2 + hisNum, 0);
		if (fighting) /* Thor.990317: ��ԼҦ� */
			prints("�� \033[1;37m%d\033[m ��, �A�q \033[1;36m%d%d%d%d\033[m, �ڻ� \033[1;33m%dA%dB\033[m; �ڲq \033[1;33m%d%d%d%d\033[m, �A�� \033[1;36m%dA%dB\033[m", hisNum, yourGuess[0], yourGuess[1], yourGuess[2], yourGuess[3], myA, myB, myGuess[0], myGuess[1], myGuess[2], myGuess[3], youA, youB);
		else
			prints("�� \033[1;37m%d\033[m ��, �ڲq \033[1;33m%d%d%d%d\033[m, �A�� \033[1;36m%dA%dB\033[m", hisNum, myGuess[0], myGuess[1], myGuess[2], myGuess[3], youA, youB);
	}
foolme:
	/* there is no posibility, show "you fool me" */
	vmsg("�A�F��! ����A���F ~~~>_<~~~");
#if 1
	{
		char msg[40];
		sprintf(msg, "HisNum:%d, fool me!", hisNum);
		blog(fighting ? "FIGHTNUM" : "GUESSNUM", msg);
	}
#endif
	return 0;
}

int guessNum()
{
	mainNum(0);
	return 0;
}

int fightNum()
{
	mainNum(1);
	return 0;
}
