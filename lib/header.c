#include "dao.h"
#include <string.h>

void
hdr_fpath(fpath, folder, hdr)
  char *fpath;
  char *folder;
  HDR *hdr;
{
  char *str = NULL;
  int cc, chrono;

  while ((cc = *folder++))
  {
    *fpath++ = cc;
    if (cc == '/')
      str = fpath;
  }

  chrono = hdr->chrono;
  folder = hdr->xname;
  cc = *folder;
  if (cc != '@')
    cc = radix32[chrono & 31];

  if (*str == '.')
  {
    *str++ = cc;
    *str++ = '/';
  }
  else
  {
    str[-2] = cc;
  }

  if (hdr->xmode & GEM_EXTEND)
  {
    *str++ = 'X';
    archiv32(chrono, str);
  }
  else
    strcpy(str, hdr->xname);
}
/* ----------------------------------------------------- */
/* nhead:						 */
/* 0 ==> 依據 TagList 連鎖刪除				 */
/* !0 ==> 依據 range [nhead, ntail] 刪除		 */
/* ----------------------------------------------------- */
/* notice : *.n - new file				 */
/* *.o - old file				 */
/* ----------------------------------------------------- */


#include "dao.h"


int
hdr_prune(fpath, nhead, ntail)
  char *fpath;
  int nhead, ntail;
{
  HDR hdr;
  char fnew[80], fold[80];
  int count, fsize, xmode, cancel, dmode;
  FILE *fpr, *fpw;

  if (!(fpr = fopen(fpath, "r")))
    return -1;

  if (!(fpw = f_new(fpath, fnew)))
  {
    fclose(fpr);
    return -1;
  }

  xmode = *fpath;
  cancel = (xmode == 'b');
  dmode = (xmode == 'u') ? 0 : (POST_CANCEL | POST_DELETE);

  fsize = count = 0;
  while (fread(&hdr, sizeof(HDR), 1, fpr) == 1)
  {
    count++;
    xmode = hdr.xmode;
    if (xmode & dmode)		/* && *//* 已刪除 */
    {				/* Thor.0616: 要刪除的先砍了再說,
				 * 免得跑出cancel message */
      hdr_fpath(fold, fpath, &hdr);
      unlink(fold);
    }
    else if			/* 未刪除 */
	((xmode & POST_MARKED) ||	/* 標記 */
	(nhead && (count < nhead || count > ntail)) ||	/* range */
      (!nhead && Tagger(hdr.chrono, count - 1, TAG_NIN)))	/* TagList */
    {
      if ((fwrite(&hdr, sizeof(HDR), 1, fpw) != 1))
      {
	fclose(fpw);
	unlink(fnew);
	fclose(fpr);
	return -1;
      }
      fsize++;
    }
    else
    {
      /* 若為看板就連線砍信 */

      if (cancel)
	cancel_post(&hdr);

      hdr_fpath(fold, fpath, &hdr);
      unlink(fold);
    }
  }
  fclose(fpr);
  fclose(fpw);

  sprintf(fold, "%s.o", fpath);
  rename(fpath, fold);
  if (fsize)
    rename(fnew, fpath);
  else
    unlink(fnew);

  return 0;
}
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

