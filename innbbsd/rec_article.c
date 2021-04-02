/*-------------------------------------------------------*/
/* rec_article.c( NTHU CS MapleBBS Ver 3.10 )            */
/*-------------------------------------------------------*/
/* target : innbbsd receive article                      */
/* create : 95/04/27                                     */
/* update :   /  /                                       */
/* author : skhuang@csie.nctu.edu.tw                     */
/* modify : itoc.bbs@bbs.tnfsh.tn.edu.tw                 */
/*-------------------------------------------------------*/


#if 0
     ���줧�峹���e�M���Y���O�b
     ���� (body)   �b char *BODY
     ���Y (header) �b char *SUBJECT, *FROM, *SITE, *DATE, *PATH, *GROUP, *MSGID, *POSTHOST, *CONTROL;
#endif


#include "innbbsconf.h"
#include "bbslib.h"
#include "inntobbs.h"
#include "nocem.h"
#include <time.h>
#include <sys/time.h>

/* ----------------------------------------------------- */
/* board�Gshm �������P cache.c �ۮe                      */
/* ----------------------------------------------------- */


static BCACHE *bshm;


void
init_bshm(void)
{
    /* itoc.030727: �b�}�� bbsd ���e�A���ӴN�n����L account�A
       �ҥH bshm ���Ӥw�]�w�n */

    bshm = (BCACHE *) shm_new(BRDSHM_KEY, sizeof(BCACHE));

    if (bshm->uptime <= 0)      /* bshm ���]�w���� */
        exit(0);
}


/* ----------------------------------------------------- */
/* �B�z DATE                                             */
/* ----------------------------------------------------- */


#if 0   /* itoc.030303.����: RFC 822 �� DATE ���FRFC 1123 �N year �令 4-DIGIT */
/* https://tools.ietf.org/html/rfc822  5.  DATE AND TIME SPECIFICATION*/
/* https://tools.ietf.org/html/rfc1123 5.2.14  RFC-822 Date and Time Specification */

date-time = [ day "," ] date time           ; dd mm yy hh:mm:ss zzz
day       = "Mon" / "Tue" / "Wed" / "Thu" / "Fri" / "Sat" / "Sun"

/* RFC-1123: All mail software SHOULD use 4-digit years in dates, [...] */
date      = 1*2DIGIT month 2*4DIGIT         ; day month year e.g. 20 Jun 82

month     = "Jan" / "Feb" / "Mar" / "Apr" / "May" / "Jun" / "Jul" / "Aug" / "Sep" / "Oct" / "Nov" / "Dec"
time      = hour zone                       ; ANSI and Military
hour      = 2DIGIT ":" 2DIGIT [":" 2DIGIT]  ; 00:00:00 - 23:59:59
zone      = "UT"  / "GMT"                   ; Universal Time

                                            ; North American : UT  /* zone: offset */
          / "EST" / "EDT"                   ;  Eastern:  - 5/ - 4
          / "CST" / "CDT"                   ;  Central:  - 6/ - 5
          / "MST" / "MDT"                   ;  Mountain: - 7/ - 6
          / "PST" / "PDT"                   ;  Pacific:  - 8/ - 7

          /* RFC-1123: [...], military time zones in RFC-822 headers carry no information. */
          / 1ALPHA                          ; Military: Z = UT; A:-1; (J not used); M:-12; N:+1; Y:+12

          /* RFC-1123: [...], and implementations SHOULD use numeric timezone instead of timezone names.
                 However, all implementations MUST accept either notation. */
          / ( ("+" / "-") 4DIGIT )          ; Local differential hours+min. (HHMM)
#endif

static time_t datevalue;

