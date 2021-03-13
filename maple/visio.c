/*-------------------------------------------------------*/
/* visio.c      ( NTHU CS MapleBBS Ver 3.00 )            */
/*-------------------------------------------------------*/
/* target : VIrtual Screen Input Output routines         */
/* create : 95/03/29                                     */
/* update : 19/07/28                                     */
/*-------------------------------------------------------*/


#include <stdarg.h>
#include <arpa/telnet.h>

#include "bbs.h"

#define VO_MAX  (5120)
#define VI_MAX  (256)

#define INPUT_ACTIVE    0
#define INPUT_IDLE      1

#define t_lines    (b_lines + 1)
#define p_lines    (b_lines - 5)
#define t_columns  (b_cols  + 1)

#ifdef M3_USE_PFTERM
// filed color   (defined in theme.h)
#define STANDOUT   (void) ( attrsetbg(FILEDBG), attrsetfg(FILEDFG) )
// default color (\x1b[37; 40m)
#define STANDEND   (void) ( attrsetbg(0), attrsetfg(7) )
#endif  /* #ifdef M3_USE_PFTERM */
int cur_row, cur_col;           /* Current position without ANSI codes (display coordination) */
int cur_pos;                    /* current column position with ANSI codes (raw character coordination) */

/* ----------------------------------------------------- */
/* output routines                                       */
/* ----------------------------------------------------- */


static char vo_pool[VO_MAX];
static int vo_size;


#ifdef  VERBOSE
static void
telnet_flush(
    const char *data,
    int size)
{
    int oset;

    oset = 1;

    if (select(1, NULL, &oset, NULL, NULL) <= 0)
    {
        abort_bbs();
    }

    xwrite(0, data, size);
}

#else

# define telnet_flush(data, size)       send(0, data, size, 0)
#endif  /* #ifdef  VERBOSE */


#ifdef M3_USE_PFTERM
void
#else
static void
#endif
oflush(void)
{
    int size;

    if ((size = vo_size))
    {
        telnet_flush(vo_pool, size);
        vo_size = 0;
    }
}

#ifndef M3_USE_PFTERM
static void
output(
    const char *str,
    int len)
{
    int size, ch;
    char *data;

    size = vo_size;
    data = vo_pool;
    if (size + len >= VO_MAX - 8)
    {
        telnet_flush(data, size);
        size = len;
    }
    else
    {
        data += size;
        size += len;
    }

    while (--len >= 0)
    {
        ch = (unsigned) *str++;
        *data++ = ch;
        if (ch == IAC)
        {
            *data++ = ch;
            size++;
        }
    }
    vo_size = size;
}
#endif  /* #ifndef M3_USE_PFTERM */

#ifdef M3_USE_PFTERM
void
#else
static void
#endif
ochar(
    int ch)
{
    char *data;
    int size;

    data = vo_pool;
    size = vo_size;
    if (size >= VO_MAX - 2)
    {
        telnet_flush(data, size);
        size = 0;
    }
    data[size++] = ch;
    if (ch == IAC)  /* `'\xff'` => `IAC` `IAC` */
    {
        data[size++] = ch;
    }
    vo_size = size;
}


void
bell(void)
{
    static const char sound[] = {Ctrl('G')};

    telnet_flush(sound, sizeof(sound));
}


/* ----------------------------------------------------- */
/* virtual screen                                        */
/* ----------------------------------------------------- */

#ifndef M3_USE_PFTERM
#define o_ansi(x)       output(x, STRLITLEN(x))

#define o_clear()       o_ansi("\x1b[;H\x1b[2J")
#define o_cleol()       o_ansi("\x1b[K")
#define o_standup()     o_ansi("\x1b[7m")
#define o_standdown()   o_ansi("\x1b[m")


static bool docls;
static int roll;
static int scrollcnt, tc_col, tc_row;


static screenline vbuf[T_LINES];  //r2: maximum totel lines (t_lines)
//static
screenline *cur_slp;    /* current screen line pointer */


void
move(
    int y,
    int x)
{
    screenline *cslp;

    y = BMIN(y, b_lines);
    x = BMIN(x, b_cols);

    cur_row = y;
    if ((y += roll) >= t_lines)
        y -= t_lines;
    cur_slp = cslp = &vbuf[y];
    cur_col = x;

    /* ------------------------------------- */
    /* 過濾 ANSI codes，計算游標真正所在位置 */
    /* ------------------------------------- */

#if 0
    if (x && (cslp->mode & SL_ANSICODE))
    {
        int ch;
        bool ansi;
        unsigned char *str;

        ansi = false;
        y = x;
        str = cslp->data;
        while (x && (ch = *str))
        {
            str++;
            if (ch == KEY_ESC)
            {
                ansi = true;
                y++;
                continue;
            }
            if (ansi)
            {
                y++;
                if (ch == 'm')          /* (!strchr(str_ansicode, ch)) */
                {
                    ansi = false;
                }
                continue;
            }
            x--;
        }
        x += y;
    }
#endif

    cur_pos = x;
}

/* verit : 030212, 扣掉 ansi code */
void
move_ansi(
    int y,
    int x)
{
    screenline *cslp;

    y = BMIN(y, b_lines);

    cur_row = y;
    if ((y += roll) >= t_lines)
        y -= t_lines;
    cur_slp = cslp = &vbuf[y];
    if (x >= t_columns)
    {
        cur_pos = cslp->len - 1;
        cur_col = t_columns - 1;
        return;
    }
    cur_col = x;

    if (x)
    {
        int ch;
        bool ansi;
        int len;
        unsigned char *str;

        ansi = false;
        y = x;
        str = cslp->data;
        str[(len = cslp->len)] = '\0';
        while (len && (ch = *str))
        {
            str++;
            len--;
            if (ch == KEY_ESC)
            {
                ansi = true;
                y++;
                continue;
            }
            if (ansi)
            {
                y++;
                if (ch == 'm')          /* (!strchr(str_ansicode, ch)) */
                {
                    ansi = false;
                }
                continue;
            }
            x--;
            if (x<=0 && !ansi)
                break;
        }
        x = y;
    }

    cur_pos = x;

}

void
getyx(
    int *y, int *x)
{
    *y = cur_row;
    *x = cur_col;
}


/*-------------------------------------------------------*/
/* 計算 slp 中 len 之處的游標 column 所在                */
/*-------------------------------------------------------*/


#if 0
static int
ansicol(
    const screenline *slp,
    int len)
{
    const unsigned char *str;
    int ch, col;
    bool ansi;

    if (!len || !(slp->mode & SL_ANSICODE))
        return len;

    col = 0;
    ansi = false;
    str = slp->data;

    while (len-- && (ch = *str++))
    {
        if (ch == KEY_ESC && *str == '[')
        {
            ansi = true;
            continue;
        }
        if (ansi)
        {
            if (ch == 'm')
                ansi = false;
            continue;
        }
        col++;
    }
    return col;
}
#endif


static void
rel_move(
    int new_col, int new_row)
{
    int was_col, was_row;
    char buf[16];

    new_row = BMIN(new_row, b_lines);
    new_col = BMIN(new_col, b_cols);

    was_col = tc_col;
    was_row = tc_row;

    tc_col = new_col;
    tc_row = new_row;

    if (new_col == 0)
    {
        if (new_row == was_row)
        {
            if (was_col)
                ochar('\r');
            return;
        }
        else if (new_row == was_row + 1)
        {
            ochar('\n');
            if (was_col)
                ochar('\r');
            return;
        }
    }

    if (new_row == was_row)
    {
        if (was_col == new_col)
            return;

        if (new_col == was_col - 1)
        {
            ochar(Ctrl('H'));
            return;
        }
    }

    sprintf(buf, "\x1b[%d;%dH", new_row + 1, new_col + 1);
    output(buf, strlen(buf));
}


static void
standoutput(
    const screenline *slp,
    int ds, int de)
{
    const char *data;
    int sso, eso;

    data = (const char *) slp->data;
    sso = slp->sso;
    eso = slp->eso;

    if (eso <= ds || sso >= de)
    {
        output(data + ds, de - ds);
        return;
    }

    if (sso > ds)
        output(data + ds, sso - ds);
    else
        sso = ds;

    o_standup();
    output(data + sso, BMIN(eso, de) - sso);
    o_standdown();

    if (de > eso)
        output(data + eso, de - eso);
}


#define STANDOUT   (void) ( cur_slp->sso = cur_pos, cur_slp->mode |= SL_STANDOUT )
#define STANDEND   (void) ( cur_slp->eso = cur_pos )


#if 0
static bool standing;


static void
standout(void)
{
    if (!standing)
    {
        standing = true;
        cur_slp->sso = cur_slp->eso = cur_pos;
        cur_slp->mode |= SL_STANDOUT;
    }
}


static void
standend(void)
{
    if (standing)
    {
        standing = false;
        if (cur_slp->eso < cur_pos)
            cur_slp->eso = cur_pos;
    }
}
#endif

#endif   // #ifdef M3_USE_PFTERM

