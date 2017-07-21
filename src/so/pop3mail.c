/*-------------------------------------------------------*/
/* pop3mail.c       ( YZU WindTop BBS )                  */
/*-------------------------------------------------------*/
/* author : statue.bbs@bbs.yzu.edu.tw			 */
/* author : visor.bbs@bbs.yzu.edu.tw                     */
/* target : pop3 mail client in mime transfer            */
/* create : 2000/06/22                                   */
/* update : 2000/07/11                                   */
/* rfc: 1734, 1939, 2449, 1225				 */
/*-------------------------------------------------------*/

#include "bbs.h"

#define MIME_BASE64     0x10000000
#define MIME_QP         0x20000000
#define MIME_MIME       0x40000000

#define	MAX_SIZE	(10485760)

static int is_mime; /* ���� MIME �Ϊ��X�� */
static int is_base64;
static int is_qp;
static int is_bbsmail;
static char boundary[128];

extern XZ xz[];

static FILE *fsock; /* �s�u�Ϊ� FILE */

static int del_num; /* �Ψ� UnDelete �� , �w�R���峹��*/

extern int TagNum;
static int server_login();
static char *pop3_mime_transfer(unsigned char *);

/* ----------------------------------------------------- */
/* QP code : "0123456789ABCDEF"                          */
/* QP �O�@�ظѽX�覡 quoted-printable                    */
/* �@�g�峹���� '=' ���S���W�L�@�w�ƥ�.                  */
/* ----------------------------------------------------- */

static int
qp_code(x)
register int x;
{
	if (x >= '0' && x <= '9')
		return x - '0';
	if (x >= 'a' && x <= 'f')
		return x - 'a' + 10;
	if (x >= 'A' && x <= 'F')
		return x - 'A' + 10;
	return -1;
}

/* ------------------------------------------------------------------ */
/* BASE64 :                                                           */
/* "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/" */
/* Base64�L�k�P�_, ���D�ڭ̦�header                                   */
/* ------------------------------------------------------------------ */

static int
base64_code(x)
register int x;
{
	if (x >= 'A' && x <= 'Z')
		return x - 'A';
	if (x >= 'a' && x <= 'z')
		return x - 'a' + 26;
	if (x >= '0' && x <= '9')
		return x - '0' + 52;
	if (x == '+')
		return 62;
	if (x == '/')
		return 63;
	return -1;
}

/* ----------------------------------------------------- */
/* judge & decode QP / BASE64                            */
/* ----------------------------------------------------- */

/* �h�������n�����Y */
static int
pop3_mime_cut(unsigned char* src, HDR *hdr, FILE *fp, FILE *sock)
{
	char *ptr;

	/* �Y�S���}�� MIME �ѽX�ﶵ�h�����ѽX */
	if (!is_mime)
		return 1;

	if (*boundary != 0 && !strncmp(src + 2, boundary, strlen(boundary) - 2))
	{
		fprintf(fp, "> ----------------------------------------------------------------------- <\n");
		return -1;
	}
	if (str_str(src, "<pre>"))
		return -1;

	if (str_str(src, "</pre>"))
	{
		hdr->xmode |= MIME_MIME;
		return 0;
	}

	if (!strncmp(src, "----", strlen("----")) ||
		!strncmp(src, "\tboundary", strlen("\tboundary")))
		return 0; /* ���L�������ѽX */

#if 1
	if (!strncmp(src, "Content-Type: ", strlen("Content-Type: ")))
	{
		ptr = src + strlen("Content-Type: ");
		if (!str_ncmp(ptr, "text/html", strlen("text/html")))
		{
			hdr->xmode |= MIME_MIME;
			fprintf(fp, "html �������榡\n");
			return 0;
		}
		if (!str_ncmp(ptr, "image", strlen("image")))
		{
			hdr->xmode |= MIME_MIME;
			fprintf(fp, "image ������ : �Ϥ���\n");
			return 0;
		}
		if (!str_ncmp(ptr, "audio", strlen("audio")))
		{
			hdr->xmode |= MIME_MIME;
			fprintf(fp, "audio ������ : ������\n");
			return 0;
		}
		if (!str_ncmp(ptr, "video", strlen("video")))
		{
			hdr->xmode |= MIME_MIME;
			fprintf(fp, "video ������ : �v����\n");
			return 0;
		}
		if (!str_ncmp(ptr, "application", strlen("application")))
		{
			hdr->xmode |= MIME_MIME;
			fprintf(fp, "application ������ : �����榡\n");
			return 0;
		}
		hdr->xmode &= (MIME_QP | MIME_BASE64);

		/* �J�� text or image �����ѽX�ʧ@ */
		fgets(src, 256, sock);
		if (*src == '\t') /* Contect-Type �ᦳ boundary & charset,�e�|�� tab */
		{
			return 0;
		}
	}
#endif
	if (!str_ncmp(src, "\tname", strlen("\tname")))
	{
		char *end;
		ptr = strstr(src, "name");
		ptr = strchr(ptr, '"');
		end = strchr(ptr + 1, '"');
		*end = 0;
		fprintf(fp, "\t����W�� : %s\n", pop3_mime_transfer((char *)ptr + 1));
		return 0;
	}
	if (!strncmp(src, "Content-Transfer-Encoding: ", strlen("Content-Transfer-Encoding: ")))
	{
		/* �� rfc1522 body code */
		ptr = src + strlen("Content-Transfer-Encoding: ");
		if (!strncmp(ptr, "base64", strlen("base64")))
		{
			hdr->xmode |= MIME_BASE64;
			hdr->xmode &= ~MIME_QP;
		}
		if (!strncmp(ptr, "quoted-printable", strlen("quoted-printable")))
		{
			hdr->xmode |= MIME_QP;
			hdr->xmode &= ~MIME_BASE64;
		}
		return 0;
	}
	if (hdr->xmode & MIME_MIME)
		return 0;

	return 1; /* �i��ѽX�ʧ@ */
}

