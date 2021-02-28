#ifndef DAO_H
#define DAO_H

#include "config.h"

#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h>

/* Macros for implementation-defined attributes */
#include "attrdef.h"

#include "cppdef.h"

#ifndef NULL
#define NULL            0               /* ((char *) 0) */
#endif

#ifndef BLK_SIZ
#define BLK_SIZ         4096            /* disk I/O block size */
#endif

#ifndef REC_SIZ
#define REC_SIZ         4096            /* disk I/O record size */
#endif

#include "logger.h"                     /* Message logging types */
#include "hdr.h"                        /* prototype */
#include "dns.h"                        /* dns type */
#include "splay.h"                      /* splay type */
#include "struct.h"                     /* Miscellaneous types */
#include "modes.h"                      /* Enumeration types */

#ifdef __cplusplus
extern "C" {
#endif

/* Thor.981206: lkchu patch */
extern const char radix32[32];

/* acl.c */
int acl_addr(const char *acl, const char *addr);
int acl_has(const char *acl, const char *user, const char *host);
/* file.c */
void f_cat(const char *fpath, const char *msg);
int f_cp(const char *src, const char *dst, int mode);
char *f_img(const char *fpath, int *fsize);
int f_ln(const char *src, const char *dst);
int f_exlock(int fd);
int f_unlock(int fd);
char *f_map(const char *fpath, int *fsize);
int f_mode(const char *fpath);
int f_mv(const char *src, const char *dst);
FILE *f_new(const char *fold, char *fnew);
int f_open(const char *fpath);
void brd_fpath(char *fpath, const char *board, const char *fname);
void gem_fpath(char *fpath, const char *board, const char *fname);
void usr_fpath(char *fpath, const char *user, const char *fname);
int f_rm(const char *fpath);
void f_suck(FILE *fp, const char *fpath);
void mak_dirs(char *fpath);
/* isnot.c */
GCC_CONSTEXPR bool is_alnum(int ch);
GCC_CONSTEXPR bool is_alpha(int ch);
GCC_PURE bool is_fname(const char *str);
int is_fpath(char *path);
GCC_PURE int not_addr(const char *addr);
/* radix32.c */
void archiv32(time_t chrono, char *fname);
void archiv32m(time_t chrono, char *fname);
GCC_PURE time_t chrono32(const char *str);
/* shm.c */
void shm_logger_init(const Logger *logger);
void shm_formatter_init(void (*formatter)(char *buf, size_t size, const char *mode, const char *msg));
GCC_NONNULLS void attach_err(int shmkey, const char *name);
void *attach_shm(int shmkey, int shmsize);
void *attach_shm_noinit(int shmkey, int shmsize);
GCC_NONNULLS void ushm_init(UCACHE **p_ushm);
GCC_NONNULLS void ushm_attach(UCACHE **p_ushm);
GCC_NONNULLS void bshm_init(BCACHE **p_bshm);
GCC_NONNULLS void bshm_attach(BCACHE **p_bshm);
GCC_NONNULLS int int_cmp(const void *a, const void *b);
GCC_NONNULLS void observeshm_load(OCACHE *oshm);
GCC_NONNULLS void observeshm_init(OCACHE **p_oshm);
GCC_NONNULLS void count_load(COUNTER *countshm);
GCC_NONNULLS void count_init(COUNTER **p_countshm);
GCC_NONNULLS void count_attach(COUNTER **p_countshm);
GCC_NONNULLS void fwshm_load(FWCACHE *fwshm);
GCC_NONNULLS void fwshm_init(FWCACHE **p_fwshm);
GCC_NONNULLS void fwoshm_load(FWOCACHE *fwoshm);
GCC_NONNULLS void fwoshm_init(FWOCACHE **p_fwoshm);
GCC_NONNULLS void fshm_init(FCACHE **p_fshm);
/* url_encode.c */
void url_encode(char *dst, const char *src);
/* dl_lib.c */
GCC_NODISCARD struct DL_handle* DL_hold(const char *path);
int DL_release(const char *path, struct DL_handle *handle);
void *DL_get(const char *name);
void *DL_get_hotswap(const char *name);
int DL_func(const char *name, ...);
int DL_func_hotswap(const char *name, ...);
/* record.c */
int rec_add(const char *fpath, const void *data, int size);
int rec_bot(const char *fpath, const void *data, int size);
int rec_del(const char *fpath, int size, int pos, int (*fchk)(const void *obj), int (*fdel)(void *obj));
int rec_get(const char *fpath, void *data, int size, int pos);
int rec_ins(const char *fpath, const void *data, int size, int pos, int num);
int rec_loc(const char *fpath, int size, int (*fchk)(const void *obj));
int rec_mov(const char *fpath, int size, int from, int to);
int rec_num(const char *fpath, int size);
int rec_put(const char *fpath, const void *data, int size, int pos);
int rec_put2(const char *fpath, const void *data, int size, int pos, int (*fchk)(const void *obj));
int rec_ref(const char *fpath, const void *data, int size, int pos, int (*fchk)(const void *obj), void (*fref)(void *obj, const void *ref));
int rec_sync(const char *fpath, int size, int (*fsync)(const void *lhs, const void *rhs), int (*fchk)(const void *obj));
int rec_append(const char *fpath, const void *data, int size);
/* splay.c */
SplayNode *splay_in(SplayNode *top, void *data, int (*compare)(const void *lhs, const void *rhs));
/* xsort.c */
void xsort(void *a, size_t n, size_t es, int (*cmp)(const void *lhs, const void *rhs));
/* attr_lib.c */
int attr_get(const char *userid, int key, void *value);
int attr_put(const char *userid, int key, const void *value);
int attr_step(const char *userid, int key, int dflt, int step);
/* header.c */
void hdr_fpath(char *fpath, const char *folder, const HDR *hdr);
int hdr_stamp(const char *folder, int token, HDR *hdr, char *fpath);
/* dns.c */
void dns_init(void);
int dns_query(const char *name, int qtype, querybuf *ans);
ip_addr dns_addr(const char *name);
void dns_ident(int sock, const ip_addr *from, char *rhost, int rhost_sz, char *ruser, int ruser_sz);
int dns_name(const ip_addr *addr, char *name, int name_sz);
int dns_openip(const ip_addr *addr, int port);
int dns_open(const char *host, int port);
int dns_smtp(char *host);
/* string.c */
GCC_NONNULLS GCC_RET_NONNULL char *str_pcpy(char *dst, const char *src);
GCC_NONNULLS void str_ansi(char *dst, const char *str, int max);
GCC_NONNULLS void str_cat(char *dst, const char *s1, const char *s2);
GCC_NONNULLS GCC_PURE int str_casecmp(const char *s1, const char *s2);
GCC_NONNULLS GCC_PURE int str_casecmp_dbcs(const char *s1, const char *s2);
GCC_NONNULLS void str_split_2nd(char *dst, const char *src);
GCC_NONNULLS GCC_PURE enum DbcsState dbcs_state(const char *str, size_t idx);
GCC_NONNULLS GCC_PURE enum DbcsState dbcs_nstate(const char *str GCC_NONSTRING, size_t idx, size_t len);
GCC_NONNULLS GCC_PURE enum DbcsState dbcs_state_ansi(const char *str, size_t idx);
GCC_NONNULLS GCC_PURE enum DbcsState dbcs_nstate_ansi(const char *str GCC_NONSTRING, size_t idx, size_t len);
GCC_NONNULLS GCC_RET_NONNULL char *str_dup(const char *src, int pad);
GCC_NONNULLS void setdirpath_root(char *fpath, const char *folder, const char *fname);
GCC_NONNULLS void setdirpath(char *fpath, const char *direct, const char *fname);
GCC_NONNULLS int from_parse(const char *from, char *addr, char *nick);
GCC_NONNULLS GCC_PURE bool str_has(const char *list, const char *tag);
GCC_NONNULLS GCC_PURE int str_hash_mult(const char *str, unsigned int seed, unsigned int mult_base);
GCC_NONNULLS GCC_PURE int str_hash(const char *str, unsigned int seed);
GCC_NONNULLS GCC_PURE int str_hash2(const char *str, unsigned int seed);
GCC_NONNULLS GCC_PURE int hash32(const char *str);
GCC_NONNULLS GCC_PURE int str_len_nospace(const char *str);
GCC_NONNULLS void str_lower(char *dst, const char *src);
GCC_NONNULLS void str_lower_dbcs(char *dst, const char *src);
GCC_NONNULLS GCC_PURE size_t str_nmove_ansi(const char *str GCC_NONSTRING, size_t idx, ssize_t diff, size_t len);
GCC_NONNULLS GCC_PURE int str_ncasecmp(const char *s1, const char *s2, int n);
GCC_NONNULLS GCC_PURE int str_ncasecmp_dbcs(const char *s1, const char *s2, int n);
GCC_NONNULLS void str_rstrip_tail(char *str);
GCC_NONNULLS ssize_t str_scpy(char *dst, const char *src GCC_NONSTRING, size_t siz);
GCC_NONNULLS ssize_t str_sncpy(char *dst, const char *src GCC_NONSTRING, size_t siz, size_t n);
GCC_NONNULLS GCC_RET_NONNULL char *str_ndup(const char *src GCC_NONSTRING, int len);
GCC_NONNULLS GCC_PURE size_t str_nlen(const char *str GCC_NONSTRING, size_t maxlen);
GCC_NONNULLS GCC_PURE bool str_pat(const char *str, const char *pat);
GCC_NONNULLS GCC_RET_NONNULL char *str_rev_tail(char *dst, const char *src);
GCC_NONNULLS int rle_encode(char *str);
GCC_NONNULLS GCC_PURE char *str_casestr(const char *str, const char *tag);
GCC_NONNULLS GCC_PURE char *str_casestr_dbcs(const char *str, const char *tag);
GCC_NONNULLS GCC_PURE GCC_RET_NONNULL char *str_tail(const char *str);
GCC_NONNULLS void str_rtrim(char *buf);
GCC_NONNULL(1) GCC_RET_NONNULL char *str_ttl_hdrmode(const char *title, enum HdrMode *pmode);
GCC_NONNULLS GCC_PURE GCC_RET_NONNULL char *str_ttl(const char *title);
GCC_NONNULLS void str_xor(char *dst GCC_NONSTRING, const char *src);
GCC_NONNULLS size_t strlcat(char *dst, const char *src, size_t siz);
GCC_NONNULLS size_t strlcpy(char *dst, const char *src, size_t siz);
/* date.c */
void str_stamp(char *str, const time_t *chrono);
char *Btime(const time_t *clock);
char *Ctime(const time_t *clock);
char *Etime(const time_t *clock);
char *Atime(const time_t *clock);
char *Now(void);
/* mime.c */
GCC_CONSTEXPR int qp_code(int x);
GCC_CONSTEXPR int base64_code(int x);
char *mm_getencode(char *str, char *code);
void mm_getcharset(const char *str, char *charset, int size);
int mmdecode(const char *src, char encode, char *dst);
void mmdecode_str(char *str);
void output_rfc2047_qp(FILE *fp, const char *prefix, const char *str, const char *charset, const char *suffix);
/* passwd.c */
char *getrandom_bytes(char *buf, size_t buflen);
void explicit_zero_bytes(char *buf, size_t buflen);
char *genpasswd(char *pw, int mode);
char *gensignature(char *pw);
int chkpasswd(const char *passwd, const char *passhash, char *test);
int chksignature(const char *passwd, char *test);
/* proc.c */
int proc_runv(const char *path, const char *argv[]);
GCC_SENTINEL(0) int proc_runl(const char *path, const char *arg0, ...);
int proc_runv_bg(const char *path, const char *argv[]);
GCC_SENTINEL(0) int proc_runl_bg(const char *path, const char *arg0, ...);
/* logger.c */
GCC_FORMAT(3, 4) GCC_NONNULL(1, 3) void loggerf(const Logger *logger, enum LogLevel level, const char *format, ...);
GCC_NONNULLS void logger_tag(const Logger *logger, const char *tag, const char *msg, void (*formatter)(char *buf, size_t size, const char *mode, const char *msg));
/* xwrite.c */
int xwrite(int fd, const char *data, int size);

#ifdef __cplusplus
}  /* extern "C" */
#endif

