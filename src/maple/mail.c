/*-------------------------------------------------------*/
/* mail.c	( NTHU CS MapleBBS Ver 2.36 )		 */
/*-------------------------------------------------------*/
/* target : local/internet mail routines	 	 */
/* create : 95/03/29				 	 */
/* update : 95/12/15				 	 */
/*-------------------------------------------------------*/

#include "bbs.h"

extern int xo_delete();
extern int xo_uquery();
extern int xo_usetup();
extern int post_cross();
extern XZ xz[];

extern int TagNum;
extern UCACHE *ushm;
//static int m_count();


/* ----------------------------------------------------- */
/* Link List routines					 */
/* ----------------------------------------------------- */


#define	MSG_CC "\033[32m[�s�զW��]\033[m\n"


LinkList *ll_head;		/* head of link list */
static LinkList *ll_tail;	/* tail of link list */
extern BCACHE *bshm;


void
ll_new()
{
  LinkList *list, *next;

  list = ll_head;

  while (list)
  {
    next = list->next;
    free(list);
    list = next;
  }

  ll_head = ll_tail = NULL;
}


void
ll_add(name)
  char *name;
{
  LinkList *node;
  int len;

  len = strlen(name) + 1;
  node = (LinkList *) malloc(sizeof(LinkList) + len);
  node->next = NULL;
  strcpy(node->data, name);

  if (ll_head)
    ll_tail->next = node;
  else
    ll_head = node;
  ll_tail = node;
}


int
ll_del(name)
  char *name;
{
  LinkList *list, *prev, *next;

  prev = NULL;
  for (list = ll_head; list; list = next)
  {
    next = list->next;
    if (!strcmp(list->data, name))
    {
      if (prev == NULL)
	ll_head = next;
      else
	prev->next = next;

      if (list == ll_tail)
	ll_tail = prev;

      free(list);
      return 1;
    }
    prev = list;
  }
  return 0;
}


int
ll_has(name)
  char *name;
{
  LinkList *list;

  for (list = ll_head; list; list = list->next)
  {
    if (!strcmp(list->data, name))
      return 1;
  }
  return 0;
}


void
ll_out(row, column, msg)
  int row, column;
  char *msg;
{
  LinkList *list;
  int cur_rownum,ch,crow;
  

  move(row, column);
  clrtobot();
  outs(msg);
  cur_rownum = row;

  column = 80;
  for (list = ll_head; list; list = list->next)
  {
    msg = list->data;
    crow = strlen(msg) + 1;
    if (column + crow > 78)
    {
      column = crow;
      outc('\n');
      cur_rownum++;
      if(cur_rownum >= b_lines-1)
      {
        outs("�� �C��(C)�~�� (Q)���� ? [C]");
        ch = vkey();
        if(ch == 'q' || ch == 'Q')
          break;
        else
        {
          move(row+2, 0);
          clrtobot();
          cur_rownum = row+1;          
        }
      }
    }
    else
    {
      column += crow;
      outc(' ');
    }
    outs(msg);
  }
}


/* ----------------------------------------------------- */
/* Internet E-mail routines				 */
/* ----------------------------------------------------- */


int mail_send();


int
m_internet()
{
  char rcpt[60];

  if(bbsothermode & OTHERSTAT_EDITING)
  {
    vmsg("�A�٦��ɮ��٨S�s���@�I");
    return 0;
  }


  if(mail_stat(CHK_MAIL_NOMSG))
  {
     vmsg("�A���H�c�e�q�W�L�W���A�о�z�I");
     chk_mailstat = 1;
     return 0;
  }
  else
     chk_mailstat = 0;

//  if (m_count())
//    return 0;

  while (vget(20, 0, "���H�H�G", rcpt, sizeof(rcpt), DOECHO))
  {
    if (not_addr(rcpt))
    {
      vmsg("E-mail �����T");
      continue;
    }

    if (vget(21, 0, "�D  �D�G", ve_title, TTLEN, DOECHO))
    {
#if 1
      char *msg;
      switch(mail_send(rcpt, ve_title))
      {
      case -1:
        msg = err_uid;
        break;

      case -2:
        msg = msg_cancel;
        break;

      case -3:  /* Thor.980707: �������p�� ?*/
        msg = "�ϥΪ̵L�k���H";
        break;

      default:
        msg = "�H�w�H�X"; 
        break;
      }
      vmsg(msg);
#endif
    }
    break;
  }
  return 0;
}


/* ----------------------------------------------------- */
/* BBS (batch) SMTP					 */
/* ----------------------------------------------------- */


#ifdef	BATCH_SMTP
int
bsmtp(fpath, title, rcpt, method)
  char *fpath, *title, *rcpt;
  int method;
{
  char buf[80];
  time_t chrono;
  MailQueue mqueue;

  chrono = time(NULL);

  /* ���~�d�I */

  if (method != MQ_JUSTIFY)
  {
    /* stamp the queue file */

    strcpy(buf, "out/");

    for (;;)
    {
      archiv32(chrono, buf + 4);
      if (!f_ln(fpath, buf))
	break;
      chrono++;
    }
    fpath = buf;

    strcpy(mqueue.filepath, fpath);
    strcpy(mqueue.subject, title);
  }

  /* setup mail queue */

  mqueue.mailtime = chrono;
  mqueue.method = method;
  strcpy(mqueue.sender, cuser.userid);
  strcpy(mqueue.username, cuser.username);
  strcpy(mqueue.rcpt, rcpt);
  if (rec_add(MAIL_QUEUE, &mqueue, sizeof(mqueue)) < 0)
    return -1;

  cuser.numemail++;		/* �O���ϥΪ̦@�H�X�X�� Internet E-mail */
  return chrono;
}

#else

/* ----------------------------------------------------- */
/* (direct) SMTP					 */
/* ----------------------------------------------------- */


