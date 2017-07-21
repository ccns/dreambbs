/*-------------------------------------------------------*/
/* util/gem-check.c	( NTHU CS MapleBBS Ver 3.00 )	 */
/*-------------------------------------------------------*/
/* target : ��ذϾ�z�{��				 */
/* create : 95/03/29				 	 */
/* update : 95/08/08				 	 */
/*-------------------------------------------------------*/
/* syntax : gem-check					 */
/*-------------------------------------------------------*/


#include "bbs.h"
#include <sys/stat.h>


/* GCHECK_LOG ����������| */
#define GCHECK_LOG      BBSHOME"/"FN_GCHECK_LOG
#define	GEM_DROP	0x0080


#define	GCHECK_PERIOD	(16 - 1)/* �C�j 16 �ѽ����@�� */


#define	GCHECK_DEPTH	30


#define	GEM_EXPIRE	45	/* gem �ܦh�s 45 �� */


#define	PROXY_DUE	30	/* proxy �ܦh�s 30 �� */


#define	CHRONO_INDEX	1


static char pgem[256], pool[256];
static FILE *flog;


static int gcheck(int level, char *fpath);


/* ----------------------------------------------------- */
/* synchronize folder & files				 */
/* ----------------------------------------------------- */


typedef struct
{
  time_t chrono;
  char prefix;
  char exotic;
}      SyncData;


static SyncData *sync_pool;
static int sync_size, sync_head;


#define	SYNC_DB_SIZE	4096


static int
sync_cmp(s1, s2)
  SyncData *s1, *s2;
{
  return s1->chrono - s2->chrono;
}


static void
sync_init(fname)
  char *fname;
{
  int ch, prefix;
  time_t chrono;
  char *str, fpath[256];
  struct stat st;
  struct dirent *de;
  DIR *dirp;

  SyncData *xpool;
  int xsize, xhead;

  if ((xpool = sync_pool))
  {
    xsize = sync_size;
  }
  else
  {
    xpool = (SyncData *) malloc(SYNC_DB_SIZE * sizeof(SyncData));
    xsize = SYNC_DB_SIZE;
  }

  xhead = 0;

  ch = strlen(fname);
  memcpy(fpath, fname, ch);
  fname = fpath + ch;
  *fname++ = '/';

  ch = '0';
  for (;;)
  {
    *fname = ch++;
    fname[1] = '\0';

    if ((dirp = opendir(fpath)))
    {
      fname[1] = '/';
      while ((de = readdir(dirp)))
      {
	str = de->d_name;
	prefix = *str;
	if (prefix == '.')
	  continue;

	strcpy(fname + 2, str);
	if (stat(fpath, &st) || !st.st_size)
	{
	  unlink(fpath);
	  fprintf(flog, "\t%s zero\n", fpath);
	  continue;
	}

	chrono = chrono32(str);

	if (xhead >= xsize)
	{
	  xsize += (xsize >> 1);
	  xpool = (SyncData *) realloc(xpool, xsize * sizeof(SyncData));
	}

	xpool[xhead].chrono = chrono;
	xpool[xhead].prefix = prefix;
	xpool[xhead].exotic = 1;
	xhead++;
      }

      closedir(dirp);
    }

    if (ch == 'W')
      break;

    if (ch == '9' + 1)
      ch = 'A';
  }

  if (xhead > 1)
    qsort(xpool, xhead, sizeof(SyncData), sync_cmp);

  sync_pool = xpool;
  sync_size = xsize;
  sync_head = xhead;

  fprintf(flog, "\n\t sync %d files\n\n", xhead);
}


