#ifndef _SPLAY_H_
#define _SPLAY_H_

typedef struct SplayNode
{
    void *data;
    struct SplayNode *left;
    struct SplayNode *right;
} SplayNode;

SplayNode *splay_in(SplayNode *top, void *data, int (*compare)(void *x, void *y));

#endif /* _SPLAY_H_ */
