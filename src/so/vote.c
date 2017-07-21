/*-------------------------------------------------------*/
/* vote.c	( NTHU CS MapleBBS Ver 2.36 )		 */
/*-------------------------------------------------------*/
/* target : boards' vote routines		 	 */
/* create : 95/03/29				 	 */
/* update : 95/12/15				 	 */
/*-------------------------------------------------------*/
/* brd/_/.VCH - Vote Control Header			 */
/* brd/_/@/@_ - vote description file			 */
/* brd/_/@/I_ - vote selection Items			 */
/* brd/_/@/V_ - Vote choice record			 */
/* brd/_/@/O_ - users' Opinions				 */
/*-------------------------------------------------------*/
/* usr/_/vote - users' vote record			 */
/*-------------------------------------------------------*/


#include "bbs.h"
/*
#include "get_socket.c"
#include "pop3_check.c"*/

extern BCACHE *bshm;
extern XZ xz[];
extern char radix32[32];


static int vote_add();
struct Tchoice
{
	int count;
	vitem_t vitem;
};

typedef struct
{
	char email[60];
} LOG;

static int
TchoiceCompare(struct Tchoice * i, struct Tchoice * j)
{
	return j->count - i->count;
}


static int
vote_seek(fpath, mail)
char *fpath;
char *mail;
{
	LOG email;
	int pos = 0, fd;
	fd = open(fpath, O_RDONLY);
	while (fd)
	{
		lseek(fd, (off_t)(sizeof(email) * pos), SEEK_SET);
		if (read(fd, &email, sizeof(email)) == sizeof(email))
		{
			if (!strcmp(email.email, mail))
			{
				close(fd);
				return 1;
			}
			pos++;
		}
		else
		{
			close(fd);
			break;
		}
	}
	return 0;
}


int
vote_result(xo)
XO *xo;
{
	char fpath[80];

	setdirpath(fpath, xo->dir, "@/@vote");
	/* Thor.990204: ���Ҽ{more �Ǧ^�� */
	if (more(fpath, NULL) == -1)
	{
		vmsg("�ثe�S������}�������G");
		return XO_FOOT;
	}

	return XO_HEAD;
}


static void
vote_item(num, vch)
int num;
VCH *vch;
{
	prints("%6d %-9.8s%-12s %-48.48s\n",
		   num, vch->cdate, vch->owner, vch->title);
}


static int
vote_body(xo)
XO *xo;
{
	VCH *vch;
	int num, max, tail;

	max = xo->max;
	if (max <= 0)
	{
		if (bbstate & STAT_BOARD)
		{
			if (vans("�n�|��벼�ܡH(Y/N)[N] ") == 'y')
				return vote_add(xo);
		}
		else
		{
			vmsg("�ثe�õL�벼�|��");
		}
		return XO_QUIT;
	}

	vch = (VCH *) xo_pool;
	num = xo->top;
	tail = num + XO_TALL;
	if (max > tail)
		max = tail;

	move(3, 0);
	do
	{
		vote_item(++num, vch++);
	}
	while (num < max);
	clrtobot();

	return XO_NONE;
}


static int
vote_head(xo)
XO *xo;
{
	vs_head(currBM, "�벼��");
	outs(NECKVOTE);
	return vote_body(xo);
}


static int
vote_init(xo)
XO *xo;
{
	xo_load(xo, sizeof(VCH));
	return vote_head(xo);
}


static int
vote_load(xo)
XO *xo;
{
	xo_load(xo, sizeof(VCH));
	return vote_body(xo);
}


static void
vch_edit(vch)
VCH *vch;
{
	int num;
	char buf[4];

	vget(21, 0, "�����벼�i��X�� (�ܤ֢���)�H", buf, 3, DOECHO);
	num = atoi(buf);
	if (num < 1)
		num = 1;
	vch->vclose = vch->chrono + num * 86400;
	str_stamp(vch->cdate, &vch->vclose);

	vch->vsort =
		(vget(22, 0, "�}�����G�O�_�Ƨ�(Y/N)[N] ", buf, 3, LCECHO) == 'y')
		? 's'
		: ' ';

	vch->vpercent =
		(vget(23, 0, "�}�����G�O�_��ܦʤ����(Y/N)[N] ", buf, 3, LCECHO) == 'y')
		? '%'
		: ' ';
}

