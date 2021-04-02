/*-------------------------------------------------------*/
/* gem.c        ( NTHU CS MapleBBS Ver 3.00 )            */
/*-------------------------------------------------------*/
/* target : ��ذϾ\Ū�B�s��                             */
/* create : 95/03/29                                     */
/* update : 2000/01/02                                   */
/*-------------------------------------------------------*/

#include "bbs.h"

/* definitions of Gem Mode */


#define GM_WORKING      0x01    /* ��ذϬI�u�� */
#define GM_DELETE       0x02    /* ��ذϤw�R�� */


#define GEM_CUT         1
#define GEM_COPY        2


#ifndef INADDR_NONE
#define INADDR_NONE     0xffffffff
#endif


#define GEM_WAY         3
static int gem_way;

static int GemBufferNum; /* Thor.990414: ���e�ŧi, �Ω�gem_head */

static int gem_add(XO *xo);
static int gem_paste(XO *xo);
static int gem_anchor(XO *xo);
static int gem_recycle(XO *xo);

static int
gem_manage(
    const char* title)
{
    const char *const l_square = strrchr(title, '[');
    if (!l_square)
        return 0;

    const char *const list = l_square + 1;
    const char *list_end = strrchr(title, ']');
    if (!list_end)
        list_end = strchr(title, '\0');

    char buf[100];
    str_scpy(buf, list, list_end - list + 1);

    return str_has(buf, cuser.userid);
}

static int
gem_foot(
    XO *xo)
{
    outf(MSG_GEM);
    return XO_NONE;
}

static void
gem_item(
    int num,
    const HDR *ghdr)
{
    int xmode, gtype;
    char fpath[64];

    /* ������������������ : A1B7 ... */

    xmode = ghdr->xmode;
    gtype = (char) 0xba;
    if (xmode & GEM_FOLDER)
        gtype += 1;
    prints("%6d%c%c\241%c ", num, (xmode & GEM_RESTRICT) ? ')' : (xmode & GEM_LOCK) ? 'L' :  ' ',
        TagNum && !Tagger(ghdr->chrono, num - 1, TAG_NIN) ? '*' : ' ', gtype);

    /* Thor.0724: �s�P recno �@�_���, �]���bcopy, paste��|��chrono�@�˪� */
    /* tag_char(ghdr->chrono), gtype); */

    gtype = gem_way;

    if (!(bbstate & STAT_BOARD)&&!HAS_PERM(PERM_ADMIN|PERM_GEM)&&(xmode & GEM_RESTRICT))
        prints("\x1b[1;33m��ƫO�K�I\x1b[m\n");
    else if (!HAS_PERM(PERM_SYSOP) && (xmode & GEM_LOCK))
        prints("\x1b[1;33m��ƫO�K�I\x1b[m\n");
    else
    {
        if (xmode & GEM_BOARD)
        {
            sprintf(fpath, "gem/brd/%s/", ghdr->xname);
            prints("%-*.*s%-13s%s\n", d_cols + 47, d_cols + 46, ghdr->title, (gtype == 1 ? ghdr->xname : ghdr->owner), access(fpath, R_OK) ? "[deleted]" : ghdr->date);
        }
        else
            prints("%-*.*s%-13s%s\n", d_cols + 47, d_cols + 46, ghdr->title, (gtype == 1 ? ghdr->xname : ghdr->owner), ghdr->date);
    }
}

static int
gem_cur(
    XO *xo)
{
    const HDR *const ghdr = (const HDR *) xo_pool_base + xo->pos;
    move(3 + xo->pos - xo->top, 0);
    gem_item(xo->pos + 1, ghdr);
    return XO_NONE;
}


