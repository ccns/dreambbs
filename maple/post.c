/*-------------------------------------------------------*/
/* post.c       ( NTHU CS MapleBBS Ver 2.39 )            */
/*-------------------------------------------------------*/
/* target : bulletin boards' routines                    */
/* create : 95/03/29                                     */
/* update : 2000/01/02                                   */
/*-------------------------------------------------------*/


#define xlog(x)         f_cat("/tmp/b.log", x)

#include "bbs.h"

#ifdef  HAVE_DETECT_CROSSPOST
CHECKSUMCOUNT cksum;
#endif

/* 071002.cat: �����i���� */
#ifdef HAVE_DETECT_ZHANGBA
static int zhangba_currentsession = 0;

#define ZHANGBA_PATTERNS COUNTOF(zhangba_patterns)

static const char *const zhangba_patterns[] = {
    "�i���n",
    "�n�J",
    "taconet.com.tw/jscha",
    "�x�W�j��",
    "�x�j"};

    static int
zhangba_detect(
    const char *fpath)
{
    char checked[ZHANGBA_PATTERNS+1] = {0};
    FILE *fp;
    char buf[256];
    int i, num=0;

    if ((fp = fopen(fpath, "r")))
    {
        while (fgets(buf, sizeof(buf), fp))
            for (i=0;  i < ZHANGBA_PATTERNS; i++)
                if (strstr(buf, zhangba_patterns[i]) && !checked[i])
                {
                    checked[i] = true;
                    num++;
                }
        fclose(fp);
    }
    return num;
}

#endif  /* #ifdef HAVE_DETECT_ZHANGBA */

/* Thor.990113: imports for anonymous log */
static char delete_reason[30] = {0};

GCC_PURE int
cmpchrono(
    const void *hdr)
{
    return ((const HDR *)hdr)->chrono == currchrono;
}


#if 0  // Unused
    static void
change_stamp(
    const char *folder,
    HDR *hdr)
{
    hdr->stamp = time(0);
}
#endif


/* ----------------------------------------------------- */
/* ��} cross post ���v                                  */
/* ----------------------------------------------------- */

#ifdef  HAVE_DETECT_CROSSPOST
    static int
checksum_add(
    const char *title)
{
    int sum=0, i, end;
    const int *ptr;
    ptr = (const int *)title;
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
    const char *fpath,
    int check,
    int state)
{
    char buf[256];
    FILE *fp;
    const char *const star = "��";

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
                    if (*buf != '>' && strncmp(buf, star, 2) && *buf != ':')
                    {
                        sum+=checksum_add(buf);
                        count++;
                    }
                }
            }
            else
                break;
        }
        fclose(fp);
    }
    return checksum_put(sum, check);
}
#endif  /* #ifdef  HAVE_DETECT_CROSSPOST */

/* ----------------------------------------------------- */
/* ��} innbbsd ��X�H��B�s�u��H���B�z�{��             */
/* ----------------------------------------------------- */

    void
btime_update(
    int bno)
{
    if (bno >= 0)
        (bshm->bcache + bno)->btime = -1;   /* �� class_item() ��s�� */
}

    void
outgo_post(
    const HDR *hdr,
    const char *board)
{
    bntp_t bntp;

    memset(&bntp, 0, sizeof(bntp_t));

    if (board)
    {
        bntp.chrono = hdr->chrono;
    }
    else
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
    const HDR *hdr)
{
    if ((hdr->xmode & POST_OUTGO) &&    /* �~��H�� */
            (hdr->chrono > ap_start - 7 * 86400))       /* 7 �Ѥ������� */
    {
        outgo_post(hdr, NULL);
    }
}


/*static inline void*/

    void
move_post(      /* �N hdr �q currboard �h�� board */
    const HDR *hdr,
    const char *board,
    int by_bm)
{
    HDR post;
    char folder[80], fpath[80];

    brd_fpath(folder, currboard, fn_dir);
    hdr_fpath(fpath, folder, hdr);

    brd_fpath(folder, board, fn_dir);
    hdr_stamp(folder, HDR_LINK | 'A', &post, fpath);
    /*unlink(fpath);*/

    /* �����ƻs trailing data */

    memcpy(post.owner, hdr->owner, sizeof(HDR) - offsetof(HDR, owner));
    if (by_bm == -1)
        strcpy(post.owner, cuser.userid);
    if (by_bm == -2)
        post.xmode |= POST_MARKED;

    if (by_bm>0)
        sprintf(post.title, "%-*s %.59s", IDLEN, cuser.userid, hdr->title);

    rec_bot(folder, &post, sizeof(post));
    if (by_bm>=0)
        cancel_post(hdr);
}


/* ----------------------------------------------------- */
/* �o��B�^���B�s��B����峹                            */
/* ----------------------------------------------------- */

#ifdef HAVE_ANONYMOUS
/* Thor.980727: lkchu patch: log anonymous post */
/* Thor.980909: gc patch: log anonymous post filename */
    void
log_anonymous(
    const char *fname)
{
    char buf[512];
    time_t now = time(0);
    /* Thor.990113: �[�W rusername �M fromhost����Ժ� */
    sprintf(buf, "%s %-*s (%s@%s) %s %s %s\n", Etime(&now), IDLEN, cuser.userid, rusername, fromhost, currboard, ve_title, fname);
    f_cat(FN_ANONYMOUS_LOG, buf);
}
#endif

#ifdef  HAVE_DETECT_VIOLATELAW
GCC_PURE int
seek_log(
    const char *title,
    int state)
{
    const BANMAIL *head, *tail;
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
    const char *title)
{
    /* Thor.1105: �i�J�e�ݳ]�n curredit */
    HDR post;
    char fpath[80], folder[80], buf[50];
    const char *nick, *rcpt;
    int mode, bno = -1;
    BRD *brd;
    time_t spendtime;

    bno = currbno;
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
        vmsg("�z�w�g��Ӧh���F�A�ФU���A�ӧa�I");
        return XO_FOOT;
    }
#endif

    if (bbsothermode & OTHERSTAT_EDITING)
    {
        vmsg("�A�٦��ɮ��٨S�s���@�I");
        return XO_FOOT;
    }

    /* cache.090519: �O�D�ۭq�ݪO�o���v�� */
    /* cache.090928: �ݪO��Ū, r2.170912: �վ�ԭz */

    if (brd->battr & BRD_NOREPLY)
    {
        if (!HAS_PERM(PERM_ADMIN))
        {
            vmsg("�藍�_�A���ݪO�ثe�T��o��峹�C");
            return XO_FOOT;
        }
        else
            vmsg("�Ъ`�N�A���ݪO�ثe�O��Ū���A�C");
    }

    if (!(bbstate & STAT_POST))
    {
        vmsg("�藍�_�A�z���v�������C");
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
                sprintf(msg, "���U�ɶ� %d �ѥH�W�A��i�b���ݪO�o��峹", th.age);
                vmsg(msg);
                return XO_FOOT;
            }
            if (cuser.numlogins < th.numlogins)
            {
                sprintf(msg, "�W������ %d ���H�W�A��i�b���ݪO�o��峹", th.numlogins);

                vmsg(msg);
                return XO_FOOT;
            }
            if (cuser.numposts < th.numposts)
            {
                sprintf(msg, "�o��峹 %d �g�H�W�A��i�b���ݪO�o��峹", th.numposts);
                vmsg(msg);
                return XO_FOOT;
            }
            if ((th.point2 != 0) && (cuser.point2 >= th.point2))
            {
                sprintf(msg, "�H�� %d �g(�t)�H�U�A��i�b���ݪO�o��峹", th.point2);
                vmsg(msg);
                return XO_FOOT;
            }
        }
    }

    brd_fpath(fpath, currboard, "post");

    if (more(fpath, (char *)-1)==-1)
        film_out(FILM_POST, 0);

    move(20, 0);
    prints("�o��峹��i %s �j�ݪO", currboard);

    if (!ve_subject(21, title, NULL))
        return XO_HEAD;

    /* ����� Internet �v���̡A�u��b�����o��峹 */
    /* Thor.990111: �S��H�X�h���ݪ�, �]�u��b�����o��峹 */

    if (!HAS_PERM(PERM_INTERNET) || (bbstate & BRD_NOTRAN))
        curredit &= ~EDIT_OUTGO;

#ifdef HAVE_ANONYMOUS
    /* Thor.980727: lkchu�s�W��[²�檺��ܩʰΦW�\��] */
    /* Thor.980909: gc patch: edit �ɰΦW����ñ�W�� */
    if (bbstate & BRD_ANONYMOUS)
    {
        if (cuser.ufo2 & UFO2_DEF_ANONY)
        {
            if (vans("�A(�p)�Q�n�ΦW��(y/N)?[N]") == 'y')
                curredit |= EDIT_ANONYMOUS;
        }
        else if (vans("�A(�p)�Q�n�ΦW��(Y/n)?[Y]") != 'n')
            curredit |= EDIT_ANONYMOUS;
    }
