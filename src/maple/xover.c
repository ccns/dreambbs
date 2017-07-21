/*-------------------------------------------------------*/
/* xover.c	( NTHU CS MapleBBS Ver 2.36 )		 */
/*-------------------------------------------------------*/
/* target : board/mail interactive reading routines 	 */
/* create : 95/03/29				 	 */
/* update : 2000/01/02				 	 */
/*-------------------------------------------------------*/

#include "bbs.h"

#define XO_STACK	(5)
#define MAX_LEVEL	(20)
static int xo_stack_level;
static int xo_user_level;

extern int boardmode;

#ifdef	HAVE_FAVORITE
#define MSG_ZONE_SWITCH \
	"�ֳt�����GA)��ذ� B)�峹�C�� C)�ݪO�C�� M)�H�� F)�ڪ��̷R P)�i���\\��G"

#define MSG_ZONE_ADVANCE \
	"�i���\\��GU)�ϥΪ̦W�� W)��ݰT���G"

#else

#define	MSG_ZONE_SWITCH \
	"�ֳt�����GA)��ذ� B)�峹�C�� C)�ݪO�C�� M)�H�� U)�ϥΪ̦W�� W)��ݰT���G"
#endif


/* ----------------------------------------------------- */
/* keep xover record					 */
/* ----------------------------------------------------- */

static XO *xo_root;		/* root of overview list */


XO *
xo_new(path)
  char *path;
{
  XO *xo;
  int len;

  len = strlen(path) + 1;

  xo = (XO *) malloc(sizeof(XO) + len);

  memcpy(xo->dir, path, len);

  return (xo);
}


XO *
xo_get(path)
  char *path;
{
  XO *xo;

  for (xo = xo_root; xo; xo = xo->nxt)
  {
    if (!strcmp(xo->dir, path))
      return xo;
  }

  xo = xo_new(path);
  xo->nxt = xo_root;
  xo_root = xo;
  xo->xyz = NULL;
  xo->pos = XO_TAIL;		/* �Ĥ@���i�J�ɡA�N��Щ�b�̫᭱ */

  return xo;
}


#if 0
void
xo_free(xo)
  XO *xo;
{
  char *ptr;

  if (ptr = xo->xyz)
    free(ptr);
  free(xo);
}
#endif


/* ----------------------------------------------------- */
/* interactive menu routines			 	 */
/* ----------------------------------------------------- */


char xo_pool[XO_TALL * XO_RSIZ];


void
xo_load(xo, recsiz)
  XO *xo;
  int recsiz;
{
  int fd, max;

  max = 0;
  if ((fd = open(xo->dir, O_RDONLY)) >= 0)
  {
    int pos, top;
    struct stat st;

    fstat(fd, &st);
    max = st.st_size / recsiz;
    if (max > 0)
    {
      pos = xo->pos;
      if (pos <= 0)
      {
	pos = top = 0;
      }
      else
      {
	top = max - 1;
	if (pos > top)
	  pos = top;
	top = (pos / XO_TALL) * XO_TALL;
      }
      xo->pos = pos;
      xo->top = top;

      lseek(fd, (off_t) (recsiz * top), SEEK_SET);
      read(fd, xo_pool, recsiz * XO_TALL);
    }
    close(fd);
  }

  xo->max = max;
}


/* static */
void
xo_fpath(fpath, dir, hdr)
  char *fpath;
  char *dir;
  HDR *hdr;
{
  if (hdr->xmode & HDR_URL)
    url_fpath(fpath, dir, hdr);
  else
    hdr_fpath(fpath, dir, hdr);
}


/* ----------------------------------------------------- */
/* nhead:						 */
/* 0 ==> �̾� TagList �s��R��				 */
/* !0 ==> �̾� range [nhead, ntail] �R��		 */
/* ----------------------------------------------------- */
/* notice : *.n - new file				 */
/* *.o - old file					 */
/* ----------------------------------------------------- */


int
hdr_prune(folder, nhead, ntail, post)
  char *folder;
  int nhead, ntail;
  int post;
{
  int count, fdr, fsize, xmode, cancel, dmode;
  HDR *hdr;
  FILE *fpw;
  char fnew[80], fold[80];

  if ((fdr = open(folder, O_RDONLY)) < 0)
    return -1;

  if (!(fpw = f_new(folder, fnew)))
  {
    close(fdr);
    return -1;
  }

  xmode = *folder;
  cancel = (xmode == 'b');
  dmode = (xmode == 'u') ? 0 : (POST_CANCEL | POST_DELETE | POST_MDELETE);

  fsize = count = 0;
  mgets(-1);
  while ((hdr = mread(fdr, sizeof(HDR))))
  {
    xmode = hdr->xmode;
    count++;
#if 0
    if (xmode & dmode)		/* �w�R�� */
	continue;
#endif
    if (((xmode & (POST_MARKED|POST_LOCK|POST_DELETE)) ||	/* �аO */
	(nhead && (count < nhead || count > ntail)) ||	/* range */
      (!nhead && Tagger(hdr->chrono, count - 1, TAG_NIN)))	/* TagList */
      && !(post == 3 && (hdr->xmode & POST_DELETE)))
    {
      if(!post)
      {
        if ((fwrite(hdr, sizeof(HDR), 1, fpw) != 1))
        {  
          close(fdr);
          fclose(fpw);
	  unlink(fnew);
	  return -1;
        }
        fsize++;
      }
    }
    else
    {
      /* �Y���ݪO�N�s�u��H */
      if (cancel)
	cancel_post(hdr);
      if(!post)
      {
      
        hdr_fpath(fold, folder, hdr);
        unlink(fold);
      }
      else
      {
        if(post == 1)
        {
          hdr->xmode |= POST_MDELETE;
          sprintf(hdr->title, "<< ���峹�g %s �R�� >>", cuser.userid);
        }
#ifdef  HAVE_MAILUNDELETE
        else if(post == 3 && (hdr->xmode & POST_DELETE))
        {
          hdr_fpath(fold, folder, hdr);
          unlink(fold);
        }
        else if(post != 3)
#else
        else
#endif
          hdr->xmode |= POST_DELETE;
      }
    }
#ifdef  HAVE_MAILUNDELETE
    if(post && !(post == 3 && (hdr->xmode & POST_DELETE)))
#else
    if(post)
#endif
    {
      if ((fwrite(hdr, sizeof(HDR), 1, fpw) != 1))
      {
        close(fdr);
        fclose(fpw);
        unlink(fnew);
        return -1;
      }
      fsize++;
    }
  }
  close(fdr);
  fclose(fpw);

  sprintf(fold, "%s.o", folder);
  rename(folder, fold);
  if (fsize)
    rename(fnew, folder);
  else
    unlink(fnew);

  return 0;
}


int
xo_delete(xo)
  XO *xo;
{
  char buf[8];
  int head, tail;

  if ((bbsmode == M_READA) && !(bbstate & STAT_BOARD))
    return XO_NONE;

  vget(b_lines, 0, "[�]�w�R���d��] �_�I�G", buf, 6, DOECHO);
  head = atoi(buf);
  if (head <= 0)
  {
    zmsg("�_�I���~");
    return XO_FOOT;
  }

  vget(b_lines, 28, "���I�G", buf, 6, DOECHO);
  tail = atoi(buf);
  if (tail < head)
  {
    zmsg("���I���~");
    return XO_FOOT;
  }


  if (vget(b_lines, 41, msg_sure_ny, buf, 3, LCECHO) == 'y')
  {
    if(bbsmode == M_READA)
      hdr_prune(xo->dir, head, tail , 0);
#ifdef	HAVE_MAILUNDELETE
    else if(bbsmode == M_RMAIL)
      hdr_prune(xo->dir, head, tail , 2);
#endif
    else
      hdr_prune(xo->dir, head, tail , 0);

    return XO_LOAD;
  }
  return XO_FOOT;
}


