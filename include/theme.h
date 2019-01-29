/*-------------------------------------------------------*/
/* theme.h      ( NTHU CS MapleBBS Ver 3.10 )            */
/*-------------------------------------------------------*/
/* target : custom theme                                 */
/* create : 09/07/28                                     */
/* update : 17/07/26                                     */
/*-------------------------------------------------------*/


#ifndef THEME_H
#define THEME_H

/* ----------------------------------------------------- */
/* 暫存區                                                */
/* ----------------------------------------------------- */

#define NEWMAILMSG              "\x1b[41;37;5m 郵差衝進來了! \x1b[0;1;33;44m"
#define NEWPASSMSG              "\x1b[42;37;5m 收到神秘留言! \x1b[0;1;33;44m"

#define ICON_GAMBLED_BRD        "\x1b[1;31m賭\x1b[m"    /* 舉行賭盤中的看板 */
#define ICON_VOTED_BRD          "\x1b[1;33m投\x1b[m"    /* 舉行投票中的看板 */

//下一次推文時間限制
#define NEXTPUSHTIME 15

/* ----------------------------------------------------- */
/* 基本顏色定義，以利介面修改                            */
/* ----------------------------------------------------- */

#define ANSIRESET       "\x1b[m"        /* 終止顏色 */

#define COLOR1          "\x1b[34;46m"   /* footer/feeter 的前段顏色 */
#define COLOR2          "\x1b[31;47m"   /* footer/feeter 的後段顏色 */
#define COLOR3          "\x1b[30;47m"   /* neck 的顏色 */
#define COLOR4          "\x1b[1;44m"    /* 光棒 的顏色 */
#define COLOR5          "\x1b[34;47m"   /* more 檔頭的標題顏色 */
#define COLOR6          "\x1b[37;44m"   /* more 檔頭的內容顏色 */
#define COLOR7          "\x1b[0m"       /* 作者在線上的顏色 */


/* ----------------------------------------------------- */
/* 使用者名單顏色                                        */
/* ----------------------------------------------------- */

#define COLOR_PAL       "\x1b[1;32m"
#define COLOR_BAD       "\x1b[1;31m"
#define COLOR_CLOAK     "\x1b[1;35m"
#define COLOR_BOTH      "\x1b[1;36m"
#define COLOR_OPAL      "\x1b[1;33m"
#define COLOR_BOARDPAL  "\x1b[36m"

/* ----------------------------------------------------- */
/* 選單位置                                              */
/* ----------------------------------------------------- */



/* ----------------------------------------------------- */
/* 訊息字串：*_neck() 時的 necker 都抓出來定義在這       */
/* ----------------------------------------------------- */

#define NECKBOARD  "  [←]主選單 [→]閱\讀 [↑↓]選擇 [c]篇數 [y]載入 [/]搜尋 [s]看板 [h]說明\n" \
                   "\x1b[30;47m  %-7s看  板            %-33s人氣 板    主     \x1b[m"

//#define NECKGEM1 \
//                   "  [←]離開 [→]瀏覽 [f]模式 [C]暫存 [F]轉寄 [Z]下載 [h]說明 "

#define NECKGEM1 \
                   "  [←]離開 [→]瀏覽 [f]模式 [C]暫存 [h]說明 "
                   /*r2.20170802: disable some tips in case of confusion*/

#define NECKGEM2   "\x1b[44m" \
                   "  編號     主              題                            [編      選] [日  期]\x1b[m"

#define NECKPOST \
                   "  [←]離開 [→]閱\讀 [^P]發表 [b]備忘錄 [d]刪除 [V]投票 [TAB]精華區 [h]說明\n" \
                   "\x1b[30;47m  編號    日 期  作  者       文  章  標  題                                  \x1b[m"

#define NECKMAIL \
                   "  [←]離開 [→]讀信 [d]刪除 [R]回信 [s]寄信 [x]轉貼 [y]群組回信 [h]說明\n\x1b[44m" \
                   "  編號    日 期  作 者          信  件  標  題                                \x1b[m"

#define NECKVOTE \
                   "  [←]離開 [v]投票 [R]結果 [E]修改 [^P]舉行 [^Q]查詢/中止/改期 [h]說明\n\x1b[44m" \
                   "  編號  開票日  主辦人       投  票  宗  旨                                  \x1b[m"

