//////////////////////////////////////////////////////////////////////////
// bbslua environment settings
//////////////////////////////////////////////////////////////////////////

/* Default settings */
#define BBSLUA_HAVE_SYNCNOW              // system needs calling sync API
#define BBSLUA_HAVE_VKEY                 // input system is vkey compatible
#define BBSLUA_HAVE_VTUIKIT              // The BBS has vtuikit
#define BBSLUA_HAVE_OPENCREATE           // The BBS has `OpenCreate` function
#define BBSLUA_NATIVE_T_LINES            // `t_lines` is a global variable
#define BBSLUA_NATIVE_T_COLUMNS          // `t_columns` is a global variable
#define BBSLUA_HAVE_GRAYOUT              // The BBS has `grayout()`
//#define BBSLUA_UNARY_GRAYOUT             // `grayout()` accepts only one argument
#define BBSLUA_HAVE_DOUPDATE             // The BBS has `doupdate()`
#define BBSLUA_HAVE_GETYX                // The BBS has `getyx()`
#define BBSLUA_FORCE_TIME4_T             // The BBS uses 32-bit `time_t` on 64-bit OSs
//#define BBSLUA_HAVE_STR_ANSI             // The BBS uses `str_ansi()` instead of `strip_ansi()`
#define BBSLUA_HAVE_STRLEN_NOANSI        // The BBS has `strlen_noansi()`
//#define BBSLUA_VER_INFO_FILE             // The file with version information for BBS-Lua
//#define BLSCONF_ENABLED                  // Enable `store.*` BBS-Lua API
//#define BBSLUA_EXPOSED_VISIO_VI          // `vi_*` variables of visio input is exposed
//#define BBSLUA_EXPOSED_VISIO_IDLE        // Idle handling of visio is exposed

#include "bbs.h"
#include <sys/time.h>

#ifdef M3_USE_BBSLUA     // For compiling on Maple3
 #include <assert.h>
 #include <stdarg.h>
 #include <sys/file.h>
 #define BBSLUA_HAVE_SYNCNOW
 #undef BBSLUA_HAVE_VKEY
 #undef BBSLUA_HAVE_VTUIKIT
 #undef BBSLUA_HAVE_OPENCREATE
 #undef BBSLUA_NATIVE_T_LINES
 #undef BBSLUA_NATIVE_T_COLUMNS
 #define BBSLUA_HAVE_GRAYOUT
 #undef BBSLUA_UNARY_GRAYOUT
 #undef BBSLUA_HAVE_DOUPDATE
 #define BBSLUA_HAVE_GETYX
 #undef BBSLUA_FORCE_TIME4_T
 #define BBSLUA_HAVE_STR_ANSI
 #undef BBSLUA_HAVE_STRLEN_NOANSI
 #define BBSLUA_VER_INFO_FILE "bbs_script.h"
 #undef BLSCONF_ENABLED
 #define BBSLUA_EXPOSED_VISIO_VI
 #define BBSLUA_EXPOSED_VISIO_IDLE
#endif //M3_USE_BBSLUA

#ifdef PTT_USE_BBSLUA    // For compiling on PttBBS
 #define BBSLUA_HAVE_SYNCNOW
 #define BBSLUA_HAVE_VKEY
 #define BBSLUA_HAVE_VTUIKIT
 #define BBSLUA_HAVE_OPENCREATE
 #define BBSLUA_NATIVE_T_LINES
 #define BBSLUA_NATIVE_T_COLUMNS
 #define BBSLUA_HAVE_GRAYOUT
 #undef BBSLUA_UNARY_GRAYOUT
 #define BBSLUA_HAVE_DOUPDATE
 #define BBSLUA_HAVE_GETYX
 #define BBSLUA_FORCE_TIME4_T
 #undef BBSLUA_HAVE_STR_ANSI
 #define BBSLUA_HAVE_STRLEN_NOANSI
 #undef BBSLUA_VER_INFO_FILE
 #undef BLSCONF_ENABLED
 #undef BBSLUA_EXPOSED_VISIO_VI
 #undef BBSLUA_EXPOSED_VISIO_IDLE
#endif //PTT_USE_BBSLUA

/* Inferred settings */

#ifdef USE_NIOS_VKEY
# define BBSLUA_HAVE_VKEY
#endif

#ifdef USE_PFTERM
# undef BBSLUA_UNARY_GRAYOUT
# define BBSLUA_HAVE_DOUPDATE
# define BBSLUA_HAVE_GETYX
#endif

#if defined(GRAYOUT) || defined(HAVE_GRAYOUT)
# define BBSLUA_HAVE_GRAYOUT
#endif

#ifdef BBSLUA_VER_INFO_FILE
# include BBSLUA_VER_INFO_FILE
#endif

#if !defined(BBSLUA_HAVE_VKEY) && defined(BBSLUA_EXPOSED_VISIO_VI)
# include <arpa/telnet.h>
#endif
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
#define ANSI_IS_PARAM(c) (c == ';' || (c >= '0' && c <= '9'))
#endif // ANSI_IS_PARAM

//////////////////////////////////////////////////////////////////////////
// grayout advanced control
// #include "grayout.h"
//////////////////////////////////////////////////////////////////////////
#ifndef GRAYOUT_DARK
#define GRAYOUT_COLORBOLD (-2)
#define GRAYOUT_BOLD (-1)
#define GRAYOUT_DARK (0)
#define GRAYOUT_NORM (1)
#define GRAYOUT_COLORNORM (+2)
#endif // GRAYOUT_DARK

//////////////////////////////////////////////////////////////////////////
// Other macros
//////////////////////////////////////////////////////////////////////////

// #include config.h

#ifndef PATHLEN
#define PATHLEN (256)
#endif

#ifndef DEFAULT_FILE_CREATE_PERM
#define DEFAULT_FILE_CREATE_PERM (0644)
#endif

// #include common.h

#ifndef MILLISECONDS
#define MILLISECONDS (1000)  // milliseconds of one second
#endif
//////////////////////////////////////////////////////////////////////////
// External variables
// #include "var.h"
//////////////////////////////////////////////////////////////////////////

#ifdef BBSLUA_HAVE_SYNCNOW
 #ifdef __cplusplus
extern "C" {
 #endif
 #ifdef BBSLUA_FORCE_TIME4_T
extern time4_t now;
 #else
extern time_t now;
 #endif
 #ifdef __cplusplus
}  /* extern "C" */
 #endif
#endif

//////////////////////////////////////////////////////////////////////////
// Redirect macros and functions
//////////////////////////////////////////////////////////////////////////

#if !defined(BBSLUA_NATIVE_T_LINES) && !defined(t_lines)
#define t_lines  (b_lines + 1)
#endif
#if !defined(BBSLUA_NATIVE_T_COLUMNS) && !defined(t_columns)
#define t_columns  (b_cols + 1)
#endif

#ifndef BBSLUA_FORCE_TIME4_T
static inline char *ctime4(const time_t *clock) { return ctime(clock); }
#endif

#ifdef BBSLUA_HAVE_STR_ANSI
static inline void strip_ansi(char *dst, const char *str, int mode)
{
    (void)mode;  // Suppress unused-parameter warning
    str_ansi(dst, str, strlen(str));
}
#endif

#ifndef BBSLUA_HAVE_DOUPDATE
static inline void doupdate(void) { refresh(); }
#endif

#ifndef BBSLUA_HAVE_GETYX
static inline void getyx(int *y, int *x)
{
    *y = cur_row;
    *x = cur_col;
}
#endif

//////////////////////////////////////////////////////////////////////////
// Utility macros and functions
//////////////////////////////////////////////////////////////////////////

#ifndef BBSLUA_HAVE_STRLEN_NOANSI
GCC_PURE static int
strlen_noansi(const char *str)        /* String length after stripping ANSI code */
{
    int count, ch, ansi;

    for (ansi = 0, count = 0; (ch = *str); str++)
    {
        if (ch == '\n')
        {
            break;
        }
        else if (ch == '\x1b')
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
            count++;
        }
    }
    return count;
}
#endif  // #ifndef BBSLUA_HAVE_STRLEN_NOANSI

#if defined(BLSCONF_ENABLED) && !defined(BBSLUA_HAVE_OPENCREATE)
static const char *const str_home_file = "usr/%c/%s/%s";
static void
setuserfile(char *buf, const char *fname)
{
    snprintf(buf, PATHLEN, str_home_file, cuser.userid[0], cuser.userid, fname);
}

static int
OpenCreate(const char *path, int flags)
{
    return open(path, flags | O_CREAT, DEFAULT_FILE_CREATE_PERM);
}
#endif


#ifndef BBSLUA_HAVE_VTUIKIT

#ifndef VBUFLEN
#define VBUFLEN     (ANSILINELEN)
#endif
GCC_CHECK_FORMAT(1, 2) static int
vmsgf(const char *fmt, ...)
{
    char msg[VBUFLEN];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(msg, sizeof(msg), fmt, ap);
    va_end(ap);

    return vmsg(msg);
}

static void
vs_hdr(const char *title)
{
    vs_bar(title);
}

// IID.20190125: To be compatible with PttBBS's BBSLua.
#define BL_NOECHO     0x0
#define BL_DOECHO     0x1
#define BL_LCECHO     0x2
#define BL_NUMECHO    0x4
#define BL_PASSECHO   0x10
#define BL_HIDEECHO   0x20
#define BL_GCARRY     0x8     // Ignored

#define NEW_HIDEECHO  0x8000

#define MACRO_NONZERO(macro)  ((macro-0) != 0)

