/*-------------------------------------------------------*/
/* maple/myfavorite.c   ( ES_BBS )                       */
/*-------------------------------------------------------*/
/* author : cat.bbs@bbs.es.ncku.edu.tw                   */
/* target : 建立 [我的最愛區]                            */
/* create : 2005/07/28                                   */
/* update :                                              */
/* note   : for ccns bbs                                 */
/*-------------------------------------------------------*/

#include "bbs.h"

#ifdef  HAVE_FAVORITE
extern BCACHE *bshm;

extern XZ xz[];
extern char brd_bits[MAXBOARD];
extern int ok;

static void XoFavorite(char *folder, char *title, int level);
static int myfavorite_add(XO *xo);
static char currdir[64];

time_t brd_visit[MAXBOARD];     /* 最近瀏覽時間 */

void
brd2myfavorite(
    BRD *brd,
    HDR *gem)
{
    memset(gem, 0, sizeof(HDR));
    time(&gem->chrono);
    strcpy(gem->xname, brd->brdname);
    sprintf(gem->title, "%-16s%s", brd->brdname, brd->title );
    gem->xmode = GEM_BOARD;
}

static void
myfavorite_item(
    int num,
    HDR *myfavorite)
{
    if (myfavorite->xmode & GEM_BOARD)
    {
        if (myfavorite->recommend == -1)
            prints("%6d   %-13s< 本看板已不存在 >\n", num, myfavorite->xname);
        else
        {
            BRD *brd;
            char str[20], buf[10], *bits, *str2, buf2[20], brdtype;
            int chn, brdnew, bno;

            chn = myfavorite->recommend;
            brd = bshm->bcache + chn;

            brh_get(brd->bstamp, chn);

//          if (cuser.ufo2 & UFO2_ENHANCE)
//              sprintf(str, "%c\x1b[1;32m%c\x1b[m", (brd->readlevel & PERM_SYSOP) ? ')' : ' ', brd->blast > brd_visit[chn] ? '+' : ' ');
//          else
//              sprintf(str, "%c\x1b[1;32m%c\x1b[m", (brd->readlevel & PERM_SYSOP) ? ')' : ' ', brh_unread(brd->blast) ? '+' : ' ');
            brdnew = cuser.ufo2 & UFO2_BRDNEW;
            bits = brd_bits;

            sprintf(buf, "%d;3%d", brd->color/10, brd->color%10);

            if (!brdnew)
            {
                int fd, fsize;
                char folder[64];
                struct stat st;

                brd_fpath(folder, brd->brdname, fn_dir);
                if ((fd = open(folder, O_RDONLY)) >= 0)
                {
                    fstat(fd, &st);

                if (st.st_mtime > brd->btime)  // 上次統計後，檔案有修改過
                {
                    if ((fsize = st.st_size) >= sizeof(HDR))
                    {
                        HDR hdr;

                        brd->bpost = fsize / sizeof(HDR);
                        while ((fsize -= sizeof(HDR)) >= 0)
                        {
                            lseek(fd, fsize, SEEK_SET);
                            read(fd, &hdr, sizeof(HDR));
                            if (!(hdr.xmode & (POST_LOCK | POST_BOTTOM)))
                                break;
                        }
                        brd->blast = hdr.chrono;
                    }
                    else
                            brd->blast = brd->bpost = 0;
                    }

                    close(fd);
                }

        }

        else
        {
            int fd, fsize;
            char folder[64];
            struct stat st;

            brd_fpath(folder, brd->brdname, fn_dir);
            if ((fd = open(folder, O_RDONLY)) >= 0)
            {
                fstat(fd, &st);

            if (st.st_mtime > brd->btime)  // 上次統計後，檔案有修改過
            {
                if ((fsize = st.st_size) >= sizeof(HDR))
                {
                    HDR hdr;

                    brd->bpost = fsize / sizeof(HDR);
                    while ((fsize -= sizeof(HDR)) >= 0)
                    {
                        lseek(fd, fsize, SEEK_SET);
                        read(fd, &hdr, sizeof(HDR));
                        if (!(hdr.xmode & (POST_LOCK | POST_BOTTOM)))
                            break;
                    }
                    brd->blast = (hdr.chrono > brd->blast) ? hdr.chrono : brd->blast;
                }
                else
                        brd->blast = brd->bpost = 0;
                }

                close(fd);
            }
            num = brd->bpost;
        }

        sprintf(str, "%s", brd->blast > brd_visit[chn] ? "\x1b[1;31m★\x1b[m" : "☆");

/* 081122.cache:看板性質, 不訂閱, 秘密, 好友, 一般 */
            if (bits[chn] & BRD_Z_BIT)
                brdtype = '-';
#ifdef HAVE_MODERATED_BOARD
            else if (brd->readlevel & PERM_BOARD)
                brdtype = '.';
            else if (brd->readlevel & PERM_SYSOP)
                brdtype = ')';
#endif
            else
                brdtype = ' ';

/* 處理 人氣 */ /* cache.20090416: 仿ptt變色*/
            bno = brd - bshm->bcache;
            bno = bshm->mantime[bno];
            str2 = buf2;

            if (bno > 4999)
                str2 = "\x1b[1;32m TOP \x1b[m";
            else if (bno > 999)
                str2 = "\x1b[1;32m  夯 \x1b[m";
            else if (bno > 799)
                str2 = "\x1b[1;35m  夯 \x1b[m";
            else if (bno > 699)
                str2 = "\x1b[1;33m  夯 \x1b[m";
            else if (bno > 599)
                str2 = "\x1b[1;42m  爆 \x1b[m";
            else if (bno > 499)
                str2 = "\x1b[1;45m  爆 \x1b[m";
            else if (bno > 449)
                str2 = "\x1b[1;44m  爆 \x1b[m";
            else if (bno > 399)
                str2 = "\x1b[1;32m  爆 \x1b[m";
            else if (bno > 349)
                str2 = "\x1b[1;35m  爆 \x1b[m";
            else if (bno > 299)
                str2 = "\x1b[1;33m  爆 \x1b[m";
            else if (bno > 249)
                str2 = "\x1b[1;36m  爆 \x1b[m";
            else if (bno > 199)
                str2 = "\x1b[1;31m  爆 \x1b[m";
            else if (bno > 149)
                str2 = "\x1b[1;37m  爆 \x1b[m";
            else if (bno > 99)
                str2 = "\x1b[1;31m HOT \x1b[m";
            else if (bno > 49)
                str2 = "\x1b[1;37m HOT \x1b[m";
            else if (bno > 1) /* r2.170810: let somebody know which board is still "alive" :P */
                sprintf(str2, "  %2d ", bno);
            else
                str2 = "     ";
//注意有三格空白, 因為 HOT 是三個 char 故更改排版

            //sprintf(buf, "%d;3%d", brd->color/10, brd->color%10);
            prints("%6d%s%c%-13s\x1b[%sm%-4s \x1b[m%-*.*s%s%.13s\n", num, str,
                brdtype, brd->brdname, buf, brd->class, d_cols + 33, d_cols + 32, brd->title, brd->bvote ? "\x1b[1;33m  投\x1b[m " : str2, brd->BM);

            //prints("%6d%s%c%-13s\x1b[%sm%-4s \x1b[m%-*.*s%c %.13s\n", num, str,
                //bits[chn] & BRD_Z_BIT ? '-' : ' ', brd->brdname,
                //buf, brd->class, d_cols + 36, d_cols + 36, brd->title, brd->bvote ? 'V' : ' ', brd->BM);

        }
    }
    else if (myfavorite->xmode & GEM_GOPHER)
    {
        prints("%6d   ■ %s 精華區捷徑\n", num, myfavorite->xname);
    }
    else if (myfavorite->xmode & GEM_HTTP)
    {
        prints("  ----------------------------------------------------------------------------\n");
    }
    else
    {
        prints("%6d   %s\n", num, myfavorite->title);
    }
}

