#include "dao.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

int
f_open(fpath)
  char *fpath;
{
  int fd;
  struct stat st;

  if ((fd = open(fpath, O_RDONLY)) >= 0)
  {
    if (fstat(fd, &st) || st.st_size <= 0 || !S_ISREG(st.st_mode))
    {
      close(fd);
      unlink(fpath);
      fd = -1;
    }
  }

  return fd;
}
