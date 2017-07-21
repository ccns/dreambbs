/*-------------------------------------------------------*/
/* util/mailpost.c	( NTHU CS MapleBBS Ver 2.36 )	 */
/*-------------------------------------------------------*/
/* target : (1) general user E-mail post 到看板		 */
/*          (2) BM E-mail post 到精華區			 */
/*          (3) 自動審核身份認證信函之回信		 */
/* create : 95/03/29				 	 */
/* update : 97/03/29				 	 */
/*-------------------------------------------------------*/
/* notice : brdshm (board shared memory) synchronize     */
/*-------------------------------------------------------*/


#include	<stdio.h>
#include	<ctype.h>
#include	<sys/file.h>
#include	<fcntl.h>
#include	<time.h>
#include	<signal.h>


#include	"bbs.h"
#include	"decode.ic"


extern char *crypt();


//#define	LOG_FILE	"run/mailog"
#define		LOG_FILE	FN_BBSMAILPOST_LOG

#define	JUNK		0
#define	NET_SAVE	1
#define	LOCAL_SAVE	2
#define	DIGEST		3

static int mymode = JUNK;


static ACCT myacct;
static char myfrom[128], mysub[128], myname[128], mypasswd[128], myboard[128], mytitle[128];


/* ----------------------------------------------------- */
/* .BOARDS shared memory (cache.c)			 */
/* ----------------------------------------------------- */


static int
brd_fetch(bname, brd)
  char *bname;
  BRD *brd;
{
  FILE *fp;

  fp = fopen(".BRD", "r");
  if (!fp)
    return -1;

  while (fread(brd, sizeof(BRD), 1, fp) == 1)
  {
    if (!strcasecmp(bname, brd->brdname))
    {
      fclose(fp);
      return 0;
    }
  }

  fclose(fp);
  return -1;
}


#if 0
int
haspostperm(bname)
  char *bname;
{
  register int i;

  if (currmode & MODE_DIGEST)
    return 0;

  if (!ci_strcmp(bname, DEFAULT_BOARD))
    return 1;

  if (!HAS_PERM(PERM_POST))
    return 0;

  if (!(i = getbnum(bname)))
    return 0;

  i = bcache[i - 1].level;

  /* 秘密看板特別處理 */

  if ((i & 0xffff) == PERM_SYSOP)
  {
    currmode |= MODE_SECRET;
    return 1;
  }

  return (HAS_PERM(i & ~PERM_POSTMASK));
}
#endif


/* ----------------------------------------------------- */
/* buffered I/O for stdin				 */
/* ----------------------------------------------------- */


#define POOL_SIZE	4096
#define LINE_SIZE	512


static char pool[POOL_SIZE];
static char mybuf[LINE_SIZE];
static int pool_size = POOL_SIZE;
static int pool_ptr = POOL_SIZE;


static int
readline(buf)
  char *buf;
{
  register int ch;
  register int len, bytes;

  len = bytes = 0;
  do
  {
    if (pool_ptr >= pool_size)
    {
/*    pool_size = read(0, pool, POOL_SIZE); */
      ch = fread(pool, 1, POOL_SIZE, stdin);
      if (ch <= 0)
	return 0;

      pool_size = ch;
      pool_ptr = 0;
    }
    ch = pool[pool_ptr++];
    bytes++;

    if (ch == '\r')
      continue;

    buf[len++] = ch;
  } while (ch != '\n' && len < (LINE_SIZE - 1));

  buf[len] = '\0';

  if (buf[0] == '.' && (buf[1] == '\n' || buf[1] == '\0'))
    return 0;

  return bytes;
}


/* ----------------------------------------------------- */
/* record run/mailog for management			 */
/* ----------------------------------------------------- */


static void
mailog(mode, msg)
  char *mode, *msg;
{
  FILE *fp;

  if ((fp = fopen(LOG_FILE, "a")))
  {
    time_t now;
    struct tm *p;

    time(&now);
    p = localtime(&now);
    fprintf(fp, "%02d/%02d %02d:%02d:%02d <%s> %s\n",
      p->tm_mon + 1, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec,
      mode, msg);
    fclose(fp);
  }
}


