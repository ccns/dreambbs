//------------------------------------------------------------------------
// bbsruby environment settings
//------------------------------------------------------------------------

/* Default settings */
#define BBSRUBY_HAVE_VTUIKIT              // The BBS has vtuikit
//#define BBSRUBY_NATIVE_B_LINES            // `b_lines` is a global variable
//#define BBSRUBY_NATIVE_B_COLS             // `b_cols` is a global variable
//#define BBSRUBY_USE_RH_COOR               // Use right-handed coordinate; `x` => row, `y` => col
#define BBSRUBY_HAVE_GETYX                // The BBS has `getyx()`
//#define BBSRUBY_HAVE_STR_RANSI            // The BBS has `str_ransi` variable
//#define BBSRUBY_VER_INFO_FILE             // The file with version information for BBS-Ruby
#define BBSRUBY_HAVE_BMIN_BMAX            // The BBS has macros `BMIN` and `BMAX`
//#define BBSRUBY_USE_MRUBY                 // Use mruby instead of CRuby
//#define BBSRUBY_MRUBY_HAS_FULLCORE_GEMBOX // The mruby library is compiled with the `full-core` gembox

#include "bbs.h"
#include <sys/time.h>

#ifdef M3_USE_BBSRUBY     // For compiling on Maple3
 #undef BBSRUBY_HAVE_VTUIKIT
 #define BBSRUBY_NATIVE_B_LINES
 #define BBSRUBY_NATIVE_B_COLS
 #undef BBSRUBY_USE_RH_COOR
 #define BBSRUBY_HAVE_GETYX
 #define BBSRUBY_HAVE_STR_RANSI
 #define BBSRUBY_VER_INFO_FILE "bbs_script.h"
 //#define BBSRUBY_USE_MRUBY
 //#define BBSRUBY_MRUBY_HAS_FULLCORE_GEMBOX
#endif //M3_USE_BBSRUBY

#ifdef PTT_USE_BBSRUBY    // For compiling on PttBBS
 #define BBSRUBY_HAVE_VTUIKIT
 #undef BBSRUBY_NATIVE_B_LINES
 #undef BBSRUBY_NATIVE_B_COLS
 #undef BBSRUBY_USE_RH_COOR
 #define BBSRUBY_HAVE_GETYX
 #undef BBSRUBY_HAVE_STR_RANSI
 #undef BBSRUBY_VER_INFO_FILE
 //#define BBSRUBY_USE_MRUBY
 //#define BBSRUBY_MRUBY_HAS_FULLCORE_GEMBOX
#endif //PTT_USE_BBSRUBY

/* Inferred settings */

#ifdef USE_PFTERM
# define BBSRUBY_HAVE_GETYX
#endif

#ifdef BBSRUBY_VER_INFO_FILE
# include BBSRUBY_VER_INFO_FILE
#endif

//------------------------------------------------------------------------
// Redirect macros and functions
//------------------------------------------------------------------------

#if !defined(BBSRUBY_NATIVE_B_LINES) && !defined(b_lines)
  #define b_lines  (t_lines - 1)
#endif
#if !defined(BBSRUBY_NATIVE_B_COLS) && !defined(b_cols)
  #define b_cols  (t_columns - 1)
#endif

#ifndef BBSRUBY_HAVE_STR_RANSI
static const char str_ransi[] = "\x1b[m";
#endif

#ifndef COLOR1
  #define COLOR1          "\x1b[34;46m"   /* footer/feeter 的前段顏色 */
#endif
#ifndef COLOR2
  #define COLOR2          "\x1b[31;47m"   /* footer/feeter 的後段顏色 */
#endif

#ifdef BBSRUBY_HAVE_VTUIKIT
  #define vs_bar(title)  vs_hdr(title)
  #define vget(y, x, msg, buf, size, mode)  getdata(y, x, msg, buf, size, mode)
#endif

#ifndef BBSRUBY_HAVE_GETYX
static inline void getyx(int *y, int *x)
{
    *y = cur_row;
    *x = cur_col;
}
#endif

#ifdef BBSRUBY_USE_RH_COOR
  #define BRB_COOR_ROW  "x"
  #define BRB_COOR_COL  "y"
#else
  #define BRB_COOR_ROW  "y"
  #define BRB_COOR_COL  "x"
#endif

#ifndef BBSRUBY_HAVE_BMIN_BMAX          // The BBS has macros `MIN` and `MAX` instead
  #define BMIN(a, b)  MIN(a, b)
  #define BMAX(a, b)  MAX(a, b)
#endif

#ifndef DL_RELEASE_VOID
  #define DL_RELEASE_VOID() ((void)0)
#endif

//-------------------------------------------------------
// BBSRuby.c
//-------------------------------------------------------
// target : Ruby script support for BBS
// author : Zero <itszero at gmail.com>
// Version: 0.2
// create : 2007/01/08
// update : 2007/01/08
// This program is licensed under MIT License.
//-------------------------------------------------------

#include "bbs.h"

#ifdef BBSRUBY_USE_MRUBY
  #include <mruby.h>
  #include <mruby/array.h>
  #include <mruby/compile.h>
  #include <mruby/error.h>
  #include <mruby/hash.h>
  #include <mruby/numeric.h>
  #include <mruby/proc.h>
  #include <mruby/string.h>
  #include <mruby/variable.h>
  #include <mruby/version.h>

  #define BRBCPP_EVAL(...) __VA_ARGS__
  #define RBF_P(paras)  (mrb_state *mrb, mrb_value self)    // `rb_func_t` parameter
  #define RB_P(paras)  (mrb_state *mrb, BRBCPP_EVAL paras)  // parameter
  #define RB_PV(paras)  (mrb_state *mrb)                    // parameter void
  #define RBF_ARG(...)  (mrb, self)                         // `rb_func_t` arguments
  #define RB_ARG(...)  (mrb, __VA_ARGS__)                   // arguments
  #define RB_VARG()  (mrb)                                  // void argument
  #define RBF_C(func)  func RBF_ARG                         // `rb_func_t` call
  #define RB_C(func)  m##func RB_ARG                        // Ruby API call
  #define RB_CV(func)  m##func RB_VARG                      // Ruby API call void
  #define BRB_C(func)  func RB_ARG                          // normal ruby call
  #define BRB_CV(func)  func RB_VARG                        // normal ruby call void
  #define CMRB_C(crbfunc, mrbfunc)  mrbfunc RB_ARG          // (CRuby, mruby) API call
  #define CMRB_CV(crbfunc, mrbfunc)  mrbfunc RB_VARG        // (CRuby, mruby) API call void

