/*-------------------------------------------------------*/
/* util/acl-sort.c      ( YZU WindTopBBS Ver 3.00 )      */
/*-------------------------------------------------------*/
/* author : visor.bbs@bbs.yzu.edu.tw                     */
/* target : sort [Access Control List]                   */
/*          主要用來排序用, 還有問題                     */
/* create : 99/03/29                                     */
/* update : 99/03/29                                     */
/*-------------------------------------------------------*/
/* syntax : acl-sort [file]                              */
/*-------------------------------------------------------*/

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "splay.h"
#include "cppdef.h"
#include "attrdef.h"


typedef struct
{
    int domain;
    char text[FLEX_SIZE];
} AclText;
#define AclText_FLEX_MEMBER    text


GCC_PURE static int
at_cmp(
    const void *x,
    const void *y)
{
    const char *tail1, *tail2;
    int c1, c2, diff;

    tail1 = ((const AclText *)x)->text + ((const AclText *)x)->domain;
    tail2 = ((const AclText *)y)->text + ((const AclText *)y)->domain;

    for (;;)
    {
        c1 = *tail1--;
        if (c1 == '@')
            c1 = 0;
        else if (c1 >= 'A' && c1 <= 'Z')
            c1 |= 32;

        c2 = *tail2--;
        if (c2 == '@')
            c2 = 0;
        else if (c2 >= 'A' && c2 <= 'Z')
            c2 |= 32;

        if ((diff = c1 - c2))
            return (diff);
    }
}


static void
at_out(
    const SplayNode *top)
{
    const AclText *at;

    if (top == NULL)
        return;

    at_out(top->left);

    at = (const AclText *) top->data;
    fputs(at->text + 1, stdout);

    at_out(top->right);
}


static void
acl_sort(
    const char *fpath)
{
    FILE *fp;
    int len, domain;
    AclText *at;
    SplayNode *top;
    char *str, buf[256];

    if (!(fp = fopen(fpath, "r")))
        return;

    top = NULL;

    while (fgets(buf, sizeof(buf) - 2, fp))
    {
        str = buf;
        if (*str <= '#')
            continue;

        while (*++str > ' ')
            ;

        domain = str - buf;


        while (*str)
            str++;

        len = str - buf;

        at = (AclText *) malloc(SIZEOF_FLEX(AclText, len + 2));
        at->domain = domain;
        at->text[0] = '\0';
        strcpy(at->text + 1, buf);

        top = splay_in(top, at, at_cmp);
    }

    at_out(top);
}

int
main(
    int argc,
    char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage:\t%s <file>\n", argv[0]);
        exit(2);
    }

    acl_sort(argv[1]);
    exit(0);
}