static int
myfavorite_body(
    XO *xo)
{
    HDR *myfavorite;
    int num, max, tail;

    move(3, 0);
    clrtobot();
    max = xo->max;
    if (max <= 0)
    {
        return myfavorite_add(xo);
    }

    myfavorite = (HDR *) xo_pool;
    num = xo->top;
    tail = num + XO_TALL;
    if (max > tail)
        max = tail;

    do
    {
        myfavorite_item(++num, myfavorite++);
    } while (num < max);

    return XO_NONE;
}


static int
myfavorite_head(
    XO *xo)
{
    vs_head("我的最愛", str_site);
    prints(NECK_MYFAVORITE,
        cuser.ufo2 & UFO2_BRDNEW ? "總數" : "編號", d_cols + 33, "中   文   敘   述");
    return myfavorite_body(xo);
}

static int
myfavorite_newmode(
    XO *xo)
{
    cuser.ufo2 ^= UFO2_BRDNEW;  /* Thor.980805: 特別注意 utmp.ufo的同步問題 */
    return myfavorite_head(xo);
}


static int
myfavorite_load(
    XO *xo)
{
    myfavorite_main();
    xo_load(xo, sizeof(HDR));
    return myfavorite_head(xo);
}


static int
myfavorite_init(
    XO *xo)
{
    xo_load(xo, sizeof(HDR));
    return myfavorite_head(xo);
}