/* �ѽX , �� bmtad.c �� multipart() �ܹ�*/
static char*
pop3_mime_transfer(unsigned char* src)
{
	unsigned char* dst = src;
	char *ans;
	register int is_done;
	register int c1, c2, c3, c4;

	/* �Y�S���}�� MIME �ѽX�ﶵ�h�����ѽX */
	if (!is_mime)
		return src;

	ans = src;

	/* �P�_�e 10 �Ӧr���O�_���bbase64 �� */
	for (c1 = 0; c1 < 10 ; c1++)
		if (base64_code(src[c1]) == -1)
		{
			is_base64 = 0;
			break;
		}
	for (is_done = 0; (c1 = *src); src++) /* ��Ƕi�Ӫ��ȧ@�ѽX */
	{
		if (c1 == '?' && src[1] == '=') /* qb code �� To: From: */
		{
			if (src[2] == ' ') /* �q�` qb code �� base64 ���ĤT�Ӥ��|�O�ť� */
			{
				src++;
				is_qp = is_base64 = 0;
			}
			continue;
		}
		else if ((c1 == '=') && (src[1] == '?'))
		{
			/* c2 : qmarks, c3 : code_kind */

			c2 = c3 = 0;

			for (;;)
			{
				c1 = *++src;
				if (c1 != '?')
				{
					if (c2 == 2)
						c3 = c1 | 0x20;
				}
				else
				{
					if (++c2 >= 3)
						break;
				}
			}

			if (c3 == 'q') /* =?q */
				is_qp = MIME_QP;
			else if (c3 == 'b') /* =?big5? */
				is_base64 = MIME_BASE64;
		}
		else if (c1 == '\n')        /* chuan: multi line encoding */
		{
			if (!is_base64)
				*dst++ = c1;
			is_done = is_qp = is_base64 = 0;
			continue;
		}
		else if (is_qp && c1 == '=')
		{
			c1 = *++src;
			if (!*src)
				break;
			c2 = *++src;
			if (!*src)
				break;
			*dst++ = (qp_code(c1) << 4) | qp_code(c2);
			if (dst[-1] == (unsigned char) - 1)
				--dst;
		}
		else if (is_base64 && !is_done)
		{
			while (isspace(c1))
			{
				c1 = *++src;
			}
			if (!c1)
				break;
			do
			{
				c2 = *++src;
			}
			while (isspace(c2));
			if (!c2)
				break;
			do
			{
				c3 = *++src;
			}
			while (isspace(c3));
			if (!c3)
				break;
			do
			{
				c4 = *++src;
			}
			while (isspace(c4));
			if (!c4)
				break;
			if (c1 == '=' || c2 == '=')
			{
				is_done = 1;
				continue;
			}
			c2 = base64_code(c2);
			*dst++ = (base64_code(c1) << 2) | ((c2 & 0x30) >> 4);
			if (c3 == '=')
				is_done = 1;
			else
			{
				c3 = base64_code(c3);
				*dst++ = ((c2 & 0xF) << 4) | ((c3 & 0x3c) >> 2);
				if (c4 == '=')
					is_done = 1;
				else
				{
					*dst++ = ((c3 & 0x03) << 6) | base64_code(c4);
				}
			}
		}
		else
			*dst++ = c1;
	}
	if (!is_base64 && dst[-1] == '=')
		dst[-1] = '\n';
	*dst = '\0';
	return ans;
}

