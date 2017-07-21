/*-------------------------------------------------------*/
/* brdstat.c   ( YZU_CSE WindTop BBS )                   */
/*-------------------------------------------------------*/
/* author : visor.bbs@bbs.yzu.edu.tw                     */
/* target : board status                                 */
/* create : 2003/08/17                                   */
/* update : NULL                                         */
/*-------------------------------------------------------*/

#undef	_MODES_C_
#include "bbs.h"

#ifdef  HAVE_COUNT_BOARD

extern void outs(uschar *str);

extern XZ xz[];
extern BCACHE *bshm;

static void
bstat_item(num, bstat)
int num;
BSTAT *bstat;
{
	prints("%6d   %-8.8s    %6d    %6d    %6d    %6d\n", num,
		   bstat->type, bstat->n_reads, bstat->n_posts, bstat->n_news, bstat->n_bans);
}

static int
bstat_body(xo)
XO *xo;
{
	BSTAT *bstat;
	int num, max, tail;

	move(3, 0);
	clrtobot();
	max = xo->max;
	if (max <= 0)
	{
		vmsg("無任何統計資料");
		return XO_QUIT;
	}

	bstat = (BSTAT *) xo_pool;
	num = xo->top;
	tail = num + XO_TALL;
	if (max > tail)
		max = tail;

	do
	{
		bstat_item(++num, bstat++);
	}
	while (num < max);

	return XO_NONE;
}


static int
bstat_head(xo)
XO *xo;
{
	BRD *brd;
	int chn;
	char buf[80];


	chn = xo->key;
	brd = bshm->bcache + chn;

	sprintf(buf, "看板使用資訊：%s", brd->brdname);

	vs_head(buf, str_site);
	outs("  [←]離開 s)重整 S)目前資訊 [h]elp\n\033[30;47m  編號   型    態  閱\讀次數  發文次數  轉信次數  擋信次數                     \033[m");
	return bstat_body(xo);
}


static int
bstat_load(xo)
XO *xo;
{
	xo_load(xo, sizeof(BSTAT));
	return bstat_body(xo);
}


static int
bstat_init(xo)
XO *xo;
{
	xo_load(xo, sizeof(BSTAT));
	return bstat_head(xo);
}


static int
bstat_stat(xo)
XO *xo;
{
	BRD *brd;
	int chn;
	char msg[80];

	chn = xo->key;
	if (chn >= 0)
	{
		brd = bshm->bcache + chn;
		sprintf(msg, "累積閱\讀：%d，發文：%d，轉信：%d，擋信：%d", brd->n_reads, brd->n_posts, brd->n_news, brd->n_bans);
		pmsg(msg);
	}

	return XO_NONE;
}

static int
bstat_clear(xo)
XO *xo;
{
	BRD *brd;
	int chn;
	char fpath[128];

	if (!HAS_PERM(PERM_ALLBOARD))
		return XO_NONE;

	if (vans("確定清除所有紀錄嗎？[y/N]") == 'y')
	{

		chn = xo->key;
		if (chn >= 0)
		{
			brd = bshm->bcache + chn;
			brd_fpath(fpath, brd->brdname, FN_BRD_STATCOUNT);
			unlink(fpath);
			brd_fpath(fpath, brd->brdname, FN_BRD_STAT);
			unlink(fpath);
		}
	}

	return XO_INIT;
}

static int
bstat_help(xo)
XO *xo;
{
	//film_out(FILM_BSTAT, -1);
	return bstat_head(xo);
}


KeyFunc bstat_cb[] =
{
	{XO_INIT, bstat_init},
	{XO_LOAD, bstat_load},
	{XO_HEAD, bstat_head},
	{XO_BODY, bstat_body},

	{'s', bstat_init},
	{'S', bstat_stat},
	{'c', bstat_clear},
	{'h', bstat_help}
};


int
main_bstat(xo)
XO *xo;
{
	XO *xx;
	char fpath[64];
	BRD *brd;
	short *chp;
	int num, chn;


	num = xo->pos;
	chp = (short *) xo->xyz + num;
	chn = *chp;
	if (chn >= 0)
	{
		brd = bshm->bcache + chn;
		utmp_mode(M_OMENU);
		brd_fpath(fpath, brd->brdname, FN_BRD_STATCOUNT);
		xz[XZ_OTHER - XO_ZONE].xo = xx = xo_new(fpath);
		xz[XZ_OTHER - XO_ZONE].cb = bstat_cb;
		xx->key = chn;
		xover(XZ_OTHER);
		free(xx);
	}

	return XO_HEAD;
}



#endif
