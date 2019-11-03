/*-------------------------------------------------------*/
/* util/transacct.c     ( NTHU CS MapleBBS Ver 3.10 )    */
/*-------------------------------------------------------*/
/* target : DreamBBS -> DreamBBS.2010 ACCT 轉換程式      */
/* create : 09/10/08                                     */
/* update :                                              */
/* author : cache.bbs@bbs.ee.ncku.edu.tw                 */
/*-------------------------------------------------------*/
/* syntax : transacct [userid]                           */
/*-------------------------------------------------------*/


#if 0

請先備份所有資料, 並檢查是否新舊資料結構是正確的

#endif


#include "bbs.h"
#define FN_MONEY        ".MONEY"        /* PostRecommendHistory */

double m1 = 0;
double m2 = 0;
double m3 = 0;

/* ----------------------------------------------------- */
/* (新的) 使用者帳號 .ACCT struct                        */
/* ----------------------------------------------------- */


typedef struct
{
    int userno;                 /* unique positive code */
    char userid[IDLEN + 1];     /* userid */
    char passwd[PASSLEN];       /* user password crypt by DES */
    unsigned char signature;    /* user signature number */
    char realname[20];          /* user realname */
    char username[24];          /* user nickname */
    unsigned int userlevel;     /* user perm */
    int numlogins;              /* user login times */
    int numposts;               /* user post times */
    unsigned int ufo;           /* user basic flags */
    time_t firstlogin;          /* user first login time */
    time_t lastlogin;           /* user last login time */
    time_t staytime;            /* user total stay time */
    time_t tcheck;              /* time to check mbox/pal */
    char lasthost[32];          /* user last login remote host */
    int numemail;               /* 原為寄發 Inetrnet E-mail 次數, 在不更改資料結構的狀況下, 擴充為積分 */
    time_t tvalid;              /* 通過認證、更改 mail address 的時間 */
    char email[60];             /* user email */
    char address[60];           /* user address */
    char justify[60];           /* FROM of replied justify mail */
    char vmail[60];             /* 通過認證之 email */
    time_t deny;                /* user violatelaw time */
    int request;                /* 點歌系統 */
    int money;                  /* 夢幣 */
    unsigned int ufo2;          /* 延伸的個人設定 */
    char ident[96];             /* user remote host ident */
    int point1;                 /* 優良積分 */
    int point2;                 /* 劣文 */
    time_t vtime;               /* validate time */
}       NEW;


/* ----------------------------------------------------- */
/* (舊的) 使用者帳號 .ACCT struct                        */
/* ----------------------------------------------------- */


typedef struct
{
    int userno;                 /* unique positive code */
    char userid[IDLEN + 1];     /* userid */
    char passwd[PASSLEN];       /* user password crypt by DES */
    unsigned char signature;    /* user signature number */
    char realname[20];          /* user realname */
    char username[24];          /* user nickname */
    unsigned int userlevel;     /* user perm */
    int numlogins;              /* user login times */
    int numposts;               /* user post times */
    unsigned int ufo;           /* user basic flags */
    time_t firstlogin;          /* user first login time */
    time_t lastlogin;           /* user last login time */
    time_t staytime;            /* user total stay time */
    time_t tcheck;              /* time to check mbox/pal */
    char lasthost[32];          /* user last login remote host */
    int numemail;               /* 寄發 Inetrnet E-mail 次數 */
    time_t tvalid;              /* 通過認證、更改 mail address 的時間 */
    char email[60];             /* user email */
    char address[60];           /* user address */
    char justify[60];           /* FROM of replied justify mail */
    char vmail[60];             /* 通過認證之 email */
    time_t deny;                /* user violatelaw time */
    int extrambox;              /* 加大信箱 (最大 50 封) */
    int extrasize;              /* 加大信箱容量 (最大 1000K) */
    unsigned int ufo2;          /* 延伸的個人設定 */
    char ident[103];            /* user remote host ident */
    char barcolor;              /* 光棒顏色 */
    time_t vtime;               /* validate time */
}       OLD;


