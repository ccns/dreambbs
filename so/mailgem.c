/*-------------------------------------------------------*/
/* so/mailgem.c        (YZU WindTop BBS 3.0)             */
/*-------------------------------------------------------*/
/* author : visor.bbs@bbs.yzu.edu.tw                     */
/* target : 精華區閱讀、編選                             */
/* create : 2000/08/27                                   */
/* update :                                              */
/*-------------------------------------------------------*/


#include "bbs.h"

/* definitions of MailGem Mode */


#define GEM_WAY         2
static int mailgem_way;

static int MailGemBufferNum;

static int mailgem_add(XO *xo);
static int mailgem_paste(XO *xo);
static int mailgem_anchor(XO *xo);
static int mailgem_recycle(XO *xo);
static void XoMailGem(const char *folder, const char *title);

static int
mailgem_foot(
    XO *xo)
{
    outf(MSG_GEM);
    return XO_NONE;
}

static void
mailgem_item(
int num,
const HDR *ghdr)
{
    int xmode, gtype;

    xmode = ghdr->xmode;
    gtype = (char) 0xba;
    if (xmode & GEM_FOLDER)
        gtype += 1;
    prints("%6d %c\241%c ", num,
           TagNum && !Tagger(ghdr->chrono, num - 1, TAG_NIN) ? '*' : ' ', gtype);

    gtype = HAS_PERM(PERM_SYSOP) ? mailgem_way : 0;

    prints("%-*.*s%-13s%s\n", d_cols + 47, d_cols + 46, ghdr->title,
           (gtype == 1 ? ghdr->xname : ghdr->owner), ghdr->date);
}

static int
mailgem_cur(
XO *xo)
{
    const HDR *const ghdr = (const HDR *) xo_pool_base + xo->pos;
    move(3 + xo->pos - xo->top, 0);
    mailgem_item(xo->pos + 1, ghdr);
    return XO_NONE;
}

static int
mailgem_body(
XO *xo)
{
    const HDR *ghdr;
    int num, max, tail;

    max = xo->max;
    if (max <= 0)
    {
        outs("\n\n《精華區》尚在吸取天地間的日精月華 :)");
        max = vans("(A)新增資料 (G)海錨功\能 (W)資源回收筒 [N]無所事事 ");
        switch (max)
        {
        case 'a':
            max = mailgem_add(xo);
            if (xo->max > 0)
                return max;
            break;
        case 'g':
            mailgem_anchor(xo);
            break;
        case 'w':
            mailgem_recycle(xo);
            break;
        }
        return XO_QUIT;
    }

    num = xo->top;
    ghdr = (const HDR *) xo_pool_base + num;
    tail = num + XO_TALL;
    max = BMIN(max, tail);

    move(3, 0);
    do
    {
        mailgem_item(++num, ghdr++);
    }
    while (num < max);
    clrtobot();

    return mailgem_foot(xo);
}


static int
mailgem_head(
XO *xo)
{
    char buf[20];

    vs_head("精華文章", (const char *) xo->xyz);

    sprintf(buf, "(剪貼版 %d 篇)\n", MailGemBufferNum);

    outs(NECK_MAILGEM1);
    outs(buf);
    prints(NECK_MAILGEM2, d_cols, "");
    return mailgem_body(xo);
}


static int
mailgem_toggle(
XO *xo)
{
    mailgem_way = (mailgem_way + 1) % GEM_WAY;
    return XO_BODY;
}


static int
mailgem_init(
XO *xo)
{
    xo_load(xo, sizeof(HDR));
    return mailgem_head(xo);
}


static int
mailgem_load(
XO *xo)
{
    xo_load(xo, sizeof(HDR));
    return mailgem_body(xo);
}


/* ----------------------------------------------------- */
/* mailgem_check : attribute / permission check out      */
/* ----------------------------------------------------- */


static HDR *
mailgem_check(
XO *xo,
char *fpath)
{
    HDR *ghdr;
    GCC_UNUSED int gtype, level;
    char *folder;

    level = xo->key;

    ghdr = (HDR *) xo_pool_base + xo->pos;
    gtype = ghdr->xmode;

    if (fpath)
    {
        folder = xo->dir;
        hdr_fpath(fpath, folder, ghdr);
    }
    return ghdr;
}


/* ----------------------------------------------------- */
/* 資料之新增：append / insert                           */
/* ----------------------------------------------------- */