static int
getdata_str(int line, int col, const char *prompt, char *buf, int len, int echo,
            const char *defaultstr)
{
    int new_echo = 0;

    if (echo == BL_NOECHO)
#if MACRO_NONZERO(VGET_STEALTH_NOECHO)
        new_echo = NEW_HIDEECHO | NOECHO | VGET_STEALTH_NOECHO;
#else
        new_echo = NEW_HIDEECHO | NOECHO;
#endif
    else if (echo == BL_DOECHO)
        new_echo = DOECHO;
    else
    {
        if (echo & BL_LCECHO)
            new_echo |= LCECHO;
#ifdef  NUMECHO   // IID.20190125: Some BBSs do not have `NUMECHO`.
        if (echo & BL_NUMECHO)
            new_echo |= NUMECHO;
#endif

        if (echo & BL_HIDEECHO)
            new_echo
#ifdef  VGET_STRICT_DOECHO
                |= NEW_HIDEECHO | NOECHO
#else
                = NEW_HIDEECHO | NOECHO
#endif
#if MACRO_NONZERO(VGET_STEALTH_NOECHO)
                    | VGET_STEALTH_NOECHO
#endif
                ;
        else if (echo & BL_PASSECHO)
            new_echo
#if defined(PASSECHO) && PASSECHO != NOECHO
                |= PASSECHO;
#else   // IID.20190125: Some BBSs do not have `PASSECHO`,
                      //    but their `NOECHO` acks just like `PASSECHO`.
    #ifdef  VGET_STRICT_DOECHO
                |= NEW_HIDEECHO | NOECHO;
    #else
                = NEW_HIDEECHO | NOECHO;
    #endif
#endif
    }

#if defined(VGET_STRICT_DOECHO) || NOECHO != 0
    // IID.20190224: Unless setting other flags does not imply `DOECHO`,
    if (defaultstr && *defaultstr)
#else  // IID.20190125: `vget()` with `NOECHO` should not have default string.
    if ((new_echo & ~NEW_HIDEECHO) != NOECHO && defaultstr && *defaultstr)
#endif
    {
        strlcpy(buf, defaultstr, len);
        str_ansi(buf, buf, len);
        new_echo |= GCARRY;
    }

#if defined(VGET_STRICT_DOECHO) && MACRO_NONZERO(VGET_BREAKABLE)
    new_echo |= VGET_BREAKABLE;
#endif

    if (new_echo & NEW_HIDEECHO)
    {
        new_echo ^= NEW_HIDEECHO;
#if MACRO_NONZERO(VGET_STRICT_DOECHO) && NOECHO == 0
        if (new_echo != NOECHO)
            new_echo |= VGET_STRICT_DOECHO;
#endif
    }
#if defined(VGET_STRICT_DOECHO) && !MACRO_NONZERO(VGET_STRICT_DOECHO)
    else
        new_echo |= DOECHO;
#endif

    return vget(line, col, prompt, buf, len, new_echo);
}

#ifndef USE_PFTERM
static void
newwin(int nlines, int ncols, int y, int x)
{
    int i, oy, ox;
    getyx(&oy, &ox);

    while (nlines-- > 0)
    {
        move_ansi(y++, x);
        for (i = 0; i < ncols; i++)
            outc(' ');
    }
    move(oy, ox);
}
#endif
#endif  // #ifndef BBSLUA_HAVE_VTUIKIT

//////////////////////////////////////////////////////////////////////////
// Input handling
//////////////////////////////////////////////////////////////////////////

#ifndef BBSLUA_HAVE_VKEY

// IID.20190124: If this BBS does not have the vkey input system,
//                  give it a simplified one.

#ifdef BBSLUA_EXPOSED_VISIO_VI
#ifdef __cplusplus
extern "C" {
#endif
// IID.20190502: Using visio internal variables.
extern const int vi_max;
extern unsigned char vi_pool[];
extern int vi_size;
extern int vi_head;
#ifdef BBSLUA_EXPOSED_VISIO_IDLE
extern int idle;
#endif
#ifdef __cplusplus
}  /* extern "C" */
#endif


static int vin_is_empty(void)
{
    return vi_head >= vi_size;
}

static void vin_clear(void)
{
    vi_head = vi_size = 0;
}

// After reading `VIN_AFTER_HEAD_SIZE()` chars from `vin`,
//    the next char cursor will be at `vi_size`.
#define VIN_AFTER_HEAD_SIZE()  \
    ((vi_size >= vi_head) ? vi_size - vi_head : 0)

// After reading `VIN_AFTER_TAIL_SPACE()` chars into `vin`,
//    the next char cursor will be at `vi_max`.
#define VIN_AFTER_TAIL_SPACE()  \
    (vi_max - vi_size)

#else  // #ifdef BBSLUA_EXPOSED_VISIO_VI

// IID.20190502: If `vi_pool` is not exposed, only peek un`recv()`ed data.
//               This is not used to store inputs, but just to peek.
#define IBUFSIZE  (128)
static unsigned char peek_buf[IBUFSIZE];
static int peek_size;

#ifndef ADD_IO_DEFAULT_TIMEOUT
#define ADD_IO_DEFAULT_TIMEOUT  60
#endif

// Try to empty `vl_pool`; may not be actually emptied
// Use `vin_clear_fd()` beforehand to ensure a complete emptying
static int vin_clear(void)
{
    int max_try = IBUFSIZE;
    int no_break = 1;

    while (max_try-- > 0)
    {
        int ch;
        add_io(0, 0);
        ch = vkey();
        if (ch == Ctrl('C'))
            no_break = 0;
#ifdef I_TIMEOUT
        else if (ch == I_TIMEOUT)
            break;
#endif
    }
    add_io(0, ADD_IO_DEFAULT_TIMEOUT);
    return no_break;
}

#endif  // #ifdef BBSLUA_EXPOSED_VISIO_VI


#ifdef BBSLUA_EXPOSED_VISIO_VI
static int
read_vin(void)
{
    int total_len = 0;
    int len;
    int iac_len;

    unsigned char *vi_tail = vi_pool + vi_size;
    len = recv(0, vi_tail, VIN_AFTER_TAIL_SPACE(), MSG_DONTWAIT);
    if (len > 0)
    {
        vi_size += len;
        iac_len = (*vi_tail == IAC) ? iac_count(vi_tail) : 0;
        if (iac_len >= len)
        {
            vi_size -= len;
            return 0;
        }
        vi_size -= iac_len;

        memmove(vi_tail, vi_tail + iac_len, iac_len);
        total_len += len - iac_len;

#ifdef BBSLUA_EXPOSED_VISIO_IDLE
        if (idle && cutmp)
        {

            cutmp->idle_time = idle = 0;
        }
#ifdef  HAVE_SHOWNUMMSG
        if (cutmp)
            cutmp->num_msg = 0;
#endif
#endif  // #ifdef BBSLUA_EXPOSED_VISIO_IDLE
    }

    if (len == 0 || (len < 0 && !(errno == EINTR || errno == EAGAIN)))
        abort_bbs();

    return total_len;
}
#endif  // #ifdef BBSLUA_EXPOSED_VISIO_VI


static void bl_double2tv(double d, struct timeval *tv);
static double bl_tv2double(const struct timeval *tv);

static int
wait_input(float f, int bIgnoreBuf)
{
    int sel = 0;
    fd_set readfds;
    struct timeval tv, *ptv, torig, tsrc, tdest;
    double ntv;

#ifdef BBSLUA_EXPOSED_VISIO_VI
    if (!bIgnoreBuf && !vin_is_empty())
        return 1;
#else
    (void)bIgnoreBuf;
#endif

    // Check if something already in input queue,
    // determine if ok to break.

    // wait for real user interaction
    FD_ZERO(&readfds);
    FD_SET(0, &readfds);

#ifdef STATINC
    STATINC(STAT_SYSSELECT);
#endif

    if (f >= 0)
    {
        tv.tv_sec = (time_t)f;
        tv.tv_usec = 1000000L * (f - (time_t)f);
        ptv = &tv;
        torig = tv;
    }
    else
    {
        ptv = NULL;
    }

    do {
        gettimeofday(&tsrc, NULL);
        sel = select(1, &readfds, NULL, NULL, ptv);
        gettimeofday(&tdest, NULL);

        // if select() stopped by other interrupt,
        // decrease tv and do it again.
        if (ptv)
        {
            ntv = bl_tv2double(&torig) - (bl_tv2double(&tdest) - bl_tv2double(&tsrc));
            if (ntv < 0)
                sel = 0;
            bl_double2tv(ntv, ptv);
            torig = tv;
        }
    } while (sel < 0 && errno == EINTR);

    // now, maybe something for read (sel > 0)
    // or time out (sel == 0)
    // or weird error (sel < 0)

    // sync clock(now) if timeout.
#ifdef BBSLUA_HAVE_SYNCNOW
    if (sel == 0)
        syncnow();
#endif // BBSLUA_HAVE_SYNCNOW
    return (sel == 0) ? 0 : 1;
}

static void
vin_clear_fd(int fd)
{
    // An arbitrarily large buffer
    unsigned char ignore_buf[4096];

    while (wait_input(fd, 1))
    {
        int len = recv(fd, ignore_buf, sizeof(ignore_buf), MSG_DONTWAIT);

        if (len == 0 || (len < 0 && !(errno == EINTR || errno == EAGAIN)))
            break;
    }
}

static int
vkey_is_ready(void)
{
    return wait_input(0, 0);
}

static int
vkey_is_full(void)
{
#ifdef BBSLUA_EXPOSED_VISIO_VI
    return vi_size >= vi_max;
#else
    // No input buffer used; never be full
    return 0;
#endif
}

static int
vkey_poll(int ms)
{
    if (ms)
        refresh();
    return wait_input(ms / (double)MILLISECONDS, 0);
}

static int
vkey_prefetch(int timeout)
{
    if (wait_input(timeout / (double)MILLISECONDS, 1)
            && !vkey_is_full())
#ifdef BBSLUA_EXPOSED_VISIO_VI
        read_vin();
    return !vin_is_empty();
#else
    // Peek un`recv()`ed data.
        peek_size = recv(0, peek_buf, IBUFSIZE, MSG_PEEK | MSG_DONTWAIT);
    else
        peek_size = 0;
    return peek_size > 0;
#endif
}

static int
vkey_is_prefetched(char c)
{
    unsigned char *res;

    if (c == EOF)
        return 0;

#ifdef BBSLUA_EXPOSED_VISIO_VI
    res = (unsigned char *) memchr(vi_pool + vi_head, c, VIN_AFTER_HEAD_SIZE());
    return res >= vi_pool + vi_head;
#else
    res = (unsigned char *) memchr(peek_buf, c, peek_size);
    return res >= peek_buf;
#endif
}

