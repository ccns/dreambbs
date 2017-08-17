/*-------------------------------------------------------*/
/* dreye.c    (YZU WindTopBBS Ver 3.02 )                 */
/*-------------------------------------------------------*/
/* author : statue.bbs@bbs.yzu.edu.tw			 */
/* target : DreyeÄ¶¨å³q½u¤W¦r¨å                          */
/* create : 01/07/09                                     */
/*-------------------------------------------------------*/
/*
¬d¸ß¤º®e¡G
http://www.dreye.com/tw/dict/dicmain.phtml?account=123456
µL·N¸q¸ê®Æ¡G
id=42651&account=123456&ver=big5&show=0&repeat=1
·N¸q¡G
w=hello&d=010300
ÅÜ¤Æ§Î¡G
w=hello&d=010301
¦P¸q¦r/¤Ï¸q¦r¡G
w=hello&d=010304
*/
#include "bbs.h"

#define mouts(y,x,s)    { move(y,x); outs(s); }

#define HTTP_PORT       80
#define SERVER_dreye	"www.dreye.com"
#define	CGI_dreye	"/tw/dict/dict.phtml"
#define	REF		"http://www.dreye.com"

#define PROXY		"proxy3.yzu.edu.tw"
#define	PROXY_PORT	8080

static char dreye_prev[30];
static char dreye_next[30];
static char dreye_curr[30];

static int
http_conn(char *server, char *s)
{
	int sockfd, start_show;
	int cc, tlen;
	char *xhead, *xtail, tag[10], fname[50];
	static char pool[2048];
	FILE *fp;
	int nptlen = 0, npstat = 0, npset = 0; /* for next and prev */
	char nptag[30]; /* for next and prev */

	if ((sockfd = dns_open(PROXY, PROXY_PORT)) < 0)
	{
		vmsg("µLªk»P¦øªA¾¹¨ú±o³sµ²¡A¬d¸ß¥¢±Ñ");
		return 0;
	}
	else
	{
		mouts(22, 0, "¥¿¦b³s±µ¦øªA¾¹¡A½Ðµy«á(«ö¥ô·NÁäÂ÷¶}).............");
		refresh();
	}

	write(sockfd, s, strlen(s));
	shutdown(sockfd, 1);

	/* parser return message from web server */
	xhead = pool;
	xtail = pool;
	tlen = 0;
	start_show = 0;

	*dreye_next = 0;
	*dreye_prev = 0;

	/* usr_fpath(fname, cuser.userid,Msg_File); */
	sprintf(fname, "tmp/%s.dreye", cuser.userid);

	fp = fopen(fname, "w");
	fputs("                        [1;44;37m  Dr.eye [1;31mi[37mDictionary ½u¤W¦r¨å  [m\n", fp);
	fputs("          ¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w\n", fp);
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

		if ((tlen == 7) && (!str_ncmp(tag, "/table", 6)))
		{
			start_show = 1;
		}
		/* ¤£¦L buttom ªº Dr.eye */
		if ((tlen == 3) && (!str_ncmp(tag, "hr", 2)))
			start_show = 0;
		/* ¯S®í´«¦æ¥Î, ¬üÆ[ */
		if ((tlen == 3) && (!str_ncmp(tag, "td", 2)))
			fputc(' ', fp);
		if ((tlen == 4) && (!str_ncmp(tag, "div", 3)))
			fputc('\n', fp);
		if (cc == '<')
		{
			tlen = 1;
			continue;
		}
		if (tlen)
		{
			/* support<br>and<P>and</P>, diable <br> for beauty :p */
			if (cc == '>')
			{
				if ((tlen == 2) && (!str_ncmp(tag, "P", 1)))
				{
					fputc('\n', fp);
				}
				else if ((tlen == 3) && (!str_ncmp(tag, "br", 2)))
				{
					fputc('\n', fp);
				}
				tlen = 0;
				continue;
			}
			if (tlen <= 6)
			{
				tag[tlen - 1] = cc;
			}
			tlen++;

			/* ?w=hello for prev & next*/
			if (npset != 2)
			{
				if (cc == '?')
				{
					nptlen = 0;
					npstat = 1;
					continue;
				}
				else if (npstat == 1 && cc == 'w')
				{
					npstat = 2;
				}
				else if (npstat == 2 && cc == '=')
				{
					npstat = 3;
				}
				else if (npstat == 3 && cc != '&' && nptlen <= 30)
				{
					if (cc == '"')
					{
						nptlen = 0;
						npstat = 0;
					}
					nptag[nptlen] = cc;
					nptlen ++;
				}
				else if (npstat == 3 && cc == '&')
				{
					nptag[nptlen] = '\0';
					if (npset == 0)
					{
						strcpy(dreye_prev, nptag);
						npset = 1;
					}
					else if (npset == 1)
					{
						strcpy(dreye_next, nptag);
						npset = 2;
					}
					npstat = 0;
					nptlen = 0;
				}
				else
				{
					npstat = 0;
					nptlen = 0;
				}
			}
			/* ?w=hello */

			continue;
		}
		if (start_show)
		{
			if (cc != '\r' && cc != '\n')
				fputc(cc, fp);
		}
	}
	close(sockfd);
	fputs("\n\n          ¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w\n", fp);
	fputs("                          [1;36mDr.eye[37m iDictionary ½u¤W¦r¨å[m\n", fp);
	fclose(fp);

	more(fname, NULL);
	unlink(fname);

	return 0;
}

