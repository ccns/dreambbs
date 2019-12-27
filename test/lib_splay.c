#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "dao.h"

void
splay_free(SplayNode * top, void (*data_free) (void *))
{
    SplayNode *node = top->left;

    if (node)
        splay_free(node, data_free);

    node = top->right;

    if (node)
        splay_free(node, data_free);

    data_free(top->data);
    free(top);
}

void
splay_out(const SplayNode * top, void (*data_out) (const void *))
{
    if (top == NULL)
        return;

    splay_out(top->left, data_out);
    data_out(top->data);
    splay_out(top->right, data_out);
}

typedef struct
{
    int i;
}      intnode;

GCC_PURE int
compareint(const void *a, const void *b)
{
    const intnode *A = (const intnode *)a;
    const intnode *B = (const intnode *)b;
    return A->i - B->i;
}


void
printint(const void *a)
{
    const intnode *A = (const intnode *) a;
    printf("%d ", A->i);
}

int main(int argc, char *argv[])
{
    int i;
    intnode *I;
    SplayNode *top = NULL;
    srandom(time(NULL));
    for (i = 0; i < 100; i++)
    {
        I = (intnode *) malloc(sizeof(intnode));
        I->i = random() % 1000;
        top = splay_in(top, I, compareint);
    }
    splay_out(top, printint);
    splay_free(top, free);
    printf("\n");
    return 0;
}
