//////////////////////////////////////////////////////////////////////////
// pfterm environment settings
//////////////////////////////////////////////////////////////////////////
//#define PFTERM_HAVE_VKEY

#ifdef PFTERM_TEST_MAIN

#define USE_PFTERM
#define EXP_PFTERM
#define DBCSAWARE
#define FT_DBCS_NOINTRESC 1
#define FT_COLOR_NOCOLOR 0
#define DBG_TEXT_FD

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#else

#define HAVE_GRAYOUT
#include "bbs.h"

#ifndef FT_DBCS_NOINTRESC
# ifdef  UF_DBCS_NOINTRESC
#  define FT_DBCS_NOINTRESC ((void)0, cuser.uflag & UF_DBCS_NOINTRESC)
# else
#  define FT_DBCS_NOINTRESC 0
# endif
#endif

#ifndef FT_COLOR_NOCOLOR
# ifdef  UF_COLOR_NOCOLOR
#  define FT_COLOR_NOCOLOR ((void)0, cuser.uflag & UF_COLOR_NOCOLOR)
# else
#  define FT_COLOR_NOCOLOR 0
# endif
#endif

#ifdef M3_USE_PFTERM
 #include <assert.h>
 #define USE_PFTERM
 #undef PFTERM_HAVE_VKEY
 #define PFTERM_EXPOSED_VISIO_VI
 #define PFTERM_SUPPORT_IAC   /* `ochar()` handles `'\xFF'` correctly */
#endif //M3_USE_PFTERM

#ifndef PFTERM_HAVE_VKEY
 #ifdef PFTERM_EXPOSED_VISIO_VI
  #ifdef __cplusplus
extern "C" {
  #endif
// Use visio internal variables
extern int vi_size;
extern int vi_head;
  #ifdef __cplusplus
}  /* extern "C" */
  #endif

static int vkey_is_typeahead(void)
{
    return vi_head < vi_size;
}
 #endif
#endif  /* #ifndef PFTERM_HAVE_VKEY */

#endif  /* #ifdef PFTERM_TEST_MAIN */

//////////////////////////////////////////////////////////////////////////
// pfterm debug settings
//////////////////////////////////////////////////////////////////////////

// #define DBG_SHOW_DIRTY
// #define DBG_SHOW_REPRINT     // will not work if you enable SHOW_DIRTY.
// #define DBG_DISABLE_OPTMOVE
// #define DBG_DISABLE_REPRINT

//////////////////////////////////////////////////////////////////////////
// pmore style ansi
// #include "ansi.h"
//////////////////////////////////////////////////////////////////////////

#ifndef PMORE_STYLE_ANSI
#define ESC_CHR '\x1b'
#define ESC_STR "\x1b"
#define ANSI_COLOR(x) ESC_STR "[" #x "m"
#define ANSI_RESET ESC_STR "[m"
#endif // PMORE_STYLE_ANSI

#ifndef ANSI_IS_PARAM
#define ANSI_IS_PARAM(c) ((c) == ';' || (c) == ':' || ((c) >= '0' && (c) <= '9'))
#endif // ANSI_IS_PARAM

//////////////////////////////////////////////////////////////////////////
// grayout advanced control
// #include "grayout.h"
//////////////////////////////////////////////////////////////////////////
#ifndef GRAYOUT_DARK
#define GRAYOUT_COLORBOLD (-2)
#define GRAYOUT_BOLD (-1)
#define GRAYOUT_DARK 0
#define GRAYOUT_NORM 1
#define GRAYOUT_COLORNORM 2
#endif // GRAYOUT_DARK

//////////////////////////////////////////////////////////////////////////
// Typeahead
//////////////////////////////////////////////////////////////////////////
#ifndef TYPEAHEAD_NONE
#define TYPEAHEAD_NONE  (-1)
#define TYPEAHEAD_STDIN 0
#endif // TYPEAHEAD

//////////////////////////////////////////////////////////////////////////
// pfterm: piaip's flat terminal system, a new replacement for 'screen'.
// pfterm can also be read as "Perfect Term"
//
// piaip's new implementation of terminal system (term/screen) with dirty
// map, designed and optimized for ANSI based output.
//
// "pfterm" is "piaip's flat terminal" or "perfect term", not "PTT's term"!!!
// pfterm is designed for general maple-family BBS systems, not
// specific to any branch.
//
// Author: Hung-Te Lin (piaip), Dec 2007.
//
// Copyright (c) 2007-2008 Hung-Te Lin <piaip@csie.ntu.edu.tw>
// All rights reserved.
//
// Distributed under a Non-Commercial 4clause-BSD alike license.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. All advertising materials mentioning features or use of this software
//    must display appropriate acknowledgement, like:
//        This product includes software developed by Hung-Te Lin (piaip).
//    The acknowledgement can be localized with the name unchanged.
// 4. You may not exercise any of the rights granted to you above in any
//    manner that is primarily intended for or directed toward commercial
//    advantage or private monetary compensation. For avoidance of doubt,
//    using in a program providing commercial network service is also
//    prohibited.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
// IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
// TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
// PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
// OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// MAJOR IMPROVEMENTS:
//  - Interpret ANSI code and maintain a virtual terminal
//  - Dual buffer for dirty map and optimized output
//
// TODO AND DONE:
//  - DBCS aware and prevent interrupting DBCS [done]
//  - optimization with relative movement [done]
//  - optimization with ENTER/clrtoeol [done]
//  - ncurses-like API [done]
//  - inansistr to output escaped string [done]
//  - handle incomplete DBCS characters [done]
//  - optimization with reprint ability [done]
//  - add auto wrap control
//
// DEPRECATED:
//  - standout() [rework getdata() and namecomplete()]
//  - talk.c (big_picture) [rework talk.c]
//
//////////////////////////////////////////////////////////////////////////
// Reference:
// https://en.wikipedia.org/wiki/ANSI_escape_code
// http://www.ecma-international.org/publications/files/ECMA-ST/Ecma-048.pdf
// https://www.itu.int/rec/T-REC-T.416-199303-I/en
// https://docs.microsoft.com/en-us/windows/console/console-virtual-terminal-sequences
//////////////////////////////////////////////////////////////////////////
// The supported escapes are based on 'cons25' termcap and
// Windows 2000/XP/Vista built-in telnet.
//
// Underline is only supported by vt100 (and monochrome) so we drop it.
// Blink is supported by vt100/cons25 and we keep it.
//////////////////////////////////////////////////////////////////////////
// Additional remarks by IID on 2019-10-02:
// About `cons25`:
// `cons25` is the termcap/terminfo for the older console driver `syscons` of FreeBSD.
// `vt` replaced `syscons` as the default system console driver in FreeBSD 9.3,
//    whose terminals have their terminfo type `$TERM` default to `xterm`.
//
// About the Windows OS versions:
// Windows 2000/XP/Vista were the dominant Windows versions in 2008,
//    which occupied 99.35% of the market share of Windows OSs in 2009-1,
//    but occupied only 3.47% of the market share from 2018-9 to 2019-9.
// The dominant Windows versions from 2018-9 to 2019-9
//    were Windows 7/8.1/10, which occupied 95.62% of the market share of Windows OSs,
//    while the officially unsupported Windows XP
//       still occupied 3.25% of the market share.
//
// The supported escape sequences should be now based on `xterm` terminfo
//    and the built-in terminal of Windows XP/7/8.1/10.
//
// The built-in terminal of Windows 10 supports many `xterm`-based escape sequences
//    under the name of 'virtual terminal sequences'.
//
// About the support for underlines and blinks, as for 2019-10:
// Support for underlines was added in the built-in terminal of Windows 10.
// Support for blinks has not hitherto happened on the built-in terminal of Windows.
//////////////////////////////////////////////////////////////////////////

// Experimental now
#if defined(EXP_PFTERM) || defined(USE_PFTERM)

//////////////////////////////////////////////////////////////////////////
// pfterm Configurations
//////////////////////////////////////////////////////////////////////////

// Prevent invalid DBCS characters
#define FTCONF_PREVENT_INVALID_DBCS

// Force to clear before drawing unsafe DBCS characters
#define FTCONF_CLEAR_UNSAFE_DBCS

// Some terminals use current attribute to erase background
#define FTCONF_CLEAR_SETATTR

// Some terminals (NetTerm, PacketSite) have bug in bolding attribute.
#define FTCONF_WORKAROUND_BOLD

// Some terminals prefer VT100 style scrolling, including Win/DOS telnet
#undef  FTCONF_USE_ANSI_SCROLL
#undef  FTCONF_USE_VT100_SCROLL

// Few poor terminals do not have relative move (ABCD).
#undef  FTCONF_USE_ANSI_RELMOVE

// Handling ANSI commands with 2 parameters (ex, ESC[m;nH)
// 2: Good terminals can accept any omit format (ESC[;nH)
// 1: Poor terminals (eg, Win/DOS telnet) can only omit 2nd (ESC[mH)
// 0: Very few poor terminals (eg, CrazyTerm/BBMan) cannot omit any parameters
#define FTCONF_ANSICMD2_OMIT 0

//////////////////////////////////////////////////////////////////////////
// Flat Terminal Definition
//////////////////////////////////////////////////////////////////////////

#define FTSZ_DEFAULT_ROW 24
#define FTSZ_DEFAULT_COL 80
#define FTSZ_MIN_ROW     24
#define FTSZ_MAX_ROW     100
#define FTSZ_MIN_COL     80
#define FTSZ_MAX_COL     320

#define FTCHAR_ERASE     ' '
#define FTATTR_ERASE     0x07
#define FTCHAR_BLANK     ' '
#define FTATTR_DEFAULT   FTATTR_ERASE
#define FTCHAR_INVALID_DBCS '?'
// #define FTATTR_TRANSPARENT 0x80
#define FTCATTR_DEFAULT  0x00

#define FTDIRTY_CHAR    0x01
#define FTDIRTY_ATTR    0x02
#define FTDIRTY_DBCS    0x04
#define FTDIRTY_INVALID_DBCS 0x08
#define FTDIRTY_RAWMOVE 0x10
#define FTDIRTY_CLEAR_DBCS 0x20

#define FTDBCS_SAFE     0       // standard DBCS
#define FTDBCS_UNSAFE   1       // not on all systems (better do rawmove)
#define FTDBCS_INVALID  2       // invalid

#define FTCMD_MAXLEN    63      // max escape command sequence length
#define FTATTR_MINCMD   16

#ifndef FTCONF_USE_ANSI_RELMOVE
# define FTMV_COST      8       // always ESC[m;nH will costs avg 8 bytes
#else
# define FTMV_COST      5       // ESC[ABCD with ESC[m;nH costs avg 4+ bytes
#endif

//////////////////////////////////////////////////////////////////////////
// Flat Terminal Data Type
//////////////////////////////////////////////////////////////////////////

