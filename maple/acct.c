/*-------------------------------------------------------*/
/* acct.c       ( NTHU CS MapleBBS Ver 3.00 )            */
/*-------------------------------------------------------*/
/* target : account / administration routines            */
/* create : 95/03/29                                     */
/* update : 96/04/05                                     */
/*-------------------------------------------------------*/

#define ADMIN_C

#include "bbs.h"

extern XZ xz[];
extern BCACHE *bshm;


#undef  CHANGE_USERNO
#undef  CHANGE_SECOND

#define STR_PERM      "bctpjm#x--------PTCMSNL*B#KGACBS"

/* log admin command by statue@WindTop */
void logitfile(const char *file, const char *key, const char *msg)
{
    time_t now;
    struct tm *p;
    char buf[256];

    time(&now);
    p = localtime(&now);
    sprintf(buf, "%02d/%02d/%02d %02d:%02d:%02d %s %-14s %s\n",
            p->tm_year % 100, p->tm_mon + 1, p->tm_mday,
            p->tm_hour, p->tm_min, p->tm_sec, key, cuser.userid,
            msg ? msg : "");
    f_cat(file, buf);
}

/* ----------------------------------------------------- */
/* 增加銀幣, 優良積分, 劣退                              */
/* ----------------------------------------------------- */

void addmoney(int addend, char *userid)
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

void addpoint1(int addend, char *userid)
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

void addpoint2(int addend, char *userid)
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

void keeplog(const char *fnlog, const char *board, const char *title, int mode    /* 0:load 1:rename 2:unlink 3:mark */
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

    if (mode == 1)
    {
        close(fd);
        /* rename(fnlog, fpath); */
        f_mv(fnlog, fpath);        /* Thor.990409:可跨partition */
    }
    else
    {
        fp = fdopen(fd, "w");
        fprintf(fp, "作者: SYSOP (%s)\n標題: %s\n時間: %s\n",
                SYSOPNICK, title, ctime(&hdr.chrono));
        f_suck(fp, fnlog);
        fclose(fp);
        close(fd);
        if (mode == 2)
            unlink(fnlog);
    }

    strcpy(hdr.title, title);
    strcpy(hdr.owner, "SYSOP");
    strcpy(hdr.nick, SYSOPNICK);
    if (mode == 3)
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


int acct_load(ACCT * acct, char *userid)
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

void acct_save(ACCT * acct)
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


int acct_userno(char *userid)
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
/* name complete for user ID                             */
/* ----------------------------------------------------- */
/* return value :                                        */
/* 0 : 使用直接按 enter ==> cancel                       */
/* -1 : bad user id                                      */
/* o.w.: 傳回該 userid 之 userno                         */
/* ----------------------------------------------------- */

int acct_get(const char *msg, ACCT * acct)
{
    if (!vget(1, 0, msg, acct->userid, IDLEN + 1, GET_USER))
        return 0;

    if (acct_load(acct, acct->userid) >= 0)
        return acct->userno;

    vmsg(err_uid);
    return -1;
}


/* ----------------------------------------------------- */
/* 設定系統檔案                                          */
/* ----------------------------------------------------- */

void x_file(int mode,            /* M_XFILES / M_UFILES */
            const char *xlist[], /* description list */
            const char *flist[]  /* filename list */
    )
{
    int n, i;
    const char *fpath, *desc;
    char buf[80];

    n = 0;
    if (mode == M_UFILES)
        move(MENU_YPOS, 0);
    else
        move(1, 0);
    clrtobot();

    while ((desc = xlist[n]))
    {
        n++;

        if (mode == M_UFILES)
        {
            move(n + MENU_YPOS - 1, MENU_XPOS + 2);
            clrtoeol();
            move(n + MENU_YPOS - 1, MENU_XPOS + 2);
            prints("(\x1b[1;36m%d\x1b[m) %s", n, desc);
        }
        else
        {
            if (n < 21)            /* statue.000703: 註解: 一個畫面只能 show 20 個資料 */
                move_ansi(n + ((b_lines-21) >> 1), 2);
            else
                move_ansi(n + ((b_lines-21) >> 1) - 20, 2 + ((b_cols+1) >> 1));

            prints("(\x1b[1;36m%2d\x1b[m) %s", n, desc);


            if (mode == M_XFILES)
            {
                if (n < 21)
                    move_ansi(n + ((b_lines-21) >> 1), 24 + (d_cols >> 2));    /* Thor.980806: 註解: 印出檔名 */
                else
                    move_ansi(n + ((b_lines-21) >> 1) - 20, 24 + (d_cols >> 2) + ((b_cols+1) >> 1));
                outs(flist[n - 1] + 4);    /* statue.000703: 註解: +4 去掉目錄 */
                clrtoeol();
            }

        }
    }

    vget(b_lines, 0, "請選擇檔案編號，或按 [0] 取消：", buf, 3, DOECHO);
    i = atoi(buf);
    if (i <= 0 || i > n)
        return;

    n = vget(b_lines, 36, "(D)刪除 (E)編輯 (V)瀏覽 [Q]取消？", buf, 3, LCECHO);
    if (n != 'd' && n != 'e' && n != 'v')
        return;

    fpath = flist[--i];
    if (mode == M_UFILES)
        usr_fpath(buf, cuser.userid, fpath);
    else                        /* M_XFILES */
        sprintf(buf, "%s", fpath);

    if (n == 'd')
    {
        unlink(buf);
    }
    else if (n == 'e')
    {
        if (bbsothermode & OTHERSTAT_EDITING)
            vmsg("你還有檔案還沒編完哦！");
        else
            vmsg(vedit(buf, NA) ? "原封不動" : "更新完畢");
    }                            /* Thor.981020: 注意被talk的問題  */
    else if (n == 'v')
    {
        more(buf, NULL);
    }
}

int check_admin(char *name)
{
    ADMIN admin;
    int pos = 0, fd;

    if (!str_cmp(cuser.userid, ELDER))
        return 1;

    fd = open(FN_ETC_ADMIN_DB, O_RDONLY);
    while (fd)
    {
        lseek(fd, (off_t) (sizeof(admin) * pos), SEEK_SET);
        if (read(fd, &admin, sizeof(admin)) == sizeof(admin))
        {
            if (!strcmp(admin.name, name))
            {
                close(fd);
                return 1;
            }
            pos++;
        }
        else
        {
            close(fd);
            return 0;
        }
    }
    return 0;
}

/* ----------------------------------------------------- */
/* bit-wise display and setup                            */
/* ----------------------------------------------------- */

void bitmsg(const char *msg, const char *str, int level)
{
    int cc;

    outs(msg);
    while ((cc = *str))
    {
        outc((level & 1) ? cc : '-');
        level >>= 1;
        str++;
    }

    outc('\n');
}


unsigned int bitset(unsigned int pbits, int count,    /* 共有幾個選項 */
                    int maxon,    /* 最多可以 enable 幾項 */
                    const char *msg, const char *perms[])
{
    int i, j, on;

    extern char radix32[32];

    move(2, 0);
    clrtobot();
    outs(msg);

    for (i = on = 0, j = 1; i < count; i++)
    {
        msg = "□";
        if (pbits & j)
        {
            on++;
            msg = "■";
        }
        move(5 + (i & 15), (i < 16 ? 0 : 40));
        prints("%c %s %s", radix32[i], msg, perms[i]);
        j <<= 1;
    }

    while ((i = vans("請按鍵切換設定，或按 [Return] 結束：")))
    {
        i -= '0';
        if (i >= 10)
            i -= 'a' - '0' - 10;

        if (i >= 0 && i < count)
        {
            j = 1 << i;
            if (pbits & j)
            {
                on--;
                msg = "□";
            }
            else
            {
                if (on >= maxon)
                    continue;
                on++;
                msg = "■";
            }

            pbits ^= j;
            move(5 + (i & 15), (i < 16 ? 2 : 42));
            outs(msg);
        }
    }
    return (pbits);
}


static unsigned int setperm(unsigned int level)
{
    if (HAS_PERM(PERM_SYSOP))
        return bitset(level, NUMPERMS, NUMPERMS, MSG_USERPERM, perm_tbl);

    /* [帳號管理員] 不能管 SYSOP */

    if (level & PERM_SYSOP)
        return level;

    return bitset(level, NUMPERMS - 5, NUMPERMS - 5, MSG_USERPERM, perm_tbl);
}


/* ----------------------------------------------------- */
/* 帳號管理                                              */
/* ----------------------------------------------------- */

/* BLACK SU */
static void acct_su(ACCT * u)
{
    XO *xo;
    char path[80], id[20];
    int level, ufo;

    if (!supervisor)
    {
        vmsg("◎ 你不是超級站務！");
        return;
    }
    ufo = cuser.ufo;
    level = cuser.userlevel;
    memcpy(&cuser, u, sizeof(ACCT));
    cuser.userlevel = level;
    cuser.ufo = ufo;
    str_lower(id, u->userid);
    sprintf(path, "usr/%c/%s/.DIR", *id, id);
    xz[XZ_MBOX - XO_ZONE].xo = xo = xo_new(path);
    free(xo);
    sprintf(path, "usr/%c/%s/bmw", *id, id);
    //  xo = xz[XZ_BMW - XO_ZONE].xo;
    //  xz[XZ_BMW - XO_ZONE].xo =  xo_new(path);
    xz[XZ_BMW - XO_ZONE].xo = xo = xo_new(path);
    free(xo);
    pal_cache();
}

/* BLACK SU */

static void bm_list(            /* 顯示 userid 是哪些板的板主 */
                       char *userid)
{
    int len, ch;
    BRD *bhdr, *tail;
    char *list;
    extern BCACHE *bshm;

    len = strlen(userid);
    outs("擔任板主：");

    bhdr = bshm->bcache;
    tail = bhdr + bshm->number;

    do
    {
        list = bhdr->BM;
        ch = *list;
        if ((ch > ' ') && (ch < 128))
        {
            do
            {
                if (!str_ncmp(list, userid, len))
                {
                    ch = list[len];
                    if ((ch == 0) || (ch == '/'))
                    {
                        outs(bhdr->brdname);
                        outc(' ');
                        break;
                    }
                }
                while ((ch = *list++))
                {
                    if (ch == '/')
                        break;
                }
            }
            while (ch);
        }
    }
    while (++bhdr < tail);

    outc('\n');
}

#ifdef LOG_ADMIN
/* Thor.990405: log permission modify */
static void perm_log(ACCT * u, int oldl)
{
    int i;
    unsigned int level;
    char buf[128];

    for (i = 0, level = 1; i < NUMPERMS; i++, level <<= 1)
    {
        if ((u->userlevel & level) != (oldl & level))
        {
            sprintf(buf, "%s %s %s (%s) by %s\n", u->userid,
                    (u->userlevel & level) ? "■" : "□",
                    perm_tbl[i], Now(), cuser.userid);
            if (!str_cmp(cuser.userid, ELDER))
                pmsg2("板主異動不加入日誌");
            else
                f_cat(FN_SECURITY, buf);
        }
    }
}
#endif

void acct_show(ACCT * u, int adm    /* 1: admin 2: reg-form */
    )
{
    time_t now;
    int diff;
    unsigned int ulevel;
    char *uid, buf[80];

    clrtobot();

    uid = u->userid;
    if (adm != 2)
        prints("\n代    號：%s        使用者編號：%d", uid, u->userno);

    prints("\n暱    稱：%s\n"
           "真實姓名：%s\n"
           "居住住址：%s\n"
           "郵件信箱：%s\n",
           u->username,
           ((adm != 3) && (adm != 4)) ? u->realname : "資料保密",
           ((adm != 3) && (adm != 4)) ? u->address : "資料保密",
           (adm != 3) ? u->email : "資料保密");

    prints("註冊日期：%s", ctime(&u->firstlogin));

    prints("光臨日期：%s", ctime(&u->lastlogin));

    diff = u->staytime / 60;
    prints("上站次數：%d 次 (共 %d 時 %d 分)\n",
           u->numlogins, diff / 60, diff % 60);

    prints("文章數目：%d 篇", u->numposts);

    prints(" (優良積分:%d/劣文:%d/夢幣:%d)\n", u->point1, u->point2, u->money);

    usr_fpath(buf, uid, fn_dir);
    prints("個人信件：%d 封", rec_num(buf, sizeof(HDR)));

    prints(" 剩餘點歌次數：%d 次\n", u->request);

    ulevel = u->userlevel;

    outs("身分認證：\x1b[32m");
    if (ulevel & PERM_VALID)
    {
        outs(u->tvalid ? Ctime(&u->tvalid) : "有效期間已過，請重新認證");
    }
    else
    {
        outs("請參考本站公佈欄進行確認，以提昇權限");
    }
    outs("\x1b[m\n");
    time(&now);
    if ((u->deny - now) > 0 || u->userlevel & PERM_DENYSTOP)
    {
        outs("處罰到期日期：\x1b[1;31m");
        if (u->userlevel & PERM_DENYSTOP)
            outs("無期徒刑 \x1b[m\n");
        else
        {
            outs(Ctime(&u->deny));
            outs("\x1b[m");
            prints("  距今還剩 %d 天 %d 時 \n", (u->deny - now) / 86400,
                   (u->deny - now) / 3600 - ((u->deny - now) / 86400) * 24);
        }
    }

    if (adm)
    {
        if (adm != 3 && adm != 4)
        {
            prints("認證資料：%s\n", u->justify);
            prints("認證地址：%s\n", u->vmail);
            prints("RFC 931 ：%s\n", u->ident);
        }
        prints("上站地點：%s (站外信 %d )\n", u->lasthost, u->numemail);
        bitmsg(MSG_USERPERM, STR_PERM, ulevel);
        bitmsg("旗 標 一：", "-----PMmane---b---Hh--Wm--p", u->ufo);
        bitmsg("旗 標 二：", "0123456789ABCDEFGHIJKLMNOPQ", u->ufo2);
    }
    else
    {
        diff = (time(0) - ap_start) / 60;
        prints("停留期間：%d 小時 %d 分\n", diff / 60, diff % 60);
    }

    if (adm == 2)
        return;

    /* Thor: 想看看這個 user 是那些版的版主 */

    if (ulevel & PERM_BM)
        bm_list(uid);

#ifdef  NEWUSER_LIMIT
    if (u->lastlogin - u->firstlogin < 3 * 86400)
        outs("\n新手上路：三天後開放權限");
#endif
}

void bm_setup(ACCT * u, int adm)
{

    acct_show(u, adm);

    if ((u->userlevel & PERM_SYSOP) && !(cuser.userlevel & PERM_SYSOP))
    {
        outs("此帳號為本站的站長，無法更改權限！");
        return;
    }
    if (!str_cmp(cuser.userid, ELDER))
        pmsg2("板主異動不加入日誌");
    else
    {
        char tmp[80], why[80], buf[80];
        (void)buf;
        pmsg2("板主異動已加入站長日誌");
        if (!vget(b_lines, 0, "請輸入異動理由：", why, 40, DOECHO))
        {
            sprintf(why, "未輸入理由，禁止異動");
            pmsg2("請輸入異動理由");
            return;
        }
        sprintf(tmp, "\n\n%s %-12s 對使用者 %-12s 執行板主異動\n理由: ", Now(),
                cuser.userid, u->userid);
        f_cat(FN_BLACKSU_LOG, tmp);
        f_cat(FN_BLACKSU_LOG, why);
    }
    adm = vans("設定版主權限 Y)確定 N)取消 Q)離開 [Q] ");
    if (adm == 'y' || adm == 'Y')
        u->userlevel = u->userlevel | PERM_BM;
    else if (adm == 'n' || adm == 'N')
        u->userlevel = u->userlevel & (~PERM_BM);
    else
        return;
    acct_save(u);
}


static int seek_log_email(char *mail, int mode)
{
    EMAIL email;
    int pos = 0, fd;
    fd = open(FN_VIOLATELAW_DB, O_RDONLY);
    while (fd)
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

void deny_log_email(char *mail, time_t deny)
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

static void deny_add_email(ACCT * he)
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
            p->tm_hour, p->tm_min, "停權", cuser.userid);
    f_cat(FN_ETC_UNTRUST_ACL, buf);
}

