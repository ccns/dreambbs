/*-------------------------------------------------------*/
/* util/news_expire.c	( YZU WindTopBBS Ver 3.00 )	 */
/*-------------------------------------------------------*/
/* target : News 自動砍信工具程式			 */
/* create : 					 	 */
/* update : 					 	 */
/*-------------------------------------------------------*/
/* syntax : news_expire 			 	 */
/*-------------------------------------------------------*/

#include <strings.h>

#include "bbs.h"
#include "news.h"

#define	DEF_DAYS	3

#define	EXPIRE_LOG	FN_NEWSEXPIRE_LOG

#if 1
/* ----------------------------------------------------- */
/* synchronize folder & files                            */
/* ----------------------------------------------------- */


typedef struct
{
  time_t chrono;
  char prefix[32];
  char exotic;
}      SyncData;


static SyncData *sync_pool;
static int sync_size, sync_head;


#define SYNC_DB_SIZE    2048


static int
sync_cmp(s1, s2)
  SyncData *s1, *s2;
{
  return s1->chrono - s2->chrono;
} 

static void
sync_init(flog,fname)
  FILE *flog;
  char *fname;
{
  int ch;
  char prefix[32],cc;
  time_t chrono;
  char *str, fpath[80];
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
        memset(prefix,0,32);
        strcpy(prefix,str);
        if (prefix[0] == '.')
          continue;

        strcpy(fname + 2, str);
        if (stat(fpath, &st) || !st.st_size)
        {
          unlink(fpath);
          fprintf(flog, "\t%s zero\n", fpath);
          continue;
        }
        
        cc = prefix[7];
        if(cc != ch-1)
        {
          unlink(fpath);
          fprintf(flog, "\t%s error file name\n", fpath);
          continue;
        }

        chrono = chrono32(str);

        if (xhead >= xsize)
        {
          xsize += (xsize >> 1);
          xpool = (SyncData *) realloc(xpool, xsize * sizeof(SyncData));
        }

        xpool[xhead].chrono = chrono;
        strcpy(xpool[xhead].prefix,prefix);
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
  fprintf(flog, "\tsync %d files\n", xhead);
}


static void
sync_check(flog, fgem)
  FILE *flog;
  char *fgem;
{
  char *str, *fname, fpath[128], fnew[128];
  SyncData *xsync;
  FILE *fpr, *fpw;
  HDR hdr;

  if ((sync_head) <= 0)
    return;

  fpr = fopen(fgem, "r");
  if (fpr == NULL)
  {
    fprintf(flog, "sync: %s\n", fgem);
    return;
  }

  sprintf(fnew, "%s.n", fgem);
  fpw = fopen(fnew, "w");
  if (fpw == NULL)
  {
    fclose(fpr);
    fprintf(flog, "sync: %s\n", fnew);
    return;
  } 
  strcpy(fpath, fgem);
  str = strchr(fpath, '@');
  str[1] = '/';
  fname = str + 2;

  while (fread(&hdr, sizeof(hdr), 1, fpr) == 1)
  {
    if ((xsync = (SyncData *) bsearch(&hdr.chrono,
          sync_pool, sync_head, sizeof(SyncData), sync_cmp)))
    {
      xsync->exotic = 0;
    }
    else
    {
      continue;
    }

    fwrite(&hdr, sizeof(hdr), 1, fpw);
  }
  fclose(fpr);
  fclose(fpw);
  rename(fnew, fgem);
}


static int
sync_sync(fgem,files)
  char *fgem;
  int *files;
{
  int expire;
  char *str, *fname, fpath[128];
  SyncData *xpool, *xtail, *xsync;

  if ((sync_head) <= 0)
    return 0;

  xpool = xsync = sync_pool;
  xtail = xpool + sync_head;

  strcpy(fpath, fgem);
  str = strchr(fpath, '@');
  str[1] = '/';
  fname = str + 2;

  expire = 0;
  *files = 0;
  
  do
  {
    if (xpool->exotic)
    {
      *str = xpool->prefix[7];
      strcpy(fname,xpool->prefix);
      unlink(fpath);
      expire++;
    }
    else
      (*files)++;
  } while (++xpool < xtail);

//  fprintf(flog, "\tsummary: %d files, %d sync\n", files, expire);
  return expire;
}

