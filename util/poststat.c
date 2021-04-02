/*-------------------------------------------------------*/
/* util/poststat.c      ( NTHU CS MapleBBS Ver 3.00 )    */
/*-------------------------------------------------------*/
/* target : �έp����B�g�B��B�~�������D                 */
/* create : 95/03/29                                     */
/* update : 97/08/29                                     */
/*-------------------------------------------------------*/
/* syntax : poststat [day]                               */
/*-------------------------------------------------------*/

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>   /* lkchu.981201 */
#include "cppdef.h"
#include "struct.h"


static const char *const myfile[] = {"day", "week", "month", "year"};
static const int mycount[4] = {7, 4, 12};
static const int mytop[] = {10, 50, 100, 100};
static const char *const mytitle[] = {"��Q", "�g���Q", "���", "�~�צ�"};


#define FN_POST_DB      "post.db"
#define FN_POST_AUTHOR  "var/post.author"
#define FN_POST_LOG     "post.log"
#define FN_POST_OLD_DB  "var/post.old.db"
#define HASHSIZE        1024U           /* Use 2's power to prevent division */
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
struct posttop  /* DISKDATA(raw) */
{
    char author[13];            /* author name */
    char board[13];             /* board name */
    char title[66];             /* title name */
    time_t date;                /* last post's date */
    int number;                 /* post number */
}       top[TOPCOUNT], *tp;


static int
hash(
    const char *key)
{
    int ch, value = 0;

    for (int i = 0; (ch = key[i]) && i < 80; i++)
    {
        value = 31 * value + ch;
    }

    return value;
}


/* ---------------------------------- */
/* hash structure : array + link list */
/* ---------------------------------- */


static void
search(
    const struct posttop *t)
{
    struct postrec *p, *q;
    int i, found = 0;

    i = hash(t->title) % HASHSIZE;
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
        if (p->date < t->date)  /* �������� */
            p->date = t->date;
    }
    else
    {
        struct postrec *s = (struct postrec *) malloc(sizeof(struct postrec));
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
    const struct postrec *pp,
    int count)
{
    for (int i = 0; i <= count; i++)
    {
        if (pp->number > top[i].number)
        {
            if (count < TOPCOUNT - 1)
                count++;
            for (int j = count - 1; j >= i; j--)
                memcpy(&top[j + 1], &top[j], sizeof(struct posttop));

            memcpy(&top[i], pp, sizeof(struct posttop));
            break;
        }
    }
    return count;
}


