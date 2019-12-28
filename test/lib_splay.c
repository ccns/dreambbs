#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include "dao.h"

typedef struct {
    FILE *fp;
    int last;
} PrintCtx;

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
printint(const void *a, FILE *fp)
{
    PrintCtx *pctx = (PrintCtx *)fp;
    const intnode *A = (const intnode *) a;
    fprintf(pctx->fp, "%d ", A->i);
    assert(A->i > pctx->last);
    pctx->last = A->i;
}

int main(int argc, char *argv[])
{
    PrintCtx ctx = {stdout, -1};
    SplayNode *top = NULL;
    srandom(time(NULL));
    for (int i = 0; i < 100; i++)
    {
        intnode *I = (intnode *) malloc(sizeof(intnode));
        I->i = random() % 1000;
        top = splay_in(top, I, compareint);
    }
    splay_out(top, printint, (FILE *)&ctx);
    splay_free(top, free);
    printf("\n");
    return 0;
}
