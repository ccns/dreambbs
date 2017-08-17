/*-------------------------------------------------------*/
/* so/graduate.c           ( YZU WindTopBBS Ver 3.00 )   */
/*-------------------------------------------------------*/
/* author : verit.bbs@bbs.yzu.edu.tw                     */
/* target : 畢業學分檢查                                 */
/* create : 2002/12/12                                   */
/*-------------------------------------------------------*/


#undef	_MODES_C_
#include "bbs.h"

extern XZ xz[];


#define	REF_SERVER	"140.138.2.234"
#define	HTTP_PORT	80

typedef struct GINDEX
{
	int graduate;
	int total;
	char major[32];
	char title[80];
} GINDEX;

typedef struct GPOINT
{
	int   year;     //年度
	int   semester; //學期
	char  kind[16]; //科別
	char  cname[8]; //課號
	char  name[32]; //課名
	int   level;    //學分
	int   grade;	  //成績
} GPOINT ;


static int
get_html(char *server, char *s, char *fpath)
{
	int sockfd , cc, fd;
	char buf[128];
	if ((sockfd = dns_open(server, HTTP_PORT)) < 0)
	{
		vmsg("無法與伺服器取得連結，查詢失敗");
		return 0;
	}
	write(sockfd, s, strlen(s));
	shutdown(sockfd, 1);
	fd = open(fpath , O_WRONLY | O_CREAT);
	for (;;)
	{
		cc = read(sockfd , buf , sizeof(buf));
		if (cc <= 0)
			break;
		write(fd, buf, cc);
	}
	close(fd);
	close(sockfd);
	chmod(fpath, S_IRUSR | S_IWUSR);
	return 1;
}



static void
gpoint_item(num, gpoint)
int num;
GPOINT *gpoint;
{
	if (gpoint->year < 0)
		prints("%6d            %-14s        %-20s%27s\n", num, gpoint->kind, gpoint->name, "\033[31;1mUNPASS!!\033[m");
	else
		prints("%6d   %2d   %d   %-14s%-8s%-30s %d %3d\n", num, gpoint->year, gpoint->semester,
			   gpoint->kind, gpoint->cname, gpoint->name, gpoint->level, gpoint->grade);
}

static int
gpoint_body(xo)
XO *xo;
{
	GPOINT *gpoint;
	int num, max, tail;

	move(3, 0);
	clrtobot();
	max = xo->max;
	if (max <= 0)
	{
		vmsg("目前沒有資料");
		return XO_QUIT;
	}

	gpoint = (GPOINT *) xo_pool;
	num = xo->top;
	tail = num + XO_TALL;
	if (max > tail)
		max = tail;

	do
	{
		gpoint_item(++num, gpoint++);
	}
	while (num < max);

	return XO_NONE;
}


static int
gpoint_head(xo)
XO *xo;
{
	vs_head("畢業學分檢查程式", str_site);
	outs("\
		 [←]離開 [q] 查詢累計成績 [m] 備份到自己信箱\n\
		 \033[30;47m  編號  學年學期  課    別      課號    課                名        學分 成績 \033[m");
	return gpoint_body(xo);
}


static int
gpoint_load(xo)
XO *xo;
{
	xo_load(xo, sizeof(GPOINT));
	return gpoint_body(xo);
}


static int
gpoint_init(xo)
XO *xo;
{
	xo_load(xo, sizeof(GPOINT));
	return gpoint_head(xo);
}


static int
gpoint_query(xo)
XO *xo;
{
	char fpath[128];
	char *tmp;

	strcpy(fpath, xo->dir);
	tmp = strstr(fpath, ".xo");
	*tmp = 0;
	strcat(fpath, ".total");
	more(fpath, NULL);

	return XO_INIT;
}

static int
gpoint_help(xo)
XO *xo;
{
	return XO_NONE;
}

