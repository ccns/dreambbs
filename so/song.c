#define _ADMIN_C_

#include "bbs.h"

#ifdef HAVE_SONG

extern BCACHE *bshm;
extern XZ xz[];
extern char xo_pool[];

static void XoSong(char *folder, char *title, int level);
static int song_order(XO *xo);

#define GEM_READ        1       /* readable */
#define GEM_WRITE       2       /* writable */
#define GEM_FILE        4       /* 預期是檔案 */

#define SONG_SRC        "<~Src~>"
#define SONG_DES        "<~Des~>"
#define SONG_SAY        "<~Say~>"
#define SONG_END        "<~End~>"

static void
log_song(
char *msg)
{
    char buf[512];
    time_t now = time(0);
    sprintf(buf, "%s %-13s %s\n", Etime(&now), cuser.userid, msg);
    f_cat(FN_SONG_LOG, buf);
}



static int
song_swap(
char *str,
char *src,
char *des)
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
char *fpath)
{
    HDR *ghdr;
    int gtype, level;
    char *folder;

    level = xo->key;

    ghdr = (HDR *) xo_pool + (xo->pos - xo->top);
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
char *fpath)
{
    HDR *ghdr;
    int gtype, level;
    char *folder;

    level = xo->key;

    ghdr = (HDR *) xo_pool + (xo->pos - xo->top);
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

static void
song_item(
int num,
HDR *ghdr)
{
    int xmode, gtype;

    xmode = ghdr->xmode;
    gtype = (char) 0xba;
    if (xmode & GEM_FOLDER)
        gtype += 1;
    if (xmode & GEM_GOPHER)
        gtype += 2;
    prints("%6d%c \241%c ", num, (xmode & GEM_RESTRICT) ? ')' : (xmode & GEM_LOCK) ? 'L' :  ' ', gtype);

    gtype = 0;

    if (!HAS_PERM(PERM_SYSOP) && (xmode & (GEM_RESTRICT | GEM_LOCK)))
        prints("\x1b[1;33m資料保密！\x1b[m\n");
    else if ((gtype == 0) || (xmode & GEM_GOPHER))
        prints("%-.64s\n", ghdr->title);
}


static int
song_body(
XO *xo)
{
    HDR *ghdr;
    int num, max, tail;

    max = xo->max;
    if (max <= 0)
    {
        outs("\n\n《歌本》尚在吸取天地間的日精月華 :)");
        vmsg(NULL);
        return XO_QUIT;
    }

    ghdr = (HDR *) xo_pool;
    num = xo->top;
    tail = num + XO_TALL;
    if (max > tail)
        max = tail;

    move(3, 0);
    do
    {
        song_item(++num, ghdr++);
    }
    while (num < max);
    clrtobot();

    return XO_NONE;
}


static int
song_head(
XO *xo)
{

    vs_head("精華文章", xo->xyz);

    outs(
        "  [←]離開 [→]瀏覽 [o]點歌到動態看板 [m]點歌到信箱 [q]查詢剩餘次數 [h]說明 \n");
    outs("\x1b[44m"
        "  編號     主              題                           [編      選] [日  期]\x1b[m");
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
XO *xo)
{
    HDR *ghdr;
    int xmode, op = 0;
    char fpath[80], title[TTLEN + 1];

    do
    {
        ghdr = song_check(xo, fpath);
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
            return song_init(xo);
        }

        /* browse article */

        /* Thor.990204: 為考慮more 傳回值 */
        if ((xmode = more(fpath, MSG_GEM)) == -2)
            return XO_INIT;
        if (xmode == -1)
            break;

        op = GEM_READ | GEM_FILE;

        xmode = xo_getch(xo, xmode);

    }
    while (xmode == XO_BODY);

    if (op != GEM_READ)
        song_head(xo);
    return XO_NONE;
}