typedef mrb_value VALUE;
typedef struct RClass *rb_class_t;
typedef mrb_func_t rb_func_t;

  #define RB_ARGS_REQ(n)  MRB_ARGS_REQ(n)
  #define RB_ARGS_REST()  MRB_ARGS_REST()

  #define rb_cObject  (mrb->object_class)
  #define Qnil  mrb_nil_value()
  #define Qundef  mrb_undef_value()
  #define INT2FIX(i)  mrb_fixnum_value(i)
  #define INT2NUM(i)  mrb_fixnum_value(i)
  #define NUM2INT(o)  mrb_int(mrb, o)
  #define NUM2DBL(o)  mrb_to_flo(mrb, o)
  #define StringValueCStr(v)  mrb_string_value_cstr(mrb, &(v))
  #define RTEST(o)  mrb_test(o)

  #define BRB_HOOK_COUNT_FACTOR  100
#else
  #include <ruby.h>

  #ifndef RUBY_VM
    #include <rubysig.h>
    #include <node.h>
typedef rb_event_t rb_event_flag_t;
  #endif

  #define RBF_P(paras)  paras                 // `rb_func_t` parameter
  #define RB_P(paras)  paras                  // parameter
  #define RB_PV(paras)  (void)                // parameter void
  #define RBF_C(func)  func                   // `rb_func_t` call
  #define RB_C(func)  func                    // Ruby API call
  #define RB_CV(func)  func                   // Ruby API call void
  #define BRB_C(func)  func                   // normal ruby call
  #define BRB_CV(func)  func                  // normal ruby call void
  #define CMRB_C(crbfunc, mrbfunc)  crbfunc   // (CRuby, mruby) API call
  #define CMRB_CV(crbfunc, mrbfunc)  crbfunc  // (CRuby, mruby) API call void

typedef VALUE (*rb_func_t)(ANYARGS);
typedef VALUE rb_class_t;

  #define RB_ARGS_REQ(n)  n
  #define RB_ARGS_REST()  -1

  #define BRB_HOOK_COUNT_FACTOR  1
#endif

#define BBSRUBY_MAJOR_VERSION (0)
#define BBSRUBY_MINOR_VERSION (3)
#ifndef BBSRUBY_PATCH_VERSION
    #define BBSRUBY_PATCH_VERSION 0
#endif
#ifndef BBSRUBY_DL_PATCH_VERSION
    #define BBSRUBY_DL_PATCH_VERSION 0
#endif

#ifndef BBSRUBY_VERSION_STR
  #define BBSRUBY_VERSION_STR "v0.3"
#endif

#define BBSRUBY_VERSION_VALUE (1000000 * BBSRUBY_MAJOR_VERSION + 10000 * BBSRUBY_MINOR_VERSION + 100 * BBSRUBY_PATCH_VERSION + BBSRUBY_DL_PATCH_VERSION)

#define BBSRUBY_SIGNATURE "###BBSRuby"

#define BBSRUBY_INTERFACE_VER 0.111
static int ABORT_BBSRUBY = 0;

#define BBSRUBY_TOC_HEADERS (6)
static const char* const TOCs_HEADER[BBSRUBY_TOC_HEADERS] = {"interface", "title", "notes", "author", "version", "date"};
static VALUE TOCs_rubyhash;
static double KBHIT_TMIN = 0.001;
static double KBHIT_TMAX = 60*10;
static VALUE KB_QUEUE;
static int badxy_compat = 0; // Whether to replicate the behavior of badly confused x/y coordination in v0.3 and before

static int pause_msg(const char *msg, const char *reason, const char *prompt);
static int getkey(double wait);

/* BBS helper class : following BBSLua SDK */
VALUE brb_keyToString RB_P((int key))
{
    if (key == Ctrl('C')) // System-wide Abort
        ABORT_BBSRUBY = 1;

    if (key == KEY_UP)
        return RB_C(rb_str_new_cstr)("UP");
    else if (key == KEY_DOWN)
        return RB_C(rb_str_new_cstr)("DOWN");
    else if (key == KEY_LEFT)
        return RB_C(rb_str_new_cstr)("LEFT");
    else if (key == KEY_RIGHT)
        return RB_C(rb_str_new_cstr)("RIGHT");
    else if (key == KEY_HOME)
        return RB_C(rb_str_new_cstr)("HOME");
    else if (key == KEY_INS)
        return RB_C(rb_str_new_cstr)("INS");
    else if (key == KEY_DEL)
        return RB_C(rb_str_new_cstr)("DEL");
    else if (key == KEY_END)
        return RB_C(rb_str_new_cstr)("END");
    else if (key == KEY_PGUP)
        return RB_C(rb_str_new_cstr)("PGUP");
    else if (key == KEY_PGDN)
        return RB_C(rb_str_new_cstr)("PGDN");
#ifdef KEY_BKSP
    else if (key == KEY_BKSP)
#else
    else if (key == '\b' || key == 0x7F)
#endif
        return RB_C(rb_str_new_cstr)("BKSP");
    else if (key == KEY_TAB)
        return RB_C(rb_str_new_cstr)("TAB");
    else if (key == KEY_ENTER)
        return RB_C(rb_str_new_cstr)("ENTER");
    else if (key > 0x00 && key < 0x1b) // Ctrl()
    {
        char buf[3];
        sprintf(buf, "^%c", key + 'A' - 1);
        return RB_C(rb_str_new_cstr)(buf);
    }
    else // Normal characters
    {
        char buf[2];
        sprintf(buf, "%c", key);
        return RB_C(rb_str_new_cstr)(buf);
    }
}

VALUE brb_clock RBF_P((VALUE self))
{
    struct timeval tv;
    struct timezone tz;
    double d = 0;
    memset(&tz, 0, sizeof(tz));
    gettimeofday(&tv, &tz);
    d = tv.tv_sec + tv.tv_usec / (double)1000000;
    return CMRB_C(rb_float_new, mrb_float_value)(d);
}

