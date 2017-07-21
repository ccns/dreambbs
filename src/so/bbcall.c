/*-------------------------------------------------------*/
/* bbcall.c     ( NTHU CS MapleBBS Ver 3.10 )            */
/*-------------------------------------------------------*/
/* author : kepler.bbs@bbs.cs.nthu.edu.tw                */
/* target : 外掛網路傳呼                                 */
/* create : 99/03/30                                     */
/* update :   /  /                                       */
/*-------------------------------------------------------*/

#include "bbs.h"

#define mouts(y,x,s)    { move(y,x); outs(s); }

#define SERVER_0940	"www.jettone.com.tw"
#define SERVER_0943     "www.pager.com.tw"
#define SERVER_0948     "www.fitel.net.tw"
#define SERVER_0941     "www.chips.com.tw"
#define SERVER_0947     "web1.hoyard.com.tw"
#define SERVER_0945     "www.tw0945.com"
#define SERVER_0949     "www.southtel.com.tw"

#define CGI_0940        "/scripts/pager/fpage.asp"
#define CGI_0943        "/tpn/tpnasp/dowebcall.asp"
#define CGI_0948        "/cgi-bin/Webpage.dll"
#define CGI_0941        "/cgi-bin/paging1.pl"
#define CGI_0947        "/scripts/fp_page1.dll"
#define CGI_0945        "/Scripts/fiss/PageForm.exe"
#define CGI_0949        "/pager/Webpg.asp"

#define REFER_0940	"http://www.jettone.com.tw/scripts/pager/fpageChk.asp"
#define REFER_0943      "http://www.pager.com.tw/tpn/webcall/webcall.asp"
#define REFER_0948      "http://www.fitel.net.tw/html/svc03.htm"
#define REFER_0941      "http://www.chips.com.tw:9100/WEB2P/page_1.htm"
#define REFER_0947      "http://web1.hoyard.com.tw/freeway/freewayi.html"
#define REFER_0945      "http://www.tw0945.com/call_AlphaPg.HTM"
#define REFER_0949      "http://www.southtel.com.tw/numpg.htm"


#define WEBPORT         80
#define PORT9100        9100
#define PARA "\
	Connection: Keep-Alive\r\n\
	User-Agent: Lynx/2.6  libwww-FM/2.14\r\n\
	Content-type: application/x-www-form-urlencoded\r\n\
	Accept: text/html, text/plain, application/x-wais-source, application/html, */*\r\n\
	Accept-Encoding: gzip\r\n\
	Accept-Language: en\r\n\
	Accept-Charset: iso-8859-1,*,utf-8\r\n"

static char Pager_No[7]; /* Thor.990331: 供外部給定, for 好友名單bbc.
                                         名字是 Kepler取的:P */

static int
http_conn(char *server, char *s)
{
	int sockfd, start_show;
	char result[2048], fname[50];
	int cc, len;
	char *xhead, *xtail, msg[101];
	static char pool[2048];
	FILE *fp;

	if (!strcmp(server, SERVER_0941))
		sockfd = 9100;
	else
		sockfd = WEBPORT;

	if (!memcmp(s, "GET", 3))
		xhead = s;
	else
		xhead = strstr(s, "\n\n") + 2;

	sprintf(result, "%s:%d->%s\n", server, sockfd, xhead);
	blog("BBCALL", result);

	if ((sockfd = dns_open(server, sockfd)) < 0)
	{
		vmsg("無法與伺服器取得連結，傳呼失敗");
		blog("BBCALL", "dns_open fail");
		return 0;
	}
	else
	{
		mouts(22, 0, "\033[1;33m伺服器已經連接上，請稍後...................\033[m");
		refresh();
	}

	write(sockfd, s, strlen(s));
	shutdown(sockfd, 1);

	/* parser return message from web server */
	xhead = pool;
	xtail = pool;
	len = 0;
	start_show = 0;

	sprintf(fname, "tmp/%s.bbc", cuser.userid);
	fp = fopen(fname, "w");
	for (;;)
	{
		if (xhead >= xtail)
		{
			xhead = pool;
			cc = read(sockfd, xhead, sizeof(pool));
			if (cc <= 0)
				break;
			xtail = xhead + cc;
		}

		cc = *xhead++;

		if ((len == 5) && (!str_ncmp(msg, "body", 4)))
		{
			start_show = 1;
		}

		if (cc == '<')
		{
			len = 1;
			continue;
		}

		if (len)
		{
			/* support<br>and<P>and</P> */

			if (cc == '>')
			{
				if ((len == 3) && (!str_ncmp(msg, "br", 2)))
				{
					fputc('\n', fp);
				}
				else if ((len == 2) && (!str_ncmp(msg, "P", 1)))
				{
					fputc('\n', fp);
				}
				else if ((len == 3) && (!str_ncmp(msg, "/P", 2)))
				{
					fputc('\n', fp);
				}

				len = 0;
				continue;
			}

			if (len <= 5)
			{
				msg[len - 1] = cc;
			}

			len++;
			continue;
		}
		if (start_show)
		{
			if (cc != '\r')
				fputc(cc, fp);
			if (cc == '.' || cc == ']')
			{
				fputc('\n', fp);
			}
		}
	}

	close(sockfd);
	fputc('\n', fp);
	fclose(fp);

	fp = fopen(fname, "r");

	if (!strcmp(server, SERVER_0947))
		len = 0;
	else
		len = 16;

	msg[100] = '\0';
	cc = 4;

	/* show message that return from the web server */
#if 0
	clear();
	move(0, 30);
	outs("\033[1;37;44m◎ 腳趾頭正在ㄎㄡ鼻孔 ◎\033[m");
#endif

	move(3, 0);
	clrtobot();

	while (fgets(msg, 100, fp))
	{
		if ((strlen(msg) > 3) && (!strstr(msg, "上一頁")))
		{
			move(cc++, len);
			prints("\033[37;1m%s\033[0m", msg);
		}
	}
	fclose(fp);
	vmsg(NULL);
	unlink(fname);

	return 0;
}


