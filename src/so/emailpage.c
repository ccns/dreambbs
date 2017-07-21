/*-------------------------------------------------------*/
/* bbcall.c     ( NTHU CS MapleBBS Ver 3.10 )            */
/*-------------------------------------------------------*/
/* author : kepler.bbs@bbs.cs.nthu.edu.tw                */
/* target : 外掛網路傳呼                                 */
/* create : 99/03/30                                     */
/* update :   /  /                                       */
/*-------------------------------------------------------*/

#include "bbs.h"


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

static char MyMsg[512];  /* lkchu.990429: 把要送的訊息抓出來 :p */


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

	if ((sockfd = dns_open(server, sockfd)) < 0)
		return 0;

	write(sockfd, s, strlen(s));
	shutdown(sockfd, 1);

	/* parser return message from web server */
	xhead = pool;
	xtail = pool;
	len = 0;
	start_show = 0;

	sprintf(fname, "tmp/%s.bbc", Pager_No);
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

	unlink(fname);

	return 0;
}


static int
bbc0940()
{
	char atrn[512], sendform[1024]; /* , passwd[64]; */
	char callmessage[sizeof MyMsg * 3];

	int Year = 99, Month = 1, Day = 15, Hour = 13, Minute = 8;

	url_encode(callmessage, MyMsg);

	sprintf(atrn, "PrefixCode=0940&subid=%s&PagerType=S&fpoption=0&od_year=19%02d&od_month=%02d&od_day=%02d&od_hour=%02d&od_min=%02d&callmessage=%s",
			Pager_No, Year, Month, Day, Hour, Minute, callmessage);

	sprintf(sendform, "POST %s HTTP/1.0\nReferer: %s\n%sContent-length:%d\n\n%s",
			CGI_0940, REFER_0940, PARA, strlen(atrn), atrn);

	http_conn(SERVER_0940, sendform);
	return 0;
}

static int
alphacall(char *CoId)
{
	char Passwd[16], Name[32], Trn_Name[sizeof Name * 3];
	char tmpbuf[sizeof MyMsg *3], atrn[512], sendform[1024];
	int Year = 99, Month = 1, Day = 15, Hour = 13, Minute = 8;

	url_encode(tmpbuf, MyMsg);
	sprintf(atrn, "CoId=%s&ID=%s&Name=%s&FriendPassword=%s&Year=19%02d&Month=%02d&Day=%02d&Hour=%02d&Minute=%02d&Msg=%s",
			CoId, Pager_No, Trn_Name, Passwd, Year, Month, Day, Hour, Minute, tmpbuf);
	sprintf(sendform, "POST %s HTTP/1.0\nReferer: %s\n%sContent-length:%d\n\n%s",
			CGI_0943, REFER_0943, PARA, strlen(atrn), atrn);
	http_conn(SERVER_0943, sendform);
	return 0;
}

static int
bbc0941()
{
	char PASSWD[8], TRAN_MSG[17*3];
	char trn[512], sendform[512];
	/* int  year=98,month=12,day=4,hour=13,min=8; */
	int year = 99, month = 2, day = 26, hour = 12, min = 30;

	url_encode(TRAN_MSG, MyMsg);
	sprintf(trn, "PAGER_NO=%s&PASSWD=%s&TRAN_MSG=%s&MSG_TYPE=NUMERIC&NOW=on&year=19%02d&month=%02d&day=%02d&hour=%02d&min=%02d",
			Pager_No, PASSWD, TRAN_MSG, year, month, day, hour, min);

	sprintf(sendform, "POST %s HTTP/1.0\nReferer: %s\n%sContent-length:%d\n\n%s",
			CGI_0941, REFER_0941, PARA, strlen(trn), trn);

	http_conn(SERVER_0941, sendform);
	return 0;
}

