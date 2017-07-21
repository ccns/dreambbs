/*-------------------------------------------------------*/
/* star.c    (YZU WindTopBBS Ver 3.02 )                  */
/*-------------------------------------------------------*/
/* author : verit.bbs@bbs.yzu.edu.tw			 */
/* target : http://mindcity.sina.com.tw			 */
/* create : 01/07/09                                     */
/* modify : 03/07/12					 */
/*-------------------------------------------------------*/

#include "bbs.h"

#define mouts(y,x,s)    { move(y,x); outs(s); }

#define HTTP_PORT       (8080)
#define SERVER_star	"proxy1.yzu.edu.tw"
#define LINE_WORD	50

char *tag_s[4] = {"!--START:HORO_TODAY--", "!--START:HORO_TOMORROW--"};
char *tag_e[4] = {"!--END:HORO_TODAY--", "!--END:HORO_TOMORROW--"};

char *star[12] = {"aries", "taurus", "gemini", "cancer", "leo", "virgo",
				  "libra", "scorpio", "sagittarius", "capricorn", "aquarius", "pisces"
				 };

char *draw_star[20] = {"�~�w��", "�x  �x", "���w��", "�x  �x", "���w��",
					   "�~�s��", "�x�x�x", "�x�x�x", "  �x  ", "  �x  ",
					   "�~�w��", "�x  �x", "�u�w�t", "�x  �x", "�x  �x",
					   "�~�w��", "�x  �x", "�u�w��", "�x �@ ", "�x  �x"
					  };

static int
http_conn(char *server, char *s, int kind)
{
	int sockfd, start_show, chinese = 0, state = 0;
	int cc, tlen, word = 0, show = 0;
	char *xhead, *xtail, tag[128], fname[50];
	static char pool[2048];
	FILE *fp;

	mouts(23, 0, "���b�s�����A���A�еy��..........");
	if ((sockfd = dns_open(server, HTTP_PORT)) < 0)
	{
		vmsg("�L�k�P���A�����o�s���A�d�ߥ���");
		return 0;
	}
	else
	{
		refresh();
	}

	write(sockfd, s, strlen(s));
	shutdown(sockfd, 1);

	/* parser return message from web server */
	xhead = pool;
	xtail = pool;
	tlen = 0;
	start_show = 0;
	sprintf(fname, "tmp/%s.star", cuser.userid);

	clear();
	fp = fopen(fname, "w");
	fprintf(fp, "\n   %s", draw_star[show++]);
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

		if ((tlen == strlen(tag_s[kind]) + 1) && (!str_ncmp(tag, tag_s[kind], strlen(tag_s[kind]))))
			start_show = 1;
		if ((tlen == strlen(tag_e[kind]) + 1) && (!str_ncmp(tag, tag_e[kind], strlen(tag_e[kind]))))
			break;

		if (cc == '<' || cc == '&')
		{
			tlen = 1;
			continue;
		}
		if (tlen)
		{
			if (cc == '>' || cc == ';')
			{
				if ((tlen == 3) && (!str_ncmp(tag, "br", 2)) && start_show == 1 && state == 0)
				{
					fprintf(fp, "\n   %s\t", draw_star[show++]);
					state = 1;
				}
				else if ((tlen == 3) && (!str_ncmp(tag, "tr", 2)) && start_show == 1 && kind > 1 && state == 0)
				{
					fprintf(fp, "\n   %s\t", draw_star[show++]);
					state = 1;
				}
				tlen = 0;
				word = 0;
				continue;
			}
			if (tlen <= 128)
				tag[tlen - 1] = cc;
			tlen++;
			continue;
		}
		if (start_show)
		{
			if (word > LINE_WORD && cc < 0 && chinese == 0 && state == 0)
			{
				fprintf(fp, "\n   %s\t", draw_star[show++]);
				state = 1;
				word = 0 ;
			}
			if (cc == ' ')
				chinese = 0;
			else if (cc != '\r' && cc != '\n')
			{
				word++;
				fputc(cc, fp);
				state = 0 ;
				chinese = (cc < 0) ? ((chinese == 1) ? 0 : 1) : 0;
			}
			else
			{
				word = 0;
			}
		}
	}
	close(sockfd);
	fprintf(fp, "\n   %s\t��ƨӷ����s���P�y��R��(http://mindcity.sina.com.tw/)",
			(show > 19) ? "" : draw_star[show]);
	fclose(fp);

	more(fname, NULL);
	unlink(fname);
	return 0;
}

int
main_star()
{
	char buf[128], c1, c2;
	char *_strades[14] = {"AZ",
						  "AAries        �d�Ϯy",  "BTaurus       �����y",  "CGemini       ���l�y",  "DCancer       ���ɮy",
						  "ELeo          ��l�y",  "FVirgo        �B�k�y",  "GLibra        �ѯ��y",  "HScorpio      ���Ȯy",
						  "ISagittarius  �g��y",  "JCapricorn    �]�~�y",  "KAquarius     ���~�y",  "LPisces       �����y", NULL
						 };
	char *_kind[4] = {"AZ", "AToday      ����B��", "BTomorrow   ����B��", NULL};

	if ((c1 = popupmenu_ans(_strades, "��ܬP�y", 5, 18)) != 'z')
	{
		if ((c2 = popupmenu_ans(_kind, "�������", 8, 18)) != 'z')
		{
			sprintf(buf, "GET http://mindcity.sina.com.tw/west/MC-12stars/%s%d.html HTTP/1.0\n\n", star[c1-'a'], (c2 - 'a' + 1));
			http_conn(SERVER_star, buf, c2 - 'a');
		}
	}
	return 0;
}