static void
sync_check(fgem)
  char *fgem;
{
  int fback, aback, expire;
  char *str, *fname, fpath[128], fnew[128];
  SyncData *xpool, *xtail, *xsync;
  time_t cc, due;
  FILE *fpr, *fpw;
  HDR hdr;
  struct tm *pt;

//  if ((/* cc =*/ sync_head) <= 0)
  if (sync_head <= 0)
    return;

  fpr = fopen(fgem, "r");
  if (fpr == NULL)
  {
    fprintf(flog, "sync: %s\n", fgem);
    fpr = fopen(fgem, "w+");
    if(fpr == NULL)
    {
      fprintf(flog, "sync error: %s\n", fgem);
      return;
    }
  }

  sprintf(fnew, "%s.n", fgem);
  fpw = fopen(fnew, "w");
  if (fpw == NULL)
  {
    fclose(fpr);
    fprintf(flog, "sync: %s\n", fnew);
    return;
  }

  /* �M�z�O������z�� */
  /* Thor.981218: ���ޤF,�ɤO�@�U */
  strcpy(fpath, fgem);
  str = strrchr(fpath, '.');
  str[1] = '/';
  fname = str + 2;

  expire = 0;
  due = time(0) - GEM_EXPIRE * 86400;

  while (fread(&hdr, sizeof(hdr), 1, fpr) == 1)
  {
    /*if ((hdr.xmode & GEM_DROP) || !(memcmp(hdr.title, "�ɮ��B�� [", 10)))*/
    /* Thor.981218: �Y����� */
    if ((hdr.xmode & GEM_DROP) && !(memcmp(hdr.title, "�ɮ��B�� [", 10)))
    {
      if ((xsync = (SyncData *) bsearch(&hdr.chrono,
	  sync_pool, sync_head, sizeof(SyncData), sync_cmp)))
      {
	if (xsync->exotic == 0)	/* �w�Q reference */
	  continue;
        else
        {
        /* Thor.981218: ��U��, �S�S�Hreference */
        /* Thor.981218: hdr.xid ���^���ɶ� */
           if (hdr.xid < due)
           {
             expire++;

             /* Thor.981218: unlink file */
             xsync->exotic = 0; /* Thor.981218: �Q�ᱼ�F,�O�˦^�� */
             cc = xsync->chrono;
             *str = radix32[cc & 31];
             archiv32m(cc, fname);
             fname[0] = xsync->prefix;
	     fprintf(flog, "\texpire: %s\n", fpath);
   	     unlink(fpath);
	     continue;
           }
           else
           {
             /* Thor.981218: �h���@�| */
             /* hdr.xmode |= GEM_DROP; */
           }
        }
      }
      /* Thor.981210: �S���N��F */
      else
      {
	continue;
      }
    }

    fwrite(&hdr, sizeof(hdr), 1, fpw);
  }
  fclose(fpr);

  /* recycle : recover "lost chain" */

  xpool = xsync = sync_pool;
  xtail = xpool + sync_head /*cc*/;

  strcpy(fpath, fgem);
  str = strrchr(fpath, '.');
  str[1] = '/';
  fname = str + 2;

  /* setup header */

  memset(&hdr, 0, sizeof(hdr));
  time((time_t *) &hdr.xid);
  pt = localtime((time_t *) &hdr.xid);
  /* Thor.990329: y2k */
  sprintf(hdr.date, "%02d/%02d/%02d", pt->tm_mon + 1, pt->tm_mday, pt->tm_year % 100);

  /* �Y lost chain �� folder, ����g�^ */

  hdr.xmode = GEM_FOLDER | GEM_DROP;
  fback = 0;

  do
  {
    if ((xsync->exotic) && (xsync->prefix == 'F'))
    {
      xsync->exotic = 0;
      cc = xsync->chrono;
      *str = radix32[cc & 31];
      archiv32m(cc, fname);
      fname[0] = xsync->prefix;

      if (gcheck(1, fpath))
      {
	hdr.chrono = cc;
	strcpy(hdr.xname, fname);
	sprintf(hdr.title, "�ɮ��B�� [%s]", str);
	fwrite(&hdr, sizeof(hdr), 1, fpw);
	fback++;
      }
      else
      {
	fprintf(flog, "\tprune: %s\n", fpath);
	unlink(fpath);
      }
    }
  } while (++xsync < xtail);

  /* �B�z�@���ɮ� */

  hdr.xmode = GEM_DROP;
  aback = 0;

  do
  {
    if (xpool->exotic)
    {
      cc = xpool->chrono;
      *str = radix32[cc & 31];
      archiv32m(cc, fname);
      fname[0] = xpool->prefix;

      hdr.chrono = cc;
      strcpy(hdr.xname, fname);
      strcpy(hdr.owner, "�t�Φ۰ʺ��@");
      sprintf(hdr.title, "�ɮ��B�� [%s]", str);
      fwrite(&hdr, sizeof(hdr), 1, fpw);
      aback++;
    }
  } while (++xpool < xtail);

  fclose(fpw);

  rename(fnew, fgem);

  fprintf(flog, "\tsummary: %d F, %d A, %d X\n", fback, aback, expire);
}


