/*-------------------------------------------------------*/
/* util/transacct.c     ( NTHU CS MapleBBS Ver 3.10 )    */
/*-------------------------------------------------------*/
/* target : DreamBBS -> DreamBBS.2010 ACCT �ഫ�{��      */
/* create : 09/10/08                                     */
/* update :                                              */
/* author : cache.bbs@bbs.ee.ncku.edu.tw                 */
/*-------------------------------------------------------*/
/* syntax : transacct [userid]                           */
/*-------------------------------------------------------*/


#if 0

�Х��ƥ��Ҧ����, ���ˬd�O�_�s�¸�Ƶ��c�O���T��

#endif


#include "bbs.h"
#define FN_MONEY        ".MONEY"        /* PostRecommendHistory */

double m1 = 0;
double m2 = 0;
double m3 = 0;

/* ----------------------------------------------------- */
/* (�s��) �ϥΪ̱b�� .ACCT struct                        */
/* ----------------------------------------------------- */


typedef struct
{
    int userno;                 /* unique positive code */
    char userid[IDLEN + 1];     /* userid */
    char passwd[PASSSIZE];      /* user password crypt by DES */
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
    int numemail;               /* �쬰�H�o Inetrnet E-mail ����, �b������Ƶ��c�����p�U, �X�R���n�� */
    time_t tvalid;              /* �q�L�{�ҡB��� mail address ���ɶ� */
    char email[60];             /* user email */
    char address[60];           /* user address */
    char justify[60];           /* FROM of replied justify mail */
    char vmail[60];             /* �q�L�{�Ҥ� email */
    time_t deny;                /* user violatelaw time */
    int request;                /* �I�q�t�� */
    int money;                  /* �ڹ� */
    unsigned int ufo2;          /* �������ӤH�]�w */
    char ident[96];             /* user remote host ident */
    int point1;                 /* �u�}�n�� */
    int point2;                 /* �H�� */
    time_t vtime;               /* validate time */
}       NEW;  /* DISKDATA(raw) */


/* ----------------------------------------------------- */
/* (�ª�) �ϥΪ̱b�� .ACCT struct                        */
/* ----------------------------------------------------- */


typedef struct
{
    int userno;                 /* unique positive code */
    char userid[IDLEN + 1];     /* userid */
    char passwd[PASSSIZE];      /* user password crypt by DES */
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
    int numemail;               /* �H�o Inetrnet E-mail ���� */
    time_t tvalid;              /* �q�L�{�ҡB��� mail address ���ɶ� */
    char email[60];             /* user email */
    char address[60];           /* user address */
    char justify[60];           /* FROM of replied justify mail */
    char vmail[60];             /* �q�L�{�Ҥ� email */
    time_t deny;                /* user violatelaw time */
    int extrambox;              /* �[�j�H�c (�̤j 50 ��) */
    int extrasize;              /* �[�j�H�c�e�q (�̤j 1000K) */
    unsigned int ufo2;          /* �������ӤH�]�w */
    char ident[103];            /* user remote host ident */
    char barcolor;              /* �����C�� */
    time_t vtime;               /* validate time */
}       OLD;  /* DISKDATA(raw) */


#define OLDUFO2_COLOR      BFLAG(0)        /* true if the ANSI color mode open */
#define OLDUFO2_MOVIE      BFLAG(1)        /* true if show movie */
#define OLDUFO2_BRDNEW     BFLAG(2)        /* �s�峹�Ҧ� */
#define OLDUFO2_BNOTE      BFLAG(3)        /* ��ܶi�O�e�� */
#define OLDUFO2_VEDIT      BFLAG(4)        /* ²�ƽs�边 */
#define OLDUFO2_PAL        BFLAG(5)        /* true if show pals only */
#define OLDUFO2_MOTD       BFLAG(6)        /* ²�ƶi���e�� */
#define OLDUFO2_MIME       BFLAG(7)        /* MIME �ѽX */
#define OLDUFO2_SIGN       BFLAG(8)        /* ñ�W�� */
#define OLDUFO2_SHOWUSER   BFLAG(9)        /* ��� ID �M �ʺ� */
#define OLDUFO2_PRH        BFLAG(10)       /* �������ˤ峹���� */
#define OLDUFO2_SHIP       BFLAG(11)       /* visor.991030: �n�ʹy�z */
#define OLDUFO2_NWLOG      BFLAG(12)       /* lkchu.990510: ���s��ܬ��� */
#define OLDUFO2_NTLOG      BFLAG(13)       /* lkchu.990510: ���s��Ѭ��� */
#define OLDUFO2_CIRCLE     BFLAG(14)       /* �`���\Ū */
#define OLDUFO2_ORIGUI     BFLAG(15)       /* �����W������ */
#define OLDUFO2_DEF_ANONY  BFLAG(16)       /* �w�]���ΦW */
#define OLDUFO2_DEF_LEAVE  BFLAG(17)       /* �w�]������ */
#define OLDUFO2_ACL        BFLAG(24)       /* true if ACL was ON */
#define OLDUFO2_REALNAME   BFLAG(28)       /* visor.991030: �u��m�W */

