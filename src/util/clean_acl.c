/*-------------------------------------------------------*/
/* util/clean_acl.c     ( YZU WindTopBBS Ver 3.00 )      */
/*-------------------------------------------------------*/
/* target : �M�� mail.acl �����Ƥ�����                   */
/* create : 95/03/29                                     */
/* update : 97/03/29                                     */
/*-------------------------------------------------------*/
/* syntax : clean_acl [acl file]                         */
/*-------------------------------------------------------*/

#include "bbs.h"

#define MAX_AC	(50000)

char map[MAX_AC][256];
int total;

static int
check_in(email)
  char *email;
{
  int i;
  for(i=0;i<MAX_AC;i++)
    if(!strcmp(map[i],email))
      return 1;
  return 0;
}

int
main(argc, argv)
  int argc;
  char *argv[];
{
  FILE *fp,*fd;
  char buf[256],tmp[256],*ptr;
  
  if(argc > 2)
  {
    fp = fopen(argv[1],"r");
    if(fp)
    {
      fd = fopen(argv[2],"w");
      if(fd)
      {
        while(fgets(buf,256,fp))
        {
          if(strstr(buf,".epaper.com.tw"))
            continue;
          if(strstr(buf,MYHOSTNAME))
            continue;  
          strcpy(tmp,buf);
          ptr = (char *)strchr(buf,'#');
          if(ptr)
            *ptr = '\0';
          if(!check_in(buf))
          {
            fprintf(fd,"%s",tmp);
            strcpy(map[total++],buf);
          }
        }
        fclose(fd);
      }
      fclose(fp);
    } 
  }
  return 0;
}
