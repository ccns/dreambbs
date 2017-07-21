/*-------------------------------------------------------*/
/* newboard.c   ( YZU_CSE WindTop BBS )                  */
/*-------------------------------------------------------*/
/* target : newboard routines			 	 */
/* create : 2000/01/02				 	 */
/* update : NULL				 	 */
/*-------------------------------------------------------*/

#include "bbs.h"

#undef	TEST_COSIGN


extern XZ xz[];

extern char xo_pool[];

static int nbrd_add();
static int nbrd_body();
static int nbrd_head();

extern BCACHE *bshm;

#define	S_PART		"-----------------------------------------------------------\n"
#define	S_AGREE_START	">------------------- [   �٦��}�l   ] --------------------<\n"
#define	S_AGREE_END	">------------------- [   �٦�����   ] --------------------<\n"
#define	S_ASSIST_START	">------------------- [   �Ϲ�}�l   ] --------------------<\n"
#define	S_ASSIST_END	">------------------- [   �Ϲﵲ��   ] --------------------<\n"




typedef struct
{
	char email[60];
} LOG;


static char
nbrd_attr(nbrd)
NBRD *nbrd;
{
	if (nbrd->mode & (NBRD_REJECT | NBRD_STOP))
		return 'R';
	else if (nbrd->mode & NBRD_CLOSE)
		return 'C';
	else if (nbrd->mode & NBRD_OPEN)
		return 'O';
	else if (nbrd->mode & NBRD_OK)
		return 'Y';
	else if (nbrd->mode & NBRD_START)
		return ' ';
	else
		return 'N';
}

static int
nbrd_stamp(folder, nbrd, fpath)
char *folder;
NBRD *nbrd;
char *fpath;
{
	char *fname, *family = NULL;
	int rc;
	int token;

	fname = fpath;
	while ((rc = *folder++))
	{
		*fname++ = rc;
		if (rc == '/')
			family = fname;
	}
	fname = family;
	*family++ = '@';

	token = time(0);

	archiv32(token, family);

	rc = open(fpath, O_WRONLY | O_CREAT | O_EXCL, 0600);
	memset(nbrd, 0, sizeof(NBRD));
	nbrd->btime = token;
	str_stamp(nbrd->date, &nbrd->btime);
	strcpy(nbrd->xname, fname);
	return rc;
}

static void
nbrd_fpath(fpath, folder, nbrd)
char *fpath;
char *folder;
NBRD *nbrd;
{
	char *str = NULL;
	int cc;

	while ((cc = *folder++))
	{
		*fpath++ = cc;
		if (cc == '/')
			str = fpath;
	}
	strcpy(str, nbrd->xname);
}

static int
nbrd_init(xo)
XO *xo;
{
	xo_load(xo, sizeof(NBRD));
	return nbrd_head(xo);
}


static int
nbrd_load(xo)
XO *xo;
{
	xo_load(xo, sizeof(NBRD));
	return nbrd_body(xo);
}


static void
nbrd_item(num, nbrd)
int num;
NBRD *nbrd;
{
	if (nbrd->mode & NBRD_NBRD)
		prints("%6d %c %-5s %-13s %-13s:%-22.22s\n", num, nbrd_attr(nbrd), nbrd->date + 3, nbrd->owner, nbrd->brdname, nbrd->title);
	else if (nbrd->mode & NBRD_CANCEL)
		prints("%6d %c %-5s %-13s �o�� %s �����D\n", num, nbrd_attr(nbrd), nbrd->date + 3, nbrd->owner, nbrd->brdname);
	else
		prints("%6d %c %-5s %-13s %-36.36s\n", num, nbrd_attr(nbrd), nbrd->date + 3, nbrd->owner, nbrd->title);
}


static int
nbrd_body(xo)
XO *xo;
{
	NBRD *nbrd;
	int num, max, tail;

	max = xo->max;
	if (max <= 0)
	{
		if (HAS_PERM(PERM_VALID))
		{
			if (vans("�n�s�W�s�p���ضܡH(Y/N) [N] ") == 'y')
				return nbrd_add(xo);
		}
		else
		{
			vmsg("�|�L�s�p����");
		}
		return XO_QUIT;
	}

	nbrd = (NBRD *) xo_pool;
	num = xo->top;
	tail = num + XO_TALL;
	if (max > tail)
		max = tail;

	move(3, 0);
	do
	{
		nbrd_item(++num, nbrd++);
	}
	while (num < max);

	clrtobot();
	return XO_NONE;
}