#if 1
static int
song_order(
XO *xo)
{
    char xboard[20], fpath[80], xfolder[80], xtitle[80], *dir, buf[128];
    char tmp[256], idwho[20], want_say[32];
    HDR *hdr, xpost;
    int method, battr, flag;
    FILE *xfp, *fp;
    ACCT acct;
    time_t token, now;

    memset(&xpost, 0, sizeof(HDR));
    acct_load(&acct, cuser.userid);

    hdr = song_get(xo, fpath);
    if (hdr == NULL)
        return XO_NONE;

    if (!cuser.userlevel)
        return XO_NONE;
    if (acct.request < 1)
    {
        vmsg("點歌次數已用完！");
        return XO_NONE;
    }


    if (!vget(b_lines, 0, "點歌給誰：", idwho, sizeof(idwho), DOECHO))
        strcpy(idwho, "大家");
    if (!vget(b_lines, 0, "想說的話：", want_say, sizeof(want_say), DOECHO))
        strcpy(want_say, ".........");

    if (vans("要匿名嗎 [y/N]：") == 'y')
        flag = 1;
    else
        flag = 0;

    if (vans("確定點歌嗎 [y/N]：") != 'y' )
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

        while (song_swap(tmp, SONG_SRC, flag ? "某人" : cuser.userid));
        while (song_swap(tmp, SONG_DES, idwho));
        while (song_swap(tmp, SONG_SAY, want_say));

        fputs(tmp, xfp);
    }
    sprintf(buf, "\x1b[1;33m%s\x1b[m 想對 \x1b[1;33m%s\x1b[m 說 %s\n\x1b[30m%s\x1b[m\n", flag ? "某人" : cuser.userid, idwho, want_say, ctime(&now));
    fputs(buf, xfp);

    fclose(fp);

    fclose(xfp);
    close(method);

    acct_load(&acct, cuser.userid);
    acct.request -= 1;
    cuser.request = acct.request;
    sprintf(buf, "剩餘點歌次數：%d 次", acct.request);
    vmsg(buf);
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
    vmsg(buf);
    return XO_HEAD;
}

