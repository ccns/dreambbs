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

static int contact_add();
void contact_send(CONTACT *contact);

static void
contact_item(num, contact)
int num;
CONTACT *contact;
{
	prints("%6d     %-13s      %-48s\n", num, contact->name, contact->email);
}

static int
contact_body(xo)
XO *xo;
{
	CONTACT *contact;
	int num, max, tail;

	move(3, 0);
	clrtobot();
	max = xo->max;
	if (max <= 0)
	{
		if (vans("�n�s�W��ƶ�(Y/N)�H[N] ") == 'y')
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
contact_head(xo)
XO *xo;
{
	vs_head("�p���W��", str_site);
	outs("\
		 [��]���} ^P)�s�W c)�ק� d)�R�� s)���� m)�H�H [h]elp\n\
		 \033[30;47m  �s��     �p  ��  �W  ��     e-mail address                                  \033[m");
	return contact_body(xo);
}


static int
contact_load(xo)
XO *xo;
{
	xo_load(xo, sizeof(CONTACT));
	return contact_body(xo);
}


static int
contact_init(xo)
XO *xo;
{
	xo_load(xo, sizeof(CONTACT));
	return contact_head(xo);
}


static int
contact_edit(contact, echo)
CONTACT *contact;
int echo;
{
	if (echo == DOECHO)
		memset(contact, 0, sizeof(CONTACT));
	if (vget(b_lines, 0, "�W�١G", contact->name, sizeof(contact->name), echo)
		&& vget(b_lines, 0, "e-mail address�G", contact->email, sizeof(contact->email), echo))
		return 1;
	else
		return 0;
}


static int
contact_add(xo)
XO *xo;
{
	CONTACT contact;
	if (xo->max >= MAX_CONTACT)
		vmsg("�A���p���W��w��F�W��!!");
	else if (contact_edit(&contact, DOECHO))
	{
		rec_add(xo->dir, &contact, sizeof(CONTACT));
		xo->pos = XO_TAIL /* xo->max */ ;
		return contact_init(xo);
	}
	return contact_head(xo);
}

static int
contact_delete(xo)
XO *xo;
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
contact_change(xo)
XO *xo;
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
contact_help(xo)
XO *xo;
{
	film_out(FILM_CONTACT, -1);
	return contact_head(xo);
}


static int
contact_mail(xo)
XO *xo;
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
contact_pop3(xo)
XO *xo;
{
	int pos, cur;
	CONTACT *contact;
	static int (*pop3)();

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
			vmsg("�ʺA���J���ѡA���p���t�κ޲z���I");
	}
	return contact_init(xo);
}

void
contact_send(contact)
CONTACT *contact;
{
	if (bbsothermode & OTHERSTAT_EDITING)
	{
		vmsg("�A�٦��ɮ��٨S�s���@�I");
		return;
	}

	if (not_addr(contact->email))
		vmsg("E-mail �����T!!");
	else if (cuser.userlevel & PERM_DENYMAIL)
		vmsg("�A���H�c�Q��!!");
	else if (vget(21, 0, "�D  �D�G", ve_title, TTLEN, DOECHO))
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

		case - 3: /* Thor.980707: �������p�� ?*/
			msg = "�ϥΪ̵L�k���H";
			break;

		default:
			msg = "�H�w�H�X";
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
Contact()
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



