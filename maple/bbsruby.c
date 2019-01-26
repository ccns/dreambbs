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
#include <rubysig.h>
#include <ruby/node.h>
#include <signal.h>
#include "RubyFix.h"

#define BBSRUBY_MAJOR_VERSION (0)
#define BBSRUBY_MINOR_VERSION (3)
#define BBSRUBY_VERSION_STR "v0.3"
#define BBSRUBY_SIGNATURE "###BBSRuby"

double BBSRUBY_INTERFACE_VER = 0.111;
int ABORT_BBSRUBY = 0;

#define BBSRUBY_TOC_HEADERS (6)
char* TOCs_HEADER[BBSRUBY_TOC_HEADERS] = {"Interface", "Title", "Notes", "Author", "Version", "Date"};
char* TOCs_DATA[BBSRUBY_TOC_HEADERS] = {""};
VALUE TOCs_rubyhash;
double KBHIT_TMIN = 0.001;
double KBHIT_TMAX = 60*10;
VALUE KB_QUEUE;

/* BBS helper class : following BBSLua SDK */
VALUE bbs_keyToString(int key)
{
	if (key == Ctrl('C')) // System-wide Abort
		ABORT_BBSRUBY = 1;

	if (key == KEY_UP)
		return rb_str_new2("UP");
	else if (key == KEY_DOWN)
		return rb_str_new2("DOWN");
	else if (key == KEY_LEFT)
		return rb_str_new2("LEFT");
	else if (key == KEY_RIGHT)
		return rb_str_new2("RIGHT");
	else if (key == KEY_HOME)
		return rb_str_new2("HOME");
	else if (key == KEY_INS)
		return rb_str_new2("INS");
	else if (key == KEY_DEL)
		return rb_str_new2("DEL");
	else if (key == KEY_END)
		return rb_str_new2("END");
	else if (key == KEY_PGUP)
		return rb_str_new2("PGUP");
	else if (key == KEY_PGDN)
		return rb_str_new2("PGDN");
	else if (key == KEY_BKSP)
		return rb_str_new2("BKSP");
	else if (key == KEY_TAB)
		return rb_str_new2("TAB");
	else if (key == KEY_ENTER)
		return rb_str_new2("ENTER");
	else if (key > 0x00 && key < 0x1b) // Ctrl()
	{
		char buf[3];
		sprintf(buf, "^%c", key + 'A' - 1);
		return rb_str_new2(buf);
	}
	else // Normal characters
	{
		char buf[2];
		sprintf(buf, "%c", key);
		return rb_str_new2(buf);
	}
}

VALUE bbs_clock(VALUE self)
{
	struct timeval tv;
	struct timezone tz;
	double d = 0;
	memset(&tz, 0, sizeof(tz));
	gettimeofday(&tv, &tz);
	d = tv.tv_sec + tv.tv_usec / 1000000;
	return rb_float_new(d);
}

VALUE bbs_getch(VALUE self)
{
	//if (RARRAY(KB_QUEUE)->len == 0)
		int c = vkey();
		return bbs_keyToString(c);
	//else
		//return rb_ary_pop(KB_QUEUE);
}

VALUE bbs_getdata(VALUE self, VALUE args)
{
	int count = RARRAY(args)->len;
	int echo = DOECHO;
	if (count > 1 && NUM2INT(rb_ary_entry(args, 1)) == 0)
		echo = NOECHO;
	
	int maxsize = NUM2INT(rb_ary_entry(args, 0));
	if (maxsize > 511) maxsize = 511;
	char data[512] = "";
	int cur_row, cur_col;
	getxy(&cur_col, &cur_row);
	
	vget(cur_col, cur_row, "", data, maxsize, echo);

	return rb_str_new2(data);
}

VALUE bbs_kbhit(VALUE self, VALUE wait)
{
	double data = NUM2DBL(wait);
	
	if (data < KBHIT_TMIN) data = KBHIT_TMIN;
	if (data > KBHIT_TMAX) data = KBHIT_TMAX;

	if (getkey(data) != 0)
	{
		//rb_ary_push(KB_QUEUE, bbs_keyToString(data));
		return INT2NUM(1);
	}
	else
		return INT2NUM(0);
}

