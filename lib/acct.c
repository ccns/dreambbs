/*-------------------------------------------------------*/
/* lib/acct.c   ( NCKU CCNS WindTop-DreamBBS 3.21 )      */
/*-------------------------------------------------------*/
/* Author: ???                                           */
/* Target: User account structure library for DreamBBS   */
/* Create: 95/03/29 (as maple/acct.c)                    */
/* Update: 2021-05-20 (split from maple/acct.c)          */
/*       : by Wei-Cheng Yeh (IID) <iid@ccns.ncku.edu.tw> */
/*-------------------------------------------------------*/

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "global_def.h"
#include "struct.h"
#include "timetype.h"
#include "dao.h"

/* ----------------------------------------------------- */
/* �W�[�ȹ�, �u�}�n��, �H�h                              */
/* ----------------------------------------------------- */

void addmoney(int addend, const char *userid)
{
    ACCT acct;
    if (acct_load(&acct, userid) >= 0)
    {
        double temp = (acct.money + addend);    /* �קK���� */
        if (temp < INT_MAX)
            acct.money += addend;
        else
        {
            acct.money = (INT_MAX - 1);
        }
        acct_save(&acct);
    }
}

void addpoint1(int addend, const char *userid)
{
    ACCT acct;
    if (acct_load(&acct, userid) >= 0)
    {
        double temp = (acct.point1 + addend);    /* �קK���� */
        if (temp < INT_MAX)
            acct.point1 += addend;
        acct_save(&acct);
    }
}

void addpoint2(int addend, const char *userid)
{
    ACCT acct;
    if (acct_load(&acct, userid) >= 0)
    {
        double temp = (acct.point2 + addend);    /* �קK���� */
        if (temp < INT_MAX)
            acct.point2 += addend;
        acct_save(&acct);
    }
}

/* ----------------------------------------------------- */
/* (.ACCT) �ϥΪ̱b�� (account) subroutines              */
/* ----------------------------------------------------- */

void keeplog(const char *fnlog, const char *board, const char *title, int mode    /* 0:load 1:rename 2:load+unlink 3:load+mark 4:rename+mark */
    )
{
    HDR hdr;
    char folder[128], fpath[128];
    int fd;
    FILE *fp;

    if (!board)
        board = BRD_SYSTEM;

    sprintf(folder, "brd/%s/.DIR", board);
    fd = hdr_stamp(folder, 'A', &hdr, fpath);
    if (fd < 0)
        return;

    if (mode == 1 || mode == 4)
    {
        close(fd);
        /* rename(fnlog, fpath); */
        f_mv(fnlog, fpath);        /* Thor.990409:�i��partition */
    }
    else
    {
        fp = fdopen(fd, "w");
        fprintf(fp, "�@��: SYSOP (%s)\n���D: %s\n�ɶ�: %s\n",
                SYSOPNICK, title, ctime_any(&hdr.chrono));
        f_suck(fp, fnlog);
        fclose(fp);
        if (mode == 2)
            unlink(fnlog);
    }

    strcpy(hdr.title, title);
    strcpy(hdr.owner, "SYSOP");
    strcpy(hdr.nick, SYSOPNICK);
    if (mode == 3 || mode == 4)
        hdr.xmode |= POST_MARKED;
    fd = open(folder, O_WRONLY | O_CREAT | O_APPEND, 0600);
    if (fd < 0)
    {
        unlink(fpath);
        return;
    }
    write(fd, &hdr, sizeof(HDR));
    close(fd);
}


int acct_load(ACCT *acct, const char *userid)
{
    int fd;

    usr_fpath((char *)acct, userid, FN_ACCT);
    fd = open((char *)acct, O_RDONLY);
    if (fd >= 0)
    {
        /* Thor.990416: �S�O�`�N, ���� .ACCT�����׷|�O0 */
        read(fd, acct, sizeof(ACCT));
        close(fd);
    }
    return fd;
}

void acct_save(const ACCT *acct)
{
    int fd;
    char fpath[80];

    usr_fpath(fpath, acct->userid, FN_ACCT);
    fd = open(fpath, O_WRONLY, 0600);    /* fpath �����w�g�s�b */
    if (fd >= 0)
    {
        write(fd, acct, sizeof(ACCT));
        close(fd);
    }
}


int acct_userno(const char *userid)
{
    int fd;
    int userno;
    char fpath[80];

    usr_fpath(fpath, userid, FN_ACCT);
    fd = open(fpath, O_RDONLY);
    if (fd >= 0)
    {
        read(fd, &userno, sizeof(userno));
        close(fd);
        return userno;
    }
    return 0;
}