static int
nbrd_head(xo)
XO *xo;
{
	clear();
	vs_head("�s�p�t��", NULL);
	outs("\
		 [��]���} [��]�\\Ū [^P]�o�� [d]�R�� [j]�[�J�s�p [TAB]��ذ� [h]elp\n\
		 \033[44m  �s��   �� �� �|��H        ��  ��  ��  �D                                   \033[m");
	return nbrd_body(xo);
}

static int
nbrd_find(xo, ptr, mode)
XO *xo;
char *ptr;
int mode;
{
	int pos = 0, fd;
	NBRD nbrd;
	fd = open(xo->dir, O_RDONLY);
	while (fd)
	{
		lseek(fd, (off_t)(sizeof(NBRD) * pos), SEEK_SET);
		if (read(fd, &nbrd, sizeof(NBRD)) == sizeof(NBRD))
		{
			if (!str_cmp(nbrd.brdname, ptr) && !(nbrd.mode & (NBRD_REJECT | NBRD_STOP | NBRD_CLOSE | NBRD_OPEN)) && (nbrd.mode & mode))
			{
				return 1;
				close(fd);
			}
			pos++;
		}
		else
			break;
	}
	return 0;
	close(fd);
}


static int
nbrd_add(xo)
XO *xo;
{
	int fd, mode, days = 0, numbers = 0;
	char *dir, fpath[80], buf[IDLEN +1];
	char buf1[49], tmp[10], path[80];
	FILE *fp;
	NBRD nbrd;
	time_t etime;
	time_t now;

	if (bbsothermode & OTHERSTAT_EDITING)
	{
		vmsg("�A�٦��ɮ��٨S�s���@�I");
		return XO_FOOT;
	}



	if (!HAS_PERM(PERM_POST))
	{
		return XO_NONE;
	}

	sprintf(path, "newboard/%s", cuser.userid);
	mode = vans("�s�p�Ҧ� 1)�}�s�� 2)�o���D 3)��L 0)���� [0] :") - '0';

	if (mode == 1)
	{
		more(FN_NEWBOARD_HELP, NULL);
		if (!vget(b_lines, 0, "���W�G", buf, sizeof(nbrd.brdname), DOECHO))
			return XO_INIT;
		if (brd_bno(buf) >= 0)
		{
			vmsg("�w�������I");
			return XO_INIT;
		}
		if (nbrd_find(xo, buf, NBRD_NBRD))
		{
			vmsg("���b�s�p���I");
			return XO_FOOT;
		}
		if (!vget(b_lines, 0, "�ݪO�D�D�G", buf1, sizeof(nbrd.title), DOECHO))
			return XO_FOOT;
	}
	else if (mode == 2)
	{
		if (!vget(b_lines, 0, "���W�G", buf, sizeof(nbrd.brdname), DOECHO))
			return XO_INIT;
		if (brd_bno(buf) < 0)
		{
			vmsg("�L�����I");
			return XO_INIT;
		}
		if (nbrd_find(xo, buf, NBRD_CANCEL))
		{
			vmsg("���b�s�p���I");
			return XO_FOOT;
		}
		strcpy(buf, (bshm->bcache + brd_bno(buf))->brdname);
	}
	else if (mode == 3)
	{
		if (!vget(b_lines, 0, "�s�p�D�D�G", buf1, sizeof(nbrd.title), DOECHO))
			return XO_FOOT;
		if (!vget(b_lines, 0, "�s�p�ѼơG", tmp, 5, DOECHO))
			return XO_FOOT;
		days = atoi(tmp);
		if (days > 30 || days < 1)
			return XO_FOOT;
		if (!vget(b_lines, 0, "�s�p�H�ơG", tmp, 6, DOECHO))
			return XO_FOOT;
		numbers = atoi(tmp);
		if (numbers > 500 || numbers < 1)
			return XO_FOOT;
	}
	else
		return 0;


	dir = xo->dir;
	fd = nbrd_stamp(dir, &nbrd, fpath);
	if (fd < 0)
		return XO_HEAD;
	close(fd);

	now = time(0);


	if (mode == 1)
	{
		vmsg("�}�l�s�� [�ݪO�����P���D��t]");
		fd = vedit(path, 0);
		if (fd)
		{
			unlink(path);
			unlink(fpath);
			vmsg("�����s�p");
			return XO_HEAD;
		}
		nbrd.etime = nbrd.btime + NBRD_DAYS * 86400;
		fp = fopen(fpath, "a+");
		fprintf(fp, "�@��: SYSOP (" SYSOPNICK ") ����: NewBoard\n");
		fprintf(fp, "���D: %s �ӽзs���s�p\n", buf);
		fprintf(fp, "�ɶ�: %s\n\n", ctime(&now));
		fprintf(fp, S_PART);
		fprintf(fp, "    ���W�G%s\n", buf);
		fprintf(fp, "    �ݪO�D�D�G%s\n", buf1);
		fprintf(fp, "    �|�����G%s\n", nbrd.date);
		fprintf(fp, "    ����ѼơG%d\n", NBRD_DAYS);
		fprintf(fp, "    ���D�W�١G%s\n", cuser.userid);
		fprintf(fp, "    �q�l�l��H�c�G%s\n", cuser.email);
		fprintf(fp, "    �ݳs�p�H�ơG%d\n", NBRD_MAX);
		fprintf(fp, S_PART);
		fprintf(fp, "    ���D��t�G\n");
		f_suck(fp, path);
		unlink(path);
		fprintf(fp, "\n");
	}
	else if (mode == 2)
	{
		vmsg("�}�l�s�� [�o���D��]]");
		fd = vedit(path, 0);
		if (fd)
		{
			unlink(path);
			unlink(fpath);
			vmsg("�����s�p");
			return XO_HEAD;
		}
		nbrd.etime = etime = nbrd.btime + NBRD_DAYS * 86400;
		fp = fopen(fpath, "a+");
		fprintf(fp, "�@��: SYSOP (" SYSOPNICK ") ����: NewBoard\n");
		fprintf(fp, "���D: �ӽмo�� %s �O�D\n", buf);
		fprintf(fp, "�ɶ�: %s\n\n", ctime(&now));
		fprintf(fp, S_PART);
		fprintf(fp, "    ���W�G%s\n", buf);
		fprintf(fp, "    �|�����G%s\n", nbrd.date);
		fprintf(fp, "    ����ѼơG%d\n", NBRD_DAYS);
		fprintf(fp, "    �|��H�G%s\n", cuser.userid);
		fprintf(fp, "    �q�l�l��H�c�G%s\n", cuser.email);
		fprintf(fp, "    �̤��٦��H�ơG%d\n", NBRD_MAX_CANCEL);
		fprintf(fp, S_PART);
		fprintf(fp, "    �o���D��]�G\n");
		f_suck(fp, path);
		unlink(path);
	}
	else
	{
		vmsg("�}�l�s�� [�s�p��]]");
		fd = vedit(path, 0);
		if (fd)
		{
			unlink(path);
			unlink(fpath);
			vmsg("�����s�p");
			return XO_HEAD;
		}
		nbrd.etime = etime = nbrd.btime + days * 86400;
		fp = fopen(fpath, "a+");
		fprintf(fp, "�@��: SYSOP (" SYSOPNICK ") ����: NewBoard\n");
		fprintf(fp, "���D: %s �s�p\n", buf1);
		fprintf(fp, "�ɶ�: %s\n\n", ctime(&now));
		fprintf(fp, S_PART);
		fprintf(fp, "    �s�p�D�D�G%s\n", buf1);
		fprintf(fp, "    �|�����G%s\n", nbrd.date);
		fprintf(fp, "    ����ѼơG%d\n", days);
		str_stamp(tmp, &etime);
		fprintf(fp, "    �������G%s\n", tmp);
		fprintf(fp, "    ���D�W�١G%s\n", cuser.userid);
		fprintf(fp, "    �q�l�l��H�c�G%s\n", cuser.email);
		fprintf(fp, "    �ݳs�p�H�ơG%d\n", numbers);
		fprintf(fp, S_PART);
		fprintf(fp, "    �s�p��]�G\n");
		f_suck(fp, path);
		unlink(path);
		fprintf(fp, S_PART);
		fprintf(fp, "    �}�l�s�p�G\n");
		fprintf(fp, S_AGREE_START);
		fprintf(fp, S_AGREE_END);
		fprintf(fp, S_ASSIST_START);
		fprintf(fp, S_ASSIST_END);
	}


	strcpy(nbrd.title, buf1);
	strcpy(nbrd.brdname, buf);

	strcpy(nbrd.owner, cuser.userid);

	if (mode == 1)
	{
		nbrd.mode = NBRD_NBRD;
		nbrd.total = NBRD_MAX;
		vmsg("�e��ӽФF�A�е��Ԯ֭�a�I");
	}
	else if (mode == 2)
	{
		nbrd.mode = NBRD_CANCEL;
		nbrd.total = NBRD_MAX_CANCEL;
		vmsg("�e��ӽФF�A�е��Ԯ֭�a�I");
	}
	else
	{
		nbrd.mode = NBRD_OTHER | NBRD_START;
		nbrd.total = numbers;
		vmsg("�}�l�s�p�F�I");
	}


	rec_add(dir, &nbrd, sizeof(NBRD));
	fclose(fp);

	return nbrd_init(xo);
}

