/*-------------------------------------------------------*/
/* theme.h      ( NTHU CS MapleBBS Ver 3.10 )            */
/*-------------------------------------------------------*/
/* target : custom theme                                 */
/* create : 09/07/28                                     */
/* update : 17/07/26                                     */
/*-------------------------------------------------------*/


#ifndef THEME_H
#define THEME_H

#include "cppdef.h"

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

#define COLOR0          "\x1b[44m"      /* header 的底色 */
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

#define NECKBOARD  "  [←]主選單 [→]閱\讀 [↑↓]選擇 [c]篇數 [y]列zap [/]搜尋 [s]看板 [h]說明\n" \
            COLOR3 "  %-7s看  板            %-*s人氣 板    主      \x1b[m"

/*#define NECKGEM1 \
                   "  [←]離開 [→]瀏覽 [f]模式 [C]暫存 [F]轉寄 [Z]下載 [h]說明 "
*/

#define NECKGEM1 \
                   "  [←]離開 [→]瀏覽 [f]模式 [C]暫存 [h]說明 "
                   /*r2.20170802: disable some tips in case of confusion*/

#define NECKGEM2_WAY0 \
            COLOR0 "  編號     主              題%*s                                                  \x1b[m"

#define NECKGEM2_WAY1 \
            COLOR0 "  編號     主              題%*s                             [檔      名] [日  期]\x1b[m"

#define NECKGEM2_WAY2 \
            COLOR0 "  編號     主              題%*s                             [編      選] [日  期]\x1b[m"

#define NECKPOST \
                   "  [←]離開 [→]閱\讀 [^P]發表 [b]備忘錄 [d]刪除 [V]投票 [TAB]精華區 [h]說明\n" \
            COLOR3 "  編號     日 期 作  者       文  章  標  題%*s                 %16s  \x1b[m"

#define NECKMAIL \
                   "  [←]離開 [→]讀信 [d]刪除 [R]回信 [s]寄信 [x]轉貼 [y]群組回信 [h]說明\n" \
            COLOR0 "  編號   日 期 作 者        信  件  標  題%*s                                     \x1b[m"

#define NECKVOTE \
                   "  [←]離開 [v]投票 [R]結果 [E]修改 [^P]舉行 [^Q]查詢/中止/改期 [h]說明\n" \
            COLOR0 "  編號  開票日  主辦人       投  票  宗  旨%*s                                    \x1b[m"

#define NECKINNBBS "  [←]離開 [^P]新增 [d]刪除 [E]編輯 [/]搜尋 [Enter]詳細\n" \
            COLOR3 "  編號              內         容%*s                                              \x1b[m"

// IID.190324

/* ulist_neck() 及 xpost_head() 的第一行比較特別，不在此定義 */

#define NECK_BANMAIL \
                   "  [←]離開 ^P)新增 c)修改 d)刪除 S)重整 [h]elp\n" \
            COLOR3 "  編號  使用率 更新期 模  式  擋  信  內  容%*s                                   \x1b[m"

#define NECK_MYFAVORITE \
                   "  [←]主選單 [→]閱\讀 [a]新增 [d]刪除 [c]篇數 [/]搜尋 [s]看板 [h]說明\n" \
            COLOR0 "  %-7s看  板            %-*s人氣 板    主      \x1b[m"

#define NECK_XPOST "\n" /*r2.20181219: this part may need to be refined*/ \
            COLOR3 "  編號    日 期 作  者       文  章  標  題%*s                                    \x1b[m"

#define NECK_ULIST "\n" \
            COLOR3 "  No.  代號         %-*s %-*s PM %-14s閒置\x1b[m"

#define NECK_PAL   "  [←]離開 a)新增 c)修改 d)刪除 m)寄信 s)整理 [/?]搜尋 [q]查詢 [h]elp\n" \
            COLOR3 "  編號    代 號         友       誼%*s                                            \x1b[m"

#define NECK_BMW   "  [←]離開  [d]刪除  [m]寄信  [w]快訊  [s]更新  [→]查詢  [h]elp\n" \
            COLOR3 "  編號 代 號        內       容%*s                                                \x1b[m"

