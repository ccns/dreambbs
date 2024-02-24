#define ADMIN_C

#include "bbs.h"

#ifdef HAVE_SONG

static void XoSong(const char *folder, const char *title, int level);
static int song_order(XO *xo, int pos);

#define GEM_READ        1       /* readable */
#define GEM_WRITE       2       /* writable */
#define GEM_FILE        4       /* 預期是檔案 */

#define SONG_SRC        "<~Src~>"
#define SONG_DES        "<~Des~>"
#define SONG_SAY        "<~Say~>"
#define SONG_END        "<~End~>"

static void
log_song(
const char *msg)
{
    char buf[512];
    time_t now = time(0);
    sprintf(buf, "%s %-*s %s\n", Etime(&now), IDLEN, cuser.userid, msg);
    f_cat(FN_SONG_LOG, buf);
}



static int
song_swap(
char *str,
const char *src,
const char *des)
{
    char *ptr, *tmp;
    char buf[300];
    ptr = strstr(str, src);
    if (ptr)
    {
        *ptr = '\0';
        tmp = ptr + strlen(src);
        sprintf(buf, "%s%s%s", str, des, tmp);
        strcpy(str, buf);
        return 1;
    }
    else
        return 0;
}


static HDR *
song_check(
XO *xo,
int pos,
char *fpath)
{
    HDR *ghdr;
    int gtype, level;
    char *folder;

    level = xo->key;

    ghdr = (HDR *) xo_pool_base + pos;
    gtype = ghdr->xmode;

    if ((gtype & GEM_RESTRICT) && (level <= GEM_USER))
        return NULL;

    if ((gtype & GEM_LOCK) && (!HAS_PERM(PERM_SYSOP)))
        return NULL;

    if (fpath)
    {
        if (gtype & GEM_BOARD)
        {
            sprintf(fpath, "gem/brd/%s/.DIR", ghdr->xname);
        }
        else
        {
            folder = xo->dir;
            if (gtype & GEM_GOPHER)
            {
                return NULL;
            }
            else
            {
                hdr_fpath(fpath, folder, ghdr);
            }
        }
    }
    return ghdr;
}

static HDR *
song_get(
XO *xo,
int pos,
char *fpath)
{
    HDR *ghdr;
    int gtype, level;
    char *folder;

    level = xo->key;

    ghdr = (HDR *) xo_pool_base + pos;
    gtype = ghdr->xmode;

    if ((gtype & GEM_RESTRICT) && (level <= GEM_USER))
        return NULL;

    if ((gtype & GEM_LOCK) && (!HAS_PERM(PERM_SYSOP)))
        return NULL;

    if (fpath)
    {
        folder = xo->dir;
        if (gtype & (GEM_GOPHER | GEM_FOLDER | GEM_RESERVED))
        {
            return NULL;
        }
        else
        {
            hdr_fpath(fpath, folder, ghdr);
        }
    }
    return ghdr;
}

static int
song_foot(
    XO *xo)
{
    outf(MSG_GEM);
    return XO_NONE;
}

static int
song_item(
XO *xo,
int pos)
{
    const HDR *const ghdr = (const HDR *) xo_pool_base + pos;
    const int num = pos + 1;
    int xmode;
    const char *gtype;

    xmode = ghdr->xmode;
    gtype = ((xmode & GEM_FOLDER) ? "◆" : "◇");
    if (xmode & GEM_GOPHER)
        gtype = ((xmode & GEM_FOLDER) ? "■" : "□");
    prints("%6d%c %s ", num, (xmode & GEM_RESTRICT) ? ')' : (xmode & GEM_LOCK) ? 'L' :  ' ', gtype);

    const int gway = 0;

    if (!HAS_PERM(PERM_SYSOP) && (xmode & (GEM_RESTRICT | GEM_LOCK)))
        prints("\x1b[1;33m資料保密！\x1b[m\n");
    else if ((gway == 0) || (xmode & GEM_GOPHER))
        prints("%-.*s\n", d_cols + 68, ghdr->title);

    return XO_NONE;
}

static int
song_cur(
XO *xo,
int pos)
{
    move(3 + pos - xo->top, 0);
    return song_item(xo, pos);
}


static int
song_body(
XO *xo)
{
    int num, max, tail;

    move(3, 0);

    max = xo->max;
    if (max <= 0)
    {
        outs("\n《歌本》尚在吸取天地間的日精月華 :)\n");
        clrtobot();
        return song_foot(xo);
    }

    num = xo->top;
    tail = num + XO_TALL;
    max = BMIN(max, tail);

    do
    {
        song_item(xo, num++);
    } while (num < max);
    clrtobot();

    return song_foot(xo);
}


