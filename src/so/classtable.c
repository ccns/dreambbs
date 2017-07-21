#include "bbs.h"

CLOCK START[] = {{8, 10}, {9, 10}, {10, 10}, {11, 10}, {12, 10}, {13, 10}, {14, 10}, {15, 10}, {16, 10}, {17, 10}, {18, 30}, {19, 30}, {20, 30}};
CLOCK END[] = {{9, 0}, {10, 0}, {11, 0}, {12, 0}, {13, 0}, {14, 0}, {15, 0}, {16, 0}, {17, 0}, {18, 0}, {19, 20}, {20, 20}, {21, 20}};

static int
table_show(table)
CLASS_TABLE *table;
{
	int i, j;
	FILE *fp;
	char *clock[] = {" �@ ", " �G ", " �T ", " �| ", " �� ", " �� ", " �C ", " �K ", " �E ", " �Q ", "�Q�@", "�Q�G", "�Q�T"};
	char buf[80];

	memcpy(table->time.start, START, sizeof(CLOCK)*13);
	memcpy(table->time.end, END, sizeof(CLOCK)*13);

	usr_fpath(buf, cuser.userid, "classtable.log");
	fp = fopen(buf, "w");
	fprintf(fp, "           �P���@    �P���G    �P���T    �P���|    �P����    �P����\n");
	for (i = 0;i <= 12;i++)
	{
		fprintf(fp, "��%s�`  ", clock[i]);
		for (j = 0;j <= 5;j++)
		{
			fprintf(fp, "%-8.8s  ", table->table[j][i].name);
		}
		fprintf(fp, "\n  %02d:%02d   ", table->time.start[i].hour, table->time.start[i].min);
		for (j = 0;j <= 5;j++)
		{
			fprintf(fp, "%-8.8s  ", table->table[j][i].teacher);
		}
		fprintf(fp, "\n   ��     ");
		for (j = 0;j <= 5;j++)
		{
			fprintf(fp, "%-8.8s  ", table->table[j][i].class);
		}
		fprintf(fp, "\n  %02d:%02d   ", table->time.end[i].hour, table->time.end[i].min);
		for (j = 0;j <= 5;j++)
		{
			fprintf(fp, "%-8.8s  ", table->table[j][i].obj_id);
		}
		fprintf(fp, "\n\n");

	}
	fclose(fp);
	more(buf, NULL);
	return 0;
}


static void
add_table(class)
CLASS *class;
{
	CLASS tmp;
	int echo;
	if (*(class->name))
	{
		memcpy(&tmp, class, sizeof(CLASS));
		echo = GCARRY;
	}
	else
	{
		memset(&tmp, 0, sizeof(CLASS));
		echo = DOECHO;
	}
	vget(4, 0, "�ҦW�G", tmp.name, sizeof(tmp.name), echo);
	vget(5, 0, "�Юv�G", tmp.teacher, sizeof(tmp.teacher), echo);
	vget(6, 0, "�ЫǡG", tmp.class, sizeof(tmp.class), echo);
	vget(7, 0, "�Ҹ��G", tmp.obj_id, sizeof(tmp.obj_id), echo);
	if (vans("�T�w�ܡH [Y/n] ") != 'n')
		memcpy(class, &tmp, sizeof(CLASS));
}

static int
table_mail(table)
CLASS_TABLE *table;
{
	char folder[128], fpath[128];
	HDR mhdr;

	usr_fpath(fpath,  cuser.userid, "classtable.log");
	usr_fpath(folder, cuser.userid, fn_dir);
	hdr_stamp(folder, HDR_LINK, &mhdr, fpath);

	mhdr.xmode = MAIL_READ | MAIL_HOLD;
	strcpy(mhdr.owner, "[�\\�Ҫ�]");
	strcpy(mhdr.nick, cuser.username);
	strcpy(mhdr.title, "�ӤH�\\�Ҫ�");

	rec_add(folder, &mhdr, sizeof(HDR));
	return 0;
}

static int
table_add(table)
CLASS_TABLE *table;
{
	int i, j;
	char tmp[80];

	clear();
	vs_bar("�s�W�ӤH�\\�Ҫ�");
	vget(2, 0, "�W�Үɶ��G", tmp, 4, DOECHO);
	i = *tmp - '1';
	j = atoi(tmp + 1) - 1;
	if (i > 5 || i < 0 || j > 12 || j < 0)
	{
		vmsg("�W�L�d��I");
		return 0;
	}

	add_table(&(table->table[i][j]));

	return 0;
}

static void
show_class(x, y, class)
int x, y;
CLASS *class;
{
	move(x, y);
	prints("%s", class->name);
	move(x + 1, y);
	prints("%s", class->teacher);
	move(x + 2, y);
	prints("%s", class->class);
	move(x + 3, y);
	prints("%s", class->obj_id);
}

static int
table_copy(table)
CLASS_TABLE *table;
{
	int i, j, x, y;
	char tmp[80];

	clear();
	vs_bar("�ӤH�\\�Ҫ�");
	prints("�ӷ��G");
	vget(2, 0, "�W�Үɶ��G", tmp, 4, DOECHO);
	i = *tmp - '1';
	j = atoi(tmp + 1) - 1;
	if (i > 5 || i < 0 || j > 12 || j < 0)
	{
		vmsg("�W�L�d��I");
		return 0;
	}
	show_class(2, 40, &(table->table[i][j]));
	move(8, 0);
	prints("�ت��G");
	vget(9, 0, "�W�Үɶ��G", tmp, 4, DOECHO);
	x = *tmp - '1';
	y = atoi(tmp + 1) - 1;
	if (x > 5 || x < 0 || y > 12 || y < 0)
	{
		vmsg("�W�L�d��I");
		return 0;
	}
	show_class(9, 40, &(table->table[x][y]));

	if (vans("�T�w�ܡH [y/N] ") == 'y')
		memcpy(&(table->table[x][y]), &(table->table[i][j]), sizeof(CLASS));

	return 0;
}

static int
table_del(table)
CLASS_TABLE *table;
{
	int i, j;
	char tmp[80];

	clear();
	vs_bar("�R���ӤH�\\�Ҫ�");
	vget(2, 0, "�W�Үɶ��G", tmp, 4, DOECHO);
	i = *tmp - '1';
	j = atoi(tmp + 1) - 1;
	if (i > 5 || i < 0 || j > 12 || j < 0)
	{
		vmsg("�W�L�d��I");
		return 0;
	}
	show_class(2, 40, &(table->table[i][j]));
	if (vans("�T�w�ܡH [y/N] ") == 'y')
		memset(&(table->table[i][j]), 0, sizeof(CLASS));

	return 0;
}


int
ClassTable()
{
	CLASS_TABLE mytable, *ptr;
	char ans, buf[80];

	ptr = &mytable;

	usr_fpath(buf, cuser.userid, "classtable");
	if (rec_get(buf, ptr, sizeof(CLASS_TABLE), 0))
		memset(&mytable, 0, sizeof(CLASS_TABLE));

	for (;;)
	{
		ans = vans("�Ҫ�t�� A)�s�W C)�ƻs D)�R�� S)�]�w P)�q�X V)�s�� M)�H�c Q)���} [Q]");
		switch (ans)
		{
		case 'm':
			table_mail(ptr);
			break;
		case 'd':
			table_del(ptr);
			break;
		case 'v':
			rec_put(buf, ptr, sizeof(CLASS_TABLE), 0);
			break;
		case 'a':
			table_add(ptr);
			break;
		case 'c':
			table_copy(ptr);
			break;
		case 'p':
			table_show(ptr);
			break;
		default:
			return 0;
		}
	}
}
