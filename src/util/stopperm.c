/*-------------------------------------------------------*/
/* stopperm.c	( YZU WindTopBBS Ver 3.00 )		 */
/*-------------------------------------------------------*/
/* target : �s�y�B�@�{��			 	 */
/* create : 95/03/29				 	 */
/* update : 96/04/05				 	 */
/*-------------------------------------------------------*/


#undef	_ADMIN_C_
#include "bbs.h"

static int funo;
static char *kmail;
static int total;
static FILE *flog;

static void
reaper(fpath, lowid)
  char *fpath;
  char *lowid;
{
  int fd;

  char buf[256];
  ACCT acct;

  sprintf(buf, "%s/.ACCT", fpath);
  fd = open(buf, O_RDONLY, 0);
  if (fd < 0)
     return;

  if(read(fd, &acct, sizeof(acct))!=sizeof(acct))
  {
    close(fd);
    return;
  }
  close(fd);

  if(!strcmp(acct.email,kmail))
  {
    fprintf(flog,"%-13s\n",acct.userid);    
    total++;
  }
}

static void
traverse(fpath)
  char *fpath;
{
  DIR *dirp;
  struct dirent *de;
  char *fname, *str;

  if (!(dirp = opendir(fpath)))
  {
    return;
  }
  for (str = fpath; *str; str++);
  *str++ = '/';

  while ((de = readdir(dirp)))
  {
    fname = de->d_name;
    if (fname[0] > ' ' && fname[0] != '.')
    {
      strcpy(str, fname);
      reaper(fpath, fname);
    }
  }
  closedir(dirp);
}

static int 
same_mail2(mail,file)
  char *mail;
  char *file;
{
  int ch;
  char *fname, fpath[256];
  kmail = mail;
  total = 0;

  sprintf(fpath,"%s.%s",FN_SAMEEMAIL_LOG,file);

  flog = fopen(fpath, "w");
  if (flog == NULL)
    return 0;

  funo = open(".USR", O_RDWR | O_CREAT, 0600);

  if (funo < 0)
    return 0;

  strcpy(fname = fpath, "usr/@");
  fname = (char *) strchr(fname, '@');

  for (ch = 'a'; ch <= 'z'; ch++)
  {
    fname[0] = ch;
    fname[1] = '\0';
    traverse(fpath);
  }
  for (ch = '0'; ch <= '9'; ch++)
  {
    fname[0] = ch;
    fname[1] = '\0';
    traverse(fpath);
  }  
  fclose(flog);
  close(funo);
  return total;
}

void
keeplog(fnlog, board, title, mode)
  char *fnlog;
  char *board;
  char *title;
  int mode;		/* 0:load 1:rename 2:unlink 3:mark */
{
  HDR hdr;
  char folder[128], fpath[128];
  int fd;
  FILE *fp;

  if (!board)
    board = BRD_SYSTEM;

  sprintf(folder, "brd/%s/.DIR", board);
  fd = hdr_stamp(folder, 'A', &hdr, fpath);
  if (fd < 0)
    return;

    fp = fdopen(fd, "w");
    fprintf(fp, "�@��: SYSOP (" SYSOPNICK ")\n���D: %s\n�ɶ�: %s\n",
      title, ctime(&hdr.chrono));
    f_suck(fp, fnlog);
    fclose(fp);
    close(fd);

  strcpy(hdr.title, title);
  strcpy(hdr.owner, "SYSOP");
  strcpy(hdr.nick, SYSOPNICK);
  if (mode == 3)
    hdr.xmode |= POST_MARKED;
  fd = open(folder, O_WRONLY | O_CREAT | O_APPEND, 0600);
  if (fd < 0)
  {
    unlink(fpath);
    return;
  }
  write(fd, &hdr, sizeof(HDR));
  close(fd);
}


int
acct_load(acct, userid)
  ACCT *acct;
  char *userid;
{
  int fd;

  usr_fpath((char *) acct, userid, ".ACCT");
  fd = open((char *) acct, O_RDONLY);
  if (fd >= 0)
  {
    read(fd, acct, sizeof(ACCT));
    close(fd);
  }
  return fd;
}


void
acct_save(acct)
  ACCT *acct;
{
  int fd;
  char fpath[80];

  usr_fpath(fpath, acct->userid, ".ACCT");
  fd = open(fpath, O_WRONLY, 0600);	/* fpath �����w�g�s�b */
  if (fd >= 0)
  {
    write(fd, acct, sizeof(ACCT));
    close(fd);
  }
}