/* `IS_DBCS_*()`: Whether the byte at character position `_idx` in the string `_str` is the leading/trailing DBCS byte */

/* Not boundary-checking version
 * `idx` must smaller than the length of `str`, otherwise buffer overrun may occur */
#define IS_DBCS_LEAD(_str, _idx) \
    (bool)(dbcs_state(_str, _idx) & DBCS_LEAD)
#define IS_DBCS_TRAIL(_str, _idx) \
    (bool)(dbcs_state(_str, _idx) & DBCS_TRAIL)

/* Boundary-checking version */
#define IS_DBCS_LEAD_N(_str, _idx, _len) \
    (bool)(dbcs_nstate(_str, _idx, _len) & DBCS_LEAD)
#define IS_DBCS_TRAIL_N(_str, _idx, _len) \
    (bool)(dbcs_nstate(_str, _idx, _len) & DBCS_TRAIL)

/* `IS_DBCS_*_ANSI()`: Whether the byte at character position `_idx` in the string `_str` is the leading/trailing DBCS byte after ignoring ANSI escapes */

/* Not boundary-checking version
 * `idx` must smaller than the length of `str`, otherwise buffer overrun may occur */
#define IS_DBCS_LEAD_ANSI(_str, _idx) \
    (bool)(dbcs_state_ansi(_str, _idx) & DBCS_LEAD)
