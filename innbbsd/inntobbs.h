#ifndef INNTOBBS_H
#define INNTOBBS_H

#ifdef __cplusplus
extern "C" {
#endif

/* inntobbs.c */
extern const char *NODENAME;
extern char *BODY;
extern const char *DATE, *PATH, *MSGID, *POSTHOST, *CONTROL;
extern char *SUBJECT, *FROM, *GROUP, *SITE;

/* inntobbs.c */
int readlines(char *data);

/* history.c */
void HISmaint(void);
void HISadd(const char *msgid, const char *board, const char *xname);
int HISfetch(const char *msgid, char *board, char *xname);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif  /* INNTOBBS_H */