#define NECKINNBBS "  [←]離開 [^P]新增 [d]刪除 [E]編輯 [/]搜尋 [Enter]詳細\n" \
            COLOR3 "  編號            內         容%*s                                               \x1b[m"


// cache.101119

#define NECKER_VOTE     "  [←]離開 [R]結果 [^P]舉行 [E]修改 [V]預覽 [^Q]改期 [o]名單 [h]說明\n" \

#define NECKER_VOTEALL  "  [↑/↓]上下 [PgUp/PgDn]上下頁 [Home/End]首尾 [→]投票 [←][q]離開\n" \


/* ----------------------------------------------------- */
/* 訊息字串：more() 時的 footer 都抓出來定義在這         */
/* ----------------------------------------------------- */


/* itoc.010914.註解: 單一篇，所以叫 FOOTER，都是 78 char */

#define FOOTER_POST     \
    COLOR1 " 文章選讀 " COLOR2 " (ry)回應 (=\\[]<>-+;'`)主題 (|?QA)搜尋標題作者 (kj)上下篇 (C)暫存   "

#define FOOTER_MORE     \
    COLOR1 " 瀏覽 P.%d (%d%%) " COLOR2 " (h)說明 [PgUp][PgDn][0][$]移動 (/n)搜尋 (C)暫存 (←q)結束 "


/* ----------------------------------------------------- */
/* 訊息字串：xo_foot() 時的 feeter 都抓出來定義在這      */
/* ----------------------------------------------------- */


/* itoc.010914.註解: 列表多篇，所以叫 FEETER，都是 78 char */

#define FEETER_INNBBS   \
    COLOR1 " 轉信設定 " COLOR2 " (↑/↓)上下 (PgUp/PgDn)上下頁 (Home/End)首尾 (←)(q)離開           "

#define FEETER_VOTEALL  \
    COLOR1 " 投票中心 " COLOR2 " (↑/↓)上下 (PgUp/PgDn)上下頁 (Home/End)首尾 (→)投票 (←)(q)離開  "

/* ----------------------------------------------------- */
/* 站簽：站台來源簽名                                    */
/* ----------------------------------------------------- */

#ifndef ORIGIN_TAG
#define ORIGIN_TAG \
    "--\n" \
    "\x1b[1;44;32m  ◢\x1b[0;32;44m◣   \x1b[1;37m︵︵     \x1b[30m█▔◣ █▔█ █▔▔ █▔█ █▆▉ █   █▔█ █◣█ █▔\x1b[31m● \x1b[m\n" \
    "\x1b[1;32;44m◢\x1b[42m◤\x1b[0;44;32m█◣\x1b[1m◢\x1b[0;32;44m◣ \x1b[1;37m︵︵ \x1b[0;37;44m█  █ █▁◤ █▁▁ █▁█ ▉▉▉ █   █▁█ █◥█ █  █ \x1b[m\n" \
    "\x1b[1;36;42m夢之大地 逼逼ㄟ四 \x1b[37m█▁◤ █  █ █▁▁ █  █ ▉▉▉ █▁ █  █ █  █ █▁◤ \x1b[m\n" \
    "\x1b[1;32m※ Origin:\x1b[33m <bbs.ccns.ncku.edu.tw>  \x1b[31m◆ From:\x1b[36m %-34.34s \x1b[m\n"
#endif  // ORIGIN_TAG

#define MODIFY_TAG    "\x1b[1;32m※ Modify: \x1b[33m<%s> \x1b[m%s"

#define ANONYMOUS_TAG "--\n\x1b[1;32m※ Origin: \x1b[33m%s \x1b[37m<%s> \x1b[m\n\x1b[1;31m◆ From: \x1b[36m%s\x1b[m\n"

#define EMAIL_TAG     "\x1b[1;32m※ E-mail: \x1b[36m%s\x1b[m\n"

/* ----------------------------------------------------- */
/* 其他訊息字串                                          */
/* ----------------------------------------------------- */

//for M3_USE_PFTERM
#define FILEDFG            (0)
#define FILEDBG            (7)

#endif                          /* THEME_H */
