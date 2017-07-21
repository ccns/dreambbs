/*-------------------------------------------------------*/
/* railway.c    ( NTHU CS MapleBBS Ver 3.10 )            */
/*-------------------------------------------------------*/
/* author : kepler.bbs@bbs.cs.nthu.edu.tw                */
/* modify : statue.bbs@bbs.yzu.edu.tw			 */
/* target : �x�K�����ɨ��d��                           */
/* create : 99/03/30                                     */
/* update : 01/01/30                                     */
/*-------------------------------------------------------*/

#include "bbs.h"

#define mouts(y,x,s)    { move(y,x); outs(s); }

// #define SERVER_railway     "www.railway.gov.tw"
//#define SERVER_railway     "db.twtraffic.com"
#define SERVER_railway	"passenger.tra.gov.tw"
#define HTTP_PORT	80

//#define CGI_railway     "/cgi-bin/taimain.fpl"
//#define	HTTP_REF		"http://www.twtraffic.com.tw"
#define CGI_railway	"/ap/"
#define HTTP_REF	"http://passenger.tra.gov.tw"

#define PROXY           "proxy3.yzu.edu.tw"
#define PROXY_PORT      8080

#define REFER_railway_1      "http://db.twtraffic.com/train/tai1w.html"
#define REFER_railway_2      "http://db.twtraffic.com/train/tai2e.html"
#define REFER_railway_3      "http://db.twtraffic.com/train/tai3s.html"
#define REFER_railway_4      "http://db.twtraffic.com/train/tai4c.html"
#define REFER_railway_5      "http://db.twtraffic.com/train/tai5i.html"
#define REFER_railway_6      "http://db.twtraffic.com/train/tai6g.html"


#define PARA "\
Connection: Keep-Alive\r\n\
User-Agent: Lynx/2.6  libwww-FM/2.14\r\n\
Content-type: application/x-www-form-urlencoded\r\n\
Accept: text/html, text/plain, application/x-wais-source, application/html, */*\r\n\
Accept-Encoding: gzip\r\n\
Accept-Language: zh-tw\r\n\
Accept-Charset: iso-8859-1,*,utf-8\r\n\
Host: db.twtraffic.com:80\r\n"

static void
out_song()
{
#if 0
  static unsigned char song[] = "�����j�������Y���V���V���V�A�Q�[�����"
                                "�T���j�d�������z�K�z�K�z�K�A�Q�Q����"
                                "�J�����u���j����N�V�N�V�N�A�Q�������"
                                "�i�O���Y�O���j�O�d�եd�եd�A�Q������O"
                                "�������˴����q�K�ʩK�ʩK�ʧA�Q�������" ;
#endif
#if 0
    ����  �N��  ��: Michael  ��: Tori Amos
#endif
  static unsigned char song[] = 
    "�H�q�������J��n�I�l  ��Ĳ�N�Ů��a�F����"
    "���A���F�ݸ�  ���˷R���A�k�h  ���F�ۤv"
    "�q���Q���Ԥ߬ݬ�A  ť���A�M�ڳ̦n�B�ͦb�@�_"
    "�ڤ]�ǧO�H�R�R�a�ݧA�Ǻt��  �]���@����"
    "�۹��q�q�L�y  �n���A�ڦ����F�q��  �ä��n���X�a�q��"
    "����  �������N�N�a�����N���~��"
    "�h�֦~  �R�A�]��A  �h�֦~  �۷q�p�B"
    "�y��  ���@��b���䭸�ӭ��h  �q��ť���ݤ��Q�h��"
    "�ȷ|�L�O�s´  �a�[�M�Ѫ����Ҿ�  ��򵹧A"
    "���h�ߤ��ݧA�o���o���z  ���j�D�����Q���d�����"
    "�û��M�ߦa�I��a�M�A�t��  �����@���@�N"
    "�ڷR�A�A�]�R�ڳo�ӿը��{�b�A��o�ӿ�"
    "�̿�L�ŵ�L�˹L�h�L�n�L�ڿ鱼�F�A�W���G"
    "�N�R�A�N�R�A�N�R  �N���N��  �R��������"
    "���ۧA���q�A�a�e�A  �i�|�ϧA�^�Y"
    "�۹��q�q�L�y  �n���A�ڦ����F�q��  �ä��|���X�a�q��"
    "���N�N�����N�N  �S�����y�S�����y�S�����y  �N�M�M"
    "�h�֦~  �۹�������  �s���h�֦~ �i���S�i��  �i���S�i��"
    "���A���L���e";

  static unsigned char *p = song;
  outc(*p++);
  outc(*p++);
  if(!*p) p = song;
}

static int
data_ready(int sockfd)
{
  static struct timeval tv = {1, 100};
  /* Thor.980806: man page ���] timeval struct�O�|���ܪ� */ 
 
  int cc;
  fd_set rset; 
 
  for(;;)
  {
    struct timeval tmp_tv = tv;
    FD_ZERO(&rset); 
    FD_SET(0, &rset);
    FD_SET(sockfd, &rset);
    cc = select(sockfd + 1, &rset, NULL, NULL, &tmp_tv);
    if(cc > 0)
    {
      if(FD_ISSET(0, &rset))
        return 0; /* Thor.990401: user break */
      if(FD_ISSET(sockfd, &rset))
        return 1; /* Thor.990401: data ready */
    }
    else if(cc == 0) /* Thor.990401: time out */
    {
      cursor_save();
      out_song();
      refresh();
      cursor_restore();
    }
  }
}