VALUE brb_getch RBF_P((VALUE self))
{
    //if (RARRAY_LEN(KB_QUEUE) == 0)
        int c = vkey();
        return BRB_C(brb_keyToString)(c);
    //else
        //return RB_C(rb_ary_pop)(KB_QUEUE);
}

#define MACRO_NONZERO(macro)  ((macro-0) != 0)

VALUE brb_getdata RBF_P((int argc, VALUE *argv, VALUE self))
{
#ifdef BBSRUBY_USE_MRUBY
    mrb_int argc;
    VALUE *argv;
    mrb_get_args(mrb, "*", &argv, &argc);
#endif

    int echo = DOECHO;
    if (argc > 1 && NUM2INT(argv[1]) == 0)
#if MACRO_NONZERO(VGET_STEALTH_NOECHO)
        echo = NOECHO | VGET_STEALTH_NOECHO;
#else
        echo = NOECHO;
#endif
#if MACRO_NONZERO(VGET_STRICT_DOECHO)
    if (echo == NOECHO)
        echo |= VGET_STRICT_DOECHO;
#endif
#if defined(VGET_STRICT_DOECHO) && MACRO_NONZERO(VGET_BREAKABLE)
    echo |= VGET_BREAKABLE;
#endif

    int maxsize = NUM2INT(argv[0]);
    maxsize = BMIN(maxsize, 511);
    char data[512] = "";
    int cur_row, cur_col;
    getyx(&cur_row, &cur_col);

    int res = vget(cur_row, cur_col, "", data, maxsize, echo);
    if (res <= 0)
    {
#ifdef VGET_EXIT_BREAK
        if (res == VGET_EXIT_BREAK)
#else
        if (buf[1] == Ctrl('C'))
#endif
        {
            ABORT_BBSRUBY = 1;
        }
        return RB_C(rb_str_new_cstr)("");
    }
    else
    {
        return RB_C(rb_str_new_cstr)(data);
    }
}

VALUE brb_kbhit RBF_P((VALUE self, VALUE wait))
{
#ifdef BBSRUBY_USE_MRUBY
    VALUE wait;
    mrb_get_args(mrb, "o", &wait);
#endif

    double data = NUM2DBL(wait);
    data = BMIN(BMAX(data, KBHIT_TMIN), KBHIT_TMAX);

    if (getkey(data) != 0)
    {
        //RB_C(rb_ary_push)(KB_QUEUE, BRB_C(brb_keyToString)(data));
        return INT2NUM(1);
    }
    else
        return INT2NUM(0);
}

VALUE brb_outs RBF_P((int argc, VALUE *argv, VALUE self))
{
#ifdef BBSRUBY_USE_MRUBY
    mrb_int argc;
    VALUE *argv;
    mrb_get_args(mrb, "*", &argv, &argc);
#endif

    for (int i=0; i<argc; i++)
    {
        outs(StringValueCStr(argv[i]));
    }
    return Qnil;
}

VALUE brb_title RBF_P((VALUE self, VALUE msg))
{
#ifdef BBSRUBY_USE_MRUBY
    VALUE msg;
    mrb_get_args(mrb, "o", &msg);
#endif

    vs_bar(StringValueCStr(msg));
    return Qnil;
}

VALUE brb_print RBF_P((int argc, VALUE *argv, VALUE self))
{
    RBF_C(brb_outs)(argc, argv, self);
    outs("\n");
    return Qnil;
}

VALUE brb_getmaxyx RBF_P((VALUE self))
{
    VALUE rethash = RB_CV(rb_hash_new)();
    CMRB_C(rb_hash_aset, mrb_hash_set)(rethash, RB_C(rb_str_new_cstr)(badxy_compat ? "x" : BRB_COOR_ROW), INT2NUM(b_lines + 1));
    CMRB_C(rb_hash_aset, mrb_hash_set)(rethash, RB_C(rb_str_new_cstr)(badxy_compat ? "y" : BRB_COOR_COL), INT2NUM(b_cols + 1));
    return rethash;
}

VALUE brb_getyx RBF_P((VALUE self))
{
    VALUE rethash = RB_CV(rb_hash_new)();
    int cur_row, cur_col;
    getyx(&cur_row, &cur_col);
    CMRB_C(rb_hash_aset, mrb_hash_set)(rethash, RB_C(rb_str_new_cstr)(badxy_compat ? "x" : BRB_COOR_ROW), INT2NUM(badxy_compat ? cur_col : cur_row));
    CMRB_C(rb_hash_aset, mrb_hash_set)(rethash, RB_C(rb_str_new_cstr)(badxy_compat ? "y" : BRB_COOR_COL), INT2NUM(badxy_compat ? cur_row : cur_col));
    return rethash;
}

VALUE brb_move RBF_P((VALUE self, VALUE y, VALUE x))
{
#ifdef BBSRUBY_USE_MRUBY
    VALUE y, x;
    mrb_get_args(mrb, "oo", &y, &x);
#endif

    move(NUM2INT(badxy_compat ? x : y), NUM2INT(badxy_compat ? y : x));
    return Qnil;
}

VALUE brb_moverel RBF_P((VALUE self, VALUE dy, VALUE dx))
{
#ifdef BBSRUBY_USE_MRUBY
    VALUE dy, dx;
    mrb_get_args(mrb, "oo", &dy, &dx);
#endif

    int cur_row, cur_col;
    getyx(&cur_row, &cur_col);
    move(cur_row + NUM2INT(badxy_compat ? dx : dy), cur_col + NUM2INT(badxy_compat ? dy : dx));
    return Qnil;
}

VALUE brb_clear RBF_P((VALUE self)) { clear(); return Qnil; }

VALUE brb_clrtoeol RBF_P((VALUE self)) { clrtoeol(); return Qnil; }
VALUE brb_clrtobot RBF_P((VALUE self)) { clrtobot(); return Qnil; }

VALUE brb_refresh RBF_P((VALUE self)) { refresh(); return Qnil; }

VALUE brb_vmsg RBF_P((VALUE self, VALUE msg))
{
#ifdef BBSRUBY_USE_MRUBY
    VALUE msg;
    mrb_get_args(mrb, "o", &msg);
#endif

    vmsg(StringValueCStr(msg));
    return Qnil;
}

VALUE brb_name RBF_P((VALUE self)) { return RB_C(rb_str_new_cstr)(BBSNAME); }
VALUE brb_interface RBF_P((VALUE self)) { return CMRB_C(rb_float_new, mrb_float_value)(BBSRUBY_INTERFACE_VER); }