static void
show_stud(path)
char *path;
{
	VCHS stud;
	int pos = 0, fd;
	move(3, 0);
	prints("�����벼�W�U�p�U�G\n");
	fd = open(path, O_RDONLY);
	while (fd)
	{
		lseek(fd, (off_t)(sizeof(VCHS) * pos), SEEK_SET);
		if (read(fd, &stud, sizeof(VCHS)) == sizeof(VCHS))
		{
			/*prints("%2d)�Ǧ~�סG%2s  �t�šG%c  �d��G%3s  ->  %3s\n", pos, stud.grad, stud.major[0], stud.first, stud.last);*/
			prints("%2d)�|�t�G%2s  ���G%s  �J�Ǧ~�סG%2s  �d��G%4s  ->  %4s\n", pos, stud.inst, stud.level, stud.admis, stud.first, stud.last);
			pos++;
		}
		else
			return;
	}
	close(fd);
	return;
}

static int
check_stud(account, path)
char *account;
char *path;
{
	VCHS stud, buf;
	int pos = 0, fd;
	/* char buf1[3], buf2[2], buf3[4]; */
	char buf1[3], buf2[2], buf3[3], buf4[5];
	/*
	buf1[0] = account[0];
	buf1[1] = account[1];
	buf1[2] = '\0';
	buf2[0] = account[2];
	buf2[1] = '\0';
	strcpy(buf3, account + 3);
	*/
	strncpy(buf1, account, 2);
	strncpy(buf2, account + 2, 1);
	strncpy(buf3, account + 3, 2);
	strncpy(buf4, account + 5, 4);
	buf1[2] = buf2[1] = buf3[2] = buf4[4] = '\0';
	
	fd = open(path, O_RDONLY);
	while (fd)
	{
		lseek(fd, (off_t)(sizeof(VCHS) * pos), SEEK_SET);
		if (read(fd, &stud, sizeof(VCHS)) == sizeof(VCHS))
		{
			if (stud.inst[0] == '*' || !strcasecmp(stud.inst, buf1))
				if (stud.level[0] == '*' || (stud.level[0] == buf2[0]))
					if(stud.admis[0] == '*' || !strcmp(stud.admis, buf3))
						if(atoi(buf4) >= atoi(stud.first) && atoi(buf4) <= atoi(stud.last))
							return 1;
		/*
			if (((stud.grad[0] == '*' && stud.grad[1] == '*') || atoi(stud.grad) == atoi(buf1)) &&  atoi(buf1) <= 99 && atoi(buf1) >= 0)
				if ((stud.major[0] == '*' || stud.major[0] == buf2[0]) && atoi(buf2) <= 9 && atoi(buf2) >= 0)
					if (atoi(buf3) >= atoi(stud.first) && atoi(buf3) <= atoi(stud.last))
						return 1;
		*/
			pos++;
		}
		else
			return 0;
	}
	close(fd);
	return 0;
}


