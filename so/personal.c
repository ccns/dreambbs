/*-------------------------------------------------------*/
/* personal.c   ( CCNS BBS )  		                 */
/*-------------------------------------------------------*/
/* author : cat@ccns.ncku.edu.tw                         */
/* target : personal                                     */
/* create : 2004/12/12                                   */
/* update : NULL                                         */
/*-------------------------------------------------------*/

#undef	_MODES_C_
#include "bbs.h"


extern XZ xz[];
extern BCACHE *bshm;
static int mode = 0;	/* 0:email 1:brdtitle */
char msg[60];

static int
personal_log(personal,admin)
  PB *personal;
  int admin;		/* 0:¥Ó½Ð  1:¶}ªO  2:©Úµ´ */
{
  FILE *fp;
  time_t now;
  char tag[3][5] = {"¥Ó½Ð","¶}ªO","©Úµ´"};
  
  if(fp = fopen(FN_PERSONAL_LOG,"a+"))
  {
    time(&now);
    
    fprintf(fp,"%24.24s ",ctime(&now));
    fprintf(fp,"%s ",(admin) ? cuser.userid : personal->userid);
    fprintf(fp,"%s ",tag[admin]);
    if(!admin)
      fprintf(fp,"%s ªO\n",personal->brdname);
    else
      fprintf(fp,"%s ªº %s ªO\n",personal->userid,personal->brdname);
    fclose(fp);
  }

  return 0;
}

static int
belong(flist, key)
  char *flist;
  char *key;
{
  int fd, rc;

  rc = 0;
  fd = open(flist, O_RDONLY);
  if (fd >= 0)
  {
    mgets(-1);

    while (flist = mgets(fd))
    {
      str_lower(flist, flist);
      if (str_str(key, flist))
      {
        rc = 1;
        break;
      }
    }

    close(fd);
  }
  return rc;
}


static int
is_badid(userid)
  char *userid;
{
  int ch;
  char *str;

  if (strlen(userid) < 2)
    return 1;

  if (!is_alpha(*userid))
    return 1;

  if (!str_cmp(userid, "new"))
    return 1;

  str = userid;
  while (ch = *(++str))
  {
    if (!is_alnum(ch))
      return 1;
  }
  return (belong(FN_ETC_BADID, userid));
}


int
personal_apply()
{
  char validemail[2][20] = {"ccmail.ncku.edu.tw","mail.ncku.edu.tw"};
  int i,num;
  char *c,/*buf[60],*/brdname[IDLEN + 1];
  PB pb;
  struct tm *t;
  time_t now;
  int thisyear,enteryear;
  
  now = time(0);
  t = localtime(&now);
  
  
  
  if(cuser.numposts < 20 || cuser.numlogins < 500)
  {
    vmsg("¸ê®æ¤£²ÅµLªk¥Ó½Ð­Ó¤HªO");
    return 0;
  }
  
  c = strchr(cuser.email,'@');
  if(c == NULL || (strcmp(c+1,validemail[0]) && strcmp(c+1,validemail[1])))
  {
    vmsg("±zªº E-mail ¤£¦X®æ!");
    return 0;
  }
  
  thisyear = t->tm_year - 11;
  enteryear = (cuser.email[3]-'0') * 10 + (cuser.email[4]-'0');
  
  //¦Ê¦~ÂÎ ecchi float 2012/4/25
  if( (thisyear - enteryear)%100 > 5)
  {
    vmsg("±zªº¨­¥÷¤£¦X®æ!");
    return 0;
  }

  num = rec_num(FN_ETC_PERSONAL,sizeof(PB));
  for(i=0;i<num;i++)
  {
    rec_get(FN_ETC_PERSONAL,&pb,sizeof(PB),i);
    
    c = strchr(pb.email,'@');
    
    if(!strcmp(cuser.userid,pb.userid) || !strncmp(cuser.email,pb.email,c-pb.email))
    {
      if(pb.state & PB_OPEN)
        vmsg("±z¤w¸g¥Ó½Ð¹L­Ó¤HªO¡I");
      else
        vmsg("±zªº¥Ó½Ð®Ñ¥¿¦b¼f®Ö¤¤¡I");
      return 0;
    }
  }
  
  /* ¸ê®æ¼f®Ö³q¹L */
  
  memset(&pb,0,sizeof(PB));
  
  clear();
  vs_head("­Ó¤HªO¥Ó½Ð³æ",str_site);
  
  move(2,0);
  prints("¥Ó½Ð¤H¡G   %s\n",cuser.userid);
  prints("E-mail¡G   %s\n",cuser.email);
  prints("¤W¯¸¦¸¼Æ¡G %d\n",cuser.numlogins);
  prints("¤å³¹¼Æ¡G   %d",cuser.numposts);

  num = DOECHO;
    
  while(1)
  {
    while(1)
    {
      if(!vget(7,0,"¬ÝªO­^¤å¦WºÙ¡G ",brdname, IDLEN - 1, num))
        return 0;
      
      if(is_badid(brdname))
        vmsg("µLªk±µ¨ü³o­ÓªO¦W¡A½Ð¨Ï¥Î­^¤å¦r¥À¡A¨Ã¥B¤£­n¥]§tªÅ®æ");
      else
      {
        sprintf(pb.brdname,"P_%s",brdname);
        if(brd_bno(pb.brdname) >= 0)
          vmsg("¦¹ªO¦W¤w¸g¦³¤H¨Ï¥Î");
        else
          break;
      }
    }
  
    while(1)
      if(vget(8,0,"¬ÝªO¤¤¤å¦WºÙ¡G ",pb.brdtitle, BTLEN + 1, num))
        break;
    
    if(vans("½T©w¸ê®Æ³£¥¿½T¶Ü¡H[y/N]") == 'y')
      break;
    else
      num = GCARRY;
  }
  
  strcpy(pb.userid,cuser.userid);
  strcpy(pb.email,cuser.email);
  pb.state = PB_APPLY;
  
  rec_add(FN_ETC_PERSONAL,&pb,sizeof(PB));
  personal_log(&pb,0);
  
  vmsg("¥Ó½Ð®Ñ¶ñ¼g§¹¦¨¡A½Ðµ¥«Ý¯¸°È¼f®Ö");
  return 0;
  
}

