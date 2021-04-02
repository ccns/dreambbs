/*-------------------------------------------------------*/
/* util/account.c       ( NTHU CS MapleBBS Ver 3.00 )    */
/*-------------------------------------------------------*/
/* target : �W���H���έp�B�t�θ�Ƴƥ��B�}��             */
/* create : 95/03/29                                     */
/* update : 97/03/29                                     */
/*-------------------------------------------------------*/
/* syntax : ���{���y�H crontab ����A�]�w�ɶ����C�p��    */
/* 1-5 �� ����                                           */
/*-------------------------------------------------------*/
/* notice : brdshm (board shared memory) synchronize     */
/*-------------------------------------------------------*/

#include "bbs.h"

#define MAX_LINE        16
#define ADJUST_M        60      /* adjust back 1 hour */

static const char fn_today[] = "gem/@/@-act"; /* ����W���H���έp */
static const char fn_yesterday[] = "gem/@/@=act";     /* �Q��W���H���έp */

/* ----------------------------------------------------- */
/* �}���Gshm �������P cache.c �ۮe                       */
/* ----------------------------------------------------- */

static BCACHE *bshm;


struct Tchoice
{
    int count;
    vitem_t vitem;
};


static int
TchoiceCompare(const void * i, const void * j)
{
    return ((const struct Tchoice *)j)->count - ((const struct Tchoice *)i)->count;
}


static int
draw_vote(
    const BRD *bdh,             /* Thor: �ǤJ BRD, �i�d battr */
    char *fpath,
    const VCH *vch)
{
    FILE *fp;
    char *fname, buf[80], bpath[80];
    const char *bid;
    struct Tchoice choice[MAX_CHOICES];
    int total, items, num, fd, ticket, bollt, *list, *head, *tail;
    struct stat st;

    bid = bdh->brdname;
    fname = strrchr(fpath, '@');

    /* vote item */

    *fname = 'I';
    items = 0;
    if ((fp = fopen(fpath, "r")))
    {
        while (fread(choice[items].vitem, sizeof(vitem_t), 1, fp) == 1)
        {
            choice[items].count = 0;
            items++;
        }
        fclose(fp);
        unlink(fpath);
    }

    if (items == 0)
        return 0;

    /* vote ballots */

    *fname = 'V';

    if ((fd = open(fpath, O_RDONLY)) < 0)
        return 0;

    if (fstat(fd, &st) || (total = st.st_size) <= 0)
    {
        close(fd);
        unlink(fpath);
        return 0;
    }

    /* �֭p�벼���G */

    bollt = 0;                  /* Thor: �`�����k0 */

    list = (int *) malloc(total);
    total = read(fd, list, total);
    close(fd);
    unlink(fpath);

    if (total <= 0)
    {
        free(list);
        return 0;
    }

    head = list;
    tail = (int *) ((char *) list + total);

    do
    {
        for (ticket = *head++, num = 0; ticket && num < items; ticket >>= 1, num++)
        {
            if (ticket & 1)
            {
                choice[num].count++;
                bollt++;
            }
        }
    } while (head < tail);

    free(list);

    /* ���Ͷ}�����G */

    *fname = 'Z';
    if ((fp = fopen(fpath, "w")) == NULL)
        return 0;

    memset(buf, '-', 74);
    buf[74] = '\0';
    fprintf(fp, "\n\n> %s <\n\n�� [%s] �ݪO�벼�G%s\n\n�|��H  �G%s\n\n�|�����G%s\n",
        buf, bid, vch->title, vch->owner, ctime_any(&vch->chrono));
    fprintf(fp, "�}������G%s\n�� �벼�D�D:\n\n", ctime_any(&vch->vclose));

    *fname = '@';
    f_suck(fp, fpath);
    unlink(fpath);

    fprintf(fp, "�� �벼���G�G�C�H�i�� %d ���A�@ %zu �H�ѥ[�A��X %d ��\n\n",
        vch->maxblt, total / sizeof(int), bollt);

    if (vch->vsort == 's')
        qsort(choice, items, sizeof(struct Tchoice), (int (*)(const void *lhs, const void *rhs))TchoiceCompare);

    if (vch->vpercent == '%')
        fd = BMAX((size_t)1, total / sizeof(int));
    else
        fd = 0;

    for (num = 0; num < items; num++)
    {
        ticket = choice[num].count;
        if (fd)
            fprintf(fp, "\t%-42s%3d �� (%4.1f%%)\n",
                (char *)choice[num].vitem, ticket, 100.0 * ticket / fd);
        else
            fprintf(fp, "\t%-42s%3d ��\n",
                (char *)choice[num].vitem, ticket);
    }

    /* other opinions */

    *fname = 'O';
    fputs("\n�� �ڦ��ܭn���G\n\n", fp);
    f_suck(fp, fpath);
    fclose(fp);

    fp = fopen(fpath, "w");     /* Thor: �� O_ �Ȧs�@�U�U... */
    *fname = 'Z';
    f_suck(fp, fpath);
    sprintf(bpath, "brd/%s/@/@vote", bid);
    f_suck(fp, bpath);
    fclose(fp);
    *fname = 'O';
    rename(fpath, bpath);

    /* �N�}�����G post �� [system] �P ���ݪO */
    *fname = 'S';
    unlink(fpath);
    *fname = 'E';
    unlink(fpath);

    *fname = 'Z';

    /* Thor: �Y�ݪ��ݩʬ� BRD_NOVOTE, �h��post�� sysop */

    if (!(bdh->battr & BRD_NOVOTE))
    {
        sprintf(buf, "[�O��] %s <<�ݪO�ﱡ����>>", bid);
        keeplog(fpath, NULL, buf, 0);
    }

    keeplog(fpath, bid, "[�O��] �ﱡ����", 2);

    return 1;
}