// IID.20190502: Return `0` if encounter `Ctrl-C` with `vkey()`; return `1` otherwise.
static int
vkey_purge(void)
{
#ifndef BBSLUA_EXPOSED_VISIO_VI
    int no_break = 1;
#else
    // a magic number to set how much data we're going to process
    // before a real purge (may contain telnet protocol)
    int bytes_before_purge = 32;
#endif

#ifdef STATINC
    STATINC(STAT_SYSREADSOCKET);
#endif

#ifdef BBSLUA_EXPOSED_VISIO_VI
    // Process some of the remaining input bytes
    vkey_prefetch(0);
    while (wait_input(0, 0) && bytes_before_purge-- > 0)
    {
        int ch = vkey();
#ifdef I_OTHERDATA
        // we can't deal with I_OTHERDATA...
        if (ch == I_OTHERDATA)
            break;
#endif
    }
    vin_clear_fd(0);
    vin_clear();
    return 1;

#else  // #ifdef BBSLUA_EXPOSED_VISIO_VI

    // Process some of the remaining input bytes
    if (!vin_clear())
        no_break = 0;

    vin_clear_fd(0);
    if (!vin_clear())
        no_break = 0;
    return no_break;
#endif  // #ifdef BBSLUA_EXPOSED_VISIO_VI
}

#endif  // #ifndef BBSLUA_HAVE_VKEY

//////////////////////////////////////////////////////////////////////////
// BBS-Lua Project
//
// Author: Hung-Te Lin(piaip), Jan. 2008.
// <piaip@csie.ntu.edu.tw>
// Create: 2008-01-04 22:02:58
//
// This source is released in MIT License, same as Lua 5.0
// http://www.lua.org/license.html
//
// Copyright 2008 Hung-Te Lin <piaip@csie.ntu.edu.tw>
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
// BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// TODO:
//  BBSLUA 1.0
//  1. add quick key/val conversion [deprecated]
//  2. add key values (UP/DOWN/...) [done]
//  3. remove i/o libraries [done]
//  4. add system break key (Ctrl-C) [done]
//  5. add version string and script tags [done]
//  6. standalone w32 sdk [done]
//  7. syntax highlight in editor [done]
//  8. prevent loadfile, dofile [done]
//  9. provide local storage
//  ?. modify bbs user data (eg, money)
//  ?. os.date(), os.exit(), abort(), os.time()
//  ?. memory free issue in C library level?
//  ?. add digital signature
//
//  BBSLUA 2.0
//  1. 2 people communication
//
//  BBSLUA 3.0
//  1. n people communication
//////////////////////////////////////////////////////////////////////////

// #include "bbs.h"  // Included

#ifdef BLSCONF_ENABLED
  #include "fnv_hash.h"
#endif

// #include <sys/time.h>  // Included

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#ifdef BBSLUA_USE_LUAJIT
  #include <luajit.h>
#endif

//////////////////////////////////////////////////////////////////////////
// CONST DEFINITION
//////////////////////////////////////////////////////////////////////////

#define BBSLUA_INTERFACE_VER    0.119 // (0.201)
#define BBSLUA_SIGNATURE        "--#BBSLUA"

// BBS-Lua script format:
// $BBSLUA_SIGNATURE
// -- Interface: $interface
// -- Title: $title
// -- Notes: $notes
// -- Author: $author <email@domain>
// -- Version: $version
// -- Date: $date
// -- LatestRef: #AID board
//  [... script ...]
// $BBSLUA_SIGNATURE

//////////////////////////////////////////////////////////////////////////
// CONFIGURATION VARIABLES
//////////////////////////////////////////////////////////////////////////
#define BLAPI_PROTO     int

#define BLCONF_BREAK_KEY    Ctrl('C')
#define BLCONF_EXEC_COUNT   (5000)
#define BLCONF_PEEK_TIME    (0.01)
#define BLCONF_KBHIT_TMIN   (BLCONF_PEEK_TIME)
#define BLCONF_KBHIT_TMAX   (60*10)
#define BLCONF_SLEEP_TMIN   (BLCONF_PEEK_TIME)
#define BLCONF_SLEEP_TMAX   (BLCONF_KBHIT_TMAX)
#define BLCONF_U_SECOND     (1000000L)
#define BLCONF_PRINT_TOC_INDEX (2)

#define BLCONF_MMAP_ATTACH
#define BLCONF_CURRENT_USERID   cuser.userid

#ifdef M3_USE_BBSLUA
 #define BLCONF_CURRENT_USERNICK cuser.username
#else
 #define BLCONF_CURRENT_USERNICK cuser.nickname
#endif

// BBS-Lua Storage
enum {
    BLS_INVALID= 0,
    BLS_GLOBAL = 1,
    BLS_USER,
};

// #define BLSCONF_ENABLED
#define BLSCONF_BN_VAL  "global"
#define BLSCONF_USER_VAL    "user"
#define BLSCONF_GMAXSIZE    (16*1024)   // should be aligned to block size
#define BLSCONF_UMAXSIZE    (16*1024)   // should be aligned to block size
#define BLSCONF_GPATH       BBSHOME "/luastore"
#define BLSCONF_UPATH       ".luastore"
#define BLSCONF_PREFIX      "v1_"
#define BLSCONF_MAXIO       32          // prevent bursting system

// #define BBSLUA_USAGE

#ifdef _WIN32
# undef  BLCONF_MMAP_ATTACH
# undef  BLCONF_CURRENT_USERID
# define BLCONF_CURRENT_USERID    "guest"
# undef  BLCONF_CURRENT_USERNICK
# define BLCONF_CURRENT_USERNICK  "測試帳號"
#endif

//////////////////////////////////////////////////////////////////////////
// GLOBAL VARIABLES
//////////////////////////////////////////////////////////////////////////
typedef struct {
    char running;   // prevent re-entrant
    char abort;     // system break key hit
    char iocounter; // prevent bursting i/o
#ifdef BLSCONF_ENABLED
    Fnv32_t storename;  // storage filename
#endif
} BBSLuaRT;

// runtime information
// static
BBSLuaRT blrt = {0};

#ifdef BLSCONF_ENABLED
  #define BL_INIT_RUNTIME()  (void) ( \
      memset(&blrt, 0, sizeof(blrt)), \
      blrt.storename = FNV1_32_INIT \
  )
#else
  #define BL_INIT_RUNTIME()  (void) ( \
      memset(&blrt, 0, sizeof(blrt)) \
  )
#endif

#define BL_END_RUNTIME()  (void) ( \
    memset(&blrt, 0, sizeof(blrt)) \
)

#ifdef BBSLUA_USAGE
static int bbslua_count;
#endif

//////////////////////////////////////////////////////////////////////////
// UTILITIES
//////////////////////////////////////////////////////////////////////////

static void
bl_double2tv(double d, struct timeval *tv)
{
    tv->tv_sec = d;
    tv->tv_usec = (d - tv->tv_sec) * BLCONF_U_SECOND;
}

static double
bl_tv2double(const struct timeval *tv)
{
    double d = tv->tv_sec;
    d += tv->tv_usec / (double)BLCONF_U_SECOND;
    return d;
}

static int
bl_peekbreak(float f)
{
    if (vkey_is_full())
        vkey_purge();

    if (vkey_prefetch(f * MILLISECONDS) &&
        vkey_is_prefetched(BLCONF_BREAK_KEY)) {
        vkey_purge();
        blrt.abort = 1;
        return 1;
    }
    return 0;
}

#if KEY_UP > 0  /* Key values for special keys are negative on some BBSs */
  #define KEY_BIT(v)  (v)
  #define IS_NORMAL_KEY(v) (v < 0x100)
#else
  #define KEY_BIT(v)  ((v > 0) ? v : ~v)
  #define IS_NORMAL_KEY(v) (v > 0 && v < 0x100)
#endif

#ifndef META_BIT
  #ifndef Meta
    // Find other possible macro names
    #if defined(META)
      #define Meta  META
    #elif defined(ALT)
      #define Meta  ALT
    #elif defined(Alt)
      #define Meta  Alt
    #elif defined(ESC)
      #define Meta  ESC
    #elif defined(Esc)
      #define Meta  Esc
    #endif
  #endif
  #ifdef Meta
    // Found a defined macro for Meta key
    // Find the bit representing Meta
    #if Meta != 0  // `Meta` is a nonzero macro value
      #define META_BIT  Meta
    #elif Meta(0) != 0  // `Meta` is a macro function and not a no-op
      #define META_BIT  Meta(0)
    #endif
  #endif
#endif
// Define `IS_META()`
#if !defined(IS_META) && defined(META_BIT)
  #define IS_META(v)  ((KEY_BIT(v) & META_BIT) != 0)
#endif

#ifndef SHIFT_BIT
  #ifndef Shift
    // Find other possible macro names
    #if defined(SHIFT)
      #define Shift  SHIFT
    #endif
  #endif
  #ifdef Shift
    // Found a defined macro for Shift key
    // Find the bit representing Shift
    #if Shift != 0  // `Shift` is a nonzero macro value
      #define SHIFT_BIT  Shift
    #elif Shift(0) != 0  // `Shift` is a macro function and not a no-op
      #define SHIFT_BIT  Shift(0)
    #endif
  #endif
#endif
// Define `IS_SHIFT()`
#if !defined(IS_SHIFT) && defined(SHIFT_BIT)
  #define IS_SHIFT(v)  ((KEY_BIT(v) & SHIFT_BIT) != 0)
#endif

// Define `CTRL_BIT`
#ifndef CTRL_BIT
  #ifndef Ctrl
    // Find other possible macro names
    // IID.20190509: `CTRL` is predefined in system; do not use this one.
    //#if defined(CTRL)     // Defined in "/usr/include/*-*-gnu/sys/ttydefaults.h"
    //  #define Ctrl  CTRL
    //#endif
  #endif
  #ifdef Ctrl
    // Found a defined macro for Ctrl key
    // Find the bit representing Ctrl
    #if Ctrl != 0  // `Ctrl` is a nonzero macro value
      #define CTRL_BIT  Ctrl
      #define CTRL_FLIP
    #elif Ctrl(0) != 0  // `Ctrl` is a macro function which sets bits
      #define CTRL_BIT  Ctrl(0)
      #define CTRL_FLIP
    #elif Ctrl(~0) != ~0  // `Ctrl` is a macro function which unsets bits
      #ifdef SHIFT_BIT      // `Ctrl` may unsets `SHIFT_BIT`
        #define CTRL_BIT  ~ (Ctrl(~0) | SHIFT_BIT)
      #else
        #define CTRL_BIT  ~ Ctrl(~0)
      #endif
      #define CTRL_FLIP ~  // Flip the result
    #endif
  #endif
