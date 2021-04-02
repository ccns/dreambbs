/*-------------------------------------------------------*/
/* util.c       ( YZU WindTopBBS Ver 3.02 )              */
/*-------------------------------------------------------*/
/* target : ���D�T�{�A�H�H�������A��s�t���ɮ�           */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/


#include "bbs.h"

#define BM_CHECK_FILE   FN_CHECKBM_LOG

typedef struct
{
    char id[IDLEN+1];
    char brd[IDLEN+1];
    int32_t check;
} BM;  /* DISKDATA(raw) */


static int
use_io(void)
{
    int mode;
    mode = vans("�ϥΥ~���{�ǶܡH [Y/n] ");
    if (mode == 'n')
        return 0;
    else
        return 1;
}

GCC_PURE static int
check_in_memory(const char *bm, const char *id)
{
    const char *i;
    for (i = bm; strlen(i); i = i + IDLEN + 1)
        if (!strcmp(i, id))
            return 0;
    return 1;
}

/* �M���S�w�ݪO�����峹 cancel */
int
m_expire(void)
{
    DL_HOLD;
    BRD *brd;
    char bname[16];

    move(22, 0);
    outs("�M���S�w�ݪO cancel ���峹�C");
    if ((brd = ask_board(bname, BRD_R_BIT, NULL)))
    {
        char buf[80];
        sprintf(buf, "expire 999 20000 20000 %s", brd->brdname);
        PROC_CMD_BG(BINARY_SUFFIX"expire", "999", "20000", "20000", brd->brdname);
        logitfile(FN_EXPIRED_LOG, cuser.userid, buf);
    }
    else
    {
        vmsg(err_bid);
    }

    return DL_RELEASE(0);
}

static void
send_to_all(const char *title, const char *fpath, const char *bm)
{
    char buf[128];
    const char *ptr;
    HDR mhdr;

    for (ptr = bm; strlen(ptr); ptr = ptr + IDLEN + 1)
    {
        usr_fpath(buf, ptr, fn_dir);
        hdr_stamp(buf, HDR_LINK, &mhdr, (char *)fpath);
        strcpy(mhdr.owner, STR_SYSOP);
        strcpy(mhdr.title, title);
        mhdr.xmode = MAIL_MULTI;
        rec_add(buf, &mhdr, sizeof(HDR));
    }
}

int
mail_to_bm(void)
{
    DL_HOLD;
    BRD *bhdr, *head, *tail;
    char *ptr, *bm;
    char fpath[256], *title, buf[128];
    FILE *fp;

    if (bbsothermode & OTHERSTAT_EDITING)
    {
        vmsg("�A�٦��ɮ��٨S�s���@�I");
        return DL_RELEASE(-1);
    }


    bm = (char *)malloc(MAXBOARD * (IDLEN + 1) * 3);
    memset(bm, 0, MAXBOARD*(IDLEN + 1)*3);
    ptr = bm;
    utmp_mode(M_SMAIL);

    head = bhdr = bshm->bcache;
    tail = bhdr + bshm->number;
    do                          /* �ܤ֦�sysop�@�� */
    {
        char *c;
        char buf[BMLEN + 1];

        strcpy(buf, head->BM);
        c = buf;
        while (1)
        {
            char *d;
            d = strchr(c, '/');
            if (*c)
            {
                if (d)
                {
                    *d++ = 0;
                    if (check_in_memory(bm, c))
                    {
                        strcpy(ptr, c);
                        ptr += IDLEN + 1;
                    }
                    c = d;
                }
                else
                {
                    if (check_in_memory(bm, c))
                    {
                        strcpy(ptr, c);
                        ptr += IDLEN + 1;
                    }
                    break;
                }
            }
            else
                break;
        }
    }
    while (++head < tail);
    strcpy(ve_title, "[�O�D�q�i]");
    title = ve_title;
    vget(1, 0, "�� �D �D�G", title, 60, GCARRY);
    sprintf(buf, "mailtobm.%lld", (long long)time(0));
    usr_fpath(fpath, cuser.userid, buf);
    if ((fp = fopen(fpath, "w")))
    {
        fprintf(fp, "�� [�O�D�q�i] �����q�i�A���H�H�G�U�O�D\n");
        fprintf(fp, "-------------------------------------------------------------------------\n");
        fclose(fp);
    }
    utmp_mode(M_SMAIL);
    curredit = EDIT_MAIL | EDIT_LIST;

    if (vedit(fpath, true) == -1)
    {
        vmsg(msg_cancel);
        free(bm);
        return DL_RELEASE(-1);
    }
    else
    {
        if (!use_io())
        {
            send_to_all(title, fpath, bm);
            unlink(fpath);
        }
        else
        {
            PROC_CMD_BG(BINARY_SUFFIX"mailtoall", "2", fpath, title);
        }
    }
    free(bm);
    return DL_RELEASE(0);
}