static char
personal_attr(usint state)
{
  if(state & PB_APPLY)
    return 'A';
    
  if(state & PB_OPEN)
    return 'O';
}

static void
personal_item(num, personal)
  int num;
  PB *personal;
{
  if(!mode)
    prints("%6d %c %-12s %-12s %-40s\n", num, personal_attr(personal->state) ,personal->userid,personal->brdname,personal->email);
  else
    prints("%6d %c %-12s %-12s %-40s\n", num, personal_attr(personal->state) ,personal->userid,personal->brdname,personal->brdtitle);
}

static int
personal_body(xo)
  XO *xo;
{
  PB *personal;
  int num, max, tail;

  move(3, 0);
  clrtobot();
  max = xo->max;
  if (max <= 0)
  {
    vmsg("¥Ø«e¨S¦³¸ê®Æ");
    return XO_QUIT;
  }
  personal = (PB *) xo_pool;
  num = xo->top;
  tail = num + XO_TALL;
  if (max > tail)
    max = tail;

  do
  {
    personal_item(++num, personal++);
  } while (num < max);

  return XO_NONE;
}


static int
personal_head(xo)
  XO *xo;
{
  vs_head("­Ó¤HªO²M³æ", str_site);
  outs("\
  [¡ö]Â÷¶} c)­×§ï d)§R°£ s)­«¾ã TAB)¤¤¤åª©¦W/E-mail O)¶}ªO D)°h¥ó [h]elp\n\
\033[44m  ½s¸¹   ¥Ó½Ð¤H       ¬ÝªO¦WºÙ     ");
  if(!mode) 
    outs("E-mail                                  \033[m");
  else 
    outs("¤¤¤åª©¦W                                \033[m");
  return personal_body(xo);
}


static int
personal_load(xo)
  XO *xo;
{
  xo_load(xo, sizeof(PB));
  return personal_body(xo);
}


static int
personal_init(xo)
  XO *xo;
{
  xo_load(xo, sizeof(PB));
  return personal_head(xo);
}


static int
personal_edit(personal,echo)
  PB *personal;
  int echo;
{
  if(echo == DOECHO)
    memset(personal, 0, sizeof(PB));
  if(vget(b_lines, 0, "¥Ó½Ð¤H:", personal->userid, sizeof(personal->userid), echo)
   && vget(b_lines, 0, "¬ÝªO¦W:",personal->brdname, sizeof(personal->brdname), echo)
   && vget(b_lines, 0, "¬ÝªO¼ÐÃD:",personal->brdtitle, sizeof(personal->brdtitle), echo)
   && vget(b_lines, 0, "E-mail:",personal->email, sizeof(personal->email), echo))
    return 1;
  else
    return 0;
}


