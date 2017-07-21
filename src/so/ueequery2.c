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
x_ueequery2()
{
	char buf[256], query[30], query2[30], msg[60];
	char fpath[80], now[80], *ptr;
	int  i, count = 0, year, examnum = 0;
	int school = 0, class = 0;
	FILE *fp;

	vs_bar("Áp¦Òº]³æ¬d¸ß");
	outs("§@ªÌ: Shen Chuan-Hsing <statue.bbs@bbs.yzu.edu.tw>\n");

	outs("ª`·N¨Æ¶µ\n"
		 "1. ¤j¾Ç¤Î¬ì¨t¨â¶µ¦Ü¤Ö»Ý¶ñ¼g¤@¶µ¤~¯à°÷¬d¸ß.\n"
		 "2. ¿é¤J³¡¤À¦r¦ê§Y¥i.\n");
	if (!vget(7, 0, "½Ð¿é¤JÁp¦Ò¦~«× (83 ~ 94)[94]¡G", buf, 3, DOECHO))
		year = 94;
	else
		year = atoi(buf);
	if (year < 83 || year > 94) return 0;

	if (year >= 83 && year <= 86)
		examnum = 6;
	else if (year >= 87 && year <= 90)
		examnum = 7;
	else if (year >= 91)
		examnum = 8;
	else
		return 0;
	sprintf(fpath, "game/%d/fa00.txt", year);

	vget(9, 0, "½Ð¿é¤J¬d¸ß¤j¾Ç (¨Ò¡G¤¸´¼)¡G", query, 20, DOECHO);
	vget(11, 0, "½Ð¿é¤J¬d¸ß¬ì¨t (¨Ò¡G¸ê°T¤uµ{)¡G", query2, 20, DOECHO);

	if (!strlen(query) && !strlen(query2))
		return 0;

	i = 0;
	sprintf(msg, "%s:%s", query, query2);
	logitfile(FN_UEEQUERY_LOG, fpath, msg);

	if ((fp = fopen(fpath, "r")))
	{
		while (fgets(buf, 256, fp))
		{

			/*
			 0001°ê¥ß»OÆW¤j¾Ç              ¤¤°ê¤å¾Ç¨t                                  1²Ä
			*/
			if (buf[0] == ' ') /* ³o¼Ë´N¬O¬ì¨tªº¶}ÀY */
			{
				ptr = strtok(buf + 5, " ");
				strcpy(now, ptr);
				if (strstr(now, query) || !query) // §ä¨ì
					school = 1;
				else
					school = 0;
				ptr = strtok(NULL, " ");
				if (strstr(ptr, query2) || !query2) //§ä¨ì
					class = 1;
				else
					class = 0;
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
			ptr = strtok(buf, " ");

			if (school && class && strlen(ptr) >= 6)  /* ³B²z¤¤¤å¦rªº°ÝÃD */
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
				if (school && class && strlen(ptr) >= 6)  /* ³B²z¤¤¤å¦rªº°ÝÃD */
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