#define OLDUFO2_COLOR      BFLAG(0)        /* true if the ANSI color mode open */
#define OLDUFO2_MOVIE      BFLAG(1)        /* true if show movie */
#define OLDUFO2_BRDNEW     BFLAG(2)        /* 新文章模式 */
#define OLDUFO2_BNOTE      BFLAG(3)        /* 顯示進板畫面 */
#define OLDUFO2_VEDIT      BFLAG(4)        /* 簡化編輯器 */
#define OLDUFO2_PAL        BFLAG(5)        /* true if show pals only */
#define OLDUFO2_MOTD       BFLAG(6)        /* 簡化進站畫面 */
#define OLDUFO2_MIME       BFLAG(7)        /* MIME 解碼 */
#define OLDUFO2_SIGN       BFLAG(8)        /* 簽名檔 */
#define OLDUFO2_SHOWUSER   BFLAG(9)        /* 顯示 ID 和 暱稱 */
#define OLDUFO2_PRH        BFLAG(10)       /* 關閉推薦文章分數 */
#define OLDUFO2_SHIP       BFLAG(11)       /* visor.991030: 好友描述 */
#define OLDUFO2_NWLOG      BFLAG(12)       /* lkchu.990510: 不存對話紀錄 */
#define OLDUFO2_NTLOG      BFLAG(13)       /* lkchu.990510: 不存聊天紀錄 */
#define OLDUFO2_CIRCLE     BFLAG(14)       /* 循環閱讀 */
#define OLDUFO2_ORIGUI     BFLAG(15)       /* 關閉超炫介面 */
#define OLDUFO2_DEF_ANONY  BFLAG(16)       /* 預設不匿名 */
#define OLDUFO2_DEF_LEAVE  BFLAG(17)       /* 預設不離站 */
#define OLDUFO2_ACL        BFLAG(24)       /* true if ACL was ON */
#define OLDUFO2_REALNAME   BFLAG(28)       /* visor.991030: 真實姓名 */

#define UFO2_COLOR         BFLAG(0)        /* true if the ANSI color mode open */
#define UFO2_MOVIE         BFLAG(1)        /* true if show movie */
#define UFO2_BRDNEW        BFLAG(2)        /* 新文章模式 */
#define UFO2_BNOTE         BFLAG(3)        /* 顯示進板畫面 */
#define UFO2_VEDIT         BFLAG(4)        /* 簡化編輯器 */
#define UFO2_PAL           BFLAG(5)        /* true if show pals only */
#define UFO2_MOTD          BFLAG(6)        /* 簡化進站畫面 */
#define UFO2_MIME          BFLAG(7)        /* MIME 解碼 */
#define UFO2_SIGN          BFLAG(8)        /* 簽名檔 */
#define UFO2_SHOWUSER      BFLAG(9)        /* 顯示 ID 和 暱稱 */
#define UFO2_PRH           BFLAG(10)       /* 關閉推薦文章分數 */
#define UFO2_SHIP          BFLAG(11)       /* visor.991030: 好友描述 */
#define UFO2_NWLOG         BFLAG(12)       /* lkchu.990510: 不存對話紀錄 */
#define UFO2_NTLOG         BFLAG(13)       /* lkchu.990510: 不存聊天紀錄 */
#define UFO2_CIRCLE        BFLAG(14)       /* 循環閱讀 */
#define UFO2_ORIGUI        BFLAG(15)       /* 關閉超炫介面 */
#define UFO2_DEF_ANONY     BFLAG(16)       /* 預設不匿名 */
#define UFO2_DEF_LEAVE     BFLAG(17)       /* 預設不離站 */
#define UFO2_ACL           BFLAG(24)       /* true if ACL was ON */
#define UFO2_REALNAME      BFLAG(28)       /* visor.991030: 真實姓名 */


static unsigned int
trans_ufo2(
    unsigned int oldufo2)
{
    unsigned int ufo;

    ufo = 0;

//  if (oldufo2 & OLDUFO2_COLOR)
        ufo |= UFO2_COLOR;

//  if (oldufo2 & OLDUFO2_MOVIE)
        ufo |= UFO2_MOVIE;

//  if (oldufo2 & OLDUFO2_BRDNEW)
        ufo |= UFO2_BRDNEW;

//  if (oldufo2 & OLDUFO2_BNOTE)
        ufo |= UFO2_BNOTE;

    if (oldufo2 & OLDUFO2_VEDIT)
        ufo |= UFO2_VEDIT;

    if (oldufo2 & OLDUFO2_MOTD)
        ufo |= UFO2_MOTD;

    if (oldufo2 & OLDUFO2_MIME)
        ufo |= UFO2_MIME;

    if (oldufo2 & OLDUFO2_SIGN)
        ufo |= UFO2_SIGN;

    if (oldufo2 & OLDUFO2_SHOWUSER)
        ufo |= UFO2_SHOWUSER;

    if (oldufo2 & OLDUFO2_PRH)
        ufo |= UFO2_PRH;

    if (oldufo2 & OLDUFO2_SHIP)
        ufo |= UFO2_SHIP;

    if (oldufo2 & OLDUFO2_NWLOG)
        ufo |= UFO2_NWLOG;

    if (oldufo2 & OLDUFO2_NTLOG)
        ufo |= UFO2_NTLOG;

    if (oldufo2 & OLDUFO2_CIRCLE)
        ufo |= UFO2_CIRCLE;

    if (oldufo2 & OLDUFO2_ORIGUI)
        ufo |= UFO2_ORIGUI;

    if (oldufo2 & OLDUFO2_DEF_ANONY)
        ufo |= UFO2_DEF_ANONY;

//  if (oldufo2 & OLDUFO2_DEF_LEAVE)
//      ufo |= UFO2_DEF_LEAVE;

    if (oldufo2 & OLDUFO2_ACL)
        ufo |= UFO2_ACL;

    if (oldufo2 & OLDUFO2_REALNAME)
        ufo |= UFO2_REALNAME;

    return ufo;
}