#ifdef M3_USE_PFTERM
void vs_redraw(void)
{
    redrawwin(); refresh(); return;
}
#else
static void
vs_redraw(void)
{
    screenline *slp;
    int i, j, len, mode, width;

    tc_col = tc_row = scrollcnt = vo_size = i = 0;
    docls = false;
    o_clear();
    for (slp = &vbuf[j = roll]; i < t_lines; i++, j++, slp++)
    {
        if (j >= t_lines)
        {
            j = 0;
            slp = vbuf;
        }

        len = slp->len;
        width = slp->width;
        slp->oldlen = width;
        mode = slp->mode &=
            (len <= slp->sso) ? ~(SL_MODIFIED | SL_STANDOUT) : ~(SL_MODIFIED);
        if (len)
        {
            rel_move(0, i);

            if (mode & SL_STANDOUT)
                standoutput(slp, 0, len);
            else
                output((char *) slp->data, len);

            tc_col = width;
        }
    }
    rel_move(cur_col, cur_row);
    oflush();
}


void
refresh(void)
{
    screenline *slp;
    int i, j, len, mode, width, smod, emod;

    i = scrollcnt;

    if ((docls) || (UABS(i) >= p_lines))
    {
        vs_redraw();
        return;
    }

    if (i)
    {
        char buf[T_LINES - 6];

        scrollcnt = j = 0;
        if (i < 0)
        {
            sprintf(buf, "\x1b[%dL", -i);
            i = strlen(buf);
        }
        else
        {
            do
            {
                buf[j] = '\n';
            } while (++j < i);
            j = b_lines;
        }
        rel_move(0, j);
        output(buf, i);
    }

    for (i = 0, slp = &vbuf[j = roll]; i < t_lines; i++, j++, slp++)
    {
        if (j >= t_lines)
        {
            j = 0;
            slp = vbuf;
        }

        len = slp->len;
        width = slp->width;
        mode = slp->mode;

        if (mode & SL_MODIFIED)
        {
            slp->mode = mode &=
                (len <= slp->sso) ? ~(SL_MODIFIED | SL_STANDOUT) : ~(SL_MODIFIED);

            if ((smod = slp->smod) < len)
            {
                emod = BMIN(slp->emod + 1, len);

                rel_move(smod, i);

                /* rel_move(ansicol(slp, smod), i); */

                if (mode & SL_STANDOUT)
                    standoutput(slp, smod, emod);
                else
                    output((char *) &slp->data[smod], emod - smod);

                /* tc_col = ansicol(slp, emod); */

#if 0                           /* 0501 */
                if (mode & SL_ANSICODE)
                {
                    unsigned char *data;

                    data = slp->data;
                    mode = 0;
                    len = emod;

                    while (len--)
                    {
                        smod = *data++;
                        if (smod == KEY_ESC)
                        {
                            mode = 1;
                            emod--;
                            continue;
                        }

                        if (mode)
                        {
                            if (smod == 'm')
                                mode = 0;
                            emod--;
                        }
                    }
                }

                tc_col = emod;
#endif

                tc_col = (width != len) ? width : emod;
            }
        }

        if (slp->oldlen > width)
        {
            rel_move(width, i);
            o_cleol();
        }
        slp->oldlen = width;
    }
    rel_move(cur_col, cur_row);
    oflush();
}


void
clear(void)
{
    int i;
    screenline *slp;

    docls = true;
    cur_pos = cur_col = cur_row = roll = i = 0;
    cur_slp = slp = vbuf;
    while (i++ < t_lines)
    {
        memset(slp++, 0, offsetof(screenline, data) + 1);
    }
}

void
clearange(
    int from, int to)
{
    int i;
    screenline *slp;

    docls = true;
    cur_pos = cur_col = roll = 0;
    i = cur_row = from;
    cur_slp = slp = &vbuf[from];
    while (i++ < to)
    {
        memset(slp++, 0, offsetof(screenline, data) + 1);
    }
}

void
clrtoeol(void)
{
    screenline *slp = cur_slp;
    int len;

    if ((len = cur_pos))
    {
        if (len > slp->len)
            for (len = slp->len; len < cur_pos; len++)
                slp->data[len] = ' ';
        slp->len = len;
        slp->width = cur_col;
    }
    else
    {
        memset((char *) slp + sizeof(slp->oldlen), 0, offsetof(screenline, data) + 1 - sizeof(slp->oldlen));
    }
}


void
clrtobot(void)
{
    screenline *slp;
    int i, j;

    i = cur_row;
    j = i + roll;
    slp = cur_slp;
    while (i < t_lines)
    {
        if (j >= t_lines)
        {
            j = 0;
            slp = vbuf;
        }
        memset((char *) slp + sizeof(slp->oldlen), 0, offsetof(screenline, data) + 1 - sizeof(slp->oldlen));

#if 0
        if (slp->oldlen)
            slp->oldlen = 255;
#endif

        i++;
        j++;
        slp++;
    }
}

void
outc(
    int ch)
{
    screenline *slp;
    unsigned char *data;
    int i, cx, pos;

    static char ansibuf[16] = "\x1b";
    static int ansipos = 0;

    slp = cur_slp;
    pos = cur_pos;

    if (ch == '\n')
    {
        cx = cur_col;

new_line:

        ansipos = 0;
        if (pos)
        {
            slp->len = pos;
            slp->width = cx;

#if 0
            if (standing)
            {
                standing = false;
                if (pos <= slp->sso)
                    slp->mode &= ~SL_STANDOUT;
                else if (slp->eso < pos)
                    slp->eso = pos;
            }
#endif
        }
        else
        {
            memset((char *) slp + sizeof(slp->oldlen), 0, offsetof(screenline, data) + 1 - sizeof(slp->oldlen));
        }

        move(cur_row + 1, 0);
        return;
    }

    if (ch < 0x20)
    {
        if (ch == KEY_ESC)
            ansipos = 1;

        return;
    }

    data = &(slp->data[pos]);   /* 指向目前輸出位置 */

    /* -------------------- */
    /* 補足所需要的空白字元 */
    /* -------------------- */

    cx = slp->len - pos;
    if (cx > 0)
    {
        cx = *data;
    }
    else
    {
        while (cx < 0)
        {
            data[cx++] = ' ';
        }

        slp->len = /* slp->width = */ pos + 1;
    }

    /* ---------------------------- */
    /* ANSI control code 之特別處理 */
    /* ---------------------------- */

    if ((i = ansipos))
    {
        if ((i < 15) &&
            ((ch >= '0' && ch <= '9') || ch == '[' || ch == 'm' || ch == ';'))
        {
            ansibuf[i++] = ch;

            if (ch != 'm')
            {
                ansipos = i;
                return;
            }

            if (showansi)
            {
                ch = i + pos;
                if (ch < ANSILINESIZE - 1)
                {
                    memcpy(data, ansibuf, i);
                    slp->len = slp->emod = cur_pos = ch;
                    slp->mode |= SL_MODIFIED;
                    if (slp->smod > pos)
                        slp->smod = pos;
                }
            }
        }
        ansipos = 0;
        return;
    }

    /* ---------------------------- */
    /* 判定哪些文字需要重新送出螢幕 */
    /* ---------------------------- */

    if ( /* !(slp->mode & SL_ANSICODE) && */ (ch != cx) )
    {
        *data = ch;
        cx = slp->mode;
        if (cx & SL_MODIFIED)
        {
            if (slp->smod > pos)
                slp->smod = pos;
            if (slp->emod < pos)
                slp->emod = pos;
        }
        else
        {
            slp->mode = cx | SL_MODIFIED;
            slp->smod = slp->emod = pos;
        }
    }

    cur_pos = ++pos;
    cx = ++cur_col;

    if ((pos >= ANSILINESIZE) /* || (cx >= t_columns) */ )
        goto new_line;

    if (slp->width < cx)
        slp->width = cx;
}


void
outns(
    const char *str, int n)
{
    int ch;

    while (n-- > 0 && (ch = (unsigned char) *str))
    {
        outc(ch);
        str++;
    }
}

void
outs(
    const char *str)
{
    outns(str, strlen(str));
}

#endif // #ifdef M3_USE_PFTERM

/* Screen size referencing coordinate mapping */

/* IID.20200113: Map a cursor referencing to screen size to real position */
/* `(T_LINES_REF/a + b, T_COLS_REF/c + d)` is mapped to `(t_lines/a + b, t_columns/c + d)`
      if `a` divides `T_LINES_DIV_RES` and `abs(b) < T_LINES_OFF_MAX`
         and `c` divides `T_COLS_DIV_RES` and `abs(d) < T_COLS_OFF_MAX` */
