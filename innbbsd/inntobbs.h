#ifndef INNTOBBS_H
#define INNTOBBS_H

/* inntobbs.c */
extern char *NODENAME;
extern char *BODY;
extern char *SUBJECT, *FROM, *DATE, *PATH, *GROUP, *MSGID, *SITE, *POSTHOST, *CONTROL;

/* inntobbs.c */
extern int readlines(char *data);

/* history.c */
extern void HISmaint(void);
extern void HISadd(char *msgid, char *board, char *xname);
extern int HISfetch(char *msgid, char *board, char *xname);

#endif  /* INNTOBBS_H */