static void
traverse(
char *fpath,
const char *path,
const char *title)
{
    DIR *dirp;
    struct dirent *de;
    char *fname, *str;

    if (!(dirp = opendir(fpath)))
    {
        return;
    }
    for (str = fpath; *str; str++);
    *str++ = '/';

    while ((de = readdir(dirp)))
    {
        HDR mhdr;
        fname = de->d_name;
        if (fname[0] > ' ' && fname[0] != '.')
        {
            strcpy(str, fname);
            strcat(str, "/.DIR");
            hdr_stamp(fpath, HDR_LINK, &mhdr, (char *)path);
            strcpy(mhdr.owner, "SYSOP");
            strcpy(mhdr.title, title);
            mhdr.xmode = MAIL_MULTI;
            rec_add(fpath, &mhdr, sizeof(HDR));
        }
    }
    closedir(dirp);
}


static int
open_mail(
const char *path,
const char *title)
{
    int ch;
    char *fname, fpath[256];

    strcpy(fname = fpath, BBSHOME"usr/@");
    fname = (char *) strchr(fname, '@');

    for (ch = 'a'; ch <= 'z'; ch++)
    {
        fname[0] = ch;
        fname[1] = '\0';
        traverse(fpath, path, title);
    }
    return 1;
}

int
mail_to_all(void)
{
    DL_HOLD;
    char *title;
    char fpath[256];
    char buf[128];

    if (bbsothermode & OTHERSTAT_EDITING)
    {
        vmsg("�A�٦��ɮ��٨S�s���@�I");
        return DL_RELEASE(-1);
    }


    strcpy(ve_title, "[�t�γq�i]");
    title = ve_title;
    vget(1, 0, "�� �D �D�G", title, 60, GCARRY);
    sprintf(buf, "mailtoall.%lld", (long long)time(0));
    usr_fpath(fpath, cuser.userid, buf);
    utmp_mode(M_SMAIL);
    curredit = EDIT_MAIL | EDIT_LIST;
    if (vedit(fpath, true) == -1)
    {
        vmsg(msg_cancel);
        return DL_RELEASE(-1);
    }
    else
    {
        if (!use_io())
        {
            open_mail(fpath, title);
            unlink(fpath);
        }
        else
        {
            PROC_CMD_BG(BINARY_SUFFIX"mailtoall", "1", fpath, title);
        }
    }
    return DL_RELEASE(0);
}

GCC_PURE static bool
is_bms(
const char *list,             /* �O�D�GBM list */
const char *userid)
{
    return str_has(list, userid);
}

GCC_PURE static inline bool
is_bm(
const char *list)             /* �O�D�GBM list */
{
    return str_has(list, cuser.userid);
}


