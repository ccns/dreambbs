/*-------------------------------------------------------*/
/* icq.c    (YZU WindTopBBS Ver 3.02 )                   */
/*-------------------------------------------------------*/
/* author : Verit.bbs@bbs.yzu.edu.tw                     */
/* target : ICQ�ǰT                                      */
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
		vmsg("�L�k�P���A�����o�s���A�d�ߥ���");
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
			vmsg("�ǰe���� ~~");
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
				vmsg("�ǰe���� ~ �Э��s�ǰe�@�� ~~");
				return 0;
			}
			else if (cc == 'Y' || cc == 'u' || cc == 'o')
			{
				close(sockfd);
				fclose(fp);
				vmsg("�ǰe���\\ ~ ^^");
				return 1;
			}
		}
	}
	close(sockfd);
	fclose(fp);
	vmsg("�ǰe���� ~~");
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
	outs("\033[1;37;44m�� ICQ�ǰT�t�� ��\033[m");

	move(2, 0);
	prints("\033[37;1m");
	prints("     (1) ICQ �O\033[32m Mirabilis���q\033[37m�]�p�o�檺�u�W�I�s�n��A�ϱo�b�����W��\n");
	prints("         �e�����N��M�B�̥ͭH�h�ؤ覡�����V���pô�A�]�� ICQ�֦��s�j\n");
	prints("         ���ϥθs�A�Ӥ��q���F���_�[�j ICQ���\\�ध�~�A�]�]�p�F�@�Ǥ�\n");
	prints("         �ε{���A���S�� ICQ�����ͤ]�ઽ���z�L�����N�T���ǻ��� ICQ��\n");
	prints("         �ϥΪ̡C\n\n");
	prints("     (2) ��A�̷ӤU���榡��g��A���t�αN�|�N�A���ǰe�T���A�ǰe��A\n");
	prints("         �ҿ�J�� ICQ���X�C�p�G���å��b�u�W�A�������A���s�u��\n");
	prints("         �B�ҥ� ICQ �ɡA�N�|�ܧ֪��N����o�ӰT���C\n\n");
	prints("     (3) �����t�ζȨ�\033[32m�ӤH�ϥ�\033[37m�A�Y��T��j�q���ϥΥ��t�ζǻ����g���\n");
	prints("         ���\\�γy�����x�Z���T���A�礣�i�H�Ϩ㦳�k�߮ĤO��\033[33m���w����\033[37m\n");
	prints("         �Ҵ��Τ��ƶ��A��z�ϥΥ��t�Ϋ�A�Y��ܱz�P�N��u���w�ƶ��C\n");
	prints("         (\033[31;1m���w���ڰѨ� http://www.mirabilis.com/legal-main.html \033[37m)  \n");
	prints("                              \033[30mDesigned By verit.bbs@bbs.yzu.edu.tw\033[37m\n");
	prints("  -----------------------------------------------------------------------------\n");

	strcpy(from, cuser.userid);
	sprintf(fromemail, "%s.bbs@bbs.yzu.edu.tw", cuser.userid);
	strcpy(send, "�ǰe�T��");
	strcpy(subject, "WindTop�H�t");
#if 0
	strcpy(icq_num, "11582546");
	strcpy(subject, "test");
	strcpy(body, "hihi");
#else
	do
	{
		error = 0 ;
		if (!vget(18, 5, "�п�J���ICQ���X : " , icq_num , sizeof(icq_num) , DOECHO))
			return 0;
		for (i = 0;i < strlen(icq_num);i++)
			if (icq_num[i] < '0' || icq_num[i] > '9')
			{
				error = 1;
				break;
			}
		if (error)
			vmsg("��J���~ ~ �ЦA��J�@�� ~~ ");
		else
			break;
	}
	while (1);
//  move(19,5);prints("\033[1;37m�п�J�ǰe�����e  :");
	for (i = 0;i < 3;i++)
	{
		if (!vget(19 + i, 5, i == 0 ? "\033[1;37m�п�J�ǰe�����e  : " : "                    ", tmp , sizeof(tmp), DOECHO))
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
	mouts(23, 0, "���b�s�����A���A�еy��.............");
	http_conn(SERVER_icq, sendform);

	return 0;
}