static int select_mode(int adm)
{
    int select, days = 0, mode = 0;
    if (!adm)
    {
        select =
            vans
            ("處罰 1)不當言論 2)Cross Post 3)連鎖信 4)廣告信 5)販賣 6)復權 0)取消 [0] ")
            - '0';
        if (select > 6 || select <= 0)
            return 0;
        if (select != 6)
        {
            days =
                vans
                ("處罰期限 1)一星期 2)兩星期 3)三星期 4)一個月 5)永久 [1] ") -
                '0';
            mode =
                vans
                ("處罰方式 1)禁止 talk 2)鎖信箱 3)禁止 post 4)暱稱 5)全部 6)同 guest [3] ")
                - '0';
        }

        if (vans("你確定嗎 [y/N] ") != 'y')
            return 0;

        if (days > 5 || days < 1)
            days = 1;
        if (mode > 6 || mode < 1)
            mode = 3;

        switch (select)
        {
        case 1:
            adm |= DENY_SEL_TALK;
            break;
        case 2:
            adm |= DENY_SEL_POST;
            break;
        case 3:
            adm |= DENY_SEL_MAIL;
            break;
        case 4:
            adm |= DENY_SEL_AD;
            break;
        case 5:
            adm |= DENY_SEL_SELL;
            break;
        case 6:
            adm |= DENY_SEL_OK;
            break;
        }
        switch (days)
        {
        case 1:
            adm |= DENY_DAYS_1;
            break;
        case 2:
            adm |= DENY_DAYS_2;
            break;
        case 3:
            adm |= DENY_DAYS_3;
            break;
        case 4:
            adm |= DENY_DAYS_4;
            break;
        case 5:
            adm |= DENY_DAYS_5;
            break;
        }
        switch (mode)
        {
        case 1:
            adm |= DENY_MODE_TALK;
            break;
        case 2:
            adm |= DENY_MODE_MAIL;
            break;
        case 3:
            adm |= DENY_MODE_POST;
            break;
        case 4:
            adm |= DENY_MODE_NICK;
            break;
        case 5:
            adm |= DENY_MODE_ALL;
            break;
        case 6:
            adm |= DENY_MODE_GUEST;
            break;
        }

    }
    return adm;
}