/* ----------------------------------------------------- */
/* �b���޲z                                              */
/* ----------------------------------------------------- */

int seek_log_email(const char *mail, int mode)
{
    EMAIL email;
    int pos = 0, fd;
    fd = open(FN_VIOLATELAW_DB, O_RDONLY);
    while (fd >= 0)
    {
        lseek(fd, (off_t) (sizeof(email) * pos), SEEK_SET);
        if (read(fd, &email, sizeof(email)) == sizeof(email))
        {
            if (!strcmp(email.email, mail)
                && (mode ? (email.deny > time(0) || email.deny == -1) : 1))
            {
                close(fd);
                return pos;
            }
            pos++;
        }
        else
        {
            close(fd);
            break;
        }
    }
    return -1;
}

void deny_log_email(const char *mail, time_t deny)
{
    EMAIL email;
    int pos;
    pos = seek_log_email(mail, 0);
    if (pos >= 0)
    {
        rec_get(FN_VIOLATELAW_DB, &email, sizeof(EMAIL), pos);
        if (deny > email.deny || deny == -1)
            email.deny = deny;
        email.times++;
        rec_put(FN_VIOLATELAW_DB, &email, sizeof(EMAIL), pos);
    }
    else
    {
        memset(&email, 0, sizeof(email));
        strcpy(email.email, mail);
        email.deny = deny;
        rec_add(FN_VIOLATELAW_DB, &email, sizeof(EMAIL));
    }
}

static void deny_add_email(ACCT *he, const char *exer)
{
    char buf[128];
    time_t now;
    struct tm *p;

    time(&now);

    p = localtime(&now);
    str_lower(he->vmail, he->vmail);
    sprintf(buf, "%s # %02d/%02d/%02d %02d:%02d %s (%s)\n",
            he->vmail,
            p->tm_year % 100, p->tm_mon + 1, p->tm_mday,
            p->tm_hour, p->tm_min, "���v", exer);
    f_cat(FN_ETC_UNTRUST_ACL, buf);
}