int
seek_log_email(mail)
  char *mail;
{
  EMAIL email;
  int pos=0,fd;
  fd = open(FN_VIOLATELAW_DB, O_RDONLY);
  while(fd)
  {
    lseek(fd, (off_t) (sizeof(email) * pos), SEEK_SET);
      if (read(fd, &email, sizeof(email)) == sizeof(email))
      {
        if(!strcmp(email.email,mail))
        {
          close(fd);
          return pos;
        }
        pos++;
      }
      else
      {
        close(fd);
        break;
      }  
  }
  close(fd);
  return -1;
}

void
deny_log_email(mail,deny)
  char *mail;
  time_t deny;
{
  EMAIL email;
  int pos;
  pos = seek_log_email(mail);
  if(pos >=0)
  {
    rec_get(FN_VIOLATELAW_DB, &email, sizeof(EMAIL), pos);
    if(deny > email.deny || deny == -1)
       email.deny = deny;
    email.times++;
    rec_put(FN_VIOLATELAW_DB, &email, sizeof(EMAIL), pos); 
  }
  else
  {
    memset(&email,0,sizeof(email));
    strcpy(email.email,mail);
    email.deny = deny;
    rec_add(FN_VIOLATELAW_DB, &email, sizeof(EMAIL));
  }
}

static void
deny_add_email(he,exer)
  ACCT *he;
  char *exer;
{
  char buf[128];
  time_t now;
  struct tm *p;

  time(&now);
  
  p = localtime(&now);
  str_lower(he->vmail, he->vmail);
  sprintf(buf, "%s # %02d/%02d/%02d %02d:%02d %s (%s)\n",
    he->vmail,
    p->tm_year % 100, p->tm_mon + 1, p->tm_mday,
    p->tm_hour, p->tm_min, "���v" ,exer);
  f_cat(FN_ETC_UNTRUST_ACL,buf);
}

