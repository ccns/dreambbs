/*-------------------------------------------------------*/
/* sec_hand.c   ( NTHU CS MapleBBS Ver 3.10 )            */
/*-------------------------------------------------------*/
/* author : ernie.bbs@bbs.ee.nthu.edu.tw                 */
/* target : A second hand market                         */
/* create : 00/04/03                                     */
/* update :                                              */
/*-------------------------------------------------------*/

#include "bbs.h"
#undef	DEBUG


// SLOT propeties, put them in struct.h
#define	PROP_EMPTY		0x0000	// slot is empty, available
#define	PROP_G_GROUP		0x0001
#define	PROP_G_CANCEL		0x0002
#define	PROP_G_OPEN		0x0004
#define PROP_I_SELL		0x0010
#define	PROP_I_WANT		0x0020
#define	PROP_I_CANCEL		0x0040	// if set, another prog will expire
// this item.
#define	PROP_IS_GROUP		(PROP_G_GROUP | PROP_G_CANCEL)
#define	PROP_IS_ITEM		(PROP_I_SELL | PROP_I_WANT | PROP_I_CANCEL)
#define PROP_IS_CANCEL		(PROP_G_CANCEL | PROP_I_CANCEL)

typedef struct SLOT		// if is group, only prop/reply/title/fn
{
	// will be used
	time_t chrono;		// time stamp
	int prop;			// propety of this slot
	int reply;			// number of replied mail of this item
	char title[30];
	char userid[IDLEN + 1];	// userid for mail reply and owner del check
	char price[10];
	char contact[20];
	char date[6];			// only 6 bytes, not 9
	char fn[9];			// dirname or filename of item desc, max9 bytes
} SLOT;				// 80 bytes totally


typedef struct SLOT_DOUBLE_LINKLIST
{
	struct SLOT_DOUBLE_LINKLIST *prev;
	SLOT slot;
	struct SLOT_DOUBLE_LINKLIST *next;
} SDL;

//------------------------------------------------------------------------

// filenames, put them in bbs.h
#define FN_GRP			".GRP"
#define FN_ITEM			".ITEM"
#define FN_2ND_DECL		"2ndhand_decl"
#define	FN_2ND_HELP		"2ndhand_help"

//------------------------------------------------------------------------

#undef	HAVE_RESTRICTION	// restrict posting according to cuser.vmail
#ifdef HAVE_RESTRICTION
#define MSG_RES		"對不起，本功\能只限成大同學使用，您的認證信箱不在其內"
#endif

