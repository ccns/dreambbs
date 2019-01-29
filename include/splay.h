#ifndef SPLAY_H
#define SPLAY_H

typedef struct SplayNode
{
    void *data;
    struct SplayNode *left;
    struct SplayNode *right;
} SplayNode;

SplayNode *splay_in(SplayNode *top, void *data, int (*compare)(const void *x, const void *y));

#endif /* SPLAY_H */
