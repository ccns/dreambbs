/*-------------------------------------------------------*/
/* util/poststat.c      ( NTHU CS MapleBBS Ver 3.00 )    */
/*-------------------------------------------------------*/
/* target : 統計今日、週、月、年熱門話題                 */
/* create : 95/03/29                                     */
/* update : 97/08/29                                     */
/*-------------------------------------------------------*/
/* syntax : poststat [day]                               */
/*-------------------------------------------------------*/


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>   /* lkchu.981201 */
#include "config.h"
#include "cppdef.h"


static char *myfile[] = {"day", "week", "month", "year"};
static int mycount[4] = {7, 4, 12};
static int mytop[] = {10, 50, 100, 100};
static char *mytitle[] = {"日十", "週五十", "月百", "年度百"};


#define FN_POST_DB      "post.db"
#define FN_POST_AUTHOR  "var/post.author"
#define FN_POST_LOG     "post.log"
#define FN_POST_OLD_DB  "var/post.old.db"
#define HASHSIZE        1024            /* 2's power */
#define TOPCOUNT        200
#define LOWER_BOUND     4


static
struct postrec
{
    char author[13];            /* author name */
    char board[13];             /* board name */
    char title[66];             /* title name */
    time_t date;                /* last post's date */
    int number;                 /* post number */
    struct postrec *next;       /* next rec */
}      *bucket[HASHSIZE];


/* 100 bytes */


static
struct posttop
{
    char author[13];            /* author name */
    char board[13];             /* board name */
    char title[66];             /* title name */
    time_t date;                /* last post's date */
    int number;                 /* post number */
}       top[TOPCOUNT], *tp;


static int
hash(
    char *key)
{
    int i, ch, value = 0;

    for (i = 0; (ch = key[i]) && i < 80; i++)
    {
        value = (value << 5) - value + ch;
    }

    return value;
}


/* ---------------------------------- */
/* hash structure : array + link list */
/* ---------------------------------- */


static void
search(
    struct posttop *t)
{
    struct postrec *p, *q, *s;
    int i, found = 0;

    i = hash(t->title) & (HASHSIZE - 1);
    q = NULL;
    p = bucket[i];
    while (p && (!found))
    {
        if (!strcmp(p->title, t->title) && !strcmp(p->board, t->board))
            found = 1;
        else
        {
            q = p;
            p = p->next;
        }
    }
    if (found)
    {
        p->number += t->number;
        if (p->date < t->date)  /* 取較近日期 */
            p->date = t->date;
    }
    else
    {
        s = (struct postrec *) malloc(sizeof(struct postrec));
        memcpy(s, t, sizeof(struct posttop));
        s->next = NULL;
        if (q == NULL)
            bucket[i] = s;
        else
            q->next = s;
    }
}


static int
sort(
    struct postrec *pp,
    int count)
{
    int i, j;

    for (i = 0; i <= count; i++)
    {
        if (pp->number > top[i].number)
        {
            if (count < TOPCOUNT - 1)
                count++;
            for (j = count - 1; j >= i; j--)
                memcpy(&top[j + 1], &top[j], sizeof(struct posttop));

            memcpy(&top[i], pp, sizeof(struct posttop));
            break;
        }
    }
    return count;
}


static void
load_stat(
    char *fname)
{
    FILE *fp;

    if ((fp = fopen(fname, "r")))
    {
        int count = fread(top, sizeof(struct posttop), TOPCOUNT, fp);
        fclose(fp);
        while (count)
            search(&top[--count]);
    }
}


static void
poststat(
    int mytype)
{
    FILE *fp;
    char buf[40], curfile[80] = "var/day.0", *p;
    struct postrec *pp;
    int i, j;

    if (mytype < 0)
    {
        FILE *fpw;

        /* --------------------------------------- */
        /* load .post and statictic processing     */
        /* --------------------------------------- */

        unlink(FN_POST_OLD_DB);
        rename(FN_POST_DB, FN_POST_OLD_DB);
        if ((fp = fopen(FN_POST_OLD_DB, "r")) == NULL)
            return;

        if ((fpw = fopen(FN_POST_AUTHOR, "a")) == NULL)
        {
            fclose(fp);
            return;
        }

        mytype = 0;
        load_stat(curfile);

        while (fread(top, sizeof(struct posttop), 1, fp) == 1)
        {
            fwrite(top, sizeof(struct posttop), 1, fpw);
            search(top);
        }
        fclose(fp);
        fclose(fpw);
    }
    else
    {
        /* ---------------------------------------------- */
        /* load previous results and statictic processing */
        /* ---------------------------------------------- */

        i = mycount[mytype];
        p = myfile[mytype];
        while (i)
        {
            sprintf(buf, "var/%s.%d", p, i);
            sprintf(curfile, "var/%s.%d", p, --i);
            load_stat(curfile);
            rename(curfile, buf);
        }
        mytype++;
    }

    /* ---------------------------------------------- */
    /* sort top 100 issue and save results            */
    /* ---------------------------------------------- */

    memset(top, 0, sizeof(top));
    for (i = j = 0; i < HASHSIZE; i++)
    {
        for (pp = bucket[i]; pp; pp = pp->next)
        {

#ifdef  DEBUG
            printf("Title : %s, Board: %s\nPostNo : %d, Author: %s\n"
                , pp->title
                , pp->board
                , pp->number
                , pp->author);
#endif

            j = sort(pp, j);
        }
    }

    p = myfile[mytype];
    sprintf(curfile, "var/%s.0", p);
    if ((fp = fopen(curfile, "w")))
    {
        fwrite(top, sizeof(struct posttop), j, fp);
        fclose(fp);
    }

    /* --------------------------------------------------- */
    /* report file : gem/@/@-                              */
    /* --------------------------------------------------- */

    sprintf(curfile, BBSHOME "/gem/@/@-%s", p);
    if ((fp = fopen(curfile, "w")))
    {
        int max, cnt;

        fprintf(fp, "\t\t\x1b[1;34m-----\x1b[37m=====\x1b[41m 本%s大熱門話題 \x1b[40m=====\x1b[34m-----\x1b[0m\n\n", mytitle[mytype]);

        max = mytop[mytype];
        p = buf + 4;
        for (i = cnt = 0; (cnt < max) && (i < j); i++)
        {
            tp = &top[i];
            strcpy(buf, ctime(&(tp->date)));
//          buf[20] = (char) NULL;
            buf[20] = '\0';
            fprintf(fp,
                "\x1b[1;31m%3d. \x1b[33m看板 : \x1b[32m%-16s\x1b[35m《 %s》\x1b[36m%4d 篇\x1b[33m%16s\n"
                "     \x1b[33m標題 : \x1b[0;44;37m%-60.60s\x1b[40m\n"
                , ++cnt, tp->board, p, tp->number, tp->author, tp->title);
        }
        fclose(fp);
    }

    /* free statistics */

    for (i = 0; i < HASHSIZE; i++)
    {
        struct postrec *next;

        for (pp = bucket[i]; pp; pp = next)
        {
            next = pp->next;
            free(pp);
        }
        bucket[i] = NULL;
    }
}