static int
draw_board(
    BRD *brd)
{
    int bvote, fd, fsize, alive;
    VCH *vch, *head, *tail;
    time_t now;
    char *fname, fpath[80], buf[80];
    struct stat st;

    /* �� account �� maintain brd->bvote */

    bvote = brd->bvote;

    sprintf(fpath, "brd/%s/.VCH", brd->brdname);

    if ((fd = open(fpath, O_RDWR | O_APPEND, 0600)) < 0
        || fstat(fd, &st)
        || (fsize = st.st_size) <= 0)
    {
        if (fd >= 0)
        {
            close(fd);
            unlink(fpath);
        }
        brd->bvote = 0;
        return bvote ? -1 : 0;
    }

    vch = (VCH *) malloc(fsize);

    /* flock(fd, LOCK_EX); */
    /* Thor.981205: �� fcntl ���Nflock, POSIX�зǥΪk */
    f_exlock(fd);

    read(fd, vch, fsize);

    strcpy(buf, fpath);
    fname = strrchr(buf, '.');
    *fname++ = '@';
    *fname++ = '/';

    head = vch;
    tail = (VCH *) ((char *) vch + fsize);

    alive = 0;

    now = time(NULL);

    do
    {
        if (head->vclose < now)
        {
            strcpy(fname, head->xname);
            draw_vote(brd, buf, head);/* Thor: �ǤJ BRD, �i�d battr */
            head->chrono = 0;
        }
        else
        {
            alive++;
        }
    } while (++head < tail);


    if (alive && alive != fsize / sizeof(VCH))
    {
        ftruncate(fd, 0);
        head = vch;
        do
        {
            if (head->chrono)
            {
                write(fd, head, sizeof(VCH));
            }
        } while (++head < tail);
    }

    /* flock(fd, LOCK_UN);  */
    /* Thor.981205: �� fcntl ���Nflock, POSIX�зǥΪk */
    f_unlock(fd);

    close(fd);
    if (!alive)
        unlink(fpath);

    free(vch);

    if (alive != bvote)
    {
        brd->bvote = alive;
        return 1;
    }

    return 0;
}