typedef unsigned char ftchar;   // primitive character type
typedef unsigned char ftattr;   // primitive attribute type
typedef unsigned char ftcattr;  // primitive type for cursor attributes

//////////////////////////////////////////////////////////////////////////
// Flat Terminal Structure
//////////////////////////////////////////////////////////////////////////

typedef struct
{
    ftchar  **cmap[2];      // character map
    ftattr  **amap[2];      // attribute map
    ftchar  *dmap;          // dirty map
    ftchar  *dcmap;         // processed display map
    ftattr  attr;
    int     rows, cols;
    int     y, x;
    int     sy, sx;   // stored cursor
    int     mi;       // map index, mi = current map and (1-mi) = old map
    int     dirty;
    int     scroll;   // Positive: Scrolling up (forward)

    // memory allocation
    int     mrows, mcols;

    // raw terminal status
    int     ry, rx;
    ftattr  rattr;

    // typeahead
    char    typeahead;

    // extra attributes for the cursor
    ftcattr cattr;

    // escape command
    ftchar  cmd[FTCMD_MAXLEN+1];
    int     szcmd;

} FlatTerm;

static FlatTerm ft;

#ifdef _WIN32
    // sorry, we support only 80x24 on Windows now.
    HANDLE hStdout;
    COORD coordBufSize = {80, 24}, coordBufCoord = {0, 0};
    SMALL_RECT winrect = {0, 0, 79, 23};
    CHAR_INFO winbuf[80*24];
#endif

//////////////////////////////////////////////////////////////////////////
// Flat Terminal Utility Macro
//////////////////////////////////////////////////////////////////////////

// ftattr: 0| FG(3) | BOLD(1) | BG(3) | BLINK(1) |8
#define FTATTR_FGSHIFT  0
#define FTATTR_BGSHIFT  4
#define FTATTR_GETFG(x) (((x) >> FTATTR_FGSHIFT) & 0x7)
#define FTATTR_GETBG(x) (((x) >> FTATTR_BGSHIFT) & 0x7)
#define FTATTR_FGMASK   ((ftattr)(0x7 << FTATTR_FGSHIFT))
#define FTATTR_BGMASK   ((ftattr)(0x7 << FTATTR_BGSHIFT))
#define FTATTR_BOLD     0x08
#define FTATTR_BLINK    0x80
#define FTATTR_DEFAULT_FG   (FTATTR_GETFG(FTATTR_DEFAULT))
#define FTATTR_DEFAULT_BG   (FTATTR_GETBG(FTATTR_DEFAULT))
#define FTATTR_MAKE(f, b)   (((f)<<FTATTR_FGSHIFT)|((b)<<FTATTR_BGSHIFT))
#define FTCHAR_ISBLANK(x)   ((x) == (FTCHAR_BLANK))

// ftcattr: 0| UNUSED(3) | BGBRIGHT(1) | FGBRIGHT(1) | ITALIC(1) | REVERSE(1) | CONCEAL(1) |8
#define FTCATTR_FGBRIGHT   0x08
#define FTCATTR_BGBRIGHT   0x10
#define FTCATTR_ITALIC     0x20
#define FTCATTR_REVERSE    0x40
#define FTCATTR_CONCEAL    0x80

#define FTCMAP  ft.cmap[ft.mi]
#define FTAMAP  ft.amap[ft.mi]
#define FTCROW  FTCMAP[ft.y]
#define FTAROW  FTAMAP[ft.y]
#define FTC     FTCROW[ft.x]
#define FTA     FTAROW[ft.x]
#define FTD     ft.dmap
#define FTDC    ft.dcmap
#define FTPC    (FTCROW+ft.x)
#define FTPA    (FTAROW+ft.x)
#define FTOCMAP ft.cmap[1-ft.mi]
#define FTOAMAP ft.amap[1-ft.mi]


// for fast checking, we use reduced range here.
// Big5: LEAD = 0x81-0xFE, TAIL = 0x40-0x7E/0xA1-0xFE
#define FTDBCS_ISLEAD(x) (((unsigned char)(x))>=(0x80))
#define FTDBCS_ISTAIL(x) (((unsigned char)(x))>=(0x40))

#ifdef PFTERM_SUPPORT_IAC
//  - 0x80 is invalid for Big5.
#define FTDBCS_ISBADLEAD(x) (((unsigned char)(x)) == 0x80)
#else
//  - 0xFF is used as telnet IAC, don't use it!
#define FTDBCS_ISBADLEAD(x) ((((unsigned char)(x)) == 0x80) || (((unsigned char)(x)) == 0xFF))
#endif

// even faster:
// #define FTDBCS_ISLEAD(x) (((unsigned char)(x)) & 0x80)
// #define FTDBCS_ISTAIL(x) (((unsigned char)(x)) & ((unsigned char)~0x3F))

#define FTDBCS_ISSBCSPRINT(x) \
    (((unsigned char)(x))>=' ' && ((unsigned char)(x))<0x80)

#ifndef min
#define min(x, y) (((x)<(y))?(x):(y))
#endif

#ifndef max
#define max(x, y) (((x)>(y))?(x):(y))
#endif

#ifndef abs
#define abs(x) (((x)>0)?(x):-(x))
#endif

#define ranged(x, vmin, vmax) max(min(x, vmax), vmin)

#ifdef __cplusplus
extern "C" {
#endif

//////////////////////////////////////////////////////////////////////////
// Flat Terminal API
//////////////////////////////////////////////////////////////////////////

//// common ncurse-like library interface

// initialization
void    initscr     (void);
int     resizeterm  (int rows, int cols);
int     endwin      (void);

// attributes
ftattr  attrget     (void);
void    attrset     (ftattr attr);
void    attrsetfg   (ftattr attr);
void    attrsetbg   (ftattr attr);

// cursor
void    getyx       (int *y, int *x);
void    getmaxyx    (int *y, int *x);
void    move        (int y, int x);

// clear
void    clear       (void); // clrscr + move(0, 0)
void    clrtoeol    (void); // end of line
void    clrtobot    (void);
// clear (non-ncurses)
void    clrtoln     (int ln); // clear down to ln ( excluding ln, as [y, ln) )
void    clrcurln    (void); // whole line
void    clrtobeg    (void); // begin of line
void    clrtohome   (void);
void    clrscr      (void); // clear and keep cursor untouched
void    clrregion   (int r1, int r2); // clear [r1, r2], bi-directional.

// window control
void    newwin      (int nlines, int ncols, int y, int x);

// flushing
void    refresh     (void); // optimized refresh
void    doupdate    (void); // optimized refresh, ignore input queue
void    redrawwin   (void); // invalidate whole screen
int     typeahead   (int fd);// prevent refresh if input queue is not empty

// scrolling
void    scroll      (void);     // scroll up (forward)
void    rscroll     (void);     // scroll down (backward)
void    scrl        (int rows);

// output (ncurses flavor)
void    addch       (unsigned char c);  // equivalent to outc()
void    addstr      (const char *s);    // equivalent to outs()
void    addnstr     (const char *s, int n);

// output (non-ncurses)
void    outc        (unsigned char c);
void    outs        (const char *s);
void    outns       (const char *s, int n);
void    outstr      (const char *str);  // prepare and print a complete string.
void    addstring   (const char *str);  // ncurses-like of outstr().

// readback
int     instr       (char *str);
int     innstr      (char *str, int n); // n includes \0
int     inansistr   (char *str, int n); // n includes \0

// deprecated
void    standout    (void);
void    standend    (void);

// grayout advanced control
void    grayoutrect (int y, int yend, int x, int xend, int level);
void    grayout     (int y, int end, int level);

//// flat-term internal processor

int     fterm_typeahead (void);         // raw input  adapter
void    fterm_rawc      (int c);        // raw output adapter
void    fterm_rawnewline(void);         // raw output adapter
void    fterm_rawflush  (void);         // raw output adapter
void    fterm_raws      (const char *s);
void    fterm_rawnc     (int c, int n);
void    fterm_rawnum    (int arg);
void    fterm_rawcmd    (int arg, int defval, char c);
void    fterm_rawcmd2   (int arg1, int arg2, int defval, char c);
void    fterm_rawattr   (ftattr attr);  // optimized changing attribute
void    fterm_rawclear  (void);
void    fterm_rawclreol (void);
void    fterm_rawhome   (void);
void    fterm_rawscroll (int dy);
void    fterm_rawcursor (void);
void    fterm_rawmove   (int y, int x);
void    fterm_rawmove_opt(int y, int x);
void    fterm_rawmove_rel(int dy, int dx);

int     fterm_chattr    (char *s, ftattr oa, ftattr na); // size(s) > FTATTR_MINCMD
void    fterm_exec      (void);             // execute ft.cmd
void    fterm_flippage  (void);
void    fterm_dupe2bk   (void);
void    fterm_markdirty (void);             // mark as dirty
int     fterm_strdlen   (const char *s);    // length of string for display
int     fterm_prepare_str(int len);

// DBCS supporting
int     fterm_DBCS_Big5(unsigned char c1, unsigned char c2);

// internal attribute processing
ftattr  fterm_attrget_raw(void);
ftattr  fterm_apply_cattr(ftattr attr, ftcattr cattr);  // Apply `cattr` to `attr`

#ifdef __cplusplus
}  /* extern "C" */
#endif

//////////////////////////////////////////////////////////////////////////
// Flat Terminal Implementation
//////////////////////////////////////////////////////////////////////////

#define fterm_markdirty() (void) ( ft.dirty = 1, (void)0 )

// initialization

void
initscr(void)
{
#ifdef _WIN32
    hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleScreenBufferSize(hStdout, coordBufSize);
    SetConsoleCursorPosition(hStdout, coordBufCoord);
#endif

    memset(&ft, 0, sizeof(ft));
    ft.attr = ft.rattr = FTATTR_DEFAULT;
    ft.cattr = FTCATTR_DEFAULT;
    resizeterm(FTSZ_DEFAULT_ROW, FTSZ_DEFAULT_COL);

    // clear both pages
    ft.mi = 0; clrscr();
    ft.mi = 1; clrscr();
    ft.mi = 0;

    // typeahead
    ft.typeahead = 1;

    fterm_rawclear();
    move(0, 0);
}

int
endwin(void)
{
    int r, mi = 0;

    // fterm_rawclear();

    for (mi = 0; mi < 2; mi++)
    {
        for (r = 0; r < ft.mrows; r++)
        {
            free(ft.cmap[mi][r]);
            free(ft.amap[mi][r]);
        }
    }
    free(ft.dmap);
    free(ft.dcmap);
    memset(&ft, 0, sizeof(ft));
    return 0;
}

