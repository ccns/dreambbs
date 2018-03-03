/*-------------------------------------------------------*/
/* util/bbsmail.c	( NTHU CS MapleBBS Ver 3.00 )	 */
/*-------------------------------------------------------*/
/* target : 由 Internet 寄信給 BBS 站內使用者		 */
/* create : 95/03/29					 */
/* update : 97/03/29					 */
/*-------------------------------------------------------*/


#include "bbs.h"

#include <sysexits.h>

#define	ANTI_HTMLMAIL		/* itoc.021014: 擋 html_mail */
#define	ANTI_NOTMYCHARSETMAIL	/* itoc.030513: 擋 not-mycharset mail */

#define         LOG_FILE        FN_BBSMAILPOST_LOG

static void
mailog(msg)
  char *msg;
{
  FILE *fp;

  if ((fp = fopen(LOG_FILE, "a")))
  {
    time_t now;
    struct tm *p;

    time(&now);
    p = localtime(&now);
    fprintf(fp, "%02d/%02d %02d:%02d:%02d <bbsmail> %s\n",
      p->tm_mon + 1, p->tm_mday,
      p->tm_hour, p->tm_min, p->tm_sec,
      msg);
    fclose(fp);
  }
}


/* ----------------------------------------------------- */
/* user：shm 部份須與 cache.c 相容			 */
/* ----------------------------------------------------- */


static UCACHE *ushm;


static inline void
init_ushm()
{
  ushm = shm_new(UTMPSHM_KEY, sizeof(UCACHE));
}

static inline void
my_biff(char *userid)
{
  UTMP *utmp, *uceil;

  // XXX 這個 userid 已經轉成小寫了嗎? 好像是

  // XXX 沒效率? 唉
  ushm_init();

  utmp = ushm->uslot;
  uceil = (void *) utmp + ushm->offset;
  do
  {
    if (!strcasecmp(utmp->userid, userid))
    {
      utmp->ufo |= UFO_BIFF;

#ifdef  BELL_ONCE_ONLY
      return;
#endif
     }
  } while (++utmp <= uceil);
}

#if 1
static inline void
bbs_biff(userid)
  char *userid;
{
  UTMP *utmp, *uceil;
  usint offset;

  offset = ushm->offset;
  if (offset > (MAXACTIVE - 1) * sizeof(UTMP))	/* Thor.980805: 不然call不到 */
    offset = (MAXACTIVE - 1) * sizeof(UTMP);

  utmp = ushm->uslot;
  uceil = (void *) utmp + offset;

  do
  {
    if (!str_cmp(utmp->userid, userid))
      utmp->ufo |= UFO_BIFF;
  } while (++utmp <= uceil);
}
#endif

/* ----------------------------------------------------- */
/* 主程式						 */
/* ----------------------------------------------------- */

int
acct_load(acct, userid)
  ACCT *acct;
  char *userid;
{
  int fd;

  usr_fpath((char *) acct, userid, FN_ACCT);
  fd = open((char *) acct, O_RDONLY);
  if (fd >= 0)
  {
    /* Thor.990416: 特別注意, 有時 .ACCT的長度會是0 */
    read(fd, acct, sizeof(ACCT));
    close(fd);
  }
  return fd;
}

