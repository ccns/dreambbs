/*-------------------------------------------------------*/
/* yzuemail.c	( YZU WindTop BBS )			 */
/*-------------------------------------------------------*/
/* author : visor.bbs@bbs.yzu.edu.tw			 */
/* author : statue.bbs@bbs.yzu.edu.tw			 */
/* target : mime transfer			 	 */
/* create : 99/12/26				 	 */
/* update : 2000/01/02				 	 */
/*-------------------------------------------------------*/

#undef	_YZUEMAIL_C_

#include "bbs.h"
#ifdef HAVE_MIME_TRANSFER
/*#include "get_socket.c"*/
extern XZ xz[];

static FILE *fsock; /* 連線用的 FILE */
static int is_mime; /* 切換 MIME 用的旗標 */
static int is_base64;
static int is_qp;
#define MIME_BASE64	0x01
#define MIME_QP		0x02
#define MIME_MIME       0x10

static int del_num; /* 用來 UnDelete 用 , 已刪除文章數*/

extern int TagNum;

/* ----------------------------------------------------- */
/* QP code : "0123456789ABCDEF"                          */
/* QP 是一種解碼方式 quoted-printable			 */
/* 一篇文章中的 '=' 有沒有超過一定數目.			 */
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
/* Base64無法判斷, 除非我們有header				      */
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

/* 去除不必要的標頭 */
static int
mime_cut(unsigned char* src, HDR *hdr, FILE *fp)
{
	char *ptr;

	/* 若沒有開啟 MIME 解碼選項則不予解碼 */
	if (!is_mime)
		return 1;

	if (!strncmp(src, "----", strlen("----")) ||
		!strncmp(src, "\tboundary", strlen("\tboundary")))
		return 0; /* 跳過此部份解碼 */

	if (!strncmp(src, "Content-Type: ", strlen("Content-Type: ")))
	{
		ptr = src + strlen("Content-Type: ");
		if (!strncmp(ptr, "text/html", strlen("text/html")))
		{
			hdr->xid |= MIME_MIME;

			fprintf(fp, "html 有網頁格式\n");
			return 0;
		}
		/*if(!strncmp(ptr, "text/plain",strlen("text/plain")))
		{
		  hdr->xid |= MIME_MIME;
		  fprintf(fp, "text/plain 有附件 : 純文字檔\n");
		}*/
		if (!strncmp(ptr, "image", strlen("image")))
		{
			hdr->xid |= MIME_MIME;
			fprintf(fp, "image 有附件 : 圖片檔\n");
			return 0;
		}
		if (!strncmp(ptr, "audio", strlen("audio")))
		{
			hdr->xid |= MIME_MIME;
			fprintf(fp, "audio 有附件 : 音效檔\n");
			return 0;
		}
		if (!strncmp(ptr, "video", strlen("video")))
		{
			hdr->xid |= MIME_MIME;
			fprintf(fp, "video 有附件 : 影音檔\n");
			return 0;
		}
		if (!strncmp(ptr, "application", strlen("application")))
		{
			hdr->xid |= MIME_MIME;
			fprintf(fp, "application 有附件 : 不明格式\n");
			return 0;
		}
		hdr->xid &= MIME_QP | MIME_BASE64;

		/* 遇到 text or image 結束解碼動作 */
		fgets(src, 256, fsock);
		if (*src == '\t') /* Contect-Type 後有 boundary & charset,前會有 tab */
		{
			return 0;
		}
	}

	if (!strncmp(src, "Content-Transfer-Encoding: ", strlen("Content-Transfer-Encoding: ")))
	{
		/* 解 rfc1522 body code */
		ptr = src + strlen("Content-Transfer-Encoding: ");
		if (!strncmp(ptr, "base64", strlen("base64")))
		{
			hdr->xid |= MIME_BASE64;
			hdr->xid &= ~MIME_QP;
		}
		if (!strncmp(ptr, "quoted-printable", strlen("quoted-printable")))
		{
			hdr->xid |= MIME_QP;
			hdr->xid &= ~MIME_BASE64;
		}
		return 0;
	}
	if (hdr->xid & MIME_MIME)
		return 0;

	return 1; /* 進行解碼動作 */
}

