#include "bbs.h"

//char replace[30];

char *
where(from)
char *from;
 {
 char *area, *token/*, *replace = (char *) malloc ( sizeof(char) * 30 )*/;
 char domain[30], *ip1, *ip2, fromhost[30], mydomain[30];
 static char replace[30];
 FILE *fp;
 int i,count,ok=0;

 strcpy (replace,from);
 strcpy (fromhost,from);

 if (fp=fopen("etc/hosts.db","r"))
 {
   while (fscanf(fp,"%s %s\n",domain,replace) != EOF)
   {
     if (strchr(domain,'!')) 
     {
       area=strtok(domain,"!");
       if(!strstr(fromhost,area)) 
         continue;
       strcpy(mydomain,&fromhost[strlen(area)]);
       ip1=strtok(NULL, "~");
       ip2=strtok(NULL, "~");
       strtok(mydomain, ".");
       if ( ip1 != NULL && ip2 != NULL &&
          (atoi(ip1) <= atoi(mydomain) && atoi(mydomain) <= atoi(ip2)))
       {
          ok = 1;
          break;
       }
       continue;
     }
     token = strtok(domain,"&");
     i=0;count=0;
     while(token)
     {
       if (strstr(from,token)) count++;
       token=strtok(NULL, "&");
       i++;
     }
     if (i==count) 
       break;
   }
   fclose (fp);
   if (i!=count && !ok)
     strcpy (replace,from);
 }
 return replace;
}

void gethost()
{
  char buf[50];
  struct hostent *host;
  unsigned char *addr;
  
  if(!vget(b_lines, 0, "請輸入欲查詢的DN ",buf,sizeof(buf),DOECHO))
    return;
    
  if(!(host = gethostbyname(buf)))
  {
    vmsg("輸入的DN錯誤或找不到相關資料");
    return;
  }
  
  addr = (unsigned char *) host->h_addr;
  sprintf(buf, "查詢結果：%d.%d.%d.%d\n",addr[0],addr[1],addr[2],addr[3]);
  vmsg(buf);
  
}

void getip()
{
   char genbuf[80];
   char buf[80];

   if(!vget(b_lines, 0, "請輸入欲查詢的 IP：",genbuf,16,DOECHO))
     return;
   sprintf(buf, "查詢結果：%s", where(genbuf));
   vmsg(buf);
}

int
x_whois()
{
   if(vans("請選擇功\能 [1] IP->位置 (2) DN->IP " ) == '2')
     gethost();
   else
     getip();
   return 0;
}

