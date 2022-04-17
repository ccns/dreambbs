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
    char data[ANSILINESIZE];
    char color[4];
    int ch, i;
    int len;
    int ansi;           /* 1: 在 ANSI 中 */
    int in_dbcs = 0;    /* 1: 在中文字中 */
    int fg, bg, hl;     /* 前景/背景/高彩 */
    screenline slt;

    hl = 0;
    fg = 37;
    bg = 40;

    i = y + x_roll;
    if (i > b_lines)
        i -= b_lines + 1;

    vs_save_line(&slt, i);
    str_sncpy(data, (char *) slt.data, sizeof(data), slt.len);
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
                if (!in_dbcs && IS_DBCS_HI(ch))
                {
                    outc(' ');
                    in_dbcs ^= 1;
                }
                else
                {
                    outc(ch);
                    in_dbcs = 0;
                }
                outs(str_ransi);
                break;
            }

            if (in_dbcs || IS_DBCS_HI(ch))
                in_dbcs ^= 1;
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

            if (in_dbcs || IS_DBCS_HI(ch))
                in_dbcs ^= 1;
        }
    }

    /* 印出 (y, x + strip_ansi_len(msg)) 這個字及後面的控制碼 */
    prints("\x1b[%d;%d;%dm", hl, fg, bg);
    /* 此字若是中文字的尾碼，就不印 */
    outc(in_dbcs ? ' ' : ch);

    /* 印出 (y, x + strip_ansi_len(msg) + 1) 至 行尾 */
    outs(str);
    outs(str_ransi);
}
#endif //M3_USE_PFTERM

/* ----------------------------------------------------- */
/* 選項繪製                                              */
/* ----------------------------------------------------- */

static int popup2_cur_color[1 << XO_NCUR] = {
    7, 41, 44, 45,
};

GCC_CONSTEXPR
static int get_dec_attr(int cur_idx, bool is_moving)
{
    return is_moving ? 7 : (cur_idx == 0) ? 41 : 44;
}

static const char *get_dec_color(int dec_attr)
{
    static char dec_color[32];
    if (dec_attr >= 90)
        sprintf(dec_color, "\x1b[1;%um", 30 + (dec_attr - 90));
    else
        sprintf(dec_color, "\x1b[%um", dec_attr);
    return dec_color;
}

static void
draw_item(
    int y, int x,
    const char *desc,
    char hotkey,
    int cur_st,
    int cur_idx)
{
    char buf[128];
    const int color = popup2_cur_color[cur_st];
    const bool met = cur_st & (1U << cur_idx);

    sprintf(buf, " │\x1b[%um%c %c%c%c%-25s  \x1b[m│ ",
        color, met ? '>' : ' ',
        (hotkey == *desc) ? '[' : '(', *desc,
        (hotkey == *desc) ? ']' : ')', desc + 1);

    draw_line(y, x, buf);
}


static void
draw_menu(
    int y, int x,
    const char *title,
    const char *const desc[],
    char hotkey,
    int *cur,
    int cur_idx,
    int dec_attr)
{
    int i;
    char buf[128];

    draw_line(y++, x, " ╭────────────────╮ ");

    sprintf(buf, " │\x1b[1m%s  %-28s  \x1b[m│ ", get_dec_color(dec_attr), title);
    draw_line(y++, x, buf);

    draw_line(y++, x, " ├────────────────┤ ");

    for (i = 1; desc[i]; i++)
    {
        draw_item(y++, x, desc[i], hotkey, cursor_get_state(cur, i), cur_idx);
    }

    draw_line(y, x, " ╰────────────────╯ ");

    /* 避免在偵測左右鍵全形下，按左鍵會跳離二層選單的問題 */
    move(b_lines, 0);
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
        if (!desc[i])
            return i;
        cc = desc[i][0];
        if (cc >= 'A' && cc <= 'Z')
            cc |= 0x20; /* 換小寫 */

        if (ch == cc)
            return i;
    }

    return -1;
}

GCC_CONSTEXPR static int popup2_geth(int max)
{
    return max + 3;
}

GCC_CONSTEXPR static int popup2_getw(void)
{
    return 38;
}