int
bsmtp(fpath, title, rcpt, method)
  char *fpath, *title, *rcpt;
  int method;
{
  int sock;
  time_t chrono, stamp;
  FILE *fp, *fr, *fw;
  char *str, buf[512], from[80], subject[80], msgid[80], keyfile[80], valid[10];
#ifdef HAVE_SIGNED_MAIL
  char prikey[9];
  union{
    char str[9];
    struct {
      unsigned int hash, hash2;
    } val;
  } sign;

  *prikey = prikey[8] = sign.str[8] = '\0'; /* Thor.990413:����: �r�굲�� */
#endif

  cuser.numemail++;		/* �O���ϥΪ̦@�H�X�X�� Internet E-mail */
  chrono = time(&stamp);

  /* --------------------------------------------------- */
  /* �����{�ҫH��					 */
  /* --------------------------------------------------- */

  if (method == MQ_JUSTIFY)
  {
    fpath = FN_ETC_VALID;
    title = subject;
    /* Thor.990125: MYHOSTNAME�Τ@��J str_host */
    sprintf(from, "bbsreg@%s", str_host);
    archiv32(str_hash(rcpt, chrono), buf);
    /* sprintf(title, "[MapleBBS]To %s(%s) [VALID]", cuser.userid, buf); */
    /* Thor.981012: �����b config.h �޲z */
    sprintf(title, TAG_VALID"%s(%s) [VALID]", cuser.userid, buf);

    usr_fpath(keyfile, cuser.userid, FN_REGKEY);
    if (fp = fopen(keyfile, "w"))
    {
      fprintf(fp, "%s\n", buf);
      fclose(fp);
    }
    strncpy(valid,buf,10);

  }
  else
  {
    /* Thor.990125: MYHOSTNAME�Τ@��J str_host */
    sprintf(from, "%s.bbs@%s", cuser.userid, str_host);
  }

#ifdef HAVE_SMTP_SERVER
  {
    int i;
    char *alias[] = SMTP_SERVER;
    for( i=0 ; (str = alias[i]) ; i++)
    {
      sock = dns_open(str, 25);
      if(sock >= 0)
        break;
    }
    if(sock < 0)
    {
      str = strchr(rcpt, '@') + 1;
      sock = dns_smtp(str);
    }
  }
#else
  str = strchr(rcpt, '@') + 1;
  sock = dns_smtp(str);
#endif

  if (sock >= 0)
  {
    archiv32(chrono, msgid);

    move(b_lines, 0);
    clrtoeol();
    
    if(method)
    {
      prints("�� �H�H�� %s \033[5m...\033[m", rcpt);
      refresh();
    }
    else
    {
      prints("�� �H�H�� \033[5m...\033[m");
      refresh();
    }

    sleep(1);			/* wait for mail server response */

    fr = fdopen(sock, "r");
    fw = fdopen(sock, "w");

    fgets(buf, sizeof(buf), fr);
    if (memcmp(buf, "220", 3))
      goto smtp_error;
    while (buf[3] == '-') /* maniac.bbs@WMStar.twbbs.org 2000.04.18 */
      fgets(buf, sizeof(buf), fr);

    /* Thor.990125: MYHOSTNAME�Τ@��J str_host */
    fprintf(fw, "HELO %s\r\n", str_host);
    fflush(fw);
    do
    {
      fgets(buf, sizeof(buf), fr);
      if (memcmp(buf, "250", 3))
	goto smtp_error;
    } while (buf[3] == '-');

    fprintf(fw, "MAIL FROM:<%s>\r\n", from);
    fflush(fw);
    do
    {
      fgets(buf, sizeof(buf), fr);
      if (memcmp(buf, "250", 3))
	goto smtp_error;
    } while (buf[3] == '-');

    fprintf(fw, "RCPT TO:<%s>\r\n", rcpt);
    fflush(fw);
    do
    {
      fgets(buf, sizeof(buf), fr);
      if (memcmp(buf, "250", 3))
	goto smtp_error;
    } while (buf[3] == '-');

/*    fprintf(fw, "DATA\r\n", rcpt);*/ /* statue.000713 */
    fprintf(fw, "DATA\r\n");
    fflush(fw);
    do
    {
      fgets(buf, sizeof(buf), fr);
      if (memcmp(buf, "354", 3))
	goto smtp_error;
    } while (buf[3] == '-');

    /* ------------------------------------------------- */
    /* begin of mail header				 */
    /* ------------------------------------------------- */

    /* Thor.990125: ���i�઺�� RFC822 & sendmail���@�k, �K�o�O�H����:p */
    fprintf(fw, "From: %s\r\nTo: %s\r\nSubject: %s\r\nX-Sender: %s (%s)\r\n"
      "Date: %s\r\nMessage-Id: <%s@%s>\r\n"
      "X-Disclaimer: [%s] �糧�H���e�����t�d\r\n\r\n",
      from, rcpt, title, cuser.userid, cuser.username, 
      Atime(&stamp), msgid, str_host, 
      str_site);

    if (method & MQ_JUSTIFY)	/* �����{�ҫH�� */
    {
      fprintf(fw, " ID: %s (%s)  E-mail: %s\r\n\r\n",
	cuser.userid, cuser.username, rcpt);
    }

    /* ------------------------------------------------- */
    /* begin of mail body				 */
    /* ------------------------------------------------- */

    if ((fp = fopen(fpath, "r")))
    {
      char *ptr;

      str = buf;
      *str++ = '.';
      while (fgets(str, sizeof(buf) - 3, fp))
      {
	if ((ptr = strchr(str, '\n')))
	{
	  *ptr++ = '\r';
	  *ptr++ = '\n';
	  *ptr = '\0';
	}
	fputs((*str == '.' ? buf : str), fw);
      }
      fclose(fp);
    }
#ifdef HAVE_SIGNED_MAIL
    if(!(method & MQ_JUSTIFY) && !rec_get(PRIVATE_KEY, prikey, 8, 0))
    /* Thor.990413: ���F�{�Ҩ�~, ��L�H�󳣭n�[sign */
    {
      /* Thor.990413: buf�Τ���F, �ɨӥΥ� :P */
      sprintf(buf,"%s -> %s", cuser.userid, rcpt);
      sign.val.hash = str_hash(buf, stamp);
      sign.val.hash2 = str_hash2(buf, sign.val.hash);
      str_xor(sign.str, prikey);
      /* Thor.990413: ���[()����, �ɶ����ťշ|�Q�Y��(���Ү�) */
      fprintf(fw,"\033[1;32m�� X-Info: \033[33m%s\033[m\r\n\033[1;32m�� X-Sign: \033[36m%s%s \033[37m(%s)\033[m\r\n", 
                  buf, msgid, genpasswd(sign.str), Btime(&stamp));
    }
#endif
    fputs("\r\n.\r\n", fw);
    fflush(fw);

    fgets(buf, sizeof(buf), fr);
    if (memcmp(buf, "250", 3))
      goto smtp_error;

    fputs("QUIT\r\n", fw);
    fflush(fw);
    fclose(fw);
    fclose(fr);
    goto smtp_log;

smtp_error:

    fclose(fr);
    fclose(fw);
    sprintf(msgid + 7, "\n\t%.70s", buf);
    chrono = -1;
  }
  else
  {
    chrono = -1;
    strcpy(msgid, "CONN");
  }

smtp_log:

  /* --------------------------------------------------- */
  /* �O���H�H						 */
  /* --------------------------------------------------- */

  sprintf(buf, "%s%-13s%c> %s %s %s\n\t%s\n\t%s\n", Btime(&stamp), cuser.userid,
    ((method == MQ_JUSTIFY) ? '=' : '-'), rcpt, msgid, 
#ifdef HAVE_SIGNED_MAIL
      *prikey ? genpasswd(sign.str): "NoPriKey",
#else
      "",
#endif
      title, fpath);
  f_cat(FN_MAIL_LOG, buf);

  return chrono;
}
#endif

#ifdef HAVE_DOWNLOAD
int
bsmtp_file(fpath, title, rcpt)
  char *fpath, *title, *rcpt;
{
  int sock;
  time_t chrono, stamp;
  FILE *fp, *fr, *fw;
  char *str, buf[512], from[80], msgid[80],boundry[256];
  char fname[256];
  struct tm ntime, *xtime;
            
  

  cuser.numemail++;		/* �O���ϥΪ̦@�H�X�X�� Internet E-mail */
  chrono = time(&stamp);
  xtime = localtime(&chrono);
  ntime = *xtime;
  
  sprintf(fname,"mail_%04d%02d%02d.tgz",ntime.tm_year + 1900, ntime.tm_mon + 1, ntime.tm_mday);

  /* --------------------------------------------------- */
  /* �����{�ҫH��					 */
  /* --------------------------------------------------- */

  /* Thor.990125: MYHOSTNAME�Τ@��J str_host */
  sprintf(from, "%s.bbs@%s", cuser.userid, str_host);

#ifdef HAVE_SMTP_SERVER
  {
    int i;
    char *alias[] = SMTP_SERVER;
    for( i=0 ; (str = alias[i]) ; i++)
    {
      sock = dns_open(str, 25);
      if(sock >= 0)
        break;
    }
    if(sock < 0)
    {
      str = strchr(rcpt, '@') + 1;
      sock = dns_smtp(str);
    }
  }
#else
  str = strchr(rcpt, '@') + 1;
  sock = dns_smtp(str);
#endif

  if (sock >= 0)
  {
    archiv32(chrono, msgid);
    
    sprintf(boundry,"----=_NextPart_%s",msgid);

    move(b_lines, 0);
    clrtoeol();
    
    prints("�� �H�H�� %s \033[5m...\033[m", rcpt);
    refresh();

    sleep(1);			/* wait for mail server response */

    fr = fdopen(sock, "r");
    fw = fdopen(sock, "w");

    fgets(buf, sizeof(buf), fr);
    if (memcmp(buf, "220", 3))
      goto smtp_file_error;
    while (buf[3] == '-') /* maniac.bbs@WMStar.twbbs.org 2000.04.18 */
      fgets(buf, sizeof(buf), fr);

    /* Thor.990125: MYHOSTNAME�Τ@��J str_host */
    fprintf(fw, "HELO %s\r\n", str_host);
    fflush(fw);
    do
    {
      fgets(buf, sizeof(buf), fr);
      if (memcmp(buf, "250", 3))
	goto smtp_file_error;
    } while (buf[3] == '-');

    fprintf(fw, "MAIL FROM:<%s>\r\n", from);
    fflush(fw);
    do
    {
      fgets(buf, sizeof(buf), fr);
      if (memcmp(buf, "250", 3))
	goto smtp_file_error;
    } while (buf[3] == '-');

    fprintf(fw, "RCPT TO:<%s>\r\n", rcpt);
    fflush(fw);
    do
    {
      fgets(buf, sizeof(buf), fr);
      if (memcmp(buf, "250", 3))
	goto smtp_file_error;
    } while (buf[3] == '-');

    fprintf(fw, "DATA\r\n");
    fflush(fw);
    do
    {
      fgets(buf, sizeof(buf), fr);
      if (memcmp(buf, "354", 3))
	goto smtp_file_error;
    } while (buf[3] == '-');

    /* ------------------------------------------------- */
    /* begin of mail header				 */
    /* ------------------------------------------------- */

    /* Thor.990125: ���i�઺�� RFC822 & sendmail���@�k, �K�o�O�H����:p */
    fprintf(fw, "From: %s\r\nTo: %s\r\nSubject: %s\r\nX-Sender: %s (%s)\r\n"
      "Date: %s\r\nMessage-Id: <%s@%s>\r\n"
      "X-Disclaimer: [%s] �糧�H���e�����t�d\r\n",
      from, rcpt, title, cuser.userid, cuser.username, 
      Atime(&stamp), msgid, str_host, 
      str_site);
      
    fprintf(fw,"MIME-Version: 1.0\r\nContent-Type: multipart/mixed;\r\n"
    		"\tboundary=\"%s\"\r\n\r\n",boundry);
    		
    fprintf(fw,"This is a multi-part message in MIME format.\r\n");
    fprintf(fw,"--%s\r\nContent-Type: text/plain;\r\n\tcharset=\"big5\"\r\n"
    	       "Content-Transfer-Encoding: 8bit\r\n\r\n����W�١G%s\r\n",boundry,fname);

    fprintf(fw,"--%s\r\nContent-Type: application/x-compressed;\r\n\tname=\"%s\"\r\n"
    	       "Content-Transfer-Encoding: base64\r\nContent-Disposition: attachment;\r\n"
    	       "\tfilename=\"%s\"\r\n\r\n",boundry,fname,fname);

    /* ------------------------------------------------- */
    /* begin of mail body				 */
    /* ------------------------------------------------- */

    if ((fp = fopen(fpath, "r")))
    {
      char *ptr;

      str = buf;
      *str++ = '.';
      while (fgets(str, sizeof(buf) - 3, fp))
      {
	if ((ptr = strchr(str, '\n')))
	{
	  *ptr++ = '\r';
	  *ptr++ = '\n';
	  *ptr = '\0';
	}
	fputs((*str == '.' ? buf : str), fw);
      }
      fclose(fp);
    }
    fprintf(fw,"--%s--\r\n",boundry);

    fputs("\r\n.\r\n", fw);
    fflush(fw);

    fgets(buf, sizeof(buf), fr);
    if (memcmp(buf, "250", 3))
      goto smtp_file_error;

    fputs("QUIT\r\n", fw);
    fflush(fw);
    fclose(fw);
    fclose(fr);
    goto smtp_file_log;

smtp_file_error:

    fclose(fr);
    fclose(fw);
    sprintf(msgid + 7, "\n\t%.70s", buf);
    chrono = -1;
  }
  else
  {
    chrono = -1;
    strcpy(msgid, "CONN");
  }

smtp_file_log:

  /* --------------------------------------------------- */
  /* �O���H�H						 */
  /* --------------------------------------------------- */

  sprintf(buf, "%s%-13s> %s %s\n\t%s\n\t%s\n", Btime(&stamp), cuser.userid,
    rcpt, msgid, title, fpath);
  f_cat(FN_MAIL_LOG, buf);

  return chrono;
}

