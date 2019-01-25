/*-------------------------------------------------------*/
/* proto.h      ( NTHU CS MapleBBS Ver 3.02 )            */
/*-------------------------------------------------------*/
/* target : prototype and macros                         */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/

#ifndef _PROTO_H_
#define _PROTO_H_

#ifdef M3_USE_PFTERM
#include "pfterm.h"
#endif

/* ----------------------------------------------------- */
/* External function declarations                        */
/* ----------------------------------------------------- */

/* OS */
char *genpasswd(char *pw);

/* ----------------------------------------------------- */
/* prototypes                                            */
/* ----------------------------------------------------- */

/* acct.c */
void logitfile(char *file, char *key, char *msg);
void addmoney(int addend, char *userid);
void addpoint1(int addend, char *userid);
void addpoint2(int addend, char *userid);
void keeplog(char *fnlog, char *board, char *title, int mode);
int acct_load(ACCT *acct, char *userid);
void acct_save(ACCT *acct);
int acct_userno(char *userid);
int acct_get(char *msg, ACCT *acct);
void x_file(int mode, char *xlist[], char *flist[]);
int check_admin(char *name);
void bitmsg(char *msg, char *str, int level);
unsigned int bitset(unsigned int pbits, int count, int maxon, char *msg, char *perms[]);
void acct_show(ACCT *u, int adm);
void bm_setup(ACCT *u, int adm);
void deny_log_email(char *mail, time_t deny);
int add_deny(ACCT *u, int adm, int cross);
void acct_setup(ACCT *u, int adm);
int u_info(void);
int m_user(void);
int m_bmset(void);
int ban_addr(char *addr);
void check_nckuemail(char *email);
int find_same_email(char *mail, int mode);
int u_addr(void);
void su_setup(ACCT *u);
int u_setup(void);
int ue_setup(void);
int u_lock(void);
int u_xfile(void);
int m_newbrd(void);
void brd_edit(int bno);
int a_editbrd(void);
int u_verify(void);
/* bbsd.c */
void blog(char *mode, char *msg);
void u_exit(char *mode);
void abort_bbs(void);
/* board.c */
void brh_get(time_t bstamp, int bhno);
int brh_unread(time_t chrono);
void brh_visit(int mode);
void brh_add(time_t prev, time_t chrono, time_t next);
void remove_perm(void);
int Ben_Perm(BRD *bhdr, unsigned int ulevel);
int bstamp2bno(time_t stamp);
void brh_load(void);
void brh_save(void);
void XoPost(int bno);
int Select(void);
int Class(void);
void check_new(BRD *brd);
int Favorite(void);
void board_main(void);
int Boards(void);
int brd_list(int reciper);
/* cache.c */
void sem_init(void);
void ushm_init(void);
void utmp_mode(int mode);
int utmp_new(UTMP *up);
void utmp_free(void);
UTMP *utmp_find(int userno);
UTMP *pid_find(int pid);
int utmp_count(int userno, int show);
int cmpclasstable(const void *ptr);
void classtable_free(void);
void classtable_main(void);
void bshm_init(void);
int brd_bno(const char *bname);
int observeshm_find(int userno);
void observeshm_load(void);
void observeshm_init(void);
void count_update(void);
void count_load(void);
void count_init(void);
void fwshm_load(void);
void fwshm_init(void);
void fshm_init(void);
int film_out(int tag, int row);
UTMP *utmp_check(char *userid);
/* edit.c */
void ve_string(char *str);
char *tbf_ask(void);
FILE *tbf_open(void);
void ve_backup(void);
void ve_recover(void);
void ve_header(FILE *fp);
int ve_subject(int row, char *topic, char *dft);
int vedit(char *fpath, int ve_op);
/* gem.c */
int url_fpath(char *fpath, char *folder, HDR *hdr);
void brd2gem(BRD *brd, HDR *gem);
int gem_gather(XO *xo);
void XoGem(char *folder, char *title, int level);
void gem_main(void);
/* mail.c */
void ll_new(void);
void ll_add(char *name);
int ll_del(char *name);
int ll_has(char *name);
void ll_out(int row, int column, char *msg);
int bsmtp(char *fpath, char *title, char *rcpt, int method);
int bsmtp_file(char *fpath, char *title, char *rcpt);
int m_verify(void);
int m_total_size(void);
unsigned int m_quota(void);
int m_zip(void);
int m_query(char *userid);
void m_biff(int userno);
int m_setforward(void);
int m_setmboxdir(void);
int hdr_reply(int row, HDR *hdr);
int mail_external(char *addr);
int mail_send(char *rcpt, char *title);
void mail_reply(HDR *hdr);
void my_send(char *rcpt);
int m_send(void);
int mail_sysop(void);
int mail_list(void);
int tag_char(int chrono);
void hdr_outs(HDR *hdr, int cc);
int mbox_send(XO *xo);
int mail_stat(int mode);
int mbox_check(void);
void mbox_main(void);
/* menu.c */
int pad_view(void);
void vs_head(char *title, char *mid);
void clear_notification(void);
void movie(void);
char *check_info(char *input);
void menu(void);
/* more.c */
char *mgets(int fd);
void *mread(int fd, int len);
int more(char *fpath, char *footer);
/* post.c */
int cmpchrono(const void *hdr);
int checksum_find(char *fpath, int check, int state);
void btime_update(int bno);
void outgo_post(HDR *hdr, char *board);
void cancel_post(HDR *hdr);
void move_post(HDR *hdr, char *board, int by_bm);
void log_anonymous(char *fname);
int seek_log(char *title, int state);
int getsubject(int row, int reply);
int post_cross(XO *xo);
void post_history(XO *xo, HDR *fhdr);
int post_gem(XO *xo);
int post_tag(XO *xo);
int post_edit(XO *xo);
void header_replace(XO *xo, HDR *hdr);
int post_title(XO *xo);
int post_ban_mail(XO *xo);
void record_recommend(const int chrono, const char *const text);
int post_resetscore(XO *xo);
int post_recommend(XO *xo);
int post_manage(XO *xo);
int post_write(XO *xo);
/* banmail.c */
int BanMail(void);
void post_mail(void);
/* talk.c */
char *bmode(UTMP *up, int simple);
int is_boardpal(UTMP *up);
int is_pal(int userno);
int is_banmsg(int userno);
void pal_cache(void);
void aloha_sync(void);
void pal_sync(char *fpath);
int t_pal(void);
int t_bmw(void);
int bm_belong(char *board);
int XoBM(XO *xo);
void my_query(char *userid, int paling);
void bmw_edit(UTMP *up, char *hint, BMW *bmw, int cc);
void bmw_reply(int replymode);
int pal_list(int reciper);
void aloha(void);
int t_loginNotify(void);
void loginNotify(void);
int t_recall(void);
void talk_save(void);
void bmw_save(void);
void bmw_rqst(void);
int t_message(void);
int t_pager(void);
int t_cloak(XO *xo);
int t_query(void);
void talk_rqst(void);
void talk_main(void);
int check_personal_note(int newflag, char *userid);
void banmsg_cache(void);
void banmsg_sync(char *fpath);
int t_banmsg(void);