#endif
// Define `IS_CTRL()`
#if !defined(IS_CTRL) && defined(CTRL_BIT)
  #define IS_CTRL(v)  ((CTRL_FLIP KEY_BIT(v) & CTRL_BIT) != 0)
#endif

static void
bl_k2s(lua_State* L, int v)
{
#if defined(IS_META) || defined(BBSLUA_HAVE_VKEY)
    int is_meta;
#endif
#ifdef IS_SHIFT
    int is_shift;
#endif
#ifdef IS_CTRL
    int is_ctrl;
#endif

    // Find the unmodified key value
#ifdef IS_META
    is_meta = IS_META(v);
    if (is_meta)
        v ^= META_BIT;
#elif defined(BBSLUA_HAVE_VKEY)
    is_meta = v == '\x1b';
    if (is_meta)
        v = KEY_ESC_arg;
#endif
#ifdef IS_SHIFT
    is_shift = !IS_NORMAL_KEY(v) && IS_SHIFT(v);
    if (is_shift)
        v ^= SHIFT_BIT;
#endif
#ifdef IS_CTRL
    is_ctrl = !IS_NORMAL_KEY(v) && IS_CTRL(v);
    if (is_ctrl)
        v ^= CTRL_BIT;
#endif

#if IS_NORMAL_KEY(-1)  /* Negative key values are ignored */
    if (v <= 0)
        lua_pushnil(L);
    else
#endif
    if (v == KEY_TAB)
        lua_pushstring(L, "TAB");
    else if (v == '\b' || v == 0x7F)
        lua_pushstring(L, "BS");
    else if (v == '\n' || v == '\r')
        lua_pushstring(L, "ENTER");
    else if (v < ' ')
        lua_pushfstring(L, "^%c", v-1+'A');
    else if (IS_NORMAL_KEY(v))
        lua_pushfstring(L, "%c", v);
#ifdef KEY_F1
  #if KEY_F1 < KEY_F2
    else if (v >= KEY_F1 && v <= KEY_F12)
        lua_pushfstring(L, "F%d", v - KEY_F1 +1);
  #else  /* Key values for special keys are negative on some BBSs */
    else if (v >= KEY_F12 && v <= KEY_F1)
        lua_pushfstring(L, "F%d", KEY_F1 - v +1);
  #endif
#endif
    else switch (v)
    {
        case KEY_UP:    lua_pushstring(L, "UP");    break;
        case KEY_DOWN:  lua_pushstring(L, "DOWN");  break;
        case KEY_RIGHT: lua_pushstring(L, "RIGHT"); break;
        case KEY_LEFT:  lua_pushstring(L, "LEFT");  break;
#ifdef KEY_STAB
        case KEY_STAB:  lua_pushstring(L, "STAB");  break;
#endif
        case KEY_HOME:  lua_pushstring(L, "HOME");  break;
        case KEY_END:   lua_pushstring(L, "END");   break;
        case KEY_INS:   lua_pushstring(L, "INS");   break;
        case KEY_DEL:   lua_pushstring(L, "DEL");   break;
        case KEY_PGUP:  lua_pushstring(L, "PGUP");  break;
        case KEY_PGDN:  lua_pushstring(L, "PGDN");  break;
        default:        lua_pushnil(L);             break;
    }

#ifdef IS_CTRL
    if (!lua_isnil(L, -1) && is_ctrl)
    {
        lua_pushstring(L, "^");
        lua_insert(L, -2);
        lua_concat(L, 2);
    }
#endif
#ifdef IS_SHIFT
    if (!lua_isnil(L, -1) && is_shift)
    {
        lua_pushstring(L, "SHIFT-");
        lua_insert(L, -2);
        lua_concat(L, 2);
    }
#endif
#if defined(BBSLUA_HAVE_VKEY) || defined(IS_META)
    if (!lua_isnil(L, -1) && is_meta)
    {
        lua_pushstring(L, "ESC-");
        lua_insert(L, -2);
        lua_concat(L, 2);
    }
#endif
}

BLAPI_PROTO
bl_newwin(int rows, int cols, const char *title)
{
    // input: (rows, cols, title)
    int y = 0, x = 0, n = 0;
    int oy = 0, ox = 0;

    if (rows <= 0 || cols <= 0)
        return 0;

    getyx(&oy, &ox);
    // now, draw the window
    newwin(rows, cols, oy, ox);

    if (!title || !*title)
        return 0;

    // draw center-ed title
    n = strlen_noansi(title);
    x = ox + (cols - n)/2;
    y = oy + (rows)/2;
    move(y, x);
    outs(title);

    move(oy, ox);
    return 0;
}

//////////////////////////////////////////////////////////////////////////
// BBSLUA API IMPLEMENTATION
//////////////////////////////////////////////////////////////////////////

BLAPI_PROTO
bl_getyx(lua_State* L)
{
    int y, x;
    getyx(&y, &x);
    lua_pushinteger(L, y);
    lua_pushinteger(L, x);
    return 2;
}

BLAPI_PROTO
bl_getmaxyx(lua_State* L)
{
    lua_pushinteger(L, t_lines);
    lua_pushinteger(L, t_columns);
    return 2;
}

BLAPI_PROTO
bl_move(lua_State* L)
{
    int n = lua_gettop(L);
    int y = 0, x = 0;
    if (n > 0)
        y = lua_tointeger(L, 1);
    if (n > 1)
        x = lua_tointeger(L, 2);
    move_ansi(y, x);
    return 0;
}

BLAPI_PROTO
bl_moverel(lua_State* L)
{
    int n = lua_gettop(L);
    int y = 0, x = 0;
    getyx(&y, &x);
    if (n > 0)
        y += lua_tointeger(L, 1);
    if (n > 1)
        x += lua_tointeger(L, 2);
    move(y, x);
    getyx(&y, &x);
    lua_pushinteger(L, y);
    lua_pushinteger(L, x);
    return 2;
}

BLAPI_PROTO
bl_clear(lua_State* L)
{
    (void)L;  /* to avoid warnings */
    clear();
    return 0;
}

BLAPI_PROTO
bl_clrtoeol(lua_State* L)
{
    (void)L;  /* to avoid warnings */
    clrtoeol();
    return 0;
}

BLAPI_PROTO
bl_clrtobot(lua_State* L)
{
    (void)L;  /* to avoid warnings */
    clrtobot();
    return 0;
}

BLAPI_PROTO
bl_refresh(lua_State* L)
{
    (void)L;  /* to avoid warnings */
    // refresh();
    // Seems like that most people don't understand the relationship
    // between refresh() and input queue, so let's force update here.
    doupdate();
    return 0;
}

BLAPI_PROTO
bl_addstr(lua_State* L)
{
    int n = lua_gettop(L);
    int i = 1;
    for (i = 1; i <= n; i++)
    {
        const char *s = lua_tostring(L, i);
        if (s)
            outs(s);
    }
    return 0;
}

BLAPI_PROTO
bl_rect(lua_State *L)
{
    // input: (rows, cols, title)
    int rows = 1, cols = 1;
    int n = lua_gettop(L);
    const char *title = NULL;

    if (n > 0)
        rows = lua_tointeger(L, 1);
    if (n > 1)
        cols = lua_tointeger(L, 2);
    if (n > 2)
        title = lua_tostring(L, 3);
    if (rows <= 0 || cols <= 0)
        return 0;

    // now, draw the rectangle
    bl_newwin(rows, cols, title);
    return 0;
}

BLAPI_PROTO
bl_print(lua_State* L)
{
    bl_addstr(L);
    outc('\n');
    return 0;
}

BLAPI_PROTO
bl_getch(lua_State* L)
{
    int c = vkey();
    if (c == BLCONF_BREAK_KEY)
    {
        vkey_purge();
        blrt.abort = 1;
        return lua_yield(L, 0);
    }
    bl_k2s(L, c);
    return 1;
}


BLAPI_PROTO
bl_getstr(lua_State* L)
{
    int y, x;
    // TODO not using fixed length here?
    char buf[PATHLEN] = "";
    int len = 2, echo = 1;
    int n = lua_gettop(L);
    const char *pmsg = NULL;

    if (n > 0)
        len = lua_tointeger(L, 1);
    if (n > 1)
        echo = lua_tointeger(L, 2);
    if (n > 2)
        pmsg = lua_tostring(L, 3);

#ifdef BBSLUA_HAVE_VKEY
    // Avoid assertion failure
    if (echo & BL_GCARRY)
        echo ^= BL_GCARRY;
#endif

    if (len < 2)
        len = 2;
    if (len >= (int)sizeof(buf))
        len = sizeof(buf) - 1;
    /*
     * this part now done in getdata_str
    if (pmsg && *pmsg)
    {
        strlcpy(buf, pmsg, sizeof(buf));
    }
    */

    // TODO process Ctrl-C here
    getyx(&y, &x);
    if (!pmsg) pmsg = "";
    len = getdata_str(y, x, NULL, buf, len, echo, pmsg);
    if (len <= 0)
    {
#ifdef VGET_EXIT_BREAK
        int res = len;
#endif
        len = 0;

#ifdef VGET_EXIT_BREAK
        if (res == VGET_EXIT_BREAK)
#else
        // check if we got Ctrl-C? (workaround in getdata)
        // TODO someday write 'ungetch()' in io.c to prevent
        // such workaround.
        if (buf[1] == Ctrl('C'))
#endif
        {
            vkey_purge();
            blrt.abort = 1;
            return lua_yield(L, 0);
        }
        lua_pushstring(L, "");
    }
    else
    {
        lua_pushstring(L, buf);
    }
    // return len ? 1 : 0;
    return 1;
}

