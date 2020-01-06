/*-------------------------------------------------------*/
/* newboard.c   ( YZU_CSE WindTop BBS )                  */
/*-------------------------------------------------------*/
/* target : newboard routines                            */
/* create : 2000/01/02                                   */
/* update : NULL                                         */
/*-------------------------------------------------------*/

#include "bbs.h"

#undef  TEST_COSIGN


static int nbrd_add(XO *xo);
static int nbrd_body(XO *xo);
static int nbrd_head(XO *xo);

#define S_PART          "-----------------------------------------------------------\n"
#define S_AGREE_START   ">------------------- [   贊成開始   ] --------------------<\n"
#define S_AGREE_END     ">------------------- [   贊成結束   ] --------------------<\n"
#define S_ASSIST_START  ">------------------- [   反對開始   ] --------------------<\n"
#define S_ASSIST_END    ">------------------- [   反對結束   ] --------------------<\n"




typedef struct
{
    char email[60];
} LOG;  /* DISKDATA(raw) */


static char
nbrd_attr(
const NBRD *nbrd)
{
    if (nbrd->mode & (NBRD_REJECT | NBRD_STOP))
        return 'R';
    else if (nbrd->mode & NBRD_CLOSE)
        return 'C';
    else if (nbrd->mode & NBRD_OPEN)
        return 'O';
    else if (nbrd->mode & NBRD_OK)
        return 'Y';
    else if (nbrd->mode & NBRD_START)
        return ' ';
    else
        return 'N';
}

static int
nbrd_stamp(
const char *folder,
NBRD *nbrd,
char *fpath)
{
    char *fname, *family = NULL;
    int rc;
    int token;

    fname = fpath;
    while ((rc = *folder++))
    {
        *fname++ = rc;
        if (rc == '/')
            family = fname;
    }
    fname = family;
    *family++ = '@';

    token = time(0);

    archiv32(token, family);

    rc = open(fpath, O_WRONLY | O_CREAT | O_EXCL, 0600);
    memset(nbrd, 0, sizeof(NBRD));
    nbrd->btime = token;
    str_stamp(nbrd->date, &nbrd->btime);
    strcpy(nbrd->xname, fname);
    return rc;
}

static void
nbrd_fpath(
char *fpath,
const char *folder,
const NBRD *nbrd)
{
    char *str = NULL;
    int cc;

    while ((cc = *folder++))
    {
        *fpath++ = cc;
        if (cc == '/')
            str = fpath;
    }
    strcpy(str, nbrd->xname);
}

static int
nbrd_init(
XO *xo)
{
    xo_load(xo, sizeof(NBRD));
    return nbrd_head(xo);
}


static int
nbrd_load(
XO *xo)
{
    xo_load(xo, sizeof(NBRD));
    return nbrd_body(xo);
}


static void
nbrd_item(
int num,
const NBRD *nbrd)
{
    if (nbrd->mode & NBRD_NBRD)
        prints("%6d %c %-5s %-13s %-13s:%-*.*s\n", num, nbrd_attr(nbrd), nbrd->date + 3, nbrd->owner, nbrd->brdname, d_cols + 36, d_cols + 36, nbrd->title);
    else if (nbrd->mode & NBRD_CANCEL)
        prints("%6d %c %-5s %-13s 廢除 %s 版版主\n", num, nbrd_attr(nbrd), nbrd->date + 3, nbrd->owner, nbrd->brdname);
    else
        prints("%6d %c %-5s %-13s %-*.*s\n", num, nbrd_attr(nbrd), nbrd->date + 3, nbrd->owner, d_cols + 50, d_cols + 50, nbrd->title);
}


static int
nbrd_body(
XO *xo)
{
    const NBRD *nbrd;
    int num, max, tail;

    max = xo->max;
    if (max <= 0)
    {
        if (HAS_PERM(PERM_VALID))
        {
            if (vans("要新增連署項目嗎？(y/N) [N] ") == 'y')
                return nbrd_add(xo);
        }
        else
        {
            vmsg("尚無連署活動");
        }
        return XO_QUIT;
    }

    nbrd = (const NBRD *) xo_pool;
    num = xo->top;
    tail = num + XO_TALL;
    max = BMIN(max, tail);

    move(3, 0);
    do
    {
        nbrd_item(++num, nbrd++);
    }
    while (num < max);

    clrtobot();
    return XO_NONE;
}


