#ifndef	_HDR_H_
#define	_HDR_H_

#include <sys/types.h>

/* ----------------------------------------------------- */
/* DIR of post / mail struct : 256 bytes		 */
/* ----------------------------------------------------- */

typedef struct
{
  time_t chrono;		/* timestamp */
  int xmode;

  int xid;			/* reserved 保留*/

  char xname[32];		/* 檔案名稱 */
  char owner[47];		/* 作者 (E-mail address) */
  time_t stamp;			/* 未讀標記 */
  unsigned int expire;          /* 自動刪除 */
  char lastrecommend[13];	/* 最後推文者 */
  time_t pushtime;		/* 最後推文時間 */
//  unsigned int recommend;	/* 推薦文章 */
  short modifytimes;		/* 改文次數 */
  short recommend;		/* 推薦文章 */
  char nick[50];		/* 暱稱 */

  char date[9];			/* [96/12/01] */
  /* Thor.990329:特別注意, date只供顯示用, 不作比較, 以避免 y2k 問題,
                 定義 2000為 00, 2001為01 */

  char title[73];		/* 主題 (TTLEN + 1) */
}          HDR;

/* gopher url <A6>r<A6><EA><A1>Gxname + owner + nick + date */
#define        GEM_URLEN       (32 + 80 + 50 + 9 - 1)

/* ----------------------------------------------------- */
/* post.xmode 的定義					 */
/* ----------------------------------------------------- */

#define POST_READ	       0x000000001	/* already read */
#define POST_MARKED	       0x000000002	/* marked */
#define POST_GEM	       0x000000004	/* gemed */
#define POST_CANCEL	       0x000000040	/* canceled */
#define POST_DELETE	       0x000000080	/* deleted */
#define	POST_INCOME	       0x000000100	/* 轉信進來的 */
#define	POST_EMAIL	       0x000000200	/* E-mail post 進來的 */
#define	POST_OUTGO	       0x000000400	/* 須轉信出去 */
#define	POST_RESTRICT	   0x000000800	/* 限制級文章，須 manager 才能看 */
#define POST_MDELETE	   0x000100000	/* 版主或站長刪除 */
#define POST_LOCK	       0x000200000	/* 文章鎖定 */
#define POST_COMPLETE	   0x000400000	/* 完成處理 */
#define POST_MODIFY	       0x000800000	/* 修改過了 */
#define POST_CURMODIFY	   0x001000000	/* 正在修改中 */
#define	POST_EXPIRE	       0x002000000	/* 一個禮拜後自動刪除 */
#define POST_BOTTOM	       0x004000000	/* 置底文 */
//#define POST_BOTTOM_ORIG 0x008000000	/* 置底本文 */
#define POST_RECOMMEND	   0x008000000	/* 推過的文 */
#define POST_RECOMMEND_ING 0x010000000  /* 正在推文*/

/* ----------------------------------------------------- */
/* mail.xmode 的定義					 */
/* ----------------------------------------------------- */


#define MAIL_READ	0x00000001	/* already read */
#define MAIL_MARKED	0x00000002	/* marked */
#define MAIL_REPLIED	0x00000004	/* 已經回過信了 */
#define	MAIL_MULTI	0x00000008	/* mail list */
#define	MAIL_HOLD	0x00000010	/* 自存底稿 */
#define	MAIL_NOREPLY	0x00000020	/* 不可回信 */
#define MAIL_DELETE	0x00000080	/* 將遭刪除 */

#define	MAIL_INCOME	0x00000100	/* bbsmail 進來的 */


/* ----------------------------------------------------- */
/* gem(gopher).xmode 的定義				 */
/* ----------------------------------------------------- */


#define	GEM_FULLPATH	0x00008000	/* 絕對路徑 */

#define	GEM_RESTRICT	0x00000800	/* 限制級精華區，須 manager 才能看 */
#define	GEM_RESERVED	0x00001000	/* 限制級精華區，須 sysop or board or gem 才能更改 */
#define	GEM_LOCK	0x00002000	/* 超限制級，須 sysop 才能更改 */

#define	GEM_FOLDER	0x00010000	/* folder / article */
#define	GEM_BOARD	0x00020000	/* 看板精華區 */
#define	GEM_GOPHER	0x00040000	/* gopher */
#define	GEM_HTTP	0x00080000	/* http */
#define	GEM_EXTEND	0x80000000	/* 延伸式 URL */


#define	HDR_URL		(GEM_GOPHER | GEM_HTTP)
#define	HDR_LINK	0x00000400	/* link() */
#define	HDR_COPY	0x00000800	/* copy() */

#endif	/* _HDR_H_ */
