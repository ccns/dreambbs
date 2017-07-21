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