/* Useful for functions which use fixed `y` and `x` and need to handle screen resizing */
GCC_PURE int
gety_ref(
    int y_ref)
{
    /* Substitute `T_LINES` with `t_lines` */
    const int y_clipped = TCLAMP(y_ref, 0, T_LINES_REF - 1);
    /* Calculate the multiplier/divisor (`n*`, `/n`) */
    const int y_segment = y_clipped / (2*T_LINES_OFF_MAX);
    /*    and then substitute the multiplicand/dividend with `t_lines` */
    const int y_base = y_segment * t_lines / T_LINES_DIV_RES;
    /* Division truncation compensation, for making segments continuous */
    const int y_compensate = (y_segment * t_lines % T_LINES_DIV_RES != 0);
    /* Get offset (`+n`, `-n`) (with range `[0, 2*T_LINES_OFF_MAX)`) */
    /*    `[0, T_LINES_OFF_MAX)` => positive (`[0, T_LINES_OFF_MAX)`);
          `[T_LINES_OFF_MAX, 2*T_LINES_OFF_MAX)` => negative (`[-T_LINES_OFF_MAX, 0)`) */
    const int y_offset = y_clipped % (2*T_LINES_OFF_MAX);
    return y_base + y_compensate + y_offset - ((y_offset >= T_LINES_OFF_MAX) ? 2*T_LINES_OFF_MAX : 0);
}
GCC_PURE int
getx_ref(
    int x_ref)
{
    /* Substitute `T_COLS` with `t_columns` */
    /* Do the same with `x` */
    const int x_clipped = TCLAMP(x_ref, 0, T_COLS_REF - 1);
    const int x_segment = x_clipped / (2*T_COLS_OFF_MAX);
    const int x_base = x_segment * t_columns / T_COLS_DIV_RES;
    const int x_compensate = (x_segment * t_columns % T_COLS_DIV_RES != 0);
    const int x_offset = x_clipped % (2*T_COLS_OFF_MAX);
    return x_base + x_compensate + x_offset - ((x_offset >= T_COLS_OFF_MAX) ? 2*T_COLS_OFF_MAX : 0);
}

void
move_ref(
    int y,
    int x)
{
    move_ansi(gety_ref(y), getx_ref(x));
}

/* ----------------------------------------------------- */
/* eXtended output: 秀出 user 的 name 和 nick            */
/* ----------------------------------------------------- */

/* 090924.cache: pmore使用的控制碼 */

int
expand_esc_star_visio(char *buf, const char *src, int szbuf)
{
    if (*src != KEY_ESC)
    {
        strlcpy(buf, src, szbuf);
        return 0;
    }

    if (*++src != '*') // unknown escape... strip the ESC.
    {
        strlcpy(buf, src, szbuf);
        return 0;
    }

    switch (*++src)
    {
        // insecure content
        case 's':   // current user id
            strlcpy(buf, cuser.userid, szbuf);
            return 2;
        case 'n':   // current user nick
            strlcpy(buf, cuser.username, szbuf);
            return 2;
        case 'l':   // current user logins
            snprintf(buf, szbuf, "%d", cuser.numlogins);
            return 2;
        case 'p':   // current user posts
            snprintf(buf, szbuf, "%d", cuser.numposts);
            return 2;
        case 't':   // current time
            strlcpy(buf, Now(), szbuf);
            return 1;
    }

    // unknown characters, return from star.
    strlcpy(buf, src-1, szbuf);
    return 0;
}

#ifdef SHOW_USER_IN_TEXT
void
outx(
    const char *str)
{
/*
    unsigned char *t_name = cuser.userid;
    unsigned char *t_nick = cuser.username;
*/

    time_t now;
    int ch;

/* cache.090922: 控制碼 */

    while ((ch = (unsigned char) *str))
    {
        if (ch == KEY_ESC)
        {
            ch = (unsigned char) str[1];
            if (ch == '*')
            {
                ch = (unsigned char) str[2];
                switch (ch)
                {
                    case 's':       /* **s 顯示 ID */
                        outs(cuser.userid);
                        str += 3;
                        break;
                    case 'n':       /* **n 顯示暱稱 */
                        outs(cuser.username);
                        str += 3;
                        break;
                    case 't':       /* **t 顯示日期 */
                        time(&now);
                        outs(Ctime(&now));
                        str += 3;
                        break;
                    default:
                        break;
                }
            }
            ch = (unsigned char) str[0];
        }
        outc(ch);
        str++;
    }
/*
    while ((ch = (unsigned char) *str))
    {



        switch (ch)
        {
        case 1:
            if ((ch = *t_name) && (cuser.ufo2 & UFO2_SHOWUSER))
                t_name++;
            else
                ch = (cuser.ufo2 & UFO2_SHOWUSER) ? ' ' : '#';
            break;

        case 2:
            if ((ch = *t_nick) && (cuser.ufo2 & UFO2_SHOWUSER))
                t_nick++;
            else
                ch = (cuser.ufo2 & UFO2_SHOWUSER) ? ' ' : '%';
        }
        outc(ch);
        str++;
    }
*/

}
#endif  /* #ifdef SHOW_USER_IN_TEXT */

/* ----------------------------------------------------- */
/* clear the bottom line and show the message            */
/* ----------------------------------------------------- */


void
outnz(
    const char *msg, int n)
{
    move(b_lines, 0);
    clrtoeol();
    outns(msg, n);
}

void
outz(
    const char *msg)
{
    outnz(msg, strlen(msg));
}

void
outf(
    const char *str)
{
    const int lstr_len = strcspn(str, "\t");
    outnz(str, lstr_len);
    prints("%*s%s\x1b[m", d_cols, "", str + str_nlen(str, lstr_len + 1));
}

#ifdef M3_USE_PFTERM

void outl (int line, const char *msg)     /* line output */
{
    move (line, 0);
    clrtoeol();


    if (msg != NULL)
        outs(msg);
}

#define ANSI_COLOR_CODE            "[m;0123456789"
#define ANSI_COLOR_END     "m"

void outr (const char *str)
/* restricted output (strip the ansiscreen contolling code only) */
{
    char ch, buf[256], *p = NULL;
    int ansi = 0;

    while ((ch = *str++))
    {
        if (ch == KEY_ESC)
        {
            ansi = 1;
            p = buf;
            *p++ = ch;
        }
        else if (ansi)
        {
            if (p)
                *p++ = ch;

            if (!strchr(ANSI_COLOR_CODE, ch))
            {
                ansi = 0;
                buf[0] = '\0';
                p = NULL;
            }
            else if (strchr(ANSI_COLOR_END, ch))
            {
                *p++ = '\0';
                ansi = 0;
                p = NULL;
                outs(buf);

            }
        }
        else
            outc((unsigned char)ch);
    }
}

#endif //M3_USE_PFTERM

/* IID.20191224: Output a repeated pattern as a separator */
void
outsep(
    int xend, const char *pat)
{
    const char *str = pat;
    int x;
    bool dbcs_hi = false;
    char hi;

    getyx(&SINKVAL(int), &x);

    if (!str || !*str)
    {
        prints("%*s\x1b[m", BMAX(xend - x, 0), "");
        return;  /* Prevent infinity loops */
    }

    while (x < xend)
    {
        int ch;
        if (!(ch = *str++))
        {
            str = pat;
            if (dbcs_hi)
            {
                dbcs_hi = false;
                outc(' ');
            }
            continue;
        }
        if (dbcs_hi)
        {
            dbcs_hi = false;
            outc(hi);
        }
        else if (IS_DBCS_HI(ch))
        {
            dbcs_hi = true;
            hi = ch;
            x++;
            continue;
        }
        outc(ch);
        x++;
    }

    if (dbcs_hi)
    {
        dbcs_hi = false;
        outc(' ');
    }

    outs("\x1b[m");
}

GCC_FORMAT(1, 2) void
prints(const char *fmt, ...)
{
    va_list args;
    char buf[512];

    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);
    outs(buf);
}

#ifndef M3_USE_PFTERM
void
scroll(void)
{
    scrollcnt++;
    if (++roll >= t_lines)
        roll = 0;
    move(b_lines, 0);
    clrtoeol();
}

void
rscroll(void)
{
    scrollcnt--;
    if (--roll < 0)
        roll = b_lines;
    move(0, 0);
    clrtoeol();
}
#endif //M3_USE_PFTERM

/* ----------------------------------------------------- */

#ifdef M3_USE_PFTERM
static int save_y, save_x;

void
cursor_save(void)
{
    getyx(&save_y, &save_x);
}

void
cursor_restore(void)
{
    move(save_y, save_x);
}
#else
static int old_col, old_row;
static int old_pos; /* Thor.990401: 多存一個 */


/* static void */
void                            /* Thor.1028: 為了讓 talk.c
                                 * 有人呼叫時會show字 */
cursor_save(void)
{
    old_col = cur_col;
    old_row = cur_row;

    old_pos = cur_pos; /* Thor.990401: 多存一個 */
}


/* static void */
void                            /* Thor.1028: 為了讓 talk.c
                                 * 有人呼叫時會show字 */
cursor_restore(void)
{
    move(old_row, old_col);

    cur_pos = old_pos; /* Thor.990401: 多還原一個 */
}


void
save_foot(
    screenline *slp)
{
#if 0
    cursor_save();
    /* Thor.980827: 所有用到 save_foot的時機都會重新定位, 故不用存游標位置 */
#endif

    move(b_lines - 1, 0);
    memcpy(slp, cur_slp, sizeof(screenline) * 2);
    slp[0].smod = 0;
    slp[0].emod = 255; /* Thor.990125:不論最後一次改到哪, 全部要繪上 */
    slp[0].oldlen = 255;
    slp[0].mode |= SL_MODIFIED;
    slp[1].smod = 0;
    slp[1].emod = 255; /* Thor.990125:不論最後一次改到哪, 全部要繪上 */
    slp[1].oldlen = 255;
    slp[1].mode |= SL_MODIFIED;
}


