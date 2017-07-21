#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/stat.h>
#include "dao.h"

int
rec_ins(fpath, data, size, pos, num)
  char *fpath;
  void *data;
  int size;
  int pos;
  int num;
{
  int fd;
  off_t off, len;
  struct stat st;

  if ((fd = open(fpath, O_RDWR | O_CREAT, 0600)) < 0)
    return -1;

  /* flock(fd, LOCK_EX); */
  /* Thor.981205: �� fcntl ���Nflock, POSIX�зǥΪk */
  f_exlock(fd);

  fstat(fd, &st);
  len = st.st_size;

  /* lkchu.990428: ernie patch �p�G len=0 & pos>0
                   (�b��}��ذϥؿ��i�h�K�W�A��U�@��) �ɷ|�g�J�U�� */
  off = len ? size * pos : 0;
  lseek(fd, off, SEEK_SET);

  size *= num;
  len -= off;
  if (len > 0)
  {
    fpath = (char *) malloc(pos = len + size);
    memcpy(fpath, data, size);
    read(fd, fpath + size, len);
    lseek(fd, off, SEEK_SET);
    data = fpath;
    size = pos;
  }

  write(fd, data, size);

  /* flock(fd, LOCK_UN); */
  /* Thor.981205: �� fcntl ���Nflock, POSIX�зǥΪk */
  f_unlock(fd);

  close(fd);

  if (len > 0)
    free(data);

  return 0;
}