static void
parse_date(void)        /* ��ŦX "dd mmm yyyy hh:mm:ss" ���榡�A�ন time_t */
{
    static const char months[12][4] = {"jan", "feb", "mar", "apr", "may", "jun", "jul", "aug", "sep", "oct", "nov", "dec"};
    int i;
    char *ptr, *str, buf[80];
    struct tm ptime;

    str_scpy(buf, DATE, sizeof(buf));
    str_lower(buf, buf);                        /* �q�q���p�g�A�]�� Dec DEC dec �U�س����H�� */

    str = buf + 2;  /* Skip `mday` (assume no `wday`) */
    for (i = 0; i < 12; i++)
    {
        if ((ptr = strstr(str, months[i])))
            break;
    }

    if (ptr)
    {
        /* buf,  buf2,  ,ptr ,ptr+4                      */
        /*   [Thu , ]11 Feb [19]99 06 : 00 [: 37] + 0800 */
        /*       ^ ^ optional spaces ^ ^  ^  ^     ^     */

        /* RFC 822 ���\ mday �O 1- �� 2- DIGIT */
        /* IID.20200108: Spaces may not present after the comma; */
        /*    Find the comma and then let `atoi()` skip the initial spaces */
        const char *const wday_comma = memchr(buf, ',', ptr-buf);
        const char *const mday_start = (wday_comma) ? wday_comma+1 : buf;
        const char *zone;

        /* IID.20200108: `year` can also be 2-digit in RFC-1123 */
        char *year_end;
        str = ptr + 4 + strspn(ptr + 4, " ");  /* Skip spaces */
        ptime.tm_year = strtol(str, &year_end, 10);
        if (year_end - str >= 4)
            ptime.tm_year -= 1900;
        str = year_end;

        ptime.tm_hour = strtol(str+1, &str, 10);
        str += strcspn(str, ":");  /* Seek the next `':'` */
        ptime.tm_min = strtol(str+1, &str, 10);

        /* IID.20200108: `second` is optional */
        str += strspn(str, " ");  /* Skip spaces */
        ptime.tm_sec = (*str == ':') ? strtol(str+1, &str, 10) : 0;

        ptime.tm_mday = atoi(mday_start);
        ptime.tm_mon = i;
        ptime.tm_isdst = 0;
#ifndef __CYGWIN__
        ptime.tm_zone = "GMT";
        ptime.tm_gmtoff = 0;
#endif

        datevalue = mktime(&ptime);
        /* �p�G�� +0100 �� -1000 �������ɰϡA���զ^ GMT �ɰ� */
        zone = str;
        zone += strcspn(zone, "+-");  /* Seek `'+'` or `'-'` */
        if (*zone)
            datevalue -= ((*zone == '-') ? -1 : 1) * ((strtol(zone+1, NULL, 10) / 100) * 3600 + (strtol(zone+1, NULL, 10) % 100) * 60);
        /* Convert it to the local timezone */
        zone = INNBBS_UTCZONE;
        zone += strcspn(zone, "+-");  /* Seek `'+'` or `'-'` */
        if (*zone)
            datevalue += ((*zone == '-') ? -1 : 1) * ((strtol(zone+1, NULL, 10) / 100) * 3600 + (strtol(zone+1, NULL, 10) % 100) * 60);
    }
    else
    {
        /* �p�G���R���ѡA���򮳲{�b�ɶ��ӷ�o��ɶ� */
        time(&datevalue);
        /* bbslog("<rec_article> :Warn: parse_date ���~�G%s\n", DATE); */
    }
}


/* ----------------------------------------------------- */
/* process post write                                    */
/* ----------------------------------------------------- */


static void
update_btime(
    const char *brdname)
{
    BRD *brdp, *bend;

    brdp = bshm->bcache;
    bend = brdp + bshm->number;
    do
    {
        if (!strcmp(brdname, brdp->brdname))
        {
            brdp->btime = -1;
            break;
        }
    } while (++brdp < bend);
}


static void
bbspost_add(
    const char *board, const char *addr, const char *nick)
{
    int cc;
    const char *str;
    char folder[64], fpath[64];
    HDR hdr;
    FILE *fp;

    /* �g�J�峹���e */

    brd_fpath(folder, board, FN_DIR);

    if ((fp = fdopen(hdr_stamp(folder, 'A', &hdr, fpath), "w")))
    {
        fprintf(fp, "�o�H�H: %.50s �ݪO: %s\n", FROM, board);
        fprintf(fp, "��  �D: %.70s\n", SUBJECT);
        fprintf(fp, "�o�H��: %.27s (%.40s)\n", SITE, DATE);
        fprintf(fp, "��H��: %.70s\n", PATH);
        fprintf(fp, "\n");

        /* chuan: header �� body �n�Ŧ�j�} */

        /* fprintf(fp, "%s", BODY); */

        for (str = BODY; (cc = *str); str++)
        {
            if (cc == '.')
            {
                /* for line beginning with a period, collapse the doubled period to a single one. */
                if (str >= BODY + 2 && str[-1] == '.' && str[-2] == '\n')
                    continue;
            }

            fputc(cc, fp);
        }

        fclose(fp);
    }

    /* �y HDR */

    hdr.xmode = POST_INCOME;

    /* Thor.980825: ����r��Ӫ��\�L�Y */
    str_scpy(hdr.owner, addr, sizeof(hdr.owner));
    str_scpy(hdr.nick, nick, sizeof(hdr.nick));
    str_stamp(hdr.date, &datevalue);    /* �� DATE: ��쪺����A�P hdr.chrono ���P�B */
    str_scpy(hdr.title, SUBJECT, sizeof(hdr.title));

    rec_bot(folder, &hdr, sizeof(HDR));

    update_btime(board);

    HISadd(MSGID, board, hdr.xname);
}


/* ----------------------------------------------------- */
/* process cancel write                                  */
/* ----------------------------------------------------- */


