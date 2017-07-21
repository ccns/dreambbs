/*-------------------------------------------------------*/
/* edit.c	( NTHU CS MapleBBS Ver 2.36 )		 */
/*-------------------------------------------------------*/
/* target : simple ANSI/Chinese editor			 */
/* create : 95/03/29					 */
/* update : 95/12/15					 */
/*-------------------------------------------------------*/

#include "bbs.h"

typedef struct textline
{
  struct textline *prev;
  struct textline *next;
  int len;
  uschar data[ANSILINELEN];
}        textline;


static textline *vx_ini;	/* first line */
static textline *vx_cur;	/* current line */
static textline *vx_top;	/* top line in current window */


static int ve_lno;		/* current line number */
static int ve_row;		/* cursor position */
static int ve_col;


static int ve_mode;		/* operation mode */


#define	VE_INSERT	0x01
#define	VE_ANSI		0x02
#define	VE_FOOTER	0x04
#define	VE_REDRAW	0x08

#ifdef	HAVE_INPUT_TOOLS
#define	VE_INPUTOOL	0x100
#endif

#ifdef EVERY_BIFF
#define VE_BIFF		0x10
#define VE_BIFFN	0x20
#endif /* Thor.980805: �l�t��B�ӫ��a */


#define	FN_BAK		"bak"


/* ----------------------------------------------------- */
/* �O����޲z�P�s��B�z					 */
/* ----------------------------------------------------- */


#ifdef	DEBUG_VEDIT
static void
ve_abort(i)
  int i;
{
  char msg[40];

  sprintf(msg, "�Y������ %d", i);
  blog("VEDIT", msg);
}

#else

#define	ve_abort(n)	;
#endif


static void
ve_position(cur, top)
  textline *cur;
  textline *top;
{
  int row;

  row = cur->len;
  if (ve_col > row)
    ve_col = row;

  row = 0;
  while (cur != top)
  {
    row++;
    cur = cur->prev;
  }
  ve_row = row;

  ve_mode |= VE_REDRAW;
}


static inline void
ve_pageup()
{
  textline *cur, *top, *tmp;
  int lno, n;

  cur = vx_cur;
  top = vx_top;
  lno = ve_lno;
  for (n = 0; n < 22; n++)
  {
    if (!(tmp = cur->prev))
      break;

    cur = tmp;
    lno--;

    if ((tmp = top->prev))
      top = tmp;
  }

  vx_cur = cur;
  vx_top = top;
  ve_lno = lno;

  ve_position(cur, top);
}


static inline void
ve_forward(n)
  int n;
{
  textline *cur, *top, *tmp;
  int lno;

  cur = vx_cur;
  top = vx_top;
  lno = ve_lno;
  while (--n != 0)
  {
    if (!(tmp = cur->next))
      break;

    lno++;
    cur = tmp;

    if ((tmp = top->next))
      top = tmp;
  }

  vx_cur = cur;
  vx_top = top;
  ve_lno = lno;

  ve_position(cur, top);
}


#if 0
static void
ve_goto()
{
  int lno;
  char buf[8];

  if (vget(b_lines, 0, "���ܲĴX��G", buf, sizeof(buf), DOECHO) &&
    (lno = atoi(buf)) > 0 )
  {
    textline *vln, *tmp, *top;

    for (vln = vx_ini; tmp = vln->next; vln = tmp)
    {
      if (--lno <= 0)
        break;
    }

    vx_cur = top = vln;

    for (lno = 0; lno < 10; lno++)
    {
      if (!(tmp = top->prev))
	break;
      top = tmp;
    }
    ve_position(vln, top);
  }
}
#endif


static inline char *
ve_strim(s)
  char *s;
{
  while (*s == ' ')
    s++;
  return s;
}


static textline *
ve_alloc()
{
  textline *p;

  if ((p = (textline *) malloc(sizeof(textline))))
  {
    p->prev = NULL;
    p->next = NULL;
    p->len = 0;
    p->data[0] = '\0';
    return p;
  }

  ve_abort(13);			/* �O����Υ��F */
  abort_bbs();
  return NULL;
}


/* ----------------------------------------------------- */
/* Thor: ansi �y���ഫ  for color �s��Ҧ�		 */
/* ----------------------------------------------------- */


static int
ansi2n(ansix, line)
  int ansix;
  textline *line;
{
  uschar *data, *tmp;
  int ch;

  data = tmp = line->data;

  while ((ch = *tmp))
  {
    if (ch == KEY_ESC)
    {
      for (;;)
      {
	ch = *++tmp;
	if (ch >= 'a' && ch <= 'z' /* isalpha(ch) */ )
	{
	  tmp++;
	  break;
	}
	if (!ch)
	  break;
      }
      continue;
    }
    if (ansix <= 0)
      break;
    tmp++;
    ansix--;
  }
  return tmp - data;
}


static int
n2ansi(nx, line)
  int nx;
  textline *line;
{
  uschar *tmp, *nxp;
  int ansix;
  int ch;

  tmp = nxp = line->data;
  nxp += nx;
  ansix = 0;

  while ((ch = *tmp))
  {
    if (ch == KEY_ESC)
    {
      for (;;)
      {
	ch = *++tmp;
	if (ch >= 'a' && ch <= 'z' /* isalpha(ch) */ )
	{
	  tmp++;
	  break;
	}
	if (!ch)
	  break;
      }
      continue;
    }
    if (tmp >= nxp)
      break;
    tmp++;
    ansix++;
  }
  return ansix;
}


/* ----------------------------------------------------- */
/* delete_line deletes 'line' from the list,		 */
/* and maintains the vx_ini pointers.			 */
/* ----------------------------------------------------- */


static void
delete_line(line)
  textline *line;
{
  textline *p = line->prev;
  textline *n = line->next;

  if (p || n)
  {
    if (n)
      n->prev = p;

    if (p)
      p->next = n;
    else
      vx_ini = n;

    free(line);
  }
  else
  {
    line->data[0] = line->len = 0;
  }
}


/* ----------------------------------------------------- */
/* split 'line' right before the character pos		 */
/* ----------------------------------------------------- */


static void
ve_split(line, pos)
  textline *line;
  int pos;
{
  int len = line->len - pos;

  if (len >= 0)
  {
    textline *p, *n;
    uschar *ptr;

    line->len = pos;
    p = ve_alloc();
    p->len = len;
    strcpy(p->data, (ptr = line->data + pos));
    *ptr = '\0';

    /* --------------------------------------------------- */
    /* append p after line in list. keep up with last line */
    /* --------------------------------------------------- */

    if ((p->next = n = line->next))
      n->prev = p;
    line->next = p;
    p->prev = line;

    if (line == vx_cur && pos <= ve_col)
    {
      vx_cur = p;
      ve_col -= pos;
      ve_row++;
      ve_lno++;
    }
    ve_mode |= VE_REDRAW;
  }
}