static int
nbrd_head(
XO *xo)
{
    clear();
    vs_head("連署系統", NULL);
    prints(NECK_NBRD, d_cols, "");
    return nbrd_body(xo);
}

static int
nbrd_find(
XO *xo,
const char *ptr,
int mode)
{
    int pos = 0, fd;
    NBRD nbrd;
    fd = open(xo->dir, O_RDONLY);
    while (fd >= 0)
    {
        lseek(fd, (off_t)(sizeof(NBRD) * pos), SEEK_SET);
        if (read(fd, &nbrd, sizeof(NBRD)) == sizeof(NBRD))
        {
            if (!str_cmp(nbrd.brdname, ptr) && !(nbrd.mode & (NBRD_REJECT | NBRD_STOP | NBRD_CLOSE | NBRD_OPEN)) && (nbrd.mode & mode))
            {
                close(fd);
                return 1;
            }
            pos++;
        }
        else
        {
            close(fd);
            break;
        }
    }
    return 0;
}


static int
nbrd_add(
XO *xo)
{
    int fd, mode, days = 0, numbers = 0;
    char *dir, fpath[80], buf[IDLEN +1];
    char buf1[49], tmp[10], path[80];
    FILE *fp;
    NBRD nbrd;
    time_t etime;
    time_t now;

    if (bbsothermode & OTHERSTAT_EDITING)
    {
        vmsg("你還有檔案還沒編完哦！");
        return XO_FOOT;
    }



    if (!HAS_PERM(PERM_POST))
    {
        return XO_NONE;
    }

    sprintf(path, "newboard/%s", cuser.userid);
    mode = vans("連署模式 1)開新版 2)廢版主 3)其他 0)取消 [0] :") - '0';

    if (mode == 1)
    {
        more(FN_NEWBOARD_HELP, NULL);
        if (!vget(b_lines, 0, "版名：", buf, sizeof(nbrd.brdname), DOECHO))
            return XO_INIT;
        if (brd_bno(buf) >= 0)
        {
            vmsg("已有此版！");
            return XO_INIT;
        }
        if (nbrd_find(xo, buf, NBRD_NBRD))
        {
            vmsg("正在連署中！");
            return XO_FOOT;
        }
        if (!vget(b_lines, 0, "看板主題：", buf1, sizeof(nbrd.title), DOECHO))
            return XO_FOOT;
    }
    else if (mode == 2)
    {
        if (!vget(b_lines, 0, "版名：", buf, sizeof(nbrd.brdname), DOECHO))
            return XO_INIT;
        if (brd_bno(buf) < 0)
        {
            vmsg("無此版！");
            return XO_INIT;
        }
        if (nbrd_find(xo, buf, NBRD_CANCEL))
        {
            vmsg("正在連署中！");
            return XO_FOOT;
        }
        strcpy(buf, (bshm->bcache + brd_bno(buf))->brdname);
    }
    else if (mode == 3)
    {
        if (!vget(b_lines, 0, "連署主題：", buf1, sizeof(nbrd.title), DOECHO))
            return XO_FOOT;
        if (!vget(b_lines, 0, "連署天數：", tmp, 5, DOECHO))
            return XO_FOOT;
        days = atoi(tmp);
        if (days > 30 || days < 1)
            return XO_FOOT;
        if (!vget(b_lines, 0, "連署人數：", tmp, 6, DOECHO))
            return XO_FOOT;
        numbers = atoi(tmp);
        if (numbers > 500 || numbers < 1)
            return XO_FOOT;
    }
    else
        return 0;


    dir = xo->dir;
    fd = nbrd_stamp(dir, &nbrd, fpath);
    if (fd < 0)
        return XO_HEAD;
    close(fd);

    now = time(0);


    if (mode == 1)
    {
        vmsg("開始編輯 [看板說明與版主抱負]");
        fd = vedit(path, 0);
        if (fd)
        {
            unlink(path);
            unlink(fpath);
            vmsg("取消連署");
            return XO_HEAD;
        }
        nbrd.etime = nbrd.btime + NBRD_DAYS * 86400;
        fp = fopen(fpath, "a+");
        fprintf(fp, "作者: SYSOP (" SYSOPNICK ") 站內: NewBoard\n");
        fprintf(fp, "標題: %s 申請新版連署\n", buf);
        fprintf(fp, "時間: %s\n\n", ctime(&now));
        fprintf(fp, "%s", S_PART);
        fprintf(fp, "    版名：%s\n", buf);
        fprintf(fp, "    看板主題：%s\n", buf1);
        fprintf(fp, "    舉辦日期：%s\n", nbrd.date);
        fprintf(fp, "    到期天數：%d\n", NBRD_DAYS);
        fprintf(fp, "    版主名稱：%s\n", cuser.userid);
        fprintf(fp, "    電子郵件信箱：%s\n", cuser.email);
        fprintf(fp, "    需連署人數：%d\n", NBRD_MAX);
        fprintf(fp, "%s", S_PART);
        fprintf(fp, "    版主抱負：\n");
        f_suck(fp, path);
        unlink(path);
        fprintf(fp, "\n");
    }
    else if (mode == 2)
    {
        vmsg("開始編輯 [廢版主原因]");
        fd = vedit(path, 0);
        if (fd)
        {
            unlink(path);
            unlink(fpath);
            vmsg("取消連署");
            return XO_HEAD;
        }
        nbrd.etime = etime = nbrd.btime + NBRD_DAYS * 86400;
        fp = fopen(fpath, "a+");
        fprintf(fp, "作者: SYSOP (" SYSOPNICK ") 站內: NewBoard\n");
        fprintf(fp, "標題: 申請廢除 %s 板主\n", buf);
        fprintf(fp, "時間: %s\n\n", ctime(&now));
        fprintf(fp, "%s", S_PART);
        fprintf(fp, "    版名：%s\n", buf);
        fprintf(fp, "    舉辦日期：%s\n", nbrd.date);
        fprintf(fp, "    到期天數：%d\n", NBRD_DAYS);
        fprintf(fp, "    舉辦人：%s\n", cuser.userid);
        fprintf(fp, "    電子郵件信箱：%s\n", cuser.email);
        fprintf(fp, "    最少贊成人數：%d\n", NBRD_MAX_CANCEL);
        fprintf(fp, "%s", S_PART);
        fprintf(fp, "    廢版主原因：\n");
        f_suck(fp, path);
        unlink(path);
    }
    else
    {
        vmsg("開始編輯 [連署原因]");
        fd = vedit(path, 0);
        if (fd)
        {
            unlink(path);
            unlink(fpath);
            vmsg("取消連署");
            return XO_HEAD;
        }
        nbrd.etime = etime = nbrd.btime + days * 86400;
        fp = fopen(fpath, "a+");
        fprintf(fp, "作者: SYSOP (" SYSOPNICK ") 站內: NewBoard\n");
        fprintf(fp, "標題: %s 連署\n", buf1);
        fprintf(fp, "時間: %s\n\n", ctime(&now));
        fprintf(fp, "%s", S_PART);
        fprintf(fp, "    連署主題：%s\n", buf1);
        fprintf(fp, "    舉辦日期：%s\n", nbrd.date);
        fprintf(fp, "    到期天數：%d\n", days);
        str_stamp(tmp, &etime);
        fprintf(fp, "    到期日期：%s\n", tmp);
        fprintf(fp, "    版主名稱：%s\n", cuser.userid);
        fprintf(fp, "    電子郵件信箱：%s\n", cuser.email);
        fprintf(fp, "    需連署人數：%d\n", numbers);
        fprintf(fp, "%s", S_PART);
        fprintf(fp, "    連署原因：\n");
        f_suck(fp, path);
        unlink(path);
        fprintf(fp, "%s", S_PART);
        fprintf(fp, "    開始連署：\n");
        fprintf(fp, "%s", S_AGREE_START);
        fprintf(fp, "%s", S_AGREE_END);
        fprintf(fp, "%s", S_ASSIST_START);
        fprintf(fp, "%s", S_ASSIST_END);
    }


    strcpy(nbrd.title, buf1);
    strcpy(nbrd.brdname, buf);

    strcpy(nbrd.owner, cuser.userid);

    if (mode == 1)
    {
        nbrd.mode = NBRD_NBRD;
        nbrd.total = NBRD_MAX;
        vmsg("送交申請了，請等候核准吧！");
    }
    else if (mode == 2)
    {
        nbrd.mode = NBRD_CANCEL;
        nbrd.total = NBRD_MAX_CANCEL;
        vmsg("送交申請了，請等候核准吧！");
    }
    else
    {
        nbrd.mode = NBRD_OTHER | NBRD_START;
        nbrd.total = numbers;
        vmsg("開始連署了！");
    }


    rec_add(dir, &nbrd, sizeof(NBRD));
    fclose(fp);

    return XO_INIT;
}

