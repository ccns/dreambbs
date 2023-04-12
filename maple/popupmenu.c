/*-------------------------------------------------------*/
/* popupmenu.c   ( YZU_CSE WindTop BBS )                 */
/*-------------------------------------------------------*/
/* author : verit.bbs@bbs.yzu.edu.tw                     */
/* target : popup menu                                   */
/* create : 2003/02/12                                   */
/*-------------------------------------------------------*/

#include "bbs.h"

/* For screenshot */
static screen_backup_t *popup_old_screen = NULL;

static int do_menu(MENU pmenu[], XO *xo, int y_ref, int x_ref);

/* ----------------------------------------- */
/* verit 判斷字串的第 pos 各字元是那一種情況 */
/*     return 1  代表中文字的前半部          */
/*           -1  代表中文字的後半部          */
/*            0  代表不是中文字              */
/* ----------------------------------------- */

#ifndef M3_USE_PFTERM
GCC_PURE static int
is_big5(const char *src, int pos, int mode)
{
    bool wstate=false;
    int word=0;
    const char *str;

    for (str = src; word<pos; str++)
    {
        if (mode)
        {
            if (*str == '\x1b')
            {
                str = strchr(str, 'm');
                if (!str)
                    return -1;
                continue;
            }
        }

        wstate = (*str<0) && !wstate;
        word++;
    }

    if (wstate)
        return -1;

    if (*str<0 && !wstate)
        return 1;
    else
        return 0;
}
#endif  /* #ifndef M3_USE_PFTERM */

static int
do_cmd(MENU *mptr, XO *xo, int y, int x)
{
    GCC_UNUSED unsigned int mode;
    screen_backup_t old_screen;
    unsigned int mmode = mptr->umode;
    /* IID.2021-12-02: Helper union for proper handling of `M_ARG` */
    typedef union {
        MenuItem item;
        FuncArg funcarg;
        DlFuncArg dlfuncarg;
    } MItem;
    MItem m =
        (mmode & M_ARG) ? LISTLIT(MItem){.funcarg = *mptr->item.funcarg} /* Make a copy of the `FuncArg` object */
        : LISTLIT(MItem){.item = mptr->item};

#if !NO_SO
    if (mmode & M_DL(0))
    {
        if (mmode & M_ARG)
        {
            m.funcarg.func = (int (*)(const void *)) DL_GET(m.dlfuncarg.func);
            if (!m.funcarg.func)
                return 0;
  #ifndef DL_HOTSWAP
            /* Update the `FuncArg` object */
            mptr->item.funcarg->func = m.funcarg.func;
  #endif
        }
        else
        {
            m.item.func = (int (*)(void)) DL_GET(m.item.dl.func);
            if (!m.item.func)
                return 0;
  #ifndef DL_HOTSWAP
            mptr->item.func = m.item.func;
  #endif
        }
        mmode &= ~M_DL(0);
  #ifndef DL_HOTSWAP
        mptr->umode = mmode;
  #endif
    }
#endif

    scr_dump(&old_screen);

    if ((mmode & M_MASK) >= M_MENU && (mmode & M_MASK) < M_FUN)
    {
//      sprintf(t, "【%s】", mptr->desc);
        scr_restore_free(&old_screen);
        return do_menu(m.item.menu, xo, y, x);
    }
    else if (mmode & M_ARG)
    {
        if (mmode & M_XO)
            mode = m.funcarg.xofunc(xo, m.funcarg.arg);
        else
            mode = m.funcarg.func(m.funcarg.arg);
    }
    else
    {
        if (mmode & M_XO)
            mode = m.item.xofunc(xo);
        else
            mode = m.item.func();
    }

    scr_free(&old_screen);
    return -1;
}

#ifndef M3_USE_PFTERM
/* verit . 計算扣掉色碼的實際長度 */
GCC_PURE static int
count_len(
    const char *data)
{
    int len;
    const char *ptr, *tmp;
    ptr = data;
    len = strlen(data);

    while (ptr)
    {
        ptr = strstr(ptr, "\x1b");
        if (ptr)
        {
            for (tmp=ptr; *tmp!='m'; tmp++);
            len -= (tmp-ptr+1);
            ptr = tmp+1;
        }
    }
    return len;
}