static int
song_head(
XO *xo)
{

    vs_head("精華文章", (const char *) xo->xyz);
    prints(NECK_SONG, d_cols, "");
    return song_body(xo);
}


static int
song_init(
XO *xo)
{
    xo_load(xo, sizeof(HDR));
    return song_head(xo);
}


static int
song_load(
XO *xo)
{
    xo_load(xo, sizeof(HDR));
    return song_body(xo);
}


/* ----------------------------------------------------- */
/* 資料之新增：append / insert                           */
/* ----------------------------------------------------- */


static int
song_browse(
XO *xo,
int pos)
{
    HDR *ghdr;
    int xmode, op = 0;
    char fpath[80], title[TTLEN + 1];

    do
    {
        ghdr = song_check(xo, pos, fpath);
        if (ghdr == NULL)
            break;

        xmode = ghdr->xmode;

        /* browse folder */

        if (xmode & GEM_FOLDER)
        {
            op = xo->key;
            if (xmode & GEM_BOARD)
            {
                op = brd_bno(ghdr->xname);
                if (HAS_PERM(PERM_SYSOP))
                    op = GEM_SYSOP;
                else
                    op = GEM_USER;
            }
            else if (xmode & HDR_URL)
            {
                return XO_NONE;
            }

            strcpy(title, ghdr->title);
            XoSong(fpath, title, op);
            return XO_INIT;
        }

        /* browse article */

        /* Thor.990204: 為考慮more 傳回值 */
        if ((xmode = more(fpath, MSG_GEM)) == XO_HEAD)
            return XO_INIT;
        if (xmode == -1)
            break;

        op = GEM_READ | GEM_FILE;

        xmode = xo_getch(xo, pos, xmode);
        pos = xo->pos[xo->cur_idx];
    } while (xmode == XO_BODY);

    if (op != GEM_READ)
        return XO_HEAD;
    return XO_NONE;
}

#if 1
static int
song_order(
XO *xo,
int pos)
{
    char xboard[20], fpath[80], xfolder[80], xtitle[80], *dir, buf[128];
    char tmp[256], idwho[20], want_say[32];
    HDR *hdr, xpost;
    int method, battr GCC_UNUSED, flag;
    FILE *xfp, *fp;
    ACCT acct;
    time_t token, now;

    memset(&xpost, 0, sizeof(HDR));
    acct_load(&acct, cuser.userid);

    hdr = song_get(xo, pos, fpath);
    if (hdr == NULL)
        return XO_NONE;

    if (!cuser.userlevel)
        return XO_NONE;
    if (acct.request < 1)
    {
        vmsg_xo(xo, "點歌次數已用完！");
        return XO_FOOT;
    }


    if (!vget_xo(xo, B_LINES_REF, 0, "點歌給誰：", idwho, sizeof(idwho), DOECHO))
        strcpy(idwho, "大家");
    if (!vget_xo(xo, B_LINES_REF, 0, "想說的話：", want_say, sizeof(want_say), DOECHO))
        strcpy(want_say, ".........");

    if (vans_xo(xo, "要匿名嗎 [y/N]：") == 'y')
        flag = 1;
    else
        flag = 0;

    if (vans_xo(xo, "確定點歌嗎 [y/N]：") != 'y')
        return XO_HEAD;

    strcpy(xboard, BRD_ORDERSONGS);


    method = 0;

    dir = xo->dir;
    battr = (bshm->bcache + brd_bno(xboard))->battr;
    strcpy(xtitle, hdr->title);
    xo_fpath(fpath, dir, hdr);
    brd_fpath(xfolder, xboard, fn_dir);

    method = hdr_stamp(xfolder, 'A', &xpost, buf);
    xfp = fdopen(method, "w");



//  memcpy(xpost.owner, cuser.userid, sizeof(xpost.owner) + sizeof(xpost.nick));
    strcpy(xpost.owner, cuser.userid);
    token = time(0);
    if (flag)
        strcpy(xpost.owner, "[不告訴你]");

    str_stamp(xpost.date, &token);
    sprintf(xpost.title, "%s 點給 %s", xpost.owner, idwho);

    now = time(0);

    fprintf(xfp, "%s %s (%s) %s %s\n", str_author1, flag ? "[不告訴你]" : cuser.userid,
            flag ? "啦啦啦~~" : cuser.username, str_post2, BRD_ORDERSONGS);
    fprintf(xfp, "標題: %s\n時間: %s\n", xpost.title, ctime(&now));

    log_song(xpost.title);

    fp = fopen(fpath, "r+");

    while (fgets(tmp, 256, fp))
    {
        if (strstr(tmp, SONG_END))
            break;

        while (song_swap(tmp, SONG_SRC, flag ? "某人" : cuser.userid))
            ;
        while (song_swap(tmp, SONG_DES, idwho))
            ;
        while (song_swap(tmp, SONG_SAY, want_say))
            ;

        fputs(tmp, xfp);
    }
    sprintf(buf, "\x1b[1;33m%s\x1b[m 想對 \x1b[1;33m%s\x1b[m 說 %s\n\x1b[30m%s\x1b[m\n", flag ? "某人" : cuser.userid, idwho, want_say, ctime(&now));
    fputs(buf, xfp);

    fclose(fp);

    fclose(xfp);

    acct_load(&acct, cuser.userid);
    acct.request -= 1;
    cuser.request = acct.request;
    sprintf(buf, "剩餘點歌次數：%d 次", acct.request);
    vmsg_xo(xo, buf);
    acct_save(&acct);

    rec_add(xfolder, &xpost, sizeof(xpost));
    return XO_HEAD;
}
#endif  /* #if 1 */