/* bbslua.c */
int bbslua(const char *fpath);
int bbslua_isHeader(const char *ps, const char *pe);

/* visio.c */
void bell(void);
#ifdef M3_USE_PFTERM
void ochar(int ch);
void outl(int line, char *msg);
void outr(char *str);
void oflush(void);
#else
void move(int y, int x);
void move_ansi(int y, int x);
void refresh(void);
void clear(void);
void clrtoeol(void);
void clrtobot(void);
void outc(int ch);
void outs(const char *str);
void scroll(void);
void rscroll(void);
void save_foot(screenline *slp);
void restore_foot(screenline *slp);
int vs_save(screenline *slp);
void vs_restore(screenline *slp);
void clearange(int from, int to);
void clrtohol(void);
#endif  /* #ifdef M3_USE_PFTERM */

void getyx(int *y, int *x);
int expand_esc_star_visio(char *buf, const char *src, int szbuf);
void outx(char *str);
void outz(char *msg);
void outf(char *str);
void prints(char *fmt, ...);
void cursor_save(void);
void cursor_restore(void);
int vmsg(const char *msg);
void zmsg(char *msg);
void vs_bar(char *title);
#ifndef M3_USE_PFTERM
void grayout(int y, int end, int level);
#endif  /* #ifndef M3_USE_PFTERM */
void add_io(int fd, int timeout);
int igetch(void);
BRD *ask_board(char *board, int perm, char *msg);
int vget(int line, int col, const char *prompt, char *data, int max, int echo);
int vans(char *prompt);
int vkey(void);

/* xover.c */
XO *xo_new(char *path);
XO *xo_get(char *path);
void xo_load(XO *xo, int recsiz);
void xo_fpath(char *fpath, char *dir, HDR *hdr);
int hdr_prune(char *folder, int nhead, int ntail, int post);
int xo_delete(XO *xo);
int Tagger(time_t chrono, int recno, int op);
void EnumTagHdr(HDR *hdr, char *dir, int locus);
int AskTag(char *msg);
int xo_uquery_lite(XO *xo);
int xo_uquery(XO *xo);
int xo_usetup(XO *xo);
int xo_getch(XO *xo, int ch);
void xover(int cmd);
void every_Z(void);
void every_U(void);
void every_B(void);
void every_S(void);
int xo_cursor(int ch, int pagemax, int num, int *pageno, int *cur, int *redraw);
/* favorite.c */
void favorite_main(void);
/* socket.c */
int Get_Socket(char *site, int *sock);
int POP3_Check(char *site, char *account, char *passwd);
int Ext_POP3_Check(char *site, char *account, char *passwd);
#ifdef M3_USE_PMORE
/* pmore.c */
int pmore(const char *fpath, int promptend);
#endif  /* #ifdef M3_USE_PMORE */
/* popupmenu.c */
int popupmenu_ans(char *desc[], char *title, int x, int y);
void popupmenu(MENU pmenu[], XO *xo, int x, int y);
int pmsg(char *msg);
int Every_Z_Screen(void);
/* window.c */
int popupmenu_ans2(char *desc[], char *title, int x, int y);
int pmsg2(char *msg);
/* myfavorite.c */
void brd2myfavorite(BRD *brd, HDR *gem);
int MyFavorite(void);
int myfavorite_find_chn(char *brdname);
void myfavorite_parse(char *fpath);
void myfavorite_main(void);
int class_add(XO *xo);

/* ----------------------------------------------------- */
/* macros                                                */
/* ----------------------------------------------------- */

#define TRACE   blog

#define dashd(fpath)    S_ISDIR(f_mode(fpath))
#define dashf(fpath)    S_ISREG(f_mode(fpath))

#define STR4(x)         ((x[0] << 24) + (x[1] << 16) + (x[2] << 8) + x[3])
                        /* Thor.980913: «OÃÒprecedence */

#endif                          /* _PROTO_H_ */