/* verit . 取得顏色 */
static void
get_color(const char *s, int len, int *fc, int *bc, int *bbc)
{
    char buf[32], *p, *e;
    int color;
    int state = 0, reset=0, exit = 0;

    str_sncpy(buf, s+2, sizeof(buf), len-1);

    for (p = e = &buf[0]; exit == 0; ++p)
    {
        if (*p == ';' || *p == 'm')
        {
            if (*p == 'm')
                    exit = 1;
            *p = 0;

            color = atoi(e);

            if (color == 0)
            {
                *bbc = 0;
                reset = 1;
            }
            else if (color > 0 && color < 10)
            {
                *bbc = color;
                state |= 0x1;
            }
            else if (color>29 && color < 38)
            {
                *fc = color;
                state |= 0x2;
            }
            else if (color>39 && color < 48)
            {
                *bc = color;
                state |= 0x4;
            }
            e = p+1;
        }
    }

    if (reset == 1)
    {
        if (!(state & 0x4))
            *bc = 40;
        if (!(state & 0x2))
            *fc = 37;
        if (!(state & 0x1))
            *bbc = 0;
    }

    if (state == 0)
    {
        *bc = 40;
        *fc = 37;
        *bbc = 0;
    }

}
#endif  /* #ifndef M3_USE_PFTERM */

#ifndef M3_USE_PFTERM
static void
vs_line(const char *msg, int y, int x)
{
    char buf[512], color[16], *str, *tmp, *cstr;
    int len = count_len(msg);
    int word, slen, fc=37, bc=40, bbc=0;
    screenline sl;

    vs_save_line(&sl, y);
    sl.data[sl.len] = '\0';
    str = tmp = (char *) sl.data;

    for (word=0; word<x && *str; ++str)
    {
        if (*str == KEY_ESC)
        {
            cstr = strchr(str, 'm');
            get_color(str, cstr-str, &fc, &bc, &bbc);
            str = cstr;
            continue;
        }
        word++;
    }

    str_sncpy(buf, (char *) sl.data, sizeof(buf), str - tmp);

    while (word++<x)
        strcat(buf, " ");

    slen = strlen(buf)-1;

    /* verit . 假如這為中文字元的前半碼就清掉 */
    if (is_big5(buf, slen, 0)>0)
        buf[slen] = ' ';

    strcat(buf, msg);

    if (*str)
    {
        for (word=0; word<len && *str; ++str)
        {
            if (*str == KEY_ESC)
            {
                cstr = strchr(str, 'm');
                get_color(str, cstr-str, &fc, &bc, &bbc);
                str = cstr;
                continue;
            }
            word++;
        }

        if (*str)
        {
            /* verit . 解決後面顏色補色 */
            sprintf(color, "\x1b[%d;%d;%dm", bbc, fc, bc);
            strcat(buf, color);

            /* verit . 假如最後一字元為中文的後半部就清掉 */
            slen = strlen(buf);
            strcat(buf, str);
            if (is_big5(tmp, str-tmp, 0)<0)
                buf[slen] = ' ';
        }
    }

    move(y, 0);
    outs(buf);

}
#else
static void
vs_line(
    const char *msg, int y, int x)
{
    move(y, x);

    if (msg)
    {
        outstr(msg);
    }

    move(y, 0);
}
#endif //not M3_USE_PFTERM

static int popup_cur_color[XO_NCUR][1 << XO_NCUR] = {
    {0, 1},
    {0, 1, 4, 5},
};

static int popup_dec_color[XO_NCUR][1 << XO_NCUR] = {
    {90, 90},
    {90, 31, 36, 35},
};