void
restore_foot(
    const screenline *slp)
{
    move(b_lines - 1, 0);
    memcpy(cur_slp, slp, sizeof(screenline) * 2);
#if 0
    cursor_restore();
    /* Thor.980827: 所有用到 restore_foot的時機都會重新定位, 故不用存游標位置 */
#endif
    refresh(); /* Thor.981222: 為防整screen重繪, 採用 refresh */
    /* vs_redraw();*/  /* lkchu.981201: 用 refresh() 會把 b_lines - 1 行清掉 */
}


void
vs_save_line(
    screenline *slp,
    int y)
{
    memcpy(slp, &vbuf[y], sizeof(screenline));
}


void
vs_restore_line(
    const screenline *slp,
    int y)
{
    memcpy(&vbuf[y], slp, sizeof(screenline));
}


int
vs_save(
    screen_backup_t *psb)
{
    psb->slp = NULL;
    return vs_resave(psb);
}

int
vs_resave(
    screen_backup_t *psb)
{
#if 0
    cursor_save();
    /* Thor.980827: 所有用到 vs_save的時機都會重新定位, 故不用存游標位置 */
#endif
    psb->old_t_lines = t_lines;
    psb->old_roll = roll;
    psb->slp = (screenline *)realloc(psb->slp, sizeof(screenline) * t_lines);
    memcpy(psb->slp, vbuf, sizeof(screenline) * t_lines);
    return psb->old_roll;   /* itoc.030723: 傳回目前的 roll */
}


void
vs_free(
    screen_backup_t *psb)
{
    free(psb->slp);
    psb->slp = NULL;
}

void
vs_restore_free(
    screen_backup_t *psb)
{
    vs_restore(psb);
    vs_free(psb);
}

void
vs_restore(
    const screen_backup_t *psb)
{
    memcpy(vbuf, psb->slp, sizeof(screenline) * psb->old_t_lines);
#if 0
    cursor_restore();
    /* Thor.980827: 所有用到 vs_restore的時機都會重新定位, 故不用存游標位置 */
#endif
    roll = psb->old_roll;
    vs_redraw();
}

#endif  // M3_USE_PFTERM

#define VMSG_NULL "\x1b[1;37;45m%*s● 請按任意鍵繼續 ●%*s\x1b[m"

// IID.20190909: `vmsg()` without blocking.
void
vmsg_body(
    const char *msg)             /* length <= 54 */
{
    move(b_lines, 0);
    clrtoeol();
    if (msg)
    {
        prints(COLOR1 " ★ %-*s " COLOR2 " [請按任意鍵繼續] \x1b[m", d_cols + 56, msg);
    }
    else
    {
#ifdef HAVE_COLOR_VMSG
        int color;
        color =time(0)%6+31;
        prints("\x1b[1;%dm%*s▏▎▍▌▋▊▉ \x1b[1;37m請按任意鍵繼續 \x1b[1;%dm▉\x1b[m ", color, d_cols + 47, "", color);
#else
        outs(VMSG_NULL, (d_cols >> 1) + 30, "", (d_cols+1 >> 1) + 29, "");
#endif
#ifdef M3_USE_PFTERM
        move(b_lines, 0);
#endif
    }
}

int
vmsg(
    const char *msg)             /* length <= 54 */
{
    int b_lines_prev = b_lines;
    int res_key;
    for (;;)
    {
        vmsg_body(msg);
        if ((res_key = vkey()) != I_RESIZETERM)
            break;
        move(b_lines_prev, 0);
        clrtoeol();
        b_lines_prev = b_lines;
    }
    return res_key;
}

static inline void
zkey(void)                              /* press any key or timeout */
{
    /* static */ struct timeval tv = {1, 100};
    /* Thor.980806: man page 假設 timeval struct是會改變的 */

    fd_set rset;

    FD_ZERO(&rset);
    FD_SET(0, &rset);

    select(1, &rset, NULL, NULL, &tv);

#if 0
    if (select(1, &rset, NULL, NULL, &tv) > 0)
    {
        recv(0, &rset, sizeof(&rset), 0);
    }
#endif
}


void
zmsg(                   /* easy message */
    const char *msg)
{
#if 0
    move(b_lines, 0);
    clrtoeol();
    outs(msg);
#endif
    outz(msg); /* Thor.980826: 就是 outz嘛 */

    refresh();
    zkey();
}


void
vs_bar(
    const char *title)
{
#ifdef  COLOR_HEADER
/*  int color = (time(0) % 7) + 41;        lkchu.981201: random color */
    int color = 44; //090911.cache: 太花了固定一種顏色
#endif

    clear();
#ifdef  COLOR_HEADER
    prints("\x1b[1;%2d;37m【%s】\x1b[m\n", color, title);
#else
    prints("\x1b[1;46;37m【%s】\x1b[m\n", title);
#endif
}


void
cursor_bar_show(
    int row, int column, int width)
{
    move(row, column);
    if (HAVE_UFO2_CONF(UFO2_MENU_LIGHTBAR))
    {
        if (width > 0)
            grayoutrect(row, row + 1, column, column + width + 2, GRAYOUT_COLORBOLD);
        else
            grayout(row, row + 1, GRAYOUT_COLORBOLD);
    }
    outs(STR_CURSOR);
    move(row, column + 1);
}


void
cursor_bar_clear(
    int row, int column, int width)
{
    move(row, column);
    if (HAVE_UFO2_CONF(UFO2_MENU_LIGHTBAR))
    {
        if (width > 0)
            grayoutrect(row, row + 1, column, column + width + 2, GRAYOUT_COLORNORM);
        else
            grayout(row, row + 1, GRAYOUT_COLORNORM);
    }
    outs(STR_UNCUR);
}


int
cursor_bar_key(
    int row, int column, int width)
{
    int ch;

    cursor_bar_show(row, column, width);
    ch = vkey();
    cursor_bar_clear(row, column, width);
    return ch;
}


void
cursor_show(
    int row, int column)
{
    cursor_bar_show(row, column, 0);
}


void
cursor_clear(int row, int column)
{
    cursor_bar_clear(row, column, 0);
}

int
cursor_key(int row, int column)
{
    return cursor_bar_key(row, column, 0);
}

static void
vs_line(
    const char *msg)
{
    int head, tail;

    if (msg)
        head = (strlen(msg) + 1) >> 1;
    else
        head = 0;

    tail = head;

    while (head++ < (b_cols >> 1) - 1)
        outc('-');

    if (tail)
    {
        outc(' ');
        outs(msg);
        outc(' ');
    }

    while (tail++ < ((b_cols+1) >> 1) - 1)
        outc('-');
    outc('\n');
}

#ifndef M3_USE_PFTERM
/* For interface compatibility with pfterm */
void grayoutrect(int y, int yend, int x, int xend, int level)
{
    return grayout(y, yend, level);
}

/* 090127.cache 淡入淡出特效 */
void
grayout(int y, int end, int level)
// GRAYOUT_DARK(0): dark, GRAYOUT_BOLD(1): bold, GRAYOUR_NORMAL(2): normal
{
    static const char *const prefix[3] = { "\x1b[1;30m", "\x1b[1;37m", "\x1b[0;37m" };
    char buf[ANSILINESIZE];

    if (level < 0 || level > 2)
        return;

    y = BMAX(y, 0);

    for (int i = y; i < end; i++)
    {
        if (!vbuf[i].width)
            continue;

        vbuf[i].oldlen = vbuf[i].len;
        vbuf[i].len = vbuf[i].width + 7 + 3;

        str_ansi(buf, (char *) vbuf[i].data, vbuf[i].width + 1);
        sprintf((char *) vbuf[i].data, "%s%s\x1b[m", prefix[level], buf);
    }
    vs_redraw();
}
#endif //#ifndef M3_USE_PFTERM

/* ----------------------------------------------------- */
/* input routines                                        */
/* ----------------------------------------------------- */

// IID.20190502: Exposed to BBS-Lua/BBS-Ruby.
#ifdef __cplusplus
extern "C" {
#endif
extern const int vi_max;
const int vi_max = VI_MAX;
unsigned char vi_pool[VI_MAX];
int vi_size;
int vi_head;
time_t idle;
#ifdef __cplusplus
}  /* extern "C" */
#endif


/* static int vio_fd; */
int vio_fd;                     /* Thor.0725: 為以後在talk & chat 進 ^z 作準備 */

#ifdef EVERY_Z
int holdon_fd;                  /* Thor.0727: 跳出chat&talk暫存vio_fd用 */
#endif


#define VKEY_ESC_WAIT_TIME_MS  650  /* time to wait after receiving an initial `ESC` */
#define VKEY_SEQ_WAIT_TIME_MS  10   /* time to wait for the next character of the escape sequence */

/* `VKEY_ESC_WAIT_TIME_MS` >= `VKEY_SEQ_WAIT_TIME_MS` is expected */

