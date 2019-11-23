#include "dao.h"
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <unistd.h>

int rec_add(const char *fpath, const void *data, int size)
{
    int fd;

    if ((fd = open(fpath, O_WRONLY | O_CREAT | O_APPEND, 0600)) < 0)
        return -1;

    /* flock(fd, LOCK_EX); */
    write(fd, data, size);
    /* flock(fd, LOCK_UN); */
    close(fd);

    return 0;
}

static bool is_bottompost(HDR * hdr)
{
    return (hdr->xmode & POST_BOTTOM);
}


int rec_bot(                    /* amaki.040715: 嵌入式寫檔 */
               const char *fpath, const void *data, int size)
{
    int fd, fsize, count;
    HDR *pool = NULL, *set;
    char set_pool[REC_SIZ];
    struct stat st;

    if ((fd = open(fpath, O_RDWR | O_CREAT, 0600)) < 0)
        return -1;

    /* flock(fd, LOCK_EX); */
    /* Thor.981205: 用 fcntl 取代flock, POSIX標準用法 */
    f_exlock(fd);

    fstat(fd, &st);

    count = 0;
    set = (HDR *)set_pool;

    if ((fsize = st.st_size))
    {
        while ((fsize -= size) >= 0)
        {
            lseek(fd, fsize, SEEK_SET);
            read(fd, set, size);
            if (!is_bottompost(set))
            {
                if (count)
                {
                    pool = (HDR *)malloc(count * size);
                    read(fd, pool, count * size);
                    lseek(fd, -size * count, SEEK_CUR);
                }
                break;
            }
            else if (fsize <= 0)    /* amaki.040715: 全部都是置底的東西 */
            {
                count++;
                pool = (HDR *)malloc(count * size);

                lseek(fd, -size, SEEK_CUR);
                read(fd, pool, count * size);
                lseek(fd, -size * count, SEEK_CUR);
                break;
            }
            else
                count++;
        }
    }

    write(fd, data, size);

    if (count)
    {
        write(fd, pool, count * size);
        free(pool);
    }

    /* flock(fd, LOCK_EX); */
    /* Thor.981205: 用 fcntl 取代flock, POSIX標準用法 */
    f_unlock(fd);

    close(fd);

    return 0;
}

int
rec_del(const char *fpath,
        int size, int pos, int (*fchk) (const void *obj), int (*fdel) (void *obj))
{
    int fd;
    off_t off, len;
    char pool[REC_SIZ], *data;
    struct stat st;

    if ((fd = open(fpath, O_RDWR)) < 0)
        return -1;

    /* flock(fd, LOCK_EX); */
    /* Thor.981205: 用 fcntl 取代flock, POSIX標準用法 */
    f_exlock(fd);

    fstat(fd, &st);
    len = st.st_size;

    data = pool;
    off = size * pos;

    /* 驗證 pos 位置資料的正確性 */

    if (len > off)
    {
        lseek(fd, off, SEEK_SET);
        read(fd, data, size);

        pos = fchk ? (*fchk) (data) : 1;
    }
    else
    {
        pos = 0;
    }

    /* 不對的話，從頭找起 */

    if (!pos)
    {
        off = 0;
        lseek(fd, off, SEEK_SET);
        while (read(fd, data, size) == size)
        {
            if ((pos = (*fchk) (data)))
                break;

            off += size;
        }
    }

    /* 找到之後，刪除資料 */

    if (pos)
    {
        if (fdel)
            pos = (*fdel) (data);

        if (pos)
        {
            /* really delete it */

            len -= (off + size);
            data = (char *)malloc(len);
            read(fd, data, len);
        }
        else
        {
            /* just update it */

            len = size;
        }

        lseek(fd, off, SEEK_SET);
        write(fd, data, len);

        if (pos)
        {
            ftruncate(fd, off + len);
            free(data);
        }
    }

    /* flock(fd, LOCK_UN); */
    /* Thor.981205: 用 fcntl 取代flock, POSIX標準用法 */
    f_unlock(fd);

    close(fd);

    return 0;
}

int rec_get(const char *fpath, void *data, int size, int pos)
{
    int fd;
    int ret;

    ret = -1;

    if ((fd = open(fpath, O_RDONLY)) >= 0)
    {
        if (lseek(fd, (off_t) (size * pos), SEEK_SET) >= 0)
        {
            if (read(fd, data, size) == size)
                ret = 0;
        }
        close(fd);
    }
    return ret;
}


