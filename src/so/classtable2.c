/*-------------------------------------------------------*/
/* classtable.c   ( YZU_CSE WindTop BBS )                */
/*-------------------------------------------------------*/
/* author : verit.bbs@bbs.yzu.edu.tw                     */
/* modify : verit.bbs@bbs.yzu.edu.tw			 */
/* target : �\�Ҫ�                                       */
/* create : 2002/01/18                                   */
/* update : 2004/05/20                                   */
/*-------------------------------------------------------*/

#include "bbs.h"



static CLASS_TABLE2 tmp_table[78];
static char fpath_classtable[80];


int show_classtable(int x, int y, char *msg)
{
	y--;
	move(x + 3, 7 + y*12);
	prints("%-8s", msg);
	return 0;
}

void show_icon_classtable(int x, int y, int mode)
{
	int p;
	p = (x - 1) + (y - 1) * 13;
	y--;
	if (mode == 1)
	{
		move(x + 3, 6 + y*12);outc('[');move(x + 3, 15 + y*12);outc(']');
	}
	else
	{
		move(x + 3, 6 + y*12);outc(' ');move(x + 3, 15 + y*12);outc(' ');
	}

	move(19, 20);prints("                                ");
	move(20, 20);prints("           ");
	move(21, 20);prints("                                ");
	move(20, 55);prints("                    ");

	if (tmp_table[p].valid == 1)
	{
		move(19, 20);
		prints("%-s", tmp_table[p].name);
		move(20, 20);
		prints("%-s", tmp_table[p].teacher);
		move(20, 55);
		prints("%-s", tmp_table[p].room);
		move(21, 20);
		prints("%-s", tmp_table[p].other);
	}
	move(b_lines, 78);
	return ;
}

void help_classtable(void)
{
	outz("\033[1;37;42m �i�ާ@�����ja)�s�W e)�ק� d)�R�� q)���} i)�פJ c)�M�� \033[1;30mCopyRight By Verit@yzu \033[m");
}

int show_table(void)
{
	int i;
	int x = 1;
	int y = 0;

	vs_head("�ӤH�Ҫ�", str_site);
	move(x++, y);
	prints("�z�w�s�w�w�w�w�w�s�w�w�w�w�w�s�w�w�w�w�w�s�w�w�w�w�w�s�w�w�w�w�w�s�w�w�w�w�w�{");
	move(x++, y);
	prints("�x�@�x  �P���@  �x  �P���G  �x  �P���T  �x  �P���|�@�x�@�P�����@�x�@�P�����@�x");
	move(x++, y);
	prints("�u�w�q�w�w�w�w�w�q�w�w�w�w�w�q�w�w�w�w�w�q�w�w�w�w�w�q�w�w�w�w�w�q�w�w�w�w�w�t");
	for (i = 1;i < 14;i++)
	{
		move(x++, y);
		prints("�x%2d�x          �x          �x          �x          �x          �x          �x", i);
	}
	move(x++, y);
	prints("�|�w�r�w�w�w�w�w�r�w�w�w�w�w�r�w�w�w�w�w�r�w�w�w�w�w�r�w�w�w�w�w�r�w�w�w�w�w�}");
	move(x++, y);
	prints("�~�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w��");
	move(x++, y);
	prints("�x    �i�ҵ{�W�١j                                                          �x");
	move(x++, y);
	prints("�x    �i�½ұЮv�j                        �i�W�Ҧa�I�j                      �x");
	move(x++, y);
	prints("�x    �i��    ���j                                                          �x");
	move(x++, y);
	prints("���w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w��");
	help_classtable();
	return 0;
}

int load_table()
{
	int i;
	int fd;
	fd = open(fpath_classtable, O_RDONLY);
	if (fd >= 0)
	{
		read(fd, tmp_table, sizeof(CLASS_TABLE2)*78);
		close(fd);
	}
	else
	{
		memset(tmp_table, 0, sizeof(CLASS_TABLE2)*78);
	}

	for (i = 0;i < 78;i++)
	{
		if (tmp_table[i].valid == 1)
		{
			show_classtable(tmp_table[i].x, tmp_table[i].y, tmp_table[i].condensation);
		}
	}
	return 0;
}