/* ----------------------------------------------------- */
/* string subroutines					 */
/* ----------------------------------------------------- */


/* static */
void
str_lower(t, s)
  char *t, *s;
{
  register int ch;

  do
  {
    ch = *s++;
    *t++ = (ch >= 'A' && ch <= 'Z') ? ch | 32 : ch;
  } while (ch);
}


static void
str_cut(dst, src)
  char *dst;
  char *src;
{
  int cc;

  for (;;)
  {
    cc = *src++;
    if (!cc)
    {
      *dst = '\0';
      return;
    }

    if (cc == ' ')
    {
      while (*src == ' ')
	src++;

      while ((cc = *src++))
      {
	if (cc == ' ' || cc == '\n' || cc == '\r')
	  break;
	*dst++ = cc;
      }

      *dst = '\0';
      return;
    }
  }
}


static int
ci_strcmp(s1, s2)
  register char *s1, *s2;
{
  register int c1, c2, diff;

  do
  {
    c1 = *s1++;
    c2 = *s2++;
    if (c1 >= 'A' && c1 <= 'Z')
      c1 |= 32;
    if (c2 >= 'A' && c2 <= 'Z')
      c2 |= 32;
    if ((diff = c1 - c2))
      return (diff);
  } while (c1);
  return 0;
}


static void
strip_ansi(buf, str)
  char *buf, *str;
{
  register int ch, ansi;

  for (ansi = 0; (ch = *str); str++)
  {
    if (ch == '\n')
    {
      break;
    }
    else if (ch == 27)
    {
      ansi = 1;
    }
    else if (ansi)
    {
      if (!strchr("[01234567;", ch))
	ansi = 0;
    }
    else
    {
      *buf++ = ch;
    }
  }
  *buf = '\0';
}


static int
valid_ident(ident)
  char *ident;
{
  static char *invalid[] = {"bbs@", "@bbs", "unknown@", "root@", "gopher@",
  "guest@", "@ppp", "@slip", NULL};
  char buf[128], *str;
  int i;

  str_lower(buf, ident);
  for (i = 0; (str = invalid[i]); i++)
  {
    if (strstr(buf, str))
      return 0;
  }
  return 1;
}


static void
str_strip(str)      /* remove trailing space */
  char *str;
{
  int ch;

	do
	{
	  ch = *(--str);
	} while (ch == ' ' || ch == '\t');
  str[1] = '\0';
}


/* ------------------------------------------------------- */
/* 記錄驗證資料：user 有可能正在線上，所以寫入檔案以保周全 */
/* ------------------------------------------------------- */


static int
acct_fetch(userid)
  char *userid;
{
  int fd;
  char fpath[80], buf[80];

  str_lower(buf, userid);
  sprintf(fpath, "usr/%c/%s/.ACCT", *buf, buf);
  fd = open(fpath, O_RDWR, 0600);
  if (fd >= 0)
  {
    if (read(fd, &myacct, sizeof(ACCT)) != sizeof(ACCT))
    {
      close(fd);
      fd = -1;
    }
  }
  return fd;
}


static int
rec_append(fpath, data, size)
  char *fpath;
  void *data;
  int size;
{
  register int fd;

  if ((fd = open(fpath, O_WRONLY | O_CREAT | O_APPEND, 0600)) < 0)
    return -1;

  write(fd, data, size);
  close(fd);

  return 0;
}


/* ----------------------------------------------------- */
/* chrono ==> file name (32-based)			 */
/* 0123456789ABCDEFGHIJKLMNOPQRSTUV			 */
/* ----------------------------------------------------- */

#if 0
static char radix32[32] = {
  '0', '1', '2', '3', '4', '5', '6', '7',
  '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
  'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
  'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V',
};
#endif

/* static */
void
archiv32(chrono, fname)
  register time_t chrono;	/* 32 bits */
  register char *fname;		/* 7 chars */
{
  register char *str;

  str = fname + 7;
  *str = '\0';
  for (;;)
  {
    *(--str) = radix32[chrono & 31];
    if (str == fname)
      return;
    chrono >>= 5;
  }
}


