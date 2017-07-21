/*-------------------------------------------------------*/
/* util.c	( YZU WindTopBBS Ver 3.02 )		 */
/*-------------------------------------------------------*/
/* target : ���D�T�{�A�H�H�������A��s�t���ɮ�	 	 */
/* create : 95/03/29				 	 */
/* update : 95/12/15				 	 */
/*-------------------------------------------------------*/


#include "bbs.h"

#define	BM_CHECK_FILE	FN_CHECKBM_LOG

extern BCACHE *bshm;

typedef struct
{
	char id[IDLEN+1];
	char brd[IDLEN+1];
	int check;
} BM;


static int
use_io()
{
	int mode;
	mode = vans("�ϥΥ~���{�ǶܡH [Y/n] ");
	if (mode == 'n')
		return 0;
	else
		return 1;
}

static int
check_in_memory(char *bm, char *id)
{
	char *i;
	for (i = bm;strlen(i);i = i + IDLEN + 1)
		if (!strcmp(i, id))
			return 0;
	return 1;
}

/* �M���S�w�ݪO�����峹 cancel */
int
m_expire()
{
	BRD *brd;
	char bname[16];
	char buf[80];

	move(22, 0);
	outs("�M���S�w�ݪO cancal ���峹�C");
	if ((brd = ask_board(bname, BRD_R_BIT, NULL)))
	{
		sprintf(buf, "bin/expire 999 20000 20000 \"%s\" &", brd->brdname);
		system(buf);
		logitfile(FN_EXPIRED_LOG, cuser.userid, buf);
	}
	else
	{
		vmsg(err_bid);
	}

	return 0;
}

static void
send_to_all(char *title, char *fpath, char *bm)
{
	char buf[128], *ptr;
	HDR mhdr;

	for (ptr = bm;strlen(ptr);ptr = ptr + IDLEN + 1)
	{
		usr_fpath(buf, ptr, fn_dir);
		hdr_stamp(buf, HDR_LINK, &mhdr, fpath);
		strcpy(mhdr.owner, "sysop");
		strcpy(mhdr.title, title);
		mhdr.xmode = MAIL_MULTI;
		rec_add(buf, &mhdr, sizeof(HDR));
	}
}

int
mail_to_bm(void)
{
	BRD *bhdr, *head, *tail;
	char *ptr, *bm;
	char fpath[256], *title, buf[128];
	FILE *fp;

	if (bbsothermode & OTHERSTAT_EDITING)
	{
		vmsg("�A�٦��ɮ��٨S�s���@�I");
		return -1;
	}


	bm = (char *)malloc(MAXBOARD * (IDLEN + 1) * 3);
	memset(bm, 0, MAXBOARD*(IDLEN + 1)*3);
	ptr = bm;
	utmp_mode(M_SMAIL);

	head = bhdr = bshm->bcache;
	tail = bhdr + bshm->number;
	do                          /* �ܤ֦�sysop�@�� */
	{
		char *c;
		char buf[BMLEN + 1];

		strcpy(buf, head->BM);
		c = buf;
		while (1)
		{
			char *d;
			d = strchr(c, '/');
			if (*c)
			{
				if (d)
				{
					*d++ = 0;
					if (check_in_memory(bm, c))
					{
						strcpy(ptr, c);
						ptr += IDLEN + 1;
					}
					c = d;
				}
				else
				{
					if (check_in_memory(bm, c))
					{
						strcpy(ptr, c);
						ptr += IDLEN + 1;
					}
					break;
				}
			}
			else
				break;
		}
	}
	while (++head < tail);
	strcpy(ve_title, "[�O�D�q�i]");
	title = ve_title;
	vget(1, 0, "�� �D �D�G", title, 60, GCARRY);
	sprintf(buf, "mailtobm.%d", time(0));
	usr_fpath(fpath, cuser.userid, buf);
	if ((fp = fopen(fpath, "w")))
	{
		fprintf(fp, "�� [�O�D�q�i] �����q�i�A���H�H�G�U�O�D\n");
		fprintf(fp, "-------------------------------------------------------------------------\n");
		fclose(fp);
	}
	utmp_mode(M_SMAIL);
	curredit = EDIT_MAIL | EDIT_LIST;

	if (vedit(fpath, YEA) == -1)
	{
		vmsg(msg_cancel);
		free(bm);
		return -1;
	}
	else
	{
		if (!use_io())
		{
			send_to_all(title, fpath, bm);
			unlink(fpath);
		}
		else
		{
			char command[128];
			sprintf(command, "bin/mailtoall 2 \"%s\" \"%s\" &", fpath, title);
			system(command);
		}
	}
	free(bm);
	return 0;
}