static int
int_len(num)
int num;
{
	int i;
	i = 1;
	while ((num / 10) > 0)
	{
		num /= 10;
		i++;
	}
	return i;
}



/* ��ӷ����X�ݭn������ */
static char *
parse_owner(token)
char *token;
{
	char *ptr;

	/* =?big5?B?qHSyzrresnqt+w==?= <postmaster@mail86.yzu.edu.tw> */

	ptr = strchr(token, '>'); /* > */
	if (ptr) /* �ݳ̫�@�Ӧr���O�_�� > */
	{
		*ptr = 0; /*  */
		ptr = strchr(token, '<'); /* <postmaster@mail86.yzu.edu.tw */
		if (ptr[-1] == ' ')
			ptr[-1] = 0; /* <postmaster@mail86.yzu.edu.tw */
		*ptr++ = 0; /* postmaster@mail86.yzu.edu.tw */
		pop3_mime_transfer(ptr);
	}
	else /* statue.bbs@cnpa-6.admin.yzu.edu.tw */
		ptr = token;

	return ptr;
}

static char *
parse_nick(token)
char *token;
{
	char *ptr;

	/* =?big5?B?qHSyzrresnqt+w==?= <postmaster@mail86.yzu.edu.tw> */

	ptr = token;
	if (*ptr == '=')
	{
		ptr = strchr(token, '<'); /* <postmaster@mail86.yzu.edu.tw> */
		if (ptr[-1] == ' ');
		ptr[-1] = 0; /* <postmaster@mail86.yzu.edu.tw> */
	}
	return token; /* =?big5?B?qHSyzrresnqt+w==?= */
}

/* �q header ���X�ݭn������ */
static void
parse_header(hdr, count)
HDR *hdr;
int count;
{
	char *ptr, *token, buf[256];
	int lines;
	struct tm tmdate;

	lines = -1;
	fprintf(fsock, "TOP %d 0\r\n", count);

	fgets(buf, 256, fsock);

	while (strncmp(buf, ".", 1))
	{
		/* �@�� */
		if (!strncmp(buf, "From: ", strlen("From: ")))
		{
			char tmp[256];
			token = ptr = buf + strlen("From: ");
			strcpy(tmp, token);
			buf[strlen(buf)-2] = 0;
			tmp[strlen(tmp)-2] = 0;

			/* �n���n is_base64 | is_qb */
			snprintf(hdr->nick, 48, "%s", pop3_mime_transfer(parse_nick(token)));
			/* �ӷ��ݨ��X�u�ݭn������ <s874070@mail.yzu.edu.tw> */
			hdr->nick[48] = 0;

			snprintf(hdr->owner, 78, "%s", pop3_mime_transfer(parse_owner(tmp)));
			hdr->owner[78] = 0;
		}

		/* ��� */
		/* Date: Sun, 26 Dec 1999 02:07:46 -0800 */
		/* Date: 04 Jul 2000 07:18:54 GMT */
		if (!strncmp(buf, "Date: ", strlen("Date: ")))
		{
			token = ptr = buf + strlen("Date: ");
			buf[strlen(buf)-2] = 0;

			if (!strstr(token, "GMT"))
				token += 5;
			for (;*token;token++)
			{
				if (*token != ' ')
					break;
			}
			strptime(token, "%d %b %Y", &tmdate);
//      strptime(token, "%a, %d %b %Y", &tmdate);
			snprintf(hdr->date, 9, "%02d/%02d/%02d",
					 (tmdate.tm_year) % 100, tmdate.tm_mon + 1, tmdate.tm_mday);
		}

		/* ���D */
		/* Subject: xxxxxx */
		if (!strncmp(buf, "Subject: ", strlen("Subject: ")))
		{
			token = ptr = buf + strlen("Subject: ");
			buf[strlen(buf)-2] = 0;
			pop3_mime_transfer(ptr);
			strncpy(hdr->title, ptr, 72);
			hdr->title[72] = 0;
		}

		fgets(buf, 256, fsock);
		lines++;
	}
	hdr->xid = lines;
}