static struct timeval vio_to = {60, 0};
static struct timeval esc_tv = {0, (long)(VKEY_ESC_WAIT_TIME_MS * 1000)};
static struct timeval seq_tv = {0, (long)(VKEY_SEQ_WAIT_TIME_MS * 1000)};


void
add_io(
    int fd,
    int timeout)
{
    vio_fd = fd;
    vio_to.tv_sec = timeout;

    if (!fd)
    {
        /* Reinitialize them here so that `vkey` and `igetch` do not need to reinitialize them every time */
        esc_tv = LISTLIT(struct timeval){0, (long)(VKEY_ESC_WAIT_TIME_MS * 1000)};
        seq_tv = LISTLIT(struct timeval){0, (long)(VKEY_SEQ_WAIT_TIME_MS * 1000)};
        if (vio_to.tv_sec <= esc_tv.tv_sec && vio_to.tv_usec <= esc_tv.tv_usec)
        {
            esc_tv = vio_to;
            if (vio_to.tv_sec <= seq_tv.tv_sec && vio_to.tv_usec <= seq_tv.tv_usec)
                seq_tv = vio_to;
        }
    }
}


/* `iac_process()`: Returns the key to be handled
    A boundary-checking version of `iac_count()`.
*/
int
iac_process(
    const unsigned char *current,
    const unsigned char *end,
    int *pcount)
{
    const unsigned char *look = current;
    int res_key = KEY_NONE;

    if (look >= end || *look != IAC || ++look >= end)
        goto iac_process_end;

    switch (*look++)
    {
    case DO:
    case DONT:
    case WILL:
    case WONT:
        look++;  /* Ignore option */
        goto iac_process_end;

    case SB:                    /* loop forever looking for the SE */
        {
            if (look >= end)
                goto iac_process_end;

            /* fuse.030518: 線上調整畫面大小，重抓 b_lines */
            if ((*look++) == TELOPT_NAWS)
            {
                if (look + 3 >= end)  /* Incompleted command */
                {
                    look += 3;
                    goto iac_process_end;
                }
                b_lines = ntohs(* (const short *) (look + 2)) - 1;
                b_cols = ntohs(* (const short *) look) - 1;

                /* b_lines 至少要 23，最多不能超過 T_LINES - 1 */
                b_lines = TCLAMP(b_lines, 23, T_LINES - 1);
                /* b_cols 至少要 79，最多不能超過 T_COLS - 1 */
                b_cols = TCLAMP(b_cols, 79, T_COLS - 1);
#ifdef M3_USE_PFTERM
                resizeterm(b_lines + 1, b_cols + 1);
#endif
                d_cols = b_cols - 79;
                res_key = I_RESIZETERM;

                look += 4;
            }

            for (;;)
            {
                if (look >= end || (*look++) == IAC)
                {
                    if (look >= end || (*look++) == SE)
                    {
                        goto iac_process_end;
                    }
                }
            }
        }
    case IAC:  /* `IAC` `IAC` => `'\xff'` */
        res_key = '\xff';
        goto iac_process_end;
    default:;
    }

iac_process_end:
    if (pcount)
        *pcount = look - current;
    return res_key;
}


/* `iac_count()`: Returns the count of bytes to skip
    Only use it when backward compatibility is needed.
*/
int
iac_count(
    const unsigned char *current)
{
    int count = 0;
    int iac_key = iac_process(current, vi_pool + vi_size, &count);
    if (iac_key != KEY_NONE && count > 0)
    {
        // A hack to keep the key returned by `iac_process()`
        vi_pool[current - vi_pool + --count] = iac_key;
    }
    return count;
}

static int vi_mode = 0;
static int vi_unget_key = KEY_NONE;

int
igetch(void)
{

#define IM_REPLY        0x02    /* ^R */
#define IM_TALK         0x04

    fd_set rset;
    int cc, fd=0, nfds;
    unsigned char *data;

    data = vi_pool;
    nfds = 0;

    for (;;)
    {
        if (vi_unget_key != KEY_NONE)
        {
            int key = vi_unget_key;
            vi_unget_key = KEY_NONE;
            return key;
        }

        if (vi_size <= vi_head)
        {
            if (nfds == 0)
            {
                refresh();
                fd = (vi_mode & IM_REPLY) ? 0 : vio_fd;
                nfds = fd + 1;
                FD_ZERO(&rset);
            }

            for (;;)
            {
                struct timeval tv = vio_to;
                /* Thor.980806: man page 假設 timeval 是會改變的 */

                FD_SET(0, &rset);
                FD_SET(fd, &rset);
                cc = select(nfds, &rset, NULL, NULL, &tv /*&vio_to*/);
                                      /* Thor.980806: man page 假設 timeval 是會改變的 */

                if (cc > 0)
                {
                    if (fd != 0 && FD_ISSET(fd, &rset))
                        return I_OTHERDATA;

                    cc = recv(0, data, VI_MAX, 0);
                    if (cc > 0)
                    {
                        int iac_key;
                        vi_size = cc;
                        vi_head = 0;
                        iac_key = iac_process(data, data + cc, &vi_head);
                        if (iac_key != KEY_NONE)
                            return iac_key;
                        if (vi_head >= cc)
                            continue;

                        if (cutmp)
                            time32(&cutmp->idle_time);
                        else
                            idle = 0;
#ifdef  HAVE_SHOWNUMMSG
                        if (cutmp)
                            cutmp->num_msg = 0;
#endif
                        break;
                    }
                    if ((cc == 0) || (errno != EINTR))
                        abort_bbs();
                }
                else if (cc == 0)
                {
                    cc = vio_to.tv_sec;
                    if (cc < 60)                /* paging timeout */
                        return I_TIMEOUT;

                    if (cutmp)
                        idle = (time(NULL) - cutmp->idle_time);
                    else
                        idle += cc;
                    vio_to.tv_sec = cc + 60;  /* Thor.980806: 每次 timeout都增加60秒,
                                                              所以片子愈換愈慢, 好懶:p */
                    /* Thor.990201: 註解: 除了talk_rqst, chat之外, 需要在動一動之後
                                          重設 tv_sec為 60秒嗎? (預設值) */

                    if (idle >= 5 * 60 && !cutmp)  /* 登入時 idle 超過 5 分鐘就斷線 */
                    {
                        pmsg2_body("登入逾時，請重新上站");
                        refresh();
                        abort_bbs();
                    }

                    cc = bbsmode;
                    if ( (idle > (cc ? IDLE_TIMEOUT : 4) * 60) && (!cuser.userlevel) )
                    {
                        clear();
                        outs("超過閒置時間！");
                        bell();
                        refresh();
                        abort_bbs();
                    }
                    else if ( (idle > (cc ? (IDLE_TIMEOUT-4) : 4) * 60) && (!cuser.userlevel) )
                    {
                        outz("\x1b[41;5;1;37m警告！你已經閒置過久，系統將在三分後將你踢除！\x1b[m");
                        bell();
                        refresh();
                    }

                    if (cc)
                    {
                        /*if (cc < M_CLASS)
                        {
                            movie();
                            refresh();
                        }*/
                    }
                }
                else
                {
                    if (errno != EINTR)
                        abort_bbs();
                }
            }
        }

        return (unsigned char) data[vi_head++];
    }
}


#define TRAP_ESC  /* `ESC` is invalid (trap) if not at the beginning of the escape sequences */

static inline int
char_opt(int ch, int key, int key_timeout)
{
    switch (ch)
    {
      case I_TIMEOUT:   /* "<sequence_until_ch> END" => `key_timeout` */
        return key_timeout;
      case I_OTHERDATA:
      case I_RESIZETERM:
        if (key_timeout != KEY_INVALID)
        {
            /* Return the interrupted key and put back the interrupting key */
            vi_unget_key = ch;
            return key_timeout;
        }
        return ch; /* Ignore the invalid interrupted key. Just return the interrupting key */
      default:          /* "<sequence_until_ch> `ch`" => `key` */
        if (key == KEY_INVALID && key_timeout != KEY_INVALID)
        {
            /* Return the interrupted key and put back the interrupting key */
            vi_unget_key = ch;
            return key_timeout;
        }
        return key; /* Return the uninterrupted key */
    }
}

#define META_CODE   0x8
#define CTRL_CODE   0x4
#define ALT_CODE    0x2
#define SHIFT_CODE  0x1

static inline int
mod_key(int mod, int key)   /* xterm-style modifier */
{
    if (mod & CTRL_CODE)
        key = Ctrl(key);
    if (mod & META_CODE || mod & ALT_CODE)
        key = Meta(key);
    if (mod & SHIFT_CODE)
        key = Shift(key);
    return key;
}

static inline int
char_mod(int ch)    /* rxvt-style modifier character */
{
    switch (ch)
    {
      case '~':
        return 0;
      case '$':
        return SHIFT_CODE;
      case '^':
        return CTRL_CODE;
      case '@':
        return CTRL_CODE | SHIFT_CODE;
      default:
        return -1;
    }
}

