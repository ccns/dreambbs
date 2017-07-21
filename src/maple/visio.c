/*-------------------------------------------------------*/
/* visio.c	( NTHU CS MapleBBS Ver 3.00 )		 */
/*-------------------------------------------------------*/
/* target : VIrtual Screen Input Output routines 	 */
/* create : 95/03/29				 	 */
/* update : 96/10/10				 	 */
/*-------------------------------------------------------*/


#include <stdarg.h>
#include <arpa/telnet.h>

#include "bbs.h"

#define VO_MAX  (5120)
#define VI_MAX  (256)

#define INPUT_ACTIVE	0
#define INPUT_IDLE	1


#define	t_columns	80
#define	t_lines		24
#define	p_lines		18


int cur_row, cur_col;
int cur_pos;			/* current position with ANSI codes */


/* ----------------------------------------------------- */
/* output routines					 */
/* ----------------------------------------------------- */


static uschar vo_pool[VO_MAX];
static int vo_size;


#ifdef	VERBOSE
static void
telnet_flush(data, size)
  char *data;
  int size;
{
  int oset;

  oset = 1;

  if (select(1, NULL, &oset, NULL, NULL) <= 0)
  {
    abort_bbs();
  }

  xwrite(0, data, size);
}

#else

# define telnet_flush(data, size)	send(0, data, size, 0)
#endif


static void
oflush()
{
  int size;

  if ((size = vo_size))
  {
    telnet_flush(vo_pool, size);
    vo_size = 0;
  }
}


static void
output(str, len)
  uschar *str;
  int len;
{
  int size, ch;
  uschar *data;

  size = vo_size;
  data = vo_pool;
  if (size + len > VO_MAX - 8)
  {
    telnet_flush(data, size);
    size = len;
  }
  else
  {
    data += size;
    size += len;
  }

  while (--len >= 0)
  {
    ch = *str++;
    *data++ = ch;
    if (ch == IAC)
    {
      *data++ = ch;
      size++;
    }
  }
  vo_size = size;
}


static void
ochar(ch)
  int ch;
{
  uschar *data;
  int size;

  data = vo_pool;
  size = vo_size;
  if (size > VO_MAX - 2)
  {
    telnet_flush(data, size);
    size = 0;
  }
  data[size++] = ch;
  vo_size = size;
}


void
bell()
{
  static char sound[1] = {Ctrl('G')};

  telnet_flush(sound, sizeof(sound));
}


/* ----------------------------------------------------- */
/* virtual screen					 */
/* ----------------------------------------------------- */


#define	o_ansi(x)	output(x, sizeof(x)-1)

#define o_clear()	o_ansi("\033[;H\033[2J")
#define o_cleol()	o_ansi("\033[K")
#define o_standup()	o_ansi("\033[7m")
#define o_standdown()	o_ansi("\033[m")


static int docls;
static int roll;
static int scrollcnt, tc_col, tc_row;


static screenline vbuf[t_lines];
static screenline *cur_slp;	/* current screen line pointer */


void
move(y, x)
  int y;
  int x;
{
  screenline *cslp;

  if (y > b_lines)
    return;

  if (x >= t_columns)
    x = 0;

  cur_row = y;
  if ((y += roll) >= t_lines)
    y -= t_lines;
  cur_slp = cslp = &vbuf[y];
  cur_col = x;

  /* ------------------------------------- */
  /* �L�o ANSI codes�A�p���Яu���Ҧb��m */
  /* ------------------------------------- */

#if 0
  if (x && (cslp->mode & SL_ANSICODE))
  {
    int ch, ansi;
    uschar *str;

    ansi = 0;
    y = x;
    str = cslp->data;
    while (x && (ch = *str))
    {
      str++;
      if (ch == KEY_ESC)
      {
	ansi = YEA;
	y++;
	continue;
      }
      if (ansi)
      {
	y++;
	if (ch == 'm')		/* (!strchr(str_ansicode, ch)) */
	{
	  ansi = NA;
	}
	continue;
      }
      x--;
    }
    x += y;
  }
#endif

  cur_pos = x;
}

#if 0
/* verit : 030212 , ���� ansi code */
void
ansi_move(y, x)
  int y;
  int x;
{
  screenline *cslp;

  if (y > b_lines)
    return;

  if (x >= t_columns)
    x = 0;

  cur_row = y;
  if ((y += roll) >= t_lines)
    y -= t_lines;
  cur_slp = cslp = &vbuf[y];
  cur_col = x;

  if (x)
  {
    int ch, ansi;
    int len;
    uschar *str;

    ansi = 0;
    y = x;
    str = cslp->data;
    str[(len = cslp->len)] = '\0';
    while (len && (ch = *str))
    {
      str++;
      len--;
      if (ch == KEY_ESC)
      {
        ansi = YEA;
        y++;
        continue;
      }
      if (ansi)
      {
        y++;
        if (ch == 'm')          /* (!strchr(str_ansicode, ch)) */
        {
          ansi = NA;
        }
        continue;
      }
      x--;
      if(x<=0 && (ansi==NA))
        break;
    }
    x = y;
  }

  cur_pos = x;

}
#endif

void
getyx(y, x)
  int *y, *x;
{
  *y = cur_row;
  *x = cur_col;
}


/*-------------------------------------------------------*/
/* �p�� slp �� len ���B����� column �Ҧb		 */
/*-------------------------------------------------------*/