static int
nbrd_seek(fpath, mail)
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

static int
nbrd_join(xo)
XO *xo;
{
	NBRD *nbrd;
	int fd, fv, lock;
	char fpath[80], buf[128], logpath[80], flocks[80];
	time_t check;
	LOG mail;

	memset(&mail, 0, sizeof(LOG));
	nbrd = (NBRD *) xo_pool + (xo->pos - xo->top);
	if (nbrd->mode & NBRD_REJECT)
	{
		vmsg("�ڵ��ӽСA��Ƥ�����I");
		return XO_FOOT;
	}
	else if (nbrd->mode & (NBRD_CLOSE | NBRD_STOP))
	{
		vmsg("����s�p�I");
		return XO_FOOT;
	}
	else if (nbrd->mode & NBRD_OPEN)
	{
		vmsg("�w�����}��");
		return XO_FOOT;
	}
	else if (!(nbrd->mode & NBRD_START))
	{
		vmsg("�|���q�L�ӽСI");
		return XO_FOOT;
	}
	else if (time(0) > nbrd->etime || nbrd->mode & NBRD_STOP)
	{
		nbrd->mode = NBRD_STOP  | (nbrd->mode & NBRD_MASK);
		rec_put(xo->dir, nbrd, sizeof(NBRD), xo->pos);
		vmsg("�s�p�w�g�I��F�A�ФU���A�ӡI");
		return XO_FOOT;
	}
	else if (nbrd->mode & NBRD_OK)
	{
		if (vmsg("�w�F��s�p�H�ơA�O�_�~��s�p [y/N]") != 'y')
			return XO_FOOT;
	}

	/* --------------------------------------------------- */
	/* �ˬd�O�_�w�g�s�p�L					 */
	/* --------------------------------------------------- */

#define	FV_SZ	(sizeof(time_t))

	usr_fpath(buf, cuser.userid, "newboard");
	fv = open(buf, O_RDWR | O_CREAT, 0600);
	f_exlock(fv);

	while (read(fv, &check, FV_SZ) == FV_SZ)
	{
		if (check == nbrd->btime)
		{
			f_unlock(fv);
			close(fv);
			vmsg("�A�w�g�s�p�L�F�I");
			return XO_FOOT;
		}
	}
	check = nbrd->btime;
	/* --------------------------------------------------- */
	/* �}�l�s�p						 */
	/* --------------------------------------------------- */

	nbrd_fpath(fpath, xo->dir, nbrd);
	sprintf(logpath, "%s.log", fpath);

	if (nbrd_seek(logpath, cuser.email))
	{
		vmsg("�A�w�g�s�p�L�F�I");
		return XO_FOOT;
	}

	more(fpath, NULL);
	sprintf(flocks, "%s.lock", fpath);
#ifdef LINUX
	lock = open(flocks, O_WRONLY | O_CREAT | O_APPEND | O_NONBLOCK, 0600);
#else
	lock = open(flocks, O_WRONLY | O_CREAT | O_APPEND | O_EXLOCK | O_NONBLOCK, 0600);
#endif
	if (lock < 0)
	{
		vmsg("���H���b�s�p���A�еy�ݡI");
		return XO_HEAD;
	}

#ifdef LINUX
	flock(lock, LOCK_EX);
#endif

	while (1)
	{
		if (nbrd->mode & NBRD_NBRD)
		{
			fd = vans("�n�[�J�s�p�� (Y)�٦� (Q)���} [y/Q]�G");
			if (fd != 'y' && fd != 'Y')
				fd = 'Q';
			break;
		}
		else
		{
			int ans;
			fd = vans("�n�[�J�s�p�� (1)�٦� (2)�Ϲ� (Q)���} [Q]�G");
			if (fd == '1')
			{
				ans = vans("�z�몺�O�٦����A�T�w�ܡH (Y)�T�w (N)���� [y/N]�G");
				fd = 'y';
			}
			else if (fd == '2')
			{
				ans = vans("�z�몺�O�Ϲﲼ�A�T�w�ܡH (Y)�T�w (N)���� [y/N]�G");
				fd = 'n';
			}
			else
			{
				fd = 'Q';
				break;
			}
			if (ans == 'y' || ans == 'Y')
				break;
		}
	}
	if (fd == 'y' || fd == 'Y' || fd == 'n' || fd == 'N')
	{
		FILE *fp, *fds;
		char say[128];
		char nfile[128];
		int  rmode;
		if (fd == 'y' || fd == 'Y')
			rmode = 1;
		else
			rmode = 2;

		sprintf(nfile, "%s.new", fpath);
		fp = fopen(fpath, "r+");
		fds = fopen(nfile, "w");

		rec_get(xo->dir, nbrd, sizeof(NBRD), xo->pos);
		if (fp)
		{
			while (fgets(buf, 128, fp))
			{
				if (rmode == 1)
				{
					if (!strncmp(buf, S_AGREE_END, strlen(S_AGREE_END)))
					{
						nbrd->agree++;
						break;
					}
				}
				else
					if (!strncmp(buf, S_ASSIST_END, strlen(S_ASSIST_END)))
					{
						nbrd->assist++;
						break;
					}
				fprintf(fds, "%s", buf);
			}
			fprintf(fds, "%3d -> %-12s: %s\n", rmode == 1 ? nbrd->agree : nbrd->assist, cuser.userid, cuser.email);
			if (vget(b_lines, 0, "�ڦ��ܭn���G", say, 65, DOECHO))
				fprintf(fds, "    %s : %s\n", cuser.userid, say);

			fprintf(fds, "%s", buf);
			while (fgets(buf, 128, fp))
				fprintf(fds, "%s", buf);
			fclose(fds);
			fclose(fp);
			unlink(fpath);
			f_mv(nfile, fpath);

			write(fv, &check, FV_SZ);
			if ((nbrd->mode & NBRD_OK) ? 0 : (rmode == 1 ? (--nbrd->total > 0) : 1))
			{
				sprintf(buf, "�[�J�s�p�����I�|�ݳs�p�H�� %d �H�C", nbrd->total);
				vmsg(buf);
			}
			else
			{
				vmsg("�w�F�s�p�зǡA���R�Լf�z�I");
				nbrd->mode = NBRD_OK  | (nbrd->mode & NBRD_MASK) | NBRD_START;
			}
			strcpy(mail.email, cuser.email);
			rec_put(xo->dir, nbrd, sizeof(NBRD), xo->pos);
			rec_add(logpath, &mail, sizeof(LOG));
		}
		else
			vmsg("�[�J�s�p����!!");
	}

#ifdef LINUX
	flock(lock, LOCK_UN);
#endif

	close(lock);
	unlink(flocks);
	f_unlock(fv);
	close(fv);
	return nbrd_head(xo);
}

