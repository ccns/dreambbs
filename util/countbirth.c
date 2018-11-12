/*------------------------------------------------*/
/*     ¹Ø¬Pµ{¦¡               96 10/11            */
/*     the program is designed by Ptt             */
/*     you can emailto://b3504102@csie.ntu.edu.tw */
/*     Ptt BBS telnet://ptt.m8.ntu.edu.tw         */
/*------------------------------------------------*/

/*-------------------------------------------------------*/
/* util/countbirth.c    ( NTHU CS MapleBBS Ver 3.00 )    */
/*-------------------------------------------------------*/
/* author : wsyfish.bbs@fpg.m4.ntu.edu.tw                */
/* Modify : visor.bbs@bbs.yzu.edu.tw                     */
/* target : ¹Ø¬P²Î­pµ{¦¡         	                 */
/* create : 96/10/11                                     */
/* update : 2000/07/27                                   */
/* µù: §ï¦Û ptt ¹Ø¬Pµ{¦¡  Maple 3.x ¥i¾A¥Î  	         */
/*     ¤£±H«Hµ¹¨Ï¥ÎªÌ³qª¾¥Í¤é§Ö¼Ö               	 */
/*-------------------------------------------------------*/


#include "bbs.h"

#define HAVE_MONTH

#define OUTFILE            BBSHOME"/gem/@/@-birth"


#define	MAX_MONTH	(500)
#define	MAX_WEEK	(200)
#define	MAX_TODAY	(50)

static int
mail2usr(
  char *userid,
  char *fpath)
{
  HDR mhdr;
  time_t now;
  char folder[128],buf[128];
  FILE *fp;
  
  usr_fpath(buf,userid,FN_DIR);
  now = time(0);  

  if((fp = fdopen(hdr_stamp(buf, 0, &mhdr, folder),"w")))
  {
    fprintf(fp,"§@ªÌ: SYSOP (%s)\n",SYSOPNICK);
    fprintf(fp,"¼ÐÃD: [®¥³ß]¥Í¤é§Ö¼Ö\n");
    fprintf(fp,"®É¶¡: %s",ctime(&now));

    f_suck(fp, fpath);         
    fprintf(fp,"--\n\033[1;32m¡° Origin: \033[1;33m%s \033[1;37m<%s>\n\033[1;31m¡» From: \033[1;36m%s\033[m\n"
             ,BOARDNAME,MYHOSTNAME,MYHOSTNAME);  
    fclose(fp);             
    strcpy(mhdr.owner, "SYSOP");
    strcpy(mhdr.nick, SYSOPNICK);
    strcpy(mhdr.title, "[®¥³ß]¥Í¤é§Ö¼Ö");
    rec_add(buf, &mhdr, sizeof(HDR));
    return 1;
  }
  return 0;
}


int 
main(void)
{
    FILE *fp1;
    char today[MAX_TODAY][14],today_name[MAX_TODAY][24],week[MAX_WEEK][14],week_day[MAX_WEEK],c;

    #ifdef HAVE_MONTH
    char month[MAX_MONTH][14],month_day[MAX_MONTH];
    int mon =0;
    #endif

    int i,day=0,wee=0,a[MAX_TODAY],b[MAX_TODAY];
    time_t now;
    struct tm *ptime;

    chdir(BBSHOME);

    now = time(NULL) ;     /* back to ancent */
    ptime = localtime(&now);


  for (c = 'a'; c <= 'z'; c++)
  {
    char buf[64];
    struct dirent *de;
    DIR *dirp;

    sprintf(buf, BBSHOME "/usr/%c", c);

    
    if (!(dirp = opendir(buf)))
      continue;

    while ((de = readdir(dirp)))
    {
      ACCT cuser;
      USER_ATTR	attr;
      int fd;

      if (de->d_name[0] <= ' ' || de->d_name[0] == '.')
        continue;

      usr_fpath(buf,de->d_name,".ACCT");
      if ((fd = open(buf, O_RDONLY)) < 0)
        continue;

      read(fd, &cuser, sizeof(cuser));
      close(fd);
      
      if(attr_get(cuser.userid,ATTR_USER_KEY,&attr) < 0)
        continue;
      
      if((attr.month == ptime->tm_mon +1) && (attr.day == ptime->tm_mday))
        mail2usr(cuser.userid,FN_ETC_BIRTHDAY);


      if(!(attr.mode & USER_ATTR_SUPPORT))
        continue;

           if(attr.month == ptime->tm_mon +1)
           {
             if(attr.day == ptime->tm_mday)
             {
                if((cuser.numlogins + cuser.numposts) < 10) continue;
                a[day]=cuser.numlogins;
                b[day]=cuser.numposts;
                strcpy(today[day],cuser.userid);
                strcpy(today_name[day++],cuser.username);
             }
             else if(attr.day <= ptime->tm_mday+2 && attr.day >=
                 ptime->tm_mday-2)
             {
                if((cuser.numlogins + cuser.numposts) < 10) 
                  continue;
                week_day[wee] = attr.day;
                strcpy(week[wee++],cuser.userid);
             }
#ifdef HAVE_MONTH
             else
             {
               month_day[mon] = attr.day;
               strcpy(month[mon++],cuser.userid);
             }
           }
#endif

    } /* while */
    closedir(dirp);
  } /* for */

    fp1=fopen(OUTFILE,"w");

    fprintf(fp1,"\n                      "
"[1;37m¡¹[35m¡¹[34m¡¹[33m¡¹[32m¡¹[31m¡¹[45;33m ¹Ø¬P¤jÆ[ "
"[31;40m¡¹[32m¡¹[33m¡¹[34m¡¹[35m¡¹[37m¡¹[m \n\n");
    if(day>0)
    {
      fprintf(fp1,"[33m¡i[1;45m¥»¤é¹Ø¬P[40;33m¡j[m \n");
      for (i=0;i<day;i++)
      {
        fprintf(fp1,"   [33m[%2d/%-2d] %-14s[0m %-24s login:%-5d post:%-5d\n",
           ptime->tm_mon+1,ptime->tm_mday,today[i],today_name[i],a[i],b[i]);
      }
      fprintf(fp1,"\n");
    }
    if(wee>0)
    {
      fprintf(fp1,"[33m¡i[1;45m«e«á¨â¤Ñ¤º¹Ø¬P[40;33m¡j[m \n");
      for (i=0;i<wee;i++)
      {
        fprintf(fp1,"   [%2d/%-2d] [36m%-14s[m"
               ,ptime->tm_mon+1,week_day[i],week[i]);
        if(!((i+1)%3)) 
          fprintf(fp1,"\n");
      }
      if(i%3)
        fprintf(fp1,"\n");
    }
#ifndef HAVE_MONTH    
    fclose(fp1);
#endif

#ifdef HAVE_MONTH

    fprintf(fp1,"\n[33m¡i[1;45m¥»¤ë¹Ø¬P[40;33m¡j[m \n");
    for (i=0;i<mon;i++)
    {
      fprintf(fp1,"   [%2d/%-2d] %-14s",ptime->tm_mon+1,month_day[i],month[i]);
      if(!((i+1)%3)) 
        fprintf(fp1,"\n");
    }
    fclose(fp1);
#endif
    return 0;
}