int
bm_check(void)
{
    DL_HOLD;
    BRD *bhdr, *head, *tail;
    BM *bm, *ptr;
    char fpath[80], ans;
    ACCT acct;
    int fd;


    strcpy(fpath, BM_CHECK_FILE);
    move(22, 0);
    outs("�C�b�~�T�{���D�O�_�~��s���A�i��}�ǫ��g����C");
    ans = vans("�n�����D�T�{�� y)�T�w r)�_�� s)���� d)�R�� q)���} [q]:");
    if (ans == 'r')
    {
        BM tmp;
        int pos = 0;

        fd = open(fpath, O_RDONLY);
        while (fd >= 0)
        {
            lseek(fd, (off_t)(sizeof(BM) * pos), SEEK_SET);
            if (read(fd, &tmp, sizeof(BM)) == sizeof(BM))
            {
                head = bshm->bcache + brd_bno(tmp.brd);
                if (*(head->BM) && !is_bms(head->BM, tmp.id))
                {
                    strcat(head->BM, "/");
                    strcat(head->BM, tmp.id);
                }
                else
                {
                    strcpy(head->BM, tmp.id);
                }
                pos++;
            }
            else
            {
                close(fd);
                break;
            }
        }
        unlink(fpath);
        return DL_RELEASE(0);
    }
    else if (ans == 's')
    {
        BM tmp;
        int pos = 0;

        fd = open(fpath, O_RDWR);
        while (fd >= 0)
        {
            lseek(fd, (off_t)(sizeof(BM) * pos), SEEK_SET);
            if (read(fd, &tmp, sizeof(BM)) == sizeof(BM))
            {
                lseek(fd, (off_t)(sizeof(BM) * pos), SEEK_SET);
                tmp.check = 0;
                write(fd, &tmp, sizeof(BM));
                pos++;
            }
            else
            {
                close(fd);
                break;
            }
        }
        return DL_RELEASE(0);
    }
    else if (ans == 'd')
    {
        unlink(fpath);
        return DL_RELEASE(0);
    }
    else if (ans != 'y')
    {
        return DL_RELEASE(0);
    }
    if (!access(fpath, 0))
    {
        vmsg("���b�T�{���I");
        return DL_RELEASE(0);
    }

    bm = (BM *)malloc(sizeof(BM) * MAXBOARD * 3);
    memset(bm, 0, sizeof(BM)*MAXBOARD*3);
    ptr = bm;
    utmp_mode(M_SMAIL);

    head = bhdr = bshm->bcache;
    tail = bhdr + bshm->number;
    do                          /* �ܤ֦�sysop�@�� */
    {
        char *c;
        char buf[BMLEN + 1];

        strcpy(buf, head->BM);
        c = buf;
        while (1)
        {
            char *d;
            d = strchr(c, '/');
            if (*c)
            {
                if (d)
                {
                    *d++ = 0;
                    strcpy(ptr->brd, head->brdname);
                    strcpy(ptr->id, c);
                    ptr++;
                    c = d;
                }
                else
                {
                    strcpy(ptr->brd, head->brdname);
                    strcpy(ptr->id, c);
                    ptr++;
                    break;
                }
            }
            else
                break;
        }
        head->BM[0] = '\0';
    }
    while (++head < tail);

    fd = open(fpath, O_WRONLY | O_CREAT | O_TRUNC, 0600);
    ptr = bm;
    do
    {
        acct_load(&acct, ptr->id);
        head = bshm->bcache + brd_bno(ptr->brd);
        if (acct.userlevel & PERM_SYSOP)
        {
            if (*(head->BM))
            {
                strcat(head->BM, "/");
                strcat(head->BM, ptr->id);
            }
            else
            {
                strcpy(head->BM, ptr->id);
            }
        }
        else
        {
            write(fd, ptr, sizeof(BM));
        }
    }
    while (*(++ptr)->id);

    close(fd);
    free(bm);
    return DL_RELEASE(0);
}

static int
find_bm(
const char *fpath,
const char *id)
{
    BM bm;
    int fd;
    int pos = 0;

    fd = open(fpath, O_RDONLY);
    while (fd >= 0)
    {
        lseek(fd, (off_t)(sizeof(BM) * pos), SEEK_SET);
        if (read(fd, &bm, sizeof(BM)) == sizeof(BM))
        {
            if (!strcmp(bm.id, id) && bm.check == 0)
            {
                close(fd);
                return pos;
            }
            pos++;
        }
        else
        {
            close(fd);
            return -1;
        }
    }
    return -1;
}