/* returns whether the menu moved successfully */
GCC_NONNULLS
static int popup2_move(int cmd, int *py_ref, int *px_ref, int max)
{
    const int h = popup2_geth(max);
    const int w = popup2_getw();
    const int y_ref = gety_bound_move(cmd, *py_ref, 0, (B_LINES_REF - h) >> 1, B_LINES_REF - h);
    const int x_ref = getx_bound_move(cmd, *px_ref, 0, (B_COLS_REF - w) >> 1, B_COLS_REF - w);
    const bool diff = (y_ref != *py_ref) || (x_ref != *px_ref);
    *py_ref = y_ref;
    *px_ref = x_ref;
    return diff;
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
    int cur[XO_NCUR], old_cur, max, dflt;
    int ch = KEY_NONE;
    char hotkey;
    int cur_idx = 0;
    bool is_moving = false;

    screen_backup_t old_screen;
    screen_backup_t old_screen_dark;

#ifndef M3_USE_PFTERM
    x_roll =
#endif
    scr_dump(&old_screen);

    grayout(0, b_lines, GRAYOUT_DARK);
    scr_dump(&old_screen_dark);

    hotkey = desc[0][0];
    max = find_cur('\0', INT_MAX, desc) - 1;
    dflt = find_cur(hotkey, max, desc);

    for (int i = 0; i < COUNTOF(cur); ++i)
        cur[i] = dflt;

popupmenu_ans2_redraw:
    y = gety_ref(y_ref);
    x = getx_ref(x_ref);

    /* 畫出整個選單 */
    draw_menu(y, x, title, desc, hotkey, cur, cur_idx, get_dec_attr(cur_idx, is_moving));
    y += 2;

    /* 一進入，游標停在預設值 */
    if (ch != I_RESIZETERM)
        old_cur = cur[cur_idx];

    while (1)
    {
        if (old_cur != cur[cur_idx])   /* 游標變動位置才需要重繪 */
        {
            draw_item(y + old_cur, x, desc[old_cur], hotkey, cursor_get_state(cur, old_cur), cur_idx);
            draw_item(y + cur[cur_idx], x, desc[cur[cur_idx]], hotkey, cursor_get_state(cur, cur[cur_idx]), cur_idx);
            old_cur = cur[cur_idx];
            /* 避免在偵測左右鍵全形下，按左鍵會跳離二層選單的問題 */
            move(b_lines, 0);
        }

        ch = vkey();
        switch (ch)
        {
        case KEY_PGUP:
        case KEY_PGDN:
        case KEY_UP:
        case KEY_DOWN:
        case KEY_HOME:
        case KEY_END:
        case KEY_LEFT:
        case KEY_RIGHT:
            if (is_moving)
            {
                if (popup2_move(ch, &y_ref, &x_ref, max))
                {
                    ch = I_RESIZETERM;
                    scr_restore_keep(&old_screen_dark);
                    goto popupmenu_ans2_redraw;
                }
                is_moving = false;
                draw_menu(y - 2, x, title, desc, hotkey, cur, cur_idx, get_dec_attr(cur_idx, is_moving));
            }
            break;
        case I_RESIZETERM:
            /* Screen size changed and redraw is needed */
            /* clear */
            scr_restore_keep(&old_screen_dark);
            /* Keep menu in bound after resizing */
            popup2_move(ch, &y_ref, &x_ref, max);
            goto popupmenu_ans2_redraw;
        default:
            ;
        }

        switch (ch)
        {
        case KEY_TAB:
            is_moving = !is_moving;
            ch = I_RESIZETERM;
            goto popupmenu_ans2_redraw;
        case ' ':
            is_moving = false;
            cur_idx = (cur_idx + 1) % XO_NCUR;
            ch = I_RESIZETERM;
            goto popupmenu_ans2_redraw;
        case KEY_LEFT:
        case KEY_ESC:
        case Meta(KEY_ESC):
        case KEY_RIGHT:
        case '\n':
            scr_free(&old_screen_dark);
            scr_restore_free(&old_screen);
            ch = (ch == KEY_RIGHT || ch == '\n') ? desc[cur[cur_idx]][0] : desc[0][1];
            if (ch >= 'A' && ch <= 'Z')
                ch |= 0x20;             /* 回傳小寫 */
            return ch;

        case KEY_UP:
            cur[cur_idx] = (cur[cur_idx] == 1) ? max : cur[cur_idx] - 1;
            break;

        case KEY_DOWN:
            cur[cur_idx] = (cur[cur_idx] == max) ? 1 : cur[cur_idx] + 1;
            break;

        case KEY_HOME:
            cur[cur_idx] = 1;
            break;

        case KEY_END:
            cur[cur_idx] = max;
            break;

        default:                /* 去找所按鍵是哪一個選項 */
            if ((ch = find_cur(ch, max, desc)) > 0)
                cur[cur_idx] = ch;
            break;
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

    for (;;)
    {
        pmsg2_body(msg);
        if ((x = vkey()) != I_RESIZETERM)
            break;
        scr_restore_keep(&old_screen);
    }

    scr_restore_free(&old_screen);
    return x;
}
