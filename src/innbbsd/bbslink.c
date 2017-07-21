/*-------------------------------------------------------*/
/* bbslink.c	( NTHU CS MapleBBS Ver 3.10 )		 */
/*-------------------------------------------------------*/
/* target : innbbsd NNTP and NNRP			 */
/* create : 95/04/27					 */
/* update : 04/10/23					 */
/* author : skhuang@csie.nctu.edu.tw			 */
/* modify : itoc.bbs@bbs.tnfsh.tn.edu.tw		 */
/*-------------------------------------------------------*/


#include "innbbsconf.h"
#include "bbslib.h"
#include "inntobbs.h"
#include "nntp.h"
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>

#if 0	/* itoc.030122.����: �{���y�{ */

  0. bbsd �|��s�峹�����Y�O���b out.bntp

  1. ���楻�{���H��A�b main() �B�z�@�U�Ѽ�

  2. �b main():initial_bbs() Ū�X�]�w�ɡA�M��i�J bbslink()

  3. �b bbslink():deal_bntp() �������B�z out.bntp
     �ѩ� out.bntp �O��Ҧ��O���s�峹����b�@�_�A�ҥH�b�o�̧�o out.bntp �ɨ̯��x���h *.link

  4. �b bbslink():visit_site() ���̥H�U�B�J�A�@�@���X�U��

     4.1. open_connect() �}�ҳs�u
     4.2. send_outgoing() �⥻���x������ link �ɤ@���@��Ū�X�ӡA��H�e�h��诸
     4.3. readnews() �̧�Ū���C�ӷQ�n�� newsgroup�A�è���诸���H
     4.4. close_connect() �����s�u

  [��] �Y�ϨS���Ұ� innbbsd�A�]�i�H�ϥ� bbslink

#endif


static int SERVERfd = -1;
static FILE *SERVERrfp = NULL;
static FILE *SERVERwfp = NULL;
static char SERVERbuffer[1024];


/* itoc.030122.����: �H�U�o�X�Ӧb���w�ѼƮɤ~���� */
static int Verbose = 0;			/* 1: ��ܸԲӰT�� */
static int KillFormerProc = 0;		/* 1: �R���W�����楢�Ѫ� bbslink */
static int ResetActive = 0;		/* 1: �N high-number ��s��P news server �W�ۦP */
static int MaxArts = MAX_ARTS;		/* �� news server �C�Ӹs�ճ̦h�u��X�ʤ峹 */
static char *DefaultProcSite = NULL;	/* !=NULL: �u�B�z�Y�S�w���x */


#define DEBUG(arg)	if (Verbose) printf arg


/*-------------------------------------------------------*/
/* �B�z bntp ��						 */
/*-------------------------------------------------------*/


static nodelist_t *
search_nodelist_bynode(name)
  char *name;
{
  nodelist_t nl;

  str_ncpy(nl.name, name, sizeof(nl.name));
  return bsearch(&nl, NODELIST, NLCOUNT, sizeof(nodelist_t), nl_bynamecmp);
}


static newsfeeds_t *
search_newsfeeds_byboard(board)
  char *board;
{
  newsfeeds_t nf;

  str_ncpy(nf.board, board, sizeof(nf.board));
  return bsearch(&nf, NEWSFEEDS_B, NFCOUNT, sizeof(newsfeeds_t), nf_byboardcmp);
}


typedef struct
{
  char board[IDLEN + 1];
  char filename[9];
  char group[80];
  char from[80];
  char title[80];
  char date[40];
  char msgid[80];
  char control[80];
  char charset[20];
}	soverview_t;