static int
bbc0945()
{
	char txSender[16], txPasswd[5], txContent[63 * 3], trn[512], sendform[1024];
	int txYear = 99, txMonth = 1, txDay = 15, txHour = 8, txMinute = 30;
	char caller[sizeof txSender * 3];

	url_encode(caller, txSender);
	url_encode(txContent, MyMsg);

	sprintf(trn, "hiUsage=AlphaPage&hiLanguage=Taiwan&hiDataDir=R%%3A%%5Cfiss%%5CRmtFiles&hiInterval=5&txPagerNo=%s&txPassword=%s&txSender=%s&txContent=%s&txYear=19%02d&txMonth=%02d&txDay=%02d&txHour=%02d&txMinute=%02d\n",
			Pager_No, txPasswd, caller, txContent, txYear, txMonth, txDay, txHour, txMinute);

	sprintf(sendform, "POST %s HTTP/1.0\nReferer:%s\n%sContent-length:%d\n\n%s",
			CGI_0945, REFER_0945, PARA, strlen(trn), trn);

	http_conn(SERVER_0945, sendform);
	return 0;
}

static int
bbc0947()
{
	char Message[63 * 3], trn[512], sendform[1024], Sender[16];
	char Name[sizeof Sender * 3], Password_0[32];

	url_encode(Name, Sender);
	url_encode(Message, MyMsg);

	sprintf(trn, "AccountNo_0=%s&Password_0=%s&Sender=%s&Message=%s", Pager_No, Password_0, Name, Message);

	sprintf(sendform, "POST %s HTTP/1.0\nReferer:%s\n%sContent-length:%d\n\n%s",
			CGI_0947, REFER_0947, PARA, strlen(trn), trn);

	http_conn(SERVER_0947, sendform);
	return 0;
}


static int
bbc0948()
{
	int Year = 88, Month = 3, Day = 26, Hour = 13, Minute = 8;
	char message[61*3], trn[256], sendform[512];

	url_encode(message, MyMsg);
	sprintf(trn, "MfcISAPICommand=SinglePage&svc_no=%s&reminder=0&year=%02d&month=%02d&day=%02d&hour=%02d&min=%02d&message=%s",
			Pager_No, Year, Month, Day, Hour, Minute, message);

	sprintf(sendform, "GET %s?%s Http/1.0\n\n", CGI_0948, trn);

	http_conn(SERVER_0948, sendform);
	return 0;
}

static int
bbc0949()
{
	char PASSWD[8], TRAN_MSG[17 * 3], username[16], Alias[sizeof username * 3];
	char trn[512], sendform[512];
	int year = 99, month = 2, day = 26, hour = 12, min = 30;

	url_encode(Alias, username);
	url_encode(TRAN_MSG, MyMsg);

	sprintf(trn, "func=Numpg&txPagerNo=%s&username=%s&txPassword=%s&txContent=%s&tran_type=on&txYear=19%02d&txMonth=%02d&txDay=%02d&txHour=%02d&txMinute=%02d",
			Pager_No, Alias, PASSWD, TRAN_MSG, year, month, day, hour, min);

	sprintf(sendform, "POST %s HTTP/1.0\nReferer: %s\n%sContent-length:%d\n\n%s",
			CGI_0949, REFER_0949, PARA, strlen(trn), trn);

	http_conn(SERVER_0949, sendform);
	return 0;
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
EMailPager(p, author, title)
char *p, *author, *title;
{
	struct Map *map;

	for (map = bbc_map; map->prefix; map ++)
		if (!memcmp(p, map->prefix, 4))
			break;

	if (!map->func)	/* 不支援此廠商 BBCall */
		return 0;

	str_ncpy(Pager_No, p + 4, sizeof(Pager_No));
	sprintf(MyMsg, "%s[%s]", title, author);

	(*map->func)();

	return 0;
}


#include<stdarg.h>
int vaEMailPager(va_list pvar)
{
	char *pno, *author, *title;

	pno = va_arg(pvar, char *);
	author = va_arg(pvar, char *);
	title = va_arg(pvar, char *);

	return EMailPager(pno, author, title);
}
