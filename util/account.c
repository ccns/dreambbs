/*-------------------------------------------------------*/
/* util/account.c       ( NTHU CS MapleBBS Ver 3.00 )    */
/*-------------------------------------------------------*/
/* target : 上站人次統計、系統資料備份、開票             */
/* create : 95/03/29                                     */
/* update : 97/03/29                                     */
/*-------------------------------------------------------*/
/* syntax : 本程式宜以 crontab 執行，設定時間為每小時    */
/* 1-5 分 之間                                           */
/*-------------------------------------------------------*/
/* notice : brdshm (board shared memory) synchronize     */
/*-------------------------------------------------------*/

#include "bbs.h"

#define MAX_LINE        16

static const char fn_today[] = "gem/@/@-act"; /* 今日上站人次統計 */
static const char fn_yesterday[] = "gem/@/@=act";     /* 昨日上站人次統計 */

/* ----------------------------------------------------- */
/* 開票：shm 部份須與 cache.c 相容                       */
/* ----------------------------------------------------- */

static BCACHE *bshm;


struct Tchoice
{
    int count;
    vitem_t vitem;
};


static int
TchoiceCompare(const void * i, const void * j)
{
    return ((const struct Tchoice *)j)->count - ((const struct Tchoice *)i)->count;
}


static int
draw_vote(
    const BRD *bdh,             /* Thor: 傳入 BRD, 可查 battr */
    char *fpath,
    const VCH *vch)
{
    FILE *fp;
    char *fname, buf[80], bpath[80];
    const char *bid;
    struct Tchoice choice[MAX_CHOICES];
    int total, items, num, fd, ticket, bollt, *list, *head, *tail;
    struct stat st;

    bid = bdh->brdname;
    fname = strrchr(fpath, '@');

    /* vote item */

    *fname = 'I';
    items = 0;
    if ((fp = fopen(fpath, "r")))
    {
        while (fread(choice[items].vitem, sizeof(vitem_t), 1, fp) == 1)
        {
            choice[items].count = 0;
            items++;
        }
        fclose(fp);
        unlink(fpath);
    }

    if (items == 0)
        return 0;

    /* vote ballots */

    *fname = 'V';

    if ((fd = open(fpath, O_RDONLY)) < 0)
        return 0;

    if (fstat(fd, &st) || (total = st.st_size) <= 0)
    {
        close(fd);
        unlink(fpath);
        return 0;
    }

    /* 累計投票結果 */

    bollt = 0;                  /* Thor: 總票數歸0 */

    list = (int *) malloc(total);
    total = read(fd, list, total);
    close(fd);
    unlink(fpath);

    if (total <= 0)
    {
        free(list);
        return 0;
    }

    head = list;
    tail = (int *) ((char *) list + total);

    do
    {
        for (ticket = *head++, num = 0; ticket && num < items; ticket >>= 1, num++)
        {
            if (ticket & 1)
            {
                choice[num].count++;
                bollt++;
            }
        }
    } while (head < tail);

    free(list);

    /* 產生開票結果 */

    *fname = 'Z';
    if ((fp = fopen(fpath, "w")) == NULL)
        return 0;

    memset(buf, '-', 74);
    buf[74] = '\0';
    fprintf(fp, "\n\n> %s <\n\n◆ [%s] 看板投票：%s\n\n舉辦人  ：%s\n\n舉辦日期：%s\n",
        buf, bid, vch->title, vch->owner, ctime_any(&vch->chrono));
    fprintf(fp, "開票日期：%s\n◆ 投票主題:\n\n", ctime_any(&vch->vclose));

    *fname = '@';
    f_suck(fp, fpath);
    unlink(fpath);

    fprintf(fp, "◆ 投票結果：每人可投 %d 票，共 %zu 人參加，投出 %d 票\n\n",
        vch->maxblt, total / sizeof(int), bollt);

    if (vch->vsort == 's')
        qsort(choice, items, sizeof(struct Tchoice), (int (*)(const void *lhs, const void *rhs))TchoiceCompare);

    if (vch->vpercent == '%')
        fd = BMAX((size_t)1, total / sizeof(int));
    else
        fd = 0;

    for (num = 0; num < items; num++)
    {
        ticket = choice[num].count;
        if (fd)
            fprintf(fp, "\t%-42s%3d 票 (%4.1f%%)\n",
                (char *)choice[num].vitem, ticket, 100.0 * ticket / fd);
        else
            fprintf(fp, "\t%-42s%3d 票\n",
                (char *)choice[num].vitem, ticket);
    }

    /* other opinions */

    *fname = 'O';
    fputs("\n◆ 我有話要說：\n\n", fp);
    f_suck(fp, fpath);
    fclose(fp);

    fp = fopen(fpath, "w");     /* Thor: 用 O_ 暫存一下下... */
    *fname = 'Z';
    f_suck(fp, fpath);
    sprintf(bpath, "brd/%s/@/@vote", bid);
    f_suck(fp, bpath);
    fclose(fp);
    *fname = 'O';
    rename(fpath, bpath);

    /* 將開票結果 post 到 [system] 與 本看板 */
    *fname = 'S';
    unlink(fpath);
    *fname = 'E';
    unlink(fpath);

    *fname = 'Z';

    /* Thor: 若看版屬性為 BRD_NOVOTE, 則不post到 sysop */

    if (!(bdh->battr & BRD_NOVOTE))
    {
        sprintf(buf, "[記錄] %s <<看板選情報導>>", bid);
        keeplog(fpath, NULL, buf, 0);
    }

    keeplog(fpath, bid, "[記錄] 選情報導", 2);

    return 1;
}