VALUE brb_ansi_color RBF_P((int argc, VALUE *argv, VALUE self))
{
#ifdef BBSRUBY_USE_MRUBY
    mrb_int argc;
    VALUE *argv;
    mrb_get_args(mrb, "*", &argv, &argc);
#endif

    char buf[50] = "\033[";
    char *p = buf + strlen(buf);

    for (int i=0; i<argc; i++)
    {
        int ansi = NUM2INT(argv[i]);
        sprintf(p, "%d%s", ansi, (i == argc - 1) ? "" : ";");
        p += strlen(p);
    }

    *p++ = 'm'; *p = '\0';

    return RB_C(rb_str_new_cstr)(buf);
}

VALUE brb_ansi_reset RBF_P((VALUE self))
{
    return RB_C(rb_str_new_cstr)("\033[m");
}

VALUE brb_esc RBF_P((VALUE self))
{
    return RB_C(rb_str_new_cstr)("\033");
}

VALUE brb_color RBF_P((int argc, VALUE *argv, VALUE self))
{
#ifdef BBSRUBY_USE_MRUBY
    mrb_int argc;
    VALUE *argv;
    mrb_get_args(mrb, "*", &argv, &argc);
#endif

    VALUE str;
    if (argc == 0)
        str = RBF_C(brb_ansi_reset)(self);
    else
        str = RBF_C(brb_ansi_color)(argc, argv, self);

#ifdef BBSRUBY_USE_MRUBY
    mrb_funcall_argv(mrb, self, mrb_intern_cstr(mrb, "outs"), 1, &str);
#else
    brb_outs(1, &str, self);
#endif
    return Qnil;
}

VALUE brb_userid RBF_P((VALUE self))
{
    return RB_C(rb_str_new_cstr)(cuser.userid);
}

VALUE brb_pause RBF_P((VALUE self, VALUE msg))
{
#ifdef BBSRUBY_USE_MRUBY
    VALUE msg;
    mrb_get_args(mrb, "o", &msg);
#endif

    int k = pause_msg(StringValueCStr(msg), "", "請按任意鍵繼續");

    move(b_lines, 0);
    clrtoeol();

    return BRB_C(brb_keyToString)(k);
}

GCC_PURE VALUE brb_toc RBF_P((VALUE self))
{
    return TOCs_rubyhash;
}
/* End of BBS helper class */

static int pause_msg(const char *msg, const char *reason, const char *prompt)
{
    char buf[200];
    move(b_lines, 0);

    sprintf(buf, COLOR1 " ★ %s%s", msg, (reason) ? reason : "");
    outs(buf);

    char buf2[200];
    sprintf(buf2, COLOR2 " [%s] ", prompt);

    for (int i = b_cols + (int)(unsigned)(sizeof(COLOR1) + sizeof(COLOR2)) - (int)(unsigned)(strlen(buf) + strlen(buf2)); i > 3; i--)
    {
        outc(' ');
    }

    outs(buf2);
    outs(str_ransi);

    return vkey();
}

static int out_footer(
    const char* reason,
    const char* msg)
{
    return pause_msg("BBSRuby " BBSRUBY_VERSION_STR " (" __DATE__ " " __TIME__ ")", reason, msg);
}


static int getkey(double wait)
{
    fd_set fds;
    struct timeval tv = {0L, 0L};

    FD_ZERO(&fds);
    FD_SET(0, &fds);

    /* Determine whether to refresh */
    if (select(1, &fds, NULL, NULL, &tv) > 0)
        return vkey();
    if (wait > KBHIT_TMIN)
        refresh();

    tv.tv_sec = (long)wait;
    wait-=(long)wait;
    tv.tv_usec = wait * 1000000;

    /* 若有按鍵，回傳所按的鍵；若 delay 的時間到了仍沒有按鍵，回傳 0 */

    if (select(1, &fds, NULL, NULL, &tv) > 0)
        return vkey();

    return 0;
}

static void BBSRubyHook(
#ifdef BBSRUBY_USE_MRUBY
    mrb_state *mrb,
  #if MRUBY_RELEASE_NO >= 30000
    const mrb_irep *irep,
  #else
    mrb_irep *irep,
  #endif
  #if MRUBY_RELEASE_NO >= 20100
    const mrb_code *pc,
  #else
    mrb_code *pc,
  #endif
    mrb_value *regs
#else
   rb_event_flag_t event,
  #ifdef RUBY_VM
   VALUE data,
  #else
   NODE *node,
  #endif
   VALUE self,
   ID mid,
   VALUE klass
#endif
)
{
    static int hook_count = 0;
    hook_count++;
    if (hook_count == BRB_HOOK_COUNT_FACTOR * 1000)
    {
        int key = getkey(KBHIT_TMIN);
        if (key == Ctrl('C'))
            ABORT_BBSRUBY = 1;
        // else
            // RB_C(rb_ary_push)(KB_QUEUE, INT2NUM(key));
        hook_count = 0;
    }

    if (ABORT_BBSRUBY)
    {
#ifdef BBSRUBY_USE_MRUBY
        // Raise an exception to terminate execution
        mrb_raise(mrb, mrb_exc_get(mrb, "Exception"), "User Interrupt");
#else
        rb_thread_kill(rb_thread_current());
        // TODO: Ensure all thread cleanup.
#endif
    }

    return;
}

static char*
ruby_script_attach(const char *fpath, size_t *plen)
{
    struct stat st;
    int fd = open(fpath, O_RDONLY, 0600);
    char *buf = NULL;

    *plen = 0;

    if (fd < 0) return buf;
    if (fstat(fd, &st) || ((*plen = st.st_size) < 1) || S_ISDIR(st.st_mode))
    {
        close(fd);
        return buf;
    }

    buf = (char *) mmap(NULL, *plen, PROT_READ, MAP_SHARED, fd, 0);
    close(fd);

    if (buf == NULL || buf == MAP_FAILED)
    {
        *plen = 0;
        return NULL;
    }

    return buf;
}

static void ruby_script_detach(char *p, int len)
{
    munmap(p, len);
}