#if 0
static int
ansicol(slp, len)
  screenline *slp;
  int len;
{
  uschar *str;
  int ch, ansi, col;

  if (!len || !(slp->mode & SL_ANSICODE))
    return len;

  ansi = col = 0;
  str = slp->data;

  while (len-- && (ch = *str++))
  {
    if (ch == KEY_ESC && *str == '[')
    {
      ansi = YEA;
      continue;
    }
    if (ansi)
    {
      if (ch == 'm')
	ansi = NA;
      continue;
    }
    col++;
  }
  return col;
}
#endif


static void
rel_move(new_col, new_row)
  int new_col, new_row;
{
  int was_col, was_row;
  char buf[16];

  if (new_row >= t_lines || new_col >= t_columns)
    return;

  was_col = tc_col;
  was_row = tc_row;

  tc_col = new_col;
  tc_row = new_row;

  if (new_col == 0)
  {
    if (new_row == was_row)
    {
      if (was_col)
	ochar('\r');
      return;
    }
    else if (new_row == was_row + 1)
    {
      ochar('\n');
      if (was_col)
	ochar('\r');
      return;
    }
  }

  if (new_row == was_row)
  {
    if (was_col == new_col)
      return;

    if (new_col == was_col - 1)
    {
      ochar(Ctrl('H'));
      return;
    }
  }

  sprintf(buf, "\033[%d;%dH", new_row + 1, new_col + 1);
  output(buf, strlen(buf));
}


static void
standoutput(slp, ds, de)
  screenline *slp;
  int ds, de;
{
  uschar *data;
  int sso, eso;

  data = slp->data;
  sso = slp->sso;
  eso = slp->eso;

  if (eso <= ds || sso >= de)
  {
    output(data + ds, de - ds);
    return;
  }

  if (sso > ds)
    output(data + ds, sso - ds);
  else
    sso = ds;

  o_standup();
  output(data + sso, BMIN(eso, de) - sso);
  o_standdown();

  if (de > eso)
    output(data + eso, de - eso);
}


#define	STANDOUT	cur_slp->sso = cur_pos; cur_slp->mode |= SL_STANDOUT;
#define	STANDEND	cur_slp->eso = cur_pos;


#if 0
static int standing;


static void
standout()
{
  if (!standing)
  {
    standing = YEA;
    cur_slp->sso = cur_slp->eso = cur_pos;
    cur_slp->mode |= SL_STANDOUT;
  }
}


static void
standend()
{
  if (standing)
  {
    standing = NA;
    if (cur_slp->eso < cur_pos)
      cur_slp->eso = cur_pos;
  }
}
#endif


static void
vs_redraw()
{
  screenline *slp;
  int i, j, len, mode, width;

  tc_col = tc_row = docls = scrollcnt = vo_size = i = 0;
  o_clear();
  for (slp = &vbuf[j = roll]; i < t_lines; i++, j++, slp++)
  {
    if (j >= t_lines)
    {
      j = 0;
      slp = vbuf;
    }

    len = slp->len;
    width = slp->width;
    slp->oldlen = width;
    mode = slp->mode &=
      (len <= slp->sso) ? ~(SL_MODIFIED | SL_STANDOUT) : ~(SL_MODIFIED);
    if (len)
    {
      rel_move(0, i);

      if (mode & SL_STANDOUT)
	standoutput(slp, 0, len);
      else
	output(slp->data, len);

      tc_col = width;
    }
  }
  rel_move(cur_col, cur_row);
  oflush();
}


void
refresh()
{
  screenline *slp;
  int i, j, len, mode, width, smod, emod;

  i = scrollcnt;

  if ((docls) || ((i < 0 ? -i : i) >= p_lines))
  {
    vs_redraw();
    return;
  }

  if (i)
  {
    char buf[p_lines];

    scrollcnt = j = 0;
    if (i < 0)
    {
      sprintf(buf, "\033[%dL", -i);
      i = strlen(buf);
    }
    else
    {
      do
      {
	buf[j] = '\n';
      } while (++j < i);
      j = b_lines;
    }
    rel_move(0, j);
    output(buf, i);
  }

  for (i = 0, slp = &vbuf[j = roll]; i < t_lines; i++, j++, slp++)
  {
    if (j >= t_lines)
    {
      j = 0;
      slp = vbuf;
    }

    len = slp->len;
    width = slp->width;
    mode = slp->mode;

    if (mode & SL_MODIFIED)
    {
      slp->mode = mode &=
        (len <= slp->sso) ? ~(SL_MODIFIED | SL_STANDOUT) : ~(SL_MODIFIED);

      if ((smod = slp->smod) < len)
      {
	emod = slp->emod + 1;
	if (emod >= len)
	  emod = len;

	rel_move(smod, i);

	/* rel_move(ansicol(slp, smod), i); */

	if (mode & SL_STANDOUT)
	  standoutput(slp, smod, emod);
	else
	  output(&slp->data[smod], emod - smod);

	/* tc_col = ansicol(slp, emod); */

#if 0				/* 0501 */
	if (mode & SL_ANSICODE)
	{
	  uschar *data;

	  data = slp->data;
	  mode = 0;
	  len = emod;

	  while (len--)
	  {
	    smod = *data++;
	    if (smod == KEY_ESC)
	    {
	      mode = 1;
	      emod--;
	      continue;
	    }

	    if (mode)
	    {
	      if (smod == 'm')
		mode = 0;
	      emod--;
	    }
	  }
	}

	tc_col = emod;
#endif

	tc_col = (width != len) ? width : emod;
      }
    }

    if (slp->oldlen > width)
    {
      rel_move(width, i);
      o_cleol();
    }
    slp->oldlen = width;
  }
  rel_move(cur_col, cur_row);
  oflush();
}