static void
queuefeed(node, sover)
  nodelist_t *node;
  soverview_t *sover;
{
  int fd;

  /* itoc.030122.����: *.link �ɬO�̯��x���n �ݰe(�ΰe����)�� batch */

  if (node->feedfd < 0)
  {
    char linkfile[64];

    sprintf(linkfile, "innd/%s.link", node->name);
    if ((fd = open(linkfile, O_WRONLY | O_CREAT | O_APPEND, 0600)) < 0)
      return;
    node->feedfd = fd;
  }
  else
  {
    fd = node->feedfd;
  }

  /* flock(fd, LOCK_EX); */
  /* Thor.981205: �� fcntl ���Nflock, POSIX�зǥΪk */
  f_exlock(fd);

  write(fd, sover, sizeof(soverview_t));

  /* flock(fd, LOCK_UN); */
  /* Thor.981205: �� fcntl ���Nflock, POSIX�зǥΪk */
  f_unlock(fd);
}


static char *
Gtime(now)
  time_t now;
{
  static char datemsg[40];

  strftime(datemsg,sizeof(datemsg),"%d %b %Y %H:%M:%S GMT",localtime(&now));
  return datemsg;
}


static void
deal_sover(bntp)
  bntp_t *bntp;
{
  newsfeeds_t *nf;
  nodelist_t *nl;
  soverview_t sover;
  time_t mtime;
  char buf[80];
  char *board, *filename;

  board = bntp->board;

  if (!(nf = search_newsfeeds_byboard(board)))
  {
    bbslog("<bbslink> :Warn: %s ���O���b newsfeeds.bbs ��\n", board);
    DEBUG(("�w��:Warn: %s ���O���b newsfeeds.bbs ��\n", board));
    return;
  }

  if (!(nl = search_nodelist_bynode(nf->path)))
    return;

  filename = bntp->xname;

  memset(&sover, 0, sizeof(soverview_t));

  if (bntp->chrono > 0)		/* �s�H */
  {
    mtime = bntp->chrono;
    str_ncpy(sover.title, bntp->title, sizeof(sover.title));
    sprintf(sover.msgid, "%s$%s@" MYHOSTNAME, board, filename);
  }
  else				/* cancel */
  {
    time(&mtime);
    sprintf(buf, "%s$%s@" MYHOSTNAME, board, filename);		/* ����峹�� Message-ID */
    sprintf(sover.title, "cmsg cancel <%s>", buf);
    sprintf(sover.msgid, "C%s$%s@" MYHOSTNAME, board, filename);/* LHD.030628: �b�� msgid �[���N�r���@ cmsg �� Message-ID */
    sprintf(sover.control, "cancel <%s>", buf);
  }

  str_ncpy(sover.board, board, sizeof(sover.board));
  str_ncpy(sover.filename, filename, sizeof(sover.filename));
  sprintf(sover.from, "%s.bbs@" MYHOSTNAME " (%s)", bntp->owner, bntp->nick);
  str_ncpy(sover.date, Gtime(mtime), sizeof(sover.date));
  str_ncpy(sover.group, nf->newsgroup, sizeof(sover.group));
  str_ncpy(sover.charset, nf->charset, sizeof(sover.charset));
 
  queuefeed(nl, &sover);
}


static void
deal_bntp()
{
  char *OUTING = "innd/.outing";		/* �B�z�ɼȦs���� */
  int fd, i;
  nodelist_t *node;
  bntp_t bntp;

  if (rename("innd/out.bntp", OUTING))	/* �S���s�峹 */
    return;

  /* initail �U node �� feedfd */
  for (i = 0; i < NLCOUNT; i++)
  {
    node = NODELIST + i;
    node->feedfd = -1;
  }

  /* �K��U�ӯ��x���ݪ� *.link */
  if ((fd = open(OUTING, O_RDONLY)) >= 0)
  {
    while (read(fd, &bntp, sizeof(bntp_t)) == sizeof(bntp_t))
      deal_sover(&bntp);
    close(fd);
  }

  /* close �U node �� feedfd */
  for (i = 0; i < NLCOUNT; i++)
  {
    node = NODELIST + i;
    if (node->feedfd >= 0)
      close(node->feedfd);
  }

  unlink(OUTING);
}


/*-------------------------------------------------------*/
/* �s�h�Y�ӯ�						 */
/*-------------------------------------------------------*/