/* ----------------------------------------------------- */
/* connects 'line' and the next line. returns true if:	 */
/* 1) lines were joined and one was deleted		 */
/* 2) lines could not be joined		 		 */
/* 3) next line is empty				 */
/* ----------------------------------------------------- */
/* returns false if:					 */
/* 1) Some of the joined line wrapped			 */
/* ----------------------------------------------------- */


static int
ve_join(line)
  textline *line;
{
  textline *n;
  uschar *data, *s;
  int sum, len;

  if (!(n = line->next))
    return YEA;

  if (!*ve_strim(data = n->data))
    return YEA;

  len = line->len;
  sum = len + n->len;
  if (sum < VE_WIDTH)
  {
    strcpy(line->data + len, data);
    line->len = sum;
    delete_line(n);
    return YEA;
  }

  s = data - len + VE_WIDTH - 1;
  while (*s == ' ' && s != data)
    s--;
  while (*s != ' ' && s != data)
    s--;
  if (s == data)
    return YEA;

  ve_split(n, (s - data) + 1);
  if (len + n->len >= VE_WIDTH)
  {
    ve_abort(0);
    return YEA;
  }

  ve_join(line);
  n = line->next;
  len = n->len;
  if (len >= 1 && len < VE_WIDTH - 1)
  {
    s = n->data + len - 1;
    if (*s != ' ')
    {
      *s++ = ' ';
      *s = '\0';
      n->len = len + 2;
    }
  }
  return NA;
}


static void
join_up(line)
  textline *line;
{
  while (!ve_join(line))
  {
    line = line->next;
    if (line == NULL)
    {
      ve_abort(2);
      abort_bbs();
    }
  }
}


/* ----------------------------------------------------- */
/* character insert / detete				 */
/* ----------------------------------------------------- */