/* ----------------------------------------------------- */
/* Tag List ����					 */
/* ----------------------------------------------------- */


int TagNum;			/* tag's number */
TagItem TagList[TAG_MAX];	/* ascending list */


int
Tagger(chrono, recno, op)
  time_t chrono;
  int recno;
  int op;			/* op : TAG_NIN / TOGGLE / INSERT */
/* ----------------------------------------------------- */
/* return 0 : not found	/ full				 */
/* 1 : add						 */
/* -1 : remove						 */
/* ----------------------------------------------------- */
{
  int head, tail, pos=0, cmp;
  TagItem *tagp;

  for (head = 0, tail = TagNum - 1, tagp = TagList, cmp = 1; head <= tail;)
  {
    pos = (head + tail) >> 1;
    cmp = tagp[pos].chrono - chrono;
    if (!cmp)
    {
      break;
    }
    else if (cmp < 0)
    {
      head = pos + 1;
    }
    else
    {
      tail = pos - 1;
    }
  }

  if (op == TAG_NIN)
  {
    if (!cmp && recno)		/* �����Y�ԡG�s recno �@�_��� */
      cmp = recno - tagp[pos].recno;
    return cmp;
  }

  tail = TagNum;

  if (!cmp)
  {
    if (op != TAG_TOGGLE)
      return NA;

    TagNum = --tail;
    memcpy(&tagp[pos], &tagp[pos + 1], (tail - pos) * sizeof(TagItem));
    return -1;
  }

  if (tail < TAG_MAX)
  {
    TagItem buf[TAG_MAX];

    TagNum = tail + 1;
    tail = (tail - head) * sizeof(TagItem);
    tagp += head;
    memcpy(buf, tagp, tail);
    tagp->chrono = chrono;
    tagp->recno = recno;
    memcpy(++tagp, buf, tail);
    return YEA;
  }

  /* TagList is full */

  bell();
  return 0;
}


void
EnumTagHdr(hdr, dir, locus)
  HDR *hdr;
  char *dir;
  int locus;
{
  rec_get(dir, hdr, sizeof(HDR), TagList[locus].recno);
}


int
AskTag(msg)
  char *msg;
/* ----------------------------------------------------- */
/* return value :					 */
/* -1	: ����						 */
/* 0	: single article				 */
/* o.w.	: whole tag list				 */
/* ----------------------------------------------------- */
{
  char buf[80];
  /* Thor.990108: �Ȥ����j */
  /* char buf[100]; */
  int num;

  num = TagNum;
  sprintf(buf, "�� %s A)rticle T)ag Q)uit�H[%c] ", msg, num ? 'T' : 'A');
  switch (vans(buf))
  {
  case 'q':
    return -1;

  case 'a':
    return 0;
  }
  return num;
}


/* ----------------------------------------------------- */
/* tag articles according to title / author		 */
/* ----------------------------------------------------- */


static int
xo_tag(xo, op)
  XO *xo;
  int op;
{
  int fsize, count;
  char *token, *fimage;
  HDR *head, *tail;

  fimage = f_map(xo->dir, &fsize);
  if (fimage == (char *) -1)
    return XO_NONE;

  head = (HDR *) xo_pool + (xo->pos - xo->top);
  if (op == Ctrl('A'))
  {
    token = head->owner;
    op = 0;
  }
  else
  {
    token = str_ttl(head->title);
    op = 1;
  }

  head = (HDR *) fimage;
  tail = (HDR *) (fimage + fsize);

  count = 0;

  do
  {
    if (!strcmp(token, op ? str_ttl(head->title) : head->owner))
    {
      if (!Tagger(head->chrono, count, TAG_INSERT))
	break;
    }
    count++;
  } while (++head < tail);

  munmap(fimage, fsize);
  return XO_BODY;
}


static int
xo_prune(xo)
  XO *xo;
{
  int num;
  char buf[80];

  if (!(num = TagNum) || ((bbsmode == M_READA) && !(bbstate & STAT_BOARD)))
    return XO_NONE;

  sprintf(buf, "�T�w�n�R�� %d �g���Ҷ�(Y/N)�H[N] ", num);
  if (vans(buf) != 'y')
    return XO_FOOT;

#if 1
  /* Thor.981122: �O���R���O�� */
  sprintf(buf,"(%d)%s",num, xo->dir);
  blog("PRUNE", buf);
#endif

  if(bbsmode == M_READA)
    hdr_prune(xo->dir, 0, 0 , 1);
#ifdef  HAVE_MAILUNDELETE
  else if(bbsmode == M_RMAIL)
    hdr_prune(xo->dir, 0, 0 , 2);
#endif    
  else
    hdr_prune(xo->dir, 0, 0 , 0);

  TagNum = 0;
  return XO_LOAD;
}


/* ----------------------------------------------------- */
/* Tag's batch operation routines			 */
/* ----------------------------------------------------- */


extern BCACHE *bshm;    /* lkchu.981229 */


static int
xo_copy(xo)
  XO *xo;
{
  char fpath[128], *dir;
  HDR *hdr, xhdr;
  int tag, locus;
  FILE *fp;

  if (!cuser.userlevel)
    return XO_NONE;

  /* lkchu.990428: mat patch ��ݪ��|����w�A�ץ�copy�|�_�u�����D */
  if (bbsmode == M_READA)
  {
    /* lkchu.981205: �ɥ� tag �s��ݪ��ݩ� */
    tag = (bshm->bcache + brd_bno(currboard))->battr;
    if (!HAS_PERM(PERM_SYSOP) && (tag & BRD_NOFORWARD))
    {
      outz("�� ���O�峹���i��K");
      return XO_NONE;
    } 
  }

  tag = AskTag("������Ȧs��");
  if (tag < 0)
    return XO_FOOT;

  fp = tbf_open();
  if (fp == NULL)
    return XO_FOOT;

  if (tag)
    hdr = &xhdr;
  else
    hdr = (HDR *) xo_pool + xo->pos - xo->top;

  locus = 0;
  dir = xo->dir;

  do
  {
    if (tag)
    {
      fputs(STR_LINE, fp);
      EnumTagHdr(hdr, dir, locus++);
    }

    // check delete or not .. by statue 2000/05/18
    if(hdr->xmode & (POST_DELETE | POST_CANCEL | POST_MDELETE))
      continue;

    if ((hdr->xmode & (POST_LOCK|GEM_RESERVED|GEM_RESTRICT)) && !(HAS_PERM(PERM_ALLBOARD) || (bbstate & STAT_BOARD)))
      continue;

    if ((hdr->xmode & (GEM_LOCK)) && !HAS_PERM(PERM_SYSOP))
      continue;
    

    if (!(hdr->xmode & GEM_FOLDER))	/* �d hdr �O�_ plain text */
    {
      xo_fpath(fpath, dir, hdr);
      f_suck(fp, fpath);
    }
  } while (locus < tag);

  fclose(fp);
  zmsg("��������");

  return XO_FOOT;
}


#if 0
/* Thor.981027: �� mail.c���� mail_external���N */
static inline int
rcpt_local(addr)
  char *addr;
{
  char *str;
  int cc;

  str = addr;
  while (cc = *str++)
  {
    if (cc == '@')
    {
      /* Thor.990125: MYHOSTNAME �Τ@��J str_host */
      if (str_cmp(str, str_host))
	return 0;
      str[-1] = '\0';
      if (str = strchr(addr, '.'))
	*str = '\0';
      break;
    }
  }
  return 1;
}
#endif

