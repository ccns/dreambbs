/*-------------------------------------------------------*/
/* util/brdmail.c	( NTHU CS MapleBBS Ver 3.00 )	 */
/*-------------------------------------------------------*/
/* target : �� Internet �H�H�� BBS �����ݪO�A���� post	 */
/* create : 95/03/29					 */
/* update : 97/03/29					 */
/*-------------------------------------------------------*/


#include "bbs.h"

#include <sysexits.h>

#define	ANTI_HTMLMAIL		/* itoc.021014: �� html_mail */
#define	ANTI_NOTMYCHARSETMAIL	/* itoc.030513: �� not-mycharset mail */


static void
mailog(msg)
  char *msg;
{
  FILE *fp;
/*
  if (fp = fopen(BMTA_LOGFILE, "a"))
  {
    time_t now;
    struct tm *p;

    time(&now);
    p = localtime(&now);
    fprintf(fp, "%02d/%02d %02d:%02d:%02d <brdmail> %s\n",
      p->tm_mon + 1, p->tm_mday,
      p->tm_hour, p->tm_min, p->tm_sec,
      msg);
    fclose(fp);
  }
*/
}


/* ----------------------------------------------------- */
/* board�Gshm �������P cache.c �ۮe			 */
/* ----------------------------------------------------- */


static BCACHE *bshm;


static void
init_bshm()
{
  /* itoc.030727: �b�}�� bbsd ���e�A���ӴN�n����L account�A
     �ҥH bshm ���Ӥw�]�w�n */

  bshm = shm_new(BRDSHM_KEY, sizeof(BCACHE));

  if (bshm->uptime <= 0)	/* bshm ���]�w���� */
    exit(0);
}


static BRD *
brd_get(bname)
  char *bname;
{
  BRD *bhdr, *tail;

  bhdr = bshm->bcache;
  tail = bhdr + bshm->number;
  do
  {
    if (!str_cmp(bname, bhdr->brdname))
      return bhdr;
  } while (++bhdr < tail);
  return NULL;
}


/* ----------------------------------------------------- */
/* �D�{��						 */
/* ----------------------------------------------------- */


