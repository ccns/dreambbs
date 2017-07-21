/*-------------------------------------------------------*/
/* classtable.c   ( YZU_CSE WindTop BBS )                */
/*-------------------------------------------------------*/
/* author : verit.bbs@bbs.yzu.edu.tw                     */
/* modify : visor.bbs@bbs.yzu.edu.tw			 */
/* target : �\�Ҫ�                                       */
/* create : 2002/01/18                                   */
/* update : NULL                                         */
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
	outz("\033[1;37;42m �i�ާ@�����ja)�s�W e)�ק� d)�R�� q)���} i)�פJ c)�M�� \033[1;30mCopyRight By verit@yzu \033[m");
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
#define SERVER_student    "isdna2.yzu.edu.tw"
#define CGI_stage1       "/student/signform.asp"
#define CGI_stage2       "/student/stdinfomain.asp"
#define CGI_stage3       "/student/GetTurStdCos.asp?m_folder=2"
#define CGI_stage4       "/academic/pubinfo/pubinfomain.asp"
#define CGI_stage5       "/academic/pubinfo/GetCosOutline.asp"
#define	CLASS_GRADUATE	 (912)

int
teacher_classtable(CLASS_TABLE2 *classtable)
{
	char atrn[256], sendform[512];
	char cookie[80], pool[2048];
	char *xhead;
	int cookie_len;
	int sockfd;
	int cc;
	char mCos_Id[80], *mCos_Class;

	/* stage1 */
	sprintf(sendform, "GET %s HTTP/1.0\r\n\r\n", CGI_stage4);
	if ((sockfd = dns_open(SERVER_student, HTTP_PORT)) < 0)
	{
		vmsg("�L�k�P���A�����o�s���A�d�ߥ���\n");
		return 1;
	}
	else
	{
//    vmsg("���b�s�����A���A�еy��(�����N�����}).............\n");
	}
	write(sockfd, sendform, strlen(sendform));
	shutdown(sockfd, 1);
	for (;;)
	{
//    *xhead = '\0';
		xhead = pool;
		cc = read(sockfd, xhead, sizeof(pool));
		if (*xhead != '\0' && cc > 0)
		{
			char *s;
			xhead[cc] = '\0';
			s = strstr(xhead, "Cookie:");
			if (s != NULL)
			{
				s[strlen(s) - strlen(strstr(s, "\n")) + 1] = '\0';
				strcpy(cookie, s);
//        vmsg(cookie);
				break;
			}
		}
		else
		{
			return 1;
			break;
		}
	}
	close(sockfd);

	/* stage2 */
	mCos_Class = strstr(classtable->condensation, "_") + 1;
	strcpy(mCos_Id, classtable->condensation);
	mCos_Id[strlen(mCos_Id)-strlen(strstr(mCos_Id, "_"))] = 0;
	sprintf(atrn, "mCos_Id=%s&mCos_Class=%s&mSmtr=%d", mCos_Id, mCos_Class, CLASS_GRADUATE);
//  vmsg(atrn);
	cookie_len = strlen(atrn);
	sprintf(sendform, "GET %s?%s HTTP/1.0\r\n%s\r\n", CGI_stage5, atrn, cookie);
	if ((sockfd = dns_open(SERVER_student, HTTP_PORT)) < 0)
	{
		vmsg("�L�k�P���A�����o�s���A�d�ߥ���\n");
		return 1;
	}
	else
	{
//    vmsg("���b�s�����A���A�еy��(�����N�����}).............\n");
	}
	write(sockfd, sendform, strlen(sendform));
	shutdown(sockfd, 1);
	for (;;)
	{
//    *xhead = '\0';
//    xhead = pool;
		cc = read(sockfd, xhead, sizeof(pool));
		if (*xhead != '\0' && cc > 0)
		{
			char *s;
			xhead[cc] = 0;
			s = strstr(xhead, "���ҦѮv");
			if (s != NULL && strlen(s) > 41)
			{
				s += 41;
				s[strlen(s) - strlen(strstr(s, " </td>"))] = 0;
				strcpy(classtable->teacher, s);
//        vmsg(classtable->teacher);
				break;
			}
			else if (s != NULL)
				break;
		}
		else
		{
			break;
		}
	}
	close(sockfd);
}

