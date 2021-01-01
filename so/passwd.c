#include "bbs.h"

int
new_passwd(void)
{
    DL_HOLD;
    ACCT acct;
    FILE *fp;
    int ans, fd, len;
    char Email[61], passwd[PLAINPASSLEN], *pw;

    srand(time(0));
    move(b_lines-1, 0);
    outs("當使用者忘記密碼時，重送新密碼至該使用者的註冊信箱。");
    while ((ans = acct_get(msg_uid, &acct)))
    {
        if (ans > 0)
        {
            vget(B_LINES_REF-2, 0, "請輸入認證時的 Email：", Email, 40, DOECHO);

            if (strcmp(acct.email, Email) == 0 || strcmp(acct.vmail, Email) == 0)
            {
                if (not_addr(acct.email))
                {
                    vmsg("此 ID 填寫之 E-mail 不正確，無法寄出.");
                    break;
                }

                vget(B_LINES_REF-3, 0, "Email 正確，請確認是否產生新密碼？(y/N)[N] ", Email, 2, LCECHO);
                if (Email[0] != 'y')
                    break;
                /* IID.20190530: For forward compatibility with older versions */
                if (vget(B_LINES_REF-3, 0, "是否使用新式密碼加密(y/N)？[N]", Email, 3, LCECHO) == 'y')
                {
                    ans = GENPASSWD_SHA256;
                    len = PLAINPASSLEN;
                }
                else
                {
                    ans = GENPASSWD_DES;
                    len = OLDPLAINPASSLEN;
                }

                getrandom_bytes(passwd, len-1);
                for (fd = 0; fd < len-1; fd++)
                {
                    passwd[fd] = (passwd[fd] % 26) + ((passwd[fd] % 52 >= 26) ? 'a' : 'A');
                }
                passwd[len-1] = '\0';
                str_scpy(acct.passwd, pw = genpasswd(passwd, ans), PASSLEN);
                str_scpy(acct.passhash, pw + PASSLEN, sizeof(acct.passhash));
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
                fprintf(fp, BOARDNAME "新密碼 : %s\n", passwd);

                fclose(fp);

                bsmtp(Email, BOARDNAME "新密碼", acct.email, 0);

                unlink(Email);

                vmsg("新密碼已寄出.");
            }
            else
            {
                vmsg("Email 錯誤...");
                break;
            }
        }
    }
    return DL_RELEASE(0);
}