int rec_ins(const char *fpath, const void *data, int size, int pos, int num)
{
    int fd;
    off_t off, len;
    struct stat st;
    char *data_ins = NULL;

    if ((fd = open(fpath, O_RDWR | O_CREAT, 0600)) < 0)
        return -1;

    /* flock(fd, LOCK_EX); */
    /* Thor.981205: 用 fcntl 取代flock, POSIX標準用法 */
    f_exlock(fd);

    fstat(fd, &st);
    len = st.st_size;

    /* lkchu.990428: ernie patch 如果 len=0 & pos>0
       (在剛開精華區目錄進去貼上，選下一個) 時會寫入垃圾 */
    off = len ? size * pos : 0;
    lseek(fd, off, SEEK_SET);

    size *= num;
    len -= off;
    if (len > 0)
    {
        data_ins = (char *)malloc(pos = len + size);
        memcpy(data_ins, data, size);
        read(fd, data_ins + size, len);
        lseek(fd, off, SEEK_SET);
        data = data_ins;
        size = pos;
    }

    write(fd, data, size);

    /* flock(fd, LOCK_UN); */
    /* Thor.981205: 用 fcntl 取代flock, POSIX標準用法 */
    f_unlock(fd);

    close(fd);

    if (len > 0)
        free(data_ins);

    return 0;
}


int rec_loc(const char *fpath, int size, int (*fchk) (const void *obj))
{
    int fd, pos, tmp;
    off_t off;
    char pool[REC_SIZ], *data;

    if ((fd = open(fpath, O_RDWR)) < 0)
        return -1;

    f_exlock(fd);

    pos = -1;
    data = pool;
    off = 0;
    tmp = 0;
    if (fchk)
    {
        lseek(fd, off, SEEK_SET);
        while (read(fd, data, size) == size)
        {
            pos++;
            if ((*fchk) (data))
            {
                tmp = 1;
                break;
            }
        }
    }

    f_unlock(fd);

    close(fd);

    return tmp ? pos : -1;
}

int rec_mov(const char *fpath, int size, int from, int to)
{
    int fd, backward;
    off_t off, len;
    char *pool, *data;
    struct stat st;

    if ((fd = open(fpath, O_RDWR)) < 0)
        return -1;

    /* flock(fd, LOCK_EX); */
    /* Thor.981205: 用 fcntl 取代flock, POSIX標準用法 */
    f_exlock(fd);

    fstat(fd, &st);
    len = st.st_size / size - 1;

    if (from > to)
    {
        backward = from;
        from = to;
        to = backward;
        backward = 1;
    }
    else
    {
        backward = 0;
    }

    to = BMIN(to, len);

    off = size * from;
    lseek(fd, off, SEEK_SET);

    len = (to - from + 1) * size;
    pool = data = (char *)malloc(len + size);

    if (backward)
        data += size;
    read(fd, data, len);

    data = pool + len;
    if (backward)
        memcpy(pool, data, size);
    else
        memcpy(data, pool, size);

    data = pool;
    if (!backward)
        data += size;

    lseek(fd, off, SEEK_SET);
    write(fd, data, len);

    /* flock(fd, LOCK_UN); */
    /* Thor.981205: 用 fcntl 取代flock, POSIX標準用法 */
    f_unlock(fd);

    close(fd);
    free(pool);

    return 0;
}


int rec_num(const char *fpath, int size)
{
    struct stat st;

    if (stat(fpath, &st) == -1)
        return 0;
    return (st.st_size / size);
}

int rec_put(const char *fpath, const void *data, int size, int pos)
{
    int fd;

    if ((fd = open(fpath, O_WRONLY | O_CREAT, 0600)) < 0)
        return -1;

    /* flock(fd, LOCK_EX); */
    /* Thor.981205: 用 fcntl 取代flock, POSIX標準用法 */
    f_exlock(fd);

    lseek(fd, (off_t) (size * pos), SEEK_SET);
    write(fd, data, size);

    /* flock(fd, LOCK_UN); */
    /* Thor.981205: 用 fcntl 取代flock, POSIX標準用法 */
    f_unlock(fd);

    close(fd);

    return 0;
}