int add_classtable(int x, int y)
{
	CLASS_TABLE2 classtable;
	int p;

	p = (x - 1) + (y - 1) * 13;

	if (tmp_table[p].valid == 1)
		return 0;
	classtable.y = y;
	classtable.x = x;

	if (vget(b_lines, 0, "�ҵ{²�� : " , classtable.condensation, sizeof(classtable.condensation), DOECHO))
	{
		vget(b_lines, 0, "�ҵ{�W�� : " , classtable.name, sizeof(classtable.name), DOECHO);
		vget(b_lines, 0, "�½ұЮv : " , classtable.teacher, sizeof(classtable.teacher), DOECHO);
		vget(b_lines, 0, "�W�Ҧa�I : " , classtable.room, sizeof(classtable.room), DOECHO);
		vget(b_lines, 0, "��    �� : " , classtable.other, sizeof(classtable.other), DOECHO);


		memcpy(&tmp_table[p], &classtable, sizeof(CLASS_TABLE2));
		tmp_table[p].valid = 1;
		show_classtable(x, y, classtable.condensation);
	}
	show_icon_classtable(x, y, 1);
	help_classtable();
	return 0;
}

int del_classtable(int x, int y)
{
	int p;
	p = (x - 1) + (y - 1) * 13;

	if (vans("�O�_�n�R�� �H [y/N]") != 'y')
	{
		help_classtable();
		return 0;
	}
	else
	{
		memset(&tmp_table[p], 0, sizeof(CLASS_TABLE2));
		show_classtable(x, y, "         ");
	}
	help_classtable();
	return 0;
}

int edit_classtable(int x, int y)
{
	int p;
	CLASS_TABLE2 classtable;

	p = (x - 1) + (y - 1) * 13;

	if (tmp_table[p].valid == 1)
	{
		memcpy(&classtable, &tmp_table[p], sizeof(CLASS_TABLE2));
		if (vget(b_lines, 0, "�ҵ{²�� : " , classtable.condensation, sizeof(classtable.condensation), GCARRY))
		{
			vget(b_lines, 0, "�ҵ{�W�� : " , classtable.name, sizeof(classtable.name), GCARRY);
			vget(b_lines, 0, "�½ұЮv : " , classtable.teacher, sizeof(classtable.teacher), GCARRY);
			vget(b_lines, 0, "�W�Ҧa�I : " , classtable.room, sizeof(classtable.room), GCARRY);
			vget(b_lines, 0, "��    �� : " , classtable.other, sizeof(classtable.other), GCARRY);
			memcpy(&tmp_table[p], &classtable, sizeof(CLASS_TABLE2));

			show_classtable(x, y, classtable.condensation);
		}
	}
	show_icon_classtable(x, y, 1);
	help_classtable();
	return 0;
}

#define HTTP_PORT         80
#define SERVER_student    "portal.yzu.edu.tw"
#define CGI_stage1	  "/logincheck.asp"
#define CGI_stage2	  "/"
#define CGI_stage3        "/VC/classLeft.asp"
#define CGI_stage4        "/VC/Login_Student.asp"

