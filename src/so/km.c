/*-------------------------------------------------------*/
/* km.c         ( NTHU CS MapleBBS Ver 3.10 )            */
/*-------------------------------------------------------*/
/* target : KongMing Chess routines                      */
/* create : 01/02/08                                     */
/* update : 01/05/09                                     */
/* author : einstein@bbs.tnfsh.tn.edu.tw                 */
/* recast : itoc.bbs@bbs.tnfsh.tn.edu.tw                 */
/*-------------------------------------------------------*/


#include "bbs.h"


#ifdef HAVE_GAME

#define RETRACT_CHESS	/* �O�_���Ѯ��ѥ\�� */

#ifdef RETRACT_CHESS
#  define LOG_KM	/* �O�_���ѰO�����Ъ��\�� */
#endif


#if 0

�ѽL�b etc/game/km �榡�p�U�G

�Ĥ@����`�@���X�L�ѽL(�T���)�A�q�ĤT��}�l�h�O�@�L�@�L�����СC

TILE_NOUSE 0 ��ܤ��ಾ�ʪ���l
TILE_BLANK 1 ��ܪŮ�
TILE_CHESS 2 ��ܴѤl

#123�V�@�t��
0 0 2 2 2 0 0
0 0 2 2 2 0 0
2 2 2 2 2 2 2
2 2 2 1 2 2 2
2 2 2 2 2 2 2
0 0 2 2 2 0 0
0 0 2 2 2 0 0

#endif


enum
{
  KM_XPOS = 5,
  KM_YPOS = 5,
  MAX_X = 7,			/* �n�O�_�� */
  MAX_Y = 7,			/* �n�O�_�� */

  /* �� bitwise operators & �Ө��N == */
  TILE_NOUSE = 0,		/* ���ಾ�ʪ���l */
  TILE_BLANK = 1,		/* �Ů� */
  TILE_CHESS = 2		/* �Ѥl */
};


static int board[MAX_X][MAX_Y];
#ifdef LOG_KM
static int origin_board[MAX_X][MAX_Y];
#endif
static int cx, cy;
static int stage, NUM_TABLE;
static char piece[4][3] = {"�@", "��", "��", "��"};
static char title[20];		/* ���ЦW�� */

#ifdef RETRACT_CHESS
static int route[MAX_X * MAX_Y][4];	/* �O�� (fx, fy) -> (tx, ty)�A���ѨB�Ƥ��i��W�L�ѽL�j�p */
static int step;
#endif


static void
out_song()
{
  /* itoc.����: �C�y�ܳ��˦��@�˪��סA�N���� clrtoeol() :p */
  uschar *msg[8] = 
  {
    "�z�ӱj�F�A�N�O�o�ˡI",
    "�z���i��Q��o�@�B",
    "�o�u�O�ӯ��_�F�A�ǧJ",
    "�ڤ����D�ӻ��Ǥ���F",
    "�o�@�ۯu�O�ѤH�ⵧ�r",
    "�ӨتA�z�F�A�o�ˤ]��",
    "�֧����F�I�[�o�[�o�I",
    "�n�����Эn�i�D������"
  };
  move(21, 0);
  prints("\033[1;3%dm%s\033[m", time(0) % 7, msg[time(0) % 8]);
}


static void
show_board()
{
  int i, j;

  vs_bar("�թ���");
  move(2, KM_YPOS + MAX_Y - 6);		/* �m����ܴ��ЦW�� */
  outs(title);

  for (i = 0; i < MAX_X; i++)
  {
    for (j = 0; j < MAX_Y; j++)
    {
      move(KM_XPOS + i, KM_YPOS + j * 2);
      outs(piece[board[i][j]]);
    }
  }

  move(3, 40);
  outs("��������  ��V��");
  move(5, 40);
  outs("[Enter]   ���/�Ͽ��");
  move(7, 40);
  outs("Q/q       ���}");
  move(9, 40);
  outs("h         Ū�����нd��");

#ifdef RETRACT_CHESS
  move(11, 40);
  outs("r         ����");
#endif

  move(13, 40);
  outs("��        �Ŧ�");
  move(14, 40);
  outs("��        �Ѥl");
  move(15, 40);
  outs("��        ���");

  out_song();
  move(KM_XPOS + MAX_X / 2, KM_YPOS + MAX_Y / 2 * 2 + 1);	/* �@�}�l�N��иm�� */
}


static int
read_board()
{
  int i, j, count;
  FILE *fp;
  char buf[40], ans[4];

  if (!(fp = fopen("etc/game/km", "r")))
    return 0;

  if (stage < 0)	/* �Ĥ@���i�J�C�� */
  {
    fgets(buf, 4, fp);
    NUM_TABLE = atoi(buf);	/* etc/game/km �Ĥ@��O�����м� */
    sprintf(buf, "�п�ܽs�� [1-%d]�A[0] �H���X�D�A�Ϋ� [Q] ���}�G", NUM_TABLE);
    if (vget(b_lines, 0, buf, ans, 4, DOECHO) == 'q')
    {  
      fclose(fp);
      return 0;
    }  

    stage = atoi(ans) - 1;
    if (stage < 0 || stage >= NUM_TABLE)	/* �H���X�D */
      stage = time(0) % NUM_TABLE;
  }

  fseek(fp, 4 + stage * (2 * MAX_X * MAX_Y + 14), SEEK_SET);
  /* 4: �Ĥ@�檺�T��ƴ��мƥ�\n  14: \n#999���ЦW��\n */

  fscanf(fp, "%s", &title);		/* ���ЦW�� */

  count = 0;
  for (i = 0; i < MAX_X; i++)
  {
    for (j = 0; j < MAX_Y; j++)
    {
      fscanf(fp, "%d", &board[i][j]);
      if (board[i][j] & TILE_CHESS)
      {
	count++;
      }
    }
  }
  fclose(fp);

#ifdef LOG_KM
  memcpy(origin_board, board, sizeof(origin_board));
#endif

  return count;
}


static inline int
valid_pos(x, y)
  int x, y;
{
  if (x < 0 || x >= MAX_X || y < 0 || y >= MAX_Y || 
    board[x][y] == TILE_NOUSE)	/* TILE_NOUSE = 0 ����� & operation */
  {
    return 0;
  }
  return 1;
}


static void
get_pos(x, y)
  int *x, *y;
{
  int ch;
  while (1)
  {
    ch = vkey();
    if (ch == KEY_UP && valid_pos(cx - 1, cy))
    {
      cx--;
      move(KM_XPOS + cx, KM_YPOS + cy * 2 + 1);
    }
    else if (ch == KEY_DOWN && valid_pos(cx + 1, cy))
    {
      cx++;
      move(KM_XPOS + cx, KM_YPOS + cy * 2 + 1);
    }
    else if (ch == KEY_LEFT && valid_pos(cx, cy - 1))
    {
      cy--;
      move(KM_XPOS + cx, KM_YPOS + cy * 2 + 1);
    }
    else if (ch == KEY_RIGHT && valid_pos(cx, cy + 1))
    {
      cy++;
      move(KM_XPOS + cx, KM_YPOS + cy * 2 + 1);
    }
    else if (ch == 'h')
    {
      more("etc/game/km.hlp", NULL);
      show_board();
      move(KM_XPOS + cx, KM_YPOS + cy * 2 + 1);
    }
    else if (ch == 'q' || ch == 'Q')
    {
      vmsg(MSG_QUITGAME);
      *x = -1;
      break;
    }
    else if (ch == '\n')
    {
      *x = cx;
      *y = cy;
      break;
    }
#ifdef RETRACT_CHESS
    else if (ch == 'r')
    {
      *x = -2;
      break;
    }
#endif
  }
}


static inline void
jump(fx, fy, tx, ty)
  int fx, fy, tx, ty;		/* From (fx, fy) To (tx, ty) */
{
  out_song();

  board[fx][fy] = TILE_BLANK;
  move(KM_XPOS + fx, KM_YPOS + fy * 2);
  outs(piece[1]);

  board[(fx + tx) / 2][(fy + ty) / 2] = TILE_BLANK;
  move(KM_XPOS + (fx + tx) / 2, KM_YPOS + (fy + ty));
  outs(piece[1]);

  board[tx][ty] = TILE_CHESS;
  move(KM_XPOS + tx, KM_YPOS + ty * 2);
  outs(piece[2]);
  move(KM_XPOS + tx, KM_YPOS + ty * 2 + 1);

#ifdef RETRACT_CHESS
  route[step][0] = fx;
  route[step][1] = fy;
  route[step][2] = tx;
  route[step][3] = ty;
  step++;
#endif
}


#ifdef RETRACT_CHESS
static inline void
retract()
{
  int fx, fy, tx, ty;

  out_song();

  step--; 
  ty = route[step][3];
  tx = route[step][2];
  fy = route[step][1];
  fx = route[step][0];

  board[tx][ty] = TILE_BLANK;
  move(KM_XPOS + tx, KM_YPOS + ty * 2);
  outs(piece[1]);

  board[(fx + tx) / 2][(fy + ty) / 2] = TILE_CHESS;
  move(KM_XPOS + (fx + tx) / 2, KM_YPOS + (fy + ty));
  outs(piece[2]);

  board[fx][fy] = TILE_CHESS;
  move(KM_XPOS + fx, KM_YPOS + fy * 2);
  outs(piece[2]);
  move(KM_XPOS + fx, KM_YPOS + fy * 2);
  cx = fx;
  cy = fy;
}
#endif


static inline int
check(fx, fy, tx, ty)
  int fx, fy, tx, ty;
{
  if ((board[(fx + tx) / 2][(fy + ty) / 2] & TILE_CHESS) &&
    ((abs(fx - tx) == 2 && fy == ty) || (fx == tx && abs(fy - ty) == 2)))
  {
    return 1;
  }
  return 0;
}


static inline int
live()
{
  int dir[4][2] = {1, 0, -1, 0, 0, 1, 0, -1};
  int i, j, k, nx, ny, nx2, ny2;
  for (i = 0; i < MAX_X; i++)
  {
    for (j = 0; j < MAX_Y; j++)
    {
      for (k = 0; k < 4; k++)
      {
	nx = i + dir[k][0];
	ny = j + dir[k][1];
	nx2 = nx + dir[k][0];
	ny2 = ny + dir[k][1];
	if (valid_pos(nx2, ny2) && (board[i][j] & TILE_CHESS) && 
	  (board[nx][ny] & TILE_CHESS) && (board[nx2][ny2] & TILE_BLANK))
	{
	  return 1;
	}
      }
    }
  }
  return 0;
}


#ifdef LOG_KM
static void
log_km()
{
  int s, i, j;
  int fx, fy, tx, ty;
  char fpath[64], buf[80];
  FILE *fp;

  usr_fpath(fpath, cuser.userid, "km.log");
  fp = fopen(fpath, "w");
  fprintf(fp, "%s %s (%s)\n", str_author1, cuser.userid, cuser.username);
  fprintf(fp, "���D: �թ����� %s �}�ѹL�{\n�ɶ�: %s\n\n", title, Now());
  fprintf(fp, "%s\n\n", title);

  memcpy(board, origin_board, sizeof(board));
  fx = fy = tx = ty = -1;

  for (s = 0;; s++)
  {
    for (i = 0; i < MAX_X; i++)
    {
      for (j = 0; j < MAX_Y; j++)
      {
#if 0	/* �[�C��n���S����M�� */
	fprintf(fp, "%s%s%s", 
	  (i == fx && j == fy) ? "\033[1;43m" : (i == tx && j == ty) ? "\033[1;33m" : "", 
	  piece[board[i][j]], 
	  (i == fx && j == fy) || (i == tx && j == ty) ? str_ransi : "");
#endif
	fprintf(fp, "%s", piece[board[i][j]]);
      }
      fprintf(fp, "\n");
    }
    fprintf(fp, "\n");

    if (s >= step)
      break;

    fx = route[s][0];
    fy = route[s][1];
    tx = route[s][2];
    ty = route[s][3];
    board[fx][fy] = TILE_BLANK;
    board[(fx + tx) / 2][(fy + ty) / 2] = TILE_BLANK;
    board[tx][ty] = TILE_CHESS;
  }

  ve_banner(fp, 0);
  fclose(fp);

  sprintf(buf, "�թ����� %s �}�ѹL�{", title);
  mail_self(fpath, cuser.userid, buf, MAIL_READ);

  unlink(fpath);
}
#endif


int
main_km()
{
  int fx, fy, tx, ty, count;

  stage = -1;

start_game:

  if (!(count = read_board()))
    return 0;

#ifdef RETRACT_CHESS
  step = 0;
#endif

  show_board();
  cx = MAX_X / 2;
  cy = MAX_Y / 2;

  while (1)
  {
    if (count == 1 && board[MAX_X / 2][MAX_Y / 2] & TILE_CHESS)
    {			/* �̫�@�l�n�b������ */
      vmsg("���߱z���\\�F");

#ifdef LOG_KM
      if (vans("�z�O�_�n�⧹�������ЫO�s�b�H�c��(Y/N)�H[Y] ") != 'n')
	log_km();
#endif

      switch (vans("�п�� 1)�~��U�@�� 2)���s�D�Ԧ��� Q)���}�H[1] "))
      {
      case 'q':
        goto abort_game;
      case '2':
        stage--;
      default:
        if (++stage >= NUM_TABLE)
          stage = 0;
        goto start_game;
      }
    }
    if (!live())
    {
      vmsg("�V�|...�S�ѤF...@@");

      switch (vans("�п�� 1)�~��U�@�� 2)���s�D�Ԧ��� Q)���}�H[2] "))
      {
      case 'q':
        goto abort_game;
      case '1':
        if (++stage >= NUM_TABLE)
          stage = 0;
      default:
        goto start_game;
      }
    }

    while (1)		/* �Ĥ@�� */
    {
      get_pos(&fx, &fy);
      if (fx < 0)
      {
#ifdef RETRACT_CHESS
	if (fx == -2)
	{
	  if (step)	/* �@�B���٨S���A���஬�� */
	  {
	    retract();
	    count++;
	  }
	  continue;
	}
#endif
	goto abort_game;
      }
      if (!(board[fx][fy] & TILE_CHESS))
      {
	continue;
      }
      else		/* ��l */
      {
	move(KM_XPOS + fx, KM_YPOS + fy * 2);
	outs(piece[3]);
	move(KM_XPOS + fx, KM_YPOS + fy * 2 + 1);
	break;
      }
    }

    while (1)		/* �ĤG�� */
    {
      get_pos(&tx, &ty);
      if (tx < 0)
      {
#ifdef RETRACT_CHESS
	if (tx == -2)
	{
	  continue;	/* �n������l�~�஬�� */
	}
#endif
	goto abort_game;
      }
      if (fx == tx && fy == ty)	/* ����l */
      {
	move(KM_XPOS + tx, KM_YPOS + ty * 2);
	outs(piece[2]);
	move(KM_XPOS + tx, KM_YPOS + ty * 2 + 1);
	break;
      }
      else if (!(board[tx][ty] & TILE_BLANK) || !check(fx, fy, tx, ty))	/* ������a�褣��� */
      {
	continue;
      }
      else		/* ����Ӧa�� */
      {
	jump(fx, fy, tx, ty);
	count--;
	break;
      }
    }
  }
abort_game:
  return 0;
}

#endif	/* HAVE_GAME */
