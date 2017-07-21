/*-------------------------------------------------------*/
/* so/mailgem.c        (YZU WindTop BBS 3.0)		 */
/*-------------------------------------------------------*/
/* author : visor.bbs@bbs.yzu.edu.tw     		 */
/* target : ��ذϾ\Ū�B�s��                             */
/* create : 2000/08/27					 */
/* update : 						 */
/*-------------------------------------------------------*/


#include "bbs.h"

extern XZ xz[];
extern char xo_pool[];
extern char radix32[];
extern char brd_bits[];

extern BCACHE *bshm;
extern int TagNum;
extern TagItem TagList[];

/* definitions of MailGem Mode */


#define	GEM_WAY		2
static int mailgem_way;

static int MailGemBufferNum;

static int mailgem_add();
static int mailgem_paste();
static int mailgem_anchor();
static int mailgem_recycle();
static void XoMailGem();

static void
mailgem_item(num, ghdr)
int num;
HDR *ghdr;
{
	int xmode, gtype;

	xmode = ghdr->xmode;
	gtype = (char) 0xba;
	if (xmode & GEM_FOLDER)
		gtype += 1;
	prints("%6d %c\241%c ", num,
		   TagNum && !Tagger(ghdr->chrono, num - 1, TAG_NIN) ? '*' : ' ', gtype);

	gtype = HAS_PERM(PERM_SYSOP) ? mailgem_way : 0;

	prints("%-46.45s%-13s%s\n", ghdr->title,
		   (gtype == 1 ? ghdr->xname : ghdr->owner), ghdr->date);
}


static int
mailgem_body(xo)
XO *xo;
{
	HDR *ghdr;
	int num, max, tail;

	max = xo->max;
	if (max <= 0)
	{
		outs("\n\n�m��ذϡn�|�b�l���Ѧa��������� :)");
		max = vans("(A)�s�W��� (G)����\\�� (W)�귽�^���� [N]�L�Ҩƨ� ");
		switch (max)
		{
		case 'a':
			max = mailgem_add(xo);
			if (xo->max > 0)
				return max;
			break;
		case 'g':
			mailgem_anchor(xo);
			break;
		case 'w':
			mailgem_recycle(xo);
			break;
		}
		return XO_QUIT;
	}

	ghdr = (HDR *) xo_pool;
	num = xo->top;
	tail = num + XO_TALL;
	if (max > tail)
		max = tail;

	move(3, 0);
	do
	{
		mailgem_item(++num, ghdr++);
	}
	while (num < max);
	clrtobot();

	return XO_NONE;
}


static int
mailgem_head(xo)
XO *xo;
{
	char buf[20];

	vs_head("��ؤ峹", xo->xyz);

	sprintf(buf, "(�ŶK�� %d �g)\n", MailGemBufferNum);

	outs("\
		 [��]���} [��]�s�� [f]�Ҧ� [C]�Ȧs [F]��H [Z]�U�� [h]����   ");
	outs(buf);
	outs("\033[44m\
		 �s��     �D              �D                            [�s      ��] [��  ��]\033[m");
	return mailgem_body(xo);
}


static int
mailgem_toggle(xo)
XO *xo;
{
	mailgem_way = (mailgem_way + 1) % GEM_WAY;
	return mailgem_body(xo);
}


static int
mailgem_init(xo)
XO *xo;
{
	xo_load(xo, sizeof(HDR));
	return mailgem_head(xo);
}


static int
mailgem_load(xo)
XO *xo;
{
	xo_load(xo, sizeof(HDR));
	return mailgem_body(xo);
}


/* ----------------------------------------------------- */
/* mailgem_check : attribute / permission check out	 */
/* ----------------------------------------------------- */


static HDR *
mailgem_check(xo, fpath)
XO *xo;
char *fpath;
{
	HDR *ghdr;
	int gtype, level;
	char *folder;

	level = xo->key;

	ghdr = (HDR *) xo_pool + (xo->pos - xo->top);
	gtype = ghdr->xmode;

	if (fpath)
	{
		folder = xo->dir;
		hdr_fpath(fpath, folder, ghdr);
	}
	return ghdr;
}


/* ----------------------------------------------------- */
/* ��Ƥ��s�W�Gappend / insert				 */
/* ----------------------------------------------------- */