/* lkchu.990428: 是否為數字 */
int
is_digit(str)
char *str;
{
	while (*str)
	{
		if (*str < '0' || *str > '9')
			return 0;
		str++;
	}
	return 1;
}


void
vgetime(int flag, int *Year, int *Month, int *Day, int *Hour, int *Minute)
{
	char ans[5];
	do
	{
		vget(10, 0,  "年[19-]:",   ans, 3, LCECHO);*Year = atoi(ans);
	}
	while (*Year != 99);
	do
	{
		vget(10, 15, "月[1-12]:", ans, 3, LCECHO);
	}
	while (!is_digit(ans) || (*Month = atoi(ans)) > 12 || *Month < 1);
	do
	{
		vget(10, 30, "日[1-31]:", ans, 3, LCECHO);
	}
	while (!is_digit(ans) || (*Day = atoi(ans)) > 31 || *Day < 1);
	do
	{
		vget(10, 45, "時[0-23]:", ans, 3, LCECHO);
	}
	while (!is_digit(ans) || (*Hour = atoi(ans)) > 23 || *Hour < 0);
	do
	{
		vget(10, 60, "分[0-59]:", ans, 3, LCECHO);
	}
	while (!is_digit(ans) || (*Minute = atoi(ans)) > 59 || *Minute < 0);
	if (flag == 1) *Year -= 11;
}


static void
Warnning(row)
int row;
{
	static char *Warnstr[] =
		{"\033[37;41m╭═══════════════════════╮\033[0m"
		 , "\033[37;41m║                                              ║\033[0m"
		 , "\033[37;41m║您要送出的資訊不具安全性，在傳輸期間，可能遭到║\033[0m"
		 , "\033[37;41m║                                              ║\033[0m"
		 , "\033[37;41m║第三者的竊取，尤其是密碼，請使用者小心。      ║\033[0m"
		 , "\033[37;41m║                                              ║\033[0m"
		 , "\033[37;41m╰═══════════════════════╯\033[0m"
		 , NULL
		};
	char **pstr = Warnstr;

	for (; *pstr; pstr++, row++)
		mouts(row, 15, *pstr);
}

static int
bbc0940()
{
	char Msg[64], atrn[512], sendform[1024]; /* , passwd[64]; */
	char ans[2], callmessage[sizeof Msg * 3];

	int Year = 99, Month = 1, Day = 15, Hour = 13, Minute = 8;

	if (!vget(7, 0, "請輸入傳呼號碼：0940-", Pager_No, 7, GCARRY)
		/* || !vget(8,0, "請輸入傳呼通行碼：", passwd, 15, LCECHO) */
		|| !vget(8, 0, "請輸入傳呼訊息：", Msg, 60, LCECHO))
	{
		vmsg("放棄傳呼");
		return 0;
	}

	Warnning(11);

	if (vget(20, 0, "確定要送出傳呼？[N] ", ans, 3, LCECHO) != 'y')
	{
		vmsg("放棄傳呼");
		return 0;
	}
	else
	{
		url_encode(callmessage, Msg);

		sprintf(atrn, "PrefixCode=0940&subid=%s&PagerType=S&fpoption=0&od_year=19%02d&od_month=%02d&od_day=%02d&od_hour=%02d&od_min=%02d&callmessage=%s",
				Pager_No, Year, Month, Day, Hour, Minute, callmessage);

		sprintf(sendform, "POST %s HTTP/1.0\nReferer: %s\n%sContent-length:%d\n\n%s",
				CGI_0940, REFER_0940, PARA, strlen(atrn), atrn);

		http_conn(SERVER_0940, sendform);
		return 0;
	}
}

