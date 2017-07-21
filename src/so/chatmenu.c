/*-------------------------------------------------------*/
/* chatmenu.c   ( YZU_CSE WindTop BBS )                  */
/*-------------------------------------------------------*/
/* author : visor.bbs@bbs.yzu.edu.tw                     */
/* target : chatroom actions routines                    */
/* create : 2000/01/02                                   */
/* update : NULL                                         */
/*-------------------------------------------------------*/

#undef	_MODES_C_
#include "bbs.h"

extern XZ xz[];


static int chat_add();
static int mode = 0;
static int kind = 0;

static void
chat_item(num, chat)
int num;
ChatAction *chat;
{
	if (!mode)
		prints("%6d %-9s %-6s %-54.54s\n", num, chat->verb, chat->chinese, chat->part1_msg);
	else
		prints("%6d %-9s %-6s %-54.54s\n", num, chat->verb, chat->chinese, chat->part2_msg);
}

static int
chat_body(xo)
XO *xo;
{
	ChatAction *chat;
	int num, max, tail;

	move(3, 0);
	clrtobot();
	max = xo->max;
	if (max <= 0)
	{
		if (vans("�n�s�W��ƶ�(Y/N)�H[N] ") == 'y')
			return chat_add(xo);
		return XO_QUIT;
	}

	chat = (ChatAction *) xo_pool;
	num = xo->top;
	tail = num + XO_TALL;
	if (max > tail)
		max = tail;

	do
	{
		chat_item(++num, chat++);
	}
	while (num < max);

	return XO_NONE;
}


static int
chat_head(xo)
XO *xo;
{
	char *title = NULL;
	switch (kind)
	{
	case 0:
		title = CHATROOMNAME "�ʵ��@";
		break;
	case 1:
		title = CHATROOMNAME "�ʵ��G";
		break;
	case 2:
		title = CHATROOMNAME "�ʵ��T";
		break;
	case 3:
		title = CHATROOMNAME "�ʵ��|";
		break;
	case 4:
		title = CHATROOMNAME "�ʵ��� - �ӤH��";
		break;
	}

	vs_head(title, str_site);
	if (mode == 0)
		outs("\
			 [��]���} ^P)�s�W c)�ק� d)�R�� s)���� M)�h�� TAB)�ʵ� f)�T�� [h]elp\n\
			 \033[30;47m  �s�� �ʵ�      ����   �T���@                                                \033[m");
	else
		outs("\
			 [��]���} ^P)�s�W c)�ק� d)�R�� s)���� M)�h�� TAB)�ʵ� f)�T�� [h]elp\n\
			 \033[30;47m  �s�� �ʵ�      ����   �T���G                                                \033[m");
	return chat_body(xo);
}


static int
chat_load(xo)
XO *xo;
{
	xo_load(xo, sizeof(ChatAction));
	return chat_body(xo);
}


static int
chat_init(xo)
XO *xo;
{
	xo_load(xo, sizeof(ChatAction));
	return chat_head(xo);
}


static int
chat_edit(chat, echo)
ChatAction *chat;
int echo;
{
	if (echo == DOECHO)
		memset(chat, 0, sizeof(ChatAction));
	if (vget(b_lines, 0, "�ʵ��G", chat->verb, sizeof(chat->verb), echo)
		&& vget(b_lines, 0, "��������G", chat->chinese, sizeof(chat->chinese), echo))
	{
		vget(b_lines, 0, "�T���@�G", chat->part1_msg, sizeof(chat->part1_msg), echo);
		vget(b_lines, 0, "�T���G�G", chat->part2_msg, sizeof(chat->part2_msg), echo);
		return 1;
	}
	else
		return 0;
}


static int
chat_add(xo)
XO *xo;
{
	ChatAction chat;

	if (chat_edit(&chat, DOECHO))
	{
		rec_add(xo->dir, &chat, sizeof(ChatAction));
		xo->pos = XO_TAIL ;
		xo_load(xo, sizeof(ChatAction));
	}
	return chat_head(xo);
}

static int
chat_delete(xo)
XO *xo;
{

	if (vans(msg_del_ny) == 'y')
	{
		if (!rec_del(xo->dir, sizeof(ChatAction), xo->pos, NULL, NULL))
		{
			return chat_load(xo);
		}
	}
	return XO_FOOT;
}