static void
closepolls(void)
{
    BRD *bcache, *head, *tail;
    int state;

    state = 0;

    head = bcache = bshm->bcache;
    tail = head + bshm->number;
    do
    {
        state |= draw_board(head);
    } while (++head < tail);

    if (!state)
        return;

    /* write back the shm cache data */

    if ((state = open(FN_BRD, O_WRONLY | O_CREAT, 0600)) < 0)
        return;

    /* flock(state, LOCK_EX); */
    /* Thor.981205: �� fcntl ���Nflock, POSIX�зǥΪk */
    f_exlock(state);

    write(state, bcache, (char *) tail - (char *) bcache);

    /* flock(state, LOCK_UN); */
    /* Thor.981205: �� fcntl ���Nflock, POSIX�зǥΪk */
    f_unlock(state);

    close(state);
    time32(&bshm->uptime);
}


static void
ansi_puts(
    FILE *fp,
    char buf[], char mode)
{
    static char state = '0';

    if (state != mode)
        fprintf(fp, "\x1b[3%cm", state = mode);
    if (buf[0])
    {
        fprintf(fp, "%s", buf);
        buf[0] = 0;
    }
}


static void
gzip(
    const char *source, const char *target, char *stamp)
{
    char buf[128];

    sprintf(buf, "log/%s%s", target, stamp);
    /* rename(source, buf); */
    f_mv(source, &buf[4]); /* Thor.990409: �i�� partition */
    PROC_CMD("/bin/gzip", "-n", buf);
}


#if 0  // Unused
static void
gtar(
    const char *source, const char *target, const char *stamp,
    int prune)
{
    char buf[128];

    sprintf(buf, "log/%s%s.tgz", target, stamp);
    PROC_CMD("/bin/tar", "cfz", buf, source);

    if (prune)
    {
        f_rm(source);
    }
}
#endif

static void
error(
    const char *fpath)
{
    printf("can not open [%s]\n", fpath);
    exit(1);
}