static int ruby_script_range_detect(char **pStart, char **pEnd, int *lineshift)
{
    int lenSignature = strlen(BBSRUBY_SIGNATURE);

    // Search range
    char *cStart, *cEnd;
    int line = 0;
    cStart = *pStart;
    cEnd = *pEnd;

    // Find head signature first
    while (cStart + lenSignature < cEnd)
    {
        if (strncmp(cStart, BBSRUBY_SIGNATURE, lenSignature) == 0)
            break;

        // Skip to next line
        while (cStart + lenSignature < cEnd && *cStart++ != '\n');
        line++;
    }
    *lineshift = line;

    if (cStart + lenSignature >= *pEnd) // Cannot found signature.
        return 0;

    *pStart = cStart;

    // Find the tail signature
    cEnd = cStart + 1;
    while (cEnd + lenSignature < *pEnd)
    {
        if (strncmp(cEnd, BBSRUBY_SIGNATURE, lenSignature) == 0)
            break;

        // Skip to next line
        while (cEnd + lenSignature < *pEnd && *cEnd++ != '\n');
    }

    if (cEnd + lenSignature >= *pEnd)
    {
        // Not found
        *pStart = cStart;
        *pEnd = cEnd;
        return 0;
    }
    else
    {
        cEnd--;
        *pEnd = cEnd;
    }

    cEnd = *pEnd;
    while (cEnd > cStart && !*cEnd)
        cEnd--;
    *pEnd = cEnd;
    return 1;
}

static void bbsruby_load_TOC RB_P((const char *cStart, const char *cEnd))
{
    // Create TOC class wrapping these information
    const char *tStart, *tEnd;
    tStart = cStart;
    VALUE hashTOC = RB_CV(rb_hash_new)();
    GCC_UNUSED int TOCfound = 0;
    // In this implement, we only allow TOC to be put BEFORE the actual script code
    while (tStart < cEnd)
    {
        if (tStart[0] == '#' && tStart[1] == '#')
        {
            tStart += 2;

            // The third '#' is optional
            if (tStart[0] == '#')
                tStart++;

            while (*tStart == ' ') tStart++;

            tEnd = tStart;
            while (*tEnd != '\n') tEnd++;

            // Possible TOC item, check patterns
            for (int i=0; i<BBSRUBY_TOC_HEADERS; i++)
            {
                int lenBuf = strlen(TOCs_HEADER[i]);
                if (strncasecmp(tStart, TOCs_HEADER[i], lenBuf) == 0)
                {
                    tStart+=lenBuf;
                    while (*tStart == ' ') tStart++;
                    if (*tStart != ':') break;
                    tStart++;

                    while (*tStart == ' ') tStart++;
                    CMRB_C(rb_hash_aset, mrb_hash_set)(hashTOC, RB_C(rb_str_new_cstr)(TOCs_HEADER[i]), RB_C(rb_str_new)(tStart, tEnd - tStart));
                    CMRB_C(rb_hash_aset, mrb_hash_set)(hashTOC, CMRB_C(rb_funcallv, mrb_funcall_argv)(RB_C(rb_str_new_cstr)(TOCs_HEADER[i]), CMRB_C(rb_intern, mrb_intern_cstr)("capitalize!"), 0, NULL), RB_C(rb_str_new)(tStart, tEnd - tStart));
                    TOCfound = 1;
                    break;
                }
            }
            tStart = tEnd + 1;
        }
        else
            break;
    }

    TOCs_rubyhash = hashTOC;
}

static void print_exception RB_PV((void))
{
    clear();
#ifdef BBSRUBY_USE_MRUBY
    VALUE exception = mrb_obj_value(mrb->exc);
    mrb->exc = mrb_obj_ptr(Qnil);
#else
    VALUE exception = rb_errinfo();
    rb_set_errinfo(Qnil);
#endif
    if (!RTEST(exception)) return;

    char* buffer = RSTRING_PTR(RB_C(rb_obj_as_string)(exception));
    outs("\033[m");
    clear();
    move(0, 0);
    outs("程式發生錯誤，無法繼續執行。請通知原作者。\n錯誤資訊：\n");
    outs(buffer);
    outs("\n");
    VALUE ary = CMRB_C(rb_funcallv, mrb_funcall_argv)(exception, CMRB_C(rb_intern, mrb_intern_cstr)("backtrace"), 0, NULL);
#ifdef BBSRUBY_USE_MRUBY
    // Some `Exception` objects have theirs `backtrace` unset in mruby
    if (mrb_array_p(ary))
#endif
    {
        for (int c=0; c < RARRAY_LEN(ary); c++)
        {
            outs("  from: ");
            outs(StringValueCStr(RARRAY_PTR(ary)[c]));
            outs("\n");
        }
    }
    out_footer(" (發生錯誤)", "按任意鍵返回");
}

#ifdef BBSRUBY_USE_MRUBY

#define BRBCPP_APPENDTO_NOEVAL(x, ...)  __VA_ARGS__ ## x
#define BRBCPP_APPENDTO(...)  BRBCPP_APPENDTO_NOEVAL(__VA_ARGS__)

#define BRBCPP_EXPAND_LIST(list)  \
    BRBCPP_APPENDTO(_END, BRBCPP_EXPAND_LIST_OP1 list)

#define BRBCPP_EXPAND_LIST_OP1(elem)  \
    BRBCPP_LIST_TRANSFORM(elem) BRBCPP_EXPAND_LIST_OP2
#define BRBCPP_EXPAND_LIST_OP2(elem)  \
    BRBCPP_LIST_TRANSFORM(elem) BRBCPP_EXPAND_LIST_OP1

#define BRBCPP_EXPAND_LIST_OP1_END
#define BRBCPP_EXPAND_LIST_OP2_END

/* mruby gem enabling conditions. Can be combined to achieve the logical AND logic */

#ifdef BBSRUBY_MRUBY_HAS_FULLCORE_GEMBOX
  #define BRB_FULLCORE(_gem) _gem
#else
  #define BRB_FULLCORE(_gem) /* Empty */
#endif

/* Assume `MRUBY_RELEASE_NO >= 10400` */

#if MRUBY_RELEASE_NO >= 10401
  #define BRB_MRB10401(_gem) _gem
  #define BRB_BEFORE_MRB10401(_gem) /* Empty */
#else
  #define BRB_MRB10401(_gem) /* Empty */
  #define BRB_BEFORE_MRB10401(_gem) _gem
#endif