static int
gem_body(
    XO *xo)
{
    const HDR *ghdr;
    int num, max, tail;

    max = xo->max;
    if (max <= 0)
    {
        outs("\n\n�m��ذϡn�|�b�l���Ѧa��������� :)");
        if (xo->key >= GEM_LMANAGER)
        {
            max = vans("(A)�s�W��� (G)����\\�� (W)�귽�^���� [N]�L�Ҩƨ� ");
            switch (max)
            {
                case 'a':
                    max = gem_add(xo);
                    if (xo->max > 0)
                        return max;
                    break;
                case 'g':
                    gem_anchor(xo);
                    break;
                case 'w':
                    gem_recycle(xo);
                    break;
            }

        }
        else
        {
            vmsg(NULL);
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
        gem_item(++num, ghdr++);
    } while (num < max);
    clrtobot();

    return gem_foot(xo);
}


static int
gem_head(
    XO *xo)
{
    char buf[20];

    vs_head("��ؤ峹", (const char *) xo->xyz);

    if (xo->key > GEM_USER && GemBufferNum > 0)
    {
        sprintf(buf, "(�ŶK�� %d �g)\n", GemBufferNum);
    }
    else
    {
        buf[0] = '\n';
        buf[1] = '\0';
    }

    outs(NECKGEM1);
    outs(buf);
    prints(NECKGEM2, d_cols, "");
    return gem_body(xo);
}


static int
gem_toggle(
    XO *xo)
{
    gem_way = (gem_way + 1) % GEM_WAY;
    if (!HAS_PERM(PERM_SYSOP) && (gem_way ==1))
        gem_way = 2;
    return XO_BODY;
}


static int
gem_init(
    XO *xo)
{
    xo_load(xo, sizeof(HDR));
    return gem_head(xo);
}


static int
gem_load(
    XO *xo)
{
    xo_load(xo, sizeof(HDR));
    return gem_body(xo);
}

/* ----------------------------------------------------- */
/* gem_check : attribute / permission check out          */
/* ----------------------------------------------------- */


#define GEM_READ        1       /* readable */
#define GEM_WRITE       2       /* writable */
#define GEM_FILE        4       /* �w���O�ɮ� */


static HDR *
gem_check(
    XO *xo,
    char *fpath,
    int op)
{
    HDR *ghdr;
    int gtype, level;
    char *folder;

    level = xo->key;
    if ((op & GEM_WRITE) && (level <= GEM_USER))
        return NULL;

    ghdr = (HDR *) xo_pool_base + xo->pos;
    gtype = ghdr->xmode;

//  if ((gtype & GEM_RESTRICT) && (level <= GEM_USER))
//      return NULL;

    if ((gtype & GEM_RESTRICT) && (level <= GEM_USER) && !gem_manage(ghdr->title))
        return NULL;

    if ((gtype & GEM_LOCK) && (!HAS_PERM(PERM_SYSOP)))
        return NULL;

    if ((op & GEM_FILE) && (gtype & GEM_FOLDER))
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
            hdr_fpath(fpath, folder, ghdr);
        }
    }
    return ghdr;
}


/* ----------------------------------------------------- */
/* ��Ƥ��s�W�Gappend / insert                           */
/* ----------------------------------------------------- */


void
brd2gem(
    const BRD *brd,
    HDR *gem)
{
    memset(gem, 0, sizeof(HDR));
    time(&gem->chrono);
    strcpy(gem->xname, brd->brdname);
    sprintf(gem->title, "%-16s%s", brd->brdname, brd->title);
    gem->xmode = GEM_BOARD | GEM_FOLDER;
}


static void
gem_log(
    const char *folder,
    const char *action,
    const HDR *hdr)
{
    char fpath[80], buf[256];

    if (hdr->xmode & (GEM_RESTRICT | GEM_RESERVED | GEM_LOCK))
        return;

    setdirpath_root(fpath, folder, "@/@log");
    sprintf(buf, "[%s] %s (%s) %s\n%s\n\n",
        action, hdr->xname, Now(), cuser.userid, hdr->title);
    f_cat(fpath, buf);
}


