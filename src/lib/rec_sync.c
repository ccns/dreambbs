#include "dao.h"
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>


int
rec_sync(fpath, size, fsync, fchk)
  char *fpath;
  int size;
  int (*fsync) ();
  int (*fchk) ();
{
  int fd, fsize;
  struct stat st;

  fsize = 0;

  if ((fd = open(fpath, O_RDWR, 0600)) < 0)
    return fsize;

  if (!fstat(fd, &st) && (fsize = st.st_size) > 0)
  {
    char *base;

    base = (char *) malloc(fsize);
    fsize = read(fd, base, fsize);

    if (fsize >= size)
    {
      if (fchk)		/* �ˬd�O�_�������T����� */
      {
	char *head, *tail;

	head = base;
	tail = base + fsize;
	while (head < tail)
	{
	  if (fchk(head))	/* ������ƥ��T */
	  {
	    head += size;
	    continue;
	  }

	  /* �����D����ƭn�R�� */
	  tail -= size;
	  if (head >= tail)
	    break;
	  memcpy(head, tail, size);
	}
	fsize = tail - base;
      }

      if (fsize > 0)
      {
	if (fsize > size)
	  qsort(base, fsize / size, size, fsync);

	lseek(fd, 0, SEEK_SET);
	write(fd, base, fsize);
	ftruncate(fd, fsize);
      }
    }
    free(base);
  }
  close(fd);

  if (fsize <= 0)
    unlink(fpath);

  return fsize;
}