int add_deny_exer(ACCT *u, int adm, int cross, const char *exer)
{
    FILE *fp;
    char buf[80];
    ACCT x;
    time_t now;
    const char *cselect;
    char *ptr = buf;

    memcpy(&x, u, sizeof(ACCT));
    time(&now);

    if (!strncmp(u->justify, "reg:", 4))
        adm = (adm & ~DENY_MODE_ALL) | DENY_MODE_GUEST;

    unlink(FN_STOP_LOG);
    fp = fopen(FN_STOP_LOG, "w");

    /* The reason for suspension */
    switch (adm & DENY_SEL)
    {
    case DENY_SEL_TALK:
        cselect = "������";
        break;
    case DENY_SEL_POST:
        cselect = " Cross Post";
        break;
    case DENY_SEL_MAIL:
        cselect = "���o�s��H";
        break;
    case DENY_SEL_AD:
        cselect = "���o�s�i�H";
        break;
    case DENY_SEL_SELL:
        cselect = "�c��D�k�ƪ�";
        break;
    case DENY_SEL_NONE:
    default: /* Unknown reasons */
        cselect = NULL;
    }
    if (cselect)
        fprintf(fp, "�d %s �H�ϯ��W%s�A�̯��W", u->userid, cselect);

    /* Permissions to be suspended */
    if (adm & DENY_MODE)
    {
        typedef struct {
            int deny_mode;
            int perm;
            const char *name;
        } SusPerm ;
        static const SusPerm sus_perm[] = {
            {DENY_MODE_POST, PERM_DENYPOST, " Post "},
            {DENY_MODE_TALK, PERM_DENYTALK | PERM_DENYCHAT, " Talk "},
            {DENY_MODE_TALK_PERM, PERM_DENYTALK, " Talk (�O�d Chat �v��) "},
            {DENY_MODE_CHAT, PERM_DENYCHAT, " Chat "},
            {DENY_MODE_MAIL, PERM_DENYMAIL, " Mail "},
            {DENY_MODE_NICK, PERM_DENYNICK, "���ʺ�"},
            {DENY_MODE_UNUSED7, 0, NULL},
        };

        const char *br = ptr; /* The character right after the last line break */
        int deny_mode = adm & DENY_MODE;
        for (const SusPerm *sus = sus_perm; sus < sus_perm + COUNTOF(sus_perm); ++sus)
        {
            if ((deny_mode & sus->deny_mode) == sus->deny_mode)
            {
                deny_mode &= ~sus->deny_mode;
                if (!sus->perm)
                {
                    /* Unused flags */
                    continue;
                }
                x.userlevel |= sus->perm;
                if (ptr > buf)
                {
                    /* Remove spaces around fullwidth punctuation marks */
                    if (!IS_DBCS_TRAIL(buf, ptr - buf) && ptr[-1] == ' ')
                        --ptr;
                    ptr = str_pcpy(ptr, "�B");
                    if (ptr - br > 14)
                        br = ptr = str_pcpy(ptr, "\n");
                    ptr = str_pcpy(ptr, sus->name + (sus->name[0] == ' '));
                }
                else
                {
                    ptr += sprintf(ptr, "����%s", sus->name);
                }
            }
        }
        if (ptr > buf)
            ptr = str_pcpy(ptr, "�v��");
        else
            ptr = strcpy(buf, "");
        if (adm & DENY_MODE_LEVEL)
        {
            x.userlevel &= ~(PERM_BASIC | PERM_VALID);
            if (ptr > buf)
                ptr = str_pcpy(ptr, "�A");
            if (ptr - br > 21)
                br = ptr = str_pcpy(ptr, "\n");
            ptr = str_pcpy(ptr, "�v������ guest ");
        }

    }
    if (DENY_DAYS_ADM(adm) > 0)
    {
        const int days = DENY_DAYS_ADM(adm);
        char *cdays = ptr;
        bool check_time;
        if (adm & DENY_DAYS_RESET)
        {
            /* Non-accumulative suspension duration */
            x.deny = now;
            check_time = false;
        }
        else
        {
            x.deny = ((now > x.deny) ? now : x.deny);
            check_time = (x.deny > now);
        }
        x.deny += 86400 * days;
        if (adm & DENY_DAYS_PERM)
            x.userlevel |= PERM_DENYSTOP;

        if (!(adm & DENY_DAYS_PERM))
        {
            switch (days)
            {
            case 7:
                strcpy(cdays, "�@�P��");
                break;
            case 14:
                strcpy(cdays, "��P��");
                break;
            case 21:
                strcpy(cdays, "�T�P��");
                break;
            case 31:
                strcpy(cdays, "�@�Ӥ�");
                break;
            default:
                /* Collapse consecutive spaces */
                if (ptr > buf && !IS_DBCS_TRAIL(buf, ptr - buf) && ptr[-1] == ' ')
                    --cdays;
                sprintf(cdays, " %d ��", days);
            }
            ptr = cdays;
        }
        /* Remove spaces before fullwidth punctuation marks */
        if (ptr > buf && !IS_DBCS_TRAIL(buf, ptr - buf) && ptr[-1] == ' ')
            strcpy(--ptr, "");
        if (adm & DENY_MODE_LEVEL)
            fprintf(fp, "%s�A", buf);
        else
            fprintf(fp, "%s�C\n����: ", buf);
        if (adm & DENY_DAYS_PERM)
        {
            fprintf(fp, "�ä��_�v");
            if (adm & DENY_MODE_LEVEL)
                fprintf(fp, "�A�ëO�d�b��");
        }
        else
        {
            fprintf(fp, "%s%s�A�����@�L�۰ʴ_�v", check_time ? "�W���B�@�����֥[" : "�q���Ѱ_", ptr);
            if (adm & DENY_MODE_LEVEL)
                fprintf(fp, "�A�B�����s�{��");
        }
    }
    if (adm & DENY_MODE_VMAIL)
    {
        if (ptr > buf)
            fprintf(fp, "�A");
        fprintf(fp, "\n�� E-mail�G%s �ä��o�b�������U",
            u->vmail[0] ? u->vmail : "[�L�{�ҫH�c]");
        deny_add_email(u, exer);
    }
    fprintf(fp, "�C\n\n");

    /* Check whether any suspension is performed */
    if (!(cselect && (adm & DENY_MODE) && DENY_DAYS_ADM(adm) > 0))
    {
        if (adm & DENY_DAYS_RESET)
        {
            /* Lift the previous suspension */
            memcpy(&x, u, sizeof(x)); /* Discard any permission changes */
            x.deny = now;
            memcpy(u, &x, sizeof(x));
            acct_save(u);
            /* Now the permissions will be reset at the next time the user logs in */
        }
        /* Discard any permission changes and the generated post */
        fclose(fp);
        return adm;
    }

    fprintf(fp,
            "\x1b[1;32m�� Origin: \x1b[1;33m%s \x1b[1;37m<%s>\x1b[m\n\x1b[1;31m�� From: \x1b[1;36m%s\x1b[m\n",
            BOARDNAME, MYHOSTNAME, MYHOSTNAME);

    fclose(fp);
    sprintf(buf, "[%s�B�@] %s %s", cross ? "�s��" : "", u->userid, cselect + (cselect[0] == ' '));
    keeplog(FN_STOP_LOG, BRD_VIOLATELAW, buf, 3);
    usr_fpath(buf, x.userid, FN_STOPPERM_LOG);
    fp = fopen(buf, "a+");
    f_suck(fp, FN_STOP_LOG);
    fclose(fp);

    memcpy(u, &x, sizeof(x));
    acct_save(u);
    return adm;
}