BLAPI_PROTO
bl_kbhit(lua_State *L)
{
    int n = lua_gettop(L);
    double f = 0.1f;

    if (n > 0)
        f = (double)lua_tonumber(L, 1);

    if (f < BLCONF_KBHIT_TMIN) f = BLCONF_KBHIT_TMIN;
    if (f > BLCONF_KBHIT_TMAX) f = BLCONF_KBHIT_TMAX;

    if (vkey_is_ready() || vkey_poll(f * MILLISECONDS))
        lua_pushboolean(L, 1);
    else
        lua_pushboolean(L, 0);
    return 1;
}

BLAPI_PROTO
bl_kbreset(lua_State *L)
{
    // peek input queue first!
    if (bl_peekbreak(BLCONF_PEEK_TIME))
        return lua_yield(L, 0);

#if defined(BBSLUA_HAVE_VKEY) || defined(BBSLUA_EXPOSED_VISIO_VI)
    vkey_purge();
#else
    // `recv()`ed bytes/keys have not being checked; check purged keys
    if (!vkey_purge())
    {
        blrt.abort = 1;
        return lua_yield(L, 0);
    }
#endif

    return 0;
}

BLAPI_PROTO
bl_sleep(lua_State *L)
{
    int n = lua_gettop(L);
    double us = 0, nus = 0;

    // update screen first.
    bl_refresh(L);

    if (n > 0)
        us = lua_tonumber(L, 1);
    if (us < BLCONF_SLEEP_TMIN) us = BLCONF_SLEEP_TMIN;
    if (us > BLCONF_SLEEP_TMAX) us = BLCONF_SLEEP_TMAX;
    nus = us;

#ifdef _WIN32

    Sleep(us * 1000);

#else // !_WIN32
    {
        struct timeval tp, tdest;

        gettimeofday(&tp, NULL);

        // nus is the destination time
        nus = bl_tv2double(&tp) + us;
        bl_double2tv(nus, &tdest);

        while ( (tp.tv_sec < tdest.tv_sec) ||
                ((tp.tv_sec == tdest.tv_sec) && (tp.tv_usec < tdest.tv_usec)))
        {
            // calculate new peek time
            us = nus - bl_tv2double(&tp);

            // check if input key is system break key.
            if (bl_peekbreak(us))
                return lua_yield(L, 0);

            // check time
            gettimeofday(&tp, NULL);
        }
    }
#endif // !_WIN32

    return 0;
}

BLAPI_PROTO
bl_kball(lua_State *L)
{
    // first, sleep by given seconds
    int r = 0, i = 0;

    r = bl_sleep(L);
    if (blrt.abort)
        return r;

    // pop all arguments
    lua_pop(L, lua_gettop(L));


#ifdef _WIN32
    while (i < LUA_MINSTACK && peekch(0))
    {
        bl_k2s(L, vkey());
        i++;
    }
#else
#if defined(BBSLUA_HAVE_VKEY) || defined(BBSLUA_EXPOSED_VISIO_VI)
    while (i < LUA_MINSTACK && vkey_is_ready()) {
        bl_k2s(L, vkey());
        i++;
    }
#else
    while (i < LUA_MINSTACK)
    {
        int ch;
        add_io(0, 0);
        ch = vkey();
        // Handle `Ctrl-C`s from `vi_pool`.
        if (ch == BLCONF_BREAK_KEY)
        {
            vkey_purge();
            blrt.abort = 1;
            return lua_yield(L, 0);
        }
#ifdef I_TIMEOUT
        if (ch == I_TIMEOUT)
            break;
#endif
        bl_k2s(L, ch);
        i++;
    }
    add_io(0, ADD_IO_DEFAULT_TIMEOUT);
#endif  // #if defined(BBSLUA_HAVE_VKEY) || defined(BBSLUA_EXPOSED_VISIO_VI)
#endif  // #ifdef _WIN32

    return i;
}


BLAPI_PROTO
bl_pause(lua_State* L)
{
    int n = lua_gettop(L);
    const char *s = NULL;
    if (n > 0)
        s = lua_tostring(L, 1);

    n = vmsg(s);
    if (n == BLCONF_BREAK_KEY)
    {
        vkey_purge();
        blrt.abort = 1;
        return lua_yield(L, 0);
    }
    bl_k2s(L, n);
    return 1;
}

BLAPI_PROTO
bl_title(lua_State* L)
{
    int n = lua_gettop(L);
    const char *s = NULL;
    if (n > 0)
        s = lua_tostring(L, 1);
    if (s == NULL)
        return 0;

    vs_hdr(s);
    return 0;
}

BLAPI_PROTO
bl_ansi_color(lua_State *L)
{
    char buf[PATHLEN] = ESC_STR "[";
    char *p = buf + strlen(buf);
    int i = 1;
    int n = lua_gettop(L);
    if (n >= 10) n = 10;
    for (i = 1; i <= n; i++)
    {
        if (i > 1) *p++ = ';';
        sprintf(p, "%d", (int)lua_tointeger(L, i));
        p += strlen(p);
    }
    *p++ = 'm';
    *p   = 0;
    lua_pushstring(L, buf);
    return 1;
}

BLAPI_PROTO
bl_strip_ansi(lua_State *L)
{
    int n = lua_gettop(L);
    const char *s = NULL;
    char *s2 = NULL;
    size_t os2 = 0;

    if (n < 1 || (s = lua_tostring(L, 1)) == NULL ||
            *s == 0)
    {
        lua_pushstring(L, "");
        return 1;
    }

    os2 = strlen(s)+1;
    s2 = (char*) lua_newuserdata(L, os2);
    strip_ansi(s2, s, STRIP_ALL);
    lua_pushstring(L, s2);
    lua_remove(L, -2);
    return 1;
}

BLAPI_PROTO
bl_attrset(lua_State *L)
{
    char buf[PATHLEN] = ESC_STR "[";
    char *p = buf + strlen(buf);
    int i = 1;
    int n = lua_gettop(L);
    if (n == 0)
        outs(ANSI_RESET);
    for (i = 1; i <= n; i++)
    {
        sprintf(p, "%dm", (int)lua_tointeger(L, i));
        outs(buf);
    }
    return 0;
}

BLAPI_PROTO
bl_time(lua_State *L)
{
#ifdef BBSLUA_HAVE_SYNCNOW
    syncnow();
#else
    time_t now = time(NULL);
#endif
    lua_pushinteger(L, now);
    return 1;
}

BLAPI_PROTO
bl_ctime(lua_State *L)
{
#ifdef BBSLUA_HAVE_SYNCNOW
    syncnow();
#else
    time_t now = time(NULL);
#endif
    lua_pushstring(L, ctime4(&now));
    return 1;
}

BLAPI_PROTO
bl_clock(lua_State *L)
{
    double d = 0;

#ifdef _WIN32

    // XXX this is a fast hack because we don't want to do 64bit calculation.
    SYSTEMTIME st;
    GetSystemTime(&st);
#ifdef BBSLUA_HAVE_SYNCNOW
    syncnow();
    // XXX the may be some latency between our GetSystemTime and syncnow.
    // So build again the "second" part.
    d = (int)((now / 60) * 60);
#else
    d = (int)((time(NULL) / 60) * 60);
#endif
    d += st.wSecond;
    d += (st.wMilliseconds / 1000.0f);

#else // !_WIN32

    struct timeval tp;
    gettimeofday(&tp, NULL);
    d = bl_tv2double(&tp);

#endif // !_WIN32

    lua_pushnumber(L, d);
    return 1;
}

//////////////////////////////////////////////////////////////////////////
// BBS-Lua Storage System
//////////////////////////////////////////////////////////////////////////
#ifdef BLSCONF_ENABLED
static int
bls_getcat(const char *s)
{
    if (!s || !*s)
        return BLS_INVALID;
    if (strcmp(s,       BLSCONF_BN_VAL) == 0)
        return          BLS_GLOBAL;
    else if (strcmp(s,  BLSCONF_USER_VAL) == 0)
        return          BLS_USER;
    return BLS_INVALID;
}

static int
bls_getlimit(const char *p)
{
    switch (bls_getcat(p))
    {
        case BLS_GLOBAL:
            return  BLSCONF_GMAXSIZE;
        case BLS_USER:
            return  BLSCONF_UMAXSIZE;
    }
    return 0;
}

static int
bls_setfn(char *fn, size_t sz, const char *p)
{
    *fn = 0;
    switch (bls_getcat(p))
    {
        case BLS_GLOBAL:
            snprintf(fn, sz, "%s/" BLSCONF_PREFIX "U%08x",
                    BLSCONF_GPATH, blrt.storename);
            return  1;

        case BLS_USER:
            setuserfile(fn, BLSCONF_UPATH);
#ifndef DEFAULT_FOLDER_CREATE_PERM
#define DEFAULT_FOLDER_CREATE_PERM (0755)
#endif
            mkdir(fn, DEFAULT_FOLDER_CREATE_PERM);
            assert(strlen(fn) +8 <= sz);
            snprintf(fn + strlen(fn),
                    sz - strlen(fn),
                    "/" BLSCONF_PREFIX "G%08x", blrt.storename);
            return  1;
    }
    return 0;
}

BLAPI_PROTO
bls_iolimit(lua_State *L)
{
    lua_pushinteger(L, BLSCONF_MAXIO);
    return 1;
}

BLAPI_PROTO
bls_limit(lua_State *L)
{
    int n = lua_gettop(L);
    const char *s = NULL;
    if (n > 0)
        s = lua_tostring(L, 1);
    lua_pushinteger(L, bls_getlimit(s));
    return 1;
}

BLAPI_PROTO
bls_load(lua_State *L)
{
    int n = lua_gettop(L);
    const char *cat = NULL;
    char fn[PATHLEN];

    if (blrt.iocounter >= BLSCONF_MAXIO)
    {
        lua_pushnil(L);
        return 1;
    }

    if (n != 1)
    {
        lua_pushnil(L);
        return 1;
    }
    cat = lua_tostring(L, 1); // category
    if (!cat)
    {
        lua_pushnil(L);
        return 1;
    }

    blrt.iocounter++;
    // read file!
    if (bls_setfn(fn, sizeof(fn), cat))
    {
        int fd = open(fn, O_RDONLY);
        if (fd >= 0)
        {
            char buf[2048];
            luaL_Buffer b;

            luaL_buffinit(L, &b);
            while ((n = read(fd, buf, sizeof(buf))) > 0)
                luaL_addlstring(&b, buf, n);
            close(fd);
            luaL_pushresult(&b);
        } else {
            lua_pushnil(L);
        }
    } else {
            lua_pushnil(L);
    }
    return 1;
}

