/*-------------------------------------------------------*/
/* maple/myfavorite.c   ( ES_BBS )                       */
/*-------------------------------------------------------*/
/* author : cat.bbs@bbs.es.ncku.edu.tw                   */
/* target : �إ� [�ڪ��̷R��]                            */
/* create : 2005/07/28                                   */
/* update :                                              */
/* note   : for ccns bbs                                 */
/*-------------------------------------------------------*/

#include "bbs.h"

#ifdef  HAVE_FAVORITE

static void XoFavorite(const char *folder, const char *title, int level);
static int myfavorite_add(XO *xo);
static char currdir[64];

void
brd2myfavorite(
    const BRD *brd,
    HDR *gem)
{
    memset(gem, 0, sizeof(HDR));
    time32(&gem->chrono);
    strcpy(gem->xname, brd->brdname);
    sprintf(gem->title, "%-16s%s", brd->brdname, brd->title);
    gem->xmode = GEM_BOARD;
}

static void
myfavorite_item(
    int num,
    const HDR *myfavorite)
{
    if (myfavorite->xmode & GEM_BOARD)
    {
        if (myfavorite->recommend == -1)
            prints("%6d   %-*s < ���ݪO�w���s�b >\n", num, IDLEN, myfavorite->xname);
        else
        {
            BRD *brd;
            int chn;

            chn = myfavorite->recommend;
            brd = bshm->bcache + chn;
            brh_get(brd->bstamp, chn);

            board_outs(chn, num);
        }
    }
    else if (myfavorite->xmode & GEM_GOPHER)
    {
        prints("%6d   �� %s ��ذϱ��|\n", num, myfavorite->xname);
    }
    else if (myfavorite->xmode & GEM_HTTP)
    {
        outs("  ");
        outsep(b_cols, "----------------------------------------------------------------------------");
        outc('\n');
    }
    else
    {
        class_outs(myfavorite->title, num);
    }
}

static int
myfavorite_cur(
    XO *xo,
    int pos)
{
    const HDR *const myfavorite = (const HDR *) xo_pool_base + pos;
    move(3 + pos - xo->top, 0);
    myfavorite_item(pos + 1, myfavorite);
    return XO_NONE;
}

static int
myfavorite_body(
    XO *xo)
{
    const HDR *myfavorite;
    int num, max, tail;

    move(3, 0);
    clrtobot();
    max = xo->max;
    if (max <= 0)
    {
        return myfavorite_add(xo);
    }

    num = xo->top;
    myfavorite = (const HDR *) xo_pool_base + num;
    tail = num + XO_TALL;
    max = BMIN(max, tail);

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
    vs_head("�ڪ��̷R", str_site);
    prints(NECK_MYFAVORITE,
        cuser.ufo2 & UFO2_BRDNEW ? "�`��" : "�s��", d_cols + 33, "��   ��   ��   �z");
    return myfavorite_body(xo);
}

static int
myfavorite_newmode(
    XO *xo)
{
    cuser.ufo2 ^= UFO2_BRDNEW;  /* Thor.980805: �S�O�`�N utmp.ufo���P�B���D */
    return XO_HEAD;
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
    return XO_LOAD;
}

static int
myfavorite_browse(
    XO *xo,
    int pos)
{
    const HDR *ghdr;
    int xmode, op=0, chn;
    char fpath[80], title[TTLEN + 1];

    ghdr = (const HDR *) xo_pool_base + pos;

    xmode = ghdr->xmode;
    /* browse folder */

    if (xmode & GEM_HTTP)
        return XO_NONE;

    if (xmode & GEM_BOARD)
    {
        if (ghdr->recommend == -1)
            return XO_NONE;
        chn = brd_bno(ghdr->xname);
        if (XoPost(chn))
        {
            xover(XZ_POST);
            time(&brd_visit[chn]);
        }
    }
    else if (xmode & GEM_GOPHER)
    {
        sprintf(fpath, "gem/brd/%s/.DIR", ghdr->xname);

        chn = brd_bno(ghdr->xname);/*20100916.float �i�J��ذϵ��P�i�J�ݪO*/
        XoPost(chn);                            /*����Q�Ψ�L�ݪO�����D�v���ק�L����ذ� ���[����w�峹*/

        op = HAS_PERM(PERM_ALLBOARD) ? GEM_SYSOP : (bbstate & STAT_BOARD ? GEM_MANAGER : GEM_USER);
        strcpy(title, ghdr->title);
        XoGem(fpath, title, op);
        return XO_INIT;
    }
    else
    {
        char buf[40];

        op = xo->key;

        sprintf(buf, "MF/%s", ghdr->xname);
        usr_fpath(fpath, cuser.userid, buf);
        strcpy(title, ghdr->title);
        XoFavorite(fpath, title, op);
        return XO_INIT;
    }
    return XO_INIT;
}

