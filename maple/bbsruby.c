//------------------------------------------------------------------------
// bbslua environment settings
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
#endif //M3_USE_BBSRUBY

#ifdef PTT_USE_BBSRUBY    // For compiling on PttBBS
 #define BBSRUBY_HAVE_VTUIKIT
 #undef BBSRUBY_NATIVE_B_LINES
 #undef BBSRUBY_NATIVE_B_COLS
 #undef BBSRUBY_USE_RH_COOR
 #define BBSRUBY_HAVE_GETYX
 #undef BBSRUBY_HAVE_STR_RANSI
 #undef BBSRUBY_VER_INFO_FILE
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
#include <ruby.h>

#ifndef RUBY_VM
#include <rubysig.h>
#include <node.h>
typedef rb_event_t rb_event_flag_t;
#endif

typedef VALUE (*rb_func_t)(ANYARGS);

#define BBSRUBY_MAJOR_VERSION (0)
#define BBSRUBY_MINOR_VERSION (3)

#ifndef BBSRUBY_VERSION_STR
  #define BBSRUBY_VERSION_STR "v0.3"
#endif

#define BBSRUBY_SIGNATURE "###BBSRuby"

#define BBSRUBY_INTERFACE_VER 0.111
int ABORT_BBSRUBY = 0;

#define BBSRUBY_TOC_HEADERS (6)
const char* const TOCs_HEADER[BBSRUBY_TOC_HEADERS] = {"interface", "title", "notes", "author", "version", "date"};
char* TOCs_DATA[BBSRUBY_TOC_HEADERS] = {0};
VALUE TOCs_rubyhash;
double KBHIT_TMIN = 0.001;
double KBHIT_TMAX = 60*10;
VALUE KB_QUEUE;

int getkey(double wait);

/* BBS helper class : following BBSLua SDK */
VALUE brb_keyToString(int key)
{
    if (key == Ctrl('C')) // System-wide Abort
        ABORT_BBSRUBY = 1;

    if (key == KEY_UP)
        return rb_str_new_cstr("UP");
    else if (key == KEY_DOWN)
        return rb_str_new_cstr("DOWN");
    else if (key == KEY_LEFT)
        return rb_str_new_cstr("LEFT");
    else if (key == KEY_RIGHT)
        return rb_str_new_cstr("RIGHT");
    else if (key == KEY_HOME)
        return rb_str_new_cstr("HOME");
    else if (key == KEY_INS)
        return rb_str_new_cstr("INS");
    else if (key == KEY_DEL)
        return rb_str_new_cstr("DEL");
    else if (key == KEY_END)
        return rb_str_new_cstr("END");
    else if (key == KEY_PGUP)
        return rb_str_new_cstr("PGUP");
    else if (key == KEY_PGDN)
        return rb_str_new_cstr("PGDN");
#ifdef KEY_BKSP
    else if (key == KEY_BKSP)
#else
    else if (key == '\b' || key == 0x7F)
#endif
        return rb_str_new_cstr("BKSP");
    else if (key == KEY_TAB)
        return rb_str_new_cstr("TAB");
    else if (key == KEY_ENTER)
        return rb_str_new_cstr("ENTER");
    else if (key > 0x00 && key < 0x1b) // Ctrl()
    {
        char buf[3];
        sprintf(buf, "^%c", key + 'A' - 1);
        return rb_str_new_cstr(buf);
    }
    else // Normal characters
    {
        char buf[2];
        sprintf(buf, "%c", key);
        return rb_str_new_cstr(buf);
    }
}

VALUE brb_clock(VALUE self)
{
    struct timeval tv;
    struct timezone tz;
    double d = 0;
    memset(&tz, 0, sizeof(tz));
    gettimeofday(&tv, &tz);
    d = tv.tv_sec + tv.tv_usec / 1000000;
    return rb_float_new(d);
}

VALUE brb_getch(VALUE self)
{
    //if (RARRAY_LEN(KB_QUEUE) == 0)
        int c = vkey();
        return brb_keyToString(c);
    //else
        //return rb_ary_pop(KB_QUEUE);
}

#define MACRO_NONZERO(macro)  ((macro-0) != 0)

VALUE brb_getdata(int argc, VALUE *argv, VALUE self)
{
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
        return rb_str_new_cstr("");
    }
    else
    {
        return rb_str_new_cstr(data);
    }
}

VALUE brb_kbhit(VALUE self, VALUE wait)
{
    double data = NUM2DBL(wait);
    data = BMIN(BMAX(data, KBHIT_TMIN), KBHIT_TMAX);

    if (getkey(data) != 0)
    {
        //rb_ary_push(KB_QUEUE, brb_keyToString(data));
        return INT2NUM(1);
    }
    else
        return INT2NUM(0);
}