static int
song_send(
XO *xo)
{
    char fpath[128], folder[128], *dir, title[80], buf[128], want_say[32], date[9];
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
    hdr = song_get(xo, fpath);


    if (!cuser.userlevel)
        return XO_NONE;

    if (cacct.request < 1)
    {
        vmsg("點歌次數已用完！");
        return XO_NONE;
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

    if (!vget(b_lines, 0, "想說的話：", want_say, sizeof(want_say), DOECHO))
        strcpy(want_say, ".........");

    fp = fopen(fpath, "r+");

    while (fgets(tmp, 256, fp))
    {
        if (strstr(tmp, SONG_END))
            break;

        while (song_swap(tmp, SONG_SRC, cuser.userid));
        while (song_swap(tmp, SONG_DES, acct.userid));
        while (song_swap(tmp, SONG_SAY, want_say));

        fputs(tmp, xfp);
    }
    now = time(0);
    sprintf(buf, "\x1b[1;33m%s\x1b[m 想對 \x1b[1;33m%s\x1b[m 說 %s\n\x1b[30m%s\x1b[m\n", cuser.userid, acct.userid, want_say, ctime(&now));
    //sprintf(buf, "%s\n", ctime(&now));
    fputs(buf, xfp);
    fclose(fp);


    rec_add(folder, &xhdr, sizeof(HDR));

    fclose(xfp);
    close(method);
    cacct.request -= 1;
    if (cacct.request <= 0) cacct.request = 0;
    cuser.request = cacct.request;
    sprintf(buf, "剩餘點歌次數：%d 次", cacct.request);
    vmsg(buf);
    acct_save(&cacct);
    m_biff(acct.userno);
    return XO_INIT;
}

static int
song_edit(
XO *xo)
{
    char fpath[80];
    HDR *hdr;

    if (bbsothermode & OTHERSTAT_EDITING)
    {
        vmsg("你還有檔案還沒編完哦！");
        return XO_FOOT;
    }


    if (!HAS_PERM(PERM_KTV))
        return XO_NONE;
    hdr = song_get(xo, fpath);
    if (hdr)
        vedit(fpath, NA);
    return song_head(xo);
}

static int
song_title(
XO *xo)
{
    HDR *ghdr, xhdr;
    int num;
    char *dir;
    char fpath[128];

    ghdr = song_get(xo, fpath);
    if (ghdr == NULL)
        return XO_NONE;

    xhdr = *ghdr;
    vget(b_lines, 0, "標題：", xhdr.title, TTLEN + 1, GCARRY);

    dir = xo->dir;
    if (cuser.userlevel & (PERM_SYSOP | PERM_KTV))
    {
        vget(b_lines, 0, "編者：", xhdr.owner, IDLEN + 2, GCARRY);
        vget(b_lines, 0, "時間：", xhdr.date, 9, GCARRY);
    }

    if (memcmp(ghdr, &xhdr, sizeof(HDR)) &&
        vans("確定要修改嗎(Y/N)？[N]") == 'y')
    {
        *ghdr = xhdr;
        num = xo->pos;
        rec_put(dir, ghdr, sizeof(HDR), num);
        num++;
        move(num - xo->top + 2, 0);
        song_item(num, ghdr);

    }
    return XO_FOOT;
}

static int
song_help(
XO *xo)
{
    film_out(FILM_SONG, -1);
    return song_head(xo);
}

static KeyFunc song_cb[] =
{
    {XO_INIT, song_init},
    {XO_LOAD, song_load},
    {XO_HEAD, song_head},
    {XO_BODY, song_body},

    {'r', song_browse},
    {'o', song_order},
    {'E', song_edit},
    {'T', song_title},
    {'q', song_query},
    {'m', song_send},
    {'h', song_help}
};


static void
XoSong(
char *folder,
char *title,
int level)
{
    XO *xo, *last;
    char *str;

    last = xz[XZ_OTHER - XO_ZONE].xo;   /* record */

    xz[XZ_OTHER - XO_ZONE].xo = xo = xo_new(folder);
    xz[XZ_OTHER - XO_ZONE].cb = song_cb;
    xo->pos = 0;
    xo->key = XZ_OTHER;
    xo->xyz = "點歌系統";
    str = "系統管理者";
    sprintf(currBM, "板主：%s", str);

    xover(XZ_OTHER);

    free(xo);

    xz[XZ_OTHER - XO_ZONE].xo = last;   /* restore */
}

int
XoSongMain(void)
{
    char fpath[64];

    strcpy(currboard, BRD_REQUEST);
#ifdef HAVE_BOARD_PAL
    cutmp->board_pal = brd_bno(currboard);
#endif
    bbstate = STAT_STARTED;

    sprintf(fpath, "gem/brd/%s/@/@SongBook", currboard);
    XoSong(fpath, "點歌系統", XZ_OTHER);
    return 0;
}

int
XoSongSub(void)
{
    int chn;
    chn = brd_bno(BRD_REQUEST);
    XoPost(chn);
    xover(XZ_POST);
    return 0;
}

int
XoSongLog(void)
{
    int chn;
    chn = brd_bno(BRD_ORDERSONGS);
    XoPost(chn);
    xover(XZ_POST);
    return 0;
}

int
AddRequestTimes(void)
{
    ACCT acct;
    char buf[128];
    int n, times;

    if (!vget(b_lines, 0, "請選擇：1)增加單人 2)條件增加 0)取消 [0]", buf, 3, DOECHO))
        return 0;
    if (*buf == '1')
    {
        if (acct_get("要送誰點歌次數：", &acct) < 1)
            return 0;
        clrtobot();
        prints("剩餘點歌次數：%d 次。", acct.request);
        if (!vget(3, 0, "加幾次：", buf, 5, DOECHO))
            return 0;
        if (vans("確定增加嗎？ [y/N]") == 'y')
        {
            acct.request += atoi(buf);
            acct_save(&acct);
        }
    }
    else if (*buf == '2')
    {
        n = 0;
        n = bitset(n, NUMPERMS, NUMPERMS, MSG_USERPERM, perm_tbl);
        if (!vget(b_lines, 0, "加幾次：", buf, 5, DOECHO))
            return 0;
        times = atoi(buf);
        sprintf(buf, BINARY_PREFIX"addsong %d %d &", n, times);
        if (vans("確定增加嗎？ [y/N]") == 'y')
            system(buf);
    }
    return 0;
}


#endif          /* HAVE_SONG */