static int
alphacall(char *CoId)
{
	char tmpbuf[64], ans[2];
	char Passwd[16], Name[32], Trn_Name[sizeof Name * 3];
	char Msg[sizeof tmpbuf *3], atrn[512], sendform[1024];
	int Year = 99, Month = 1, Day = 15, Hour = 13, Minute = 8;

	sprintf(tmpbuf, "請輸入您要傳呼的號碼 : %s-", CoId);
	vget(7, 0, tmpbuf, Pager_No, 7, GCARRY);

	vget(8, 0, "或輸入傳呼姓名：", Name, 31, LCECHO);
	if (!*Pager_No && !*Name)
	{
		vmsg("放棄傳呼");
		return 0;
	}

	url_encode(Trn_Name, Name);

	vget(9, 0, "請輸入傳呼訊息：", tmpbuf, 63, LCECHO);
	if (strlen(tmpbuf) == 0)
	{
		vmsg("放棄傳呼");
		return 0;
	}
	vget(10, 0, "請輸入傳呼密碼：", Passwd, 15, NOECHO);

	Warnning(11);

	vget(20, 0, "確定要送出傳呼?[N] ", ans, 2, LCECHO);

	if (ans[0] != 'y')
	{
		vmsg("放棄傳呼");
		return 0;
	}
	/* gettime(0,&Year,&Month,&Day,&Hour,&Minute); */
	else
	{
		url_encode(Msg, tmpbuf);
		sprintf(atrn, "CoId=%s&ID=%s&Name=%s&FriendPassword=%s&Year=19%02d&Month=%02d&Day=%02d&Hour=%02d&Minute=%02d&Msg=%s",
				CoId, Pager_No, Trn_Name, Passwd, Year, Month, Day, Hour, Minute, Msg);
		sprintf(sendform, "POST %s HTTP/1.0\nReferer: %s\n%sContent-length:%d\n\n%s",
				CGI_0943, REFER_0943, PARA, strlen(atrn), atrn);
		http_conn(SERVER_0943, sendform);
		return 0;
	}
}

static int
bbc0941()
{
	char ans[2];
	char PASSWD[8], TRAN_MSG[17*3];
	char trn[512], sendform[512];
	/* int  year=98,month=12,day=4,hour=13,min=8; */
	int year = 99, month = 2, day = 26, hour = 12, min = 30;

	if (!vget(7, 0, "請您輸入您要傳呼的號碼 : 0941-", Pager_No, 7, GCARRY)
		|| !vget(8, 0, "請輸入傳呼訊息：", trn, 17, LCECHO))
	{
		vmsg("放棄傳呼");
		return 0;
	}

	vget(9, 0, "請輸入傳呼密碼：", PASSWD, 7, NOECHO);

	Warnning(11);

	if (vget(20, 0, "確定要送出傳呼?[N] ", ans, 2, LCECHO) != 'y')
	{
		vmsg("放棄傳呼");
		return 0;
	}
	else
	{
		url_encode(TRAN_MSG, trn);
		sprintf(trn, "PAGER_NO=%s&PASSWD=%s&TRAN_MSG=%s&MSG_TYPE=NUMERIC&NOW=on&year=19%02d&month=%02d&day=%02d&hour=%02d&min=%02d",
				Pager_No, PASSWD, TRAN_MSG, year, month, day, hour, min);

		sprintf(sendform, "POST %s HTTP/1.0\nReferer: %s\n%sContent-length:%d\n\n%s",
				CGI_0941, REFER_0941, PARA, strlen(trn), trn);

		http_conn(SERVER_0941, sendform);
		return 0;
	}
}