static int
song_query(
XO *xo)
{
    char buf[80];

    sprintf(buf, "剩餘點歌次數：%d", cuser.request);
    vmsg_xo(xo, buf);
    return XO_HEAD;
}

static int
song_send(
XO *xo,
int pos)
{
    char fpath[128], folder[128], *dir GCC_UNUSED, title[80], buf[128], want_say[32], date[9];
    char tmp[300];
    HDR *hdr, xhdr;
    ACCT acct, cacct;
    int method;
    FILE *xfp, *fp;
    time_t now;

    now = time(0);

    memset(&xhdr, 0, sizeof(HDR));

    str_stamp(date, &now);
    acct_load(&cacct, cuser.userid);
    hdr = song_get(xo, pos, fpath);


    if (!cuser.userlevel)
        return XO_NONE;

    if (cacct.request < 1)
    {
        vmsg_xo(xo, "點歌次數已用完！");
        return XO_FOOT;
    }
    method = 0;
    if (acct_get("要點歌給誰：", &acct) < 1)
        return XO_HEAD;

    dir = xo->dir;
    strcpy(title, hdr->title);

    usr_fpath(folder, acct.userid, fn_dir);
    method = hdr_stamp(folder, 0, &xhdr, buf);

    xfp = fdopen(method, "w");

    strcpy(xhdr.owner, cuser.userid);
    strcpy(xhdr.nick, cuser.username);
    sprintf(xhdr.title, "%s 點歌給您", cuser.userid);
    strcpy(xhdr.date, date);

    sprintf(tmp, "%s 點歌給 %s", cuser.userid, acct.userid);
    log_song(tmp);

    if (!vget_xo(xo, B_LINES_REF, 0, "想說的話：", want_say, sizeof(want_say), DOECHO))
        strcpy(want_say, ".........");

    fp = fopen(fpath, "r+");

    while (fgets(tmp, 256, fp))
    {
        if (strstr(tmp, SONG_END))
            break;

        while (song_swap(tmp, SONG_SRC, cuser.userid))
            ;
        while (song_swap(tmp, SONG_DES, acct.userid))
            ;
        while (song_swap(tmp, SONG_SAY, want_say))
            ;

        fputs(tmp, xfp);
    }
    now = time(0);
    sprintf(buf, "\x1b[1;33m%s\x1b[m 想對 \x1b[1;33m%s\x1b[m 說 %s\n\x1b[30m%s\x1b[m\n", cuser.userid, acct.userid, want_say, ctime(&now));
    //sprintf(buf, "%s\n", ctime(&now));
    fputs(buf, xfp);
    fclose(fp);


    rec_add(folder, &xhdr, sizeof(HDR));

    fclose(xfp);
    cacct.request -= 1;
    if (cacct.request <= 0)
        cacct.request = 0;
    cuser.request = cacct.request;
    sprintf(buf, "剩餘點歌次數：%d 次", cacct.request);
    vmsg_xo(xo, buf);
    acct_save(&cacct);
    m_biff(acct.userno);
    return XO_INIT;
}

static int
song_edit(
XO *xo,
int pos)
{
    char fpath[80];
    HDR *hdr;

    if (bbsothermode & OTHERSTAT_EDITING)
    {
        vmsg_xo(xo, "你還有檔案還沒編完哦！");
        return XO_FOOT;
    }


    if (!HAS_PERM(PERM_KTV))
        return XO_NONE;
    hdr = song_get(xo, pos, fpath);
    if (hdr)
        vedit(fpath, false);
    return XO_HEAD;
}