/* 解碼 , 跟 bmtad.c 的 multipart() 很像*/
static char*
mime_transfer(unsigned char* src)
{
	unsigned char* dst = src;
	char *ans;
	register int is_done;
	register int c1, c2, c3, c4;

	/* 若沒有開啟 MIME 解碼選項則不予解碼 */
	if (!is_mime)
		return src;

	ans = src;

	/* 判斷前 10 個字元是否都在base64 中 */
	for (c1 = 0; c1 < 10 ; c1++)
		if (base64_code(src[c1]) == -1)
		{
			is_base64 = 0;
			break;
		}

	for (is_done = 0; (c1 = *src); src++) /* 把傳進來的值作解碼 */
	{
		if (c1 == '?' && src[1] == '=') /* qb code 的 To: From: */
		{
			if (src[2] == ' ') /* 通常 qb code 或 base64 的第三個不會是空白 */
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

/* 把來源取出需要的部分 */
static char *
parse_owner(token)
char *token;
{
	char *ptr;

	/* =?big5?B?qHSyzrresnqt+w==?= <postmaster@mail86.yzu.edu.tw> */

	ptr = strchr(token, '>'); /* > */
	if (ptr) /* 看最後一個字元是否為 > */
	{
		*ptr = 0; /*  */
		ptr = strchr(token, '<'); /* <postmaster@mail86.yzu.edu.tw */
		if (ptr[-1] == ' ')
			ptr[-1] = 0; /* <postmaster@mail86.yzu.edu.tw */
		*ptr++ = 0; /* postmaster@mail86.yzu.edu.tw */
	}
	else /* statue.bbs@cnpa-6.admin.yzu.edu.tw */
		ptr = token;

	return ptr;
}

/* 從 header 取出需要的部分 */
static void
parse_header(hdr, ptr)
HDR *hdr;
char *ptr;
{
	char *token, buf[256];
	struct tm tmdate;

	sprintf(buf, "%s.mail", cuser.userid);
	snprintf(hdr->xname, 32, "%s", buf);

	strcpy(buf, ptr);

	hdr->xid = 0; /* 預設不用 qp | base64 解碼 */
	/* From: */
	/* Content-Type: multipart/alternative; */
	/* Content-Type: text/plain; charset="big5" */
	/*         boundary="----=_NextPart_000_0007_01BF4F46.00513410" */
	/* Content-Transfer-Encoding: base64 */

	/* 要求資料的結尾是 ) 的字元 , 不知道有沒有比較好的做法 statue.1227*/
	while (*buf != ')')
	{
		fgets(buf, 256, fsock);
		if (!strncmp(buf, "Content-Type: ", strlen("Content-Type: ")))
		{
			ptr = buf + strlen("Content-Type: ");
			if (!strncmp(ptr, "multipart", strlen("multipart")))
				hdr->xid |= MIME_QP;
		}

		if (!strncmp(buf, "Content-Transfer-Encoding: ", strlen("Content-Transfer-Encoding: ")))
		{
			ptr = buf + strlen("Content-Transfer-Encoding: ");
			if (!strncmp(ptr, "quoted-printable", strlen("quoted-printable")))
				hdr->xid |= MIME_QP;
			if (!strncmp(ptr, "base64", strlen("base64")))
				hdr->xid |= MIME_BASE64;
		}

		/* 作者 */
		/* From: =?big5?B?qEirVL+z?= <s874070@mail.yzu.edu.tw> */
		if (!strncmp(buf, "From: ", strlen("From: ")))
		{
			char tmp[256];
			token = ptr = buf + strlen("From: ");
			strcpy(tmp, token);
			buf[strlen(buf)-2] = 0;

			/* 要不要 is_base64 | is_qb */
			snprintf(hdr->nick, 48, "%s", mime_transfer(parse_nick(token)));
			/* 來源需取出只需要的部分 <s874070@mail.yzu.edu.tw> */
			hdr->nick[48] = 0;
			mime_transfer(tmp);
			snprintf(hdr->owner, 78, "%s", parse_owner(tmp));
			hdr->owner[78] = 0;
		}

		/* 日期 */
		/* Date: Sun, 26 Dec 1999 02:07:46 -0800 */
		/* Date: 04 Jul 2000 07:18:54 GMT */
		if (!strncmp(buf, "Date: ", strlen("Date: ")))
		{
			token = ptr = buf + strlen("Date: ");
			buf[strlen(buf)-2] = 0;

			if (!strstr(token, "GMT"))
				token += 5;
			strptime(token, "%d %b %Y", &tmdate);
//    strptime(token, "%a, %d %b %Y", &tmdate);
			snprintf(hdr->date, 9, "%02d/%02d/%02d",
					 (tmdate.tm_year) % 100, tmdate.tm_mon + 1, tmdate.tm_mday);
		}

		/* 標題 *//* Subject: xxxxxx */
		if (!strncmp(buf, "Subject: ", strlen("Subject: ")))
		{
			token = ptr = buf + strlen("Subject: ");
			buf[strlen(buf)-2] = 0;

			is_base64 = hdr->xid & MIME_BASE64;
			is_qp = hdr->xid & MIME_QP;
			mime_transfer(ptr); /* 標題需解碼 */

			strncpy(hdr->title, token, 72);
			hdr->title[72] = 0;
		}
	}
}

static int
load_dir()
{
	char buf[256];
	int total, count, step = 0, begin = 1;
	HDR *hdr;
	char smaildir[80];

	total = 0;
	hdr = NULL;

	/* * 4 EXISTS */
	/* * 0 RECENTM */
	/* * FLAGS (\Seen \Answered \Flagged \Deleted \Draft) */
	/* * OK [UIDVALIDITY 16097] UIDVALIDITY value. */
	/* ~ OK [READ-WRITE] SELECT completed. */
	strcpy(buf, "~ SELECT INBOX\r\n");
	fprintf(fsock, buf);
	while (strncmp(buf, "~ OK", 3))
	{
		fgets(buf, 256, fsock);
		if (strstr(buf, "EXISTS"))              /* meet system message */
		{
			if (!total)
				total = atoi(buf + 2);
			if (total >= 256) /* 如果信件量大於 256 篇 , 只讀後 256 篇*/
				begin = total - 255;
			else if (total == 0)
			{
				vmsg("您的電子郵件信箱沒有信件");
				return 1;
			}
			else
				begin = 1;
		}
	}

	/* * 1 FETCH (FLAGS (\Seen)) */
	/* * 2 FETCH (FLAGS (\Seen)) */
	/* ~ OK FETCH completed. */
	count = 0;
	hdr = (HDR *)malloc((total - begin + 1) * sizeof(HDR));    /* memory */
	if (!hdr)                                /* fail */
		return 1;
	memset(hdr, 0, ((total - begin + 1) * sizeof(HDR)));
	snprintf(buf, 256, "~ FETCH %d:%d FLAGS\r\n", begin, total);
	fprintf(fsock, buf);
	while (strncmp(buf, "~ OK", 3))
	{
		fgets(buf, 256, fsock);
		if (strstr(buf, "Seen")) /* * 1 FETCH (FLAGS (\Seen)) */
			hdr[count].xmode = MAIL_READ;
		hdr[count].chrono = begin + count;          /* mail number */
		count++;                             /* 先借用 HDR.chrono 用一下 */
	}

	/* * 1 FETCH (RFC822.HEADER {103} *//* 抓 HEADER lo */
	/* * 2 FETCH (RFC822.HEADER {103} */
	/* ~ OK FETCH completed. */
	count = 0;
#if 1
	snprintf(buf, 256, "~ FETCH %d:%d RFC822.HEADER.LINES \
			 (CONTENT-TYPE CONTENT-TRANSFER-ENCODING FROM DATE SUBJECT)\r\n", begin, total);
#else
	snprintf(buf, 256, "~ FETCH %d:%d RFC822.HEADER.LINES \
			 (FROM DATE SUBJECT)\r\n", begin, total);
#endif
	fprintf(fsock, buf);
	while (strncmp(buf, "~ OK", 3))
	{
		fgets(buf, 256, fsock);
		if (*buf == '*')
		{
			parse_header(&hdr[count], buf);
			count++;
		}
	}

	sprintf(smaildir, "tmp/%s.DIR", cuser.userid);   /* DIRECTORY/MBOX */
	unlink(smaildir);
	if ((step = open(smaildir, O_WRONLY | O_CREAT | O_TRUNC, 0600)) < 0)
		return 1;
	write(step, hdr, sizeof(HDR) * (total - begin + 1));  /* hdr to file */
	close(step);

	return 0;
}

static int
imap4_range_delete(xo)
XO *xo;
{
	char buf[8];
	int head, tail;

	vget(b_lines, 0, "[設定刪除範圍] 起點：", buf, 6, DOECHO);
	head = atoi(buf);
	if (head <= 0)
	{
		zmsg("起點有誤");
		return XO_FOOT;
	}

	vget(b_lines, 28, "終點：", buf, 6, DOECHO);
	tail = atoi(buf);
	if (tail < head)
	{
		zmsg("終點有誤");
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
imap4_delete(xo)
XO *xo;
{
	HDR *hdr;

	hdr = (HDR *) xo_pool + (xo->pos - xo->top);

	if (hdr->xmode & MAIL_MARKED)
		return XO_NONE;

	if (!(hdr->xmode & POST_DELETE) ?
		(vans(MSG_DEL_NY) == 'y') : (vans("請確定救回(Y/N)?[N]") == 'y'))
	{
		if (hdr->xmode & POST_DELETE)
			del_num--;
		else
			del_num++;
		hdr->xmode ^= POST_DELETE;

		/* 刪除完文章自動 reload 螢幕 */
		if (!rec_put(xo->dir, hdr, sizeof(HDR), xo->pos))
			return XO_LOAD;
	}
	return XO_BODY;
}

static int
imap4_get(fpath, hdr)
char *fpath;
HDR *hdr;
{
	char buf[256];
	char *ptr;
	FILE *fp;
	int total = 0, now, ck, OK_FETCH, check_size, count;
	/* ck 是做 mime transfer 的 check */
	/* OK_FETCH 是把後面的文章抓完, 直到 ~ OK FETCH completed., 檔案結尾*/

	check_size = 0;
	count = 0;

	if ((fp = fopen(fpath, "w")) == NULL)
		return 0;

	/* GET HEADER, 因 HDR 資料長度不夠, 必須從抓一次 */
	fprintf(fsock, "~ FETCH %d RFC822.HEADER.LINES (FROM DATE)\r\n",
			(int)hdr->chrono);
	fgets(buf, 256, fsock);
	if (strstr(buf, " EXIST\r\n"))
	{
		fgets(buf, 256, fsock);
		fgets(buf, 256, fsock);
	}
	fgets(buf, 256, fsock);
	buf[strlen(buf)-2] = 0;

	is_qp = hdr->xid & MIME_QP;
	is_base64 = hdr->xid & MIME_BASE64;
	mime_transfer(buf);

	strncpy(buf, "作者", 4); /* 作者的暱稱可能有加密動作 */
	fprintf(fp, "%s\n", buf); /* 把 From 改成 作者 */
	fprintf(fp, "標題: %s\n", hdr->title);
	fgets(buf, 256, fsock);
	if (*buf != '\r')
	{
		strncpy(buf, "時間", 4);
		fprintf(fp, "%s\n\n", buf);
	}
	else
		fprintf(fp, "時間: 不詳\n\n");
	/* From: "s874070" <s874070@mail.yzu.edu.tw> */
	/* Date: Sun, 26 Dec 1999 02:07:46 -0800*/


	/* GET BODY */
	fprintf(fsock, "~ FETCH %d RFC822.TEXT\r\n", (int)hdr->chrono);
	fgets(buf, 256, fsock);
	/* * 1 FETCH (RFC822.TEXT {624} */
	if (strstr(buf, " EXIST\r\n"))
	{
		fgets(buf, 256, fsock);
		fgets(buf, 256, fsock);
	}
	if ((ptr = strrchr(buf, '{')))
		total = atoi(ptr + 1) + 1;                  /* total = size */

	if (total > 10485760)
		check_size = 1;


	now = 0;
	OK_FETCH = 1; /* 抓到文章結尾 */
	while (1)
	{
		if (now >= total)
		{
			OK_FETCH = 0;
			break;
		}
		fgets(buf, 256, fsock);

		if (strstr(buf, "~ OK FETCH completed"))
		{
			OK_FETCH = 0;
			ck = -1;
			break;
		}

		now += strlen(buf);
		ck = mime_cut(buf, hdr, fp); /* 去掉不必要的標頭 */

		if (ck == 1) /* 以 mime_cut 的傳回直判斷是否需要解碼 */
		{

			is_qp = hdr->xid & MIME_QP;
			is_base64 = hdr->xid & MIME_BASE64;
			mime_transfer(buf); /* 解碼動作 */
			count += strlen(buf);

			if (count > 10485760)
				check_size = 1;

			if (!check_size)
				fprintf(fp, "%s", buf);
		}
		else if (ck == -1)
			break;
	}

	while (OK_FETCH && fgets(buf, 256, fsock))
	{
		if (strstr(buf, "~ OK FETCH completed"))
			break;
	}

	if ((hdr->xmode & (MIME_BASE64 | MIME_QP)) && is_mime)
	{
		fprintf(fp, "\n**這是一封經過 MIME 解碼的文件，如有問題請通知系統管理者。**\n");
	}

	if (check_size)
	{
		fprintf(fp, "\n信件容量太大，無法收取。\n");
	}

	fclose(fp);
	return 1;
}

static int
imap4_browse(xo)
XO *xo;
{
	char  file[32];
	HDR *hdr;

	hdr = (HDR *) xo_pool + (xo->pos - xo->top);

	sprintf(file, "tmp/%s.mail", cuser.userid);             /* Tsaoyc.mail */

	if (!imap4_get(file, hdr))
		return XO_NONE;

	strncpy(currtitle, hdr->title, 40); /* title 只取用前 40 個 ? */

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
imap4_tag(xo)
XO *xo;
{
	HDR *hdr;
	int tag, pos, cur;

	pos = xo->pos;
	cur = pos - xo->top;
	hdr = (HDR *) xo_pool + cur;

	if (tag = Tagger(hdr->chrono, pos, TAG_TOGGLE))
	{
		move(3 + cur, 8);
		outc(tag > 0 ? '*' : ' ');
	}
	return xo->pos + 1 + XO_MOVE;
}

static int
imap4_prune(xo)
XO *xo;
{
	int num;
	char buf[80];

	if (!(num = TagNum))
		return XO_NONE;

	sprintf(buf, "確定要刪除 %d 篇標籤嗎(Y/N)？[N] ", num);
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
imap4_mime_change()
{
	is_mime ^= 1;
	return XO_HEAD;
}

static int
imap4_mark(xo)
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
imap4_item(pos, hdr)
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
imap4_body(xo)
XO *xo;
{
	HDR *mime;
	int num, max, tail;

	max = xo->max;

	if (max <= 0)
	{
		vmsg("您的電子郵件信箱沒有信件");
		return XO_QUIT;
	}

	mime = (HDR *) xo_pool;
	num = xo->top;
	tail = num + XO_TALL;
	if (max > tail)
		max = tail;

	move(3, 0);
	do
	{
		imap4_item(++num, mime++);
	}
	while (num < max);
	clrtobot();

	return XO_NONE;
}

static int
imap4_head(xo)
XO *xo;
{
	vs_head("\0郵件選單", str_site);
	outs("[←]離開 [→,r]讀信 [d]刪除/救回 [S]重讀 [R]回信 [x]轉達 [h]說明");
	if (is_mime)
		outs(" [MIME 解碼]");
	else
		outs(" [ 無  解碼]");
	outs("\n\033[44m\
		 編號   日 期  作 者          信  件  標  題                                  \033[m");

	return imap4_body(xo);
}

static int
imap4_load(xo)
XO *xo;
{
	xo_load(xo, sizeof(HDR));
	return imap4_body(xo);
}

static int
imap4_init(xo)
XO *xo;
{
	xo_load(xo, sizeof(HDR));
	return imap4_head(xo);
}

static int
imap4_help(xo)
XO *xo;
{
	film_out(FILM_MIME, -1);
	return imap4_head(xo);
}

static int
imap4_send(xo)
XO *xo;
{
	sprintf(quote_file, "tmp/%s.mail", cuser.userid);
	m_send();
	return imap4_head(xo);
}

static int
imap4_reply(xo)
XO *xo;
{
	HDR *hdr;
	hdr = (HDR *) xo_pool + (xo->pos - xo->top);
	sprintf(quote_file, "tmp/%s.mail", cuser.userid);
	if (!imap4_get(quote_file, hdr))
		return XO_NONE;
	mail_reply(hdr);
	*quote_file = '\0';
	return XO_HEAD;
}

static int
imap4_forward(xo)
XO *xo;
{
	HDR *hdr, mhdr;
	char fpath[80], rcpt[64], folder[128];
	int userno;

	hdr = (HDR *) xo_pool + (xo->pos - xo->top);
	strcpy(rcpt, cuser.email);
	sprintf(fpath, "tmp/%s.mail", cuser.userid);
	if (!imap4_get(fpath, hdr))
		return XO_NONE;
	if (!vget(b_lines, 0, "目的地：", rcpt, sizeof(rcpt), GCARRY))
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
imap4_load_dir(xo)
XO *xo;
{
	if (load_dir())
	{
		fclose(fsock);
		vmsg("重讀失敗");
		return XO_QUIT;
	}
	del_num = 0; /* 重設已刪除文章為 1 */
	return XO_INIT;
}

KeyFunc imap4_cb[] =
{
	{XO_INIT, imap4_init},
	{XO_LOAD, imap4_load},
	{XO_HEAD, imap4_head},
	{XO_BODY, imap4_body},

	{'r', imap4_browse},
	{'d', imap4_delete},
	{'R', imap4_reply},
	{'s', imap4_send},
	{'t', imap4_tag},
	{'S', imap4_load_dir},
	{'x', imap4_forward},
	{'m', imap4_mark},
	{'M', imap4_mime_change},
	{Ctrl('D'), imap4_prune},
	{'D', imap4_range_delete},
	{'h', imap4_help}
};

static int
server_login(xo)
{
	char buf[256], email[60], pass[20], user[60], *host;
	int sock = 143; /* imap4 port */

	more(FN_ETC_MAILSERVICE, (char *) - 1);

	strcpy(email, cuser.email); /* 自動幫 user 加上帳號名 */

	check_yzuemail(email); // 用 acct.c 中的 check_yzumail 來將 email 改變

	if (!vget(21, 0, "帳號: ", email, 60, GCARRY) ||
		!vget(22, 0, "密碼: ", pass, 20, NOECHO))
	{
		/* 有一項沒輸入則放棄 */
		return -1;
	}

	if (!strchr(email, '@'))
	{
		vmsg("輸入帳號有誤");
		return 1;
	}

	strcpy(user, email);
	*(strchr(user, '@')) = 0;
	host = strchr(email, '@') + 1;

	if (Get_Socket(host, &sock))
	{
		vmsg("主機無法連線");
		return 1;
	}

	fsock = fdopen(sock, "r+");
	fgets(buf, 256, fsock);                                  /* Welcome */
	fprintf(fsock, "BBS LOGIN %s \"%s\"\r\n", user, pass);   /* LOGIN */

	fgets(buf, 256, fsock);
	/* BBS OK LOGIN completed.*/
	if (strncmp(buf, "BBS OK", 6))                        /* Verify */
	{
		logitfile(FN_MAILSERVICE_LOG, "-imap4 NO-", email);            /* 登入失敗記錄 */
		vmsg("登入失敗");
		fclose(fsock);
		return 1;
	}
	else
		logitfile(FN_MAILSERVICE_LOG, "-imap4 OK-", email);            /* 登入成功記錄 */

	if (load_dir())
	{
		fclose(fsock);
		return 1;
	}
	return 0;
}

static void
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
					fprintf(fsock, "~ STORE %d +FLAGS (\\Deleted)\r\n", (int)hdr.chrono);
					while (1)
						if (!strncmp(fgets(buf, 200, fsock), "~ OK", 3))/* purge messages */
							break;
				}
				pos++;
			}
			else
				break;
		}
		close(fd);
		fprintf(fsock, "BBS CLOSE\r\nBBS LOGOUT\r\n");
		fclose(fsock);
	}
}

int
Imap4mail()
{
	XO *xo;
	char currsmaildir[30];

	if (server_login())
		return 0;

	del_num = is_qp = is_base64 = 0; /* 預設已刪除文章數為 0, 無解碼方式 */

	is_mime = (cuser.ufo2 & UFO2_MIME) ? 1 : 0; /* 若無 UFO_MIME 則將這行取消 */

	sprintf(currsmaildir, "tmp/%s.DIR", cuser.userid);   /* DIRECTORY/MBOX */

	utmp_mode(M_MIME); /* 閒置狀態重設 */
	xz[XZ_OTHER - XO_ZONE].xo = xo = xo_new(currsmaildir); /* 設取 .DIR 路徑 */
	xz[XZ_OTHER - XO_ZONE].cb = imap4_cb;
	xover(XZ_OTHER);

	server_logout(xo);

	free(xo);
	return 0;
}

#endif
