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
int cur_row, cur_col;
int cur_pos;                    /* current position with ANSI codes */

/* ----------------------------------------------------- */
/* output routines                                       */
/* ----------------------------------------------------- */


static char vo_pool[VO_MAX];
static int vo_size;


#ifdef  VERBOSE
static void
telnet_flush(
    char *data,
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
    char *str,
    int len)
{
    int size, ch;
    char *data;

    size = vo_size;
    data = vo_pool;
    if (size + len > VO_MAX - 8)
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
    if (size > VO_MAX - 2)
    {
        telnet_flush(data, size);
        size = 0;
    }
    data[size++] = ch;
    vo_size = size;
}


void
bell(void)
{
    static char sound[1] = {Ctrl('G')};

    telnet_flush(sound, sizeof(sound));
}


/* ----------------------------------------------------- */
/* virtual screen                                        */
/* ----------------------------------------------------- */

#ifndef M3_USE_PFTERM
#define o_ansi(x)       output(x, sizeof(x)-1)

#define o_clear()       o_ansi("\x1b[;H\x1b[2J")
#define o_cleol()       o_ansi("\x1b[K")
#define o_standup()     o_ansi("\x1b[7m")
#define o_standdown()   o_ansi("\x1b[m")


static int docls;
static int roll;
static int scrollcnt, tc_col, tc_row;


static screenline vbuf[100];  //r2: maximum totel lines (t_lines)
//static
screenline *cur_slp;    /* current screen line pointer */