#include "splay.h"


    typedef struct PostText
    {
        struct PostText *ptnext;
        int count;
        char title[FLEX_SIZE];
    } PostText;
    #define PostText_FLEX_MEMBER     title


    typedef struct PostAuthor
    {
        struct PostAuthor *panext;
        PostText *text;
        int count;
        int hash;
        char author[FLEX_SIZE];
    } PostAuthor;
    #define PostAuthor_FLEX_MEMBER   author


static int
pa_cmp(
    void const *x,
    void const *y)
{
    int dif;

    dif = ((const PostAuthor *)y)->count - ((const PostAuthor *)x)->count;
    if (dif)
        return dif;
    return strcmp(((const PostAuthor *)x) -> author, ((const PostAuthor *)y) -> author);
}


static void
pa_out(
    SplayNode *top,
    FILE *fp)
{
    PostAuthor *pa;
    PostText *text;

    if (top == NULL)
        return;

    pa_out(top->left, fp);

    pa = (PostAuthor *) top->data;
    if (pa->count <= LOWER_BOUND)
        return;

    fprintf(fp, "\n>%6d %s\n", pa->count, pa->author);
    for (text = pa->text; text; text = text->ptnext)
    {
        fprintf(fp, "%7d %.70s\n", text->count, text->title);
    }

    pa_out(top->right, fp);
}


static void
post_author(void)
{
    int cc, i, len;
    char *str;
    FILE *fp;
    struct posttop post;
    PostAuthor **paht, *pahe;
    PostText *text;
    SplayNode *patop;

    unlink(FN_POST_OLD_DB);
    rename(FN_POST_AUTHOR, FN_POST_OLD_DB);
    if ((fp = fopen(FN_POST_OLD_DB, "r")) == NULL)
        return;

    paht = (PostAuthor **) calloc(sizeof(PostAuthor *), HASHSIZE);

    while (fread(&post, sizeof(post), 1, fp) == 1)
    {
        cc = hash(str = post.author);
        pahe = paht[i = cc & (HASHSIZE - 1)];

        for (;;)
        {
            if (pahe == NULL)
            {
                len = strlen(str) + 1;
                pahe = (PostAuthor *) malloc(SIZEOF_FLEX(PostAuthor, len));
                pahe->panext = paht[i];
                pahe->text = NULL;
                pahe->count = 1;
                pahe->hash = cc;
                memcpy(pahe->author, str, len);
                paht[i] = pahe;
                break;
            }

            if ((cc == pahe->hash) && !strcmp(str, pahe->author))
            {
                pahe->count++;
                break;
            }

            pahe = pahe->panext;
        }

        text = pahe->text;
        str = post.title;
        for (;;)
        {
            if (text == NULL)
            {
                len = strlen(str) + 1;
                text = (PostText *) malloc(SIZEOF_FLEX(PostText, len + 13));
                text->ptnext = pahe->text;
                text->count = 1;
                sprintf(text->title, "%-13s%s", post.board, str);
                /* memcpy(text->title, str, len); */
                pahe->text = text;
                break;
            }

            if (!strcmp(str, text->title + 13))
            {
                text->count ++;
                break;
            }

            text = text->ptnext;
        }
    }
    fclose(fp);

    /* splay sort */

    patop = NULL;

    for (i = 0; i < HASHSIZE; i++)
    {
        for (pahe = paht[i]; pahe; pahe = pahe->panext)
        {
            patop = splay_in(patop, pahe, pa_cmp);
        }
    }

    /* report */

    if ((fp = fopen(FN_POST_LOG, "w")))
    {
        pa_out(patop, fp);
        fclose(fp);
    }
}

int
main(
    int argc,
    char *argv[])
{
    time_t now;
    struct tm *ptime;

    chdir(BBSHOME "/run");

    if (argc == 2)
    {
        argc = atoi(argv[1]);
        if (argc == 100)
            post_author();
        else
            poststat(argc);
        exit(0);
    }

    time(&now);
    ptime = localtime(&now);
    argc = ptime->tm_hour;

    if (argc == 0)
    {
        if (ptime->tm_mday == 1)
            poststat(2);
        if (ptime->tm_wday == 0)
            poststat(1);
        poststat(0);
    }

    poststat(-1);

    if (argc == 23)
        post_author();

    exit(0);
}