static void
ve_char(ch)
  int ch;
{
  textline *p;
  int col, len, mode;
  uschar *data;

  p = vx_cur;
  len = p->len;
  col = ve_col;

  if (col > len)
  {
    ve_abort(1);
    return;
  }

  data = p->data;
  mode = ve_mode;

  /* --------------------------------------------------- */
  /* overwrite						 */
  /* --------------------------------------------------- */

  if ((col < len) && !(mode & VE_INSERT))
  {
    data[col++] = ch;

    /* Thor: ansi �s��, �i�Hoverwrite, ���\�� ansi code */

    if (mode & VE_ANSI)
      col = ansi2n(n2ansi(col, p), p);

    ve_col = col;
    return;
  }

  /* --------------------------------------------------- */
  /* insert / append					 */
  /* --------------------------------------------------- */

  for (mode = len; mode >= col; mode--)
  {
    data[mode + 1] = data[mode];
  }
  data[col++] = ch;
  ve_col = col;
  p->len = ++len;

  if (len >= VE_WIDTH - 2)
  {
    /* Thor.980727: �ץ� editor buffer overrun ���D, ���� */

    ve_split(p, VE_WIDTH - 3);

#if 0
    uschar *str = data + len;

    while (*--str == ' ')
    {
      if (str == data)
	break;
    }

    ve_split(p, (str - data) + 1);
#endif
#if 0
 �@��  yvb (yvb)                                            �ݪO  SYSOP
 ���D  ���� editor...
 �ɶ�  Sun Jun 28 11:28:02 1998
�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w

    post �� mail �����o�� editor, �p�G�A�@�����᭱���r,
    �̦h�i�H����@�C 157 �r�a... �A���N�|�Q������.

    ���L�p�G�A������n�b�᭱�[�r... �]�N�O�A���^��~����
    �~�򥴦r... ���t�Τ��|���A�b�ӦC�~��[�@�Ӧr��, �åB
    ���X�s���@���...

    ���гo�ӨB�J, ��� 170 �r��, �A�N�|�Q�_�u�F...
    ��... ���ܺA�a :P

    ���, �q�t�@���[�I�ӻ�, �Y�o���O�t�ίS�N�o�ˤl�]�p,
    ���N��ܳo�ئ��G���õۥi�ǥ� buffer overrun ���覡,
    �i��F���J�I�t�Ϊ��M��...
--
�� �ӷ�: ����˪L �� From: bamboo.Dorm6.NCTU.edu.tw

#endif

  }
}


static void
delete_char(cur, col)
  textline *cur;
  int col;
{
  uschar *dst, *src;

  cur->len--;
  dst = cur->data + col;
  for (;;)
  {
    src = dst + 1;
    if (!(*dst = *src))
      break;
    dst = src;
  }
}


void
ve_string(str)
  uschar *str;
{
  int ch;

  while ((ch = *str))
  {

#ifdef	SHOW_USER_IN_TEXT
    if (isprint2(ch) || ch == KEY_ESC || ch <= 2)
#else
    if (isprint2(ch) || ch == KEY_ESC)
#endif

    {
      ve_char(ch);
    }
    else if (ch == '\t')
    {
      do
      {
	ve_char(' ');
      } while (ve_col & TAB_WIDTH);
    }
    else if (ch == '\n')
    {
      ve_split(vx_cur, ve_col);
    }
    str++;
  }
}


static void
ve_ansi()
{
  int fg, bg, mode;
  char ans[4], buf[16], *apos, *color, *tmp;
  static char t[] = "BRGYLPCW";

  mode = ve_mode | VE_REDRAW;
  color = str_ransi;

  if (mode & VE_ANSI)
  {
    move(b_lines - 1, 55);
    outs("\033[1;33;40mB\033[41mR\033[42mG\033[43mY\033[44mL\033[45mP\033[46mC\033[47mW\033[m");
    if ((fg = vget(b_lines, 0, "�п�J  �G��/�e��/�I��[���`�զr�©�][0wb]�G",
	apos = ans, 4, LCECHO)))
    {
      color = buf;
      strcpy(color, "\033[");
      if (isdigit(fg))
      {
	sprintf(color, "%s%c", color, *(apos++));
	if (*apos)
	  strcat(color, ";");
      }
      if (*apos)
      {
	if ((tmp = strchr(t, toupper(*(apos++)))))
	  fg = tmp - t + 30;
	else
	  fg = 37;
	sprintf(color, "%s%d", color, fg);
      }
      if (*apos)
      {
	if ((tmp = strchr(t, toupper(*(apos++)))))
	  bg = tmp - t + 40;
	else
	  bg = 40;
	sprintf(color, "%s;%d", color, bg);
      }
      strcat(color, "m");
    }
  }

  ve_mode = mode | VE_INSERT;
  ve_string(color);
  ve_mode = mode;
}


static textline *
ve_line(this, str)
  textline *this;
  uschar *str;
{
  int cc, len;
  uschar *data;
  textline *line;

  do
  {
    line = ve_alloc();
    data = line->data;
    len = 0;

    for (;;)
    {
      cc = *str;

      if (cc == '\n')
	cc = 0;
      if (cc == 0)
	break;

      str++;

      if (cc == '\t')
      {
	do
	{
	  *data++ = ' ';
	  len++;
	} while ((len & TAB_WIDTH) && (len < VE_WIDTH));
      }
      else if (cc > 2 && cc < ' ' && cc != 27)
      {
	continue;
      }
      else
      {
	*data++ = cc;
	len++;
      }
      if (len >= VE_WIDTH)
	break;
    }

    *data = '\0';
    line->len = len;
    line->prev = this;
    this = this->next = line;

  } while (cc);

  return this;
}


/* ----------------------------------------------------- */
/* �Ȧs�� TBF (Temporary Buffer File) routines		 */
/* ----------------------------------------------------- */


char *
tbf_ask()
{
  static char fn_tbf[] = "buf.1";
  int ch;

  do
  {
    ch = vget(b_lines, 0, "�п�ܼȦs��(1-5)�G", fn_tbf + 4, 2, GCARRY);
  } while (ch < '1' || ch > '5');
  return fn_tbf;
}


FILE *
tbf_open()
{
  int ans;
  char fpath[64], op[4];

  usr_fpath(fpath, cuser.userid, tbf_ask());
  ans = 'a';

  if (dashf(fpath))
  {
    ans = vans("�Ȧs�ɤw����� (A)���[ (W)�мg (Q)�����H[A] ");
    if (ans == 'q')
      return NULL;

    if (ans != 'w')
      ans = 'a';
  }

  op[0] = ans;
  op[1] = '\0';

  return fopen(fpath, op);
}


static textline *
ve_load(this, fd)
  textline *this;
  int fd;
{
  uschar *str;
  textline *next;

  next = this->next;

  mgets(-1);
  while ((str = mgets(fd)))
  {
    this = ve_line(this, str);
  }

  this->next = next;
  if (next)
    next->prev = this;

  return this;
}


static inline void
tbf_read()
{
  int fd;
  char fpath[80];

  usr_fpath(fpath, cuser.userid, tbf_ask());

  fd = open(fpath, O_RDONLY);
  if (fd >= 0)
  {
    ve_load(vx_cur, fd);
    close(fd);
  }
}


static inline void
tbf_write()
{
  FILE *fp;
  textline *p;
  uschar *data;

  if ((fp = tbf_open()))
  {
    for (p = vx_ini; p;)
    {
      data = p->data;
      p = p->next;
      if (p || *data)
	fprintf(fp, "%s\n", data);
    }
    fclose(fp);
  }
}


static inline void
tbf_erase()
{
  char fpath[80];

  usr_fpath(fpath, cuser.userid, tbf_ask());
  unlink(fpath);
}


/* ----------------------------------------------------- */
/* �s�边�۰ʳƥ�					 */
/* ----------------------------------------------------- */


void
ve_backup()
{
  textline *p, *n;

  if ((p = vx_ini))
  {
    FILE *fp;
    char bakfile[64];

    vx_ini = NULL;
    usr_fpath(bakfile, cuser.userid, FN_BAK);
    if ((fp = fopen(bakfile, "w")))
    {
      do
      {
	n = p->next;
	fprintf(fp, "%s\n", p->data);
	free(p);
      } while ((p = n));
      fclose(fp);
    }
  }
}


void
ve_recover()
{
  char fpbak[80], fpath[80];

  usr_fpath(fpbak, cuser.userid, FN_BAK);
  if (dashf(fpbak))
  {
    if (vans("�z���@�g�峹�|�������A(S)�g�J�Ȧs�� (Q)��F�H[S] ") == 'q')
    {
      unlink(fpbak);
    }
    else
    {
      usr_fpath(fpath, cuser.userid, tbf_ask());
      rename(fpbak, fpath);
    }
  }
}


/* ----------------------------------------------------- */
/* �ޥΤ峹						 */
/* ----------------------------------------------------- */


static int
is_quoted(str)
  char *str;			/* "--\n", "-- \n", "--", "-- " */
{
  if (*str == '-')
  {
    if (*++str == '-')
    {
      if (*++str == ' ')
	str++;
      if (*str == '\n')
	str++;
      if (!*str)
	return 1;
    }
  }
  return 0;
}


static inline int
quote_line(str, qlimit)
  char *str;
  int qlimit;			/* ���\�X�h�ި��H */
{
  int qlevel = 0;
  int ch;

  while ((ch = *str) == QUOTE_CHAR1 || ch == ':')
  {
    if (*(++str) == ' ')
      str++;
    if (qlevel++ >= qlimit)
      return 0;
  }
  while ((ch = *str) == ' ' || ch == '\t')
    str++;
  if (qlevel >= qlimit)
  {
    if (!memcmp(str, "�� ", 3) || !memcmp(str, "==>", 3) ||
      strstr(str, ") ����:\n"))
      return 0;
  }
  return (*str != '\n' && (strncmp(str,"\033[m\n",4)));
}

/* ----------------------------------------------------- */
/* �f�d user �o��峹�r��/�`���媺�ϥ�			 */
/* ----------------------------------------------------- */


int wordsnum;		/* itoc.010408: ��峹�r�� */
int keysnum;

#ifdef ANTI_PHONETIC
static int
words_check()
{
  textline *p; 
  uschar *str, *pend;
  int phonetic;		/* �`����ƥ� */

  wordsnum = phonetic = 0;

  for (p = vx_ini; p; p = p->next)
  {
    if (is_quoted(str = p->data))	/* ñ�W�ɶ}�l */
      break;

    if (!(str[0] == QUOTE_CHAR1 && str[1] == ' ') && strncmp(str, "�� ", 3)) /* �D�ޥΥL�H�峹 */
    {
      wordsnum += p->len;

      pend = str + p->len;
      while (str < pend)
      {
	if (str[0] >= 0x81 && str[0] < 0xFE && str[1] >= 0x40 && str[1] <= 0xFE && str[1] != 0x7F)	/* ����r BIG5+ */
	{
	  if (str[0] == 0xA3 && str[1] >= 0x74 && str[1] <= 0xBA)	/* �`���� */
	    phonetic++;
	  str++;	/* ����r���줸�A�n�h�[�@�� */
	}
	str++;
      }

    }
  }
  return phonetic;
}

#else

static void
words_check()
{
  textline *p; 
  char *str;

  wordsnum = 0;

  for (p = vx_ini; p; p = p->next)
  {
    if (is_quoted(str = p->data))	/* ñ�W�ɶ}�l */
      break;

    if (!(str[0] == QUOTE_CHAR1 && str[1] == ' ') && strncmp(str, "�� ", 3)) /* �D�ޥΥL�H�峹 */
      wordsnum += p->len;
  }
}
#endif

static void
ve_quote(this)
  textline *this;
{
  int fd, op;
  FILE *fp;
  textline *next;
  char *str, buf[256];
  static char msg[] = "���ñ�W�� (1/2/3, 0=���[)[0]�G";

  next = this->next;

  /* --------------------------------------------------- */
  /* �ި�						 */
  /* --------------------------------------------------- */

  if (*quote_file)
  {
    op = vans("�аݭn�ޥέ���(Y/N/All/Repost/1-9)�H[Y] ");

    if (op != 'n')
    {
      if ((fp = fopen(quote_file, "r")))
      {
	str = buf;

	if ((op >= '1') && (op <= '9'))
	  op -= '1';
	else if ((op != 'a') && (op != 'r'))
	  op = 1;		/* default : 2 level */

	if (op != 'a')		/* �h�� header */
	{
	  if (*quote_nick)
	    sprintf(buf + 128, " (%s)", quote_nick);
	  else
	    buf[128] = '\0';
	  
	  sprintf(str, "�� �ޭz�m%s%s�n���ʨ��G", quote_user, buf + 128);

	  this = ve_line(this, str);

	  while (fgets(str, 256, fp) && *str != '\n');

	  if (curredit & EDIT_LIST)	/* �h�� mail list �� header */
	  {
	    while (fgets(str, 256, fp) && (!memcmp(str, "�� ", 3)));
	  }
	}

	if (op == 'r')
	{
	  op = 'a';
	}
	else
	{
	  *str++ = QUOTE_CHAR1;
	  *str++ = ' ';
	}

	if (op == 'a')
	{
	  while (fgets(str, 254, fp))
	    this = ve_line(this, buf);
	}
	else
	{
	  while (fgets(str, 254, fp))
	  {
	    if (is_quoted(str))	/* "--\n" */
	      break;
	    if (quote_line(str, op))
	      this = ve_line(this, buf);
	  }
	}
	fclose(fp);
      }
    }
    *quote_file = '\0';
  }

  this = ve_line(this, "");

  /* --------------------------------------------------- */
  /* ñ�W��						 */
  /* --------------------------------------------------- */

#ifdef HAVE_ANONYMOUS
  /* Thor.980909: gc patch: �ΦW�Ҧ�����ñ�W�� */
  if(curredit & EDIT_ANONYMOUS || cuser.ufo2 & UFO2_SIGN)
    goto OUT_ve_quote; 
#endif

  msg[27] = op = cuser.signature + '0';
  if ((fd = vget(b_lines, 0, msg, buf, 3, DOECHO)))
  {
    if (op != fd && fd >= '0' && fd <= '3')
    {
      cuser.signature = fd - '0';
      op = fd;
    }
  }

  if (op != '0')
  {
    usr_fpath(buf, cuser.userid, FN_SIGN);
    fd = open(buf, O_RDONLY);
    if (fd >= 0)
    {
      op = (op - '1') * MAXSIGLINES;

      mgets(-1);
      while ((str = mgets(fd)))
      {
	if (--op >= 0)
	  continue;

	if (op == -1)
	{
	  this = ve_line(this, "--");
	}

	this = ve_line(this, str);

	if (op <= -MAXSIGLINES)
	  break;
      }
      close(fd);
    }
  }

#ifdef HAVE_ANONYMOUS
OUT_ve_quote:
#endif

  this->next = next;
  if (next)
    next->prev = this;
}


/* ----------------------------------------------------- */
/* �f�d user �ި����ϥ�					 */
/* ----------------------------------------------------- */


static int
quote_check()
{
  textline *p;
  char *str;
  int post_line;
  int quot_line;

  post_line = quot_line = 0;
  for (p = vx_ini; p; p = p->next)
  {
    if (is_quoted(str = p->data))
      break;

    if (str[0] == QUOTE_CHAR1 && str[1] == ' ')
    {
      quot_line++;
    }
    else
    {
      while (*str == ' ' || *str == '\n' || *str == '\r')
	str++;
      if (*str && (strlen(str) > 4))
	post_line++;
    }
  }
  
#ifdef	HAVE_RESIST_WATER
  if((post_line <= CHECK_QUOT) && (bbstate & BRD_CHECKWATER))
    checkqt++;
  else if (checkqt > 0)
    checkqt--;
#endif

  if ((quot_line >> 2) <= post_line)
    return 0;

/*  if (HAS_PERM(PERM_SYSOP))*/
    return (vans("�ި��L�h (E)�~��s�� (W)�j��g�J�H[E] ") != 'w');

/*  vmsg("�ި��Ӧh�A�� [�W�[�@�Ǥ峹] �� [�R�������n���ި�]");
  return 1;*/
}


/* ----------------------------------------------------- */
/* �ɮ׳B�z�GŪ�ɡB�s�ɡB���D�Bñ�W��			 */
/* ----------------------------------------------------- */


void
ve_header(fp)
  FILE *fp;
{
  time_t now;
  char *title;

  title = ve_title;
  title[72] = '\0';
  time(&now);

  if (curredit & EDIT_MAIL)
  {
    fprintf(fp, "%s %s (%s)\n", str_author1, cuser.userid,

#if defined(REALINFO) && defined(MAIL_REALNAMES)
      cuser.realname);
#else
      cuser.username);
#endif
  }
  else
  {   

    /* Thor.980613: �p�G�b����峹, �I�s ve_header, �ѩ󤣬O�b�Q�઺��, �]���ݩʤ���,
                     ���i��ϯ��K���X�{ , BRD_ANONYMOUS ��P*/

    if (!(bbstate & BRD_NOSTAT))/* �ݪO���ǤJ�έp */
    {
      /* ���Ͳέp��� */

      struct
      {
	char author[IDLEN + 1];
	char board[IDLEN + 1];
	char title[66];
	time_t date;		/* last post's date */
	int number;		/* post number */
      }      postlog;

#ifdef HAVE_ANONYMOUS
      /* Thor.980909: anonymous post mode */
      if ((bbstate & BRD_ANONYMOUS) && (curredit & EDIT_ANONYMOUS))
	/* strcpy(postlog.author, "anonymous"); */
        /* Thor.990113: �ȻP�{�s id �V�c */
	strcpy(postlog.author, "[���i�D�A]");
      else
#endif

	strcpy(postlog.author, cuser.userid);

      strcpy(postlog.board, currboard);
      /* str_ncpy(postlog.title, str_ttl(title), sizeof(postlog.title) - 1); */
      /* Thor.980921: str_ncpy �t 0 */
      str_ncpy(postlog.title, str_ttl(title), sizeof(postlog.title));
      postlog.date = now;
      postlog.number = 1;

            /* Thor.980613: �J�M�e����check, ����n�Acheck�O? */
      if (!(bbstate & BRD_NOSTAT))   
	rec_add(FN_POST_DB, &postlog, sizeof(postlog));
    }

#ifdef HAVE_ANONYMOUS
    /* Thor.980909: anonymous post mode */
    if ((bbstate & BRD_ANONYMOUS) && (curredit & EDIT_ANONYMOUS))
    {
      fprintf(fp, "%s %s (%s) %s %s\n",
        /* Thor.990113: �ȻP�{�s id �V�c */
	str_author1, "���i�D�A" /*"anonymous"*/, "�q�q�ڬO�� ? ^o^",
	curredit & EDIT_OUTGO ? str_post1 : str_post2, currboard);
    }
    else
#endif

    {
      fprintf(fp, "%s %s (%s) %s %s\n", str_author1, cuser.userid,

#if defined(REALINFO) && defined(POSTS_REALNAMES)
	cuser.realname
#else
	HAS_PERM(PERM_DENYNICK) ? cutmp->username : cuser.username
#endif

	,curredit & EDIT_OUTGO ? str_post1 : str_post2, currboard);
    }
  }
  fprintf(fp, "���D: %s\n�ɶ�: %s\n", title, ctime(&now));
}