/* ----------------------------------------------------- */
/* check the BM's operation log				 */
/* ----------------------------------------------------- */


static void
check_log(fpath)
  char *fpath;
{
  FILE *fpr, *fpw;
  struct stat st;
  char fold[256], fnew[256], buf[2048];

  sprintf(fold, "%s/@/@log", fpath);
  if ((fpr = fopen(fold, "r")))
  {
    fpw = NULL;
    if (!fstat(fileno(fpr), &st) && st.st_size > 32768)
    {
      fseek(fpr, st.st_size >> 1, 0);

      while (fgets(buf, sizeof(buf), fpr))
      {
	if (buf[0] == '[' && buf[5] == ']' && buf[6] == ' ' &&
	  buf[15] == ' ')
	{
	  sprintf(fnew, "%s.n", fold);
	  if ((fpw = fopen(fnew, "w")))
	  {
	    int len;

	    fputs(buf, fpw);
	    fputc('\n', fpw);
	    while ((len = fread(buf, 1, sizeof(buf), fpr)) > 0)
	    {
	      fwrite(buf, 1, len, fpw);
	    }
	    fclose(fpw);
	  }
	  break;
	}
      }

    }
    fclose(fpr);

    if (fpw)
    {
      sprintf(buf, "%s.o", fold);
      rename(fold, buf);
      rename(fnew, fold);
    }
  }
}


/* ----------------------------------------------------- */
/* visit the hierarchy recursively			 */
/* ----------------------------------------------------- */


static int
gcheck(level, fpath)
  int level;
  char *fpath;
{
  int count, xmode, xhead;
  char *fname, *ptr=NULL, buf[256];
  FILE *fp;
  HDR hdr;
  SyncData *xsync;

  if (!level)
  {
    fprintf(flog, "\n%s\n", fpath);	/* report */

    check_log(fpath);

    sync_init(fpath);

    sprintf(pgem, "%s/.GEM", fpath);

    sprintf(pool, "%s/.DIR", fpath);
    fpath = pool;
  }
  else if (level > GCHECK_DEPTH)/* endless loop ? */
  {
    fprintf(flog, "\tlevel: %d [%s]\n", level, fpath);
    return 1;
  }

  /* open the folder */

  fp = fopen(fpath, "r");
  if (!fp)
    return 0;

  strcpy(buf, fpath);

  fname = fpath;
  while ((xmode = *fname++))
  {
    if (xmode == '/')
      ptr = fname;
  }
  if (*ptr != '.')
    ptr -= 2;
  fname = ptr;

  /* --------------------------------------------------- */
  /* visit the header file				 */
  /* --------------------------------------------------- */

  count = 0;
  xhead = sync_head;
  while (fread(&hdr, sizeof(hdr), 1, fp) == 1)
  {
    ptr = hdr.xname;		/* F1234567 */

    xmode = hdr.xmode;

    if (*ptr == '@')
    {
      fprintf(flog, "\tspecial: %s\n", ptr);
      if((xmode & (GEM_FOLDER | GEM_BOARD | GEM_GOPHER | GEM_HTTP)) ==
            GEM_FOLDER)
      {
        sprintf(fname, "@/%s", ptr);
        gcheck(level + 1, fpath);
      }
      continue;
    }

    if (!(xmode & (GEM_BOARD | GEM_GOPHER | GEM_HTTP)))
    {
      if (hdr.chrono != chrono32(ptr))
	fprintf(flog, "\tchrono: %s in %s\n", ptr, buf);

      if ((xsync = (SyncData *) bsearch(&hdr.chrono,
	  sync_pool, xhead, sizeof(SyncData), sync_cmp)))
      {
	xsync->exotic = 0;	/* ���`���p : ���Q reference */
      }
      else if (hdr.chrono > time(0) - 20 * 60)
      {
	/* dummy entry, �åB�T�w���O�̪� 20 �������s�W����� */

	fprintf(flog, "\tentry: %s\n", ptr);
	continue;
      }
    }

    /* �Y���@�� folder �h recursive �i�J */

    if ((xmode & (GEM_FOLDER | GEM_BOARD | GEM_GOPHER | GEM_HTTP)) ==
      GEM_FOLDER)
    {
      sprintf(fname, "%c/%s", ptr[7], ptr);
      if (!gcheck(level + 1, fpath))
	continue;
    }

    count++;
  }

  fclose(fp);

  if (!level)
  {
    strcpy(pool, pgem);
//    fprintf(flog, "\sync: %s\n", pool);
//    gcheck(1, pool);

    sync_check(pgem);
  }

  return count;
}