/* static */
time_t
chrono32(str)
  register char *str;		/* 0123456 */
{
  register time_t chrono;
  register int ch;

  chrono = 0;
  while ((ch = *str++))
  {
    ch -= '0';
    if (ch >= 10)
      ch -= 'A' - '0' - 10;
    chrono = (chrono << 5) + ch;
  }
  return chrono;
}


/* static */
int
hash32(str)
  unsigned char *str;
{
  int xo, cc;

  xo = 1048583;			/* a big prime number */
  while ((cc = *str++))
  {
    xo = (xo << 5) - xo + cc;	/* 31 * xo + cc */
  }
  return (xo & 0x7fffffff);
}


/* static */
int
str_hash(str, seed)
  char *str;
  int seed;
{
  int cc;

  while ((cc = *str++))
  {
    seed = (seed << 5) - seed + cc;     /* 31 * seed + cc */
  }
  return (seed & 0x7fffffff);
}


/* ------------------------------------------ */
/* mail / post 時，依據時間建立檔案，加上郵戳 */
/* ------------------------------------------ */
/* Input: fpath = directory;		      */
/* Output: fpath = full path;		      */
/* ------------------------------------------ */


/* static */
void
str_stamp(str, chrono)
  char *str;
  time_t *chrono;
{
  register struct tm *ptime;

  ptime = localtime(chrono);
  /* Thor.990329: y2k */
  sprintf(str, "%02d/%02d/%02d",
    ptime->tm_year % 100, ptime->tm_mon + 1, ptime->tm_mday);
}


/* ----------------------------------------------------- */
/* Link() : link() cross partition / disk		 */
/* ----------------------------------------------------- */


static int
f_copy(src, dst, mode)
  char *src, *dst;
  int mode;
{
  int fsrc, fdst, ret;

  ret = -1;

  if ((fsrc = open(src, O_RDONLY)) >= 0)
  {
    if ((fdst = open(dst, O_WRONLY | O_CREAT | mode, 0600)) >= 0)
    {
      char pool[BLK_SIZ];

      src = pool;
      for (;;)
      {
	ret = read(fsrc, src, BLK_SIZ);
	if (ret <= 0)
	  break;
	ret = write(fdst, src, ret);
	if (ret < 0)
	  break;
      }

      close(fdst);
    }
    close(fsrc);
  }
  return ret;
}


static int
Link(src, dst)
  char *src, *dst;
{
  int ret;

  if ((ret = link(src, dst)))
  {
    if (errno != EEXIST)
      ret = f_copy(src, dst, O_EXCL);
  }
  return ret;
}


/* ----------------------------------------------------- */
/* hdr_stamp - create unique HDR based on timestamp	 */
/* ----------------------------------------------------- */
/* fpath - directory					 */
/* token - A / F / 0					 */
/* ----------------------------------------------------- */
/* return : open() fd (not close yet) or link() result	 */
/* ----------------------------------------------------- */


/* static */
int
hdr_stamp(folder, token, hdr, fpath)
  register char *folder;
  register int token;
  register HDR *hdr;
  register char *fpath;
{
  register char *fname, *family=NULL;
  register int rc;
  char *flink, buf[128];

  flink = NULL;
  if (token & HDR_LINK)
  {
    flink = fpath;
    fpath = buf;
  }

  fname = fpath;
  while ((rc = *folder++))
  {
    *fname++ = rc;
    if (rc == '/')
      family = fname;
  }
  if (*family != '.')
  {
    fname = family;
    family -= 2;
  }
  else
  {
    fname = family + 1;
    *fname++ = '/';
  }
  
  if (token &= 0xdf)		/* 變大寫 */
  {
    *fname++ = token;
  }
  else
  {
    *fname = *family = '@';
    family = ++fname;
  }

  token = time(NULL);

  for (;;)
  {
    *family = radix32[token & 31];
    archiv32(token, fname);

    if (flink)
      rc = Link(flink, fpath);
    else
      rc = open(fpath, O_WRONLY | O_CREAT | O_EXCL, 0600);

    if (rc >= 0)
    {
      memset(hdr, 0, sizeof(HDR));
      hdr->chrono = token;
      str_stamp(hdr->date, &hdr->chrono);
      strcpy(hdr->xname, --fname);
      break;
    }

    if (errno != EEXIST)
      break;

    token++;
  }

  return rc;
}