static int
nbrd_seek(
const char *fpath,
const char *mail)
{
    LOG email;
    int pos = 0, fd;
    fd = open(fpath, O_RDONLY);
    while (fd >= 0)
    {
        lseek(fd, (off_t)(sizeof(email) * pos), SEEK_SET);
        if (read(fd, &email, sizeof(email)) == sizeof(email))
        {
            if (!strcmp(email.email, mail))
            {
                close(fd);
                return 1;
            }
            pos++;
        }
        else
        {
            close(fd);
            break;
        }
    }
    return 0;
}

static int
nbrd_join(
XO *xo)
{
    NBRD *nbrd;
    int fd, fv, lock;
    char fpath[80], buf[128], logpath[80], flocks[80];
    time_t check;
    LOG mail;

    memset(&mail, 0, sizeof(LOG));
    nbrd = (NBRD *) xo_pool + (xo->pos - xo->top);
    if (nbrd->mode & NBRD_REJECT)
    {
        vmsg("拒絕申請，資料不完整！");
        return XO_FOOT;
    }
    else if (nbrd->mode & (NBRD_CLOSE | NBRD_STOP))
    {
        vmsg("停止連署！");
        return XO_FOOT;
    }
    else if (nbrd->mode & NBRD_OPEN)
    {
        vmsg("已完成開版");
        return XO_FOOT;
    }
    else if (!(nbrd->mode & NBRD_START))
    {
        vmsg("尚未通過申請！");
        return XO_FOOT;
    }
    else if (time(0) > nbrd->etime || nbrd->mode & NBRD_STOP)
    {
        nbrd->mode = NBRD_STOP  | (nbrd->mode & NBRD_MASK);
        rec_put(xo->dir, nbrd, sizeof(NBRD), xo->pos);
        vmsg("連署已經截止了，請下次再來！");
        return XO_FOOT;
    }
    else if (nbrd->mode & NBRD_OK)
    {
        if (vmsg("已達到連署人數，是否繼續連署 [y/N]") != 'y')
            return XO_FOOT;
    }

    /* --------------------------------------------------- */
    /* 檢查是否已經連署過                                  */
    /* --------------------------------------------------- */

#define FV_SZ   (sizeof(time_t))

    usr_fpath(buf, cuser.userid, FN_NEWBOARD);
    fv = open(buf, O_RDWR | O_CREAT, 0600);
    f_exlock(fv);

    while (read(fv, &check, FV_SZ) == FV_SZ)
    {
        if (check == nbrd->btime)
        {
            f_unlock(fv);
            close(fv);
            vmsg("你已經連署過了！");
            return XO_FOOT;
        }
    }
    check = nbrd->btime;
    /* --------------------------------------------------- */
    /* 開始連署                                            */
    /* --------------------------------------------------- */

    nbrd_fpath(fpath, xo->dir, nbrd);
    sprintf(logpath, "%s.log", fpath);

    if (nbrd_seek(logpath, cuser.email))
    {
        vmsg("你已經連署過了！");
        return XO_FOOT;
    }

    more(fpath, NULL);
    sprintf(flocks, "%s.lock", fpath);
#ifdef __linux__
    lock = open(flocks, O_WRONLY | O_CREAT | O_APPEND | O_NONBLOCK, 0600);
#else
    lock = open(flocks, O_WRONLY | O_CREAT | O_APPEND | O_EXLOCK | O_NONBLOCK, 0600);
#endif
    if (lock < 0)
    {
        vmsg("有人正在連署中，請稍待！");
        return XO_HEAD;
    }

#ifdef __linux__
    flock(lock, LOCK_EX);
#endif

    while (1)
    {
        if (nbrd->mode & NBRD_NBRD)
        {
            fd = vans("要加入連署嗎 (Y)贊成 (Q)離開 [y/Q]：");
            if (fd != 'y' && fd != 'Y')
                fd = 'Q';
            break;
        }
        else
        {
            int ans;
            fd = vans("要加入連署嗎 (1)贊成 (2)反對 (Q)離開 [Q]：");
            if (fd == '1')
            {
                ans = vans("您投的是贊成票，確定嗎？ (Y)確定 (N)取消 [y/N]：");
                fd = 'y';
            }
            else if (fd == '2')
            {
                ans = vans("您投的是反對票，確定嗎？ (Y)確定 (N)取消 [y/N]：");
                fd = 'n';
            }
            else
            {
                fd = 'Q';
                break;
            }
            if (ans == 'y' || ans == 'Y')
                break;
        }
    }
    if (fd == 'y' || fd == 'Y' || fd == 'n' || fd == 'N')
    {
        FILE *fp, *fds;
        char say[128];
        char nfile[128];
        int  rmode;
        if (fd == 'y' || fd == 'Y')
            rmode = 1;
        else
            rmode = 2;

        sprintf(nfile, "%s.new", fpath);
        fp = fopen(fpath, "r+");
        fds = fopen(nfile, "w");

        rec_get(xo->dir, nbrd, sizeof(NBRD), xo->pos);
        if (fp)
        {
            while (fgets(buf, 128, fp))
            {
                if (rmode == 1)
                {
                    if (!strncmp(buf, S_AGREE_END, strlen(S_AGREE_END)))
                    {
                        nbrd->agree++;
                        break;
                    }
                }
                else
                    if (!strncmp(buf, S_ASSIST_END, strlen(S_ASSIST_END)))
                    {
                        nbrd->assist++;
                        break;
                    }
                fprintf(fds, "%s", buf);
            }
            fprintf(fds, "%3u -> %-12s: %s\n", rmode == 1 ? nbrd->agree : nbrd->assist, cuser.userid, cuser.email);
            if (vget(b_lines, 0, "我有話要說：", say, 65, DOECHO))
                fprintf(fds, "    %s : %s\n", cuser.userid, say);

            fprintf(fds, "%s", buf);
            while (fgets(buf, 128, fp))
                fprintf(fds, "%s", buf);
            fclose(fds);
            fclose(fp);
            unlink(fpath);
            f_mv(nfile, fpath);

            write(fv, &check, FV_SZ);
            if ((nbrd->mode & NBRD_OK) ? 0 : (rmode == 1 ? (--nbrd->total > 0) : 1))
            {
                sprintf(buf, "加入連署完成！尚需連署人數 %u 人。", nbrd->total);
                vmsg(buf);
            }
            else
            {
                vmsg("已達連署標準，請靜候審理！");
                nbrd->mode = NBRD_OK  | (nbrd->mode & NBRD_MASK) | NBRD_START;
            }
            strcpy(mail.email, cuser.email);
            rec_put(xo->dir, nbrd, sizeof(NBRD), xo->pos);
            rec_add(logpath, &mail, sizeof(LOG));
        }
        else
            vmsg("加入連署失敗!!");
    }

#ifdef __linux__
    flock(lock, LOCK_UN);
#endif

    close(lock);
    unlink(flocks);
    f_unlock(fv);
    close(fv);
    return XO_HEAD;
}

