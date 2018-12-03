/*-------------------------------------------------------*/
/* post.c       ( NTHU CS MapleBBS Ver 2.39 )            */
/*-------------------------------------------------------*/
/* target : bulletin boards' routines                    */
/* create : 95/03/29                                     */
/* update : 2000/01/02                                   */
/*-------------------------------------------------------*/


#define xlog(x)         f_cat("/tmp/b.log", x)

#include "bbs.h"
extern int wordsnum;            /* itoc.010408: 計算文章字數 */
extern int keysnum;
extern FWCACHE *fwshm;
extern BCACHE *bshm;
extern XZ xz[];
#ifdef HAVE_MULTI_CROSSPOST
extern LinkList *ll_head;
#endif

extern int can_message(UTMP *up);
extern int cmpchrono(HDR *hdr);
extern int xo_delete(XO *xo);
extern int xo_uquery_lite(XO *xo);
extern int xo_usetup(XO *xo);
/*extern int xo_fpath(char *fpath, char *dir, HDR *hdr);*/          /* lkchu.981201 */

#ifdef  HAVE_DETECT_CROSSPOST
CHECKSUMCOUNT cksum;
#endif

/* 071002.cat: 偵測張爸文 */
#ifdef HAVE_DETECT_ZHANGBA
static int zhangba_currentsession = 0;

#define ZHANGBA_PATTERNS 5

static char zhangba_patterns[ZHANGBA_PATTERNS][25] = {
    "張振聲",
    "聲仔",
    "taconet.com.tw/jscha",
    "台灣大學",
    "台大"};

    static int
zhangba_detect(
    char *fpath)
{
    char checked[ZHANGBA_PATTERNS+1];
    FILE *fp;
    char buf[256];
    int i, num=0;

    if ((fp = fopen(fpath, "r")))
    {
        while (fgets(buf, sizeof(buf), fp))
            for (i=0;  i < ZHANGBA_PATTERNS; i++)
                if (strstr(buf, zhangba_patterns[i]) && (checked[i] != '1'))
                {
                    checked[i] = '1';
                    num++;
                }
        fclose(fp);
    }
    return num;
}

#endif

extern int TagNum;
extern char xo_pool[];
extern char brd_bits[];

/* Thor.990113: imports for anonymous log */
extern char rusername[];
static char delete_reason[30] = {};

    int
cmpchrono(
    HDR *hdr)
{
    return hdr->chrono == currchrono;
}


    static void
change_stamp(
    char *folder,
    HDR *hdr)
{
    hdr->stamp = time(0);
}


/* ----------------------------------------------------- */
/* 改良 cross post 停權                                  */
/* ----------------------------------------------------- */

#ifdef  HAVE_DETECT_CROSSPOST
    static int
checksum_add(
    char *title)
{
    int sum=0, i, end;
    int *ptr;
    ptr = (int *)title;
    end = strlen(title)/4;
    for (i=0; i<end; i++)
    {
        sum += *ptr++;
    }
    return sum;
}

    static int
checksum_put(
    int sum,
    int check)
{
    int i;
    for (i=0; i<5; i++)
    {
        if (cksum.checksum[i].sum == sum)
        {
            if (check)
                cksum.checksum[i].total--;
            else
                cksum.checksum[i].total++;
            if (cksum.checksum[i].total > MAX_CROSS_POST)
            {
                cksum.post_modetype |= POST_STOP_PERM;
                return 1;
            }
            return 0;
        }
    }
    if (!check)
    {
        if ((++(cksum.checknum))>=5)
            cksum.checknum = 0;
        cksum.checksum[cksum.checknum].sum = sum;
        cksum.checksum[cksum.checknum].total = 1;
    }
    return 0;
}

    int
checksum_find(
    char *fpath,
    int check,
    int state)
{
    char buf[256];
    FILE *fp;
    char *star = "※";

    int sum, i, count=0;

    if ((state & BRD_NOCNTCROSS) || HAS_PERM(PERM_ADMIN) ||
            (cksum.post_modetype & POST_STOP_PERM))
        return 0;

    sum = 0;
    fp = fopen(fpath, "r");
    if (fp)
    {
        for (i=0; count <= MAX_CHECKSUM; i++)
        {
            if (fgets(buf, 256, fp))
            {
                if (i>3)
                {
                    if (*buf != '>' && strncmp(buf, star, 2) && *buf != ':' )
                    {
                        sum+=checksum_add(buf);
                        count++;
                    }
                }
            }
            else
                break;
        }
    }
    fclose(fp);
    return checksum_put(sum, check);
}
#endif

/* ----------------------------------------------------- */
/* 改良 innbbsd 轉出信件、連線砍信之處理程序             */
/* ----------------------------------------------------- */

    void
btime_update(
    int bno)
{
    if (bno >= 0)
        (bshm->bcache + bno)->btime = -1;   /* 讓 class_item() 更新用 */
}

    void
outgo_post(
    HDR *hdr,
    char *board)
{
    bntp_t bntp;

    memset(&bntp, 0, sizeof(bntp_t));

    if (board)
    {
        bntp.chrono = hdr->chrono;
    }else
    {
        bntp.chrono = -1;
        board=currboard;
    }

    strcpy(bntp.board, board);
    strcpy(bntp.xname, hdr->xname);
    strcpy(bntp.owner, hdr->owner);
    strcpy(bntp.nick, hdr->nick);
    strcpy(bntp.title, hdr->title);
    rec_add("innd/out.bntp", &bntp, sizeof(bntp_t));
    /*
    char *fpath, buf[256];

    if (board)
    {
        fpath = "innd/out.bntp";
    }
    else
    {
        board = currboard;
        fpath = "innd/cancel.bntp";
    }

    sprintf(buf, "%s\t%s\t%s\t%s\t%s\n",
            board, hdr->xname, hdr->owner, hdr->nick, hdr->title);
    f_cat(fpath, buf);
    */
}


    void
cancel_post(
    HDR *hdr)
{
    if ((hdr->xmode & POST_OUTGO) &&    /* 外轉信件 */
            (hdr->chrono > ap_start - 7 * 86400))       /* 7 天之內有效 */
    {
        outgo_post(hdr, NULL);
    }
}


/*static inline void*/

    void
move_post(      /* 將 hdr 從 currboard 搬到 board */
    HDR *hdr,
    char *board,
    int by_bm)
{
    HDR post;
    char folder[80], fpath[80];

    brd_fpath(folder, currboard, fn_dir);
    hdr_fpath(fpath, folder, hdr);

    brd_fpath(folder, board, fn_dir);
    hdr_stamp(folder, HDR_LINK | 'A', &post, fpath);
    /*unlink(fpath);*/

    /* 直接複製 trailing data */

    memcpy(post.owner, hdr->owner, TTLEN + 140);
    if (by_bm == -1)
        strcpy(post.owner, cuser.userid);
    if (by_bm == -2)
        post.xmode |= POST_MARKED;

    if (by_bm>0)
        sprintf(post.title, "%-13s%.59s", cuser.userid, hdr->title);

    rec_bot(folder, &post, sizeof(post));
    if (by_bm>=0)
        cancel_post(hdr);
}


/* ----------------------------------------------------- */
/* 發表、回應、編輯、轉錄文章                            */
/* ----------------------------------------------------- */

#ifdef HAVE_ANONYMOUS
/* Thor.980727: lkchu patch: log anonymous post */
/* Thor.980909: gc patch: log anonymous post filename */
    void
log_anonymous(
    char *fname)
{
    char buf[512];
    time_t now = time(0);
    /* Thor.990113: 加上 rusername 和 fromhost比較詳盡 */
    sprintf(buf, "%s %-13s(%s@%s) %s %s %s\n", Etime(&now), cuser.userid, rusername, fromhost, currboard, ve_title, fname);
    f_cat(FN_ANONYMOUS_LOG, buf);
}
#endif

#ifdef  HAVE_DETECT_VIOLATELAW
    int
seek_log(
    char *title,
    int state)
{
    BANMAIL *head, *tail;
    if (state & BRD_NOLOG)
        return 0;


    head = fwshm->fwcache;
    tail = head + fwshm->number;

    while (fwshm->number && head<tail)
    {
        if (strstr(title, head->data))
            return 1;
        head++;
    }
    return 0;
}
#endif

    static int
do_post(
    char *title)
{
    /* Thor.1105: 進入前需設好 curredit */
    HDR post;
    char fpath[80], folder[80], *nick, *rcpt, buf[50];
    int mode, bno = -1;
    BRD *brd;
    time_t spendtime;

    bno = brd_bno(currboard);
    brd = bshm->bcache + bno;

#ifdef  HAVE_DETECT_VIOLATELAW
    int banpost;
#endif
#ifdef  HAVE_DETECT_CROSSPOST
    int crosspost;
#endif

#ifdef  HAVE_RESIST_WATER
    if (checkqt > CHECK_QUOT_MAX)
    {
        vmsg("您已經灌太多水了，請下次再來吧！");
        return XO_FOOT;
    }
#endif

    if (bbsothermode & OTHERSTAT_EDITING)
    {
        vmsg("你還有檔案還沒編完哦！");
        return XO_FOOT;
    }

    /* cache.090519: 板主自訂看板發文權限 */
    /* cache.090928: 看板唯讀, r2.170912: 調整敘述 */

    if (brd->battr & BRD_NOREPLY)
    {
        if (!HAS_PERM(PERM_ADMIN))
        {
            vmsg("對不起，本看板目前禁止發表文章。");
            return XO_FOOT;
        }
        else
            vmsg("請注意，本看板目前是唯讀狀態。");
    }

    if (!(bbstate & STAT_POST))
    {
        vmsg("對不起，您的權限不足。");
        return XO_FOOT;
    }
    else if (brd->battr & BRD_THRESHOLD)
    {
        THRESHOLD th;
        char msg[80];

        brd_fpath(fpath, currboard, FN_THRESHOLD);
        if (!rec_get(fpath, &th, sizeof(THRESHOLD), 0))
        {
            if (cuser.lastlogin - cuser.firstlogin < th.age * 86400)
            {
                sprintf(msg, "註冊時間 %d 天以上，方可在此看板發表文章", th.age);
                vmsg(msg);
                return XO_FOOT;
            }
            if (cuser.numlogins < th.numlogins)
            {
                sprintf(msg, "上站次數 %d 次以上，方可在此看板發表文章", th.numlogins);

                vmsg(msg);
                return XO_FOOT;
            }
            if (cuser.numposts < th.numposts)
            {
                sprintf(msg, "發表文章 %d 篇以上，方可在此看板發表文章", th.numposts);
                vmsg(msg);
                return XO_FOOT;
            }
            if ((th.point2 != 0 ) && (cuser.point2 >= th.point2))
            {
                sprintf(msg, "劣文 %d 篇(含)以下，方可在此看板發表文章", th.point2);
                vmsg(msg);
                return XO_FOOT;
            }
        }
    }

    brd_fpath(fpath, currboard, "post");

    if (more(fpath, (char *)-1)==-1)
        film_out(FILM_POST, 0);

    move(20, 0);
    prints("發表文章於【 %s 】看板", currboard);

    if (!ve_subject(21, title, NULL))
        return XO_HEAD;

    /* 未具備 Internet 權限者，只能在站內發表文章 */
    /* Thor.990111: 沒轉信出去的看版, 也只能在站內發表文章 */

    if (!HAS_PERM(PERM_INTERNET) || (bbstate & BRD_NOTRAN))
        curredit &= ~EDIT_OUTGO;

#ifdef HAVE_ANONYMOUS
    /* Thor.980727: lkchu新增之[簡單的選擇性匿名功能] */
    /* Thor.980909: gc patch: edit 時匿名不需簽名檔 */
    if (bbstate & BRD_ANONYMOUS)
    {
        if (cuser.ufo2 & UFO2_DEF_ANONY)
        {
            if (vans("你(妳)想要匿名嗎(Y/N)?[N]") == 'y')
                curredit |= EDIT_ANONYMOUS;
        }
        else if (vans("你(妳)想要匿名嗎(Y/N)?[Y]") != 'n')
            curredit |= EDIT_ANONYMOUS;
    }
#endif

    utmp_mode(M_POST);
    fpath[0] = 0;
    time(&spendtime);

    if (vedit(fpath, YEA) < 0)
    {
        unlink(fpath);
        pmsg2("取消");
        return XO_HEAD;
    }

    spendtime = time(0) - spendtime;    /* itoc.010712: 總共花的時間(秒數) */

    //bno = brd_bno(currboard);
    brd = bshm->bcache + currbno;
    brh_get(brd->bstamp, bno);

    /* build filename */

    brd_fpath(folder, currboard, fn_dir);
    do  /* cat.20050729 黑洞問題 @@ 黑洞經常沒有日期 */
    {
        hdr_stamp(folder, HDR_LINK | 'A', &post, fpath);
    } while (strlen(post.date) != 8);

#ifdef  HAVE_DETECT_VIOLATELAW
    banpost = seek_log(ve_title, bbstate);
#endif
#ifdef  HAVE_DETECT_CROSSPOST
    crosspost = checksum_find(fpath, 0, bbstate);
#endif

    /* set owner to anonymous for anonymous board */

    rcpt = cuser.userid;
    nick = cuser.username;
    mode = curredit & POST_OUTGO;
    title = ve_title;

#ifdef HAVE_ANONYMOUS
    /* Thor.980727: lkchu新增之[簡單的選擇性匿名功能] */
    if (curredit & EDIT_ANONYMOUS)
    {
        /* nick = rcpt; */ /* lkchu: nick 不能為  userid */
        nick = "猜猜我是誰 ? ^o^";
        /* Thor.990113: 怕與現存id混淆 */
        /* rcpt = "anonymous"; */
        rcpt = "不告訴你^^b";
        mode = 0;

        /* Thor.980727: lkchu patch: log anonymous post */
        /* Thor.980909: gc patch: log anonymous post filename */
        log_anonymous(post.xname);

    }
#endif

    post.xmode = mode;

    if (curredit & EDIT_RESTRICT)
        post.xmode |= POST_LOCK;

    strcpy(post.owner, rcpt);
    strcpy(post.nick, nick);
    strcpy(post.title, title);

    if (brd->battr & BRD_PRH)
        strcpy(post.lastrecommend, "$");

    post.modifytimes = 0;
    post.pushtime = 0;
    post.recommend = 0;
    strcpy(post.lastrecommend, cuser.userid);

#ifdef  HAVE_DETECT_CROSSPOST
    if (crosspost)
    {
        move_post(&post, BRD_VIOLATELAW, -2);
        add_deny(&cuser, DENY_SEL_POST|DENY_DAYS_1|DENY_MODE_POST, 0);
        deny_log_email(cuser.vmail, (cuser.userlevel & PERM_DENYSTOP) ? -1 : cuser.deny);
        bbstate &= ~STAT_POST;
        cuser.userlevel &= ~PERM_POST;
    }
#endif

#ifdef  HAVE_DETECT_ZHANGBA
    if (zhangba_detect(fpath) >= 3)
        zhangba_currentsession++;

    if (zhangba_currentsession > 5)
    {
        move_post(&post, BRD_VIOLATELAW, -2);
        add_deny(&cuser, DENY_SEL_POST|DENY_DAYS_5|DENY_MODE_POST, 0);
        deny_log_email(cuser.vmail, (cuser.userlevel & PERM_DENYSTOP) ? -1 : cuser.deny);
        bbstate &= ~STAT_POST;
        cuser.userlevel &= ~PERM_POST;
    }
#endif

#ifdef  HAVE_DETECT_VIOLATELAW
    if (banpost)
    {
        move_post(&post, BRD_BANPOSTLOG, -1);
    }
#endif

#ifdef  HAVE_OBSERVE_LIST
    if (observeshm_find(cuser.userno))
    {
        move_post(&post, BRD_OBSERVE, -1);
    }
#endif

    move_post(&post, BRD_LOCALPOSTS, -3);

    if (!rec_bot(folder, &post, sizeof(HDR)))
    {
        /* if ((mode) && (!(bbstate & BRD_NOTRAN))) */
        /* Thor.990111: 已由 edit.c 中統一check */
        brh_add( post.chrono, post.chrono,  post.chrono);
        //post_history(xz[XZ_POST - XO_ZONE].xo, &post);
#ifdef  HAVE_DETECT_VIOLATELAW
        if (mode && !banpost)
#else
            if (mode)
#endif
                outgo_post(&post, currboard);

        clear();
        //outs("順利貼出佈告，");

        if (bbstate & BRD_NOCOUNT || (/*BMIN(wordsnum, */ keysnum/*)*/) < 30)
        {
            pmsg2("文章不列入紀錄，敬請包涵。");
        }
        else
        {
          if (spendtime*3 < keysnum)
          {
            pmsg2("文章不列入紀錄，敬請包涵。");
          }
          else if (spendtime < 30)
          {
            sprintf(buf, "這是您的第 %d 篇文章。", ++cuser.numposts);
            pmsg2(buf);
            brd->blast = time(0);
          }
          else
          {
            mode = BMIN(keysnum, spendtime) / 10;       /* 每十字/秒 一元 */
            sprintf(buf, "這是您的第 %d 篇文章，獲得 %d 夢幣。", ++cuser.numposts, mode);
            pmsg2(buf);
            brd->blast = time(0);
            addmoney(mode, cuser.userid);
          }
        }

        /* 回應到原作者信箱 */

        if (curredit & EDIT_BOTH)
        {
            char *msg = "作者無法收信";
#define _MSG_OK_        "回應至作者信箱"

            rcpt = quote_user;
            if (strchr(rcpt, '@'))
            {
                if (bsmtp(fpath, title, rcpt, 0) >= 0)
                    msg = _MSG_OK_;
            }
            else
            {
                usr_fpath(folder, rcpt, fn_dir);
                if (hdr_stamp(folder, HDR_LINK, &post, fpath) == 0)
                {
                    strcpy(post.owner, cuser.userid);
                    strcpy(post.title, title);
                    if (!rec_bot(folder, &post, sizeof(post)))
                        msg = _MSG_OK_;
                }
            }
            outs(msg);
        }
    }
    unlink(fpath);

    //vmsg(NULL);
#ifdef  HAVE_DETECT_CROSSPOST
    if (crosspost)
        remove_perm();
#endif

#ifdef  HAVE_COUNT_BOARD
    //  if (!(strcmp(brd->brdname, "Test")))
    if (!(bbstate & BRD_NOTOTAL))
        brd->n_posts++;
#endif


#ifdef  HAVE_RESIST_WATER
    if (checkqt > CHECK_QUOT_MAX && !HAS_PERM(PERM_ADMIN))
    {
        remove_perm();
        vmsg("您已經灌太多水了，請下次再來吧！");
    }
#endif

    return XO_INIT;
}


    static int