static void
justify_user()
{
  char buf[128];
  HDR mhdr;
  int fd;

  sprintf(buf, "usr/%c/%s/.DIR", *myname, myname);
  if (!hdr_stamp(buf, HDR_LINK, &mhdr, "etc/justified"))
  {
  strcpy(mhdr.title, "您已經通過身分認證了！");
  strcpy(mhdr.owner, "SYSOP");
  mhdr.xmode = MAIL_NOREPLY;
  rec_append(buf, &mhdr, sizeof(mhdr));
  }

  sprintf(buf, "usr/%c/%s/email", *myname, myname);
  fd = open(buf, O_WRONLY | O_CREAT | O_APPEND, 0600);
  if (fd >= 0)
  {
    char *base, *str;
    int count;

    count = pool_ptr;
    base = pool;
    str = base + count;
    str = strstr(str, "\n\n");
    if (str != NULL)
    {
      count = str - pool + 1;
    }
    write(fd, pool, count);
    close(fd);
  }

  myacct.vtime = time(&myacct.tvalid);
  strcpy(myacct.justify, myfrom);
  myfrom[sizeof(myacct.justify)-1] = 0;
  strcpy(myacct.vmail, myacct.email);
  myacct.userlevel |= PERM_VALID;
}


static void
verify_user(magic)
  char *magic;
{
  char *ptr, *next, buf[80];
  int fh;
  char buf2[512];
  int done;

/* printf("V1: %s\n", magic); */

  done = 0;
  strcpy(buf2, magic);

  if (valid_ident(myfrom) && (ptr = strchr(magic, '(')))
  {
    *ptr++ = '\0';

    fh = acct_fetch(magic);
    if (fh < 0)
    {
      sprintf(buf, "BBS user <%s> unknown: %s", magic, myfrom);
      mailog("verify", buf);
      puts(buf);
      return;
    }

#if 0
    if (next = (char *) strchr(ptr, ':'))	/* old */
    {
  ushort checksum, userno;

      *next++ = '\0';
      userno = atoi(ptr) - MAGIC_KEY;
      if (userno != myacct.userno)
      {
	close(fh);
	printf("BBS user <%s> mismatch\n", magic);
	return;
      }

      if (ptr = (char *) strchr(next, ')'))
      {
	*ptr = '\0';
	checksum = atoi(next);

#ifdef	CHECK_RETURN_ADDRESS
	ptr = myfrom;		/* return address maybe != target address */
#else
	ptr = myacct.email;
#endif

	while (ch = *ptr)
	{
	  ptr++;
	  if (ch <= ' ')
	    break;
	  if (ch >= 'A' && ch <= 'Z')
	    ch |= 0x20;
	  userno = (userno << 1) ^ ch;
	}
	if (userno == checksum)
	{
	  str_lower(myname, magic);
	  justify_user();
	  lseek(fh, (off_t) 0, SEEK_SET);
	  write(fh, &myacct, sizeof(ACCT));
	  sprintf(mybuf, "[%s]%s", myacct.userid, myfrom);
	  mailog("verify", mybuf);
	  done = 1;
	}
      }

    }
    else	/* new */
#endif

    {
/* printf("V2: %s\n", ptr); */

      if ((next = (char *) strchr(ptr, ')')))
      {
	*next = 0;

	if (strstr(next+1, "[VALID]"))
	{
	if (str_hash(myacct.email, myacct.vtime) == chrono32(ptr))
	{
	  str_lower(myname, magic);
	  justify_user();
	  lseek(fh, (off_t) 0, SEEK_SET);
	  write(fh, &myacct, sizeof(ACCT));
	  sprintf(buf, "[%s]%s", myacct.userid, myfrom);
	  mailog("valid", buf);
	  done = 1;
	}
	}
	else
	{

	str_lower(buf, myacct.email);
/* printf("V3: %x %x\n", hash32(buf), chrono32(ptr)); */
	if (hash32(buf) == chrono32(ptr))
	{
	  str_lower(myname, magic);
	  justify_user();
	  lseek(fh, (off_t) 0, SEEK_SET);
	  write(fh, &myacct, sizeof(ACCT));
	  sprintf(buf, "[%s]%s", myacct.userid, myfrom);
	  mailog("verify", buf);
	  done = 1;
	}
	}
      }
    }
	close(fh);
  }
  
  if (!done)
  {
    sprintf(buf, "Invalid [%s] %s", buf2, myfrom);
    mailog("verify", buf);
  }
}


