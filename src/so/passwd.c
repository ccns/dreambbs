#include "bbs.h"

int
new_passwd()
{
	ACCT acct;
	FILE *fp;
	int ans, fd;
	char Email[61], passwd[9];

	srand(time(0));
	move(22, 0);
	outs("��ϥΪ̧ѰO�K�X�ɡA���e�s�K�X�ܸӨϥΪ̪����U�H�c�C");
	while ((ans = acct_get(msg_uid, &acct)))
	{
		if (ans > 0)
		{
			vget(21, 0, "�п�J�{�Үɪ� Email�G", Email, 40, DOECHO);

			if (strcmp(acct.email, Email) == 0 || strcmp(acct.vmail, Email) == 0)
			{
				if (not_addr(acct.email))
				{
					vmsg("�� ID ��g�� E-mail �����T, �L�k�H�X.");
					break;
				}

				vget(22, 0, "Email ���T�A�нT�{�O�_���ͷs�K�X�H(Y/N)[N] ", Email, 2, LCECHO);
				if (Email[0] != 'y')
					break;
				for (fd = 0 ; fd < 8 ; fd++)
				{
					passwd[fd] = (rand() % 26) + ((rand() % 2 == 1) ? 'a' : 'A');
				}
				passwd[8] = '\0';
				str_ncpy(acct.passwd, genpasswd(passwd), PASSLEN);
				acct_save(&acct);
				do
				{
					strcpy(Email, "tmp/sendpass");
					Email[12] = (random() % 10) + '0';
					Email[13] = (random() % 10) + '0';
					Email[14] = (random() % 10) + '0';
					Email[15] = '\0';
					fd = open(Email, O_WRONLY | O_CREAT | O_EXCL, 0600);
				}
				while (fd < 0);
				fp = fdopen(fd, "w");
				fprintf(fp, BOARDNAME "ID : %s\n\n", acct.userid);
				fprintf(fp, BOARDNAME "�s�K�X : %s\n", passwd);

				fclose(fp);
				close(fd);

				bsmtp(Email, BOARDNAME "�s�K�X", acct.email, 0);

				unlink(Email);

				vmsg("�s�K�X�w�H�X.");
			}
			else
			{
				vmsg("Email ���~...");
				break;
			}
		}
	}
	return 0;
}