static int
mailgem_add(
XO *xo)
{
    int gtype, level GCC_UNUSED, fd, ans;
    char title[80], fpath[80], *dir;
    HDR ghdr;

    level = xo->key;

    gtype = vans("新增 (A)文章 (F)卷宗 (P)貼複 (Q)取消？[Q] ");

    if (gtype == 'p')
        return mailgem_paste(xo);

    if (gtype != 'a' && gtype != 'f')
        return XO_FOOT;

    dir = xo->dir;
    fd = -1;
    memset(&ghdr, 0, sizeof(HDR));

    if (!vget(B_LINES_REF, 0, "標題：", title, 64, DOECHO))
        return XO_FOOT;

    fd = hdr_stamp(dir, gtype, &ghdr, fpath);
    if (fd < 0)
        return XO_FOOT;
    close(fd);
    if (gtype == 'a')
    {
        if (bbsothermode & OTHERSTAT_EDITING)
        {
            vmsg("你還有檔案還沒編完哦！");
            return XO_FOOT;
        }
        else if (vedit(fpath, false))
        {
            unlink(fpath);
            zmsg(msg_cancel);
            return XO_HEAD;
        }
        gtype = 0;
    }
    else if (gtype == 'f')
    {
        gtype = GEM_FOLDER;
    }
    ghdr.xmode = gtype;
    strcpy(ghdr.title, title);

    ans = vans("存放位置 A)ppend I)nsert N)ext Q)uit [A] ");

    if (ans == 'q')
    {
        if (fd >= 0)
            unlink(fpath);
        return (gtype ? XO_FOOT : XO_HEAD);
    }

    if (ans == 'i' || ans == 'n')
        rec_ins(dir, &ghdr, sizeof(HDR), xo->pos + (ans == 'n'), 1);
    else
        rec_add(dir, &ghdr, sizeof(HDR));

    return (gtype ? XO_LOAD : XO_INIT);
}


/* ----------------------------------------------------- */
/* 資料之修改：edit / title                              */
/* ----------------------------------------------------- */


static int
mailgem_edit(
XO *xo)
{
    char fpath[80];
    HDR *hdr;

    if (bbsothermode & OTHERSTAT_EDITING)
    {
        vmsg("你還有檔案還沒編完哦！");
        return XO_FOOT;
    }

    if (!(hdr = mailgem_check(xo, fpath)))
        return XO_NONE;
    vedit(fpath, false);
    return XO_HEAD;
}


static int
mailgem_title(
XO *xo)
{
    HDR *ghdr, xhdr;
    int num;
    char *dir;

    ghdr = mailgem_check(xo, NULL);
    if (ghdr == NULL)
        return XO_NONE;

    xhdr = *ghdr;
    vget(B_LINES_REF, 0, "標題：", xhdr.title, TTLEN + 1, GCARRY);

    dir = xo->dir;

    if (memcmp(ghdr, &xhdr, sizeof(HDR)) &&
        vans("確定要修改嗎(y/N)？[N]") == 'y')
    {
        *ghdr = xhdr;
        num = xo->pos;
        rec_put(dir, ghdr, sizeof(HDR), num);
        return XR_FOOT + XO_CUR;

    }
    return XO_FOOT;
}

static int
mailgem_state(
XO *xo)
{
    HDR *ghdr;
    char *dir, fpath[80];
    struct stat st;

    if (!HAS_PERM(PERM_SYSOP))
        return XO_NONE;

    if (!(ghdr = mailgem_check(xo, fpath)))
        return XO_NONE;

    dir = xo->dir;

    move(12, 0);
    clrtobot();
    outs("\nDir : ");
    outs(dir);
    outs("\nName: ");
    outs(ghdr->xname);
    outs("\nFile: ");
    outs(fpath);

    if (!stat(fpath, &st))
        prints("\nTime: %s\nSize: %lld", Ctime(&st.st_mtime), (long long)st.st_size);

    vmsg(NULL);

    return XO_BODY;
}


/* ----------------------------------------------------- */
/* 資料之瀏覽：edit / title                              */
/* ----------------------------------------------------- */


static int
mailgem_browse(
XO *xo)
{
    HDR *ghdr;
    int xmode;
    char fpath[80], title[TTLEN + 1];

    do
    {
        ghdr = mailgem_check(xo, fpath);
        if (ghdr == NULL)
            break;

        xmode = ghdr->xmode;

        /* browse folder */

        if (xmode & GEM_FOLDER)
        {
            strcpy(title, ghdr->title);
            XoMailGem(fpath, title);
            return XO_INIT;
        }

        /* browse article */

        if ((xmode = more(fpath, MSG_GEM)) == -2)
            return XO_INIT;
        if (xmode == -1)
            break;

        xmode = xo_getch(xo, xmode);

    }
    while (xmode == XO_BODY);

    return XO_HEAD;
}


