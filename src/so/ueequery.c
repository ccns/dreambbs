/*-------------------------------------------------------*/
/* ueequery.c   ( NTU FPG BBS Ver 2.00 )       by §d²Ð­l */
/*-------------------------------------------------------*/
/* target : Áp¦Òº]³æ¬d¸ß                                 */
/* create : 98/08/08                                     */
/* update : 98/08/08                                     */
/*-------------------------------------------------------*/
/*
¸ê®Æ¨Ó·½
http://www.ncu.edu.tw/~center25/86union/fa01.txt ~ fa04.txt
http://www.ncu.edu.tw/~center25/87union/fa01.txt ~ fa04.txt
http://www.ncu.edu.tw/~center25/88union/fa01.txt ~ fa04.txt

ftp://140.112.2.84/pub/89exam/
http://www.csie.nctu.edu.tw/service/jcee/ ¾ú¦~
http://www.cts.com.tw/exam/university/
http://www.ncu.edu.tw/~center25/89union/
http://140.112.3.171/uee88/default.htm
http://info.ntu.edu.tw/88/
¤K¤ë¤C¤é¤W¤È¤E®É¶}©ñ¡I¡I
*/
#include "bbs.h"

int
x_ueequery()
{
	char buf[256], query[20];
	char fpath[80], now[80], *ptr;
	int  i, count = 0, year, examnum = 0;
	FILE *fp;

	vs_bar("Áp¦Òº]³æ¬d¸ß");
	outs("§@ªÌ: Shen Chuan-Hsing <statue.bbs@bbs.yzu.edu.tw>\n");

	outs("ª`·N¨Æ¶µ\n"
		 "1. §¹¾ã­ã¦ÒÃÒ¸¹½X¤Î³¡¥÷©m¦W¨â¶µ¦Ü¤Ö»Ý¶ñ¼g¤@¶µ¤~¯à°÷¬d¸ß.\n"
		 "2. ­Y©m¦W¥u¦³¨â¦r¡A½Ð©ó¨â¦r¤¤¶¡¥[¥þ§ÎªÅ¥Õ.\n"
		 "3. 83-86 ¦~»Ý­n¤»¦ì­ã¦ÒÃÒ¸¹½X, 87-90 ¦~»Ý­n¤C¦ì­ã¦ÒÃÒ¸¹½X.\n"
		 "4. 91-94 ¦~»Ý­n¤K¦ì­ã¦ÒÃÒ¸¹½X.\n");
	if (!vget(8, 0, "½Ð¿é¤JÁp¦Ò¦~«× (83 ~ 94)[94]¡G", buf, 3, DOECHO))
		year = 94;
	else
		year = atoi(buf);
	if (year < 83 || year > 94) return 0;
	sprintf(fpath, "game/%d/fa00.txt", year);

	/*  if(year != 83 && year != 84)
	  {
	    vget(9, 0, "½Ð¿é¤JÁp¦Ò²Õ§O¡G1)²Ä¤@Ãþ²Õ 2)²Ä¤GÃþ²Õ 3)²Ä¤TÃþ²Õ 4)²Ä¥|Ãþ²Õ (1 - 4)¡H ", buf, 3, DOECHO);
	    i = atoi(buf);
	  }
	  else
	    i = 1;
	  if (i < 1 || i > 4) return 0;
	*/


	if (year >= 83 && year <= 86)
		vget(10, 0, "½Ð¿é¤J§¹¾ã­ã¦Òµý¸¹½X¡G", buf, 7 , DOECHO);
	else if (year >= 91)
		vget(10, 0, "½Ð¿é¤J§¹¾ã­ã¦Òµý¸¹½X¡G", buf, 9 , DOECHO);
	else
		vget(10, 0, "½Ð¿é¤J§¹¾ã­ã¦Òµý¸¹½X¡G", buf, 8 , DOECHO);


	examnum = strlen(buf);
	if ((year >= 83 && year <= 86) && (examnum != 6 && examnum != 0))
	{
		vmsg("¤£¥¿½Tªº§¹¾ã­ã¦Òµý¸¹½X, 83-86 ¦~»Ý­n¤»¦ì­ã¦ÒÃÒ¸¹½X.");
		return 0;
	}
	else if ((year >= 91) && (examnum != 8 && examnum != 0))
	{
		vmsg("¤£¥¿½Tªº§¹¾ã­ã¦Òµý¸¹½X, 91-94 ¦~»Ý­n¤K¦ì­ã¦ÒÃÒ¸¹½X.");
		return 0;
	}
	else if ((year >= 87 && year <= 90) && (examnum != 7 && examnum != 0))
	{
		vmsg("¤£¥¿½Tªº§¹¾ã­ã¦Òµý¸¹½X, 87-90 ¦~»Ý­n¤C¦ì­ã¦ÒÃÒ¸¹½X.");
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
		vget(12, 0, "½Ð¿é¤J³¡¥÷©m¦W¡G", buf, 10, DOECHO);
		strcpy(query, buf);
	}

	if (!query[0])
	{
		vmsg("¤£¥¿½Tªº¬d¸ß¸ê®Æ");
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
			 0001°ê¥ß»OÆW¤j¾Ç              ¤¤°ê¤å¾Ç¨t                                  1²Ä
			*/
			if (buf[0] == ' ') /* ³o¼Ë´N¬O¬ì¨tªº¶}ÀY */
			{
				ptr = strtok(buf + 5, " ");
				strcpy(now, ptr);
				ptr = strtok(NULL, " ");
				sprintf(now, "%-24s%-30s", now, ptr);
				continue;
			}

			if (i == 0)
			{
				vs_bar("Áp¦Òº]³æ¬d¸ß");
				move(2, 0);
				outs("[7m ­ã¦ÒÃÒ    ©m  ¦W    ¿ý ¨ú ¤j ¾Ç             ¿ý ¨ú ¬ì ¨t                        [m");
			}
			move(1, 0);
			clrtoeol();
			prints("¬d¸ß¡m%s¡n¤¤....", query);
			ptr = strtok(buf, "    ");

			if ((pquery = strstr(ptr, query)))
				j = (pquery - ptr) + examnum + 1;
			else
				j = 2; /* ªí¥Ü¥¼§ä¨ì */
			/*
			      if (strstr(ptr, query))
			*/
			/* j=0 => ­ã¦ÒÃÒ¸¹½X match
			   examnum = 6 => j=0,6,8,10+examnum+1=7,13,15,17 ¤¤¤å match
			                  j= 2 % 2 = 0 no match
			                 => (6+1)+6,8,10 %2 = 1 match
			   examnum = 7 => j=0,7,9,11+examnum+1=8,15,17,19 ¤¤¤å match
			                  j= 2 % 2 = 0 no match
			                 => (7+1)+7,9,11 %2 = 1 match
			*/
			if (j == (pquery - ptr) + examnum + 1 || j % 2)    /* ³B²z¤¤¤å¦rªº°ÝÃD */
			{
				count++;
				if (count > 20 && (count % 20) == 1)
				{
					int ch;

					move(1, 0);
					clrtoeol();
					outs("[1;33m¥»­¶¬d¸ß¤wº¡¡A´«­¶¬d¸ß½Ð«ö¥ô·NÁä¡Aµ²§ô¬d¸ß½Ð«ö q[m");
					ch = vkey();
					if (ch == 'q' || ch == 'Q')
					{
						fclose(fp);
						return 0;
					}
					i = 0;
					vs_bar("Áp¦Òº]³æ¬d¸ß");
					move(2, 0);
					outs("[7m ­ã¦ÒÃÒ    ©m  ¦W    ¿ý ¨ú ¤j ¾Ç         ¿ý ¨ú ¬ì ¨t                            [m");
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
				prints("[1;33;42m º]³æ¬d¸ß [;30;47m ¥Ø«e¦@¬d¸ß¨ì [31m%5d [30m¸ê®Æ                                             [m",
					   count);
				refresh();
			}
			while ((ptr = strtok(NULL, "    ")))
			{
				if ((pquery = strstr(ptr, query)))
					j = (pquery - ptr) + examnum + 1;
				else
					j = 2; /* ªí¥Ü¥¼§ä¨ì */

				if (j == (pquery - ptr) + examnum + 1 || j % 2)    /* ³B²z¤¤¤å¦rªº°ÝÃD */
				{
					count++;
					if (count > 20 && (count % 20) == 1)
					{
						int ch;

						move(1, 0);
						clrtoeol();
						outs("[1;33m¥»­¶¬d¸ß¤wº¡¡A´«­¶¬d¸ß½Ð«ö¥ô·NÁä¡Aµ²§ô¬d¸ß½Ð«ö q[m");
						ch = vkey();
						if (ch == 'q' || ch == 'Q')
						{
							fclose(fp);
							return 0;
						}
						i = 0;
						vs_bar("Áp¦Òº]³æ¬d¸ß");
						move(2, 0);
						outs("[7m ­ã¦ÒÃÒ    ©m ¦W     ¿ý ¨ú ¤j ¾Ç         ¿ý ¨ú ¬ì ¨t                            [m");
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
					prints("[1;33;42m º]³æ¬d¸ß [;30;47m ¥Ø«e¦@¬d¸ß¨ì [31m%5d [30m¸ê®Æ                                             [m",
						   count);
					refresh();
				} // end while
			}
		}
		fclose(fp);
	}
	move(1, 0);
	clrtoeol();
	outs("[1;36m¬d¸ßµ²§ô¡A«ö¥ô·NÁäÂ÷¶}....[m");
	move(b_lines, 0);
	clrtoeol();
	prints("[1;33;42m º]³æ¬d¸ß [;30;47m ¥Ø«e¦@¬d¸ß¨ì [31m%5d [30m¸ê®Æ                                             [m",
		   count);
	if (count == 0)
	{
		move(4, 0);
		outs("¨S¦³¬d¸ß¨ì¥ô¦ó¸ê®Æ");
	}
	vkey();
	return 0;
}
