#include <fcntl.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/stat.h>
#include "dao.h"

int
rec_put(fpath, data, size, pos)
  char *fpath;
  void *data;
  int size, pos;
{
  int fd;

  if ((fd = open(fpath, O_WRONLY | O_CREAT, 0600)) < 0)
    return -1;

  /* flock(fd, LOCK_EX); */
  /* Thor.981205: �� fcntl ���Nflock, POSIX�зǥΪk */
  f_exlock(fd);

  lseek(fd, (off_t) (size * pos), SEEK_SET);
  write(fd, data, size);

  /* flock(fd, LOCK_UN); */
  /* Thor.981205: �� fcntl ���Nflock, POSIX�зǥΪk */
  f_unlock(fd);

  close(fd);

  return 0;
}

int
rec_put2(fpath, data, size, pos, fchk)
  char *fpath;
  void *data;
  int size, pos;
  int (*fchk)();
{
  int fd;
  off_t off, len;
  char pool[REC_SIZ];
  struct stat st;

  if ((fd = open(fpath, O_RDWR | O_CREAT, 0600)) < 0)
    return -1;

  /* flock(fd, LOCK_EX); */
  /* Thor.981205: �� fcntl ���Nflock, POSIX�зǥΪk */
  f_exlock(fd);

  fstat(fd, &st);
  len = st.st_size;

  fpath = pool;
  off = size * pos;

  /* ���� pos ��m��ƪ����T�� */

  if (len > off)
  {
    if (fchk)
    {
      lseek(fd, off, SEEK_SET);
      read(fd, fpath, size);
      pos = (*fchk) (fpath);
    }
    else
    {
      pos = 1;
    }
  }
  else
  {
    if (len)
    {
      pos = 0;	/* �q�Y��_ */
    }
    else
    {
      /* �Y�쥻�O���ɮסA���� rec_put �� rec_add */
      pos = 1;
      off = 0;
    }
  }

  /* ���諸�ܡA���Y��_ */

  if (!pos)
  {
    off = 0;
    lseek(fd, off, SEEK_SET);
    while (read(fd, fpath, size) == size)
    {
      if (pos = (*fchk) (fpath))
	break;

      off += size;
    }
  }

  /* ��줧��A��s��� */

  if (pos)
  {
    lseek(fd, off, SEEK_SET);
    write(fd, data, size);
  }

  /* flock(fd, LOCK_UN); */
  /* Thor.981205: �� fcntl ���Nflock, POSIX�зǥΪk */
  f_unlock(fd);

  close(fd);

  return 0;
}