static int
add_deny_exer(u,adm,cross,exer)
  ACCT *u;
  int adm;
  int cross;
  char *exer;
{
  FILE *fp;
  char buf[80];
  ACCT x;
  time_t now;
  int check_time;
  char *cselect=NULL,*cdays=NULL,*cmode=NULL;

  memcpy(&x, u, sizeof(ACCT));
  time(&now);
  check_time = (x.deny > now) ? 1 : 0; 

  if(!strncmp(u->justify,"reg:",4))
    adm = (adm & ~DENY_MODE_ALL)|DENY_MODE_GUEST;

  unlink(FN_STOP_LOG);
  fp = fopen(FN_STOP_LOG,"w");
  if(adm & DENY_SEL_OK)
  {
    x.deny = now;
    memcpy(u, &x, sizeof(x));
    acct_save(u);
    return adm;  
  }
  if(adm & DENY_SEL)
  {
    if(adm & DENY_SEL_TALK) cselect = "������";
    else if(adm & DENY_SEL_POST) cselect = " Cross Post";
    else if(adm & DENY_SEL_MAIL) cselect = "���o�s��H";
    else if(adm & DENY_SEL_AD)   cselect = "���o�s�i�H";
    else if(adm & DENY_SEL_SELL) cselect = "�c��D�k�ƪ�";
    fprintf(fp,"�d %s �H�ϯ��W%s�A�̯��W����",u->userid,cselect);
  }
  if((adm & (DENY_MODE_ALL)) && !(adm & DENY_MODE_GUEST))
  {
    if((adm & DENY_MODE_ALL) == DENY_MODE_ALL)
    {
      x.userlevel |= (PERM_DENYPOST | PERM_DENYTALK | PERM_DENYCHAT | PERM_DENYMAIL | PERM_DENYNICK);
      cmode = " Talk , Mail , \nPost , ���ʺ�";
    }
    else if(adm & DENY_MODE_POST)
    {
      x.userlevel |= (PERM_DENYPOST);
      cmode = " Post ";
    }
    else if(adm & DENY_MODE_TALK)
    {
      x.userlevel |= (PERM_DENYTALK|PERM_DENYCHAT);
      cmode = " Talk ";
    }    
    else if(adm & DENY_MODE_MAIL)
    {
      x.userlevel |= (PERM_DENYMAIL);
      cmode = " Mail ";
    }
    else if(adm & DENY_MODE_NICK)                                               
    {
      x.userlevel |= (PERM_DENYNICK);
      cmode = "���ʺ�";
    }    
    fprintf(fp,"%s�v��",cmode);
  }
  if(adm & DENY_MODE_GUEST)
  {
    x.userlevel |= (PERM_DENYPOST | PERM_DENYTALK | PERM_DENYCHAT | PERM_DENYMAIL | PERM_DENYSTOP);
    x.userlevel &= ~(PERM_BASIC | PERM_VALID);
    x.deny += 86400 * 31;
    cmode = " Talk , Mail , \nPost , ���ʺ�";
    fprintf(fp,"%s�v���A�v������ guest �A�ä��_�v�A�ëO�d�b���A\n�� E-mail�G%s �ä��o�b�������U�C\n\n",cmode,u->vmail);
    deny_add_email(u,exer);
  }
  if((adm & DENY_DAYS) && !(adm & DENY_MODE_GUEST))
  {
    if(adm & DENY_DAYS_1) { cdays = "�@�P��";x.deny = now + 86400 * 7;}
    else if(adm & DENY_DAYS_2) { cdays = "��P��";x.deny = now + 86400 * 14;}
    else if(adm & DENY_DAYS_3) { cdays = "�ѬP��";x.deny = now + 86400 * 21;}
    else if(adm & DENY_DAYS_4) { cdays = "�@�Ӥ�";x.deny = now + 86400 * 31;}
    else if(adm & DENY_DAYS_5) { cdays = "";x.deny = now + 86400 * 31;x.userlevel |= PERM_DENYSTOP;}
    fprintf(fp,"%s\n",cdays);
    if(adm & DENY_DAYS_5)
      fprintf(fp,"����: �ä��_�v\n\n");
    else
      fprintf(fp,"����: %s%s�A�����@�L�۰ʴ_�v�C\n\n",check_time ? "�W���B�@�����֥[":"�q���Ѱ_",cdays);
  }
  fprintf(fp,"\033[1;32m�� Origin: \033[1;33m%s \033[1;37m<%s>\n\033[1;31m�� From: \033[1;36m%s\033[m\n",BOARDNAME,MYHOSTNAME,MYHOSTNAME); 

  fclose(fp);
  sprintf(buf, "[%s�B�@] %s %s",cross ? "�s��":"", u->userid,cselect);
  keeplog(FN_STOP_LOG, BRD_VIOLATELAW, buf, 3);    
  usr_fpath(buf,x.userid,FN_STOPPERM_LOG);
  unlink(buf);
  fp = fopen(buf,"a+");
  f_suck(fp,FN_STOP_LOG);
  fclose(fp);


  memcpy(u, &x, sizeof(x));
  acct_save(u);
  return adm;
}


static void
setup(id,email,mode,exer,file)
  char *id;
  char *email;
  int mode;
  char *exer;
  char *file;
{
  ACCT x,*u;
  int i, num;
  FILE *flog;
  char buf[80];
  char fpath[128];
  
  u = &x;

  sprintf(fpath,"%s.%s",FN_SAMEEMAIL_LOG,file);

  num = same_mail2(email,file);
  flog = fopen(fpath, "r");
      
  if (!flog) 
    return;
      
  for(i=1;i<=num;i++)
  {
    fscanf(flog,"%13s",buf);
    acct_load(u,buf);

    if(u != NULL)
    {
      if(strcmp(u->userid,id))
        add_deny_exer(u,mode,1,exer);      
      else 
        add_deny_exer(u,mode,NULL,exer);
    }
  }
      
  if(mode & DENY_MODE_ALL)     
    deny_log_email(email,(x.userlevel & PERM_DENYSTOP) ? -1 : x.deny);
  fclose(flog); 
  unlink(fpath);
}

/* stopperm id email  mode  */


int 
main(argc, argv)
  int argc;
  char *argv[];  
{
  char buf[256];
  if(argc > 5 )
  {
    setup(argv[1],argv[2],atoi(argv[3]),argv[4],argv[5]);
    sprintf(buf,"mail %s.bbs@" MYHOSTNAME " < " FN_STOPPERM_MAIL,argv[4]);
    system(buf);
  }
  else
    system("mail bbs@" MYHOSTNAME " < " FN_STOPPERM_MAIL);
  return 0;
}