#ifdef KEEP_CANCEL
static inline void
move_post(
    HDR *hdr,
    const char *board, const char *filename)
{
    HDR post;
    char folder[64];

    brd_fpath(folder, board, FN_DIR);
    hdr_stamp(folder, HDR_LINK | 'A', &post, filename);
    unlink(filename);

    /* �����ƻs trailing data */

    memcpy(post.owner, hdr->owner, sizeof(HDR) - offsetof(HDR, owner));

    sprintf(post.title, "[cancel] %-60.60s", FROM);

    rec_bot(folder, &post, sizeof(HDR));
}
#endif


static void
bbspost_cancel(
    const char *board,
    time_t chrono,
    const char *fpath)
{
    HDR hdr;
    struct stat st;
    long size;
    int fd, ent;
    char folder[64], *data;
    off_t off, len;

    /* XLOG("cancel [%s] %d\n", board, time); */

    brd_fpath(folder, board, FN_DIR);
    if ((fd = open(folder, O_RDWR)) == -1)
        return;

    /* flock(fd, LOCK_EX); */
    /* Thor.981205: �� fcntl ���Nflock, POSIX�зǥΪk */
    f_exlock(fd);

    fstat(fd, &st);
    size = sizeof(HDR);
    ent = ((long) st.st_size) / size;

    /* itoc.030307.����: �h .DIR ���ǥѤ�� chrono ��X�O���@�g */

    while (1)
    {
        /* itoc.030307.����: �C 16 �g���@�� block */
        ent -= 16;
        if (ent <= 0)
            break;

        lseek(fd, size * ent, SEEK_SET);
        if (read(fd, &hdr, size) != size)
            break;

        if (hdr.chrono <= chrono)       /* ���b�o�� block �� */
        {
            do
            {
                if (hdr.chrono == chrono)
                {
                    /* Thor.981014: mark ���峹���Q cancel */
                    if (hdr.xmode & POST_MARKED)
                        break;

#ifdef KEEP_CANCEL
                    /* itoc.030613: �O�d�Q cancel ���峹�� [deleted] */
                    move_post(&hdr, BN_DELETED, fpath);
#else
                    unlink(fpath);
#endif

                    update_btime(board);

                    /* itoc.030307: �Q cancel ���峹���O�d header */

                    off = lseek(fd, 0, SEEK_CUR);
                    len = st.st_size - off;

                    data = (char *) malloc(len);
                    read(fd, data, len);

                    lseek(fd, off - size, SEEK_SET);
                    write(fd, data, len);
                    ftruncate(fd, st.st_size - size);

                    free(data);
                    break;
                }

                if (hdr.chrono > chrono)
                    break;
            } while (read(fd, &hdr, size) == size);

            break;
        }
    }

    /* flock(fd, LOCK_UN); */
    /* Thor.981205: �� fcntl ���Nflock, POSIX�зǥΪk */
    f_unlock(fd);

    close(fd);
    return;
}


int                     /* 0:cancel success  -1:cancel fail */
cancel_article(
    const char *msgid)
{
    int fd;
    char fpath[64], cancelfrom[128], buffer[128];
    char board[IDLEN + 1], xname[9];

    /* XLOG("cancel %s <%s>\n", FROM, msgid); */

    if (!HISfetch(msgid, board, xname))
        return -1;

    from_parse(FROM, cancelfrom, buffer);

    /* XLOG("cancel %s (%s)\n", cancelfrom, buffer); */

    sprintf(fpath, "brd/%s/%c/%s", board, xname[7], xname);     /* �h��X���g�峹 */

    /* XLOG("cancel fpath (%s)\n", fpath); */

    if ((fd = open(fpath, O_RDONLY)) >= 0)
    {
        int len;

        len = read(fd, buffer, sizeof(buffer));
        close(fd);

        /* Thor.981221.����: �~�Ӥ峹�~��Q cancel */
        if ((len > 10) && !memcmp(buffer, "�o�H�H: ", 8))
        {
            char *xfrom, *str;

            xfrom = buffer + 8;
            if ((str = strchr(xfrom, ' ')))
            {
                *str = '\0';

#ifdef NoCeM
                /* gslin.000607: ncm_issuer �i�H��O���o���H */
                if (strcmp(xfrom, cancelfrom) && !search_issuer(FROM, NULL))
#else
                if (strcmp(xfrom, cancelfrom))
#endif
                {
                    /* itoc.030107.����: �Y cancelfrom �M���a�峹 header �O���� xfrom ���P�A�N�O fake cancel */
                    bbslog("<rec_article> :Warn: �L�Ī� cancel�G%s, sender: %s, path: %s\n", xfrom, FROM, PATH);
                    return -1;
                }

                bbspost_cancel(board, chrono32(xname), fpath);
            }
        }
    }

    return 0;
}


/* ----------------------------------------------------- */
/* check spam rule                                       */
/* ----------------------------------------------------- */