static int
gem_add(
    XO *xo)
{
    int gtype, level, fd, ans;
    char title[80], fpath[80], *dir;
    HDR ghdr;

    level = xo->key;
    if (level < GEM_LMANAGER)   /* [�^����] ������s�W */
        return XO_NONE;

    gtype = vans(level == GEM_SYSOP ?
        "�s�W A)rticle B)oard C)lass D)ata F)older P)aste Q)uit [Q] " :
        "�s�W (A)�峹 (F)���v (P)�K�� (Q)�����H[Q] ");

    if (gtype == 'p')
        return gem_paste(xo);

    if (gtype != 'a' && gtype != 'f' &&
        (level != GEM_SYSOP || (gtype != 'b' && gtype != 'c' && gtype != 'd')))
        return XO_FOOT;

    dir = xo->dir;
    fd = -1;
    memset(&ghdr, 0, sizeof(HDR));

    if (gtype == 'b')
    {
        BRD *brd;

        if (!(brd = ask_board(fpath, BRD_R_BIT, NULL)))
            return XO_HEAD;

        brd2gem(brd, &ghdr);
        gtype = 0;
    }
    else
    {
        if (!vget(B_LINES_REF, 0, "���D�G", title, 64, DOECHO))
            return XO_FOOT;

        if (gtype == 'c' || gtype == 'd')
        {
            if (!vget(B_LINES_REF, 0, "�ɦW�G", fpath, (gtype == 'c') ? IDLEN :IDLEN + 1, DOECHO))
                return XO_FOOT;

            if (strchr(fpath, '/'))
            {
                zmsg("���X�k���ɮצW��");
                return XO_FOOT;
            }

            time(&ghdr.chrono);
            sprintf(ghdr.xname, "@%s", fpath);
            if (gtype == 'c')
            {
                strcat(fpath, "/");
                sprintf(ghdr.title, "%-13s�i %s �j", fpath, title);
                ghdr.xmode = GEM_FOLDER;
            }
            else
            {
                strcpy(ghdr.title, title);
                ghdr.xmode = 0;
            }
            gtype = 1;
        }
        else
        {
            fd = hdr_stamp(dir, gtype, &ghdr, fpath);
            if (fd < 0)
                return XO_FOOT;
            close(fd);

            if (gtype == 'a')
            { /* Thor.981020: �`�N�Qtalk�����D */
                if (bbsothermode & OTHERSTAT_EDITING)
                {
                    vmsg("�A�٦��ɮ��٨S�s���@�I");
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
        }
    }

    ans = vans("�s���m A)ppend I)nsert N)ext Q)uit [A] ");

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

    gem_log(dir, "�s�W", &ghdr);

    return (gtype ? XO_LOAD : XO_INIT);
}


/* ----------------------------------------------------- */
/* ��Ƥ��ק�Gedit / title                              */
/* ----------------------------------------------------- */


static int
gem_edit(
    XO *xo)
{
    char fpath[80];
    HDR *hdr;

    if (bbsothermode & OTHERSTAT_EDITING)
    {
        vmsg("�A�٦��ɮ��٨S�s���@�I");
        return XO_FOOT;
    }

    if (!(hdr = gem_check(xo, fpath, GEM_WRITE | GEM_FILE)))
        return XO_NONE;
    if ((hdr->xmode & GEM_RESERVED) && (xo->key < GEM_SYSOP))
        return XO_NONE;
    if (vedit(fpath, false) >= 0) /* Thor.981020: �`�N�Qtalk�����D */
        gem_log(xo->dir, "�ק�", hdr);
    return XO_HEAD;
}


static int
gem_title(
    XO *xo)
{
    HDR *ghdr, xhdr;
    int num;
    char *dir;

    ghdr = gem_check(xo, NULL, GEM_WRITE);
    if (ghdr == NULL)
        return XO_NONE;

    xhdr = *ghdr;
    vget(B_LINES_REF, 0, "���D�G", xhdr.title, TTLEN + 1, GCARRY);

    dir = xo->dir;
    if (HAS_PERM(PERM_ALLBOARD|PERM_GEM))
    {
        vget(B_LINES_REF, 0, "�s�̡G", xhdr.owner, IDLEN + 2, GCARRY);
        vget(B_LINES_REF, 0, "�ɶ��G", xhdr.date, 9, GCARRY);
    }

    if (memcmp(ghdr, &xhdr, sizeof(HDR)) &&
        vans("�T�w�n�ק��(y/N)�H[N]") == 'y')
    {
        *ghdr = xhdr;
        num = xo->pos;
        rec_put(dir, ghdr, sizeof(HDR), num);

        gem_log(xo->dir, "���D", ghdr);
        return XR_FOOT + XO_CUR;
    }
    return XO_FOOT;
}


static int
gem_lock(
    XO *xo)
{
    HDR *ghdr;
    int num;

    if (!HAS_PERM(PERM_SYSOP))
        return XO_NONE;

    if ((ghdr = gem_check(xo, NULL, GEM_WRITE)))
    {
        ghdr->xmode ^= GEM_LOCK;

        num = xo->pos;
        rec_put(xo->dir, ghdr, sizeof(HDR), num);
        return XO_CUR;
    }

    return XO_NONE;
}


static int
gem_mark(
    XO *xo)
{
    HDR *ghdr;
    int num;

    if ((ghdr = gem_check(xo, NULL, GEM_WRITE)))
    {
        ghdr->xmode ^= GEM_RESTRICT;

        num = xo->pos;
        rec_put(xo->dir, ghdr, sizeof(HDR), num);
        return XO_CUR;
    }

    return XO_NONE;
}


static int
gem_state(
    XO *xo)
{
    HDR *ghdr;
    char *dir, fpath[80], site[64] GCC_UNUSED, path[512] GCC_UNUSED, *str;
    struct stat st;
    int bno;

    /* Thor.990107: Ernie patch:
      gem.c gem_browse() �b�i�J ��)�� folder �ɤ@�� op = GEM_VISIT
      �ϱo�O�D�u��b gopher �̥~�h�[���ɮ��ݩʤ� update proxy�A�i�J gopher
      �K���ġC

      �ѨM��k: �i gem_state() �ɦh�P�_�O�_���ӪO�O�D�A����n���覡�Ы��� :)
    */
    if (!(bbstate & STAT_BOARD) && xo->key <= GEM_USER && !(HAS_PERM(PERM_SYSOP)))
        return XO_NONE;


    if (!(ghdr = gem_check(xo, fpath, GEM_READ)))
        return XO_NONE;
    /* Thor.980216: �`�N! ���i��Ǧ^ NULL�ɭP��H */

    if (!(HAS_PERM(PERM_ALLBOARD)))
    {
        if (!str_ncasecmp(fpath, "gem/brd/", 8))
        {
            dir = fpath + 8;
            if ((str = strchr(dir, '/')))
            {
                *str = '\0';
                bno = brd_bno(dir);
                *str = '/';
                if (bno < 0 || !(brd_bits[bno] & BRD_X_BIT))
                    return XO_NONE;
            }
            else
                return XO_NONE;
        }
        else
            return XO_NONE;
    }

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
/* ��Ƥ��s���Gedit / title                              */
/* ----------------------------------------------------- */


static int
gem_browse(
    XO *xo)
{
    HDR *ghdr;
    int op, xmode;
    char fpath[80], title[TTLEN + 1], *ptr;

    op = GEM_READ;

    do
    {
        ghdr = gem_check(xo, fpath, op);
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
                if (HAS_PERM(PERM_SYSOP|PERM_BOARD|PERM_GEM)) /* visor.991119: ���٬O���ݪ��`�� GEM_SYSOP */
                    op = GEM_SYSOP;
                else if ((op >= 0) && (brd_bits[op] & BRD_X_BIT))
                    op = GEM_MANAGER;
                else if ( ( ptr = strrchr(ghdr->title, '[') ) )
                    op = GEM_MANAGER;
                else
                    op = GEM_USER;
            }
            else if (xmode & HDR_URL)
            {
                op = GEM_VISIT;
            }

            strcpy(title, ghdr->title);

            if (gem_manage(title))
                op = GEM_LMANAGER;

            XoGem(fpath, title, op);
            return XO_INIT;
        }

        /* browse article */

        /* Thor.990204: ���Ҽ{more �Ǧ^�� */
        if ((xmode = more(fpath, MSG_GEM)) == -2)
            return XO_INIT;
        if (xmode == -1)
            break;

        op = GEM_READ | GEM_FILE;

        xmode = xo_getch(xo, xmode);

    } while (xmode == XO_BODY);

    if (op != GEM_READ)
        return XO_HEAD;
    return XO_NONE;
}


/* ----------------------------------------------------- */
/* ��ذϤ��R���G copy / cut (delete) paste / move       */
/* ----------------------------------------------------- */


static char GemFolder[80], GemAnchor[80], GemSailor[24];
static HDR *GemBuffer;
static int GemBufferSiz; /*, GemBufferNum; */
                         /* Thor.990414: ���e�ŧi��gem_head�� */


/* �t�m�������Ŷ���J header */


static HDR *
gbuf_malloc(
    int num)
{
    HDR *gbuf;

    GemBufferNum = num;
    if ((gbuf = GemBuffer))
    {
        if (GemBufferSiz < num)
        {
            num += (num >> 1);
            GemBufferSiz = num;
            GemBuffer = gbuf = (HDR *) realloc(gbuf, sizeof(HDR) * num);
        }
    }
    else
    {
        GemBufferSiz = num;
        GemBuffer = gbuf = (HDR *) malloc(sizeof(HDR) * num);
    }

    return gbuf;
}


static void
gem_buffer(
    char *dir,
    HDR *ghdr)                  /* NULL �N���J TagList, �_�h�N�ǤJ����J */
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

    gbuf = gbuf_malloc(num);

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
        } while (++locus < num);
    }

    strcpy(GemFolder, dir);
}