static void
ve_show_sign(fpath)
  char *fpath;
{
  char buf[256];
  int i,j;
  FILE *fp;
  fp = fopen(fpath,"r");
  clear();
  if(fp)
  {
    for(j=1;j<=3;j++)
    {
      prints("\033[36m�iñ�W��.%d�j\033[m\n",j); 
      for (i=0;i<MAXSIGLINES;i++)
      {
        if(fgets(buf, 256, fp))
          prints("%s",buf);
        else
          prints("\n");
      }
    }
    fclose(fp);
  }
                                    
}

static void
ve_select_sign(fp)
  FILE *fp;
{
  FILE *fd;
  char msg[] = "���ñ�W�� (1/2/3, 0=���[)[0]�G";
  char buf[256];
  int ans,op,len,used;

  msg[27] = op = cuser.signature + '0';
  usr_fpath(buf, cuser.userid, FN_SIGN);
  ve_show_sign(buf);
  if ((ans = vget(b_lines, 0, msg, buf, 3, DOECHO)))
  {
    if (op != ans && ans >= '0' && ans <= '3')
    {
      cuser.signature = ans - '0';
      op = ans;
    }
  }  
     
  if (op != '0')
  {
    usr_fpath(buf, cuser.userid, FN_SIGN);
    fd = fopen(buf, "r");
    if (fd)
    {
      int i;
      
      op = (op - '1') * MAXSIGLINES;
      for(i=0;i<op;i++)
        fgets(buf, 256, fd);
      fprintf(fp,"--");
      
      len = 0;
      used = 0;
      for (i=0;i<MAXSIGLINES;i++)
      {
        if(fgets(buf, 256, fd))
        {
          fprintf(fp,"\n");
          len = strlen(buf);
          if(len > 0)
          {
            used = 1;
            buf[len - 1] = '\0';
            fprintf(fp,"%s",buf);
          }
        }
      }
      if(used)
        fprintf(fp,"\033[m\n");
      else
        fprintf(fp,"\n");
      fclose(fd);
    }
  }  
}