int
import_classtable(int x, int y)
{
	char sendform[1024], sendform2[1024];
	char cookie[80];
	int sockfd;
	int cc;
	char *xhead;
	char pool[2048], buf[1025];
	FILE *fp;
	char *s;
	char fname[80];
	char User[7], Password[25], ans;
	CLASS_TABLE2 classtable;

	if (!vget(b_lines, 0, "�b��(���[s) : " , User, sizeof(User), DOECHO) ||
		!vget(b_lines, 0, "�K�X : " , Password, sizeof(Password), NOECHO))
		return 0;

	ans = vans("�O�_�л\\��l��� [y/N]: ");

	/* -------------------------- */
	/*  ���o Cookie		*/
	/* -------------------------- */
	sprintf(sendform, "GET %s HTTP/1.0\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: 0\r\n\r\n", CGI_stage2);

	if ((sockfd = dns_open(SERVER_student, HTTP_PORT)) < 0)
	{
		vmsg("�L�k�P���A�����o�s���A�d�ߥ���\n");
		return 1;
	}

	write(sockfd, sendform, strlen(sendform));
	shutdown(sockfd, 1);

	for (;;)
	{
		xhead = pool;
		cc = read(sockfd, xhead, sizeof(pool));
		if (*xhead != '\0' && cc > 0)
		{
			xhead[cc] = '\0';
			s = strstr(xhead, "Cookie:");
			if (s != NULL)
			{
				s[strlen(s) - strlen(strstr(s, "\n")) + 1] = '\0';
				strcpy(cookie, s);
				break;
			}
		}
		else
		{
			vmsg("MISS");
			return 1;
		}
	}
	close(sockfd);

	/* -------------------------- */
	/*  Login			*/
	/* -------------------------- */
	sprintf(sendform2, "uid=s%s&pwd=%s&ok=�T�w", User, Password);
	sprintf(sendform, "POST %s HTTP/1.0\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: %d\r\n%s\r\n%s\r\n", CGI_stage1, strlen(sendform2), cookie, sendform2);

	if ((sockfd = dns_open(SERVER_student, HTTP_PORT)) < 0)
	{
		vmsg("�L�k�P���A�����o�s���A�d�ߥ���\n");
		return 1;
	}

	write(sockfd, sendform, strlen(sendform));
	shutdown(sockfd, 1);

	for (;;)
	{
		xhead = pool;
		cc = read(sockfd, xhead, sizeof(pool));
		if (*xhead != '\0' && cc > 0)
		{
			xhead[cc] = '\0';
		}
		if (cc <= 0)
		{
			break;
		}
	}
	close(sockfd);

	/* -------------------------- */
	/*  �i�J�ǲ��ɮ�		*/
	/* -------------------------- */
	sprintf(sendform, "GET %s HTTP/1.1\r\n%sHost: portal.yzu.edu.tw\r\nReferer: http://portal.yzu.edu.tw/VC/left_index.asp\r\n\r\n", CGI_stage4, cookie);


	if ((sockfd = dns_open(SERVER_student, HTTP_PORT)) < 0)
	{
		vmsg("�L�k�P���A�����o�s���A�d�ߥ���\n");
		return 1;
	}

	write(sockfd, sendform, strlen(sendform));
	shutdown(sockfd, 1);
	for (;;)
	{
		xhead = pool;
		cc = read(sockfd, xhead, sizeof(pool));
		if (*xhead != '\0' && cc > 0)
		{
			xhead[cc] = '\0';
		}
		if (cc <= 0)
		{
			break;
		}
	}
	close(sockfd);

	/* -------------------------- */
	/*  ���o��ҲM��		*/
	/* -------------------------- */
	sprintf(sendform, "GET %s HTTP/1.1\r\n%sHost: portal.yzu.edu.tw\r\nReferer: http://portal.yzu.edu.tw/VC/Login_Student.asp\r\n\r\n", CGI_stage3, cookie);
//  sprintf(sendform, "GET %s HTTP/1.1\r\n%sHost: portal.yzu.edu.tw\r\n\r\n", CGI_stage3, cookie);


	if ((sockfd = dns_open(SERVER_student, HTTP_PORT)) < 0)
	{
		vmsg("�L�k�P���A�����o�s���A�d�ߥ���\n");
		return 1;
	}

	write(sockfd, sendform, strlen(sendform));
	shutdown(sockfd, 1);
	sprintf(fname, "tmp/%s.student", cuser.userid);
	fp = fopen(fname, "w");
	fputs(sendform, fp);
	if (fp)
	{
		for (;;)
		{
			xhead = pool;
			cc = read(sockfd, xhead, sizeof(pool));
			if (*xhead != '\0' && cc > 0)
			{
				xhead[cc] = '\0';
				fputs(xhead, fp);
			}
			if (cc <= 0)
			{
				break;
			}
		}
		fclose(fp);
	}
	close(sockfd);


	/* stage4 */
	fp = fopen(fname, "r");
	memset(&classtable, 0, sizeof(CLASS_TABLE2));
	if (fp)
	{
		while (fgets(buf, 1024, fp) != NULL)
		{
			s = strstr(buf, "�ҵ{�W��");
			if (s != NULL)  // �ҵ{�W��
			{
				s += 10;
				s[strlen(s) - strlen(strstr(s, "�j"))] = 0;
				strcpy(classtable.name, s);
				strncpy(classtable.condensation, s, sizeof(classtable.condensation) - 1);
				fgets(buf, 1024, fp);
				fgets(buf, 1024, fp);
				s = strstr(buf, "�½ұЮv");
				s += 10;
				s[strlen(s) - strlen(strstr(s, "�j"))] = 0;
				strcat(classtable.teacher, s);

				while (fgets(buf, 1024, fp))
				{
					if ((s = strstr(buf, "�]RM")))
					{
						int p, x, y, num;
						char *pp;

						*s = 0;
						pp = strstr(s + 2, "�^");
						num = atoi(buf);
						y = num / 100;
						x = num % 100;

						p = (x - 1) + (y - 1) * 13;
						if (tmp_table[p].valid == 0 || ans == 'y' || ans == 'Y')
						{
							classtable.x = x;
							classtable.y = y;
							classtable.valid = 1;

							*pp = 0;
							strcpy(classtable.room, s + 2);
							memcpy(&tmp_table[p], &classtable, sizeof(CLASS_TABLE2));
							show_classtable(x, y, classtable.condensation);
						}

						if (strstr(pp + 3 , "pimindex.asp"))
							break;
					}
				}
				memset(&classtable, 0, sizeof(CLASS_TABLE2));
			}
		}
		fclose(fp);
		unlink(fname);
	}
	show_icon_classtable(x, y, 1);
	help_classtable();
	return 0;
}