void
move(
    int y,
    int x)
{
    screenline *cslp;

    if (y > b_lines)
        return;

    if (x >= t_columns)
        x = 0;

    cur_row = y;
    if ((y += roll) >= t_lines)
        y -= t_lines;
    cur_slp = cslp = &vbuf[y];
    cur_col = x;

    /* ------------------------------------- */
    /* �L�o ANSI codes�A�p���Яu���Ҧb��m */
    /* ------------------------------------- */

#if 0
    if (x && (cslp->mode & SL_ANSICODE))
    {
        int ch, ansi;
        unsigned char *str;

        ansi = 0;
        y = x;
        str = cslp->data;
        while (x && (ch = *str))
        {
            str++;
            if (ch == KEY_ESC)
            {
                ansi = YEA;
                y++;
                continue;
            }
            if (ansi)
            {
                y++;
                if (ch == 'm')          /* (!strchr(str_ansicode, ch)) */
                {
                    ansi = NA;
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

/* verit : 030212, ���� ansi code */
void
move_ansi(
    int y,
    int x)
{
    screenline *cslp;

    if (y > b_lines)
        return;

    if (x >= t_columns)
        x = 0;

    cur_row = y;
    if ((y += roll) >= t_lines)
        y -= t_lines;
    cur_slp = cslp = &vbuf[y];
    cur_col = x;

    if (x)
    {
        int ch, ansi;
        int len;
        unsigned char *str;

        ansi = 0;
        y = x;
        str = cslp->data;
        str[(len = cslp->len)] = '\0';
        while (len && (ch = *str))
        {
            str++;
            len--;
            if (ch == KEY_ESC)
            {
                ansi = YEA;
                y++;
                continue;
            }
            if (ansi)
            {
                y++;
                if (ch == 'm')          /* (!strchr(str_ansicode, ch)) */
                {
                    ansi = NA;
                }
                continue;
            }
            x--;
            if (x<=0 && (ansi==NA))
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
/* �p�� slp �� len ���B����� column �Ҧb                */
/*-------------------------------------------------------*/


#if 0
static int
ansicol(
    screenline *slp,
    int len)
{
    unsigned char *str;
    int ch, ansi, col;

    if (!len || !(slp->mode & SL_ANSICODE))
        return len;

    ansi = col = 0;
    str = slp->data;

    while (len-- && (ch = *str++))
    {
        if (ch == KEY_ESC && *str == '[')
        {
            ansi = YEA;
            continue;
        }
        if (ansi)
        {
            if (ch == 'm')
                ansi = NA;
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

    if (new_row >= t_lines || new_col >= t_columns)
        return;

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
    screenline *slp,
    int ds, int de)
{
    char *data;
    int sso, eso;

    data = (char *) slp->data;
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


#define STANDOUT        cur_slp->sso = cur_pos; cur_slp->mode |= SL_STANDOUT;
#define STANDEND        cur_slp->eso = cur_pos;


#if 0
static int standing;


static void
standout(void)
{
    if (!standing)
    {
        standing = YEA;
        cur_slp->sso = cur_slp->eso = cur_pos;
        cur_slp->mode |= SL_STANDOUT;
    }
}


static void
standend(void)
{
    if (standing)
    {
        standing = NA;
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

    tc_col = tc_row = docls = scrollcnt = vo_size = i = 0;
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

    if ((docls) || ((i < 0 ? -i : i) >= p_lines))
    {
        vs_redraw();
        return;
    }

    if (i)
    {
        char buf[p_lines];

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
                emod = slp->emod + 1;
                if (emod >= len)
                    emod = len;

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

    docls = YEA;
    cur_pos = cur_col = cur_row = roll = i = 0;
    cur_slp = slp = vbuf;
    while (i++ < t_lines)
    {
        memset(slp++, 0, 9);
    }
}

void
clearange(
    int from, int to)
{
    int i;
    screenline *slp;

    docls = YEA;
    cur_pos = cur_col = roll = 0;
    i = cur_row = from;
    cur_slp = slp = &vbuf[from];
    while (i++ < to)
    {
        memset(slp++, 0, 9);
    }
}

void
clrtoeol(void)
{
    screenline *slp = cur_slp;
    int len;

    if ((len = cur_pos))
    {
        if ( len > slp->len)
            for ( len = slp->len; len < cur_pos; len++)
                slp->data[len] = ' ';
        slp->len = len;
        slp->width = cur_col;
    }
    else
    {
        memset((char *) slp + 1, 0, 8);
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
        memset((char *) slp + 1, 0, 8);

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
                standing = NA;
                if (pos <= slp->sso)
                    slp->mode &= ~SL_STANDOUT;
                else if (slp->eso < pos)
                    slp->eso = pos;
            }
#endif
        }
        else
        {
            memset((char *) slp + 1, 0, 8);
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

    data = &(slp->data[pos]);   /* ���V�ثe��X��m */

    /* -------------------- */
    /* �ɨ��һݭn���ťզr�� */
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
    /* ANSI control code ���S�O�B�z */
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
                if (ch < ANSILINELEN - 1)
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
    /* �P�w���Ǥ�r�ݭn���s�e�X�ù� */
    /* ---------------------------- */

    if ( /* !(slp->mode & SL_ANSICODE) && */ (ch != cx))
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

    if ((pos >= ANSILINELEN) /* || (cx >= t_columns) */ )
        goto new_line;

    if (slp->width < cx)
        slp->width = cx;
}


void
outs(
    char *str)
{
    int ch;

    while ((ch = (unsigned char) *str))
    {
        outc(ch);
        str++;
    }
}

#endif // #ifdef M3_USE_PFTERM

/* ----------------------------------------------------- */
/* eXtended output: �q�X user �� name �M nick            */
/* ----------------------------------------------------- */

/* 090924.cache: pmore�ϥΪ�����X */

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

    switch(*++src)
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
    char *str)
{
/*
    unsigned char *t_name = cuser.userid;
    unsigned char *t_nick = cuser.username;
*/

    time_t now;
    int ch;

/* cache.090922: ����X */

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
                    case 's':       /* **s ��� ID */
                        outs(cuser.userid);
                        str += 3;
                        break;
                    case 'n':       /* **n ��ܼʺ� */
                        outs(cuser.username);
                        str += 3;
                        break;
                    case 't':       /* **t ��ܤ�� */
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
outz(
    char *msg)
//  const char *msg)
{
    int ch;

    move(b_lines, 0);
    clrtoeol();
    while ((ch = (unsigned char) *msg))
    {
        outc(ch);
        msg++;
    }
}

void
outf(
    char *str)
{
    outz(str);
    prints("%*s\x1b[m", d_cols, "");
}

#ifdef M3_USE_PFTERM

void outl (int line, char *msg)   /* line output */
{
    move (line, 0);
    clrtoeol();


    if (msg != NULL)
        outs(msg);
}

#define ANSI_COLOR_CODE            "[m;0123456789"
#define ANSI_COLOR_END     "m"

void outr (char *str)
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

void
prints(char *fmt, ...)
{
    va_list args;
    char buf[512], *str;
//  char buf[512], *str;
    int cc;

    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);
    for (str = buf; (cc = (unsigned char) *str); str++)
        outc(cc);
}

#ifdef M3_USE_PFTERM
static int save_y, save_x;
#else
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
static int old_col, old_row, old_roll;
static int old_pos; /* Thor.990401: �h�s�@�� */


/* static void */
void                            /* Thor.1028: ���F�� talk.c
                                 * ���H�I�s�ɷ|show�r */
cursor_save(void)
{
    old_col = cur_col;
    old_row = cur_row;

    old_pos = cur_pos; /* Thor.990401: �h�s�@�� */
}


/* static void */
void                            /* Thor.1028: ���F�� talk.c
                                 * ���H�I�s�ɷ|show�r */
cursor_restore(void)
{
    move(old_row, old_col);

    cur_pos = old_pos; /* Thor.990401: �h�٭�@�� */
}


void
save_foot(
    screenline *slp)
{
#if 0
    cursor_save();
    /* Thor.980827: �Ҧ��Ψ� save_foot���ɾ����|���s�w��, �G���Φs��Ц�m */
#endif

    move(b_lines - 1, 0);
    memcpy(slp, cur_slp, sizeof(screenline) * 2);
    slp[0].smod = 0;
    slp[0].emod = 255; /* Thor.990125:���׳̫�@������, �����nø�W */
    slp[0].oldlen = 255;
    slp[0].mode |= SL_MODIFIED;
    slp[1].smod = 0;
    slp[1].emod = 255; /* Thor.990125:���׳̫�@������, �����nø�W */
    slp[1].oldlen = 255;
    slp[1].mode |= SL_MODIFIED;
}


void
restore_foot(
    screenline *slp)
{
    move(b_lines - 1, 0);
    memcpy(cur_slp, slp, sizeof(screenline) * 2);
#if 0
    cursor_restore();
    /* Thor.980827: �Ҧ��Ψ� restore_foot���ɾ����|���s�w��, �G���Φs��Ц�m */
#endif
    refresh(); /* Thor.981222: ������screen��ø, �ĥ� refresh */
    /* vs_redraw();*/  /* lkchu.981201: �� refresh() �|�� b_lines - 1 ��M�� */
}


int
vs_save(
    screenline *slp)
{
#if 0
    cursor_save();
    /* Thor.980827: �Ҧ��Ψ� vs_save���ɾ����|���s�w��, �G���Φs��Ц�m */
#endif
    old_roll = roll;
    memcpy(slp, vbuf, sizeof(screenline) * t_lines);
    return old_roll;    /* itoc.030723: �Ǧ^�ثe�� roll */
}


void
vs_restore(
    screenline *slp)
{
    memcpy(vbuf, slp, sizeof(screenline) * t_lines);
#if 0
    cursor_restore();
    /* Thor.980827: �Ҧ��Ψ� vs_restore���ɾ����|���s�w��, �G���Φs��Ц�m */
#endif
    roll = old_roll;
    vs_redraw();
}

#endif  // M3_USE_PFTERM

#define VMSG_NULL "\x1b[1;37;45m%*s�� �Ы����N���~�� ��%*s\x1b[m"

int
vmsg(
    char *msg)                   /* length <= 54 */
{
    move(b_lines, 0);
    clrtoeol();
    if (msg)
    {
        prints(COLOR1 " �� %-*s " COLOR2 " [�Ы����N���~��] \x1b[m", d_cols + 55, msg);
    }
    else
    {
#ifdef HAVE_COLOR_VMSG
        int color;
        color =time(0)%6+31;
        prints("\x1b[1;%dm%*s�j�k�l�m�n�o�p \x1b[1;37m�Ы����N���~�� \x1b[1;%dm�p\x1b[m ", color, d_cols + 45, "", color);
#else
        outs(VMSG_NULL, (d_cols >> 1) + 30, "", (d_cols+1 >> 1) + 27, "");
#endif
#ifdef M3_USE_PFTERM
        move(b_lines, 0);
#endif
    }
    return vkey();
}

static inline void
zkey(void)                              /* press any key or timeout */
{
    /* static */ struct timeval tv = {1, 100};
    /* Thor.980806: man page ���] timeval struct�O�|���ܪ� */

    int rset;

    rset = 1;
    select(1, (fd_set *) &rset, NULL, NULL, &tv);

#if 0
    if (select(1, &rset, NULL, NULL, &tv) > 0)
    {
        recv(0, &rset, sizeof(&rset), 0);
    }
#endif
}


void
zmsg(                   /* easy message */
    char *msg)
{
#if 0
    move(b_lines, 0);
    clrtoeol();
    outs(msg);
#endif
    outz(msg); /* Thor.980826: �N�O outz�� */

    refresh();
    zkey();
}


void
vs_bar(
    char *title)
{
#ifdef  COLOR_HEADER
/*  int color = (time(0) % 7) + 41;        lkchu.981201: random color */
    int color = 44; //090911.cache: �Ӫ�F�T�w�@���C��
#endif

    clear();
#ifdef  COLOR_HEADER
    prints("\x1b[1;%2d;37m�i%s�j\x1b[m\n", color, title);
#else
    prints("\x1b[1;46;37m�i%s�j\x1b[m\n", title);
#endif
}


#if 0
void
cursor_show(
    int row, int column)
{
    move(row, column);
    outs(STR_CURSOR);
    move(row, column + 1);
}


void
cursor_clear(
    int row, int column)
{
    move(row, column);
    outs(STR_UNCUR);
}


int
cursor_key(
    int row, int column)
{
    int ch;

    cursor_show(row, column);
    ch = vkey();
    move(row, column);
    outs(STR_UNCUR);
    return ch;
}
#endif  /* #if 0 */

static void
vs_line(
    char *msg)
{
    int head, tail;

    if (msg)
        head = (strlen(msg) + 1) >> 1;
    else
        head = 0;

    tail = head;

    while (head++ < 38)
        outc('-');

    if (tail)
    {
        outc(' ');
        outs(msg);
        outc(' ');
    }

    while (tail++ < 38)
        outc('-');
    outc('\n');
}

#ifndef M3_USE_PFTERM
/* 090127.cache �H�J�H�X�S�� */
void
grayout(int y, int end, int level)
// GRAYOUT_DARK(0): dark, GRAYOUT_BOLD(1): bold, GRAYOUR_NORMAL(2): normal
{
    screenline slp[T_LINES], newslp[T_LINES];
    char *prefix[3] = { "\x1b[1;30m", "\x1b[1;37m", "\x1b[0;37m" };
    char buf[ANSILINELEN];
    register int i;

    vs_save(slp);
    memcpy(newslp, slp, sizeof(newslp));

    if (y < 0)
        y = 0;
    if (end > T_LINES)
        end = T_LINES;

    for (i = y; i < end; i++)
    {
        if (!newslp[i].width)
            continue;

        newslp[i].oldlen = newslp[i].len;
        newslp[i].len = newslp[i].width + 7 + 3;

        str_ansi(buf, (char *) slp[i].data, slp[i].width + 1);
        sprintf((char *) newslp[i].data, "%s%s\x1b[m", prefix[level], buf);
    }
    vs_restore(newslp);
}
#endif //#ifndef M3_USE_PFTERM

/* ----------------------------------------------------- */
/* input routines                                        */
/* ----------------------------------------------------- */

static unsigned char vi_pool[VI_MAX];
static int vi_size;
static int vi_head;


/* static int vio_fd; */
int vio_fd;                     /* Thor.0725: ���H��btalk & chat �i ^z �@�ǳ� */

#ifdef EVERY_Z
int holdon_fd;                  /* Thor.0727: ���Xchat&talk�Ȧsvio_fd�� */
#endif


static struct timeval vio_to = {60, 0};


void
add_io(
    int fd,
    int timeout)
{
    vio_fd = fd;
    vio_to.tv_sec = timeout;
}


static inline int
iac_count(
    unsigned char *current)
{
    switch (*(current + 1))
    {
    case DO:
    case DONT:
    case WILL:
    case WONT:
        return 3;

    case SB:                    /* loop forever looking for the SE */
        {
            unsigned char *look = current + 2;

            /* fuse.030518: �u�W�վ�e���j�p�A���� b_lines */
            if ((*look) == TELOPT_NAWS)
            {
                b_lines = ntohs(* (short *) (look + 3)) - 1;
                b_cols = ntohs(* (short *) (look + 1)) - 1;

                /* b_lines �ܤ֭n 23�A�̦h����W�L T_LINES - 1 */
                if (b_lines >= T_LINES)
                    b_lines = T_LINES - 1;
                else if (b_lines < 23)
                    b_lines = 23;
                /* b_cols �ܤ֭n 79�A�̦h����W�L T_COLS - 1 */
                if (b_cols >= T_COLS)
                    b_cols = T_COLS - 1;
                else if (b_cols < 79)
                    b_cols = 79;
#ifdef M3_USE_PFTERM
                resizeterm(b_lines + 1, b_cols + 1);
#endif
                d_cols = b_cols - 79;
            }

            for (;;)
            {
                if ((*look++) == IAC)
                {
                    if ((*look++) == SE)
                    {
                        return look - current;
                    }
                }
            }
        }
    }
    return 1;
}


int
igetch(void)
{

#define IM_TRAIL        0x01
#define IM_REPLY        0x02    /* ^R */
#define IM_TALK         0x04

    static int imode = 0;
    static int idle = 0;

    int cc, fd=0, nfds, rset;
    unsigned char *data;

    data = vi_pool;
    nfds = 0;

    for (;;)
    {
        if (vi_size <= vi_head)
        {
            if (nfds == 0)
            {
                refresh();
                fd = (imode & IM_REPLY) ? 0 : vio_fd;
                nfds = fd + 1;
                if (fd)
                    fd = 1 << fd;
            }

            for (;;)
            {
                struct timeval tv = vio_to;
                /* Thor.980806: man page ���] timeval �O�|���ܪ� */

                rset = 1 | fd;
                cc = select(nfds, (fd_set *) & rset, NULL, NULL, &tv /*&vio_to*/);
                                      /* Thor.980806: man page ���] timeval �O�|���ܪ� */

                if (cc > 0)
                {
                    if (fd & rset)
                        return I_OTHERDATA;

                    cc = recv(0, data, VI_MAX, 0);
                    if (cc > 0)
                    {
                        vi_head = (*data) == IAC ? iac_count(data) : 0;
                        if (vi_head >= cc)
                            continue;
                        vi_size = cc;

                        if (idle && cutmp)
                        {

                            cutmp->idle_time = idle = 0;
                        }
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

                    idle += cc / 60;
                    vio_to.tv_sec = cc + 60;  /* Thor.980806: �C�� timeout���W�[60��,
                                                              �ҥH���l�U���U�C, �n�i:p */
                    /* Thor.990201: ����: ���Ftalk_rqst, chat���~, �ݭn�b�ʤ@�ʤ���
                                          ���] tv_sec�� 60���? (�w�]��) */

                    if (idle >= 5 && !cutmp)  /* �n�J�� idle �W�L 5 �����N�_�u */
                    {
                        pmsg2("�n�J�O�ɡA�Э��s�W��");
                        refresh();
                        abort_bbs();
                    }

                    cc = bbsmode;
                    if ( (idle > (cc ? IDLE_TIMEOUT : 4) ) && ( strcmp(cuser.userid, STR_GUEST ) == 0 ) )
                    {
                        clear();
                        outs("�W�L���m�ɶ��I");
                        bell();
                        refresh();
                        abort_bbs();
                    }
                    else if ( (idle > (cc ? (IDLE_TIMEOUT-4) : 4) ) && ( strcmp(cuser.userid, STR_GUEST ) == 0 ) )
                    {
                        outz("\x1b[41;5;1;37mĵ�i�I�A�w�g���m�L�[�A�t�αN�b�T����N�A�𰣡I\x1b[m");
                        bell();
                        refresh();
                    }

                    if (cc)
                    {
                        cutmp->idle_time = idle;
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

        cc = (unsigned char) data[vi_head++];
        if (imode & IM_TRAIL)
        {
            imode ^= IM_TRAIL;
            if (cc == 0 || cc == 0x0a)
                continue;
        }

        if (cc == 0x0d)
        {
            imode |= IM_TRAIL;
            return '\n';
        }

        if (cc == 0x7f)
        {
            return Ctrl('H');
        }

        if (cc == Ctrl('L'))
        {
            vs_redraw();
            continue;
        }

        if ((cc == Ctrl('R')) && (bbstate & STAT_STARTED) && !(bbstate & STAT_LOCK)
                && !(imode & IM_REPLY))         /* lkchu.990513: ��w�ɤ��i�^�T */
        {
            /*
             * Thor.980307: �Q���줰��n��k, �b^R�ɸT��talk, �_�h�|�],
             * �S��vio_fd, �ݤ��� I_OTHERDATA �ҥH�b ctrl-r��talk, �ݤ����襴���r
             */
            signal(SIGUSR1, SIG_IGN);

            imode |= IM_REPLY;
            bmw_reply(0);
            imode ^= IM_REPLY;

            /*
             * Thor.980307: �Q���줰��n��k, �b^R�ɸT��talk, �_�h�|�],
             * �S��vio_fd, �ݤ��� I_OTHERDATA �ҥH�b ctrl-r��talk, �ݤ����襴���r
             */
            extern void (*talk_rqst_signal)(int signum);
            signal(SIGUSR1, talk_rqst_signal);

            continue;
        }

        return (cc);
    }
}


#define MATCH_END       0x8000
/* Thor.990204: ����: �N��MATCH����, �n���N�ɨ�,
                      �n���N�����쪬, ���q�X�i�઺�ȤF */

static void
match_title(void)
{
    move(2, 0);
    clrtobot();
    vs_line("������T�@����");
}


static int
match_getch(void)
{
    int ch;

    outs("\n�� �C��(C)�~�� (Q)���� ? [C] ");
    ch = vkey();
    if (ch == 'q' || ch == 'Q')
        return ch;

    move(3, 0);
    clrtobot();
    return 0;
}


/* ----------------------------------------------------- */
/* ��� board                                            */
/* ----------------------------------------------------- */


static BRD *xbrd;


BRD *
ask_board(
    char *board,
    int perm,
    char *msg)
{
    if (msg)
    {
        move(2, 0);
        outs(msg);
    }

    if (vget(1, 0, "�п�J�ݪO�W��(���ť���۰ʷj�M)�G",
            board, IDLEN + 1, GET_BRD | perm))
    {
        if (!str_cmp(board, currboard))
            *board = 0;         /* ��ثe���ݪO�@�� */
        return xbrd;
    }

    return NULL;
}


static int
vget_match(
    char *prefix,
    int len,
    int op)
{
    char *data, *hit=NULL;
    int row, col, match;

    row = 3;
    col = match = 0;

    if (op & GET_BRD)
    {
        unsigned int perm;
        char *bits;
        BRD *head, *tail;

        extern BCACHE *bshm;
        extern char brd_bits[];

        perm = op & (BRD_R_BIT | BRD_W_BIT);
        bits = brd_bits;
        head = bshm->bcache;
        tail = head + bshm->number;

        do
        {
            if (perm & *bits++)
            {
                data = head->brdname;

                if (str_ncmp(prefix, data, len))
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
                if (col >= 77)
                {
                    col = 0;
                    if (++row >= b_lines)
                    {
                        if (match_getch() == 'q')
                            break;

                        move(row = 3, 0);
                        clrtobot();
                    }
                }
            }
        } while (++head < tail);
    }
    else if (op & GET_USER)
    {
        struct dirent *de;
        DIR *dirp;
        int cc;
        int cd;
        char fpath[16];

        /* Thor.981203: USER name�ܤ֥��@�r, ��"<="�|����n��? */
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
            if (!HAS_PERM(PERM_ADMIN))  /* �@��ϥΪ̭n��J�@�r�~��� */
                return 0;
            cc = 'a';
            cd = 'z';
        }

        for (; cc <= cd; cc++)
        {
            sprintf(fpath, "usr/%c", cc);
            dirp = opendir(fpath);
            while ((de = readdir(dirp)))
            {
                data = de->d_name;
                if (*data <= ' ' || *data == '.')
                    continue;

//              if (str_ncmp(prefix, data, len))
                if (len && str_ncmp(prefix, data, len))
                    continue;

                if (!match++)
                {
                    match_title();
                    strcpy(hit = fpath, data);  /* �Ĥ@���ŦX����� */
                }

                move(row, col);
                outs(data);
                col += IDLEN + 1;

                if (col >= 72)
                {
                    col = 0;
                    if (++row >= b_lines)
                    {
                        if (match_getch())
                    {
                        cc = 'z';     /* ���} for �j�� */
                        break;
                    }
                    row = 3;
                }
            }
        }

        closedir(dirp);
        }
    }
    else /* Thor.990203: ����, GET_LIST */
    {
        LinkList *list;
        extern LinkList *ll_head;

        for (list = ll_head; list; list = list->next)
        {
            data = list->data;

            if (str_ncmp(prefix, data, len))
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
            if (col >= 77)
            {
                col = 0;
                if (++row >= b_lines)
                {
                    if (match_getch())
                        break;
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


int vget(int line, int col, char *prompt, char *data, int max, int echo)
{
    int ch, len;
    int x, y;
    int i, next;

    if (prompt)
    {
        move(line, col);
        clrtoeol();
        outs(prompt);
    }
    else
    {
        clrtoeol();
    }

    STANDOUT;

#ifdef M3_USE_PFTERM
    getyx (&y, &x);
#else
    y = cur_row;
    x = cur_col;
#endif

    if (echo & GCARRY)
    {
        if ((len = strlen(data)))
        {
            outs(data);
        }
    }
    else
    {
        len = 0;
    }

    /* --------------------------------------------------- */
    /* ���o board / userid / on-line user                  */
    /* --------------------------------------------------- */

    ch = len;
    do
    {
        outc(' ');
    } while (++ch < max);

    STANDEND;

    line = -1;
    col = len;
    max--;

    for (;;)
    {
        move(y, x + col);
        ch = vkey();
        if (ch == '\n')
        {
            data[len] = '\0';
            if ((echo & (GET_BRD | GET_LIST)) && len > 0)
            /* Thor.990204:�n�D��J���@�r�~�N��۰� match, �_�h��cancel */
            {
                ch = len;
                len = vget_match(data, len, echo | MATCH_END);
#ifdef M3_USE_PFTERM
                STANDOUT;
#endif
                if (len > ch)
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

        if (isprint2(ch))
        {
            if (ch == ' ' && (echo & (GET_USER | GET_BRD | GET_LIST)))
            {
                ch = vget_match(data, len, echo);
#ifdef M3_USE_PFTERM
                STANDOUT;
#endif
                if (ch > len)
                {
                    move(y, x);
                    outs(data);
                    col = len = ch;
                }
#ifdef M3_USE_PFTERM
                STANDEND;
#endif
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

            prompt = &data[col];
            i = col;
            move(y, x + col);
#ifdef M3_USE_PFTERM
            STANDOUT;
#endif

            for (;;)
            {
                outc(echo ? ch : '*');
                next = (unsigned char) *prompt;
                *prompt++ = ch;
                if (i >= len)
                    break;
                i++;
                ch = next;
            }
#ifdef M3_USE_PFTERM
            STANDEND;
#endif
            col++;
            len++;
            continue;
        }

        /* ----------------------------------------------- */
        /* ��J password / match-list �ɥu��� BackSpace   */
        /* ----------------------------------------------- */

        if (!echo && ch != Ctrl('H'))
        {
            bell();
            continue;
        }

#ifdef M3_USE_PFTERM
        STANDOUT;
#endif
        switch (ch)
        {
        case KEY_DEL:
        case Ctrl('D'):

            if (col >= len)
            {
                bell();
                break;
            }

            col++;
            // Falls through

        case Ctrl('H'):

            if (!col)
            {
                bell();
                break;
            }

            /* ----------------------------------------------- */
            /* remove data and display it                      */
            /* ----------------------------------------------- */

            i = col--;
            len--;
            move(y, x + col);
            while (i <= len)
            {
                data[i - 1] = ch = (unsigned char) data[i];
                outc(echo ? ch : '*');
                i++;
            }
            outc(' ');
            break;

        case KEY_LEFT:
        case Ctrl('B'):
            if (col)
                --col;
            else
                bell();
            break;

        case KEY_RIGHT:
        case Ctrl('F'):
            if (col < len)
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
                for (ch = 0; ch < len; ch++)
                    outc(' ');
                col = len = 0;
            }
            break;

        case Ctrl('K'):         /* delete to end of line */
            if (col < len)
            {
                move(y, x + col);
                for (ch = col; ch < len; ch++)
                    outc(' ');
                len = col;
            }
            break;

        default:
            ch |= KEY_NONE;   /* Non-processed key */
            break;
        }
#ifdef M3_USE_PFTERM
        STANDEND;
#endif

        /* No further processing is needed */
        if (!(ch & KEY_NONE))
            continue;

        /* No input history hidden inputs */
        if (!echo)
        {
            bell();
            continue;
        }

#ifdef M3_USE_PFTERM
        STANDOUT;
#endif

        ch ^= KEY_NONE;
        switch (ch)
        {
        case KEY_DOWN:
        case Ctrl('N'):

            line += MAXLASTCMD - 2;
            // Falls through

        case KEY_UP:
        case Ctrl('P'):

            line = (line + 1) % MAXLASTCMD;
            prompt = lastcmd[line];
            col = 0;
            move(y, x);

            do
            {
                if (!(ch = (unsigned char) *prompt++))
                {
                    /* clrtoeol */

                    for (ch = col; ch < len; ch++)
                        outc(' ');
                    break;
                }

                outc(ch);
                data[col] = ch;
            } while (++col < max);

            len = col;
            break;

        case Ctrl('K'):         /* delete to end of line */
            if (col < len)
            {
                move(y, x + col);
                for (ch = col; ch < len; ch++)
                    outc(' ');
                len = col;
            }
        default:
            /* Invalid key */
            bell();
            break;
        }
#ifdef M3_USE_PFTERM
        STANDEND;
#endif
    }

    if (len > 2 && echo)
    {
        for (line = MAXLASTCMD - 1; line; line--)
            strcpy(lastcmd[line], lastcmd[line - 1]);
        strcpy(lastcmd[0], data);
    }

    outc('\n');

    if (echo & LCECHO)
    {
        for (col = 0; col < len; col++)
        {
            ch = (unsigned char) data[col];
            if (ch >= 0x81 && ch <= 0xFE)
                /* The first byte of double-byte char of Big5 */
                col++;  /* Skip the next byte */
            else if (ch >= 'A' && ch <= 'Z')
                data[col] += 32;
        }
    }
    ch = (unsigned char) data[0];

#ifdef M3_USE_PFTERM
        STANDEND;
#endif

    return ch;
}


int
vans(
    char *prompt)
{
    char ans[3];

    return vget(b_lines, 0, prompt, ans, sizeof(ans), LCECHO);
}


#undef  TRAP_ESC

int
vkey(void)
{
    int mode;
    int ch, last, last2;

    mode = last = last2 = 0;
    for (;;)
    {
        ch = igetch();
#ifdef  TRAP_ESC
        if (mode == 0)
        {
            if (ch == KEY_ESC)
                mode = 1;
            else
                return ch;              /* Normal Key */
        }
#else
        if (ch == KEY_ESC)
            mode = 1;
        else if (mode == 0)             /* Normal Key */
        {
            return ch;
        }
#endif
        else if (mode == 1)
        {                               /* Escape sequence */
            if (ch == '[' || ch == 'O')
                mode = 2;
            else if (ch == '1' || ch == '4')
                mode = 3;
            else
            {
#ifdef  TRAP_ESC
                return Meta(ch);
#else
                return ch;
#endif
            }
        }
        else if (mode == 2)
        {
            if (ch >= 'A' && ch <= 'D')      /* Cursor key */
                return KEY_UP + (ch - 'A');
            else if (last == 'O')
            {
                if (ch >= 'P' && ch <= 'S')  /* F1 - F4 */
                    return KEY_F1 + (ch - 'P');
                else
                    return ch;
            }
            else if (ch == 'Z')              /* Shift-Tab */
                return KEY_STAB;
            else if (ch >= '1' && ch <= '6')
                mode = 3;
            else
                return ch;
        }
        else if (mode == 3)
        {                               /* Ins Del Home End PgUp PgDn */
            if (ch == '~')
                return KEY_HOME + (last - '1');
            else if (last >= '1' && last <= '2')
                mode = 4;
            else
                return ch;
        }
        else if (mode == 4)
        {                               /* F1 - F12 */
            if (ch == '~')
            {
                if (last2 == '1')       /* F1 - F8 */
                    return KEY_F1 + (last - '1') - (last > '6');
                else if (last2 == '2')  /* F9 - F12 */
                    return KEY_F9 + (last - '0') - (last > '2');
                else
                    return ch;
            }
            else
                return ch;
        }
        last2 = last;
        last = ch;
    }
}