static int
draw_board(
    BRD *brd)
{
    int bvote, fd, fsize, alive;
    VCH *vch, *head, *tail;
    time_t now;
    char *fname, fpath[80], buf[80];
    struct stat st;

    /* 由 account 來 maintain brd->bvote */

    bvote = brd->bvote;

    sprintf(fpath, "brd/%s/.VCH", brd->brdname);

    if ((fd = open(fpath, O_RDWR | O_APPEND, 0600)) < 0
        || fstat(fd, &st)
        || (fsize = st.st_size) <= 0)
    {
        if (fd >= 0)
        {
            close(fd);
            unlink(fpath);
        }
        brd->bvote = 0;
        return bvote ? -1 : 0;
    }

    vch = (VCH *) malloc(fsize);

    /* flock(fd, LOCK_EX); */
    /* Thor.981205: 用 fcntl 取代flock, POSIX標準用法 */
    f_exlock(fd);

    read(fd, vch, fsize);

    strcpy(buf, fpath);
    fname = strrchr(buf, '.');
    *fname++ = '@';
    *fname++ = '/';

    head = vch;
    tail = (VCH *) ((char *) vch + fsize);

    alive = 0;

    now = time(NULL);

    do
    {
        if (head->vclose < now)
        {
            strcpy(fname, head->xname);
            draw_vote(brd, buf, head);/* Thor: 傳入 BRD, 可查 battr */
            head->chrono = 0;
        }
        else
        {
            alive++;
        }
    } while (++head < tail);


    if (alive && alive != fsize / sizeof(VCH))
    {
        ftruncate(fd, 0);
        head = vch;
        do
        {
            if (head->chrono)
            {
                write(fd, head, sizeof(VCH));
            }
        } while (++head < tail);
    }

    /* flock(fd, LOCK_UN);  */
    /* Thor.981205: 用 fcntl 取代flock, POSIX標準用法 */
    f_unlock(fd);

    close(fd);
    if (!alive)
        unlink(fpath);

    free(vch);

    if (alive != bvote)
    {
        brd->bvote = alive;
        return 1;
    }

    return 0;
}


static void
closepolls(void)
{
    BRD *bcache, *head, *tail;
    int state;

    state = 0;

    head = bcache = bshm->bcache;
    tail = head + bshm->number;
    do
    {
        state |= draw_board(head);
    } while (++head < tail);

    if (!state)
        return;

    /* write back the shm cache data */

    if ((state = open(FN_BRD, O_WRONLY | O_CREAT, 0600)) < 0)
        return;

    /* flock(state, LOCK_EX); */
    /* Thor.981205: 用 fcntl 取代flock, POSIX標準用法 */
    f_exlock(state);

    write(state, bcache, (char *) tail - (char *) bcache);

    /* flock(state, LOCK_UN); */
    /* Thor.981205: 用 fcntl 取代flock, POSIX標準用法 */
    f_unlock(state);

    close(state);
    time32(&bshm->uptime);
}