static int
mailgem_add(xo)
XO *xo;
{
	int gtype, level, fd, ans;
	char title[80], fpath[80], *dir;
	HDR ghdr;

	level = xo->key;

	gtype = vans("�s�W (A)�峹 (F)���v (P)�K�� (Q)�����H[Q] ");

	if (gtype == 'p')
		return mailgem_paste(xo);

	if (gtype != 'a' && gtype != 'f')
		return XO_FOOT;

	dir = xo->dir;
	fd = -1;
	memset(&ghdr, 0, sizeof(HDR));

	if (!vget(b_lines, 0, "���D�G", title, 64, DOECHO))
		return XO_FOOT;

	fd = hdr_stamp(dir, gtype, &ghdr, fpath);
	if (fd < 0)
		return XO_FOOT;
	close(fd);
	if (gtype == 'a')
	{
		if (bbsothermode & OTHERSTAT_EDITING)
		{
			vmsg("�A�٦��ɮ��٨S�s���@�I");
			return XO_FOOT;
		}
		else if (vedit(fpath, NA))
		{
			unlink(fpath);
			zmsg(msg_cancel);
			return mailgem_head(xo);
		}
		gtype = 0;
	}
	else if (gtype == 'f')
	{
		gtype = GEM_FOLDER;
	}
	ghdr.xmode = gtype;
	strcpy(ghdr.title, title);

	ans = vans("�s���m A)ppend I)nsert N)ext Q)uit [A] ");

	if (ans == 'q')
	{
		if (fd >= 0)
			unlink(fpath);
		return (gtype ? XO_FOOT : mailgem_head(xo));
	}

	if (ans == 'i' || ans == 'n')
		rec_ins(dir, &ghdr, sizeof(HDR), xo->pos + (ans == 'n'), 1);
	else
		rec_add(dir, &ghdr, sizeof(HDR));

	return (gtype ? mailgem_load(xo) : mailgem_init(xo));
}


/* ----------------------------------------------------- */
/* ��Ƥ��ק�Gedit / title				 */
/* ----------------------------------------------------- */


static int
mailgem_edit(xo)
XO *xo;
{
	char fpath[80];
	HDR *hdr;

	if (bbsothermode & OTHERSTAT_EDITING)
	{
		vmsg("�A�٦��ɮ��٨S�s���@�I");
		return XO_FOOT;
	}

	if (!(hdr = mailgem_check(xo, fpath)))
		return XO_NONE;
	vedit(fpath, NA);
	return mailgem_head(xo);
}


static int
mailgem_title(xo)
XO *xo;
{
	HDR *ghdr, xhdr;
	int num;
	char *dir;

	ghdr = mailgem_check(xo, NULL);
	if (ghdr == NULL)
		return XO_NONE;

	xhdr = *ghdr;
	vget(b_lines, 0, "���D�G", xhdr.title, TTLEN + 1, GCARRY);

	dir = xo->dir;

	if (memcmp(ghdr, &xhdr, sizeof(HDR)) &&
		vans("�T�w�n�ק��(Y/N)�H[N]") == 'y')
	{
		*ghdr = xhdr;
		num = xo->pos;
		rec_put(dir, ghdr, sizeof(HDR), num);
		num++;
		move(num - xo->top + 2, 0);
		mailgem_item(num, ghdr);

	}
	return XO_FOOT;
}

static int
mailgem_state(xo)
XO *xo;
{
	HDR *ghdr;
	char *dir, fpath[80];
	struct stat st;

	if (!HAS_PERM(PERM_SYSOP))
		return XO_NONE;

	if (!(ghdr = mailgem_check(xo, fpath)))
		return XO_NONE;

	dir = xo->dir;

	move(12, 0);
	clrtobot();
	outs("\nDir : ");
	outs(dir);
	outs("\nName: ");
	outs(ghdr->xname);
	outs("\nFile: ");
	outs(fpath);

	if (!stat(fpath, &st))
		prints("\nTime: %s\nSize: %d", Ctime(&st.st_mtime), st.st_size);

	vmsg(NULL);

	return mailgem_body(xo);
}


/* ----------------------------------------------------- */
/* ��Ƥ��s���Gedit / title				 */
/* ----------------------------------------------------- */


