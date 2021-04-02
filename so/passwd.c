#include "bbs.h"

int
new_passwd(void)
{
    DL_HOLD;
    ACCT acct;
    FILE *fp;
    int ans, fd, len;
    char Email[61], passwd[PLAINPASSSIZE], *pw;

    srand(time(0));
    move(b_lines-1, 0);
    outs("��ϥΪ̧ѰO�K�X�ɡA���e�s�K�X�ܸӨϥΪ̪����U�H�c�C");
    while ((ans = acct_get(msg_uid, &acct)))
    {
        if (ans > 0)
        {
            vget(B_LINES_REF-2, 0, "�п�J�{�Үɪ� Email�G", Email, 40, DOECHO);

            if (strcmp(acct.email, Email) == 0 || strcmp(acct.vmail, Email) == 0)
            {
                if (not_addr(acct.email))
                {
                    vmsg("�� ID ��g�� E-mail �����T�A�L�k�H�X.");
                    break;
                }

                vget(B_LINES_REF-3, 0, "Email ���T�A�нT�{�O�_���ͷs�K�X�H(y/N)[N] ", Email, 2, LCECHO);
                if (Email[0] != 'y')
                    break;
                /* IID.20190530: For forward compatibility with older versions */
                if (vget(B_LINES_REF-3, 0, "�O�_�ϥηs���K�X�[�K(y/N)�H[N]", Email, 3, LCECHO) == 'y')
                {
                    ans = GENPASSWD_SHA256;
                    len = PLAINPASSSIZE;
                }
                else
                {
                    ans = GENPASSWD_DES;
                    len = OLDPLAINPASSSIZE;
                }

                getrandom_bytes(passwd, len-1);
                for (fd = 0; fd < len-1; fd++)
                {
                    passwd[fd] = (passwd[fd] % 26) + ((passwd[fd] % 52 >= 26) ? 'a' : 'A');
                }
                passwd[len-1] = '\0';
                str_scpy(acct.passwd, pw = genpasswd(passwd, ans), PASSSIZE);
                str_scpy(acct.passhash, pw + PASSSIZE, sizeof(acct.passhash));
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
    return DL_RELEASE(0);
}