static inline void
gem_store(void)
{
    int num;
    char *folder;

    num = GemBufferNum;
    if (num <= 0)
        return;

    folder = GemFolder;
    setdirpath_root(folder, folder, FN_GEM);

    rec_add(folder, GemBuffer, sizeof(HDR) * num);
}


static int
gem_delete(
    XO *xo)
{
    HDR *ghdr;
    char *dir, buf[80];
    int tag;

    if (!(ghdr = gem_check(xo, NULL, GEM_WRITE)))
        return XO_NONE;

    tag = AskTag("��ذϧR��");

    if (tag < 0)
        return XO_FOOT;

    if (tag > 0)
    {
        sprintf(buf, "�T�w�n�R�� %d �g���Һ�ض�(y/N)�H[N] ", tag);
        if (vans(buf) != 'y')
            return XO_FOOT;
    }

    dir = xo->dir;

    gem_buffer(dir, tag ? NULL : ghdr);

    if (xo->key > GEM_RECYCLE &&
        vans("�O�_��i�귽�^����(y/N)�H[N] ") == 'y')
        gem_store();

    /* �u�R�� HDR �ä��R���ɮ� */

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
            else
            {
                gem_log(dir, "�R��", ghdr);
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
        const HDR ghdr_orig = *ghdr;
        currchrono = ghdr->chrono;
        rec_del(dir, sizeof(HDR), xo->pos, cmpchrono, NULL);
        gem_log(dir, "�R��", &ghdr_orig);
    }

    /* return XO_LOAD; */
    return XO_INIT; /* Thor.990414: ���F�N�ŶKï�g�Ƥ@�֤��� */
}