int
resizeterm(int rows, int cols)
{
    int dirty = 0, i = 0;

    rows = ranged(rows, FTSZ_MIN_ROW, FTSZ_MAX_ROW);
    cols = ranged(cols, FTSZ_MIN_COL, FTSZ_MAX_COL);

    // adjust memory only for increasing buffer
    if (rows > ft.mrows || cols > ft.mcols)
    {
        int mi;
        for (mi = 0; mi < 2; mi++)
        {
            // allocate rows
            if (rows > ft.mrows)
            {
                ft.cmap[mi] = (ftchar**)realloc(ft.cmap[mi],
                        sizeof(ftchar*) * rows);
                ft.amap[mi] = (ftattr**)realloc(ft.amap[mi],
                        sizeof(ftattr*) * rows);

                // allocate new columns
                for (i = ft.mrows; i < rows; i++)
                {
                    ft.cmap[mi][i] = (ftchar*)malloc((cols+1) * sizeof(ftchar));
                    ft.amap[mi][i] = (ftattr*)malloc((cols+1) * sizeof(ftattr));
                    // zero at end to prevent over-run
                    ft.cmap[mi][i][cols] = 0;
                    ft.amap[mi][i][cols] = 0;
                }
            }

            // resize cols
            if (cols > ft.mcols)
            {
                for (i = 0; i < ft.mrows; i++)
                {
                    ft.cmap[mi][i] = (ftchar*)realloc(ft.cmap[mi][i],
                            (cols+1) * sizeof(ftchar));
                    ft.amap[mi][i] = (ftattr*)realloc(ft.amap[mi][i],
                            (cols+1) * sizeof(ftattr));
                    // zero at end to prevent over-run
                    ft.cmap[mi][i][cols] = 0;
                    ft.amap[mi][i][cols] = 0;
                }
            } else {
                // we have to deal one case:
                // expand x, shrink x, expand y ->
                //   now new allocated lines will have small x(col) instead of mcol.
                // the solution is to modify mcols here, or change the malloc above
                // to max(ft.mcols, cols).
                ft.mcols = cols;
            }
        }

        // adjusts dirty and display map.
        // no need to initialize anyway.
        if (cols > ft.mcols)
        {
            ft.dmap = (ftchar*) realloc(ft.dmap,
                    (cols+1) * sizeof(ftchar));
            ft.dcmap = (ftchar*) realloc(ft.dcmap,
                    (cols+1) * sizeof(ftchar));
        }

        // do mrows/mcols assignment here, because we had 2 maps running loop above.
        if (cols > ft.mcols) ft.mcols = cols;
        if (rows > ft.mrows) ft.mrows = rows;
        dirty = 1;
    }

    // clear new exposed buffer after resized
    // because we will redrawwin(), so need to change front buffer only.
    for (i = ft.rows; i < rows; i++)
    {
        memset(FTCMAP[i], FTCHAR_ERASE,
                (cols) * sizeof(ftchar));
        memset(FTAMAP[i], FTATTR_ERASE,
                (cols) * sizeof(ftattr));
    }
    if (cols > ft.cols)
    {
        for (i = 0; i < ft.rows; i++)
        {
            memset(FTCMAP[i]+ft.cols, FTCHAR_ERASE,
                    (cols-ft.cols) * sizeof(ftchar));
            memset(FTAMAP[i]+ft.cols, FTATTR_ERASE,
                    (cols-ft.cols) * sizeof(ftattr));
        }
    }

    if (ft.rows != rows || ft.cols != cols)
    {
        ft.rows = rows;
        ft.cols = cols;
        redrawwin();
    }

    ft.x = ranged(ft.x, 0, ft.cols-1);
    ft.y = ranged(ft.y, 0, ft.rows-1);

    return dirty;
}

// attributes

ftattr
fterm_attrget_raw(void)
{
    return ft.attr;
}

ftattr
fterm_apply_cattr(ftattr attr, ftcattr cattr)
{
    if (cattr & (FTCATTR_ITALIC | FTCATTR_REVERSE))
    {
        attr = (attr & ~(FTATTR_FGMASK | FTATTR_BOLD | FTATTR_BGMASK))
            | FTATTR_MAKE(FTATTR_GETBG(attr), FTATTR_GETFG(attr));
        if (cattr & FTCATTR_BGBRIGHT)
            attr |= FTATTR_BOLD;
    }
    else if (cattr & FTCATTR_FGBRIGHT)
        attr |= FTATTR_BOLD;
    if (cattr & FTCATTR_CONCEAL)
        attr = (attr & ~(FTATTR_FGMASK | FTATTR_BOLD))
            | FTATTR_MAKE(FTATTR_GETBG(attr), 0);
    return attr;
}

ftattr
attrget(void)
{
    return fterm_apply_cattr(ft.attr, ft.cattr);
}

void
attrset(ftattr attr)
{
    ft.attr = attr;
}

void
attrsetfg(ftattr attr)
{
    ft.attr &= (~FTATTR_FGMASK);
    ft.attr |= ((attr << FTATTR_FGSHIFT) & FTATTR_FGMASK);
}

void
attrsetbg(ftattr attr)
{
    ft.attr &= (~FTATTR_BGMASK);
    ft.attr |= ((attr << FTATTR_BGSHIFT) & FTATTR_BGMASK);
}

// clear

void
clrscr(void)
{
    int r;
    for (r = 0; r < ft.rows; r++)
        memset(FTCMAP[r], FTCHAR_ERASE, ft.cols * sizeof(ftchar));
    for (r = 0; r < ft.rows; r++)
        memset(FTAMAP[r], FTATTR_ERASE, ft.cols * sizeof(ftattr));
    fterm_markdirty();
}

void
clear(void)
{
    clrscr();
    move(0, 0);
}

void
clrtoeol(void)
{
    ft.x = ranged(ft.x, 0, ft.cols-1);
    ft.y = ranged(ft.y, 0, ft.rows-1);
    memset(FTPC, FTCHAR_ERASE,  ft.cols - ft.x);
    memset(FTPA, FTATTR_ERASE,  ft.cols - ft.x);
    fterm_markdirty();
}

void
clrtobeg(void)
{
    ft.x = ranged(ft.x, 0, ft.cols-1);
    ft.y = ranged(ft.y, 0, ft.rows-1);
    memset(FTCROW, FTCHAR_ERASE, ft.x+1);
    memset(FTAROW, FTATTR_ERASE, ft.x+1);
    fterm_markdirty();
}

void
clrcurrline(void)
{
    ft.y = ranged(ft.y, 0, ft.rows-1);
    memset(FTCROW, FTCHAR_ERASE, ft.cols);
    memset(FTAROW, FTATTR_ERASE, ft.cols);
    fterm_markdirty();
}

void
clrtoln(int line)
{
    if (line <= ft.y)
        return;
    clrregion(ft.y, line-1);
}

void
clrregion(int r1, int r2)
{
    // bi-direction
    if (r1 > r2)
    {
        int r = r1;
        r1 = r2; r2 = r;
    }

    // check r1, r2 range
    r1 = ranged(r1, 0, ft.rows-1);
    r2 = ranged(r2, 0, ft.rows-1);

    for (; r1 <= r2; r1++)
    {
        memset(FTCMAP[r1], FTCHAR_ERASE, ft.cols);
        memset(FTAMAP[r1], FTATTR_ERASE, ft.cols);
    }
    fterm_markdirty();
}

void
clrtobot(void)
{
    clrtoeol();
    clrregion(ft.y+1, ft.rows-1);
}

void
clrtohome(void)
{
    clrtobeg();
    clrregion(ft.y-1, 0);
}

void newwin (int nlines, int ncols, int y, int x)
{
    int oy, ox;
    // check range

    x = ranged(x, 0, ft.cols-1);
    y = ranged(y, 0, ft.rows-1);
    ncols  = ranged(x+ncols-1,  x, ft.cols-1);
    nlines = ranged(y+nlines-1, y, ft.rows-1);
    ncols = ncols - x + 1;
    nlines= nlines- y + 1;

    if (nlines <= 0 || ncols <= 0)
        return;
    getyx(&oy, &ox);

    while (nlines-- > 0)
    {
        move(y++, x);
        // use prepare_str to erase character
        fterm_prepare_str(ncols);
        // memset(FTAMAP[y]+x, ft.attr, ncols);
        // memset(FTCMAP[y]+x, FTCHAR_ERASE, ncols);
    }
    move(oy, ox);
}

// dirty and flushing

void
redrawwin(void)
{
    // flip page
    fterm_flippage();
    clrscr();

    // clear raw terminal
    fterm_rawclear();

    // flip page again
    fterm_flippage();

    // mark for refresh.
    fterm_markdirty();
}

int
typeahead(int fd)
{
    switch (fd)
    {
        case TYPEAHEAD_NONE:
            ft.typeahead = 0;
            break;
        case TYPEAHEAD_STDIN:
            ft.typeahead = 1;
            break;
        default: // shall never reach here
            assert(NULL);
            break;
    }
    return 0;
}

void
refresh(void)
{
    // prevent passive update
    if (ft.typeahead && fterm_typeahead())
        return;
    doupdate();
}

