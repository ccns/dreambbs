#include <fcntl.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include "dao.h"

int
rec_mov(data, size, from, to)
  char *data;
  int size;
  int from;
  int to;
{
  int fd, backward;
  off_t off, len;
  char *pool;
  struct stat st;

  if ((fd = open(data, O_RDWR)) < 0)
    return -1;

  /* flock(fd, LOCK_EX); */
  /* Thor.981205: 用 fcntl 取代flock, POSIX標準用法 */
  f_exlock(fd);

  fstat(fd, &st);
  len = st.st_size / size - 1;

  if (from > to)
  {
    backward = from;
    from = to;
    to = backward;
    backward = 1;
  }
  else
  {
    backward = 0;
  }

  if (to >= len)
    to = len;

  off = size * from;
  lseek(fd, off, SEEK_SET);

  len = (to - from + 1) * size;
  pool = data = (char *) malloc(len + size);

  if (backward)
    data += size;
  read(fd, data, len);

  data = pool + len;
  if (backward)
    memcpy(pool, data, size);
  else
    memcpy(data, pool, size);

  data = pool;
  if (!backward)
    data += size;

  lseek(fd, off, SEEK_SET);
  write(fd, data, len);

  /* flock(fd, LOCK_UN); */
  /* Thor.981205: 用 fcntl 取代flock, POSIX標準用法 */
  f_unlock(fd);

  close(fd);
  free(pool);

  return 0;
}
