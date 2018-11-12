/************************************************
*  pnote.c                                      *
*  ¯d¸Ü¾÷                                       *
*                                1998.4.14      *
*                                    by herb    *
*************************************************/

#include        "bbs.h"
#define MAX_PNOTE        (10)            /* µª¿ý¾÷«O¯d³q¼Æ */
#define MAXHINTLINES     (10)            /* µª¿ý¾÷¥D¤H¯d¨¥ªø«× */

static char *fn_note_tmp      = FN_PNOTE_TMP;
static char *fn_note_dat      = FN_PNOTE_DAT;
static char *fn_note_dat_save = FN_PNOTE_DAT_SAVE;
static char *fn_pnote_ans      = FN_PNOTE_ANS;
static char *fn_pnote_ans_save = FN_PNOTE_ANS_SAVE;

char *fn_pnote_hint = FN_PNOTE_HINT;    /* µª¿ý¾÷¥D¤H¯d¨¥ */

char Bdate[128];
#include "pipfun.c"

#define getdata(x1,x2,x3,x4,x5,x6,x7)  vget(x1,x2,x3,x4,x5,DOECHO)
#define setuserfile(x1,x2)  usr_fpath(x1,cuser.userid,x2)

int t_lines = b_lines + 1;

extern UCACHE *ushm;

static void
m_biffn(
int userno)
{
	UTMP *utmp, *uceil;

	utmp = ushm->uslot;
	uceil = (void *) utmp + ushm->offset;
	do
	{
		if (utmp->userno == userno)
		{
			utmp->ufo |= UFO_BIFFN;

#ifdef  BELL_ONCE_ONLY
			return;
#endif
		}
	}
	while (++utmp <= uceil);
}

static void
rebuild_pnote_ansi(int newflag)
{
	char fname[MAXPATHLEN];
	char fpath[MAXPATHLEN];
	char buf[256], buf2[80];
	int total = 0, len, i;
	int fd;
	struct stat st;
	notedata myitem;
	FILE *fp;

	if (newflag)
	{
		setuserfile(fname, fn_pnote_ans);
		setuserfile(fpath, fn_note_dat);
	}
	else
	{
		setuserfile(fname, fn_pnote_ans_save);
		setuserfile(fpath, fn_note_dat_save);
	}

	if ((fp = fopen(fname, "w")) == NULL)
	{
		return;
	}

	if ((fd = open(fpath, O_RDONLY)) == -1)
	{
		unlink(fname);
		return;
	}
	else if (fstat(fd, &st) != -1)
	{
		total = st.st_size / sizeof(notedata);
		if (total > MAX_PNOTE)
			total = MAX_PNOTE;
	}
	fputs("\t\t\t[1;32m ¡¹ [37mµª ¿ý ¾÷ ¤¤ ªº ¯d ¨¥[32m¡¹ \n\n", fp);

	while (total)
	{
		if (total--)
			read(fd, (char *) &myitem, sizeof(myitem));
		sprintf(buf, "[1;33m¡¼ùù [32m%s[37m(%s)",
				myitem.userid, myitem.username);
		len = strlen(buf);
		strcat(buf, " [33m" + (len & 1));

		for (i = len >> 1; i < 36; i++)
			strcat(buf, "ùù");
		sprintf(buf2, "ùù[32m %.14s [33mùù¡¼[m\n",
				Cdate(&(myitem.date)));
		strcat(buf, buf2);
		fputs(buf, fp);

		sprintf(buf, "%s\n%s\n%s\n", myitem.buf[0], myitem.buf[1], myitem.buf[2]);
		fputs(buf, fp);

	}
	fclose(fp);
	close(fd);
}