void
clear()
{
  int i;
  screenline *slp;

  docls = YEA;
  cur_pos = cur_col = cur_row = roll = i = 0;
  cur_slp = slp = vbuf;
  while (i++ < t_lines)
  {
    memset(slp++, 0, 9);
  }
}

void
clearange(from,to)
  int from,to;
{
  int i;
  screenline *slp;

  docls = YEA;
  cur_pos = cur_col = roll = 0;
  i = cur_row = from;
  cur_slp = slp = &vbuf[from];
  while (i++ < to)
  {
    memset(slp++, 0, 9);
  }
}

void
clrtohol()
{
  int n;
  for(n=cur_col;n<36;n++) /* 36�O�ڪ��ʺA�ݪO�_�l��m, �ۤv�վ� */
    outc(' ');
}

void
clrtoeol()
{
  screenline *slp = cur_slp;
  int len;

  if ((len = cur_pos))
  {
    if ( len > slp->len)
      for ( len = slp->len; len < cur_pos; len++)
        slp->data[len] = ' ';
    slp->len = len;
    slp->width = cur_col;
  }
  else
  {
    memset((char *) slp + 1, 0, 8);
  }
}


void
clrtobot()
{
  screenline *slp;
  int i, j;

  i = cur_row;
  j = i + roll;
  slp = cur_slp;
  while (i < t_lines)
  {
    if (j >= t_lines)
    {
      j = 0;
      slp = vbuf;
    }
    memset((char *) slp + 1, 0, 8);

#if 0
    if (slp->oldlen)
      slp->oldlen = 255;
#endif

    i++;
    j++;
    slp++;
  }
}

void
outc(ch)
  int ch;
{
  screenline *slp;
  uschar *data;
  int i, cx, pos;

  static char ansibuf[16] = "\033";
  static int ansipos = 0;

  slp = cur_slp;
  pos = cur_pos;

  if (ch == '\n')
  {
    cx = cur_col;

new_line:

    ansipos = 0;
    if (pos)
    {
      slp->len = pos;
      slp->width = cx;

#if 0
      if (standing)
      {
	standing = NA;
	if (pos <= slp->sso)
	  slp->mode &= ~SL_STANDOUT;
	else if (slp->eso < pos)
	  slp->eso = pos;
      }
#endif
    }
    else
    {
      memset((char *) slp + 1, 0, 8);
    }

    move(cur_row + 1, 0);
    return;
  }

  if (ch < 0x20)
  {
    if (ch == KEY_ESC)
      ansipos = 1;

    return;
  }

  data = &(slp->data[pos]);	/* ���V�ثe��X��m */

  /* -------------------- */
  /* �ɨ��һݭn���ťզr�� */
  /* -------------------- */

  cx = slp->len - pos;
  if (cx > 0)
  {
    cx = *data;
  }
  else
  {
    while (cx < 0)
    {
      data[cx++] = ' ';
    }

    slp->len = /* slp->width = */ pos + 1;
  }

  /* ---------------------------- */
  /* ANSI control code ���S�O�B�z */
  /* ---------------------------- */

  if ((i = ansipos))
  {
    if ((i < 15) &&
      ((ch >= '0' && ch <= '9') || ch == '[' || ch == 'm' || ch == ';'))
    {
      ansibuf[i++] = ch;

      if (ch != 'm')
      {
	ansipos = i;
	return;
      }

      if (showansi)
      {
	ch = i + pos;
	if (ch < ANSILINELEN - 1)
	{
	  memcpy(data, ansibuf, i);
	  slp->len = slp->emod = cur_pos = ch;
	  slp->mode |= SL_MODIFIED;
	  if (slp->smod > pos)
	    slp->smod = pos;
	}
      }
    }
    ansipos = 0;
    return;
  }

  /* ---------------------------- */
  /* �P�w���Ǥ�r�ݭn���s�e�X�ù� */
  /* ---------------------------- */

  if ( /* !(slp->mode & SL_ANSICODE) && */ (ch != cx))
  {
    *data = ch;
    cx = slp->mode;
    if (cx & SL_MODIFIED)
    {
      if (slp->smod > pos)
	slp->smod = pos;
      if (slp->emod < pos)
	slp->emod = pos;
    }
    else
    {
      slp->mode = cx | SL_MODIFIED;
      slp->smod = slp->emod = pos;
    }
  }

  cur_pos = ++pos;
  cx = ++cur_col;

  if ((pos >= ANSILINELEN) /* || (cx >= t_columns) */ )
    goto new_line;

  if (slp->width < cx)
    slp->width = cx;
}


void
outs(str)
  uschar *str;
{
  int ch;

  while ((ch = *str))
  {
    outc(ch);
    str++;
  }
}


/* ----------------------------------------------------- */
/* eXtended output: �q�X user �� name �M nick		 */
/* ----------------------------------------------------- */

/* 090924.cache: pmore�ϥΪ�����X */

