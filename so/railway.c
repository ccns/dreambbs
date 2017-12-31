/*-------------------------------------------------------*/
/* railway.c    ( NTHU CS MapleBBS Ver 3.10 )            */
/*-------------------------------------------------------*/
/* author : kepler.bbs@bbs.cs.nthu.edu.tw                */
/* modify : statue.bbs@bbs.yzu.edu.tw			 */
/* target : 台鐵火車時刻表查詢                           */
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
  static unsigned char song[] = "紅豆大紅豆芋頭挫銼挫銼挫銼你想加什麼料"
                                "汽車大卡車機車叭八叭八叭八你想吊什麼車"
                                "宿網有線網蜘蛛網咚冬咚冬咚你想停什麼網"
                                "檯燈床頭燈關大燈卡啦卡啦卡你想關什麼燈"
                                "停水停瓦斯停電呱瓜呱瓜呱瓜你想停什麼哩" ;
#endif
#if 0
    王菲  冷戰  詞: Michael  曲: Tori Amos
#endif
  static unsigned char song[] = 
    "沈默之中仍克制不要呼吸  怕觸摸空氣壞了情緒"
    "為你鎖住了問號  等親愛的你吻去  欺騙自己"
    "從不想不忍心看穿你  聽說你和我最好朋友在一起"
    "我也學別人靜靜地看你倆演戲  也不願分離"
    "相對默默無語  好像你我早有了默契  永不要幼稚地猜疑"
    "但表情  彼此表情冷冷地說明冷戰繼續"
    "多少年  愛你也恨你  多少年  相敬如冰"
    "流言  風一般在身邊飛來飛去  從不聽不問不想懷疑"
    "怕會無力編織  地久和天長的證據  怎麼給你"
    "不多心不問你卻不得不理  不強求偏偏想挽留不放棄"
    "永遠專心地寂寞地和你演戲  不管願不願意"
    "我愛你你也愛我這個諾言現在你對她承諾"
    "依賴過空虛過傷過痛過好過我輸掉了你獨消瘦"
    "冷靜再冷靜再冷靜  將錯就錯  愛情怎麼算對錯"
    "陪著你順從你縱容你  可會使你回頭"
    "相對默默無語  好像你我早有了默契  永不會幼稚地猜疑"
    "但冷冷表情表情冷冷  沒有言語沒有言語沒有言語  冷清清"
    "多少年  相對難相戀  究竟多少年 可惜又可憐  可惜又可憐"
    "讓你走過眼前";

  static unsigned char *p = song;
  outc(*p++);
  outc(*p++);
  if(!*p) p = song;
}