do_reply(
    HDR *hdr)
{
    char *msg;

    curredit = 0;
    if ((bbstate & BRD_NOREPLY) && !HAS_PERM(PERM_SYSOP))
        msg = "▲ 回應至 (M)作者信箱 (Q)取消？[Q] ";
    else
        msg = "▲ 回應至 (F)看板 (M)作者信箱 (B)二者皆是 (Q)取消？[F] ";


    switch (vans(msg))
    {
        case 'm':
            mail_reply(hdr);
            *quote_file = '\0';
            return XO_HEAD;

        case 'q':
            /*
             * Thor: 解決 Gao 發現的 bug.. 先假裝 reply文章，卻按 Q取消， 然後去
             * Admin/Xfile下隨便選一個編輯， 你就會發現跑出 reply文章時的選項了。
             */
            *quote_file = '\0';
            return XO_FOOT;

        case 'b':
            if ((bbstate & BRD_NOREPLY) && !HAS_PERM(PERM_SYSOP))
            {
                *quote_file = '\0';
                return XO_FOOT;
            }
            curredit = EDIT_BOTH;
            break;

        case 'F': case 'f':
        default:
            if ((bbstate & BRD_NOREPLY) && !HAS_PERM(PERM_SYSOP))
            {
                *quote_file = '\0';
                return XO_FOOT;
            }
    }

    /*
     * Thor.1105: 不論是轉進的, 或是要轉出的, 都是別站可看到的,
     * 所以回信也都應該轉出
     */
    if (hdr->xmode & (POST_INCOME | POST_OUTGO))
        curredit |= POST_OUTGO;

    strcpy(quote_user, hdr->owner);
    strcpy(quote_nick, hdr->nick);
    return do_post(hdr->title);
}


    static int
post_reply(
    XO *xo)
{
    if (bbstate & STAT_POST)
    {
        HDR *hdr;

        hdr = (HDR *) xo_pool + (xo->pos - xo->top);
        if (!(hdr->xmode & (POST_CANCEL | POST_DELETE | POST_MDELETE | POST_CURMODIFY)))
        {
            if ((hdr->xmode & POST_LOCK) && !(HAS_PERM(PERM_SYSOP| PERM_BOARD) || bbstate & STAT_BOARD))
                return XO_NONE;
            hdr_fpath(quote_file, xo->dir, hdr);
            return do_reply(hdr);
        }
    }
    return XO_NONE;
}


/* ----------------------------------------------------- */
/* 看板功能表                                            */
/* ----------------------------------------------------- */

#ifdef HAVE_MODERATED_BOARD
extern int XoBM(XO *xo);
#endif


/* ----------------------------------------------------- */


static int post_add(XO *xo);
static int post_body(XO *xo);
static int post_head(XO *xo);           /* Thor: 因為 XoBM 要用 */


#ifdef XZ_XPOST
static int XoXpost(XO *xo);             /* Thor: for XoXpost */
#endif


    static int
post_init(
    XO *xo)
{
    xo_load(xo, sizeof(HDR));
    return post_head(xo);
}

    static int          /* cat@20050628 search record in .DIR file */
seek_rec(
    XO *xo,
    HDR *hdr)
{
    int fd, total, pos;
    struct stat st;
    HDR thdr;

    if ((fd = open(xo->dir, O_RDWR, 0600)) == -1)
        return -1;

    pos = xo->pos;

    fstat(fd, &st);

    total = st.st_size / sizeof(HDR);
    if (pos >= total)
        pos = total-1;

    memcpy(&thdr, hdr, sizeof(HDR));

    f_exlock(fd);

    while (pos >= -1)
    {
        lseek(fd, (off_t) (sizeof(HDR) * pos--), SEEK_SET);
        read(fd, hdr, sizeof(HDR));
        if (hdr->chrono == thdr.chrono)
            break;
    }

    f_unlock(fd);
    close(fd);

    return pos+1;

}

    static int
post_load(
    XO *xo)
{
    xo_load(xo, sizeof(HDR));
    return post_body(xo);
}


    static int
post_attr(
    HDR *fhdr)
{
    int mode, attr;

    mode = fhdr->xmode;

    if (mode & POST_CANCEL)
        return 'c';

    if (mode & POST_DELETE)
        return 'd';

    if (mode & POST_MDELETE)
        return 'D';

    if (mode & POST_EXPIRE)
        return (brh_unread(BMAX(fhdr->chrono, fhdr->stamp)) ? 0 : 0x20 ) | 'E';

    if (mode & POST_LOCK)
        return 'L';

    if (mode & POST_COMPLETE)
        return (brh_unread(BMAX(fhdr->chrono, fhdr->stamp)) ? 0 : 0x20 ) | 'S';

    attr = brh_unread(BMAX(fhdr->chrono, fhdr->stamp)) ? 0 : 0x20;
    //attr = brh_unread(fhdr->chrono) ? 0 : 0x20;
    if (fhdr->pushtime)
      attr = brh_unread(fhdr->pushtime) ? 0 : 0x20;
    mode &= (bbstate & STAT_BOARD) ? ~0 : ~POST_GEM;    /* Thor:一般user看不到G */

    if (mode &= (POST_MARKED | POST_GEM))
        attr |= (mode == POST_MARKED ? 'M' : (mode == POST_GEM ? 'G' : 'B'));
    else if (!attr)
        attr = '+';
    return attr;
}

    static void
post_item(
    int num,
    HDR *hdr)
{
#ifdef HAVE_RECOMMEND

    if (hdr->xmode & POST_BOTTOM)
    {
        /* 由於置底文沒有閱讀記錄，所以弄成已讀 */
        char attr = post_attr(hdr);
        if (attr == '+')
            attr = ' ';
        else if (attr == 'M')
            attr |= 0x20;
        prints("  \033[1;33m  ★\033[m%c%s%c%s", tag_char(hdr->chrono), hdr->xmode & POST_MARKED ? "\033[1;36m" : "", attr, hdr->xmode & POST_MARKED ? "\033[m" : "");
    }
    else
        prints("%6d%c%s%c%s", num, tag_char(hdr->chrono),
                hdr->xmode & POST_MARKED ? "\033[1;36m" : "", post_attr(hdr),
                hdr->xmode & POST_MARKED ? "\033[m" : "");

    /* 考量到舊版本相容性先註解掉, 加此判斷可以快一點點 */
    if (/*hdr->xmode & POST_RECOMMEND &&*/ !(hdr->xmode & POST_BOTTOM) && !(cuser.ufo2 & UFO2_PRH))
    {
        num = hdr->recommend;

        if (num>0)
        {
            if (num > 120)                    /* 推爆 */
                prints("\033[1;33m爆\033[m");
            else if (num > 99)                /* 推爆 */
                prints("\033[1;31m爆\033[m");
            else if (num > 4)
                prints("\033[1;31m%02d\033[m", num);
            else
                prints("\033[1;31m%02d\033[m", num);
        }
        else if (num<0)
        {
            if (num < -120)              /* 推爛 */
                prints("\033[1;30m弱\033[m");
            else if (num < -99)               /* 推爛 */
                prints("\033[1;32m嫩\033[m");
            else if (num < -5)
                prints("\033[32m%02d\033[m", -num);
            else
                prints("\033[32m%02d\033[m", -num);
        }
        else
            prints("  ");
    }
    else
    {
        outs("  ");
    }

    hdr_outs(hdr, 47);   /* 少一格來放分數 */
#else
    prints("%6d%c%c ", (hdr->xmode & POST_BOTTOM) ? -1 : num, tag_char(hdr->chrono), post_attr(hdr));
    hdr_outs(hdr, 47);
#endif
}

    static int
post_body(
    XO *xo)
{
    HDR *fhdr;
    int num, max, tail;

    max = xo->max;
    if (max <= 0)
    {
        if (bbstate & STAT_POST)
        {
            if (vans("要新增資料嗎？(Y/N) [N] ") == 'y')
                return post_add(xo);
        }
        else
        {
            vmsg("本看板尚無文章");
        }
        return XO_QUIT;
    }

    fhdr = (HDR *) xo_pool;
    num = xo->top;
    tail = num + XO_TALL;
    if (max > tail)
        max = tail;

    move(3, 0);
    do
    {
        post_item(++num, fhdr++);
    } while (num < max);

    clrtobot();
    return XO_NONE;
}


    static int                  /* Thor: 因為 XoBM 要用 */
post_head(
    XO *xo)
{
    vs_head(currBM, xo->xyz);
    outs(NECKPOST);
    return post_body(xo);
}


    static int
post_visit(
    XO *xo)
{
    int ans, row, max;
    HDR *fhdr;

    ans = vans("設定所有文章 (U)未讀 (V)已讀 (Q)取消？ [Q] ");
    if (ans == 'v' || ans == 'u')
    {
        brh_visit(ans = ans == 'u');

        row = xo->top;
        max = xo->max - xo->top + 3;
        if (max > b_lines)
            max = b_lines;

        fhdr = (HDR *) xo_pool;
        row = 3;

        do
        {
            move(row, 7);
            outc(post_attr(fhdr++));
        } while (++row < max);
    }
    return XO_FOOT;
}


    int
getsubject(
    int row,
    int reply)
{
    char *title;

    title = ve_title;

    if (reply)
    {
        char *str;

        str = currtitle;
        if (STR4(str) == STR4(STR_REPLY)) /* Thor.980914: 有比較快點嗎? */
        {
            strcpy(title, str);
        }
        else
        {
            sprintf(title, STR_REPLY "%s", str);
            title[TTLEN] = '\0';
        }
    }
    else
    {
        *title = '\0';
    }

    return vget(row, 0, "標題：", title, TTLEN + 1, GCARRY);
}


    static int
post_add(
    XO *xo)
{
    int cmd;

    curredit = EDIT_OUTGO;
    cmd = do_post(NULL);
    return cmd;
}


    int
post_cross(
    XO *xo)
{
    char xboard[20], fpath[80], xfolder[80], xtitle[80], buf[80], *dir;
    HDR *hdr, xpost, xhdr;
    int method, rc, tag, locus, battr;
    FILE *xfp;
#ifdef  HAVE_DETECT_CROSSPOST
    HDR bhdr;
#endif
#ifdef  HAVE_CROSSPOSTLOG
    char cbuf[128];
    HDR chdr;
#endif

    if (!cuser.userlevel)
        return XO_NONE;

    /* lkchu.990428: mat patch 當看板尚未選定時，修正cross post會斷線的問題 */
    if (bbsmode == M_READA)
    {
        battr = (bshm->bcache + brd_bno(currboard))->battr;
        if (!HAS_PERM(PERM_SYSOP) && (battr & BRD_NOFORWARD))
        {
            outz("★ 此板文章不可轉錄");
            return -1;
        }
    }

    // check delete or not .. by statue 2000/05/18
    hdr = (HDR *) xo_pool + (xo->pos - xo->top);
    if (hdr->xmode & (POST_DELETE | POST_CANCEL | POST_MDELETE | POST_CURMODIFY))
        return XO_NONE;
    if ((hdr->xmode & POST_LOCK) && !HAS_PERM(PERM_SYSOP))
    {
        vmsg("Access Deny!");
        return XO_NONE;
    }

    /* verit 021113 : 解決在 po 文章然後用 ctrl+u 然後換到看板去轉錄的重複標題問題 */
    if (bbsothermode & OTHERSTAT_EDITING)
    {
        vmsg("你還有檔案還沒編完哦！");
        return XO_FOOT;
    }


    /* lkchu.981201: 整批轉錄 */
    tag = AskTag("轉錄");

    if (tag < 0)
        return XO_FOOT;

    if (ask_board(xboard, BRD_W_BIT,
                "\n\n\x1b[1;33m請挑選適當的看板，相同文章切勿超過限制數目。\x1b[m\n\n")
            && (*xboard || xo->dir[0] == 'u'))  /* 信箱中可以轉貼至currboard */
    {
        if (*xboard == 0)
            strcpy(xboard, currboard);

        hdr = tag ? &xhdr : (HDR *) xo_pool + (xo->pos - xo->top);
        /* lkchu.981201: 整批轉貼 */
        if (!tag && (hdr->xmode & POST_LOCK) && !HAS_PERM(PERM_SYSOP))
        {
            vmsg("此文章禁止轉錄！");
            return XO_HEAD;
        }

        method = 1;
        if ((HAS_PERM(PERM_ALLBOARD) || !strcmp(hdr->owner, cuser.userid)) &&
                (vget(2, 0, "(1)原文轉載 (2)轉錄文章？[1] ", buf, 3, DOECHO) != '2'))
        {
            method = 0;
        }

        if (!tag)   /* lkchu.981201: 整批轉錄就不要一一詢問 */
        {
            if (method)
                sprintf(xtitle, "[轉錄]%.66s", hdr->title);
            else
                strcpy(xtitle, hdr->title);

            if (!vget(2, 0, "標題：", xtitle, TTLEN + 1, GCARRY))
                return XO_HEAD;
        }

        rc = vget(2, 0, "(S)存檔 (L)站內 (Q)取消？[Q] ", buf, 3, LCECHO);
        if (rc != 'l' && rc != 's')
            return XO_HEAD;

        locus = 0;
        dir = xo->dir;

        battr = (bshm->bcache + brd_bno(xboard))->battr;

        do      /* lkchu.981201: 整批轉貼 */
        {
            if (tag)
            {
                EnumTagHdr(hdr, dir, locus++);

                if (method)
                    sprintf(xtitle, "[轉錄]%.66s", hdr->title);
                else
                    strcpy(xtitle, hdr->title);
            }

            /* verit 2002.04.04 : 整批轉錄時, 檢查 tag 那篇是否被刪除或取消過 */
            if (hdr->xmode & (POST_DELETE | POST_CANCEL | POST_MDELETE | POST_CURMODIFY))
                continue;

            /* if (rc == 'l' || rc == 's') */
            /* lkchu.981201: 能執行到這表示 rc 為 's' or 'l' */
            if (!((hdr->xmode & POST_LOCK) && !HAS_PERM(PERM_SYSOP)))
            {
                /* hdr_fpath(fpath, xo->dir, hdr); */
                xo_fpath(fpath, dir, hdr);      /* lkchu.981201 */
                brd_fpath(xfolder, xboard, fn_dir);

                if (method)
                {
                    method = hdr_stamp(xfolder, 'A', &xpost, buf);
                    xfp = fdopen(method, "w");

                    strcpy(ve_title, xtitle);
                    strcpy(buf, currboard);
                    strcpy(currboard, xboard);

                    ve_header(xfp);

                    strcpy(currboard, buf);

                    if (hdr->xname[0] == '@')
                        sprintf(buf, "%s] 信箱", cuser.userid);
                    else
                        strcat(buf, "] 看板");
                    fprintf(xfp, "※ 本文轉錄自 [%s\n\n", buf);

                    f_suck(xfp, fpath);
                    /* ve_sign(xfp); */
                    fclose(xfp);
                    close(method);

                    strcpy(xpost.owner, cuser.userid);
                    /* if (rc == 's') */
                    strcpy(xpost.nick, cuser.username);
                }
                else
                {
                    hdr_stamp(xfolder, HDR_COPY | 'A', &xpost, fpath);
                    memcpy(xpost.owner, hdr->owner,
                            sizeof(xpost.owner) + sizeof(xpost.nick));
                    memcpy(xpost.date, hdr->date, sizeof(xpost.date));
                    /* lkchu.981201: 原文轉載保留原日期 */
                }

                /* Thor.981205: 借用 method 存放看版屬性 */
                /* method = (bshm->bcache + brd_bno(xboard))->battr; */

                /* Thor.990111: 在可以轉出前, 要check user有沒有轉出的權力? */
                if (!HAS_PERM(PERM_INTERNET) || (/* method */ battr & BRD_NOTRAN))
                    rc = 'l';

                strcpy(xpost.title, xtitle);

                if (rc == 's' && (!(battr & BRD_NOTRAN)))
                    xpost.xmode = POST_OUTGO;

#ifdef  HAVE_DETECT_CROSSPOST
                memcpy(&bhdr, hdr, sizeof(HDR));
                strcpy(bhdr.owner, cuser.userid);
                if (checksum_find(fpath, 0, battr))
                {
                    move_post(&bhdr, BRD_VIOLATELAW, -2);

                    add_deny(&cuser, DENY_SEL_POST|DENY_DAYS_1|DENY_MODE_POST, 0);
                    deny_log_email(cuser.vmail, (cuser.userlevel & PERM_DENYSTOP) ? -1 : cuser.deny);
                    bbstate &= ~STAT_POST;
                    cuser.userlevel &= ~PERM_POST;

                    board_main();
                }
#endif
#ifdef  HAVE_CROSSPOSTLOG
                if (hdr->xname[0] != '@')
                {
                    memcpy(&chdr, hdr, sizeof(HDR));
                    strcpy(chdr.owner, cuser.userid);
                    sprintf(cbuf, "(%s) %s", xboard, hdr->title);
                    strncpy(chdr.title, cbuf, TTLEN);
                    move_post(&chdr, BRD_CROSSPOST, -3);
                }
#endif

                rec_bot(xfolder, &xpost, sizeof(xpost));
#ifdef  HAVE_DETECT_VIOLATELAW
                if (rc == 's' && (!(battr & BRD_NOTRAN)) && (!(seek_log(xpost.title, battr))))
#else
                    if (rc == 's' && !(battr & BRD_NOTRAN))
#endif
                        outgo_post(&xpost, xboard);
            }
        } while (locus < tag);

        if (!HAS_PERM(PERM_ADMIN))
        {
            time_t now;
            struct tm *ptime;
            char add[180], tgt[30];

            time(&now);
            ptime = localtime(&now);
            sprintf(tgt, "轉錄至 %s 看板", xboard);
            xfp = fopen(fpath, "a");
            sprintf(add, "\x1b[1;33m→ %12s：\x1b[36m%-54.54s \x1b[m%5.5s\n", cuser.userid, tgt, Btime(&hdr->pushtime)+3);
            fprintf(xfp, "%s", add);
            fclose(xfp);
        }

        /* Thor.981205: check 被轉的版有沒有列入紀錄? */
        if (/* method */ battr & BRD_NOCOUNT)
        {
            outs("轉錄完成，文章不列入紀錄，敬請包涵。");
        }
        else
        {
            /* cuser.numposts++; */
            cuser.numposts += (tag == 0) ? 1 : tag; /* lkchu.981201: 要算 tag */
            vmsg("轉錄完成");
        }
    }
    return XO_HEAD;
}