int
user_check_bm(void)
{
    DL_HOLD;
    char buf[128], temp[3];
    int ans, i;
    const char *fpath;
    BM bm;
    BRD *head;
    struct stat st;

    i = 1;
    fpath = BM_CHECK_FILE;

    if (utmp_count(cuser.userno, 0) > 1)
    {
        vmsg("�Х��n�X��L�b���I");
        return DL_RELEASE(0);
    }

    if (access(fpath, 0))
    {
        vmsg("�{�b�S�����D�T�{�\\��I");
        return DL_RELEASE(0);
    }
    if (!stat(fpath, &st) && (st.st_mtime + CHECK_BM_TIME) < time(0))
    {
        vmsg("�W�L�{�Үɶ��A�Э��s�ӽСI");
        return DL_RELEASE(0);
    }


    clear();

    while ((ans = find_bm(fpath, cuser.userid)) >= 0)
    {
        rec_get(fpath, &bm, sizeof(BM), ans);
        head = bshm->bcache + brd_bno(bm.brd);
        sprintf(buf, "�A�n�~�򱵥� %s �����D�� [Y/n/q]:", bm.brd);
        vget(i++, 0, buf, temp, 3, DOECHO);
        if (*temp == 'q')
            return DL_RELEASE(0);
        if (*temp != 'n' && !is_bm(head->BM))
        {
            if (*(head->BM))
            {
                strcat(head->BM, "/");
                strcat(head->BM, cuser.userid);
            }
            else
            {
                strcpy(head->BM, cuser.userid);
            }
            rec_put(FN_BRD, head, sizeof(BRD), brd_bno(bm.brd));
        }
        bm.check = 1;
        rec_put(fpath, &bm, sizeof(BM), ans);
    }
    vmsg("�A�w�g�����Ҧ������D�T�{�F�I");
    return DL_RELEASE(0);
}

static void
search(void)
{
    FILE *fp;
    char buf[256], input[60];
    int i, key = 0;


    i = 1;
    fp = fopen(FN_MATCH_LOG, "r");
    if (fp)
    {
        clear();
        vs_bar("�S��j�M");
        vget(B_LINES_REF, 0, "�j�M���e�G", input, sizeof(input), DOECHO);
        while (fgets(buf, sizeof(buf), fp))
        {
            if (strstr(buf, input))
            {
                move(i, 0);
                prints("%s", buf);
                i++;
            }
            if (i >= b_lines)
            {
                key = vmsg("�� q ���}�A���N���~��");
                i = 1;
                move(i, 0);
                clrtobot();
            }
            if (key == 'q')
                break;
        }

    }
    if (fp)
    {
        vmsg("�j�M����");
        fclose(fp);
    }
    else
    {
        vmsg("�|����s");
    }
}

static void
update_match(void)
{
    if (access(FN_MATCH_NEW, 0))
        PROC_CMD_BG(BINARY_SUFFIX"match", cuser.userid);
    else
        vmsg("���b�u�@��");
}

static void
update_email(void)
{
    if (access(FN_ETC_EMAILADDR_ACL".new", 0))
        PROC_CMD_BG(BINARY_SUFFIX"checkemail", cuser.userid);
    else
        vmsg("���b�u�@��");
}

static void
update_spammer_acl(void)
{
    if (access(FN_ETC_SPAMMER_ACL".new", 0))
    {
        PROC_CMD(BINARY_SUFFIX"clean_acl", FN_ETC_SPAMMER_ACL, FN_ETC_SPAMMER_ACL".new");
        rename(FN_ETC_SPAMMER_ACL".new", FN_ETC_SPAMMER_ACL);
    }
    else
        vmsg("���b�u�@��");
}

