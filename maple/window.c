/*-------------------------------------------------------*/
/* window.c     ( NTHU CS MapleBBS Ver 3.10 )            */
/*-------------------------------------------------------*/
/* target : popup window menu                            */
/* create : 03/02/12                                     */
/* update : 03/07/23                                     */
/* author : verit.bbs@bbs.yzu.edu.tw                     */
/* modify : itoc.bbs@bbs.tnfsh.tn.edu.tw                 */
/*-------------------------------------------------------*/

/* 90929.cache: 第二種圖形選單 */

#include "bbs.h"

#ifdef M3_USE_PFTERM
/* ----------------------------------------------------- */
/* M3_USE_PFTERM                                         */
/* ----------------------------------------------------- */

static void
draw_line(
    int y, int x,
    const char *msg)
{
    /* hrs.090928: 讓 terminal 去處理 */
    move(y, x);
    outstr(msg);
    return;
}

#else

static int x_roll;

/* ----------------------------------------------------- */
/* 畫面繪製                                              */
/* ----------------------------------------------------- */


static void
draw_line(              /* 在 (y, x) 的位置塞入 msg，左右仍要印出原來的彩色文字 */
    int y, int x,
    const char *msg)
{
    char *str;
    const char *ptr;
    char data[ANSILINELEN];
    char color[4];
    int ch, i;
    int len;
    int ansi;           /* 1: 在 ANSI 中 */
    int in_chi = 0;     /* 1: 在中文字中 */
    int fg, bg, hl;     /* 前景/背景/高彩 */
    screenline slt;

    hl = 0;
    fg = 37;
    bg = 40;

    i = y + x_roll;
    if (i > b_lines)
        i -= b_lines + 1;

    memset(data, 0, sizeof(data));
    vs_save_line(&slt, i);
    strncpy(data, (char *) slt.data, slt.len);
    str = data;

    move(y, 0);
    clrtoeol();

    /* 印出 (y, 0) 至 (y, x - 1) */
    ansi = 0;
    len = 0;            /* 已印出幾個字 (不含控制碼) */
    while ((ch = (unsigned char) *str++))
    {
        if (ch == KEY_ESC)
        {
            ansi = 1;
            i = 0;
        }
        else if (ansi)
        {
            if (ch == '[')
            {
            }
            else if (ch >= '0' && ch <= '9')
            {
                color[i] = ch;
                if (++i >= 4)
                    i = 0;
            }
            else
            {
                color[i] = 0;

                i = atoi(color);
                if (i == 0)
                {
                    hl = 0;
                    fg = 37;
                    bg = 40;
                }
                else if (i == 1)
                    hl = 1;
                else if (i >= 30 && i <= 37)
                    fg = i;
                else if (i >= 40 && i <= 47)
                    bg = i;

                i = 0;

                if (ch != ';')
                    ansi = 0;
            }
        }
        else
        {
            if (++len >= x)
            {
                /* 最後一字若是中文字的首碼，就不印 */
                if (!in_chi && IS_DBCS_HI(ch))
                {
                    outc(' ');
                    in_chi ^= 1;
                }
                else
                {
                    outc(ch);
                    in_chi = 0;
                }
                outs(str_ransi);
                break;
            }

            if (in_chi || IS_DBCS_HI(ch))
                in_chi ^= 1;
        }

        outc(ch);
    }
    while (len++ < x)
        outc(' ');

    /* 印出 (y, x) 至 (y, x + strip_ansi_len(msg) - 1) */
    ptr = msg;
    ansi = 0;
    len = 0;            /* msg 的長度(不含控制碼) */
    while ((ch = (unsigned char) *ptr++))
    {
        if (ch == KEY_ESC)
        {
            ansi = 1;
        }
        else if (ansi)
        {
            if ((ch < '0' || ch > '9') && ch != ';' && ch != '[')
                ansi = 0;
        }
        else
        {
            len++;
        }
        outc(ch);
    }

    /* 跳掉 str 中間一整段，並取出最後的顏色 */
    ansi = 0;
    while ((ch = (unsigned char) *str++))
    {
        if (ch == KEY_ESC)
        {
            ansi = 1;
            i = 0;
        }
        else if (ansi)
        {
            if (ch == '[')
                continue;
            if (ch >= '0' && ch <= '9')
            {
                color[i] = ch;
                if (++i >= 4)
                    i = 0;
            }
            else
            {
                color[i] = 0;

                i = atoi(color);
                if (i == 0)
                {
                    hl = 0;
                    fg = 37;
                    bg = 40;
                }
                else if (i == 1)
                    hl = 1;
                else if (i >= 30 && i <= 37)
                    fg = i;
                else if (i >= 40 && i <= 47)
                    bg = i;

                i = 0;

                if (ch != ';')
                    ansi = 0;
            }
        }
        else
        {
            if (--len < 0)      /* 跳過 strip_ansi_len(msg) 的長度 */
                break;

            if (in_chi || IS_DBCS_HI(ch))
                in_chi ^= 1;
        }
    }

    /* 印出 (y, x + strip_ansi_len(msg)) 這個字及後面的控制碼 */
    prints("\x1b[%d;%d;%dm", hl, fg, bg);
    /* 此字若是中文字的尾碼，就不印 */
    outc(in_chi ? ' ' : ch);

    /* 印出 (y, x + strip_ansi_len(msg) + 1) 至 行尾 */
    outs(str);
    outs(str_ransi);
}
#endif //M3_USE_PFTERM