#endif

static void
expire(flog, bname, days)
  FILE *flog;
  char *bname;
  int days;
{
  HDR hdr;
  char fpath[128], fnew[128], index[128], *fname, *str;
  int done, keep;
  FILE *fpr, *fpw;

  int duetime;
  int files = 0, total, sync;
//  int xhead=0;
//  time_t uptime=NULL;

  struct dirent *de;
  DIR *dirp;
  char folder[128],*ptr;
  
  total = 0;
  duetime = time(NULL) - days * 24 * 60 * 60;
  

  
  sprintf(folder,"%s/@",bname);

  fprintf(flog, "%s\n", bname);  

  sync_init(flog,bname);

  if (!(dirp = opendir(folder)))
  {
    fprintf(flog, ":Err: unable to visit boards \n"); 
    return;
  }

  while ((de = readdir(dirp)))
  {
    ptr = de->d_name;
    if (ptr[0] == '@' && !strchr(ptr,'.'))
    {

      fprintf(flog, "\texpire class : %s\n", ptr);
      sprintf(index, "%s/@/%s", bname, ptr);
      sync_check(flog,index);
      if (!(fpr = fopen(index, "r")))
      {
        fprintf(flog, "\tError open file: %s\n", index);
        return;
      }

      fpw = f_new(index, fnew);
      if (!fpw)
      {
        fprintf(flog, "\tExclusive lock: %s\n", fnew);
        fclose(fpr);
        return;
      }

      strcpy(fpath, index);
      str = (char *) strchr(fpath, '@');
      fname = str + 1;
      *fname++ = '/';

      done = 1;

      while (fread(&hdr, sizeof(hdr), 1, fpr) == 1)
      {
        if (hdr.chrono < duetime)
          keep = 0;
        else
          keep = 1;

        if (keep)
        {
          if (fwrite(&hdr, sizeof(hdr), 1, fpw) != 1)
          { 
    	    fprintf(flog, "\tError in write DIR.n: %s\n", hdr.xname);
	    done = 0;
	    break;
          }
        }
        else
        {
          *str = hdr.xname[7];
          strcpy(fname, hdr.xname);
          unlink(fpath);
          total++;
          fprintf(flog, "\t\t%s\n", fname);
        }
      }
      fclose(fpr);
      fclose(fpw);

      if (done)
      {
        sprintf(fpath, "%s.o", index);
        if (!rename(index, fpath))
        {
          if (rename(fnew, index))
            rename(fpath, index);		/* 換回來 */
        }
      }
      unlink(fnew);    
    
    }
  }
  closedir(dirp);
  sync = sync_sync(folder,&files);
  fprintf(flog, "\tsummary: %d files, %d expire, %d sync\n", files, total, sync);

}


int
main(argc, argv)
  int argc;
  char *argv[];
{
  FILE *fp;
  int days,number;
  struct dirent *de;
  DIR *dirp;
  char *ptr;

  days = ((argc > 1) && (number = atoi(argv[1])) > 0) ? number : DEF_DAYS;

  /* --------------------------------------------------- */
  /* load expire.ctl					 */
  /* --------------------------------------------------- */

  setgid(BBSGID);
  setuid(BBSUID);
  chdir(BBSHOME);

  /* --------------------------------------------------- */
  /* visit all boards					 */
  /* --------------------------------------------------- */

  fp = fopen(EXPIRE_LOG, "w");

  if (chdir(NEWS_ROOT) || !(dirp = opendir(".")))
  {
    fprintf(fp, ":Err: unable to visit news \n"); /* statue.000713 */
    fclose(fp);
    exit(-1);
  }

  while ((de = readdir(dirp)))
  {
    ptr = de->d_name;
    if (ptr[0] > ' ' && ptr[0] != '.')
      expire(fp, ptr,days);	
  }
  closedir(dirp);

  fclose(fp);
  
  if(sync_pool)
    free(sync_pool);
      
  exit(0);
}