static int
http_conn(char *server, char *s)
{
  int sockfd, start_show;
  int cc, tlen;
  char *xhead, *xtail, tag[15], fname[50];
  static char pool[2048];
  FILE *fp;
  int space; // remove unuse space

  if ((sockfd = dns_open(PROXY, PROXY_PORT)) < 0)
  {
    vmsg("�L�k�P���A�����o�s���A�d�ߥ���");
    return 0;
  }
  else
  {
    /* mouts(22, 0, "\033[1;36m���b�s�����A���A�еy��(�����N�����}).............\033[m"); */
    mouts(22, 0, "���b�s�����A���A�еy��(�����N�����}).............");
    refresh();
  }

  write(sockfd, s, strlen(s));
  shutdown(sockfd, 1);

  /* parser return message from web server */
  xhead = pool;
  xtail = pool;
  tlen = 0;
  start_show = 0;

  /* usr_fpath(fname, cuser.userid,Msg_File); */
  sprintf(fname, "tmp/%s.railway", cuser.userid);

  fp = fopen(fname, "w");
  for (;;)
  {
    if (xhead >= xtail)
    {
      xhead = pool;
#if 1
      /* Thor.990401: �L��@�U */
      if(!data_ready(sockfd))
      {
        close(sockfd);
        fclose(fp);
        unlink(fname);
        return 0;
      }
#endif
      cc = read(sockfd, xhead, sizeof(pool));
      if (cc <= 0)
	break;
      xtail = xhead + cc;
    }

    cc = *xhead++;

    if ((tlen == 6) && (!str_ncmp(tag, "table", 5)))
    {
      start_show = 1;
      /* continue; */
    }

    if (cc == '[')
      start_show = 0;

    if ((tlen == 3) && (!str_ncmp(tag, "td", 2)))
      fputc(' ', fp);

    if (cc == '<')
    {
      tlen = 1;
      continue;
    }

    if (tlen)
    {
      /* support<br>and<P>and</P> */

      if (cc == '>')
      {
	if ((tlen == 3) && (!str_ncmp(tag, "tr", 2)))
	{
	  fputc('\n', fp);
	}
	else if ((tlen == 2) && (!str_ncmp(tag, "P", 1)))
	{
	  fputc('\n', fp);
	}
/*	else if ((tlen == 3) && (!str_ncmp(tag, "br", 2)))
	{
	  fputc('\n', fp);
	}*/
	else if ((tlen == 13) && (!str_ncmp(tag, "table border", 12)))
	{
	  fputc('\n', fp);
	}

	tlen = 0;
	continue;
      }

      if (tlen <= 13)
      {
	tag[tlen - 1] = cc;
      }

      tlen++;
      continue;
    }
    if (start_show)
    {
      if (cc == ' ') {
        if(space == 0) {
          fputc(cc, fp);
          space = 1;
        }
      } else if (cc != '\r' && cc != '\n') {
	fputc(cc, fp);
	space = 0;
      }
    }
  }

  close(sockfd);
  fputc('\n', fp);
  fclose(fp);

  /* show message that return from the web server */

  /* usr_fpath(fname, cuser.userid,Msg_File); */
  more(fname, NULL);
  unlink(fname);

  return 0;
}