#define MSG_2ND		" \033[0;34;46m 二手市場 \033[31;47m (ESC/←)\033[30m\
	離開  \033[31m(H)\033[30m操作說明\
	\033[0m"
#define SPACE		"                                                  "

SDL *sdl_head;
SDL *top, *bot;
SDL *curr_slot, *curr_grp, *last_grp, *curr_item;
uschar f_size;
int sdl_add(int fd, SDL **where);


#ifdef DEBUG
void
show(str, no)
int no;
char *str;
{
	char msg[128];

	move(b_lines - 1, 0);
	clrtoeol();
	sprintf(msg, "%s : %d", str, no);
	outs(msg);
	vkey();
}

void
outt(str, str1, str2, str3)
char *str, *str1, *str2, *str3;
{
	char buf[256];
	move(b_lines - 1, 0);
	clrtoeol();
	sprintf(buf, "%s::%s::%s::%s", str, str1, str2, str3);
	outs(buf);
}

void
sdl_out(str)				// for debugging
char *str;
{
	FILE *fp;
	char buf[256];
	SDL *x;
	SLOT *s;

	x = sdl_head;
	fp = fopen("src/maple/dump", "a");
	fprintf(fp, "%s\n", str);
	while (x)
	{
		s = &(x->slot);
		sprintf(buf, "%s%d 0x%02X %02d %20s %12s %6s %s\n",
				(s->prop & PROP_IS_ITEM) ? "===" : "",
				s->chrono , s->prop, s->reply,
				s->title, s->userid, s->date, s->fn);
		fprintf(fp, "%s", buf);
		x = x->next;
	}
	fprintf(fp, "--------------------------------------------------------\n");
	fclose(fp);
	return;
}
#endif

//------------------------------------------------------------------------

#define	NEXT			1
#define	PREV			0

SDL *
ptr_move(x, dir, n)
SDL *x;
int dir, n;
{
	int i = 0;

	switch (dir)
	{
	case NEXT:
		for (i = 1; i <= n; i++)
		{
			if (x->next)
				x = x->next;
			else
				return x;
		}
		break;

	case PREV:
		for (i = 1; i <= n; i++)
		{
			if (x->prev)
				x = x->prev;
			else
				return x;
		}
		break;
	}
	return x;
}

SDL *
seek_curr_grp()
{
	SDL *x;

	if (curr_slot->slot.prop & PROP_IS_GROUP)
		return curr_slot;

	x = curr_slot;
	while (x->prev && (x->prev)->slot.prop & PROP_IS_ITEM)
		x = x->prev;

	return x->prev;
}

void
set_frame(x)
SDL *x;
{
	bot = top = x;
	bot = ptr_move(bot, NEXT, f_size - 1);
	return;
}

void
clrhalf()
{
	move(f_size + 2, 0);
	clrtobot();
	outz(MSG_2ND);
	return;
}

void
draw_frame()
{
	clear();
	outs(" \033[0;36m─\033[1;33;46m DreamBBS  Market \033[0;36m──────\
		 ──────────────────────\033[0m");
	move(f_size + 1, 0);
	outs(" \033[0;36m────────────────────────────\
		 ──────────\033[0m");
	return;
}

void
more_it(title, fname)
char *title, *fname;
{
	int i, j;
	char buf[256];
	FILE *fp;

	clrhalf();
	move(f_size + 2, 0);
	prints("  \033[1;37;46m %s \033[0m\n\n", title);
	if ((fp = fopen(fname, "r")))
	{
		i = 0;
		j = 19 - f_size;
		while (fgets(buf, sizeof(buf), fp) && (++i <= j))
			outs(buf);
		fclose(fp);
	}
	curr_item = NULL;

	return;
}


void
do_draw(x, row, is_bar)
SDL *x;
int row, is_bar;
{
	SLOT *s;
	char buf[256];

	move(row, 0);
	s = &(x->slot);
	if (s->prop & PROP_IS_GROUP)
	{
		sprintf(buf, "%s%s %s (%d) \033[0m",
				s->reply ?
				((s->prop & PROP_G_OPEN) ? "[-]" : "[+]") : "   ",
						is_bar ? "\033[1;37;41m" : "",
						s->title, s->reply);
	}
	else
{
		sprintf(buf, "    %s%s%s (%2d) %s%s",
				x->next ?
				(((x->next)->slot.prop & PROP_IS_ITEM) ? "├" : "└") : ("└"),
						is_bar ? "\033[1;37;41m" : "\033[0;37;40m",
						s->date, s->reply, s->title, SPACE);
		sprintf(&buf[73], " %12s \033[0m", s->userid);
	}

	outs(buf);
	clrtoeol();
	return;
}

#define	BAR_YES			1
#define	BAR_NO			0
#define	DRAW_BAR_UP		-1
#define	DRAW_ALL		0
#define	DRAW_BAR_DOWN		1
#define	DRAW_TO_BOT		2

void
draw(where, how)
SDL *where;
int how;
{
	int row, flag;
	SDL *x;

	if (!f_size)
		return;

	x = top;
	row = 1;
	flag = 0;

	while (x)
	{
		if (x == curr_slot)
		{
			do_draw(x, row, BAR_YES);
			flag = 1;
		}
		else if ((how == DRAW_ALL) ||
				 ((how == DRAW_TO_BOT) && (flag == 1)) ||
				 ((how == DRAW_BAR_UP) && (x == curr_slot->next)) ||
				 ((how == DRAW_BAR_DOWN) && (x == curr_slot->prev)))
		{
			do_draw(x, row, BAR_NO);
		}
		row++;
		if (x == bot)
			break;
		else
			x = x->next;
	}

	while (row <= f_size)
	{
		move(row++, 0);
		clrtoeol();
	}
	return;
}

void
read_in_rec(fname, where)
char *fname;
SDL *where;
{
	int fd;
	SDL *x;

	fd = open(fname, O_RDONLY);
	if (fd > 0)
	{
		x = where;
		while (sdl_add(fd, &x));
		close(fd);
	}

	return;
}

void
open_grp(grp)
SDL *grp;
{
	char fname[80];

	if ((grp->slot.prop & PROP_G_OPEN) ||
		(grp->slot.prop & PROP_IS_CANCEL))
		return;

	sprintf(fname, "2nd/%s/%s", grp->slot.fn, FN_ITEM);
	read_in_rec(fname, grp);
	grp->slot.prop |= PROP_G_OPEN;
	return;
}

SDL *close_grp(grp)
SDL *grp;
{
	SDL *this, *next;

	if (!(grp->slot.prop & PROP_G_OPEN))
		return grp;

	this = grp->next;
	while (this && (this->slot.prop & PROP_IS_ITEM))
	{
		next = this->next;
		free(this);
		this = next;
	}

	grp->next = this;
	if (this)			// "this" could be NULL,
		this->prev = grp;		// while grp is the last group

	grp->slot.prop ^= PROP_G_OPEN;
	return grp;
}


void
sdl_new()                  // modified from mail.c  :p
{
	SDL *list, *next;

	list = sdl_head;

	while (list)
	{
		next = list->next;
		free(list);
		list = next;
	}

	sdl_head = NULL;
}

int
sdl_add(fd, where)
int fd;
SDL **where;
{
	SDL *x;

	x = (SDL *) malloc(sizeof(SDL));
	if (read(fd, &(x->slot), sizeof(SLOT)) != sizeof(SLOT))
	{
		free(x);
		return 0;
	}

	if (x->slot.prop & PROP_IS_CANCEL)
	{
		free(x);
		return 1;
	}

	x->prev = x->next = NULL;

	if (*where)
	{
		SDL *tmp;

		tmp = (*where)->next;
		(*where)->next = x;
		x->prev = *where;
		if (tmp)
		{
			tmp->prev = x;
			x->next = tmp;
		}
	}
	else
	{
		sdl_head = x;
	}

	*where = x;
	return 1;
}

void
func_i(a, b)
SLOT *a, *b;
{
	a->prop |= PROP_I_CANCEL;
	sprintf(a->title, "<< 本佈告經 %s 刪除 >>", cuser.userid);
	b->prop |= PROP_I_CANCEL;
	sprintf(b->title, "<< 本佈告經 %s 刪除 >>", cuser.userid);
	return;
}

void
func_g(a, b)
SLOT *a, *b;
{
	a->prop |= PROP_G_CANCEL;
	sprintf(a->title, "<< 本群組經 %s 刪除 >>", cuser.userid);
	b->prop |= PROP_G_CANCEL;
	sprintf(b->title, "<< 本群組經 %s 刪除 >>", cuser.userid);
	return;
}

void
func_inc(a, b)
SLOT *a, *b;
{
	a->reply++, b->reply++;
	return;
}

void
func_dec(a, b)
SLOT *a, *b;
{
	a->reply--, b->reply--;
	return;
}

int
slot_change(fpath, s, func_ptr)
char *fpath;
SLOT *s;
void *func_ptr();
{
	int fd, pos;
	SLOT slot;

	if ((fd = open(fpath, O_RDONLY)) < 0)
		return 0;

	pos = 0;
	while (read(fd, &slot, sizeof(SLOT)) == sizeof(SLOT))
	{
		if (s->chrono == slot.chrono)	// hope there's no 2 same chronos
		{
			// of groups
			func_ptr(s, &slot);
			close(fd);

			rec_put(fpath, &slot, sizeof(SLOT), pos);
			return 1;
		}
		pos++;
	}

	close(fd);
	return 0;
}

int
sdl_del()
{
	char fpath[80];
	SDL *x;

	if (curr_slot->slot.prop & PROP_IS_CANCEL)
		return 0;

	if ((cuser.userlevel < PERM_BOARD) &&
		strcmp(curr_slot->slot.userid, cuser.userid))
		return 0;

	sprintf(fpath, " 確定刪除：%s？[N] ", curr_slot->slot.title);
	if (vans(fpath) != 'y')
		return 0;

	x = curr_slot;
	if (x->slot.prop & PROP_IS_ITEM)
	{
		sprintf(fpath, "2nd/%s/%s", curr_grp->slot.fn, FN_ITEM);
		slot_change(fpath, &x->slot, func_i);
		slot_change("2nd/"FN_GRP, &curr_grp->slot, func_dec);
	}
	else if (x->slot.prop & PROP_IS_GROUP)
	{
		if (vans(" 您將刪除的是整個分類群組，真的要繼續？[N] ") != 'y')
			return 0;
		sprintf(fpath, "2nd/%s", FN_GRP);
		slot_change(fpath, &x->slot, func_g);
		close_grp(x);
	}

	return 1;
}


void
short_stamp(str, chrono)
char *str;
time_t *chrono;
{
	struct tm *ptime;

	ptime = localtime(chrono);
	sprintf(str, "%2d/%02d", ptime->tm_mon + 1, ptime->tm_mday);
}

int
slot_stamp(folder, slot, fpath)	// modified from hdr_stamp()
char *folder;
SLOT *slot;
char *fpath;
{
	char *fname, *family = NULL;
	char *flink, buf[128];
	int rc;
	time_t t;

	flink = fpath;
	fpath = buf;
	fname = fpath;
	while ((rc = *folder++))
	{
		*fname++ = rc;
		if (rc == '/')
			family = fname;
	}

	fname = family + 1;
	*fname++ = '/';
	*fname++ = 'A';

	t = time(0);

	for (;;)
	{
		*family = radix32[t & 31];
		archiv32(t, fname);
		rc = f_ln(flink, fpath);

		if (rc >= 0)
		{
			memset(slot, 0, sizeof(slot));
			slot->chrono = t;
			strcpy(slot->fn, --fname);
			short_stamp(slot->date, &(slot->chrono));
			break;
		}

		if (errno != EEXIST)
			break;

		t++;
	}
	return rc;
}


void
more_item()
{
	int i, j;
	char fname[80], buf[256];
	SLOT *s;
	FILE *fp;

	if (!curr_item)
		return;

	s = &(curr_item->slot);
	sprintf(fname, "2nd/%s/%c/%s", curr_grp->slot.fn, s->fn[7], s->fn);
	sprintf(buf, "  \033[1;37;46m %s \033[0m %s  %s\n\n",
			curr_grp->slot.title, s->title,
			(s->prop & PROP_I_WANT) ? "\033[1;5;31m(WANTED)\033[0m" : "");

	clrhalf();
	move(f_size + 2, 0);
	outs(buf);
	if ((fp = fopen(fname, "r")))
	{
		i = 0;
		j = 19 - f_size;
		while (fgets(buf, sizeof(buf), fp) && ++i <= j)
			outs(buf);
		fclose(fp);
	}
	move(20, 0);
	sprintf(buf, "  ◆ 希望價格    ：%s\n", s->price);
	outs(buf);
	sprintf(buf, "  ◆ 請回信給我  ：%s\n", s->userid);
	outs(buf);
	sprintf(buf, "  ◆ 其他聯絡方式：%s", s->contact);
	outs(buf);
	return;
}

#ifdef HAVE_RESTRICTION
int
can_post()
{
	int i;
	char *table[2] = {".ncku.edu.tw", NULL};

	i = 0;
	while (table[i])
	{
		if (str_str(cuser.vmail, table[i]))
			return 1;
		i++;
	}
	return 0;
}
#endif

int
new_item()
{
	struct stat xx;
	char ans[3];
	char fpath[80];

	if (bbsothermode & OTHERSTAT_EDITING)
	{
		vmsg("你還有檔案還沒編完哦！");
		return -1;
	}


	// return -1: cancel posting, redraw frame
	//         0: do nothing, frame remains untouched
	//         1: creat a new group, redraw frame
	//         2: creat a new item, redraw frame

	if (cuser.userlevel >= PERM_BOARD)
		vget(b_lines, 0, " 我要 (S)賣東西 (W)徵求東西 (N)新增群組 (Q)取消？[Q] ",
			 ans, 2, LCECHO);
#ifdef HAVE_RESTRICTION
	else if (can_post())
		vget(b_lines, 0, " 我要 (S)賣東西 (W)徵求東西 (Q)取消？[Q] ",
			 ans, 2, LCECHO);
	else
	{
		vmsg(MSG_RES);
		return 0;
	}
#else
	else
		vget(b_lines, 0, " 我要 (S)賣東西 (W)徵求東西 (Q)取消？[Q] ",
			 ans, 2, LCECHO);
#endif

	if (ans[0] == 'n' || ans[0] == 'N')
	{
		// a new group will appear at the next
		// time entering second hand market

		SLOT grp;

		if (cuser.userlevel < PERM_BOARD)
			return 0;

		clrhalf();
		move(f_size + 3, 4);
		outs("\033[1;33m◆ 新增群組\033[0m");

		do
		{
			vget(f_size + 5, 0, "    群組的英文目錄名稱：", grp.fn, 9, DOECHO);
			sprintf(fpath, "2nd/%s", grp.fn);
		}
		while (stat(fpath, &xx) == 0);	// loop when dir exists
		vget(f_size + 7, 0, "    群組標題：",
			 grp.title, sizeof(grp.title), DOECHO);
		vget(f_size + 9, 0, "    請您確定？[N] ", ans, 2, LCECHO);

		if (ans[0] == 'y' || ans[0] == 'Y')
		{
			char dir[80];

			grp.chrono = time(NULL);
			grp.prop = PROP_G_GROUP;
			grp.reply = 0;			// num of items contain in this grp
			strcpy(grp.userid, cuser.userid);
			short_stamp(grp.date, &grp.chrono);
			rec_add("2nd/"FN_GRP, &grp, sizeof(SLOT));

			sprintf(dir, "2nd/%s", grp.fn);
			mak_dirs(dir);

			return 1;
		}
	}
	else if (ans[0] == 'w' || ans[0] == 'w' || ans[0] == 's' || ans[0] == 'S')
	{
		// a new item will appear at the
		// next time opening the group

		SLOT item;
		SDL *x;
		char title[30], fpath[80], folder[80];

		if (!sdl_head)
		{
			outz(" 尚未建立群組，請先建立分類群組");
			return 0;
		}

		if (curr_grp->slot.prop & PROP_G_CANCEL)
			return 0;

//    outz(" 記得先把光棒移動至你想要的分類群組! ^_^");
//    vkey();

		sprintf(fpath, "在「%s」貼佈告，標題：", curr_grp->slot.title);
		sprintf(folder, "2nd/%s/"FN_ITEM, curr_grp->slot.fn);

		if (!vget(b_lines, 0, fpath, title, sizeof(title), DOECHO))
			return 0;

		fpath[0] = 0;
		if (vedit(fpath, NA) < 0)
		{
			unlink(fpath);
			vmsg("取消");
			return -1;
		}

		slot_stamp(folder, &item, fpath);

		vget(3, 0, " 您所希望的價格？ ", item.price, sizeof(item.price), DOECHO);
		vget(5, 0, " 除回信外的其他聯絡方式？ ",
			 item.contact, sizeof(item.contact), DOECHO);

		if (ans[0] == 'w' || ans[0] == 'W')
			item.prop = PROP_I_WANT;
		else
			item.prop = PROP_I_SELL;
		item.reply = 0;
		strcpy(item.userid, cuser.userid);
		strcpy(item.title, title);

		rec_ins(folder, &item, sizeof(SLOT), 0, 1);	// the newer the upper
		slot_change("2nd/"FN_GRP, &curr_grp->slot, func_inc);
		unlink(fpath);

		x = curr_grp;
		close_grp(last_grp);
		open_grp(curr_grp);
		last_grp = curr_grp;
		curr_item = curr_slot = curr_grp->next;

		top = ptr_move(curr_slot, PREV, 3);
		set_frame(top);

		return 2;
	}

	return 0;
}



void
reply_item()
{
	HDR hdr;
	SLOT *s;

	if (!curr_item || (curr_item->slot.prop & PROP_I_CANCEL))
		return;

#ifdef HAVE_RESTRICTION
	if (!can_post())
	{
		vmsg(MSG_RES);
		return;
	}
#endif

	if (vans(" 回應給作者？[N] ") != 'y')
		return;

	s = &(curr_item->slot);
	sprintf(quote_file, "2nd/%s/%c/%s", curr_grp->slot.fn, s->fn[7], s->fn);
	memset(&hdr, 0, sizeof(HDR));
	hdr.chrono = s->chrono;
	strcpy(hdr.owner, s->userid);
	strcpy(hdr.title, s->title);
	str_stamp(hdr.date, &hdr.chrono);

	mail_reply(&hdr);

	sprintf(quote_file, "2nd/%s/%s", curr_grp->slot.fn, FN_ITEM);
	slot_change(quote_file, s, func_inc);


	*quote_file = '\0';

	draw_frame();
	draw(top, DRAW_ALL);
	more_item();

	return;
}

void
bar_up()
{
	if (curr_slot->prev)
	{
		SDL *prev = curr_slot->prev;

		curr_slot = prev;
		if ((curr_slot->next)->slot.prop & PROP_IS_GROUP)
		{
			if (curr_slot->slot.prop & PROP_IS_GROUP)
				curr_grp = prev;
			else
				curr_grp = seek_curr_grp();
		}

		curr_slot = prev;
		if (curr_slot == top->prev)
		{
			set_frame(top->prev);
			draw(top, DRAW_ALL);
		}
		else
			draw(curr_slot, DRAW_BAR_UP);
	}
}

void
bar_down()
{
	if (curr_slot->next)
	{
		SDL *next = curr_slot->next;

		curr_slot = next;
		if (next->slot.prop & PROP_IS_GROUP)
			curr_grp = next;

		if (curr_slot == bot->next)
		{
			set_frame(top->next);
			draw(top, DRAW_ALL);
		}
		else
			draw(curr_slot, DRAW_BAR_DOWN);
	}
}

void
bar_right()
{
	if (curr_slot->slot.prop & PROP_IS_CANCEL)
		return;

	if (curr_slot->slot.prop & PROP_IS_GROUP)
	{
		// here, curr_slot == curr_grp
		if (curr_grp->slot.prop & PROP_G_OPEN)
		{
			curr_slot = close_grp(curr_grp);
			last_grp = curr_grp;
			set_frame(top);
		}
		else if (curr_grp->slot.reply)
		{
			close_grp(last_grp);
			open_grp(curr_grp);
			last_grp = curr_grp;
			set_frame(curr_grp);
		}
		draw(top, DRAW_TO_BOT);
	}
	else if (curr_slot->slot.prop & PROP_IS_ITEM)
	{
		curr_item = curr_slot;
		more_item();
	}
}

void
bar_pgup()
{
	curr_slot = ptr_move(curr_slot, PREV, f_size - 1);
	if (curr_slot->slot.prop & PROP_IS_GROUP)
		curr_grp = curr_slot;
	curr_grp = seek_curr_grp();

	top = ptr_move(top, PREV, f_size - 1);
	set_frame(top);
	draw(top, DRAW_ALL);
}

void
bar_pgdn()
{
	curr_slot = ptr_move(curr_slot, NEXT, f_size - 1);
	if (curr_slot->slot.prop & PROP_IS_GROUP)
		curr_grp = curr_slot;
	else
		curr_grp = seek_curr_grp();

	if (bot->next)
		set_frame(bot);
	draw(top, DRAW_ALL);
}

void
bar_home()
{
	curr_grp = curr_slot = sdl_head;
	set_frame(sdl_head);
	draw(top, DRAW_ALL);
}

void
bar_end()
{
	while (curr_slot->next)
		curr_slot = curr_slot->next;
	curr_grp = seek_curr_grp();
	top = ptr_move(curr_slot, PREV, 6);
	set_frame(top);
	draw(top, DRAW_ALL);
}

int
sec_hand()
{
	int key;

	utmp_mode(M_XMODE);
	f_size = 7;
	draw_frame();
	more_it("公告事項", "etc/"FN_2ND_DECL);

	sdl_new();
	while (!sdl_head)
	{
		read_in_rec("2nd/"FN_GRP, sdl_head);

		if (!sdl_head)
		{
			if (new_item() == 1)
				continue;
			else
				return 0;
		}
	}
	curr_slot = curr_grp = sdl_head;
	last_grp = sdl_head;
	curr_item = NULL;

	set_frame(sdl_head);
	draw(top, DRAW_ALL);

	while (1)
	{
		key = vkey();

		switch (key)
		{
		case 'q':
		case 'e':
		case KEY_LEFT:
		case KEY_ESC:
			goto exit;

		case '+':
			(f_size <= 15) ? ++f_size : f_size;
			goto REFRESH;
		case '-':
			f_size ? --f_size : f_size;
REFRESH:
			set_frame(top);
			draw_frame();
			draw(top, DRAW_ALL);
			clrhalf();
			more_item();
			break;

			// keys of moving bar:
			//   1. set curr_slot first
			//   2. then set curr_grp
			//   3. set top and bot
			//   4. redraw screen

		case KEY_UP:	if (f_size) bar_up();	break;
		case KEY_DOWN:	if (f_size) bar_down();	break;
		case KEY_PGDN:
		case ' ':		if (f_size) bar_pgdn();	break;
		case KEY_PGUP:	if (f_size) bar_pgup();	break;
		case KEY_HOME:
		case '0':		if (f_size) bar_home();	break;
		case KEY_END:
		case '$':		if (f_size) bar_end();	break;
		case KEY_RIGHT:
		case '\n':	bar_right();	break;


		case 'y':
		case 'r':
		case 'm':
			reply_item();
			outz(MSG_2ND);
			break;

		case 'd':
			if (sdl_del())
				draw(top, DRAW_ALL);
			outz(MSG_2ND);
			break;

		case Ctrl('P'):
						if (new_item())
				{
					draw_frame();
					draw(top, DRAW_ALL);
					more_item();
				}
			outz(MSG_2ND);
			break;

		case 'h':
		case 'H':
			more_it("操作說明", "etc/"FN_2ND_HELP);
			break;

		case Ctrl('D'):
						more_it("公告事項", "etc/"FN_2ND_DECL);
			break;

#ifdef DEBUG
		case Ctrl('S'):
			{
				char buf[4];
				sprintf(buf, "%d", f_size);
				outt(curr_grp->slot.title, curr_slot->slot.title, buf, "0");
			}
			break;

		case Ctrl('Q'):
						sdl_out("Ctrl-Q");
			break;
#endif
		}
	}

	exit:
	utmp_mode(M_MMENU);
	sdl_new();
	return 0;
}