#endif

#ifdef HAVE_SIGNED_MAIL
/* Thor.990413: �������ҥ\�� */
int
m_verify()
{
  extern char rusername[]; /* Thor: �O�� RFC931, ��guest verify */
  time_t chrono; 
  char info[79], *p;
  char sign[79], *q;
  char buf[160];

  char prikey[9];
  union{
    char str[9];
    struct {
      unsigned int hash, hash2;
    } val;
  } s;

  prikey[8] = s.str[8] = '\0'; /* Thor.990413:����: �r�굲�� */

  if(rec_get(PRIVATE_KEY, prikey, 8, 0))
  {
    zmsg("���t�ΨõL�q�lñ���A�Ь�SYSOP");
    return XEASY;
  }

  move(13, 0); 
  clrtobot();
  move(15, 0); 
  outs("�Ш̧ǿ�J�H����� X-Info X-Sign �H�i������");

  if(!vget(17, 0, ":", info, sizeof info, DOECHO) ||
     !vget(18, 0, ":", sign, sizeof sign, DOECHO))
    return 0;

  str_trim(info); /* Thor: �h����, for ptelnet�۰ʥ[�ť� */
  str_trim(sign);

  if(!memcmp("�� X-Info: ", p = info, 11))
    p += 11;
  while(*p == ' ') p++; /* Thor: �h�e�Y */

  if(!memcmp("�� X-Sign: ", q = sign, 11))
    q += 11;
  while(*q == ' ') q++;

  if(strlen(q) < 7 + 13) 
  {
    vmsg("�q�lñ�����~");
    return 0;
  }
      
  str_ncpy(s.str + 1, q, 8);  /* Thor: �ȭɤ@�U s.str */
  chrono = chrono32(s.str); /* prefix 1 char */ 

  q += 7; /* real sign */
  q[PASSLEN-1] = 0; /* �ɧ�0 */

  s.val.hash = str_hash(p, chrono);
  s.val.hash2 = str_hash2(p, s.val.hash);
  str_xor(s.str, prikey);

  sprintf(buf,"(%s)", Btime(&chrono));

  if(chkpasswd(q, s.str) || strcmp(q + PASSLEN, buf)) 
  { 
    /* Thor.990413: log usage */
    sprintf(buf,"%s@%s - XInfo:%s", rusername, fromhost, p);
    blog("VRFY",buf);
    /* Thor: fake sign */
    move(20, 25);
    outs("\033[41;37;5m *�`�N* ���ҿ��~! \033[m");
    vmsg("���H�ëD�ѥ����ҵo�A�Ьd��");
    return 0;
  }

  sprintf(buf,"%s@%s + XInfo:%s", rusername, fromhost, p);
  blog("VRFY",buf);

  vmsg("���H�ѥ����ҵo�X");
  return 0;
}
#endif

/* ----------------------------------------------------- */
/* mail routines					 */
/* ----------------------------------------------------- */

static struct
{
  XO mail_xo;
  char dir[32];
}      cmbox;

int
m_total_size()
{
  int fd, fsize,total;
  struct stat st;
  HDR *head, *tail;
  char *base, *folder, fpath[80];
  int changed;

  if ((fd = open(folder = cmbox.dir, O_RDWR)) < 0)
    return 0;

  fsize = 0;
  total = 0;
  changed = 0;

  if (!fstat(fd, &st) && (fsize = st.st_size) >= sizeof(HDR) &&
    (base = (char *) malloc(fsize)))
  {
   
    f_exlock(fd);

    if ((fsize = read(fd, base, fsize)) >= sizeof(HDR))
    {
      head = (HDR *) base;
      tail = (HDR *) (base + fsize);

      do
      { 
        if(head->xid > 0)
        {
          total += head->xid;
        }
        else
        {
          hdr_fpath(fpath, folder, head);
          stat(fpath, &st);
          total += st.st_size;
          head->xid = st.st_size;
          changed = 1;
        }
      } while (++head < tail);

    }
    
    if(changed == 1)
    {
      lseek(fd, (off_t) 0, SEEK_SET);
      write(fd, base, fsize);
      ftruncate(fd, fsize);
    }
    f_unlock(fd);

    free(base);
  }

  close(fd);

  return total;
}


usint
m_quota()
{
  usint ufo;
  int fd, count, fsize, limit, xmode;
  time_t mail_due, mark_due;
  struct stat st;
  HDR *head, *tail;
  char *base, *folder, date[9];

  if ((fd = open(folder = cmbox.dir, O_RDWR)) < 0)
    return 0;

  ufo = 0;
  fsize = 0;

  if (!fstat(fd, &st) && (fsize = st.st_size) >= sizeof(HDR) &&
    (base = (char *) malloc(fsize)))
  {

    /* flock(fd, LOCK_EX); */
    /* Thor.981205: �� fcntl ���Nflock, POSIX�зǥΪk */
    f_exlock(fd);

    if ((fsize = read(fd, base, fsize)) >= sizeof(HDR))
    {
      int prune;		/* number of pruned mail */

      limit = time(0);
      mail_due = limit - MAIL_DUE * 86400;
      mark_due = limit - MARK_DUE * 86400;
      st.st_mtime = limit + CHECK_PERIOD;
      str_stamp(date, &st.st_mtime);

      limit = cuser.userlevel;
      if (limit & (PERM_SYSOP | PERM_MBOX))
	limit = MAX_BBSMAIL;
      else
	limit = limit & PERM_VALID ? MAX_VALIDMAIL : MAX_NOVALIDMAIL;

      count = fsize / sizeof(HDR);

      head = (HDR *) base;
      tail = (HDR *) (base + fsize);

      prune = 0;
#if 1
      do
      {
	xmode = head->xmode;

#if 0
	if (xmode & MAIL_DELETE)
	{
	  char fpath[64];

	  hdr_fpath(fpath, folder, head);
	  unlink(fpath);
	  prune--;
	  continue;
	}
#endif
	if (!(xmode & MAIL_READ))
	  ufo |= UFO_BIFF;
	  
#if 0
	if ((count > limit) ||
	  (head->chrono <= (xmode & MAIL_MARKED ? mark_due : mail_due)))
	{
	  count--;
	  head->xmode = xmode | MAIL_DELETE;
	  strcpy(head->date, date);
	  ufo |= UFO_MQUOTA;
	}
#else
          head->xmode = xmode & (~MAIL_DELETE);
#endif
	if (prune)
	  head[prune] = head[0];

      } while (++head < tail);
#endif
      fsize += (prune * sizeof(HDR));
#if 0
      if ((fsize > 0) && (prune || (ufo & UFO_MQUOTA)))
      {
	lseek(fd, 0, SEEK_SET);
	write(fd, base, fsize);
	ftruncate(fd, fsize);
      }
      
#else
      ufo &= ~UFO_MQUOTA;
      if (fsize > 0)
      {
        lseek(fd, 0, SEEK_SET);
        write(fd, base, fsize);
        ftruncate(fd, fsize);
      }
#endif
      
    }

    /* flock(fd, LOCK_UN); */
    /* Thor.981205: �� fcntl ���Nflock, POSIX�зǥΪk */
    f_unlock(fd);

    free(base);
  }

  close(fd);
  if (fsize < sizeof(HDR))
    unlink(folder);

  return ufo;
}

