/*
    NoCeM-INNBBSD
    Yen-Ming Lee <leeym@cae.ce.ntu.edu.tw>
*/

#ifndef NOCEM_H
#define NOCEM_H

#define NCMINNBBSVER    "NoCeM-INNBBSD-0.8"     /* ª©¥»«Å§i */


#define TEXT            0
#define NCMHDR          1
#define NCMBDY          2

#define NOPGP           -1
#define PGPGOOD         0
#define PGPBAD          1
#define PGPUN           2

#define P_OKAY          0
#define P_FAIL          -1
#define P_UNKNOWN       -2
#define P_DISALLOW      -3

#define MAXSPAMMID      10000
#define LINELEN         512


/* bbslib.c */
extern ncmperm_t *NCMPERM;
extern int NCMCOUNT;

/* receive_article.c */
extern newsfeeds_t *search_newsfeeds_bygroup(char *newsgroup);

/* nocem.c */
extern ncmperm_t *search_issuer(char *issuer, char *type);
extern int receive_nocem(void);

#endif  /* NOCEM_H */