static int
ve_filer(fpath, ve_op)
  char *fpath;
  int ve_op; /* Thor.981020: 1 ��header, 0 �Lheader */
{
  int ans = 0;
  FILE *fp=NULL;
  textline *p, *v;
  char buf[80], *msg , re;

#ifdef  HAVE_INPUT_TOOLS
  char *menu1[] =  {"SE","Save     �s��","Abort    ���","Title    ����D","Edit     �~��s��","Input    �Ÿ���J�u��","Read     Ū���Ȧs��","Write    �g�J�Ȧs��","Delete   �R���Ȧs��" , "Quit     ���}���" , NULL};
  char *menu2[] = {"SE","Save     �s��","Local    �s��������","Abort    ���","Title    ����D","Edit     �~��s��","Input    �Ÿ���J�u��","Read     Ū���Ȧs��","Write    �g�J�Ȧs��","Delete   �R���Ȧs��" , "Quit     ���}���" , NULL};
  char *menu3[] = {"LE","Local    �s��������","Save     �s��","Abort    ���","Title    ����D","Edit     �~��s��","Input    �Ÿ���J�u��","Read     Ū���Ȧs��","Write    �g�J�Ȧs��","Delete   �R���Ȧs��" , "Quit     ���}���" , NULL};
#else
  char *menu1[] =  {"SE","Save     �s��","Abort    ���","Title    ����D","Edit     �~��s��","Read     Ū���Ȧs��","Write    �g�J�Ȧs��","Delete   �R���Ȧs��" , "Quit     ���}���" , NULL};
  char *menu2[] = {"SE","Save     �s��","Local    �s��������","Abort    ���","Title    ����D","Edit     �~��s��","Read     Ū���Ȧs��","Write    �g�J�Ȧs��","Delete   �R���Ȧs��" , "Quit     ���}���" , NULL};
  char *menu3[] = {"LE","Local    �s��������","Save     �s��","Abort    ���","Title    ����D","Edit     �~��s��","Read     Ū���Ȧs��","Write    �g�J�Ȧs��","Delete   �R���Ȧs��" , "Quit     ���}���" , NULL};
#endif


#ifdef	HAVE_INPUT_TOOLS  
  if (bbsmode != M_POST)
    msg = "[S]�s�� (A)��� (T)����D (E)�~�� (I)�Ÿ� (R/W/D)Ū�g�R�Ȧs�ɡH";
  else if (curredit & EDIT_OUTGO)
    msg = "[S]�s�� (L)���� (A)��� (T)����D (E)�~�� (R/W/D)Ū�g�R�Ȧs�ɡH";
  else
    msg = "[L]���� (S)�s�� (A)��� (T)����D (E)�~�� (R/W/D)Ū�g�R�Ȧs�ɡH";
#else
  if (bbsmode != M_POST)
    msg = "[S]�s�� (A)��� (T)����D (E)�~�� (R/W/D)Ū�g�R�Ȧs�ɡH";
  else if (curredit & EDIT_OUTGO)
    msg = "[S]�s�� (L)���� (A)��� (T)����D (E)�~�� (R/W/D)Ū�g�R�Ȧs�ɡH";
  else
    msg = "[L]���� (S)�s�� (A)��� (T)����D (E)�~�� (R/W/D)Ū�g�R�Ȧs�ɡH"; 
#endif

/* cache.091023: �o����ѱ��N�i�H�j��ϥιϧο�� */
//  if(cuser.ufo2 & UFO2_ORIGUI)
//    re = vget(b_lines, 0, msg, buf, 3, LCECHO);
//  else
//  {
    if (bbsmode != M_POST)
      re = popupmenu_ans2(menu1,"�s�ɿﶵ",4,20);
    else if (curredit & EDIT_OUTGO)
      re = popupmenu_ans2(menu2,"�s�ɿﶵ",4,20);
    else
      re = popupmenu_ans2(menu3,"�s�ɿﶵ",4,20);
    
//  }  

  switch (re)
  {
#ifdef	HAVE_INPUT_TOOLS  
  case 'i':
    return VE_INPUTOOL;
#endif
  case 's':
    /* Thor.990111: ����H�h���~�y */
    if (HAS_PERM(PERM_INTERNET) && !(bbstate & BRD_NOTRAN))
      curredit |= EDIT_OUTGO;
    break;

  case 'a':
    /* outs("�峹\033[1m �S�� \033[m�s�J"); */
    ans = -1;
    break;

  case 'l':
    curredit &= ~EDIT_OUTGO;
    break;

  case 'x':
   if ((bbsmode == M_POST))   /* �O�D�o�� */
     curredit |= EDIT_RESTRICT;
   break;

  case 'r':
    tbf_read();
    return VE_REDRAW;

  case 'e':
    return VE_FOOTER;

  case 'w':
    tbf_write();
    return VE_FOOTER;

  case 'd':
    tbf_erase();
    return VE_FOOTER;

  case 't':
    strcpy(buf, ve_title);
    if (!vget(b_lines, 0, "���D�G", ve_title, TTLEN, GCARRY))
      strcpy(ve_title, buf);
    return VE_FOOTER;
    
  default:
    return VE_FOOTER;          
  }

  if (!ans)
  {
    if (ve_op && !(curredit & EDIT_MAIL) && quote_check())
      return VE_FOOTER;

#ifdef ANTI_PHONETIC
    if (bbstate & BRD_NOPHONETIC && words_check() > 2)
    {
      vmsg("���O�T��`����A�Э��s�ˬd����");
      return VE_FOOTER;
    }
#endif

    if (!*fpath)
    {
      usr_fpath(fpath, cuser.userid, FN_NOTE);
    }

    if ((fp = fopen(fpath, "w")) == NULL)
    {
      ve_abort(5);
      abort_bbs();
    }
    
#ifndef ANTI_PHONETIC
    words_check();	/* itoc.010408: ��峹�r�� */
#endif
    
    if (ve_op == 1)
      ve_header(fp);
  }

  if ((p = vx_ini))
  {
    vx_ini = NULL;

    do
    {
      v = p->next;
      if (!ans)
      {
	msg = p->data;
	if (v || msg[0])
	{
	  str_trim(msg);
	  
	  if(v || (bbsmode != M_POST))
  	    fprintf(fp, "%s\n", msg);
  	  else
  	    fprintf(fp, "%s\033[m\n", msg);
	}
	else if(bbsmode == M_POST)
	{
	  fprintf(fp, "\033[m\n");
	}
      }
      free(p);
    } while ((p = v));
  }

  if (!ans)
  {
    if ((bbsmode == M_POST) || (bbsmode & M_SMAIL))
    {
    
      if(!(curredit & EDIT_ANONYMOUS) && (cuser.ufo2 & UFO2_SIGN) && ve_op)
      {
        ve_select_sign(fp);
      }
  
#ifdef	HAVE_ORIGIN

#ifdef 	HAVE_ANONYMOUS
      /* Thor.980909: anonymous post mode */
      if ((bbstate & BRD_ANONYMOUS) && (curredit & EDIT_ANONYMOUS) 
           && (bbsmode != M_SMAIL))
              /* lkchu.981201: M_SMAIL �ɤ����n�ʦW */
	  fprintf(fp, ANONYMOUS_TAG, 
  	    str_site, MYHOSTNAME, "�ʦW�ѨϪ��a");
      else
#endif

      if(ve_op)
      {
	fprintf(fp, ORIGIN_TAG,
	  /*str_site, MYHOSTNAME,*/ ((cuser.ufo & UFO_HIDEDN)&&(str_cmp(cuser.userid,"guest"))) ? HIDEDN_SRC : fromhost);
	
	if((bbstate & BRD_LOGEMAIL) && !(bbsmode == M_SMAIL))
	  fprintf(fp, EMAIL_TAG, cuser.email);
      }
#endif				/* HAVE_ORIGIN */
    }
    fclose(fp);
  }

  return ans;
}