#define IS_DBCS_TRAIL_ANSI(_str, _idx) \
    (bool)(dbcs_state_ansi(_str, _idx) & DBCS_TRAIL)

/* Boundary-checking version */
#define IS_DBCS_LEAD_ANSI_N(_str, _idx, _len) \
    (bool)(dbcs_nstate_ansi(_str, _idx, _len) & DBCS_LEAD)
#define IS_DBCS_TRAIL_ANSI_N(_str, _idx, _len) \
    (bool)(dbcs_nstate_ansi(_str, _idx, _len) & DBCS_TRAIL)

/* Helper macros for fixed-size `time_t` */
#define str_stamp_any(_str, _chrono) \
    str_stamp(_str, &TEMPLVAL(time_t, {*(_chrono)}))
#define Btime_any(_clock) \
    Btime(&TEMPLVAL(time_t, {*(_clock)}))
#define Ctime_any(_clock) \
    Ctime(&TEMPLVAL(time_t, {*(_clock)}))
#define Etime_any(_clock) \
    Etime(&TEMPLVAL(time_t, {*(_clock)}))
#define Atime_any(_clock) \
    Atime(&TEMPLVAL(time_t, {*(_clock)}))

/* `proc_runl` without the need of `arg0` and trailing `NULL` */
/* Use `PROC_CMD(path, NULL)` when no other arguments are needed */
#define PROC_CMD(path, ...) proc_runl(path, path, __VA_ARGS__, NULL)