static void
traverse(fpath, path, title)
char *fpath;
char *path;
char *title;
{
	DIR *dirp;
	struct dirent *de;
	char *fname, *str;

	if (!(dirp = opendir(fpath)))
	{
		return;
	}
	for (str = fpath; *str; str++);
	*str++ = '/';

	while ((de = readdir(dirp)))
	{
		HDR mhdr;
		fname = de->d_name;
		if (fname[0] > ' ' && fname[0] != '.')
		{
			strcpy(str, fname);
			strcat(str, "/.DIR");
			hdr_stamp(fpath, HDR_LINK, &mhdr, path);
			strcpy(mhdr.owner, "SYSOP");
			strcpy(mhdr.title, title);
			mhdr.xmode = MAIL_MULTI;
			rec_add(fpath, &mhdr, sizeof(HDR));
		}
	}
	closedir(dirp);
}


static int
open_mail(path, title)
char *path;
char *title;
{
	int ch;
	char *fname, fpath[256];

	strcpy(fname = fpath, BBSHOME"usr/@");
	fname = (char *) strchr(fname, '@');

	for (ch = 'a'; ch <= 'z'; ch++)
	{
		fname[0] = ch;
		fname[1] = '\0';
		traverse(fpath, path, title);
	}
	return 1;
}

int
mail_to_all()
{
	char *title;
	char fpath[256];
	char buf[128];

	if (bbsothermode & OTHERSTAT_EDITING)
	{
		vmsg("�A�٦��ɮ��٨S�s���@�I");
		return -1;
	}


	strcpy(ve_title, "[�t�γq�i]");
	title = ve_title;
	vget(1, 0, "�� �D �D�G", title, 60, GCARRY);
	sprintf(buf, "mailtoall.%d", time(0));
	usr_fpath(fpath, cuser.userid, buf);
	utmp_mode(M_SMAIL);
	curredit = EDIT_MAIL | EDIT_LIST;
	if (vedit(fpath, YEA) == -1)
	{
		vmsg(msg_cancel);
		return -1;
	}
	else
	{
		if (!use_io())
		{
			open_mail(fpath, title);
			unlink(fpath);
		}
		else
		{
			char command[128];
			sprintf(command, "bin/mailtoall 1 \"%s\" \"%s\" &", fpath, title);
			system(command);
		}
	}
	return 0;
}

static int
is_bms(list, userid)
char *list;                   /* �O�D�GBM list */
char *userid;
{
	int cc, len;

	len = strlen(userid);
	do
	{
		cc = list[len];
		if ((!cc || cc == '/') && !str_ncmp(list, userid, len))
		{
			return 1;
		}
		while ((cc = *list++))
		{
			if (cc == '/')
				break;
		}
	}
	while (cc);

	return 0;
}

static inline int
is_bm(list)
char *list;                   /* �O�D�GBM list */
{
	int cc, len;
	char *userid;

	len = strlen(userid = cuser.userid);
	do
	{
		cc = list[len];
		if ((!cc || cc == '/') && !str_ncmp(list, userid, len))
		{
			return 1;
		}
		while ((cc = *list++))
		{
			if (cc == '/')
				break;
		}
	}
	while (cc);

	return 0;
}