static void
update_untrust_acl(void)
{
    if (access(FN_ETC_UNTRUST_ACL".new", 0))
    {
        PROC_CMD(BINARY_SUFFIX"clean_acl", FN_ETC_UNTRUST_ACL, FN_ETC_UNTRUST_ACL".new");
        rename(FN_ETC_UNTRUST_ACL".new", FN_ETC_UNTRUST_ACL);
    }
    else
        vmsg("���b�u�@��");
}

int
update_all(void)
{
    DL_HOLD;
    int ans;
    ans = vans("��s���ءG 1)�S��j�M 2)���U�H�c�Ӽ� 3)SPAM�W�� 4)���H���W�� 0)���� [0]");
    switch (ans)
    {
    case '1':
        update_match();
        break;
    case '2':
        update_email();
        break;
    case '3':
        update_spammer_acl();
        break;
    case '4':
        update_untrust_acl();
        break;
    }
    return DL_RELEASE(0);
}


int
special_search(void)
{
    DL_HOLD;
    int ans;
    move(b_lines - 1, 0);
    outs("�S��j�M�� ID�B�u��m�W�B�{�ҫH�c����M��ơA�Ω�B�z�H�k�ưȤ��d�ߡC");
    ans = vans("�S��j�M�G 1)��s��� 2)�j�M 0)���� [0]");
    switch (ans)
    {
    case '1':
        update_match();
        break;
    case '2':
        search();
        break;
    }

    return DL_RELEASE(1);
}

int
m_xfile(void)
{
    DL_HOLD;

    static const char *const desc[] =
    {
        "���n���i",             /* lkchu.990510: edit ~/etc/announce online */
        "�����W��",
        "�ק� Email",
        "�s��W������",
        "�����{�Ҫ���k",
        "�����{�ҫH��",
        "�ݪO����",
        "�s�i/�U���H�W��",         /* lkchu.981201: �u�W�s�� mail.acl */
        "���\\���U�W��",
        "�T��W����m",
        "���H���W��",           /* pcbug.990806: edit ~/etc/untrust */
        "�{������",
        "���v�W���@��",
        "���ȦC��",
        "���q�L�����{��",
        "�׫H����",
        "�I�q����",
        "�ΦW������",
        "���U�满��",
        "�׵o�H�n��",
        "Email �q�L�{��",
        "POP3 �q�L�{��",
        "BMTA �q�L�{��",
        NULL
    };

    static const char *const path[] =
    {
        FN_ETC_ANNOUNCE,
        FN_ETC_BADID,
        FN_ETC_EMAIL,
        FN_ETC_NEWUSER,
        FN_ETC_JUSTIFY,
        FN_ETC_VALID,
        FN_ETC_EXPIRE_CONF,
        FN_ETC_SPAMMER_ACL,
        FN_ETC_ALLOW_ACL,
        FN_ETC_BANIP_ACL,
        FN_ETC_UNTRUST_ACL,
        FN_ETC_VERSION,
        FN_ETC_COUNTER,
        FN_ETC_SYSOP,
        FN_ETC_NOTIFY,
        FN_BANMAIL_LOG,
        FN_SONG_LOG,
        FN_ANONYMOUS_LOG,
        FN_ETC_RFORM,
        FN_ETC_MAILER_ACL,
        FN_ETC_APPROVED,
        FN_ETC_JUSTIFIED_POP3,
        FN_ETC_JUSTIFIED_BMTA
    };

    x_file(M_XFILES, desc, path);
    return DL_RELEASE(0);
}

