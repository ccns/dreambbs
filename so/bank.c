/*-------------------------------------------------------*/
/* so/bank.c                                             */
/*-------------------------------------------------------*/
/* author : shiun.bbs@ccca.mksh.phc.edu.tw               */
/* modify : cat.bbs@ccca.mksh.phc.edu.tw                 */
/* update : cache.bbs@bbs.ee.ncku.edu.tw                 */
/* create : unknown                                      */
/* update : 09/10/11                                     */
/*-------------------------------------------------------*/
/* syntax :                                              */
/*-------------------------------------------------------*/

#include "bbs.h"
/*
void log_bank(
    int mode,
    int a,
    int b,
    const char *who)
{
    time_t now;
    char c_time[25], c_buf[100]={0};

    now = time(0);
    str_scpy(c_time, ctime(&now), sizeof(c_time));

    if (mode == 1)
        sprintf(c_buf, "%s %s 優良點數(%d)->夢幣(%d)\n", c_time, cuser.userid, a, b);
    else if (mode == 2)
        sprintf(c_buf, "%s %s 夢幣(%d)->優良點數(%d)\n", c_time, cuser.userid, a, b);
    else if (mode == 3)
        sprintf(c_buf, "%s %s 夢幣(%d)->股票(%d)\n", c_time, cuser.userid, a, b);
    else if (mode == 4)
    {
        sprintf(c_buf, "%s %s 匯款(%d)-> %s (%d)\n", c_time, cuser.userid, a, *who, b);
    }
    else if (mode == 5)
        sprintf(c_buf, "%s %s 匯入舊夢幣(%d)\n", c_time, cuser.userid, a);

    f_cat(FN_BANK, c_buf);
}
*/

static int point1_money(void)
{
    int num;
    char buf[10];
    GCC_UNUSED int money;
    ACCT acct;

    if (acct_load(&acct, cuser.userid) >= 0)
        money = acct.money;
    else
    {
        pmsg2("查無您的帳戶資訊...");
        return 0;
    }

#ifdef M3_USE_PFTERM
    clrregion(0, 22);
#else
    clearange(0, 22);
#endif
    vs_bar("夢幣轉換");

    move(2, 0);
    prints("你的身上有 %9d 夢幣\n\n           %9d 優良點數",
           acct.money, acct.point1);
    if (acct.point1 < 1)
    {
        pmsg2("優良點數不足");
        return 0;
    }

    vget(8, 0, "要轉換多少優良點數？", buf, 8, DOECHO);
    if ((num = atoi(buf)) <= 0)
        return 0;
    else
    {
        double temp = num*4096;

        if (temp + acct.money > INT_MAX)
        {
            pmsg2("轉換後夢幣超過上限！");
            return 0;
        }

        const int dmoney = (int)temp;

        if (acct_load(&acct, cuser.userid) >= 0)
        {
            acct.money += dmoney;
            acct.point1 -= num;
        }

        acct_save(&acct);

        time_t now;
        char c_time[25], c_buf[100]={0};
        now = time(0);
        str_scpy(c_time, ctime(&now), sizeof(c_time));
        sprintf(c_buf, "%s %s 優良點數(%d)->夢幣(%+d)\n", c_time, cuser.userid, num, dmoney);
        f_cat(FN_BANK, c_buf);

    }
    pmsg2("轉換完畢");
    return 0;
}


static int
TransferAccount(void)
{
    ACCT acct, selfacct;
    char buf[128];
    time_t now;
    HDR xhdr;
    FILE *fp;
    char folder[128], date[9], fpath[128];
    char userid[IDLEN + 1];
    char str[128];
    int selfmoney GCC_UNUSED, pay;
    int dmoney;

#ifdef M3_USE_PFTERM
    clrregion(0, 22);
#else
    clearange(0, 22);
#endif
    vs_bar("匯款");

    move (9, 8);
    prints("\x1b[1;33m轉帳相關規定： \x1b[36m１. 一次最少要給 100 夢幣(稅前)。\x1b[m\n"
        "                       \x1b[36m２. 收手續費 10 %%。\x1b[m");

    if (acct_get("要匯給誰：", &acct)<1)
        return 0;
    if (acct.userno == cuser.userno)
    {
        pmsg2("不能跟自己交易啦！");
        return 0;
    }
    strcpy(userid, acct.userid);

#ifdef M3_USE_PFTERM
    clrregion(1, 21);
#else
    clearange(1, 21);
#endif
    move(3, 0);

    if (acct_load(&selfacct, cuser.userid) >= 0)
        selfmoney = selfacct.money;
    else
    {
        pmsg2("查無您的帳戶資訊...");
        return 0;
    }

    prints("你自己的身上還有 %9d 夢幣。\n", selfacct.money);
    prints("\n%-12s則有 %9d 夢幣。", userid, acct.money);

    if (!vget(7, 0, "你要匯款多少夢幣：", buf, 10, DOECHO))
        return 0;

    dmoney = atoi(buf);

    if (dmoney < 100)
    {
        pmsg2("匯款金額不得低於 100 元");
        return 0;
    }
    else if (dmoney > selfacct.money)
    {
        pmsg2("匯款金額超過能匯出的上限");
        return 0;
    }
    else if ((double)dmoney + acct.money > INT_MAX)
    {
        pmsg2("匯款金額超過對方能接受的上限");
        return 0;
    }

    pay = (int)(dmoney * 1.1);

    move(9, 0);
    prints("欲轉 %d 元夢幣(稅前)，實際支付 %d 夢幣(稅後)", dmoney, pay);

    move(11, 0);
    clrtobot();

    if (!vget(B_LINES_REF, 0, "匯款理由：", str, 60, DOECHO))
        return 0;

    if (vans("確定要給他嗎？ [Y/n]") != 'n')
    {

        now = time(0);
        str_stamp(date, &now);

        usr_fpath(folder, userid, FN_DIR);
        fp = fdopen(hdr_stamp(folder, 0, &xhdr, fpath), "w");
        strcpy(xhdr.owner, cuser.userid);
        strcpy(xhdr.nick, cuser.username);
        sprintf(xhdr.title, "匯款通知");
        strcpy(xhdr.date, date);

        fprintf(fp, "作者: %s (%s)\n", cuser.userid, cuser.username);
        fprintf(fp, "標題: 匯款通知\n");
        fprintf(fp, "時間: %s\n", date);
        fprintf(fp, "\n\n%s 送給你 \x1b[1;31m%d\x1b[m 夢幣，請笑納~\n", cuser.userid, dmoney);
        fprintf(fp, "\n\n匯款理由：%s\n", str);
        fclose(fp);
        rec_add(folder, &xhdr, sizeof(HDR));

        acct.money += dmoney;
        selfacct.money -= pay;
        acct_save(&acct);
        acct_save(&selfacct);

        time_t now;
        char c_time[25], c_buf[100]={0};
        now = time(0);
        str_scpy(c_time, ctime(&now), sizeof(c_time));
        sprintf(c_buf, "%s %s 匯款(%d)-> %s (%+d)\n", c_time, cuser.userid, pay, userid, dmoney);
        f_cat(FN_BANK, c_buf);

    }

    pmsg2("交易完成");
    return 0;
}