VALUE brb_outs(int argc, VALUE *argv, VALUE self)
{
    int i;
    for (i=0; i<argc; i++)
    {
        outs(StringValueCStr(argv[i]));
    }
    return Qnil;
}

VALUE brb_title(VALUE self, VALUE msg)
{
    vs_bar(StringValueCStr(msg));
    return Qnil;
}

VALUE brb_print(int argc, VALUE *argv, VALUE self)
{
    brb_outs(argc, argv, self);
    outs("\n");
    return Qnil;
}

VALUE brb_getmaxyx(VALUE self)
{
    VALUE rethash = rb_hash_new();
    rb_hash_aset(rethash, rb_str_new_cstr(BRB_COOR_ROW), INT2NUM(b_lines + 1));
    rb_hash_aset(rethash, rb_str_new_cstr(BRB_COOR_COL), INT2NUM(b_cols + 1));
    return rethash;
}

VALUE brb_getyx(VALUE self)
{
    VALUE rethash = rb_hash_new();
    int cur_row, cur_col;
    getyx(&cur_row, &cur_col);
    rb_hash_aset(rethash, rb_str_new_cstr(BRB_COOR_ROW), INT2NUM(cur_row));
    rb_hash_aset(rethash, rb_str_new_cstr(BRB_COOR_COL), INT2NUM(cur_col));
    return rethash;
}

VALUE brb_move(VALUE self, VALUE y, VALUE x) { move(NUM2INT(y), NUM2INT(x)); return Qnil; }
VALUE brb_moverel(VALUE self, VALUE dy, VALUE dx) {
    int cur_row, cur_col;
    getyx(&cur_row, &cur_col);
    move(cur_row + NUM2INT(dy), cur_col + NUM2INT(dx));
    return Qnil;
}

VALUE brb_clear(VALUE self) { clear(); return Qnil; }

VALUE brb_clrtoeol(VALUE self) { clrtoeol(); return Qnil; }
VALUE brb_clrtobot(VALUE self) { clrtobot(); return Qnil; }

VALUE brb_refresh(VALUE self) { refresh(); return Qnil; }

VALUE brb_vmsg(VALUE self, VALUE msg) { vmsg(StringValueCStr(msg)); return Qnil; }

VALUE brb_name(VALUE self) { return rb_str_new_cstr(BBSNAME); }
VALUE brb_interface(VALUE self) { return rb_float_new(BBSRUBY_INTERFACE_VER); }

VALUE brb_ansi_color(int argc, VALUE *argv, VALUE self)
{
    char buf[50] = "\033[";
    char *p = buf + strlen(buf);

    const char sep[2] = ";";
    int i;
    for (i=0; i<argc; i++)
    {
        int ansi = NUM2INT(argv[i]);
        sprintf(p, "%d%s", ansi, (i == argc - 1) ? "" : sep);
        p += strlen(p);
    }

    *p++ = 'm'; *p = '\0';

    return rb_str_new_cstr(buf);
}

VALUE brb_ansi_reset(VALUE self)
{
    return rb_str_new_cstr("\033[m");
}

VALUE brb_esc(VALUE self)
{
    return rb_str_new_cstr("\033");
}

VALUE brb_color(int argc, VALUE *argv, VALUE self)
{
    VALUE str;
    if (argc == 0)
        str = brb_ansi_reset(self);
    else
        str = brb_ansi_color(argc, argv, self);

    brb_outs(1, &str, self);
    return Qnil;
}

VALUE brb_userid(VALUE self)
{
    return rb_str_new_cstr(cuser.userid);
}

VALUE brb_pause(VALUE self, VALUE msg)
{
    char buf[200];
    move(b_lines, 0);

    sprintf(buf, COLOR1 " ★ %s", StringValueCStr(msg));
    outs(buf);

    char buf2[200];
    sprintf(buf2, COLOR2 " [請按任意鍵繼續] ");

    int i;
    for (i = b_cols + sizeof(COLOR1) + sizeof(COLOR2) - strlen(buf) - strlen(buf2); i > 3; i--)
    {
        outc(' ');
    }

    outs(buf2);
    outs(str_ransi);

    int k = vkey();

    move(b_lines, 0);
    clrtoeol();

    return brb_keyToString(k);
}

GCC_PURE VALUE brb_toc(VALUE self)
{
    return TOCs_rubyhash;
}
/* End of BBS helper class */

void out_footer(
    const char* reason,
    const char* msg)
{
    char buf[200];
    move(b_lines, 0);

    sprintf(buf, COLOR1 " ★ BBSRuby " BBSRUBY_VERSION_STR " (" __DATE__ " " __TIME__ ")%s", reason);
    outs(buf);

    char buf2[200];
    sprintf(buf2, COLOR2 " [%s] ", msg);

    int i;
    for (i = b_cols + sizeof(COLOR1) + sizeof(COLOR2) - strlen(buf) - strlen(buf2); i > 3; i--)
    {
        outc(' ');
    }

    outs(buf2);
    outs(str_ransi);

    vkey();
}