void bbs_outs(VALUE self, VALUE args)
{
	int i, count = RARRAY(args)->len;
	for(i=0;i<count;i++)
	{
		outs(STR2CSTR(rb_ary_entry(args, i)));
	}
}

void bbs_title(VALUE self, VALUE msg)
{
	vs_bar(STR2CSTR(msg));
}

void bbs_print(VALUE self, VALUE args)
{
	bbs_outs(self, args);
	outs("\n");
}

VALUE bbs_getmaxyx(VALUE self)
{
	VALUE rethash = rb_hash_new();
	rb_hash_aset(rethash, rb_str_new2("x"), INT2NUM(b_lines + 1));
	rb_hash_aset(rethash, rb_str_new2("y"), INT2NUM(b_cols + 1));
	return rethash;
}

VALUE bbs_getyx(VALUE self)
{
	VALUE rethash = rb_hash_new();
	int cur_row, cur_col;
	getxy(&cur_col, &cur_row);
        rb_hash_aset(rethash, rb_str_new2("x"), INT2NUM(cur_row));
        rb_hash_aset(rethash, rb_str_new2("y"), INT2NUM(cur_col));
        return rethash;
}

void bbs_move(VALUE self, VALUE y, VALUE x) { move(NUM2INT(x), NUM2INT(y)); }
void bbs_moverel(VALUE self, VALUE dy, VALUE dx) { 
	int cur_row, cur_col;
	getxy(&cur_col, &cur_row);
	move(cur_col + dx, cur_row + dy); 
}

void bbs_clear(VALUE self) { clear(); }

void bbs_clrtoeol(VALUE self) { clrtoeol(); }
void bbs_clrtobot(VALUE self) { clrtobot(); }

void bbs_refresh(VALUE self) { refresh(); }

void bbs_vmsg(VALUE self, VALUE msg) { vmsg(STR2CSTR(msg)); }

VALUE bbs_name(VALUE self) { return rb_str_new2(BBSNAME); }
VALUE bbs_interface(VALUE self) { return rb_float_new(BBSRUBY_INTERFACE_VER); }

VALUE bbs_ansi_color(VALUE self, VALUE args)
{
	char buf[50] = "\033[";
	char *p = buf + strlen(buf);

	int count = RARRAY(args)->len;
	char sep[2] = ";";
	int i;
	for(i=0;i<count;i++)
	{
		int ansi = NUM2INT(rb_ary_entry(args, i));
		sprintf(p, "%d%s", ansi, (i == count - 1) ? "" : sep);
		p += strlen(p);
	}

	*p++ = 'm'; *p = '\0';
	
	return rb_str_new2(buf);
}

VALUE bbs_ansi_reset(VALUE self)
{
	return rb_str_new2("\033[m");
}

VALUE bbs_esc(VALUE self)
{
	return rb_str_new2("\033");
}

VALUE bbs_color(VALUE self, VALUE args)
{
	int count = RARRAY(args)->len;
	VALUE str;
	if (count == 0)
		str = bbs_ansi_reset(self);
	else
		str = bbs_ansi_color(self, args);

	VALUE arr = rb_ary_new();
	rb_ary_push(arr, str);
	bbs_outs(self, arr);
}

VALUE bbs_userid(VALUE self)
{
	return rb_str_new2(cuser.userid);
}

VALUE bbs_pause(VALUE self, VALUE msg)
{
        char buf[200];
        move(b_lines, 0);

        sprintf(buf, COLOR1 " ★ %s", STR2CSTR(msg));
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

	return bbs_keyToString(k);
}

VALUE bbs_toc(VALUE self)
{
	return TOCs_rubyhash;
}
/* End of BBS helper class */

void out_footer(reason, msg)
	char* reason;
	char* msg;
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