static int
gem_copy(
    XO *xo)
{
    HDR *ghdr;
    int tag;

    ghdr = gem_check(xo, NULL, GEM_WRITE);
    if (ghdr == NULL)
        return XO_NONE;

    tag = AskTag("��ذϫ���");

    if (tag < 0)
        return XO_FOOT;

    gem_buffer(xo->dir, tag ? NULL : ghdr);

    zmsg("��������");
    /* return XO_FOOT; */
    return XO_HEAD; /* Thor.990414: ���ŶK�g�Ƨ�s */
}


static inline int
gem_extend(
    XO *xo,
    int num)
{
    char *dir, fpath[80], gpath[80];
    FILE *fp;
    time_t chrono;
    HDR *hdr;

    if (!(hdr = gem_check(xo, fpath, GEM_WRITE | GEM_FILE)))
        return -1;

    if (!(fp = fopen(fpath, "a")))
        return -1;

    dir = xo->dir;
    chrono = hdr->chrono;

    for (hdr = GemBuffer; num--; hdr++)
    {
        if ((hdr->chrono != chrono) && !(hdr->xmode & GEM_FOLDER))
        {
            hdr_fpath(gpath, dir, hdr); /* Thor: ���] hdr�M xo->dir�O�P�@�ؿ� */
            fputs(STR_LINE, fp);
            f_suck(fp, gpath);
        }
    }

    fclose(fp);
    return 0;
}


static int
gem_paste(
    XO *xo)
{
    int num, ans;
    char *dir, srcDir[80], dstDir[80];

    /* �O�_���� cycle ? */

    if (xo->key <= GEM_USER)
        return XO_NONE;

    if (!(num = GemBufferNum))
    {
        zmsg("�Х����� copy �R�O��A paste");
        return XO_FOOT;
    }

    setdirpath_root(srcDir, GemFolder, FN_GEM);
    setdirpath_root(dstDir, dir = xo->dir, FN_GEM);

    if (strcmp(srcDir, dstDir))
    {
        zmsg("�ثe���䴩[��ϫ���]");
        return XO_FOOT;
    }

    switch (ans = vans("�s���m A)ppend I)nsert N)ext E)xtend Q)uit [A] "))
    {
    case 'q':
        return XO_FOOT;

    case 'e':
        if (gem_extend(xo, num))
        {
            zmsg("[Extend �ɮת��[] �ʧ@�å��������\\");
            return XO_FOOT;
        }
        /* Thor.990105: ���\�h�M���T�� */
        return XO_FOOT;

    case 'i':
    case 'n':
        rec_ins(dir, GemBuffer, sizeof(HDR), xo->pos + (ans == 'n'), num);
        break;

    default:
        rec_add(dir, GemBuffer, sizeof(HDR) * num);
    }

    return XO_LOAD;
}


static int
gem_move(
    XO *xo)
{
    HDR *ghdr;
    char *dir, buf[80];
    int pos, newOrder;

    ghdr = gem_check(xo, NULL, GEM_WRITE);

    if (ghdr == NULL)
        return XO_NONE;

    pos = xo->pos;
    sprintf(buf + 5, "�п�J�� %d �ﶵ���s��m�G", pos + 1);
    if (!vget(B_LINES_REF, 0, buf + 5, buf, 5, DOECHO))
        return XO_FOOT;

    newOrder = TCLAMP(atoi(buf) - 1, 0, xo->max - 1);

    if (newOrder != pos)
    {

#if 0
        if (!rec_mov(xo->dir, sizeof(HDR), pos, newOrder))
            return XO_LOAD;
#else
        const HDR ghdr_orig = *ghdr;
        dir = xo->dir;
        if (!rec_del(dir, sizeof(HDR), pos, NULL, NULL))
        {
            rec_ins(dir, &ghdr_orig, sizeof(HDR), newOrder, 1);
            xo->pos = newOrder;
            return XO_LOAD;
        }
#endif
    }
    return XO_FOOT;
}