int getkey(double wait)
{
    int fd;
    struct timeval tv;

    fd = 1;

    tv.tv_sec = (int)wait;
    wait-=(int)wait;
    tv.tv_usec = wait * 1000000;

    /* 若有按鍵，回傳所按的鍵；若 delay 的時間到了仍沒有按鍵，回傳 0 */

    if (select(1, (fd_set *) &fd, NULL, NULL, &tv) > 0)
        return vkey();

    return 0;
}

void BBSRubyHook(
    rb_event_flag_t event,
#ifdef RUBY_VM
    VALUE data,
#else
    NODE *node,
#endif
    VALUE self,
    ID mid,
    VALUE klass)
{
    static int hook_count = 0;
    hook_count++;
    if (hook_count == 1000)
    {
        int key = getkey(KBHIT_TMIN);
        if (key == Ctrl('C'))
            ABORT_BBSRUBY = 1;
        // else
            // rb_ary_push(KB_QUEUE, INT2NUM(key));
        hook_count = 0;
    }

    if (ABORT_BBSRUBY)
    {
        rb_thread_kill(rb_thread_current());
        // TODO: Ensure all thread cleanup.
    }

    return;
}

static char*
ruby_script_attach(const char *fpath, int *plen)
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

void ruby_script_detach(char *p, int len)
{
    munmap(p, len);
}

int ruby_script_range_detect(char **pStart, char **pEnd, int *lineshift)
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

