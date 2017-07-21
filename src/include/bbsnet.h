/* Written by HYD & kids */ 

#define MY_BOOK         1                       /* �ڪ��̷R�Ҧ� */
#define ALL             2                       /* �����C��Ҧ� */

#define	COL		4			/* ��Ʀ��h�֭� */
#define	ROW		19			/* ���Ʀ��h�֭� */
#define NAME		17			/* ���W���ץ[�@ */
#define	TOTAL		( COL * ROW )		/* �@�����X�ӯ� */
#define DELIMITER	" \t"			/* �w�q���ɦr�� */
#define	CLS		"\x1b[2J\x1b[1;1f"	/* ansi macro   */
#define	YELLOW_BLUE	"\x1b[1;33;44m"		/* ���r�ũ�     */
#define RED		"\x1b[1;31m"		/* ����         */
#define	GREEN		"\x1b[1;32m"		/* ���         */
#define	YELLOW		"\x1b[1;33m"		/* ����         */
#define BLUE		"\x1b[1;34m"		/* �Ŧ�         */
#define MAGENTA		"\x1b[1;35m"		/* ������       */
#define CYAN		"\x1b[1;36m"		/* �H�Ŧ�       */
#define WHITE		"\x1b[1;37m"		/* �զ�         */
#define	NORMAL		"\x1b[m"		/* �٭�         */

#define GOTOXY( y, x )	printf( "\x1b[%d;%df", x, y );		/* ansi macro           */
#define SPACE		for( w = 0; w < NAME - 1; w ++ ) printf( " " )

#define MAX_PAGE        10                      /* �����C��̤j���� */
#define MAX_MYPAGE      1                       /* �ڪ��̷R�̤j���� */
#define MAX_MYBOOK      TOTAL * MAX_MYPAGE      /* �ڪ��̷R����� */
#define MAX_USE         10                      /* �̤j�ϥΪ̼ƭ��� */  

#define TELNET          "/usr/bin/telnet"               /* telnet ���|  */  
#define	DATAFILE	"/home/bbs/etc/bbsdata"         /* bbsnet File  */
#define WELCOME         "/home/bbs/etc/Welcome.bbsnet"  /* Welcome File */
#define HELP            "/home/bbs/etc/Help.bbsnet"     /* Help File    */
#define BOOK_FILE       ".BBS-BOOK"                     /* �ڪ��̷R�ɦW */ 

#ifdef LOG
#define LOGFILE		"/home/bbs/bbsnet.log"    /* bbsnet �O���ɦ�m */
#endif
#define SHM_KEY		1453			/* �г]�j�� 1024 ���ƭ� */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <termios.h>
#include <ctype.h>
#include <sys/wait.h>
#include <ncurses.h>
#ifdef LOG
#include <utmp.h>
struct tm *tm_buf3;
#endif
#include <sys/ipc.h>
#include <sys/shm.h>
unsigned int shm_key;
struct shmid_ds shmseg;
int shmid;

/* HYD */
void print_screen();
void message();
void cursor( u_short );
void cursor_move( char, char );
void finish();

/* kids */
void show_help();   
void load_data();
void add_book();
void del_book();
void save_book();
void show_file();
void pressanykey();
char load_from_file();
char load_from_shm();

char max_page, page = 1, last_page=1,has_error = 0, 
     cursor_x, cursor_y, flag = 0,buf_string[80],bbs_book[80],
     mode,shm_flag=0,del_flag=0,view_flag=1,max_use;   

unsigned int current = 1, counter = 1, last_current=1,w, y, z,
         myarray_size,array_size;

time_t now;
int fd;
FILE *fd1,*fd2;

struct termios modes, savemodes, tempmodes;
struct stat stat_data;
struct tm *tm_buf1, *tm_buf2;

/* kids: �[�J num:���x�s��, times:�s�u���� */
struct data
{
  char name[NAME + 2];
  char domain[32];
  char ip[16];
  char port[6];
  unsigned int num;
  unsigned int times;
}
*array,*shm_array,*my_array;