/* ----------------------------------------------------- */
/* �ù��B�z�G���U�T���B��ܽs�褺�e			 */
/* ----------------------------------------------------- */


static void
ve_outs(text)
  uschar *text;
{
  int ch;
  uschar *tail;

  tail = text + SCR_WIDTH - 1;
  while ((ch = *text))
  {
    switch (ch)
    {

#ifdef SHOW_USER_IN_TEXT
    case 1:
      ch = '#';
      break;

    case 2:
      ch = '%';
      break;
#endif

    case 27:
      ch = '*';
      break;
    }
    outc(ch);

    if (++text >= tail)
      break;
  }
}

static int
select_title(title)
  char *title;
{
  char *objs[] = {"[���i]","[�s�D]","[����]","[���]","[���D]","[����]"};
  int select; 
  outs("\n\n1.�i���i�j2.�i�s�D�j3.�i����j4.�i���j5.�i���D�j6.�i���աj7.�i��L�j\n");
  select = vans("�п�ܤ峹���O�Ϋ� Enter ���L�G") - '1';   
  if(select >=0 && select <=5)
  {
    sprintf(title,"%s",objs[select]);
    return 1;
  }
  else
    *title = '\0';
  return 0;  
}

int
ve_subject(row, topic, dft)
  int row;
  char *topic;
  char *dft;
{
  char *title;
  int select=0;
  title = ve_title;

  if (topic)
  {
    sprintf(title, "Re: %s", str_ttl(topic));
    title[TTLEN] = '\0';
  }
  else
  {

/* 090924.cache: ��ܤ峹���O�\�� */
#ifdef CAN_POSTFIX
    if (dft)
      strcpy(title, dft);
    else
#else
     *title = '\0';    
     //select = select_title(title);
#endif      
  }
  return vget(row, 0, "���D�G", select ? title+6:title, select ? 
            TTLEN - 5 : TTLEN + 1, GCARRY);
}


/* ----------------------------------------------------- */
/* �s��B�z�G�D�{���B��L�B�z				 */
/* ----------------------------------------------------- */