#ifdef HAVE_MULTI_CROSSPOST
    static int
post_xcross(
    XO *xo)
{
    char *xboard, fpath[80], xfolder[80], buf[80], *dir;
    HDR *hdr, xpost, xhdr;
    int tag, locus, listing, do_expire;
    LinkList *wp;

    if (!HAS_PERM(PERM_ALLBOARD))
        return XO_NONE;

    // check delete or not .. by statue 2000/05/18
    hdr = (HDR *) xo_pool + (xo->pos - xo->top);
    if (hdr->xmode & (POST_DELETE | POST_CANCEL | POST_MDELETE | POST_CURMODIFY))
        return XO_NONE;

    tag = AskTag("群組轉貼");

    if (tag < 0)
        return XO_FOOT;

    ll_new();
    listing = brd_list(0);
    do_expire = 0;

    if (listing)
    {

        hdr = tag ? &xhdr : hdr;

        vget(2, 0, "設定刪除嗎 ?? (Y)要 (N)不要？[Y] ", buf, 3, LCECHO);
        if (*buf != 'n')
            do_expire = time(0) + 86400 * 7;

        vget(2, 0, "(Y)確定 (Q)取消？[Q] ", buf, 3, LCECHO);
        if (*buf != 'y')
            return XO_HEAD;

        dir = xo->dir;

        wp = ll_head;

        do
        {
            locus = 0;
            xboard = wp->data;
            do
            {
                if (tag)
                {
                    EnumTagHdr(hdr, dir, locus++);
                }
                if (hdr->xmode & (POST_DELETE | POST_CANCEL | POST_MDELETE | POST_CURMODIFY))
                    continue;

                xo_fpath(fpath, dir, hdr);
                brd_fpath(xfolder, xboard, fn_dir);

                hdr_stamp(xfolder, HDR_LINK | 'A', &xpost, fpath);
                memcpy(xpost.owner, hdr->owner,
                        sizeof(xpost.owner) + sizeof(xpost.nick));
                memcpy(xpost.date, hdr->date, sizeof(xpost.date));

                strcpy(xpost.title, hdr->title);
                xpost.xmode |= POST_COMPLETE;
                if (do_expire > 0)
                {
                    xpost.expire = do_expire;
                    xpost.xmode |= POST_EXPIRE;
                }

                rec_bot(xfolder, &xpost, sizeof(xpost));
            } while (locus < tag);
        } while ((wp = wp->next));
    }
    return XO_HEAD;
}

#endif


/* ----------------------------------------------------- */
/* 資料之瀏覽：edit / title                              */
/* ----------------------------------------------------- */

void
post_history(
    XO *xo,
    HDR *fhdr)
{
    int prev, chrono, next, pos, top, push=0;
    char *dir;
    HDR buf;

#ifdef  HAVE_BOTTOM
    if (fhdr->xmode & POST_BOTTOM && fhdr->xmode & POST_COMPLETE)
        return;
#endif


    dir = xo->dir;
    pos = xo->pos;
    top = xo->top;

    chrono = fhdr->chrono;
    push = fhdr->pushtime;


    if (brh_unread(push))
        brh_add(push, push, push);

    if (!brh_unread(chrono))
        //if ( !brh_unread(push))
            return;

    if (--pos >= top)
    {
        prev = fhdr[-1].chrono;
    }
    else
    {
        if (!rec_get(dir, &buf, sizeof(HDR), pos))
                prev = buf.chrono;
        else
            prev = chrono;
    }

    pos +=2;
    if (pos < top + XO_TALL)
            next = fhdr[1].chrono;
    else
    {
        if (!rec_get(dir, &buf, sizeof(HDR), pos))
            next = buf.chrono;
        else
            next = chrono;
    }
/*
    if (push)
        prev = chrono = next = push;
*/
    brh_add(prev, chrono, next);

}

#if 0
    void
post_history(          /* 將 hdr 這篇加入 brh */
    XO *xo,
    HDR *hdr)
{
    int fd;
    time_t prev, chrono, next, this;
    HDR buf;


    if (hdr->xmode & POST_BOTTOM)     /* 置底文不加入閱讀記錄 */
        return;

    chrono = BMAX(hdr->chrono, hdr->stamp);

    if (!brh_unread(chrono))      /* 如果已在 brh 中，就無需動作 */
        return;

    if ((fd = open(xo->dir, O_RDONLY)) >= 0)
    {
        prev = chrono + 1;
        next = chrono - 1;

        while (read(fd, &buf, sizeof(HDR)) == sizeof(HDR))
        {
            this = BMAX(buf.chrono, buf.stamp);

            if (chrono - this < chrono - prev)
                prev = this;
            else if (this - chrono < next - chrono)
                next = this;
        }
        close(fd);

        if (prev > chrono)      /* 沒有下一篇 */
            prev = chrono;
        if (next < chrono)      /* 沒有上一篇 */
            next = chrono;

        brh_add(prev, chrono, next);
    }
}
#endif

    static int
post_browse(
    XO *xo)
{
    HDR *hdr;
    int cmd, xmode, pos;
    char *dir, fpath[64], *board;

    char poolbuf[sizeof(HDR)*20];

    int key;

    dir = xo->dir;
    cmd = XO_NONE;
    board=currboard;

    for (;;)
    {

        pos = xo->pos;
        hdr = (HDR *) xo_pool + (pos - xo->top);
        xmode = hdr->xmode;
        if (xmode & (POST_CANCEL | POST_DELETE | POST_MDELETE))
            break;

#ifdef  HAVE_USER_MODIFY
        if (xmode & POST_CURMODIFY)
        {
            if (pid_find(hdr->xid))
            {
                vmsg("此文章正在修改中!!");
                break;
            }
            else
            {
                xmode = (hdr->xmode &= ~POST_CURMODIFY);
                hdr->xid = 0;
                rec_put(dir, hdr, sizeof(HDR), pos);
            }
        }
#endif

        if ((hdr->xmode & POST_LOCK) && !(HAS_PERM(PERM_SYSOP | PERM_BOARD) || (bbstate & STAT_BOARD) || !strcmp(hdr->owner, cuser.userid)))
            break;

        /* cache.20130407: for preventing bot to get data */
        char desc[128];
        time_t now;
        time(&now);

        snprintf(desc, sizeof(desc), "%s %s %s %d %s\n", Atime(&now), cuser.userid, currboard, hdr->chrono, ipv4addr);
        f_cat(FN_BROWSE_LOG, desc);

        hdr_fpath(fpath, dir, hdr);

        /* Thor.990204: 為考慮more 傳回值 */
        //    if ((key = more(fpath, MSG_POST)) == -1)
        //      break;

        cmd = XO_LOAD;
        post_history(xo, hdr);
        strcpy(currtitle, str_ttl(hdr->title));

        /* Thor.990204: 為考慮more 傳回值 */
        if ((key = more(fpath, FOOTER_POST)) < 0)
            break;

        if (key == -2)
            return XO_INIT;
        switch (xo_getch(xo, key))
        {
            case XO_BODY:
                continue;
            case Ctrl('U'):
                memcpy(poolbuf, xo_pool, sizeof(HDR)*20);
                every_U();
                memcpy(xo_pool, poolbuf, sizeof(HDR)*20);
                continue;
            case Ctrl('B'):
                every_B();
                continue;
            case 'y':
            case 'r':
                if (bbstate & STAT_POST)
                {
                    strcpy(quote_file, fpath);
                    if (do_reply(hdr) == XO_INIT)       /* 有成功地 post 出去了 */
                        return post_init(xo);
                }
                break;

            case 'm':
                if ((bbstate & STAT_BOARD) && !(xmode & POST_MARKED))
                {
                    hdr->xmode = xmode | POST_MARKED;
                    rec_put(dir, hdr, sizeof(HDR), pos);
                }
                break;

#ifdef HAVE_RECOMMEND
            case 'X':
            case '%':
                post_recommend(xo);
                break;
#endif
        }
        break;
    }

    return post_init(xo);
}


/* ----------------------------------------------------- */
/* 精華區                                                */
/* ----------------------------------------------------- */


    int
post_gem(
    XO *xo)
{
    char fpath[32];

    strcpy(fpath, "gem/");
    strcpy(fpath + 4, xo->dir);

    /* Thor.990118: 看版總管不給 GEM_SYSOP */
    XoGem(fpath, "", (HAS_PERM(PERM_SYSOP|PERM_BOARD|PERM_GEM)) ? GEM_SYSOP :
            (bbstate & STAT_BOARD ? GEM_MANAGER : GEM_USER));

    return post_init(xo);
}


/* ----------------------------------------------------- */
/* 看板備忘錄                                            */
/* ----------------------------------------------------- */


    static int
post_memo(
    XO *xo)
{
    char fpath[64];

    brd_fpath(fpath, currboard, FN_NOTE);
    /* Thor.990204: 為考慮more 傳回值 */
    if (more(fpath, NULL) == -1)
    {
        vmsg("本看板尚無「備忘錄」");
        return XO_FOOT;
    }

    return post_head(xo);
}

    static int
post_post(
    XO *xo)
{
    int mode;
    char fpath[64];

    if (!(bbstate & STAT_BOARD))
        return XO_NONE;

    mode = vans("發文公告 (D)刪除 (E)修改 (Q)取消？[E] ");
    if (mode != 'q')
    {
        brd_fpath(fpath, currboard, "post");
        if (mode == 'd')
        {
            unlink(fpath);
        }
        else
        {
            if (bbsothermode & OTHERSTAT_EDITING)
            {
                vmsg("你還有檔案還沒編完哦！");
            }
            else
            {
                if (vedit(fpath, NA))
                    vmsg(msg_cancel);
                return post_head(xo);
            }
        }
    }
    return XO_HEAD;
}

    static int
post_memo_edit(
    XO *xo)
{
    int mode;
    char fpath[64];

    if (!(bbstate & STAT_BOARD))
        return XO_NONE;

    mode = vans("備忘錄 (D)刪除 (E)修改 (Q)取消？[E] ");
    if (mode != 'q')
    {
        brd_fpath(fpath, currboard, FN_NOTE);
        if (mode == 'd')
        {
            unlink(fpath);
        }
        else
        {
            if (bbsothermode & OTHERSTAT_EDITING)
            {
                vmsg("你還有檔案還沒編完哦！");
            }
            else
            {
                if (vedit(fpath, NA))
                    vmsg(msg_cancel);
                return post_head(xo);
            }
        }
    }
    return XO_HEAD;
}


    static int
post_switch(
    XO *xo)
{
    int bno;
    BRD *brd;
    char bname[16];

    if ((brd = ask_board(bname, BRD_R_BIT, NULL)))
    {
        if (*bname && ((bno = brd - bshm->bcache) >= 0))
        {
            XoPost(bno);
            return XZ_POST;
        }
    }
    else
    {
        vmsg(err_bid);
    }
    return post_head(xo);
}


/* ----------------------------------------------------- */
/* 功能：tag / copy / forward / download                 */
/* ----------------------------------------------------- */


    int
post_tag(
    XO *xo)
{
    HDR *hdr;
    int tag, pos, cur;

    pos = xo->pos;
    cur = pos - xo->top;
    hdr = (HDR *) xo_pool + cur;

#ifdef XZ_XPOST
    if (xo->key == XZ_XPOST)
        pos = hdr->xid;
#endif

    if ((tag = Tagger(hdr->chrono, pos, TAG_TOGGLE)))
    {
        move(3 + cur, 0);
        //move(3 + cur, 8);
        //outc(tag > 0 ? '*' : ' ');
        //outs(tag > 0 ? " *" : "  ");
        post_item(xo->pos + 1, hdr);
    }

    /* return XO_NONE; */
    return xo->pos + 1 + XO_MOVE; /* lkchu.981201: 跳至下一項 */
}


/* ----------------------------------------------------- */
/* 板主功能：mark / delete                               */
/* ----------------------------------------------------- */


    static int
post_mark(
    XO *xo)
{
    if (bbstate & STAT_BOARD)
    {
        HDR *hdr;
        int pos, cur;

        pos = xo->pos;
        cur = pos - xo->top;
        hdr = (HDR *) xo_pool + cur;

        if (hdr->xmode & (POST_DELETE | POST_CANCEL | POST_MDELETE))
            return XO_NONE;

        hdr->xmode ^= POST_MARKED;
        rec_put(xo->dir, hdr, sizeof(HDR), xo->key == XZ_POST ? pos : hdr->xid);
        //    move(3 + cur, 7);
        //    outc(post_attr(hdr));
        move(3 + cur, 0);
        post_item(pos + 1, hdr);

    }
    return XO_NONE;
}

    static int
lazy_delete(
    HDR *hdr)
{
    if (!strcmp(hdr->owner, cuser.userid))
    {
        sprintf(hdr->title, "<< 本文章經 %s 刪除 >>", cuser.userid);
        hdr->xmode |= POST_DELETE;
    }
    else if (strlen(delete_reason) < 1)
    {
        sprintf(hdr->title, "<< 本文章經 %s 刪除 >>", cuser.userid);
        hdr->xmode |= POST_MDELETE;
    }
    else
    {
        sprintf(hdr->title, "[刪除]%s：%s", cuser.userid, delete_reason);
        hdr->xmode |= POST_MDELETE;
    }

    return 0;
}

    static int