void
doupdate(void)
{
    int y, x;
    char touched = 0;

    if (!ft.dirty)
    {
        fterm_rawcursor();
        return;
    }

#ifdef _WIN32

    assert(ft.rows == coordBufSize.Y);
    assert(ft.cols == coordBufSize.X);

    for (y = 0; y < ft.rows; y++)
    {
        for (x = 0; x < ft.cols; x++)
        {
            WORD xAttr = FTAMAP[y][x], xxAttr;
            // w32 attribute: bit swap (0, 2) and (4, 6)
            xxAttr = xAttr & 0xAA;
            if (xAttr & 0x01) xxAttr |= 0x04;
            if (xAttr & 0x04) xxAttr |= 0x01;
            if (xAttr & 0x10) xxAttr |= 0x40;
            if (xAttr & 0x40) xxAttr |= 0x10;

            winbuf[y*ft.cols + x].Attributes= xxAttr;
            winbuf[y*ft.cols + x].Char.AsciiChar = FTCMAP[y][x];
        }
    }
    WriteConsoleOutputA(hStdout, winbuf, coordBufSize, coordBufCoord, &winrect);

#else // !_WIN32

    // if scroll, do it first
    if (ft.scroll)
        fterm_rawscroll(ft.scroll);

    // calculate and optimize dirty
    for (y = 0; y < ft.rows; y++)
    {
        int len = ft.cols, ds = 0, derase = 0;
        char dbcs = 0, odbcs = 0; // 0: none, 1: lead, 2: tail

        // reset dirty and display map
        memset(FTD, 0,          ft.cols * sizeof(ftchar));
        memcpy(FTDC, FTCMAP[y], ft.cols * sizeof(ftchar));

        // first run: character diff
        for (x = 0; x < len; x++)
        {
            // build base dirty information
            if (FTCMAP[y][x] != FTOCMAP[y][x])
                FTD[x] |= FTDIRTY_CHAR, ds++;
            if (FTAMAP[y][x] != FTOAMAP[y][x])
                FTD[x] |= FTDIRTY_ATTR, ds++;

            // determine DBCS status
            if (dbcs == 1)
            {
#ifdef FTCONF_PREVENT_INVALID_DBCS
                switch (fterm_DBCS_Big5(FTCMAP[y][x-1], FTCMAP[y][x]))
                {
                    case FTDBCS_SAFE:
                        // safe to print
                        FTD[x-1] &= ~FTDIRTY_INVALID_DBCS;
                        FTDC[x-1] = FTCMAP[y][x-1];
                        break;

                    case FTDBCS_UNSAFE:
                        // ok to print, but need to rawmove.
                        FTD[x-1] &= ~FTDIRTY_INVALID_DBCS;
                        FTDC[x-1] = FTCMAP[y][x-1];
                        FTD[x-1] |= FTDIRTY_CHAR;
#ifdef FTCONF_CLEAR_UNSAFE_DBCS
                        FTD[x-1] |= FTDIRTY_CLEAR_DBCS;
#endif
                        FTD[x]   |= FTDIRTY_RAWMOVE;
                        break;

                    case FTDBCS_INVALID:
                        // only SBCS safe characters can be print.
                        if (!FTDBCS_ISSBCSPRINT(FTCMAP[y][x]))
                        {
                            FTD[x] |= FTDIRTY_INVALID_DBCS;
                            FTDC[x] = FTCHAR_INVALID_DBCS;
                        }
                        break;
                }
#endif // FTCONF_PREVENT_INVALID_DBCS

                dbcs  = 2;
                // TAIL: dirty prev and me if any is dirty.
                if (FTD[x] || FTD[x-1])
                {
                    FTD[x]  |= FTDIRTY_DBCS;
                    FTD[x-1]|= FTDIRTY_CHAR;
                }
            }
            else if (FTDBCS_ISLEAD(FTCMAP[y][x]))
            {
                // LEAD: clear dirty when tail was found.
                dbcs  = 1;
#ifdef FTCONF_PREVENT_INVALID_DBCS
                FTD[x] |= FTDIRTY_INVALID_DBCS;
                FTDC[x] = FTCHAR_INVALID_DBCS;
#endif // FTCONF_PREVENT_INVALID_DBCS
            }
            else
            {
                // NON-DBCS
                dbcs  = 0;
            }

            if (odbcs == 1)
            {
                // TAIL: dirty prev and me if any is dirty.
                odbcs  = 2;
                if (FTD[x] || FTD[x-1])
                {
                    FTD[x]  |= FTDIRTY_CHAR;
                    FTD[x-1]|= FTDIRTY_CHAR;
                }
            }
            else if (FTDBCS_ISLEAD(FTOCMAP[y][x]))
            {
                // LEAD: dirty next?
                odbcs  = 1;
            }
            else
            {
                odbcs = 0;
            }
        }

#ifndef DBG_SHOW_DIRTY
        if (!ds)
            continue;
#endif // DBG_SHOW_DIRTY

        // Optimization: calculate ERASE section
        // TODO ERASE takes 3 bytes (ESC [ K), so enable only if derase >= 3?
        // TODO ERASE then print can avoid lots of space, optimize in future.
        for (x = ft.cols - 1; x >= 0; x--)
            if (FTCMAP[y][x] != FTCHAR_ERASE ||
                FTAMAP[y][x] != FTATTR_ERASE)
                break;
            else if (FTD[x])
                derase++;

        len = x+1;

        for (x = 0; x < len; x++)
        {
#ifndef DBG_SHOW_DIRTY
            if (!FTD[x])
                continue;
#endif // !DBG_SHOW_DIRTY

            // Optimization: re-print or move?
#ifndef DBG_DISABLE_REPRINT
            while (ft.ry == y && x > ft.rx && abs(x-ft.rx) < FTMV_COST)
            {
                int i;
                // Special case: we may be in position of DBCS tail...
                // Inside a refresh() loop, this will never happen.
                // However it may occur for the first print entering refresh.
                // So enable only space if this is the first run (!touched).

                // if we don't need to change attributes,
                // just print remaining characters
                for (i = ft.rx; i < x; i++)
                {
                    // if same attribute, simply accept.
                    if (FTAMAP[y][i] == ft.rattr && touched)
                        continue;
                    // XXX spaces may accept (BG=rBG),
                    // but that will also change cached attribute.
                    if (!FTCHAR_ISBLANK(FTCMAP[y][i]))
                        break;
                    if (FTATTR_GETBG(FTAMAP[y][i]) != FTATTR_GETBG(ft.rattr))
                        break;
                }
                if (i != x)
                    break;

                // safe to print all!
                // printf("[re-print %d chars]", i-ft.rx);

#ifdef DBG_SHOW_REPRINT
                // reverse to hint this is a re-print
                fterm_rawattr(FTATTR_MAKE(0, 7) | FTATTR_BOLD);
#endif // DBG_SHOW_REPRINT

                for (i = ft.rx; i < x; i++)
                {
                    fterm_rawc(FTDC[i]);
                    FTAMAP[y][i] = FTOAMAP[y][i]; // spaces may change attr...
                    ft.rx++;
                }

                break;
            }
#endif // !DBG_DISABLE_REPRINT

            if (y != ft.ry || x != ft.rx)
                fterm_rawmove_opt(y, x);

#ifdef DBCSAWARE
            if (FTD[x] & FTDIRTY_CLEAR_DBCS) {
                fterm_raws("  \b\b");
            }
            if ((FTD[x] & FTDIRTY_DBCS) && (FT_DBCS_NOINTRESC))
            {
                // prevent changing attributes inside DBCS
            }
            else
#endif // DBCSAWARE
#ifdef DBG_SHOW_DIRTY
            fterm_rawattr(FTD[x] ?
                (FTAMAP[y][x] | FTATTR_BOLD) : (FTAMAP[y][x] & ~FTATTR_BOLD));
#else // !DBG_SHOW_DIRTY
            fterm_rawattr(FTAMAP[y][x]);
#endif // !DBG_SHOW_DIRTY

#ifdef PFTERM_DISABLE_HIDDEN_MESSAGE
            // Disable hidden message, only if current and previous chars are
            // not DBCS chars.  Current dirty format: [CHAR][DBCS]
            if (FTATTR_GETFG(FTAMAP[y][x]) == FTATTR_GETBG(FTAMAP[y][x]) &&
                (FTAMAP[y][x] & ~(FTATTR_FGMASK | FTATTR_BGMASK)) == 0 &&
                !(FTD[x] & FTDIRTY_DBCS) &&
                !(x + 1 < len && (FTD[x+1] & FTDIRTY_DBCS)))
                fterm_rawc(' ');
            else
#endif
            fterm_rawc(FTDC[x]);
            ft.rx++;
            touched = 1;

            if (FTD[x] & FTDIRTY_RAWMOVE)
            {
                fterm_rawcmd2(ft.ry+1, ft.rx+1, 1, 'H');
            }
        }

        if (derase)
        {
            fterm_rawmove_opt(y, len);
            fterm_rawclreol();
        }
    }

#endif // !_WIN32

    // doing fterm_rawcursor() earlier to enable max display time
    fterm_rawcursor();
    fterm_dupe2bk();
    ft.dirty = 0;
}

// cursor management

void
getyx(int *y, int *x)
{
    if (y)
        *y = ft.y;
    if (x)
        *x = ft.x;
}

void
getmaxyx(int *y, int *x)
{
    if (y)
        *y = ft.rows;
    if (x)
        *x = ft.cols;
}

void
move(int y, int x)
{
    ft.y = ranged(y, 0, ft.rows-1);
    ft.x = ranged(x, 0, ft.cols-1);
}

// scrolling

void
scrl(int rows)
{
    if (!rows)
        return;
    rows = ranged(rows, -ft.rows, ft.rows);
    if (rows > 0)
    {
        for (; rows > 0; rows--)
            scroll();
    } else {
        for (; rows < 0; rows++)
            rscroll();
    }
}

void
scroll(void)
{
    // scroll up (forward)
    int y;
    ftchar *c0 = FTCMAP[0], *oc0 = FTOCMAP[0];
    ftattr *a0 = FTAMAP[0], *oa0 = FTOAMAP[0];

    // prevent mixing buffered scroll up+down
    if (ft.scroll < 0)
        fterm_rawscroll(ft.scroll);

    // smart scroll: move pointers
    for (y = 0; y < ft.rows-1; y++)
    {
        FTCMAP[y] = FTCMAP[y+1];
        FTAMAP[y] = FTAMAP[y+1];
        FTOCMAP[y]= FTOCMAP[y+1];
        FTOAMAP[y]= FTOAMAP[y+1];
    }
    FTCMAP[y] = c0;
    FTAMAP[y] = a0;
    FTOCMAP[y]= oc0;
    FTOAMAP[y]= oa0;

    // XXX also clear backup buffer
    // must carefully consider if up then down scrolling.
    fterm_flippage();
    clrregion(ft.rows-1, ft.rows-1);
    fterm_flippage();
    clrregion(ft.rows-1, ft.rows-1);

    ft.scroll ++;
    // fterm_markdirty(); // should be already dirty
}

void
rscroll(void)
{
    // scroll down (backward)
    int y;
    ftchar *c0 = FTCMAP[ft.rows -1], *oc0 = FTOCMAP[ft.rows -1];
    ftattr *a0 = FTAMAP[ft.rows -1], *oa0 = FTOAMAP[ft.rows -1];

    // prevent mixing buffered scroll up+down
    if (ft.scroll > 0)
        fterm_rawscroll(ft.scroll);

    // smart scroll: move pointers
    for (y = ft.rows -1; y > 0; y--)
    {
        FTCMAP[y] = FTCMAP[y-1];
        FTAMAP[y] = FTAMAP[y-1];
        FTOCMAP[y]= FTOCMAP[y-1];
        FTOAMAP[y]= FTOAMAP[y-1];
    }
    FTCMAP[y] = c0;
    FTAMAP[y] = a0;
    FTOCMAP[y]= oc0;
    FTOAMAP[y]= oa0;

    // XXX also clear backup buffer
    // must carefully consider if up then down scrolling.
    fterm_flippage();
    clrregion(0, 0);
    fterm_flippage();
    clrregion(0, 0);

    ft.scroll --;
    // fterm_markdirty(); // should be already dirty
}

// output

void
addch (unsigned char c)
{
    outc(c);
}

void
addstr (const char *s)
{
    outs(s);
}

void
addnstr(const char *s, int n)
{
    outns(s, n);
}

void
addstring(const char *s)
{
    outstr(s);
}

void
outs(const char *s)
{
    if (!s)
        return;
    while (*s)
        outc(*s++);
}

void
outns(const char *s, int n)
{
    if (!s)
        return;
    while (*s && n-- > 0)
        outc(*s++);
}