static int
railway(char *REF)
{
  char from_station[10], to_station[10];
  char atrn[256], ans[2], sendform[512];
  char Ft[30], Tt[30], type[8], tt[8];
  int from_time, to_time;

  /* mouts(7,0,"�ҵ{��-->         ��F��-->"); */
  if(!vget(17, 0, "�ҵ{��-->", from_station, 9, DOECHO)
   ||!vget(17, 19, "��F��-->", to_station, 9, DOECHO))
  {
    vmsg("hmm...���Q�d�o....^o^");
    return 0;
  }

  if(!vget(18, 0, "�d�߮ɶ��G  ", atrn, 3, LCECHO))
  {
    from_time = 0;
    mouts(18, 12, "00");
  }
  else
    from_time = atoi(atrn);

  mouts(18, 14, ":00 ");

  if(!vget(18, 18, "�� ", atrn, 3, LCECHO))
  {
    to_time = 24;
    mouts(18, 21, "24");
  }
  else
    to_time = atoi(atrn);

  mouts(18, 23, ":00 ");

  if(vget(18, 28, "����  <1>�}�� <2> ��F�G[1] ", ans, 3, LCECHO) != '2')
    strcpy(tt, "D");
  else
    strcpy(tt, "A");

  if(vget(19, 0, "�d�ߨ��ءG<1>�︹�֨�  <2>���q���֡G[1] ", ans, 3, LCECHO) != '2')
    strcpy(type, "F");
  else
    strcpy(type, "S");

  if(vget(20, 0, "�T�w�n�}�l�d�ߡH[y] ", ans, 3, LCECHO) == 'n')
  {
    vmsg("hmm.....���Q�d�o...^o^");
    return 0;
  }
  else
  {
    char datebuf[100];
    struct tm *ptime;
    time_t now;
    now = time(NULL);
    ptime = localtime(&now);
    sprintf(datebuf, "%d%%2F%d%%2F%d", 
      ptime->tm_year+1900, ptime->tm_mon + 1, ptime->tm_mday);
    url_encode(Ft, from_station);
    url_encode(Tt, to_station);

    sprintf(atrn, "MIval=Train_Time&line=%s&encode=Big5&action=on&Departure_Date=%s&Station1=%s&Station2=%s&From_Time=%02d&To_Time=%02d&Time=%s&Type=%s", REF, datebuf, Ft, Tt, from_time, to_time, tt, type);

    /* Thor.990330: �[�W log */
    blog("RAILWAY",atrn);

//    sprintf(sendform, "POST %s%s HTTP/1.0 Referer: %s\n%sContent-length:%d\n\n%s", HTTP_REF, CGI_railway, REF, PARA, strlen(atrn), atrn);
    sprintf(sendform, "GET %s%s?%s HTTP/1.0\n\n", HTTP_REF, CGI_railway, atrn);

    http_conn(SERVER_railway, sendform);
    return 0;
  }
}