static int
mailgem_browse(xo)
XO *xo;
{
	HDR *ghdr;
	int xmode;
	char fpath[80], title[TTLEN + 1];

	do
	{
		ghdr = mailgem_check(xo, fpath);
		if (ghdr == NULL)
			break;

		xmode = ghdr->xmode;

		/* browse folder */

		if (xmode & GEM_FOLDER)
		{
			strcpy(title, ghdr->title);
			XoMailGem(fpath, title);
			return mailgem_init(xo);
		}

		/* browse article */

		if ((xmode = more(fpath, MSG_GEM)) == -2)
			return XO_INIT;
		if (xmode == -1)
			break;

		xmode = xo_getch(xo, xmode);

	}
	while (xmode == XO_BODY);

	return mailgem_head(xo);
}


/* ----------------------------------------------------- */
/* ��ذϤ��R���G copy / cut (delete) paste / move	 */
/* ----------------------------------------------------- */


static char MailGemFolder[80], MailGemAnchor[80], MailGemSailor[24];
static HDR *MailGemBuffer;
static int MailGemBufferSiz; /* , MailGemBufferNum; */
/* Thor.990414: ���e�ŧi��mailgem_head�� */


/* �t�m�������Ŷ���J header */


static HDR *
mbuf_malloc(num)
int num;
{
	HDR *gbuf;

	MailGemBufferNum = num;
	if ((gbuf = MailGemBuffer))
	{
		if (MailGemBufferSiz < num)
		{
			num += (num >> 1);
			MailGemBufferSiz = num;
			MailGemBuffer = gbuf = (HDR *) realloc(gbuf, sizeof(HDR) * num);
		}
	}
	else
	{
		MailGemBufferSiz = num;
		MailGemBuffer = gbuf = (HDR *) malloc(sizeof(HDR) * num);
	}

	return gbuf;
}


static void
mailgem_buffer(dir, ghdr)
char *dir;
HDR *ghdr;			/* NULL �N���J TagList, �_�h�N�ǤJ����J */
{
	int num, locus;
	HDR *gbuf;

	if (ghdr)
	{
		num = 1;
	}
	else
	{
		num = TagNum;
		if (num <= 0)
			return;
	}

	gbuf = mbuf_malloc(num);

	if (ghdr)
	{
		memcpy(gbuf, ghdr, sizeof(HDR));
	}
	else
	{
		locus = 0;
		do
		{
			EnumTagHdr(&gbuf[locus], dir, locus);
		}
		while (++locus < num);
	}

	strcpy(MailGemFolder, dir);
}

static int
mailgem_delete(xo)
XO *xo;
{
	HDR *ghdr;
	char *dir, buf[80];
	int tag;

	if (!(ghdr = mailgem_check(xo, NULL)))
		return XO_NONE;

	tag = AskTag("��ذϧR��");

	if (tag < 0)
		return XO_FOOT;

	if (tag > 0)
	{
		sprintf(buf, "�T�w�n�R�� %d �g���Һ�ض�(Y/N)�H[N] ", tag);
		if (vans(buf) != 'y')
			return XO_FOOT;
	}

	dir = xo->dir;

	mailgem_buffer(dir, tag ? NULL : ghdr);

	/* �u�R�� HDR �ä��R���ɮ� */

	if (tag)
	{
		int fd;
		FILE *fpw;

		if ((fd = open(dir, O_RDONLY)) < 0)
			return XO_FOOT;

		if (!(fpw = f_new(dir, buf)))
		{
			close(fd);
			return XO_FOOT;
		}

		mgets(-1);
		tag = 0;

		while ((ghdr = mread(fd, sizeof(HDR))))
		{
			if (Tagger(ghdr->chrono, tag, TAG_NIN))
			{
				if ((fwrite(ghdr, sizeof(HDR), 1, fpw) != 1))
				{
					fclose(fpw);
					unlink(buf);
					close(fd);
					return XO_FOOT;
				}
			}
			tag++;
		}
		close(fd);
		fclose(fpw);
		rename(buf, dir);
		TagNum = 0;
	}
	else
	{
		currchrono = ghdr->chrono;
		rec_del(dir, sizeof(HDR), xo->pos, (void *)cmpchrono, NULL);
	}

	return mailgem_init(xo);
}