static int
vlist_student(path)
char *path;
{
	VCHS stud[MAX_BOOKS], tmp;
	int item, i, fd;
	char buf[80];

	move(0, 0);
	clrtobot();

	outs("�Ш̧ǿ�J�W�U (�̦h 16 ��)�A�� ENTER �����G");

	//strcpy(buf, " ) ");
	for (;;)
	{
		item = 0;
		for (;;)
		{
			stud[item].inst[0] = '\0';
			stud[item].level[0] = '\0';
			stud[item].admis[0] = '\0';
			stud[item].first[0] = '\0';
			stud[item].last[0] = '\0';
			
			sprintf(buf, "%2d%s", item, ")�|�t�G");
			if (vget(item + 3, 0, buf, stud[item].inst, sizeof(tmp.inst), GCARRY))
			{
				while ( strcmp("*", stud[item].inst)&&strcasecmp("AN", stud[item].inst)&&!(isalpha(stud[item].inst[0])&&isdigit(stud[item].inst[1])) )
				{
					vmsg("�Ǹ��榡���~�I");
					vget(item + 3, 0, buf, stud[item].inst, sizeof(tmp.inst), GCARRY);
				}
				
				sprintf(buf, "%s", "���G");
				for (;;)
				{
					if (!vget(item + 3, 13, buf, stud[item].level, sizeof(tmp.level), GCARRY))
						strcpy(stud[item].level, "*");
					if (strcmp("*", stud[item].level)&&(!isdigit(stud[item].level[0])||(stud[item].level[0]!='4'&&stud[item].level[0]!='6'&&stud[item].level[0]!='7'&&stud[item].level[0]!='8')))
						vmsg("�Ǹ��榡���~�I");
					else
						break;
				}
					
				sprintf(buf, "%s", "�J�Ǧ~�סG");
				for (;;)
				{
					if (!vget(item + 3, 20, buf, stud[item].admis, sizeof(tmp.admis), GCARRY))
						strcpy(stud[item].admis, "*");
					if (strcmp("*", stud[item].admis)&&!(isdigit(stud[item].admis[0])&&isdigit(stud[item].admis[1])))
						vmsg("�Ǹ��榡���~�I");
					else
						break;
				}

				sprintf(buf, "%s", "�d��G");
				for (;;)
				{
					if (!vget(item + 3, 34, buf, stud[item].first, sizeof(tmp.first), GCARRY))
						strcpy(stud[item].first, "0001");
					if (!(isdigit(stud[item].first[0])&&isdigit(stud[item].first[1])&&isdigit(stud[item].first[2])&&isdigit(stud[item].first[3])))
						vmsg("�Ǹ��榡���~�I");
					else
						break;
				}

				sprintf(buf, "%s", "  ->  ");
				for (;;)
				{
					if (!vget(item + 3, 44, buf, stud[item].last, sizeof(tmp.last), GCARRY))
						strcpy(stud[item].last, stud[item].first);
					if (!(isdigit(stud[item].last[0])&&isdigit(stud[item].last[1])&&isdigit(stud[item].last[2])&&isdigit(stud[item].last[3])))
						vmsg("�Ǹ��榡���~�I");
					else
						break;
				}
					
				stud[item].end = '\n';
			}	
			/*
			stud[item].grad[0] = '\0';
			stud[item].major[0] = '\0';
			stud[item].first[0] = '\0';
			stud[item].last[0] = '\0';
			sprintf(buf, "%2d%s", item, ")�Ǧ~�סG");
			if (vget(item + 3, 0, buf, stud[item].grad, sizeof(tmp.grad), GCARRY))
			{
				sprintf(buf, "%s", "�t�šG");
				if (!vget(item + 3, 15, buf, stud[item].major, sizeof(tmp.major), GCARRY))
					strcpy(stud[item].major, "*");
				sprintf(buf, "%s", "�d��G");
				if (!vget(item + 3, 24, buf, stud[item].first, sizeof(tmp.first), GCARRY))
					strcpy(stud[item].first, "001");
				sprintf(buf, "%s", "  ->  ");
				if (!vget(item + 3, 33, buf, stud[item].last, sizeof(tmp.last), GCARRY))
					strcpy(stud[item].last, stud[item].first);
				stud[item].end = '\n';
			}	
			*/
			else
				break;
			if ((++item >= MAX_CHOICES))
				break;
		}
		if (vans("�O�_���s��J�ﶵ(Y/N)[N] ") != 'y')
			break;
	}
	if (item == 0)
		return 0;
	if ((fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0600)) < 0)
		return 0;
	for (i = 0;i < item;i++)
		write(fd, &stud[i], sizeof(VCHS));
	close(fd);
	return item;
}


static int
vlist_edit(vlist)
vitem_t vlist[];
{
	int item;
	char buf[80];

	/* Thor.980902: lkchu patch: �w��M�e�� */
	move(0, 0);
	clrtobot();

	outs("�Ш̧ǿ�J�ﶵ (�̦h 32 ��)�A�� ENTER �����G");

	strcpy(buf, " ) ");
	for (;;)
	{
		item = 0;
		for (;;)
		{
			buf[0] = radix32[item];
			if (!vget((item & 15) + 3, (item / 16) * 40,
					  buf, vlist[item], sizeof(vitem_t), GCARRY) || (++item >= MAX_CHOICES))
				break;
		}
		if (vans("�O�_���s��J�ﶵ(Y/N)[N] ") != 'y')
			break;
	}
	return item;
}


