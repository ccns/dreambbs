/*-------------------------------------------------------*/
/* bbslib.c	( NTHU CS MapleBBS Ver 3.10 )		 */
/*-------------------------------------------------------*/
/* target : innbbsd library				 */
/* create : 95/04/27					 */
/* update :   /  /  					 */
/* author : skhuang@csie.nctu.edu.tw			 */
/* modify : itoc.bbs@bbs.tnfsh.tn.edu.tw		 */
/*-------------------------------------------------------*/


#include "innbbsconf.h"
#include "bbslib.h"
#include <time.h>
#include <sys/time.h>

#ifdef _NoCeM_
#include "nocem.h"
#endif

#include <stdarg.h>


/* ----------------------------------------------------- */
/* read nodelist.bbs					 */
/* ----------------------------------------------------- */


int NLCOUNT;
nodelist_t *NODELIST = NULL;


int
nl_bynamecmp(a, b)
  nodelist_t *a, *b;
{
  return str_cmp(a->name, b->name);
}


static int		/* 0:success  -1:fail */
read_nodelist()
{
  int fd, size;
  struct stat st;

  if ((fd = open("innd/nodelist.bbs", O_RDONLY)) < 0)
    return -1;

  fstat(fd, &st);
  if ((size = st.st_size) <= 0)
  {
    close(fd);
    return -1;
  }
  NODELIST = !NODELIST ? (nodelist_t *) malloc(size) : (nodelist_t *) realloc(NODELIST, size);
  read(fd, NODELIST, size);
  close(fd);

  NLCOUNT = size / sizeof(nodelist_t);
  if (NLCOUNT > 1)
  {
    /* �N NODELIST[] �� name �ƧǡA����b search_nodelist_byname() �i�H��֤@�I */
    qsort(NODELIST, NLCOUNT, sizeof(nodelist_t), nl_bynamecmp);
  }

  return 0;
}


/* ----------------------------------------------------- */
/* read newsfeeds.bbs					 */
/* ----------------------------------------------------- */


int NFCOUNT;
newsfeeds_t *NEWSFEEDS = NULL;
newsfeeds_t *NEWSFEEDS_B = NULL;
newsfeeds_t *NEWSFEEDS_G = NULL;


int
nf_byboardcmp(a, b)
  newsfeeds_t *a, *b;
{
  return str_cmp(a->board, b->board);
}


int
nf_bygroupcmp(a, b)
  newsfeeds_t *a, *b;
{
  return str_cmp(a->newsgroup, b->newsgroup);
}


static int		/* 0:success  -1:fail */
read_newsfeeds()
{
  int fd, size;
  struct stat st;

  if ((fd = open("innd/newsfeeds.bbs", O_RDONLY)) < 0)
    return -1;

  fstat(fd, &st);
  if ((size = st.st_size) <= 0)
  {
    close(fd);
    return -1;
  }
  NEWSFEEDS = !NEWSFEEDS ? (newsfeeds_t *) malloc(size) : (newsfeeds_t *) realloc(NEWSFEEDS, size);
  read(fd, NEWSFEEDS, size);
  close(fd);

  /* �t�~�ǳƤG���ۦP����T�A���O�ƧǤ�k���P */
  NEWSFEEDS_B = !NEWSFEEDS_B ? (newsfeeds_t *) malloc(size) : (newsfeeds_t *) realloc(NEWSFEEDS_B, size);
  memcpy(NEWSFEEDS_B, NEWSFEEDS, size);
  NEWSFEEDS_G = !NEWSFEEDS_G ? (newsfeeds_t *) malloc(size) : (newsfeeds_t *) realloc(NEWSFEEDS_G, size);
  memcpy(NEWSFEEDS_G, NEWSFEEDS, size);

  NFCOUNT = size / sizeof(newsfeeds_t);
  if (NFCOUNT > 1)
  {
    /* NEWSFEEDS[] ���ܰʡA�w�]�̯��x�W�ٱƧ� */

    /* �N NEWSFEEDS_B[] �� board �ƧǡA����b search_newsfeeds_byboard() �i�H��֤@�I */
    qsort(NEWSFEEDS_B, NFCOUNT, sizeof(newsfeeds_t), nf_byboardcmp);

    /* �N NEWSFEEDS_G[] �� group �ƧǡA����b search_newsfeeds_bygroup() �i�H��֤@�I */
    qsort(NEWSFEEDS_G, NFCOUNT, sizeof(newsfeeds_t), nf_bygroupcmp);
  }

  return 0;
}