GCC_CONSTEXPR
static int get_dec_attr(int cur_idx, bool is_moving)
{
    return is_moving ? 90 : popup_dec_color[xo_ncur - 1][1 << cur_idx];
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

/* mode 代表加背景的顏色 */
static void
draw_item(const char *desc, int cur_st, int cur_idx, int y, int x, int dec_attr)
{
    char buf[128];
    const int color = popup_cur_color[xo_ncur - 1][cur_st];
    const bool met = cur_st & (1U << cur_idx);

    sprintf(buf, " \x1b[0;37m▏\x1b[4%d;37m%s(\x1b[1;36m%c\x1b[0;37;4%dm)%-25s%s\x1b[0;47;30m▉\x1b[40m%s▉\x1b[m ", color, met?"┤":"  ", *desc, color, desc+1, met?"├":"  ", get_dec_color(dec_attr));
    vs_line(buf, y, x);
    move(b_lines, 0);
}


static int
draw_menu(const MENU *const pmenu[20], int num, const char *title, int y, int x, int *cur, int cur_idx, int dec_attr)
{
    char buf[128];
    int i;
    char t[128];

    sprintf(t, "【%s】", title);

    sprintf(buf, " \x1b[0;40;37mˍˍˍˍˍˍˍˍˍˍˍˍˍˍˍˍˍˍ\x1b[m   ");
    vs_line(buf, y-2, x);
    sprintf(buf, " \x1b[0;37;44m▏\x1b[1m%-31s \x1b[0;47;34m▉\x1b[m   ", t);
    vs_line(buf, y-1, x);

    for (i=0; i<num; ++i, ++y)
    {
        draw_item(pmenu[i]->desc, cursor_get_state(cur, i), cur_idx, y, x, dec_attr);
    }
    sprintf(buf, " \x1b[0;47;30m▇%s▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇\x1b[40m▉\x1b[m ", get_dec_color(dec_attr));
    vs_line(buf, y, x);
    return 0;
}

GCC_CONSTEXPR static int popup_geth(int num)
{
    return num + 2;
}

GCC_CONSTEXPR static int popup_getw(void)
{
    return 39;
}

/* returns whether the menu moved successfully */
GCC_NONNULLS
static bool popup_move(int cmd, int *py_ref, int *px_ref, int num)
{
    const int h = popup_geth(num);
    const int w = popup_getw();
    const int y_ref = gety_bound_move(cmd, *py_ref, 3, 3 + ((B_LINES_REF - h) >> 1), B_LINES_REF - h);
    const int x_ref = getx_bound_move(cmd, *px_ref, 0, (B_COLS_REF - w) >> 1, B_COLS_REF - w);
    const bool diff = (y_ref != *py_ref) || (x_ref != *px_ref);
    *py_ref = y_ref;
    *px_ref = x_ref;
    return diff;
}

/* IID.20200204: Use screen size referencing coordinate */
static int
do_menu(
    MENU pmenu[],
    XO *xo,
    int y_ref,
    int x_ref)
{
    int y, x;
    int cur[XO_NCUR], old_cur, num, tmp;
    int c;
    MENU *table[20];
    const char *title;
    MENU *table_title;
    screen_backup_t old_screen;
    int cur_idx = 0;
    bool is_moving = false;

    memset(table, 0, sizeof(table));
    /* verit. menu 權限檢查 */
    for ( tmp=0, num=-1; !(pmenu[tmp].umode & M_TAIL_MASK || pmenu[tmp].level & PERM_MENU); tmp++ )
    {
        if (pmenu[tmp].level == 0 || pmenu[tmp].level & cuser.userlevel)
            table[++num] = &pmenu[tmp];

    }

    /* verit. 更新動態 */
    if (pmenu[tmp].item.title)
    {
        strcpy(cutmp->mateid, pmenu[tmp].item.title);
        utmp_mode(M_IDLE);
    }

    title = pmenu[tmp].desc;
    table_title = &pmenu[tmp];

    /* verit . 假如沒有任何選項就 return */
    if (num < 0)
        return 1;

    old_cur = 0;
    for (int i = 0; i < COUNTOF(cur); ++i)
        cur[i] = 0;

    /* 跳到預設選項 */
    for ( tmp=0; tmp<= num; tmp++ )
    {
        if (tolower(table[tmp]->desc[0]) == tolower(table_title->level & ~PERM_MENU))
        {
            old_cur = tmp;
            for (int i = 0; i < COUNTOF(cur); ++i)
                cur[i] = tmp;
            break;
        }
    }


    scr_dump(&old_screen);
    popup_old_screen = &old_screen;

do_menu_redraw:
    y = gety_ref(y_ref);
    x = getx_ref(x_ref);

    draw_menu((const MENU *const *)table, num+1, title, y, x, cur, cur_idx, get_dec_attr(cur_idx, is_moving));

    while (1)  /* verit . user 選擇 */
    {
        c = vkey();
        if (c == I_RESIZETERM)
        {
            /* Screen size changed and redraw is needed */
            /* clear */
            if (xo)
            {
                /* Redraw and redump */
                popup_old_screen = NULL;
                scr_restore_free(&old_screen);
                xover_resize(xo);
                scr_dump(&old_screen);
                popup_old_screen = &old_screen;
            }
            else
            {
                scr_restore_keep(&old_screen);
                popup_old_screen = &old_screen;
            }

            /* Keep menu in bound after resizing */
            popup_move(c, &y_ref, &x_ref, num);
            goto do_menu_redraw;
        }

        old_cur = cur[cur_idx];

        switch (is_moving ? c : KEY_NONE)
        {
            case KEY_PGUP:
            case KEY_PGDN:
            case KEY_UP:
            case KEY_DOWN:
            case KEY_HOME:
            case KEY_END:
            case KEY_LEFT:
            case KEY_RIGHT:
                if (popup_move(c, &y_ref, &x_ref, num))
                {
                    scr_restore_keep(&old_screen);
                    goto do_menu_redraw;
                }
                is_moving = false;
                draw_menu((const MENU *const *)table, num+1, title, y, x, cur, cur_idx, get_dec_attr(cur_idx, is_moving));
                break;
            default:
                ;
        }

        switch (c)
        {
            case KEY_KONAMI:
                for (int i = xo_ncur; i < XO_NCUR; ++i)
                    cur[i] = cur[xo_ncur - 1];
                xo_ncur = xo_ncur % XO_NCUR + 1;
                cur_idx %= xo_ncur;
                goto do_menu_redraw;
            case KEY_TAB:
                if (xo_ncur == 1) // Plain mode
                    break;
                is_moving = !is_moving;
                goto do_menu_redraw;
            case ' ':
                is_moving = false;
                cur_idx = (cur_idx + 1) % xo_ncur;
                goto do_menu_redraw;
            case KEY_LEFT:
            case KEY_ESC:
            case Meta(KEY_ESC):
                popup_old_screen = NULL;
                scr_restore_free(&old_screen);
                return 1;
            case KEY_UP:
                cur[cur_idx] = (cur[cur_idx]==0)?num:cur[cur_idx]-1;
                break;
            case KEY_DOWN:
                cur[cur_idx] = (cur[cur_idx]==num)?0:cur[cur_idx]+1;
                break;
            case KEY_HOME:
                cur[cur_idx] = 0;
                break;
            case KEY_END:
                cur[cur_idx] = num;
                break;
            case KEY_RIGHT:
            case '\n':
                if (table[cur[cur_idx]]->umode & M_QUIT)
                {
                    popup_old_screen = NULL;
                    scr_restore_free(&old_screen);
                    return 1;
                }
                if (do_cmd(table[cur[cur_idx]], xo, y, x)<0)
                {
                    popup_old_screen = NULL;
                    scr_restore_free(&old_screen);
                    return -1;
                }
                scr_restore_keep(&old_screen);
                popup_old_screen = &old_screen;
                draw_menu((const MENU *const *)table, num+1, title, y, x, cur, cur_idx, get_dec_attr(cur_idx, is_moving));
                break;
            default:
                for (tmp=0; tmp<=num; tmp++)
                {
                    if (tolower(c) == tolower(table[tmp]->desc[0]))
                    {
                        cur[cur_idx] = tmp;
                        if (table_title->umode & M_DOINSTANT)
                        {
                            if (table[cur[cur_idx]]->umode & M_QUIT)
                            {
                                popup_old_screen = NULL;
                                scr_restore_free(&old_screen);
                                return 1;
                            }
                            if (do_cmd(table[cur[cur_idx]], xo, y, x)<0)
                            {
                                popup_old_screen = NULL;
                                scr_restore_free(&old_screen);
                                return -1;
                            }
                            scr_restore_keep(&old_screen);
                            popup_old_screen = &old_screen;
                            draw_menu((const MENU *const *)table, num+1, title, y, x, cur, cur_idx, get_dec_attr(cur_idx, is_moving));
                        }
                        break;
                    }
                }
                break;
        }
        if (old_cur != cur[cur_idx])
        {
            draw_item(table[old_cur]->desc, cursor_get_state(cur, old_cur), cur_idx, y+old_cur, x, get_dec_attr(cur_idx, is_moving));
            draw_item(table[cur[cur_idx]]->desc, cursor_get_state(cur, cur[cur_idx]), cur_idx, y+cur[cur_idx], x, get_dec_attr(cur_idx, is_moving));
        }
    }

    // return 0;
}


/* mode 代表加背景的顏色 */
static void
draw_ans_item(
    const char *desc,
    int cur_st,
    int cur_idx,
    int y,
    int x,
    char hotkey,
    int dec_attr)
{
    char buf[128];
    const int color = popup_cur_color[xo_ncur - 1][cur_st];
    const bool met = cur_st & (1U << cur_idx);

    sprintf(buf, " \x1b[0;37m▏\x1b[4%d;37m%s%c\x1b[1;36m%c\x1b[0;37;4%dm%c%-25s%s\x1b[0;47;30m▉\x1b[40m%s▉\x1b[m ", color, met?"┤":"  ", (hotkey==*desc)?'[':'(', *desc, color, (hotkey==*desc)?']':')', desc+1, met?"├":"  ", get_dec_color(dec_attr));
    vs_line(buf, y, x);
    move(b_lines, 0);
}


static int
draw_menu_des(const char *const desc[], const char *title, int y, int x, int *cur, int cur_idx, int dec_attr)
{
    int num;
    char buf[128];
    char hotkey;
    hotkey = desc[0][0];

    sprintf(buf, " \x1b[0;40;37mˍˍˍˍˍˍˍˍˍˍˍˍˍˍˍˍˍˍ\x1b[m   ");
    vs_line(buf, y-2, x);
    sprintf(buf, " \x1b[0;37;44m▏%-31s \x1b[0;47;34m▉\x1b[m   ", title);
    vs_line(buf, y-1, x);
    for (num=1; desc[num]; num++)
        draw_ans_item(desc[num], cursor_get_state(cur, num), cur_idx, y++, x, hotkey, dec_attr);
    sprintf(buf, " \x1b[0;47;30m▇\x1b[30;1m▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇\x1b[40m%s▉\x1b[m ", get_dec_color(dec_attr));
    vs_line(buf, y, x);
    return num-2;
}

/*------------------------------------------------------------- */
/* verit . desc 第一個必須有兩個 char,                          */
/*              第一個字元代表一開始游標停的位置                */
/*              第二個字元代表按下 KEY_LEFT 的預設回傳值        */
/*         desc 最後一個必須為 NULL                             */
/*------------------------------------------------------------- */
/* IID.20200204: Use screen size referencing coordinate */
int
popupmenu_ans(const char *const desc[], const char *title, int y_ref, int x_ref)
{
    int y, x;
    int cur[XO_NCUR], old_cur, num, tmp;
    int c = KEY_NONE;
    char t[64];
    char hotkey;
    screen_backup_t old_screen;
    int cur_idx = 0;
    bool is_moving = false;

    scr_dump(&old_screen);
    hotkey = desc[0][0];

popupmenu_ans_redraw:
    y = gety_ref(y_ref);
    x = getx_ref(x_ref);

    if (c != I_RESIZETERM)
        sprintf(t, "【%s】", title);

    num = draw_menu_des(desc, t, y, x, cur, cur_idx, get_dec_attr(cur_idx, is_moving));

    if (c != I_RESIZETERM)
    {
        old_cur = 0;
        for (int i = 0; i < COUNTOF(cur); ++i)
            cur[i] = 0;
        for (tmp=0; tmp<num; tmp++)
        {
            if (desc[tmp+1][0] == hotkey)
            {
                old_cur = tmp;
                for (int i = 0; i < COUNTOF(cur); ++i)
                    cur[i] = tmp;
            }
        }
    }

    draw_ans_item(desc[cur[cur_idx]+1], cursor_get_state(cur, old_cur), cur_idx, y+cur[cur_idx], x, hotkey, get_dec_attr(cur_idx, is_moving));

    while (1)
    {
        c = vkey();
        old_cur = cur[cur_idx];

        switch (c)
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
                    if (popup_move(c, &y_ref, &x_ref, num))
                    {
                        c = I_RESIZETERM;
                        scr_restore_keep(&old_screen);
                        goto popupmenu_ans_redraw;
                    }
                    is_moving = false;
                    draw_menu_des(desc, t, y, x, cur, cur_idx, get_dec_attr(cur_idx, is_moving));
                }
                break;
            case I_RESIZETERM:
                /* Screen size changed and redraw is needed */
                /* clear */
                scr_restore_keep(&old_screen);
                /* Keep menu in bound after resizing */
                popup_move(c, &y_ref, &x_ref, num);
                goto popupmenu_ans_redraw;
            default:
                ;
        }

        switch (c)
        {
            case KEY_KONAMI:
                for (int i = xo_ncur; i < XO_NCUR; ++i)
                    cur[i] = cur[xo_ncur - 1];
                xo_ncur = xo_ncur % XO_NCUR + 1;
                cur_idx %= xo_ncur;
                goto popupmenu_ans_redraw;
            case KEY_TAB:
                if (xo_ncur == 1) // Plain mode
                    break;
                is_moving = !is_moving;
                c = I_RESIZETERM;
                goto popupmenu_ans_redraw;
            case ' ':
                is_moving = false;
                cur_idx = (cur_idx + 1) % xo_ncur;
                c = I_RESIZETERM;
                goto popupmenu_ans_redraw;
            case KEY_LEFT:
            case KEY_ESC:
            case Meta(KEY_ESC):
                scr_restore_free(&old_screen);
                return tolower(desc[0][1]);
            case KEY_UP:
                cur[cur_idx] = (cur[cur_idx]==0)?num:cur[cur_idx]-1;
                break;
            case KEY_DOWN:
                cur[cur_idx] = (cur[cur_idx]==num)?0:cur[cur_idx]+1;
                break;
            case KEY_HOME:
                cur[cur_idx] = 0;
                break;
            case KEY_END:
                cur[cur_idx] = num;
                break;
            case KEY_RIGHT:
            case '\n':
                scr_restore_free(&old_screen);
                return tolower(desc[cur[cur_idx]+1][0]);
            default:
                for (tmp=0; tmp<=num; tmp++)
                {
                    if (tolower(c) == tolower(desc[tmp+1][0]))
                    {
                        cur[cur_idx] = tmp;
                        break;
                    }
                }
                break;
        }
        if (old_cur != cur[cur_idx])
        {
            draw_ans_item(desc[old_cur+1], cursor_get_state(cur, old_cur), cur_idx, y+old_cur, x, hotkey, get_dec_attr(cur_idx, is_moving));
            draw_ans_item(desc[cur[cur_idx]+1], cursor_get_state(cur, cur[cur_idx]), cur_idx, y+cur[cur_idx], x, hotkey, get_dec_attr(cur_idx, is_moving));
        }
    }
    // return 0;
}

