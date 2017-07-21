/*-------------------------------------------------------*/
/* util/flowlog.c             ( WindTopBBS Ver 3.00 )    */
/*-------------------------------------------------------*/
/* target : 定期去 log.yzu.edu.tw 抓 log                 */
/* create : 2000/11/04                                   */
/* update : 2000/11/04                                   */
/*-------------------------------------------------------*/
#include "bbs.h"

#define HTTP_PORT		80
#define HTTP_SERVER		"log.yzu.edu.tw"
#define	FLOWLOG			"tmp/flowlog.log"
#define BRD_FLOWLOG		"FlowLog"

void
keeplog(fnlog, board, title, mode)
  char *fnlog;
  char *board;
  char *title;
  int mode;   
{
  HDR hdr;
  char folder[128], fpath[128];
  int fd;
  FILE *fp;

  if (!board)
    board = BRD_SYSTEM;

  sprintf(folder, "brd/%s/.DIR", board);
  fd = hdr_stamp(folder, 'A', &hdr, fpath);
  if (fd < 0)
    return;

  fp = fdopen(fd, "w");
  fprintf(fp, "作者: SYSOP (" SYSOPNICK ")\n標題: %s\n時間: %s\n",
    title, ctime(&hdr.chrono));   
  f_suck(fp, fnlog);
  fclose(fp);
  close(fd);
  unlink(fnlog);

  strcpy(hdr.title, title);
  strcpy(hdr.owner, "SYSOP");
  strcpy(hdr.nick, SYSOPNICK);
  fd = open(folder, O_WRONLY | O_CREAT | O_APPEND, 0600);
  if (fd < 0)
  {
    unlink(fpath);
    return;
  }
  write(fd, &hdr, sizeof(HDR));
  close(fd);
}

static int
http_conn(char *s)
{
  int sockfd;
  char *in,*out,*total;
  char buf[128],*ptr,str[128],*cc;
  FILE *fp,*fw;

  if(!(fp = fopen(FLOWLOG, "w+")))
  {
    printf("Error in open file\n");
  }
  
  if ((sockfd = dns_open(HTTP_SERVER, HTTP_PORT)) < 0)
  {
    printf("Error in dns_open\n");
    return 0;
  }

  write(sockfd, s, strlen(s));
  shutdown(sockfd, 1);
  fw = fdopen(sockfd,"r");
  fgets(buf,128,fw);
  sprintf(str,"%-40.40s  %10s  %10s  %10s\n","HOST","IN","OUT","TOTAL");
  fputs(str,fp);
  fgets(buf,128,fw);

  for(;fgets(buf,128,fw);)
  {
    ptr = strtok(buf," \r\n");
    in = strtok(NULL," \r\n");
    out = strtok(NULL," \r\n");
    total = strtok(NULL," \r\n");
    cc = strstr(buf,"bbs.yzu.edu.tw");
    if(cc)
      sprintf(str,"\033[1;33m%-40.40s  %10s  %10s  %10s\033[m\n",buf,in,out,total);
    else
      sprintf(str,"%-40.40s  %10s  %10s  %10s\n",buf,in,out,total);
    fputs(str,fp);
  }
  fclose(fp);
  close(sockfd);

  return 0;  
}

int
main()
{
  char webpage[80], sendform[80], title[80];
  time_t now;
  struct tm ntime, *xtime ;
  now = time(NULL) - 3600;
  xtime = localtime(&now);
  ntime = *xtime;

  chdir(BBSHOME);
  system("/sbin/ping -c 10 log.yzu.edu.tw");

  sprintf(webpage, "/ipfm/ipfm-2001-%02d.%02d-%02d.txt\n",
    ntime.tm_mon+1, ntime.tm_mday, ntime.tm_hour);
  sprintf(sendform, "GET %s HTTP/1.0\n\n", webpage);

  http_conn(sendform);

  sprintf(title, "[%02d月%02d日%02d點] 風之塔對外流量紀錄",
    ntime.tm_mon+1, ntime.tm_mday, ntime.tm_hour);
  keeplog(FLOWLOG,BRD_FLOWLOG, title, 2);

#if 1
  if(ntime.tm_hour == 4)
  {
    now = time(NULL) - 3600*6;
    xtime = localtime(&now);
    ntime = *xtime;

    sprintf(webpage, "/ipfm2/ipfm-2001-%02d.%02d.txt\n",
      ntime.tm_mon+1, ntime.tm_mday);
    sprintf(sendform, "GET %s HTTP/1.0\n\n", webpage);

    http_conn(sendform);

    sprintf(title, "[%02d月%02d日]     風之塔對外流量紀錄",
      ntime.tm_mon+1, ntime.tm_mday);
    keeplog(FLOWLOG, BRD_FLOWLOG, title, 2);
  }
#endif

  return 0;
}