int
m_xhlp(void)
{
    DL_HOLD;

    static const char *const desc[] =
    {
        "�i���s�i",
        "���U���ܵe��",
        "���~�ʺA�ݪO�e��",
        "�i���e��",
        "�峹�o�����",
        "���~�n�J�e��",
        "�w��e��",
        "�t�κ޲z��",
        "�W���q���W��",
        "�׫H�C��",
        "�T�����",
        "�ݪO",
        "�ݪO���",
        "�p���W��",
        "�s�边",
        "�n�ͦW��",
        "��ذ�",
        "�q�l�H�c",
        "�Ƨѿ�",
        "�l��t��",
        "�\\Ū�峹",
        "�s�p�t��",
        "�I�q�t��",
        "������Ѥ�U",
        "�벼�c",
        "�̷R���ܵe��",
        NULL
    };

    static const char *const path[] =
    {
        "gem/@/@AD",
        "gem/@/@apply",
        FN_ERROR_CAMERA,
        "gem/@/@income",
        "gem/@/@post",
        "gem/@/@tryout",
        "gem/@/@welcome",
        "gem/@/@admin.hlp",
        "gem/@/@aloha.hlp",
        "gem/@/@banmail.hlp",
        "gem/@/@bmw.hlp",
        "gem/@/@board.hlp",
        "gem/@/@class.hlp",
        "gem/@/@contact.hlp",
        "gem/@/@edit.hlp",
        "gem/@/@friend.hlp",
        "gem/@/@gem.hlp",
        "gem/@/@mbox.hlp",
        "gem/@/@memorandum.hlp",
        "gem/@/@mime.hlp",
        "gem/@/@more.hlp",
        "gem/@/@signup.hlp",
        "gem/@/@song.hlp",
        "gem/@/@ulist.hlp",
        "gem/@/@vote.hlp",
        "gem/@/@myfav.hlp"
    };

    x_file(M_XFILES, desc, path);
    return DL_RELEASE(0);
}

/* pcbug.990620: �i�ologin...:p */
int
m_resetsys(
const void *arg)
{
    DL_HOLD;
    time_t now;
    struct tm ntime, *xtime;
    int select = (int)arg;
    now = time(NULL);
    xtime = localtime(&now);
    ntime = *xtime;

    if (vans("�z�T�w�n���m�t�ζܡH[y/N]") != 'y')
        return DL_RELEASE(0);
    switch (select)
    {
    case 1:
        PROC_CMD(BINARY_SUFFIX"camera", NULL);
        logitfile(FN_RESET_LOG, "< �ʺA�ݪO >", NULL);
        break;
    case 2:
        PROC_CMD(BINARY_SUFFIX"acpro", NULL);
        board_main();
        logitfile(FN_RESET_LOG, "< �����ݪO >", NULL);
        break;
    case 3:
        system("kill -9 `ps -auxwww | grep innbbsd | awk '{print $2}'`; "
               "kill -9 `ps -auxwww | grep bbslink | awk '{print $2}'`; "
               "kill -9 `ps -auxwww | grep bbsnnrp | awk '{print $2}'`");
        logitfile(FN_RESET_LOG, "< ��H���H >", NULL);
        break;
    case 4:
        system("kill -9 `top | grep RUN | grep bbsd | awk '{print $1}'`");
        logitfile(FN_RESET_LOG, "< ���`�{�� >", NULL);
        break;
    case 5:
        PROC_CMD(BINARY_SUFFIX"makefw", NULL);
        logitfile(FN_RESET_LOG, "< �׫H�C�� >", NULL);
        break;
    case 6:
        system("kill -9 `ps -auxwww | grep xchatd | awk '{print $2}'`");
        logitfile(FN_RESET_LOG, "< �D��ѫ� >", NULL);
        break;
    case 7:
        PROC_CMD(BINARY_SUFFIX"camera", NULL);
        PROC_CMD(BINARY_SUFFIX"acpro", NULL);
        system("kill -9 `ps -auxwww | grep innbbsd | awk '{print $2}'`; "
               "kill -9 `ps -auxwww | grep bbslink | awk '{print $2}'`; "
               "kill -9 `ps -auxwww | grep bbsnnrp | awk '{print $2}'`; "
               "kill -9 `ps -auxwww | grep xchatd  | awk '{print $2}'`");
        board_main();
        logitfile(FN_RESET_LOG, "< �����t�� >", NULL);
        break;
    }
    return DL_RELEASE(0);
}
