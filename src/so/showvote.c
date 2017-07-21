/*-------------------------------------------------------*/
/* show.c    ( YZU_CSE WindTop BBS )                     */
/*-------------------------------------------------------*/
/* author : visor.bbs@bbs.yzu.edu.tw                     */
/* target : administrator routines                       */
/* create : 2000/01/02                                   */
/* update : NULL                                         */
/*-------------------------------------------------------*/

#undef	_MODES_C_
#include "bbs.h"

extern XZ xz[];

static int show_add();
typedef struct
{
	char email[60];
} LOG;

static void
show_item(num, show)
int num;
LOG *show;
{
	prints("%6d     %s\n", num, show->email);
}

static int
show_body(xo)
XO *xo;
{
	LOG *show;
	int num, max, tail;

	move(3, 0);
	clrtobot();
	max = xo->max;
	if (max <= 0)
	{
		if (vans("�n�s�W��ƶ�(Y/N)�H[N] ") == 'y')
			return show_add(xo);
		return XO_QUIT;
	}

	show = (LOG *) xo_pool;
	num = xo->top;
	tail = num + XO_TALL;
	if (max > tail)
		max = tail;

	do
	{
		show_item(++num, show++);
	}
	while (num < max);

	return XO_NONE;
}


static int
show_head(xo)
XO *xo;
{
	vs_head("�w�벼�W��", str_site);
	outs("\
		 [��]���} ^P)�s�W c)�ק� d)�R�� s)���� [h]elp\n\
		 \033[30;47m  �s��     �w�벼�� Email                                                     \033[m");
	return show_body(xo);
}


static int
show_load(xo)
XO *xo;
{
	xo_load(xo, sizeof(LOG));
	return show_body(xo);
}


static int
show_init(xo)
XO *xo;
{
	xo_load(xo, sizeof(LOG));
	return show_head(xo);
}


static int
show_edit(show, echo)
LOG *show;
int echo;
{
	if (echo == DOECHO)
		memset(show, 0, sizeof(LOG));
	if (vget(b_lines, 0, "E-mail�G", show->email, sizeof(show->email), echo))
		return 1;
	else
		return 0;
}


static int
show_add(xo)
XO *xo;
{
	LOG show;

	if (show_edit(&show, DOECHO))
	{
		rec_add(xo->dir, &show, sizeof(LOG));
		xo->pos = XO_TAIL ;
		xo_load(xo, sizeof(LOG));
	}
	return show_head(xo);
}

static int
show_delete(xo)
XO *xo;
{

	if (vans(msg_del_ny) == 'y')
	{
		if (!rec_del(xo->dir, sizeof(LOG), xo->pos, NULL, NULL))
		{
			return show_load(xo);
		}
	}
	return XO_FOOT;
}


static int
show_change(xo)
XO *xo;
{
	LOG *show, mate;
	int pos, cur;

	pos = xo->pos;
	cur = pos - xo->top;
	show = (LOG *) xo_pool + cur;

	mate = *show;
	show_edit(show, GCARRY);
	if (memcmp(show, &mate, sizeof(LOG)))
	{
		rec_put(xo->dir, show, sizeof(LOG), pos);
		move(3 + cur, 0);
		show_item(++pos, show);
	}

	return XO_FOOT;
}

static int
show_help(xo)
XO *xo;
{
	return XO_NONE;
}


KeyFunc show_cb[] =
{
	{XO_INIT, show_init},
	{XO_LOAD, show_load},
	{XO_HEAD, show_head},
	{XO_BODY, show_body},

	{Ctrl('P'), show_add},
	{'r', show_change},
	{'c', show_change},
	{'s', show_init},
	{'d', show_delete},
	{'h', show_help}
};


int
Showvote(xo)
XO *xo;
{
	VCH *vch;
	char fpath[128], *fname;
	if (!HAS_PERM(PERM_SYSOP))
		return XO_NONE;
	vch = (VCH *) xo_pool + (xo->pos - xo->top);
	hdr_fpath(fpath, xo->dir, (HDR *) vch);
	fname = strrchr(fpath, '@');
	*fname = 'E';

	utmp_mode(M_OMENU);

	xz[XZ_OTHER - XO_ZONE].xo = xo = xo_new(fpath);
	xz[XZ_OTHER - XO_ZONE].cb = show_cb;
	xover(XZ_OTHER);
	free(xo);
	return XO_INIT;
}



