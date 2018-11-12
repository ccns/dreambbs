/*-------------------------------------------------------*/
/* util/hdr-dump.c	( NTHU CS MapleBBS Ver 2.36 )	 */
/*-------------------------------------------------------*/
/* target : 看板標題表     				 */
/* create : 95/03/29				 	 */
/* update : 95/12/15				 	 */
/*-------------------------------------------------------*/
/* Usage:	hdr-dump .DIR   			 */
/*-------------------------------------------------------*/


#include "bbs.h"
int
main(
  int argc,
  char *argv[])
{
  int inf, count;
  BMW bmw;
  ACCT acct;
  char fpath[80];

  if (argc < 3)
  {
    printf("Usage:\t%s <bmw file> <userid>\n", argv[0]);
    exit(1);
  }

  strcpy(fpath,BBSHOME"/");
  usr_fpath(fpath + strlen(BBSHOME) + 1,argv[2],".ACCT");
  inf = open(fpath, O_RDONLY);
  if (inf == -1)
  {
    printf("error open acct file %s\n",fpath);
    exit(1);
  }
  if(read(inf, &acct, sizeof(ACCT)) != sizeof(ACCT))
  {
    close(inf);
    printf("error open acct file\n");
    exit(1);
  }
  close(inf);


  inf = open(argv[1], O_RDONLY);
  if (inf == -1)
  {
    printf("error open bmw file\n");
    exit(1);
  }

  count = 0;

          
  while (read(inf, &bmw, sizeof(BMW)) == sizeof(BMW)) 
  {
     struct tm *ptime = localtime(&bmw.btime);
     count++;
     printf("%s%s(%02d:%02d)：%s\033[m\n", 
            bmw.sender == acct.userno ? "☆" : "\033[32m★",
            bmw.userid, ptime->tm_hour, ptime->tm_min, bmw.msg);
  }
  close(inf);

  exit(0);
}