static int
gem_recycle(
    XO *xo)
{
    int level;
    char fpath[64];

    level = xo->key;

    if (level <= GEM_USER)
        return XO_NONE;

    if (level == GEM_LMANAGER)
    {
        vmsg("�p�O�D����i�J�^�����I");
        return XO_FOOT;
    }

    if (level == GEM_RECYCLE)
    {
        if (vans("�T�w�n�M�z�귽�^������[y/N]?(N) ") == 'y')
        {
            unlink(xo->dir);
            return XO_QUIT;
        }
        return XO_FOOT;
    }

    setdirpath_root(fpath, xo->dir, FN_GEM);
    if (rec_num(fpath, sizeof(HDR)) <= 0)
    {
        zmsg("�귽�^�����õL���");
        return XO_FOOT;
    }

    XoGem(fpath, "�� �귽�^���� ��", GEM_RECYCLE);
    return XO_INIT;
}


static int
gem_anchor(
    XO *xo)
{
    int ans;
    char *folder;

    /* Thor.981020: �W�w�귽�^�������o�w��, �t, �����, g�h�|��J�^���� */
    if (xo->key < GEM_LMANAGER)  /* Thor.981020: �u�n���D�H�W�Y�i�ϥ�anchor */
        return XO_NONE;
    /* Thor.981020: ���}��@��user�ϥάO���F����D�եX�t�@�Ӥpbug:P */

    ans = vans("��ذ� A)�w�� D)���� J)�N�� Q)���� [J] ");
    if (ans != 'q')
    {
        folder = GemAnchor;

        if (ans == 'a')
        {
            strcpy(folder, xo->dir);
            str_scpy(GemSailor, (const char *) xo->xyz, sizeof(GemSailor));
        }
        else if (ans == 'd')
        {
            *folder = '\0';
        }
        else
        {
            if (!*folder)
                return gem_recycle(xo);

            XoGem(folder, "�� ��ةw��� ��", xo->key);
            return XO_INIT;
        }

        zmsg("��ʧ@����");
    }

    return XO_NONE;
}