#define NECK_BMWTIME \
                   "  [←]離開  [d]刪除  [m]寄信  [w]快訊  [s]更新  [→]查詢  [h]elp\n" \
            COLOR3 "  編號 時 間 代 號        內       容%*s                                          \x1b[m"

#define NECK_BANMSG \
                   "  [←]離開 a)新增 c)修改 d)刪除 m)寄信 s)整理 [q]查詢 [h]elp\n" \
            COLOR3 "  編號    代 號         描       述%*s                                            \x1b[m"

// so/*

#define NECK_ADMIN \
                   "  [←]離開 ^P)新增 c)修改 d)刪除 s)重整 [h]elp\n" \
            COLOR3 "  編號     站  務  名  單%*s                                                      \x1b[m"

#define NECK_ALOHA \
                   "  [←]離開 a)新增 d)刪除 s)重整 f)引入好友名單 [h]elp\n" \
            COLOR5 "  編號     上 站 通 知 名 單%*s                                                   \x1b[m"

#define NECK_BRDSTAT \
                   "  [←]離開 s)重整 S)目前資訊 [h]elp\n" \
            COLOR3 "  編號   型    態  閱\讀次數  發文次數  轉信次數  擋信次數%*s                      \x1b[m"

#define NECK_CHATMENU \
                   "  [←]離開 ^P)新增 c)修改 d)刪除 s)重整 M)搬移 TAB)動詞 f)訊息 [h]elp\n" \
            COLOR3 "  編號 動詞      中文   %-*s                                                       \x1b[m"

#define NECK_CLEANRECOMMEND \
                   "  [←]離開 c)修改[站長專用] d)刪除 D)清除全部 s)重整 [h]elp\n" \
            COLOR0 "  編號 推    使用者 留言%*s                                                  日 期\x1b[m"

#define NECK_CONTACT \
                   "  [←]離開 ^P)新增 c)修改 d)刪除 s)重整 m)寄信 [h]elp\n" \
            COLOR3 "  編號     聯  絡  名  單     e-mail address%*s                                   \x1b[m"

#define NECK_LISTBRD \
                   "  [←]離開 a)新增 d)刪除 s)重整 TAB)切換名單 T)更改群組名稱 /)搜尋\n" \
            COLOR3 "  編號     看板名稱%*s                                                            \x1b[m"

#define NECK_LISTUSR \
                   "  [←]離開 a)新增 d)刪除 s)重整 TAB)切換名單 T)更改群組名稱 /)搜尋\n" \
            COLOR3 "  編號     使用者 ID%*s                                                           \x1b[m"

/*#define NECK_MAILGEM1 \
                   "  [←]離開 [→]瀏覽 [f]模式 [C]暫存 [F]轉寄 [Z]下載 [h]說明 "
*/
#define NECK_MAILGEM1 \
                   "  [←]離開 [→]瀏覽 [f]模式 [C]暫存 [h]說明 "
                   /*r2.20170802: disable "Z" option in case of confusion*/

#define NECK_MEMORANDUM \
                   "  [←]離開 ^P)新增 c)修改 d)刪除 s)重整 [h]elp\n" \
            COLOR3 "  編號  日期      時間      工作或行程%*s                                         \x1b[m"

#define NECK_NBRD  "  [←]離開 [→]閱\讀 [^P]發表 [d]刪除 [j]加入連署 [TAB]精華區 [h]elp\n" \
            COLOR0 "  編號   日 期 舉辦人       看  版  標  題%*s                                     \x1b[m"

#define NECK_OBSERVE \
                   "  [←]離開 ^P)新增 c)修改 d)刪除 S)重整 [h]elp\n" \
            COLOR3 "  編號 使用者ID     說明%*s                                                       \x1b[m"

#define NECK_PERSONAL1 \
                   "  [←]離開 c)修改 d)刪除 s)重整 TAB)中文版名/E-mail O)開板 D)退件 [h]elp\n" \
            COLOR0 "  編號   申請人       看板名稱     "

#define NECK_PERSONALEMAIL2 \
                   "E-mail%*s                                   \x1b[m"

#define NECK_PERSONALTITLE2 \
                   "中文版名%*s                                 \x1b[m"