void BBSRubyHook(event, node, self, mid, klass)
	rb_event_flag_t event;
	NODE *node;
	VALUE self;
	ID mid;
	VALUE klass;
{
	static int hook_count = 0;
	hook_count++;
	if (hook_count == 1000)
	{
		int key = getkey(KBHIT_TMIN);
		if (key == Ctrl('C'))
			ABORT_BBSRUBY = 1;
		else
			rb_ary_push(KB_QUEUE, INT2NUM(key));
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
		fclose(fd);
		return buf;
	}

	buf = mmap(NULL, *plen, PROT_READ, MAP_SHARED, fd, 0);
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

int ruby_script_range_detect(char **pStart, char **pEnd)
{
	int lenSignature = strlen(BBSRUBY_SIGNATURE);
	
	// Search range
	char *cStart, *cEnd;
	cStart = *pStart;
	cEnd = *pEnd;

	// Find head signature first
	while(cStart + lenSignature < cEnd)
	{
		if (strncmp(cStart, BBSRUBY_SIGNATURE, lenSignature) == 0)
			break;

		// Skip to next line
		while(cStart + lenSignature < cEnd && *cStart++ != '\n');
	}

	if (cStart + lenSignature >= *pEnd) // Cannot found signature.
		return 0;
	
	*pStart = cStart;

	// Find the tail signature
	cEnd = cStart + 1;
	while(cEnd + lenSignature < *pEnd)
	{
		 if (strncmp(cEnd, BBSRUBY_SIGNATURE, lenSignature) == 0)
                        break;

                // Skip to next line
                while(cEnd + lenSignature < *pEnd && *cEnd++ != '\n');
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
	while(cEnd > cStart && !*cEnd)
		cEnd--;
	*pEnd = cEnd;

	// Create TOC class wrapping these information
	char *tStart, *tEnd;
	tStart = cStart;	
	VALUE hashTOC = rb_hash_new();
	int TOCfound = 0;
	// In this implement, we only allowd TOC be put BEFORE the actual script code
	while(tStart < cEnd)
	{
		if (tStart[0] == '#' && tStart[1] == '#' && tStart[2] == '#')
		{
			tStart += 3;
			tEnd = tStart + 1;
			while(*tEnd != '\n') tEnd++;

			// Possible TOC item, check patterns
			int i;
			for(i=0;i<BBSRUBY_TOC_HEADERS;i++)
			{
				char preBuf[100];
				sprintf(preBuf, "%s:", TOCs_HEADER[i]);
				int lenBuf = strlen(preBuf);
				if (strncmp(tStart, preBuf, lenBuf) == 0)
				{
					tStart+=lenBuf;
					while(*tStart == ' ') tStart++;
				 	malloc(sizeof(char) * (tEnd - tStart) + 1);
					char data[200];
					strncpy(data, tStart, tEnd - tStart);
					data[tEnd - tStart] ='\0';
					TOCs_DATA[i] = data;
					rb_hash_aset(hashTOC, rb_str_new2(TOCs_HEADER[i]), rb_str_new2(data));
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
	return 1;
}

void run_ruby_test(void)
{
	run_ruby("test.rb");
}

void print_exception()
{
                clear();
                VALUE exception = rb_gv_get("$!");
                char* buffer = RSTRING(rb_obj_as_string(exception))->as.ary;
                clear();
                move(0, 0);
                outs("程式發生錯誤，無法繼續執行。請通知原作者。\n錯誤資訊：\n");
                outs(buffer);
                outs("\n");
                /*VALUE ary = rb_funcall(rb_errinfo, rb_intern("backtrace"), 0);
                int c;
                for(c=0;c < RARRAY(ary)->len;c++)
                {
                        outs("  from: ");
                        outs(STR2CSTR(RARRAY(ary)->ptr[c]));
                        outs("\n");
                }*/
                out_footer(" (發生錯誤)", "按任意鍵返回");
}

void sig_handler(int sig)
{
	vmsg("嚴重錯誤！無法繼續執行！");
	// print_exception();
	rb_thread_kill(rb_thread_current());
}

void run_ruby(fpath)
	char* fpath;
{
	static int ruby_inited = 0;
	ABORT_BBSRUBY = 0;
	
	int sig;
	for(sig=0;sig<31;sig++)
		signal(sig, sig_handler);

	// Initalize Ruby interpreter first.
	if (!ruby_inited)
	{
		RUBY_INIT_STACK;
		ruby_init();
		ruby_init_loadpath();
		ruby_inited = 1;
	
		// Prepare BBS wrapper class
		VALUE rb_cBBS = rb_define_class("BBS", rb_cObject);
		rb_define_singleton_method(rb_cBBS, "outs", bbs_outs, -2);
		rb_define_singleton_method(rb_cBBS, "title", bbs_title, 1);
		rb_define_singleton_method(rb_cBBS, "print", bbs_print, -2);
		rb_define_singleton_method(rb_cBBS, "getyx", bbs_getyx, 0);
		rb_define_singleton_method(rb_cBBS, "getmaxyx", bbs_getmaxyx, 0);
		rb_define_singleton_method(rb_cBBS, "move", bbs_move, 2);
		rb_define_singleton_method(rb_cBBS, "moverel", bbs_moverel, 2);
		rb_define_singleton_method(rb_cBBS, "clear", bbs_clear, 0);
		rb_define_singleton_method(rb_cBBS, "clrtoeol", bbs_clrtoeol, 0);
		rb_define_singleton_method(rb_cBBS, "clrtobot", bbs_clrtobot, 0);
		rb_define_singleton_method(rb_cBBS, "refresh", bbs_refresh, 0);
		rb_define_singleton_method(rb_cBBS, "vmsg", bbs_vmsg, 1);
		rb_define_singleton_method(rb_cBBS, "pause", bbs_pause, 1);
		rb_define_singleton_method(rb_cBBS, "sitename", bbs_name, 0);
		rb_define_singleton_method(rb_cBBS, "interface", bbs_interface, 0);
		rb_define_singleton_method(rb_cBBS, "toc", bbs_toc, 0);
		rb_define_singleton_method(rb_cBBS, "ansi_color", bbs_ansi_color, -2);
		rb_define_singleton_method(rb_cBBS, "color", bbs_color, -2);
		rb_define_singleton_method(rb_cBBS, "ANSI_RESET", bbs_ansi_reset, 0);
		rb_define_singleton_method(rb_cBBS, "ESC", bbs_esc, 0);
		rb_define_singleton_method(rb_cBBS, "userid", bbs_userid, 0);
		rb_define_singleton_method(rb_cBBS, "getdata", bbs_getdata, -2);
		rb_define_singleton_method(rb_cBBS, "clock", bbs_clock, 0);
		rb_define_singleton_method(rb_cBBS, "getch", bbs_getch, 0);
		rb_define_singleton_method(rb_cBBS, "kbhit", bbs_kbhit, 1);

		// Set safe level to 2
		// We cannot have protection if the safe level > 2
		rb_set_safe_level(2);

		// Hook Ruby
		rb_add_event_hook(BBSRubyHook, RUBY_EVENT_LINE, 0);
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
	cStart = post;
	cEnd = post + pLen;
	if (ruby_script_range_detect(&cStart, &cEnd) == 0 || cStart == NULL || cEnd == NULL)
	{
		out_footer(" (找不到程式區段)",  "按任意鍵返回");
                return;
	}
	rb_load_file("empty.rb");

	// Check interface version
	/*
	float d;
	sscanf(TOCs_DATA[4], "%f", d);
	move(b_lines - 1, 0);
	char msgBuf[200]="";
	if (d == 0)
		sprintf(msgBuf, "\033[1;41m ● 程式未載明相容的Interface版本，可能發生不相容問題");
	else if (d < BBSRUBY_INTERFACE_VER)
		sprintf(msgBuf, "\033[1;41m ● 程式版本過舊，可能發生不相容問題");
	outs(msgBuf);
	int i;
	for(i=0;i<b_cols - strlen(msgBuf) + 7;i++)
		outs(" ");
	outs("\033[m");
	*/
	char *cpBuf = malloc(sizeof(char) * strlen(post) + 1);
	// char *evalBuf = malloc(sizeof(char) * strlen(post) + 1 + 10);
	strncpy(cpBuf, cStart, cEnd - cStart);
	cpBuf[cEnd - cStart + 1] = '\0';
	// sprintf(evalBuf, "begin\n%s\nend", cpBuf);
	out_footer("", "按任意鍵開始執行");

	//Before execution, preapre keyboard buffer
	//KB_QUEUE = rb_ary_new();
	NODE* root = rb_compile_string("BBSRuby", rb_str_new2(cpBuf), 1);
	error = ruby_exec_node(root, "BBSRuby");
	
	if (error == 0 || ABORT_BBSRUBY)
		out_footer(ABORT_BBSRUBY ? " (使用者中斷)" : " (程式結束)", "按任意鍵返回");
	else
	{
		print_exception();
	}
}
