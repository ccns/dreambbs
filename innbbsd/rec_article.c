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
     收到之文章內容和檔頭分別在
     內文 (body)   在 char *BODY
     檔頭 (header) 在 char *SUBJECT, *FROM, *SITE, *DATE, *PATH, *GROUP, *MSGID, *POSTHOST, *CONTROL;
#endif


#include "innbbsconf.h"
#include "bbslib.h"
#include "inntobbs.h"
#include "nocem.h"
#include <time.h>
#include <sys/time.h>

/* ----------------------------------------------------- */
/* board：shm 部份須與 cache.c 相容                      */
/* ----------------------------------------------------- */


BCACHE *bshm;


/* ----------------------------------------------------- */
/* 處理 DATE                                             */
/* ----------------------------------------------------- */


#if 0   /* itoc.030303.註解: RFC 822 的 DATE 欄位；RFC 1123 將 year 改成 4-DIGIT */
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
parse_date(void)        /* 把符合 "dd mmm yyyy hh:mm:ss" 的格式，轉成 time_t */
{
    static const char months[12][4] = {"jan", "feb", "mar", "apr", "may", "jun", "jul", "aug", "sep", "oct", "nov", "dec"};
    int i;
    char *ptr, *str, buf[80];
    struct tm ptime;

    str_scpy(buf, DATE, sizeof(buf));
    str_lower(buf, buf);                        /* 通通換小寫，因為 Dec DEC dec 各種都有人用 */

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

        /* RFC 822 允許 mday 是 1- 或 2- DIGIT */
        /* IID.20200108: Spaces may not present after the comma; */
        /*    Find the comma and then let `atoi()` skip the initial spaces */
        const char *const wday_comma = (const char *)memchr(buf, ',', ptr-buf);
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
        ptime.tm_zone = "GMT";
        ptime.tm_gmtoff = 0;

        datevalue = mktime(&ptime);
        /* 如果有 +0100 或 -1000 等註明時區，先調回 GMT 時區 */
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
        /* 如果分析失敗，那麼拿現在時間來當發文時間 */
        time(&datevalue);
        /* bbslog("<rec_article> :Warn: parse_date 錯誤：%s\n", DATE); */
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

    /* 寫入文章內容 */

    brd_fpath(folder, board, FN_DIR);

    if ((fp = fdopen(hdr_stamp(folder, 'A', &hdr, fpath), "w")))
    {
        fprintf(fp, "發信人: %.50s 看板: %s\n", FROM, board);
        fprintf(fp, "標  題: %.70s\n", SUBJECT);
        fprintf(fp, "發信站: %.27s (%.40s)\n", SITE, DATE);
        fprintf(fp, "轉信站: %.70s\n", PATH);
        fprintf(fp, "\n");

        /* chuan: header 跟 body 要空行隔開 */

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

    /* 造 HDR */

    hdr.xmode = POST_INCOME;

    /* Thor.980825: 防止字串太長蓋過頭 */
    str_scpy(hdr.owner, addr, sizeof(hdr.owner));
    str_scpy(hdr.nick, nick, sizeof(hdr.nick));
    str_stamp(hdr.date, &datevalue);    /* 依 DATE: 欄位的日期，與 hdr.chrono 不同步 */
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

    /* 直接複製 trailing data */

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
    /* Thor.981205: 用 fcntl 取代flock, POSIX標準用法 */
    f_exlock(fd);

    fstat(fd, &st);
    size = sizeof(HDR);
    ent = ((long) st.st_size) / size;

    /* itoc.030307.註解: 去 .DIR 中藉由比對 chrono 找出是哪一篇 */

    while (1)
    {
        /* itoc.030307.註解: 每 16 篇為一個 block */
        ent -= 16;
        if (ent <= 0)
            break;

        lseek(fd, size * ent, SEEK_SET);
        if (read(fd, &hdr, size) != size)
            break;

        if (hdr.chrono <= chrono)       /* 落在這個 block 裡 */
        {
            do
            {
                if (hdr.chrono == chrono)
                {
                    /* Thor.981014: mark 的文章不被 cancel */
                    if (hdr.xmode & POST_MARKED)
                        break;

#ifdef KEEP_CANCEL
                    /* itoc.030613: 保留被 cancel 的文章於 [deleted] */
                    move_post(&hdr, BN_DELETED, fpath);
#else
                    unlink(fpath);
#endif

                    update_btime(board);

                    /* itoc.030307: 被 cancel 的文章不保留 header */

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
    /* Thor.981205: 用 fcntl 取代flock, POSIX標準用法 */
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

    sprintf(fpath, "brd/%s/%c/%s", board, xname[7], xname);     /* 去找出那篇文章 */

    /* XLOG("cancel fpath (%s)\n", fpath); */

    if ((fd = open(fpath, O_RDONLY)) >= 0)
    {
        int len;

        len = read(fd, buffer, sizeof(buffer));
        close(fd);

        /* Thor.981221.註解: 外來文章才能被 cancel */
        if ((len > 10) && !strncmp(buffer, "發信人: ", 8))
        {
            char *xfrom, *str;

            xfrom = buffer + 8;
            if ((str = strchr(xfrom, ' ')))
            {
                *str = '\0';

#ifdef NoCeM
                /* gslin.000607: ncm_issuer 可以砍別站發的信 */
                if (strcmp(xfrom, cancelfrom) && !search_issuer(FROM, NULL))
#else
                if (strcmp(xfrom, cancelfrom))
#endif
                {
                    /* itoc.030107.註解: 若 cancelfrom 和本地文章 header 記錄的 xfrom 不同，就是 fake cancel */
                    bbslog("<rec_article> :Warn: 無效的 cancel：%s, sender: %s, path: %s\n", xfrom, FROM, PATH);
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


static bool             /* 1: 符合擋信規則 */
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
        else if (xmode & INN_SPAMSITE && SITE)          /* SITE 可以是 NULL */
            compare = SITE;
        else if (xmode & INN_SPAMPOSTHOST && POSTHOST)  /* POSTHOST 可以是 NULL */
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

        if (firstboard) /* opus: 第一個板才需要處理 */
        {
            /* Thor.980825: gc patch: lib/str_decode 只能接受 decode 完 strlen < 256 */
            /* IID.2020-12-14: `mmdecode_str` now support strings of arbitrary length */

            str_scpy(poolx, SUBJECT, sizeof(poolx));
            mmdecode_str(poolx);
            str_ansi(mysubject, poolx, 70);     /* 70 是 bbspost_add() 標題所需的長度 */
            SUBJECT = mysubject;

            str_scpy(poolx, FROM, sizeof(poolx));
            mmdecode_str(poolx);
            str_ansi(myfrom, poolx, 128);       /* 雖然 bbspost_add() 發信人所需的長度只需要 50，但是 from_parse() 需要長一些 */
            FROM = myfrom;

            str_scpy(poolx, PATH, sizeof(poolx));
            mmdecode_str(poolx);
            str_ansi(mypath, poolx, 128);
            sprintf(mypath, "%s!%.*s", MYBBSID, INT(sizeof(mypath) - strlen(MYBBSID) - 2), PATH);
            /* itoc.030115.註解: PATH 如果有 .edu.tw 就截掉 */
            for (pathptr = mypath; (pathptr = strstr(pathptr, ".edu.tw"));)
                memmove(pathptr, pathptr + 7, strlen(pathptr + 7) + 1);
            mypath[70] = '\0';
            PATH = mypath;

            /* itoc.030218.註解: 處理「發信站」中的時間 */
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