#define UFO2_COLOR         BFLAG(0)        /* true if the ANSI color mode open */
#define UFO2_MOVIE         BFLAG(1)        /* true if show movie */
#define UFO2_BRDNEW        BFLAG(2)        /* �s�峹�Ҧ� */
#define UFO2_BNOTE         BFLAG(3)        /* ��ܶi�O�e�� */
#define UFO2_VEDIT         BFLAG(4)        /* ²�ƽs�边 */
#define UFO2_PAL           BFLAG(5)        /* true if show pals only */
#define UFO2_MOTD          BFLAG(6)        /* ²�ƶi���e�� */
#define UFO2_MIME          BFLAG(7)        /* MIME �ѽX */
#define UFO2_SIGN          BFLAG(8)        /* ñ�W�� */
#define UFO2_SHOWUSER      BFLAG(9)        /* ��� ID �M �ʺ� */
#define UFO2_PRH           BFLAG(10)       /* �������ˤ峹���� */
#define UFO2_SHIP          BFLAG(11)       /* visor.991030: �n�ʹy�z */
#define UFO2_NWLOG         BFLAG(12)       /* lkchu.990510: ���s��ܬ��� */
#define UFO2_NTLOG         BFLAG(13)       /* lkchu.990510: ���s��Ѭ��� */
#define UFO2_CIRCLE        BFLAG(14)       /* �`���\Ū */
#define UFO2_ORIGUI        BFLAG(15)       /* �����W������ */
#define UFO2_DEF_ANONY     BFLAG(16)       /* �w�]���ΦW */
#define UFO2_DEF_LEAVE     BFLAG(17)       /* �w�]������ */
#define UFO2_ACL           BFLAG(24)       /* true if ACL was ON */
#define UFO2_REALNAME      BFLAG(28)       /* visor.991030: �u��m�W */


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
/* �ഫ�D�{��                                            */
/* ----------------------------------------------------- */


static void
trans_acct(
    const OLD *old,
    NEW *new_)
{
    memset(new_, 0, sizeof(NEW));

    new_->userno = old->userno;

    str_scpy(new_->userid, old->userid, sizeof(new_->userid));
    str_scpy(new_->passwd, old->passwd, sizeof(new_->passwd));
    str_scpy(new_->realname, old->realname, sizeof(new_->realname));
    str_scpy(new_->username, old->username, sizeof(new_->username));

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

    str_scpy(new_->lasthost, old->lasthost, sizeof(new_->lasthost));
    str_scpy(new_->email, old->email, sizeof(new_->email));
    str_scpy(new_->address, old->address, sizeof(new_->address));
    str_scpy(new_->justify, old->justify, sizeof(new_->justify));
    str_scpy(new_->vmail, old->vmail, sizeof(new_->vmail));
    str_scpy(new_->ident, old->ident, sizeof(new_->ident));

    new_->request = (int)m3;
    new_->money  = (int)m1;

    new_->point1 =0;           /* �u�}�n�� */
    new_->point2 =0;           /* �H�� */

}

typedef struct
{
    int money;                /* �ڹ� */
    int save;                 /* �s�� */
    int request;              /* �p���I�� */
}       MONEY;  /* DISKDATA(raw) */

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

    setgid(BBSGID);
    setuid(BBSUID);

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

            if ((argc == 2) && str_casecmp(str, argv[1]))
                continue;

//Ū��MONEY
            sprintf(buf, "%s/" FN_MONEY, str);  //.MONEY���G�w�q��FN_MONEY
            if ((fd = open(buf, O_RDONLY)) < 0)
                continue;

            read(fd, &oldwealth, sizeof(MONEY));
            close(fd);
//��������屼���ʧ@            unlink(buf2);

            m1 = oldwealth.money;
            m2 = oldwealth.save;
            m3 = oldwealth.request;

            m1 = ((m1+m2)/100000);  //�Цۦ�ק�

            if (m1 >= INT_MAX)
                m1 = INT_MAX;

            sprintf(buf, "%s/" FN_ACCT, str);
            if ((fd = open(buf, O_RDONLY)) < 0)
                continue;

            read(fd, &old, sizeof(OLD));
            close(fd);
            unlink(buf);                    /* itoc.010831: �屼��Ӫ� FN_ACCT */

            trans_acct(&old, &new_);

            fd = open(buf, O_WRONLY | O_CREAT, 0600);       /* itoc.010831: ���طs�� FN_ACCT */
            write(fd, &new_, sizeof(NEW));
            close(fd);

        }
        printf("/usr/%c is done.\n", c);

        closedir(dirp);
    }

    return 0;
}