/* ----------------------------------------------------- */
/* �]�w E-mail address                                   */
/* ----------------------------------------------------- */

/* Tag logger */

static TLogger ban_addr_tlogger = TLOGGER_DEFAULT;

void ban_addr_tlogger_init(const TLogger *tlogger)
{
    ban_addr_tlogger = (tlogger) ? *tlogger : TLOGGER_DEFAULT;
}

int ban_addr(const char *addr)
{
    int i;
    char *host, *str GCC_UNUSED;
    char foo[64];                /* SoC: ��m���ˬd�� email address */
    const char* str_invalid;

    static const char *const invalid[] = { "@bbs", "bbs@", "root@", "gopher@",
        "guest@", "@ppp", "@slip", "@dial", "unknown@", "@anon.penet.fi",
        "193.64.202.3", "brd@", NULL
    };

    /* SoC: �O���� email ���j�p�g */
    str_lower(foo, addr);

    for (i = 0; (str_invalid = invalid[i]); i++)
    {
        if (strstr(foo, str_invalid))
            return 1;
    }

    /* check for mail.acl (lower case filter) */

    host = (char *)strchr(foo, '@');
    *host = '\0';
    /* i = acl_has(FN_ETC_SPAMMER_ACL, foo, host + 1); */
    /* Thor.981223: �Nbbsreg�ڵ��������} */
    i = acl_has(FN_ETC_UNTRUST_ACL, foo, host + 1);
    /* *host = '@'; */
    if (i < 0)
        logger_tag(&ban_addr_tlogger, "NOACL", host);
    return i > 0;
}

int allow_addr(const char *addr)
{
    int i;
    char *host;
    char foo[64];

    str_lower(foo, addr);

    host = (char *)strchr(foo, '@');
    *host = '\0';
    i = acl_has(FN_ETC_ALLOW_ACL, foo, host + 1);
    return i > 0;
}

/* gaod:���s:p */
void check_nckuemail(char *email)
{
    char *ptr;
    ptr = strstr(email, DEFAULTSERVER);

    if (ptr)
    {
        strcpy(ptr, NCKUMAIL);
    }
}

/* ��M�O�_�����U�T�ӥH�W�� Email */
/* mode : 1.find 2.add 3.del */
int find_same_email(const char *mail, int mode)
{
    int pos = 0, fd;
    const char *fpath;
    SAME_EMAIL email;


    fpath = FN_ETC_EMAILADDR_ACL;

    if (mode >= 1 && mode <= 3)
    {
        fd = open(fpath, O_RDONLY);
        pos = 0;
        while (fd >= 0)
        {
            lseek(fd, (off_t) (sizeof(SAME_EMAIL) * pos), SEEK_SET);
            if (read(fd, &email, sizeof(SAME_EMAIL)) == sizeof(SAME_EMAIL))
            {
                if (!strcmp(mail, email.email))
                    break;
                pos++;
            }
            else
            {
                pos = -1;
                break;
            }
        }
        if (fd >= 0)
            close(fd);
    }


    if (mode == 1)
    {
        if (pos >= 0)
            return email.num;
        else
            return 0;
    }
    if (mode == 2)
    {
        if (pos == -1)
        {
            memset(&email, 0, sizeof(SAME_EMAIL));
            strcpy(email.email, mail);
            email.num = 1;
            rec_add(fpath, &email, sizeof(SAME_EMAIL));
        }
        else
        {
            email.num++;
            rec_put(fpath, &email, sizeof(SAME_EMAIL), pos);
        }
    }
    if (mode == 3)
    {
        if (pos == -1)
            return 0;
        if (email.num == 1)
        {
            rec_del(fpath, sizeof(SAME_EMAIL), pos, NULL, NULL);
        }
        else
        {
            email.num--;
            rec_put(fpath, &email, sizeof(SAME_EMAIL), pos);
        }
    }
    return 0;
}