static int
pop3_load_dir()
{
	char buf[256], tmp[80];
	int total = 0, step = 0, begin = 1, count;
	HDR hdr;
	/*  int totalsize;*/

	fprintf(fsock, "STAT\r\n");
	fgets(buf, 256, fsock); /* +OK 14 63665 */
	if (!strncmp(buf, "+OK", 3))
	{
		total = atoi(buf + 4);
		/*        count = int_len(total);
		        totalsize = atoi(buf+4+count);
		        sprintf(tmp,"�H��G%d �ʡA�@ %d bytes", total, totalsize);
		        vmsg(tmp);*/
		if (total == 0)
		{
			vmsg("�z���q�l�l��H�c�S���H��C");
			return 1;
		}
		else
			begin = 1;
	}
	else
		return 1;

	sprintf(tmp, "tmp/%s.DIR", cuser.userid); /* DIRECTORY/MBOX */
	//unlink(tmp);
	if ((step = open(tmp, O_WRONLY | O_CREAT | O_TRUNC, 0600)) < 0)
		return 1;
	for (count = begin; count <= total; count++)
	{
		memset(&hdr, 0, sizeof(HDR));
		hdr.xmode = MAIL_READ;
		hdr.chrono = count;
		snprintf(hdr.xname, 32, "%s.mail", cuser.userid);
		parse_header(&hdr, count); // ��Ķ header
		write(step, &hdr, sizeof(HDR)); /* hdr to file */
	}
	close(step);
	return 0;
}

static int
pop3_range_delete(xo)
XO *xo;
{
	char buf[8];
	int head, tail;

	vget(b_lines, 0, "[�]�w�R���d��] �_�I�G", buf, 6, DOECHO);
	head = atoi(buf);
	if (head <= 0)
	{
		zmsg("�_�I���~");
		return XO_FOOT;
	}

	vget(b_lines, 28, "���I�G", buf, 6, DOECHO);
	tail = atoi(buf);
	if (tail < head)
	{
		zmsg("���I���~");
		return XO_FOOT;
	}

	if (vget(b_lines, 41, msg_sure_ny, buf, 3, LCECHO) == 'y')
	{
		del_num = 0;
		hdr_prune(xo->dir, head, tail , 2);
		{
			int pos = 0, fd;
			HDR hdr;
			fd = open(xo->dir, O_RDONLY);
			while (fd)
			{
				lseek(fd, (off_t)(sizeof(HDR) * pos), SEEK_SET);
				if (read(fd, &hdr, sizeof(hdr)) == sizeof(hdr))
				{
					if (hdr.xmode & POST_DELETE)
						del_num++;
					pos++;
				}
				else
					break;
			}
		}
		return XO_LOAD;
	}
	return XO_FOOT;
}

static int
pop3_delete(xo)
XO *xo;
{
	HDR *hdr;

	hdr = (HDR *) xo_pool + (xo->pos - xo->top);

	if (hdr->xmode & MAIL_MARKED)
		return XO_NONE;

	if (!(hdr->xmode & POST_DELETE) ?
		(vans(MSG_DEL_NY) == 'y') : (vans("�нT�w�Ϧ^(Y/N)?[N]") == 'y'))
	{
		if (hdr->xmode & POST_DELETE)
			del_num--;
		else
			del_num++;
		hdr->xmode ^= POST_DELETE;

		/* �R�����峹�۰� reload �ù� */
		if (!rec_put(xo->dir, hdr, sizeof(HDR), xo->pos))
			return XO_LOAD;
	}
	return XO_BODY;
}