static void
load_stat(
    const char *fname)
{
    FILE *fp = fopen(fname, "r");

    if (fp)
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
    char buf[40], curfile[80] = "var/day.0";
    const char *p;
    int j;

    if (mytype < 0)
    {
        FILE *fpw;

        /* --------------------------------------- */
        /* load .post and statistic processing     */
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
        /* load previous results and statistic processing */
        /* ---------------------------------------------- */

        int i = mycount[mytype];
        const char *p = myfile[mytype];
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
    for (int i = j = 0; i < HASHSIZE; i++)
    {
        for (struct postrec *pp = bucket[i]; pp; pp = pp->next)
        {

#ifdef  DEBUG
            printf("Title : %s, Board: %s\nPostNo : %d, Author: %s\n",
                pp->title,
                pp->board,
                pp->number,
                pp->author);
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

        fprintf(fp, "\t\t\x1b[1;34m-----\x1b[37m=====\x1b[41m ��%s�j�������D \x1b[40m=====\x1b[34m-----\x1b[0m\n\n", mytitle[mytype]);

        max = mytop[mytype];
        p = buf + 4;
        for (int i = cnt = 0; (cnt < max) && (i < j); i++)
        {
            tp = &top[i];
            strcpy(buf, ctime(&(tp->date)));
//          buf[20] = (char) NULL;
            buf[20] = '\0';
            fprintf(fp,
                "\x1b[1;31m%3d. \x1b[33m�ݪO : \x1b[32m%-16s\x1b[35m�m %s�n\x1b[36m%4d �g\x1b[33m%16s\n"
                "     \x1b[33m���D : \x1b[0;44;37m%-60.60s\x1b[40m\n",
                ++cnt, tp->board, p, tp->number, tp->author, tp->title);
        }
        fclose(fp);
    }

    /* free statistics */

    for (int i = 0; i < HASHSIZE; i++)
    {
        struct postrec *next;

        for (struct postrec *pp = bucket[i]; pp; pp = next)
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
    const void *x,
    const void *y)
{
    int dif = ((const PostAuthor *)y)->count - ((const PostAuthor *)x)->count;
    if (dif)
        return dif;
    return strcmp(((const PostAuthor *)x) -> author, ((const PostAuthor *)y) -> author);
}


static void
pa_out(
    const void *pa_obj,
    FILE *fp)
{
    const PostAuthor *pa = (const PostAuthor *) pa_obj;
    if (pa->count <= LOWER_BOUND)
        return;

    fprintf(fp, "\n>%6d %s\n", pa->count, pa->author);
    for (const PostText *text = pa->text; text; text = text->ptnext)
    {
        fprintf(fp, "%7d %.70s\n", text->count, text->title);
    }
}

static void
pa_free(
    void *pa_obj)
{
    PostAuthor *pa = (PostAuthor *) pa_obj;
    for (PostText *text = pa->text; text; text = text->ptnext)
    {
        free(text);
    }
    free(pa);
}


static void
post_author(void)
{
    FILE *fp;
    struct posttop post;
    PostAuthor **paht;
    SplayNode *patop;

    unlink(FN_POST_OLD_DB);
    rename(FN_POST_AUTHOR, FN_POST_OLD_DB);
    if ((fp = fopen(FN_POST_OLD_DB, "r")) == NULL)
        return;

    paht = (PostAuthor **) calloc(sizeof(PostAuthor *), HASHSIZE);

    while (fread(&post, sizeof(post), 1, fp) == 1)
    {
        char *str = post.author;
        int cc = hash(str);
        int i = cc % HASHSIZE;
        PostAuthor *pahe = paht[i];
        PostText *text;

        for (;;)
        {
            if (pahe == NULL)
            {
                int len = strlen(str) + 1;
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
                int len = strlen(str) + 1;
                text = (PostText *) malloc(SIZEOF_FLEX(PostText, len + 13));
                text->ptnext = pahe->text;
                text->count = 1;
                sprintf(text->title, "%-*s %s", IDLEN, post.board, str);
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

    for (int i = 0; i < HASHSIZE; i++)
    {
        for (PostAuthor *pahe = paht[i]; pahe; pahe = pahe->panext)
        {
            patop = splay_in(patop, pahe, pa_cmp);
        }
    }

    /* report */

    if ((fp = fopen(FN_POST_LOG, "w")))
    {
        splay_out(patop, pa_out, fp);
        fclose(fp);
    }
    splay_free(patop, pa_free);
}

int
main(
    int argc,
    char *argv[])
{
    time_t now;
    struct tm *ptime;

    setgid(BBSGID);
    setuid(BBSUID);
    chdir(BBSHOME "/run");

    if (argc > 1)
    {
        switch (argc = atoi(argv[1]))
        {
        case 100:
            post_author();
            break;

        default:
            if (!(argc >= -1 && argc < COUNTOF(myfile) - 1))
            {
                fprintf(stderr, "Usage: %s [action]\n", argv[0]);
                fprintf(stderr, "actions:\n");
                for (argc = 0; argc < COUNTOF(myfile); argc++)
                    fprintf(stderr, "\t%d: Generate top %d of the %s\n", argc-1, mytop[argc], myfile[argc]);
                fprintf(stderr, "\t100: Generate post.log of the %s\n", myfile[0]);
                return 2;
            }
            poststat(argc);
            break;
        }
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