int
expand_esc_star_visio(char *buf, const char *src, int szbuf)
{
    if (*src != KEY_ESC)
    {
        strlcpy(buf, src, szbuf);
        return 0;
    }

    if (*++src != '*') // unknown escape... strip the ESC.
    {
        strlcpy(buf, src, szbuf);
        return 0;
    }

    switch(*++src)
    {
        // insecure content
        case 's':   // current user id
            strlcpy(buf, cuser.userid, szbuf);
            return 2;
        case 'n':   // current user nick
            strlcpy(buf, cuser.username, szbuf);
            return 2;
        case 'l':   // current user logins
            snprintf(buf, szbuf, "%d", cuser.numlogins);
            return 2;
        case 'p':   // current user posts
            snprintf(buf, szbuf, "%d", cuser.numposts);
            return 2;
        case 't':   // current time
            strlcpy(buf, Now(), szbuf);
            return 1;   
    }

    // unknown characters, return from star.
    strlcpy(buf, src-1, szbuf);
    return 0;
}

#ifdef SHOW_USER_IN_TEXT
void
outx(str)
  uschar *str;
{
/*
  uschar *t_name = cuser.userid;
  uschar *t_nick = cuser.username;
*/

  time_t now;
  int ch;

/* cache.090922: ����X */

  while (ch = *str)
  {
    if (ch == KEY_ESC)
    {
      str++;
      ch = *str;
      if (ch = '*')
      {
        str++;
        ch = *str;
        switch (ch)
        {
          case 's':       /* **s ��� ID */
            outs(cuser.userid);
            str++;
            str++;
            str++;
            str++;
            break;                                                                  
          case 'n':       /* **n ��ܼʺ� */
            outs(cuser.username);
            str++;
            str++;
            str++;
            str++;
            break;
          case 't':       /* **t ��ܤ�� */
            time(&now);
            outs(Ctime(&now));
            str++;
            str++;
            str++;
            str++;
            break;                                                                              
          default:
            str++;
            break;  
        }
      }
      else
      {
        str--;     
      }
      str--;
      str--;  
      str--;
      ch = *str;
      outc(ch);
      str++;
    }
    else
    {
      outc(ch);
      str++;
    }
  }
/*
  while ((ch = *str))
  {
        
        
        
    switch (ch)
    {
    case 1:
      if ((ch = *t_name) && (cuser.ufo2 & UFO2_SHOWUSER))
	t_name++;
      else
	ch = (cuser.ufo2 & UFO2_SHOWUSER) ? ' ' : '#';
      break;

    case 2:
      if ((ch = *t_nick) && (cuser.ufo2 & UFO2_SHOWUSER))
	t_nick++;
      else
	ch = (cuser.ufo2 & UFO2_SHOWUSER) ? ' ' : '%';
    }
    outc(ch);
    str++;
  }
*/  
  
}
#endif

/* ----------------------------------------------------- */
/* clear the bottom line and show the message		 */
/* ----------------------------------------------------- */


void
outz(msg)
  uschar *msg;
//  const char *msg;
{
  int ch;

  move(b_lines, 0);
  clrtoeol();
  while ((ch = *msg))
  {
    outc(ch);
    msg++;
  }
}

void
outf(str)
  uschar *str;
{
  outz(str);
  prints("%*s\033[m", d_cols, "");
}

void
prints(char *fmt, ...)
{
  va_list args;
  uschar buf[512],*str;
//  char buf[512], *str;
  int cc;

  va_start(args,fmt);
  vsprintf(buf, fmt, args);
  va_end(args);
  for (str = buf; (cc = *str); str++)
    outc(cc);
}


void
scroll()
{
  scrollcnt++;
  if (++roll >= t_lines)
    roll = 0;
  move(b_lines, 0);
  clrtoeol();
}


void
rscroll()
{
  scrollcnt--;
  if (--roll < 0)
    roll = b_lines;
  move(0, 0);
  clrtoeol();
}


/* ----------------------------------------------------- */


static int old_col, old_row, old_roll;
static int old_pos; /* Thor.990401: �h�s�@�� */


/* static void */
void				/* Thor.1028: ���F�� talk.c
				 * ���H�I�s�ɷ|show�r */
cursor_save()
{
  old_col = cur_col;
  old_row = cur_row;

  old_pos = cur_pos; /* Thor.990401: �h�s�@�� */
}


/* static void */
void				/* Thor.1028: ���F�� talk.c
				 * ���H�I�s�ɷ|show�r */
cursor_restore()
{
  move(old_row, old_col);
  
  cur_pos = old_pos; /* Thor.990401: �h�٭�@�� */
}


void
save_foot(slp)
  screenline *slp;
{
#if 0
  cursor_save();
  /* Thor.980827: �Ҧ��Ψ� save_foot���ɾ����|���s�w��, �G���Φs��Ц�m */
#endif

  move(b_lines - 1, 0);
  memcpy(slp, cur_slp, sizeof(screenline) * 2);
  slp[0].smod = 0;
  slp[0].emod = 255; /* Thor.990125:���׳̫�@������, �����nø�W */
  slp[0].oldlen = 255;
  slp[0].mode |= SL_MODIFIED;
  slp[1].smod = 0;
  slp[1].emod = 255; /* Thor.990125:���׳̫�@������, �����nø�W */
  slp[1].oldlen = 255;
  slp[1].mode |= SL_MODIFIED;
}


void
restore_foot(slp)
  screenline *slp;
{
  move(b_lines - 1, 0);
  memcpy(cur_slp, slp, sizeof(screenline) * 2);
#if 0
  cursor_restore();
  /* Thor.980827: �Ҧ��Ψ� restore_foot���ɾ����|���s�w��, �G���Φs��Ц�m */
#endif
  refresh(); /* Thor.981222: ������screen��ø, �ĥ� refresh */
  /* vs_redraw();*/  /* lkchu.981201: �� refresh() �|�� b_lines - 1 ��M�� */
}