static int
myfavorite_find_same(
    const BRD *brd,
    const char *dir)
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
        vmsg("�|���q�L�{�ҡA�L�k�s�W�ڪ��̷R�I");
        return XO_QUIT;
    }
    memset(&hdr, 0, sizeof(HDR));
    ans = vans("�s�W (B)�ݪO���| (F)��Ƨ� (G)��ذϱ��| (L)���j�u (Q)���} [Q]");

    if (ans == 'b')
    {
        brd = ask_board(buf, BRD_R_BIT, NULL);
        if (brd == NULL)
        {
            vmsg(ERR_BID);
            return XO_HEAD;
        }
        brd2myfavorite(brd, &hdr);
        if (myfavorite_find_same(brd, currdir) >= 0)
        {
            vmsg("�w�����ݪO!");
            return XO_FOOT;
        }
    }
    else if (ans == 'f')
    {
        char title[64];
        char fpath[64];

        if (!vget(B_LINES_REF, 0, "�п�J���D: ", title, sizeof(title), DOECHO))
            return XO_NONE;

        hdr_stamp(currdir, ans|HDR_LINK, &hdr, fpath);
        hdr.xmode = GEM_FOLDER;
        sprintf(hdr.title, "�� %s", title);
    }
    else if (ans == 'g')
    {
        brd = ask_board(buf, BRD_R_BIT, NULL);
        if (brd == NULL)
        {
            vmsg(ERR_BID);
            return XO_HEAD;
        }
        brd2myfavorite(brd, &hdr);
        hdr.xmode = GEM_GOPHER;

    }
    else if (ans == 'l')
    {
        hdr.xmode = GEM_HTTP;
        //sprintf(hdr.title, "�� %s", title);
    }
    else
    {
        if (!xo->max)
            return XO_QUIT;
        return XO_FOOT;
    }

    ans = vans("�s���m A)ppend I)nsert N)ext Q)uit [A] ");

    if (ans == 'q')
    {
        return XO_FOOT;
    }

    if (ans == 'i' || ans == 'n')
        rec_ins(currdir, &hdr, sizeof(HDR), xo->pos + (ans == 'n'), 1);
    else
        rec_add(currdir, &hdr, sizeof(HDR));

    logitfile(FN_FAVORITE_LOG, "< ADD >", hdr.xname);

    return XO_LOAD;
}

static void
remove_dir(
    const char *fpath)
{
    HDR hdr;
    int max, i;
    char buf[40], path[80];

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
    XO *xo,
    int pos)
{
    if (!HAS_PERM(PERM_VALID))
        return XO_NONE;

    if (vans(msg_del_ny) == 'y')
    {
        const HDR *hdr = (const HDR *) xo_pool_base + pos;
        const HDR hdr_orig = *hdr;

        if (hdr->xmode & GEM_FOLDER)
        {
            char buf[40];
            char fpath[64];

            sprintf(buf, "MF/%s", hdr->xname);
            usr_fpath(fpath, cuser.userid, buf);
            remove_dir(fpath);
        }

        if (!rec_del(currdir, sizeof(HDR), pos, NULL, NULL))
        {
            logitfile(FN_FAVORITE_LOG, "< DEL >", hdr_orig.xname);
            return XO_LOAD;
        }
    }
    return XO_FOOT;
}

static int
myfavorite_mov(
    XO *xo,
    int pos)
{
    const HDR *ghdr;
    char buf[80];
    int newOrder;

    if (!HAS_PERM(PERM_VALID))
        return XO_NONE;
    ghdr = (const HDR *) xo_pool_base + pos;

    sprintf(buf + 5, "�п�J�� %d �ﶵ���s��m�G", pos + 1);
    if (!vget(B_LINES_REF, 0, buf + 5, buf, 5, DOECHO))
        return XO_FOOT;

    newOrder = TCLAMP(atoi(buf) - 1, 0, xo->max - 1);

    if (newOrder != pos)
    {
        const HDR ghdr_orig = *ghdr;
        if (!rec_del(currdir, sizeof(HDR), pos, NULL, NULL))
        {
            rec_ins(currdir, &ghdr_orig, sizeof(HDR), newOrder, 1);
            xo->pos = newOrder;
            logitfile(FN_FAVORITE_LOG, "< MOV >", ghdr_orig.xname);
            return XO_LOAD;
        }
    }
    return XO_FOOT;
}