static int
dreye(char *word, char *ans)
{
	char atrn[256], sendform[512];
	char ue_word[90];
	int d;

	url_encode(ue_word, word);

	if (ans[0] == '3')
		d = 10304;
	else if (ans[0] == '2')
		d = 10301;
	else
		d = 10300;
	sprintf(atrn, "w=%s&d=%d&account=123456", ue_word, d);

	sprintf(sendform, "GET %s%s?%s HTTP/1.0\n\n", REF, CGI_dreye, atrn);

	http_conn(SERVER_dreye, sendform);
	return 0;
}

int
main_dreye()
{
	char ans[2];
	char word[30];

	ans[0] = '1';
	do
	{
		clear();
		mouts(0, 13, "\033[1;37;44m¡· Dr.eye Ä¶¨å³q½u¤W¦r¨å(http://www.dreye.com/) v0.3 ¡·\033[m");
		mouts(2, 0, "\n\
			  [0m[33;1m¢z¢w¢w¢w¢w¢¡  [0m              [32;1m¢z¢w¢w¢w¢w¢{\n\
			  [0m[33;1m¢x¢i¢«¢ª¢©¢x¢z¢w¢~¢w¢w¢¡ [0m   [32;1m¢x¢i¢«¢ª¢i¢x¢z¢w¢{¢z¢w¢{¢~¢w¢w¢w¢w¢¡\n\
			  [0m[33;1m¢x¢i¢z¢¡¢i¢x¢x¢i¢¨¢i¢©¢x [0m   [32;1m¢x¢i¢©¢{¢w¢}¢x¢i¢¢¢£¢i¢x¢x¢¨¢«¢ª¢©¢x\n\
			  [0m[33;1m¢x¢i¢|¢£¢i¢x¢x¢i¢«¢¡¢i¢x [0m   [32;1m¢x¢i¢«¢}¢w¢{¢x¢i¢©¢¨¢i¢x¢x¢i¢©¢¨¢«¢£\n\
			  [0m[33;1m¢x¢i¢©¢¨¢«¢x¢x¢i¢x¢|¢w¢}[37m¢~¢¡[32m¢x¢i¢©¢¨¢i¢x¢¢¢¡¢ª¢«¢i¢x¢x¢ª¢©¢¨¢«¢x\n\
			  [0m[33;1m¢|¢w¢w¢w¢w¢£¢|¢w¢}      [37m¢¢¢£[32m¢|¢w¢w¢w¢w¢}¢x¢ª¢©¢¨¢«¢x¢¢¢w¢w¢w¢w¢£\n\
			  [0m                                        [32;1m¢¢¢w¢w¢w¢w¢£[0m\n\n");
		outs("§@ªÌ: Shen Chuan-Hsing <statue.bbs@bbs.yzu.edu.tw>\n\n");
		outs("«e¦¸¬d¸ß¡G");
		outs(dreye_curr);
		outs("\nprev¡G");
		outs(dreye_prev);
		outs("\nnext¡G");
		outs(dreye_next);
		outs("\n");
		if (!vget(18, 0, "Search¡G", word, 30, LCECHO))
			return 0;
		strcpy(dreye_curr, word);
		vget(19, 0, "1)·N¸q 2)ÅÜ¤Æ§Î 3)¦P¸q¦r/¤Ï¸q¦r q)Â÷¶} [1] ", ans, 3, LCECHO);
		if (ans[0] != 'q')
		{
			dreye(word, ans);
			logitfile(FN_PYDICT_LOG, "DREYE", word);
		}
	}
	while (ans[0] != 'q');
	return 0;
}