int
main(void)
{
    int fact, hour, max, item, total, i, j, over;
    char date[16];
    char title[80];

    static const char act_file[] = "run/var/act";
    static const char run_file[] = FN_USIES;
    static const char tmp_file[] = "run/tmp";
    static const char log_file[] = "run/usies=";
//  static const char brd_file[] = FN_BRD_USIES;

    char buf[256], ymd[16];
    FILE *fp, *fpw;

    int act[26];                        /* ����/�֭p�ɶ� */
    time_t now;
    struct tm ntime, ptime, *xtime;

    now = time(NULL);
    xtime = localtime(&now);
    ntime = *xtime;

    now -= ADJUST_M * 60;               /* back to ancient */
    xtime = localtime(&now);
    ptime = *xtime;

    setgid(BBSGID);
    setuid(BBSUID);
    chdir(BBSHOME);
    umask(077);

    /* --------------------------------------------------- */
    /* �W���H���έp                                        */
    /* --------------------------------------------------- */

    memset(act, 0, sizeof(act));
    fact = open(act_file, O_RDWR | O_CREAT, 0600);
    if (fact < 0)
        error(act_file);

    if (ptime.tm_hour != 0)
    {
        read(fact, act, sizeof(act));
        lseek(fact, 0, SEEK_SET);
    }

    if (rename(run_file, tmp_file))
    {
        PROC_CMD("bin/touch", tmp_file);
    }
    if ((fp = fopen(tmp_file, "r")) == NULL)
        error(tmp_file);

    if ((fpw = fopen(log_file, "a")) == NULL)
        error(log_file);

    while (fgets(buf, sizeof(buf), fp))
    {
        fputs(buf, fpw);

        if (!memcmp(buf + 22, "ENTER", 5))
        {
            hour = atoi(buf + 9);
            if (hour >= 0 && hour <= 23)
                act[hour]++;
            continue;
        }

        if (!memcmp(buf + 41, "Stay:", 5))
        {
            if ((hour = atoi(buf + 47)))
            {
                act[24] += hour;
                act[25]++;
            }
            continue;
        }
    }
    fclose(fp);
    fclose(fpw);
    unlink(tmp_file);

    // IID.20190311: Zero-out the file `fact` to reset the login count at 0 am.
    if (ntime.tm_hour == 0)
        for (i = 0; i < sizeof(act); i++)
            write(fact, "", sizeof(char));
    else
        write(fact, act, sizeof(act));

    close(fact);

    for (i = max = total = 0; i < 24; i++)
    {
        total += act[i];
        if (act[i] > max)
            max = act[i];
    }

    item = max / MAX_LINE + 1;
    over = max > 1000;

    if (!ptime.tm_hour)
        keeplog(fn_today, NULL, "[�O��] �W���H���έp", 1);

    if ((fp = fopen(fn_today, "w")) == NULL)
        error(fn_today);

    /* Thor.990329: y2k */
    fprintf(fp, "\t\t\t   \x1b[1;33;46m [%02d/%02d/%02d] �W���H���έp \x1b[40m\n",
        ptime.tm_year % 100, ptime.tm_mon + 1, ptime.tm_mday);
    for (i = MAX_LINE + 1; i > 0; i--)
    {
        strcpy(buf, "   ");
        for (j = 0; j < 24; j++)
        {
            max = item * i;
            hour = act[j];
            if (hour && (max > hour) && (max - item <= hour))
            {
                ansi_puts(fp, buf, '3');
                if (over)
                    hour = (hour + 5) / 10;
                fprintf(fp, "%-3d", hour);
            }
            else if (max <= hour)
            {
                ansi_puts(fp, buf, '1');
                fprintf(fp, "�i ");
            }
            else
                strcat(buf, "   ");
        }
        fprintf(fp, "\n");
    }

    if (act[25] == 0) act[25]=1; /* Thor.980928: lkchu patch: ����Ƭ�0 */

    fprintf(fp, "\x1b[34m"
        "  �梤����������������������������������������������������������������������\n  \x1b[32m"
        "0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23\n\n"
        "\t%s\t\x1b[35m�`�@�W���H���G\x1b[37m%-9d\x1b[35m�����ϥήɶ��G\x1b[37m%d\x1b[m\n",
        over ? "\x1b[35m���G\x1b[37m10 �H" : "", total, act[24] / act[25] + 1);
    fclose(fp);

    /* --------------------------------------------------- */
    /* Load board shm                                      */
    /* --------------------------------------------------- */

    shm_logger_init(NULL);
    bshm_init(&bshm);

    /* --------------------------------------------------- */
    /* �t�ζ}��                                            */
    /* --------------------------------------------------- */

    closepolls();

    /* --------------------------------------------------- */
    /* ������Y�ƥ��B�������D�έp                          */
    /* --------------------------------------------------- */

    /* Thor.990329: y2k */
    sprintf(ymd, "-%02d%02d%02d",
        ptime.tm_year % 100, ptime.tm_mon + 1, ptime.tm_mday);

    /* if (ptime.tm_hour == 23) */
    if (ntime.tm_hour == 0)
    {

        sprintf(date, "[ %02d �� %02d ��] ", ptime.tm_mon + 1, ptime.tm_mday);

        /* �H�U�O�ثe�S���b�ϥΪ����� */

//        sprintf(title, "[�O��] %s�ϥΦ��Ʋέp", date);
//        PROC_CMD("bin/sh", BINARY_SUFFIX"spss.sh");
//        keeplog("run/spss.log", NULL, title, 2);

//        sprintf(title, "[�O��] %s�����\\Ū���Ʋέp", date);
//        PROC_CMD("bin/sh", BINARY_SUFFIX"brd_usies.sh");
//        keeplog(FN_BRD_USIES".log", NULL, title, 2); /* ��z�L�᪺ log */
//        keeplog(FN_BRD_USIES, BRD_SECRET, title, 2); /* ����z�e�� log */
//        gzip(FN_BRD_USIES, "brdusies/brdusies", ymd);   /* ����z�e�� log */

//        sprintf(title, "[�O��] %s�ק�峹����", date);
//        keeplog(FN_POSTEDIT_LOG, BRD_SECRET, title, 2);

//        sprintf(title, "[�O��] %sbbsmail mailpost����", date);
//        keeplog(FN_BBSMAILPOST_LOG, BRD_SECRET, title, 2);

//        sprintf(title, "[�O��] %s�׫H����", date);
//        keeplog(FN_BANMAIL_LOG, NULL, title, 2);
//        gzip(FN_BANMAIL_LOG, "banmail/banmail", ymd);  /* �׫H���� */

//        sprintf(title, "[�O��] %s�H�H����", date);
//        keeplog(FN_MAIL_LOG, BRD_SECRET, title, 2);

//        sprintf(title, "[�O��] %s��H����", date);
//        keeplog(FN_INNBBS_LOG, BRD_SECRET, title, 2);
//        gzip(FN_INNBBS_LOG, "innbbsd/innbbsd", ymd);  /* ��H���� */


//        sprintf(title, "[�O��] %s�ݪ��H��R������", date);
//        keeplog(FN_EXPIRE_LOG, BRD_SECRET, title, 2);
//        gzip(FN_EXPIRE_LOG, "expire/expire", ymd);  /* �ݪ��H��R������ */

//        sprintf(title, "%sE-Mail �y�q�έp", date);
//        keeplog("run/unlog.log", NULL, title, 2);

        /* Thor.990403: �έpOVER */
//        system("grep OVER run/bmta.log | cut -f2 | cut -d' ' -f2- | sort | uniq -c > run/over.log");
//        sprintf(title, "%sE-Mail over max connection �έp", date);
//        keeplog("run/over.log", NULL, title, 2);


        /* �H�U�O���K���� */
        sprintf(title, "[�O��] %s�峹�[�ݬ���", date);
        keeplog(FN_BROWSE_LOG, BRD_SECRET, title, 3);

        sprintf(title, "[�O��] %s�ΦW�O����", date);
        keeplog(FN_ANONYMOUS_LOG, BRD_SECRET, title, 3);

        /*  �ѩ�@���B��}�ɪ��A, �O�G�� log by statue
            �ק� SIG_USR1 �i�N log dump �X�� by visor */
        system("kill -USR1 `ps -auxwww | grep xchatd | awk '{print $2}'`");
        sprintf(title, "[�O��] %s��ѫǶi�X����", date);
        keeplog(FN_CHAT_LOG".old", BRD_SECRET, title, 2);

        sprintf(title, "[�O��] %spop3�{�Ҭ���", date);
        keeplog(FN_VERIFY_LOG, BRD_SECRET, title, 2);

        /* �ѩ� FN_POP3_LOG �@���B��}�ɪ��A, �O�G���N�� pop3d ����
           �ɮצ۵M�N�|�g�J, ���� log �~�|���ɮ� by statue */
        sprintf(title, "[�O��] %sPOP3D����", date);
        keeplog(FN_POP3_LOG, BRD_SECRET, title, 2);

        sprintf(title, "[�O��] %s���� E-mail �{�Ҭ���", date);
        keeplog(FN_EMAILADDR_LOG, BRD_SECRET, title, 2);

        sprintf(title, "[�O��] %s���Ȧ欰����", date);
        keeplog(FN_BLACKSU_LOG, BRD_SECRET, title, 2);

        sprintf(title, "[�O��] %s�I�q����", date);
        keeplog(FN_SONG_LOG, BRD_SECRET, title, 2);

        sprintf(title, "[�O��] %s���ˤ峹����", date);
        keeplog(FN_RECOMMEND_LOG, BRD_SECRET, title, 2);

        sprintf(title, "[�O��] %s�Ȧ�������", date);
        keeplog(FN_BANK, BRD_SECRET, title, 2);

        sprintf(title, "[�O��] %s�ө��������", date);
        keeplog(FN_SHOP, BRD_SECRET, title, 2);

        system("cat run/usies= | grep APPLY > run/apply.log");
        sprintf(title, "[�O��] %s�C����U�ϥΪ̬���", date);
        keeplog("run/apply.log", BRD_SECRET, title, 2);

        sprintf(title, "[�O��] %s���ȦC��", date);
        keeplog(FN_MANAGER_LOG, BRD_SECRET, title, 2);

        sprintf(title, "[�O��] %s���v�C��", date);
        keeplog(FN_CRIMINAL_LOG, BRD_SECRET, title, 2);

        sprintf(title, "[�O��] %s�O�D�C��", date);
        keeplog(FN_BMLIST_LOG, BRD_SECRET, title, 2);

        sprintf(title, "[�O��] %s�v���ק����", date);
        keeplog(FN_SECURITY, BRD_SECRET, title, 2);

        sprintf(title, "[�O��] %s��ذϯ��ެ���", date);
        keeplog(FN_GINDEX_LOG, BRD_SECRET, title, 2);

        sprintf(title, "[�O��] %s��ذ��ˬd����", date);
        keeplog(FN_GCHECK_LOG, BRD_SECRET, title, 2);

        sprintf(title, "[�O��] %s�ݪO�H���ʧR������", date);
        keeplog(FN_EXPIRED_LOG, BRD_SECRET, title, 2);

        sprintf(title, "[�O��] %s�ϥΪ̿� ACCT ����", date);
        keeplog("run/NOACCT.log", BRD_SECRET, title, 2);

        sprintf(title, "[�O��] %s�ϥΪ̿� .DIR ����", date);
        keeplog("run/NOUSRDIR.log", BRD_SECRET, title, 2);

        sprintf(title, "[�O��] %s�ݪO�� .DIR ����", date);
        keeplog("run/NOBRDDIR.log", BRD_SECRET, title, 2);

#ifdef  HAVE_FAVORITE
        sprintf(title, "[�O��] %s�ڪ��̷R����", date);
        keeplog(FN_FAVORITE_LOG, BRD_SECRET, title, 2);
#endif

        /* �H�U�O���}���� */

        //sprintf(date, "[%2d �� %2d ��] ", ptime.tm_mon + 1, ptime.tm_mday);
        sprintf(title, "[�O��] %s�峹�g�Ʋέp", date);
        keeplog(FN_POST_LOG, NULL, title, 2);

        sprintf(title, "[�O��] %s�p����������", date);
        keeplog(FN_PIPMONEY_LOG, NULL, title, 2);

        sprintf(title, "[�O��] %s" BOARDNAME "�d����", date);
        keeplog(FN_PIP_LOG, NULL, title, 2);

        sprintf(title, "[�O��] %s" BOARDNAME "��a�p", date);
        keeplog(FN_MINE_LOG, NULL, title, 2);

        sprintf(title, "[�O��] %s�Ĳ��W���d���O", date);
        keeplog(FN_NOTE_ALL, NULL, title, 2);

        sprintf(title, "[�O��] %s�����b������", date);
        keeplog(FN_REAPER_LOG, NULL, title, 2);

        sprintf(title, "[�O��] %s���i�O�D����", date);
        keeplog(FN_LAZYBM_LOG, NULL, title, 2);

        sprintf(title, "[�O��] %s����Q�j�������D", date);
        keeplog("gem/@/@-day", NULL, title, 0);

        if ((fp = fopen(fn_yesterday, "w")))
        {
            f_suck(fp, fn_today);
            fclose(fp);
        }

        if (ntime.tm_wday == 0)
        {
            sprintf(title, "[�O��] %s���g���Q�j�������D", date);
            keeplog("gem/@/@-week", NULL, title, 0);
        }

        if (ntime.tm_mday == 1)
        {
            sprintf(title, "[�O��] %s����ʤj�������D", date);
            keeplog("gem/@/@-month", NULL, title, 0);
        }

        if (ntime.tm_yday == 1)
        {
            sprintf(title, "[�O��] %s�~�צʤj�������D", date);
            keeplog("gem/@/@-year", NULL, title, 0);
        }

        gzip(log_file, "usies/usies", ymd);

        gzip(FN_CHATDATA_LOG".old", "chat/chat", ymd);

//#ifdef  LOG_BRD_USIES
//      gzip(brd_file, FN_BRD_USIES, ymd);   /* lkchu.981201: �ƥ��ݪ��\Ū�O�� */
//#endif

    }
    else if (ntime.tm_hour == 1)
    {
        sprintf(date, "[%02d �� %02d ��] ", ptime.tm_mon + 1, ptime.tm_mday);

        sprintf(title, "[�O��] %s�ϥΪ̽s������", date);
        PROC_CMD(BINARY_SUFFIX"userno", NULL);
        keeplog(FN_USERNO_LOG, BRD_SECRET, title, 2);
        gzip(FN_USERNO_LOG, "userno/userno", ymd);        /* �Ҧ� [�ϥΪ̽s������] �O�� */
        gzip(FN_MAIL_LOG, "mail/mail", ymd);    /* �Ҧ� [�H�H] �O�� */


        if (ntime.tm_wday == 6)
        {
//��091205����
//          gtar("usr/@", "reaper/reaper", ymd, 1);
        }
    }

#ifdef HAVE_SIGNED_MAIL
    srandom(time(NULL));
#if (PRIVATE_KEY_PERIOD == 0)
    if (!dashf(PRIVATE_KEY))
#else
    if (!dashf(PRIVATE_KEY) || (random() % PRIVATE_KEY_PERIOD) == 0)
#endif
    {
        sprintf(title, "log/prikey%s", ymd);
        f_mv(PRIVATE_KEY, title);
        i = PLAINPASSSIZE-1;
        for (;;)
        {
            j = random() % 0x100U;
            if (!j) continue;
            title[--i] = j;
            if (i == 0) break;
        }
        rec_add(PRIVATE_KEY, title, PLAINPASSSIZE-1);
    }
#endif  /* #ifdef HAVE_SIGNED_MAIL */

    exit(0);
}