#endif

    utmp_mode(M_POST);
    fpath[0] = 0;
    time(&spendtime);

    if (vedit(fpath, true) < 0)
    {
        unlink(fpath);
        pmsg2(MSG_CANCEL);
        return XO_HEAD;
    }

    spendtime = time(0) - spendtime;    /* itoc.010712: �`�@�᪺�ɶ�(���) */

    //bno = currbno;
    brd = bshm->bcache + currbno;
    brh_get(brd->bstamp, bno);

    /* build filename */

    brd_fpath(folder, currboard, fn_dir);
    do  /* cat.20050729 �¬}���D @@ �¬}�g�`�S����� */
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
    /* Thor.980727: lkchu�s�W��[²�檺��ܩʰΦW�\��] */
    if (curredit & EDIT_ANONYMOUS)
    {
        /* nick = rcpt; */ /* lkchu: nick ���ର  userid */
        nick = "�q�q�ڬO�� ? ^o^";
        /* Thor.990113: �ȻP�{�sid�V�c */
        /* rcpt = "anonymous"; */
        rcpt = "���i�D�A^^b";
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
        deny_log_email(cuser.vmail, (HAS_PERM(PERM_DENYSTOP)) ? -1 : cuser.deny);
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
        deny_log_email(cuser.vmail, (HAS_PERM(PERM_DENYSTOP)) ? -1 : cuser.deny);
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
        /* Thor.990111: �w�� edit.c ���Τ@check */
        brh_add(post.chrono, post.chrono, post.chrono);
        //post_history(xz[XZ_POST - XO_ZONE].xo, &post);
#ifdef  HAVE_DETECT_VIOLATELAW
        if (mode && !banpost)
#else
            if (mode)
#endif
                outgo_post(&post, currboard);

        clear();
        //outs("���Q�K�X�G�i�A");

        if (bbstate & BRD_NOCOUNT || (/*BMIN(wordsnum, */ keysnum/*)*/) < 30)
        {
            pmsg2("�峹���C�J�����A�q�Х]�[�C");
        }
        else
        {
          if (spendtime*3 < keysnum)
          {
            pmsg2("�峹���C�J�����A�q�Х]�[�C");
          }
          else if (spendtime < 30)
          {
            sprintf(buf, "�o�O�z���� %d �g�峹�C", ++cuser.numposts);
            pmsg2(buf);
            brd->blast = time(0);
          }
          else
          {
            mode = BMIN((time_t)keysnum, spendtime) / 10;  /* �C�Q�r/�� �@�� */
            sprintf(buf, "�o�O�z���� %d �g�峹�A��o %d �ڹ��C", ++cuser.numposts, mode);
            pmsg2(buf);
            brd->blast = time(0);
            addmoney(mode, cuser.userid);
          }
        }

        /* �^�����@�̫H�c */

        if (curredit & EDIT_BOTH)
        {
            const char *msg = "�@�̵L�k���H";
#define MSG_OK                "�^���ܧ@�̫H�c"

            char *const rcpt = quote_user;
            if (strchr(rcpt, '@'))
            {
                if (bsmtp(fpath, title, rcpt, 0) >= 0)
                    msg = MSG_OK;
            }
            else
            {
                usr_fpath(folder, rcpt, fn_dir);
                if (hdr_stamp(folder, HDR_LINK, &post, fpath) == 0)
                {
                    strcpy(post.owner, cuser.userid);
                    strcpy(post.title, title);
                    if (!rec_bot(folder, &post, sizeof(post)))
                        msg = MSG_OK;
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
        vmsg("�z�w�g��Ӧh���F�A�ФU���A�ӧa�I");
    }
#endif

    return XO_INIT;
}


    static int
do_reply(
    HDR *hdr)
{
    const char *msg;

    curredit = 0;
    if ((bbstate & BRD_NOREPLY) && !HAS_PERM(PERM_SYSOP))
        msg = "�� �^���� (M)�@�̫H�c (Q)�����H[Q] ";
    else
        msg = "�� �^���� (F)�ݪO (M)�@�̫H�c (B)�G�̬ҬO (Q)�����H[F] ";


    switch (vans(msg))
    {
        case 'm':
            mail_reply(hdr);
            *quote_file = '\0';
            return XO_HEAD;

        case 'q':
            /*
             * Thor: �ѨM Gao �o�{�� bug.. ������ reply�峹�A�o�� Q�����A �M��h
             * Admin/Xfile�U�H�K��@�ӽs��A �A�N�|�o�{�]�X reply�峹�ɪ��ﶵ�F�C
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
     * Thor.1105: ���׬O��i��, �άO�n��X��, ���O�O���i�ݨ쪺,
     * �ҥH�^�H�]��������X
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

        hdr = (HDR *) xo_pool_base + xo->pos;
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
/* �ݪO�\���                                            */
/* ----------------------------------------------------- */

/* ----------------------------------------------------- */


static int post_add(XO *xo);
static int post_body(XO *xo);
static int post_head(XO *xo);           /* Thor: �]�� XoBM �n�� */


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

    fstat(fd, &st);

    total = st.st_size / sizeof(HDR);
    pos = BMIN(xo->pos, total-1);

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
    const HDR *fhdr)
{
    int mode, attr;
    bool zap = (brd_bits[currbno] & BRD_Z_BIT);

    mode = fhdr->xmode;
    attr = (!zap && brh_unread(BMAX(fhdr->chrono, fhdr->stamp))) ? 0 : 0x20;
    //attr = brh_unread(fhdr->chrono) ? 0 : 0x20;

    if (mode & POST_CANCEL)
        return 'c';

    if (mode & POST_DELETE)
        return 'd';

    if (mode & POST_MDELETE)
        return 'D';

    if (mode & POST_EXPIRE)
        return attr | 'E';

    if (mode & POST_LOCK)
        return 'L';

    if (mode & POST_COMPLETE)
        return attr | 'S';

    if (fhdr->pushtime)
        attr = (!zap && brh_unread(fhdr->pushtime)) ? 0 : 0x20;
    mode &= ~((bbstate & STAT_BOARD) ? 0 : POST_GEM);    /* Thor:�@��user�ݤ���G */

    if (mode &= (POST_MARKED | POST_GEM))
        return attr | ((mode == POST_MARKED) ? 'M' : (mode == POST_GEM) ? 'G' : 'B');
    if (!attr)
        return '+';
    return attr;
}

static int
post_foot(
    XO *xo)
{
    outf(MSG_POST);
    return XO_NONE;
}

    static void
post_item(
    int num,
    const HDR *hdr)
{
#ifdef HAVE_RECOMMEND

    if (hdr->xmode & POST_BOTTOM)
    {
        /* �ѩ�m����S���\Ū�O���A�ҥH�˦��wŪ */
        char attr = post_attr(hdr);
        if (attr == '+')
            attr = ' ';
        else if (attr == 'M')
            attr |= 0x20;
        prints("  \x1b[%s33m  ��\x1b[m%c%s%c%s",
                HAVE_UFO2_CONF(UFO2_MENU_LIGHTBAR) ? "" : "1;", tag_char(hdr->chrono),
                hdr->xmode & POST_MARKED ? (HAVE_UFO2_CONF(UFO2_MENU_LIGHTBAR) ? "\x1b[36m" : "\x1b[1;36m") : "",
                attr, hdr->xmode & POST_MARKED ? "\x1b[m" : "");
    }
    else
        prints("%6d%c%s%c%s", num, tag_char(hdr->chrono),
                hdr->xmode & POST_MARKED ? (HAVE_UFO2_CONF(UFO2_MENU_LIGHTBAR) ? "\x1b[36m" : "\x1b[1;36m") : "",
                post_attr(hdr),
                hdr->xmode & POST_MARKED ? "\x1b[m" : "");

    /* �Ҷq���ª����ۮe�ʥ����ѱ�, �[���P�_�i�H�֤@�I�I */
    if (/*hdr->xmode & POST_RECOMMEND &&*/ !(hdr->xmode & POST_BOTTOM) && !HAVE_UFO2_CONF(UFO2_PRH))
    {
        num = hdr->recommend;

        if (num>0)
        {
            if (num > 120)                    /* ���z */
                prints("\x1b[1;33m�z\x1b[m");
            else if (num > 99)                /* ���z */
                prints("\x1b[1;31m�z\x1b[m");
            else if (num > 4)
                prints("\x1b[%s31m%02d\x1b[m", HAVE_UFO2_CONF(UFO2_MENU_LIGHTBAR) ? "" : "1;", num);
            else
                prints("\x1b[%s31m%02d\x1b[m", HAVE_UFO2_CONF(UFO2_MENU_LIGHTBAR) ? "" : "1;", num);
        }
        else if (num<0)
        {
            if (num < -120)              /* ���� */
                prints("\x1b[1;30m�z\x1b[m");
            else if (num < -99)               /* ���� */
                prints("\x1b[1;32m��\x1b[m");
            else if (num < -5)
                prints("\x1b[32m%02d\x1b[m", -num);
            else
                prints("\x1b[32m%02d\x1b[m", -num);
        }
        else
            prints("  ");
    }
    else
    {
        outs("  ");
    }

    hdr_outs(hdr, d_cols + 47);   /* �֤@��ө���� */
#else
    prints("%6d%c%c ", (hdr->xmode & POST_BOTTOM) ? -1 : num, tag_char(hdr->chrono), post_attr(hdr));
    hdr_outs(hdr, d_cols + 47);
#endif  /* #ifdef HAVE_RECOMMEND */
}

static int
post_cur(
    XO *xo)
{
    const HDR *const fhdr = (const HDR *) xo_pool_base + xo->pos;
    move(3 + xo->pos - xo->top, 0);
    post_item(xo->pos + 1, fhdr);
    return XO_NONE;
}

    static int
post_body(
    XO *xo)
{
    const HDR *fhdr;
    int num, max, tail;

    max = xo->max;
    if (max <= 0)
    {
        if (bbstate & STAT_POST)
        {
            if (vans("�n�s�W��ƶܡH(y/N) [N] ") == 'y')
                return post_add(xo);
        }
        else
        {
            vmsg("���ݪO�|�L�峹");
        }
        return XO_QUIT;
    }

    num = xo->top;
    fhdr = (const HDR *) xo_pool_base + num;
    tail = num + XO_TALL;
    max = BMIN(max, tail);

    move(3, 0);
    do
    {
        post_item(++num, fhdr++);
    } while (num < max);

    clrtobot();
    return post_foot(xo);
}


static int
post_neck(
    XO *xo)
{
    char buf[17];
    sprintf(buf, "�H��: %d", BMAX(bshm->mantime[currbno], 1));
    prints(NECKPOST, d_cols, "", buf);
    return post_body(xo);
}


    static int                  /* Thor: �]�� XoBM �n�� */
post_head(
    XO *xo)
{
    vs_head(currBM, (const char *) xo->xyz);
    return post_neck(xo);
}


    static int
post_visit(
    XO *xo)
{
    int ans, row, max;
    const HDR *fhdr;

    ans = vans("�]�w�Ҧ��峹 (U)��Ū (V)�wŪ (Q)�����H [Q] ");
    if (ans == 'v' || ans == 'u')
    {
        brh_visit(ans = ans == 'u');
        return XO_BODY;  /* Redraw the list to show changes */
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
        sprintf(title, STR_REPLY " %s", str_ttl(currtitle));
        title[TTLEN] = '\0';
    }
    else
    {
        *title = '\0';
    }

    return vget(row, 0, "���D�G", title, TTLEN + 1, GCARRY);
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
    int success_count = 0;

    if (!cuser.userlevel)
        return XO_NONE;

    /* lkchu.990428: mat patch ��ݪO�|����w�ɡA�ץ�cross post�|�_�u�����D */
    if (bbsmode == M_READA)
    {
        battr = (bshm->bcache + currbno)->battr;
        if (!HAS_PERM(PERM_SYSOP) && (battr & BRD_NOFORWARD))
        {
            outz("�� ���O�峹���i���");
            return XO_FOOT;
        }
    }

    // check delete or not .. by statue 2000/05/18
    hdr = (HDR *) xo_pool_base + xo->pos;
    if (hdr->xmode & (POST_DELETE | POST_CANCEL | POST_MDELETE | POST_CURMODIFY))
        return XO_NONE;
    if ((hdr->xmode & POST_LOCK) && !HAS_PERM(PERM_SYSOP))
    {
        vmsg("Access Deny!");
        return XO_FOOT;
    }

    /* verit 021113 : �ѨM�b po �峹�M��� ctrl+u �M�ᴫ��ݪO�h��������Ƽ��D���D */
    if (bbsothermode & OTHERSTAT_EDITING)
    {
        vmsg("�A�٦��ɮ��٨S�s���@�I");
        return XO_FOOT;
    }


    /* lkchu.981201: ������ */
    tag = AskTag("���");

    if (tag < 0)
        return XO_FOOT;

    if (ask_board(xboard, BRD_W_BIT,
                "\n\n\x1b[1;33m�ЬD��A���ݪO�A�ۦP�峹���ŶW�L����ƥءC\x1b[m\n\n")
            && (*xboard || xo->dir[0] == 'u'))  /* �H�c���i�H��K��currboard */
    {
        if (*xboard == 0)
            strcpy(xboard, currboard);

        hdr = tag ? &xhdr : (HDR *) xo_pool_base + xo->pos;
        /* lkchu.981201: �����K */
        if (!tag && (hdr->xmode & POST_LOCK) && !HAS_PERM(PERM_SYSOP))
        {
            vmsg("���峹�T������I");
            return XO_HEAD;
        }

        method = 1;
        if ((HAS_PERM(PERM_ALLBOARD) || !strcmp(hdr->owner, cuser.userid)) &&
                (vget(2, 0, "(1)������ (2)����峹�H[1] ", buf, 3, DOECHO) != '2'))
        {
            method = 0;
        }

        if (!tag)   /* lkchu.981201: �������N���n�@�@�߰� */
        {
            if (method)
                sprintf(xtitle, STR_FORWARD " %.66s", hdr->title);
            else
                strcpy(xtitle, hdr->title);

            if (!vget(2, 0, "���D�G", xtitle, TTLEN + 1, GCARRY))
                return XO_HEAD;
        }

        rc = vget(2, 0, "(S)�s�� (L)���� (Q)�����H[Q] ", buf, 3, LCECHO);
        if (rc != 'l' && rc != 's')
            return XO_HEAD;

        locus = 0;
        dir = xo->dir;

        battr = (bshm->bcache + brd_bno(xboard))->battr;

        do      /* lkchu.981201: �����K */
        {
            if (tag)
            {
                EnumTagHdr(hdr, dir, locus++);

                if (method)
                    sprintf(xtitle, STR_FORWARD " %.66s", hdr->title);
                else
                    strcpy(xtitle, hdr->title);
            }

            /* verit 2002.04.04 : ��������, �ˬd tag ���g�O�_�Q�R���Ψ����L */
            if (hdr->xmode & (POST_DELETE | POST_CANCEL | POST_MDELETE | POST_CURMODIFY))
                continue;

            /* if (rc == 'l' || rc == 's') */
            /* lkchu.981201: ������o��� rc �� 's' or 'l' */
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
                        sprintf(buf, "%s] �H�c", cuser.userid);
                    else
                        strcat(buf, "] �ݪO");
                    fprintf(xfp, "�� ��������� [%s\n\n", buf);

                    f_suck(xfp, fpath);
                    /* ve_sign(xfp); */
                    fclose(xfp);

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
                    /* lkchu.981201: �������O�d���� */
                }

                /* Thor.981205: �ɥ� method �s��ݪ��ݩ� */
                /* method = (bshm->bcache + brd_bno(xboard))->battr; */

                /* Thor.990111: �b�i�H��X�e, �ncheck user���S����X���v�O? */
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
                    deny_log_email(cuser.vmail, (HAS_PERM(PERM_DENYSTOP)) ? -1 : cuser.deny);
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
                    str_sncpy(chdr.title, cbuf, sizeof(chdr.title), TTLEN + 1);
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

                success_count++;
            }
        } while (locus < tag);

        if (!HAS_PERM(PERM_ADMIN))
        {
            time_t now;
            GCC_UNUSED struct tm *ptime;
            char add[180], tgt[30];

            time(&now);
            ptime = localtime(&now);
            sprintf(tgt, "����� %s �ݪO", xboard);
            xfp = fopen(fpath, "a");
            sprintf(add, "\x1b[1;33m�� %*s�G\x1b[36m%-54.54s \x1b[m%5.5s\n", IDLEN, cuser.userid, tgt, Btime(&now)+3);
            fprintf(xfp, "%s", add);
            fclose(xfp);
        }

        if (success_count == 0)
        {
            vmsg("������ѡC");
        }
        /* Thor.981205: check �Q�઺�����S���C�J����? */
        else if (/* method */ battr & BRD_NOCOUNT)
        {
            if (success_count == ((tag == 0) ? 1 : tag))
                prints("��� %d �g���\\�A�峹���C�J�����A�q�Х]�[�C", success_count);
            else
                prints("��� %d �g���\\�A%d �g���ѡA�峹���C�J�����A�q�Х]�[�C",
                    success_count, ((tag == 0) ? 1 : tag) - success_count);
        }
        else
        {
            /* cuser.numposts++; */
            /* cuser.numposts += (tag == 0) ? 1 : tag; */ /* lkchu.981201: �n�� tag */
            cuser.numposts += success_count; /* IID.20190730: Use the count of successful reposting */
            if (success_count == ((tag == 0) ? 1 : tag))
                sprintf(buf, "��� %d �g���\\�A�A���峹�W�� %d �g",
                    success_count, cuser.numposts);
            else
                sprintf(buf, "��� %d �g���\\�A%d �g���ѡA�A���峹�W�� %d �g",
                    success_count, ((tag == 0) ? 1 : tag) - success_count, cuser.numposts);
            vmsg(buf);
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
    hdr = (HDR *) xo_pool_base + xo->pos;
    if (hdr->xmode & (POST_DELETE | POST_CANCEL | POST_MDELETE | POST_CURMODIFY))
        return XO_NONE;

    tag = AskTag("�s����K");

    if (tag < 0)
        return XO_FOOT;

    ll_new();
    listing = brd_list(0);
    do_expire = 0;

    if (listing)
    {

        hdr = tag ? &xhdr : hdr;

        vget(2, 0, "�]�w�R���� ?? (Y)�n (N)���n�H[Y] ", buf, 3, LCECHO);
        if (*buf != 'n')
            do_expire = time(0) + 86400 * 7;

        vget(2, 0, "(Y)�T�w (Q)�����H[Q] ", buf, 3, LCECHO);
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

#endif  /* #ifdef HAVE_MULTI_CROSSPOST */


/* ----------------------------------------------------- */
/* ��Ƥ��s���Gedit / title                              */
/* ----------------------------------------------------- */

void
post_history(
    XO *xo,
    const HDR *fhdr)
{
    int prev, chrono, next, pos, max, push=0;
    char *dir;
    HDR buf;

#ifdef  HAVE_BOTTOM
    if (fhdr->xmode & POST_BOTTOM && fhdr->xmode & POST_COMPLETE)
        return;
#endif


    dir = xo->dir;
    pos = xo->pos;
    max = xo->max;

    chrono = fhdr->chrono;
    push = fhdr->pushtime;


    if (brh_unread(push))
        brh_add(push, push, push);

    if (!brh_unread(chrono))
        //if (!brh_unread(push))
            return;

    if (pos - 1 >= 0)
        prev = fhdr[-1].chrono;
    else
        prev = chrono;

    if (pos + 1 < xo->max)
        next = fhdr[1].chrono;
    else
        next = chrono;
/*
    if (push)
        prev = chrono = next = push;
*/
    brh_add(prev, chrono, next);

}

#if 0
    void
post_history(          /* �N hdr �o�g�[�J brh */
    XO *xo,
    const HDR *hdr)
{
    int fd;
    time_t prev, chrono, next, this_;
    HDR buf;


    if (hdr->xmode & POST_BOTTOM)     /* �m���夣�[�J�\Ū�O�� */
        return;

    chrono = BMAX(hdr->chrono, hdr->stamp);

    if (!brh_unread(chrono))      /* �p�G�w�b brh ���A�N�L�ݰʧ@ */
        return;

    if ((fd = open(xo->dir, O_RDONLY)) >= 0)
    {
        prev = chrono + 1;
        next = chrono - 1;

        while (read(fd, &buf, sizeof(HDR)) == sizeof(HDR))
        {
            this_ = BMAX(buf.chrono, buf.stamp);

            if (chrono - this_ < chrono - prev)
                prev = this_;
            else if (this_ - chrono < next - chrono)
                next = this_;
        }
        close(fd);

        prev = BMIN(prev, chrono);  /* �S���U�@�g */
        next = BMAX(next, chrono);  /* �S���W�@�g */

        brh_add(prev, chrono, next);
    }
}
#endif  /* #if 0 */

    static int
post_browse(
    XO *xo)
{
    HDR *hdr;
    int cmd GCC_UNUSED, xmode, pos;
    char *dir, fpath[64], *board GCC_UNUSED;

    int key;

    dir = xo->dir;
    cmd = XO_NONE;
    board=currboard;

    for (;;)
    {

        pos = xo->pos;
        hdr = (HDR *) xo_pool_base + pos;
        xmode = hdr->xmode;
        if (xmode & (POST_CANCEL | POST_DELETE | POST_MDELETE))
            break;

#ifdef  HAVE_USER_MODIFY
        if (xmode & POST_CURMODIFY)
        {
            if (pid_find(hdr->xid))
            {
                vmsg("���峹���b�ק襤!!");
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

        snprintf(desc, sizeof(desc), "%s %s %s %lld %s\n", Atime(&now), cuser.userid, currboard, (long long)hdr->chrono, ipv6addr);
        f_cat(FN_BROWSE_LOG, desc);

        hdr_fpath(fpath, dir, hdr);

        /* Thor.990204: ���Ҽ{more �Ǧ^�� */
        //    if ((key = more(fpath, MSG_POST)) == -1)
        //      break;

        cmd = XO_LOAD;
        post_history(xo, hdr);

        if (!(hdr->xmode & POST_LOCK) || HAS_PERM(PERM_SYSOP))
            strcpy(currtitle, str_ttl(hdr->title));
        else
            currtitle[0] = '\0';

        /* Thor.990204: ���Ҽ{more �Ǧ^�� */
        if ((key = more(fpath, FOOTER_POST)) < 0)
            break;

        if (key == -2)
            return XO_INIT;
        switch (xo_getch(xo, key))
        {
            case XO_BODY:
                continue;
            case Ctrl('U'):
                every_U();
                xo_load(xo, sizeof(HDR));
                continue;
            case Ctrl('B'):
                every_B();
                continue;
            case 'y':
            case 'r':
                if (bbstate & STAT_POST)
                {
                    strcpy(quote_file, fpath);
                    if (do_reply(hdr) == XO_INIT)       /* �����\�a post �X�h�F */
                        return XO_INIT;
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

    return XO_INIT;
}


/* ----------------------------------------------------- */
/* ��ذ�                                                */
/* ----------------------------------------------------- */


    int
post_gem(
    XO *xo)
{
    char fpath[32];

    strcpy(fpath, "gem/");
    strcpy(fpath + 4, xo->dir);

    /* Thor.990118: �ݪ��`�ޤ��� GEM_SYSOP */
    XoGem(fpath, "", (HAS_PERM(PERM_SYSOP|PERM_BOARD|PERM_GEM)) ? GEM_SYSOP :
            (bbstate & STAT_BOARD ? GEM_MANAGER : GEM_USER));

    return XO_INIT;
}


/* ----------------------------------------------------- */
/* �ݪO�Ƨѿ�                                            */
/* ----------------------------------------------------- */


    static int
post_memo(
    XO *xo)
{
    char fpath[64];

    brd_fpath(fpath, currboard, FN_NOTE);
    /* Thor.990204: ���Ҽ{more �Ǧ^�� */
    if (more(fpath, NULL) == -1)
    {
        vmsg("���ݪO�|�L�u�Ƨѿ��v");
        return XO_FOOT;
    }

    return XO_HEAD;
}

    static int
post_post(
    XO *xo)
{
    int mode;
    char fpath[64];

    if (!(bbstate & STAT_BOARD))
        return XO_NONE;

    mode = vans("�o�夽�i (D)�R�� (E)�ק� (Q)�����H[E] ");
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
                vmsg("�A�٦��ɮ��٨S�s���@�I");
            }
            else
            {
                if (vedit(fpath, false))
                    vmsg(msg_cancel);
                return XO_HEAD;
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

    mode = vans("�Ƨѿ� (D)�R�� (E)�ק� (Q)�����H[E] ");
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
                vmsg("�A�٦��ɮ��٨S�s���@�I");
            }
            else
            {
                if (vedit(fpath, false))
                    vmsg(msg_cancel);
                return XO_HEAD;
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
    return XO_HEAD;
}


/* ----------------------------------------------------- */
/* �\��Gtag / copy / forward / download                 */
/* ----------------------------------------------------- */


    int
post_tag(
    XO *xo)
{
    const HDR *hdr;
    int tag, pos;

    pos = xo->pos;
    hdr = (const HDR *) xo_pool_base + pos;

#ifdef XZ_XPOST
    if (xo->key == XZ_XPOST)
        pos = hdr->xid;
#endif

    if ((tag = Tagger(hdr->chrono, pos, TAG_TOGGLE)))
    {
        return XO_CUR + 1;
    }

    /* return XO_NONE; */
    return XO_MOVE + XO_REL + 1; /* lkchu.981201: ���ܤU�@�� */
}


/* ----------------------------------------------------- */
/* �O�D�\��Gmark / delete                               */
/* ----------------------------------------------------- */


    static int
post_mark(
    XO *xo)
{
    if (bbstate & STAT_BOARD)
    {
        HDR *hdr;
        int pos;

        pos = xo->pos;
        hdr = (HDR *) xo_pool_base + pos;

        if (hdr->xmode & (POST_DELETE | POST_CANCEL | POST_MDELETE))
            return XO_NONE;

        hdr->xmode ^= POST_MARKED;
        rec_put(xo->dir, hdr, sizeof(HDR), xo->key == XZ_POST ? pos : hdr->xid);
        return XO_CUR;

    }
    return XO_NONE;
}

    static int
lazy_delete(
    void *hdr_obj)
{
    HDR *hdr = (HDR *)hdr_obj;
    if (!strcmp(hdr->owner, cuser.userid))
    {
        sprintf(hdr->title, "<< ���峹�� %s �R�� >>", cuser.userid);
        hdr->xmode |= POST_DELETE;
    }
    else if (strlen(delete_reason) < 1)
    {
        sprintf(hdr->title, "<< ���峹�� %s �R�� >>", cuser.userid);
        hdr->xmode |= POST_MDELETE;
    }
    else
    {
        sprintf(hdr->title, "[�R��]%s�G%s", cuser.userid, delete_reason);
        hdr->xmode |= POST_MDELETE;
    }

    return 0;
}

    static int
post_delete(
    XO *xo)
{
    int pos;
    bool by_BM;
    HDR *fhdr, phdr;
    char buf[80], fpath[80];

#define BN_DELETED      BRD_DELETED
#define BN_JUNK         BRD_JUNK

    if (!cuser.userlevel ||
            !strcmp(currboard, BN_DELETED) ||
            !strcmp(currboard, BN_JUNK))
        return XO_NONE;

    if (HAS_PERM(PERM_DENYPOST))
    {
        vmsg("�A���Q���v���A�L�k�R������峹�I");
        return XO_FOOT;
    }

    pos = xo->pos;
    fhdr = (HDR *) xo_pool_base + pos;

    if (fhdr->xmode & (POST_MARKED | POST_CANCEL | POST_DELETE | POST_MDELETE))
        return XO_NONE;

    if ((fhdr->xmode & POST_LOCK) && !(HAS_PERM(PERM_SYSOP| PERM_BOARD)||bbstate & STAT_BOARD))
        return XO_NONE;

    /* 090911.cache: �ˬd�峹���p */
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
            vmsg("��L�ϥΪ̥��b�s����ˤ峹�d���A�еy�ԡC");
            return XO_FOOT;
        }
    }

    if (phdr.xmode & POST_CURMODIFY)
    {
        vmsg("�峹���b�Q�ק�A�еy�ԡC");
        return XO_FOOT;
    }

    by_BM = strcmp(fhdr->owner, cuser.userid);
    if (!(bbstate & STAT_BOARD) && by_BM)
        return XO_NONE;

    hdr_fpath(fpath, xo->dir, fhdr);
    if (vans(msg_del_ny) == 'y')
    {
        currchrono = fhdr->chrono;

        if (by_BM && (fhdr->xmode & POST_BOTTOM))
        {
            lazy_delete(fhdr); /* Thor.980911: ����: �ק� xo_pool */
            return XR_FOOT + XO_CUR;
        }

        if (by_BM/* && (bbstate & BRD_NOTRAN) && !(fhdr->xmode & POST_BOTTOM)*/)
            vget(B_LINES_REF, 0, "�п�J�R���z�ѡG", delete_reason, 29, DOECHO);
        //    return 0;
        if (by_BM/* && bbstate & BRD_NOTRAN*/&& (bbstate & STAT_BOARD) && !strstr(fhdr->owner, ".") && !strstr(fhdr->lastrecommend, "$") && !(fhdr->xmode & POST_BOTTOM))
        {
            char folder[128], buf[80], deleted_notify[64];
            ACCT tmp;

            usr_fpath(folder, fhdr->owner, fn_dir);
            if (acct_load(&tmp, fhdr->owner) >= 0)
            {
                if (vans("�O�_�h�^�峹�H[y/N]") == 'y')
                {
                    if (vans("�O�_�����H��H[y/N]") == 'y')
                    {
                        addpoint2(1, fhdr->owner);
                        pmsg2("�H�h�����I");
                    }

                    FILE *fp;
                    time_t now = time(0);
                    HDR mhdr;
                    hdr_stamp(folder, 0, &mhdr, buf);
                    strcpy(mhdr.owner, cuser.userid);
                    strcpy(mhdr.title, "�峹�h�^�q��");
                    rec_add(folder, &mhdr, sizeof(mhdr));

                    fp = fopen(buf, "w");
                    fprintf(fp, "�@��: %s (%s)\n", cuser.userid, cuser.username);
                    fprintf(fp, "���D: %s\n�ɶ�: %s\n", "�峹�h�^�q��", ctime(&now));
                    fprintf(fp, "\n\x1b[1;31m***** ���H��Ѩt�Φ۰ʲ��͡A�p�n�ӶD�Э��s�W������H�����ȨëO�d���H�� *****\x1b[m\n\n");
                    fprintf(fp, "\x1b[1;33m�z�b %s �O���峹�m%s�n�Q�h�^\x1b[m\n", currboard, fhdr->title);
                    fprintf(fp, "\x1b[1;33m�z�ѡG%s\x1b[m\n\n", delete_reason);
                    fprintf(fp, "�峹���e�p�U�G\n\n");

                    if (dashf(fpath))
                    {
                        sprintf(deleted_notify, "run/deleted.%s", cuser.userid);
                        f_cp(fpath, deleted_notify, 0600);
                        f_suck(fp, deleted_notify);
                        unlink(deleted_notify);
                    }
                    fclose(fp);
                }
            }

        }

        /* Thor.980911: for ���D��峹 in �걵 */
        /* if (!rec_del(xo->dir, sizeof(HDR), xo->pos, cmpchrono, lazy_delete)) */
        if (!rec_del(xo->dir, sizeof(HDR), xo->key == XZ_POST ? pos : fhdr->xid, cmpchrono, lazy_delete))
        {
            move_post(fhdr, by_BM ? BN_DELETED : BN_JUNK, by_BM);
            if (!by_BM && !(bbstate & BRD_NOCOUNT))
            {
                if (cuser.numposts > 0)
                    cuser.numposts--;
                sprintf(buf, "%s�A�z���峹� %d �g", MSG_DEL_OK, cuser.numposts);
                vmsg(buf);
#ifdef  HAVE_DETECT_CROSSPOST
                checksum_find(fpath, 1, bbstate);
#endif
            }
            lazy_delete(fhdr); /* Thor.980911: ����: �ק� xo_pool */
            return XR_FOOT + XO_CUR;
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
    int pos;
    bool by_BM;
    const HDR *hdr;

    pos = xo->pos;
    hdr = (const HDR *) xo_pool_base + pos;

    by_BM = strcmp(hdr->owner, cuser.userid);

    if ((hdr->xmode & POST_MARKED) || (hdr->xmode & POST_LOCK) || !(bbstate & STAT_BOARD))
    {
        return XO_NONE;
    }

    if (vans("�O�_�����尣�峹�H[y/N]") == 'y')
    {
        const HDR hdr_orig = *hdr;
        currchrono = hdr->chrono;

        if (!rec_del(xo->dir, sizeof(HDR), xo->key == XZ_POST ? pos : hdr->xid, cmpchrono, 0))
        {
            move_post(&hdr_orig, by_BM ? BRD_DELETED : BRD_JUNK, by_BM);
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
        const HDR *hdr;
        HDR post;
        char fpath[64];

        hdr = (const HDR *) xo_pool_base + xo->pos;

        //if ((hdr->xmode & POST_BOTTOM) && !HAVE_PERM(PERM_SYSOP)) /* �w�m���N����A�m�� */
        //    return post_delete(xo);

        //TODO: �h�ˤƸm���\��

        hdr_fpath(fpath, xo->dir, hdr);
        hdr_stamp(xo->dir, HDR_LINK | 'A', &post, fpath);

        if (hdr->xmode & POST_BOTTOM) /* �w�g�Q�m��������A�m�� */
            return XO_NONE;
        else
            post.xmode = POST_MARKED | POST_BOTTOM;  /* �۰ʥ[ mark */

        strcpy(post.owner, hdr->owner);
        strcpy(post.nick, hdr->nick);
        strcpy(post.title, hdr->title);

        rec_add(xo->dir, &post, sizeof(HDR));
        btime_update(currbno);

        return XO_LOAD;     /* �ߨ���ܸm���峹 */
    }
    return XO_NONE;
}
#endif  /* #ifdef HAVE_POST_BOTTOM */

    static int
post_complete(
    XO *xo)
{
    if (HAS_PERM(PERM_SYSOP|PERM_BOARD))
    {
        HDR *hdr;
        int pos;

        pos = xo->pos;
        hdr = (HDR *) xo_pool_base + pos;

        hdr->xmode ^= POST_COMPLETE;
        rec_put(xo->dir, hdr, sizeof(HDR), xo->key == XZ_POST ? pos : hdr->xid);
        return XO_CUR;
    }
    return XO_NONE;
}

    static int
post_lock(
    XO *xo)
{
    HDR *hdr;
    int pos;

    if (!cuser.userlevel) /* itoc.020114: guest ������L guest ���峹�[�K */
        return XO_NONE;

    pos = xo->pos;
    hdr = (HDR *) xo_pool_base + pos;

    if (!strcmp(hdr->owner, cuser.userid) || HAS_PERM(PERM_SYSOP | PERM_BOARD) || (bbstate & STAT_BOARD))
    {
        int redraw_flags = 0;

        if (hdr->xmode & (POST_DELETE | POST_CANCEL | POST_MDELETE))
            return XO_NONE;

        /* cache.100529: prevent user unlock then delete */
        if (!(bbstate & STAT_BOARD) && !HAS_PERM(PERM_ADMIN))
        {
            if (hdr->xmode & POST_LOCK)
                return XO_NONE;

            /* IID.2020-10-07: Prevent the user from accidentally locking their post */
            if (vans("�`�N�G�D�O�D���A����ۦ����C�T�w�n���ܡH[y/N] ") != 'y')
                return XO_FOOT;
            redraw_flags |= XR_FOOT;
        }

        hdr->xmode ^= POST_LOCK;
        rec_put(xo->dir, hdr, sizeof(HDR), xo->key == XZ_POST ? pos : hdr->xid);

        if (redraw_flags)
            vmsg("�w���C�p������A�Ь��O�D�C");
        return redraw_flags | XO_CUR;
    }
    return XO_NONE;
}

/*cache.080520: �s���[�ݤ峹�ݩ�*/
    static int
post_state(
    XO *xo)
{
    const HDR *ghdr;
    char fpath[64], *dir, buf[32];
    struct stat st;

    ghdr = (const HDR *) xo_pool_base + xo->pos;

    dir = xo->dir;
    hdr_fpath(fpath, dir, ghdr);

    strcpy(buf, currboard);

    grayout(0, b_lines, GRAYOUT_DARK);

    if (HAS_PERM(PERM_ADMIN))
    {
        move(b_lines - 10, 0);
        clrtobot();

        prints("\x1b[1;34m");
        outsep(b_cols, MSG_BLINE);
        prints("\n\x1b[1;33;44m \x1b[37m�峹�N�X�θ�T�d�ߡG %*s \x1b[m", d_cols + 56, "");
        outs("\n\n \x1b[1;37m��\x1b[m �峹����: ");
        outs(dir);
        outs("\n \x1b[1;37m��\x1b[m �峹�N�X: #");
        outs("\x1b[1;32m");
        outs(ghdr->xname);
        outs("\x1b[m");
        outs("\n \x1b[1;37m��\x1b[m �nŪ�s��: " URL_PREFIX "/");
        outs(buf);
        outs("/");
        outs(ghdr->xname);
        outs("\x1b[m");
        outs("\n \x1b[1;37m��\x1b[m �峹��m: ");
        outs(fpath);
/*
        int k, l, m;
        k = l = m = 0;
        if (ghdr->chrono > ghdr->stamp)
            k=1;
        else if (ghdr->chrono < ghdr->stamp)
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
            prints("\n \x1b[1;37m��\x1b[m �̫�s��: %s\n \x1b[1;37m��\x1b[m �ɮפj�p: \x1b[1;32m%lld\x1b[m bytes", Ctime(&st.st_mtime), (long long)st.st_size);

    }
    else if (!(cuser.userlevel))
    {
        vmsg("�z���v������");
        return XO_HEAD;
    }
    else
    {
        move(b_lines - 8, 0);
        clrtobot();

        prints("\x1b[1;34m");
        outsep(b_cols, MSG_BLINE);
        prints("\n\x1b[1;33;44m \x1b[37m�峹�N�X�θ�T�d�ߡG %*s \x1b[m", d_cols + 56, "");
        if (ghdr->xmode & (POST_EXPIRE | POST_MDELETE | POST_DELETE | POST_CANCEL | POST_LOCK | POST_CURMODIFY))
        {
            outs("\n\n \x1b[1;37m��\x1b[m �峹�Q��w�B�s��Ϊ̧R����");
            outs("\x1b[m");
            outs("\n");
        }
        else
        {
            outs("\n\n \x1b[1;37m��\x1b[m �峹�N�X: #");
            outs("\x1b[1;32m");
            outs(ghdr->xname);
            outs("\x1b[m");
            outs("\n \x1b[1;37m��\x1b[m �nŪ�s��: " URL_PREFIX "/");
            outs(buf);
            outs("/");
            outs(ghdr->xname);
            outs("\x1b[m");
        }
        if (!stat(fpath, &st))
            prints("\n \x1b[1;37m��\x1b[m �̫�s��: %s\n \x1b[1;37m��\x1b[m �ɮפj�p: \x1b[1;32m%lld\x1b[m bytes", Ctime(&st.st_mtime), (long long)st.st_size);

    }

    vmsg(NULL);

    return XO_HEAD;
}

#if 0
    static int
post_state(
    XO *xo)
{
    const HDR *hdr;
    char *dir, fpath[80];
    struct stat st;

    if (!(HAS_PERM(PERM_SYSOP)))
        return XO_NONE;

    dir = xo->dir;
    hdr = (const HDR *) xo_pool_base + xo->pos;
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
    prints("Chrono : %lld Pushtime : %d\n", (long long)hdr->chrono, hdr->pushtime);

    vmsg(NULL);

    return XO_BODY;
}
#endif  /* #if 0 */

    static int
post_undelete(
    XO *xo)
{
    int pos, i, len;
    HDR *fhdr;
    char buf[256], fpath[128], *ptr;
    FILE *fp;

    if (!cuser.userlevel)
        return XO_NONE;

    pos = xo->pos;
    fhdr = (HDR *) xo_pool_base + pos;
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
        str_sncpy(fhdr->title, ptr, sizeof(fhdr->title), 60);
        if (!HAS_PERM(PERM_SYSOP))
        {
            const int len_strip_id = strlen(fhdr->title) - strlen(cuser.userid) - 2;
            sprintf(buf, "{%s}", cuser.userid);

            /* IID.2020-11-16: Prevent repeated undeletion information */
            if (len_strip_id < 0 || strcmp(fhdr->title + len_strip_id, buf))
                strcat(fhdr->title, buf);
        }
        fhdr->title[71] = 0;  /* verit 2002.01.23 �קK�Ϥ峹�y�� title �z�� */

        /* verit 2003.10.16 �קK�Ϥ峹��, �X�{�m����D */
        len = strlen(fhdr->title);
        for ( i=0; i<len; ++i )
            if ( fhdr->title[i] == '\x1b' )
                fhdr->title[i] = '*';

        fclose(fp);
#if 0
        if (!strcmp(fhdr->owner, cuser.userid) && (fhdr->xmode & POST_DELETE)
                && !(bbstate & BRD_NOCOUNT))
        {
            /*
            cuser.numposts++;
            sprintf(buf, "�_��R���A�z���峹�W�� %d �g", cuser.numposts);
            vmsg(buf);*/  /* 20000724 visor: ���峹�g�ƪ� bug */
#ifdef  HAVE_DETECT_CROSSPOST
            checksum_find(fpath, 0, bbstate);
#endif
        }
#endif  /* #if 0 */
    }
    fhdr->xmode &= (~(POST_MDELETE | POST_DELETE | POST_CANCEL));
    if (!rec_put(xo->dir, fhdr, sizeof(HDR), pos))
    {
        return XO_CUR + 1;
    }
    /*return XO_LOAD;*/
    return XO_MOVE + XO_REL + 1;
}

    static int
post_expire(
    XO *xo)
{
    int pos;
    HDR *fhdr;
    char fpath[80];

    if (!cuser.userlevel ||
            !strcmp(currboard, BRD_DELETED) ||
            !strcmp(currboard, BRD_JUNK))
        return XO_NONE;

    if (!HAS_PERM(PERM_ALLBOARD))
        return XO_NONE;

    pos = xo->pos;
    fhdr = (HDR *) xo_pool_base + pos;

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
        return XO_CUR;
    }
    return XO_NONE;

}

    static int
post_unexpire(
    XO *xo)
{
    int pos;
    HDR *fhdr;
    char fpath[128];

    if (!HAS_PERM(PERM_ALLBOARD))
        return XO_NONE;

    pos = xo->pos;
    fhdr = (HDR *) xo_pool_base + pos;

    if (!(fhdr->xmode & (POST_EXPIRE)))
        return XO_NONE;

    hdr_fpath(fpath, xo->dir, fhdr);

    if ( !((bbstate & STAT_BOARD) || HAS_PERM(PERM_ALLBOARD)) )
        return XO_NONE;

    fhdr->xmode &= (~(POST_EXPIRE));
    fhdr->expire = 0;
    if (!rec_put(xo->dir, fhdr, sizeof(HDR), pos))
    {
        return XO_CUR;
    }
    return XO_NONE;
}

/* ----------------------------------------------------- */
/* �����\��Gedit / title                                */
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

    bno = currbno;
    brd = bshm->bcache + bno;

#endif

    if (bbsothermode & OTHERSTAT_EDITING)
    {
        vmsg("�A�٦��ɮ��٨S�s���@�I");
        return XO_FOOT;
    }
    pos = xo->pos;
    hdr = (HDR *) xo_pool_base + pos;

    pos = seek_rec(xo, hdr);

    hdr_fpath(fpath, xo->dir, hdr);
#if 0
    if ((HAS_PERM(PERM_ALLBOARD))|| ((HAS_PERM(PERM_VALID)) \
                                            && !strcmp(hdr->owner, cuser.userid)))
#endif
    if (HAS_PERM(PERM_SYSOP) && !(hdr->xmode & (POST_CANCEL|POST_DELETE)))
    {
        /*hdr = (HDR *) xo_pool_base + xo->pos;
        hdr_fpath(fpath, xo->dir, hdr);*/
        vedit(fpath, false); /* Thor.981020: �`�N�Qtalk�����D */
        return XO_HEAD;
    }
#ifdef  HAVE_USER_MODIFY
    if ((!(brd->battr & BRD_MODIFY)) && HAS_PERM(PERM_VALID) && !strcmp(hdr->owner, cuser.userid) && !(hdr->xmode & (/*POST_MODIFY|*/POST_CANCEL|POST_DELETE|POST_LOCK|POST_MARKED|POST_MDELETE|POST_CURMODIFY)))
    {
        if (hdr->xmode & POST_RECOMMEND)
        {
            if (!pid_find(hdr->xid))
                hdr->xmode &= ~POST_RECOMMEND;
            else
            {
                vmsg("���H�b���˱z���峹�A�еy�ԡC");
                return XO_FOOT;
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
//          if (!strncmp(str, "\x1b[1;33m��", 9))
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

        if ((temp = vedit(buf, false)) < 0)
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
                vmsg("�����ק�");
            else
            {
                brd->blast = time(0);
                vmsg("�ק粒��");
            }
            return XO_INIT;
        }
    }
    else if (brd->battr & BRD_MODIFY)
    {
        vmsg("���O����ק�峹!!");
    }
    else
    {
        /* cache.090922: �ˬd���� */
        if (hdr->modifytimes < 0)
            hdr->modifytimes = 0;
        vmsg("���峹����Q�ק�!!");
    }
#endif  /* #ifdef  HAVE_USER_MODIFY */
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

    bno = currbno;
    brd = bshm->bcache + bno;


#endif

    if (bbsothermode & OTHERSTAT_EDITING)
    {
        vmsg("�A�٦��ɮ��٨S�s���@�I");
        return XO_FOOT;
    }
    pos = xo->pos;
    hdr = (HDR *) xo_pool_base + xo->pos;
    hdr_fpath(fpath, xo->dir, hdr);
#if 0
    if ((HAS_PERM(PERM_ALLBOARD))|| ((HAS_PERM(PERM_VALID)) \
                && !strcmp(hdr->owner, cuser.userid)))
#endif
        if (HAS_PERM(PERM_ALLBOARD))
        {
            /*hdr = (HDR *) xo_pool_base + xo->pos;
                hdr_fpath(fpath, xo->dir, hdr);*/
            vedit(fpath, false); /* Thor.981020: �`�N�Qtalk�����D */
            return XO_HEAD;
        }
#ifdef  HAVE_USER_MODIFY
        if ((brd->battr & BRD_MODIFY) && HAS_PERM(PERM_VALID) /*&& ((hdr->modifytimes)<MAX_MODIFY)*/ && !strcmp(hdr->owner, cuser.userid) && !(hdr->xmode & (/*POST_MODIFY|*/POST_CANCEL|POST_DELETE|POST_LOCK|POST_MARKED|POST_MDELETE/*|POST_CURMODIFY*/)))
        {
            //    move_post(hdr, BRD_MODIFIED, -3);

            brd_fpath(mfolder, BRD_MODIFIED, FN_DIR);
            fd = hdr_stamp(mfolder, 'A', &phdr, mfpath);
            fp = fdopen(fd, "w");

            f_suck(fp, fpath);
            fclose(fp);
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

            if (vedit(buf, false)<0)
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
                vmsg("�����ק�");
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

                /* cache.090922: �ק隸���ˬd���� */
                /*
                    if (hdr->modifytimes < 0)
                        hdr->modifytimes = 1;
                    else
                        hdr->modifytimes += 1;
                 */
                vmsg("�ק粒��");
            }
            return XO_INIT;
        }
        else if (!(brd->battr & BRD_MODIFY))
        {
            vmsg("���ݪO����ק�峹!!");
            return XO_FOOT;
        }
        else
        {
            /* cache.090922: �ˬd���� */
            if (hdr->modifytimes < 0)
                hdr->modifytimes = 0;

            vmsg("���峹����Q�ק�!!");
            return XO_FOOT;
        }
#endif  /* #ifdef  HAVE_USER_MODIFY */
    return XO_NONE;
}
#endif  /* #if 0 */

void
header_replace(         /* 0911105.cache: �ק�峹���D���K�ק鷺�媺���D */
    XO *xo,
    const HDR *hdr)
{
    FILE *fpr, *fpw;
    char srcfile[64], tmpfile[64], buf[ANSILINESIZE];

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

    fgets(buf, sizeof(buf), fpr);               /* �[�J�@�� */
    fputs(buf, fpw);

    fgets(buf, sizeof(buf), fpr);               /* �[�J���D */
    if (!str_ncasecmp(buf, "��", 2))                /* �p�G�� header �~�� */
    {
        strcpy(buf, buf[2] == ' ' ? "��  �D: " : "���D: ");
        strcat(buf, hdr->title);
        strcat(buf, "\n");
    }
    fputs(buf, fpw);

    while (fgets(buf, sizeof(buf), fpr))        /* �[�J��L */
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
    int pos;

    pos = xo->pos;
    fhdr = (HDR *) xo_pool_base + pos;
    mhdr = *fhdr;

    /* 100620.cache: �@�̥i�H����D */
    //if (strcmp(mhdr.owner, cuser.userid))
    //{
    //    if (!(bbstate & STAT_BOARD))
    //        return XO_NONE;  /* 0911105.cache: �H���U�@ */
    //}

    if (!(bbstate & STAT_BOARD))
        return XO_NONE;

    vget(B_LINES_REF, 0, "���D�G", mhdr.title, sizeof(mhdr.title), GCARRY);

    if (HAS_PERM(PERM_ALLBOARD))  /* 0911105.cache: �D�ݪO�`�ޥu�����D */
    {
        vget(B_LINES_REF, 0, "�@�̡G", mhdr.owner, 74 /* sizeof(mhdr.owner)*/, GCARRY);
        /* Thor.980727:lkchu patch: sizeof(mhdr.owner) = 80�|�W�L�@�� */
        vget(B_LINES_REF, 0, "����G", mhdr.date, sizeof(mhdr.date), GCARRY);
    }

    if (vans(msg_sure_ny) == 'y' &&
            memcmp(fhdr, &mhdr, sizeof(HDR)))
    {
        *fhdr = mhdr;
        rec_put(xo->dir, fhdr, sizeof(HDR), pos);

        /* 0911105.cache: ���K�鷺����D */
        header_replace(xo, fhdr);

        return XR_FOOT + XO_CUR;
    }
    return XO_FOOT;
}


#ifdef HAVE_TERMINATOR
    static int
post_cross_terminator(  /* Thor.0521: �׷��峹�j�k */
    XO *xo)
{
    char *title, buf[128], other[128];
    int mode;
    HDR *fhdr;

    fhdr = (HDR *) xo_pool_base + xo->pos;
    if (fhdr->xmode & (POST_DELETE | POST_MDELETE | POST_CANCEL | POST_LOCK))
        return XO_NONE;


    if (!HAS_PERM(PERM_ALLBOARD))
        return XO_NONE;

    mode = vans("�m�تḨ���١n�G 1)����D 2)��ϥΪ� 3)��L [1]�G") - '1';
    if (mode > 2 || mode < 0)
        mode =0;

    strcpy(currtitle, str_ttl(fhdr->title));

    if (mode==1)
        title = fhdr->owner;
    else if (mode == 2)
    {
        if (!vget(B_LINES_REF, 0, "��L�G", other, sizeof(other), DOECHO))
            return XO_HEAD;
        title = other;
    }
    else
        title = currtitle;
    if (!*title)
        return XO_NONE;

    if (mode==1)
        sprintf(buf, "�m�تḨ���١n�ϥΪ̡G%.40s�A�T�w�ܡHY/[N]", title);
    else if (mode ==2)
        sprintf(buf, "�m�تḨ���١n��L�G%.50s�A�T�w�ܡHY/[N]", title);
    else
        sprintf(buf, "�m�تḨ���١n���D�G%.40s�A�T�w�ܡHY/[N]", title);


    if (vans(buf) == 'y')
    {
        BRD *bhdr, *head, *tail;

        /* Thor.0616: �O�U currboard, �H�K�_�� */
        strcpy(buf, currboard);

        head = bhdr = bshm->bcache;
        tail = bhdr + bshm->number;
        do                              /* �ܤ֦�sysop�@�� */
        {
            int fdr, fsize, xmode;
            FILE *fpw;
            char fpath[80];
            char fnew[80], fold[80];
            HDR *hdr;

            if (!str_casecmp(head->brdname, BRD_LOCALPOSTS))  /* LocalPosts ������ */
                continue;

            if (!str_casecmp(head->brdname, brd_sysop))  /* SYSOP ������ */
                continue;

            if (!str_casecmp(head->brdname, BRD_CAMERA))  /* ActiveInfo ������ */
                continue;

#ifdef  HAVE_CROSSPOSTLOG
            if (!str_casecmp(head->brdname, BRD_CROSSPOST))  /* CostPost ������ */
                continue;
#endif

            /* Thor.0616:���currboard, �Hcancel post */

            strcpy(currboard, head->brdname);

            sprintf(fpath, "�m�تḨ���١n�ݪ��G%s \x1b[5m...\x1b[m", currboard);
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
            while ((hdr = (HDR *) mread(fdr, sizeof(HDR))))
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
                    /* �Y���ݪO�N�s�u��H */

                    cancel_post(hdr);
                    hdr->xmode |= POST_MDELETE;
                    sprintf(hdr->title, "<< ���峹�� %s �H�t�Υ\\��R�� >>", cuser.userid);
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
        return XO_LOAD;
    }

    return XO_FOOT;
}
#endif  /* #ifdef HAVE_TERMINATOR */

    int
post_ban_mail(
    XO *xo)
{

    if ((bbstate & STAT_BOARD)||HAS_PERM(PERM_ALLBOARD))
    {
        post_mail();
        return XO_INIT;
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

    if ( !(bbstate & STAT_BOARD) ) /* �P�� visor@YZU */
        return XO_NONE;

    bno = currbno;
    oldbrd = bshm->bcache + bno;

    memcpy(&newbrd, oldbrd, sizeof(BRD));

    vget(23, 0, "�ݪO�W�١G", newbrd.title+3, BTLEN - 2, GCARRY);

    if ((vans(msg_sure_ny) == 'y') &&
            memcmp(&newbrd, oldbrd, sizeof(BRD)))
    {
        memcpy(oldbrd, &newbrd, sizeof(BRD));
        rec_put(FN_BRD, &newbrd, sizeof(BRD), bno);
    }
    vmsg("�]�w����");

    return XO_HEAD;
}
#endif  /* #ifdef  HAVE_BRDTITLE_CHANGE */



#ifdef HAVE_RECOMMEND   /* gaod: ���� */

/* 090907.cache ���媺 code �ݭץ� */

    void
record_recommend(const int chrono, const char * const text)
{
    char desc[256]/*, fpath[128]*/;
    time_t now;

    time(&now);

    /* �O����t�� */
    snprintf(desc, sizeof(desc), "%s %s ����G%s %d %s\n", Atime(&now),
            cuser.userid, currboard, chrono, text);
    f_cat(FN_RECOMMEND_LOG, desc);
}

/*
    void
post_recommend_log(
    int mode,
    const HDR *hdr)
{
    time_t now;
    char c_time[25], buf[300];

    now = time(0);
    str_scpy(c_time, ctime(&now), sizeof(c_time));

    sprintf(buf, "%s %s %s %s �O�G%s(%lld) from %s\n", c_time, cuser.userid, (mode == 0) ? "�M��":"����", currboard, hdr->title, (long long)hdr->chrono, fromhost);
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
        int pos, xmode GCC_UNUSED, recommend, pm;
        char ans[3];

        pos = xo->pos;
        hdr = (HDR *) xo_pool_base + pos;

        xmode = hdr->xmode;
        brd = bshm->bcache + currbno;

        if (hdr->xmode & (POST_DELETE | POST_CANCEL | POST_MDELETE | POST_LOCK | POST_CURMODIFY))
            return XO_FOOT;

        //if (!hdr->recommend)
        //{
        //    vmsg("���g�峹�S������");
        //    return XO_FOOT;
        //}
        //else
        //{
            switch (vans("�������]�w 1)�ۭq 2)�M�� [Q] "))
            {
                case '1':

                    if (!HAS_PERM(PERM_SYSOP))
                    {
                        pmsg2("�ثe�T��ۭq�����");
                        return XO_FOOT;
                    }

                    if (!vget(B_LINES_REF, 0, "�п�J�Ʀr�G", ans, 3, DOECHO))
                        return XO_FOOT;

                    if ((brd->battr & BRD_PUSHSNEER) || (brd->battr & BRD_PUSHDEFINE))
                        pm = vans("�п�ܥ��t 1)�� 2)�t [Q] ");
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
            return XR_LOAD + XO_CUR;
        //}


    }
    else
    {
        vmsg("�z���v�������I");
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
    char fpath[80], msg[53], add[128], lastrecommend[IDLEN+1], verb[3];
    char ans, getans, pushverb GCC_UNUSED;

    /* 081122.cache: ����ɶ����� */
    static time_t next = 0;

    brd = bshm->bcache + currbno;

    if (HAS_PERM(PERM_VALID) && (!(brd->battr & BRD_PRH)) && (bbstate & STAT_POST))
    {
        pos = xo->pos;
        cur = pos - xo->top;
        hdr = (HDR *) xo_pool_base + pos;

        if (!strcmp(hdr->lastrecommend, "$"))
        {
            zmsg("���峹���i���ˡI");
            return XO_NONE;
        }

        /* 081122.cache: ����ɶ����� */
        if (brd->battr & BRD_PUSHTIME)
        {
            if ((ans = next - time(NULL)) > 0)
            {
                sprintf(fpath, "�٦� %d ��~������", ans);
                vmsg(fpath);
                return XO_FOOT;
            }
        }

        //��s��ƾ޵w��
        pos = seek_rec(xo, hdr);

        if (pos < 0)
            return XO_NONE;

        if (hdr->xmode & (POST_DELETE | POST_CANCEL | POST_MDELETE | POST_LOCK))
            return XO_NONE;
        else if (hdr->xmode & POST_CURMODIFY)
        {
            zmsg("�@�̭ק�峹���A�еy�ԡC");
            return XO_NONE;
        }

        else if (brd->battr & BRD_PUSHDISCON)
        {
            if (!strcmp(hdr->lastrecommend, cuser.userid))
            {
                zmsg("���i�s����P�ˤ@�g�峹�I�ۤv�]����Ĥ@��");
                return XO_NONE;
            }
        }

        hdr->xmode |= POST_RECOMMEND_ING;
        hdr->xid = cutmp->pid;

        rec_put(xo->dir, hdr, sizeof(HDR), pos);

        /* 081121.cache: ���N��\�� */
        /* 081122.cache: �ۭq���N��ʵ� */
        if (brd->battr & BRD_PUSHSNEER)
        {
            //addscore = 0;
            //switch (ans = vans("�� ���� 1)���� 2)�N�� 3)�d�� �H[Q] "))
            //�Ҷq�ڤj�w�g�ߺD����O�b�Y�Ÿ�
            switch (ans = vans("�� ���� 1)���� 2)�N�� �H[Q] "))
            {
                case '1':
                    getans = vget(B_LINES_REF, 0, "����G", msg, 53, DOECHO);
                    addscore = 1;
                    break;
                case '2':
                    getans = vget(B_LINES_REF, 0, "�N��G", msg, 53, DOECHO);
                    addscore = -1;
                    break;
                case '3':
                    getans = vget(B_LINES_REF, 0, "�d���G", msg, 53, DOECHO);
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
            switch (ans = vans("�� ���� 1)���� 2)�N�� 3)�d�� 4)�ۭq���� 5)�ۭq�N�� �H[Q] "))
            {
                case '1':
                    getans = vget(B_LINES_REF, 0, "����G", msg, 53, DOECHO);
                    strcpy(verb, "��");
                    addscore = 1;
                    break;
                case '2':
                    getans = vget(B_LINES_REF, 0, "�N��G", msg, 53, DOECHO);
                    strcpy(verb, "�N");
                    addscore = -1;
                    break;
                case '3':
                    getans = vget(B_LINES_REF, 0, "�d���G", msg, 53, DOECHO);
                    addscore = 0;
                    break;

                case '4':
                    pushverb = vget(B_LINES_REF, 0, "�п�J�ۭq�������ʵ��G", verb, 3, DOECHO);
                    eof = strlen(verb);
                    if (eof<2)
                    {
                        zmsg("�ʵ������@�Ӥ���r���Ϊ̨�ӭ^��r��");
                        return XO_FOOT;
                    }
                    getans = vget(B_LINES_REF, 0, "����G", msg, 53, DOECHO);
                    addscore = 1;
                    break;

                case '5':
                    pushverb = vget(B_LINES_REF, 0, "�п�J�ۭq���t���ʵ��G", verb, 3, DOECHO);
                    eof = strlen(verb);
                    if (eof<2)
                    {
                        zmsg("�ʵ������@�Ӥ���r���Ϊ̨�ӭ^��r��");
                        return XO_FOOT;
                    }
                    getans = vget(B_LINES_REF, 0, "�N��G", msg, 53, DOECHO);
                    addscore = -1;
                    break;

                default:
                    getans = 0;
                    break;
            }
        }
        else
            getans = vget(B_LINES_REF, 0, "����G", msg, 53, DOECHO);

        /* 081121.cache: �ᮬ�����| */
        if (getans)
            ans = vans("�нT�w�O�_�e�X ? [y/N]");
        else
            ans = 'n';

        //��s��ƾ޵w��
        pos = seek_rec(xo, hdr);
        hdr->xmode &= ~POST_RECOMMEND_ING;
        hdr->xid = 0;

        if (pos < 0)
            return XO_NONE;

        //�u�H��
        if ( (brd->battr & (BRD_PUSHTIME | BRD_PUSHDISCON)) && (brd->battr & BRD_VALUE) )
        {
            if      (/*hdr->recommend == 49 || */hdr->recommend == 99)
            {
                if (addscore > 0)
                    point = 1;
                else
                    point = -1;
            }
            else if (/*hdr->recommend == -49 || */hdr->recommend == -99)
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

            //�b item ���P�_�i�H�[�֤@�I�I
            //�Ҷq���ª��峹�ۮe���D, �����¤峹�M����n���P�_
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

                    /* 090923.cache: �p�G�o�� race condition, �W���[�������ѱ���γo�q*/
                    /* ������ .DIR ���� score ��s�A���� XO �̭��� score �O�O���h�� */
                    /*
                        hdr->recommend +=addscore;
                     */
                }
            }
            else //�L�N��ۮe��
            {
                if (hdr->recommend < 99)
                    hdr->recommend++;
            }

            strcpy(hdr->lastrecommend, cuser.userid);
            rec_put(xo->dir, hdr, sizeof(HDR), pos);

            hdr_fpath(fpath, xo->dir, hdr);

            /* 081121.cache: ���N��M���q���˦����P��outs */
            /* 081122.cache: �ۭq���N��ʵ� */
            if (brd->battr & BRD_PUSHSNEER)
            {
                if (addscore == 1)
                    sprintf(add, "\x1b[1;33m�� %*s�G\x1b[36m%-54.54s \x1b[m%5.5s\n", IDLEN, cuser.userid, msg, Btime(&hdr->pushtime)+3);
                else if (addscore == -1)
                    sprintf(add, "\x1b[1;31m�N\x1b[m \x1b[1;33m%*s�G\x1b[36m%-54.54s \x1b[m%5.5s\n", IDLEN, cuser.userid, msg, Btime(&hdr->pushtime)+3);
                else
                    sprintf(add, "\x1b[m\x1b[1;33m   %*s�G\x1b[36m%-54.54s \x1b[m%5.5s\n", IDLEN, cuser.userid, msg, Btime(&hdr->pushtime)+3);
            }
            else if (brd->battr & BRD_PUSHDEFINE)
            {
                if (addscore == 1)
                    sprintf(add, "\x1b[1;33m%2.2s %*s�G\x1b[36m%-54.54s \x1b[m%5.5s\n", verb, IDLEN, cuser.userid, msg, Btime(&hdr->pushtime)+3);
                else if (addscore == -1)
                    sprintf(add, "\x1b[1;31m%2.2s\x1b[m \x1b[1;33m%*s�G\x1b[36m%-54.54s \x1b[m%5.5s\n", verb, IDLEN, cuser.userid, msg, Btime(&hdr->pushtime)+3);
                else
                    sprintf(add, "\x1b[1;33m��\x1b[m \x1b[1;33m%*s�G\x1b[36m%-54.54s \x1b[m%5.5s\n", IDLEN, cuser.userid, msg, Btime(&hdr->pushtime)+3);
            }
            else
                sprintf(add, "\x1b[1;33m�� %*s�G\x1b[36m%-54.54s \x1b[m%5.5s\n", IDLEN, cuser.userid, msg, Btime(&hdr->pushtime)+3);
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

            /* 081122.cache: ����ɶ����� */
            if (brd->battr & BRD_PUSHTIME)
                next = time(NULL) + NEXTPUSHTIME;  /* �w�q�btheme.h */

            //change_stamp(xo->dir, hdr);
            brh_add(hdr->pushtime, hdr->pushtime, hdr->pushtime);

            /* 091009.cache: �u�}�n�� */
            if ( point!=0 )
            {
                addpoint1(point, hdr->owner);
                pmsg2("���ק����I(�@���u����)");
            }
            else
                zmsg("���ק����I");

            /* �������� */
            if (!HAVE_PERM(PERM_SYSOP))
                record_recommend(hdr->chrono, msg);

            //    post_recommend_log(1, hdr);
            //    brd->btime = time(0);[A
            //btime_update(currbno);
            brd->blast = hdr->pushtime;
            return XO_INIT;
            //move(3 + cur, 0);
            //post_item(pos+1, hdr);
            //cursor_show(3 + cur, 0);

        }
        else
        {
            strcpy(hdr->lastrecommend, lastrecommend);
            rec_put(xo->dir, hdr, sizeof(HDR), pos);
            zmsg(MSG_CANCEL);
        }
    }
    return XO_FOOT;
}

#endif  /* #ifdef HAVE_RECOMMEND */

/* cache.081122: �ݪO��T��� */
    static int
post_showBRD_setting(
    XO *xo)
{
    const char *str;
    BRD *brd;

    brd = bshm->bcache + currbno;

    str = brd->BM;
    if (*str <= ' ')
        str = "\x1b[1;33m�x�D��\x1b[m";

    grayout(0, b_lines, GRAYOUT_DARK);

    move(b_lines - 14, 0);
    clrtobot();  /* �קK�e���ݯd */

    prints("\x1b[1;34m");
    outsep(b_cols, MSG_BLINE);
    prints("\n\x1b[1;33;44m \x1b[37m�ݪO�]�w�θ�T�d�ߡG %*s \x1b[m\n", d_cols + 56, "");

    prints("\n�ݪO:[%s]  �O�D:[%s] \n", brd->brdname, str);

    prints("\n �ݪO�ʽ� - %s",
            (brd->battr & BRD_NOTRAN) ? "����" : "\x1b[1;33m��H\x1b[m");

    if (brd->battr & BRD_RSS)
        prints("    RSS �\\�� - " URL_PREFIX "/%s.xml ", brd->brdname);
    else
        prints("    RSS �\\�� - ����");

    prints("\n �O���g�� - %s    ����峹 - %s",
            (brd->battr & BRD_NOCOUNT) ? "����" : "\x1b[1;33m�O��\x1b[m",
            (brd->battr & BRD_NOFORWARD) ? "\x1b[1;31m���i���\x1b[m" : "�i�H���");

    prints("\n �������D - %s    �ݪO���A - %s",
            (brd->battr & BRD_NOSTAT) ? "����" : "\x1b[1;33m�O��\x1b[m",
            (brd->battr & BRD_NOREPLY) ? "\x1b[1;31m�ݪO��Ū\x1b[m" : "�ۥѵo��");

    prints("\n �벼���G - %s    �ק�O�W - %s",
            (brd->battr & BRD_NOVOTE) ? "����" : "\x1b[1;33m�O��\x1b[m",
            (brd->battr & BRD_CHANGETITLE) ? "�i�H�ק�" : "\x1b[1;33m����ק�\x1b[m");

    prints("\n �ΦW�\\�� - %s    �ק�峹 - %s",
            (brd->battr & BRD_ANONYMOUS) ? "\x1b[1;33m�}��\x1b[m" : "����",
            (brd->battr & BRD_MODIFY) ? "\x1b[1;31m����ק�\x1b[m" : "�i�H�ק�");

    prints("\n ����\\�� - %s    �N��\\�� - %s",
            (brd->battr & BRD_PRH) ? "����" : "\x1b[1;33m�}��\x1b[m",
            (brd->battr & BRD_PUSHSNEER || brd->battr & BRD_PUSHDEFINE) ? "\x1b[1;33m�}��\x1b[m" : "����");

    prints("\n ���孭�� - %s    �ۭq�ʵ� - %s",
            (brd->battr & BRD_PUSHDISCON) ? "\x1b[1;36m�ע�\x1b[m" : (brd->battr & BRD_PUSHTIME) ?
            "\x1b[1;36m�ɶ�\x1b[m" : "�S��",
            (brd->battr & BRD_PUSHDEFINE) ? "\x1b[1;33m�}��\x1b[m" : "����");

    prints("\n �峹���O - %s    �T�`���� - %s",
            (brd->battr & BRD_POSTFIX) ? "\x1b[1;33m�}��\x1b[m" : "����",
            (brd->battr & BRD_NOPHONETIC) ? "\x1b[1;33m�}��\x1b[m" : "����");

    prints("\n �i�O�O�� - %s    ",
            (brd->battr & BRD_USIES) ? "\x1b[1;33m�}��\x1b[m" : "����");

    if ((bbstate & STAT_BOARD) || HAS_PERM(PERM_BOARD))
        prints("\n\n�z�ثe \x1b[1;33m�֦�\x1b[m ���ݪO���޲z�v��");
    else
        prints("\n\n�z�ثe �S�� ���ݪO���޲z�v��");

    vmsg(NULL);

    return XO_HEAD;
}

/* magicallove.081207: �����O�_���n�ͪO */
    static int
post_FriendSet(
    XO *xo)
{
    if (!(bbstate & STAT_BOARD))
        return XO_NONE;
    //�P�_����

    BRD *oldbrd, newbrd;
    int bno;
    bno = currbno;

    oldbrd = bshm->bcache + bno;
    memcpy(&newbrd, oldbrd, sizeof(BRD));

    if (vans("�T�w�n�ܧ�ݪO�v���H[y/N] ") != 'y')
        return XO_HEAD;

    //���X��
    if (newbrd.readlevel & PERM_SYSOP) {
        newbrd.readlevel = 0;
        vmsg("�ثe�����}�ݪO");
    }
    else {
        newbrd.readlevel = PERM_SYSOP;
        vmsg("�ثe���n�ͬݪO");
    }

    memcpy(oldbrd, &newbrd, sizeof(BRD));
    rec_put(FN_BRD, &newbrd, sizeof(BRD), bno);

    return XO_HEAD;
}

/* cache.090412: �����i�_���� */
/* cache.090928 �ݪO�����ݩ� */
    static int
post_battr_score(
    XO *xo)
{
    //�P�_�O�_���O�D
    if (!(bbstate & STAT_BOARD))
        return XO_NONE;
    //�P�_����

    BRD *oldbrd, newbrd;
    int bno;
    bno = currbno;

    oldbrd = bshm->bcache + bno;
    memcpy(&newbrd, oldbrd, sizeof(BRD));

    switch (vans("������]�w 1)����\\�� 2)�N�� 3)�ۭq�ʵ� 4)�PID���� 5)�ɶ����� [Q] "))
    {
        case '1':
            if (vans("�T�w�n�ܧ����]�w�H[y/N] ") != 'y')
                return XO_HEAD;
            //���X��
            if (newbrd.battr & BRD_PRH) {
                newbrd.battr &= ~BRD_PRH;
                vmsg("���\\����");
            }
            else {
                newbrd.battr |= BRD_PRH;
                newbrd.battr &= ~BRD_PUSHDISCON;
                newbrd.battr &= ~BRD_PUSHTIME;
                newbrd.battr &= ~BRD_PUSHSNEER;
                newbrd.battr &= ~BRD_PUSHDEFINE;
                vmsg("�T�����");
            }
            memcpy(oldbrd, &newbrd, sizeof(BRD));
            rec_put(FN_BRD, &newbrd, sizeof(BRD), bno);
            return XO_HEAD;

        case '2':
            if (vans("�T�w�n�ܧ�N��Ҧ��H[y/N] ") != 'y')
                return XO_HEAD;
            //���X��
            if (newbrd.battr & BRD_PUSHSNEER) {
                newbrd.battr &= ~BRD_PUSHSNEER;
                vmsg("�����N��Ҧ�");
            }
            else {
                newbrd.battr |= BRD_PUSHSNEER;
                newbrd.battr &= ~BRD_PUSHDEFINE;
                vmsg("�}�ҼN��Ҧ�");
            }
            memcpy(oldbrd, &newbrd, sizeof(BRD));
            rec_put(FN_BRD, &newbrd, sizeof(BRD), bno);
            return XO_HEAD;

        case '3':
            if (vans("�T�w�n�ܧ�ۭq����ʵ��Ҧ��H[y/N] ") != 'y')
                return XO_HEAD;
            //���X��
            if (newbrd.battr & BRD_PUSHDEFINE) {
                newbrd.battr &= ~BRD_PUSHDEFINE;
                vmsg("�����ۭq����ʵ�");
            }
            else {
                newbrd.battr |= BRD_PUSHDEFINE;
                newbrd.battr &= ~BRD_PUSHSNEER;
                vmsg("�}�Ҧۭq����ʵ�");
            }
            memcpy(oldbrd, &newbrd, sizeof(BRD));
            rec_put(FN_BRD, &newbrd, sizeof(BRD), bno);
            return XO_HEAD;

        case '4':
            if (vans("�T�w�n�ܧ�ID�s������H[y/N] ") != 'y')
                return XO_HEAD;
            //���X��
            if (newbrd.battr & BRD_PUSHDISCON) {
                newbrd.battr &= ~BRD_PUSHDISCON;
                vmsg("�PID���\\�s��");
            }
            else {
                newbrd.battr |= BRD_PUSHDISCON;
                vmsg("�PID�T��s��");
            }
            memcpy(oldbrd, &newbrd, sizeof(BRD));
            rec_put(FN_BRD, &newbrd, sizeof(BRD), bno);
            return XO_HEAD;

        case '5':
            if (vans("�T�w�n�ܧ�ɶ��s������H[y/N] ") != 'y')
                return XO_HEAD;
            //���X��
            if (newbrd.battr & BRD_PUSHTIME) {
                newbrd.battr &= ~BRD_PUSHTIME;
                vmsg("���\\�ֳt�s��");
            }
            else {
                newbrd.battr |= BRD_PUSHTIME;
                vmsg("�T��ֳt�s��");
            }
            memcpy(oldbrd, &newbrd, sizeof(BRD));
            rec_put(FN_BRD, &newbrd, sizeof(BRD), bno);
            return XO_HEAD;

        default:
            return XO_HEAD;
    }
}

/* cache.100917: RSS �]�w�\�� */
/* cache.090928: �ݪO��Ū, �@�̭פ�, ���]�w */
/* cache.090928: �ݪO�����ݩ� */
    static int
post_rule(
    XO *xo)
{
    //�P�_�O�_���O�D
    if (!(bbstate & STAT_BOARD))
        return XO_NONE;
    //�P�_����

    BRD *oldbrd, newbrd;
    int bno;
    bno = currbno;

    oldbrd = bshm->bcache + bno;
    memcpy(&newbrd, oldbrd, sizeof(BRD));

    switch (vans("���ݪO�]�w 1)���O��Ū 2)�@�̭פ� 3)����峹 4)�T�`���� 5)RSS�\\�� [Q] "))
    {
        case '1':
            if (vans("�T�w�n�ܧ�ݪO��Ū�]�w�H[y/N] ") != 'y')
                return XO_HEAD;
            //���X��
            if (newbrd.battr & BRD_NOREPLY) {
                newbrd.battr &= ~BRD_NOREPLY;
                vmsg("������Ū");
            }
            else {
                newbrd.battr |= BRD_NOREPLY;
                newbrd.battr |= BRD_PRH;
                newbrd.battr &= ~BRD_PUSHDISCON;
                newbrd.battr &= ~BRD_PUSHTIME;
                newbrd.battr &= ~BRD_PUSHSNEER;
                newbrd.battr &= ~BRD_PUSHDEFINE;
                vmsg("�ݪO��Ū - �T��o��^��α���");
            }
            memcpy(oldbrd, &newbrd, sizeof(BRD));
            rec_put(FN_BRD, &newbrd, sizeof(BRD), bno);
            return XO_HEAD;

        case '2':
            if (vans("�T�w�n�ܧ�@�̭פ�]�w�H[y/N] ") != 'y')
                return XO_HEAD;
            //���X��
            if (newbrd.battr & BRD_MODIFY) {
                newbrd.battr &= ~BRD_MODIFY;
                vmsg("���\\�@�̭פ�");
            }
            else {
                newbrd.battr |= BRD_MODIFY;
                vmsg("�T��@�̭פ�");
            }
            memcpy(oldbrd, &newbrd, sizeof(BRD));
            rec_put(FN_BRD, &newbrd, sizeof(BRD), bno);
            return XO_HEAD;

        case '3':
            if (vans("�T�w�n�ܧ�����峹�]�w�H[y/N] ") != 'y')
                return XO_HEAD;
            //���X��
            if (newbrd.battr & BRD_NOFORWARD) {
                newbrd.battr &= ~BRD_NOFORWARD;
                vmsg("���\\����峹");
            }
            else {
                newbrd.battr |= BRD_NOFORWARD;
                vmsg("�T������峹");
            }
            memcpy(oldbrd, &newbrd, sizeof(BRD));
            rec_put(FN_BRD, &newbrd, sizeof(BRD), bno);
            return XO_HEAD;

        case '4':
            if (vans("�T�w�n�ܧ�`���孭��]�w�H[y/N] ") != 'y')
                return XO_HEAD;
            //���X��
            if (newbrd.battr & BRD_NOPHONETIC) {
                newbrd.battr &= ~BRD_NOPHONETIC;
                vmsg("���\\�`����");
            }
            else {
                newbrd.battr |= BRD_NOPHONETIC;
                vmsg("�T��`����");
            }
            memcpy(oldbrd, &newbrd, sizeof(BRD));
            rec_put(FN_BRD, &newbrd, sizeof(BRD), bno);
            return XO_HEAD;

        case '5':
            if (vans("�T�w�n�ܧ�ݪORSS�]�w�H[y/N] ") != 'y')
                return XO_HEAD;
            //���X��
            if (newbrd.battr & BRD_RSS) {
                newbrd.battr &= ~BRD_RSS;
                vmsg("����RSS�\\��");
            }
            else {
                newbrd.battr |= BRD_RSS;
                vmsg("�}��RSS�\\��");
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
    bno = currbno;

    oldbrd = bshm->bcache + bno;
    memcpy(&newbrd, oldbrd, sizeof(BRD));

    brd_fpath(fpath, newbrd.brdname, FN_THRESHOLD);

    switch (ans = vans("�� �o����e���� 1)��������e 2)������e [Q] "))
    {
        case '1':
            newbrd.battr &= ~BRD_THRESHOLD;
            break;

        case '2':
            newbrd.battr |= BRD_THRESHOLD;

            echo = rec_get(fpath, &th, sizeof(THRESHOLD), 0) ? DOECHO : GCARRY;

            if (echo & GCARRY)
                sprintf(buf, "%d", th.age);
            if (!vget(B_LINES_REF, 0, "�п�J�o����e�е��U�X�ѥH�W�H", buf, 4, echo))
                return XO_HEAD;
            if ((num = atoi(buf)) < 0)
                return XO_HEAD;
            th.age = num;

            if (echo & GCARRY)
                sprintf(buf, "%d", th.numlogins);
            if (!vget(B_LINES_REF, 0, "�п�J�o����e�еn�J�X���H�W�H", buf, 4, echo))
                return XO_HEAD;
            if ((num = atoi(buf)) < 0)
                return XO_HEAD;
            th.numlogins = num;

            if (echo & GCARRY)
                sprintf(buf, "%d", th.numposts);
            if (!vget(B_LINES_REF, 0, "�п�J�o����e�еo��X�g�H�W�H", buf, 4, echo))
                return XO_HEAD;
            if ((num = atoi(buf)) < 0)
                return XO_HEAD;
            th.numposts = num;

            if (echo & GCARRY)
                sprintf(buf, "%d", th.point2);
            if (!vget(B_LINES_REF, 0, "�п�J�o����e�ЦH��X�g�H�U�H", buf, 4, echo))
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
                vans("�аݬO�_�R���o�ǬݪO�\\Ū�O��(y/N)�H[N] ") == 'y')
            unlink(fpath);
    }
    else
        pmsg2("�ثe�����Ѭd��");

    return XO_HEAD;
}

    int
post_manage(
    XO *xo)
{
    char re;

    const char *const menu[] =
    {
        "BQ",
        "Title   �ק�ݪO�D�D",
        "Memo    �s��i�O�e��",
        "Post    �s��o�夽�i",
        "Banmail �ݪO�ɫH�]�w",
        "Close   �ݪO�o��]�w",
#  ifdef HAVE_RECOMMEND
        "Score   �ݪO����]�w",
#  endif

#  ifdef HAVE_MODERATED_BOARD
        "Level   �ݪO�ݩʳ]�w",
        "OPal    �O�ͤ���]�w",
#  endif
        "ZLevel  �o����e�]�w",
        "Usies   �ݪO�\\Ū�O��",
        "Quit    ���}���",
        NULL
    };

    if (!(bbstate & STAT_BOARD))
    {
        vmsg(NULL);
        return XO_HEAD;
    }

    grayout(0, b_lines, GRAYOUT_DARK);

    switch (re = popupmenu_ans2(menu, "�O�D�޲z", (B_LINES_REF >> 1) - 8, (D_COLS_REF >> 1) + 20))
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
    const char *query;
    char aid[9];
    int currpos, pos, max, match = 0;
    const HDR *hdr;

    /* �O�s�ثe�Ҧb����m */
    currpos = xo->pos;
    /* �����ݪO�峹�`�ơF�s�K��AID�O���������i��b�䤤 */
    max = xo->max;

    /* �Y�S���峹�Ψ�L�ҥ~���p */
    if (max <= 0)
        return XO_FOOT;

    /* �ШD�ϥΪ̿�J�峹�N�X(AID) */
    if (!vget(B_LINES_REF, 0, "�п�J�峹�N�X(AID)�G #", aid, sizeof(aid), DOECHO))
        return XO_FOOT;
    query = aid;

    for (pos = 0; pos < max; pos++)
    {
        const char *tag;
        xo->pos = pos;
        xo_load(xo, sizeof(HDR));
        /* �]�wHDR��T */
        hdr = (const HDR *) xo_pool_base + xo->pos;
        tag = hdr->xname;
        /* �Y���������峹�A�h�]�wmatch�ø��X */
        if (!strcmp(query, tag))
        {
            match = 1;
            break;
        }
    }

    /* �S���h��_xo->pos����ܴ��ܤ�r */
    if (!match)
    {
        zmsg("�䤣��峹�A�O���O����ݪO�F�O�H");
        xo->pos = currpos;  /* ��_xo->pos���� */
    }

    return XO_LOAD;
}

int
post_write(                  /* ��u�W�@�̼��T */
    XO *xo)
{
    if (HAS_PERM(PERM_PAGE))
    {
        const HDR *fhdr;
        UTMP *up;

        fhdr = (const HDR *) xo_pool_base + xo->pos;

        if ((up = utmp_check(fhdr->owner)) && can_message(up))
/*      if ((up = utmp_check(fhdr->owner)) && can_override(up))*/
        {
            BMW bmw;
            char buf[20];

            sprintf(buf, "��[%s]", up->userid);
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
    return XO_HEAD;
}


    static int
post_spam(
    XO *xo)
{
    const HDR *hdr;
    char *dir, fpath[80];
    char msg[128];


    if (!supervisor)
        return XO_NONE;

    dir = xo->dir;
    hdr = (const HDR *) xo_pool_base + xo->pos;
    hdr_fpath(fpath, dir, hdr);

    sprintf(msg, "%s\n", fpath);
    f_cat(FN_SPAMPATH_LOG, msg);

    vmsg(fpath);

    return XO_FOOT;
}



KeyFuncList post_cb =
{
    {XO_INIT, {post_init}},
    {XO_LOAD, {post_load}},
    {XO_HEAD, {post_head}},
    {XO_NECK, {post_neck}},
    {XO_BODY, {post_body}},
    {XO_FOOT, {post_foot}},
    {XO_CUR, {post_cur}},

    {'B', {post_manage}},
    {'r', {post_browse}},
    {'s', {post_switch}},
    {KEY_TAB, {post_gem}},
    {'z', {post_gem}},
    {'u', {post_undelete}},
    {'y', {post_reply}},
    {'d', {post_delete}},
    {'v', {post_visit}},
    {'q', {post_state}},
    {'S', {post_complete}}, //�O�D�B�z�аO
    {'w', {post_write}},
    {Ctrl('W'), {post_spam}},
    {Meta('W'), {post_spam}},
    {'e', {post_expire}},
    {'U', {post_unexpire}},
    {'#', {post_aid}},              /* cache.090612: �H�峹�N�X(AID)�ֳt�M�� */
    {'i', {post_showBRD_setting}},  /* cache.081122:�ݪO��T��� */
    {Ctrl('P'), {post_add}},
    {Ctrl('N'), {post_clean_delete}},
    {Meta('N'), {post_clean_delete}},
#ifdef HAVE_MULTI_CROSSPOST
    {Ctrl('X'), {post_xcross}},
#endif
    {'x', {post_cross}},
    {Ctrl('Q'), {xo_uquery_lite}},
    //  {'I', {xo_usetup}}, /* cache.081122: ���ǤH�|�ѰO�]�T��d�߸��, ��ĳ�����O�@���p */
#if 1 /* Thor.981120: �Ȯɨ���, ���~�� */
    /* lkchu.981201: �S�� 'D' �ܤ��ߺD :p */
    {'D', {xo_delete}},
#endif

#ifdef HAVE_TERMINATOR
    {'Z', {post_cross_terminator}},
#endif

    {'t', {post_tag}},
    {'l', {post_lock}},

    {'E', {post_edit}},
    {'T', {post_title}},
    {'m', {post_mark}},

#ifdef  HAVE_RECOMMEND
    {'X', {post_recommend}},
    {'%', {post_recommend}},           /* r2.20170802: �P itoc ������q�� */
    //  {'o', {post_recommend_options}},
    {'o' | XO_DL, {.dlfunc = DL_NAME("cleanrecommend.so", clean)}},
    {Ctrl('S'), {post_resetscore}},         /* cache.090416: ����]�w */
#endif

    {'R' | XO_DL, {.dlfunc = DL_NAME("vote.so", vote_result)}},
    {'V' | XO_DL, {.dlfunc = DL_NAME("vote.so", XoVote)}},

    {'b', {post_memo}},

#ifdef HAVE_MODERATED_BOARD
    {Ctrl('G'), {XoBM}},
#endif

#ifdef XZ_XPOST
    {'/', {XoXpost}},                     /* Thor: for XoXpost */
    {'~', {XoXpost}},                     /* Thor: for XoXpost */
#endif

#ifdef HAVE_POST_BOTTOM
    {'_', {post_bottom}},
#endif

    {'h', {post_help}}
};


#ifdef XZ_XPOST
/*------------------------------------------------------------------------
  Thor.0509: �s�� �峹�j�M�Ҧ�
  �i���w�@keyword, �C�X�Ҧ�keyword�������峹�C��

  �b tmp/ �U�} xpost.{pid} �@�� folder, �t�ؤ@map�}�C, �Χ@�P��post�@map
  �O���Ӥ峹�O�b��post����B, �p���i�@ mark, gem, edit, title���\��,
  �B�����}�ɦ^�ܹ����峹�B
  <�H�W�Q�k obsolete...>

  Thor.0510:
  �إߤ峹�Q�צ�, like tin, �N�峹�� index ��J memory��,
  ���ϥ� thread, �]�� thread�n�� folder��...

  �������Mode, Title & post list

  ���Ҽ{����²�ƪ� �W�U�䲾��..

  O->O->O->...
  |  |  |
  o  o  o
  |  |  |

  index�tfield {next, text} ����int, �t�m�]�� int
  �Ĥ@�h sorted by title, ���J�ɥ� binary search
  �B MMAP only, �Ĥ@�h��� # and +

  �����ѥ���R���ʧ@, �קK�V��

  Thor.980911: �Ҽ{���ѧR�����O, ��K���D
  -------------------------------------------------------------------------*/
#if 0
extern XO *xpost_xo;            /* Thor: dynamic programming for variable dir
                                 * name */
extern XO *ypost_xo;
#endif


#define MSG_XYPOST      "[�걵�Ҧ�]���D����r:"
#define MSG_XY_NONE     "�ŵL�@��"


typedef struct
{
    char *subject;
    int first;
    int last;
    time_t chrono;
}      Chain;                   /* Thor: negative is end */



    static int
chain_cmp(
    const void *a,
    const void *b)
{
    return ((const Chain *)a)->chrono - ((const Chain *)b)->chrono;
}


static int *xypostI;


/* Thor: first ypost pos in ypost_xo.key */

static int comebackPos;

/* Thor: first xpost pos in xpost_xo.key */

static char xypostKeyword[30];

#if 0
/* Thor.980911: �@�� post_body() �Y�i*/
    static int
xpost_body(
    XO *xo)
{
    const HDR *fhdr;
    int num, max, tail;

    max = xo->max;
#if 0
    if (max <= 0)
    { /* Thor.980911: ����: �H���U�@�� */
        vmsg(MSG_XY_NONE);
        return XO_QUIT;
    }
#endif

    num = xo->top;
    fhdr = (const HDR *) xo_pool_base + num;
    tail = num + XO_TALL;
    max = BMIN(max, tail);

    move(3, 0);
    do
    {
        post_item(++num, fhdr++);
    } while (num < max);

    clrtobot();
    return XO_NONE;
}

#endif  /* #if 0 */

    static int
xpost_head(
    XO *xo)
{
    vs_head("�D�D��C" /* currBM */, (const char *) xo->xyz);
    outs(MSG_XYPOST);
    if (*xypostKeyword)
        outs(xypostKeyword);

    //r2.20181219: this part may need to be refined
    prints(NECK_XPOST, d_cols, "");

    /* return xpost_body(xo); */
    return post_body(xo); /* Thor.980911: �@�ΧY�i */
}


    static void
xypost_pick(
    XO *xo)
{
    int *xyp, fsize, pos, max, top;
    bool scrl_up;
    HDR *fimage, *hdr;

    fimage = (HDR *) f_map(xo->dir, &fsize);
    if (fimage == (HDR *) - 1)
        return;

    xyp = xypostI;

    pos = xo->pos;
    top = xo->top;
    scrl_up = (pos < top);
    xo->top = BMAX(top + ((pos - top + scrl_up) / XO_TALL - scrl_up) * XO_TALL, 0);
    hdr = (HDR *) xo_pool_base;
    top = 0;
    max = xo->max;

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
    return post_body(xo); /* Thor.980911: �@�ΧY�i */
}


    static int
xpost_help(
    XO *xo)
{
    film_out(FILM_BOARD, -1);
    return XO_HEAD;
}


/* Thor.0509: �n�Q��k�T�� ctrl('D') */


    static int
xpost_browse(
    XO *xo)
{
    HDR *hdr;
    int cmd, chrono, xmode;
    char *dir, fpath[64], *board GCC_UNUSED;

    int key;

    cmd = XO_NONE;
    dir = xo->dir;
    board=currboard;

    for (;;)
    {
        hdr = (HDR *) xo_pool_base + xo->pos;
        xmode = hdr->xmode;
        if (xmode & (POST_CANCEL | POST_DELETE | POST_MDELETE))
            break;

#ifdef  HAVE_USER_MODIFY
        if (xmode & POST_CURMODIFY)
        {
            vmsg("���峹���b�ק襤!!");
            break;
        }
#endif

        if ((hdr->xmode & POST_LOCK) && !(HAS_PERM(PERM_SYSOP | PERM_BOARD) || (bbstate & STAT_BOARD) || !strcmp(hdr->owner, cuser.userid)))
            break;

        /* cache.20130407: for preventing bot to get data */
        char desc[128];
        time_t now;
        time(&now);

        snprintf(desc, sizeof(desc), "%s %s %s %lld %s\n", Atime(&now), cuser.userid, currboard, (long long)hdr->chrono, ipv6addr);
        f_cat(FN_BROWSE_LOG, desc);

        hdr_fpath(fpath, dir, hdr);

        /* Thor.990204: ���Ҽ{more �Ǧ^�� */
        //    if ((key = more(fpath, MSG_POST)) == -1)
        //      break;

        comebackPos = hdr->xid;
        /* Thor.980911: �q�걵�Ҧ��^�Ӯɭn�^��ݹL�����g�峹��m */

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

        if (!(hdr->xmode & POST_LOCK) || HAS_PERM(PERM_SYSOP))
            strcpy(currtitle, str_ttl(hdr->title));
        else
            currtitle[0] = '\0';

        // Thor.990204: ���Ҽ{more �Ǧ^��
        if ((key = more(fpath, FOOTER_POST)) < 0)
            break;

        /* Thor.990204: ���Ҽ{more �Ǧ^�� */
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
            case 'F':/*float.101109: �ץ����i��H*/
                if ((hdr->xmode & POST_LOCK) && !HAS_PREM(PERM_SYSOP))
                {
                    vmsg("��w�峹������H");
                    return XO_NONE;
                }
                else
                {
                    return xo_forward(xo);
                }
                break;
#endif
            case ']':  /* Thor.990204: ���ɷQ��]�ݫ᭱���峹 */
            case 'j':  /* Thor.990204: ���ɷQ��j�ݫ᭱���峹 */
            case ' ':
                {
                    int top = xo->top;
                    int pos = xo->pos + 1;

                    /* Thor.980727: �ץ��ݹL�Y��bug */

                    if (pos >= xo->max)
                        return cmd;

                    xo->pos = pos;

                    if (pos >= xo->top + XO_TALL)
                        xo->top = BMAX(top + ((pos - top) / XO_TALL) * XO_TALL, 0);

                    continue;
                }

            case 'y':
            case 'r':
                if (bbstate & STAT_POST)
                {
                    strcpy(quote_file, fpath);
                    if (do_reply(hdr) == XO_INIT)       /* �����\�a post �X�h�F */
                        return XO_INIT;
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
                return XO_INIT;
#endif

        }
        break;
    }

    return XO_INIT;
}


static KeyFuncList xpost_cb =
{
    {XO_INIT, {xpost_init}},
    {XO_LOAD, {xpost_load}},
    {XO_HEAD, {xpost_head}},
#if 0
    {XO_BODY, {xpost_body}},
#endif
    {XO_BODY, {post_body}}, /* Thor.980911: �@�ΧY�i */

    {'r', {xpost_browse}},
    {'y', {post_reply}},
    {'t', {post_tag}},
    {'m', {post_mark}},

    {'d', {post_delete}},  /* Thor.980911: ��K���D*/

    {Ctrl('P'), {post_add}},
    {Ctrl('Q'), {xo_uquery}},
    {'q', {post_spam}},
    {'I', {xo_usetup}},
#ifdef HAVE_MULTI_CROSSPOST
    {Ctrl('X'), {post_xcross}},
#endif
    {'x', {post_cross}},

    {'h', {xpost_help}}
};
#endif  /* #ifdef XZ_XPOST */


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

    if ((max = xo->max) <= 0) /* Thor.980911: ����: �H���U�@ */
        return XO_FOOT;

    if (xz[XZ_XPOST - XO_ZONE].xo)
    {
        vmsg("�A�w�g�ϥΤF�걵�Ҧ�");
        return XO_FOOT;
    }

    /* input condition */
    /* 090928.cache: �����i�J�걵�Ҧ� */
    //  mode = vans("�� 0)�걵 1)�s�峹 2)LocalPost [0]�G") - '0';
    //  if (mode > 2 || mode < 0)
    mode = 0;

    if (!mode)
    {
        key = xypostKeyword;
        filter_title = vget(B_LINES_REF, 0, MSG_XYPOST, key, sizeof(xypostKeyword), GCARRY);
        str_lower(buf, key);
        key = buf;

        if ((filter_author = vget(B_LINES_REF, 0, "[�걵�Ҧ�]�@�̡G", author, 30, DOECHO)))
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
        vmsg("�ثe�L�k�}�ү�����");
        return XO_FOOT;
    }

    free(xypostI); /* Thor.980911: ����: �ȭ��жi�J��, ���O�O���� */

    /* allocate index memory, remember free first */

    /* Thor.990113: �Ȱ�title, author�������S���Hpost */
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
            continue;                   /* Thor.0701: ���L�ݤ��쪺�峹 */

        if ((head->xmode & POST_LOCK) && !(HAS_PERM(PERM_SYSOP| PERM_BOARD)||bbstate & STAT_BOARD))
            continue;

        /* check author */

        /* Thor.981109: �S�O�`�N, author�O�q�Ymatch, ���Osubstr match, �����Cload */
        if (!mode)
        {
            if (filter_author && str_ncasecmp(head->owner, author, filter_author))
                continue;

            /* check condition */

            title = str_ttl(head->title); /* Thor.980911: ���� Re: ���~ */

            if (*key && !str_casestr(title, key))
                continue;
        }
        else if (mode == 1)
        {
            title = head->title;
            if (!strncmp(title, STR_REPLY, sizeof(STR_REPLY) - 1))
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

    free(xz[XZ_XPOST - XO_ZONE].xo);

    comebackPos = xo->pos;      /* Thor: record pos, future use */
    xz[XZ_XPOST - XO_ZONE].xo = xt = xo_new(xo->dir);
    xt->cb = xpost_cb;
    xt->recsiz = sizeof(HDR);
    xt->pos = 0;
    xt->max = sum;
    xt->xyz = xo->xyz;
    xt->key = XZ_XPOST;

    xover(XZ_XPOST);

    /* set xo->pos for new location */

    xo->pos = comebackPos;

    /* free xpost_xo */

    free(xz[XZ_XPOST - XO_ZONE].xo);
    xz[XZ_XPOST - XO_ZONE].xo = NULL;

    /* free index memory, remember check free pointer */

    free(xypostI);
    xypostI = NULL;

    return XO_INIT;
}
