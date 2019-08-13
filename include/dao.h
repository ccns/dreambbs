#ifndef DAO_H
#define DAO_H

#include <stdio.h>
#include <sys/types.h>

#ifndef NULL
#define NULL            0               /* ((char *) 0) */
#endif

#ifndef BLK_SIZ
#define BLK_SIZ         4096            /* disk I/O block size */
#endif

#ifndef REC_SIZ
#define REC_SIZ         4096            /* disk I/O record size */
#endif

/* Thor.981206: lkchu patch */
extern char radix32[32];

#include "hdr.h"                        /* prototype */
#include "dns.h"                        /* dns type */
#include "splay.h"                      /* splay type */

/* acl.c */
int acl_addr(char *acl, char *addr);
int acl_has(const char *acl, const char *user, char *host);
/* chrono32.c */
time_t chrono32(char *str);
/* file.c */
void f_cat(const char *fpath, const char *msg);
int f_cp(const char *src, const char *dst, int mode);
char *f_img(const char *fpath, int *fsize);
int f_ln(const char *src, const char *dst);
int f_exlock(int fd);
int f_unlock(int fd);
char *f_map(char *fpath, int *fsize);
int f_mode(const char *fpath);
int f_mv(const char *src, const char *dst);
FILE *f_new(char *fold, char *fnew);
int f_open(char *fpath);
void brd_fpath(char *fpath, const char *board, const char *fname);
void gem_fpath(char *fpath, char *board, char *fname);
void usr_fpath(char *fpath, const char *user, const char *fname);
int f_rm(char *fpath);
void f_suck(FILE *fp, const char *fpath);
void mak_dirs(char *fpath);
/* isnot.c */
int is_alnum(int ch);
int is_alpha(int ch);
int is_fname(char *str);
int is_fpath(char *path);
int not_addr(char *addr);
/* radix32.c */
/* shm.c */
void *shm_new(int shmkey, int shmsize);
/* url_encode.c */
void url_encode(char *dst, const char *src);
/* archiv32.c */
void archiv32(time_t chrono, char *fname);
void archiv32m(time_t chrono, char *fname);
/* dl_lib.c */
void *DL_get(const char *name);
int DL_func(char *name, ...);
/* record.c */
int rec_add(const char *fpath, void *data, int size);
int rec_bot(char *fpath, void *data, int size);
int rec_del(const char *fpath, int size, int pos, int (*fchk)(const void *obj), int (*fdel)(void *obj));
int rec_get(const char *fpath, void *data, int size, int pos);
int rec_ins(char *fpath, void *data, int size, int pos, int num);
int rec_loc(const char *fpath, int size, int (*fchk)(const void *obj));
int rec_mov(char *data, int size, int from, int to);
int rec_num(const char *fpath, int size);
int rec_put(const char *fpath, void *data, int size, int pos);
int rec_put2(char *fpath, void *data, int size, int pos, int (*fchk)(const void *obj));
int rec_ref(const char *fpath, void *data, int size, int pos, int (*fchk)(const void *obj), void (*fref)(void *obj, const void *ref));
int rec_sync(const char *fpath, int size, int (*fsync)(const void *lhs, const void *rhs), int (*fchk)(const void *obj));
int rec_append(char *fpath, void *data, int size);
/* splay.c */
SplayNode *splay_in(SplayNode *top, void *data, int (*compare)(const void *lhs, const void *rhs));
/* xsort.c */
void xsort(void *a, size_t n, size_t es, int (*cmp)(const void *lhs, const void *rhs));
/* attr_lib.c */
int attr_get(char *userid, int key, void *value);
int attr_put(char *userid, int key, void *value);
int attr_step(char *userid, int key, int dflt, int step);
/* header.c */
void hdr_fpath(char *fpath, const char *folder, HDR *hdr);
int hdr_stamp(char *folder, int token, HDR *hdr, char *fpath);
/* dns.c */
void dns_init(void);
unsigned long dns_addr(char *name);
void dns_ident(int sock, struct sockaddr_in *from, char *rhost, char *ruser);
int dns_name(unsigned char *addr, char *name);
int dns_open(const char *host, int port);
int dns_smtp(char *host);
/* rfc2047.c */
void output_rfc2047_qp(FILE *fp, const char *prefix, char *str, char *charset, const char *suffix);
/* string.c */
char *str_add(char *dst, char *src);
void str_ansi(char *dst, const char *str, int max);
void str_cat(char *dst, const char *s1, const char *s2);
int str_cmp(const char *s1, const char *s2);
void str_cut(char *dst, char *src);
int qp_code(register int x);
int base64_code(register int x);
char *mm_getencode(char *str, char *code);
void mm_getcharset(const char *str, char *charset, int size);
int mmdecode(const char *src, char encode, char *dst);
void str_decode(char *str);
char *str_dup(char *src, int pad);
void str_folder(char *fpath, char *folder, const char *fname);
void setdirpath(char *fpath, char *direct, const char *fname);
int str_from(char *from, char *addr, char *nick);
int str_has(char *list, char *tag);
int str_hash2(char *str, int seed);
int str_hash(const char *str, int seed);
int str_len(char *str);
void str_lower(char *dst, const char *src);
void str_lowest(char *dst, char *src);
int str_ncmp(const char *s1, const char *s2, int n);
void str_strip(char *str);
void str_ncpy(char *dst, const char *src, int n);
char *str_ndup(char *src, int len);
char *getrandom_bytes(char *buf, size_t buflen);
void explicit_zero_bytes(char *buf, size_t buflen);
char *genpasswd(char *pw, int mode);
char *gensignature(char *pw);
int chkpasswd(char *passwd, char *passhash, char *test);
int chksignature(char *passwd, char *test);
int str_pat(const char *str, const char *pat);
char *str_rev(char *dst, char *src);
int str_rle(char *str);
char *str_str(const char *str, const char *tag);
char *str_sub(char *str, char *tag);
char *str_tail(char *str);
void str_trim(char *buf);
const char *str_ttl(const char *title);
void str_xor(char *dst, const char *src);
size_t strlcat(char *dst, const char *src, size_t siz);
size_t strlcpy(char *dst, const char *src, size_t siz);
int hash32(const char *str);
/* date_str.c */
void str_stamp(char *str, time_t *chrono);
char *Btime(time_t *clock);
char *Ctime(time_t *clock);
char *Etime(time_t *clock);
char *Atime(time_t *clock);
char *Now(void);
void syncnow(void);
/* xwrite.c */
int xwrite(int fd, char *data, int size);

#endif  /* DAO_H */
