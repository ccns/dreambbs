/* ----------------------------------------------------- */
/* hdr_stamp - create unique HDR based on timestamp	 */
/* ----------------------------------------------------- */
/* fpath - directory					 */
/* token - A / F / 0					 */
/* ----------------------------------------------------- */
/* return : open() fd (not close yet) or link() result	 */
/* ----------------------------------------------------- */


#include "dao.h"
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <time.h>

#if 0
int
hdr_stamp(folder, token, hdr, fpath)
  char *folder;
  int token;
  HDR *hdr;
  char *fpath;
{
  char *fname, *family = NULL;
  int rc;
  char *flink, buf[128];

  flink = NULL;
  if (token & HDR_LINK)
  {
    flink = fpath;
    fpath = buf;
  }

  fname = fpath;
  while (rc = *folder++)
  {
    *fname++ = rc;
    if (rc == '/')
      family = fname;
  }
  if (*family != '.')
  {
    fname = family;
    family -= 2;
  }
  else
  {
    fname = family + 1;
    *fname++ = '/';
  }

  if (token &= 0xdf)		/* 變大寫 */
  {
    *fname++ = token;
  }
  else
  {
    *fname = *family = '@';
    family = ++fname;
  }

  token = time(0);

  for (;;)
  {
    *family = radix32[token & 31];
    archiv32(token, fname);

    if (flink)
      rc = f_ln(flink, fpath);
    else
      rc = open(fpath, O_WRONLY | O_CREAT | O_EXCL, 0600);

    if (rc >= 0)
    {
      memset(hdr, 0, sizeof(HDR));
      hdr->chrono = token;
      str_stamp(hdr->date, &hdr->chrono);
      strcpy(hdr->xname, --fname);
      break;
    }

    if (errno != EEXIST)
      break;

    token++;
  }

  return rc;
}
#endif

int
hdr_stamp(folder, token, hdr, fpath)
  char *folder;
  int token;
  HDR *hdr;
  char *fpath;
{
  char *fname, *family = NULL;
  int rc, chrono;
  char *flink, buf[128];

  flink = NULL;
  if (token & (HDR_LINK | HDR_COPY))
  {
    flink = fpath;
    fpath = buf;
  }

  fname = fpath;
  while ((rc = *folder++))
  {
    *fname++ = rc;
    if (rc == '/')
      family = fname;
  }
  if (*family != '.')
  {
    fname = family;
    family -= 2;
  }
  else
  {
    fname = family + 1;
    *fname++ = '/';
  }

  if ((rc = token & 0xdf))        /* 變大寫 */
  {
    *fname++ = rc;
  }
  else
  {
    *fname = *family = '@';
    family = ++fname;
  }

  chrono = time(0);

  for (;;)
  {
    *family = radix32[chrono & 31];
    archiv32(chrono, fname);

    if (flink)
    {
      if (token & HDR_LINK)
        rc = f_ln(flink, fpath);
      else
        rc = f_cp(flink, fpath, O_EXCL);
    }
    else
    {
      rc = open(fpath, O_WRONLY | O_CREAT | O_EXCL, 0600);
    }

    if (rc >= 0)
    {
      memset(hdr, 0, sizeof(HDR));
      hdr->chrono = chrono;
      str_stamp(hdr->date, &hdr->chrono);
      strcpy(hdr->xname, --fname);
      break;
    }

    if (errno != EEXIST)
      break;

    chrono++;
  }

  return rc;
}