void
popupmenu(MENU pmenu[], XO *xo, int y_ref, int x_ref)
{
    do_menu(pmenu, xo, y_ref, x_ref);
}

static void pcopy(char *buf, const char *patten, int len)
{
    int i, size;

    *buf = '\0';
    size = strlen(patten);

    for (i=1; i<=len; i++)
    {
        strcpy(buf, patten);
        buf += size;
    }
}

// IID.20190909: `pmsg()` without blocking and without screen restoring.
void pmsg_body(const char *msg)
{
    char buf[ANSILINESIZE];
    char patten[ANSILINESIZE];
    int len, plen, cc GCC_UNUSED;

    if (cuser.ufo2 & UFO2_ORIGUI)
    {
        vmsg_body(msg);
        return;
    }

    len = (msg ? strlen(msg) : 0);
    plen = BMAX(len - 29, 0) / 2;

    if (len > 0)
    {
        pcopy(patten, "ˍ", plen);
        sprintf(buf, " \x1b[0;40;37mˍˍˍˍˍˍˍˍˍˍˍˍˍˍˍˍˍ%s\x1b[m  ", plen?patten:"");
        vs_line(buf, (b_lines >> 1) - 4, (d_cols >> 1) + 18-plen);
        pcopy(patten, "  ", plen);
        sprintf(buf, " \x1b[0;37;44m▏ 請按任意鍵繼續 .....         %s\x1b[0;47;34m▉\x1b[m ", plen?patten:"");
        vs_line(buf, (b_lines >> 1) - 3, (d_cols >> 1) + 18-plen);
        sprintf(buf, " \x1b[0;37m▏                              %s\x1b[0;47;30m▉\x1b[m\x1b[40;30;1m▉\x1b[m ", plen?patten:"");
        vs_line(buf, (b_lines >> 1) - 2, (d_cols >> 1) + 18-plen);
        sprintf(buf, " \x1b[0;37m▏%-30s%s\x1b[0;47;30m▉\x1b[m\x1b[40;30;1m▉\x1b[m ", msg, ((len > 30 && len%2 == 1) ? " " : ""));
        vs_line(buf, (b_lines >> 1) - 1, (d_cols >> 1) + 18-plen);
        sprintf(buf, " \x1b[0;37m▏                              %s\x1b[0;47;30m▉\x1b[m\x1b[40;30;1m▉\x1b[m ", plen?patten:"");
        vs_line(buf, (b_lines >> 1) + 0, (d_cols >> 1) + 18-plen);
        pcopy(patten, "▇", plen);
        sprintf(buf, " \x1b[0;47;30m▇\x1b[30;1m▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇%s\x1b[40;30;1m▉\x1b[m ", plen?patten:"");
        vs_line(buf, (b_lines >> 1) + 1, (d_cols >> 1) + 18-plen);
    }
    else
    {
        sprintf(buf, " \x1b[0;40;37mˍˍˍˍˍˍˍˍˍˍˍˍˍˍˍˍˍˍ\x1b[m  ");
        vs_line(buf, (b_lines >> 1) - 4, (d_cols >> 1) + 18);
        sprintf(buf, " \x1b[0;37;44m▏ 請按任意鍵繼續 .....           \x1b[0;47;34m▉\x1b[m ");
        vs_line(buf, (b_lines >> 1) - 3, (d_cols >> 1) + 18);
        sprintf(buf, " \x1b[0;37m▏                                \x1b[0;47;30m▉\x1b[m\x1b[40;30;1m▉\x1b[m ");
        vs_line(buf, (b_lines >> 1) - 2, (d_cols >> 1) + 18);
        sprintf(buf, " \x1b[0;37m▏%-30s  \x1b[0;47;30m▉\x1b[m\x1b[40;30;1m▉\x1b[m ", "[請按任意鍵繼續]");
        vs_line(buf, (b_lines >> 1) - 1, (d_cols >> 1) + 18);
        sprintf(buf, " \x1b[0;37m▏                                \x1b[0;47;30m▉\x1b[m\x1b[40;30;1m▉\x1b[m ");
        vs_line(buf, (b_lines >> 1) + 0, (d_cols >> 1) + 18);
        sprintf(buf, " \x1b[0;47;30m▇\x1b[30;1m▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇\x1b[40;30;1m▉\x1b[m ");
        vs_line(buf, (b_lines >> 1) + 1, (d_cols >> 1) + 18);
    }
}

