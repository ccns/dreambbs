/* ----------------------------------------------------- */
/* exclusively create file [*.n]			 */
/* ----------------------------------------------------- */


#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include "dao.h"

FILE *
f_new(fold, fnew)
  char *fold;
  char *fnew;
{
  int fd, try;
  extern int errno;

  try = 0;
  str_cat(fnew, fold, ".n");

  for (;;)
  {
    fd = open(fnew, O_WRONLY | O_CREAT | O_EXCL, 0600);

    if (fd >= 0)
      return fdopen(fd, "w");

    if (errno != EEXIST)
      break;

    if (!try++)
    {
      struct stat st;

      if (stat(fnew, &st) < 0)
	break;
      if (st.st_mtime < time(NULL) - 20 * 60)	/* ���] 20 ���������ӳB�z�� */
	unlink(fnew);
    }
    else
    {
      if (try > 24)		/* ���� 120 ���� */
	break;
      sleep(5);
    }
  }
  return NULL;
}
