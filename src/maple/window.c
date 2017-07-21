/*-------------------------------------------------------*/
/* window.c	( NTHU CS MapleBBS Ver 3.10 )		 */
/*-------------------------------------------------------*/
/* target : popup window menu				 */
/* create : 03/02/12					 */
/* update : 03/07/23					 */
/* author : verit.bbs@bbs.yzu.edu.tw			 */
/* modify : itoc.bbs@bbs.tnfsh.tn.edu.tw		 */
/*-------------------------------------------------------*/

/* 90929.cache: �ĤG�عϧο�� */ 

#include "bbs.h"

static screenline slt[T_LINES];
static int x_roll;

/* ----------------------------------------------------- */
/* �e��ø�s						 */
/* ----------------------------------------------------- */


static void
draw_line(x, y, msg)	/* �b (x, y) ����m��J msg�A���k���n�L�X��Ӫ��m���r */
  int x, y;
  uschar *msg;
{
  uschar *str, *ptr;
  uschar data[ANSILINELEN];
  char color[4];
  int ch, i;
  int len;
  int ansi;		/* 1: �b ANSI �� */
  int in_chi = 0;	/* 1: �b����r�� */
  int fg, bg, hl;	/* �e��/�I��/���m */

  hl = 0;
  fg = 37;
  bg = 40;

  i = x + x_roll;
  if (i > b_lines)
    i -= b_lines + 1;

  memset(data, 0, sizeof(data));
  strncpy(data, slt[i].data, slt[i].len);
  str = data;
  
  move(x, 0);
  clrtoeol();

  /* �L�X (x, 0) �� (x, y - 1) */
  ansi = 0;
  len = 0;		/* �w�L�X�X�Ӧr (���t����X) */
  while (ch = *str++)
  {
    if (ch == KEY_ESC)
    {
      ansi = 1;
      i = 0;
    }
    else if (ansi)
    {
      if (ch == '[')
      {
      }
      else if (ch >= '0' && ch <= '9')
      {
	color[i] = ch;
	if (++i >= 4)
	  i = 0;
      }
      else
      {
	color[i] = 0;

	i = atoi(color);
	if (i == 0)
	{
	  hl = 0;
	  fg = 37;
	  bg = 40;
	}
	else if (i == 1)
	  hl = 1;
	else if (i >= 30 && i <= 37)
	  fg = i;
	else if (i >= 40 && i <= 47)
	  bg = i;

	i = 0;

	if (ch != ';')
	  ansi = 0;
      }
    }
    else
    {
      if (++len >= y)
      {
	/* �̫�@�r�Y�O����r�����X�A�N���L */
	if (!in_chi && IS_ZHC_HI(ch))
	{
	  outc(' ');
	  in_chi ^= 1;
	}
	else
	{
	  outc(ch);
	  in_chi = 0;
	}
	outs(str_ransi);
	break;
      }

      if (in_chi || IS_ZHC_HI(ch))
	in_chi ^= 1;
    }

    outc(ch);
  }
  while (len++ < y)
    outc(' ');

  /* �L�X (x, y) �� (x, y + strip_ansi_len(msg) - 1) */
  ptr = msg;
  ansi = 0;
  len = 0;		/* msg ������(���t����X) */
  while (ch = *ptr++)
  {
    if (ch == KEY_ESC)
    {
      ansi = 1;
    }
    else if (ansi)
    {
      if ((ch < '0' || ch > '9') && ch != ';' && ch != '[')
	ansi = 0;
    }
    else
    {
      len++;
    }
    outc(ch);
  }

  /* ���� str �����@��q�A�è��X�̫᪺�C�� */
  ansi = 0;
  while (ch = *str++)
  {
    if (ch == KEY_ESC)
    {
      ansi = 1;
      i = 0;
    }
    else if (ansi)
    {
      if (ch == '[')
	continue;
      if (ch >= '0' && ch <= '9')
      {
	color[i] = ch;
	if (++i >= 4)
	  i = 0;
      }
      else
      {
	color[i] = 0;

	i = atoi(color);
	if (i == 0)
	{
	  hl = 0;
	  fg = 37;
	  bg = 40;
	}
	else if (i == 1)
	  hl = 1;
	else if (i >= 30 && i <= 37)
	  fg = i;
	else if (i >= 40 && i <= 47)
	  bg = i;

	i = 0;

	if (ch != ';')
	  ansi = 0;
      }
    }
    else
    {
      if (--len < 0)	/* ���L strip_ansi_len(msg) ������ */
	break;

      if (in_chi || IS_ZHC_HI(ch))
	in_chi ^= 1;
    }
  }

  /* �L�X (x, y + strip_ansi_len(msg)) �o�Ӧr�Ϋ᭱������X */
  prints("\033[%d;%d;%dm", hl, fg, bg);
  /* ���r�Y�O����r�����X�A�N���L */
  outc(in_chi ? ' ' : ch);

  /* �L�X (x, y + strip_ansi_len(msg) + 1) �� ��� */
  outs(str);
  outs(str_ransi);
}

/* ----------------------------------------------------- */
/* �ﶵø�s						 */
/* ----------------------------------------------------- */


static int
draw_item(x, y, desc, hotkey, mode)
  int x, y;
  char *desc;
  char hotkey;
  int mode;		/* 0:�M������  1:�e�W���� */
{
  char buf[128];

  sprintf(buf, " �x%s%c %c%c%c%-25s  \033[m�x ",
    mode ? COLOR4 : "\033[30;47m", mode ? '>' : ' ',
    (hotkey == *desc) ? '[' : '(', *desc,
    (hotkey == *desc) ? ']' : ')', desc + 1);

  draw_line(x, y, buf);
}