static int
inetclient(server, port)
  char *server;
  int port;
{
  struct hostent *host;		/* host information entry */
  struct sockaddr_in sin;	/* Internet endpoint address */
  int fd;

  if (!*server || !port)
    return -1;

  memset(&sin, 0, sizeof(sin));

  if (!(host = gethostbyname(server)))
    sin.sin_addr.s_addr = inet_addr(server);
  else
    memcpy(&sin.sin_addr.s_addr, host->h_addr, host->h_length);

  sin.sin_family = AF_INET;
  sin.sin_port = htons(port);

  /* Allocate a socket */
  fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (fd < 0)
    return -1;

  /* Connect the socket to the server */
  if (connect(fd, (struct sockaddr *) & sin, sizeof(sin)) < 0)
  {
    close(fd);
    return -1;
  }

  return fd;
}


static int
tcpcommand(char *fmt, ...)
{
  va_list args;
  char *ptr;

  va_start(args, fmt);
  vfprintf(SERVERwfp, fmt, args);
  va_end(args);
  fprintf(SERVERwfp, "\r\n");
  fflush(SERVERwfp);

  if (!fgets(SERVERbuffer, sizeof(SERVERbuffer), SERVERrfp))
    return 0;

  if (ptr = strchr(SERVERbuffer, '\r'))
    *ptr = '\0';
  if (ptr = strchr(SERVERbuffer, '\n'))
    *ptr = '\0';

  return atoi(SERVERbuffer);
}


static int			/* 200~202:���\ 0:���� */
open_connect(node)		/* �s�h�o�ӯ� */
  nodelist_t *node;
{
  char *host = node->host;
  int port = node->port;

  DEBUG(("�~<open_connect> ���b�}�ҳs�u\n"));

  if ((SERVERfd = inetclient(host, port)) < 0)
  {
    bbslog("<bbslink> :Err: ���A���s�u���ѡG%s %d\n", host, port);
    DEBUG(("��<open_connect> ���A���s�u����\n"));
    return 0;
  }

  if (!(SERVERrfp = fdopen(SERVERfd, "r")) || !(SERVERwfp = fdopen(SERVERfd, "w")))
  {
    bbslog("<bbslink> :Err: fdopen �o�Ϳ��~\n");
    DEBUG(("��<open_connect> fdopen �o�Ϳ��~\n"));
    return 0;
  }

  if (!fgets(SERVERbuffer, sizeof(SERVERbuffer), SERVERrfp) || SERVERbuffer[0] != '2')	/* 200 201 202 ������H */
  {
    bbslog("<bbslink> :Err: ���A���ڵ��s�u�G%s %d\n", host, port);
    DEBUG(("��<open_connect> ���A���ڵ��s�u\n"));
    return 0;
  }

  /* itoc.040512: MODE READER �u�n���@���N���F */
  if (node->xmode & INN_USEPOST)
  {
    tcpcommand("MODE READER");
    if (SERVERbuffer[0] != '2')	/* 200 201 202 ������H */
    {
      bbslog("<bbslink> :Err: ���A���ڵ��s�u�G%s %d\n", host, port);
      DEBUG(("��<open_connect> ���A���ڵ��s�u\n"));
      return 0;
    }
  }

  DEBUG(("�x<open_connect> ���A���s�u���\\\n"));
  return atoi(SERVERbuffer);
}


static void
close_connect()		/* �����s�h�o�ӯ� */
{
  int status;

  status = tcpcommand("QUIT");
  if (status != NNTP_GOODBYE_ACK_VAL && status != 221)
  {
    bbslog("<bbslink> :Warn: �L�k���`�_�u\n");
    DEBUG(("�x<close_connect> �L�k���`�_�u\n"));
  }

  DEBUG(("��<close_connect> �w�����s�u\n"));

  if (SERVERrfp)
    fclose(SERVERrfp);
  if (SERVERwfp)
    fclose(SERVERwfp);
  if (SERVERfd >= 0)
    close(SERVERfd);
}


/*-------------------------------------------------------*/
/* �e�X�峹						 */
/*-------------------------------------------------------*/