int
import_classtable(int x, int y)
{
	char atrn[256], sendform[512];
	char cookie[80], condensation[9];
	int cookie_len;
	int sockfd;
	int cc;
	char *xhead;
	char pool[2048], buf[1025];
	FILE *fp;
	char *s;
	char fname[80], classtime[80];
	char User[14], Password[14], ans, ansc;
	CLASS_TABLE2 classtable;

	if (!vget(b_lines, 0, "�b��(���[s) : " , User, sizeof(User), DOECHO) ||
		!vget(b_lines, 0, "�K�X : " , Password, sizeof(Password), NOECHO))
		return 0;

	ans = vans("�O�_�л\\��l��� [y/N]: ");
	ansc = vans("����ҵ{²�� [y/N]: ");

	/* stage1 */
	sprintf(sendform, "GET %s HTTP/1.0\r\n\r\n", CGI_stage1);
	if ((sockfd = dns_open(SERVER_student, HTTP_PORT)) < 0)
	{
		vmsg("�L�k�P���A�����o�s���A�d�ߥ���");
		return 1;
	}
	else
	{
//    vmsg("���b�s�����A���A�еy�� (1).............");
	}
	write(sockfd, sendform, strlen(sendform));
	shutdown(sockfd, 1);
	for (;;)
	{
//    *xhead = '\0';
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

	/* stage2 */
	sprintf(atrn, "User=%s&Password=%s", User, Password);
	cookie_len = strlen(atrn);
	sprintf(sendform, "POST %s HTTP/1.0\r\n%sContent-Type: application/x-www-form-urlencoded\r\nContent-Length: %d\r\n\r\n%s\r\n\r\n", CGI_stage2, cookie, cookie_len, atrn);
	if ((sockfd = dns_open(SERVER_student, HTTP_PORT)) < 0)
	{
		vmsg("�L�k�P���A�����o�s���A�d�ߥ���");
		return 1;
	}
	else
	{
//    vmsg("���b�s�����A���A�еy�� (2).............");
	}
	write(sockfd, sendform, strlen(sendform));
	shutdown(sockfd, 1);
	for (;;)
	{
//    *xhead = '\0';
		xhead = pool;
		cc = read(sockfd, xhead, sizeof(pool));
		if (*xhead != '\0' && cc > 0)
		{
			xhead[cc] = '\0';
			s = strstr(xhead, "���~");
			if (s != NULL)
			{
				vmsg("�z��J���b���αK�X���~!");
				return 1;
			}
		}
		else
		{
//      vmsg("�z��J���b���αK�X���T!");
			break;
		}
	}
	close(sockfd);

	/* stage3 */
	sprintf(sendform, "GET %s HTTP/1.0\r\n%s\r\n", CGI_stage3, cookie);
	if ((sockfd = dns_open(SERVER_student, HTTP_PORT)) < 0)
	{
		vmsg("�L�k�P���A�����o�s���A�d�ߥ���\n");
		return 1;
	}
	else
	{
//    vmsg("���b�s�����A���A�еy�� (3).............\n");
	}
	write(sockfd, sendform, strlen(sendform));
	shutdown(sockfd, 1);
	sprintf(fname, "tmp/%s.student", cuser.userid);
	fp = fopen(fname, "w");
	if (fp)
	{
		for (;;)
		{
//    *xhead = '\0';
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
			s = strstr(buf, "	<CENTER>");
			if (s != NULL)  // �ҵ{�s��
			{
				s += 9;
				s[strlen(s) - strlen(strstr(s, "</CENTER>"))] = 0;
				strcpy(classtable.condensation, s);
				fgets(buf, 1024, fp);  // �}�үZ�O
				s = strstr(buf, "<CENTER>");
				s += 8;
				s[strlen(s) - strlen(strstr(s, "</CENTER>"))] = 0;
				strcat(classtable.condensation, "_");
				strcat(classtable.condensation, s);
				fgets(buf, 1024, fp);  // ��O
				fgets(buf, 1024, fp);  // �ҦW
				s = strstr(buf, "<CENTER>");
				s += 8;
				s[strlen(s) - strlen(strstr(s, "</CENTER>"))] = 0;
				strncpy(condensation, s, 8);
				condensation[8] = '\0';
				strncpy(classtable.name, s, 28);
				strcat(classtable.name, "[");
				strcat(classtable.name, classtable.condensation);
				strcat(classtable.name, "]");
				fgets(buf, 1024, fp);  // �Ǥ���
				s = strstr(buf, "<CENTER>");
				s += 8;
				s[strlen(s) - strlen(strstr(s, "</CENTER>"))] = 0;
				sprintf(sendform, "�Ǥ��� [%s]", s);
				fgets(buf, 1024, fp);  // �������q
				s = strstr(buf, "<CENTER>");
				s += 8;
				s[strlen(s) - strlen(strstr(s, "</CENTER>"))] = 0;
				sprintf(classtable.other, "%s�A�������q [%s]", sendform, s);
				fgets(buf, 1024, fp);  // �ť�
				fgets(buf, 1024, fp);  // �ť�
				fgets(buf, 1024, fp);  // �ɶ�
				s = strstr(buf, ">");
				s += 1;
				s[strlen(s) - strlen(strstr(s, "</TD>"))] = 0;
				strcpy(classtime, s);
				fgets(buf, 1024, fp);  // �a�I
				s = strstr(buf, ">");
				s += 1;
				s[strlen(s) - strlen(strstr(s, "</TD>"))] = 0;
				if (!strstr(s, "�L���"))
					s[strlen(s) - strlen(strstr(s, ","))] = 0;
				strcpy(classtable.room, s);
				// �ɶ����R
				s = classtime;
				while (!strstr(s, "�L���") && strlen(s) > 3)
				{
					int p, x, y;
					x = atoi(s + 1);
					s[1] = '\0';
					y = atoi(s);
					p = (x - 1) + (y - 1) * 13;
					if (tmp_table[p].valid == 0 || ans == 'y' || ans == 'Y')
					{
						classtable.x = x;
						classtable.y = y;
						classtable.valid = 1;
						if (strlen(classtable.teacher) <= 0)
							teacher_classtable(&classtable);
//            vmsg(classtable.teacher);
						memcpy(&tmp_table[p], &classtable, sizeof(CLASS_TABLE2));
						if (ansc == 'y' || ansc == 'Y')
						{
							memcpy(tmp_table[p].condensation, condensation, sizeof(condensation));
							show_classtable(x, y, condensation);
						}
						else
							show_classtable(x, y, classtable.condensation);
					}
					s += 4;
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