int gpoint_mail(xo)
XO *xo;
{
	FILE *fout;
	int fd, pos;
	char buf[80], folder[80], *tmp;
	HDR fhdr;
	GPOINT gpoint;

	if (vans("是否要備份到自己信箱(Y/N)[N] ") != 'y')
		return XO_FOOT;

	usr_fpath(folder, cuser.userid, fn_dir);
	if ((fout = fdopen(hdr_stamp(folder, 0, &fhdr, buf), "w")))
	{
		fprintf(fout, "作者: SYSOP ("SYSOPNICK")\n標題: %s\n時間: %s\n",
				"畢業學分檢查", ctime(&fhdr.chrono));


		fd = open(xo->dir, O_RDONLY);
		pos = 0;
		while (fd)
		{
			lseek(fd, (off_t)(sizeof(gpoint) * pos), SEEK_SET);
			if (read(fd, &gpoint, sizeof(gpoint)) == sizeof(gpoint))
			{
				++pos;
				if (gpoint.year < 0)
					fprintf(fout, "%4d            %-14s        %-20s%27s\n", pos, gpoint.kind, gpoint.name, "\033[31;1mUNPASS!!\033[m");
				else
					fprintf(fout, "%4d   %2d   %d   %-14s%-8s%-30s %d %3d\n", pos, gpoint.year, gpoint.semester,
							gpoint.kind, gpoint.cname, gpoint.name, gpoint.level, gpoint.grade);
			}
			else
				break;
		}

		strcpy(buf, xo->dir);
		tmp = strstr(buf, ".xo");
		*tmp = 0;
		strcat(buf, ".total");

		f_suck(fout, buf);

		fprintf(fout, "\n\n\033[33;1m    此結果僅供參考之用，一切以學校公佈的為準，如有造成各位任何形式的損失，\033[m\n    \033[33;1m請自行負責。\033[m\n");
		fprintf(fout, "\n\033[33;1m    本資料庫由 visor 所維護，如有任何問題請洽 visor.bbs@bbs.yzu.edu.tw。\033[m\n");

		fprintf(fout, "\n\n--\n\033[1;32m※ Origin: \033[33m%s \n\033[1;32m※ From  : \033[37m<%s>\033[m\n",
				MYHOSTNAME, "www.yzu.edu.tw");


		close(fd);
		fclose(fout);
	}

	fhdr.xmode = MAIL_READ | MAIL_NOREPLY;
	strcpy(fhdr.title, "[備 忘 錄] 畢業學分檢查");
	strcpy(fhdr.owner, cuser.userid);
	rec_add(folder, &fhdr, sizeof(fhdr));

	return XO_FOOT;
}



KeyFunc gpoint_cb[] =
{
	{XO_INIT, gpoint_init},
	{XO_LOAD, gpoint_load},
	{XO_HEAD, gpoint_head},
	{XO_BODY, gpoint_body},

	{'q', gpoint_query},
	{'m', gpoint_mail},
	{'h', gpoint_help}
};


int
show(char *f)
{
	XO *xo;
	char fpath[64];

	utmp_mode(M_OMENU);
	sprintf(fpath, "%s.xo", f);
	xz[XZ_OTHER - XO_ZONE].xo = xo = xo_new(fpath);
	xz[XZ_OTHER - XO_ZONE].cb = gpoint_cb;
	xover(XZ_OTHER);
	free(xo);
	return 0;
}