static int
nbrd_start(
XO *xo)
{
    NBRD *nbrd;
    char fpath[80], buf[128], tmp[10];
    time_t etime;
    if (!HAS_PERM(PERM_SYSOP | PERM_BOARD))
        return XO_NONE;


    nbrd = (NBRD *) xo_pool + (xo->pos - xo->top);
    if (nbrd->mode & ~(NBRD_MASK))
        vmsg("已通過或已停止！");
    else
    {
        nbrd_fpath(fpath, xo->dir, nbrd);
        etime = time(0) + NBRD_DAYS * 86400;

        str_stamp(tmp, &etime);
        f_cat(fpath, S_PART);
        sprintf(buf, "    開始連署：      到期日期：%s\n", tmp);
        f_cat(fpath, buf);
        f_cat(fpath, S_AGREE_START);
        f_cat(fpath, S_AGREE_END);
        if (!(nbrd->mode & NBRD_NBRD))
        {
            f_cat(fpath, S_ASSIST_START);
            f_cat(fpath, S_ASSIST_END);
        }
        nbrd->etime = etime;
        nbrd->mode = NBRD_START  | (nbrd->mode & NBRD_MASK);
        rec_put(xo->dir, nbrd, sizeof(NBRD), xo->pos);
        vmsg("申請通過");
    }
    return XO_HEAD;
}