enum VkeyMode {
    VKEYMODE_NORMAL,
    VKEYMODE_CR,      /* "`'\r'` `ch`" */
    VKEYMODE_ESC,     /* "<Esc> `ch`" */
    VKEYMODE_CSI_APP, /* "<Esc> <[O> `ch`" */
    VKEYMODE_CSI_CH1,  /* "<Esc> [ <1-8> `ch`" */
    VKEYMODE_CSI_CH2,  /* "<Esc> [ <1-8> <0-9> `ch`" */
};

GCC_NONNULLS
int vkey_process(int (*fgetch)(void))
{
    const struct timeval vio_to_backup = vio_to;
    struct timeval seq_tv_dec = seq_tv; /* It decreases to prevent infinity escape sequences */
    enum VkeyMode mode = VKEYMODE_NORMAL;
    int mod = 0;
    int ch;
    int last = 0;
    int last2 = 0;

    for (;;)
    {
        ch = fgetch();
        switch (mode)
        {
        case VKEYMODE_NORMAL:
            switch (ch)
            {
            case '\r':
                vio_to = seq_tv_dec;
                mode = VKEYMODE_CR;
                break;
            case KEY_ESC:
                vio_to = esc_tv;
                mode = VKEYMODE_ESC;
                break;
            case 0x7f:
                ch = Ctrl('H');
                goto vkey_end;
            default:
                goto vkey_end;          /* Normal Key */
            }
            break;

        case VKEYMODE_CR:
            switch (ch)
            {
            case '\0': case '\n':
                ch = KEY_ENTER;
                break;
            default:
                ch = char_opt(ch, KEY_INVALID, KEY_ENTER);
            }
            goto vkey_end;

        case VKEYMODE_ESC:   /* "<Esc> `ch`" */ /* Escape sequence */
            vio_to = seq_tv_dec;
            switch (ch)
            {
            case '[': case 'O':     /* "<Esc> <[O>" */
                mode = VKEYMODE_CSI_APP;
                break;
            case KEY_ESC: /* "<Esc> <Esc>" */ /* <Esc> + possible special keys */
                seq_tv_dec.tv_usec >>= 1;  /* Prevent infinity "<Esc>..." */
                mod = META_CODE;       /* Make the key Meta-ed */
                break;
            default: /* "<Esc> <Esc> {`ch`|END}" | "<Esc> {`ch`|END}" */
                ch = char_opt(ch, Meta(ch), mod_key(mod, KEY_ESC));
                goto vkey_end;
            }
            break;

        case VKEYMODE_CSI_APP:   /* "<Esc> <[O> `ch`" */
            switch (ch)
            {
            /* "<Esc> <[O> <A-D>" */ /* Cursor key */
            case 'A': case 'B': case 'C': case 'D':
                ch = mod_key(mod, KEY_UP + (ch - 'A'));
                goto vkey_end;

            /* "<Esc> <[O> <HF>" */ /* Home End (xterm) */
            case 'H':
                ch = mod_key(mod, KEY_HOME);
                goto vkey_end;
            case 'F':
                ch = mod_key(mod, KEY_END);
                goto vkey_end;
            default:;
            }
            if (last == 'O' || mod)   /* "<Esc> O `ch`" | "<Esc> [ 1 ; <2-9> `ch`" | "<Esc> [ 1 ; 1 <0-6> `ch`" */
            {
                switch (ch)
                {
                /* "<Esc> O <PQRS>" */ /* F1 - F4 */
                case 'P': case 'Q': case 'R': case 'S':
                    ch = mod_key(mod, KEY_F1 + (ch - 'P'));
                    goto vkey_end;

                /* "<Esc> O w" */ /* END (PuTTY-rxvt) */
                case 'w':
                    ch = mod_key(mod, KEY_END);
                    goto vkey_end;

                /* "<Esc> O <a-d>" */ /* Ctrl-ed cursor key (rxvt) */
                case 'a': case 'b': case 'c': case 'd':
                    ch = mod_key(mod | CTRL_CODE, KEY_UP + (ch - 'a'));
                    goto vkey_end;

                default:
                    if (last == 'O')  /* "<Esc> O {`ch`|END}" */
                    {
                        ch = char_opt(ch, KEY_INVALID, Meta('O'));
                        goto vkey_end;
                    }
                }
            }
            switch (ch)                  /* "<Esc> [ `ch`" */
            {
            /* "<Esc> [ <GIL>" */ /* PgDn PgUp Ins (SCO) */
            case 'G':
                ch = mod_key(mod, KEY_PGDN);
                goto vkey_end;
            case 'I':
                ch = mod_key(mod, KEY_PGUP);
                goto vkey_end;
            case 'L':
                ch = mod_key(mod, KEY_INS);
                goto vkey_end;

            /* "<Esc> [ Z" */ /* Shift-Tab */
            case 'Z':
                ch = mod_key(mod, KEY_STAB);
                goto vkey_end;

            /* "<Esc> [ <a-d>" */ /* Shift-ed cursor key (rxvt) */
            case 'a': case 'b': case 'c': case 'd':
                ch = mod_key(mod | SHIFT_CODE, KEY_UP + (ch - 'a'));
                goto vkey_end;

            default:
                if (ch >= '1' && ch <= '8')   /* "<Esc> [ <1-8>" */
                    mode = VKEYMODE_CSI_CH1;
                else    /* "<Esc> [ {`ch`|END}" */
                {
                    ch = char_opt(ch, KEY_INVALID, Meta('['));
                    goto vkey_end;
                }
            }
            break;

        case VKEYMODE_CSI_CH1:
        case VKEYMODE_CSI_CH2:
            if (ch == ';')   /* "<Esc> [ <1-8> ;" | "<Esc> [ <1-8> <0-9> ;" */
            {
                int mod_ch = fgetch();      /* "; [END|`ch`]" */
                if (mod_ch < '1' || mod_ch > '9')
                {
                    ch = char_opt(ch, KEY_INVALID, KEY_INVALID);
                    goto vkey_end;
                }
                if (mod_ch == '1')          /* "; <1-9>" */
                {
                    mod_ch = fgetch();      /* "; 1 [END|`ch`]" */
                    if (mod_ch < '0' || mod_ch > '6')
                    {
                        ch = char_opt(ch, KEY_INVALID, KEY_INVALID);
                        goto vkey_end;
                    }
                    mod_ch += 10;           /* "; 1 <0-6>" */
                }
                mod_ch -= '1';   /* Change to bit number */
                mod |= mod_ch;
                if (mode == VKEYMODE_CSI_CH1 && last == '1')  /* "<Esc> [ 1 ; <2-9>" | "<Esc> [ 1 ; 1 <0-6>" */
                {
                    /* Recover state to "<Esc> [ `ch`" */
                    mode = VKEYMODE_CSI_APP;
                    last = last2;
                } /* else "<Esc> [ <1-8> <0-9> ; <2-9>" | "<Esc> [ <1-8> <0-9> ; 1 <0-6>" */
                continue;     /* Get next `ch`; keep current state */
            }
            switch (mode)
            {
            case VKEYMODE_CSI_CH1:    /* "<Esc> [ <1-8> `ch`" */
                                            /* Home Ins Del End PgUp PgDn Home(rxvt) End(rxvt) */
                if (char_mod(ch) != -1)     /* "<Esc> [ <1-8> <~$^@>" */
                {
                    mod |= char_mod(ch);
                    if (last <= '6')        /* "<Esc> [ <1-6> <~$^@>" */
                    {
                        ch = mod_key(mod, KEY_HOME + (last - '1'));
                        goto vkey_end;
                    }
                    switch (last)           /* "<Esc> [ <78> <~$^@>" */ /* Home End (rxvt) */
                    {
                    case '7':
                        ch = mod_key(mod, KEY_HOME);
                        goto vkey_end;
                    case '8':
                        ch = mod_key(mod, KEY_END);
                        goto vkey_end;
                    default:
                        ch = char_opt(ch, KEY_INVALID, KEY_INVALID);
                        goto vkey_end;
                    }
                }
                else if (ch >= '0' && ch <= '9')      /* "<Esc> [ <1-8> <0-9>" */
                    mode = VKEYMODE_CSI_CH2;
                else
                {
                    ch = char_opt(ch, KEY_INVALID, KEY_INVALID);
                    goto vkey_end;
                }
                break;

            case VKEYMODE_CSI_CH2:    /* "<Esc> [ <1-8> <0-9> `ch`" */
                                            /* F1 - F12 */
                if (char_mod(ch) != -1)     /* "<Esc> [ <1-8> <0-9> <~$^@>" */
                {
                    mod |= char_mod(ch);
                    switch (last2)
                    {
                    case '1':               /* "<Esc> [ 1 `last` <~$^@>" */ /* F1 - F8 */
                        ch = mod_key(mod, KEY_F1 + (last - '1') - (last > '6'));
                        goto vkey_end;
                    case '2':               /* "<Esc> [ 2 `last` <~$^@>" */ /* F9 - F12 */
                        ch = mod_key(mod, KEY_F9 + (last - '0') - (last > '2'));
                        goto vkey_end;
                    default:;
                    }
                }
                ch = char_opt(ch, KEY_INVALID, KEY_INVALID);
                goto vkey_end;

            default: /* Impossible to get here */
                goto vkey_end;
            }
        }
        last2 = last;
        last = ch;
    }

vkey_end:
    vio_to = vio_to_backup;
    return ch;
}

