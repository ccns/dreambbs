/*-------------------------------------------------------*/
/* cleanrecommend.c   ( CCNS BBS )                       */
/*-------------------------------------------------------*/
/* author : cat@ccns.ncku.edu.tw                         */
/* target : cleanrecommend                               */
/* create : 2004/12/12                                   */
/* update : NULL                                         */
/*-------------------------------------------------------*/

#undef  MODES_C
#include "bbs.h"

#define NEGATIVE 0
#define POSITIVE 1
#define COMMENT  2

#ifdef  HAVE_RECOMMEND
typedef struct RecommendMessage
{
    char userid[IDLEN + 1];
    char verb[3];
    int32_t pn;
    char msg[55];
    char rtime[6];
}       RMSG;  /* DISKDATA(raw); runtime */


int counter;
char title[80], name[10];

static int
cleanrecommend_log(
    const RMSG *rmsg,
    int mode)   /* 0:partial 1:all */
{
    FILE *fp;
    time_t now;

    if ((fp = fopen(FN_RECOMMEND_LOG, "a+")))
    {
        time(&now);

        fprintf(fp, "%24.24s %s 砍 %s 板 %s(%s) ", ctime(&now), cuser.userid, currboard, title, name);
        if (!mode)
            fprintf(fp, "中 %s 的留言 %s\n", rmsg->userid, rmsg->msg);
        else
            fprintf(fp, "所有留言\n");
        fclose(fp);
    }

    return 0;
}

static void
cleanrecommend_item(
    int num,
    const RMSG *cleanrecommend)
{

    char tmp[10];
    const char *pn;

    pn = tmp;

    if (cleanrecommend->pn == POSITIVE)
    {
        pn = "\x1b[1;33m+";
        prints("%4d%s%2s\x1b[m%-*s %-*s%-5s\n", num, pn, cleanrecommend->verb, IDLEN, cleanrecommend->userid, d_cols + 54, cleanrecommend->msg, cleanrecommend->rtime);
    }
    else if (cleanrecommend->pn == NEGATIVE)
    {
        pn = "\x1b[1;31m-";
        prints("%4d%s%2s\x1b[m%-*s %-*s%-5s\n", num, pn, cleanrecommend->verb, IDLEN, cleanrecommend->userid, d_cols + 54, cleanrecommend->msg, cleanrecommend->rtime);
    }
    else
    {
        pn = " ";
        prints("%4d%s%2s\x1b[m%-*s %-*s%-5s\n", num, pn, cleanrecommend->verb, IDLEN, cleanrecommend->userid, d_cols + 54, cleanrecommend->msg, cleanrecommend->rtime);
    }
}

static int
cleanrecommend_cur(
    XO *xo,
    int pos)
{
    const RMSG *const cleanrecommend = (const RMSG *) xo_pool_base + pos;
    move(3 + pos - xo->top, 0);
    cleanrecommend_item(pos + 1, cleanrecommend);
    return XO_NONE;
}

static int
cleanrecommend_body(
    XO *xo)
{
    const RMSG *cleanrecommend;
    int num, max, tail;

    move(3, 0);
    clrtobot();
    max = xo->max;
    if (max <= 0)
    {
        //counter = 0;
        vmsg("沒有留言");
        return XO_QUIT;
    }
    num = xo->top;
    cleanrecommend = (const RMSG *) xo_pool_base + num;
    tail = num + XO_TALL;
/*
    counter = TCLAMP(max, -127, 127);
*/
    max = BMIN(max, tail);

    do
    {
        cleanrecommend_item(++num, cleanrecommend++);
    } while (num < max);

    return XO_NONE;
}


static int
cleanrecommend_head(
    XO *xo)
{
    vs_head("推薦留言清單", str_site);
    prints(NECK_CLEANRECOMMEND, d_cols, "");
    return cleanrecommend_body(xo);
}


static int
cleanrecommend_load(
    XO *xo)
{
    xo_load(xo, sizeof(RMSG));
    return cleanrecommend_body(xo);
}


static int
cleanrecommend_init(
    XO *xo)
{
    xo_load(xo, sizeof(RMSG));
    return cleanrecommend_head(xo);
}


static int
cleanrecommend_edit(
    RMSG *cleanrecommend,
    int echo)
{
    if (echo == DOECHO)
        memset(cleanrecommend, 0, sizeof(RMSG));
    if (vget(B_LINES_REF, 0, "使用者:", cleanrecommend->userid, sizeof(cleanrecommend->userid), echo)
     && vget(B_LINES_REF, 0, "動詞:", cleanrecommend->verb, sizeof(cleanrecommend->verb), echo)
     && vget(B_LINES_REF, 0, "留言:", cleanrecommend->msg, sizeof(cleanrecommend->msg), echo)
     && vget(B_LINES_REF, 0, "日期:", cleanrecommend->rtime, sizeof(cleanrecommend->rtime), echo))
        return 1;
    else
        return 0;
}


