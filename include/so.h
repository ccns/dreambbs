/*-------------------------------------------------------*/
/* so.h         ( NCKU CCNS WindTop-DreamBBS 2.0 )       */
/*-------------------------------------------------------*/
/* Author: Wei-Cheng Yeh (IID) <iid@ccns.ncku.edu.tw>    */
/* Target: Prototype for shared object library           */
/* Create: 2019/05/07                                    */
/*-------------------------------------------------------*/

#ifndef PROTO_SO_H
#define PROTO_SO_H

/* Macros for implementation-defined attributes */
#include "attrdef.h"

#include "cppdef.h"

/* ----------------------------------------------------- */
/* prototypes                                            */
/* ----------------------------------------------------- */

#ifdef __cplusplus
extern "C" {
#endif

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
int m_resetsys(const void *arg);

/* so/aloha.c */
int t_aloha(void);

/* so/ascii.c */
void input_tools(void);

/* so/bank.c */
int money_back(void);
int bank_main(void);

/* so/bj.c */
int BlackJack(void);

/* so/brdstat.c */
int main_bstat(XO *xo);

/* so/chat.c */
int t_chat(void);

/* so/chatmenu.c */
int Chatmenu(void);

/* so/classtable2.c */
int main_classtable(void);

/* so/cleanrecommend.c */
int clean(XO *xo);

/* so/contact.c */
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
int Mine(void);

/* so/newboard.c */
int XoNewBoard(void);

/* so/observe.c */
int Observe_list(void);

/* so/passwd.c */
int new_passwd(void);

/* so/personal.c */
int personal_apply(void);
int personal_admin(void);

/* so/pip.c */
int p_pipple(void);

/* so/pipfun.c */

/* so/pnote.c */
int main_note(void);

/* so/same_mail.c */
int same_mail(char *mail);

/* so/shop.c */
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

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif  /* #define PROTO_SO_H */