int
bm_check()
{
	BRD *bhdr, *head, *tail;
	BM *bm, *ptr;
	char fpath[80], ans;
	ACCT acct;
	int fd;


	strcpy(fpath, BM_CHECK_FILE);
	move(22, 0);
	outs("�C�b�~�T�{���D�O�_�~��s���A�i��}�ǫ��g����C");
	ans = vans("�n�����D�T�{�� y)�T�w r)�_�� s)���� d)�R�� q)���} [q]:");
	if (ans == 'r')
	{
		BM tmp;
		int pos = 0;

		fd = open(fpath, O_RDONLY);
		while (fd)
		{
			lseek(fd, (off_t)(sizeof(BM) * pos), SEEK_SET);
			if (read(fd, &tmp, sizeof(BM)) == sizeof(BM))
			{
				head = bshm->bcache + brd_bno(tmp.brd);
				if (*(head->BM) && !is_bms(head->BM, tmp.id))
				{
					strcat(head->BM, "/");
					strcat(head->BM, tmp.id);
				}
				else
				{
					strcpy(head->BM, tmp.id);
				}
				pos++;
			}
			else
			{
				close(fd);
				break;
			}
		}
		unlink(fpath);
		return 0;
	}
	else if (ans == 's')
	{
		BM tmp;
		int pos = 0;

		fd = open(fpath, O_RDWR);
		while (fd)
		{
			lseek(fd, (off_t)(sizeof(BM) * pos), SEEK_SET);
			if (read(fd, &tmp, sizeof(BM)) == sizeof(BM))
			{
				lseek(fd, (off_t)(sizeof(BM) * pos), SEEK_SET);
				tmp.check = 0;
				write(fd, &tmp, sizeof(BM));
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
	else if (ans == 'd')
	{
		unlink(fpath);
		return 0;
	}
	else if (ans != 'y')
	{
		return 0;
	}
	if (!access(fpath, 0))
	{
		vmsg("���b�T�{���I");
		return 0;
	}

	bm = (BM *)malloc(sizeof(BM) * MAXBOARD * 3);
	memset(bm, 0, sizeof(BM)*MAXBOARD*3);
	ptr = bm;
	utmp_mode(M_SMAIL);

	head = bhdr = bshm->bcache;
	tail = bhdr + bshm->number;
	do                          /* �ܤ֦�sysop�@�� */
	{
		char *c;
		char buf[BMLEN + 1];

		strcpy(buf, head->BM);
		c = buf;
		while (1)
		{
			char *d;
			d = strchr(c, '/');
			if (*c)
			{
				if (d)
				{
					*d++ = 0;
					strcpy(ptr->brd, head->brdname);
					strcpy(ptr->id, c);
					ptr++;
					c = d;
				}
				else
				{
					strcpy(ptr->brd, head->brdname);
					strcpy(ptr->id, c);
					ptr++;
					break;
				}
			}
			else
				break;
		}
		head->BM[0] = '\0';
	}
	while (++head < tail);

	fd = open(fpath, O_WRONLY | O_CREAT | O_TRUNC, 0600);
	ptr = bm;
	do
	{
		acct_load(&acct, ptr->id);
		head = bshm->bcache + brd_bno(ptr->brd);
		if (acct.userlevel & PERM_SYSOP)
		{
			if (*(head->BM))
			{
				strcat(head->BM, "/");
				strcat(head->BM, ptr->id);
			}
			else
			{
				strcpy(head->BM, ptr->id);
			}
		}
		else
		{
			write(fd, ptr, sizeof(BM));
		}
	}
	while (*(++ptr)->id);

	close(fd);
	free(bm);
	return 0;
}

static int
find_bm(fpath, id)
char *fpath;
char *id;
{
	BM bm;
	int fd;
	int pos = 0;

	fd = open(fpath, O_RDONLY);
	while (fd)
	{
		lseek(fd, (off_t)(sizeof(BM) * pos), SEEK_SET);
		if (read(fd, &bm, sizeof(BM)) == sizeof(BM))
		{
			if (!strcmp(bm.id, id) && bm.check == 0)
			{
				close(fd);
				return pos;
			}
			pos++;
		}
		else
		{
			close(fd);
			return -1;
		}
	}
	return -1;
}

int
user_check_bm()
{
	char buf[128], temp[3];
	int ans, i;
	char *fpath;
	BM bm;
	BRD *head;
	struct stat st;

	i = 1;
	fpath = BM_CHECK_FILE;

	if (utmp_count(cuser.userno, 0) > 1)
	{
		vmsg("�Х��n�X��L�b���I");
		return 0;
	}

	if (access(fpath, 0))
	{
		vmsg("�{�b�S�����D�T�{�\\��I");
		return 0;
	}
	if (!stat(fpath, &st) && (st.st_mtime + CHECK_BM_TIME) < time(0))
	{
		vmsg("�W�L�{�Үɶ��A�Э��s�ӽСI");
		return 0;
	}


	clear();

	while ((ans = find_bm(fpath, cuser.userid)) >= 0)
	{
		rec_get(fpath, &bm, sizeof(BM), ans);
		head = bshm->bcache + brd_bno(bm.brd);
		sprintf(buf, "�A�n�~�򱵥� %s �����D�� [Y/n/q]:", bm.brd);
		vget(i++, 0, buf, temp, 3, DOECHO);
		if (*temp == 'q')
			return 0;
		if (*temp != 'n' && !is_bm(head->BM))
		{
			if (*(head->BM))
			{
				strcat(head->BM, "/");
				strcat(head->BM, cuser.userid);
			}
			else
			{
				strcpy(head->BM, cuser.userid);
			}
			rec_put(FN_BRD, head, sizeof(BRD), brd_bno(bm.brd));
		}
		bm.check = 1;
		rec_put(fpath, &bm, sizeof(BM), ans);
	}
	vmsg("�A�w�g�����Ҧ������D�T�{�F�I");
	return 0;
}

static void
search()
{
	FILE *fp;
	char buf[256], input[60];
	int i, key = 0;


	i = 1;
	fp = fopen(FN_MATCH_LOG, "r");
	if (fp)
	{
		clear();
		vs_bar("�S��j�M");
		vget(b_lines, 0, "�j�M���e�G", input, sizeof(input), DOECHO);
		while (fgets(buf, sizeof(buf), fp))
		{
			if (strstr(buf, input))
			{
				move(i, 0);
				prints("%s", buf);
				i++;
			}
			if (i >= b_lines)
			{
				key = vmsg("�� q ���}�A���N���~��");
				i = 1;
				move(i, 0);
				clrtobot();
			}
			if (key == 'q')
				break;
		}

	}
	if (fp)
	{
		vmsg("�j�M����");
		fclose(fp);
	}
	else
	{
		vmsg("�|����s");
	}
}

static void
update_match()
{
	char fpath[128];
	sprintf(fpath, "bin/match \"%s\" &", cuser.userid);
	if (access(FN_MATCH_NEW, 0))
		system(fpath);
	else
		vmsg("���b�u�@��");
}

static void
update_email()
{
	char fpath[128];
	sprintf(fpath, "bin/checkemail \"%s\" &", cuser.userid);
	if (access(FN_ETC_EMAILADDR_ACL".new", 0))
		system(fpath);
	else
		vmsg("���b�u�@��");
}

static void
update_spamer_acl()
{
	if (access(FN_ETC_SPAMER_ACL".new", 0))
	{
		system("bin/clean_acl " FN_ETC_SPAMER_ACL " " FN_ETC_SPAMER_ACL".new");
		rename(FN_ETC_SPAMER_ACL".new", FN_ETC_SPAMER_ACL);
	}
	else
		vmsg("���b�u�@��");
}

static void
update_untrust_acl()
{
	if (access(FN_ETC_UNTRUST_ACL".new", 0))
	{
		system("bin/clean_acl " FN_ETC_UNTRUST_ACL " " FN_ETC_UNTRUST_ACL ".new");
		rename(FN_ETC_UNTRUST_ACL".new", FN_ETC_UNTRUST_ACL);
	}
	else
		vmsg("���b�u�@��");
}

int
update_all()
{
	int ans;
	ans = vans("��s���ءG 1)�S��j�M 2)���U�H�c�Ӽ� 3)SPAM�W�� 4)���H���W�� 0)���� [0]");
	switch (ans)
	{
	case '1':
		update_match();
		break;
	case '2':
		update_email();
		break;
	case '3':
		update_spamer_acl();
		break;
	case '4':
		update_untrust_acl();
		break;
	}
	return 0;
}


int
special_search()
{
	int ans;
	move(22, 0);
	outs("�S��j�M�� ID�B�u��m�W�B�{�ҫH�c����M��ơA�Ω�B�z�H�k�ưȤ��d�ߡC");
	ans = vans("�S��j�M�G 1)��s��� 2)�j�M 0)���� [0]");
	switch (ans)
	{
	case '1':
		update_match();
		break;
	case '2':
		search();
		break;
	}

	return 1;
}

int
m_xfile()
{
	static char *desc[] =
	{
		"���n���i",		/* lkchu.990510: edit ~/etc/announce online */
		"�����W��",
		"�ק� Email",
		"�s��W������",
		"�����{�Ҫ���k",
		"�����{�ҫH��",
		"�ݪO����",
		"�s�i/�U���H�W��",         /* lkchu.981201: �u�W�s�� mail.acl */
		"���\\���U�W��",
		"�T��W����m",
		"���H���W��",		/* pcbug.990806: edit ~/etc/untrust */
		"���Ѳ��;�",
		"�{������",
		"���v�W���@��",
		"���ȦC��",
		"���q�L�����{��",
		"�׫H����",
		"�I�q����",
		"�ΦW������",
		"���U�满��",
		"�׵o�H�n��",
		"Email �q�L�{��",
		"POP3 �q�L�{��",
		"BMTA �q�L�{��",
		NULL
	};

	static char *path[] =
	{
		FN_ETC_ANNOUNCE,
		FN_ETC_BADID,
		FN_ETC_EMAIL,
		FN_ETC_NEWUSER,
		FN_ETC_JUSTIFY,
		FN_ETC_VALID,
		FN_ETC_EXPIRE_CONF,
		FN_ETC_SPAMER_ACL,
		FN_ETC_ALLOW_ACL,
		FN_ETC_BANIP_ACL,
		FN_ETC_UNTRUST_ACL,
		FN_ETC_LOVEPAPER,
		FN_ETC_VERSION,
		FN_ETC_COUNTER,
		FN_ETC_SYSOP,
		FN_ETC_NOTIFY,
		FN_BANMAIL_LOG,
		FN_SONG_LOG,
		FN_ANONYMOUS_LOG,
		FN_ETC_RFORM,
		FN_ETC_MAILER_ACL,
		FN_ETC_APPROVED,
		FN_ETC_JUSTIFIED_POP3,
		FN_ETC_JUSTIFIED_BMTA
	};

	x_file(M_XFILES, desc, path);
	return 0;
}

int
psaux()
{
	system("ps -aux > tmp/psaux");
	more("tmp/psaux", NULL);
	return 0;
}

int
df()
{
	system("df -i > tmp/df");
	more("tmp/df", NULL);
	return 0;
}

int
dmesg()
{
	system("dmesg > tmp/dmesg");
	more("tmp/dmesg", NULL);
	return 0;
}

int
top()
{
	system("top > tmp/top");
	more("tmp/top", NULL);
	return 0;
}


int
m_xhlp()
{
	static char *desc[] =
	{
		"�i���s�i",
		"���U���ܵe��",
		"���~�ʺA�ݪO�e��",
		"�i���e��",
		"�峹�o�����",
		"���~�n�J�e��",
		"�w��e��",
		"�t�κ޲z��",
		"�W���q���W��",
		"�׫H�C��",
		"�T�����",
		"�ݪO",
		"�ݪO���",
		"�p���W��",
		"�s�边",
		"�n�ͦW��",
		"��ذ�",
		"�q�l�H�c",
		"�Ƨѿ�",
		"�l��t��",
		"�\\Ū�峹",
		"�s�p�t��",
		"�I�q�t��",
		"������Ѥ�U",
		"�벼�c",
		NULL
	};

	static char *path[] =
	{
        "gem/@/@AD",
		"gem/@/@apply",
		"gem/@/@error-camera",
		"gem/@/@income",
		"gem/@/@post",
		"gem/@/@tryout",
		"gem/@/@welcome",
		"gem/@/@admin.hlp",
		"gem/@/@aloha.hlp",
		"gem/@/@banmail.hlp",
		"gem/@/@bmw.hlp",
		"gem/@/@board.hlp",
		"gem/@/@class.hlp",
		"gem/@/@contact.hlp",
		"gem/@/@edit.hlp",
		"gem/@/@friend.hlp",
		"gem/@/@gem.hlp",
		"gem/@/@mbox.hlp",
		"gem/@/@memorandum.hlp",
		"gem/@/@mime.hlp",
		"gem/@/@more.hlp",
		"gem/@/@signup.hlp",
		"gem/@/@song.hlp",
		"gem/@/@ulist.hlp",
		"gem/@/@vote.hlp"
	};

	x_file(M_XFILES, desc, path);
	return 0;
}

/* pcbug.990620: �i�ologin...:p */
static void
m_resetsys(select)
int select;
{
	time_t now;
	struct tm ntime, *xtime ;
	now = time(NULL);
	xtime = localtime(&now);
	ntime = *xtime;

	if (vans("�z�T�w�n���m�t�ζܡH[y/N]") != 'y')
		return;
	switch (select)
	{
	case 1:
		system("bin/camera");
		logitfile(FN_RESET_LOG, "< �ʺA�ݪO >", NULL);
		break;
	case 2:
		if (ntime.tm_hour != 0 && ntime.tm_hour != 1)
		{
			system("bin/account; bin/acpro");
			board_main();
			logitfile(FN_RESET_LOG, "< �����ݪO >", NULL);
		}
		else
		{
			vmsg("�{�b�T��ϥΤ����ݪO���m!!");
		}
		break;
	case 3:
		system("kill -9 `cat run/bmta.pid`;\
			   kill -9 `cat run/bguard.pid`;\
			   kill -9 `ps -auxwww | grep innbbsd | awk '{print $2}'`;\
			   kill -9 `ps -auxwww | grep bbslink | awk '{print $2}'`;\
			   kill -9 `ps -auxwww | grep bbsnnrp | awk '{print $2}'`");
		logitfile(FN_RESET_LOG, "< ��H���H >", NULL);
		break;
	case 4:
		system("kill -9 `top | grep RUN | grep bbsd | awk '{print $1}'`");
		logitfile(FN_RESET_LOG, "< ���`�{�� >", NULL);
		break;
	case 5:
		system("bin/makefw");
		logitfile(FN_RESET_LOG, "< �׫H�C�� >", NULL);
		break;
	case 6:
		system("kill -9 `ps -auxwww | grep xchatd | awk '{print $2}'`");
		logitfile(FN_RESET_LOG, "< �D��ѫ� >", NULL);
		break;
	case 7:
		if (ntime.tm_hour != 0 && ntime.tm_hour != 1)
		{
			system("kill -9 `cat run/bmta.pid`;\
				   kill -9 `cat run/bguard.pid`;\
				   bin/camera;\
				   bin/account;\
				   bin/acpro;\
				   kill -9 `ps -auxwww | grep innbbsd | awk '{print $2}'`;\
				   kill -9 `ps -auxwww | grep bbslink | awk '{print $2}'`;\
				   kill -9 `ps -auxwww | grep bbsnnrp | awk '{print $2}'`;\
				   kill -9 `ps -auxwww | grep xchatd  | awk '{print $2}'`");
			board_main();
			logitfile(FN_RESET_LOG, "< �����t�� >", NULL);
		}
		else
		{
			vmsg("�{�b�T��m�Ҧ��t��!!");
		}
		break;
	}

}

int
reset1()
{
	m_resetsys(1);
	return 0;
}

int
reset2()
{
	m_resetsys(2);
	return 0;
}

int
reset3()
{
	m_resetsys(3);
	return 0;
}

int
reset4()
{
	m_resetsys(4);
	return 0;
}

int
reset5()
{
	m_resetsys(5);
	return 0;
}

int
reset6()
{
	m_resetsys(6);
	return 0;
}

int
reset7()
{
	m_resetsys(7);
	return 0;
}
