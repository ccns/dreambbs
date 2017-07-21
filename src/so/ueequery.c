/*-------------------------------------------------------*/
/* ueequery.c   ( NTU FPG BBS Ver 2.00 )       by �d�Эl */
/*-------------------------------------------------------*/
/* target : �p�Һ]��d��                                 */
/* create : 98/08/08                                     */
/* update : 98/08/08                                     */
/*-------------------------------------------------------*/
/*
��ƨӷ�
http://www.ncu.edu.tw/~center25/86union/fa01.txt ~ fa04.txt
http://www.ncu.edu.tw/~center25/87union/fa01.txt ~ fa04.txt
http://www.ncu.edu.tw/~center25/88union/fa01.txt ~ fa04.txt

ftp://140.112.2.84/pub/89exam/
http://www.csie.nctu.edu.tw/service/jcee/ ���~
http://www.cts.com.tw/exam/university/
http://www.ncu.edu.tw/~center25/89union/
http://140.112.3.171/uee88/default.htm
http://info.ntu.edu.tw/88/
�K��C��W�ȤE�ɶ}��I�I
*/
#include "bbs.h"

int
x_ueequery()
{
	char buf[256], query[20];
	char fpath[80], now[80], *ptr;
	int  i, count = 0, year, examnum = 0;
	FILE *fp;

	vs_bar("�p�Һ]��d��");
	outs("�@��: Shen Chuan-Hsing <statue.bbs@bbs.yzu.edu.tw>\n");

	outs("�`�N�ƶ�\n"
		 "1. �������Ҹ��X�γ����m�W�ⶵ�ܤֻݶ�g�@���~����d��.\n"
		 "2. �Y�m�W�u����r�A�Щ��r�����[���Ϊť�.\n"
		 "3. 83-86 �~�ݭn�������Ҹ��X, 87-90 �~�ݭn�C�����Ҹ��X.\n"
		 "4. 91-94 �~�ݭn�K�����Ҹ��X.\n");
	if (!vget(8, 0, "�п�J�p�Ҧ~�� (83 ~ 94)[94]�G", buf, 3, DOECHO))
		year = 94;
	else
		year = atoi(buf);
	if (year < 83 || year > 94) return 0;
	sprintf(fpath, "game/%d/fa00.txt", year);

	/*  if(year != 83 && year != 84)
	  {
	    vget(9, 0, "�п�J�p�ҲէO�G1)�Ĥ@���� 2)�ĤG���� 3)�ĤT���� 4)�ĥ|���� (1 - 4)�H ", buf, 3, DOECHO);
	    i = atoi(buf);
	  }
	  else
	    i = 1;
	  if (i < 1 || i > 4) return 0;
	*/


	if (year >= 83 && year <= 86)
		vget(10, 0, "�п�J�����ҵ����X�G", buf, 7 , DOECHO);
	else if (year >= 91)
		vget(10, 0, "�п�J�����ҵ����X�G", buf, 9 , DOECHO);
	else
		vget(10, 0, "�п�J�����ҵ����X�G", buf, 8 , DOECHO);


	examnum = strlen(buf);
	if ((year >= 83 && year <= 86) && (examnum != 6 && examnum != 0))
	{
		vmsg("�����T�������ҵ����X, 83-86 �~�ݭn�������Ҹ��X.");
		return 0;
	}
	else if ((year >= 91) && (examnum != 8 && examnum != 0))
	{
		vmsg("�����T�������ҵ����X, 91-94 �~�ݭn�K�����Ҹ��X.");
		return 0;
	}
	else if ((year >= 87 && year <= 90) && (examnum != 7 && examnum != 0))
	{
		vmsg("�����T�������ҵ����X, 87-90 �~�ݭn�C�����Ҹ��X.");
		return 0;
	}

	if ((year >= 87 && year <= 90) && examnum == 7)
		strcpy(query, buf);
	else if ((year >= 83 && year <= 86) && examnum == 6)
		strcpy(query, buf);
	else if ((year >= 91) && examnum == 8)
		strcpy(query, buf);
	else
	{
		vget(12, 0, "�п�J�����m�W�G", buf, 10, DOECHO);
		strcpy(query, buf);
	}

	if (!query[0])
	{
		vmsg("�����T���d�߸��");
		return 0;
	}

	if (year >= 91)
		examnum = 8;
	else if (year >= 87 && year <= 90)
		examnum = 7;
	else if (year >= 83 && year <= 86)
		examnum = 6;
	else
		return 0;

	i = 0;
	logitfile(FN_UEEQUERY_LOG, fpath, query);

	if ((fp = fopen(fpath, "r")))
	{
		int  j;
		char *pquery;

		while (fgets(buf, 256, fp))
		{

			/*
			 0001��߻O�W�j��              �����Ǩt                                  1��
			*/
			if (buf[0] == ' ') /* �o�˴N�O��t���}�Y */
			{
				ptr = strtok(buf + 5, " ");
				strcpy(now, ptr);
				ptr = strtok(NULL, " ");
				sprintf(now, "%-24s%-30s", now, ptr);
				continue;
			}

			if (i == 0)
			{
				vs_bar("�p�Һ]��d��");
				move(2, 0);
				outs("[7m �����    �m  �W    �� �� �j ��             �� �� �� �t                        [m");
			}
			move(1, 0);
			clrtoeol();
			prints("�d�ߡm%s�n��....", query);
			ptr = strtok(buf, "    ");

			if ((pquery = strstr(ptr, query)))
				j = (pquery - ptr) + examnum + 1;
			else
				j = 2; /* ��ܥ���� */
			/*
			      if (strstr(ptr, query))
			*/
			/* j=0 => ����Ҹ��X match
			   examnum = 6 => j=0,6,8,10+examnum+1=7,13,15,17 ���� match
			                  j= 2 % 2 = 0 no match
			                 => (6+1)+6,8,10 %2 = 1 match
			   examnum = 7 => j=0,7,9,11+examnum+1=8,15,17,19 ���� match
			                  j= 2 % 2 = 0 no match
			                 => (7+1)+7,9,11 %2 = 1 match
			*/
			if (j == (pquery - ptr) + examnum + 1 || j % 2)    /* �B�z����r�����D */
			{
				count++;
				if (count > 20 && (count % 20) == 1)
				{
					int ch;

					move(1, 0);
					clrtoeol();
					outs("[1;33m�����d�ߤw���A�����d�߽Ы����N��A�����d�߽Ы� q[m");
					ch = vkey();
					if (ch == 'q' || ch == 'Q')
					{
						fclose(fp);
						return 0;
					}
					i = 0;
					vs_bar("�p�Һ]��d��");
					move(2, 0);
					outs("[7m �����    �m  �W    �� �� �j ��         �� �� �� �t                            [m");
				}
				move(3 + i, 0);
				if (examnum == 6)
					prints(" %-6.6s    %-8s  %s", ptr, ptr + examnum, now);
				else if (examnum == 7)
					prints(" %-7.7s   %-8s  %s", ptr, ptr + examnum, now);
				else if (examnum == 8)
					prints(" %-8.8s  %-8s  %s", ptr, ptr + examnum, now);
				i++;
				move(b_lines, 0);
				clrtoeol();
				prints("[1;33;42m �]��d�� [;30;47m �ثe�@�d�ߨ� [31m%5d [30m���                                             [m",
					   count);
				refresh();
			}
			while ((ptr = strtok(NULL, "    ")))
			{
				if ((pquery = strstr(ptr, query)))
					j = (pquery - ptr) + examnum + 1;
				else
					j = 2; /* ��ܥ���� */

				if (j == (pquery - ptr) + examnum + 1 || j % 2)    /* �B�z����r�����D */
				{
					count++;
					if (count > 20 && (count % 20) == 1)
					{
						int ch;

						move(1, 0);
						clrtoeol();
						outs("[1;33m�����d�ߤw���A�����d�߽Ы����N��A�����d�߽Ы� q[m");
						ch = vkey();
						if (ch == 'q' || ch == 'Q')
						{
							fclose(fp);
							return 0;
						}
						i = 0;
						vs_bar("�p�Һ]��d��");
						move(2, 0);
						outs("[7m �����    �m �W     �� �� �j ��         �� �� �� �t                            [m");
					}
					move(3 + i, 0);
					if (examnum == 6)
						prints(" %-6.6s    %-8s  %s", ptr, ptr + examnum, now);
					else if (examnum == 7)
						prints(" %-7.7s   %-8s  %s", ptr, ptr + examnum, now);
					else if (examnum == 8)
						prints(" %-8.8s  %-8s  %s", ptr, ptr + examnum, now);
					i++;
					move(b_lines, 0);
					clrtoeol();
					prints("[1;33;42m �]��d�� [;30;47m �ثe�@�d�ߨ� [31m%5d [30m���                                             [m",
						   count);
					refresh();
				} // end while
			}
		}
		fclose(fp);
	}
	move(1, 0);
	clrtoeol();
	outs("[1;36m�d�ߵ����A�����N�����}....[m");
	move(b_lines, 0);
	clrtoeol();
	prints("[1;33;42m �]��d�� [;30;47m �ثe�@�d�ߨ� [31m%5d [30m���                                             [m",
		   count);
	if (count == 0)
	{
		move(4, 0);
		outs("�S���d�ߨ������");
	}
	vkey();
	return 0;
}
