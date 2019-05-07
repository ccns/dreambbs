/*-------------------------------------------------------*/
/* popupmenu.c   ( YZU_CSE WindTop BBS )                 */
/*-------------------------------------------------------*/
/* author : verit.bbs@bbs.yzu.edu.tw                     */
/* target : popup menu                                   */
/* create : 2003/02/12                                   */
/*-------------------------------------------------------*/

#include "bbs.h"

#ifdef M3_USE_PFTERM
screen_backup_t old_screen;
#else
static screenline sl[24];
#endif

static int do_menu(MENU pmenu[], XO *xo, int x, int y);

/* ----------------------------------------- */
/* verit 判斷字串的第 pos 各字元是那一種情況 */
/*     return 1  代表中文字的前半部          */
/*           -1  代表中文字的後半部          */
/*            0  代表不是中文字              */
/* ----------------------------------------- */

#ifndef M3_USE_PFTERM
static int
is_big5(char *src, int pos, int mode)
{
    int wstate=0;
    int word=0;
    char *str;

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

        wstate = (*str<0)?((wstate==1)?0:1):0;
        word++;
    }

    if (wstate)
        return -1;

    if (*str<0 && wstate==0)
        return 1;
    else
        return 0;
}
#endif  /* #ifndef M3_USE_PFTERM */

static int
do_cmd(MENU *mptr, XO *xo, int x, int y)
{
    unsigned int mode;
    const void *p;
    int (*func) (void);
    int (*func_unary) (XO *xo);

    if (mptr->umode < 0)
    {
        p = DL_GET(mptr->func);
        if (!p)
            return 0;
        mptr->func = p;
        mptr->umode = - (mptr->umode);
    }

    switch (mptr->umode)
    {
        case POPUP_SO :
            p = DL_GET(mptr->func);
            if (!p) return 0;
            mptr->func = p;
            mptr->umode = POPUP_FUN;
            func = p;
            mode = (*func) ();
            return -1;
        case POPUP_MENU :
//          sprintf(t, "【%s】", mptr->desc);
#ifdef M3_USE_PFTERM
            scr_restore_keep(&old_screen); // restore foot
#else
            vs_restore(sl);
#endif
            return do_menu((MENU *)mptr->func, xo, x, y);
        case POPUP_XO :
            func_unary = mptr->func;
            mode = (*func_unary) (xo);
            return -1;
        case POPUP_FUN :
            func = mptr->func;
            mode = (*func) ();
            return -1;
    }

    return -1;
}