#if 1
static inline int
deny_forward()
{
  usint level;

  /* Thor.980602: �Q�N�Ҧ��ʺA�v�������ܲΤ@���login�B, �Pı������� 
                  �P�� deny_mail�Ʊ���W�@�� BAN mail���@��
                  �òΤ@�N PERM_CHAT, PERM_PAGE, PERM_POST
                  �� �۰��ܧ��v��, �Τ@�޲z, �P����ܧ��v�����O */

  level = cuser.userlevel;
  if ((level & (PERM_FORWARD | PERM_DENYMAIL)) != PERM_FORWARD)
  {
    if (level & PERM_DENYMAIL)
    {
      /*
      if ((cuser.numemail >> 4) < (cuser.numlogins + cuser.numposts))
      {
        cuser.userlevel = level ^ PERM_DENYMAIL;
	return 0;
      }
      */
      outz("�� �z���H�c�Q��F�I");
    }
    return -1;
  }
  return 0;
}
#endif

static int
xo_forward(xo)
  XO *xo;
{
  static char rcpt[64];
  char fpath[128], folder[80], *dir, *title, *userid , ckforward[80];
  HDR *hdr, xhdr;
  int tag, locus, userno, cc , check;
  usint method;			/* �O�_ uuencode */
  ACCT acct;

  if (deny_forward())
    return XO_NONE;

  /* lkchu.990428: mat patch ��ݪ��|����w�A�ץ�forward�|�_�u�����D */
  if (bbsmode == M_READA)
  {
    /* lkchu.981205: �ɥ� method �s��ݪ��ݩ� */
    method = (bshm->bcache + brd_bno(currboard))->battr;
    if (!HAS_PERM(PERM_SYSOP) && (method & BRD_NOFORWARD))
    {
      outz("�� ���O�峹���i��K");
      return XO_NONE;
    } 
  }

/*
  if((hdr->xmode & POST_LOCK) && !HAS_PERM(PERM_SYSOP))
  {
     vmsg("Access Deny!");
     return XO_NONE;
  }
*/

  tag = AskTag("��H");
  if (tag < 0)
    return XO_FOOT;

  if (!rcpt[0])
    strcpy(rcpt, cuser.email);

  if (!vget(b_lines, 0, "�ت��a�G", rcpt, sizeof(rcpt), GCARRY))
    return XO_FOOT;

  userid = cuser.userid;

  /* �Ѧ� struct.h �� MQ_UUENCODE / MQ_JUSTIFY */

#define	MF_SELF	0x04
#define	MF_USER	0x08

  userno = 0;
  check = 0;

  if (!mail_external(rcpt))    /* ���~�d�I */
  {
    usr_fpath(ckforward,rcpt,"forward");
    if(!access(ckforward,0))
    {
      if(acct_load(&acct, rcpt) >= 0)
      {
        strcpy(rcpt,acct.email);
        method = 0;
        check = 1;
      }
      else
      {
        sprintf(fpath, "�d�L���H�G%s", rcpt);
        zmsg(fpath);
        return XO_FOOT;
      }    
    } 
    else if (!str_cmp(rcpt, userid))
    {
      /* userno = cuser.userno; */ /* Thor.981027: �H��ﶰ���ۤv���q���ۤv */
      method = MF_SELF;
      
      if(mail_stat(CHK_MAIL_NOMSG))
      {
        vmsg("�A���H�c�e�q�W�L�W���A�L�k�ϥΥ��\\��I");
        chk_mailstat = 1;
        return XO_FOOT;
      }
      else
       chk_mailstat = 0;      
      
    }
    else
    {
      if ((userno = acct_userno(rcpt)) <= 0)
      {
	sprintf(fpath, "�d�L���H�G%s", rcpt);
	zmsg(fpath);
	return XO_FOOT;
      }
      method = MF_USER;
    }

    usr_fpath(folder, rcpt, fn_dir);
  }
  else
  {
    if (not_addr(rcpt))
      return XO_FOOT;

    method = 0;

#if 0
    method = vans("�O�_�ݭn uuencode(Y/N)�H[N] ") == 'y' ?
      MQ_UUENCODE : 0;
#endif
  }

  hdr = tag ? &xhdr : (HDR *) xo_pool + xo->pos - xo->top;

  dir = xo->dir;
  title = hdr->title;
  locus = cc = 0;

  do
  {
    if (tag)
      EnumTagHdr(hdr, dir, locus++);

    // check delete or not .. by statue 2000/05/18
    if(hdr->xmode & (POST_DELETE | POST_CANCEL | POST_MDELETE))
      continue;

    if((hdr->xmode & (POST_LOCK|GEM_RESTRICT|GEM_RESERVED)) && !(HAS_PERM(PERM_ALLBOARD) || (bbstate & STAT_BOARD)))
      continue;
    
    if((hdr->xmode & GEM_LOCK) && !HAS_PERM(PERM_SYSOP))
      continue;

    if (!(hdr->xmode & GEM_FOLDER))	/* �d hdr �O�_ plain text */
    {
      xo_fpath(fpath, dir, hdr);

      if (method >= MF_SELF)
      {
	HDR mhdr;

	if ((cc = hdr_stamp(folder, HDR_LINK, &mhdr, fpath)) < 0)
	  break;

	if (method == MF_SELF)
	{
	  strcpy(mhdr.owner, "[�� �� ��]");
	  mhdr.xmode = MAIL_READ | MAIL_NOREPLY;
	}
	else
	{
	  strcpy(mhdr.owner, userid);
	}
	strcpy(mhdr.nick, cuser.username);
	strcpy(mhdr.title, title);
	if ((cc = rec_add(folder, &mhdr, sizeof(HDR))) < 0)
	  break;
      }
      else
      {
	if ((cc = bsmtp(fpath, title, rcpt, method)) < 0)
	  break;
      }
    }
  } while (locus < tag);

#undef	MF_SELF
#undef	MF_USER

  if(check)
    strcpy(rcpt,cuser.email);

  if (userno > 0)
    m_biff(userno);

  zmsg(cc < 0 ? "�����H��L�k�H�F" : "�H�H����");

  return XO_FOOT;
}

#if 0
static void
z_download(fpath)
  char *fpath;
{
  static int num = 0;		/* �y���� */
  int pid, status;
  char buf[64];

  /* Thor.0728: �� refresh�@�U, �A�ݬݷ|���|�� */

  move(b_lines, 0);
  clrtoeol();
  refresh();
  move(b_lines, 0);
  outc('\n');
  refresh();

  sprintf(buf, "tmp/%.8s.%03d", cuser.userid, ++num);
  f_cp(fpath, buf, O_TRUNC);
  if (pid = fork())
    waitpid(pid, &status, 0);
  else
  {
    execl("bin/sz", "-a", buf, NULL);
    exit(0);
  }
  unlink(buf);
}


static int
xo_zmodem(xo)
  XO *xo;
{
  char fpath[128], *dir;
  HDR *hdr, xhdr;
  int tag, locus;

  if (!HAS_PERM(PERM_FORWARD))
    return XO_NONE;

  tag = AskTag("Z-modem �U��");
  if (tag < 0)
    return XO_FOOT;

  if (tag)
    hdr = &xhdr;
  else
    hdr = (HDR *) xo_pool + xo->pos - xo->top;

  locus = 0;
  dir = xo->dir;

  do
  {
    if (tag)
      EnumTagHdr(hdr, dir, locus++);

    // check delete or not .. by statue 2000/05/18
    if(hdr->xmode & (POST_DELETE | POST_CANCEL | POST_MDELETE))
      continue;

    if (hdr->xmode & (POST_LOCK|GEM_LOCK|GEM_RESERVED|GEM_RESTRICT))
      continue;


    if (!(hdr->xmode & GEM_FOLDER))	/* �d hdr �O�_ plain text */
    {
      xo_fpath(fpath, dir, hdr);
      z_download(fpath);
    }
  } while (locus < tag);

  return XO_HEAD;
}
#endif

/* ----------------------------------------------------- */
/* �峹�@�̬d�ߡB�v���]�w				 */
/* ----------------------------------------------------- */