#define NECK_SHOW  "  [←]離開 ^P)新增 c)修改 d)刪除 s)重整 [h]elp\n" \
            COLOR3 "  編號     已投票的 Email%*s                                                      \x1b[m"

#define NECK_VIOL  "  [←]離開 ^P)新增 c)修改 d)刪除 f)搜尋 [h]elp\n" \
            COLOR3 "  編號 次數 時間 禁止註冊 Email%*s                                                \x1b[m"

#define NECK_SONG  "  [←]離開 [→]瀏覽 [o]點歌到動態看板 [m]點歌到信箱 [q]查詢剩餘次數 [h]說明 \n" \
            COLOR0 "  編號     主              題%*s                             [編      選] [日  期]\x1b[m"

// cache.101119

#define NECKER_VOTE     "  [←]離開 [R]結果 [^P]舉行 [E]修改 [V]預覽 [^Q]改期 [o]名單 [h]說明\n" \

#define NECKER_VOTEALL  "  [↑/↓]上下 [PgUp/PgDn]上下頁 [Home/End]首尾 [→]投票 [←][q]離開\n" \


/* ----------------------------------------------------- */
/* 訊息字串：more() 時的 footer 都抓出來定義在這         */
/* ----------------------------------------------------- */


/* itoc.010914.註解: 單一篇，所以叫 FOOTER，都是 79 char */

#define FOOTER_POST     \
    COLOR1 " 文章選讀 " COLOR2 " (ry)回應 (=\\[]<>-+;'`)主題 (|?QA)搜尋標題作者 (kj)上下篇 (C)暫存    "

#define FOOTER_MORE     \
    COLOR1 " 瀏覽 P.%d (%d%%) " COLOR2 " (h)說明 [PgUp][PgDn][0][$]移動 (/n)搜尋 (C)暫存 (←q)結束  "


#define FOOTER_TALK \
    COLOR1 " 交談模式 " COLOR2 " (^A)\x1b[30m對奕模式 \x1b[31m(^B)\x1b[30m象棋模式 \x1b[31m(^C, ^D)\x1b[30m結束交談 \x1b[31m(^Z)\x1b[30m快捷列表 \x1b[31m(^G)\x1b[30m嗶嗶    "


#define FOOTER_VEDIT \
               "%s\x1b[m" COLOR2 "  (Ctrl-Z)\x1b[30m 操作說明  \x1b[31m(^w, ^x)\x1b[30m 檔案處理 ║%s│%s│%s║%5d:%3d%*s \x1b[m"


/*#define FOOTER_BMW_REPLY \
    COLOR1 " 熱訊回應 " COLOR2 " (←)\x1b[30m離開 \x1b[31m(↑↓→)\x1b[30m瀏覽 \x1b[31m(Enter)\x1b[30m選擇線上使用者扣應 \x1b[31m(其他)\x1b[30m回應%*s        \x1b[m"
*/
#define FOOTER_BMW_REPLY \
    COLOR1 " 熱訊回應 " COLOR2 " (← Enter)\x1b[30m離開 \x1b[31m(↑↓→)\x1b[30m瀏覽 \x1b[31m(其他)\x1b[30m回應%*s                            \x1b[m"


#define MSG_POST \
    COLOR1 " 文章選讀 " COLOR2 " (y)\x1b[30m回應 \x1b[31m(=\\[]<>-+;'`jk)\x1b[30m相關主題 \x1b[31m(/?)\x1b[30m搜尋標題 \x1b[31m(aA)\x1b[30m搜尋作者           "

#define MSG_MAILER \
    COLOR1 " 魚雁往返 " COLOR2 " (r)\x1b[30m回信 \x1b[31m(x)\x1b[30m轉達 \x1b[31m(y)\x1b[30m群組回信 \x1b[31m(d)\x1b[30m刪除 \x1b[31m[m]\x1b[30m標記                         "

#define MSG_GEM \
    COLOR1 " 閱\讀精華 " COLOR2 " (=\\[]<>-+jk)\x1b[30m相關主題 \x1b[31m(/?aA)\x1b[30m搜尋標題作者 \x1b[31m(↑↓)\x1b[30m上下 \x1b[31m(←)\x1b[30m離開         "