#if MRUBY_RELEASE_NO >= 20000
  #define BRB_MRB20000(_gem) _gem
  #define BRB_BEFORE_MRB20000(_gem) /* Empty */
#else
  #define BRB_MRB20000(_gem) /* Empty */
  #define BRB_BEFORE_MRB20000(_gem) _gem
#endif

#if MRUBY_RELEASE_NO >= 20001
  #define BRB_MRB20001(_gem) _gem
  #define BRB_BEFORE_MRB20001(_gem) /* Empty */
#else
  #define BRB_MRB20001(_gem) /* Empty */
  #define BRB_BEFORE_MRB20001(_gem) _gem
#endif

#if MRUBY_RELEASE_NO >= 20100
  #define BRB_MRB20100(_gem) _gem
  #define BRB_BEFORE_MRB20100(_gem) /* Empty */
#else
  #define BRB_MRB20100(_gem) /* Empty */
  #define BRB_BEFORE_MRB20100(_gem) _gem
#endif

#if MRUBY_RELEASE_NO >= 20102
  #define BRB_MRB20102(_gem) _gem
  #define BRB_BEFORE_MRB20102(_gem) /* Empty */
#else
  #define BRB_MRB20102(_gem) /* Empty */
  #define BRB_BEFORE_MRB20102(_gem) _gem
#endif

#if MRUBY_RELEASE_NO >= 30000
  #define BRB_MRB30000(_gem) _gem
  #define BRB_BEFORE_MRB30000(_gem) /* Empty */
#else
  #define BRB_MRB30000(_gem) /* Empty */
  #define BRB_BEFORE_MRB30000(_gem) _gem
#endif

#define BRB_MRBGEM_LIST  \
    (mruby_sprintf) \
    /* (mruby_print) */ \
    (mruby_math) \
    (mruby_time) \
    (mruby_struct) \
    (mruby_compar_ext) \
    (mruby_enum_ext) \
    (mruby_string_ext) \
    (mruby_numeric_ext) \
    (mruby_array_ext) \
    (mruby_hash_ext) \
    (mruby_range_ext) \
    (mruby_proc_ext) \
    (mruby_symbol_ext) \
    (mruby_random) \
    (mruby_object_ext) \
    (mruby_objectspace) \
    (mruby_fiber) \
    (mruby_enumerator) \
    (mruby_enum_lazy) \
    (mruby_toplevel_ext) \
    (mruby_kernel_ext) \
    (mruby_class_ext) \
    (mruby_error) \
    /* BRB_FULLCORE((mruby_exit)) */ \
    BRB_FULLCORE(BRB_BEFORE_MRB20102((mruby_eval))) \
    /* BRB_FULLCORE(BRB_BEFORE_MRB10401((mruby_io))) */ \
    BRB_FULLCORE(BRB_BEFORE_MRB20100((mruby_method))) \
    /* BRB_MRB10401((mruby_io)) */ \
    BRB_MRB10401((mruby_pack)) \
    /* BRB_FULLCORE(BRB_BEFORE_MRB30000(BRB_MRB10401((mruby_socket)))) */ \
    BRB_MRB20000((mruby_metaprog)) \
    /* BRB_FULLCORE(BRB_MRB20000((mruby_sleep))) */ \
    BRB_FULLCORE(BRB_MRB20001((mruby_enum_chain))) \
    BRB_MRB20100((mruby_method)) \
    BRB_FULLCORE(BRB_BEFORE_MRB20102(BRB_MRB20100((mruby_complex)))) \
    BRB_FULLCORE(BRB_BEFORE_MRB20102(BRB_MRB20100((mruby_rational)))) \
    BRB_MRB20102((mruby_eval)) \
    BRB_MRB20102((mruby_complex)) \
    BRB_MRB20102((mruby_rational)) \
    /* BRB_MRB30000((mruby_socket)) */ \
    BRB_FULLCORE(BRB_MRB30000((mruby_catch))) \
    BRB_FULLCORE(BRB_MRB30000((mruby_os_memsize))) \
    /* End of list */

#ifdef __cplusplus
extern "C" {
#endif

#undef BRBCPP_LIST_TRANSFORM
#define BRBCPP_LIST_TRANSFORM(elem)  \
    void GENERATED_TMP_mrb_ ## elem ## _gem_init(mrb_state *mrb); \
    void GENERATED_TMP_mrb_ ## elem ## _gem_final(mrb_state *mrb);
BRBCPP_EXPAND_LIST(BRB_MRBGEM_LIST)

#ifdef __cplusplus
}  // extern "C"
#endif

static void bbsruby_final_mrbgems(mrb_state *mrb)
{
#undef BRBCPP_LIST_TRANSFORM
#define BRBCPP_LIST_TRANSFORM(elem)  \
    GENERATED_TMP_mrb_ ## elem ## _gem_final(mrb);
    BRBCPP_EXPAND_LIST(BRB_MRBGEM_LIST)
}

static void bbsruby_init_mrbgems(mrb_state *mrb)
{
#undef BRBCPP_LIST_TRANSFORM
#define BRBCPP_LIST_TRANSFORM(elem)  \
    GENERATED_TMP_mrb_ ## elem ## _gem_init(mrb);
    BRBCPP_EXPAND_LIST(BRB_MRBGEM_LIST)
    mrb_state_atexit(mrb, bbsruby_final_mrbgems);
}
#endif

