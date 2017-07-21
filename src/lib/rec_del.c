#include "dao.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <stdlib.h>

int
rec_del(data, size, pos, fchk, fdel)
  char *data;
  int size;
  int pos;
  int (*fchk) ();
  int (*fdel) ();
{
  int fd;
  off_t off, len;
  char pool[REC_SIZ];
  struct stat st;

  if ((fd = open(data, O_RDWR)) < 0)
    return -1;

  /* flock(fd, LOCK_EX); */
  /* Thor.981205: �� fcntl ���Nflock, POSIX�зǥΪk */
  f_exlock(fd);

  fstat(fd, &st);
  len = st.st_size;

  data = pool;
  off = size * pos;

  /* ���� pos ��m��ƪ����T�� */

  if (len > off)
  {
    lseek(fd, off, SEEK_SET);
    read(fd, data, size);

    pos = fchk ? (*fchk) (data) : 1;
  }
  else
  {
    pos = 0;
  }

  /* ���諸�ܡA���Y��_ */

  if (!pos)
  {
    off = 0;
    lseek(fd, off, SEEK_SET);
    while (read(fd, data, size) == size)
    {
      if ((pos = (*fchk) (data)))
	break;

      off += size;
    }
  }

  /* ��줧��A�R����� */

  if (pos)
  {
    if (fdel)
      pos = (*fdel) (data);

    if (pos)
    {
      /* really delete it */

      len -= (off + size);
      data = (char *) malloc(len);
      read(fd, data, len);
    }
    else
    {
      /* just update it */

      len = size;
    }

    lseek(fd, off, SEEK_SET);
    write(fd, data, len);

    if (pos)
    {
      ftruncate(fd, off + len);
      free(data);
    }
  }

  /* flock(fd, LOCK_UN); */
  /* Thor.981205: �� fcntl ���Nflock, POSIX�зǥΪk */
  f_unlock(fd);

  close(fd);

  return 0;
}