void
outstr(const char *str)
{
    if (!str)
    {
        fterm_prepare_str(0);
        return;
    }

    // calculate real length of str (stripping escapes)
    // TODO only print by the returned size

    fterm_prepare_str(fterm_strdlen(str));

    outs(str);

    // maybe the source string itself is broken...
    // basically this check should be done by clients, not term library.
#if 0
    {
        int isdbcs = 0;
        while (*str)
        {
            if (isdbcs == 1) isdbcs = 2;
            else if (FTDBCS_ISLEAD(*str)) isdbcs = 1;
            else isdbcs = 0;
            str++;
        }

        if (isdbcs == 1) // incomplete string!
            outs("\b?");
    }
#endif
}

void
outc(unsigned char c)
{
#ifndef PFTERM_SUPPORT_IAC
    // 0xFF is invalid for most cases (even DBCS),
    if (c == 0xFF)
        return;
#endif
    if (c == 0x00)
        return;

    fterm_markdirty();
    if (ft.szcmd)
    {
        // collecting commands
        ft.cmd[ft.szcmd++] = c;

        // control sequence
        if ((ft.szcmd == 2 && c == '[') ||  // Control Sequence Introducer (CSI)
            (ANSI_IS_PARAM(c) && ft.szcmd < FTCMD_MAXLEN))  // a control sequence parameter byte
            return;

        // command is a C1 set element or an independent control function
        // or either a control sequence final byte or an invalid character is reached
        // process as command
        fterm_exec();
        ft.szcmd = 0;
    }
    else if (c == ESC_CHR)
    {
        // start of escaped commands
        ft.cmd[ft.szcmd++] = c;
    }
    else if (c == '\t')
    {
        // tab: move by 8, and erase the moved range
        int x = ft.x;
        if (x % 8 == 0)
            x += 8;
        else
            x += (8-(x%8));
        x = ranged(x, 0, ft.cols-1);
        // erase the characters between
        if (x > ft.x)
        {
            memset(FTCROW+ft.x, FTCHAR_ERASE, x - ft.x);
            memset(FTAROW+ft.x, ft.attr, x-ft.x);
        }
        ft.x = x;
    }
    else if (c == '\b')
    {
        ft.x = ranged(ft.x-1, 0, ft.cols-1);
    }
    else if (c == '\r' || c == '\n')
    {
        // new line: cursor movement, and do not print anything
        // XXX old screen.c also calls clrtoeol() for newlines.
        clrtoeol();
        ft.x = 0;
        ft.y ++;
        while (ft.y >= ft.rows)
        {
            // XXX scroll at next dirty?
            // screen.c ignored such scroll.
            // scroll();
            ft.y --;
        }
    }
    else if (iscntrl((unsigned char)c))
    {
        // unknown control characters: ignore
    }
    else // normal characters
    {
        assert (ft.x >= 0 && ft.x < ft.cols);

#ifdef FTATTR_TRANSPARENT
        if (ft.attr != FTATTR_TRANSPARENT)
#endif // FTATTR_TRANSPARENT
        FTA = attrget();

        // normal characters
        FTC = c;

        ft.x++;
        // XXX allow x == ft.cols?
        if (ft.x >= ft.cols)
        {
            ft.x = 0;
            ft.y ++;
            while (ft.y >= ft.rows)
            {
                // XXX scroll at next dirty?
                // screen.c ignored such scroll.
                // scroll();
                ft.y --;
            }
        }
    }
}

// readback
int
instr       (char *str)
{
    int x = ft.cols -1;
    *str = 0;
    if (ft.y < 0 || ft.y >= ft.rows || ft.x < 0 || ft.x >= ft.cols)
        return 0;

    // determine stopping location
    while (x >= ft.x && FTCROW[x] == FTCHAR_ERASE)
        x--;
    if (x < ft.x) return 0;
    x = x - ft.x + 1;
    memcpy(str, FTCROW+ft.x, x);
    str[x] = 0;
    return x;
}

int
innstr      (char *str, int n)
{
    int on = n;
    int x = ranged(ft.x + n-1, 0, ft.cols-1);
    *str = 0;
    n = x - ft.x+1;
    if (on < 1 || ft.y < 0 || ft.y >= ft.rows || ft.x < 0 || ft.x >= ft.cols)
        return 0;

    // determine stopping location
    while (x >= ft.x && FTCROW[x] == FTCHAR_ERASE)
        x--;
    if (x < ft.x) return 0;
    n = x - ft.x + 1;
    if (n >= on) n = on-1;
    memcpy(str, FTCROW+ft.x, n);
    str[n] = 0;
    return n;
}

int
inansistr   (char *str, int n)
{
    int x = ft.cols -1, i = 0, szTrail = 0;
    char *ostr = str;
    char cmd[FTATTR_MINCMD*2] = "";

    ftattr a = FTATTR_DEFAULT;
    *str = 0;
    if (ft.y < 0 || ft.y >= ft.rows || ft.x < 0 || ft.x >= ft.cols)
        return 0;

    if (n < 1)
        return 0;
    n--; // preserve last zero

    // determine stopping location
    while (x >= ft.x && FTCROW[x] == FTCHAR_ERASE && FTAROW[x] == FTATTR_ERASE)
        x--;

    // retrieve [rt.x, x]
    if (x < ft.x) return 0;

    // preserve some bytes if last attribute is not FTATTR_DEFAULT
    for (i = ft.x; n > szTrail && i <= x; i++)
    {
        *str = 0;

        if (a != FTAROW[i])
        {
            fterm_chattr(cmd, a, FTAROW[i]);
            a = FTAROW[i];

            if (a != FTATTR_DEFAULT)
                szTrail = 3; // ESC [ m
            else
                szTrail = 0;

            if ((int)strlen(cmd) >= n - szTrail)
                break;

            strcpy(str, cmd);
            n -= strlen(cmd);
            str += strlen(cmd);
        }

        // n should > szTrail
        *str ++ = FTCROW[i];
        n--;
    }

    if (szTrail && n >= szTrail)
    {
        *str++ = ESC_CHR; *str++ = '['; *str++ = 'm';
    }

    *str = 0;
    return (str - ostr);
}

// internal core of piaip's flat-term

void
fterm_flippage (void)
{
    // we have only 2 pages now.
    ft.mi = 1 - ft.mi;
}

#ifndef fterm_markdirty
void
fterm_markdirty (void)
{
    ft.dirty = 1;
}
#endif

void fterm_dupe2bk(void)
{
    int r = 0;

    for (r = 0; r < ft.rows; r++)
    {
        memcpy(FTOCMAP[r], FTCMAP[r], ft.cols * sizeof(ftchar));
        memcpy(FTOAMAP[r], FTAMAP[r], ft.cols * sizeof(ftattr));
    }
}

int
fterm_DBCS_Big5(unsigned char c1, unsigned char c2)
{
    // ref: http://www.cns11643.gov.tw/web/word/big5/index.html
    // High byte: 0xA1-0xFE, 0x8E-0xA0, 0x81-0x8D
    // Low  byte: 0x40-0x7E, 0xA1-0xFE
    // C1:  0x80-0x9F
#ifdef FT_DBCS_BIG5
    FT_DBCS_BIG5(c1, c2);
#endif
    if (FTDBCS_ISBADLEAD(c1))
        return  FTDBCS_INVALID;
    if (!FTDBCS_ISTAIL(c2))
        return FTDBCS_INVALID;
    if (c1 >= 0x80 && c1 <= 0x9F)
        return FTDBCS_UNSAFE;
    return FTDBCS_SAFE;
}

int
fterm_prepare_str(int len)
{
    // clear and make (cursor, +len) as DBCS-ready.
    int x = ranged(ft.x, 0, ft.cols-1);
    int y = ranged(ft.y, 0, ft.rows-1);
    int dbcs = 0, sdbcs = 0;

    // TODO what if x+len is outside range?

    // check if (x, y) is in valid range
    if (x != ft.x || y != ft.y)
        return -1;

    len = ranged(x+len, x, ft.cols);

    for (x = 0; x < len; x++)
    {
        // determine DBCS status
        if (dbcs == 1)
            dbcs = 2; // TAIL
        else if (FTDBCS_ISLEAD(FTCROW[x]))
            dbcs = 1; // LEAD
        else
            dbcs = 0;
        if (x == ft.x) sdbcs = dbcs;
    }

    x = ft.x;
    // fix start and end
    if (sdbcs == 2 && x > 0) // TAIL, remove word
        x--;
    if (dbcs == 1 && len < ft.cols) // LEAD, remove word
        len ++;
    len = ranged(len, 0, ft.cols);
    len -= x;
    if (len < 0) len = 0;

    memset(FTCROW + x, FTCHAR_ERASE, len);
    memset(FTAROW + x, ft.attr, len);
    return len;
}