/* 090929.cache: ²���� */
int
xo_uquery_lite(xo)
  XO *xo;
{
  HDR *hdr;
  char *userid;

  hdr = (HDR *) xo_pool + (xo->pos - xo->top);
  if (hdr->xmode & (GEM_GOPHER | POST_INCOME | MAIL_INCOME))
    return XO_NONE;

  userid = hdr->owner;
  if (strchr(userid, '.'))
    return XO_NONE;

  grayout(GRAYOUT_DARK);

  move(b_lines - 8, 0);
  clrtobot();  /* �קK�e���ݯd */

  prints("\033[1;34m"MSG_BLINE"\033[m");    
  prints("\n\033[1;33;44m \033[37m�峹�@�̤θ�T�d�ߡG %*s \033[m\n", 55,"");
  prints("\n");
//  clrtobot();
  /* cpos = xo->pos; */		/* chuan �O�d xo->pos ���ȡA����^�s */

  my_query(userid, 2);

  move(b_lines - 1, 0);
  clrtobot();  /* �קK�e���ݯd */
  prints("\n");
          
  /* xo->pos = cpos; */
  return XO_HEAD;
}

int
xo_uquery(xo)
  XO *xo;
{
  HDR *hdr;
  char *userid;

  hdr = (HDR *) xo_pool + (xo->pos - xo->top);
  if (hdr->xmode & (GEM_GOPHER | POST_INCOME | MAIL_INCOME))
    return XO_NONE;

  userid = hdr->owner;
  if (strchr(userid, '.'))
    return XO_NONE;

  move(1, 0);
  clrtobot();
//  move(2, 0);
  /* cpos = xo->pos; */		/* chuan �O�d xo->pos ���ȡA����^�s */
  my_query(userid, 0);
  /* xo->pos = cpos; */
  return XO_HEAD;
}


int
xo_usetup(xo)
  XO *xo;
{
  HDR *hdr;
  char *userid;
  ACCT xuser;

  if (!HAVE_PERM(PERM_SYSOP | PERM_ACCOUNTS))
    return XO_NONE;

  hdr = (HDR *) xo_pool + (xo->pos - xo->top);
  userid = hdr->owner;
  if (strchr(userid, '.') || (acct_load(&xuser, userid) < 0))
    return XO_NONE;

  move(2, 0);
  acct_setup(&xuser, 1);
  return XO_HEAD;
}


/* ----------------------------------------------------- */
/* �D�D���\Ū						 */
/* ----------------------------------------------------- */


#define RS_TITLE        0x001	/* author/title */
#define RS_FORWARD      0x002	/* backward */
#define RS_RELATED      0x004
#define RS_FIRST        0x008	/* find first article */
#define RS_CURRENT      0x010	/* match current read article */
#define RS_THREAD	0x020	/* search the first article */
#define RS_SEQUENT	0x040	/* sequential read */
#define RS_MARKED 	0x080	/* marked article */
#define RS_UNREAD 	0x100	/* unread article */

#define CURSOR_FIRST	(RS_RELATED | RS_TITLE | RS_FIRST)
#define CURSOR_NEXT	(RS_RELATED | RS_TITLE | RS_FORWARD)
#define CURSOR_PREV	(RS_RELATED | RS_TITLE)
#define RELATE_FIRST	(RS_RELATED | RS_TITLE | RS_FIRST | RS_CURRENT)
#define RELATE_NEXT	(RS_RELATED | RS_TITLE | RS_FORWARD | RS_CURRENT)
#define RELATE_PREV	(RS_RELATED | RS_TITLE | RS_CURRENT)
#define THREAD_NEXT	(RS_THREAD | RS_FORWARD)
#define THREAD_PREV	(RS_THREAD)

/* Thor: �e���mark�峹, ��K���D��������D���B�z */

#define MARK_NEXT	(RS_MARKED | RS_FORWARD | RS_CURRENT)
#define MARK_PREV	(RS_MARKED | RS_CURRENT)


typedef struct
{
  int key;			/* key stroke */
  int map;			/* the mapped threading op-code */
}      KeyMap;


static KeyMap keymap[] =
{
  /* search title / author */

  {'"', RS_TITLE | RS_FORWARD},
  {'?', RS_TITLE},
  {'a', RS_FORWARD},
  {'A', 0},

  /* thread : currtitle */

  {'[', RS_RELATED | RS_TITLE | RS_CURRENT},
  {']', RS_RELATED | RS_TITLE | RS_FORWARD | RS_CURRENT},
  {'=', RS_RELATED | RS_TITLE | RS_FIRST | RS_CURRENT},

  /* i.e. < > : make life easier */

  {',', RS_THREAD},
  {'.', RS_THREAD | RS_FORWARD},

  /* thread : cursor */

  {'-', RS_RELATED | RS_TITLE},
  {'+', RS_RELATED | RS_TITLE | RS_FORWARD},
  {'\\', RS_RELATED | RS_TITLE | RS_FIRST},

  /* Thor: marked : cursor */
  {'\'', RS_MARKED | RS_FORWARD | RS_CURRENT},
  {';', RS_MARKED | RS_CURRENT},

  /* Thor: �V�e��Ĥ@�g��Ū���峹 */
  /* Thor.980909: �V�e�䭺�g��Ū, �Υ��g�wŪ */
  {'`', RS_UNREAD /* | RS_FIRST */},

  /* sequential */

  {' ', RS_SEQUENT | RS_FORWARD},
  {KEY_RIGHT, RS_SEQUENT | RS_FORWARD},
  {KEY_PGDN, RS_SEQUENT | RS_FORWARD},
  {KEY_DOWN, RS_SEQUENT | RS_FORWARD},
  /* Thor.990208: ���F��K�ݤ峹�L�{��, ���ܤU�g, ���M�W�h�Qxover�Y���F:p */
  {'j', RS_SEQUENT | RS_FORWARD},

  {KEY_UP, RS_SEQUENT},
  {KEY_PGUP, RS_SEQUENT},
  /* Thor.990208: ���F��K�ݤ峹�L�{��, ���ܤW�g, ���M�W�h�Qxover�Y���F:p */
  {'k', RS_SEQUENT},

  /* end of keymap */

  {(char) NULL, -1}
};


static int
xo_keymap(key)
  int key;
{
  KeyMap *km;
  int ch;

  km = keymap;
  while ((ch = km->key))
  {
    if (ch == key)
      break;
    km++;
  }
  return km->map;
}