/* 100618.cache: 舊夢幣轉換 */
#define FN_MONEY        ".MONEY"        /* PostRecommendHistory */

typedef struct
{
    int32_t money;      /* 夢幣 */
    int32_t save;       /* 存款 */
    int32_t request;    /* 小雞點券 */
}       MONEY;  /* DISKDATA(raw) */

/*
int
money_back(void)
{
    DL_HOLD;
    pmsg2("此活動已截止");
    return DL_RELEASE(0);
}
*/

    int
money_back(void)
{
    DL_HOLD;
    ACCT acct;
    char buf[128];
    char fpath[80];
    int fd;
    double m1 = 0;
    double m2 = 0;
    GCC_UNUSED double m3 = 0;
    MONEY oldwealth;

    //if (acct_get("要送誰點歌次數：", &acct) < 1)
    //    return 0;
    //clrtobot();

    if (acct_get("請輸入ID：", &acct) < 1)
    {
        pmsg2("查無此 ID 的帳戶資訊...");
        return DL_RELEASE(0);
    }
    {
        clrtobot();
        usr_fpath(fpath, acct.userid, FN_MONEY);

        //讀取MONEY
        if ((fd = open(fpath, O_RDONLY)) < 0)
        {
            pmsg2("查無此 ID 的舊夢幣資料...");
            return DL_RELEASE(0);
        }

        read(fd, &oldwealth, sizeof(MONEY));

        m1 = oldwealth.money;
        m2 = oldwealth.save;
        m3 = oldwealth.request;

        m1 = ((m1+m2)/2) - 1;  //請自行修改

        m1 = BMIN(m1, (double)INT_MAX);
        m2 = BMIN(m2, (double)INT_MAX);

        //為了方便所以沒有砍掉舊記錄, 對方可能洗錢
        //if (acct.money > 65535)
        //{
        //    pmsg2("不符合匯入資格...");
        //    return 0;
        //}

        //if (acct.money < 10)
        //{
            //pmsg2("不符合匯入資格...");
            //return 0;
        //}

        m1 = (int)m1;

        acct.money = m1;
        acct_save(&acct);

        time_t now;
        char c_time[25], c_buf[100]={0};
        now = time(0);
        str_scpy(c_time, ctime(&now), sizeof(c_time));
        sprintf(c_buf, "%s %s 匯入舊夢幣(%d)\n", c_time, acct.userid, (int)m1);
        f_cat(FN_BANK, c_buf);

        pmsg2("舊夢幣匯入完畢");

        unlink(buf);
        close(fd);

        return DL_RELEASE(0);

    }

    // return DL_RELEASE(0);

}


int bank_main(void)
{
    DL_HOLD;
    char buf[2];
    int money;
    int point1;

    ACCT acct;
    if (acct_load(&acct, cuser.userid) >= 0)
    {
        money = acct.money;
        point1 = acct.point1;
    }
    else
    {
        pmsg2("查無您的帳戶資訊...");
        return DL_RELEASE(0);
    }

    clear();

    move(0, 0);
    prints("\x1b[1;33;42m");
    vs_mid(BOARDNAME "    銀行");
    prints("\n");
    move(10, 0);
    prints("  您的帳號資訊如下    １. 用優良點數換取夢幣\n\n");
    prints("                      ２. 用夢幣換取優良點數\n\n");
    prints("                      ３. 投資\n\n");
    prints("                      ４. 匯款給其他人\n");
    prints("\n");

    move (4, 2);
    prints("夢幣 %d ", money);
    move (6, 2);
    prints("優良積分 %d ", point1);
    if (!vget(B_LINES_REF, 0, "請選擇您要的服務： [Q] 離開 ", buf, 2, DOECHO))
        return DL_RELEASE(0);

    if (*buf == '1')
        point1_money();
    else if (*buf == '2')
        pmsg2("此功\能尚未開放");
    else if (*buf == '3')
        pmsg2("此功\能尚未開放");
    else if (*buf == '4')
        TransferAccount();
    else
        pmsg2("離開銀行");

    return DL_RELEASE(0);
}