static int
nbrd_reject(
XO *xo)
{
    NBRD *nbrd;
    char path[128], fpath[128];
    int fd;
    FILE *fp;
    if (bbsothermode & OTHERSTAT_EDITING)
    {
        vmsg("你還有檔案還沒編完哦！");
        return XO_FOOT;
    }


    nbrd = (NBRD *) xo_pool + (xo->pos - xo->top);
    if (!HAS_PERM(PERM_SYSOP | PERM_BOARD))
        return XO_NONE;

    if (nbrd->mode & ~(NBRD_MASK))
        vmsg("已通過或已停止！");
    else
    {
        usr_fpath(path, cuser.userid, "newboard.note");
        nbrd_fpath(fpath, xo->dir, nbrd);
        vmsg("請編輯拒絕連署原因");
        fd = vedit(path, 0);
        if (fd)
        {
            unlink(path);
            vmsg(MSG_CANCEL);
            return XO_HEAD;
        }

        f_cat(fpath, S_PART);
        f_cat(fpath, "    拒絕連署原因：\n\n");
        fp = fopen(fpath, "a+");
        f_suck(fp, path);
        fclose(fp);
        f_cat(fpath, S_PART);
        nbrd->mode |= NBRD_REJECT  | (nbrd->mode & NBRD_MASK);
        rec_put(xo->dir, nbrd, sizeof(NBRD), xo->pos);
        vmsg("拒絕申請");
        unlink(path);
    }
    return XO_HEAD;
}