int add_deny(ACCT * u, int adm, int cross)
{
    FILE *fp;
    char buf[80];
    ACCT x;
    time_t now;
    int check_time;
    const char *cselect = NULL, *cdays = NULL, *cmode = NULL;

    memcpy(&x, u, sizeof(ACCT));
    time(&now);
    check_time = (x.deny > now) ? 1 : 0;


    fp = fopen(FN_STOP_LOG, "w");
    if (!adm)
    {
        adm = select_mode(adm);
    }
    if (!adm)
    {
        if (fp)
            fclose(fp);
        return 0;
    }

    if (!strncmp(u->justify, "reg:", 4))
        adm = (adm & ~DENY_MODE_ALL) | DENY_MODE_GUEST;

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
        deny_add_email(u);
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
            "\x1b[1;32m※ Origin: \x1b[1;33m%s \x1b[1;37m<%s>\n\x1b[1;31m◆ From: \x1b[1;36m%s\x1b[m\n",
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


void acct_setup(ACCT * u, int adm)
{
    ACCT x;

    int (*sm) (char *mail);

    int i, num, tmp, mode;
    FILE *flog;
    char *str, buf[80], pass[PLAINPASSLEN];
    char id[13];
    tmp = 0;

    acct_show(u, adm);

    memcpy(&x, u, sizeof(ACCT));
    sm = NULL;

    if (((u->userlevel & PERM_SYSOP) && strcmp(cuser.userid, u->userid))
        && !check_admin(cuser.userid))
    {
        outs("此帳號為本站的站長，無法更改！");
        return;
    }

    if (adm)
    {
        if (supervisor)
        {
            if (!str_cmp(cuser.userid, ELDER))
                pmsg2("查詢動作不加入日誌");
            else
            {
                char tmp[80], why[80];
                pmsg2("查詢動作已加入站長日誌");
                if (!vget(b_lines, 0, "請輸入理由：", why, 40, DOECHO))
                {
                    sprintf(why, "未輸入理由，禁止查詢");
                    pmsg2("請輸入查詢理由");
                    return;
                }
                sprintf(tmp,
                        "\n\n%s %-12s 對使用者 %-12s 執行查詢動作\n理由: ",
                        Now(), cuser.userid, u->userid);
                f_cat(FN_BLACKSU_LOG, tmp);
                f_cat(FN_BLACKSU_LOG, why);
            }
            adm =
                vans
                ("設定 1)資料 2)權限 3)連坐處罰 4)單人處罰 5)基本權限 6)旗標 7)SU Q)取消 [Q] ");
        }
        else
        {
            if (!str_cmp(cuser.userid, ELDER))
                pmsg2("查詢動作不加入日誌");
            else
            {
                char tmp[80], why[80];
                pmsg2("查詢動作已加入站長日誌");
                if (!vget(b_lines, 0, "請輸入查詢理由：", why, 40, DOECHO))
                {
                    sprintf(why, "未輸入理由，禁止查詢");
                    pmsg2("請輸入查詢理由");
                    return;
                }
                sprintf(tmp,
                        "\n\n%s %-12s 對使用者 %-12s 執行查詢動作\n理由: ",
                        Now(), cuser.userid, u->userid);
                f_cat(FN_BLACKSU_LOG, tmp);
                f_cat(FN_BLACKSU_LOG, why);
            }
            adm =
                vans
                ("設定 1)資料 2)權限 3)連坐處罰 4)單人處罰 5)基本權限 6)旗標 Q)取消 [Q] ");
        }
        if (adm == '6')
        {
            su_setup(u);
            acct_save(u);
        }
        /* BLACK SU */
        if (adm == '7' && supervisor)
            acct_su(u);
        /* BLACK SU */
        if (adm == '4')
        {
            tmp = add_deny(u, tmp, 0);
            if ((tmp & DENY_MODE_ALL) && (u->deny > time(0)))
                deny_log_email(u->vmail,
                               (u->userlevel & PERM_DENYSTOP) ? -1 : u->deny);
        }
        if (adm == '3')
        {
            switch (vans("使用程序 i)內部 o)外部 q)取消：[q]"))
            {
            case 'i':
                if (!sm)
                {
                    sm = DL_get(BINARY_PREFIX"same_mail.so:same_mail");
                }
                strcpy(id, u->userid);
                if (sm)
                {
                    num = (*sm) (u->vmail);
                }
                else
                    break;

                flog = fopen(FN_SAMEEMAIL_LOG, "r");
                tmp = 0;
                if (flog == NULL)
                    return;

                for (i = 1; i <= num; i++)
                {
                    fscanf(flog, "%13s", buf);
                    acct_load(u, buf);
                    if (u != NULL)
                    {
                        if (strcmp(u->userid, id))
                            tmp = add_deny(u, tmp, 1);
                        else
                            tmp = add_deny(u, tmp, 0);
                    }
                    if (!tmp)
                        break;
                }

                if (tmp & DENY_MODE_ALL)
                    deny_log_email(u->vmail,
                                   (u->userlevel & PERM_DENYSTOP) ? -1 : u->
                                   deny);
                fclose(flog);
                break;
            case 'o':
                {
                    char command[256];
                    mode = select_mode(0);
                    sprintf(command, BINARY_PREFIX"stopperm %s %s %d %s %d &",
                            u->userid, u->vmail, mode, cuser.userid,
                            (int)time(0));
                    system(command);
                }
                break;
            }
        }
        if (adm == '5')
        {
            u->userlevel = PERM_BASIC;
            acct_save(u);
            return;
        }
        if (adm == '2')
            goto set_perm;

        if (adm != '1')
            return;
    }
    else
    {
        if (vans("修改資料(Y/N)?[N] ") != 'y')
            return;
    }

    move(i = 3, 0);
    clrtobot();

    if (adm)
    {
        str = x.userid;
        for (;;)
        {
            vget(i, 0, "使用者代號(不改請按 Enter)：", str, IDLEN + 1, GCARRY);
            if (!str_cmp(str, u->userid) || !acct_userno(str))
                break;
            vmsg("錯誤！已有相同 ID 的使用者");
        }
    }
    else
    {
        /* pcbug.990813: 新PASSLEN過長, 改成直接寫死 */
        /*    vget(i, 0, "請確認密碼：", buf, PASSLEN, NOECHO); */
        vget(i, 0, "請確認密碼：", buf, PLAINPASSLEN, NOECHO);
        if (chkpasswd(u->passwd, buf))
        {
            vmsg("密碼錯誤");
            return;
        }
    }

    i++;
    for (;;)
    {
        if (!vget
            (++i, 0, "設定新密碼(不改請按 Enter)：", buf, /*PASSLEN*/ PLAINPASSLEN,
             NOECHO))
            break;

        strcpy(pass, buf);
        vget(i + 1, 0, "檢查新密碼：", buf, /*PASSLEN*/ PLAINPASSLEN, NOECHO);
        if (!strcmp(buf, pass))
        {
            buf[PLAINPASSLEN-1] = '\0';
            str_ncpy(x.passwd, genpasswd(buf), PASSLEN);
            i++;
            logitfile(FN_PASS_LOG, cuser.userid, cuser.lasthost);
            break;
        }
    }

    i++;
    str = x.username;
    do
    {
        vget(i, 0, "暱    稱：", str, sizeof(x.username), GCARRY);
    }
    while (str_len(str) < 1);

    i++;
    str = x.realname;
    do
    {
        vget(i, 0, "真實姓名：", str, sizeof(x.realname), GCARRY);
    }
    while (str_len(str) < 4);

    i++;
    str = x.address;
    do
    {
        vget(i, 0, "居住地址：", str, sizeof(x.address), GCARRY);
    }
    while (str_len(str) < 8);

    if (adm)
    {
        i++;
        str = x.email;

        vget(i, 0, "E-mail 信箱：", str, sizeof(x.email), GCARRY);

        vget(++i, 0, "認證資料：", x.justify, 44, GCARRY);
        if (strlen(x.justify) > 4)
        {
            vget(++i, 0, "增加有效期限(y/N)：", buf, 2, DOECHO);
            if (buf[0] == 'y' || buf[0] == 'Y')
            {
                time(&x.tvalid);
                x.userlevel |=
                    (PERM_BASIC | PERM_CHAT | PERM_PAGE | PERM_POST |
                     PERM_VALID);
                /*by visor */
            }
        }
        sprintf(buf, "%d", u->numlogins);
        vget(++i, 0, "上線次數：", buf, 10, GCARRY);
        if ((num = atoi(buf)) >= 0)
            x.numlogins = num;

#ifdef CHANGE_USERNO
        sprintf(buf, "%d", u->userno);
        vget(++i, 0, "使用者編號：", buf, 10, GCARRY);
        if ((num = atoi(buf)) >= 0)
            x.userno = num;
#endif

        sprintf(buf, "%d", u->numposts);
        vget(++i, 0, "文章篇數：", buf, 10, GCARRY);
        if ((num = atoi(buf)) >= 0)
            x.numposts = num;

        sprintf(buf, "%d", u->money);
        vget(++i, 0, "夢幣：", buf, 10, GCARRY);
        if ((num = atoi(buf)) >= 0)
            x.money = num;

        sprintf(buf, "%d", u->point1);
        vget(++i, 0, "優良積分：", buf, 10, GCARRY);
        if ((num = atoi(buf)) >= 0)
            x.point1 = num;

        sprintf(buf, "%d", u->point2);
        vget(++i, 0, "劣文：", buf, 10, GCARRY);
        if ((num = atoi(buf)) >= 0)
            x.point2 = num;

#ifdef  CHANGE_SECOND
        sprintf(buf, "%d", u->staytime);
        vget(++i, 0, "上站總秒數：", buf, 20, GCARRY);
        if ((num = atoi(buf)) >= 0)
            x.staytime = num;
#endif

        sprintf(buf, "%d", u->request);
        vget(++i, 0, "點歌次數：", buf, 10, GCARRY);
        if ((num = atoi(buf)) >= 0)
            x.request = num;

        /* lkchu.981201: 特殊用途 :p */
        vget(++i, 0, "認證地址：", x.vmail, 44, GCARRY);
        vget(++i, 0, "上站地點：", x.lasthost, 30, GCARRY);
        vget(++i, 0, "RFC 931 ：", x.ident, 44, GCARRY);

        if (vans("設定權限(Y/N)?[N] ") == 'y')
        {
          set_perm:

            i = setperm(num = x.userlevel);

            if (i == num)
            {
                vmsg("取消修改");
                if (adm == '2')
                    return;
            }
            else
            {
                x.userlevel = i;
            }
        }
    }

    if (vans(msg_sure_ny) != 'y')
        return;

    if (adm)
    {
        if (str_cmp(u->userid, x.userid))
        {                        /* Thor: 980806: 特別注意如果 usr每個字母不在同一partition的話會有問題 */
            char dst[80];

            usr_fpath(buf, u->userid, NULL);
            usr_fpath(dst, x.userid, NULL);
            rename(buf, dst);
            /* Thor.990416: 特別注意! .USR並未一併更新, 可能有部分問題 */
        }
#ifdef LOG_ADMIN
        /* lkchu.981201: security log */
        perm_log(&x, u->userlevel);
#endif
    }
    else
    {
        /* Thor: 這樣即使在線上, 也可以改 userlevel */

        if (acct_load(u, x.userid) >= 0)
            x.userlevel = u->userlevel;
    }

    memcpy(u, &x, sizeof(x));
    acct_save(u);
}


int u_info(void)
{
    char *str, username[24];    /* Thor.980727:lkchu patch: username[20] -> 24 */

    move(2, 0);
    strcpy(username, str = cuser.username);
    acct_setup(&cuser, 0);
    if (strcmp(username, str) && HAS_PERM(PERM_VALID)
        && (!HAS_PERM(PERM_DENYNICK) || HAS_PERM(PERM_SYSOP)))
        memcpy(cutmp->username, str, sizeof(cuser.username));
    else if (HAS_PERM(PERM_DENYNICK))
        vmsg("禁止修改暱稱");
    return 0;
}


int m_user(void)
{
    int ans;
    ACCT acct;

    while ((ans = acct_get(msg_uid, &acct)))
    {
        if (ans > 0)
            acct_setup(&acct, 1);
    }
    return 0;
}

int m_bmset(void)
{
    int ans;
    ACCT acct;
    while ((ans = acct_get(msg_uid, &acct)))
    {
        if (ans > 0)
            bm_setup(&acct, 4);
    }
    return 0;
}


/* ----------------------------------------------------- */
/* 設定 E-mail address                                   */
/* ----------------------------------------------------- */


int ban_addr(char *addr)
{
    int i;
    char *host, *str;
    char foo[64];                /* SoC: 放置待檢查的 email address */
    const char* str_invalid;

    static const char *invalid[] = { "@bbs", "bbs@", "root@", "gopher@",
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
        TRACE("NOACL", host);
    return i > 0;
}

#ifdef HAVE_WRITE
static int allow_addr(char *addr)
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
#endif

void                            /* gaod:換新:p */
check_nckuemail(char *email)
{
    char *ptr;
    ptr = strstr(email, DEFAULTSERVER);

    if (ptr)
    {
        strcpy(ptr, NCKUMAIL);
    }
}

/* 找尋是否有註冊三個以上之 Email */
int find_same_email(            /* mode : 1.find 2.add 3.del */
                       char *mail, int mode)
{
    int pos = 0, fd;
    const char *fpath;
    SAME_EMAIL email;


    fpath = FN_ETC_EMAILADDR_ACL;

    if (mode >= 1 && mode <= 3)
    {
        fd = open(fpath, O_RDONLY);
        pos = 0;
        while (fd)
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
        if (fd)
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

int u_addr(void)
{
    const char *msg;
    char addr[60], buf[30], agent[128], temp[60];
    HDR fhdr;
    FILE *fout;
    int vtime;
    unsigned int tmp_perm;
    int popreturn;

    msg = NULL;
    more(FN_ETC_EMAIL, (char *)-1);
    strcpy(temp, cuser.email);
    tmp_perm = cuser.userlevel;
    /* lkchu.981201 */
    if (vget
        (b_lines - 1, 0, "E-Mail 地址：", addr, sizeof(cuser.email), DOECHO))
    {
#ifndef HAVE_SIMPLE_RFORM
        vtime = REG_REQUEST;
#endif
        str_lower(addr, addr);
        if (not_addr(addr))
        {
            msg = "不合格的 E-mail address";
            pmsg(msg);
            return 0;
        }
        else if (ban_addr(addr))
        {
            msg = "禁止註冊的 E-mail address";
            pmsg(msg);
            return 0;
        }
#ifdef HAVE_WRITE
        else if (!allow_addr(addr))
        {
#ifndef HAVE_SIMPLE_RFORM
            attr_put(cuser.userid, ATTR_REG_KEY, &vtime);
#endif
            msg = "尚未申請通過的 E-mail 主機";
            pmsg(msg);
            return 0;
        }
#endif  /* #ifdef HAVE_WRITE */
        else if (strcmp(temp, addr) && (seek_log_email(addr, 1) != -1))
        {
            msg = "暫時禁止註冊的 E-mail address";
            pmsg(msg);
            return 0;
        }
        else if (strcmp(temp, addr) ? (find_same_email(addr, 1) >= MAX_REGIST)
                 : (find_same_email(addr, 1) > MAX_REGIST))
        {
            msg = "註冊人數超過 3 個的 E-mail address";
            pmsg(msg);
            return 0;
        }

        /* pcbug.990522: pop3認證. */
        vget(b_lines - 2, 0, "是否使用 POP3 認證?[Y]", buf, 2, LCECHO);

        if (buf[0] != 'n' && buf[0] != 'N')
        {
            char title[80], *ptr;
            int sock = 110;

            strcpy(title, addr);
            check_nckuemail(title);

            *(ptr = strchr(title, '@')) = 0;

            clear();
            move(2, 0);
            prints("主機: %s\n帳號: %s\n", ptr + 1, title);
            prints("\x1b[1;5;36m連線遠端主機中...請稍候\x1b[m\n");
            refresh();
            if (!Get_Socket(ptr + 1, &sock))
            {
                close(sock);
                move(4, 0);
                clrtoeol();
                while (1)
                {
                    move(15, 0);
                    clrtobot();
                    vget(15, 0, "請輸入以上所列出之工作站帳號的密碼: ", buf,
                         20, NOECHO);
                    move(16, 0);
                    prints("\x1b[5;37m身份確認中...請稍候\x1b[m\n\n");
                    refresh();

                    if (!(popreturn = POP3_Check(ptr + 1, title, buf)))
                    {
                        logitfile(FN_VERIFY_LOG, "-POP3 Verify OK -", addr);
                        cuser.userlevel |=
                            (PERM_VALID | PERM_POST | PERM_PAGE | PERM_CHAT);
                        if (cuser.userlevel & PERM_DENYPOST)
                            cuser.userlevel &= ~PERM_POST;

                        if (cuser.userlevel & PERM_DENYTALK)
                            cuser.userlevel &= ~PERM_PAGE;

                        if (cuser.userlevel & PERM_DENYCHAT)
                            cuser.userlevel &= ~PERM_CHAT;
                        str_ncpy(cuser.vmail, addr, sizeof(cuser.vmail));
                        sprintf(agent, "pop3認證:%s", addr);
                        str_ncpy(cuser.justify, agent, sizeof(cuser.justify));
                        time(&cuser.tvalid);
                        strcpy(cuser.email, addr);
                        acct_save(&cuser);
                        find_same_email(addr, 2);
                        if (tmp_perm & PERM_VALID)
                            find_same_email(temp, 3);
                        usr_fpath(buf, cuser.userid, FN_JUSTIFY);
                        if ((fout = fopen(buf, "a")))
                        {
                            fprintf(fout, "%s\n", agent);
                            fclose(fout);
                        }
                        usr_fpath(buf, cuser.userid, fn_dir);
                        hdr_stamp(buf, HDR_LINK, &fhdr, FN_ETC_JUSTIFIED_POP3);
                        strcpy(fhdr.title,
                               "[註冊成功\] 您已經通過身分認證了！");
                        strcpy(fhdr.owner, "SYSOP");
                        rec_add(buf, &fhdr, sizeof(fhdr));
                        board_main();
                        gem_main();
                        talk_main();
                        cutmp->ufo |= UFO_BIFF;
                        msg = "身份確認成功\，立刻提昇權限";
                        break;
                    }
                    else
                    {
                        logitfile(FN_VERIFY_LOG, "-POP3 Verify ERR-", addr);
                        if (popreturn != 8)
                            vget(17, 0,
                                 "身份確認失敗，是否重新確認 (Y/N) ? [Y]", buf,
                                 3, LCECHO);

                        if (buf[0] == 'n' || buf[0] == 'N' || popreturn == 8)
                        {
                            cuser.userlevel &=
                                ~(PERM_VALID | PERM_POST | PERM_PAGE |
                                  PERM_CHAT);
                            cuser.vtime = -1;
                            strcpy(cuser.email, addr);
                            acct_save(&cuser);
                            board_main();
                            gem_main();
                            talk_main();

                            msg = "身分確認失敗，權限移除....";
                            break;
                        }
                    }
                }
            }
            else
            {
                prints("POP3: 不支援，身份確認\x1b[1;36m使用認證信函\x1b[m"
                       "\n\n\x1b[1;36;5m系統送信中...\x1b[m");
                refresh();
                sleep(1);
                buf[0] = 'n';
            }
        }



        if (buf[0] == 'n' || buf[0] == 'N')
        {

            if (tmp_perm & PERM_VALID)
                find_same_email(temp, 3);

            vtime = bsmtp(NULL, NULL, addr, MQ_JUSTIFY);

            if (vtime < 0)
            {
                msg = "身份認證信函無法寄出，請正確填寫 E-mail address";
                cuser.userlevel &=
                    ~(PERM_VALID | PERM_POST | PERM_PAGE | PERM_CHAT);
                cuser.vtime = vtime;
                strcpy(cuser.email, addr);
                acct_save(&cuser);
                board_main();
                gem_main();
                talk_main();
            }
            else
            {
                cuser.userlevel &= ~(PERM_VALID);
                cuser.vtime = vtime;
                strcpy(cuser.email, addr);
                acct_save(&cuser);

                more(FN_ETC_JUSTIFY, (char *)-1);
                /* lkchu.981201 */
                prints("\n%s(%s)您好，由於您更新 E-mail address 的設定，\n"
                       "請您儘快到 \x1b[44m%s\x1b[m 所在的工作站回覆『身份認證信函』。",
                       cuser.userid, cuser.username, addr);
                msg = NULL;
            }
        }
    }
    vmsg(msg);
    return 0;
}

static const char *UFO_FLAGS[] = {
    "【保留】",
    "【保留】",
    "【保留】",
    "【保留】",
    "【保留】",

    /* PAGER */ "關閉呼叫器",
    /* QUITE */ "關閉訊息",
    /* MAXMSG */ "訊息上限拒收訊息",
    /* FORWARD */ "自動轉寄",
#ifdef HAVE_CLASSTABLEALERT
    /* CLASSTABLE */ "課表時刻通知",
#else
    /* CLASSTABLE */ "課表時刻通知(系統功\能尚未開啟)",
#endif
    /* MPAGER */ "電子郵件傳呼",
    "【保留】",
    "【保留】",
    "【保留】",
    /* REJECT */ "拒收廣播",
    "【保留】",
    "【保留】",
    "【保留】",
    /* HIDDEN */ "隱藏來源",

    /* CLOAK */ "隱身術",
    "【保留】"
};

static const char *UFO2_FLAGS[] = {
    /* COLOR */ "彩色模式",
    /* MOVIE */ "動畫顯示",
    /* BRDNEW */ "新推文",
    /* BNOTE */ "顯示進板畫面",
    /* VEDIT */ "簡化編輯器",
    /* PAL */ "只顯示好友",
    /* MOTD */ "簡化進站畫面",
#ifdef HAVE_MIME_TRANSFER
    /* MIME */ "MIME 解碼",
#else
    /* MIME */ "MIME 解碼(系統功\能尚未開啟)",
#endif
    /* SIGN */ "選擇簽名檔 開啟:後 關閉:前",
    /* SHOWUSER */ "顯示自己 ID 和暱稱",
#ifdef HAVE_RECOMMEND
    /* PRH */ "關閉推薦文章分數",
#else
    /* PRH */ "關閉推薦文章分數(系統功\能尚未開啟)",
#endif
    /* SHIP */ "好友描述",
    /* NWLOG */ "不儲存熱訊紀錄",
    /* NTLOG */ "不儲存聊天紀錄",
    /* CIRCLE */ "循環閱\讀",
    /* ORIGUI */ "關閉超炫介面",
    /* DEF_ANONY */ "預設不匿名",
    /* DEF_LEAVE */ "預設不離站",
    /* REPLY */ "記錄水球資訊",
    /* DEF_LOCALMAIL */ "只收站內信",
    /* RESERVE */ "【保留】",
    /* RESERVE */ "【保留】",
    /* RESERVE */ "【保留】",
    /* RESERVE */ "【保留】",
    /* ACL */ "ACL",
    /* RESERVE */ "【保留】",
    /* RESERVE */ "【保留】",
    /* RESERVE */ "【保留】",
    /* REALNAME */ "真實姓名",
    /* RESERVE */ "【保留】",
    /* RESERVE */ "【保留】",
    /* REALNAME */ "【保留】"
};


void su_setup(ACCT * u)
{
    int ufo, nflag, len;
    char fpath[80];
    UTMP *up;
    const char **flags = UFO_FLAGS;

    up = utmp_find(u->userno);
    len = 21;
    ufo = u->ufo;
    nflag = bitset(ufo, len, len, "操作模式設定：", flags);
    if (nflag != ufo)
    {
        if (up)
        {
            nflag = (nflag & ~UFO_UTMP_MASK) | (up->ufo & UFO_UTMP_MASK);
            up->ufo = u->ufo = nflag;
        }
        else
            u->ufo = nflag;
        //      showansi = nflag & UFO_COLOR;
        outs(str_ransi);
    }
    usr_fpath(fpath, u->userid, FN_FORWARD);
    if (u->ufo & UFO_FORWARD)
    {
        FILE *fd;
        fd = fopen(fpath, "w");
        fclose(fd);
    }
    else
        unlink(fpath);

}

int u_setup(void)
{
    int ufo, nflag, len;
    char fpath[80];

    const char **flags = UFO_FLAGS;

    nflag = cuser.userlevel;
    if (!nflag)
        len = 5;
    else if (nflag & (PERM_SYSOP | PERM_BOARD | PERM_ACCOUNTS | PERM_CHATROOM))
        len = 21;
    /* Thor.980910: 需注意有PERM_ADMIN除了可用acl, 還順便也可以用隱身術了:P */
    else if (nflag & PERM_CLOAK)
        len = 20;
    else
        len = 18;                /* lkchu.990428: 加了 電子郵件傳呼 */

    ufo = cuser.ufo;
    nflag = bitset(ufo, len, len, "操作模式設定：", flags);
    if (nflag != ufo)
    {
        /* Thor.980805: 解決 ufo BIFF的同步問題 */
        nflag = (nflag & ~UFO_UTMP_MASK) | (cutmp->ufo & UFO_UTMP_MASK);

        cutmp->ufo = cuser.ufo = nflag;
        /* Thor.980805: 要特別注意 cuser.ufo和cutmp->ufo的UFO_BIFF的同步問題, 再改 */

        //      showansi = nflag & UFO_COLOR;
        outs(str_ransi);
    }
    usr_fpath(fpath, cuser.userid, FN_FORWARD);
    if (cuser.ufo & UFO_FORWARD)
    {
        FILE *fd;
        fd = fopen(fpath, "w");
        fclose(fd);
    }
    else
        unlink(fpath);

    return 0;
}

int ue_setup(void)
{
    int ufo2, nflag, len;

    const char **flags = UFO2_FLAGS;

    nflag = cuser.userlevel;
    if (!nflag)
        len = 5;
    else if (nflag & (PERM_SYSOP | PERM_ACCOUNTS))
        len = 32;
    else if (nflag & PERM_ADMIN)
        len = 28;
    else
        len = 24;

    ufo2 = cuser.ufo2;
    nflag = bitset(ufo2, len, len, "喜愛功\能設定：", flags);
    if (nflag != ufo2)
    {
        cuser.ufo2 = nflag;
        showansi = nflag & UFO2_COLOR;
        outs(str_ransi);
    }
    return 0;
}

int u_lock(void)
{
    char buf[PLAINPASSLEN];
    char swapmateid[IDLEN + 1] = "";
    char IdleState[][IDLEN] = {
        "自強觀星",
        "勝後放閃",
        "光復打球",
        "力行探險",
        "成功\散步",
        "敬業耍宅"
    };

    strcpy(swapmateid, cutmp->mateid);
    vget(b_lines - 1, 0,
         "理由:[0]發呆 (1)接電話 (2)覓食 (3)打瞌睡 (4)裝死 (5)哭哭 (6)其他 (Q)沒事:",
         buf, 2, DOECHO);

    if (buf[0] <= '5' && buf[0] >= '0')
    {
        strcpy(cutmp->mateid, IdleState[buf[0] - '0']);
    }
    else if (buf[0] == '6')
    {
        vget(b_lines - 1, 0, "發呆的理由:", cutmp->mateid, IDLEN, DOECHO);
    }
    else if (buf[0] == 'q' || buf[0] == 'Q')
    {
        strcpy(cutmp->mateid, swapmateid);
        return XEASY;
    }
    else
    {
        strcpy(cutmp->mateid, IdleState[0]);
    }
    cutmp->ufo |= UFO_REJECT;
    utmp_mode(M_IDLE);

    buf[0] = 'n';
    if (str_cmp(cutmp->userid, "guest"))
        vget(b_lines - 1, 0, "是否要進入螢幕鎖定狀態(Y/N)?[N]", buf, 2,
             DOECHO);

    clear();
    prints("\x1b[1;44;33m%*s" BOARDNAME "    閒置/鎖定狀態%*s\x1b[m",
           (d_cols >> 1) + 36 - sizeof(BOARDNAME), "", ((d_cols+1) >> 1) + 26, "");
    move(4, 6);
    prints("閒置中：%s", cutmp->mateid);
    if (buf[0] == 'y' || buf[0] == 'Y')
    {
        int check;
        blog("LOCK", "screen");
        bbstate |= STAT_LOCK;    /* lkchu.990513: 鎖定時不可回訊息 */
        check = 0;
        do
        {
            vget(7, 0, "▲ 請輸入密碼，以解除螢幕鎖定：", buf, PLAINPASSLEN, NOECHO);
            check = chkpasswd(cuser.passwd, buf);
            if (check)
            {
                char fpath[80];
                char temp[80];
                usr_fpath(fpath, cuser.userid, FN_LOGINS_BAD);
                sprintf(temp, "[%s] BBS %s\n", Ctime(&ap_start), fromhost);
                f_cat(fpath, temp);
            }
        }
        while (check);
    }
    else
    {
        igetch();
    }

    strcpy(cutmp->mateid, swapmateid);
    bbstate ^= STAT_LOCK;
    cutmp->ufo &= ~UFO_REJECT;
    return 0;
}

int u_xfile(void)
{
    int i;

    static const char *desc[] = {
        "上站地點設定檔",
        "名片檔",
        "簽名檔 (每個 6 行，共 3 個)",
        "暫存檔.1",
        "暫存檔.2",
        "暫存檔.3",
        "暫存檔.4",
        "暫存檔.5",
        NULL
    };

    static const char *path[] = {
        "acl",
        FN_PLANS,
        FN_SIGN,
        "buf.1",
        "buf.2",
        "buf.3",
        "buf.4",
        "buf.5"
    };

    i = (HAS_PERM(PERM_ADMIN)) ? 0 : 1;
    x_file(M_UFILES, &desc[i], &path[i]);
    return 0;
}


/* ----------------------------------------------------- */
/* 看板管理                                              */
/* ----------------------------------------------------- */


static int valid_brdname(char *brd)
{
    int ch;

    if (!is_alnum(*brd))
        return 0;

    while ((ch = *++brd))
    {
        if (!is_alnum(ch) && ch != '.' && ch != '-' && ch != '_')
            return 0;
    }
    return 1;
}

static int m_setbrd(BRD * brd)
{
    int i;
    char *data, buf[16], old_brdname[IDLEN + 1];
    FILE *fp;
    char fpath[80];

    if (!str_cmp(cuser.userid, ELDER))
        pmsg2("修改動作不加入日誌");
    else
    {
        char tmp[80], why[80];
        pmsg2("修改動作已加入站長日誌");
        if (!vget(b_lines, 0, "請輸入修改理由：", why, 40, DOECHO))
        {
            sprintf(why, "未輸入理由，禁止修改");
            pmsg2("請輸入修改理由");
            return 0;
        }
        sprintf(tmp, "\n\n%s %-12s 對看板 %-12s 執行修改動作\n理由: ", Now(),
                cuser.userid, brd->brdname);
        f_cat(FN_BLACKSU_LOG, tmp);
        f_cat(FN_BLACKSU_LOG, why);
    }

    data = brd->brdname;
    i = *data ? 11 : 1;
    strcpy(old_brdname, data);

    for (;;)
    {
        if (!vget(i, 0, MSG_BID, data, IDLEN + 1, GCARRY))
        {
            if (i == 1)
                return -1;

            strcpy(data, old_brdname);    /* Thor:若是清空則設為原名稱 */
            continue;
        }

        if (!strcmp(old_brdname, data) && valid_brdname(data))
        {                        /* Thor: 與原名同則跳過 */
            break;
        }

        if (brd_bno(data) >= 0)
        {
            outs("\n錯誤! 板名雷同");
        }
        else if (valid_brdname(data))
        {
            break;
        }
    }

    data = buf;
    vget(++i, 0, "看板主題：", brd->title, BTLEN + 1, GCARRY);
    vget(++i, 0, "類別：", brd->class, 5, GCARRY);
    sprintf(data, "%02d", brd->color);
    vget(++i, 0, "顏色格式 X.亮度(0~1) Y.顏色(0~7) [XY]：", data, 3, GCARRY);
    if (data[0] < '0' || data[0] > '1')
        data[0] = '0';
    if (data[1] < '0' || data[1] > '7')
        data[1] = '7';
    brd->color = (char)atoi(data);
    vget(++i, 0, "板主名單：", brd->BM, BMLEN + 1, GCARRY);

    sprintf(data, "%d", brd->expiremax);
    vget(++i, 0, "最大文章數量 ( [0] 為預設)：", data, 6, GCARRY);
    brd->expiremax = atoi(data);

    sprintf(data, "%d", brd->expiremin);
    vget(++i, 0, "最小文章數量 ( [0] 為預設)：", data, 6, GCARRY);
    brd->expiremin = atoi(data);

    sprintf(data, "%d", brd->expireday);
    vget(++i, 0, "文章保留天數 ( [0] 為預設)：", data, 6, GCARRY);
    brd->expireday = atoi(data);

#ifdef HAVE_MODERATED_BOARD
    /* cache.091124: 改用較便利的看板權限設定 */
    switch (vget
            (++i, 0, "看板權限 A)一般 B)自定 C)秘密 D)好友？[Q] ", buf, 3,
             LCECHO))
    {
    case 'c':
        brd->readlevel = (PERM_SYSOP | PERM_BOARD);    /* 秘密看板 */
        brd->postlevel = PERM_POST;
        brd->battr |= (BRD_NOSTAT | BRD_NOVOTE);
        break;

    case 'd':
        brd->readlevel = PERM_SYSOP;    /* 好友看板 */
        brd->postlevel = PERM_POST;
        brd->battr |= (BRD_NOSTAT | BRD_NOVOTE);
        break;
#else
    switch (vget(++i, 0, "看板權限 A)一般 B)自定？[Q] ", buf, 3, LCECHO))
    {
#endif

    case 'a':
        brd->readlevel = 0;
        brd->postlevel = PERM_POST;    /* 一般看板發表權限為 PERM_POST */
        brd->battr &= ~(BRD_NOSTAT | BRD_NOVOTE);    /* 拿掉好友＆秘密板屬性 */
        break;

    case 'b':
        if (vget(++i, 0, "閱\讀權限(Y/N)？[N] ", buf, 3, LCECHO) == 'y')
        {
            brd->readlevel =
                bitset(brd->readlevel, NUMPERMS, NUMPERMS, MSG_READPERM,
                       perm_tbl);
            move(2, 0);
            clrtobot();
            i = 1;
        }

        if (vget(++i, 0, "發表權限(Y/N)？[N] ", buf, 3, LCECHO) == 'y')
        {
            brd->postlevel =
                bitset(brd->postlevel, NUMPERMS, NUMPERMS, MSG_POSTPERM,
                       perm_tbl);
            move(2, 0);
            clrtobot();
            i = 1;
        }
        break;

    default:                    /* 預設不變動 */
        break;
    }

    if (vget(++i, 0, "設定屬性(Y/N)？[N] ", data, 3, LCECHO) == 'y')
    {
        brd->battr =
            bitset(brd->battr, NUMATTRS, NUMATTRS, MSG_BRDATTR, battrs);
    }
    brd_fpath(fpath, brd->brdname, CHECK_BAN);

    /* cache.090928 看板互斥屬性 - 有推文和自定推噓文(預設) */
    if ((brd->battr & BRD_PUSHSNEER) && (brd->battr & BRD_PUSHDEFINE))
        brd->battr &= ~BRD_PUSHSNEER;

    /* cache.090928 看板互斥屬性 - 當看板唯讀時也禁止推文 */
    if (brd->battr & BRD_NOREPLY)
        brd->battr |= BRD_PRH;

    /* cache.090928 看板互斥屬性 - 不可推文時清除所有推文模式和限制 */
    if (brd->battr & BRD_PRH)
    {
        brd->battr &= ~BRD_PUSHDISCON;
        brd->battr &= ~BRD_PUSHTIME;
        brd->battr &= ~BRD_PUSHSNEER;
        brd->battr &= ~BRD_PUSHDEFINE;
    }

    if (brd->battr & BRD_NOBAN)
    {
        fp = fopen(fpath, "w+");
        if (fp)
            fclose(fp);
    }
    else
        unlink(fpath);
    return 0;
}


int m_newbrd(void)
{
    BRD newboard;
    int bno;
    char fpath[80];
    HDR hdr;

    vs_bar("建立新板");
    memset(&newboard, 0, sizeof(newboard));
    strcpy(newboard.title, "□■☆★※◆◎●○");
    if (m_setbrd(&newboard))
        return -1;

    if (vans(msg_sure_ny) != 'y')
        return 0;

    time(&newboard.bstamp);
    if ((bno = brd_bno("")) >= 0)
    {
        rec_put(FN_BRD, &newboard, sizeof(newboard), bno);
    }
    /* Thor.981102: 防止超過shm看板個數 */
    else if (bshm->number >= MAXBOARD)
    {
        vmsg("超過系統所能容納看版個數，請調整系統參數");
        return -1;
    }
    else if (rec_add(FN_BRD, &newboard, sizeof(newboard)) < 0)
    {
        vmsg("無法建立新板");
        return -1;
    }

    sprintf(fpath, "gem/brd/%s", newboard.brdname);
    mak_dirs(fpath);
    mak_dirs(fpath + 4);

    bshm->uptime = 0;            /* force reload of bcache */
    bshm_init();

    /* 順便加進 NewBoard */

    if (vans("是否加入 [NewBoard] 群組(Y/N)?[Y] ") != 'n')
    {
        brd2gem(&newboard, &hdr);
        rec_add("gem/@/@NewBoard", &hdr, sizeof(HDR));
    }

    vmsg("新板成立");
    return 0;
}

void brd_edit(int bno)
{
    BRD *bhdr, newbh;
    char buf[80];

    vs_bar("看板設定");
    bhdr = bshm->bcache + bno;
    memcpy(&newbh, bhdr, sizeof(BRD));
    prints("看板名稱：%s\n看板說明：%s\n板主名單：%s\n",
           newbh.brdname, newbh.title, newbh.BM);
    prints("看板類別：[%4s] 類別顏色：%d\n", newbh.class, newbh.color);
    prints("文章數目：[最大] %d [最小] %d [天數] %d\n", newbh.expiremax,
           newbh.expiremin, newbh.expireday);

    bitmsg(MSG_READPERM, STR_PERM, newbh.readlevel);
    bitmsg(MSG_POSTPERM, STR_PERM, newbh.postlevel);
    bitmsg(MSG_BRDATTR, "zTcsvAPEblpRLWiePSt", newbh.battr);

    switch (vget
            (9, 0, "(D)刪除 (E)設定 (V)預設權限 (Q)取消？[Q] ", buf, 3,
             LCECHO))
    {
    case 'v':
        bhdr->postlevel |= PERM_VALID;
        rec_put(FN_BRD, bhdr, sizeof(BRD), bno);
        vmsg("設定完畢");
        break;
    case 'd':

        if (vget(10, 0, msg_sure_ny, buf, 3, LCECHO) != 'y')
        {
            vmsg(MSG_DEL_CANCEL);
        }
        else
        {
            char *bname = bhdr->brdname;
            /* 以免造成*bname為NULL時，會砍到 gem/brd and brd。 statue.000728 */
            if (*bname)
            {
                char cmd[256];
                sprintf(buf, "gem/brd/%s", bname);
                //f_rm(buf);
                //f_rm(buf + 4);
                /* 100721.cache: f_rm is buggy... */
                sprintf(cmd, "rm -rf %s", buf);
                system(cmd);
                sprintf(cmd, "rm -rf %s", buf + 4);
                system(cmd);
                memset(&newbh, 0, sizeof(newbh));
                sprintf(newbh.title, "[%s] deleted by %s", bname,
                        cuser.userid);
                memcpy(bhdr, &newbh, sizeof(BRD));
                rec_put(FN_BRD, &newbh, sizeof(newbh), bno);
                blog("Admin", newbh.title);
                vmsg("刪板完畢");
            }
        }
        break;

    case 'e':

        move(9, 0);
        clrtoeol();
        outs("直接按 [Return] 不修改該項設定");

        if (!m_setbrd(&newbh))
        {
            if ((vans(msg_sure_ny) == 'y') &&
                memcmp(&newbh, bhdr, sizeof(newbh)))
            {
                if (strcmp(bhdr->brdname, newbh.brdname))
                {
                    char src[80], dst[80];
                    /* Thor.980806: 特別注意如果 board不在同一partition裏的話會有問題 */
                    sprintf(src, "gem/brd/%s", bhdr->brdname);
                    sprintf(dst, "gem/brd/%s", newbh.brdname);
                    rename(src, dst);
                    rename(src + 4, dst + 4);
                }
                memcpy(bhdr, &newbh, sizeof(BRD));
                rec_put(FN_BRD, &newbh, sizeof(BRD), bno);
            }
        }
        vmsg("設定完畢");
        break;
    }
}

int a_editbrd(void)                /* cache.100618: 修改看板選項 */
{
    int bno;
    BRD *brd;
    char bname[IDLEN + 1];

    if ((brd = ask_board(bname, BRD_R_BIT, NULL)))
    {
        bno = brd - bshm->bcache;
        brd_edit(bno);
    }
    else
    {
        vmsg("找不到這個看板");
    }

    return 0;
}

#ifdef  HAVE_REGISTER_FORM
/* ----------------------------------------------------- */
/* 使用者填寫註冊表格                                    */
/* ----------------------------------------------------- */


static void getfield(int line, int len, char *hint, char *desc, char *buf)
{
    move(line, 0);
    prints("%s:%s\n", desc, hint);
    vget(line + 1, 0, "         ", buf, len, GCARRY);
}

int check_idno(char *s)
{
    char *p, *LEAD = "ABCDEFGHJKLMNPQRSTUVXYWZIO";
    int x, i;

    if (strlen(s) != 10 || (p = strchr(LEAD, toupper(*s))) == NULL)
        return 0;
    x = p - LEAD;
    x = x / 10 + x % 10 * 9;
    p = s + 1;
    if (*p != '1' && *p != '2')
        return 0;
    for (i = 1; i < 9; i++)
    {
        if (isdigit(*p))
            x += (*p++ - '0') * (9 - i);
        else
            return 0;
    }
    x = 9 - x % 10;
    return (x == *p - '0');
}                                /* CheckID */

#ifndef HAVE_SIMPLE_RFORM
static void send_request(void)
{
    RFORM_R form;
    int check;

    check = REG_SENT;

    strcpy(form.userid, cuser.userid);
    form.userno = cuser.userno;
    if (!vget(b_lines, 0, "請輸入原因 :", form.msg, 80, DOECHO))
    {
        vmsg("送交失敗");
        return;
    }
    rec_add(FN_RFORM_R, &form, sizeof(RFORM_R));
    attr_put(cuser.userid, ATTR_REG_KEY, &check);
    vmsg("送交成功\");
}

#endif

int u_register(void)
{
    FILE *fn;
    int ans;
    RFORM rform;
#ifndef HAVE_SIMPLE_RFORM
    int formstate = 0;

    if (attr_get(cuser.userid, ATTR_REG_KEY, &formstate) >= 0)
    {
        if (!(formstate))
        {
            vmsg("請善用 POP3 或 E-mail 認證方式");
            return 0;
        }
        else if (formstate == REG_REQUEST)
        {
            if (vans("是否請求註冊單申請方式 ? [N]") == 'y')
                send_request();
            return 0;
        }
        else if (formstate == REG_SENT)
        {
            vmsg("已經請求過了，請耐心等待 !");
            return 0;
        }
        else if (formstate == REG_FAULT)
        {
            vmsg("您不能使用註冊單 !");
            return 0;
        }
    }
    else
    {
        vmsg("請善用 POP3 或 E-mail 認證方式");
        return 0;
    }
#endif  /* #ifndef HAVE_SIMPLE_RFORM */

    if (HAS_PERM(PERM_VALID))
    {
        zmsg("您的身份確認已經完成，不需填寫申請表");
        return XEASY;
    }

    if ((fn = fopen(FN_RFORM, "rb")))
    {
        while (fread(&rform, sizeof(RFORM), 1, fn))
        {
            if ((rform.userno == cuser.userno) &&
                !str_cmp(rform.userid, cuser.userid))
            {
                fclose(fn);
                zmsg("您的註冊申請單尚在處理中，請耐心等候");
                return XEASY;
            }
        }
        fclose(fn);
    }

    if (vans("您確定要填寫註冊單嗎(Y/N)？[N] ") != 'y')
        return XEASY;

    clear();
    more(FN_ETC_RFORM, (char *)-1);
    for (ans = 0; ans <= 1; ans++)
    {
        char msg[128];
        sprintf(msg, "%s你願意遵守此規定嗎 ? [N]",
                ans ? "請再詳細看一遍，" : "");
        if (vans(msg) != 'y')
            return 0;
        bell();
    }
    move(2, 0);
    clrtobot();
    prints("%s(%s) 您好，請據實填寫以下的資料：\n(按 [Enter] 接受初始設定)",
           cuser.userid, cuser.username);

    memset(&rform, 0, sizeof(RFORM));
    strcpy(rform.realname, cuser.realname);
    strcpy(rform.address, cuser.address);
    rform.career[0] = rform.phone[0] = '\0';
    for (;;)
    {
        getfield(5, 20, rform.realname, "真實姓名", "請用中文");
        getfield(7, 50, rform.career, "服務單位", "學校系級或單位職稱");
        getfield(9, 60, rform.address, "目前住址", "包括寢室或門牌號碼");
        getfield(11, 20, rform.phone, "連絡電話", "包括長途撥號區域碼");
        getfield(13, 11, rform.idno, "身分證號碼", "請詳細填寫");
        ans = vans("以上資料是否正確(Y/N)？(Q)取消 [N] ");
        if (ans == 'q')
            return 0;
        if (ans == 'y')
            break;
    }
    strcpy(cuser.realname, rform.realname);
    strcpy(cuser.address, rform.address);

    rform.userno = cuser.userno;
    strcpy(rform.userid, cuser.userid);
    (void)time(&rform.rtime);
    rec_add(FN_RFORM, &rform, sizeof(RFORM));
    return 0;
}


/* ----------------------------------------------------- */
/* 處理 Register Form                                    */
/* ----------------------------------------------------- */


static int scan_register_form(int fd)
{
    static char logfile[] = FN_RFORM_LOG;
    static char *reason[] = { "輸入真實姓名", "詳實填寫申請表",
        "詳填住址資料", "詳填連絡電話", "詳填服務單位、或學校系級",
        "用中文填寫申請單", "採用 E-mail 認證", "填寫身分證號碼", NULL
    };

    ACCT muser;
    RFORM rform;
    HDR fhdr;
    FILE *fout;

    int op, n;
    char buf[128], msg[128], *agent, *userid, *str;

    vs_bar("審核使用者註冊資料");
    agent = cuser.userid;

    while (read(fd, &rform, sizeof(RFORM)) == sizeof(RFORM))
    {
        userid = rform.userid;
        move(2, 0);
        prints("申請代號: %s (申請時間：%s)\n", userid, Ctime(&rform.rtime));
        prints("真實姓名: %s    身分證號碼: %s(%s)\n", rform.realname,
               rform.idno, check_idno(rform.idno) ? "正確" : "錯誤");
        prints("服務單位: %s\n", rform.career);
        prints("目前住址: %s\n", rform.address);
        prints("連絡電話: %s\n%s\n", rform.phone, msg_separator);
        clrtobot();

        if ((acct_load(&muser, userid) < 0) || (muser.userno != rform.userno))
        {
            vmsg("查無此人");
            op = 'd';
        }
        else
        {
            acct_show(&muser, 2);
            if (muser.userlevel & PERM_VALID)
            {
                vmsg("此帳號已經完成註冊");
                op = 'd';
            }
            else
            {
                op = vans("是否接受(Y/N/Q/Del/Skip)？[S] ");
            }
        }

        switch (op)
        {
        case 'y':

            muser.userlevel |= PERM_VALID;
            strcpy(muser.realname, rform.realname);
            strcpy(muser.address, rform.address);
            sprintf(muser.email, "%s.bbs@%s", muser.userid, MYHOSTNAME);
            strcpy(muser.vmail, muser.email);
            sprintf(msg, "reg:%s:%s:%s", rform.phone, rform.career, agent);
            str_ncpy(muser.justify, msg, sizeof(muser.justify));
            /* Thor.980921: 保險起見 */

            /* Thor.981022: 手動認證也改認證時間, 每半年會再自動認證一次 */
            time(&muser.tvalid);

            acct_save(&muser);

            usr_fpath(buf, userid, FN_JUSTIFY);
            if ((fout = fopen(buf, "a+")))
            {
                fprintf(fout, "%s\n", msg);
                fclose(fout);
            }

            usr_fpath(buf, userid, fn_dir);
            hdr_stamp(buf, HDR_LINK, &fhdr, FN_ETC_APPROVED);
            strcpy(fhdr.title, "[註冊成功\] 您已經通過身分認證了！");
            strcpy(fhdr.owner, cuser.userid);
            rec_add(buf, &fhdr, sizeof(fhdr));

            strcpy(rform.agent, agent);
            rec_add(logfile, &rform, sizeof(RFORM));

            break;

        case 'q':                /* 太累了，結束休息 */

            do
            {
                rec_add(FN_RFORM, &rform, sizeof(RFORM));
            }
            while (read(fd, &rform, sizeof(RFORM)) == sizeof(RFORM));

        case 'd':
            break;

        case 'n':

            move(9, 0);
            prints("請提出退回申請表原因，按 <enter> 取消\n\n");
            for (n = 0; (str = reason[n]); n++)
                prints("%d) 請%s\n", n, str);
            clrtobot();

            if ((op = vget(b_lines, 0, "退回原因：", buf, 60, DOECHO)))
            {
                int i;
                char folder[80], fpath[80], boardbuf[IDLEN + 1];
                HDR fhdr;

                i = op - '0';
                if (i >= 0 && i < n)
                    strcpy(buf, reason[i]);

                usr_fpath(folder, muser.userid, fn_dir);
                if ((fout = fdopen(hdr_stamp(folder, 0, &fhdr, fpath), "w")))
                {
                    strcpy(ve_title, "[" SYSOPNICK "] 請您重新填寫註冊表單");
                    strcpy(boardbuf, currboard);
                    *currboard = 0;
                    ve_header(fout);
                    *ve_title = 0;
                    strcpy(currboard, boardbuf);

                    fprintf(fout, "\t由於您提供的資料不夠詳實，無法確認身分，"
                            "\n\n\t請重新填寫註冊表單：%s。\n", buf);
                    fclose(fout);

                    strcpy(fhdr.owner, agent);
                    strcpy(fhdr.title, "[" SYSOPNICK "] 請您重新填寫註冊表單");
                    rec_add(folder, &fhdr, sizeof(fhdr));
                }

                strcpy(rform.reply, buf);    /* 理由 */
                strcpy(rform.agent, agent);
                rec_add(logfile, &rform, sizeof(RFORM));

                break;
            }

        default:                /* put back to regfile */

            rec_add(FN_RFORM, &rform, sizeof(RFORM));
        }
    }
    return 0;
}

#ifndef HAVE_SIMPLE_RFORM
static int ans_request(void)
{
    int num, fd;
    RFORM_R form;
    ACCT muser;
    char fpath[128], op, buf[128];
    sprintf(fpath, "%s.tmp", FN_RFORM_R);

    rename(FN_RFORM_R, fpath);
    num = rec_num(fpath, sizeof(RFORM_R));
    if (num <= 0)
    {
        zmsg("目前並無要求註冊的使用者");
        return XEASY;
    }
    vs_bar("審核註冊單請求");
    if ((fd = open(fpath, O_RDONLY)) >= 0)
    {
        while (read(fd, &form, sizeof(RFORM_R)) == sizeof(RFORM_R))
        {
            move(2, 0);
            prints("申請代號: %s\n", form.userid);
            prints("申請理由: %s\n%s\n", form.msg, msg_separator);
            clrtobot();
            if ((acct_load(&muser, form.userid) < 0)
                || (muser.userno != form.userno))
            {
                vmsg("查無此人");
                op = 'd';
            }
            else
            {
                acct_show(&muser, 2);
                if (muser.userlevel & PERM_VALID)
                {
                    vmsg("此帳號已經完成註冊");
                    op = 'd';
                }
                else
                {
                    op = vans("是否接受(yes/no/quit/Skip)？");
                }
            }
            switch (op)
            {
            case 'd':
                break;
            case 'y':
            case 'Y':
                num = REG_OPEN;
                attr_put(muser.userid, ATTR_REG_KEY, &num);
                break;
            case 'n':
            case 'N':
                move(9, 0);
                prints("請提出退回申請表原因，按 <enter> 取消\n\n");
                clrtobot();

                if ((op = vget(b_lines, 0, "退回原因：", buf, 60, DOECHO)))
                {
                    char folder[80], fpath[80], boardbuf[IDLEN + 1];
                    HDR fhdr;
                    FILE *fout;

                    usr_fpath(folder, muser.userid, fn_dir);
                    if ((fout =
                         fdopen(hdr_stamp(folder, 0, &fhdr, fpath), "w")))
                    {
                        strcpy(ve_title, "[" SYSOPNICK "]註冊表單申請退件");
                        strcpy(boardbuf, currboard);
                        *currboard = 0;
                        ve_header(fout);
                        *ve_title = 0;
                        strcpy(currboard, boardbuf);

                        fprintf(fout, "\n\n\t退回原因：%s。\n", buf);
                        fclose(fout);

                        strcpy(fhdr.owner, cuser.userid);
                        strcpy(fhdr.title, "[" SYSOPNICK "] 註冊表單申請退件");
                        rec_add(folder, &fhdr, sizeof(fhdr));
                    }
                    num = REG_FAULT;
                    attr_put(muser.userid, ATTR_REG_KEY, &num);
                }
                else
                    rec_add(FN_RFORM_R, &form, sizeof(RFORM_R));

                break;
            case 'q':
            case 'Q':
                do
                {
                    rec_add(FN_RFORM_R, &form, sizeof(RFORM_R));
                }
                while (read(fd, &form, sizeof(RFORM_R)) == sizeof(RFORM_R));
                break;
            default:
                rec_add(FN_RFORM_R, &form, sizeof(RFORM_R));
            }
        }
        close(fd);
        unlink(fpath);
    }

    return 0;
}
#endif  /* #ifndef HAVE_SIMPLE_RFORM */

int m_register(void)
{
    int num;
#ifndef HAVE_SIMPLE_RFORM
    int num2;
    char msg[128];
#endif
    char buf[80];

    num = rec_num(FN_RFORM, sizeof(RFORM));
#ifndef HAVE_SIMPLE_RFORM
    num2 = rec_num(FN_RFORM_R, sizeof(RFORM_R));
    sprintf(msg, "審核 : 1)註冊單< %d 筆> 2)註冊單請求< %d 筆> [1]", num,
            num2);
#endif

#ifndef HAVE_SIMPLE_RFORM
    if (vans(msg) == '2')
    {
        return ans_request();
    }
#endif
    if (num <= 0)
    {
        clrtoeol();
        zmsg("目前並無新註冊資料");
        return XEASY;
    }

    sprintf(buf, "共有 %d 筆資料，開始審核嗎(Y/N)？[N] ", num);
    num = XEASY;

    if (vans(buf) == 'y')
    {
        sprintf(buf, "%s.tmp", FN_RFORM);
        if (dashf(buf))
        {
            vmsg("其他 SYSOP 也在審核註冊申請單");
            if (vans("是否使用修正功\能？") == 'y')
            {
                clear();
                prints
                    ("\n\n\x1b[1;33m請確定沒有其他站務在審核，否則將造成\x1b[1;31;5m使用者資料嚴重錯誤!\x1b[m\n\n\n");
                if (vans("確定無其他站務審核中？") == 'y')
                {
                    system("/bin/cat run/" FN_RFORM ".tmp >> run/" FN_RFORM
                           ";/bin/rm -f ~bbs/run/" FN_RFORM ".tmp");
                    vmsg("修正完畢，下次請小心審核! 按任意鍵重新開始.");
                }
            }
        }
        else
        {
            int fd;

            rename(FN_RFORM, buf);
            fd = open(buf, O_RDONLY);
            if (fd >= 0)
            {
                scan_register_form(fd);
                close(fd);
                unlink(buf);
                num = 0;
            }
            else
            {
                vmsg("無法開啟註冊資料工作檔");
            }
        }
    }
    return num;
}
#endif  /* #ifdef  HAVE_REGISTER_FORM */


/* ----------------------------------------------------- */
/* 產生追蹤記錄：建議改用 log_usies()、TRACE()           */
/* ----------------------------------------------------- */


#ifdef  HAVE_REPORT
void report(char *s)
{
    static int disable = NA;
    int fd;

    if (disable)
        return;

    if ((fd = open("trace", O_WRONLY, 0600)) != -1)
    {
        char buf[256];
        char *thetime;
        time_t dtime;

        time(&dtime);
        thetime = Etime(&dtime);

        /* flock(fd, LOCK_EX); */
        /* Thor.981205: 用 fcntl 取代flock, POSIX標準用法 */
        f_exlock(fd);

        lseek(fd, 0, L_XTND);
        sprintf(buf, "%s %s %s\n", cuser.userid, thetime, s);
        write(fd, buf, strlen(buf));

        /* flock(fd, LOCK_UN); */
        /* Thor.981205: 用 fcntl 取代flock, POSIX標準用法 */
        f_unlock(fd);

        close(fd);
    }
    else
        disable = YEA;
}


int m_trace(void)
{
    struct stat bstatb, ostatb, cstatb;
    int btflag, otflag, ctflag, done = 0;
    char ans[2];
    char *msg;

    clear();
    move(0, 0);
    outs("Set Trace Options");
    clrtobot();
    while (!done)
    {
        move(2, 0);
        otflag = stat("trace", &ostatb);
        ctflag = stat("trace.chatd", &cstatb);
        btflag = stat("trace.bvote", &bstatb);
        outs("Current Trace Settings:\n");
        if (otflag)
            outs("Normal tracing is OFF\n");
        else
            prints("Normal tracing is ON (size = %d)\n", ostatb.st_size);
        if (ctflag)
            outs("Chatd  tracing is OFF\n");
        else
            prints("Chatd  tracing is ON (size = %d)\n", cstatb.st_size);
        if (btflag)
            outs("BVote  tracing is OFF\n");
        else
            prints("BVote  tracing is ON (size = %d)\n", bstatb.st_size);

        move(8, 0);
        outs("Enter:\n");
        prints("<1> to %s Normal tracing\n", otflag ? "enable " : "disable");
        prints("<2> to %s Chatd  tracing\n", ctflag ? "enable " : "disable");
        prints("<3> to %s BVote  tracing\n", btflag ? "enable " : "disable");
        vget(12, 0, "Anything else to exit：", ans, 2, DOECHO);

        switch (ans[0])
        {
        case '1':
            if (otflag)
            {
                system("/bin/touch trace");
                msg = "BBS   tracing enabled.";
                report("opened report log");
            }
            else
            {
                report("closed report log");
                f_mv("trace","trace.old");
                msg = "BBS   tracing disabled; log is in trace.old";
            }
            break;

        case '2':
            if (ctflag)
            {
                system("/bin/touch trace.chatd");
                msg = "Chat  tracing enabled.";
                report("chatd trace log opened");
            }
            else
            {
                f_mv("trace.chatd","trace.chatd.old");
                msg = "Chat  tracing disabled; log is in trace.chatd.old";
                report("chatd trace log closed");
            }
            break;

        case '3':
            if (btflag)
            {
                system("/bin/touch trace.bvote");
                msg = "BVote tracing enabled.";
                report("BVote trace log opened");
            }
            else
            {
                f_mv("trace.bvote","trace.bvote.old");
                msg = "BVote tracing disabled; log is in trace.bvote.old";
                report("BoardVote trace log closed");
            }
            break;

        default:
            msg = NULL;
            done = 1;
        }
        move(b_lines - 1, 0);
        if (msg)
            prints("%s\n", msg);
    }
    clear();
}
#endif /* HAVE_REPORT */

int u_verify(void)
{
    char keyfile[80], buf[80], /* buf2[80], */ inbuf[8], *key;
    FILE *fp;
    HDR fhdr;

    if (HAS_PERM(PERM_VALID))
    {
        zmsg("您的身份確認已經完成，不需填寫認證碼");
        return XEASY;
    }

    usr_fpath(keyfile, cuser.userid, FN_REGKEY);
    if (!dashf(keyfile))
    {
        zmsg("沒有您的認證碼喔，請重填 email address");
        return XEASY;
    }

    if (!(fp = fopen(keyfile, "r")))
    {
        fclose(fp);
        zmsg("開啟檔案有問題，請通知站長");
        return XEASY;
    }
    // FIXME(IID.20190415): What is the intent of the loop? Fetch last line?
    while (fgets(buf, 80, fp))
    {
        // FIXME(IID.20190415): The `strtok()` does absolutely nothing other than `key = buf`.
        //    To skip initial null characters? This does not work
        key = strtok(buf, "");
    }
    fclose(fp);

    if (vget(b_lines, 0, "請輸入認證碼：", inbuf, 8, DOECHO))
    {
        if (strncmp(key, inbuf, 7))
        {
            zmsg("抱歉，您的認證碼錯誤。");

            /* jasonmel 20030731 : Hide the keys... */
            /*sprintf(buf2, "KEY: %s", key);
               vmsg(buf3);
               sprintf(buf2, "INPUT: %s", inbuf);
               vmsg(buf2); */
            return XEASY;
        }
        else
        {
            unlink(keyfile);

            cuser.userlevel |=
                (PERM_VALID | PERM_POST | PERM_PAGE | PERM_CHAT);
            strcpy(cuser.vmail, cuser.email);
            sprintf(buf, "key認證:%s", cuser.email);
            str_ncpy(cuser.justify, buf, sizeof(cuser.justify));
            time(&cuser.tvalid);
            acct_save(&cuser);
            usr_fpath(buf, cuser.userid, fn_dir);
            hdr_stamp(buf, HDR_LINK, &fhdr, "etc/justified");
            strcpy(fhdr.title, "[註冊成功\] 您已經通過身分認證了！");
            strcpy(fhdr.owner, str_sysop);
            rec_add(buf, &fhdr, sizeof(fhdr));
            board_main();
            gem_main();
            talk_main();
            cutmp->ufo |= UFO_BIFF;
            vmsg("身份確認成功\，立刻提昇權限");
        }
    }

    return 0;
}
