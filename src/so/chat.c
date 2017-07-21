/*-------------------------------------------------------*/
/* chat.c	( NTHU CS MapleBBS Ver 2.36 )		 */
/*-------------------------------------------------------*/
/* target : chat client for xchatd			 */
/* create : 95/03/29					 */
/* update : 99/12/21					 */
/*-------------------------------------------------------*/


#include "bbs.h"


#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>


static char chatroom[IDLEN];	/* Chat-Room Name */
static int chatline;		/* Where to display message now */
static char chatopic[48];
static FILE *frec;
#ifdef	LOG_CHAT
static FILE *fmail;
#endif

#define	stop_line	(b_lines - 2)


extern char *bmode();

#ifdef EVERY_Z
extern int vio_fd, holdon_fd;
#endif


static void
chat_topic()
{
	move(0, 0);
	prints("[1;37;46m %s�G%-12s[45m ���D�G%-48s[m",
		   (frec ? " ������ " : CHATROOMNAME), chatroom, chatopic);
}


static void
printchatline(msg)
char *msg;
{
	int line;

	line = chatline;
	move(line, 0);
	outs(msg);
	outc('\n');

	if (frec)
		fprintf(frec, "%s\n", msg);

	if (fmail)
		fprintf(fmail, "%s\n", msg);

	if (++line == stop_line)
		line = 2;
	move(line, 0);
	outs("��");
	clrtoeol();
	chatline = line;
}


static void
chat_record()
{
	FILE *fp;
	time_t now;
	char buf[80];

	if (!cuser.userlevel)
		return;

	time(&now);

	if ((fp = frec))
	{
		fprintf(fp, "%s\n�����G%s\n", msg_seperator, Ctime(&now));
		fclose(fp);
		frec = NULL;
		printchatline("�� ���������I");
	}
	else
	{
		/* Thor.980602: �ѩ� tbf_ask�ݰ��ɦW, ���ɷ|�Ψ�igetch,
		                ���� I_OTHERDATA�y�����, �b���� ctrlZ_everywhere�覡,
		                �O�svio_fd, �ݰݧ���A�٭� */

		/* Thor.980602: �Ȧs vio_fd */
		holdon_fd = vio_fd;
		vio_fd = 0;

		usr_fpath(buf, cuser.userid, tbf_ask());

		/* Thor.980602: �٭� vio_fd */
		vio_fd = holdon_fd;
		holdon_fd = 0;


		move(b_lines, 0);
		clrtoeol();

		fp = fopen(buf, "a");
		if (fp)
		{
			fprintf(fp, "�D�D: %s\n�]�[: %s\n����: %s (%s)\n�}�l: %s\n%s\n",
					chatopic, chatroom, cuser.userid, cuser.username,
					Ctime(&now), msg_seperator);
			printchatline("�� �}�l�����o�I");
			frec = fp;
		}
		else
		{
			printchatline("�� �������G�٤F�A�гq����������");
		}
	}
	bell();
	chat_topic();
}

#ifdef	LOG_CHAT
static void
chat_recordtomail(mode)
int mode;
{
	time_t now;
	char buf[80];

	if (!cuser.userlevel)
		return;

	time(&now);
	usr_fpath(buf, cuser.userid, FN_UCHAT_LOG);


	if (!mode)
	{
		if (fmail)
		{
			fprintf(fmail, "%s\n�����G%s\n", msg_seperator, Ctime(&now));
			fclose(fmail);
			fmail = NULL;
			holdon_fd = vio_fd;
			vio_fd = 0;
			switch (vans("������ѫǬ����B�z (M)�Ƨѿ� (C)�M���H[C] "))
			{
		case 'm': case 'M':
				{
					char tmp[64];
					HDR fhdr;
					usr_fpath(tmp, cuser.userid, fn_dir);
					hdr_stamp(tmp, HDR_LINK, &fhdr, buf);
					strcpy(fhdr.title, "[�� �� ��] ��ѫǬ���");
					strcpy(fhdr.owner, cuser.userid);
					fhdr.xmode = MAIL_READ | MAIL_NOREPLY;
					rec_add(tmp, &fhdr, sizeof(fhdr));
				}
			default:
				unlink(buf);
			}
			vio_fd = holdon_fd;
			holdon_fd = 0;
		}
	}
	else
	{

		fmail = fopen(buf, "a");
		if (fmail)
		{
			fprintf(fmail, "��ѫǿ���: %s (%s)\n�}�l: %s\n%s\n",
					cuser.userid, cuser.username, Ctime(&now), msg_seperator);
		}
		else
		{
			fmail = NULL;
		}
	}
}
#endif


