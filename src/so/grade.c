/*-------------------------------------------------------*/
/* so/extra.c        ( YZU WindTopBBS Ver 3.00 )         */
/*-------------------------------------------------------*/
/* author : verit.bbs@bbs.yzu.edu.tw                     */
/* target : �@���B�~���\��                               */
/* create : 2002/09/02                                   */
/*-------------------------------------------------------*/

#include "bbs.h"

/*-------------------------------------------------------*/
/* ���Z�d�ߨt��						 */
/*-------------------------------------------------------*/


int strip_self(char *fpath)
{
	FILE *fout;
	char line[80];
	char fbuf[80];
	FILE *fp;
	int state = 0;

	sprintf(fbuf, "%s.tmp", fpath);
	f_mv(fpath, fbuf);

	if ((fp = fopen(fbuf, "r")))
	{
		if ((fout = fopen(fpath, "w")))
		{
			while (fgets(line, sizeof(line), fp))
			{
				if (strcmp(line, "+------------------------------------------------+\n") == 0)
				{
					if (state == 0)
					{
						fprintf(fout, "%s", line);
						state = 1;
					}
				}
				else if (strcmp(line, "|------------------------------------------------|\n") &&
						 strcmp(line, "|                                                |\n"))
				{
					fprintf(fout, "%s", line);
					state = 0;
				}
			}

			fclose(fout);
		}
		fclose(fp);
	}
	unlink(fbuf);
}

int mail_to_self(char *fpath)
{
	FILE *fout;
	char buf[80], folder[80];
	HDR fhdr;

	usr_fpath(folder, cuser.userid, fn_dir);
	if ((fout = fdopen(hdr_stamp(folder, 0, &fhdr, buf), "w")))
	{
		f_suck(fout, fpath);
		fclose(fout);
		fhdr.xmode = MAIL_READ | MAIL_NOREPLY;
		strcpy(fhdr.title, "[�� �� ��] �������Z�d��");
		strcpy(fhdr.owner, cuser.userid);
		rec_add(folder, &fhdr, sizeof(fhdr));
	}
}

int main_grade()
{
	char id[10], pwd[32];
	char fpath[64], cmd[256];
	char ans;

	memset(id, 0, sizeof(id));
	memset(pwd, 0, sizeof(pwd));
	if (vget(b_lines, 0, "�Ǹ� : ", id + 1, 7, DOECHO) &&
		vget(b_lines, 0, "�K�X : ", pwd, sizeof(pwd), NOECHO))
	{
		id[0] = 's';

		logitfile(FN_YZUSERVICE_LOG, "<GRADE>", id);

		sprintf(fpath, "tmp/grade/%s.grade", id);
		sprintf(cmd, "php bin/portal.php %s %s | w3m -dump -T text/html > %s", id, pwd, fpath);
		system(cmd);

		strip_self(fpath);

		more(fpath, NULL);

		ans = vans("�O�_�s��H�c [y/N]�G");
		if (ans == 'y')
		{
			mail_to_self(fpath);
			vmsg("�w�g�N���G�H��A���H�c");
		}
		unlink(fpath);
	}
	return 0;
}