static int
nbrd_start(xo)
XO *xo;
{
	NBRD *nbrd;
	char fpath[80], buf[128], tmp[10];
	time_t etime;
	if (!HAS_PERM(PERM_SYSOP | PERM_BOARD))
		return XO_NONE;


	nbrd = (NBRD *) xo_pool + (xo->pos - xo->top);
	if (nbrd->mode & ~(NBRD_MASK))
		vmsg("�w�q�L�Τw����I");
	else
	{
		nbrd_fpath(fpath, xo->dir, nbrd);
		etime = time(0) + NBRD_DAYS * 86400;

		str_stamp(tmp, &etime);
		f_cat(fpath, S_PART);
		sprintf(buf, "    �}�l�s�p�G      �������G%s\n", tmp);
		f_cat(fpath, buf);
		f_cat(fpath, S_AGREE_START);
		f_cat(fpath, S_AGREE_END);
		if (!(nbrd->mode & NBRD_NBRD))
		{
			f_cat(fpath, S_ASSIST_START);
			f_cat(fpath, S_ASSIST_END);
		}
		nbrd->etime = etime;
		nbrd->mode = NBRD_START  | (nbrd->mode & NBRD_MASK);
		rec_put(xo->dir, nbrd, sizeof(NBRD), xo->pos);
		vmsg("�ӽгq�L");
	}
	return nbrd_head(xo);
}