static int
mailgem_copy(xo)
XO *xo;
{
	HDR *ghdr;
	int tag;

	ghdr = mailgem_check(xo, NULL);
	if (ghdr == NULL)
		return XO_NONE;

	tag = AskTag("��ذϫ���");

	if (tag < 0)
		return XO_FOOT;

	mailgem_buffer(xo->dir, tag ? NULL : ghdr);

	zmsg("��������");
	return XO_HEAD;
}


static inline int
mailgem_extend(xo, num)
XO *xo;
int num;
{
	char *dir, fpath[80], gpath[80];
	FILE *fp;
	time_t chrono;
	HDR *hdr;

	if (!(hdr = mailgem_check(xo, fpath)))
		return -1;

	if (!(fp = fopen(fpath, "a")))
		return -1;

	dir = xo->dir;
	chrono = hdr->chrono;

	for (hdr = MailGemBuffer; num--; hdr++)
	{
		if ((hdr->chrono != chrono) && !(hdr->xmode & GEM_FOLDER))
		{
			hdr_fpath(gpath, dir, hdr);	/* Thor: ���] hdr�M xo->dir�O�P�@�ؿ� */
			fputs(STR_LINE, fp);
			f_suck(fp, gpath);
		}
	}

	fclose(fp);
	return 0;
}


static int
mailgem_paste(xo)
XO *xo;
{
	int num, ans;
	char *dir;

	if (!(num = MailGemBufferNum))
	{
		zmsg("�Х����� copy �R�O��A paste");
		return XO_NONE;
	}

	dir = xo->dir;
	switch (ans = vans("�s���m A)ppend I)nsert N)ext E)xtend Q)uit [A] "))
	{
	case 'q':
		return XO_FOOT;

	case 'e':
		if (mailgem_extend(xo, num))
		{
			zmsg("[Extend �ɮת��[] �ʧ@�å��������\\");
			return XO_NONE;
		}
		return XO_FOOT;

	case 'i':
	case 'n':
		rec_ins(dir, MailGemBuffer, sizeof(HDR), xo->pos + (ans == 'n'), num);
		break;

	default:
		rec_add(dir, MailGemBuffer, sizeof(HDR) * num);
	}

	return mailgem_load(xo);
}


static int
mailgem_move(xo)
XO *xo;
{
	HDR *ghdr;
	char *dir, buf[80];
	int pos, newOrder;

	ghdr = mailgem_check(xo, NULL);

	if (ghdr == NULL)
		return XO_NONE;

	pos = xo->pos;
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
		if (!rec_del(dir, sizeof(HDR), pos, NULL, NULL))
		{
			rec_ins(dir, ghdr, sizeof(HDR), newOrder, 1);
			xo->pos = newOrder;
			return XO_LOAD;
		}
	}
	return XO_FOOT;
}


static int
mailgem_anchor(xo)
XO *xo;
{
	int ans;
	char *folder;

	ans = vans("��ذ� A)�w�� D)���� J)�N�� Q)���� [J] ");
	if (ans != 'q')
	{
		folder = MailGemAnchor;

		if (ans == 'a')
		{
			strcpy(folder, xo->dir);
			str_ncpy(MailGemSailor, xo->xyz, sizeof(MailGemSailor));
		}
		else if (ans == 'd')
		{
			*folder = '\0';
		}
		else
		{
			if (!*folder)
			{
				vmsg("�|���w��");
				return  XO_FOOT;
			}

			XoMailGem(folder, "�� ��ةw��� ��");
			return XO_INIT;
		}

		zmsg("��ʧ@����");
	}

	return XO_NONE;
}


