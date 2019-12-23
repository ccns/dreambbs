/*-------------------------------------------------------*/
/* util.c       ( YZU WindTopBBS Ver 3.02 )              */
/*-------------------------------------------------------*/
/* target : 版主確認，寄信給全站，更新系統檔案           */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/


#include "bbs.h"

#define BM_CHECK_FILE   FN_CHECKBM_LOG

extern BCACHE *bshm;

typedef struct
{
    char id[IDLEN+1];
    char brd[IDLEN+1];
    int check;
} BM;


static int
use_io(void)
{
    int mode;
    mode = vans("使用外部程序嗎？ [Y/n] ");
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

/* 清除特定看板版面文章 cancel */
int
m_expire(void)
{
    BRD *brd;
    char bname[16];
    char buf[80];

    move(22, 0);
    outs("清除特定看板 cancel 之文章。");
    if ((brd = ask_board(bname, BRD_R_BIT, NULL)))
    {
        sprintf(buf, BINARY_SUFFIX"expire 999 20000 20000 \"%s\" &", brd->brdname);
        system(buf);
        logitfile(FN_EXPIRED_LOG, cuser.userid, buf);
    }
    else
    {
        vmsg(err_bid);
    }

    return 0;
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
    BRD *bhdr, *head, *tail;
    char *ptr, *bm;
    char fpath[256], *title, buf[128];
    FILE *fp;

    if (bbsothermode & OTHERSTAT_EDITING)
    {
        vmsg("你還有檔案還沒編完哦！");
        return -1;
    }


    bm = (char *)malloc(MAXBOARD * (IDLEN + 1) * 3);
    memset(bm, 0, MAXBOARD*(IDLEN + 1)*3);
    ptr = bm;
    utmp_mode(M_SMAIL);

    head = bhdr = bshm->bcache;
    tail = bhdr + bshm->number;
    do                          /* 至少有sysop一版 */
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
    strcpy(ve_title, "[板主通告]");
    title = ve_title;
    vget(1, 0, "◎ 主 題：", title, 60, GCARRY);
    sprintf(buf, "mailtobm.%lld", (long long)time(0));
    usr_fpath(fpath, cuser.userid, buf);
    if ((fp = fopen(fpath, "w")))
    {
        fprintf(fp, "※ [板主通告] 站長通告，收信人：各板主\n");
        fprintf(fp, "-------------------------------------------------------------------------\n");
        fclose(fp);
    }
    utmp_mode(M_SMAIL);
    curredit = EDIT_MAIL | EDIT_LIST;

    if (vedit(fpath, true) == -1)
    {
        vmsg(msg_cancel);
        free(bm);
        return -1;
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
            char command[128];
            sprintf(command, BINARY_SUFFIX"mailtoall 2 \"%s\" \"%s\" &", fpath, title);
            system(command);
        }
    }
    free(bm);
    return 0;
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
    char *title;
    char fpath[256];
    char buf[128];

    if (bbsothermode & OTHERSTAT_EDITING)
    {
        vmsg("你還有檔案還沒編完哦！");
        return -1;
    }


    strcpy(ve_title, "[系統通告]");
    title = ve_title;
    vget(1, 0, "◎ 主 題：", title, 60, GCARRY);
    sprintf(buf, "mailtoall.%lld", (long long)time(0));
    usr_fpath(fpath, cuser.userid, buf);
    utmp_mode(M_SMAIL);
    curredit = EDIT_MAIL | EDIT_LIST;
    if (vedit(fpath, true) == -1)
    {
        vmsg(msg_cancel);
        return -1;
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
            char command[128];
            sprintf(command, BINARY_SUFFIX"mailtoall 1 \"%s\" \"%s\" &", fpath, title);
            system(command);
        }
    }
    return 0;
}

GCC_PURE static bool
is_bms(
const char *list,             /* 板主：BM list */
const char *userid)
{
    int cc, len;

    len = strlen(userid);
    do
    {
        cc = list[len];
        if ((!cc || cc == '/') && !str_ncmp(list, userid, len))
        {
            return true;
        }
        while ((cc = *list++))
        {
            if (cc == '/')
                break;
        }
    }
    while (cc);

    return false;
}

GCC_PURE static inline bool
is_bm(
const char *list)             /* 板主：BM list */
{
    int cc, len;
    const char *userid;

    len = strlen(userid = cuser.userid);
    do
    {
        cc = list[len];
        if ((!cc || cc == '/') && !str_ncmp(list, userid, len))
        {
            return true;
        }
        while ((cc = *list++))
        {
            if (cc == '/')
                break;
        }
    }
    while (cc);

    return false;
}


