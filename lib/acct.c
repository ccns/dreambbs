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
/* 增加銀幣, 優良積分, 劣退                              */
/* ----------------------------------------------------- */

void addmoney(int addend, const char *userid)
{
    ACCT acct;
    if (acct_load(&acct, userid) >= 0)
    {
        double temp = (acct.money + addend);    /* 避免溢位 */
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
        double temp = (acct.point1 + addend);    /* 避免溢位 */
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
        double temp = (acct.point2 + addend);    /* 避免溢位 */
        if (temp < INT_MAX)
            acct.point2 += addend;
        acct_save(&acct);
    }
}

/* ----------------------------------------------------- */
/* (.ACCT) 使用者帳號 (account) subroutines              */
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
        f_mv(fnlog, fpath);        /* Thor.990409:可跨partition */
    }
    else
    {
        fp = fdopen(fd, "w");
        fprintf(fp, "作者: SYSOP (%s)\n標題: %s\n時間: %s\n",
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
        /* Thor.990416: 特別注意, 有時 .ACCT的長度會是0 */
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
    fd = open(fpath, O_WRONLY, 0600);    /* fpath 必須已經存在 */
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
/* 帳號管理                                              */
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
            p->tm_hour, p->tm_min, "停權", exer);
    f_cat(FN_ETC_UNTRUST_ACL, buf);
}

int add_deny_exer(ACCT *u, int adm, int cross, const char *exer)
{
    FILE *fp;
    char buf[80];
    ACCT x;
    time_t now;
    bool check_time;
    const char *cselect = NULL, *cdays = NULL, *cmode = NULL;

    memcpy(&x, u, sizeof(ACCT));
    time(&now);
    check_time = (x.deny > now);

    if (!strncmp(u->justify, "reg:", 4))
        adm = (adm & ~DENY_MODE_ALL) | DENY_MODE_GUEST;

    unlink(FN_STOP_LOG);
    fp = fopen(FN_STOP_LOG, "w");
    if (adm & DENY_SEL_OK)
    {
        x.deny = now;
        memcpy(u, &x, sizeof(x));
        acct_save(u);
        return adm;
    }
    if (adm & DENY_SEL)
    {
        if (adm & DENY_SEL_TALK)
            cselect = "不當言論";
        else if (adm & DENY_SEL_POST)
            cselect = " Cross Post";
        else if (adm & DENY_SEL_MAIL)
            cselect = "散發連鎖信";
        else if (adm & DENY_SEL_AD)
            cselect = "散發廣告信";
        else if (adm & DENY_SEL_SELL)
            cselect = "販賣非法事物";
        fprintf(fp, "查 %s 違反站規%s，依站規停止", u->userid, cselect);
    }
    if ((adm & (DENY_MODE_ALL)) && !(adm & DENY_MODE_GUEST))
    {
        if ((adm & DENY_MODE_ALL) == DENY_MODE_ALL)
        {
            x.userlevel |=
                (PERM_DENYPOST | PERM_DENYTALK | PERM_DENYCHAT | PERM_DENYMAIL
                 | PERM_DENYNICK);
            cmode = " Talk、Mail、\nPost、更改暱稱";
        }
        else if (adm & DENY_MODE_POST)
        {
            x.userlevel |= (PERM_DENYPOST);
            cmode = " Post ";
        }
        else if (adm & DENY_MODE_TALK)
        {
            x.userlevel |= (PERM_DENYTALK | PERM_DENYCHAT);
            cmode = " Talk ";
        }
        else if (adm & DENY_MODE_MAIL)
        {
            x.userlevel |= (PERM_DENYMAIL);
            cmode = " Mail ";
        }
        else if (adm & DENY_MODE_NICK)
        {
            x.userlevel |= (PERM_DENYNICK);
            cmode = "更改暱稱";
        }
        fprintf(fp, "%s權限", cmode);
    }
    if (adm & DENY_MODE_GUEST)
    {
        x.userlevel |=
            (PERM_DENYPOST | PERM_DENYTALK | PERM_DENYCHAT | PERM_DENYMAIL |
             PERM_DENYNICK | PERM_DENYSTOP);
        x.userlevel &= ~(PERM_BASIC | PERM_VALID);
        x.deny += 86400 * 31;
        cmode = " Talk、Mail、\nPost、更改暱稱";
        fprintf(fp,
                "%s權限，權限降至 guest，永不復權，並保留帳號，\n其 E-mail：%s 永不得在本站註冊。\n\n",
                cmode, u->vmail[0] ? u->vmail : "[無認證信箱]");
        deny_add_email(u, exer);
    }
    if ((adm & DENY_DAYS) && !(adm & DENY_MODE_GUEST))
    {
        x.deny = ((now > x.deny) ? now : x.deny);
        if (adm & DENY_DAYS_1)
        {
            cdays = "一星期";
            x.deny += 86400 * 7;
        }
        else if (adm & DENY_DAYS_2)
        {
            cdays = "兩星期";
            x.deny += 86400 * 14;
        }
        else if (adm & DENY_DAYS_3)
        {
            cdays = "三星期";
            x.deny += 86400 * 21;
        }
        else if (adm & DENY_DAYS_4)
        {
            cdays = "一個月";
            x.deny += 86400 * 31;
        }
        else if (adm & DENY_DAYS_5)
        {
            cdays = "";
            x.deny += 86400 * 31;
            x.userlevel |= PERM_DENYSTOP;
        }
        fprintf(fp, "%s。\n", cdays);
        if (adm & DENY_DAYS_5)
            fprintf(fp, "期間: 永不復權。\n\n");
        else
            fprintf(fp, "期間: %s%s，期限一過自動復權。\n\n",
                    check_time ? "上次處罰到期日累加" : "從今天起", cdays);
    }
    fprintf(fp,
            "\x1b[1;32m※ Origin: \x1b[1;33m%s \x1b[1;37m<%s>\x1b[m\n\x1b[1;31m◆ From: \x1b[1;36m%s\x1b[m\n",
            BOARDNAME, MYHOSTNAME, MYHOSTNAME);

    fclose(fp);
    sprintf(buf, "[%s處罰] %s %s", cross ? "連坐" : "", u->userid, cselect);
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
/* 設定 E-mail address                                   */
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
    char foo[64];                /* SoC: 放置待檢查的 email address */
    const char* str_invalid;

    static const char *const invalid[] = { "@bbs", "bbs@", "root@", "gopher@",
        "guest@", "@ppp", "@slip", "@dial", "unknown@", "@anon.penet.fi",
        "193.64.202.3", "brd@", NULL
    };

    /* SoC: 保持原 email 的大小寫 */
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
    /* Thor.981223: 將bbsreg拒絕部分分開 */
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

/* gaod:換新:p */
void check_nckuemail(char *email)
{
    char *ptr;
    ptr = strstr(email, DEFAULTSERVER);

    if (ptr)
    {
        strcpy(ptr, NCKUMAIL);
    }
}

/* 找尋是否有註冊三個以上之 Email */
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