static int
myfavorite_switch(
    XO *xo)
{
    Select();
    return myfavorite_load(xo);
}

static int
myfavorite_browse(
    XO *xo)
{
    HDR *ghdr;
    int xmode, op=0, chn;
    char fpath[80], title[TTLEN + 1];

    ghdr = (HDR *) xo_pool + (xo->pos - xo->top);

    xmode = ghdr->xmode;
    /* browse folder */

    if ( xmode & GEM_HTTP)
        return XO_NONE;

    if (xmode & GEM_BOARD)
    {
        if (ghdr->recommend == -1)
            return XO_NONE;
        chn = brd_bno(ghdr->xname);
        XoPost(chn);
        if (ok == 1)
        {
            xover(XZ_POST);
            time(&brd_visit[chn]);
        }
    }
    else if (xmode & GEM_GOPHER)
    {
        sprintf(fpath, "gem/brd/%s/.DIR", ghdr->xname);

        chn = brd_bno(ghdr->xname);/*20100916.float 進入精華區等同進入看板*/
        XoPost(chn);                            /*防止利用其他看板的版主權限修改他版精華區 或觀看鎖定文章*/

        op = HAS_PERM(PERM_ALLBOARD) ? GEM_SYSOP : (bbstate & STAT_BOARD ? GEM_MANAGER : GEM_USER);
        strcpy(title, ghdr->title);
        XoGem(fpath, title, op);
        return myfavorite_init(xo);
    }
    else
    {
        char buf[20];

        op = xo->key;

        sprintf(buf, "MF/%s", ghdr->xname);
        usr_fpath(fpath, cuser.userid, buf);
        strcpy(title, ghdr->title);
        XoFavorite(fpath, title, op);
        return myfavorite_init(xo);
    }
    return myfavorite_init(xo);
}

static int
myfavorite_find_same(
    BRD *brd,
    char *dir)
{
    int max, i;
    HDR hdr;

    max = rec_num(dir, sizeof(HDR));

    for (i=0; i<max; i++)
    {
        rec_get(dir, &hdr, sizeof(HDR), i);
        if (!strcmp(hdr.xname, brd->brdname) && hdr.xmode & GEM_BOARD)
            return i;
    }
    return -1;
}