post_delete(
    XO *xo)
{
    int pos, cur, by_BM;
    HDR *fhdr, phdr;
    char buf[80], fpath[80];

#define BN_DELETED      BRD_DELETED
#define BN_JUNK         BRD_JUNK

    if (!cuser.userlevel ||
            !strcmp(currboard, BN_DELETED) ||
            !strcmp(currboard, BN_JUNK))
        return XO_NONE;

    if (cuser.userlevel & PERM_DENYPOST)
    {
        vmsg("你正被停權中，無法刪除任何文章！");
        return XO_NONE;
    }

    pos = xo->pos;
    cur = pos - xo->top;
    fhdr = (HDR *) xo_pool + cur;

    if (fhdr->xmode & (POST_MARKED | POST_CANCEL | POST_DELETE | POST_MDELETE))
        return XO_NONE;

    if ((fhdr->xmode & POST_LOCK) && !(HAS_PERM(PERM_SYSOP| PERM_BOARD)||bbstate & STAT_BOARD))
        return XO_NONE;

    /* 090911.cache: 檢查文章狀況 */
    rec_get(xo->dir, &phdr, sizeof(HDR), pos);
    if (phdr.xmode & POST_RECOMMEND_ING)
    {
        if (!pid_find(phdr.xid))
        {
            phdr.xmode &= ~POST_RECOMMEND;
            phdr.xid = 0;
            rec_put(xo->dir, &phdr, sizeof(HDR), pos);
        }
        else
        {
            vmsg("其他使用者正在編輯推薦文章留言，請稍候。");
            return XO_NONE;
        }
    }

    if (phdr.xmode & POST_CURMODIFY)
    {
        vmsg("文章正在被修改，請稍候。");
        return XO_NONE;
    }

    by_BM = (strcmp(fhdr->owner, cuser.userid) ? 1 : 0);
    if (!(bbstate & STAT_BOARD) && by_BM)
        return XO_NONE;

    hdr_fpath(fpath, xo->dir, fhdr);
    if (vans(msg_del_ny) == 'y')
    {
        currchrono = fhdr->chrono;

        if (by_BM && (fhdr->xmode & POST_BOTTOM))
        {
            lazy_delete(fhdr); /* Thor.980911: 註解: 修改 xo_pool */
            move(3 + cur, 0);
            post_item(++pos, fhdr);
            return XO_FOOT;
        }

        if (by_BM/* && (bbstate & BRD_NOTRAN) && !(fhdr->xmode & POST_BOTTOM)*/)
            vget(b_lines, 0, "請輸入刪除理由：", delete_reason, 29, DOECHO);
        //    return 0;
        if (by_BM/* && bbstate & BRD_NOTRAN*/&& (bbstate & STAT_BOARD) && !strstr(fhdr->owner, ".") && !strstr(fhdr->lastrecommend, "$") && !(fhdr->xmode & POST_BOTTOM))
        {
            char folder[128], buf[80], cmd[64];
            ACCT tmp;

            usr_fpath(folder, fhdr->owner, fn_dir);
            if (acct_load(&tmp, fhdr->owner) >= 0)
            {
                if (vans("是否退回文章？[y/N]") == 'y')
                {
                    if (vans("是否給予劣文？[y/N]") == 'y')
                    {
                      addpoint2(1, fhdr->owner);
                      pmsg2("劣退完畢！");
                    }

                    FILE *fp;
                    time_t now = time(0);
                    HDR mhdr;
                    hdr_stamp(folder, 0, &mhdr, buf);
                    strcpy(mhdr.owner, cuser.userid);
                    strcpy(mhdr.title, "文章退回通知");
                    rec_add(folder, &mhdr, sizeof(mhdr));

                    fp = fopen(buf, "w");
                    fprintf(fp, "作者: %s (%s)\n", cuser.userid, cuser.username);
                    fprintf(fp, "標題: %s\n時間: %s\n", "文章退回通知", ctime(&now));
                    fprintf(fp, "\n\033[1;31m***** 本信件由系統自動產生，如要申訴請重新上站後轉寄給站務並保留此信件 *****\033[m\n\n");
                    fprintf(fp, "\033[1;33m您在 %s 板的文章《%s》被退回\033[m\n", currboard, fhdr->title);
                    fprintf(fp, "\033[1;33m理由：%s\033[m\n\n", delete_reason);
                    fprintf(fp, "文章內容如下：\n\n");

                    if (dashf(fpath))
                    {
                        sprintf(cmd, "cp %s run/deleted.%s", fpath, cuser.userid);
                        system(cmd);
                        sprintf(cmd, "run/deleted.%s", cuser.userid);
                        f_suck(fp, cmd);
                        unlink(cmd);
                    }
                    fclose(fp);
                }
            }

        }

        /* Thor.980911: for 版主砍文章 in 串接 */
        /* if (!rec_del(xo->dir, sizeof(HDR), xo->pos, cmpchrono, lazy_delete)) */
        if (!rec_del(xo->dir, sizeof(HDR), xo->key == XZ_POST ? pos : fhdr->xid, (void *)cmpchrono, lazy_delete))
        {
            move_post(fhdr, by_BM ? BN_DELETED : BN_JUNK, by_BM);
            if (!by_BM && !(bbstate & BRD_NOCOUNT))
            {
                if (cuser.numposts > 0)
                    cuser.numposts--;
                sprintf(buf, "%s，您的文章減為 %d 篇", MSG_DEL_OK, cuser.numposts);
                vmsg(buf);
#ifdef  HAVE_DETECT_CROSSPOST
                checksum_find(fpath, 1, bbstate);
#endif
            }
            lazy_delete(fhdr); /* Thor.980911: 註解: 修改 xo_pool */
            move(3 + cur, 0);
            post_item(++pos, fhdr);
        }
    }
    return XO_FOOT;

#undef  BN_DELETED
#undef  BN_JUNK
}

static int
post_clean_delete(
    XO *xo)
{
    int pos, cur, by_BM;
    HDR *hdr;

    pos = xo->pos;
    cur = pos - xo->top;
    hdr = (HDR *) xo_pool + cur;

    by_BM = (strcmp(hdr->owner, cuser.userid) ? 1 : 0);

    if ((hdr->xmode & POST_MARKED) || (hdr->xmode & POST_LOCK) || !(bbstate & STAT_BOARD) )
    {
        return XO_NONE;
    }

    if (vans("是否直接砍除文章？[y/N]") == 'y')
    {
        currchrono = hdr->chrono;

        if (!rec_del(xo->dir, sizeof(HDR), xo->key == XZ_POST ? pos : hdr->xid, (void *)cmpchrono, 0))
        {
            move_post(hdr, by_BM ? BRD_DELETED : BRD_JUNK, by_BM);
            return XO_LOAD;
        }
    }
    return XO_FOOT;
}

#ifdef HAVE_POST_BOTTOM
    static int
post_bottom(
    XO *xo)
{
    if (bbstate & STAT_BOARD)
    {
        HDR *hdr, post;
        char fpath[64];

        hdr = (HDR *) xo_pool + (xo->pos - xo->top);

        //if ((hdr->xmode & POST_BOTTOM) && !HAVE_PERM(PERM_SYSOP)) /* 已置底就不能再置底 */
        //    return post_delete(xo);

        //TODO: 多樣化置底功能

        hdr_fpath(fpath, xo->dir, hdr);
        hdr_stamp(xo->dir, HDR_LINK | 'A', &post, fpath);

        if (hdr->xmode & POST_BOTTOM) /* 已經被置底的不能再置底 */
          return XO_NONE;
        else
          post.xmode = POST_MARKED | POST_BOTTOM;  /* 自動加 mark */

        strcpy(post.owner, hdr->owner);
        strcpy(post.nick, hdr->nick);
        strcpy(post.title, hdr->title);

        rec_add(xo->dir, &post, sizeof(HDR));
        btime_update(currbno);

        return post_load(xo);       /* 立刻顯示置底文章 */
    }
    return XO_NONE;
}
#endif

    static int
post_complete(
    XO *xo)
{
    if (HAS_PERM(PERM_SYSOP|PERM_BOARD))
    {
        HDR *hdr;
        int pos, cur;

        pos = xo->pos;
        cur = pos - xo->top;
        hdr = (HDR *) xo_pool + cur;

        hdr->xmode ^= POST_COMPLETE;
        rec_put(xo->dir, hdr, sizeof(HDR), xo->key == XZ_POST ? pos : hdr->xid);
        move(3 + cur, 7);
        outc(post_attr(hdr));
    }
    return XO_NONE;
}

    static int
post_lock(
    XO *xo)
{
    HDR *hdr;
    int pos, cur;

    if (!cuser.userlevel) /* itoc.020114: guest 不能對其他 guest 的文章加密 */
        return XO_NONE;

    pos = xo->pos;
    cur = pos - xo->top;
    hdr = (HDR *) xo_pool + cur;

    if (!strcmp(hdr->owner, cuser.userid) || HAS_PERM(PERM_SYSOP | PERM_BOARD) || (bbstate & STAT_BOARD))
    {
        if (hdr->xmode & (POST_DELETE | POST_CANCEL | POST_MDELETE))
            return XO_NONE;

        /* cache.100529: prevent user unlock then delete */
        if ((hdr->xmode & POST_LOCK) && !(bbstate & STAT_BOARD) && !HAS_PERM(PERM_ADMIN))
            return XO_NONE;

        hdr->xmode ^= POST_LOCK;
        rec_put(xo->dir, hdr, sizeof(HDR), xo->key == XZ_POST ? pos : hdr->xid);
        move(3 + cur, 7);
        outc(post_attr(hdr));
    }
    return XO_NONE;
}

/*cache.080520: 新版觀看文章屬性*/
    static int
post_state(
    XO *xo)
{
    HDR *ghdr;
    char fpath[64], *dir, buf[32];
    struct stat st;

    ghdr = (HDR *) xo_pool + (xo->pos - xo->top);

    dir = xo->dir;
    hdr_fpath(fpath, dir, ghdr);

    strcpy(buf, currboard);

#ifndef M3_USE_PFTERM
    grayout(GRAYOUT_DARK);
#endif

    if (HAS_PERM(PERM_ADMIN))
    {
        move(b_lines - 10, 0);
        clrtobot();

        prints("\033[1;34m"MSG_BLINE"\033[m");
        prints("\n\033[1;33;44m \033[37m文章代碼及資訊查詢： %*s \033[m", 55, "");
        outs("\n\n \033[1;37m★\033[m 文章索引: ");
        outs(dir);
        outs("\n \033[1;37m★\033[m 文章代碼: #");
        outs("\033[1;32m");
        outs(ghdr->xname);
        outs("\033[m");
        outs("\n \033[1;37m★\033[m 好讀連結: " URL_PREFIX "/");
        outs(buf);
        outs("/");
        outs(ghdr->xname);
        outs("\033[m");
        outs("\n \033[1;37m★\033[m 文章位置: ");
        outs(fpath);
#/*
        int k, l, m;
        k = l = m = 0;
        if (ghdr->chrono > ghdr->stamp)
            k=1;
        else if (ghdr->chrono < ghdr->stamp )
            k=2;
        else if (ghdr->chrono = ghdr->stamp)
            k=3;
        else
            k=4;

        if (ghdr->stamp = ghdr->pushtime)
            l=1;
        else
            l=2;
*/
        if (!stat(fpath, &st))
            prints("\n \033[1;37m★\033[m 最後編輯: %s\n \033[1;37m★\033[m 檔案大小: \033[1;32m%d\033[m bytes", Ctime(&st.st_mtime), st.st_size);

    }
    else if (!(cuser.userlevel))
    {
        vmsg("您的權限不足");
        return XO_HEAD;
    }
    else
    {
        move(b_lines - 8, 0);
        clrtobot();

        prints("\033[1;34m"MSG_BLINE"\033[m");
        prints("\n\033[1;33;44m \033[37m文章代碼及資訊查詢： %*s \033[m", 55, "");
        if (ghdr->xmode & (POST_EXPIRE | POST_MDELETE | POST_DELETE | POST_CANCEL | POST_LOCK | POST_CURMODIFY))
        {
          outs("\n\n \033[1;37m★\033[m 文章被鎖定、編輯或者刪除中");
          outs("\033[m");
          outs("\n");
        }
        else
        {
          outs("\n\n \033[1;37m★\033[m 文章代碼: #");
          outs("\033[1;32m");
          outs(ghdr->xname);
          outs("\033[m");
          outs("\n \033[1;37m★\033[m 好讀連結: " URL_PREFIX "/");
          outs(buf);
          outs("/");
          outs(ghdr->xname);
          outs("\033[m");
        }
        if (!stat(fpath, &st))
            prints("\n \033[1;37m★\033[m 最後存取: %s\n \033[1;37m★\033[m 檔案大小: \033[1;32m%d\033[m bytes", Ctime(&st.st_mtime), st.st_size);

    }

    vmsg(NULL);

    return XO_HEAD;
}

#if 0
    static int
post_state(
    XO *xo)
{
    HDR *hdr;
    char *dir, fpath[80];
    struct stat st;

    if (!(HAS_PERM(PERM_SYSOP)))
        return XO_NONE;

    dir = xo->dir;
    hdr = (HDR *) xo_pool + xo->pos - xo->top;
    hdr_fpath(fpath, dir, hdr);


    move(12, 0);
    clrtobot();
    outs("\nDir : ");
    outs(dir);
    outs("\nName: ");
    outs(hdr->xname);
    outs("\nFile: ");
    outs(fpath);
    if (!stat(fpath, &st))
        prints("\nTime: %s\nSize: %d\n", Ctime(&st.st_mtime), st.st_size);
    bitmsg("Flag: ", "rmg---cdIEOR--------DLSMC-------", hdr->xmode);
    prints("Xid : %d\n", hdr->xid);
    prints("Modify : %d times\n", hdr->modifytimes);
    prints("Chrono : %d Pushtime : %d\n", hdr->chrono, hdr->pushtime);

    vmsg(NULL);

    return post_body(xo);
}
#endif

    static int
post_undelete(
    XO *xo)
{
    int pos, cur, i, len;
    HDR *fhdr;
    char buf[256], fpath[128], *ptr;
    FILE *fp;

    if (!cuser.userlevel)
        return XO_NONE;

    pos = xo->pos;
    cur = pos - xo->top;
    fhdr = (HDR *) xo_pool + cur;
    hdr_fpath(fpath, xo->dir, fhdr);

    if (!(fhdr->xmode & (POST_MDELETE | POST_DELETE | POST_CANCEL)))
        return XO_NONE;

    if ( !(((fhdr->xmode & POST_DELETE) && !strcmp(fhdr->owner, cuser.userid))||
                ((fhdr->xmode & (POST_MDELETE | POST_CANCEL)) && (bbstate & STAT_BOARD))||
                HAS_PERM(PERM_SYSOP)) )
        return XO_NONE;


    fp = fopen(fpath, "r");
    if (fp)
    {
        fgets(buf, 256, fp);
        fgets(buf, 256, fp);
        buf[strlen(buf)-1] = 0;
        ptr = strchr(buf, ':');
        ptr = ptr ? ptr+2:buf;
        strncpy(fhdr->title, ptr, 60);
        if (!HAS_PERM(PERM_SYSOP))
        {
            sprintf(buf, "{%s}", cuser.userid);
            strcat(fhdr->title, buf);
        }
        fhdr->title[71] = 0;  /* verit 2002.01.23 避免救文章造成 title 爆掉 */

        /* verit 2003.10.16 避免救文章時, 出現彩色標題 */
        len = strlen(fhdr->title);
        for ( i=0; i<len; ++i )
            if ( fhdr->title[i] == '\033' )
                fhdr->title[i] = '*';

        fclose(fp);
#if 0
        if (!strcmp(fhdr->owner, cuser.userid) && (fhdr->xmode & POST_DELETE)
                && !(bbstate & BRD_NOCOUNT))
        {
            /*
            cuser.numposts++;
            sprintf(buf, "復原刪除，您的文章增為 %d 篇", cuser.numposts);
            vmsg(buf);*/  /* 20000724 visor: 有文章篇數的 bug */
#ifdef  HAVE_DETECT_CROSSPOST
            checksum_find(fpath, 0, bbstate);
#endif
        }
#endif
    }
    fhdr->xmode &= (~(POST_MDELETE | POST_DELETE | POST_CANCEL));
    if (!rec_put(xo->dir, fhdr, sizeof(HDR), pos))
    {
        move(3 + cur, 0);
        post_item(++pos, fhdr);
    }
    /*return XO_LOAD;*/
    return xo->pos + 1 + XO_MOVE;
}

    static int
post_expire(
    XO *xo)
{
    int pos, cur;
    HDR *fhdr;
    char fpath[80];

    if (!cuser.userlevel ||
            !strcmp(currboard, BRD_DELETED) ||
            !strcmp(currboard, BRD_JUNK))
        return XO_NONE;

    if (!HAS_PERM(PERM_ALLBOARD))
        return XO_NONE;

    pos = xo->pos;
    cur = pos - xo->top;
    fhdr = (HDR *) xo_pool + cur;

    if (fhdr->xmode & (POST_MARKED | POST_CANCEL | POST_DELETE | POST_MDELETE))
        return XO_NONE;

    if ((fhdr->xmode & POST_LOCK) && !(HAS_PERM(PERM_SYSOP| PERM_BOARD)||bbstate & STAT_BOARD))
        return XO_NONE;

    hdr_fpath(fpath, xo->dir, fhdr);
    if (fhdr->xmode & POST_EXPIRE)
      fhdr->xmode &= ~POST_EXPIRE;
    else
      fhdr->xmode |= POST_EXPIRE;
    fhdr->expire = time(0) + 86400 * 7;
    if (!rec_put(xo->dir, fhdr, sizeof(HDR), pos))
    {
        move(3 + cur, 0);
        post_item(++pos, fhdr);
    }
    return XO_NONE;

}

    static int
post_unexpire(
    XO *xo)
{
    int pos, cur;
    HDR *fhdr;
    char fpath[128];

    if (!HAS_PERM(PERM_ALLBOARD))
        return XO_NONE;

    pos = xo->pos;
    cur = pos - xo->top;
    fhdr = (HDR *) xo_pool + cur;

    if (!(fhdr->xmode & (POST_EXPIRE)))
        return XO_NONE;

    hdr_fpath(fpath, xo->dir, fhdr);

    if ( !((bbstate & STAT_BOARD) || HAS_PERM(PERM_ALLBOARD)) )
        return XO_NONE;

    fhdr->xmode &= (~(POST_EXPIRE));
    fhdr->expire = 0;
    if (!rec_put(xo->dir, fhdr, sizeof(HDR), pos))
    {
        move(3 + cur, 0);
        post_item(++pos, fhdr);
    }
    return XO_NONE;
}

/* ----------------------------------------------------- */
/* 站長功能：edit / title                                */
/* ----------------------------------------------------- */