int
gem_gather(
    XO *xo)
{
    HDR *hdr, *gbuf, ghdr, xhdr;
    int tag, locus, rc, xmode, anchor, mode;
    char *dir, *folder, fpath[80], buf[80];
    const char *msg;
    FILE *fp, *fd;

    folder = GemAnchor;
    if ((anchor = *folder))
    {
        sprintf(buf, "�����ܺ�ةw��� (%s)", GemSailor);
        msg = buf;
    }
    else
    {
        msg = "�����ܺ�ئ^����";
    }
    tag = AskTag(msg);

    if (tag < 0)
        return XO_FOOT;

    if (!anchor)
    {
        sprintf(folder, "gem/brd/%s/%s", currboard, FN_GEM);
    }

    fd = fp = NULL;

    if (vans("���[�@�� ID [y/N] ") == 'y')
        mode = 1;
    else
        mode = 0;

    dir = xo->dir;

    if (tag)
    {
        hdr = &xhdr;
        EnumTagHdr(hdr, dir, 0);
    }
    else
        hdr = (HDR *) xo_pool_base + xo->pos;

    if (hdr->xmode & POST_DELETE)
        return XO_FOOT;


    if (tag > 0)
    {
        switch (vans("��C�峹 1)�X���@�g 2)���O���� Q)���� [2] "))
        {
        case 'q':
            return XO_FOOT;

        case '1':
            if (!vget(B_LINES_REF, 0, "���D�G", xhdr.title, TTLEN + 1, GCARRY))
                return XO_FOOT;
            fp = fdopen(hdr_stamp(folder, 'A', &ghdr, fpath), "w");
            strcpy(ghdr.owner, cuser.userid);
            strcpy(ghdr.title, xhdr.title);
            break;
        default:
            break;

        }
    }

    rc = (*dir == 'b') ? XO_LOAD : XO_FOOT;

    /* gather ���P copy, �i�ǳƧ@ paste */

    strcpy(GemFolder, folder);
    gbuf = gbuf_malloc((fp != NULL || tag == 0) ? 1 : tag);

    locus = 0;

    do
    {
        if (tag)
        {
            EnumTagHdr(hdr, dir, locus);
        }

        xmode = hdr->xmode;

        /* Thor.981018: �S�O�`�N, anchor �S�k���� folder, �]�{�����n�g(���copy),
                        ���ɦP�Ϫ�folder�i�H��copy & paste,
                        ���P�ϴN�u�n�@�g�g���� */
        if (!(xmode & (GEM_FOLDER|POST_DELETE)))        /* �d hdr �O�_ plain text */
        {
            hdr_fpath(fpath, dir, hdr);

            if (fp)
            {
                f_suck(fp, fpath);
                fputs(STR_LINE, fp);
            }
            else
            {
                /*strcpy(buf, fpath);*/
                fd = fdopen(hdr_stamp(folder, 'A', &ghdr, buf), "w"); /*by visor*/
                /*hdr_stamp(folder, HDR_LINK | 'A', &ghdr, fpath);*/
                strcpy(ghdr.owner, cuser.userid);
                if (mode)
                {
                    char tmp[80], *ptr;
                    strcpy(tmp, hdr->owner);
                    ptr = strchr(tmp, '.');
                    if (ptr)
                        *ptr = '\0';
                    ptr = strchr(tmp, '@');
                    if (ptr)
                        *ptr = '\0';
                    strncpy(ghdr.title, hdr->title, sizeof(ghdr.title) - IDLEN - 4);
                    strcat(ghdr.title, " (");
                    strcat(ghdr.title, tmp);
                    strcat(ghdr.title, ")");
//                  sprintf(ghdr.title, "(%s)", tmp);
//                  strncat(ghdr.title, hdr->title, sizeof(ghdr.title) - strlen(ghdr.title) - 1);
                }
                else
                {
                    strcpy(ghdr.title, hdr->title);
                }
                f_suck(fd, fpath);
                rec_add(folder, &ghdr, sizeof(HDR));
                gem_log(folder, "�s�W", &ghdr);

                if (fd)                 /* by visor */
                    fclose(fd);

                gbuf[locus] = ghdr;     /* ��J Gembuffer */
            }

            if ((rc == XO_LOAD) && !(xmode & POST_GEM))
            {
                /* update �ݩ� */

                hdr->xmode = xmode | POST_GEM;
                rec_put(dir, hdr, sizeof(HDR), tag ? TagList[locus].recno :
                    (xo->key == XZ_POST ? xo->pos : hdr->xid));
            }
        }
    } while (++locus < tag);

    if (fp)
    {
        fclose(fp);
        gbuf[0] = ghdr;
        rec_add(folder, &ghdr, sizeof(HDR));
        gem_log(folder, "�s�W", &ghdr);
    }

    zmsg("��������");

    return rc;                  /* Thor: �M���W����ܪ��T�� */
}


static int
gem_tag(
    XO *xo)
{
    HDR *ghdr;
    int pos, tag;

    if ((ghdr = gem_check(xo, NULL, GEM_READ)) && (tag = Tagger(ghdr->chrono, pos = xo->pos, TAG_TOGGLE)))
    {
        return XO_CUR + 1;
    }

    /* return XO_NONE; */
    return XO_MOVE + XO_REL + 1; /* lkchu.981201: ���ܤU�@�� */
}


static int
gem_help(
    XO *xo)
{
    film_out(FILM_GEM, -1);
    return XO_HEAD;
}