static void bbsruby_init_bbs_module RB_PV((void))
{
#ifdef BBSRUBY_USE_MRUBY
    // Remove former definition
    mrb_const_remove(mrb, mrb_obj_value(mrb->object_class), mrb_intern_cstr(mrb, "BBS"));
#else
    // Remove former definition
    rb_define_global_const("BBS", Qundef);
#endif

    // Prepare BBS wrapper module
    rb_class_t brb_mBBS = RB_C(rb_define_module)("BBS");
    RB_C(rb_define_module_function)(brb_mBBS, "outs", (rb_func_t) brb_outs, RB_ARGS_REST());
    RB_C(rb_define_module_function)(brb_mBBS, "title", (rb_func_t) brb_title, RB_ARGS_REQ(1));
    RB_C(rb_define_module_function)(brb_mBBS, "print", (rb_func_t) brb_print, RB_ARGS_REST());
    RB_C(rb_define_module_function)(brb_mBBS, "getyx", (rb_func_t) brb_getyx, RB_ARGS_REQ(0));
    RB_C(rb_define_module_function)(brb_mBBS, "getmaxyx", (rb_func_t) brb_getmaxyx, RB_ARGS_REQ(0));
    RB_C(rb_define_module_function)(brb_mBBS, "move", (rb_func_t) brb_move, RB_ARGS_REQ(2));
    RB_C(rb_define_module_function)(brb_mBBS, "moverel", (rb_func_t) brb_moverel, RB_ARGS_REQ(2));
    RB_C(rb_define_module_function)(brb_mBBS, "clear", (rb_func_t) brb_clear, RB_ARGS_REQ(0));
    RB_C(rb_define_module_function)(brb_mBBS, "clrtoeol", (rb_func_t) brb_clrtoeol, RB_ARGS_REQ(0));
    RB_C(rb_define_module_function)(brb_mBBS, "clrtobot", (rb_func_t) brb_clrtobot, RB_ARGS_REQ(0));
    RB_C(rb_define_module_function)(brb_mBBS, "refresh", (rb_func_t) brb_refresh, RB_ARGS_REQ(0));
    RB_C(rb_define_module_function)(brb_mBBS, "vmsg", (rb_func_t) brb_vmsg, RB_ARGS_REQ(1));
    RB_C(rb_define_module_function)(brb_mBBS, "pause", (rb_func_t) brb_pause, RB_ARGS_REQ(1));
    RB_C(rb_define_module_function)(brb_mBBS, "sitename", (rb_func_t) brb_name, RB_ARGS_REQ(0));
    RB_C(rb_define_module_function)(brb_mBBS, "interface", (rb_func_t) brb_interface, RB_ARGS_REQ(0));
    RB_C(rb_define_module_function)(brb_mBBS, "toc", (rb_func_t) brb_toc, RB_ARGS_REQ(0));
    RB_C(rb_define_module_function)(brb_mBBS, "ansi_color", (rb_func_t) brb_ansi_color, RB_ARGS_REST());
    RB_C(rb_define_module_function)(brb_mBBS, "color", (rb_func_t) brb_color, RB_ARGS_REST());
    RB_C(rb_define_module_function)(brb_mBBS, "ANSI_RESET", (rb_func_t) brb_ansi_reset, RB_ARGS_REQ(0));
    RB_C(rb_define_module_function)(brb_mBBS, "ESC", (rb_func_t) brb_esc, RB_ARGS_REQ(0));
    RB_C(rb_define_module_function)(brb_mBBS, "userid", (rb_func_t) brb_userid, RB_ARGS_REQ(0));
    RB_C(rb_define_module_function)(brb_mBBS, "getdata", (rb_func_t) brb_getdata, RB_ARGS_REST());
    RB_C(rb_define_module_function)(brb_mBBS, "clock", (rb_func_t) brb_clock, RB_ARGS_REQ(0));
    RB_C(rb_define_module_function)(brb_mBBS, "getch", (rb_func_t) brb_getch, RB_ARGS_REQ(0));
    RB_C(rb_define_module_function)(brb_mBBS, "kbhit", (rb_func_t) brb_kbhit, RB_ARGS_REQ(1));
}

static VALUE bbsruby_eval_code RB_P((VALUE eval_args))
{
#ifdef BBSRUBY_USE_MRUBY
    VALUE codestr = ((VALUE *)mrb_cptr(eval_args))[0];
    VALUE filename = ((VALUE *)mrb_cptr(eval_args))[1];
    VALUE lineno = ((VALUE *)mrb_cptr(eval_args))[2];
    mrbc_context *cxt = mrbc_context_new(mrb);

    mrbc_filename(mrb, cxt, StringValueCStr(filename));
    cxt->lineno = NUM2INT(lineno);
    cxt->capture_errors = 1;
#endif

    BRB_CV(bbsruby_init_bbs_module)();

#ifdef BBSRUBY_USE_MRUBY
    mrb_load_nstring_cxt(mrb, RSTRING_PTR(codestr), RSTRING_LEN(codestr), cxt);
    mrbc_context_free(mrb, cxt);
#else
    rb_obj_instance_eval(3, (VALUE *)eval_args, rb_class_new_instance(0, NULL, rb_cObject));
#endif
    return Qnil;
}

/* "0.111 v0.3-DL-2" => *apiver = 0.111, *brbver = 30002 */
static void parse_toc_apiver(const char *str, double *apiver, int *brbver)
{
    /* The BBS-Ruby API version (optional): A floating-point number without any suffixes */
    {
        const char *const p = str;
        *apiver = strtod(p, (char **)&str);
        if (str != p && *str && *str++ != ' ') // Reject "0.111v..." & Accept "0.111"
        {
            *apiver = 0;
            *brbver = 0;
            return;
        }
        /* Else if str == p, apiver may be omitted */
    }
    while (*str == ' ')
        ++str;
    /* The targeting BBS-Ruby version (optional): "v1.2.3-DL-4a" => 1020304 */
    if (*str == 'v' && *++str >= '0' && *str <= '9') // Reject "v.2" & "v+1"
    {
        *brbver = 1000000 * strtoul(str, (char **)&str, 10); // Major version
        if (*str == '.')
        {
            if (!(*++str >= '0' && *str <= '9'))
                goto invalid_brbver;
            *brbver += 10000 * strtoul(str, (char **)&str, 10); // Minor version (optional)
            if (*str == '.')
            {
                if (!(*++str >= '0' && *str <= '9'))
                    goto invalid_brbver;
                *brbver += 100 * strtoul(str, (char **)&str, 10); // Patch version (optional)
            }
        }
        int has_exver = 0;
        while (*str == '-') // Extra version (optional) & tags (0 or more): "-DL-4a" (of "v1.2.3-DL-4a") => 4
        {
            if (has_exver || !(*++str && *str != ' ' && *str != '-')) // Reject "v1.2.3-DL-4a-...", "v1.2.3-", "v1.2.3- ...", & "v1.2.3--..."
                goto invalid_brbver;
            if ((has_exver = (*str >= '0' && *str <= '9')))
                *brbver += strtoul(str, (char **)&str, 10); // Extra version
            while (*str && *str != ' ' && *str != '-') // Tag content ("DL" in "-DL") / version suffix ("a" in "-4a")
                ++str;
        }
        if (*str && *str != ' ') // Reject "v1.2.3a"
            goto invalid_brbver;
    }
    else
    {
invalid_brbver:
        *brbver = 0;
    }
}