static int
pop3_get(fpath, hdr)
char *fpath;
HDR *hdr;
{
	char mail[80], buf[256] , *ptr , *end;
	int start, totalsize, count, ck, page;
	int check_size;
	FILE *fp;
	snprintf(hdr->xname, 32, "%s.mail", cuser.userid);
	sprintf(mail, "tmp/%s.mail", cuser.userid);

	fp = fopen(mail, "w");

	/* GET SIZE*/
	fprintf(fsock, "LIST %d\r\n", (int)hdr->chrono);
	fgets(buf, 256, fsock);
	totalsize = atoi(buf + 5 + int_len(hdr->chrono));

	if (totalsize > MAX_SIZE)
	{
		fprintf(fp, "\n�H��e�q�Ӥj�A�L�k�����C\n");
		fclose(fp);
		return 0;
	}
	count = hdr->xid;

	if (!is_bbsmail)
	{
		fprintf(fp, "�@��: %s\n", hdr->owner); /* �� From �令 �@�� */
		fprintf(fp, "���D: %s\n", hdr->title);
		fprintf(fp, "�ɶ�: %s\n\n", hdr->date);
	}

	/* GET BODY */

	fprintf(fsock, "RETR %d\r\n", (int)hdr->chrono);
	fgets(buf, 256, fsock);

	start = is_bbsmail ? -1 : 1;

	page = 0;
	is_qp = hdr->xmode & MIME_QP;
	is_base64 = hdr->xmode & MIME_BASE64;

	fgets(buf, 256, fsock);
	totalsize = 0;
	boundary[0] = 0;
	page = 1;
	check_size = 0;
	do
	{
		start++;
		ck = pop3_mime_cut(buf, hdr, fp, fsock);
		if (start < count && *boundary == 0 && (ptr = strstr(buf, "boundary")))
		{
			ptr = strchr(buf, '"');
			end = strchr(ptr + 1, '"');
			*end = 0;
			strcpy(boundary, ptr + 1);
		}
		if (ck == -1)
		{
			page = 1;
			hdr->xmode &= ~MIME_MIME;
		}

		is_qp = hdr->xmode & MIME_QP;
		is_base64 = hdr->xmode & MIME_BASE64;
		if (start >= count && page == 1 && ck == 1 && !check_size)
		{
			pop3_mime_transfer(buf);
			fprintf(fp, "%s", buf);
			totalsize += strlen(buf);
			if (totalsize > MAX_SIZE)
			{
				fprintf(fp, "\n�H��e�q�Ӥj�A�L�k�����C\n");
				check_size = 1;
			}
		}
		ptr = fgets(buf, 256, fsock);
	}
	while (strcmp(buf, ".\r\n") && ptr);
	if ((hdr->xmode & (MIME_BASE64 | MIME_QP)) && is_mime)
	{
		fprintf(fp, "\n**�o�O�@�ʸg�L MIME �ѽX�����A�p�����D�гq���t�κ޲z�̡C**\n");
	}
	fclose(fp);
	return 1;
}

static int
pop3_browse(xo)
XO *xo;
{
	char  file[32];
	HDR *hdr;


	hdr = (HDR *) xo_pool + (xo->pos - xo->top);

	sprintf(file, "tmp/%s.mail", cuser.userid);


	if (!pop3_get(file, hdr))
		return XO_NONE;

	strncpy(currtitle, hdr->title, 40); /* title �u���Ϋe 40 �� ? */

	more(file, NULL);
	hdr->xmode |= MAIL_READ;

	return XO_HEAD;
}

static inline int
mbox_attr(type)
int type;
{
	if (type & MAIL_DELETE)
		return 'D';

	if (type & MAIL_REPLIED)
		return (type & MAIL_MARKED) ? 'R' : 'r';

	return "+ Mm"[type & 3];
}

static int
pop3_tag(xo)
XO *xo;
{
	HDR *hdr;
	int tag, pos, cur;

	pos = xo->pos;
	cur = pos - xo->top;
	hdr = (HDR *) xo_pool + cur;

	if ((tag = Tagger(hdr->chrono, pos, TAG_TOGGLE)))
	{
		move(3 + cur, 8);
		outc(tag > 0 ? '*' : ' ');
	}
	return xo->pos + 1 + XO_MOVE;
}

static int
pop3_prune(xo)
XO *xo;
{
	int num;
	char buf[80];

	if (!(num = TagNum))
		return XO_NONE;

	sprintf(buf, "�T�w�n�R�� %d �g���Ҷ�(Y/N)�H[N] ", num);
	if (vans(buf) != 'y')
		return XO_FOOT;

	hdr_prune(xo->dir, 0, 0 , 2);
	{
		int pos = 0, fd;
		HDR hdr;
		del_num = 0;
		fd = open(xo->dir, O_RDONLY);
		while (fd)
		{
			lseek(fd, (off_t)(sizeof(HDR) * pos), SEEK_SET);
			if (read(fd, &hdr, sizeof(hdr)) == sizeof(hdr))
			{
				if (hdr.xmode & POST_DELETE)
					del_num++;
				pos++;
			}
			else
				break;
		}
	}

	TagNum = 0;
	return XO_LOAD;
}