int
bm_check(void)
{
    BRD *bhdr, *head, *tail;
    BM *bm, *ptr;
    char fpath[80], ans;
    ACCT acct;
    int fd;


    strcpy(fpath, BM_CHECK_FILE);
    move(22, 0);
    outs("每半年確認版主是否繼續連任，可於開學後兩週執行。");
    ans = vans("要做版主確認嗎 y)確定 r)復原 s)重來 d)刪除 q)離開 [q]:");
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
        return 0;
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
        return 0;
    }
    else if (ans == 'd')
    {
        unlink(fpath);
        return 0;
    }
    else if (ans != 'y')
    {
        return 0;
    }
    if (!access(fpath, 0))
    {
        vmsg("正在確認中！");
        return 0;
    }

    bm = (BM *)malloc(sizeof(BM) * MAXBOARD * 3);
    memset(bm, 0, sizeof(BM)*MAXBOARD*3);
    ptr = bm;
    utmp_mode(M_SMAIL);

    head = bhdr = bshm->bcache;
    tail = bhdr + bshm->number;
    do                          /* 至少有sysop一版 */
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
    return 0;
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
        vmsg("請先登出其他帳號！");
        return 0;
    }

    if (access(fpath, 0))
    {
        vmsg("現在沒有版主確認功\能！");
        return 0;
    }
    if (!stat(fpath, &st) && (st.st_mtime + CHECK_BM_TIME) < time(0))
    {
        vmsg("超過認證時間，請重新申請！");
        return 0;
    }


    clear();

    while ((ans = find_bm(fpath, cuser.userid)) >= 0)
    {
        rec_get(fpath, &bm, sizeof(BM), ans);
        head = bshm->bcache + brd_bno(bm.brd);
        sprintf(buf, "你要繼續接任 %s 版版主嗎 [Y/n/q]:", bm.brd);
        vget(i++, 0, buf, temp, 3, DOECHO);
        if (*temp == 'q')
            return 0;
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
    vmsg("你已經做完所有的版主確認了！");
    return 0;
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
        vs_bar("特殊搜尋");
        vget(b_lines, 0, "搜尋內容：", input, sizeof(input), DOECHO);
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
                key = vmsg("按 q 離開，任意鍵繼續");
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
        vmsg("搜尋結束");
        fclose(fp);
    }
    else
    {
        vmsg("尚未更新");
    }
}

static void
update_match(void)
{
    char fpath[128];
    sprintf(fpath, BINARY_SUFFIX"match \"%s\" &", cuser.userid);
    if (access(FN_MATCH_NEW, 0))
        system(fpath);
    else
        vmsg("正在工作中");
}

static void
update_email(void)
{
    char fpath[128];
    sprintf(fpath, BINARY_SUFFIX"checkemail \"%s\" &", cuser.userid);
    if (access(FN_ETC_EMAILADDR_ACL".new", 0))
        system(fpath);
    else
        vmsg("正在工作中");
}

static void
update_spammer_acl(void)
{
    if (access(FN_ETC_SPAMMER_ACL".new", 0))
    {
        system(BINARY_SUFFIX"clean_acl " FN_ETC_SPAMMER_ACL " " FN_ETC_SPAMMER_ACL".new");
        rename(FN_ETC_SPAMMER_ACL".new", FN_ETC_SPAMMER_ACL);
    }
    else
        vmsg("正在工作中");
}

static void
update_untrust_acl(void)
{
    if (access(FN_ETC_UNTRUST_ACL".new", 0))
    {
        system(BINARY_SUFFIX"clean_acl " FN_ETC_UNTRUST_ACL " " FN_ETC_UNTRUST_ACL ".new");
        rename(FN_ETC_UNTRUST_ACL".new", FN_ETC_UNTRUST_ACL);
    }
    else
        vmsg("正在工作中");
}

int
update_all(void)
{
    int ans;
    ans = vans("更新項目： 1)特殊搜尋 2)註冊信箱個數 3)SPAM名單 4)不信任名單 0)結束 [0]");
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
    return 0;
}


int
special_search(void)
{
    int ans;
    move(22, 0);
    outs("特殊搜尋為 ID、真實姓名、認證信箱的對映資料，用於處理違法事務之查詢。");
    ans = vans("特殊搜尋： 1)更新資料 2)搜尋 0)結束 [0]");
    switch (ans)
    {
    case '1':
        update_match();
        break;
    case '2':
        search();
        break;
    }

    return 1;
}