/* Ignore automatic key repeats for DBCS characters send by some clients */
GCC_NONNULLS
int vkey_process_no_dbcs_repeat(int (*fgetch)(void))
{
    static int unget_key = KEY_NONE;
    if (unget_key != KEY_NONE)
    {
        const int key = unget_key;
        unget_key = KEY_NONE;
        return key;
    }

    const int key = vkey_process(fgetch);
    switch (key)
    {
    case Ctrl('H'): /* Backspace */
    /* case Ctrl('D'): */ /* KKman 3 handles this as Del */
    /* However KKman users are probably few in 2020 */
    case KEY_DEL:
    case KEY_LEFT:
    case KEY_RIGHT:
        {
            /* Check the upcoming key to see whether the key is repeated */
            const struct timeval vio_to_backup = vio_to;
            vio_to = seq_tv;
            {
                const int key2 = vkey_process(fgetch);
                if (key2 != I_TIMEOUT && key2 != KEY_INVALID && key2 != key)
                    unget_key = key2; /* Not repeated; put it back */
            }
            vio_to = vio_to_backup;
        }
        break;
    default:
        ;
    }
    return key;
}

int
vkey(void)
{
    const int key = vkey_process_no_dbcs_repeat(igetch);

    switch (key)
    {
    case Ctrl('L'):
        vs_redraw();
        break;
    case Ctrl('R'):
    case Meta('R'):
        if ((bbstate & STAT_STARTED)
            && !(bbstate & STAT_LOCK)       /* lkchu.990513: 鎖定時不可回訊 */
            && !(vi_mode & IM_REPLY))
        {
            /*
             * Thor.980307: 想不到什麼好方法, 在^R時禁止talk, 否則會因,
             * 沒有vio_fd, 看不到 I_OTHERDATA 所以在 ctrl-r時talk, 看不到對方打的字
             */
            signal(SIGUSR1, SIG_IGN);

            vi_mode |= IM_REPLY;
            bmw_reply(0);
            vi_mode ^= IM_REPLY;

            /*
             * Thor.980307: 想不到什麼好方法, 在^R時禁止talk, 否則會因,
             * 沒有vio_fd, 看不到 I_OTHERDATA 所以在 ctrl-r時talk, 看不到對方打的字
             */
            signal(SIGUSR1, talk_rqst_signal);

            return KEY_INVALID; /* Consume the key */
        }
        break;
    default:
        ;
    }
    return key;
}


#define MATCH_END       0x8000
/* Thor.990204: 註解: 代表MATCH完結, 要嘛就補足,
                      要嘛就維持原狀, 不秀出可能的值了 */

static void
match_title(void)
{
    move(2, 0);
    clrtobot();
    vs_line("相關資訊一覽表");
}


static int
match_getch(void)
{
    int ch;

    outs("\n★ 列表 (Enter)(Space)(Tab) 繼續 (Q)結束 ? [Q] ");
    ch = vkey();
    if (ch != '\n' && ch != ' ' && ch != '\t')
        return ch;

    move(3, 0);
    clrtobot();
    return -1;
}


/* ----------------------------------------------------- */
/* 選擇 board                                            */
/* ----------------------------------------------------- */


static BRD *xbrd;


BRD *
ask_board(
    char *board,
    unsigned int perm,
    const char *msg)
{
    if (msg)
    {
        move(2, 0);
        outs(msg);
    }

    if (vget(1, 0, "請輸入看板名稱 (按 SPACE 或 TAB 自動搜尋)：",
            board, IDLEN + 1, GET_BRD | perm))
    {
        if (!str_casecmp(board, currboard))
            *board = 0;         /* 跟目前的看板一樣 */
        return xbrd;
    }

    return NULL;
}


/* IID.20200101: Return `-key` for key. */
static int
vget_match(
    char *prefix,
    int len,
    int op)
{
    char fpath[16];
    char *data, *hit=NULL;
    int row, col, match;

    row = 3;
    col = match = 0;

    if (op & GET_BRD)
    {
        unsigned int perm;
        char *bits;
        BRD *head, *tail;

        perm = op & (BRD_R_BIT | BRD_W_BIT);
        bits = brd_bits;
        head = bshm->bcache;
        tail = head + bshm->number;

        do
        {
            if (perm & *bits++)
            {
                data = head->brdname;

                if (str_ncasecmp(prefix, data, len))
                    continue;

                xbrd = head;

                if ((op & MATCH_END) && !data[len])
                {
                    strcpy(prefix, data);
                    return len;
                }

                match++;
                hit = data;

                if (op & MATCH_END)
                    continue;

                if (match == 1)
                    match_title();

                move(row, col);
                outs(data);

                col += IDLEN + 1;
                if (col + IDLEN >= b_cols)
                {
                    col = 0;
                    if (++row >= b_lines)
                    {
                        int ch = match_getch();
                        if (ch >= 0)
                            return -ch;
                        row = 3;
                    }
                }
            }
        } while (++head < tail);
    }
    else if (op & GET_USER)
    {
        int cc;
        int cd;

        /* Thor.981203: USER name至少打一字, 用"<="會比較好嗎? */
//      if (len == 0)
//          return 0;
        if (len)
        {
            cc = *prefix;
            if (cc >= 'A' && cc <= 'Z')
                cc |= 0x20;
            if (cc < 'a' || cc > 'z')
                return 0;
            cd = cc;
        }
        else
        {
            if (!HAS_PERM(PERM_ADMIN))  /* 一般使用者要輸入一字才比對 */
                return 0;
            cc = 'a';
            cd = 'z';
        }

        for (; cc <= cd; cc++)
        {
            struct dirent *de;
            DIR *dirp;
            sprintf(fpath, "usr/%c", cc);
            dirp = opendir(fpath);
            while ((de = readdir(dirp)))
            {
                data = de->d_name;
                if (*data <= ' ' || *data == '.')
                    continue;

//              if (str_ncasecmp(prefix, data, len))
                if (len && str_ncasecmp(prefix, data, len))
                    continue;

                match++;
                if (match == 1)
                    strcpy(hit = fpath, data);  /* 第一筆符合的資料 */

                if (op & MATCH_END)
                    continue;

                if (match == 1)
                    match_title();

                move(row, col);
                outs(data);

                col += IDLEN + 1;
                if (col + IDLEN >= b_cols)
                {
                    col = 0;
                    if (++row >= b_lines)
                    {
                        int ch = match_getch();
                        if (ch >= 0)
                            return -ch;
                        row = 3;
                    }
                }
            }
            closedir(dirp);
        }
    }
    else /* Thor.990203: 註解, GET_LIST */
    {
        LinkList *list;

        for (list = ll_head; list; list = list->next)
        {
            data = list->data;

            if (str_ncasecmp(prefix, data, len))
                continue;

            if ((op & MATCH_END) && !data[len])
            {
                strcpy(prefix, data);
                return len;
            }

            match++;
            hit = data;

            if (op & MATCH_END)
                continue;

            if (match == 1)
                match_title();

            move(row, col);
            outs(data);

            col += IDLEN + 1;
            if (col + IDLEN >= b_cols)
            {
                col = 0;
                if (++row >= b_lines)
                {
                    int ch = match_getch();
                    if (ch >= 0)
                        return -ch;
                    row = 3;
                }
            }
        }
    }

    if (match == 1)
    {
        strcpy(prefix, hit);
        return strlen(hit);
    }

    return 0;
}


char lastcmd[MAXLASTCMD][80];