int
mailgem_gather(xo)
XO *xo;
{
	HDR *hdr, *gbuf, ghdr, xhdr;
	int tag, locus, rc, xmode, anchor;
	char *dir, *folder, *msg, fpath[80], buf[80];
	FILE *fp, *fd;

	usr_fpath(fpath, cuser.userid, "gem");

	if (access(fpath, 0))
		mak_dirs(fpath);

	folder = MailGemAnchor;
	if ((anchor = *folder))
	{
		msg = buf;
		sprintf(msg, "�����ܫH���ةw��� (%s)", MailGemSailor);
	}
	else
	{
		msg = "�����ܺ�ئ^����";
	}
	tag = AskTag(msg);

	if (tag < 0)
		return XO_FOOT;

	if (!anchor)
	{
		usr_fpath(folder, cuser.userid, "gem/.GEM");
	}

	fd = fp = NULL;

	if (tag > 0)
	{
		switch (vans("��C�峹 1)�X���@�g 2)���O���� Q)���� [2] "))
		{
		case 'q':
			return XO_FOOT;

		case '1':
			strcpy(xhdr.title, currtitle);
			if (!vget(b_lines, 0, "���D�G", xhdr.title, TTLEN + 1, GCARRY))
				return XO_FOOT;
			fp = fdopen(hdr_stamp(folder, 'A', &ghdr, fpath), "w");
			strcpy(ghdr.owner, cuser.userid);
			strcpy(ghdr.title, xhdr.title);
			break;
		default:
			break;

		}
	}

	if (tag)
		hdr = &xhdr;
	else
		hdr = (HDR *) xo_pool + xo->pos - xo->top;

	dir = xo->dir;
	rc = (*dir == 'b') ? XO_LOAD : XO_FOOT;

	/* gather ���P copy, �i�ǳƧ@ paste */

	strcpy(MailGemFolder, folder);
	gbuf = mbuf_malloc((fp != NULL || tag == 0) ? 1 : tag);

	locus = 0;

	do
	{
		if (tag)
		{
			EnumTagHdr(hdr, dir, locus);
		}

		xmode = hdr->xmode;

		if (!(xmode & GEM_FOLDER))	/* �d hdr �O�_ plain text */
		{
			hdr_fpath(fpath, dir, hdr);

			if (fp)
			{
				f_suck(fp, fpath);
				fputs(STR_LINE, fp);
			}
			else
			{
				fd = fdopen(hdr_stamp(folder, 'A', &ghdr, buf), "w");
				strcpy(ghdr.owner, cuser.userid);
				strcpy(ghdr.title, hdr->title);
				f_suck(fd, fpath);
				rec_add(folder, &ghdr, sizeof(HDR));

				if (fd >= 0)              /* by visor */
					fclose(fd);

				gbuf[locus] = ghdr;	/* ��J MailGembuffer */
			}

		}
	}
	while (++locus < tag);

	if (fp)
	{
		fclose(fp);
		gbuf[0] = ghdr;
		rec_add(folder, &ghdr, sizeof(HDR));
	}

	zmsg("��������");

	return rc;
}


static int
mailgem_tag(xo)
XO *xo;
{
	HDR *ghdr;
	int pos, tag, cur;

	pos = xo->pos;
	cur = pos - xo->top;
	ghdr = (HDR *) xo_pool + cur;

	if ((tag = Tagger(ghdr->chrono, pos, TAG_TOGGLE)))
	{
		move(3 + pos - xo->top, 7);
		outc(tag > 0 ? '*' : ' ');
	}

	return xo->pos + 1 + XO_MOVE;
}


static int
mailgem_help(xo)
XO *xo;
{
	/*  film_out(FILM_GEM, -1);*/
	vmsg("�|���s��ϥλ���");
	return mailgem_head(xo);
}

