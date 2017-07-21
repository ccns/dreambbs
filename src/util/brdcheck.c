/*-------------------------------------------------------*/
/* checkbrd.c                                            */
/*-------------------------------------------------------*/
/* author : cache.bbs@bbs.ee.ncku.edu.tw			     */
/* target : �ˬd brd �U�� .DIR                           */
/* create : 09/12/18                                     */
/* update :                                              */
/*-------------------------------------------------------*/


#if 0

�Х��ƥ��Ҧ����, ���ˬd�O�_�sDIR.o���c�O�_�s�b 

#endif


#include "bbs.h"

int
main(argc, argv)
  int argc;
  char *argv[];
{
  char c;

  if (argc > 2)
  {
    printf("Usage: %s [brdname]\n", argv[0]);
    return -1;
  }

  for (c = 'a'; c <= 'z'; c++)
  {
    char buf[64];
    char buf2[64];
    struct dirent *de;
    DIR *dirp;

    sprintf(buf, "/home/bbs/brd/%c", c);
    chdir(buf);

    if (!(dirp = opendir(".")))
      continue;

    while (de = readdir(dirp))
    {
      int fd;
      char *str;

      str = de->d_name;
      if (*str <= ' ' || *str == '.')
	continue;

      if ((argc == 2) && str_cmp(str, argv[1]))
	continue;

      sprintf(buf, "%s/" ".DIR", str);
      if ((fd = open(buf, O_RDONLY)) < 0)
      {
      printf("brd/%s is missing, cp DIR.o\n", buf);              
      //sprintf(buf2, "cp %s/.DIR.o %s/.DIR", str, str);
      //system(buf2);      
      }
      else
	    continue;


    }
      printf("brd/%c is done.\n", c);

    closedir(dirp);
  }

  return 0;
}