/* ----------------------------------------------------- */
/* 精華區之刪除： copy / cut (delete) paste / move       */
/* ----------------------------------------------------- */


static char MailGemFolder[80], MailGemAnchor[80], MailGemSailor[24];
static HDR *MailGemBuffer;
static int MailGemBufferSiz; /*, MailGemBufferNum; */
/* Thor.990414: 提前宣告給mailgem_head用 */


/* 配置足夠的空間放入 header */


static HDR *
mbuf_malloc(
int num)
{
    HDR *gbuf;

    MailGemBufferNum = num;
    if ((gbuf = MailGemBuffer))
    {
        if (MailGemBufferSiz < num)
        {
            num += (num >> 1);
            MailGemBufferSiz = num;
            MailGemBuffer = gbuf = (HDR *) realloc(gbuf, sizeof(HDR) * num);
        }
    }
    else
    {
        MailGemBufferSiz = num;
        MailGemBuffer = gbuf = (HDR *) malloc(sizeof(HDR) * num);
    }

    return gbuf;
}


static void
mailgem_buffer(
const char *dir,
const HDR *ghdr)                /* NULL 代表放入 TagList, 否則將傳入的放入 */
{
    int num, locus;
    HDR *gbuf;

    if (ghdr)
    {
        num = 1;
    }
    else
    {
        num = TagNum;
        if (num <= 0)
            return;
    }

    gbuf = mbuf_malloc(num);

    if (ghdr)
    {
        memcpy(gbuf, ghdr, sizeof(HDR));
    }
    else
    {
        locus = 0;
        do
        {
            EnumTagHdr(&gbuf[locus], dir, locus);
        }
        while (++locus < num);
    }

    strcpy(MailGemFolder, dir);
}

static int
mailgem_delete(
XO *xo)
{
    HDR *ghdr;
    char *dir, buf[80];
    int tag;

    if (!(ghdr = mailgem_check(xo, NULL)))
        return XO_NONE;

    tag = AskTag("精華區刪除");

    if (tag < 0)
        return XO_FOOT;

    if (tag > 0)
    {
        sprintf(buf, "確定要刪除 %d 篇標籤精華嗎(y/N)？[N] ", tag);
        if (vans(buf) != 'y')
            return XO_FOOT;
    }

    dir = xo->dir;

    mailgem_buffer(dir, tag ? NULL : ghdr);

    /* 只刪除 HDR 並不刪除檔案 */

    if (tag)
    {
        int fd;
        FILE *fpw;

        if ((fd = open(dir, O_RDONLY)) < 0)
            return XO_FOOT;

        if (!(fpw = f_new(dir, buf)))
        {
            close(fd);
            return XO_FOOT;
        }

        mgets(-1);
        tag = 0;

        while ((ghdr = (HDR *) mread(fd, sizeof(HDR))))
        {
            if (Tagger(ghdr->chrono, tag, TAG_NIN))
            {
                if ((fwrite(ghdr, sizeof(HDR), 1, fpw) != 1))
                {
                    fclose(fpw);
                    unlink(buf);
                    close(fd);
                    return XO_FOOT;
                }
            }
            tag++;
        }
        close(fd);
        fclose(fpw);
        rename(buf, dir);
        TagNum = 0;
    }
    else
    {
        currchrono = ghdr->chrono;
        rec_del(dir, sizeof(HDR), xo->pos, cmpchrono, NULL);
    }

    return XO_INIT;
}


static int
mailgem_copy(
XO *xo)
{
    HDR *ghdr;
    int tag;

    ghdr = mailgem_check(xo, NULL);
    if (ghdr == NULL)
        return XO_NONE;

    tag = AskTag("精華區拷貝");

    if (tag < 0)
        return XO_FOOT;

    mailgem_buffer(xo->dir, tag ? NULL : ghdr);

    zmsg("拷貝完成");
    return XO_HEAD;
}


