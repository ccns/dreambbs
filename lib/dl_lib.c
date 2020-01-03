/* ----------------------------------------------------- */
/* author : thor.bbs@bbs.cs.nthu.edu.tw                  */
/* target : dynamic link modules library for maple bbs   */
/* create : 99/02/14                                     */
/* update :   /  /                                       */
/* ----------------------------------------------------- */

#include "config.h"

#include <dlfcn.h>
#include <limits.h>
#include <stdarg.h>
#include <strings.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "dao.h"

typedef struct  /* Hashable */
{
    char *path;
    void *handle;
} DL_list;

DL_list *dl_pool;
int dl_size, dl_head;

#define DL_ALLOC_MIN    5

#if defined(__OpenBSD__)
  #define DL_OPEN_FLAGS  RTLD_LAZY
#else
  #define DL_OPEN_FLAGS  RTLD_NOW
#endif

/* `DL_list` manipulations */

static DL_list *DL_find(const char *path, int path_len)
{
    const DL_list *const tail = dl_pool + dl_head;
    DL_list *p;
    for (p = dl_pool; p < tail; p++)
    {
        if (!strncmp(path, p->path, path_len))
            break;
    }
    return p;
}

static DL_list *DL_insert(const char *path, int path_len)
{
    DL_list *p;

    if (dl_head >= dl_size)
    {                            /* not enough space or not initialized */
        dl_size += DL_ALLOC_MIN;
        dl_pool = (DL_list *) realloc(dl_pool, dl_size * sizeof(DL_list));
    }

    p = DL_find(path, path_len);
    if (p >= dl_pool + dl_head)
    {                            /* not found */
        p->path = strndup(path, path_len);
        p->handle = dlopen(p->path, DL_OPEN_FLAGS);
        dl_head++;
    }

    return p;
}

/* Dynamic library object getters */

/* format: "Xmodule_path:Xname" */
static inline char *DL_name_delim(const char *name)
{
    return (char *)memchr(name, ':', strnlen(name, PATH_MAX));
}

void *DL_get(const char *name)
{
    const char *t = DL_name_delim(name);
    DL_list *p;
    if (!t)
        return NULL;

    p = DL_insert(name, t - name);
    if (!p->handle)              /* IID.20200104: Failed to load the last time; try again */
        p->handle = dlopen(p->path, DL_OPEN_FLAGS);
    if (!p->handle)              /* Still failed; continue */
        return NULL;

    return dlsym(p->handle, t+1);
}

void *DL_get_hotswap(const char *name)
{
    const char *t = DL_name_delim(name);
    DL_list *p;
    if (!t)
        return NULL;

    p = DL_insert(name, t - name);
    if (p->handle)              /* Unload the library to load a updated one */
        dlclose(p->handle);

    p->handle = dlopen(p->path, DL_OPEN_FLAGS);
    if (!p->handle)
        return NULL;

    return dlsym(p->handle, t+1);
}

/* Dynamic library function invokers */

static int DL_invoke(int (*f)(va_list), ...)
{
    va_list args;
    int ret;

    if (!f)                      /* not get func */
        return -1;

    va_start(args, f);
    ret = (*f)(args);
    va_end(args);

    return ret;
}

int DL_func(const char *name, ...)
{
    return DL_invoke((int (*)(va_list)) DL_get(name));
}
int DL_func_hotswap(const char *name, ...)
{
    return DL_invoke((int (*)(va_list)) DL_get_hotswap(name));
}