/* ----------------------------------------------------- */
/* 轉換主程式                                            */
/* ----------------------------------------------------- */


static void
trans_acct(
    const OLD *old,
    NEW *new_)
{
    memset(new_, 0, sizeof(NEW));

    new_->userno = old->userno;

    str_ncpy(new_->userid, old->userid, sizeof(new_->userid));
    str_ncpy(new_->passwd, old->passwd, sizeof(new_->passwd));
    str_ncpy(new_->realname, old->realname, sizeof(new_->realname));
    str_ncpy(new_->username, old->username, sizeof(new_->username));

    new_->userlevel = old->userlevel;
    new_->ufo = old->ufo;
    new_->signature = old->signature;
    new_->ufo2 = trans_ufo2(old->ufo2);

    new_->numlogins = old->numlogins;
    new_->numposts = old->numposts;
    new_->numemail = old->numemail;

    new_->firstlogin = old->firstlogin;
    new_->lastlogin = old->lastlogin;
    new_->tcheck = old->tcheck;
    new_->staytime = old->staytime;
    new_->tvalid = old->tvalid;
    new_->deny = old->deny;
    new_->vtime = old->vtime;

    str_ncpy(new_->lasthost, old->lasthost, sizeof(new_->lasthost));
    str_ncpy(new_->email, old->email, sizeof(new_->email));
    str_ncpy(new_->address, old->address, sizeof(new_->address));
    str_ncpy(new_->justify, old->justify, sizeof(new_->justify));
    str_ncpy(new_->vmail, old->vmail, sizeof(new_->vmail));
    str_ncpy(new_->ident, old->ident, sizeof(new_->ident));

    new_->request = (int)m3;
    new_->money  = (int)m1;

    new_->point1 =0;           /* 優良積分 */
    new_->point2 =0;           /* 劣文 */

}

typedef struct
{
    int money;                /* 夢幣 */
    int save;                 /* 存款 */
    int request;              /* 小雞點券 */
}       MONEY;

int
main(
    int argc,
    char *argv[])
{
    NEW new_;
    MONEY oldwealth;
    char c;

    if (argc > 2)
    {
        fprintf(stderr, "Usage: %s [userid]\n", argv[0]);
        return 2;
    }

    for (c = 'a'; c <= 'z'; c++)
    {
        char buf[64];
        struct dirent *de;
        DIR *dirp;

        sprintf(buf, BBSHOME "/usr/%c", c);
        chdir(buf);

        if (!(dirp = opendir(".")))
            continue;

        while ( ( de = readdir(dirp) ) )
        {
            OLD old;
            int fd;
            char *str;

            str = de->d_name;
            if (*str <= ' ' || *str == '.')
                continue;

            if ((argc == 2) && str_cmp(str, argv[1]))
                continue;

//讀取MONEY
            sprintf(buf, "%s/" FN_MONEY, str);  //.MONEY似乎定義成FN_MONEY
            if ((fd = open(buf, O_RDONLY)) < 0)
                continue;

            read(fd, &oldwealth, sizeof(MONEY));
            close(fd);
//先不執行砍掉的動作            unlink(buf2);

            m1 = oldwealth.money;
            m2 = oldwealth.save;
            m3 = oldwealth.request;

            m1 = ((m1+m2)/100000);  //請自行修改

            if (m1 >= INT_MAX)
                m1 = INT_MAX;

            sprintf(buf, "%s/" FN_ACCT, str);
            if ((fd = open(buf, O_RDONLY)) < 0)
                continue;

            read(fd, &old, sizeof(OLD));
            close(fd);
            unlink(buf);                    /* itoc.010831: 砍掉原來的 FN_ACCT */

            trans_acct(&old, &new_);

            fd = open(buf, O_WRONLY | O_CREAT, 0600);       /* itoc.010831: 重建新的 FN_ACCT */
            write(fd, &new_, sizeof(NEW));
            close(fd);

        }
        printf("/usr/%c is done.\n", c);

        closedir(dirp);
    }

    return 0;
}