static int
cleanrecommend_delete(
    XO *xo,
    int pos)
{

    if (vans(msg_del_ny) == 'y')
    {
        const RMSG *rmsg = (const RMSG *) xo_pool_base + pos;
        const RMSG rmsg_orig = *rmsg;

        if (!rec_del(xo->dir, sizeof(RMSG), pos, NULL, NULL))
        {
            cleanrecommend_log(&rmsg_orig, 0);
            return XO_LOAD;
        }
    }
    return XO_FOOT;
}

static int
cleanrecommend_change(
    XO *xo,
    int pos)
{
    RMSG *cleanrecommend, mate;

    if (!HAS_PERM(PERM_BOARD))
        return XO_NONE;

    cleanrecommend = (RMSG *) xo_pool_base + pos;

    mate = *cleanrecommend;
    cleanrecommend_edit(cleanrecommend, GCARRY);
    if (memcmp(cleanrecommend, &mate, sizeof(RMSG)))
    {
        rec_put(xo->dir, cleanrecommend, sizeof(RMSG), pos);
        return XR_FOOT + XO_CUR;
    }

    return XO_FOOT;
}

static int
cleanrecommend_cleanall(
    XO *xo)
{
    if (vans("確定要刪除所有的留言嗎？[y/N]") == 'y')
    {
        unlink(xo->dir);
        cleanrecommend_log(NULL, 1);
        return XO_LOAD;
    }
    return XO_FOOT;
}

static int
cleanrecommend_help(
    XO *xo)
{
//  film_out(FILM_RMSG, -1);
    return XO_HEAD;
}

KeyFuncList cleanrecommend_cb =
{
    {XO_INIT, {cleanrecommend_init}},
    {XO_LOAD, {cleanrecommend_load}},
    {XO_HEAD, {cleanrecommend_head}},
    {XO_BODY, {cleanrecommend_body}},
    {XO_CUR | XO_POSF, {.posf = cleanrecommend_cur}},

    {'c' | XO_POSF, {.posf = cleanrecommend_change}},
    {'s', {xo_cb_init}},
    {'d' | XO_POSF, {.posf = cleanrecommend_delete}},
    {'D', {cleanrecommend_cleanall}},
    {'h', {cleanrecommend_help}}
};