static int
xo_thread(xo, op)
  XO *xo;
  int op;
{
  static char s_author[16], s_title[32], s_unread[2]="0";
  char buf[80];

  char *tag, *query=NULL, *title=NULL;
  int pos, match, near=0, neartop=0, max;	/* Thor: neartop�Pnear����� */

  int fd, top, bottom, step, len;
  HDR *pool, *fhdr;

  match = XO_NONE;
  pos = xo->pos;
  top = xo->top;
  pool = (HDR *) xo_pool;
  fhdr = pool + (pos - top);
  step = (op & RS_FORWARD) - 1;

  if (op & RS_RELATED)
  {
    tag = fhdr->title;

    if (op & RS_CURRENT)
    {
      query = currtitle;
      if (op & RS_FIRST)
      {
	if (!strcmp(query, tag))/* �ثe���N�O�Ĥ@���F */
	  return XO_NONE;
	near = -1;
      }
    }
    else
    {
      title = str_ttl(tag);
      if (op & RS_FIRST)
      {
	if (title == tag)
	  return XO_NONE;
	near = -1;
      }
      query = buf;
      strcpy(query, title);
    }
  }
  else if (op & RS_UNREAD)	/* Thor: �V�e��M�Ĥ@�g��Ū�峹,�M near */
  {
#define	RS_BOARD	0x1000	/* �Ω� RS_UNREAD�A��e�������i���| */
    /* Thor.980909: �߰� "���g��Ū" �� "���g�wŪ" */
    if (!vget(b_lines, 0, "�V�e��M 0)���g��Ū 1)���g�wŪ ", s_unread, sizeof(s_unread), GCARRY))
      return XO_FOOT; /* Thor.980911: ����, �h�S�MXO_FOOT, �A�ݬݫ��� */

    if (*s_unread == '0') 
      op |= RS_FIRST;  /* Thor.980909: �V�e��M���g��Ū */

    near = xo->dir[0];
    if (near == 'b')		/* search board */
      op |= RS_BOARD;
    else if (near != 'u')	/* search user's mbox */
      return XO_NONE;

    near = -1;
  }
  else if (!(op & (RS_THREAD | RS_SEQUENT | RS_MARKED)))
  {
    if (op & RS_TITLE)
    {
      title = "���D";
      tag = s_title;
      len = sizeof(s_title);
    }
    else
    {
      title = "�@��";
      tag = s_author;
      len = sizeof(s_author);
    }
    query = buf;
    sprintf(query, "�j�M%s(%s)�G", title, (step > 0) ? "��" : "��");
    if (!vget(b_lines, 0, query, tag, len, GCARRY))
      return XO_FOOT;
    /* Thor.980911: �n�`�N, �p�G�S���, "�j�M"���T���|�Q�M,
                    �p�G���F, �h�S�Q�M, �]�Ǧ^�Ȭ�match, �S�k�a XO_FOOT */

    str_lower(query, tag);
  }

  fd = -1;
  len = sizeof(HDR) * XO_TALL;
  bottom = top + XO_TALL;
  max = xo->max;
  if (bottom > max)
    bottom = max;

  for (;;)
  {
    if (step > 0)
    {
      if (++pos >= max)
	break;
    }
    else
    {
      if (--pos < 0)
	break;
    }

    /* buffer I/O : shift sliding window scope */

    if ((pos < top) || (pos >= bottom))
    {
      if (fd < 0)
      {
	fd = open(xo->dir, O_RDONLY);
	if (fd < 0)
	  return XO_QUIT;
      }

      if (step > 0)
      {
	top += XO_TALL;
	bottom = top + XO_TALL;
	if (bottom > max)
	  bottom = max;
      }
      else
      {
	bottom = top;
	top -= XO_TALL;
      }

      lseek(fd, (off_t) (sizeof(HDR) * top), SEEK_SET);
      read(fd, pool, len);

      fhdr = (step > 0) ? pool : pool + XO_TALL - 1;
    }
    else
    {
      fhdr += step;
    }

    /* ���L�w�R�� or lock �峹 */
    if(fhdr->xmode & (POST_CANCEL | POST_DELETE | POST_MDELETE | POST_LOCK))
      continue;

    if (op & RS_SEQUENT)
    {
      match = -1;
      break;
    }

    /* Thor: �e�� search marked �峹 */

    if (op & RS_MARKED)
    {
      if (fhdr->xmode & (POST_MARKED /* | POST_GEM */))
      {
	match = -1;
	break;
      }
      continue;
    }

    /* �V�e��M�Ĥ@�g��Ū�峹 */

    if (op & RS_UNREAD)
    {
      /* Thor.980909: ���g��Ū(RS_FIRST) �P ���g�wŪ(!RS_FIRST) */
      if (op & RS_BOARD)
      {
        /* if (!brh_unread(fhdr->chrono)) */
//        if (!(op & RS_FIRST) ^ !brh_unread(fhdr->chrono))
        if (!(op & RS_FIRST) ^ !brh_unread(BMAX(fhdr->chrono, fhdr->stamp)))
   	  continue;
      }
      else
      {
  	/* if ((fhdr->xmode & MAIL_READ) */
  	if (!(op & RS_FIRST) == !(fhdr->xmode & MAIL_READ))
	  continue;
      }

#undef	RS_BOARD
      /* Thor.980909: ���g�wŪ(!RS_FIRST) */
      if (!(op & RS_FIRST))
      {
        match = -1;
        break;
      }

	near = pos;		/* Thor:�O�U�̱���_�I����m */
	neartop = top;
      continue;
    }

    /* ------------------------------------------------- */
    /* �H�U�j�M title / author				 */
    /* ------------------------------------------------- */

    if (op & (RS_TITLE | RS_THREAD))
    {
      title = fhdr->title;	/* title ���V [title] field */
      tag = str_ttl(title);	/* tag ���V thread's subject */

      if (op & RS_THREAD)
      {
	if (tag == title)
	{
	  match = -1;
	  break;
	}
	continue;
      }
    }
    else
    {
      tag = fhdr->owner;	/* tag ���V [owner] field */
    }

    if (((op & RS_RELATED) && !strncmp(tag, query, 40)) ||
      (!(op & RS_RELATED) && str_str(tag, query)))
    {
      if (op & RS_FIRST)
      {
	if (tag != title)
	{
	  near = pos;		/* �O�U�̱���_�I����m */
	  neartop = top;
	  continue;
	}
      }

#if 0
      if ((!(op & RS_CURRENT)) && (op & RS_RELATED) &&
	strncmp(currtitle, query, TTLEN))
      {
	str_ncpy(currtitle, query, TTLEN);
	match = XO_BODY;
      }
      else
#endif

	match = -1;
      break;
    }
  }

  bottom = xo->top;

  if (match < 0)
  {
    xo->pos = pos;
    if (bottom != top)
    {
      xo->top = top;
      match = XO_BODY;		/* ���F�A�åB�ݭn��s�e�� */
    }
  }				/* Thor: �[�W RS_FIRST�\�� */
  else if ((op & RS_FIRST) && near >= 0)
  {
    xo->pos = near;
    if (top != neartop)		/* Thor.0609: top ���ثe��buffer��top */
    {
      lseek(fd, (off_t) (sizeof(HDR) * neartop), SEEK_SET);
      read(fd, pool, len);
    }
    if (bottom != neartop)	/* Thor.0609: bottom���e����top */
    {
      xo->top = neartop;
      match = XO_BODY;		/* ���F�A�åB�ݭn��s�e�� */
    }
    else
      match = -1;
  }
  else if (bottom != top)
  {
    lseek(fd, (off_t) (sizeof(HDR) * bottom), SEEK_SET);
    read(fd, pool, len);
  }

  if (fd >= 0)
    close(fd);

  return match;
}


/* Thor.990204: ���Ҽ{more �Ǧ^��, �H�K�ݤ@�b�i�H�� []... 
                ch �����emore()���ҫ���key */   
int
xo_getch(xo, ch)
  XO *xo;
  int ch;
{
  int op;

  if (!ch)
    ch = vkey();

  op = xo_keymap(ch);
  if (op >= 0)
  {
    ch = xo_thread(xo, op);
    if (ch != XO_NONE)
      ch = XO_BODY;		/* �~���s�� */
  }

#if 0
  else
  {
    if (ch == KEY_LEFT || ch == 'Q')
      ch = 'q';
  }
#endif

  return ch;
}


static int
xo_jump(pos)			/* ���ʴ�Ш� number �Ҧb���S�w��m */
  int pos;
{
  char buf[6];

  buf[0] = pos;
  buf[1] = '\0';
  vget(b_lines, 0, "���ܲĴX���G", buf, sizeof(buf), GCARRY);
  move(b_lines, 0);
  clrtoeol();
  pos = atoi(buf);
  if (pos >= 0)
    return XO_MOVE + pos - 1;
  return XO_NONE;
}


/* ----------------------------------------------------- */
/* ----------------------------------------------------- */

#ifdef XZ_XPOST
/* Thor.990303: �p�G�� XZ_XPOST���� */
extern KeyFunc xpost_cb[];
#endif
extern KeyFunc post_cb[];