static int
pop3_mime_change()
{
	is_mime ^= 1;
	return XO_HEAD;
}

static int
pop3_mark(xo)
XO *xo;
{
	HDR *mhdr;
	int cur, pos;

	pos = xo->pos;
	cur = pos - xo->top;
	mhdr = (HDR *) xo_pool + cur;
	move(3 + cur, 6);
	outc(mbox_attr(mhdr->xmode ^= MAIL_MARKED));
	rec_put(xo->dir, mhdr, sizeof(HDR), pos);
	return XO_NONE;
}

static void
pop3_item(pos, hdr)
int pos;
HDR *hdr;
{
	int xmode = hdr->xmode;
	prints(xmode & MAIL_DELETE ? "%5d \033[1;5;37;41m%c\033[m"
		   : xmode & MAIL_MARKED ? "%5d \033[1;36m%c\033[m"
		   : "%5d %c", pos, mbox_attr(hdr->xmode));

	hdr_outs(hdr, 47);
}

static int
pop3_body(xo)
XO *xo;
{
	HDR *pop3;
	int num, max, tail;

	max = xo->max;

	if (max <= 0)
	{
		vmsg("�z���q�l�l��H�c�S���H��C");
		return XO_QUIT;
	}

	pop3 = (HDR *) xo_pool;
	num = xo->top;
	tail = num + XO_TALL;
	if (max > tail)
		max = tail;

	move(3, 0);
	do
	{
		pop3_item(++num, pop3++);
	}
	while (num < max);
	clrtobot();

	return XO_NONE;
}

static int
pop3_head(xo)
XO *xo;
{
	vs_head("\0�l����", str_site);
	outs("[��]���} [��,r]Ū�H [d]�R��/�Ϧ^ [S]��Ū [R]�^�H [x]��F [h]����");
	if (is_mime)
		outs(" [MIME �ѽX]");
	else
		outs(" [ �L  �ѽX]");
	outs("\n\033[44m\
		 �s��    �� ��  �@ ��          �H  ��  ��  �D                                 \033[m");

	return pop3_body(xo);
}

static int
pop3_load(xo)
XO *xo;
{
	xo_load(xo, sizeof(HDR));
	return pop3_body(xo);
}

static int
pop3_init(xo)
XO *xo;
{
	xo_load(xo, sizeof(HDR));
	return pop3_head(xo);
}

static int
pop3_help(xo)
XO *xo;
{
	film_out(FILM_MIME, -1);
	return pop3_head(xo);
}

static int
pop3_send(xo)
XO *xo;
{
	m_send();
	return pop3_head(xo);
}

static int
pop3_reply(xo)
XO *xo;
{
	HDR *hdr;

	hdr = (HDR *) xo_pool + (xo->pos - xo->top);

	sprintf(quote_file, "tmp/%s.mail", cuser.userid);
	if (!pop3_get(quote_file, hdr))
		return XO_NONE;

	mail_reply(hdr);
	return XO_HEAD;
}

static int
pop3_forward(xo)
XO *xo;
{
	HDR *hdr, mhdr;
	char fpath[80], rcpt[64], folder[128];
	int userno;

	hdr = (HDR *) xo_pool + (xo->pos - xo->top);
	strcpy(rcpt, cuser.email);
	sprintf(fpath, "tmp/%s.mail", cuser.userid);
	if (!pop3_get(fpath, hdr))
		return XO_NONE;
	if (!vget(b_lines, 0, "�ت��a�G", rcpt, sizeof(rcpt), GCARRY))
		return XO_FOOT;

	if (mail_external(rcpt))
		bsmtp(fpath, hdr->title, rcpt, MQ_MIME);
	else
	{
		if ((userno = acct_userno(rcpt)) <= 0)
			return -1;
		usr_fpath(folder, rcpt, fn_dir);
		hdr_stamp(folder, HDR_LINK, &mhdr, fpath);
		strcpy(mhdr.owner, cuser.userid);
		strcpy(mhdr.nick, cuser.username);
		strcpy(mhdr.title, hdr->title);
		rec_add(folder, &mhdr, sizeof(mhdr));
		m_biff(userno);
	}
	return XO_HEAD;

}

