#include "dao.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/stat.h>


int
rec_loc(data, size, fchk)
  char *data;
  int size;
  int (*fchk) ();
{
  int fd,pos,tmp;
  off_t off;
  char pool[REC_SIZ];

  if ((fd = open(data, O_RDWR)) < 0)
    return -1;

  f_exlock(fd);

  pos = -1;
  data = pool;
  off = 0;
  tmp = 0;
  if(fchk)
  {
    lseek(fd, off, SEEK_SET);
    while (read(fd, data, size) == size)
    {
      pos++;
      if ((*fchk) (data))
      {
        tmp = 1;
        break;
      }
    }
  }

  f_unlock(fd);

  close(fd);

  return tmp ? pos : -1;
}