static int
myfavorite_edit(
    XO *xo,
    int pos)
{
    HDR *hdr;

    if (!HAS_PERM(PERM_VALID))
        return XO_NONE;
    hdr = (HDR *) xo_pool_base + pos;
    if (hdr->xmode & GEM_BOARD)
    {
        int chn;

        if (!HAS_PERM(PERM_BOARD))
            return XO_NONE;

        chn = hdr->recommend;
        if (chn >= 0)
        {
            brd_edit(chn);
            return XO_INIT;
        }

    }
    else if (hdr->xmode & GEM_FOLDER)
    {
        if (!vget(B_LINES_REF, 0, "�п�J���D: ", hdr->title, 64, GCARRY))
            return XO_FOOT;
        rec_put(currdir, hdr, sizeof(HDR), pos);
        return XO_LOAD;
    }
    return XO_NONE;

}


static int
myfavorite_help(
    XO *xo)
{
    film_out(FILM_FAVORITE, -1);
    return XO_HEAD;
}

static int
myfavorite_search(
    XO *xo,
    int pos)
{
    int num, max;
    char *ptr;
    char buf[IDLEN + 1];
    const HDR *hdr;

    ptr = buf;
    pos = vget(B_LINES_REF, 0, "�п�J�j�M����r�G", ptr, IDLEN + 1, DOECHO);
    move(b_lines, 0);
    clrtoeol();

    if (pos)
    {
        int chn;
        const BRD *brd;

        //bcache = bshm->bcache;

        //str_lower(ptr, ptr);
        num = pos;
        max = xo->max;
        //chp = (short *) xo->xyz;
        do
        {
            if (++pos >= max)
                pos = 0;

            hdr = (const HDR *) xo_pool_base + pos;

            if (hdr->xmode & GEM_BOARD)
            {
                chn = hdr->recommend;
                brd = bshm->bcache + chn;
                //vmsg(ptr);

                if (strstr(brd->brdname, ptr) || strstr(brd->title, ptr))
                    return XO_MOVE + pos;
            }
            else if (hdr->xmode & GEM_FOLDER)
            {
                if (strstr(hdr->title, ptr))
                    return XO_MOVE + pos;
            }
            //chn = chp[pos];
            //if (chn >= 0)
            //{
                //brd = bcache + chn;
                //if (str_casestr(brd->brdname, ptr) || str_casestr(brd->title, ptr))
                    //return XO_MOVE + pos;
            //}
        } while (pos != num);
    }

    return XO_NONE;
}


KeyFuncList myfavorite_cb =
{
    {XO_INIT, {myfavorite_init}},
    {XO_LOAD, {myfavorite_load}},
    {XO_HEAD, {myfavorite_head}},
    {XO_BODY, {myfavorite_body}},
    {XO_CUR | XO_POSF, {.posf = myfavorite_cur}},

    {Ctrl('P'), {myfavorite_add}},
    {'a', {myfavorite_add}},
    {'r' | XO_POSF, {.posf = myfavorite_browse}},
    {'s', {myfavorite_switch}},
    {'c', {myfavorite_newmode}},
    {'d' | XO_POSF, {.posf = myfavorite_delete}},
    {'M' | XO_POSF, {.posf = myfavorite_mov}},
    {'E' | XO_POSF, {.posf = myfavorite_edit}},
    {'/' | XO_POSF, {.posf = myfavorite_search}},
    {'h', {myfavorite_help}}
};

static void
XoFavorite(
    const char *folder,
    const char *title,
    int level)
{
    XO *xo, *last;
    char old[64];

    last = xz[XZ_MYFAVORITE - XO_ZONE].xo;     /* record */

    strcpy(old, currdir);

    strcpy(currdir, folder);

    xz[XZ_MYFAVORITE - XO_ZONE].xo = xo = xo_new(folder);
    xo->cb = myfavorite_cb;
    xo->recsiz = sizeof(HDR);
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

    XoFavorite(fpath, "�ڪ��̷R", XZ_MYFAVORITE);

    return XO_HEAD;
}

int
myfavorite_find_chn(
    const char *brdname)
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
        char old[80], new_[80];
        usr_fpath(old, cuser.userid, FN_FAVORITE);
        usr_fpath(new_, cuser.userid, FN_MYFAVORITE);
        f_cp(old, new_, 0600);
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
    XO *xo,
    int pos)
{
    HDR hdr;
    char fpath[64];
    short chn, *chp;
    BRD *brd;

    usr_fpath(fpath, cuser.userid, FN_MYFAVORITE);

    chp = (short *) xo->xyz + pos;
    chn = *chp;
    if (chn < 0)
    {
        return XO_NONE;
    }

    brd = bshm->bcache + chn;

    if (myfavorite_find_same(brd, fpath) >= 0)
    {
        vmsg("�w�����ݪO!");
        return XO_FOOT;
    }

    memset(&hdr, 0, sizeof(HDR));
    brd2myfavorite(brd, &hdr);

    rec_add(fpath, &hdr, sizeof(HDR));
    logitfile(FN_FAVORITE_LOG, "< ADD >", hdr.xname);
    vmsg("�w�[�J�ڪ��̷R");

    return XO_FOOT;
}

#endif  /* HAVE_FAVORITE */