int
vs_save(slp)
  screenline *slp;
{
#if 0
  cursor_save();
  /* Thor.980827: �Ҧ��Ψ� vs_save���ɾ����|���s�w��, �G���Φs��Ц�m */
#endif
  old_roll = roll;
  memcpy(slp, vbuf, sizeof(screenline) * t_lines);
  return old_roll;	/* itoc.030723: �Ǧ^�ثe�� roll */
}


void
vs_restore(slp)
  screenline *slp;
{
  memcpy(vbuf, slp, sizeof(screenline) * t_lines);
#if 0
  cursor_restore();
  /* Thor.980827: �Ҧ��Ψ� vs_restore���ɾ����|���s�w��, �G���Φs��Ц�m */
#endif
  roll = old_roll;
  vs_redraw();
}


int
vmsg(msg)
  char *msg;			/* length < 54 */
{
  
  if (msg)
  {
    move(b_lines, 0);
    clrtoeol();
    prints("\033[34;46m �� %-54s\033[31;47m [�Ы����N���~��] \033[m", msg);
  }
  else
  {
    int color;
    move(b_lines, 0);
    clrtoeol();
    color =time(0)%6+31;
#ifdef HAVE_COLOR_VMSG 
    prints("\033[1;%dm                                             �j�k�l�m�n�o�p \033[1;37m�Ы����N���~�� \033[1;%dm�p\033[m ",color,color);
#else    
    outs("\033[1;37;45m                              �� �Ы����N���~�� ��                           \033[m");    
#endif    
  }
  return vkey();
}


static inline void
zkey()				/* press any key or timeout */
{
  /* static */ struct timeval tv = {1, 100};  
  /* Thor.980806: man page ���] timeval struct�O�|���ܪ� */
                                                 
  int rset;

  rset = 1;
  select(1, (fd_set *) &rset, NULL, NULL, &tv);

#if 0
  if (select(1, &rset, NULL, NULL, &tv) > 0)
  {
    recv(0, &rset, sizeof(&rset), 0);
  }
#endif
}


void
zmsg(msg)			/* easy message */
  char *msg;
{
#if 0
  move(b_lines, 0);
  clrtoeol();
  outs(msg);
#endif
  outz(msg); /* Thor.980826: �N�O outz�� */

  refresh();
  zkey();
}


void
vs_bar(title)
  char *title;
{
  clear();
  prints("\033[1;33;44m�i %s �j\033[m\n", title);
}


#if 0
void
cursor_show(row, column)
  int row, column;
{
  move(row, column);
  outs(STR_CURSOR);
  move(row, column + 1);
}


void
cursor_clear(row, column)
  int row, column;
{
  move(row, column);
  outs(STR_UNCUR);
}


int
cursor_key(row, column)
  int row, column;
{
  int ch;

  cursor_show(row, column);
  ch = vkey();
  move(row, column);
  outs(STR_UNCUR);
  return ch;
}
#endif


static void
vs_line(msg)
  char *msg;
{
  int head, tail;

  if (msg)
    head = (strlen(msg) + 1) >> 1;
  else
    head = 0;

  tail = head;

  while (head++ < 38)
    outc('-');

  if (tail)
  {
    outc(' ');
    outs(msg);
    outc(' ');
  }

  while (tail++ < 38)
    outc('-');
  outc('\n');
}

/* 090127.cache �H�J�H�X�S�� */
void
grayout(int type)
// GRAYOUT_DARK(0): dark, GRAYOUT_BOLD(1): bold, GRAYOUR_NORMAL(2): normal
{
  screenline slp[T_LINES], newslp[T_LINES];
  char *prefix[3] = { "\033[1;30m", "\033[1;37m", "\033[0;37m" };
  char buf[ANSILINELEN];
  register int i;
                                                                                
  vs_save(slp);
  memcpy(newslp, slp, sizeof(newslp));

  for (i = 0; i < T_LINES; i++)
  {
      if (!newslp[i].width)
  continue;

      newslp[i].oldlen = newslp[i].len;
      newslp[i].len = newslp[i].width + 7 + 3;

      str_ansi(buf, slp[i].data, slp[i].width + 1);
      sprintf(newslp[i].data, "%s%s\033[m" , prefix[type], buf);
  }
  vs_restore(newslp);
}

/* ----------------------------------------------------- */
/* input routines					 */
/* ----------------------------------------------------- */


static uschar vi_pool[VI_MAX];
static int vi_size;
static int vi_head;


/* static int vio_fd; */
int vio_fd;			/* Thor.0725: ���H��btalk & chat �i ^z �@�ǳ� */

#ifdef EVERY_Z
int holdon_fd;			/* Thor.0727: ���Xchat&talk�Ȧsvio_fd�� */
#endif


static struct timeval vio_to = {60, 0};


void
add_io(fd, timeout)
  int fd;
  int timeout;
{
  vio_fd = fd;
  vio_to.tv_sec = timeout;
}


static inline int
iac_count(current)
  uschar *current;
{
  switch (*(current + 1))
  {
  case DO:
  case DONT:
  case WILL:
  case WONT:
    return 3;

  case SB:			/* loop forever looking for the SE */
    {
      uschar *look = current + 2;

      for (;;)
      {
	if ((*look++) == IAC)
	{
	  if ((*look++) == SE)
	  {
	    return look - current;
	  }
	}
      }
    }
  }
  return 1;
}