#ifdef	HAVE_DOWNLOAD
int
m_zip()
{
  char cmd[512],fpath[128],user[128],title[256];
  char buf[IDLEN+1];

  if(strstr(cuser.email,".bbs@"MYHOSTNAME))
  { 
     vmsg("�ϥε��U��{�ҳq�L���ϥΪ̵L�k���]�I");
  }
  else
  {
    sprintf(cmd,"�O�_�n���]�ӤH�H�c�� %s�G[N/y]",cuser.email);
    if(vans(cmd) == 'y')
    {
      str_lower(buf,cuser.userid);
      move(b_lines, 0);
      clrtoeol();
        
      prints("�� �ɮ����Y�� \033[5m...\033[m");
      refresh();
                    
      sprintf(user,"%s/.DIR %s/@/",buf,buf);
      sprintf(fpath,"tmp/%s.b64",cuser.userid);

      sprintf(cmd,"tar -C usr/%c -zcf - %s | bin/base64encode > %s",*buf,user,fpath);
      system(cmd);
  
      sprintf(title,"�i%s�j%s �ӤH�H�c", BOARDNAME, cuser.userid);
      bsmtp_file(fpath,title,cuser.email);
    }
  }
  return 0;
}
#endif

int
m_query(userid)
  char *userid;
{
  int fd, ans, fsize;
  HDR *head, *tail;
  char folder[64];
  struct stat st;

  ans = 0;
  usr_fpath(folder, userid, fn_dir);
  if ((fd = open(folder, O_RDONLY)) >= 0)
  {
    fsize = 0;

    if (!fstat(fd, &st) && (fsize = st.st_size) >= sizeof(HDR) &&
      (head = (HDR *) malloc(fsize)))
    {
      if ((fsize = read(fd, head, fsize)) >= sizeof(HDR))
      {
	tail = (HDR *) ((char *) head + fsize);

	while (--tail >= head)
	{
	  if (!(tail->xmode & MAIL_READ))
	  {
	    ans++;
	    /*break;*/
	  }
	}
      }
      free(head);
    }

    close(fd);
    if (fsize < sizeof(HDR))
      unlink(folder);
  }

  return ans;
}


void
m_biff(userno)
  int userno;
{
  UTMP *utmp, *uceil;

  utmp = ushm->uslot;
  uceil = (void *) utmp + ushm->offset;
  do
  {
    if (utmp->userno == userno)
    {
      utmp->ufo |= UFO_BIFF;

#ifdef	BELL_ONCE_ONLY
      return;
#endif
    }
  } while (++utmp <= uceil);
}

#if 0
static int
m_count()
{
  int quota;
  usint ulevel;
  struct stat st;

  ulevel = cuser.userlevel;

  /* Thor.980806: ����: DENYMAIL�b�g��Ȥ����ɷ|�blogin�۰ʳ]�w */
  if (ulevel & PERM_DENYMAIL)
  {
    vmsg("�z���H�c�Q��F�I");
    return 1;
  }

  if (stat(cmbox.dir, &st))
    return 0;

  if (ulevel & (PERM_SYSOP | PERM_ACCOUNTS | PERM_MBOX))
    quota = MAX_BBSMAIL * sizeof(HDR);
  else if (ulevel & PERM_VALID)
    quota = MAX_VALIDMAIL * sizeof(HDR);
  else
    quota = MAX_NOVALIDMAIL * sizeof(HDR);

  if (st.st_size <= quota)
    return 0;

  more(FN_ETC_MAIL_OVER, NULL);
  return 1;
}
#endif

static void
mail_hold(fpath, rcpt)
  char *fpath;
  char *rcpt;
{
  char *title, *folder, buf[256];
  HDR mhdr;

  if (vans("�O�_�ۦs���Z(Y/N)�H[N] ") != 'y')
    return;

  folder = cmbox.dir;
  hdr_stamp(folder, HDR_LINK, &mhdr, fpath);

  mhdr.xmode = MAIL_READ | MAIL_HOLD /* | MAIL_NOREPLY */ ;
  strcpy(mhdr.owner, "[�� �� ��]");
  strcpy(mhdr.nick, cuser.username);
  title = ve_title;
  if (rcpt)
  {
    sprintf(buf, "<%s> %s", rcpt, title);
    title = buf;
    title[TTLEN] = '\0';
  }
  strcpy(mhdr.title, title);
  rec_add(folder, &mhdr, sizeof(HDR));
}

/* cache.091209: �۰���H */ 
int
m_setforward()
{
  char fpath[64], ip[50];
  FILE *fp;
                                                                                
  usr_fpath(fpath, cuser.userid, FN_FORWARD);
  if (fp = fopen(fpath, "r"))
  {
    fscanf(fp, "%s", ip);
    fclose(fp);
  }
  else
  {
    ip[0] = '\0';
  }
                                                                                
  vget(b_lines - 1, 0, "�п�J�H��۰���H�� E-mail�G", ip, 50, GCARRY);
                                                                                
  if (ip[0] && !not_addr(ip) &&
    vans("�T�w�}�ҫH����H�\\��(Y/N)�H[N] ") == 'y')
  {
    if (fp = fopen(fpath, "w"))
    {
      fprintf(fp, "%s", ip);
      fclose(fp);
      pmsg2("�]�w����");
      return 0;
    }
  }
                                                                                
  unlink(fpath);
  pmsg2("�����۰���H�εL�� E-mail");
  return 0;
}

/* cache.100129: ���ثH�c���� */ 
int
m_setmboxdir()
{

  char upath[128], fpath1[128], fpath2[128], fpath3[128], id[5];

  pmsg2("ĵ�i�G���\\��u��b�H�c�w���l�ɨϥ�");
  pmsg2("ĵ�i�G���د��ިä���O�ҫH�c������");


  if (vans("�T�w���د���(Y/N)�H[N] ") == 'y')
  {  

    vget(b_lines - 1, 0, "�п�J�b�����Ĥ@�ӭ^��r��(�p�g)�G", id, 5, GCARRY);

    usr_fpath(upath, cuser.userid, NULL); 

    sprintf(fpath1, "/home/bbs/usr/%s/%s", id, cuser.userid);

    sprintf(fpath2, "~/bin/redir");

    sprintf(fpath3, "mv .DIR.@ .DIR");

    chdir(fpath1);
	system(fpath2);	
	system(fpath3);
    chdir("/home/bbs");

	pmsg2("���ا���");
    return 0;
  }
  
  pmsg2("�������ثH�c����");
  return 0;

}

/* ----------------------------------------------------- */
/* in boards/mail �^�H����@�̡A��H����i		 */
/* ----------------------------------------------------- */


int
hdr_reply(row, hdr)
  int row;
  HDR *hdr;
{
  char *title, *str;

  title = str = ve_title;

  if (hdr)
  {
    sprintf(title, "Re: %s", str_ttl(hdr->title));
    str += TTLEN;
  }
  *str = '\0';

  return vget(row, 0, "���D�G", title, TTLEN + 1, GCARRY);
}


/* static inline */ int
mail_external(addr)
  char *addr;
{
  char *str;

  str = strchr(addr, '@');
  if (!str)
    return 0;

    /* Thor.990125: MYHOSTNAME�Τ@��J str_host */
  if (str_cmp(str_host, str + 1))
    return 1;

  /* �d�I xyz@domain �� xyz.bbs@domain */

  *str = '\0';
  if ((str = strchr(addr, '.')))
    *str = '\0';
  return 0;
}

/* cache.091209: �۰���H�H��*/ 
/* cuser.userid �N�u���D title�B�ɮצb fpath�v���H��H�� userid ���~���H�c */
static void
forward_mail(fpath, userid, title)
  char *fpath, *userid, *title;
{
  FILE *fp;
  char ip[80];
                                                                                
  usr_fpath(ip, userid, FN_FORWARD);
  if (fp = fopen(ip, "r"))
  {
    fscanf(fp, "%s", ip);
    fclose(fp);
                                                                                
    if (ip[0])
      bsmtp(fpath, title, ip, 0);
  }
}