void
keeplog(
    const char *fnlog,
    const char *board,
    const char *title,
    int mode)           /* 0:load 1: rename  2:unlink 3:mark*/
{
    HDR hdr;
    char folder[128], fpath[128];
    int fd;
    FILE *fp;

    if (!board)
        board = BRD_SYSTEM;

    sprintf(folder, "brd/%s/.DIR", board);
    fd = hdr_stamp(folder, 'A', &hdr, fpath);
    if (fd < 0)
        return;

    if (mode == 1 || mode == 3)
    {
        close(fd);
        /* rename(fnlog, fpath); */
        f_mv(fnlog, fpath); /* Thor.990409:�i��partition */
    }
    else
    {
        fp = fdopen(fd, "w");
        fprintf(fp, "�@��: SYSOP (" SYSOPNICK ")\n���D: %s\n�ɶ�: %s\n",
            title, ctime_any(&hdr.chrono));
        f_suck(fp, fnlog);
        fclose(fp);
        if (mode)
            unlink(fnlog);
    }
    if (mode == 3)
        hdr.xmode |= POST_MARKED;

    strcpy(hdr.title, title);
    strcpy(hdr.owner, "SYSOP");
    strcpy(hdr.nick, SYSOPNICK);
    fd = open(folder, O_WRONLY | O_CREAT | O_APPEND, 0600);
    if (fd < 0)
    {
        unlink(fpath);
        return;
    }
    write(fd, &hdr, sizeof(HDR));
    close(fd);
}