int
post_edit(
    XO *xo)
{
    HDR *hdr;
    char fpath[80];
    int pos;
#ifdef  HAVE_USER_MODIFY
    int temp=0;
    int bno;
    BRD *brd;
    char buf[512], mfpath[80], mfolder[80], str[256];
    int fd;
    time_t now;
    FILE *fp, *xfp;

    bno = brd_bno(currboard);
    brd = bshm->bcache + bno;

#endif

    if (bbsothermode & OTHERSTAT_EDITING)
    {
        vmsg("你還有檔案還沒編完哦！");
        return XO_FOOT;
    }
    pos = xo->pos;
    hdr = (HDR *) xo_pool + (pos - xo->top);

    pos = seek_rec(xo, hdr);

    hdr_fpath(fpath, xo->dir, hdr);
#if 0
    if ((cuser.userlevel & PERM_ALLBOARD)|| ( (cuser.userlevel & PERM_VALID) \
                                            && !strcmp(hdr->owner, cuser.userid)))
#endif
    if (HAS_PERM(PERM_SYSOP) && !(hdr->xmode & (POST_CANCEL|POST_DELETE)))
    {
        /*hdr = (HDR *) xo_pool + (xo->pos - xo->top);
        hdr_fpath(fpath, xo->dir, hdr);*/
        vedit(fpath, NA); /* Thor.981020: 注意被talk的問題 */
        post_head(xo);
    }
#ifdef  HAVE_USER_MODIFY
    else if ((!(brd->battr & BRD_MODIFY)) && HAS_PERM(PERM_VALID) && !strcmp(hdr->owner, cuser.userid) && !(hdr->xmode & (/*POST_MODIFY|*/POST_CANCEL|POST_DELETE|POST_LOCK|POST_MARKED|POST_MDELETE|POST_CURMODIFY)))
    {
        if (hdr->xmode & POST_RECOMMEND)
        {
            if (!pid_find(hdr->xid))
                hdr->xmode &= ~POST_RECOMMEND;
            else
            {
                vmsg("有人在推薦您的文章，請稍候。");
                return XO_NONE;
            }
        }

        hdr->xmode |= POST_CURMODIFY;
        hdr->xid = cutmp->pid;
        rec_put(xo->dir, hdr, sizeof(HDR), pos);

        if (strcmp(brd->brdname, "test"))
        {
            HDR phdr;

            brd_fpath(mfolder, BRD_MODIFIED, FN_DIR);
            fd = hdr_stamp(mfolder, 'A', &phdr, mfpath);
            fp = fdopen(fd, "w");
            f_suck(fp, fpath);
            fclose(fp);
            close(fd);

            strcpy(phdr.owner, hdr->owner);
            strcpy(phdr.nick, hdr->nick);
            strcpy(phdr.title, hdr->title);

            rec_add(mfolder, &phdr, sizeof(HDR));
        }

        strcpy(ve_title, hdr->title);

        fp = fopen(fpath, "r");

        sprintf(buf, "tmp/%s.header", cuser.userid);
        xfp = fopen(buf, "w");
        while (fgets(str, 256, fp) && *str != '\n')
        {
            fputs(str, xfp);
        }
        fputs("\n", xfp);
        fclose(xfp);

        sprintf(buf, "tmp/%s.edit", cuser.userid);
        xfp = fopen(buf, "w");
        while (fgets(str, 256, fp))
        {
            if (!strcmp(str, "--\n"))
                break;
            fputs(str, xfp);
        }
        fclose(xfp);

        sprintf(buf, "tmp/%s.footer", cuser.userid);
        xfp = fopen(buf, "w");
        fputs("--\n", xfp);
        while (fgets(str, 256, fp))
        {
//          if (!strncmp(str, "\x1b[1;33m→", 9))
//  modified by cat@20090422
            if ((!strncmp(str, "\x1b[1;33m", 7) || !strncmp(str, "\x1b[1;31m", 7)) && strrchr(str, '/') > str)
            {
                temp = 1;
                break;
            }
            else if ((!strncmp(str, "\x1b[m\x1b[1;33m", 10) || !strncmp(str, "\x1b[m\x1b[1;31m", 7)) && strrchr(str, '/') > str)
            {
                temp = 1;
                break;
            }
            fputs(str, xfp);
        }
        fclose(xfp);

        sprintf(buf, "tmp/%s.recommend", cuser.userid);
        xfp = fopen(buf, "w");
        if (temp)
            fputs(str, xfp);

        while (fgets(str, 256, fp) && *str != '\n')
            fputs(str, xfp);

        fclose(xfp);
        fclose(fp);


        sprintf(buf, "tmp/%s.edit", cuser.userid);

        if ((temp = vedit(buf, NA)) < 0)
        {
            sprintf(buf, "tmp/%s.header", cuser.userid);
            unlink(buf);
            sprintf(buf, "tmp/%s.edit", cuser.userid);
            unlink(buf);
            sprintf(buf, "tmp/%s.footer", cuser.userid);
            unlink(buf);
            sprintf(buf, "tmp/%s.recommend", cuser.userid);
            unlink(buf);
        }
        else
        {
            char tmp[128];

            fp = fopen(fpath, "w");
            sprintf(buf, "tmp/%s.header", cuser.userid);
            f_suck(fp, buf);
            unlink(buf);
            sprintf(buf, "tmp/%s.edit", cuser.userid);
            f_suck(fp, buf);
            unlink(buf);
            sprintf(buf, "tmp/%s.footer", cuser.userid);
            time(&now);
            sprintf(tmp, MODIFY_TAG, fromhost, ctime(&now));
            f_cat(buf, tmp);
            f_suck(fp, buf);
            unlink(buf);
            sprintf(buf, "tmp/%s.recommend", cuser.userid);
            f_suck(fp, buf);
            unlink(buf);
            fclose(fp);
        }

        if ((pos = seek_rec(xo, hdr)) >= 0)
        {
            if (temp>=0 && strcmp(brd->brdname, "test") /*&& ++hdr->modifytimes >= 5*/)
                hdr->xmode |= POST_MODIFY;
            hdr->xmode &= ~POST_CURMODIFY;
            hdr->xid = 0;
            if (temp>=0)
            {
                hdr->pushtime = time(0);
                brh_add(hdr->pushtime, hdr->pushtime, hdr->pushtime);
            }

            rec_put(xo->dir, hdr, sizeof(HDR), pos);
            if (temp < 0)
                vmsg("取消修改");
            else
            {
                brd->blast = time(0);
                vmsg("修改完成");
            }
            post_init(xo);
        }
    }
    else if (brd->battr & BRD_MODIFY)
    {
        vmsg("本板不能修改文章!!");
    }
    else
    {
        /* cache.090922: 檢查機制 */
        if (hdr->modifytimes < 0)
            hdr->modifytimes = 0;
        vmsg("此文章不能被修改!!");
    }
#endif
    return XO_FOOT;
}

#if 0
int post_edit(XO *xo)
{
    HDR *hdr;
    char fpath[80];
    int pos;
#ifdef  HAVE_USER_MODIFY
    int bno;
    BRD *brd;
    char buf[512], mfpath[80], mfolder[80], str[256];
    int fd;
    HDR phdr;
    time_t now;
    FILE *fp, *xfp;

    bno = brd_bno(currboard);
    brd = bshm->bcache + bno;


#endif

    if (bbsothermode & OTHERSTAT_EDITING)
    {
        vmsg("你還有檔案還沒編完哦！");
        return XO_FOOT;
    }
    pos = xo->pos;
    hdr = (HDR *) xo_pool + (xo->pos - xo->top);
    hdr_fpath(fpath, xo->dir, hdr);
#if 0
    if ((cuser.userlevel & PERM_ALLBOARD)|| ( (cuser.userlevel & PERM_VALID) \
                && !strcmp(hdr->owner, cuser.userid)))
#endif
        if (HAS_PERM(PERM_ALLBOARD))
        {
            /*hdr = (HDR *) xo_pool + (xo->pos - xo->top);
              hdr_fpath(fpath, xo->dir, hdr);*/
            vedit(fpath, NA); /* Thor.981020: 注意被talk的問題 */
            post_head(xo);
        }
#ifdef  HAVE_USER_MODIFY
        else if ((brd->battr & BRD_MODIFY) && HAS_PERM(PERM_VALID) /*&& ((hdr->modifytimes)<MAX_MODIFY)*/ && !strcmp(hdr->owner, cuser.userid) && !(hdr->xmode & (/*POST_MODIFY|*/POST_CANCEL|POST_DELETE|POST_LOCK|POST_MARKED|POST_MDELETE/*|POST_CURMODIFY*/)) )
        {
            //    move_post(hdr, BRD_MODIFIED, -3);

            brd_fpath(mfolder, BRD_MODIFIED, FN_DIR);
            fd = hdr_stamp(mfolder, 'A', &phdr, mfpath);
            fp = fdopen(fd, "w");

            f_suck(fp, fpath);
            fclose(fp);
            close(fd);
            strcpy(phdr.owner, hdr->owner);
            strcpy(phdr.nick, hdr->nick);
            strcpy(phdr.title, hdr->title);

            rec_bot(mfolder, &phdr, sizeof(HDR));

            hdr->xmode |= POST_CURMODIFY;
            hdr->xid = cutmp->pid;
            rec_put(xo->dir, hdr, sizeof(HDR), pos);

            strcpy(ve_title, hdr->title);

            fp = fopen(fpath, "r");

            sprintf(buf, "tmp/%s.header", cuser.userid);
            xfp = fopen(buf, "w");

            while (fgets(str, 256, fp) && *str != '\n')
            {
                fputs(str, xfp);
            }
            fputs("\n", xfp);
            fclose(xfp);

            sprintf(buf, "tmp/%s.edit", cuser.userid);
            xfp = fopen(buf, "w");
            while (fgets(str, 256, fp))
            {
                if (!strcmp(str, "--\n"))
                    break;
                fputs(str, xfp);
            }
            fclose(xfp);

            sprintf(buf, "tmp/%s.footer", cuser.userid);
            xfp = fopen(buf, "w");
            fputs("--\n", xfp);
            while (fgets(str, 256, fp))
            {
                fputs(str, xfp);
            }
            fclose(xfp);

            fclose(fp);

            sprintf(buf, "tmp/%s.edit", cuser.userid);

            if (vedit(buf, NA)<0)
            {
                sprintf(buf, "tmp/%s.header", cuser.userid);
                unlink(buf);
                sprintf(buf, "tmp/%s.edit", cuser.userid);
                unlink(buf);
                sprintf(buf, "tmp/%s.footer", cuser.userid);
                unlink(buf);

                rec_get(xo->dir, &phdr, sizeof(HDR), pos);
                phdr.xmode &= ~POST_CURMODIFY;
                phdr.xid = 0;
                rec_put(xo->dir, &phdr, sizeof(HDR), pos);
                vmsg("取消修改");
            }
            else
            {
                fp = fopen(fpath, "w");
                sprintf(buf, "tmp/%s.header", cuser.userid);
                f_suck(fp, buf);
                unlink(buf);
                sprintf(buf, "tmp/%s.edit", cuser.userid);
                f_suck(fp, buf);
                unlink(buf);
                sprintf(buf, "tmp/%s.footer", cuser.userid);
                f_suck(fp, buf);
                unlink(buf);
                fclose(fp);

                time(&now);
                sprintf(buf, MODIFY_TAG, fromhost, ctime(&now));
                f_cat(fpath, buf);
                rec_get(xo->dir, &phdr, sizeof(HDR), pos);

                phdr.xmode |= POST_MODIFY;
                phdr.xmode &= ~POST_CURMODIFY;
                phdr.xid = 0;
                rec_put(xo->dir, &phdr, sizeof(HDR), pos);

                /* cache.090922: 修改次數檢查機制 */
                /*
                    if (hdr->modifytimes < 0)
                        hdr->modifytimes = 1;
                    else
                        hdr->modifytimes += 1;
                 */
                vmsg("修改完成");
            }
            post_init(xo);
        }
        else if (!(brd->battr & BRD_MODIFY))
        {
            vmsg("本看板不能修改文章!!");
            return XO_FOOT;
        }
        else
        {
            /* cache.090922: 檢查機制 */
            if (hdr->modifytimes < 0)
                hdr->modifytimes = 0;

            vmsg("此文章不能被修改!!");
            return XO_FOOT;
        }
#endif
    return XO_NONE;
}
#endif

void
header_replace(         /* 0911105.cache: 修改文章標題順便修改內文的標題 */
    XO *xo,
    HDR *hdr)
{
    FILE *fpr, *fpw;
    char srcfile[64], tmpfile[64], buf[ANSILINELEN];

    hdr_fpath(srcfile, xo->dir, hdr);
    strcpy(tmpfile, "tmp/");
    strcat(tmpfile, hdr->xname);
    f_cp(srcfile, tmpfile, O_TRUNC);

    if (!(fpr = fopen(tmpfile, "r")))
        return;

    if (!(fpw = fopen(srcfile, "w")))
    {
        fclose(fpr);
        return;
    }

    fgets(buf, sizeof(buf), fpr);               /* 加入作者 */
    fputs(buf, fpw);

    fgets(buf, sizeof(buf), fpr);               /* 加入標題 */
    if (!str_ncmp(buf, "標", 2))                /* 如果有 header 才改 */
    {
        strcpy(buf, buf[2] == ' ' ? "標  題: " : "標題: ");
        strcat(buf, hdr->title);
        strcat(buf, "\n");
    }
    fputs(buf, fpw);

    while (fgets(buf, sizeof(buf), fpr))        /* 加入其他 */
        fputs(buf, fpw);

    fclose(fpr);
    fclose(fpw);
    f_rm(tmpfile);
}

    int
post_title(
    XO *xo)
{
    HDR *fhdr, mhdr;
    int pos, cur;

    pos = xo->pos;
    cur = pos - xo->top;
    fhdr = (HDR *) xo_pool + cur;
    mhdr = *fhdr;

    /* 100620.cache: 作者可以改標題 */
    //if (strcmp(mhdr.owner, cuser.userid))
    //{
    //    if (!(bbstate & STAT_BOARD))
    //        return XO_NONE;  /* 0911105.cache: 以防萬一 */
    //}

    if (!(bbstate & STAT_BOARD))
        return XO_NONE;

    vget(b_lines, 0, "標題：", mhdr.title, sizeof(mhdr.title), GCARRY);

    if (HAS_PERM(PERM_ALLBOARD))  /* 0911105.cache: 非看板總管只能改標題 */
    {
        vget(b_lines, 0, "作者：", mhdr.owner, 74 /* sizeof(mhdr.owner)*/, GCARRY);
        /* Thor.980727:lkchu patch: sizeof(mhdr.owner) = 80會超過一行 */
        vget(b_lines, 0, "日期：", mhdr.date, sizeof(mhdr.date), GCARRY);
    }

    if (vans(msg_sure_ny) == 'y' &&
            memcmp(fhdr, &mhdr, sizeof(HDR)))
    {
        *fhdr = mhdr;
        rec_put(xo->dir, fhdr, sizeof(HDR), pos);
        move(3 + cur, 0);
        post_item(++pos, fhdr);

        /* 0911105.cache: 順便改內文標題 */
        header_replace(xo, fhdr);

    }
    return XO_FOOT;
}


#ifdef HAVE_TERMINATOR
    static int
post_cross_terminator(  /* Thor.0521: 終極文章大法 */
    XO *xo)
{
    char *title, buf[128], other[128];
    int mode;
    HDR *fhdr;

    fhdr = (HDR *) xo_pool + xo->pos - xo->top;
    if (fhdr->xmode & (POST_DELETE | POST_MDELETE | POST_CANCEL | POST_LOCK))
        return XO_NONE;


    if (!HAS_PERM(PERM_ALLBOARD))
        return XO_NONE;

    mode = vans("《拂花落楓斬》： 1)砍標題 2)砍使用者 3)其他 [1]：") - '1';
    if (mode > 2 || mode < 0)
        mode =0;

    strcpy(currtitle, str_ttl(fhdr->title));

    if (mode==1)
        title = fhdr->owner;
    else if (mode == 2)
    {
        if (!vget(b_lines, 0, "其他：", other, sizeof(other), DOECHO))
            return XO_HEAD;
        title = other;
    }
    else
        title = currtitle;
    if (!*title)
        return XO_NONE;

    if (mode==1)
        sprintf(buf, "《拂花落楓斬》使用者：%.40s，確定嗎？Y/[N]", title);
    else if (mode ==2)
        sprintf(buf, "《拂花落楓斬》其他：%.50s，確定嗎？Y/[N]", title);
    else
        sprintf(buf, "《拂花落楓斬》標題：%.40s，確定嗎？Y/[N]", title);


    if (vans(buf) == 'y')
    {
        BRD *bhdr, *head, *tail;

        /* Thor.0616: 記下 currboard, 以便復原 */
        strcpy(buf, currboard);

        head = bhdr = bshm->bcache;
        tail = bhdr + bshm->number;
        do                              /* 至少有sysop一版 */
        {
            int fdr, fsize, xmode;
            FILE *fpw;
            char fpath[80];
            char fnew[80], fold[80];
            HDR *hdr;

            if (!str_cmp(head->brdname, BRD_LOCALPOSTS))  /* LocalPosts 版不砍 */
                continue;

            if (!str_cmp(head->brdname, brd_sysop))  /* SYSOP 版不砍 */
                continue;

            if (!str_cmp(head->brdname, BRD_CAMERA))  /* ActiveInfo 版不砍 */
                continue;

#ifdef  HAVE_CROSSPOSTLOG
            if (!str_cmp(head->brdname, BRD_CROSSPOST))  /* CostPost 版不砍 */
                continue;
#endif

            /* Thor.0616:更改currboard, 以cancel post */

            strcpy(currboard, head->brdname);

            sprintf(fpath, "《拂花落楓斬》看版：%s \033[5m...\033[m", currboard);
            outz(fpath);
            refresh();

            brd_fpath(fpath, currboard, fn_dir);

            if ((fdr = open(fpath, O_RDONLY)) < 0)
                continue;

            if (!(fpw = f_new(fpath, fnew)))
            {
                close(fdr);
                continue;
            }

            fsize = 0;
            mgets(-1);
            while ((hdr = mread(fdr, sizeof(HDR))))
            {
                int check_mode;
                xmode = hdr->xmode;

                /*if (xmode & (POST_CANCEL | POST_DELETE | POST_MDELETE | POST_LOCK))
                    continue;*/

                if (mode==1)
                    check_mode = strcmp(title, str_ttl(hdr->owner));
                else if (mode==2)
                    check_mode = !((int)strstr(hdr->owner, title)|(int)strstr(hdr->title, title));
                else
                    check_mode = strcmp(title, str_ttl(hdr->title));

                if ((xmode & (POST_MARKED | POST_CANCEL | POST_DELETE | POST_MDELETE |
                                POST_LOCK)) || check_mode )
                {
#if 0
                    if ((fwrite(hdr, sizeof(HDR), 1, fpw) != 1))
                    {
                        fclose(fpw);
                        unlink(fnew);
                        close(fdr);
                        goto contWhileOuter;
                    }
                    fsize++;
#endif
                }
                else
                {
                    /* 若為看板就連線砍信 */

                    cancel_post(hdr);
                    hdr->xmode |= POST_MDELETE;
                    sprintf(hdr->title, "<< 本文章經 %s 做系統功\能刪除 >>", cuser.userid);
                    /*hdr_fpath(fold, fpath, hdr);
                    unlink(fold);*/
                }

                if ((fwrite(hdr, sizeof(HDR), 1, fpw) != 1))
                {
                    fclose(fpw);
                    unlink(fnew);
                    close(fdr);
                    goto contWhileOuter;
                }
                fsize++;
            }
            close(fdr);
            fclose(fpw);

            sprintf(fold, "%s.o", fpath);
            rename(fpath, fold);
            if (fsize)
                rename(fnew, fpath);
            else
                unlink(fnew);

contWhileOuter:
            ;
        } while (++head < tail);

        strcpy(currboard, buf);
        post_load(xo);
    }

    return XO_FOOT;
}
#endif

    int
