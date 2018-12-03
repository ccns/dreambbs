#ifndef __PFTERM_H__
#define __PFTERM_H__

//////////////////////////////////////////////////////////////////////////
// Flat Terminal API
//////////////////////////////////////////////////////////////////////////

//// common ncurse-like library interface

// initialization
void    initscr     (void);
int     resizeterm  (int rows, int cols);
int     endwin      (void);

// attributes
//ftattr  attrget     (void);
//void    attrset     (ftattr attr);
//void    attrsetfg   (ftattr attr);
//void    attrsetbg   (ftattr attr);

// cursor
void    getyx       (int *y, int *x);
void    getmaxyx    (int *y, int *x);
void    move        (int y, int x);

// clear
void    clear       (void);             // clrscr + move(0, 0)
void    clrtoeol    (void);             // end of line
void    clrtobot    (void);
// clear (non-ncurses)
void    clrtoln     (int ln);           // clear down to ln ( excluding ln, as [y, ln) )
void    clrcurln    (void);             // whole line
void    clrtobeg    (void);             // begin of line
void    clrtohome   (void);
void    clrscr      (void);             // clear and keep cursor untouched
void    clrregion   (int r1, int r2);   // clear [r1, r2], bi-directional.

// window control
void    newwin      (int nlines, int ncols, int y, int x);

// flushing
void    refresh     (void);             // optimized refresh
void    doupdate    (void);             // optimized refresh, ignore input queue
void    redrawwin   (void);             // invalidate whole screen
int     typeahead   (int fd);           // prevent refresh if input queue is not empty

// scrolling
void    scroll      (void);             // scroll up
void    rscroll     (void);             // scroll down
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
void    grayout     (int y, int end, int level);

#endif // __PFTERM_H__