#ifndef M3_USE_PFTERM
/* verit . 計算扣掉色碼的實際長度 */
static int
count_len(
    char *data)
{
    int len;
    char *ptr, *tmp;
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
get_color(char *s, int len, int *fc, int *bc, int *bbc)
{
    char buf[32], *p, *e;
    int color;
    int state = 0, reset=0, exit = 0;

    memset(buf, 0, sizeof(buf));
    strncpy(buf, s+2, len-1);

    for ( p = e = &buf[0]; exit == 0; ++p)
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
vs_line(char *msg, int y, int x)
{
    char buf[512], color[16], *str, *tmp, *cstr;
    int len = count_len(msg);
    int word, slen, fc=37, bc=40, bbc=0;

    memset(buf, 0, sizeof(buf));

    sl[y].data[sl[y].len] = '\0';
    str = tmp = (char *) sl[y].data;

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

    strncpy(buf, (char *) sl[y].data, str - tmp);

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
    char *msg, int y, int x)
{
    move(y, x);

    if (msg)
    {
        outstr(msg);
    }

    move(y, 0);
}
#endif //not M3_USE_PFTERM

/* mode 代表加背景的顏色 */
static void
draw_item(const char *desc, int mode, int x, int y)
{
    char buf[128];

    sprintf(buf, " \x1b[0;37m▏\x1b[4%d;37m%s(\x1b[1;36m%c\x1b[0;37;4%dm)%-25s%s\x1b[0;47;30m▉\x1b[40;30;1m▉\x1b[m ", mode, (mode>0)?"┤":"  ", *desc, mode, desc+1, (mode>0)?"├":"  ");
    vs_line(buf, x, y);
    move(b_lines, 0);
}


static int
draw_menu(MENU *pmenu[20], int num, const char *title, int x, int y, int cur)
{
    char buf[128];
    int i;
    char t[128];

    sprintf(t, "【%s】", title);

    sprintf(buf, " \x1b[0;40;37mˍˍˍˍˍˍˍˍˍˍˍˍˍˍˍˍˍˍ\x1b[m   ");
    vs_line(buf, x-2, y);
    sprintf(buf, " \x1b[0;37;44m▏\x1b[1m%-31s \x1b[0;47;34m▉\x1b[m   ", t);
    vs_line(buf, x-1, y);
    for (i=0; i<num; ++i, ++x)
    {
        draw_item(pmenu[i]->desc, (i==cur)?1:0, x, y);
    }
    sprintf(buf, " \x1b[0;47;30m▇\x1b[30;1m▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇\x1b[40;30;1m▉\x1b[m ");
    vs_line(buf, x, y);
    return 0;
}

static int
do_menu(
    MENU pmenu[],
    XO *xo,
    int x,
    int y)
{
    int cur, old_cur, num, tmp;
    int c;
    MENU *table[20];
    const char *title;
    MENU *table_title;

    memset(table, 0, sizeof(table));
    /* verit. menu 權限檢查 */
    for ( tmp=0, num=-1; pmenu[tmp].umode != POPUP_MENUTITLE; tmp++ )
    {
        if (pmenu[tmp].level == 0 || pmenu[tmp].level & cuser.userlevel)
            table[++num] = &pmenu[tmp];

    }

    /* verit. 更新動態 */
    if (pmenu[tmp].func)
    {
        strcpy(cutmp->mateid, pmenu[tmp].func);
        utmp_mode(M_IDLE);
    }

    title = pmenu[tmp].desc;
    table_title = &pmenu[tmp];

    /* verit . 假如沒有任何選項就 return */
    if (num < 0)
        return 1;

    cur = old_cur = 0;

    /* 跳到預設選項 */
    for ( tmp=0; tmp<= num; tmp++ )
    {
        if ((table[tmp]->desc[0] | 0x20) == ((table_title->level & POPUP_MASK) | 0x20))
        {
            cur = old_cur = tmp;
            break;
        }
    }


    draw_menu(table, num+1, title, x, y, cur);

    while (1)  /* verit . user 選擇 */
    {
        c = vkey();
        old_cur = cur;
        switch(c)
        {
            case KEY_LEFT:
                return 1;
            case KEY_UP:
                cur = (cur==0)?num:cur-1;
                break;
            case KEY_DOWN:
                cur = (cur==num)?0:cur+1;
                break;
            case KEY_HOME:
                cur = 0;
                break;
            case KEY_END:
                cur = num;
                break;
            case KEY_RIGHT:
            case '\n':
                if (table[cur]->umode == POPUP_QUIT)
                    return 1;
                if (do_cmd(table[cur], xo, x, y)<0)
                    return -1;
#ifdef M3_USE_PFTERM
                scr_restore_keep(&old_screen); // restore foot
#else
                vs_restore(sl);
#endif
                draw_menu(table, num+1, title, x, y, cur);
                break;
            default:
                for (tmp=0; tmp<=num; tmp++)
                {
                    if ( (c | 0x20) == (table[tmp]->desc[0] | 0x20 ))
                    {
                        cur = tmp;
                        if (table_title->level & POPUP_DO_INSTANT)
                        {
                            if (table[cur]->umode == POPUP_QUIT)
                                return 1;
                            if (do_cmd(table[cur], xo, x, y)<0)
                                return -1;
#ifdef M3_USE_PFTERM
                            scr_restore_keep(&old_screen); // restore foot
#else
                            vs_restore(sl);
#endif
                            draw_menu(table, num+1, title, x, y, cur);
                        }
                        break;
                    }
                }
                break;
        }
        if (old_cur != cur)
        {
            draw_item(table[old_cur]->desc, 0, x+old_cur, y);
            draw_item(table[cur]->desc, 1, x+cur, y);
        }
    }

    // return 0;
}


/* mode 代表加背景的顏色 */
static void
draw_ans_item(
    char *desc,
    int mode,
    int x,
    int y,
    char hotkey)
{
    char buf[128];

    sprintf(buf, " \x1b[0;37m▏\x1b[4%d;37m%s%c\x1b[1;36m%c\x1b[0;37;4%dm%c%-25s%s\x1b[0;47;30m▉\x1b[40;30;1m▉\x1b[m ", mode, (mode>0)?"┤":"  ", (hotkey==*desc)?'[':'(', *desc, mode, (hotkey==*desc)?']':')', desc+1, (mode>0)?"├":"  ");
    vs_line(buf, x, y);
    move(b_lines, 0);
}


static int
draw_menu_des(char *desc[], char *title, int x, int y, int cur)
{
    int num;
    char buf[128];
    char hotkey;
    hotkey = desc[0][0];

    sprintf(buf, " \x1b[0;40;37mˍˍˍˍˍˍˍˍˍˍˍˍˍˍˍˍˍˍ\x1b[m   ");
    vs_line(buf, x-2, y);
    sprintf(buf, " \x1b[0;37;44m▏%-31s \x1b[0;47;34m▉\x1b[m   ", title);
    vs_line(buf, x-1, y);
    for (num=1; desc[num]; num++)
        draw_ans_item(desc[num], (num==cur)?1:0, x++, y, hotkey);
    sprintf(buf, " \x1b[0;47;30m▇\x1b[30;1m▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇▇\x1b[40;30;1m▉\x1b[m ");
    vs_line(buf, x, y);
    return num-2;
}

/*------------------------------------------------------------- */
/* verit . desc 第一個必須有兩個 char,                          */
/*              第一個字元代表一開始游標停的位置                */
/*              第二個字元代表按下 KEY_LEFT 的預設回傳值        */
/*         desc 最後一個必須為 NULL                             */
/*------------------------------------------------------------- */
int
popupmenu_ans(char *desc[], char *title, int x, int y)
{
    int cur, old_cur, num, tmp;
    int c;
    char t[64];
    char hotkey;
#ifdef M3_USE_PFTERM
    screen_backup_t old_screen = {0};

    scr_dump(&old_screen);
#else
    vs_save(sl);
#endif
    hotkey = desc[0][0];

    sprintf(t, "【%s】", title);
    num = draw_menu_des(desc, t, x, y, 0);
    cur = old_cur = 0;
    for (tmp=0; tmp<num; tmp++)
    {
        if (desc[tmp+1][0] == hotkey)
            cur = old_cur = tmp;
    }
    draw_ans_item(desc[cur+1], 1, x+cur, y, hotkey);

    while (1)
    {
        c = vkey();
        old_cur = cur;
        switch(c)
        {
            case KEY_LEFT:
#ifdef M3_USE_PFTERM
                scr_restore_free(&old_screen); // restore foot
#else
                vs_restore(sl);
#endif
                return (desc[0][1] | 0x20);
            case KEY_UP:
                cur = (cur==0)?num:cur-1;
                break;
            case KEY_DOWN:
                cur = (cur==num)?0:cur+1;
                break;
            case KEY_HOME:
                cur = 0;
                break;
            case KEY_END:
                cur = num;
                break;
            case KEY_RIGHT:
            case '\n':
#ifdef M3_USE_PFTERM
                scr_restore_free(&old_screen); // restore foot
#else
                vs_restore(sl);
#endif
                return (desc[cur+1][0] | 0x20);
            default:
                for (tmp=0; tmp<=num; tmp++)
                {
                    if ( (c | 0x20) == (desc[tmp+1][0] | 0x20 ))
                    {
                        cur = tmp;
                        break;
                    }
                }
                break;
        }
        if (old_cur != cur)
        {
            draw_ans_item(desc[old_cur+1], 0, x+old_cur, y, hotkey);
            draw_ans_item(desc[cur+1], 1, x+cur, y, hotkey);
        }
    }
    // return 0;
}

void
popupmenu(MENU pmenu[], XO *xo, int x, int y)
{
#ifdef M3_USE_PFTERM
    scr_dump(&old_screen);
#else
    vs_save(sl);
#endif
    do_menu(pmenu, xo, x, y);
#ifdef M3_USE_PFTERM
    scr_restore_free(&old_screen); // restore foot
#else
    vs_restore(sl);
#endif
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

int pmsg(const char *msg)
{
    char buf[ANSILINELEN];
    char patten[ANSILINELEN];
    int len, plen, cc;

    if (cuser.ufo2 & UFO2_ORIGUI)
        return vmsg(msg);

#ifdef M3_USE_PFTERM
    scr_dump(&old_screen);
#else
    vs_save(sl);
#endif

    len = (msg ? strlen(msg) : 0 );
    if (len > 30)
    {
        plen = (len - 29) / 2;
    }
    else
        plen = 0;

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

    cc = vkey();
#ifdef M3_USE_PFTERM
    scr_restore_free(&old_screen); // restore foot
#else
    vs_restore(sl);
#endif
    return cc;
}

/* verit. 030211 抓取螢幕畫面到暫存檔 */
int
Every_Z_Screen(void)
{
    FILE *fp;
    int i;
    char buf[512];
#ifdef M3_USE_PFTERM
    int oy, ox;

    getyx(&oy, &ox);
#endif

    fp = tbf_open();
    if (!fp)
    {
        vmsg("檔案開啟錯誤 !!");
        return 0;
    }
    for (i=0; i<=b_lines; ++i)
    {
        memset(buf, 0, sizeof(buf));
#ifdef M3_USE_PFTERM
        move(i, 0);
        inansistr(buf, 512);
#else
        strncpy(buf, (char *) sl[i].data, sl[i].len);
#endif
        fprintf(fp, "%s\n", buf);
    }
    fclose(fp);

#ifdef M3_USE_PFTERM
    move(oy, ox);
#endif
    return 1;
}