void
fterm_exec(void)
{
    ftchar cmd = ft.cmd[ft.szcmd-1];  // The control function final byte
    char    *p = (char*)ft.cmd + 2;  // For ESC [ (followed by a control sequence parameter string)
    int n = -1, x = -1, y;

    ft.cmd[ft.szcmd] = 0;

    if (isdigit(*p))
        n = strtol(p, &p, 10);
    if (*p == ';')
        p++;
    // p now points to the next parameter sub-string or the final byte

#define EXEC_C1_FS  0x0100     // For ESC cmd (a C1 set element or an independent control function)
    switch ((ft.szcmd) > 2 ? cmd : EXEC_C1_FS | cmd)
    {
        // Cursor Movement

    case 'A':   // CUU: CSI n A
    case 'B':   // CUD: CSI n B
    case 'C':   // CUF: CSI n C
    case 'D':   // CUB: CSI n D
    case EXEC_C1_FS | 'A': // CUU: ESC A (VT52; non-ECMA-48)
    case EXEC_C1_FS | 'B': // CUD: ESC B (VT52; non-ECMA-48)
    case EXEC_C1_FS | 'C': // CUF: ESC C (VT52; non-ECMA-48)
    case EXEC_C1_FS | 'D': // CUB: ESC D (VT52; non-ECMA-48)
        // Moves the cursor n (default 1) cells in the given direction.
        // In this implementation, if the cursor is already at the edge of the screen, this has no effect.
        if (n < 1)
            n = 1;
        getyx(&y, &x);
        if      (cmd == 'A') { y -= n; }
        else if (cmd == 'B') { y += n; }
        else if (cmd == 'C') { x += n; }
        else if (cmd == 'D') { x -= n; }
        move(y, x);
        break;

    case 'E':   // CNL: CSI n E
    case 'F':   // CPL: CSI n F
        // Moves cursor to beginning of the line
        // n (default 1) lines up/down (next/previous line).
        if (n < 1)
            n = 1;
        getyx(&y, &x);
        if      (cmd == 'E') { y -= n; }
        else if (cmd == 'F') { y += n; }
        move(y, 0);
        break;

    case 'G':   // CHA: CSI n G
    case 'd':   // VPA: CSI n d
        // Moves the cursor to column/row n.
        // VPA is affected by text orientation and character path
        if (n < 1)
            n = 1;
        getyx(&y, &x);
        if      (cmd == 'G') { x = n-1; }
        else if (cmd == 'd') { y = n-1; }
        move(y, x);
        break;

    case 'H':   // CUP: CSI n ; m H
    case 'f':   // HVP: CSI n ; m f
        // Moves the cursor to row n, column m.
        // HVP is affected by text orientation and character path
        // The values are 1-based, and default to 1 (top left corner) if omitted.
        // A sequence such as CSI ;5H is a synonym for CSI 1;5H as well as
        // CSI 17;H is the same as CSI 17H and CSI 17;1H
        y = n;
        if (isdigit(*p))  // `p` points to the second param or letter `H`/`f`
            x = atoi((char*)p);
        if (y < 0) y = 1;
        if (x < 0) x = 1;
        move(y-1, x-1);
        break;

        // Clear

    case 'J':   // ED: CSI n J
        //  Clears part of the screen.
        // If n is zero (or missing), clear from cursor to end of screen.
        // If n is one, clear from cursor to beginning of the screen.
        // If n is two, clear entire screen
        if (n == 0 || n < 0)
            clrtobot();
        else if (n == 1)
            clrtohome();
        else if (n == 2)
        {
            clrregion(0, ft.rows-1);
        }
        break;

    case 'K':   // EL: CSI n K
        // Erases part of the line.
        // If n is zero (or missing), clear from cursor to the end of the line.
        // If n is one, clear from cursor to beginning of the line.
        // If n is two, clear entire line. Cursor position does not change.
        if (n == 0 || n < 0)
            clrtoeol();
        else if (n == 1)
            clrtobeg();
        else if (n == 2)
            clrcurrline();
        break;

    case 's':   // SCP: CSI s (VT100; private use)
        // Saves the cursor position.
        getyx(&ft.sy, &ft.sx);
        break;

    case 'u':   // RCP: CSI u (VT100; private use)
        // Restores the cursor position.
        move(ft.sy, ft.sx);
        break;

    case 'S':   // SU: CSI n S
        // Scroll whole page up (forward) by n (default 1) lines.
        // New lines are added at the bottom.
        if (n < 1)
            n = 1;
        scrl(n);
        break;

    case 'T':   // SD: CSI n T
        // Scroll whole page down (backward) by n (default 1) lines.
        // New lines are added at the top.
        if (n < 1)
            n = 1;
        scrl(-n);
        break;

    case 'm':   // SGR: CSI n [;k] m
        // Sets SGR (Select Graphic Rendition) parameters.
        // After CSI, there can be zero or more parameters separated with ;.
        // With no parameters, CSI m is treated as CSI 0 m (reset / normal),
        // which is typical of most of the ANSI code usages.
        // ---------------------------------------------------------
        // SGR implementation:
        //  SGR 0 (reset/normal)        is supported.
        //  SGR 1 (intensity: bold)     is supported.
        //  SGR 2 (intensity: faint)    is converted to (intensity: normal).
        //  SGR 3 (italic: on)          is converted to (italic (cursor): toggle; converted to inverse)
        //  SGR 4 (underline: single)   is not supported.
        //  SGR 5 (blink: slow)         is supported.
        //  SGR 6 (blink: rapid)        is converted to (blink: slow)
        //  SGR 7 (inverse: on)         is supported. (converted to inverse (cursor): toggle; swap fg & bg)
        //  SGR 8 (conceal: on)         is supported (cursor attribute).
        //  SGR 21(underline: double)   is not supported.
        //  SGR 22(intensity: normal)   is supported.
        //  SGR 23(italic: off)         is supported (cursor attribute).
        //  SGR 24(underline: none)     is not supported.
        //  SGR 25(blink: off)          is supported.
        //  SGR 27(inverse: off)        is supported (cursor attribute).
        //  SGR 28(conceal: off)        is supported (cursor attribute).
        //  SGR 30-37 (FG)              is supported.
        //  SGR 38 (FG-extended)        is not supported.
        //  SGR 39 (FG-reset)           is supported.
        //  SGR 40-47 (BG)              is supported.
        //  SGR 48 (BG-extended)        is not supported.
        //  SGR 49 (BG-reset)           is supported.
        //  SGR 90-97   (FG-bright) (aixterm; non-ECMA-48)   is supported (cursor attribute; intensity: bold).
        //  SGR 100-107 (BG-bright) (aixterm; non-ECMA-48)   is converted to (BG-normal)
        if (n == -1)    // the first parameter sub-string
            n = 0;
        while (n > -1)
        {
            if (n >= 30 && n <= 37)
            {
                // set foreground (normal)
                attrsetfg(n - 30);
                ft.cattr &= ~FTCATTR_FGBRIGHT;
            }
            else if (n >= 90 && n <= 97)
            {
                // set foreground (bright)
                attrsetfg(n - 90);
                ft.cattr |= FTCATTR_FGBRIGHT;
            }
            else if (n >= 40 && n <= 47)
            {
                // set background (normal)
                attrsetbg(n - 40);
                ft.cattr &= ~FTCATTR_BGBRIGHT;
            }
            else if (n >= 100 && n <= 107)
            {
                // set background (bright; converted to normal)
                attrsetbg(n - 100);
                ft.cattr |= FTCATTR_BGBRIGHT;
            }
            else switch (n)
            {
            case 0:
                attrset(FTATTR_DEFAULT);
                ft.cattr = FTCATTR_DEFAULT;
                break;
            case 1:
                attrset(fterm_attrget_raw() | FTATTR_BOLD);
                break;
            case 2:
            case 22:
                attrset(fterm_attrget_raw() & ~FTATTR_BOLD);
                break;
            case 3:
                ft.cattr ^= FTCATTR_ITALIC;
                break;
            case 23:
                ft.cattr &= ~FTCATTR_ITALIC;
                break;
            case 5:
            case 6:
                attrset(fterm_attrget_raw() | FTATTR_BLINK);
                break;
            case 25:
                attrset(fterm_attrget_raw() & ~FTATTR_BLINK);
                break;
            case 7:
                ft.cattr ^= FTCATTR_REVERSE;
                break;
            case 27:
                ft.cattr &= ~FTCATTR_REVERSE;
                break;
            case 8:
                ft.cattr |= FTCATTR_CONCEAL;
                break;
            case 28:
                ft.cattr &= ~FTCATTR_CONCEAL;
                break;
            case 39:
                attrsetfg(FTATTR_DEFAULT_FG);
                break;
            case 49:
                attrsetbg(FTATTR_DEFAULT_BG);
                break;
            case 38:
            case 48:
                {
                    // Select Character Foreground Colour & Select Character Background Colour
                    // After `38` or `48`, there is a parameter sub-string,
                    //    consists of zero or more parameter elements separated by `:` (colon)
                    //    or (xterm; non-ECMA-48) by `;` (semicolon).
                    // A parameter element with no digits represents the default value.
                    // ---------------------------------------------------------
                    // Select Character Colour implementation:
                    // SGR 38 0 (implement defined) is not supported.
                    // SGR 38|48 1 (transparent)    is not supported.
                    // SGR 38|48 2 (RGB direct)     is not supported.
                    // SGR 38|48 3 (CMY direct)     is not supported.
                    // SGR 38|48 4 (CMYK direct)    is not supported.
                    // SGR 38|48 5 (indexed)        is not supported.

                    // Note: Possible forms: `{38|48} ; <params>...` and `{38|48} : <params>...`
                    // Since ECMA-48 specifies the parameter sub-strings separator to be ';' (semicolon),
                    //    the former form conforms to the ECMA-48 standard therefore.
                    // However, the latter form is also used by xterm, so both forms need to be handled.

                    // Collect parameter elements
                    int params[8], pos = 0, is_xterm = 0;
                    // Skip the ':' following the `38` or `48`
                    if (*p == ':' && p[-1] != ';')
                        p++;
                    do
                    {
                        if (!ANSI_IS_PARAM(*p))
                            break;
                        if (isdigit(*p))
                            params[pos] = strtol(p, &p, 10);
                        else
                            params[pos] = 0;  // Defaults to `0` for simplicity
                        if (*p == ':')  // Parameter element separator
                        {
                            // This separator is not allowed after a xterm-style separator is used
                            if (!is_xterm)
                                p++;
                            else
                                break;
                        }
                        else if (*p == ';')
                        {
                            if ((params[0] == 2 && pos < 3)  // 4 args; 3 `;`s
                                || (params[0] == 5 && pos < 1))  // 2 args; 1`;`s
                            {
                                // xterm-style parameter element separator
                                is_xterm = 1;
                                p++;
                                continue;
                            }
                            // End of parameter elements
                            break;
                        }
                    } while (++pos < 8);

                    // Process the parameter elements
                    switch (params[0])
                    {
                    case 0:  // implementation defined (only applicable for the character foreground colour)
                        break;
                    case 1:  // transparent
                        break;
                    case 2:  // direct colour in rgb space
                    case 3:  // direct colour in cmy space
                    case 4:  // direct colour in cmyk space
                        // <params[0]> : <color-space-id> : <r/c> : <g/m> : <b/y> : <k> : <tolerance-value> : <tolerance-color-space>
                        // 2 ; <r> ; <g> ; <b> (xterm; non-ECMA-48)
                        break;
                    case 5:  // indexed colour
                        // 5 : <index>
                        // 5 ; <index> (xterm; non-ECMA-48)
                        break;
                    }

                    // Skip ignored parameter elements
                    p += strcspn(p, ";");
                    if (*p == ';')
                        p++;
                    break;
                }
            }

            // parse next command
            n = -1;
            if (*p == ';')
            {
                n = 0;
                p++;
            }
            else if (isdigit(*p))
            {
                n = strtol(p, &p, 10);
                if (*p == ';')
                    p++;
            }
        }
        break;

    default:    // unknown command.
        break;
    }
#undef EXEC_C1_FS
}