static void
chat_clear()
{
	int line;

	for (line = 2; line < stop_line; line++)
	{
		move(line, 0);
		clrtoeol();
	}
	move(b_lines, 0);
	clrtoeol();
	chatline = stop_line - 1;
	printchatline("");
}


static void
print_chatid(chatid)
char *chatid;
{
	move(b_lines - 1, 0);
	outs(chatid);
	outc(':');
}


static inline int
chat_send(fd, buf)
int fd;
char *buf;
{
	int len;

	len = strlen(buf);
	return (send(fd, buf, len, 0) == len);
}


static inline int
chat_recv(fd, chatid)
int fd;
char *chatid;
{
	static char buf[512];
	static int bufstart = 0;
	int cc, len;
	char *bptr, *str;

	bptr = buf;
	cc = bufstart;
	len = sizeof(buf) - cc - 1;
	if ((len = recv(fd, bptr + cc, len, 0)) <= 0)
		return -1;
	cc += len;

	for (;;)
	{
		len = strlen(bptr);

		if (len >= cc)
		{
			/* wait for trailing data */
			memcpy(buf, bptr, len);
			bufstart = len;
			break;
		}
		if (*bptr == '/')
		{
			str = bptr + 1;
			fd = *str++;

			if (fd == 'c')
			{
				chat_clear();
			}
			else if (fd == 'n')
			{
				/* str_ncpy(chatid, str, sizeof(chatid) - 1); */
				/* Thor.0819: chatid array����9, ���S�ǤJ, �ҥH�w���o */
				/* str_ncpy(chatid, str, 8); */

				/* Thor.980921: str_ncpy�t0*/
				str_ncpy(chatid, str, 9);

				/* Thor.0819: ���K���@�U mateid �n�F... */
				/*str_ncpy(cutmp->mateid, str, 8); */
				/* Thor.980921: str_ncpy�t0*/
				str_ncpy(cutmp->mateid, str, sizeof(cutmp->mateid));

				print_chatid(chatid);
				clrtoeol();
			}
			else if (fd == 'r')
			{
				/* str_ncpy(chatroom, str, sizeof(chatroom) - 1); */
				/* Thor.980921: str_ncpy�t0*/
				str_ncpy(chatroom, str, sizeof(chatroom));
				chat_topic();
			}
			else if (fd == 't')
			{
				/* str_ncpy(chatopic, str, sizeof(chatopic) - 1); */
				/* Thor.980921: str_ncpy�t0*/
				str_ncpy(chatopic, str, sizeof(chatopic));
				chat_topic();
			}
		}
		else
		{
			printchatline(bptr);
		}

		cc -= ++len;
		if (cc <= 0)
		{
			bufstart = 0;
			break;
		}
		bptr += len;
	}

	return 0;
}

#if 0
static void
chat_pager(arg)
char *arg;
{
	cuser.ufo ^= UFO_PAGER;
	cutmp->ufo ^= UFO_PAGER;
	/* Thor.980805: �ѨMufo �P�B���D */

	sprintf(arg, "�� �z���I�s���w�g%s�F!",
			cuser.ufo & UFO_PAGER ? "����" : "���}");
	printchatline(arg);
}
#endif

