/*------------------------------------------------------*/
/* battr.h     ( YZU WindTopBBS Ver 3.02 )              */
/*------------------------------------------------------*/
/* author : gaod.bbs@bbs.yzu.edu.tw                     */
/* target : Board Attribution                           */
/* create : 2008/10/05                                  */
/*------------------------------------------------------*/

#ifndef BATTR_H
#define BATTR_H

/* ----------------------------------------------------- */
/* Board Attribution : flags in BRD.battr                */
/* ----------------------------------------------------- */

#define BRD_NOZAP        0x00000001       /* 不可 zap */
#define BRD_NOTRAN       0x00000002       /* 不轉信 */
#define BRD_NOCOUNT      0x00000004       /* 不計文章發表篇數 */
#define BRD_NOSTAT       0x00000008       /* 不納入熱門話題統計 */
#define BRD_NOVOTE       0x00000010       /* 不公佈投票結果於 [sysop] 板 */
#define BRD_ANONYMOUS    0x00000020       /* 匿名看板 */
#define BRD_NOFORWARD    0x00000040       /* lkchu.981201: 不可轉寄 */
#define BRD_LOGEMAIL     0x00000080       /* 自動附加e-mail */
#define BRD_NOBAN        0x00000100       /* 不擋信 */
#define BRD_NOLOG        0x00000200       /* 不紀錄站內違法 */
#define BRD_NOCNTCROSS   0x00000400       /* 不紀錄 cross post */
#define BRD_NOREPLY      0x00000800       /* 不能回文章 -- cache.090928: 禁止發表, 回推文 */
#define BRD_NOLOGREAD    0x00001000       /* 不紀錄看版閱讀率 */
#define BRD_CHECKWATER   0x00002000       /* 紀錄灌水次數 */
#define BRD_CHANGETITLE  0x00004000       /* 板主修改版名 */
#define BRD_MODIFY       0x00008000       /* 使用者不可修改文章 */
#define BRD_PRH          0x00010000       /* 關閉推薦文章 */
#define BRD_PUSHDISCON   0x00020000       /* 不可同ID連推 */
#define BRD_PUSHTIME     0x00040000       /* 不可快速連推 */
#define BRD_PUSHSNEER    0x00080000       /* 可以推噓文 */
#define BRD_PUSHDEFINE   0x00100000       /* 可以自訂推文動詞 */
#define BRD_NOTOTAL      0x00200000       /* 不統計看板使用紀錄 */
#define BRD_USIES        0x00400000       /* 觀看進板紀錄 */
#define BRD_BOTTOM       0x00800000       /* 置底功能 */
#define BRD_VALUE        0x01000000       /* 優良點數看板 */
#define BRD_26           0x02000000       /* 有問題勿使用 */
#define BRD_NOPHONETIC   0x04000000       /* 注音文限制*/
#define BRD_THRESHOLD    0x08000000       /* 發文限制看板 */
#define BRD_POSTFIX      0x10000000       /* 板主可自訂發文類別 */
#define BRD_RSS          0x20000000       /* RSS看板 */

/* ----------------------------------------------------- */
/* 各種旗標的中文意義                                    */
/* ----------------------------------------------------- */

#ifdef ADMIN_C

#define NUMATTRS        30

static const char *battrs[NUMATTRS] =
{
      "不可 Zap",
      "不轉信",
      "不記錄篇數",
      "不做熱門話題統計",
      "不公開選舉結果",
      "匿名看板",
      "不可轉寄轉貼文章",
      "自動附加e-mail",
      "不擋信",
#ifdef  HAVE_DETECT_VIOLATELAW
      "不紀錄站內違法",
#else
      "不紀錄站內違法(系統功\能未開啟)",
#endif
#ifdef  HAVE_DETECT_CROSSPOST
      "不紀錄 cross post",
#else
      "不紀錄 cross post(系統功\能未開啟)",
#endif
      "看板唯讀禁止發表回推",
      "不紀錄看版閱\讀率",
#ifdef  HAVE_RESIST_WATER
      "紀錄看版灌水次數",
#else
      "紀錄看版灌水次數(系統功\能未開啟)",
#endif
#ifdef  HAVE_BRDTITLE_CHANGE
      "板主修改版名",
#else
      "板主修改版名(系統功\能未開啟)",
#endif
#ifdef  HAVE_USER_MODIFY
      "使用者不可修改文章",
#else
      "使用者不可修改文章(系統功\能未開啟)",
#endif
#ifdef  HAVE_RECOMMEND
      "禁止推薦文章",
#else
      "禁止推薦文章(系統功\能未開啟)",
#endif
#ifdef MultiRecommend
      "不可同ID連推",
      "不可快速連推",
      "可以推噓文",
      "可以自訂推文動詞",
#else
      "不可同ID連推(系統功\能未開啟)",
      "不可快速連推(系統功\能未開啟)",
      "可以推噓文(系統功\能未開啟)",
      "可以自訂推文動詞(系統功\能未開啟)",
#endif
#ifdef HAVE_COUNT_BOARD
      "不紀錄看板資訊統計",
#else
      "不紀錄看板資訊統計(系統功\能未開啟)",
#endif
      "觀看進板紀錄",
#ifdef HAVE_POST_BOTTOM
      "文章置底",
#else
      "文章置底(系統功\能未開啟)",
#endif
      "優良點數看板",
      "未使用",
#ifdef ANTI_PHONETIC
      "禁止注音文",
#else
      "禁止注音文(系統功\能未開啟)",
#endif
      "發文限制看板",
      "板主可自訂發文類別",
      "RSS看板"
};
#endif  /* ADMIN_C */

#endif  /* BATTR_H */