int
mail_send(rcpt, title)
  char *rcpt, *title;
{
  HDR mhdr;
  char fpath[80], folder[80],ckforward[80];
  int rc, userno=0;
  ACCT acct;
  
  int internet_mail;
  
  if (!(internet_mail = mail_external(rcpt)))
  {
    if ((userno = acct_userno(rcpt)) <= 0)
      return -1;

    if (!title)
      vget(2, 0, "�D�D�G", ve_title, TTLEN, DOECHO);

  }

  utmp_mode(M_SMAIL);
  fpath[0] = '\0';

  curredit = EDIT_MAIL;		/* Thor.1105: �������w�g�H */

  if (vedit(fpath, internet_mail ? 2 : 1) == -1)
  {
    unlink(fpath);
    clear();
    return -2;
  }

  if (internet_mail)
  {
    clear();
    prints("�H��Y�N�H�� %s\n���D���G%s\n�T�w�n�H�X��? (Y/N) [Y]",
      rcpt, title);
    switch (vkey())
    {
    case 'n':
    case 'N':
      outs("N\n�H��w����");
      refresh();
      rc = -2;
      break;

    default:
      outs("Y\n�еy��, �H��ǻ���...\n");
      refresh();
      rc = bsmtp(fpath, title, rcpt, 0);
      if (rc < 0)
	vmsg("�H��L�k�H�F");
      mail_hold(fpath, rcpt);
    }
    unlink(fpath);
    return rc;
  }
    usr_fpath(ckforward,rcpt,"forward");
    if(access(ckforward,0))
    {
      usr_fpath(folder, rcpt, fn_dir);
      hdr_stamp(folder, HDR_LINK, &mhdr, fpath);
      strcpy(mhdr.owner, cuser.userid);
      strcpy(mhdr.nick, cuser.username);	/* :chuan: �[�J nick */
      strcpy(mhdr.title, ve_title);
      rc = rec_add(folder, &mhdr, sizeof(mhdr));
      forward_mail(fpath, rcpt, ve_title);      
    }
    else
    {  
      if(acct_load(&acct, rcpt) >= 0)
      {
        if (!title)
          title = ve_title;
        rc = bsmtp(fpath, title, acct.email, 0);
        if (rc < 0)
          vmsg("�H��L�k�H�F");
        mail_hold(fpath, acct.email);
        unlink(fpath);
        return rc;
      }
      else
        vmsg("�H��L�k�H�F");
      return -1;
    }
      if (!rc) {
        mail_hold(fpath, rcpt);
  }

#if 0
  prints("\n���H�H: %s (%s)\n��  �D: %s\n", mhdr.owner, mhdr.nick, mhdr.title);
  refresh();
#endif

  unlink(fpath);

  m_biff(userno);

#ifdef EMAIL_PAGE
  /* --------------------------------------------------- */
  /* E-Mail �����ǩI  by lkchu@dragon2.net               */
  /* --------------------------------------------------- */
              
  if ((acct_load(&acct, rcpt) >= 0) && (acct.ufo & UFO_MPAGER))
  {
    char *p;
    
    if ((p = str_str(acct.address, "bbc")) != NULL)  /* �� BBC �y�z */
      DL_func("bin/emailpage.so:vaEMailPager", p + 3, cuser.userid, ve_title);
  }
#endif

  return rc;
}


void
mail_reply(hdr)
  HDR *hdr;
{
  int xmode, prefix;
  char *msg, buf[80];

  if(bbsothermode & OTHERSTAT_EDITING)
  {
    vmsg("�A�٦��ɮ��٨S�s���@�I");
    return;
  }

  
  if(!(HAS_PERM(PERM_INTERNET)))
    return ;

//  if ((hdr->xmode & MAIL_NOREPLY) || m_count())
//    return;

  if((hdr->xmode & MAIL_NOREPLY) || mail_stat(CHK_MAIL_NOMSG))
  {
     vmsg("�A���H�c�e�q�W�L�W���A�о�z�I");
     chk_mailstat = 1;
     return;
  }
  else
     chk_mailstat = 0;

  vs_bar("�^  �H");

  /* find the author */

  strcpy(quote_user, hdr->owner);
  strcpy(quote_nick, hdr->nick);

  /* make the title */

  if (!hdr_reply(3, hdr))
    return;

  prints("\n���H�H: %s (%s)\n��  �D: %s\n", quote_user, quote_nick, ve_title);

  /* Thor: ���F�٤@�� rec_put �^�H�h���]�ݹL���e */

  xmode = hdr->xmode | MAIL_READ;
  prefix = quote_file[0];

  /* edit, then send the mail */

  switch (mail_send(quote_user, ve_title))
  {
  case -1:
    msg = err_uid;
    break;

  case -2:
    msg = msg_cancel;
    break;

  case -3:  /* Thor.980707: �������p�� ?*/
    sprintf(msg = buf, "[%s] �L�k���H", quote_user);
    break;

  default:
    xmode |= MAIL_REPLIED;
    msg = "�H�w�H�X";  /* Thor.980705: mail_send()�w�g��ܹL�@���F.. ������? */
    break;
  }

  if (prefix == 'u')  /* user mail �ݫH�ɤ~�� r */
  {
    hdr->xmode = xmode;
  }

  vmsg(msg);
}


void
my_send(rcpt)
  char *rcpt;
{
  int result;
  char *msg;

  if(bbsothermode & OTHERSTAT_EDITING)
  {
    vmsg("�A�٦��ɮ��٨S�s���@�I");
    return ;
  }



  if(mail_stat(CHK_MAIL_NOMSG))
  {
     vmsg("�A���H�c�e�q�W�L�W���A�о�z�I");
     chk_mailstat = 1;
     return;
  }
  else
     chk_mailstat = 0;

//  if (m_count())
//    return;

  msg = "�H�w�H�X";

  if ((result = mail_send(rcpt, NULL)))
  {
    switch (result)
    {
    case -1:
      msg = err_uid;
      break;

    case -2:
      msg = msg_cancel;
      break;

    case -3:  /* Thor.980707: �������p�� ?*/
      msg = "�ϥΪ̵L�k���H";
    }
  }
  vmsg(msg);
}


int
m_send()
{
  if(cuser.userlevel & PERM_DENYMAIL)
    vmsg("�z���H�c�Q��F�I");
  else
  {
    ACCT muser;
    vs_bar("�H  �H");
    if (acct_get(msg_uid, &muser) > 0)
      my_send(muser.userid);
  }
  return 0;
}


int
mail_sysop()
{
  int fd;
  if(bbsothermode & OTHERSTAT_EDITING)
  {
    vmsg("�A�٦��ɮ��٨S�s���@�I");
    return 0;
  }


  if ((fd = open(FN_ETC_SYSOP, O_RDONLY)) >= 0)
  {
    int i, j;
    char *ptr, *str;

    struct SYSOPLIST
    {
      char userid[IDLEN + 1];
      char duty[40];
    }         sysoplist[9];	/* ���] 9 �Ө��o */

    j = 0;
    mgets(-1);
    while ((str = mgets(fd)))
    {
	if ((ptr = strchr(str, ':')))
	{
	  *ptr = '\0';
	  do
	  {
	    i = *++ptr;
	  } while (i == ' ' || i == '\t');

	  if (i)
	  {
	    strcpy(sysoplist[j].userid, str);
	    strcpy(sysoplist[j++].duty, ptr);
	  }
	}
    }
    close(fd);

    move(11, 0);
    clrtobot();
    prints("%16s   %-18s�v�d����\n%s\n", "�s��", "���� ID", msg_seperator);

    for (i = 0; i < j; i++)
    {
      prints("%15d.   \033[1;%dm%-16s%s\033[m\n",
	i + 1, 31 + i, sysoplist[i].userid, sysoplist[i].duty);
    }
    prints("%-14s0.   \033[1;%dm���}\033[m", "", 31 + j);

    i = vans("�п�J�N�X[0]�G") - '0' - 1;
    if (i >= 0 && i < j)
    {
      clear();
      mail_send(sysoplist[i].userid, NULL);
    }
  }
  return 0;
}


/* ----------------------------------------------------- */
/* �s�ձH�H�B�^�H : multi_send, multi_reply		 */
/* ----------------------------------------------------- */

#ifndef MULTI_MAIL

#define multi_reply(x) mail_reply(x)

#else  /* Thor.981009: ����R�����B�H */

