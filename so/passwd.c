#include "bbs.h"

int
new_passwd(void)
{
    ACCT acct;
    FILE *fp;
    int ans, fd;
    char Email[61], passwd[PLAINPASSLEN], *pw;

    srand(time(0));
    move(22, 0);
    outs("當使用者忘記密碼時，重送新密碼至該使用者的註冊信箱。");
    while ((ans = acct_get(msg_uid, &acct)))
    {
        if (ans > 0)
        {
            vget(21, 0, "請輸入認證時的 Email：", Email, 40, DOECHO);

            if (strcmp(acct.email, Email) == 0 || strcmp(acct.vmail, Email) == 0)
            {
                if (not_addr(acct.email))
                {
                    vmsg("此 ID 填寫之 E-mail 不正確，無法寄出.");
                    break;
                }

                vget(22, 0, "Email 正確，請確認是否產生新密碼？(Y/N)[N] ", Email, 2, LCECHO);
                if (Email[0] != 'y')
                    break;
                getrandom_bytes(passwd, PLAINPASSLEN-1);
                for (fd = 0; fd < PLAINPASSLEN-1; fd++)
                {
                    passwd[fd] = (passwd[fd] % 26) + ((passwd[fd] % 52 >= 26) ? 'a' : 'A');
                }
                passwd[PLAINPASSLEN-1] = '\0';
                str_ncpy(acct.passwd, pw = genpasswd(passwd, GENPASSWD_SHA256), PASSLEN);
                str_ncpy(acct.passhash, pw + PASSLEN, sizeof(acct.passhash));
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
    return 0;
}
