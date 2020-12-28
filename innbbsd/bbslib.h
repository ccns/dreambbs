#ifndef BBSLIB_H
#define BBSLIB_H

#include "attrdef.h" /* Macros for implementation-defined attributes */

#ifdef __cplusplus
extern "C" {
#endif

/* bbslib.c */
extern int NLCOUNT;
extern nodelist_t *NODELIST;
GCC_PURE int nl_bynamecmp(const void *a, const void *b);

/* bbslib.c */
extern int NFCOUNT;
extern newsfeeds_t *NEWSFEEDS;
extern newsfeeds_t *NEWSFEEDS_B;
extern newsfeeds_t *NEWSFEEDS_G;
GCC_PURE int nf_byboardcmp(const void *a, const void *b);
GCC_PURE int nf_bygroupcmp(const void *a, const void *b);

/* bbslib.c */
extern int SPAMCOUNT;
extern spamrule_t *SPAMRULE;

/* bbslib.c */
int initial_bbs(void);
GCC_CHECK_FORMAT(1, 2) void bbslog(const char *fmt, ...);

/* convcode.c */
void b52gb(char *str);
void gb2b5(char *str);

/* rec_article.c */
void init_bshm(void);
int cancel_article(const char *msgid);
int receive_article(void);
#ifdef NoCeM
int receive_nocem(void);
int read_ncmperm(void);
#endif

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif  /* BBSLIB_H */