static int
mailgem_cross(xo)
XO *xo;
{
	char xboard[20], fpath[80], xfolder[80], xtitle[80], buf[80], *dir;
	HDR *hdr, xpost, *ghdr;
	int method = 1, rc, tag, locus, battr;
	FILE *xfp;

	if (!cuser.userlevel)
		return XO_NONE;

	ghdr = mailgem_check(xo, NULL);
	tag = AskTag("��K");
	if ((tag < 0) || (tag == 0 && (ghdr->xmode & GEM_FOLDER)))
		return XO_FOOT;


	if (ask_board(xboard, BRD_W_BIT,
				  "\n\n\033[1;33m�ЬD��A���ݪO�A������K�W�L�T�O�C\033[m\n\n"))
	{
		if (*xboard == 0)
			strcpy(xboard, currboard);

		hdr = tag ? &xpost : (HDR *) xo_pool + (xo->pos - xo->top);


		if (!tag)
		{
			if (!(hdr->xmode & GEM_FOLDER))
			{
				sprintf(xtitle, "[���]%.66s", hdr->title);
				if (!vget(2, 0, "���D:", xtitle, TTLEN + 1, GCARRY))
					return XO_HEAD;
			}
			else
				return XO_HEAD;
		}

		rc = vget(2, 0, "(S)�s�� (Q)�����H[Q] ", buf, 3, LCECHO);
		if (*buf != 's' && *buf != 'S')
			return XO_HEAD;

		locus = 0;
		dir = xo->dir;

		battr = (bshm->bcache + brd_bno(xboard))->battr;

		do
		{
			if (tag)
			{
				EnumTagHdr(hdr, dir, locus++);

				sprintf(xtitle, "[���]%.66s", hdr->title);
			}
			if (!(hdr->xmode & GEM_FOLDER))
			{
				xo_fpath(fpath, dir, hdr);
				brd_fpath(xfolder, xboard, fn_dir);

				method = hdr_stamp(xfolder, 'A', &xpost, buf);
				xfp = fdopen(method, "w");

				strcpy(ve_title, xtitle);
				strcpy(buf, currboard);
				strcpy(currboard, xboard);

				ve_header(xfp);

				strcpy(currboard, buf);

				strcpy(buf, cuser.userid);
				strcat(buf, "] �H���ذ�");
				fprintf(xfp, "�� ��������� [%s\n\n", buf);

				f_suck(xfp, fpath);
				fclose(xfp);
				close(method);

				strcpy(xpost.owner, cuser.userid);
				strcpy(xpost.nick, cuser.username);


				strcpy(xpost.title, xtitle);

				rec_add(xfolder, &xpost, sizeof(xpost));

			}
		}
		while (locus < tag);

		if (battr & BRD_NOCOUNT)
		{
			outs("��������A�峹���C�J�����A�q�Х]�[�C");
		}
		else
		{
			cuser.numposts += (tag == 0) ? 1 : tag;
			vmsg("�������");
		}
	}
	return XO_HEAD;
}

static int
mailgem_recycle(xo)
XO *xo;
{
	char fpath[128];
	usr_fpath(fpath, cuser.userid, "gem/.GEM");
	XoMailGem(fpath, "�ڪ���ذϸ귽�^����");
	return XO_INIT;
}

static KeyFunc mailgem_cb[] =
{
	{XO_INIT, mailgem_init},
	{XO_LOAD, mailgem_load},
	{XO_HEAD, mailgem_head},
	{XO_BODY, mailgem_body},

	{'r', mailgem_browse},

	{Ctrl('P'), mailgem_add},
	{'E', mailgem_edit},
	{'T', mailgem_title},
	{'x', mailgem_cross},
	{'M', mailgem_move},
	{'d', mailgem_delete},
	{'c', mailgem_copy},
	{'W', mailgem_recycle},

	{Ctrl('G'), mailgem_anchor},
	{Ctrl('V'), mailgem_paste},

	{'t', mailgem_tag},
	{'f', mailgem_toggle},

	{'S', mailgem_state},

	{'h', mailgem_help}
};


static void
XoMailGem(folder, title)
char *folder;
char *title;
{
	XO *xo, *last;

	last = xz[XZ_MAILGEM - XO_ZONE].xo;	/* record */

	xz[XZ_MAILGEM - XO_ZONE].xo = xo = xo_new(folder);
	xo->pos = 0;
	xo->key = 0;
	xo->xyz = title;

	xover(XZ_MAILGEM);

	free(xo);

	xz[XZ_MAILGEM - XO_ZONE].xo = last;	/* restore */
}


void
mailgem_main()
{
	XO *xo;
	char fpath[128];

	usr_fpath(fpath, cuser.userid, "gem");

	if (access(fpath, 0))
		mak_dirs(fpath);

	usr_fpath(fpath, cuser.userid, "gem/.DIR");
	xz[XZ_MAILGEM - XO_ZONE].xo = xo = xo_new(fpath);
	xz[XZ_MAILGEM - XO_ZONE].cb = mailgem_cb;
	xo->pos = 0;
	xo->key = 0;
	xo->xyz = "�ڪ���ذ�";
	xover(XZ_MAILGEM);
	free(xo);
}