int
clean(
    XO *xo,
    int pos)
{
    DL_HOLD;
    XO *xoo, *last;
    const HDR *hdr;
    HDR phdr;
    char fpath[128], buf[256], tmp[128], recommenddb[128];
    FILE *fp;
    RMSG rmsg;
    int i, pushstart;
    time_t chrono;
    struct stat st;
    int total, fd;
    const BRD *brd;
    unsigned int battr;

    counter = 0;
    pushstart = 0;

    if (!(bbstate & STAT_BOARD))
        return DL_RELEASE(0);

    hdr = (const HDR *) xo_pool_base + pos;

    if (!hdr->recommend || hdr->xmode & (POST_DELETE | POST_CANCEL | POST_MDELETE | POST_LOCK | POST_CURMODIFY))
        return DL_RELEASE(XO_NONE);

    chrono = hdr->chrono;
    strcpy(title, hdr->title);
    strcpy(name, hdr->xname);

    hdr_fpath(fpath, xo->dir, hdr);
    sprintf(recommenddb, "tmp/%s.recommenddb", cuser.userid);
    sprintf(tmp, "tmp/%s.clean", cuser.userid);
    unlink(tmp);
    unlink(recommenddb);

    brd = bshm->bcache + currbno;
    battr = brd->battr;

    if ((fp = fopen(fpath, "r")))
    {

/*
        if (brd->battr & BRD_PUSHSNEER)
        {
            if (addscore == 1)
                sprintf(add,                "[[1;33m→ %*s：[[36m%-54.54s [[m%5.5s\n", IDLEN, cuser.userid, msg, Btime(&hdr->pushtime)+3);
            else if (addscore == -1)
                sprintf(add,      "[[1;31m噓[[m [[1;33m%*s：[[36m%-54.54s [[m%5.5s\n", IDLEN, cuser.userid, msg, Btime(&hdr->pushtime)+3);
        }
        else if (brd->battr & BRD_PUSHDEFINE)
        {
            if (addscore == 1)
                sprintf(add, "[[1;33m%02.2s %*s：[[36m%-54.54s [[m%5.5s\n", verb, IDLEN, cuser.userid, msg, Btime(&hdr->pushtime)+3);
            else if (addscore == -1)
                sprintf(add, "[[1;31m%02.2s[[m [[1;33m%*s：[[36m%-54.54s [[m%5.5s\n", verb, IDLEN, cuser.userid, msg, Btime(&hdr->pushtime)+3);
            else
                sprintf(add,                "[[1;33m→ %*s：[[36m%-54.54s [[m%5.5s\n", IDLEN, cuser.userid, msg, Btime(&hdr->pushtime)+3);
        }
        else
            sprintf(add,                  "[[1;33m→ %*s：[[36m%-54.54s [[m%5.5s\n", IDLEN, cuser.userid, msg, Btime(&hdr->pushtime)+3);
*/
        while (fgets(buf, 256, fp))
        {
            memset(&rmsg, 0, sizeof(RMSG));
            if (!strncmp(buf, "\x1b[1;32m※", 9))
                pushstart = 1;

            if (pushstart)
            {
                const char *c2;
                if (!strncmp(buf, "\x1b[1;32m※", 9))
                {
                    f_cat(tmp, buf);
                    continue;
                }
                c2 = strrchr(buf, '\n') - 5;
                str_scpy(rmsg.rtime, c2, sizeof(rmsg.rtime));

                c2 -= 58;
                str_scpy(rmsg.msg, c2, sizeof(rmsg.msg));

                c2 -= 19;
                str_scpy(rmsg.userid, c2, sizeof(rmsg.userid));

                c2 = strchr(buf, 'm');
                str_scpy(rmsg.verb, c2+1, sizeof(rmsg.verb));

                if ((battr & BRD_PUSHDEFINE) && !strncmp(rmsg.verb, "→", 2) )
                    rmsg.pn = COMMENT;
                else if (!strncmp(rmsg.verb, "\x1b[m\x1b[1;33", 2))
                    rmsg.pn = COMMENT;
                /*else if (strncmp(buf, "\x1b[1;33→", 8))
                    rmsg.pn = POSITIVE;*/
                else
                    rmsg.pn = !strncmp(buf, "\x1b[1;33", 6);

                rec_add(recommenddb, &rmsg, sizeof(RMSG));
//              if (!strncmp(buf, "\x1b[1;33m→", 9))
//              {
/*
                    for (i=0; i<12; i++)
                        rmsg.userid[i] = buf[i+10];
                    rmsg.userid[12] = '\0';
                    for (i=0; i<54; i++)
                        rmsg.msg[i] = buf[i+29];
                    rmsg.msg[54] = '\0';
                    for (i=0; i<5; i++)
                        rmsg.rtime[i] = buf[i+87];
                    rmsg.rtime[5] = '\0';
                    rec_add(recommenddb, &rmsg, sizeof(RMSG));
*/
//              }

            }
            else
                f_cat(tmp, buf);
        }
        fclose(fp);
    }

    last = xz[XZ_OTHER - XO_ZONE].xo;  /* record */

    xz[XZ_OTHER - XO_ZONE].xo = xoo = xo_new(recommenddb);
    xoo->cb = cleanrecommend_cb;
    xoo->recsiz = sizeof(RMSG);
    xoo->pos = 0;
    xover(XZ_OTHER);
    free(xoo);

    xz[XZ_OTHER - XO_ZONE].xo = last;  /* restore */

    for (i=0; i<rec_num(recommenddb, sizeof(RMSG)); i++)
    {
        rec_get(recommenddb, &rmsg, sizeof(RMSG), i);
        if (rmsg.pn == POSITIVE)
        {
            counter++;
            sprintf(buf, "\x1b[1;33m%2s %*s：\x1b[36m%-54.54s \x1b[m%5.5s\n", rmsg.verb, IDLEN, rmsg.userid, rmsg.msg, rmsg.rtime);
        }
        else if (rmsg.pn == NEGATIVE)
        {
            counter--;
            sprintf(buf, "\x1b[1;31m%2s \x1b[33m%*s：\x1b[36m%-54.54s \x1b[m%5.5s\n", rmsg.verb, IDLEN, rmsg.userid, rmsg.msg, rmsg.rtime);
        }
        else
        {
            sprintf(buf, "\x1b[m\x1b[1;33m   %*s：\x1b[36m%-54.54s \x1b[m%5.5s\n", IDLEN, rmsg.userid, rmsg.msg, rmsg.rtime);
        }
        f_cat(tmp, buf);
    }

    if (dashf(tmp))
    {
        f_mv(tmp, fpath);
    }

    if ((fd = open(xo->dir, O_RDWR, 0600)) == -1)
        return DL_RELEASE(XO_NONE);

    fstat(fd, &st);
    total = st.st_size / sizeof(HDR);
    pos = BMIN(pos, total);

    f_exlock(fd);
    while (pos >= -1)
    {
        lseek(fd, (off_t) (sizeof(HDR) * pos--), SEEK_SET);
        read(fd, &phdr, sizeof(HDR));
        if (chrono == phdr.chrono)
            break;
    }

    if (++pos >= 0)
    {
        phdr.recommend = counter;
        phdr.xmode &= ~POST_RECOMMEND;
        phdr.xid = 0;
        lseek(fd, (off_t) (sizeof(HDR) * pos), SEEK_SET);
        write(fd, &phdr, sizeof(HDR));
//      rec_put(xo->dir, &phdr, sizeof(HDR), pos);
    }

    f_unlock(fd);
    close(fd);

    return DL_RELEASE(XO_INIT);
}
#endif  /* #ifdef  HAVE_RECOMMEND */