int
fterm_chattr(char *s, ftattr oattr, ftattr nattr)
{
    ftattr
        fg, bg, bold, blink,
        ofg, obg, obold, oblink;
    char lead = 1;

    if (oattr == nattr)
        return 0;

    *s++ = ESC_CHR;
    *s++ = '[';

    // optimization: reset as default
    if (nattr == FTATTR_DEFAULT)
    {
        *s++ = 'm';
        *s++ = 0;
        return 1;
    }

    fg = FTATTR_GETFG(nattr);
    bg = FTATTR_GETBG(nattr);
    bold =  (nattr & FTATTR_BOLD) ? 1 : 0;
    blink = (nattr & FTATTR_BLINK)? 1 : 0;

    ofg = FTATTR_GETFG(oattr);
    obg = FTATTR_GETBG(oattr);
    obold =  (oattr & FTATTR_BOLD) ? 1 : 0;
    oblink = (oattr & FTATTR_BLINK)? 1 : 0;

    // we dont use "disable blink/bold" commands,
    // so if these settings are changed then we must reset.
    // another case is changing background to default background -
    // better use "RESET" to override it.
    // Same for foreground.
    // Possible optimization: when blink/bold on, don't RESET
    // for background change?
    if ((oblink != blink && !blink) ||
        (obold  != bold  && !bold)  ||
        (bg == FTATTR_DEFAULT_BG && obg != bg) ||
        (fg == FTATTR_DEFAULT_FG && ofg != fg) )
    {
        // lead must be 1 since this is the first check.
        // We have dead code (else *s++) here but that's simply to ease moving
        // code around.
        if (lead) lead = 0; else *s++ = ';';
        *s++ = '0';

        ofg = FTATTR_DEFAULT_FG;
        obg = FTATTR_DEFAULT_BG;
        obold = 0; oblink = 0;
    }

    if (bold && !obold)
    {
        if (lead) lead = 0; else *s++ = ';';
        *s++ = '1';

#ifdef FTCONF_WORKAROUND_BOLD
        // Issue here:
        // PacketSite does not understand ESC[1m. It needs ESC[1;37m
        // NetTerm defaults bold color to yellow.
        // As a result, we use ESC[1;37m for the case.
        if (fg == FTATTR_DEFAULT_FG)
            ofg = ~ofg;
#endif // FTCONF_WORKAROUND_BOLD

    }
    if (blink && !oblink)
    {
        if (lead) lead = 0; else *s++ = ';';
        *s++ = '5'; // XXX 5(slow) or 6(fast)?
    }
    if (ofg != fg)
    {
        if (lead) lead = 0; else *s++ = ';';
        *s++ = '3';
        *s++ = '0' + fg;
    }
    if (obg != bg)
    {
        if (lead) lead = 0; else *s++ = ';';
        *s++ = '4';
        *s++ = '0' + bg;
    }
    *s++ = 'm';
    *s++ = 0;
    return 1;
}

int
fterm_strdlen(const char *s)
{
    char ansi = 0;
    int sz = 0;

    // the logic should match outc().

    while (s && *s)
    {
        if (!ansi) // ansi == 0
        {
            switch (*s)
            {
                case ESC_CHR:
                    ansi++;
                    break;

                case '\r':
                case '\n':
                    break;

                case '\b':
                    if (sz) sz--;
                    break;

                case '\t':
                    // XXX how to deal with this?
                    sz ++;
                    break;

                default:
                    if (!iscntrl((unsigned char)*s))
                        sz++;
                    break;
            }
        }
        else if (ansi == 1)
        {
            if (*s == '[')
                ansi++;
            else
                ansi = 0;
        }
        else if (!ANSI_IS_PARAM(*s)) // ansi == 2
        {
            // TODO outc() take max to FTCMD_MAXLEN now...
            ansi = 0;
        }
        s++;
    }
    return sz;
}

void
fterm_rawattr(ftattr rattr)
{
    static char cmd[FTATTR_MINCMD*2];
    if (!fterm_chattr(cmd, ft.rattr, rattr) || (FT_COLOR_NOCOLOR))
        return;

    fterm_raws(cmd);
    ft.rattr = rattr;
}

void
fterm_rawnum(int arg)
{
    if (arg < 0 || arg > 99)
    {
        // complex. use printf.
        char sarg[16]; // max int
        sprintf(sarg, "%d", arg);
        fterm_raws(sarg);
    } else if (arg < 10) {
        // 0 .. 10
        fterm_rawc('0' + arg);
    } else {
        fterm_rawc('0' + arg/10);
        fterm_rawc('0' + arg%10);
    }
}
void
fterm_rawcmd(int arg, int defval, char c)
{
    fterm_rawc(ESC_CHR);
    fterm_rawc('[');
    if (arg != defval)
        fterm_rawnum(arg);
    fterm_rawc(c);
}

void
fterm_rawcmd2(int arg1, int arg2, int defval, char c)
{
    fterm_rawc(ESC_CHR);
    fterm_rawc('[');

    // See FTCONF_ANSICMD2_OMIT
    // XXX Win/DOS telnet does now accept omitting first value
    // ESC[nX and ESC[n;X works, but ESC[;mX does not work.
    if (arg1 != defval || arg2 != defval)
    {
#if (FTCONF_ANSICMD2_OMIT >= 2)
        if (arg1 != defval)
#endif
            fterm_rawnum(arg1);

#if (FTCONF_ANSICMD2_OMIT >= 1)
        if (arg2 != defval)
#endif
        {
            fterm_rawc(';');
            fterm_rawnum(arg2);
        }
    }
    fterm_rawc(c);
}

void
fterm_rawclear(void)
{
    fterm_rawhome();
    // ED: CSI n J, 0 = cursor to bottom, 2 = whole
    fterm_raws(ESC_STR "[2J");
}

void
fterm_rawclreol(void)
{
#ifdef FTCONF_CLEAR_SETATTR
    // ftattr oattr = ft.rattr;
    // XXX If we skip with "background only" here, future updating
    // may get wrong attributes. Or not? (consider DBCS...)
    // if (FTATTR_GETBG(oattr) != FTATTR_GETBG(FTATTR_ERASE))
    fterm_rawattr(FTATTR_ERASE);
#endif

    // EL: CSI n K, n = 0
    fterm_raws(ESC_STR "[K");

#ifdef FTCONF_CLEAR_SETATTR
    // No need to do so? because we will always reset attribute...
    // fterm_rawattr(oattr);
#endif
}

void
fterm_rawhome(void)
{
    // CUP: CSI n ; m H
    fterm_raws(ESC_STR "[H");
    ft.rx = ft.ry = 0;
}

void
fterm_rawmove_rel(int dy, int dx)
{
#ifndef FTCONF_USE_ANSI_RELMOVE
    // Old BBS system does not output relative moves (ESC[ABCD) .
    // Poor terminals (ex, pcman-1.0.5-FF20.xpi)
    // do not support relmoves
    fterm_rawmove(ft.ry + dy, ft.rx + dx);
#else
    if (!dx)
    {
        int y = ranged(dy + ft.ry, 0, ft.rows-1);
        dy = y - ft.ry;
        if (!dy)
            return;

        fterm_rawcmd(abs(dy), 1, dy < 0 ? 'A' : 'B');
        ft.ry = y;
    }
    else if (!dy)
    {
        int x = ranged(dx + ft.rx, 0, ft.cols-1);
        dx = x - ft.rx;
        if (!dx)
            return;

        fterm_rawcmd(abs(dx), 1, dx < 0 ? 'D' : 'C');
        ft.rx = x;
    }
    else
    {
        // (dy, dx) are given - use fterm_move.
        fterm_rawmove(ft.ry + dy, ft.rx + dx);
    }
#endif  /* #ifndef FTCONF_USE_ANSI_RELMOVE */
}

void
fterm_rawmove(int y, int x)
{
    y = ranged(y, 0, ft.rows-1);
    x = ranged(x, 0, ft.cols-1);

    if (y == ft.ry && x == ft.rx)
        return;

    // CUP: CSI n ; m H
    fterm_rawcmd2(y+1, x+1, 1, 'H');

    ft.ry = y;
    ft.rx = x;
}

void
fterm_rawmove_opt(int y, int x)
{
    // optimized move
    int ady = abs(y-ft.ry), adx=abs(x-ft.rx);

    if (!adx && !ady)
        return;

#ifdef DBG_DISABLE_OPTMOVE
    return fterm_rawmove(y, x);
#endif

    // known hacks: \r = (x=0), \b=(x--), \n = (y++)
    //
    // Warning: any optimization here should not change displayed content,
    // because we don't have information about content variation information
    // (eg, invalid DBCS bytes will become special marks) here.
    // Any hacks which will try to display data from FTCMAP should be done
    // inside dirty-map calculation, for ex, using spaces to move right,
    // or re-print content.

#ifndef DBG_TEXT_FD
    // x=0: the cheapest output. However not work for text mode fd output.
    // a special case is "if we have to move y to up".
    // and FTCONF_ANSICMD2_OMIT < 1 (cannot omit x).
#if FTCONF_ANSICMD2_OMIT < 1
    if (y >= ft.ry)
#endif
    if (adx && x == 0)
    {
        fterm_rawc('\r');
        ft.rx = x = adx = 0;
    }

#endif // !DBG_TEXT_FD

    // x--: compare with FTMV_COST: ESC[m;nH costs 5-8 bytes
    // in order to prevent wrap, don't use bs when rx exceed boundary (ft.cols)
    if (x < ft.rx && y >= ft.ry && (adx+ady) < FTMV_COST && ft.rx < ft.cols)
    {
        while (adx > 0)
            fterm_rawc('\b'), adx--;
        ft.rx = x;
    }

    // finishing movement
    if (y > ft.ry && ady < FTMV_COST && adx == 0)
    {
        while (ft.ry++ < y)
            fterm_rawc('\n');
        ft.ry = y;
    }
    else if (ady && adx)
    {
        fterm_rawmove(y, x);
    }
    else if (ady)
    {
        fterm_rawmove_rel(y-ft.ry, 0);
    }
    else if (adx)
    {
        fterm_rawmove_rel(0, x-ft.rx);
    }
}

void
fterm_rawcursor(void)
{
#ifdef _WIN32
    COORD cursor;
    cursor.X = ft.x;
    cursor.Y = ft.y;
    SetConsoleCursorPosition(hStdout, cursor);
#else
    // fterm_rawattr(FTATTR_DEFAULT);
    fterm_rawattr(ft.attr);
    fterm_rawmove_opt(ft.y, ft.x);
    fterm_rawflush();
#endif // !_WIN32
}

void
fterm_rawscroll (int dy)
{
#ifdef FTCONF_USE_ANSI_SCROLL
    // SU: CSI n S (up; forward)
    // SD: CSI n T (down; backward)

    char cmd = (dy > 0) ? 'S' : 'T';
    int ady = abs(dy);
    if (ady == 0)
        return;
    if (ady >= ft.rows) ady = ft.rows;
    fterm_rawcmd(ady, 1, cmd);
    ft.scroll -= dy;

#else
    // VT100 flavor:
    //  *  ESC D: scroll down (backward)
    //  *  ESC M: scroll up (forward)
    //
    // Elder BBS systems works in a mixed way:
    // \n at (rows-1) as scroll()
    // and ESC-M at(0) as rscroll().
    //
    // SCP: CSI s / RCP: CSI u
    // Warning: cannot use SCP/RCP here, because on Win/DOS telnet
    // the position will change after scrolling (ex, (25, 0)->(24, 0).
    //
    // Since scroll does not happen very often, let's relax and not
    // optimize these commands here...

    int ady = abs(dy);
    if (ady == 0)
        return;
    if (ady >= ft.rows) ady = ft.rows;

    // we are not going to preserve (rx, ry)
    // so don't use fterm_move*.
    if (dy > 0)
        fterm_rawcmd2(ft.rows, 1, 1, 'H');
    else
        fterm_rawcmd2(1, 1, 1, 'H');

    for (; ady > 0; ady--)
    {
        if (dy >0)
        {
            // Win/DOS telnet may have extra text in new line,
            // because of the IME line.
#ifdef FTCONF_USE_VT100_SCROLL
            fterm_raws(ESC_STR "D" ESC_STR "[K"); // ESC_STR "[K");
#else
            fterm_raws("\n" ESC_STR "[K");
#endif
        } else {
            fterm_raws(ESC_STR "M"); // ESC_STR "[K");
        }
    }

    // Do not use optimized move here, because in poor terminals
    // the coordinates are already out of sync.
    fterm_rawcmd2(ft.ry+1, ft.rx+1, 1, 'H');
    ft.scroll -= dy;
#endif  /* #ifdef FTCONF_USE_ANSI_SCROLL */
}