int
vedit(fpath, ve_op)
  char *fpath;
  int ve_op;	/* 0: �º�s���ɮ� 1: quote/header 2: quote */
{
  textline *vln, *tmp;
  int cc, col, mode, margin, pos;
  static void (*input_tool)();

  /* --------------------------------------------------- */
  /* ��l�]�w�G���J�ɮסB�ޥΤ峹�B�]�w�s��Ҧ�		 */
  /* --------------------------------------------------- */

  if(bbsothermode & OTHERSTAT_EDITING)
  {
    vmsg("�A�٦��ɮ��٨S�s���@�I");
    return -1;
  }
  bbsothermode |= OTHERSTAT_EDITING;
  
  tmp = vln = ve_alloc();

  if (*fpath)
  {
    cc = open(fpath, O_RDONLY);
    if (cc >= 0)
    {
      vln = ve_load(vln, cc);
    }
    else
    {
      cc = open(fpath, O_WRONLY | O_CREAT, 0600);
      if (cc < 0)
      {
	ve_abort(4);
	abort_bbs();
      }
    }
    close(cc);
  }

  if (ve_op)
  {
    ve_quote(vln);
  }

  if ((vln = tmp->next))
  {
    free(tmp);
    vln->prev = NULL;
  }
  else
  {
    vln = tmp;
  }

  vx_cur = vx_top = vx_ini = vln;

  ve_col = ve_row = margin = 0;
  ve_lno = 1;
  ve_mode = VE_INSERT | VE_REDRAW | (cuser.ufo2 & UFO2_VEDIT ? 0 : VE_FOOTER);

  /* --------------------------------------------------- */
  /* �D�j��G�ù���ܡB��L�B�z�B�ɮ׳B�z		 */
  /* --------------------------------------------------- */

  clear();

  keysnum = 0;

  for (;;)
  {
    vln = vx_cur;
    mode = ve_mode;
    col = ve_col;
    cc = (col < SCR_WIDTH - 1) ? 0 : (col / 72) * 72;
    if (cc != margin)
    {
      mode |= VE_REDRAW;
      margin = cc;
    }

    if (mode & VE_REDRAW)
    {
      ve_mode = (mode ^= VE_REDRAW);

      tmp = vx_top;

      for (pos = 0;; pos++)
      {
	move(pos, 0);
	clrtoeol();
	if (pos == b_lines)
	  break;
	if (tmp)
	{
	  if (mode & VE_ANSI)
	    outx(tmp->data);
	  else if (tmp->len > margin)
	    ve_outs(tmp->data + margin);
	  tmp = tmp->next;
	}
	else
	{
	  outc('~');
	}
      }
#ifdef EVERY_BIFF
      if(!(mode & VE_BIFF))
      {
        if(cutmp->ufo & UFO_BIFF)
          ve_mode = mode |= VE_BIFF;
        else if(cutmp->ufo & UFO_BIFFN)
          ve_mode = mode |= VE_BIFFN;
      }
#endif
    }
    else
    {
      move(ve_row, 0);
      if (mode & VE_ANSI)
	outx(vln->data);
      else if (vln->len > margin)
	ve_outs(vln->data + margin);
      clrtoeol();
    }

    /* ------------------------------------------------- */
    /* ��ܪ��A�BŪ����L				 */
    /* ------------------------------------------------- */

    if (mode & VE_ANSI)		/* Thor: �@ ansi �s�� */
      pos = n2ansi(col, vln);	/* Thor: ansi ���|�Ψ�cc */
    else			/* Thor: ���Oansi�n�@margin shift */
      pos = col - margin;

    if (mode & VE_FOOTER)
    {
      move(b_lines, 0);
      clrtoeol();
#ifdef EVERY_BIFF
      prints("%s\033[0;31;47m  (Ctrl-Z)\033[30m �ާ@����  \033[31m(^w,^x)\033[30m �ɮ׳B�z ��%s�x%s��%5d:%3d  \033[m",
        mode & VE_BIFF ? "\033[1;41;37;5m  �l�t�ӤF  ": mode & VE_BIFFN ? "\033[1;41;37;5m  �T�t�ӤF  ":"\033[0;34;46m  �s��峹  ",
	mode & VE_INSERT ? "���J" : "���N",
	mode & VE_ANSI ? "ANSI" : "�@��",
	ve_lno, 1 + (mode & VE_ANSI ? pos : col));
        /* Thor.980805: UFO_BIFF everywhere */
#else

      prints("\033[34;46m  �s��峹  \033[31;47m  (Ctrl-Z)\033[30m �ާ@����  \033[31m(^w,^x)\033[30m �ɮ׳B�z ��%s�x%s��%5d:%3d  \033[m",
	mode & VE_INSERT ? "���J" : "���N",
	mode & VE_ANSI ? "ANSI" : "�@��",
	ve_lno, 1 + (mode & VE_ANSI ? pos : col));
#endif
    }

    move(ve_row, pos);

ve_key:

    cc = vkey();

    if (isprint2(cc))
    {
      ve_char(cc);
      keysnum++;      
    }
    else
    {
      switch (cc)
      {
      case '\n':

	ve_split(vln, col);
	break;

      case KEY_TAB:

	do
	{
	  ve_char(' ');
	} while (ve_col & (TAB_STOP - 1));
	break;

      case KEY_INS:		/* Toggle insert/overwrite */
      case Ctrl('O'):

	ve_mode = mode ^ VE_INSERT;
	continue;

      case Ctrl('H'):		/* backspace */

	/* Thor: �b ANSI �s��Ҧ��U, ���i�H���˰h, ���M�|�ܥi��.... */

	if (mode & VE_ANSI)
	  goto ve_key;

	if (col)
	{
	  delete_char(vln, ve_col = --col);
	  continue;
	}

	if (!(tmp = vln->prev))
	  goto ve_key;

	ve_row--;
	ve_lno--;
	vx_cur = tmp;
	ve_col = tmp->len;
	if (*ve_strim(vln->data))
	  join_up(tmp);
	else
	  delete_line(vln);
	ve_mode = mode | VE_REDRAW;
	break;

      case Ctrl('D'):
      case KEY_DEL:		/* delete current character */

	cc = vln->len;
	if (cc == col)
	{
	  join_up(vln);
	  ve_mode = mode | VE_REDRAW;
	}
	else
	{
	  if (cc == 0)
	    goto ve_key;
	  delete_char(vln, col);
	  if (mode & VE_ANSI)	/* Thor: ���M�W�[ load, ���Ledit �ɷ|����n�� */
	    ve_col = ansi2n(n2ansi(col, vln), vln);
	}
	continue;

      case KEY_LEFT:

	if (col)
	{
	  ve_col = (mode & VE_ANSI) ? ansi2n(pos - 1, vln) : col - 1;
	  continue;
	}

	if (!(tmp = vln->prev))
	  goto ve_key;

	ve_row--;
	ve_lno--;
	ve_col = tmp->len;
	vx_cur = tmp;
	break;

      case KEY_RIGHT:

	if (vln->len != col)
	{
	  ve_col = (mode & VE_ANSI) ? ansi2n(pos + 1, vln) : col + 1;
	  continue;
	}

	if (!(tmp = vln->next))
	  goto ve_key;

	ve_row++;
	ve_lno++;
	ve_col = 0;
	vx_cur = tmp;
	break;

      case KEY_HOME:
      case Ctrl('A'):

	ve_col = 0;
	continue;

      case KEY_END:
      case Ctrl('E'):

	ve_col = vln->len;
	continue;

      case KEY_UP:
      case Ctrl('P'):

	if (!(tmp = vln->prev))
	  goto ve_key;

	ve_row--;
	ve_lno--;
	if (mode & VE_ANSI)
	{
	  ve_col = ansi2n(pos, tmp);
	}
	else
	{
	  cc = tmp->len;
	  if (col > cc)
	    ve_col = cc;
	}
	vx_cur = tmp;
	break;

      
      case KEY_DOWN:
      case Ctrl('N'):

	if (!(tmp = vln->next))
	  goto ve_key;

	ve_row++;
	ve_lno++;
	if (mode & VE_ANSI)
	{
	  ve_col = ansi2n(pos, tmp);
	}
	else
	{
	  cc = tmp->len;
	  if (col > cc)
	    ve_col = cc;
	}
	vx_cur = tmp;
	break;

      case KEY_PGUP:
      case Ctrl('B'):

	ve_pageup();
	continue;

      case KEY_PGDN:
      case Ctrl('F'):
      case Ctrl('T'):		/* tail of file */

	ve_forward(cc == Ctrl('T') ? 0 : 22);
	continue;

      case Ctrl('S'):		/* start of file */

	vx_cur = vx_top = vx_ini;
	ve_col = ve_row = 0;
	ve_lno = 1;
	ve_mode = mode | VE_REDRAW;
	continue;

      case Ctrl('V'):		/* Toggle ANSI color */

	mode ^= VE_ANSI;
	clear();
	ve_mode = mode | VE_REDRAW;
	continue;

      case Ctrl('X'):		/* Save and exit */
      case Ctrl('W'):

	cc = ve_filer(fpath, ve_op & 11);
#ifdef  HAVE_INPUT_TOOLS
        if (cc == VE_INPUTOOL)
        {
          if(!input_tool)
          {
            input_tool = DL_get("bin/ascii.so:input_tools");
            if(input_tool)
              (*input_tool)();
          }
          else
          {
            (*input_tool)();
          }	
          break;
        }
#endif
	if (cc <= 0)
	{
	  bbsothermode &= ~OTHERSTAT_EDITING;
	  return cc;
	}
	ve_mode = mode | cc;
	continue;

      case Ctrl('Z'):

	film_out(FILM_EDIT, -1);
	ve_mode = mode | VE_REDRAW;
	continue;

      case Ctrl('C'):

	ve_ansi();
	break;

      case Ctrl('G'):		/* delete to end of file */

	/* vln->len = ve_col = cc = 0; */
        tmp = vln->next;
	vln->next = NULL;
	while (tmp)
	{
	  vln = tmp->next;
	  free(tmp);
	  tmp = vln;
	}
	ve_mode = mode | VE_REDRAW;
	continue;

      case Ctrl('Y'):		/* delete current line */

	vln->len = ve_col = 0;
        vln->data[0] = '\0'; /* Thor.981001: �N���e�@�ֲM�� */

      case Ctrl('K'):		/* delete to end of line */

	if ((cc = vln->len))
	{
	  if (cc != col)
	  {
	    vln->len = col;
	    vln->data[col] = '\0';
	    continue;
	  }

	  join_up(vln);
	}
	else
	{
	  tmp = vln->next;
	  if (!tmp)
	  {
	    tmp = vln->prev;
	    if (!tmp)
	      break;

	    if (ve_row > 0)
	    {
	      ve_row--;
	      ve_lno--;
	    }
	  }
	  if (vln == vx_top)
	    vx_top = tmp;
	  delete_line(vln);
	  vx_cur = tmp;
	}

	ve_mode = mode | VE_REDRAW;
	break;

      case Ctrl('U'):

        every_U();
	/*ve_char(27);*/
	break;
	
      case KEY_ESC:
              
        ve_char(27);
        break;
	

#ifdef SHOW_USER_IN_TEXT
      case Ctrl('Q'):
	{

/*
	  static char msg[] = "��ܨϥΪ̸�� (1)id (2)�ʺ١H[1]";

	  cc = vans(msg);
	  if (cc == '1' || cc == '2')
	    msg[sizeof(msg) - 3] = cc;
	  else
	    cc = msg[sizeof(msg) - 3];
	  cc -= '0';
	  for (col = cc * 12; col; col--)
	    ve_char(cc);
	}
	break;
*/

/* cache.090922: ����X */
  char *menu[] = 
  {
    "IQ",
    "Id     user�ע�(**s)",
    "Nick   user�ʺ�(**n)",
//    "Login  �n�J����(**l)", 
//    "Post   PO�妸��(**p)",     
    "Time   �{�b�ɶ�(**t)",    
    "Quit   ���}���     ",
    NULL
  };

    switch (cc = popupmenu_ans2(menu,"����X���", 7, 20))
      {
      case 'i':
      ve_char(KEY_ESC);
      ve_char('*');
      ve_char('s');
        break;
      case 'n':
      ve_char(KEY_ESC);
      ve_char('*');      
      ve_char('n');
        break;
/*      case 'l':
      ve_char(KEY_ESC);
      ve_char('*');      
      ve_char('l');
        break;
      case 'p':
      ve_char(KEY_ESC);
      ve_char('*');      
      ve_char('p');
        break;                
*/      case 't':
      ve_char(KEY_ESC);
      ve_char('*');   
      ve_char('t');
        break;       
      default :
        break;
      }
    }
    break;
                                                                                
#endif

      default:

	goto ve_key;
      }
    }

    /* ------------------------------------------------- */
    /* ve_row / ve_lno �վ�				 */
    /* ------------------------------------------------- */


    cc = ve_row;
    if (cc < 0)
    {
      ve_row = 0;
      if ((vln = vx_top->prev))
      {
	vx_top = vln;
	rscroll();
      }
      else
      {
	ve_abort(6);
      }
    }
    else if (cc >= b_lines)
    {
      ve_row = b_lines - 1;
      if ((vln = vx_top->next))
      {
	vx_top = vln;

	scroll();
      }
      else
      {
	ve_abort(7);
      }
    }
  }
}