static int
data_ready(int sockfd)
{
  static struct timeval tv = {1, 100};
  /* Thor.980806: man page 假設 timeval struct是會改變的 */ 
 
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
    vmsg("無法與伺服器取得連結，查詢失敗");
    return 0;
  }
  else
  {
    /* mouts(22, 0, "\033[1;36m正在連接伺服器，請稍後(按任意鍵離開).............\033[m"); */
    mouts(22, 0, "正在連接伺服器，請稍後(按任意鍵離開).............");
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
      /* Thor.990401: 無聊一下 */
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

  /* mouts(7,0,"啟程站-->         到達站-->"); */
  if(!vget(17, 0, "啟程站-->", from_station, 9, DOECHO)
   ||!vget(17, 19, "到達站-->", to_station, 9, DOECHO))
  {
    vmsg("hmm...不想查囉....^o^");
    return 0;
  }

  if(!vget(18, 0, "查詢時間：  ", atrn, 3, LCECHO))
  {
    from_time = 0;
    mouts(18, 12, "00");
  }
  else
    from_time = atoi(atrn);

  mouts(18, 14, ":00 ");

  if(!vget(18, 18, "至 ", atrn, 3, LCECHO))
  {
    to_time = 24;
    mouts(18, 21, "24");
  }
  else
    to_time = atoi(atrn);

  mouts(18, 23, ":00 ");

  if(vget(18, 28, "之間  <1>開車 <2> 到達：[1] ", ans, 3, LCECHO) != '2')
    strcpy(tt, "D");
  else
    strcpy(tt, "A");

  if(vget(19, 0, "查詢車種：<1>對號快車  <2>普通平快：[1] ", ans, 3, LCECHO) != '2')
    strcpy(type, "F");
  else
    strcpy(type, "S");

  if(vget(20, 0, "確定要開始查詢？[y] ", ans, 3, LCECHO) == 'n')
  {
    vmsg("hmm.....不想查囉...^o^");
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

    /* Thor.990330: 加上 log */
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
  outs("\033[1;37;44m◎ 台鐵火車時刻表查詢系統 ◎\033[m\n");
  outs("作者: Shen Chuan-Hsing <statue.bbs@bbs.yzu.edu.tw>\n");
  move(3, 0);

  outs("\n\
            ▃ ▃      ≒ ≒       ▃ ▃      ▃     ▃\n\
      ◢█████████████████〣〣████████████████\n\
    ／  \033[30;41m■\033[31;40m██≡██████████≡██\033[31;40m █ \033[30;41m╭─╮\033[31;40m█    ██       ██\n\
  \033[30;47m◤\033[37;41m◤\033[31;40m█▉████████████████ █ \033[30;41m│  │\033[31;40m█    ██       ██\n\
\033[30;41m◤  \033[31;47m◤\033[37;40m█▉██████\033[34;47m㊣\033[37;40m█████████〣〣\033[30;47m╰─╯\033[37;40m█████████████\n\
◥▓◤◎≡≡◎◥▓▓▓▓▓▓◤◎≡≡◎◥▓    ▓◤◎≡≡◎◥▓︺︺︼__∞≡≡∞\n\
");


  mouts(15, 0, "1)西部幹線(含台中線)  2)東部幹線(含北迴線)  3)南迴線");
  mouts(16, 0, "4)平溪線  5)內灣線  6)集集線:[1] ");
  mouts(17, 0, "啟程站-->          到達站-->");
  mouts(18, 0, "查詢時間：  00:00 至 24:00  之間  <1>開車 <2> 到達 ");
  mouts(19, 0, "查詢車種：<1>對號快車  <2>普通平快 ");
  switch(vget(16, 0, "4)平溪線  5)內灣線  6)集集線:[1] ", ans, 3, LCECHO))
  {
    case '2':
      move(1, 0);
      outs("\n\
        0    1    2    3        4    5    6        7    8    9\n\
00        台東 馬蘭      台東新站      山里          鹿野 瑞源\n\
01   瑞和 月美 關山          海瑞 池上               富里\n\
02   東竹      東里 安通          玉里          三民      瑞穗\n\
03   瑞北 富源 大富          光復 萬榮 鳳林     南平      溪口\n\
04   豐田 壽豐 平和 志學          吉安\n\
05        花蓮 北埔 景美     新城 崇德 和仁     和平 漢本\n\
06   武塔      南澳 東澳     永樂 永春 蘇澳 蘇澳新站 新馬 冬山\n\
07   羅東 中里 二結 宜蘭     四城 礁溪 頂埔     頭城 外澳 龜山\n\
08   大溪 大里 石城 福隆     貢寮 雙溪 牡丹   三貂嶺 侯硐 瑞芳\n\
09 四腳亭 暖暖 基隆 八堵               汐止     南港 松山\n\
10   台北 萬華 板橋 樹林\n");
      railway("maineast");
      break;
    case '3':
      move(1, 0);
      outs("\n\
      0    1    2    3        4    5    6      7      8      9\n\
00      台東 馬蘭      台東新站\n\
18                              高雄 鳳山   後庄 九曲堂 六塊厝\n\
19 屏東 歸來 麟洛 西勢     竹田 潮州 崁頂   南州   鎮安   林邊\n\
20 佳冬 東海      枋寮     加祿 內獅 枋山\n\
21 古莊 大武      瀧溪     多良 金崙      太麻里          知本\n\
22 康樂\n\n");
      railway("mainsouth");
      break;
    case '4':
      move(1, 0);
      outs("\n\
      0    1    2    3    4    5    6      7    8    9\n\
08                                    三貂嶺 侯硐 瑞芳\n\
23      大華 十分 望古 嶺腳 平溪 菁桐\n\n\n\n\n\n");
      railway("pin");
      break;
    case '5':
      move(1, 0);
      outs("\n\
      0    1    2    3    4      5    6    7    8    9\n\
11                     新竹\n\
24      竹中 上員 竹東 橫山 九讚頭 合興 南河 內灣\n\n\n\n\n\n");
      railway("innerbay");
      break;
    case '6':
      move(1, 0);
      outs("\n\
      0    1    2    3    4    5    6      7      8      9\n\
15                          二水\n\
25      源泉 濁水 龍泉 集集 水里 車埕\n\n\n\n\n\n");
      railway("gg");
      break;
    default: /* case '1' */
      move(1, 0);
      outs("\
      0     1      2      3      4    5    6      7    8    9\n\
100      基隆   八堵   七堵   五堵 汐止 南港   松山 台北 萬華\n\
101      板橋   樹林   山佳   鶯歌 桃園 內壢   中壢 埔心 楊梅\n\
102 富岡 湖口   新豐   竹北        新竹 香山   崎頂 竹南\n\
110             談文          大山 後龍 龍港 白沙屯 新埔 通霄\n\
111 苑裡 日南   大甲 臺中港   清水 沙鹿 龍井   大肚 追分\n\
130             造橋          豐富 苗栗        南勢 銅鑼\n\
131 三義                      泰安 后里        豐原 潭子 台中\n\
132 烏日 成功\   大慶   太原       1120:彰化\n\
120             花壇   員林   永靖 社頭 田中   二水 林內 石榴\n\
121 斗六 斗南   石龜   大林   民雄 嘉義        水上 南靖 後壁\n\
122 新營 柳營 林鳳營   隆田   拔林 善化 新市   永康 台南 保安\n\
123 中洲 大湖   路竹   岡山   橋頭 楠梓 左營        高雄\n\
140             鳳山   後庄 九曲堂 六塊厝 屏東");
      railway("mainwest");
      break;
  }
  return 0;
}