int pmsg(const char *msg)
{
    int cc;
    screen_backup_t old_screen;

    if (cuser.ufo2 & UFO2_ORIGUI)
        return vmsg(msg);

    scr_dump(&old_screen);

    for (;;)
    {
        pmsg_body(msg);
        if ((cc = vkey()) != I_RESIZETERM)
            break;
        scr_restore_keep(&old_screen);
    }

    scr_restore_free(&old_screen);
    return cc;
}

/* verit. 030211 抓取螢幕畫面到暫存檔 */
int
Every_Z_Screen(void)
{
    FILE *fp;
    int i;
    char buf[512];

    screen_backup_t old_screen = {0};

#ifdef M3_USE_PFTERM
    int oy, ox;

    getyx(&oy, &ox);
#endif

    fp = tbf_open(-1);
    if (!fp)
    {
        vmsg("檔案開啟錯誤 !!");
        return 0;
    }

    /* IID.2021-02-26: Restore the screen saved by `popupmenu()`
     * or use the current screen if the saved screen is not available */
    if (popup_old_screen && popup_old_screen->IF_ON(M3_USE_PFTERM, raw_memory, slp))
    {
        scr_dump(&old_screen);
        scr_restore_keep(popup_old_screen);
    }

    for (i=0; i<=b_lines; ++i)
    {
#ifdef M3_USE_PFTERM
        move(i, 0);
        inansistr(buf, 512);
#else
        {
            screenline sl;
            vs_save_line(&sl, i);
            str_sncpy(buf, (char *) sl.data, sizeof(buf), sl.len);
        }
#endif
        fprintf(fp, "%s\n", buf);
    }
    fclose(fp);

    /* Restore the screen if the saved screen is used */
    if (old_screen.IF_ON(M3_USE_PFTERM, raw_memory, slp))
        scr_restore_free(&old_screen);
#ifdef M3_USE_PFTERM
    else
        move(oy, ox);
#endif
    return 1;
}