int
rec_put2(const char *fpath, const void *data, int size, int pos, int (*fchk) (const void *obj))
{
    int fd;
    off_t off, len;
    char pool[REC_SIZ];
    struct stat st;
    char* data_chk;

    if ((fd = open(fpath, O_RDWR | O_CREAT, 0600)) < 0)
        return -1;

    /* flock(fd, LOCK_EX); */
    /* Thor.981205: 用 fcntl 取代flock, POSIX標準用法 */
    f_exlock(fd);

    fstat(fd, &st);
    len = st.st_size;

    data_chk = pool;
    off = size * pos;

    /* 驗證 pos 位置資料的正確性 */

    if (len > off)
    {
        if (fchk)
        {
            lseek(fd, off, SEEK_SET);
            read(fd, data_chk, size);
            pos = (*fchk) (data_chk);
        }
        else
        {
            pos = 1;
        }
    }
    else
    {
        if (len)
        {
            pos = 0;            /* 從頭找起 */
        }
        else
        {
            /* 若原本是空檔案，那麼 rec_put 當成 rec_add */
            pos = 1;
            off = 0;
        }
    }

    /* 不對的話，從頭找起 */

    if (!pos)
    {
        off = 0;
        lseek(fd, off, SEEK_SET);
        while (read(fd, data_chk, size) == size)
        {
            if ((pos = (*fchk) (data_chk)))
                break;

            off += size;
        }
    }

    /* 找到之後，更新資料 */

    if (pos)
    {
        lseek(fd, off, SEEK_SET);
        write(fd, data, size);
    }

    /* flock(fd, LOCK_UN); */
    /* Thor.981205: 用 fcntl 取代flock, POSIX標準用法 */
    f_unlock(fd);

    close(fd);

    return 0;
}

int
rec_ref(const char *fpath,
        const void *data,
        int size,
        int pos, int (*fchk) (const void *obj), void (*fref) (void *obj, const void *ref))
{
    int fd;
    off_t off, len;
    char pool[REC_SIZ];
    char *data_read;
    struct stat st;

    if ((fd = open(fpath, O_RDWR, 0600)) < 0)
        return -1;

    /* flock(fd, LOCK_EX); */
    /* Thor.981205: 用 fcntl 取代flock, POSIX標準用法 */
    f_exlock(fd);

    fstat(fd, &st);
    len = st.st_size;

    data_read = pool;
    off = size * pos;

    /* 驗證 pos 位置資料的正確性 */

    if (len > off)
    {
        lseek(fd, off, SEEK_SET);
        read(fd, data_read, size);
        pos = fchk ? (*fchk) (data_read) : 1;
    }
    else
    {
        if (len)
        {
            pos = 0;            /* 從頭找起 */
        }
        else
        {
            /* 若原本是空檔案，那麼放棄 */
            f_unlock(fd);
            close(fd);
            return -1;
        }
    }

    /* 不對的話，從頭找起 */

    if (!pos)
    {
        off = 0;
        lseek(fd, off, SEEK_SET);
        while (read(fd, data_read, size) == size)
        {
            if ((pos = (*fchk) (data_read)))
                break;

            off += size;
        }
    }

    /* 找到之後，更新資料 */

    if (pos)
    {
        (*fref) (data_read, data);
        lseek(fd, off, SEEK_SET);
        write(fd, data_read, size);
    }

    /* flock(fd, LOCK_UN); */
    /* Thor.981205: 用 fcntl 取代flock, POSIX標準用法 */
    f_unlock(fd);

    close(fd);

    return 0;
}

int
rec_sync(const char *fpath,
         int size,
         int (*fsync) (const void *lhs, const void *rhs), int (*fchk) (const void *obj))
{
    int fd, fsize;
    struct stat st;

    fsize = 0;

    if ((fd = open(fpath, O_RDWR, 0600)) < 0)
        return fsize;

    if (!fstat(fd, &st) && (fsize = st.st_size) > 0)
    {
        char *base;

        base = (char *)malloc(fsize);
        fsize = read(fd, base, fsize);

        if (fsize >= size)
        {
            if (fchk)            /* 檢查是否有不正確的資料 */
            {
                char *head;
                char *tail;

                head = base;
                tail = base + fsize;
                while (head < tail)
                {
                    if (fchk(head))    /* 此筆資料正確 */
                    {
                        head += size;
                        continue;
                    }

                    /* 有問題的資料要刪除 */
                    tail -= size;
                    if (head >= tail)
                        break;
                    memcpy(head, tail, size);
                }
                fsize = tail - base;
            }

            if (fsize > 0)
            {
                if (fsize > size)
                    qsort(base, fsize / size, size, fsync);
                lseek(fd, 0, SEEK_SET);
                write(fd, base, fsize);
                ftruncate(fd, fsize);
            }
        }
        free(base);
    }
    close(fd);

    if (fsize <= 0)
        unlink(fpath);

    return fsize;
}

int rec_append(const char *fpath, const void *data, int size)
{
    register int fd;

    if ((fd = open(fpath, O_WRONLY | O_CREAT | O_APPEND, 0600)) < 0)
        return -1;

    write(fd, data, size);
    close(fd);

    return 0;
}