static int			/* -1:���� */
sover_post(sover)
  soverview_t *sover;
{
  if (sover->control[0])	/* �e�X cancel message */
  {
    static char BODY_BUF[128];

    sprintf(BODY_BUF, "%s\r\n", sover->title);
    BODY = BODY_BUF;	/* cancel message �ɡABODY ���V BODY_BUF */
  }
  else				/* �e�X�s�峹 */
  {
    static char *BODY_BUF;
    char *ptr, *str, fpath[64];
    int fd, size;
    struct stat st;

    /* �ˬd�峹�٦b���b */
    sprintf(fpath, "brd/%s/%c/%s", sover->board, sover->filename[7], sover->filename);
    if ((fd = open(fpath, O_RDONLY)) < 0)
      return -1;
    fstat(fd, &st);
    size = st.st_size;
    if (size <= 0)
    {
      close(fd);
      return -1;
    }

    /* �@��峹�ɡABODY ���V malloc �ͥX�Ӫ��϶� */

    BODY_BUF = !BODY_BUF ? (char *) malloc(size + 1) : (char *) realloc(BODY_BUF, size + 1);
    read(fd, BODY_BUF, size);
    close(fd);
    ptr = BODY_BUF + size;
    *ptr = '\0';

    /* ���L�峹���e�X�����Y���n */
    for (str = BODY_BUF;;str = ptr + 1)
    {
      ptr = strchr(str, '\n');
      if (!ptr)			/* ���峹�̫�F�٧䤣��Ŧ�A�������ɮ׳������� */
      {
	BODY = BODY_BUF;
	break;
      }

      if (ptr == str)		/* ���@��Ŧ�A����H�U�N���O����F */
      {
	BODY = str + 1;
	break;
      }
    }
  }

  if (sover->charset[0] == 'g')
  {
    b52gb(BODY);
    b52gb(sover->from);
    b52gb(sover->title);
  }

  return 0;
}


static void
fail_post(msgid)
  char *msgid;
{
  bbslog("<bbslink> :Warn: %s <%s>\n", SERVERbuffer, msgid);
  DEBUG(("�x��:Warn: %s <%s>\n", SERVERbuffer, msgid));
}


static void
send_outgoing(node, sover)
  nodelist_t *node;
  soverview_t *sover;
{
  int cc, status;
  char *msgid, *str;

  msgid = sover->msgid;

  DEBUG(("�x�z MSGID: %s\n", msgid));
  DEBUG(("�x�x GROUP: %s\n", sover->group));
  DEBUG(("�x�x FROM : %s\n", sover->from));
  DEBUG(("�x�| SUBJ : %s\n", sover->title));

  /* ����峹�ǳƦn */
  if (sover_post(sover) < 0)
  {
    DEBUG(("�x�� ���g�峹�w�D�R�����ɮ׿򥢡A�����e�X\n"));
    return;
  }

  /* �V server �e�X IHAVE/POST �n�D */
  if (node->xmode & INN_USEIHAVE)
  {
    status = tcpcommand("IHAVE <%s>", msgid);
    if (status != NNTP_SENDIT_VAL)
    {
      fail_post(msgid);
      return;
    }
  }
  else /* if (node->xmode & INN_USEPOST) */
  {
    status = tcpcommand("POST");
    if (status != NNTP_START_POST_VAL)
    {
      fail_post(msgid);
      return;
    }
  }
  
  /* �g�J�峹�����Y */
  fprintf(SERVERwfp, "Path: %s\r\n", MYBBSID);
  fprintf(SERVERwfp, "From: %s\r\n", sover->from);
  fprintf(SERVERwfp, "Newsgroups: %s\r\n", sover->group);
  /*fprintf(SERVERwfp, "Subject: %s\r\n", sover->title);*/ 
  output_rfc2047_qp(SERVERwfp, "Subject: ", sover->title, sover->charset, "\r\n");
  fprintf(SERVERwfp, "Date: %s\r\n", sover->date);
  fprintf(SERVERwfp, "Organization: %s\r\n", *sover->charset == 'b' ? BBSNAME : BBSNAME);	/* itoc.040425: �Y���O big5 �N�έ^�寸�W */
  fprintf(SERVERwfp, "Message-ID: <%s>\r\n", msgid);
  fprintf(SERVERwfp, "Mime-Version: 1.0\r\n");
  fprintf(SERVERwfp, "Content-Type: text/plain; charset=\"%s\"\r\n", sover->charset);
  fprintf(SERVERwfp, "Content-Transfer-Encoding: 8bit\r\n");
  if (sover->control[0])
    fprintf(SERVERwfp, "Control: %s\r\n", sover->control);
  fputs("\r\n", SERVERwfp);	/* ���Y�M����Ť@�� */

  /* �g�J�峹�����e */
  for (str = BODY; cc = *str; str++)
  {
    if (cc == '\n')
    {
      /* itoc.030127.����: �� "\n" ���� "\r\n" */
      fputc('\r', SERVERwfp);
    }
    else if (cc == '.')
    {
      /* If the text contained a period as the first character of the text 
         line in the original, that first period is doubled. */
      if (str == BODY || str[-1] == '\n')
        fputc('.', SERVERwfp);
    }
      
    fputc(cc, SERVERwfp);
	
  }

 
  /* IHAVE/POST ���� */
  status = tcpcommand(".");
  
  if (node->xmode & INN_USEIHAVE)
  {
    if (status != NNTP_TOOKIT_VAL)
      fail_post(msgid);
  }
  else /* if (node->xmode & INN_USEPOST) */
  {
    if (status != NNTP_POSTEDOK_VAL)
      fail_post(msgid);
  }
}