static int
multi_send(title)
  char *title;
{
  FILE *fp;
  HDR mhdr;
  char buf[128], fpath[64], *userid;
  int userno, reciper, listing;
  LinkList *wp;

  if(bbsothermode & OTHERSTAT_EDITING)
  {
    vmsg("�A�٦��ɮ��٨S�s���@�I");
    return -1;
  }

  vs_bar(title ? "�s�զ^�H" : "�s�ձH�H");

  ll_new();
  listing = reciper = 0;

  /* �^�H��Ū�� mail list �W�� */

  if (*quote_file)
  {
    ll_add(quote_user);
    reciper = 1;

    fp = fopen(quote_file, "r");
    while (fgets(buf, sizeof(buf), fp))
    {
      if (memcmp(buf, "�� ", 3))
      {
	if (listing)
	  break;
      }
      else
      {
	userid = buf + 3;
	if (listing)
	{
	  strtok(userid, " \n\r");
	  do
	  {
	    if ((userno = acct_userno(userid)) && (userno != cuser.userno) &&
	      !ll_has(userid))
	    {
	      ll_add(userid);
	      reciper++;
	    }
	  } while ((userid = (char *) strtok(NULL, " \n\r")));
	}
	else if (!memcmp(userid, "[�q�i]", 6))
	  listing = 1;
      }
    }
    fclose(fp);
    ll_out(3, 0, MSG_CC);
  }

  /* �]�w mail list ���W�� */

  reciper = pal_list(reciper);

  /* �}�l�H�H */

  move(1, 0);
  clrtobot();

  if (reciper == 1)
  {
    mail_send(ll_head->data, title);
  }
  else if (reciper >= 2 && ve_subject(2, title, "[�q�i] "))
  {
    usr_fpath(fpath, cuser.userid, FN_NOTE);

    if ((fp = fopen(fpath, "w")))
    {
      fprintf(fp, "�� [�q�i] �@ %d �H����", reciper);
      listing = 80;
      wp = ll_head;

      do
      {
	userid = wp->data;
	reciper = strlen(userid) + 1;

	if (listing + reciper > 75)
	{
	  listing = reciper;
	  fprintf(fp, "\n��");
	}
	else
	{
	  listing += reciper;
	}

	fprintf(fp, " %s", userid);
      } while ((wp = wp->next));

      memset(buf, '-', 75);
      buf[75] = '\0';
      fprintf(fp, "\n%s\n\n", buf);
      fclose(fp);
    }

    utmp_mode(M_SMAIL);
    curredit = EDIT_MAIL | EDIT_LIST;

    if (vedit(fpath, YEA) == -1)
    {
      outs(msg_cancel);
    }
    else
    {
      vs_bar("�H�H��...");

      listing = 80;
      wp = ll_head;
      title = ve_title;

      do
      {
        ACCT cacct;
	userid = wp->data;
	acct_load(&cacct, userid);
	reciper = strlen(userid) + 1;
	if (listing + reciper > 75)
	{
	  listing = reciper;
	  outc('\n');
	}
	else
	{
	  listing += reciper;
	  outc(' ');
	}
	outs(userid);

	usr_fpath(buf, userid, fn_dir);
	hdr_stamp(buf, HDR_LINK, &mhdr, fpath);
	strcpy(mhdr.owner, cuser.userid);
	strcpy(mhdr.title, title);
	mhdr.xmode = MAIL_MULTI;
	rec_add(buf, &mhdr, sizeof(HDR));
    forward_mail(fpath, userid, title);	
	m_biff(cacct.userno);
      } while ((wp = wp->next));

      mail_hold(fpath, NULL);
    }
    unlink(fpath);

#if 0
    curredit = 0;		/* Thor.1105: ���O�i�H���[�F */
#endif
  }
  else
  {
    vmsg(msg_cancel);
    return -1;
  }
  vmsg(NULL);
  return 0;
}


static void
multi_reply(mhdr)
  HDR *mhdr;
{
  if(!HAS_PERM(PERM_INTERNET))
    return ;
  strcpy(quote_user, mhdr->owner);
  strcpy(quote_nick, mhdr->nick);
  if (!multi_send(mhdr->title))
    mhdr->xmode |= (MAIL_REPLIED | MAIL_READ);
}


int
mail_list()
{

  if(mail_stat(CHK_MAIL_NOMSG))
  {
     vmsg("�A���H�c�e�q�W�L�W���A�о�z�I");
     chk_mailstat = 1;
     return 0;
  }
  else
     chk_mailstat = 0;

//  if (m_count())
//    return 0;

  multi_send(NULL);
  return 0;
}

#endif

/* ----------------------------------------------------- */
/* Mail Box call-back routines				 */
/* ----------------------------------------------------- */


static inline int
mbox_attr(type)
  int type;
{
  if (type & MAIL_DELETE)
    return 'D';

  if (type & MAIL_REPLIED)
    return (type & MAIL_MARKED) ? 'R' : 'r';

  return "+ Mm"[type & 3];
}


int
tag_char(chrono)
  int chrono;
{
  return TagNum && !Tagger(chrono, 0, TAG_NIN) ? '*' : ' ';
}


void
hdr_outs(hdr, cc)		/* print HDR's subject */
  HDR *hdr;
  int cc;
{
  static char *type[4] =
  {"Re", "��", "\033[1;33m=>", "\033[1;32m��"};
  uschar *title, *mark;
  int ch, len;
  UTMP *online;

  if (cc)
  {
#if 0
    if(tag_char(hdr->chrono) == '*')
    {
	  outc(' ');
	  outc('*');
    }
#ifdef	HAVE_RECOMMEND
    else if(!(hdr->xmode &(POST_LOCK | POST_CANCEL | POST_DELETE | POST_MDELETE)) && hdr->recommend >= (MIN_RECOMMEND) && !(cuser.ufo2 & UFO2_PRH))
    {
      if(hdr->recommend <=30)
  	prints("\033[36m%02.2d\033[m",hdr->recommend);
      else if(hdr->recommend > 30 && hdr->recommend <= 60)
  	prints("\033[1;33m%02.2d\033[m",hdr->recommend);
      else
  	prints("\033[1;31m%02.2d\033[m",hdr->recommend);
    }
#endif
    else
    {
      outc(' ');
      outc(' ');
    }
#endif
    outs("\033[m ");
    outs(hdr->date + 3);
    outc(' ');

    mark = hdr->owner;
    len = 13;

      online = utmp_check(mark);  /* �ϥΪ̦b���W�N�ܦ� */
      if (online != NULL)
      outs("\033[1;37m");

    while ((ch = *mark))
    {
      if ((--len == 0) || (ch == '@'))
	ch = '.';
      outc(ch);

      if (ch == '.')
	break;

      mark++;
    }

    while (len--)
    {
      outc(' ');
    }
  }
  else
  {
    cc = 64;
  }

  if (online != NULL)
    outs("\033[m");

  title = str_ttl(mark = hdr->title);
  ch = title == mark;
  if (!strcmp(currtitle, title))
    ch += 2;
  outs(type[ch]);

  mark = title + cc;
  cc = ' ';
  
  if(hdr->xmode & POST_LOCK && !HAS_PERM(PERM_SYSOP))
  {
    outs(" ���峹�w�[�K��w�I\033[m");
    outc('\n');
    return;
  }


  {

#ifdef	HAVE_DECLARE		/* Thor.0508: Declaration, ���ըϬY��title����� */
    int square = 0;		/* 0���L��A,
				 * 1���Ĥ@�r����A"["��out,2���wout"["��out"]"
				 * , 3����out, ���A�B�z */
    int angle = 0;		/* 0��normal���ܦ�, 1���J��angle,�w�ܦ� */
    if (ch < 2)
    {
      if (*title == '[')
      {
	square = 1;
      }
    }
#endif

    do
    {

#ifdef	HAVE_DECLARE
      if (ch < 2)
      {
	switch (square)
	{
	case 1:
	  if (cc == '[')
	  {
	    outs("\033[1;37m[");
	    square = 2;
	    continue;
	  }
	case 2:
	  if (cc == ']')
	  {
	    outs("]\033[m");
	    square = 3;
	    continue;
	  }
	}
#if 0
	if (angle)
	{
	  if (cc != '<' && cc != '>')
	  {
	    outs("\033[m");
	    angle = 0;
	  }
	}
	else
	{
	  if (cc == '<' || cc == '>')
	  {
	    outs("\033[1;37m");
	    angle = 1;
	  }
	}
#endif
      }
#endif

      outc(cc);
    } while ((cc = *title++) && (title < mark));

#ifdef	HAVE_DECLARE
    if (angle || square == 2)	/* Thor.0508: �ܦ��٭�� */
      outs("\033[m");
#endif
  }

  if (ch >= 2)
    outs("\033[m");

  outc('\n');
}