static int
vote_add(xo)
XO *xo;
{
	VCH vch;
	int fd, item, check = 0;
	char *dir, *str, fpath[80], buf[TTLEN + 1 -5]; /* Thor.981016: �ȶW�L�ù� */
	vitem_t vlist[MAX_CHOICES];
	BRD *brd;

	if (bbsothermode & OTHERSTAT_EDITING)
	{
		vmsg("�A�٦��ɮ��٨S�s���@�I");
		return XO_FOOT;
	}


	if (!(bbstate & STAT_BOARD))
		return XO_NONE;

	if (vans("�벼�覡�G1)�@��벼 2)�ǥͧ벼�t�� �G") == '2')
	{
		check = 1;
	}

	if (!vget(b_lines, 0, "�벼�D�D�G", buf, sizeof(buf), DOECHO))
		return XO_INIT;

	dir = xo->dir;
	fd = hdr_stamp(dir, 0, (HDR *) & vch, fpath);
	if (fd < 0)
	{
		vmsg("�L�k�إߧ벼������");
		blog("VOTE ", fpath);
		return XO_INIT;
	}

	close(fd);
	vmsg("�}�l�s�� [�벼����]");
	fd = vedit(fpath, 0); /* Thor.981020: �`�N�Qtalk�����D */
	if (fd)
	{
		unlink(fpath);
		vmsg("�����벼");
		return XO_HEAD;
	}

	strcpy(vch.title, buf);
	str = strrchr(fpath, '@');

	/* --------------------------------------------------- */
	/* �벼�ﶵ�� : Item					 */
	/* --------------------------------------------------- */
	if (check)
	{
		*str = 'S';
		if (vlist_student(fpath) == 0)
		{
			vmsg("�����벼");
			return XO_INIT;
		}
	}

	memset(vlist, 0, sizeof(vlist));
	item = vlist_edit(vlist);

	*str = 'I';
	fd = open(fpath, O_WRONLY | O_CREAT | O_TRUNC, 0600);
	if (fd < 0)
	{
		vmsg("�L�k�إߧ벼�ﶵ��");
		blog("VOTE ", fpath);
		return XO_INIT;
	}
	write(fd, vlist, item * sizeof(vitem_t));
	close(fd);

	sprintf(buf + 3, "�аݨC�H�̦h�i��X���H([1]��%d): ", item);
	vget(20, 0, buf + 3, buf, 3, DOECHO);
	fd = atoi(buf);
	if (fd < 1)
		fd = 1;
	else if (fd > item)
		fd = item;
	vch.maxblt = fd;

	vch_edit(&vch);

	strcpy(vch.owner, cuser.userid);

	brd = bshm->bcache + brd_bno(currboard);
	brd->bvote++;
	vch.bstamp = brd->bstamp;
	if (check == 1)
		vch.check = 1;

	rec_add(dir, &vch, sizeof(vch));

	vmsg("�}�l�벼�F�I");
	return vote_init(xo);
}


static int
vote_edit(xo)
XO *xo;
{
	int pos, check = 0;
	VCH *vch, vxx;
	char *dir, fpath[80];
	/* Thor: for �ק�벼�ﶵ */
	int fd, item;
	vitem_t vlist[MAX_CHOICES];
	char *fname, buf[TTLEN+1];

	if (bbsothermode & OTHERSTAT_EDITING)
	{
		vmsg("�A�٦��ɮ��٨S�s���@�I");
		return XO_FOOT;
	}


	if (!(bbstate & STAT_BOARD))
		return XO_NONE;

	pos = xo->pos;
	dir = xo->dir;
	vch = (VCH *) xo_pool + (pos - xo->top);

	/* Thor: �ק�벼�D�D */
	vxx = *vch;

	if (vans("�벼�覡�G1)�@��벼 2)�ǥͧ벼�t�� �G") == '2')
	{
		check = 1;
		vxx.check = 1;
	}


	if (!vget(b_lines, 0, "�벼�D�D�G", vxx.title, TTLEN + 1 - 5 /* sizeof(vxx.title) Thor.981020: �ȶW�L�ù� */ , GCARRY))
		return XO_FOOT;

	hdr_fpath(fpath, dir, (HDR *) vch);
	if (vedit(fpath, 0)) /* Thor.981020: �`�N�Qtalk�����D  */
		outs("�A");  /* Thor.981020: �o�ӬO�F��??:P */

	/* Thor: �ק�벼�ﶵ */

	memset(vlist, 0, sizeof(vlist));
	fname = strrchr(fpath, '@');

	if (check)
	{
		*fname = 'S';
		if (vlist_student(fpath) == 0)
		{
			vmsg("�����벼");
			return XO_HEAD;
		}
	}

	*fname = 'I';
	fd = open(fpath, O_RDONLY);

#if 0
	count = read(fd, vlist, sizeof(vlist)) / sizeof(vitem_t);
#endif
	read(fd, vlist, sizeof(vlist));
	close(fd);

	item = vlist_edit(vlist);

	fd = open(fpath, O_WRONLY | O_CREAT | O_TRUNC, 0600);
	if (fd < 0)
	{
		vmsg("�L�k�إߧ벼�ﶵ��");
		blog("VOTE ", fpath);
		return XO_HEAD;
	}
	write(fd, vlist, item * sizeof(vitem_t));
	close(fd);

	/* Thor: �ק�C�H�i�벼�� */

	sprintf(buf + 3, "�аݨC�H�̦h�i��X���H([1]��%d): ", item);
	sprintf(buf, "%d", vxx.maxblt);
	vget(20, 0, buf + 3, buf, 3, GCARRY);
	fd = atoi(buf);
	if (fd < 1)
		fd = 1;
	else if (fd > item)
		fd = item;
	vxx.maxblt = fd;

	vch_edit(&vxx);
	if (memcmp(&vxx, vch, sizeof(VCH)))
	{
		if (vans("�T�w�n�ק�o���벼�ܡH(Y/N)[N]") == 'y')
		{
			*vch = vxx;
			rec_put(dir, vch, sizeof(VCH), pos);
		}
	}

	return vote_head(xo);
}