/* IID.20200202: Use screen size referencing coordinate */
int vget(int y_ref, int x_ref, const char *prompt, char *data, int max, int echo)
{
    int ch = KEY_NONE;
    int len;
    int line, col;
    int x, y, x_prompt;
    bool dirty, key_done;

    /* Adjust flags */
    if (echo & VGET_FORCE_DOECHO)
        echo &= ~HIDEECHO;
    if (!(echo & HIDEECHO))
        echo &= ~VGET_STEALTH_NOECHO;

    max--;

vget_redraw:
    move_ref(y_ref, x_ref);

    getyx(&SINKVAL(int), &x_prompt);

    clrtoeol();
    if (prompt)
        outs(prompt);

    if (!(echo & VGET_STEALTH_NOECHO))
        STANDOUT;

    getyx(&y, &x);

    if (echo & GCARRY)
    {
        if (ch != I_RESIZETERM)
        {
            len = strlen(data);
            if (len && (echo & NUMECHO))
            {
                /* Remove non-digit characters */
                int col = 0;
                for (int ch = 0; ch < len; ch++)
                    if (isdigit(data[ch]))
                        data[col++] = data[ch];
                data[col] = '\0';
                len = col;
            }
        }
        if (len && !(echo & VGET_STEALTH_NOECHO))
        {
            if (echo & HIDEECHO)
            {
                for (int ch = 0; ch < len; ch++)
                    outc('*');
            }
            else
            {
                outs(data);
            }
        }
    }
    else
    {
        len = 0;
        echo |= GCARRY;  /* For redrawing */
    }

    if (!(echo & VGET_STEALTH_NOECHO))
    {
        int ch = len;
        do
        {
            outc(' ');
        } while (++ch < max+1);

        STANDEND;
    }

    if (ch != I_RESIZETERM)
    {
        dirty = len > 2;  /* Store default string */
        line = MAXLASTCMD - 1;  /* Obsolete the oldest entry */
        col = len;
    }

    for (;;)
    {
        if (!(echo & VGET_STEALTH_NOECHO))
            move(y, x + col);

        ch = vkey();
        if (ch == I_RESIZETERM)
        {
            /* Screen size changed and redraw is needed */
            /* clear */
            move(y, x_prompt);
            clrtoeol();
            /* redraw */
            data[len] = '\0';
            goto vget_redraw;
        }

        /* --------------------------------------------------- */
        /* 取得 board / userid / on-line user                  */
        /* --------------------------------------------------- */

        if (ch == '\n')
        {
            data[len] = '\0';

            /* IID.2020-11-17: Make sure the input field is truncated at the input end */
            if (!(echo & VGET_STEALTH_NOECHO))
                move(y, x + len);

            if ((echo & (GET_BRD | GET_LIST)) && len > 0)
            /* Thor.990204:要求輸入任一字才代表自動 match, 否則算cancel */
            {
                int len_prev = len;
                len = vget_match(data, len, echo | MATCH_END);
#ifdef M3_USE_PFTERM
                STANDOUT;
#endif
                if (len > len_prev)
                {
                    move(y, x);
                    outs(data);
                }
                else if (len == 0)
                {
                    data[0] = '\0';
                }
#ifdef M3_USE_PFTERM
                STANDEND;
#endif
            }
            break;
        }

        if ((ch == ' ' || ch == '\t') && (echo & (GET_USER | GET_BRD | GET_LIST)))
        {
            int len_match = vget_match(data, len, echo);
#ifdef M3_USE_PFTERM
            STANDOUT;
#endif
            if (len_match > len)
            {
                move(y, x);
                outs(data);
                col = len = len_match;
                dirty = true;
            }
#ifdef M3_USE_PFTERM
            STANDEND;
#endif
            if (len_match >= 0)
                continue;
            else
                ch = -len_match;
        }

        if (ch == Ctrl('C') && (echo & VGET_BREAKABLE))
        {
            data[0] = '\0';

            /* IID.2020-11-17: Make sure the input field is truncated at the input end */
            if (!(echo & VGET_STEALTH_NOECHO))
                move(y, x + len);

            outc('\n');
#ifdef M3_USE_PFTERM
            if (!(echo & VGET_STEALTH_NOECHO))
                STANDEND;
#endif
            return VGET_EXIT_BREAK;
        }

        if (isprint2(ch))
        {
            if (!isdigit(ch) && (echo & NUMECHO))
            {
                bell();
                continue;
            }

            if (len >= max)
            {
                bell();
                continue;
            }

            /* ----------------------------------------------- */
            /* insert data and display it                      */
            /* ----------------------------------------------- */

            if (!(echo & VGET_STEALTH_NOECHO))
            {
                move(y, x + col);
#ifdef M3_USE_PFTERM
                STANDOUT;
#endif
            }

            for (int i = col; i <= len; i++)
            {
                int next = (unsigned char)data[i];

                if (!(echo & VGET_STEALTH_NOECHO))
                    outc((echo & HIDEECHO) ? '*' : ch);

                data[i] = ch;
                ch = next;
            }
#ifdef M3_USE_PFTERM
            if (!(echo & VGET_STEALTH_NOECHO))
                STANDEND;
#endif
            col++;
            len++;
            dirty = true;
            continue;
        }

        /* ----------------------------------------------- */
        /* 輸入 password / match-list 時只能按 BackSpace   */
        /* ----------------------------------------------- */

#if 0
        if (((echo & HIDEECHO) || mfunc) && ch != Ctrl('H'))
        {
            bell();
            continue;
        }
#endif

        if ((echo & HIDEECHO) && ch != Ctrl('H'))
        {
            bell();
            continue;
        }

        key_done = true;  /* Assume key processing will be done */
#ifdef M3_USE_PFTERM
        if (!(echo & VGET_STEALTH_NOECHO))
            STANDOUT;
#endif
        switch (ch)
        {
        case Ctrl('H'):
            if (!col)
            {
                bell();
                break;
            }

            col--;
            if (col && IS_DBCS_TRAIL_N(data, col, len))
                col--;

            /* `col >= len` should hold; skip the check */
            if (0)
            {
                // Falls through

        case KEY_DEL:
        case Ctrl('D'):
                if (col >= len)
                {
                    bell();
                    break;
                }
            }

            /* ----------------------------------------------- */
            /* remove data and display it                      */
            /* ----------------------------------------------- */

            {
                const int del_len = (col + 1 < len && IS_DBCS_LEAD_N(data, col, len)) ? 2 : 1;
                len -= del_len;
                dirty = true;

                if (!(echo & VGET_STEALTH_NOECHO))
                    move(y, x + col);

                for (int i = col; i < len; i++)
                {
                    data[i] = ch = (unsigned char) data[i + del_len];

                    if (!(echo & VGET_STEALTH_NOECHO))
                        outc((echo & NOECHO) ? '*' : ch);
                }
                for (int i = 0; i < del_len; i++)
                {
                    data[len + i] = '\0';
                    if (!(echo & VGET_STEALTH_NOECHO))
                        outc(' ');
                }
            }

            break;

        case KEY_LEFT:
        case Ctrl('B'):
            if (col)
            {
                --col;
                if (col && IS_DBCS_TRAIL_N(data, col, len))
                    --col;
            }
            else
                bell();
            break;

        case KEY_RIGHT:
        case Ctrl('F'):
            if (col + 1 < len && IS_DBCS_LEAD_N(data, col, len))
                col += 2;
            else if (col < len)
                ++col;
            else
                bell();
            break;

        case KEY_HOME:
        case Ctrl('A'):
            col = 0;
            break;

        case KEY_END:
        case Ctrl('E'):
            col = len;
            break;

        case Ctrl('Y'):         /* clear / reset */
            if (len)
            {
                move(y, x);
                for (int ch = 0; ch < len; ch++)
                    outc(' ');
                col = len = 0;
                dirty = true;
            }
            break;

        case Ctrl('K'):         /* delete to end of line */
            if (col < len)
            {
                move(y, x + col);
                for (int ch = col; ch < len; ch++)
                    outc(' ');
                len = col;
                dirty = true;
            }
            break;

        default:
            key_done = false; /* Non-processed key */
            break;
        }
#ifdef M3_USE_PFTERM
        if (!(echo & VGET_STEALTH_NOECHO))
            STANDEND;
#endif

        /* No further processing is needed */
        if (key_done)
            continue;

        /* No input history for `NUMECHO` or hidden inputs */
        if ((echo & (HIDEECHO | NUMECHO)))
        {
            bell();
            continue;
        }

#ifdef M3_USE_PFTERM
        STANDOUT;
#endif

        /* Seek history */
        {
            int line_prev = line;
            switch (ch)
            {
            case KEY_DOWN:
            case Ctrl('N'):

                /* IID.20200106: Prevent jumping to the oldest entry. */
                if (line == MAXLASTCMD - 1)
                    break;
                line = line - 2 + MAXLASTCMD;
                // Falls through

            case KEY_UP:
            case Ctrl('P'):

                {
                    /* IID.20200123: Prevent getting into empty entries. */
                    int line_next = (line + 1) % MAXLASTCMD;
                    if (line_next < MAXLASTCMD - 1 && !*lastcmd[line_next])
                        break;
                    line = line_next;
                }
                if (dirty && len > 2)
                {
                    str_sncpy(lastcmd[line_prev], data, sizeof(lastcmd[line_prev]), len);  /* Save changes */
                    dirty = 0;
                }
                move(y, x);

                for (col = 0; col < max; ++col)
                {
                    int ch = (unsigned char) lastcmd[line][col];
                    if (!ch)
                    {
                        /* clrtoeol */

                        for (int ch = col; ch < len; ch++)
                            outc(' ');
                        break;
                    }

                    outc(ch);
                    data[col] = ch;
                }

                len = col;
                break;

            default:
                /* Invalid key */
                bell();
                break;
            }
        }
#ifdef M3_USE_PFTERM
        STANDEND;
#endif
    }

    if (len > 2 && !(echo & (HIDEECHO | NUMECHO)))
    {
        /* Move current history entry to the front */
        for (int ln = line; ln; ln--)
            strcpy(lastcmd[ln], lastcmd[ln - 1]);
        strcpy(lastcmd[0], data);
    }

    outc('\n');

    if (echo & LCECHO)
        str_lower_dbcs(data, data);
    ch = (unsigned char) data[0];

#ifdef M3_USE_PFTERM
    if (!(echo & VGET_STEALTH_NOECHO))
        STANDEND;
#endif

    return ch;
}


int
vans(
    const char *prompt)
{
    char ans[3];

    return vget(B_LINES_REF, 0, prompt, ans, sizeof(ans), LCECHO);
}