int
handle(char *fpath)
{
	FILE *fp, *fp2;
	int fd;
	char buf[256], fpath2[80], fpath3[80];
	char *tmp;
	GPOINT gpoint;
	int state;

	sprintf(fpath2, "%s.xo", fpath);
	fp = fopen(fpath, "r");
	if (!fp)
	{
		vmsg("檔案處理錯誤 !!");
		return 0;
	}
	fd = open(fpath2, O_WRONLY | O_CREAT | O_TRUNC, 0600);
	if (fd < 0)
	{
		fclose(fp);
		vmsg("檔案處理錯誤 !!");
		return 0;
	}
	sprintf(fpath3, "%s.total", fpath);
	fp2 = fopen(fpath3, "w");
	if (!fp2)
	{
		fclose(fp);
		close(fd);
		vmsg("檔案處理錯誤 !!");
		return 0;
	}

	state = 0;

	fprintf(fp2, "\n%15s%-15s%8s%8s%8s\n", " ", "   項   目", "學分", "修滿", "剩餘");
	fprintf(fp2, "%11s===============================================\n", " ");

	while (fgets(buf, sizeof(buf), fp))
	{
		if (!strncmp(buf, "<start>", 6))
			state = 1;
		else if (!strncmp(buf, "<next>", 5))
			state = 2;
		else if (!strncmp(buf, "</end>", 5))
			break;
		else if (!strncmp(buf, "<error>", 6))
		{
			fclose(fp);
			fclose(fp2);
			close(fd);

			return -2;
		}
		else if (state == 0)
			continue ;
		else if (state == 1)
		{
			memset(&gpoint, 0, sizeof(gpoint));
			tmp = strtok(buf, "\t\n");
			gpoint.year = atoi(tmp);
			tmp = strtok(NULL, "\t\n");
			gpoint.semester = atoi(tmp);
			tmp = strtok(NULL, "\t\n");
			strncpy(gpoint.kind, tmp, 14);
			tmp = strtok(NULL, "\t\n");
			strcpy(gpoint.cname, tmp);
			tmp = strtok(NULL, "\t\n");
			strncpy(gpoint.name, tmp, 28);
			tmp = strtok(NULL, "\t\n");
			gpoint.level = atoi(tmp);
			tmp = strtok(NULL, "\t\n");
			gpoint.grade = atoi(tmp);
			write(fd, &gpoint, sizeof(gpoint));
		}
		else if (state == 2)
		{
			tmp = strtok(buf, "\t\n");
			fprintf(fp2, "%-15s%-14s", " ", tmp);
			tmp = strtok(NULL, "\t\n");
			fprintf(fp2, "%8d", atoi(tmp));
			tmp = strtok(NULL, "\t\n");
			fprintf(fp2, "%8d", atoi(tmp));
			tmp = strtok(NULL, "\t\n");
			fprintf(fp2, "%8d\n", atoi(tmp));
		}
	}

	fclose(fp);
	fclose(fp2);
	close(fd);
	return 1;
}



static int
gindex_check(xo)
XO *xo;
{
	char user[7];
	char pass[24];
	char query[512];
	char fpath[128];
	int state;

	GINDEX *gindex;
	int pos, cur;

	pos = xo->pos;
	cur = pos - xo->top;


	gindex = (GINDEX *) xo_pool + cur;

	if (vget(b_lines, 0, "您的學號：", user, sizeof(user), DOECHO) &&
		vget(b_lines, 0, "您的密碼：", pass, sizeof(pass), NOECHO))
	{
		logitfile(FN_YZUSERVICE_LOG, "<GRADUATE>", user);

		sprintf(fpath, "tmp/graduate/%s.html", cuser.userid);
		memset(query, 0, sizeof(query));
		sprintf(query, "GET /~visor/graduate/bbsgraduate.php?major=%s&user=%s&pass=%s&graduate=%d HTTP/1.0\n\n", gindex->major, user, pass, gindex->graduate);
		outz("系統搜尋中 ........... ");

		if (!get_html(REF_SERVER, query, fpath))
		{
			vmsg("遠端連線失敗");
			return XO_QUIT;
		}

		state = handle(fpath) ;

		if (state == 1)
			show(fpath);
		else if (state == -2)
			vmsg("您的密碼或學號有錯誤，請檢查看看");

		strcpy(query, fpath);
		unlink(fpath);
		strcat(fpath, ".xo");
		unlink(fpath);
		strcat(query, ".total");
		unlink(query);
	}

	return XO_QUIT;
}

static void
gindex_item(num, gindex)
int num;
GINDEX *gindex;
{
	prints("%6d %6d     %-6s %-40s %4d\n", num, gindex->graduate, gindex->major, gindex->title, gindex->total);
}