static int
nbrd_close(
XO *xo)
{
    NBRD *nbrd;

    nbrd = (NBRD *) xo_pool + (xo->pos - xo->top);
    if (!HAS_PERM(PERM_SYSOP | PERM_BOARD))
        return XO_NONE;
    if (nbrd->mode & NBRD_OK)
        vmsg("已連署完畢，不能關閉！");
    else if (nbrd->mode & NBRD_OPEN)
        vmsg("已完成開版！");
    else if (nbrd->mode & NBRD_CLOSE)
        vmsg("已關閉連署！");
    else
    {
        nbrd->mode = NBRD_CLOSE  | (nbrd->mode & NBRD_MASK);
        rec_put(xo->dir, nbrd, sizeof(NBRD), xo->pos);
        vmsg("關閉完成");
    }
    return XO_HEAD;
}

static int
nbrd_open(
XO *xo)
{
    NBRD *nbrd;

    nbrd = (NBRD *) xo_pool + (xo->pos - xo->top);
    if (!HAS_PERM(PERM_SYSOP | PERM_BOARD))
        return XO_NONE;

    if (!(nbrd->mode & NBRD_OK))
        vmsg("尚未連署完畢，不能開版！");
    else if (nbrd->mode & NBRD_OPEN)
        vmsg("已完成開版！");
    else
    {
        nbrd->mode = NBRD_OPEN | (nbrd->mode & NBRD_MASK);
        rec_put(xo->dir, nbrd, sizeof(NBRD), xo->pos);
        vmsg("開版完成");
    }
    return XO_HEAD;
}

#ifdef  TEST_COSIGN
static int
nbrd_zero(
XO *xo)
{
    NBRD *nbrd;

    nbrd = (NBRD *) xo_pool + (xo->pos - xo->top);
    if (!HAS_PERM(PERM_SYSOP | PERM_BOARD))
        return XO_NONE;

    nbrd->mode |= NBRD_START;
    rec_put(xo->dir, nbrd, sizeof(NBRD), xo->pos);
    return XO_HEAD;
}
#endif