BLAPI_PROTO
bls_save(lua_State *L)
{
    int n = lua_gettop(L), fd = -1;
    int limit = 0, slen = 0;
    const char *s = NULL, *cat = NULL;
    char fn[PATHLEN] = "", ret = 0;

    if (blrt.iocounter >= BLSCONF_MAXIO)
    {
        lua_pushboolean(L, 0);
        return 1;
    }

    if (n != 2) { lua_pushboolean(L, 0); return 1; }

    cat = lua_tostring(L, 1); // category
    s   = lua_tostring(L, 2); // data
    limit = bls_getlimit(cat);

    if (!cat || !s || limit < 1)
    {
        lua_pushboolean(L, 0);
        return 1;
    }

    slen = lua_objlen(L, 2);
    if (slen >= limit) slen = limit;

    blrt.iocounter++;
    // write file!
    if (bls_setfn(fn, sizeof(fn), cat))
    {
        fd = OpenCreate(fn, O_WRONLY);
        if (fd >= 0)
        {
            write(fd, s, slen);
            close(fd);
            ret = 1;
        }
    }

    lua_pushboolean(L, ret);
    return 1;
}
#endif  // #ifdef BLSCONF_ENABLED

//////////////////////////////////////////////////////////////////////////
// BBSLUA LIBRARY
//////////////////////////////////////////////////////////////////////////

static const struct luaL_Reg lib_bbslua [] = {
    /* curses output */
    { "getyx",      bl_getyx },
    { "getmaxyx",   bl_getmaxyx },
    { "move",       bl_move },
    { "moverel",    bl_moverel },
    { "clear",      bl_clear },
    { "clrtoeol",   bl_clrtoeol },
    { "clrtobot",   bl_clrtobot },
    { "refresh",    bl_refresh },
    { "addstr",     bl_addstr },
    { "outs",       bl_addstr },
    { "print",      bl_print },
    /* input */
    { "getch",      bl_getch },
    { "getdata",    bl_getstr },
    { "getstr",     bl_getstr },
    { "kbhit",      bl_kbhit },
    { "kbreset",    bl_kbreset },
    { "kball",      bl_kball },
    /* advanced output */
    { "rect",       bl_rect },
    /* BBS utilities */
    { "pause",      bl_pause },
    { "title",      bl_title },
    /* time */
    { "time",       bl_time },
    { "now",        bl_time },
    { "clock",      bl_clock },
    { "ctime",      bl_ctime },
    { "sleep",      bl_sleep },
    /* ANSI helpers */
    { "ANSI_COLOR", bl_ansi_color },
    { "color",      bl_attrset },
    { "attrset",    bl_attrset },
    { "strip_ansi", bl_strip_ansi },
    { NULL, NULL },
};

#ifdef BLSCONF_ENABLED
static const struct luaL_Reg lib_store [] = {
    { "load",       bls_load },
    { "save",       bls_save },
    { "limit",      bls_limit },
    { "iolimit",    bls_iolimit },
    { NULL, NULL },
};
#endif

#ifndef BBSLUA_USE_LUAJIT
// non-standard modules in bbsluaext.c
LUALIB_API int luaopen_bit (lua_State *L);
#endif

static const luaL_Reg bbslualibs[] = {
  // standard modules
  {"", luaopen_base},

  // {LUA_LOADLIBNAME, luaopen_package},
  {LUA_TABLIBNAME, luaopen_table},
  // {LUA_IOLIBNAME, luaopen_io},
  // {LUA_OSLIBNAME, luaopen_os},
  {LUA_STRLIBNAME, luaopen_string},
  {LUA_MATHLIBNAME, luaopen_math},
  // {LUA_DBLIBNAME, luaopen_debug},

#ifdef BBSLUA_USE_LUAJIT
  // LuaJIT built-in extension modules
  {LUA_BITLIBNAME, luaopen_bit},
  {LUA_JITLIBNAME, luaopen_jit},
  // {LUA_FFILIBNAME, luaopen_ffi},
#else
  // bbslua-ext modules
  {"bit", luaopen_bit},
#endif

  {NULL, NULL}
};


LUALIB_API void bbsluaL_openlibs (lua_State *L) {
  const luaL_Reg *lib = bbslualibs;
  for (; lib->func; lib++) {
    lua_pushcfunction(L, lib->func);
    lua_pushstring(L, lib->name);
    lua_call(L, 1, 0);
  }
}


// Constant registration

typedef struct bbsluaL_RegStr {
  const char *name;
  const char *val;
} bbsluaL_RegStr;

typedef struct bbsluaL_RegNum {
  const char *name;
  lua_Number val;
} bbsluaL_RegNum;

static const bbsluaL_RegStr bbsluaStrs[] = {
    {"ESC",         ESC_STR},
    {"ANSI_RESET",  ANSI_RESET},
    {"sitename",    BBSNAME},
    {NULL,          NULL},
};

static const  bbsluaL_RegNum bbsluaNums[] = {
    {"interface",   BBSLUA_INTERFACE_VER},
    {NULL,          0},
};

static void
bbsluaRegConst(lua_State *L)
{
    int i = 0;

    // unbind unsafe API
    lua_pushnil(L); lua_setglobal(L, "dofile");
    lua_pushnil(L); lua_setglobal(L, "loadfile");

    // Loading bytecode is dangerous.
    // Lua 5.1 has a vulnerable bytecode verifier.
    // Lua 5.2 even removed the verifier.
    lua_pushnil(L); lua_setglobal(L, "load");
    lua_pushnil(L); lua_setglobal(L, "loadstring");

    // global
    lua_pushcfunction(L, bl_print);
    lua_setglobal(L, "print");

    // bit.*
    // IID.20190224: If using BitOp, make `bit.cast` an alias for `bit.tobit`
    //                  to be compatible with bitlib.
    lua_getglobal(L, "bit");
    lua_getfield(L, -1, "cast");
    if (lua_isnil(L, -1)) {
        lua_getfield(L, -2, "tobit");
        lua_setfield(L, -3, "cast");
    }
    lua_pop(L, 2);

#ifdef BBSLUA_USE_LUAJIT
    // jit.*
    // IID.20190224: The module needs to be loaded to enable JIT compilation,
    //                  but the functions are either unsafe or useless.
    lua_pushnil(L); lua_setglobal(L, "jit");
#endif

    // bbs.*
    lua_getglobal(L, "bbs");
    for (i = 0; bbsluaStrs[i].name; i++)
    {
        lua_pushstring(L, bbsluaStrs[i].val);
        lua_setfield(L, -2, bbsluaStrs[i].name);
    }

    for (i = 0; bbsluaNums[i].name; i++)
    {
        lua_pushnumber(L, bbsluaNums[i].val);
        lua_setfield(L, -2, bbsluaNums[i].name);
    }
    // dynamic info
    lua_pushstring(L, BLCONF_CURRENT_USERID);
    lua_setfield(L, -2, "userid");
    lua_pushstring(L, BLCONF_CURRENT_USERNICK);
    lua_setfield(L, -2, "usernick");

    lua_pop(L, 1);

#ifdef BLSCONF_ENABLED
    // store.*
    lua_getglobal(L, "store");
    lua_pushstring(L, BLSCONF_USER_VAL);
    lua_setfield(L, -2, "USER");
    lua_pushstring(L, BLSCONF_BN_VAL);
    lua_setfield(L, -2, "GLOBAL");
    lua_pop(L, 1);
#endif // BLSCONF_ENABLED
}

//////////////////////////////////////////////////////////////////////////
// BBSLUA ENGINE UTILITIES
//////////////////////////////////////////////////////////////////////////

static void
bbsluaHook(lua_State *L, lua_Debug* ar)
{
    // vmsg("bbslua HOOK!");
    if (blrt.abort)
    {
        vkey_purge();
        lua_yield(L, 0);
        return;
    }

    if (ar->event != LUA_HOOKCOUNT)
        return;
#ifdef BBSLUA_USAGE
    bbslua_count += BLCONF_EXEC_COUNT;
#endif

    // now, peek and check
    if (vkey_is_full())
        vkey_purge();

    // refresh();

    // check if input key is system break key.
    if (bl_peekbreak(BLCONF_PEEK_TIME))
    {
        lua_yield(L, 0);
        return;
    }
}

static char *
bbslua_attach(const char *fpath, size_t *plen)
{
    char *buf = NULL;

#ifdef BLCONF_MMAP_ATTACH
    struct stat st;
    int fd = open(fpath, O_RDONLY);

    *plen = 0;

    if (fd < 0) return buf;
    if (fstat(fd, &st) || ((*plen = st.st_size) < 1) || S_ISDIR(st.st_mode))
    {
        close(fd);
        return buf;
    }
    *plen = *plen +1;

    buf = (char *) mmap(NULL, *plen, PROT_READ, MAP_SHARED, fd, 0);
    close(fd);

    if (buf == NULL || buf == MAP_FAILED)
    {
        *plen = 0;
        return  NULL;
    }
    madvise(buf, *plen, MADV_SEQUENTIAL);

#else // !BLCONF_MMAP_ATTACH

    FILE *fp = fopen(fpath, "rt");
    *plen = 0;
    if (!fp)
        return NULL;
    fseek(fp, 0, SEEK_END);
    *plen = ftell(fp);
    buf = (char*) malloc (*plen);
    rewind(fp);
    fread(buf, *plen, 1, fp);

#endif // !BLCONF_MMAP_ATTACH

    return buf;
}

static void
bbslua_detach(char *p, int len)
{
    assert(p);

#ifdef BLCONF_MMAP_ATTACH
    munmap(p, len);
#else // !BLCONF_MMAP_ATTACH
    free(p);
#endif // !BLCONF_MMAP_ATTACH
}

int
bbslua_isHeader(const char *ps, const char *pe)
{
    int szsig = strlen(BBSLUA_SIGNATURE);
    if (ps + szsig > pe)
        return 0;
    if (strncmp(ps, BBSLUA_SIGNATURE, szsig) == 0)
        return 1;
    return 0;
}