static int
bbc0945()
{
	char txSender[16], txPasswd[5], txContent[63 * 3], trn[512], sendform[1024], ans[2];
	int txYear = 99, txMonth = 1, txDay = 15, txHour = 8, txMinute = 30;
	char caller[sizeof txSender * 3];

	vget(7, 0, "請輸入您想呼叫的號碼 0945-", Pager_No, 7, GCARRY);
	vget(8, 0, "傳呼者:", txSender, 15, LCECHO);
	url_encode(caller, txSender);
	vget(9, 0, "請輸入傳呼訊息：", trn, 63, LCECHO);
	if (!*Pager_No || !*trn)
	{
		vmsg("放棄傳呼");
		return 0;
	}

	vget(10, 0, "請輸入傳呼密碼：", txPasswd, 5, NOECHO);

	Warnning(12);

	if (vget(21, 0, "確定要送出傳呼?[N] ", ans, 2, LCECHO) != 'y')
	{
		vmsg("放棄傳呼");
		return 0;
	}
	else
	{
		url_encode(txContent, trn);

		sprintf(trn, "hiUsage=AlphaPage&hiLanguage=Taiwan&hiDataDir=R%%3A%%5Cfiss%%5CRmtFiles&hiInterval=5&txPagerNo=%s&txPassword=%s&txSender=%s&txContent=%s&txYear=19%02d&txMonth=%02d&txDay=%02d&txHour=%02d&txMinute=%02d\n",
				Pager_No, txPasswd, caller, txContent, txYear, txMonth, txDay, txHour, txMinute);

		sprintf(sendform, "POST %s HTTP/1.0\nReferer:%s\n%sContent-length:%d\n\n%s",
				CGI_0945, REFER_0945, PARA, strlen(trn), trn);

		http_conn(SERVER_0945, sendform);
		return 0;
	}
}

static int
bbc0947()
{
	char Message[63 * 3], trn[512], sendform[1024], Sender[16], ans[2];
	char Name[sizeof Sender * 3], Password_0[32];

	if (!vget(7, 0, "請輸入您想傳呼的號碼 0947-", Pager_No, 7, GCARRY))
	{
		vmsg("放棄傳呼!!");
		return 0;
	}

	vget(8, 0, "請輸入傳送者代號:", Sender, 15, LCECHO);

	url_encode(Name, Sender);

	vget(9, 0, "請輸入網呼密碼:", Password_0, 9, NOECHO);

	if (!vget(10, 0, "您想傳呼的內容:", trn, 63, LCECHO))
	{
		vmsg("放棄傳呼!!");
		return 0;
	}

	Warnning(12);

	if (vget(21, 0, "確定要送出傳呼?[N] ", ans, 2, LCECHO) != 'y')
	{
		vmsg("放棄傳呼");
		return 0;
	}
	else
	{
		url_encode(Message, trn);

		sprintf(trn, "AccountNo_0=%s&Password_0=%s&Sender=%s&Message=%s", Pager_No, Password_0, Name, Message);

		sprintf(sendform, "POST %s HTTP/1.0\nReferer:%s\n%sContent-length:%d\n\n%s",
				CGI_0947, REFER_0947, PARA, strlen(trn), trn);

		http_conn(SERVER_0947, sendform);
		return 0;
	}
}


static int
bbc0948()
{
	int Year = 88, Month = 3, Day = 26, Hour = 13, Minute = 8, reminder = 0;
	char message[61*3], trn[256], sendform[512], ans[3];

	move(7, 0);
	clrtoeol();

	if (!vget(7, 0, "請輸入您要傳呼的號碼：0948-", Pager_No, 7, GCARRY)
		|| !vget(8, 0, "請輸入傳呼訊息：", trn, 61, LCECHO))
	{
		vmsg("放棄傳呼");
		return 0;
	}

	/* vget(9,0, "請輸入傳呼密碼：", PASSWD, 7, NOECHO); */
	/* lkchu.990428: 還是喜歡有定時功能 :p */
	vget(9, 0, "如果你要馬上送請按 '1' 如果要定時送請按 '2': ", ans, 2, LCECHO);
	if (ans[0] == '2')
	{
		vgetime(1, &Year, &Month, &Day, &Hour, &Minute);
		reminder = 1;
	}

	Warnning(11);

	if (vget(20, 0, "確定要送出傳呼?[N] ", ans, 2, LCECHO) != 'y')
	{
		vmsg("放棄傳呼");
		return 0;
	}
	else
	{
		url_encode(message, trn);
		sprintf(trn, "MfcISAPICommand=SinglePage&svc_no=%s&reminder=%d&year=%02d&month=%02d&day=%02d&hour=%02d&min=%02d&message=%s",
				Pager_No, reminder, Year, Month, Day, Hour, Minute, message);

		sprintf(sendform, "GET %s?%s Http/1.0\n\n", CGI_0948, trn);

		http_conn(SERVER_0948, sendform);
		return 0;
	}
}

