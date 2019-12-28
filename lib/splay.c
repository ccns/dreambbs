/*-------------------------------------------------------*/
/* lib/splay.c          ( NTHU CS MapleBBS Ver 3.00 )    */
/*-------------------------------------------------------*/
/* author : opus.bbs@bbs.cs.nthu.edu.tw                  */
/* target : splay-tree sort routines                     */
/* create : 97/03/29                                     */
/* update : 97/03/29                                     */
/*-------------------------------------------------------*/

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include "splay.h"

SplayNode *splay_in(SplayNode * top,
                    void *data, int (*compare) (const void *lhs, const void *rhs))
{
    int splay_cmp;
    SplayNode *node, *l, *r, *x, N;

    node = (SplayNode *) malloc(sizeof(SplayNode));
    node->data = data;

    if (top == NULL)
    {
        node->left = node->right = NULL;
        return node;
    }

    /* --------------------------------------------------- */
    /* splay this splay tree                               */
    /* --------------------------------------------------- */

    N.left = N.right = NULL;
    l = r = &N;

    for (;;)
    {
        splay_cmp = compare(data, top->data);
        if (splay_cmp < 0)
        {
            if (!(x = top->left))
                break;
            if ((splay_cmp = compare(data, x->data)) < 0)
            {
                /* rotate right */

                top->left = x->right;
                x->right = top;
                top = x;
                if (top->left == NULL)
                    break;
            }
            r->left = top;        /* link right */
            r = top;
            top = top->left;
        }
        else if (splay_cmp > 0)
        {
            if (!(x = top->right))
                break;
            if ((splay_cmp = compare(data, x->data)) > 0)
            {
                /* rotate left */

                top->right = x->left;
                x->left = top;
                top = x;
                if (top->right == NULL)
                    break;
            }
            l->right = top;        /* link left */
            l = top;
            top = top->right;
        }
        else
        {
            break;
        }
    }

    l->right = top->left;        /* assemble */
    r->left = top->right;
    top->left = N.right;
    top->right = N.left;

    /* --------------------------------------------------- */
    /* construct this splay tree                           */
    /* --------------------------------------------------- */

    if (splay_cmp < 0)
    {
        node->left = top->left;
        node->right = top;
        top->left = NULL;
        return node;
    }

    if (splay_cmp > 0)
    {
        node->right = top->right;
        node->left = top;
        top->right = NULL;
        return node;
    }

    /* duplicate entry */

    free(node);
    return top;
}

void
splay_out(const SplayNode * top, void (*data_out) (const void *data, FILE *fp), FILE *fp)
{
    if (top == NULL)
        return;

    splay_out(top->left, data_out, fp);
    data_out(top->data, fp);
    splay_out(top->right, data_out, fp);
}

void
splay_free(SplayNode * top, void (*data_free) (void *data))
{
    SplayNode *node;

    if (top == NULL)
        return;

    if ((node = top->left))
        splay_free(node, data_free);

    if ((node = top->right))
        splay_free(node, data_free);

    data_free(top->data);
    free(top);
}