static bool             /* 1: �ŦX�׫H�W�h */
is_spam(
    const char *board, const char *addr, const char *nick)
{
    spamrule_t *spam;
    int i, xmode;
    const char *compare, *detail;

    for (i = 0; i < SPAMCOUNT; i++)
    {
        spam = SPAMRULE + i;

        compare = spam->path;
        if (*compare && strcmp(compare, NODENAME))
            continue;

        compare = spam->board;
        if (*compare && strcmp(compare, board))
            continue;

        xmode = spam->xmode;
        detail = spam->detail;

        if (xmode & INN_SPAMADDR)
            compare = addr;
        else if (xmode & INN_SPAMNICK)
            compare = nick;
        else if (xmode & INN_SPAMSUBJECT)
            compare = SUBJECT;
        else if (xmode & INN_SPAMPATH)
            compare = PATH;
        else if (xmode & INN_SPAMMSGID)
            compare = MSGID;
        else if (xmode & INN_SPAMBODY)
            compare = BODY;
        else if (xmode & INN_SPAMSITE && SITE)          /* SITE �i�H�O NULL */
            compare = SITE;
        else if (xmode & INN_SPAMPOSTHOST && POSTHOST)  /* POSTHOST �i�H�O NULL */
            compare = POSTHOST;
        else
            continue;

        if (str_casestr_dbcs(compare, detail))
            return true;
    }
    return false;
}


/* ----------------------------------------------------- */
/* process receive article                               */
/* ----------------------------------------------------- */


#ifndef NoCeM
static
#endif
newsfeeds_t *
search_newsfeeds_bygroup(
    const char *newsgroup)
{
    newsfeeds_t nf, *find;

    str_scpy(nf.newsgroup, newsgroup, sizeof(nf.newsgroup));
    find = (newsfeeds_t *) bsearch(&nf, NEWSFEEDS_G, NFCOUNT, sizeof(newsfeeds_t), nf_bygroupcmp);
    if (find && !(find->xmode & INN_NOINCOME))
        return find;
    return NULL;
}


int                     /* 0:success  -1:fail */
receive_article(void)
{
    newsfeeds_t *nf;
    char myaddr[128], mynick[128], mysubject[128], myfrom[128], mydate[80];
    char poolx[256];
    char *group;
    char mypath[128], *pathptr;
    int firstboard = 1;

    /* try to split newsgroups into separate group */
    for (group = strtok(GROUP, ","); group; group = strtok(NULL, ","))
    {
        if (!(nf = search_newsfeeds_bygroup(group)))
            continue;

        if (firstboard) /* opus: �Ĥ@�ӪO�~�ݭn�B�z */
        {
            /* Thor.980825: gc patch: lib/str_decode �u�౵�� decode �� strlen < 256 */
            /* IID.2020-12-14: `mmdecode_str` now support strings of arbitrary length */

            str_scpy(poolx, SUBJECT, sizeof(poolx));
            mmdecode_str(poolx);
            str_ansi(mysubject, poolx, 70);     /* 70 �O bbspost_add() ���D�һݪ����� */
            SUBJECT = mysubject;

            str_scpy(poolx, FROM, sizeof(poolx));
            mmdecode_str(poolx);
            str_ansi(myfrom, poolx, 128);       /* ���M bbspost_add() �o�H�H�һݪ����ץu�ݭn 50�A���O from_parse() �ݭn���@�� */
            FROM = myfrom;

            str_scpy(poolx, PATH, sizeof(poolx));
            mmdecode_str(poolx);
            str_ansi(mypath, poolx, 128);
            sprintf(mypath, "%s!%.*s", MYBBSID, (int)(sizeof(mypath) - strlen(MYBBSID) - 2), PATH);
            /* itoc.030115.����: PATH �p�G�� .edu.tw �N�I�� */
            for (pathptr = mypath; (pathptr = strstr(pathptr, ".edu.tw"));)
                memmove(pathptr, pathptr + 7, strlen(pathptr + 7) + 1);
            mypath[70] = '\0';
            PATH = mypath;

            /* itoc.030218.����: �B�z�u�o�H���v�����ɶ� */
            parse_date();
            strcpy(mydate, (char *) Btime(&datevalue));
            DATE = mydate;

            if (*nf->charset == 'g')
            {
                gb2b5(BODY);
                gb2b5(FROM);
                gb2b5(SUBJECT);
                if (SITE)
                    gb2b5(SITE);
            }

            from_parse(FROM, myaddr, mynick);

            if (is_spam(nf->board, myaddr, mynick))
            {
#ifdef KEEP_CANCEL
                bbspost_add(BN_DELETED, myaddr, mynick);
#endif
                break;
            }

            firstboard = 0;
        }

        bbspost_add(nf->board, myaddr, mynick);
    }           /* for board1, board2, ... */

    return 0;
}