static int
post_article()
{
  int fd;
  FILE *fp;
  HDR hdr;
  char fpath[80], buf[128];

  if (mymode == JUNK)
  {
    pool_ptr = 0;
    if (!readline(mybuf))
      exit(0);

    if (!*myname)
      strcpy(myname, "<mailpost>");

    if (!*mytitle)
      strcpy(mytitle, *mysub ? mysub : "<< 原信照登 >>");
  }

  sprintf(fpath, "brd/%s/.DIR", mymode == JUNK ? BRD_JUNK : myboard);

#ifdef	DEBUG
  printf("dir: %s\n", fpath);
#endif

  fd = hdr_stamp(fpath, 'A', &hdr, buf);
  if (fd < 0)
  {
    sprintf(buf, "file error <%s>", fpath);
    mailog("mailpost", buf);
    return -1;
  }

  fp = fdopen(fd, "w");

#ifdef	DEBUG
  printf("post to %s\n", buf);
#endif

  if (mymode != JUNK)
  {
    fprintf(fp, "作者: %s (%s) %s: %s\n標題: %s\n時間: %s\n",
      myname, myacct.username, (mymode == LOCAL_SAVE ? "站內" : "看板"),
      myboard, mytitle, ctime(&hdr.chrono));

    hdr.xmode = (mymode == LOCAL_SAVE ? POST_EMAIL : POST_EMAIL | POST_OUTGO);
  }

  do
  {
    fputs(mybuf, fp);
  } while (readline(mybuf));
  fprintf(fp, "\n--\n※ Origin: %s ◆ Mail: %s\n", BOARDNAME, myfrom);
  fclose(fp);

  strcpy(hdr.owner, myname);
  if (mymode != JUNK)
    strcpy(hdr.nick, myacct.username);
  mytitle[TTLEN] = '\0';
  strcpy(hdr.title, mytitle);
  rec_append(fpath, &hdr, sizeof(hdr));

  if ((mymode == NET_SAVE) && (fp = fopen("innd/out.bntp", "a")))
  {
    fprintf(fp, "%s\t%s\t%s\t%s\t%s\n",
      myboard, hdr.xname, hdr.owner, hdr.nick, hdr.title);
    fclose(fp);
  }

  if (mymode != JUNK)
  {
    sprintf(buf, "[%s]%s => %s", myname, myfrom, myboard);
    mailog("mailpost", buf);
  }

  return 0;
}


/* ----------------------------------------------------- */
/* E-mail post to gem					 */
/* ----------------------------------------------------- */