#if 0
/* Thor.0727: �M /flag ��key */
static void
chat_write(arg)
char *arg;
{
	int uno;
	UTMP *up;
	char *str;
	CallMsg cmsg;

	strtok(arg, STR_SPACE);
	if ((str = strtok(NULL, STR_SPACE)) && (uno = acct_userno(str)) > 0)
	{
		cmsg.recver = uno;		/* ���O�U userno �@�� check */
		if (up = utmp_find(uno))
		{
			if (can_override(up))
			{
				if (str = strtok(NULL, "\n"))	/* Thor.0725:���y�� */
				{
					/* Thor.0724: �q my_write ��L�� */
					int len;
					char buf[80];
					extern char fpmsg[];
					/* Thor.0722: msg file�[�W�ۤv������ */

					sprintf(fpmsg + 4, "%s-", cuser.userid);
					/* Thor.0722: �ɥ� len��@�Ufd :p */
					len = open(fpmsg, O_WRONLY | O_CREAT | O_APPEND, 0600);
					sprintf(buf, "��%s�G%s\n", up->userid, str);
					write(len, buf, strlen(buf));
					close(len);

					sprintf(buf, "%s(%s", cuser.userid, cuser.username);
					len = strlen(str);
					buf[71 - len] = '\0';
					sprintf(cmsg.msg, "\033[1;33;46m�� %s) \033[37;45m %s \033[m", buf, str);

					cmsg.caller = cutmp;
					cmsg.sender = cuser.userno;

					if (do_write(up, &cmsg))
						printchatline("�� ���w�g���h");
				}
				else
				{
					printchatline("�� �O�u�w���A���Ǹܧa�I");
				}
			}
			else
			{
				printchatline("�� ����զ�ݳ���G�y�ڨSť��K�K�ڨSť��K�K�z");
			}
		}
		else
		{
			printchatline("�� ��褣�b���W");
		}
	}
	else
	{
		printchatline(err_uid);
	}
}


static int
printuserent(uentp)
user_info *uentp;
{
	static char uline[80];
	static int cnt;
	char pline[30];
	int cloak;

	if (!uentp)
	{
		if (cnt)
			printchatline(uline);
		memset(uline, 0, 80);
		return cnt = 0;
	}
	cloak = uentp->ufo & UFO_CLOAK;
	if (cloak && !HAS_PERM(PERM_SEECLOAK))
		return 0;

	sprintf(pline, " %-13s%c%-10s", uentp->userid,
			cloak ? '#' : ' ', bmode(uentp, 1));
	if (cnt < 2)
		strcat(pline, "�x");
	strcat(uline, pline);
	if (++cnt == 3)
	{
		printchatline(uline);
		memset(uline, 0, 80);
		cnt = 0;
	}
	return 0;
}


static void
chat_users()
{
	/* �]���H�ưʻ��W�ʡA�N�q���j */
	printchatline("");
	printchatline("�i " BOARDNAME "�C�ȦC�� �j");
	printchatline(MSG_CHAT_ULIST);

	if (apply_ulist(printuserent) == -1)
		printchatline("�ŵL�@�H");

	printuserent(NULL);
}
#endif


struct chat_command
{
	char *cmdname;		/* Char-room command length */
	void (*cmdfunc)();		/* Pointer to function */
};


struct chat_command chat_cmdtbl[] =
{
	{"clear", chat_clear/*chat_pager*/},
	{"tape", chat_record},

#if 0
	/* Thor.0727: �M /flag ��key */
	{"fire", chat_write},

	{"users", chat_users},
#endif

	{NULL, NULL}
};


static inline int
chat_cmd_match(buf, str)
char *buf;
char *str;
{
	int c1, c2;

	for (;;)
	{
		c1 = *str++;
		if (!c1)
			break;

		c2 = *buf++;
		if (!c2 || c2 == ' ' || c2 == '\n')
			break;

		if (c2 >= 'A' && c2 <= 'Z')
			c2 |= 0x20;

		if (c1 != c2)
			return 0;
	}

	return 1;
}


static inline int
chat_cmd(fd, buf)
int fd;
char *buf;
{
	struct chat_command *cmd;
	char *key;

	buf++;
	for (cmd = chat_cmdtbl; (key = cmd->cmdname); cmd++)
	{
		if (chat_cmd_match(buf, key))
		{
			cmd->cmdfunc(buf);
			return '/';
		}
	}

	return 0;
}


extern char lastcmd[MAXLASTCMD][80];