static int
gindex_body(xo)
XO *xo;
{
	GINDEX *gindex;
	int num, max, tail;

	move(3, 0);
	clrtobot();
	max = xo->max;
	if (max <= 0)
	{
		vmsg("目前沒有資料");
		return XO_QUIT;
	}

	gindex = (GINDEX *) xo_pool;
	num = xo->top;
	tail = num + XO_TALL;
	if (max > tail)
		max = tail;

	do
	{
		gindex_item(++num, gindex++);
	}
	while (num < max);

	return XO_NONE;
}


static int
gindex_head(xo)
XO *xo;
{
	vs_head("畢業學分檢查程式", str_site);
	outs("\
		 [←]離開 [Enter] 開始查詢\n\
		 \033[30;47m  編號 入學年度 系所代碼 系所名稱                                  畢業學分   \033[m");
	return XO_BODY;
}


static int
gindex_load(xo)
XO *xo;
{
	xo_load(xo, sizeof(GINDEX));
	return XO_BODY;
}


static int
gindex_init(xo)
XO *xo;
{
	xo_load(xo, sizeof(GINDEX));
	return XO_HEAD;
}


static int
gindex_help(xo)
XO *xo;
{
	return XO_NONE;
}

KeyFunc gindex_cb[] =
{
	{XO_INIT, gindex_init},
	{XO_LOAD, gindex_load},
	{XO_HEAD, gindex_head},
	{XO_BODY, gindex_body},

	{'r', gindex_check},
	{'h', gindex_help}
};


int
show_index(char *f)
{
	XO *xo;
	char fpath[64];

	utmp_mode(M_OMENU);
	sprintf(fpath, "%s.xo", f);
	xz[XZ_OTHER - XO_ZONE].xo = xo = xo_new(fpath);
	xz[XZ_OTHER - XO_ZONE].cb = gindex_cb;
	xover(XZ_OTHER);
	free(xo);
	return 0;
}


int
handle_index(char *fpath)
{
	FILE *fp;
	int fd;
	char buf[256], fpath2[80];
	char *tmp;
	GINDEX gindex;
	int state = 0;

	sprintf(fpath2, "%s.xo", fpath);
	fp = fopen(fpath, "r");
	if (!fp)
	{
		vmsg("檔案處理錯誤 !!");
		return 0;
	}
	fd = open(fpath2, O_WRONLY | O_CREAT | O_TRUNC, 0600);
	if (fd < 0)
	{
		fclose(fp);
		vmsg("檔案處理錯誤 !!");
		return 0;
	}


	while (fgets(buf, sizeof(buf), fp))
	{
		if (strstr(buf, "<start>"))
		{
			state = 1;
			continue;
		}
		else if (strstr(buf, "<end>"))
		{
			state = 2;
			break;
		}

		if (state == 1)
		{
			memset(&gindex, 0, sizeof(GINDEX));
			tmp = strtok(buf, "\t\n");
			strncpy(gindex.major, tmp, 32);
			tmp = strtok(NULL, "\t\n");
			gindex.graduate = atoi(tmp);
			tmp = strtok(NULL, "\t\n");
			strncpy(gindex.title, tmp, 80);
			tmp = strtok(NULL, "\t\n");
			gindex.total = atoi(tmp);
			write(fd, &gindex, sizeof(GINDEX));
		}
	}

	fclose(fp);
	close(fd);
	return 1;
}


int
main_gpoint()
{
//	char user[7];
//	char pass[24];
	char query[512];
	char fpath[128];
	int state;

	sprintf(fpath, "tmp/graduate/%s.index", cuser.userid);
	memset(query, 0, sizeof(query));
	sprintf(query, "GET /~visor/graduate/bbsindex.php HTTP/1.0\n\n"/*, user, pass*/);
	outz("系統搜尋中 ........... ");
	if (!get_html(REF_SERVER, query, fpath))
	{
		vmsg("遠端連線失敗");
		return -1;
	}

	state = handle_index(fpath) ;

	if (state == 1)
		show_index(fpath);

	strcpy(query, fpath);
	unlink(fpath);
	strcat(fpath, ".xo");

	unlink(fpath);


	return 0;
}