static int
song_title(
XO *xo,
int pos)
{
    HDR *ghdr, xhdr;
    int num;
    char *dir;
    char fpath[128];

    ghdr = song_get(xo, pos, fpath);
    if (ghdr == NULL)
        return XO_NONE;

    xhdr = *ghdr;
    vget_xo(xo, B_LINES_REF, 0, "標題：", xhdr.title, TTLEN + 1, GCARRY);

    dir = xo->dir;
    if (HAS_PERM(PERM_SYSOP | PERM_KTV))
    {
        vget_xo(xo, B_LINES_REF, 0, "編者：", xhdr.owner, IDLEN + 1, GCARRY);
        vget_xo(xo, B_LINES_REF, 0, "時間：", xhdr.date, 9, GCARRY);
    }

    if (memcmp(ghdr, &xhdr, sizeof(HDR)) &&
        vans_xo(xo, "確定要修改嗎(y/N)？[N]") == 'y')
    {
        *ghdr = xhdr;
        num = pos;
        rec_put(dir, ghdr, sizeof(HDR), num);
        return XR_FOOT + XO_CUR;

    }
    return XO_FOOT;
}

static int
song_help(
XO *xo)
{
    film_out(FILM_SONG, -1);
    return XO_HEAD;
}

static KeyFuncList song_cb =
{
    {XO_INIT, {song_init}},
    {XO_LOAD, {song_load}},
    {XO_HEAD, {song_head}},
    {XO_BODY, {song_body}},
    {XO_FOOT, {song_foot}},
    {XO_CUR | XO_POSF, {.posf = song_cur}},

    {'r' | XO_POSF, {.posf = song_browse}},
    {'o' | XO_POSF, {.posf = song_order}},
    {'E' | XO_POSF, {.posf = song_edit}},
    {'T' | XO_POSF, {.posf = song_title}},
    {'q', {song_query}},
    {'m' | XO_POSF, {.posf = song_send}},
    {'h', {song_help}},
};


static void
XoSong(
const char *folder,
const char *title,
int level)
{
    XO *xo, *last;
    const char *str;

    last = xz[XZ_OTHER - XO_ZONE].xo;   /* record */

    xz[XZ_OTHER - XO_ZONE].xo = xo = xo_new(folder);
    xo->cb = song_cb;
    xo->recsiz = sizeof(HDR);
    xo->xz_idx = XZ_INDEX_OTHER;
    for (int i = 0; i < COUNTOF(xo->pos); ++i)
        xo->pos[i] = 0;
    xo->key = XZ_OTHER;
    xo->xyz = (void *) "點歌系統";
    str = "系統管理者";
    sprintf(currBM, "板主：%s", str);

    xover(XZ_OTHER);

    free(xo);

    xz[XZ_OTHER - XO_ZONE].xo = last;   /* restore */
}

int
XoSongMain(void)
{
    DL_HOLD;
    char fpath[64];

    strcpy(currboard, BRD_REQUEST);
#ifdef HAVE_BOARD_PAL
    cutmp->board_pal = currbno;
#endif
    bbstate = STAT_STARTED;

    sprintf(fpath, "gem/brd/%s/@/@SongBook", currboard);
    XoSong(fpath, "點歌系統", XZ_OTHER);
    return DL_RELEASE(0);
}

int
XoSongSub(void)
{
    DL_HOLD;
    int chn;
    chn = brd_bno(BRD_REQUEST);
    XoPost(chn);
    xover(XZ_POST);
    return DL_RELEASE(0);
}

int
XoSongLog(void)
{
    DL_HOLD;
    int chn;
    chn = brd_bno(BRD_ORDERSONGS);
    XoPost(chn);
    xover(XZ_POST);
    return DL_RELEASE(0);
}

int
AddRequestTimes(void)
{
    DL_HOLD;
    ACCT acct;
    char buf[5];
    int n;

    if (!vget(B_LINES_REF, 0, "請選擇：1)增加單人 2)條件增加 0)取消 [0]", buf, 3, DOECHO))
        return DL_RELEASE(0);
    if (*buf == '1')
    {
        if (acct_get("要送誰點歌次數：", &acct) < 1)
            return DL_RELEASE(0);
        clrtobot();
        prints("剩餘點歌次數：%d 次。", acct.request);
        if (!vget(3, 0, "加幾次：", buf, 5, DOECHO))
            return DL_RELEASE(0);
        if (vans("確定增加嗎？ [y/N]") == 'y')
        {
            acct.request += atoi(buf);
            acct_save(&acct);
        }
    }
    else if (*buf == '2')
    {
        char buf_n[12];
        n = 0;
        n = bitset(n, NUMPERMS, NUMPERMS, MSG_USERPERM, perm_tbl);
        if (!vget(B_LINES_REF, 0, "加幾次：", buf, 5, DOECHO))
            return DL_RELEASE(0);
        sprintf(buf_n, "%u", n);
        if (vans("確定增加嗎？ [y/N]") == 'y')
            PROC_CMD_BG(BINARY_SUFFIX"addsong", buf_n, buf);
    }
    return DL_RELEASE(0);
}


#endif          /* HAVE_SONG */
