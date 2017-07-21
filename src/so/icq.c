/*-------------------------------------------------------*/
/* icq.c    (YZU WindTopBBS Ver 3.02 )                   */
/*-------------------------------------------------------*/
/* author : Verit.bbs@bbs.yzu.edu.tw                     */
/* target : ICQ傳訊                                      */
/* create : 01/07/17                                     */
/*-------------------------------------------------------*/

#include "bbs.h"

#define mouts(y,x,s)    { move(y,x); outs(s); }

#define SERVER_icq     "wwp.icq.com"
#define HTTP_PORT	80

#define PARA "\
	POST http://wwp.icq.com/scripts/WWPMsg.dll HTTP/1.0\r\n\
	User-Agent: Mozilla/4.0 (compatible; MSIE 5.01; Windows NT 5.0)\r\n\
	Accept-Language: zh-tw\r\n\
	Content-Type: application/x-www-form-urlencoded\r\n\
	Accept-Encoding: gzip, deflate\r\n\
	Host: wwp.icq.com:80\r\n\
	Content-Length: "

static int
http_conn(char *server, char *s)
{
	int sockfd, start_show;
	int cc, tlen;
	char *xhead, *xtail, tag[80];
	static char pool[2048];
	FILE *fp;

	fd_set rd;
	struct timeval to;
	fp = fopen("etc/get", "w");

	if ((sockfd = dns_open(server, HTTP_PORT)) < 0)
	{
		vmsg("無法與伺服器取得連結，查詢失敗");
		return 0;
	}

	FD_ZERO(&rd);
	FD_SET(sockfd, &rd);
	to.tv_sec = 20;
	to.tv_usec = 0;
	pool[0] = 0;

	write(sockfd, s, strlen(s));
	shutdown(sockfd, 1);

	/* parser return message from web server */
	xhead = pool;
	xtail = pool;
	tlen = 0;
	start_show = 0;

	for (;;)
	{
		if (xhead >= xtail)
		{
			xhead = pool;
			if (select(sockfd + 1, &rd, NULL, NULL, &to) <= 0)
			{
				printf("DEBUG: GET ARTICLE %s TIME OUT !!\n", index);
				free(pool);
				close(sockfd);
				return 0;
			}
			cc = read(sockfd, xhead, sizeof(pool));
			if (cc <= 0)
				break;
			xtail = xhead + cc;
		}

		cc = *xhead++;
		fputc(cc, fp);

		if ((tlen == 16) && (!str_ncmp(tag, "!-- Document --", 15)))
		{
			start_show = 1;
		}

		if (((tlen == 6) && (!str_ncmp(tag, "/html", 5))))
		{
			close(sockfd);
			fclose(fp);
			vmsg("傳送失敗 ~~");
			return 0;
		}

		if (cc == '<')
		{
			tlen = 1;
			continue;
		}

		if (tlen)
		{
			if (cc == '>')
			{
				tlen = 0;
				continue;
			}
			if (tlen <= 75)
			{
				tag[tlen - 1] = cc;
			}
			tlen++;
			continue;
		}
		if (start_show)
		{
			if (cc == 'A')
			{
				close(sockfd);
				fclose(fp);
				vmsg("傳送失敗 ~ 請重新傳送一次 ~~");
				return 0;
			}
			else if (cc == 'Y' || cc == 'u' || cc == 'o')
			{
				close(sockfd);
				fclose(fp);
				vmsg("傳送成功\ ~ ^^");
				return 1;
			}
		}
	}
	close(sockfd);
	fclose(fp);
	vmsg("傳送失敗 ~~");
	return 0;
}

int
main_icq()
{
	char buf[512], sendform[512], tmp[50];
	char icq_num[13], from[13], fromemail[30], subject[30], body[96], send[10];
	char en_subject[64], en_body[256], en_send[30];
	int i, error;

	clear();
	move(0, 28);
	outs("\033[1;37;44m◎ ICQ傳訊系統 ◎\033[m");

	move(2, 0);
	prints("\033[37;1m");
	prints("     (1) ICQ 是\033[32m Mirabilis公司\033[37m設計發行的線上呼叫軟體，使得在網路上很\n");
	prints("         容易的就能和朋友們以多種方式做雙向的聯繫，因而 ICQ擁有廣大\n");
	prints("         的使用群，該公司除了不斷加強 ICQ的功\能之外，也設計了一些公\n");
	prints("         用程式，讓沒有 ICQ的網友也能直接透過網頁將訊息傳遞給 ICQ的\n");
	prints("         使用者。\n\n");
	prints("     (2) 當你依照下面格式填寫後，本系統將會將你欲傳送訊息，傳送到你\n");
	prints("         所輸入的 ICQ號碼。如果對方並未在線上，那麼當對方再次連線並\n");
	prints("         且啟用 ICQ 時，將會很快的就收到這個訊息。\n\n");
	prints("     (3) 本項系統僅供\033[32m個人使用\033[37m，嚴格禁止大量的使用本系統傳遞未經對方\n");
	prints("         允許\或造成對方困擾的訊息，亦不可違反具有法律效力的\033[33m約定條款\033[37m\n");
	prints("         所提及之事項，當您使用本系統後，即表示您同意遵守約定事項。\n");
	prints("         (\033[31;1m約定條款參見 http://www.mirabilis.com/legal-main.html \033[37m)  \n");
	prints("                              \033[30mDesigned By verit.bbs@bbs.yzu.edu.tw\033[37m\n");
	prints("  -----------------------------------------------------------------------------\n");

	strcpy(from, cuser.userid);
	sprintf(fromemail, "%s.bbs@bbs.yzu.edu.tw", cuser.userid);
	strcpy(send, "傳送訊息");
	strcpy(subject, "WindTop信差");
#if 0
	strcpy(icq_num, "11582546");
	strcpy(subject, "test");
	strcpy(body, "hihi");
#else
	do
	{
		error = 0 ;
		if (!vget(18, 5, "請輸入對方ICQ號碼 : " , icq_num , sizeof(icq_num) , DOECHO))
			return 0;
		for (i = 0;i < strlen(icq_num);i++)
			if (icq_num[i] < '0' || icq_num[i] > '9')
			{
				error = 1;
				break;
			}
		if (error)
			vmsg("輸入錯誤 ~ 請再輸入一次 ~~ ");
		else
			break;
	}
	while (1);
//  move(19,5);prints("\033[1;37m請輸入傳送的內容  :");
	for (i = 0;i < 3;i++)
	{
		if (!vget(19 + i, 5, i == 0 ? "\033[1;37m請輸入傳送的內容  : " : "                    ", tmp , sizeof(tmp), DOECHO))
			break;
		if (i == 0)
			strcpy(body, tmp);
		else
		{
			strcat(body, "\n");
			strcat(body, tmp);
		}
	}
#endif
	url_encode(en_subject, subject);
	url_encode(en_body, body);
	url_encode(en_send, send);

	sprintf(buf, "to=%s&from=%s&fromemail=%s&subject=%s&body=%s&Send=%s",
			icq_num, from, fromemail, en_subject, en_body, en_send);
	sprintf(sendform, "%s%d\r\n\r\n%s\r\n\r\n", PARA, strlen(buf), buf);
	mouts(23, 0, "正在連接伺服器，請稍後.............");
	http_conn(SERVER_icq, sendform);

	return 0;
}