static int
vote_browse(xo)
XO *xo;
{
	VCH *vch;
	FILE *fp;
	char *fname, buf[80], fpath[80];
	struct Tchoice choice[MAX_CHOICES];
	int total, items, num, fd, ticket, bollt, *list, *head, *tail;
	struct stat st;

	if (!(bbstate & STAT_BOARD))
		return XO_NONE;

	vch = (VCH *) xo_pool + (xo->pos - xo->top);
	hdr_fpath(fpath, xo->dir, (HDR *) vch);


	fname = strrchr(fpath, '@');

	/* vote item */

	*fname = 'I';
	items = 0;
	if ((fp = fopen(fpath, "r")))
	{
		while (fread(&choice[items].vitem, sizeof(vitem_t), 1, fp) == 1)
		{
			choice[items].count = 0;
			items++;
		}
		fclose(fp);
	}

	if (items == 0)
		return XO_NONE;

	/* vote ballots */

	*fname = 'V';

	if ((fd = open(fpath, O_RDONLY)) < 0)
		return XO_NONE;

	if (fstat(fd, &st) || (total = st.st_size) <= 0)
	{
		close(fd);
		return XO_NONE;
	}

	/* �֭p�벼���G */

	bollt = 0;

	list = (int *) malloc(total);
	total = read(fd, list, total);
	close(fd);

	if (total <= 0)
	{
		free(list);
		return XO_NONE;
	}

	head = list;
	tail = (int *)((char *) list + total);

	do
	{
		for (ticket = *head++, num = 0; ticket && num < items; ticket >>= 1, num++)
		{
			if (ticket & 1)
			{
				choice[num].count++;
				bollt++;
			}
		}
	}
	while (head < tail);

	free(list);

	/* ���Ͷ}�����G */
	*fname = 'Z';
	if ((fp = fopen(fpath, "w")) == NULL)
		return XO_NONE;

	memset(buf, '-', 74);
	buf[74] = '\0';

	fprintf(fp, "\n\n> %s <\n\n�� [%s]�ݪO�벼�G%s\n\n�|��H  �G%s\n\n�|�����G%s\n",
			buf, currboard , vch->title, vch->owner, ctime(&vch->chrono));
	fprintf(fp, "�}������G%s\n�� �벼�D�D:\n\n", ctime(&vch->vclose));

	*fname = '@';
	f_suck(fp, fpath);

	fprintf(fp, "�� �벼���G�G�C�H�i�� %d ���A�@ %d �H�ѥ[�A��X %d ��\n\n",
			vch->maxblt, total / sizeof(int), bollt);

	if (vch->vsort == 's')
		qsort(choice, items, sizeof(struct Tchoice), (int (*)())TchoiceCompare);

	if (vch->vpercent == '%')
		fd = BMAX(1, total / sizeof(int));
	else
		fd = 0;

	for (num = 0; num < items; num++)
	{
		ticket = choice[num].count;
		if (fd)
			fprintf(fp, "\t%-42s%3d �� (%4.1f%%)\n",
					choice[num].vitem, ticket, 100.0 * ticket / fd);
		else
			fprintf(fp, "\t%-42s%3d ��\n",
					choice[num].vitem, ticket);
	}

	/* other opinions */

	*fname = 'O';
	fputs("\n�� �ڦ��ܭn���G\n\n", fp);
	f_suck(fp, fpath);
	fclose(fp);
	*fname = 'Z';
	more(fpath, NULL);

	return XO_INIT;

}