/* `proc_runl_bg` without the need of `arg0` and trailing `NULL` */
/* Use `PROC_CMD_BG(path, NULL)` when no other arguments are needed */
#define PROC_CMD_BG(path, ...) proc_runl_bg(path, path, __VA_ARGS__, NULL)

/* `loggerf` shorthands */
#define LOGF_EMERG(_logger, ...) \
    loggerf(_logger, LOGLV_EMERG, __VA_ARGS__)
#define LOGF_ALERT(_logger, ...) \
    loggerf(_logger, LOGLV_ALERT, __VA_ARGS__)
#define LOGF_CRIT(_logger, ...) \
    loggerf(_logger, LOGLV_CRIT, __VA_ARGS__)
#define LOGF_ERR(_logger, ...) \
    loggerf(_logger, LOGLV_ERR, __VA_ARGS__)
#define LOGF_WARN(_logger, ...) \
    loggerf(_logger, LOGLV_WARN, __VA_ARGS__)
#define LOGF_NOTICE(_logger, ...) \
    loggerf(_logger, LOGLV_NOTICE, __VA_ARGS__)
#define LOGF_INFO(_logger, ...) \
    loggerf(_logger, LOGLV_INFO, __VA_ARGS__)
#define LOGF_DEBUG(_logger, ...) \
    loggerf(_logger, LOGLV_DEBUG, __VA_ARGS__)

#endif  /* DAO_H */