static void
proxy_check(fpath)
  char *fpath;
{
  char *fname, *str;
  int cc, prefix;
  time_t due;
  struct dirent *de;
  DIR *dirp;

  fname = strchr(fpath, '/') + 1;
  due = time(0) - PROXY_DUE * 86400;

  cc = '0';
  for (;;)
  {
    *fname = cc++;
    fname[1] = '\0';

    if ((dirp = opendir(fpath)))
    {
      fname[1] = '/';
      while ((de = readdir(dirp)))
      {
	str = de->d_name;
	prefix = *str;
	if (prefix == '.')
	  continue;

	if (chrono32(str) < due)
	{
	  fprintf(flog, "\t%s\n", str);
	  strcpy(fname + 2, str);
	  unlink(fpath);
	}
      }
      closedir(dirp);
    }

    if (cc == 'W')
      return;

    if (cc == '9' + 1)
      cc = 'A';
  }
}


int
main(argc, argv)
  int argc;
  char *argv[];
{
  DIR *dirp;
  struct dirent *de;
  char *fname, fpath[128];

  /* -------------------------------------------------- */
  /* ��z��ذ� gem/					 */
  /* -------------------------------------------------- */

  umask(077);
  chdir(BBSHOME "/gem");

  if (argc > 1)
  {
    flog = stderr;
    sprintf(fpath, "brd/%s", argv[1]);
    gcheck(0, fpath);
    exit(0);
  }

  /* --------------------------------------------------- */
  /* visit the top folder				 */
  /* --------------------------------------------------- */

  flog = fopen(GCHECK_LOG, "w");

  fprintf(flog, "[gem]\n");

  argc = time(0) / 86400;

  if ((argc & GCHECK_PERIOD) == 0)
    gcheck(0, ".");

  /* --------------------------------------------------- */
  /* visit the second hierarchy for all boards		 */
  /* --------------------------------------------------- */

  strcpy(fpath, "brd");
  if ((dirp = opendir(fpath)))
  {
    fpath[3] = '/';
    while ((de = readdir(dirp)))
    {
      if (++argc & GCHECK_PERIOD)
	continue;

      fname = de->d_name;
      if (*fname != '.' && fname[1])
      {
	strcpy(fpath + 4, fname);
	gcheck(0, fpath);
	fflush(flog);
      }
    }
    closedir(dirp);
  }
  else
  {
    fprintf(flog, "## unable to visit [gem/brd]\n");
  }

  /* -------------------------------------------------- */
  /* ��z proxy net/					 */
  /* -------------------------------------------------- */

  fprintf(flog, "\n[proxy]\n");

  chdir(BBSHOME "/net");

  if ((dirp = opendir(".")))
  {
    argc = time(0) / 86400;

    while ((de = readdir(dirp)))
    {
      if (++argc & GCHECK_PERIOD)
	continue;

      fname = de->d_name;
      if (*fname != '.')
      {
	fprintf(flog, "\n%s\n", fname);
	sprintf(fpath, "%s/", fname);
	proxy_check(fpath);
	fflush(flog);
      }
    }
    closedir(dirp);
  }

  fclose(flog);
  exit(0);
}