static inline int
mailgem_extend(
XO *xo,
int num)
{
    char *dir, fpath[80], gpath[80];
    FILE *fp;
    time_t chrono;
    HDR *hdr;

    if (!(hdr = mailgem_check(xo, fpath)))
        return -1;

    if (!(fp = fopen(fpath, "a")))
        return -1;

    dir = xo->dir;
    chrono = hdr->chrono;

    for (hdr = MailGemBuffer; num--; hdr++)
    {
        if ((hdr->chrono != chrono) && !(hdr->xmode & GEM_FOLDER))
        {
            hdr_fpath(gpath, dir, hdr); /* Thor: 假設 hdr和 xo->dir是同一目錄 */
            fputs(STR_LINE, fp);
            f_suck(fp, gpath);
        }
    }

    fclose(fp);
    return 0;
}


static int
mailgem_paste(
XO *xo)
{
    int num, ans;
    char *dir;

    if (!(num = MailGemBufferNum))
    {
        zmsg("請先執行 copy 命令後再 paste");
        return XO_FOOT;
    }

    dir = xo->dir;
    switch (ans = vans("存放位置 A)ppend I)nsert N)ext E)xtend Q)uit [A] "))
    {
    case 'q':
        return XO_FOOT;

    case 'e':
        if (mailgem_extend(xo, num))
        {
            zmsg("[Extend 檔案附加] 動作並未完全成功\");
            return XO_FOOT;
        }
        return XO_FOOT;

    case 'i':
    case 'n':
        rec_ins(dir, MailGemBuffer, sizeof(HDR), xo->pos + (ans == 'n'), num);
        break;

    default:
        rec_add(dir, MailGemBuffer, sizeof(HDR) * num);
    }

    return XO_LOAD;
}


static int
mailgem_move(
XO *xo)
{
    HDR *ghdr;
    char *dir, buf[80];
    int pos, newOrder;

    ghdr = mailgem_check(xo, NULL);

    if (ghdr == NULL)
        return XO_NONE;

    pos = xo->pos;
    sprintf(buf + 5, "請輸入第 %d 選項的新位置：", pos + 1);
    if (!vget(B_LINES_REF, 0, buf + 5, buf, 5, DOECHO))
        return XO_FOOT;

    newOrder = TCLAMP(atoi(buf) - 1, 0, xo->max - 1);

    if (newOrder != pos)
    {
        const HDR ghdr_orig = *ghdr;

        dir = xo->dir;
        if (!rec_del(dir, sizeof(HDR), pos, NULL, NULL))
        {
            rec_ins(dir, &ghdr_orig, sizeof(HDR), newOrder, 1);
            xo->pos = newOrder;
            return XO_LOAD;
        }
    }
    return XO_FOOT;
}


static int
mailgem_anchor(
XO *xo)
{
    int ans;
    char *folder;

    ans = vans("精華區 A)定錨 D)拔錨 J)就位 Q)取消 [J] ");
    if (ans != 'q')
    {
        folder = MailGemAnchor;

        if (ans == 'a')
        {
            strcpy(folder, xo->dir);
            str_scpy(MailGemSailor, (const char *) xo->xyz, sizeof(MailGemSailor));
        }
        else if (ans == 'd')
        {
            *folder = '\0';
        }
        else
        {
            if (!*folder)
            {
                vmsg("尚未定錨");
                return  XO_FOOT;
            }

            XoMailGem(folder, "● 精華定錨區 ●");
            return XO_INIT;
        }

        zmsg("錨動作完成");
        return XO_FOOT;
    }

    return XO_NONE;
}


int
mailgem_gather(
XO *xo)
{
    DL_HOLD;
    HDR *hdr, *gbuf, ghdr, xhdr;
    int tag, locus, rc, xmode, anchor;
    char *dir, *folder, fpath[80], buf[80];
    const char *msg;
    FILE *fp, *fd;

    usr_fpath(fpath, cuser.userid, "gem");

    if (access(fpath, 0))
        mak_dirs(fpath);

    folder = MailGemAnchor;
    if ((anchor = *folder))
    {
        sprintf(buf, "收錄至信件精華定錨區 (%s)", MailGemSailor);
        msg = buf;
    }
    else
    {
        msg = "收錄至精華回收筒";
    }
    tag = AskTag(msg);

    if (tag < 0)
        return DL_RELEASE(XO_FOOT);

    if (!anchor)
    {
        usr_fpath(folder, cuser.userid, "gem/.GEM");
    }

    fd = fp = NULL;

    if (tag > 0)
    {
        switch (vans("串列文章 1)合成一篇 2)分別建檔 Q)取消 [2] "))
        {
        case 'q':
            return DL_RELEASE(XO_FOOT);

        case '1':
            strcpy(xhdr.title, currtitle);
            if (!vget(B_LINES_REF, 0, "標題：", xhdr.title, TTLEN + 1, GCARRY))
                return DL_RELEASE(XO_FOOT);
            fp = fdopen(hdr_stamp(folder, 'A', &ghdr, fpath), "w");
            strcpy(ghdr.owner, cuser.userid);
            strcpy(ghdr.title, xhdr.title);
            break;
        default:
            break;

        }
    }

    if (tag)
        hdr = &xhdr;
    else
        hdr = (HDR *) xo_pool_base + xo->pos;

    dir = xo->dir;
    rc = (*dir == 'b') ? XO_LOAD : XO_FOOT;

    /* gather 視同 copy, 可準備作 paste */

    strcpy(MailGemFolder, folder);
    gbuf = mbuf_malloc((fp != NULL || tag == 0) ? 1 : tag);

    locus = 0;

    do
    {
        if (tag)
        {
            EnumTagHdr(hdr, dir, locus);
        }

        xmode = hdr->xmode;

        if (!(xmode & GEM_FOLDER))      /* 查 hdr 是否 plain text */
        {
            hdr_fpath(fpath, dir, hdr);

            if (fp)
            {
                f_suck(fp, fpath);
                fputs(STR_LINE, fp);
            }
            else
            {
                fd = fdopen(hdr_stamp(folder, 'A', &ghdr, buf), "w");
                strcpy(ghdr.owner, cuser.userid);
                strcpy(ghdr.title, hdr->title);
                f_suck(fd, fpath);
                rec_add(folder, &ghdr, sizeof(HDR));

                if (fd)                 /* by visor */
                    fclose(fd);

                gbuf[locus] = ghdr;     /* 放入 MailGembuffer */
            }

        }
    }
    while (++locus < tag);

    if (fp)
    {
        fclose(fp);
        gbuf[0] = ghdr;
        rec_add(folder, &ghdr, sizeof(HDR));
    }

    zmsg("收錄完成");

    return DL_RELEASE(rc);
}


static int
mailgem_tag(
XO *xo)
{
    const HDR *ghdr;
    int pos, tag;

    pos = xo->pos;
    ghdr = (const HDR *) xo_pool_base + pos;

    if ((tag = Tagger(ghdr->chrono, pos, TAG_TOGGLE)))
    {
        return XO_CUR + 1;
    }

    return XO_MOVE + XO_REL + 1;
}


static int
mailgem_help(
XO *xo)
{
    /*  film_out(FILM_GEM, -1);*/
    vmsg("尚未編輯使用說明");
    return XO_HEAD;
}

static int
mailgem_cross(
XO *xo)
{
    char xboard[20], fpath[80], xfolder[80], xtitle[80], buf[80], *dir;
    HDR *hdr, xpost, *ghdr;
    int method = 1, rc GCC_UNUSED, tag, locus, battr;
    FILE *xfp;
    int success_count = 0;

    if (!cuser.userlevel)
        return XO_NONE;

    ghdr = mailgem_check(xo, NULL);
    tag = AskTag("轉貼");
    if ((tag < 0) || (tag == 0 && (ghdr->xmode & GEM_FOLDER)))
        return XO_FOOT;


    if (ask_board(xboard, BRD_W_BIT,
                  "\n\n\x1b[1;33m請挑選適當的看板，切勿轉貼超過三板。\x1b[m\n\n"))
    {
        if (*xboard == 0)
            strcpy(xboard, currboard);

        hdr = tag ? &xpost : (HDR *) xo_pool_base + xo->pos;


        if (!tag)
        {
            if (!(hdr->xmode & GEM_FOLDER))
            {
                sprintf(xtitle, STR_FORWARD " %.66s", hdr->title);
                if (!vget(2, 0, "標題:", xtitle, TTLEN + 1, GCARRY))
                    return XO_HEAD;
            }
            else
                return XO_HEAD;
        }

        rc = vget(2, 0, "(S)存檔 (Q)取消？[Q] ", buf, 3, LCECHO);
        if (*buf != 's' && *buf != 'S')
            return XO_HEAD;

        locus = 0;
        dir = xo->dir;

        battr = (bshm->bcache + brd_bno(xboard))->battr;

        do
        {
            if (tag)
            {
                EnumTagHdr(hdr, dir, locus++);

                sprintf(xtitle, STR_FORWARD " %.66s", hdr->title);
            }
            if (!(hdr->xmode & GEM_FOLDER))
            {
                xo_fpath(fpath, dir, hdr);
                brd_fpath(xfolder, xboard, fn_dir);

                method = hdr_stamp(xfolder, 'A', &xpost, buf);
                xfp = fdopen(method, "w");

                strcpy(ve_title, xtitle);
                strcpy(buf, currboard);
                strcpy(currboard, xboard);

                ve_header(xfp);

                strcpy(currboard, buf);

                strcpy(buf, cuser.userid);
                strcat(buf, "] 信件精華區");
                fprintf(xfp, "※ 本文轉錄自 [%s\n\n", buf);

                f_suck(xfp, fpath);
                fclose(xfp);

                strcpy(xpost.owner, cuser.userid);
                strcpy(xpost.nick, cuser.username);


                strcpy(xpost.title, xtitle);

                rec_add(xfolder, &xpost, sizeof(xpost));

                success_count++;
            }
        }
        while (locus < tag);

        if (success_count == 0)
        {
            vmsg("轉錄失敗。");
        }
        if (battr & BRD_NOCOUNT)
        {
            if (success_count == ((tag == 0) ? 1 : tag))
                prints("轉錄 %d 篇成功\，文章不列入紀錄，敬請包涵。", success_count);
            else
                prints("轉錄 %d 篇成功\，%d 篇失敗，文章不列入紀錄，敬請包涵。",
                    success_count, ((tag == 0) ? 1 : tag) - success_count);
        }
        else
        {
            cuser.numposts += success_count; /* IID.20190730: Use the count of successful reposting */
            if (success_count == ((tag == 0) ? 1 : tag))
                sprintf(buf, "轉錄 %d 篇成功\，你的文章增為 %d 篇",
                    success_count, cuser.numposts);
            else
                sprintf(buf, "轉錄 %d 篇成功\，%d 篇失敗，你的文章增為 %d 篇",
                    success_count, ((tag == 0) ? 1 : tag) - success_count, cuser.numposts);
            vmsg(buf);
        }
    }
    return XO_HEAD;
}

static int
mailgem_recycle(
XO *xo)
{
    char fpath[128];
    usr_fpath(fpath, cuser.userid, "gem/.GEM");
    XoMailGem(fpath, "我的精華區資源回收筒");
    return XO_INIT;
}

static KeyFuncList mailgem_cb =
{
    {XO_INIT, {mailgem_init}},
    {XO_LOAD, {mailgem_load}},
    {XO_HEAD, {mailgem_head}},
    {XO_BODY, {mailgem_body}},
    {XO_FOOT, {mailgem_foot}},
    {XO_CUR, {mailgem_cur}},

    {'r', {mailgem_browse}},

    {Ctrl('P'), {mailgem_add}},
    {'E', {mailgem_edit}},
    {'T', {mailgem_title}},
    {'x', {mailgem_cross}},
    {'M', {mailgem_move}},
    {'d', {mailgem_delete}},
    {'c', {mailgem_copy}},
    {'W', {mailgem_recycle}},

    {Ctrl('G'), {mailgem_anchor}},
    {Ctrl('V'), {mailgem_paste}},

    {'t', {mailgem_tag}},
    {'f', {mailgem_toggle}},

    {'S', {mailgem_state}},

    {'h', {mailgem_help}}
};


static void
XoMailGem(
const char *folder,
const char *title)
{
    XO *xo, *last;

    last = xz[XZ_MAILGEM - XO_ZONE].xo; /* record */

    xz[XZ_MAILGEM - XO_ZONE].xo = xo = xo_new(folder);
    xo->cb = mailgem_cb;
    xo->recsiz = sizeof(HDR);
    xo->pos = 0;
    xo->key = 0;
    xo->xyz = (void *)title;

    xover(XZ_MAILGEM);

    free(xo);

    xz[XZ_MAILGEM - XO_ZONE].xo = last; /* restore */
}


void
mailgem_main(void)
{
    DL_HOLD;
    XO *xo, *last;
    char fpath[128];

    last = xz[XZ_MAILGEM - XO_ZONE].xo;  /* record */

    usr_fpath(fpath, cuser.userid, "gem");

    if (access(fpath, 0))
        mak_dirs(fpath);

    usr_fpath(fpath, cuser.userid, "gem/.DIR");
    xz[XZ_MAILGEM - XO_ZONE].xo = xo = xo_new(fpath);
    xo->cb = mailgem_cb;
    xo->recsiz = sizeof(HDR);
    xo->pos = 0;
    xo->key = 0;
    xo->xyz = (void *)"我的精華區";
    xover(XZ_MAILGEM);
    free(xo);

    xz[XZ_MAILGEM - XO_ZONE].xo = last;  /* restore */

    DL_RELEASE(0);
}


/* ----------------------------------------------------- */
/* sync mailgem                                          */
/* ----------------------------------------------------- */
#define GEM_DROP        0x0080
#define GCHECK_DEPTH    30
#define GEM_EXPIRE      45      /* gem 至多存 45 天 */

static char pgem[128], pool[128];

/* ----------------------------------------------------- */
/* synchronize folder & files                            */
/* ----------------------------------------------------- */


typedef struct
{
    time_t chrono;
    char prefix;
    char exotic;
}      SyncData;


static SyncData *sync_pool;
static int sync_size, sync_head;


#define SYNC_DB_SIZE    2048


static int
sync_cmp(
const void *s1, const void *s2)
{
    return ((const SyncData *)s1)->chrono - ((const SyncData *)s2)->chrono;
}


static void
sync_init(
const char *fname)
{
    int ch, prefix;
    time_t chrono;
    char *str, *fname_str, fpath[80];
    struct stat st;
    struct dirent *de;
    DIR *dirp;

    SyncData *xpool;
    int xsize, xhead;

    if ((xpool = sync_pool))
    {
        xsize = sync_size;
    }
    else
    {
        xpool = (SyncData *) malloc(SYNC_DB_SIZE * sizeof(SyncData));
        xsize = SYNC_DB_SIZE;
    }

    xhead = 0;

    ch = strlen(fname);
    memcpy(fpath, fname, ch);
    fname_str = fpath + ch;
    *fname_str++ = '/';

    ch = '0';
    for (;;)
    {
        *fname_str = ch++;
        fname_str[1] = '\0';

        if ((dirp = opendir(fpath)))
        {
            fname_str[1] = '/';
            while ((de = readdir(dirp)))
            {
                str = de->d_name;
                prefix = *str;
                if (prefix == '.')
                    continue;

                strcpy(fname_str + 2, str);
                if (stat(fpath, &st) || !st.st_size)
                {
                    unlink(fpath);
                    continue;
                }

                chrono = chrono32(str);

                if (xhead >= xsize)
                {
                    xsize += (xsize >> 1);
                    xpool = (SyncData *) realloc(xpool, xsize * sizeof(SyncData));
                }

                xpool[xhead].chrono = chrono;
                xpool[xhead].prefix = prefix;
                xpool[xhead].exotic = 1;
                xhead++;
            }

            closedir(dirp);
        }

        if (ch == 'W')
            break;

        if (ch == '9' + 1)
            ch = 'A';
    }

    if (xhead > 1)
        qsort(xpool, xhead, sizeof(SyncData), sync_cmp);

    sync_pool = xpool;
    sync_size = xsize;
    sync_head = xhead;

}


static void
sync_check(
const char *fgem)
{
    int expire;
    char *str, *fname, fpath[128], fnew[128];
    SyncData *xpool, *xtail, *xsync;
    time_t cc, due;
    FILE *fpr, *fpw;
    HDR hdr;
    struct tm *pt;

    if ((sync_head) <= 0)
        return;

    fpr = fopen(fgem, "r");
    if (fpr == NULL)
    {
        return;
    }

    sprintf(fnew, "%s.n", fgem);
    fpw = fopen(fnew, "w");
    if (fpw == NULL)
    {
        fclose(fpr);
        return;
    }

    /* 清理逾期未整理者 */
    strcpy(fpath, fgem);
    str = strrchr(fpath, '.');
    str[1] = '/';
    fname = str + 2;

    expire = 0;
    due = time(0) - GEM_EXPIRE * 86400;

    while (fread(&hdr, sizeof(hdr), 1, fpr) == 1)
    {
        if ((hdr.xmode & GEM_DROP) && !(memcmp(hdr.title, "滄海拾遺 [", 10)))
        {
            if ((xsync = (SyncData *) bsearch(&hdr.chrono, sync_pool, sync_head, sizeof(SyncData), sync_cmp)))
            {
                if (xsync->exotic == 0) /* 已被 reference */
                    continue;
                else
                {
                    if (hdr.xid < due)
                    {
                        expire++;

                        xsync->exotic = 0;
                        cc = xsync->chrono;
                        *str = radix32[cc % 32U];
                        archiv32m(cc, fname);
                        fname[0] = xsync->prefix;
                        unlink(fpath);
                        continue;
                    }
                }
            }
            else
            {
                continue;
            }
        }

        fwrite(&hdr, sizeof(hdr), 1, fpw);
    }
    fclose(fpr);

    /* recycle : recover "lost chain" */

    xpool = xsync = sync_pool;
    xtail = xpool + sync_head;

    strcpy(fpath, fgem);
    str = strrchr(fpath, '.');
    str[1] = '/';
    fname = str + 2;

    /* setup header */

    memset(&hdr, 0, sizeof(hdr));
    time((time_t *) &hdr.xid);
    pt = localtime((time_t *) & hdr.xid);
    sprintf(hdr.date, "%02d/%02d/%02d", pt->tm_mon + 1, pt->tm_mday, pt->tm_year % 100);

    /* 若 lost chain 為 folder, 先行寫回 */

    hdr.xmode = GEM_FOLDER | GEM_DROP;

    do
    {
        if ((xsync->exotic) && (xsync->prefix == 'F'))
        {
            xsync->exotic = 0;
            cc = xsync->chrono;
            *str = radix32[cc % 32U];
            archiv32m(cc, fname);
            fname[0] = xsync->prefix;
            if (gcheck(1, fpath))
            {
                hdr.chrono = cc;
                strcpy(hdr.xname, fname);
                sprintf(hdr.title, "滄海拾遺 [%s]", str);
                fwrite(&hdr, sizeof(hdr), 1, fpw);
            }
            else
            {
                unlink(fpath);
            }
        }

    }
    while (++xsync < xtail);

    /* 處理一般檔案 */

    hdr.xmode = GEM_DROP;

    do
    {
        if (xpool->exotic)
        {
            cc = xpool->chrono;
            *str = radix32[cc % 32U];
            archiv32m(cc, fname);
            fname[0] = xpool->prefix;

            hdr.chrono = cc;
            strcpy(hdr.xname, fname);
            strcpy(hdr.owner, "系統自動維護");
            sprintf(hdr.title, "滄海拾遺 [%s]", str);
            fwrite(&hdr, sizeof(hdr), 1, fpw);
        }
    }
    while (++xpool < xtail);

    fclose(fpw);

    rename(fnew, fgem);

}


/* ----------------------------------------------------- */
/* visit the hierarchy recursively                       */
/* ----------------------------------------------------- */


int
gcheck(
int level,
char *fpath)
{
    DL_HOLD;
    int count, xmode, xhead;
    char *fname, *ptr = NULL, buf[80];
    FILE *fp;
    HDR hdr;
    SyncData *xsync;

    if (!level)
    {
        sync_init(fpath);
        sprintf(pgem, "%s/.GEM", fpath);
        sprintf(pool, "%s/.DIR", fpath);
        fpath = pool;
    }
    else if (level > GCHECK_DEPTH)
    {
        return DL_RELEASE(1);
    }

    /* open the folder */

    fp = fopen(fpath, "r");
    if (!fp)
        return DL_RELEASE(0);

    strcpy(buf, fpath);

    fname = fpath;
    while ((xmode = *fname++))
    {
        if (xmode == '/')
            ptr = fname;
    }
    if (*ptr != '.')
        ptr -= 2;
    fname = ptr;

    /* --------------------------------------------------- */
    /* visit the header file                               */
    /* --------------------------------------------------- */

    count = 0;
    xhead = sync_head;
    while (fread(&hdr, sizeof(hdr), 1, fp) == 1)
    {
        ptr = hdr.xname;                /* F1234567 */

        if (*ptr == '@')
        {
            continue;
        }

        xmode = hdr.xmode;

        if ((xsync = (SyncData *) bsearch(&hdr.chrono, sync_pool, xhead, sizeof(SyncData), sync_cmp)))
        {
            xsync->exotic = 0;  /* 正常情況 : 有被 reference */
        }

        /* 若為一般 folder 則 recursive 進入 */

        if ((xmode & GEM_FOLDER))
        {
            sprintf(fname, "%c/%s", ptr[7], ptr);
            if (!gcheck(level + 1, fpath))
                continue;
        }

        count++;
    }

    fclose(fp);

    if (!level)
    {
        strcpy(pool, pgem);
        gcheck(1, pool);
        sync_check(pgem);
    }

    return DL_RELEASE(count);
}