void bbsruby_load_TOC(const char *cStart, const char *cEnd)
{
    // Create TOC class wrapping these information
    const char *tStart, *tEnd;
    tStart = cStart;
    VALUE hashTOC = rb_hash_new();
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
            int i;
            for (i=0; i<BBSRUBY_TOC_HEADERS; i++)
            {
                int lenBuf = strlen(TOCs_HEADER[i]);
                if (strncasecmp(tStart, TOCs_HEADER[i], lenBuf) == 0)
                {
                    tStart+=lenBuf;
                    while (*tStart == ' ') tStart++;
                    if (*tStart != ':') break;
                    tStart++;

                    while (*tStart == ' ') tStart++;
                    char *data = (char *) malloc(sizeof(char) * (tEnd - tStart) + 1);
                    strncpy(data, tStart, tEnd - tStart);
                    data[tEnd - tStart] ='\0';
                    free(TOCs_DATA[i]);
                    TOCs_DATA[i] = data;
                    rb_hash_aset(hashTOC, rb_str_new_cstr(TOCs_HEADER[i]), rb_str_new_cstr(data));
                    rb_hash_aset(hashTOC, rb_funcallv(rb_str_new_cstr(TOCs_HEADER[i]), rb_intern("capitalize!"), 0, NULL), rb_str_new_cstr(data));
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

void print_exception(void)
{
    clear();
    VALUE exception = rb_errinfo();
    rb_set_errinfo(Qnil);
    if (!RTEST(exception)) return;

    char* buffer = RSTRING_PTR(rb_obj_as_string(exception));
    outs("\033[m");
    clear();
    move(0, 0);
    outs("程式發生錯誤，無法繼續執行。請通知原作者。\n錯誤資訊：\n");
    outs(buffer);
    outs("\n");
    VALUE ary = rb_funcallv(exception, rb_intern("backtrace"), 0, NULL);
    int c;
    for (c=0; c < RARRAY_LEN(ary); c++)
    {
        outs("  from: ");
        outs(StringValueCStr(RARRAY_PTR(ary)[c]));
        outs("\n");
    }
    out_footer(" (發生錯誤)", "按任意鍵返回");
}

static void bbsruby_init_bbs_module(void)
{
    // Remove former definition
    rb_define_global_const("BBS", Qundef);

    // Prepare BBS wrapper module
    VALUE brb_mBBS = rb_define_module("BBS");
    rb_define_module_function(brb_mBBS, "outs", (rb_func_t) brb_outs, -1);
    rb_define_module_function(brb_mBBS, "title", (rb_func_t) brb_title, 1);
    rb_define_module_function(brb_mBBS, "print", (rb_func_t) brb_print, -1);
    rb_define_module_function(brb_mBBS, "getyx", (rb_func_t) brb_getyx, 0);
    rb_define_module_function(brb_mBBS, "getmaxyx", (rb_func_t) brb_getmaxyx, 0);
    rb_define_module_function(brb_mBBS, "move", (rb_func_t) brb_move, 2);
    rb_define_module_function(brb_mBBS, "moverel", (rb_func_t) brb_moverel, 2);
    rb_define_module_function(brb_mBBS, "clear", (rb_func_t) brb_clear, 0);
    rb_define_module_function(brb_mBBS, "clrtoeol", (rb_func_t) brb_clrtoeol, 0);
    rb_define_module_function(brb_mBBS, "clrtobot", (rb_func_t) brb_clrtobot, 0);
    rb_define_module_function(brb_mBBS, "refresh", (rb_func_t) brb_refresh, 0);
    rb_define_module_function(brb_mBBS, "vmsg", (rb_func_t) brb_vmsg, 1);
    rb_define_module_function(brb_mBBS, "pause", (rb_func_t) brb_pause, 1);
    rb_define_module_function(brb_mBBS, "sitename", (rb_func_t) brb_name, 0);
    rb_define_module_function(brb_mBBS, "interface", (rb_func_t) brb_interface, 0);
    rb_define_module_function(brb_mBBS, "toc", (rb_func_t) brb_toc, 0);
    rb_define_module_function(brb_mBBS, "ansi_color", (rb_func_t) brb_ansi_color, -1);
    rb_define_module_function(brb_mBBS, "color", (rb_func_t) brb_color, -1);
    rb_define_module_function(brb_mBBS, "ANSI_RESET", (rb_func_t) brb_ansi_reset, 0);
    rb_define_module_function(brb_mBBS, "ESC", (rb_func_t) brb_esc, 0);
    rb_define_module_function(brb_mBBS, "userid", (rb_func_t) brb_userid, 0);
    rb_define_module_function(brb_mBBS, "getdata", (rb_func_t) brb_getdata, -1);
    rb_define_module_function(brb_mBBS, "clock", (rb_func_t) brb_clock, 0);
    rb_define_module_function(brb_mBBS, "getch", (rb_func_t) brb_getch, 0);
    rb_define_module_function(brb_mBBS, "kbhit", (rb_func_t) brb_kbhit, 1);
}

static VALUE bbsruby_eval_code(VALUE eval_args)
{
    bbsruby_init_bbs_module();

    rb_obj_instance_eval(3, (VALUE *)eval_args, rb_class_new_instance(0, NULL, rb_cObject));
    return Qnil;
}

void run_ruby(
    const char* fpath)
{
    static int ruby_inited = 0;
    ABORT_BBSRUBY = 0;

    // Initialize Ruby interpreter first.
    if (!ruby_inited)
    {
        RUBY_INIT_STACK;
        if (ruby_setup())
        {
            out_footer(" (內部錯誤)",  "按任意鍵返回");
            return;
        }

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

        // Hook Ruby
#ifdef RUBY_VM
        rb_add_event_hook(BBSRubyHook, RUBY_EVENT_LINE, Qnil);
#else
        rb_add_event_hook(BBSRubyHook, RUBY_EVENT_LINE);
#endif
    }

    // Run
    int error=0;
    ruby_script("BBSRUBY");
    int pLen = 0;
    char *post;
    post = ruby_script_attach(fpath, &pLen);
    if (!post)
    {
        out_footer(" (內部錯誤)",  "按任意鍵返回");
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
        return;
    }

    bbsruby_load_TOC(cStart, cEnd);
    // Check interface version
    float d = 0;
    if (TOCs_DATA[4])
        d = atof(TOCs_DATA[4]);
    move(b_lines - 1, 0);
    char msgBuf[200]="";
    if (d == 0)
        sprintf(msgBuf, "\033[1;41m ● 程式未載明相容的Interface版本，可能發生不相容問題");
    else if (d < BBSRUBY_INTERFACE_VER)
        sprintf(msgBuf, "\033[1;41m ● 程式版本過舊，可能發生不相容問題");
    outs(msgBuf);
    int i;
    for (i=0; i<b_cols - strlen(msgBuf) + 7; i++)
        outs(" ");
    outs("\033[m");

    for (int i = 0; i < BBSRUBY_TOC_HEADERS; i++)
    {
        free(TOCs_DATA[i]);
        TOCs_DATA[i] = NULL;
    }

    //Before execution, prepare keyboard buffer
    //KB_QUEUE = rb_ary_new();
    VALUE eval_args[] = {rb_str_new(cStart, cEnd - cStart), rb_str_new_cstr("BBSRuby"), INT2FIX(lineshift + 1)};
    ruby_script_detach(post, pLen);

    out_footer("", "按任意鍵開始執行");
    clear();
    refresh();

    rb_protect(bbsruby_eval_code, (VALUE)eval_args, &error);

    if (error == 0 || ABORT_BBSRUBY)
        out_footer(ABORT_BBSRUBY ? " (使用者中斷)" : " (程式結束)", "按任意鍵返回");
    else
    {
        print_exception();
    }
}