static int
mail2bbs(userid)
  char *userid;
{
  HDR hdr;
  char buf[512], title[256], sender[256], owner[256], nick[256], folder[64];
  char *str, *ptr, decode;
  int fd;
  FILE *fp;
  ACCT acct;

  /* 091209.cache: 限收站內信 */
  if(acct_load(&acct, userid) >= 0)
    {
       if (acct.ufo2 & UFO2_DEF_LOCALMAIL)	
       {
           sprintf(buf, "BBS user <%s> no income mail", userid);
           mailog(buf);
           return EX_NOUSER;
       }
    }

  /* check if the userid is in our bbs now */

  usr_fpath(folder, userid, NULL);
  if (!dashd(folder))
  {
    sprintf(buf, "BBS user <%s> not existed", userid);
    mailog(buf);
    return EX_NOUSER;
  }
  strcat(folder, "/" FN_DIR);

  if (rec_num(folder, sizeof(HDR)) >= MAX_BBSMAIL)
  {
    sprintf(buf, "BBS user <%s> over-spammed", userid);
    mailog(buf);
    return EX_NOUSER;
  }

  /* parse header */

  title[0] = sender[0] = owner[0] = nick[0] = '\0';
  decode = 0;

  while (fgets(buf, sizeof(buf), stdin))
  {
  start:
    if (!memcmp(buf, "From", 4))
    {
      if ((str = strrchr(buf, '<')) && (ptr = strrchr(str, '>')))
      {
	if (str[-1] == ' ')
	  str[-1] = '\0';

	if (strchr(++str, '@'))
	  *ptr = '\0';
	else					/* 由 local host 寄信 */
	  strcpy(ptr, "@" MYHOSTNAME);

	if ((ptr = (char *) strchr(buf, ' ')))
	{
	  while (*++ptr == ' ')
	    ;
	}

	if (ptr && *ptr == '"')
	{
	  char *right;

	  if ((right = strrchr(++ptr, '"')))
	    *right = '\0';

	  str_decode(ptr);
	  sprintf(sender, "%s (%s)", str, ptr);
	  strcpy(nick, ptr);
	  strcpy(owner, str);
	}
	else	/* Thor.980907: 沒有 finger name, 特別處理 */
	{
	  strcpy(sender, str);
	  strcpy(owner, str);
	}
      }
      else
      {
	strtok(buf, " \t\n\r");
	strcpy(sender, (char *) strtok(NULL, " \t\n\r"));

	if (!strchr(sender, '@'))	/* 由 local host 寄信 */
	  strcat(sender, "@" MYHOSTNAME);
	strcpy(owner, sender);
      }

#if 0
      /* itoc.040804: 擋信黑白名單 */
      str_lower(buf, owner);	/* 保持原 email 的大小寫 */
      if (ptr = (char *) strchr(buf, '@'))
      {
	*ptr++ = '\0';

	if (!acl_has(MAIL_ACLFILE, buf, ptr) ||
	  acl_has(UNMAIL_ACLFILE, buf, ptr) > 0)
	{
	  sprintf(buf, "SPAM %s", sender);
	  mailog(buf);
	  return EX_NOUSER;
	}
      }
#endif
    }

    else if (!memcmp(buf, "Subject: ", 9))
    {
      str_ansi(title, buf + 9, sizeof(title));
      /* str_decode(title); */ 
      /* LHD.051106: 若可能經 RFC 2047 QP encode 則有可能多行 subject */ 
      if (strstr(buf + 9, "=?")) 
      { 
        while (fgets(buf, sizeof(buf), stdin)) 
        { 
          if (buf[0] == ' ' || buf[0] == '\t')  /* 第二行以後會以空白或 TAB 開頭 */ 
            str_ansi(title + strlen(title), strstr(buf, "=?"), sizeof(title)); 
          else 
          { 
            str_decode(title);
            goto start; 
          } 
        } 
      } 
    }

    else if (!memcmp(buf, "Content-Type: ", 14))
    {
      str = buf + 14;

#ifdef ANTI_HTMLMAIL
      /* 一般 BBS 使用者通常只寄文字郵件或是從其他 BBS 站寄文章到自己的信箱
         而廣告信件通常是 html 格式或是裡面有夾帶其他檔案
         利用郵件的檔頭有 Content-Type: 的屬性把除了 text/plain (文字郵件) 的信件都擋下來 */
      if (*str != '\0' && str_ncmp(str, "text/plain", 10))
      {
	sprintf(buf, "ANTI-HTML [%d] %s => %s", getppid(), sender, userid);
	mailog(buf);
	return EX_NOUSER;
      }
#endif

#ifdef ANTI_NOTMYCHARSETMAIL
      {
	char charset[32];
	mm_getcharset(str, charset, sizeof(charset));
	if (str_cmp(charset, "big5") && str_cmp(charset, "us-ascii"))
	{
	  sprintf(buf, "ANTI-NONMYCHARSET [%d] %s => %s", getppid(), sender, userid);
	  mailog(buf);
	  return EX_NOUSER;
	}
      }
#endif
    }

    else if (!memcmp(buf, "Content-Transfer-Encoding: ", 27))
    {
      mm_getencode(buf + 27, &decode);
    }

    else if (buf[0] == '\n')
    {
      break;
    }
  }

  /* allocate a file for the new mail */

  fd = hdr_stamp(folder, 0, &hdr, buf);
  hdr.xmode = MAIL_INCOME;

  str_ncpy(hdr.owner, owner, sizeof(hdr.owner));
  str_ncpy(hdr.nick, nick, sizeof(hdr.nick));
  if (!title[0])
    sprintf(title, "來自 %.64s", sender);
  str_ncpy(hdr.title, title, sizeof(hdr.title));

  /* copy the stdin to the specified file */

  fp = fdopen(fd, "w");

  fprintf(fp, "作者: %s\n標題: %s\n時間: %s\n\n",
    sender, title, Btime(&hdr.chrono));

  while (fgets(buf, sizeof(buf), stdin))
  {
    if (decode && ((fd = mmdecode(buf, decode, buf)) > 0))
      buf[fd] = '\0';

    fputs(buf, fp);
  }

  fclose(fp);

  /* append the record to the .DIR */

  rec_add(folder, &hdr, sizeof(HDR));

  bbs_biff(userid);	/* itoc.021113: 通知 user 有新信件 */
//  my_biff(userid);

  /* Thor.980827: 加上 parent process id，以便抓垃圾信 */
  sprintf(buf, "[%d] %s => %s", getppid(), sender, userid); 
  mailog(buf);

  return 0;
}


static void
sig_catch(sig)
  int sig;
{
  char buf[512];

  while (fgets(buf, sizeof(buf), stdin))
    ;
  sprintf(buf, "signal [%d]", sig);
  mailog(buf);
  exit(0);
}


int
main(argc, argv)
  int argc;
  char *argv[];
{
  char buf[512];

  /* argv[1] is userid in bbs */

  if (argc < 2)
  {
    printf("Usage:\t%s <bbs_userid>\n", argv[0]);
    exit(-1);
  }

  setgid(BBSGID);
  setuid(BBSUID);
  chdir(BBSHOME);

  signal(SIGBUS, sig_catch);
  signal(SIGSEGV, sig_catch);
  signal(SIGPIPE, sig_catch);

  init_ushm();
  str_lower(buf, argv[1]);	/* 把 userid 換成小寫 */

  if (mail2bbs(buf))
  {
    /* eat mail queue */
    while (fgets(buf, sizeof(buf), stdin))
      ;
  }
  exit(0);
}