static int
vote_query(xo)
XO *xo;
{
	char *dir, *fname, fpath[80], buf[80];
	VCH *vch;
	int cc, pos;

	if (!(bbstate & STAT_BOARD))
		return XO_NONE;

	pos = xo->pos;
	dir = xo->dir;
	vch = (VCH *) xo_pool + (pos - xo->top);

	hdr_fpath(fpath, dir, (HDR *) vch);
	more(fpath, (char *) - 1);

	fname = strrchr(fpath, '@');
	*fname = 'V';
	sprintf(buf, "�@�� %d �H�ѥ[�벼, A)��� B)���� C)���e�}�� Q>uit�H ", /* Thor:�Ȼ~�| */
			rec_num(fpath, sizeof(int)));
	cc = vans(buf);

	if (cc == 'c')
	{
		vch->vclose = time(0);
		str_stamp(vch->cdate, &vch->vclose);
		rec_put(dir, vch, sizeof(VCH), pos);
	}
	else if (cc == 'a')
	{
		vget(b_lines, 0, "�Ч��}���ɶ�(-���e/+����/0����)�G", buf, 3, DOECHO);
		if ((cc = atoi(buf)))
		{
			vch->vclose = vch->vclose + cc * 86400;
			str_stamp(vch->cdate, &vch->vclose);
			rec_put(dir, vch, sizeof(VCH), pos);
		}
	}
	else if (cc == 'b')
	{
		if (vans("�T�w�n�����o���벼�ܡH") == 'y')
		{
			char *list = "@IOVSE";

			while ((*fname = *list++))
			{
				unlink(fpath); /* Thor: �T�w�W�r�N�� */
			}

			cc = currchrono;
			currchrono = vch->chrono;
			rec_del(dir, sizeof(VCH), pos, (void *)cmpchrono, NULL);
			currchrono = cc;
			return vote_init(xo);
		}
	}

	return vote_head(xo);
}


static int
check_mail(account)
char *account;
{
	//char validemail[3][20] = {"ccmail.ncku.edu.tw", "mail.ncku.edu.tw", "nckualumni.org.tw"};
	char addr[20], buf[30] , line[80], server[60], *ptr;
	char year[3];
	int sock = 110;

	sprintf(line, "%s@%s", account, DEFAULTSERVER);

	ptr = strchr(line, '@');
	if (ptr)
		strcpy(server, ptr + 1);
	else
		return 0;
	
	strncpy(year, account + 3, 2);
	
	if( atoi(year) <= 97 )// Ecchi.100331: 98�~�H�e�J�Ǫ��H�c�A�b���������ݳ��n���h�@�X
		strncpy(addr, account, 8);
	else
		strcpy(addr, account);
	
	sprintf(line, "�п�J %s �u�@���b�����K�X�G", server);

	if (!Get_Socket(server, &sock))
	{
		close(sock);
		vget(b_lines - 3, 0, line , buf, 20, NOECHO);
		if (strlen(buf) < 1)
			return 0;
		if (!POP3_Check(server, addr, buf))
			return 1;
		else
			return 0;
	}
	return 0;

}
/*
static int
check_mail(mail)
char *mail;
{
	char addr[20], buf[30] , line[80], server[60], *ptr;
	int sock = 110;

	sprintf(line, "s%s@%s", mail, DEFAULTSERVER);
	check_nckuemail(line);
	ptr = strchr(line, '@');
	if (ptr)
	{
		strcpy(server, ptr + 1);
	}
	else
		return 0;


	addr[0] = 's';
	addr[1] = '\0';
	strcat(addr, mail);

	sprintf(line, "�п�J %s �u�@���b�����K�X�G", server);

	if (!Get_Socket(server, &sock))
	{
		close(sock);
		vget(b_lines - 3, 0, line , buf, 20, NOECHO);
		if (strlen(buf) < 1)
			return 0;
		if (!POP3_Check(server, addr, buf))
		{
			return 1;
		}
		else
			return 0;
	}
	return 0;
}
*/