int init_classtable()
{
	show_table();
	load_table();
	return 0;
}

int main_classtable(void)
{
	char c;
	int x = 1, y = 1;
	int fd;

#ifdef	HAVE_CLASSTABLEALERT
	classtable_free();
#endif
	usr_fpath(fpath_classtable, cuser.userid, FN_CLASSTABLE2);

	init_classtable();
	show_icon_classtable(x, y, 1);

	do
	{
		c = vkey();
		switch (c)
		{
		case 'q' :
			break;
		case KEY_DOWN :
			show_icon_classtable(x, y, 0);
			x++;
			if (x == 14) x = 1;
			show_icon_classtable(x, y, 1);
			break;
		case KEY_UP:
			show_icon_classtable(x, y, 0);
			x--;
			if (x == 0) x = 13;
			show_icon_classtable(x, y, 1);
			break;
		case KEY_LEFT:
			show_icon_classtable(x, y, 0);
			y--;
			if (y == 0) y = 6;
			show_icon_classtable(x, y, 1);
			break;
		case KEY_RIGHT :
			show_icon_classtable(x, y, 0);
			y++;
			if (y == 7) y = 1;
			show_icon_classtable(x, y, 1);
			break;
		case 'a' :
			add_classtable(x, y);
			break;
		case 'd' :
			del_classtable(x, y);
			break;
		case 'e' :
			edit_classtable(x, y);
			break;
		case 'i' :
			import_classtable(x, y);
			break;
		case 'c' :
			if (vans("�T�w�R���ܡH[y/N] ") == 'y')
			{
				memset(tmp_table, 0, sizeof(CLASS_TABLE2)*78);
				show_table();
			}
			break;
		case '\n' :
			if (tmp_table[x+y*13-14].valid == 1)
				edit_classtable(x, y);
			else
				add_classtable(x, y);
			break;
		}
	}
	while (c != 'q');

	fd = open(fpath_classtable, O_WRONLY | O_CREAT | O_TRUNC, 0600);
	if (fd >= 0)
	{
		write(fd, tmp_table, sizeof(CLASS_TABLE2)*78);
		close(fd);
	}
#ifdef	HAVE_CLASSTABLEALERT
	classtable_main();
#endif

	vmsg(NULL);
	return 0;
}