static inline void
mbox_item(pos, hdr)
  int pos;			/* sequence number */
  HDR *hdr;
{

#if 0				/* Thor.0508: �ܦ�ݬ� */
  prints("%5d %c%", pos, mbox_attr(hdr->xmode));
#endif

  int xmode = hdr->xmode;
  prints(xmode & MAIL_DELETE ? "%5d \033[1;5;37;41m%c\033[m"
    : xmode & MAIL_MARKED ? "%5d \033[1;36m%c\033[m"
    : "%5d %c", pos, mbox_attr(hdr->xmode));

  hdr_outs(hdr, 47);
}


static int
mbox_body(xo)
  XO *xo;
{
  HDR *mhdr;
  int num, max, tail;

  max = xo->max;
  
  if (max <= 0)
  {
    vmsg("�z�S���ӫH");
    return XO_QUIT;
  }

  num = xo->top;
  mhdr = (HDR *) xo_pool;
  tail = num + XO_TALL;
  if (max > tail)
    max = tail;

  move(3, 0);
  do
  {
    mbox_item(++num, mhdr++);
  } while (num < max);
  clrtobot();

  return XO_NONE;
}


static int
mbox_head(xo)
  XO *xo;
{
  vs_head("\0�l����", str_site);

  outs(NECKMAIL);

  return mbox_body(xo);
}


static int
mbox_load(xo)
  XO *xo;
{
  xo_load(xo, sizeof(HDR));
  return mbox_body(xo);
}


static int
mbox_init(xo)
  XO *xo;
{
  xo_load(xo, sizeof(HDR));
  return mbox_head(xo);
}


static int
mbox_delete(xo)
  XO *xo;
{
  int pos, xmode;
  HDR *hdr;
  char *dir;
#ifndef	HAVE_MAILUNDELETE
  char fpath[64];
#endif

  pos = xo->pos;
  hdr = (HDR *) xo_pool + (pos - xo->top);

  xmode = hdr->xmode;
  if ((xmode & (MAIL_MARKED | MAIL_DELETE)) == MAIL_MARKED)
    return XO_NONE; /* Thor.980901: mark��Y�Q'D'�_��, �h�@�˥i�Hdelete,
                                    �u�� MARK & no delete�~�|�L�� */

  if (vans(msg_del_ny) == 'y')
  {
    dir = xo->dir;
    currchrono = hdr->chrono;
#ifdef HAVE_MAILUNDELETE
    hdr->xmode |= POST_DELETE;
    if (!rec_put(dir, hdr, sizeof(HDR), pos))
    {
      return mbox_load(xo);
    }
#else    
    if (!rec_del(dir, sizeof(HDR), pos, cmpchrono, NULL))
    {
      hdr_fpath(fpath, dir, hdr);
      unlink(fpath);
      return mbox_load(xo);
    }
#endif
  }
  return XO_FOOT;
}

static int
mbox_forward(xo)
  XO *xo;
{
  ACCT muser;
  int pos;
  HDR *hdr;
  if(bbsothermode & OTHERSTAT_EDITING)
  {
    vmsg("�A�٦��ɮ��٨S�s���@�I");
    return XO_FOOT;
  }


  if(mail_stat(CHK_MAIL_NOMSG))
  {
     vmsg("�A���H�c�e�q�W�L�W���A�о�z�I");
     chk_mailstat = 1;
     return XO_HEAD;
  }
  else
     chk_mailstat = 0;

  if (acct_get("��F�H�󵹡G", &muser) > 0)
  {
    pos = xo->pos;
    hdr = (HDR *) xo_pool + (pos - xo->top);

    strcpy(quote_user, hdr->owner);
    hdr_fpath(quote_file, xo->dir, hdr);
    sprintf(ve_title, "%.64s (fwd)", hdr->title);
    move(1, 0);
    clrtobot();
    prints("��H��: %s (%s)\n��  �D: %s\n", muser.userid, muser.username,
      ve_title);

    switch (mail_send(muser.userid, ve_title))
    {
    case -1:
      outs(err_uid);
      break;

    case -2:
      outs(msg_cancel);
      break;

    case -3:  /* Thor.980707: �������p�� ?*/
      prints("�ϥΪ� [%s] �L�k���H", muser.userid);
    }
    *quote_file = '\0';
    vmsg(NULL);
  }
  return mbox_head(xo);
}


static int
mbox_browse(xo)
  XO *xo;
{
  HDR *mhdr,hdr;
  int pos, xmode, nmode,mode;
  char *dir, *fpath;

  dir = xo->dir;
  pos = xo->pos;
  fpath = quote_file;
  mhdr = (HDR *) xo_pool + (pos - xo->top);
  memcpy(&hdr,mhdr,sizeof(HDR));
  mhdr = &hdr;
  strcpy(currtitle, str_ttl(mhdr->title));
  xmode = mhdr->xmode;

#ifdef  HAVE_CHK_MAILSIZE
 if(!(mhdr->xmode & MAIL_READ) && chk_mailstat == 1)
 {
   if(mail_stat(CHK_MAIL_NOMSG))
   {
     vmsg("�z���H�c�w�W�X�e�q�A�L�k�\\Ū�s�H��A�вM�z�z���H�c�I");
     return XO_FOOT;
   }
   else
     chk_mailstat = 0;
 }
#endif

  hdr_fpath(fpath, dir, mhdr);

  if((mode = more(fpath, MSG_MAILER)) == -1)
  {
    *fpath = '\0';
    return XO_INIT;
  }
  else if(mode == -2)
  {
    mhdr->xmode |= MAIL_READ;
    rec_put(dir, mhdr, sizeof(HDR), pos);
    *fpath = '\0';
    return XO_INIT;
    /*nmode = vkey();*/
  }
  else if(mode > 0)
    nmode = mode;
  else
    nmode = vkey();

  if (nmode == 'd')
  {
    if (mbox_delete(xo) != XO_FOOT)
    {
      *fpath = '\0';
      return mbox_init(xo);
    }
  }
  else if (nmode == 'y' || nmode == 'r')
  {
    if (!(xmode & MAIL_NOREPLY) && !mail_stat(CHK_MAIL_NOMSG) /* m_count() */)
    {
      if ((nmode == 'y') && (xmode & MAIL_MULTI))
        multi_reply(mhdr);
      else
        mail_reply(mhdr);
    }
  }
  else if (nmode == 'm')
  {
    mhdr->xmode |= MAIL_MARKED;
  }
  else if (nmode == 'x')
  {
    mbox_forward(xo);
  }

  nmode = mhdr->xmode | MAIL_READ;

  if (xmode != nmode)
  {
    mhdr->xmode = nmode;
    rec_put(dir, mhdr, sizeof(HDR), pos);
  }
  *fpath = '\0';

  return XO_INIT;
}

static int
mbox_reply(xo)
  XO *xo;
{
  int pos, xmode;
  HDR *mhdr,hdr;

//  if (m_count())
//    return mbox_head(xo);

  if(mail_stat(CHK_MAIL_NOMSG))
  {
     vmsg("�A���H�c�e�q�W�L�W���A�о�z�I");
     chk_mailstat = 1;
     return XO_HEAD;
  }
  else
     chk_mailstat = 0;

  pos = xo->pos;
  mhdr = (HDR *) xo_pool + pos - xo->top;
  memcpy(&hdr,mhdr,sizeof(HDR));
  mhdr = &hdr;    

  xmode = mhdr->xmode;
  if (xmode & MAIL_NOREPLY)
    return XO_NONE;
  hdr_fpath(quote_file, xo->dir, mhdr);
  if (xmode & MAIL_MULTI)
    multi_reply(mhdr);
  else
    mail_reply(mhdr);
  *quote_file = '\0';

  if (mhdr->xmode != xmode)
    rec_put(xo->dir, mhdr, sizeof(HDR), pos);

  return XO_INIT;
}


static int
mbox_mark(xo)
  XO *xo;
{
  HDR *mhdr;
  int cur, pos;

  pos = xo->pos;
  cur = pos - xo->top;
  mhdr = (HDR *) xo_pool + cur;
  move(3 + cur, 6);
  outc(mbox_attr(mhdr->xmode ^= MAIL_MARKED));
  rec_put(xo->dir, mhdr, sizeof(HDR), pos);
  return XO_NONE;
}


static int
mbox_tag(xo)
  XO *xo;
{
  HDR *hdr;
  int tag, pos, cur;

  pos = xo->pos;
  cur = pos - xo->top;
  hdr = (HDR *) xo_pool + cur;

  if ((tag = Tagger(hdr->chrono, pos, TAG_TOGGLE)))
  {
    move(3 + cur, 7);
    outc(tag > 0 ? '*' : ' ');
  }

  /* return XO_NONE; */
  return xo->pos + 1 + XO_MOVE; /* lkchu.981201: ���ܤU�@�� */
}


int
mbox_send(xo)
  XO *xo;
{
  m_send();
  return mbox_head(xo);
}