static int
digest_article()
{
  /* return post_article();*/	/* quick & dirty */
  /* Thor.0606: post 到 精華區資源回收筒 */
  int fd;
  FILE *fp;
  HDR hdr;
  char fpath[80], buf[128];

  if (mymode == JUNK)
  {
/*    pool_ptr = 0;
    if (!readline(mybuf)) */
      exit(0);
/*
    if (!*myname)
      strcpy(myname, "<mailpost>");

    if (!*mytitle)
      strcpy(mytitle, *mysub ? mysub : "<< 原信照登 >>");
*/
  }

  sprintf(fpath, "gem/brd/%s/.GEM", myboard);

#ifdef	DEBUG
  printf("dir: %s\n", fpath);
#endif

  fd = hdr_stamp(fpath, 'A', &hdr, buf);
  if (fd < 0)
  {
    sprintf(buf, "file error <%s>", fpath);
    mailog("mailpost", buf);
    return -1;
  }

  fp = fdopen(fd, "w");

#ifdef	DEBUG
  printf("gem to %s\n", buf);
#endif

/*
  if (mymode != JUNK)
  {
*/
    fprintf(fp, "作者: %s (%s) %s: %s\n標題: %s\n時間: %s\n",
      myname, myacct.username, (mymode == LOCAL_SAVE ? "站內" : "看板"),
      myboard, mytitle, ctime(&hdr.chrono));

    hdr.xmode =  POST_EMAIL;
/*
    hdr.xmode = (mymode == LOCAL_SAVE ? POST_EMAIL : POST_EMAIL | POST_OUTGO);
  }
*/

  do
  {
    fputs(mybuf, fp);
  } while (readline(mybuf));
  fprintf(fp, "\n--\n※ Origin: %s ◆ Mail: %s\n", BOARDNAME, myfrom);
  fclose(fp);

  strcpy(hdr.owner, myname);
/*
  if (mymode != JUNK)
*/
    strcpy(hdr.nick, myacct.username);
  mytitle[TTLEN] = '\0';
  strcpy(hdr.title, mytitle);
  rec_append(fpath, &hdr, sizeof(hdr));
/*
  if ((mymode == NET_SAVE) && (fp = fopen("innd/out.bntp", "a")))
  {
    fprintf(fp, "%s\t%s\t%s\t%s\t%s\n",
      myboard, hdr.xname, hdr.owner, hdr.nick, hdr.title);
    fclose(fp);
  }
*/
/*
  if (mymode != JUNK)
  {
*/
    sprintf(buf, "[%s]%s => %s", myname, myfrom, myboard);
    mailog("mailpost", buf);
/*
  }
*/
  return 0;
}