#ifdef _NoCeM_
/* ----------------------------------------------------- */
/* read ncmperm.bbs					 */
/* ----------------------------------------------------- */


ncmperm_t *NCMPERM = NULL;
int NCMCOUNT = 0;


int			/* 0:success  -1:fail */
read_ncmperm()
{
  int fd, size;
  struct stat st;

  if ((fd = open("innd/ncmperm.bbs", O_RDONLY)) < 0)
    return -1;

  fstat(fd, &st);
  if ((size = st.st_size) <= 0)
  {
    close(fd);
    return -1;
  }
  NCMPERM = !NCMPERM ? (ncmperm_t *) malloc(size) : (ncmperm_t *) realloc(NCMPERM, size);
  read(fd, NCMPERM, size);
  close(fd);

  NCMCOUNT = size / sizeof(ncmperm_t);

  return 0;
}
#endif	/* _NoCeM_ */


/* ----------------------------------------------------- */
/* read spamrule.bbs					 */
/* ----------------------------------------------------- */


spamrule_t *SPAMRULE = NULL;
int SPAMCOUNT = 0;


static int		/* 0:success  -1:fail */
read_spamrule()
{
  int fd, size;
  struct stat st;
  spamrule_t *spam;
  char *detail;

  if ((fd = open("innd/spamrule.bbs", O_RDONLY)) < 0)
    return -1;

  fstat(fd, &st);
  if ((size = st.st_size) <= 0)
  {
    close(fd);
    return -1;
  }
  SPAMRULE = !SPAMRULE ? (spamrule_t *) malloc(size) : (spamrule_t *) realloc(SPAMRULE, size);
  read(fd, SPAMRULE, size);
  close(fd);

  SPAMCOUNT = size / sizeof(spamrule_t);
  for (fd = 0; fd < SPAMCOUNT; fd++)
  {
    /* �N SPAMRULE[] ���ܦ��p�g�A�o�ˤ��ɴN�i�H�j�p�g�q�Y */
    spam = SPAMRULE + fd;
    detail = spam->detail;
    str_lowest(detail, detail);
  }

  return 0;
}


/* ----------------------------------------------------- */
/* initail INNBBSD					 */
/* ----------------------------------------------------- */


int			/* 1:success  0:failure */
initial_bbs()
{
  chdir(BBSHOME);		/* chdir to bbs_home first */

  /* �̧Ǹ��J nodelist.bbs�Bnewsfeeds.bbs�Bncmperm.bbs�Bspamrule.bbs */

  if (read_nodelist() < 0)
  {
    printf("���ˬd nodelist.bbs�A�L�kŪ��\n");
    return 0;
  }

  if (read_newsfeeds() < 0)
  {
    printf("���ˬd newsfeeds.bbs�A�L�kŪ��\n");
    return 0;
  }

#ifdef _NoCeM_
  if (read_ncmperm() < 0)
  {
    printf("���ˬd ncmperm.bbs�A�L�kŪ�ɡF�p�G�z���Q�]�w NoCeM�A����Щ������T��\n");
    /* return 0; */	/* ncmperm.bbs �i�H�O�Ū� */
  }
#endif

  if (read_spamrule() < 0)
  {
    printf("���ˬd spamrule.bbs�A�L�kŪ�ɡF�p�G�z���Q�]�w�׫H�W�h�A����Щ������T��\n");
    /* return 0; */	/* spamrule.bbs �i�H�O�Ū� */
  }

  return 1;
}


/* ----------------------------------------------------- */
/* log function						 */
/* ----------------------------------------------------- */


void
bbslog(char *fmt, ...)
{
  va_list args;
  char datebuf[40];
  time_t now;
  FILE *fp;

  if (fp = fopen(LOGFILE, "a"))
  {
    time(&now);
    strftime(datebuf, sizeof(datebuf), "%d/%b/%Y %H:%M:%S", localtime(&now));
    fprintf(fp, "%s ", datebuf);

    va_start(args, fmt);
    vfprintf(fp, fmt, args);
    va_end(args);

    fclose(fp);
  }
}
