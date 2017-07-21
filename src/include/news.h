/*-------------------------------------------------------*/
/* news.h    (YZU WindTopBBS Ver 3.02 )                  */
/*-------------------------------------------------------*/
/* author : Verit.bbs@bbs.yzu.edu.tw			 */
/* modify : visor.bbs@bbs.yzu.edu.tw			 */
/* target : Kimo News                                    */
/* create : 01/07/12                                     */
/* update : 01/07/13					 */
/*-------------------------------------------------------*/

#ifndef	_NEWS_H_

#define	_NEWS_H_

#define PARA "\
Connection: Keep-Alive\r\n\
User-Agent: Lynx/2.6  libwww-FM/2.14\r\n\
Content-type: application/x-www-form-urlencoded\r\n\
Accept: text/html, text/plain, application/x-wais-source, application/html, */*\r\n\
Accept-Encoding: gzip\r\n\
Accept-Language: zh-tw\r\n\
Accept-Charset: iso-8859-1,*,utf-8\r\n\
Host: %s:%d\r\n"


#define HTTP_PORT       80
#define LINE_WORD	55


#define	INI_GLOBAL	"[global]"
#define	INI_DEFAULT	"[class:default]"
#define	INI_CLASS	"[class:"

#define	NEWS_ROOT	"news"
#define	NEWS_ETC_ROOT	"etc/news"
#define	NEWS_SETUP_FILE	"news.ini"
#define	NEWS_TMP_FILE	"tmp/news.tmp"
#define	PAGE		4096


typedef	struct PARSER
{
  char name[32];
  
  char l_start[128];
  char l_end[128];
  char l_hrefs[128];
  char l_hrefe[128];
  char l_ts[128];
  char l_te[128];
  char l_show[128];
  char l_unshow[128];
  char l_new[128];
  char l_art[128];
  
  char a_s[128];
  char a_e[128];
  char a_show[128];
  char a_unshow[128];
  
  struct PARSER *next;
}	PARSER;

typedef	struct 
{			     
  char site[32];
  char name[32];
  char href[128];
  char server[32];
  int port;
  char proxy[32];
  int proxyport;
  char class[50][32];
  int class_len;
  char index[32];
  char apath[128];
  char path[128];
  char doc[128];
  PARSER *parser;
  PARSER *def;
}	NEWS;

#endif