static int
pop3_reload_dir(xo)
XO *xo;
{
	char buf[128];
	fprintf(fsock, "RSET\r\n");

	do
	{
		fgets(buf, 128, fsock);
	}
	while (strncmp(buf, "+OK", 3));

	if (pop3_load_dir())
	{
		fclose(fsock);
		vmsg("��Ū����");
		return XO_QUIT;
	}
	del_num = 0; /* ���]�w�R���峹�� 1 */
	return XO_INIT;
}

static int
pop3_mailsize(xo)
XO *xo;
{
	char buf[128], msg[128], *str;
	HDR *hdr;
	int total, totalsize;

	hdr = (HDR *) xo_pool + (xo->pos - xo->top);
	fprintf(fsock, "LIST %d\r\n", xo->pos + 1);

	do
	{
		str = fgets(buf, 128, fsock);
	}
	while (strncmp(buf, "+OK", 3) && str);

	if (str)
	{
		str = strchr(buf, ' ');
		total = atoi(str);
		str = strchr(str + 1, ' ');
		totalsize = atoi(str);
		sprintf(msg, "�H��G�� %d �ʡA�@ %d bytes", total, totalsize);
		vmsg(msg);
	}
	else
		return XO_HEAD;

	return XO_INIT;
}

static int
pop3_stat(xo)
XO *xo;
{
	char buf[128], msg[128], *str;
	int total, totalsize;
	fprintf(fsock, "STAT\r\n");

	do
	{
		str = fgets(buf, 128, fsock);
	}
	while (strncmp(buf, "+OK", 3) && str);

	if (str)
	{
		str = strchr(buf, ' ');
		total = atoi(str);
		str = strchr(str + 1, ' ');
		totalsize = atoi(str);
		sprintf(msg, "�H��G%d �ʡA�@ %d bytes", total, totalsize);
		vmsg(msg);
	}
	else
		return XO_HEAD;

	return XO_INIT;
}

KeyFunc pop3_cb[] =
{
	{XO_INIT, pop3_init},
	{XO_LOAD, pop3_load},
	{XO_HEAD, pop3_head},
	{XO_BODY, pop3_body},

	{'r', pop3_browse},
	{'d', pop3_delete},
	{'R', pop3_reply},
	{'s', pop3_send},
	{'C', pop3_stat},
	{'c', pop3_mailsize},
	{'t', pop3_tag},
	{'S', pop3_reload_dir},
	{'x', pop3_forward},
	{'m', pop3_mark},
	{'M', pop3_mime_change},
	{Ctrl('D'), pop3_prune},
	{'D', pop3_range_delete},
	{'h', pop3_help}
};

static int
server_login(inputemail)
char *inputemail;
{
	char buf[256], user[60], email[60], pass[20], *host;
	int sock = 110; /* pop3 port */

	more(FN_ETC_MAILSERVICE, (char *) - 1);

	if (inputemail)
		strcpy(email, inputemail);
	else
		strcpy(email, cuser.email); /* �۰��� user �[�W�b���W */

	check_yzuemail(email); /* �� acct.c ���� check_yzumail �ӱN email ����  */

	pass[0] = 0;
	if (!vget(21, 0, "�b���G", email, 60, GCARRY) ||
		!vget(22, 0, "�K�X�G", pass, 20, NOECHO))
	{
		/* ���@���S��J�h��� */
		return -1;
	}

	if (!strchr(email, '@'))
	{
		vmsg("��J�b�����~");
		return 1;
	}

	strcpy(user, email);
	*(strchr(user, '@')) = 0;
	host = strchr(email, '@') + 1;

	if (Get_Socket(host, &sock))
	{
		vmsg("�D���L�k�s�u");
		return 1;
	}

	fsock = fdopen(sock, "r+");
	fgets(buf, 256, fsock);		/* Welcome */
	fprintf(fsock, "USER %s\r\n", user);	/* USER NAME */
	fgets(buf, 256, fsock);

	if (!strncmp(buf, "+OK", 3))
	{
		is_bbsmail = strstr(buf, ".bbs@") ? 1 : 0;
		fprintf(fsock, "PASS %s\r\n", pass);  /* PASSWORD */
		fgets(buf, 256, fsock);
		if (strncmp(buf, "+OK", 3))
		{
			logitfile(FN_MAILSERVICE_LOG, "-pop3  NO-", email);       /* �n�J���ѰO�� */
			vmsg("�n�J����, �K�X���~.");
			fclose(fsock);
			return 1;
		}
		else
		{
			logitfile(FN_MAILSERVICE_LOG, "-pop3  OK-", email);       /* �n�J���\�O�� */
		}
	}
	else
	{
		logitfile(FN_MAILSERVICE_LOG, "-pop3  NO-", email);       /* �n�J���ѰO�� */
		vmsg("�n�J����, �L���b��.");
		fclose(fsock);
		return 1;
	}

	if (pop3_load_dir())
	{
		fclose(fsock);
		return 1;
	}
	return 0;
}

