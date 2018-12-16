/*-------------------------------------------------------*/
/* util/redir.c         ( NTHU CS MapleBBS Ver 3.10 )    */
/*-------------------------------------------------------*/
/* target : 自動重建.DIR程式                             */
/* author : Thor.bbs@bbs.cs.nthu.edu.tw                  */
/* create : 99/10/07                                     */
/* update :   /  /                                       */
/*-------------------------------------------------------*/
/* syntax : redir                                        */
/* input  : scan current directory                       */
/* output : generate .DIR.re(for radix32) and .DIR.@     */
/*-------------------------------------------------------*/
/*
   還沒詳細的測試, 大家可以試看看

   此程式可以重建看版, 精華區, 信箱的.DIR
   會scan current directory, 產生 .DIR.re (即 0-9, A-V的目錄下文章)
   和 .DIR.@ (即 @ 下文章)
   會以 chrono來排序
 */
#include "bbs.h"


#define FNAME_DB_SIZE  2048

typedef char FNAME[9];
static FNAME *n_pool;
static int n_size, n_head;

    int
pool_add(
    FNAME fname)
{
    char *p;

    /* initial pool */
    if (!n_pool)
    {
        n_pool = (FNAME *) malloc(FNAME_DB_SIZE * sizeof(FNAME));
        n_size = FNAME_DB_SIZE;
        n_head = 0;
    }

    if (n_head >= n_size)
    {
        n_size += (n_size >> 1);
        n_pool = (FNAME *) realloc(n_pool, n_size * sizeof(FNAME));
    }

    p = n_pool[n_head];

    if (fname[8])
        return -1;          /* too long */

    strcpy(p, fname);

    n_head++;

    return 0;
}

#if 0
/* ----------------------------------------------------- */
/* chrono ==> file name (32-based)                       */
/* 0123456789ABCDEFGHIJKLMNOPQRSTUV                      */
/* ----------------------------------------------------- */

static char radix32[32] = {
    '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
    'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
    'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V',
};
#endif


    HDR *
article_parse(
    FILE *fp)
{
    static HDR hdr;
    char buf[256], *ptr, *ptr2, *ptr3;
    int local;

    if (!fgets(buf, sizeof(buf), fp))
        return NULL;

    if ((ptr2 = strrchr(buf, '\n')))
        *ptr2 = '\0';

    memset(&hdr, 0, sizeof hdr);

    local = 0;
    if ((ptr = strstr(buf, STR_POST1))) /* 看版 */
    {
        local = 0;
    }
    else if ((ptr = strstr(buf, STR_POST2)))    /* 站內 */
    {
        local = 1;
    }

    if (ptr)
    {
        ptr[-1] = '\0';
        if (ptr[-2] == ',')
        {
            ptr[-2] = '\0';
        }
    }

    /* hdr.owner, hdr.nick */

    if (strncmp(STR_AUTHOR1, buf, LEN_AUTHOR1) == 0)
        ptr = buf + LEN_AUTHOR1 + 1;
    else if (strncmp(STR_AUTHOR2, buf, LEN_AUTHOR2) == 0)
        ptr = buf + LEN_AUTHOR2 + 1;
    else
        return &hdr;        /* no information */

    if (strchr(ptr, '@'))
    {
        str_from(ptr, hdr.owner, hdr.nick);
        hdr.xmode |= POST_INCOME;   /* also MAIL_INCOME */
    }
    else
    {
        if ((ptr2 = strchr(ptr, '(')))
        {
            ptr2[-1] = '\0';
            strcpy(hdr.owner, ptr);
            if ((ptr3 = strchr(ptr2 + 1, ')')))
            {
                *ptr3 = '\0';
                strcpy(hdr.nick, ptr2 + 1);
            }
        }
    }

    /* hdr.title */
    if (!fgets(buf, sizeof(buf), fp))
        return &hdr;        /* no title info */

    if ((ptr2 = strrchr(buf, '\n')))
        *ptr2 = '\0';

    strcpy(hdr.title, ptr);

    /* go through until final part */
    while (!feof(fp))
        fgets(buf, sizeof(buf), fp);

    /* part of hdr.xmode */

#if 0
    if (strncmp("※ Origin: " BOARDNAME, buf, 11 + sizeof(BOARDNAME) - 1) == 0)
#endif
    if (!(hdr.xmode & POST_INCOME))
    {             /* inside maple */
        hdr.xmode |= local ? 0 : POST_OUTGO;
        if (strstr(buf, "◆ Mail:"))
            hdr.xmode |= POST_EMAIL;
    }
#if 0
    else
    {
        hdr.xmode |= POST_INCOME;
    }
#endif

    return &hdr;
}

    static int
