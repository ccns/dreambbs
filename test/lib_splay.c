#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "dao.h"

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
    const intnode *A = (const intnode *) a;
    fprintf(fp, "%d ", A->i);
}

int main(int argc, char *argv[])
{
    int i;
    SplayNode *top = NULL;
    srandom(time(NULL));
    for (i = 0; i < 100; i++)
    {
        intnode *I = (intnode *) malloc(sizeof(intnode));
        I->i = random() % 1000;
        top = splay_in(top, I, compareint);
    }
    splay_out(top, printint, stdout);
    splay_free(top, free);
    printf("\n");
    return 0;
}
