#ifndef INNTOBBS_H
#define INNTOBBS_H

/* inntobbs.c */
extern const char *NODENAME;
extern char *BODY;
extern const char *DATE, *PATH, *MSGID, *POSTHOST, *CONTROL;
extern char *SUBJECT, *FROM, *GROUP, *SITE;

/* inntobbs.c */
extern int readlines(char *data);

/* history.c */
extern void HISmaint(void);
extern void HISadd(const char *msgid, const char *board, const char *xname);
extern int HISfetch(const char *msgid, char *board, char *xname);

#endif  /* INNTOBBS_H */