static int
nbrd_reject(xo)
XO *xo;
{
	NBRD *nbrd;
	char path[128], fpath[128];
	int fd;
	FILE *fp;
	if (bbsothermode & OTHERSTAT_EDITING)
	{
		vmsg("�A�٦��ɮ��٨S�s���@�I");
		return XO_FOOT;
	}


	nbrd = (NBRD *) xo_pool + (xo->pos - xo->top);
	if (!HAS_PERM(PERM_SYSOP | PERM_BOARD))
		return XO_NONE;

	if (nbrd->mode & ~(NBRD_MASK))
		vmsg("�w�q�L�Τw����I");
	else
	{
		usr_fpath(path, cuser.userid, "newboard.note");
		nbrd_fpath(fpath, xo->dir, nbrd);
		vmsg("�нs��ڵ��s�p��]");
		fd = vedit(path, 0);
		if (fd)
		{
			unlink(path);
			vmsg("����");
			return XO_HEAD;
		}

		f_cat(fpath, S_PART);
		f_cat(fpath, "    �ڵ��s�p��]�G\n\n");
		fp = fopen(fpath, "a+");
		f_suck(fp, path);
		fclose(fp);
		f_cat(fpath, S_PART);
		nbrd->mode |= NBRD_REJECT  | (nbrd->mode & NBRD_MASK);
		rec_put(xo->dir, nbrd, sizeof(NBRD), xo->pos);
		vmsg("�ڵ��ӽ�");
		unlink(path);
	}
	return nbrd_head(xo);
}