post_ban_mail(
    XO *xo)
{

    if ((bbstate & STAT_BOARD)||HAS_PERM(PERM_ALLBOARD))
    {
        post_mail();
        return post_init(xo);
    }
    else
        return XO_NONE;
}

#ifdef  HAVE_BRDTITLE_CHANGE
    static int
post_brdtitle(
    XO *xo)
{
    int bno;
    BRD *oldbrd, newbrd;

    if ( !(bbstate & STAT_BOARD) ) /* 感謝 visor@YZU */
        return XO_NONE;

    bno = brd_bno(currboard);
    oldbrd = bshm->bcache + bno;

    memcpy(&newbrd, oldbrd, sizeof(BRD));

    vget(23, 0, "看板名稱：", newbrd.title+3, BTLEN - 2, GCARRY);

    if ((vans(msg_sure_ny) == 'y') &&
            memcmp(&newbrd, oldbrd, sizeof(BRD)))
    {
        memcpy(oldbrd, &newbrd, sizeof(BRD));
        rec_put(FN_BRD, &newbrd, sizeof(BRD), bno);
    }
    vmsg("設定完畢");

    return XO_HEAD;
}
#endif



#ifdef HAVE_RECOMMEND   /* gaod: 推文 */

/* 090907.cache 推文的 code 待修正 */

    void
record_recommend(const int chrono, const char * const text)
{
    char desc[256]/*, fpath[128]*/;
    time_t now;

    time(&now);

    /* 記錄到系統 */
    snprintf(desc, sizeof(desc), "%s %s 推文：%s %d %s\n", Atime(&now),
            cuser.userid, currboard, chrono, text);
    f_cat(FN_RECOMMEND_LOG, desc);
}

/*
    void
post_recommend_log(
    int mode,
    HDR *hdr)
{
    time_t now;
    char c_time[25], buf[300];

    now = time(0);
    strncpy(c_time, ctime(&now), 24);
    c_time[24] = '\0';

    sprintf(buf, "%s %s %s %s 板：%s(%d) from %s\n", c_time, cuser.userid, (mode == 0) ? "清除":"推薦", currboard, hdr->title, hdr->chrono, fromhost);
    f_cat(FN_RECOMMEND_LOG, buf);
}
 */

    int
post_resetscore(
    XO *xo)
{
    if ((bbstate & STAT_BOARD) || HAS_PERM(PERM_BOARD))
    {
        HDR *hdr;
        BRD *brd;
        int pos, cur, xmode, recommend, pm;
        char ans[3];

        pos = xo->pos;
        cur = pos - xo->top;
        hdr = (HDR *) xo_pool + cur;

        xmode = hdr->xmode;
        brd = bshm->bcache + brd_bno(currboard);

        if ( hdr->xmode & (POST_DELETE | POST_CANCEL | POST_MDELETE | POST_LOCK | POST_CURMODIFY))
            return XO_FOOT;

        //if (!hdr->recommend)
        //{
        //    vmsg("本篇文章沒有推文");
        //    return XO_FOOT;
        //}
        //else
        //{
            switch (vans("◎評分設定 1)自訂 2)清除 [Q] "))
            {
                case '1':

                    if (!HAS_PERM(PERM_SYSOP))
                    {
                        pmsg2("目前禁止自訂推文數");
                        return XO_FOOT;
                    }

                    if (!vget(b_lines, 0, "請輸入數字：", ans, 3, DOECHO))
                        return XO_FOOT;

                    if ((brd->battr & BRD_PUSHSNEER) || (brd->battr & BRD_PUSHDEFINE))
                        pm = vans("請選擇正負 1)正 2)負 [Q] ");
                    else
                        pm = '1';

                    if (pm =='1')
                    {
                        recommend = atoi(ans);
                    }
                    else if (pm == '2')
                    {
                        recommend = atoi(ans);
                        recommend = -recommend;
                    }
                    else
                        return XO_FOOT;

                    if (recommend > 99 || recommend < -99)
                        return XO_FOOT;
                    hdr->recommend = recommend;
                    break;

                case '2':
                    hdr->recommend = 0;
                    break;

                default:
                    return XO_FOOT;
            }

            strcpy(hdr->lastrecommend, cuser.userid);
            rec_put(xo->dir, hdr, sizeof(HDR), pos);

            move(3 + cur, 7);
            outc(post_attr(hdr));

            return XO_LOAD;
        //}


    }
    else
    {
        vmsg("您的權限不足！");
        return XO_FOOT;
    }
}


    int
post_recommend(
    XO *xo)
{
    HDR *hdr;
    int pos, cur, addscore, eof, point=0;
    BRD *brd;
    char fpath[80], msg[53], add[128], lastrecommend[IDLEN+1], verb[2];
    char ans, getans, pushverb;

    /* 081122.cache: 推文時間限制 */
    static time_t next = 0;

    brd = bshm->bcache + brd_bno(currboard);

    if (HAS_PERM(PERM_VALID) && (!(brd->battr & BRD_PRH)) && (bbstate & STAT_POST))
    {
        pos = xo->pos;
        cur = pos - xo->top;
        hdr = (HDR *) xo_pool + cur;

        if (!strcmp(hdr->lastrecommend, "$"))
        {
            zmsg("此文章不可推薦！");
            return XO_NONE;
        }

        /* 081122.cache: 推文時間限制 */
        if (brd->battr & BRD_PUSHTIME)
        {
            if ((ans = next - time(NULL)) > 0)
            {
                sprintf(fpath, "還有 %d 秒才能推文喔", ans);
                vmsg(fpath);
                return XO_FOOT;
            }
        }

        //更新資料操硬碟
        pos = seek_rec(xo, hdr);

        if (pos < 0)
            return XO_NONE;

        if (hdr->xmode & (POST_DELETE | POST_CANCEL | POST_MDELETE | POST_LOCK))
            return XO_NONE;
        else if (hdr->xmode & POST_CURMODIFY)
        {
            zmsg("作者修改文章中，請稍候。");
            return XO_NONE;
        }

        else if (brd->battr & BRD_PUSHDISCON)
        {
            if (!strcmp(hdr->lastrecommend, cuser.userid))
            {
                zmsg("不可連續推同樣一篇文章！自己也不能第一推");
                return XO_NONE;
            }
        }

        hdr->xmode |= POST_RECOMMEND_ING;
        hdr->xid = cutmp->pid;

        rec_put(xo->dir, hdr, sizeof(HDR), pos);

        /* 081121.cache: 推噓文功能 */
        /* 081122.cache: 自訂推噓文動詞 */
        if (brd->battr & BRD_PUSHSNEER)
        {
            //addscore = 0;
            //switch(ans = vans("◎ 評論 1)推文 2)噓文 3)留言 ？[Q] "))
            //考量夢大已經習慣推文是箭頭符號
            switch(ans = vans("◎ 評論 1)推文 2)噓文 ？[Q] "))
            {
                case '1':
                    getans = vget(b_lines, 0, "推文：", msg, 53, DOECHO);
                    addscore = 1;
                    break;
                case '2':
                    getans = vget(b_lines, 0, "噓文：", msg, 53, DOECHO);
                    addscore = -1;
                    break;
                case '3':
                    getans = vget(b_lines, 0, "留言：", msg, 53, DOECHO);
                    addscore = 0;
                    break;
                default:
                    getans = 0;
                    break;
            }
        }
        else if (brd->battr & BRD_PUSHDEFINE)
        {
            //addscore = 0;
            switch(ans = vans("◎ 評論 1)推文 2)噓文 3)留言 4)自訂推文 5)自訂噓文 ？[Q] "))
            {
                case '1':
                    getans = vget(b_lines, 0, "推文：", msg, 53, DOECHO);
                    strcpy(verb, "推");
                    addscore = 1;
                    break;
                case '2':
                    getans = vget(b_lines, 0, "噓文：", msg, 53, DOECHO);
                    strcpy(verb, "噓");
                    addscore = -1;
                    break;
                case '3':
                    getans = vget(b_lines, 0, "留言：", msg, 53, DOECHO);
                    addscore = 0;
                    break;

                case '4':
                    pushverb = vget(b_lines, 0, "請輸入自訂的正面動詞：", verb, 3, DOECHO);
                    eof = strlen(verb);
                    if (eof<2)
                    {
                        zmsg("動詞須為一個中文字元或者兩個英文字元");
                        return XO_FOOT;
                    }
                    getans = vget(b_lines, 0, "推文：", msg, 53, DOECHO);
                    addscore = 1;
                    break;

                case '5':
                    pushverb = vget(b_lines, 0, "請輸入自訂的負面動詞：", verb, 3, DOECHO);
                    eof = strlen(verb);
                    if (eof<2)
                    {
                        zmsg("動詞須為一個中文字元或者兩個英文字元");
                        return XO_FOOT;
                    }
                    getans = vget(b_lines, 0, "噓文：", msg, 53, DOECHO);
                    addscore = -1;
                    break;

                default:
                    getans = 0;
                    break;
            }
        }
        else
            getans = vget(b_lines, 0, "推文：", msg, 53, DOECHO);

        /* 081121.cache: 後悔的機會 */
        if (getans)
            ans = vans("請確定是否送出 ? [y/N]");
        else
            ans = 'n';

        //更新資料操硬碟
        pos = seek_rec(xo, hdr);
        hdr->xmode &= ~POST_RECOMMEND_ING;
        hdr->xid = 0;

        if (pos < 0)
            return XO_NONE;

        //優劣文
        if ( (brd->battr & (BRD_PUSHTIME | BRD_PUSHDISCON)) && (brd->battr & BRD_VALUE) )
        {
            if      (/*(hdr->recommend == 49) || */(hdr->recommend == 99))
            {
                if (addscore > 0)
                    point = 1;
                else
                    point = -1;
            }
            else if (/*(hdr->recommend == -49) || */(hdr->recommend == -99))
            {
                if (addscore > 0)
                    point = 1;
                else
                    point = -1;
            }
        }

        strcpy(lastrecommend, hdr->lastrecommend);

        if (ans == 'y' || ans == 'Y')
        {
            int fd;

            hdr->pushtime = time(0);

            //在 item 有判斷可以加快一點點
            //考量到舊版文章相容問題, 等到舊文章清除後要更改判斷
            if (!(hdr->xmode & POST_RECOMMEND))
                hdr->xmode |= POST_RECOMMEND;

            if (brd->battr & BRD_PUSHSNEER || brd->battr & BRD_PUSHDEFINE)
            {
                if (hdr->recommend < 125 && hdr->recommend > -125)
                {
                    if (addscore == 1)
                        hdr->recommend += 1;
                    else if (addscore == -1)
                        hdr->recommend -= 1;

                    /* 090923.cache: 如果發生 race condition, 上面加分的註解掉改用這段*/
                    /* 直接把 .DIR 中的 score 更新，不管 XO 裡面的 score 是記錄多少 */
                    /*
                        hdr->recommend +=addscore;
                     */
                }
            }
            else //無噓文相容性
            {
                if (hdr->recommend < 99)
                    hdr->recommend++;
            }

            strcpy(hdr->lastrecommend, cuser.userid);
            rec_put(xo->dir, hdr, sizeof(HDR), pos);

            hdr_fpath(fpath, xo->dir, hdr);

            /* 081121.cache: 推噓文和普通推薦有不同的outs */
            /* 081122.cache: 自訂推噓文動詞 */
            if (brd->battr & BRD_PUSHSNEER)
            {
                if (addscore == 1)
                    sprintf(add, "\x1b[1;33m→ %12s：\x1b[36m%-54.54s \x1b[m%5.5s\n", cuser.userid, msg, Btime(&hdr->pushtime)+3);
                else if (addscore == -1)
                    sprintf(add, "\x1b[1;31m噓\x1b[m \x1b[1;33m%12s：\x1b[36m%-54.54s \x1b[m%5.5s\n", cuser.userid, msg, Btime(&hdr->pushtime)+3);
                else
                    sprintf(add, "\x1b[m\x1b[1;33m   %12s：\x1b[36m%-54.54s \x1b[m%5.5s\n", cuser.userid, msg, Btime(&hdr->pushtime)+3);
            }
            else if (brd->battr & BRD_PUSHDEFINE)
            {
                if (addscore == 1)
                    sprintf(add, "\x1b[1;33m%02.2s %12s：\x1b[36m%-54.54s \x1b[m%5.5s\n", verb, cuser.userid, msg, Btime(&hdr->pushtime)+3);
                else if (addscore == -1)
                    sprintf(add, "\x1b[1;31m%02.2s\x1b[m \x1b[1;33m%12s：\x1b[36m%-54.54s \x1b[m%5.5s\n", verb, cuser.userid, msg, Btime(&hdr->pushtime)+3);
                else
                    sprintf(add, "\x1b[1;33m→\x1b[m \x1b[1;33m%12s：\x1b[36m%-54.54s \x1b[m%5.5s\n", cuser.userid, msg, Btime(&hdr->pushtime)+3);
            }
            else
                sprintf(add, "\x1b[1;33m→ %12s：\x1b[36m%-54.54s \x1b[m%5.5s\n", cuser.userid, msg, Btime(&hdr->pushtime)+3);
            /*
            if (dashf(fpath))
                f_cat(fpath, add);
             */
            if ((fd = open(fpath, O_WRONLY | O_APPEND)) >= 0)
            {
                f_exlock(fd);

                write(fd, add, strlen(add));

                f_unlock(fd);
                close(fd);
            }

            /* 081122.cache: 推文時間限制 */
            if (brd->battr & BRD_PUSHTIME)
                next = time(NULL) + NEXTPUSHTIME;  /* 定義在theme.h */

            //change_stamp(xo->dir, hdr);
            brh_add( hdr->pushtime, hdr->pushtime,  hdr->pushtime);

            /* 091009.cache: 優良積分 */
            if ( point!=0 )
            {
                addpoint1(point, hdr->owner);
                pmsg2("評論完成！(作者優文更動)");
            }
            else
                zmsg("評論完成！");

            /* 紀錄推文 */
            if (!HAVE_PERM(PERM_SYSOP))
                record_recommend(hdr->chrono, msg);

            //    post_recommend_log(1, hdr);
            //    brd->btime = time(0);[A
            //btime_update(currbno);
            brd->blast = hdr->pushtime;
            return XO_INIT;
            //move(3 + cur, 0);
            //post_item(pos+1, hdr);

        }
        else
        {
            strcpy(hdr->lastrecommend, lastrecommend);
            rec_put(xo->dir, hdr, sizeof(HDR), pos);
            zmsg("取消");
        }
    }
    return XO_FOOT;
}

#endif

/* cache.081122: 看板資訊顯示 */
    static int
post_showBRD_setting(
    XO *xo)
{
    char *str;
    BRD *brd;

    brd = bshm->bcache + brd_bno(currboard);

    str = brd->BM;
    if (*str <= ' ')
        str = "\033[1;33m徵求中\033[m";

#ifndef M3_USE_PFTERM
    grayout(GRAYOUT_DARK);
#endif

    move(b_lines - 14, 0);
    clrtobot();  /* 避免畫面殘留 */

    prints("\033[1;34m"MSG_BLINE"\033[m");
    prints("\n\033[1;33;44m \033[37m看板設定及資訊查詢： %*s \033[m\n", 55, "");

    prints("\n看板:[%s]  板主:[%s] \n", brd->brdname, str);

    prints("\n 看板性質 - %s",
            (brd->battr & BRD_NOTRAN) ? "站內" : "\033[1;33m轉信\033[m");

    if (brd->battr & BRD_RSS)
        prints("    RSS 功\能 - " URL_PREFIX "/%s.xml ", brd->brdname);
    else
        prints("    RSS 功\能 - 關閉");

    prints("\n 記錄篇數 - %s    轉錄文章 - %s",
            (brd->battr & BRD_NOCOUNT) ? "忽略" : "\033[1;33m記錄\033[m",
            (brd->battr & BRD_NOFORWARD) ? "\033[1;31m不可轉錄\033[m" : "可以轉錄");

    prints("\n 熱門話題 - %s    看板狀態 - %s",
            (brd->battr & BRD_NOSTAT) ? "忽略" : "\033[1;33m記錄\033[m",
            (brd->battr & BRD_NOREPLY) ? "\033[1;31m看板唯讀\033[m" : "自由發表");

    prints("\n 投票結果 - %s    修改板名 - %s",
            (brd->battr & BRD_NOVOTE) ? "忽略" : "\033[1;33m記錄\033[m",
            (brd->battr & BRD_CHANGETITLE) ? "可以修改" : "\033[1;33m不能修改\033[m");

    prints("\n 匿名功\能 - %s    修改文章 - %s",
            (brd->battr & BRD_ANONYMOUS) ? "\033[1;33m開啟\033[m" : "關閉",
            (brd->battr & BRD_MODIFY) ? "\033[1;31m不能修改\033[m" : "可以修改");

    prints("\n 推文功\能 - %s    噓文功\能 - %s",
            (brd->battr & BRD_PRH) ? "關閉" : "\033[1;33m開啟\033[m",
            (brd->battr & BRD_PUSHSNEER || brd->battr & BRD_PUSHDEFINE) ? "\033[1;33m開啟\033[m" : "關閉");

    prints("\n 推文限制 - %s    自訂動詞 - %s",
            (brd->battr & BRD_PUSHDISCON) ? "\033[1;36mＩＤ\033[m" : (brd->battr & BRD_PUSHTIME) ?
            "\033[1;36m時間\033[m" : "沒有",
            (brd->battr & BRD_PUSHDEFINE) ? "\033[1;33m開啟\033[m" : "關閉");

    prints("\n 文章類別 - %s    禁注音文 - %s",
            (brd->battr & BRD_POSTFIX) ? "\033[1;33m開啟\033[m" : "關閉",
            (brd->battr & BRD_NOPHONETIC) ? "\033[1;33m開啟\033[m" : "關閉");

    prints("\n 進板記錄 - %s    ",
            (brd->battr & BRD_USIES) ? "\033[1;33m開啟\033[m" : "關閉");

    if ((bbstate & STAT_BOARD) || HAS_PERM(PERM_BOARD))
        prints("\n\n您目前 \033[1;33m擁有\033[m 此看板的管理權限");
    else
        prints("\n\n您目前 沒有 此看板的管理權限");

    vmsg(NULL);

    return XO_HEAD;
}