static void
ansi_puts(
    FILE *fp,
    char buf[], char mode)
{
    static char state = '\0';

    if (mode == '\n')
    {
        /* Reset the last ANSI state on line wraps */
        state = '\0';
        fprintf(fp, "\x1b[m\n");
        return;
    }

    if (state != mode)
        fprintf(fp, "\x1b[3%cm", state = mode);
    if (buf[0])
    {
        fprintf(fp, "%s", buf);
        buf[0] = 0;
    }
}


static void
gzip(
    const char *source, const char *target, char *stamp)
{
    char buf[128];

    sprintf(buf, "log/%s%s", target, stamp);
    /* rename(source, buf); */
    f_mv(source, &buf[4]); /* Thor.990409: 可跨 partition */
    PROC_CMD("/bin/gzip", "-n", buf);
}


#if 0  // Unused
static void
gtar(
    const char *source, const char *target, const char *stamp,
    int prune)
{
    char buf[128];

    sprintf(buf, "log/%s%s.tgz", target, stamp);
    PROC_CMD("/bin/tar", "cfz", buf, source);

    if (prune)
    {
        f_rm(source);
    }
}
#endif

static void
error(
    const char *fpath)
{
    printf("can not open [%s]\n", fpath);
    exit(1);
}


int
main(void)
{
    /* IID.2021-04-05: 128 resolution bar chart for login statistic */
    static const char *const act_bar[] = {"　", "▁", "▂", "▃", "▄", "▅", "▆", "▇"};
    static const char *const act_bar_full = "█";

    int fact, max, item, total, over;
    char date[16];
    char title[80];

    static const char act_file[] = "run/var/act";
    static const char run_file[] = FN_USIES;
    static const char tmp_file[] = "run/usies_tmp";
    static const char log_file[] = "run/usies=";
//  static const char brd_file[] = FN_BRD_USIES;

    char buf[256], ymd[16];
    FILE *fp, *fpw;

    int act[26];                        /* 次數/累計時間 */
    time_t now, day;
    struct tm ntime, ptime;

    /* Get the current time point */
    now = time(NULL);
    ntime = *localtime(&now);

    /* Get the begin time point of today */
    {
        struct tm dtime = ntime;
        dtime.tm_hour = 0;
        dtime.tm_min = 0;
        dtime.tm_sec = 0;
        day = mktime(&dtime);
    }

    /* Get a time point on yesterday */
    now = day - 60;  /* back to yesterday */
    ptime = *localtime(&now);

    setgid(BBSGID);
    setuid(BBSUID);
    chdir(BBSHOME);
    umask(077);

    /* --------------------------------------------------- */
    /* 上站人次統計                                        */
    /* --------------------------------------------------- */

    memset(act, 0, sizeof(act));
    fact = open(act_file, O_RDWR | O_CREAT, 0600);
    if (fact < 0)
        error(act_file);

    if (ntime.tm_hour != 1)
    {
        read(fact, act, sizeof(act));
        lseek(fact, 0, SEEK_SET);
    }

    if (rename(run_file, tmp_file))
    {
        PROC_CMD("/bin/touch", tmp_file);
    }
    if ((fp = fopen(tmp_file, "r")) == NULL)
        error(tmp_file);

    if ((fpw = fopen(log_file, "a")) == NULL)
        error(log_file);

    while (fgets(buf, sizeof(buf), fp))
    {
        fputs(buf, fpw);

        if (!strncmp(buf + 22, "ENTER", 5))
        {
            const int hour = atoi(buf + 9);
            if (hour >= 0 && hour <= 23)
                act[hour]++;
            continue;
        }

        if (!strncmp(buf + 41, "Stay:", 5))
        {
            const int hour = atoi(buf + 47);
            if (hour)
            {
                act[24] += hour;
                act[25]++;
            }
            continue;
        }
    }
    fclose(fp);
    fclose(fpw);
    unlink(tmp_file);

    // IID.20190311: Zero-out the file `fact` to reset the login count at 0 am.
    if (ntime.tm_hour == 0)
        for (int i = 0; i < sizeof(act); i++)
            write(fact, "", sizeof(char));
    else
        write(fact, act, sizeof(act));

    close(fact);

    max = 1; /* Make the minimum value of the bar unit to be `1` */
    total = 0;
    for (int i = 0; i < 24; i++)
    {
        total += act[i];
        if (act[i] > max)
            max = act[i];
    }

    /* Calculate the units for the bar chart */

    /* Use ceiling division for the bar unit to prevent the bar from exceeding the chart */
    item = (max + (MAX_LINE * COUNTOF(act_bar)) - 1) / (MAX_LINE * COUNTOF(act_bar));
    /* Keep the displayed numbers in the bar chart under `1000` */
    for (over = 1; over < 1000000000; over *= 10)
    {
        if (1000 * over > max)
            break;
    }

    if (ntime.tm_hour == 1)
        keeplog(fn_today, NULL, "[記錄] 上站人次統計", 1);

    if ((fp = fopen(fn_today, "w")) == NULL)
        error(fn_today);

    /* Thor.990329: y2k */
    fprintf(fp, "\t\t\t   \x1b[1;33;46m [%02d/%02d/%02d] 上站人次統計 \x1b[m\n",
        ptime.tm_year % 100, ptime.tm_mon + 1, ptime.tm_mday);

    /* Generate the chart from the topmost row to the bottommost row */
    /* The topmost row is reserved for the number texts */
    for (int i = MAX_LINE; i >= 0; i--)
    {
        fprintf(fp, "\x1b[1m");
        strcpy(buf, "   ");
        /* Output the bar or number text for each hour */
        for (int h = 0; h < 24; h++)
        {
            const int bar_min = item * i * COUNTOF(act_bar);
            const int acth = act[h];
            const int acth_num = acth + item * COUNTOF(act_bar); /* Space for the number text */
            if (acth_num - bar_min < item)
            {
                /* Above the bar and the number */
                strcat(buf, "   ");
            }
            else if (acth - bar_min < item)
            {
                /* The number text above the bar */
                if (acth)
                {
                    ansi_puts(fp, buf, (h % 2U) ? '7' : '3');
                    fprintf(fp, "%-3d", (acth + (over >> 1)) / over);
                }
                else if (h >= ntime.tm_hour)
                {
                    /* The datum is still unavailable */
                    ansi_puts(fp, buf, '0');
                    fprintf(fp, "-- ");
                }
                else
                    strcat(buf, "   "); /* No one logged in */
            }
            else
            {
                /* The body or the top of the bar which is not empty */
                const bool full = acth - bar_min >= item * COUNTOF(act_bar);
                ansi_puts(fp, buf, (h % 2U) ? '5' : '1');
                fprintf(fp, "%s ", (full) ? act_bar_full : act_bar[(acth - bar_min) / item]);
            }
        }
        ansi_puts(fp, buf, '\n');
    }

    if (act[25] == 0) act[25]=1; /* Thor.980928: lkchu patch: 防止除數為0 */

    fprintf(fp, "\x1b[1;34m  ╒═══════════════════════════════════╕\x1b[m\n"
        "  \x1b[1;32m0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23\x1b[m\n"
        "\n\t");
    if (over > 1)
        fprintf(fp, "\x1b[1;35m單位：\x1b[37m%d 人", over);
    fprintf(fp, "\t\x1b[1;35m總共上站人次：\x1b[37m%-9d\x1b[35m平均使用時間：\x1b[37m%d\x1b[m\n",
        total, act[24] / act[25] + 1);
    fclose(fp);

    /* --------------------------------------------------- */
    /* Load board shm                                      */
    /* --------------------------------------------------- */

    shm_logger_init(NULL);
    bshm_init(&bshm);

    /* --------------------------------------------------- */
    /* 系統開票                                            */
    /* --------------------------------------------------- */

    closepolls();

    /* --------------------------------------------------- */
    /* 資料壓縮備份、熱門話題統計                          */
    /* --------------------------------------------------- */

    /* Thor.990329: y2k */
    sprintf(ymd, "-%02d%02d%02d",
        ptime.tm_year % 100, ptime.tm_mon + 1, ptime.tm_mday);

    if (ntime.tm_hour == 0)
    {

        sprintf(date, "[ %02d 月 %02d 日] ", ptime.tm_mon + 1, ptime.tm_mday);

        /* 以下是目前沒有在使用的紀錄 */

//        sprintf(title, "[記錄] %s使用次數統計", date);
//        PROC_CMD("/bin/sh", BINARY_SUFFIX"spss.sh");
//        keeplog("run/spss.log", NULL, title, 2);

//        sprintf(title, "[記錄] %s版面閱\讀次數統計", date);
//        PROC_CMD("/bin/sh", BINARY_SUFFIX"brd_usies.sh");
//        keeplog(FN_BRD_USIES".log", NULL, title, 2); /* 整理過後的 log */
//        keeplog(FN_BRD_USIES, BRD_SECRET, title, 2); /* 未整理前的 log */
//        gzip(FN_BRD_USIES, "brdusies/brdusies", ymd);   /* 未整理前的 log */

//        sprintf(title, "[記錄] %s修改文章紀錄", date);
//        keeplog(FN_POSTEDIT_LOG, BRD_SECRET, title, 2);

//        sprintf(title, "[記錄] %sbbsmail mailpost紀錄", date);
//        keeplog(FN_BBSMAILPOST_LOG, BRD_SECRET, title, 2);

//        sprintf(title, "[記錄] %s擋信紀錄", date);
//        keeplog(FN_BANMAIL_LOG, NULL, title, 2);
//        gzip(FN_BANMAIL_LOG, "banmail/banmail", ymd);  /* 擋信紀錄 */

//        sprintf(title, "[記錄] %s寄信紀錄", date);
//        keeplog(FN_MAIL_LOG, BRD_SECRET, title, 2);

//        sprintf(title, "[記錄] %s轉信紀錄", date);
//        keeplog(FN_INNBBS_LOG, BRD_SECRET, title, 2);
//        gzip(FN_INNBBS_LOG, "innbbsd/innbbsd", ymd);  /* 轉信紀錄 */


//        sprintf(title, "[記錄] %s看版信件刪除紀錄", date);
//        keeplog(FN_EXPIRE_LOG, BRD_SECRET, title, 2);
//        gzip(FN_EXPIRE_LOG, "expire/expire", ymd);  /* 看版信件刪除紀錄 */

//        sprintf(title, "%sE-Mail 流量統計", date);
//        keeplog("run/unlog.log", NULL, title, 2);

        /* Thor.990403: 統計OVER */
//        system("grep OVER run/bmta.log | cut -f2 | cut -d' ' -f2- | sort | uniq -c > run/over.log");
//        sprintf(title, "%sE-Mail over max connection 統計", date);
//        keeplog("run/over.log", NULL, title, 2);


        /* 以下是秘密紀錄 */
        sprintf(title, "[記錄] %s文章觀看紀錄", date);
        keeplog(FN_BROWSE_LOG, BRD_SECRET, title, 3);

        sprintf(title, "[記錄] %s匿名板紀錄", date);
        keeplog(FN_ANONYMOUS_LOG, BRD_SECRET, title, 3);

        /*  由於一直處於開檔狀態, 是故不 log by statue
            修改 SIG_USR1 可將 log dump 出來 by visor */
        system("kill -USR1 `ps -auxwww | grep xchatd | awk '{print $2}'`");
        sprintf(title, "[記錄] %s聊天室進出紀錄", date);
        keeplog(FN_CHAT_LOG".old", BRD_SECRET, title, 2);

        sprintf(title, "[記錄] %spop3認證紀錄", date);
        keeplog(FN_VERIFY_LOG, BRD_SECRET, title, 2);

        /* 由於 FN_POP3_LOG 一直處於開檔狀態, 是故先將此 pop3d 關閉
           檔案自然就會寫入, 此時 log 才會有檔案 by statue */
        sprintf(title, "[記錄] %sPOP3D紀錄", date);
        keeplog(FN_POP3_LOG, BRD_SECRET, title, 2);

        sprintf(title, "[記錄] %s重複 E-mail 認證紀錄", date);
        keeplog(FN_EMAILADDR_LOG, BRD_SECRET, title, 2);

        sprintf(title, "[記錄] %s站務行為紀錄", date);
        keeplog(FN_BLACKSU_LOG, BRD_SECRET, title, 2);

        sprintf(title, "[記錄] %s點歌紀錄", date);
        keeplog(FN_SONG_LOG, BRD_SECRET, title, 2);

        sprintf(title, "[記錄] %s推薦文章紀錄", date);
        keeplog(FN_RECOMMEND_LOG, BRD_SECRET, title, 2);

        sprintf(title, "[記錄] %s銀行交易紀錄", date);
        keeplog(FN_BANK, BRD_SECRET, title, 2);

        sprintf(title, "[記錄] %s商店交易紀錄", date);
        keeplog(FN_SHOP, BRD_SECRET, title, 2);

        system("cat run/usies= | grep APPLY > run/apply.log");
        sprintf(title, "[記錄] %s每日註冊使用者紀錄", date);
        keeplog("run/apply.log", BRD_SECRET, title, 2);

        sprintf(title, "[記錄] %s站務列表", date);
        keeplog(FN_MANAGER_LOG, BRD_SECRET, title, 2);

        sprintf(title, "[記錄] %s停權列表", date);
        keeplog(FN_CRIMINAL_LOG, BRD_SECRET, title, 2);

        sprintf(title, "[記錄] %s板主列表", date);
        keeplog(FN_BMLIST_LOG, BRD_SECRET, title, 2);

        sprintf(title, "[記錄] %s權限修改紀錄", date);
        keeplog(FN_SECURITY, BRD_SECRET, title, 2);

        sprintf(title, "[記錄] %s精華區索引紀錄", date);
        keeplog(FN_GINDEX_LOG, BRD_SECRET, title, 2);

        sprintf(title, "[記錄] %s精華區檢查紀錄", date);
        keeplog(FN_GCHECK_LOG, BRD_SECRET, title, 2);

        sprintf(title, "[記錄] %s看板信件手動刪除紀錄", date);
        keeplog(FN_EXPIRED_LOG, BRD_SECRET, title, 2);

        sprintf(title, "[記錄] %s使用者遺失 ACCT 紀錄", date);
        keeplog("run/NOACCT.log", BRD_SECRET, title, 2);

        sprintf(title, "[記錄] %s使用者遺失 .DIR 紀錄", date);
        keeplog("run/NOUSRDIR.log", BRD_SECRET, title, 2);

        sprintf(title, "[記錄] %s看板遺失 .DIR 紀錄", date);
        keeplog("run/NOBRDDIR.log", BRD_SECRET, title, 2);

#ifdef  HAVE_FAVORITE
        sprintf(title, "[記錄] %s我的最愛紀錄", date);
        keeplog(FN_FAVORITE_LOG, BRD_SECRET, title, 2);
#endif

        /* 以下是公開紀錄 */

        //sprintf(date, "[%2d 月 %2d 日] ", ptime.tm_mon + 1, ptime.tm_mday);
        sprintf(title, "[記錄] %s文章篇數統計", date);
        keeplog(FN_POST_LOG, NULL, title, 2);

        sprintf(title, "[記錄] %s小雞金錢紀錄", date);
        keeplog(FN_PIPMONEY_LOG, NULL, title, 2);

        sprintf(title, "[記錄] %s" BOARDNAME "寵物雞", date);
        keeplog(FN_PIP_LOG, NULL, title, 2);

        sprintf(title, "[記錄] %s" BOARDNAME "踩地雷", date);
        keeplog(FN_MINE_LOG, NULL, title, 2);

        sprintf(title, "[記錄] %s酸甜苦辣留言板", date);
        keeplog(FN_NOTE_ALL, NULL, title, 2);

        sprintf(title, "[記錄] %s本日砍帳號紀錄", date);
        keeplog(FN_REAPER_LOG, NULL, title, 2);

        sprintf(title, "[記錄] %s偷懶板主紀錄", date);
        keeplog(FN_LAZYBM_LOG, NULL, title, 2);

        sprintf(title, "[記錄] %s本日十大熱門話題", date);
        keeplog("gem/@/@-day", NULL, title, 0);

        if ((fp = fopen(fn_yesterday, "w")))
        {
            f_suck(fp, fn_today);
            fclose(fp);
        }

        if (ntime.tm_wday == 0)
        {
            sprintf(title, "[記錄] %s本週五十大熱門話題", date);
            keeplog("gem/@/@-week", NULL, title, 0);
        }

        if (ntime.tm_mday == 1)
        {
            sprintf(title, "[記錄] %s本月百大熱門話題", date);
            keeplog("gem/@/@-month", NULL, title, 0);
        }

        if (ntime.tm_yday == 1)
        {
            sprintf(title, "[記錄] %s年度百大熱門話題", date);
            keeplog("gem/@/@-year", NULL, title, 0);
        }

        gzip(log_file, "usies/usies", ymd);

        gzip(FN_CHATDATA_LOG".old", "chat/chat", ymd);

//#ifdef  LOG_BRD_USIES
//      gzip(brd_file, FN_BRD_USIES, ymd);   /* lkchu.981201: 備份看版閱讀記錄 */
//#endif

    }
    else if (ntime.tm_hour == 1)
    {
        sprintf(date, "[%02d 月 %02d 日] ", ptime.tm_mon + 1, ptime.tm_mday);

        sprintf(title, "[記錄] %s使用者編號紀錄", date);
        PROC_CMD(BINARY_SUFFIX"userno", NULL);
        keeplog(FN_USERNO_LOG, BRD_SECRET, title, 2);
        gzip(FN_USERNO_LOG, "userno/userno", ymd);        /* 所有 [使用者編號紀錄] 記錄 */
        gzip(FN_MAIL_LOG, "mail/mail", ymd);    /* 所有 [寄信] 記錄 */


        if (ntime.tm_wday == 6)
        {
//於091205註解
//          gtar("usr/@", "reaper/reaper", ymd, 1);
        }
    }

#ifdef HAVE_SIGNED_MAIL
    srandom(time(NULL));
#if (PRIVATE_KEY_PERIOD == 0)
    if (!dashf(PRIVATE_KEY))
#else
    if (!dashf(PRIVATE_KEY) || (random() % PRIVATE_KEY_PERIOD) == 0)
#endif
    {
        sprintf(title, "log/prikey%s", ymd);
        f_mv(PRIVATE_KEY, title);
        int i = PLAINPASSSIZE-1;
        for (;;)
        {
            const int j = random() % 0x100U;
            if (!j) continue;
            title[--i] = j;
            if (i == 0) break;
        }
        rec_add(PRIVATE_KEY, title, PLAINPASSSIZE-1);
    }
#endif  /* #ifdef HAVE_SIGNED_MAIL */

    exit(0);
}

void
keeplog(
    const char *fnlog,
    const char *board,
    const char *title,
    int mode)           /* 0:load 1: rename  2:unlink 3:mark*/
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

    if (mode == 1 || mode == 3)
    {
        close(fd);
        /* rename(fnlog, fpath); */
        f_mv(fnlog, fpath); /* Thor.990409:可跨partition */
    }
    else
    {
        fp = fdopen(fd, "w");
        fprintf(fp, "作者: SYSOP (" SYSOPNICK ")\n標題: %s\n時間: %s\n",
            title, ctime_any(&hdr.chrono));
        f_suck(fp, fnlog);
        fclose(fp);
        if (mode)
            unlink(fnlog);
    }
    if (mode == 3)
        hdr.xmode |= POST_MARKED;

    strcpy(hdr.title, title);
    strcpy(hdr.owner, "SYSOP");
    strcpy(hdr.nick, SYSOPNICK);
    fd = open(folder, O_WRONLY | O_CREAT | O_APPEND, 0600);
    if (fd < 0)
    {
        unlink(fpath);
        return;
    }
    write(fd, &hdr, sizeof(HDR));
    close(fd);
}