static int
bbslua_detect_range(char **pbs, char **pbe, int *lineshift)
{
    int szsig = strlen(BBSLUA_SIGNATURE);
    char *be, *ps, *pe;
    int line = 0;

    ps = *pbs;
    be = pe = *pbe;

    // find start
    while (ps + szsig < pe)
    {
        if (strncmp(ps, BBSLUA_SIGNATURE, szsig) == 0)
            break;
        // else, skip to next line
        while (ps + szsig < pe && *ps++ != '\n');
        line++;
    }
    *lineshift = line;

    if (!(ps + szsig < pe))
        return 0;

    *pbs = ps;
    *pbe = be;

    // find tail by SIGNATURE
    pe = ps + 1;
    while (pe + szsig < be)
    {
        if (strncmp(pe, BBSLUA_SIGNATURE, szsig) == 0)
            break;
        // else, skip to next line
        while (pe + szsig < be && *pe++ != '\n');
    }

    if (pe + szsig < be)
    {
        // found sig, end at such line.
        pe--;
        *pbe = pe;
    } else {
        // abort.
        *pbe = NULL;
        *pbs = NULL;
        return 0;
    }

    // prevent trailing zeros
    pe = *pbe;
    while (pe > ps && !*pe)
        pe--;
    *pbe = pe;

    return 1;
}

//////////////////////////////////////////////////////////////////////////
// BBSLUA TOC Processing
//////////////////////////////////////////////////////////////////////////

static const char *const bbsluaTocTags[] =
{
    "interface",
    "latestref",

    // BLCONF_PRINT_TOC_INDEX
    "title",
    "notes",
    "author",
    "version",
    "date",
    NULL
};

static const char *const bbsluaTocPrompts[] =
{
    "界面版本",
    "最新版本",

    // BLCONF_PRINT_TOC_INDEX
    "名稱",
    "說明",
    "作者",
    "版本",
    "日期",
    NULL
};

int
bbslua_load_TOC(lua_State *L, const char *bs, const char *be, char **ppc)
{
    const unsigned char *ps = NULL, *pe = NULL;
    int i = 0;

    lua_newtable(L);
    *ppc = NULL;

    while (bs < be)
    {
        // find stripped line start, end
        ps = pe = (const unsigned char *) bs;
        while (pe < (const unsigned char*)be && *pe != '\n' && *pe != '\r')
            pe ++;
        bs = (const char*)pe+1;
        while (ps < pe && *ps <= ' ') ps++;
        while (pe > ps && *(pe-1) <= ' ') pe--;
        // at least "--"
        if (pe < ps+2)
            break;
        if (*ps++ != '-' || *ps++ != '-')
            break;
        while (ps < pe && *ps <= ' ') ps++;
        // empty entry?
        if (ps >= pe)
            continue;
        // find pattern
        for (i = 0; bbsluaTocTags[i]; i++)
        {
            int l = strlen(bbsluaTocTags[i]);
            if (ps + l > pe)
                continue;
            if (strncasecmp((const char*)ps, bbsluaTocTags[i], l) != 0)
                continue;
            ps += l;
            // found matching pattern, now find value
            while (ps < pe && *ps <= ' ') ps++;
            if (ps >= pe || *ps++ != ':')
                break;
            while (ps < pe && *ps <= ' ') ps++;
            // finally, (ps, pe) is the value!
            if (ps >= pe)
                break;

            lua_pushlstring(L, (const char*)ps, pe-ps);

            // accept only real floats for interface ([0])
            if (i == 0 && lua_tonumber(L, -1) <= 0)
            {
                lua_pop(L, 1);
            }
            else
            {
                lua_setfield(L, -2, bbsluaTocTags[i]);
            }
            break;
        }
    }

    if (ps >= (const unsigned char*)bs && ps < (const unsigned char*)be)
        *ppc = (char*)ps;
    lua_setglobal(L, "toc");
    return 0;
}

static void
fullmsg(const char *s)
{
    clrtoeol();
    prints("%-*.*s\n", t_columns-1, t_columns-1, s);
}

void
bbslua_logo(lua_State *L)
{
    int y, by = b_lines -1; // print - back from bottom
    int i = 0;
    double tocinterface = 0;
    int tocs = 0;
    char msg[STRLEN];

    // get toc information
    lua_getglobal(L, "toc");
    lua_getfield(L, -1, bbsluaTocTags[0]);
    tocinterface = lua_tonumber(L, -1); lua_pop(L, 1);

    // query print-able toc tags
    for (i = BLCONF_PRINT_TOC_INDEX; bbsluaTocTags[i]; i++)
    {
        lua_getfield(L, -1, bbsluaTocTags[i]);
        if (lua_tostring(L, -1))
            tocs++;
        lua_pop(L, 1);
    }

    // prepare logo window
#ifdef BBSLUA_HAVE_GRAYOUT
 #ifdef BBSLUA_UNARY_GRAYOUT
    grayout(GRAYOUT_DARK);
 #else
    grayout(0, b_lines, GRAYOUT_DARK);
 #endif
#endif

    // print compatibility test
    // now (by) is the base of new information
    if (tocinterface == 0)
    {
        by -= 4; y = by+2;
        move(y-1, 0);
        outs(ANSI_COLOR(0;31;47));
        fullmsg("");
        fullmsg(" ▲ 此程式缺少相容性資訊，您可能無法正常執行");
        fullmsg("    若執行出現錯誤，請向原作者取得新版");
        fullmsg("");
    }
    else if (tocinterface > BBSLUA_INTERFACE_VER)
    {
        by -= 4; y = by+2;
        move(y-1, 0);
        outs(ANSI_COLOR(0;1;37;41));
        fullmsg("");
        snprintf(msg, sizeof(msg),
                " ▲ 此程式使用新版的 BBS-Lua 規格 (%0.3f)，您可能無法正常執行",
                tocinterface);
        fullmsg(msg);
        fullmsg("   若執行出現錯誤，建議您重新登入 BBS 後再重試");
        fullmsg("");
    }
    else if (tocinterface == BBSLUA_INTERFACE_VER)
    {
        // do nothing!
    }
    else
    {
        // should be compatible
        // prints("相容 (%.03f)", tocinterface);
    }

    // print toc, if any.
    if (tocs)
    {
        y = by - 1 - tocs;
        by = y-1;

        move(y, 0); outs(ANSI_COLOR(0;1;30;47));
        fullmsg("");

        // now try to print all TOC infos
        for (i = BLCONF_PRINT_TOC_INDEX; bbsluaTocTags[i]; i++)
        {
            lua_getfield(L, -1, bbsluaTocTags[i]);
            if (!lua_isstring(L, -1))
            {
                lua_pop(L, 1);
                continue;
            }
            move(++y, 0);
            snprintf(msg, sizeof(msg), "  %s: %-.*s",
                    bbsluaTocPrompts[i], STRLEN-12, lua_tostring(L, -1));
            msg[sizeof(msg)-1] = 0;
            fullmsg(msg);
            lua_pop(L, 1);
        }
        fullmsg("");
    }

    // print caption
    move(by-2, 0); outc('\n');
    outs(ANSI_COLOR(0;1;37;44));
    snprintf(msg, sizeof(msg),
            " ■ BBS-Lua %.03f  (Build " __DATE__ " " __TIME__") ",
            (double)BBSLUA_INTERFACE_VER);
    fullmsg(msg);

    // system break key prompt
    {
        int sz = t_columns -1;
        const char
            *const prompt1 = "    提醒您執行中隨時可按 ",
            *const prompt2 = "[Ctrl-C]",
            *const prompt3 = " 強制中斷 BBS-Lua 程式";
        sz -= strlen(prompt1);
        sz -= strlen(prompt2);
        sz -= strlen(prompt3);
        outs(ANSI_COLOR(22;37));  outs(prompt1);
        outs(ANSI_COLOR(1;31));   outs(prompt2);
        outs(ANSI_COLOR(0;37;44));outs(prompt3);
        prints("%*s", sz, "");
        outs(ANSI_RESET);
    }
    lua_pop(L, 1);
}

//////////////////////////////////////////////////////////////////////////
// BBSLUA Script Loader
//////////////////////////////////////////////////////////////////////////

typedef struct LoadS {
    const char *s;
    size_t size;
    int lineshift;
} LoadS;

static const char* bbslua_reader(lua_State *L, void *ud, size_t *size)
{
    LoadS *ls = (LoadS *)ud;
    (void)L;
    if (ls->size == 0) return NULL;
    if (ls->lineshift > 0) {
        const char *const linefeed = "\n";
        *size = 1;
        ls->lineshift--;
        return linefeed;
    }
    *size = ls->size;
    ls->size = 0;
    return ls->s;
}

static int bbslua_loadbuffer (lua_State *L, const char *buff, size_t size,
        const char *name, int lineshift) {
    LoadS ls;
    ls.s = buff;
    ls.size = size;
    ls.lineshift = lineshift;
    return lua_load(L, bbslua_reader, &ls, name);
}

typedef struct AllocData {
    size_t alloc_size;
    size_t max_alloc_size;
    size_t alloc_limit;
} AllocData;

static void alloc_init(AllocData *ad)
{
    memset(ad, 0, sizeof(*ad));
    // real limit is not determined yet, just assign a big value
    ad->alloc_limit = 20*1048576;
}

static void *allocf (void *ud, void *ptr, size_t osize, size_t nsize) {
    /* TODO use our own allocator, for better memory control, avoid fragment and leak */
    AllocData *ad = (AllocData*)ud;

    if (ad->alloc_size + nsize - osize > ad->alloc_limit) {
        return NULL;
    }
    ad->alloc_size += nsize - osize;
    if (ad->alloc_size > ad->max_alloc_size) {
        ad->max_alloc_size = ad->alloc_size;
    }
    if (nsize == 0) {
        free(ptr);
        return NULL;
    }
    else
        return realloc(ptr, nsize);
}
static int panic (lua_State *L) {
    (void)L;  /* to avoid warnings */
    fprintf(stderr, "PANIC: unprotected error in call to Lua API (%s)\n",
            lua_tostring(L, -1));
    return 0;
}

