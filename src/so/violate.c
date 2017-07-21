/*-------------------------------------------------------*/
/* viol.c   ( YZU_CSE WindTop BBS )                     */
/*-------------------------------------------------------*/
/* author : visor.bbs@bbs.yzu.edu.tw                     */
/* target : administrator routines                       */
/* create : 2000/01/02                                   */
/* update : NULL                                         */
/*-------------------------------------------------------*/

#undef	_MODES_C_
#include "bbs.h"

extern XZ xz[];

static int viol_add();


static void
viol_item(num, viol)
int num;
EMAIL *viol;
{
	char buf[5];
	int now;
	now = (viol->deny - time(0)) / 3600;

	if (viol->deny == -1)
		sprintf(buf, "%s", "�ä[");
	else
		sprintf(buf, "%4d", now > 0 ? now : 0);
	prints("%6d %4d %4s %-56.56s\n", num, viol->times, buf, viol->email);
}

static int
viol_body(xo)
XO *xo;
{
	EMAIL *viol;
	int num, max, tail;

	move(3, 0);
	clrtobot();
	max = xo->max;
	if (max <= 0)
	{
		if (vans("�n�s�W��ƶ�(Y/N)�H[N] ") == 'y')
			return viol_add(xo);
		return XO_QUIT;
	}

	viol = (EMAIL *) xo_pool;
	num = xo->top;
	tail = num + XO_TALL;
	if (max > tail)
		max = tail;

	do
	{
		viol_item(++num, viol++);
	}
	while (num < max);

	return XO_NONE;
}


static int
viol_head(xo)
XO *xo;
{
	vs_head("�ȮɸT��W��", str_site);
	outs("\
		 [��]���} ^P)�s�W c)�ק� d)�R�� f)�j�M [h]elp\n\
		 \033[30;47m  �s�� ���� �ɶ� �T����U Email                                               \033[m");
	return viol_body(xo);
}


static int
viol_load(xo)
XO *xo;
{
	xo_load(xo, sizeof(EMAIL));
	return viol_body(xo);
}


static int
viol_init(xo)
XO *xo;
{
	xo_load(xo, sizeof(EMAIL));
	return viol_head(xo);
}


static int
viol_edit(viol, echo)
EMAIL *viol;
int echo;
{
	if (echo == DOECHO)
		memset(viol, 0, sizeof(EMAIL));
	if (vget(b_lines, 0, "E-mail�G", viol->email, sizeof(viol->email), echo))
		return 1;
	else
		return 0;
}


static int
viol_add(xo)
XO *xo;
{
	EMAIL viol;

	if (viol_edit(&viol, DOECHO))
	{
		rec_add(xo->dir, &viol, sizeof(EMAIL));
		xo->pos = XO_TAIL ;
		xo_load(xo, sizeof(EMAIL));
	}
	return viol_head(xo);
}

static int
viol_delete(xo)
XO *xo;
{

	if (vans(msg_del_ny) == 'y')
	{
		if (!rec_del(xo->dir, sizeof(EMAIL), xo->pos, NULL, NULL))
		{
			return viol_load(xo);
		}
	}
	return XO_FOOT;
}


static int
viol_change(xo)
XO *xo;
{
	EMAIL *viol, mate;
	int pos, cur;

	pos = xo->pos;
	cur = pos - xo->top;
	viol = (EMAIL *) xo_pool + cur;

	mate = *viol;
	viol_edit(viol, GCARRY);
	if (memcmp(viol, &mate, sizeof(EMAIL)))
	{
		rec_put(xo->dir, viol, sizeof(EMAIL), pos);
		move(3 + cur, 0);
		viol_item(++pos, viol);
	}

	return XO_FOOT;
}

static int
viol_find(xo)
XO *xo;
{
	EMAIL viol;
	int pos, fd;
	char buf[64];

	if (!vget(b_lines, 0, "�п�J�d�ߦr��:", buf, sizeof(buf), DOECHO))
		return XO_FOOT;

	fd = open(FN_VIOLATELAW_DB, O_RDONLY);

	pos = xo->pos + 1;

	while (fd)
	{
		lseek(fd, (off_t)(sizeof(EMAIL) * pos), SEEK_SET);
		if (read(fd, &viol, sizeof(EMAIL)) == sizeof(EMAIL))
		{
			if (str_str(viol.email, buf))
			{
				xo->pos = pos;
				close(fd);
				return viol_init(xo);
			}
			pos++;
		}
		else
			break;
	}
	close(fd);

	return viol_init(xo);
}


static int
viol_help(xo)
XO *xo;
{
	/*film_out(FILM_EMAIL, -1);*/
	return viol_head(xo);
}


KeyFunc viol_cb[] =
{
	{XO_INIT, viol_init},
	{XO_LOAD, viol_load},
	{XO_HEAD, viol_head},
	{XO_BODY, viol_body},

	{Ctrl('P'), viol_add},
	{'r', viol_change},
	{'f', viol_find},
	{'c', viol_change},
	{'s', viol_init},
	{'d', viol_delete},
	{'h', viol_help}
};


int
Violate()
{
	XO *xo;
	char fpath[64];

	utmp_mode(M_OMENU);
	sprintf(fpath, FN_VIOLATELAW_DB);
	xz[XZ_OTHER - XO_ZONE].xo = xo = xo_new(fpath);
	xz[XZ_OTHER - XO_ZONE].cb = viol_cb;
	xover(XZ_OTHER);
	free(xo);
	return 0;
}



