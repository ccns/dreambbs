#include "bbs.h"

void  keeplog(fnlog, board, title, mode)
  char *fnlog;
  char *board;
  char *title;
  int mode;		/* 0:load 1: rename  2:unlink 3:mark*/
{
  HDR hdr;
  char folder[128], fpath[128];
  int fd;
  FILE *fp;

  if (!board)
    board = BRD_SYSTEM;

  sprintf(folder, "brd/%s/.DIR", board);
  fd = hdr_stamp(folder, 'A', &hdr, fpath);
  if (fd < 0)
    return;

  if (mode == 1 || mode == 3)
  {
    close(fd);
    /* rename(fnlog, fpath); */
    f_mv(fnlog, fpath); /* Thor.990409:可跨partition */
  }
  else
  {
    fp = fdopen(fd, "w");
    fprintf(fp, "作者: SYSOP (" SYSOPNICK ")\n標題: %s\n時間: %s\n",
      title, ctime(&hdr.chrono));
    f_suck(fp, fnlog);
    fclose(fp);
    close(fd);
    if (mode)
      unlink(fnlog);
  }
  if(mode == 3)
    hdr.xmode |= POST_MARKED;

  strcpy(hdr.title, title);
  strcpy(hdr.owner, "SYSOP");
  strcpy(hdr.nick, SYSOPNICK);

  rec_bot(folder, &hdr, sizeof(HDR));
}