void bbslua_loadLatest(lua_State *L,
        char **pbs, char **pps, char **ppe, char **ppc, int *psz,
        int *plineshift, char *bfpath, const char **pfpath)
{
    int r = 1; // max redirect for one article only.

    do {
        char *xbs = NULL, *xps = NULL, *xpe = NULL, *xpc = NULL;
        int xlineshift = 0;
        size_t xsz;
#ifdef AID_DISPLAYNAME
        const char *lastref = NULL;
        char loadnext = 0;
#endif
        char isnewver = 0;

        // detect file
        xbs = bbslua_attach(bfpath, &xsz);
        if (!xbs)
            break;

        xps = xbs;
        xpe = xps + xsz;

        if (!bbslua_detect_range(&xps, &xpe, &xlineshift))
        {
            // not detected
            bbslua_detach(xbs, xsz);
            break;
        }

        // detect and verify TOC meta data
        lua_getglobal(L, "toc");    // stack 1
        if (lua_istable(L, -1))
        {
            const char *oref, *otitle, *nref, *ntitle;
            // load and verify tocinfo
            lua_getfield(L, -1, "latestref");   // stack 2
            oref = lua_tostring(L, -1);
            lua_getfield(L, -2, "title");       // stack 3
            otitle = lua_tostring(L, -1);
            bbslua_load_TOC(L, xps, xpe, &xpc);
            lua_getglobal(L, "toc");            // stack 4
            lua_getfield(L, -1, "latestref");   // stack 5
            nref = lua_tostring(L, -1);
            lua_getfield(L, -2, "title");       // stack 6
            ntitle = lua_tostring(L, -1);

            if (oref && nref && otitle && ntitle &&
                    strcmp(oref, nref) == 0 &&
                    strcmp(otitle, ntitle) == 0)
                isnewver = 1;

            // pop all
            lua_pop(L, 5);              // stack = 1 (old toc)
            if (!isnewver)
                lua_setglobal(L, "toc");
            else
                lua_pop(L, 1);
        } else {
            lua_pop(L, 1);
            bbslua_load_TOC(L, xps, xpe, &xpc);
        }

        if (*pbs && !isnewver)
        {
            // different.
            bbslua_detach(xbs, xsz);
            break;
        }

        // now, apply current buffer
        if (*pbs)
        {
            bbslua_detach(*pbs, *psz);
            // XXX fpath=bfpath, supporting only one level redirection now.
            *pfpath = bfpath;
        }
        *pbs = xbs; *pps = xps; *ppe = xpe; *psz = xsz;
        *ppc = xpc;
        *plineshift = xlineshift;

#ifdef AID_DISPLAYNAME
        // quick exit
        if (r <= 0)
            break;

        // LatestRef only works if system supports AID.
        // try to load next reference.
        lua_getglobal(L, "toc");            // stack 1
        lua_getfield(L, -1, "latestref");   // stack 2
        lastref = lua_tostring(L, -1);

        while (lastref && *lastref)
        {
            // try to load by AID
            char bn[IDLEN+1] = "";
            aidu_t aidu = 0;
            unsigned char *p = (unsigned char*)bn;

            if (*lastref == '#') lastref++;
            aidu = aidc2aidu((char*)lastref);
            if (aidu <= 0) break;

            while (*lastref > ' ') lastref ++;              // lastref points to zero of space
            while (*lastref && *lastref <= ' ') lastref++;  // lastref points to zero or board name
            if (*lastref == '(') lastref ++;

            if (!*lastref) break;
            strlcpy(bn, lastref, sizeof(bn));
            // truncate board name
            // (not_alnum(ch) && ch != '_' && ch != '-' && ch != '.')
            while (*p &&
                (isalnum(*p) || *p == '_' || *p == '-' || *p == '.')) p++;
            *p = 0;
            if (bn[0])
            {
                bfpath[0] = 0;
                setaidfile(bfpath, bn, aidu);
            }

            if (bfpath[0])
                loadnext = 1;
            break;
        }
        lua_pop(L, 2);

        if (loadnext) continue;
#endif // AID_DISPLAYNAME
        break;

    } while (r-- > 0);
}

//////////////////////////////////////////////////////////////////////////
// BBSLUA Hash
//////////////////////////////////////////////////////////////////////////

#ifdef BLSCONF_ENABLED
#if 0
static int
bbslua_hashWriter(lua_State *L, const void *p, size_t sz, void *ud)
{
    Fnv32_t *phash = (Fnv32_t*) ud;
    *phash = fnv_32_buf(p, sz, *phash);
    return 0;
}

static Fnv32_t
bbslua_f2hash(lua_State *L)
{
    Fnv32_t fnvseed = FNV1_32_INIT;
    lua_dump(L, bbslua_hashWriter, &fnvseed);
    return fnvseed;
}

static Fnv32_t
bbslua_str2hash(const char *str)
{
    if (!str)
        return 0;
    return fnv_32_str(str, FNV1_32_INIT);
}
#endif

static Fnv32_t
bbslua_path2hash(const char *path)
{
    Fnv32_t seed = FNV1_32_INIT;
    if (!path)
        return 0;
    if (path[0] != '/') // relative, append BBSHOME
        seed = fnv_32_str(BBSHOME "/", seed);
    return fnv_32_str(path, seed);
}
#endif  // #ifdef BLSCONF_ENABLED

//////////////////////////////////////////////////////////////////////////
// BBSLUA Main
//////////////////////////////////////////////////////////////////////////

int
bbslua(const char *fpath)
{
    int r = 0;
    lua_State *L;
    char *bs = NULL, *ps = NULL, *pe = NULL, *pc = NULL;
    char bfpath[PATHLEN] = "";
    int sz = 0;
    int lineshift = 0;
    AllocData ad;
#ifdef BBSLUA_USAGE
    struct rusage rusage_begin, rusage_end;
    struct timeval lua_begintime, lua_endtime;
    gettimeofday(&lua_begintime, NULL);
    getrusage(0, &rusage_begin);
#endif

#ifdef UMODE_BBSLUA
    unsigned int prevmode = getutmpmode();
#endif

    // re-entrant not supported!
    if (blrt.running)
        return 0;

    // initialize runtime
    BL_INIT_RUNTIME();

#ifdef BBSLUA_USAGE
    bbslua_count = 0;
#endif

    // init lua
    alloc_init(&ad);
    L = lua_newstate(allocf, &ad);
    if (!L)
        return 0;
    lua_atpanic(L, &panic);

    strlcpy(bfpath, fpath, sizeof(bfpath));

    // load file
    bbslua_loadLatest(L, &bs, &ps, &pe, &pc, &sz, &lineshift, bfpath, &fpath);

    if (!ps || !pe || ps >= pe)
    {
        if (bs)
            bbslua_detach(bs, sz);
        lua_close(L);
        vmsg("BBS-Lua 載入錯誤: 未含 BBS-Lua 程式");
        return 0;
    }

    // init library
    bbsluaL_openlibs(L);
    luaL_openlib(L,   "bbs",    lib_bbslua, 0);

#ifdef BLSCONF_ENABLED
    luaL_openlib(L,   "store",  lib_store, 0);
#endif // BLSCONF_ENABLED

    bbsluaRegConst(L);

    // load script
    r = bbslua_loadbuffer(L, ps, pe-ps, "BBS-Lua", lineshift);

#ifdef BLSCONF_ENABLED
    // build hash or store name
    blrt.storename = bbslua_path2hash(fpath);
    // vmsgf("BBS-Lua Hash: %08X", blrt.storename);
#endif

    // unmap
    bbslua_detach(bs, sz);

    if (r != 0)
    {
        const char *errmsg = lua_tostring(L, -1);
        outs(ANSI_RESET);
        move(b_lines-3, 0); clrtobot();
        outs("\n");
        outs(errmsg);
        lua_close(L); // delay closing because we need to print out error message
        vmsg("BBS-Lua 載入錯誤: 請通知作者修正程式碼。");
        return 0;
    }

#ifdef UMODE_BBSLUA
    setutmpmode(UMODE_BBSLUA);
#endif

    bbslua_logo(L);
    vmsgf("提醒您執行中隨時可按 [Ctrl-C] 強制中斷 BBS-Lua 程式");

    // ready for running
    clear();
    blrt.running =1;
    lua_sethook(L, bbsluaHook, LUA_MASKCOUNT, BLCONF_EXEC_COUNT);

    refresh();
    // check is now done inside hook
    r = lua_resume(L, 0);

    // even if r == yield, let's abort - you cannot yield main thread.

    if (r != 0 && !blrt.abort)
    {
        const char *errmsg = lua_tostring(L, -1);
        move(b_lines-3, 0); clrtobot();
        outs("\n");
        if (errmsg) outs(errmsg);
    }

    lua_close(L);
    blrt.running =0;
    vkey_purge();
#ifdef BBSLUA_USAGE
    {
        double cputime;
        double load;
        double walltime;
        getrusage(0, &rusage_end);
        gettimeofday(&lua_endtime, NULL);
        cputime = bl_tv2double(&rusage_end.ru_utime) - bl_tv2double(&rusage_begin.ru_utime);
        walltime = bl_tv2double(&lua_endtime) - bl_tv2double(&lua_begintime);
        load = cputime / walltime;
        log_filef("log/bbslua.log", LOG_CREAT,
                "maxalloc=%d leak=%d op=%d cpu=%.3f Mop/s=%.1f load=%f file=%s\n",
                (int)ad.max_alloc_size, (int)ad.alloc_size,
                bbslua_count, cputime, bbslua_count / cputime / 1000000.0, load * 100,
                fpath);
    }
#endif

#ifdef BBSLUA_HAVE_GRAYOUT
 #ifdef BBSLUA_UNARY_GRAYOUT
    // grayout(GRAYOUT_DARK);
 #else
    // grayout(0, b_lines, GRAYOUT_DARK);
 #endif
#endif
    move(b_lines, 0); clrtoeol();
    vmsgf("BBS-Lua 執行結束%s。",
            blrt.abort ? " (使用者中斷)" : r ? " (程式錯誤)" : "");
    BL_END_RUNTIME();
    clear();

#ifdef UMODE_BBSLUA
    setutmpmode(prevmode);
#endif

    return 0;
}

// vim:ts=4:sw=4:expandtab