fname_cmp(
    char *s1, char *s2)
{
    return strcmp(s1 + 1, s2 + 1);
}

int main(int argc, char *argv[])
{
    struct dirent *de;
    DIR *dirp;
    int ch;
    char fpath[80];
    char *str;
    FILE *fp, *fh;
    HDR *hdr;

    dirp = NULL;
    fpath[1] = '\0';
    ch = '0';
    /* readdir 0-9A-V if exists */
    for (;;)
    {
        *fpath = ch++;
        if ((dirp = opendir(fpath)))
        {
            while ((de = readdir(dirp)))
            {
                str = de->d_name;
                if (*str == '.')
                    continue;

                if (pool_add(str) < 0)
                {
                    printf("Bad article/folder name %c/%s\n", ch, str);
                }
            }
            closedir(dirp);
        }

        if (ch == 'W')
            break;

        if (ch == '9' + 1)
            ch = 'A';
    }

    if (dirp)
    {
        /* qsort chrono */
        if (n_head > 1)
            qsort(n_pool, n_head, sizeof(FNAME), fname_cmp);

        /* generate .DIR.re */
        if ((fh = fopen(".DIR.re", "wb")))
        {
            /* for each file/folder, 0-9A-V */
            for (ch = 0; ch < n_head; ch++)
            {
                sprintf(fpath, "%c/%s", n_pool[ch][7], n_pool[ch]);

                if ((fp = fopen(fpath, "r")))
                {
                    /* parse artcile header */
                    if ((hdr = article_parse(fp)))
                    {
                        /* fill in chrono/date/xmode/xid/xname */
                        hdr->chrono = chrono32(n_pool[ch]);
                        str_stamp(hdr->date, &hdr->chrono);
                        strcpy(hdr->xname, n_pool[ch]);
                        hdr->xid = 0;
                        if (n_pool[ch][0] == 'F')
                            hdr->xmode |= GEM_FOLDER;

                        /* write to .DIR */
                        fwrite(hdr, sizeof(HDR), 1, fh);
                    }
                    fclose(fp);
                }
            }
            fclose(fh);
        }
    }

    n_head = 0;           /* reset pool */

    /* readdir @ if exists */
    if ((dirp = opendir("@")))
    {
        while ((de = readdir(dirp)))
        {
            str = de->d_name;
            if (*str == '.')
                continue;

            if (pool_add(str) < 0)
            {
                printf("Bad article/folder name @/%s\n", str);
            }
        }
        closedir(dirp);

        /* qsort chrono */
        if (n_head > 1)
            qsort(n_pool, n_head, sizeof(FNAME), fname_cmp);

        /* generate .DIR.@ */
        if ((fh = fopen(".DIR.@", "wb")))
        {
            for (ch = 0; ch < n_head; ch++)
            {
                sprintf(fpath, "@/%s", n_pool[ch]);

                if ((fp = fopen(fpath, "r")))
                {
                    /* parse article header */
                    if ((hdr = article_parse(fp)))
                    {
                        /* fill in chrono/date/xid/xname */
                        hdr->chrono = chrono32(n_pool[ch]);
                        str_stamp(hdr->date, &hdr->chrono);
                        strcpy(hdr->xname, n_pool[ch]);
                        hdr->xid = 0;

                        /* write to .DIR */
                        fwrite(hdr, sizeof(HDR), 1, fh);
                    }
                    fclose(fp);
                }
            }
            fclose(fh);
        }
    }
    return 0;
}