static int	/* �^���`�@���X�ӿﶵ */
draw_menu(x, y, title, desc, hotkey, cur)
  int x, y;
  char *title;
  char *desc[];
  char hotkey;
  int *cur;	/* �^�ǹw�]�ȩҦb��m */
{
  int i, meet;
  char buf[128];

  draw_line(x++, y, " �~�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�� ");

  sprintf(buf, " �x" COLOR4 "  %-28s  \033[m�x ", title);
  draw_line(x++, y, buf);

  draw_line(x++, y, " �u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t ");

  for (i = 1; desc[i]; i++)
  {
    meet = (desc[i][0] == hotkey);
    draw_item(x++, y, desc[i], hotkey, meet);
    if (meet)
      *cur = i;
  }

  draw_line(x, y, " ���w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�� ");

  /* �קK�b�������k����ΤU�A������|�����G�h��檺���D */
  move(b_lines, 0);

  return i - 1;
}


/* ----------------------------------------------------- */
/* ��ﶵ						 */
/* ----------------------------------------------------- */


static int			/* -1:�䤣�� >=0:�ĴX�ӿﶵ */
find_cur(ch, max, desc)		/* �� ch �o�ӫ���O�ĴX�ӿﶵ */
  int ch, max;
  char *desc[];
{
  int i, cc;

  if (ch >= 'A' && ch <= 'Z')
    ch |= 0x20;		/* ���p�g */

  for (i = 1; i <= max; i++)
  {
    cc = desc[i][0];
    if (cc >= 'A' && cc <= 'Z')
      cc |= 0x20;	/* ���p�g */

    if (ch == cc)
      return i;
  }

  return -1;
}


/*------------------------------------------------------ */
/* �߰ݿﶵ�A�i�ΨӨ��N vans()				 */
/*------------------------------------------------------ */
/* x, y  �O�ۥX�������W���� (x, y) ��m			 */
/* title �O���������D					 */
/* desc  �O�ﶵ���ԭz�G					 */
/*       �Ĥ@�Ӧr�ꥲ������� char			 */
/*         �Ĥ@�Ӧr���N��@�}�l��а�����m		 */
/*         �ĤG�Ӧr���N����U KEY_LEFT ���w�]�^�ǭ�	 */
/*       �������r��O�C�ӿﶵ���ԭz (���r��������)	 */
/*       �̫�@�Ӧr�ꥲ���� NULL			 */
/*------------------------------------------------------ */

int		/* �Ǧ^�p�g�r���μƦr */
popupmenu_ans2(char *desc[],char *title,int x,int y)
{
  int cur, old_cur, max, ch;
  char hotkey;

  x_roll = vs_save(slt);

  hotkey = desc[0][0];

  /* �e�X��ӿ�� */
  max = draw_menu(x, y, title, desc, hotkey, &cur);
  x += 2;

  /* �@�i�J�A��а��b�w�]�� */
  old_cur = cur;

  while (1)
  {
    switch (ch = vkey())
    {
    case KEY_LEFT:
    case KEY_RIGHT:
    case '\n':
      vs_restore(slt);
      ch = (ch == KEY_LEFT) ? desc[0][1] : desc[cur][0];
      if (ch >= 'A' && ch <= 'Z')
	ch |= 0x20;		/* �^�Ǥp�g */
      return ch;

    case KEY_UP:
      cur = (cur == 1) ? max : cur - 1;
      break;

    case KEY_DOWN:
      cur = (cur == max) ? 1 : cur + 1;
      break;

    case KEY_HOME:
      cur = 1;
      break;

    case KEY_END:
      cur = max;
      break;

    default:		/* �h��ҫ���O���@�ӿﶵ */
      if ((ch = find_cur(ch, max, desc)) > 0)
	cur = ch;
      break;
    }

    if (old_cur != cur)		/* ����ܰʦ�m�~�ݭn��ø */
    {
      draw_item(x + old_cur, y, desc[old_cur], hotkey, 0);
      draw_item(x + cur, y, desc[cur], hotkey, 1);
      old_cur = cur;
      /* �קK�b�������k����ΤU�A������|�����G�h��檺���D */
      move(b_lines, 0);
    }
  }
}

/*------------------------------------------------------ */
/* �ۥX�������T���A�i�ΨӨ��N vmsg()			 */
/*------------------------------------------------------ */

int
pmsg2(char *msg)
/* ���i�� NULL */
{
  int len, x, y, i;
  char buf[80];

  x_roll = vs_save(slt);

  len = strlen(msg);
  if (len < 16)		/* �� msg title �䤤�����̬� len */
    len = 16;
  if (len % 2)		/* �ܦ����� */
    len++;
  x = (b_lines - 4) >> 1;	/* �m�� */
  y = (b_cols - 8 - len) >> 1;

  strcpy(buf, "�~");
  for (i = -4; i < len; i += 2)
    strcat(buf, "�w");
  strcat(buf, "��");
  draw_line(x++, y, buf);

  sprintf(buf, "�x" COLOR4 "  %-*s  \033[m�x", len, "�Ы����N���~��..");
  draw_line(x++, y, buf);

  strcpy(buf, "�u");
  for (i = -4; i < len; i += 2)
    strcat(buf, "�w");
   strcat(buf, "�t");
  draw_line(x++, y, buf);

  sprintf(buf, "�x\033[30;47m  %-*s  \033[m�x", len, msg);
  draw_line(x++, y, buf);

  strcpy(buf, "��");
  for (i = -4; i < len; i += 2)
    strcat(buf, "�w");
  strcat(buf, "��");
  draw_line(x++, y, buf);

  move(b_lines, 0);

  x = vkey();
  vs_restore(slt);
  return x;
}