/*-------------------------------------------------------*/
/* �� news server �U���O				 */
/*-------------------------------------------------------*/


static int		/* 1:���\ 0:���� */
NNRPgroup(newsgroup, low, high)	/* ���� group�A�öǦ^ low-number �� high-number */
  char *newsgroup;
  int *low, *high;
{
  int i;
  char *ptr;

  if (tcpcommand("GROUP %s", newsgroup) != NNTP_GROUPOK_VAL)
    return 0;

  ptr = SERVERbuffer;

  /* �� SERVERbuffer ���ĤG�� ' ' */
  for (i = 0; i < 2; i++)
  {
    ptr++;
    if (!*ptr || !(ptr = strchr(ptr, ' ')))
      return 0;
  }
  if ((i = atoi(ptr + 1)) >= 0)
    *low = i;

  /* �� SERVERbuffer ���ĤT�� ' ' */
  ptr++;
  if (!*ptr || !(ptr = strchr(ptr, ' ')))
    return 0;
  if ((i = atoi(ptr + 1)) >= 0)
    *high = i;

  return 1;
}


static char *tempfile = "innd/bbslinktmp";

static int			/* 1:���\ 0:���� */
NNRParticle(artno)		/* ���^�� artno �g������ */
  int artno;
{
  FILE *fp;
  char *ptr;

  if (tcpcommand("ARTICLE %d", artno) != NNTP_ARTICLE_FOLLOWS_VAL)
    return 0;

  if (!(fp = fopen(tempfile, "w")))
    return 0;

  while (fgets(SERVERbuffer, sizeof(SERVERbuffer), SERVERrfp))
  {
    if (ptr = strchr(SERVERbuffer, '\r'))
      *ptr = '\0';
    if (ptr = strchr(SERVERbuffer, '\n'))
      *ptr = '\0';

    if (!strcmp(SERVERbuffer, "."))	/* �峹���� */
      break;

    fprintf(fp, "%s\n", SERVERbuffer);
  }

  fclose(fp);
  return 1;
}



#if 0	/* itoc.030109.����: my_post ���y�{ */
            �z�� receive_article() �� bbspost_add()
  my_post() �u�� receive_nocem()   �� �e�h nocem.c �B�z
            �|�� cancel_article()  �� bbspost_cancel()
#endif