/* magicallove.081207: 切換是否為好友板 */
    static int
post_FriendSet(
    XO *xo)
{
    if (!(bbstate & STAT_BOARD))
        return XO_NONE;
    //判斷結束

    BRD *oldbrd, newbrd;
    int bno;
    bno = brd_bno(currboard);

    oldbrd = bshm->bcache + bno;
    memcpy(&newbrd, oldbrd, sizeof(BRD));

    if (vans("確定要變更看板權限？[y/N] ") != 'y')
        return XO_HEAD;

    //更改旗標
    if (newbrd.readlevel & PERM_SYSOP){
        newbrd.readlevel = 0;
        vmsg("目前為公開看板");
    }
    else{
        newbrd.readlevel = PERM_SYSOP;
        vmsg("目前為好友看板");
    }

    memcpy(oldbrd, &newbrd, sizeof(BRD));
    rec_put(FN_BRD, &newbrd, sizeof(BRD), bno);

    return XO_HEAD;
}

/* cache.090412: 切換可否推文 */
/* cache.090928 看板互斥屬性 */
    static int
post_battr_score(
    XO *xo)
{
    //判斷是否為板主
    if (!(bbstate & STAT_BOARD))
        return XO_NONE;
    //判斷結束

    BRD *oldbrd, newbrd;
    int bno;
    bno = brd_bno(currboard);

    oldbrd = bshm->bcache + bno;
    memcpy(&newbrd, oldbrd, sizeof(BRD));

    switch (vans("◎推文設定 1)推文功\能 2)噓文 3)自訂動詞 4)同ID限制 5)時間限制 [Q] "))
    {
        case '1':
            if (vans("確定要變更推文設定？[y/N] ") != 'y')
                return XO_HEAD;
            //更改旗標
            if (newbrd.battr & BRD_PRH){
                newbrd.battr &= ~BRD_PRH;
                vmsg("允許\推文");
            }
            else{
                newbrd.battr |= BRD_PRH;
                newbrd.battr &= ~BRD_PUSHDISCON;
                newbrd.battr &= ~BRD_PUSHTIME;
                newbrd.battr &= ~BRD_PUSHSNEER;
                newbrd.battr &= ~BRD_PUSHDEFINE;
                vmsg("禁止推文");
            }
            memcpy(oldbrd, &newbrd, sizeof(BRD));
            rec_put(FN_BRD, &newbrd, sizeof(BRD), bno);
            return XO_HEAD;

        case '2':
            if (vans("確定要變更噓文模式？[y/N] ") != 'y')
                return XO_HEAD;
            //更改旗標
            if (newbrd.battr & BRD_PUSHSNEER){
                newbrd.battr &= ~BRD_PUSHSNEER;
                vmsg("關閉噓文模式");
            }
            else{
                newbrd.battr |= BRD_PUSHSNEER;
                newbrd.battr &= ~BRD_PUSHDEFINE;
                vmsg("開啟噓文模式");
            }
            memcpy(oldbrd, &newbrd, sizeof(BRD));
            rec_put(FN_BRD, &newbrd, sizeof(BRD), bno);
            return XO_HEAD;

        case '3':
            if (vans("確定要變更自訂推文動詞模式？[y/N] ") != 'y')
                return XO_HEAD;
            //更改旗標
            if (newbrd.battr & BRD_PUSHDEFINE){
                newbrd.battr &= ~BRD_PUSHDEFINE;
                vmsg("關閉自訂推文動詞");
            }
            else{
                newbrd.battr |= BRD_PUSHDEFINE;
                newbrd.battr &= ~BRD_PUSHSNEER;
                vmsg("開啟自訂推文動詞");
            }
            memcpy(oldbrd, &newbrd, sizeof(BRD));
            rec_put(FN_BRD, &newbrd, sizeof(BRD), bno);
            return XO_HEAD;

        case '4':
            if (vans("確定要變更ID連推限制？[y/N] ") != 'y')
                return XO_HEAD;
            //更改旗標
            if (newbrd.battr & BRD_PUSHDISCON){
                newbrd.battr &= ~BRD_PUSHDISCON;
                vmsg("同ID允許\連推");
            }
            else{
                newbrd.battr |= BRD_PUSHDISCON;
                vmsg("同ID禁止連推");
            }
            memcpy(oldbrd, &newbrd, sizeof(BRD));
            rec_put(FN_BRD, &newbrd, sizeof(BRD), bno);
            return XO_HEAD;

        case '5':
            if (vans("確定要變更時間連推限制？[y/N] ") != 'y')
                return XO_HEAD;
            //更改旗標
            if (newbrd.battr & BRD_PUSHTIME){
                newbrd.battr &= ~BRD_PUSHTIME;
                vmsg("允許\快速連推");
            }
            else{
                newbrd.battr |= BRD_PUSHTIME;
                vmsg("禁止快速連推");
            }
            memcpy(oldbrd, &newbrd, sizeof(BRD));
            rec_put(FN_BRD, &newbrd, sizeof(BRD), bno);
            return XO_HEAD;

        default:
            return XO_HEAD;
    }
}

/* cache.100917: RSS 設定功能 */
/* cache.090928: 看板唯讀, 作者修文, 轉文設定 */
/* cache.090928: 看板互斥屬性 */
    static int
post_rule(
    XO *xo)
{
    //判斷是否為板主
    if (!(bbstate & STAT_BOARD))
        return XO_NONE;
    //判斷結束

    BRD *oldbrd, newbrd;
    int bno;
    bno = brd_bno(currboard);

    oldbrd = bshm->bcache + bno;
    memcpy(&newbrd, oldbrd, sizeof(BRD));

    switch (vans("◎看板設定 1)關板唯讀 2)作者修文 3)轉錄文章 4)禁注音文 5)RSS功\能 [Q] "))
    {
        case '1':
            if (vans("確定要變更看板唯讀設定？[y/N] ") != 'y')
                return XO_HEAD;
            //更改旗標
            if (newbrd.battr & BRD_NOREPLY){
                newbrd.battr &= ~BRD_NOREPLY;
                vmsg("取消唯讀");
            }
            else{
                newbrd.battr |= BRD_NOREPLY;
                newbrd.battr |= BRD_PRH;
                newbrd.battr &= ~BRD_PUSHDISCON;
                newbrd.battr &= ~BRD_PUSHTIME;
                newbrd.battr &= ~BRD_PUSHSNEER;
                newbrd.battr &= ~BRD_PUSHDEFINE;
                vmsg("看板唯讀 - 禁止發文回文及推文");
            }
            memcpy(oldbrd, &newbrd, sizeof(BRD));
            rec_put(FN_BRD, &newbrd, sizeof(BRD), bno);
            return XO_HEAD;

        case '2':
            if (vans("確定要變更作者修文設定？[y/N] ") != 'y')
                return XO_HEAD;
            //更改旗標
            if (newbrd.battr & BRD_MODIFY){
                newbrd.battr &= ~BRD_MODIFY;
                vmsg("允許\作者修文");
            }
            else{
                newbrd.battr |= BRD_MODIFY;
                vmsg("禁止作者修文");
            }
            memcpy(oldbrd, &newbrd, sizeof(BRD));
            rec_put(FN_BRD, &newbrd, sizeof(BRD), bno);
            return XO_HEAD;

        case '3':
            if (vans("確定要變更轉錄文章設定？[y/N] ") != 'y')
                return XO_HEAD;
            //更改旗標
            if (newbrd.battr & BRD_NOFORWARD){
                newbrd.battr &= ~BRD_NOFORWARD;
                vmsg("允許\轉錄文章");
            }
            else{
                newbrd.battr |= BRD_NOFORWARD;
                vmsg("禁止轉錄文章");
            }
            memcpy(oldbrd, &newbrd, sizeof(BRD));
            rec_put(FN_BRD, &newbrd, sizeof(BRD), bno);
            return XO_HEAD;

        case '4':
            if (vans("確定要變更注音文限制設定？[y/N] ") != 'y')
                return XO_HEAD;
            //更改旗標
            if (newbrd.battr & BRD_NOPHONETIC){
                newbrd.battr &= ~BRD_NOPHONETIC;
                vmsg("允許\注音文");
            }
            else{
                newbrd.battr |= BRD_NOPHONETIC;
                vmsg("禁止注音文");
            }
            memcpy(oldbrd, &newbrd, sizeof(BRD));
            rec_put(FN_BRD, &newbrd, sizeof(BRD), bno);
            return XO_HEAD;

        case '5':
            if (vans("確定要變更看板RSS設定？[y/N] ") != 'y')
              return XO_HEAD;
            //更改旗標
            if (newbrd.battr & BRD_RSS){
              newbrd.battr &= ~BRD_RSS;
              vmsg("關閉RSS功\能");
            }
            else{
              newbrd.battr |= BRD_RSS;
              vmsg("開啟RSS功\能");
            }
            memcpy(oldbrd, &newbrd, sizeof(BRD));
            rec_put(FN_BRD, &newbrd, sizeof(BRD), bno);
            return XO_HEAD;

        default:
            return XO_HEAD;
    }
}

    static int
post_battr_threshold(
    XO *xo)
{
    int ans, echo, num;
    BRD *oldbrd, newbrd;
    THRESHOLD th;
    char fpath[64], buf[80];

    int bno;
    bno = brd_bno(currboard);

    oldbrd = bshm->bcache + bno;
    memcpy(&newbrd, oldbrd, sizeof(BRD));

    brd_fpath(fpath, newbrd.brdname, FN_THRESHOLD);

    switch (ans = vans("◎ 發文門檻限制 1)不限制門檻 2)限制門檻 [Q] "))
    {
        case '1':
            newbrd.battr &= ~BRD_THRESHOLD;
            break;

        case '2':
            newbrd.battr |= BRD_THRESHOLD;

            echo = rec_get(fpath, &th, sizeof(THRESHOLD), 0) ? DOECHO : GCARRY;

            if (echo & GCARRY)
                sprintf(buf, "%d", th.age);
            if (!vget(b_lines, 0, "請輸入發文門檻－註冊幾天以上？", buf, 4, echo))
                return XO_HEAD;
            if ((num = atoi(buf)) < 0)
                return XO_HEAD;
            th.age = num;

            if (echo & GCARRY)
                sprintf(buf, "%d", th.numlogins);
            if (!vget(b_lines, 0, "請輸入發文門檻－登入幾次以上？", buf, 4, echo))
                return XO_HEAD;
            if ((num = atoi(buf)) < 0)
                return XO_HEAD;
            th.numlogins = num;

            if (echo & GCARRY)
                sprintf(buf, "%d", th.numposts);
            if (!vget(b_lines, 0, "請輸入發文門檻－發文幾篇以上？", buf, 4, echo))
                return XO_HEAD;
            if ((num = atoi(buf)) < 0)
                return XO_HEAD;
            th.numposts = num;

            if (echo & GCARRY)
                sprintf(buf, "%d", th.point2);
            if (!vget(b_lines, 0, "請輸入發文門檻－劣文幾篇以下？", buf, 4, echo))
                return XO_HEAD;
            if ((num = atoi(buf)) < 0)
                return XO_HEAD;
            th.point2 = num;

            if (th.age <= 0 && th.numlogins <= 0 && th.numposts <= 0 && th.point2 <=0)
                return XO_HEAD;

            break;

        default:
            return XO_HEAD;
    }

    if ((memcmp(&newbrd, oldbrd, sizeof(BRD)) || (ans == '2')) &&
            vans(msg_sure_ny) == 'y')
    {
        memcpy(oldbrd, &newbrd, sizeof(BRD));
        rec_put(FN_BRD, &newbrd, sizeof(BRD), bno);

        if (ans == '1')
            unlink(fpath);
        else /* if (ans == '2') */
            rec_put(fpath, &th, sizeof(THRESHOLD), 0);
    }

    return XO_HEAD;
}

static int
post_usies_BMlog(
    XO *xo)
{
    char fpath[64];

    if (HAVE_PERM(PERM_ADMIN))
    {
        brd_fpath(fpath, currboard, "usies");
        if (more(fpath, (char *) -1) >= 0 &&
                vans("請問是否刪除這些看板閱\讀記錄(Y/N)？[N] ") == 'y')
            unlink(fpath);
    }
    else
        pmsg2("目前不提供查詢");

    return XO_HEAD;
}

    int
post_manage(
    XO *xo)
{
    char re;

    char *menu[] =
    {
        "BQ",
        "Title   修改看板主題",
        "Memo    編輯進板畫面",
        "Post    編輯發文公告",
        "Banmail 看板檔信設定",
        "Close   看板發表設定",
#  ifdef HAVE_RECOMMEND
        "Score   看板推文設定",
#  endif

#  ifdef HAVE_MODERATED_BOARD
        "Level   看板屬性設定",
        "OPal    板友水桶設定",
#  endif
        "ZLevel  發文門檻設定",
        "Usies   看板閱\讀記錄",
        "Quit    離開選單",
        NULL
    };

    if (!(bbstate & STAT_BOARD))
    {
        vmsg(NULL);
        return XO_HEAD;
    }

#ifndef M3_USE_PFTERM
    grayout(GRAYOUT_DARK);
#endif

    switch (re = popupmenu_ans2(menu, "板主管理", 3, 20))
    {
        case 't':
            return post_brdtitle(xo);

        case 'm':
            return post_memo_edit(xo);

        case 'p':
            return post_post(xo);

        case 'b':
            return post_ban_mail(xo);

        case 'c':
            return post_rule(xo);

#ifdef HAVE_RECOMMEND
        case 's':
            return post_battr_score(xo);
#endif
#ifdef HAVE_MODERATED_BOARD
        case 'l':
            return post_FriendSet(xo);

        case 'o':
            return XoBM(xo);
#endif

        case 'z':
            return post_battr_threshold(xo);

        case 'u':
            return post_usies_BMlog(xo);

        case 'q':
            return XO_HEAD;
    }
    return XO_HEAD;
}

    static int
post_aid(
    XO *xo)
{
    char *tag, *query, aid[9];
    int currpos, pos, max, match = 0;
    HDR *hdr;

    /* 保存目前所在的位置 */
    currpos = xo->pos;
    /* 紀錄看板文章總數；新貼文AID是未知的不可能在其中 */
    max = xo->max;

    /* 若沒有文章或其他例外狀況 */
    if (max <= 0)
        return XO_FOOT;

    /* 請求使用者輸入文章代碼(AID) */
    if (!vget(b_lines, 0, "請輸入文章代碼(AID)： #", aid, sizeof(aid), DOECHO))
        return XO_FOOT;
    query = aid;

    for (pos = 0; pos < max; pos++)
    {
        xo->pos = pos;
        xo_load(xo, sizeof(HDR));
        /* 設定HDR資訊 */
        hdr = (HDR *) xo_pool + (xo->pos - xo->top);
        tag = hdr->xname;
        /* 若找到對應的文章，則設定match並跳出 */
        if (!strcmp(query, tag))
        {
            match = 1;
            break;
        }
    }

    /* 沒找到則恢復xo->pos並顯示提示文字 */
    if (!match)
    {
        zmsg("找不到文章，是不是找錯看板了呢？");
        xo->pos = currpos;  /* 恢復xo->pos紀錄 */
    }

    return post_load(xo);
}

int
post_write(                  /* 丟線上作者熱訊 */
    XO *xo)
{
    if (HAS_PERM(PERM_PAGE))
    {
        HDR *fhdr, mhdr;
        UTMP *up;

        fhdr = (HDR *) xo_pool + (xo->pos - xo->top);
        mhdr = *fhdr;

        if ((up = utmp_check(mhdr.owner)) && can_message(up))
/*      if ((up = utmp_check(mhdr.owner)) && can_override(up))*/
        {
            BMW bmw;
            char buf[20];

            sprintf(buf, "★[%s]", up->userid);
            bmw_edit(up, buf, &bmw, 0);
        }
    }
    return XO_NONE;
}

    static int