static int
chat_change(xo)
XO *xo;
{
	ChatAction *chat, mate;
	int pos, cur;

	pos = xo->pos;
	cur = pos - xo->top;
	chat = (ChatAction *) xo_pool + cur;

	mate = *chat;
	chat_edit(chat, GCARRY);
	if (memcmp(chat, &mate, sizeof(ChatAction)))
	{
		rec_put(xo->dir, chat, sizeof(ChatAction), pos);
		move(3 + cur, 0);
		chat_item(++pos, chat);
	}

	return XO_FOOT;
}

static int
chat_help(xo)
XO *xo;
{
	return XO_NONE;
}

static int
chat_mode(xo)
XO *xo;
{
	mode ^= 1;
	return chat_head(xo);
}

static int
chat_kind(xo)
XO *xo;
{
	char fpath[80];

	kind++;
	if (kind > 4) kind = 0;
	switch (kind)
	{
	case 0:
		sprintf(fpath, FN_CHAT_PARTY_DB);
		break;
	case 1:
		sprintf(fpath, FN_CHAT_SPEAK_DB);
		break;
	case 2:
		sprintf(fpath, FN_CHAT_CONDITION_DB);
		break;
	case 3:
		sprintf(fpath, FN_CHAT_PARTY2_DB);
		break;
	case 4:
		sprintf(fpath, FN_CHAT_PERSON_DB);
		break;
	}
	free(xz[XZ_OTHER - XO_ZONE].xo);
	xz[XZ_OTHER - XO_ZONE].xo = xo_new(fpath);
	return XO_INIT;
}

static int
chat_move(xo)
XO *xo;
{
	ChatAction *ghdr;
	char *dir, buf[80];
	int pos, newOrder, cur;

	pos = xo->pos;
	cur = pos - xo->top;
	ghdr = (ChatAction *) xo_pool + cur;

	sprintf(buf + 5, "�п�J�� %d �ﶵ���s��m�G", pos + 1);
	if (!vget(b_lines, 0, buf + 5, buf, 5, DOECHO))
		return XO_FOOT;

	newOrder = atoi(buf) - 1;
	if (newOrder < 0)
		newOrder = 0;
	else if (newOrder >= xo->max)
		newOrder = xo->max - 1;
	if (newOrder != pos)
	{
		dir = xo->dir;
		if (!rec_del(dir, sizeof(ChatAction), pos, NULL, NULL))
		{
			rec_ins(dir, ghdr, sizeof(ChatAction), newOrder, 1);
			xo->pos = newOrder;
			return XO_LOAD;
		}
	}
	return XO_FOOT;
}

static int
chat_sync(xo)
XO *xo;
{
	int fd, size;
	struct stat st;

	if ((fd = open(xo->dir, O_RDWR, 0600)) < 0)
		return 0;

	outz("�� ��ƾ�z�]�֤��A�еy�� \033[5m...\033[m");
	refresh();

	if (!fstat(fd, &st) && (size = st.st_size) > 0)
	{
		int total;

		total = size / sizeof(ChatAction);
		size = total * sizeof(ChatAction);
		if (size >= sizeof(ChatAction))
			ftruncate(fd, size);
	}
	close(fd);
	return XO_INIT;
}


KeyFunc chat_cb[] =
{
	{XO_INIT, chat_init},
	{XO_LOAD, chat_load},
	{XO_HEAD, chat_head},
	{XO_BODY, chat_body},

	{Ctrl('P'), chat_add},
	{'a', chat_add},
	{'r', chat_change},
	{'c', chat_change},
	{'s', chat_init},
	{'S', chat_sync},
	{'f', chat_mode},
	{'M', chat_move},
	{KEY_TAB, chat_kind},
	{'d', chat_delete},
	{'h', chat_help}
};


int
Chatmenu()
{
	char fpath[64];
	XO *xx;

	utmp_mode(M_OMENU);
	sprintf(fpath, FN_CHAT_PARTY_DB);
	kind = 0;
	xz[XZ_OTHER - XO_ZONE].xo = xx = xo_new(fpath);
	xz[XZ_OTHER - XO_ZONE].cb = chat_cb;
	xover(XZ_OTHER);
	free(xx);
	return 0;
}