/* ----------------------------------------------------- */
/* sync mailgem			                         */
/* ----------------------------------------------------- */
#define	GEM_DROP	0x0080
#define	GCHECK_DEPTH	30
#define	GEM_EXPIRE	45	/* gem �ܦh�s 45 �� */

static char pgem[128], pool[128];

int gcheck(int level, char *fpath);


/* ----------------------------------------------------- */
/* synchronize folder & files				 */
/* ----------------------------------------------------- */


typedef struct
{
	time_t chrono;
	char prefix;
	char exotic;
}      SyncData;


static SyncData *sync_pool;
static int sync_size, sync_head;


#define	SYNC_DB_SIZE	2048


static int
sync_cmp(s1, s2)
SyncData *s1, *s2;
{
	return s1->chrono - s2->chrono;
}


static void
sync_init(fname)
char *fname;
{
	int ch, prefix;
	time_t chrono;
	char *str, fpath[80];
	struct stat st;
	struct dirent *de;
	DIR *dirp;

	SyncData *xpool;
	int xsize, xhead;

	if ((xpool = sync_pool))
	{
		xsize = sync_size;
	}
	else
	{
		xpool = (SyncData *) malloc(SYNC_DB_SIZE * sizeof(SyncData));
		xsize = SYNC_DB_SIZE;
	}

	xhead = 0;

	ch = strlen(fname);
	memcpy(fpath, fname, ch);
	fname = fpath + ch;
	*fname++ = '/';

	ch = '0';
	for (;;)
	{
		*fname = ch++;
		fname[1] = '\0';

		if ((dirp = opendir(fpath)))
		{
			fname[1] = '/';
			while ((de = readdir(dirp)))
			{
				str = de->d_name;
				prefix = *str;
				if (prefix == '.')
					continue;

				strcpy(fname + 2, str);
				if (stat(fpath, &st) || !st.st_size)
				{
					unlink(fpath);
					continue;
				}

				chrono = chrono32(str);

				if (xhead >= xsize)
				{
					xsize += (xsize >> 1);
					xpool = (SyncData *) realloc(xpool, xsize * sizeof(SyncData));
				}

				xpool[xhead].chrono = chrono;
				xpool[xhead].prefix = prefix;
				xpool[xhead].exotic = 1;
				xhead++;
			}

			closedir(dirp);
		}

		if (ch == 'W')
			break;

		if (ch == '9' + 1)
			ch = 'A';
	}

	if (xhead > 1)
		qsort(xpool, xhead, sizeof(SyncData), sync_cmp);

	sync_pool = xpool;
	sync_size = xsize;
	sync_head = xhead;

}


