/* ----------------------------------------------------- */
/* author : thor.bbs@bbs.cs.nthu.edu.tw                  */
/* target : dynamic link modules library for maple bbs   */
/* create : 99/02/14                                     */
/* update :   /  /                                       */
/* ----------------------------------------------------- */

#include <dlfcn.h>
#include <stdarg.h>
#include <strings.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct
{
    char *path;
    void *handle;
} DL_list;

DL_list  *dl_pool;
int dl_size, dl_head;

#define DL_ALLOC_MIN	5

#if 1
extern void blog(char *, char *);
#define TRACE blog
#endif

void*
DL_get(
    char *name
    /* format: "Xmodule_path:Xname" */
)
{
    char buf[512], *t;
    DL_list *p, *tail;

    strcpy(buf,name);
    if (!(t = (char *) strchr(buf,':')))
        return NULL;

    *t++ = 0;

    if (!dl_pool)
    {
        /* Initialize DL entries */
        dl_size = DL_ALLOC_MIN;
        /* dl_head = 0 */
        dl_pool = (DL_list *)malloc(dl_size * sizeof(DL_list));
    }

    p = dl_pool;
    tail = p + dl_head;
    while (p < tail)
    {
        if (!strcmp(buf, p->path))
            break;
        p++;
    }

    if (p >= tail)
    { /* not found */
        if (dl_head >= dl_size)
        { /* not enough space */
            dl_size += DL_ALLOC_MIN;
            dl_pool = (DL_list *)realloc(dl_pool, dl_size * sizeof(DL_list));
            p = dl_pool + dl_head; /* visor.20000718: to a new place */
        }
#if defined(__OpenBSD__)
        p->handle = dlopen(p->path = (char *) strdup(buf), RTLD_LAZY);
#else
        p->handle = dlopen(p->path = (char *) strdup(buf), RTLD_NOW);
#endif
        dl_head ++;
    }

    if (!p->handle)
        return NULL;

    return dlsym(p->handle,t);
}

int
DL_func(char *name, ...)
{
    va_list args;
    int (*f)(va_list), ret;

    va_start(args,name);

    if (!(f = DL_get(name)))
    { /* not get func */
        return -1;
    }

    ret = (*f)(args);
    va_end(args);

    return ret;
}