static void
do_pnote(char *userid)
{
	int total = 0, i, collect, len;
	struct stat st;
	char buf[256], buf2[80];
	char fname[MAXPATHLEN], fname2[MAXPATHLEN];
	int fd, fx;
	FILE *fp;
	notedata myitem;

	clrtobot();
	move(13, 0);
	prints("[1;33m¤f²¦¡ã¡ã¡ã¡I[m");
	memset(&myitem, 0, sizeof(notedata));
	do
	{
		/*    myitem.buf[0][0] = myitem.buf[1][0] = myitem.buf[2][0] = '\0';*/
		move(14, 0);
		clrtobot();
		outs("\n½Ð¯d¸Ü (¦Ü¦h¤T¥y)¡A«ö[Enter]µ²§ô");
		for (i = 0; (i < 3) &&
			 getdata(16 + i, 0, ":", myitem.buf[i], 77, DOECHO, 0); i++);

		getdata(b_lines - 1, 0, "(S)Àx¦s (E)­«·s¨Ó¹L (Q)¨ú®ø¡H[S] ", buf, 3, LCECHO, 0);
		if ((buf[0] == 'q' || i == 0) && *buf != 'e')
		{
			return;
		}
		myitem.mode = 0;
	}
	while (buf[0] == 'e');

	utmp_mode(M_XMODE);
	strcpy(myitem.userid, cuser.userid);
	strncpy(myitem.username, cuser.username, 18);
	myitem.username[18] = '\0';
	time(&(myitem.date));

	/* begin load file */

	usr_fpath(fname, userid, fn_pnote_ans);
	if ((fp = fopen(fname, "w")) == NULL)
	{
		return;
	}

	usr_fpath(fname, userid, fn_note_tmp);
	if ((fx = open(fname, O_WRONLY | O_CREAT, 0644)) <= 0)
	{
		fclose(fp);
		return;
	}

	usr_fpath(fname2, userid, fn_note_dat);
	if ((fd = open(fname2, O_RDONLY)) == -1)
	{
		total = 1;
	}
	else if (fstat(fd, &st) != -1)
	{
		total = st.st_size / sizeof(notedata) + 1;
		if (total > MAX_PNOTE)
			total = MAX_PNOTE;
	}

	fputs("\t\t\t[1;32m ¡¹ [37m±z ªº µª ¿ý ¾÷ !!! [32m¡¹ \n\n", fp);
	collect = 1;
	while (total)
	{
		sprintf(buf, "[1;33m¡¼ùù [32m%s[37m(%s)",
				myitem.userid, myitem.username);
		len = strlen(buf);
		strcat(buf, " [33m" + (len & 1));

		for (i = len >> 1; i < 36; i++)
			strcat(buf, "ùù");
		sprintf(buf2, "ùù[32m %.14s [33mùù¡¼[m\n",
				Cdate(&(myitem.date)));
		strcat(buf, buf2);
		fputs(buf, fp);

		sprintf(buf, "%s\n%s\n%s\n", myitem.buf[0], myitem.buf[1], myitem.buf[2]);
		fputs(buf, fp);

		write(fx, &myitem, sizeof(myitem));

		if (--total)
			read(fd, (char *) &myitem, sizeof(myitem));
	}
	fclose(fp);
	close(fd);
	close(fx);
	f_mv(fname, fname2);
	m_biffn(acct_userno(userid));
}

static void
show_pnote(notedata *pitem)
{
	char str[200];

	clrchyiuan(2, 6);
	move(2, 0);
	sprintf(str, "[1;36m¢z¢w¢w¢w [37m%s(%s)¦b [33m%s[37m ¯dªº¸Ü [m", pitem->userid, pitem->username,
			Cdate(&(pitem->date)));
	prints(str);
	prints("\n[1;37m  %s\n  %s\n  %s\n[0m", pitem->buf[0], pitem->buf[1], pitem->buf[2], "");
	prints("                 [1;36m¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢}[m\n");
	pitem->mode = 1;
}

#if 0
static void
del_pnote(notedata *pitem, int newflag)
{
	char fpath[MAXPATHLEN];
	char fold[MAXPATHLEN];
	FILE *fp1, *fp2;
	notedata item;

	if (newflag)
		setuserfile(fpath, fn_note_dat);
	else
		setuserfile(fpath, fn_note_dat_save);

	sprintf(fold, "%s.tmp", fpath);
	f_mv(fpath, fold);
	if ((fp1 = fopen(fold, "r")) != NULL)
	{
		if ((fp2 = fopen(fpath, "w")) != NULL)
		{
			while (fread(&item, sizeof(item), 1, fp1) != 0)
			{
				if (memcmp(pitem, &item, sizeof(item)))
				{
					fwrite(&item, sizeof(item), 1, fp2);
				}
			}
			fclose(fp2);
		}
		fclose(fp1);
	}
	unlink(fold);
}
#endif

/*                                                              *
 *  show_pnote_hint()§ï¦Ûshowplan(), ¥i¥Hshow¥X¥D¤H¦Û»sªºµª¿ý¾÷ *
 *  ¯d¨¥, ­YµL¯d¨¥«h·|show¹w³]¯d¨¥                              *
 *                                          - add by wisely -   */

static void
show_pnote_hint(
char *uid)
{
	FILE *hintfile;
	int i;
	char genbuf[256];

	usr_fpath(genbuf, uid, fn_pnote_hint);
	if ((hintfile = fopen(genbuf, "r")))
	{
		prints("[1;34m¡´¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¡´[m\n", uid);
		i = 0;
		while (i++ < MAXHINTLINES && fgets(genbuf, 256, hintfile))
		{
			outs(genbuf);
		}
		prints("[1;34m¡´¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¡´[m\n", uid);
		fclose(hintfile);
	}
	else
		prints("±z¦n¡A³o¬O %s ªº¹q¸Üµª¿ý¾÷¡A", uid);
	prints("\n½Ð¦bÅ¥¨ì¡u¹Í¡vÁn«á¡A¶}©l¯d¨¥¡AÁÂÁÂ¡C");
}