/* ----------------------------------------------------- */
/* 選項繪製                                              */
/* ----------------------------------------------------- */


static void
draw_item(
    int y, int x,
    const char *desc,
    char hotkey,
    int mode)           /* 0:清除光棒  1:畫上光棒 */
{
    char buf[128];

    sprintf(buf, " │%s%c %c%c%c%-25s  \x1b[m│ ",
        mode ? COLOR4 : "\x1b[30;47m", mode ? '>' : ' ',
        (hotkey == *desc) ? '[' : '(', *desc,
        (hotkey == *desc) ? ']' : ')', desc + 1);

    draw_line(y, x, buf);
}


static int      /* 回傳總共有幾個選項 */
draw_menu(
    int y, int x,
    const char *title,
    const char *const desc[],
    char hotkey,
    int *cur)   /* 回傳預設值所在位置 */
{
    int i, meet;
    char buf[128];

    draw_line(y++, x, " ╭────────────────╮ ");

    sprintf(buf, " │" COLOR4 "  %-28s  \x1b[m│ ", title);
    draw_line(y++, x, buf);

    draw_line(y++, x, " ├────────────────┤ ");

    for (i = 1; desc[i]; i++)
    {
        meet = (desc[i][0] == hotkey);
        draw_item(y++, x, desc[i], hotkey, meet);
        if (meet)
            *cur = i;
    }

    draw_line(y, x, " ╰────────────────╯ ");

    /* 避免在偵測左右鍵全形下，按左鍵會跳離二層選單的問題 */
    move(b_lines, 0);

    return i - 1;
}


/* ----------------------------------------------------- */
/* 找選項                                                */
/* ----------------------------------------------------- */


static int                      /* -1:找不到 >=0:第幾個選項 */
find_cur(               /* 找 ch 這個按鍵是第幾個選項 */
    int ch, int max,
    const char *const desc[])
{
    int i, cc;

    if (ch >= 'A' && ch <= 'Z')
        ch |= 0x20;             /* 換小寫 */

    for (i = 1; i <= max; i++)
    {
        cc = desc[i][0];
        if (cc >= 'A' && cc <= 'Z')
            cc |= 0x20; /* 換小寫 */

        if (ch == cc)
            return i;
    }

    return -1;
}


/*------------------------------------------------------ */
/* 詢問選項，可用來取代 vans()                           */
/*------------------------------------------------------ */
/* y, x  是蹦出視窗左上角的 (y, x) 位置                  */
/* title 是視窗的標題                                    */
/* desc  是選項的敘述：                                  */
/*       第一個字串必須為兩個 char                       */
/*         第一個字元代表一開始游標停的位置              */
/*         第二個字元代表按下 KEY_LEFT 的預設回傳值      */
/*       中間的字串是每個選項的敘述 (首字母為熱鍵)       */
/*       最後一個字串必須為 NULL                         */
/*------------------------------------------------------ */

