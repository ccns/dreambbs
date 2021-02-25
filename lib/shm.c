/*-------------------------------------------------------*/
/* shm.c        ( NTHU CS MapleBBS Ver 3.02 )            */
/*-------------------------------------------------------*/
/* Author: ???                                           */
/* Target: Shared memory loading library                 */
/* Create: ????-??-??                                    */
/* Update: 2021-02-25                                    */
/*       : by Wei-Cheng Yeh (IID) <iid@ccns.ncku.edu.tw> */
/*-------------------------------------------------------*/

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <fcntl.h>

#include "global_def.h"
#include "struct.h"
#include "dao.h"

/* Log output and formatting */

static Logger shm_logger = {
    .file = NULL,
    .path = NULL,
    .lv_skip = LOGLV_WARN,
};

static void shm_formatter_default(char *buf, size_t size, const char *tag, const char *msg)
{
    snprintf(buf, size, "[%s] %s", tag, msg);
}

static void (*shm_formatter)(char *buf, size_t size, const char *tag, const char *msg) = shm_formatter_default;

/* Setters */

/* Set `shm_logger` to `*logger`
 * If `logger` is `NULL`, the default value is used */
void shm_logger_init(const Logger *logger)
{
    shm_logger = (logger) ? *logger : LISTLIT(Logger){
        .file = stderr,
        .path = NULL,
        .lv_skip = LOGLV_WARN,
    };
}

/* Set `shm_formatter` to `formatter`
 * If `formatter` is `NULL`, the default value is used */
void shm_formatter_init(void (*formatter)(char *buf, size_t size, const char *mode, const char *msg))
{
    shm_formatter = (formatter) ? formatter : shm_formatter_default;
}

/* SHM loader */

GCC_NONNULLS
void attach_err(int shmkey, const char *name)
{
    char tag[16];
    char msg[80];

    sprintf(tag, "%s error", name);
    sprintf(msg, "key = %lx", (unsigned long)shmkey);
    logger_tag(&shm_logger, tag, msg, shm_formatter);
    exit(1);
}

void *attach_shm(int shmkey, int shmsize)
{
    void *shmptr;
    int shmid;

    shmid = shmget(shmkey, shmsize, 0);
    if (shmid < 0)
    {
        shmid = shmget(shmkey, shmsize, IPC_CREAT | 0600);
        if (shmid < 0)
            attach_err(shmkey, "shmget");
    }
    else
    {
        shmsize = 0;
    }

    shmptr = (void *) shmat(shmid, NULL, 0);
    if (shmptr == (void *) -1)
        attach_err(shmkey, "shmat");

    if (shmsize)
        memset(shmptr, 0, shmsize);

    return shmptr;
}

/* Similar to `attach_shm()` but does not initialize the SHM */
void *attach_shm_noinit(int shmkey, int shmsize)
{
    void *shmptr;
    int shmid;

    shmid = shmget(shmkey, shmsize, 0);
    if (shmid < 0)
        return NULL;

    shmptr = (void *) shmat(shmid, NULL, 0);
    if (shmptr == (void *) -1)
        attach_err(shmkey, "shmat");

    return shmptr;
}

/*-------------------------------------------------------*/
/* .UTMP cache                                           */
/*-------------------------------------------------------*/

GCC_NONNULLS
void ushm_init(UCACHE **p_ushm)
{
    UCACHE *xshm;

    *p_ushm = xshm = (UCACHE *) attach_shm(UTMPSHM_KEY, sizeof(UCACHE));

#if 0
    if (xshm->mbase < xshm->mpool)
        xshm->mbase = xshm->mpool;
#endif
}

/* Similar to `ushm_init()` but does not initialize `BCACHE` */
GCC_NONNULLS
void ushm_attach(UCACHE **p_ushm)
{
    *p_ushm = (UCACHE *) attach_shm_noinit(UTMPSHM_KEY, sizeof(UCACHE));
}

/*-------------------------------------------------------*/
/* .BRD cache                                            */
/*-------------------------------------------------------*/

GCC_NONNULLS
void bshm_init(BCACHE **p_bshm)
{
    BCACHE *xshm;
    time32_t *uptime;
    int turn;

    turn = 0;
    *p_bshm = xshm = (BCACHE *) attach_shm(BRDSHM_KEY, sizeof(BCACHE));

    uptime = &(xshm->uptime);

    for (;;)
    {
        const time_t t = *uptime;
        if (t > 0)
            return;

        if (t < 0)
        {
            if (++turn < 30)
            {
                sleep(2);
                continue;
            }
        }

        *uptime = -1;

        const int fd = open(FN_BRD, O_RDONLY);
        if (fd >= 0)
        {
            xshm->number =
                read(fd, xshm->bcache, MAXBOARD * sizeof(BRD)) / sizeof(BRD);
            close(fd);
        }

        /* 等所有 boards 資料更新後再設定 uptime */

        time32(uptime);
        logger_tag(&shm_logger, "CACHE", "reload bcache", shm_formatter);
        return;
    }
}

/* Similar to `bshm_init()` but does not initialize `BCACHE` */
GCC_NONNULLS
void bshm_attach(BCACHE **p_bshm)
{
    *p_bshm = (BCACHE *) attach_shm_noinit(BRDSHM_KEY, sizeof(BCACHE));

    if ((*p_bshm)->uptime <= 0) /* bshm 未設定完成 */
        *p_bshm = NULL;
}

/*-------------------------------------------------------*/
/* etc/observe.db cache                                  */
/*-------------------------------------------------------*/