int 
main_railway()
{
  char ans[2];

  clear();
  move(0, 23);
  outs("\033[1;37;44m�� �x�K�����ɨ��d�ߨt�� ��\033[m\n");
  outs("�@��: Shen Chuan-Hsing <statue.bbs@bbs.yzu.edu.tw>\n");
  move(3, 0);

  outs("\n\
            �d �d      �� ��       �d �d      �d     �d\n\
      ���i�i�i�i�i�i�i�i�i�i�i�i�i�i�i�i�i�ŢŢi�i�i�i�i�i�i�i�i�i�i�i�i�i�i�i\n\
    ��  \033[30;41m��\033[31;40m�i�i�ݢi�i�i�i�i�i�i�i�i�i�ݢi�i\033[31;40m �i \033[30;41m�~�w��\033[31;40m�i    �i�i       �i�i\n\
  \033[30;47m��\033[37;41m��\033[31;40m�i�p�i�i�i�i�i�i�i�i�i�i�i�i�i�i�i�i �i \033[30;41m�x  �x\033[31;40m�i    �i�i       �i�i\n\
\033[30;41m��  \033[31;47m��\033[37;40m�i�p�i�i�i�i�i�i\033[34;47m��\033[37;40m�i�i�i�i�i�i�i�i�i�Ţ�\033[30;47m���w��\033[37;40m�i�i�i�i�i�i�i�i�i�i�i�i�i\n\
���������ݡݡ��������������������ݡݡ�����    �������ݡݡ������h�h�l__�ۡݡݡ�\n\
");


  mouts(15, 0, "1)�賡�F�u(�t�x���u)  2)�F���F�u(�t�_�j�u)  3)�n�j�u");
  mouts(16, 0, "4)���˽u  5)���W�u  6)�����u:[1] ");
  mouts(17, 0, "�ҵ{��-->          ��F��-->");
  mouts(18, 0, "�d�߮ɶ��G  00:00 �� 24:00  ����  <1>�}�� <2> ��F ");
  mouts(19, 0, "�d�ߨ��ءG<1>�︹�֨�  <2>���q���� ");
  switch(vget(16, 0, "4)���˽u  5)���W�u  6)�����u:[1] ", ans, 3, LCECHO))
  {
    case '2':
      move(1, 0);
      outs("\n\
        0    1    2    3        4    5    6        7    8    9\n\
00        �x�F ����      �x�F�s��      �s��          ���� �緽\n\
01   ��M ��� ���s          ���� ���W               �I��\n\
02   �F��      �F�� �w�q          �ɨ�          �T��      ���J\n\
03   ��_ �I�� �j�I          ���_ �U�a ��L     �n��      �ˤf\n\
04   �ץ� ���� ���M �Ӿ�          �N�w\n\
05        �Ὤ �_�H ����     �s�� �R�w �M��     �M�� �~��\n\
06   �Z��      �n�D �F�D     �ü� �ìK Ĭ�D Ĭ�D�s�� �s�� �V�s\n\
07   ù�F ���� �G�� �y��     �|�� �G�� ���H     �Y�� �~�D �t�s\n\
08   �j�� �j�� �۫� �ֶ�     �^�d ���� �d��   �T�I�� �Jֻ ���\n\
09 �|�}�F �x�x �� �K��               ����     �n�� �Q�s\n\
10   �x�_ �U�� �O�� ��L\n");
      railway("maineast");
      break;
    case '3':
      move(1, 0);
      outs("\n\
      0    1    2    3        4    5    6      7      8      9\n\
00      �x�F ����      �x�F�s��\n\
18                              ���� ��s   ���� �E���� ������\n\
19 �̪F �k�� �ﬥ ���     �˥� ��{ �r��   �n�{   ��w   �L��\n\
20 �ΥV �F��      �D�d     �[�S ���� �D�s\n\
21 �j�� �j�Z      �]��     �h�} ���[      �ӳ¨�          ����\n\
22 �d��\n\n");
      railway("mainsouth");
      break;
    case '4':
      move(1, 0);
      outs("\n\
      0    1    2    3    4    5    6      7    8    9\n\
08                                    �T�I�� �Jֻ ���\n\
23      �j�� �Q�� ��j ���} ���� �׮�\n\n\n\n\n\n");
      railway("pin");
      break;
    case '5':
      move(1, 0);
      outs("\n\
      0    1    2    3    4      5    6    7    8    9\n\
11                     �s��\n\
24      �ˤ� �W�� �˪F ��s �E�g�Y �X�� �n�e ���W\n\n\n\n\n\n");
      railway("innerbay");
      break;
    case '6':
      move(1, 0);
      outs("\n\
      0    1    2    3    4    5    6      7      8      9\n\
15                          �G��\n\
25      ���u �B�� �s�u ���� ���� ���L\n\n\n\n\n\n");
      railway("gg");
      break;
    default: /* case '1' */
      move(1, 0);
      outs("\
      0     1      2      3      4    5    6      7    8    9\n\
100      ��   �K��   �C��   ���� ���� �n��   �Q�s �x�_ �U��\n\
101      �O��   ��L   �s��   �a�q ��� ���c   ���c �H�� ����\n\
102 �I�� ��f   �s��   �˥_        �s�� ���s   �T�� �˫n\n\
110             �ͤ�          �j�s ���s �s�� �ըF�� �s�H �q�]\n\
111 �b�� ��n   �j�� �O����   �M�� �F�� �s��   �j�{ �l��\n\
130             �y��          �״I �]��        �n�� ���r\n\
131 �T�q                      ���w �Z��        �׭� ��l �x��\n\
132 �Q�� ���\\   �j�y   �ӭ�       1120:����\n\
120             ���   ���L   �ùt ���Y �Ф�   �G�� �L�� �ۺh\n\
121 �椻 ��n   ���t   �j�L   ���� �Ÿq        ���W �n�t ���\n\
122 �s�� �h�� �L����   ����   �ުL ���� �s��   �ñd �x�n �O�w\n\
123 ���w �j��   ����   ���s   ���Y ���� ����        ����\n\
140             ��s   ���� �E���� ������ �̪F");
      railway("mainwest");
      break;
  }
  return 0;
}

