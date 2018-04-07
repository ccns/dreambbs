/*-------------------------------------------------------*/
/* util/topuser.c         (MapleBBS Ver 3.00 )           */
/*-------------------------------------------------------*/
/* author : gslin@abpe.org			         */
/*          mat.bbs@fall.twbbs.org                       */
/* target : ¤W¯¸¦¸¼Æ¡BÄé¤ô¦¸¼Æ¡B±¾¯¸®É¼Æ±Æ¦æº]           */
/* create : 99/03/29                                     */
/* update : 99/03/29                                     */
/*-------------------------------------------------------*/
/* syntax : topuser                                      */
/*-------------------------------------------------------*/

#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bbs.h"

DATA toplogins[TOPNUM], topposts[TOPNUM], topstay[TOPNUM];

int
sort_compare(p1, p2)
  const void *p1;
  const void *p2;
{
  DATA *a1, *a2;

  a1 = (DATA *) p1;
  a2 = (DATA *) p2;

  return (a2->num-a1->num);
}

DATA *
findmin(src)
  DATA *src;
{
  int i;
  DATA *p;

  p = src;
  for (i = 0; i < TOPNUM; i++)
    if (src[i].num < p->num)
      p = src+i;
  return (p);
}

void
merge_id_nick(dst, userid, nick)
  char *dst;
  char *userid;
  char *nick;
{
  if (*userid)
  {
    sprintf(dst, "%s (%s)", userid, nick);

    if (strlen(dst) > 25)
      dst[25] = 0;
  }
  else
    dst[0] = 0;
}

void
write_data(title, data,mode)
  char *title;
  DATA *data;
  int mode;
{
  char buf[256];
  int i;
  char *color;

  sprintf(buf, "[1;32m%%%ds[m\n\n", 80);
  printf(buf, "%s", title);
  if(mode == 0)
  {
    puts("[1;31m¦W¦¸[m  [1;33mID[m ([1;34mNickname[m)  \
           [1;36m¦¸¼Æ[m    [1;31m¦W¦¸[m  [1;33mID[m ([1;34m\
Nickname[m)             [1;36m¦¸¼Æ[m\n-----------------------------\
-------    ------------------------------------");
  }
  else
  {
    puts("[1;31m¦W¦¸[m  [1;33mID[m ([1;34mNickname[m)  \
           [1;36m®É¼Æ[m    [1;31m¦W¦¸[m  [1;33mID[m ([1;34m\
Nickname[m)             [1;36m®É¼Æ[m\n-----------------------------\
-------    ------------------------------------");
  }

  for (i = 0; i < TOPNUM_HALF; i++)
  {
    char buf1[80], buf2[80];

    merge_id_nick(buf1, data[i].userid, data[i].username);
    merge_id_nick(buf2, data[i+TOPNUM_HALF].userid,
		  data[i+TOPNUM_HALF].username);
    switch(i)
    {
      case 0:
        color = "\033[1;31m";
        break;
      case 1:
        color = "\033[1;32m";
        break;
      case 2:
        color = "\033[1;33m";
        break;
      default:
        color = "\033[m";
    }

    printf("%s[%2d]  %-25s%5d\033[m    [%2d]  %-25s%5d\n",color, i+1, buf1, data[i].num,
	   i+TOPNUM_HALF+1, buf2, data[i+TOPNUM_HALF].num);
  }

  printf("\n");
}

int
main()
{
  char c;

  memset(toplogins, 0, sizeof(toplogins));
  memset(topposts, 0, sizeof(topposts));
  memset(topstay, 0, sizeof(topstay));

  for (c = 'a'; c <= 'z'; c++)
  {
    char buf[64];
    struct dirent *de;
    DIR *dirp;

    sprintf(buf, BBSHOME "/usr/%c", c);
    chdir(buf);

    if (!(dirp = opendir(".")))
      continue;

    while ((de = readdir(dirp)))
    {
      ACCT cuser;
      DATA *p;
      int fd;

      if (de->d_name[0] <= ' ' || de->d_name[0] == '.')
	continue;

      sprintf(buf, "%s/.ACCT", de->d_name);
      if ((fd = open(buf, O_RDONLY)) < 0)
	continue;

      read(fd, &cuser, sizeof(cuser));
      close(fd);

      if ((p = findmin(toplogins))->num < cuser.numlogins)
      {
	strncpy(p->userid, cuser.userid, IDLEN);
	strncpy(p->username, cuser.username, 24);
	p->num = cuser.numlogins;
      }

      if ((p = findmin(topposts))->num < cuser.numposts)
      {
	strncpy(p->userid, cuser.userid, IDLEN);
	strncpy(p->username, cuser.username, 24);
	p->num = cuser.numposts;
      }

      if ((p = findmin(topstay))->num < (cuser.staytime/3600))
      {
        strncpy(p->userid, cuser.userid, IDLEN);
        strncpy(p->username, cuser.username, 24);
        p->num = cuser.staytime/3600;
      }
    }
  }

  qsort(toplogins, TOPNUM, sizeof(DATA), sort_compare);
  qsort(topposts, TOPNUM, sizeof(DATA), sort_compare);
  qsort(topstay, TOPNUM, sizeof(DATA), sort_compare);

  chdir(BBSHOME);

  write_data("\033[33;1m===========\033[44m    ¤W¯¸¦¸¼Æ±Æ¦æº]    \033[40m============\033[m", toplogins,0);
  write_data("\033[33;1m===========\033[44m    Äé¤ô¦¸¼Æ±Æ¦æº]    \033[40m============\033[m", topposts,0);
  write_data("\033[33;1m===========\033[44m    ±¾¯¸®É¼Æ±Æ¦æº]    \033[40m============\033[m", topstay,1);
  return 0;
}
