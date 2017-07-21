/*-------------------------------------------------------*/
/* admin.c   ( YZU_CSE WindTop BBS )                     */
/*-------------------------------------------------------*/
/* author : visor.bbs@bbs.yzu.edu.tw                     */
/* target : administrator routines                       */
/* create : 2000/01/02                                   */
/* update : NULL                                         */
/*-------------------------------------------------------*/

#undef	_MODES_C_
#include "bbs.h"

extern XZ xz[];

static int admin_add();


static void
admin_item(num, admin)
int num;
ADMIN *admin;
{
	prints("%6d     %s\n", num, admin->name);
}

static int
admin_body(xo)
XO *xo;
{
	ADMIN *admin;
	int num, max, tail;

	move(3, 0);
	clrtobot();
	max = xo->max;
	if (max <= 0)
	{
		if (vans("�n�s�W��ƶ�(Y/N)�H[N] ") == 'y')
			return admin_add(xo);
		return XO_QUIT;
	}

	admin = (ADMIN *) xo_pool;
	num = xo->top;
	tail = num + XO_TALL;
	if (max > tail)
		max = tail;

	do
	{
		admin_item(++num, admin++);
	}
	while (num < max);

	return XO_NONE;
}


static int
admin_head(xo)
XO *xo;
{
	vs_head("�W�ů���", str_site);
	outs("\
		 [��]���} ^P)�s�W c)�ק� d)�R�� s)���� [h]elp\n\
		 \033[30;47m  �s��     ��  ��  �W  ��                                                     \033[m");
	return admin_body(xo);
}


static int
admin_load(xo)
XO *xo;
{
	xo_load(xo, sizeof(ADMIN));
	return admin_body(xo);
}


static int
admin_init(xo)
XO *xo;
{
	xo_load(xo, sizeof(ADMIN));
	return admin_head(xo);
}


static int
admin_edit(admin, echo)
ADMIN *admin;
int echo;
{
	if (echo == DOECHO)
		memset(admin, 0, sizeof(ADMIN));
	if (vget(b_lines, 0, "�W�ů��ȦC��G", admin->name, sizeof(admin->name), echo))
		return 1;
	else
		return 0;
}


static int
admin_add(xo)
XO *xo;
{
	ADMIN admin;

	if (admin_edit(&admin, DOECHO))
	{
		rec_add(xo->dir, &admin, sizeof(ADMIN));
		xo->pos = XO_TAIL /* xo->max */ ;
		xo_load(xo, sizeof(ADMIN));
	}
	return admin_head(xo);
}

static int
admin_delete(xo)
XO *xo;
{

	if (vans(msg_del_ny) == 'y')
	{
		if (!rec_del(xo->dir, sizeof(ADMIN), xo->pos, NULL, NULL))
		{
			return admin_load(xo);
		}
	}
	return XO_FOOT;
}


static int
admin_change(xo)
XO *xo;
{
	ADMIN *admin, mate;
	int pos, cur;

	pos = xo->pos;
	cur = pos - xo->top;
	admin = (ADMIN *) xo_pool + cur;

	mate = *admin;
	admin_edit(admin, GCARRY);
	if (memcmp(admin, &mate, sizeof(ADMIN)))
	{
		rec_put(xo->dir, admin, sizeof(ADMIN), pos);
		move(3 + cur, 0);
		admin_item(++pos, admin);
	}

	return XO_FOOT;
}

static int
admin_help(xo)
XO *xo;
{
	film_out(FILM_ADMIN, -1);
	return admin_head(xo);
}


KeyFunc admin_cb[] =
{
	{XO_INIT, admin_init},
	{XO_LOAD, admin_load},
	{XO_HEAD, admin_head},
	{XO_BODY, admin_body},

	{Ctrl('P'), admin_add},
	{'r', admin_change},
	{'c', admin_change},
	{'s', admin_init},
	{'d', admin_delete},
	{'h', admin_help}
};


int
Admin()
{
	XO *xo;
	char fpath[64];
	if (!check_admin(cuser.userid) && str_cmp(cuser.userid, SYSOPNAME))
	{
		vmsg("�� �A���O�t�κ޲z���I");
		return 0;
	}

	utmp_mode(M_OMENU);
	sprintf(fpath, FN_ETC_ADMIN_DB);
	xz[XZ_OTHER - XO_ZONE].xo = xo = xo_new(fpath);
	xz[XZ_OTHER - XO_ZONE].cb = admin_cb;
	xover(XZ_OTHER);
	free(xo);
	return 0;
}