static int
mail2brd(brd)
  BRD *brd;
{
  HDR hdr;
  char buf[512], title[256], sender[256], owner[256], nick[256], folder[64];
  char *str, *ptr, decode, *brdname;
  int fd;
  FILE *fp;

  /* check if the brdname is in our bbs now */

  brdname = brd->brdname;

  if (strstr(brdname, "ream_lo"))
    ;
  else
    return 0;


  brd_fpath(folder, brdname, NULL);
  if (!dashd(folder))
  {
    sprintf(buf, "BBS brd <%s> not existed", brdname);
    mailog(buf);
    return EX_NOUSER;
  }
  strcat(folder, "/" FN_DIR);

  /* parse header */

  title[0] = sender[0] = owner[0] = nick[0] = '\0';
  decode = 0;

  while (fgets(buf, sizeof(buf), stdin))
  {
start:
    if (!memcmp(buf, "From: ", 6))
    {
      str = buf + 6;

      if (*str == '\0')
	return EX_NOUSER;

      if (ptr = strchr(str, '\n'))
	*ptr = '\0';

      str_from(str, owner, nick);
      if (*nick)
	sprintf(sender, "%s (%s)", owner, nick);
      else
	strcpy(sender, owner);

      /* itoc.040804: �׫H�¥զW�� */
      str_lower(buf, owner);	/* �O���� email ���j�p�g */
      if (ptr = (char *) strchr(buf, '@'))
      {
	*ptr++ = '\0';

/*	if (!acl_has(MAIL_ACLFILE, buf, ptr) ||
	  acl_has(UNMAIL_ACLFILE, buf, ptr) > 0)
	{
	  sprintf(buf, "SPAM %s", sender);
	  mailog(buf);
	  return EX_NOUSER;
	}
*/
      }
    }

    else if (!memcmp(buf, "Subject: ", 9))
    {
      str_ansi(title, buf + 9, sizeof(title));
      /* str_decode(title); */
      /* LHD.051106: �Y�i��g RFC 2047 QP encode �h���i��h�� subject */
      if (strstr(buf + 9, "=?"))
      {
	while (fgets(buf, sizeof(buf), stdin))
	{
	  if (buf[0] == ' ' || buf[0] == '\t')  /* �ĤG��H��|�H�ťթ� TAB �}�Y */
	    str_ansi(title + strlen(title), strstr(buf, "=?"), sizeof(title));
	  else
	  {
	    str_decode(title);
	    goto start;
	  }
	}
      }
    }

    else if (!memcmp(buf, "Content-Type: ", 14))
    {
      str = buf + 14;

#ifdef ANTI_HTMLMAIL
      /* �@�� BBS �ϥΪ̳q�`�u�H��r�l��άO�q��L BBS ���H�峹��ۤv���H�c
         �Ӽs�i�H��q�`�O html �榡�άO�̭������a��L�ɮ�
         �Q�ζl�����Y�� Content-Type: ���ݩʧⰣ�F text/plain (��r�l��) ���H�󳣾פU�� */
      if (*str != '\0' && str_ncmp(str, "text/plain", 10))
      {
	sprintf(buf, "ANTI-HTML [%d] %s => %s", getppid(), sender, brdname);
	mailog(buf);
	return EX_NOUSER;
      }
#endif

#ifdef ANTI_NOTMYCHARSETMAIL
      {
	char charset[32];
	mm_getcharset(str, charset, sizeof(charset));
/*	if (str_cmp(charset, MYCHARSET) && str_cmp(charset, "us-ascii"))
	{
	  sprintf(buf, "ANTI-NONMYCHARSET [%d] %s => %s", getppid(), sender, brdname);
	  mailog(buf);
	  return EX_NOUSER;
	}
*/
      }
#endif
    }

    else if (!memcmp(buf, "Content-Transfer-Encoding: ", 27))
    {
      mm_getencode(buf + 27, &decode);
    }

    else if (buf[0] == '\n')
    {
      break;
    }
  }

  /* allocate a file for the new post */

  fd = hdr_stamp(folder, 'A', &hdr, buf);
  hdr.xmode = POST_INCOME;

  str_ncpy(hdr.owner, owner, sizeof(hdr.owner));
  str_ncpy(hdr.nick, nick, sizeof(hdr.nick));
  if (!title[0])
    sprintf(title, "�Ӧ� %.64s", sender);
  str_ncpy(hdr.title, title, sizeof(hdr.title));

  /* copy the stdin to the specified file */

  fp = fdopen(fd, "w");

  fprintf(fp, "�o�H�H: %.50s �ݪO: %s\n��  �D: %.72s\n�o�H��: %s\n\n",
    sender, brdname, title, Btime(&hdr.chrono));

  while (fgets(buf, sizeof(buf), stdin))
  {
    if (decode && ((fd = mmdecode(buf, decode, buf)) > 0))
      buf[fd] = '\0';

    fputs(buf, fp);
  }

  fclose(fp);

  /* append the record to the .DIR */

  rec_bot(folder, &hdr, sizeof(HDR));

  /* amaki.040311: �n��class_item()��s�� */
  brd->btime = -1;

  /* Thor.0827: �[�W parent process id�A�H�K��U���H */
  sprintf(buf, "[%d] %s => %s", getppid(), sender, brdname); 
  mailog(buf);

  return 0;
}


static void
sig_catch(sig)
  int sig;
{
  char buf[512];

  while (fgets(buf, sizeof(buf), stdin))
    ;
  sprintf(buf, "signal [%d]", sig);
  mailog(buf);
  exit(0);
}


int
main(argc, argv)
  int argc;
  char *argv[];
{
  char buf[512];
  BRD *brd;

  /* argv[1] is brdname in bbs */

  if (argc < 2)
  {
    printf("Usage:\t%s <bbs_brdname>\n", argv[0]);
    exit(-1);
  }

  setgid(BBSGID);
  setuid(BBSUID);
  chdir(BBSHOME);

  signal(SIGBUS, sig_catch);
  signal(SIGSEGV, sig_catch);
  signal(SIGPIPE, sig_catch);

  init_bshm();
  brd = brd_get(argv[1]);

  if (!brd || mail2brd(brd))
  {
    /* eat mail queue */
    while (fgets(buf, sizeof(buf), stdin))
      ;
  }
  exit(0);
}