GCC_NONNULLS
int int_cmp(const void *a, const void *b)
{
    return *(const int *)a - *(const int *)b;
}

GCC_NONNULLS
void observeshm_load(OCACHE *oshm)
{
    OBSERVE *head, *tail;
    int size;
    char *fimage;

    size = 0;
    oshm->total = 0;
    memset(oshm->userno, 0, sizeof(int)*MAXOBSERVELIST);
    fimage = f_img(FN_ETC_OBSERVE, &size);
    if (fimage)
    {
        if (size > 0)
        {
            head = (OBSERVE *) fimage;
            tail = (OBSERVE *) (fimage + size);
            for (; head<tail; head++)
            {
                oshm->userno[oshm->total++] = head->userno;
            }
            if (oshm->total>1)
                xsort(oshm->userno, oshm->total, sizeof(int), int_cmp);
        }
        free(fimage);
    }

}

GCC_NONNULLS
void observeshm_init(OCACHE **p_oshm)
{
    *p_oshm = (OCACHE *) attach_shm(OBSERVE_KEY, sizeof(OCACHE));
    observeshm_load(*p_oshm);
}

/*-------------------------------------------------------*/
/* run/var/counter cache                                 */
/*-------------------------------------------------------*/

GCC_NONNULLS
void count_load(COUNTER *countshm)
{
    COUNTER *head;
    int fw, size;
    struct stat st;

    head = countshm;
    if ((fw = open(FN_VAR_SYSHISTORY, O_RDONLY)) >= 0)
    {

        if (!fstat(fw, &st) && (size = st.st_size) > 0)
        {
            read(fw, head, sizeof(COUNTER));
        }
        close(fw);
    }
}

GCC_NONNULLS
void count_init(COUNTER **p_countshm)
{
    *p_countshm = (COUNTER *) attach_shm(COUNT_KEY, sizeof(COUNTER));
    if ((*p_countshm)->hour_max_login == 0)
        count_load(*p_countshm);
}

GCC_NONNULLS
void count_attach(COUNTER **p_countshm)
{
    *p_countshm = (COUNTER *) attach_shm_noinit(COUNT_KEY, sizeof(COUNTER));
}

/*-------------------------------------------------------*/
/* etc/banmail cache                                     */
/*-------------------------------------------------------*/

static BANMAIL *curfw;

GCC_NONNULLS
static int
cmpban(
    const void *ban)
{
    return !strcmp(((const BANMAIL *)ban) -> data, curfw->data);
}

GCC_NONNULLS
void fwshm_load(FWCACHE *fwshm)
{
    BANMAIL *head, data;
    int fw, size, pos;
    struct stat st;

    head = fwshm->fwcache;

    while (*head->data)
    {
        curfw = head;
        pos = rec_loc(FN_ETC_BANMAIL_ACL, sizeof(BANMAIL), cmpban);
        if (pos >= 0)
        {
            rec_get(FN_ETC_BANMAIL_ACL, &data, sizeof(BANMAIL), pos);
            data.usage = head->usage;
            data.time = head->time;
            rec_put(FN_ETC_BANMAIL_ACL, &data, sizeof(BANMAIL), pos);
        }
        head++;
    }

    head = fwshm->fwcache;
    fw = open(FN_ETC_BANMAIL_ACL, O_RDONLY);
    fstat(fw, &st);

    if (!fstat(fw, &st) && (size = st.st_size) > 0)
    {
        if (size > MAXFIREWALL * sizeof(BANMAIL))
            size = MAXFIREWALL * sizeof(BANMAIL);

        if (size)
            read(fw, head, size);
        fwshm->number = size / sizeof(BANMAIL);
    }
    close(fw);
}

GCC_NONNULLS
void fwshm_init(FWCACHE **p_fwshm)
{
    *p_fwshm = (FWCACHE *) attach_shm(FWSHM_KEY, sizeof(FWCACHE));
    if ((*p_fwshm)->number == 0)
        fwshm_load(*p_fwshm);
}

FW *cur;

static int
cmpfw(
    const void *ban)
{
    return !strcmp(((const BANMAIL *)ban) -> data, cur->data);
}

GCC_NONNULLS
void fwoshm_load(FWOCACHE *fwoshm)
{
    BANMAIL data;
    FW *head;
    char fpath[128];
    int pos;
    time_t now;

    now = time(0);

    head = fwoshm->fwocache;
    while (head->name[0])
    {
        brd_fpath(fpath, head->name, BANMAIL_ACL);
        cur = head;
        pos = rec_loc(fpath, sizeof(BANMAIL), cmpfw);
        if (pos >= 0)
        {
            rec_get(fpath, &data, sizeof(BANMAIL), pos);
            data.usage = head->usage;
            if ((now - head->time) > (BANMAIL_EXPIRE * 86400))
                rec_del(fpath, sizeof(BANMAIL), pos, NULL, NULL);
            else
            {
                data.time = head->time;
                rec_put(fpath, &data, sizeof(BANMAIL), pos);
            }
        }
        head++;
    }
}

GCC_NONNULLS
void fwoshm_init(FWOCACHE **p_fwoshm)
{
    *p_fwoshm = (FWOCACHE *) attach_shm(FWOSHM_KEY, sizeof(FWOCACHE));
}

/*-------------------------------------------------------*/
/* etc/movie cache                                       */
/*-------------------------------------------------------*/

GCC_NONNULLS
void fshm_init(FCACHE **p_fshm)
{
    *p_fshm = (FCACHE *) attach_shm(FILMSHM_KEY, sizeof(FCACHE));
}
