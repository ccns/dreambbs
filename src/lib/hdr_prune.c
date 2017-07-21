/* ----------------------------------------------------- */
/* nhead:						 */
/* 0 ==> �̾� TagList �s��R��				 */
/* !0 ==> �̾� range [nhead, ntail] �R��		 */
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
    if (xmode & dmode)		/* && *//* �w�R�� */
    {				/* Thor.0616: �n�R��������F�A��,
				 * �K�o�]�Xcancel message */
      hdr_fpath(fold, fpath, &hdr);
      unlink(fold);
    }
    else if			/* ���R�� */
	((xmode & POST_MARKED) ||	/* �аO */
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
      /* �Y���ݪO�N�s�u��H */

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
