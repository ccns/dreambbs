#ifndef SPLAY_H
#define SPLAY_H

typedef struct SplayNode
{
    void *data;
    struct SplayNode *left;
    struct SplayNode *right;
} SplayNode;

#ifdef __cplusplus
extern "C" {
#endif

SplayNode *splay_in(SplayNode *top, void *data, int (*compare)(const void *x, const void *y));
void splay_out(const SplayNode * top, void (*data_out) (const void *data, FILE *fp), FILE *fp);
void splay_free(SplayNode * top, void (*data_free) (void *data));

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* SPLAY_H */