void
fterm_raws(const char *s)
{
    while (*s)
        fterm_rawc(*s++);
}

void
fterm_rawnc(int c, int n)
{
    while (n-- > 0)
        fterm_rawc(c);
}

//////////////////////////////////////////////////////////////////////////
// grayout advanced control
//////////////////////////////////////////////////////////////////////////
void
grayoutrect(int y, int yend, int x, int xend, int level)
{
    char grattr = FTATTR_DEFAULT;
    int rx;

    y    = ranged(y,   0, ft.rows-1);
    yend = ranged(yend, 0, ft.rows-1);
    x    = ranged(x,   0, ft.cols-1);
    xend = ranged(xend, x, ft.cols);

    // modify attribute based on existing data.
    switch (level) {
        case GRAYOUT_COLORBOLD:
            for (; y < yend; y++) {
                for (rx = x; rx < xend; rx++) {
                    grattr = ((FTAMAP[y][rx] & FTATTR_BOLD) ? FTATTR_BLINK :
                              FTATTR_BOLD);
                    FTAMAP[y][rx] |= grattr;
                }
            }
            return;

        case GRAYOUT_COLORNORM:
            for (; y < yend; y++) {
                for (rx = x; rx < xend; rx++) {
                    grattr = (FTAMAP[y][rx] & FTATTR_BLINK) ? FTATTR_BOLD : 0;
                    FTAMAP[y][rx] = (FTAMAP[y][rx] & ~(FTATTR_BLINK |
                                                     FTATTR_BOLD)) | grattr;
                }
            }
            return;
    }

    // filling with current attributes.
    switch (level) {
        case GRAYOUT_BOLD:
            grattr |= FTATTR_BOLD;
            break;

        case GRAYOUT_DARK:
            grattr = FTATTR_MAKE(0, 0);
            grattr |= FTATTR_BOLD;
            break;

        case GRAYOUT_NORM:
            // normal text
            break;
    }

    for (; y <= yend; y++)
    {
        memset(FTAMAP[y] + x, grattr, xend - x);
    }
}

void
grayout(int y, int end, int level)
{
    grayoutrect(y, end, 0, ft.cols, level);
}

//////////////////////////////////////////////////////////////////////////
// environment specific
//////////////////////////////////////////////////////////////////////////

#ifndef PFTERM_TEST_MAIN

#ifdef __cplusplus
extern "C" {
#endif

void    scr_dump    (screen_backup_t *psb);
void    scr_redump  (screen_backup_t *psb);
void    scr_free    (screen_backup_t *psb);
void    scr_restore_free   (screen_backup_t *psb);
void    scr_restore_keep   (const screen_backup_t *psb);

void move_ansi(int y, int x);
void getyx_ansi(int *y, int *x);
void region_scroll_up(int top, int bottom);

#ifdef __cplusplus
}  /* extern "C" */
#endif

void
scr_dump(screen_backup_t *psb)
{
    psb->raw_memory = NULL;
    scr_redump(psb);
}

void
scr_redump(screen_backup_t *psb)
{
    int y = 0;
    char *p = NULL;

    psb->row= ft.rows;
    psb->col= ft.cols;
    psb->y   = ft.y;
    psb->x   = ft.x;
    p = (char *)(psb->raw_memory =
        realloc (psb->raw_memory, ft.rows * ft.cols * (sizeof(ftchar) + sizeof(ftattr))));

    for (y = 0; y < ft.rows; y++)
    {
        memcpy(p, FTCMAP[y], ft.cols * sizeof(ftchar));
        p += ft.cols * sizeof(ftchar);
        memcpy(p, FTAMAP[y], ft.cols * sizeof(ftattr));
        p += ft.cols * sizeof(ftattr);
    }
}

void
scr_free(screen_backup_t *psb)
{
    free(psb->raw_memory);
    psb->raw_memory = NULL;
}

void
scr_restore_free(screen_backup_t *psb)
{
    scr_restore_keep(psb);
    scr_free(psb);
}

void
scr_restore_keep(const screen_backup_t *psb)
{
    int y = 0;
    char *p = NULL;
    int c = ft.cols, r = ft.rows;
    if (!psb || !psb->raw_memory)
        return;

    p = (char *)psb->raw_memory;
    c = ranged(c, 0, psb->col);
    r = ranged(r, 0, psb->row);

    ft.y = ranged(psb->y, 0, ft.rows-1);
    ft.x = ranged(psb->x, 0, ft.cols-1);
    clrscr();

    for (y = 0; y < r; y++)
    {
        memcpy(FTCMAP[y], p, c * sizeof(ftchar));
        p += psb->col * sizeof(ftchar);
        memcpy(FTAMAP[y], p, c * sizeof(ftattr));
        p += psb->col * sizeof(ftattr);
    }

    ft.dirty = 1;
    refresh();
}

void
move_ansi(int y, int x)
{
    move(y, x);
}

void
getyx_ansi(int *y, int *x)
{
    getyx(y, x);
}

void
region_scroll_up(int top, int bottom)
{
    int i;
    ftchar *c0;
    ftattr *a0;

    // logic same with old screen.c
    if (top > bottom) {
        i = top;
        top = bottom;
        bottom = i;
    }
    if (top < 0 || bottom >= ft.rows)
        return;

    c0 = FTCMAP[top];
    a0 = FTAMAP[top];

    for (i = top; i < bottom; i++)
    {
        FTCMAP[i] = FTCMAP[i+1];
        FTAMAP[i] = FTAMAP[i+1];
    }
    FTCMAP[bottom] = c0;
    FTAMAP[bottom] = a0;

    clrregion(bottom, bottom);
    fterm_markdirty();
}

#endif  /* #ifndef PFTERM_TEST_MAIN */

//////////////////////////////////////////////////////////////////////////
// adapter
//////////////////////////////////////////////////////////////////////////

int
fterm_typeahead(void)
{
#if defined (PFTERM_TEST_MAIN) || !(defined (PFTERM_HAVE_VKEY) || defined (PFTERM_EXPOSED_VISIO_VI))
    return 0;
#else
    return vkey_is_typeahead();
#endif
}

void
fterm_rawc(int c)
{
#ifdef PFTERM_TEST_MAIN
    // if (c == ESC_CHR) putchar('*'); else
    putchar(c);
#else
    ochar(c);
#endif
}

void
fterm_rawnewline(void)
{
#ifdef PFTERM_TEST_MAIN
    putchar('\n');
#else
    ochar('\r');
    ochar('\n');
#endif
}

void
fterm_rawflush(void)
{
#ifdef PFTERM_TEST_MAIN
    fflush(stdout);
#else
    oflush();
#endif
}

//////////////////////////////////////////////////////////////////////////
// test
//////////////////////////////////////////////////////////////////////////

#ifdef PFTERM_TEST_MAIN
int main(int argc, char* argv[])
{
    char buf[512];
    initscr();

    if (argc < 2)
    {
#if 0
        // DBCS test
        char *a1 = ANSI_COLOR(1;33) "代刚" ANSI_COLOR(34) "いゅ"
            ANSI_REVERSE "代刚" ANSI_RESET "代刚"
            "代刚a" ANSI_RESET "\n";
        outstr(a1);
        move(0, 2);
        outstr("いゅ1");
        outstr(ANSI_COLOR(1;33)"いゅ2");
        outstr(" い\x85");
        outstr("okok herer\x8a");

        move(0, 8);
        inansistr(buf, sizeof(buf)-1);

        move(3, 5); outs(ANSI_RESET "(From inansistr:) "); outs(buf);
        move(7, 3);
        sprintf(buf, "strlen()=%d\n", fterm_strdlen(a1));
        outstr(buf);
        refresh();
        getchar();

        outs(ANSI_COLOR(1;33) "test " ANSI_COLOR(34) "x"
                ANSI_RESET "te" ANSI_COLOR(43;0;1;35) " st"
                ANSI_RESET "testx\n");
        refresh();
        getchar();

        clear();
        outs("いゅいゅいゅいゅいゅいゅいゅいゅいゅいゅいゅいゅ");
        move(0, 0);
        outs(" this\xFF (ff)is te.(80 tail)->\x80 (80)");
        refresh();
        getchar();
#endif  /* #if 0 */

#if 1
        // test resize
        clear(); move(1, 0); outs("test resize\n");
        doupdate(); getchar();
        // expand X
        resizeterm(25, 200);
        clear(); move(20, 0); clrtoeol(); outs("200x25");
        doupdate(); getchar();
        // resize back
        resizeterm(25, 80);
        clear(); move(20, 0); clrtoeol(); outs("80x25");
        doupdate(); getchar();
        // expand Y
        resizeterm(60, 80);
        clear(); move(20, 0); clrtoeol(); outs("80x60");
        doupdate(); getchar();
        // see if you crash here.
        resizeterm(60, 200);
        clear(); move(20, 0); clrtoeol(); outs("200x60");
        doupdate(); getchar();
#endif

#if 0
        // test optimization
        clear();
        move(1, 0);
        outs("x++ optimization test\n");
        outs("1 2  3   4    5     6      7       8        9         0\n");
        outs("1122233334444455555566666667777777788888888899999999990\n");
        refresh();
        getchar();

        move(2, 0);
        outs("1122233334444455555566666667777777788888888899999999990\n");
        outs("1 2  3   4    5     6      7       8        9         0\n");
        refresh();
        getchar();

        rscroll();
        refresh();
        getchar();
#endif
    } else {
        FILE *fp = fopen(argv[1], "r");
        int c = 0;

        while (fp && (c=getc(fp)) > 0)
        {
            outc(c);
        }
        fclose(fp);
        refresh();
    }

    endwin();
    printf("\ncomplete. enter to exit.\n");
    getchar();
    return 0;
}
#endif // PFTERM_TEST_MAIN

#endif // defined(EXP_PFTERM) || defined(USE_PFTERM)

// vim:ts=4:sw=4:expandtab