int
m_xfile(void)
{
    static const char *const desc[] =
    {
        "重要公告",             /* lkchu.990510: edit ~/etc/announce online */
        "不雅名單",
        "修改 Email",
        "新手上路須知",
        "身份認證的方法",
        "身份認證信函",
        "看板期限",
        "廣告/垃圾信名單",         /* lkchu.981201: 線上編輯 mail.acl */
        "允許\註冊名單",
        "禁止上站位置",
        "不信任名單",           /* pcbug.990806: edit ~/etc/untrust */
        "情書產生器",
        "程式版本",
        "歷史上的一刻",
        "站務列表",
        "未通過身分認證",
        "擋信紀錄",
        "點歌紀錄",
        "匿名版紀錄",
        "註冊單說明",
        "擋發信軟體",
        "Email 通過認證",
        "POP3 通過認證",
        "BMTA 通過認證",
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
        FN_ETC_LOVEPAPER,
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
    return 0;
}

int
m_xhlp(void)
{
    static const char *const desc[] =
    {
        "進站廣告",
        "註冊提示畫面",
        "錯誤動態看板畫面",
        "進站畫面",
        "文章發表綱領",
        "錯誤登入畫面",
        "歡迎畫面",
        "系統管理員",
        "上站通知名單",
        "擋信列表",
        "訊息選單",
        "看板",
        "看板選單",
        "聯絡名單",
        "編輯器",
        "好友名單",
        "精華區",
        "電子信箱",
        "備忘錄",
        "郵件系統",
        "閱\讀文章",
        "連署系統",
        "點歌系統",
        "完全聊天手冊",
        "投票箱",
        "最愛提示畫面",
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
    return 0;
}

/* pcbug.990620: 懶得login...:p */
static void
m_resetsys(
int select)
{
    time_t now;
    struct tm ntime, *xtime;
    now = time(NULL);
    xtime = localtime(&now);
    ntime = *xtime;

    if (vans("您確定要重置系統嗎？[y/N]") != 'y')
        return;
    switch (select)
    {
    case 1:
        system(BINARY_SUFFIX"camera");
        logitfile(FN_RESET_LOG, "< 動態看板 >", NULL);
        break;
    case 2:
        system(BINARY_SUFFIX"acpro");
        board_main();
        logitfile(FN_RESET_LOG, "< 分類看板 >", NULL);
        break;
    case 3:
        system("kill -9 `cat run/bmta.pid`; "
               "kill -9 `ps -auxwww | grep innbbsd | awk '{print $2}'`; "
               "kill -9 `ps -auxwww | grep bbslink | awk '{print $2}'`; "
               "kill -9 `ps -auxwww | grep bbsnnrp | awk '{print $2}'`");
        logitfile(FN_RESET_LOG, "< 轉信收信 >", NULL);
        break;
    case 4:
        system("kill -9 `top | grep RUN | grep bbsd | awk '{print $1}'`");
        logitfile(FN_RESET_LOG, "< 異常程序 >", NULL);
        break;
    case 5:
        system(BINARY_SUFFIX"makefw");
        logitfile(FN_RESET_LOG, "< 擋信列表 >", NULL);
        break;
    case 6:
        system("kill -9 `ps -auxwww | grep xchatd | awk '{print $2}'`");
        logitfile(FN_RESET_LOG, "< 主聊天室 >", NULL);
        break;
    case 7:
        system("kill -9 `cat run/bmta.pid`; "
               BINARY_SUFFIX"camera; "
               BINARY_SUFFIX"acpro; "
               "kill -9 `ps -auxwww | grep innbbsd | awk '{print $2}'`; "
               "kill -9 `ps -auxwww | grep bbslink | awk '{print $2}'`; "
               "kill -9 `ps -auxwww | grep bbsnnrp | awk '{print $2}'`; "
               "kill -9 `ps -auxwww | grep xchatd  | awk '{print $2}'`");
        board_main();
        logitfile(FN_RESET_LOG, "< 全部系統 >", NULL);
        break;
    }

}

int
reset1(void)
{
    m_resetsys(1);
    return 0;
}

int
reset2(void)
{
    m_resetsys(2);
    return 0;
}

int
reset3(void)
{
    m_resetsys(3);
    return 0;
}

int
reset4(void)
{
    m_resetsys(4);
    return 0;
}

int
reset5(void)
{
    m_resetsys(5);
    return 0;
}

int
reset6(void)
{
    m_resetsys(6);
    return 0;
}

int
reset7(void)
{
    m_resetsys(7);
    return 0;
}