static int
nbrd_close(xo)
XO *xo;
{
	NBRD *nbrd;

	nbrd = (NBRD *) xo_pool + (xo->pos - xo->top);
	if (!HAS_PERM(PERM_SYSOP | PERM_BOARD))
		return XO_NONE;
	if (nbrd->mode & NBRD_OK)
		vmsg("�w�s�s�p�����A���������I");
	else if (nbrd->mode & NBRD_OPEN)
		vmsg("�w�����}���I");
	else if (nbrd->mode & NBRD_CLOSE)
		vmsg("�w�����s�p�I");
	else
	{
		nbrd->mode = NBRD_CLOSE  | (nbrd->mode & NBRD_MASK);
		rec_put(xo->dir, nbrd, sizeof(NBRD), xo->pos);
		vmsg("��������");
	}
	return nbrd_head(xo);
}

static int
nbrd_open(xo)
XO *xo;
{
	NBRD *nbrd;

	nbrd = (NBRD *) xo_pool + (xo->pos - xo->top);
	if (!HAS_PERM(PERM_SYSOP | PERM_BOARD))
		return XO_NONE;

	if (!(nbrd->mode & NBRD_OK))
		vmsg("�|���s�p�����A����}���I");
	else if (nbrd->mode & NBRD_OPEN)
		vmsg("�w�����}���I");
	else
	{
		nbrd->mode = NBRD_OPEN | (nbrd->mode & NBRD_MASK);
		rec_put(xo->dir, nbrd, sizeof(NBRD), xo->pos);
		vmsg("�}������");
	}
	return nbrd_head(xo);
}

#ifdef	TEST_COSIGN
static int
nbrd_zero(xo)
XO *xo;
{
	NBRD *nbrd;

	nbrd = (NBRD *) xo_pool + (xo->pos - xo->top);
	if (!HAS_PERM(PERM_SYSOP | PERM_BOARD))
		return XO_NONE;

	nbrd->mode |= NBRD_START;
	rec_put(xo->dir, nbrd, sizeof(NBRD), xo->pos);
	return nbrd_head(xo);
}
#endif