static int
personal_delete(xo)
  XO *xo;
{

  if (vans(msg_del_ny) == 'y')
  {
    if (!rec_del(xo->dir, sizeof(PB), xo->pos, NULL, NULL))
    {
      return personal_load(xo);
    }
  }
  return XO_FOOT;
}


static int
personal_change(xo)
  XO *xo;
{
  PB *personal, mate;
  int pos, cur;
  
  pos = xo->pos;
  cur = pos - xo->top;
  personal = (PB *) xo_pool + cur;

  mate = *personal;
  personal_edit(personal, GCARRY);
  if (memcmp(personal, &mate, sizeof(PB)))
  {
    rec_put(xo->dir, personal, sizeof(PB), pos);
    move(3 + cur, 0);
    personal_item(++pos, personal);
  }

  return XO_FOOT;
}

static int
personal_switch(xo)
  XO *xo;
{
  mode++;
  mode %= 2;
  return XO_INIT;
}

static int
mail2usr(personal,admin)
  PB *personal;
  int admin;		/* 0:open 1:deny */
{
  HDR hdr;
  time_t now;
  char folder[50],fpath[128];
  char title[2][30] = {"±zªº­Ó¤HªO³q¹L¥Ó½ÐÅo¡I","±zªº­Ó¤HªO¥Ó½Ð®Ñ³Q°h¦^¡I"};
  FILE *fp;
  
  now = time(0);
  
  memset(&hdr,0,sizeof(hdr));

  usr_fpath(folder, personal->userid, fn_dir);
  hdr_stamp(folder, 0 , &hdr, fpath);  
  strcpy(hdr.owner,"SYSOP");
  strcpy(hdr.nick, SYSOPNICK);
  strcpy(hdr.title,title[admin]);
  rec_add(folder, &hdr, sizeof(HDR));
  
  if(fp = fopen(fpath,"w"))
  {
    fprintf(fp,"§@ªÌ: SYSOP (%s)\n¼ÐÃD: %s\n®É¶¡: %s\n",SYSOPNICK,title[admin],ctime(&now));
    fprintf(fp,"¥Ó½Ð¤H:   %s\n",personal->userid);
    fprintf(fp,"­^¤åªO¦W: %s\n",personal->brdname);
    fprintf(fp,"¤¤¤åªO¦W: %s\n\n",personal->brdtitle);
    if(!admin)
      f_suck(fp,"gem/@/@accept");
    else
    {
      fprintf(fp,"[1;33m°h¥ó²z¥Ñ: %s[m\n\n",msg);
      f_suck(fp,"gem/@/@deny");
    }
    
    fclose(fp);
  }
  return 0;
  
  
}

int
sort_compare(p1, p2)
  const void *p1;
  const void *p2;
{
  HDR *a1, *a2;

  a1 = (HDR *) p1;
  a2 = (HDR *) p2;
  return str_cmp(a1->xname,a2->xname);

}

static int
personal_sort(gem)
  char *gem;
{
  HDR *sort;
  int max,fd,total;
  struct stat st;
  
  if ((fd = open(gem, O_RDWR, 0600)) < 0)
    return 0;
  
  if (fstat(fd, &st) || (total = st.st_size) <= 0)
  {
    close(fd);
    return 0;
  }
  f_exlock(fd);
  
  sort = (HDR *) malloc(total);
  read(fd,sort,total);
  
  max = total / sizeof(HDR);
  
  qsort(sort,max,sizeof(HDR),sort_compare);
  
  lseek(fd, (off_t) 0, SEEK_SET);
  write(fd,sort,total);
  f_unlock(fd);
  close(fd);
  free(sort);
  
  return 0;
  
  
}