post_help(
    XO *xo)
{
    film_out(FILM_BOARD, -1);
    return post_head(xo);
}


    static int
post_spam(
    XO *xo)
{
    HDR *hdr;
    char *dir, fpath[80];
    char msg[128];


    if (!supervisor)
        return XO_NONE;

    dir = xo->dir;
    hdr = (HDR *) xo_pool + xo->pos - xo->top;
    hdr_fpath(fpath, dir, hdr);

    sprintf(msg, "%s\n", fpath);
    f_cat(FN_SPAMPATH_LOG, msg);

    vmsg(fpath);

    return XO_FOOT;
}



KeyFunc post_cb[] =
{
    {XO_INIT, post_init},
    {XO_LOAD, post_load},
    {XO_HEAD, post_head},
    {XO_BODY, post_body},

    {'B', post_manage},
    {'r', post_browse},
    {'s', post_switch},
    {KEY_TAB, post_gem},
    {'z', post_gem},
    {'u', post_undelete},
    {'y', post_reply},
    {'d', post_delete},
    {'v', post_visit},
    {'q', post_state},
    {'S', post_complete}, //板主處理標記
    {'w', post_write},
    {Ctrl('W'), post_spam},
    {'e', post_expire},
    {'U', post_unexpire},
    {'#', post_aid},              /* cache.090612: 以文章代碼(AID)快速尋文 */
    {'i', post_showBRD_setting},  /* cache.081122:看板資訊顯示 */
    {Ctrl('P'), post_add},
    {Ctrl('N'), post_clean_delete},
#ifdef HAVE_MULTI_CROSSPOST
    {Ctrl('X'), post_xcross},
#endif
    {'x', post_cross},
    {Ctrl('Q'), xo_uquery_lite},
    //  {'I', xo_usetup}, /* cache.081122: 有些人會忘記設禁止查詢資料, 建議關閉保護隱私 */
#if 1 /* Thor.981120: 暫時取消, 防誤用 */
    /* lkchu.981201: 沒有 'D' 很不習慣 :p */
    {'D', xo_delete},
#endif

#ifdef HAVE_TERMINATOR
    {'Z', post_cross_terminator},
#endif

    {'t', post_tag},
    {'l', post_lock},

    {'E', post_edit},
    {'T', post_title},
    {'m', post_mark},

#ifdef  HAVE_RECOMMEND
    {'X', post_recommend},
    {'%', post_recommend},           /* r2.20170802: 與 itoc 版熱鍵通用 */
    //  {'o', post_recommend_options},
    {'o' | XO_DL, (int (*)(XO *xo))"bin/cleanrecommend.so:clean"},
    {Ctrl('S'), post_resetscore},         /* cache.090416: 推文設定 */
#endif

    {'R' | XO_DL, (int (*)(XO *xo))"bin/vote.so:vote_result"},
    {'V' | XO_DL, (int (*)(XO *xo))"bin/vote.so:XoVote"},

    {'b', post_memo},

#ifdef HAVE_MODERATED_BOARD
    {Ctrl('G'), XoBM},
#endif

#ifdef XZ_XPOST
    {'/', XoXpost},                     /* Thor: for XoXpost */
    {'~', XoXpost},                     /* Thor: for XoXpost */
#endif

#ifdef HAVE_POST_BOTTOM
    {'_', post_bottom},
#endif

    {'h', post_help}
};


#ifdef XZ_XPOST
/*------------------------------------------------------------------------
  Thor.0509: 新的 文章搜尋模式
  可指定一keyword, 列出所有keyword相關之文章列表

  在 tmp/ 下開 xpost.{pid} 作為 folder, 另建一map陣列, 用作與原post作map
  記載該文章是在原post的何處, 如此可作 mark, gem, edit, title等功能,
  且能離開時回至對應文章處
  <以上想法 obsolete...>

  Thor.0510:
  建立文章討論串, like tin, 將文章串 index 放入 memory中,
  不使用 thread, 因為 thread要用 folder檔...

  分為兩種Mode, Title & post list

  但考慮提供簡化的 上下鍵移動..

  O->O->O->...
  |  |  |
  o  o  o
  |  |  |

  index含field {next, text} 均為int, 配置也用 int
  第一層 sorted by title, 插入時用 binary search
  且 MMAP only, 第一層顯示 # and +

  不提供任何刪除動作, 避免混亂

  Thor.980911: 考慮提供刪除指令, 方便版主
  -------------------------------------------------------------------------*/
#if 0
extern XO *xpost_xo;            /* Thor: dynamic programming for variable dir
                             * name */
extern XO *ypost_xo;
#endif


#define MSG_XYPOST      "[串接模式]標題關鍵字:"
#define MSG_XY_NONE     "空無一物"


typedef struct
{
    char *subject;
    int first;
    int last;
    time_t chrono;
}      Chain;                   /* Thor: negative is end */



    static int
chain_cmp(
    Chain *a,
    Chain *b)
{
    return a->chrono - b->chrono;
}


static int *xypostI;


/* Thor: first ypost pos in ypost_xo.key */

static int comebackPos;

/* Thor: first xpost pos in xpost_xo.key */

static char xypostKeyword[30];


/* -1 to find length, otherwise return index */


    static int
XoXpost(                        /* Thor: call from post_cb */
    XO *xo)
{
    int *plist, *xlist, fsize, max, locus, sum, i, m, n;
    Chain *chain;
    char *fimage, *key=NULL, author[30], buf[30];
    HDR *head, *tail;
    int filter_author=0, filter_title=0, mode;
    XO *xt;

    if ((max = xo->max) <= 0) /* Thor.980911: 註解: 以防萬一 */
        return XO_FOOT;

    if (xz[XZ_XPOST - XO_ZONE].xo)
    {
        vmsg("你已經使用了串接模式");
        return XO_FOOT;
    }

    /* input condition */
    /* 090928.cache: 直接進入串接模式 */
    //  mode = vans("◎ 0)串接 1)新文章 2)LocalPost [0]：") - '0';
    //  if (mode > 2 || mode < 0)
    mode = 0;

    if (!mode)
    {
        key = xypostKeyword;
        filter_title = vget(b_lines, 0, MSG_XYPOST, key, sizeof(xypostKeyword), GCARRY);
        str_lower(buf, key);
        key = buf;

        if ((filter_author = vget(b_lines, 0, "[串接模式]作者：", author, 30, DOECHO)))
        {
            filter_author = strlen(author);
            str_lower(author, author);
        }
    }

    if (!(filter_title || filter_author || mode))
        return XO_HEAD;

    /* build index according to input condition */

    fimage = f_map(xo->dir, &fsize);

    if (fimage == (char *) -1)
    {
        vmsg("目前無法開啟索引檔");
        return XO_FOOT;
    }

    if ((xlist = xypostI)) /* Thor.980911: 註解: 怕重覆進入時, 浪費記憶體 */
        free(xlist);

    /* allocate index memory, remember free first */

    /* Thor.990113: 怕問title, author的瞬間又有人post */
    max = fsize / sizeof(HDR);

    plist = (int *) malloc(sizeof(int) * max);
    chain = (Chain *) malloc(sizeof(Chain) * max);

    max = sum = 0;

    head = (HDR *) fimage;
    tail = (HDR *) (fimage + fsize);

    locus = -1;
    do
    {
        int left, right, mid;
        char *title = NULL;

        locus++;
        if (head->xmode & (POST_CANCEL | POST_DELETE | POST_MDELETE))
            continue;                   /* Thor.0701: 跳過看不到的文章 */

        if ((head->xmode & POST_LOCK) && !(HAS_PERM(PERM_SYSOP| PERM_BOARD)||bbstate & STAT_BOARD))
            continue;

        /* check author */

        /* Thor.981109: 特別注意, author是從頭match, 不是substr match, 為降低load */
        if (!mode)
        {
            if (filter_author && str_ncmp(head->owner, author, filter_author))
                continue;

            /* check condition */

            title = head->title;

            if (STR4(title) == STR4(STR_REPLY)) /* Thor.980911: 先把 Re: 除外 */
                title += 4;

            if (*key && !str_str(title, key))
                continue;
        }
        else if (mode == 1)
        {
            title = head->title;
            if (STR4(title) == STR4(STR_REPLY))
                continue;
        }
        else
        {
            if (strchr(head->owner, '.'))
                continue;
        }

        sum++;

        /* check if in table, binary check */

        left = 0;
        right = max - 1;
        for (;;)
        {
            int cmp;
            Chain *cptr;

            if (left > right)
            {
                for (i = max; i > left; i--)
                    chain[i] = chain[i - 1];

                cptr = &chain[left];
                cptr->subject = title;
                cptr->first = cptr->last = locus;
                cptr->chrono = head->chrono;
                max++;
                break;
            }

            mid = (left + right) >> 1;
            cptr = &chain[mid];
            cmp = strcmp(title, cptr->subject);

            if (!cmp)
            {
                plist[cptr->last] = locus;
                cptr->last = locus;
                break;
            }

            if (cmp < 0)
                right = mid - 1;
            else
                left = mid + 1;
        }
    } while (++head < tail);
    munmap(fimage, fsize);

    if (max <= 0)
    {
        free(chain);
        free(plist);
        vmsg(MSG_XY_NONE);
        return XO_FOOT;
    }

    if (max > 1)
        xsort(chain, max, sizeof(Chain), chain_cmp);

    xypostI = xlist = (int *) malloc(sizeof(int) * sum);

    i = locus = 0;
    do
    {
        xlist[locus++] = n = chain[i].first;
        m = chain[i].last;

        while (n != m)
        {
            xlist[locus++] = n = plist[n];
        }

    } while (++i < max);

    free(chain);
    free(plist);

    /* build XO for xpost_xo */

    if ((xt = xz[XZ_XPOST - XO_ZONE].xo))
        free(xt);

    comebackPos = xo->pos;      /* Thor: record pos, future use */
    xz[XZ_XPOST - XO_ZONE].xo = xt = xo_new(xo->dir);
    xt->pos = 0;
    xt->max = sum;
    xt->xyz = xo->xyz;
    xt->key = XZ_XPOST;

    xover(XZ_XPOST);

    /* set xo->pos for new location */

    xo->pos = comebackPos;

    /* free xpost_xo */

    if ((xt = xz[XZ_XPOST - XO_ZONE].xo))
    {
        free(xt);
        xz[XZ_XPOST - XO_ZONE].xo = NULL;
    }

    /* free index memory, remember check free pointer */

    if ((xlist = xypostI))
    {
        free(xlist);
        xypostI = NULL;
    }

    return XO_INIT;
}


#if 0
/* Thor.980911: 共用 post_body() 即可*/
    static int
xpost_body(
    XO *xo)
{
    HDR *fhdr;
    int num, max, tail;

    max = xo->max;
#if 0
    if (max <= 0)
    { /* Thor.980911: 註解: 以防萬一用 */
        vmsg(MSG_XY_NONE);
        return XO_QUIT;
    }
#endif

    fhdr = (HDR *) xo_pool;
    num = xo->top;
    tail = num + XO_TALL;
    if (max > tail)
        max = tail;

    move(3, 0);
    do
    {
        post_item(++num, fhdr++);
    } while (num < max);

    clrtobot();
    return XO_NONE;
}

#endif

    static int
xpost_head(
    XO *xo)
{
    vs_head("主題串列" /* currBM */, xo->xyz);
    outs(MSG_XYPOST);
    if (*xypostKeyword)
        outs(xypostKeyword);

    outs(
            "\033[30;47m  編號   日 期  作  者       文  章  標  題                                   \033[m");

    /* return xpost_body(xo); */
    return post_body(xo); /* Thor.980911: 共用即可 */
}


    static void
xypost_pick(
    XO *xo)
{
    int *xyp, fsize, pos, max, top;
    HDR *fimage, *hdr;

    fimage = (HDR *) f_map(xo->dir, &fsize);
    if (fimage == (HDR *) - 1)
        return;

    hdr = (HDR *) xo_pool;
    xyp = xypostI;

    pos = xo->pos;
    xo->top = top = (pos / XO_TALL) * XO_TALL;
    max = xo->max;
    pos = top + XO_TALL;
    if (max > pos)
        max = pos;

    do
    {
        pos = xyp[top++];
        *hdr = fimage[pos];
        hdr->xid = pos;
        hdr++;
    } while (top < max);

    munmap((void *)fimage, fsize);
}


    static int
xpost_init(
    XO *xo)
{
    /* load into pool */

    xypost_pick(xo);

    return xpost_head(xo);
}


    static int
xpost_load(
    XO *xo)
{
    /* load into pool */

    xypost_pick(xo);

    /* return xpost_body(xo); */
    return post_body(xo); /* Thor.980911: 共用即可 */
}


    static int
xpost_help(
    XO *xo)
{
    film_out(FILM_BOARD, -1);
    return xpost_head(xo);
}


/* Thor.0509: 要想辦法禁用 ctrl('D') */


    static int
xpost_browse(
    XO *xo)
{
    HDR *hdr;
    int cmd, chrono, xmode;
    char *dir, fpath[64], *board;

    int key;

    cmd = XO_NONE;
    dir = xo->dir;
    board=currboard;

    for (;;)
    {
        hdr = (HDR *) xo_pool + (xo->pos - xo->top);
        xmode = hdr->xmode;
        if (xmode & (POST_CANCEL | POST_DELETE | POST_MDELETE))
            break;

#ifdef  HAVE_USER_MODIFY
        if (xmode & POST_CURMODIFY)
        {
            vmsg("此文章正在修改中!!");
            break;
        }
#endif

        if ((hdr->xmode & POST_LOCK) && !(HAS_PERM(PERM_SYSOP | PERM_BOARD) || (bbstate & STAT_BOARD) || !strcmp(hdr->owner, cuser.userid)))
            break;

        /* cache.20130407: for preventing bot to get data */
        char desc[128];
        time_t now;
        time(&now);

        snprintf(desc, sizeof(desc), "%s %s %s %d %s\n", Atime(&now), cuser.userid, currboard, hdr->chrono, ipv4addr);
        f_cat(FN_BROWSE_LOG, desc);

        hdr_fpath(fpath, dir, hdr);

        /* Thor.990204: 為考慮more 傳回值 */
        //    if ((key = more(fpath, MSG_POST)) == -1)
        //      break;

        comebackPos = hdr->xid;
        /* Thor.980911: 從串接模式回來時要回到看過的那篇文章位置 */

        cmd = XO_HEAD;
        if (key == -2)
            return XO_INIT;

        chrono = hdr->chrono;
        if (brh_unread(chrono))
        {
            int prev, next, pos;
            char *dir;
            HDR buf;

            dir = xo->dir;
            pos = hdr->xid;

            if (!rec_get(dir, &buf, sizeof(HDR), pos - 1))
                prev = buf.chrono;
            else
                prev = chrono;

            if (!rec_get(dir, &buf, sizeof(HDR), pos + 1))
                next = buf.chrono;
            else
                next = chrono;

            brh_add(prev, hdr->chrono, next);
        }

        strcpy(currtitle, str_ttl(hdr->title));

        // Thor.990204: 為考慮more 傳回值
        if ((key = more(fpath, FOOTER_POST)) < 0)
            break;

        /* Thor.990204: 為考慮more 傳回值 */
        if (!key)
            key = vkey();

        switch (key)
        {
            case Ctrl('U'):
                every_U();
                continue;
            case Ctrl('B'):
                every_B();
                continue;
#if 0
            case 'F':/*float.101109: 修正鎖文可轉寄*/
                if ((hdr->xmode & POST_LOCK) && !HAS_PREM(PERM_SYSOP))
                {
                    vmsg("鎖定文章不能轉寄");
                    return XO_NONE;
                }
                else
                {
                    return xo_forward(xo);
                }
                break;
#endif
            case ']':  /* Thor.990204: 有時想用]看後面的文章 */
            case 'j':  /* Thor.990204: 有時想用j看後面的文章 */
            case ' ':
                {
                    int pos = xo->pos + 1;

                    /* Thor.980727: 修正看過頭的bug */

                    if (pos >= xo->max)
                        return cmd;

                    xo->pos = pos;

                    if (pos >= xo->top + XO_TALL)
                        xypost_pick(xo);

                    continue;
                }

            case 'y':
            case 'r':
                if (bbstate & STAT_POST)
                {
                    strcpy(quote_file, fpath);
                    if (do_reply(hdr) == XO_INIT)       /* 有成功地 post 出去了 */
                        return xpost_init(xo);
                }
                break;

            case 'm':
                if ((bbstate & STAT_BOARD) && !(xmode & POST_MARKED))
                {
                    hdr->xmode = xmode | POST_MARKED;
                    rec_put(dir, hdr, sizeof(HDR), hdr->xid);
                }
                break;

#ifdef HAVE_RECOMMEND
            case 'p':
                post_recommend(xo);
                return xpost_init(xo);
#endif

        }
        break;
    }

    return XO_INIT;
}


KeyFunc xpost_cb[] =
{
    {XO_INIT, xpost_init},
    {XO_LOAD, xpost_load},
    {XO_HEAD, xpost_head},
#if 0
    {XO_BODY, xpost_body},
#endif
    {XO_BODY, post_body}, /* Thor.980911: 共用即可 */

    {'r', xpost_browse},
    {'y', post_reply},
    {'t', post_tag},
    {'m', post_mark},

    {'d', post_delete},  /* Thor.980911: 方便版主*/

    {Ctrl('P'), post_add},
    {Ctrl('Q'), xo_uquery},
    {'q', post_spam},
    {'I', xo_usetup},
#ifdef HAVE_MULTI_CROSSPOST
    {Ctrl('X'), post_xcross},
#endif
    {'x', post_cross},

    {'h', xpost_help}
};
#endif