static int
myfavorite_add(
    XO *xo)
{
    char ans;
    char buf[20];
    BRD *brd;
    HDR hdr;

    if (!HAS_PERM(PERM_VALID))
    {
        vmsg("尚未通過認證，無法新增我的最愛！");
        return XO_QUIT;
    }
    memset(&hdr, 0, sizeof(HDR));
    ans = vans("新增 (B)看板捷徑 (F)資料夾 (G)精華區捷徑 (L)分隔線 (Q)離開 [Q]");

    if (ans == 'b')
    {
        brd = ask_board(buf, BRD_R_BIT, NULL);
        if (brd == NULL)
        {
            vmsg("錯誤的看板名稱");
            return XO_HEAD;
        }
        brd2myfavorite(brd, &hdr);
        if (myfavorite_find_same(brd, currdir) >= 0)
        {
            vmsg("已有此看板!");
            return XO_FOOT;
        }
    }
    else if (ans == 'f')
    {
        char title[64];
        char fpath[64];

        if (!vget(b_lines, 0, "請輸入標題: ", title, sizeof(title), DOECHO))
            return XO_NONE;

        hdr_stamp(currdir, ans|HDR_LINK, &hdr, fpath);
        hdr.xmode = GEM_FOLDER;
        sprintf(hdr.title, "◆ %s", title);
    }
    else if (ans == 'g')
    {
        brd = ask_board(buf, BRD_R_BIT, NULL);
        if (brd == NULL)
        {
            vmsg("錯誤的看板名稱");
            return XO_HEAD;
        }
        brd2myfavorite(brd, &hdr);
        hdr.xmode = GEM_GOPHER;

    }
    else if (ans == 'l')
    {
        hdr.xmode = GEM_HTTP;
        //sprintf(hdr.title, "◆ %s", title);
    }
    else
    {
        if (!xo->max)
            return XO_QUIT;
        return XO_FOOT;
    }

    ans = vans("存放位置 A)ppend I)nsert N)ext Q)uit [A] ");

    if (ans == 'q')
    {
        return XO_FOOT;
    }

    if (ans == 'i' || ans == 'n')
        rec_ins(currdir, &hdr, sizeof(HDR), xo->pos + (ans == 'n'), 1);
    else
        rec_add(currdir, &hdr, sizeof(HDR));

    logitfile(FN_FAVORITE_LOG, "< ADD >", hdr.xname);

    return myfavorite_load(xo);
}

static void
remove_dir(
    char *fpath)
{
    HDR hdr;
    int max, i;
    char buf[20], path[80];

    max = rec_num(fpath, sizeof(HDR));
    for (i=0; i<max; i++)
    {
        rec_get(fpath, &hdr, sizeof(HDR), i);
        if (hdr.xmode & GEM_FOLDER)
        {
            sprintf(buf, "MF/%s", hdr.xname);
            usr_fpath(path, cuser.userid, buf);
            remove_dir(path);
        }
    }
    unlink(fpath);
}

static int
myfavorite_delete(
    XO *xo)
{
    if (!HAS_PERM(PERM_VALID))
        return XO_NONE;

    if (vans(msg_del_ny) == 'y')
    {
        HDR *hdr;

        hdr = (HDR *) xo_pool + (xo->pos - xo->top);

        if (hdr->xmode & GEM_FOLDER)
        {
            char buf[20];
            char fpath[64];

            sprintf(buf, "MF/%s", hdr->xname);
            usr_fpath(fpath, cuser.userid, buf);
            remove_dir(fpath);
        }

        if (!rec_del(currdir, sizeof(HDR), xo->pos, NULL, NULL))
        {
            logitfile(FN_FAVORITE_LOG, "< DEL >", hdr->xname);
            return myfavorite_load(xo);
        }
    }
    return XO_FOOT;
}

static int
myfavorite_mov(
    XO *xo)
{
    HDR *ghdr;
    char buf[80];
    int pos, newOrder;

    if (!HAS_PERM(PERM_VALID))
        return XO_NONE;
    ghdr = (HDR *) xo_pool + (xo->pos - xo->top);

    pos = xo->pos;
    sprintf(buf + 5, "請輸入第 %d 選項的新位置：", pos + 1);
    if (!vget(b_lines, 0, buf + 5, buf, 5, DOECHO))
        return XO_FOOT;

    newOrder = atoi(buf) - 1;
    if (newOrder < 0)
        newOrder = 0;
    else if (newOrder >= xo->max)
        newOrder = xo->max - 1;

    if (newOrder != pos)
    {
        if (!rec_del(currdir, sizeof(HDR), pos, NULL, NULL))
        {
            rec_ins(currdir, ghdr, sizeof(HDR), newOrder, 1);
            xo->pos = newOrder;
            logitfile(FN_FAVORITE_LOG, "< MOV >", ghdr->xname);
            return XO_LOAD;
        }
    }
    return XO_FOOT;
}

