/*-------------------------------------------------------*/
/* contact.c   ( YZU_CSE Train BBS )                     */
/*-------------------------------------------------------*/
/* author : Jerics.bbs@bbs.yzu.edu.tw                    */
/* target : contacts list                                */
/* create : 2000/01/12                                   */
/* update : NULL                                         */
/*-------------------------------------------------------*/

#undef	_MODES_C_
#include "bbs.h"

extern XZ xz[];

static int contact_add(XO *xo);
void contact_send(CONTACT *contact);

static void
contact_item(
int num,
CONTACT *contact)
{
	prints("%6d     %-13s      %-48s\n", num, contact->name, contact->email);
}

static int
contact_body(
XO *xo)
{
	CONTACT *contact;
	int num, max, tail;

	move(3, 0);
	clrtobot();
	max = xo->max;
	if (max <= 0)
	{
		if (vans("要新增資料嗎(Y/N)？[N] ") == 'y')
			return contact_add(xo);
		return XO_QUIT;
	}

	contact = (CONTACT *) xo_pool;
	num = xo->top;
	tail = num + XO_TALL;
	if (max > tail)
		max = tail;

	do
	{
		contact_item(++num, contact++);
	}
	while (num < max);

	return XO_NONE;
}


static int
contact_head(
XO *xo)
{
	vs_head("聯絡名單", str_site);
	outs("\
		 [←]離開 ^P)新增 c)修改 d)刪除 s)重整 m)寄信 [h]elp\n\
		 \033[30;47m  編號     聯  絡  名  單     e-mail address                                  \033[m");
	return contact_body(xo);
}


static int
contact_load(
XO *xo)
{
	xo_load(xo, sizeof(CONTACT));
	return contact_body(xo);
}


static int
contact_init(
XO *xo)
{
	xo_load(xo, sizeof(CONTACT));
	return contact_head(xo);
}


static int
contact_edit(
CONTACT *contact,
int echo)
{
	if (echo == DOECHO)
		memset(contact, 0, sizeof(CONTACT));
	if (vget(b_lines, 0, "名稱：", contact->name, sizeof(contact->name), echo)
		&& vget(b_lines, 0, "e-mail address：", contact->email, sizeof(contact->email), echo))
		return 1;
	else
		return 0;
}


static int
contact_add(
XO *xo)
{
	CONTACT contact;
	if (xo->max >= MAX_CONTACT)
		vmsg("你的聯絡名單已到達上限!!");
	else if (contact_edit(&contact, DOECHO))
	{
		rec_add(xo->dir, &contact, sizeof(CONTACT));
		xo->pos = XO_TAIL /* xo->max */ ;
		return contact_init(xo);
	}
	return contact_head(xo);
}

static int
contact_delete(
XO *xo)
{

	if (vans(msg_del_ny) == 'y')
	{
		if (!rec_del(xo->dir, sizeof(CONTACT), xo->pos, NULL, NULL))
		{
			return contact_load(xo);
		}
	}
	return XO_FOOT;
}


static int
contact_change(
XO *xo)
{
	CONTACT *contact, mate;
	int pos, cur;

	pos = xo->pos;
	cur = pos - xo->top;
	contact = (CONTACT *) xo_pool + cur;

	mate = *contact;
	contact_edit(contact, GCARRY);
	if (memcmp(contact, &mate, sizeof(CONTACT)))
	{
		rec_put(xo->dir, contact, sizeof(CONTACT), pos);
		move(3 + cur, 0);
		contact_item(++pos, contact);
	}

	return XO_FOOT;
}

static int
contact_help(
XO *xo)
{
	film_out(FILM_CONTACT, -1);
	return contact_head(xo);
}


static int
contact_mail(
XO *xo)
{
	int pos, cur;
	CONTACT *contact;

	pos = xo->pos;
	cur = pos - xo->top;
	contact = (CONTACT *) xo_pool + cur;
	contact_send(contact);
	return contact_init(xo);
}

static int
contact_pop3(
XO *xo)
{
	int pos, cur;
	CONTACT *contact;
	static int (*pop3)(char *email);

	pos = xo->pos;
	cur = pos - xo->top;
	contact = (CONTACT *) xo_pool + cur;
	if (pop3)
		(*pop3)(contact->email);
	else
	{
		pop3 = DL_get("bin/pop3mail.so:Pop3Contact");
		if (pop3)
			(*pop3)(contact->email);
		else
			vmsg("動態載入失敗，請聯絡系統管理員！");
	}
	return contact_init(xo);
}

void
contact_send(
CONTACT *contact)
{
	if (bbsothermode & OTHERSTAT_EDITING)
	{
		vmsg("你還有檔案還沒編完哦！");
		return;
	}

	if (not_addr(contact->email))
		vmsg("E-mail 不正確!!");
	else if (cuser.userlevel & PERM_DENYMAIL)
		vmsg("你的信箱被鎖!!");
	else if (vget(21, 0, "主  題：", ve_title, TTLEN, DOECHO))
	{
		char *msg;
		switch (mail_send(contact->email, ve_title))
		{
		case - 1:
			msg = err_uid;
			break;

		case - 2:
			msg = msg_cancel;
			break;

		case - 3: /* Thor.980707: 有此情況嗎 ?*/
			msg = "使用者無法收信";
			break;

		default:
			msg = "信已寄出";
			break;
		}
		vmsg(msg);
	}
}


KeyFunc contact_cb[] =
{
	{XO_INIT, contact_init},
	{XO_LOAD, contact_load},
	{XO_HEAD, contact_head},
	{XO_BODY, contact_body},

	{Ctrl('P'), contact_add},
	{'m', contact_mail},
	{'r', contact_mail},
	{'c', contact_change},
	{'s', contact_init},
	{'d', contact_delete},
	{'p', contact_pop3},
	{'h', contact_help}
};

int
Contact(void)
{
	XO *xo;
	char fpath[80];
	utmp_mode(M_OMENU);
	usr_fpath(fpath, cuser.userid, "contact");
	xz[XZ_OTHER - XO_ZONE].xo = xo = xo_new(fpath);
	xz[XZ_OTHER - XO_ZONE].cb = contact_cb;
	xover(XZ_OTHER);
	free(xo);
	return 0;
}