static int
personal_open(xo)
  XO *xo;
{
  PB *personal;
  int cur,pos, index;
  char fpath[80];
  BRD newboard;
  HDR hdr;
  int bno;
  ACCT acct;
  char gem[5][20] = {"gem/@/@Person_A_E","gem/@/@Person_F_J","gem/@/@Person_K_O","gem/@/@Person_P_T","gem/@/@Person_U_Z"};
  
  pos = xo->pos;
  cur = pos - xo->top;
  personal = (PB *) xo_pool + cur; 
  
  if(personal->state & PB_OPEN)
    return XO_NONE;
  
  if(brd_bno(personal->brdname) >= 0)
  {
    vmsg("ªO¦W¹p¦P");
    return XO_NONE;
  }
  
  if ( bshm->number >= MAXBOARD)
  {
    vmsg("¶W¹L¨t²Î©Ò¯à®e¯Ç¬Ýª©­Ó¼Æ¡A½Ð½Õ¾ã¨t²Î°Ñ¼Æ");
    return XO_NONE;
  }
  
  if(vans("½T©w­n¶}³]¦¹¬ÝªO¶Ü¡H[y/N]") != 'y')
    return XO_NONE;

  memset(&newboard,0,sizeof(newboard));
  
  strcpy(newboard.brdname,personal->brdname);
  sprintf(newboard.title,"¡³ %s",personal->brdtitle);
  newboard.color = 7;
  strcpy(newboard.class,"­Ó¤H");
  strcpy(newboard.BM,personal->userid);
  time(&newboard.bstamp);
//  newboard.readlevel |= PERM_SYSOP;
  newboard.postlevel |= (PERM_POST|PERM_VALID|PERM_BASIC);
  newboard.battr |= BRD_NOTRAN;
  newboard.expiremax = 8000;
  newboard.expiremin = 7500;

  if ((bno = brd_bno("")) >= 0)
  {
    rec_put(FN_BRD, &newboard, sizeof(newboard), bno);
  }  
  else if(rec_add(FN_BRD, &newboard, sizeof(newboard)) < 0)
  {
    vmsg("µLªk«Ø¥ß·sªO");
    return XO_NONE;
  }
                
  sprintf(fpath, "gem/brd/%s", newboard.brdname);
  mak_dirs(fpath);
  mak_dirs(fpath + 4);

  bshm->uptime = 0;             /* force reload of bcache */
  bshm_init();

  /* ¶¶«K¥[¶i NewBoard */

  brd2gem(&newboard, &hdr);
  rec_add("gem/@/@NewBoard", &hdr, sizeof(HDR));
  rec_add("gem/@/@Person_All", &hdr, sizeof(HDR));

  personal_sort("gem/@/@Person_All");

  if((index = (tolower(newboard.brdname[2]) - 'a') / 5) == 5)
    index--;
  rec_add(gem[index], &hdr, sizeof(HDR));
  
  personal_sort(gem[index]);

  personal->state = PB_OPEN;

  rec_put(xo->dir, personal, sizeof(PB), pos);
  move(3 + cur, 0);
  personal_item(++pos, personal);
  
  mail2usr(personal,0);
  if(acct_load(&acct, personal->userid) >= 0)
    if(!(acct.userlevel & PERM_BM))
    {
      acct.userlevel |= PERM_BM;
      acct_save(&acct);
    }

  personal_log(personal,1);

  vmsg("·sªO¦¨¥ß");
  
  return XO_FOOT;
}

static int
personal_deny(xo)
  XO *xo;
{
  PB *personal;
  int pos,cur;

  pos = xo->pos;
  cur = pos - xo->top;
  personal = (PB *) xo_pool + cur;

  if(personal->state & PB_OPEN)
    return XO_NONE;

  if(!vget(b_lines,0,"©Úµ´¶}ªO²z¥Ñ: ",msg,sizeof(msg),DOECHO))
    return XO_NONE;
  
  if(vans("½T©w©Úµ´¦¹¥Ó½Ð¶Ü¡H[y/N]") != 'y')
    return XO_NONE;

  mail2usr(personal,1);

  if (!rec_del(xo->dir, sizeof(PB), xo->pos, NULL, NULL))
  {
    personal_log(personal,2);
    return personal_load(xo);
  }


  return XO_FOOT;
}

static int
personal_help(xo)
  XO *xo;
{
//  film_out(FILM_PB, -1);
  return personal_head(xo);
}

KeyFunc personal_cb[] =
{
  XO_INIT, personal_init,
  XO_LOAD, personal_load,
  XO_HEAD, personal_head,
  XO_BODY, personal_body,

  'c', personal_change,
  's', personal_init,
  'd', personal_delete,
  KEY_TAB, personal_switch,
  'O', personal_open,
  'D', personal_deny,
  'h', personal_help
};

int
personal_admin()
{
  XO *xo;
  utmp_mode(M_OMENU);
  xz[XZ_OTHER - XO_ZONE].xo = xo = xo_new(FN_ETC_PERSONAL);
  xz[XZ_OTHER - XO_ZONE].cb = personal_cb;
  xover(XZ_OTHER);
  free(xo);
  return 0;
}

