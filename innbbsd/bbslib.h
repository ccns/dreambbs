#ifndef _BBSLIB_H_
#define _BBSLIB_H_

/* bbslib.c */
extern int NLCOUNT;
extern nodelist_t *NODELIST;
extern int nl_bynamecmp(nodelist_t *a, nodelist_t *b);

/* bbslib.c */
extern int NFCOUNT;
extern newsfeeds_t *NEWSFEEDS;
extern newsfeeds_t *NEWSFEEDS_B;
extern newsfeeds_t *NEWSFEEDS_G;
extern int nf_byboardcmp(newsfeeds_t *a, newsfeeds_t *b);
extern int nf_bygroupcmp(newsfeeds_t *a, newsfeeds_t *b);

/* bbslib.c */
extern int SPAMCOUNT;
extern spamrule_t *SPAMRULE;

/* bbslib.c */
extern int initial_bbs(void);
extern void bbslog(char *fmt, ...);

/* convcode.c */
extern void b52gb(char *str);
extern void gb2b5(char *str);

/* rec_article.c */
extern void init_bshm(void);
extern int cancel_article(char *msgid);
extern int receive_article(void);
#ifdef _NoCeM_
extern int receive_nocem(void);
extern int read_ncmperm(void);
#endif

#endif  /* _BBSLIB_H_ */