/* ----------------------------------------------------- */
/* 訊息字串：xo_foot() 時的 feeter 都抓出來定義在這      */
/* ----------------------------------------------------- */


/* itoc.010914.註解: 列表多篇，所以叫 FEETER，都是 79 char */

#define FEETER_INNBBS   \
    COLOR1 " 轉信設定 " COLOR2 " (↑/↓)上下 (PgUp/PgDn)上下頁 (Home/End)首尾 (←)(q)離開            "

#define FEETER_VOTEALL  \
    COLOR1 " 投票中心 " COLOR2 " (↑/↓)上下 (PgUp/PgDn)上下頁 (Home/End)首尾 (→)投票 (←)(q)離開   "

/* ----------------------------------------------------- */
/* 站簽：站台來源簽名                                    */
/* ----------------------------------------------------- */

/* IID.2021-03-06: Formatted customization of `ORIGIN_TAG` */

/* `ORIGIN_TAG_PIC` should end with an ANSI reset and a newline if not empty */
#ifndef ORIGIN_TAG_PIC
#define ORIGIN_TAG_PIC \
    "\x1b[1;44;32m  ◢\x1b[0;32;44m◣   \x1b[1;37m︵︵     \x1b[30m█▔◣ █▔█ █▔▔ █▔█ █▆▉ █   █▔█ █◣█ █▔\x1b[31m● \x1b[m\n" \
    "\x1b[1;32;44m◢\x1b[42m◤\x1b[0;44;32m█◣\x1b[1m◢\x1b[0;32;44m◣ \x1b[1;37m︵︵ \x1b[0;37;44m█  █ █▁◤ █▁▁ █▁█ ▉▉▉ █   █▁█ █◥█ █  █ \x1b[m\n" \
    "\x1b[1;36;42m夢之大地 逼逼ㄟ四 \x1b[37m█▁◤ █  █ █▁▁ █  █ ▉▉▉ █▁ █  █ █  █ █▁◤ \x1b[m\n"
#endif

/* `ORIGIN_TAG_SITENAME` should start and end with a space after excluding ANSI escapes */
#ifndef ORIGIN_TAG_SITENAME
#define ORIGIN_TAG_SITENAME " "
#endif

/* `ORIGIN_TAG_SITENAME` should start with a `"<"` and end with a `">"` after excluding ANSI escapes */
#ifndef ORIGIN_TAG_HOSTNAME
#define ORIGIN_TAG_HOSTNAME "\x1b[33m<" MYHOSTNAME ">"
#endif

/* Forbid `ORIGIN_TAG` from being defined elsewhere */
#ifdef ORIGIN_TAG
CPP_PRAGMA(GCC error "Customizing `ORIGIN_TAG` by defining it elsewhere is no longer supported. Use `ORIGIN_TAG_PIC`/`ORIGIN_TAG_SITENAME`/`ORIGIN_TAG_HOSTNAME` to customize `ORIGIN_TAG` instead");
#endif

/* IID.2021-03-06: Print the user address information in a separated line if too long */
#define ORIGIN_TAG \
    "--\n" \
    ORIGIN_TAG_PIC \
    "\x1b[1;32m※ Origin:" ORIGIN_TAG_SITENAME ORIGIN_TAG_HOSTNAME " %s\x1b[1;31m◆ From:\x1b[36m %s \x1b[m\n"

#define MODIFY_TAG    "\x1b[1;32m※ Modify: \x1b[33m<%s> \x1b[m%s"

#define ANONYMOUS_TAG "--\n\x1b[1;32m※ Origin: \x1b[33m%s \x1b[37m<%s> \x1b[m\n\x1b[1;31m◆ From: \x1b[36m%s\x1b[m\n"

#define EMAIL_TAG     "\x1b[1;32m※ E-mail: \x1b[36m%s\x1b[m\n"

/* ----------------------------------------------------- */
/* 其他訊息字串                                          */
/* ----------------------------------------------------- */

//for M3_USE_PFTERM
#define FILEDFG            0
#define FILEDBG            7

#endif                          /* THEME_H */