XZ xz[] =
{
  {NULL, NULL, M_BOARD},	/* XZ_CLASS */
  {NULL, NULL, M_LUSERS},	/* XZ_ULIST */
  {NULL, NULL, M_PAL},		/* XZ_PAL */
  {NULL, NULL, M_VOTE},		/* XZ_VOTE */
  {NULL, NULL, M_BMW},          /* XZ_BMW */    /* lkchu.981230: BMW �s���� */
#ifdef XZ_XPOST /* Thor.990303: �p�G�� XZ_XPOST���� */
  {NULL, xpost_cb, M_READA},		/* XZ_XPOST */
#else
  {NULL, NULL, M_READA},		/* skip XZ_XPOST */
#endif
  {NULL, NULL, M_RMAIL},	/* XZ_MBOX */
  {NULL, post_cb, M_READA},		/* XZ_POST */
  {NULL, NULL, M_GEM},		/* XZ_GEM */
  {NULL, NULL, M_RMAIL},	/* XZ_MAILGEM */
  {NULL, NULL, M_BANMAIL},	/* XZ_BANMAIL */
  {NULL, NULL, M_OMENU},	/* XZ_OTHER */
};


/* ----------------------------------------------------- */
/* interactive menu routines			 	 */
/* ----------------------------------------------------- */


void
xover(cmd)
  int cmd;
{
  int pos;
  int num=0;
  int zone=0;
  int sysmode=0;
  XO *xo=NULL;
  KeyFunc *xcmd=NULL;
  KeyFunc *cb;
  
   
#if 1
  /* Thor.0613: ���U�T�� */
  static int msg = 0;
#endif

  if(xo_user_level >= MAX_LEVEL)
  {
    vmsg("�w�g�W�L�̤j�h�ơA�����D�гq�� root �I");
    return;
  }

  xo_user_level++;


  for (;;)
  {
    while ((cmd != XO_NONE))
    {
      if (cmd == XO_FOOT)
      {
	move(b_lines, 0);
	clrtoeol();

	/* Thor.0613: ���U�T���M�� */
	msg = 0;

	break;
      }

      if (cmd >= XO_ZONE)
      {
	/* --------------------------------------------- */
	/* switch zone					 */
	/* --------------------------------------------- */


	zone = cmd;
	cmd -= XO_ZONE;
	xo = xz[cmd].xo;
	xcmd = xz[cmd].cb;
	sysmode = xz[cmd].mode;

	TagNum = 0;		/* clear TagList */
	cmd = XO_INIT;
	utmp_mode(sysmode);
      }
      else if (cmd >= XO_MOVE - XO_TALL)
      {
	/* --------------------------------------------- */
	/* calc cursor pos and show cursor correctly	 */
	/* --------------------------------------------- */

	/* cmd >= XO_MOVE - XO_TALL so ... :chuan: ���] cmd = -1 ?? */

	/* fix cursor's range */

	num = xo->max - 1;
	pos = (cmd | XO_WRAP) - (XO_MOVE + XO_WRAP);
	cmd &= XO_WRAP;

	if (pos < 0)
	{
	  pos = cmd ? num : 0;
	  /* pos = 0; :chuan: */
	}
	else if (pos > num)
	{
	  if(!(cuser.ufo2 & UFO2_CIRCLE) && (zone == XZ_POST || zone == XZ_XPOST))
	    pos = num;
	  else
            pos = cmd ? 0 : num;
	  /* pos = num; :chuan: */
	}

	/* check cursor's range */

	cmd = xo->pos;

	if (cmd == pos)
	  break;

	xo->pos = pos;
	num = xo->top;
	if ((pos < num) || (pos >= num + XO_TALL))
	{
	  xo->top = (pos / XO_TALL) * XO_TALL;
	  cmd = XO_LOAD;	/* ���J��ƨä��H��� */
	}
	else
	{


	  move(3 + cmd - num, 0);
	  outc(' ');

	  break;		/* �u���ʴ�� */
	}
      }

      /* ----------------------------------------------- */
      /* ���� call-back routines			 */
      /* ----------------------------------------------- */

      cb = xcmd;
      num = cmd | XO_DL; /* Thor.990220: for dynamic load */
      for (;;)
      {
	pos = cb->key;
#if 1
        /* Thor.990220: dynamic load , with key | XO_DL */
        if (pos == num)
        {
          void *p = DL_get((char *) cb->func);
          if(p) 
          {
            cb->func = p;
            pos = cb->key = cmd;
          }
          else
          {
            cmd = XO_NONE;
            break;
          }
        }
#endif
	if (pos == cmd)
	{
	  cmd = (*(cb->func)) (xo);

	  if (cmd == XO_QUIT)
	  {  
	    xo_user_level--;
	    return;
	  }

	  break;
	}
	if (pos == 'h')
	  break;

	cb++;
      }

      if (pos == 'h')
	break;


#if 0
      if (pos >= XO_INIT && pos <= XO_BODY)
      {
	if (xo->top + XO_TALL == xo->max)
	{
	  /*outz("\033[44m �����ڬݥ����F! ^O^ \033[m");*/	/* Thor.0616 */
	  msg = 1;
	}
	else if (msg)
	{
	  move(b_lines, 0);
	  clrtoeol();
	  msg = 0;
	}
      }
#endif

    } /* Thor.990220:����: end of while(cmd!=XO_NONE) */


    utmp_mode(sysmode); 
    /* Thor.990220:����:�ΨӦ^�_ event handle routine �^�ӫ᪺�Ҧ� */

    pos = xo->pos;

    if (xo->max > 0)		/* Thor:�Y�O�L�F��N��show�F */
    {
      num = 3 + pos - xo->top;


      move(num, 0);
      outc('>');
    }

    cmd = vkey();
    
    /* ------------------------------------------------- */
    /* switch Zone					 */
    /* ------------------------------------------------- */
    if(cmd == Ctrl('B'))
    {
      every_B();
      cmd = XO_INIT;
      continue;
    }
    if (cmd == Ctrl('U') && zone != XZ_ULIST)
    {
      every_U();
      cmd = XO_INIT;
      continue;
    }
    
    
    if (cmd == Ctrl('Z'))
    {
      every_Z();
      cmd = XO_INIT;
      /* cmd = XO_FOOT;*/            /* by visor : �ץ� ���D bug */
#if 0
      /* switch (vans(MSG_ZONE_SWITCH)) */
      /* Thor.980921: �֤@����ո� */
      outz(MSG_ZONE_SWITCH);
      switch(vkey())
      {
      case 'a':
	cmd = XZ_GEM;
	break;

      case 'b':
	if (xz[XZ_POST - XO_ZONE].xo)
	{
	  cmd = XZ_POST;
	  break;
	}

      case 'c':
	cmd = XZ_CLASS;
	break;

      case 'm':
        /* Thor.981022: �����S���v���i�H�c */
	if (HAS_PERM(PERM_BASIC) /*cuser.userlevel*/)
	  cmd = XZ_MBOX;
	else
	  cmd = zone;
	break;

      case 'u':
	cmd = XZ_ULIST;
	break;

#if 1
      /* lkchu.981230: �Q�� xover ��X bmw */
      case 'w':
        cmd = XZ_BMW;
        break;
#endif

      default:
	cmd = XO_FOOT;
	continue;
      }

      if (zone == cmd)		/* ���Ӫ��@�� */
      {
	cmd = XO_FOOT;
      }
#endif

    }
    /* ------------------------------------------------- */
    /* �򥻪���в��� routines				 */
    /* ------------------------------------------------- */

    else if (cmd == KEY_LEFT /*|| cmd == 'q'*/)
    {
      /* cmd = XO_LAST; *//* try to load the last XO in future */
      if(zone == XZ_MBOX)
      {
        
#ifdef HAVE_MAILUNDELETE
        int deltotal;
        char fpath[256];

        if ((deltotal = mbox_check()))
        {
          sprintf(fpath,"�� %d �ʫH��N�n�R���A�T�w�ܡH [y/N]",deltotal);
          if(vans(fpath) == 'y')
          {
            usr_fpath(fpath,cuser.userid,FN_DIR);
            hdr_prune(fpath, 0, 0 , 3);
          }
        }
#endif        
        if(mail_stat(CHK_MAIL_VALID))
        {
          vmsg("�A���H�c�e�q�W�L�W���A�о�z�I");
          chk_mailstat = 1;
          continue;
        }
        else
          chk_mailstat = 0;
      }
      xo_user_level--;
      return;
    }
    else if (xo->max <= 0)	/* Thor: �L�F��h�L�k����� */
    {
      continue;
    }
    else if (cmd == KEY_UP /*|| cmd == 'p' || cmd == 'k'*/)
    {
      cmd = pos - 1 + XO_MOVE + XO_WRAP;
    }
    else if (cmd == KEY_DOWN /*|| cmd == 'n' || cmd == 'j'*/)
    {
      cmd = pos + 1 + XO_MOVE + XO_WRAP;
    }
    else if (cmd == ' ' || cmd == KEY_PGDN || cmd == 'N'  /*|| cmd == Ctrl('F') */)
    {				       /* lkchu.990428: ���u�Ȯɧ��ӷ��v�� */
      cmd = pos + XO_MOVE + XO_TALL;
    }
    else if (cmd == KEY_PGUP || cmd == 'P' /*|| cmd == Ctrl('B')*/)
    {
      cmd = pos + XO_MOVE - XO_TALL;
    }
    else if (cmd == KEY_HOME || cmd == '0')
    {
      cmd = XO_MOVE;
    }
    else if (cmd == KEY_END || cmd == '$')
    {
      /* cmd = xo->max + XO_MOVE; */
      cmd = XO_MOVE + XO_WRAP - 1;	/* force re-load */
    }
    else if (cmd >= '1' && cmd <= '9')
    {
      cmd = xo_jump(cmd);
    }
    else 
    {
      /* ----------------------------------------------- */
      /* keyboard mapping				 */
      /* ----------------------------------------------- */

      if (cmd == KEY_RIGHT || cmd == '\n')
      {
	if(zone == XZ_ULIST)
	  cmd = 'q'; //�ϥΪ̦W��| Q 
	else
	  cmd = 'r';
      }
#ifdef XZ_XPOST
      else if (zone >= XZ_XPOST && zone < XZ_BANMAIL/* XZ_MBOX */ )
#else
      else if (zone >= XZ_MBOX && zone < XZ_BANMAIL)
#endif
      {
	/* --------------------------------------------- */
	/* Tag						 */
	/* --------------------------------------------- */

	if (cmd == 'C')
	{
	  cmd = xo_copy(xo);
	}
//	else if (cmd == 'F')
//	{
//	  cmd = xo_forward(xo);
//	}
#if 0
	else if (cmd == 'Z')
	{
	  cmd = xo_zmodem(xo);
	}
#endif
	else if (cmd == Ctrl('C'))
	{
	  extern int TagNum;

	  if (TagNum)
	  {
	    TagNum = 0;
	    cmd = XO_BODY;
	  }
	  else
	    cmd = XO_NONE;
	}
	else if (cmd == Ctrl('A') || cmd == Ctrl('T'))
	{
	  cmd = xo_tag(xo, cmd);
	}
	else if (cmd == Ctrl('D') && zone < XZ_GEM)
	{
	  /* ��ذϭn�v�@�R��, �H�קK�~�� */

	  cmd = xo_prune(xo);
	}
	else if (cmd == 'g' && (bbstate & STAT_BOARD))
	{ /* Thor.980806: �n�`�N�S�i�ݪ�(���w�ݪ�)��, bbstate�|�S��STAT_BOARD
                          �����|�L�k�����峹 */
	  cmd = gem_gather(xo);		/* �����峹���ذ� */
	}
#ifdef	HAVE_MAILGEM	
	else if(cmd == 'G' && HAS_PERM(PERM_MBOX))
	{
	  static int (*mgp)();
	  if(!mgp)
	  {
	    mgp = DL_get("bin/mailgem.so:mailgem_gather");
	    if(mgp)
	      cmd = (*mgp)(xo);
	    else
	      vmsg("�ʺA�s�����ѡA���p���t�κ޲z���I");        
	  }
	  else
            cmd = (*mgp)(xo);
	}
#endif
	/* --------------------------------------------- */
	/* �D�D���\Ū					 */
	/* --------------------------------------------- */

#ifdef XZ_XPOST
        if (zone == XZ_XPOST)
	  continue;
#endif

	pos = xo_keymap(cmd);
	if (pos >= 0)
	{
	  cmd = xo_thread(xo, pos);

#if 1
	  if (cmd == XO_NONE)
	  {			/* Thor.0612: ��S���άO �w�g�O�F,��Ф��Q�� */
	    outz("\033[44m ��S���F�C...:( \033[m");
	    msg = 1;
	  }
	  else if (msg)
	  {
	    move(b_lines, 0);
	    clrtoeol();
	    msg = 0;
	  }
#endif

	  if (cmd < 0)
	  {


	    move(num, 0);
	    outc(' ');
	    cmd = XO_NONE;
	  }
	}
      }
      /* ----------------------------------------------- */
      /* ��L���浹 call-back routine �h�B�z		 */
      /* ----------------------------------------------- */

    } /* Thor.990220:����: end of vkey() handling */
  }
}