static int
server_logout(xo)
XO *xo;
{
	if (del_num && vans(msg_del_ny) == 'y')
	{
		int pos = 0, fd;
		HDR hdr;
		char buf[200];
		fd = open(xo->dir, O_RDONLY);
		while (fd)
		{
			lseek(fd, (off_t)(sizeof(HDR) * pos), SEEK_SET);
			if (read(fd, &hdr, sizeof(hdr)) == sizeof(hdr))
			{
				if (hdr.xmode & POST_DELETE)
				{
					fprintf(fsock, "DELE %d\r\n", (int)hdr.chrono);
					while (1)
						if (!strncmp(fgets(buf, 200, fsock), "+OK", 3))/* purge messages */
							break;
				}
				pos++;
			}
			else
				break;
		}
		close(fd);
	}
	fprintf(fsock, "QUIT\r\n");
	fclose(fsock);
	return 0;
}

int
Pop3mail()
{
	XO *xo;
	char currsmaildir[64];

	del_num = is_base64 = is_qp = 0; /* �w�]�w�R���峹�Ƭ� 0, �L�ѽX�覡 */

	is_mime = (cuser.ufo2 & UFO2_MIME) ? 1 : 0; /* �Y�L UFO_MIME �h�N�o����� */

	sprintf(currsmaildir, "tmp/%s.DIR", cuser.userid); /*DIRECTORY/MBOX*/

	/* by statue */
	if (server_login(NULL))
	{
		return 0;
	}

	utmp_mode(M_MIME); /* ���m���A���] */
	xz[XZ_OTHER - XO_ZONE].xo = xo = xo_new(currsmaildir); /* �]�� .DIR ���| */
	xz[XZ_OTHER - XO_ZONE].cb = pop3_cb;
	xover(XZ_OTHER);

	server_logout(xo);

	free(xo);

	sprintf(currsmaildir, "tmp/%s.DIR", cuser.userid);
	unlink(currsmaildir);
	sprintf(currsmaildir, "tmp/%s.DIR.o", cuser.userid);
	unlink(currsmaildir);
	sprintf(currsmaildir, "tmp/%s.mail", cuser.userid);
	unlink(currsmaildir);

	return 0;
}

int
Pop3Contact(email)
char *email;
{
	XO *xo;
	char currsmaildir[64];

	del_num = is_base64 = is_qp = 0; /* �w�]�w�R���峹�Ƭ� 0, �L�ѽX�覡 */

	is_mime = (cuser.ufo2 & UFO2_MIME) ? 1 : 0; /* �Y�L UFO_MIME �h�N�o����� */

	sprintf(currsmaildir, "tmp/%s.DIR", cuser.userid); /*DIRECTORY/MBOX*/

	if (server_login(email))
	{
		return 0;
	}

	utmp_mode(M_MIME); /* ���m���A���] */
	xz[XZ_OTHER - XO_ZONE].xo = xo = xo_new(currsmaildir); /* �]�� .DIR ���| */
	xz[XZ_OTHER - XO_ZONE].cb = pop3_cb;
	xover(XZ_OTHER);

	server_logout(xo);

	free(xo);

	sprintf(currsmaildir, "tmp/%s.DIR", cuser.userid);
	unlink(currsmaildir);
	sprintf(currsmaildir, "tmp/%s.DIR.o", cuser.userid);
	unlink(currsmaildir);
	sprintf(currsmaildir, "tmp/%s.mail", cuser.userid);
	unlink(currsmaildir);


	return 0;
}