static int
nbrd_browse(
XO *xo)
{
    const NBRD *nbrd;
    char fpath[80];

    nbrd = (const NBRD *) xo_pool + (xo->pos - xo->top);
    nbrd_fpath(fpath, xo->dir, nbrd);
    more(fpath, NULL);
    return XO_INIT;
}

static int
nbrd_delete(
XO *xo)
{
    const NBRD *nbrd;
    char fpath[80];


    nbrd = (const NBRD *) xo_pool + (xo->pos - xo->top);
    if (strcmp(cuser.userid, nbrd->owner) && !HAS_PERM(PERM_SYSOP | PERM_BOARD))
        return XO_NONE;

    if (vans("確定刪除嗎 [y/N] :") != 'y')
        return XO_HEAD;
    nbrd_fpath(fpath, xo->dir, nbrd);
    unlink(fpath);
    strcat(fpath, ".log");
    unlink(fpath);
    rec_del(xo->dir, sizeof(NBRD), xo->pos, NULL, NULL);
    return XO_INIT;
}

static int
nbrd_cross(
XO *xo)
{
    char xboard[20], fpath[80], xfolder[80], buf[80];
    HDR xpost;
    int rc;
    FILE *fd;
    const NBRD *nbrd;

    if (!HAS_PERM(PERM_ADMIN))
        return XO_NONE;

    nbrd = (const NBRD *) xo_pool + (xo->pos - xo->top);

    if (ask_board(xboard, BRD_W_BIT,
                  "\n\n\x1b[1;33m請挑選適當的看板，切勿轉貼超過三板。\x1b[m\n\n")
        && (*xboard || xo->dir[0] == 'u'))
    {
        if (*xboard == 0)
            return XO_HEAD;

        rc = vget(2, 0, "(S)存檔 (Q)取消？[Q] ", buf, 3, LCECHO);
        if (rc != 's' && rc != 'S')
            return XO_HEAD;

        nbrd_fpath(fpath, xo->dir, nbrd);
        brd_fpath(xfolder, xboard, fn_dir);

        if (!(fd = fdopen(hdr_stamp(xfolder, 'A', &xpost, buf), "w")))
            return XO_HEAD;

        f_suck(fd, fpath);
        fclose(fd);

        strcpy(xpost.owner, cuser.userid);
        strcpy(xpost.nick, cuser.username);
        memcpy(xpost.date, nbrd->date, sizeof(xpost.date));

        if (nbrd->mode & NBRD_NBRD)
            sprintf(xpost.title, "申請新版 %-13s:%-22.22s", nbrd->brdname, nbrd->title);
        else if (nbrd->mode & NBRD_CANCEL)
            sprintf(xpost.title, "廢除 %s 版版主", nbrd->brdname);
        else
            sprintf(xpost.title, "連署 %-36.36s", nbrd->title);

        rec_add(xfolder, &xpost, sizeof(xpost));

        vmsg("轉錄完成");
    }
    return XO_HEAD;

}

static int
nbrd_help(
XO *xo)
{
    film_out(FILM_SIGNUP, -1);
    return XO_HEAD;
}

KeyFuncList nbrd_cb =
{
    {XO_INIT, {nbrd_init}},
    {XO_LOAD, {nbrd_load}},
    {XO_HEAD, {nbrd_head}},
    {XO_BODY, {nbrd_body}},

    {'j', {nbrd_join}},
    {'r', {nbrd_browse}},
    {'o', {nbrd_open}},
    {'s', {nbrd_start}},
    {'R', {nbrd_reject}},
    {'c', {nbrd_close}},
    {'d', {nbrd_delete}},
    {'x', {nbrd_cross}},
#ifdef  TEST_COSIGN
    {'z', {nbrd_zero}},
#endif
    {Ctrl('P'), {nbrd_add}},
    {'h', {nbrd_help}}
};

int
XoNewBoard(void)
{
    DL_HOLD;
    XO *xo;
    char fpath[64];
    clear();
    sprintf(fpath, "newboard/%s", fn_dir);
    xz[XZ_OTHER - XO_ZONE].xo = xo = xo_new(fpath);
    xo->cb = nbrd_cb;
    xo->pos = 0;
    xo->key = XZ_OTHER;
    xover(XZ_OTHER);
    free(xo);
    return DL_RELEASE(0);
}