/* ----------------------------------------------------- */
/* Thor.0725: ctrl Z everywhere				 */
/* ----------------------------------------------------- */


#ifdef	EVERY_Z
static void
every_Z_Orig()
{
  int cmd;
  char select;
  screenline sl[b_lines + 1];
  

  save_foot(sl);
  cmd = 0;

  outz(MSG_ZONE_SWITCH);
  select = vkey();
  
#ifdef	HAVE_FAVORITE  
  if(select == 'p')
  {
    outz(MSG_ZONE_ADVANCE);
    select = vkey();
    if(select != 'w' && select != 'u')
      select = ' ';
  }
#endif  
  
  switch(select)
  {
#ifdef  HAVE_FAVORITE  
    case 'f':
      restore_foot(sl);
      Favorite();
      break;
#endif      
    case 'a':
      cmd = XZ_GEM;
      break;

    case 'b':
      if (xz[XZ_POST - XO_ZONE].xo)
      {
	cmd = brd_bno(currboard);
	XoPost(cmd);
        cmd = XZ_POST;
        break;
      }

    case 'c':
      cmd = XZ_CLASS;
      break;

    case 'm':
      if (HAS_PERM(PERM_BASIC) && !HAS_PERM(PERM_DENYMAIL))
        cmd = XZ_MBOX;
      break;

    case 'u':
      cmd = XZ_ULIST;
      break;

    case 'w':
      cmd = XZ_BMW;
      break;

    default:
      break;
  }

  restore_foot(sl);

  if (cmd)
    xover(cmd);
}




static int
Every_Z_Favorite()
{
  Favorite();
  return 0;
}

static int
Every_Z_Gem()
{
  xover(XZ_GEM);
  return 0;
}

static int
Every_Z_Ulist()
{
  xover(XZ_ULIST);
  return 0;
}

static int
Every_Z_Board()
{
  if (xz[XZ_POST - XO_ZONE].xo)
  {
    xover(XZ_POST);
  }
  else
    vmsg("�|����ܬݪO");

  return 0;
}

static int
Every_Z_Class()
{
  xover(XZ_CLASS);
  return 0;
}

static int
Every_Z_MBox()
{
  if (HAS_PERM(PERM_BASIC) && !HAS_PERM(PERM_DENYMAIL))
    xover(XZ_MBOX);
  else
    vmsg("�v�������άO�Q���v��");
  return 0;
}