int
igetch()
{

#define	IM_TRAIL	0x01
#define	IM_REPLY	0x02	/* ^R */
#define	IM_TALK		0x04

  static int imode = 0;
  static int idle = 0;

  int cc, fd=0, nfds, rset;
  uschar *data;

  data = vi_pool;
  nfds = 0;

  for (;;)
  {
    if (vi_size <= vi_head)
    {
      if (nfds == 0)
      {
	refresh();
	fd = (imode & IM_REPLY) ? 0 : vio_fd;
	nfds = fd + 1;
	if (fd)
	  fd = 1 << fd;
      }

      for (;;)
      {
        struct timeval tv = vio_to;
        /* Thor.980806: man page ���] timeval �O�|���ܪ� */

	rset = 1 | fd;
	cc = select(nfds, (fd_set *) & rset, NULL, NULL, &tv /*&vio_to*/);
                        /* Thor.980806: man page ���] timeval �O�|���ܪ� */

	if (cc > 0)
	{
	  if (fd & rset)
	    return I_OTHERDATA;

	  cc = recv(0, data, VI_MAX, 0);
	  if (cc > 0)
	  {
	    vi_head = (*data) == IAC ? iac_count(data) : 0;
	    if (vi_head >= cc)
	      continue;
	    vi_size = cc;

	    if (idle && cutmp)
	    {

	      cutmp->idle_time = idle = 0;
	    }
#ifdef	HAVE_SHOWNUMMSG
            if(cutmp)
	      cutmp->num_msg = 0;
#endif	      	    
	    break;
	  }
	  if ((cc == 0) || (errno != EINTR))
	    abort_bbs();
	}
	else if (cc == 0)
	{
	  cc = vio_to.tv_sec;
	  if (cc < 60)		/* paging timeout */
	    return I_TIMEOUT;

	  idle += cc / 60;
	  vio_to.tv_sec = cc + 60; /* Thor.980806: �C�� timeout���W�[60��,
                                                   �ҥH���l�U���U�C, �n�i:p */
          /* Thor.990201: ����: ���Ftalk_rqst, chat���~, �ݭn�b�ʤ@�ʤ���
                                ���] tv_sec�� 60���? (�w�]��) */

      if (idle >= 5 && !cutmp)  /* �n�J�� idle �W�L 5 �����N�_�u */
      {
        pmsg2("�n�J�O�ɡA�Э��s�W��");
        refresh();
        abort_bbs();
      }

	  cc = bbsmode;
	  if (idle > (cc ? IDLE_TIMEOUT : 4))
	  {
	    clear();
	    outs("�W�L���m�ɶ��I");
	    refresh();
	    abort_bbs();
	  }
	  else if(idle > (cc ? (IDLE_TIMEOUT-4) : 4))
	  {
            outz("\033[41;5;1;37mĵ�i�I�A�w�g���m�L�[�A�t�αN�b�T����N�A�𰣡I\033[m");
            refresh();
	  }
          
	  if (cc)
	  {
	    cutmp->idle_time = idle;
	    /*if (cc < M_CLASS)
	    {
	      movie();
	      refresh();
	    }*/
	  }
	}
	else
	{
	  if (errno != EINTR)
	    abort_bbs();
	}
      }
    }

    cc = data[vi_head++];
    if (imode & IM_TRAIL)
    {
      imode ^= IM_TRAIL;
      if (cc == 0 || cc == 0x0a)
	continue;
    }

    if (cc == 0x0d)
    {
      imode |= IM_TRAIL;
      return '\n';
    }

    if (cc == 0x7f)
    {
      return Ctrl('H');
    }

    if (cc == Ctrl('L'))
    {
      vs_redraw();
      continue;
    }

    if ((cc == Ctrl('R')) && (bbstate & STAT_STARTED) && !(bbstate & STAT_LOCK) 
         && !(imode & IM_REPLY))		/* lkchu.990513: ��w�ɤ��i�^�T */
    {
      /*
       * Thor.980307: �Q���줰��n��k, �b^R�ɸT��talk, �_�h�|�] ,
       * �S��vio_fd, �ݤ��� I_OTHERDATA �ҥH�b ctrl-r��talk, �ݤ����襴���r
       */
      signal(SIGUSR1, SIG_IGN);

      imode |= IM_REPLY;
      bmw_reply(0);
      imode ^= IM_REPLY;

      /*
       * Thor.980307: �Q���줰��n��k, �b^R�ɸT��talk, �_�h�|�] ,
       * �S��vio_fd, �ݤ��� I_OTHERDATA �ҥH�b ctrl-r��talk, �ݤ����襴���r
       */
      signal(SIGUSR1, (void *) talk_rqst);

      continue;
    }

    return (cc);
  }
}


#define	MATCH_END	0x8000
/* Thor.990204: ����: �N��MATCH����, �n���N�ɨ�, 
                      �n���N�����쪬, ���q�X�i�઺�ȤF */

static void
match_title()
{
  move(2, 0);
  clrtobot();
  vs_line("������T�@����");
}


static int
match_getch()
{
  int ch;

  outs("\n�� �C��(C)�~�� (Q)���� ? [C] ");
  ch = vkey();
  if (ch == 'q' || ch == 'Q')
    return ch;

  move(3, 0);
  clrtobot();
  return 0;
}


/* ----------------------------------------------------- */
/* ��� board	 					 */
/* ----------------------------------------------------- */


static BRD *xbrd;