static int
myfavorite_edit(
    XO *xo)
{
    HDR *hdr;

    if (!HAS_PERM(PERM_VALID))
        return XO_NONE;
    hdr = (HDR *) xo_pool + (xo->pos - xo->top);
    if (hdr->xmode & GEM_BOARD)
    {
        int chn;

        if (!HAS_PERM(PERM_BOARD))
            return XO_NONE;

        chn = hdr->recommend;
        if (chn >= 0)
        {
            brd_edit(chn);
            return myfavorite_init(xo);
        }

    }
    else if (hdr->xmode & GEM_FOLDER)
    {
        if (!vget(b_lines, 0, "請輸入標題: ", hdr->title, 64, GCARRY))
            return XO_FOOT;
        rec_put(currdir, hdr, sizeof(HDR), xo->pos);
        return myfavorite_load(xo);
    }
    return XO_NONE;

}


static int
myfavorite_help(
    XO *xo)
{
    film_out(FILM_FAVORITE, -1);
    return myfavorite_head(xo);
}

static int
myfavorite_search(
    XO *xo)
{
    int num, pos, max;
    char *ptr;
    char buf[IDLEN + 1];
    HDR *hdr;

    ptr = buf;
    pos = vget(b_lines, 0, "請輸入搜尋關鍵字：", ptr, IDLEN + 1, DOECHO);
    move(b_lines, 0);
    clrtoeol();

    if (pos)
    {
        int chn;
        BRD *brd;

        //bcache = bshm->bcache;

        //str_lower(ptr, ptr);
        pos = num = xo->pos;
        max = xo->max;
        //chp = (short *) xo->xyz;
        do
        {
            if (++pos >= max)
                pos = 0;

            hdr = (HDR *) xo_pool + (pos - xo->top);

            if (hdr->xmode & GEM_BOARD)
            {
                chn = hdr->recommend;
                brd = bshm->bcache + chn;
                //vmsg(ptr);

                if (strstr(brd->brdname, ptr) || strstr(brd->title, ptr))
                    return pos + XO_MOVE;
            }
            else if (hdr->xmode & GEM_FOLDER)
            {
                if (strstr(hdr->title, ptr))
                    return pos + XO_MOVE;
            }
            //chn = chp[pos];
            //if (chn >= 0)
            //{
                //brd = bcache + chn;
                //if (str_str(brd->brdname, ptr) || str_str(brd->title, ptr))
                    //return pos + XO_MOVE;
            //}
        } while (pos != num);
    }

    return XO_NONE;
}


KeyFunc myfavorite_cb[] =
{
#ifdef  HAVE_LIGHTBAR
    {XO_ITEM, myfavorite_item_bar},
#endif
    {XO_INIT, myfavorite_init},
    {XO_LOAD, myfavorite_load},
    {XO_HEAD, myfavorite_head},
    {XO_BODY, myfavorite_body},

    {Ctrl('P'), myfavorite_add},
    {'a', myfavorite_add},
    {'r', myfavorite_browse},
    {'s', myfavorite_switch},
    {'c', myfavorite_newmode},
    {'d', myfavorite_delete},
    {'M', myfavorite_mov},
    {'E', myfavorite_edit},
    {'/', myfavorite_search},
    {'h', myfavorite_help}
};

static void
XoFavorite(
    char *folder,
    char *title,
    int level)
{
    XO *xo, *last;
    char old[64];

    last = xz[XZ_MYFAVORITE - XO_ZONE].xo;     /* record */

    strcpy(old, currdir);

    strcpy(currdir, folder);

    xz[XZ_MYFAVORITE - XO_ZONE].xo = xo = xo_new(folder);
    xz[XZ_MYFAVORITE - XO_ZONE].cb = myfavorite_cb;
    xo->pos = 0;
    xo->key = XZ_MYFAVORITE;

    xover(XZ_MYFAVORITE);

    free(xo);

    strcpy(currdir, old);

    xz[XZ_MYFAVORITE - XO_ZONE].xo = last;     /* restore */
}