static int
gem_cross(
    XO *xo)
{
    char xboard[20], fpath[80], xfolder[80], xtitle[80], buf[80], *dir;
    HDR *hdr, xpost, *ghdr;
#ifdef  HAVE_DETECT_CROSSPOST
    HDR bpost;
#endif
    int method=1, rc GCC_UNUSED, tag, locus, battr;
    FILE *xfp;
    int success_count = 0;

    if (!cuser.userlevel)
        return XO_NONE;

    ghdr = gem_check(xo, NULL, GEM_READ);
    tag = AskTag("��K");
    if ((tag < 0) || (tag==0 && (!ghdr || ghdr->xmode & GEM_FOLDER)))
        return XO_FOOT;


    if (ask_board(xboard, BRD_W_BIT,
            "\n\n\x1b[1;33m�ЬD��A���ݪO�A������K�W�L�T�O�C\x1b[m\n\n"))
    {
        if (*xboard == 0)
            strcpy(xboard, currboard);

        hdr = tag ? &xpost : (HDR *) xo_pool_base + xo->pos;


        if (!tag)
        {
            if (!(hdr->xmode & GEM_FOLDER) && !((hdr->xmode & (GEM_RESTRICT|GEM_RESERVED)) && (xo->key < GEM_MANAGER))
                && !((hdr->xmode & GEM_LOCK) && !HAS_PERM(PERM_SYSOP)))
            {
                sprintf(xtitle, STR_FORWARD " %.66s", hdr->title);
                if (!vget(2, 0, "���D:", xtitle, TTLEN + 1, GCARRY))
                    return XO_HEAD;
            }
            else
                return XO_HEAD;
        }

        rc = vget(2, 0, "(S)�s�� (Q)�����H[Q] ", buf, 3, LCECHO);
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
            if (!(hdr->xmode & GEM_FOLDER) && !((hdr->xmode & (GEM_RESTRICT|GEM_RESERVED)) && (xo->key < GEM_MANAGER))
                  && !((hdr->xmode & GEM_LOCK) && !HAS_PERM(PERM_SYSOP)))
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

                strcat(buf, "] ��ذ�");
                fprintf(xfp, "�� ��������� [%s\n\n", buf);

                f_suck(xfp, fpath);
                fclose(xfp);

                strcpy(xpost.owner, cuser.userid);
                strcpy(xpost.nick, cuser.username);


                strcpy(xpost.title, xtitle);

                rec_bot(xfolder, &xpost, sizeof(xpost));

#ifdef  HAVE_DETECT_CROSSPOST
                memcpy(&bpost, hdr, sizeof(HDR));
                if (checksum_find(fpath, 0, battr))
                {
                    strcpy(bpost.owner, cuser.userid);
                    add_deny(&cuser, DENY_SEL_POST|DENY_DAYS_1|DENY_MODE_POST, 0);
                    deny_log_email(cuser.vmail, (HAS_PERM(PERM_DENYSTOP)) ? -1 : cuser.deny);
                    bbstate &= ~STAT_POST;
                    cuser.userlevel &= ~PERM_POST;

                    move_post(&bpost, BRD_VIOLATELAW, -2);

                    board_main();
                }
#endif
                success_count++;
            }
        } while (locus < tag);

        if (success_count == 0)
        {
            vmsg("������ѡC");
        }
        else if (battr & BRD_NOCOUNT)
        {
            if (success_count == ((tag == 0) ? 1 : tag))
                prints("��� %d �g���\\�A�峹���C�J�����A�q�Х]�[�C", success_count);
            else
                prints("��� %d �g���\\�A%d �g���ѡA�峹���C�J�����A�q�Х]�[�C",
                    success_count, ((tag == 0) ? 1 : tag) - success_count);
        }
        else
        {
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

static KeyFuncList gem_cb =
{
    {XO_INIT, {gem_init}},
    {XO_LOAD, {gem_load}},
    {XO_HEAD, {gem_head}},
    {XO_BODY, {gem_body}},
    {XO_FOOT, {gem_foot}},
    {XO_CUR, {gem_cur}},

    {'r', {gem_browse}},

    {Ctrl('P'), {gem_add}},
    {'E', {gem_edit}},
    {'T', {gem_title}},
    {'m', {gem_mark}},
    {'x', {gem_cross}},
    {'l', {gem_lock}},

    {'d', {gem_delete}},
    {'c', {gem_copy}},

    {Ctrl('G'), {gem_anchor}},
    {Ctrl('V'), {gem_paste}},

    {'t', {gem_tag}},
    {'f', {gem_toggle}},
    {'M', {gem_move}},

    {'S', {gem_state}},

    {Ctrl('W'), {gem_recycle}},
    {Meta('W'), {gem_recycle}},


    {'h', {gem_help}}
};


void
XoGem(
    const char *folder,
    const char *title,
    int level)
{
    XO *xo, *last;

    last = xz[XZ_GEM - XO_ZONE].xo;     /* record */

    xz[XZ_GEM - XO_ZONE].xo = xo = xo_new(folder);
    xo->cb = gem_cb;
    xo->recsiz = sizeof(HDR);
    xo->pos = 0;
    xo->key = level;
    xo->xyz = (void *)title;

    xover(XZ_GEM);

    free(xo);

    xz[XZ_GEM - XO_ZONE].xo = last;     /* restore */
}


void
gem_main(void)
{
    XO *xo;

    free(xz[XZ_GEM - XO_ZONE].xo);
    xz[XZ_GEM - XO_ZONE].xo = xo = xo_new("gem/.DIR");
    xo->cb = gem_cb;
    xo->recsiz = sizeof(HDR);
    xo->pos = 0;
    xo->key = ((HAS_PERM(PERM_SYSOP|PERM_BOARD|PERM_GEM)) ? GEM_SYSOP : GEM_USER);
    xo->xyz = (void *)"";
}