/*by visor*/
static int
mbox_sysop(xo)
  XO *xo;
{
  if (/*(xo == (XO *) & cmbox) &&*/ (cuser.userlevel & PERM_SYSOP))
  {
    XO *xx;

    xz[XZ_MBOX - XO_ZONE].xo = xx = xo_new("usr/s/sysop/.DIR");
    xover(XZ_MBOX);
    free(xx);

    xz[XZ_MBOX - XO_ZONE].xo = xo;
    mbox_init(xo);
  }

  return XO_NONE;
}

static int
mbox_other(xo)
  XO *xo;
{
 
  ACCT acct;
  char path[80];  
  
  if(!supervisor)
     return XO_NONE;

  while (acct_get(msg_uid, &acct) > 0)
  {
    XO *xx;

    //str_lower(id, acct.userid);  
    //sprintf(path,"usr/%c/%s/.DIR",*id,id);
  
    usr_fpath(path, acct.userid, fn_dir);
    usr_fpath(cmbox.dir, acct.userid, fn_dir);
  
    xz[XZ_MBOX - XO_ZONE].xo = xx = xo_new(path);
    xover(XZ_MBOX);
    free(xx);
    
    usr_fpath(cmbox.dir, cuser.userid, fn_dir);

    xz[XZ_MBOX - XO_ZONE].xo = xo;
    mbox_init(xo);
 
 }
  return XO_HEAD;
}

static int
mbox_help(xo)
  XO *xo;
{
  film_out(FILM_MAIL, -1);
  return mbox_head(xo);
}

int
mail_stat(mode)
  int mode;
{
  int limit_e,total_e;
  int limit_k,total_k;
  char buf[128];
  
      limit_e = cuser.userlevel;
      if (limit_e & (PERM_SYSOP | PERM_MBOX))
        limit_e = MAX_BBSMAIL;
      else if (mode & CHK_MAIL_VALID)
        limit_e = MAX_VALIDMAIL;
      else
        limit_e = limit_e & PERM_VALID ? MAX_VALIDMAIL : MAX_NOVALIDMAIL;
  
      limit_k = cuser.userlevel;
      if (limit_k & (PERM_SYSOP | PERM_MBOX))
        limit_k = MAX_MAIL_SIZE;
      else if (mode & CHK_MAIL_VALID)
        limit_k = MAIL_SIZE;
      else
        limit_k = limit_k & PERM_VALID ? MAIL_SIZE : MAIL_SIZE_NO;  

  usr_fpath(buf, cuser.userid, fn_dir);
  total_e = rec_num(buf, sizeof(HDR));
  total_k = m_total_size() / 1024;
  sprintf(buf,"�H��� %d/%d �ʡA�e�q�j�p %d/%d K�I",total_e,limit_e,total_k,limit_k);
  if(mode & CHK_MAIL_NORMAL)
    vmsg(buf);
  return (total_k>=limit_k)||(total_e>limit_e);
}

static int
mbox_stat(xo)
  XO *xo;
{
  mail_stat(CHK_MAIL_NORMAL);
  return XO_HEAD;
}

static int
mbox_edit(xo)
  XO *xo;
{
  HDR *hdr;
  char fpath[128];
  if(bbsothermode & OTHERSTAT_EDITING)
  {
    vmsg("�A�٦��ɮ��٨S�s���@�I");
    return XO_FOOT;
  }

  hdr = (HDR *) xo_pool + (xo->pos - xo->top);
  hdr_fpath(fpath, xo->dir, hdr);
  if (cuser.userlevel & PERM_SYSOP)
  {
    vedit(fpath, NA); 
    return mbox_head(xo);
  }
  return XO_NONE;
}

static int
mbox_size(xo)
  XO *xo;
{
  HDR *hdr;
  char *dir, fpath[80],buf[128];
  struct stat st;

  dir = xo->dir;
  hdr = (HDR *) xo_pool + xo->pos - xo->top;
  hdr_fpath(fpath,dir,hdr);

  if (HAS_PERM(PERM_SYSOP))
  {
    move(12, 0);
    clrtobot();
    outs("\nDir : ");
    outs(dir);
    outs("\nName: ");
    outs(hdr->xname);
    outs("\nFile: ");
    outs(fpath);

    if (!stat(fpath, &st))
      prints("\nTime: %s\nSize: %d", Ctime(&st.st_mtime), st.st_size);
    vmsg(NULL);
  }
  else
  {
    stat(fpath, &st);
    sprintf(buf,"�e�q�j�p: %d K", (int)st.st_size/1024);
    vmsg(buf);
  }


  return XO_HEAD;
}

#ifdef  HAVE_MAIL_FIX
static int
mbox_title(xo)
  XO *xo;
{
  HDR *hdr,mhdr;
     
  if(!supervisor)
    return XO_NONE;


  hdr = (HDR *) xo_pool + (xo->pos - xo->top);
  mhdr = *hdr;

  vget(b_lines, 0, "���D�G", mhdr.title, sizeof(mhdr.title), GCARRY);
  vget(b_lines, 0, "�@�̡G", mhdr.owner, 74 , GCARRY);
  vget(b_lines, 0, "����G", mhdr.date, sizeof(mhdr.date), GCARRY);
  vget(b_lines, 0, "�ɦW�G", mhdr.xname, sizeof(mhdr.date), GCARRY);
  if(mhdr.xid > 1000)
    mhdr.xid = 0;
  if (vans(msg_sure_ny) == 'y' &&
    memcmp(hdr, &mhdr, sizeof(HDR)))
  {
    *hdr = mhdr;
    rec_put(xo->dir, hdr, sizeof(HDR), xo->pos);
    return XO_INIT;
  }

  return XO_FOOT;
}
#endif /* HAVE_MAIL_FIX */


#ifdef HAVE_MAILUNDELETE

static int
mbox_undelete(xo)
  XO *xo;
{
  HDR *hdr;

  hdr = (HDR *) xo_pool + (xo->pos - xo->top);

  hdr->xmode &= ~POST_DELETE;

  if (!rec_put(xo->dir, hdr, sizeof(HDR), xo->pos))
    return XO_INIT;
  return XO_NONE;
}

static int
mbox_clean(xo)
  XO *xo;
{
  if(vans("\033[1;5;41;33mĵ�i�G\033[m�M�����ᤣ��Ϧ^�C�T�w�n�M���ܡH(y/N)") == 'y')
  {
    hdr_prune(xo->dir, 0, 0 , 3);
    return XO_INIT;
  }
  return XO_HEAD;
}

int
mbox_check()
{
  HDR hdr;
  int fd,total;
  char fpath[256];

  usr_fpath(fpath,cuser.userid,FN_DIR);
  total = 0;
  if((fd = open(fpath, O_RDONLY)) >= 0)
  {
    while(read(fd, &hdr, sizeof(HDR)) == sizeof(HDR))
    {
      if( hdr.xmode & POST_DELETE )
        total++;
    }
    close(fd);
  }
  return total;
}
#endif  /* HAVE_MAILUNDELETE */

#ifdef	HAVE_MAILGEM
static int
mbox_gem(xo)
  XO *xo;
{
  static void (*mgp)();
  if(!HAS_PERM(PERM_MBOX))
    return XO_NONE;
  if(!mgp)
  {
    mgp = DL_get("bin/mailgem.so:mailgem_main");
    if(mgp)
      (*mgp)();
    else
      vmsg("�ʺA�s�����ѡA���p���t�κ޲z���I");
  }
  else
    (*mgp)();
  return XO_INIT;
}
#endif

static KeyFunc mbox_cb[] =
{
  {XO_INIT, mbox_init},
  {XO_LOAD, mbox_load},
  {XO_HEAD, mbox_head},
  {XO_BODY, mbox_body},

#ifdef	HAVE_MAIL_FIX
  {'T', mbox_title},
#endif
  {'r', mbox_browse},
  {'E', mbox_edit},
  {'s', mbox_send},
  {'d', mbox_delete},
  {Ctrl('X'), mbox_forward},
  {'m', mbox_mark},
#ifdef	HAVE_MAILGEM
  {'z', mbox_gem},
#endif  
  {'R', mbox_reply},
  {'y', mbox_reply},
  {'c', mbox_stat},
#ifdef HAVE_MAILUNDELETE
  {'U', mbox_clean}, 
  {'u', mbox_undelete},
#endif

  {KEY_TAB, mbox_sysop},
  {'I', mbox_other},
  {'t', mbox_tag},
  {'S', mbox_size},
  {'D', xo_delete},

  {Ctrl('Q'), xo_uquery},
  {'X', xo_usetup},
  {'x', post_cross},

  {'h', mbox_help}
};


void
mbox_main()
{
  cmbox.mail_xo.pos = XO_TAIL;
  usr_fpath(cmbox.dir, cuser.userid, fn_dir);
  xz[XZ_MBOX - XO_ZONE].xo = (XO *) &cmbox;
  xz[XZ_MBOX - XO_ZONE].cb = mbox_cb;
}

