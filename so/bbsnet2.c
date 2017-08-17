/*-------------------------------------------------------*/
/* bbsnet.c   ( YZU_CSE WindTop BBS )                    */
/*-------------------------------------------------------*/
/* author : visor.bbs@bbs.yzu.edu.tw                     */
/* target : bbsnet    			                 */
/* create : 2000/02/05                                   */
/* update : NULL                                         */
/*-------------------------------------------------------*/

#undef	_MODES_C_
#include "bbs.h"

extern XZ xz[];
extern UCACHE *ushm;

static int bbsnet_add();


static void
bbsnet_item(num, bbsnet)
int num;
BBSNET *bbsnet;
{
	prints("%6d %-20.20s %-26.26s %-16.16s %6.6s\n", num, bbsnet->name,
		   bbsnet->host, bbsnet->ip, bbsnet->port);
}

static int
bbsnet_body(xo)
XO *xo;
{
	BBSNET *bbsnet;
	int num, max, tail;

	move(3, 0);
	clrtobot();
	max = xo->max;
	if (max <= 0 && HAS_PERM(PERM_ADMIN))
	{
		if (vans("要新增資料嗎(Y/N)？[N] ") == 'y')
			return bbsnet_add(xo);
		return XO_QUIT;
	}

	bbsnet = (BBSNET *) xo_pool;
	num = xo->top;
	tail = num + XO_TALL;
	if (max > tail)
		max = tail;

	do
	{
		bbsnet_item(++num, bbsnet++);
	}
	while (num < max);

	return XO_NONE;
}


static int
bbsnet_head(xo)
XO *xo;
{
	vs_head("超級站務", str_site);
	outs("\
		 [←]離開 ^P)新增 c)修改 d)刪除 s)重整 [h]elp\n\
		 \033[30;47m  編號 站台名稱             對應 DN                    對應 IP            PORT\033[m");
	return bbsnet_body(xo);
}


static int
bbsnet_load(xo)
XO *xo;
{
	xo_load(xo, sizeof(BBSNET));
	return bbsnet_body(xo);
}


static int
bbsnet_init(xo)
XO *xo;
{
	xo_load(xo, sizeof(BBSNET));
	return bbsnet_head(xo);
}


static int
bbsnet_edit(bbsnet, echo)
BBSNET *bbsnet;
int echo;
{
	if (echo == DOECHO)
		memset(bbsnet, 0, sizeof(BBSNET));
	if (vget(b_lines, 0, "站台名稱：", bbsnet->name, sizeof(bbsnet->name), echo))
	{
		vget(b_lines, 0, "對應 DN ：", bbsnet->host, sizeof(bbsnet->host), echo);
		vget(b_lines, 0, "對應 IP ：", bbsnet->ip, sizeof(bbsnet->ip), echo);
		vget(b_lines, 0, "連接阜：", bbsnet->port, sizeof(bbsnet->port), echo);
		return 1;
	}
	else
		return 0;
}


static int
bbsnet_add(xo)
XO *xo;
{
	BBSNET bbsnet;
	if (!HAS_PERM(PERM_ADMIN))
		return XO_NONE;

	if (bbsnet_edit(&bbsnet, DOECHO))
	{
		logitfile(FN_BBSNET_LOG, "< ADDNEW >", bbsnet.name);
		rec_add(xo->dir, &bbsnet, sizeof(BBSNET));
		xo->pos = XO_TAIL ;
		xo_load(xo, sizeof(BBSNET));
	}
	return bbsnet_head(xo);
}

static int
bbsnet_delete(xo)
XO *xo;
{

	BBSNET *bbsnet;
	int pos, cur;

	if (!HAS_PERM(PERM_ADMIN))
		return XO_NONE;

	pos = xo->pos;
	cur = pos - xo->top;
	bbsnet = (BBSNET *) xo_pool + cur;

	if (vans(msg_del_ny) == 'y')
	{
		logitfile(FN_BBSNET_LOG, "< DELETE >", bbsnet->name);
		if (!rec_del(xo->dir, sizeof(BBSNET), xo->pos, NULL, NULL))
		{
			return bbsnet_load(xo);
		}
	}
	return XO_FOOT;
}


static int
bbsnet_change(xo)
XO *xo;
{
	BBSNET *bbsnet, mate;
	int pos, cur;
	if (!HAS_PERM(PERM_ADMIN))
		return XO_NONE;

	pos = xo->pos;
	cur = pos - xo->top;
	bbsnet = (BBSNET *) xo_pool + cur;

	mate = *bbsnet;
	bbsnet_edit(bbsnet, GCARRY);
	if (memcmp(bbsnet, &mate, sizeof(BBSNET)))
	{
		logitfile(FN_BBSNET_LOG, "< CHANGE >", bbsnet->name);
		rec_put(xo->dir, bbsnet, sizeof(BBSNET), pos);
		move(3 + cur, 0);
		bbsnet_item(++pos, bbsnet);
	}

	return XO_FOOT;
}

static int
bbsnet_telnet(xo)
XO *xo;
{
	UTMP *up, *uceil;
	BBSNET *bbsnet;
	int pos, cur, fd;
	int total;
	char buf[80];

	up = ushm->uslot;
	uceil = (void *) up + ushm->offset;
	total = 0;

	do
	{
		if (up->userno <= 0 || up->pid <= 0)
			continue;

		if (up->ufo & UFO_REJECT)
			total++;
	}
	while (++up <= uceil);

	if (total > BBSNETMAX)
	{
		vmsg("有太多人使用 BBSNET 中，請稍後再來...");
		return XO_FOOT;
	}

	pos = xo->pos;
	cur = pos - xo->top;
	bbsnet = (BBSNET *) xo_pool + cur;
	if (strlen(bbsnet->host) > 0)
		sprintf(buf, "telnet %s %s", bbsnet->host, strlen(bbsnet->port) ? bbsnet->port : "");
	else if (strlen(bbsnet->ip) > 0)
		sprintf(buf, "telnet %s %s", bbsnet->ip, strlen(bbsnet->port) ? bbsnet->port : "");
	else
	{
		vmsg("格式錯誤");
		return XO_FOOT;
	}

	vmsg("光波傳送中...");
	fd = dup(0);
	if (fd > 0)
	{
		logitfile(FN_BBSNET_LOG, "< TELNET >", bbsnet->name);
		cutmp->ufo |= UFO_REJECT;
		clear();
		move(0, 0);
		outs("正連入主機...\n");
		system(buf);
		close(fd);
		cutmp->ufo &= ~UFO_REJECT;
	}
	else
		vmsg("登入主機失敗...");

	return XO_HEAD;
}

static int
bbsnet_help(xo)
XO *xo;
{
//  film_out(FILM_BBSNET, -1);
	return bbsnet_head(xo);
}


KeyFunc bbsnet_cb[] =
{
	XO_INIT, bbsnet_init,
	XO_LOAD, bbsnet_load,
	XO_HEAD, bbsnet_head,
	XO_BODY, bbsnet_body,

	Ctrl('P'), bbsnet_add,
	'r', bbsnet_telnet,
	'c', bbsnet_change,
	's', bbsnet_init,
	'd', bbsnet_delete,
	'h', bbsnet_help
};


int
Bbsnet()
{
	XO *xo;
	char fpath[64];

	utmp_mode(M_OMENU);
	sprintf(fpath, FN_GAME_BBSNET);
	xz[XZ_OTHER - XO_ZONE].xo = xo = xo_new(fpath);
	xz[XZ_OTHER - XO_ZONE].cb = bbsnet_cb;
	xover(XZ_OTHER);
	free(xo);
	return 0;
}