static void
my_post()
{
  int rel, size;
  char *ptr, *data;
  struct stat st;

  if ((rel = open(tempfile, O_RDONLY)) >= 0)
  {
    fstat(rel, &st);
    size = st.st_size;
    data = (char *) malloc(size + 1);	/* �O�d 1 byte �� '\0' */
    size = read(rel, data, size);
    close(rel);

    if (size >= 2)
    {
      if (data[size - 2] == '\n')	/* ��̫᭫�Ъ� '\n' ���� '\0' */
        size--;
    }
    data[size] = '\0';		/* �ɤW '\0' */

    rel = readlines(data - 1);

    if (rel > 0)
    {
      if (ptr = CONTROL)
      {
	if (!str_ncmp(ptr, "cancel ", 7))
	  rel = cancel_article(ptr + 7);
      }
      else
      {
#ifdef _NoCeM_
	if (strstr(SUBJECT, "@@") && strstr(BODY, "NCM") && strstr(BODY, "PGP"))
	  rel = receive_nocem();
	else
#endif
	  rel = receive_article();
      }

      if (rel < 0)
      {
	DEBUG(("�x��<my_post> �����峹����\n"));
      }
    }
    else if (rel == 0)		/* PATH�]�A�ۤv */
    {
      DEBUG(("�x��<my_post> PATH �]�A�ۤv\n"));
    }
    else /* if (rel < 0) */	/* ���Y��줣���� */
    {
      DEBUG(("�x��<my_post> ���Y��줣����\n"));
    }

    free(data);
  }

  unlink(tempfile);
}


/*-------------------------------------------------------*/
/* ��s high-number					 */
/*-------------------------------------------------------*/


static int
nf_samegroup(nf)
  newsfeeds_t *nf;
{
  return !strcmp(nf->newsgroup, GROUP) && !strcmp(nf->path, NODENAME);
}


static void
changehigh(hdd, ram)
  newsfeeds_t *hdd, *ram;
{
  if (ram->high >= 0)
  {
    hdd->high = ram->high;
    hdd->xmode &= ~INN_ERROR;
  }
  else
  {
    hdd->xmode |= INN_ERROR;
  }
}


static void
updaterc(nf, pos, high)
  newsfeeds_t *nf;
  int pos;			/* �� newsfeeds.bbs �̭�����m */
  int high;			/* >=0:�ثe�����@�g <0:error */
{
  nf->high = high;
  GROUP = nf->newsgroup;
  rec_ref("innd/newsfeeds.bbs", nf, sizeof(newsfeeds_t), pos, nf_samegroup, changehigh);
}


/*-------------------------------------------------------*/
/* ����峹						 */
/*-------------------------------------------------------*/


static void
readnews(node)
  nodelist_t *node;
{
  int i, low, high, artcount, artno;
  char *name, *newsgroup;
  newsfeeds_t *nf;

  name = node->name;

  for (i = 0; i < NFCOUNT; i++)	/* �̧�Ū���C�� newsgroup */
  {
    nf = NEWSFEEDS + i;

    if (strcmp(name, nf->path))	/* �p�G���O�o�ӯ��x�N���L */
      continue;

    newsgroup = nf->newsgroup;

    DEBUG(("�x�z<readnews> �i�J %s\n", newsgroup));

    /* ���o news server �W�� low-number �� high-number */
    if (!NNRPgroup(newsgroup, &low, &high))
    {
      updaterc(nf, i, -1);
      DEBUG(("�x�|<readnews> �L�k���o���s�ժ� low-number �� high-number �Φ��s�դ��s�b\n"));
      continue;		/* ���s�դ��s�b�A���U�@�Ӹs�� */
    }

    if (ResetActive)
    {
      if (nf->high != high || nf->xmode & INN_ERROR)
        updaterc(nf, i, high);
      DEBUG(("�x�|<readnews> ���� %s�A���s�դ� high-number �w�P���A���P�B\n", newsgroup));
      continue;		/* �Y ResetActive �h�����H�A���U�@�Ӹs�� */
    }

    if (nf->high >= high)
    {
      if (nf->high > high || nf->xmode & INN_ERROR)	/* server re-number */
	updaterc(nf, i, high);

      DEBUG(("�x�|<readnews> ���� %s�A���s�դw�S���s�峹\n", newsgroup));
      continue;		/* �o�s�դw�S���s�峹�A���U�@�Ӹs�� */
    }

    if (nf->high < low - 1)				/* server re-number */
    {
      updaterc(nf, i, high);
      DEBUG(("�x�|<readnews> ���� %s�A���s�դ� high-number �]���A�����ʦӧ�s\n", newsgroup));
      continue;		/* �o�s���ܧ�L low-number�A���U�@�Ӹs�� */
    }

    /* ���^�s�դW�� nf->high + 1 �}�l�� MaxArts �g���峹 */

    artcount = 0;
    for (artno = nf->high + 1;; artno++)
    {
      if (NNRParticle(artno))
      {
	DEBUG(("�x�x<readnews> [%d] �����^�s�դW�� %d �g�峹\n", artcount, artno));
	my_post();
	if (++artcount >= MaxArts)
	  break;
      }
      if (artno >= high)
	break;
    }

    updaterc(nf, i, artno);

    DEBUG(("�x�|<readnews> ���� %s�A�@�@���^ %d �g�s�峹\n", newsgroup, artcount));
  }			/* end for() */
}