static int
Every_Z_BMW()
{
  xover(XZ_BMW);
  return 0;
}

extern int Every_Z_Screen();

static MENU menu_everyz[] =
{
#ifdef HAVE_FAVORITE
  {Every_Z_Favorite, PERM_VALID, POPUP_FUN,
  "Favorite �ڪ��̷R"},
#endif

  {Every_Z_Gem, 0, POPUP_FUN,
  "Gem      ��ذ�"},

  {Every_Z_Ulist, 0, POPUP_FUN,
  "Ulist    �ϥΪ̦W��"},

  {Every_Z_Board, 0, POPUP_FUN,
  "Post     �峹�C��"},

  {Every_Z_Class, 0, POPUP_FUN,
  "Class    �ݪO�C��"},

  {Every_Z_MBox, PERM_BASIC, POPUP_FUN,
  "Mail     �H�c"},

  {Every_Z_BMW, 0, POPUP_FUN,
  "Bmw      ���T����"},

  {Every_Z_Screen, 0, POPUP_FUN,
  "Screen   �ù��^��"},

  {"bin/dictd.so:xover_dict", 0 , POPUP_SO,
  "Dict     ���^��r��"},

  {NULL, 0, POPUP_QUIT,
  "Quit     ���}"},

  {"�ֳt���", POPUP_DO_INSTANT + 'U', POPUP_MENUTITLE,
  "�ֳt������"}
};

void
every_Z()
{
  int tmpmode,savemode;
  screenline sl[b_lines + 1];
  int tmpbno;
  XZ xy;


#ifdef  HAVE_CHK_MAILSIZE
 if(chk_mailstat == 1)
 {
   if(mail_stat(CHK_MAIL_VALID))
   {
     vmsg("�z���H�c�w�W�X�e�q�A�L�k�ϥΥ��\\��A�вM�z�z���H�c�I");
     return;
   }
   else
     chk_mailstat = 0;
 }
#endif

  
  
  memcpy(&xy,&(xz[XZ_OTHER - XO_ZONE]),sizeof(XZ));

  tmpbno = currbno;

  if(xo_stack_level < XO_STACK)
    xo_stack_level++;
  else
  {
    vmsg("�w�F��̤j�W�����|�Ŷ��I");
    return;
  }

  savemode = boardmode;
  tmpmode = bbsmode;
  vs_save(sl);


  if( cuser.ufo2 & UFO2_ORIGUI)
    every_Z_Orig();
  else
    popupmenu(menu_everyz,NULL,7,20);

  memcpy(&(xz[XZ_OTHER - XO_ZONE]),&xy,sizeof(XZ));

  if(tmpbno >= 0)
    XoPost(tmpbno);

  vs_restore(sl);
  utmp_mode(tmpmode);
  
  xo_stack_level--;
  boardmode = savemode;

}  


#endif


void
every_U()
{
  int cmd, tmpmode;
  screenline sl[b_lines + 1];
  XZ xy;

#ifdef  HAVE_CHK_MAILSIZE
 if(chk_mailstat == 1)
 {
   if(mail_stat(CHK_MAIL_VALID))
   {
     vmsg("�z���H�c�w�W�X�e�q�A�L�k�ϥΥ��\\��A�вM�z�z���H�c�I");
     return;
   }
   else
     chk_mailstat = 0;
 }
#endif

   extern int pickup_way;
   int tmpway = pickup_way;                                                             
   if (bbsmode == M_READA)  /* guessi.061218: �i�J�ݪO�� ^U �w�]�ƦC */
     pickup_way = 1;
  
  memcpy(&xy,&(xz[XZ_OTHER - XO_ZONE]),sizeof(XZ));

  cmd = XZ_ULIST;
  tmpmode = bbsmode;
  vs_save(sl);
  xover(cmd);
  vs_restore(sl);

  memcpy(&(xz[XZ_OTHER - XO_ZONE]),&xy,sizeof(XZ));

  utmp_mode(tmpmode);

  pickup_way = tmpway; /* �٭���e�]�w���ƦC�覡 */

  return;
}

void
every_B()
{
  screenline sl[b_lines + 1];
  int tmpmode,stat;

  stat = bbstate;
  tmpmode = bbsmode;
  vs_save(sl);
 
  u_lock();

  vs_restore(sl);
  bbstate = stat;

  utmp_mode(tmpmode);
  return;
}

void
every_S()
{
  int tmpmode;
  screenline sl[b_lines + 1];

  tmpmode = bbsmode;
  vs_save(sl);
  Select();
  vs_restore(sl);
  utmp_mode(tmpmode);

  return;
}
/* ----------------------------------------------------- */
/* �� XZ_* ���c����в���				 */
/* ----------------------------------------------------- */


/* �ǤJ: ch, pagemax, num, pageno, cur, redraw */
/* �ǥX: ch, pageno, cur, redraw */
int
xo_cursor(ch, pagemax, num, pageno, cur, redraw)
  int ch, pagemax, num;
  int *pageno, *cur, *redraw;
{
  switch (ch)
  {
  case KEY_LEFT:
  case 'q':
    return 'q';

  case KEY_PGUP:
    if (pagemax != 0)
    {
      if (*pageno)
      {
	(*pageno)--;
      }
      else
      {
	*pageno = pagemax;
	*cur = num % XO_TALL;
      }
      *redraw = 1;
    }
    break;

  case KEY_PGDN:
    if (pagemax != 0)
    {
      if (*pageno == pagemax)
      {
	/* �b�̫�@�����@�U */
	if (*cur != num % XO_TALL)
	{
	  *cur = num % XO_TALL;
	}
	else
	{
	  *pageno = 0;
	  *cur = 0;
	}
      }
      else
      {
	(*pageno)++;
	if (*pageno == pagemax && *cur > num % XO_TALL)
	  *cur = num % XO_TALL;
      }
      *redraw = 1;
    }
    break;

  case KEY_UP:
  case 'k':
    if (*cur == 0)
    {
      if (*pageno != 0)
      {
	*cur = XO_TALL - 1;
	*pageno = *pageno - 1;
      }
      else
      {
	*cur = num % XO_TALL;
	*pageno = pagemax;
      }
      *redraw = 1;
    }
    else
    {
      move(3 + *cur, 0);
      outc(' ');
      (*cur)--;
      move(3 + *cur, 0);
      outc('>');
    }
    break;

  case KEY_DOWN:
  case 'j':
    if (*cur == XO_TALL - 1)
    {
      *cur = 0;
      *pageno = (*pageno == pagemax) ? 0 : *pageno + 1;
      *redraw = 1;
    }
    else if (*pageno == pagemax && *cur == num % XO_TALL)
    {
      *cur = 0;
      *pageno = 0;
      *redraw = 1;
    }
    else
    {
      move(3 + *cur, 0);
      outc(' ');
      (*cur)++;
      move(3 + *cur, 0);
      outc('>');
    }
    break;

  case KEY_HOME:
  case '0':
    *pageno = 0;
    *cur = 0;
    *redraw = 1;
    break;

  case KEY_END:
  case '$':
    *pageno = pagemax;
    *cur = num % XO_TALL;
    *redraw = 1;
    break;

  default:
    if (ch >= '1' && ch <= '9')
    {
      int pos;
      char buf[6];

      buf[0] = ch;
      buf[1] = '\0';
      vget(b_lines, 0, "���ܲĴX���G", buf, sizeof(buf), GCARRY);

      pos = atoi(buf);

      if (pos > 0)
      {
	pos--;
	if (pos >num)
	  pos = num;
	*pageno = pos / XO_TALL;
	*cur = pos % XO_TALL;
      }

      *redraw = 1;	/* �N��S�������A�]�n��ø feeter */
    }
  }

  return ch;
}