/* IID.20200204: Use screen size referencing coordinate */
int             /* 傳回小寫字母或數字 */
popupmenu_ans2(const char *const desc[], const char *title, int y_ref, int x_ref)
{
    int y, x;
    int cur, old_cur, max, ch;
    char hotkey;

    screen_backup_t old_screen;

#ifndef M3_USE_PFTERM
    x_roll =
#endif
    scr_dump(&old_screen);

    y = gety_ref(y_ref);
    x = getx_ref(x_ref);

    grayout(0, b_lines, GRAYOUT_DARK);

    hotkey = desc[0][0];

    /* 畫出整個選單 */
    max = draw_menu(y, x, title, desc, hotkey, &cur);
    y += 2;

    /* 一進入，游標停在預設值 */
    old_cur = cur;

    while (1)
    {
        ch = vkey();
        if (gety_ref(y_ref) != y - 2 || getx_ref(x_ref) != x)
        {
            /* Screen size changed and redraw is needed */
            /* clear */
            scr_restore_keep(&old_screen);
            /* update position */
            y = gety_ref(y_ref);
            x = getx_ref(x_ref);
            /* redraw */
            grayout(0, b_lines, GRAYOUT_DARK);
            max = draw_menu(y, x, title, desc, hotkey, &SINKVAL(int));
            /* update parameters */
            y += 2;
            old_cur = cur;
        }

        switch (ch)
        {
        case KEY_LEFT:
        case KEY_ESC:
        case Meta(KEY_ESC):
        case KEY_RIGHT:
        case '\n':
            scr_restore_free(&old_screen);
            ch = (ch == KEY_RIGHT || ch == '\n') ? desc[cur][0] : desc[0][1];
            if (ch >= 'A' && ch <= 'Z')
                ch |= 0x20;             /* 回傳小寫 */
            return ch;

        case KEY_UP:
            cur = (cur == 1) ? max : cur - 1;
            break;

        case KEY_DOWN:
            cur = (cur == max) ? 1 : cur + 1;
            break;

        case KEY_HOME:
            cur = 1;
            break;

        case KEY_END:
            cur = max;
            break;

        default:                /* 去找所按鍵是哪一個選項 */
            if ((ch = find_cur(ch, max, desc)) > 0)
                cur = ch;
            break;
        }

        if (old_cur != cur)             /* 游標變動位置才需要重繪 */
        {
            draw_item(y + old_cur, x, desc[old_cur], hotkey, 0);
            draw_item(y + cur, x, desc[cur], hotkey, 1);
            old_cur = cur;
            /* 避免在偵測左右鍵全形下，按左鍵會跳離二層選單的問題 */
            move(b_lines, 0);
        }
    }
}

/*------------------------------------------------------ */
/* 蹦出式視窗訊息，可用來取代 vmsg()                     */
/*------------------------------------------------------ */

// IID.20190909: `pmsg2()` without blocking and without screen restoring.
void
pmsg2_body(const char *msg)
/* 不可為 NULL */
{
    int len, y, x, i;
    char buf[80];

    if (!msg)
    {
        vmsg_body(NULL);
        return;
    }

    grayout(0, b_lines, GRAYOUT_DARK);

    len = BMAX(strlen(msg), (size_t)16); /* 取 msg title 其中較長者為 len */
    if (len % 2)                /* 變成偶數 */
        len++;
    y = (b_lines - 4) >> 1;     /* 置中 */
    x = (b_cols - 8 - len) >> 1;

    strcpy(buf, "╭");
    for (i = -4; i < len; i += 2)
        strcat(buf, "─");
    strcat(buf, "╮");
    draw_line(y++, x, buf);

    sprintf(buf, "│" COLOR4 "  %-*s  \x1b[m│", len, "請按任意鍵繼續..");
    draw_line(y++, x, buf);

    strcpy(buf, "├");
    for (i = -4; i < len; i += 2)
        strcat(buf, "─");
    strcat(buf, "┤");
    draw_line(y++, x, buf);

    sprintf(buf, "│\x1b[30;47m  %-*s  \x1b[m│", len, msg);
    draw_line(y++, x, buf);

    strcpy(buf, "╰");
    for (i = -4; i < len; i += 2)
        strcat(buf, "─");
    strcat(buf, "╯");
    draw_line(y++, x, buf);

    move(b_lines, 0);
}

int
pmsg2(const char *msg)
/* 不可為 NULL */
{
    int x;

    if (!msg)
        return vmsg(NULL);

    screen_backup_t old_screen;

#ifndef M3_USE_PFTERM
    x_roll =
#endif
    scr_dump(&old_screen);

    pmsg2_body(msg);

    x = vkey();
    scr_restore_free(&old_screen);
    return x;
}