/*-------------------------------------------------------*/
/* lock/unlock �{���A�P�ɥu�঳�@�� bbslink �b�]	 */
/*-------------------------------------------------------*/


static char *lockfile = "innd/bbslinking";

static void
bbslink_un_lock()
{
  unlink(lockfile);
}


static int
bbslink_get_lock()
{
  int fd;
  char buf[10];

  if ((fd = open(lockfile, O_RDONLY)) >= 0)
  {
    int pid;
    struct stat st;

    /* lockfile �w�s�b�A�N�� bbslink ���b�] */

    if (read(fd, buf, sizeof(buf)) > 0 && (pid = atoi(buf)) > 0 && kill(pid, 0) == 0)
    {
      /* �p�G�d�Ӥ[�A�N�۰� kill �� */
      if (KillFormerProc || (!fstat(fd, &st) && st.st_mtime > time(NULL) + BBSLINK_EXPIRE))
      {
	kill(pid, SIGTERM);
      }
      else
      {
	DEBUG(("���t�~�@�� bbslink �� process [%d] ���b�B�@��\n", pid));
	return 0;
      }
    }

    close(fd);

    bbslink_un_lock();
  }

  sprintf(buf, "%d\n", getpid());
  f_cat(lockfile, buf);

  return 1;
}


/*-------------------------------------------------------*/
/* �D�{��						 */
/*-------------------------------------------------------*/


static void
visit_site(node)
  nodelist_t *node;
{
  int status, response, fd, num;
  char linkfile[64];
  soverview_t sover;

  NODENAME = node->name;

  /* �Y�����w�u�B�z�Y�S�w���A����N�u�B�z�ӯ��x */
  if (DefaultProcSite && strcmp(NODENAME, DefaultProcSite))
  {
    DEBUG(("�� �o�ëD�ҫ��w�n�B�z�����x�A�������L\n"));
    return;
  }

  status = 0;
  sprintf(linkfile, "innd/%s.link", NODENAME);
  if (dashf(linkfile))
    status ^= 0x01;
  if (!(node->xmode & INN_FEEDED))
    status ^= 0x02;

  if (!status)		/* ���ݭn�h���X��� */
  {
    DEBUG(("�� �����x�S���s�H�ݰe�B�Q���H�A���ݭn�h���X\n"));
    return;
  }

  if (!(response = open_connect(node)))		/* �s�u���� */
    return;

  if (status & 0x01)	/* ���s�H�ݰe */
  {
    if (response == NNTP_POSTOK_VAL)
    {
      /* �� linkfile �̭��ҰO���n�e���H�@�@�e�X */
      num = 0;
      if ((fd = open(linkfile, O_RDONLY)) >= 0)
      {
	while (read(fd, &sover, sizeof(soverview_t)) == sizeof(soverview_t))
	{
	  send_outgoing(node, &sover);
	  num++;
	}
	close(fd);
	unlink(linkfile);
      }
      DEBUG(("�x�� �`�@�e�X %d �g�峹\n", num));
    }
    else
    {
      DEBUG(("�x�� �S���b�����x�o��峹���v��\n"));
    }
  }
  else
  {
    DEBUG(("�x�� �S���s�H�ݰe\n"));
  }

  if (status & 0x02)	/* �ݭn�s�h���H */
  {
    readnews(node);
  }
  else
  {
    DEBUG(("�x�� �����x�]�w�Q���H�A���ݭn�h���H\n"));
  }

  close_connect();
}