static int
nbrd_browse(xo)
XO *xo;
{
	NBRD *nbrd;
	char fpath[80];

	nbrd = (NBRD *) xo_pool + (xo->pos - xo->top);
	nbrd_fpath(fpath, xo->dir, nbrd);
	more(fpath, NULL);
	return nbrd_init(xo);
}

static int
nbrd_delete(xo)
XO *xo;
{
	NBRD *nbrd;
	char fpath[80];


	nbrd = (NBRD *) xo_pool + (xo->pos - xo->top);
	if (strcmp(cuser.userid, nbrd->owner) && !HAS_PERM(PERM_SYSOP | PERM_BOARD))
		return XO_NONE;

	if (vans("�T�w�R���� [y/N] :") != 'y')
		return XO_HEAD;
	nbrd_fpath(fpath, xo->dir, nbrd);
	unlink(fpath);
	strcat(fpath, ".log");
	unlink(fpath);
	rec_del(xo->dir, sizeof(NBRD), xo->pos, NULL, NULL);
	return nbrd_init(xo);
}

static int
nbrd_cross(xo)
XO *xo;
{
	char xboard[20], fpath[80], xfolder[80], buf[80];
	HDR xpost;
	int rc;
	FILE *fd;
	NBRD *nbrd;

	if (!HAS_PERM(PERM_ADMIN))
		return XO_NONE;

	nbrd = (NBRD *) xo_pool + (xo->pos - xo->top);

	if (ask_board(xboard, BRD_W_BIT,
				  "\n\n\033[1;33m�ЬD��A���ݪO�A������K�W�L�T�O�C\033[m\n\n")
		&& (*xboard || xo->dir[0] == 'u'))
	{
		if (*xboard == 0)
			return XO_HEAD;

		rc = vget(2, 0, "(S)�s�� (Q)�����H[Q] ", buf, 3, LCECHO);
		if (rc != 's' && rc != 'S')
			return XO_HEAD;

		nbrd_fpath(fpath, xo->dir, nbrd);
		brd_fpath(xfolder, xboard, fn_dir);

		if (!(fd = fdopen(hdr_stamp(xfolder, 'A', &xpost, buf), "w")))
			return XO_HEAD;

		f_suck(fd, fpath);
		fclose(fd);

		strcpy(xpost.owner, cuser.userid);
		strcpy(xpost.nick, cuser.username);
		memcpy(xpost.date, nbrd->date, sizeof(xpost.date));

		if (nbrd->mode & NBRD_NBRD)
			sprintf(xpost.title, "�ӽзs�� %-13s:%-22.22s", nbrd->brdname, nbrd->title);
		else if (nbrd->mode & NBRD_CANCEL)
			sprintf(xpost.title, "�o�� %s �����D", nbrd->brdname);
		else
			sprintf(xpost.title, "�s�p %-36.36s", nbrd->title);

		rec_add(xfolder, &xpost, sizeof(xpost));

		vmsg("�������");
	}
	return XO_HEAD;

}

static int
nbrd_help(xo)
XO *xo;
{
	film_out(FILM_SIGNUP, -1);
	return nbrd_head(xo);
}

KeyFunc nbrd_cb[] =
{
	{XO_INIT, nbrd_init},
	{XO_LOAD, nbrd_load},
	{XO_HEAD, nbrd_head},
	{XO_BODY, nbrd_body},

	{'j', nbrd_join},
	{'r', nbrd_browse},
	{'o', nbrd_open},
	{'s', nbrd_start},
	{'R', nbrd_reject},
	{'c', nbrd_close},
	{'d', nbrd_delete},
	{'x', nbrd_cross},
#ifdef	TEST_COSIGN
	{'z', nbrd_zero},
#endif
	{Ctrl('P'), nbrd_add},
	{'h', nbrd_help}
};

int
XoNewBoard(void)
{
	XO *xo;
	char fpath[64];
	clear();
	sprintf(fpath, "newboard/%s", fn_dir);
	xz[XZ_OTHER - XO_ZONE].xo = xo = xo_new(fpath);
	xz[XZ_OTHER - XO_ZONE].cb = nbrd_cb;
	xo->key = XZ_OTHER;
	xover(XZ_OTHER);
	free(xo);
	return 0;
}
