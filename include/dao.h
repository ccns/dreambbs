#ifndef	_DAO_H_
#define	_DAO_H_


#include <stdio.h>
#include <sys/types.h>


#ifndef	NULL
#define	NULL	0			/* ((char *) 0) */
#endif


#ifndef	BLK_SIZ
#define BLK_SIZ         4096		/* disk I/O block size */
#endif


#ifndef	REC_SIZ
#define REC_SIZ         4096		/* disk I/O record size */
#endif


/* Thor.981206: lkchu patch */
extern char radix32[32];


#include "hdr.h"			/* prototype */
#include "dns.h"			/* dns type */
#include "splay.h"			/* splay type */

/* is_alnum.c */
int is_alnum(int ch);
/* is_alpha.c */
int is_alpha(int ch);
/* is_fname.c */
int is_fname(char *str);
/* is_fpath.c */
int is_fpath(char *path);
/* not_addr.c */
int not_addr(char *addr);
/* str_add.c */
char *str_add(char *dst, char *src);
/* str_cat.c */
void str_cat(char *dst, char *s1, char *s2);
/* str_cmp.c */
int str_cmp(char *s1, char *s2);
/* str_decode.c */
char *mm_getencode(unsigned char *str, char *code);
void mm_getcharset(const char *str, char *charset, int size);
int mmdecode(unsigned char *src, unsigned char encode, unsigned char *dst);
void str_decode(unsigned char *str);
/* str_dup.c */
char *str_dup(char *src, int pad);
/* str_folder.c */
void str_folder(char *fpath, char *folder, char *fname);
/* str_fpath.c */
void setdirpath(char *fpath, char *direct, char *fname);
/* str_from.c */
int str_from(char *from, char *addr, char *nick);
/* str_has.c */
int str_has(char *list, char *tag);
/* str_hash.c */
int str_hash(char *str, int seed);
/* str_len.c */
int str_len(char *str);
/* str_lower.c */
void str_lower(char *dst, char *src);
/* str_ncmp.c */
int str_ncmp(char *s1, char *s2, int n);
/* str_ncpy.c */
void str_ncpy(char *dst, char *src, int n);
/* str_ndup.c */
char *str_ndup(char *src, int len);
/* str_passwd.c */
char *genpasswd(char *pw);
int chkpasswd(char *passwd, char *test);
/* str_pat.c */
int str_pat(const char *str, const char *pat);
/* str_rev.c */
char *str_rev(char *dst, char *src);
/* str_rle.c */
int str_rle(unsigned char *str);
/* str_stamp.c */
void str_stamp(char *str, time_t *chrono);
/* str_str.c */
char *str_str(char *str, char *tag);
/* str_tail.c */
char *str_tail(char *str);
/* str_time.c */
char *Btime(time_t *clock);
char *Ctime(time_t *clock);
char *Etime(time_t *clock);
char *Atime(time_t *clock);
char *Now(void);
/* str_hash2.c */
int str_hash2(char *str, int seed);
/* str_trim.c */
void str_trim(char *buf);
/* str_ttl.c */
char *str_ttl(char *title);
/* str_xor.c */
void str_xor(unsigned char *dst, unsigned char *src);
/* url_encode.c */
void url_encode(unsigned char *dst, unsigned char *src);
/* str_ansi.c */
void str_ansi(char *dst, char *str, int max);
/* str_lcpy.c */
size_t strlcpy(char *dst, const char *src, size_t siz);
/* str_lcat.c */
size_t strlcat(char *dst, const char *src, size_t siz);
/* str_sub.c */
char *str_sub(char *str, char *tag);
/* str_lowest.c */
void str_lowest(char *dst, char *src);
/* strlcat.c */
size_t strlcat(char *dst, const char *src, size_t siz);
/* strlcpy.c */
size_t strlcpy(char *dst, const char *src, size_t siz);
/* archiv32.c */
void archiv32(time_t chrono, char *fname);
/* archiv32m.c */
void archiv32m(time_t chrono, char *fname);
/* chrono32.c */
time_t chrono32(char *str);
/* hash32.c */
int hash32(unsigned char *str);
/* radix32.c */
/* f_cat.c */
void f_cat(char *fpath, char *msg);
/* f_cp.c */
int f_cp(char *src, char *dst, int mode);
/* f_img.c */
char *f_img(char *fpath, int *fsize);
/* f_ln.c */
int f_ln(char *src, char *dst);
/* f_map.c */
char *f_map(char *fpath, int *fsize);
/* f_mode.c */
int f_mode(char *fpath);
/* f_mv.c */
int f_mv(char *src, char *dst);
/* f_new.c */
FILE *f_new(char *fold, char *fnew);
/* f_open.c */
int f_open(char *fpath);
/* f_path.c */
void brd_fpath(char *fpath, char *board, char *fname);
void gem_fpath(char *fpath, char *board, char *fname);
void usr_fpath(char *fpath, char *user, char *fname);
/* f_rm.c */
int f_rm(char *fpath);
/* f_suck.c */
void f_suck(FILE *fp, char *fpath);
/* mak_dirs.c */
void mak_dirs(char *fpath);
/* f_lock.c */
int f_exlock(int fd);
int f_unlock(int fd);
/* rec_add.c */
int rec_add(char *fpath, void *data, int size);
/* rec_num.c */
int rec_num(char *fpath, int size);
/* rec_del.c */
int rec_del(char *data, int size, int pos, int (*fchk)(void), int (*fdel)(void));
/* rec_get.c */
int rec_get(char *fpath, void *data, int size, int pos);
/* rec_loc.c */
int rec_loc(char *data, int size, int (*fchk)(void));
/* rec_ins.c */
int rec_ins(char *fpath, void *data, int size, int pos, int num);
/* rec_mov.c */
int rec_mov(char *data, int size, int from, int to);
/* rec_put.c */
int rec_put(char *fpath, void *data, int size, int pos);
int rec_put2(char *fpath, void *data, int size, int pos, int (*fchk)(void));
/* rec_bot.c */
int rec_bot(char *fpath, void *data, int size);
/* rec_ref.c */
int rec_ref(char *fpath, void *data, int size, int pos, int (*fchk)(void), void (*fref)(void));
/* rec_sync.c */
int rec_sync(char *fpath, int size, int (*fsync)(void), int (*fchk)(void));
/* hdr_fpath.c */
void hdr_fpath(char *fpath, char *folder, HDR *hdr);
/* hdr_stamp.c */
int hdr_stamp(char *folder, int token, HDR *hdr, char *fpath);
/* shm.c */
void *shm_new(int shmkey, int shmsize);
/* splay.c */
SplayNode *splay_in(SplayNode *top, void *data, int (*compare)(void));
/* acl_addr.c */
int acl_addr(char *acl, char *addr);
/* acl_has.c */
int acl_has(char *acl, char *user, char *host);
/* xsort.c */
void xsort(void *a, size_t n, size_t es, int (*cmp)(void));
/* xwrite.c */
int xwrite(int fd, char *data, int size);
/* dl_lib.c */
void *DL_get(char *name);
int DL_func(char *name, ...);
/* attr_lib.c */
int attr_get(char *userid, int key, void *value);
int attr_put(char *userid, int key, void *value);
int attr_step(char *userid, int key, int dflt, int step);
/* rfc2047.c */
void output_rfc2047_qp(FILE *fp, char *prefix, char *str, char *charset, char *suffix);
/* keeplog.c */
void keeplog(char *fnlog, char *board, char *title, int mode);

#endif	/* _DAO_H_ */