int
t_chat()
{
	int ch, cfd, cmdpos, cmdcol;
	char *ptr = NULL, buf[80], chatid[9];
	struct sockaddr_in sin;
#if     defined(__OpenBSD__)
	struct hostent *h;
#endif

#ifdef CHAT_SECURE
	extern char passbuf[];
#endif

#ifdef EVERY_Z
	/* Thor.0725: �� talk & chat �i�� ^z �@�ǳ� */
	if (holdon_fd)
	{
		vmsg("�z�������@�b�٨S�����C");
		return -1;
	}
#endif

#if     defined(__OpenBSD__)

	if (!(h = gethostbyname(MYHOSTNAME)))
		return -1;

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(CHAT_PORT);
	memcpy(&sin.sin_addr, h->h_addr, h->h_length);

#else

	sin.sin_family = AF_INET;
	sin.sin_port = htons(CHAT_PORT);
	sin.sin_addr.s_addr = INADDR_ANY /* htonl(INADDR_LOOPBACK) */;
	memset(sin.sin_zero, 0, sizeof(sin.sin_zero));

#endif

	cfd = socket(AF_INET, SOCK_STREAM, 0);
	if (cfd < 0)
		return -1;

	if (connect(cfd, (struct sockaddr *) & sin, sizeof sin))
	{
		close(cfd);
		blog("CHAT ", "connect");
		return -1;
	}

	for (;;)
	{
		move(b_lines - 1, 0);
		outs("�Y���i�J��ѫǡA�h�ʺٽШϥ� * ");
		ch = vget(b_lines, 0, "�п�J��ѥN���G", chatid, 9, DOECHO);
		if (ch == '/')
			continue;
		else if (ch == '*')
		{
			close(cfd);
			return -1;
		}
		else if (!ch)
		{
#if 0
			memcpy(chatid, cuser.userid, 8);
			chatid[8] = '\0';
#endif
			str_ncpy(chatid, cuser.userid, sizeof(chatid));
			/* Thor.980921: �R�� dao lib */
		}
		else
		{
			/* Thor.980911: chatid�����i�H�ť�, ���� parse���~ */
			for (ch = 0;ch < 8;ch++)
			{
				if (chatid[ch] == ' ')
					break;
				else if (!chatid[ch]) /* Thor.980921: �p�G0���ܴN���� */
					ch = 8;
			}
			if (ch < 8)
				continue;
		}

#ifdef CHAT_SECURE
		/* Thor.0729: secured chat room */
		/* sprintf(buf, "/! %d %s %s %s\n", */
		sprintf(buf, "/! %s %s %s\n",
				/* cuser.userno, cuser.userid, chatid, cuser.passwd); */
				/* cuser.userno, cuser.userid, chatid, passbuf); */
				cuser.userid, chatid, passbuf);
		/* Thor.0819: ����userno */
		/* Thor.0730: passwd �אּ�̫᪺�Ѽ� */
		/* Thor.0813: passwd �אּ�u����password, for C/S bbs */
		/* Thor.0813: xchatd��, chatid �۰ʸ��L�ť�, �ҥH���ťշ| invalid login */

		/*
		 * �@��  opus (�H�ͦ����O�M�w)                                ����
		 * sysopplan ���D  Re: ���� chatroom �ɶ�  Wed Jul 30 03:14:56 1997
		 * �w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w
		 * �w�w�w
		 *
		 * �� �ޭz�mThor (BBS�ǧٳ�����)�n���ʨ��G > �� �ޭz�mThor
		 * (BBS�ǧٳ�����)�n���ʨ��G > > �p�G�βĤ@�۴N�@�F�ʤF�F...:p > >
		 * ��...���n�N���Ǫ�����F...:p >
		 * �Ĥ@�ۧ@�n�F...�i�O�����D�|���|��...:p > ���ݤ@�U, passwd
		 * �O�s�򪺫D�ťզr����?? �T�ӰѼ� userid + chatid + passwd ��, userid /
		 * chatid �Ҥ��t space, �� passwd �i�]�t space,
		 * �ҥH�����N���\�b�ĤT�Ӧ�m�H�K token-parsing�C
		 *
		 * > �t�~, ACCT ���� passwd �OPASSLEN�۰ʸɪť��٬O�n��ʥ[? > �|�۰ʸɺ���?
		 * Unix �� crypt �̦h�u���e 14 �Ӧr, �ҥH���[��i�C�o�Ӧa�� �ѷ� user
		 * login ���a��g�N�n�F�C -- �� Origin: �����毸(bbs.cs.nthu.edu.tw)
		 * om: thcs-8.cs.nthu.edu.tw
		 */
#else
		sprintf(buf, "/! %d %d %s %s\n",
				cuser.userno, cuser.userlevel, cuser.userid, chatid);
#endif

		chat_send(cfd, buf);
		if (recv(cfd, buf, 3, 0) != 3)
			return 0;

		if (!strcmp(buf, CHAT_LOGIN_OK))
			break;
		else if (!strcmp(buf, CHAT_LOGIN_EXISTS))
			ptr = "�o�ӥN���w�g���H�ΤF";
		else if (!strcmp(buf, CHAT_LOGIN_INVALID))
			ptr = "�o�ӥN���O���~��";
		else if (!strcmp(buf, CHAT_LOGIN_BOGUS))
		{
			/* Thor: �T��ۦP�G�H�i�J */
			close(cfd);
			vmsg("�ФŬ����u�����v�i�J�ͤѫ�");
			return 0;
		}
		move(b_lines - 1, 0);
		outs(ptr);
		clrtoeol();
		bell();
	}

	clear();
	move(1, 0);
	outs(msg_seperator);
	move(stop_line, 0);
	outs(msg_seperator);
	print_chatid(chatid);
	memset(ptr = buf, 0, sizeof(buf));
	chatline = 2;
	cmdcol = 0;
	cmdpos = -1;

	add_io(cfd, 60);

	strcpy(cutmp->mateid, chatid);
#ifdef	LOG_CHAT
	chat_recordtomail(1);
#endif
	for (;;)
	{
		move(b_lines - 1, cmdcol + 10);
		ch = vkey();

		if (ch == I_OTHERDATA)
		{
			/* incoming */
			if (chat_recv(cfd, chatid) == -1)
				break;
			continue;
		}

		if (isprint2(ch))
		{
			if (cmdcol < 68)
			{
				if (ptr[cmdcol])
				{
					/* insert */
					int i;

					for (i = cmdcol; ptr[i] && i < 68; i++);
					ptr[i + 1] = '\0';
					for (; i > cmdcol; i--)
						ptr[i] = ptr[i - 1];
				}
				else
				{
					/* append */
					ptr[cmdcol + 1] = '\0';
				}
				ptr[cmdcol] = ch;
				move(b_lines - 1, cmdcol + 10);
				outs(&ptr[cmdcol++]);
			}
			continue;
		}

		if (ch == '\n')
		{
#ifdef EVERY_BIFF
			/* Thor.980805: ���H�b�����enter�~�ݭncheck biff */
			static int old_biff1, old_biff2;
			int biff1 = cutmp->ufo & UFO_BIFF;
			int biff2 = cutmp->ufo & UFO_BIFFN;
			if (biff1 && !old_biff1)
				printchatline("�� ��! �l�t�ӫ��a�F!");
			if (biff2 && !old_biff1)
				printchatline("�� ��! �z���s�d���@!");
			old_biff1 = biff1;
			old_biff2 = biff2;
#endif
			if ((ch = *ptr))
			{
				if (ch == '/')
					ch = chat_cmd(cfd, ptr);

				/* Thor.980602: ���ӭn�`�N���p�a��, �쥻�p�G�O�y/�z,
				                �|�q�X /help���e��,
				                �{�b�� /, �|�ܦ� /p ���� pager */

				/* Thor.980925: �O�d ptr �̭�l��, ���[ /n */
				for (cmdpos = MAXLASTCMD - 1; cmdpos; cmdpos--)
					strcpy(lastcmd[cmdpos], lastcmd[cmdpos - 1]);
				strcpy(lastcmd[0], ptr);

				if (ch != '/')
				{
					strcat(ptr, "\n");
					if (!chat_send(cfd, ptr))
						break;
				}
				if (*ptr == '/' && ptr[1] == 'b')
					break;

				if (*ptr == '/' && !strncmp(ptr, "/�۳¶}��", 9))
					break;


				*ptr = '\0';
				cmdcol = 0;
				cmdpos = -1;
				move(b_lines - 1, 10);
				clrtoeol();
			}
			continue;
		}

		if (ch == Ctrl('C'))
		{
			chat_send(cfd, "/b\n");
			break;
		}

		if (ch == Ctrl('Y'))
		{
			*ptr = '\0';
			cmdcol = 0;
			move(b_lines - 1, 10);
			clrtoeol();
			continue;
		}

		if (ch == Ctrl('A'))
		{
			cmdcol = 0;
			continue;
		}

		if (ch == Ctrl('E'))
		{
			cmdcol = strlen(ptr);
			continue;
		}


		if (ch == Ctrl('H'))
		{
			if (cmdcol)
			{
				ch = cmdcol--;
				memcpy(&ptr[cmdcol], &ptr[ch], 69 - cmdcol);
				move(b_lines - 1, cmdcol + 10);
				outs(&ptr[cmdcol]);
				clrtoeol();
			}
			continue;
		}

		if (ch == Ctrl('D'))
		{
			chat_send(cfd, "/b\n");
			break;
		}


		if (ch == KEY_LEFT)
		{
			if (cmdcol)
				--cmdcol;
			continue;
		}

		if (ch == KEY_RIGHT)
		{
			if (ptr[cmdcol])
				++cmdcol;
			continue;
		}

#ifdef EVERY_Z
		/* Thor: Chat ���� ctrl-z */
		if (ch == Ctrl('Z'))
		{

			char buf[IDLEN + 1];
			screenline scr[b_lines + 1];
			vs_save(scr);

			/* Thor.0731: �Ȧs mateid, �]���X�h�ɥi��|�α� mateid(like query) */
			strcpy(buf, cutmp->mateid);

			/* Thor.0727: �Ȧs vio_fd */
			holdon_fd = vio_fd;
			vio_fd = 0;
			every_Z();
			/* Thor.0727: �٭� vio_fd */
			vio_fd = holdon_fd;
			holdon_fd = 0;

			/* Thor.0731: �٭� mateid, �]���X�h�ɥi��|�α� mateid(like query) */
			strcpy(cutmp->mateid, buf);
			vs_restore(scr);
			continue;
		}
#endif
		if (ch == Ctrl('U'))
		{
			char buf[IDLEN + 1];

			strcpy(buf, cutmp->mateid);

			/* Thor.0727: �Ȧs vio_fd */
			holdon_fd = vio_fd;
			vio_fd = 0;
			every_U();
			/* Thor.0727: �٭� vio_fd */
			vio_fd = holdon_fd;
			holdon_fd = 0;

			/* Thor.0731: �٭� mateid, �]���X�h�ɥi��|�α� mateid(like query) */
			strcpy(cutmp->mateid, buf);
			continue;
		}
		if (ch == Ctrl('B'))
		{
			char buf[IDLEN + 1];

			strcpy(buf, cutmp->mateid);

			holdon_fd = vio_fd;
			vio_fd = 0;
			every_B();
			vio_fd = holdon_fd;
			holdon_fd = 0;

			strcpy(cutmp->mateid, buf);
			continue;
		}
		if (ch == KEY_DOWN)
		{
			cmdpos += MAXLASTCMD - 2;
			ch = KEY_UP;
		}

		if (ch == KEY_UP)
		{
			cmdpos++;
			cmdpos %= MAXLASTCMD;
			strcpy(ptr, lastcmd[cmdpos]);
			move(b_lines - 1, 10);
			outs(ptr);
			clrtoeol();
			cmdcol = strlen(ptr);
		}
	}

	if (frec)
		chat_record();

#ifdef	LOG_CHAT
	chat_recordtomail(0);
#endif

	close(cfd);
	add_io(0, 60);
	cutmp->mateid[0] = 0;
	return 0;
}