BRD *
ask_board(board, perm, msg)
  char *board;
  int perm;
  char *msg;
{
  if (msg)
  {
    move(2, 0);
    outs(msg);
  }

  if (vget(1, 0, "�п�J�ݪO�W��(���ť���۰ʷj�M)�G",
      board, IDLEN + 1, GET_BRD | perm))
  {
    if (!str_cmp(board, currboard))
      *board = 0;		/* ��ثe���ݪO�@�� */
    return xbrd;
  }

  return NULL;
}


static int
vget_match(prefix, len, op)
  char *prefix;
  int len;
  int op;
{
  char *data, *hit=NULL;
  int row, col, match;

  row = 3;
  col = match = 0;

  if (op & GET_BRD)
  {
    usint perm;
    char *bits;
    BRD *head, *tail;

    extern BCACHE *bshm;
    extern char brd_bits[];

    perm = op & (BRD_R_BIT | BRD_W_BIT);
    bits = brd_bits;
    head = bshm->bcache;
    tail = head + bshm->number;

    do
    {
      if (perm & *bits++)
      {
	data = head->brdname;

	if (str_ncmp(prefix, data, len))
	  continue;

	xbrd = head;

	if ((op & MATCH_END) && !data[len])
	{
	  strcpy(prefix, data);
	  return len;
	}

	match++;
	hit = data;

	if (op & MATCH_END)
	  continue;

	if (match == 1)
	  match_title();

	move(row, col);
	outs(data);

	col += IDLEN + 1;
	if (col >= 77)
	{
	  col = 0;
	  if (++row >= b_lines)
	  {
	    if (match_getch() == 'q')
	      break;

	    move(row = 3, 0);
	    clrtobot();
	  }
	}
      }
    } while (++head < tail);
  }
  else if (op & GET_USER)
  {
    struct dirent *de;
    DIR *dirp;
    int cc;
    int cd;    
    char fpath[16];

    /* Thor.981203: USER name�ܤ֥��@�r, ��"<="�|����n��? */
//    if (len == 0)
//      return 0;
    if (len)
    {
    cc = *prefix;
    if (cc >= 'A' && cc <= 'Z')
      cc |= 0x20;
    if (cc < 'a' || cc > 'z')
      return 0;
    cd = cc;
    }
    else
    {
      if (!HAS_PERM(PERM_ADMIN))  /* �@��ϥΪ̭n��J�@�r�~��� */
        return 0;
      cc = 'a';
      cd = 'z';
    }

    for (; cc <= cd; cc++)
    {
    sprintf(fpath,"usr/%c", cc);
    dirp = opendir(fpath);
    while ((de = readdir(dirp)))
    {
      data = de->d_name;
      if (*data <= ' ' || *data == '.')
        continue;
 
 //     if (str_ncmp(prefix, data, len))
      if (len && str_ncmp(prefix, data, len))
  	    continue;

      if (!match++)
      {
	match_title();
	strcpy(hit = fpath, data);	/* �Ĥ@���ŦX����� */
      }

      move(row, col);
      outs(data);
      col += IDLEN + 1;

      if (col >= 72)
      {
	col = 0;
	if (++row >= b_lines)
	{
	  if (match_getch())
         {
           cc = 'z';     /* ���} for �j�� */
    	   break;
         }	    
	  row = 3;
	}
      }
    }

    closedir(dirp);
    }
  }
  else /* Thor.990203: ����, GET_LIST */
  {
    LinkList *list;
    extern LinkList *ll_head;

    for (list = ll_head; list; list = list->next)
    {
      data = list->data;

      if (str_ncmp(prefix, data, len))
	continue;

      if ((op & MATCH_END) && !data[len])
      {
	strcpy(prefix, data);
	return len;
      }

      match++;
      hit = data;

      if (op & MATCH_END)
	continue;

      if (match == 1)
	match_title();

      move(row, col);
      outs(data);

      col += IDLEN + 1;
      if (col >= 77)
      {
	col = 0;
	if (++row >= b_lines)
	{
	  if (match_getch())
	    break;
	  row = 3;
	}
      }
    }
  }

  if (match == 1)
  {
    strcpy(prefix, hit);
    return strlen(hit);
  }

  return 0;
}


char lastcmd[MAXLASTCMD][80];