static int
vote_join(xo)
XO *xo;
{
	VCH *vch;
	int count, choice, fd, fv;
	char *fname, fpath[80], buf[80], *slist[MAX_CHOICES];
	char account[10/*7*/];
	vitem_t vlist[MAX_CHOICES];

	vch = (VCH *) xo_pool + (xo->pos - xo->top);
	if (time(0) > vch->vclose)
	{
		vmsg("�벼�w�g�I��F�A���R�Զ}��");
		return XO_FOOT;
	}

	/* --------------------------------------------------- */
	/* �ˬd�O�_�w�g��L��					 */
	/* --------------------------------------------------- */

#define	FV_SZ	(sizeof(time_t) * 2)

	usr_fpath(buf, cuser.userid, FN_VOTE);
	fv = open(buf, O_RDWR | O_CREAT, 0600);

	/* flock(fv, LOCK_EX); */
	/* Thor.981205: �� fcntl ���Nflock, POSIX�зǥΪk */
	f_exlock(fv);

	while (read(fv, buf, FV_SZ) == FV_SZ)
	{
		if (!memcmp(vch, buf, FV_SZ))
		{
			/* flock(fv, LOCK_UN); */
			/* Thor.981205: �� fcntl ���Nflock, POSIX�зǥΪk */
			f_unlock(fv);

			close(fv);
			vmsg("�A�w�g��L���F�I");
			return XO_FOOT;
		}
	}

	/* --------------------------------------------------- */
	/* �}�l�벼						 */
	/* --------------------------------------------------- */

	hdr_fpath(fpath, xo->dir, (HDR *) vch);
	more(fpath, NULL);

	/* --------------------------------------------------- */
	/* ���J�벼�ﶵ��					 */
	/* --------------------------------------------------- */

	fname = strrchr(fpath, '@');

	*fname = 'E';
	if (vote_seek(fpath, cuser.email))
	{
		f_unlock(fv);
		close(fv);
		vmsg("�A�w�g��L���F�I");
		return XO_INIT;
	}

	if (vch->check == 1)
	{
		*fname = 'S';
		show_stud(fpath);
		account[0] = '\0';
		if (vget(b_lines - 4, 0, "�п�J�A���Ǹ��G", account, 10/*7*/, GCARRY))
		{
			if (strlen(account) < 9/*6*/)
			{
				f_unlock(fv);
				close(fv);
				vmsg("�A���Ǹ����~�I");
				return vote_head(xo);
			}
			if (!check_stud(account, fpath))
			{
				f_unlock(fv);
				close(fv);
				vmsg("�A���b�W�U�̡I");
				return vote_head(xo);
			}
			if (!check_mail(account))
			{
				f_unlock(fv);
				close(fv);
				vmsg("�A���K�X�����T�I");
				return vote_head(xo);
			}
		}
		else
		{
			f_unlock(fv);
			close(fv);
			vmsg("�A���b�W�U�̡I");
			return vote_head(xo);
		}
	}

	*fname = 'I';
	fd = open(fpath, O_RDONLY);
	count = read(fd, vlist, sizeof(vlist)) / sizeof(vitem_t);
	close(fd);

	for (fd = 0; fd < count; fd++)
	{
		slist[fd] = (char *) & vlist[fd];
	}

	/* --------------------------------------------------- */
	/* �i��벼						 */
	/* --------------------------------------------------- */

	choice = 0;
	sprintf(buf, "��U���t�� %d ��", vch->maxblt); /* Thor: ��̦ܳh�X�� */
	vs_bar(buf);
	outs("�벼�D�D�G");
	*buf = '\0';
	for (;;)
	{
		choice = bitset(choice, count, vch->maxblt, vch->title, slist);
		vget(b_lines - 1, 0, "�ڦ��ܭn���G", buf, 60, GCARRY);
		fd = vans("�벼 (Y)�T�w (N)���� (Q)�����H[N] ");
		if (fd == 'y' || fd == 'Y' || fd == 'q' || fd == 'Q')
			break;
	}

	/* --------------------------------------------------- */
	/* �O�����G�G�@���]���몺���p ==> �۷���o��	 */
	/* --------------------------------------------------- */

	if (fd != 'q')
	{
		*fname = 'V';
		fd = open(fpath, O_WRONLY | O_CREAT | O_APPEND, 0600);
		if (fd >= 0)
		{
			write(fd, &choice, sizeof(choice));
			close(fd);

			/* Thor: �g�J�ϥΪ̷N�� */

			if (*buf)
			{
				FILE *fp;

				*fname = 'O';
				if ((fp = fopen(fpath, "a")))
				{
					fprintf(fp, "%-12s: %s\n", cuser.userid, buf);
					fclose(fp);
				}
			}

			write(fv, vch, FV_SZ);
			vmsg("�벼�����I");
		}
		*fname = 'E';
		fd = open(fpath, O_WRONLY | O_CREAT | O_APPEND, 0600);
		if (fd >= 0)
		{
			LOG email;
			strcpy(email.email, cuser.email);
			write(fd, &email, sizeof(email));
			close(fd);
		}
	}

	/* flock(fv, LOCK_UN); */
	/* Thor.981205: �� fcntl ���Nflock, POSIX�зǥΪk */
	f_unlock(fv);

	close(fv);

	return vote_head(xo);
}