static void
bbslink()
{
  int i;
  nodelist_t *node;

  /* �T����� */
  DEBUG(("�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w\n"));
  DEBUG(("�� nodelist.bbs �̭��@�@�� %d �ӯ��x�A���U�ӱN�@�@�h���X\n", NLCOUNT));
  DEBUG(("�� �ѼƳ]�w�G\n"));
  DEBUG(("   (1) �R���W�����楢�Ѫ� bbslink�G%s\n", KillFormerProc ? "�O" : "�_"));
  DEBUG(("   (2) �N high-number ��s��P news server �W�ۦP�G%s\n", ResetActive ? "�O" : "�_"));
  DEBUG(("   (3) �� news server �C�Ӹs�ճ̦h�u�� %d �ʤ峹\n", MaxArts));
  DEBUG(("   (4) �u�B�z�Y�S�w���x�άO�B�z�Ҧ����x�G%s\n", DefaultProcSite ? DefaultProcSite : "�B�z�Ҧ����x"));

  DEBUG(("�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w\n"));
  DEBUG(("�� �}�l�B�z out.bntp�A��z�n�e�X�h���峹\n"));
  deal_bntp();
  DEBUG(("�� out.bntp ��z����\n"));
  DEBUG(("�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w\n"));

  /* �� nodelist.bbs �����Ҧ����x���h���X�@�M */
  for (i = 0; i < NLCOUNT; i++)
  {
    node = NODELIST + i;
    DEBUG(("�� [%d] �}�l���X <%s> %s (%d)\n", i + 1, node->name, node->host, node->port));
    visit_site(node);
    DEBUG(("�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w\n"));
  }
}


static void
usage(argv)
  char *argv;
{
  printf("Usage: %s [options]\n", argv);
  printf("       -c  �N high-number �P���A���W�P�B(�����H)\n");
  printf("       -k  �屼�ثe���b�]�� bbslink�A�í��s�Ұ� bbslink\n");
  printf("       -v  ��ܸԲӪ��s�u�L�{\n");
  printf("       -a ######  ���w�C�Ӹs�ճ̦h���X�ʫH(�w�] %d ��)\n", MAX_ARTS);
  printf("       -s site    �u���o�ӯ��x���峹\n");
}


int
main(argc, argv)
  int argc;
  char *argv[];
{
  int c, errflag = 0;

  chdir(BBSHOME);
  umask(077);

  while ((c = getopt(argc, argv, "a:s:ckv")) != -1)
  {
    switch (c)
    {
    case 'a':
      if ((c = atoi(optarg)) > 0)
	MaxArts = c;
      break;

    case 's':
      DefaultProcSite = optarg;
      break;

    case 'c':
      ResetActive = 1;
      break;

    case 'k':
      KillFormerProc = 1;
      break;

    case 'v':
      Verbose = 1;
      break;

    default:
      errflag++;
      break;
    }
  }

  if (errflag > 0)
  {
    usage(argv[0]);
    return -1;
  }

  /* �}�l bbslink�A�N bbslink ��� */
  if (!bbslink_get_lock())
    return -1;

  init_bshm();

  if (initial_bbs())
    bbslink();

  /* ���� bbslink�A�N bbslink �Ѷ} */
  bbslink_un_lock();

  return 0;
}