int
MyFavorite(void)
{
    char fpath[64];

    utmp_mode(M_MYFAVORITE);
    usr_fpath(fpath, cuser.userid, FN_MYFAVORITE);
    myfavorite_main();

    XoFavorite(fpath, "我的最愛", XZ_MYFAVORITE);

    return XO_HEAD;
}

int
myfavorite_find_chn(
    char *brdname)
{
    BRD *bp;
    int max, i;
//  char *userid, bm[40];

    bp = bshm->bcache;
    max = bshm->number;

    for (i=0; i<max; i++, bp++)
    {
        if (!strcmp(bp->brdname, brdname))
        {
        /*
            strcpy(bm, bp->BM);
            if (strstr(bm, cuser.userid))
            {
                userid = (char *) strtok(bm, "/");
                do
                {
                    if (!strcmp(cuser.userid, userid))
                        return i;
                } while (userid = (char *) strtok(NULL, "/"));
            }
            else
            */
            if (HAS_PERM(PERM_SYSOP) || HAS_PERM(PERM_ALLBOARD) /* || !(bp->readlevel & PERM_SYSOP) || bm_belong(brdname) & BRD_R_BIT || (bp->battr & BRD_FRIEND) */ || (Ben_Perm(bp, cuser.userlevel) & (BRD_R_BIT|BRD_F_BIT|BRD_X_BIT)))
                return i;
            else
                break;
        }
    }

    return -1;
}

void
myfavorite_parse(
    char *fpath)
{
    int i, max;
    char buf[20];
    HDR hdr;

    sprintf(buf, "MF/%s", fpath);

    usr_fpath(fpath, cuser.userid, buf);
    max = rec_num(fpath, sizeof(HDR));

    for (i=0; i<max; i++)
    {
        rec_get(fpath, &hdr, sizeof(HDR), i);
        if (hdr.xmode & GEM_BOARD)
        {
            hdr.recommend = myfavorite_find_chn(hdr.xname);
            rec_put(fpath, &hdr, sizeof(HDR), i);
        }
        else if (hdr.xmode & GEM_FOLDER)
        {
            myfavorite_parse(hdr.xname);
        }
    }

}


void
myfavorite_main(void)
{
    int i, max;
    char fpath[80];
    HDR hdr;

    usr_fpath(fpath, cuser.userid, "MF");
    if (!mkdir(fpath, 0700))
    {
        char old[80], new[80];
        usr_fpath(old, cuser.userid, FN_FAVORITE);
        usr_fpath(new, cuser.userid, FN_MYFAVORITE);
        f_cp(old,new,0600);
    }

    usr_fpath(fpath, cuser.userid, FN_MYFAVORITE);
    max = rec_num(fpath, sizeof(HDR));

    for (i=0; i<max; i++)
    {
        rec_get(fpath, &hdr, sizeof(HDR), i);
        if (hdr.xmode & GEM_BOARD)
        {
            hdr.recommend = myfavorite_find_chn(hdr.xname);
            rec_put(fpath, &hdr, sizeof(HDR), i);
        }
        else if (hdr.xmode & GEM_FOLDER)
        {
            myfavorite_parse(hdr.xname);
        }
    }
}

int
class_add(
    XO *xo)
{
    HDR hdr;
    char fpath[64];
    short chn, *chp;
    BRD *brd;

    usr_fpath(fpath, cuser.userid, FN_MYFAVORITE);

    chp = (short *) xo->xyz + xo->pos;
    chn = *chp;
    if (chn < 0)
    {
        return XO_NONE;
    }

    brd = bshm->bcache + chn;

    if (myfavorite_find_same(brd, fpath) >= 0)
    {
        vmsg("已有此看板!");
        return XO_FOOT;
    }

    memset(&hdr, 0, sizeof(HDR));
    brd2myfavorite(brd, &hdr);

    rec_add(fpath, &hdr, sizeof(HDR));
    logitfile(FN_FAVORITE_LOG, "< ADD >", hdr.xname);
    vmsg("已加入我的最愛");

    return XO_FOOT;
}

#endif  /* HAVE_FAVORITE */