static int
vote_help(xo)
XO *xo;
{
	film_out(FILM_VOTE, -1);
	return vote_head(xo);
}


static KeyFunc vote_cb[] =
{
	{XO_INIT, vote_init},
	{XO_LOAD, vote_load},
	{XO_HEAD, vote_head},
	{XO_BODY, vote_body},

	{'r', vote_join},
	{'v', vote_join},
	{'R', vote_result},
	{'m', vote_browse},
	{'S' | XO_DL, (int (*)())"bin/showvote.so:Showvote"},
	{'E', vote_edit},
	{Ctrl('P'), vote_add},
	{Ctrl('Q'), vote_query},

	{'h', vote_help}
};


int
XoVote(xo)
XO *xo;
{
	char fpath[32];

	/* �� post �v�Q���~��ѥ[�벼 */

	if (!(bbstate & STAT_POST) || !cuser.userlevel)
		return XO_NONE;

	setdirpath(fpath, xo->dir, ".VCH");
	if (!(bbstate & STAT_BOARD) && !rec_num(fpath, sizeof(VCH)))
	{
		vmsg("�ثe�S���벼�|��");
		return XO_FOOT;
	}

	xz[XZ_VOTE - XO_ZONE].xo = xo = xo_new(fpath);
	xz[XZ_VOTE - XO_ZONE].cb = vote_cb;
	xover(XZ_VOTE);
	free(xo);

	return XO_INIT;
}

int
SystemVote()
{
	char fpath[80];
	XO *xo;

	/* �� post �v�Q���~��ѥ[�벼 */
	XoPost(brd_bno("SYSOP"));

	sprintf(fpath, "brd/%s/.VCH", brd_sysop);
	if (!(bbstate & STAT_BOARD) && !rec_num(fpath, sizeof(VCH)))
	{
		vmsg("�ثe�S���벼�|��");
		return 0;
	}

	xz[XZ_VOTE - XO_ZONE].xo = xo = xo_new(fpath);
	xz[XZ_VOTE - XO_ZONE].cb = vote_cb;
	xover(XZ_VOTE);
	free(xo);

	return 0;
}


#if 0
void
vote_sync()
{
	int fd, size;
	struct stat st;
	char fpath[64];

	usr_fpath(fpath, cuser.userid, FN_VOTE);

	if ((fd = open(fpath, O_RDWR, 0600)) < 0)
		return;

	outz("�� ��ƾ�z�]�֤��A�еy�� \033[5m...\033[m");
	refresh();

	if (!fstat(fd, &st) && (size = st.st_size) > 0)
	{
		time_t *base, *head, *tail;

		base = head = (time_t *) malloc(size);
		size = read(fd, base, size);
		if (size >= FV_SZ)
		{
			tail = (time_t *)((char *) base + size);

outerLoop:
			while (head < tail)
			{
				int bno = bstamp2bno(head[1]);  /* Thor: head[1] : VCH.bstamp */

				if (bno >= 0)
				{
					int fv;

					brd_fpath(buf, bshm->bcache[bno].brdname, ".VCH");
					fv = open(buf, O_RDONLY);
					if (fv >= 0)
					{
						VCH vch;
						while (read(fv, vch, sizeof vch) == sizeof vch)
						{
							if (!memcmp(&vch, head, FV_SZ))
							{
								head += 2;
								close(fv);
								goto outerLoop;/* continue;  outer while */
							}
						}
						close(fv);
					}
				}

				tail -= 2;
				if (head >= tail)
					break;
				memcpy(head, tail, FV_SZ);
			}

			size = (char *) tail - (char *) base;
			if (size > 0)
			{
				lseek(fd, 0, SEEK_SET);
				write(fd, base, size);
				ftruncate(fd, size);
			}
		}
		else
			size = 0;
		free(base);
	}
	close(fd);

	if (size <= 0)
	{
		unlink(fpath);
	}
}
#endif