/*                                                              *
 *  p_edithint()§ï¦Ûu_editplan(), ¥i¥H­×§ï¥D¤H¯d¨¥              *
 *                                      - add by wisely         */

static int
p_edithint(void)
{
	char genbuf[200];

	if (bbsothermode & OTHERSTAT_EDITING)
	{
		vmsg("§AÁÙ¦³ÀÉ®×ÁÙ¨S½s§¹®@¡I");
		return -1;
	}


	sprintf(genbuf, "¥D¤H¯d¨¥(³Ì¦h%d¦æ) (D)§R°£ (E)¿ý»s [Q]¨ú®ø¡H[Q]", MAXHINTLINES);
	getdata(b_lines - 1, 0, genbuf, genbuf, 3, LCECHO, 0);

	if (genbuf[0] == 'e')
	{
		int aborted;

		utmp_mode(M_XMODE);
		setuserfile(genbuf, fn_pnote_hint);
		aborted = vedit(genbuf, NA);
		if (!aborted)
			outs("¯d¨¥¿ý»s§¹²¦");
		vmsg(NULL);
		return 0;
	}
	else if (genbuf[0] == 'd')
	{
		setuserfile(genbuf, fn_pnote_hint);
		unlink(genbuf);
		vmsg("¯d¨¥§R°£§¹²¦");
	}
	return 0;
}


static int
get_pnote(notedata *pitem, int newflag)
{
	FILE *fp;
	int  total = 0;
	notedata myitem;
	char fpath[MAXPATHLEN];

	if (newflag)
		setuserfile(fpath, fn_note_dat);
	else
		setuserfile(fpath, fn_note_dat_save);

	if ((fp = fopen(fpath, "r")) != NULL)
	{
		while (fread(&myitem, sizeof(myitem), 1, fp) == 1)
		{
			memcpy(&pitem[total++], &myitem, sizeof(notedata));
		}
		fclose(fp);
		pitem[total].userid[0] = '\0';
		return total;
	}
	return 0;
}


static void
Pnote(int newflag)
{
	int offset, num, num1, i;
	char ans[4], prompt[STRLEN];
	notedata item_array[MAX_PNOTE + 1];
	char fpath[MAXPATHLEN];
	FILE *fp;

	if ((num = get_pnote(item_array, newflag)) == 0)
		return;

	utmp_mode(M_XMODE);
	rebuild_pnote_ansi(newflag);
	offset = 1;
	while (offset <= num)
	{
		move(1, 0);
		clrtobot();
		prints("[1;36mµª¿ý¾÷¸Ìªº[37m%2d/%2d[36m³q%s¯d¨¥[0m\n", offset, num, newflag ? "·sªº" : "");

		show_pnote(&item_array[offset - 1]);
		sprintf(prompt, "(N/P)©¹«e/«á (A)¥þ³¡ (R)¦^¹q %s(X)§R°£¥þ³¡ (E)Â÷¶}:", newflag ? "(S)«O¯d " : "(D)§R°£ ");
		getdata(t_lines - 2, 0, prompt, ans, 2, DOECHO, 0);

		if (ans[0] == 'r' || ans[0] == 'R')
		{
			do_pnote(item_array[offset - 1].userid);
			offset++;
		}
		else if (ans[0] == 'p' || ans[0] == 'P')
		{
			if (offset <= 1) offset = 1;
			else offset--;
		}
#if 0
		else if (ans[0] == 'q' || ans[0] == 'Q')
		{
			my_query(item_array[offset - 1].userid, 0);
			offset++;
		}
#endif
		else if (ans[0] == 'a' || ans[0] == 'A')
		{
			for (i = 0; i < num; i++)
				item_array[i].mode = 0;
			if (newflag)
				setuserfile(fpath, fn_pnote_ans);
			else
				setuserfile(fpath, fn_pnote_ans_save);
			more(fpath, NULL);
			break;
		}
		else if ((ans[0] == 'd' || ans[0] == 'D') && !newflag)
		{
			for (i = offset - 1;i < num;i++)
				memcpy(&item_array[i], &item_array[i+1], sizeof(notedata));

			num--;
			if (num == 0)
				break;
			if (offset > num)
				offset = num;
		}
		else if ((ans[0] == 's' || ans[0] == 'S') && newflag)
		{
			setuserfile(fpath, fn_note_dat_save);
			if ((num1 = get_pnote(item_array, 0)) >= MAX_PNOTE)
			{
				move(t_lines - 3, 0);
				vmsg("µª¿ý¾÷¤w¸g¿ý¨ì©³Åo¡A¨S¿ìªk«O¦s¤F¡A°O±o§Ö¾ã²z¾ã²z³á...");
				break;
			}
			else if ((num1 = get_pnote(item_array, 0)) >= MAX_PNOTE - 3)
			{
				move(t_lines - 3, 0);
				vmsg("µª¿ý¾÷§Öº¡¤F, °O±o²M²z²M²z³á...");
			}
			// shakalaca patch [­ì¥»Åã¥Üªº¤º®e³£ÅÜ¦¨²Ä¤@½g]
			get_pnote(item_array, 1);
			if ((fp = fopen(fpath, "a")) != NULL)
			{
				if (fwrite(&item_array[offset - 1], sizeof(notedata), 1, fp) != 1)
					break;

				fclose(fp);
			}
			for (i = offset - 1;i < num;i++)
				memcpy(&item_array[i], &item_array[i+1], sizeof(notedata));

			num--;
			if (num == 0)
				break;
			if (offset > num)
				offset = num;

		}
#if 0
		else if (ans[0] == 'm'  || ans[0] == 'M')
		{
			fileheader mymail;
			char title[128], buf[80];

			if (newflag)
				setuserfile(fpath, fn_pnote_ans);
			else
				setuserfile(fpath, fn_pnote_ans_save);

			sethomepath(buf, cuser.userid);
			stampfile(buf, &mymail);

			mymail.savemode = 'H';        /* hold-mail flag */
			mymail.filemode = FILE_READ;
			strcpy(mymail.owner, "[³Æ.§Ñ.¿ý]");
			strcpy(mymail.title, "¯d¨¥[37;41m°O¿ý[m");
			sethomedir(title, cuser.userid);
			rec_add(title, &mymail, sizeof(mymail));
			f_mv(fpath, buf);
		}
#endif
		else if (ans[0] == 'x' || ans[0] == 'X')
		{
			item_array[0].userid[0] = '\0';
			break;
		}
		else if (ans[0] == 'e' || ans[0] == 'E')
		{
			break;
		}
		else
		{
			offset++;
		}
		if (offset > num)
			offset = num;
	}

	if (newflag)
		setuserfile(fpath, fn_note_dat);
	else
		setuserfile(fpath, fn_note_dat_save);

	offset = 0;
	if ((fp = fopen(fpath, "w")) != NULL)
	{
		while (item_array[offset].userid[0] != '\0')
		{
			if (newflag && item_array[offset].mode != 0)
			{
				offset++;
				continue;
			}

			if (fwrite(&item_array[offset], sizeof(notedata), 1, fp) != 1)
				break;

			offset++;
		}
		fclose(fp);
	}
	rebuild_pnote_ansi(newflag);
	return;
}