static int
bbc0949()
{
	char ans[2];
	char PASSWD[8], TRAN_MSG[17 * 3], username[16], Alias[sizeof username * 3];
	char trn[512], sendform[512];
	int year = 99, month = 2, day = 26, hour = 12, min = 30;

	vget(7, 0, "請您輸入您要傳呼的號碼 : 0949-", Pager_No, 7, GCARRY);
	vget(8, 0, "或輸入別名：", username, 15, LCECHO);
	url_encode(Alias, username);
	vget(9, 0, "請輸入傳呼訊息：", trn, 17, LCECHO);

	if ((!*Pager_No && !*username) || !*trn)
	{
		vmsg("放棄傳呼");
		return 0;
	}

	vget(10, 0, "請輸入傳呼密碼：", PASSWD, 5, NOECHO);

	Warnning(12);

	if (vget(21, 0, "確定要送出傳呼?[N] ", ans, 2, LCECHO) != 'y')
	{
		vmsg("放棄傳呼");
		return 0;
	}
	else
	{
		url_encode(TRAN_MSG, trn);

		sprintf(trn, "func=Numpg&txPagerNo=%s&username=%s&txPassword=%s&txContent=%s&tran_type=on&txYear=19%02d&txMonth=%02d&txDay=%02d&txHour=%02d&txMinute=%02d",
				Pager_No, Alias, PASSWD, TRAN_MSG, year, month, day, hour, min);

		sprintf(sendform, "POST %s HTTP/1.0\nReferer: %s\n%sContent-length:%d\n\n%s",
				CGI_0949, REFER_0949, PARA, strlen(trn), trn);

		http_conn(SERVER_0949, sendform);
		return 0;
	}

}

static int
bbc0943()
{
	alphacall("0943");
	return 0;
}

static int
bbc0946()
{
	alphacall("0946");
	return 0;
}

static struct Map
{
	char *prefix;
	int (*func)();
} bbc_map[] =
{
	{"0940", bbc0940},
	{"0941", bbc0941},
	{"0943", bbc0943},
	{"0945", bbc0945},
	{"0946", bbc0946},
	{"0947", bbc0947},
	{"0948", bbc0948},
	{"0949", bbc0949},
	{ NULL, NULL},
};

int
main_bbcall()
{
	char ch[2];

	clear();
	move(0, 30);
	outs("\033[1;37;44m◎ 腳趾頭正在ㄎㄡ鼻孔 ◎\033[m");
	move(3, 0);
	outs("  \033[1;33;44m╭════════════════════════════════════╮\033[0m\n");
	outs("  \033[1;33;44m║ (1)0940  (2)0941  (3)0943  (4)0945  (5)0946  (6)0947  (7)0948  (8)0949 ║\033[0m\n");
	outs("  \033[1;33;44m╰════════════════════════════════════╯\033[0m\n");
	vget(7, 0, "你的選擇? [1-8]", ch, 2, LCECHO);
	if (*ch >= '1' && *ch <= '8')
	{
		*Pager_No = '\0';
		(*bbc_map[*ch-'1'].func)();
	}
	return 0;
}

int
pal_bbc(xo)
XO *xo;
{
	PAL *pal;
	struct Map *map;
	int pos, cur;
	char *p;

	if (!HAS_PERM(PERM_INTERNET))
		return XO_NONE;

	pos = xo->pos;
	cur = pos - xo->top;
	pal = (PAL *) xo_pool + cur;

	if (!(p = str_str(pal->ship, "bbc")))
	{
		vmsg("無 BBC 描述");
		return XO_FOOT;
	}

	for (map = bbc_map; map->prefix; map ++)
		if (!memcmp(p + 3, map->prefix, 4))
			break;

	if (!map->func)
	{
		vmsg("不支援此廠商 BBCall");
		return XO_FOOT;
	}

	clear();
	move(0, 30);
	outs("\033[1;37;44m◎ 腳趾頭正在ㄎㄡ鼻孔 ◎\033[m");

	move(2, 0);
	prints("被呼叫的人是: %s", pal->userid);

	str_ncpy(Pager_No, p + 7, sizeof(Pager_No));

	(*map->func)();

	/* finish call */
	return XO_HEAD;
}