static void
sync_check(fgem)
char *fgem;
{
	int expire;
	char *str, *fname, fpath[128], fnew[128];
	SyncData *xpool, *xtail, *xsync;
	time_t cc, due;
	FILE *fpr, *fpw;
	HDR hdr;
	struct tm *pt;

	if ((sync_head) <= 0)
		return;

	fpr = fopen(fgem, "r");
	if (fpr == NULL)
	{
		return;
	}

	sprintf(fnew, "%s.n", fgem);
	fpw = fopen(fnew, "w");
	if (fpw == NULL)
	{
		fclose(fpr);
		return;
	}

	/* �M�z�O������z�� */
	strcpy(fpath, fgem);
	str = strrchr(fpath, '.');
	str[1] = '/';
	fname = str + 2;

	expire = 0;
	due = time(0) - GEM_EXPIRE * 86400;

	while (fread(&hdr, sizeof(hdr), 1, fpr) == 1)
	{
		if ((hdr.xmode & GEM_DROP) && !(memcmp(hdr.title, "�ɮ��B�� [", 10)))
		{
			if ((xsync = (SyncData *) bsearch(&hdr.chrono, sync_pool, sync_head, sizeof(SyncData), sync_cmp)))
			{
				if (xsync->exotic == 0)	/* �w�Q reference */
					continue;
				else
				{
					if (hdr.xid < due)
					{
						expire++;

						xsync->exotic = 0;
						cc = xsync->chrono;
						*str = radix32[cc & 31];
						archiv32m(cc, fname);
						fname[0] = xsync->prefix;
						unlink(fpath);
						continue;
					}
				}
			}
			else
			{
				continue;
			}
		}

		fwrite(&hdr, sizeof(hdr), 1, fpw);
	}
	fclose(fpr);

	/* recycle : recover "lost chain" */

	xpool = xsync = sync_pool;
	xtail = xpool + sync_head;

	strcpy(fpath, fgem);
	str = strrchr(fpath, '.');
	str[1] = '/';
	fname = str + 2;

	/* setup header */

	memset(&hdr, 0, sizeof(hdr));
	time((time_t *) &hdr.xid);
	pt = localtime((time_t *) & hdr.xid);
	sprintf(hdr.date, "%02d/%02d/%02d", pt->tm_mon + 1, pt->tm_mday, pt->tm_year % 100);

	/* �Y lost chain �� folder, ����g�^ */

	hdr.xmode = GEM_FOLDER | GEM_DROP;

	do
	{
		if ((xsync->exotic) && (xsync->prefix == 'F'))
		{
			xsync->exotic = 0;
			cc = xsync->chrono;
			*str = radix32[cc & 31];
			archiv32m(cc, fname);
			fname[0] = xsync->prefix;
			if (gcheck(1, fpath))
			{
				hdr.chrono = cc;
				strcpy(hdr.xname, fname);
				sprintf(hdr.title, "�ɮ��B�� [%s]", str);
				fwrite(&hdr, sizeof(hdr), 1, fpw);
			}
			else
			{
				unlink(fpath);
			}
		}

	}
	while (++xsync < xtail);

	/* �B�z�@���ɮ� */

	hdr.xmode = GEM_DROP;

	do
	{
		if (xpool->exotic)
		{
			cc = xpool->chrono;
			*str = radix32[cc & 31];
			archiv32m(cc, fname);
			fname[0] = xpool->prefix;

			hdr.chrono = cc;
			strcpy(hdr.xname, fname);
			strcpy(hdr.owner, "�t�Φ۰ʺ��@");
			sprintf(hdr.title, "�ɮ��B�� [%s]", str);
			fwrite(&hdr, sizeof(hdr), 1, fpw);
		}
	}
	while (++xpool < xtail);

	fclose(fpw);

	rename(fnew, fgem);

}


/* ----------------------------------------------------- */
/* visit the hierarchy recursively			 */
/* ----------------------------------------------------- */


int
gcheck(level, fpath)
int level;
char *fpath;
{
	int count, xmode, xhead;
	char *fname, *ptr = NULL, buf[80];
	FILE *fp;
	HDR hdr;
	SyncData *xsync;

	if (!level)
	{
		sync_init(fpath);
		sprintf(pgem, "%s/.GEM", fpath);
		sprintf(pool, "%s/.DIR", fpath);
		fpath = pool;
	}
	else if (level > GCHECK_DEPTH)
	{
		return 1;
	}

	/* open the folder */

	fp = fopen(fpath, "r");
	if (!fp)
		return 0;

	strcpy(buf, fpath);

	fname = fpath;
	while ((xmode = *fname++))
	{
		if (xmode == '/')
			ptr = fname;
	}
	if (*ptr != '.')
		ptr -= 2;
	fname = ptr;

	/* --------------------------------------------------- */
	/* visit the header file				 */
	/* --------------------------------------------------- */

	count = 0;
	xhead = sync_head;
	while (fread(&hdr, sizeof(hdr), 1, fp) == 1)
	{
		ptr = hdr.xname;		/* F1234567 */

		if (*ptr == '@')
		{
			continue;
		}

		xmode = hdr.xmode;

		if ((xsync = (SyncData *) bsearch(&hdr.chrono, sync_pool, xhead, sizeof(SyncData), sync_cmp)))
		{
			xsync->exotic = 0;	/* ���`���p : ���Q reference */
		}

		/* �Y���@�� folder �h recursive �i�J */

		if ((xmode & GEM_FOLDER))
		{
			sprintf(fname, "%c/%s", ptr[7], ptr);
			if (!gcheck(level + 1, fpath))
				continue;
		}

		count++;
	}

	fclose(fp);

	if (!level)
	{
		strcpy(pool, pgem);
		gcheck(1, pool);
		sync_check(pgem);
	}

	return count;
}