int vget(int line,int col,uschar *prompt,uschar *data,int max,int echo)
{
  int ch, len;
  int x, y;
  int i, next;

  if (prompt)
  {
    move(line, col);
    clrtoeol();
    outs(prompt);
  }
  else
  {
    clrtoeol();
  }

  STANDOUT;

  y = cur_row;
  x = cur_col;

  if (echo & GCARRY)
  {
    if ((len = strlen(data)))
      outs(data);
  }
  else
  {
    len = 0;
  }

  /* --------------------------------------------------- */
  /* ���o board / userid / on-line user			 */
  /* --------------------------------------------------- */

  ch = len;
  do
  {
    outc(' ');
  } while (++ch < max);

  STANDEND;

  line = -1;
  col = len;
  max--;

  for (;;)
  {
    move(y, x + col);
    ch = vkey();
    if (ch == '\n')
    {
      data[len] = '\0';
      if ((echo & (GET_BRD | GET_LIST)) && len > 0) 
      /* Thor.990204:�n�D��J���@�r�~�N��۰� match, �_�h��cancel */
      {
	ch = len;
	len = vget_match(data, len, echo | MATCH_END);
	if (len > ch)
	{
	  move(y, x);
	  outs(data);
	}
	else if (len == 0)
	{
	  data[0] = '\0';
	}
      }
      break;
    }

    if (isprint2(ch))
    {
      if (ch == ' ' && (echo & (GET_USER | GET_BRD | GET_LIST)))
      {
	ch = vget_match(data, len, echo);
	if (ch > len)
	{
	  move(y, x);
	  outs(data);
	  col = len = ch;
	}
	continue;
      }

      if (len >= max)
	continue;

      /* ----------------------------------------------- */
      /* insert data and display it			 */
      /* ----------------------------------------------- */

      prompt = &data[col];
      i = col;
      move(y, x + col);

      for (;;)
      {
	outc(echo ? ch : '*');
	next = *prompt;
	*prompt++ = ch;
	if (i >= len)
	  break;
	i++;
	ch = next;
      }
      col++;
      len++;
      continue;
    }

    /* ----------------------------------------------- */
    /* ��J password / match-list �ɥu��� BackSpace   */
    /* ----------------------------------------------- */

#if 0
    if ((!echo || mfunc) && ch != Ctrl('H'))
      continue;
#endif

    if (!echo && ch != Ctrl('H'))
      continue;

    switch (ch)
    {
    case Ctrl('D'):

      if (col >= len)
	continue;

      col++;

    case Ctrl('H'):

      if (!col)
	continue;

      /* ----------------------------------------------- */
      /* remove data and display it			 */
      /* ----------------------------------------------- */

      i = col--;
      len--;
      move(y, x + col);
      while (i <= len)
      {
	data[i - 1] = ch = data[i];
	outc(echo ? ch : '*');
	i++;
      }
      outc(' ');
      break;

    case KEY_LEFT:
    case Ctrl('B'):
      if (col)
	--col;
      break;

    case KEY_RIGHT:
    case Ctrl('F'):
      if (col < len)
	++col;
      break;

    case KEY_HOME:
    case Ctrl('A'):
      col = 0;
      break;

    case KEY_END:
    case Ctrl('E'):
      col = len;
      break;

    case Ctrl('Y'):		/* clear / reset */
      if (len)
      {
	move(y, x);
	for (ch = 0; ch < len; ch++)
	  outc(' ');
	col = len = 0;
      }
      break;

    case KEY_DOWN:
    case Ctrl('N'):

      line += MAXLASTCMD - 2;

    case KEY_UP:
    case Ctrl('P'):

      line = (line + 1) % MAXLASTCMD;
      prompt = lastcmd[line];
      col = 0;
      move(y, x);

      do
      {
	if (!(ch = *prompt++))
	{
	  /* clrtoeol */

	  for (ch = col; ch < len; ch++)
	    outc(' ');
	  break;
	}

	outc(ch);
	data[col] = ch;
      } while (++col < max);

      len = col;
      break;

    case Ctrl('K'):		/* delete to end of line */
      if (col < len)
      {
	move(y, x + col);
	for (ch = col; ch < len; ch++)
	  outc(' ');
	len = col;
      }
      break;
    }
  }

  if (len > 2 && echo)
  {
    for (line = MAXLASTCMD - 1; line; line--)
      strcpy(lastcmd[line], lastcmd[line - 1]);
    strcpy(lastcmd[0], data);
  }

  outc('\n');

  ch = data[0];
  if ((echo & LCECHO) && (ch >= 'A' && ch <= 'Z'))
    data[0] = (ch += 32);

  return ch;
}


int
vans(prompt)
  char *prompt;
{
  char ans[3];

  return vget(b_lines, 0, prompt, ans, sizeof(ans), LCECHO);
}


#undef	TRAP_ESC

#ifdef	TRAP_ESC
int
vkey()
{
  int mode;
  int ch, last;

  mode = last = 0;
  for (;;)
  {
    ch = igetch();
    if (mode == 0)
    {
      if (ch == KEY_ESC)
	mode = 1;
      else
	return ch;		/* Normal Key */
    }
    else if (mode == 1)
    {				/* Escape sequence */
      if (ch == '[' || ch == 'O')
	mode = 2;
      else if (ch == '1' || ch == '4')
	mode = 3;
      else
      {
	return Meta(ch);
      }
    }
    else if (mode == 2)
    {				/* Cursor key */
      if (ch >= 'A' && ch <= 'D')
	return KEY_UP - (ch - 'A');
      else if (ch >= '1' && ch <= '6')
	mode = 3;
      else
	return ch;
    }
    else if (mode == 3)
    {				/* Ins Del Home End PgUp PgDn */
      if (ch == '~')
	return KEY_HOME - (last - '1');
      else
	return ch;
    }
    last = ch;
  }
}

#else				/* TRAP_ESC */

int
vkey()
{
  int mode;
  int ch, last;

  mode = last = 0;
  for (;;)
  {
    ch = igetch();
    if (ch == KEY_ESC)
      mode = 1;
    else if (mode == 0)		/* Normal Key */
    {
      return ch;
    }
    else if (mode == 1)
    {				/* Escape sequence */
      if (ch == '[' || ch == 'O')
	mode = 2;
      else if (ch == '1' || ch == '4')
	mode = 3;
      else
	return ch;
    }
    else if (mode == 2)
    {				/* Cursor key */
      if (ch >= 'A' && ch <= 'D')
	return KEY_UP - (ch - 'A');
      else if (ch >= '1' && ch <= '6')
	mode = 3;
      else
	return ch;
    }
    else if (mode == 3)
    {				/* Ins Del Home End PgUp PgDn */
      if (ch == '~')
	return KEY_HOME - (last - '1');
      else
	return ch;
    }
    last = ch;
  }
}
#endif				/* TRAP_ESC */