/*                                                      *
 *  p_call(), ¤@­ÓÂ²³æªº¯d¨¥¤¶­±, ¥[¦bmenu¤¤´N¥i¥H¥Î¤F  *
 *  ±ý¥[¤J¨ä¥¦ªº¶Ç©I¥\¯à¥i¥Ñ¦¹¥[¤J.                     *
 *                          - add by wisely 5/5/98 -    */

static int
p_call(void)
{
	char genbuf[200];

	vs_bar("¯d¨¥µ¹.....");
	usercomplete("§A·Q¯d¨¥µ¹½Ö¡G", genbuf);

	if (genbuf[0])
	{
		clear();
		move(1, 0);
		show_pnote_hint(genbuf);
		do_pnote(genbuf);
	}
	return 0;
}

/*
 * p_read() ¬O³]­p¥Î¨Ó¥i¥HÂ\¦b menu ¤¤ªº. ¥u­n§â¥L¥[¦b·Q¥[ªºmenu¸Ì
 * ´N¥i¥H¨Ï¥ÎÅo.
 */
static int
p_read(void)
{
	char ans[4];
	char prompt[STRLEN];

	sprintf(prompt, "(N)·sªº¯d¨¥/(S)³Q«O¦sªº¯d¨¥ [%c]", check_personal_note(1, NULL) ? 'N' : 'S');
	getdata(b_lines, 0, prompt, ans, 2, DOECHO, 0);
	if (ans[0] == 'n')
		Pnote(1);
	else if (ans[0] == 's')
		Pnote(0);
	else if (check_personal_note(1, NULL))
		Pnote(1);
	else
		Pnote(0);

	return 0;
}

int
main_note(void)
{
	char ans;
	ans = vans("­Ó¤Hµª¿ý¾÷¡GR)¾\\Åª¯d¨¥ C)¿ý»s¯d¨¥ E)§ó§ï°Ý­Ô»y Q)Â÷¶} [Q]¡G");
	switch (ans)
	{
case 'r': case 'R':
		cutmp->ufo &= ~UFO_BIFFN;
		p_read();
		break;
case 'c': case 'C':
		p_call();
		break;
case 'e': case 'E':
		p_edithint();
		break;
	}
	return 0;
}