static int
mailpost()
{
  int fh, dirty;
  char *ip, *ptr, *token, *key, buf[80];
  BRD brd;

  /* parse header */

  if (!readline(mybuf))
    return 0;

  if (strncasecmp(mybuf, "From ", 5)) 
    return post_article();	/* junk */

  dirty = *myfrom = *mysub = *myname = *mypasswd = *myboard = *mytitle = 0;

  while (!*myname || !*mypasswd || !*myboard || !*mytitle)
  {
    if (mybuf[0] == '#')
    {
      key = mybuf + 1;

      /* remove trailing space */

      if ((ptr = strchr(key, '\n')))
      {
	str_strip(ptr);
      }

      /* split token & skip leading space */

      if ((token = strchr(key, ':')))
      {
	str_strip(token);

	do
	{
	  fh = *(++token);
	} while (fh == ' ' || fh == '\t');
      }

      if (!ci_strcmp(key, "name"))
      {
	strcpy(myname, token);
      }
      else if (!ci_strcmp(key, "passwd") || !ci_strcmp(key, "password") || !ci_strcmp(key, "passward"))
      {
	strcpy(mypasswd, token);
      }
      else if (!ci_strcmp(key, "board"))
      {
	strcpy(myboard, token);
      }
      else if (!ci_strcmp(key, "title") || !ci_strcmp(key, "subject"))
      {
	strip_ansi(mytitle, token);
      }
      else if (!ci_strcmp(key, "digest"))
      {
	mymode = DIGEST;
      }
      else if (!ci_strcmp(key, "local"))
      {
	mymode = LOCAL_SAVE;
      }
    }
    else if (!strncasecmp(mybuf, "From", 4))
    {
      str_lower(myfrom, mybuf + 4);
      if (strstr(myfrom, "mailer-daemon"))	/* junk */
      {
	strcpy(mytitle, "<< 系統退信 >>");
	return post_article();
      }

      if ((ip = strchr(mybuf, '<')) && (ptr = strrchr(ip, '>')))
      {
	*ptr = '\0';
	if (ip[-1] == ' ')
	  ip[-1] = '\0';
	ptr = (char *) strchr(mybuf, ' ');
	while (*++ptr == ' ');
	sprintf(myfrom, "%s (%s)", ip + 1, ptr);
      }
      else
      {
#if 0
	strtok(mybuf, " ");
	strcpy(myfrom, (char *) strtok(NULL, " "));
#endif
	str_cut(myfrom, mybuf);
      }
    }
    else if (!strncmp(mybuf, "Subject: ", 9))
    {
      /* audit justify mail */

      str_decode(mybuf);
      /* if (ptr = strstr(mybuf, "[MapleBBS]To ")) */
      /* Thor.981012: 集中於 config.h 定義 */
      if ((ptr = strstr(mybuf, TAG_VALID)))
      {
        /* gslin.990101: TAG_VALID 長度不一定 */
        verify_user(ptr + sizeof(TAG_VALID) - 1);
	/* verify_user(ptr + 13); */
	return 1;		/* eat mail queue */
      }

      if ((ptr = strchr(token = mybuf + 9, '\n')))
	*ptr = '\0';
      strip_ansi(mysub, token);
    }

    if ((++dirty > 70) || !readline(mybuf))
    {
      mymode = JUNK;
      return post_article();	/* junk */
    }
  }

  dirty = 0;

  /* check if the userid is in our bbs now */

  fh = acct_fetch(myname);
  if (fh < 0)
  {
    sprintf(buf, "BBS user <%s> not existed", myname);
    mailog("mailpost", buf);
    puts(buf);
    return -1;
  }

  /* check password */

  key = crypt(mypasswd, myacct.passwd);
  if (strncmp(key, myacct.passwd, PASSLEN))
  {
    close(fh);
    sprintf(buf, "BBS user <%s> password incorrect", myname);
    mailog("mailpost", buf);
    puts(buf);
    return -1;
  }

#ifdef	MAIL_POST_VALID
  if (!(myacct.userlevel & PERM_VALID) && valid_ident(myfrom))
  {

    /* ------------------------------ */
    /* 順便記錄 user's E-mail address */
    /* ------------------------------ */

    str_lower(myname, myname);
    justify_user();
    dirty = YEA;
  }
#endif

  /* check if the board is in our bbs now */

  if (brd_fetch(myboard, &brd))
  {
    close(fh);
    sprintf(buf, "No such board [%s]", myboard);
    mailog("mailpost", buf);
    puts(buf);
    return -1;
  }

  strcpy(myboard, brd.brdname);

  /* check permission */

  if (mymode != DIGEST)
  {
    if (mymode != LOCAL_SAVE)
      mymode = NET_SAVE;

    /* if (brd.battr & BRD_NOCOUNT == 0) */
    /* Thor.981123: lkchu patch: mailpost 文章數不增加問題 */
    if (!(brd.battr & BRD_NOCOUNT))
    {
      myacct.numposts++;
      dirty = YEA;
    }
  }

  while (mybuf[0] && mybuf[0] != '\n')
  {
    if (!readline(mybuf))
      return 0;
  }

  while (mybuf[0] == '\n')
  {
    if (!readline(mybuf))
      return 0;
  }

  if (dirty && mybuf[0])
  {
    lseek(fh, (off_t) 0, SEEK_SET);
    write(fh, &myacct, sizeof(ACCT));
  }
  close(fh);

  strcpy(myname, myacct.userid);
  if (mybuf[0])
    return (mymode == DIGEST) ? digest_article() : post_article();

  mymode = JUNK;
  return post_article();
}


static void
sig_catch(sig)
  int sig;
{
  char buf[40];

  sprintf(buf, "signal [%d]", sig);
  mailog("mailpost", buf);
  exit(0);
}

int
main()
{
  setgid(BBSGID);
  setuid(BBSUID);
  chdir(BBSHOME);

  signal(SIGBUS, sig_catch);
  signal(SIGSEGV, sig_catch);
  signal(SIGPIPE, sig_catch);

/*
*/

  if (mailpost())
  {
    /* eat mail queue */
    while (fread(pool, 1, POOL_SIZE, stdin) > 0)
    {
      sleep(10);
    }
    /* exit(-1); */
  }
  exit(0);
}
