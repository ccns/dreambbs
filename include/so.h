/*-------------------------------------------------------*/
/* so.h         ( NCKU CCNS WindTop-DreamBBS 2.0 )       */
/*-------------------------------------------------------*/
/* Author: 37586669+IepIweidieng@users.noreply.github.com*/
/* Target: Prototype for shared object library           */
/* Create: 2019/05/07                                    */
/*-------------------------------------------------------*/

#ifndef PROTO_SO_H
#define PROTO_SO_H

/* Macros for implementation-defined attributes */

#ifndef GCC_CHECK_FORMAT
  #ifdef __GNUC__
    #define GCC_CHECK_FORMAT(ifmt, iarg)  __attribute__((format(printf, ifmt, iarg)))
  #else
    #define GCC_CHECK_FORMAT(ifmt, iarg)  /* Ignored */
  #endif
#endif

/* ----------------------------------------------------- */
/* prototypes                                            */
/* ----------------------------------------------------- */

/* so/admin.c */
int Admin(void);

/* so/adminutil.c */
int m_expire(void);
int mail_to_bm(void);
int mail_to_all(void);
int bm_check(void);
int user_check_bm(void);
int update_all(void);
int special_search(void);
int m_xfile(void);
int m_xhlp(void);
int reset1(void);
int reset2(void);
int reset3(void);
int reset4(void);
int reset5(void);
int reset6(void);
int reset7(void);

/* so/aloha.c */
int cmpbmw(const void *benz);
int t_aloha(void);

/* so/ascii.c */
void input_tools(void);

/* so/bank.c */
int point1_money(void);
int TransferAccount(void);
int money_back(void);
int bank_main(void);

/* so/bj.c */
void show_money(int m);
int print_card(int card, int x, int y);
int BlackJack(void);

/* so/brdstat.c */

/* so/chat.c */
int t_chat(void);

/* so/chatmenu.c */
int Chatmenu(void);

/* so/classtable2.c */
int show_classtable(int x, int y, const char *msg);
void show_icon_classtable(int x, int y, int mode);
void help_classtable(void);
int show_table(void);
int load_table(void);
int add_classtable(int x, int y);
int del_classtable(int x, int y);
int edit_classtable(int x, int y);
int init_classtable(void);
int main_classtable(void);

/* so/cleanrecommend.c */
int clean(XO *xo);

/* so/contact.c */
void contact_send(const CONTACT *contact);
int Contact(void);

/* so/gamef.c */

/* so/guessnum.c */
int mainNum(int fighting);
int guessNum(void);
int fightNum(void);

/* so/innbbs.c */
int a_innbbs(void);
/* so/list.c */
int List(void);

/* so/mailgem.c */
int mailgem_gather(XO *xo);
void mailgem_main(void);
int gcheck(int level, char *fpath);

/* so/memorandum.c */
int Memorandum(void);

/* so/mine.c */
void clrtokol(void);
void initMap(void);
int show_fasttime(void);
int load_fasttime(void);
int change_fasttime(int n, int t);
int countNeighbor(int y, int x, int bitmask);
void drawInfo(void);
void drawPrompt(void);
void drawMapLine(int y, int flShow);
void drawMap(int flShow);
void ExpandMap(int y, int x, int flTrace);
void TraceMap(int y, int x);
void playMine(void);
int Mine(void);

/* so/newboard.c */
int XoNewBoard(void);

/* so/observe.c */
int Observe_list(void);

/* so/passwd.c */
int new_passwd(void);

/* so/personal.c */
int personal_apply(void);
int sort_compare(const void *p1, const void *p2);
int personal_admin(void);

/* so/pip.c */
int p_pipple(void);

/* so/pipfun.c */
/* so/pnote.c */
int main_note(void);

/* so/same_mail.c */
int same_mail(char *mail);

/* so/shop.c */
int cloak_temp(void);
int hidefrom_temp(void);
int sysop(void);
int shop_main(void);

/* so/showvote.c */
int Showvote(XO *xo);

/* so/song.c */
int XoSongMain(void);
int XoSongSub(void);
int XoSongLog(void);
int AddRequestTimes(void);

/* so/violate.c */
int Violate(void);

/* so/vote.c */
int vote_result(XO *xo);
int XoVote(XO *xo);
int SystemVote(void);

/* so/xyz.c */
int x_siteinfo(void);

#endif  /* #define PROTO_SO_H */