void run_ruby(
    const char* fpath)
{
#ifdef DL_HOLD
    DL_HOLD;
#endif
#ifdef BBSRUBY_USE_MRUBY
    mrb_state *mrb;
    mrb_bool error = 0;
#else
    static int ruby_inited = 0;
    int error = 0;
#endif
    ABORT_BBSRUBY = 0;

    // Run
    size_t pLen = 0;
    char *post;
    post = ruby_script_attach(fpath, &pLen);
    if (!post)
    {
        out_footer(" (內部錯誤)",  "按任意鍵返回");
        DL_RELEASE_VOID();
        return;
    }

    char *cStart, *cEnd;
    int lineshift;
    cStart = post;
    cEnd = post + pLen;
    if (ruby_script_range_detect(&cStart, &cEnd, &lineshift) == 0 || cStart == NULL || cEnd == NULL)
    {
        ruby_script_detach(post, pLen);
        out_footer(" (找不到程式區段)",  "按任意鍵返回");
        DL_RELEASE_VOID();
        return;
    }

    // Initialize Ruby interpreter now.
#ifndef BBSRUBY_USE_MRUBY
    if (!ruby_inited)
#endif
    {
#ifdef BBSRUBY_USE_MRUBY
        if (!(mrb = mrb_open_core(mrb_default_allocf, NULL)))
#else
        RUBY_INIT_STACK;
        if (ruby_setup())
#endif
        {
            out_footer(" (內部錯誤)",  "按任意鍵返回");
            DL_RELEASE_VOID();
            return;
        }

#ifdef BBSRUBY_USE_MRUBY
        bbsruby_init_mrbgems(mrb);
#else
        ruby_init_loadpath();
        ruby_inited = 1;

        // Set safe level to 2
        // We cannot have protection if the safe level < 2
  #if defined(RUBY_SAFE_LEVEL_MAX) && RUBY_SAFE_LEVEL_MAX < 2
        // IID.20190129: Set to 1 to be compatible with Ruby 2.3+
        rb_set_safe_level(1);

        // IID.20190211: Vanilla Ruby with safe level < 2 is not safe;
        //    temporarily disable compiling BBS-Ruby against Ruby 2.3+
        //    where safe levels 2 to 4 are obsoleted
        #error  BBS-Ruby: Ruby 2.3+ is not supported by now for security reasons.
  #else
        rb_set_safe_level(2);
  #endif
#endif

        // Hook Ruby
#ifdef BBSRUBY_USE_MRUBY
  #if defined MRB_ENABLE_DEBUG_HOOK /* mruby2 */ || defined MRB_USE_DEBUG_HOOK /* mruby3 */
        mrb->code_fetch_hook = BBSRubyHook;
  #endif
#else
  #ifdef RUBY_VM
        rb_add_event_hook(BBSRubyHook, RUBY_EVENT_LINE, Qnil);
  #else
        rb_add_event_hook(BBSRubyHook, RUBY_EVENT_LINE);
  #endif
#endif
    }

    BRB_C(bbsruby_load_TOC)(cStart, cEnd);
    // Check interface version
    VALUE toc_apiver = CMRB_C(rb_hash_aref, mrb_hash_get)(TOCs_rubyhash, RB_C(rb_str_new_cstr)(TOCs_HEADER[0]));
    double apiver = 0; // Prevent further floating-point rounding errors
    int brbver = 0;
    if (RTEST(toc_apiver))
        parse_toc_apiver(StringValueCStr(toc_apiver), &apiver, &brbver);
    /* For the compatibility of the coordination behavior before v0.3-DL-1 (API 0.111) */
    badxy_compat = (brbver ? brbver < 30001 : apiver <= 0.111); // Will be exammed again later
    move(b_lines - 1 - badxy_compat, 0);
    char msgBuf[200]="";
    if (apiver == 0 && brbver == 0)
        sprintf(msgBuf, "\033[1;41m ● 程式未載明相容的Interface版本，可能發生不相容問題");
    else if (apiver < BBSRUBY_INTERFACE_VER || (brbver && brbver < BBSRUBY_VERSION_VALUE))
        sprintf(msgBuf, "\033[1;41m ● 程式版本過舊，可能發生不相容問題");
    if (*msgBuf)
    {
        outs(msgBuf);
        for (int i=0; i<b_cols - (int)(unsigned)strlen(msgBuf) + 7; i++)
            outs(" ");
        outs("\033[m");
    }

    if (badxy_compat)
    {
        move(b_lines - 1, 0);
        sprintf(msgBuf, "\033[1;41m %s 此程式%s是為舊版 BBS-Ruby (<= v3.0) 撰寫的。以座標相容模式執行？[Y/n]", (*msgBuf) ? "  " : "●", (!brbver && (!apiver || apiver == 0.111)) ? "可能" : "");
        outs(msgBuf);
        for (int i=0; i<b_cols - (int)(unsigned)strlen(msgBuf) + 7; i++)
            outs(" ");
        outs("\033[m");
    }

    //Before execution, prepare keyboard buffer
    //KB_QUEUE = RB_CV(rb_ary_new)();

    if ((out_footer("", "按任意鍵開始執行") | 0x20) == 'n')
        badxy_compat = 0;
    clear();
    refresh();

    VALUE eval_args[] = {RB_C(rb_str_new)(cStart, cEnd - cStart), RB_C(rb_str_new_cstr)("BBSRuby"), INT2FIX(lineshift + 1)};
    ruby_script_detach(post, pLen);

#ifdef BBSRUBY_USE_MRUBY
    mrb_protect(mrb, bbsruby_eval_code, mrb_cptr_value(mrb, eval_args), &error);
#else
    rb_protect(bbsruby_eval_code, (VALUE)eval_args, &error);
#endif

#ifdef BBSRUBY_USE_MRUBY
    if (mrb->exc)
        error = 1;
#endif

    if (!error || ABORT_BBSRUBY)
        out_footer(ABORT_BBSRUBY ? " (使用者中斷)" : " (程式結束)", "按任意鍵返回");
    else
    {
        BRB_CV(print_exception)();
    }

#ifdef BBSRUBY_USE_MRUBY
    mrb_close(mrb);
#endif
    DL_RELEASE_VOID();
}
